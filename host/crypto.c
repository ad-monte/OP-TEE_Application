// encrypt file
#include <crypto.h>

//Global variables - vulnerability
char clear[AES_TEST_BUFFER_SIZE];
char ciph[AES_TEST_BUFFER_SIZE];
size_t clear_len;

encrypt_file(char* filename, struct test_ctx *ctx_sess1){

	char key[AES_TEST_KEY_SIZE];
	char iv[AES_BLOCK_SIZE];

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open input file");
        return 1;
    }

	// Read file into buffer
    clear_len = fread(clear, 1, AES_TEST_BUFFER_SIZE, fp);
    fclose(fp);

    if (clear_len == 0) {
        fprintf(stderr, "Input file is empty or read error\n");
        return 1;
    }


	printf("Prepare encode operation\n");
	prepare_aes(ctx_sess1, ENCODE);

	printf("Load key in TA\n");
	memset(key, 0xa5, sizeof(key)); /* Load some dummy value  for the KEY*/
	set_key(ctx_sess1, key, AES_TEST_KEY_SIZE);

	printf("Reset ciphering operation in TA (provides the initial vector)\n");
	memset(iv, 0, sizeof(iv)); /* Load some dummy value as initial vector*/
	set_iv(ctx_sess1, iv, AES_BLOCK_SIZE);

	printf("Encode buffer from TA\n");
	//memset(clear, 0x5a, sizeof(clear)); /* Load some dummy value */
	memset(ciph, 0, sizeof(ciph));
	//cipher_buffer(&ctx, clear, ciph, AES_TEST_BUFFER_SIZE); // this was before adding the possibility to passs the text file name
	cipher_buffer(ctx_sess1, clear, ciph, clear_len);


	// write ciphertext to a file
    FILE *fout = fopen("ciphertext.bin", "wb");
    if (fout) {
        fwrite(ciph, 1, clear_len, fout);
        fclose(fout);
        printf("Ciphertext written to ciphertext.bin\n");
    }
}

decript_file(struct test_ctx *ctx_sess1){

	char key[AES_TEST_KEY_SIZE];
	char iv[AES_BLOCK_SIZE];
	char temp[AES_TEST_BUFFER_SIZE];

    printf("Prepare decode operation\n");
	prepare_aes(ctx_sess1, DECODE);

	printf("Load key in TA\n");
	memset(key, 0xa5, sizeof(key)); /* Load some dummy value */
	set_key(ctx_sess1, key, AES_TEST_KEY_SIZE);

	printf("Reset ciphering operation in TA (provides the initial vector)\n");
	memset(iv, 0, sizeof(iv)); /* Load some dummy value */
	set_iv(ctx_sess1, iv, AES_BLOCK_SIZE);

	printf("Decode buffer from TA\n");
	//cipher_buffer(&ctx, ciph, temp, AES_TEST_BUFFER_SIZE);
	cipher_buffer(ctx_sess1, ciph, temp, clear_len);

    printf("Decoded text is: %s\n", temp);

	/* Check decoded is the clear content */
	if (memcmp(clear, temp, clear_len)) //AES_TEST_BUFFER_SIZE))
		printf("Plain text and decoded text differ => ERROR\n");
	else
		printf("Plain text and decoded text match\n");
}