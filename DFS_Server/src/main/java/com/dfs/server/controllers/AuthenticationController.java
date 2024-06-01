package com.dfs.server.controllers;

import com.dfs.server.models.AuthenticationPayload;
import com.dfs.server.models.Pair;
import com.dfs.server.models.Status;
import com.dfs.server.models.ServerUserData;
import com.dfs.server.services.AuthenticationService;
import com.dfs.server.services.DirectoryService;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping( value = "/authentication", produces = MediaType.APPLICATION_JSON_VALUE )
public class AuthenticationController {

    final AuthenticationService authenticationService;
    final DirectoryService directoryService;

    public AuthenticationController( final AuthenticationService authenticationService, final DirectoryService directoryService ) {
        this.authenticationService = authenticationService;
        this.directoryService = directoryService;
    }

    @PostMapping( value = "/register" )
    @ResponseStatus( HttpStatus.OK )
    public Pair<Status, ServerUserData> registerUser(@RequestBody AuthenticationPayload authenticationPayload) {
        final Pair<Status, ServerUserData> registerResponse = authenticationService.register( authenticationPayload );

        if( registerResponse.getKey().getMessage().equals( "User created" ) ) {
            final Status createUserFolderStatus = directoryService.createUserFolder( authenticationPayload.getUsername() );

            if( createUserFolderStatus.getMessage().equals( "Failed to create user folder" ) )
                return Pair.of( new Status( "Failed to register user" ), registerResponse.getValue() );
        }

        return registerResponse;
    }

    @PostMapping( value = "/login" )
    @ResponseStatus( HttpStatus.OK )
    public Pair<Status, ServerUserData> loginUser(@RequestBody AuthenticationPayload authenticationPayload) {
        return authenticationService.login( authenticationPayload );
    }
}
