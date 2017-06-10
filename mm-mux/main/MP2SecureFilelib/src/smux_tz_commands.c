/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                 Securemux

GENERAL DESCRIPTION
	Secure mux TrustZone commands.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                            EDIT HISTORY FOR MODULE

$Header:

when       who    what, where, why
--------   ---    ----------------------------------------------------------
09/30/2013  yb    Created
===========================================================================*/
#include <ctype.h>
#include <stdio.h>
#include "QSEEComAPI.h"
#include "smux_tz_commands.h"
#include <utils/Log.h>
#include "common_log.h"

/* commands supported  */
#define SECURE_MUX_CMD0_INITIALIZE 0
#define SECURE_MUX_CMD1_PROCESS_SAMPLE 1

#define MUX_MAX_MEDIA_STREAMS 6

typedef struct smux_generic_req_s {
	uint32_t cmd_id;
} smux_generic_req_t;

/*-------------------------------------------------------------------------*/

typedef struct smux_generic_rsp_s {
	smux_status_t ret;
} smux_generic_rsp_t;

/*-------------------------------------------------------------------------*/

typedef struct smux_config_req_s {
	uint32_t cmd_id;
   smux_config_t sCfg;
   smux_buff_descr_t PAT_PMT_PCR_buffer;
} smux_config_req_t;

/*-------------------------------------------------------------------------*/

typedef struct smux_config_rsp_s {
	smux_status_t       ret;
} smux_config_rsp_t;
/*-------------------------------------------------------------------------*/

typedef struct smux_proc_req_s {
	uint32_t cmd_id;
	smux_sample_info_t sampleInfo;
} smux_proc_req_t;

/*-------------------------------------------------------------------------*/

typedef struct smux_proc_rsp_s {
	smux_status_t       ret;
   uint32_t  nInOffset;   /* Offset in the input buffer                        */
   uint32_t  nPCROffset;  /* Offset in output buffer to write                  */
   uint32_t  nUserDataOffset;  /* Offset in output buffer to write             */
   uint32_t nOutputOffset;
} smux_proc_rsp_t;

/*-------------------------------------------------------------------------*/

static void get_cmd_rsp_buffers(struct QSEECom_handle *handle, void **cmd, int *cmd_len, void **rsp, int *rsp_len)
{
		*cmd = handle->ion_sbuffer;

		if (*cmd_len & QSEECOM_ALIGN_MASK)
			*cmd_len = QSEECOM_ALIGN(*cmd_len);

		*rsp = handle->ion_sbuffer + *cmd_len;

		if (*rsp_len & QSEECOM_ALIGN_MASK)
			*rsp_len = QSEECOM_ALIGN(*rsp_len);

}

/*-------------------------------------------------------------------------*/
smux_status_t SMUX_config(struct QSEECom_handle *handle , smux_config_t *configArgs ,struct smux_ion_info *tables_ihandle)
{
	int res, cmd_len, rsp_len;
	smux_config_req_t *cmd;
	smux_config_rsp_t *rsp;
	smux_status_t ret;
	struct QSEECom_ion_fd_info  ion_fd_info;

	memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));
	ion_fd_info.data[0].fd = tables_ihandle->ifd_data_fd;
	ion_fd_info.data[0].cmd_buf_offset = (char*)&(cmd->PAT_PMT_PCR_buffer.base_addr) - (char*)cmd;

	cmd_len = sizeof(smux_config_req_t);
	rsp_len = sizeof(smux_config_rsp_t);

	/* Get command and response buffers */
	get_cmd_rsp_buffers( handle, (void**)&cmd, &cmd_len, (void**)&rsp, &rsp_len);

	/* Populate command struct */
	cmd->cmd_id = SECURE_MUX_CMD0_INITIALIZE;
	cmd->PAT_PMT_PCR_buffer.length = tables_ihandle->sbuf_len;

	memcpy(&cmd->sCfg, configArgs, sizeof(smux_config_t));

	/* Issue QSEECom command */
	res = QSEECom_send_modified_cmd(handle, (void *)cmd, cmd_len,
			(void *)rsp, rsp_len, &ion_fd_info);

	if (res < 0)
	{
		LOGE("QSEECom_send_modified_cmd fail\n");
		return SEC_MUX_GENERAL_FAILURE;
	}

	/* Parse response struct */
	ret = rsp->ret;

	return ret;
}

/*-------------------------------------------------------------------------*/
smux_status_t SMUX_Process_Sample(struct QSEECom_handle *handle ,smux_sample_info_t* sample_info,
		struct smux_ion_info *input_ihandle,struct smux_ion_info *output_ihandle,struct smux_ion_info *userdata_ihandle,
	    uint32_t*  nInOffset, uint32_t*  nPCROffset, uint32_t*  nUserDataOffset,uint32_t* nOutputOffset)
{
	int res, cmd_len, rsp_len;
	smux_proc_req_t *cmd;
	smux_proc_rsp_t *rsp;
	smux_status_t ret;
	struct QSEECom_ion_fd_info  ion_fd_info;

	memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));

	ion_fd_info.data[0].fd = input_ihandle->ifd_data_fd;
	ion_fd_info.data[0].cmd_buf_offset = (char*)&(cmd->sampleInfo.inputSample.buff_chunks[0].base_addr) - (char*)cmd;
	ion_fd_info.data[1].fd = output_ihandle->ifd_data_fd;
	ion_fd_info.data[1].cmd_buf_offset = (char*)&(cmd->sampleInfo.outputBuffer.buff_chunks[0].base_addr) - (char*)cmd;
	if(userdata_ihandle)
	{
		ion_fd_info.data[2].fd = userdata_ihandle->ifd_data_fd;
		ion_fd_info.data[2].cmd_buf_offset = (char*)&(cmd->sampleInfo.userData.base_addr) - (char*)cmd;
	}

	cmd_len = sizeof(smux_proc_req_t);
	rsp_len = sizeof(smux_proc_rsp_t);

	/* Get command and response buffers */
	get_cmd_rsp_buffers( handle, (void**)&cmd, &cmd_len, (void**)&rsp, &rsp_len);

	/* Populate command struct */
	cmd->cmd_id = SECURE_MUX_CMD1_PROCESS_SAMPLE;
	memcpy(&cmd->sampleInfo, sample_info, sizeof(smux_sample_info_t));

	/* Issue QSEECom command */
	res = QSEECom_send_modified_cmd(handle, (void *)cmd, cmd_len,
			(void *)rsp, rsp_len, &ion_fd_info);

	if (res < 0)
	{
		LOGE("QSEECom_send_modified_cmd fail\n");
		return SEC_MUX_GENERAL_FAILURE;
	}

	/* Parse response struct */
	ret = rsp->ret;

	*nInOffset = rsp->nInOffset;
	*nPCROffset = rsp->nPCROffset;
	*nUserDataOffset = rsp->nUserDataOffset;
	*nOutputOffset = rsp->nOutputOffset;

	return ret;
}

