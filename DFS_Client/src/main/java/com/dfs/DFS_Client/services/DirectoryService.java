package com.dfs.DFS_Client.services;

import com.dfs.DFS_Client.models.*;
import com.dfs.DFS_Client.clients.DirectoryClient;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;

public class DirectoryService {

    private final DirectoryClient directoryClient;
    private final DriveService driveService;

    public DirectoryService() {
        this.directoryClient = new DirectoryClient();
        this.driveService = new DriveService();
    }

    public void createDirectory( final List<String> commandTokens, final LocalUserData localUserData) {
        if(commandTokens.size() != 4)
        {
            System.out.println( "'mkdir' command should have 4 arguments!\n" );
            return;
        }

        final String parentFolder = commandTokens.get( 1 );
        final String directoryName = commandTokens.get( 2 );
        final DirectoryType type = DirectoryType.valueOf( commandTokens.get( 3 ) );
        final Status createDirectoryStatus = directoryClient.createDirectory( parentFolder, directoryName, type, localUserData.getUsername() );

        switch ( createDirectoryStatus.getMessage() ){
            case "Directory created":
                System.out.println( "Directory created" );

                if( !localUserData.getLocalFSPath().isEmpty() )
                    driveService.createDirectory( parentFolder, directoryName, type, localUserData);
              break;
            case "Directory already exists":
                System.out.println( "Directory already exists" );
                break;
            case "Parent directory does not exist":
                System.out.println( "Parent directory does not exist" );
                break;
            default: //"Failed to create folder" or "ERROR" from client method
                System.out.println( "Failed to create directory" );
        }
    }

    public void writeFile( final List<String> commandTokens, final LocalUserData localUserData) {
        if(commandTokens.size() != 3)
        {
            System.out.println( "'write' command should have 3 arguments!\n" );
            return;
        }

        final String filePath = commandTokens.get( 1 );
        final int bytesToWrite = Integer.parseInt( commandTokens.get( 2 ) );

        final Scanner scanner = new Scanner(System.in);
        final StringBuilder buffer = new StringBuilder();

        String line;
        do {
            line = scanner.nextLine();
            if ( !line.equals( "EOF" ) )
                buffer.append( line ).append( "\n" );
        } while ( !line.equals( "EOF" ) );

        final byte[] fileData = Arrays.copyOfRange( buffer.toString().getBytes(), 0 , bytesToWrite );

        final Status writeFileResult = directoryClient.writeFile( filePath, fileData, bytesToWrite, localUserData.getUsername() );

        switch ( writeFileResult.getMessage() ) {
            case "File written":
                System.out.println( "File written" );

                try {
                    final String localDrivePath = localUserData.getLocalFSPath();
                    if( !localDrivePath.isEmpty() && LocalDriveHandler.getDirectoryMetadataForFile( localDrivePath, filePath ).isPresent() )
                        driveService.writeFile( filePath, fileData, bytesToWrite, localUserData);
                } catch ( final IOException exception) {

                }

                break;
            case "File does not exist":
                System.out.println( "File does not exist" );
                break;
            default: //"Failed to write file" or "ERROR" in client method
                System.out.println( "Failed to write file" );
        }
    }

    public void readFile( final List<String> commandTokens, final LocalUserData localUserData) {
        if(commandTokens.size() != 4)
        {
            System.out.println( "'read' command should have 4 arguments!\n" );
            return;
        }

        final String filePath = commandTokens.get( 1 );
        final int startPosition = Integer.parseInt( commandTokens.get( 2 ) );
        final int bytesToRead = Integer.parseInt( commandTokens.get( 3 ) );

        byte[] bytesReadFromLocal = new byte[ 0 ];
        int bytesReadFromLocalDrive = 0;
        int bytesToReadFromServer = bytesToRead;
        int startingPositionOnServer = startPosition;
        String dataAsString;

        //read the first bytes from local drive
        Pair<Status, ReadResponse> readFromLocalDriveResult = null;

        try {
            final String localDrivePath = localUserData.getLocalFSPath();
            if( !localDrivePath.isEmpty() && LocalDriveHandler.getDirectoryMetadataForFile( localDrivePath, filePath ).isPresent() )
                readFromLocalDriveResult = driveService.readFile( filePath, startPosition, bytesToRead, localUserData);
        } catch ( final IOException exception) {

        }

        if( readFromLocalDriveResult != null && readFromLocalDriveResult.getKey().getMessage().equals( "File read" ) ) {
            bytesReadFromLocalDrive = readFromLocalDriveResult.getValue().getBytesRead();
            bytesReadFromLocal = readFromLocalDriveResult.getValue().getBuffer();
            bytesToReadFromServer -= readFromLocalDriveResult.getValue().getBytesRead();
            startingPositionOnServer += readFromLocalDriveResult.getValue().getBytesRead();
            System.out.println( readFromLocalDriveResult.getValue().getBytesRead() + " bytes read from local drive ");
        }
        else
            System.out.println( "0 bytes read from local drive ");

        //we read all of them from local (ignore case where we want to read 0 in command)
        if( bytesToReadFromServer == 0) {
            System.out.println( "Total bytes read " + bytesToRead + "/" + bytesToRead);
            dataAsString = new String( bytesReadFromLocal );
            System.out.println( dataAsString );

            return;
        }

        final Pair<Status, ReadResponse> readFromServerResult = directoryClient.readFile( filePath, startingPositionOnServer, bytesToReadFromServer, localUserData.getUsername() );

        switch ( readFromServerResult.getKey().getMessage() ) {
            case "File read":
                System.out.println( readFromServerResult.getValue().getBytesRead() + " bytes read from server ");
                int totalBytesRead = bytesReadFromLocalDrive + readFromServerResult.getValue().getBytesRead();
                System.out.println( "Total bytes read " + totalBytesRead + "/" + bytesToRead);

                dataAsString = new String( bytesReadFromLocal );
                System.out.print( dataAsString );

                dataAsString = new String( readFromServerResult.getValue().getBuffer() );
                System.out.print( dataAsString );

                System.out.println();
                break;
            case "File does not exist":
                if( bytesReadFromLocalDrive != 0 ) {
                    System.out.println( "Total bytes read " + bytesReadFromLocalDrive + "/" + bytesToRead);

                    dataAsString = new String( bytesReadFromLocal );
                    System.out.println( dataAsString );
                }
                System.out.println( "File does not exist on server" );
                break;
            default: //"Failed to read file" or "ERROR" in client method
                if( bytesReadFromLocalDrive != 0 ) {
                    System.out.println( "Total bytes read " + bytesReadFromLocalDrive + "/" + bytesToRead);

                    dataAsString = new String( bytesReadFromLocal );
                    System.out.println( dataAsString );
                }
                System.out.println( "Failed to read file from server" );
        }
    }

    public void deleteDirectory( final List<String> commandTokens, final LocalUserData localUserData ) {
        if(commandTokens.size() != 2)
        {
            System.out.println( "'delete' command should have 2 arguments!\n" );
            return;
        }

        final String directoryPath = commandTokens.get( 1 );

        final Status deleteDirectoryResult = directoryClient.deleteDirectory( directoryPath, localUserData.getUsername() );

        switch ( deleteDirectoryResult.getMessage() ) {
            case "Deleted directory":
                System.out.println( "Deleted directory" );

                try {
                    final String localDrivePath = localUserData.getLocalFSPath();
                    if( !localDrivePath.isEmpty() && LocalDriveHandler.getDirectoryMetadataForFile( localDrivePath, directoryPath ).isPresent() )
                        driveService.deleteDirectory( directoryPath, localUserData );
                } catch ( final IOException exception) {

                }
                break;
            case "Directory does not exist":
                System.out.println( "Directory does not exist" );
                break;
            default: //"Failed to delete directory" or "ERROR" in client method
                System.out.println( "Failed to delete directory" );
        }
    }
}
