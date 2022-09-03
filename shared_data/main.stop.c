#include <stdio.h>
#include <stdlib.h>
#include "shared_data.h"
LIST_SHARED_DATA *p = 0;
int main(int argc, char *argv[])
{	
	ntt_open_shm(0);
	p = (LIST_SHARED_DATA *) ntt_data_shm;
	if(!p)
	{
		exit (1);
	}
	set_exit_group(1);	
	ntt_unlink_shm();
	return 0;
}
