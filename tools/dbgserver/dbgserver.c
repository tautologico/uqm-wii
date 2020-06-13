#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// 
// A simple server that displays messages sent to it, meant
// for debugging the Wii version
//

int main(int argc, char *argv[]) {
	// port to start the server on
	int SERVER_PORT = 8877;

	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htonl: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	// create a TCP socket, creation returns -1 on failure
	int listen_sock;
	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Could not create listen socket\n");
		return 1;
	}

	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(listen_sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		fprintf(stderr, "Could not bind socket\n");
		return 1;
	}

	int wait_size = 16;  // maximum number of waiting clients, after which
	                     // dropping begins
	if (listen(listen_sock, wait_size) < 0) {
		fprintf(stderr, "Could not open socket for listening\n");
		return 1;
	}

	printf("=== Listening on port %d\n", SERVER_PORT);

	// socket address used to store client address
	struct sockaddr_in client_address;
	int client_address_len = sizeof(client_address);

	// log file
	char log_file_name[256];
	FILE *log;

	// main server loop
	for (;;) {
		// open a new socket to transmit data per connection
		int sock;
		if ((sock = accept(listen_sock, (struct sockaddr *)&client_address,
		                   &client_address_len)) < 0) {
			fprintf(stderr, "Could not open a socket to accept data\n");
			return 1;
		}

		sprintf(log_file_name, "dbg%ld.log", time(NULL));
		log = fopen(log_file_name, "w");
		if (log == NULL) {
		    fprintf(stderr, "could not open file %s\n", log_file_name);
		    return -1;
	    }

		int n = 0;
		int len = 0, maxlen = 1200;
		char buffer[maxlen];

		printf("=== Client connected with ip address: %s\n", inet_ntoa(client_address.sin_addr));
		fprintf(log, "=== Client connected with ip address: %s\n", inet_ntoa(client_address.sin_addr));

		// keep running as long as the client keeps the connection open
		while ((n = recv(sock, buffer, maxlen, 0)) > 0) {
			buffer[n] = '\0';

			printf("=> %s", buffer);
			fprintf(log, "%s", buffer);
		}

		close(sock);
		fclose(log);

		printf("### Connection closed, return to listening...\n");
	}

	fclose(log);
	close(listen_sock);
	return 0;
}
