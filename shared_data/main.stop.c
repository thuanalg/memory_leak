#include <stdio.h>
#include <stdlib.h>
#include "shared_data.h"
LIST_SHARED_DATA *p = 0;
int main(int argc, char *argv[])
{	
	int val = 1;
	if(argv[1])
	{
		if(argv[1][0] != '1') {
			val = 0;
		}
	}
	ntt_open_shm(0);
	p = (LIST_SHARED_DATA *) ntt_data_shm;
	if(!p)
	{
		exit (1);
	}
	set_exit_group(val);	
	ntt_unlink_shm();
	return 0;
}
