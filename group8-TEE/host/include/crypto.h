#ifndef CRYPTO_H
#define CRYPTO_H

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <tee_client_api.h>
#include <light_crypto_host.h>

#define AES_TEST_BUFFER_SIZE	4096
#define AES_TEST_KEY_SIZE	16
#define AES_BLOCK_SIZE		16

#define DECODE			0
#define ENCODE			1

struct crypto_params {
	char key[AES_TEST_KEY_SIZE];
	char iv[AES_BLOCK_SIZE];
};
extern struct test_ctx;

void encrypt_file(char* filename, struct test_ctx *ctx_sess, char *key);
void decrypt_file(struct test_ctx *ctx_sess1, char *key);

#endif