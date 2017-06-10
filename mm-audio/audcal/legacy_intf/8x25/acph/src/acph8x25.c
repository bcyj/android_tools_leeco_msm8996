/**
  \file **************************************************************************
 *
 *  A U D I O   C A L I B R A T I O N   P A C K E T   H A N D L E R
 *
 *FILE:    acph8x25.c
 *
 *DESCRIPTION
 * This file contains the implementation of ACPH for 8x25
 * Parses the command sent from PC and calls appropriate kernel driver API.
 *
 *REFERENCES
 * None.
 *
 *
 *Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */
/*
   -------------------------------
   |Include Files                |
   -------------------------------
   */

#include "acph8x25.h"
#include "rtc_q5_intf8x25.h"
#include "initialize_audcal8x25.h"

/*
   -------------------------------
   |Macros                       |
   -------------------------------
   */
/*
   ---------------------------------
   |Static Variable Definitions    |
   ---------------------------------
   */
char_t * acph_main_buffer = NULL;

/*
   --------------------------------------------------
   |Static Function Declarations and Definitions    |
   --------------------------------------------------
   */

extern void actp_diag_init(
        void (*callback_function)(char_t*, uint32_t, char_t**, uint32_t*)
        );

extern void rtc_query_all_active_devices (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

/**
 * FUNCTION : create_error_resp
 *
 * DESCRIPTION : Create a response error buffer with the specified error code
 * and request command id.
 *
 * DEPENDENCIES : None
 *
 * PARAMS:
 *   error_code - response error code
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void create_error_resp (
        uint32_t error_code,
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint8_t suc_flag = ACPH_SUC_FLAG_FALSE;
    uint32_t resp_data_length;
    (*resp_buf_ptr) = acph_main_buffer;
    if (NULL == (*resp_buf_ptr))
    {
        /**not initialized*/
        *resp_buf_length = 0;
        return;
    }
    *resp_buf_length = ACPH_ERROR_FRAME_LENGTH;
    resp_data_length = ACPH_ERROR_FRAME_LENGTH - ACPH_HEADER_LENGTH;
    /**copy command id */
    memcpy(*resp_buf_ptr, req_buf_ptr, ACPH_COMMAND_ID_LENGTH);
    /**copy data length */
    memcpy(*resp_buf_ptr + ACPH_COMMAND_ID_LENGTH,
            &resp_data_length,
            ACPH_DATA_LENGTH_LENGTH);
    /**copy suc flag */
    memcpy(*resp_buf_ptr + ACPH_HEADER_LENGTH,
            &suc_flag,
            ACPH_ERROR_FLAG_LENGTH);
    /**copy error code */
    memcpy(*resp_buf_ptr + ACPH_HEADER_LENGTH + ACPH_ERROR_FLAG_LENGTH,
            &error_code,
            ACPH_ERROR_CODE_LENGTH);
}

/**
 * FUNCTION : create_suc_resp
 *
 * DESCRIPTION : Create a response buffer with the specified response data
 * and request command id.
 *
 * DEPENDENCIES : None
 *
 * PARAMS:
 *   data_length - response data length
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void create_suc_resp (
        uint32_t data_length,
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint8_t suc_flag = ACPH_SUC_FLAG_TRUE;
    uint32_t resp_data_length;
    *resp_buf_length = ACPH_HEADER_LENGTH + ACPH_SUC_FLAG_LENGTH + data_length;
    (*resp_buf_ptr) = acph_main_buffer;
    if (NULL == (*resp_buf_ptr))
    {
        /**not initialized*/
        *resp_buf_length = 0;
        return;
    }
    resp_data_length = ACPH_SUC_FLAG_LENGTH + data_length;
    /**copy command id */
    memcpy(*resp_buf_ptr, req_buf_ptr, ACPH_COMMAND_ID_LENGTH);
    /**copy response data length */
    memcpy(*resp_buf_ptr + ACPH_COMMAND_ID_LENGTH,
            &resp_data_length,
            ACPH_DATA_LENGTH_LENGTH);
    /**copy suc flag */
    memcpy(*resp_buf_ptr + ACPH_HEADER_LENGTH,
            &suc_flag,
            ACPH_SUC_FLAG_LENGTH);
}

/**
 * FUNCTION : get_command_length
 *
 * DESCRIPTION : Get request data length from a request buffer
 *
 * DEPENDENCIES : the address of a uint32_t variable need to be passed in to
 * contain the data length
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   req_buf_length - length of the request buffer
 *   data_length - pointer to the length of data
 *
 * RETURN VALUE : return false if the there is no data length information
 * or the data length read from the request buffer is not consistent with
 * the total buffer length.
 * return true otherwise and data length is returned in the last parameter.
 *
 * SIDE EFFECTS : None
 */
static bool_t get_command_length (
        char_t *req_buf_ptr,
        uint32_t req_buf_length,
        uint32_t *data_length
        )
{
    bool_t bResult = FALSE;
    if (req_buf_length < ACPH_HEADER_LENGTH)
    {
        /**there is no data length information*/
        bResult = FALSE;
    }
    else
    {
        /** copy data length*/
        memcpy(data_length,
                req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
                ACPH_DATA_LENGTH_LENGTH);
        if (req_buf_length - ACPH_HEADER_LENGTH == *data_length)
        {
            bResult = TRUE;
        }
        else
        {
            /**the data length read from the request buffer is not consistent
              with the total buffer length.*/
            bResult = FALSE;
        }
    }
    return bResult;
}

/**
 * FUNCTION : rtc_get_cal_data
 *
 * DESCRIPTION : Get real time calibration data
 *
 * DEPENDENCIES : DSP must be active
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
static void rtc_get_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    rtc_get_q5_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : rtc_set_cal_data
 *
 * DESCRIPTION : Set real time calibration data
 *
 * DEPENDENCIES : DSP must be active
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
static void rtc_set_cal_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    rtc_set_q5_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : get_command_id
 *
 * DESCRIPTION : Get request command id from a request buffer
 * The address of a pointer to a function with type void (char_t*, char_t**, uint32_t*) should
 * be pass in, and a ACDB operation function will be plug into it
 *
 * DEPENDENCIES : get_command_length is supposed to be called before this function,
 * which will check the command length and make sure command id is there
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   command_id - pointer to command id (to be returned)
 *   pfnRequested_Operation_Function -
 *     [out] pointer to the requested operation function ACDB or ADIE
 *
 * RETURN VALUE : TRUE if the command id is valid; FALSE otherwise.
 *
 * SIDE EFFECTS : None
 */
static bool_t get_command_id (
        char_t *req_buf_ptr,
        uint16_t *command_id,
        void (**pfnRequested_Operation_Function)(
            char_t*,
            char_t**,
            uint32_t*
            )
        )
{
    bool_t bResult = TRUE;
    /**copy command id*/
    memcpy((command_id), req_buf_ptr + ACPH_COMMAND_ID_POSITION,
            ACPH_COMMAND_ID_LENGTH);
    /**choose corresponding ACDB operation function */
    switch(*command_id)
    {

        case ACPH_CMD_GET_ACTIVE_DEVICELIST:
            *pfnRequested_Operation_Function = rtc_query_all_active_devices;
            break;
        case ACPH_CMD_RTC_GET_CAL_DATA:
            *pfnRequested_Operation_Function = rtc_get_cal_data;
            break;
        case ACPH_CMD_RTC_SET_CAL_DATA:
            *pfnRequested_Operation_Function = rtc_set_cal_data;
            break;
        default:
            LOGE("libaudcal: get_command_id() - Unknown command - %d \n",*command_id);
            bResult = FALSE;
            break;
    }
    return bResult;
}

/*
   ----------------------------------
   | Externalized Function Definitions    |
   ----------------------------------
   */
/**
 * FUNCTION : acph_init
 *
 * DESCRIPTION : Initilize ACPH. Allocate memory.
 *
 * DEPENDENCIES : None
 *
 * RETURN VALUE : ACPH_SUCCESS when success, ACPH_FAILURE otherwise
 *
 * SIDE EFFECTS : None
 */
int32_t acph_init(void)
{
    int32_t result = ACPH_SUCCESS;
    if (NULL == acph_main_buffer)
    {
        acph_main_buffer = (char_t *) malloc(ACPH_ACDB_BUFFER_POSITION + ACPH_BUFFER_LENGTH);
        if (NULL == acph_main_buffer)
        {
            /**malloc failed*/
            LOGE("libaudcal: acph_init() - Memory allocation failed for acph_main_buffer!!\n");
            result = ACPH_FAILURE;
        }
        else
        {
            LOGE("libaudcal: acph_init() - Allocated memory for acph_main_buffer!!\n");
        }
    }
    return result;
}

/**
 * FUNCTION : acph_deinit
 *
 * DESCRIPTION : Deinitilize ACPH. Free memory.
 *
 * DEPENDENCIES : None
 *
 * RETURN VALUE : ACPH_SUCCESS when success, ACPH_FAILURE otherwise
 *
 * SIDE EFFECTS : None
 */
int32_t acph_deinit(void)
{
    int32_t result = ACPH_SUCCESS;
    if (NULL != acph_main_buffer)
    {
        free(acph_main_buffer);
        acph_main_buffer = NULL;
    }
    return result;
}

/**
 * FUNCTION : acph_execute_command
 *
 * DESCRIPTION : Interpret request and operate RTC correspondingly, response with
 * the result.
 *
 * DEPENDENCIES : acph_main_buffer needs to be initialized before this function is called
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   req_buf_length - length of the request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void acph_execute_command (
        char_t *req_buf_ptr,
        uint32_t req_buf_length,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t ulData_Length;
    uint16_t usCommand_ID;
    void (*pfnRequested_Operation_Function)(char_t*, char_t**, uint32_t*);

    if(NULL == acph_main_buffer)
    {
    // ACPH main buffer is not initialized!! Doing nothing, just returning.
        LOGE("libaudcal: acph_execute_command() - acph_main_buffer is NULL. is acph_init() called??\n");
        return;
    }

    // setting the buffer to zero's for each call
    memset(acph_main_buffer,0,ACPH_ACDB_BUFFER_POSITION + ACPH_BUFFER_LENGTH);

    if (FALSE == get_command_length(req_buf_ptr, req_buf_length, &ulData_Length))
    {
        /**return error response: ERR_LENGTH_NOT_MATCH */
        create_error_resp(ACPH_ERR_LENGTH_NOT_MATCH, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (FALSE == get_command_id(req_buf_ptr, &usCommand_ID, &pfnRequested_Operation_Function))
    {
        /**return error response: ERR_INVALID_COMMAND */
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    pfnRequested_Operation_Function(req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : audcal_initialize
 *
 * DESCRIPTION : Initialize Audcal during driver initialization. Should be called
 *                         before any command is sent from PC
 *               Allocates memory for the buffer which is used to communicate with PC.
 *
 * DEPENDENCIES : None
 *
 * PARAMS: None
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void audcal_initialize(void){
    acph_init();
    actp_diag_init(acph_execute_command);
}
/**
 * FUNCTION : audcal_deinitialize
 *
 * DESCRIPTION : Deinitialize Audcal during driver termination.
 *
 * DEPENDENCIES : None
 *
 * PARAMS: None
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void audcal_deinitialize(void){
    acph_deinit();
}
