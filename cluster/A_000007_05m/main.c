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
	char targetpath[1024];
	char name_ana[64];
	char prefixshmkey[512];
}
LCS_TEMPLATE_INFO;

/***********************************************************************************/
static void lcs_init_app();
static void lcs_unit_analysis(int *);
static void lcs_load_shmagent(int *err);
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void *lcs_read_shmagent(char * key, int n, int *err);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_exec_sql(char *qpath, char *db, char *writefile, int *err);
static int  lcs_exec_cb(void* data, int ncols, char** values, char** headers);
static int  lcs_exec_cbtrue(void* data, int ncols, char** values, char** headers);
/***********************************************************************************/
static  LCS_TEMPLATE_INFO			lcs_info_cfg;
static	LCS_AGENTS_SHARED			*lcs_agt 		= 0;
static	LCS_AGENTS_SHARED_AH		*lcs_agt_ah 	= 0;
static	char 						*lcs_cfg_file 	= 0;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err 				= 0;
	unsigned long diff 		= 0;
	struct timeval tv0, tv1;


	if(argc < 2)
	{
		return EXIT_FAILURE;
	}

	lcs_cfg_file = argv[1];
	lcs_init_app();

	gettimeofday(&tv0,NULL);

	lcs_load_shmagent(&err);
	
	if(err)
	{
		return EXIT_SUCCESS;
	}

	lcs_unit_analysis(&err);
	gettimeofday(&tv1,NULL);
	diff = (tv1.tv_sec - tv0.tv_sec) * 1000000 + tv1.tv_usec - tv0.tv_usec;
	fprintf(stdout, "\n\ntime: %ld\n\n", diff);

	if(err)
	{
		return EXIT_SUCCESS;
	}
	fprintf(stdout, "\n\n\n\n\n\n\nA_000007_05m SUCCCESS.");
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
	char *path 			= 0;
	char *name 			= 0;
	int isexist 		= 0;

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
		*err = __LINE__;
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
			*err = 1; break;
		}
		lcs_file_text(query, &sql, err);
		if(!sql)
		{
			*err = 1;
			break;	
		}
		//fprintf(stdout, "sql: %s\n", sql);
		*err = sqlite3_open(db, &conn);
		if(*err)
		{
			break;
		}
        *err = sqlite3_exec(conn, "BEGIN TRANSACTION", 0, 0, &sErrMsg);
		if(*err)
		{
			break;
		}
		*err = sqlite3_exec(conn, sql, cb, fp, &sErrMsg);
		if(*err)
		{
			break;
		}
        *err = sqlite3_exec(conn, "END TRANSACTION", 0, 0, &sErrMsg);
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
	if(sql)
	{
		free(sql);
	}
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
int  lcs_exec_cbtrue(void *fp, int n, char** val, char** headers)
{
	int i									  	= 0;
	int t									  	= 0;
	int h									  	= -1;
	int *arr									= 0;
	lcs_mmap_agents	*p1							= 0;

	/* +++++++++++++++++++++*/	

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
        fprintf((FILE*)fp, "%s\t", val[i++]);
	}
	while( i < 6);
	/*
	for(i = 0; i < 6; ++i)
	{
        fprintf((FILE*)fp, "%s\t", val[i]);
	}
	*/
	/* +++++++++++++++++++++*/	
	fprintf((FILE*) fp, "%s\t", (p1) ? (p1->vcontext):("NULL")  );
	/* +++++++++++++++++++++*/	
	do
	{
        fprintf((FILE*)fp, "%s\t", val[i++]);
	}
	while( i < 8);
	/*
	for(i = 6; i < 8; ++i)
	{
        fprintf((FILE*)fp, "%s\t", val[i]);
	}
	*/
	/* +++++++++++++++++++++*/	
	fprintf((FILE*) fp, "%s\t", (p1) ? (p1->name):("NULL")  );
	/* +++++++++++++++++++++*/	
    fprintf((FILE*)fp, "%s\t", val[i]);
	/*
	for(i = 8; i < 9; ++i)
	{
        fprintf((FILE*)fp, "%s\t", val[i]);
	}
	*/
	/* +++++++++++++++++++++*/	
	if(p1)
	{
		fprintf((FILE*) fp, "%s\t", 
			(strstr(val[9], p1->fixed_extension)) ? (val[10]):("0")  );
	}
	else
	{
		fprintf((FILE*) fp, "%s\t", 
			(strstr(val[9], "NULL")) ? (val[10]):("0")  );
	}
	/* +++++++++++++++++++++*/	
    fprintf((FILE*)fp, "%s\t", val[n - 1]);
	/* +++++++++++++++++++++*/	
    fprintf((FILE*)fp, "\n");
	return 0;
}
/***********************************************************************************/
int  lcs_exec_cb(void* data, int ncols, char** values, char** headers)
{
	int h 		= -1;
	int i 		= 0;
	int t		= 0;
	int *arr 	= 0;
	char * tmp = 0;
	FILE *fp = data;

    for(i=0; i < ncols; i++) {
        fprintf(fp, "%s\t", values[i]);
    }
	i--;
	lcs_comm_hash(values[i], strlen(values[i]) , &h, lcs_agt->n);
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
			if( strcmp(lcs_agt_ah[arr[t]].a.agent_number, values[i]) == 0)
			{
				tmp = lcs_agt_ah[arr[t]].a.name;	
    			fprintf(fp, "%s\t", lcs_agt_ah[arr[t]].a.name);
				break;
			}
			++t;
		}
		if(!tmp)
		{
    		fprintf(fp, "%s\t", "Add agent info here!!!");
		}
	}
    fprintf(fp, "\n");
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
void lcs_load_shmagent(int *error)
{
	int sz 		= 0;
	int err 	= 0;
	char key[256];

	do
	{
		memset(key, 0, 256);
		sprintf(key, "%s.agent.num", lcs_info_cfg.prefixshmkey);
		sz = sizeof(LCS_AGENTS_SHARED);
		lcs_agt = lcs_read_shmagent(key, sz, &err);
		if(!lcs_agt)
		{
			err = __LINE__;	
			break;
		}

		memset(key, 0, 256);
		sprintf(key, "%s.agent.data", lcs_info_cfg.prefixshmkey);
		sz = (lcs_agt->n + 1) * sizeof(LCS_AGENTS_SHARED_AH);
		lcs_agt_ah = lcs_read_shmagent(key, sz, &err);
		if(!lcs_agt_ah)
		{
			err = __LINE__;
			break;	
		} 
	}
	while(0);
	*error = err;
}
/***********************************************************************************/
