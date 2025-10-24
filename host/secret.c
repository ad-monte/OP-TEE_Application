#include <secret.h>

void get_secret(uint8_t* secret_get, uint32_t size,  struct test_ctx *ctx_sess){

	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(secret_get, 0,size);//This seems like a security risk, assigning data to a memory region without knowing if 
								//the size is valid
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
