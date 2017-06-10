/** 
  \file **************************************************************************
 *
 *  A U D I O   C A L I B R A T I O N   P A C K E T   H A N D L E R   
 *
 *DESCRIPTION
 * This file contains the implementation of ACPH 
 *
 *REFERENCES
 * None.
 *
 *Copyright (c) 2010-2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
 *******************************************************************************
 */
/*
   -------------------------------
   |Include Files                |
   -------------------------------
   */
#include "acdb.h"
#include "acph.h"
#include "rtc_q6_intf.h"

/*
   -------------------------------
   |Macros                       |
   -------------------------------
   */
uint32_t ACPH_HALF_BUF_LEN = ACPH_BUFFER_LENGTH/2 - sizeof(uint32_t);
/*
   ---------------------------------
   |Static Variable Definitions    |
   ---------------------------------
   */
char_t * acph_main_buffer = NULL;
char_t * acph_sub_buffer = NULL;

/*
   --------------------------------------------------
   |Static Function Declarations and Definitions    |
   --------------------------------------------------
   */
extern int32_t adie_execute_command(
        void *input_buf_ptr, 
        uint32_t *resp_buf_length_ptr
        );

extern void query_aud_topology_copp_handles (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

extern void query_aud_copp_stream_handles (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

extern void query_voc_all_active_streams (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

extern void query_voc_vs_copp_handles (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

extern void query_voc_vc_topology (
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
#ifdef LOGE
    LOGE("[ACPH ERROR]->Command ID [%08]\n",*((uint32_t*)req_buf_ptr));
#endif
    /**copy data length */
    memcpy(*resp_buf_ptr + ACPH_COMMAND_ID_LENGTH, 
           &resp_data_length, 
           ACPH_DATA_LENGTH_LENGTH);
#ifdef LOGE
    LOGE("[ACPH ERROR]->data length [%d]\n",resp_data_length);
#endif
    /**copy suc flag */
    memcpy(*resp_buf_ptr + ACPH_HEADER_LENGTH, 
           &suc_flag, 
           ACPH_ERROR_FLAG_LENGTH);
#ifdef LOGE
    LOGE("[ACPH ERROR]->success flag [%08X]\n",suc_flag);
#endif
    /**copy error code */
    memcpy(*resp_buf_ptr + ACPH_HEADER_LENGTH + ACPH_ERROR_FLAG_LENGTH, 
           &error_code, 
           ACPH_ERROR_CODE_LENGTH);
#ifdef LOGE
    LOGE("[ACPH ERROR]->error code [%08X]\n",error_code);
#endif
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
 * FUNCTION : get_acdb_version
 *
 * DESCRIPTION : get ACDB version
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_acdb_version (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_ACDB_VERSION,
            (const uint8_t *)NULL, 0, 
            (uint8_t *)(acph_main_buffer+ACPH_ACDB_BUFFER_POSITION), 
            sizeof(AcdbModuleVersionType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(AcdbModuleVersionType),
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_target_version
 *
 * DESCRIPTION : Get target version from ACDB
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_target_version (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_TARGET_VERSION,
            (const uint8_t *)NULL, 0, 
            (uint8_t *)(acph_main_buffer+ACPH_ACDB_BUFFER_POSITION), 
            sizeof(AcdbTargetVersionType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(ACPH_ERR_INVALID_TARGET_VERSION, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(AcdbTargetVersionType),
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : init_acdb
 *
 * DESCRIPTION : init ACDB to the default values
 *
 * DEPENDENCIES : None
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
static void init_acdb (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_INITIALIZE,
            (const uint8_t *)NULL, 0,
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : reset_acdb
 *
 * DESCRIPTION : reset ACDB module to the initial state
 *
 * DEPENDENCIES : None
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
static void reset_acdb (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_RESET,
            (const uint8_t *)NULL, 0,
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr,
                resp_buf_ptr,
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0,
                req_buf_ptr,
                resp_buf_ptr,
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : estimate_memory_use
 *
 * DESCRIPTION : calculates the memory usage used by the ACDB data struct
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void estimate_memory_use (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON,
                req_buf_ptr,
                resp_buf_ptr,
                resp_buf_length);
        return;
    }
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_ESTIMATE_MEMORY_USE,
            (const uint8_t *)NULL, 0,
            (uint8_t *)(acph_main_buffer+ACPH_ACDB_BUFFER_POSITION), 
            sizeof(AcdbMemoryUsageType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(AcdbMemoryUsageType),
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_device_info
 *
 * DESCRIPTION : Request device information from ACDB
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_device_info (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbDeviceInfoCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2*ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /**copy device id to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_DEVICE_INFO,
            (const uint8_t *)&acdb_command, 
            sizeof(AcdbDeviceInfoCmdType),
            (uint8_t *)&acdb_result, 
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_device_capabilities
 *
 * DESCRIPTION : get the device capabilities from ACDB
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_device_capabilities (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbDeviceCapabilitiesCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH,
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_DEVICE_CAPABILITIES,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbDeviceCapabilitiesCmdType),
            (uint8_t *)&acdb_result, 
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/* TODO: need to update this routine once ACDB implementation is done */
/**
 * FUNCTION : is_device_paired
 *
 * DESCRIPTION : check if two devices (tx and rx) are paired or not
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void is_device_paired (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbDevicePairType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,
            req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_IS_DEVICE_PAIRED,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbDevicePairType), 
            acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
            sizeof(AcdbDevicePairingResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(AcdbDevicePairingResponseType),
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_vocproc_common_data
 *
 * DESCRIPTION : get vocproc common leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_vocproc_common_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbVocProcCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (8 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nTxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 6 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 7 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocProcCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_vocproc_common_data
 *
 * DESCRIPTION : set vocproc common leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_vocproc_common_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbVocProcCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (8 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 8*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nTxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 6 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 7 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 8 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 8 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_VOCPROC_COMMON_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocProcCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_vocproc_common_table
 *
 * DESCRIPTION : get vocproc common leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_vocproc_common_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbVocProcTableCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (6 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nTxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_VOCPROC_COMMON_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocProcTableCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_vocproc_common_table
 *
 * DESCRIPTION : set vocproc common leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_vocproc_common_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbVocProcTableCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (6 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 6*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nTxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 6 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 6 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_VOCPROC_COMMON_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocProcTableCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_vocproc_stream_data
 *
 * DESCRIPTION : get vocproc stream leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_vocproc_stream_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbVocStrmCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_VOCPROC_STREAM_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocStrmCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_vocproc_stream_data
 *
 * DESCRIPTION : set vocproc stream leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_vocproc_stream_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbVocStrmCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 4*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 4 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_VOCPROC_STREAM_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocStrmCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_vocproc_stream_table
 *
 * DESCRIPTION : get vocproc stream leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_vocproc_stream_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbVocStrmTableCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_VOCPROC_STREAM_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocStrmTableCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_vocproc_stream_table
 *
 * DESCRIPTION : set vocproc stream leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_vocproc_stream_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbVocStrmTableCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 2*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 2 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_VOCPROC_STREAM_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocStrmTableCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_common_data
 *
 * DESCRIPTION : get audproc common leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_common_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAudProcCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (5 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_audproc_common_data
 *
 * DESCRIPTION : set audproc common leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_audproc_common_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAudProcCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (5 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 5*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 5 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_AUDPROC_COMMON_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_common_table
 *
 * DESCRIPTION : get audproc common leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_common_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAudProcTableCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (3 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcTableCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_audproc_common_table
 *
 * DESCRIPTION : set audproc common leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_audproc_common_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAudProcTableCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (3 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 3*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nDeviceSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 3 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_AUDPROC_COMMON_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcTableCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_stream_data
 *
 * DESCRIPTION : get audproc stream leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_stream_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAudStrmCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationTypeId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_STREAM_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudStrmCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_audproc_stream_data
 *
 * DESCRIPTION : set audproc stream leg data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_audproc_stream_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAudStrmCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 4*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationTypeId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 4 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_AUDPROC_STREAM_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudStrmCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_stream_table
 *
 * DESCRIPTION : get audproc stream leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_stream_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAudStrmTableCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationTypeId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_STREAM_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudStrmTableCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_audproc_stream_table
 *
 * DESCRIPTION : set audproc stream leg table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_audproc_stream_table (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAudStrmTableCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 2*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationTypeId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 2 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_AUDPROC_STREAM_TABLE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudStrmTableCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_vocproc_gain_dep_voltbl
 *
 * DESCRIPTION : get vocproc gain dependent volume table data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_vocproc_gain_dep_voltbl (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbVocProcVolTblCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocProcVolTblCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_vocproc_step_data_gain_dep_voltbl
 *
 * DESCRIPTION : get vocproc gain dependent volume table step data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_vocproc_step_data_gain_dep_voltbl (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbVocProcVolTblDataCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (7 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVolumeIndex), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 6 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocProcVolTblDataCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_vocproc_step_data_gain_dep_voltbl
 *
 * DESCRIPTION : set vocproc gain dependent volume table step data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_vocproc_step_data_gain_dep_voltbl (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbVocProcVolTblDataCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (7 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 7*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nNetworkId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVocProcSampleRateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVolumeIndex), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 6 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 7 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 7 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbVocProcVolTblDataCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_gain_dep_voltbl
 *
 * DESCRIPTION : get audproc gain dependent volume table
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_gain_dep_voltbl (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAudProcVolTblCmdType acdb_command;
    AcdbAudProcGainDepVolTblRspType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);

    acdb_command.nCoppBufferLength = ACPH_HALF_BUF_LEN - ACPH_ACDB_BUFFER_POSITION;
    acdb_command.nCoppBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION
                                      + sizeof(uint32_t);
    acdb_command.nPoppBufferLength = ACPH_HALF_BUF_LEN;
    acdb_command.nPoppBufferPointer = acph_sub_buffer;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcVolTblCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbAudProcGainDepVolTblRspType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** Need to combine two buffers into one and set the correct
         * buffer length values */
        uint32_t total_len = sizeof(uint32_t) + sizeof(uint32_t)
                            + acdb_result.nBytesUsedCoppBuffer
                            + acdb_result.nBytesUsedPoppBuffer;
        memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
                (uint8_t *)&acdb_result.nBytesUsedCoppBuffer,
                sizeof(uint32_t));
        memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION
                + sizeof(uint32_t) + acdb_result.nBytesUsedCoppBuffer,
                (uint8_t *)&acdb_result.nBytesUsedPoppBuffer,
                sizeof(uint32_t));
        memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION
                + sizeof(uint32_t) + acdb_result.nBytesUsedCoppBuffer
                + sizeof(uint32_t),
                acph_sub_buffer,
                acdb_result.nBytesUsedPoppBuffer);
        /** ACDB succeeded*/
        create_suc_resp(total_len, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_popp_gain_dep_voltbl_step
 *
 * DESCRIPTION : get audproc gain dependent volume table step for POPP
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_popp_gain_dep_voltbl_step (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAudProcGainDepVolTblStepCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (3 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVolumeIndex), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_POPP,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcGainDepVolTblStepCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_copp_gain_dep_voltbl_step
 *
 * DESCRIPTION : get audproc gain dependent volume table step for COPP
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_copp_gain_dep_voltbl_step (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAudProcGainDepVolTblStepCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (3 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVolumeIndex), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcGainDepVolTblStepCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_step_data_gain_dep_voltbl
 *
 * DESCRIPTION : get audproc gain dependent volume table step data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_step_data_gain_dep_voltbl (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAudProcVolTblDataCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (5 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVolumeIndex), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcVolTblDataCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_audproc_step_data_gain_dep_voltbl
 *
 * DESCRIPTION : set audproc gain dependent volume table step data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_audproc_step_data_gain_dep_voltbl (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAudProcVolTblDataCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (5 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 5*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nVolumeIndex), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 5 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 5 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAudProcVolTblDataCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_vol_table_step_size
 *
 * DESCRIPTION : get volume table step size
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_vol_table_step_size (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_VOL_TABLE_STEP_SIZE,
            (const uint8_t *)NULL,
            0,
            acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
            sizeof(AcdbVolTblStepSizeRspType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(AcdbVolTblStepSizeRspType),
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_device_pair_list
 *
 * DESCRIPTION : get device pair list
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_device_pair_list (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGeneralInfoCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_DEVICE_PAIR_LIST,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGeneralInfoCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_date_info
 *
 * DESCRIPTION : get date information
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_date_info (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGeneralInfoCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_DATE_INFO,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGeneralInfoCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_oem_info
 *
 * DESCRIPTION : get oem information
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_oem_info (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGeneralInfoCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_OEM_INFO,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGeneralInfoCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_anc_tx_device
 *
 * DESCRIPTION : get ANC TX device
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_anc_tx_device (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAncDevicePairCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_ANC_TX_DEVICE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAncDevicePairCmdType),
            acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
            sizeof(AcdbAncDevicePairRspType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(AcdbAncDevicePairRspType),
                    req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_anc_device_pair_list
 *
 * DESCRIPTION : get ANC device pair list
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_anc_device_pair_list (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGeneralInfoCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_ANC_DEVICE_PAIR_LIST,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGeneralInfoCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_adie_codec_path_profile
 *
 * DESCRIPTION : get adie codec path profile
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_adie_codec_path_profile (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAdiePathProfileCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.ulCodecPathId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nFrequencyId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nOversamplerateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_ADIE_CODEC_PATH_PROFILE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAdiePathProfileCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_adie_codec_path_profile
 *
 * DESCRIPTION : set adie codec path profile
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_adie_codec_path_profile (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAdiePathProfileCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 4*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.ulCodecPathId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nFrequencyId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nOversamplerateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 4 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAdiePathProfileCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_anc_setting
 *
 * DESCRIPTION : get anc setting
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_anc_setting (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbANCSettingCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nFrequencyId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nOversamplerateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_ANC_SETTING,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbANCSettingCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_anc_setting
 *
 * DESCRIPTION : set anc setting
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_anc_setting (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbANCSettingCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 4*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nFrequencyId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nOversamplerateId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 4 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_ANC_SETTING,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbANCSettingCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_afe_data
 *
 * DESCRIPTION : get afe data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_afe_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbAfeDataCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AFE_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAfeDataCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : set_afe_data
 *
 * DESCRIPTION : set afe data
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void set_afe_data (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    AcdbAfeDataCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (4 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else if ((ulData_Length - 4*ACPH_CAL_DATA_UNIT_LENGTH) > 
            (ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION))
    {
        create_error_resp(ACPH_ERR_OUT_OF_BUFFER_SIZE,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nTxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nRxDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nModuleId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = ulData_Length - 4 * ACPH_CAL_DATA_UNIT_LENGTH;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /**copy calibration data to acdb command*/ 
    memcpy((void *) acdb_command.nBufferPointer, 
            req_buf_ptr + ACPH_HEADER_LENGTH + 4 * ACPH_CAL_DATA_UNIT_LENGTH, 
            acdb_command.nBufferLength);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_SET_AFE_DATA,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbAfeDataCmdType),
            (uint8_t *)NULL, 0);
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : check_connection
 *
 * DESCRIPTION : check connection
 *
 * DEPENDENCIES : none
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
static void check_connection (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    create_suc_resp(0,
            req_buf_ptr, 
            resp_buf_ptr, 
            resp_buf_length);
}

/**
 * FUNCTION : get_lookup_table_size
 *
 * DESCRIPTION : get lookup table size
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_lookup_table_size (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGetTableSizeCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_LOOKUP_TABLE_SIZE,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGetTableSizeCmdType),
            (uint8_t *)acph_main_buffer+ACPH_ACDB_BUFFER_POSITION,
            sizeof(AcdbGetTableSizeRspType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(AcdbGetTableSizeRspType),
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_table_index_combination
 *
 * DESCRIPTION : get the combined table indexes
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_table_index_combination (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbQueriedTblIndexCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (3 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nParamId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nTblKeyIndexStart), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nDataLenToCopy), 
            req_buf_ptr + ACPH_HEADER_LENGTH + 2 * ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_TABLE_INDEX_COMBINATION,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbQueriedTblIndexCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_common_topology_id
 *
 * DESCRIPTION : get the audproc common topology id
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_common_topology_id (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGetAudProcTopIdCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (2 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nDeviceId), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGetAudProcTopIdCmdType),
            (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION),
            sizeof(AcdbGetTopologyIdRspType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(uint32_t), req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_stream_topology_id
 *
 * DESCRIPTION : get the audproc stream topology id
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_stream_topology_id (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGetAudProcStrmTopIdCmdType acdb_command;
    int32_t result_flag;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    /**copy calibration ids to acdb command*/
    memcpy(&(acdb_command.nApplicationType), 
            req_buf_ptr + ACPH_HEADER_LENGTH, 
            ACPH_CAL_DATA_UNIT_LENGTH);
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGetAudProcStrmTopIdCmdType),
            (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION),
            sizeof(AcdbGetTopologyIdRspType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(sizeof(uint32_t), req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}


/**
 * FUNCTION : get_audproc_common_topology_id_list
 *
 * DESCRIPTION : get the audproc common topology id list
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_common_topology_id_list (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGeneralInfoCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGeneralInfoCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : get_audproc_stream_topology_id_list
 *
 * DESCRIPTION : get the audproc stream topology id list
 *
 * DEPENDENCIES : ACDB needs to be available and initialized
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
static void get_audproc_stream_topology_id_list (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t nPageSize = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    AcdbGeneralInfoCmdType acdb_command;
    AcdbQueryResponseType acdb_result;
    int32_t result_flag;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    acdb_command.nBufferLength = nPageSize;
    acdb_command.nBufferPointer = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
    /** call ACDB */
    result_flag = acdb_ioctl(ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST,
            (const uint8_t *)&acdb_command,
            sizeof(AcdbGeneralInfoCmdType),
            (uint8_t *)&acdb_result,
            sizeof(AcdbQueryResponseType));
    if (ACDB_SUCCESS != result_flag)
    {
        /** ACDB failed*/
        create_error_resp(result_flag, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
    else
    {
        /** ACDB succeeded*/
        create_suc_resp(acdb_result.nBytesUsedInBuffer,
                req_buf_ptr, resp_buf_ptr, resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : query_max_buffer_length
 *
 * DESCRIPTION : get related maximum buffer length
 *
 * DEPENDENCIES : ACPH needs to be initialized
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * INPUT: None
 *
 * RETURN VALUE : 
 *      32-bit ACPH BUFFER SIZE
 *
 * SIDE EFFECTS : None
 */
static void query_max_buffer_length (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    uint32_t acph_buf_len = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
            &acph_buf_len, sizeof(uint32_t));
    create_suc_resp(sizeof(uint32_t),
            req_buf_ptr, 
            resp_buf_ptr, 
            resp_buf_length);
}

/**
 * FUNCTION : set_adie_register
 *
 * DESCRIPTION : Set value of mask to ADIE register
 *
 * DEPENDENCIES : ADIE API must be available for query
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
static void set_adie_register (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    if(NULL!=resp_buf_ptr && NULL !=resp_buf_length)
    {
        int32_t result = adie_execute_command(req_buf_ptr, resp_buf_length);
        if(ACPH_SUCCESS!=result)
        {
            create_error_resp(result, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
        else
        {
            create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
    }
    return;
}

/**
 * FUNCTION : get_adie_register
 *
 * DESCRIPTION : Get value of register with the specified mask
 *
 * DEPENDENCIES : ADIE API must be available for query
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
static void get_adie_register (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    if(NULL!=resp_buf_ptr && NULL !=resp_buf_length)
    {
        int32_t result = adie_execute_command(req_buf_ptr, resp_buf_length);
        if(ACPH_SUCCESS!=result)
        {
            create_error_resp(result, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
        else
        {
            create_suc_resp(*resp_buf_length, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
    }
    return;
}

/**
 * FUNCTION : get_multiple_adie_registers
 *
 * DESCRIPTION : Get values of multiple ADIE registers
 *
 * DEPENDENCIES : ADIE API must be available for query
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
static void get_multiple_adie_registers (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    if(NULL!=resp_buf_ptr && NULL !=resp_buf_length)
    {
        int32_t result = adie_execute_command(req_buf_ptr,resp_buf_length);
        if(ACPH_SUCCESS!=result)
        {
            create_error_resp(result, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
        else
        {
            create_suc_resp(*resp_buf_length, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
    }
    return;
}

/**
 * FUNCTION : set_multiple_adie_registers
 *
 * DESCRIPTION : Set values to multiple ADIE registers
 *
 * DEPENDENCIES : ADIE API must be available for query
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
static void set_multiple_adie_registers (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    if(NULL!=resp_buf_ptr && NULL !=resp_buf_length)
    {
        int32_t result = adie_execute_command(req_buf_ptr,resp_buf_length);
        if(ACPH_SUCCESS!=result)
        {
            create_error_resp(result, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
        else
        {
            create_suc_resp(*resp_buf_length, req_buf_ptr, resp_buf_ptr, resp_buf_length);
        }
    }
    return;
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
    rtc_get_q6_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length);
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
    rtc_set_q6_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : rtc_get_cal_data_shmem
 *
 * DESCRIPTION : Get real time calibration data using shared memory
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
static void rtc_get_cal_data_shmem (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    rtc_get_q6_cal_data_shmem(req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : rtc_set_cal_data_shmem
 *
 * DESCRIPTION : Set real time calibration data using shared memory
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
static void rtc_set_cal_data_shmem (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    rtc_set_q6_cal_data_shmem(req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
 * FUNCTION : rtc_trans_apr_pkt
 *
 * DESCRIPTION : Transmit pre-packaged APR packet to DSP
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
static void rtc_trans_apr_pkt (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    void* send_buf = (void*) (req_buf_ptr + ACPH_HEADER_LENGTH);
    void* resp_buf = (void*) ((uint32_t)acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    uint32_t resp_len = 0;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if ( ACPH_APR_MIN_HEADER_SIZE > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /**transmit the APR packet*/
    result_flag = rtc_q6_intf_send(send_buf,resp_buf,&resp_len);
    if (RTC_INTF_SUCCESS != result_flag)
    {
        /** APR packet failed*/
        create_error_resp(ACPH_ERR_APR_DSP_CMD_FAILURE, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** APR packet succeeded*/
        create_suc_resp(resp_len,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : rtc_trans_apr_get_pkt_shmem
 *
 * DESCRIPTION : Transmit pre-packaged APR getparam packet to DSP using shared memory
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
static void rtc_trans_apr_get_pkt_shmem (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    void* send_buf = (void*) (req_buf_ptr + ACPH_HEADER_LENGTH);
    void* resp_buf = (void*) ((uint32_t)acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    uint32_t resp_len = 0;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if ( ACPH_APR_MIN_HEADER_SIZE > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /**transmit the APR getparam packet using shared memory*/
    result_flag = rtc_q6_intf_send_get_shmem(send_buf,resp_buf,&resp_len);
    if (RTC_INTF_SUCCESS != result_flag)
    {
        /** APR packet failed*/
        create_error_resp(ACPH_ERR_APR_DSP_CMD_FAILURE, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** APR packet succeeded*/
        create_suc_resp(resp_len,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
}

/**
 * FUNCTION : rtc_trans_apr_set_pkt_shmem
 *
 * DESCRIPTION : Transmit pre-packaged APR setparam packet to DSP using shared memory
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
static void rtc_trans_apr_set_pkt_shmem (
        char_t *req_buf_ptr,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        )
{
    int32_t result_flag;
    void* send_buf = (void*) (req_buf_ptr + ACPH_HEADER_LENGTH);
    void* resp_buf = (void*) ((uint32_t)acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
    uint32_t resp_len = 0;
    uint32_t ulData_Length;
    memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
            ACPH_DATA_LENGTH_LENGTH);
    if ( ACPH_APR_MIN_HEADER_SIZE > ulData_Length)
    {
        /**command parameter missing*/
        create_error_resp(ACPH_ERR_INVALID_COMMAND, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    if (NULL == acph_main_buffer)
    {
        /**not initilized*/
        create_error_resp(ACPH_ERR_UNKNOWN_REASON, 
                req_buf_ptr, resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    /**transmit the APR setparam packet using shared memory*/
    result_flag = rtc_q6_intf_send_set_shmem(send_buf,resp_buf,&resp_len);
    if (RTC_INTF_SUCCESS != result_flag)
    {
        /** APR packet failed*/
        create_error_resp(ACPH_ERR_APR_DSP_CMD_FAILURE, 
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
    else
    {
        /** APR packet succeeded*/
        create_suc_resp(resp_len,
                req_buf_ptr, 
                resp_buf_ptr, 
                resp_buf_length);
        return;
    }
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
       //Online Calibration
        case ACPH_CMD_GET_ACDB_VERSION:
            *pfnRequested_Operation_Function = get_acdb_version;
            break;
        case ACPH_CMD_GET_TARGET_VERSION:
            *pfnRequested_Operation_Function = get_target_version;
            break;
        case ACPH_CMD_ACDB_INIT:
            *pfnRequested_Operation_Function = init_acdb;
            break;
        case ACPH_CMD_ACDB_RESET:
            *pfnRequested_Operation_Function = reset_acdb;
            break;
        case ACPH_CMD_ESTIMATE_MEMORY_USE:
            *pfnRequested_Operation_Function = estimate_memory_use;
            break;
        case ACPH_CMD_GET_DEVICE_INFO:
            *pfnRequested_Operation_Function = get_device_info;
            break;
        case ACPH_CMD_GET_DEVICE_CAPABILITIES:
            *pfnRequested_Operation_Function = get_device_capabilities;
            break;
        case ACPH_CMD_IS_DEVICE_PAIRED:
            *pfnRequested_Operation_Function = is_device_paired;
            break;
        case ACPH_CMD_GET_VOCPROC_COMMON_DATA:
            *pfnRequested_Operation_Function = get_vocproc_common_data;
            break;
        case ACPH_CMD_SET_VOCPROC_COMMON_DATA:
            *pfnRequested_Operation_Function = set_vocproc_common_data;
            break;
        case ACPH_CMD_GET_VOCPROC_COMMON_TABLE:
            *pfnRequested_Operation_Function = get_vocproc_common_table;
            break;
        case ACPH_CMD_SET_VOCPROC_COMMON_TABLE:
            *pfnRequested_Operation_Function = set_vocproc_common_table;
            break;
        case ACPH_CMD_GET_VOCPROC_STREAM_DATA:
            *pfnRequested_Operation_Function = get_vocproc_stream_data;
            break;
        case ACPH_CMD_SET_VOCPROC_STREAM_DATA:
            *pfnRequested_Operation_Function = set_vocproc_stream_data;
            break;
        case ACPH_CMD_GET_VOCPROC_STREAM_TABLE:
            *pfnRequested_Operation_Function = get_vocproc_stream_table;
            break;
        case ACPH_CMD_SET_VOCPROC_STREAM_TABLE:
            *pfnRequested_Operation_Function = set_vocproc_stream_table;
            break;
        case ACPH_CMD_GET_AUDPROC_COMMON_DATA:
            *pfnRequested_Operation_Function = get_audproc_common_data;
            break;
        case ACPH_CMD_SET_AUDPROC_COMMON_DATA:
            *pfnRequested_Operation_Function = set_audproc_common_data;
            break;
        case ACPH_CMD_GET_AUDPROC_COMMON_TABLE:
            *pfnRequested_Operation_Function = get_audproc_common_table;
            break;
        case ACPH_CMD_SET_AUDPROC_COMMON_TABLE:
            *pfnRequested_Operation_Function = set_audproc_common_table;
            break;
        case ACPH_CMD_GET_AUDPROC_STREAM_DATA:
            *pfnRequested_Operation_Function = get_audproc_stream_data;
            break;
        case ACPH_CMD_SET_AUDPROC_STREAM_DATA:
            *pfnRequested_Operation_Function = set_audproc_stream_data;
            break;
        case ACPH_CMD_GET_AUDPROC_STREAM_TABLE:
            *pfnRequested_Operation_Function = get_audproc_stream_table;
            break;
        case ACPH_CMD_SET_AUDPROC_STREAM_TABLE:
            *pfnRequested_Operation_Function = set_audproc_stream_table;
            break;
        case ACPH_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL:
            *pfnRequested_Operation_Function = get_vocproc_gain_dep_voltbl;
            break;
        case ACPH_CMD_GET_VOCPROC_STEP_DATA_GAIN_DEP_VOLTBL:
            *pfnRequested_Operation_Function = get_vocproc_step_data_gain_dep_voltbl;
            break;
        case ACPH_CMD_SET_VOCPROC_STEP_DATA_GAIN_DEP_VOLTBL:
            *pfnRequested_Operation_Function = set_vocproc_step_data_gain_dep_voltbl;
            break;
        case ACPH_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL:
            *pfnRequested_Operation_Function = get_audproc_gain_dep_voltbl;
            break;
        case ACPH_CMD_GET_AUDPROC_POPP_GAIN_DEP_VOLTBL_STEP:
            *pfnRequested_Operation_Function = get_audproc_popp_gain_dep_voltbl_step;
            break;
        case ACPH_CMD_GET_AUDPROC_COPP_GAIN_DEP_VOLTBL_STEP:
            *pfnRequested_Operation_Function = get_audproc_copp_gain_dep_voltbl_step;
            break;
        case ACPH_CMD_GET_AUDPROC_STEP_DATA_GAIN_DEP_VOLTBL:
            *pfnRequested_Operation_Function = get_audproc_step_data_gain_dep_voltbl;
            break;
        case ACPH_CMD_SET_AUDPROC_STEP_DATA_GAIN_DEP_VOLTBL:
            *pfnRequested_Operation_Function = set_audproc_step_data_gain_dep_voltbl;
            break;
        case ACPH_CMD_GET_VOL_TABLE_STEP_SIZE:
            *pfnRequested_Operation_Function = get_vol_table_step_size;
            break;
        case ACPH_CMD_GET_DEVICE_PAIR_LIST:
            *pfnRequested_Operation_Function = get_device_pair_list;
            break;
        case ACPH_CMD_GET_DATE_INFO:
            *pfnRequested_Operation_Function = get_date_info;
            break;
        case ACPH_CMD_GET_OEM_INFO:
            *pfnRequested_Operation_Function = get_oem_info;
            break;
        case ACPH_CMD_GET_ANC_TX_DEVICE:
            *pfnRequested_Operation_Function = get_anc_tx_device;
            break;
        case ACPH_CMD_GET_ANC_DEVICE_PAIR_LIST:
            *pfnRequested_Operation_Function = get_anc_device_pair_list;
            break;
        case ACPH_CMD_GET_ADIE_CODEC_PATH_PROFILE:
            *pfnRequested_Operation_Function = get_adie_codec_path_profile;
            break;
        case ACPH_CMD_SET_ADIE_CODEC_PATH_PROFILE:
            *pfnRequested_Operation_Function = set_adie_codec_path_profile;
            break;
        case ACPH_CMD_GET_ANC_SETTING:
            *pfnRequested_Operation_Function = get_anc_setting;
            break;
        case ACPH_CMD_SET_ANC_SETTING:
            *pfnRequested_Operation_Function = set_anc_setting;
            break;
        case ACPH_CMD_CHECK_CONNECTION:
            *pfnRequested_Operation_Function = check_connection;
            break;
        case ACPH_CMD_GET_LOOKUP_TABLE_SIZE:
            *pfnRequested_Operation_Function = get_lookup_table_size;
            break;
        case ACPH_CMD_GET_TABLE_INDEX_COMBINATION:
            *pfnRequested_Operation_Function = get_table_index_combination;
            break;
        case ACPH_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID:
            *pfnRequested_Operation_Function = get_audproc_common_topology_id;
            break;
        case ACPH_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID:
            *pfnRequested_Operation_Function = get_audproc_stream_topology_id;
            break;
        case ACPH_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST:
            *pfnRequested_Operation_Function = get_audproc_common_topology_id_list;
            break;
        case ACPH_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST:
            *pfnRequested_Operation_Function = get_audproc_stream_topology_id_list;
            break;
        case ACPH_CMD_GET_AFE_DATA:
            *pfnRequested_Operation_Function = get_afe_data;
            break;
        case ACPH_CMD_SET_AFE_DATA:
            *pfnRequested_Operation_Function = set_afe_data;
            break;
        case ACPH_CMD_QUERY_MAX_BUFFER_LENGTH:
            *pfnRequested_Operation_Function = query_max_buffer_length;
            break;

        //Adie RTC
        case ACPH_CMD_SET_ADIE_REGISTER:
            *pfnRequested_Operation_Function = set_adie_register;
            break;
        case ACPH_CMD_GET_ADIE_REGISTER:
            *pfnRequested_Operation_Function = get_adie_register;
            break;
        case ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS:
            *pfnRequested_Operation_Function = set_multiple_adie_registers;
            break;
        case ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS:
            *pfnRequested_Operation_Function = get_multiple_adie_registers;
            break;

        //Audio and Voice RTC
        case ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES:
            *pfnRequested_Operation_Function = query_aud_topology_copp_handles;
            break;
        case ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES:
            *pfnRequested_Operation_Function = query_aud_copp_stream_handles;
            break;
        case ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS:
            *pfnRequested_Operation_Function = query_voc_all_active_streams;
            break;
        case ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES:
            *pfnRequested_Operation_Function = query_voc_vs_copp_handles;
            break;
        case ACPH_CMD_QUERY_VOC_VC_TOPOLOGY:
            *pfnRequested_Operation_Function = query_voc_vc_topology;
            break;
        case ACPH_CMD_RTC_GET_CAL_DATA:
            *pfnRequested_Operation_Function = rtc_get_cal_data;
            break;
        case ACPH_CMD_RTC_SET_CAL_DATA:
            *pfnRequested_Operation_Function = rtc_set_cal_data;
            break;

        //Future Command
        case ACPH_CMD_RTC_GET_CAL_DATA_SHMEM:
            *pfnRequested_Operation_Function = rtc_get_cal_data_shmem;
            break;
        case ACPH_CMD_RTC_SET_CAL_DATA_SHMEM:
            *pfnRequested_Operation_Function = rtc_set_cal_data_shmem;
            break;
        case ACPH_CMD_RTC_TRANS_APR_PKT:
            *pfnRequested_Operation_Function = rtc_trans_apr_pkt;
            break;
        case ACPH_CMD_RTC_TRANS_APR_GET_PKT_SHMEM:
            *pfnRequested_Operation_Function = rtc_trans_apr_get_pkt_shmem;
            break;
        case ACPH_CMD_RTC_TRANS_APR_SET_PKT_SHMEM:
            *pfnRequested_Operation_Function = rtc_trans_apr_set_pkt_shmem;
            break;
        default:
            bResult = FALSE;
            break;
    }

    LOGE("[ACDB]->Get Command ID [%08x], result [%d]\n",*command_id, bResult);
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
    int16_t nCommand = ACPH_CMD_ADIE_DAL_ATTACH;
    if (NULL == acph_main_buffer)
    {
        acph_main_buffer = (char_t *) malloc(ACPH_ACDB_BUFFER_POSITION + ACPH_BUFFER_LENGTH);
        acph_sub_buffer = (char_t *) malloc(ACPH_HALF_BUF_LEN);
        if (NULL == acph_main_buffer || NULL == acph_sub_buffer)
        {
            /**malloc failed*/
            result = ACPH_FAILURE;
        }
        else
        {
            //result = adie_execute_command(&nCommand,NULL);
            (void)adie_execute_command(&nCommand,NULL);
        }
    }
    if (ACPH_SUCCESS == result)
    {
       result = rtc_q6_intf_init();
       LOGE("[ACDB RTC]->rtc init done!->result [%d]\n",result);
    }
    else
    {
       LOGE("[ACDB ACPH]->acph init failed!->result [%d]\n",result);
    }
    
    actp_diag_init (acph_execute_command);
    LOGE("[ACDB ACPH]->actp diag init done!");
    
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
    int16_t nCommand = ACPH_CMD_ADIE_DAL_DETACH;
    if (NULL != acph_main_buffer)
    {
        free(acph_main_buffer);
        acph_main_buffer = NULL;
    }
    if (NULL != acph_sub_buffer)
    {
        free(acph_sub_buffer);
        acph_sub_buffer = NULL;
    }
    //result = adie_execute_command(&nCommand,NULL);
    (void)adie_execute_command(&nCommand,NULL);
    if (ACPH_SUCCESS == result)
    {
        result = rtc_q6_intf_deinit();
    }
    else
    {
        (void) rtc_q6_intf_deinit();
    }
    return result;
}

/**
 * FUNCTION : acph_execute_command
 *
 * DESCRIPTION : Interpret request and operate ACDB correspondingly, response with 
 * the result.
 *
 * DEPENDENCIES : ACDB needs to be initialized before this function is called
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



