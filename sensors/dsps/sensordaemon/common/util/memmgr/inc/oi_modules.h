#ifndef _OI_MODULES_H
#define _OI_MODULES_H
/**
 * @file  
 *
   This file provides the enumeration type defining the
   individual stack components.
 *
 */
/**********************************************************************************
  $AccuRev-Revision: 813/1 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

/*==============================================================================
  Edit History

  This section contains comments describing changes made to the module. Notice
  that changes are listed in reverse chronological order. Please use ISO format
  for dates.

  $Header: //source/qcom/qct/core/sensors/dsps/common/main/latest/util/memmgr/inc/oi_modules.h#1 $
  $DateTime: 2011/01/28 12:40:41 $

  when       who  what, where, why 
  ---------- ---  -----------------------------------------------------------
  2010-12-03 pg   Updated OI_MODULE enum with additional modules.

==============================================================================*/


/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This enumeration lists constants for referencing the components of 
 * the BLUEmagic 3.0 protocol stack, profiles, and other functionalities.
 *
 * In order to distinguish types of modules, items are grouped with markers to 
 * delineate start and end of the groups
 *
 * The module type is used for various purposes:
 *      identification in debug print statements
 *      access to initialization flags
 *      access to the configuration table
 */
 
typedef enum {
    /* profiles and protocols  --> Updates to oi_debug.c and oi_config_table.c */

                                /*  XX --> Keep Enum values up-to-date! */
    SNS_SAM,                    /*  00 SAM                              */
    SNS_SMGR,                   /*  01 SMGR                             */
    SNS_SMR,                    /*  02 SMR                              */
    SNS_PM,                     /*  03 POWER                            */
    SNS_TEST,                   /*  04 TEST                             */
    SNS_DDF,                    /*  05 Device Driver Framework          */
    SNS_EM,                     /*  06 Event (Timer) Manager            */
    SNS_MODULE7,                /*  07 TEMP_MODULE7                     */
    SNS_MODULE8,                /*  08 TEMP_MODULE8                     */
    SNS_MODULE9,                /*  09 TEMP_MODULE9                     */

    OI_MODULE_MEMMGR,           /*  10 MEMMGR                           */

    /* various pieces of code depend on these last 2 elements occuring in a specific order:
       OI_MODULE_ALL must be the 2nd to last element
       OI_MODULE_UNKNOWN must be the last element 
       */
    OI_MODULE_ALL,              /**< 11 special value identifying all modules - used for control of debug print statements */
    OI_MODULE_UNKNOWN           /**< 12 special value - used for debug print statements */
} OI_MODULE;

/**
 * This constant is the number of actual modules in the list.  ALL and UNKNOWN are 
 * special values that are not actually modules.
 * Used for debug print and memmgr profiling
 */
#define OI_NUM_MODULES  OI_MODULE_ALL    


/**
 * This constant is the number of profile and core components.  It is used to size
 * the initialization and configuration tables.
 */
#define OI_NUM_STACK_MODULES    OI_MODULE_BHAPI     


#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_MODULES_H */

