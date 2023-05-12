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
#include <arpa/inet.h>
#include <lcs_common.h>
#include <lcs_rsa_des.h>
/***********************************************************************************/

#ifdef LCS_RELEASE_MOD

#endif
#define LCS_ODBC_BUF	2048

/***********************************************************************************/

typedef struct LCS_WHSEVER_CFG
{
	char serverip[64];
	char serverport[12];
	char dbserver[64];
	char dbname[64];
	char dbuser[64];
	char dbpass[64];
	char datefrom[32];
	char dateto[32];
	char analysis[32];
	char mode[32];
	char filepath[512];
	char publickey[512];
}
LCS_WHSEVER_CFG;


/***********************************************************************************/
static void lcs_init_app();
static void lcs_init_socket(int *);
static void lcs_connected_socket(int, int *);
static void lcs_cfg_load(LCS_WHSEVER_CFG *, int *);
static void lcs_cfg_item(LCS_WHSEVER_CFG *, char *, int *);
static void lcs_odbc_3des(int sock, int *);
static void lcs_odbc_transaction(int sock, int *);
static void lcs_odbc_recv_ready(int sock, int *);
static void lcs_odbc_send_request(int sock, int *);
static void lcs_odbc_recv_data(int sock, int *);
static void lcs_odbc_recv_file(int sock, char *, int *);
/***********************************************************************************/
static	char 				*lcs_cfg_file 		= 0;
static  LCS_WHSEVER_CFG		lcs_info_cfg;
static	RSA					*lcs_public_key 	= 0;
static	DES_cblock			*lcs_3des_key 		= 0;
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err = 0;
	struct timeval t0;
	struct timeval t1;
	gettimeofday(&t0, 0);
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
	gettimeofday(&t1, 0);
	lcs_diff_time(&t1, &t0, (char *)__FILE__, (char *)__FUNCTION__, __LINE__);
	if(err)
	{
		fprintf(stdout, LCS_RETURN_STATUS, "lcs_odbc", "EXIT_FAILURE", err);
		return EXIT_FAILURE;
	}
	fprintf(stdout, LCS_RETURN_STATUS, "lcs_odbc", "EXIT_SUCCESS", err);
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
	int i 			= 0;
	int len 		= 0;
	FILE *fp 		= 0;
	char tmp[2048 + 1];
	do
	{
		fp = fopen(lcs_cfg_file, "r");
		if(!fp)
		{
			break;
		}
		memset(tmp, 0, sizeof(tmp));
		while(fgets(tmp, 2048, fp))
		{
			i = 0;
			len = strlen(tmp);
			while(tmp[len - i - 1]  == '\n' || tmp[len - i -1] == '\r')
			{
				i++;
			}
			tmp[len -i] = 0;
			lcs_cfg_item(info, tmp, err);
			memset(tmp, 0, sizeof(tmp));
		}
	}
	while(0);
	if(fp)
	{
		fclose(fp);
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
		base = "serverip_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->serverip, &item[sz], strlen(item)-sz);
			break;
		}

		base = "serverport_0001:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->serverport, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dbserver_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbserver, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dbname_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbname, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dbuser_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbuser, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dbpass_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbpass, &item[sz], strlen(item)-sz);
			break;
		}

		base = "datefrom_0003:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->datefrom, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dateto_0004:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dateto, &item[sz], strlen(item)-sz);
			break;
		}

		base = "analysis_0005:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->analysis, &item[sz], strlen(item)-sz);
			break;
		}

		base = "mode_0006:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->mode, &item[sz], strlen(item)-sz);
			break;
		}

		base = "filepath_0007:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->filepath, &item[sz], strlen(item)-sz);
			break;
		}

		base = "publickey_0008:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->publickey, &item[sz], strlen(item)-sz);
			fprintf(stdout, "pubkey: %s\n", info->publickey);
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
	//lcs_init_libcrypto(&err);
}
/***********************************************************************************/
void lcs_init_socket(int *err)
{
	int sock = 0;
    struct sockaddr_in dest;
	do
	{
		memset(&dest, 0, sizeof(dest));
		sock = socket (AF_INET, SOCK_STREAM, 0);
		if(sock < 0)
		{
			*err = __LINE__;
			break;
		}
		dest.sin_family = AF_INET;    /* select the desired network */
		dest.sin_port = htons(LCS_WHS_PORT);        /* select the port */
		//inet_aton(host, &dest.sin_addr);          /* remote address */
		dest.sin_addr.s_addr = inet_addr(lcs_info_cfg.serverip);
		*err = connect(sock, (struct sockaddr *)&dest, sizeof(dest));
		if(*err)
		{
			*err = __LINE__;
			break;
		}
		lcs_connected_socket(sock, err);	
	}
	while(0);
}
/***********************************************************************************/
void lcs_connected_socket(int sock, int *err)
{
	socklen_t optval 		= 0;

	do
	{
		*err = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, 
				(const void *)&optval , sizeof(socklen_t));
		lcs_odbc_3des(sock, err);
		if(*err)
		{
			break;
		}

		//memset(buf, 0, LCS_ODBC_BUF + 1);
		//n = recv(sock, buf, LCS_ODBC_BUF, 0);
		lcs_odbc_transaction(sock, err);
	}
	while(0);

	shutdown(sock, 2);
};

/**********************************************************************************/
void lcs_odbc_3des(int sock, int *err)
{
	char *pubkey			= 0;
	int n 					= 0;
	LCSUSHORT length 		= 0;
	unsigned char *encrypt 	= 0;
	unsigned char buf[32];
	do
	{
		pubkey = lcs_info_cfg.publickey;
		lcs_3des_key = calloc(3, sizeof(DES_cblock));
		lcs_3des_gen(lcs_3des_key, err);
		if(!lcs_3des_key)
		{
			*err = __LINE__;
			break;
		}
		//lcs_get_public_key("./public_key.pem", (void **)&lcs_public_key, err);
		lcs_get_public_key(pubkey, (void **)&lcs_public_key, err);
		if(!lcs_public_key)
		{
			*err = __LINE__;
			break;
		}
		memset(buf, 0, 32);
		buf[0] = 0;
		length = sizeof(DES_cblock) * 3;
		memcpy(buf + 1, &length, sizeof(unsigned short));
		memcpy(buf + 3, lcs_3des_key, length);
		length += 3;
		encrypt = lcs_rsa_encrypt(lcs_public_key, buf, length, (LCSUSHORT *)&length);		
		n = send(sock, encrypt, length, 0);
		if(n != length)
		{
			*err = __LINE__;
			break;
		}
	}
	while(0);
	if(lcs_public_key)
	{
		free(lcs_public_key);
	}
}
/**********************************************************************************/
void lcs_odbc_transaction(int sock, int *err)
{
	do
	{
		lcs_odbc_recv_ready(sock, err);
		if(*err)
		{
			break;
		}
		lcs_odbc_send_request(sock, err);
		if(*err)
		{
			break;
		}
		lcs_odbc_recv_data(sock, err);
	}
	while(0);
}
/**********************************************************************************/
void lcs_odbc_send_request(int sock, int *err)
{
	char msg[1024];
	LCS_WHSEVER_CFG *p = &lcs_info_cfg;

	fprintf(stdout, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "%s:%s\t%s\t%s\t%s\t%s\t%s",
			LCS_WHS_REQUEST, p->dbserver, 
			p->dbname, p->datefrom, p->dateto, p->analysis, p->mode);
	fprintf(stdout, "msg: %s\n", msg);	
	lcs_3des_send(sock, lcs_3des_key, 
		LCS_RSA_3DES_OVER, (LCSUCHAR *)msg, strlen(msg) + 1, err);
}
/**********************************************************************************/
void lcs_odbc_recv_ready(int sock, int *err)
{
	char *data = 0;
	LCSUCHAR mode = 0;
	int len = 0;
	fprintf(stdout, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	do
	{
		lcs_3des_recv(sock, lcs_3des_key, 
			&mode, (LCSUCHAR **)&data, &len, err);
		if(*err)
		{
			break;
		}
		if(data)
		{
			if(strcmp(data, LCS_WHS_READY))
			{
				free(data);
				*err = __LINE__;
				break;
			}
			free(data);
		}
	}
	while(0);
}
/**********************************************************************************/
void lcs_odbc_recv_data(int sock, int *err)
{
	char *path = lcs_info_cfg.filepath;
	lcs_odbc_recv_file(sock, path, err);
}
/**********************************************************************************/
void lcs_odbc_recv_file(int sock, char *path, int *err)
{
	int n 					= 0;
	unsigned int total 		= 0;
	FILE *fp 				= 0;
	unsigned char cmod 		= 0;
	LCSUSHORT length 		= 0;
	unsigned long checksum 	= 0;
	DES_cblock *key 		= lcs_3des_key;

	DES_cblock cblock;
	DES_key_schedule ks1,ks2,ks3;
	unsigned char	cipher[LCS_RSA_3DES_BUF_HIH + 5];
	unsigned char	segment[LCS_RSA_3DES_BUF_HIH + 5];

	fprintf(stdout, "function: %s\n", __FUNCTION__);

	do
	{
		fp = fopen(path, "w+");
		if(!fp)
		{
			break;
		}

		memset(cblock, 0, sizeof(DES_cblock));
		DES_set_odd_parity(&cblock);
		if (DES_set_key_checked(&(key[0]), &ks1) ||
			DES_set_key_checked(&(key[1]), &ks2) ||
			DES_set_key_checked(&(key[2]), &ks3))
		{
			*err = __LINE__;
			break;
		}
		do
		{

			memset(cipher, 0, sizeof(cipher));
			memset(segment, 0, sizeof(segment));
			n = recv(sock, cipher, LCS_RSA_3DES_BUF_LOW, 0);
			//fprintf(stdout, "n:------------------------%d\n", n);
			if(n < 1)
			{
				break;
			}
			memset(cblock, 0, sizeof(DES_cblock));
			DES_set_odd_parity(&cblock);
			DES_ede3_cbc_encrypt(cipher, segment, 
				LCS_RSA_3DES_BUF_HIH, &ks1, &ks2, &ks3, &cblock,DES_DECRYPT);
			cmod = segment[0];
			if(cmod == LCS_RSA_3DES_DATA)
			{
				memcpy(&length, segment + 1, sizeof(length));
				total += length;
				//fprintf(stdout, "total: %d\n", total);
				fwrite(segment + 3, 1 , length, fp);
				continue;
			}
			if(cmod == LCS_RSA_3DES_CHECKSUM)
			{
				sscanf((char*) (segment + 3), "%lu", &checksum);
				continue;
			}
			if(cmod == LCS_RSA_3DES_OVER)
			{
				fprintf(stdout, "line: %d, Transfer: DONE.\n", __LINE__);
				break;
			}
			if(cmod == LCS_RSA_3DES_OFF)
			{
				fprintf(stdout, "line: %d, Transfer: DONE.\n", __LINE__);
				break;
			}
		}
		while(1);
	}
	while(0);
	if(fp)
	{
		fclose(fp);
	}
	if(checksum != total)
	{
		*err = __LINE__;
	}
	fprintf(stdout, "checksum:total, %lu:%d\n", checksum, total);
}
/**********************************************************************************/
