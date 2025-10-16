#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <my_ta.h>
#include <string.h>  // For memcpy()




// VULN 1: Global shared state
static uint8_t g_secret[32];
static uint32_t g_secret_size;
static bool g_initialized = false;

// VULN 2: Global pointer to secret data
static uint8_t *g_secret_buffer = NULL;
static uint32_t g_secret_size = 0;

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
	DMSG("has been called");
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

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World Ragazzo!\n");
	IMSG("Value exp_param_types %d }",exp_param_types);

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


static TEE_Result store_secret(uint32_t param_types,
	TEE_Param params[4],uint32_t cmd_id) {

		IMSG("unsecure storing");
		
		uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
							   TEE_PARAM_TYPE_VALUE_INPUT,
							   TEE_PARAM_TYPE_NONE,
							   TEE_PARAM_TYPE_NONE);
		
		if (param_types != exp_param_types)
			return TEE_ERROR_BAD_PARAMETERS;
		//vulnerability: unchecked parameter bounds
		uint8_t *secret      = params[0].memref.buffer;
		uint32_t secret_size = params[0].memref.size;
		uint32_t access_id   = params[1].value.a;
		// vulnerability: possible overflow
		memcpy(g_secret, secret, secret_size);
		
		g_secret_buffer = secret;	
		g_secret_size = secret_size;
		
		IMSG("g_secret_size: %d", g_secret_size);
		return TEE_SUCCESS;
	}


	static TEE_Result retrieve_secret(uint32_t param_types,
		TEE_Param params[4],uint32_t cmd_id) {
	
			IMSG("unsecure storing");
			
			uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
								   TEE_PARAM_TYPE_VALUE_INPUT,
								   TEE_PARAM_TYPE_NONE,
								   TEE_PARAM_TYPE_NONE);
			
			if (param_types != exp_param_types)
				return TEE_ERROR_BAD_PARAMETERS;
		

			uint8_t *out_buffer = params[0].memref.buffer;
			uint32_t out_buffer_size = params[0].memref.size;
			uint32_t access_id = params[1].value.a;
	
			memcpy( out_buffer , g_secret , out_buffer_size);
	
	
			IMSG("g_secret_size: %d", g_secret_size);
			return TEE_SUCCESS;
		}


/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	// We could have a command ID for each type of vulneravility.

	switch (cmd_id) {
	case CMD_SECRET_MANAGMENT_STR:
		return store_secret(param_types, params, cmd_id);
	case CMD_SECRET_MANAGMENT_GET:
		return retrieve_secret(param_types, params, cmd_id);

	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
