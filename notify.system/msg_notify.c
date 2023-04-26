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


int reg_to_table(MSG_REGISTER *msg, int n, struct timespec *t)
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
				put_time_to_msg(com, t);
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

	fprintf(stdout, "============ Func: %s, line: %d, Update route path, device id: %s.\n\n", 	
		__FUNCTION__, __LINE__, msg->com.dev_id);
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
			fprintf(stdout, "============ line: %d, Update route path, device id: %s.\n\n", __LINE__, hitem->msg->com.dev_id);
			hitem->ipv4 = *addr;
			dum_ipv4(&(hitem->ipv4), __LINE__);
			res = 1;
		}
	}
	while(0);
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	return res;
}

#define MAX(a,b) ((a) > (b) ? (a) : (b))

int add_to_notify_list(MSG_NOTIFY *msg, int sz) {
	int err = 0;
	int n = 0;
	int rc = 0;
	HASH_ITEM *hi = 0;

	fprintf(stdout, "func: %s, line: %d, hi: %p\n\n", __FUNCTION__, __LINE__, hi);
	do {
		hi = malloc(sizeof(HASH_ITEM));	
		if(!hi) {
			//LOG_FATAL
			err = 1;
			break;
		}
		n = MAX(sz, sizeof(MSG_NOTIFY));
		memset(hi, 0, sizeof(HASH_ITEM));
		hi->msg = malloc(n);
		if(!hi->msg) {
			//LOG FATAL
			break;
		}
		memset(hi->msg, 0, n);
		memcpy(hi->msg, (char*) msg, n);

		rc = pthread_mutex_lock(&hash_tb_mtx);
		//>>>>>>
		if(rc) {
			//LOG FATAL
		}
		if(!notified_list) {
			notified_list = hi;
		}
		else {
			hi->next = (void*)notified_list;
			notified_list = hi;
		}
		//<<<<<<
		rc = pthread_mutex_unlock(&hash_tb_mtx);
		if(rc) {
			//LOG FATAL
		}
	}	
	while(0);
	fprintf(stdout, "===========func: %s, line: %d, hi: %p\n\n", __FUNCTION__, __LINE__, hi);
	return err;
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

//typedef struct {
////0: Registering message
////1: Tracking msg 
////2: Notifying msg 
////3: Confirming msg 
//	unsigned char type;
//	char dev_id[LEN_DEVID];
//	unsigned char second[LEN_U64INT];
//	unsigned char nano[LEN_U64INT];
//	unsigned char len[LEN_U32INT];
//} MSG_COMMON;
//
int notify_to_client(int sockfd, int *count) {
	int err = 0;
	int rc = 0;
	int index = 0;
	char *devid = 0;
	HASH_ITEM *hi = 0;
	HASH_ITEM *gr = 0;

	rc = pthread_mutex_lock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	//>>>>>>>
	do {
		hi = notified_list;
		if(!count) {
			//LOG ERR
			break;
		}
		if(!hi) 	
		{
			break;	
		}
		while(hi)
		{
			HASH_ITEM *t = 0;
			devid = hi->msg->com.dev_id;
			index = hash_func(devid, 64);		
			gr = (HASH_ITEM*) list_reg_dev[index].group;

			fprintf(stdout, "===========func: %s, line: %d, hi: %p, gr: %p, devid: %s\n\n\n",
				 __FUNCTION__, __LINE__, hi, gr, devid);
			if(!gr) {
				//NOT found in registed list.
				err=1;
				//E_NOT_IN_REG
				break;
			}
			while(gr) {
				if(strncmp(devid, gr->msg->com.dev_id, LEN_DEVID) == 0) {
					t = gr;
					break;		
				}
				gr = gr->next;
			}
			if(!t) {
				//NOT found in registed list.
				err = 1;
				//E_NOT_IN_REG
				break;
			}
			fprintf(stdout, "===========func: %s, line: %d, hi: %p, gr: %p, t: %p,devid: %s\n\n\n",
				 __FUNCTION__, __LINE__, hi, gr, t, devid);
			dum_ipv4(&(t->ipv4), __LINE__);
			//haha	
//		Prepare list should be done, that will be better. ntthuan: NOT DONE
			sendto(sockfd, (const char *)hi->msg, sizeof(MSG_COMMON),
				MSG_CONFIRM, (const struct sockaddr *) &(t->ipv4), sizeof(t->ipv4));
			(*count)++;
			hi = hi->next;
		}
	}
	while(0);
	//<<<<<<<
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	return err;
}


//2023-04-26
void put_time_to_msg( MSG_COMMON *msg, struct timespec *t)
{
	do {
		if(!t) {
			//LOG ERROR
			break;
		}
		if(!msg) {
			//LOG ERROR
			break;
		}
		memset(msg->second, 0, LEN_U64INT);
		memset(msg->nano, 0, LEN_U64INT);
		uint64_2_arr(msg->second, t->tv_sec, LEN_U64INT);	
		uint64_2_arr(msg->nano, t->tv_nsec, LEN_U64INT);	
	}
	while(0);
}


void dum_msg(MSG_COMMON *item, int line)
{
	const char *text[] = { "Register", "Trace", "Notify", "Confirm", "" };
	do {
		unsigned char type = 0;
		if(!item) break;
		type = item->type;
		fprintf(stdout, "line: %d -------- Type of message: %s\n", line, text[type]);
		fprintf(stdout, "line: %d -------- Is feedback: %s\n", line, item->ifback ? "YES" : "NO");
		fprintf(stdout, "line: %d -------- Device ID: %s\n", line, item->dev_id);
		fprintf(stdout, "line: %d -------- Hash number: %u\n", line, hash_func(item->dev_id, 64));
	} while(0);	
}

int rm_msg_sent(MSG_COMMON *msg)
{
	int err = 0;
	HASH_ITEM *hi = 0;
	HASH_ITEM *prev = 0;
	HASH_ITEM *next = 0;
	int i = 0, rc = 0;
	int found = 0;

	fprintf(stdout, ">>>>>>>>>>: fun: %s, line: %d, left: %s, right: %s\n", __FUNCTION__, __LINE__, "---::w", msg->dev_id);
	rc = pthread_mutex_lock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	//>>>>>>>
	do {
	fprintf(stdout, ">>>>>>>>>>: fun: %s, line: %d, left: %s, right: %s\n", __FUNCTION__, __LINE__, "---::w", msg->dev_id);
		hi = notified_list;
		if(!hi) {
			//LOG ERROR
			err = 1;
			break;
		}	
	fprintf(stdout, ">>>>>>>>>>: fun: %s, line: %d, left: %s, right: %s\n", __FUNCTION__, __LINE__, "---::w", msg->dev_id);
		if(!msg) {
			//LOG ERROR
			err = 1;
			break;
		}	
	fprintf(stdout, ">>>>>>>>>>: fun: %s, line: %d, left: %s, right: %s\n", __FUNCTION__, __LINE__, "---::w", msg->dev_id);
		while(hi) {
			int k = 0;
			char *devid = 0;
			devid = hi->msg->com.dev_id;
			fprintf(stdout, ">>>>>>>>>>: fun: %s, line: %d, left: %s, right: %s\n", __FUNCTION__, __LINE__, devid, msg->dev_id);
			k = strncmp(devid, msg->dev_id, LEN_DEVID);
			if(k == 0) {
				next = hi->next;
				found = 1;
				break;
			} 
			if(i > 0) {
				prev = hi;
			}
			hi = hi->next;
			++i;
		}	
		if(found) {
			if(i == 0) {
				notified_list = next;
			}
			else {
				prev->next = next;
			}
		}
	}
	while(0);	
	//<<<<<<<
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	if(found && hi) {
		if(hi->msg) {
			free(hi->msg);
		}
		free(hi);
	}		
	return 0;
}

HASH_LIST list_reg_dev[HASH_SIZE + 1];
HASH_ITEM *notified_list = 0;
//After receiving a message, include MSG_REG, except MSG_TRA:
//1. Get current time
//2. Put current time to the message
//3. Update current time into the hash list
//4. Put the message to the "notified list"
//5. Send message to a specific device
