package com.dfs.server.models;

public class AuthenticationPayload {

    private String username;
    private String password;

    public AuthenticationPayload() {

    }

    public AuthenticationPayload(String username, String password) {
        this.username = username;
        this.password = password;
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public void setPassword(String password) {
        this.password = password;
    }
}
