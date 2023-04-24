// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "msg_notify.h"
	
#define PORT	 9090
#define MAXLINE 1024
const char *id = "b7bb3690-ebcb-4bf9-88b0-31c130ec44a2";

int send_msg_resgister(int sockfd, struct sockaddr_in* addr);
int send_msg_resgister(int sockfd, struct sockaddr_in* addr)
{
	MSG_REGISTER msg;
	char buff[1501];
	int n = 0;
	memset(&msg, 0, sizeof(msg));
	msg.com.type = MSG_REG;
	memcpy(msg.com.dev_id, id, LEN_DEVID);
	msg.com.type = 0;
	
	memset(buff, 0, 1501);
	memcpy(buff, (char*) &msg, sizeof(msg));
	
	
	addr->sin_port = htons(PORT);
	n = sendto(sockfd, buff, sizeof(msg),
		MSG_CONFIRM, (const struct sockaddr *) addr,
			sizeof(*addr));
	printf("line: %d, Hello message sent: n: %d.\n", __LINE__, n);

	
	addr->sin_port = htons(PORT + 1);
	n = sendto(sockfd, buff, sizeof(msg),
		MSG_CONFIRM, (const struct sockaddr *) addr,
			sizeof(*addr));
	printf("line: %d, Hello message sent: n: %d.\n", __LINE__, n);

	return 0;
}
	

void client() {
	
}
// Driver code

int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAXLINE];
	char *hello = "Hello from client";
	struct sockaddr_in	 servaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	if (!argv[1]) {
		fprintf(stdout, "Address need entering as argv[1].\n");
		exit(EXIT_FAILURE);
	}
	if(INADDR_NONE == inet_addr(argv[1])) {
		fprintf(stdout, "Address need entering as argv[1].\n");
		exit(EXIT_FAILURE);
	}	
	memset(&servaddr, 0, sizeof(servaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET;
	//servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(PORT);
		
	int n, len = sizeof(servaddr);
			
	send_msg_resgister(sockfd, &servaddr);

	while(1) {
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_WAITALL, (struct sockaddr *) &servaddr,
					&len);
		if(n < 1 )
		{
			sleep(2); 
			send_msg_resgister(sockfd, &servaddr);
			continue;
		}
		if(n>0 && servaddr.sin_family == AF_INET) {
			char str[INET_ADDRSTRLEN + 1];
			str[INET_ADDRSTRLEN] = 0;
			inet_ntop(AF_INET, &(servaddr.sin_addr), str, INET_ADDRSTRLEN);
			fprintf(stdout, "feedback port: %d\n", (int) servaddr.sin_port);
			fprintf(stdout, "feedback IP: %s\n", str);
		}
		buffer[n] = '\0';
		printf("Server : %s\n", buffer);
		printf("DID REGISTER\n");
		break;	
	}
/*
	while(1) {


	}
*/
	close(sockfd);
	return 0;
}
