package com.dfs.DFS_Client.models;

import org.apache.tomcat.jni.Local;

import java.util.ArrayList;
import java.util.List;

public class LocalUserData {

    private String username;
    private String localFSPath;
    private int localFSMaximumSize;
    private int localFSMaximumFileSize;
    private List<DirectoryMetadata> directoriesMetadata;

    public LocalUserData(){
        this.localFSPath = "";
        this.localFSMaximumSize = -1;
        this.localFSMaximumFileSize = -1;
        this.directoriesMetadata = new ArrayList<>();
    }

    public LocalUserData( String username ) {
        this.username = username;
        this.localFSPath = "";
        this.localFSMaximumSize = -1;
        this.localFSMaximumFileSize = -1;
        this.directoriesMetadata = new ArrayList<>();
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

    public void setUsername(String username) {
        this.username = username;
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

    public static LocalUserData copyOf( LocalUserData localUserData ){
        LocalUserData localUserDataCopy = new LocalUserData();
        localUserDataCopy.localFSPath = localUserData.localFSPath;
        localUserDataCopy.localFSMaximumSize = localUserData.localFSMaximumSize;
        localUserDataCopy.localFSMaximumFileSize = localUserData.localFSMaximumFileSize;
        localUserDataCopy.directoriesMetadata = localUserData.directoriesMetadata;
        return localUserDataCopy;
    }
}
