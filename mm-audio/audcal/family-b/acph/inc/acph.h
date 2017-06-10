#ifndef _AUDCAL_ACPH_H_
#define _AUDCAL_ACPH_H_
/****************************************************************************
  @file achp.h
 *
 *                                       A C P H   H E A D E R    F I L E
 *
 *DESCRIPTION
 * This header file contains all the definitions necessary for Audio Calibration
 * Packet Handler to handle request buffer and operate ACDB
 * This acph works only in ARM9
 */
 /*===========================================================================
      NOTE: The description above does not appear in the PDF.

      The acdb_mainpage.dox file contains all file/group descriptions that are
      in the output PDF generated using Doxygen and Latex. To edit or update
      any of the file/group text in the PDF, edit the acdb_mainpage.dox file
      (contact Tech Pubs).
===========================================================================*/

/*===========================================================================

                    Copyright (c) 2010, 2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Confidential and Proprietary - Qualcomm Technologies, Inc.
========================================================================== */

/*
 *                      EDIT HISTORY FOR FILE
 *
 *  This section contains comments describing changes made to this file.
 *  Notice that changes are listed in reverse chronological order.
 *
 *  $Header:
 *
 *when         who     what, where, why
 *--------   ---     ----------------------------------------------------------
 *02/14/14   avi     Support commands for ACDB persistence.
 *12/25/13   mh      Corrected compilation errors
 *08/02/13   leo     (Tech Pubs) Edited Doxygen comment/markup for new cmds.
 *06/10/13   nvd     added new acph commands under dsprtc and adie rtc service
 *06/07/13   nvd     added new acph commands under dsprtc service
 *06/07/13   mh      Corrected checkpatch errors
 *02/01/13   leo     (Tech Pubs) Edited/added Doxygen comments and markup.
 *05/28/10   ayin     initial draft
 ********************************************************************************
 */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acph/inc/acph.h#13 $ */
/*
   --------------------
   |include files                |
   --------------------
   */
#include "acdb_os_includes.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

//#define ACPH_BUFFER_LENGTH 0x2500

/** @addtogroup acph_err_codes
@{ */

/*------------------------------------------
 ** ERROR CODE definitions 4 char_t
 *-------------------------------------------*/
/** Operation completed successfully. */
#define ACPH_SUCCESS    0

/** Indicates that a failure has occured. */
#define ACPH_FAILURE    (-1)

/** Failure reason unknown. */
#define ACPH_ERR_UNKNOWN_REASON           0x00000001

/** Invalid command. */
#define ACPH_ERR_INVALID_COMMAND          0x00000002

/** Invalid target version. */
#define ACPH_ERR_INVALID_TARGET_VERSION   0x00000003

/** Data length does not match the expected length. */
#define ACPH_ERR_LENGTH_NOT_MATCH         0x00000004

/** Invalid device ID. */
#define ACPH_ERR_INVALID_DEVICE_ID        0x00000005

/** Invalid block ID. */
#define ACPH_ERR_INVALID_BLOCK_ID         0x00000006

/** Invalid interface ID. */
#define ACPH_ERR_INVALID_INTERFACE_ID     0x00000007

/** Invalid network ID. */
#define ACPH_ERR_INVALID_NETWORK_ID       0x00000008

/** Invalid sample rate ID. */
#define ACPH_ERR_INVALID_SAMPLE_RATE_ID   0x00000009

/** ACDB command failure. */
#define ACPH_ERR_ACDB_COMMAND_FAILURE     0x0000000A

/** Core Sound Driver (CSD) audio command failure. */
#define ACPH_ERR_CSD_AUD_CMD_FAILURE      0x0000000B

/** CSD voice command failure. */
#define ACPH_ERR_CSD_VOC_CMD_FAILURE      0x0000000C

/** Aysnchronous Packet Router (APR) DSP command failure. */
#define ACPH_ERR_APR_DSP_CMD_FAILURE      0x0000000D

/** Error due to a CSD open handle. */
#define ACPH_ERR_CSD_OPEN_HANDLE          0x0000000E

/** Insufficient buffer size. */
#define ACPH_ERR_OUT_OF_BUFFER_SIZE       0x0000000F

/** ADIE initiation failure. */
#define ACPH_ERR_ADIE_INIT_FAILURE        0x00000010

/** ADIE Set command failure. */
#define ACPH_ERR_ADIE_SET_CMD_FAILURE     0x00000011

/** ADIE Get command failure. */
#define ACPH_ERR_ADIE_GET_CMD_FAILURE     0x00000012

/** Invalid service ID. */
#define ACPH_ERR_INVALID_SERVICE_ID       0x00000013

/** Module is not enabled. */
#define ACPH_ERR_MODULE_DISABLED          0x00000014

/** If the file path is invalid. */
#define ACPH_ERR_INVALID_FILE_PATH        0x00000015

/** If the invalid record duration is provided. */
#define ACPH_ERR_INVALID_REC_DURATION     0x00000016

/** If there is no playback or rec session initiated by media control service. */
#define ACPH_ERR_NO_ACTIVE_SESSION_FOUND  0x00000017

/** If there is already a playback or record or playback and record session initiated by media control service. */
#define ACPH_ERR_ACTIVE_SESSION_FOUND     0x00000018

/** @} */ /* end_addtogroup acph_err_codes */

#define ACPH_FILENAME_MAX_CHARS          256

/** Enum type to specify the playback modes.
 */
enum AcphPlaybackModes {
   ACPH_PLAYBACK_MODE_REGULAR  = 1,
     /**< Specifies that the playback mode is a regular playback*/
   ACPH_PLAYBACK_MODE_ANC,
     /**< Specifies that the playback mode is ANC playback*/
	 };

/*--------------------------------------------------
 ** ACPH Online Service and Command range declarations
 *-------------------------------------------------*/

/** @addtogroup online_serv_defs
@{ */

/** Online registration service ID definition. Command IDs for the Online
    service range from 1 to 250. */
#define ACPH_ONLINE_REG_SERVICEID 0x00000001

// Command IDs of value from 1 to 250 should be used only by Online service

/** Online command ID start value = 1. */
#define ACPH_ONLINE_CMD_ID_START  0x0001 //1
/** Online command ID end value = 250. */
#define ACPH_ONLINE_CMD_ID_END    0x00FA //250

/** @} */ /* end_addtogroup online_serv_defs */

/*------------------------------------------
 ** COMMAND ID definitions 2 char_t
 *-------------------------------------------*/
//Online Calibration Command Ids

/** @ingroup get_tgt_ver
Queries for the target version ID.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_TARGET_VERSION.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to zero.
@param[in,out] resp_buf_ptr Pointer to the response buffer.
               On successful execution of this function, the response buffer
               is filled in this format:
@verbatim
 struct RespPktType
 {
  uint32_t tgtvId;  // Target version ID
 } @endverbatim
@param[in] resp_buf_len Size of the response buffer.
@param[out] resp_buf_bytes_filled Equals the size of RespPktType on successful
                                  execution.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_GET_TARGET_VERSION                0x0001

/** @ingroup check_tgt_conn
Checks the target connection.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_CHECK_CONNECTION.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to zero.
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACPH_SUCCESS -- Command executed successfully.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_CHECK_CONNECTION                  0x0002

/** @ingroup get_acdb_files_info
Queries for the ACDB files information.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_ACDB_FILES_INFO.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to zero.
@param[in,out] resp_buf_ptr Response buffer where the output is filled.
			   If the function call is successful, the response buffer is
               filled in this format:
@verbatim
 struct AcdbFilesInfo
 {
   uint32_t noOfFiles;
   FileInfo finf[];
 }
 struct FileInfo
 {
   uint32_t fileLen;
   uint32_t fileNameLen;
   uint8_t  fileName[fileNameLen];
 } @endverbatim
@param[in] resp_buf_len Length of the output buffer passed.
@param[out] resp_buf_bytes_filled Valid output buffer length.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.
@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_GET_ACDB_FILES_INFO                       0x0003

/** @ingroup get_max_buf_len
Queries for the maximum buffer length.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_MAX_BUFFER_LENGTH.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to zero.
@param[in,out] resp_buf_ptr Response buffer where the output is filled.
			   If the function call is successful, the response buffer is
               filled in this format:
@verbatim
 struct RespPktType
 {
   uint32_t maxRespLen; // Maximum response buffer
                           size
 } @endverbatim
@param[in] resp_buf_len Length of the output buffer passed.
@param[out] resp_buf_bytes_filled Valid output buffer length.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.
@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_QUERY_MAX_BUFFER_LENGTH           0x0004

/** @ingroup get_acdb_file
Queries for the ACDB file associated with the current target.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_ACDB_FILE.
@param[in] req_buf_ptr Request buffer pointer. The request buffer is filled in
                       this format:
@verbatim
 struct ReqPktType
 {
   FileDataReqType fdr; // Request command
                           structure
 }
 struct FileDataReqType
 {
   uint32_t fileStartOffset;   // Start position
                                  in the file from
                                  where the data is
                                  to be copied
   uint32_t fileDataLen;       // Length of the
                                  requested file
   uint32_t fileNameLen;       // File name length
   uint8_t data[fileNameLen];  // File name
 } @endverbatim
@param[in] req_buf_len Length of the request buffer.
@param[in,out]  resp_buf_ptr Response buffer where the output is filled.
			    If the function call is successful, the response buffer is
                filled in this format:
@verbatim
 struct RespPktType
 {
   byte[] fileData; // No. of bytes from the file
                       specified in the requested
                       buffer
 } @endverbatim
@param[in] resp_buf_len Length of the output buffer passed.
@param[out] resp_buf_bytes_filled Valid output buffer length.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.
@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_GET_ACDB_FILE                         0x0005

/** @ingroup get_no_tbl_entries_on_heap
Queries for the number of table entries on the heap.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_NO_OF_TBL_ENTRIES_ON_HEAP.
@param[in] req_buf_ptr Request buffer pointer. The request buffer is filled in
                       this format:
@verbatim
 struct ReqPktType
 {
   uint32_t tblId; // Table ID for which the number
                      of entries is required
 }
@endverbatim
@param[in] req_buf_len Length of the request buffer.
@param[in,out]  resp_buf_ptr Response buffer where the output is filled.
			    If the function call is successful, the response buffer is
                filled in this format:
@verbatim
 struct RespPktType
 {
   uint32_t noOfHeapEntries;  // Number of heap
                                 entries for the
                                 given table
 }
@endverbatim

@param[in] resp_buf_len Length of the output buffer passed.
@param[out] resp_buf_bytes_filled Valid output buffer length.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.
@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_GET_NO_OF_TBL_ENTRIES_ON_HEAP     0x0006

/** @ingroup get_tbl_entries_on_heap
Queries for the table entries on the heap.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_TBL_ENTRIES_ON_HEAP.
@param[in] req_buf_ptr Request buffer pointer. The request buffer is filled in
                       this format:
@verbatim
 struct ReqPktType
 {
   uint32_t tblId; // Table ID for which the number
                      of entries is required
   uint32_t entriesStartOffset; // Offset from where
                                   the entries are
                                   to be provided
   }
@endverbatim
@param[in] req_buf_len Length of the request buffer
@param[in,out]  resp_buf_ptr Response buffer where the output is filled.
			    If the function call is successful, the response buffer is
                filled in this format:
@verbatim
 struct RespPktType
 {
   uint32_t noOfEntriesCopied;  // Number of heap
                                   entries copied
                                   in the response
   uint8_t data[];   // Lookup table entries based
                        on the table ID, including
                        MID and PID
 }
@endverbatim
@param[in] resp_buf_len Length of the output buffer passed.
@param[out] resp_buf_bytes_filled Valid output buffer length.

@newpage
@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.
@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_GET_TBL_ENTRIES_ON_HEAP               0x0007

/** @ingroup get_acdb_data
Queries for the data for the specified lookup table and MID, PID values.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_ACDB_DATA.
@param[in] req_buf_ptr Request buffer pointer. The request buffer is filled
                       in this format:
@verbatim
 struct ReqPktType
 {
   uint32_t tblId; // Table ID for which the number
                      of entries is required
   uint32_t[] indices; // Indices, including MID and
                          PID, or only PID, based
                          on the table for which
                          the data is being
                          requested
   }
@endverbatim
@param[in] req_buf_len Length of the request buffer
@param[in,out]  resp_buf_ptr Response buffer where the output is filled.
			    If the function call is successful, the response buffer is
                filled in this format:
@verbatim
 struct RespPktType
 {
   uint8_t data[]; // Data for which the request
                      is made
 }
@endverbatim
@param[in] resp_buf_len Length of the output buffer passed.
@param[out] resp_buf_bytes_filled Valid output buffer length.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.
@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_GET_ACDB_DATA                         0x0008

/** @ingroup set_acdb_data
Queries for setting the data for the specified lookup table and MID, PID values.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_SET_ACDB_DATA.
@param[in] req_buf_ptr Request buffer pointer. The request buffer is
                       filled in this format:
@verbatim
 struct ReqPktType
 {
   uint32_t tblId; // Table ID for which the number
                      of entries is required
   uint32_t indices[]; // Indices, including MID and
                          PID, or only PID, based on
                          the table for which the
                          data is being requested
   uint8_t data[];
 }
@endverbatim

@param[in] req_buf_len Length of the request buffer.
@param[in,out]  resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERROR -- Command failed.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_SET_ACDB_DATA                         0x0009

/** @addtogroup get_online_ver
@{ */

/**
Queries for the version number of the online service.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_ONLINE_VERSION.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to zero.
@param[in,out] resp_buf_ptr Pointer to the response buffer.
               On successful execution of the function, the response buffer
               stuct ACPH_CMD_QUERY_ONLINE_VERSION_rsp
               is filled in this format:
@verbatim
 struct ACPH_CMD_QUERY_ONLINE_VERSION_rsp
 {
   uint32_t  online_major_version;
   uint32_t  online_minor_version;
 }
@endverbatim
@param[in] resp_buf_len Size of the response buffer.
@param[out] resp_buf_bytes_filled Equals the size of RespPktType on
                                  successful execution.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Response buffer provided is insufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_ONLINE_VERSION                         0x000A

typedef struct ACPH_CMD_QUERY_ONLINE_VERSION_rsp ACPH_CMD_QUERY_ONLINE_VERSION_rsp;

#include "acdb_begin_pack.h"

/** Response structure for the online version query.
*/
struct  ACPH_CMD_QUERY_ONLINE_VERSION_rsp{
      uint32_t  online_major_version;   /**< Online major version value. */
      uint32_t  online_minor_version;   /**< Online minor version value. */
}

#include "acdb_end_pack.h"
;

/** Enum to represent the major version number for online service.
*/
enum  ACPH_SERVICE_MAJOR_VERSION {
   ACPH_SERVICE_MAJOR_VERSION_1 = 1,
};
/** Enum to represent the minor version number for online service.
*/
enum  ACPH_SERVICE_MINOR_VERSION {
   ACPH_SERVICE_MINOR_VERSION_0 = 0,
   ACPH_SERVICE_MINOR_VERSION_1 = 1,
   ACPH_SERVICE_MINOR_VERSION_2 = 2,
   ACPH_SERVICE_MINOR_VERSION_3 = 3,
   ACPH_SERVICE_MINOR_VERSION_4 = 4
};

/** @} */ /* end_addtogroup get_online_ver */

/** @ingroup support_acdb_persistence
Queries for supporting ACDB persistence feature.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_IS_PERSISTENCE_SUPPORTED.
@param[in] req_buf_ptr Request buffer pointer. The request buffer is NULL
@param[in] req_buf_len Length of the input buffer is passed as zero.
@param[in,out]  resp_buf_ptr Response buffer with response code (of size int32_t)
@param[in] resp_buf_len Length of the output buffer is passed as sizeof(int32_t).
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  sizeof(int32_t).

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERROR -- Command failed.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_IS_PERSISTENCE_SUPPORTED                         0x000B

/** @ingroup delete_delta_acdb_file
Queries for deleting delta ACDB files.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_DELETE_DELTA_ACDB_FILES.
@param[in] req_buf_ptr Request buffer pointer. The request buffer is NULL
@param[in] req_buf_len Length of the input buffer is passed as zero.
@param[in,out]  resp_buf_ptr Response buffer with response code (of size int32_t).
@param[in] resp_buf_len Length of the output buffer is passed as sizeof(int32_t).
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  sizeof(int32_t).

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERROR -- Command failed.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR

*/
#define ACPH_CMD_DELETE_DELTA_ACDB_FILES                         0x000C

/*--------------------------------------------------
 ** ACPH DSP RTC Service and Command range declarations
 *-------------------------------------------------*/

/** @addtogroup dsp_rtc_svc_defs
@{ */

/** DSP RTC registration service ID. Command IDs for the DSP RTC
    service range from 251 to 500. */
#define ACPH_DSP_RTC_REG_SERVICEID 0x00000002

// Command IDs of value from 251 to 500 should be used only by DSP RTC service

/** DSP RTC start command ID value = 251. */
#define ACPH_DSP_RTC_CMD_ID_START  0x00FB //251

/** DSP RTC end command ID value = 500. */
#define ACPH_DSP_RTC_CMD_ID_END    0x01F4 //500

/** @} */ /* end_addtogroup dsp_rtc_svc_defs */

//RTC command Id for RTC protocol version 1.0:
//#define ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES     0x00FB // 251 ---used to be 0x00C1, for other PL than LA, to get active devices and COPPs
//#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES     0x00FC // 252 ---used to be 0x00C2
//#define ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS      0x00FD // 253 ---used to be 0x00C3
//#define ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES         0x00FE // 254 ---used to be 0x00C4
//#define ACPH_CMD_QUERY_VOC_VC_DEVICES              0x00FF // 255 ---used to be 0x00C5, for other PLs than LA, to get active device pairs
//#define ACPH_CMD_RTC_GET_CAL_DATA                  0x0100 // 256 ---used to be 0x0041
//#define ACPH_CMD_RTC_SET_CAL_DATA                  0x0101 // 257 ---used to be 0x0042
//#define ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES   0x0102 // 258; for LA, to get active topology and COPPs
//#define ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES           0x0103 // 259; for LA, to get active topology pairs
//#define ACPH_CMD_QUERY_DSP_RTC_VERSION             0x0104 // 260 ---new command

//RTC command Id for RTC protocol version 1.1:
//#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_V2     0x0105

//RTC command Id for RTC protocol version 1.2:
//#define ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS     0x0106
//#define ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID      0x0107
//#define ACPH_GET_AFE_DATA                          0x0108
//#define ACPH_SET_AFE_DATA                          0x0109
//#define ACPH_GET_AFE_SIDETONE_GAIN                 0x010A
//#define ACPH_SET_AFE_SIDETONE_GAIN                 0x010B

//RTC command Id for RTC protocol version 1.3:
//#define ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2     0x010C // 268 --- for other PLs than LA, to get device and app type COPPs
//#define ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2   0x010D // 269 --- for LA, to get active topology, app type and COPPs
//#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_V3     0x010E // 270 --- to get Audio COPP strem handles

/** @addtogroup get_dsp_rtc_ver
@{ */

/**
Queries for the DSP RTC protocol version.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_DSP_RTC_VERSION.
@param[in] req_buf_ptr Set to NULL.
@param[in] req_buf_len Set to 0.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packded struct
                            ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp.
@param[in] resp_buf_len Length of input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_DSP_RTC_VERSION                     0x0104 // 260 ---new command

typedef struct ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the DSP RTC version query.
*/
struct  ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp{
      uint32_t  dsp_rtc_major_version;  /**< DSP RTC major version value. */
      uint32_t  dsp_rtc_minor_version;  /**< DSP RTC minor version value. */
  }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_dsp_rtc_ver */

/** @addtogroup get_aud_dev_copp_hndls
@{ */

/**
Queries for audio device COPP handles.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES.
@param[in] req_buf_ptr Set to NULL.
@param[in] req_buf_len Set to 0.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp +
                                  the size of
                                  AUD_DEVICE_COPP_HANDLE * num_of_entry.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES     0x00FB // 251 ---used to be 0x00C1

typedef struct ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio device COPP handles query.
*/
 struct  ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp{
      uint32_t num_of_entry;   /**< Number of the entry. */
      //struct AUD_DEVICE_COPP_HANDLE[<num_of_entry>];
  }

#include "acdb_end_pack.h"
;

typedef struct AUD_DEVICE_COPP_HANDLE AUD_DEVICE_COPP_HANDLE;
#include "acdb_begin_pack.h"

/** Structure for the audio device COPP handle.
*/
 struct  AUD_DEVICE_COPP_HANDLE{
      uint32_t device_id;    /**< Device ID. */
      uint32_t copp_handle;  /**< COPP handle. */
      uint32_t copp_id;      /**< COPP ID. */
      uint32_t num_of_popps; /**< Number of POPPs. */
  }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_dev_copp_hndls */

/** @addtogroup get_aud_copp_strm_hndls
@{ */

/**
Queries for audio COPP stream handles.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct.
@param[in] req_buf_len Length of the request buffer in bytes.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp +
                                  the size of popp_id * num_of_popps.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES     0x00FC // 252 ---used to be 0x00C2

typedef struct ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req;
#include "acdb_begin_pack.h"

/** Request structure for the audio COPP stream handles query.
*/
 struct   ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req{
      uint32_t copp_handle;   /**< COPP handle. */
      uint32_t num_of_popps;  /**< Number of POPPs. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio COPP stream handles query.
*/
 struct  ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp{
      uint32_t num_of_popp_ids;  /**< Number of POPP IDs. */
      //uint32_t popp_ids[<num_of_popp_ids>];
   }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_copp_strm_hndls */

/** @addtogroup get_voc_all_active_strms
@{ */

/**
Queries for the DSP voice all active streams.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to 0.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp +
                                  the size of
                                  VOC_ALL_ACTIVE_STREAMS * num_of_streams.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS      0x00FD // 253 ---used to be 0x00C3

typedef struct ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the voice all active streams query.
*/
 struct    ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp{
      uint32_t num_of_streams;  /**< Number of streams. */
      //struct VOC_ALL_ACTIVE_STREAMS[<num_of_streams>];
   }
#include "acdb_end_pack.h"
;

typedef struct VOC_ALL_ACTIVE_STREAMS VOC_ALL_ACTIVE_STREAMS;
#include "acdb_begin_pack.h"

/** Structure for the voice all active streams query.
*/
 struct    VOC_ALL_ACTIVE_STREAMS{
      uint32_t popp_id;      /**< POPP ID. */
      uint32_t popp_handle;  /**< POPP handle. */
   }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_voc_all_active_strms */

/** @addtogroup get_voc_vs_copp_hndls
@{ */

/**
Queries for the DSP Voice Stream (VS) COPP handles.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct
                       ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req.
@param[in] req_buf_len Equals the size of ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp +
		                          the size of VOC_VS_COPP_HANDLES * num_of_copps.
@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES         0x00FE // 254 ---used to be 0x00C4
                                                          //

typedef struct ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req;
#include "acdb_begin_pack.h"

/** Request structure for the voice stream COPP handles query.
*/
 struct  ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req{
      uint32_t popp_handle;  /**< POPP handle. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the voice stream COPP handles query.
*/
 struct    ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp{
      uint32_t num_of_copps;  /**< Number of COPPs. */
      //struct VOC_VS_COPP_HANDLES[<num_of_copp >];
   }
#include "acdb_end_pack.h"
;

typedef struct VOC_VS_COPP_HANDLES VOC_VS_COPP_HANDLES;
#include "acdb_begin_pack.h"

/** Structure for the voice stream COPP handles.
*/
 struct    VOC_VS_COPP_HANDLES{
      uint32_t copp_id;      /**< COPP ID. */
      uint32_t copp_handle;  /**< COPP handle. */
   }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_voc_vs_copp_hndls */

/** @addtogroup get_voc_vc_devices
@{ */

/**
Queries for the DSP Voice Context (VC) devices.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_VOC_VC_DEVICES.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct
                       ACPH_CMD_QUERY_VOC_VC_DEVICES_req.
@param[in] req_buf_len Equals the size of ACPH_CMD_QUERY_VOC_VC_DEVICES_req.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp +
                                  the size of
                                  VOC_VC_DEVICES * num_of_device_pairs.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_VOC_VC_DEVICES              0x00FF // 255 ---used to be 0x00C5

typedef struct ACPH_CMD_QUERY_VOC_VC_DEVICES_req ACPH_CMD_QUERY_VOC_VC_DEVICES_req;
#include "acdb_begin_pack.h"

/** Request structure for the voice context devices query.
*/
 struct  ACPH_CMD_QUERY_VOC_VC_DEVICES_req{
      uint32_t copp_handle;  /**< COPP handle. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the voice context devices query.
*/
 struct    ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp{
      uint32_t num_of_device_pairs;  /**< Number of device pairs. */
      //struct VOC_VC_DEVICES[<num_of_device_pairs >];
   }
#include "acdb_end_pack.h"
;

typedef struct VOC_VC_DEVICES VOC_VC_DEVICES;
#include "acdb_begin_pack.h"

/** Structure for the voice context devices.
*/
 struct    VOC_VC_DEVICES{
      uint32_t rx_device_id;  /**< Rx device ID. */
      uint32_t tx_device_id;  /**< Tx device ID. */
   }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_voc_vc_devices */

/** @addtogroup get_rtc_cal_data
@{ */

/**
Queries for the DSP RTC calibration data.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_RTC_GET_CAL_DATA.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct
                       ACPH_CMD_RTC_GET_CAL_DATA_req.
@param[in] req_buf_len: Equals the size of ACPH_CMD_RTC_GET_CAL_DATA_req.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_RTC_GET_CAL_DATA_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_RTC_GET_CAL_DATA_rsp + the
                                  size of uint8 * length.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_RTC_GET_CAL_DATA                  0x0100 // 256 ---used to be 0x0041

typedef struct ACPH_CMD_RTC_GET_CAL_DATA_req ACPH_CMD_RTC_GET_CAL_DATA_req;
#include "acdb_begin_pack.h"

/** Request structure for the RTC calibration data query.
*/
 struct  ACPH_CMD_RTC_GET_CAL_DATA_req{
      uint32_t domain_id;     /**< Domain ID. */
      uint32_t service_id;    /**< Service ID. */
      uint32_t pp_id;         /**< COPP or POPP ID. */
      uint32_t module_id;     /**< Module ID. */
      uint32_t parameter_id;  /**< Parameter ID. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_RTC_GET_CAL_DATA_rsp ACPH_CMD_RTC_GET_CAL_DATA_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the RTC calibration data query.
*/
 struct    ACPH_CMD_RTC_GET_CAL_DATA_rsp{
      uint32_t module_id;     /**< Module ID. */
      uint32_t parameter_id;  /**< Parameter ID. */
      uint16_t length;        /**< Length of the calibration data. */
      uint16_t reserved;      /**< Reserved. */
      //uint8  data[<length>];
   }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_rtc_cal_data */
                                                          //

/** @addtogroup set_rtc_cal_data
@{ */

/**
Sets the DSP RTC calibration data.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId The command ID os ACPH_CMD_RTC_SET_CAL_DATA.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct
                       ACPH_CMD_RTC_SET_CAL_DATA_req.
@param[in] req_buf_len Equals the size of ACPH_CMD_RTC_SET_CAL_DATA_req +
                       the size of uint8 * lengh.
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_RTC_SET_CAL_DATA                  0x0101 // 257 ---used to be 0x0042

typedef struct ACPH_CMD_RTC_SET_CAL_DATA_req ACPH_CMD_RTC_SET_CAL_DATA_req;
#include "acdb_begin_pack.h"

/** Request structure for the Set RTC Calibration Data command.
*/
 struct  ACPH_CMD_RTC_SET_CAL_DATA_req{
      uint32_t domain_id;     /**< Domain ID. */
      uint32_t service_id;    /**< Service ID. */
      uint32_t pp_id;         /**< COPP or POPP ID. */
      uint32_t module_id;     /**< Module ID. */
      uint32_t parameter_id;  /**< Parameter ID. */
      uint16_t length;        /**< Length of the calibration data. */
      uint16_t reserved;      /**< Reserved. */
      //uint8  data[<length>];
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup set_rtc_cal_data */

/** @addtogroup get_aud_topol_copp_hndls
@{ */

/**
Queries for audio topology COPP handles.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to 0.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed structs
                            ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp and
                            AUD_TOPOLOGY_COPP_HANDLES.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp
                                  + the size of AUD_TOPOLOGY_COPP_HANDLES *
                                  num_of_entry.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES 0x0102 // 258; LA-only command

typedef struct ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio topology COPP handles query.
*/
 struct  ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp{
      uint32_t num_of_entry;  /**< Number of COPP handles. */
      //struct AUD_TOPOLOGY_COPP_HANDLES[<num_of_entry>];
   }
#include "acdb_end_pack.h"
;

typedef struct AUD_TOPOLOGY_COPP_HANDLES AUD_TOPOLOGY_COPP_HANDLES;
#include "acdb_begin_pack.h"

/** Structure for the audio topology COPP handles.
*/
 struct  AUD_TOPOLOGY_COPP_HANDLES{
      uint32_t topology_Id;   /**< Topology ID. */
      uint32_t copp_handle;   /**< COPP handle. */
      uint32_t copp_id;       /**< COPP ID. */
      uint32_t num_of_popps;  /**< Number of POPPs. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_topol_copp_hndls */

/** @addtogroup get_voc_vc_topol
@{ */

/**
Queries for voice VC topologies.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct
                       ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req.
@param[in] req_buf_len Length of the request buffer.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed structs
                            ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp and
                            VOC_VC_TOPOLOGIES.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp +
	                              the size of VOC_VC_TOPOLOGIES * num_of_entry.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES 0x0103 // 259; LA-only command

typedef struct ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req;
#include "acdb_begin_pack.h"

/** Request structure for the voice context topologies query.
*/
 struct  ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req{
      uint32_t copp_handle;  /**< COPP handle. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the voice context topologies query.
*/
 struct  ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp{
      uint32_t num_of_entry;  /**< Number of the entry. */
      //struct VOC_VC_TOPOLOGIES[<num_of_entry>];
   }
#include "acdb_end_pack.h"
;

typedef struct VOC_VC_TOPOLOGIES VOC_VC_TOPOLOGIES;
#include "acdb_begin_pack.h"

/** Structure for the voice context topologies.
*/
 struct  VOC_VC_TOPOLOGIES{
      uint32_t rx_topology_id;  /**< Rx topology ID. */
      uint32_t tx_topology_id;  /**< Tx topoloty ID. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_voc_vc_topol */

/** @addtogroup get_aud_copp_strm_hndls_v2
@{ */

/**
Queries for audio COPP stream handles version2.

@cmdversion
Major - 1, Minor - 1

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_V2.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req.
@param[in] req_buf_len Length of the request buffer in bytes (8).
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                        with the serialized (little ENDIAN) packed struct
                        ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v2_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v2_rsp +
                                  the size of Aud_Stream_Popp_Info * num_of_popp_ids.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_V2     0x0105

typedef struct ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v2_rsp ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v2_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio COPP stream handles V2 query.
*/
 struct  ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v2_rsp{
      uint32_t num_of_popp_ids;  /**< Number of POPP IDs. */
	  //followed by:
      //Aud_Stream_Popp_Info popp_info[num_of_popp_ids];
   }
#include "acdb_end_pack.h"
;

#include "acdb_begin_pack.h"

/** Audio stream POPP information.
*/
 struct Aud_Stream_Popp_Info{
    uint32_t popp_id;            /**< POPP ID. */
	uint32_t popp_topology_id;   /**< POPP topology ID. */
 }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_copp_strm_hndls_v2 */

/** @addtogroup get_voc_afe_active_port_ids
@{ */

/**
Gets the voice AFE active port IDs.

@cmdversion
Major - 1, Minor - 2

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_req.
@param[in] req_buf_len Length of the request buffer in bytes (4).
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little ENDIAN) packed struct
                            ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_rsp.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS     0x0106

typedef struct ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_req ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_req;
#include "acdb_begin_pack.h"

/** Request structure for the voice AFE active port IDs query.
*/
 struct  ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_req{
      uint32_t voc_copp_handle;  /**< Voice COPP hanlde. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_rsp ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the voice AFE active port IDs query.
*/
   struct ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_rsp {
      uint32_t tx_afe_port_id;  /**< Tx AFE port ID. */
	  uint32_t rx_afe_port_id;  /**< Rx AFE port ID. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_voc_afe_active_port_ids */

/** @addtogroup get_aud_afe_active_port_ids
@{ */

/**
Gets the audio AFE active port ID.

@cmdversion
Major - 1, Minor - 2

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_req.
@param[in] req_buf_len Length of the request buffer in bytes (4).
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little ENDIAN) packed struct
                            ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_rsp.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID     0x0107

typedef struct ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_req ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_req;
#include "acdb_begin_pack.h"

/** Request structure for the audio AFE active port ID query.
*/
 struct  ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_req{
      uint32_t aud_copp_handle;  /**< Audio COPP handle. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_rsp ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio AFE active port ID query.
*/
   struct ACPH_CMD_QUERY_AUD_AFE_ACTIVE_PORT_ID_rsp {
      uint32_t afe_port_id;  /**< AFE port ID. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_afe_active_port_ids */

/** @addtogroup get_afe_data
@{ */

/**
Gets the AFE calibration data.

@cmdversion
Major - 1, Minor - 2

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_GET_AFE_DATA.
@param[in] req_buf_ptr Input request buffer filled with the serialized (little
                       ENDIAN) packed struct ACPH_GET_AFE_DATA_req.
@param[in] req_buf_len Equals the size of ACPH_GET_AFE_DATA_req.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little ENDIAN) packed struct
                            ACPH_GET_AFE_DATA_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of ACPH_GET_AFE_DATA_rsp +
                                  the size of uint8 * length.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.
   - ACPH_ERR_MODULE_DISABLED -- Module is currently disabled or not available
                                 in the AFE.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_GET_AFE_DATA                  0x0108

typedef struct ACPH_GET_AFE_DATA_req ACPH_GET_AFE_DATA_req;
#include "acdb_begin_pack.h"

/** Request structure for the AFE calibration data query.
*/
 struct  ACPH_GET_AFE_DATA_req{
      uint32_t afe_port_id;   /**< AFE port ID. This can be either a Tx or
                                   an Rx port ID, depending on the use case. */
      uint32_t module_id;     /**< Module ID. */
      uint32_t parameter_id;  /**< Parameter ID. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_GET_AFE_DATA_rsp ACPH_GET_AFE_DATA_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the AFE calibration data query.
*/
 struct    ACPH_GET_AFE_DATA_rsp{
      uint32_t module_id;     /**< Module ID. */
      uint32_t parameter_id;  /**<  Parameter ID. */
      uint16_t length;        /**< Length of the response. */
      uint16_t reserved;      /**< Reserved. */
      // followed by
      //uint8  data[<length>];
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_afe_data */

/** @addtogroup set_afe_data
@{ */

/**
Sets the AFE calibration data.

@cmdversion
Major - 1, Minor - 2

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_SET_AFE_DATA.
@param[in] req_buf_ptr Input request buffer filled with serialized (little
                       ENDIAN) packed struct ACPH_SET_AFE_DATA_req.
@param[in] req_buf_len Equals the size of ACPH_SET_AFE_DATA_req + the
                       size of uint8 * length.
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_SET_AFE_DATA                  0x0109

typedef struct ACPH_SET_AFE_DATA_req ACPH_SET_AFE_DATA_req;
#include "acdb_begin_pack.h"

/** Request structure for the set AFE calibration data command.
*/
 struct  ACPH_SET_AFE_DATA_req{
	  uint32_t op_mode;           /**< Operating mode. See #Afe_Op_Mode. */
	  uint32_t tx_afe_port_id;    /**< Tx AFE port ID. Applicable if
                                       op_mode = 0 or 1. */
	  uint32_t rx_afe_port_id;    /**< Rx AFE port ID. Applicable if
                                       op_mode = 0 or 2. */
          uint32_t module_id;     /**< Module ID. */
          uint32_t parameter_id;  /**< Parameter ID. */
          uint16_t length;        /**< Length of the request. */
          uint16_t reserved;      /**< Reserved. */
          //followed by
          //uint8  data[<length>];
   }
#include "acdb_end_pack.h"
;

/** AFE operating mode. */
enum Afe_Op_Mode{
	sidetone = 0,   /**< Sidetone mode. */
	tx = 1,         /**< Transmit mode. */
	rx = 2          /**< Receive mode. */
};

/** @} */ /* end_addtogroup set_afe_data */

/** @addtogroup get_afe_sidetone_gain
@{ */

/**
Gets the AFE sidetone gain data.

@cmdversion
Major - 1, Minor - 2

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_GET_AFE_SIDETONE_GAIN.
@param[in] req_buf_ptr Input request buffer filled with serialized (little
                       ENDIAN) packed struct ACPH_GET_AFE_SIDETONE_GAIN_req.
@param[in] req_buf_len Equals the size of ACPH_GET_AFE_SIDETONE_GAIN_req.
@param[in,out] resp_buf_ptr Response buffer to be filled by the service
                            provider with the serialized (little ENDIAN)
                            packed struct ACPH_GET_AFE_SIDETONE_GAIN_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of ACPH_GET_AFE_SIDETONE_GAIN_rsp.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.
   - ACPH_ERR_MODULE_DISABLED -- Module is currently disabled or not available
                                 in the AFE.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_GET_AFE_SIDETONE_GAIN                  0x010A

typedef struct ACPH_GET_AFE_SIDETONE_GAIN_req ACPH_GET_AFE_SIDETONE_GAIN_req;
#include "acdb_begin_pack.h"

/** Request structure for the get AFE sidetone gain data query.
*/
 struct  ACPH_GET_AFE_SIDETONE_GAIN_req{
      uint32_t tx_afe_port_id;   /**< Tx AFE port ID. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_GET_AFE_SIDETONE_GAIN_rsp ACPH_GET_AFE_SIDETONE_GAIN_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the get AFE sidetone gain data query.
*/
 struct    ACPH_GET_AFE_SIDETONE_GAIN_rsp{
      uint16_t enable;  /**< Enable. 0 = FALSE; 1 = TRUE. */
      uint16_t gain;    /**< Sidetone gain data. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_afe_sidetone_gain */

/** @addtogroup set_afe_sidetone_gain
@{ */

/**
Sets the AFE sidetone gain.

@cmdversion
Major - 1, Minor - 2

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_SET_AFE_SIDETONE_GAIN.
@param[in] req_buf_ptr Input request buffer filled with the serialized (little
                       ENDIAN) packed struct ACPH_SET_AFE_SIDETONE_GAIN_req.
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_SET_AFE_SIDETONE_GAIN                  0x010B

typedef struct ACPH_SET_AFE_SIDETONE_GAIN_req ACPH_SET_AFE_SIDETONE_GAIN_req;
#include "acdb_begin_pack.h"

/** Request structure for the set AFE sidetone gain data command.
*/
 struct  ACPH_SET_AFE_SIDETONE_GAIN_req{
	  uint32_t tx_afe_port_id;  /**< Tx AFE port ID. */
      uint32_t rx_afe_port_id;  /**< Rx AFE port ID. */
      uint16_t enable;          /**< Enable. 0 = FALSE; 1 = TRUE. */
      uint16_t gain;            /**< Sidetone gain to set. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup set_afe_sidetone_gain */

/** @addtogroup get_aud_dev_copp_handls_v2
@{ */

/**
Queries for audio device, app type and topology COPP handles.
NOTE: This command API is applicable to PLs other than LA

@cmdversion
Major - 1, Minor - 3

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2.
@param[in] req_buf_ptr Set to NULL.
@param[in] req_buf_len Set to 0.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2_rsp +
                                  the size of
                                  AUD_DEVICE_COPP_HANDLE_V2 * num_of_entry.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2     0x010C // 268

typedef struct ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2_rsp ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio device COPP handles query.
*/
 struct  ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_V2_rsp{
      uint32_t num_of_entry;   /**< Number of the entry. */
      //struct AUD_DEVICE_COPP_HANDLE_V2[<num_of_entry>];
  }

#include "acdb_end_pack.h"
;

typedef struct AUD_DEVICE_COPP_HANDLE_V2 AUD_DEVICE_COPP_HANDLE_V2;
#include "acdb_begin_pack.h"

/** Structure for the audio device and app type COPP handle.
*/
 struct  AUD_DEVICE_COPP_HANDLE_V2{
      uint32_t device_id;    /**< Device ID. */
      uint32_t topology_id;  /**< Topplogy ID */
      uint32_t appType_id;   /**< App Type ID */
      uint32_t copp_handle;  /**< COPP handle */
      uint32_t copp_id;      /**< COPP ID */
      uint32_t num_of_popps; /**< Number of POPPs */
  }

#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_dev_copp_handls_v2 */

/** @addtogroup get_aud_topol_copp_hndls_v2
@{ */

/**
Queries for audio topology, app type and COPP handles.
NOTE: This command API is applicable to LA only

@cmdversion
Major - 1, Minor - 3

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to 0.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed structs
                            ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2_rsp and
                            AUD_TOPOLOGY_COPP_HANDLES_V2.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2_rsp
                                  + the size of AUD_TOPOLOGY_COPP_HANDLES_V2 *
                                  num_of_entry.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2 0x010D // 269; LA-only command

typedef struct ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2_rsp ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio topology COPP handles query.
*/
 struct ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_V2_rsp{
      uint32_t num_of_entry;  /**< Number of COPP handles. */
      //struct AUD_TOPOLOGY_COPP_HANDLES_V2[<num_of_entry>];
   }
#include "acdb_end_pack.h"
;

typedef struct AUD_TOPOLOGY_COPP_HANDLES_V2 AUD_TOPOLOGY_COPP_HANDLES_V2;
#include "acdb_begin_pack.h"

/** Structure for the audio topology COPP handles.
*/
 struct  AUD_TOPOLOGY_COPP_HANDLES_V2{
      uint32_t topology_Id;   /**< Topology ID. */
      uint32_t appType_id;    /**< App Type ID. */
      uint32_t copp_handle;   /**< COPP handle. */
      uint32_t copp_id;       /**< COPP ID. */
      uint32_t num_of_popps;  /**< Number of POPPs. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_topol_copp_hndls_v2 */

/** @addtogroup get_aud_copp_strm_hndls_v3
@{ */

/**
Queries for audio COPP stream handles with app type version3.

@cmdversion
Major - 1, Minor - 3

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_V3.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req.
@param[in] req_buf_len Length of the request buffer in bytes (8).
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                        with the serialized (little ENDIAN) packed struct
                        ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v3_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v3_rsp +
                                  the size of Aud_Stream_Popp_Info_v2 * num_of_popp_ids.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_V3     0x010E

typedef struct ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v3_rsp ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v3_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the audio COPP stream handles V3 query.
*/
 struct  ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_v3_rsp{
      uint32_t num_of_popp_ids;  /**< Number of POPP IDs. */
	  //followed by:
      //Aud_Stream_Popp_Info popp_info_v2[num_of_popp_ids];
   }
#include "acdb_end_pack.h"
;

#include "acdb_begin_pack.h"

/** Audio stream POPP information version 2.
*/
 struct Aud_Stream_Popp_Info_v2{
    uint32_t popp_id;            /**< POPP ID. */
    uint32_t popp_topology_id;   /**< POPP topology ID. */
    uint32_t appType_id;         /**< App Type ID. */
  }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_aud_copp_strm_hndls_v3 */

/*--------------------------------------------------
 ** ACPH ADIE RTC Service and Command range declarations
 *-------------------------------------------------*/

/** @addtogroup adie_rtc_svc_defs
@{ */

/** ADIE RTC registration service ID. Command IDs for the ADIE RTC service
range from 501 to 750. */
#define ACPH_ADIE_RTC_REG_SERVICEID 0x00000003

// Command IDs of value from 501 to 750 should be used only by DSP ADIE service

/** ADIE RTC start command ID value = 501. */
#define ACPH_ADIE_RTC_CMD_ID_START  0x01F5 //501

/** ADIE RTC end command ID value = 750. */
#define ACPH_ADIE_RTC_CMD_ID_END    0x02EE //

/** @} */ /* end_addtogroup adie_rtc_svc_defs */

// ADIE RTC protocal version 1.0 command definitions:
//#define ACPH_CMD_GET_ADIE_REGISTER  0x01F6 //502, ---used to be 0x00A1
//#define ACPH_CMD_SET_ADIE_REGISTER  0x01F5 //501, ---used to be 0x00A0
//#define ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS  0x01F8 //504, ---used to be 0x00A3
//#define ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS  0x01F7 //503, ---used to be 0x00A2
//#define ACPH_CMD_QUERY_ADIE_RTC_VERSION  0x01F9 //505 //  ---new command

// ADIE RTC protocal version 1.1 command definitions:
//#define ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES  0x01FA //506,
//#define ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA  0x01FB //507,
//#define ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA  0x01FC //508,

/** @addtogroup get_adie_rtc_ver
@{ */

/**
Queries for the ADIE RTC protocol version.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_ADIE_RTC_VERSION.
@param[in] req_buf_ptr Set this to NULL.
@param[in] req_buf_len Set this to 0.
@param[in,out] resp_buf_ptr  Input buffer to be filled by the service provider
                             with the serialized (little Endian) packed struct
                             ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp.
@param[in] resp_buf_len  Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of struct
                                   ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_QUERY_ADIE_RTC_VERSION                    0x01F9 //505 //  ---new command

typedef struct ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the ADIE RTC version query.
*/
 struct  ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp{
      uint32_t  adie_rtc_major_version;   /**< ADIE RTC major version. */
      uint32_t  adie_rtc_minor_version;   /**< ADIE RTC minor version. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_adie_rtc_ver */

/** @addtogroup get_adie_reg
@{ */

/**
Queries for the ADIE register.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_ADIE_REGISTER.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct
                       ACPH_CMD_GET_ADIE_REGISTER_req.
@param[in] req_buf_len Request buffer length.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little Endian) packed struct
                            ACPH_CMD_GET_ADIE_REGISTER_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_GET_ADIE_REGISTER_rsp.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_GET_ADIE_REGISTER  0x01F6 //502, ---used to be 0x00A1

typedef struct ACPH_CMD_GET_ADIE_REGISTER_req ACPH_CMD_GET_ADIE_REGISTER_req;
#include "acdb_begin_pack.h"

/** Request structure for the ADIE register query.
*/
 struct  ACPH_CMD_GET_ADIE_REGISTER_req{
      uint32_t  register_id;    /**< Register ID. */
      uint32_t  register_mask;  /**< Register mask. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_GET_ADIE_REGISTER_rsp ACPH_CMD_GET_ADIE_REGISTER_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the ADIE register query.
*/
 struct  ACPH_CMD_GET_ADIE_REGISTER_rsp{
      uint32_t  value;  /**< ADIE register value. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_adie_reg */

/** @addtogroup set_adie_reg
@{ */

/**
Sets the ADIE register.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_SET_ADIE_REGISTER.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed struct
                       ACPH_CMD_SET_ADIE_REGISTER_req.
@param[in] req_buf_len Request buffer length.
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_SET_ADIE_REGISTER  0x01F5 //501, ---used to be 0x00A0

typedef struct ACPH_CMD_SET_ADIE_REGISTER_req ACPH_CMD_SET_ADIE_REGISTER_req;
#include "acdb_begin_pack.h"

/** Request structure for the Set ADIE Register command.
*/
 struct  ACPH_CMD_SET_ADIE_REGISTER_req{
      uint32_t  register_id;     /**< Register ID. */
      uint32_t  register_mask;   /**< Register mask. */
      uint32_t register_value;   /**< Register value. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup set_adie_reg */

/** @addtogroup get_mult_adie_regs
@{ */

/**
Queries for multiple ADIE registers.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) of packed struct
                       ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS_req.
@param[in] req_buf_len Equals the size of
                       ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS_req + the size of
	               ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS_req * num_of_registers.
@param[in,out] resp_buf_ptr Buffer to be filled by the service provider
                            with the serialized (little Endian) packed array
							of uint32 type register values.
@code
  // uint32_t  register_values[<num_of_registers>];
@endcode
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Number of bytes filled in the resp_buf_ptr
                                  output buffer.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.
   - ACDB_INSUFFICIENTMEMORY -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS  0x01F8 //504, ---used to be 0x00A3

typedef struct ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS_req ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS_req;
#include "acdb_begin_pack.h"

/** Request structure for the multiple ADIE registers query.
*/
 struct  ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS_req{
      uint32_t num_of_registers;  /**< Number of registers. */
      //struct ACPH_ADIE_req[<num_of_registers>];
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_ADIE_req ACPH_ADIE_req;
#include "acdb_begin_pack.h"

/** Get register value query format for a single register.
*/
 struct  ACPH_ADIE_req{
         uint32_t register_id;    /**< Register ID. */
         uint32_t register_mask;  /**< Register mask. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_mult_adie_regs */

   //Response buffer: (little Endian) packed array:
   //uint32_t  register_values[<num_of_registers>];

/** @addtogroup set_mult_adie_regs
@{ */

/**
Sets multiple ADIE registers.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled);
@endcode

@param[in] nCommandId Command ID is ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little Endian) packed structs
                       ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS_req and
                       ACPH_SET_ADIE.
@param[in] req_buf_len Equals the size of
                       ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS_req +
	                   the size of uint32_t * num_of_registers.
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACDB_BADPARM -- One or more invalid parameters were provided.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
@newpage
*/
#define ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS  0x01F7 //503, ---used to be 0x00A2

typedef struct ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS_req ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS_req;
#include "acdb_begin_pack.h"

/** Request structure for the Set Multiple ADIE Registers command.
*/
 struct  ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS_req{
      uint32_t  num_of_registers;  /**< Number of registers. */
      //struct ACPH_SET_ADIE[<num_of_registers>];
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_SET_ADIE ACPH_SET_ADIE;
#include "acdb_begin_pack.h"

/** Set register value query format for a single register.
*/
 struct  ACPH_SET_ADIE{
      uint32_t  register_id;    /**< Register ID. */
      uint32_t  register_mask;  /**< Register mask. */
      uint32_t register_value;  /**< Register value. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup set_mult_adie_regs */

/** @addtogroup get_voc_adie_hndls
@{ */

/**
Gets the voice ADIE handles.

@cmdversion
Major - 1, Minor - 1

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES.
@param[in] req_buf_ptr Input request buffer filled with the serialized (little
                 ENDIAN) packed struct ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_req.
@param[in] req_buf_len Request buffer length in bytes (4).
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little ENDIAN) packed struct
                            ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_rsp.
@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES     0x01FA

typedef struct ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_req ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_req;
#include "acdb_begin_pack.h"

/** Request structure for the Get the Voice ADIE Handles command.
*/
 struct  ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_req{
      uint32_t voc_copp_handle;  /**< Voice COPP handle. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_rsp ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the Set Multiple ADIE Registers command.
*/
   struct ACPH_CMD_QUERY_GET_VOC_ADIE_HANDLES_rsp {
      uint32_t tx_codec_handle;  /**< Tx codec handle. */
      uint32_t rx_codec_handle;  /**< Rx codec handle. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_voc_adie_hndls */

/** @addtogroup get_adie_sidetone_iir_data
@{ */

/**
Gets the ADIE sidetone Infinite Impulse Response (IIR) filter data.

@cmdversion
Major - 1, Minor - 1

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_req struct.
@param[in] req_buf_len Equals the size of
                       ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_req.
@param[in,out] resp_buf_ptr Input buffer to be filled by the service provider
                            with the serialized (little ENDIAN) packed struct
                            ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_rsp.
@param[in] resp_buf_len Length of the input buffer for the response data.
@param[out] resp_buf_bytes_filled Equals the size of
                                  ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_rsp +
                                  the size of uint8 * length.

@return
   - ACPH_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.
   - ACPH_ERR_OUT_OF_BUFFER_SIZE -- Buffer size provided is not sufficient.
   - ACPH_ERR_MODULE_DISABLED -- Module is currently disabled or not available
                                 on the codec.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA                  0x01FB

typedef struct ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_req ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_req;
#include "acdb_begin_pack.h"

/** Request structure for the Get ADIE Sidetone IIR Data command.
*/
 struct  ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_req{
      uint32_t rx_codec_handle;  /**< Rx codec handle. */
      uint32_t module_id;        /**< Module ID. */
      uint32_t parameter_id;     /**< Parameter ID. */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_rsp ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_rsp;
#include "acdb_begin_pack.h"

/** Response structure for the Get the Voice ADIE Handles command.
*/
 struct    ACPH_CMD_GET_ADIE_SIDETONE_IIR_DATA_rsp{
      uint32_t module_id;     /**< Module ID. */
      uint32_t parameter_id;  /**< Parameter ID. */
      uint16_t length;        /**< Response length. */
      uint16_t reserved;      /**< Reserved. */
	  // followed by
      //uint8  data[<length>];
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup get_adie_sidetone_iir_data */

/** @addtogroup set_adie_sidetone_iir_data
@{ */

/**
Sets the ADIE sidetone IIR filter data.

@cmdversion
Major - 1, Minor - 1

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA_req.
@param[in] req_buf_len Equals the size of
                       ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA_req + the
                       size of uint8 * lengh
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACDB_SUCCESS -- Command executed successfully.
   - ACPH_ERR_UNKNOWN_REASON -- Unknown error occured.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA                  0x01FC

typedef struct ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA_req ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA_req;
#include "acdb_begin_pack.h"

/** Request structure for the Set ADIE Sidetone IIR Data command.
*/
 struct  ACPH_CMD_SET_ADIE_SIDETONE_IIR_DATA_req{
      uint32_t rx_codec_handle;  /**< Rx codec handle. */
      uint32_t tx_codec_handle;  /**< Tx codec handle. */
      uint32_t module_id;        /**< Module ID. */
      uint32_t parameter_id;     /**< Parameter ID. */
      uint16_t length;           /**< Length of the request. */
      uint16_t reserved;         /**< Reserved. */
      // followed by
      //uint8  data[<length>];
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup set_adie_sidetone_iir_data */

/** @addtogroup fts_serv_defs
@{ */

/** File Transfer Service registration service ID definition. Command IDs for the file
    transfer service range from 751 to 800. */
#define ACPH_FILE_TRANSFER_REG_SERVICEID 0x00000004

// Command IDs of value from 751 to 800 should be used only by file transfer service

/** File Transfer Service command ID start value = 751. */
#define ACPH_FILE_TRANSFER_CMD_ID_START  0x02EF //751
/** File Transfer Service command ID end value = 800. */
#define ACPH_FILE_TRANSFER_CMD_ID_END    0x0320 //800

/** @} */ /* end_addtogroup fts_serv_defs */

//File Transfer Service command Id's for version 1.0:
//#define ACPH_FTS_CMD_OPEN_FILE     0x02EF // 751
//#define ACPH_FTS_CMD_CLOSE_FILE     0x02F0 // 752
//#define ACPH_FTS_CMD_WRITE_FILE_DATA   0x02F1 // 753

/** @addtogroup open_file
@{ */

/**
This API is used to open a file or create a file if not existent on target file
system in write mode permissions.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_FTS_CMD_OPEN_FILE.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_FTS_CMD_OPEN_FILE_req.
@param[in] req_buf_len Equals the size of
                       ACPH_FTS_CMD_OPEN_FILE_req struct
@param[in,out] resp_buf_ptr output request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_FTS_CMD_OPEN_FILE_resp.
@param[in] resp_buf_len Equals the size of
                       ACPH_FTS_CMD_OPEN_FILE_resp struct
@param[out] resp_buf_bytes_filled outputs number of bytes filled in resp_buf_ptr.

@return
   - ACPH_SUCCESS -- If the file open operation is successfull.
   - ACPH_ERR_INVALID_FILE_PATH -- If the invalid file path is provided.
   - ACPH_FAILURE -- If the file open operation fails.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_FTS_CMD_OPEN_FILE                  0x02EF

typedef struct ACPH_FTS_CMD_OPEN_FILE_req ACPH_FTS_CMD_OPEN_FILE_req;
#include "acdb_begin_pack.h"

/** Request structure for the open file command.
*/
 struct  ACPH_FTS_CMD_OPEN_FILE_req{
      uint32_t file_name_len;  /**< Full file path name length. */
      char fileName[ACPH_FILENAME_MAX_CHARS];
	    /**< Array that holds the file path and name. The file size cannot
	       exceed 256 characters, including the NULL-termiated character.
		   @newpagetable */
   }
#include "acdb_end_pack.h"
;

typedef struct ACPH_FTS_CMD_OPEN_FILE_resp ACPH_FTS_CMD_OPEN_FILE_resp;
/** Response structure for the open file command.
*/
 struct  ACPH_FTS_CMD_OPEN_FILE_resp{
      uint32_t file_handle_id;  /**< file handle id. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup open_file */

/** @addtogroup close_file
@{ */

/**
This API is used to close the file handle which is opened using file open command.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_FTS_CMD_CLOSE_FILE.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_FTS_CMD_CLOSE_FILE_req.
@param[in] req_buf_len Equals the size of
                       ACPH_FTS_CMD_CLOSE_FILE_req struct
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as zero.

@return
   - ACPH_SUCCESS -- If the file close operation is success.
   - ACPH_FAILURE -- If the file close operation fails.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_FTS_CMD_CLOSE_FILE                  0x02F0

typedef struct ACPH_FTS_CMD_CLOSE_FILE_req ACPH_FTS_CMD_CLOSE_FILE_req;
#include "acdb_begin_pack.h"

/** Request structure for the open file command.
*/
 struct  ACPH_FTS_CMD_CLOSE_FILE_req{
      uint32_t file_handle_id;  /**< file handle id. */
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup close_file */

/**
This API is used to write the data to the file in the file system. Based on the mode,
it will either overwrite or create a file or append to an existing file.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_FTS_CMD_WRITE_FILE_DATA.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_FTS_CMD_WRITE_FILE_DATA_req.
@param[in] req_buf_len Equals the size of
                       ACPH_FTS_CMD_WRITE_FILE_DATA_req + the
                       size of uint8 * data_length
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as
                                  zero.

@return
   - ACPH_SUCCESS -- If the write/append operation is success.
   - ACPH_FAILURE -- If the write/append operation failed.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_FTS_CMD_WRITE_FILE_DATA                  0x02F1

typedef struct ACPH_FTS_CMD_WRITE_FILE_DATA_req ACPH_FTS_CMD_WRITE_FILE_DATA_req;
#include "acdb_begin_pack.h"

/** Request structure for the set file name command.
*/
struct  ACPH_FTS_CMD_WRITE_FILE_DATA_req{
	  uint32_t file_handle_id;  /**< file handle id. */
      uint32_t data_length; /**< Lenght of the data to be written*/
	  // followed by
      //uint8  data[<data_length>];
   }
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup write_file_data */

/** @addtogroup mcs_serv_defs
@{ */

/** Media Control Service registration service ID definition. Command IDs for the Media
    control service range from 801 to 850. */
#define ACPH_MEDIA_CONTROL_REG_SERVICEID 0x00000005

// Command IDs of value from 801 to 850 should be used only by media control service

/** Media Control Service command ID start value = 801. */
#define ACPH_MEDIA_CONTROL_CMD_ID_START  0x0321 //801
/** File Control Service command ID end value = 850. */
#define ACPH_MEDIA_CONTROL_CMD_ID_END    0x0352 //850

/** @} */ /* end_addtogroup mcs_serv_defs */

//Medi Control Service command Id's for version 1.0:
//#define ACPH_MCS_CMD_REC               0x0321 // 801
//#define ACPH_MCS_CMD_PLAY              0x0322 // 802
//#define ACPH_MCS_CMD_PLAY_REC          0x0323 // 803
//#define ACPH_MCS_CMD_STOP              0x0324 // 804

/** @addtogroup mcs_rec
@{ */

/**
This API is used to instantiate recording session on target.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_MCS_CMD_REC.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_MCS_CMD_REC_req.
@param[in] req_buf_len Equals the size of ACPH_MCS_CMD_REC_req
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as zero.

@return
   - ACPH_SUCCESS -- If the recording operation started successfully.
   - ACPH_FAILURE -- If the recoridng operation failed to start.
   - ACPH_ERR_INVALID_REC_DURATION -- If invalid record duration is provided.
   - ACPH_ERR_ACTIVE_SESSION_FOUND -- If a session is already in running state.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_MCS_CMD_REC                  0x0321

typedef struct ACPH_MCS_CMD_REC_req ACPH_MCS_CMD_REC_req;
#include "acdb_begin_pack.h"

/** Request structure for the mcs rec command.
*/
 struct  ACPH_MCS_CMD_REC_req{
      uint32_t tx_device_id;  /**< Tx device Id*/
	  uint32_t sample_rate; /**< sample rate*/
      uint32_t no_of_channels; /**< no of channels*/
	  uint32_t bit_width; /**< bit width*/
	  int32_t rec_dur_in_sec; /**< recording duration in seconds
									(value < 0) - indicates infinite duration.
									(value > 0) - indicates recording duration in seconds, after which the recording has to stop automatically.
									(value = 0) - should throw error code ACPH_ERR_INVALID_PLAYBACK_DURATION*/
	  uint32_t write_to_file; /**< determines whether to store recording data on
                                  file system or not.
								  value '0' indicates should not write
								  value '1' indicates should write*/
      uint32_t file_name_len;  /**< Full file path name length where the recorded
                                    raw pcm has to be written. */
      char fileName[ACPH_FILENAME_MAX_CHARS];
	    /**< Array that holds the file path and name where the recorded raw pcm
		   has to be written. The file size cannot
	       exceed 256 characters, including the NULL-termiated character.
		   @newpagetable */
}
#include "acdb_end_pack.h"
;

/** @} */ /* end_addtogroup mcs_rec */

/** @addtogroup mcs_play
@{ */

/**
This API is used to instantiate playback.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_MCS_CMD_PLAY.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_MCS_CMD_PLAY_req.
@param[in] req_buf_len Equals the size of ACPH_MCS_CMD_PLAY_req
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as zero.

@return
   - ACPH_SUCCESS -- If the playback operation started successfully.
   - ACPH_FAILURE -- If the playback operation failed to start.
   - ACPH_ERR_ACTIVE_SESSION_FOUND -- If a session is already in running state.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_MCS_CMD_PLAY                  0x0322

typedef struct ACPH_MCS_CMD_PLAY_req ACPH_MCS_CMD_PLAY_req;
#include "acdb_begin_pack.h"

/** Request structure for the mcs play command.
*/
 struct  ACPH_MCS_CMD_PLAY_req{
      uint32_t rx_device_id;  /**< Rx device Id*/
	  uint32_t sample_rate; /**< sample rate*/
      uint32_t no_of_channels; /**< no of channels*/
	  uint32_t bit_width; /**< bit width*/
	  uint32_t playback_mode; /**< playback mode, refer the enum AcphPlaybackModes for supported values
					  1 - ACPH_PLAYBACK_MODE_REGULAR: regular playback
					  2 - ACPH_PLAYBACK_MODE_ANC: ANC playback */
	  int32_t play_dur_in_sec; /**< Specifies playback duration in seconds, below are the accpetable values
					  (value < 0) - playback duration is infinite, so the playback has to be repeated
                                  even if EOF is Received until mcs stop command is issued explicitly.
					  (value = 0) - Playback ends as and when file EOF is reached.
					  (value > 0) - Playback duration in seconds.*/
	  uint32_t file_name_len;  /**< Full file path name length which contains raw pcm data. */
      char fileName[ACPH_FILENAME_MAX_CHARS];
	    /**< Array that holds the file path and name of the playback file of raw pcm data.
           The file size cannot exceed 256 characters, including the NULL-termiated character.
		   @newpagetable */
   }
#include "acdb_end_pack.h"
;
/** @} */ /* end_addtogroup mcs_play_rec */

/** @addtogroup mcs_play_rec
@{ */

/**
This API is used to instantiate playback and recording simultaneously.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_MCS_CMD_PLAY_REC.
@param[in] req_buf_ptr Input request buffer filled with the serialized
                       (little ENDIAN) packed struct
                       ACPH_MCS_CMD_PLAY_REC_req.
@param[in] req_buf_len Equals the size of ACPH_MCS_CMD_PLAY_REC_req
@param[in,out] resp_buf_ptr Response buffer is NULL
@param[in] resp_buf_len Length of the output buffer is passed as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as zero.

@return
   - ACPH_SUCCESS -- If the recording and playback operation started successfully.
   - ACPH_FAILURE -- If the recoridng and playback operation failed to start.
   - ACPH_ERR_INVALID_RECORD_DURATION -- If invalid record duration is provided.
   - ACPH_ERR_ACTIVE_SESSION_FOUND -- If a session is already in running state.

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_MCS_CMD_PLAY_REC                  0x0323

typedef struct ACPH_MCS_CMD_PLAY_REC_req ACPH_MCS_CMD_PLAY_REC_req;
#include "acdb_begin_pack.h"

/** Request structure for the mcs play rec command.
*/
 struct  ACPH_MCS_CMD_PLAY_REC_req{
      ACPH_MCS_CMD_REC_req rec_session; /**< Recording session request data */
      ACPH_MCS_CMD_PLAY_req play_session; /**< Playback session request data */
   }
#include "acdb_end_pack.h"
;
/** @} */ /* end_addtogroup mcs_play_rec */

/** @addtogroup mcs_stop
@{ */

/**
This API is used to stop  any playback or rec activity going on triggered by this service.
The expectation of stop is to close the playback and recording or recording only.
All the resources needs to be released.

@cmdversion
Major - 1, Minor - 0

The parameters determine the callback values using this format:
@code
ACPH_CALLBACK_PTR (uint16_t nCommandId,
                   uint8_t *req_buf_ptr,
                   uint32_t req_buf_len,
                   uint8_t *resp_buf_ptr,
                   uint32_t resp_buf_len,
                   uint32_t *resp_buf_bytes_filled); @endcode

@param[in] nCommandId Command ID is ACPH_MCS_CMD_STOP.
@param[in] req_buf_ptr input req_buf_ptr is passed NULL.
@param[in] req_buf_len req_buf_len should be proivded as zero.
@param[in,out] resp_buf_ptr output buffer resp_buf_ptr is passed NULL.
@param[in] resp_buf_len resp_buf_len should be provided as zero.
@param[out] resp_buf_bytes_filled Valid output buffer length is returned as zero.

@return
   - ACPH_SUCCESS -- Successfully able to stop playback or record session.
   - ACPH_ERR_NO_ACTIVE_SESSION_FOUND -- If there is not active session to stop

@sa
acph_register_command() \n
ACPH_CALLBACK_PTR
*/
#define ACPH_MCS_CMD_STOP                  0x0324

/** @} */ /* end_addtogroup mcs_stop */

/*------------------------------------------
 ** Type Declarations
 *-------------------------------------------*/

/** @addtogroup acph_callback_def
@{ */

/**
  ACPH callback pointer.

  @param[in] nCommandId Command ID.
  @param[in] req_buf_ptr Pointer to the request buffer.
  @param[in] req_buf_len Length of the request buffer.
  @param[in,out] resp_buf_ptr Pointer to the response buffer.
  @param[in] resp_buf_len Length of the response buffer.
  @param[out] resp_buf_bytes_filled Number of bytes filled in the response
                                    buffer resp_buf_ptr.
*/
typedef int32_t (*ACPH_CALLBACK_PTR)(uint16_t nCommandId,
                                                      uint8_t *req_buf_ptr,
                                                          uint32_t req_buf_len,
                                                          uint8_t *resp_buf_ptr,
                                                          uint32_t resp_buf_len,
                                                          uint32_t *resp_buf_bytes_filled
                                                          );

/** @} */ /* end_addtogroup acph_callback_def */

/*
   --------------------
   | External functions |
   --------------------
   */

/** @ingroup acph_init
 * Initializes the ACPH and allocates memory.
 *
 * @return
 * - ACPH_SUCCESS -- Initialization was successful.
 * - ACPH_ERROR -- An error occured during initialization.
 *
 * @dependencies
 * None.
 * @newpage
 */
int32_t acph_init(void);

/** @ingroup acph_deinit
 * De-initializes the ACPH and frees memory.
 *
 * @return
 * - ACPH_SUCCESS -- De-initialization was successful.
 * - ACPH_ERROR -- An error occured during de-initialization.
 *
 * @dependencies
 * None.
 */
int32_t acph_deinit(void);

/** @ingroup acph_reg_cmd
 * Registers the command ID into the ACPH registry table.
 *
 * @param[in] nService_id Service ID the client must use to register the
 *                        command. \n
 *                        Possible values: \n
 *                        - 0x00000001 -- ACPH_ONLINE_REG_SERVICEID \n
 *                        - 0x00000002 -- ACPH_DSP_RTC_REG_SERVICEID \n
 *                        - 0x00000003 -- ACPH_ADIE_RTC_REG_SERVICEID
 *                        @tablebulletend
 * @param[in] fcn_ptr Callback function pointer that handles the commands in
                      the service nServiceId.
 *
 * @return
   - ACPH_SUCCESS -- Registration with the function pointer was successful.
   - ACPH_FAILURE -- Registration failed.
 *
 * @dependencies
 * None.
 */
int32_t acph_register_command(uint32_t nService_id,ACPH_CALLBACK_PTR fcn_ptr);

/** @ingroup acph_dereg_cmd
 * Deregisters a command ID from the ACPH registry table.
 *
 * @param[in] nService_id Service ID the client must use to deregister the
 *                        command. \n
 *                        Possible values: \n
 *                        - 0x00000001 -- ACPH_ONLINE_REG_SERVICEID \n
 *                        - 0x00000002 -- ACPH_DSP_RTC_REG_SERVICEID \n
 *                        - 0x00000003 -- ACPH_ADIE_RTC_REG_SERVICEID
 *                        @tablebulletend
 *
 * @return
 * - ACPH_SUCCESS -- Deregistration was successful.
 * - ACPH_FAILURE -- Deregistration failed.
 *
 * @dependencies
 * None.
 */
int32_t acph_deregister_command(uint32_t nService_id);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif //_AUDCAL_ACPH_H_

