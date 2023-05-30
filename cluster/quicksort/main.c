#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SORT_LENGTH 16000

//ntthuan: nguyenthaithuanalg@gmail.com

typedef struct __SORT_SEGMENT__ {
	int low;
	int hi;
	struct __SORT_SEGMENT__ *next;
} SORT_SEGMENT;

int partition (void **arr, int low, int hi);
void quick_sort_non_recursive (void ** arr, int low, int hi);
void quick_sort_non_recursive_improve (void ** arr, int low, int hi);
void quick_sort (void ** arr, int low, int hi);
void show_output( long int * arr, int length);
short unit_test(long int *arr, int length);

int main (int argc, char *argv[]) {
	long int *arr = 0;
	long int *arr_b = 0;
	long int *arr_c = 0;
	struct timespec start, end;
	int i = 0;
	unsigned int length = SORT_LENGTH * sizeof(long int);
	arr = ( long int *) malloc( length);
	memset(arr, 0, length);
	arr_b = ( long int *) malloc( length);
	memset(arr_b, 0, length);
	arr_c = ( long int *) malloc( length);
	memset(arr_c, 0, length);

	fprintf(stdout, "\n");

	for ( i = 0; i < SORT_LENGTH; ++i)
	{
		arr[i] = (random() % 100);
	}
	
	fprintf(stdout, "\n");
	//show_output(arr, SORT_LENGTH);

	clock_gettime( CLOCK_REALTIME , &start);
	quick_sort_non_recursive( (void **) &arr, 0, SORT_LENGTH - 1);
	clock_gettime( CLOCK_REALTIME , &end);

	
	fprintf(stdout, "delta 1: %lu\n", ((end.tv_sec - start.tv_sec) * 1000000000 + end.tv_nsec - start.tv_nsec));

	for ( i = 0; i < SORT_LENGTH; ++i)
	{
		arr_b[i] = (random() % 10000);
	}
	
	clock_gettime( CLOCK_REALTIME , &start);
	quick_sort( (void **) &arr_b, 0, SORT_LENGTH - 1);
	clock_gettime( CLOCK_REALTIME , &end);
	fprintf(stdout, "delta 2: %lu\n", ((end.tv_sec - start.tv_sec) * 1000000000 + end.tv_nsec - start.tv_nsec));

	for ( i = 0; i < SORT_LENGTH; ++i)
	{
		arr_c[i] = (random() % 100);
	}
	
	clock_gettime( CLOCK_REALTIME , &start);
	quick_sort_non_recursive_improve( (void **) &arr_c, 0, SORT_LENGTH - 1);
	clock_gettime( CLOCK_REALTIME , &end);
	fprintf(stdout, "delta 3: %lu\n", ((end.tv_sec - start.tv_sec) * 1000000000 + end.tv_nsec - start.tv_nsec));

	//fprintf(stdout, "\n");
	//show_output(arr, SORT_LENGTH);
	fprintf(stdout, "\n");
	unit_test(arr, SORT_LENGTH);
	unit_test(arr_b, SORT_LENGTH);
	unit_test(arr_c, SORT_LENGTH);
	fprintf(stdout, "\n");
	//show_output(arr_c, SORT_LENGTH);
	fprintf(stdout, "\n");
	return EXIT_SUCCESS;
}
int partition (void **arr, int low, int hi) {
	long int pivot = 0;
	int i, j;
	int tmp = 0;
	long int * data = (long int *) (*arr);
	pivot = data[low];
	i = low + 1;
	j = hi;
	while (i < j)
	{
		while ( data[j] >= pivot) {
			if ( i < j) {
				j--;
			}
			else {
				break;
			}
		}
		while (data[i] <= pivot) {
			if (i < j) {
				i++;
			}else {
				break;
			}
		}
		if (i < j) {
			//Swap
			tmp = data[i];
			data[i] = data[j];
			data[j] = tmp;
			++i;
			--j;
		}
	}
	
	if (data[j] < pivot) {
		//Swap
		tmp = data[low];
		data[low] = data[j];
		data[j] = tmp;
	}
	
	return j;
}
void quick_sort (void ** arr, int low, int hi) {
	int j = 0;
	j = partition (arr, low, hi);
	if (low < j -1)
		quick_sort(arr, low, j -1 );
	if (j < hi)
		quick_sort(arr, j , hi);
}

void quick_sort_non_recursive (void ** arr, int low, int hi) {
	SORT_SEGMENT * root = 0;
	SORT_SEGMENT * last = 0;
	SORT_SEGMENT * tmp  = 0;
	int j = 0;
	int tmp_low, tmp_hi;
	if (low >= hi) {
		return;
	}
	root = (SORT_SEGMENT *)malloc (sizeof (SORT_SEGMENT));
	root->low = low;
	root->hi = hi;
	root->next = 0;
	last = root;
	while (root) {

		tmp_low = root->low;
		tmp_hi = root->hi;
		tmp = root;
		root = root->next;
		free (tmp);
		tmp = 0;

		j = partition (arr, tmp_low, tmp_hi);
		if (tmp_low < j -1) {
			tmp = (SORT_SEGMENT *)malloc (sizeof (SORT_SEGMENT));
			tmp->next = 0;
			tmp->low = tmp_low;
			tmp->hi = j-1;
			if ( !root) {
				root = tmp;
				last = root;
			}
			else {
				last->next = tmp;
				last = tmp;
			}
		}
		if( j < tmp_hi) {
			tmp = (SORT_SEGMENT *)malloc (sizeof (SORT_SEGMENT));
			tmp->low = j ;
			tmp->hi = tmp_hi;
			tmp->next = 0;
			if ( !root) {
				root = tmp;
				last = root;
			}
			else {
				last->next = tmp;
				last = tmp;
			}
		}
	}
}


void quick_sort_non_recursive_improve (void ** arr, int low, int hi) {
	int *arr_low;
	int *arr_hi;
	int total = 0;
	int j = 0;
	int jump = 0;
	unsigned int size_tmp = 0;
	int tmp_low, tmp_hi;
	if (low >= hi) {
		return;
	}

	size_tmp = (hi - low + 1) * sizeof (int); 
	arr_low = (int *) malloc ( size_tmp);
	arr_hi = (int *) malloc ( size_tmp);
	memset(arr_low, 0, size_tmp);
	memset(arr_hi, 0, size_tmp);
	arr_low[total] = low;
	arr_hi[total] = hi;

	while (arr_hi[jump] > 0) {
		tmp_low = arr_low[jump];
		tmp_hi = arr_hi[jump];
		j = partition (arr, tmp_low, tmp_hi);
		if (tmp_low < j -1) {
			total++;
			arr_low[total] = tmp_low;
			arr_hi[total] = j -1;
		}
		if( j < tmp_hi) {
			++total;
			arr_low[total] = j;
			arr_hi[total] = tmp_hi;
		}
		jump++;
	}
	free(arr_low);
	free(arr_hi);
}

void show_output( long int * arr, int length) {
	int i = 0;
	for ( i = 0; i< length; ++i) {
		fprintf(stdout, "%d ", arr[i]);
	}
}

short unit_test(long int *arr, int length) {
	short result = 1;
	int i = 0;
	for (i = 0; i < length -1; i++){
		if ( arr[i] > arr[i+1]) {
			result = 0;
			fprintf(stdout, "error: %d\n", i);
		}
	}
	return result;
}
