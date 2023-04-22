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
	
#define PORT	 9090
#define MAXLINE 1024
const char *id = "b7bb3690-ebcb-4bf9-88b0-31c130ec44a2";
typedef struct msg_format {
	char msg_id[128];
	char data[512];
} MSG_FORMAT;
	
// Driver code
int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAXLINE];
	char *hello = "Hello from client";
	struct sockaddr_in	 servaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
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
		
	n = sendto(sockfd, (const char *)hello, strlen(hello),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	printf("line: %d, Hello message sent: n: %d.\n", __LINE__, n);
			
	
	servaddr.sin_port = htons(PORT + 1);


	n = sendto(sockfd, (const char *)hello, strlen(hello),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	printf("line: %d, Hello message sent: n: %d.\n", __LINE__, n);
			

	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, (struct sockaddr *) &servaddr,
				&len);
	if(n>0 && servaddr.sin_family == AF_INET) {
		char str[INET_ADDRSTRLEN + 1];
		str[INET_ADDRSTRLEN] = 0;
		inet_ntop(AF_INET, &(servaddr.sin_addr), str, INET_ADDRSTRLEN);
		fprintf(stdout, "feedback port: %d\n", (int) servaddr.sin_port);
		fprintf(stdout, "feedback IP: %s\n", str);
	}
	buffer[n] = '\0';
	printf("Server : %s\n", buffer);
	
	close(sockfd);
	return 0;
}
