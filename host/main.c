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


void prepare_tee_session(struct test_ctx *ctx)
{
	TEEC_UUID uuid = MY_TA_UUID;
	uint32_t origin;
	TEEC_Result res;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/* Open a session with the TA */
	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, origin);
}

void terminate_tee_session(struct test_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

int main(int argc, char *argv[])
{
	//TEEC_Result res;
	//TEEC_Context ctx;
	//TEEC_Session sess;
	//TEEC_Operation op;
	//TEEC_UUID uuid = MY_TA_UUID;
	//uint32_t err_origin;

	/* Initialize a context connecting us to the TEE */
	/*res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);*/

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	/*res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);*/

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */

	/* Clear the TEEC_Operation struct */
	//memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	/*op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].value.a = 64;*/

	/*
	 * TA_HELLO_WORLD_CMD_INC_VALUE is the actual function in the TA to be
	 * called.
	 */


	/*printf("Invoking TA to increment %d\n", op.params[0].value.a);
	
	res = TEEC_InvokeCommand(&sess, MY_TA_CMD_INC_VALUE, &op, &err_origin);
	
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	printf("TA incremented value to %d\n", op.params[0].value.a);*/

	struct test_ctx ctx;
	char key[AES_TEST_KEY_SIZE];
	char iv[AES_BLOCK_SIZE];
	char clear[AES_TEST_BUFFER_SIZE];
	char ciph[AES_TEST_BUFFER_SIZE];
	char temp[AES_TEST_BUFFER_SIZE];

	
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


	
	printf("Prepare session with the TA\n");
	prepare_tee_session(&ctx);

	printf("Prepare encode operation\n");
	prepare_aes(&ctx, ENCODE);

	printf("Load key in TA\n");
	memset(key, 0xa5, sizeof(key)); /* Load some dummy value  for the KEY*/
	set_key(&ctx, key, AES_TEST_KEY_SIZE);

	printf("Reset ciphering operation in TA (provides the initial vector)\n");
	memset(iv, 0, sizeof(iv)); /* Load some dummy value as initial vector*/
	set_iv(&ctx, iv, AES_BLOCK_SIZE);

	printf("Encode buffer from TA\n");
	//memset(clear, 0x5a, sizeof(clear)); /* Load some dummy value */
	memset(ciph, 0, sizeof(ciph));
	//cipher_buffer(&ctx, clear, ciph, AES_TEST_BUFFER_SIZE); // this was before adding the possibility to passs the text file name
	cipher_buffer(&ctx, clear, ciph, clear_len);

	// write ciphertext to a file
    FILE *fout = fopen("ciphertext.bin", "wb");
    if (fout) {
        fwrite(ciph, 1, clear_len, fout);
        fclose(fout);
        printf("Ciphertext written to ciphertext.bin\n");
    }
	
	
	printf("Prepare decode operation\n");
	prepare_aes(&ctx, DECODE);

	printf("Load key in TA\n");
	memset(key, 0xa5, sizeof(key)); /* Load some dummy value */
	set_key(&ctx, key, AES_TEST_KEY_SIZE);

	printf("Reset ciphering operation in TA (provides the initial vector)\n");
	memset(iv, 0, sizeof(iv)); /* Load some dummy value */
	set_iv(&ctx, iv, AES_BLOCK_SIZE);

	printf("Decode buffer from TA\n");
	//cipher_buffer(&ctx, ciph, temp, AES_TEST_BUFFER_SIZE);
	cipher_buffer(&ctx, ciph, temp, clear_len);

	/* Check decoded is the clear content */
	if (memcmp(clear, temp, clear_len)) //AES_TEST_BUFFER_SIZE))
		printf("Plain text and decoded text differ => ERROR\n");
	else
		printf("Plain text and decoded text match\n");

	terminate_tee_session(&ctx);

	return 0;
}
