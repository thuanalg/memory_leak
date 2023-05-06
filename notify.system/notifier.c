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

int send_msg_track(int sockfd, struct sockaddr_in* addr, struct timespec *);

void notifier(char *ip, int sockfd) {
	int sz = MAX_MSG;
	struct sockaddr_in	 servaddr;
	MSG_NOTIFY *msg = 0;
	uint16_t n = 0;
	struct timespec t;
	char buf [2];

	clock_gettime(CLOCK_REALTIME, &t);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(PORT);

	MY_MALLOC(msg, sz);
	memset(msg, 0, sz);	
	memset(msg, 0, sz);	

	msg->com.type = MSG_NTF;
	msg->com.ifroute = G_NTF_CLI;
	memcpy(msg->com.dev_id, dev_id, MIN(LEN_DEVID, strlen(dev_id) + 1));
	memcpy(msg->com.ntf_id, id, MIN(LEN_DEVID, strlen(id) + 1));
	n = MAX_MSG - sizeof(MSG_COMMON);
	memset(buf, 0, sizeof(buf));
	uint16_2_arr(buf, n, 2);
	memcpy(msg->com.len, buf, 2);
	memcpy(msg->data, "thuan", sizeof("thuan"));
	uint64_2_arr(msg->com.second, t.tv_sec, 8);
	uint64_2_arr(msg->com.nano, t.tv_nsec, 8);


/*	
int uint32_2_arr(unsigned char *arr, uint32_t , int sz);
int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz);
int uint16_2_arr(unsigned char *arr, uint16_t , int sz);
int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz);
*/
	//len = sizeof(servaddr);
	DUM_MSG(&(msg->com));
	n = sendto(sockfd, (char *)msg, MAX_MSG,
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	MY_FREE(msg);
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
	int count = 0;
	
	setlogmask (LOG_UPTO (LOG_INFO));
	openlog ("znotifier", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	// Creating socket file descriptor
	clock_gettime(CLOCK_REALTIME, &t0);
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
		LOG(LOG_ERR, "Cannot create socket.");
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
	notifier(argv[1], sockfd);
	while(1) {
		usleep(100 * 1000);
		clock_gettime(CLOCK_REALTIME, &t1);
		if(t1.tv_sec - t0.tv_sec > 2) {
			if(count > 3) {
				LOG(LOG_ERR, "Cannot notify to CLI");
				break;
			}
			t0 = t1;
			send_msg_track(sockfd, &servaddr, &t1);
			notifier(argv[1], sockfd);
			++count;
		}
		len = sizeof(servaddr);
		memset(buffer, 0, sizeof(buffer));
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_WAITALL, (struct sockaddr *) &servaddr,
					&len);

		if(n < 1 ) {
			continue;
		}
		fprintf(stdout, "=========== n receive: %d, \n", n);
		if(n >= sizeof(MSG_COMMON)) {
			MSG_COMMON *msg = (MSG_COMMON *) buffer;
			DUM_MSG(msg); 
			break;
		}
	}
	close(sockfd);
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

