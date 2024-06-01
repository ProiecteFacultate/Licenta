package com.dfs.server;

import com.dfs.server.services.InitializationService;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

import java.io.IOException;

@SpringBootApplication
public class DfsServerApplication {

	public static void main(String[] args) throws Exception {
		SpringApplication.run(DfsServerApplication.class, args);

		new InitializationService().initialize();
	}

}
