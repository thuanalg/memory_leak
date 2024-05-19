#ifndef ___MYSIMPLE_LOG__
#define ___MYSIMPLE_LOG__
#ifdef __cplusplus
extern "C" {
#endif
#ifndef ______MYSIMPLE_STATIC_LOG__
	#ifdef EXPORT_DLL_API_SIMPLE_LOG
		#define DLL_API_SIMPLE_LOG		__declspec(dllexport)
	#else
		#define DLL_API_SIMPLE_LOG		__declspec(dllimport)
	#endif
#else
	#define DLL_API_SIMPLE_LOG
#endif
#define		LOG_DEBUG				0
#define		LOG_INFO				10
#define		LOG_WARNING				30
#define		LOG_ERROR				50
#define		LOG_FATAL				100
	typedef struct __SIMPLE_LOG_ST__ {
		char filepath[1024];
		void* fp;
		int szbuf;
		void *mtx;
		void* sem_trigger;
		char off;
	} SIMPLE_LOG_ST;
	extern int			simple_log_levwel;
	DLL_API_SIMPLE_LOG int			simple_set_log_levwel(int val);
	DLL_API_SIMPLE_LOG int			simple_get_log_levwel();
	DLL_API_SIMPLE_LOG int			simple_init_log(int lvel, char *path);

#ifdef __cplusplus
}
#endif
#endif