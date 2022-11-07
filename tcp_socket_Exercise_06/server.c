/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "iknlib.h"

void error(const char* msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char* argv[]) {
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) {
		portno = 9000;
	} else {
		portno = atoi(argv[1]);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");
	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	
	for(;;) {
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
		if (newsockfd < 0) error("ERROR on accept");
		bzero(buffer, 256);
		readTextTCP(newsockfd, buffer, 255);
		printf("Here is the filename: %s\n", buffer);
		printf("Here is the newsockfd: %d\n", newsockfd);
		
		long filesize = getFilesize(buffer);  // Gets the filesize return 0 if
											 // there is no file of that name
		char fileSizeStringBuffer[32];
		snprintf(fileSizeStringBuffer,sizeof(fileSizeStringBuffer),"%ld",filesize);
		printf("%s\n", fileSizeStringBuffer);
		writeTextTCP(newsockfd, fileSizeStringBuffer);
		if (filesize) {
			int fd = 0;
			char fileBuffer[1000];

			fd = open(buffer, O_RDONLY);
			if (fd < 0) error("File does not exist");

			int buffersize;
			for(;;)
			{
				buffersize = read(fd, fileBuffer, 1000);
				buffersize = write(newsockfd, fileBuffer, buffersize);
				if (buffersize < 0) { error("ERROR writing to socket"); }
				else if(buffersize < 1000) break;
			}
			close(fd);
		}
		close(newsockfd);
	}
	close(sockfd);
	return 0;
}