package com.dfs.DFS_Client.models;

public class FileHashPayload {

    private String filePath;
    private int bufferSize; //the size of the buffer to be hashed (hash always starts from byte 0 of the file)
    private String username;

    public FileHashPayload() {

    }

    public FileHashPayload(String filePath, int bufferSize, String username) {
        this.filePath = filePath;
        this.bufferSize = bufferSize;
        this.username = username;
    }

    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    public int getBufferSize() {
        return bufferSize;
    }

    public void setBufferSize(int bufferSize) {
        this.bufferSize = bufferSize;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }
}
