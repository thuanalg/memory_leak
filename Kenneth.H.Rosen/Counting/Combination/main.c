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

int get_next_rcom(combination_st* p);

int main(int argc, char* argv[]) {
	combination_st rcom;
	memset(&rcom, 0, sizeof(rcom));
	rcom.n = 8;
	rcom.r = 5;
	rcom.b = 0;
	rcom.e = rcom.r - 1;
	My_GENERIC* p = 0;
	MY_MALLOC(rcom.n, char, p, My_GENERIC);
	rcom.arr = p;
	char* arr = rcom.arr->data;
	for (int i = rcom.b; i <= rcom.e; ++i) {
		arr[i] = 1;
	}
	get_next_rcom(&rcom);
	return 0;
}
typedef enum MY_ROLE__{
	STRAIGHT = 0,
	HOLE_GAP = 1,

} MY_ROLE;
int get_next_rcom(combination_st* comr) {
	MY_ROLE mode = STRAIGHT;
	uint i = 0;
	uint j = 0;
	char* p = comr->arr->data;
	for (i = comr->b; i <= comr->e; ++i) {
		if (!p[i]) {
			j = i;
			mode = HOLE_GAP;
			break;
		}
	}
	do {
		if (mode == STRAIGHT && (comr->e >= comr->n - 1)) {
			break;
		}
		if (mode == STRAIGHT) {
			for (i = 0; i < comr->n; ++i) {
				p[i] = 0;
			}
			comr->b = 0;
			++(comr->e);
			p[comr->e] = 1;
			p[comr->b] = 0;
			for (i = 0; i < comr->r - 1; ++i) {
				p[i] = 1;
			}
			break;
		}
		if (mode == HOLE_GAP) {
			p[i] = 1;
			p[i-1] = 0;
			break;
		}
	} while (0);
	return 0;
}