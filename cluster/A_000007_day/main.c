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
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD
#endif
/***********************************************************************************/

typedef struct LCS_TEMPLATE_INFO
{
	char targetpath[1024];
	char name_ana[64];
	char subprocess[1024];
	char prefixshmkey[512];
}
LCS_TEMPLATE_INFO;

typedef struct LCS_ANA007_HOUR
{
	char sub[6];
	char ana[64];
	char dir[1024];
}
LCS_ANA007_HOUR;

/***********************************************************************************/
static void lcs_cfg_load(LCS_TEMPLATE_INFO *, int *);
static void lcs_cfg_item(LCS_TEMPLATE_INFO *, char *, int *);
static void lcs_init_app();

static void lcs_hcollect_ana(char *, char **, int *err);
static void lcs_hcollect_sum(char *, char *, int *err);
static void lcs_dgen_subtask(char *, char *, char *, int *);
static void lcs_dgen_config(char*, char*, char *, char*, int *);
static void	*lcs_ana_007(void* );
static void lcs_increase_count(pthread_mutex_t *, int *);
static void lcs_decrease_count(pthread_mutex_t *, int *);
static void lcs_count_get(pthread_mutex_t *, int *in, int *out);
/***********************************************************************************/
static	char 				 	*lcs_cfg_file 	= 0;
static	int 				 	lcs_count 		= 0;
static	LCS_TEMPLATE_INFO	 	lcs_info_cfg;
static	pthread_mutex_t			lcs_mtx = PTHREAD_MUTEX_INITIALIZER;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	int i = 0;
	char **arrdir = 0;
	struct timeval t0;
	struct timeval t1;
	int count = 0;
	if(argc < 2)
	{
		err = __LINE__;
		fprintf(stdout, LCS_RETURN_STATUS, "A_000007_day", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	gettimeofday(&t0, 0);
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_comm_subdir_ext(
		lcs_info_cfg.targetpath, &arrdir, &count, &err);
	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "A_000007_day", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
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
	gettimeofday(&t1, 0);
	lcs_diff_time(&t1, &t0, (char*)__FILE__,(char *) __FUNCTION__,__LINE__);
	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "A_000007_day", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}

	fprintf(stdout, LCS_RETURN_STATUS, "A_000007_day", "EXIT_SUCCESS", err);
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

		base = "prefixshmkey_0004:";
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
void lcs_hcollect_ana(char *dir, char **sub, int *err)
{
	int i 				= 0;
	int isexist 		= 0;
	int count 			= 0;
	pthread_t id 		= 0;
	LCS_ANA007_HOUR *p 	= 0;
	char tmp[1024];
	char dest[1024];

	memset(dest, 0, 1024);
	sprintf(dest, "%s/%s.analysis", dir, lcs_info_cfg.name_ana);
	while(sub[i])	
	{
		memset(tmp, 0, 1024);
		sprintf(tmp, "%s/%s/%s.analysis", dir, sub[i], lcs_info_cfg.name_ana);
	
		//fprintf(stdout, "ana: %s\n", tmp);
		lcs_comm_filexist(tmp, &isexist);
		if(!isexist)
		{
			//lcs_dgen_subtask(dir, sub[i], lcs_info_cfg.name_ana, err);

			p = calloc(1, sizeof(LCS_ANA007_HOUR));
			strcat(p->dir, dir);
			strcat(p->sub, sub[i]);
			strcat(p->ana, lcs_info_cfg.name_ana);
			lcs_increase_count(&lcs_mtx, &lcs_count);
			pthread_create(&id, 0, lcs_ana_007, p);
			//lcs_ana_007(p);
			count++;
		}
		++i;
		//fprintf(stdout, "\n%s\n", text);
	}	
	if(count)
	{
		remove(dest);
	}
	while(1)
	{
		lcs_count_get(&lcs_mtx, &lcs_count, &count);	
		if(!count)
		{
			break;	
		}
		sleep(1);
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
		*err = __LINE__;
		return;
	}
	sprintf(cmd, "%s %s > /dev/null", lcs_info_cfg.subprocess,path);
	//fprintf(stdout, "cmd: %s\n", cmd);
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
	fprintf(fp, "subprocess_0003:../A_000007_05m/lcs_A_000007_05m\n");
	fprintf(fp, "prefixshmkey_0004:%s", lcs_info_cfg.prefixshmkey);
	fclose(fp);
}
/***********************************************************************************/
void *lcs_ana_007(void* arg)
{
	int err 				= 0;
	LCS_ANA007_HOUR *p 		= 0;

	p = arg;
	if(!p) 
	{ 
		return 0; 
	}
	lcs_dgen_subtask(p->dir, p->sub, p->ana, &err);
	free(p);	
	lcs_decrease_count(&lcs_mtx, &lcs_count);
	return 0;
}
/***********************************************************************************/
void lcs_increase_count(pthread_mutex_t *mtx, int *val)
{
	pthread_mutex_lock(mtx);
		(*val)++;
	pthread_mutex_unlock(mtx);
}
/***********************************************************************************/
void lcs_decrease_count(pthread_mutex_t *mtx, int *val)
{
	pthread_mutex_lock(mtx);
		(*val)--;
	pthread_mutex_unlock(mtx);
}
/***********************************************************************************/
void lcs_count_get(pthread_mutex_t *mtx, int *in, int *out)
{
	pthread_mutex_lock(mtx);
		(*out) = (*in);
	pthread_mutex_unlock(mtx);
}
/***********************************************************************************/
