#ifndef _ONLINE_INTF_H_
#define _ONLINE_INTF_H_
/**
  \file **************************************************************************
 *
 *                    Online_Intf   H E A D E R    F I L E
 *
 *DESCRIPTION
 * This header file contains all the definitions necessary for Audio Calibration
 * Packet Handler to handle request buffer and operate ACDB
 * This acph works only in ARM9
 *
 *Copyright (c) 2011-2014 by Qualcomm Technologies, Inc.
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
 *when       who     what, where, why
 *--------   ---     ----------------------------------------------------------
 *08/03/11   ernanl  initial draft
 ********************************************************************************
 */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acph_online/inc/acph_online.h#2 $ */
/*
   --------------------
   |include files                |
   --------------------
   */
#include "acph.h"

/*
   --------------------
   | External functions |
   --------------------
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
int32_t acph_online_ioctl(uint16_t commandId,
                          uint8_t *req_buf_ptr,
						  uint32_t req_buf_len,
                          uint8_t *resp_buf_ptr,
						  uint32_t resp_buf_len,
                          uint32_t *resp_buf_bytes_filled
                          );

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
int32_t acph_online_init(void);
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
int32_t acph_online_deinit(void);

#endif //_ONLINE_INTF_H_
