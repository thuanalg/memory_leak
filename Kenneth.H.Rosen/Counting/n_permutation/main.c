//Nguyen Thai Thuan, modify later
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define llog(fm, ...) fprintf(stdout, "%s:%d >>> "fm"", __FILE__, __LINE__, ##__VA_ARGS__)
#define MY_MALLOC(k, btype, obj, objtype) {obj = (objtype *) malloc(k * sizeof(btype) + sizeof(int)); if(!obj) { llog("malloc error.\n");exit(1);} else { llog("malloc 0x:%p.\n", obj);memset(obj, 0, (k * sizeof(btype) + sizeof(int))); obj->n = (k * sizeof(btype) + sizeof(int));}}
#define MY_FREE(obj) if(obj) { llog("free :0x%p.\n", obj); free(obj); (obj) = 0;} 


#define swap(A, B) {(A) += (B); (B) = (A) - (B); (A) -= (B);}
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
int init_npermu(permutation_st* npermu, int n);
int init_npermu(permutation_st* npermu, int n) {
	int err = 0;
	My_GENERIC* p = 0;
	do {
		if (!npermu) {
			err = 1; break;
		}
		memset(npermu, 0, sizeof(permutation_st));
		npermu->n = n;
		MY_MALLOC((npermu->n), int, p, My_GENERIC);
		npermu->arr = p;
		int* arr = (int *)npermu->arr->data;
		for (int i = 0; i < npermu->n; ++i) {
			arr[i] = i;
		}
	} while (0);
	return err;
}

int countt = 1;
int get_next_npermu(permutation_st* p);
void dum_value(permutation_st* , int *);

int main(int argc, char* argv[]) {
	int next = 1;
	permutation_st npermu;
	init_npermu(&npermu, 3);
	while (next) {
			
		dum_value(&npermu, &countt);
		next = get_next_npermu(&npermu);
	}
	MY_FREE(npermu.arr);
	return 0;
}


void dum_value(permutation_st* permu, int *count) {
	int i = 0;
	int* p = (int*) permu->arr->data;
	fprintf(stdout, "\ncount : %d====\t", count ? ((*count)++):0);
	//++countt;
	for (i = 0; i < permu->n; ++i) {
		fprintf(stdout, "%u, ", p[i]);
	}
	fprintf(stdout, "\n");
}

int get_next_npermu(permutation_st* permu) {
	MY_ROLE mode = STRAIGHT;
	int i = 0;
	int next = 0;
	int* p = (int *) permu->arr->data;
	//int tmp = 0;
	do {
		int t = -1;
		int u = -1;
		int v = 1;
		do {
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
					swap(p[i], p[t])
					break;
				}
			}
			//Reverse
			i = 0;
			while ((u + i) < (v - i)) {
				swap(p[u + i], p[v - i]);
				++i;
			}
			next = 1;
		} while (0);
	} while (0);

	return next;
}