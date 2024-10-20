#include "msg_notify.h"
#include <string.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <openssl/ossl_typ.h>



//int file_2_pubrsa(const uchar *path, RSA **output);
//int file_2_prvrsa(const uchar *path, RSA **output);
//int rsa_dec(RSA *priv, const uchar *in, uchar **out, int lenin, int *outlen);
//int rsa_enc(RSA *pubkey, const uchar *in, uchar **out, int lenin, int *outlen);

//https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
int gcm_ev_enc(unsigned char *plaintext, int plaintext_len,
                unsigned char *aad, int aad_len,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *ciphertext,
                unsigned char *tag);
int gcm_ev_dec(unsigned char *ciphertext, int ciphertext_len,
                unsigned char *aad, int aad_len,
                unsigned char *tag,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *plaintext);
const char *text = "nguyen thai thuan algebra, apple, lllllllllllllll";
void handleErrors() {

}

int main(int argc, char *argv[]) {
	int err = 0;
	char ciphertext[1024];
	char buffer[1024];
	char decbuf[1024];
	char tag[1024];
	int n = 0;
	memset(ciphertext, 0, sizeof(ciphertext));
	memset(buffer, 0, sizeof(buffer));
	memset(tag, 0, sizeof(tag));
	sprintf(buffer, "%s", argv[1]);
	
	n = gcm_ev_enc(buffer, strlen(buffer), 
		aes_iv, 16, 
		aes_key, 
		aes_iv, 16, 
		ciphertext, 
		tag);
	fprintf(stdout, "----%s---\n", ciphertext);
	fprintf(stdout, "----n= %d---\n", n);
	fprintf(stdout, "----tag= %s---, taglen: %d\n", tag, strlen(tag));
	//memset(tag, 0, sizeof(tag));
	gcm_ev_dec(ciphertext, n,
                aes_iv, 16,
                tag,
                aes_key,
                aes_iv, 16,
                decbuf);
	fprintf(stdout, "----%s---\n", decbuf);
	
	return EXIT_SUCCESS;
}
//https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
int gcm_ev_enc(unsigned char *plaintext, int plaintext_len,
                unsigned char *aad, int aad_len,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *ciphertext,
                unsigned char *tag)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;


    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /* Initialise the encryption operation. */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();

    /*
     * Set IV length if default 12 bytes (96 bits) is not appropriate
     */
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL))
        handleErrors();

    /* Initialise key and IV */
    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv))
        handleErrors();

    /*
     * Provide any AAD data. This can be called zero or more times as
     * required
     */
    if(1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
        handleErrors();

	fprintf(stdout, "len: %d\n", len);
    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;
	fprintf(stdout, "ciphertext: %d\n", ciphertext_len);
    /*
     * Finalise the encryption. Normally ciphertext bytes may be written at
     * this stage, but this does not occur in GCM mode
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

	fprintf(stdout, "len: %d\n", len);
//    /* Get the tag */
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
        handleErrors();

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int gcm_ev_dec(unsigned char *ciphertext, int ciphertext_len,
                unsigned char *aad, int aad_len,
                unsigned char *tag,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    int ret;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /* Initialise the decryption operation. */
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL))
        handleErrors();

    /* Initialise key and IV */
    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
        handleErrors();

    /*
     * Provide any AAD data. This can be called zero or more times as
     * required
     */
    if(!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
        handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
  if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
      handleErrors();

    /*
     * Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    if(ret > 0) {
        /* Success */
        plaintext_len += len;
		fprintf(stdout, "plaintext_len: %d\n", plaintext_len);
        return plaintext_len;
    } else {
        /* Verify failed */
		fprintf(stdout, "plaintoooooooooext_len: %d\n", plaintext_len);

    		fprintf (stdout, "aes: %s\n", 
			ERR_error_string( ERR_get_error(), NULL ) ) ;
        return -1;
    }
}

