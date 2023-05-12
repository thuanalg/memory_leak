#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/des.h>
#include <openssl/rand.h>


#define BUFSIZE 512 
//#define CBCM_ONE

/***************************************************************/
RSA* loadPUBLICKeyFromString( const char* publicKeyStr );
void get_pub_key(const char *, char **);
unsigned char* rsaEncrypt( RSA *pubKey, const unsigned char* str, int dataSize, int *resultLen );
RSA* loadPRIVATEKeyFromString( const char* privateKeyStr );
unsigned char* rsaDecrypt( RSA *privKey, const unsigned char* encryptedData, int *resultLen );
static void lcs_gen_des();
void try_with_sample();
/***************************************************************/
const char hello[] = "Hello, I am Nguyen Thai Thuan.";
/***************************************************************/
int main
(int argc, char* argv[]) 
{
    FILE * fp = 0; 
    char *pkey = 0;
    RSA *rsapkey = 0;
    RSA *rsaprikey = 0;
    char *ppubout = 0;
    char *priout = 0;
    int outn = 0;
	try_with_sample();
	return;
	lcs_gen_des();
	return 0;
    fp = fopen("test.txt", "a+");
    if(fp)
    {
        fprintf(fp, "%s\n", "Hello world! Nguyen Thai Thuan.");
        fclose(fp);
    }
    get_pub_key("public_key.pem", &pkey);
	fprintf(stdout, "---------:%d, outn: %d\n", __LINE__, strlen(hello));
    rsapkey = loadPUBLICKeyFromString(pkey);
    ppubout = rsaEncrypt(rsapkey, hello, strlen(hello), &outn);
	fprintf(stdout, "________:%d, outn: %d\n", __LINE__, outn);
    /***********************************************************/
    get_pub_key("private_key.pem", &pkey);
    rsaprikey = loadPRIVATEKeyFromString(pkey);
    rsaDecrypt(rsaprikey,ppubout, &outn);
	fprintf(stdout, "_________:%d, outn: %d\n",__LINE__, outn);
    return EXIT_SUCCESS;
}
/***************************************************************/
RSA* loadPUBLICKeyFromString( const char* publicKeyStr )
{
  BIO* bio = BIO_new_mem_buf( (void*)publicKeyStr, -1 ) ; // -1: assume string is null terminated
  
  BIO_set_flags( bio, BIO_FLAGS_BASE64_NO_NL ) ; // NO NL
  
  RSA* rsaPubKey = PEM_read_bio_RSA_PUBKEY( bio, NULL, NULL, NULL ) ;
  if( !rsaPubKey )
    printf( "ERROR: Could not load PUBLIC KEY!  PEM_read_bio_RSA_PUBKEY FAILED: %s\n", 
		ERR_error_string( ERR_get_error(), NULL ) ) ;
  
  BIO_free( bio ) ;
  return rsaPubKey ;
}
/***************************************************************/
void get_pub_key(const char *path, char **output)
{
    FILE *fp = 0;
    unsigned int sz = 0;
    fp = fopen(path, "r");
    if(!fp)
    {
        return;
    }
    fseek(fp, 0,SEEK_END);
    sz = ftell(fp);
    rewind(fp);
    (*output) = malloc(sz + 1);
    memset(*output, 0, sz + 1);
    fread(*output, 1, sz, fp);
    fclose(fp);
}
/***************************************************************/
unsigned char* rsaEncrypt( RSA *pubKey, const unsigned char* str, int dataSize, int *resultLen )
{
  int rsaLen = RSA_size( pubKey ) ;
  unsigned char* ed = (unsigned char*)calloc( 1, rsaLen ) ;
  
  //*resultLen = RSA_public_encrypt( dataSize, (const unsigned char*)str, ed, pubKey, PADDING ) ; 
  *resultLen = RSA_public_encrypt( dataSize, (const unsigned char*)str, ed, pubKey, RSA_PKCS1_PADDING ) ; 
  if( *resultLen == -1 )
    printf("ERROR: RSA_public_encrypt: %s\n", ERR_error_string(ERR_get_error(), NULL));
    //fprintf(stdout, "output: %s\n", ed);
    //fprintf(stdout, "output len: %d\n", *resultLen);
  return ed ;
}
/***************************************************************/
RSA* loadPRIVATEKeyFromString( const char* privateKeyStr )
{
  BIO *bio = BIO_new_mem_buf( (void*)privateKeyStr, -1 );
  RSA* rsaPrivKey = PEM_read_bio_RSAPrivateKey( bio, NULL, NULL, NULL ) ;
  
  if ( !rsaPrivKey )
    printf("ERROR: Could not load PRIVATE KEY!  PEM_read_bio_RSAPrivateKey FAILED: %s\n", 
		ERR_error_string(ERR_get_error(), NULL));
  
  BIO_free( bio ) ;
  return rsaPrivKey ;
}
/***************************************************************/
unsigned char* rsaDecrypt( RSA *privKey, const unsigned char* encryptedData, int *resultLen )
{
  int rsaLen = RSA_size( privKey ) ; // That's how many bytes the decrypted data would be
  
  unsigned char *decryptedBin = (unsigned char*)calloc( 1, rsaLen ) ;
  *resultLen = RSA_private_decrypt( RSA_size(privKey), encryptedData, decryptedBin, privKey, RSA_PKCS1_PADDING) ;
  if( *resultLen == -1 )
    printf( "ERROR: RSA_private_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;
    fprintf(stdout, "out decrypt: %s\n", decryptedBin); 
    fprintf(stdout, "out decrypt len: %d\n", *resultLen); 
  return decryptedBin ;
}
/***************************************************************/
void lcs_gen_des()
{
	unsigned char in[BUFSIZE], out[BUFSIZE], back[BUFSIZE];
    unsigned char *e = out;
    int len;

	fprintf(stdout, "sizeof(DES_cblock): %d\n", sizeof(DES_cblock));

    DES_cblock key1, key2, key3;
    DES_cblock seed = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    DES_cblock ivecstr = {0xE1, 0xE2, 0xE3, 0xD4, 0xD5, 0xC6, 0xC7, 0xA8};
    DES_cblock ivec2, ivec1;
    DES_key_schedule ks1, ks2, ks3;
 
    memset(in, 0, sizeof(in));
    memset(out, 0, sizeof(out));
    memset(back, 0, sizeof(back));
 
    RAND_seed(seed, sizeof(DES_cblock));
 
    DES_random_key(&key1);
    DES_random_key(&key2);
    DES_random_key(&key3);
 
    DES_set_key((C_Block *)key1, &ks1);
    DES_set_key((C_Block *)key2, &ks2);
    DES_set_key((C_Block *)key3, &ks3);

    /* 64 bytes of plaintext */
    strcpy(in, "Now is the time for all men to do.");
 
    printf("Plaintext: [%s]\n", in);
 
    memcpy(ivec2, ivecstr, sizeof(ivecstr));
    memset(ivec1,'\0',sizeof(ivec2));
    len = strlen(in) + 1;
 
#ifdef CBCM_ONE
    DES_ede3_cbcm_encrypt(in, out, len, &ks1, &ks2, &ks3, &ivec2, &ivec1, DES_ENCRYPT);
#else
    DES_ede3_cbcm_encrypt(in, out, 16, &ks1, &ks2, &ks3, &ivec2, &ivec1, DES_ENCRYPT);
    DES_ede3_cbcm_encrypt(&in[16], &out[16],len-16, &ks1, &ks2, &ks3, &ivec2, &ivec1, DES_ENCRYPT);
#endif
 
    printf("Ciphertext:");
    while (*e) printf(" [%02x]", *e++);
    printf("\n");
 
    len = strlen(out) + 1;
    memcpy(ivec2, ivecstr, sizeof(ivecstr));
    memset(ivec1,'\0',sizeof(ivec2));
 
    DES_ede3_cbcm_encrypt(out, back, len, &ks1, &ks2, &ks3, &ivec2, &ivec1, DES_DECRYPT);
 
    printf("Decrypted Text: [%s]\n", back);
	fprintf(stdout, "++++++in:%s\n++++back:%s\n", in, back);
	if(strlen(in) != strlen(back))
	{
		fprintf(stdout, "------------------error--------------------------\n");
	}
}

void try_with_sample()
{
	int err = 0;
	DES_cblock cb1 = { 0xBE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE };
	DES_cblock cb2 = { 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE };
	DES_cblock cb3 = { 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE, 0xAE };

	DES_key_schedule ks1,ks2,ks3;

	DES_cblock cblock = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	char string[] = "I am a software developer Nguyễn Thái Thuận.";
	int stringLen = 0;
	char cipher[65];
	char *text = 0;
	stringLen = sizeof(string);
	stringLen = strlen(string);
	DES_random_key(&cb1);
	DES_random_key(&cb2);
	DES_random_key(&cb3);
	printf("Plain Text : %s\n",string);
	printf("length : %d\n",stringLen);
	text = malloc(stringLen);
	memset(cipher,0, 65);
	memset(text,0,stringLen);

	DES_set_odd_parity(&cblock);

	if (DES_set_key_checked(&cb1, &ks1) ||
		DES_set_key_checked(&cb2, &ks2) ||
		DES_set_key_checked(&cb3, &ks3)) 
	{
		printf("Key error, exiting ....\n");
		return ;
	}
	if (DES_set_key_checked(&cb1, &ks1) ||
		DES_set_key_checked(&cb2, &ks2) ||
		DES_set_key_checked(&cb3, &ks3)) 
	{
		printf("Key error, exiting ....\n");
		return ;
	}

   DES_ede3_cbc_encrypt((const unsigned char*)string,
                         (unsigned char*)cipher,
                          stringLen, &ks1, &ks2, &ks3,
                                  &cblock, DES_ENCRYPT);
   //printf("Encrypted : %32.32s\n",cipher);
   printf("Encrypted : %s\n",cipher);
	printf("length : %d\n",strlen(cipher));
   memset(cblock,0,sizeof(DES_cblock));
   DES_set_odd_parity(&cblock);
   DES_ede3_cbc_encrypt((const unsigned char*)cipher,
                         (unsigned char*)text,
                          65, &ks1, &ks2, &ks3,
                                     &cblock,DES_DECRYPT);
   printf("Decrypted : %s\n",text);
}
//http://www.cplusplus.com/forum/unices/114504/
