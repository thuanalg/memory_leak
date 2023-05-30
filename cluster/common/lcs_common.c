#include "lcs_common.h"
#include <fcntl.h> /* Definition of AT_* constants */
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
/***********************************************************************************/
/***********************************************************************************/
void lcs_create_dir(char *dirpath, int *err)
{
	FILE *fp = 0;
	char cmd[2048];

	memset(cmd, 0, 2048);
	sprintf(cmd, "mkdir -p %s", dirpath);
	fp = popen(cmd, "r");
	if(fp)
	{
		pclose(fp);
	}	
}
/***********************************************************************************/
void lcs_str2_time(char *str, long long int *time)
{
	struct tm tm_;
	time_t t = 0;
	memset(&tm_, 0, sizeof(struct tm));
	strptime(str, "%Y-%m-%d %H:%M:%S", &tm_);
	t = mktime(&tm_);
	*time = (long long int)t;
}
/***********************************************************************************/
void lcs_time2_str(time_t *t, char **str)
{
	*str = malloc(32);
	memset(*str, 0, 32);
	strftime(*str, 20, "%Y-%m-%d %H:%M:%S", localtime(t));
}
//***********************************************************************************/
/* Get data from file, and store in memory/RAM
 * path		:	file path
 * text		:	out text
 * err		:	error code
 */
/***********************************************************************************/
void lcs_file_text(char *path, char **text, int *err)
{
	FILE *fp = 0;
	unsigned int sz = 0;
	fp = fopen(path, "r");
	if(!fp)
	{
		*err = 1;
		return;
	}
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	*text = malloc(sz + 1);
	memset(*text, 0, sz + 1);
	rewind(fp);
	fread(*text, 1, sz, fp);
	fclose(fp);
}
/***********************************************************************************/
void lcs_bash_cmd(char *cmd, int *err)
{	
	FILE *fp = 0;
	fp = popen(cmd, "r");
	if(fp)
	{
		pclose(fp);
	}	
}
/***********************************************************************************/
void lcs_bash_cmdext(char *cmd, char **text,int *err)
{
	FILE *fp = 0;
	unsigned int sz = 0;
	char tmp[1024 + 1];
	fp = popen(cmd, "r");
	if(fp)
	{
		if(text)
		{
			while(1)
			{
				memset(tmp, 0, 1024 + 1);
				fgets(tmp, 1024, fp);
				sz = strlen(tmp);
				if(*text)
				{
					*text = realloc(*text, sz + 1 + strlen(*text));
					memset(&((*text)[strlen(*text)]), 0, sz + 1);
				}
				else
				{
					*text = realloc(*text, sz + 1);
					memset(*text, 0, sz + 1);
				}
				strcat(*text, tmp);
				if(sz < 1024)
				{
					break;
				}
			}
		}		
		pclose(fp);
	}	
}
/***********************************************************************************/
void lcs_cgen_list
(char *tables, LCS_COMM_TABLES **root, LCS_COMM_TABLES **last, int *err)
{
    char *pch = 0;
    LCS_COMM_TABLES *p = 0;
    pch = strtok(tables, ",\n");
    while(pch)
    {   
        if(strlen(pch))
        {   
            p = calloc( 1, sizeof(LCS_COMM_TABLES));
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
void lcs_cclear_list(LCS_COMM_TABLES **root, int *err)
{
    LCS_COMM_TABLES *p = 0;
    while( *root)
    {   
        p = (*root)->next;
        free(*root);
        (*root) = p;
    }   
}
/***********************************************************************************/
void lcs_commpre_env(char *templatedir, char *targetdir, int *err)
{
	char cmd[1024];
	memset(cmd, 0, 1024);
	sprintf(cmd, "cp -f \"%s\"/lcs.db \"%s\"", templatedir, targetdir);
	lcs_bash_cmd(cmd, err);
}
/***********************************************************************************/
void lcs_comm_moving
(char *templatedir, char *targetdir, 
char *tables, char *insertprocess, char **text,int *err)
{
	FILE *fp 			= 0;

	char cmd[1024];
	char listtab[1024];
	char templ[1024];
	char target[1024];
	char pathcfg[1024];
	char process[512];

	memset(process, 0, 512);
	memset(listtab, 0, 1024);
	memset(templ, 0, 1024);
	memset(target, 0, 1024);
	memset(pathcfg, 0, 1024);
	sprintf(listtab, "tables_0001:%s\n", tables);
	sprintf(target, "targetdir_0002:%s\n", targetdir);
	sprintf(templ, "templatedir_0003:%s\n", templatedir);
	sprintf(pathcfg, "%s/insert.cfg", targetdir);
	fp = fopen(pathcfg, "w+");
	if(!fp)
	{
		*err = 1;
		return;
	}
	fwrite(listtab, 1, strlen(listtab), fp);
	fwrite(target, 1, strlen(target), fp);
	fwrite(templ, 1, strlen(templ), fp);
	fclose(fp);
	memset(cmd, 0, 1024);
	sprintf(cmd, "%s %s", insertprocess, pathcfg);
	fprintf(stdout, "cmd: %s\n", cmd);
	lcs_bash_cmdext(cmd, text,err);
}
/***********************************************************************************/
void lcs_comm_replace(char *src, char nwe, char old, int *err)
{
	unsigned int i = 0;
	while(src[i])
	{
		if(src[i] == old)
		{
			src[i] = nwe;
		}
		++i;
	}
}
/***********************************************************************************/
void lcs_comm_getrange(LCS_LIST_DAY_PIECE *node, int *err)
{
//	int i = 0;
//	unsigned int mode = 0;
//	unsigned int div  = 0;
//	lcs_str2_time(node->from, (time_t *)&(node->ulfrom));
//	lcs_str2_time(node->to, (time_t*)&(node->ulto));
//	if(node->to < node->from)
//	{
//		*err = 1;
//		return;
//	}	
//	else if( node->from == node->to)
//	{
//				
//	}
}
/***********************************************************************************/
//https://linux.die.net/man/2/rmdir, remove empty directory
void lcs_comm_subdir_ext(char *path, char ***result, int *count, int* err)
{	
	int dir_count = 0;
    struct dirent* dent;
	char tmp[1024];
    DIR* srcdir = opendir(path);
	unsigned int sz = 0;
	
	if(!result)
	{
		*err = 1;
		return;
	}
    if (srcdir == NULL)
    {
		*err = 1;
        perror("opendir");
        return;
    }
	sz = sizeof(void **) * 62;
	*result = calloc(1, sz); 
    while((dent = readdir(srcdir)) != NULL)
    {
        struct stat st;
		memset(tmp, 0, 1024);

        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;
		
		sprintf(tmp, "%s/%s", path, dent->d_name);
        if (stat(tmp, &st) < 0)
        {
            perror(dent->d_name);
			*err = 1;
            continue;
        }
        if (S_ISDIR(st.st_mode)) 
		{
			//fprintf(stdout, "dent->d_name: %s\n", dent->d_name);
			(*result)[dir_count] = calloc(1, strlen(dent->d_name) + 1) ;
			strcat((*result)[dir_count], dent->d_name);	
			dir_count++;
		}
    }
    closedir(srcdir);
	*count = dir_count++;
	(*result) = realloc(*result, dir_count * sizeof(void **));

}
//https://linux.die.net/man/2/rmdir
//https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
/***********************************************************************************/
void lcs_comm_subdir(char *path, char ***result, int *err)
{
	int dir_count = 0;
    struct dirent* dent;
	char tmp[1024];
    DIR* srcdir = opendir(path);
	unsigned int sz = 0;
	
	if(!result)
	{
		*err = 1;
		return;
	}
    if (srcdir == NULL)
    {
		*err = 1;
        perror("opendir");
        return;
    }
	sz = sizeof(void **) * 62;
	*result = calloc(1, sz); 
    while((dent = readdir(srcdir)) != NULL)
    {
        struct stat st;
		memset(tmp, 0, 1024);

        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;
		
		sprintf(tmp, "%s/%s", path, dent->d_name);
        if (stat(tmp, &st) < 0)
        {
            perror(dent->d_name);
			*err = 1;
            continue;
        }
        if (S_ISDIR(st.st_mode)) 
		{
			//fprintf(stdout, "dent->d_name: %s\n", dent->d_name);
			(*result)[dir_count] = calloc(1, strlen(dent->d_name) + 1) ;
			strcat((*result)[dir_count], dent->d_name);	
			dir_count++;
		}
    }
    closedir(srcdir);
	dir_count++;
	(*result) = realloc(*result, dir_count * sizeof(void **));
	lcs_quick_sort( (void **)(*result), 0, dir_count - 2);
	//fprintf(stdout, "dir_count: %d\n", dir_count );
}
/***********************************************************************************/
void lcs_comm_filexist(char *path, int *isexist)
{
	FILE *fp = fopen(path, "r");
	*isexist = 0;
	if(fp)
	{
		*isexist = 1;
		fclose(fp);
	}
}
/***********************************************************************************/
int lcs_partition (char **arr, int low, int hi)
{
	int i, j;
	char *tmp = 0;
	char *pivot = 0;
	char **data = arr;

	pivot = data[low];
	i = low + 1;
	j = hi;
	while (i < j)
	{
		//while ( data[j] >= pivot) {
		while ( strcmp(pivot, data[j]) <= 0) {
			if ( i < j) {
				j--;
			}
			else {
				break;
			}
		}
		//while (data[i] <= pivot) {
		while (strcmp(pivot, data[i]) >= 0) {
			if (i < j) {
				i++;
			}else {
				break;
			}
		}
		if (i < j) {
			//Swap
			tmp = data[i];
			data[i] = data[j];
			data[j] = tmp;
			++i;
			--j;
		}
	}
	
	//if (data[j] < pivot) {
	if (strcmp(data[j],pivot) < 0) {
		//Swap
		tmp = data[low];
		data[low] = data[j];
		data[j] = tmp;
	}
	return j;

}
/***********************************************************************************/
void lcs_quick_sort (void ** arr, int low, int hi)
{	
	int j = 0;
	j = lcs_partition ((char **)arr, low, hi);
	if (low < j -1)
		lcs_quick_sort(arr, low, j -1 );
	if (j < hi)
		lcs_quick_sort(arr, j , hi);
}
/***********************************************************************************/
void lcs_comm_hash(char* p, int len, int *index, int sz)
{
	/*
	unsigned h = 0x811c9dc5;
    int i;

    for ( i = 0; i < len; i++ )
      h = ( h ^ p[i] ) * 0x01000193;	
	*index = (h%sz);
	*/
	sscanf(p, "%d", index);
	*index %= sz;
}
/***********************************************************************************/
void lcs_diff_time
(struct timeval *t1,struct timeval *t0,char *file, char *func, int line)
{
	unsigned long long diff = 0;
	diff = (t1->tv_sec - t0->tv_sec) * 1000000 + t1->tv_usec - t0->tv_usec;
	fprintf(stdout, "[%s->%s->%d], diff: %.3llu.%.6llu\n", 
		file, func, line, (diff/1000000),diff%1000000);
}
/***********************************************************************************/
int  lcs_partition_ext 
(void **data, int low, int hi, lcs_compare_cb f)
{
	int i, j;
	void *tmp = 0;
	void *pivot = 0;

	pivot = data[low];
	i = low + 1;
	j = hi;
	while (i < j)
	{
		//while ( data[j] >= pivot) {
		while ( f(pivot, data[j]) <= 0) {
			if ( i < j) {
				j--;
			}
			else {
				break;
			}
		}
		//while (data[i] <= pivot) {
		while (f(pivot, data[i]) >= 0) {
			if (i < j) {
				i++;
			}else {
				break;
			}
		}
		if (i < j) {
			//Swap
			tmp = data[i];
			data[i] = data[j];
			data[j] = tmp;
			++i;
			--j;
		}
	}
	
	//if (data[j] < pivot) {
	if (f(data[j],pivot) < 0) {
		//Swap
		tmp = data[low];
		data[low] = data[j];
		data[j] = tmp;
	}
	return j;
}
/***********************************************************************************/
void lcs_quick_sort_ext 
(void ** arr, int low, int hi, lcs_compare_cb f)
{
	int j = 0;
	j = lcs_partition_ext (arr, low, hi, f);
	if (low < j -1)
		lcs_quick_sort_ext(arr, low, j -1, f );
	if (j < hi)
		lcs_quick_sort_ext(arr, j , hi, f);
}
/***********************************************************************************/
int	 lcs_merge_counting_cb(void *a, void *b)
{
	int r = 0;

	r = strcmp( ((LCS_MERGE_COUNTING *)a)->date, ((LCS_MERGE_COUNTING *)b)->date );
	if(r)
	{
		return r;
	}

	return strcmp( ((LCS_MERGE_COUNTING *)a)->hour, ((LCS_MERGE_COUNTING *)b)->hour );
}
/***********************************************************************************/
void lcs_hcollect005_sum(char *path, char *text, int *err)
{
	char *p	  = 0;
	int  data = 0;
	int  count = 0;

	LCS_MERGE_COUNTING_LIST *root = 0;
	LCS_MERGE_COUNTING_LIST *last = 0;
	LCS_MERGE_COUNTING_LIST *node = 0;

	do
	{
		p = strtok(text, "\t\n");
		while(p)
		{
			fprintf(stdout, "%s\t", p);

			node = calloc(1, sizeof( LCS_MERGE_COUNTING_LIST ));	
			strcat(node->row.date, p);

			p = strtok(0, "\t\n");
			strcat(node->row.hour, p);
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.totalcall = data;
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.totalfullchannel = data;
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.viettelauto = data;
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.viettelfull = data;
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.mobileauto = data;
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.mobilefull = data;
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.vinaauto = data;
			fprintf(stdout, "%s\t", p);

			p = strtok(0, "\t\n");
			sscanf(p, "%d", &data);
			node->row.vinafull = data;
			fprintf(stdout, "%s\t", p);
			
			if(root)
			{
				last->next = node;
				last = node;
			}
			else
			{
				root = node;
				last = root;
			}
			/*****************************/
			fprintf(stdout, "\n");
			count++;
			p = strtok(0, "\t\n");
		}
		if(count)
		{
			lcs_merge005_list(root, count, path, err);
		}
	}
	while(0);	
	if(root)
	{
		node = root;
		while(node)
		{
			free(node);
			node = node->next;
		}
	}
}

/***********************************************************************************/
void lcs_merge005_list
(LCS_MERGE_COUNTING_LIST *root, int n, char *filepath, int *err)
{
	int i 							= 0;
	int l 							= 0;
	int m 							= 0;
	FILE *fp 						= 0;
	LCS_MERGE_COUNTING ** arr 		= 0;
	LCS_MERGE_COUNTING_LIST *p 		= 0;
	do 
	{
		arr = calloc(n + 1, sizeof(void *));
		if(!arr)
		{
			break;
		}
		p = root;
		while(p)
		{
			arr[i++] = &(p->row);
			p = p->next;
		}	
		lcs_quick_sort_ext((void **)arr, 0, i -1, lcs_merge_counting_cb);
	
		fp = fopen(filepath, "w+");
		if(!fp)
		{
			break;
		}

		for(i = 0; i <  n ; )
		{
			m = i;
			do
			{
				if(i == n - 1) 
				{
					break;
				}
				if(strcmp(arr[i]->date, arr[i+1]->date))
				{
					break;
				}

				if(strcmp(arr[i]->hour, arr[i+1]->hour))
				{
					break;
				}

				++i;

			}
			while(1);
			for(l = m+1; l <= i; ++l)
			{
				arr[m]->totalcall += arr[l]->totalcall;
				arr[m]->totalfullchannel += arr[l]->totalfullchannel;
				arr[m]->viettelauto += arr[l]->viettelauto;
				arr[m]->viettelfull += arr[l]->viettelfull;
				arr[m]->mobileauto += arr[l]->mobileauto;
				arr[m]->mobilefull += arr[l]->mobilefull;
				arr[m]->vinaauto += arr[l]->vinaauto;
				arr[m]->vinafull += arr[l]->vinafull;
			}
			fprintf(fp, "%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				arr[m]->date, arr[m]->hour, 
				arr[m]->totalcall, arr[m]->totalfullchannel,
				arr[m]->viettelauto, arr[m]->viettelfull, 
				arr[m]->mobileauto, arr[m]->mobilefull, 
				arr[m]->vinaauto, arr[m]->vinafull);
			++i;
		}
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
	if(arr)
	{
		free(arr);
	}
}
/***********************************************************************************/
void lcs_list005_rows
(void *fp, char **output,int *err)
{
	char *txt 			= 0;
	unsigned int sz 	= 0;

	sz = ftell((FILE*)fp);
	rewind((FILE*)fp);
	txt = calloc(1, sz + 1);
	fread(txt, 1, sz, (FILE*)fp);
	*output = txt;
}
/***********************************************************************************/
void lcs_comm_get_process
(char *path, char *result,int *err)
{
	char *p = 0;
	int having = 0;
	char tmp[1024];
	char tmppath[1024];
	
	memset(tmp, 0, 1024);
	strcat(tmp, path);
	
	p = strstr(path, "lcs_");
	if(p)
	{
		memset(tmppath, 0, 1024);
		sprintf(tmppath, "./%s", p);
		lcs_comm_filexist(tmppath, &having);
		if(having)
		{
			strcat(result, tmppath);
		}		
	}
	if(!having)
	{
		strcat(result, path);
	}
}
/***********************************************************************************/
void lcs_comm_list_files(char *path, char ***result, int *err)
{
	int dir_count 			= 0;
    struct dirent* dent		= 0;
	unsigned int sz 		= 0;

	char tmp[1024];
    DIR* srcdir = opendir(path);
	
	if(!result)
	{
		*err = 1;
		return;
	}
    if (srcdir == NULL)
    {
		*err = 1;
        perror("opendir");
        return;
    }
	sz = sizeof(void **) * 62;
	*result = calloc(1, sz); 
    while((dent = readdir(srcdir)) != NULL)
    {
        struct stat st;
		memset(tmp, 0, 1024);

        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;
		
		sprintf(tmp, "%s/%s", path, dent->d_name);
        if (stat(tmp, &st) < 0)
        {
            perror(dent->d_name);
			*err = 1;
            continue;
        }
        if (S_ISREG(st.st_mode)) 
		{
			//fprintf(stdout, "dent->d_name: %s\n", dent->d_name);
			(*result)[dir_count] = calloc(1, strlen(dent->d_name) + 1) ;
			strcat((*result)[dir_count], dent->d_name);	
			dir_count++;
		}
    }
    closedir(srcdir);
	dir_count++;
	(*result) = realloc(*result, dir_count * sizeof(void **));
}
/***********************************************************************************/
