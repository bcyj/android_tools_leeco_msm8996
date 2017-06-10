/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

    B L U E T O O T H   B T S   Q S O C  N V M   U T I Ls   

GENERAL DESCRIPTION
  This btqsoc utility file provides functionality like 
  1. BT QSOC lookup 

Copyright (c) 2009-2012 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order. Please
use ISO format for dates.

$Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/src/btqsocnvmutils.c#11 $
$DateTime: 2011/06/14 23:36:13 $
$Author: roystonr $

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2012-10-01  dv  Added support for wcn2243 2.1 SOC.
2011-07-18  bn  Added support for 8960 for sending NVM tags to RIVA.
2010-10-25  tw  Added support for 4025 B3
2010-07-23  rr  Added support for Bahama A0 (Manihiki)
2010-02-19  rr  Added support for Marimba B1
2010-01-08  dgh  Passing a NULL app_ver_str or hw_ver_str pointer to 
                 bt_qsoc_type_look_up() will now return BT_QSOC_UNKNOWN instead
                 of causing an access violation exception.
2009-10-07   sp  Added support for Marimba B0
2009-10-05  dgh  Removed reference to comdef.h.
2009-07-09   sa  Adding the Header, DateTime, Author information.
2009-06-30   sa  Added Support for BT QSOC Lookup functionality. 

===============================================================================*/

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/
#include <string.h>

/********************* PRIVATE INCLUDE FILES********************************/
#include "btqsocnvmprivate.h"

/*=========================================================================*/
/*                               TYPEDEFS                                  */
/*=========================================================================*/

/**
  @defgroup btqsocvmutils_ver_strings BTS Version Identification Strings
  @{
*/
#define BT_QSOC_R2B_STR_ID                 "Release 3.03.0"
#define BT_QSOC_R2C_STR_ID                 "Release 3.03.1"
#define BT_QSOC_R3_STR_ID                  "Release 3.03.2"
#define BT_QSOC_R3BD_STR_ID                "Release 3.03.3"
#define BT_QSOC_R4_STR_ID                  "Release 3.03.4"
/* BD B0 and B1 Id are the same */  
#define BT_QSOC_4020BD_B0_or_B1_STR_ID     "Release 3.03.5"
#define BT_QSOC_4021_B1_STR_ID             "Release 4.03.0"
#define BT_QSOC_4025_A0_STR_ID             "Release 4.00.0"
#define BT_QSOC_4025_B0_STR_ID             "Release 4.00.1"
#define BT_QSOC_4025_B1_STR_ID             "Release 4.00.2"
#define BT_QSOC_4025_B2_STR_ID             "Release 4.00.3"
#define BT_QSOC_4025_B3_STR_ID             "Release 4.00.4"
#define BT_QSOC_MBA_A0_STR_ID              "Release 4.05.0"
#define BT_QSOC_MBA_B0_STR_ID              "Release 4.05.1"
#define BT_QSOC_MBA_B1_STR_ID              "Release 4.05.2"
#define BT_QSOC_BHA_A0_STR_ID              "Release 5.00.0"
#define BT_QSOC_BHA_B0_STR_ID              "Release 5.00.1"
#define BT_QSOC_BHA_B1_STR_ID              "Release 5.00.2"
#define BT_QSOC_RIVA_STR_ID                "Release 5.01.0"

/** @} */ /* end of btqsocvmutils_ver_strings group */

/*===========================================================================
                                 Globals
===========================================================================*/
const uint8 bt_qsoc_hw_ver_reg_4020_BD_B0 [] = { 0x05, 0x00, 0x00, 0x00 };


/*===========================================================================
                         FUNCTION DEFINITIONS
===========================================================================*/

/*==========================================================================

  FUNCTION       BT_QSOC_TYPE_LOOK_UP

 
  DESCRIPTION    Provides BTS SOC Type based on the response to app 
                 version and hw register command.

  DEPENDENCIES   None.

  PARAMETERS     Take bt_qsoc_nvm_lookup_param as input.

  RETURN VALUE   TRUE (NVM init success) or FALSE (NVM init failure).


==========================================================================*/

bt_qsoc_enum_type bt_qsoc_type_look_up 
(
const bt_qsoc_lookup_param *lookup_param
)
{
  bt_qsoc_enum_type bt_qsoc_type = BT_QSOC_NONE;
  const char *bt_qsoc_fw_ver;
  const char *hw_ver_reg_id;

  if(lookup_param != NULL)
  {
    bt_qsoc_fw_ver = lookup_param->app_ver_str;
    hw_ver_reg_id  = lookup_param->hw_ver_str;
    
    if( NULL == bt_qsoc_fw_ver )
    {
      bt_qsoc_type = BT_QSOC_UNKNOWN;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_4025_B1_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_4025_B1;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_4025_B2_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_4025_B2;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_4025_B3_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_4025_B3;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_MBA_A0_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_MBA_A0;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_MBA_B0_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_MBA_B0;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_MBA_B1_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_MBA_B1;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_BHA_A0_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_BHA_A0;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_BHA_B0_STR_ID) != NULL)
    {
      bt_qsoc_type = BT_QSOC_BHA_B0;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_BHA_B1_STR_ID) != NULL)
    {
      bt_qsoc_type = BT_QSOC_BHA_B1;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_R3_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_R3;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, 
                     BT_QSOC_4020BD_B0_or_B1_STR_ID) != NULL)
    {
      if( NULL == hw_ver_reg_id )
      { /* if the HW_VER data is NULL, default to 4020 BD B0 */
        bt_qsoc_type = BT_QSOC_4020BD_B0; 
      }
      else if(memcmp((const void *)hw_ver_reg_id,
                (const void *)bt_qsoc_hw_ver_reg_4020_BD_B0, 
                0x04) == 0)
      {     
        bt_qsoc_type = BT_QSOC_4020BD_B0;
      }
      else
      {
        bt_qsoc_type = BT_QSOC_4020BD_B1;
      }     
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_4021_B1_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_4021_B1;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_R2B_STR_ID) != NULL)
    {
      bt_qsoc_type = BT_QSOC_R2B;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_R2C_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_R2C;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_R3BD_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_R3BD;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_R4_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_R4;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_4025_A0_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_4025_A0;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_4025_B0_STR_ID) != NULL) 
    {
      bt_qsoc_type = BT_QSOC_4025_B0;
    }
    else if (strstr( (const char *)bt_qsoc_fw_ver, BT_QSOC_RIVA_STR_ID) != NULL)
    {
      bt_qsoc_type = BT_QSOC_RIVA;
    }
    else
    {
      bt_qsoc_type = BT_QSOC_UNKNOWN;
    }

  }
 
  return bt_qsoc_type;

}
