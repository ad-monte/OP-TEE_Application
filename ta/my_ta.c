#include <inttypes.h> // to implement light cryptographic services AES
#include <light_crypto_TA.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>  // For memcpy()
#include <my_ta.h>
#include <string.h>  // For memcpy()

//AES constants

#define AES128_KEY_BIT_SIZE		128
#define AES128_KEY_BYTE_SIZE		(AES128_KEY_BIT_SIZE / 8)
#define AES256_KEY_BIT_SIZE		256
#define AES256_KEY_BYTE_SIZE		(AES256_KEY_BIT_SIZE / 8)

extern struct aes_cipher; //aes global

/*-------------secret managment variables--------------------*/ 

// VULN 2: Global pointer to secret data
static uint8_t *secret      = NULL;
static uint32_t secret_size = 0;
static uint32_t access_id   = 0;

/*-------------end secret managment variables -------------------*/ 

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

    return TEE_SUCCESS;
}
/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("TA_DestroyEntryPoint has been called");
}
/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(	TEE_PARAM_TYPE_NONE,
												TEE_PARAM_TYPE_NONE,
												TEE_PARAM_TYPE_NONE,
												TEE_PARAM_TYPE_NONE);

	struct aes_cipher *sess;

	DMSG("has been called");
	
	sess = TEE_Malloc(sizeof(*sess), 0);
	if (!sess)
		return TEE_ERROR_OUT_OF_MEMORY;

	sess->key_handle = TEE_HANDLE_NULL;
	sess->op_handle = TEE_HANDLE_NULL;

	*sess_ctx = (void *)sess;

	IMSG("Session %p: newly allocated", *sess_ctx);


	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;
	// The DMSG() macro is non-standard, TEE Internal API doesn't specify any means to logging from a TA.
	IMSG("Hello World Ragazzo!\n");
	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}


static char user[] = "Sebastian";
static char pass[] = "Alfonso";
//static char buf[MAX_SIZE];

static TEE_Result password_validation(uint32_t param_types,
			TEE_Param params[4])
		{
			uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
								   TEE_PARAM_TYPE_VALUE_OUTPUT,
								   TEE_PARAM_TYPE_NONE,
								   TEE_PARAM_TYPE_NONE);

			if (param_types != exp_param_types){
				IMSG("Parameter type mismatch!");
				return TEE_ERROR_BAD_PARAMETERS;
			}
			int8_t int_validity	=	0;
			int8_t str_validity	=	0;
			int8_t validated 	= params[1].value.a; // output param
			char* input_str 	= params[0].memref.buffer;
			uint32_t length    	= params[0].memref.size;


			IMSG("validate password is being called");
			// IMSG("Parameter type is: %u", param_types);
			// IMSG("Expected parameter type is: %u", exp_param_types);

			//validate str input from NW
			//validate int input from NW (length)

			IMSG("Got value: %s from NW 1", input_str);
			IMSG("Length received is: %u", (length));
			
			const char *pass_str = (const char *)pass;
	
			validated = (strcmp((const char*)input_str, pass_str)==0) ? 1 : 0;//Seems vulnerable to timing attacks
			printf("Password validation result: %d\n", validated);

			params[1].value.a = validated; // set output param

			return TEE_SUCCESS;
}
		



static TEE_Result store_secret(uint32_t param_types,
	TEE_Param params[4],uint32_t cmd_id) {

		IMSG("unsecure storing");
		
		uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
							   TEE_PARAM_TYPE_VALUE_INPUT,
							   TEE_PARAM_TYPE_NONE,
							   TEE_PARAM_TYPE_NONE);
		
		if (param_types != exp_param_types){
			IMSG("bad param types");
			return TEE_ERROR_BAD_PARAMETERS;
		}
			
		// Store the secret in global variables (ignore security issues): use secret and secret_size. 
		
		// Vulnerability: pointer to secret data in global variable is dangling after free
		if(secret){
			TEE_Free(secret);  								// free previous secret
			secret = NULL;
		}			
		secret = TEE_Malloc(params[0].memref.size, 0);		// allocate memory for new secret
		if(!secret){
			return TEE_ERROR_OUT_OF_MEMORY;					//	 check allocation
		}
		memcpy( secret , params[0].memref.buffer 
		, params[0].memref.size); 							// copy new secret

		secret_size = params[0].memref.size;
		access_id   = params[1].value.a;
	

		return TEE_SUCCESS;
	}

static TEE_Result retrieve_secret(uint32_t param_types,
		TEE_Param params[4],uint32_t cmd_id) {
	
			IMSG("unsecure retrieving");
			
			uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
								   TEE_PARAM_TYPE_VALUE_INPUT,
								   TEE_PARAM_TYPE_NONE,
								   TEE_PARAM_TYPE_NONE);
			
			if (param_types != exp_param_types){
				IMSG("bad param types");
				return TEE_ERROR_BAD_PARAMETERS;
			}	

			uint8_t *out_buffer = params[0].memref.buffer;
			uint32_t out_buffer_size = params[0].memref.size;
			uint32_t access_id = params[1].value.a;
	
			if(secret == NULL){
				IMSG("No secret stored");
				return TEE_ERROR_BAD_PARAMETERS;
			}
			memcpy( out_buffer , secret , out_buffer_size);
	
	
			//IMSG("g_secret_size: %d", g_secret_size);
			return TEE_SUCCESS;
		}





	
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	// We could have a command ID for each type of vulneravility.

	switch (cmd_id) {
	case CMD_SECRET_MANAGMENT_STR:
		return store_secret(param_types, params, cmd_id); //0
	case CMD_SECRET_MANAGMENT_GET:
		return retrieve_secret(param_types, params, cmd_id); //1 
	case TA_AES_CMD_PREPARE:
		return alloc_resources(sess_ctx, param_types, params); //2
	case TA_AES_CMD_SET_KEY:
		return set_aes_key(sess_ctx, param_types, params); //3
	case TA_AES_CMD_SET_IV:
		return reset_aes_iv(sess_ctx, param_types, params); //4
	case TA_AES_CMD_CIPHER:
		return cipher_buffer(sess_ctx, param_types, params); //5
	case CMD_PASSWORD_VALIDATION:
		return password_validation(param_types, params); //6

	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
