#include <stdio.h>
#include "shared_data.h"
#include <string.h>
LIST_SHARED_DATA *p;
int main(int argc, char *argv[])
{
	int i = 0;
	SHARED_ITEM t;
	ntt_open_shm();
	p = ntt_data_shm;
	for(i = 0; i<10; ++i) {
		memset(&t, 0, sizeof(t));
		clock_gettime( CLOCK_REALTIME, &(t.timee));
		ntt_write_shm(p, (char*)&t, sizeof(t));
	}
	ntt_unlink_shm();
	return 0;
}
