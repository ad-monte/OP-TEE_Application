
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
        fprintf(stderr, "BAD ARGUMENTS: %s called with %d arguments when at least 2 are required.\n", argv[0], argc -1);
        return 1;
    }
	char *filename = argv[2]; //parse file name
	char *instruction = argv[1]; //parse instruction
	char *key;//parse key
	char *pwd;
	int success = 0;
	prepare_tee_session(&ctx_sess1);
	TEEC_Result res;
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
			char log_msg[100];
			char log_time[100];
			int32_t lower_index = atoi(argv[2]); // to be defined
			int32_t higher_index = atoi(argv[3]); // to be defined			
			if(password_validation(pwd, &ctx_sess1)){
				updateLog("Accessing log",strlen("Accessing log")+1, &ctx_sess1);
				for(int i=lower_index; i<=higher_index; i++){		
					memset(log_msg, 0, sizeof(log_msg));
					memset(log_time, 0, sizeof(log_time));			
					res=getLog(log_msg,sizeof(log_msg),log_time,sizeof(log_time),i,&ctx_sess1); // Parameters to be defined
					print_buffer("Log entry: ", sizeof("Log entry: "));
					print_buffer(log_msg, sizeof(log_msg));
					print_buffer(" - ", sizeof(" - "));
					print_buffer(log_time, sizeof(log_time));
					print_buffer("\n", sizeof("\n"));
					printf("%d - %d\n",res,TEEC_SUCCESS);

					if (res != TEEC_SUCCESS){
						fprintf(stderr, "Error getting log entry %d, code 0x%x\n", i, res);
						terminate_tee_session(&ctx_sess1);
						return 1;
					}
				}
				success = 1;
			}
			break;
		default:
			updateLog("BAD ARGUMENTS",strlen("BAD ARGUMENTS")+1, &ctx_sess1);
			fprintf(stderr,"BAD ARGUMENTS: %s called with an unrecognized instruction: %s\n", argv[0], argv[1]);
			terminate_tee_session(&ctx_sess1);
			return 1;
	}
	
	
	if(!success){
		updateLog("WRONG CREDENTIALS",strlen("WRONG CREDENTIALS")+1, &ctx_sess1);
		fprintf(stderr, "WRONG CREDENTIALS\n");
		terminate_tee_session(&ctx_sess1);
		return 1;
	}
	
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
