#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "shared_data.h"
#define COUNT_EXIT_READ 1 
pthread_t read_threadid = 0;
int main(int argc, char *argv[])
{
	ntt_open_shm();
	read_threadid = ntt_read_thread();
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
	ntt_unlink_shm();
	return 0;
}
