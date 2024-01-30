//Nguyen Thai Thuan, modify later
#include <stdlib.h>
#include <stdio.h>

#define llog(fm, ...) fprintf(stdout, "%s:%d >>> "fm"", __FILE__, __LINE__, ##__VA_ARGS__)
#define MY_MALLOC(k, btype, obj, objtype) {obj = (objtype *) malloc(k * sizeof(btype) + sizeof(uint)); if(!obj) { llog("malloc error.\n");exit(1);} else {llog("malloc error.\n");memset(obj, 0, (k * sizeof(btype) + sizeof(uint))); obj->n = (k * sizeof(btype) + sizeof(uint));}}
#define MY_FREE(obj) if(obj) { llog("free :0x%p.\n", obj); free(obj); (obj) = 0;} 

#define uint		unsigned int
#define uchar		unsigned char
typedef enum MY_ROLE__ {
	STRAIGHT = 0,
	HOLE_GAP = 1,

} MY_ROLE;
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
int init_rcom(combination_st* rcom, uint n, uint r);
int init_rcom(combination_st* rcom, uint n, uint r) {
	int err = 0;
	My_GENERIC* p = 0;
	do {
		if (!rcom) {
			err = 1; break;
		}
		memset(rcom, 0, sizeof(combination_st));
		rcom->n = n;
		rcom->r = r;
		rcom->b = 0;
		rcom->e = rcom->r - 1;
		MY_MALLOC((rcom->n), char, p, My_GENERIC);
		rcom->arr = p;
		char* arr = rcom->arr->data;
		for (int i = rcom->b; i <= rcom->e; ++i) {
			arr[i] = 1;
		}
	} while (0);
	MY_FREE(rcom->arr);
	return err;
}

int countt = 1;
int get_next_rcom(combination_st* p);
void dum_value(combination_st* comr);
int main(int argc, char* argv[]) {
	int next = 1;
	combination_st rcom;
	init_rcom(&rcom, 6, 3);
	while (next) {
		dum_value(&rcom);
		next = get_next_rcom(&rcom);
	}
	return 0;
}


void dum_value(combination_st* comr) {
	uint i = comr->b;
	char* p = comr->arr->data;
	fprintf(stdout, "\ncount : %d====\t", countt );
	++countt;
	for (; i <= comr->e; ++i) {
		if (p[i]) {
			fprintf(stdout, "%u, ", i);
		}
	}
	fprintf(stdout, "\n");
}
int get_next_rcom(combination_st* comr) {
	MY_ROLE mode = STRAIGHT;
	uint i = 0;
	uint j = 0;
	int next = 1;
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
			next = 0;
			break;
		}
		if (mode == STRAIGHT) {
			for (i = 0; i < comr->n; ++i) {
				p[i] = 0;
			}
			comr->b = 0;
			++(comr->e);
			p[comr->b] = 0;
			for (i = 0; i < comr->r - 1; ++i) {
				p[i] = 1;
			}
			p[comr->e] = 1;
			break;
		}
		if (mode == HOLE_GAP) {
			p[i] = 1;
			p[i-1] = 0;
			if(i - 1 == comr->b) {
				comr->b = i;
			}
			else if (i - 1 > comr->b) {
				uint range = i - 1 - comr->b;
				uint j = 0;
				uint  k = i - 1 - comr->b;
				for (j = 0; j < range; ++j) {
					p[j] = 1;
				}
				for (j = range; j <=  i -1; ++j) {
					p[j] = 0;
				}
				comr->b = 0;
			}
			break;
		}
	} while (0);	

	return next;
}