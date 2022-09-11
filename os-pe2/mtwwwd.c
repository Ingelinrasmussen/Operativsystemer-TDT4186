#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sem.h"
#include "bbuffer.h"

#define BACKLOG 5
#define PORT 7654  //Port used to acess web
#define BUFFER_SIZE 1024
#define MAXREQ (4096 * 1024)  //Hvordan vet vi hvor mye buffer vi bør allokere? 

//MÅ legge inn feilhåndtering
char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

BNDBUF *BB; 
char const *www_path; 


void *handle_connection() {


	while (1) {

		int newsocketfd = bb_get(BB);
		if (newsocketfd < 0)
            // Error has occured 
            error("ERROR on accepting buffer");
		bzero(buffer, sizeof(buffer));
	
		//Reads the message from client and copy to buffer
		int request = read(newsocketfd, buffer, sizeof(buffer) - 1); 

		if( request < 0){
			error("ERROR could not read from socket");
		}

		//read file 
		//vet ikke om dette fungerer
		char path[32]; 
		char *fbuffer = NULL;
		FILE *f;
		f = fopen(path, "r");
		//feilhåndtering 

		fseek(f, 0, SEEK_END);
		int bytes = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *file_string = malloc(sizeof(char)* bytes);
		int reading = fread(file_string, sizeof(char), bytes, f);
		if (reading < 0)
			error("ERROR reading from socket");

		//usikker på denne, må og ha www_path med? 
		snprintf(body, sizeof(body), file_string, buffer);

		//headers, browser page 
		snprintf(msg, sizeof(msg),
				"HTTP/1.1 200 OK\n" //skal det være HTTP/0.9? 
				"Content-Type: text/html\n"
				"Content-Length: %ld\n\n%s",
				strlen(body), body);
				
		//close file connected to socket 
		fclose(f);

/*		snprintf(body, sizeof(body),
					"<html>\n<body>\n"
					"<h1>Hello web browser</h1>\nThis is a website made by Vår\n"
					"<pre>%s</pre>\n"
					"</body>\n</html>\n",
					buffer);
*/
		
		//Sends buffer to back to client
		request = write(newsocketfd, msg, strlen(msg));

		if (request < 0){
			error("ERROR could not write to socket");
			close(newsocketfd);
		}
		close(newsocketfd); //Closes the new socket 
		printf("Connection made");
	}
}

//Inspired by lectures
int main(int argc, char* argv[]) {

	if (argc < 3 ){
		printf("Not enough arguments");
		exit(0);
	}
	else if( argc > 5){
		printf("Too many arguments");
		exit(0);
	}

	//parsing 
	if (argv[1]) {
		www_path = argv[1];
		printf("Path %s enabled\n", www_path);
	}
	else {
		fprintf(stderr, "Did not find path.\n");
		exit(EXIT_FAILURE);
	}
	unsigned int port = strtoul(argv[2], NULL, 10);

	//initialize buffer 
	int maximum = strtol(argv[4], NULL, 10);
	BB = bb_init(maximum); 

	//initialize threads with for-loop 
//	malloc(sizeof(pthread_t)*2);
    for( int i =0; i<4; i++){
        pthread_t thread; 
    	pthread_create(&thread, NULL, &handle_connection, BB);

	}

	//socket creation 
	int socketfd, accept_socket, request, bind_socket;
	struct sockaddr_in server, client;
	int serverlen = sizeof(server);
	socklen_t clientlen = sizeof(client);
	int sockaddrlen = sizeof(struct sockaddr_in);


	socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (socketfd < 0){
		printf("Error. Could not create socket");
	}
	printf("Socket created sucessfully");

	
	bzero((char *)&server, serverlen); //Erases the data in the sever address

	//Prepare the adresses
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT); //Sets port nr

	//Bind - The server adress is assigned to the socket
	bind_socket = bind(socketfd, (struct sockaddr*)&server, serverlen);
	if (bind_socket < 0){
		printf("Error. Bind failed");
	}
	printf("Bind created sucessfully");

	//Listen
	listen(socketfd, BACKLOG);

	printf("Server sucessfully started on %d", PORT);

	//Accept
	while(1){
	
		//Creates new socket to the first connection reuest to the listening socket
		accept_socket = accept(socketfd, (struct sockaddr*)&client, &clientlen); 
		if (accept_socket < 0){
			printf("Error. Accept failed");
		}

		bb_add(BB, socketfd);
		printf("New socket added to buffer");


	}

}



