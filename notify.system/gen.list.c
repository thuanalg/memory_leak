#include <gen.list.h>
#include <pthread.h>

pthread_mutex_t gen_list_mtx = PTHREAD_MUTEX_INITIALIZER; 

int add_item_gen_list(GEN_LIST **p, char *item, int n)
{
	int rc = 1;
	do {
		rc = pthread_mutex_lock(&geb_list_mtx);
		if(!rc) { 
			fprintf(stdout, "lock error .\n");
			break;
		}

		do {
			int err = add_item_traffic( p, item, n); 
			if(err) {
				fprintf(stdout, "add item error.\n");
			}
		} while(0);

		rc = pthread_mutex_unlock(&geb_list_mtx);
		if(!rc) { 
			fprintf(stdout, "lock error .\n");
			break;
		}
	} while (0); 
	return rc;
}

//#define GEN_LIST_STEP (1024 * 32)
#define GEN_LIST_STEP (1 << 15)

int add_item_traffic(GEN_LIST **p, char *item, int sz)
{
    GEN_LIST *t = 0;
		int rc = -1;
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
            (*p) = (GEN_LIST *) malloc(GEN_LIST_STEP);
            if(!(*p))
            {
                //LOG(FATAL) << __LINE__ <<", MALLOC error " << std::endl;
                exit(1);
            }
            t = *p;
            memset(t, 0, GEN_LIST_STEP);
            t->total = GEN_LIST_STEP;
        }
        t = *p;
        n = t->used_data + sizeof(GEN_LIST) + sz;
        if( (n+1) > t->total)
        {
            unsigned long long up_size = t->total + GEN_LIST_STEP;
            while( up_size < (n+1))
            {
                up_size += GEN_LIST_STEP;
            }

            (*p) = (GEN_LIST *) realloc( (*p), up_size);
            if(!(*p))
            {
                //LOG(FATAL) << __LINE__ <<", MC2-985 , REALLOC error " << std::endl;
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
		return rc;
}
