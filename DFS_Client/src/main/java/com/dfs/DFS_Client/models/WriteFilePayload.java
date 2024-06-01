package com.dfs.DFS_Client.models;

public class WriteFilePayload {

    private final String filePath; //with Root
    private final int bytesToWrite;
    private final byte[] fileData;
    private final String username;

    public WriteFilePayload( final String filePath, final int bytesToWrite, final byte[] fileData, final String username ) {
        this.filePath = filePath;
        this.bytesToWrite = bytesToWrite;
        this.fileData = fileData;
        this.username = username;
    }

    public String getFilePath() {
        return filePath;
    }

    public byte[] getFileData() {
        return fileData;
    }

    public int getBytesToWrite() {
        return bytesToWrite;
    }

    public String getUsername() {
        return username;
    }
}
