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
	#error "Using MUTEX"
#elif defined(USING_SEMAPHORE) 
	#error "Using SEMAPHORE"
#else
	//#error "Choose MUTEX OR SEMAPHORE"
#endif

#define _X(u) AAA_##u

#define NTT_PATH  										"/77b5ea39-1dc2-4875-8b4c-8700972db7ff_"
#define NTT_SHARED_MODE      							(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define LIST_SHARED_DATA_SZ        						(1 * 1024 * 1024)
#define SHARED_DATA_STEP 								(100 *1024) 


static int _X(init_service)(char *path, void **, int);
static int _X(init_shm_mtx)(pthread_mutex_t *);

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
	#ifdef USING_NTT_MTX
		LIST_SHARED_DATA *p = (LIST_SHARED_DATA *)data;
		pthread_mutex_t *mtx = &(p->frame_mtx);
		p->total = LIST_SHARED_DATA_SZ;
    	err  = _X(init_shm_mtx)(mtx);
	#else

	#endif
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

pthread_t dummy_watching = 0;
LIST_SHARED_DATA *psessions = 0;
pthread_mutex_t session_mtx =  PTHREAD_MUTEX_INITIALIZER;
//void analyse_data();
void *dummy_watching_body(void *dta)
{
	while(1)
	{
		sleep(3);	
		//analyse_data();
	}
	return 0;	
}

void  ntt_init_watching_thread()
{
	int rc = 0;
  int err = 0;                                                           
	if(!psessions)
	{
		psessions = (LIST_SHARED_DATA*)malloc(sizeof(LIST_SHARED_DATA));
		memset(psessions, 0, sizeof(LIST_SHARED_DATA));
		psessions->total = sizeof(LIST_SHARED_DATA);
    err = pthread_mutex_init( &(psessions->frame_mtx), 0);                                                           
	}
	if(err)
	{
		exit(1);	
	}
	rc = pthread_create(&dummy_watching, 0, dummy_watching_body, 0);
	if(rc)
	{
		exit(1);	
	}
}

void *ntt_data_shm = 0;


void add_item_traffic(LIST_SHARED_DATA **p, char *item, int sz)
{
    LIST_SHARED_DATA *t = 0;
    unsigned long long n = 0;
    char *pdata = 0;
    do
    {
        if(sz < 1){
            break;
        }
        if(!item){
            break;
        }
        if(!p){
            break;
        }
        if(!(*p))
        {
            (*p) = (LIST_SHARED_DATA *) malloc(SHARED_DATA_STEP);
            if(!(*p))
            {
                //LOG(FATAL) << __LINE__ <<", MALLOC error " << std::endl;
                exit(1);
            }
            t = *p;
            memset(t, 0, SHARED_DATA_STEP);
            t->total = SHARED_DATA_STEP;
        }
        t = *p;
        n = t->used_data + sizeof(LIST_SHARED_DATA) + sz;
        if( (n+1) > t->total)
        {
            unsigned long long up_size = t->total + SHARED_DATA_STEP;
            while( up_size < (n+1))
            {
                up_size += SHARED_DATA_STEP;
            }

            (*p) = (LIST_SHARED_DATA *) realloc( (*p), up_size);
            if(!(*p))
            {
                exit(1);
            }
            t = *p;
            t->total = up_size;
        }

        pdata = t->data;
        memcpy(pdata + t->used_data, item, sz);
        t->used_data += sz;
    }
    while(0);
}
void ntt_read_pthread()
{

}
void ntt_write_pthread()
{

}
//Endfile
