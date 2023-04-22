
#ifndef __cplusplus
#endif
#ifndef _DEFAULT_SOURCE
	#define _DEFAULT_SOURCE
#endif
#include <endian.h>
#include <stdint.h>
typedef struct {
//0: Registering message
//1: Tracking msg 
//2: Notifying msg 
//3: Confirming msg 
	unsigned char type;
	
	char dev_id[64];
	unsigned char second[8];
	unsigned char nano[8];
	
	unsigned char len[4];
} MSG_COMMON;

typedef struct {
	MSG_COMMON com;
	char data[0];
} MSG_NOTIFY;

int uint64_2_arr(unsigned char *arr, uint64_t , int sz);
int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz);


int uint32_2_arr(unsigned char *arr, uint32_t , int sz);
int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz);


int uint16_2_arr(unsigned char *arr, uint16_t , int sz);
int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz);

#ifndef __cplusplus
#endif
