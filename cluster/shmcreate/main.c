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
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD

#endif

/***********************************************************************************/

typedef struct LCS_SHM_A_INFO
{
	char dbpath[1024];
	char prefixshmkey[128];
}
LCS_SHM_A_INFO;


/***********************************************************************************/
static void lcs_init_app();
static void lcs_shm_start(int *);
static void lcs_shm_load(char *dbpath, int *err);
static void lcs_cfg_load(LCS_SHM_A_INFO *, int *);
static void lcs_cfg_item(LCS_SHM_A_INFO *, char *, int *);
static void *lcs_create_shmagent(char * key, int n, int *err);
static int  lcs_shm_write(void* data, int ncols, char** values, char** headers);
static int  lcs_agents_count(void* data, int ncols, char** values, char** headers);
/***********************************************************************************/
static	char 				*lcs_cfg_file = 0;
static  LCS_SHM_A_INFO	lcs_info_cfg;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err = 0;
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_shm_start(&err);
	if(err)
	{
		fprintf(stdout, "FAILURE message, err: %d.\n", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, "SUCCESS message.\n");
	return EXIT_SUCCESS;
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load(LCS_SHM_A_INFO *info, int *err)
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
void lcs_cfg_item(LCS_SHM_A_INFO *info, char *item, int *err)
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
	memset(&lcs_info_cfg, 0 , sizeof(LCS_SHM_A_INFO));
	lcs_cfg_load(&lcs_info_cfg, &err);

}
/***********************************************************************************/
void lcs_shm_start(int *err)
{
	int isexist = 0;
	char db[1024];
	memset(db, 0, 1024);
	sprintf(db, "%s/lcs.db", lcs_info_cfg.dbpath);
	fprintf(stdout, "db: %s\n", db);
	lcs_comm_filexist(db, &isexist);
	if(!isexist)
	{
		*err = __LINE__;
		return;
	}
	//fprintf(stdout, "db: %s\n", db);
	lcs_shm_load(db, err);
}
/***********************************************************************************/
void lcs_shm_load(char *db, int *err)
{
	int n						= 0;
	int i  						= 0;
	int sz						= 0;
	char key[256];
	char *sql 					= 0;
	sqlite3 *conn				= 0;
	char *sErrMsg				= 0;
	LCS_SELECT_CB cb			= 0;
	LCS_AGENTS_SHARED *agred 	= 0;      

	do
	{
		*err = sqlite3_open(db, &conn);
		if(*err)
		{
			break;
		}
        *err = sqlite3_exec(conn, "BEGIN TRANSACTION", 0, 0, &sErrMsg);
		if(*err)
		{
			*err = __LINE__;
			break;
		}

		cb = lcs_agents_count;
		sql = "select count(*) from agents where is_enable='true';";
		*err = sqlite3_exec(conn, sql, cb, &n, &sErrMsg);
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
		sz = sizeof(LCS_AGENTS_SHARED);
		//agred = calloc(1, sizeof(LCS_AGENTS_SHARED));
		memset(key, 0, 256);
		sprintf(key, "%s.agent.num", lcs_info_cfg.prefixshmkey);
		agred = lcs_create_shmagent(key, sz, err);		
		if(!agred)
		{
			fprintf(stdout, "err: key: %s\n", key);
			*err = __LINE__;
			break;
		}
		agred->n = n;
		agred->i = 0;
		//agred->a = calloc(n + 1, sizeof(lcs_mmap_agents));
		//agred->h = calloc(n + 1, sizeof(void *));
		memset(key, 0, 256);
		sz = (n + 1) * sizeof(LCS_AGENTS_SHARED_AH);
		sprintf(key, "%s.agent.data", lcs_info_cfg.prefixshmkey);
		agred->ah = lcs_create_shmagent(key, sz, err);		
		if(!(agred->ah))
		{
			*err = __LINE__;
			break;
		}
		sz = LCS_AGENTS_HASH_ARR_N * sizeof( agred->ah[0].h[0]);
		for(i = 0; i < n; ++i)
		{
			memset (agred->ah[i].h, -1, sz);
		}

//		sz = (n + 1) * sizeof(void *);
//		agred->h = lcs_create_shmagent("asterisk_demo_1day_2017_10_29_hash_a", sz, err);		

		cb = lcs_shm_write;
		sql = "select * from agents where is_enable='true';";
		*err = sqlite3_exec(conn, sql, cb, agred, &sErrMsg);
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
int  lcs_shm_write(void* data, int ncols, char** val, char** headers)
{
	int i 					= 0;
	int k 					= 0;
	int index 				= 0;
	int t 					= 0;
	LCS_AGENTS_SHARED 		*obj 	= 0;
	LCS_AGENTS_SHARED_AH 	*cur  	= 0;
	

	obj = (LCS_AGENTS_SHARED*)data;

	k = obj->i;
	cur = &((obj->ah)[k]);
	sscanf(val[i], "%lu", &(cur->a.id));
	++i;
	sscanf(val[i], "%lu", &(cur->a.version));
	++i;
	lcs_comm_hash(val[i], strlen(val[i]), &index, obj->n);
	
	while(t < LCS_AGENTS_HASH_ARR_N)
	{
		if( (obj->ah)[index].h[t] < 0)
		{
			(obj->ah)[index].h[t] = k; 
			break;
		}
		//fprintf(stdout, "key: %s, value: %d, k: %d\n", val[i], index, k);	
		++t;
	}
	
/*
	if(  ((obj->ah)[index]).h < 0)
	{
		(obj->ah)[index].h = k;
	}
	else 
	{
		agt = (obj->ah)[index].h;
		(obj->ah)[index].h = k;
		(obj->ah)[index].next = agt;
	}
*/
	strcat(cur->a.agent_number, val[i]);
	++i;
	strcat(cur->a.vcontext, val[i]);
	++i;
	strcat(cur->a.agent_status, val[i]);
	++i;
	strcat(cur->a.current_dnd, val[i]);
	++i;
	strcat(cur->a.email, val[i]);
	++i;
	strcat(cur->a.extension, val[i]);
	++i;
	sscanf(val[i], "%c", &(cur->a.is_allow_mapping));
	++i;
	sscanf(val[i], "%c", &(cur->a.is_enable));
	++i;
	sscanf(val[i], "%c", &(cur->a.is_inbound));
	++i;
	sscanf(val[i], "%lu",&(cur->a.last_history_id));
	++i;
	strcat(cur->a.name, val[i]);
	++i;
	strcat(cur->a.password, val[i]);
	++i;
	strcat(cur->a.phone_number, val[i]);
	++i;
	sscanf(val[i], "%u", &(cur->a.priority));
	++i;
	sscanf(val[i], "%u", &(cur->a.queue_id));
	++i;
	strcat(cur->a.f1_code, val[i]);
	++i;
	strcat(cur->a.fixed_extension, val[i]);
	++i;
	strcat(cur->a.customer_name, val[i]);

	(obj->i)++;
	return 0;
}
/***********************************************************************************/
int  lcs_agents_count(void* data, int ncols, char** values, char** headers)
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
void *lcs_create_shmagent(char * key, int sz, int *err)
{
	int shm = 0;
	void *data = 0;
	fprintf(stdout, "size: %d\n", sz);
	do
	{
		shm = shm_open(key, O_RDONLY, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
		fprintf(stdout, "shm: %d\n", shm);
		if(shm > -1)
		{
			fprintf(stdout, "Having!!!\n");
			break;
		}
		
		shm = shm_open(key, O_CREAT|O_RDWR, LCS_SHM_FILE_MODE);  
		if(shm < 0)
		{
			*err = __LINE__;
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
