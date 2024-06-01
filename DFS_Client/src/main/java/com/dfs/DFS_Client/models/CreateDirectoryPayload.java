package com.dfs.DFS_Client.models;

public class CreateDirectoryPayload {

    private final String parentFolder;
    private final String directoryName;
    private final DirectoryType type;
    private final String username;

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
}
