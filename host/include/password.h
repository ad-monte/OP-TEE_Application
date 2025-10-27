#ifndef PASSWORD_H
#define PASSWORD_H

#include <err.h>
#include <stdio.h>
#include <string.h>

#include <my_ta.h>
#include <tee_client_api.h>
#include <light_crypto_host.h>

#define VALID_PASSWORD 1
#define INVALID_PASSWORD 0

extern struct test_ctx;

int password_validation(char* password_input, struct test_ctx *ctx_sess);

#endif