#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sqlite3.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <lcs_common.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#include <sys/time.h>
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD

#endif
/***********************************************************************************/

/***********************************************************************************/

typedef struct LCS_TEMPLATE_INFO
{
	char name_ana[64];
	char targetpath[1024];
	char prefixshmkey[512];
}
LCS_TEMPLATE_INFO;

/***********************************************************************************/
static void lcs_init_app();
static void lcs_audd_loadshm(int *);
static void lcs_unit_analysis(int *);
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void *lcs_read_shmagent(char * key, int n, int *err);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_exec_sql(char *qpath, char *path, char *file, int *err);
static int  lcs_exec_cb(void* data, int ncols, char** values, char** headers);
static int  lcs_exec_cbtrue(void* data, int ncols, char** values, char** headers);
/***********************************************************************************/
static	char 						*lcs_cfg_file 	= 0;
static  LCS_TEMPLATE_INFO			lcs_info_cfg;
static	LCS_AGENTS_SHARED			*lcs_agt 		= 0;
static	LCS_AGENTS_SHARED_AH		*lcs_agt_ah 	= 0;
static	LCS_AUDD_SHARED				*lcs_audd 		= 0;
static	LCS_AUDD_SHARED_AUH			*lcs_audd_auh 	= 0;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err = 0;
	struct timeval tv0, tv1;
	unsigned long diff = 0;


	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	gettimeofday(&tv0,NULL);
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_audd_loadshm(&err);

	if(err)
	{
		return EXIT_FAILURE;
	}

	lcs_unit_analysis(&err);
	gettimeofday(&tv1,NULL);
	diff = (tv1.tv_sec - tv0.tv_sec) * 1000000 + tv1.tv_usec - tv0.tv_usec;
	fprintf(stdout, "\n\ntime: %ld\n\n", diff);

	if(err)
	{
		return EXIT_FAILURE;
	}

	fprintf(stdout, "\n\n\n\n\n\n\nA_000010_05m SUCCESS.");
	return EXIT_SUCCESS;
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load(LCS_TEMPLATE_INFO *info, int *err)
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
void lcs_cfg_item(LCS_TEMPLATE_INFO *info, char *item, int *err)
{
	char *base = 0;
	unsigned int sz = 0;
	do
	{
		base = "targetpath_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->targetpath, &item[sz], strlen(item)-sz);
			break;
		}

		base = "name_ana_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->name_ana, &item[sz], strlen(item)-sz);
			break;
		}

		base = "prefixshmkey_0003:";
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
	memset(&lcs_info_cfg, 0 , sizeof(LCS_TEMPLATE_INFO));
	lcs_cfg_load(&lcs_info_cfg, &err);
}
/***********************************************************************************/
void lcs_unit_analysis(int *err)
{
	char *path 		= 0;
	char *name 		= 0;
	int isexist 	= 0;

	char db[1024];
	char qpath[1024];
	char writepath[1024];

	path = lcs_info_cfg.targetpath;
	name = lcs_info_cfg.name_ana;
/*
	memset(data, 0, 1024);
	sprintf(data, "%s/cdr.data", path);

	lcs_comm_filexist(data, &isexist);
	if(!isexist)
	{
		fprintf(stdout, "No cdr data.\n");
		return;	
	}
*/
	memset(qpath, 0, 1024);
	sprintf(qpath, "../sql/%s.sql", name);

	lcs_comm_filexist(qpath, &isexist);
	if(!isexist)
	{
		*err = __LINE__;
		return;	
	}

	memset(db, 0, 1024);
	sprintf(db, "%s/lcs.db", path);

	lcs_comm_filexist(db, &isexist);
	if(!isexist)
	{
		*err = __LINE__;
		return;	
	}

	memset(writepath, 0, 1024);
	sprintf(writepath, "%s/%s.analysis", path, name);

	lcs_exec_sql(qpath, db, writepath, err);
}
/***********************************************************************************/
void lcs_exec_sql(char *query, char *db, char *writepath, int *err)
{
	FILE *fp 			= 0;
	char *sql			= 0;
	sqlite3 *conn		= 0;
	char *sErrMsg		= 0;
	LCS_SELECT_CB cb 	= 0;

	cb = lcs_exec_cbtrue;
	do
	{
		fp = fopen(writepath, "w+");
		if(!fp)
		{
			*err = __LINE__; 
			break;
		}
		lcs_file_text(query, &sql, err);
		//fprintf(stdout, "sql: %s\n", sql);
		if(!sql)
		{
			*err = __LINE__;
			break;	
		}
		//fprintf(stdout, "sql: %s\n", sql);
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
		*err = sqlite3_exec(conn, sql, cb, fp, &sErrMsg);
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

	if(sql)
	{
		free(sql);
	}
	if(sErrMsg)
	{
		fprintf(stdout, "Err: %s\n", sErrMsg);
		sqlite3_free(sErrMsg);
	}
	if(conn)
	{
		sqlite3_close(conn);
	}
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
int  lcs_exec_cbtrue(void* data, int ncols, char** val, char** headers)
{
	int i									= 0;
	int t									= 0;
	int h									= -1;
	int *arr								= 0;
	unsigned long long int u				= 0;
	lcs_mmap_agents	*p1						= 0;
	lcs_mmap_auto_dial_detail *p0			= 0;

	sscanf(val[ncols -1], "%llu", &u);
	h = lcs_comm_hash_audd(u, (lcs_audd->n));
	if(h >  -1)
	{
		arr = lcs_audd_auh[h].h;
		while(t < LCS_AUDD_HASH_ARR_N)
		{
			if(arr[t] < 0)
			{
				break;
			}

			if(u == lcs_audd_auh[arr[t]].au.id)
			{
				p0 = &(lcs_audd_auh[arr[t]].au);
				break;
			}
			++t;
		}
	}	
	/* +++++++++++++++++++++*/	
	t = 0; h = -1;
	lcs_comm_hash(val[7], 0, &h, lcs_agt->n);
	if(h > -1)
	{
        arr = lcs_agt_ah[h].h;
        while(t < LCS_AGENTS_HASH_ARR_N)
        {
            if(arr[t] < 0)
            {
                break;
            }
            if( strcmp(lcs_agt_ah[arr[t]].a.agent_number, val[7]) == 0)
            {
				p1 = &(lcs_agt_ah[arr[t]].a);
                break;
            }
            ++t;
        }
	}
	/* +++++++++++++++++++++*/	
	do
	{
		fprintf((FILE*) data, "%s\t", val[i++]);
	}
	while( i < 6 );
	/*
	for(i = 0; i < 6; ++i)
	{
		fprintf((FILE*) data, "%s\t", val[i]);
	}
	*/
	/* +++++++++++++++++++++*/	

	fprintf((FILE*) data, "%s\t", (p1) ? (p1->vcontext):("NULL")  );
	/* +++++++++++++++++++++*/	
	
	do
	{
		fprintf((FILE*) data, "%s\t", val[i++]);
	}
	while( i < 8 );
	/*
	for(i = 6; i < 8; ++i)
	{
		fprintf((FILE*) data, "%s\t", val[i]);
	}
	*/
	/* +++++++++++++++++++++*/	

	fprintf((FILE*) data, "%s\t", (p1) ? (p1->name):("NULL")  );
	/* +++++++++++++++++++++*/	
	ncols--;
	do
	{
		fprintf((FILE*) data, "%s\t", val[i++]);
	}
	while( i < ncols );
	/*
	for(i = 8; i < ncols - 1; ++i)
	{
		fprintf((FILE*) data, "%s\t", val[i]);
	}
	*/
	fprintf((FILE*) data, "%s\t", (p0)?(p0->phone_type):("NULL"));

	fprintf((FILE*)data, "\n");
	return 0;
};
/***********************************************************************************/
int  lcs_exec_cb(void* data, int ncols, char** val, char** headers)
{
	int i = 0;
	int h = -1;
	int t = 0;
	int *arr = 0; 
	int *arr1 = 0;
	unsigned long long int u = 0;
	char * tmp = 0;
	
	for(i = 0; i < ncols - 2; ++i)
	{
		fprintf((FILE*) data, "%s\t", val[i]);
	}
	lcs_comm_hash(val[i], 0, &h, lcs_agt->n);
    if(h > -1)
    {
        tmp = 0;
        arr = lcs_agt_ah[h].h;
        while(t < LCS_AGENTS_HASH_ARR_N)
        {
            if(arr[t] < 0)
            {
                break;
            }
            if( strcmp(lcs_agt_ah[arr[t]].a.agent_number, val[i]) == 0)
            {
                tmp = lcs_agt_ah[arr[t]].a.name;
                fprintf((FILE*) data, "%s\t", lcs_agt_ah[arr[t]].a.name);
                fprintf((FILE*) data, "%s\t", lcs_agt_ah[arr[t]].a.vcontext);
                break;
            }
            ++t;
        }
        if(!tmp)
        {
            fprintf((FILE *)data, "%s\t", "Add agent info here!!!");
        }
    }

	//fprintf((FILE*) data, "%s\t", val[i]);
	++i;
	sscanf(val[i], "%llu", &u);
	h = -1;
	h = lcs_comm_hash_audd(u, (lcs_audd->n));
	if(h > -1)
	{
		t = 0;
		arr1 = lcs_audd_auh[h].h;
		while(t < LCS_AUDD_HASH_ARR_N)
		{
			if(arr1[t] < 0)
			{
				break;
			}
			if(u == lcs_audd_auh[arr1[t]].au.id)
			{
				fprintf((FILE*) data, "%s", lcs_audd_auh[arr1[t]].au.phone_type);
			}
			++t;
		}
	}
	fprintf((FILE*)data, "\n");
	return 0;
}

/***********************************************************************************/
void *lcs_read_shmagent(char * key, int sz, int *err)
{
	int shm = 0;
	void *data = 0;
	fprintf(stdout, "size: %d\n", sz);
	do
	{
		shm = shm_open(key, O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
		if(shm < 0)
		{
			*err = 1;
			break;
		}
		ftruncate(shm, sz);
		data = mmap(0, sz, PROT_READ, MAP_SHARED, shm, 0);
		close(shm);
	}
	while(0);
	return data;
}
/***********************************************************************************/
void lcs_audd_loadshm(int *error)
{
	int sz 					= 0;
	int err					= 0;
	char key[256];
	do
	{
		sz = sizeof(LCS_AGENTS_SHARED);
		//p = calloc(1, sz);
		memset(key, 0, 256);
		sprintf(key, "%s.agent.num", lcs_info_cfg.prefixshmkey);
		lcs_agt = lcs_read_shmagent(key, sz, &err);

		if(!lcs_agt)
		{
			err = __LINE__; break;
		}
		
		memset(key, 0, 256);
		sprintf(key, "%s.agent.data", lcs_info_cfg.prefixshmkey);
		sz = (lcs_agt->n + 1) * sizeof(LCS_AGENTS_SHARED_AH);
		lcs_agt_ah = lcs_read_shmagent(key, sz, &err);
		if(!lcs_agt_ah)
		{
			err = __LINE__; break;
		}
		
		memset(key, 0, 256);
		sprintf(key, "%s.audd.num", lcs_info_cfg.prefixshmkey);
		sz = sizeof(LCS_AUDD_SHARED);	
		lcs_audd = lcs_read_shmagent(key, sz, &err);
		if(!lcs_audd)
		{
			fprintf(stdout, "lcs_audd error. \n");
			err = __LINE__;
			break;
		}
		memset(key, 0, 256);
		sprintf(key, "%s.audd.data", lcs_info_cfg.prefixshmkey);
		sz = (lcs_audd->n + 1) * sizeof( LCS_AUDD_SHARED_AUH);
		fprintf(stdout, "size audd_auh: %d\n", sz);
		lcs_audd_auh = lcs_read_shmagent(key, sz, &err);
		if(!lcs_audd_auh)
		{
			err = __LINE__;
			fprintf(stdout, "lcs_audd_auh error. \n");
			break;
		}
	}
	while(0);
	*error = err;
}
/***********************************************************************************/
