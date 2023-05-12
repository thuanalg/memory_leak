#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#include <lcs_common.h>
#include <sys/time.h>
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD

#endif

/***********************************************************************************/

typedef struct LCS_AUDD_INFO
{
	char dbpath[1024];
	char prefixshmkey[128];
}
LCS_AUDD_INFO;

typedef struct LCS_AUDD_GROUP
{
	LCS_AUDD_SHARED *ni       ;
	LCS_AUDD_SHARED_AUH *data ;
} 
LCS_AUDD_GROUP;

/***********************************************************************************/
static void lcs_init_app();
static void lcs_audd_start(int *);
static void lcs_audd_shmload(char *, int *err);
static void lcs_cfg_load(LCS_AUDD_INFO *, int *);
static void lcs_cfg_item(LCS_AUDD_INFO *, char *, int *);
static void *lcs_create_shmaudd(char * key, int n, int *err);
static int  lcs_audd_shmwrite(void* data, int n, char** val, char** headers);
static int  lcs_audd_count(void* data, int ncols, char** val, char** headers);
/***********************************************************************************/
static	char 				*lcs_cfg_file = 0;
static  LCS_AUDD_INFO	lcs_info_cfg;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err = 0;
	struct timeval t0, t1;
	unsigned long int diff = 0;
	gettimeofday(&t0, 0);
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_audd_start(&err);
	gettimeofday(&t1, 0);
	diff = (t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec;
	if(err)
	{
		fprintf(stdout, "AUDD-SHM failure message, err: %d.\n", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, "\n\ndiff: %lu\n\n", diff);
	fprintf(stdout, "AUDD-SHM success message.\n");
	return EXIT_SUCCESS;
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load(LCS_AUDD_INFO *info, int *err)
{
	char *text = 0;
	char *pch = 0;
	lcs_file_text(lcs_cfg_file, &text, err);
	pch = strtok(text, " \r\n");
	while(pch)
	{
		lcs_cfg_item(info, pch, err);
		pch = strtok(0, " \r\n");
	}
	if(text)
	{
		free(text);
	}
}
/***********************************************************************************/
/* Store text into data structure
 * info		:	output, it is to store configured information
 * item		:	text, input
 * error	:	error code
 */
/***********************************************************************************/
void lcs_cfg_item(LCS_AUDD_INFO *info, char *item, int *err)
{
	char *base = 0;
	unsigned int sz = 0;
	do
	{
		base = "dbpath_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbpath, &item[sz], strlen(item)-sz);
			break;
		}

		base = "prefixshmkey_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->prefixshmkey, &item[sz], strlen(item)-sz);
			break;
		}
	}
	while(0);	
}
/***********************************************************************************/
/* Init information/variables for whole process
 *
 */
/***********************************************************************************/
void lcs_init_app()
{
	int err = 0;
	memset(&lcs_info_cfg, 0 , sizeof(LCS_AUDD_INFO));
	lcs_cfg_load(&lcs_info_cfg, &err);

}
/***********************************************************************************/
void lcs_audd_start(int *err)
{
	int isexist = 0;
	char db[1024];

	memset(db, 0, 1024);
	sprintf(db, "%s/lcs.db", lcs_info_cfg.dbpath);
	lcs_comm_filexist(db, &isexist);
	if(!isexist)
	{
		*err = __LINE__;
		return;	
	}
	//fprintf(stdout, "db: %s\n", db);
	lcs_audd_shmload(db, err);
}
/***********************************************************************************/
void lcs_audd_shmload(char *db, int *err)
{
	int n								= 0;
	int i  								= 0;
	int sz								= 0;
	sqlite3 *conn						= 0;
	char *sErrMsg						= 0;
	LCS_SELECT_CB cb					= 0;
	char key[256];
	LCS_AUDD_GROUP group;
	LCS_AUDD_SHARED *lcs_audd_n 		= 0;      
	LCS_AUDD_SHARED_AUH *lcs_audd_data	= 0;		
	do
	{
		*err = sqlite3_open(db, &conn);
		if(*err)
		{
			*err = __LINE__;
			break;
		}
        *err = sqlite3_exec(conn, "BEGIN TRANSACTION", 0, 0, &sErrMsg);
		if(*err)
		{
			*err = __LINE__;
			break;
		}
		cb = lcs_audd_count;
		*err = sqlite3_exec(conn, "select count(*) from auto_dial_detail;", cb, &n, &sErrMsg);
		if(*err)
		{
			*err = __LINE__;
			break;
		}
		if(!n)
		{
			*err = __LINE__;
			break;
		}
		sz = sizeof(LCS_AUDD_SHARED);
		fprintf(stdout, "sz num: %d\n", sz);
		memset(key, 0, 256);
		sprintf(key, "%s.audd.num", lcs_info_cfg.prefixshmkey);
		lcs_audd_n = lcs_create_shmaudd(key, sz, err);		
		if(!lcs_audd_n)
		{
			fprintf(stdout, "key: %s\n", key);
			*err = __LINE__;
			break;
		}

		lcs_audd_n->n = n;
		fprintf(stdout, "count: %d\n", n);
		lcs_audd_n->i = 0;

		sz = (n + 1) * sizeof(LCS_AUDD_SHARED_AUH);
		fprintf(stdout, "sz data: %d\n", sz);
		memset(key, 0, 256);
		sprintf(key, "%s.audd.data", lcs_info_cfg.prefixshmkey);
		lcs_audd_data = lcs_create_shmaudd(key, sz, err);		
		sz = LCS_AUDD_HASH_ARR_N * sizeof(lcs_audd_data[i].h[0]);
		for(i = 0; i < n; ++i)
		{
			memset (lcs_audd_data[i].h, -1, sz);
		}
		group.ni = lcs_audd_n;
		group.data = lcs_audd_data;	
		cb = lcs_audd_shmwrite;
		*err = sqlite3_exec(conn, 
			"select id, phone_type, skill, customer_id, application_id, note, phone_number from auto_dial_detail;", 
			cb, &group, &sErrMsg);
		if(*err)
		{
			*err = __LINE__;
			break;
		}

        *err = sqlite3_exec(conn, "END TRANSACTION", 0, 0, &sErrMsg);
		if(*err)
		{
			*err = __LINE__;
			break;
		}
	}
	while(0);

	if(sErrMsg)
	{
		fprintf(stdout, "Err: %s\n", sErrMsg);
		sqlite3_free(sErrMsg);
	}
	if(conn)
	{
		sqlite3_close(conn);
	}
}
/***********************************************************************************/
int  lcs_audd_shmwrite(void* data, int ncols, char** val, char** headers)
{
	int i = 0;
	int t = 0;
	int k = 0;
	int index = 0;
	LCS_AUDD_GROUP *p = 0;
	LCS_AUDD_SHARED_AUH *d = 0;

	p = data;
	d = p->data;	

	k = p->ni->i;
	i = 0;	
	sscanf(val[i], "%llu", &(d[k].au.id));
	index = lcs_comm_hash_audd((d[k].au.id), (p->ni->n));
	while(t < LCS_AUDD_HASH_ARR_N)
	{
		//Take care more here
		if(d[index].h[t] < 0)
		{
			d[index].h[t] = k;
			break;
		}
		++t;
		fprintf(stdout, "t: %d\n", t);
	}
	++i;
	strcat(d[k].au.phone_type, val[i]);

	++i;
	sscanf(val[i], "%hu", &(d[k].au.skill) );

	++i;
	strcat(d[k].au.customer_id, val[i]);

	++i;
	strcat(d[k].au.application_id, val[i]);

	++i;
	strcat(d[k].au.note, val[i]);

	++i;
	strcat(d[k].au.phone_number, val[i]);

	(p->ni->i)++;
	return 0;
}
/***********************************************************************************/
int  lcs_audd_count(void* data, int ncols, char** values, char** headers)
{
	int *n = (int *) data;
	if(!n) 
	{
		return 1;
	}
	sscanf(values[0], "%d", n);
	return 0;
}
/***********************************************************************************/
void *lcs_create_shmaudd(char * key, int sz, int *err)
{
	int shm = 0;
	void *data = 0;
	fprintf(stdout, "size: %d\n", sz);
	do
	{
		shm = shm_open(key, O_RDONLY, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
		if(shm > -1)
		{
			fprintf(stdout, "Having!!!\n");
			break;
		}
		
		shm = shm_open(key, O_CREAT|O_RDWR, LCS_SHM_FILE_MODE);  
		if(shm < 0)
		{
			*err = 1;
			shm_unlink(key);			
			break;  
		}
		ftruncate(shm, sz);
		data = mmap(0,
		sz, PROT_READ | PROT_WRITE | PROT_EXEC , MAP_SHARED, shm, 0);
		close(shm); 
		memset(data, 0, sz);
	}
	while(0);
	return data;   
}
/***********************************************************************************/
