package com.dfs.DFS_Client;

import com.dfs.DFS_Client.clients.AuthenticationClient;
import com.dfs.DFS_Client.models.Pair;
import com.dfs.DFS_Client.models.Status;
import com.dfs.DFS_Client.models.UserData;
import com.dfs.DFS_Client.services.DirectoryService;
import com.dfs.DFS_Client.services.DriveService;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;

public class Interface {

    final AuthenticationClient authenticationClient;
    final DirectoryService directoryService;
    final DriveService driveService;

    public Interface() {
        this.authenticationClient = new AuthenticationClient();
        this.directoryService = new DirectoryService();
        this.driveService = new DriveService();
    }

    public void run() throws Exception {
        final UserData userData = authenticate();
        System.out.println( "\n ---- You are now logged in as '" + userData.getUsername() + "' ----" );

        Scanner scanner = new Scanner(System.in);

        while( true ) {
            final String command = scanner.nextLine();
            final List<String> commandTokens = Arrays.asList( command.split(" ") );

            if( commandTokens.get( 0 ).equals( "mkdrive") )
                driveService.createLocalDrive( commandTokens, userData );
            else if( commandTokens.get( 0 ).equals( "mkdir") )
                directoryService.createDirectory( commandTokens, userData );
            else if( commandTokens.get( 0 ).equals( "write") )
                directoryService.writeFile( commandTokens, userData );
            else if( commandTokens.get( 0 ).equals( "read") )
                directoryService.readFile( commandTokens, userData );
            else if( commandTokens.get( 0 ).equals( "syncdrive") )
                driveService.syncDrive( userData );
            else if( commandTokens.get( 0 ).equals( "exit" ) ) {
                scanner.close();
                System.out.println( "Session ended!" );
                return;
            }
            else
                System.out.println( "Unknown command" );
        }
    }

    private UserData authenticate() throws Exception {
        Scanner scanner = new Scanner(System.in);

        while ( true ) {
            System.out.println( "\nDo you want to Register or Login?" );
            final String authenticationCommand = scanner.nextLine();
            String username = "", password = "";

            if( authenticationCommand.equals( "Login") || authenticationCommand.equals( "Register") ) {
                System.out.print( "Enter Username: " );
                username = scanner.nextLine();

                System.out.print( "Enter password: " );
                password = scanner.nextLine();
            }

            if( authenticationCommand.equals( "Login") ) {
                final Pair<Status, UserData> loginResponse = authenticationClient.login( username, password );

                switch ( loginResponse.getKey().getMessage() ) {
                    case "User logged in":
                        System.out.println( "Log in successful" );
                        return loginResponse.getValue();
                    case "User do not exist":
                        System.out.println( "User do not exist" );
                        break;
                    case "Wrong password":
                        System.out.println( "Wrong password" );
                        break;
                    default: //"Failed to login user" from server or "ERROR" from register method
                        System.out.println( "Log in failed" );
                }

            } else if( authenticationCommand.equals( "Register") ) {
                final Pair<Status, UserData> registerResponse = authenticationClient.register( username, password );

                switch ( registerResponse.getKey().getMessage() ) {
                    case "User created":
                        System.out.println( "Register successful" );
                        return registerResponse.getValue();
                    case "User already exists":
                        System.out.println( "User already exists" );
                        break;
                    default: //"Failed to register user" from server or "ERROR" from register method
                        System.out.println( "Registration failed" );
                }
            } else {
                System.out.println( "Unrecognized command! [Register/Login]" );
            }
        }
    }
}
