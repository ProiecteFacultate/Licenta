package com.dfs.DFS_Client.services;

import com.dfs.DFS_Client.clients.DirectoryClient;
import com.dfs.DFS_Client.clients.DriveClient;
import com.dfs.DFS_Client.models.*;
import org.apache.catalina.User;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

public class DriveService {

    private final DirectoryClient directoryClient;
    private final DriveClient driveClient;

    public DriveService() {
        this.directoryClient = new DirectoryClient();
        this.driveClient = new DriveClient();
    }

    public void createLocalDrive(final List<String> commandTokens, final UserData userData ) {
        if(commandTokens.size() != 4)
        {
            System.out.println( "'mkdrive' command should have 4 arguments!\n" );
            return;
        }

        final String localDrivePath = commandTokens.get( 1 );
        final int localDriveMaximumSize = Integer.parseInt( commandTokens.get( 2 ) );
        final int localDriveMaximumFileSize = Integer.parseInt( commandTokens.get( 3 ) );

        final File file = new File( localDrivePath );
        if( file.exists() )
            file.delete();

        file.mkdir();

        final Status createLocalDriveStatus = driveClient.createLocalDrive( localDrivePath, localDriveMaximumSize, localDriveMaximumFileSize, userData.getUsername() );

        switch ( createLocalDriveStatus.getMessage() ){
            case "Successfully created local drive":
                System.out.println( "Successfully created local drive" );
                userData.setLocalFSPath( localDrivePath );
                userData.setLocalFSMaximumSize( localDriveMaximumSize );
                userData.setLocalFSMaximumFileSize( localDriveMaximumFileSize );
                break;
            default: //"Failed to create local drive" or "ERROR" from client method
                System.out.println( "Failed to create local drive" );
        }
    }

    public void createDirectory( final String parentFolder, final String directoryName, final DirectoryType type, final UserData userData ) {
        final String directoryAbsolutePath = userData.getLocalFSPath() + "\\" + parentFolder.substring( 4 ) + "\\" + directoryName;
        final File newDirectory = new File( directoryAbsolutePath );

        try {
            boolean cratedResult = (type == DirectoryType.FOLDER) ? newDirectory.mkdir() : newDirectory.createNewFile();

            if( cratedResult ) {
                final String directoryRelativePath = parentFolder + "\\" + directoryName;
                final DirectoryMetadata newDirectoryMetadata = new DirectoryMetadata( directoryRelativePath, type, 0 );
                userData.getDirectoriesMetadata().add( newDirectoryMetadata );
                System.out.println( "Successfully created directory on local drive" );
            }

        } catch ( final IOException e) {
            System.out.println( "Failed to create directory on local drive" );
        }
    }

    public void writeFile( final String filePath, byte[] fileData, final int bytesToWrite, final UserData userData ) {
        final String fileAbsolutePath = userData.getLocalFSPath() + "\\" + filePath.substring( 5 );
        final File localDriveFolder = new File( userData.getLocalFSPath() );
        final int freeSpaceInLocalDrive = userData.getLocalFSMaximumSize() - (int) localDriveFolder.getTotalSpace();
        final int bytesToWriteToFile = Math.min( freeSpaceInLocalDrive, userData.getLocalFSMaximumFileSize() );

        try {
            Files.write( Path.of( fileAbsolutePath ), Arrays.copyOfRange( fileData, 0 , bytesToWriteToFile ) );

            final DirectoryMetadata writtenFileMetadata = userData.getDirectoriesMetadata().stream()
                            .filter( directoryMetadata -> directoryMetadata.getDirectoryRelativePath().equals( filePath ) )
                            .findAny()
                            .get();

            writtenFileMetadata.setFileLength( bytesToWriteToFile );

            System.out.println( "Wrote " + bytesToWriteToFile + " bytes to local drive" );
        } catch ( final IOException e) {
            System.out.println( "Failed to write file to local drive" );
        }
    }

    public Pair<Status, ReadResponse> readFile( final String filePath, final int startPosition, final int bytesToRead, final UserData userData ) {
        final String fileAbsolutePath = userData.getLocalFSPath() + "\\" + filePath.substring( 5 );
        final File file = new File( fileAbsolutePath );

        if( !file.exists() )
            return Pair.of( new Status( "File does not exist on local drive" ), null);

        try( RandomAccessFile raf = new RandomAccessFile( file, "r") ) {
            final DirectoryMetadata fileMetadata = userData.getDirectoriesMetadata().stream()
                    .filter( directoryMetadata -> directoryMetadata.getDirectoryRelativePath().equals( filePath ) )
                    .findAny()
                    .get();

            if( startPosition >= file.length() )
                return Pair.of( new Status( "File read" ), new ReadResponse( 0, new byte[ 0 ] ) );

            int bytesToReadFromDriveFile = Math.min( (int) file.length() - startPosition, bytesToRead );
            raf.seek( startPosition );
            byte[] buffer = new byte[ bytesToReadFromDriveFile ];
            int bytesRead = raf.read( buffer );

            if( bytesRead == -1 )
                return Pair.of( new Status( "Failed to read file from local drive" ), null);

            return Pair.of( new Status( "File read" ), new ReadResponse( bytesToReadFromDriveFile, buffer ));
        } catch ( final IOException e) {
            return Pair.of( new Status( "Failed to read file from local drive" ), null);
        }
    }

    public void syncDrive( final UserData userData ) {
        //read user data (maybe someone changed it)
        final Pair<Status, UserData> getUserDataResult = directoryClient.getUserData(userData.getUsername() );

        if( !getUserDataResult.getKey().getMessage().equals( "Retrieved user data" ) ) {
            System.out.println( "Failed to sync drive to local drive" );
            return;
        }

        List<DirectoryMetadata> directoriesOnLocalDrive = new ArrayList<>();

        try {
            getAllDirectoriesOnLocalDrive( userData.getLocalFSPath(), userData.getLocalFSPath(), directoriesOnLocalDrive );
        } catch ( final Exception exception ) {
            System.out.println( "Failed to sync drive to local drive" );
            return;
        }

        final UserData newUserData = getUserDataResult.getValue();
        final List<DirectoryMetadata> directoriesNotInLocalDrive = newUserData.getDirectoriesMetadata().stream()
                .filter( directoryMetadata -> !directoriesOnLocalDrive.stream()
                                              .map( DirectoryMetadata::getDirectoryRelativePath )
                                              .collect( Collectors.toList() ).contains( directoryMetadata.getDirectoryRelativePath() ) )
                .collect( Collectors.toList() );

        for( final DirectoryMetadata directoryMetadata : directoriesNotInLocalDrive ) {
            int lastSlashIndex = directoryMetadata.getDirectoryRelativePath().lastIndexOf('\\');
            final String parentFolder = directoryMetadata.getDirectoryRelativePath().substring( 0, lastSlashIndex );
            final String directoryName = directoryMetadata.getDirectoryRelativePath().substring(lastSlashIndex + 1);

            createDirectory( parentFolder, directoryName, directoryMetadata.getType(), userData );
        }

        System.out.println( "Successfully sync drive to local drive" );
    }

    private void getAllDirectoriesOnLocalDrive( final String localDrivePath, final String directoryPath, List<DirectoryMetadata> directoriesMetadata ) throws IOException {
        final File directory = new File( directoryPath );
        final Path absoluteLocalDirectoryPath = directory.toPath();
        final BasicFileAttributes attributes = Files.readAttributes( absoluteLocalDirectoryPath, BasicFileAttributes.class );

        final DirectoryType type = attributes.isDirectory() ? DirectoryType.FOLDER : DirectoryType.FILE;
        int size = (int) attributes.size();

        final String directoryPathAsString = absoluteLocalDirectoryPath.toString();
        int directoryNameStartIndexInFullPath = directoryPathAsString.indexOf( localDrivePath + "\\" ) + ( localDrivePath + "\\" ).length();
        final String directoryPathNonRelativized = directoryPathAsString.substring( directoryNameStartIndexInFullPath );
        final String directoryPathRelativized = "Root\\" + directoryPathNonRelativized;

        DirectoryMetadata directoryMetadata = new DirectoryMetadata( directoryPathRelativized, type, 0 );
        directoriesMetadata.add( directoryMetadata );

        if ( attributes.isDirectory() )
            for ( File child : directory.listFiles() ) {
                final Path childPath = child.toPath();
                getAllDirectoriesOnLocalDrive( localDrivePath, childPath.toString(), directoriesMetadata );
            }
    }
}
