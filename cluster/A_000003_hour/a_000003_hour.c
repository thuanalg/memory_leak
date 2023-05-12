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
static void lcs_hcollect_sum(char *, int *err);
static void lcs_unit_gen(char *, char *, char *, int *err);
void lcs_unit_config(char *dir, char *sub, char *ana, char *path, int *err);
/***********************************************************************************/
static	char 				*lcs_cfg_file = 0;
static	char				*lcs_arr_key[] = 
									{"ANSWERED", "BUSY","FAILED","NO ANSWER", 0};
static	unsigned int		 lcs_arr_count[5]; 
static  LCS_TEMPLATE_INFO	lcs_info_cfg;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	int i = 0;
	char **arrdir = 0;
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_comm_subdir(lcs_info_cfg.targetpath, &arrdir, &err);
	lcs_hcollect_ana(lcs_info_cfg.targetpath, arrdir, &err);
	if(arrdir)
	{
		while(arrdir[i])
		{
			//fprintf(stdout, "arr[%.2d]: %s\n", i, arrdir[i]);
			free(arrdir[i]);
			++i;
		}
		free(arrdir);
	}
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
	char *text = 0;
	FILE *fp = 0;
	while(sub[i])	
	{
		text = 0;
		memset(tmp, 0, 1024);
		sprintf(tmp, "%s/%s/%s.analysis", dir, sub[i], lcs_info_cfg.name_ana);
		//fprintf(stdout, "ana: %s\n", tmp);
		lcs_file_text(tmp, &text, err);
		fprintf(stdout, "\n%s\n", text);
		if(!text)
		{
			lcs_unit_gen(dir, sub[i], lcs_info_cfg.name_ana, err);
			lcs_file_text(tmp, &text, err);
		}
		if(text)
		{
			if(text[0])
			{
				lcs_hcollect_sum(text, err);	
			}
			free(text);
		}
		++i;
	}	
	memset(tmp, 0, 1024);
	sprintf(tmp, "%s/%s.analysis", dir, lcs_info_cfg.name_ana);
	fp = fopen(tmp, "w+");
	if(!fp)
	{
		*err = 1;
		return;
	}
	i = 0;
	while(lcs_arr_key[i])
	{
		fprintf(fp, "%u\t%s\n", 
			lcs_arr_count[i], lcs_arr_key[i]);
		++i;
	}
	fclose(fp);
}
/***********************************************************************************/
void lcs_hcollect_sum(char *text, int *err)
{
	int i 							= 0;
	int j 							= 0;
	char *pch 						= 0;
	unsigned long int count 		= 0;

	char tmp[64];

	pch = strtok(text, "\r\n");
	while(pch)
	{
		count = 0;
		memset(tmp, 0, 64);
		sscanf(pch, "%lu\t%s\t", &count, tmp);
		if(tmp[0])
		{
			j = 0;
			while(lcs_arr_key[j])
			{
				//fprintf(stdout, "++++++%u, %s, %s\n", lcs_arr_count[j], lcs_arr_key[j], tmp);
				if(strstr(lcs_arr_key[j], tmp) == lcs_arr_key[j])
				{
					lcs_arr_count[j] += count;
					break;
				}	
				++j;
			}
		}
		pch = strtok(0, "\r\n");
		++i;
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
