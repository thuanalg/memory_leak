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
	#define lcs_gen_templ		lcs_yyyyyy_000005
	#define lcs_info_cfg		lcs_yyyyyy_000006
	#define lcs_init_app		lcs_yyyyyy_000007
	#define lcs_start_gen		lcs_yyyyyy_000008
	#define lcs_try_close		lcs_yyyyyy_000009
	#define lcs_try_connect     lcs_yyyyyy_00000a              

#endif
/***********************************************************************************/

#define LCS_GEN_FORMAT 		"drop table if exists %s;\ncreate table %s(\n%s\n);\n"
#define LCS_INSERT_FORMAT 	"insert into  \"%s\" VALUES (%s);"

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
static void lcs_try_connect(LCS_TEMPLATE_INFO *, MYSQL **con, int *);
static void lcs_try_close(MYSQL **con, int *);
static void lcs_init_app();

static void lcs_start_gen(void *, int *);
static void lcs_gen_templ(char *, char *, char *, MYSQL *, int *err);
static void lcs_insert_format(char *, char *, char *, char *, int *err);
static void lcs_create_db(char *, char*, int *err);
/***********************************************************************************/
static  LCS_TEMPLATE_INFO			lcs_info_cfg;
static	char 						*lcs_cfg_file = 0;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	lcs_cfg_file = argv[1];
	lcs_init_app();
	//lcs_start_gen(0, &err);
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
void lcs_gen_templ(char *dir,char *path, char *table, MYSQL *con, int *err)
{
	int error = 0;
	char *text = 0;
	MYSQL_ROW row = 0;
	char tmp[64];
	MYSQL_RES *result = 0;
	char sql[1024];
	unsigned int current = 1;
	unsigned int sz = 0;
	char primary[128];
	char *sql_text;
	FILE *fp = 0;
	char text_insert_data[4096];
	char fullpath[1024];
	
	if(!con)
	{
		return;
	}	
	memset(fullpath, 0, 1024);
	sprintf(fullpath, "%s/%s", dir, path);
	memset(text_insert_data, 0, 4096);
	memset(primary, 0, 128);
	memset(sql, 0, 1024);
	sprintf(sql, "show columns from %s;", table);
	text = realloc(text, current);
	memset(&(text[current -1]), 0, current);
	error = mysql_query(con, sql);
	do
	{
		if(error)
		{
			
			fprintf(stdout, "error code: %d, sql: %s\n", error, sql);
			fprintf(stderr, "%s\n", mysql_error(con));
			break;
		}
		result = mysql_store_result(con);

		if (result == 0) 
		{
			break;
		}
		strcat(primary, "PRIMARY KEY(");
		while ((row = mysql_fetch_row(result)))
		{
			memset(tmp, 0, 64);
			lcs_comm_replace(row[0], '_', '-', &error);
			sprintf(tmp, "@%s, ", row[0]);
			strcat(text_insert_data, tmp);
			memset(tmp, 0, 64);
			if(strcmp(row[3], "PRI") == 0)
			{
				strcat(primary, row[0]);
				strcat(primary, ",");
			}
			if(strstr( row[1], "bit("))
			{
				sprintf(tmp, "\"%s\"\t\t\t\t%s,\n", row[0], "boolean");
			}
			else if(strstr( row[1], "enum("))
			{
				sprintf(tmp, "\"%s\"\t\t\t\t%s,\n", row[0], "varchar(50)");
			}
			else if(strstr( row[1], "unsigned"))
			{
				sprintf(tmp, "\"%s\"\t\t\t\t%s,\n", row[0], "bigint(20)");
			}
			else
			{
				sprintf(tmp, "\"%s\"\t\t\t\t%s,\n", row[0], row[1]);
			}
			sz = strlen(tmp);
			current = strlen(text);
			text = realloc(text, current + sz + 1);
			memset(&(text[current]), 0, sz + 1);
			memcpy(&(text[current]), tmp, sz);
		}
		mysql_free_result(result);

	}
	while(0);
	sz = strlen(primary);
	if(sz <= strlen("PRIMARY KEY("))
	{
		memset(primary, 0, 128);
		current = strlen(text);
		while(text[current] != ',')
		{
			--current;
		}
		text[current] = 0;
	}
	else
	{
		primary[sz - 1] = 0;
		strcat(primary, ")");
	}
	sz = strlen(primary);
	current = strlen(text);
	text = realloc(text, current + sz + 1);
	memset(&(text[current]), 0, sz + 1);
	memcpy(&(text[current]), primary, sz);
	
	if(text)
	{
		sz = strlen(text);
		sz += strlen(LCS_GEN_FORMAT) + 100;
		sql_text = malloc(sz);
		memset(sql_text, 0 , sz);
		sprintf(sql_text, LCS_GEN_FORMAT,table, table, text);
		fp = fopen(fullpath, "a+");
		sz = strlen(sql_text);
		if(fp)
		{
			sz = strlen(sql_text);
			fwrite(sql_text, 1, sz, fp);
			fclose(fp);
		}
		free(text);
		free(sql_text);
	}
	//fprintf(stdout, "insert: %s\n", text_insert_data);
	if(text_insert_data[0])
	{
		lcs_insert_format(dir, path, table, text_insert_data, err);
	}
	*err = error;
}
/***********************************************************************************/
void lcs_insert_format(char *dir, char *path, char *table, char *data, int *err)
{
	char *text_insert = 0;
	unsigned int sz = 0;
	FILE *fp = 0;
	char filepath[1024];
	
	sz = strlen(data);
	while(data[sz - 1] != ',')
	{
		sz--;
	};
	data[sz -1] = 0;
	sz += strlen(table);
	sz += strlen(LCS_INSERT_FORMAT);
	sz += 10;
	//fprintf(stdout, "table: %s, line: %d, length, sz: %u\n", table, __LINE__,sz);
	text_insert = malloc(sz);
	memset(text_insert, 0, sz);
	sprintf(text_insert, LCS_INSERT_FORMAT, table, data);

	do
	{
		if(!text_insert)
		{
			*err = 1;
			break;
		}
		memset(filepath, 0, 1024);
		sprintf(filepath, "%s/%s.format.insert.sql", dir, table);
		fprintf(stdout, "path: %s\n", filepath);
		//sprintf(filepath, "%s.format.insert.sql", table);
		fp = fopen(filepath, "w+");	
		if(!fp)
		{
			*err = 1;
			break;
		}
		fwrite(text_insert, 1, strlen(text_insert), fp);
	}while(0);

	if(text_insert)
	{
		free(text_insert);
	}
	if(fp)
	{
		fclose(fp);
	}
}
/***********************************************************************************/
void lcs_create_db(char *dir, char *name, int *err)
{
	char cmd[1024];
	memset(cmd, 0, 1024);
	sprintf(cmd, "sqlite3 \"%s\"/lcs.db < \"%s/%s\"", dir, dir, name);
	lcs_bash_cmd(cmd, err);
}
/***********************************************************************************/
