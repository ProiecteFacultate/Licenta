package com.dfs.server.models;

import java.util.List;

public class UserData {

    private String username;
    private String password;
    private String localFSPath;
    private int localFSMaximumSize;
    private int localFSMaximumFileSize;
    private List<DirectoryMetadata> directoriesMetadata;

    public UserData(){

    }

    public UserData(String username, String password) {
        this.username = username;
        this.password = password;
        this.localFSPath = "";
        this.localFSMaximumSize = -1;
        this.localFSMaximumFileSize = -1;
        this.directoriesMetadata = List.of();
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
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

    public List<DirectoryMetadata> getDirectoriesMetadata() {
        return directoriesMetadata;
    }

    public void setDirectoriesMetadata(List<DirectoryMetadata> directoriesMetadata) {
        this.directoriesMetadata = directoriesMetadata;
    }

    public void copy( final UserData userData ) {
        this.username = userData.username;
        this.password = userData.password;
        this.localFSPath = userData.localFSPath;
        this.localFSMaximumSize = userData.localFSMaximumSize;
        this.localFSMaximumFileSize = userData.localFSMaximumFileSize;
        this.directoriesMetadata = userData.directoriesMetadata;
    }
}
