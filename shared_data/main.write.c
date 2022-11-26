#include <stdio.h>
#include "shared_data.h"
#include <string.h>
#include <signal.h>

LIST_SHARED_DATA *p;



int main(int argc, char *argv[])
{
	int i = 0;
	char sendsig = 0;
	pid_t read_pid = 0;
	SHARED_ITEM t;
	ntt_open_shm();
	p = ntt_data_shm;
	for(i = 0; i<10; ++i) {
		memset(&t, 0, sizeof(t));
		clock_gettime( CLOCK_REALTIME, &(t.timee));
		ntt_write_shm(p, (char*)&t, sizeof(t), &sendsig, &read_pid);
		fprintf(stdout, "sendsig: %d, read_pid: %llu\n", (int) sendsig, (unsigned long long)read_pid);
	}

	if(sendsig && read_pid)
	{
		kill(read_pid, SIGALRM);	
	}
	ntt_unlink_shm();
	return 0;
}
