package com.dfs.server.models;

public class DeletePayload {

    private String directoryPath;
    private String username;

    public DeletePayload() {

    }

    public DeletePayload(String directoryPath, String username) {
        this.directoryPath = directoryPath;
        this.username = username;
    }

    public String getDirectoryPath() {
        return directoryPath;
    }

    public void setDirectoryPath(String directoryPath) {
        this.directoryPath = directoryPath;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }
}
