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
}
LCS_TEMPLATE_INFO;

/***********************************************************************************/
static void lcs_init_app();
static void lcs_unit_analysis(int *);
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_exec_sql(char *qpath, char *path, char *file, int *err);
static int  lcs_exec_cbtrue(void* data, int ncols, char** values, char** headers);
/***********************************************************************************/
static	char 						*lcs_cfg_file 	= 0;
static  LCS_TEMPLATE_INFO			lcs_info_cfg;
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

	fprintf(stdout, "\n\n\n\n\n\n\nA_000005_05m SUCCESS.");
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
int  lcs_exec_cbtrue(void* fp, int n, char** val, char** headers)
{
	int i = 0;
	do
	{
		fprintf((FILE*) fp, "%s\t", val[i++]);
	}
	while ( i < n);
	fprintf((FILE*) fp, "\n");
	return 0;
}

/***********************************************************************************/
