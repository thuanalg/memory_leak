// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "gen_list.h"
#include "msg_notify.h"
	
#define RECV_POST	 		PORT
#define SEND_POST	 		(PORT + 1)
#define MAXLINE 			MAX_MSG
#define USER_SIG 			SIGALRM


//b1142898-ca9b-425f-ba2e-407a2afe128c
int reg_user_sig();
static pthread_mutex_t iswaiting_mtx = PTHREAD_MUTEX_INITIALIZER; 
int set_waiting(int w);
int get_waiting(int *ret);

int g_waiting = 1;
char g_runnow = 0;

int set_waiting(int w) {
	int err = 0;
	int rc = 0;

	rc = pthread_mutex_lock(&iswaiting_mtx);
	if(rc) {
		//LOG FATAL
		err = rc;
	}
	//>>>	
	g_waiting = w ? 1 : 0;
	//<<<	
	rc = pthread_mutex_unlock(&iswaiting_mtx);
	if(rc) {
		//LOG FATAL
		err = rc;
	}
	return err;
}

int get_waiting(int *ret) {
	int err = 0;
	int rc = 0;

	rc = pthread_mutex_lock(&iswaiting_mtx);
	if(rc) {
		//LOG FATAL
		err = rc;
	}
	//>>>	
	do {
		if(!ret) {
			//LOG err
			err = 1;
			*ret = g_waiting;
		}
		*ret = g_waiting;
	} while(0);
	//<<<	
	rc = pthread_mutex_unlock(&iswaiting_mtx);
	if(rc) {
		//LOG FATAL
		err = rc;
	}
	return err;
}


#define COUNT_EXIT_READ 1 
pthread_t read_threadid = 0;
pid_t main_pid = 0;
char is_stop_server = 0;
pthread_t sending_thread(void *arg);
void *sending_routine_thread(void *arg);

GEN_LIST *gen_list = 0;

void handler(int signo, siginfo_t *info, void *context)
{
		if(main_pid != info->si_pid) {
			pthread_kill(read_threadid, USER_SIG);
		}
		if(pthread_self() == read_threadid) {
			g_runnow = 1;
		}
		//set_waiting(0);
		LOG(LOG_INFO, "sending pid--------------: %llu\n", (unsigned long long)info->si_pid);
}


//typedef struct {
//	int len_addr;
//	int len_buff;
//	struct sockaddr_in addr;
//	char data[MAXLINE + 1];
//} item_feedback;

void *sending_routine_thread(void *arg)
{
	char  buffer[MAX_MSG+1];
	char  bufout[MAX_MSG+1];
	char *p = 0;
	struct sockaddr_in servaddr, cliaddr;
	int sockfd = 0;
	int n, err;
	int val = 1;
	int c = 0;
	struct timespec t0;
	clock_gettime(CLOCK_REALTIME, &t0);

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
	//if ( (sockfd = socket(AF_INET, SOCK_DGRAM , 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));		
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(SEND_POST);
		
	int on = 1;
	err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	if(err) {
			perror("setsockopt");
			exit(1);
	}
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
		
	
	while(1)
	{
		int len = 0;
		//int zcom = (int) sizeof(MSG_COMMON);
		MSG_COMMON *msg = 0;
		memset(&cliaddr, 0, sizeof(cliaddr));
		memset(buffer, 0, sizeof(buffer));
		len = sizeof(cliaddr);	
		memset(buffer, 0, sizeof(buffer));
		n = recvfrom(sockfd, (char *)buffer, MAX_MSG,
			MSG_DONTWAIT, ( struct sockaddr *) &cliaddr,	&len);
		//S2 socket
		if(n < 1)
		{
			if(!g_runnow) {
				usleep(1000 * 1000);
				//sleep(10);
			}
			g_runnow++;
			g_runnow %= 3;
		}
		do {
			if(n < sizeof(MSG_COMMON)) {
				break;
			}
			p = buffer;
			if(buffer[n-1] == ENCRYPT_SRV_PUB) {
				int len = 0;
				uchar *out = 0;
				RSA *prv = get_srv_prv();
				if(!prv) {
					LOG(LOG_ERR, "cannot get server private key.");
					continue;	
				}
				rsa_dec(prv, buffer, &out, n - 1, &len);
				if(out) {
					n = len;
					memset(buffer, 0, sizeof(buffer));
					memcpy(buffer, out, len);
					MY_FREE(out);
					msg = (MSG_COMMON*) buffer;
				}
			} else if(buffer[n-1] == ENCRYPT_AES) {
				//thuannt 03
				err = msg_aes_dec(buffer, bufout, aes_key, aes_iv, n, &n, MAX_MSG + 1); 
				if(err) {
					n = 0;
					continue;
				}
				p = bufout;
			}
			msg = (MSG_COMMON*) p;
			//S2
			if(msg->type == MSG_TRA) {
				int done = hl_track_msg((MSG_TRACKING *)msg, n, &cliaddr, 0);
				if(!done) {
					LOG(LOG_ERR, "Handle tracking error.");
					break;
				}
			}
			else if(msg->type == MSG_TRA) {
			}
			else {
				//Notify immediately
				//ntthuan NOT DONE
			}
			//LOG OK
			if(n > 0 && cliaddr.sin_family == AF_INET) {
				DUM_IPV4(&cliaddr);
			}
			break;
		} while(0);
		//Send immediate feedback list to a destination
		send_imd_fwd(sockfd, &imd_fwd_lt, &c, 1); 
	}
	err = close(sockfd);
	if(err)
	{
		LOG(LOG_ERR, "close socket error.\n");
	}
	return 0;
}



pthread_t sending_thread(void *arg)
{
	pthread_t ptid = 0;
	int rc = 0;

	rc = pthread_create(&ptid, 0, sending_routine_thread, arg);
	if(!rc) {
		read_threadid = ptid;
	}
	return rc ? 0 : ptid;
}

void server() {

}	
// Driver code
int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAX_MSG + 1];
	char bufout[MAX_MSG + 1];
	char *pglobal = 0;
	struct sockaddr_in servaddr, cliaddr;
	int val = 1;
	int count = 0;
	int len, n, err;

	setlogmask (LOG_UPTO (LOG_INFO));
	openlog ("zserver_notify", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	main_pid = getpid();
		
	reg_user_sig();
	
	count = load_reg_list();
	fprintf(stdout, "count: %d\n", count);

	sending_thread(0);

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
		
	setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));		

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(RECV_POST);
		
	int on = 1;
	err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	if(err) {
			perror("setsockopt");
			exit(1);
	}
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
		
	
	len = sizeof(cliaddr); //len is value/result
	while(!is_stop_server) {
		MSG_COMMON *msg = 0;
		uchar enc = 0;
		//S1 socket 
		memset(buffer, 0, sizeof(buffer));
		n = recvfrom(sockfd, (char *)buffer, MAX_MSG + 1,
					MSG_DONTWAIT, ( struct sockaddr *) &cliaddr,	&len);

		if(n < 1) {
			//Error here
			usleep(100 * 1000);
			continue;
		}
		if( n < sizeof(MSG_COMMON)) {
			//Error here
			continue;
		}
		pglobal = buffer;
		enc = buffer[n-1];
		fprintf(stdout, "+++++++++n: %d, has encrypt: %s\n", n, buffer[n-1] ? "YES" : "NO");
		if(enc == ENCRYPT_SRV_PUB) {
			int len = 0;
			uchar *out = 0;
			RSA *prv = get_srv_prv();
			if(!prv) {
				LOG(LOG_ERR, "S1 cannot get server private key.");
				continue;	
			}
			rsa_dec(prv, buffer, &out, n - 1, &len);
			fprintf(stdout, "S1 LET use RSA dec, rsa private------: %p.\n", prv);
			if(out) {
				n = len;
				memset(buffer, 0, sizeof(buffer));
				fprintf(stdout, "len: %d\n", len);
				memcpy(buffer, out, len);
				MY_FREE(out);
				msg = (MSG_COMMON*) buffer;
				fprintf(stdout, "S1 devid: %s\n", msg->dev_id);
			}
		} else if(enc == ENCRYPT_AES) {
			err = msg_aes_dec(buffer, bufout, 
					aes_key, aes_iv, n, &n, MAX_MSG + 1); 
			if(err) {
				n = 0;
				continue;
			}
			pglobal = bufout;
		}
		msg = (MSG_COMMON*) pglobal;	
		DUM_MSG(msg);
		if(msg->ifroute == G_NTF_CLI || msg->ifroute == G_CLI_NTF) {
			if(msg->type == MSG_NTF) {
				int err = 0;
				//uint16_t k = 0;
				//char buf[MAX_MSG + 1];
				MSG_NOTIFY *p = (MSG_NOTIFY *) msg;
				//memset(buf, 0, sizeof(buf));
				//arr_2_uint16( msg->len, &k, 2);			
				//Add to immediate forward list
				add_to_imd_fwd( p, &imd_fwd_lt, n);
				err = pthread_kill( read_threadid, USER_SIG);
				if(err) {	
					LOG(LOG_ERR, "signaling error.");
				}
			}
			
		} else if(msg->ifroute == G_CLI_SRV) {
			int err = 0;
			//uint16_t k = 0;
			//char buf[MAX_MSG + 1];
			MSG_NOTIFY *p = (MSG_NOTIFY *) msg;
			p->com.ifroute = F_SRV_CLI;
			//memset(buf, 0, sizeof(buf));
			uint16_2_arr( msg->len, AES_BYTES + AES_IV_BYTES, 2);			
			//Add to immediate forward list
			memcpy(pglobal + n, aes_key, AES_BYTES); 
			memcpy(pglobal + n + AES_BYTES, aes_iv, AES_IV_BYTES); 
			n += AES_BYTES + AES_IV_BYTES;
			fprintf(stdout, "ADDD to list FW______: %s, n: %d\n", msg->dev_id, n);
			add_to_imd_fwd( p, &imd_fwd_lt, n);
			err = pthread_kill( read_threadid, USER_SIG);
			if(err) {	
				LOG(LOG_ERR, "signaling error.");
			}
		} else if(msg->ifroute == G_NTF_SRV) {
			int err = 0;
			//uint16_t k = 0;
			//char buf[MAX_MSG + 1];
			MSG_NOTIFY *p = (MSG_NOTIFY *) msg;
			p->com.ifroute = F_SRV_NTF;
			//memset(buf, 0, sizeof(buf));
			uint16_2_arr( msg->len, AES_BYTES + AES_IV_BYTES, 2);			
			//Add to immediate forward list
			memcpy(pglobal + n, aes_key, AES_BYTES); 
			memcpy(pglobal + n + AES_BYTES, aes_iv, AES_IV_BYTES); 
			n += AES_BYTES + AES_IV_BYTES;
			fprintf(stdout, "ADDD to list FW_____ fROM NTF to SRV_: %s, n: %d\n", msg->dev_id, n);
			add_to_imd_fwd( p, &imd_fwd_lt, n);
			err = pthread_kill( read_threadid, USER_SIG);
			if(err) {	
				LOG(LOG_ERR, "signaling error.");
			}
		}
		if(n > 0 && cliaddr.sin_family == AF_INET) {
			DUM_IPV4(&cliaddr);
		}
	}	
	err = close(sockfd);
	if(err)
	{
		LOG(LOG_ERR, "close fd error: %d", err);
	}	
	sleep(1);
	closelog ();
	return 0;
}


int reg_user_sig() {
	struct sigaction act = { 0 };	
	act.sa_flags = SA_SIGINFO | SA_ONSTACK;
	act.sa_sigaction = &handler;
	if (sigaction(USER_SIG, &act, NULL) == -1) {
	    perror("sigaction");
	    exit(EXIT_FAILURE);
	}		
}


