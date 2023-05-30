#include <stdio.h>
#include <stdlib.h>
#include "shared_data.h"
LIST_SHARED_DATA *p = 0;
#include <signal.h>
int main(int argc, char *argv[])
{	
	int val = 1;
	pid_t read_pid = 0;
	if(argv[1])
	{
		if(argv[1][0] != '1') {
			val = 0;
		}
	}
	ntt_open_shm(LIST_SHARED_DATA_SZ);
	p = (LIST_SHARED_DATA *) ntt_data_shm;
	if(!p)
	{
		exit (1);
	}
	set_exit_group(val);	
	read_pid = get_read_pid();
	if(read_pid) {
		fprintf(stdout, "readid: %llu\n", (unsigned long long) read_pid);
		kill(read_pid, SIGALRM);
	}
	ntt_unlink_shm(LIST_SHARED_DATA_SZ);
	
	return 0;
}
