package com.dfs.server.services;

import com.dfs.server.models.CreateLocalDrivePayload;
import com.dfs.server.models.Status;
import com.dfs.server.models.UserData;
import com.dfs.server.models.UsersSerializableData;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.util.Optional;

@Service
public class DriveService {

    private final String dfsServerUsersDataFilePath = "D:\\Facultate\\Licenta\\HardDisks\\DFS_Server_Disk\\UsersData";
    private final String dfsServerDataFolderPath = "D:\\Facultate\\Licenta\\HardDisks\\DFS_Server_Disk\\Data";

    public Status createLocalDrive(final CreateLocalDrivePayload createLocalDrivePayload ) {
        try {
            final UsersSerializableData usersSerializableData = UsersSerializableData.readFromFile( dfsServerUsersDataFilePath );
            final UserData userData = usersSerializableData.getUsersDataList().stream()
                    .filter( user -> user.getUsername().equals( createLocalDrivePayload.getUsername() ) )
                    .findAny()
                    .get();

            userData.setLocalFSPath( createLocalDrivePayload.getLocalFSPath() );
            userData.setLocalFSMaximumSize( createLocalDrivePayload.getLocalFSMaximumSize() );
            userData.setLocalFSMaximumFileSize( createLocalDrivePayload.getLocalFSMaximumFileSize() );

            UsersSerializableData.writeToFile( dfsServerUsersDataFilePath, usersSerializableData );

            return new Status( "Successfully created local drive" );
        } catch ( final IOException exception ) {
            return new Status( "Failed to create local drive" );
        }
    }
}
