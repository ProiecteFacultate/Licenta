package com.dfs.DFS_Client.clients;

import com.dfs.DFS_Client.models.AuthenticationPayload;
import com.dfs.DFS_Client.models.Pair;
import com.dfs.DFS_Client.models.Status;
import com.dfs.DFS_Client.models.UserData;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.http.*;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestTemplate;

import java.util.Collections;

public class AuthenticationClient {

    private final String dfsServerUrl = "http://localhost:6789";

    private RestTemplate restTemplate;

    public AuthenticationClient() {
        this.restTemplate = new RestTemplate();
    }

    public Pair<Status, UserData> register(final String username, final String password ) {
        final AuthenticationPayload authenticationPayload = new AuthenticationPayload( username, password );

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<AuthenticationPayload> requestEntity = new HttpEntity<>( authenticationPayload, headers );

        final String url = String.format( "%s/authentication/register", dfsServerUrl );

        try {
            final ResponseEntity<Pair<Status, UserData>> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, new ParameterizedTypeReference<Pair<Status, UserData>>() {}
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return Pair.of( new Status( "ERROR" ), null );
        }
    }

    public Pair<Status, UserData> login( final String username, final String password ) {
        final AuthenticationPayload authenticationPayload = new AuthenticationPayload( username, password );

        final HttpHeaders headers = new HttpHeaders();
        headers.setContentType( MediaType.APPLICATION_JSON );
        headers.setAccept( Collections.singletonList( MediaType.APPLICATION_JSON ) );
        final HttpEntity<AuthenticationPayload> requestEntity = new HttpEntity<>( authenticationPayload, headers );

        final String url = String.format( "%s/authentication/login", dfsServerUrl );

        try {
            final ResponseEntity<Pair<Status, UserData>> response = restTemplate.exchange(
                    url, HttpMethod.POST, requestEntity, new ParameterizedTypeReference<Pair<Status, UserData>>() {}
            );

            return response.getBody();

        } catch ( final Exception e ) {
            return Pair.of( new Status( "ERROR" ), null );
        }
    }
}
