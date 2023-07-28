#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SORT_LENGTH 4096000
#include <time.h>

//ntthuan: nguyenthaithuanalg@gmaul.com

typedef struct __SORT_SEGMENT__ {
	int low;
	int hi;
	struct __SORT_SEGMENT__ *next;
} SORT_SEGMENT;


int quicksort(int **arr, int l, int h);
int partition (void **arr, int low, int hi);
void quick_sort_non_recursive (void ** arr, int low, int hi);
void show_output( long int * arr, int length);
short unit_test(long int *arr, int length);
int main (int argc, char *argv[]) {
  struct timespec t;
	long int *arr = 0;
	int i = 0;
	unsigned int length = SORT_LENGTH * sizeof(long int);
  memset(&t, 0, sizeof(t));
	arr = ( long int *) malloc( length);
	memset(arr, 0, length);
	fprintf(stdout, "\n");
	for ( i = 0; i < SORT_LENGTH; ++i)
	{
    clock_gettime(CLOCK_REALTIME, &t);
		arr[i] = (t.tv_nsec % random());
	}
	//quick_sort_non_recursive( (void **) &arr, 0, SORT_LENGTH - 1);
	quicksort( (void **) &arr, 0, SORT_LENGTH - 1);
	fprintf(stdout, "\n");
	show_output(arr, SORT_LENGTH);
	fprintf(stdout, "\n");
	unit_test(arr, SORT_LENGTH);
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
		while( i <  j)
		{
			if (pivot > data[j]) break;
			j--;
		}
		while( i < j)
		{
			if(data[i] > pivot) break;
			i++;
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
	
	if(i == j)
	{
		if(data[low] > data[j])
		{
			//Swap
			tmp = data[low];
			data[low] = data[j];
			data[j] = tmp;
			j--;
		}
		else if( data[low] < data[j])
		{
			j--;
			if(data[low] > data[j])
			{
				//Swap
				tmp = data[low];
				data[low] = data[j];
				data[j] = tmp;
			}
		}
	}
	else if ( i > j)
	{
		//Swap
		tmp = data[low];
		data[low] = data[j];
		data[j] = tmp;
		j--;
	}
	
	return j;
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
		fprintf(stdout, "low: %d, high: %d\n", root->low, root->hi);
		j = partition (arr, root->low, root->hi);
		//fprintf(stdout, "\n");
		//show_output((long int*)*arr, SORT_LENGTH);

		tmp = root;
		root = root->next;
		free (tmp);
		tmp = 0;

		if ( j > tmp_low ) {
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
			tmp = (SORT_SEGMENT *)malloc (sizeof (SORT_SEGMENT));
			tmp->low = j ;
			tmp->hi = tmp_hi;
			tmp->next = 0;
			last->next = tmp;
			last = tmp;
		}
	}
}

void show_output( long int * arr, int length) {
	int i = 0;
	for ( i = 0; i< length; ++i) {
		fprintf(stdout, "%ld ", arr[i]);
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

int quicksort(int **arr, int l, int h)
{
  int rs = 0;
  if(l < h)
  { 
    int j = partition(arr, l, h);
    quicksort(arr, l, j -1);
    quicksort(arr, j + 1, h);
  }
  return rs;
}
