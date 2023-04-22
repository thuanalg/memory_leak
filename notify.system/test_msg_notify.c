#include <stdio.h>
#include <stdlib.h>
#include "msg_notify.h"
int main(int argc, char *argv[])
{

	//int uint64_2_arr(unsigned char *arr, uint64_t n, int sz)
	int i = 0;
	uint64_t a = 0x112233446677;
	uint64_t b = 0;
	char arr[8];
	uint64_2_arr(arr, a, 8);
	fprintf(stdout, "\narr: \n");
	for(i = 0; i < 8; ++i)
	{
		int j = 0;
		for(j = 0; j < i; ++j)
		{
			fprintf(stdout, "\t");
		}
		fprintf(stdout, "%x\n", arr[i]);
	}
	fprintf(stdout, "a: %lx\n", a);

	//int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz)
	arr_2_uint64(arr, &b, 8);
	fprintf(stdout, "b: %lx\n", b);
	return 0;
}
