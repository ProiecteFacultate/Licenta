package com.dfs.DFS_Client;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.web.embedded.tomcat.TomcatServletWebServerFactory;
import org.springframework.boot.web.servlet.server.ServletWebServerFactory;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@SpringBootApplication
@Configuration
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

	@Bean
	public ServletWebServerFactory servletWebServerFactory() {
		final TomcatServletWebServerFactory tomcat = new TomcatServletWebServerFactory();
		tomcat.setPort(0);
		return tomcat;
	}
}
