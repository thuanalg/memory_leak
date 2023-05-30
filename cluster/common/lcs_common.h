#ifndef __LCS_COMMON_HEADER__

	#ifndef _XOPEN_SOURCE
		#define _XOPEN_SOURCE
	#endif 

	#include <time.h> 
	#include <sys/time.h>

	#define __LCS_COMMON_HEADER__

	 #ifdef LCS_RELEASE_MOD
	 
	 #endif

	#ifndef LCSU64
		#define LCSU64	unsigned long long
	#endif

	#ifndef	LCSUCHAR
		#define LCSUCHAR unsigned char
	#endif	

	#ifndef	LCSUSHORT
		#define LCSUSHORT unsigned short
	#endif	

	#define LCS_AUDD_HASH_ARR_N				2
	#define LCS_AGENTS_HASH_ARR_N			4

	#define lcs_comm_hash_audd(val,sz) (val%sz) 

	#define LCS_SHM_FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

	#define LCS_RETURN_STATUS		"\n%s|%s|%d"	

	#define LCS_WHS_PORT		9999
	#define LCS_WHS_LIMIT		50

	#define LCS_HOUR_INIT		"00:00:00"
	#define LCS_HOUR_LAST		"23:59:59"


	typedef int (lcs_compare_cb)(void *, void *);

	#ifdef __cplusplus
	extern "C" {
	#endif

	
typedef struct LCS_MERGE_COUNTING
{
	char 	date[12];
	char 	hour[4];
	int	 	totalcall;
	int	 	totalfullchannel;
	int 	viettelauto;
	int 	viettelfull;
	int 	mobileauto;
	int		mobilefull;
	int		vinaauto;
	int 	vinafull;	
} 
LCS_MERGE_COUNTING;

typedef struct LCS_MERGE_COUNTING_LIST
{
	LCS_MERGE_COUNTING row;
	struct LCS_MERGE_COUNTING_LIST *next;
} 
LCS_MERGE_COUNTING_LIST;


	

		typedef struct lcs_mmap_agents 
		{
			unsigned long id;
			unsigned long version;
			char agent_number[40];
			char vcontext[255];
			char agent_status[7]; 
			char current_dnd[255];
			char email[255];
			char extension[255];
			char is_allow_mapping;
			char is_enable;
			char is_inbound;
			unsigned long last_history_id;
			char name[40];
			char password[40];
			char phone_number[255];
			unsigned int priority;
			unsigned int queue_id;	
			char f1_code[255];
			char fixed_extension[255];
			char customer_name[255];

		} 
		lcs_mmap_agents;

		typedef struct lcs_mmap_auto_dial_detail
		{
			unsigned long long int 	id;
			char					phone_type[8];
			LCSUSHORT				skill;
			char					customer_id[16];
			char					application_id[32];
			char					note[255];
			char					phone_number[25];
		}
		lcs_mmap_auto_dial_detail;

		typedef struct LCS_AGENTS_SHARED_AH
		{
			lcs_mmap_agents 				a;
			int 							h[LCS_AGENTS_HASH_ARR_N];
		}
		LCS_AGENTS_SHARED_AH;

		typedef struct LCS_AUDD_SHARED_AUH
		{
			lcs_mmap_auto_dial_detail		au;
			int 							h[LCS_AUDD_HASH_ARR_N];
		}
		LCS_AUDD_SHARED_AUH;

		typedef struct LCS_AGENTS_SHARED
		{
			int 				n;
			int 				i;
			LCS_AGENTS_SHARED_AH *ah;
		} 
		LCS_AGENTS_SHARED;

		typedef struct LCS_AUDD_SHARED
		{
			int 				n;
			int 				i;
		} 
		LCS_AUDD_SHARED;

		typedef int (*LCS_SELECT_CB)(void* data, int ncols, char** values, char** headers);

		typedef struct LCS_COMM_TABLES
		{
			char name[64];
			struct LCS_COMM_TABLES *next;
		} LCS_COMM_TABLES;

		typedef struct LCS_LIST_DAY_PIECE
		{
			char node[512];
			char from[32];
			char to[32];
			unsigned char step;
			unsigned long int ulfrom;
			unsigned long int ulto;
			unsigned long int *arr;
		} LCS_LIST_DAY_PIECE;
		
		void lcs_create_dir(char *dirpath, int *err);
		void lcs_time2_str(time_t *, char **);
		void lcs_str2_time(char *, long long int *);
		void lcs_file_text(char *, char **, int *);
		void lcs_bash_cmd(char *, int *);
		void lcs_bash_cmdext(char *, char **,int *);
		void lcs_cgen_list(char*, LCS_COMM_TABLES**, LCS_COMM_TABLES **, int*);
		void lcs_cclear_list(LCS_COMM_TABLES **, int*);
		void lcs_commpre_env(char *, char *, int*);
		void lcs_comm_moving(char *, char *, char *, char *, char **, int*);
		void lcs_comm_replace(char *, char , char , int*);
		void lcs_comm_getrange(LCS_LIST_DAY_PIECE *, int*);
		void lcs_comm_subdir(char *, char ***, int*);
		void lcs_comm_subdir_ext(char *, char ***, int *, int*);
		void lcs_comm_filexist(char *, int*);
		int  lcs_partition (char **arr, int low, int hi);
		void lcs_quick_sort (void ** arr, int low, int hi);
		void lcs_comm_hash(char* key, int len, int *index, int sz);
		//Should use macro to replace simple hash function to speed up
		//void lcs_comm_hash_audd(unsigned long key, int *index, int isize);
		void lcs_diff_time(struct timeval *,struct timeval*,char*, char*, int );
		void lcs_quick_sort_ext(void ** arr, int low, int hi, lcs_compare_cb);
		int  lcs_partition_ext(void **arr, int low, int hi, lcs_compare_cb);
		int	 lcs_merge_counting_cb(void *, void *);

		void lcs_hcollect005_sum(char*, char *, int *);
		void lcs_merge005_list(LCS_MERGE_COUNTING_LIST*, int,char *, int *);
		void lcs_list005_rows(void *, char **,int *err);

		void lcs_comm_get_process(char *path, char *, int *err);
		void lcs_comm_list_files(char *path, char ***, int *err);

	#ifdef __cplusplus
	}
	#endif

#endif
