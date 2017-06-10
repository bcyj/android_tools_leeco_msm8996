#ifndef _BCSTSRV_DEMOD_FW_LOADER_
#define _BCSTSRV_DEMOD_FW_LOADER_

/** @file bcstsrv_demod_fw_loader.h
 * @brief
 * This file contains the API header for the Demod FW Loader feature
 */

/*===========================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

					EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/1/13   gs      Initial Version.

===========================================================================*/

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/msm_ion.h>
#include <utils/Log.h>
#include <sys/mman.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

#define DEMOD_LOADER_MAX_NUM_OF_IMAGES 15

/*----------------------------------------------------------------------------
 *Type Declarations
 *--------------------------------------------------------------------------*/

/** FW Table Header Standard Descriptor Type **/

typedef struct
{
	uint32_t id;
	uint32_t size;
	uint32_t offset;
	uint8_t description[12];
} FWStandardDescriptorType;

typedef struct
{
	uint32_t total_size;
	uint32_t useful_size;
	uint32_t num_of_images;
	FWStandardDescriptorType desc[DEMOD_LOADER_MAX_NUM_OF_IMAGES];
} FWBundleHeaderType;

/** Result Types **/

typedef enum {
	DEMOD_LOADER_SUCCESS = 0,
	DEMOD_LOADER_BAD_PARAMETERS = -1,
	DEMOD_LOADER_SHORT_BUFFER = -2,
	DEMOD_LOADER_MEMORY_ERROR = -3,
	DEMOD_LOADER_QSEECOM_ERROR = -4,
	DEMOD_LOADER_TZ_ERROR = -5,
	DEMOD_LOADER_NO_INIT = -6
} DEMOD_LOADER_RESULT;

/*----------------------------------------------------------------------------
 *Function Declarations and Documentation
 *--------------------------------------------------------------------------*/

/**
  @Brief Initialize Demod FW loader data for API functionality

  @param[in] phys_start  - physical address of memory area
  @param[in] total_size  - size of memory area
  @param[in] useful_size - size of relevant memory region containing the images

  @return
  DEMOD_LOADER_SUCCESS - Success
  Other value       - Failure

  @dependencies
  None.

  @sideeffects
  None.

**/

DEMOD_LOADER_RESULT fw_loader_init();

/**
  @Brief Load requested FW image within the DDR to "dynamic" memory area
	 Function writes a pre-defined sequence to 3 registers
  @param[in] fw_index  - index of rquested FW image

  @return
  DEMOD_LOADER_SUCCESS - Success
  Other value       - Failure

  @dependencies
  None.

  @sideeffects
  None.

**/

DEMOD_LOADER_RESULT load_fw_image(uint32_t fw_index);

/**
  @Brief Copies FW table header from area loaded by PIL into the shared buffer


  @param[in] buf       - address of shared buffer
  @param[in] buf_size  - shared buffer size

  @return
  DEMOD_LOADER_SUCCESS - Success
  Other value       - Failure

  @dependencies
  None.

  @sideeffects
  None.

**/

DEMOD_LOADER_RESULT get_fw_table_header(unsigned char* buf, unsigned int buf_size);

/**
  @Brief Copies content of 2 specific status registers

  @param[in] uccp_stat_reg1 - preallocated integer address to contain UCCP_META_TXENABLE register value
  @param[in] uccp_stat_reg2 - preallocated integer address to contain UCCP_META_TXSTATUS register value

  @return
  DEMOD_LOADER_SUCCESS - Success
  Other value       - Failure

  @dependencies
  None.

  @sideeffects
  None.

**/

DEMOD_LOADER_RESULT get_status_register_values(uint32_t* uccp_stat_reg1, uint32_t* uccp_stat_reg2);

/**
  @Brief Uninitialize Demod FW loader data

  @return
  DEMOD_LOADER_SUCCESS - Success
  Other value       - Failure

  @dependencies
  None.

  @sideeffects
  None.

**/

DEMOD_LOADER_RESULT fw_loader_uninit();

#endif
