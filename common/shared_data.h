//---
//ntthuan

#ifndef __SHARE_DATA__
#define __SHARE_DATA__

#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>

#ifndef llog
	#define llog(p, fmt, ... ) syslog(p, "%s:%d <<<>>> "fmt, __FILE__, __LINE__, __VA_ARGS__)
#endif

#define LIST_SHARED_DATA_SZ        									(2 * 1024 * 1024)
#define LLU 														unsigned long long

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
	

	//Mutex with used case: USING_SPIN_LOCK macro
	//To access critical region
	pthread_spinlock_t frame_spin_lock;
	//To stop working a group of processes
	pthread_spinlock_t exit_spin_lock;




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
	//pid_t write_pid;


	//Check group exit, you can ignore
	char should_exit;

	//Check sleeping, you can ignore
	char sleeping;
	
	//to occupy data
	char data[0];
} LIST_SHARED_DATA;


typedef struct {
	struct timespec timee;
	char protocol[32];
} SHARED_ITEM;


extern void *ntt_data_shm;

void add_item_traffic(LIST_SHARED_DATA **p, char *item, int sz);
void  ntt_write_thread();
void *ntt_open_shm(int n);
int   ntt_unlink_shm(int n);
int   ntt_write_shm(LIST_SHARED_DATA *p, char *data, int n, char *sendsig, pid_t *);
int   ntt_read_shm(LIST_SHARED_DATA *p, char **data, char only_read);
int set_exit_group(char val);
int check_exiit(char increase);
int set_read_pid(pid_t pid);


pid_t get_read_pid();
pthread_t  ntt_read_thread();

void ntt_daemonize();
#ifdef __cpluspplus
}
#endif

#endif
