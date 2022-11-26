//---
//ntthuan

#ifndef __SHARE_DATA__
#define __SHARE_DATA__

#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cpluspplus
extern "C" {
#endif

typedef struct {
	//Size of whole region
	unsigned long long total;
	//Size of segment is used
	unsigned long long used_data;


	//Mutex with used case: USING_MUTEX macro
	//To access critical region
	pthread_mutex_t frame_mtx;
	//To stop working a group of processes
	pthread_mutex_t exit_mtx;
	

	//Mutex with used case: USING_SEMAPORE macro
	//To access critical region	
	sem_t frame_sem;
	//To stop working a group of processes
	sem_t exit_sem;



	//Mutex with used case: USING_RWLOCK macro
	//To access critical region	
	pthread_rwlock_t frame_rwlock;
	//To stop working a group of processes
	pthread_rwlock_t exit_rwlock;
	

	pid_t read_pid;
	pid_t write_pid;


	//Check group exit, you can ignore
	char should_exit;
	
	//to occupy data
	char data[0];
} LIST_SHARED_DATA;


typedef struct {
	struct timespec timee;
	char protocol[32];
} SHARED_ITEM;


extern void *ntt_data_shm;

void add_item_traffic(LIST_SHARED_DATA **p, char *item, int sz);
void *ntt_open_shm();
int   ntt_unlink_shm();
int   ntt_write_shm(LIST_SHARED_DATA *p, char *data, int n, char *sendsig, pid_t *);
int   ntt_read_shm(LIST_SHARED_DATA *p, char **data, char only_read);
pthread_t  ntt_read_thread();
void  ntt_write_thread();
int set_exit_group(char val);
int check_exiit(char increase);

#ifdef __cpluspplus
}
#endif

#endif
