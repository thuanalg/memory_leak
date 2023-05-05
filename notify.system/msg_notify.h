
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
#include <syslog.h>


#define MY_MALLOC(p, n) {p=malloc(n);syslog(LOG_INFO, "- File: %s, func: %s, line: %d, malloc p: %p, n: %d\n", __FILE__, __FUNCTION__, __LINE__, p, (n)); }
#define MY_FREE(p) {free(p);syslog(LOG_INFO, "- File: %s, func: %s, line: %d, free p: %p\n", __FILE__, __FUNCTION__, __LINE__, p);}

#define LOG 		syslog

#define LEN_DEVID 	(40) 
#define LEN_U64INT 	(8) 
#define LEN_U32INT 	(4) 
#define LEN_U16INT 	(2) 
#define MAX_MSG 	(1200) 

#define MAX(a, b) 	((a) > (b) ? (a) : (b))
#define MIN(a, b) 	((a) > (b) ? (b) : (a))

#define HASH_SIZE 		(10001)
//Interval sending tracking message
#define INTER_TRACK 	(30)


typedef enum {
	MSG_REG = 0,
	MSG_TRA, //msg tracing device`
	MSG_NTF, // msg notification from dev
	MSG_CNF, // msg confirmation
} MSG_ENUM;

typedef enum {
	// From a notifier to server
	G_NTF_CLI,
	//The server forwards to client
	F_NTF_CLI,
	//From the server feedback to a notifier
	G_CLI_NTF,
	//From a client to the server in order to confirm
	F_CLI_NTF,
} MSG_ROUTE;


typedef struct {
	int n;
	void *group;
} HASH_LIST;


typedef struct {
//0: Registering message
//1: Tracking msg 
//2: Notifying msg 
//3: Confirming msg 
	unsigned char type; //MSG_ENUM
	unsigned char ifroute;
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

//sk: socket
//l: a linked list
//c: number sent
//clear: clear the list
int send_to_dst(int sk, HASH_ITEM **l, int *c, char clear);

#define send_imd_fwd			send_to_dst 
#define send_imd_fbk			send_to_dst 
#define send_rgl_fwd			send_to_dst 
#define send_rgl_fbk			send_to_dst 

int notify_to_client(int sockfd, int *count);

//2023-04-26
void put_time_to_msg( MSG_COMMON *, struct timespec *t);
int rm_msg_sent(MSG_COMMON *msg);

extern HASH_LIST list_reg_dev[HASH_SIZE + 1];
extern HASH_LIST list_reg_notifier[HASH_SIZE + 1];

extern HASH_ITEM *notified_list;



extern HASH_ITEM *imd_fwd_lt;

int load_reg_list();

#ifndef __cplusplus
#endif
