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
int send_msg_track(int sockfd, struct sockaddr_in* addr, struct timespec *);

void client() {
	
}
// Driver code

int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAXLINE];
	struct timespec t0 = {0};
	struct timespec t1 = {0};
	char *hello = "Hello from client";
	struct sockaddr_in	 servaddr;
	struct sockaddr_in	 fbaddr;
	int val = 1;
	
	// Creating socket file descriptor
	clock_gettime(CLOCK_REALTIME, &t0);
//	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));		
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
		memset(&fbaddr, 0, sizeof(fbaddr));
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_WAITALL, (struct sockaddr *) &fbaddr,
					&len);
		if(n < 1 )
		{
			usleep(10000);
			clock_gettime(CLOCK_REALTIME, &t1);
			if(t1.tv_sec - t0.tv_sec > 0)
			{
					t0 = t1;
					send_msg_resgister(sockfd, &servaddr);
			}
			continue;
		}
		if(n>0 && fbaddr.sin_family == AF_INET) {
			char str[INET_ADDRSTRLEN + 1];
			str[INET_ADDRSTRLEN] = 0;
			inet_ntop(AF_INET, &(fbaddr.sin_addr), str, INET_ADDRSTRLEN);
			fprintf(stdout, "---- size: %d, feedback port: %d\n", n, (int) fbaddr.sin_port);
			fprintf(stdout, "-----size n: %d, feedback IP: %s\n", n, str);
		}
		buffer[n] = '\0';
		printf("Server : %s\n", buffer);
		printf("DID REGISTER\n");
		while(1) {
			usleep(10000);
			clock_gettime(CLOCK_REALTIME, &t1);
			if(t1.tv_sec - t0.tv_sec > 10) {
				t0 = t1;
				send_msg_track(sockfd, &servaddr, &t1);
			}
			len = sizeof(servaddr);
			n = recvfrom(sockfd, (char *)buffer, MAXLINE,
						MSG_WAITALL, (struct sockaddr *) &servaddr,
						&len);
		}
		break;	
	}
/*
	while(1) {


	}
*/
	close(sockfd);
	return 0;
}


int send_msg_track(int sockfd, struct sockaddr_in* addr, struct timespec *t) {

	MSG_NOTIFY msg;
	char buff[1501];
	int n = 0;
	do {
		if(!t) break;
		if(!addr) break;
		fprintf(stdout, "\ntv_sec: %llu\n\n", t->tv_sec);
		(*addr).sin_port = htons(PORT);

		memset(&msg, 0, sizeof(msg));
		msg.com.type = MSG_TRA;
		memcpy(msg.com.dev_id, id, LEN_DEVID);
		
		memset(buff, 0, 1501);
		memcpy(buff, (char*) &msg, sizeof(msg));
		
		addr->sin_port = htons(PORT + 1);
		n = sendto(sockfd, buff, sizeof(msg),
			MSG_CONFIRM, (const struct sockaddr *) addr,
				sizeof(*addr));
		printf("line: %d, Send tracking message: n: %d.\n", __LINE__, n);
	}
	while(0);
	return 0;

}

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
	
