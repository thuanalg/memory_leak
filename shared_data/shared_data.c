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
#ifdef USING_MUTEX
	//#error "Using MUTEX"
#elif defined(USING_SEMAPHORE) 
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
void *read_body_thread(void *data);
void *write_body_thread(void *data);
int check_exit(char increase);

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
		#ifdef USING_MUTEX
			LIST_SHARED_DATA *p = (LIST_SHARED_DATA *)data;
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
		#else
	
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

int ntt_write_shm(LIST_SHARED_DATA *p, char *data, int n)
{
	int rs = 0;
	//size_t n = 0;
	int errx = 0;
	
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

		errx = pthread_mutex_lock(&(p->frame_mtx));

			//fprintf(stdout, "errx: %d\n", errx);
			do {
				if(errx) {
					break;	
				}		
				
				if( (n + p->used_data + sizeof(LIST_SHARED_DATA)) >= p->total)
				{
					p->used_data = 0;	
					//Call backup here
				}

				memcpy(tmp + p->used_data, data, n);
				p->used_data += n;
				rs = p->used_data;
			}
			while(0);

		if(!errx){
			errx = pthread_mutex_unlock(&(p->frame_mtx));
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

		errx = pthread_mutex_lock(&(p->frame_mtx));
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
			errx = pthread_mutex_unlock(&(p->frame_mtx));
		}
		(*data) = tmp;
	}
	while(0);
	return rs;
}


void ntt_read_thread()
{

}
void ntt_write_thread()
{

}
void *read_body_thread(void *data) {
	while(1)
	{
		sleep(1);
	}
	return 0;
}

void *write_body_thread(void *data) {
	return 0;
}
int count_should_exit = 0; 
int check_exit(char increase) {
	int rs = 0;
	LIST_SHARED_DATA *p = (LIST_SHARED_DATA*) ntt_data_shm;
	pthread_mutex_t *mtx = &(p->exit_mtx);
	pthread_mutex_lock(mtx);
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
	pthread_mutex_unlock(mtx);
	return rs;
}


void *ntt_data_shm = 0;
//Endfile
