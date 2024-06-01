package com.dfs.server.services;

import java.io.File;

public class InitializationService {

    private final String dfsServerDiskPath= "D:\\Facultate\\Licenta\\HardDisks\\DFS_Server_Disk";

    public void initialize() throws Exception {
        initializeUsersDataFile();
        initializeDataFolder();
    }

    private void initializeUsersDataFile() throws Exception {
        final String dfsServerUsersDataFilePath = dfsServerDiskPath + "\\UsersData";
        final File file = new File( dfsServerUsersDataFilePath );

        UsersSerializableData usersSerializableData = new UsersSerializableData();

        if( !file.exists() ) {
            final boolean fileCreated = file.createNewFile();

            if( fileCreated )
                UsersSerializableData.writeToFile( dfsServerUsersDataFilePath, usersSerializableData ); //this throws if write fails
            else
                throw new Exception( "File " + dfsServerUsersDataFilePath + " failed to create" );

            System.out.println( "File " + dfsServerUsersDataFilePath + " created" );
        }
        else
            System.out.println( "File " + dfsServerUsersDataFilePath + " already exists" );
    }

    private void initializeDataFolder() throws Exception {
        final String dfsServerDataFolderPath = dfsServerDiskPath + "\\Data";
        final File folder = new File( dfsServerDataFolderPath );

        if( !folder.exists() ) {
            final boolean folderCreated = folder.mkdir();

            if( !folderCreated )
                throw new Exception("Folder " + dfsServerDataFolderPath + " failed to create");

            System.out.println( "File " + dfsServerDataFolderPath + " created" );
        }
        else
            System.out.println( "Folder " + dfsServerDataFolderPath + " already exists" );
    }
}
