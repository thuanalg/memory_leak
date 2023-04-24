
#ifndef __cplusplus
#endif
#ifndef _DEFAULT_SOURCE
	#define _DEFAULT_SOURCE
#endif
#include <endian.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>



#define LEN_DEVID 64
#define LEN_U64INT 8
#define LEN_U32INT 4
#define LEN_U16INT 2


#define HASH_SIZE 10001


typedef enum {
	MSG_REG = 0,
	MSG_TRA,
	MSG_NOT,
	MSG_CON,
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
	char dev_id[LEN_DEVID];
	unsigned char second[LEN_U64INT];
	unsigned char nano[LEN_U64INT];
	unsigned char len[LEN_U32INT];
} MSG_COMMON;

typedef struct {
	MSG_COMMON com;
	char data[0];
} MSG_NOTIFY;


typedef struct __HASH_ITEM {
	struct sockaddr_in ipv4;
	struct sockaddr_in6 ipv6;
	MSG_NOTIFY *msg;
	struct __HASH_ITEM *next;
} HASH_ITEM;

unsigned int hash_func(char *id, int n);

#define MSG_REGISTER MSG_NOTIFY

int uint64_2_arr(unsigned char *arr, uint64_t , int sz);
int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz);


int uint32_2_arr(unsigned char *arr, uint32_t , int sz);
int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz);


int uint16_2_arr(unsigned char *arr, uint16_t , int sz);
int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz);


extern HASH_LIST list_reg_dev[HASH_SIZE + 1];

#ifndef __cplusplus
#endif
