/**
  \file **************************************************************************
 *
 *  A U D I O   C A L I B R A T I O N   P A C K E T   H A N D L E R
 *
 *DESCRIPTION
 * This file contains the implementation of online_intf
 *
 *REFERENCES
 * None.
 *
 *Copyright (c) 2011-2014 by Qualcomm Technologies, Inc.  All Rights Reserved.
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
 *  $Header: acph.h
 *
 *when         who     what, where, why
 *--------     ---     ----------------------------------------------------------
 *05/28/14     mh      SW migration from 32-bit to 64-bit architecture
 *02/14/14     avi     Support commands for ACDB persistence.
 *06/07/13     avi     Support Voice Volume boost feature
 *08/03/11     ernanl  initial draft
 ********************************************************************************
 */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acph_online/src/acph_online.c#11 $ */
/*
   -------------------------------
   |Include Files                |
   -------------------------------
   */

#include "acdb_os_includes.h"
#include "acph_online.h"
#include "acdb_command.h"
#include "acdb_datainfo.h"
/*===========================================================================
  Macro
  ===========================================================================*/

const uint32_t ACDB_TARGET_VERSION = 0x00012A7B;

/*===========================================================================
  External VARIABLES
  ===========================================================================*/
//extern char_t * acph_main_buffer;

/*===========================================================================
  Internal VARIABLE
  ===========================================================================*/

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
static int32_t get_target_version (uint8_t *req_buf_ptr,
								   uint32_t req_buf_len,
								   uint8_t *resp_buf_ptr,
								   uint32_t resp_buf_length,
								   uint32_t *resp_buf_bytes_filled
								   )
{
    if ((NULL == resp_buf_ptr) || (resp_buf_length < sizeof(uint32_t)))
    {
		return ACPH_ERR_OUT_OF_BUFFER_SIZE;
    }
    else
    {
		ACDB_MEM_CPY((void*)(resp_buf_ptr),sizeof(uint32_t),(void*)&ACDB_TARGET_VERSION,sizeof(uint32_t));
		*resp_buf_bytes_filled = sizeof(uint32_t);
		return ACPH_SUCCESS;
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
static int32_t check_connection(uint8_t *req_buf_ptr,
											 uint32_t req_buf_len,
											 uint8_t *resp_buf_ptr,
											 uint32_t resp_buf_length,
											 uint32_t *resp_buf_bytes_filled
											 )
{
	return ACPH_SUCCESS;
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
static int32_t query_max_buffer_length(uint8_t *req_buf_ptr,
								    uint32_t req_buf_len,
									uint8_t *resp_buf_ptr,
									uint32_t resp_buf_length,
									uint32_t *resp_buf_bytes_filled
									)
{
	uint32_t acph_buf_len = resp_buf_length;

    if ((NULL == resp_buf_ptr) || (resp_buf_length < sizeof(uint32_t)))
    {
		return ACPH_ERR_OUT_OF_BUFFER_SIZE;
    }
    else
    {
		ACDB_MEM_CPY((void*)(resp_buf_ptr),sizeof(uint32_t),(void*)&acph_buf_len,sizeof(uint32_t));
		*resp_buf_bytes_filled = sizeof(uint32_t);
		return ACPH_SUCCESS;
    }
}

/**
 * FUNCTION : get_acdb_files_info
 *
 * DESCRIPTION : get the files info which were currently loaded in the memory
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
static int32_t get_acdb_files_info(uint8_t *req_buf_ptr,
								uint32_t req_buf_len,
								uint8_t *resp_buf_ptr,
								uint32_t resp_buf_length,
								uint32_t *resp_buf_bytes_filled
								)
{
	//uint32_t acph_buf_len = resp_buf_length;
	AcdbQueryCmdType cmd;
	AcdbQueryResponseType rsp;
	int32_t result = ACPH_SUCCESS;
    if (NULL == resp_buf_ptr)
    {
        /**not initilized*/
        return ACPH_ERR_OUT_OF_BUFFER_SIZE;
    }

	cmd.nBufferLength = resp_buf_length;
	cmd.pBufferPointer = resp_buf_ptr;
	result = AcdbCmdGetFilesInfo(&cmd,&rsp);
	if(result == ACPH_SUCCESS)
		*resp_buf_bytes_filled = rsp.nBytesUsedInBuffer;
	return result;
}

/**
 * FUNCTION : get_no_of_tbl_entries_on_heap
 *
 * DESCRIPTION : get the heap info which were currently loaded in the memory
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
static int32_t get_no_of_tbl_entries_on_heap(uint8_t *req_buf_ptr,
											 uint32_t req_buf_len,
											 uint8_t *resp_buf_ptr,
											 uint32_t resp_buf_length,
											 uint32_t *resp_buf_bytes_filled
											 )
{
    //uint32_t acph_buf_len = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
	int32_t result = ACPH_SUCCESS;

	AcdbQueryNoOfTblEntriesCmdType cmd;
	AcdbRespNoOfTblEntriesCmdType rsp = {0};
	if (NULL == resp_buf_ptr)
    {
        /**not initilized*/
		return ACPH_ERR_OUT_OF_BUFFER_SIZE;
    }

	if(req_buf_len != sizeof(AcdbQueryNoOfTblEntriesCmdType))
    {
        /**command parameter missing*/
		ACDB_DEBUG_LOG("Invalid getheapdata request made to target from client.Insufficient data provided to process the req");
        return ACPH_ERR_ACDB_COMMAND_FAILURE;
    }
	if(resp_buf_length < sizeof(rsp.nNoOfEntries))
	{
		return ACPH_ERR_OUT_OF_BUFFER_SIZE;
	}
	ACDB_MEM_CPY((void *)&cmd.nTblId,sizeof(cmd.nTblId),req_buf_ptr ,sizeof(cmd.nTblId));

	result = AcdbCmdGetNoOfTblEntriesOnHeap((uint8_t*)&cmd,sizeof(cmd),(uint8_t*)&rsp,sizeof(rsp));
	if(result == ACPH_SUCCESS)
	{
		ACDB_MEM_CPY( (resp_buf_ptr),sizeof(rsp.nNoOfEntries),&rsp.nNoOfEntries,sizeof(rsp.nNoOfEntries));
		*resp_buf_bytes_filled = sizeof(rsp.nNoOfEntries);
	}
	return result;
}

/**
 * FUNCTION : get_tbl_entries_on_heap
 *
 * DESCRIPTION : get the heap info which were currently loaded in the memory
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
static int32_t get_tbl_entries_on_heap(uint8_t *req_buf_ptr,
									uint32_t req_buf_len,
									uint8_t *resp_buf_ptr,
									uint32_t resp_buf_length,
									uint32_t *resp_buf_bytes_filled
									)
{
    //uint32_t acph_buf_len = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
	int32_t result = ACPH_SUCCESS;

	uint32_t offset = 0;

	AcdbQueryTblEntriesCmdType cmd;
	AcdbQueryResponseType rsp = {0};
	if (NULL == resp_buf_ptr)
    {
        /**not initilized*/
		return ACPH_ERR_OUT_OF_BUFFER_SIZE;
    }

	if(req_buf_len != (2*sizeof(uint32_t)))
    {
        /**command parameter missing*/
		ACDB_DEBUG_LOG("Invalid getheapdata request made to target from client.Insufficient data provided to process the req");
        return ACPH_ERR_ACDB_COMMAND_FAILURE;
    }
	ACDB_MEM_CPY((void *)&cmd.nTblId,sizeof(cmd.nTblId),req_buf_ptr + offset,sizeof(cmd.nTblId));
	offset += sizeof(cmd.nTblId);

	ACDB_MEM_CPY((void *)&cmd.nTblEntriesOffset,sizeof(cmd.nTblEntriesOffset),req_buf_ptr + offset,sizeof(cmd.nTblEntriesOffset));
	offset += sizeof(cmd.nTblEntriesOffset);

	//memcpy((void *)&cmd.nRequiredNoOfTblEntries,req_buf_ptr + offset,sizeof(cmd.nRequiredNoOfTblEntries));
	//offset += sizeof(cmd.nRequiredNoOfTblEntries);

	cmd.pBuff = resp_buf_ptr;
	cmd.nBuffSize = resp_buf_length;

	result = AcdbCmdGetTblEntriesOnHeap((uint8_t*)&cmd,sizeof(cmd),(uint8_t*)&rsp,sizeof(rsp));
	if(result == ACPH_SUCCESS)
	{
		*resp_buf_bytes_filled = rsp.nBytesUsedInBuffer;
	}
	return result;
}

/**
 * FUNCTION : get_acdb_file
 *
 * DESCRIPTION : gets the acdb file which was requested by client
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
static int32_t get_acdb_file(uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
						  uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_length,
						  uint32_t *resp_buf_bytes_filled
						  )
{
    //uint32_t acph_buf_len = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
	AcdbCmdGetFileDataReq req;
	AcdbCmdResp resp;
	uint32_t offset = 0;
	int32_t result = ACPH_SUCCESS;
	if (NULL == resp_buf_ptr)
    {
        /**not initilized*/
		return ACPH_ERR_OUT_OF_BUFFER_SIZE;
    }
	ACDB_MEM_CPY(&req.nfile_offset,sizeof(req.nfile_offset),req_buf_ptr + offset,sizeof(req.nfile_offset));
	offset += sizeof(req.nfile_offset);
	ACDB_MEM_CPY(&req.nfile_data_len,sizeof(req.nfile_data_len),req_buf_ptr + offset,sizeof(req.nfile_data_len));
	offset += sizeof(req.nfile_data_len);
	ACDB_MEM_CPY(&req.nfileNameLen,sizeof(req.nfileNameLen),req_buf_ptr + offset,sizeof(req.nfileNameLen));
	offset += sizeof(req.nfileNameLen);
	req.pFileName = req_buf_ptr + offset;

	resp.pRespBuff = resp_buf_ptr;
	resp.nresp_buff_len = resp_buf_length;

	result = AcdbCmdGetFileData(&req,&resp);
	if(result == ACPH_SUCCESS)
	{
		*resp_buf_bytes_filled = resp.nresp_buff_filled;
	}
	return result;
}

/**
 * FUNCTION : get_acdb_data
 *
 * DESCRIPTION : gets the acdb data which was requested by client
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
static int32_t get_acdb_data(uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
						  uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_length,
						  uint32_t *resp_buf_bytes_filled
						  )
{
    //uint32_t acph_buf_len = ACPH_BUFFER_LENGTH - ACPH_ACDB_BUFFER_POSITION;
	uint32_t offset = 0;
	//uint32_t reqdatalen = 0;
	int32_t result = ACPH_SUCCESS;
	int32_t tblId = 0;
	int32_t mid =0;
	int32_t pid =0;
	uint8_t *pIndices=NULL;
	uint32_t nBytesFilled = 0;
    uint32_t noOfTableIndices = 0;
    uint32_t nonModuleTblFound = 0;
	uint8_t *pRspBuff=NULL;

	if (NULL == resp_buf_ptr)
    {
        /**not initilized*/
        return ACPH_ERR_UNKNOWN_REASON;
    }
	//memcpy((void *)&reqdatalen,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,ACPH_DATA_LENGTH_LENGTH);
	if( (req_buf_len == 0 ) || ((req_buf_len % 4)!=0))
    {
        /**command parameter missing*/
		ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Insufficient data provided to process the req");
        return ACPH_ERR_ACDB_COMMAND_FAILURE;
    }
	ACDB_MEM_CPY((void *)&tblId,sizeof(tblId),req_buf_ptr + offset,sizeof(tblId));
	offset += sizeof(uint32_t);
	switch(tblId)
	{
	   case AUDPROC_GAIN_INDP_TBL:
		   noOfTableIndices = AUDPROCTBL_INDICES_COUNT;
		   break;
	   case AUDPROC_COPP_GAIN_DEP_TBL:
		   noOfTableIndices = AUDPROCTBL_GAIN_DEP_INDICES_COUNT;
		   break;
	   case AUDPROC_AUD_VOL_TBL:
		   noOfTableIndices = AUDPROCTBL_VOL_INDICES_COUNT;
		   break;
	   case AUD_STREAM_TBL:
		   noOfTableIndices = AUDSTREAMTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_GAIN_INDP_TBL:
		   noOfTableIndices = VOCPROCTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_COPP_GAIN_DEP_TBL:
		   noOfTableIndices = VOCPROCTBL_VOL_INDICES_COUNT;
		   break;
	   case VOC_STREAM_TBL:
		   noOfTableIndices = VOCSTREAMTBL_INDICES_COUNT;
		   break;
	   case AFE_TBL:
		   noOfTableIndices = AFETBL_INDICES_COUNT;
		   break;
	   case AFE_CMN_TBL:
		   noOfTableIndices = AFECMNTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_DEV_CFG_TBL:
		   noOfTableIndices = VOCPROCDEVCFGTBL_INDICES_COUNT;
		   break;
	   case ADIE_ANC_TBL:
		   noOfTableIndices = ANCTBL_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case ADIE_CODEC_TBL:
		   noOfTableIndices = ANCTBL_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case GLOBAL_DATA_TBL:
		   noOfTableIndices = GLOBALTBL_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case LSM_TBL:
		   noOfTableIndices = LSM_INDICES_COUNT;
		   break;
	   case CDC_FEATURES_TBL:
		   noOfTableIndices = CDC_FEATURES_DATA_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case ADIE_SIDETONE_TBL:
		   noOfTableIndices = ADST_INDICES_COUNT;
		   break;
	   case AANC_CFG_TBL:
		   noOfTableIndices = AANC_CFG_TBL_INDICES_COUNT;
		   break;
      case VOCPROC_COPP_GAIN_DEP_V2_TBL:
         noOfTableIndices = VOCPROCTBL_VOL_V2_INDICES_COUNT;
         break;
      case VOICE_VP3_TBL:
         noOfTableIndices = VOICE_VP3TBL_INDICES_COUNT;
         break;
      case AUDIO_REC_VP3_TBL:
         noOfTableIndices = AUDREC_VP3TBL_INDICES_COUNT;
         break;
      case AUDIO_REC_EC_VP3_TBL:
           noOfTableIndices = AUDREC_EC_VP3TBL_INDICES_COUNT;
           break;
	   case METAINFO_LOOKUP_TBL:
		   noOfTableIndices = MINFOTBL_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
     case VOCPROC_DYNAMIC_TBL:
		   noOfTableIndices = VOCPROCDYNTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_STATIC_TBL:
		   noOfTableIndices = VOCPROCSTATTBL_INDICES_COUNT;
		   break;
	   case VOC_STREAM2_TBL:
		   noOfTableIndices = VOCSTREAM2TBL_INDICES_COUNT;
		   break;

	   default:
		   ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Provided invalid tableid");
         return ACPH_ERR_ACDB_COMMAND_FAILURE;
	}

	if( (nonModuleTblFound == 0) &&
		(req_buf_len != (sizeof(tblId) + (noOfTableIndices*sizeof(uint32_t)) + sizeof(mid) + sizeof(pid))))
	{
		ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Provided insufficient no of table params");
        return ACPH_ERR_ACDB_COMMAND_FAILURE;
	}
	else if( (nonModuleTblFound == 1) &&
		(req_buf_len != (sizeof(tblId) + (noOfTableIndices*sizeof(uint32_t)) )))
	{
		ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Provided insufficient no of table params");
        return ACPH_ERR_ACDB_COMMAND_FAILURE;
	}

	pIndices = (uint8_t *)(req_buf_ptr + offset);
	offset += (noOfTableIndices*sizeof(uint32_t));
	if(nonModuleTblFound == 0)
	{
		ACDB_MEM_CPY((void *)&mid,sizeof(uint32_t),(req_buf_ptr+offset),sizeof(uint32_t));
		offset += sizeof(uint32_t);
		ACDB_MEM_CPY((void *)&pid,sizeof(uint32_t),(req_buf_ptr+offset),sizeof(uint32_t));
	}

	pRspBuff = resp_buf_ptr;

	result = AcdbCmdGetOnlineData(tblId,pIndices,noOfTableIndices,mid,pid,pRspBuff,resp_buf_length,&nBytesFilled);
	if(result == ACPH_SUCCESS)
	{
		*resp_buf_bytes_filled = nBytesFilled;
	}
	return result;
}

/**
 * FUNCTION : set_acdb_data
 *
 * DESCRIPTION : set the acdb file which was requested by client
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
static int32_t set_acdb_data(uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
						  uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_length,
						  uint32_t *resp_buf_bytes_filled
						  )
{
	uint32_t offset = 0;
	int32_t result = ACPH_SUCCESS;
	uint32_t tblId = 0;
	int32_t mid =0;
	int32_t pid =0;
	uint8_t *pIndices=NULL;
        uint32_t noOfTableIndices = 0;
        uint32_t nonModuleTblFound = 0;
	uint8_t *pInBuff=NULL;
	uint32_t nInDataBufLen = 0;
   uint32_t persistData = FALSE;
   uint32_t persistanceSupported = FALSE;
   int32_t persistResult = ACDB_ERROR;

	//memcpy((void *)&reqdatalen,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,ACPH_DATA_LENGTH_LENGTH);
	if( req_buf_len== 0 )
        {
        /**command parameter missing*/
		ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Insufficient data provided to process the req");
                return ACPH_ERR_ACDB_COMMAND_FAILURE;
        }
	else if ( (req_buf_len% 4)!=0)
	{
		ACDB_DEBUG_LOG("ACPH:Warning The set data request provided is not 4 byte aligned");
	}
	ACDB_MEM_CPY((void *)&tblId,sizeof(tblId),req_buf_ptr + offset,sizeof(tblId));
	offset += sizeof(uint32_t);
	switch(tblId)
	{
	   case AUDPROC_GAIN_INDP_TBL:
		   noOfTableIndices = AUDPROCTBL_INDICES_COUNT;
		   break;
	   case AUDPROC_COPP_GAIN_DEP_TBL:
		   noOfTableIndices = AUDPROCTBL_GAIN_DEP_INDICES_COUNT;
		   break;
	   case AUDPROC_AUD_VOL_TBL:
		   noOfTableIndices = AUDPROCTBL_VOL_INDICES_COUNT;
		   break;
	   case AUD_STREAM_TBL:
		   noOfTableIndices = AUDSTREAMTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_GAIN_INDP_TBL:
		   noOfTableIndices = VOCPROCTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_COPP_GAIN_DEP_TBL:
		   noOfTableIndices = VOCPROCTBL_VOL_INDICES_COUNT;
		   break;
	   case VOC_STREAM_TBL:
		   noOfTableIndices = VOCSTREAMTBL_INDICES_COUNT;
		   break;
	   case AFE_TBL:
		   noOfTableIndices = AFETBL_INDICES_COUNT;
		   break;
	   case AFE_CMN_TBL:
		   noOfTableIndices = AFECMNTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_DEV_CFG_TBL:
		   noOfTableIndices = VOCPROCDEVCFGTBL_INDICES_COUNT;
		   break;
	   case ADIE_ANC_TBL:
		   noOfTableIndices = ANCTBL_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case ADIE_CODEC_TBL:
		   noOfTableIndices = ANCTBL_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case GLOBAL_DATA_TBL:
		   noOfTableIndices = GLOBALTBL_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case LSM_TBL:
		   noOfTableIndices = LSM_INDICES_COUNT;
		   break;
	   case CDC_FEATURES_TBL:
		   noOfTableIndices = CDC_FEATURES_DATA_INDICES_COUNT;
		   nonModuleTblFound = 1;
		   break;
	   case ADIE_SIDETONE_TBL:
		   noOfTableIndices = ADST_INDICES_COUNT;
		   break;
	   case AANC_CFG_TBL:
		   noOfTableIndices = AANC_CFG_TBL_INDICES_COUNT;
		   break;
      case VOCPROC_COPP_GAIN_DEP_V2_TBL:
         noOfTableIndices = VOCPROCTBL_VOL_V2_INDICES_COUNT;
         break;
      case VOICE_VP3_TBL:
         noOfTableIndices = VOICE_VP3TBL_INDICES_COUNT;
         break;
      case AUDIO_REC_VP3_TBL:
         noOfTableIndices = AUDREC_VP3TBL_INDICES_COUNT;
         break;
      case AUDIO_REC_EC_VP3_TBL:
         noOfTableIndices = AUDREC_EC_VP3TBL_INDICES_COUNT;
         break;
      case METAINFO_LOOKUP_TBL:
         noOfTableIndices = MINFOTBL_INDICES_COUNT;
         nonModuleTblFound = 1;
         break;
       case VOCPROC_DYNAMIC_TBL:
		   noOfTableIndices = VOCPROCDYNTBL_INDICES_COUNT;
		   break;
	   case VOCPROC_STATIC_TBL:
		   noOfTableIndices = VOCPROCSTATTBL_INDICES_COUNT;
		   break;
	   case VOC_STREAM2_TBL:
		   noOfTableIndices = VOCSTREAM2TBL_INDICES_COUNT;
		   break;
      default:
         ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Provided invalid tableid");
         return ACPH_ERR_UNKNOWN_REASON;


	}

	if( (nonModuleTblFound == 0) &&
		(req_buf_len <= (sizeof(tblId) + (noOfTableIndices*sizeof(uint32_t)) + sizeof(mid) + sizeof(pid))))
	{
		ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Provided insufficient no of table params");
        return ACPH_ERR_ACDB_COMMAND_FAILURE;
	}
	else if( (nonModuleTblFound == 1) &&
		(req_buf_len <= (sizeof(tblId) + (noOfTableIndices*sizeof(uint32_t)) )))
	{
		ACDB_DEBUG_LOG("Invalid getdata request made to target from client.Provided insufficient no of table params");
        return ACPH_ERR_ACDB_COMMAND_FAILURE;
	}

	pIndices = (uint8_t *)(req_buf_ptr + offset);
	offset += (noOfTableIndices*sizeof(uint32_t));
	if(nonModuleTblFound == 0)
	{
		ACDB_MEM_CPY((void *)&mid,sizeof(uint32_t),(req_buf_ptr+offset),sizeof(uint32_t));
		offset += sizeof(uint32_t);
		ACDB_MEM_CPY((void *)&pid,sizeof(uint32_t),(req_buf_ptr+offset),sizeof(uint32_t));
		offset += sizeof(uint32_t);
	}

	pInBuff = req_buf_ptr + offset;
	if(nonModuleTblFound == 0)
	{
		nInDataBufLen = req_buf_len-( (noOfTableIndices*sizeof(uint32_t))+sizeof(tblId)+sizeof(mid)+sizeof(pid));
	}
	else
	{
		nInDataBufLen = req_buf_len-( (noOfTableIndices*sizeof(uint32_t))+sizeof(tblId));
	}

   result = AcdbCmdSetOnlineData(persistData, tblId,pIndices,noOfTableIndices,mid,pid,pInBuff,nInDataBufLen);

   if(persistData == TRUE)
   {
      persistResult = AcdbCmdIsPersistenceSupported(&persistanceSupported);
      if(persistResult == ACDB_SUCCESS)
      {
         if(persistanceSupported == TRUE)
         {
            result = AcdbCmdSaveDeltaFileData();
            if(result != ACDB_SUCCESS)
            {
               ACDB_DEBUG_LOG("[ACPH Online]->[set_acdb_data]->Unable to save delta file data\n");
            }
         }
      }
   }
	return result;
}

/**
 * FUNCTION : query_online_version
 *
 * DESCRIPTION : retrieve the version of ACPH Online
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
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
static int32_t query_online_version(uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
						  uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_length,
						  uint32_t *resp_buf_bytes_filled
						  )
{
    if ((NULL == resp_buf_ptr) || (resp_buf_length < 2*sizeof(uint32_t)))
    {
		return ACPH_ERR_OUT_OF_BUFFER_SIZE;
    }
    else
    {
		ACPH_CMD_QUERY_ONLINE_VERSION_rsp rsp;
		rsp.online_major_version = ACPH_SERVICE_MAJOR_VERSION_1;
		rsp.online_minor_version = ACPH_SERVICE_MINOR_VERSION_0;
		ACDB_MEM_CPY((void*)(resp_buf_ptr),sizeof(ACPH_CMD_QUERY_ONLINE_VERSION_rsp),(void*)&rsp,sizeof(ACPH_CMD_QUERY_ONLINE_VERSION_rsp));
		*resp_buf_bytes_filled = sizeof(ACPH_CMD_QUERY_ONLINE_VERSION_rsp);
		return ACPH_SUCCESS;
    }
}

/**
 * FUNCTION : support_acdb_persistence
 *
 * DESCRIPTION : Get information of acdb persistence support.
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
static int32_t support_acdb_persistence(uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
						  uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_length,
						  uint32_t *resp_buf_bytes_filled
						  )
{
	int32_t result = ACPH_SUCCESS;
   uint32_t response = FALSE;

   result = AcdbCmdIsPersistenceSupported(&response);

	if(result == ACPH_SUCCESS)
	{
      ACDB_MEM_CPY((void*)(resp_buf_ptr),sizeof(response),(void*)&response,sizeof(response));
		*resp_buf_bytes_filled = sizeof(response);
	}

	return result;
}

/**
 * FUNCTION : delete_delta_acdb_files
 *
 * DESCRIPTION : Deletes all the delta acdb files present.
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
static int32_t delete_delta_acdb_files(uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
						  uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_length,
						  uint32_t *resp_buf_bytes_filled
						  )
{
	int32_t result = ACPH_SUCCESS;
   int32_t response = ACDB_ERROR;

   result = AcdbCmdDeleteAllDeltaFiles(&response);

	if(result == ACPH_SUCCESS)
	{
      ACDB_MEM_CPY((void*)(resp_buf_ptr),sizeof(response),(void*)&response,sizeof(response));
		*resp_buf_bytes_filled = sizeof(response);
	}

	return result;
}

/*
   ----------------------------------
   | Externalized Function Definitions    |
   ----------------------------------
   */
/**
 * FUNCTION : acph_online_ioctl
 *
 * DESCRIPTION : acph online function call
 *
 * DEPENDENCIES : NONE
 *
 * PARAMS:
 *   nCommandId - command Id;
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : ACPH_SUCCESS/ACPH_FAILURE/ACPH_ERR_INVALID_COMMAND
 *
 * SIDE EFFECTS : None
 */
int32_t acph_online_ioctl(uint16_t nCommandId,
                          uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
                          uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_len,
                          uint32_t *resp_buf_bytes_filled
                          )
{
   int32_t result = ACPH_SUCCESS;

   int32_t (*fcnPtr)(uint8_t *req_buf_ptr,
					 uint32_t req_buf_len,
					 uint8_t *resp_buf_ptr,
					 uint32_t resp_buf_length,
					 uint32_t *resp_buf_bytes_filled
					 ) = NULL;

   switch(nCommandId)
   {
   case ACPH_CMD_GET_TARGET_VERSION:
      fcnPtr = get_target_version;
      break;
   case ACPH_CMD_CHECK_CONNECTION:
      fcnPtr = check_connection;
      break;
   case ACPH_CMD_QUERY_MAX_BUFFER_LENGTH:
      fcnPtr = query_max_buffer_length;
      break;
   case ACPH_CMD_GET_ACDB_FILES_INFO:
	   fcnPtr = get_acdb_files_info;
	   break;
   case ACPH_CMD_GET_ACDB_FILE:
	   fcnPtr = get_acdb_file;
	   break;
   case ACPH_CMD_GET_NO_OF_TBL_ENTRIES_ON_HEAP:
	   fcnPtr = get_no_of_tbl_entries_on_heap;
	   break;
   case ACPH_CMD_GET_TBL_ENTRIES_ON_HEAP:
	   fcnPtr = get_tbl_entries_on_heap;
	   break;
   case ACPH_CMD_GET_ACDB_DATA:
	   fcnPtr = get_acdb_data;
	   break;
   case ACPH_CMD_SET_ACDB_DATA:
	   fcnPtr = set_acdb_data;
	   break;
   case ACPH_CMD_QUERY_ONLINE_VERSION:
	   fcnPtr = query_online_version;
	   break;
   case ACPH_CMD_IS_PERSISTENCE_SUPPORTED:
      fcnPtr = support_acdb_persistence;
      break;
   case ACPH_CMD_DELETE_DELTA_ACDB_FILES:
	   fcnPtr = delete_delta_acdb_files;
	   break;
   default:
       result = ACPH_ERR_INVALID_COMMAND;
   }
   //ACDB_DEBUG_LOG("ACPH Online:: CommandID: %x \n", nCommandId);
   if (result == ACPH_SUCCESS)
   {
      result = fcnPtr(req_buf_ptr,
		      req_buf_len,
			  resp_buf_ptr,
			  resp_buf_len,
			  resp_buf_bytes_filled);
   }

   return result;
}

/**
 * FUNCTION : acph_online_init
 *
 * DESCRIPTION : Initialize online calibration
 *
 * DEPENDENCIES : NONE
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : ACPH_SUCCESS or ACPH_FAILURE
 *
 * SIDE EFFECTS : None
 */
int32_t acph_online_init(void)
{
   int32_t result = ACPH_SUCCESS;

   result = acph_register_command(ACPH_ONLINE_REG_SERVICEID,acph_online_ioctl);
   if (result != ACPH_SUCCESS)
   {
      ACDB_DEBUG_LOG("[acph_online_intf]->acph_online_init->register command[acph_online_ioctl] failed\n");
      goto end;
   }

end:
   return result;
}

/**
 * FUNCTION : acph_online_deinit
 *
 * DESCRIPTION : De-initialize online calibration
 *
 * DEPENDENCIES : NONE
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : ACPH_SUCCESS or ACPH_FAILURE
 *
 * SIDE EFFECTS : None
 */
int32_t acph_online_deinit(void)
{
   int32_t result = ACPH_SUCCESS;

   result = acph_deregister_command(ACPH_ONLINE_REG_SERVICEID);
   if (result != ACPH_SUCCESS)
   {
      ACDB_DEBUG_LOG("[acph_online_intf]->acph_online_deinit->unregister command[acph_online_ioctl] failed\n");
      goto end;
   }

end:
   return result;
}
