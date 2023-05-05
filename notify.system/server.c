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
	
#define RECV_POST	 		9090
#define SEND_POST	 		9091
#define MAXLINE 			1024
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
	pthread_t ptid = 0;
	char *data = 0, buffer[MAXLINE+1];
	int rc = 0;
	struct sockaddr_in servaddr, cliaddr;
	int sockfd = 0;
	int n, err;
	int val = 1;
	int c = 0;
	struct timespec t0, t1;
	clock_gettime(CLOCK_REALTIME, &t0);

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
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
		
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
		
	
	while(1)
	{
		int rc = 0; 
		int i = 0;
		int len = 0;
		int zcom = (int) sizeof(MSG_COMMON);
		MSG_COMMON *msg = 0;
		memset(&cliaddr, 0, sizeof(cliaddr));
		memset(buffer, 0, sizeof(buffer));
		len = sizeof(cliaddr);	
		memset(buffer, 0, sizeof(buffer));
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
			MSG_DONTWAIT, ( struct sockaddr *) &cliaddr,	&len);
		//S2 socket
		if(n < 1)
		{
			if(!g_runnow) {
				usleep(1000 * 100);
			}
			g_runnow++;
			g_runnow %= 7;
		}
		while( n >= zcom) {
			msg = (MSG_COMMON*) buffer;
			//Here is notifying socket
			dum_msg(msg, __LINE__);
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
				dum_ipv4(&cliaddr, __LINE__);
			}
			break;
		}
		//Send immediate feedback list to a destination
		send_imd_fwd(sockfd, &imd_fwd_lt, &c, 1); 
	}
	err = close(sockfd);
	if(err)
	{
		fprintf(stdout, "close socket error.\n");
	}
	return 0;
}



pthread_t sending_thread(void *arg)
{
	pthread_t ptid = 0;
	int rc = 0;

	rc = pthread_create(&ptid, 0, sending_routine_thread, arg);
	fprintf(stdout, "f: %s, rc: %d, ptid: %llu\n", __FUNCTION__, rc, ptid);
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
	char *hello = "Hello from server";
	struct sockaddr_in servaddr, cliaddr;
	int val = 1;
	int count = 0;

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
		
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
		
	int len, n, err, signaling, rc;
	
	len = sizeof(cliaddr); //len is value/result
	while(!is_stop_server) {
		MSG_COMMON *msg = 0;
		//S1 socket 
		memset(buffer, 0, sizeof(buffer));
		n = recvfrom(sockfd, (char *)buffer, MAX_MSG,
					MSG_WAITALL, ( struct sockaddr *) &cliaddr,	&len);
		if(n < 1) {
			//Error here
			continue;
		}
		if( n < sizeof(MSG_COMMON)) {
			//Error here
			continue;
		}
		msg = (MSG_COMMON*) buffer;	
		dum_msg(msg, __LINE__);
		if(msg->ifroute == G_NTF_CLI || msg->ifroute == G_CLI_NTF) {
			if(msg->type == MSG_NTF) {
				int err = 0;
				uint16_t k = 0;
				char buf[MAX_MSG + 1];
				MSG_NOTIFY *p = (MSG_NOTIFY *) msg;
				memset(buf, 0, sizeof(buf));
				arr_2_uint16( msg->len, &k, 2);			
				fprintf(stdout, "file: %s, line: %d, len data: %u\n", __FILE__, __LINE__, k);
				fprintf(stdout, "file: %s, line: %d, data: %s\n", __FILE__, __LINE__, p->data);

				//Add to immediate forward list
				add_to_imd_fwd( p, &imd_fwd_lt, n);
				err = pthread_kill( read_threadid, USER_SIG);
				if(err)
				{	
					LOG(LOG_ERR, "signaling error.");
				}
			}
		}

		fprintf(stdout, "line:%d, recv n: %d\n", __LINE__, n);
		if(n > 0 && cliaddr.sin_family == AF_INET) {
			dum_ipv4(&cliaddr, __LINE__);
		}
	}	
	err = close(sockfd);
	if(err)
	{
		fprintf(stdout, "close fd error: %d", err);
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


