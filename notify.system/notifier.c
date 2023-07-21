// Client side implementation of UDP client-server model
#include <stdio.h>
#include <string.h>
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
//char *id = "ed628094-63f3-452a-91c8-3ae24f281dd2";
//char *dev_id = "b7bb3690-ebcb-4bf9-88b0-31c130ec44a2";

int got_aes = 0;
uchar aes256_key[AES_BYTES];
uchar aes256_iv[AES_IV_BYTES];

void fier() {

}

#define _IP					"--target="	
#define _DEV_ID			"--dev_id="	
#define _NTF_ID			"--ntf_id="	
#define _PORT_ID		"--addport="	

char iip[20];
char dev_iid[40];
char ntf_iid[40];
int add_port;

void notifier(char *ip);


int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAXLINE + 1];
	char bufout[MAXLINE + 1];
	char *p = 0;
	struct timespec t0 = {0};
	struct timespec t1 = {0};
	struct sockaddr_in	 servaddr;
	struct sockaddr_in	 fbaddr;
	int val = 1;
	int count = 0;
	int err = 0;
	if(argc < 5 ) {
		fprintf(stdout, "add command line arguments: --target=127.0.0.1 --dev_id=b7bb3690-ebcb-4bf9-88b0-31c130ec44a2 --ntf_id=ed628094-63f3-452a-91c8-3ae24f281dd2 --addport=1\n");
		exit(EXIT_SUCCESS);
	}

	if(strlen(argv[1]) > strlen(_IP)) {
		memcpy(iip, argv[1] + strlen(_IP), strlen(argv[1]) - strlen(_IP));
		fprintf(stdout, "ip: %s\n", iip);
	}

	if(strlen(argv[2]) > strlen(_DEV_ID)) {
		memcpy(dev_iid, argv[2] + strlen(_DEV_ID), strlen(argv[2]) - strlen(_DEV_ID));
		fprintf(stdout, "dev_id: %s\n", dev_iid);
	}

	if(strlen(argv[3]) > strlen(_NTF_ID)) {
		memcpy(ntf_iid, argv[3] + strlen(_NTF_ID), strlen(argv[3]) - strlen(_NTF_ID));
		fprintf(stdout, "ntf_id: %s\n", ntf_iid);
	}

	if(strlen(argv[4]) > strlen(_PORT_ID)) {
		sscanf(argv[4] + strlen(_PORT_ID), "%d", &add_port);
		fprintf(stdout, "add_port: %d\n", add_port);
	}

	setlogmask (LOG_UPTO (LOG_INFO));
	openlog ("znotifier", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	// Creating socket file descriptor
	clock_gettime(CLOCK_REALTIME, &t0);
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
		llog(LOG_ERR, "%s", "Cannot create socket.");
		exit(EXIT_FAILURE);
	}
	setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));		
	if (!iip) {
		fprintf(stdout, "Address need entering as iip.\n");
		exit(EXIT_FAILURE);
	}
	if(INADDR_NONE == inet_addr(iip)) {
		fprintf(stdout, "Address need entering as iip.\n");
		exit(EXIT_FAILURE);
	}	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	//servaddr.sin_addr.s_addr = inet_addr(iip);
	servaddr.sin_port = htons(NTF_PORT + add_port);

	//We MUST bind  this socket to reuse	
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	//servaddr.sin_addr.s_addr = inet_addr(iip);
	//servaddr.sin_port = htons(PORT);
		
	int n = 0, len = sizeof(servaddr);
	send_msg_track(ntf_iid, sockfd, iip, PORT + 1, &t0, 0, 0);
	sleep(1);
	cmd_2_srv(MSG_GET_AES, G_NTF_SRV, 0, 0, (char*) ntf_iid, iip);
	//notifier(iip);
	while(1) {
		MSG_DATA *dt = 0;
		usleep(100 * 1000);
		clock_gettime(CLOCK_REALTIME, &t1);
		if(t1.tv_sec - t0.tv_sec > 3) {
			if(count > 30 && got_aes) {
			//if(1) {
				llog(LOG_ERR, "%s", "Cannot notify to CLI");
				break;
			}
			t0 = t1;
			if(got_aes) {
				send_msg_track(ntf_iid, sockfd, iip, PORT + 1, &t0, aes256_key, aes256_iv);
				notifier(iip);
			} else {
				send_msg_track(ntf_iid, sockfd, iip, PORT + 1, &t0, 0, 0);
				cmd_2_srv(MSG_GET_AES, G_NTF_SRV, 0, 0, (char*) ntf_iid, iip);
			}
			++count;
		}
		len = sizeof(fbaddr);
		memset(buffer, 0, sizeof(buffer));
		memset(&fbaddr, 0, sizeof(fbaddr));
		n = recvfrom(sockfd, (char *)buffer, sizeof(buffer),
					MSG_DONTWAIT, (struct sockaddr *) &fbaddr,
					&len);

		if(n < 1 ) {
			continue;
		}
		if(n < sizeof(MSG_COMMON)) {
			continue;
		}
		p = buffer;
		do {
			MSG_COMMON *msg = 0;
			uchar enc = 0;
			enc = buffer[n-1];
			fprintf(stdout, "recv -------: %d, enc: %d\n", n, enc);
			if(enc) {
				if(enc == ENCRYPT_CLI_PUB) {
					fprintf(stdout, "MUST dec by rsa private key.\n");		
					uchar *out = 0;
					int outlen = 0;
					RSA *key = 0;
					do {
						key = get_cli_prv((char*)ntf_iid);				
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
				} else if (enc == ENCRYPT_AES) {
					err = msg_aes_dec(buffer, bufout, aes256_key, aes256_iv, n, &n, MAX_MSG + 1); 
					if(err) {
						n = 0;
						continue;
					}
					p = bufout;
				}	
			} else {
				fprintf(stdout, "\n----NON BE encrypted----------\n");
			}
			dt = (MSG_DATA *) p;
			msg = (MSG_COMMON*) p;
			fprintf(stdout, "devid oooooooooooooooaes: %s\n", msg->dev_id);
			fprintf(stdout, "ntf oooooooooooooooaes: %s\n", msg->ntf_id);
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
						fprintf(stdout, "============================================did get AES key\n");
						memcpy(aes256_key, p, AES_BYTES); 
						memcpy(aes256_iv, p + AES_BYTES, AES_IV_BYTES); 
						got_aes = 1;
						notifier(iip);
					} while(0);
				}	
			}
			fprintf(stdout, "Did get  n: %d============\n.\n", n);
		} while(0);
	}
	close(sockfd);
	closelog();
	return 0;
}

void notifier(char *ip) {
	int sz = MAX_MSG + 1;
	struct sockaddr_in	 servaddr;
	//char data[MAX_DATA]
	MSG_NOTIFY *msg = 0;
	int n = 0;
	struct timespec t;
	char buf [2];
	int sockfd = 0;
	int err = 0;
	char buffer[MAX_MSG + 1];
	char bufout[MAX_MSG + 1];
	uchar *p = 0;
	do {
		memset(buffer, 0, sizeof(buffer));
		memset(bufout, 0, sizeof(bufout));
		clock_gettime(CLOCK_REALTIME, &t);
		if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
			llog(LOG_ERR, "%s", "Cannot create socket.");
			exit(EXIT_FAILURE);
		}
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inet_addr(ip);
		servaddr.sin_port = htons(PORT);
	
		msg = (MSG_NOTIFY *) buffer;
	
		msg->com.type = MSG_NTF;
		msg->com.ifroute = G_NTF_CLI;
		memcpy(msg->com.dev_id, dev_iid, MIN(LEN_DEVID, strlen(dev_iid) + 1));
		memcpy(msg->com.ntf_id, ntf_iid, MIN(LEN_DEVID, strlen(ntf_iid) + 1));
		n = MAX_MSG - sizeof(MSG_COMMON) - 1 - AES_IV_BYTES;
		memset(buf, 0, sizeof(buf));
		uint16_2_arr(buf, n, 2);
		memcpy(msg->com.len, buf, 2);
		snprintf(msg->data, n, "%s", "Nguyen Thai Thuan, Thanh Hong, Thanh Tam.");
		uint64_2_arr(msg->com.second, t.tv_sec, 8);
		uint64_2_arr(msg->com.nano, t.tv_nsec, 8);
	
		DUM_MSG(&(msg->com));
	
		p = (uchar*) msg;
		if(got_aes) {
			//nttthuan
			err = msg_aes_enc(buffer, bufout, aes256_key, aes256_iv, 
					MAX_MSG - AES_IV_BYTES, &n, MAX_MSG + 1);  
			if(err) {
				break;
			}
			p = bufout;
			sz = n;
		} else {
			p[MAX_MSG] = ENCRYPT_NON;
		}
	
		n = sendto(sockfd, p, sz,
			MSG_CONFIRM, (const struct sockaddr *) &servaddr,
				sizeof(servaddr));
	} while(0);
	err = close(sockfd);
	if(err) {
		llog(LOG_ERR, "%s", "Close socket error.");
	}
}
// Driver code

