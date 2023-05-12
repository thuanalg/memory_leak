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
	char serverip[128];
	char dbname[128];
	char analysis[16];
	char datefrom[32];
	char dateto[32];
	char mode[32];
	char publickey[512];
	char filepath[1024];
}
LCS_WHSEVER_CFG;


/***********************************************************************************/
static void lcs_init_app();
static void lcs_init_socket(int *);
static void lcs_connected_socket(int, int *);
static void lcs_cfg_load(LCS_WHSEVER_CFG *, int *);
static void lcs_cfg_item(LCS_WHSEVER_CFG *, char *, int *);
static void lcs_odbc_rsa_send_3des(int sock, int *);
static void lcs_exec_transaction(int sock, DES_cblock *,int *);
static void	lcs_odbc_recv_ready(int sock, DES_cblock *,int *);
static void	lcs_odbc_send_request(int sock, DES_cblock *,int *);
static void	lcs_odbc_recv_checksum(int sock, DES_cblock *,int *);
static void	lcs_odbc_recv_data(int sock, DES_cblock *,int *);
/***********************************************************************************/
static  LCS_WHSEVER_CFG		lcs_info_cfg;
static	char				lcs_data_checksum[512];
static	char 				*lcs_cfg_file 		= 0;
static	RSA					*lcs_public_key 	= 0;
static	DES_cblock			*lcs_3des_key 		= 0;
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
	char text[2048];
	FILE *fp = 0;
	int i = 0;
	unsigned int sz = 0;
	//lcs_file_text(lcs_cfg_file, &text, err);
	//pch = strtok(text, " \r\n");
	do
	{
		fp = fopen(lcs_cfg_file, "r");
		if(!fp)
		{
			break;
		}
		while(1)
		{
			memset(text, 0, 2048);
			fgets(text, 2048 -1,fp);
			sz = strlen(text);
			if(sz < 2)
			{
				break;
			}
			i = 0;
			while(text[sz - i -1] == '\n' || text[sz - i -1] == '\r')
			{
				++i;
			}
			text[sz - i] = 0;
			//fprintf(stdout, "text: %s\n", text);
			lcs_cfg_item(info, text, err);
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

		base = "dbname_0002:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dbname, &item[sz], strlen(item)-sz);
			break;
		}

		base = "analysis_0003:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->analysis, &item[sz], strlen(item)-sz);
			break;
		}


		base = "datefrom_0004:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->datefrom, &item[sz], strlen(item)-sz);
			break;
		}

		base = "dateto_0005:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->dateto, &item[sz], strlen(item)-sz);
			break;
		}


		base = "mode_0006:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->mode, &item[sz], strlen(item)-sz);
			break;
		}

		base = "publickey_0007:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->publickey, &item[sz], strlen(item)-sz);
			break;
		}

		base = "filepath_0008:";
		if(strstr(item, base))
		{
			sz = strlen(base);
			memcpy(info->filepath, &item[sz], strlen(item)-sz);
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

	memset(&dest, 0, sizeof(dest));
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		*err = __LINE__;
		return;
	}
	dest.sin_family = AF_INET;    /* select the desired network */
	dest.sin_port = htons(LCS_WHS_PORT);        /* select the port */
	//inet_aton(host, &dest.sin_addr);          /* remote address */
	dest.sin_addr.s_addr = inet_addr(lcs_info_cfg.serverip);
	*err = connect(sock, (struct sockaddr *)&dest, sizeof(dest));
	if(*err)
	{
		*err = __LINE__;
		return;
	}
	lcs_connected_socket(sock, err);	
}
/***********************************************************************************/
void lcs_connected_socket(int sock, int *err)
{
	//int n 					= 0;
	socklen_t optval 		= 0;
	//unsigned char buf[LCS_ODBC_BUF + 1];
	int retval 				= 0;
	int error 				= 0;
	socklen_t len 			= 0;

	do
	{
		*err = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, 
				(const void *)&optval , sizeof(socklen_t));
		lcs_odbc_rsa_send_3des(sock, err);
		if(*err)
		{
			break;
		}
		/*
		//Get status socket
		len = sizeof (error);
		retval = getsockopt (sock, SOL_SOCKET, SO_ERROR, &error, &len);
		if(error)
		{
			fprintf(stderr, "socket error: %s\n", strerror(error));
			*err = __LINE__;
			break;
		}
	
		if(retval)
		{
			fprintf(stderr, "error getting socket error code: %s\n", 
				strerror(retval));
			*err = __LINE__;
			break;
		}
		lcs_exec_transaction(sock, lcs_3des_key, err);
		*/
		lcs_3des_recv(sock, lcs_3des_key, 0, 0, 0, err);
	}
	while(0);

	shutdown(sock, 2);
};

/**********************************************************************************/
void lcs_odbc_rsa_send_3des(int sock, int *err)
{
	int n 					= 0;
	LCSUSHORT length 		= 0;
	unsigned char *encrypt 	= 0;
	unsigned char buf[32];
	do
	{
		lcs_3des_key = calloc(3, sizeof(DES_cblock));
		lcs_3des_gen(lcs_3des_key, err);
		if(!lcs_3des_key)
		{
			*err = __LINE__;
			break;
		}
		lcs_get_public_key("./public_key.pem", (void **)&lcs_public_key, err);
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
		fprintf(stdout, "n: %d, length: %d\n", n, length);
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
void lcs_exec_transaction(int sock, DES_cblock* key3,int *err)
{
	do
	{
		lcs_odbc_recv_ready(sock, key3, err);
		if(*err)
		{
			break;
		}

		lcs_odbc_send_request(sock, key3, err);
		if(*err)
		{
			break;
		}

		lcs_odbc_recv_checksum(sock, key3, err);
		if(*err)
		{
			break;
		}

		lcs_odbc_recv_data(sock, key3, err);
		if(*err)
		{
			break;
		}

	}
	while(0);
}
/**********************************************************************************/
void lcs_odbc_recv_ready(int sock, DES_cblock *key3,int *err)
{
	int len 			= 0;
	LCSUCHAR mode 		= 0;
	LCSUCHAR *data 		= 0;

	fprintf(stdout , "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	do
	{
		lcs_3des_recv(sock, lcs_3des_key, &mode, &data, &len, err);
		if(mode != LCS_RSA_3DES_READY)
		{
			*err = __LINE__;	
			break;
		}
	}
	while(0);	
	if(data)
	{
		free(data);
	}
}
/**********************************************************************************/
void lcs_odbc_send_request(int sock, DES_cblock *key3,int *err)
{
	int len 			= 0;
	LCSUCHAR buf[1024];

	memset(buf, 0,1024);
	//mode|length///////server|dbname|dbname|ana|datefrom|dateto|mode
	//length = strlen("server|dbname|ana|datefrom|dateto|mode");
	//buf[0] = LCS_RSA_3DES_REQUEST;
	sprintf((char*)buf, "%s|%s|%s|%s|%s|%s", 
		lcs_info_cfg.serverip,
		lcs_info_cfg.dbname,
		lcs_info_cfg.analysis,
		lcs_info_cfg.datefrom,
		lcs_info_cfg.dateto,
		lcs_info_cfg.mode);
	len = (int)strlen((char*)buf);
	lcs_3des_send(sock, key3, 
		LCS_RSA_3DES_REQUEST, buf, len, err);
}
/**********************************************************************************/
void lcs_odbc_recv_checksum(int sock, DES_cblock *key3,int *err)
{	
	int len 			= 0;
	LCSUCHAR mode 		= 0;
	LCSUCHAR *data 		= 0;

	do
	{
		lcs_3des_recv(sock, lcs_3des_key, &mode, &data, &len, err);
		if(mode != LCS_RSA_3DES_CSUM)
		{
			*err = __LINE__;	
			break;
		}
	}
	while(0);	
	if(data)
	{
		memcpy(lcs_data_checksum, data, len);
		free(data);
	}
	else
	{
		*err = __LINE__;
	}
}
/**********************************************************************************/
void lcs_odbc_recv_data(int sock, DES_cblock *key3,int *err)
{
	int n 						= 0;
	LCSUSHORT length 			= 0;
	unsigned char mode 			= 0;
	DES_cblock cblock;
	DES_key_schedule ks1,ks2,ks3;
	LCSUCHAR cipher[LCS_RSA_3DES_BUF_HIH + 5];
	LCSUCHAR segment[LCS_RSA_3DES_BUF_HIH + 5];

	FILE *fp = 0;
	do
	{
		fp = fopen(lcs_info_cfg.filepath, "w+");
		if(!fp)
		{
			*err = __LINE__;
			break;
		}
		do
		{
			memset(cipher, 0, sizeof(cipher));
			memset(segment, 0, sizeof(segment));
			n = recv(sock, cipher, LCS_RSA_3DES_BUF_LOW, 0);
			if(n < 1)
			{
				continue;
			}

			DES_ede3_cbc_encrypt(cipher, segment, 
				LCS_RSA_3DES_BUF_HIH, &ks1, &ks2, &ks3, &cblock,DES_DECRYPT);
			fprintf(stdout, "cipher: %s\n",(char *) cipher);
			mode = segment[0];
			memcpy(&length, segment + 1, sizeof(length));
			fprintf(stdout, "segment: %s\n", (char*) (segment + 3));
			if(mode == LCS_RSA_3DES_DATA)
			{
				fprintf(fp, "%s", (char *)(segment + 3))	;
			}
			else if(mode == LCS_RSA_3DES_DONE)
			{
				break;
			}
			else
			{
				*err = __LINE__;
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
}
/**********************************************************************************/
