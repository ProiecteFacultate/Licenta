package com.dfs.server.services;

import com.dfs.server.models.*;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

@Service
public class AuthenticationService {

    private final String dfsServerUsersDataFilePath = "D:\\Facultate\\Licenta\\HardDisks\\DFS_Server_Disk\\UsersData";

    public Pair<Status, UserData> register(final AuthenticationPayload authenticationPayload ) {
        try {
            final UsersSerializableData usersSerializableData = UsersSerializableData.readFromFile( dfsServerUsersDataFilePath );
            final Optional<UserData> existingUser = usersSerializableData.getUsersDataList().stream()
                    .filter( userData -> userData.getUsername().equals( authenticationPayload.getUsername() ) )
                    .findAny();

            if( existingUser.isEmpty() ) {
                final UserData newUser = new UserData( authenticationPayload.getUsername(), authenticationPayload.getPassword() );
                usersSerializableData.getUsersDataList().add( newUser );
                UsersSerializableData.writeToFile( dfsServerUsersDataFilePath, usersSerializableData );

                return Pair.of( new Status( "User created" ), newUser );
            }
            else
                return Pair.of( new Status( "User already exists" ), null );
        } catch ( final IOException exception ) {
            return Pair.of( new Status( "Failed to register user" ), null );
        }
    }

    public Pair<Status, UserData> login( final AuthenticationPayload authenticationPayload ) {
        try {
            final UsersSerializableData usersSerializableData = UsersSerializableData.readFromFile( dfsServerUsersDataFilePath );
            final Optional<UserData> existingUser = usersSerializableData.getUsersDataList().stream()
                    .filter( userData -> userData.getUsername().equals( authenticationPayload.getUsername() ) )
                    .findAny();

            if( existingUser.isPresent() ) {
                if( existingUser.get().getPassword().equals( authenticationPayload.getPassword() ) )
                    return Pair.of( new Status( "User logged in" ), existingUser.get() );

                return Pair.of( new Status( "Wrong password" ), null );
            }
            else
                return Pair.of( new Status( "User do not exist" ), null );

        } catch ( final IOException exception ) {
            return Pair.of( new Status( "Failed to login user" ), null );
        }
    }
}
