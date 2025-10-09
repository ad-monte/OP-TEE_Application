#include <err.h>
#include <stdio.h>
#include <string.h>

#include <tee_client_api.h>
#include <my_ta.h> 

int main(void)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = MY_TA_UUID;
	uint32_t err_origin;

	TEEC_Context ctx2;
	TEEC_Session sess2;
	TEEC_Operation op2;

	/* Initialize a context 1 connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	// Open a session 1 to the "hello world" TA, the TA will print "hello
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",res, err_origin);


	/* Initialize a context 2 connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx2);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	// Test 0: Retrieve secret uninitialized (information disclosure)
	uint8_t secret_get[32] = {0};
	memset(secret_get, 0,sizeof(secret_get));

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = secret_get;
	op.params[0].tmpref.size = sizeof(secret_get);
	op.params[1].value.a = 1;


	res = TEEC_InvokeCommand(&sess, CMD_SECRET_MANAGMENT_GET, &op, &err_origin);

	printf("Secret uninitizalized: ");
	for (int i=0; i < sizeof(secret_get); i++){
		printf("%c", secret_get[i]);
	}
	printf("\n");


	// Test1 : store secret
	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	uint8_t secret[32] = {0};  
	memset(secret, 'B', sizeof(secret));

	printf(" store secret: ");
	for(int i=0; i < sizeof(secret); i++){	printf("%c",secret[i]);}
	printf("\n");
	// uint_8_t secret[64] = "This is a very secret key that must be protected!";
	op.params[0].tmpref.buffer = secret;	
	op.params[0].tmpref.size = sizeof(secret);
	op.params[1].value.a = 1;

	res = TEEC_InvokeCommand(&sess, CMD_SECRET_MANAGMENT_STR, &op, &err_origin);
	//printf("Store secret result: 0x%x\n", res);

	// Test 2: Retrieve secret (information disclosure)
	//uint8_t secret_get[128] = {0};
	memset(secret_get, 0,sizeof(secret_get));

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = secret_get;
	op.params[0].tmpref.size = sizeof(secret_get);
	op.params[1].value.a = 1;


	res = TEEC_InvokeCommand(&sess, CMD_SECRET_MANAGMENT_GET, &op, &err_origin);

	printf(" Secret retrived :");
	for (int i=0; i < sizeof(secret_get); i++){
		printf("%c", secret_get[i]);
	}
	printf("\n");

	//TEEC_CloseSession(&sess);	

	// // Test3: leak secret using second session (improper access control)
	// //Open a session 2 to the "hello world" TA, the TA will print "hello
	// res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	// if (res != TEEC_SUCCESS)
	// 	errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",res, err_origin);

	// // secret managment tests
	// uint8_t secret_leak[128] = {0};
	// memset(secret_leak, 0,sizeof(secret_leak));

	// memset(&op2, 0, sizeof(op2));
	// op2.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
	// 				 TEEC_NONE, TEEC_NONE);

	// op2.params[0].tmpref.buffer = secret_leak;
	// op2.params[0].tmpref.size = sizeof(secret_leak);
	// op2.params[1].value.a = 1;

	// res = TEEC_InvokeCommand(&sess, CMD_SECRET_MANAGMENT_GET, &op2, &err_origin);
	// printf("Leak secret result:");
	// for (int i=0; i < sizeof(secret_leak); i++){
	// 	printf("%c", secret_leak[i]);
	// }
	// printf("\n");
	// ------------------------ //


	// Open a session 2 
	res = TEEC_OpenSession(&ctx, &sess2, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",res, err_origin);
	
	uint8_t secret_leak[32] = {0};
	memset(&op2, 0, sizeof(op2));
	op2.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op2.params[0].tmpref.buffer = secret_leak;
	op2.params[0].tmpref.size = sizeof(secret_leak);
	op2.params[1].value.a = 1;

	res = TEEC_InvokeCommand(&sess2, CMD_SECRET_MANAGMENT_GET, &op2, &err_origin);
	printf("Leak secret result sess2:");
	for (int i=0; i < sizeof(secret_leak); i++){
		printf("%c", secret_leak[i]);
	}
	printf("\n");
	/// ------------------------ //  
	//Test 4 Store secret again using sess 2
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	//uint8_t secret[32] = {0};  
	memset(secret, 'A', sizeof(secret));

	printf(" store secret ss2: ");
	for(int i=0; i < sizeof(secret); i++){	printf("%c",secret[i]);}
	printf("\n");
	// uint_8_t secret[64] = "This is a very secret key that must be protected!";
	op.params[0].tmpref.buffer = secret;	
	op.params[0].tmpref.size = sizeof(secret);
	op.params[1].value.a = 1;

	res = TEEC_InvokeCommand(&sess2, CMD_SECRET_MANAGMENT_STR, &op, &err_origin);

	// Test 5: Retrieve secret using sess 1 (information disclosure)

	memset(&op2, 0, sizeof(op2));
	op2.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op2.params[0].tmpref.buffer = secret_leak;
	op2.params[0].tmpref.size = sizeof(secret_leak);
	op2.params[1].value.a = 1;

	res = TEEC_InvokeCommand(&sess, CMD_SECRET_MANAGMENT_GET, &op2, &err_origin);
	printf("Leak secret result sess1:");
	for (int i=0; i < sizeof(secret_leak); i++){
		printf("%c", secret_leak[i]);
	}
	printf("\n");

	// Test 6: Trigger use after free (UAF)
	uint8_t secret_uaf[32] = {0};
	memset(secret_uaf, 0,sizeof(secret_uaf));

	memset(&op2, 0, sizeof(op2));
	op2.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);

	op2.params[0].tmpref.buffer = secret_uaf;
	op2.params[0].tmpref.size = sizeof(secret_uaf);

	res = TEEC_InvokeCommand(&sess, CMD_SECRET_ALLOCATE, &op2, &err_origin);	


	TEEC_CloseSession(&sess);
	TEEC_CloseSession(&sess2);
						
	TEEC_FinalizeContext(&ctx);
	TEEC_FinalizeContext(&ctx2);

	return 0;
}
