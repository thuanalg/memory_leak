#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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

typedef struct LCS_ANA_PARAM_THREAD
{
	char cmd[1024];
	char path[1024];
	char process[128];
} LCS_ANA_PARAM_THREAD;

typedef struct LCS_ANALYSIS_INFO
{
	char root_path[1024];
	char dbname[1024];
	char server[1024];
	char list_ana[250][9];
	char date[12];
	char week[12];
	char month[12];
	char year[12];
	
}
LCS_ANALYSIS_INFO;

/***********************************************************************************/
static void lcs_init_app();
static void	*lcs_ana_date(void* );
static void lcs_cfg_load(LCS_ANALYSIS_INFO *, int *);
static void lcs_load_shm(LCS_ANALYSIS_INFO *, int *);
static void lcs_increase_count(pthread_mutex_t *, int *);
static void lcs_decrease_count(pthread_mutex_t *, int *);
static void lcs_run_ana(LCS_ANALYSIS_INFO *p, char*,int *);
static void lcs_cfg_item(LCS_ANALYSIS_INFO *, char *, int *);
static void lcs_count_get(pthread_mutex_t *, int *in, int *out);
static void lcs_run_list_analysis(LCS_ANALYSIS_INFO *, int *err);
static void lcs_analysis_date_config(LCS_ANALYSIS_INFO*,char*,char*,char**,int*);
/***********************************************************************************/
static	char 				 	*lcs_cfg_file 	= 0;
static	int 				 	lcs_count 		= 0;
static	unsigned int		 	lcs_arr_count[5]; 
static	LCS_ANALYSIS_INFO	 	lcs_info_cfg;
static	pthread_mutex_t			lcs_mtx = PTHREAD_MUTEX_INITIALIZER;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	struct timeval t0;
	struct timeval t1;
	if(argc < 2)
	{
		err = __LINE__;
		fprintf(stdout, LCS_RETURN_STATUS, "analysis", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	gettimeofday(&t0, 0);
	lcs_cfg_file = argv[1];
	lcs_init_app();
	lcs_load_shm(&lcs_info_cfg, &err);
	lcs_run_list_analysis(&lcs_info_cfg, &err);
	gettimeofday(&t1, 0);
	lcs_diff_time(&t1, &t0, (char *)__FILE__, (char *)__FUNCTION__, __LINE__);

	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "lcs_analysis", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, LCS_RETURN_STATUS, "lcs_ananlysis", "EXIT_SUCCESS", err);
	return EXIT_SUCCESS;
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load(LCS_ANALYSIS_INFO *info, int *err)
{
	FILE *fp 		= 0;
	char buf[4096];
	
	fp = fopen(lcs_cfg_file, "r");
	if(!fp)
	{
		*err = __LINE__;
		return;
	}
	do
	{
		memset(buf, 0, 4096);
		if(fgets(buf,4096 -1, fp))	
		{
			lcs_cfg_item(info, buf, err);
		}
		else
		{
			break;
		}
	}
	while(1);
}
/***********************************************************************************/
/* Store text into data structure
 * info		:	output, it is to store configured information
 * item		:	text, input
 * error	:	error code
 */
/***********************************************************************************/
void lcs_cfg_item(LCS_ANALYSIS_INFO *info, char *item, int *err)
{
	char *base = 0;
	unsigned int sz = 0;
	do
	{
		sz = strlen(item);
		item[sz -1] = 0;

		base = "root_path_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->root_path, &item[sz], strlen(item)-sz);
			break;
		}
		base = "server_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->server, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dbname_0003:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbname, &item[sz], strlen(item)-sz);
			break;
		}

		base = "list_ana_0004:";
		if(strstr(item, base))
		{
			int i 	= 0;
			char *t = 0;
			char *p = 0;

			sz = strlen(base);
			t = &item[sz];
			p = strtok(t, ",\t\n");
			while(p)
			{
				memcpy(info->list_ana[i++], p, strlen(item)-sz);
				p = strtok(0, ",\t\n");
			}
			break;
		}

		base = "date_0005:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->date, &item[sz], strlen(item)-sz);
			break;
		}

		base = "week_0006:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->week, &item[sz], strlen(item)-sz);
			break;
		}

		base = "month_0007:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->month, &item[sz], strlen(item)-sz);
			break;
		}

		base = "year_0008:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->year, &item[sz], strlen(item)-sz);
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
	memset(&lcs_info_cfg, 0 , sizeof(LCS_ANALYSIS_INFO));
	lcs_cfg_load(&lcs_info_cfg, &err);
}
/***********************************************************************************/
void lcs_run_list_analysis
(LCS_ANALYSIS_INFO *p, int *err)
{
	int i 		= 0;
	int count 	= 0;
	do
	{
		if(strlen( (p->list_ana) [i]) < 1)
		{
			break;
		}
		lcs_run_ana(p, (p->list_ana)[i], err);		
		++i;
	}
	while(1);
	do
	{
		lcs_count_get(&lcs_mtx, &lcs_count, &count);
		if(!count)
		{
			break;
		}
		sleep(2);
	}
	while(1);
}
/***********************************************************************************/
void lcs_run_ana(LCS_ANALYSIS_INFO *p, char *ana, int *err)
{
	char process[128];
	char *target = 0;
	int having = 0;
	const char *array [] = 
		{
			"year", "month", "week", "day", 0
		};
	
	do
	{
		int i = 0 ;
		while(array[i])
		{
			memset(process, 0, 128);
			sprintf(process, "./lcs_%s_%s", ana, array[i]);
			lcs_comm_filexist(process, &having);
			if(having)
			{
				target = process;
				break;
			}

			memset(process, 0, 128);
			sprintf(process, "../%s_%s/lcs_%s_%s", 
						ana, array[i], ana, array[i]);
			lcs_comm_filexist(process, &having);
			if(having)
			{
				target = process;
				break;
			}
			++i;
		}
		if(!target)
		{
			*err = __LINE__;
			break;
		}
		if(strcmp(array[i], "day") == 0)		
		{
			char cmd[1024];
			char *path 		= 0;
			//pthread_t pid 	= 0;

			lcs_analysis_date_config(p, ana, process, &path, err);
			if(path)
			{
				LCS_ANA_PARAM_THREAD *param = 0;
				memset(cmd, 0, 1024);
				sprintf(cmd, "%s %s > /dev/null", process, path);
				param = calloc(1, sizeof( LCS_ANA_PARAM_THREAD));
				strcat(param->process, process);
				strcat(param->path, path);
				strcat(param->cmd, cmd);
				//pthread_create(&pid, 0, lcs_ana_date, param);
				lcs_ana_date(param);	
				lcs_increase_count(&lcs_mtx, &lcs_count);
			
				free(path);
			}
			else
			{
				*err = __LINE__;
			}
			break;	
		}
	}
	while(0);
}

/***********************************************************************************/
void lcs_analysis_date_config
(LCS_ANALYSIS_INFO *p, char *ana, char *process, char **path, int *err)
{
	int i 		= 0;
	FILE *fp 	= 0;
	char date[15];
	char datef[15];

	memset(date, 0, 15);
	memset(datef, 0, 15);
	strcat(date, p->date);
	strcat(datef, p->date);
	while(date[i])
	{
		if(date[i] == '-')
		{
			date[i] = '_';
		}
		++i;
	}
	i = 0;
	while(datef[i])
	{
		if(datef[i] == '-')
		{
			datef[i] = '/';
		}
		++i;
	}

	do
	{
		char subprocess[128];
		char result[128];
		memset(subprocess, 0, 128);
		memset(result, 0, 128);
		sprintf(subprocess, "../%s_hour/lcs_%s_hour", ana, ana);
		lcs_comm_get_process(subprocess, result, err);
		lcs_bash_cmdext("uuidgen", path, err);	
		if( ! (*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}
		fprintf(fp, "targetpath_0001:%s/%s/%s/%s\n", 
			p->root_path, p->server, p->dbname, datef);
		fprintf(fp, "name_ana_0002:%s\n", ana);
		fprintf(fp, "subprocess_0003:%s\n", result);
		fprintf(fp, "prefixshmkey_0004:%s.%s.%s_%s", 
			getenv("USER"), p->server, p->dbname, date);
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
	sleep(2);
}
/***********************************************************************************/
void *lcs_ana_date(void* arg)
{
	int err 						= 0;
	LCS_ANA_PARAM_THREAD *p 		= 0;

	p = arg;
	if(!p) 
	{ 
		return 0; 
	}
	lcs_bash_cmd(p->cmd, &err);

	remove(p->path);
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
void lcs_load_shm(LCS_ANALYSIS_INFO *p, int *err)
{
}
/***********************************************************************************/
