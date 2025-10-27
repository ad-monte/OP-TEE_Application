#ifndef _secret_manag_TA_H_
#define _secret_manag_TA_H_

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>  // For memcpy()
#include <stdint.h>

TEE_Result store_secret(uint32_t param_types,TEE_Param params[4],uint32_t cmd_id);
TEE_Result retrieve_secret(uint32_t param_types,TEE_Param params[4],uint32_t cmd_id);
TEE_Result updateLog(uint32_t param_types, TEE_Param params[4]);
TEE_Result get_log_entry(uint32_t   param_types, TEE_Param params[4]);

#endif