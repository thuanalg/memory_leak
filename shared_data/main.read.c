#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "shared_data.h"
#include <signal.h>

#define COUNT_EXIT_READ 1 
pthread_t read_threadid = 0;
pid_t main_pid = 0;


#define USER_SIG SIGALRM
int reg_user_sig();
void
handler(int signo, siginfo_t *info, void *context)
{
		int sval = 0;
		union  sigval val;
		val = info->si_value;
		sval = val.sival_int;
		fprintf(stdout, "si_value: %d.\n", sval);
		//sval = 0: stop
		//sval = 1; read
		//sval = 2; local wake up
		if(main_pid != info->si_pid) {
			pthread_kill(read_threadid, USER_SIG);
		}
		fprintf(stdout, "sending pid: %llu\n", (unsigned long long)info->si_pid);
}


int reg_user_sig() {
	struct sigaction act = { 0 };	
	
	//sigqueue
	act.sa_flags = SA_SIGINFO | SA_ONSTACK;
	act.sa_sigaction = &handler;
	if (sigaction(USER_SIG, &act, NULL) == -1) {
	    perror("sigaction");
	    exit(EXIT_FAILURE);
	}		
}


int main(int argc, char *argv[])
{
	ntt_open_shm();
	main_pid = getpid();
	read_threadid = ntt_read_thread();
	reg_user_sig();
	if(!read_threadid) {
		return EXIT_FAILURE;
	}
	while(1)
	{
		int n = 0;
		sleep(1);
		n = check_exiit(0);
		if(n >= COUNT_EXIT_READ) break;	
	}

	sleep(60);
	ntt_unlink_shm();
	return 0;
}
