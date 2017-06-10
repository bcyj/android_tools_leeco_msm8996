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
/**
  \file ***************************************************************************
 *
 *                      EDIT HISTORY FOR FILE
 *
 *  This section contains comments describing changes made to this file.
 *  Notice that changes are listed in reverse chronological order.
 *   
 *  $Header: 
 *
 *when         who     what, where, why
 *--------   ---     ----------------------------------------------------------
 *05/28/10     ayin     initial draft
 ********************************************************************************
 */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal3/acph/rel/1.0/inc/acph.h#1 $ */
/*
   --------------------
   |include files                |
   --------------------
   */
#include "acdb_os_includes.h"
 
/*------------------------------------------
 ** ERROR CODE definitions 4 char_t
 *-------------------------------------------*/
#define ACPH_SUCCESS 	0
#define ACPH_FAILURE	(-1)
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
#define ACPH_ERR_INVALID_SERVICE_ID       0x00000013

   
/*--------------------------------------------------
 ** ACPH Online Service and Command range declarations
 *-------------------------------------------------*/
#define ACPH_ONLINE_REG_SERVICEID 0x00000001

// Command IDs of value from 1 to 250 should be used only by Online service
#define ACPH_ONLINE_CMD_ID_START  0x0001 //1
#define ACPH_ONLINE_CMD_ID_END    0x00FA //250

/*------------------------------------------
 ** COMMAND ID definitions 2 char_t
 *-------------------------------------------*/
//Online Calibration Command Ids

/**
@fn  acph_online_ioctl (ACPH_CMD_GET_TARGET_VERSION,...)
@brief API to query for getting the target version id associated with B-Family PL's

Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_GET_TARGET_VERSION"
		uint32_t cmdLen; //Command Len "0"
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_GET_TARGET_VERSION
		uint32_t respLen; //Response Len: "4"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
		uint32_t tgtvId;  // Target version id of Family B PL's
	}

*/
#define ACPH_CMD_GET_TARGET_VERSION                0x0001

/**
@fn  acph_online_ioctl (ACPH_CMD_CHECK_CONNECTION,...)
@brief API to check the target connection
Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_CHECK_CONNECTION"
		uint32_t cmdLen; //Command Len "0"
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_CHECK_CONNECTION
		uint32_t respLen; //Response Len: "0"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
	}
*/
#define ACPH_CMD_CHECK_CONNECTION                  0x0002

/**
@fn  acph_online_ioctl (ACPH_CMD_GET_ACDB_FILES_INFO,...)
@brief API to check the target connection
Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_GET_ACDB_FILES_INFO"
		uint32_t cmdLen; //Command Len "0"
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_GET_ACDB_FILES_INFO
		uint32_t respLen; //Response Len: "acdbFileInf size in bytes"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
		uint32_t maxRespLen; //Max response buffer size
		AcdbFilesInfo acdbFileInf;
	}
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
	}
*/
#define ACPH_CMD_GET_ACDB_FILES_INFO			   0x0003

/**
@fn  acph_online_ioctl (ACPH_CMD_QUERY_MAX_BUFFER_LENGTH,...)
@brief API to check the target connection
Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_QUERY_MAX_BUFFER_LENGTH"
		uint32_t cmdLen; //Command Len "0"
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_QUERY_MAX_BUFFER_LENGTH
		uint32_t respLen; //Response Len: "4"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
		uint32_t maxRespLen; //Max response buffer size
	}
*/
#define ACPH_CMD_QUERY_MAX_BUFFER_LENGTH           0x0004

/**
@fn  acph_online_ioctl (ACPH_CMD_GET_ACDB_FILE,...)
@brief API to query for getting the target version id associated with B-Family PL's

Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_GET_ACDB_FILE"
		uint32_t cmdLen; //Command Len "sizeof(fdr) in bytes"
		FileDataReqType fdr;  // req cmd struct
	}

	struct FileDataReqType
	{
		uint32_t fileStartOffset; //Start position in file from where the data needs to be copied
		uint32_t fileLen;         //Len of the requested file
		uint32_t fileNameLen;     //File Name Length
		uint8_t data[fileLen];           //File Data array			
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_GET_ACDB_FILE
		uint32_t respLen; //Response Len: "sizeof(data)"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
		uint8_t data[]; // File Data copied
	}

*/
#define ACPH_CMD_GET_ACDB_FILE		               0x0005

/**
@fn  acph_online_ioctl (ACPH_CMD_GET_NO_OF_TBL_ENTRIES_ON_HEAP,...)
@brief API to query for getting the target version id associated with B-Family PL's

Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_GET_NO_OF_TBL_ENTRIES_ON_HEAP"
		uint32_t cmdLen; //Command Len "4"
		uint32_t tblId;  // Table Id for which the no of entries are required
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_GET_NO_OF_TBL_ENTRIES_ON_HEAP
		uint32_t respLen; //Response Len: "4"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
		uint32_t noOfHeapEntries;  // No of heap entries for the given table
	}

*/
#define ACPH_CMD_GET_NO_OF_TBL_ENTRIES_ON_HEAP	   0x0006

/**
@fn  acph_online_ioctl (ACPH_CMD_GET_TBL_ENTRIES_ON_HEAP,...)
@brief API to query for getting the target version id associated with B-Family PL's

Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_GET_TBL_ENTRIES_ON_HEAP"
		uint32_t cmdLen; //Command Len "12"
		uint32_t tblId;  // Table Id for which the no of entries are required
		uint32_t entriesStartOffset; //Offset from where the entries needs to be provided
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_GET_TBL_ENTRIES_ON_HEAP
		uint32_t respLen; //Response Len: "8+sizeof(data)"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
		uint32_t noOfEntriesCopied;  // No of heap entries copied in the response
		uint8_t data[]; // lookup table entries based on the table id including mid and pid
	}

*/
#define ACPH_CMD_GET_TBL_ENTRIES_ON_HEAP	       0x0007

/**
@fn  acph_online_ioctl (ACPH_CMD_GET_ACDB_DATA,...)
@brief API to query for getting the data for the given lookup table and mid, pid values

Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_GET_ACDB_DATA"
		uint32_t cmdLen; //Command Len "sizeof(indices) in bytes + sizeof(tblid)"
		uint32_t tblId;  // Table Id for which the no of entries are required
		uint32_t[] indices; //Indices including mid,pid or only pid based on the 
		                    // table for which the data is being requested for
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_GET_ACDB_DATA
		uint32_t respLen; //Response Len: "sizeof(data)"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
		uint8_t data[]; // Data for which the request is made.
	}

*/
#define ACPH_CMD_GET_ACDB_DATA		               0x0008

/**
@fn  acph_online_ioctl (ACPH_CMD_SET_ACDB_DATA,...)
@brief API to query for setting the data for the given lookup table and mid, pid values

Request Packet Struct Format:
	struct ReqPktType
	{
		uint16_t cmdId;  //Command Id "ACPH_CMD_SET_ACDB_DATA"
		uint32_t cmdLen; //Command Len "sizeof(indices) in bytes + sizeof(tblid) + sizeof(data)"
		uint32_t tblId;  // Table Id for which the no of entries are required
		uint32_t indices[]; //Indices including mid,pid or only pid based on the 
		                    // table for which the data is being requested for
		uint8_t data[];
	}

Response Packet Format:
	struct RespPktType
	{
		uint16_t cmdId;  //Command Id: ACPH_CMD_SET_ACDB_DATA
		uint32_t respLen; //Response Len: "0"
		uint8_t flag;     //Operation status: Success(1) Failure (0)
	}

*/
#define ACPH_CMD_SET_ACDB_DATA		               0x0009


/**
@fn  acph_online_ioctl (ACPH_CMD_SET_ACDB_DATA,...)
@brief API to query for setting the data for the given lookup table and mid, pid values

Request Packet Struct Format: NULL

Response Packet Format:
   struct ACPH_CMD_QUERY_ONLINE_VERSION_rsp {
      uint16_t	online_major_version;
      uint16_t	online_minor_version;
   }

*/
#define ACPH_CMD_QUERY_ONLINE_VERSION		               0x000A






/*--------------------------------------------------
 ** ACPH DSP RTC Service and Command range declarations
 *-------------------------------------------------*/
#define ACPH_DSP_RTC_REG_SERVICEID 0x00000002
// Command IDs of value from 251 to 500 should be used only by DSP RTC service
#define ACPH_DSP_RTC_CMD_ID_START  0x00FB //251
#define ACPH_DSP_RTC_CMD_ID_END    0x01F4 //500

//RTC command Id for RTC protocal version 1.0:
//#define ACPH_CMD_QUERY_RTC_VERSION			   0x0105 // 260 ---new command
//#define ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES     0x00FB // 251 ---used to be 0x00C1, for other PL than LA, to get active devices and COPPs
//#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES     0x00FC // 252 ---used to be 0x00C2
//#define ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS      0x00FD // 253 ---used to be 0x00C3
//#define ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES         0x00FE // 254 ---used to be 0x00C4
//#define ACPH_CMD_QUERY_VOC_VC_DEVICES              0x00FF // 255 ---used to be 0x00C5, for other PLs than LA, to get active device pairs
//#define ACPH_CMD_RTC_GET_CAL_DATA                  0x0101 // 256 ---used to be 0x0041
//#define ACPH_CMD_RTC_SET_CAL_DATA                  0x0102 // 257 ---used to be 0x0042
//#define ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES   0x0103 // 258; for LA, to get active topology and COPPs
//#define ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES           0x0104 // 259; for LA, to get active topology pairs

/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for DSP RTC protocol version, refer to acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_RTC_VERSION 
@param[in] req_buf_ptr: NULL 
@param[in] req_buf_len: 0 
@param[out] resp_buf_ptr[out]: serialized (little ENDIAN) of packed struct ACPH_CMD_QUERY_RTC_VERSION_rsp 
   struct ACPH_CMD_QUERY_RTC_VERSION_rsp {
      uint16_t	dsp_rtc_major_version;
      uint16_t	dsp_rtc_minor_version;
   }
@param[out] resp_buf_len[out]: >= sizeof (struct_ACPH_CMD_QUERY_RTC_VERSION_rsp)
@param[out] resp_buf_bytes_filled[out]: 4 (sizeof (struct_ACPH_CMD_QUERY_RTC_VERSION_rsp))
*/
#define ACPH_CMD_QUERY_RTC_VERSION			   0x0103 // 258 ---new command


/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for AUD_DEVICE_COPP_HANDLES, refer to acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES 
@param[in] req_buf_ptr: NULL 
@param[in] req_buf_len: 0 
@param[out] resp_buf_ptr[out]: serialized (little ENDIAN) of packed struct ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp 
   struct ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp {
      uint32 num_of_entry;
      struct AUD_DEVICE_COPP_HANDLE[<num_of_entry>];
   };
   struct AUD_DEVICE_COPP_HANDLE {
      uint32 device_id;
      uint32 copp_handle;
      uint32 copp_id;
      uint32 num_of_popps;
   }; 
@param[out] resp_buf_len[out]: >= sizeof(ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp)
@param[out] resp_buf_bytes_filled[out]: sizeof(ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES_rsp) 
*/
#define ACPH_CMD_QUERY_AUD_DEVICE_COPP_HANDLES     0x00FB // 251 ---used to be 0x00C1
                                                          //

/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for AUD_COPP_STREAM_HANDLES, refer to acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES 
@param[in] req_buf_ptr: serialized (little ENDIAN) of packed struct ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req 
   struct  ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req {
      uint32 copp_handle;
      uint32 num_of_popps;
   };
@param[in] req_buf_len: 8 
@param[out] resp_buf_ptr: serialized (little ENDIAN) of packed struct ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp 
   struct ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp {
      uint32 num_of_popp_ids;
      uint32 popp_ids[<num_of_popp_ids>];
   };
@param[out] resp_buf_len[out]: >= sizeof(ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp)
@param[out] resp_buf_bytes_filled[out]: sizeof(ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_rsp) 
*/
#define ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES     0x00FC // 252 ---used to be 0x00C2
 

/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for DSP 
       VOC_ALL_ACTIVE_STREAMS, refer to
       acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS 
@param[in] req_buf_ptr: NULL
@param[in] req_buf_len: 0 
@param[out] resp_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp struct.
 
   stuct   ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp {
      uint32 num_of_streams;
      struct VOC_ALL_ACTIVE_STREAMS[<num_of_streams>];
   };
   stuct   VOC_ALL_ACTIVE_STREAMS {
      uint32 popp_id;
      uint32 popp_handle;
   };
@param[out] resp_buf_len[out] 
   >= sizeof(ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp)
@param[out] resp_buf_bytes_filled[out]: 
   sizeof(ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS_rsp)
*/
#define ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS      0x00FD // 253 ---used to be 0x00C3
                                                          //

/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for DSP 
       VOC_VS_COPP_HANDLES, refer to
       acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES 
@param[in] req_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req struct.
 
   struct ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req {
      uint32 popp_handle;
   }
@param[in] req_buf_len: 4 
@param[out] resp_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp struct.
 
   stuct   ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp {
      uint32 num_of_copps;
      struct VOC_VS_COPP_HANDLES[<num_of_copp >];
   };
   stuct   VOC_VS_COPP_HANDLES {
      uint32 copp_id;
      uint32 copp_handle;
   };
@param[out] resp_buf_len[out] 
   >= sizeof(ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp)
@param[out] resp_buf_bytes_filled[out]: 
   sizeof(ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_rsp)
*/
#define ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES         0x00FE // 254 ---used to be 0x00C4
                                                          //


/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for DSP 
       VOC_VC_DEVICES, refer to
       acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_VOC_VC_DEVICES 
@param[in] req_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_VOC_VC_DEVICES_req struct.
 
   struct ACPH_CMD_QUERY_VOC_VC_DEVICES_req {
      uint32 copp_handle;
   }
@param[in] req_buf_len: 4 
@param[out] resp_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp struct.
 
   stuct   ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp {
      uint32 num_of_device_pairs;
      struct VOC_VC_DEVICES[<num_of_device_pairs >];
   };
   stuct   VOC_VC_DEVICES {
      uint32 rx_device_id;
      uint32 tx_device_id;
   };
@param[out] resp_buf_len[out] 
   >= sizeof(ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp)
@param[out] resp_buf_bytes_filled[out]: 
   sizeof(ACPH_CMD_QUERY_VOC_VC_DEVICES_rsp)
*/
#define ACPH_CMD_QUERY_VOC_VC_DEVICES              0x00FF // 255 ---used to be 0x00C5


/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for DSP 
       CAL_DATA, refer to
       acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_RTC_GET_CAL_DATA 
@param[in] req_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_RTC_GET_CAL_DATA_req struct.
 
   struct ACPH_CMD_RTC_GET_CAL_DATA_req {
      uint32 domain_id;
      uint32 service_id;
      uint32 pp_id; //copp or popp id
      uint32 module_id;
      uint32 parameter_id;
   };
@param[in] req_buf_len: 20
@param[out] resp_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_RTC_GET_CAL_DATA_rsp struct.
 
   stuct   ACPH_CMD_RTC_GET_CAL_DATA_rsp {
      uint32 module_id;
      uint32 parameter_id;
      uint16 length;
      uint16 reserved;
      uint8  data[<length>];
   };
@param[out] resp_buf_len[out] 
   >= sizeof(ACPH_CMD_RTC_GET_CAL_DATA_rsp)
@param[out] resp_buf_bytes_filled[out]: 
   sizeof(ACPH_CMD_RTC_GET_CAL_DATA_rsp)
*/
#define ACPH_CMD_RTC_GET_CAL_DATA                  0x0101 // 256 ---used to be 0x0041
                                                          //
/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to set DSP CAL_DATA, refer to acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_RTC_SET_CAL_DATA 
@param[in] req_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_RTC_SET_CAL_DATA_req struct.
 
   struct ACPH_CMD_RTC_SET_CAL_DATA_req {
      uint32 domain_id;
      uint32 service_id;
      uint32 pp_id; //copp or popp id
      uint32 module_id;
      uint32 parameter_id;
      uint16 length;
      uint16 reserved;
      uint8  data[<length>];
   };
@param[in] req_buf_len: sizeof(ACPH_CMD_RTC_SET_CAL_DATA_req)
@param[out] resp_buf_ptr: NULL
@param[out] resp_buf_len: 0 
@param[out] resp_buf_bytes_filled: 0
*/
#define ACPH_CMD_RTC_SET_CAL_DATA                  0x0102 // 257 ---used to be 0x0042



/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to QUERY_AUD_TOPOLOGY_COPP_HANDLES, refer to acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES 
@param[in] req_buf_ptr: NULL
@param[in] req_buf_len: 0
@param[out] resp_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp struct.
 
   struct ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp {
      uint32 num_of_entry;
      struct AUD_TOPOLOGY_COPP_HANDLES[<num_of_entry>];
   };
   struct AUD_TOPOLOGY_COPP_HANDLES {
      uint32 topology_Id;
      uint32 copp_handle;
      uint32 copp_id;
      uint32 num_of_popps;
   };
@param[out] resp_buf_len: >= sizeof(ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp)
@param[out] resp_buf_bytes_filled[out]: sizeof(ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES_rsp)
*/
//LA-only command
#define ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES 258;

/**
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to QUERY_VOC_VC_TOPOLOGIES, refer to 
       acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES 
@param[in] req_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req struct.
 
   struct ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req {
      uint32 copp_handle;
   };
 
@param[in] req_buf_len: 0
@param[out] resp_buf_ptr: serialized (little ENDIAN) of packed struct 
   ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp struct.
 
   struct ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp {
      uint32 num_of_entry;
      struct VOC_VC_TOPOLOGIES[<num_of_entry>];
   };
   struct VOC_VC_TOPOLOGIES {
      uint32 rx_topology_id;
      uint32 tx_topology_id;
   };
@param[out] resp_buf_len: >= 
      sizeof(ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp)
@param[out] resp_buf_bytes_filled[out]: 
      sizeof(ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_rsp)
*/
//LA-only command
#define ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES 259;


/*--------------------------------------------------
 ** ACPH ADIE RTC Service and Command range declarations
 *-------------------------------------------------*/
#define ACPH_ADIE_RTC_REG_SERVICEID 0x00000003

// Command IDs of value from 501 to 750 should be used only by DSP ADIE service
#define ACPH_ADIE_RTC_CMD_ID_START  0x01F5 //501
#define ACPH_ADIE_RTC_CMD_ID_END    0x02EE //200

// ADIE RTC protocal version 0 command definitions:
@fn  ACPH_CALLBACK_PTR (uint16_t nCommandId,
	                uint8_t *req_buf_ptr,
			uint32_t req_buf_len,
			uint8_t *resp_buf_ptr,
			uint32_t resp_buf_len,
			uint32_t *resp_buf_bytes_filled);
@brief API to query for ADIE RTC protocol version, refer to acph_register_command() and ACPH_CALLBACK_PTR

@param[in] nCommandId: ACPH_CMD_QUERY_ADIE_RTC_VERSION 
@param[in] req_buf_ptr: NULL 
@param[in] req_buf_len: 0 
@param[out] resp_buf_ptr[out]: serialized (little ENDIAN) of packed struct ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp 
   struct ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp {
      uint16_t	adie_rtc_major_version;
      uint16_t	adie_rtc_minor_version;
   }
@param[out] resp_buf_len[out]: >= sizeof (struct_ACPH_CMD_QUERY_RTC_VERSION_rsp)
@param[out] resp_buf_bytes_filled[out]: 4 (sizeof (struct_ACPH_CMD_QUERY_RTC_VERSION_rsp))
*/
#define ACPH_CMD_QUERY_ADIE_RTC_VERSION			   0x0501 //  ---new command



/*------------------------------------------
 ** Type Declratations
 *-------------------------------------------*/
typedef int32_t (*ACPH_CALLBACK_PTR)(uint16_t nCommandId,
						      uint8_t *req_buf_ptr,
							  uint32_t req_buf_len,
							  uint8_t *resp_buf_ptr,
							  uint32_t resp_buf_len,
							  uint32_t *resp_buf_bytes_filled
							  );


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
 * FUNCTION : acph_register_command
 *
 * DESCRIPTION : register command id into acph registry table
 * 
 *
 * DEPENDENCIES : get_command_length is supposed to be called before this function,
 * which will check the command length and make sure command id is there
 *
 * PARAMS: *   
 *   nService_id - corresponding to command id to the function name, 
*                 client must use this commandId to de-resiger the command
 *   function_ptr - a pointer to sumbit operation function name
 *
 * RETURN VALUE : ACPH_SUCCESS if the register with function pointer success; 
 *                ACPH_FAILURE otherwise. 
 *
 * SIDE EFFECTS : None
 */
int32_t acph_register_command(uint32_t nService_id,ACPH_CALLBACK_PTR fcn_ptr);

/**
 * FUNCTION : acph_deregister_command
 *
 * DESCRIPTION : deregister command into acph registry table
 * 
 *
 * DEPENDENCIES : get_command_length is supposed to be called before this function,
 * which will check the command length and make sure command id is there
 *
 * PARAMS: *   
 *   nService_id - corresponding to command id to the function name, it is dynamically
 *                created and client must use this commandId to de-resiger the command
 *
 * RETURN VALUE : ACPH_SUCCESS if the register with function pointer success; 
 *                ACPH_FAILURE otherwise. 
 *
 * SIDE EFFECTS : None
 */
int32_t acph_deregister_command(uint32_t nService_id);



#endif //_AUDCAL_ACPH_H_

