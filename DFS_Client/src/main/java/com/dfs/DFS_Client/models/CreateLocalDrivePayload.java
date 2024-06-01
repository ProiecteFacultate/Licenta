package com.dfs.DFS_Client.models;

public class CreateLocalDrivePayload {

    private final String localFSPath;
    private final int localFSMaximumSize;
    private final int localFSMaximumFileSize;
    private final String username;

    public CreateLocalDrivePayload( final String localFSPath, final int localFSMaximumSize, final int localFSMaximumFileSize, final String username ) {
        this.username = username;
        this.localFSPath = localFSPath;
        this.localFSMaximumSize = localFSMaximumSize;
        this.localFSMaximumFileSize = localFSMaximumFileSize;
    }

    public String getUsername() {
        return username;
    }

    public String getLocalFSPath() {
        return localFSPath;
    }

    public int getLocalFSMaximumSize() {
        return localFSMaximumSize;
    }

    public int getLocalFSMaximumFileSize() {
        return localFSMaximumFileSize;
    }
}
