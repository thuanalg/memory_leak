#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define PATH_MQ "/path_mq2"

#define USER_SIG SIGALRM
int isExit = 0;
int reg_user_sig();
void handler(int signo, siginfo_t *info, void *context)
{
	isExit = 1;
}


int reg_user_sig() {
	struct sigaction act = { 0 };	
	act.sa_flags = SA_SIGINFO | SA_ONSTACK;
	act.sa_sigaction = &handler;
	if (sigaction(USER_SIG, &act, NULL) == -1) {
	    perror("sigaction");
	    exit(EXIT_FAILURE);
	}
	return 0;	
}


void *body_thread(void * arg) {
	pthread_detach(pthread_self());
	sleep(10);
	kill(getpid(), SIGALRM);
	return 0;
}
int main(char argc, char *argv[])
{
	mqd_t mqds;
	int n = 0;
	pthread_t pid = 0;
	reg_user_sig();
	mqds=mq_open(PATH_MQ, O_RDWR , 0666, NULL);
	if(mqds==-1) {
		printf("open error\n");
		exit(1);
	}
	pthread_create(&pid, 0, body_thread, 0);
	while(!isExit)
	{
		for(int i = 0; (i < 1000 && !isExit); ++i) {
			mq_send(mqds,"1234",5,0);
			fprintf(stdout, "send\n");
			++n;
			//NOTE HERE, if we ignore sleep function, it can make our progress erroneous or not working.
			usleep(1000);
		};
		
	}
	fprintf(stdout, "---------------n: %d\n", n);
	sleep(1);
	mq_send(mqds,"012346789", 11, 0);
	mq_close(mqds);
	return 0;
}