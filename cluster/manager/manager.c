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

#define LCS_MYSQL_CMD		"get-mysql.cmd"
#define LCS_COUNT_TABLE		3
#define LCS_NUMBER_TABLE_THREAD				(LCS_COUNT_TABLE + 1)
#define LCS_CDR_TABLE_THREAD				0
#define LCS_QUE_TABLE_THREAD				1
#define LCS_MON_TABLE_THREAD				2

/***********************************************************************************/
#ifdef LCS_RELEASE_MOD

#endif

/***********************************************************************************/

typedef struct LCS_MYSQL_SERVER_INFO_LIST
{
	int n;
	char **listserver;
	struct LCS_MYSQL_SERVER_INFO *listinfo;

	char startdate[32];
	char currentdate[32];
	char lastdate[32];
	char iscomplete;
}
LCS_MYSQL_SERVER_INFO_LIST;

typedef struct LCS_MYSQL_SERVER_INFO
{
	char server[128];
	char user[64];
	char password[64];
	char db[64];
	char rootpath[512];

	char startdate[32];
	char currentdate[32];
	char lastdate[32];

	char agentprocess[128];
	char templateprocess[128];
	char insertprocess[128];
	char gethistoryprocess[128];
	char getfrommysqlprocess[128];
	char analysisprocess[128];
	char shmagentprocess[128];
	char shmauddprocess[128];

	char listtable[2048];
	char listtableagent[2048];
	char listanalysis[2048];

	LCSUCHAR iscompleted;

	MYSQL *conn;
}
LCS_MYSQL_SERVER_INFO;


typedef void (*LCS_SIG_CALLBACK)(int, siginfo_t *, void *);

typedef struct LCS_GET_TABLE_INFO
{
	char table[64];
	char out_file[64];
	long long int total;
	MYSQL **con;
	struct LCS_MYSQL_SERVER_INFO *gbinfo;

} LCS_GET_TABLE_INFO;

/***********************************************************************************/
static void lcs_hdl_cmd();
static void lcs_init_sig();
static void lcs_init_app();
static void lcs_get_status(char*);
static void lcs_set_status(char);
static void lcs_get_count(int *);
static void lcs_daemonize(int *);
static void lcs_increase_count();
static void lcs_decrease_count();
static void lcs_start_bdata(void *, int *);
static void *lcs_thread_spec_date(void *arg);
static void lcs_get_agent(void *p, int *err);
static void lcs_manager_shm(void *arg, int *);
static void lcs_try_close(MYSQL **con, int *);
static void lcs_get_history(void *arg, int *);
static void lcs_gen_template(void *, int *err);
static void lcs_get_from_mysql(void *arg, int *);
static void lcs_man_shm_unlink(void *, int *err);
static void lcs_try_query(MYSQL **con, void*, int *);
static void	lcs_man_shm_audd(void *, char *, int *);
static void	lcs_man_shm_agent(void *, char*, int *err);
static void lcs_hdl_signal(int sig, siginfo_t *, void *);
static void lcs_get_history_cfg(void *arg, char **, int *);
static void lcs_gen_analysis_cfg(void *arg, char **, int *);
static void lcs_try_connect(LCS_MYSQL_SERVER_INFO *, int *);
static void lcs_gen_agent_config(void *arg, char **, int *);
static void lcs_hdl_alrm(siginfo_t *siginfo, void *context);
static void lcs_get_from_mysql_cfg(void *arg, char **, int *);
static void lcs_gen_template_config(void *arg, char **, int *);
static void lcs_cfg_load(char*, LCS_MYSQL_SERVER_INFO *, int *);
static void lcs_cfg_item(LCS_MYSQL_SERVER_INFO *, char *, int *);
static void lcs_run_analysis(LCS_MYSQL_SERVER_INFO *p, int *err);
static void lcs_next_day(LCS_MYSQL_SERVER_INFO_LIST *p, int *err);
static void lcs_cfg_load_list(LCS_MYSQL_SERVER_INFO_LIST *, int *);
static void	lcs_man_shm_audd_cfg(void *, char *, char **, int *);
static void	lcs_man_shm_agent_cfg(void *, char*, char **, int *err);
static void lcs_init_server_info(LCS_MYSQL_SERVER_INFO_LIST *p, int *err);
/***********************************************************************************/
static  LCS_MYSQL_SERVER_INFO_LIST		lcs_info_cfg_list;
static  char 					LCS_APP_EXITED 		= 0;
static	pthread_t				lcs_primary_thread	= 0;
static	char					*lcs_mysql_cfg 		= 0;
static	int						lcs_count_thread 	= 0;
static  pthread_mutex_t			lcs_mtx_app = PTHREAD_MUTEX_INITIALIZER;	
static  pthread_mutex_t			lcs_mtx_thread = PTHREAD_MUTEX_INITIALIZER;	
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err				= 0;
	char status 		= 0;
	struct timeval t0, t1;
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	gettimeofday(&t0, 0);	
	lcs_mysql_cfg = calloc(1, 512);
	strcat(lcs_mysql_cfg, argv[1]);
	lcs_init_app();
	lcs_init_server_info(&lcs_info_cfg_list, &err);	
	
	lcs_start_bdata( &lcs_info_cfg_list, &err);
	do
	{	
		lcs_get_status(&status);
		fprintf(stdout, "status: %d\n", (int)status);
		if(status)
		{
			break;
		}
		pause();
		fprintf(stdout, "status: %d\n", (int)status);
		lcs_hdl_cmd();
	} while(1);
	gettimeofday(&t1, 0);	
	lcs_diff_time(&t1, &t0, (char *)__FILE__, (char *)__FUNCTION__, __LINE__);
	return EXIT_SUCCESS;
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load
(char *path, LCS_MYSQL_SERVER_INFO *info, int *err)
{
	char *text = 0;
	char *pch = 0;
	lcs_file_text(path, &text, err);
	fprintf(stdout, "text: %s\n", text);
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
void lcs_cfg_item
(LCS_MYSQL_SERVER_INFO *info, char *item, int *err)
{
	char *base 			= 0;
	unsigned int sz 	= 0;
	char pro[512];
	do
	{
		base = "server_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->server, &item[sz], strlen(item)-sz);
			break;
		}
		base = "user_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->user, &item[sz], strlen(item)-sz);
			break;
		}
		base = "password_0003:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->password, &item[sz], strlen(item)-sz);
			break;
		}
		base = "db_0004:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->db, &item[sz], strlen(item)-sz);
			break;
		}

		base = "rootpath_0005:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->rootpath, &item[sz], strlen(item)-sz);
			break;
		}

		base = "templateprocess_0006:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->templateprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->templateprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->templateprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->templateprocess);
			break;
		}

		base = "agentprocess_0007:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->agentprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->agentprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->agentprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->agentprocess);
			break;
		}

		base = "insertprocess_0008:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->insertprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->insertprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->insertprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->insertprocess);
			break;
		}


		base = "gethistoryprocess_0009:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->gethistoryprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->gethistoryprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->gethistoryprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->gethistoryprocess);
			break;
		}

		base = "getfrommysqlprocess_0010:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->getfrommysqlprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->getfrommysqlprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->getfrommysqlprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->getfrommysqlprocess);
			break;
		}

		base = "analysisprocess_0011:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->analysisprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->analysisprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->analysisprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->analysisprocess);
			break;
		}

		base = "listtable_0012:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->listtable, &item[sz], strlen(item)-sz);
			break;
		}

		base = "listanalysis_0013:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->listanalysis, &item[sz], strlen(item)-sz);
			break;
		}

		base = "listtableagent_0014:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->listtableagent, &item[sz], strlen(item)-sz);
			break;
		}

		base = "shmagent_0015:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->shmagentprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->shmagentprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->shmagentprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->shmagentprocess);
			break;
		}

		base = "shmaudd_0016:";
		if(strstr(item, base))
		{
			memset(pro, 0, 512);	
			sz = strlen(base);
			memcpy(info->shmauddprocess, &item[sz], strlen(item)-sz);
			lcs_comm_get_process(info->shmauddprocess, pro, err);
			if(strlen(pro))
			{
				memcpy(info->shmauddprocess, pro, strlen(pro) + 1);
			}
			fprintf(stdout, "%s\n", info->shmauddprocess);
			break;
		}

	}
	while(0);	
}
/***********************************************************************************/
/* Connect to mysql server with specific information
 * info		:	input
 * con		:	output, is formation about a connection
 * err		:	error code
 *
 */
/***********************************************************************************/
void lcs_try_connect(LCS_MYSQL_SERVER_INFO *info, int *err)
{
	do
	{
		info->conn = mysql_init(0);
		if(!(info->conn))
		{
			*err = 1;
			break;
		}
		if (mysql_real_connect(info->conn, info->server, info->user, info->password, 
				info->db, 0, 0, 0) == 0) 
		{
			fprintf(stderr, "%s\n", mysql_error(info->conn));
			lcs_try_close(&(info->conn), err);
			break;
		} 
		{
			fprintf(stdout, "Connect OK\n");
		}
	}
	while(0);
}
/***********************************************************************************/
/* Try to execute a query along with a connection
 * con		:	input, a connection
 * query	:	input, a cmd
 * err		:	error code
 */
/***********************************************************************************/
void lcs_try_query(
MYSQL **con, void *arg, int *err)
{
}
/***********************************************************************************/
/* Try to clode a mysql connection 
 * con		:		input
 * error	:		error code
 */
/***********************************************************************************/
void lcs_try_close(MYSQL **con, int *err)
{
	if(*con)
	{
		mysql_close(*con);
	}
	*con = 0;
}
/***********************************************************************************/
/* A thread to get a table such as cdr, monitor_recording, queue_log, ...
 * Depend on arg, we have an action
 * arg		:		argument of thread.
 * More information, later
 *
 */
/***********************************************************************************/
void *lcs_get_tab_pthread(void *arg)
{
	int err	= 0;
	LCS_GET_TABLE_INFO *p = 0;
	p = arg;
	lcs_try_query(p->con, p, &err);
	return 0;
}
/***********************************************************************************/
/* Get status of app, if 1, will exit app; if 0, continue
 * status		:		output variable
 *
 */
/***********************************************************************************/
void lcs_get_status(char *status)
{
	pthread_mutex_lock(&lcs_mtx_app);
		*status = LCS_APP_EXITED;
	pthread_mutex_unlock(&lcs_mtx_app);
}
/***********************************************************************************/
/* To tell app should exit or not. 1: should exit, 0: should not exit
 * stattus		:		0 or 1
 *
 */
/***********************************************************************************/
void lcs_set_status(char status)
{
	pthread_mutex_lock(&lcs_mtx_app);
		LCS_APP_EXITED = status;
	pthread_mutex_unlock(&lcs_mtx_app);
}
/***********************************************************************************/
/* To register handling specific signal
 * Conforming To: POSIX.1-2001, SVr4.  
 * https://linux.die.net/man/2/sigaction
 */
/***********************************************************************************/
void lcs_init_sig()
{
	struct sigaction act;
	memset (&act, 0, sizeof(act));
	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_sigaction = &lcs_hdl_signal;
 
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_flags = SA_SIGINFO;
 
	if (sigaction(SIGALRM, &act, 0) < 0) 
	{
		perror ("sigaction");
		exit(EXIT_FAILURE);
	}
}
/***********************************************************************************/
/* A callback function to handle signal
 *
 */
/***********************************************************************************/
void lcs_hdl_signal(int sig, siginfo_t *siginfo, void *context)
{
	if(sig == SIGALRM)
	{
		lcs_hdl_alrm(siginfo, context);
		return;
	}
}
/***********************************************************************************/
/* Handle signal a specific signal: SIGALRM
 *
 */
/***********************************************************************************/
void lcs_hdl_alrm(siginfo_t *siginfo, void *context)
{
	
}
/***********************************************************************************/
/* when receive a signal ( ex: SIGALRM) from other process, this function need
 * know specific order, to have suitable action.
 * The brigded file is LCS_MYSQL_CMD
 *
 */
/***********************************************************************************/
void lcs_hdl_cmd()
{
	char *cmd = 0;
	int err = 0;
	lcs_file_text(LCS_MYSQL_CMD, &cmd, &err);
	do
	{
		if(!cmd)
		{
			break;
		}
		fprintf(stdout, "cmd: %s\n", cmd);
		if(strstr(cmd, "LCS_EXIT"))
		{
			fprintf(stdout, "cmd: %s exit set\n", cmd);
			lcs_set_status(1);
			break;
		}

		if(strstr(cmd, "LCS_MEMORY"))
		{
			fprintf(stdout, "cmd: %s exit set\n", cmd);
			break;
		}

	}
	while(0);

	if(cmd)
	{
		free(cmd);
	}
}
/***********************************************************************************/
/* Start/init backup data to warehouse
 * data		:	...
 * err		:	error code
 */
/***********************************************************************************/
void lcs_start_bdata(void *data, int *err)
{
	int  i 			= 0;
	int count 		= 0;
	pthread_t id 	= 0;

	LCS_MYSQL_SERVER_INFO_LIST *p  = data;
	do
	{
		for(i = 0; i < p->n; ++i)
		{
			//gentemplate
			lcs_gen_template(&(p->listinfo[i]), err);
			if(*err)
			{
				break;
			}
			//get agent
			lcs_get_agent(&(p->listinfo[i]), err);
			if(*err)
			{
				break;
			}
		}
		if(*err)
		{
			break;
		}
		do
		{
			//set current date
			lcs_next_day(p, err);
			fprintf(stdout, "currentdate: %s\n", p->currentdate);
			if(p->iscomplete)
			{
				break;
			}
			for(i = 0; i < p->n; ++i)
			{
				//get history
				//getfrommysql
				LCS_MYSQL_SERVER_INFO *info = 0;
				info = &((p->listinfo)[i]);
				
				if(strcmp(info->startdate, p->currentdate) <= 0 &&
					strcmp(p->currentdate, info->lastdate) <= 0)
				{
					memcpy( info->currentdate, 
						p->currentdate, strlen(p->currentdate));
					pthread_create(&id, 0,lcs_thread_spec_date, info);
					lcs_increase_count();
				}
			}
			while(1)
			{
				lcs_get_count( &count);
				fprintf(stdout, "Number of thread: %d\n", count);
				if(!count)
				{
					break;
				}
				sleep(15);
			}
		}
		while(1);
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
	lcs_daemonize(&err);
	lcs_primary_thread = pthread_self();	
	lcs_init_sig();
	lcs_set_status(0);
	lcs_cfg_load_list(&lcs_info_cfg_list, &err);

}
/***********************************************************************************/
void lcs_daemonize(int *err)
{
	
}
/***********************************************************************************/
void lcs_get_inittime( LCS_MYSQL_SERVER_INFO *p, int *err)
{
	MYSQL_ROW row;
	MYSQL_RES *result = 0;

	char *query = "select DATE(MIN(calldate)), DATE(MAX(calldate)) from cdr;";

	lcs_try_connect(p, err);
	do
	{
		if(p->conn == 0)
		{
			break;
		}
		if (mysql_query(p->conn, query))
		{  
			fprintf(stderr, "%s\n", mysql_error(p->conn));
			break;
		}
		result = mysql_store_result(p->conn);

		if (result == 0) 
		{
			break;
		}
		while ((row = mysql_fetch_row(result)))
		{
			sprintf(p->startdate, "%s", row[0]);
			sprintf(p->lastdate, "%s", row[1]);
			fprintf(stdout, "start: %s, end: %s\n", p->startdate, p->lastdate);
			break;
		}
		mysql_free_result(result);
	}
	while(0);
	if(p->conn)
	{
		mysql_close(p->conn);	
	}
}
/***********************************************************************************/
void lcs_get_range(char *mile, char **low, char **hi)
{
}
/***********************************************************************************/
void lcs_cfg_load_list
(LCS_MYSQL_SERVER_INFO_LIST *list, int *err)
{
	int n 						= 0;
	char *text 					= 0;
	char *pch 					= 0;
	LCS_MYSQL_SERVER_INFO *tmp 	= 0;

	lcs_file_text(lcs_mysql_cfg, &text, err);
	fprintf(stdout, "text: %s\n", text);
	list->listserver = calloc(1, 1000 * sizeof(void *));	
	pch = strtok(text, " \r\n");
	while(pch)
	{
		list->listserver[n] = calloc(1, strlen(pch) + 1);
		strcat(list->listserver[n], pch);
		pch = strtok(0, " \r\n");
		++n;
	}
	if(text)
	{
		free(text);
	}
	if(n)
	{
		list->n = n;
		tmp = calloc(1, n * sizeof(LCS_MYSQL_SERVER_INFO));
		list->listserver = realloc(list->listserver, (n + 1) * sizeof(void *));	
		n = 0;
		while(list->listserver[n])
		{
			lcs_cfg_load(list->listserver[n], 
				&(tmp[n]),err);
			++n;
		}
		list->listinfo = tmp;
	}
}
/***********************************************************************************/
void lcs_init_server_info
(LCS_MYSQL_SERVER_INFO_LIST *list, int *err)
{
	//Should use thread here.
	int i							= 0;
	char *min						= 0;
	char *max						= 0;
	LCS_MYSQL_SERVER_INFO *p	 	= 0;

	p = list->listinfo;
	for(i = 0; i < list->n; ++i)
	{
		lcs_get_inittime( &(p[i]), err);
		if(*err)
		{
			break;
		}
		if(!min)
		{
			if(strlen(p[i].startdate))
			{
				min = p[i].startdate;
			}
		}
		else
		{
			if(strlen(p[i].startdate))
			{
				min = (strcmp(min, p[i].startdate) > 0) ? p[i].startdate : min;
			}
		}
		
		if(!max)
		{
			if(strlen(p[i].lastdate))
			{
				max = p[i].lastdate;
			}
		}
		else
		{
			if(strlen(p[i].lastdate))
			{
				max = (strcmp(max, p[i].lastdate) < 0) ? p[i].lastdate : max;
			}
		}
	}
	if( min	&& max)
	{
		strcat(list->startdate, min);
		strcat(list->lastdate, max);
	}
}
/***********************************************************************************/
void lcs_gen_template(void *arg, int *err)
{
	char * path 					= 0;
	LCS_MYSQL_SERVER_INFO *p 		= 0;

	char cmd[1024];

	p = arg;
	lcs_gen_template_config(p, &path, err);
	if(!path)
	{
		return;	
	}
	
	memset(cmd, 0, 1024);
	sprintf(cmd, "%s %s > /dev/null", p->templateprocess, path);
	fprintf(stdout, "cmd: %s\n", cmd);
	lcs_bash_cmd(cmd, err);
	if(path)
	{
		remove(path);
		free(path);
	}
}
/***********************************************************************************/
void lcs_get_agent(void *arg, int *err)
{

	char * path 					= 0;
	LCS_MYSQL_SERVER_INFO *p 		= 0;

	char cmd[1024];

	p = arg;
	lcs_gen_agent_config(p, &path, err);

	if(!path)
	{
		return;	
	}
	
	memset(cmd, 0, 1024);
	sprintf(cmd, "%s %s > /dev/null", p->agentprocess, path);
	fprintf(stdout, "cmd: %s\n", cmd);
	lcs_bash_cmd(cmd, err);
	if(path)
	{
		remove(path);
		free(path);
	}
}
/***********************************************************************************/
void lcs_next_day(LCS_MYSQL_SERVER_INFO_LIST *p, int *err)
{
	char *outtime			= 0;
	long long int t 		= 0;
	char tmp[32];

	do
	{
		if(strlen(p->startdate) == 0)
		{
			p->iscomplete = 1;
			break;
		}
		if(strlen(p->lastdate) == 0)
		{
			p->iscomplete = 1;
			break;
		}
		if(strlen(p->currentdate) == 0)
		{
			strcat(p->currentdate, p->startdate);
			break;
		}
		if(strcmp(p->currentdate, p->lastdate) >= 0)
		{
			p->iscomplete = 1;
			break;
		}
		memset(tmp, 0, 32);
		sprintf(tmp, "%s 00:00:00", p->currentdate);
		lcs_str2_time(tmp, &t);
		t += 24 * 3600;
		lcs_time2_str( (time_t*)&t, &outtime);
		if(outtime)
		{
			fprintf(stdout, "outtime: %s\n", outtime);
			memset(p->currentdate, 0, 32);
			memcpy(p->currentdate, outtime, 10);
			free(outtime);
			break;
		}
		else
		{
			*err = __LINE__;
			break;
		}
	}
	while(0);
}
/***********************************************************************************/
void *lcs_thread_spec_date(void *arg)
{
	int err 					= 0;
	LCS_MYSQL_SERVER_INFO *p 	= arg;

	do
	{
		lcs_get_history(arg, &err);
		fprintf(stdout, "complete history server: %s, on %s\n", 
			p->server, p->currentdate);
		if(err)
		{
			break;
		}
		lcs_get_from_mysql(arg, &err);
		fprintf(stdout, "complete mysql server: %s, on: %s\n", 
			p->server, p->currentdate);
		if(err)
		{
			break;
		}
	}
	while(0);
	if(!err)
	{
		fprintf(stdout, "complete download server: %s, on %s\n", 
				p->server, p->currentdate);
		lcs_run_analysis(p, &err);
		fprintf(stdout, "complete analyze server in on day : %s, on %s\n", 
				p->server, p->currentdate);
	}
	lcs_decrease_count();
	return 0;
}
/***********************************************************************************/
void lcs_gen_template_config(void *arg, char **path, int *err)
{

	FILE *fp 							= 0;
	LCS_MYSQL_SERVER_INFO *p 			= arg;

	do
	{
		lcs_bash_cmdext("uuidgen", path, err);	
		if( !(*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
//		fprintf(stdout, "%d\n", *path);
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}
		fprintf(fp, "server:%s\n", p->server);
		fprintf(fp, "user:%s\n", p->user);
		fprintf(fp, "password:%s\n", p->password);
		fprintf(fp, "db:%s\n", p->db);
		fprintf(fp, "path:%s/%s/%s/template\n", p->rootpath, p->server, p->db);
		fprintf(fp, "pathtemplate:template.sql\n");
		fprintf(fp, "listtable:%s", p->listtable) ;	
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}

}
/***********************************************************************************/
void lcs_gen_agent_config(void *arg, char **path, int *err)
{
	
	FILE *fp 						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;

	do
	{
		lcs_bash_cmdext("uuidgen", path, err);	
		if( !(*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
//		fprintf(stdout, "%d\n", *path);
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}

		fprintf(fp, "server:%s\n", p->server);
		fprintf(fp, "user:%s\n", p->user);
		fprintf(fp, "password:%s\n", p->password);
		fprintf(fp, "db:%s\n", p->db);
		fprintf(fp, "pathtemplate_0001:%s/%s/%s/template\n", p->rootpath, p->server, p->db);
		fprintf(fp, "pathagent_0002:%s/%s/%s/agent\n", p->rootpath, p->server, p->db);
		fprintf(fp, "listtable_0003:%s\n", p->listtableagent);
		//fprintf(fp, "listtable_0003:agents,auto_dial,auto_dial_callback,context,sip_addition,sip_users,user,user_context,queues,nation,province,feature_code,mobile_phone_operator\n");
		fprintf(fp, "insertprocess_0004:%s", p->insertprocess);
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
void lcs_get_history(void *arg, int *err)
{
	char *path 							= 0;
	LCS_MYSQL_SERVER_INFO *p 			= 0;
	char cmd[1024];

	p = arg;
	lcs_get_history_cfg(arg, &path, err);

	if(!path)
	{
		return;	
	}
	
	memset(cmd, 0, 1024);
	sprintf(cmd, "%s %s > /dev/null", p->gethistoryprocess, path);
	fprintf(stdout, "cmd: %s\n", cmd);
	lcs_bash_cmd(cmd, err);
	if(path)
	{
		remove(path);
		free(path);
	}

}
/***********************************************************************************/
void lcs_get_from_mysql(void *arg, int *err)
{
	char *path 							= 0;
	LCS_MYSQL_SERVER_INFO *p 			= 0;
	char cmd[1024];

	p = arg;
	lcs_get_from_mysql_cfg(arg, &path, err);


	if(!path)
	{
		return;	
	}
	
	memset(cmd, 0, 1024);
	sprintf(cmd, "%s %s > getfrommysql.log.%s.%s", 
			p->getfrommysqlprocess, path, p->currentdate, p->server);
	fprintf(stdout, ">>>>>cmd: %s\n", cmd);
	lcs_bash_cmd(cmd, err);
	if(path)
	{
		remove(path);
		free(path);
	}
}
/***********************************************************************************/
void lcs_get_history_cfg(void *arg, char **path, int *err)
{
	int i							= 0;
	FILE *fp						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;
	char tmp[32];
	do
	{
		lcs_bash_cmdext("uuidgen", path, err);	
		if( !(*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
//		fprintf(stdout, "%d\n", *path);
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}

		fprintf(fp, "server:%s\n", p->server);
		fprintf(fp, "user:%s\n", p->user);
		fprintf(fp, "password:%s\n", p->password);
		fprintf(fp, "db:%s\n", p->db);
		fprintf(fp, "date_0001:%s\n", p->currentdate);

		strcat(tmp, p->currentdate);
		while(tmp[i])
		{
			if(tmp[i] == '-')
			{
				tmp[i] = '/';
			}
			++i;
		}
		fprintf(fp, "pathhistory_0002:%s/%s/%s/%s\n", 
				p->rootpath, p->server, p->db, tmp);
		fprintf(fp, "listtable_0003:DND_History,agent_history,auto_dial_detail\n");
		fprintf(fp, "pathtemplate_0004:%s/%s/%s/template\n", 
				p->rootpath, p->server, p->db);
		fprintf(fp, "insertprocess_0005:%s", p->insertprocess);
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
void lcs_get_from_mysql_cfg(void *arg, char **path, int *err)
{
	FILE *fp 						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;

	do
	{
		lcs_bash_cmdext("uuidgen", path, err);	
		if( !(*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}

		fprintf(fp, "server:%s\n", p->server);
		fprintf(fp, "user:%s\n", p->user);
		fprintf(fp, "password:%s\n", p->password);
		fprintf(fp, "db:%s\n", p->db);
		fprintf(fp, "path:%s\n", p->rootpath);
		fprintf(fp, "date_0011:%s\n", p->currentdate);

		fprintf(fp, "step:60\n");
		fprintf(fp, "max_duration:900\n");
		fprintf(fp, "insertprocess_0009:%s", p->insertprocess);
	}
	while(0);

	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
void lcs_increase_count()
{
	pthread_mutex_lock(&lcs_mtx_thread);
		lcs_count_thread++;
	pthread_mutex_unlock(&lcs_mtx_thread);
}
/***********************************************************************************/
void lcs_decrease_count()
{
	pthread_mutex_lock(&lcs_mtx_thread);
		lcs_count_thread--;
	pthread_mutex_unlock(&lcs_mtx_thread);
}
/***********************************************************************************/
void lcs_get_count(int *val)
{
	pthread_mutex_lock(&lcs_mtx_thread);
		*val = lcs_count_thread;
	pthread_mutex_unlock(&lcs_mtx_thread);
}
/***********************************************************************************/
void lcs_run_analysis(LCS_MYSQL_SERVER_INFO *arg, int *err)
{
	char *path 					= 0;
	LCS_MYSQL_SERVER_INFO *p 	= arg;
	char cmd[1024];

	lcs_manager_shm(arg, err);	
	lcs_gen_analysis_cfg(arg, &path, err);	

	do
	{
		if(!path)
		{
			break;
		}
		memset(cmd, 0, 1024);
		sprintf(cmd, "%s %s > analysis.log.%s.%s", 
				p->analysisprocess, path, p->currentdate, p->server);
		fprintf(stdout, ">>>>>cmd: %s\n", cmd);
		lcs_bash_cmd(cmd, err);
	}
	while(0);
	
	if(path)
	{
		remove(path);
		free(path);
	}
	lcs_man_shm_unlink(arg, err);
}
/***********************************************************************************/
void lcs_gen_analysis_cfg(void *arg, char **path, int *err)
{
	FILE *fp						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;

	do
	{
		lcs_bash_cmdext("uuidgen", path, err);	
		if( !(*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
//		fprintf(stdout, "%d\n", *path);
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}

		fprintf(fp, "root_path_0001:%s\n", p->rootpath);
		fprintf(fp, "server_0002:%s\n", p->server);
		fprintf(fp, "dbname_0003:%s\n", p->db);
		fprintf(fp, "list_ana_0004:%s\n", p->listanalysis);
		fprintf(fp, "date_0005:%s\n", p->currentdate);
		fprintf(fp, "week_0006:%s\n", p->currentdate);
		fprintf(fp, "month_0007:%s\n", "");
		fprintf(fp, "year_0008:%s", "");
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
void lcs_manager_shm(void *arg, int *err)
{
	int i 							= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;
	char key[1024];
	char date[12];

	memset(key, 0, 1024);
	memset(date, 0, 12);
	strcat(date, p->currentdate);
	while(date[i])	
	{
		if(date[i] == '-')
		{
			date[i] = '_';
		}
		++i;
	}
	//113.61.111.198.asterisk_demo_1day_2017_10_29
	sprintf(key, "%s.%s.%s_%s", 
		getenv("USER"), p->server, p->db, date);
	lcs_man_shm_agent(arg, key, err);
	lcs_man_shm_audd(arg, key, err);
}
/***********************************************************************************/
void lcs_man_shm_audd(void *arg, char *key, int *err)
{
	char *path 						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;
	char cmd[1024];

	lcs_man_shm_audd_cfg(arg, key, &path, err);

	do
	{
		if(!path)
		{
			break;
		}
		memset(cmd, 0, 1024);
		sprintf(cmd, "%s %s > shmaudd.log.%s.%s", 
				p->shmauddprocess, path, p->currentdate, p->server);
		fprintf(stdout, ">>>>>cmd: %s\n", cmd);
		lcs_bash_cmd(cmd, err);
	}
	while(0);
	
	if(path)
	{
		remove(path);
		free(path);
	}
}
/***********************************************************************************/
void lcs_man_shm_agent(void *arg, char *key, int *err)
{
	char *path 						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;
	char cmd[1024];

	lcs_man_shm_agent_cfg(arg, key, &path, err);

	do
	{
		if(!path)
		{
			break;
		}
		memset(cmd, 0, 1024);
		sprintf(cmd, "%s %s > shmagent.log.%s.%s", 
				p->shmagentprocess, path, p->currentdate, p->server);
		fprintf(stdout, ">>>>>cmd: %s\n", cmd);
		lcs_bash_cmd(cmd, err);
	}
	while(0);
	
	if(path)
	{
		remove(path);
		free(path);
	}
}
/***********************************************************************************/
void lcs_man_shm_audd_cfg
(void *arg, char *key, char **path, int *err)
{
	int i 							= 0;
	FILE *fp						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;
	char date[12];

	memset(date, 0, 12);
	strcat(date, p->currentdate);
	do
	{
		while(date[i])
		{
			if(date[i] == '-')
			{
				date[i] = '/';
			}
			++i;
		}
		lcs_bash_cmdext("uuidgen", path, err);	
		if( !(*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
//		fprintf(stdout, "%d\n", *path);
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}
		fprintf(fp, "dbpath_0001:%s/%s/%s/%s\n", 
			p->rootpath, p->server, p->db,date);
		fprintf(fp, "prefixshmkey_0002:%s", key);
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
void lcs_man_shm_agent_cfg
(void *arg, char *key, char **path, int *err)
{
	FILE *fp						= 0;
	LCS_MYSQL_SERVER_INFO *p 		= arg;

	do
	{
		lcs_bash_cmdext("uuidgen", path, err);	
		if( !(*path))
		{
			break;
		}
		(*path)[strlen(*path) - 1] = 0;
//		fprintf(stdout, "%d\n", *path);
		fp = fopen(*path, "w+")	;	
		if(!fp)
		{
			break;
		}
		fprintf(fp, "dbpath_0001:%s/%s/%s/agent\n", 
			p->rootpath, p->server, p->db);
		fprintf(fp, "prefixshmkey_0002:%s", key);
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
void lcs_man_shm_unlink(void *arg, int *err)
{
	int i                           = 0;
	char **result 					= 0;
	LCS_MYSQL_SERVER_INFO *p        = arg;
	char key[1024];
	char date[12];

	memset(key, 0, 1024);
	memset(date, 0, 12);
	strcat(date, p->currentdate);
	while(date[i])
	{   
		if(date[i] == '-')
		{   
			date[i] = '_';
		}
		++i;
	}
	//113.61.111.198.asterisk_demo_1day_2017_10_29
	sprintf(key, "%s.%s.%s_%s", 
		getenv("USER"), p->server, p->db, date);
	lcs_comm_list_files("/dev/shm", &result, err);
	if(result)
	{
		i = 0;
		while(result[i])
		{
			if(strstr(result[i], key))
			{
				shm_unlink(result[i]);
			}
			free(result[i]);
			++i;
		}
		free(result);
	}
}
/***********************************************************************************/
