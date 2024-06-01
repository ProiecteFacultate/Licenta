package com.dfs.server.controllers;

import com.dfs.server.models.*;
import com.dfs.server.services.DirectoryService;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping( value = "/directory" )
public class DirectoryController {

    private final DirectoryService directoryService;

    public DirectoryController() {
        this.directoryService = new DirectoryService();
    }

    @PostMapping( value = "/create", produces = MediaType.APPLICATION_JSON_VALUE )
    @ResponseStatus( HttpStatus.OK )
    public Status createDirectory( @RequestBody CreateDirectoryPayload createDirectoryPayload ) {
        return directoryService.createDirectory( createDirectoryPayload );
    }

    @PostMapping( value = "/file/write", produces = MediaType.APPLICATION_JSON_VALUE )
    @ResponseStatus( HttpStatus.OK )
    public Status writeFile( @RequestBody WriteFilePayload writeFilePayload ) {
        return directoryService.writeFile( writeFilePayload );
    }

    @PostMapping( value = "/file/read", produces = MediaType.APPLICATION_JSON_VALUE )
    @ResponseStatus( HttpStatus.OK )
    public Pair<Status, ReadResponse> writeFile(@RequestBody ReadFilePayload readFilePayload) {
        return directoryService.readFile( readFilePayload );
    }
}
