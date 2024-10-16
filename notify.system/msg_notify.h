
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
#include <errno.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <stdarg.h>
//EVP_aes_256_ctr
//CTR mode was introduced by Whitfield Diffie and Martin Hellman in 1979
#define __LITTLE_ENDIAN__ (1 == *(unsigned char *)&(const int){1})


#define PORT	 			(7770)
#define NTF_PORT	 	(8700)
#define DEV_PORT	 	(7000)
#define MAX_PATH		(2048)	
#ifndef llog
	#define llog(p, fmt, ... ) syslog(p, "%s:%d <<<>>> "fmt, __FILE__, __LINE__, __VA_ARGS__)
#endif
//#define llog(p, fmt) syslog(p, "%s:%d"fmt, __FILE__, __LINE__)

#define MY_MALLOC(p, n) {(p)=malloc(n); if(p){memset(p,0,n); syslog(LOG_INFO, "- File: %s, func: %s, line: %d, malloc p: %p, n: %d", __FILE__, __FUNCTION__, __LINE__, p, (n)); } else { syslog(LOG_ALERT, "- File: %s, func: %s, line: %d, Memory Error.", __FILE__, __FUNCTION__, __LINE__); exit(1); } }
#define MY_FREE(p) {free((p)); syslog(LOG_INFO, "- File: %s, func: %s, line: %d, free p: %p\n", __FILE__, __FUNCTION__, __LINE__, (p)); p = 0;}

#define LOG 		syslog
//#define LOG(a,b)   



#define LEN_DEVID 			(40) 
#define LEN_U64INT 			(8) 
#define LEN_U32INT 			(4) 
#define LEN_U16INT 			(2) 
#define MAX_MSG 			(1200) 
#define uchar				unsigned char
#define cchar				const char
#define cuchar				const unsigned char
#define puchar				unsigned char*
#define uint				unsigned int
#define puint				unsigned int*
#define AES_BITS			(256)
#define RSA_BYTES			(512)
#define	UNIT_BLOCK			(128)
#define AES_BYTES			(32)
#define AES_IV_BYTES		AES_BLOCK_SIZE
#define MAX_DATA			(MAX_MSG - 1 - AES_IV_BYTES)

#define MAX(a, b) 	((a) > (b) ? (a) : (b))
#define MIN(a, b) 	((a) > (b) ? (b) : (a))

#define HASH_SIZE 		(10001)
//Interval sending tracking message
#define INTER_TRACK 	(60)


typedef enum {
	MSG_REG = 0,
	MSG_TRA, //msg tracing device`
	MSG_NTF, // msg notification from dev
	MSG_CNF, // msg confirmation
	MSG_GET_AES, // msg require aes key
} MSG_ENUM;
#define CMD_ENUM MSG_ENUM

typedef enum {
	// From a notifier to server
	G_NTF_CLI,
	//The server forwards to client
	F_NTF_CLI,
	//From the server feedback to a notifier
	G_CLI_NTF,
	//From a client to the server in order to confirm
	F_CLI_NTF,

	G_CLI_SRV,
	F_SRV_CLI,
	G_NTF_SRV,
	F_SRV_NTF,
} MSG_ROUTE;

typedef enum {
	ENCRYPT_NON,
	ENCRYPT_SRV_PUB,
	ENCRYPT_CLI_PUB,
	ENCRYPT_AES,
} RSA_AES;


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

typedef struct {
	int n;
	char *crt;
} MSG_CRT;

#define MSG_NOTIFY			MSG_DATA
#define MSG_REGISTER 		MSG_DATA
#define MSG_TRACKING 		MSG_DATA


typedef struct __HASH_ITEM {
	struct sockaddr_in ipv4;
	struct sockaddr_in ipv4_ntfier;
	struct sockaddr_in6 ipv6;
	size_t n_msg;
	MSG_NOTIFY *msg;
	MSG_CRT *crt;
	struct __HASH_ITEM *next;
} HASH_ITEM;



unsigned int hash_func(char *id, int n);

int uint64_2_arr(unsigned char *arr, uint64_t , int sz);
int arr_2_uint64(unsigned char *arr, uint64_t *n, int sz);
int uint32_2_arr(unsigned char *arr, uint32_t , int sz);
int arr_2_uint32(unsigned char *arr, uint32_t *n, int sz);
int uint16_2_arr(unsigned char *arr, uint16_t , int sz);
int arr_2_uint16(unsigned char *arr, uint16_t *n, int sz);

void dum_msg(MSG_COMMON *, const char *, const char *, int);
#define DUM_MSG(i) dum_msg(i, __FILE__, __FUNCTION__ ,__LINE__)

//struct sockaddr_in servaddr, cliaddr;
void dum_ipv4(struct sockaddr_in *, const char*, const char *, int line);
#define DUM_IPV4(i) dum_ipv4(i, __FILE__, __FUNCTION__ ,__LINE__)
//0: error, 1: done
int reg_to_table(MSG_REGISTER *msg, int n, struct timespec *);
//0: error, 1: done
int hl_track_msg(MSG_TRACKING *msg, int n, struct sockaddr_in*, int type);

int add_to_item_list(MSG_NOTIFY *msg, HASH_ITEM **l, int sz);
#define  add_to_imd_fwd 		add_to_item_list
//sk: socket
//l: a linked list
//c: number sent
//clear: clear the list
int send_to_dst(int sk, HASH_ITEM **l, int *c, char clear);

#define send_imd_fwd			send_to_dst 

//2023-04-26
void put_time_to_msg( MSG_COMMON *, struct timespec *t);

extern HASH_LIST list_reg_dev[HASH_SIZE + 1];
extern HASH_LIST list_reg_notifier[HASH_SIZE + 1];

extern HASH_ITEM *notified_list;

//20230507
int send_msg_track(const char *iid, int sockfd, 
	char *ipaddr, int port, struct timespec *t, uchar *key, uchar *iv);
//20230517
int ntf_aes_encrypt(uchar *in, uchar *out, uchar* key, uchar* ivec, int n, int enc); 
int ntf_aes_file(uchar *in, uchar *out, uchar* key, uchar* ivec, int enc); 
int file_2_bytes(const uchar *path, uchar **output);
int file_2_pubrsa(const uchar *path, RSA **output);
int file_2_prvrsa(const uchar *path, RSA **output);
int rsa_dec(RSA *priv, const uchar *in, uchar **out, int lenin, int *outlen);
int rsa_enc(RSA *pubkey, const uchar *in, uchar **out, int lenin, int *outlen);

extern HASH_ITEM *imd_fwd_lt;

int load_reg_list();//Edit here
int put_pubkey_msg(MSG_DATA *, int *);//Edit here

//RSA *get_srv_pub();
RSA *get_srv_prv();

RSA *get_cli_pub(char *idd);
RSA *get_cli_prv(char *idd);





extern uchar aes_key[];
extern uchar *aes_iv;

int get_aes256_key(uchar **key, uchar **iv);
//Refer: https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
//https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Electronic_Codebook_(ECB)

int cmd_2_srv(CMD_ENUM cmd, MSG_ROUTE r, char *data, int len, char *idd, char *ip);
int ev_aes_enc(uchar *in, uchar **out, uchar *key, uchar *iv, int lenin, int *lenout, uchar *tag);
int ev_aes_dec(uchar *in, uchar **out, uchar *key, uchar *iv, int lenin, int *lenout, uchar *tag);
int msg_aes_enc(uchar *in, uchar *buf, uchar *key, uchar *iv, int lenin, int *lenout, int lim);
int msg_aes_dec(uchar *in, uchar *buf, uchar *key, uchar *iv, int lenin, int *lenout, int lim);
#ifndef __cplusplus
#endif
//https://www.youtube.com/watch?v=BOKSEaWfnjw
