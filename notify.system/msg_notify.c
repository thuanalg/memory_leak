#include "msg_notify.h"
#include <stdio.h>

int uint64_2_arr(unsigned char *arr, uint64_t n, int sz)
{
	int i = 0;
	do {
		if(!arr) break;
		for(i = 0; i < sz; ++i) {
			arr[i] = (n >> (i*8)) & 0xFF;
		}
	}
	while(0);

	return 0;
}

int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz)
{
	int i = 0;
	uint64_t t = 0;
	do {
		if(!arr) break;
		if(!n) break;
		*n = 0;
		for(i = 0; i < sz; ++i)
		{
			t = arr[i];
			(*n) |= (t << (i*8));
		}
	}
	while(0);
	return 0;
}
