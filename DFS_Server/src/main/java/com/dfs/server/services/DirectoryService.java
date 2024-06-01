package com.dfs.server.services;

import com.dfs.server.models.*;
import org.apache.catalina.User;
import org.springframework.stereotype.Service;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class DirectoryService {

    private final String dfsServerUsersDataFilePath = "D:\\Facultate\\Licenta\\HardDisks\\DFS_Server_Disk\\UsersData";
    private final String dfsServerDataFolderPath = "D:\\Facultate\\Licenta\\HardDisks\\DFS_Server_Disk\\Data";

    public Status createUserFolder( final String username ) {
        final String userFolderPath = dfsServerDataFolderPath + "\\" + username;
        final File folder = new File( userFolderPath );
        final boolean folderCreated = folder.mkdir();

        if( folderCreated )
           return new Status( "User folder created" );

        return new Status( "Failed to create user folder" );
    }

    public Status createDirectory( final CreateDirectoryPayload createDirectoryPayload ) {
        final String parentAbsolutePath = dfsServerDataFolderPath + "\\" + createDirectoryPayload.getUsername() + createDirectoryPayload.getParentFolder().substring( 4 );
        final File parentFolder = new File( parentAbsolutePath );
        if( !parentFolder.exists() )
            return new Status( "Parent directory does not exist" );

        final String newDirectoryAbsolutePath = parentAbsolutePath + "\\" + createDirectoryPayload.getDirectoryName();
        final File newDirectory = new File( newDirectoryAbsolutePath );

        if( !newDirectory.exists() ) {
            try {
                final boolean folderCreated = ( createDirectoryPayload.getType() == DirectoryType.FOLDER ) ? newDirectory.mkdir() : newDirectory.createNewFile();

                if( folderCreated ) {
                    final String relativePath = createDirectoryPayload.getParentFolder() + "\\" + createDirectoryPayload.getDirectoryName();
                    final DirectoryMetadata newDirectoryMetadata = new DirectoryMetadata( relativePath, createDirectoryPayload.getType(), 0 );
                    UsersSerializableData.addDirectoryToListForUser( dfsServerUsersDataFilePath, newDirectoryMetadata, createDirectoryPayload.getUsername() );

                    return new Status("Directory created");
                }

                return new Status( "Failed to create directory" );
            } catch ( final IOException exception ) {
                return new Status( "Failed to create directory" );
            }
        }
        else
            return new Status( "Directory already exists" );
    }

    public Status writeFile( final WriteFilePayload writeFilePayload ) {
        final String fileAbsolutePath = dfsServerDataFolderPath + "\\" + writeFilePayload.getUsername() + "\\" + writeFilePayload.getFilePath().substring( 4 );
        final File file = new File( fileAbsolutePath );

        if( !file.exists() )
            return new Status( "File does not exist" );

        try {
            Files.write( Path.of( fileAbsolutePath ), writeFilePayload.getFileData() );

            final String fileRelativePath = writeFilePayload.getFilePath();
            UsersSerializableData.updateFileSize( dfsServerUsersDataFilePath, fileRelativePath, writeFilePayload.getBytesToWrite(), writeFilePayload.getUsername() );

            return new Status( "File written" );
        } catch ( final IOException exception ) {
            return new Status( "Failed to write file" );
        }
    }

    public Pair<Status, ReadResponse> readFile( final ReadFilePayload readFilePayload ) {
        final String fileAbsolutePath = dfsServerDataFolderPath + "\\" + readFilePayload.getUsername() + "\\" + readFilePayload.getFilePath().substring( 5 );
        final File file = new File( fileAbsolutePath );

        if( !file.exists() )
            return Pair.of( new Status( "File does not exist" ), null);

        try( RandomAccessFile raf = new RandomAccessFile( file, "r") ) {
            if( readFilePayload.getStartPosition() >= file.length() )
                return Pair.of( new Status( "File read" ), new ReadResponse( 0, new byte[ 0 ] ) );

            int bytesToRead = Math.min( (int) file.length() - readFilePayload.getStartPosition(), readFilePayload.getBytesToRead() );
            raf.seek( readFilePayload.getStartPosition() );
            byte[] buffer = new byte[ bytesToRead ];
            int bytesRead = raf.read( buffer );

            if( bytesRead == -1 )
                return Pair.of( new Status( "Failed to read file" ), null);

            return Pair.of( new Status( "File read" ), new ReadResponse( bytesRead, buffer ));
        } catch ( final IOException exception ) {
            return Pair.of( new Status( "Failed to read file" ), null);
        }
    }

    public Pair<Status, ServerUserData> getUserData( final String username ) {
        try {
            final UsersSerializableData usersSerializableData = UsersSerializableData.readFromFile( dfsServerUsersDataFilePath );

            final ServerUserData serverUserData = usersSerializableData.getUsersDataList().stream()
                    .filter( data -> data.getUsername().equals( username ) )
                    .findAny()
                    .get();

            return Pair.of( new Status( "Retrieved user data" ), serverUserData);
        } catch ( final IOException exception) {
            return Pair.of( new Status( "Failed to get user data" ), null);
        }
    }

    public Pair<Status, FileHashResponse> getFileHash( final FileHashPayload fileHashPayload ) {
        final ReadFilePayload readFilePayload = new ReadFilePayload( fileHashPayload.getFilePath(), 0, fileHashPayload.getBufferSize(), fileHashPayload.getUsername() );
        final Pair<Status, ReadResponse> readResult = readFile( readFilePayload );

        if( readResult.getKey().getMessage().equals( "Failed to read file" ) )
            return Pair.of( new Status( "Failed to get hash for file"), new FileHashResponse( null, 0 ) );

        //else is "File Read" but it can still be a fail (0 bytes read or less than expected)
        if( readResult.getValue().getBytesRead() < fileHashPayload.getBufferSize() )
            return Pair.of( new Status( "Failed to get hash for file"), new FileHashResponse( null, 0 ) );

        //else we read what we wanted

        final MessageDigest startFileDataHash = hashFileContent( readResult.getValue().getBuffer() );
        if( startFileDataHash != null ) {
//            startFileDataHash.reset();
            final byte[] hashBytes = startFileDataHash.digest();
            return Pair.of( new Status( "Hashed file" ), new FileHashResponse( hashBytes, fileHashPayload.getBufferSize() ) );
        }

        return Pair.of( new Status( "Failed to get hash for file" ), new FileHashResponse( null, 0 ) );
    }

    public Status deleteDirectory( final DeletePayload deletePayload ) {
        final String fileAbsolutePath = dfsServerDataFolderPath + "\\" + deletePayload.getUsername() + "\\" + deletePayload.getDirectoryPath().substring( 4 );
        final File directory = new File( fileAbsolutePath );

        if( !directory.exists() )
            return new Status( "Directory does not exist" );

        try {
            //delete subdirectory metadata entries from UsersData
            final UsersSerializableData usersSerializableData = UsersSerializableData.readFromFile( dfsServerUsersDataFilePath );
            final ServerUserData userData = usersSerializableData.getUsersDataList().stream()
                    .filter( data -> data.getUsername().equals( deletePayload.getUsername() ) )
                    .findAny().get();

            final List<DirectoryMetadata> directoryMetadataToDelete = userData.getDirectoriesMetadata().stream()
                    .filter( directoryMetadata -> directoryMetadata.getDirectoryRelativePath().startsWith( deletePayload.getDirectoryPath() ) )
                    .collect( Collectors.toList() );

            userData.getDirectoriesMetadata().removeAll( directoryMetadataToDelete );
            UsersSerializableData.writeToFile( dfsServerUsersDataFilePath, usersSerializableData );

            deleteDirectoryRecursive( directory ); //even if this fails we already deleted from UsersData so it won't be counted and overwritten if necessary
            return new Status( "Deleted directory" );
        } catch ( final IOException exception ) {
            return new Status( "Failed to delete directory" );
        }
    }

    ////////////////////////////////////////////////////////////////////

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
