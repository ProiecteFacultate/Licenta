package com.dfs.DFS_Client.clients;

import com.dfs.DFS_Client.models.CreateDirectoryPayload;
import com.dfs.DFS_Client.models.CreateLocalDrivePayload;
import com.dfs.DFS_Client.models.Status;
import org.springframework.http.*;
import org.springframework.web.client.RestTemplate;

import java.util.Collections;

public class DriveClient {

    private final String dfsServerUrl = "http://localhost:6789";

    private RestTemplate restTemplate;

    public DriveClient() {
        this.restTemplate = new RestTemplate();
    }

    public Status createLocalDrive( final String localDrivePath, final int localDriveMaximumSize, final int localDriveMaximumFileSize, final String username ) {
        final CreateLocalDrivePayload createLocalDrivePayload = new CreateLocalDrivePayload( localDrivePath, localDriveMaximumSize, localDriveMaximumFileSize, username );

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<CreateLocalDrivePayload> requestEntity = new HttpEntity<>( createLocalDrivePayload, headers );

        final String url = String.format( "%s/drive/local/create", dfsServerUrl );

        try {
            final ResponseEntity<Status> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, Status.class
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return new Status( "ERROR" );
        }
    }
}
