package com.dfs.server.models;

public class CreateDirectoryPayload {

    private String parentFolder;
    private String directoryName;
    private DirectoryType type;
    private String username;

    public CreateDirectoryPayload() {

    }

    public CreateDirectoryPayload(String parentFolder, String directoryName, DirectoryType type, String username) {
        this.parentFolder = parentFolder;
        this.directoryName = directoryName;
        this.type = type;
        this.username = username;
    }

    public String getParentFolder() {
        return parentFolder;
    }

    public String getDirectoryName() {
        return directoryName;
    }

    public DirectoryType getType() {
        return type;
    }

    public String getUsername() {
        return username;
    }

    public void setParentFolder(String parentFolder) {
        this.parentFolder = parentFolder;
    }

    public void setDirectoryName(String directoryName) {
        this.directoryName = directoryName;
    }

    public void setType(DirectoryType type) {
        this.type = type;
    }

    public void setUsername(String username) {
        this.username = username;
    }
}
