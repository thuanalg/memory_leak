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
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD

#endif
/***********************************************************************************/

/***********************************************************************************/

typedef struct LCS_TEMPLATE_INFO
{
	char targetpath[1024];
	char name_ana[64];
}
LCS_TEMPLATE_INFO;

/***********************************************************************************/
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_init_app();
static void lcs_unit_analysis(int *);

static void lcs_exec_sql(char *query, char *db, char *writepath, int *err);
static int  lcs_exec_cbtrue(void* data, int ncols, char** val, char** headers);
/***********************************************************************************/
static	char 				*lcs_cfg_file = 0;
static	unsigned int		 lcs_arr_count[5]; 
static  LCS_TEMPLATE_INFO	lcs_info_cfg;
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
	lcs_unit_analysis(&err);
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
	memset(lcs_arr_count, 0, sizeof(unsigned int) * 5);
	memset(&lcs_info_cfg, 0 , sizeof(LCS_TEMPLATE_INFO));
	lcs_cfg_load(&lcs_info_cfg, &err);

}
/***********************************************************************************/
void lcs_unit_analysis(int *err)
{
	char *path 				= 0;
	char *name			 	= 0;
	int isexist 			= 0;

	char query[1024];
	char dbpath[1024];
	char writepath[1024];

	path = lcs_info_cfg.targetpath;
	name = lcs_info_cfg.name_ana;
	memset(dbpath, 0, 1024);
	sprintf(dbpath, "%s/lcs.db", path);
	lcs_comm_filexist(dbpath, &isexist);
	if(!isexist)
	{
		fprintf(stdout, "No db\n");
		return;
	}

	memset(writepath, 0, 1024);
	sprintf(writepath, "%s/%s.analysis", path, name);

	memset(query, 0, 1024);
	sprintf(query, "../sql/%s.sql", name);

	lcs_exec_sql(query, dbpath, writepath, err);
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
		fprintf((FILE *)fp, "%s\t", val[i++]);
	}
	while(i < n);	
	fprintf((FILE *)fp, "\n");
	return 0;
}
/***********************************************************************************/

