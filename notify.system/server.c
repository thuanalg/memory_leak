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


void dum_msg(MSG_COMMON *);
int handle_tracking_msg(char*);
int reg_user_sig();

//The tracking message is used to update the path ( route) is from server to client. 
//It includes IP and high client port.
int handle_tracking_msg(char *buf)
{
	//ntthuan need to be done
	return 0;
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
		fprintf(stdout, "sending pid--------------: %llu\n", (unsigned long long)info->si_pid);
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
	char *data = 0, buffer[MAXLINE+1];
	int rc = 0;
	struct sockaddr_in servaddr, cliaddr;
	int sockfd = 0;
	int n, err;

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) {
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
		int rc = 0; 
		int i = 0;
		int len = 0;
		MSG_COMMON *msg = 0;
		memset(&cliaddr, 0, sizeof(cliaddr));
		memset(buffer, 0, sizeof(buffer));
		len = sizeof(cliaddr);	
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_DONTWAIT, ( struct sockaddr *) &cliaddr,	&len);
		while(n > 0) {
			if(n >= sizeof(MSG_COMMON));
			{
				msg = (MSG_COMMON*) buffer;
				//Here is notifying socket
				dum_msg(msg);
			}
			int err = handle_tracking_msg(buffer);
			if(err) {
				//LOG err
				break;
			}
			
			//LOG OK

			if(n>0 && cliaddr.sin_family == AF_INET) {
				char str[INET_ADDRSTRLEN + 1];
				str[INET_ADDRSTRLEN] = 0;
				inet_ntop(AF_INET, &(cliaddr.sin_addr), str, INET_ADDRSTRLEN);
				fprintf(stdout, "\nline: %d, cli port: %d, \n", __LINE__, (int) cliaddr.sin_port);
				fprintf(stdout, "\nline: %d, cli IP: %s\n\n", __LINE__, str);
			}
			break;
		}
		rc = get_data_gen_list(gen_list, &data); 
		fprintf(stdout, "line: %d, send to client, rc: %d.\t\n", __LINE__, rc);
		if(!rc) {
			sleep(10);
			continue;
		}
		n = rc/sizeof(item_feedback);	
		item_feedback *item = (item_feedback*) data;
		for(i = 0; i < n; ++i)
		{
			fprintf(stdout, "send to client data: %s.\t\n", (char*)item[i].data);
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
		MSG_COMMON *msg = 0;
		//Register socket, recvfrom
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
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
		dum_msg(msg);
		fprintf(stdout, "line:%d, recv n: %d\n", __LINE__, n);
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

void dum_msg(MSG_COMMON *item)
{
	const char *text[] = { "Register", "Trace", "Notify", "Confirm", "" };
	do {
		unsigned char type = 0;
		if(!item) break;
		type = item->type;
		fprintf(stdout, "-------- Type of message: %s\n", text[type]);
		fprintf(stdout, "-------- Device ID: %s\n", item->dev_id);
		fprintf(stdout, "-------- Hash number: %u\n", hash_func(item->dev_id, 64));
	} while(0);	
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


