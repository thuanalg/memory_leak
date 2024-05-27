#include "simplelog.h"
#include <stdio.h>
#include <Windows.h>
//========================================================================================
#define				SPLOG_PATHFOLDR					"pathfoder="
#define				SPLOG_LEVEL						"level="
#define				SPLOG_BUFF_SIZE					"buffsize="
static const char*				__splog_pathfolder[]		= { SPLOG_PATHFOLDR, SPLOG_LEVEL, SPLOG_BUFF_SIZE, 0 };
static	int						simple_log_levwel			=			0;
static	SIMPLE_LOG_ST			__simple_log_static__;;

static int		simple_init_log_parse(char* buff, char* key);
static void*	spl_mutex_create();
static void*	spl_sem_create(int ini);
static int		spl_mutex_lock(void* buff);
static int		spl_mutex_unlock(void* buff);
//========================================================================================
int simple_set_log_levwel(int val) {
	simple_log_levwel = val;
	__simple_log_static__.llevel = val;
	return 0;
}
//========================================================================================
int simple_get_log_levwel() {
	int ret = 0;
	ret = __simple_log_static__.llevel;
	return ret;
}
//========================================================================================
typedef enum __SPL_LOG_ERROR__{
	SPL_NO_ERROR = 0,
	SPL_INIT_PATH_FOLDER_EMPTY_ERROR,
	SPL_LOG_LEVEL_ERROR,
	SPL_ERROR_CREATE_MUTEX, 
	SPL_ERROR_CREATE_SEM, 
	SPL_LOG_BUFF_SIZE_ERROR,
} SPL_LOG_ERROR;
int	simple_init_log_parse(char* buff, char *key) {
	int ret = SPL_NO_ERROR;
	do {
		if (strcmp(key, SPLOG_PATHFOLDR) == 0) {
			if (!buff[0]) {
				ret = SPL_INIT_PATH_FOLDER_EMPTY_ERROR;
				break;
			}
			snprintf(__simple_log_static__.folder, 1024, "%s", buff);
			break;
		}
		if (strcmp(key, SPLOG_LEVEL) == 0) {
			int n = 0;
			int count = 0;
			count = sscanf(buff, "%d", &n);
			if (n < 0) {
				ret = SPL_LOG_LEVEL_ERROR;
				break;
			}
			__simple_log_static__.llevel = n;
			break;
		}
		if (strcmp(key, SPLOG_BUFF_SIZE) == 0) {
			int n = 0;
			int sz = 0;
			sz = sscanf(buff, "%d", &n);
			if (n < 0) {
				ret = SPL_LOG_BUFF_SIZE_ERROR;
				break;
			}
			__simple_log_static__.llevel = n;
			break;
		}
	} while (0);
	return ret;
}

int	simple_init_log( char *pathcfg) {
	int ret = 0;
	FILE* fp = 0;
	char c = 0;
	int count = 0;
	char buf[1024];
	void* obj = 0;
	do {
		memset(buf, 0, sizeof(buf));
		fp = fopen(pathcfg, "r");
		if (!fp) {
			ret = 1;
			consimplelog("Cannot open file error.");
			break;
		}
		while (c != EOF) {
			c = fgetc(fp);
			if (c == '\r' || c == '\n') {
				if (count > 0) {
					int  j = 0;
					char* node = 0;
					while (1) {
						node = __splog_pathfolder[j];
						if (!node) {
							break;
						}
						if (strstr(buf, node))
						{
							consimplelog("Find out the keyword: [%s] value [%s].", node, buf + strlen(node));
							ret = simple_init_log_parse(buf + strlen(node), node);
							break;
						}
						j++;
					}
					if (ret) {
						break;
					}
				}
				count = 0;
				memset(buf, 0, sizeof(buf));
				continue;
				
			}
			buf[count++] = c;
		}
		if (ret) {
			break;
		}
		obj = spl_mutex_create();
		if (!obj) {
			ret = SPL_ERROR_CREATE_MUTEX;
			break;
		}
		__simple_log_static__.mtx = obj;
		obj = spl_sem_create(1);
		if (!obj) {
			ret = SPL_ERROR_CREATE_SEM;
			break;
		}
		__simple_log_static__.sem_rwfile = obj;
	} while (0);
	if (fp) {
		ret = fclose(fp);
		consimplelog("Error, close file got trouble, error: ret: %d.\n", ret);
	}
	return ret;
}
//========================================================================================
void* spl_mutex_create() {
	void *ret = 0;
	ret = CreateMutexA(0, 1, 0);
	return ret;
}
//========================================================================================
void* spl_sem_create(int ini) {
	void* ret = 0;
	ret = CreateSemaphoreA(0, 0, ini, 0);
	return ret;
}
//========================================================================================
int spl_mutex_lock(void* obj) {
	int ret = 0;
	DWORD err = 0;
	do {
		if (!obj) {
			ret = 1;
			break;
		}
		err = WaitForSingleObject(obj, INFINITE);
		if (err != WAIT_OBJECT_0) {
			ret = 1;
			break;
		}
	} while (0);
	return ret;
}
//========================================================================================
int spl_mutex_unlock(void* obj) {
	int ret = 0;
	DWORD done = 0;
	do {
		if (!obj) {
			ret = 1;
			break;
		}
		done = ReleaseMutex(obj);
		if (!done) {
			ret = 1;
			break;
		}
	} while (0);
	return ret;
}
//========================================================================================