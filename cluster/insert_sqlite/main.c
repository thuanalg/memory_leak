#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <error.h>
#include <time.h>
#include <lcs_common.h>

/***********************************************************************************/

#define LCS_FORMAT_INSERT				"%s/%s.format.insert.sql"	
#define LCS_FORMAT_COPY					"cp -f \"%s\"/%s  \"%s\""	

/***********************************************************************************/
#ifdef LCS_RELEASE_MOD
#endif
/***********************************************************************************/
typedef void (*LCS_INSERT_CALLBACK)(void *, FILE *, sqlite3 *, sqlite3_stmt **, int*);

typedef struct LCS_INSERT_INFO
{
	char tables[1024];
	char targetdir[1024];
	char templatedir[1024];
}
LCS_INSERT_INFO;

typedef struct LCS_INSERT_TABLE_INFO
{
	char table[64];
	char db_path[1024];
	char data_path[1024];
	char *format;
	LCS_INSERT_CALLBACK fcb;

} LCS_INSERT_TABLE_INFO;

/***********************************************************************************/
static void lcs_fetch_file(void *, int *);
static void lcs_cfg_load(LCS_INSERT_INFO *, int *);
static void lcs_prepare_env(LCS_INSERT_INFO *, int *);
static void lcs_cfg_item(LCS_INSERT_INFO *, char *, int *);
static void lcs_insert_format(char *, char *, char **, int *);
static void lcs_fetch_clist(LCS_INSERT_INFO *, LCS_COMM_TABLES *, int *);
static void lcs_insert_general(void *, FILE *, sqlite3 *, sqlite3_stmt **, int *);
static void lcs_insert_context(void *, FILE *, sqlite3 *, sqlite3_stmt **, int *);
/***********************************************************************************/
LCS_INSERT_INFO			lcs_gb_info;
char					*lcs_cfg_path		= 0;
sqlite3 				*lcs_sqlite3_gp 	= 0;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err	= 0;
	LCS_COMM_TABLES *root = 0;
	LCS_COMM_TABLES *last = 0;
	char tables[1024];

	if(argc < 2)
	{
		fprintf(stdout, "%s %s\n", __DATE__, __TIME__);
		err = __LINE__;
		fprintf(stdout, LCS_RETURN_STATUS, "insert_sqlite3", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	memset(tables, 0, 1024);
	memset(&lcs_gb_info, 0, sizeof(LCS_INSERT_INFO));
	lcs_cfg_path = argv[1];
	do
	{
		lcs_cfg_load(&lcs_gb_info, &err);
		if(err)
		{
			err = __LINE__;
			break;
		}
		lcs_prepare_env(&lcs_gb_info, &err);
		if(err)
		{
			err = __LINE__;
			break;
		}
		strcat(tables, lcs_gb_info.tables);
		lcs_cgen_list(tables, &root, &last, &err);
		if(err)
		{
			err = __LINE__;
			break;
		}
		lcs_fetch_clist(&lcs_gb_info, root, &err);
		if(err)
		{
			err = __LINE__;
			break;
		}
		lcs_cclear_list(&root, &err);	
	
		if(err)
		{
			err = __LINE__;
			break;
		}

	} 
	while(0);

	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "insert_sqlite3", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, LCS_RETURN_STATUS, "insert_sqlite3", "EXIT_SUCCESS", err);
	return EXIT_SUCCESS;
}

/***********************************************************************************/
void lcs_fetch_file(void *arg, int *err)
{
	FILE *fp 					= 0;
	int error 					= 0;
	char *sErrMsg 				= 0;
	sqlite3_stmt * stmt 		= 0;	
	LCS_INSERT_TABLE_INFO *p 	= 0;

	p = arg;
	do
	{
		fp= fopen(p->data_path, "r");
		if(!fp)
		{
			break;
		}
		error = sqlite3_open(p->db_path, &lcs_sqlite3_gp);
		//error = sqlite3_open("lcs.db", &lcs_sqlite3_gp);
		if(error)
		{
			fprintf(stdout, "sqlite error at line: %d\n", __LINE__);
			break;
		}
		
		/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
		error = sqlite3_prepare_v2(lcs_sqlite3_gp,  
			(p->format), -1, &stmt, 0);
		if(error)
		{
			fprintf(stdout, "sqlite error at line: %d, err: %d\n", 
				__LINE__, error);
			//fprintf(stdout, "sql format: %s\n", (p->format));
			fprintf(stdout, "table name: %s\n", p->table);
			break;
		}
		sqlite3_exec(lcs_sqlite3_gp, "BEGIN TRANSACTION", 0, 0, &sErrMsg);
		if(p->fcb)
		{
			p->fcb(arg, fp, lcs_sqlite3_gp, &stmt, &error);
			if(error)
			{
				break;	
			}
		}
		else
		{
			error = __LINE__; break;
		}
		error = sqlite3_exec(lcs_sqlite3_gp, "END TRANSACTION", 0, 0, &sErrMsg);
		if(error)
		{
			fprintf(stdout, "error: %d, %d\n", error, __LINE__);
			error = __LINE__; break;
			break;
		}
		/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
			
		error = sqlite3_finalize(stmt);
		if(error)
		{
			fprintf(stdout, "error: %d, %d\n", error , __LINE__);
			error = __LINE__; break;
			break;
		}
	} 
	while(0);

	if(lcs_sqlite3_gp)
	{
		error = sqlite3_close(lcs_sqlite3_gp);
		if(error)
		{
			fprintf(stdout, "error: %d, %d\n", error , __LINE__);
		}
		lcs_sqlite3_gp = 0;
	}
	if(fp)
	{
		fclose(fp);
	}
	*err = error;
}
/***********************************************************************************/
void lcs_insert_general
(void *arg, FILE *fp, sqlite3 *db, sqlite3_stmt **stmt, int *err)
{
	int i 			= 0;
	int count 		= 0;
	char *pch 		= 0;
	char buffer[8192 + 1];

	while (!feof(fp))
	{
		memset(buffer, 0, 8192 + 1);
		fgets(buffer, 8192, fp);
		if(!buffer[0])
		{
			continue;
		}
		i = 1;
		pch = strtok(buffer, "\n\t");
		while(pch)
		{
			*err = sqlite3_bind_text( *stmt, i, pch, -1, 0);
			if(*err)
			{
				fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
				fprintf(stdout, "index: %d, %s\n", i, pch);
				*err = __LINE__;
				break;
			}
			pch = strtok(0, "\n\t");
			++i;
		}
		if(*err)
		{
			*err = __LINE__;
			break;
		}
		*err = sqlite3_step(*stmt);
		if(*err)
		{
			if(*err != SQLITE_DONE)
			{
				fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
				*err = __LINE__;
				break;
			}
		}
		*err = sqlite3_clear_bindings(*stmt);
		if(*err)
		{
			if(*err != SQLITE_DONE)
			{
				fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
				*err = __LINE__;
				break;
			}
		}
		*err = sqlite3_reset(*stmt);
		if(*err)
		{
			if(*err != SQLITE_DONE)
			{
				fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
				*err = __LINE__;
				break;
			}
		}
		count++;
	}	
	fprintf(stdout, "count: %d\n", count);
}
/***********************************************************************************/
void lcs_insert_format(char *root, char *table, char **fmat, int *err)
{
	char path[1024];
	memset(path, 0, 1024);	
	sprintf(path, LCS_FORMAT_INSERT, root, table);
	fprintf(stdout, "path: %s\n", path);
	lcs_file_text(path, fmat, err);
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load(LCS_INSERT_INFO *info, int *err)
{
	char *text 	= 0;
	char *pch 	= 0;
	lcs_file_text(lcs_cfg_path, &text, err);
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
void lcs_cfg_item(LCS_INSERT_INFO *info, char *item, int *err)
{
	char *base 			= 0;
	unsigned int sz 	= 0;
	do
	{
		base = "tables_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->tables, &item[sz], strlen(item)-sz);
			break;
		}
		base = "targetdir_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->targetdir, &item[sz], strlen(item)-sz);
			break;
		}
		base = "templatedir_0003:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->templatedir, &item[sz], strlen(item)-sz);
			break;
		}
	}
	while(0);	
}
/***********************************************************************************/
void lcs_prepare_env(LCS_INSERT_INFO *info, int *err)
{
	char cmd[2048];
	memset(cmd, 0, 2048);
	sprintf(cmd, LCS_FORMAT_COPY, 
		info->templatedir, "lcs.db", info->targetdir);
	lcs_bash_cmd(cmd, err);
}
/***********************************************************************************/
void lcs_fetch_clist(LCS_INSERT_INFO *info, LCS_COMM_TABLES *root, int *err)
{
	LCS_COMM_TABLES *p = 0;	
	LCS_INSERT_TABLE_INFO table;
	char *format = 0;
	p = root;
	while(p)
	{
		memset(&table, 0, sizeof(table));
		//fprintf(stdout, "name: %s\n", p->name);
		lcs_insert_format(info->templatedir, p->name, &format, err);
		if(format)
		{
			sprintf((table.table), "%s", p->name);
			sprintf((table.db_path), "%s/lcs.db", info->targetdir);
			sprintf((table.data_path), "%s/%s.data", info->targetdir, p->name);
			table.format = format;
			if(strcmp(p->name, "context") == 0)
			{
				table.fcb = lcs_insert_context;
			}
			else
			{
				table.fcb = lcs_insert_general;
			}
			lcs_fetch_file(&table, err);	

			free(format);
			format = 0;
		}
		else
		{
			*err = __LINE__;
			break;
		}
		if(*err)
		{
			break;
		}
		p = p->next;
	}
}
/***********************************************************************************/
void lcs_insert_context
(void *arg, FILE *fp, sqlite3 *db, sqlite3_stmt **stmt, int *err)
{
	int i 				= 0;
	char *pch 			= 0;
	int count 			= 0;
	char *buffer 		= 0;
	unsigned int sz 	= 0;
	do 
	{
		fseek(fp, 0, SEEK_END);
		sz = ftell(fp);
		rewind(fp);
		buffer = malloc(sz + 1);
		if(!buffer)
		{
			*err = __LINE__;
			fprintf(stdout, "file: %s:%d\n", __FILE__, __LINE__);
			break;
		}
		while (!feof(fp))
		{
			memset(buffer, 0, sz + 1);
			fgets(buffer, sz, fp);
			if(!buffer[0])
			{
				continue;
			}
			i = 1;
			pch = strtok(buffer, "\n\t");
			while(pch)
			{
				*err = sqlite3_bind_text( *stmt, i, pch, -1, 0);
				if(*err)
				{
					fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
					fprintf(stdout, "index: %d, %s\n", i, pch);
					*err = __LINE__;
					break;
				}
				pch = strtok(0, "\n\t");
				++i;
			}
			if(*err)
			{
				break;
			}
			*err = sqlite3_step(*stmt);
			if(*err)
			{
				if(*err != SQLITE_DONE)
				{
					fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
					*err = __LINE__;
					break;
				}
			}
			*err = sqlite3_clear_bindings(*stmt);
			if(*err)
			{
				if(*err != SQLITE_DONE)
				{
					fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
					*err = __LINE__;
					break;
				}
			}
			*err = sqlite3_reset(*stmt);
			if(*err)
			{
				if(*err != SQLITE_DONE)
				{
					fprintf(stdout, "error: %d, %d\n", *err, __LINE__);
					*err = __LINE__;
					break;
				}
			}
			count++;
		}	
	}
	while(0);
	fprintf(stdout, "count: %d\n", count);
	if(buffer)
	{
		free(buffer);
	}
}

/***********************************************************************************/
