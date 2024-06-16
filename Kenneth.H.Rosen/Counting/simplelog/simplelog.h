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
	#define				__FILE_LINE_SIMPLELOG__							"[%s:%d]"
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


		SPL_END_ERROR,
	} SPL_LOG_ERROR;


#define consimplelog(___fmttt___, ...)		fprintf(stdout, "[WIN32_MSVC] "__FILE_LINE_SIMPLELOG__" "___fmttt___"\n" ,__FILE__, __LINE__, ##__VA_ARGS__)
#define consimplelog_buffer(buuf__, _n_n, ___fmttt___, ...)		snprintf((buuf__), (_n_n), __SIMPLE_LOG_PLATFORM__" "__FILE_LINE_SIMPLELOG__" "___fmttt___"\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define		LOG_DEBUG				0
#define		LOG_INFO				70
#define		LOG_WARNING				80
#define		LOG_ERROR				90
#define		LOG_FATAL				100

	typedef struct __GENERIC_DTA__ {
		int total;
		int pc; //Point to the current
		int pl; //Point to the last
		char data[0];
	} generic_dta_st;
	typedef struct __SIMPLE_LOG_ST__ {
		int llevel;
		char folder[1024];

		void* mtx; //Need to close
		void* sem_rwfile; //Need to close
		void* lc_time; //Need to free
		void* fp; //Need to close

		char off; //Must be sync
		generic_dta_st* buf; //Must be sync
	} SIMPLE_LOG_ST;
	
	DLL_API_SIMPLE_LOG int			simple_set_log_levwel(int val);
	DLL_API_SIMPLE_LOG int			simple_get_log_levwel();
	DLL_API_SIMPLE_LOG int			simple_init_log(char *path);
	DLL_API_SIMPLE_LOG LLU			simple_log_time_now(int* delta);
	DLL_API_SIMPLE_LOG int			simple_log_name_now(char* name);
	DLL_API_SIMPLE_LOG int			simple_log_name_now(char* name);
	DLL_API_SIMPLE_LOG int			simple_log_fmt_now(char* fmtt, int len);
	DLL_API_SIMPLE_LOG int			spl_mutex_lock(void* mtx);
	DLL_API_SIMPLE_LOG int			spl_mutex_unlock(void* mtx);
	DLL_API_SIMPLE_LOG int			spl_set_off(int );
	DLL_API_SIMPLE_LOG int			spl_get_off();

#ifdef __cplusplus
}
#endif
#endif