#ifndef SECRET_H
#define SECRET_H

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <my_ta.h>
#include <tee_client_api.h>
#include <light_crypto_host.h>

extern struct test_ctx;

void updateLog(uint8_t* log_msg, uint32_t size, struct test_ctx *ctx_sess);
TEEC_Result getLog( uint8_t* log_msg, uint32_t size, uint8_t* log_time, uint32_t size_time, int32_t indexLog, struct test_ctx *ctx_sess);
void get_secret(uint8_t* secret_get, uint32_t size,  struct test_ctx *ctx_sess);
void store_secret(uint8_t* secret, uint32_t size, struct test_ctx *ctx_sess);
void print_buffer(char* buffer, uint32_t size);


#endif