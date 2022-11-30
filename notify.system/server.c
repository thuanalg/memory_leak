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



int reg_user_sig();
void
handler(int signo, siginfo_t *info, void *context)
{
		if(main_pid != info->si_pid) {
			pthread_kill(read_threadid, USER_SIG);
		}
		fprintf(stdout, "sending pid: %llu\n", (unsigned long long)info->si_pid);

		is_stop_server = 1;
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
	int len;
	struct sockaddr_in addr;
	char data[MAXLINE];
} list_feedback;

void *sending_routine_thread(void *arg)
{
	pthread_t ptid = 0;
	int rc = 0;
	struct sockaddr_in servaddr, cliaddr;
	int sockfd = 0;
	int len, n, err;

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
		
	
	len = sizeof(cliaddr); //len is value/result
	while(1)
	{
		int rc = 1; 
		if(rc) {
			sleep(60);
		}
	}
	return 0;
}



pthread_t sending_thread(void *arg)
{
	pthread_t ptid = 0;
	int rc = 0;

	rc = pthread_create(&ptid, 0, sending_routine_thread, arg);
	fprintf(stdout, "f: %s, rc: %d, ptid: %llu\n", __FUNCTION__, rc, ptid);
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
		buffer[n] = '\0';
		printf("Client : %s\n", buffer);
		sendto(sockfd, (const char *)hello, strlen(hello),
			MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
		printf("Hello message sent.\n");
	}	
	err = close(sockfd);
	if(err)
	{
		fprintf(stdout, "close fd error: %d", err);
	}	
	return 0;
}
