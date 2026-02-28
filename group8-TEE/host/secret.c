#include <secret.h>


void updateLog(uint8_t* log_msg, uint32_t size, struct test_ctx *ctx_sess){

	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = log_msg;
	op.params[0].tmpref.size = size;
	op.params[1].value.a = 1; // access id

	res = TEEC_InvokeCommand(&ctx_sess->sess, CMD_UPDATE_LOG, &op, &err_origin);

}

TEEC_Result getLog( uint8_t* log_msg, uint32_t size, uint8_t* log_time, uint32_t size_time, int32_t indexLog, struct test_ctx *ctx_sess){

	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
									 TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE);

	op.params[0].tmpref.buffer = log_msg;
	op.params[0].tmpref.size = size;
	op.params[1].value.a = indexLog; // access id

	op.params[2].tmpref.buffer = log_time;
	op.params[2].tmpref.size = size_time;

	res = TEEC_InvokeCommand(&ctx_sess->sess, CMD_GET_LOG, &op, &err_origin);
	return res;

}

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

void print_buffer(char* buffer, uint32_t size){
	for(int i=0; i < size; i++){
		printf("%c",buffer[i]);
	}
}
