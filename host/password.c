#include <password.h>

void password_validation(char* password_input, struct test_ctx *ctx_sess){

	TEEC_Operation op;
	TEEC_Result res;
	uint32_t validation; 
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_VALUE_OUTPUT,
					 TEEC_NONE,
					 TEEC_NONE);
	
	printf("Password to validate: %s\n", password_input);

	op.params[0].tmpref.buffer = password_input;
	op.params[0].tmpref.size = strlen(password_input) + 1;
	op.params[1].value.a = validation; // initialize output

	res = TEEC_InvokeCommand(&ctx_sess->sess, CMD_PASSWORD_VALIDATION, &op, &err_origin);
	
	// check result
	if (res != TEEC_SUCCESS) {
		printf("Password validation failed with code 0x%x origin 0x%x\n", res, err_origin);
	} 
	if(op.params[1].value.a == 1){
		printf("Password is valid\n");
	} else {
		printf("Password is invalid\n");
	}

}