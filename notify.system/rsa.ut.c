#include "msg_notify.h"
#include <string.h>

//int file_2_pubrsa(const uchar *path, RSA **output);
//int file_2_prvrsa(const uchar *path, RSA **output);
//int rsa_dec(RSA *priv, const uchar *in, uchar **out, int lenin, int *outlen);
//int rsa_enc(RSA *pubkey, const uchar *in, uchar **out, int lenin, int *outlen);

char text[1024];
 

int main(int argc, char *argv[]) {
	int err = 0;
	RSA *pubkey = 0;
	RSA *prvkey = 0;
	uchar *dec = 0;
	uchar *enc = 0;
	int n = 0;
	int k = 0;
	sprintf(text, "%s", argv[1]);
	do {
		err = file_2_pubrsa("srv-public-key.pem", &pubkey);
		if (err) {
    		fprintf(stdout, "ERROR: RSA_public_encrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;
			break;
		}

		err = file_2_prvrsa("srv-private-key.pem", &prvkey);
		if (err) {
    		fprintf(stdout, "ERROR: RSA_private_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;
			break;
		}
	
		fprintf(stdout, "size of public key: %d\n", RSA_size(pubkey));
		fprintf(stdout, "size of private key: %d\n", RSA_size(prvkey));

		err = rsa_enc(pubkey, text, &enc, strlen(text), &n);
		if (err) {
			fprintf(stdout, "enc error.\n");
			break;
		}
		fprintf(stdout, "inlen: %d, n: %d, \n ------------------- enc: %s\n", strlen(text), n, enc);

		err = rsa_dec(prvkey, enc, &dec, n, &k);
		if (err) {
			fprintf(stdout, "dec error.\n");
			break;
		}
		fprintf(stdout, "inlen: %d, outlen: %d, dec: %s, \n ------------------- enc: %s\n", n, k, dec);

	} while(0);

	if(pubkey) {
		RSA_free(pubkey);
	}

	if(prvkey) {
		RSA_free(prvkey);
	}

	if (dec) {
		MY_FREE(dec);
	}

	if (enc) {
		MY_FREE(enc);
	}

	return EXIT_SUCCESS;
}
