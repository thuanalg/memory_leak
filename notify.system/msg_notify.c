#include "msg_notify.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t hash_tb_mtx = PTHREAD_MUTEX_INITIALIZER; 

/**************************************************************************************************************/

int uint64_2_arr(unsigned char *arr, uint64_t n, int sz)
{
	int i = 0;
	do {
		if (!arr) break;
		for(i = 0; i < sz; ++i) {
			arr[i] = (n >> (i*8)) & 0xFF;
		}
	}
	while (0);

	return 0;
}

/**************************************************************************************************************/

int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz)
{
	int i = 0;
	uint64_t t = 0;
	do {
		if (!arr) break;
		if (!n) break;
		*n = 0;
		for(i = 0; i < sz; ++i)
		{
			t = arr[i];
			(*n) |= (t << (i*8));
		}
	}
	while (0);
	return 0;
}

/**************************************************************************************************************/

int uint32_2_arr(unsigned char *arr, uint32_t n, int sz)
{
	int i = 0;
	do {
		if (!arr) break;
		for(i = 0; i < sz; ++i) {
			arr[i] = (n >> (i*8)) & 0xFF;
		}
	}
	while (0);

	return 0;
}

/**************************************************************************************************************/

int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz)
{
	int i = 0;
	uint32_t t = 0;
	do {
		if (!arr) break;
		if (!n) break;
		*n = 0;
		for(i = 0; i < sz; ++i)
		{
			t = arr[i];
			(*n) |= (t << (i*8));
		}
	}
	while (0);
	return 0;
}

/**************************************************************************************************************/

int uint16_2_arr(unsigned char *arr, uint16_t n, int sz)
{
	int i = 0;
	do {
		if (!arr) break;
		for(i = 0; i < sz; ++i) {
			arr[i] = (n >> (i*8)) & 0xFF;
		}
	}
	while (0);

	return 0;
}

/**************************************************************************************************************/

int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz)
{
	int i = 0;
	uint32_t t = 0;
	do {
		if (!arr) break;
		if (!n) break;
		*n = 0;
		for(i = 0; i < sz; ++i)
		{
			t = arr[i];
			(*n) |= (t << (i*8));
		}
	}
	while (0);
	return 0;
}

/**************************************************************************************************************/

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

/**************************************************************************************************************/

void dum_ipv4(struct sockaddr_in *addr, const char *f, const char *fu, int line) {
	char buff[1024];
	char str[INET_ADDRSTRLEN + 1];
	memset(buff, 0, sizeof(buff));	
	str[INET_ADDRSTRLEN] = 0;
	inet_ntop(AF_INET, &(addr->sin_addr), str, INET_ADDRSTRLEN);
	sprintf(buff, "File: %s, func: %s, line: %d, cli port: %d, IP: %s.", 
		f, fu, line, (int) htons(addr->sin_port), str);
//	fprintf(stdout, "%s\n", buff);
	LOG(LOG_INFO, buff);
}

/**************************************************************************************************************/

int reg_to_table(MSG_REGISTER *msg, int n, struct timespec *t)
{
	int res = 0;
	unsigned int hn = 0;
	HASH_LIST *hi = 0;
	MSG_COMMON *com = 0;
	HASH_ITEM *hitem = 0;
	int rc = 0;
	int len = 0;

	com = &(msg->com);
	len = MIN(MAX_MSG, strlen(com->dev_id));
	hn = hash_func(com->dev_id, len);
	hi = &(list_reg_dev[hn]);
	
	rc = pthread_mutex_lock(&hash_tb_mtx);
	if (rc) {
		//LOG FATAL
	}

	do {
		if (hi->n)
		{
			int j = 0;
			int check = 1;
			if (!hi->group) {
				//ERROR here
				hi->n = 0;
				break;
			}	
			hitem = hi->group;
			while (hitem)
			{
				int k = strncmp(com->dev_id, hitem->msg->com.dev_id, LEN_DEVID);
				put_time_to_msg(com, t);
				if (!k)
				{
					check = 0;
					break;
				}
				hitem = hitem->next;
				++j;
				if (j >= hi->n) break;
			}
			if (!check)
			{
				fprintf(stdout, "Already existed!\n");
				break;
			}

			MY_MALLOC(hitem, sizeof(HASH_ITEM));
			if (!hitem) {
				//LOG FATAL
				break;
			}
			MY_MALLOC(hitem->msg ,sizeof(HASH_ITEM));
			memcpy(hitem->msg, (char*) msg, n);
			
			hi->n += 1;
			hitem->next = hi->group;
			hi->group = hitem;
			res = 1;
			break;
		}
		else {
			MY_MALLOC( hitem, sizeof(HASH_ITEM));
			if (!hitem) {
				//LOG FATAL
				break;
			}
			MY_MALLOC( hitem->msg, n);
			if (!hitem->msg) {
				//LOG FATAL
				break;
			}
			memcpy(hitem->msg, (char*) msg, n);
			
			hi->n = 1;
			hi->group = hitem;
			res = 1;
			break;
		}	
	}
	while (0);
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if (rc) {
		//LOG FATAL
	}
	return res;
}

/**************************************************************************************************************/

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

	rc = pthread_mutex_lock(mtx);
	if (rc) {
		//LOG FATAL
	}
	do {
		if (!mtx) {
			//LOG ERROR
			break;
		}
		if (hi->n)
		{
			int j = 0;
			int found = 0;
			if (!hi->group) {
				//ERROR here
				hi->n = 0;
				break;
			}	
			hitem = hi->group;
			while (hitem)
			{
				int k = strncmp(com->dev_id, hitem->msg->com.dev_id, LEN_DEVID);
				if (!k)
				{
					found = 1;
					break;
				}
				hitem = hitem->next;
				++j;
				if (j >= hi->n) break;
			}
			if (!found)
			{
				LOG(LOG_ERR, "Not found device ID");
				break;
			}
			//Update route path
			hitem->ipv4 = *addr;
			DUM_IPV4(&(hitem->ipv4));
			res = 1;
		}
	}
	while (0);
	rc = pthread_mutex_unlock(mtx);
	if (rc) {
		//LOG FATAL
	}
	return res;
}

/**************************************************************************************************************/

int add_to_item_list(MSG_NOTIFY *msg, HASH_ITEM **l, int sz)
{
	int err = 0;
	int n = 0;
	int rc = 0;
	HASH_ITEM *hi = 0;

	do {
		if (!l) {
			err = 1;
			//LOG ERROR
			break;
		}
		MY_MALLOC(hi, sizeof(HASH_ITEM));
		if (!hi) {
			//LOG_FATAL
			err = 1;
			break;
		}
		//n = MAX(sz, sizeof(MSG_NOTIFY));
		n = sz;
		MY_MALLOC(hi->msg, n);
		if (!hi->msg) {
			//LOG FATAL
			break;
		}
		memcpy(hi->msg, (char*) msg, n);
		hi->n_msg = n;
		fprintf(stdout, "set hi->n_msg: %d\n", hi->n_msg);

		rc = pthread_mutex_lock(&hash_tb_mtx);
		//>>>>>>
		if (rc) {
			//LOG FATAL
		}
		if (!(*l)) {
			(*l) = hi;
		}
		else {
			hi->next = (void*)(*l);
			(*l) = hi;
		}
		//<<<<<<
		rc = pthread_mutex_unlock(&hash_tb_mtx);
		if (rc) {
			//LOG FATAL
		}
	}	
	while (0);
	return err;
}

/**************************************************************************************************************/

//2023-04-26
void put_time_to_msg( MSG_COMMON *msg, struct timespec *t)
{
	do {
		if (!t) {
			//LOG ERROR
			break;
		}
		if (!msg) {
			//LOG ERROR
			break;
		}
		memset(msg->second, 0, LEN_U64INT);
		memset(msg->nano, 0, LEN_U64INT);
		uint64_2_arr(msg->second, t->tv_sec, LEN_U64INT);	
		uint64_2_arr(msg->nano, t->tv_nsec, LEN_U64INT);	
	}
	while (0);
}

/**************************************************************************************************************/

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
		if (!item) break;
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

	} while (0);	
	LOG(LOG_INFO, buf);	
}

/**************************************************************************************************************/

int load_reg_list() {
	int ret = 0;
	FILE *fp = 0;
	char buf[1024 + 1];
	size_t len = 0;
	size_t n = 0;
	char *data = 0;
	do {
		char *pch = 0;
		MSG_DATA msg;
		struct timespec t = { 0 }; 
		int sz = (int) sizeof(MSG_REGISTER);

		fp = fopen("list_dev_id.h", "r");
		if (!fp) {
			//LOG ERROR
			break;
		}	
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		if (len < 1) {
			//LOG ERROR
			break;
		}
		MY_MALLOC(data, len + 1);
		if (!data) {
			//LOG ERROR
			break;
		}
		memset(data, 0, len + 1);
		rewind(fp);
		len = 0;
		do {
			memset(buf, 0, sizeof(buf));
			n = fread(buf, 1, 1024, fp);
			if (n < 1) {
				break;
			}
			memcpy(data + len, buf, n);
			len += n;
		}
		while (1);
		if (!data) {
			break;
		}
		pch = strtok(data, "\r\n");
		while (pch) {
			if (strlen(pch) > 30) {
				memset(&msg, 0, sz);
				memcpy(msg.com.dev_id, pch, LEN_DEVID);
				clock_gettime(CLOCK_REALTIME, &t);
				ret = reg_to_table((MSG_REGISTER*) &msg, sz, &t);
			}
			pch = strtok(0, "\r\n");
		}
	}
	while (0);
	if (data) {
		MY_FREE(data);
	}
	if (fp) {
		fclose(fp);
	}
	return ret;
}

/**************************************************************************************************************/

int send_to_dst(int sockfd, HASH_ITEM **l, int *count, char clear)
{
	int err = 0;
	int rc = 0;
	int index = 0;
	char *iid = 0;
	HASH_ITEM *hi = 0;
	HASH_ITEM *gr = 0;
	HASH_ITEM *t = 0;
	int len = 0;
	int sn = 0;

	if (count) {
		*count = 0;
	}

	rc = pthread_mutex_lock(&hash_tb_mtx);
	if (rc) {
		//LOG FATAL
	}
	//>>>>>>>
	do {
		if (!l) {
			err = 1;
			//LOG ERR
			break;
		}
		hi = *l;
		*l = 0;
	}
	while (0);
	//<<<<<<<
	rc = pthread_mutex_unlock(&hash_tb_mtx);
	if (rc) {
		//LOG FATAL
	}
	while (hi)
	{
		fprintf(stdout, "error No item\n");
		if (hi->msg->com.ifroute == G_NTF_CLI ) {
			iid = hi->msg->com.dev_id;
		}
		else if (hi->msg->com.ifroute == G_CLI_NTF ) {
			iid = hi->msg->com.ntf_id;
		}
		else {
			break;
		}
		len = MIN(MAX_MSG, strlen(iid));
		index = hash_func(iid, len);		
		gr = (HASH_ITEM*) list_reg_dev[index].group;
		if (!gr) {
			//NOT found in registed list.
			err=1;
			//E_NOT_IN_REG
			break;
		}
		while (gr) {
			if (strncmp(iid, gr->msg->com.dev_id, LEN_DEVID) == 0) {
				t = gr;
				break;		
			}
			gr = gr->next;
		}
		if (!t) {
			//NOT found in registed list.
			err = 1;
			//E_NOT_IN_REG
			break;
		}
		DUM_IPV4(&(t->ipv4));
		//fprintf(stdout, "get hi->n_msg: %d, sizeof: %u\n", hi->n_msg, sizeof(MSG_COMMON));
		sn = sendto(sockfd, (const char *)hi->msg, hi->n_msg,
			MSG_CONFIRM, (const struct sockaddr *) &(t->ipv4), sizeof(t->ipv4));
		fprintf(stdout, "sn: %d\n", sn);
		if (count) {
			(*count)++;
		}
		t = hi;
		hi = hi->next;
		if (clear) {
			MY_FREE(t->msg);
			MY_FREE(t);
		}
	}
	return err;
}

/**************************************************************************************************************/

int send_msg_track(const char *iid, int sockfd, char *ipaddr, int port, struct timespec *t) {

	MSG_COMMON msg;
	struct sockaddr_in addr;
	int n = 0;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipaddr);
	addr.sin_port = htons(PORT + 1);

	do {
		if (!t) {
			break;
		}
		if (!iid) {
			break;
		}
		if (!ipaddr) { 
			break;
		}
		put_time_to_msg( &msg, t);
		memset(&msg, 0, sizeof(msg));
		msg.type = MSG_TRA;
		memcpy(msg.dev_id, iid, LEN_DEVID);

		n = sendto(sockfd, &msg, sizeof(msg),
			MSG_CONFIRM, (const struct sockaddr *) &addr,
				sizeof(addr));
		if (n < 0) {
			fprintf(stdout, "connect err: %d, errno: %d, text: %s\n", n, errno, strerror(errno));
		}
		fprintf(stdout, "tracking sent n: %d\n", n);
	}
	while (0);
	//connect err: -1, errno: 88, text: Socket operation on non-socket
	return n;

}

/**************************************************************************************************************/

int ntf_aes_file(uchar *in, uchar *out, uchar* key, uchar* ivec, int enc) {
	int err = 0;
	FILE *fin = 0;
	FILE *fout = 0;
	uchar bin[AES_BLOCK_SIZE + 1];
	uchar bout[AES_BLOCK_SIZE + 1];
    AES_KEY wctx;
	int k = 0;
	int l = 0;
	uchar iv[AES_BLOCK_SIZE + 1];

	memset(&wctx, 0, sizeof(wctx));

	do {
		int i = 0;

		if (!in) {
			err = 1;
			//SYSLOG err
			//LOG_ERR
			break;
		}
		if (!out) {
			err = 1;
			//SYSLOG err
			//LOG_ERR
			break;
		}
		fin = fopen( in, "rb");
		if (!fin) {
			err = 1;
			//SYSLOG err
			//LOG_ERR
			break;
		}
		fout = fopen( out, "w+b");
		if (!fout) {
			err = 1;
			//SYSLOG err
			//LOG_ERR
			break;
		}

		if (enc) {
    		err = AES_set_encrypt_key(key, AES_BITS, &wctx);
			if (err) {
				fprintf(stdout, "error\n");
				break;
			}
		} else {
    		err = AES_set_decrypt_key(key, AES_BITS, &wctx);
			if (err) {
				fprintf(stdout, "error\n");
				break;
			}
		}

		do {
			memset(bin, 0, sizeof(bin));
			memset(bout, 0, sizeof(bout));
			memset(iv, 0, sizeof(iv));
			memcpy(iv, ivec, AES_BLOCK_SIZE);

			k = fread( bin, 1, AES_BLOCK_SIZE, fin);
			if (k < 1) {
				break;
			}
			i = AES_BLOCK_SIZE;
    		AES_cfb128_encrypt( bin, bout, AES_BLOCK_SIZE, &wctx, iv, &i, enc);  
			l = fwrite(bout, 1, k, fout);
			if (l != k) {
				err = 1;
				LOG(LOG_ERR, "fwrite error.");
				break;
			}
			if (k < AES_BLOCK_SIZE) {
				break;
			}

		} while (1);

	} while (0);

	if (fin) {
		err = fclose(fin);
		if (err) {
			//LOG_ERR
		}
	}

	if (fout) {
		err = fclose(fout);
		if (err) {
			//LOG_ERR
		}
	}

	return err;
} 

/**************************************************************************************************************/

//in: input
//out: ouput
//key: 256 bit = 16 bytes = AES_BLOCK_SIZE bytes
//ivec: ivec suffix, 256 bits = 16 bytes = AES_BLOCK_SIZE bytes
//n: total len, MUST BE multiple of 16, AES_BLOCK_SIZE bytes 
//enc: 1: encrypt, 0: decrypt
int ntf_aes_encrypt(uchar *in, uchar *out, uchar* key, uchar* ivec, int n, int enc) {
	int err = 0;
	int i = 0;
    AES_KEY wctx;
	int k = 0;
	uchar iv[AES_BLOCK_SIZE + 1];

	memset(&wctx, 0, sizeof(wctx));

	do {
		if (n < 1 || (n%AES_BLOCK_SIZE)) {
			err = 1;
			LOG(LOG_ERR, "Length of data must be multiple of %d.", AES_BLOCK_SIZE);
			break;
		}	
		if (enc) {
    		err = AES_set_encrypt_key(key, AES_BITS, &wctx);
			if (err) {
				LOG(LOG_ERR, " set encrypt key error");
				break;
			}
		} else {
    		err = AES_set_decrypt_key(key, AES_BITS, &wctx);
			if (err) {
				LOG(LOG_ERR, " set decrypt key error");
				break;
			}
		}
		while (k < n) {
			i = AES_BLOCK_SIZE;
			memset(iv, 0, sizeof(iv));
			memcpy(iv, ivec, AES_BLOCK_SIZE);
    		AES_cfb128_encrypt(in + k, out + k, AES_BLOCK_SIZE, &wctx, iv, &i, enc);  
			k += AES_BLOCK_SIZE;
		}
	} while (0);

	return err;
} 

/**************************************************************************************************************/

int file_2_bytes(const uchar *path, uchar **output)
{
    FILE *fp = 0;
	int err = 0;
    unsigned int sz = 0, n = 0;
	do {
		if (!path) {
			err = 1;
			LOG(LOG_ERR, "File path is null.");
    	    break;
		}
		if (!output) {
			err = 1;
			LOG(LOG_ERR, "Buffer is null.");
    	    break;
		}
    	fp = fopen(path, "r");
    	if (!fp) {
			err = 1;
			LOG(LOG_ERR, "Cannot open file '%s'.", path);
    	    break;
    	}
    	fseek(fp, 0,SEEK_END);
    	sz = ftell(fp);
    	rewind(fp);
    	//(*output) = malloc(sz + 1);
    	MY_MALLOC( *output, sz + 1);
    	//memset(*output, 0, sz + 1);
    	n = fread(*output, 1, sz, fp);
		if (n != sz) {
			err = 1;
			LOG(LOG_ERR, "Reading file got erroneous.", path);
			break;
		}
	} while (0);

	if (err) {
		if (*output) {
			MY_FREE(*output);
		}
	}
	if (fp) {
    	err = fclose(fp);
		if (err) {
			LOG(LOG_ERR, "Cannot close file '%s'.", path);
		}
	}
	return err;
}

/**************************************************************************************************************/

int file_2_pubrsa(const uchar *path, RSA **output) {
	int err = 0;
	BIO *bio = 0;
	uchar *str = 0;
	RSA *pubkey = 0;
	do {
		if (!output) {
			err = 1;
			LOG(LOG_ERR, "RSA output is 'null'.");
			break;
		}
		err = file_2_bytes(path, &str);
		if (err) {
			LOG(LOG_ERR, "Read public key error.");
			break;
		}
		bio = BIO_new_mem_buf( (void*)str, -1 ) ; // -1: assume string is null terminated
		if (!bio) {
			LOG(LOG_ERR, "Load BIO error.");
			err = 1;
			break;
		}
		BIO_set_flags (bio, BIO_FLAGS_BASE64_NO_NL ) ; // NO NL
		pubkey = PEM_read_bio_RSA_PUBKEY( bio, NULL, NULL, NULL ) ;
  		if (!pubkey) {
    		LOG (LOG_ERR, "ERROR: Could not load PUBLIC KEY!  PEM_read_bio_RSA_PUBKEY FAILED: %s\n", 
			ERR_error_string( ERR_get_error(), NULL ) ) ;
  		}
		*output = pubkey;
		//RSA_free(rsa);
	} while (0);

	if (str) {
		MY_FREE(str);
	}
	if (bio) {
	  BIO_free( bio);
	}

	return err;
}

/**************************************************************************************************************/

int file_2_prvrsa(const uchar *path, RSA **output) {
	int err = 0;
	BIO *bio = 0;
	uchar *str = 0;
	RSA *prvkey = 0;
	do {
		if (!output) {
			err = 1;
			LOG(LOG_ERR, "RSA output is 'null'.");
			break;
		}
		err = file_2_bytes(path, &str);
		if (err) {
			LOG(LOG_ERR, "Read private key error.");
			break;
		}
		bio = BIO_new_mem_buf( (void*)str, -1 );
		if (!bio) {
			err = 1;
			LOG(LOG_ERR, "BIO buffer error.");
			break;
		}
		prvkey = PEM_read_bio_RSAPrivateKey( bio, NULL, NULL, NULL ) ;
		if (!prvkey) {
	  		LOG(LOG_ERR, "ERROR: Could not load PRIVATE KEY!  PEM_read_bio_RSAPrivateKey FAILED: %s\n", 
	  				ERR_error_string(ERR_get_error(), NULL));
		}
		*output = prvkey;
	} while (0);

	if (str) {
		MY_FREE(str);
	}
	if (bio) {
	  BIO_free( bio);
	}

	return err;
}

/**************************************************************************************************************/

int rsa_enc(RSA *pubkey, const uchar *in, uchar **out, int lenin, int *outlen)
{
	int n = 0;
	int err = 0;
	int buflen = 0;
	puchar buf = 0;
	do {
		if (!pubkey) {
			err = 1;
			LOG(LOG_ERR, "Have no public key.");
			break;
		}
		if (!out) {
			err = 1;
			LOG(LOG_ERR, "Have no output pointer.");
			break;
		}
		if (lenin < 1) {
			LOG(LOG_ERR, "Length of input must be greater than 0.");
		}
		buflen = lenin + (16 - lenin%16) + 256;
		MY_MALLOC(buf, buflen);
		n = RSA_public_encrypt(lenin, in, buf, pubkey, RSA_PKCS1_PADDING ) ; 
	  	if (n < 1) {
	    	LOG(LOG_ERR, "ERROR: RSA_public_encrypt: %s\n", ERR_error_string(ERR_get_error(), NULL));
			break;
		}
		fprintf(stdout, "nnnn: %d\n", n);
		if (!outlen) {
			*outlen = n;
		}
		*out = buf;
	} while(0);
  	return err ;
}

/**************************************************************************************************************/

//Refer openssl/test/rsa_test.c
int rsa_dec(RSA *priv, const uchar *in, uchar **out, int lenin, int *outlen)
{
	int err = 0;
	int buflen = 0; // That's how many bytes the decrypted data would be
	puchar buf = 0;

	do {
		int n = 0;
		if (!priv) {
			err = 1;
			LOG(LOG_ERR, "Have no priv key.");
			break;
		}
		if (!in) {
			err = 1;
			LOG(LOG_ERR, "Have no input data");
			break;
		}
		if (!out) {
			err = 1;
			LOG(LOG_ERR, "Have no output buffer.");
			break;
		}
		buflen = lenin + (16 - (lenin%16)) + 16;
		MY_MALLOC(buf, buflen);
  		n = RSA_private_decrypt(lenin, in, buf, priv, RSA_PKCS1_PADDING) ;
		if (n < 1) {
    		LOG(LOG_ERR, "ERROR: RSA_private_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;
			err = 1;
			break;
		}
		fprintf(stdout, "nnnn: %d\n");
		if (outlen) {
			*outlen = n;
		}
		*out = buf;
	} while(0);

	return err;
}

/**************************************************************************************************************/
void rsb() {

}
/**************************************************************************************************************/

HASH_LIST list_reg_dev[HASH_SIZE + 1];
HASH_ITEM *imd_fwd_lt = 0;


