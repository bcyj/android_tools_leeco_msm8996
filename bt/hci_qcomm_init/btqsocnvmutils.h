#ifndef _BTQSOCNVMUTILS_H
#define _BTQSOCNVMUTILS_H

/**
  @file btqsocnvmutils.h
  
  Public declarations for the NVM utility functions.  
*/

/*--------------------------------------------------------------
  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Qualcomm Technologies Confidential and Proprietary
--------------------------------------------------------------*/

/*==============================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/inc/btqsocnvmutils.h#8 $
  $DateTime: 2011/06/14 23:36:13 $
  $Author: roystonr $

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2012-10-01  dv  Added support for wcn2243 2.1 SOC.
2011-07-18  bn  Added support for 8960 for sending NVM tags to RIVA.
2010-10-25   tw Added support for 4025 B3
2010-07-23   rr  Added support for Bahama A0 (Manihiki)
2010-02-19   rr  Added support for Marimba B1
2009-11-06  dgh  Resurected btqsocnvmutils.h, doxygenated it, and made it stand alone.
2009-06-30   sa  Added Support for BT QSOC Lookup functionality.
===============================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif 

/**
  @mainpage Bluetooth NVM Utilites
*/

/**
  @defgroup btqsocnvmutils Bluetooth NVM Utilites
  @{
*/

/*------------------------------------------------------------------------------
Classes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Type definitions
------------------------------------------------------------------------------*/

/** Structure containing parameters for bt_qsoc_type_look_up

  @see bt_qsoc_type_look_up
*/
typedef struct {
  /** Pointer to string returned from APP_VERSION_CMD. */
  char *app_ver_str;  

  /** Pointer to string returned from HW_REG_ID. */
  char *hw_ver_str;   
} bt_qsoc_lookup_param;

/*------------------------------------------------------------------------------
Enumerated types
------------------------------------------------------------------------------*/

/** Enumeration of all supported BT QSOC versions. 

  @note Do not removed SOC identifiers from this list!  If a SOC version is no
        longer supported simply append "_UNSUPPORTED" to the SOC identifier in
        the enumeration.  For example to deprecate support for BTS420 B0 change
        the identifier to BT_QSOC_4020BD_B0_UNSUPPORTED.  This is to maintain 
        binary compatibility across versions.

  @see bt_qsoc_nvm_open
  @see bt_qsoc_type_look_up
*/
typedef enum
{
  BT_QSOC_R2B,
  BT_QSOC_R2C,
  BT_QSOC_R3,
  BT_QSOC_R3BD,
  BT_QSOC_4020BD_B0,
  BT_QSOC_4020BD_B1,
  BT_QSOC_R4,
  BT_QSOC_4021_B1,
  BT_QSOC_4025_A0,
  BT_QSOC_4025_B0,
  BT_QSOC_4025_B1,
  BT_QSOC_4025_B2,
  BT_QSOC_MBA_A0,
  BT_QSOC_MBA_B0,
  BT_QSOC_MBA_B1,
  BT_QSOC_BHA_A0,
  BT_QSOC_4025_B3,
  BT_QSOC_BHA_B0,
  BT_QSOC_BHA_B1,
  BT_QSOC_RIVA,
  BT_QSOC_UNKNOWN,
  BT_QSOC_MAX,
  BT_QSOC_NONE = 0xFF
} bt_qsoc_enum_type;

/*------------------------------------------------------------------------------
Structure definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Union definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Macros
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Constant values
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Function declarations
------------------------------------------------------------------------------*/

/*==============================================================
FUNCTION:  bt_qsoc_type_look_up
==============================================================*/
/**
  This function returns the BT SOC type code based on hardware version strings.

  @see bt_qsoc_lookup_param
  
  @return  Returns the SOC type code.           
           
  @sideeffects Undetermined.
*/
bt_qsoc_enum_type bt_qsoc_type_look_up 
(
  /** [in] Pointer to a param strucuture containing version strings */
  const bt_qsoc_lookup_param *lookup_param
);

/*------------------------------------------------------------------------------
Globals
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Notes
------------------------------------------------------------------------------*/
 
/** @} */ /* end of btqsocnvmutils group */

#ifdef __cplusplus
};
#endif 

#endif /* #ifndef _BTQSOCNVMUTILS_H */
