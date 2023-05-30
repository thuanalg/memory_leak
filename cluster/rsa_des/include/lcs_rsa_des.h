#ifndef __LCS_RSA_DES_H__
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#ifdef __cplusplus
	extern "C" {
#endif
	 #ifdef LCS_RELEASE_MOD
	 #endif

	#ifndef	LCSUCHAR
		#define LCSUCHAR unsigned char
	#endif	

	#ifndef	LCSUSHORT
		#define LCSUSHORT unsigned short
	#endif	

	#define LCS_RSA_3DES_BLOCK_CIPHER		8
	#define LCS_RSA_3DES_BUF_HIH			1024
	#define LCS_RSA_3DES_BUF_LOW			512

	#define LCS_RSA_3DES_ON					0x00
	#define LCS_RSA_3DES_OFF				0xFF
	#define LCS_RSA_3DES_OVER				0xFE

	#define LCS_RSA_3DES_READY				0x01
	#define LCS_RSA_3DES_DATA				0x02
	#define LCS_RSA_3DES_DONE				0x03
	#define LCS_RSA_3DES_CHECKSUM			0x04

	#define LCS_WHS_BUF						2048
	#define	LCS_WHS_READY					"READY"
	#define	LCS_WHS_REQUEST					"REQUEST"
	#define	LCS_WHS_FINISHED				"FINISHED"

	void lcs_init_libcrypto(int *);
	void lcs_rsa_key(char **text, char *path, int*);
	void lcs_3des_gen(DES_cblock *, int*);
	void lcs_3des_dump(DES_cblock *, int*);
	void lcs_get_public_key(char *filepath, void **, int *err);
	void lcs_get_private_key(char *filepath, void **, int *err);
	LCSUCHAR  *lcs_rsa_encrypt(RSA *key, LCSUCHAR *data, LCSUSHORT len, LCSUSHORT *outlen);
	LCSUCHAR  *lcs_rsa_decrypt(RSA *key, LCSUCHAR *data, LCSUSHORT *outlen);
	void lcs_3des_encrypt(DES_cblock *key, LCSUCHAR *in, int sz, LCSUCHAR *out, int *outsz);
	void lcs_3des_decrypt(DES_cblock *key, LCSUCHAR *in, int sz, LCSUCHAR *out, int *outsz);
	void lcs_3des_send(int sock, DES_cblock *key, LCSUCHAR mode, LCSUCHAR *data, int len, int *);
	void lcs_3des_recv(int sock, DES_cblock *key, LCSUCHAR *mode, LCSUCHAR **data, int *len, int *);
#ifdef __cplusplus
}
#endif
#endif
