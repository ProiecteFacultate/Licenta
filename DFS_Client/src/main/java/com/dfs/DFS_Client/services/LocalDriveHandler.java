package com.dfs.DFS_Client.services;

import com.dfs.DFS_Client.models.DirectoryMetadata;
import com.dfs.DFS_Client.models.LocalUserData;
import com.google.gson.Gson;

import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Optional;

public class LocalDriveHandler {

    public static LocalUserData readMetadataFile( final String localDrivePath ) throws IOException {
        final String metadataFilePath = localDrivePath + "\\Metadata";
        FileReader reader = new FileReader( metadataFilePath );
        Gson gson = new Gson();
        LocalUserData localUserData = gson.fromJson( reader, LocalUserData.class );
        reader.close();

        return localUserData;
    }

    public static void writeToMetadataFile( final String localDrivePath, final LocalUserData localUserData ) throws IOException {
        final String metadataFilePath = localDrivePath + "\\Metadata";
        Gson gson = new Gson();
        String json = gson.toJson( localUserData );

        FileWriter writer = new FileWriter( metadataFilePath );
        writer.write( json );
        writer.close();
    }

    public static void addDirectoryToListForUser( final String localDrivePath, final DirectoryMetadata newDirectory ) throws IOException {
        final LocalUserData localUserData = readMetadataFile( localDrivePath );
        localUserData.getDirectoriesMetadata().add( newDirectory );
        writeToMetadataFile( localDrivePath, localUserData );
    }

    public static void updateFileSize( final String localDrivePath, final String fileRelativePath, final int newSize ) throws IOException {
        final LocalUserData localUserData = readMetadataFile( localDrivePath );

        final DirectoryMetadata directoryMetadata = localUserData.getDirectoriesMetadata().stream()
                .filter( directory -> directory.getDirectoryRelativePath().equals( fileRelativePath ) )
                .findAny()
                .get();

        directoryMetadata.setFileLength( newSize );
        writeToMetadataFile( localDrivePath, localUserData );
    }

    public static Optional<DirectoryMetadata> getDirectoryMetadataForFile(final String localDrivePath, final String fileRelativePath ) throws IOException {
        final LocalUserData localUserData = readMetadataFile( localDrivePath );

        return localUserData.getDirectoriesMetadata().stream()
                .filter( directory -> directory.getDirectoryRelativePath().equals( fileRelativePath ) )
                .findAny();
    }
}
