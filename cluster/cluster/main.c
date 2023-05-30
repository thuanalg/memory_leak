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

#define LCS_MYSQL_CMD		"get-mysql.cmd"
#define LCS_COUNT_TABLE		3
#define LCS_NUMBER_TABLE_THREAD				(LCS_COUNT_TABLE + 1)
#define LCS_CDR_TABLE_THREAD				0
#define LCS_QUE_TABLE_THREAD				1
#define LCS_MON_TABLE_THREAD				2

/***********************************************************************************/
#define	LCS_MIN_MAX_DATE_FORMAT "select MIN(calldate), MAX(calldate) \
from cdr where calldate >='%s 00:00:00' and calldate <= '%s 23:59:59';"

#define LCS_INSERT_LINKEDID_TMP	"insert into linkedid_tmp_%s (linkedid) \
(select distinct linkedid from cdr_%s where  (calldate >= '%s' AND calldate < '%s' ));"
/*
 * We must create one more table: "linkedid_tmp"
 *
 */

#define LCS_INSERT_CDR_TMP_FORMAT "insert into cdr_tmp_%s (recid) \
(select recid from \
(select recid, linkedid from cdr_%s where calldate < '%s'  ) as tmp_ntthuan \
where tmp_ntthuan.linkedid in (select linkedid from linkedid_tmp_%s)\
);"


#define LCS_INSERT_QUEUE_TMP_FORMAT "insert into queue_tmp_%s (id) \
(select id from \
(select id, callid from queue_log_%s  where time < '%s'   ) as tmp_ntthuan \
where tmp_ntthuan.callid in (select linkedid as callid from linkedid_tmp_%s)\
);"


#define LCS_INSERT_MONITOR_TMP_FORMAT "insert into monitor_tmp_%s (id) \
(select id from \
(select id, uniqueid from monitor_recording_%s where created_time < '%s'  ) as tmp_ntthuan \
where tmp_ntthuan.uniqueid in (select linkedid from linkedid_tmp_%s)\
);"

/***********************************************************************************/
/* Should move to common */
#define	LCS_LOG_INFO		1
#define	LCS_LOG_WARNING		2
#define	LCS_LOG_SERIOUS		3

/***********************************************************************************/
#ifdef LCS_RELEASE_MOD
	#define LCS_MYSQL_INFO				lcs_zzz_a00000 
	#define lcs_cfg_load  				lcs_zzz_a00001
	#define lcs_cfg_item  				lcs_zzz_a00003
	#define lcs_info_cfg  				lcs_zzz_a00004
	#define server		  				lcs_zzz_a00005
	#define user		  				lcs_zzz_a00006
	#define password	  				lcs_zzz_a00007
	#define db			  				lcs_zzz_a00008
	#define path		  				lcs_zzz_a00009
	#define starttime	  				lcs_zzz_a00010
	#define step		  				lcs_zzz_a00000
	#define year		  				lcs_zzz_000001
	#define month		  				lcs_zzz_000002
	#define day			  				lcs_zzz_000003
	#define hour		  				lcs_zzz_000004
	#define min			  				lcs_zzz_000005
	#define sec			  				lcs_zzz_000006
	#define LCS_APP_EXITED				lcs_zzz_000007
	#define lcs_mtx_app	  				lcs_zzz_000008
	#define lcs_get_status				lcs_zzz_000009
	#define lcs_set_status				lcs_zzz_000010
	#define lcs_init_sig				lcs_zzz_000011
	#define lcs_hdl_signal				lcs_zzz_000012
	#define lcs_hdl_alrm				lcs_zzz_000013
	#define lcs_hdl_cmd					lcs_zzz_000014
	#define lcs_start_bdata				lcs_zzz_000015
	#define lcs_get_tab_pthread			lcs_zzz_000016
	#define lcs_watcher_thread			lcs_zzz_000017
	#define lcs_try_query				lcs_zzz_000019
	#define lcs_try_close				lcs_zzz_000020
	#define lcs_try_connect				lcs_zzz_000021
	#define lcs_init_app				lcs_zzz_000023
	#define lcs_try_fetch				lcs_zzz_000024
	#define lcs_daemonize				lcs_zzz_000026
	#define lcs_mtx_log					lcs_zzz_000027
	#define lcs_mk_dir					lcs_zzz_000028
	#define lcs_get_date				lcs_zzz_000030
	#define lcs_fetch_row				lcs_zzz_000033
	#define lcs_cdr_format				lcs_zzz_000035
	#define lcs_backup_cdr				lcs_zzz_000038
	#define lcs_backup_monitor			lcs_zzz_000039
	#define lcs_backup_qlog				lcs_zzz_000040
	#define lcs_monitor_format			lcs_zzz_000043
	#define lcs_qlog_format				lcs_zzz_000045

#endif

/***********************************************************************************/

typedef struct LCS_MYSQL_INFO
{
	char server[128];
	char user[64];
	char password[64];
	char db[64];
	char path[128];
	//char starttime[64];
	char date[64];
	char date_01[64];
	char current_dir[1024];
	char insertprocess[1024];
	char **analysis;
	int	 step;
	char *low;
	char *hi;
	char *inittime;
	char *endtime;
	MYSQL *con;
}
LCS_MYSQL_INFO;


typedef struct LCS_DATE_DIR
{
	unsigned short 	year;
	unsigned char  	month;
	unsigned char 	day;
	unsigned char 	hour;
	unsigned char 	min;
}
LCS_DATE_DIR;

typedef void (*LCS_SIG_CALLBACK)(int, siginfo_t *, void *);

typedef struct LCS_GET_TABLE_INFO
{
	char table[64];
	char out_file[64];
	long long int total;
	MYSQL **con;
	struct LCS_MYSQL_INFO *gbinfo;

} LCS_GET_TABLE_INFO;

/***********************************************************************************/
static void lcs_cfg_load(LCS_MYSQL_INFO *, int *);
static void lcs_cfg_item(LCS_MYSQL_INFO *, char *, int *);
static void lcs_try_connect(LCS_MYSQL_INFO *, MYSQL **con, int *);
static void lcs_try_query(MYSQL **con, void*, int *);
static void lcs_try_fetch(void *, MYSQL_RES *, int  nfield, int *);
static void lcs_try_close(MYSQL **con, int *);
static void *lcs_get_tab_pthread(void *);
static void *lcs_watcher_thread(void *);
static void lcs_get_status(char *);
static void lcs_set_status(char);
static void lcs_init_sig();
static void lcs_hdl_signal(int sig, siginfo_t *siginfo, void *context);
static void lcs_hdl_alrm(siginfo_t *siginfo, void *context);
static void lcs_hdl_cmd();
static void lcs_init_app();
static void lcs_start_bdata(void *, int *);
static void lcs_daemonize(int *);
static void lcs_mk_dir(LCS_MYSQL_INFO *, void *, LCS_DATE_DIR *, int *);
static void lcs_get_date(char *, LCS_DATE_DIR **, int *);
static void lcs_backup_cdr(void*);
static void lcs_backup_qlog(void*);
static void lcs_backup_monitor(void *);
static void lcs_backup_hdial(void *);
//static void lcs_backup_ddial(void *);
static void lcs_get_inittime(char **, char **, int *);
static void lcs_get_range(char *mile, char **low, char **hi);
static void lcs_init_circle(MYSQL *, void *, int *);
static void lcs_end_circle(void *, int*);
static void lcs_gen_linkedid(MYSQL *, void *, int *);
static void lcs_update_time(void *, int *);
static void lcs_clear_tmp_table(MYSQL *con, char *, int *);
static void lcs_clear_history(void *, int *);
static void lcs_create_subdir(void *, int *);
static void lcs_init_param(char *, void **);
static void lcs_moving_param(char *);
static void lcs_get_outtime(void *, char *);
static void lcs_geninsert_cfg(char*, char**, int *);
//static void lcs_ana_leaf(void*, int *);
//static void lcs_get_method(char*, void *,int *);
static void lcs_clear_tmp(char *, int *);
static void lcs_init_tmp(char *, int *);
static void lcs_gen_structure(int *);
static void lcs_gen_data_tables(char *, char *, int *);
//static void lcs_multi_query(MYSQL *, char **, int *);
/***********************************************************************************/
static  LCS_MYSQL_INFO			lcs_info_cfg;
static  char 					LCS_APP_EXITED 		= 0;
static	pthread_t				lcs_primary_thread	= 0;
static  pthread_mutex_t			lcs_mtx_app = PTHREAD_MUTEX_INITIALIZER;	
static	char					*lcs_mysql_cfg = 0;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int 	err				= 0;
	char 	status 			= 0;
	struct timeval t0, t1;

	gettimeofday(&t0, 0);	
	do
	{
		if(argc < 2)
		{
			err = __LINE__;
		}

		lcs_mysql_cfg = argv[1];

		lcs_init_app();

		if(!(lcs_info_cfg.inittime))
		{
			break;
		}

		lcs_start_bdata(0, &err);

		if(err)
		{
			break;
		}
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
	}
	while(0);

	lcs_clear_tmp(lcs_info_cfg.date_01, &err);
	gettimeofday(&t1, 0);	
	lcs_diff_time(&t1, &t0, (char *)__FILE__, (char *)__FUNCTION__, __LINE__);

	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "lcs_getfrommysql", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, LCS_RETURN_STATUS, "lcs_getfrommysql", "EXIT_SUCCESS", err);
	return EXIT_SUCCESS;
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load(LCS_MYSQL_INFO *info, int *err)
{
	char *text = 0;
	char *pch = 0;
	lcs_file_text(lcs_mysql_cfg, &text, err);
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
void lcs_cfg_item(LCS_MYSQL_INFO *info, char *item, int *err)
{
	char *base = 0;
	unsigned int sz = 0;
	fprintf(stdout, "item: %s\n\n", item);
	do
	{
		base = "server:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->server, &item[sz], strlen(item)-sz);
			break;
		}
		base = "user:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->user, &item[sz], strlen(item)-sz);
			break;
		}
		base = "password:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->password, &item[sz], strlen(item)-sz);
			break;
		}
		base = "db:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->db, &item[sz], strlen(item)-sz);
			break;
		}

		base = "path:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->path, &item[sz], strlen(item)-sz);
			break;
		}

		base = "step:";
		if(strstr(item, base))
		{
			sscanf(item, "step:%d", &info->step);
			break;
		}

		base = "insertprocess_0009:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->insertprocess, &item[sz], strlen(item)-sz);
			break;
		}

		base = "date_0011:";
		if(strstr(item, base))
		{
			char *str = 0;
			sz = strlen(base);
			memcpy(info->date, &item[sz], strlen(item)-sz);
			memcpy(info->date_01, &item[sz], strlen(item)-sz);
			do
			{
				str = strstr(info->date_01, "-");
				if(!str)
				{
					break;
				};
				str[0] = '_';
			}
			while(1);
			fprintf(stdout, "info->date01: %s\n", info->date_01);
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
void lcs_try_connect(LCS_MYSQL_INFO *info, MYSQL **con, int *err)
{
	do
	{
		*con = mysql_init(0);
		if(!(*con))
		{
			*err = 1;
			break;
		}
		if (mysql_real_connect(*con, info->server, info->user, info->password, 
				info->db, 0, 0, 0) == 0) 
		{
			fprintf(stderr, "%s\n", mysql_error(*con));
			lcs_try_close(con, err);
			break;
		} 
		mysql_query(*con, "SET NAMES 'utf8';");
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
	char *supc 				= 0;
	char *date				= 0;
	int num_fields 			= 0;
	long long int sup 		= 0; 
	MYSQL_RES *result 		= 0;
	char * sql_insert 		= 0;
	char * sql_select 		= 0;
	LCS_GET_TABLE_INFO *p 	= 0;

	char tmp[1024];
	char tmp_insert[2048];
	time_t t0, t1;

	p = arg;
	date = lcs_info_cfg.date_01;

	if(!(*con))
	{
		*err = 1;
		return;
	}

	lcs_str2_time(p->gbinfo->hi, &sup);
	sup += 4800;
	lcs_time2_str((time_t *)&sup, &supc);	

	memset(tmp, 0, 1024);
	memset(tmp_insert, 0, 2048);
	if( strcmp(p->table, "cdr") == 0)
	{
		sprintf(tmp_insert, LCS_INSERT_CDR_TMP_FORMAT, 
			date, date, supc, date);

		//fprintf(stdout, "\n%s\n", tmp_insert);

		sql_select = "select * from cdr_%s where linkedid in \
(select linkedid from linkedid_tmp_%s) order by calldate;";	
		sprintf(tmp, sql_select, date, date);
		sql_select = tmp;
	
		fprintf(stdout, "sql_select: %s\n", sql_select);
	}	
	else if( strcmp(p->table, "queue_log") == 0)
	{
		sprintf(tmp_insert, LCS_INSERT_QUEUE_TMP_FORMAT, 
			date, date, supc, date);
		//fprintf(stdout, "\n%s\n", tmp_insert);

		sql_select = "select * from queue_log_%s where callid in (select linkedid from linkedid_tmp_%s);";	

		sprintf(tmp, sql_select, date, date);
		sql_select = tmp;
		fprintf(stdout, "sql_select: %s\n", sql_select);
	}	
	else if( strcmp(p->table, "monitor_recording") == 0)
	{
		sprintf(tmp_insert, LCS_INSERT_MONITOR_TMP_FORMAT, 
			date, date, supc, date);
		//fprintf(stdout, "\n%s\n", tmp_insert);

		sql_select = "select * from monitor_recording_%s where uniqueid in (select linkedid  from linkedid_tmp_%s);";	
		sprintf(tmp, sql_select, date, date);
		sql_select = tmp;
		fprintf(stdout, "sql_select: %s\n", sql_select);
	}	
/*
	else if( strcmp(p->table, "auto_dial_detail") == 0)
	{	
		sql_select = "select * from %s where created_time >= '%s' AND created_time < '%s';";
		sprintf(tmp, sql_select, p->table, p->gbinfo->low, p->gbinfo->hi);
		sql_select = tmp;
	}
*/
	else if( strcmp(p->table, "auto_dial_wrapup_history") == 0)
	{
		sql_select = "select * from %s_%s where start_time >= '%s' AND start_time < '%s';";
		sprintf(tmp, sql_select, 
			p->table, date, p->gbinfo->low, p->gbinfo->hi);
		sql_select = tmp;
	}

	do
	{
		fprintf(stdout, "p->total: %lld\n", p->total);
		time(&t0);
		if(sql_insert)
		{
			*err = mysql_query(*con, sql_insert);
		}
		{
			if(strlen(tmp_insert))
			{
				*err = mysql_query(*con, tmp_insert);
			}
		}
		if (*err)
		{  
			*err = 1;
			fprintf(stderr, "%s\n", mysql_error(*con));
			break;
		}
		time(&t1);
		//fprintf(stdout, "--------insert: %d\n", (int)difftime(t1, t0));

		time(&t0);
		if(sql_select)
		{
			//fprintf(stdout, "--------sql_select: %s\n", sql_select);
			*err =  mysql_query(*con, sql_select);
		}
		if (*err)
		{  
			*err = 1;
			fprintf(stderr, "%s\n", mysql_error(*con));
			break;
		}
		time(&t1);
		//fprintf(stdout, "-----------select: %d\n", (int)difftime(t1, t0));

		result = mysql_store_result(*con);

		if (result == 0) 
		{
			break;			
		}
		num_fields = mysql_num_fields(result);
		if(p) 
		{
			lcs_try_fetch( p, result, num_fields, err);
		}
	}
	while(0);

	if(result)
	{
		mysql_free_result(result);
	}
	if(*err)
	{
		lcs_try_close(con, err);
	}
	if(supc)
	{
		free(supc);
	}
}
/***********************************************************************************/
/* Render date from mysql
 * result	:	input, result with mysql data structure
 * nfileds	:	number of fields of this table, or data type
 * err		:	error code
 */
/***********************************************************************************/
void lcs_try_fetch
(void *arg, MYSQL_RES *result, int  nfields, int *err)
{
	int i 						= 0;
	int count 					= 0;
	FILE *fp 					= 0;
	char * pch 					= 0;
	LCS_GET_TABLE_INFO *p 		= 0;
	MYSQL_ROW row;
	char path[1024];
	char buf[4096 + 1];

	p = arg;

	memset(path, 0, 1024);	
	sprintf(path, "%s%s", p->gbinfo->current_dir, p->out_file);
	//fprintf(stdout, "data path: %s\n", path);
	fp = fopen(path, "w+");
	if(!fp)
	{
		*err = 1;
		return;
	}
/*
	lcs_fetch_field(fp, result, nfields, err);
*/
	while ((row = mysql_fetch_row(result))) 
	{ 
		count++;
		for(i = 0; i < nfields; i++) 
		{ 
			if(row[i])
			{
				if(row[i][0] == 0)
				{
					fprintf(fp, "NULL\t"); 
					continue;
				}
				if(row[i][0] == 1)
				{
					fprintf(fp, "true\t"); 
					continue;
				}
				memset(buf, 0, 4096 + 1);
				pch = strtok(row[i], "\t\r\n");
				if(pch)
				{
					sprintf(buf, "%s", pch);
				}
				while(pch)
				{
					pch = strtok(0, "\t\r\n");
					if(pch)
					{
						sprintf( &(buf[strlen(buf)]), "%s ", pch);
					}
				}
				fprintf(fp, "%s\t", buf); 
			}
			else 
			{
				fprintf(fp, "NULL\t"); 
			}
		} 
		fprintf(fp, "\n");           
	}
	fclose(fp);
	if(count)
	{
		p->total += count;
		fprintf(stdout, "%s, having data, count: %d.\n", p->table, count);
		fprintf(stdout, "%s, total data %lld.\n", p->table, p->total);
	}
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
	int err					= 0;
	LCS_GET_TABLE_INFO *p 	= 0;
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
	char is_exit 					= 0;
	char outtime 					= 0;
	pthread_t pid 					= 0;
	LCS_GET_TABLE_INFO *pc 			= 0;
	LCS_GET_TABLE_INFO *pq 			= 0;
	LCS_GET_TABLE_INFO *pm 			= 0;
	LCS_GET_TABLE_INFO *pwrapup 	= 0;

	struct timeval tv0, tv1;
	//LCS_GET_TABLE_INFO *pdetail = 0;
	lcs_gen_structure(err);
	lcs_init_tmp(lcs_info_cfg.date_01, err);
	lcs_gen_data_tables(lcs_info_cfg.date, lcs_info_cfg.date_01, err);

	pthread_create(&pid, 0, lcs_watcher_thread, 0);	

	//lcs_try_connect(&lcs_info_cfg, &(lcs_info_cfg.con), err);
	lcs_init_param("cdr", (void**)&pc);
	lcs_init_param("queue_log", (void **)&pq);
	lcs_init_param("monitor_recording", (void **)&pm);
	lcs_init_param("auto_dial_wrapup_history", (void **)&pwrapup);
	//lcs_init_param("auto_dial_detail", (void **)&pdetail);
	lcs_init_circle(*(pc->con), pc, err);

	do
	{
		gettimeofday(&tv0, 0); 
		lcs_clear_history(pwrapup, err);
		//lcs_clear_history(pdetail, err);
		lcs_get_outtime(pc, &outtime);
		if(outtime)
		{
			lcs_set_status(1);
			break;
		}
		lcs_backup_cdr(pc);
		lcs_backup_monitor(pq);
		lcs_backup_qlog(pm);
		lcs_backup_hdial(pwrapup);
		//lcs_backup_ddial(pdetail);

		lcs_moving_param(pc->gbinfo->current_dir);
		//lcs_ana_leaf(pc->gbinfo, err);
		lcs_end_circle(pc, err);

		lcs_get_status(&is_exit);
		if(is_exit)
		{
			break;
		}
		gettimeofday(&tv1, 0); 
		lcs_diff_time(&tv1, &tv0, (char *)__FILE__, (char *)__FUNCTION__, __LINE__);
		fprintf(stdout, "\n\n\n\n\n");
	}
	while(1);
	
	//lcs_try_close(&(lcs_info_cfg.con), err);	
	lcs_try_close(pc->con, err);	
	lcs_try_close(pq->con, err);	
	lcs_try_close(pm->con, err);	
	lcs_try_close(pwrapup->con, err);	
}
/***********************************************************************************/
/* Body of watcher thread, is to watch other threads as, "cdr" thread, 
 * "monitor_recording" , queue_log, .... It  is very important.
 * arg		:		argument for thread
 * Note : Watcher MUST/SHOULD be VERY simple and MUST be stable
 */
/***********************************************************************************/
void *lcs_watcher_thread(void *arg)
{
	char status = 0;
	do
	{
		sleep(1);
		lcs_get_status(&status);
	}
	while(!status);

	return 0;
}
/***********************************************************************************/
/* Init information/variables for whole process
 *
 */
/***********************************************************************************/
void lcs_init_app()
{
	int err = 0;
	memset(&lcs_info_cfg, 0 , sizeof(LCS_MYSQL_INFO));

	lcs_daemonize(&err);

	lcs_primary_thread = pthread_self();	
	lcs_init_sig();
	lcs_set_status(0);
	lcs_cfg_load(&lcs_info_cfg, &err);

	lcs_get_inittime(&(lcs_info_cfg.inittime), &(lcs_info_cfg.endtime), &err);

	if(lcs_info_cfg.inittime)
	{
		lcs_get_range(lcs_info_cfg.inittime, 
			&(lcs_info_cfg.low), &(lcs_info_cfg.hi));
	}
}
/***********************************************************************************/
void lcs_daemonize(int *err)
{
	
}
/***********************************************************************************/
void lcs_mk_dir
(LCS_MYSQL_INFO *info, void *arg, LCS_DATE_DIR *date, int *err)
{
	char tmp[1024];
	int sz = 0;
	LCS_GET_TABLE_INFO *p = 0;
	
	p = arg;
	memset(tmp, 0, 1024);

	sprintf(tmp, "%s/%s/%s/%4d/%02d/%02d/%02d/%02d/",
		info->path, info->server, info->db, 
		(int)(date->year), (int)(date->month),
		(int)(date->day), (int)(date->hour), (int)(date->min));	
	sz = strlen(tmp);

	if(p)
	{
		if(p->gbinfo->current_dir)
		{
			memcpy(p->gbinfo->current_dir, tmp, sz);
			p->gbinfo->current_dir[sz] = 0;
		}
	}
	lcs_create_dir(tmp, err);
}
/***********************************************************************************/
void lcs_get_date(char *starttime, LCS_DATE_DIR **date, int *err)
{
	int year = 0, month = 0, day = 0, hour = 0, min = 0;
	if((!starttime) || (!date))
	{
		*err = 1;
		return;	
	}
	*date = calloc(1, sizeof(LCS_DATE_DIR));
	sscanf(starttime, "%d-%d-%d %d:%d:00", 
		&year,
		&month,
		&day,
		&hour,
		&min);
	(*date)->year = year;
	(*date)->month = month;
	(*date)->day = day;
	(*date)->hour = hour;
	(*date)->min = min;
}
/***********************************************************************************/
void lcs_backup_cdr(void *arg)
{
	lcs_get_tab_pthread(arg);
}
/***********************************************************************************/
void lcs_backup_qlog(void *arg)
{
	lcs_get_tab_pthread(arg);
}
/***********************************************************************************/
void lcs_backup_monitor(void *arg)
{
	lcs_get_tab_pthread(arg);
}
/***********************************************************************************/
void lcs_get_inittime(char **res, char **res1, int *error)
{
	MYSQL *con = 0;
	int err = 0;
	MYSQL_RES *result = 0;
	MYSQL_ROW row;
	char query[256];

	memset(query, 0, 256);
	sprintf(query, LCS_MIN_MAX_DATE_FORMAT, 
	lcs_info_cfg.date, lcs_info_cfg.date);
	fprintf(stdout, "querey: %s\n", query);
	lcs_try_connect(&lcs_info_cfg, &con, &err);
	do
	{
		if(!con)
		{
			err = __LINE__;
			break;
		}
		if (mysql_query(con, query))
		{  
			err = __LINE__;
			break;
		}
		result = mysql_store_result(con);

		if (result == 0) 
		{
			break;
		}
		while ((row = mysql_fetch_row(result)))
		{
			if( !row[0])
			{
				err = __LINE__;
				break;
			}
			*res = calloc(1, 60);
			*res1 = calloc(1, 60);
			sprintf(*res, "%s", row[0]);
			sprintf(*res1, "%s", row[1]);
			fprintf(stdout, "start: %s, end: %s\n", *res, *res1);
			break;
		}
		mysql_free_result(result);
	}
	while(0);
	if(con)
	{
		mysql_close(con);	
	}
	*error = err; 
}
/***********************************************************************************/
void lcs_get_range(char *mile, char **low, char **hi)
{
	long long int t = 0;
	long long int tl = 0;
	long long int th = 0;
	int mode = 0;
	if(! *low)	
	{
		*low = calloc(1, 32);	
	}
	if(!*hi)
	{
		*hi  = calloc(1, 32);	
	}
	memset(*low, 0, 32);
	memset(*hi, 0, 32);
	lcs_str2_time(mile, &t);
	mode = t % lcs_info_cfg.step; /* 5 min */	
	tl = t - mode;
	th = t + lcs_info_cfg.step - mode;
	lcs_time2_str((time_t *)&tl, low);
	lcs_time2_str((time_t *)&th, hi);
	fprintf(stdout, "low: %s, hi: %s\n", *low, *hi);
}
/***********************************************************************************/
void lcs_init_circle(MYSQL *con, void *arg, int *err)
{
	lcs_gen_linkedid(con, arg, err);	
	lcs_create_subdir(arg, err);
}
/***********************************************************************************/
void lcs_end_circle(void *arg, int *err)
{
	LCS_GET_TABLE_INFO *p 	= 0;	
	p = arg;
	lcs_update_time(p, err);
}
/***********************************************************************************/
void lcs_gen_linkedid(MYSQL *con, void *arg, int *err)
{
	char *sql 				= 0;
	char *date				= 0;
	int error 				= 0;
	unsigned int sz 		= 0;
	LCS_GET_TABLE_INFO *p 	= 0;

	p = arg;
	date = lcs_info_cfg.date_01;
	sz = strlen(LCS_INSERT_LINKEDID_TMP);
	sql = calloc(1, sz + 128);
	sprintf(sql, LCS_INSERT_LINKEDID_TMP, 
		date, date, p->gbinfo->low, p->gbinfo->hi);
	do
	{
		if(!con)
		{
			error = __LINE__;
			break;
		}	
		lcs_clear_tmp_table(con, date, &error);
		if(error)
		{
			error = __LINE__;
			break;
		}

		if (mysql_query(con, sql))
		{  
			error = __LINE__;
			fprintf(stdout, "_----------------------erorrooro\n");
			fprintf(stdout, "_----------------------sql: %s\n", sql);
			break;
		}
	}
	while(0);

	*err = error;
}
/***********************************************************************************/
void lcs_update_time(void *arg, int *err)
{
	char tmp_low[64];
	char *tmp_hi			= 0;
	long long int t0		= 0;
	long long int t1 		= 0;
	LCS_GET_TABLE_INFO *p 	= 0;
	
	p = arg;
	memset(tmp_low, 0, 64);
	memcpy(tmp_low, p->gbinfo->hi, strlen(p->gbinfo->hi));

	lcs_str2_time(tmp_low, &t0);
	t1 = t0 + lcs_info_cfg.step;

	lcs_time2_str((time_t *)&t1, (char **)&tmp_hi);	
	fprintf(stdout, "%s, %s\n", tmp_low, tmp_hi);

	memcpy(p->gbinfo->low, tmp_low, strlen(tmp_low));
	memcpy(p->gbinfo->hi, tmp_hi, strlen(tmp_hi));

	free(tmp_hi);
}
/***********************************************************************************/
void lcs_clear_tmp_table(MYSQL *con, char *date, int *err)
{
	int  i						= 0;
	char query[1024];
	//struct timeval t0, t1;
	char *s[] =
			{ 
				"delete from cdr_%s where recid in (select recid from cdr_tmp_%s);",
				"delete from queue_log_%s where id in (select id from queue_tmp_%s);",
				"delete from monitor_recording_%s where id in (select id from monitor_tmp_%s);",
				"delete from linkedid_tmp_%s;",
				"delete from cdr_tmp_%s",
				"delete from queue_tmp_%s",
				"delete from monitor_tmp_%s",
				0
			};

	//gettimeofday(&t0, 0);

	while(s[i])
	{
		memset(query, 0, 1024);
		sprintf(query, s[i], date, date);
		//fprintf(stdout, "query: %s\n", query);
		*err = mysql_query(con, query);
		if (*err)
		{  
			fprintf(stdout, "error code: %d, sql: %s\n", *err, query);
			fprintf(stderr, "%s\n", mysql_error(con));
			*err = __LINE__;
			break;
		}	
		//gettimeofday(&t1, 0);
		//lcs_diff_time(&t1, &t0, 
		//	(char*)__FILE__,(char *) __FUNCTION__, __LINE__);
		++i;
	}
	//gettimeofday(&t1, 0);
	//lcs_diff_time(&t1, &t0, 
	//	(char*)__FILE__,(char *) __FUNCTION__, __LINE__);
}
/***********************************************************************************/
void lcs_create_subdir(void * arg, int *err)
{
	int error = 0;
	char tmp[64];
	LCS_DATE_DIR *d = 0;	
	LCS_GET_TABLE_INFO *p = 0;

	p = arg;
	memset(tmp, 0, 64);
	do 
	{
		memcpy(tmp, p->gbinfo->low, strlen(p->gbinfo->low));
		lcs_get_date(tmp, &d, &error);
		lcs_mk_dir(&lcs_info_cfg, p, d,  &error);
		if(d)
		{
			free(d);
		}
	}
	while(0);	
	*err = error;
}
/***********************************************************************************/
void lcs_init_param(char *table, void **out)
{
	int err					= 0;
	MYSQL *conn 			= 0;
	LCS_GET_TABLE_INFO *p 	= 0;
	p = malloc(sizeof(LCS_GET_TABLE_INFO));
	memset(p, 0, sizeof(LCS_GET_TABLE_INFO));

	sprintf(p->table, table);
	sprintf(p->out_file, "%s.data", p->table);

	//create connection
	lcs_try_connect(&lcs_info_cfg, &(conn), &err);
	p->con = &conn;

	p->gbinfo = &lcs_info_cfg;

	*out = (void*)p;
}
/***********************************************************************************/
void lcs_moving_param(char *path)
{
	int err = 0;
	char *output = 0;
	char cmd[1024];
	LCS_MYSQL_INFO *info = 0;
	
	info = &lcs_info_cfg;
	memset(cmd, 0, 1024);
	lcs_geninsert_cfg(path, &output, &err);
	sprintf(cmd, "%s %s", info->insertprocess, output);	
	//fprintf(stdout, "line: %d, cmd: %s\n", __LINE__, cmd);
	lcs_bash_cmd(cmd, &err);
	if(output)
	{
		free(output);
	}
/*
	FILE *fp = 0;
	char cmd[1024];
	char *output = 0;
	int err = 0;
	memset(cmd, 0, 1024);
	sprintf(cmd, "./run.sh \"%s\"", path);
	fprintf(stdout, "cmd: %s\n", path);
	fp = popen(cmd, "r");
	if(fp)
	{
		pclose(fp);
	}		
*/
}
/***********************************************************************************/
void lcs_get_outtime(void *arg, char *res)
{
	int cmp = 0;
	LCS_GET_TABLE_INFO *p = 0;
	p = arg;
	cmp = strcmp(p->gbinfo->low, p->gbinfo->endtime);
	//fprintf(stdout, "hi v send:cmd, %s vs %s:%d\n", p->gbinfo->hi, p->gbinfo->endtime, cmp);
	*res = ((cmp > 0)?1:0);
}
/***********************************************************************************/
void lcs_backup_hdial(void *arg)
{
	lcs_get_tab_pthread(arg);
}
/***********************************************************************************/
/*
void lcs_backup_ddial(void *arg)
{
	lcs_get_tab_pthread(arg);
}
*/
/***********************************************************************************/
void lcs_clear_history(void *arg, int *err)
{
	char sql[1024];
	MYSQL *con 				= 0;
	char *date				= 0;
	char *format_wrapup 	= 0; 
	char *format_detail 	= 0;
	LCS_GET_TABLE_INFO *p 	= 0;


	p = arg;
	date = lcs_info_cfg.date_01;
	con = *(p->con);
	format_wrapup = 
		"delete from %s_%s where DATE('%s')=DATE(start_time) AND start_time < '%s'";
	format_detail = 
		"delete from %s_%s where DATE('%s')=DATE(created_time) AND created_time < '%s'";
	memset(sql, 0, 1024);	

	if(strcmp(p->table, "auto_dial_wrapup_history") == 0)
	{
		sprintf(sql, format_wrapup, 
			p->table, date, p->gbinfo->low, p->gbinfo->low);
	}
	else if (strcmp(p->table, "auto_dial_detail") == 0)
	{
		sprintf(sql, format_detail,
			p->table, date, p->gbinfo->low, p->gbinfo->low);
	}
	do
	{
		if(!strlen(sql))
		{
			*err = 1;
			fprintf(stdout, "error, %d\n", __LINE__);
			break;
		}
		*err = mysql_query(con, sql);
		if(*err)
		{
			fprintf(stdout, "error, %d\n", __LINE__);
			break;
		}
	}
	while(0);
}
/***********************************************************************************/
void lcs_geninsert_cfg(char *path, char**output, int *err)
{
	FILE *fp = 0;
	LCS_MYSQL_INFO *info = 0;
	char data[2048];
	char targetdir[1024];
	char templatedir[1024];
	char listtable[] = "tables_0001:cdr,queue_log,monitor_recording,auto_dial_wrapup_history";
	info = &lcs_info_cfg;
	memset(targetdir, 0, 1024);
	memset(templatedir, 0, 1024);
	memset(data, 0, 2048);
	sprintf(targetdir, "targetdir_0002:%s", path);
	sprintf(templatedir, "templatedir_0003:%s/%s/%s/template", 
			info->path, info->server, info->db);
	*output = calloc(1, strlen(path) + 32);
	sprintf(*output, "%s/insert.cfg", path);	
	fp = fopen(*output, "w+");
	if(!fp)
	{
		*err = 1;
		free(*output);
		*output = 0;
		return;
	}
	strcat(data, listtable);
	strcat(data, "\n");
	strcat(data, targetdir);
	strcat(data, "\n");
	strcat(data, templatedir);
	strcat(data, "\n");
	fwrite(data, 1, strlen(data), fp);
	fclose(fp);
}
/***********************************************************************************/
void lcs_clear_tmp(char *date, int *err)
{
	char *sql[] = 
		{
			"drop table if exists cdr_tmp_%s;",
			"drop table if exists queue_tmp_%s;",
			"drop table if exists monitor_tmp_%s;",
			"drop table if exists linkedid_tmp_%s;",
			"drop table if exists cdr_%s;",
			"drop table if exists queue_log_%s;",
			"drop table if exists monitor_recording_%s;",
			"drop table if exists auto_dial_wrapup_history_%s;",
			0
		};

	int i 			= 0;
	MYSQL *conn 	= 0;
	struct timeval t0, t1;
	char query[521];
	gettimeofday(&t0, 0);
	do
	{
		lcs_try_connect(&lcs_info_cfg, &conn, err);
		if(*err)
		{
			fprintf(stdout, "[%s-->%d] *err: %d\n", __FILE__, __LINE__, *err);
			*err = __LINE__;	
			break;
		}
		while(sql[i])	
		{
			memset(query, 0, 512);
			sprintf(query, sql[i], date);
			fprintf(stdout, "query: %s\n", query);
			*err = mysql_query(conn, query);
			if(*err)
			{
				fprintf(stdout, "%s\n", mysql_error(conn));
				*err = __LINE__;
				break;
			}
			gettimeofday(&t1, 0);
			lcs_diff_time(&t1, &t0, 
				(char*)__FILE__, (char *)__FUNCTION__, __LINE__);
			i++;
		}
	}
	while(0);
	if(conn)
	{
		lcs_try_close(&conn, err);
	}
	gettimeofday(&t1, 0);
	lcs_diff_time(&t1, &t0, 
		(char*)__FILE__, (char *)__FUNCTION__, __LINE__);

}
/***********************************************************************************/
void lcs_init_tmp(char *date, int *err)
{
	char *format[] = 
		{
			"drop table if exists linkedid_tmp_%s;",
			"create table linkedid_tmp_%s(linkedid varchar(255), PRIMARY KEY(linkedid));",
			"drop table if exists cdr_tmp_%s;",
			"create table cdr_tmp_%s(recid bigint(20), PRIMARY KEY(recid));",
			"drop table if exists queue_tmp_%s;",
			"create table queue_tmp_%s(id bigint(20),PRIMARY KEY(id));",
			"drop table if exists monitor_tmp_%s;",
			" create table monitor_tmp_%s(id bigint(20), PRIMARY KEY(id));",
			0
		};

	int i 			= 0;
	MYSQL *conn 	= 0;
	char query[1024];
	struct timeval t0, t1;
	gettimeofday(&t0, 0);
	do
	{
		lcs_try_connect(&lcs_info_cfg, &conn, err);
		if(*err)
		{
			fprintf(stdout, "[%s-->%d] *err: %d\n", __FILE__, __LINE__, *err);
			*err = __LINE__;	
			break;
		}
		while(format[i])	
		{
			memset(query, 0, 1024);
			sprintf(query, format[i], date);
			fprintf(stdout, "query: %s\n", query);
			*err = mysql_query(conn, query);
			if(*err)
			{
				fprintf(stdout, "%s\n", mysql_error(conn));
				*err = __LINE__;
				break;
			}
			gettimeofday(&t1, 0);
			lcs_diff_time(&t1, &t0, 
				(char*)__FILE__, (char *)__FUNCTION__, __LINE__);
			i++;
		}
	}
	while(0);
	if(conn)
	{
		lcs_try_close(&conn, err);
	}
	gettimeofday(&t1, 0);
	lcs_diff_time(&t1, &t0, 
		(char*)__FILE__, (char *)__FUNCTION__, __LINE__);
}
/***********************************************************************************/
void lcs_gen_structure(int *err)
{
	char * sql[] = 
		{
			"CREATE TABLE if not exists cdr_structure LIKE cdr;", 
			"CREATE TABLE if not exists queue_log_structure LIKE queue_log;", 
			"CREATE TABLE if not exists monitor_recording_structure LIKE monitor_recording;", 
			0
		};

	int i 			= 0;
	MYSQL *conn 	= 0;
	struct timeval t0, t1;
	gettimeofday(&t0, 0);
	do
	{
		lcs_try_connect(&lcs_info_cfg, &conn, err);
		if(*err)
		{
			fprintf(stdout, "[%s-->%d] *err: %d\n", __FILE__, __LINE__, *err);
			*err = __LINE__;	
			break;
		}
		while(sql[i])	
		{
			*err = mysql_query(conn, sql[i]);
			if(*err)
			{
				fprintf(stdout, "%s\n", mysql_error(conn));
				*err = __LINE__;
				break;
			}
			gettimeofday(&t1, 0);
			lcs_diff_time(&t1, &t0, 
				(char*)__FILE__, (char *)__FUNCTION__, __LINE__);
			i++;
		}
	}
	while(0);
	if(conn)
	{
		lcs_try_close(&conn, err);
	}
	gettimeofday(&t1, 0);
	lcs_diff_time(&t1, &t0, 
		(char*)__FILE__, (char *)__FUNCTION__, __LINE__);

}
/***********************************************************************************/
void lcs_gen_data_tables(char *date, char *date01, int *err)
{

	char *format[] = 
		{

"drop table if exists cdr_%s;",
"CREATE TABLE cdr_%s LIKE cdr;",
"INSERT cdr_%s SELECT * FROM cdr where calldate >='%s 00:00:00' and calldate <= '%s 23:59:59';",
	
"drop table if exists queue_log_%s;", 
"CREATE TABLE queue_log_%s LIKE queue_log;",
"INSERT queue_log_%s SELECT * FROM queue_log where time >='%s 00:00:00' and time <= '%s 23:59:59';",

"drop table if exists monitor_recording_%s;", 
"CREATE TABLE monitor_recording_%s LIKE monitor_recording;",
"INSERT monitor_recording_%s SELECT * FROM monitor_recording where created_time >='%s 00:00:00' and created_time <= '%s 23:59:59';",

"drop table if exists auto_dial_wrapup_history_%s;",
"CREATE TABLE auto_dial_wrapup_history_%s LIKE auto_dial_wrapup_history;",
"INSERT auto_dial_wrapup_history_%s SELECT * FROM  auto_dial_wrapup_history where start_time >='%s 00:00:00' and start_time <= '%s 23:59:59'",

			0	
		};


	int i 			= 0;
	MYSQL *conn 	= 0;
	char query[1024];
	struct timeval t0, t1;
	gettimeofday(&t0, 0);
	do
	{
		lcs_try_connect(&lcs_info_cfg, &conn, err);
		if(*err)
		{
			fprintf(stdout, "[%s-->%d] *err: %d\n", __FILE__, __LINE__, *err);
			*err = __LINE__;	
			break;
		}
		while(format[i])	
		{
			memset(query, 0, 1024);
			if((i%3)== 2)
			{
				sprintf(query, format[i], date01, date, date);
			}
			else
			{
				sprintf(query, format[i], date01, date01);
			}
			fprintf(stdout, "query: %s\n", query);
			*err = mysql_query(conn, query);
			if(*err)
			{
				fprintf(stdout, "%s\n", mysql_error(conn));
				*err = __LINE__;
				break;
			}
			gettimeofday(&t1, 0);
			lcs_diff_time(&t1, &t0, 
				(char*)__FILE__, (char *)__FUNCTION__, __LINE__);
			i++;
		}
	}
	while(0);
	if(conn)
	{
		lcs_try_close(&conn, err);
	}
	gettimeofday(&t1, 0);
	lcs_diff_time(&t1, &t0, 
		(char*)__FILE__, (char *)__FUNCTION__, __LINE__);

}
/***********************************************************************************/
/*
void lcs_multi_query(MYSQL *conn, char ** sql, int *err)
{
	int i = 0;
	struct timeval t0, t1;
	while(sql[i])
	{
		*err = mysql_query(conn, sql[i]);
		if(*err)
		{
			fprintf(stdout, "%s\n", mysql_error(conn));
			*err = __LINE__;
			break;
		}
		gettimeofday(&t1, 0);
		lcs_diff_time(&t1, &t0, 
			(char*)__FILE__, (char *)__FUNCTION__, __LINE__);
		i++;
	}
}
*/
/***********************************************************************************/
