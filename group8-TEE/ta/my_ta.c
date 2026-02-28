#include <inttypes.h> // to implement light cryptographic services AES
#include <light_crypto_TA.h>
#include <secret_manag_TA.h>
#include <pass_validation_TA.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>  // For memcpy()
#include <my_ta.h>


//AES constants
#define AES128_KEY_BIT_SIZE		128
#define AES128_KEY_BYTE_SIZE		(AES128_KEY_BIT_SIZE / 8)
#define AES256_KEY_BIT_SIZE		256
#define AES256_KEY_BYTE_SIZE		(AES256_KEY_BIT_SIZE / 8)

extern struct aes_cipher; //aes global

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
									  uint32_t cmd_id,
									  uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	IMSG("TA_InvokeCommandEntryPoint has been called with cmd_id=0x%" PRIx32, cmd_id);
	// We could have a command ID for each type of vulneravility.
	
	switch (cmd_id)
	{
	case CMD_SECRET_MANAGMENT_STR:
		return store_secret(param_types, params, cmd_id); // 0
	case CMD_SECRET_MANAGMENT_GET:
		return retrieve_secret(param_types, params, cmd_id); // 1
	case TA_AES_CMD_PREPARE:
		return alloc_resources(sess_ctx, param_types, params); // 2
	case TA_AES_CMD_SET_KEY:
		return set_aes_key(sess_ctx, param_types, params); // 3
	case TA_AES_CMD_SET_IV:
		return reset_aes_iv(sess_ctx, param_types, params); // 4
	case TA_AES_CMD_CIPHER:
		return cipher_buffer(sess_ctx, param_types, params); // 5
	case CMD_PASSWORD_VALIDATION:
		return password_validation(param_types, params); // 6
	case CMD_UPDATE_LOG:
		return updateLog(param_types, params); // 
	case CMD_GET_LOG:
		return get_log_entry(param_types, params); //	

	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

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


