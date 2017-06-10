#ifndef PVR_H
#define PVR_H

/** @file pvr.h
 * @brief
 * This file contains the definitions of the constants, data structures
 * and interfaces to the PVR Module, the PVR module support local encryption
 * and decryption of secure content.
 */

/*===========================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

					EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/24/14   ng      Separate in/out buff flags.
07/11/13   yb      Initial Version.

===========================================================================*/

/*----------------------------------------------------------------------------
 *Type Declarations
 *--------------------------------------------------------------------------*/
#include <ctype.h>
#include "comdef.h"
#include <stdio.h>
#include <stdlib.h>

// Input flags
#define PVR_NS_INP_DBG_FLAG_SHIFT 0
#define PVR_NS_OUTP_DBG_FLAG_SHIFT 1
#define PVR_NS_INP_DBG_FLAG_ENABLE (1<<PVR_NS_INP_DBG_FLAG_SHIFT)
#define PVR_NS_OUTP_DBG_FLAG_ENABLE (1<<PVR_NS_OUTP_DBG_FLAG_SHIFT)

/*-------------------------------------------------------------------------*/

typedef enum {
   PVR_SUCCESS = 0,
   PVR_ERR_GENERAL_FAILURE = -1,
   PVR_ERR_INVALID_INPUT_PARAMS = -2,
   PVR_ERR_SECURITY_FAULT = -3,
   PVR_ERR_SIZE = 0x7FFFFFFF
} pvr_status_t;

/*----------------------------------------------------------------------------
 *Function Declarations and Documentation
 *--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/**
  @brief Initialize the PVR Module.
  - The function loads the PVR module into TrustZone and initializes it.

  @return
  PVR_SUCCESS   - Success.
  PVR_ERR_GENERAL_FAILURE    - Failure.
  @dependencies
  Must be called prior to invoking any of the other PVR APIs.

  @sideeffects
  None.
*/
pvr_status_t PVR_initialize(void);
/**
  @brief Terminate the PVR Module
  - The function terminates the operation of the PVR module in TrustZone.

  @return
  PVR_SUCCESS   - Success.
  PVR_ERR_GENERAL_FAILURE    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
pvr_status_t PVR_terminate(void);
/**
  @brief Allocate PVR id.

  @param[out] pvr_id     - PVR id

  @return
  PVR_SUCCESS   - Success.
  PVR_ERR_GENERAL_FAILURE    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/

pvr_status_t PVR_allocate_id(uint32* pvr_id );
/**
  @brief encrypt the input buffer content and
  place the encrypt content in the output buffer.

  @param[in] pvr_id	-	PVR id
  @param[in] output_buf	-	ION output buffer for the encrypt content
  @param[in] out_buff_len	- output buffer length - should be equal to the input buffer length.
  @param[in] inp_buf	-	ION input buffer with content we want to encrypt
  @param[in] inp_buff_len	-	input buffer length.
  @return
  PVR_SUCCESS   - Success.
  PVR_ERR_INVALID_INPUT_PARAMS - invalid buffer lengths.
  PVR_ERR_GENERAL_FAILURE- other failures.

  @dependencies
  None.

  @sideeffects
  None.
*/
pvr_status_t PVR_local_encryption(
		uint32 pvr_id,
		int32 output_ifd_data_fd,
		uint32 out_buff_len,
		int32 input_ifd_data_fd,
		uint32 inp_buff_len);
/**
  @brief decrypt the input buffer content and
  place the decrypt content in the output buffer.

  @param[in] pvr_id     - PVR id
  @param[in] output_buf -  secure ION output buffer for the decrypt content
  @param[in] out_buff_len	- output buffer length - should be bigger/equal to the input buffer length.
  @param[in] inp_buf -  ION input buffer with encrypt content
  @param[in] inp_buff_len - input buffer length.
  @param[in] flags - input flags, needs to be set to 0.

  @return
  PVR_SUCCESS   - Success.
  PVR_ERR_INVALID_INPUT_PARAMS -  invalid buffer lengths.
  PVR_ERR_SECURITY_FAULT   - non secure output buffer.
  PVR_ERR_GENERAL_FAILURE  - other failures.

  @dependencies
  None.

  @sideeffects
  None.
*/
pvr_status_t PVR_local_decryption(
		uint32 pvr_id,
		int32 output_ifd_data_fd,
		uint32 out_buff_len,
		int32 input_ifd_data_fd,
		uint32 inp_buff_len,
		uint8 u8Flags);
#ifdef __cplusplus
}
#endif

#endif /*PVR_H */
