#include "simplelog.h"
#include <stdio.h>
#include <Windows.h>
#include <time.h>
//========================================================================================

#define spl_malloc(__nn__, __obj__) { (__obj__) = malloc(__nn__); if(__obj__) \
{spl_console_log("Malloc: 0x%p\n", (__obj__)); memset((__obj__), 0, (__nn__));} \
else {spl_console_log("Malloc: error.\n");}} 

#define spl_free(__obj__) { spl_console_log("Free: 0x:%p.\n", (__obj__)); free(__obj__); ; (__obj__) = 0;} 

#define FFCLOSE(fp, __n) { (__n) = fclose(fp); if(__n) {spl_console_log("Close FILE ERRR: %d.\n", (__n))};}

//========================================================================================

#define				SPLOG_PATHFOLDR					"pathfoder="
#define				SPLOG_LEVEL						"level="
#define				SPLOG_BUFF_SIZE					"buffsize="
#define				SPLOG_FILE_SIZE					"filesize="
#define				SPL_TEXT_UNKNOWN				"SPL_UNKNOWN"
#define				SPL_TEXT_DEBUG					"SPL_DEBUG"
#define				SPL_TEXT_INFO					"SPL_INFO"
#define				SPL_TEXT_WARN					"SPL_WARN"
#define				SPL_TEXT_ERROR					"SPL_ERROR"
#define				SPL_TEXT_FATAL					"SPL_FATAL"
//========================================================================================

typedef struct __GENERIC_DTA__ {
	int total;
	int pc; //Point to the current
	int pl; //Point to the last
	char data[0];
} generic_dta_st;

typedef struct __SIMPLE_LOG_ST__ {
		int llevel;
		int filesize;
		int index;
		char folder[1024];
		char off; //Must be sync

		void* mtx; //Need to close handle
		void* mtx_off; //Need to close handle
		void* sem_rwfile; //Need to close handle
		void* sem_off; //Need to close handle

		void* lc_time; //Need to sync, free
		void* fp; //Need to close

		generic_dta_st* buf; //Must be sync, free
	} SIMPLE_LOG_ST;


////========================================================================================
static const char*				__splog_pathfolder[]		= { SPLOG_PATHFOLDR, SPLOG_LEVEL, SPLOG_BUFF_SIZE, SPLOG_FILE_SIZE, 0 };
static	SIMPLE_LOG_ST			__simple_log_static__;;

static int				spl_init_log_parse(char* buff, char* key);
static void*			spl_mutex_create();
static void*			spl_sem_create(int ini);
static int				spl_verify_folder(char* folder);
static int				spl_simple_log_thread(SIMPLE_LOG_ST* t);
static int				spl_gen_file(SIMPLE_LOG_ST* t, int *n, int limit, int *);
static int				spl_get_fname_now(char* name);
static int				spl_get_fname_now(char* name);
static int				spl_standardize_path(char* fname);
static int				spl_folder_sup(char* folder,  SYSTEMTIME* lctime, char *year_month);
static DWORD WINAPI		spl_written_thread_routine(LPVOID lpParam);
//========================================================================================
int spl_set_log_levwel(int val) {
	//simple_log_levwel = val;
	__simple_log_static__.llevel = val;
	spl_console_log("log level: %d.\n", __simple_log_static__.llevel);
	return 0;
}
//========================================================================================
int spl_get_log_levwel() {
	int ret = 0;
	ret = __simple_log_static__.llevel;
	//spl_console_log("log level ret: %d.\n", ret);
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
		spl_console_log("------- errCode: %d\n", (int)errCode);
	}
	return ret;
}
//========================================================================================
int	spl_get_off() {
	int ret = 0;
	spl_mutex_lock(__simple_log_static__.mtx_off);
	do {
		ret = __simple_log_static__.off;
	} while (0);
	spl_mutex_unlock(__simple_log_static__.mtx_off);
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
			//__simple_log_static__.llevel = n;
			spl_set_log_levwel(n);
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
			spl_malloc(n + 2, p);
			//p = (char*)malloc(n + 2);
			if (!p) {
				ret = SPL_LOG_MEM_MALLOC_ERROR;
				break;
			}
			//memset(p, 0, n + 2);
			__simple_log_static__.buf = (generic_dta_st *) p;
			__simple_log_static__.buf->total = n;
			break;
		}
		if (strcmp(key, SPLOG_FILE_SIZE) == 0) {
			int n = 0;
			int sz = 0;
			sz = sscanf(buff, "%d", &n);
			if (n < 1) {
				ret = SPL_LOG_FILE_SIZE_ERROR;
				break;
			}
			__simple_log_static__.filesize = n;
		}
	} while (0);
	return ret;
}

int	spl_init_log( char *pathcfg) {
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
			spl_console_log("Cannot open file error.");
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
					char* pp = 0;
					node = (char *)__splog_pathfolder[j];
					if (!node) {
						break;
					}
					pp = strstr(buf, node);
					if (pp)
					{
						int k = strlen(node);
						char* p = (buf + k);
						spl_console_log("Find out the keyword: [%s] value [%s].", node, p);
						ret = spl_init_log_parse(p, node);
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

		obj = spl_mutex_create();
		if (!obj) {
			ret = SPL_ERROR_CREATE_MUTEX;
			break;
		}
		__simple_log_static__.mtx_off = obj;
		
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
		//ret = fclose(fp);
		FFCLOSE(fp,ret);
		spl_console_log("Close file result: %s.\n", ret ? "FAILED" : "DONE");
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
//int pthread_mutex_lock(pthread_mutex_t *mutex);
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
//int pthread_mutex_unlock(pthread_mutex_t *mutex);
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
////https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemtime
//========================================================================================
int spl_get_fname_now(char* name) {
	int ret = 0;
	SYSTEMTIME st, lt;
	GetSystemTime(&st);
	GetLocalTime(&lt);

	//spl_console_log("The system time is: %02d:%02d\n", st.wHour, st.wMinute);
	//spl_console_log(" The local time is: %02d:%02d\n", lt.wHour, lt.wMinute);
	if (name) {
		snprintf(name, 64, "%.4d-%.2d-%.2d-simplelog", lt.wYear, lt.wMonth, lt.wDay);
	}
	return ret;
}
#include <time.h>
//========================================================================================
DWORD WINAPI spl_written_thread_routine(LPVOID lpParam) {
	SIMPLE_LOG_ST* t = (SIMPLE_LOG_ST*)lpParam;
	int ret = 0;
	int off = 0;
	int ssfflush = 0;
	static time_t tttime;
	time_t tnnow;
	tnnow = time(0);
	int n = 0;
	int sz = 0;
	do {		
		if (!t) {
			exit(1);
		}
		if (!t->sem_rwfile) {
			exit(1);
		}
		spl_console_log("Semaphore: 0x%p.\n", t->sem_rwfile);
		if (!t->mtx) {
			exit(1);
		}
		spl_console_log("Mutex: 0x%p.\n", t->mtx);
		while (1) {
			WaitForSingleObject(t->sem_rwfile, INFINITE);
			off = spl_get_off();
			if (off) {
				break;
			}
			ret = spl_gen_file(t, &sz, t->filesize, &(t->index));
			if (ret) {
				//Log err
				continue;
			}
			spl_mutex_lock(t->mtx);
			do {
				
				if (t->buf->pl > t->buf->pc) {
					int k = 0;
					t->buf->data[t->buf->pl] = 0;
					k = fprintf(t->fp, "%s", t->buf->data);
					n += k;
					sz += k;
					t->buf->pl = t->buf->pc = 0;
					if (t->buf->total < (n + 1000)) {
						ssfflush = 1;
					}
				}
			} while (0);
			spl_mutex_unlock(t->mtx);
			if (!tttime) {
				tttime = tnnow;
			}
			if (tnnow > tttime + 1) {
				tttime = tnnow;
				ssfflush = 1;
			}
			if (ssfflush) {
				fflush(t->fp);
				ssfflush = 0;
			}
		}
		if (t->fp) {
			int werr = 0;
			FFCLOSE(t->fp, werr);
			if (werr) {
				//GetLastErr
				spl_console_log("close file err: %d,\n\n", werr);
			}
			else {
				t->fp = 0;
				spl_console_log("close file done,\n\n");
			}
			
			if (t->lc_time) {
				spl_free(t->lc_time);
			}
			spl_mutex_lock(t->mtx);
				if (t->buf) {
					spl_free(t->buf);
					//t->buf = 0;
				}
			spl_mutex_unlock(t->mtx);
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
		//fprintf(stdout, "n: %d.\n\n", n);
	} while (0);
	return ret;
}

//========================================================================================
int spl_fmmt_now(char* fmtt, int len) {
	int ret = 0;
	do {
		if (!fmtt) {
			ret = (int)SPL_LOG_FMT_NULL_ERROR;
			break;
		}
		int n;
		SYSTEMTIME st;
		char buff[20], buff1[20];
		memset(buff, 0, 20);
		memset(buff1, 0, 20);
		memset(&st, 0, sizeof(st));
		GetSystemTime(&st);
		n = GetDateFormatA(LOCALE_CUSTOM_DEFAULT, LOCALE_USE_CP_ACP, 0, "yyyy-MM-dd", buff, 20);
		n = GetTimeFormatA(LOCALE_CUSTOM_DEFAULT, TIME_FORCE24HOURFORMAT, 0, "HH:mm:ss", buff1, 20);
		n = snprintf(fmtt, len, "%s %s.%.3d", buff, buff1, (int)st.wMilliseconds);
	} while (0);
	return ret;
}

//========================================================================================
#define SPL_FILE_NAME_FMT			"%s\\%s\\%s_%0.8d.log"
int spl_gen_file(SIMPLE_LOG_ST* t, int *sz, int limit, int *index) {
	int ret = 0;
	SYSTEMTIME lt,* plt = 0;;
	GetLocalTime(&lt);
	int renew = 1;
	char path[1024];
	char fmt_file_name[64];
	int ferr = 0;
	char yearmonth[16];
	do {
		if (!(t->lc_time)) {
			spl_malloc(sizeof(SYSTEMTIME), t->lc_time);
			if (!t->lc_time) {
				ret = SPL_LOG_MEM_GEN_FILE_ERROR;
				break;
			}
			memcpy(t->lc_time, &lt, sizeof(SYSTEMTIME));
		}
		plt = (SYSTEMTIME*)t->lc_time;
		if (!t->fp) {
			memset(path, 0, sizeof(path));
			memset(fmt_file_name, 0, sizeof(fmt_file_name));
			spl_get_fname_now(fmt_file_name);
			ret = spl_folder_sup(t->folder, t->lc_time, yearmonth);
			do {
				int cszize = 0; //current size
				snprintf(path, 1024, SPL_FILE_NAME_FMT, t->folder, yearmonth, fmt_file_name, *index);
				spl_standardize_path(path);
				
				t->fp = fopen(path, "a+");
				if (!t->fp) {
					ret = SPL_LOG_OPEN_FILE_ERROR;
					break;
				}
				//fseek(t->tp, 0, SEEK_END);
				fseek(t->fp, 0, SEEK_END);
				cszize = ftell(t->fp);
				if (cszize > limit) {
					int err = 0;
					FFCLOSE(t->fp, err);
					t->fp = 0;
					(*index)++;
				}
				else {
					break;
				}
			} while (1);
			if (ret) {
				break;
			}
			break;
		}
		if (ret) {
			break;
		}
		do {
			if (*sz > limit) {
				(*index)++;
				break;
			}
			if (lt.wYear > plt->wYear) {
				break;
			}
			if (lt.wMonth > plt->wMonth) {
				break;
			}
			if (lt.wDay > plt->wDay) {
				break;
			}
			renew = 0; 
		} while (0);
		if (!renew) {
			break;
		}
		memcpy(t->lc_time, &lt, sizeof(SYSTEMTIME));
		spl_get_fname_now(fmt_file_name);
		//snprintf(path, 1024, SPL_FILE_NAME_FMT, t->folder, *index, fmt_file_name);
		ret = spl_folder_sup(t->folder, t->lc_time, yearmonth);
		snprintf(path, 1024, SPL_FILE_NAME_FMT, t->folder, yearmonth, fmt_file_name, *index);

		FFCLOSE(t->fp, ferr);
		if (ferr) {
			ret = SPL_LOG_CLOSE_FILE_ERROR;
			break;
		}
		spl_standardize_path(path);
		t->fp = fopen(path, "a+");
		if (sz) {
			*sz = 0;
		}
		if (!t->fp) {
			ret = SPL_LOG_OPEN1_FILE_ERROR;
			break;
		}
	} while (0);
	return ret;
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
LLU	spl_get_threadid() {
	return (LLU)GetCurrentThreadId();
}
//========================================================================================
int spl_rel_sem(void *sem) {
//int sem_post(sem_t *sem);
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

const char* spl_get_text(int lev) {
	const char* val = SPL_TEXT_UNKNOWN;
	do {
		if (lev == SPL_LOG_DEBUG) {
			val = SPL_TEXT_DEBUG;
			break;
		}
		if (lev == SPL_LOG_INFO) {
			val = SPL_TEXT_INFO;
			break;
		}
		if (lev == SPL_LOG_WARNING) {
			val = SPL_TEXT_WARN;
			break;
		}
		if (lev == SPL_LOG_ERROR) {
			val = SPL_TEXT_ERROR;
			break;
		}
		if (lev == SPL_LOG_FATAL) {
			val = SPL_TEXT_FATAL;
			break;
		}
	} while(0);
	return val;
}
//========================================================================================
int spl_finish_log() {
	int ret = 0;
	spl_set_off(1);
	CloseHandle(__simple_log_static__.mtx);
	CloseHandle(__simple_log_static__.mtx_off);
	CloseHandle(__simple_log_static__.sem_rwfile);
	CloseHandle(__simple_log_static__.sem_off);
	memset(&__simple_log_static__, 0, sizeof(__simple_log_static__));
	return ret;
}
//========================================================================================
char* spl_get_buf(int* n, int** ppl) {
	SIMPLE_LOG_ST* t = &__simple_log_static__;
	char* ret = 0;
	if (__simple_log_static__.buf);
	if (n && ppl) {
		if (t->buf) {
			*n = (t->buf->total > sizeof(generic_dta_st) + t->buf->pl) ? (t->buf->total - sizeof(generic_dta_st) - t->buf->pl) : 0;
			ret = t->buf->data;
			(*ppl) = &(t->buf->pl);
		}
	}
	return ret;
}
//========================================================================================
//https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createdirectorya
int spl_folder_sup(char* folder, SYSTEMTIME* lctime, char* year_month) {
	int ret = 0;
	int result = 0;
	//char tmp[1024];
	char path[1024];
	do {
		if (!folder) {
			ret = SPL_LOG_CHECK_FOLDER_NULL_ERROR;
			break;
		}
		if (!lctime) {
			ret = SPL_LOG_CHECK_FOLDER_NULL_ERROR;
			break;
		}
		if (!year_month) {
			ret = SPL_LOG_CHECK_FOLDER_NULL_ERROR;
			break;
		}
		snprintf(path, 1024, "%s", folder);
		result = CreateDirectoryA(path, 0);
		if (!result) {
			DWORD xerr = GetLastError();
			if (xerr != ERROR_ALREADY_EXISTS) {
				ret = SPL_LOG_CHECK_FOLDER_ERROR;
				break;
			}
		}
		snprintf(path, 1024, "%s/%0.4d", folder, lctime->wYear);
		result = CreateDirectoryA(path, 0);
		if (!result) {
			DWORD xerr = GetLastError();
			if (xerr != ERROR_ALREADY_EXISTS) {
				ret = SPL_LOG_CHECK_FOLDER_YEAR_ERROR;
				break;
			}
		}
		snprintf(path, 1024, "%s/%0.4d/%0.2d", folder, lctime->wYear, lctime->wMonth);
		result = CreateDirectoryA(path, 0);
		if (!result) {
			DWORD xerr = GetLastError();
			if (xerr != ERROR_ALREADY_EXISTS) {
				ret = SPL_LOG_CHECK_FILE_YEAR_ERROR;
				break;
			}
		}
		snprintf(year_month, 10, "%0.4d\\%0.2d", (int)lctime->wYear, (int)lctime->wMonth);
	} while (0);
	return ret;
}
//========================================================================================
int	spl_standardize_path(char* fname) {
	int ret = 0;
	int i = 0;
	while (fname[i]) {
		if (fname[i] == '\\') {
			fname[i] = '/';
		}
		++i;
	}
	return ret;
}
//========================================================================================