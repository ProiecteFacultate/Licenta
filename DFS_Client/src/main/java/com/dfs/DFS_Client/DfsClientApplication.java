package com.dfs.DFS_Client;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class DfsClientApplication {

	public static void main(String[] args) {
		SpringApplication.run(DfsClientApplication.class, args);

		final Interface inter = new Interface();

		try {
			inter.run();
		} catch ( final Exception e ) {
			System.out.println( e.getMessage() );
		}
	}
}
