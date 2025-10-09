#include <err.h>
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <time.h>
#include <my_ta.h>

void check_result(TEEC_Result res, uint32_t err_origin, const char* msg) {
    if (res != TEEC_SUCCESS) {
        printf("Error: %s\n", msg);
        printf("TEEC_Result: 0x%x\n", res);
        printf("Error origin: 0x%x\n", err_origin);
        
        // Decode common error codes
        switch (res) {
            case TEEC_ERROR_BAD_PARAMETERS:
                printf("Bad parameters\n");
                break;
            case TEEC_ERROR_ACCESS_DENIED:
                printf("Access denied\n");
                break;
            case TEEC_ERROR_OUT_OF_MEMORY:
                printf("Out of memory\n");
                break;
            case TEEC_ERROR_ITEM_NOT_FOUND:
                printf("Item not found\n");
                break;
            default:
                printf("Unknown error\n");
        }
        exit(1);
    }
}

int main(void){
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = MY_TA_UUID;
	uint32_t err_origin;


	// open a context
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
	// open a session
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);

	// prepare the secret data
	uint8_t secret[32] = {0};
	memset(secret, 'A', sizeof(secret));

	printf("Secret str result:");
	for (int i=0; i < sizeof(secret); i++){
		printf("%c", secret[i]);
	}
	printf("\n");
	// initialize the operation struct
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);

	// set the parameters
	op.params[0].tmpref.buffer = secret;
	op.params[0].tmpref.size = sizeof(secret);
	op.params[1].value.a = 1; // access_id
	// invoke the command to store the secret
	res = TEEC_InvokeCommand(&sess, CMD_SECRET_MANAGMENT_STR, &op, &err_origin);
	// check the result
	check_result(res, err_origin, "Failed to retrieve secret");
	
	// Get secret	
	uint8_t secret_leak[32] = {0};
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
	op.params[0].tmpref.buffer = secret_leak;
	op.params[0].tmpref.size = sizeof(secret_leak);
	op.params[1].value.a = 1; // access_id
	res = TEEC_InvokeCommand(&sess, CMD_SECRET_MANAGMENT_GET, &op, &err_origin);
	check_result(res, err_origin, "Failed to retrieve secret");
	printf("secret get result:");
	for (int i=0; i < sizeof(secret_leak); i++){
		printf("%c", secret_leak[i]);
	}
	printf("\n");

	return 0;
}

