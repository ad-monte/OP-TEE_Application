
#include <main_header.h>

extern struct test_ctx;  // Global shared state defined in light cripto host.h

void prepare_tee_session(struct test_ctx *ctx);
void terminate_tee_session(struct test_ctx *ctx);


int main(int argc, char *argv[])
{

	struct test_ctx ctx_sess1;

	
	//parse argument into buffer

	printf("This is the TA host application, my test\n");

	if (argc < 3) {		//take plain text param
        fprintf(stderr, "BAD ARGUMENTS: %s called with %d arguments when at least 2 are required.", argv[0], argc -1);
        return 1;
    }
	char *filename = argv[2]; //parse file name
	char *instruction = argv[1]; //parse instruction
	char *key;//parse key
	char *pwd;
	int success = 0;
	prepare_tee_session(&ctx_sess1);
	
	switch (instruction[1]) {
		case 'e':
			pwd = argv[4];//parse password
			key = argv[3];//parse key
			if(password_validation(pwd, &ctx_sess1)){
				updateLog("Encrypting",strlen("Encrypting")+1, &ctx_sess1);
				encrypt_file(filename, &ctx_sess1, key);
				success = 1;
			}
			break;
		case 'd':
			pwd = argv[3];//parse password
			key = argv[2];//parse key
			if(password_validation(pwd, &ctx_sess1)){
				updateLog("Decrypting",strlen("Decrypting")+1, &ctx_sess1);
				decrypt_file(&ctx_sess1, key);
				success = 1;
			}
			break;
		case 'a':
			pwd = argv[4];//parse password
			uint8_t log_msg[64];
			uint8_t log_time[32];
			int32_t lower_index = atoi(argv[2]); // to be defined
			int32_t higher_index = atoi(argv[3]); // to be defined			
			if(password_validation(pwd, &ctx_sess1)){
				updateLog("Accessing log",strlen("Accessing log")+1, &ctx_sess1);
				for(int i=lower_index; i<=higher_index; i++){		
					memset(log_msg, 0, sizeof(log_msg));
					memset(log_time, 0, sizeof(log_time));			
					getLog(log_msg,sizeof(log_msg),log_time,sizeof(log_time),i,&ctx_sess1); // Parameters to be defined
					printf("%s - %s\n",log_msg, log_time);
				}
				success = 1;
			}
			break;
		default:
			updateLog("BAD ARGUMENTS",strlen("BAD ARGUMENTS")+1, &ctx_sess1);
			fprintf(stderr,"BAD ARGUMENTS: %s called with an unrecognized instrution: %s", argv[0], argv[1]);
			return 1;
	}
	
	
	if(!success){
		updateLog("WRONG CREDENTIALS",strlen("WRONG CREDENTIALS")+1, &ctx_sess1);
		fprintf(stderr, "WRONG CREDENTIALS");
	}
	
	// // Test 0: Retrieve secret uninitialized (information disclosure)
	// uint8_t secret_get[64] = {0};
	// memset(secret_get, 0,sizeof(secret_get)); 

	// get_secret(secret_get, sizeof(secret_get) ,&ctx_sess1);  // Get secret using sess 1

	// print_buffer(secret_get, sizeof(secret_get), "Secret uninitizalized: ");

	// // Test1 : store secret
	// uint8_t secret[32] = {0};  
	// memset(secret, 'B', sizeof(secret));
	
	// // print_buffer(secret, sizeof(secret), "Store secret: ");
	// printf("Store secret test %s\n", (char *)secret);

	// store_secret(secret, sizeof(secret), &ctx_sess1); // Store secret using sess 1

	// terminate_tee_session(&ctx_sess1);
	// prepare_tee_session(&ctx_sess1);

	// // Test 2: Retrieve secret initialized
	// memset(secret_get, 0,sizeof(secret_get));

	// get_secret(secret_get, sizeof(secret_get) ,&ctx_sess1);  // Get secret using sess 1

	// print_buffer(secret_get, sizeof(secret_get), "Secret next session: ");

	// // Test 3: validation test
	
	// printf("Password validation test\n");

	// char* password_input = "password"; // wrong password

	// password_validation(password_input, &ctx_sess1);	

	// char* password_input2 = argv[2];//"Alfonso"; // wrong password

	// password_validation(password_input2, &ctx_sess1);	

	// //Test 4: Encrypt file
	// encrypt_file(filename, &ctx_sess1);
	// //Test 5: Decode file
	// decript_file(&ctx_sess1);
	
	//Terminate TEE session
	terminate_tee_session(&ctx_sess1);

	return 0;
}


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
