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


void dum_ipv4(struct sockaddr_in *addr, const char *f, const char *fu, int line) {
	char buff[1024];
	char str[INET_ADDRSTRLEN + 1];
	int n = 0;
	memset(buff, 0, sizeof(buff));	
	str[INET_ADDRSTRLEN] = 0;
	inet_ntop(AF_INET, &(addr->sin_addr), str, INET_ADDRSTRLEN);
	sprintf(buff, "File: %s, func: %s, line: %d, cli port: %d, IP: %s.", 
		f, fu, line, (int) addr->sin_port, str);
	LOG(LOG_INFO, buff);
}


int reg_to_table(MSG_REGISTER *msg, int n, struct timespec *t)
{
	int res = 0;
	unsigned int hn = 0;
	HASH_LIST *hi = 0;
	MSG_COMMON *com = 0;
	HASH_ITEM *hitem = 0;
	int rc = 0;
	char *devid;
	int len = 0;

	com = &(msg->com);
	len = MIN(MAX_MSG, strlen(com->dev_id));
	hn = hash_func(com->dev_id, len);
	fprintf(stdout, "hn: %llu, devid: %s\n", hn, com->dev_id);
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

			MY_MALLOC(hitem, sizeof(HASH_ITEM));
			if(!hitem) {
				//LOG FATAL
				break;
			}
			memset(hitem, 0, sizeof(HASH_ITEM));		
			MY_MALLOC(hitem->msg ,sizeof(HASH_ITEM));
			memset(hitem->msg, 0, n);
			memcpy(hitem->msg, (char*) msg, n);
			
			hi->n += 1;
			hitem->next = hi->group;
			hi->group = hitem;
			res = 1;
			break;
		}
		else {
			//hitem = malloc(sizeof(HASH_ITEM));
			MY_MALLOC( hitem, sizeof(HASH_ITEM));
			if(!hitem) {
				//LOG FATAL
				break;
			}
			memset(hitem, 0, sizeof(HASH_ITEM));		
			//hitem->msg = malloc(n);
			MY_MALLOC( hitem->msg, n);
			if(!hitem->msg) {
				//LOG FATAL
				break;
			}
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

int hl_track_msg(MSG_TRACKING *msg, int n, struct sockaddr_in *addr, int type) {
	int res = 0;
	unsigned int hn = 0;
	HASH_LIST *hi = 0;
	MSG_COMMON *com = 0;
	HASH_ITEM *hitem = 0;
	int rc = 0;
	HASH_LIST *hash_table = 0;
	pthread_mutex_t *mtx = 0;
	int len = 0;

	mtx = &hash_tb_mtx;
	hash_table = list_reg_dev;

	com = &(msg->com);
	len = MIN(MAX_MSG, strlen(com->dev_id));
	hn = hash_func(com->dev_id, len);
	hi = &(hash_table[hn]);

	fprintf(stdout, "============ Func: %s, line: %d, Update route path, device id: %s.\n\n", 	
		__FUNCTION__, __LINE__, msg->com.dev_id);
	rc = pthread_mutex_lock(mtx);
	if(rc) {
		//LOG FATAL
	}
	do {
		if(!mtx) {
			//LOG ERROR
			break;
		}
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
			DUM_IPV4(&(hitem->ipv4));
			res = 1;
		}
	}
	while(0);
	rc = pthread_mutex_unlock(mtx);
	if(rc) {
		//LOG FATAL
	}
	return res;
}

int add_to_item_list(MSG_NOTIFY *msg, HASH_ITEM **l, int sz)
{
	int err = 0;
	int n = 0;
	int rc = 0;
	HASH_ITEM *hi = 0;

	do {
		if(!l) {
			err = 1;
			//LOG ERROR
			break;
		}
		MY_MALLOC(hi, sizeof(HASH_ITEM));
		if(!hi) {
			//LOG_FATAL
			err = 1;
			break;
		}
		n = MAX(sz, sizeof(MSG_NOTIFY));
		memset(hi, 0, sizeof(HASH_ITEM));
		//hi->msg = malloc(n);
		MY_MALLOC(hi->msg, n);
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
		if(!(*l)) {
			(*l) = hi;
		}
		else {
			hi->next = (void*)(*l);
			(*l) = hi;
		}
		//<<<<<<
		rc = pthread_mutex_unlock(&hash_tb_mtx);
		if(rc) {
			//LOG FATAL
		}
	}	
	while(0);
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

void dum_msg(MSG_COMMON *item, const char *f, const char *fu, int line)
{
	const char *text[] = { "Register", "Trace", "Notify", "Confirm", "" };
	const char *ifr[] = { "From a notifier to server", "The server forwards to client", 
		"From the server feedback to a notifier", " From a client to the server in order to confirm", 
		"From the notifier feedback to the server to confirm and clean the list", ""};
	char buf[2048];
	int n = 0;
	int len = 0;
	int sz = 0;
	short ldta = 0;
	uint64_t sec = 0, nsec = 0;
	memset(buf, 0, sizeof(buf));
	do {
		unsigned char type = 0;
		unsigned char ifro = 0;
		if(!item) break;
		type = item->type;
		ifro = item->ifroute;
		len += n;
		n = sprintf(buf + len, "------File: %s, Func: %s line: %d -------- Type of message: %s. ", f, fu, line, text[type]);
		len += n;
		n = sprintf(buf + len, "------Device ID: %s. ", item->dev_id);
		len += n;
		n = sprintf(buf + len, "------Notifier ID: %s. ", item->ntf_id);
		len += n;
		sz = MIN(MAX_MSG, strlen(item->dev_id));
		n = sprintf(buf + len, "-------- Hash number device id: %u. ", hash_func(item->dev_id, sz));
		len += n;
		n = sprintf(buf + len, "-------- Hash number notifier id: %u. ",  hash_func(item->ntf_id, sz));
		len += n;
		n = sprintf(buf + len, "-------- ifroute: %s. ", ifr[ifro]);
		len += n;
		
		arr_2_uint16(item->len, &ldta, 2);
		n = sprintf(buf + len, "-------- Len data: %d. ", ldta);
		len += n;

		arr_2_uint64(item->second, &sec, 8);
		arr_2_uint64(item->nano, &nsec, 8);
		n = sprintf(buf + len, "-------- (sec, nano) = (%llu, %llu)", sec, nsec);
		len += n;

	} while(0);	
	LOG(LOG_INFO, buf);	
}

int load_reg_list() {
	int ret = 0;
	FILE *fp = 0;
	char buf[1024 + 1];
	size_t len = 0;
	size_t n = 0;
	char *data = 0;
	do {
		char *pch = 0;
		MSG_REGISTER msg;
		struct timespec t = { 0 }; 
		int ret = 0;
		int sz = (int) sizeof(MSG_REGISTER);

		fp = fopen("list_dev_id.h", "r");
		if(!fp) {
			//LOG ERROR
			break;
		}	
		fprintf(stdout, "fp: %p\n", fp);
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		if(len < 1) {
			//LOG ERROR
			break;
		}
		MY_MALLOC(data, len + 1);
		if(!data) {
			//LOG ERROR
			break;
		}
		memset(data, 0, len + 1);
		rewind(fp);
		len = 0;
		do {
			memset(buf, 0, sizeof(buf));
			n = fread(buf, 1, 1024, fp);
			if(n < 1) {
				break;
			}
			memcpy(data + len, buf, n);
			len += n;
		}
		while(1);
		if(!data) {
			break;
		}
		pch = strtok(data, "\r\n");
		while(pch) {
			if(strlen(pch) > 30) {
				fprintf(stdout, "device_id: %s\n", pch);
				memset(&msg, 0, sz);
				memcpy(msg.com.dev_id, pch, LEN_DEVID);
				clock_gettime(CLOCK_REALTIME, &t);
				ret = reg_to_table((MSG_REGISTER*) &msg, sz, &t);
			}
			pch = strtok(0, "\r\n");
		}
	}
	while(0);
	if(data) {
		MY_FREE(data);
	}
	if(fp) {
		fclose(fp);
	}
	return ret;
}


int send_to_dst(int sockfd, HASH_ITEM **l, int *count, char clear)
{
	int err = 0;
	int rc = 0;
	int index = 0;
	char *iid = 0;
	HASH_ITEM *hi = 0;
	HASH_ITEM *gr = 0;
	HASH_ITEM *t = 0;

	rc = pthread_mutex_lock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	//>>>>>>>
	do {
		if(!l) {
			err = 1;
			//LOG ERR
			break;
		}
		hi = *l;
		if(!count) {
			//LOG ERR
			break;
		}
		*count = 0;
		if(!hi) 	
		{
			break;	
		}
		*l = 0;
	}
	while(0);
	//<<<<<<<
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if(rc) {
		//LOG FATAL
	}
	while(hi)
	{
		int len = 0;
		if(hi->msg->com.ifroute == G_NTF_CLI ) {
			iid = hi->msg->com.dev_id;
		}
		else if(hi->msg->com.ifroute == G_CLI_NTF ) {
			iid = hi->msg->com.ntf_id;
		}
		else {
			break;
		}
		len = MIN(MAX_MSG, strlen(iid));
		index = hash_func(iid, len);		
		gr = (HASH_ITEM*) list_reg_dev[index].group;

		fprintf(stdout, "===========func: %s, line: %d, hi: %p, gr: %p, devid: %s\n\n\n",
			 __FUNCTION__, __LINE__, hi, gr, iid);

		if(!gr) {
			//NOT found in registed list.
			err=1;
			//E_NOT_IN_REG
			break;
		}
		while(gr) {
			if(strncmp(iid, gr->msg->com.dev_id, LEN_DEVID) == 0) {
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
		DUM_IPV4(&(t->ipv4));
		sendto(sockfd, (const char *)hi->msg, sizeof(MSG_COMMON),
			MSG_CONFIRM, (const struct sockaddr *) &(t->ipv4), sizeof(t->ipv4));
		(*count)++;
		t = hi;
		hi = hi->next;
		if(clear) {
			MY_FREE(t->msg);
			MY_FREE(t);
		}
	}
	return err;
}

HASH_LIST list_reg_dev[HASH_SIZE + 1];
HASH_ITEM *imd_fwd_lt = 0;


