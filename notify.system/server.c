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
#include "gen_list.h"
	
#define RECV_POST	 		9090
#define SEND_POST	 		9091
#define MAXLINE 			1024
#define USER_SIG 			SIGALRM



#define COUNT_EXIT_READ 1 
pthread_t read_threadid = 0;
pid_t main_pid = 0;
char is_stop_server = 0;
pthread_t sending_thread(void *arg);
void *sending_routine_thread(void *arg);

GEN_LIST *gen_list = 0;

int reg_user_sig();
void
handler(int signo, siginfo_t *info, void *context)
{
		if(main_pid != info->si_pid) {
			pthread_kill(read_threadid, USER_SIG);
		}
		fprintf(stdout, "sending pid--------------: %llu\n", (unsigned long long)info->si_pid);
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

typedef struct {
	int len_addr;
	int len_buff;
	struct sockaddr_in addr;
	char data[MAXLINE + 1];
} item_feedback;

void *sending_routine_thread(void *arg)
{
	pthread_t ptid = 0;
	char *data = 0;
	int rc = 0;
	struct sockaddr_in servaddr, cliaddr;
	int sockfd = 0;
	int n, err;

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
		
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
		int rc = get_data_gen_list(gen_list, &data); 
		fprintf(stdout, "send to client, rc: %d.\t\n", rc);
		if(!rc) {
			sleep(60);
			continue;
		}
		n = rc/sizeof(item_feedback);	
		int i = 0;
		item_feedback *item = (item_feedback*) data;
		fprintf(stdout, "send to client.\t\n");
		for(i = 0; i < n; ++i)
		{
			sendto(sockfd, (const char *)item[i].data, item[i].len_buff,
				MSG_CONFIRM, (const struct sockaddr *) &(item[i].addr), item[i].len_addr);
		}
		if(data) {
			free(data);
		}
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


	
// Driver code
int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[MAXLINE];
	char *hello = "Hello from server";
	struct sockaddr_in servaddr, cliaddr;

	main_pid = getpid();
		
	reg_user_sig();

	sending_thread(0);

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
		
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
		
	int len, n, err;
	
	len = sizeof(cliaddr); //len is value/result
	while(!is_stop_server) {
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_WAITALL, ( struct sockaddr *) &cliaddr,	&len);
		if(n < 1) {
			continue;
		}

		if(n>0 && cliaddr.sin_family == AF_INET) {
			char str[INET_ADDRSTRLEN + 1];
			str[INET_ADDRSTRLEN] = 0;
			inet_ntop(AF_INET, &(cliaddr.sin_addr), str, INET_ADDRSTRLEN);
			fprintf(stdout, "\ncli port: %d, ", (int) cliaddr.sin_port);
			fprintf(stdout, "cli IP: %s\n\n", str);
		}
		item_feedback item;
		memset(&item, 0, sizeof(item));
		item.len_addr = len;
		item.len_buff = n;
		memcpy(&(item.addr), &cliaddr, len);
		
		buffer[n] = '\0';
		printf("Client : %s\n", buffer);
		int sig = 0;
		memcpy(item.data, buffer, n);
		add_item_gen_list(&gen_list, (char*) &item, sizeof(item), &sig);

		fprintf(stdout, "Client pid: %llu, sig: %d\n", read_threadid, sig);
		if(sig) {
			int err = pthread_kill( read_threadid, USER_SIG);
			if(err)
			{

			}
		}
	}	
	err = close(sockfd);
	if(err)
	{
		fprintf(stdout, "close fd error: %d", err);
	}	
	sleep(1);
	return 0;
}
