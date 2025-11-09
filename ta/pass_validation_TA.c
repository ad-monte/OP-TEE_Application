#include <pass_validation_TA.h>
#include <string.h>  
#include <my_ta.h>

static char pass[] = "Alfonso";

TEE_Result password_validation(uint32_t param_types,TEE_Param params[4])
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
    IMSG("Password pointer: %x", pass);
    // IMSG("Parameter type is: %u", param_types);
    // IMSG("Expected parameter type is: %u", exp_param_types);

    //validate str input from NW
    //validate int input from NW (length)

    IMSG("Got value: %s from NW 1", input_str);
    IMSG("Length received is: %u", (length));
    
    const char *pass_str = (const char *)pass;
    size_t pass_len = strlen(pass_str);

    validated = 0;

    //This a vulnerable validator that check character by character and includes a delay to make timing more evident just as an excercise
    /*if (input_str != NULL && (length == pass_len || length == pass_len + 1)) {
        // compare character by character 
        validated = 1;
        for (size_t i = 0; i < pass_len; i++) {
            if (input_str[i] != pass_str[i]) {
                validated = 0;
                break;
            }
            else{ float p = 2; for (float a=0;a<7000000;a++){ //forced delay to make the timing more evident
                           p+=1.1;
            } }
        }
    } else {
        validated = 0;
    }*/

    validated = (strcmp((const char*)input_str, pass_str)==0) ? 1 : 0;//Seems vulnerable to timing attacks
    IMSG("Password validation result: %d\n", validated);

    params[1].value.a = validated; // set output param

    return TEE_SUCCESS;
}