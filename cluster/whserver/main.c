#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <lcs_common.h>
#include <lcs_rsa_des.h>
#include <pthread.h>
#include <sys/wait.h>
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD

#endif
/***********************************************************************************/

typedef struct LCS_WHSEVER_CFG
{
	char rootpath[1024];
	char dbpath[1024];
	char prefixshmkey[128];
	char privatekey[1024];
}
LCS_WHSEVER_CFG;

typedef struct LCS_WHS_REQUEST_DATA
{
	char dbserver[32];
	char dbname[64];
	char datefrom[32];
	char dateto[32];
	char analysis[32];
	char mode[16];
}
LCS_WHS_REQUEST_DATA;

/***********************************************************************************/
static void lcs_init_app();
static void lcs_init_socket(int *);
static void lcs_connected_socket(int, int *);
static void lcs_cfg_load(LCS_WHSEVER_CFG *, int *);
static void lcs_cfg_item(LCS_WHSEVER_CFG *, char *, int *);
static void lcs_whs_sendoff(int sock, int *err);
static void lcs_whs_transaction(int sock, int *err);
static void lcs_whs_3des(int sock, int *err);
static void lcs_whs_recv_request(int sock, int *err);
static void lcs_whs_send_data(int sock, int *err);
static void lcs_whs_send_file(int sock,char*, int *err);
static void lcs_whs_send_ready(int sock, int *err);
static void lcs_whs_gen_request(char* request, LCS_WHS_REQUEST_DATA *,int *err);
static void lcs_whs_get_path(char**,int *err);
static void lcs_whs_parent_wait(pid_t, int *err);
static void *lcs_whs_thread_wait(void*);
/***********************************************************************************/
static	char						*lcs_cfg_file 		= 0;
static  int							lcs_whs_socket 		= 0;
static	RSA							*lcs_private_key 	= 0;
static  DES_cblock					*lcs_3des_key 		= 0;

static  LCS_WHSEVER_CFG				lcs_info_cfg;
static LCS_WHS_REQUEST_DATA			lcs_request_data;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err = 0;
	do
	{
		if(argc < 2)
		{
			err = __LINE__;
			break;
		}
		lcs_cfg_file = argv[1];
		lcs_init_app();
		lcs_init_socket(&err);
		if(err)
		{
			break;
		}
	}
	while(0);
	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "lcs_whserver", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, LCS_RETURN_STATUS, "lcs_whserver", "EXIT_SUCCESS", err);
	return EXIT_SUCCESS;
}
/***********************************************************************************/
/* Load configuration from configured file
 * info		:	To store data
 * err		:	Error code
 */
/***********************************************************************************/
void lcs_cfg_load(LCS_WHSEVER_CFG *info, int *err)
{
	char *text = 0;
	char *pch = 0;
	lcs_file_text(lcs_cfg_file, &text, err);
	pch = strtok(text, " \r\n");
	while(pch)
	{
		lcs_cfg_item(info, pch, err);
		pch = strtok(0, " \r\n");
	}
	if(text)
	{
		free(text);
	}
}
/***********************************************************************************/
/* Store text into data structure
 * info		:	output, it is to store configured information
 * item		:	text, input
 * error	:	error code
 */
/***********************************************************************************/
void lcs_cfg_item(LCS_WHSEVER_CFG *info, char *item, int *err)
{
	char *base = 0;
	unsigned int sz = 0;
	do
	{
		base = "rootpath_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->rootpath, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dbpath_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbpath, &item[sz], strlen(item)-sz);
			break;
		}

		base = "prefixshmkey_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->prefixshmkey, &item[sz], strlen(item)-sz);
			break;
		}

		base = "privatekey_0004:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->privatekey, &item[sz], strlen(item)-sz);
			break;
		}
	}
	while(0);	
}
/***********************************************************************************/
/* Init information/variables for whole process
 *
 */
/***********************************************************************************/
void lcs_init_app()
{
	int err = 0;
	memset(&lcs_info_cfg, 0 , sizeof(LCS_WHSEVER_CFG));
	lcs_cfg_load(&lcs_info_cfg, &err);

}
/***********************************************************************************/
void lcs_init_socket(int *err)
{
	int sock = 0;
	socklen_t length = 0;
	pid_t process_id = 0;
	socklen_t optval = 0;
	struct sockaddr_in	server_address;
	struct sockaddr_in client_address;
	do
	{
		lcs_whs_socket = socket (AF_INET, SOCK_STREAM, 0);
		if(lcs_whs_socket < 0)
		{
			*err = __LINE__;
			break;
		}
		*err = setsockopt(lcs_whs_socket, SOL_SOCKET, SO_REUSEADDR, 
	     			(const void *)&optval , sizeof(socklen_t));
		if(*err)
		{
			*err = __LINE__;
			break;
		}
		memset(&server_address, 0,sizeof(server_address));
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = htonl (INADDR_ANY);
		server_address.sin_port = htons (LCS_WHS_PORT);

		*err = bind(lcs_whs_socket,(struct sockaddr *) 
				&server_address, sizeof(server_address));
		if(*err)
		{
			*err = __LINE__; break;
		}
		*err = listen(lcs_whs_socket, LCS_WHS_LIMIT);
		if(*err)
		{
			*err = __LINE__; break;
		}
		do
		{
			sock = accept(lcs_whs_socket, (struct sockaddr *)&client_address, &length);
			process_id = fork();
			if(process_id < 0)
			{
				*err = __LINE__; break;
			}
			if(process_id == 0) 
			{
				/*Child process*/	
				/* Close listening socket */
				close(lcs_whs_socket);
				/* Handling connected socket */
				lcs_connected_socket(sock, err);
				/* Shutdown connected socket */
				//shutdown(sock, 2);
				sleep(1);
				close(sock);
				break;
			}
			else
			{
				/*Parent process*/	
				/* Close connected socket */
				close(sock);
				lcs_whs_parent_wait(process_id, err);
			}
		} 
		while(1);
	}
	while(0);
}
/***********************************************************************************/
void lcs_connected_socket(int sock, int *err)
{
	socklen_t optval 			= 0;
	do
	{
		*err = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, 
				(const void *)&optval , sizeof(socklen_t));
		lcs_whs_3des(sock, err);
		if(*err)
		{
			break;
		}
		lcs_whs_transaction(sock, err);
	}
	while(0);
}
/**********************************************************************************/
void lcs_whs_transaction(int sock, int *err)
{
	do
	{
		lcs_whs_send_ready(sock, err);
		if(*err)
		{
			break;
		}
		lcs_whs_recv_request(sock, err);
		if(*err)
		{
			break;
		}
		lcs_whs_send_data(sock, err);
	}
	while(0);
	if(*err)
	{
		lcs_whs_sendoff(sock, err);
	}
}
/**********************************************************************************/
void lcs_whs_send_ready(int sock, int *err)
{
	unsigned char *msg = (unsigned  char *)LCS_WHS_READY;
	fprintf(stdout, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	lcs_3des_send(sock, lcs_3des_key, 
		LCS_RSA_3DES_OVER, msg, strlen((char *)msg) + 1, err);
}


/**********************************************************************************/
void lcs_whs_sendoff(int sock, int *err)
{
	char *msg = LCS_WHS_FINISHED;
	lcs_3des_send(sock, lcs_3des_key, 
		LCS_RSA_3DES_OFF, (LCSUCHAR *)msg, strlen(msg) + 1, err);
}

/**********************************************************************************/
void lcs_whs_3des(int sock, int *err)
{
	int n 						= 0;
	LCSUSHORT length 			= 0;
	unsigned char * decrypt 	= 0;
	char *privatekey			= 0;
	unsigned char buf[LCS_WHS_BUF + 1];

	privatekey = lcs_info_cfg.privatekey;
	do
	{
		memset(buf, 0, LCS_WHS_BUF + 1);
		lcs_get_private_key(privatekey, (void **) &lcs_private_key, err);
		if(lcs_private_key)
		{
			fprintf(stdout, "private keyi: 0x%p;\n", lcs_private_key);
		}
		else
		{
			fprintf(stdout, "Error load private key;\n");
			*err = __LINE__;
			break;
		}
		n = recv(sock, buf, LCS_WHS_BUF, 0);
		fprintf(stdout, "init receive: %d\n", n);
		decrypt = lcs_rsa_decrypt(lcs_private_key, buf, (LCSUSHORT *)&n);
		if(n < 1 )
		{
			fprintf(stdout, "%s:%d, n=%d\n", __FUNCTION__, __LINE__, n);
			*err = __LINE__;
			break;
		}
		if(decrypt[0] != LCS_RSA_3DES_ON)
		{
			fprintf(stdout, "%s:%d, n=%d\n", __FUNCTION__, __LINE__, n);
			*err = __LINE__;
			break;
		}
		memcpy(&length, decrypt + 1, sizeof(LCSUSHORT)); 
		if(length != (3 * sizeof(DES_cblock)))
		{
			fprintf(stdout, "%s:%d, n=%d\n", __FUNCTION__, __LINE__, n);
			*err = __LINE__;
			break;
		}
		lcs_3des_key = calloc(3, sizeof(DES_cblock));
		memcpy(lcs_3des_key, decrypt + 3, length);
		lcs_3des_dump( lcs_3des_key, err);
		lcs_3des_dump( lcs_3des_key + 1, err);
		lcs_3des_dump( lcs_3des_key + 2, err);
	}
	while(0);
	if(decrypt)
	{
		free(decrypt);
	}
}
/**********************************************************************************/
void lcs_whs_recv_request(int sock, int *err)
{
	char *data = 0;
	char *p = 0;
	int len = 0;
	LCSUCHAR mode = 0;
	fprintf(stdout, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	do
	{
		lcs_3des_recv(sock, lcs_3des_key, 
			&mode, (LCSUCHAR **)&data, &len, err);
		if(data)
		{
			fprintf(stdout, "request: %s\n", data);
			if(!strstr(data, LCS_WHS_REQUEST))
			{
				free(data);
				*err = __LINE__;
				break;
			}
			p = strstr(data, ":");
			++p;
			if(p)
			{
				lcs_whs_gen_request(p, &lcs_request_data, err);
				if(*err)
				{
					free(data);
					*err = __LINE__;
					break;
				}
			}
			fprintf(stdout, "p: %s\n", p);
			free(data);
		}
	}
	while(0);
}
/**********************************************************************************/
void lcs_whs_send_data(int sock, int *err)
{
	char *filepath = 0;
	lcs_whs_get_path(&filepath, err);
	if(filepath)
	{
		fprintf(stdout, "===========================filepath: %s\n", filepath);
		lcs_whs_send_file(sock, filepath, err);
		free(filepath);
	}
}
/**********************************************************************************/
void lcs_whs_gen_request
(char* request, LCS_WHS_REQUEST_DATA *p,int *err)
{
	char *t0 	= 0;
	char *t1 	= 0;
	int i 		= 0;
	char arr[32][128];

	memset(arr, 0, sizeof(arr));

	t0 = request;
	fprintf(stdout, "-----------++++++request: %s\n", request);
	do
	{
		if(!(*t0))
		{
			break;
		}
		if(*(t0 + 1) == 0)
		{
			break;
		}
		t1 = strstr(t0 + 1, "\t");
		if(!t1)
		{
			
			memcpy(arr[i], t0, strlen(t0));
			break;
		}
		memcpy(arr[i], t0, t1 - t0);
		t0 = t1 + 1;
		++i;
	}
	while(i<32);
	strcat(p->dbserver, arr[0]);
	strcat(p->dbname, arr[1]);
	strcat(p->datefrom, arr[2]);
	strcat(p->dateto, arr[3]);
	strcat(p->analysis, arr[4]);
	strcat(p->mode, arr[5]);
}
/**********************************************************************************/
void lcs_whs_send_file(int sock,char *filepath, int *err)
{
	FILE *fp = 0;
	LCSUSHORT n 					= 0;
	unsigned int k 					= 1;
	unsigned int m 					= 1;
	int len 						= 0;
	unsigned long long count 		= 0;
	unsigned long filesize			= 0;
	DES_cblock	*key 				= lcs_3des_key;

	DES_cblock cblock;
	DES_key_schedule ks1,ks2,ks3;
	unsigned char	cipher[LCS_RSA_3DES_BUF_HIH + 5];
	unsigned char	segment[LCS_RSA_3DES_BUF_LOW + 5];

	do
	{
		memset(cipher, 0, sizeof(cipher));
		memset(segment, 0, sizeof(segment));
		fp = fopen(filepath, "r");
		if(!fp)
		{
			break;
		}
		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		rewind(fp);
	
		sprintf((char*)segment, "%lu", filesize);	
	
		lcs_3des_send(sock, key, LCS_RSA_3DES_CHECKSUM, 
			segment, strlen((char *)segment) + 1, err);	

		memset(cblock, 0, sizeof(DES_cblock));
		DES_set_odd_parity(&cblock);
		if (DES_set_key_checked(&(key[0]), &ks1) ||
			DES_set_key_checked(&(key[1]), &ks2) ||
			DES_set_key_checked(&(key[2]), &ks3))
		{
			*err = __LINE__;
			break;
		}
		memset(cipher, 0, sizeof(cipher));
		memset(segment, 0, sizeof(segment));
		while( (len = fread(segment + 3, 1, LCS_RSA_3DES_BUF_LOW - 3, fp)))
		{
//
//			if(len < LCS_RSA_3DES_BUF_LOW - 3)
//			{
//				fprintf(stdout, "%s\n", segment +3);	
//				fprintf(stdout, "++++++++++++len: %d\n", len);	
//			}
			count = 0;
			//fprintf(stdout, "------------------len:%d\n", len);
			while(count < len)
			{
				k = 1;
				n = len - count;
				n = (n > LCS_RSA_3DES_BUF_LOW)?LCS_RSA_3DES_BUF_LOW:n;
				segment[0] = LCS_RSA_3DES_DATA;
				memcpy(segment + 1, &n, sizeof(LCSUSHORT));
				m = n + 3;
				//fprintf(stdout, "m: %d\n", m);
				while(k < m)
				{
					k <<= 1;
				}	
				k = (k<LCS_RSA_3DES_BLOCK_CIPHER)?LCS_RSA_3DES_BLOCK_CIPHER:k;
				//fprintf(stdout, "---------k: %d\n", k);
				memset(cblock, 0, sizeof(DES_cblock));
				DES_set_odd_parity(&cblock);
				DES_ede3_cbc_encrypt(segment, cipher,
					k, &ks1, &ks2, &ks3, &cblock, DES_ENCRYPT);
				n = send(sock, cipher, k, 0);
				//fprintf(stdout, "---------socket send: %d\n", n);
				count += LCS_RSA_3DES_BUF_LOW;
			}
			memset(cipher, 0, sizeof(cipher));
			memset(segment, 0, sizeof(segment));
			if(len < LCS_RSA_3DES_BUF_LOW - 3)
			{
				memset(cblock, 0, sizeof(DES_cblock));
				DES_set_odd_parity(&cblock);
				segment[0] = LCS_RSA_3DES_OFF;
				n = strlen("End session.") + 1;	
				memcpy(segment + 1, &n, sizeof(LCSUSHORT));
				strcat((char *)(segment + 3), "End session.");	
				DES_ede3_cbc_encrypt(segment, cipher,
					32, &ks1, &ks2, &ks3, &cblock, DES_ENCRYPT);
				n = send(sock, cipher, 32, 0);
				break;
			}
		}
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
}
/**********************************************************************************/
void lcs_whs_get_path(char **path,int *err)
{
	int i = 0;
	char datefrom[64];
	LCS_WHSEVER_CFG			*p 	= &lcs_info_cfg;
	LCS_WHS_REQUEST_DATA	*p1	= &lcs_request_data;

	memset(datefrom, 0, sizeof(datefrom));
	strcat(datefrom, p1->datefrom);

	do
	{
		if(!path)
		{
			*err = __LINE__;
			break;
		}
		while(datefrom[i])
		{
			if(datefrom[i] == '-' || datefrom[i] == ' ' || datefrom[i] == ':')
			{
				datefrom[i] = '/';
			}
			++i;
		}
		if(strcmp(p1->mode,"year") == 0)
		{
			datefrom[4] = 0;	
		}
		else if(strcmp(p1->mode,"month") == 0)
		{
			datefrom[7] = 0;	
		}
		else if(strcmp(p1->mode,"week") == 0)
		{
		}
		else if(strcmp(p1->mode,"day") == 0)
		{
			datefrom[10] = 0;	
		}
		else if(strcmp(p1->mode,"hour") == 0)
		{
			datefrom[13] = 0;
		}
		else if(strcmp(p1->mode,"minute") == 0)
		{
			datefrom[16] = 0;
		}
		*path = calloc(1, 1024);
		sprintf(*path, "%s/%s/%s/%s/%s.analysis",
			p->rootpath, p1->dbserver, p1->dbname, datefrom, p1->analysis);
	}
	while(0);
}
/**********************************************************************************/
void lcs_whs_parent_wait(pid_t id, int *err)
{
	pthread_t threadid = 0;
	pid_t *p = malloc(sizeof(pid_t));
	*p = id;
	pthread_create(&threadid, 0, lcs_whs_thread_wait, p);
}
/**********************************************************************************/
void *lcs_whs_thread_wait(void *arg)
{
	int status = 0;
	pid_t *p = arg;
	int result = 0;
	do
	{	
		result = waitpid(*p, &status, WNOHANG);
		if(result == -1)
		{
			//err
		}
		if(result == 0)
		{
			//running
		}
		sleep(1);
	} 
	while(result == 0);
	fprintf(stdout, "status: %d\n", status);
	free(p);
	return 0;
}
/**********************************************************************************/
