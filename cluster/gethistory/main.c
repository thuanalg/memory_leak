#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>
#include <mysql.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <lcs_common.h>
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD
#endif
/***********************************************************************************/
typedef struct LCS_LIST_TABLE
{
	char name[64];
	struct LCS_LIST_TABLE *next;
} LCS_LIST_TABLE;
typedef struct LCS_MYSQL_INFO
{
	char server[128];
	char user[64];
	char password[64];
	char db[64];
	char date[32];
	char pathhistory[1024];
	char pathtemplate[1024];
	char listtable[1024];
	char insertprocess[1024];
	MYSQL *con;
}
LCS_MYSQL_INFO;
/***********************************************************************************/
static void lcs_cfg_load(LCS_MYSQL_INFO *, int *);
static void lcs_cfg_item(LCS_MYSQL_INFO *, char *, int *);
static void lcs_try_connect(LCS_MYSQL_INFO *, MYSQL **con, int *);
static void lcs_try_close(MYSQL **con, int *);
static void lcs_init_app();
static void lcs_start_bdata(void *, int *);
static void lcs_get_agents(LCS_MYSQL_INFO *,char *, int *err);
static void lcs_gen_llist(char *, LCS_LIST_TABLE **, LCS_LIST_TABLE **, int *err);
static void lcs_fetch_llist(LCS_MYSQL_INFO *, LCS_LIST_TABLE ** , int *err);
static void lcs_clear_llist(LCS_LIST_TABLE **, int *err);
static void lcs_get_field(char*, char*);
/***********************************************************************************/
static  LCS_MYSQL_INFO			lcs_info_cfg;
static  char*					LCS_MYSQL_CONFIG = 0;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	LCS_MYSQL_INFO *info = 0;
	char *text = 0;
	info = &lcs_info_cfg;

	do 
	{
		if(argc < 2)
		{
			err = __LINE__;
			break;
		}
		LCS_MYSQL_CONFIG = argv[1];
		lcs_init_app();
		lcs_start_bdata(0, &err);
		if(err)
		{
			break;
		}
		lcs_commpre_env(info->pathtemplate, info->pathhistory, &err);
		lcs_comm_moving(info->pathtemplate, 
			info->pathhistory, info->listtable, 
			info->insertprocess, &text, &err);
	}
	while(0);
	if(text)
	{
		free(text);
	}
	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "gentemplate", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, LCS_RETURN_STATUS, "gentemplate", "EXIT_SUCCESS", err);
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
	lcs_file_text(LCS_MYSQL_CONFIG, &text, err);
	//fprintf(stdout, "text: %s\n", text);
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
	char *base 			= 0;
	unsigned int sz 	= 0;
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
		base = "date_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->date, &item[sz], strlen(item)-sz);
			break;
		}

		base = "pathhistory_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->pathhistory, &item[sz], strlen(item)-sz);
			break;
		}

		base = "listtable_0003:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->listtable, &item[sz], strlen(item)-sz);
			break;
		}

		base = "pathtemplate_0004:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->pathtemplate, &item[sz], strlen(item)-sz);
			break;
		}

		base = "insertprocess_0005:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->insertprocess, &item[sz], strlen(item)-sz);
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
			*err = __LINE__;
			break;
		}
		if (mysql_real_connect(*con, info->server, info->user, info->password, 
				info->db, 0, 0, 0) == 0) 
		{
			fprintf(stderr, "%s\n", mysql_error(*con));
			lcs_try_close(con, err);
			*err = __LINE__;
			break;
		} 
		mysql_query(*con, "SET NAMES 'utf8';");
	}
	while(0);
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
/* Start/init backup data to warehouse
 * data		:	...
 * err		:	error code
 */
/***********************************************************************************/
void lcs_start_bdata(void *data, int *err)
{
	char tables[1024];
	MYSQL ** con 			= 0;
	LCS_MYSQL_INFO *info 	= 0;
	LCS_LIST_TABLE *root 	= 0;
	LCS_LIST_TABLE *last 	= 0;
	
	memset(tables, 0, 1024);
	con = &(lcs_info_cfg.con);
	info = &lcs_info_cfg;
	do
	{
		lcs_create_dir(info->pathhistory, err);
		if(*err)
		{
			break;
		}
		strcat(tables, info->listtable);
		
		lcs_gen_llist(tables, &root, &last, err);
		if(*err)
		{
			break;
		}

		lcs_try_connect(info, con, err);
		if(*err)
		{
			break;
		}
		
		lcs_fetch_llist( info, &root, err);
		if(*err)
		{
			break;
		}
		lcs_try_close(con, err);	
		if(*err)
		{
			break;
		}
		lcs_clear_llist(&root, err);
		if(*err)
		{
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
	memset(&lcs_info_cfg, 0 , sizeof(LCS_MYSQL_INFO));
	lcs_cfg_load(&lcs_info_cfg, &err);
}
/***********************************************************************************/
void lcs_get_agents(LCS_MYSQL_INFO *info, char *table, int *err)
{
	int i				= 0;
	int n				= 0;
	int error			= 0;
	int count			= 0;
	char *pch 			= 0;
	char *buf 			= 0;
	FILE *fp 			= 0;
	MYSQL *con 			= 0;
	MYSQL_ROW row		= 0;
	MYSQL_RES *result 	= 0;

	char field[32];
	char sql[1024];
	char fullpath[1024];

	con = info->con;

	if(!con)
	{
		*err = __LINE__;
		return;
	}	
	
	memset(sql, 0, 1024);
	memset(fullpath, 0, 1024);
	lcs_get_field(table, field);
	sprintf(sql, "select * from %s where %s >= '%s 00:00:00' and %s <= '%s 23:59:59';", 
		table, field, info->date, field, info->date);
	sprintf(fullpath, "%s/%s.data", info->pathhistory, table);
	error = mysql_query(con, sql);
	do
	{
		if(error)
		{
			*err = __LINE__;	
			fprintf(stdout, "error code: %d, sql: %s\n", error, sql);
			fprintf(stderr, "%s\n", mysql_error(con));
			break;
		}
		result = mysql_store_result(con);

		if (result == 0) 
		{
			break;
		}
		n = mysql_num_fields(result);
		fp = fopen(fullpath, "w+");
		if(!fp)
		{
			*err = __LINE__;
			break;
		}
		while ((row = mysql_fetch_row(result)))
		{
			for(i = 0; i < n; ++i)
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
					buf = calloc(1, strlen(row[i]) + 50);
					
					if(!buf)
					{
						*err = 1; break;
					}
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
					free(buf);
					buf = 0;
				}    
				else
				{
					fprintf(fp, "NULL\t"); 
				}
			}
			fprintf(fp, "\n");
			count++; 
		}
		mysql_free_result(result);

	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
	fprintf(stdout, "count: %d\n", count);
	//*err = error;
}
/***********************************************************************************/
void lcs_gen_llist
(char *tables, LCS_LIST_TABLE **root, LCS_LIST_TABLE **last, int *err)
{
	char *pch 			= 0;
	LCS_LIST_TABLE *p 	= 0;
	pch = strtok(tables, ",\n");

	while(pch)
	{
		if(strlen(pch))
		{
			p = calloc(	1, sizeof(LCS_LIST_TABLE));
			strcat(p->name, pch);
			if(*root)
			{
				(*last)->next = p;
				(*last) = p;
			}
			else
			{
				(*root) = p;
				(*last) = p;
			}
		}
		pch = strtok(0, ",\n");
	}	
}
/***********************************************************************************/
void lcs_clear_llist(LCS_LIST_TABLE **root, int *err)
{
	LCS_LIST_TABLE *p = 0;
	while( *root)
	{
		p = (*root)->next;
		free(*root);
		(*root) = p;
	}
}
/***********************************************************************************/
void lcs_fetch_llist(LCS_MYSQL_INFO *info, LCS_LIST_TABLE **root, int *err)
{
	LCS_LIST_TABLE *p = 0;
	p = (*root);
	while(p)
	{
		fprintf(stdout, "p->name: %s\n", p->name);
		lcs_get_agents(info, p->name, err);
		p = p->next;
	}
}
/***********************************************************************************/
void lcs_get_field(char *tab, char *field)
{
	memset(field, 0, 32);
	if(strcmp(tab, "DND_History") == 0)	
	{
		strcat(field, "start_time");
		return;
	}

	if(strcmp(tab, "auto_dial_wrapup_history") == 0)	
	{
		strcat(field, "start_time");
		return;
	}

	if(strcmp(tab, "agent_history") == 0)	
	{
		strcat(field, "login_at");
		return;
	}

	if(strcmp(tab, "auto_dial_detail") == 0)	
	{
		strcat(field, "created_time");
		return;
	}
}
/***********************************************************************************/
