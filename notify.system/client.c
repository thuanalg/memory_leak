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

int send_msg_track(int sockfd, struct sockaddr_in* addr, struct timespec *);
int send_msg_fb(int sockfd, struct sockaddr_in* addr, MSG_COMMON *msg);

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
	int err = 0;
	int sz = (int) sizeof(MSG_COMMON);
	

	setlogmask (LOG_UPTO (LOG_INFO));
	openlog ("zclient_device", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	// Creating socket file descriptor
	clock_gettime(CLOCK_REALTIME, &t0);
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
		
	int n = 0, len = sizeof(servaddr);
	send_msg_track(sockfd, &servaddr, &t0);
	while(1) {
		usleep(10 * 1000);
		clock_gettime(CLOCK_REALTIME, &t1);
		if(t1.tv_sec - t0.tv_sec > INTER_TRACK) {
			t0 = t1;
			send_msg_track(sockfd, &servaddr, &t1);
		}
		len = sizeof(servaddr);
		memset(buffer, 0, sizeof(buffer));
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_WAITALL, (struct sockaddr *) &servaddr,
					&len);
		if(n < 1 ) {
			continue;
		}
		fprintf(stdout, "=========== n receive: %d\n");
		if(n >= sizeof(MSG_COMMON) && fbaddr.sin_family == AF_INET) {
			MSG_COMMON *msg = (MSG_COMMON *) buffer;

			if(msg->ifroute == G_NTF_CLI) {
				fprintf(stdout, "\n++++++++++++\n");
				DUM_MSG(msg); 
				DUM_IPV4(&fbaddr);
				msg->ifroute = G_CLI_NTF;
				memset(msg->len, 0, LEN_U16INT);
				send_msg_fb(sockfd, &servaddr, msg);
			}
		}
	}
	err = close(sockfd);
	if(err) {
		LOG(LOG_ERR, "Close socket err.");
	}
	closelog();
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

int send_msg_fb(int sockfd, struct sockaddr_in* addr, MSG_COMMON *msg) {
	int n = 0;
	int len = sizeof(MSG_COMMON);
	addr->sin_port = htons(PORT);
	n = sendto(sockfd, msg, len,
		MSG_CONFIRM, (const struct sockaddr *) addr,
			sizeof(*addr));
	DUM_MSG(msg);
	return n;
}

