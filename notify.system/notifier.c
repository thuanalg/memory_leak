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
	
#define MAXLINE 	MAX_MSG
const char *id = "ed628094-63f3-452a-91c8-3ae24f281dd2";
char *dev_id = "b7bb3690-ebcb-4bf9-88b0-31c130ec44a2";


void notifier(char *ip) {
	int sz = MAX_MSG + 1;
	struct sockaddr_in	 servaddr;
	MSG_NOTIFY *msg = 0;
	uint16_t n = 0;
	struct timespec t;
	char buf [2];
	int sockfd = 0;
	int err = 0;
	uchar *enc = 0;
	clock_gettime(CLOCK_REALTIME, &t);
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
		LOG(LOG_ERR, "Cannot create socket.");
		exit(EXIT_FAILURE);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(PORT);

	MY_MALLOC(msg, sz);

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

	DUM_MSG(&(msg->com));

	enc = (uchar*) msg;
	enc[MAX_MSG] = ENCRYPT_NON;

	n = sendto(sockfd, (char *)msg, MAX_MSG + 1,
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	MY_FREE(msg);
	err = close(sockfd);
	if(err) {
		LOG(LOG_ERR, "Close socket error.");
	}
}
// Driver code

int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAXLINE];
	struct timespec t0 = {0};
	struct timespec t1 = {0};
	struct sockaddr_in	 servaddr;
	struct sockaddr_in	 fbaddr;
	int val = 1;
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
	servaddr.sin_addr.s_addr = INADDR_ANY;
	//servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(NTF_PORT);

	//We MUST bind  this socket to reuse	
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	//servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	//servaddr.sin_port = htons(PORT);
		
	int n = 0, len = sizeof(servaddr);
	send_msg_track(id, sockfd, argv[1], PORT + 1, &t0, 0, 0);
	notifier(argv[1]);
	while(1) {
		usleep(100 * 1000);
		clock_gettime(CLOCK_REALTIME, &t1);
		if(t1.tv_sec - t0.tv_sec > 3) {
			if(count > 1) {
			//if(1) {
				LOG(LOG_ERR, "Cannot notify to CLI");
				break;
			}
			t0 = t1;
			send_msg_track(id, sockfd, argv[1], PORT + 1, &t0, 0, 0);
			notifier(argv[1]);
			++count;
		}
		len = sizeof(fbaddr);
		memset(buffer, 0, sizeof(buffer));
		memset(&fbaddr, 0, sizeof(fbaddr));
		n = recvfrom(sockfd, (char *)buffer, MAX_MSG,
					MSG_DONTWAIT, (struct sockaddr *) &fbaddr,
					&len);

		if(n < 1 ) {
			continue;
		}
		if(n >= sizeof(MSG_COMMON)) {
			MSG_COMMON *msg = (MSG_COMMON *) buffer;
			DUM_MSG(msg); 
			fprintf(stdout, "recv: %d\n", n);
			break;
		}
	}
	close(sockfd);
	closelog();
	return 0;
}

