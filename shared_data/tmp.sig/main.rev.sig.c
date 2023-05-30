#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define USER_SIG SIGALRM
pid_t main_pid = 0;
int reg_user_sig();
void
handler(int signo, siginfo_t *info, void *context)
{
		if(main_pid != info->si_pid) {
			pthread_kill(1222, SIGALRM);
		}
		fprintf(stdout, "sending pid: %llu\n", (unsigned long long)info->si_pid);
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



int main(int argc, char *argv[])
{
	reg_user_sig();
	main_pid = getpid();
	while(1) {
		fprintf(stdout, "pid: %llu\n", (unsigned long long) getpid());
		sleep(1);
	}
	return EXIT_SUCCESS;
}

