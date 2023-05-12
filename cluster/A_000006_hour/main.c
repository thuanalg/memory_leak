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

	#define lcs_cfg_file		lcs_yyyyyy_000001	
	#define lcs_cfg_item		lcs_yyyyyy_000002
	#define lcs_cfg_load		lcs_yyyyyy_000003
	#define lcs_info_cfg		lcs_yyyyyy_000006
	#define lcs_init_app		lcs_yyyyyy_000007

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

typedef struct LCS_UNIT_INFO
{
	char dir[1024];
	char sub[64];
	char ana[64];
} LCS_UNIT_INFO;
/***********************************************************************************/
static void lcs_init_app();
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_hcollect_ana(char *, char **, int *err);
static void lcs_hcollect_sum(char *, FILE *,int *err);
static void lcs_unit_gen(char *, char *, char *, int *err);
static void lcs_unit_config(char *, char *, char *, char *, int *);
static void lcs_get_count(pthread_mutex_t *, unsigned int *, int *);
static void lcs_increase_count(pthread_mutex_t *, int *);
static void lcs_decrease_count(pthread_mutex_t *, int *);
static void *lcs_unit_thread(void *);
/***********************************************************************************/
static	char 				*lcs_cfg_file = 0;
static	unsigned int		 lcs_arr_count[5]; 
static	unsigned int		 lcs_count_thread = 0; 
static	pthread_mutex_t		 lcs_mtx_count = PTHREAD_MUTEX_INITIALIZER; 
static  LCS_TEMPLATE_INFO	 lcs_info_cfg;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	int i = 0;
	int count = 0;
	char **arrdir = 0;
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_comm_subdir_ext(lcs_info_cfg.targetpath, &arrdir, &count, &err);
	if(arrdir)
	{
		lcs_quick_sort((void**)arrdir, 0, count - 1);
		lcs_hcollect_ana(lcs_info_cfg.targetpath, arrdir, &err);
		while(arrdir[i])
		{
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
	unsigned int count = 0;
	char tmp[1024];
	char tmpA[1024];
	char *text = 0;
	FILE *fp = 0;
	int isexist = 0;
	LCS_UNIT_INFO *p = 0;
	pthread_t id = 0;

	while(sub[i])	
	{
		text = 0;
		memset(tmp, 0, 1024);
		sprintf(tmp, "%s/%s/%s.analysis", dir, sub[i], lcs_info_cfg.name_ana);
		//fprintf(stdout, "ana: %s\n", tmp);
		lcs_comm_filexist(tmp, &isexist);
		fprintf(stdout, "\n%s\n", text);
		if(!isexist)
		{
			p = calloc(1, sizeof(LCS_UNIT_INFO));
			strcat(p->dir, dir);
			strcat(p->sub, sub[i]);
			strcat(p->ana, lcs_info_cfg.name_ana);
			lcs_increase_count(&lcs_mtx_count, err);
			//lcs_unit_gen(dir, sub[i], lcs_info_cfg.name_ana, err);
			pthread_create(&id, 0, lcs_unit_thread, p);
			usleep(100);
		}

		++i;
	}	
	
	while(1)
	{
		lcs_get_count(&lcs_mtx_count, &count, err);
		if(count == 0)
		{
			break;
		}
		usleep(100);
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
				lcs_hcollect_sum(text, fp, err);	
			}
			free(text);
		}
		++i;
	}	

	fclose(fp);
}
/***********************************************************************************/
void lcs_hcollect_sum(char *text, FILE *fp, int *err)
{
	fprintf(fp, "%s", text);	
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
void lcs_get_count(pthread_mutex_t *mtx, unsigned int *output, int *err)
{
	pthread_mutex_lock(mtx);
		*output = lcs_count_thread;
	pthread_mutex_unlock(mtx);
}
/***********************************************************************************/
void lcs_increase_count(pthread_mutex_t *mtx, int *err)
{
	pthread_mutex_lock(mtx);
		lcs_count_thread++;
	pthread_mutex_unlock(mtx);
}
/***********************************************************************************/
void lcs_decrease_count(pthread_mutex_t *mtx, int *err)
{
	pthread_mutex_lock(mtx);
		lcs_count_thread--;
	pthread_mutex_unlock(mtx);
}
/***********************************************************************************/
void *lcs_unit_thread(void *arg)
{
	int err = 0;
	LCS_UNIT_INFO *p = arg;
	lcs_unit_gen(p->dir, p->sub, p->ana, &err);
	lcs_decrease_count(&lcs_mtx_count, &err);
	free(arg);
	return 0;
}

/***********************************************************************************/
