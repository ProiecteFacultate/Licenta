package com.dfs.DFS_Client.models;

public class AuthenticationPayload {

    private final String username;
    private final String password;

    public AuthenticationPayload(String username, String password ) {
        this.username = username;
        this.password = password;
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }
}
