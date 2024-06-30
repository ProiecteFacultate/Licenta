package com.dfs.DFS_Client.clients;

import com.dfs.DFS_Client.models.CreateDirectoryPayload;
import com.dfs.DFS_Client.models.CreateLocalDrivePayload;
import com.dfs.DFS_Client.models.Status;
import org.springframework.http.*;
import org.springframework.web.client.RestTemplate;

import java.util.Collections;

public class DriveClient {

    private final String dfsServerUrl = "http://localhost:8080";

    private RestTemplate restTemplate;

    public DriveClient() {
        this.restTemplate = new RestTemplate();
    }
}
