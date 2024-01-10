#include <stdlib.h>
#include <stdio.h>
#define llog(fm, ...) fprintf(stdout, "%s:%d >>> "fm"", __FILE__, __LINE__, ##__VA_ARGS__)
#define uint		unsigned int
#define uchar		unsigned char
typedef struct __My_GENERIC__ {
	uint n;
	char data[0];
} My_GENERIC;
typedef struct __My_bit_COUNT__{
	uint n;
	uint r;
	uint b;
	uint e;
	My_GENERIC* arr;
} My_bit_COUNT;
#define combination_st  My_bit_COUNT

#define MY_MALLOC(k, btype, obj, objtype) {obj = (objtype *) malloc(k * sizeof(btype) + sizeof(uint)); if(!obj) { llog("malloc error.\n");exit(1);} else {memset(obj, 0, (k * sizeof(btype) + sizeof(uint))); obj->n = (k * sizeof(btype) + sizeof(uint));}}

int get_next_rcom(uchar* arr, uint n, uint r);
int get_next_rcom(uchar* arr, uint n, uint r) {
	return 0;
}
int main(int argc, char* argv[]) {
	combination_st rcom;
	memset(&rcom, 0, sizeof(rcom));
	rcom.n = 8;
	rcom.r = 5;
	rcom.b = 0;
	rcom.e = 4;
	My_GENERIC* p = 0;
	MY_MALLOC(rcom.n, char, p, My_GENERIC);
	rcom.arr = p;
	return 0;
}