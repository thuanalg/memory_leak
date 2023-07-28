#include <stdio.h>
#include <stdlib.h>
#include "shared_data.h"
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

LIST_SHARED_DATA *p;
#define COUNT_EXIT_READ 1 

int main(int argc, char *argv[])
{
	int i = 0;
	char sendsig = 0;
	pid_t read_pid = 0;
	ntt_daemonize();
	SHARED_ITEM t;
	setlogmask (LOG_UPTO (LOG_INFO));
	openlog ("main.write", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	ntt_open_shm(LIST_SHARED_DATA_SZ);
	p = ntt_data_shm;
	while(1) {
		int n = 0;

		for(i = 0; i < 10000; ++i) {
			char tmp1 = 0;
			pid_t tmp2 = 0;
			sendsig = 0;
			read_pid = 0;
			memset(&t, 0, sizeof(t));
			clock_gettime( CLOCK_REALTIME, &(t.timee));
			ntt_write_shm(p, (char*)&t, sizeof(t), &sendsig, &read_pid);
			//syslog(LOG_INFO, "sendsig: %d, read_pid: %llu\n", (int) sendsig, (unsigned long long)read_pid);
		}
		
		llog(LOG_INFO, "sendsig: %d, read_pid: %llu, i = %d\n", 
			(int) sendsig, (unsigned long long)read_pid, i);

		if(sendsig && read_pid)
		{
			union sigval sv;
			sv.sival_int = 1;
			sigqueue(read_pid, SIGALRM, sv);
			//kill(read_pid, SIGALRM);	
		}

		n = check_exiit(0);
		if(n >= COUNT_EXIT_READ) { 
			break;			
		}
		usleep(100000);
		//sleep(1);
	}
	llog(LOG_INFO, "%s", "Exit write process.");
	ntt_unlink_shm(LIST_SHARED_DATA_SZ);
	closelog();
	return 0;
}
