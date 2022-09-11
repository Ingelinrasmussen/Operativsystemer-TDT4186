#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "bbuffer.h"

#define MAXREQ (4096 * 1024)
char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
char const *www_path;

void error(const char *msg)
{
    perror(msg);
    printf("perror");
    exit(1);
}

char *read_file(char *filename){
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) return NULL; // error opening file

    // move pointers to determine size of the file
    // used for dynamically allocating space for the string 
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // allocate according to the size 
    char *html_str = malloc(sizeof(char)* (len + 1));
    char next_char; 
    int count = 0;
    //read each char until end of file 
    while ((next_char = fgetc(fp)) != EOF) {
        // add the read char to the string
        html_str[count] = next_char; 
        count++;
    }
    html_str[count] = '\0';
    return html_str; 
}

void *worker_thread(void *bbuffer)
{
    BNDBUF *bb = bbuffer;
    while (1)
    {
        // assigns a socket from the bounded buffer 
        int newsockfd = bb_get(bb);
        if (newsockfd < 0)
            // There has been an error 
            error("ERROR has occured");
        // Writes zeros to location pointed to by buffer and its size
        bzero(buffer, sizeof(buffer));

        //read upto buffer-size bytes from socket into a buffer
        int n = read(newsockfd, buffer, sizeof(buffer) - 1);
        if (n < 0)
            // Error reading from socket
            error("ERROR reading from socket");
        
        //reading the correct html file
        snprintf(body, sizeof(body),
                 read_file((char*) www_path),
                 buffer);
                // Headers for the browser to work 
                snprintf(msg, sizeof(msg),
                        "HTTP/1.0 200 OK\n"
                        "Content-Type: text/html\n"
                        "Content-Length: %ld\n\n%s",
                        strlen(body), body);
                // writes from msg to socket
                n = write(newsockfd, msg, strlen(msg));
                if (n < 0)
        {
            //error writing 
            error("ERROR writing to socket");
            close(newsockfd);
        }
        // close socket connection
        close(newsockfd);
    }
}

// inspired by lecture materials 
int main(int argc, char const *argv[])
{
    if (argc != 5)
    {
        // not invoked properly 
        error("too few arguments");
    }
    // parse the invokation 
    www_path = argv[1];
    unsigned int port = strtoul(argv[2], NULL, 10);
    unsigned int threads = strtoul(argv[3], NULL, 10);
    unsigned int bufferslots = strtoul(argv[4], NULL, 10);

    //init server variables 
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    listen(sockfd, 5);
    BNDBUF *bb = bb_init(bufferslots);

    for (int i = 0; i < threads; i++)
    {
        // create worker threads: they operate the function worker_thread
        pthread_t p;
        pthread_create(&p, NULL, &worker_thread, bb);
    }

    while (1)
    {
        // adds the requests to the buffer for delegation to worker threads
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                           &clilen);
        bb_add(bb, newsockfd);
    }
}