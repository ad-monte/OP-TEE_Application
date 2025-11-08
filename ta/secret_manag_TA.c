#include <secret_manag_TA.h>
#include <my_ta.h> 
#include <stdio.h>

// -------- Global variables to store the secret and its size  for functions store_secret and retrieve secret --------
static uint8_t *secret      = NULL;
static uint32_t secret_size = 0;
static uint32_t access_id   = 0;
//			 -------- end of global variables --------

// Persistent object variables for functions store_pobj and retrieve_pobj


struct log_entry {
	uint32_t log_count;
	char message[10][100];
	char timestamp[10][100];
};

// const char VariablePrueba[] = "Variable de prueba";

static const uint8_t OBJ_ID[] = "G8";
static const size_t  OBJ_ID_LEN = sizeof(OBJ_ID) - 1;


TEE_Result updateLog(uint32_t param_types, TEE_Param params[4])
{
	
	IMSG("update logging without sincronization");

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
												TEE_PARAM_TYPE_VALUE_INPUT,
												TEE_PARAM_TYPE_NONE,
												TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
	{
		IMSG("updateLog: bad param types 0x%x", (unsigned)param_types);
		return TEE_ERROR_BAD_PARAMETERS;
	}


	struct log_entry log_data;							// Structure to hold log data
	//TEE_MemFill(&log_data, 0, sizeof(log_data));   	// Initialize log_data to zero	
	uint32_t read_count = 0;							// Number of bytes read from the persistent object

	char *data = (char *)params[0].memref.buffer;		// Pointer to input data
	uint32_t data_size = params[0].memref.size;			// Size of input data
	(void)params; 

    TEE_Result res;
    TEE_ObjectHandle obj = TEE_HANDLE_NULL;   // IMPORTANT: init to NULL
	bool object_exists = false;
	char buffer[] = "data for testing";

    uint32_t flags = TEE_DATA_FLAG_ACCESS_READ |
                     TEE_DATA_FLAG_ACCESS_WRITE |
                     TEE_DATA_FLAG_ACCESS_WRITE_META;

    //  ----------------- Open or create object if missing ----------------- //
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                                   OBJ_ID, OBJ_ID_LEN,
                                   flags, &obj);

    if (res == TEE_ERROR_ITEM_NOT_FOUND) {
		object_exists = false;
        res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                                         OBJ_ID, OBJ_ID_LEN,
                                         flags | TEE_DATA_FLAG_OVERWRITE,
                                         TEE_HANDLE_NULL,
                                         NULL, 0, &obj);
        if (res != TEE_SUCCESS) {
            EMSG("CreatePersistentObject failed 0x%x", res);
            return res;
        }
    } else if (res == TEE_SUCCESS) {
		object_exists = true;
	}
	else {
		TEE_CloseObject(obj);
		EMSG("OpenPersistentObject failed 0x%x", res);
		return res;
	}
	//  ----------------- End open or create object  ----------------- //
	

	// ------------------- Read existing data -------------------------//

	if(object_exists) {
		//read data from persistent object
		IMSG("Updating existing persistent object.");
		//res = TEE_ReadObjectData(obj, &buffer, sizeof(buffer), &read_count);
		res = TEE_ReadObjectData(obj, &log_data, sizeof(log_data), &read_count);		
		
		if (res != TEE_SUCCESS) {
			EMSG("ReadObjectData failed 0x%x", res);
			TEE_CloseObject(obj);
			return res;
		}

	}
	else {
		IMSG("New persistent object.");
		TEE_MemFill(&log_data, 0, sizeof(log_data)); // fully init on first create
		log_data.log_count = 0;
		//buffer[0] = 'N'; // write initial data
	}

	// ----------------- End read existing data -------------------------//



	// choose row to write: append with wrap at 10
	size_t idx = (log_data.log_count % 10);

	// ------------- Store new data ------------- //
	// bounded copy into message[idx] and timestamp[idx]
	const size_t msg_cap = sizeof(log_data.message[0]); // 100
	size_t copy_len = data_size < (msg_cap - 1) ? data_size : (msg_cap - 1);
	TEE_MemFill(log_data.message[idx], 0, msg_cap);
	TEE_MemMove(log_data.message[idx], data, copy_len);
	TEE_MemFill(log_data.timestamp[idx], 0, sizeof(log_data.timestamp[0]));
	// TEE_MemMove(log_data.timestamp[idx], "00:00:00.000", sizeof("00:00:00.000"));
	// ------------ End Store new data ------------- //

	// -------------  GET time ------------- //
	TEE_Time now = (TEE_Time){0};
	//TEE_GetREETime(&now);
	TEE_GetSystemTime(&now);
	uint32_t day_secs = now.seconds % 86400;
	uint32_t hh = day_secs / 3600;
	uint32_t mm = (day_secs % 3600) / 60;
	uint32_t ss = day_secs % 60;

	TEE_MemFill(log_data.timestamp[idx], 0, sizeof(log_data.timestamp[0]));
	snprintf(log_data.timestamp[idx], sizeof(log_data.timestamp[0]),
			"%02u:%02u:%02u.%03u", hh, mm, ss, now.millis);

	// ------------- End GET time ------------- //

	// advance count (total entries written), will wrap on read with %10
	log_data.log_count++;

	// optional bounded log (avoid %s with unknown NUL)
	IMSG("data in log data [%zu]: %.*s", idx, (int)copy_len,
			(const char *)log_data.message[idx]);

	//  ------------- Write updated data back to persistent object ------------- //

    res = TEE_TruncateObjectData(obj, 0);
    if (res == TEE_SUCCESS)
        res = TEE_SeekObjectData(obj, 0, TEE_DATA_SEEK_SET);
    if (res == TEE_SUCCESS)
        //res = TEE_WriteObjectData(obj, &buffer, sizeof(buffer));
		res = TEE_WriteObjectData(obj, &log_data, sizeof(log_data));

	TEE_CloseObject(obj); // safe even if write failed

    if (res != TEE_SUCCESS) {
        EMSG("WriteObjectData failed 0x%x", res);
        return res;
    }

	return TEE_SUCCESS;
}

TEE_Result get_log_entry(uint32_t   param_types,
						 TEE_Param params[4])
{
	IMSG("get log entry");
	const char VariablePrueba[] = "Variable de prueba";
	IMSG("Variable de prueba pointer: %p",(void *) VariablePrueba);
	IMSG("Variable de prueba pointer: %s",VariablePrueba);
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
											   TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_MEMREF_OUTPUT,
											   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
	{
		IMSG("bad param types");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	TEE_Result res;
    TEE_ObjectHandle obj = TEE_HANDLE_NULL;   // IMPORTANT: init to NULL
	struct log_entry log_data;				  // Structure to hold log data
	bool object_exists = false;
	uint32_t read_count = 0;				  // Number of bytes read from the persistent object

	uint32_t flags = TEE_DATA_FLAG_ACCESS_READ;

	// Open persistent object
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
									OBJ_ID, OBJ_ID_LEN,
									flags, &obj);
	if(res != TEE_SUCCESS){
		IMSG("Persistent object not found");
		return TEE_ERROR_ITEM_NOT_FOUND;
	}

	res = TEE_ReadObjectData(obj, &log_data, sizeof(log_data), &read_count);

	if(res != TEE_SUCCESS){
		IMSG("ReadObjectData failed 0x%x", res);
		TEE_CloseObject(obj);
		return res;
	}

	TEE_CloseObject(obj);


	//uint32_t entry_index = params[1].value.a % 10;//Safety against out of bounds reading

	int entry_index = params[1].value.a;
	if(entry_index <10){//Incomplete input validation, not checking negativs
		char *out_buffer = params[0].memref.buffer;
		uint32_t out_sz  = params[0].memref.size;
		char *out_timestamp = params[2].memref.buffer;
		char *src_timestamp = params[2].memref.size;

		const char *src = log_data.message[entry_index];
		IMSG("Log pointer: %p", (void *)&log_data.message[entry_index]);
		IMSG("Test: %s", log_data.message[entry_index]);
		uint32_t src_len = (uint32_t)strnlen(src, sizeof(log_data.message[entry_index]));
		uint32_t n = src_len < out_sz ? src_len : out_sz;
		TEE_MemMove(out_buffer, src, 100);//Change back to n to only copy the lenght of the string
		out_buffer[100] = '\0';
		params[0].memref.size = 100; // tell host how many bytes returned

		const char *src_time = log_data.timestamp[entry_index];
		uint32_t src_time_len = (uint32_t)strnlen(src_time, sizeof(log_data.timestamp[entry_index]));
		uint32_t n_time = src_time_len < src_timestamp ? src_time_len : src_timestamp;
		IMSG("Copying bytes for timestamp");
		//
		TEE_MemMove(out_timestamp, src_time, n_time);
		params[2].memref.size = n_time; // tell host how many bytes returned
		void *ret = __builtin_return_address(0);
		void *ret2 = __builtin_frame_address(0);
		IMSG("Return address: %p", ret);
		IMSG("Stack address: %p", ret2);

		return TEE_SUCCESS;
	}
	else {
		IMSG("Out of bounds access");
		return TEE_ERROR_BAD_PARAMETERS;
	}
}





TEE_Result store_secret(uint32_t param_types,
						TEE_Param params[4], uint32_t cmd_id)
{

	IMSG("unsecure storing");

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
											   TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
	{
		IMSG("bad param types");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	// Store the secret in global variables (ignore security issues): use secret and secret_size.

	// Vulnerability: pointer to secret data in global variable is dangling after free
	if (secret)
	{
		TEE_Free(secret); // free previous secret
		secret = NULL;
	}
	secret = TEE_Malloc(params[0].memref.size, 0); // allocate memory for new secret
	if (!secret)
	{
		return TEE_ERROR_OUT_OF_MEMORY; //	 check allocation
	}
	memcpy(secret, params[0].memref.buffer, params[0].memref.size); // copy new secret

	secret_size = params[0].memref.size;
	access_id = params[1].value.a;

	return TEE_SUCCESS;
}

TEE_Result retrieve_secret(uint32_t param_types,
						   TEE_Param params[4], uint32_t cmd_id)
{

	IMSG("unsecure retrieving");

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
											   TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
	{
		IMSG("bad param types");
		return TEE_ERROR_BAD_PARAMETERS;
	}

	uint8_t *out_buffer = params[0].memref.buffer;
	uint32_t out_buffer_size = params[0].memref.size;
	uint32_t access_id = params[1].value.a;

	if (secret == NULL)
	{
		IMSG("No secret stored");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	memcpy(out_buffer, secret, out_buffer_size);

	// IMSG("g_secret_size: %d", g_secret_size);
	return TEE_SUCCESS;
}