
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



#define LEN_DEVID 	64
#define LEN_U64INT 	8
#define LEN_U32INT 	4
#define LEN_U16INT 	2


#define HASH_SIZE 10001


typedef enum {
	MSG_REG = 0,
	MSG_TRA, //msg tracing device`
	MSG_NOT, // msg notification from dev
	MSG_CON, // msg confirmation
	MSG_TRA_ER, //msg tracking from notifier 
	MSG_NOT_ER, //msg notification fron notifier
	MSG_CON_ER, //msg confirmation fron notifier
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
	char dev_id[LEN_DEVID];
	unsigned char second[LEN_U64INT];
	unsigned char nano[LEN_U64INT];
	unsigned char len[LEN_U32INT];
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

int notify_to_client(int sockfd, int *count);

//2023-04-26
void put_time_to_msg( MSG_COMMON *, struct timespec *t);
int rm_msg_sent(MSG_COMMON *msg);

extern HASH_LIST list_reg_dev[HASH_SIZE + 1];
extern HASH_LIST list_reg_notifier[HASH_SIZE + 1];

extern HASH_ITEM *notified_list;
extern HASH_ITEM *notifier_list;

int load_reg_list();

#ifndef __cplusplus
#endif