package com.dfs.server.services;

import com.dfs.server.models.DirectoryMetadata;
import com.dfs.server.models.ServerUserData;
import com.google.gson.Gson;

import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;
import java.util.List;

public class UsersSerializableData implements Serializable {

    private final List<ServerUserData> usersDataList;

    public UsersSerializableData() {
        this.usersDataList = List.of();
    }

    public List<ServerUserData> getUsersDataList() {
        return usersDataList;
    }

    public static UsersSerializableData readFromFile( final String filePath ) throws IOException {
        FileReader reader = new FileReader( filePath );
        Gson gson = new Gson();
        UsersSerializableData usersSerializableData = gson.fromJson( reader, UsersSerializableData.class );
        reader.close();

        return usersSerializableData;
    }

    public static void writeToFile( final String filePath, final UsersSerializableData usersSerializableData ) throws IOException {
        Gson gson = new Gson();
        String json = gson.toJson( usersSerializableData );

        FileWriter writer = new FileWriter( filePath );
        writer.write( json );
        writer.close();
    }

    public static void updateFileSize( final String dfsServerUsersDataFilePath, final String fileRelativePath, final int newSize, final String username ) throws IOException {
        final UsersSerializableData usersSerializableData = readFromFile( dfsServerUsersDataFilePath );
        final ServerUserData serverUserData = usersSerializableData.getUsersDataList().stream()
                .filter( user -> user.getUsername().equals( username ) )
                .findAny()
                .get();

        final DirectoryMetadata directoryMetadata = serverUserData.getDirectoriesMetadata().stream()
                        .filter( directory -> directory.getDirectoryRelativePath().equals( fileRelativePath ) )
                        .findAny()
                        .get();

        directoryMetadata.setFileLength( newSize );
        UsersSerializableData.writeToFile( dfsServerUsersDataFilePath, usersSerializableData );
    }

    public static void addDirectoryToListForUser( final String dfsServerUsersDataFilePath, final DirectoryMetadata newDirectory, final String username ) throws IOException {
        final UsersSerializableData usersSerializableData = readFromFile( dfsServerUsersDataFilePath );
        final ServerUserData serverUserData = usersSerializableData.getUsersDataList().stream()
                .filter( user -> user.getUsername().equals( username ) )
                .findAny()
                .get();

        serverUserData.getDirectoriesMetadata().add( newDirectory );
        UsersSerializableData.writeToFile( dfsServerUsersDataFilePath, usersSerializableData );
    }
}
