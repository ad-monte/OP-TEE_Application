#include <secret_manag_TA.h>
#include <my_ta.h> 

static uint8_t *secret      = NULL;
static uint32_t secret_size = 0;
static uint32_t access_id   = 0;

TEE_Result store_secret(uint32_t param_types,
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


TEE_Result retrieve_secret(uint32_t param_types,
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