//#ifdef USING_MUTEX
//#elif defined(USING_SEMAPHORE) 
//#else
//	#error "Choose MUTEX OR SEMAPHORE"
//#endif

//#
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#include <pthread.h>
#include <unistd.h>
#include "shared_data.h"
#include <signal.h>

#ifdef USING_MUTEX
	//#error "Using MUTEX"
#elif defined(USING_SEMAPHORE) 
	// "Using SEMAPHORE"
#elif defined(USING_RWLOCK) 
	// "Using SEMAPHORE"
#else
	#error "Choose MUTEX OR SEMAPHORE"
#endif

#define _X(u) AAA_##u

#define NTT_PATH  										"/77b5ea39-1dc2-4875-8b4c-8700972db7ff_"
#define NTT_SHARED_MODE      							(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define LIST_SHARED_DATA_SZ        						(1 * 1024 * 1024)
#define SHARED_DATA_STEP 								(100 *1024) 


static int _X(init_service)(char *path, void **, int);
static int _X(init_shm_mtx)(pthread_mutex_t *);
static int _X(init_rwlock)(pthread_rwlock_t *);
void *read_body_thread(void *data);
void *write_body_thread(void *data);

int ntt_unlink_shm()
{
  int err = 0;
	char *path = (char *)NTT_PATH;
  do
  {
    if(ntt_data_shm)
    {
      err = munmap(ntt_data_shm, LIST_SHARED_DATA_SZ);
    }

    if(!path)
    {
      err = shm_unlink(path);
    }
  }
  while(0);
  fprintf(stdout, "err -  %s: %d\n", __FUNCTION__, err);
  return err;
}

void *ntt_open_shm()
{
  int err  = 0;
  char *path = (char *)NTT_PATH;
  do
  {
    err = _X(init_service)(path, &ntt_data_shm, LIST_SHARED_DATA_SZ);
  }
  while(0);
	if(!err){
		return ntt_data_shm;	
	}

  return 0;
}


int _X(init_service)(char *path, void **out, int sz)
{
  char create = 0;
  char *key = path;
  int err = 0;
  int shm = 0;
  void *data = 0;
  //sprintf(key, "%s%s", NTT_PATH, key);

  do {
		//Try to create or read/write
	
		if(!out)
		{
	      err = __LINE__;
	      break;
		}
	
	  shm = shm_open(key, O_CREAT | O_RDWR | O_EXCL, NTT_SHARED_MODE);  
	  if(shm >= 0)
	  {
			// It is creating mode.
	    create = 1;
	  }
	  else
	  {
		// It is reading/writing mode.
	    	shm = shm_open(key, O_RDWR | O_EXCL, NTT_SHARED_MODE);  
	  }
	
		if(shm < 0)
		{
			err = __LINE__;
			break;
		}
	
		ftruncate(shm, sz);
		
		// Mapping memory with the size: sz
		data = mmap(0, sz, PROT_READ | PROT_WRITE | PROT_EXEC , MAP_SHARED, shm, 0);
	
		if(data == MAP_FAILED)
		{
	  		err = __LINE__;
			break;
		}	
		//fprintf(stdout, "sz: %d\n", sz);
	  if(!data)
	  {
	    err = __LINE__;
	    break;
	  }
		*out = data;
	  if(!create)
	  {
		//Already created!!!
			sleep(1);
	    break;
	  }
		//Must initial data
	  memset(data, 0, sz);
		do {
				LIST_SHARED_DATA *p = (LIST_SHARED_DATA *)data;
			#ifdef USING_MUTEX
				pthread_mutex_t *mtx = &(p->frame_mtx);
				p->total = LIST_SHARED_DATA_SZ;
		    err  = _X(init_shm_mtx)(mtx);
				if(err) {
					break;
				}
				mtx = &(p->exit_mtx);
		    	err  = _X(init_shm_mtx)(mtx);
				if(err) {
					break;
				}
			#elif defined(USING_RWLOCK) 
				pthread_rwlock_t *mtx = &(p->frame_rwlock);
				p->total = LIST_SHARED_DATA_SZ;
		    err  = _X(init_rwlock)(mtx);
				if(err) {
					break;
				}
				mtx = &(p->exit_rwlock);
		    err  = _X(init_rwlock)(mtx);
				if(err) {
					break;
				}
			#elif defined(USING_SEMAPHORE) 
				sem_t *t = &(p->frame_sem);
				p->total = LIST_SHARED_DATA_SZ;
				err = sem_init(t, 1, 1);
				if(err) {
					break;
				}
				t = &(p->exit_sem);
				err = sem_init(t, 1, 1);
				if(err) {
					break;
				}
				fprintf(stdout, "used_data: %d, total: %d\n", p->used_data, p->total);
			#else
				#error "Must use MUTEX or SEMAPHORE"	
			#endif
		} while(0);
  }
  while(0);

  if(err) {
		fprintf(stdout, "Error: %d\n", err);
  }
  if(shm > -1)
  {
    err = close(shm); 
  }
  return err;
}

int _X(init_shm_mtx)(pthread_mutex_t *shm_mtx)
{
  int err = 0;
  do
  {
    if(!shm_mtx)
    {
      err = __LINE__;
      break;
    }

    pthread_mutexattr_t psharedm;
    pthread_mutexattr_init(&psharedm);
    pthread_mutexattr_setpshared(&psharedm, PTHREAD_PROCESS_SHARED);
    err = pthread_mutex_init(shm_mtx, &psharedm);                                                           
    if(err)
    {
      err = __LINE__;
      break;
    }
  }
  while(0);
  fprintf(stdout, "err - %s: %d\n", __FUNCTION__, err);
  return err;
}


int _X(init_rwlock)(pthread_rwlock_t *shm_rwmtx)
{
  int err = 0;
  do
  {
    if(!shm_rwmtx)
    {
      err = __LINE__;
      break;
    }

    pthread_rwlockattr_t psharedm;
    pthread_rwlockattr_init(&psharedm);
    pthread_rwlockattr_setpshared(&psharedm, PTHREAD_PROCESS_SHARED);
    err = pthread_rwlock_init(shm_rwmtx, &psharedm);                                                           
    if(err)
    {
      err = __LINE__;
      break;
    }
  }
  while(0);
  fprintf(stdout, "err - %s: %d\n", __FUNCTION__, err);
  return err;
}



int ntt_write_shm(LIST_SHARED_DATA *p, char *data, int n, char *sendsig, pid_t *pcid)
{
	int rs = 0;
	//size_t n = 0;
	int errx = 0;
	char send_signal = 0;
	
	do
	{
		char *tmp = 0;
		if(!data)
		{
			rs = -1;
			break;	
		}
		if(!p)
		{
			rs = -2;
			break;	
		}
		if(n < 1){
			break;	
		}
	
		tmp = p->data;

#ifdef USING_MUTEX
		errx = pthread_mutex_lock(&(p->frame_mtx));
#elif defined(USING_RWLOCK) 
		errx = pthread_rwlock_wrlock(&(p->frame_rwlock));
#elif defined(USING_SEMAPHORE) 
		errx = sem_wait(&(p->frame_sem));
#else
	#error "Choose MUTEX OR SEMAPHORE, RWLOCK"
#endif
			do {
				if(errx) {
					break;	
				}		
				send_signal = (p->used_data == 0);
				if(send_signal && sendsig && pcid) {
					*sendsig = send_signal;
					*pcid = p->read_pid;
				}
				if( (n + p->used_data + sizeof(LIST_SHARED_DATA)) >= p->total)
				{
					p->used_data = 0;	
					//Call backup here
				}

				memcpy(tmp + p->used_data, data, n);
				p->used_data += n;
				rs = p->used_data;
				fprintf(stdout, "func: %s, write data: %d, total: %d\n", 
					__FUNCTION__, p->used_data, p->total);
			}
			while(0);

		if(!errx){
#ifdef USING_MUTEX
			errx = pthread_mutex_unlock(&(p->frame_mtx));
#elif defined(USING_RWLOCK) 
			errx = pthread_rwlock_unlock(&(p->frame_rwlock));
#elif defined(USING_SEMAPHORE) 
			errx = sem_post(&(p->frame_sem));
#else
	#error "Choose MUTEX OR SEMAPHORE"
#endif
		}
	}
	while(0);
	return rs;
}

int ntt_read_shm(LIST_SHARED_DATA *p, char **data, char clean)
{
	int rs = 0;
	char *tmp = 0;
	int errx = 0;
	static pid_t pidd = 0;
	do
	{
		int n = 0;
		if(!data)
		{
			rs = -1;
			break;	
		}
		if(!p)
		{
			rs = -2;
			break;	
		}

#ifdef USING_MUTEX
		errx = pthread_mutex_lock(&(p->frame_mtx));
#elif defined(USING_SEMAPHORE) 
		errx = sem_wait(&(p->frame_sem));
#elif defined(USING_RWLOCK) 
		errx = pthread_rwlock_wrlock(&(p->frame_rwlock));
#else
	#error "Choose MUTEX OR SEMAPHORE, RWLOCK"
#endif
		if (!pidd)
		{
			pidd = getpid();
			p->read_pid = pidd;
			//sigaction
		}
		do {
			if(errx) {
				break;	
			}		
			n = p->used_data;
			if( n < 1)
			{
				break;	
			}
			tmp = (char *)malloc(n + 1);
			if(!tmp)
			{
				break;	
			}
			memset(tmp, 0,n + 1);
			memcpy(tmp , p->data, n);
			p->used_data = 0;
			p->data[0] = 0;
			rs = n;
		}
		while(0);

		if(!errx) {
#ifdef USING_MUTEX
			errx = pthread_mutex_unlock(&(p->frame_mtx));
#elif defined(USING_SEMAPHORE) 
			errx = sem_post(&(p->frame_sem));
#elif defined(USING_RWLOCK) 
			errx = pthread_rwlock_unlock(&(p->frame_rwlock));
#else
	#error "Choose MUTEX OR SEMAPHORE, RWLOCK"
#endif
		}
		(*data) = tmp;
	}
	while(0);
	return rs;
}


pthread_t ntt_read_thread()
{
	int rc = 0;
	pthread_t ptid = 0;
	rc = pthread_create(&ptid, 0, read_body_thread, 0);
	fprintf(stdout, "f: %s, rc: %d, ptid: %llui\n", __FUNCTION__, rc, ptid);
	return ptid;
}

void ntt_write_thread()
{

}

void *read_body_thread(void *data) {
	int err = 0;
	while(1)
	{
		char *dta = 0;
		LIST_SHARED_DATA *p = (LIST_SHARED_DATA*) ntt_data_shm;
		int n = check_exiit(1);
		if(n) break;
		n = ntt_read_shm(p, &dta, 1);
		if(!n){
			sleep(3);
			fprintf(stdout, "Sleeping bacause of no data. Func: %s\n", __FUNCTION__);
		}
		else {
			fprintf(stdout, "Have data size: %d\n", n);
			free(dta);
			dta = 0;
		}
	}
	fprintf(stdout, "line: %d, Finish reading thread.\n", __LINE__);
	
	//err = kill(getpid(), SIGALRM);
	
	union sigval sv;
	sv.sival_int = 2;
	err =	sigqueue(getpid(), SIGALRM, sv);
	return 0;
}

void *write_body_thread(void *data) {
//	while(1)
//	{
//		int n = check_exiit(1);
//		if(n) break;
//		sleep(1);
//	}
	return 0;
}

int count_should_exit = 0; 
int check_exiit(char increase) {
	int rs = 0;
	LIST_SHARED_DATA *p = (LIST_SHARED_DATA*) ntt_data_shm;
#ifdef USING_MUTEX
	pthread_mutex_t *mtx = &(p->exit_mtx);
	pthread_mutex_lock(mtx);
#elif defined(USING_RWLOCK) 
	pthread_rwlock_t *mtx = &(p->exit_rwlock);
	pthread_rwlock_wrlock(mtx);
#elif defined(USING_SEMAPHORE) 
	sem_t *t = &(p->exit_sem);
	sem_wait(t);
#else
	#error "Choose MUTEX OR SEMAPHORE, RWLOCK"
#endif

	do
	{
		if(! p->should_exit) {
			break;
		}
		if(increase) {
			count_should_exit++;
			rs = count_should_exit;
			break;
		}
		if(!count_should_exit) count_should_exit++;
		rs = count_should_exit;
	} while(0);
#ifdef USING_MUTEX
	pthread_mutex_unlock(mtx);
#elif defined(USING_RWLOCK) 
	pthread_rwlock_unlock(mtx);
#elif defined(USING_SEMAPHORE) 
	sem_post(t);
#else
#endif
	return rs;
}


int set_exit_group(char val) {
	LIST_SHARED_DATA *p = 0;
	p = (LIST_SHARED_DATA *) ntt_data_shm;
#ifdef USING_MUTEX
	pthread_mutex_lock(&(p->exit_mtx));
		p->should_exit = val;
	pthread_mutex_unlock(&(p->exit_mtx));
#elif defined(USING_RWLOCK) 
	pthread_rwlock_wrlock(&(p->exit_rwlock));
		p->should_exit = val;
	pthread_rwlock_unlock(&(p->exit_rwlock));
#elif defined(USING_SEMAPHORE) 
	sem_wait(&(p->exit_sem));
		p->should_exit = val;
	sem_post(&(p->exit_sem));
#else
	#error "Choose MUTEX OR SEMAPHORE, RWLOCK"
#endif
	return 0;
}

pid_t get_read_pid()
{
	LIST_SHARED_DATA *p = 0;
	p = (LIST_SHARED_DATA *) ntt_data_shm;
	pid_t readpid = 0;
	int errx = -1;
	do {
		if(!p) break;
		do {
#ifdef USING_MUTEX
			errx = pthread_mutex_lock(&(p->frame_mtx));
#elif defined(USING_SEMAPHORE) 
			errx = sem_wait(&(p->frame_sem));
#elif defined(USING_RWLOCK) 
			errx = pthread_rwlock_wrlock(&(p->frame_rwlock));
#else
	#error "Choose MUTEX OR SEMAPHORE, RWLOCK"
#endif
		}
		while(0);

		if(!errx) {
			readpid = p->read_pid;
#ifdef USING_MUTEX
			errx = pthread_mutex_unlock(&(p->frame_mtx));
#elif defined(USING_SEMAPHORE) 
			errx = sem_post(&(p->frame_sem));
#elif defined(USING_RWLOCK) 
			errx = pthread_rwlock_unlock(&(p->frame_rwlock));
#else
	#error "Choose MUTEX OR SEMAPHORE, RWLOCK"
#endif
		}
	} while (0);
	return readpid;
}

void *ntt_data_shm = 0;
//Endfile
