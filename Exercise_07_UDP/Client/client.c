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
#include <ncurses.h>

void error(const char* msg) {
	perror(msg);
	exit(0);
}

int main(int argc, char* argv[]) {
	int sockfd, portno;
	struct sockaddr_in serv_addr, from;
	struct hostent* server;
	char buffer[1024];

	if (argc < 3) {
		fprintf(stderr, "usage %s <hostname> <u or l>\n", argv[0]);
		exit(0);
	}
	portno = 9000; //Port hardcoded
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
	unsigned int addr_length = sizeof(struct sockaddr_in);
	if(argv[2][0] != 'u' && argv[2][0] != 'U' && argv[2][0] != 'l' && argv[2][0] != 'L')
	{
		printf("Invalid Character\n");
		exit(-1);
	}
	for(;;)
	{//Anders being stupid and wanting to make it iterative
		initscr();
		if(argv[2][0] == 'u' || argv[2][0] == 'U')
		{
			printw("Uptime !!!\n\n");
		} 
        else if (argv[2][0] == 'l' || argv[2][0] == 'L')
        {
			printw("Load avg !!!\n\n");
		} 
		
		if (sendto(sockfd ,argv[2], strlen(argv[2]),0,(const struct sockaddr *)&serv_addr, addr_length) < 0) error("Sendto");
		bzero(buffer,sizeof(buffer));			//Clear buffer
		if (recvfrom(sockfd,buffer,1024,0,(struct sockaddr *)&serv_addr, &addr_length) < 0) error("recvfrom");
		printw("%s\n", buffer);
		refresh();			/* Print it on to the real screen */
		sleep(1);
		erase();		
	}    
	
	endwin();			/* End curses mode */
	printf("%s\n", buffer);
	close(sockfd);
	return 0;
}