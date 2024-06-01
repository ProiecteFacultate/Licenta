package com.dfs.DFS_Client.models;

public class FileHashResponse {

    private byte[] hashValueBytes; //this is not the read text as bytes; we read the text, hash it, and then get the bytes of the hash, so the size of this byte[] is not big
    private int bufferSize; //the size of the buffer that was hashed

    public FileHashResponse() {

    }

    public FileHashResponse(byte[] hashValueBytes, int bufferSize) {
        this.hashValueBytes = hashValueBytes;
        this.bufferSize = bufferSize;
    }

    public byte[] getHashValue() {
        return hashValueBytes;
    }

    public void setHashValue(byte[] hashValueBytes) {
        this.hashValueBytes = hashValueBytes;
    }

    public int getBufferSize() {
        return bufferSize;
    }

    public void setBufferSize(int bufferSize) {
        this.bufferSize = bufferSize;
    }
}
