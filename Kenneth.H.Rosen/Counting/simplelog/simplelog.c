#include "simplelog.h"
#include <stdio.h>
#include <Windows.h>
#include <time.h>
//========================================================================================
#define				SPLOG_PATHFOLDR					"pathfoder="
#define				SPLOG_LEVEL						"level="
#define				SPLOG_BUFF_SIZE					"buffsize="
static const char*				__splog_pathfolder[]		= { SPLOG_PATHFOLDR, SPLOG_LEVEL, SPLOG_BUFF_SIZE, 0 };
static	int						simple_log_levwel			=			0;
static	SIMPLE_LOG_ST			__simple_log_static__;;

static int		spl_init_log_parse(char* buff, char* key);
static void*	spl_mutex_create();
static void*	spl_sem_create(int ini);
static int		spl_verify_folder(char* folder);
static int		spl_simple_log_thread(SIMPLE_LOG_ST* t);
static int		spl_gen_file(SIMPLE_LOG_ST* t);
static DWORD WINAPI spl_written_thread_routine(LPVOID lpParam);
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
int	spl_set_off(int isoff) {
	int ret = 0;
	spl_mutex_lock(__simple_log_static__.mtx);
	do {
		__simple_log_static__.off = isoff;
	} while (0);
	spl_mutex_unlock(__simple_log_static__.mtx);
	
	if (isoff) {
		spl_rel_sem(__simple_log_static__.sem_rwfile);
		DWORD errCode = WaitForSingleObject(__simple_log_static__.sem_off, 3 * 1000);
		consimplelog("-------WaitForSingleObject, errCode: %d\n", (int)errCode);
	}
	return ret;
}
//========================================================================================
int	spl_get_off() {
	int ret = 0;
	spl_mutex_lock(__simple_log_static__.mtx);
	do {
		ret = __simple_log_static__.off;
	} while (0);
	spl_mutex_unlock(__simple_log_static__.mtx);
	return ret;
}
//========================================================================================

int	spl_init_log_parse(char* buff, char *key) {
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
			char* p = 0;
			sz = sscanf(buff, "%d", &n);
			if (n < 1) {
				ret = SPL_LOG_BUFF_SIZE_ERROR;
				break;
			}
			//__simple_log_static__.szbuf = n;
			p = (char*)malloc(n);
			if (!p) {
				ret = SPL_LOG_MEM_MALLOC_ERROR;
				break;
			}
			__simple_log_static__.buf = malloc(n + 2);
			memset(p, 0, n + 2);
			__simple_log_static__.buf = (generic_dta_st *) p;
			__simple_log_static__.buf->total = n;
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
		//while (c != EOF) {
		while (1) {
			c = fgetc(fp);
			if (c == '\r' || c == '\n' || c == EOF) {
				int  j = 0;
				char* node = 0;
				if (count < 1) {
					continue;
				}
				while (1) {
					node = __splog_pathfolder[j];
					if (!node) {
						break;
					}
					if (strstr(buf, node))
					{
						consimplelog("Find out the keyword: [%s] value [%s].", node, buf + strlen(node));
						ret = spl_init_log_parse(buf + strlen(node), node);
						break;
					}
					j++;
				}

				if (ret) {
					break;
				}			
				count = 0;
				memset(buf, 0, sizeof(buf));
				if (c == EOF) {
					break;
				}
				continue;
				
			}
			if (c == EOF) {
				break;
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

		obj = spl_sem_create(1);
		if (!obj) {
			ret = SPL_ERROR_CREATE_SEM;
			break;
		}
		__simple_log_static__.sem_off = obj;

		char* folder = __simple_log_static__.folder;
		ret = spl_verify_folder(folder);
		if (ret) {
			break;
		}
		// ret = spl_simple_log_thread(&__simple_log_static__);
	} while (0);
	if (fp) {
		ret = fclose(fp);
		consimplelog("Close file result: %s.\n", ret ? "FAILED" : "DONE");
	}
	if (ret == 0) {
		ret = spl_simple_log_thread(&__simple_log_static__);
	}
	return ret;
}
//========================================================================================
void* spl_mutex_create() {
	void *ret = 0;
	ret = CreateMutexA(0, 0, 0);
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
int spl_verify_folder(char* folder) {
	int ret = 0;
	do {
#ifdef WIN32
		//https://learn.microsoft.com/en-us/windows/win32/fileio/retrieving-and-changing-file-attributes
		// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createdirectorya
		// ERROR_ALREADY_EXISTS, ERROR_PATH_NOT_FOUND
		BOOL result = CreateDirectoryA(folder, NULL);
		if (!result) {
			DWORD werr = GetLastError();
			if (werr == ERROR_ALREADY_EXISTS) {
				//ret = 1;
				break;
			}
			ret = SPL_LOG_FOLDER_ERROR;
		}
#endif
	} while (0);
	return ret;
}
//========================================================================================
//Millisecond
LLU	simple_log_time_now(int *delta) {
//https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemtime
	static LLU ret_mile = 0;
	LLU retnow = 0;
	int elapsed = 0;
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
	time_t t1 = time(0);
	retnow = t1 * 1000 + systemTime.wMilliseconds;
	if (delta) {
		spl_mutex_lock(__simple_log_static__.mtx);
		do {
			
			if (!ret_mile) {
				*delta = 0;
			}
			else {
				*delta = (int) (retnow - ret_mile);
			}
			ret_mile = retnow;
		} while (0);
		spl_mutex_unlock(__simple_log_static__.mtx);
	}
	//if(delta)
	return retnow;
}
//========================================================================================
int simple_log_name_now(char* name) {
	int ret = 0;
	SYSTEMTIME st, lt;
	GetSystemTime(&st);
	GetLocalTime(&lt);

	printf("The system time is: %02d:%02d\n", st.wHour, st.wMinute);
	printf(" The local time is: %02d:%02d\n", lt.wHour, lt.wMinute);
	if (name) {
		snprintf(name, 64, "%.4d-%.2d-%.2d-simplelog.log", lt.wYear, lt.wMonth, lt.wDay);
	}
	return ret;
}
//========================================================================================
DWORD WINAPI spl_written_thread_routine(LPVOID lpParam) {
	SIMPLE_LOG_ST* t = (SIMPLE_LOG_ST*)lpParam;
	int ret = 0;
	int off = 0;
	do {
		if (!t) {
			exit(1);
		}
		if (!t->sem_rwfile) {
			exit(1);
		}
		consimplelog("Semaphore: 0x%p.\n", t->sem_rwfile);
		if (!t->mtx) {
			exit(1);
		}
		consimplelog("Mutex: 0x%p.\n", t->mtx);
		while (1) {
			off = spl_get_off();
			if (off) {
				break;
			}
			WaitForSingleObject(t->sem_rwfile, INFINITE);
			ret = spl_gen_file(t);
			if (ret) {
				//Log err
				continue;
			}
			spl_mutex_lock(t->mtx);
			do {
				int n = 0;
				if (t->buf->pl > t->buf->pc) {
					t->buf->data[t->buf->pl] = 0;
					n = fprintf(t->fp, "%s", t->buf->data);
					t->buf->pl = t->buf->pc = 0;
				}
			} while (0);
			spl_mutex_unlock(t->mtx);
			fflush(t->fp);
		}
		if (t->fp) {
			int werr = fclose(t->fp);
			if (werr) {
				//GetLastErr
				consimplelog("close file err: %d,\n\n", werr);
			}
			else {
				t->fp = 0;
				consimplelog("close file done,\n\n");
			}
		}
	} while (0);
	spl_rel_sem(__simple_log_static__.sem_off);
	return 0;
}
//========================================================================================
int spl_simple_log_thread(SIMPLE_LOG_ST* t) {
	int ret = 0;
	HANDLE hd = 0;
	DWORD thread_id = 0;
	hd = CreateThread( NULL, 0, spl_written_thread_routine, t, 0, &thread_id);
	do {
		if (!hd) {
			ret = SPL_LOG_CREATE_THREAD_ERROR;
			break;
		}
	} while (0);
	return ret;
}
//========================================================================================
int spl_fmt_now(char* fmtt, int len) {
	int ret = 0;
	static LLU pre_tnow = 0;
	LLU _tnow = 0;
	LLU _delta = 0;
	time_t t = time(0);
	do {
		if (!fmtt) {
			ret = (int) SPL_LOG_FMT_NULL_ERROR;
			break;
		}
		int n;
		SYSTEMTIME st;
		char buff[20], buff1[20];
		memset(buff, 0, 20);
		memset(buff1, 0, 20);
		memset(&st, 0, sizeof(st));
		GetSystemTime(&st);
		_tnow = t;
		_tnow *= 1000;
		_tnow += st.wMilliseconds;
		do {
			spl_mutex_lock(__simple_log_static__.mtx);
			do {

				if (!pre_tnow) {
					_delta = 0;
					pre_tnow = _tnow;
				}
				else {
					_delta = _tnow - pre_tnow;
				}
				pre_tnow = _tnow;
			} while (0);
			spl_mutex_unlock(__simple_log_static__.mtx);
		} while (0);
		n = GetDateFormatA(LOCALE_CUSTOM_DEFAULT, LOCALE_USE_CP_ACP, 0, "yyyy-MM-dd", buff, 20);
		n = GetTimeFormatA(LOCALE_CUSTOM_DEFAULT, TIME_FORCE24HOURFORMAT, 0, "HH:mm:ss", buff1, 20);
		n = snprintf(fmtt, len, "%s %s.%.3d (+%0.7llu)", buff, buff1, (int)st.wMilliseconds, _delta);
	} while (0);
	return ret;
}
//========================================================================================
int spl_gen_file(SIMPLE_LOG_ST* t) {
	int ret = 0;
	SYSTEMTIME st, lt,* plt = 0;;
	GetSystemTime(&st);
	GetLocalTime(&lt);
	int renew = 0;
	do {
		char path[1024];
		char fmt_file_name[64];
		memset(path, 0, sizeof(path));
		memset(fmt_file_name, 0, sizeof(fmt_file_name));
		if (!(t->lc_time)) {
			t->lc_time = (SYSTEMTIME*)malloc(sizeof(SYSTEMTIME));
			if (!t->lc_time) {
				ret = SPL_LOG_MEM_GEN_FILE_ERROR;
				break;
			}
			memset(t->lc_time, 0, sizeof(SYSTEMTIME));
			memcpy(t->lc_time, &lt, sizeof(SYSTEMTIME));
		}
		plt = (SYSTEMTIME*)t->lc_time;
		if (!t->fp) {
			simple_log_name_now(fmt_file_name);
			snprintf(path, 1024, "%s/%s", t->folder, fmt_file_name);
			t->fp = fopen(path, "a+");
			if (!t->fp) {
				ret = SPL_LOG_OPEN_FILE_ERROR;
				break;
			}
			break;
		}
		do {
			if (lt.wYear > plt->wMonth) {
				renew = 1;
				break;
			}
			if (lt.wYear > plt->wMonth) {
				renew = 1;
				break;
			}
			if (lt.wYear > plt->wMonth) {
				renew = 1;
				break;
			}
		} while (0);
		if (!renew) {
			break;
		}
		simple_log_name_now(fmt_file_name);
		snprintf(path, 1024, "%s/%s", t->folder, fmt_file_name);
		if (fclose(t->fp)) {
			ret = SPL_LOG_CLOSE_FILE_ERROR;
			break;
		}
		t->fp = fopen(path, "a+");
		if (!t->fp) {
			ret = SPL_LOG_OPEN1_FILE_ERROR;
			break;
		}
	} while (0);
	return ret;
}
//========================================================================================
char* spl_get_buf() {
	if (__simple_log_static__.buf) {
		return __simple_log_static__.buf->data;
	}
	return 0;
}
//========================================================================================
void* spl_get_mtx() {
	if (__simple_log_static__.mtx) {
		return __simple_log_static__.mtx;
	}
	return 0;
}
//========================================================================================
void* spl_get_sem() {
	if (__simple_log_static__.sem_rwfile) {
		return __simple_log_static__.sem_rwfile;
	}
	return 0;
}
//========================================================================================
SIMPLE_LOG_ST* spl_get_main_obj() {
	return &__simple_log_static__;
}
//========================================================================================
LLU	spl_get_threadid() {
	return (LLU)GetCurrentThreadId();
}
//========================================================================================
int spl_rel_sem(void *sem) {
	int ret = 0;
	do {
		if (!sem) {
			ret = SPL_LOG_SEM_NULL_ERROR;
			break;
		}
		ReleaseSemaphore(sem, 1, 0);
	} while (0);
	return ret;
}
//========================================================================================