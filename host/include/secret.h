#ifndef PASSWORD_H
#define PASSWORD_H

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <my_ta.h>
#include <tee_client_api.h>
#include <light_crypto_host.h>

extern struct test_ctx;

void get_secret(uint8_t* secret_get, uint32_t size,  struct test_ctx *ctx_sess);
void store_secret(uint8_t* secret, uint32_t size, struct test_ctx *ctx_sess);
void print_buffer(uint8_t* buffer, uint32_t size, const char* msg);


#endif