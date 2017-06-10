#ifndef _AUDCAL_ACPH7x30_H_
#define _AUDCAL_ACPH7x30_H_
/**
  \file **************************************************************************
 *
 *                                       A C P H   H E A D E R    F I L E
 *
 *DESCRIPTION
 * This header file contains all the definitions necessary for handling commands from PC
 *
 *Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 *Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */
/*
   --------------------
   |include files                |
   --------------------
   */
#include "acdb_includes.h"

#define ACPH_SUCCESS    0
#define ACPH_FAILURE    (-1)

/*------------------------------------------
 ** flag, page size, and buffer length definition
 *-------------------------------------------*/
#define ACPH_SUC_FLAG_TRUE          0x1
#define ACPH_SUC_FLAG_FALSE         0x0
#define ACPH_BUFFER_LENGTH          0x1000
#define ACPH_HEADER_LENGTH          6
#define ACPH_COMMAND_ID_POSITION    0
#define ACPH_COMMAND_ID_LENGTH      2
#define ACPH_DATA_LENGTH_POSITION   2
#define ACPH_DATA_LENGTH_LENGTH     4
#define ACPH_ERROR_FRAME_LENGTH     11
#define ACPH_ERROR_FLAG_LENGTH      1
#define ACPH_SUC_FLAG_LENGTH        1
#define ACPH_ERROR_CODE_LENGTH      4
#define ACPH_CAL_DATA_UNIT_LENGTH   4
#define ACPH_ACDB_BUFFER_POSITION   7

/*------------------------------------------
 ** COMMAND ID definitions 2 char_t
 *-------------------------------------------*/

#define ACPH_CMD_GET_ACTIVE_DEVICELIST        0x0021
#define ACPH_CMD_RTC_GET_CAL_DATA             0x0022
#define ACPH_CMD_RTC_SET_CAL_DATA             0x0023

/**to add, delete, or modify COMMAND ID, you need to change the switch case in
  function get_command_id*/

/*------------------------------------------
 ** ERROR CODE definitions 4 char_t
 *-------------------------------------------*/
#define ACPH_ERR_UNKNOWN_REASON           0x00000001
#define ACPH_ERR_INVALID_COMMAND          0x00000002
#define ACPH_ERR_INVALID_TARGET_VERSION   0x00000003
#define ACPH_ERR_LENGTH_NOT_MATCH         0x00000004
#define ACPH_ERR_INVALID_DEVICE_ID        0x00000005
#define ACPH_ERR_INVALID_BLOCK_ID         0x00000006
#define ACPH_ERR_INVALID_INTERFACE_ID     0x00000007
#define ACPH_ERR_INVALID_NETWORK_ID       0x00000008
#define ACPH_ERR_INVALID_SAMPLE_RATE_ID   0x00000009
#define ACPH_ERR_ACDB_COMMAND_FAILURE     0x0000000A
#define ACPH_ERR_CSD_AUD_CMD_FAILURE      0x0000000B
#define ACPH_ERR_CSD_VOC_CMD_FAILURE      0x0000000C
#define ACPH_ERR_APR_DSP_CMD_FAILURE      0x0000000D
#define ACPH_ERR_CSD_OPEN_HANDLE          0x0000000E
#define ACPH_ERR_OUT_OF_BUFFER_SIZE       0x0000000F
#define ACPH_ERR_CSD_WRITE_FAILURE        0x00000010

#define ACPH_MAX_ACTIVE_ENTRIES           0x30

/*
   --------------------
   | External functions |
   --------------------
   */
/**
 * FUNCTION : acph_init
 *
 * DESCRIPTION : Initilize ACPH. Allocate memory.
 *
 * DEPENDENCIES : None
 *
 * RETURN VALUE : ACPH_SUCCESS when success, ACPH_ERROR otherwise
 *
 * SIDE EFFECTS : None
 */
int32_t acph_init(void);

/**
 * FUNCTION : acph_deinit
 *
 * DESCRIPTION : Deinitilize ACPH. Free memory.
 *
 * DEPENDENCIES : None
 *
 * RETURN VALUE : ACPH_SUCCESS when success, ACPH_ERROR otherwise
 *
 * SIDE EFFECTS : None
 */
int32_t acph_deinit(void);

/**
 * FUNCTION : acph_execute_command
 *
 * DESCRIPTION : Interpret request and operate ACDB correspondingly, response with
 * the result.
 *
 * DEPENDENCIES : ACDB needs to be initialized before this function is called
 *
 * PARAMS:
 *   req_buf_ptr - the ACPH request command buffer
 *   req_buf_length - the lenght of request command buffer
 *   resp_buf_ptr - pointer of pointer to the response buffer
 *   resp_buf_length - pointer to the length of response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void acph_execute_command(
        char_t *req_buf_ptr,
        uint32_t req_buf_length,
        char_t **resp_buf_ptr,
        uint32_t *resp_buf_length
        );

/**
 * FUNCTION : acph_register_callback
 *
 * DESCRIPTION : register callback function such as audio query API
 *
 * DEPENDENCIES : ACDB needs to be initialized before this function is called
 *
 * PARAMS:
 *   cb_type - the callback function type 1:audio, 2:voice
 *   cb_ptr - the callback function pointer
 *
 * RETURN VALUE : 0 - success, 1 - failure
 *
 * SIDE EFFECTS : None
 */
int32_t acph_register_callback(
        uint32_t cb_type,
        uint8_t *cb_ptr
        );


#endif //_AUDCAL_ACPH7x30_H_