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
#ifdef USING_MUTEX
	#error "Using MUTEX"
#elif defined(USING SEMAPHORE) 
	#error "Using SEMAPHORE"
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
int real_equal(SHARED_ITEM *a, SHARED_ITEM *b);
int compare_traffic_info(SHARED_ITEM *a, SHARED_ITEM *b);

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

//void analyse_data()
//{
//	char *data = 0;
//	int n = 0;
//	int k = 0;
//	int szitem =  sizeof(SHARED_ITEM);
//	int i = 0;
//	//int szlist =  sizeof(LIST_SHARED_DATA);
//	SHARED_ITEM *tmp = 0;
//	SHARED_ITEM *t = 0;
//	
//	LIST_SHARED_DATA *p = (LIST_SHARED_DATA *)ntt_data_shm;
//	//LIST_SHARED_DATA *psessions = (LIST_SHARED_DATA *) &sdn_list;
//	struct timespec updated_at;
//	memset(&updated_at, 0, sizeof(updated_at));
//
//	if( clock_gettime( CLOCK_REALTIME, &updated_at) == -1 ) {
//		LOG(ERROR) << "MC2-985, " << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ 
//			<< " : " << ", clock_gettime: error" 
//			<< std::endl;
//		exit(1);
//	}
//
//	//std::string str = "";
//
//	if(!p)
//	{
//		return;	
//	}
//	if(!psessions)
//	{
//		return;
//	}
//	n = ntt_read_shm(p, (char **)&data, 0);
//
//	if( n < 1 || !data){
//		return;	
//	}
//	k = n / szitem;
//	tmp = (SHARED_ITEM *)data;
//
//	do 
//	{
//		int mm = 0, u = 0;
//		pthread_mutex_t *mtx = &(session_mtx);
//		pthread_mutex_lock(mtx);
//		do
//		{
//			for(i = 0; i < k; ++i)
//			{
//				char found = 0;
//				char cond = 0;
////				str = "";
////				dump_traffic_to_str(tmp + i, (void *)&str);
//
//				t = (SHARED_ITEM *)psessions->data;
//				mm = psessions->used_data/szitem;
//
//				for(u = 0; u < mm; ++u)
//				{
////					str = "";
////					dump_traffic_to_str(t + u, (void *)&str);
////					LOG(ERROR) << "MC2-985, " << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ 
////						<< " : " << ", i: " << i
////						<< " : " << ", k: " << k
////						<< " : " << ", u: " << u
////						<< " : " << ", sessioens: " << mm
////						<< ":" << str.c_str() 
////						<< std::endl;
////
//
//					cond = (updated_at.tv_sec - t[u].updated_at.tv_sec > (10*60));
//					if(cond){
//						t[u].is_closed = 1;	
//					}
//					
//					cond = (t[u].start_at.tv_sec != tmp[i].start_at.tv_sec);
//					if(cond){
//						continue;
//					}
//
//					cond = (t[u].start_at.tv_nsec != tmp[i].start_at.tv_nsec);
//					if(cond){
//						continue;
//					}
//
//					cond = (t[u].pid != tmp[i].pid);
//					if(cond){
//						continue;
//					}
//
//					cond = (t[u].stream_id != tmp[i].stream_id);
//					if(cond){
//						continue;
//					}
//
//					found = 1;
//					break;
//				}
//
////				str = "";
////				dump_traffic_to_str(t + u, (void *)&str);
////				LOG(ERROR) << "MC2-985, " << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ 
////					<< " : " << ", i: " << i
////					<< " : " << ", toal from buffer: " << k
////					<< " : " << ", u: " << u
////					<< " : " << ", sessions: " << mm
////					<< " : " << ", found: " << (found ? "1" : "0")
////					<< ":" << str.c_str() 
////					<< std::endl;
//				
//				if(!found){
//					add_item_traffic(&psessions, (char *)(tmp + i), szitem);
//				}
//				else 
//				{
//					t[u].updated_at = tmp[i].updated_at;
//					if(tmp[i].port > 0)
//					{
//						t[u].port = tmp[i].port;	
//					}
//					if(tmp[i].upload > t[u].upload)
//					{
//						t[u].delta_upload += ( tmp[i].upload - t[u].upload);
//						t[u].upload = tmp[i].upload;
//					}
//					if(tmp[i].download > t[u].download)
//					{
//						t[u].delta_download += ( tmp[i].download - t[u].download);
//						t[u].download = tmp[i].download;
//					}
//					if(t[u].is_first)
//					{
//						t[u].is_first = 0;
//						t[u].delta_download = tmp[i].download;
//						t[u].delta_upload = tmp[i].upload;
//					}
//					t[u].is_closed = tmp[i].is_closed;
//					if(tmp[i].proto[0])
//					{
//						snprintf(t[u].proto, 5, "%s", tmp[i].proto);	
//					}
//					if(tmp[i].ip[0])
//					{
//						snprintf(t[u].ip, 64, "%s", tmp[i].ip);	
//					}
//
//				}
//			}
//			//End for
//		}while(0);
//
//		pthread_mutex_unlock(mtx);
//	}
//	while(0);
//
//	if(data){
//		free(data);
//	}
//}
//
//
//
////ntthuan
////#ifndef __SHARE_DATA__
//#define __SHARE_DATA__

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
                LOG(FATAL) << __LINE__ <<", MC2-985 , REALLOC error " << std::endl;
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
//int ntt_take_traffic_data(char **data)
//{
//	int n = 0;
//	char *p = 0;
//	LIST_SHARED_DATA *pp = 0;
//	int k = 0;
//	int sz = sizeof(SHARED_ITEM);
//	SHARED_ITEM *t = 0;
//	do
//	{
//		int i = 0;
//		if(!data)
//		{
//			break;	
//		}
//		if(!psessions)
//		{
//			break;	
//		}
//		pthread_mutex_t *mtx = &(session_mtx);
//		pthread_mutex_lock(mtx);
//		do
//		{
//			n = psessions->used_data;
//			if(!n)
//			{
//				break;	
//			}
//			k = n / sz;
//			p = (char *) malloc(n + 1);
//			memset(p, 0, n + 1);
//			memcpy(p, psessions->data, n);
//			//Clean close session
//			t = (SHARED_ITEM *)psessions->data;
//			for(i = 0; i < k; ++i)
//			{
//				if(t[i].is_closed)
//				{
//					continue;	
//				}
//				t[i].delta_upload = 0;
//				t[i].delta_download = 0;
//				add_item_traffic(&pp, (char *)(t + i), sz);
//			}
//			if(pp)
//			{
//				//memset(psessions->data, 0, psessions->used_data);
//				psessions->used_data = pp->used_data;	
//				memcpy(psessions->data, pp->data, pp->used_data);	
//			}
//			else {
//				psessions->used_data = 0;	
//			}
//			//*data = p;
//		}
//		while(0);
//		pthread_mutex_unlock(mtx);
//	}
//	while(0);
//	if(pp)
//	{
//		free(pp);	
//		pp = 0;
//	}
//	if(p) {
//		int high = 0;
//		int ii = 0;
//		do
//		{
//			high = (n / sizeof(SHARED_ITEM));
//			if(high < 1){
//				break;
//			}
//			
//			SHARED_ITEM item;
//			SHARED_ITEM *info = (SHARED_ITEM *)p;
//			char checked = 0;
//			int j = 0;
//			int k = 0;
//			SHARED_ITEM *t = 0;
//			item = info[0];
//			add_item_traffic(&pp, (char *)&item, sizeof(item));
//			for(ii = 1; ii < high; ++ii)
//			{
//				if(!pp)
//				{
//					break;
//				}
//				k = pp->used_data/sizeof(SHARED_ITEM);
//				t = (SHARED_ITEM *)pp->data;
//				checked = 0;
//				for(j = 0; j < k; ++j)
//				{
//					if(real_equal(info + ii, t + j))
//					{
//						t[j].delta_upload += info[ii].delta_upload;
//						t[j].delta_download += info[ii].delta_download;
//						checked = 1;
//						break;
//					}
//				}
//				if(!checked)
//				{
//					add_item_traffic(&pp, (char *) (info + ii), sizeof(item));
//				}
//			}
//			free(p); p = 0;
//			n = pp->used_data;	
//			p = (char *) malloc(n + 1);
//			memset(p, 0, n + 1);
//			memcpy(p, pp->data, n);
//			*data = p;
//			if(pp)
//			{
//				free(pp);
//			}
//		}
//		while(0);
//	}
//	return n;
//}
//#endif
// ok
//int real_less_than(SHARED_ITEM *a, SHARED_ITEM *b){
//	return (compare_traffic_info(a, b) == -1);
//}
//ok
//int real_equal(SHARED_ITEM *a, SHARED_ITEM *b){
//	return (compare_traffic_info(a, b) == 0);
//}
//ok
//int equal_less_than(SHARED_ITEM *a, SHARED_ITEM *b){
//	return (compare_traffic_info(a, b) != -1);
//}
////ok
//int equal_greater_than(SHARED_ITEM *a, SHARED_ITEM *b){
//	return (compare_traffic_info(a, b) != 1);
//}

// a < b --> 1
// a == b --> 0
// a > b --> -1
//int compare_traffic_info(SHARED_ITEM *a, SHARED_ITEM *b)
//{
//	int n = 0;
//	do
//	{
//		if(!a) {n = 1; break;};
//		if(!b) {n = -1 ; break;};
//		if(a->port < b->port) {n = 1; break;};
//		if(a->port > b->port) {n = -1;break;};
//		n = strncmp(a->proto, b->proto, 4);
//		if(n) {
//			n = (n > 0) ? 1 : -1;
//			break;
//		}
//		n = strncmp(a->ip, b->ip, 24);
//		if(n) {
//			n = (n > 0) ? 1 : -1;
//			break;
//		}
//		n = 0;
//	}
//	while(0);
//	return n;
//}

//Endfile
