package com.dfs.DFS_Client.models;

public class ReadFilePayload {

    private String filePath; //with Root
    private int startPosition;
    private int bytesToRead;
    private String username;

    public ReadFilePayload() {

    }

    public ReadFilePayload(String filePath, int startPosition, int bytesToRead, String username) {
        this.filePath = filePath;
        this.startPosition = startPosition;
        this.bytesToRead = bytesToRead;
        this.username = username;
    }

    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    public int getStartPosition() {
        return startPosition;
    }

    public void setStartPosition(int startPosition) {
        this.startPosition = startPosition;
    }

    public int getBytesToRead() {
        return bytesToRead;
    }

    public void setBytesToRead(int bytesToRead) {
        this.bytesToRead = bytesToRead;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }
}
