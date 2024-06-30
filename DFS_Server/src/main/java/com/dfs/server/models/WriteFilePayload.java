package com.dfs.server.models;

public class WriteFilePayload {

    private String filePath; //with Root
    private int bytesToWrite;
    private byte[] fileData;
    private String username;

    public WriteFilePayload() {

    }

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

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    public void setBytesToWrite(int bytesToWrite) {
        this.bytesToWrite = bytesToWrite;
    }

    public void setFileData(byte[] fileData) {
        this.fileData = fileData;
    }

    public void setUsername(String username) {
        this.username = username;
    }
}
