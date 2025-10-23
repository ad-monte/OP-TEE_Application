#include <err.h>
#include <stdio.h>
#include <string.h>

#include <light_crypto_host.h>

#include <tee_client_api.h>
#include <my_ta.h> 



#define AES_TEST_BUFFER_SIZE	4096
#define AES_TEST_KEY_SIZE	16
#define AES_BLOCK_SIZE		16

#define DECODE			0
#define ENCODE			1



extern struct test_ctx;  // Global shared state defined in light cripto host.h
/*
struct test_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};*/

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

void password_validation(uint8_t* password_input, struct test_ctx *ctx_sess){

	TEEC_Operation op;
	TEEC_Result res;
	uint32_t validation; 
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_VALUE_OUTPUT,
					 TEEC_NONE,
					 TEEC_NONE);
	
	printf("Password to validate: %s\n", (const char*)password_input);

	op.params[0].tmpref.buffer = password_input;
	op.params[0].tmpref.size = strlen((char*)password_input) + 1;
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

int main(int argc, char *argv[])
{

	struct test_ctx ctx_sess1;
	

	char key[AES_TEST_KEY_SIZE];
	char iv[AES_BLOCK_SIZE];
	char clear[AES_TEST_BUFFER_SIZE];
	char ciph[AES_TEST_BUFFER_SIZE];
	char temp[AES_TEST_BUFFER_SIZE];

	//parse argument into buffer

	printf("This is the TA host application\n");

	if (argc < 2) {		//take plain text param
        fprintf(stderr, "Usage: %s <plaintext_file>\n", argv[0]);
        return 1;
    }
	const char *filename = argv[1]; //parse file name
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open input file");
        return 1;
    }

	// Read file into buffer
    size_t clear_len = fread(clear, 1, AES_TEST_BUFFER_SIZE, fp);
    fclose(fp);

    if (clear_len == 0) {
        fprintf(stderr, "Input file is empty or read error\n");
        return 1;
    }




	prepare_tee_session(&ctx_sess1);
	// Test 0: Retrieve secret uninitialized (information disclosure)
	uint8_t secret_get[64] = {0};
	memset(secret_get, 0,sizeof(secret_get)); 

	get_secret(secret_get, sizeof(secret_get) ,&ctx_sess1);  // Get secret using sess 1

	print_buffer(secret_get, sizeof(secret_get), "Secret uninitizalized: ");

	// Test1 : store secret
	uint8_t secret[32] = {0};  
	memset(secret, 'B', sizeof(secret));
	
	print_buffer(secret, sizeof(secret), "Store secret: ");

	store_secret(secret, sizeof(secret), &ctx_sess1); // Store secret using sess 1

	terminate_tee_session(&ctx_sess1);
	prepare_tee_session(&ctx_sess1);

	// Test 2: Retrieve secret initialized
	memset(secret_get, 0,sizeof(secret_get));

	get_secret(secret_get, sizeof(secret_get) ,&ctx_sess1);  // Get secret using sess 1

	print_buffer(secret_get, sizeof(secret_get), "Secret next session: ");

	// password validation test
	printf("Password validation test\n");

	uint8_t* password_input = "password"; // wrong password

	password_validation(password_input, &ctx_sess1);	

	password_input = "Alfonso"; // wrong password

	password_validation(password_input, &ctx_sess1);	

	//encrypt file

	printf("Prepare encode operation\n");
	prepare_aes(&ctx_sess1, ENCODE);

	printf("Load key in TA\n");
	memset(key, 0xa5, sizeof(key)); /* Load some dummy value  for the KEY*/
	set_key(&ctx_sess1, key, AES_TEST_KEY_SIZE);

	printf("Reset ciphering operation in TA (provides the initial vector)\n");
	memset(iv, 0, sizeof(iv)); /* Load some dummy value as initial vector*/
	set_iv(&ctx_sess1, iv, AES_BLOCK_SIZE);

	printf("Encode buffer from TA\n");
	//memset(clear, 0x5a, sizeof(clear)); /* Load some dummy value */
	memset(ciph, 0, sizeof(ciph));
	//cipher_buffer(&ctx, clear, ciph, AES_TEST_BUFFER_SIZE); // this was before adding the possibility to passs the text file name
	cipher_buffer(&ctx_sess1, clear, ciph, clear_len);


	// write ciphertext to a file
    FILE *fout = fopen("ciphertext.bin", "wb");
    if (fout) {
        fwrite(ciph, 1, clear_len, fout);
        fclose(fout);
        printf("Ciphertext written to ciphertext.bin\n");
    }
	
	//decode ciphered file

	printf("Prepare decode operation\n");
	prepare_aes(&ctx_sess1, DECODE);

	printf("Load key in TA\n");
	memset(key, 0xa5, sizeof(key)); /* Load some dummy value */
	set_key(&ctx_sess1, key, AES_TEST_KEY_SIZE);

	printf("Reset ciphering operation in TA (provides the initial vector)\n");
	memset(iv, 0, sizeof(iv)); /* Load some dummy value */
	set_iv(&ctx_sess1, iv, AES_BLOCK_SIZE);

	printf("Decode buffer from TA\n");
	//cipher_buffer(&ctx, ciph, temp, AES_TEST_BUFFER_SIZE);
	cipher_buffer(&ctx_sess1, ciph, temp, clear_len);

	/* Check decoded is the clear content */
	if (memcmp(clear, temp, clear_len)) //AES_TEST_BUFFER_SIZE))
		printf("Plain text and decoded text differ => ERROR\n");
	else
		printf("Plain text and decoded text match\n");

	terminate_tee_session(&ctx_sess1);

	return 0;
}
