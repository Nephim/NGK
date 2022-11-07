#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "iknlib.h"



void error(const char* msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    if (argc < 3) {
        fprintf(stderr, "usage %s <hostname> <filename>\n", argv[0]);
        exit(0);
    }
    portno = 9000; //Port hardcoded
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    
    writeTextTCP(sockfd, argv[2]);
    long filesize = 0;
    filesize = readFileSizeTCP(sockfd);
	if(!filesize) {
        fprintf(stderr, "ERROR, no such file on server\n");
        exit(0);
    }
    printf("%ld\n",filesize);
    char filebuffer[1000];
    int fd = open(extractFileName(argv[2]), O_CREAT|O_RDWR, 0640);
    if (fd < 0)
    {
        error("Could not open file");
    }  
    int buffersize;
    for(;;) {        
        buffersize = recv(sockfd, filebuffer,1000, MSG_WAITALL);
        buffersize = write(fd,filebuffer, buffersize);
        if (buffersize < 0){
       		fprintf(stderr, "ERROR, writing to file\n");
        	exit(0);
    	} else if(buffersize < 1000)  break;
    }
	//sleep(1);
    close(fd);
    close(sockfd);
    return 0;
}