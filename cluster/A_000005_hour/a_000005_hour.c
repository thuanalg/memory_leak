#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>
#include <mysql.h>
#include <pthread.h>
#include <sqlite3.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <sys/time.h>
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
	char subprocess[1024];
}
LCS_TEMPLATE_INFO;


/***********************************************************************************/
static void lcs_init_app();
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_hcollect_ana(char *, char **, int *err);
static void lcs_unit_gen(char *, char *, char *, int *err);
static void lcs_unit_config(char *, char *, char *, char *, int *);

//static void lcs_hcollect005_sum(char*, char *, int *);
//static void lcs_merge005_list(LCS_MERGE_COUNTING_LIST*, int,char *, int *);
//static void lcs_list005_rows(FILE *, char **,int *err);
//

/***********************************************************************************/
static	char 				*lcs_cfg_file = 0;
static  LCS_TEMPLATE_INFO	 lcs_info_cfg;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	int i = 0;
	int count = 0;
	char **arrdir = 0;
	struct timeval t0, t1;
	long diff = 0;
	if(argc < 2)
	{
		err = __LINE__;
		fprintf(stdout, LCS_RETURN_STATUS, 
			"A_000005_hour", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	gettimeofday(&t0, 0);
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_comm_subdir_ext(lcs_info_cfg.targetpath, &arrdir, &count, &err);
	if(arrdir)
	{
		lcs_quick_sort((void**)arrdir, 0, count - 1);
		lcs_hcollect_ana(lcs_info_cfg.targetpath, arrdir, &err);
		if(err)
		{
			fprintf(stdout, "err: %d\n", err);
		}
		while(arrdir[i])
		{
			free(arrdir[i]);
			++i;
		}
		free(arrdir);
	}
	gettimeofday(&t1, 0);
	diff = (t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec;

	fprintf(stdout, "\n\ndiff: %ld\n\n", diff);
	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, 
			"A_000005_hour", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, LCS_RETURN_STATUS, "A_000005_hour", "EXIT_SUCCESS", err);

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

		base = "subprocess_0003:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->subprocess, &item[sz], strlen(item)-sz);
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
void lcs_hcollect_ana(char *dir, char **sub, int *err)
{
	int i = 0;
	char tmp[1024];
	char tmpA[1024];
	char *text = 0;
	FILE *fp = 0;
	int isexist = 0;

	while(sub[i])	
	{
		memset(tmp, 0, 1024);
		sprintf(tmp, "%s/%s/%s.analysis", dir, sub[i], lcs_info_cfg.name_ana);
		lcs_comm_filexist(tmp, &isexist);
		if(!isexist)
		{
			//fprintf(stdout, "%s:%d, ana: %s\n", __FUNCTION__, __LINE__, tmp);
			lcs_unit_gen(dir, sub[i], lcs_info_cfg.name_ana, err);
		}
		++i;
	}	

	memset(tmpA, 0, 1024);
	sprintf(tmpA, "%s/%s.analysis", dir, lcs_info_cfg.name_ana);
	remove(tmpA);
	fp = fopen(tmpA, "a+");
	i = 0;
	while(sub[i])	
	{
		text = 0;
		memset(tmp, 0, 1024);
		sprintf(tmp, "%s/%s/%s.analysis", dir, sub[i], lcs_info_cfg.name_ana);
		//fprintf(stdout, "ana: %s\n", tmp);
		lcs_file_text(tmp, &text, err);
		//fprintf(stdout, "\n%s\n", text);
		if(text)
		{
			if(text[0])
			{
				fprintf(fp, "%s", text);	
			}
			free(text);
		}
		++i;
	}
	text = 0;	
	lcs_list005_rows(fp, &text, err);
	fclose(fp);
	remove(tmpA);
	//strcat(tmpA, ".ntt");
	if(text)
	{
		lcs_hcollect005_sum(tmpA, text, err);
		free(text);
	}
}
/***********************************************************************************/

void lcs_unit_gen(char *dir, char *sub, char *ana, int *err)
{
	char path[1024];
	char cmd[2048];
	char *subprocess = 0;
	memset(path, 0, 1024);
	lcs_unit_config(dir, sub, ana, path, err);	
	memset(cmd, 0, 2048);
	subprocess = lcs_info_cfg.subprocess;
	sprintf(cmd, "%s %s", subprocess, path);	
	fprintf(stdout, "cmd: %s\n", cmd);
	lcs_bash_cmd(cmd, err);
}
/***********************************************************************************/
void lcs_unit_config(char *dir, char *sub, char *ana, char *path, int *err)
{
	FILE *fp = 0;
	sprintf(path, "%s/%s/%s.config.cfg", dir, sub, ana);
	fp = fopen(path, "w+");
	if(!fp)
	{
		*err = 1;
		return;
	}
	fprintf(fp, "targetpath_0001:%s/%s\n", dir, sub);
	fprintf(fp, "name_ana_0002:%s", ana);
	fclose(fp);
}

/***********************************************************************************/

