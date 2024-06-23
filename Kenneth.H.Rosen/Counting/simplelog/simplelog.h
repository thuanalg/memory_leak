#ifndef ___MYSIMPLE_LOG__
#define ___MYSIMPLE_LOG__
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

#define LLU				unsigned long long

#ifndef ______MYSIMPLE_STATIC_LOG__
	#ifdef EXPORT_DLL_API_SIMPLE_LOG
		#define DLL_API_SIMPLE_LOG		__declspec(dllexport)
	#else
		#define DLL_API_SIMPLE_LOG		__declspec(dllimport)
	#endif
#else
	#define DLL_API_SIMPLE_LOG
#endif

#ifndef __SIMPLE_LOG_PLATFORM__
	#define				__SIMPLE_LOG_PLATFORM__							"[WIN32_MSVC]"
#endif // !__PLAT

#ifndef __FILE_LINE_SIMPLELOG__
	#define				__FILE_LINE_SIMPLELOG__								"[%s:%d] [threadid: %llu]"
#endif // !__FILE_LINE_SIMPLELOG__

	typedef enum __SPL_LOG_ERROR__ {
		SPL_NO_ERROR = 0,
		SPL_INIT_PATH_FOLDER_EMPTY_ERROR,
		SPL_LOG_LEVEL_ERROR,
		SPL_ERROR_CREATE_MUTEX,
		SPL_ERROR_CREATE_SEM,
		SPL_LOG_BUFF_SIZE_ERROR,
		SPL_LOG_FOLDER_ERROR,
		SPL_LOG_CREATE_THREAD_ERROR,
		SPL_LOG_FMT_NULL_ERROR,
		SPL_LOG_MEM_GEN_FILE_ERROR,
		SPL_LOG_MEM_MALLOC_ERROR,
		SPL_LOG_OPEN_FILE_ERROR,
		SPL_LOG_OPEN1_FILE_ERROR,
		SPL_LOG_CLOSE_FILE_ERROR,
		SPL_LOG_SEM_NULL_ERROR,
		SPL_LOG_FILE_SIZE_ERROR,
		SPL_LOG_MEM_FILE_MALLOC_ERROR,
		SPL_LOG_CHECK_FOLDER_ERROR,
		SPL_LOG_CHECK_FOLDER_YEAR_ERROR,
		SPL_LOG_CHECK_FILE_YEAR_ERROR,
		SPL_LOG_CHECK_FOLDER_NULL_ERROR,

		SPL_END_ERROR,
	} SPL_LOG_ERROR;


#define spl_console_log(___fmttt___, ...)		{char buf[32];spl_fmmt_now(buf, 32);\
fprintf(stdout, "[%s] "__FILE_LINE_SIMPLELOG__" "___fmttt___"\n" , buf, __FUNCTION__, __LINE__, spl_get_threadid(), ##__VA_ARGS__);}





#define __spl_log_buf__(___fmttt___, ...)	{int *__ppl = 0; char tnow[40]; int range=0; char* __p = 0; void *__mtx__ =  spl_get_mtx(); \
int len = 0; spl_fmt_now(tnow, 40);\
spl_mutex_lock(__mtx__);\
__p = spl_get_buf(&range, &__ppl); if (__p && __ppl) { len = snprintf((__p + (*__ppl)), range, \
"[%s] [threadid: %llu] [%s:%d] "___fmttt___"\n\n", \
tnow, spl_get_threadid(), __FUNCTION__, __LINE__, ##__VA_ARGS__); \
if(len > 0) (*__ppl) += (len -1);}\
spl_mutex_unlock(__mtx__); spl_rel_sem(spl_get_sem());}







#define spllog(__lv__, __fmtt__, ...) { if(spl_get_log_levwel() <= (__lv__) ) {__spl_log_buf__("[%s] -->> "__fmtt__, spl_get_text(__lv__), ##__VA_ARGS__);};}







#define		SPL_LOG_DEBUG				0
#define		SPL_LOG_INFO				70
#define		SPL_LOG_WARNING				80
#define		SPL_LOG_ERROR				90
#define		SPL_LOG_FATAL				100

//	typedef struct __GENERIC_DTA__ {
//		int total;
//		int pc; //Point to the current
//		int pl; //Point to the last
//		char data[0];
//	} generic_dta_st;
//	typedef struct __SIMPLE_LOG_ST__ {
//		int llevel;
//		char folder[1024];
//		char off; //Must be sync
//
//		void* mtx; //Need to close handle
//		void* mtx_off; //Need to close handle
//		void* sem_rwfile; //Need to close handle
//		void* sem_off; //Need to close handle
//
//		void* lc_time; //Need to sync, free
//		void* fp; //Need to close
//		generic_dta_st* buf; //Must be sync, free
//	} SIMPLE_LOG_ST;
	
	DLL_API_SIMPLE_LOG int					spl_set_log_levwel(int val);
	DLL_API_SIMPLE_LOG int					spl_get_log_levwel();
	DLL_API_SIMPLE_LOG int					spl_init_log(char *path);
	DLL_API_SIMPLE_LOG int					spl_finish_log();
	DLL_API_SIMPLE_LOG int					spl_fmt_now(char* fmtt, int len);
	DLL_API_SIMPLE_LOG int					spl_fmmt_now(char* fmtt, int len);
	DLL_API_SIMPLE_LOG int					spl_mutex_lock(void* mtx);
	DLL_API_SIMPLE_LOG int					spl_mutex_unlock(void* mtx);
	DLL_API_SIMPLE_LOG int					spl_set_off(int );
	DLL_API_SIMPLE_LOG int					spl_get_off();
	DLL_API_SIMPLE_LOG void*				spl_get_mtx();
	DLL_API_SIMPLE_LOG void*				spl_get_sem();
//	DLL_API_SIMPLE_LOG SIMPLE_LOG_ST*		spl_get_main_obj();
	DLL_API_SIMPLE_LOG LLU					spl_get_threadid();
	DLL_API_SIMPLE_LOG int					spl_rel_sem(void* sem);
	DLL_API_SIMPLE_LOG const char *			spl_get_text(int lev);
	DLL_API_SIMPLE_LOG char *				spl_get_buf(int* n, int** ppl);

#ifdef __cplusplus
}
#endif
#endif