//---
//ntthuan

#ifndef __SHARE_DATA__
#define __SHARE_DATA__

#include <sys/time.h>
#include <pthread.h>

#ifdef __cpluspplus
extern "C" {
#endif

typedef struct {
	unsigned long long total;
	unsigned long long used_data;
	//To access critical region
	pthread_mutex_t frame_mtx;
	//To stop working a group of processes
	pthread_mutex_t exit_mtx;
	char should_exit;
	

	char data[0];

} LIST_SHARED_DATA;

//typedef struct 
//{
//	char is_first;
//	char is_closed;
//	struct timespec start_at;
//	struct timespec updated_at;
//	int stream_id;
//	char ip[32];
//	char proto[7];
//	int port;
//	unsigned long long upload;
//	unsigned long long download;
//	unsigned long long pid;
//	unsigned long long delta_upload;
//	unsigned long long delta_download;
//	
//} SHARED_ITEM;

typedef struct {
	unsigned long long int raw_time;	
	char protocol[32];
} SHAARED_ITEM;


extern void *ntt_data_shm;

void add_item_traffic(LIST_SHARED_DATA **p, char *item, int sz);
void *ntt_open_shm();
int   ntt_unlink_shm();
int   ntt_write_shm(LIST_SHARED_DATA *p, char *data, int n);
int   ntt_read_shm(LIST_SHARED_DATA *p, char **data, char only_read);
void  ntt_init_watching_thread();
//int   ntt_take_traffic_data(char **p);
//void dump_traffic_to_str(SHARED_ITEM *p, void *mlog);

#ifdef __cpluspplus
}
#endif

#endif
