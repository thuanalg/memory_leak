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
const char *id = "ed628094-63f3-452a-91c8-3ae24f281dd2";
char *dev_id = "b7bb3690-ebcb-4bf9-88b0-31c130ec44a2";

int send_msg_resgister(int sockfd, struct sockaddr_in* addr);
int send_msg_track(int sockfd, struct sockaddr_in* addr, struct timespec *);
int send_msg_fb(int sockfd, struct sockaddr_in* addr, MSG_COMMON *msg);
void notifier(char *ip, int sockfd) {
	int sz = MAX_MSG;
	struct sockaddr_in	 servaddr;
	MSG_NOTIFY *msg = 0;
	uint16_t n = 0;
	struct timespec t;
	char buf [128];

	clock_gettime(CLOCK_REALTIME, &t);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(PORT);

	msg = malloc(sz);
	memset(msg, 0, sz);	
	memset(msg, 0, sz);	

	msg->com.type = MSG_NOTIFIER;
	msg->com.ifroute = G_NTF_SRV;
	memcpy(msg->com.dev_id, dev_id, MIN(LEN_DEVID, strlen(dev_id) + 1));
	memcpy(msg->com.ntf_id, dev_id, MIN(LEN_DEVID, strlen(id) + 1));
	n = MAX_MSG - sizeof(MSG_COMMON);
	memset(buf, 0, sizeof(buf));
	uint16_2_arr(buf, n, 2);
	memcpy(msg->com.len, buf, 2);
	memcpy(msg->data, "thuan", sizeof("thuan"));
/*	
int uint32_2_arr(unsigned char *arr, uint32_t , int sz);
int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz);
int uint16_2_arr(unsigned char *arr, uint16_t , int sz);
int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz);
*/
	//len = sizeof(servaddr);
	n = sendto(sockfd, (char *)msg, MAX_MSG,
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	printf("line: %d, Send tracking message: n: %u.\n", __LINE__, n);
	free(msg);
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
	int sz = (int) sizeof(MSG_COMMON);
	
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
	servaddr.sin_family = AF_INET;
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

		sleep(1);
		notifier(argv[1], sockfd);
		if(n < 1 ) {
			continue;
		}
		fprintf(stdout, "=========== n receive: %d\n");
		if(n >= sizeof(MSG_COMMON) && fbaddr.sin_family == AF_INET) {
			MSG_COMMON *msg = (MSG_COMMON *) buffer;
			if(msg->ifroute) {
				continue;
			}
			fprintf(stdout, "\n++++++++++++\n");
			dum_msg(msg, __LINE__); 
			dum_ipv4(&fbaddr, __LINE__);
			msg->ifroute = 1;
			send_msg_fb(sockfd, &servaddr, msg);
		}
	}
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
	addr->sin_port = htons(PORT);
	n = sendto(sockfd, msg, sizeof(MSG_COMMON),
		MSG_CONFIRM, (const struct sockaddr *) addr,
			sizeof(*addr));
	dum_msg(msg, __LINE__);
	return n;
}

int send_msg_resgister(int sockfd, struct sockaddr_in* addr)
{
	MSG_REGISTER msg;
	char buff[1501];
	int n = 0;
	memset(&msg, 0, sizeof(msg));
	msg.com.type = MSG_REG;
	memcpy(msg.com.dev_id, id, LEN_DEVID);
	
	memset(buff, 0, 1501);
	memcpy(buff, (char*) &msg, sizeof(msg));
	
	
//	addr->sin_port = htons(PORT);
//	n = sendto(sockfd, buff, sizeof(msg),
//		MSG_CONFIRM, (const struct sockaddr *) addr,
//			sizeof(*addr));
//	dum_msg(&(msg.com), __LINE__);

	
	msg.com.type = MSG_TRA;
	addr->sin_port = htons(PORT + 1);
	memset(buff, 0, 1501);
	memcpy(buff, (char*) &msg, sizeof(msg));
	n = sendto(sockfd, buff, sizeof(msg),
		MSG_CONFIRM, (const struct sockaddr *) addr,
			sizeof(*addr));
	dum_msg(&(msg.com), __LINE__);

	return 0;
}
	
