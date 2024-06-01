package com.dfs.DFS_Client.services;

import com.dfs.DFS_Client.clients.DirectoryClient;
import com.dfs.DFS_Client.clients.DriveClient;
import com.dfs.DFS_Client.models.*;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.BasicFileAttributes;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
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

    public void createLocalDrive( final List<String> commandTokens, final LocalUserData localUserData ) {
        if(commandTokens.size() != 4)
        {
            System.out.println( "'mkdrive' command should have 4 arguments!\n" );
            return;
        }

        final String localDrivePath = commandTokens.get( 1 );
        final int localDriveMaximumSize = Integer.parseInt( commandTokens.get( 2 ) );
        final int localDriveMaximumFileSize = Integer.parseInt( commandTokens.get( 3 ) );

        final File localDriveFolder = new File( localDrivePath );
        if( localDriveFolder.exists() ) {
            final boolean localDriveFolderDeleted = localDriveFolder.delete();
            if( !localDriveFolderDeleted ) {
                System.out.println( "Failed to create local drive folder." );
                return;
            }
        }

        boolean localDriveFolderCreated = localDriveFolder.mkdir();

        if( !localDriveFolderCreated ) {
            System.out.println( "Failed to create local drive folder." );
            return;
        }

        final String localDriveDataFolderPath = localDrivePath + "\\" + "Data";
        final File localDriveDataFolder = new File( localDriveDataFolderPath );
        boolean dataFolderCreated = localDriveDataFolder.mkdir();

        if( !dataFolderCreated ) {
            System.out.println( "Failed to create local drive folder." );
            return;
        }

        final String localDriveMetadataFilePath = localDrivePath + "\\" + "Metadata";
        final File localDriveMetadataFile = new File( localDriveMetadataFilePath );

        try {
            final boolean metadataFileCreated = localDriveMetadataFile.createNewFile();
            if( !metadataFileCreated ) {
                System.out.println( "Failed to create local drive folder." );
                return;
            }

            final LocalUserData userDataToWrite = new LocalUserData(); //we make a new object here and then update the actual userData below in case this try fails
            userDataToWrite.setUsername( localUserData.getUsername() );
            userDataToWrite.setLocalFSPath( localDrivePath );
            userDataToWrite.setLocalFSMaximumSize( localDriveMaximumSize );
            userDataToWrite.setLocalFSMaximumFileSize( localDriveMaximumFileSize );

            LocalDriveHandler.writeToMetadataFile( localDrivePath, userDataToWrite );;
        } catch ( final IOException exception ) {
            System.out.println( "Failed to create local drive folder." );
            return;
        }

        localUserData.setLocalFSPath( localDrivePath );
        localUserData.setLocalFSMaximumSize( localDriveMaximumSize );
        localUserData.setLocalFSMaximumFileSize( localDriveMaximumFileSize );

        System.out.println( "Successfully created local drive at " + localDrivePath );
    }

    public void linkLocalDrive( final List<String> commandTokens, final LocalUserData localUserData ) {
        if(commandTokens.size() != 2)
        {
            System.out.println( "'linkdrive' command should have 2 arguments!\n" );
            return;
        }

        final String localDrivePath = commandTokens.get( 1 );

        try {
            final LocalUserData userDataOnLocalDrive = LocalDriveHandler.readMetadataFile( localDrivePath );
            localUserData.setLocalFSPath( localDrivePath );
            localUserData.setLocalFSMaximumSize( userDataOnLocalDrive.getLocalFSMaximumSize() );
            localUserData.setLocalFSMaximumFileSize( userDataOnLocalDrive.getLocalFSMaximumFileSize() );
            localUserData.setDirectoriesMetadata( userDataOnLocalDrive.getDirectoriesMetadata() );

            System.out.println( "Successfully linked local drive at " + localDrivePath );
        } catch ( final IOException exception ) {
            System.out.println( "Failed to link local drive at " + localDrivePath );
        }
    }

    public void createDirectory( final String parentFolder, final String directoryName, final DirectoryType type, final LocalUserData localUserData) {
        final String directoryAbsolutePath = localUserData.getLocalFSPath() + "\\Data\\" + parentFolder.substring( 4 ) + "\\" + directoryName;
        final File newDirectory = new File( directoryAbsolutePath );

        try {
            boolean cratedResult = (type == DirectoryType.FOLDER) ? newDirectory.mkdir() : newDirectory.createNewFile();

            if( cratedResult ) {
                final String directoryRelativePath = parentFolder + "\\" + directoryName;
                final DirectoryMetadata newDirectoryMetadata = new DirectoryMetadata( directoryRelativePath, type, 0 );

                LocalDriveHandler.addDirectoryToListForUser( localUserData.getLocalFSPath(), newDirectoryMetadata );
                localUserData.getDirectoriesMetadata().add( newDirectoryMetadata );
                System.out.println( "Successfully created directory on local drive" );
            }

        } catch ( final IOException e) {
            System.out.println( "Failed to create directory on local drive" );
        }
    }

    public void writeFile( final String filePath, byte[] fileData, final int bytesToWrite, final LocalUserData localUserData) {
        final String fileAbsolutePath = localUserData.getLocalFSPath() + "\\Data\\" + filePath.substring( 5 );
        final File localDriveFolder = new File( localUserData.getLocalFSPath() );
        final int freeSpaceInLocalDrive = localUserData.getLocalFSMaximumSize() - getTotalSpaceOccupiedOnDrive( localUserData );
        final int bytesToWriteToFile = Math.min( freeSpaceInLocalDrive, Math.min( localUserData.getLocalFSMaximumFileSize(), bytesToWrite ) );

        try {
            Files.write( Path.of( fileAbsolutePath ), Arrays.copyOfRange( fileData, 0 , bytesToWriteToFile ) );
            LocalDriveHandler.updateFileSize( localUserData.getLocalFSPath(), filePath, bytesToWriteToFile );

            System.out.println( "Wrote " + bytesToWriteToFile + " bytes to local drive" );
        } catch ( final IOException e) {
            System.out.println( "Failed to write file to local drive" );
        }
    }

    public Pair<Status, ReadResponse> readFile( final String fileRelativePath, final int startPosition, final int bytesToRead, final LocalUserData localUserData) {
        final String fileAbsolutePath = localUserData.getLocalFSPath() + "\\Data\\" + fileRelativePath.substring( 5 );
        final File file = new File( fileAbsolutePath );

        if( !file.exists() )
            return Pair.of( new Status( "File does not exist on local drive" ), null);

        try( RandomAccessFile raf = new RandomAccessFile( file, "r") ) {
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

    public void syncDrive( final LocalUserData localUserData) {
        final Pair<Status, ServerUserData> getServerUserDataResult = directoryClient.getServerUserData(localUserData.getUsername() );

        if( !getServerUserDataResult.getKey().getMessage().equals( "Retrieved user data" ) ) {
            System.out.println( "Failed to sync drive to local drive" );
            return;
        }

        List<DirectoryMetadata> directoriesOnLocalDrive = new ArrayList<>();

        try {
            getAllDirectoriesOnLocalDrive( localUserData.getLocalFSPath(), localUserData.getLocalFSPath(), directoriesOnLocalDrive );
            final List<String> defaultDirectoryNamesOnLocal = List.of( "Root\\", "Root\\Data", "Root\\Metadata" );
            List<DirectoryMetadata> defaultLocalDirectories = directoriesOnLocalDrive.stream()
                    .filter( directoryMetadata -> defaultDirectoryNamesOnLocal.contains( directoryMetadata.getDirectoryRelativePath() ) )
                    .collect( Collectors.toList() );

            directoriesOnLocalDrive.removeAll( defaultLocalDirectories );
            directoriesOnLocalDrive.forEach( directoryMetadata ->
                    directoryMetadata.setDirectoryRelativePath( directoryMetadata.getDirectoryRelativePath().replace( "\\Data", "" ) ) );
        } catch ( final Exception exception ) {
            System.out.println( "Failed to sync drive to local drive" );
            return;
        }

        final ServerUserData serverUserData = getServerUserDataResult.getValue();
        final File localDriveFolder = new File( localUserData.getLocalFSPath() );

        //Finds the directories that are on server and not on local and creates & writes them on local
        final List<DirectoryMetadata> directoriesNotInLocalDrive = serverUserData.getDirectoriesMetadata().stream()
                .filter( directoryMetadata -> !directoriesOnLocalDrive.stream()
                                              .map( DirectoryMetadata::getDirectoryRelativePath )
                                              .collect( Collectors.toList() ).contains( directoryMetadata.getDirectoryRelativePath() ) )
                .collect( Collectors.toList() );

        for( final DirectoryMetadata directoryMetadata : directoriesNotInLocalDrive ) {
            //create the directories that are on remote and not on local drive
            int lastSlashIndex = directoryMetadata.getDirectoryRelativePath().lastIndexOf('\\');
            final String parentFolder = directoryMetadata.getDirectoryRelativePath().substring( 0, lastSlashIndex );
            final String directoryName = directoryMetadata.getDirectoryRelativePath().substring(lastSlashIndex + 1);

            createDirectory( parentFolder, directoryName, directoryMetadata.getType(), localUserData );

            //write data on the created files (on for FILE)
            if( directoryMetadata.getType() == DirectoryType.FOLDER )
                continue;

            final int freeSpaceInLocalDrive = localUserData.getLocalFSMaximumSize() - getTotalSpaceOccupiedOnDrive( localUserData );
            final int bytesToWriteToFile = Math.min( freeSpaceInLocalDrive, Math.min( localUserData.getLocalFSMaximumFileSize(), directoryMetadata.getFileLength() ) );

            if( bytesToWriteToFile == 0 )
                continue;

            final Pair<Status, ReadResponse> readFullFileFromServerResult = directoryClient.readFile( directoryMetadata.getDirectoryRelativePath(), 0,
                    bytesToWriteToFile, localUserData.getUsername() );

            final ReadResponse readResponse = readFullFileFromServerResult.getValue();

            switch ( readFullFileFromServerResult.getKey().getMessage() ) {
                case "File read":
                    if ( readResponse.getBytesRead() == bytesToWriteToFile )
                        writeFile(directoryMetadata.getDirectoryRelativePath(), readResponse.getBuffer(), bytesToWriteToFile, localUserData);
                    else {
                        System.out.println("Failed to read file " + directoryMetadata.getDirectoryRelativePath() + " from server");
                        return;
                    }
                    break;
                case "File does not exist":
                    System.out.println("File " + directoryMetadata.getDirectoryRelativePath() + " do not exist on server");
                    return;
                default: //"Failed to read file" or "ERROR" in client method
                    System.out.println("Failed to read file " + directoryMetadata.getDirectoryRelativePath() + " from server");
                    return;
            }
        }

        //Finds the directories that are both on server and on local and checks if data matches, otherwise updates them on local
        final List<DirectoryMetadata> directoriesOnServerAndInLocalDrive = serverUserData.getDirectoriesMetadata().stream()
                .filter( directoryMetadata -> directoriesOnLocalDrive.stream()
                        .map( DirectoryMetadata::getDirectoryRelativePath )
                        .collect( Collectors.toList() ).contains( directoryMetadata.getDirectoryRelativePath() ) )
                .collect( Collectors.toList() );

        for( final DirectoryMetadata directoryOnServerMetadata : directoriesOnServerAndInLocalDrive ) {
            final DirectoryMetadata directoryOnLocalMetadata = directoriesOnLocalDrive.stream()
                    .filter( local -> local.getDirectoryRelativePath().equals( directoryOnServerMetadata.getDirectoryRelativePath() ) )
                    .findAny().get();

            final boolean filesOnLocalContainsSameStartData = checkFilesMatch( directoryOnLocalMetadata.getDirectoryRelativePath(), directoryOnLocalMetadata,
                    directoryOnServerMetadata, localUserData );

            if( !filesOnLocalContainsSameStartData ) {
                final int freeSpaceInLocalDrive = localUserData.getLocalFSMaximumSize() - getTotalSpaceOccupiedOnDrive( localUserData );
                final int bytesThatCouldBeWrittenOnLocal = Math.min( freeSpaceInLocalDrive, Math.min( localUserData.getLocalFSMaximumFileSize(),
                        directoryOnServerMetadata.getFileLength() ) );
                final Pair<Status, ReadResponse> readStartFileFromServerResult = directoryClient.readFile( directoryOnLocalMetadata.getDirectoryRelativePath(), 0,
                        bytesThatCouldBeWrittenOnLocal, localUserData.getUsername() );

                final ReadResponse readResponse = readStartFileFromServerResult.getValue();

                switch ( readStartFileFromServerResult.getKey().getMessage() ) {
                    case "File read":
                        if ( readResponse.getBytesRead() == bytesThatCouldBeWrittenOnLocal )
                            writeFile(directoryOnLocalMetadata.getDirectoryRelativePath(), readResponse.getBuffer(), bytesThatCouldBeWrittenOnLocal, localUserData);
                        else {
                            System.out.println("Failed to update file " + directoryOnLocalMetadata.getDirectoryRelativePath() + " on local drive");
                            return;
                        }
                        break;
                    case "File does not exist":
                        System.out.println("File " + directoryOnLocalMetadata.getDirectoryRelativePath() + " do not exist on server");
                        return;
                    default: //"Failed to read file" or "ERROR" in client method
                        System.out.println("Failed to update file " + directoryOnLocalMetadata.getDirectoryRelativePath() + " on local drive");
                        return;
                }
            }
        }

        System.out.println( "Successfully sync drive to local drive" );
    }

    public void deleteDirectory( final String directoryPath, final LocalUserData localUserData) {
        final String directoryAbsolutePath = localUserData.getLocalFSPath() + "\\Data\\" + directoryPath.substring( 5 );
        final File directory = new File( directoryAbsolutePath );

        if( !directory.exists() )
            System.out.println( "Directory does not exist on local drive" );

        try {
            //delete subdirectory metadata entries from UsersData

            final List<DirectoryMetadata> directoryMetadataToDelete = localUserData.getDirectoriesMetadata().stream()
                    .filter( directoryMetadata -> directoryMetadata.getDirectoryRelativePath().startsWith( directoryPath ) )
                    .collect( Collectors.toList() );

            localUserData.getDirectoriesMetadata().removeAll( directoryMetadataToDelete );
            LocalDriveHandler.writeToMetadataFile(localUserData.getLocalFSPath(), localUserData );

            deleteDirectoryRecursive( directory ); //even if this fails we already deleted from UsersData so it won't be counted and overwritten if necessary
            System.out.println( "Deleted directory on local drive" );
        } catch ( final IOException exception ) {
            System.out.println( "Failed to delete directory on local drive" );
        }
    }

    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////

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

        DirectoryMetadata directoryMetadata = new DirectoryMetadata( directoryPathRelativized, type, size );
        directoriesMetadata.add( directoryMetadata );

        if ( attributes.isDirectory() )
            for ( File child : directory.listFiles() ) {
                final Path childPath = child.toPath();
                getAllDirectoriesOnLocalDrive( localDrivePath, childPath.toString(), directoriesMetadata );
            }
    }

    //this makes the check only for the beginning of the file (what is or could be written on local drive) not the whole file on server
    private boolean checkFilesMatch( final String fileRelativePath, final DirectoryMetadata localFileMetadata, final DirectoryMetadata serverFileMetadata,
                                     final LocalUserData localUserData ) {

        if( localFileMetadata.getFileLength() > serverFileMetadata.getFileLength() )
            return false;

        final File localDriveFolder = new File( localUserData.getLocalFSPath() );
        final int freeSpaceInLocalDrive = localUserData.getLocalFSMaximumSize() - getTotalSpaceOccupiedOnDrive( localUserData );
        final int bytesThatCouldBeWrittenOnLocal = Math.min( freeSpaceInLocalDrive, Math.min( localUserData.getLocalFSMaximumFileSize(), serverFileMetadata.getFileLength() ) );

        if( localFileMetadata.getFileLength() < bytesThatCouldBeWrittenOnLocal )
            return false;

        //else the size is fine, but we need to see if content matches
        Pair<Status, ReadResponse> readFromLocalDriveResult = readFromLocalDriveResult = readFile( fileRelativePath, 0, bytesThatCouldBeWrittenOnLocal, localUserData );
        if( readFromLocalDriveResult != null && readFromLocalDriveResult.getKey().getMessage().equals( "File read" ) ) {
            if( readFromLocalDriveResult.getValue().getBytesRead() != bytesThatCouldBeWrittenOnLocal )
                return false; //if we fail to read the required number of byres then update it to be sure

            final byte[] fileDataOnLocal = Arrays.copyOfRange( readFromLocalDriveResult.getValue().getBuffer(), 0 , bytesThatCouldBeWrittenOnLocal );
            final MessageDigest localFileDataHash = hashFileContent( fileDataOnLocal );

            if( localFileDataHash != null ) {
                final Pair<Status, FileHashResponse> getFileHashOnServerResult = directoryClient.getFileContentHash( fileRelativePath, bytesThatCouldBeWrittenOnLocal,
                        localUserData.getUsername() );

                if( getFileHashOnServerResult.getKey().getMessage().equals( "Hashed file" ) ) {
                    final byte[] localFileHashBytes = localFileDataHash.digest();
                    final byte[] serverFileHashBytes = getFileHashOnServerResult.getValue().getHashValue();
                    return Arrays.equals( localFileHashBytes, serverFileHashBytes );
                }

                return false;
            }
            else
                return false; //if we fail to hash then update it to be sure
        }
        else
            return false; //if we fail to read then update it to be sure
    }

    private MessageDigest hashFileContent( final byte[] fileContent )  {
        try {
            String algorithmName = "SHA-256";
            MessageDigest digest = null;
            digest = MessageDigest.getInstance(algorithmName);
            digest.update( fileContent );

            return digest;
        } catch (NoSuchAlgorithmException e) {
            System.err.println( "Failed to hash file " + fileContent );
            return null;
        }
    }

    private int getTotalSpaceOccupiedOnDrive( final LocalUserData localUserData ) {
        try {
            //don't used the one given as parameter to not break other stuff
            final LocalUserData userDataOnLocalDrive = LocalDriveHandler.readMetadataFile(localUserData.getLocalFSPath() );
            int totalUsedSpace = 0;

            for( DirectoryMetadata directoryMetadata : userDataOnLocalDrive.getDirectoriesMetadata() )
                totalUsedSpace += directoryMetadata.getFileLength();

            return totalUsedSpace;
        } catch ( final IOException exception ) {
            System.out.println( "Failed to get total space used on local drive" );
            return localUserData.getLocalFSMaximumFileSize(); //so in our calculation later won't be any free space
        }
    }

    private void deleteDirectoryRecursive( File directory ) {
        if ( !directory.exists() ) {
            return;
        }

        if ( directory.isDirectory() ) {
            File[] contents = directory.listFiles();
            if ( contents != null ) {
                for ( File file : contents ) {
                    deleteDirectoryRecursive( file );
                }
            }
            directory.delete();
        } else {
            directory.delete();
        }
    }
}
