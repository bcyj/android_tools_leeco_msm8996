#ifndef __USF_VALIDATION_H__
#define __USF_VALIDATION_H__

/*============================================================================
                           usf_validation.h

DESCRIPTION:  Types and function definitions for the common use
              of usf_validation.cpp.
              This file contains the main function that validates
              the config file.

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
  FUNCTION:  parsing_validation
==============================================================================*/
/**
  This function validates that the parameterStruct is correct.
  @paramStruct struct containing all the cfg parameters
  @name the name of the daemon running the function
  @return 0  In case of success
          -1 In case of bad parameters
          1  In case of a failure in validation
*/
int usf_validation_cfg_file(char *cfgFileName,
                            char *daemonName,
                            us_all_info *paramStruct,
                            FormStruct *formStruct);

/*==============================================================================
  FUNCTION:  usf_parse_transparent_data()
==============================================================================*/
/**
 * This function parses the transparent data and returns the frame, group
 * in the received parameters.
 *
 * @param transparent_data_size The side of the transparent data
 * @param transparent_data The transparent data
 * @param frame An output parameter that shall contain the frame value
 * @param group An output parameter that shall contain the group value
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
int usf_parse_transparent_data(int transparent_data_size,
                               char const *transparent_data,
                               uint16_t *frame,
                               uint16_t *group);
/*==============================================================================
  FUNCTION:  usf_parse_transparent_data()
==============================================================================*/
/**
 * This function parses the transparent data and returns the frame, group and skip
 * in the received parameters.
 *
 * @param transparent_data_size The side of the transparent data
 * @param transparent_data The transparent data
 * @param frame An output parameter that shall contain the frame value
 * @param group An output parameter that shall contain the group value
 * @param skip An output parameter that shall contain the skip value
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
int usf_parse_transparent_data(int transparent_data_size,
                               char const *transparent_data,
                               uint32_t *frame,
                               uint16_t *group,
                               uint16_t *skip);
#endif // __USF_VALIDATION_H__

