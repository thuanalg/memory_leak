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

int got_aes = 0;
uchar aes256_key[AES_BYTES];
uchar aes256_iv[AES_IV_BYTES];

void fier() {

}

void notifier(char *ip) {
	int sz = MAX_MSG + 1;
	struct sockaddr_in	 servaddr;
	//char data[MAX_DATA]
	MSG_NOTIFY *msg = 0;
	uint16_t n = 0;
	struct timespec t;
	char buf [2];
	int sockfd = 0;
	int err = 0;
	char *buffer = 0;
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
	buffer = (char*) msg;

	msg->com.type = MSG_NTF;
	msg->com.ifroute = G_NTF_CLI;
	memcpy(msg->com.dev_id, dev_id, MIN(LEN_DEVID, strlen(dev_id) + 1));
	memcpy(msg->com.ntf_id, id, MIN(LEN_DEVID, strlen(id) + 1));
	n = MAX_MSG - sizeof(MSG_COMMON) - 1 - AES_IV_BYTES;
	memset(buf, 0, sizeof(buf));
	uint16_2_arr(buf, n, 2);
	memcpy(msg->com.len, buf, 2);
	snprintf(msg->data, n, "%s", "fdjhfjhfj fdkfldfd kkjtjtk Nguyen Thai Thuan, Thanh Hong, Thanh Tam.");
	uint64_2_arr(msg->com.second, t.tv_sec, 8);
	uint64_2_arr(msg->com.nano, t.tv_nsec, 8);

	DUM_MSG(&(msg->com));

	enc = (uchar*) msg;
	if(got_aes) {
		int err = 0;
		int outlen = 0;
		uchar tag[AES_IV_BYTES + 1];
		uchar *out = 0;	
		do {
			err = ev_aes_enc(buffer, &out, aes256_key, 
				aes256_iv, sz - 1 - AES_IV_BYTES, &outlen, tag);
			if(err) {
				break;
			}
			fprintf(stdout, "====================outlen: %d\n", outlen);
			fprintf(stdout, "===================out: %s\n", out);
			memcpy(buffer, out, outlen);
			memcpy(buffer + outlen , tag, AES_IV_BYTES);
		} while(0);
		if(out) {
			MY_FREE(out);
		}
		enc[MAX_MSG] = ENCRYPT_AES;
	} else {
		enc[MAX_MSG] = ENCRYPT_NON;
	}

	n = sendto(sockfd, buffer, MAX_MSG + 1,
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
	sleep(1);
	cmd_2_srv(MSG_GET_AES, G_NTF_SRV, 0, 0, (char*) id, argv[1]);
	sleep(1);
	//notifier(argv[1]);
	while(1) {
		MSG_DATA *dt = 0;
		usleep(100 * 1000);
		clock_gettime(CLOCK_REALTIME, &t1);
		if(t1.tv_sec - t0.tv_sec > 3) {
			if(count > 1) {
			//if(1) {
				LOG(LOG_ERR, "Cannot notify to CLI");
				break;
			}
			t0 = t1;
			if(got_aes) {
				send_msg_track(id, sockfd, argv[1], PORT + 1, &t0, aes256_key, aes256_iv);
			} else {
				send_msg_track(id, sockfd, argv[1], PORT + 1, &t0, 0, 0);
			}
			//notifier(argv[1]);
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
		if(n < sizeof(MSG_COMMON)) {
			continue;
		}

		do {
			MSG_COMMON *msg = 0;
			uchar enc = 0;
			enc = buffer[n-1];
			fprintf(stdout, "recv -------: %d\n", n);
			if(enc) {
				if(enc == ENCRYPT_CLI_PUB) {
					fprintf(stdout, "MUST dec by rsa private key.\n");		
					uchar *out = 0;
					int outlen = 0;
					RSA *key = 0;
					do {
						key = get_cli_prv((char*)id);				
						rsa_dec(key, buffer, &out, n-1, &outlen);
						if(!out) {
							break;
						}
						memset(buffer, 0, sizeof(buffer));
						memcpy(buffer, out, outlen);
						n = outlen;
					} while(0);
					if(key) {
						RSA_free(key);
					}
					if(out) {
						MY_FREE(out);
					}
				}	
			}
			dt = (MSG_DATA *) buffer;
			msg = (MSG_COMMON *) &(dt->com);
			if(msg->ifroute == F_SRV_NTF) {
				if(msg->type == MSG_GET_AES) {
					do {
						uchar *p = dt->data;
						uint16_t sz = 0;
						arr_2_uint16(msg->len, &sz, 2);
						fprintf(stdout, "data size: %d\n", sz);
						if(sz != (AES_BYTES + AES_IV_BYTES) ) {
							break;
						}
						fprintf(stdout, "did get AES key\n");
						memcpy(aes256_key, p, AES_BYTES); 
						memcpy(aes256_iv, p + AES_BYTES, AES_IV_BYTES); 
						got_aes = 1;
						notifier(argv[1]);
					} while(0);
				}	
			}
			fprintf(stdout, "Did get a message devid: %s, n: %d\n.\n", msg->dev_id, n);
		} while(0);
	}
	close(sockfd);
	closelog();
	return 0;
}

