#include <err.h>
#include <stdio.h>
#include <string.h>

#include <tee_client_api.h>
#include <my_ta.h> 


struct test_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

void prepare_tee_session(struct test_ctx *ctx){

	TEEC_Result res;
	TEEC_UUID uuid = MY_TA_UUID;
	uint32_t err_origin;

	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",res, err_origin);

}
void terminate_tee_session(struct test_ctx *ctx){
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

void get_secret(uint8_t* secret_get, uint32_t size,  struct test_ctx *ctx_sess){

	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(secret_get, 0,size);
	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = secret_get;
	op.params[0].tmpref.size = size;
	op.params[1].value.a = 1;

	res = TEEC_InvokeCommand(&ctx_sess->sess, CMD_SECRET_MANAGMENT_GET, &op, &err_origin);

}

void store_secret(uint8_t* secret, uint32_t size, struct test_ctx *ctx_sess){

	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(&op, 0, size);
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = secret;	
	op.params[0].tmpref.size = size;
	op.params[1].value.a = 1;

	res = TEEC_InvokeCommand(&ctx_sess->sess, CMD_SECRET_MANAGMENT_STR, &op, &err_origin);
}

void print_buffer(uint8_t* buffer, uint32_t size, const char* msg){
	printf("%s",msg);
	for(int i=0; i < size; i++){
		printf("%c",buffer[i]);
	}
	printf("\n");
}

int main(void)
{
	TEEC_UUID uuid = MY_TA_UUID;
	uint32_t err_origin;

	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;


	struct test_ctx ctx_sess1;
	
	ctx_sess1.ctx = ctx;
	ctx_sess1.sess = sess;

	prepare_tee_session(&ctx_sess1);

	// Test 0: Retrieve secret uninitialized (information disclosure)
	uint8_t secret_get[64] = {0};
	memset(secret_get, 0,sizeof(secret_get)); 

	// get_secret(secret_get, sizeof(secret_get) ,&ctx_sess1);  // Get secret using sess 1

	// print_buffer(secret_get, sizeof(secret_get), "Secret uninitizalized: ");

	// Test1 : store secret
	uint8_t secret[32] = {0};  
	memset(secret, 'B', sizeof(secret));
	
	print_buffer(secret, sizeof(secret), "Store secret: ");

	store_secret(secret, sizeof(secret), &ctx_sess1); // Store secret using sess 1

	// Test 2: Retrieve secret (information disclosure)
	//uint8_t secret_get[128] = {0};

	memset(secret_get, 0,sizeof(secret_get));

	get_secret(secret_get, sizeof(secret_get), &ctx_sess1);  // Get secret using sess 1

	print_buffer(secret_get, sizeof(secret_get), "Secret retrived: ");
	
	terminate_tee_session(&ctx_sess1);
	
	// Test 3: Retrieve secret using a new session (information disclosure)
	
	prepare_tee_session(&ctx_sess1);

	uint8_t secret_leak[32] = {0};
	memset(secret_leak, 0,sizeof(secret_leak));

	get_secret(secret_leak, sizeof(secret_leak) ,&ctx_sess1);  // Get secret using sess 2

	print_buffer(secret_leak, sizeof(secret_leak), "Leak secret from a previous session: ");

	//Test 4 Store secret again

	uint8_t secret_bigger[64] = {0};

	memset(secret_bigger, 'A', sizeof(secret_bigger));

	print_buffer(secret_bigger, sizeof(secret_bigger), "Store secret a secret bigger than the buffer: ");
	
	store_secret(secret_bigger, sizeof(secret_bigger), &ctx_sess1); // Store secret using sess 2

	// Test 5: Retrieve secret using sess 1 (information disclosure)

	memset(secret_leak, 0,sizeof(secret_leak));
	
	get_secret(secret_leak, sizeof(secret_leak), &ctx_sess1);  // Get secret using sess 1
	
	print_buffer(secret_leak, sizeof(secret_leak), "Leak secret after overflow: ");
	

	terminate_tee_session(&ctx_sess1);

	return 0;
}
