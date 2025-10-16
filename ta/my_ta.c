#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>  // For memcpy()
#include <my_ta.h>


/*-------------secret managment variables--------------------*/ 

// VULN 1: Global shared state
TEE_ObjectHandle object;
static uint8_t g_secret[32];
static uint32_t g_secret_size;
static bool g_initialized = false;

// VULN 2: Global pointer to secret data
static uint8_t *g_secret_buffer = NULL;
static uint32_t g_secret_size = 0;

/*-------------end secret managment variables -------------------*/ 

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	//TEE_ObjectHandle object;
    uint32_t read_bytes = 0;
    
    TEE_Result res = TEE_OpenPersistentObject(
        TEE_STORAGE_PRIVATE,
        "secret",
        sizeof("secret"),
        TEE_DATA_FLAG_ACCESS_READ,
        &object);
        
    if (res == TEE_SUCCESS) {
        TEE_ReadObjectData(object, g_secret, sizeof(g_secret), &read_bytes);
        TEE_CloseObject(object);
    }
    
    return TEE_SUCCESS;
}
/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	//TEE_ObjectHandle object;
    
    TEE_CreatePersistentObject(
        TEE_STORAGE_PRIVATE,
        "secret",
        sizeof("secret"),
        TEE_DATA_FLAG_ACCESS_WRITE,
        TEE_HANDLE_NULL,
        NULL,
        0,
        &object);
        
    TEE_WriteObjectData(object, g_secret, sizeof(g_secret));
    TEE_CloseObject(object);
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
	
TEE_Result TA_ALLOCATE_SECRET(uint32_t param_types, TEE_Param params[4]) {
	// vulneravility: missing param type check
	uint8_t *secret_data = params[0].memref.buffer;
	uint32_t size = params[0].memref.size;

	// print old data in the dangling pointer
	if (g_secret_buffer) {
		IMSG("Old secret data in dangling pointer:");
		for (uint32_t i = 0; i < g_secret_size; i++) {
			IMSG("%c", g_secret_buffer[i]);
		}
		IMSG("\n");
	} else {
		IMSG("No old secret data (g_secret_buffer is NULL)\n");
	}

	// Free previous secret if exists
	if (g_secret_buffer) {
		TEE_Free(g_secret_buffer);
		//delay 1 s to increase the attack window
		TEE_Time t1, t2;
		TEE_GetSystemTime(&t1);	
		do {
			TEE_GetSystemTime(&t2);
		} while ((t2.seconds - t1.seconds) < 1);		

		// VULN: g_secret_buffer now dangles - points to freed memory
		g_secret_buffer = NULL;
	}
	
	// Allocate new memory for secret
	g_secret_buffer = TEE_Malloc(size, 0);
	if (!g_secret_buffer) {
		return TEE_ERROR_OUT_OF_MEMORY;
	}
	
	memcpy(g_secret_buffer, secret_data, size);
	g_secret_size = size;

	// print new secret allocated
	IMSG("New secret allocated:");
	for (uint32_t i = 0; i < g_secret_size; i++) {
		IMSG("%c", g_secret_buffer[i]);
	}
	
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
	case CMD_SECRET_ALLOCATE:
		return TA_ALLOCATE_SECRET(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
