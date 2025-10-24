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

extern struct test_ctx;

encrypt_file(char* filename, struct test_ctx *ctx_sess);
decript_file(struct test_ctx *ctx_sess1);

#endif