//Nguyen Thai Thuan, modify later
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MY_MALLOC(k, btype, obj, objtype) {obj = (objtype *) malloc(k * sizeof(btype) + sizeof(int)); if(!obj) { llog("malloc error.\n");exit(1);} else {memset(obj, 0, (k * sizeof(btype) + sizeof(int))); obj->n = (k * sizeof(btype) + sizeof(int));}}
#define llog(fm, ...) fprintf(stdout, "%s:%d >>> "fm"", __FILE__, __LINE__, ##__VA_ARGS__)
//#define uint		unsigned int
//#define uchar		unsigned char
#define swap(A, B, tmp) {(tmp) = (A); (A) = (B); (B) = (tmp);}
typedef enum MY_ROLE__ {
	STRAIGHT = 0,
	HOLE_GAP = 1,

} MY_ROLE;
typedef struct __My_GENERIC__ {
	int n;
	char data[0];
} My_GENERIC;
typedef struct __My_bit_COUNT__ {
	int n;
	My_GENERIC* arr;
} My_bit_COUNT;
#define permutation_st  My_bit_COUNT
int init_rpermu(permutation_st* rcom, int n);
int init_rpermu(permutation_st* rcom, int n) {
	int err = 0;
	My_GENERIC* p = 0;
	do {
		if (!rcom) {
			err = 1; break;
		}
		memset(rcom, 0, sizeof(permutation_st));
		rcom->n = n;
		MY_MALLOC((rcom->n), int, p, My_GENERIC);
		rcom->arr = p;
		int* arr = (int *)rcom->arr->data;
		for (int i = 0; i < rcom->n; ++i) {
			arr[i] = i;
		}
	} while (0);
	return err;
}

int countt = 1;
int get_next_rpermu(permutation_st* p);
void dum_value(permutation_st* comr);
int main(int argc, char* argv[]) {
	int next = 1;
	permutation_st rcom;
	init_rpermu(&rcom, 4);
	while (next) {
			
		dum_value(&rcom);
		next = get_next_rpermu(&rcom);
	}
	return 0;
}


void dum_value(permutation_st* permu) {
	int i = 0;
	int* p = (int*) permu->arr->data;
	fprintf(stdout, "\ncount : %d====\t", countt);
	++countt;
	for (i = 0; i < permu->n; ++i) {
		fprintf(stdout, "%u, ", p[i]);
	}
	fprintf(stdout, "\n");
}
int get_next_rpermu(permutation_st* permu) {
	MY_ROLE mode = STRAIGHT;
	int i = 0;
	int next = 0;
	int* p = (int *) permu->arr->data;
	int tmp = 0;
	do {
		int t = 0;
		int u = 0;
		int v = 0;
		do {
			t = -1;
			u = -1;
			v = -1;
			for (i = permu->n - 1; i > 0; i--) {
				if (p[i] > p[i - 1]) {
					t = i - 1;
					u = i;
					v = permu->n - 1;
					break;
				}
			}
			if (t < 0) {
				break;
			}
			//Swap 
			for (i = permu->n - 1; i > 0; i--) {
				if (p[i] > p[t]) {
					swap(p[i], p[t], tmp)
					break;
				}
			}
			//Reverse
			i = 0;
			while ((u + i) < (v - i)) {
				swap(p[u + i], p[v - i], tmp);
				++i;
			}
			next = 1;
		} while (0);
	} while (0);

	return next;
}