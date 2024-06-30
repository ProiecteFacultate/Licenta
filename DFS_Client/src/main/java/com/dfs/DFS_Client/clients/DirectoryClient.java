package com.dfs.DFS_Client.clients;

import com.dfs.DFS_Client.models.*;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.http.*;
import org.springframework.web.client.RestTemplate;

import java.io.File;
import java.util.Collections;

public class DirectoryClient {

    private final String dfsServerUrl = "http://localhost:8080";

    private RestTemplate restTemplate;

    public DirectoryClient() {
        this.restTemplate = new RestTemplate();
    }

    public Status createDirectory( final String parentFolder, final String directoryName, final DirectoryType type, final String username ) {
        final CreateDirectoryPayload createDirectoryPayload = new CreateDirectoryPayload( parentFolder, directoryName, type, username);

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<CreateDirectoryPayload> requestEntity = new HttpEntity<>( createDirectoryPayload, headers );

        final String url = String.format( "%s/directory/create", dfsServerUrl );

        try {
            final ResponseEntity<Status> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, Status.class
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return new Status( "ERROR" );
        }
    }

    public Status writeFile( final String filePath, final byte[] fileData, final int bytesToWrite, final String username ) {
        final WriteFilePayload writeFilePayload = new WriteFilePayload( filePath, bytesToWrite, fileData, username );

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<WriteFilePayload> requestEntity = new HttpEntity<>( writeFilePayload, headers );

        final String url = String.format( "%s/directory/file/write", dfsServerUrl );

        try {
            final ResponseEntity<Status> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, Status.class
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return new Status( "ERROR" );
        }
    }

    public Pair<Status, ReadResponse> readFile( final String filePath, final int startPosition, final int bytesToRead, final String username ) {
        final ReadFilePayload readFilePayload = new ReadFilePayload( filePath, startPosition, bytesToRead, username );

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<ReadFilePayload> requestEntity = new HttpEntity<>( readFilePayload, headers );

        final String url = String.format( "%s/directory/file/read", dfsServerUrl );

        try {
            final ResponseEntity<Pair<Status, ReadResponse>> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, new ParameterizedTypeReference<Pair<Status, ReadResponse>>() {}
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return Pair.of( new Status( "ERROR" ), null);
        }
    }

    public Pair<Status, ServerUserData> getServerUserData(final String username ) {
        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity requestEntity = new HttpEntity<>( headers );

        final String url = String.format( "%s/drive/userData/get/%s", dfsServerUrl, username );

        try {
            final ResponseEntity<Pair<Status, ServerUserData>> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, new ParameterizedTypeReference<Pair<Status, ServerUserData>>() {}
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return Pair.of( new Status( "ERROR" ), null);
        }
    }

    public Pair<Status, FileHashResponse> getFileContentHash( final String filePath, final int bytesToHash, final String username ) {
        final FileHashPayload fileHashPayload = new FileHashPayload( filePath, bytesToHash, username );

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<FileHashPayload> requestEntity = new HttpEntity<>( fileHashPayload, headers );

        final String url = String.format( "%s/directory/file/hash/get", dfsServerUrl );

        try {
            final ResponseEntity<Pair<Status, FileHashResponse>> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, new ParameterizedTypeReference<Pair<Status, FileHashResponse>>() {}
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return Pair.of( new Status( "ERROR" ), null);
        }
    }

    public Status deleteDirectory( final String directoryPath, final String username ) {
        final DeletePayload deletePayload = new DeletePayload( directoryPath, username);

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<DeletePayload> requestEntity = new HttpEntity<>( deletePayload, headers );

        final String url = String.format( "%s/directory/delete", dfsServerUrl );

        try {
            final ResponseEntity<Status> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, new ParameterizedTypeReference<Status>() {}
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return new Status( "ERROR" );
        }
    }
}
