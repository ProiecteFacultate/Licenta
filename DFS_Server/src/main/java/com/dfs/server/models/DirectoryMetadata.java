package com.dfs.server.models;

import java.util.Objects;

public class DirectoryMetadata {

    private String directoryRelativePath; //with Root
    private DirectoryType type;
    private int fileLength; //0 if folder

    public DirectoryMetadata() {

    }

    public DirectoryMetadata(String directoryRelativePath, DirectoryType type, int fileLength) {
        this.directoryRelativePath = directoryRelativePath;
        this.type = type;
        this.fileLength = fileLength;
    }

    public String getDirectoryRelativePath() {
        return directoryRelativePath;
    }

    public void setDirectoryRelativePath(String directoryRelativePath) {
        this.directoryRelativePath = directoryRelativePath;
    }

    public DirectoryType getType() {
        return type;
    }

    public void setType(DirectoryType type) {
        this.type = type;
    }

    public int getFileLength() {
        return fileLength;
    }

    public void setFileLength(int fileLength) {
        this.fileLength = fileLength;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        DirectoryMetadata that = (DirectoryMetadata) o;
        return fileLength == that.fileLength && Objects.equals(directoryRelativePath, that.directoryRelativePath) && type == that.type;
    }

    @Override
    public int hashCode() {
        return Objects.hash(directoryRelativePath, type, fileLength);
    }
}
