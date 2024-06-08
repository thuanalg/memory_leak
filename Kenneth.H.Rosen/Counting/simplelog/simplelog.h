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



#define consimplelog(___fmttt___, ...)		fprintf(stdout, "[WIN32_MSVC] "__FILE_LINE_SIMPLELOG__" "___fmttt___"\n" ,__FILE__, __LINE__, ##__VA_ARGS__)
#define consimplelog_buffer(buuf__, _n_n, ___fmttt___, ...)		snprintf((buuf__), (_n_n), __SIMPLE_LOG_PLATFORM__" "__FILE_LINE_SIMPLELOG__" "___fmttt___"\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define		LOG_DEBUG				0
#define		LOG_INFO				70
#define		LOG_WARNING				80
#define		LOG_ERROR				90
#define		LOG_FATAL				100

	typedef struct __SIMPLE_LOG_ST__ {
		int llevel;
		char folder[1024];
		char ready;
		int szbuf;
		void* mtx;
		void* sem_rwfile;

		void* fp;
		char off;		
	} SIMPLE_LOG_ST;
	
	DLL_API_SIMPLE_LOG int			simple_set_log_levwel(int val);
	DLL_API_SIMPLE_LOG int			simple_get_log_levwel();
	DLL_API_SIMPLE_LOG int			simple_init_log(char *path);
	DLL_API_SIMPLE_LOG LLU			simple_log_time_now(int* delta);

#ifdef __cplusplus
}
#endif
#endif