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
const char *id = "b7bb3690-ebcb-4bf9-88b0-31c130ec44a2";

int send_msg_fb(struct sockaddr_in* addr, MSG_COMMON *msg);

void client() {
	
}
// Driver code
uchar aes256_key[AES_BYTES];
uchar aes256_iv[AES_IV_BYTES];

int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAXLINE + 1];
	struct timespec t0 = {0};
	struct timespec t1 = {0};
	struct sockaddr_in	 servaddr;
	struct sockaddr_in	 fbaddr;
	int val = 1;
	int err = 0;
	char got_aes = 0;
	MSG_DATA *dt = 0;
	

	setlogmask (LOG_UPTO (LOG_INFO));
	openlog ("zclient_device", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	do {
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
		servaddr.sin_addr.s_addr = INADDR_ANY;
		//servaddr.sin_addr.s_addr = inet_addr(argv[1]);
		servaddr.sin_port = htons(DEV_PORT);

		//We MUST bind  this socket to reuse	
		if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
		{
			perror("bind failed");
			exit(EXIT_FAILURE);
		}
		DUM_IPV4(&servaddr);	
		int n = 0; 
		//int len = sizeof(servaddr);
		send_msg_track(id, sockfd, argv[1], PORT + 1, &t0, 0, 0);
		sleep(1);
		cmd_2_srv(MSG_GET_AES, G_CLI_SRV, 0, 0, (char*) id, argv[1]);
		while(1) {
			int enc = 0;
			usleep(10 * 1000);
			clock_gettime(CLOCK_REALTIME, &t1);
			if(t1.tv_sec - t0.tv_sec > INTER_TRACK) {
				t0 = t1;
				if(!got_aes) {
					send_msg_track(id, sockfd, argv[1], PORT + 1, &t0, 0, 0);
				} else {
					send_msg_track(id, sockfd, argv[1], PORT + 1, &t0, aes256_key, aes256_iv);
				}
			}
			//len = sizeof(fbaddr);
			memset(&fbaddr, 0, sizeof(fbaddr));
			memset(buffer, 0, sizeof(buffer));
			n = recvfrom(sockfd, (char *)buffer, sizeof(buffer),
						0, 0, 0);
			if(n < 1 ) {
				continue;
			}
			if(n < sizeof(MSG_COMMON)) {
				continue;
			}
			enc = buffer[n-1];
			MSG_COMMON *msg = (MSG_COMMON *) buffer;
			if(enc == ENCRYPT_CLI_PUB) {
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
			} else if(enc == ENCRYPT_AES) {
				fprintf(stdout, "\n\nMUST use AES+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.\n\n");
				uchar *out = 0;
				int outlen = 0;
				int inlen = 0;
				int err = 0;
				fprintf(stdout, "AES_ENCRYPT n = %d.\n", n);
				do {
					inlen = n - 1 - AES_IV_BYTES;
					uchar tag[AES_IV_BYTES + 1];
					memset(tag, 0, sizeof(tag));
					memcpy(tag, buffer + inlen, AES_IV_BYTES);
					fprintf(stdout, "tag: %s, taglen: %d, inlen: %d\n", tag, strlen(tag), inlen);
					err = ev_aes_dec(buffer, &out, aes256_key, aes256_iv, inlen, &outlen, tag);
					fprintf(stdout, "dec err: %d, outlen: %d\n", err, outlen);
					if(err) {
						break;
					}
					if(!out) {
						break;
					}
					msg = (MSG_COMMON*) out;
					memset(buffer, 0, sizeof(buffer));
					memcpy(buffer, out, outlen);
					n = outlen;
					fprintf(stdout, "devid oooooooooooooooaes: %s\n", msg->dev_id);
				} while(0);
				if(out) {
					MY_FREE(out);
				}
			} else if( enc == 0) {
				fprintf(stdout, "HAVE NO ENC=================.\n");
			}

			dt = (MSG_DATA *)buffer;
			msg = (MSG_COMMON*) buffer;

			fprintf(stdout, "\n===========\nn: %d, ifroute: %d\n=================\n", n, msg->ifroute);
			if(msg->ifroute == G_NTF_CLI) {
				DUM_MSG(msg); 
				DUM_IPV4(&fbaddr);
				msg->ifroute = G_CLI_NTF;
				memset(msg->len, 0, LEN_U16INT);
				send_msg_fb(&servaddr, msg);
			}
			else if(msg->ifroute == F_SRV_CLI) {
				if(msg->type == MSG_GET_AES) {
					do {
						uchar *p = dt->data;
						uint16_t sz = 0;
						arr_2_uint16(msg->len, &sz, 2);
						fprintf(stdout, "data size: %d\n");
						if(sz != (AES_BYTES + AES_IV_BYTES) ) {
							break;
						}
						fprintf(stdout, "did get AES key\n");
						memcpy(aes256_key, p, AES_BYTES); 
						memcpy(aes256_iv, p + AES_BYTES, AES_IV_BYTES); 
						got_aes = 1;
					} while(0);
				}	
			}
			fprintf(stdout, "Did get a message devid: %s, n: %d\n.\n", msg->dev_id, n);
		}
		err = close(sockfd);
		if(err) {
			LOG(LOG_ERR, "Close socket err.");
		}
	} while(1);
	closelog();
	return 0;
}

int send_msg_fb(struct sockaddr_in* addr, MSG_COMMON *msg) {
	int n = 0;
	int len = sizeof(MSG_COMMON);
	int sk = 0;
	int err = 0;
	uchar buffer[MAX_MSG + 1];
	memset(buffer, 0, sizeof(buffer));
	if ( (sk = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	addr->sin_port = htons(PORT);
	DUM_IPV4(addr);
	do {
		uchar *out = 0;
		int outlen = 0;
		do {
			uchar tag[AES_IV_BYTES + 1];
			memset(tag, 0, sizeof(tag));
			err = ev_aes_enc((char*)msg, &out, aes256_key, aes256_iv, len, &outlen, tag);
			if(err) {
				fprintf(stdout, "\n\n\nenc AES256 error: \n\n\n");
				break;
			}
			if(!out) {
				break;
			}
			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, out, outlen);
			memcpy(buffer + outlen, tag, AES_IV_BYTES);
			outlen += AES_IV_BYTES;
			buffer[outlen] = ENCRYPT_AES;
			++outlen;
			len = outlen;
		} while(0);
		if(out) {
			MY_FREE(out);
		}
		if(err) {
			//LOG(LOG_ERR
		}	
	} while(0);
	n = sendto(sk, buffer, len,
		MSG_CONFIRM, (const struct sockaddr *) addr,
			sizeof(*addr));
	fprintf(stdout, "senttttttttt: %d\n", n);
	fprintf(stdout, "\n\nMUST use AES+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.\n\n");
	if(n < len) {
		LOG(LOG_ERR, "close socket err.");
	}
	DUM_MSG(msg);
	err = close(sk);
	if(err) {
		LOG(LOG_ERR, "close socket err.");
	}
	return n;
}

