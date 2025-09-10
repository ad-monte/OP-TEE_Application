#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>  // For memcpy()
#include <my_ta.h>


/*-------------secret managment variables--------------------*/ 

// VULN 1: Global shared state
static uint8_t g_secret[256];
static size_t g_secret_size = 0;

// VULN 2: Unsynchronized counter
static uint32_t g_access_count = 0;  

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

static TEE_Result inc_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a++;
	IMSG("Increase value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}

static TEE_Result dec_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a--;
	IMSG("Decrease value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}


static TEE_Result unsecure_secret(uint32_t param_types,
	TEE_Param params[4],uint32_t cmd_id) {

		switch (cmd_id){
			case CMD_SECRET_MANAGMENT_STR:
			TEE_MemMove(g_secret, params[0].memref.buffer, params[0].memref.size);
				 g_secret_size = params[0].memref.size;
			break;
			case CMD_SECRET_MANAGMENT_GET:
			TEE_MemMove(params[0].memref.buffer, g_secret, g_secret_size);
				params[0].memref.size = g_secret_size;
				g_access_count++;  // VULN 5: Not atomic	
			break;
			case CMD_SECRET_MANAGMENT_ACC:
				// VULN 6: Returns sensitive info
				params[0].value.a = g_access_count;
			break;
		}

		// Danger: No bounds check!		
		memcpy(g_secret, params[0].memref.buffer, params[0].memref.size);
	 		   g_secret_size = params[0].memref.size;

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
	case MY_TA_CMD_INC_VALUE:
		return inc_value(param_types, params);
	case MY_TA_CMD_DEC_VALUE:
		return dec_value(param_types, params);
	case CMD_SECRET_MANAGMENT_STR:
		return unsecure_secret(param_types, params, cmd_id);
	case CMD_SECRET_MANAGMENT_GET:
		return unsecure_secret(param_types, params, cmd_id);
	case CMD_SECRET_MANAGMENT_ACC:
		return unsecure_secret(param_types, params, cmd_id);
	case CMD_LIGHT_CRYPTOGRAPHIC:
		return TEE_SUCCESS;
	case CMD_INPUT_VALIDATION:
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
