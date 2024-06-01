package com.dfs.server.controllers;

import com.dfs.server.models.*;
import com.dfs.server.services.DirectoryService;
import com.dfs.server.services.DriveService;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping( value = "/drive" )
public class DriveController {

    private final DriveService driveService;
    private final DirectoryService directoryService;

    public DriveController() {
        this.driveService = new DriveService();
        this.directoryService = new DirectoryService();
    }

    @PostMapping( value = "/userData/get/{username}", produces = MediaType.APPLICATION_JSON_VALUE )
    @ResponseStatus( HttpStatus.OK )
    public Pair<Status, ServerUserData> getUserData(@PathVariable final String username ) {
        return directoryService.getUserData( username );
    }
}
