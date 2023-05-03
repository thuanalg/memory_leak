
#ifndef __cplusplus
#endif
#ifndef _DEFAULT_SOURCE
	#define _DEFAULT_SOURCE
#endif
#include <syslog.h>
#include <endian.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>



#define LEN_DEVID 	(40) 
#define LEN_U64INT 	(8) 
#define LEN_U32INT 	(4) 
#define LEN_U16INT 	(2) 
#define MAX_MSG 	(1200) 

#define MAX(a, b) 	((a) > (b) ? (a) : (b))
#define MIN(a, b) 	((a) > (b) ? (b) : (a))

#define HASH_SIZE 		(10001)
#define INTER_TRACK 	(10)


typedef enum {
	MSG_REG = 0,
	MSG_TRA, //msg tracing device`
	MSG_NOT, // msg notification from dev
	MSG_CON, // msg confirmation
	MSG_NOTIFIER, //msg notification fron notifier
	MSG_NTF_CFM, //msg confirmation fron notifier
} MSG_ENUM;

typedef struct {
	int n;
	void *group;
} HASH_LIST;


typedef struct {
//0: Registering message
//1: Tracking msg 
//2: Notifying msg 
//3: Confirming msg 
	unsigned char type;
	unsigned char ifback;
	unsigned char ntf;
	char dev_id[LEN_DEVID];
	//notifier ID
	char ntf_id[LEN_DEVID];
	unsigned char second[LEN_U64INT];
	unsigned char nano[LEN_U64INT];
	unsigned char len[LEN_U16INT];
} MSG_COMMON;

typedef struct {
	MSG_COMMON com;
	char data[0];
} MSG_DATA;


#define MSG_NOTIFY			MSG_DATA
#define MSG_REGISTER 		MSG_DATA
#define MSG_TRACKING 		MSG_DATA


typedef struct __HASH_ITEM {
	struct sockaddr_in ipv4;
	struct sockaddr_in ipv4_ntfier;
	struct sockaddr_in6 ipv6;
	MSG_NOTIFY *msg;
	struct __HASH_ITEM *next;
} HASH_ITEM;



unsigned int hash_func(char *id, int n);

int uint64_2_arr(unsigned char *arr, uint64_t , int sz);
int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz);
int uint32_2_arr(unsigned char *arr, uint32_t , int sz);
int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz);
int uint16_2_arr(unsigned char *arr, uint16_t , int sz);
int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz);

void dum_msg(MSG_COMMON *, int);
//struct sockaddr_in servaddr, cliaddr;
void dum_ipv4(struct sockaddr_in *, int line);
//0: error, 1: done
int reg_to_table(MSG_REGISTER *msg, int n, struct timespec *);
//0: error, 1: done
int hl_track_msg(MSG_TRACKING *msg, int n, struct sockaddr_in*, int type);

int add_to_notify_list(MSG_NOTIFY *msg, int sz);
int add_to_item_list(MSG_NOTIFY *msg, HASH_ITEM **l, int sz);
#define  add_to_imd_fwd 		add_to_item_list
#define  add_to_imd_fbk 		add_to_item_list

#define  add_to_rgl_fwd 		add_to_item_list
#define  add_to_rgl_fbk 		add_to_item_list

int notify_to_client(int sockfd, int *count);

//2023-04-26
void put_time_to_msg( MSG_COMMON *, struct timespec *t);
int rm_msg_sent(MSG_COMMON *msg);

extern HASH_LIST list_reg_dev[HASH_SIZE + 1];
extern HASH_LIST list_reg_notifier[HASH_SIZE + 1];

extern HASH_ITEM *notified_list;



extern HASH_ITEM *imd_fbk_lt;
extern HASH_ITEM *imd_fwd_lt;
extern HASH_ITEM *rgl_fbk_lt;
extern HASH_ITEM *rgl_fwd_lt;

int load_reg_list();

#ifndef __cplusplus
#endif
