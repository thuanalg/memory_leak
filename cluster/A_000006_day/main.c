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
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_init_app();

static void lcs_hcollect_ana(char *, char **, int *err);
static void lcs_hcollect_sum(char *, char *, int *err);
static void lcs_dgen_subtask(char *, char *, char *, int *);
static void lcs_dgen_config(char*, char*, char *, char*, int *);
/***********************************************************************************/
static	char 				*lcs_cfg_file = 0;
static	unsigned int		 lcs_arr_count[5]; 
static  LCS_TEMPLATE_INFO	lcs_info_cfg;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	int i = 0;
	char **arrdir = 0;
	time_t t0;
	time_t t1;
	int count = 0;
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	time(&t0);
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_comm_subdir_ext(
		lcs_info_cfg.targetpath, &arrdir, &count, &err);
	if(arrdir)
	{
		lcs_quick_sort((void **)arrdir, 0, count -1);
		lcs_hcollect_ana(lcs_info_cfg.targetpath, arrdir, &err);
		while(arrdir[i])
		{
			fprintf(stdout, "%s\n", arrdir[i]);
			free(arrdir[i]);
			++i;
		}
		free(arrdir);
	}
	time(&t1);
	fprintf(stdout, "elapsed: %f\n", difftime(t1, t0));
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
	memset(lcs_arr_count, 0, sizeof(unsigned int) * 5);
	memset(&lcs_info_cfg, 0 , sizeof(LCS_TEMPLATE_INFO));
	lcs_cfg_load(&lcs_info_cfg, &err);
}
/***********************************************************************************/
void lcs_hcollect_ana(char *dir, char **sub, int *err)
{
	int i = 0;
	char tmp[1024];
	int isexist = 0;
	char dest[1024];
	int count = 0;

	memset(dest, 0, 1024);
	sprintf(dest, "%s/%s.analysis", dir, lcs_info_cfg.name_ana);
	while(sub[i])	
	{
		memset(tmp, 0, 1024);
		sprintf(tmp, "%s/%s/%s.analysis", dir, sub[i], lcs_info_cfg.name_ana);
	
		fprintf(stdout, "ana: %s\n", tmp);
		lcs_comm_filexist(tmp, &isexist);
		if(!isexist)
		{
			lcs_dgen_subtask(dir, sub[i], lcs_info_cfg.name_ana, err);
			count++;
		}
		++i;
		//fprintf(stdout, "\n%s\n", text);
	}	
	if(count)
	{
		remove(dest);
	}
	i = 0;
	while(sub[i])
	{
		memset(tmp, 0, 1024);
		sprintf(tmp, "%s/%s/%s.analysis", dir, sub[i], lcs_info_cfg.name_ana);
		lcs_hcollect_sum(dest, tmp, err);	
		++i;
	}
}
/***********************************************************************************/
void lcs_hcollect_sum(char *dest, char *src, int *err)
{
	char cmd[2048];

	memset(cmd, 0, 2048);
	sprintf(cmd, "cat %s >> %s", src, dest);
	lcs_bash_cmd(cmd, err);
}
/***********************************************************************************/
void lcs_dgen_subtask(char *day, char *hour, char *ana, int *err)
{
	char path[1024];
	char cmd[1024];
	memset(path, 0, 1024);
	memset(cmd, 0, 1024);
	lcs_dgen_config(day, hour, ana, path, err);	
	if(*err)
	{
		return;
	}
	sprintf(cmd, "%s %s > /dev/null", lcs_info_cfg.subprocess,path);
	fprintf(stdout, "cmd: %s\n", cmd);
	lcs_bash_cmd(cmd, err);
}
/***********************************************************************************/
void lcs_dgen_config
(char *day, char *hour, char *ana, char *path, int *err)
{
	FILE *fp = 0;
	sprintf(path, "%s/%s/%s.config.cfg", day, hour, ana);	
	fp = fopen(path, "w+");
	if(!fp)
	{
		*err = 1;
		return;	
	}
	fprintf(fp, "targetpath_0001:%s/%s", day, hour);
	fprintf(fp, "\n");
	fprintf(fp, "name_ana_0002:%s\n", ana);
	fprintf(fp, "subprocess_0003:../A_000006_05m/lcs_A_000006_05m");
	fclose(fp);
}
/***********************************************************************************/
