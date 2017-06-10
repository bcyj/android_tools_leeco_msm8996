#ifndef __UAL_UTIL_FRAME_FILE_H__
#define __UAL_UTIL_FRAME_FILE_H__

/*============================================================================
                           ual_util_frame_file.h

DESCRIPTION:  Types and function definitions for the common use
              of all usf daemons.
              Contains frame file utilities.

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <linux/types.h>
#include "ual_util.h"
/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
  Function declarations
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  ual_util_frame_file_add_wave_header
==============================================================================*/
/**
 * This function would write the suitable wave header to the given file.
 *
 * @param paramsStruct reference to the common daemon parameters
 * @param frameFileName the name of the frame file
 *
 * @return int 0 - success
 *             -1 - failure
 */
int ual_util_frame_file_add_wave_hdr(us_all_info const *paramsStruct,
                                     FILE *frameFileName);

/*==============================================================================
  FUNCTION:  ual_util_frame_file_write
==============================================================================*/
/**
 * This function would write the given data to the given framefile.
 *
 * @param data_buf the data to write
 * @param element_size the size of each element to write
 * @param bytesFromRegion the number of bytes in the given data
 * @param paramsStruct a struct containing all the usefull daemon data
 * @param frameFile the frame file to write the data to.
 *
 * @return int 0 - success
 *             -1 - failure
 */
int ual_util_frame_file_write(uint8_t *data_buf,
                              size_t element_size,
                              uint32_t bytesFromRegion,
                              us_all_info const *paramsStruct,
                              FILE *frameFile);

#endif // __UAL_UTIL_FRAME_FILE_H__
