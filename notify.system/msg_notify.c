#include "msg_notify.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t hash_tb_mtx = PTHREAD_MUTEX_INITIALIZER; 

int uint64_2_arr(unsigned char *arr, uint64_t n, int sz)
{
	int i = 0;
	do {
		if(!arr) break;
		for(i = 0; i < sz; ++i) {
			arr[i] = (n >> (i*8)) & 0xFF;
		}
	}
	while(0);

	return 0;
}

int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz)
{
	int i = 0;
	uint64_t t = 0;
	do {
		if(!arr) break;
		if(!n) break;
		*n = 0;
		for(i = 0; i < sz; ++i)
		{
			t = arr[i];
			(*n) |= (t << (i*8));
		}
	}
	while(0);
	return 0;
}


int uint32_2_arr(unsigned char *arr, uint32_t n, int sz)
{
	int i = 0;
	do {
		if(!arr) break;
		for(i = 0; i < sz; ++i) {
			arr[i] = (n >> (i*8)) & 0xFF;
		}
	}
	while(0);

	return 0;
}

int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz)
{
	int i = 0;
	uint32_t t = 0;
	do {
		if(!arr) break;
		if(!n) break;
		*n = 0;
		for(i = 0; i < sz; ++i)
		{
			t = arr[i];
			(*n) |= (t << (i*8));
		}
	}
	while(0);
	return 0;
}

int uint16_2_arr(unsigned char *arr, uint16_t n, int sz)
{
	int i = 0;
	do {
		if(!arr) break;
		for(i = 0; i < sz; ++i) {
			arr[i] = (n >> (i*8)) & 0xFF;
		}
	}
	while(0);

	return 0;
}

int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz)
{
	int i = 0;
	uint32_t t = 0;
	do {
		if(!arr) break;
		if(!n) break;
		*n = 0;
		for(i = 0; i < sz; ++i)
		{
			t = arr[i];
			(*n) |= (t << (i*8));
		}
	}
	while(0);
	return 0;
}


unsigned int hash_func(char *id, int n)
{
	unsigned int res = 0;
	int i = 0;
	for(i = 0; i<n; i++)
	{
		unsigned long long k = 1;
		for(int j = 0; j < i; ++j) {
			k *= 71;
			k %= HASH_SIZE;
		}
		res += (id[i] * k) % HASH_SIZE; 
	}
	res = res % HASH_SIZE;
	return res;
}


void dum_ipv4(struct sockaddr_in *addr, int line) {
	char buff[1024];
	char str[INET_ADDRSTRLEN + 1];
	int n = 0;
	memset(buff, 0, sizeof(buff));	
	str[INET_ADDRSTRLEN] = 0;
	inet_ntop(AF_INET, &(addr->sin_addr), str, INET_ADDRSTRLEN);
	sprintf(buff, "\nfunc: %s, line: %d, cli port: %d, IP: %s\n", 
		__FUNCTION__, line, (int) addr->sin_port, str);
	fprintf(stdout, buff);
}


int reg_to_table(MSG_REGISTER *msg, int n)
{
	int res = 0;
	unsigned int hn = 0;
	HASH_LIST *hi = 0;
	MSG_COMMON *com = 0;
	HASH_ITEM *hitem = 0;
	int rc = 0;

	com = &(msg->com);
	hn = hash_func(com->dev_id, 64);
	hi = &(list_reg_dev[hn]);
	
	rc = pthread_mutex_lock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}

	do {
		if(hi->n)
		{
			int j = 0;
			int check = 1;
			if(!hi->group) {
				//ERROR here
				hi->n = 0;
				break;
			}	
			hitem = hi->group;
			while(hitem)
			{
				int k = strncmp(com->dev_id, hitem->msg->com.dev_id, 64);
				fprintf(stdout, "\n\nkkkkkkk: %d, id: %s, id_old: %s\n\n", k, com->dev_id, hitem->msg->com.dev_id);
				if(!k)
				{
					check = 0;
					break;
				}
				hitem = hitem->next;
				++j;
				if(j >= hi->n) break;
			}
			if(!check)
			{
				fprintf(stdout, "Already existed!\n");
				break;
			}
//typedef struct {
//	int n;
//	void *group;
//} HASH_LIST;

//typedef struct __HASH_ITEM {
//	struct sockaddr_in ipv4;
//	struct sockaddr_in6 ipv6;
//	MSG_NOTIFY *msg;
//	struct __HASH_ITEM *next;
//} HASH_ITEM;


			hitem = malloc(sizeof(HASH_ITEM));
			if(!hitem) {
				//LOG FATAL
			}
			memset(hitem, 0, sizeof(HASH_ITEM));		
			hitem->msg = malloc(n);
			memset(hitem->msg, 0, n);
			memcpy(hitem->msg, (char*) msg, n);
			
			hi->n += 1;
			hitem->next = hi->group;
			hi->group = hitem;
			res = 1;
			break;
		}
		else {
			hitem = malloc(sizeof(HASH_ITEM));
			if(!hitem) {
				//LOG FATAL
			}
			memset(hitem, 0, sizeof(HASH_ITEM));		
			hitem->msg = malloc(n);
			memset(hitem->msg, 0, n);
			memcpy(hitem->msg, (char*) msg, n);
			
			hi->n = 1;
			hi->group = hitem;
			res = 1;
			break;
		}	
	}
	while(0);
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	return res;
}

int hl_track_msg(MSG_TRACKING *msg, int n, struct sockaddr_in *addr) {
	int res = 0;
	unsigned int hn = 0;
	HASH_LIST *hi = 0;
	MSG_COMMON *com = 0;
	HASH_ITEM *hitem = 0;
	int rc = 0;

	com = &(msg->com);
	hn = hash_func(com->dev_id, 64);
	hi = &(list_reg_dev[hn]);

	rc = pthread_mutex_lock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	do {
		if(hi->n)
		{
			int j = 0;
			int found = 0;
			if(!hi->group) {
				//ERROR here
				hi->n = 0;
				break;
			}	
			hitem = hi->group;
			while(hitem)
			{
				int k = strncmp(com->dev_id, hitem->msg->com.dev_id, 64);
				fprintf(stdout, "\n\nkkkkkkk: %d, id: %s, id_old: %s\n\n", k, com->dev_id, hitem->msg->com.dev_id);
				if(!k)
				{
					found = 1;
					break;
				}
				hitem = hitem->next;
				++j;
				if(j >= hi->n) break;
			}
			if(!found)
			{
				fprintf(stdout, "Not found device!\n");
				break;
			}
			//Update route path
			fprintf(stdout, "Update route path.\n");
			hitem->ipv4 = *addr;
			dum_ipv4(&(hitem->ipv4), __LINE__);
			res = 1;
		}
//typedef struct {
//	int n;
//	void *group;
//} HASH_LIST;

//typedef struct __HASH_ITEM {
//	struct sockaddr_in ipv4;
//	struct sockaddr_in6 ipv6;
//	MSG_NOTIFY *msg;
//	struct __HASH_ITEM *next;
//} HASH_ITEM;
	}
	while(0);
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	return res;
}

HASH_LIST list_reg_dev[HASH_SIZE + 1];
