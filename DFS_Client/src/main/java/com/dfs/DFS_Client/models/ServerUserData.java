package com.dfs.DFS_Client.models;

import java.util.List;

public class ServerUserData {

    private String username;
    private String password;
    private List<DirectoryMetadata> directoriesMetadata;

    public ServerUserData(){

    }

    public ServerUserData(String username, String password) {
        this.username = username;
        this.password = password;
        this.directoriesMetadata = List.of();
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }

    public List<DirectoryMetadata> getDirectoriesMetadata() {
        return directoriesMetadata;
    }

    public void setDirectoriesMetadata(List<DirectoryMetadata> directoriesMetadata) {
        this.directoriesMetadata = directoriesMetadata;
    }

    public void copy( final ServerUserData serverUserData) {
        this.username = serverUserData.username;
        this.password = serverUserData.password;
        this.directoriesMetadata = serverUserData.directoriesMetadata;
    }
}
