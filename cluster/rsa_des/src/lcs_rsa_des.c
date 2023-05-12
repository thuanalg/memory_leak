#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <lcs_common.h>
#include <lcs_rsa_des.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <lcs_common.h>

//Read more: https://blog.fpmurphy.com/2010/04/openssl-des-api.html#ixzz4zcl8eJg6
/***********************************************************************************/
#define LCS_MODULO_SEED			0xFA
#define LCS_MODULO_IVECTOR		0xFF
/***********************************************************************************/
void lcs_rsa_key(char **pkey, char *file, int *err)
{
	
}
/***********************************************************************************/
void lcs_3des_gen(DES_cblock *arr3, int *err)
{
	int i = 0;
	for(i = 0; i < 3; ++i)	
	{
		DES_random_key(&(arr3[i]));
		lcs_3des_dump(&(arr3[i]), err);
	}
}
/***********************************************************************************/
void lcs_3des_dump(DES_cblock *key, int* err)
{
	int k = 0;
	fprintf(stdout, "\n");
	for(k = 0; k< sizeof(DES_cblock); k++)
	{
		fprintf(stdout, "%.2X ", (*key)[k]);
	}
	fprintf(stdout, "\n");
}
/***********************************************************************************/
void lcs_init_libcrypto(int *err)
{
	DES_cblock key[3];
	lcs_3des_gen(key, err);
}
/***********************************************************************************/
void lcs_get_public_key(char *filepath, void **key, int *err)
{
	BIO *bio 		= 0;
	char *txt 		= 0;

	do
	{
		lcs_file_text(filepath, &txt, err);
		if(!txt)
		{
			*err = __LINE__;
			break;
		}
		bio = BIO_new_mem_buf( (void*)txt, -1 ) ;
		if(!bio)
		{
			*err = __LINE__;
			break;
		}
		(*key) = (void *)PEM_read_bio_RSA_PUBKEY( bio, 0, 0, 0) ;
		if(!(*key))
		{
			*err = __LINE__;		
			break;
		}
	}
	while(0);
	if(txt)
	{
		free(txt);
	}	
	if(bio)
	{
		BIO_free(bio);
	}	
}
/***********************************************************************************/
void lcs_get_private_key(char *filepath, void **key, int *err)
{
	BIO *bio 		= 0;
	char *txt 		= 0;

	do
	{
		lcs_file_text(filepath, &txt, err);
		if(!txt)
		{
			*err = __LINE__;
			break;
		}
		bio = BIO_new_mem_buf( (void*)txt, -1 ) ;
		if(!bio)
		{
			*err = __LINE__;
			break;
		}
		(*key) = (void *)PEM_read_bio_RSAPrivateKey( bio, 0, 0, 0) ;
		if(!(*key))
		{
			*err = __LINE__;		
			break;
		}
	}
	while(0);
	if(txt)
	{
		free(txt);
	}	
	if(bio)
	{
		BIO_free(bio);
	}	

}
/***********************************************************************************/
LCSUCHAR  *lcs_rsa_encrypt
(RSA *key, LCSUCHAR *data, LCSUSHORT len, LCSUSHORT *outlen)
{
	int n = 0; 
	unsigned char* decrypt_data = 0;
	do
	{
		n = RSA_size(key ) ;
		decrypt_data = calloc(1, n) ;

		*outlen = RSA_public_encrypt( len, data, decrypt_data, key, RSA_PKCS1_PADDING ) ;
		if( *outlen == -1 )
		{
			fprintf(stdout, 
				"ERROR: RSA_public_encrypt: %s\n", 
					ERR_error_string(ERR_get_error(), 0));
		}
	}
	while(0);
	return decrypt_data;
/*
  int rsaLen = RSA_size( privKey ) ; // That's how many bytes the decrypted data would be
  
  unsigned char *decryptedBin = (unsigned char*)calloc( 1, rsaLen ) ;
  *resultLen = RSA_private_decrypt( RSA_size(privKey), encryptedData, decryptedBin, privKey, RSA_PKCS1_PADDING) ;
  if( *resultLen == -1 )
    printf( "ERROR: RSA_private_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;
    fprintf(stdout, "out decrypt: %s\n", decryptedBin); 
    fprintf(stdout, "out decrypt len: %d\n", *resultLen); 
  return decryptedBin ;
*/
}
/***********************************************************************************/
LCSUCHAR  *lcs_rsa_decrypt
(RSA *key, LCSUCHAR *data, LCSUSHORT *outlen)
{
	LCSUCHAR *decrypt_data = 0;

	decrypt_data = calloc(1, RSA_size(key));
	*outlen = RSA_private_decrypt(RSA_size(key), data, decrypt_data, key, RSA_PKCS1_PADDING);
	if(*outlen < 0)
	{
    	fprintf(stdout,  "ERROR: RSA_private_decrypt: %s\n", 
				ERR_error_string(ERR_get_error(), 0) ) ;
	}
	return decrypt_data;
}
/***********************************************************************************/
void lcs_3des_encrypt(DES_cblock *key, LCSUCHAR *in, int sz, LCSUCHAR *out, int *outsz)
{

}
/***********************************************************************************/
void lcs_3des_decrypt(DES_cblock *key, LCSUCHAR *in, int sz, LCSUCHAR *out, int *outsz)
{

}
/***********************************************************************************/
void lcs_3des_send
(int sock, DES_cblock *key, LCSUCHAR mode, LCSUCHAR *data, int len, int *err)
{
	LCSUSHORT n 					= 0;
	unsigned int k 					= 1;
	unsigned int m 					= 1;
	unsigned long long count 		= 0;

	DES_cblock cblock;
	DES_key_schedule ks1,ks2,ks3;
	unsigned char	cipher[LCS_RSA_3DES_BUF_HIH + 5];
	unsigned char	segment[LCS_RSA_3DES_BUF_HIH + 5];

	do	
	{
		memset(cblock, 0, sizeof(DES_cblock));
		DES_set_odd_parity(&cblock);
		if (DES_set_key_checked(&(key[0]), &ks1) ||
			DES_set_key_checked(&(key[1]), &ks2) ||
			DES_set_key_checked(&(key[2]), &ks3))
		{
			*err = __LINE__;
			break;
		}

		while(count < len)
		{
			k = 1;
			memset(cipher, 0, sizeof(cipher));
			memset(segment, 0, sizeof(segment));
			n = len - count;
			n = (n > LCS_RSA_3DES_BUF_LOW)?LCS_RSA_3DES_BUF_LOW:n;
			segment[0] = mode;
			memcpy(segment + 1, &n, sizeof(LCSUSHORT));
			memcpy(segment + 3, data + count, n);
			m = n + 3;
			while(k < m)
			{
				k <<= 1;
			}	
			k = (k<LCS_RSA_3DES_BLOCK_CIPHER)?LCS_RSA_3DES_BLOCK_CIPHER:k;
			DES_ede3_cbc_encrypt(segment, cipher,
				k, &ks1, &ks2, &ks3, &cblock, DES_ENCRYPT);
			fprintf(stdout, "strlen(cipher): %d\n",
				(int) strlen((const char *)cipher));
			fprintf(stdout, "(cipher): %s\n", (char *)(cipher));
			n = send(sock, (char *)cipher, k, 0);
			fprintf(stdout, "socket send: %d\n", n);
			count += LCS_RSA_3DES_BUF_LOW;
		}

	}
	while(0);
}
/***********************************************************************************/
void lcs_3des_recv
(int sock, DES_cblock *key, LCSUCHAR *mode, LCSUCHAR **data, int *len, int *err)
{
	DES_cblock cblock;
	DES_key_schedule ks1,ks2,ks3;
	unsigned char	cipher[LCS_RSA_3DES_BUF_LOW + 5];
	unsigned char	segment[LCS_RSA_3DES_BUF_HIH + 5];
	unsigned char cmod = 0;
	int n = 0;
	LCSUSHORT length = 0;
	fprintf(stdout, "function: %s\n", __FUNCTION__);
	do
	{

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
			fprintf(stdout, "-----n: %d\n", n);
			if(n < 1)
			{
				continue;
			}

			DES_ede3_cbc_encrypt(cipher, segment, 
				LCS_RSA_3DES_BUF_HIH, &ks1, &ks2, &ks3, &cblock,DES_DECRYPT);
			fprintf(stdout, "cipher: %s\n",(char *) cipher);
			cmod = segment[0];
			memcpy(&length, segment + 1, sizeof(length));
			fprintf(stdout, "segment: %s\n", (char*) (segment + 3));
			if(data)
			{
				*data = calloc(1, length + 1);
				sprintf((char *)(*data), "%s", (char *)(segment + 3));
			}
			if(mode)
			{
				*mode = cmod;
			}
			if(len)
			{
				*len = length;
			}
			if(cmod == LCS_RSA_3DES_OVER)
			{
				break;
			}
			if(cmod == LCS_RSA_3DES_OFF)
			{
				break;
			}
		}
		while(1);	
	}
	while(0);
}
/***********************************************************************************/
