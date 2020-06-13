#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
	const char* server_name = "localhost";
	const int server_port = 8877;

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// creates binary representation of server name
	// and stores it as sin_addr
	// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
	inet_pton(AF_INET, server_name, &server_address.sin_addr);

	// htons: port in network order format
	server_address.sin_port = htons(server_port);

	// open a stream socket
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "could not create socket\n");
		return 1;
	}

	// TCP is connection oriented, a reliable connection
	// **must** be established before any data is exchanged
	if (connect(sock, (struct sockaddr*)&server_address,
	            sizeof(server_address)) < 0) {
		fprintf(stderr, "could not connect to server\n");
		return 1;
	}

	// send

	// data that will be sent to the server
	const char* data_to_send[] = 
       { "Lorem ipsum dolor sit amet consectueris amaunensis foederis\n",
         "ipsos literis quo vadis latensis est voegel vanitatis\n",
         "al khwarizmi al jabr al alamiah al dente amaterasu humma kavullah\n",
         "Be yourself; everyone else is already taken.\n",
         "No man has a good enough memory to be a successful liar.\n",
         "It was the best of times, it was the worst of times, it was the age of wisdom,\n",
         "it was the age of foolishness, it was the epoch of belief, it was the epoch of\n", 
         "incredulity, it was the season of Light, it was the season of Darkness, it was\n",
         "the spring of hope, it was the winter of despair, we had everything before us,\n", 
         "we had nothing before us, we were all going direct to heaven, we were all going\n", 
         "direct the other way - in short, the period was so far like the present period,\n", 
         "that some of its noisiest authorities insisted on its being received, for good\n", 
         "or for evil, in the superlative degree of comparison only.\n" };

    printf("Connected, sending data...\n");

    // send all the messages
    for (int i = 0; i < 13; i++) {
        printf("%d - %s", i, data_to_send[i]);
	    send(sock, data_to_send[i], strlen(data_to_send[i]), 0);
        sleep(5);
    }

	// close the socket
	close(sock);
	return 0;
}
