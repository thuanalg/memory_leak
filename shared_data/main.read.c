#include <stdio.h>
#include "shared_data.h"
#define COUNT_EXIT_READ 1 
int main(int argc, char *argv[])
{
	ntt_open_shm();
	ntt_read_thread();
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
