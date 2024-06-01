package com.dfs.server.models;

public class ReadResponse {

    private int bytesRead;
    private byte[] buffer;

    public ReadResponse() {

    }

    public ReadResponse(int bytesRead, byte[] buffer) {
        this.bytesRead = bytesRead;
        this.buffer = buffer;
    }

    public int getBytesRead() {
        return bytesRead;
    }

    public void setBytesRead(int bytesRead) {
        this.bytesRead = bytesRead;
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public void setBuffer(byte[] buffer) {
        this.buffer = buffer;
    }
}
