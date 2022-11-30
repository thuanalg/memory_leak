//---
//ntthuan

#ifndef __SHARE_TRAFFIC__
#define __SHARE_TRAFFIC__

#include <sys/time.h>
#include <pthread.h>

#ifdef __cpluspplus
extern "C" {
#endif

typedef struct {
	unsigned long long total;
	unsigned long long used_data;
	pthread_mutex_t frame_mtx;

	char data[0];

} LIST_SHARED_TRAFFIC;

typedef struct 
{
	char is_first;
	char is_closed;
	struct timespec start_at;
	struct timespec updated_at;
	int stream_id;
	char ip[32];
	char proto[7];
	int port;
	unsigned long long upload;
	unsigned long long download;
	unsigned long long pid;
	unsigned long long delta_upload;
	unsigned long long delta_download;
	
} TRAFFIC_STREAM_INFO;

typedef struct {
	int low_port;	
	int high_port;	
	char protocol[7];
} TRAFFIC_ROLE;

//MC2-985, TRAFFIC_STREAM_INFO;

extern void *skylab_data_shm;

void add_item_traffic(LIST_SHARED_TRAFFIC **p, char *item, int sz);
void *skylab_open_shm();
int   skylab_unlink_shm();
int   skylab_write_shm(LIST_SHARED_TRAFFIC *p, char *data, int n);
int   skylab_read_shm(LIST_SHARED_TRAFFIC *p, char **data, char only_read);
void  skylab_init_watching_thread();
int   skylab_take_traffic_data(char **p);
void dump_traffic_to_str(TRAFFIC_STREAM_INFO *p, void *mlog);

#ifdef __cpluspplus
}
#endif

#endif
