#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void error(const char* msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char* argv[]) {
	int sockfd, portno;
	socklen_t clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) {
		portno = 9000;
	} else {
		portno = atoi(argv[1]);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);			//UDP SOCK_DGRAM
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

		if (recvfrom(sockfd,buffer,1024,0,(struct sockaddr *)&cli_addr,&clilen) < 0) error("recvfrom");

		int fd = 0;
		if(buffer[0] == 'U' || buffer[0] == 'u')
		{
			fd = open("/proc/uptime", O_RDONLY);
			if(fd < 0) {error("File does not exist");}
		}
		else if (buffer[0] == 'L' || buffer[0] == 'l')
		{
			fd = open("/proc/loadavg", O_RDONLY);
			if(fd < 0) {error("File does not exist");}
		} else
		{
			printf("Invalid input\n");
		}
		int buffersize = read(fd, buffer, 1024);
		
		if (sendto(sockfd, buffer, buffersize, 0, (struct sockaddr *)&cli_addr, clilen) < 0) error("sendto");
			if(close(fd)){error("Error closing file");}
		}
	close(sockfd);
	return 0;
}