#ifndef _AUDCAL_ACPH_H_
#define _AUDCAL_ACPH_H_
/** 
  \file **************************************************************************
 *
 *                                       A C P H   H E A D E R    F I L E
 *
 *DESCRIPTION
 * This header file contains all the definitions necessary for Audio Calibration
 * Packet Handler to handle request buffer and operate ACDB 
 * This acph works only in ARM9
 *  
 * Copyright (c) 2010-2012 by Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */
/*
   --------------------
   |include files                |
   --------------------
   */
#include "acdb_includes.h"

#define ACPH_SUCCESS 	0
#define ACPH_FAILURE	(-1)

/*------------------------------------------
 ** flag, page size, and buffer length definition
 *-------------------------------------------*/
#define ACPH_SUC_FLAG_TRUE          0x1
#define ACPH_SUC_FLAG_FALSE         0x0

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
#define ACPH_APR_MIN_HEADER_SIZE    20

/*------------------------------------------
 ** COMMAND ID definitions 2 char_t
 *-------------------------------------------*/

//Online Calibration
#define ACPH_CMD_GET_TARGET_VERSION                0x0001
#define ACPH_CMD_GET_ACDB_VERSION                  0x0002
#define ACPH_CMD_ACDB_INIT                         0x0003
#define ACPH_CMD_ACDB_RESET                        0x0004
#define ACPH_CMD_ESTIMATE_MEMORY_USE               0x0005
#define ACPH_CMD_GET_DEVICE_INFO                   0x0006
#define ACPH_CMD_GET_DEVICE_CAPABILITIES           0x0007
#define ACPH_CMD_IS_DEVICE_PAIRED                  0x0008   //Not used in QACT
#define ACPH_CMD_GET_VOCPROC_COMMON_DATA           0x0009
#define ACPH_CMD_SET_VOCPROC_COMMON_DATA           0x000A
#define ACPH_CMD_GET_VOCPROC_COMMON_TABLE          0x000B
#define ACPH_CMD_SET_VOCPROC_COMMON_TABLE          0x000C
#define ACPH_CMD_GET_VOCPROC_STREAM_DATA           0x000D
#define ACPH_CMD_SET_VOCPROC_STREAM_DATA           0x000E
#define ACPH_CMD_GET_VOCPROC_STREAM_TABLE          0x000F
#define ACPH_CMD_SET_VOCPROC_STREAM_TABLE          0x0010
#define ACPH_CMD_GET_AUDPROC_COMMON_DATA           0x0011
#define ACPH_CMD_SET_AUDPROC_COMMON_DATA           0x0012
#define ACPH_CMD_GET_AUDPROC_COMMON_TABLE          0x0013
#define ACPH_CMD_SET_AUDPROC_COMMON_TABLE          0x0014
#define ACPH_CMD_GET_AUDPROC_STREAM_DATA           0x0015
#define ACPH_CMD_SET_AUDPROC_STREAM_DATA           0x0016
#define ACPH_CMD_GET_AUDPROC_STREAM_TABLE          0x0017
#define ACPH_CMD_SET_AUDPROC_STREAM_TABLE          0x0018

#define ACPH_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL            0x0019
#define ACPH_CMD_GET_VOCPROC_STEP_DATA_GAIN_DEP_VOLTBL  0x001A
#define ACPH_CMD_SET_VOCPROC_STEP_DATA_GAIN_DEP_VOLTBL  0x001B
#define ACPH_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL            0x001C
#define ACPH_CMD_GET_AUDPROC_POPP_GAIN_DEP_VOLTBL_STEP  0x001D
#define ACPH_CMD_GET_AUDPROC_COPP_GAIN_DEP_VOLTBL_STEP  0x001E
#define ACPH_CMD_GET_AUDPROC_STEP_DATA_GAIN_DEP_VOLTBL  0x001F
#define ACPH_CMD_SET_AUDPROC_STEP_DATA_GAIN_DEP_VOLTBL  0x0020

#define ACPH_CMD_GET_VOL_TABLE_STEP_SIZE           0x0021
#define ACPH_CMD_GET_DEVICE_PAIR_LIST              0x0022
#define ACPH_CMD_GET_DATE_INFO                     0x0023
#define ACPH_CMD_GET_OEM_INFO                      0x0024
#define ACPH_CMD_GET_ANC_TX_DEVICE                 0x0025   //Not used in QACT
#define ACPH_CMD_GET_ANC_DEVICE_PAIR_LIST          0x0026

#define ACPH_CMD_GET_ADIE_CODEC_PATH_PROFILE       0x0027
#define ACPH_CMD_SET_ADIE_CODEC_PATH_PROFILE       0x0028
#define ACPH_CMD_GET_ANC_SETTING                   0x0029
#define ACPH_CMD_SET_ANC_SETTING                   0x002A

#define ACPH_CMD_CHECK_CONNECTION                  0x0030
#define ACPH_CMD_GET_LOOKUP_TABLE_SIZE             0x0031
#define ACPH_CMD_GET_TABLE_INDEX_COMBINATION       0x0032
#define ACPH_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID         0x0033    //Not used in QACT
#define ACPH_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID         0x0034    //Not used in QACT
#define ACPH_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST    0x0035
#define ACPH_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST    0x0036

#define ACPH_CMD_GET_AFE_DATA                      0x0037
#define ACPH_CMD_SET_AFE_DATA                      0x0038
#define ACPH_CMD_GET_GLBTBL_DATA                   0x0039
#define ACPH_CMD_SET_GLBTBL_DATA                   0x003A

//new 8960 online interface
#define ACPH_CMD_GET_COMMON_DEVICE_INFO                   0x003B
#define ACPH_CMD_GET_TARGET_SPECIFIC_DEVICE_INFO          0x003C
#define ACPH_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID           0x003D
#define ACPH_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID_LIST      0x003E
#define ACPH_CMD_GET_QACT_INFO                            0x003F
#define ACPH_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID  		  0x0040
#define ACPH_CMD_GET_REC_DEVICE_PAIR_LIST  		  0x0050

#define ACPH_CMD_GET_AFE_COMMON_DATA           0x0054
#define ACPH_CMD_SET_AFE_COMMON_DATA           0x0055
#define ACPH_CMD_GET_AFE_COMMON_TABLE          0x0056
#define ACPH_CMD_SET_AFE_COMMON_TABLE          0x0057
#define ACPH_CMD_GET_AFE_TOPOLOGY_LIST         0x0058
#define ACPH_CMD_GET_AUD_VOL_POPP_MODULE_LIST         0x0059

#define ACPH_CMD_GET_DEVICE_CHANNEL_TYPE_LIST         0x005A

#define ACPH_CMD_QUERY_MAX_BUFFER_LENGTH           0x00D0

//Adie RTC
#define ACPH_CMD_SET_ADIE_REGISTER                 0x00A0
#define ACPH_CMD_GET_ADIE_REGISTER                 0x00A1
#define ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS       0x00A2
#define ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS       0x00A3
#define ACPH_CMD_ADIE_DAL_ATTACH                   0x00A4
#define ACPH_CMD_ADIE_DAL_DETACH                   0x00A5
#define ACPH_CMD_GET_ADIE_SEQUENCE_DATA            0x00A6
#define ACPH_CMD_SET_ADIE_SEQUENCE_DATA            0x00A7

#define ACPH_CMD_GET_PMIC_DATA                     0x00A8
#define ACPH_CMD_SET_PMIC_DATA                     0x00A9

//RTC command Id
#define ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES     0x00C1
#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES     0x00C2
#define ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS      0x00C3
#define ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES         0x00C4
#define ACPH_CMD_QUERY_VOC_VC_DEVICES              0x00C5
#define ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES   0x00C6
#define ACPH_CMD_QUERY_VOC_VC_TOPOLOGY             0x00C7

#define ACPH_CMD_RTC_GET_CAL_DATA                  0x0041
#define ACPH_CMD_RTC_SET_CAL_DATA                  0x0042

/* --------------------------------------------------------
//Future Commands
-----------------------------------------------------------*/
//Online Calibration
#define ACPH_CMD_GET_GLBTBL_DATA                   0x0039
#define ACPH_CMD_SET_GLBTBL_DATA                   0x003A

//Audio and Voice RTC
#define ACPH_CMD_RTC_GET_CAL_DATA_SHMEM            0x0043
#define ACPH_CMD_RTC_SET_CAL_DATA_SHMEM            0x0044

#define ACPH_CMD_RTC_TRANS_APR_PKT                 0x0051
#define ACPH_CMD_RTC_TRANS_APR_SET_PKT_SHMEM       0x0052
#define ACPH_CMD_RTC_TRANS_APR_GET_PKT_SHMEM       0x0053

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
#define ACPH_ERR_ADIE_INIT_FAILURE        0x00000010
#define ACPH_ERR_ADIE_SET_CMD_FAILURE     0x00000011
#define ACPH_ERR_ADIE_GET_CMD_FAILURE     0x00000012

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

#endif //_AUDCAL_ACPH_H_

