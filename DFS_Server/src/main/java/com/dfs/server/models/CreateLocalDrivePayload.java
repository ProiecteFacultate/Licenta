package com.dfs.server.models;

public class CreateLocalDrivePayload {

    private String localFSPath;
    private int localFSMaximumSize;
    private int localFSMaximumFileSize;
    private String username;

    public CreateLocalDrivePayload() {

    }


    public CreateLocalDrivePayload(final String localFSPath, final int localFSMaximumSize, final int localFSMaximumFileSize, final String username ) {
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

    public void setLocalFSPath(String localFSPath) {
        this.localFSPath = localFSPath;
    }

    public void setLocalFSMaximumSize(int localFSMaximumSize) {
        this.localFSMaximumSize = localFSMaximumSize;
    }

    public void setLocalFSMaximumFileSize(int localFSMaximumFileSize) {
        this.localFSMaximumFileSize = localFSMaximumFileSize;
    }

    public void setUsername(String username) {
        this.username = username;
    }
}
