#ifndef _BTQSOCNVM_PRIVATE_H_
#define _BTQSOCNVM_PRIVATE_H_

/**
  @file btqsocnvmprivate.h
  
  Private declarations for the NVM system.  
*/

/*--------------------------------------------------------------
  Copyright (c) 2008-2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Qualcomm Technologies Confidential and Proprietary
--------------------------------------------------------------*/

/*==============================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/src/btqsocnvmprivate.h#14 $
  $DateTime: 2011/01/21 17:41:16 $
  $Author: tylerw $

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2012-09-14   dv  Added support for wcn2243 2.1
2011-03-01   rr  Deprecating unused formal paramters to parser routines.
2010-10-25   tjw Added support for 4025 B3
2010-07-23   rr  Added support for Bahama A0 (Manihiki)
2010-07-07   rr  Guarded customer.h under UNDER_CE to exclude its exclusion for WM7 compile.
2010-03-03  ag  Bringing up Logger Redirection Feature on Marimba. Change HC_NUM_RX_BUF and HC_NUM_TX_BUF
2010-02-19   rr  Added support for Marimba B1
2009-11-05  dgh  Removed VS opcode defines that are not used in SOCCFG.
2009-10-07   sp  Added support for Marimba B0
2009-09-02  dgh  Merged contents from btqsocnvmBTS.h
2009-07-28  dgh  Removed unused globals.  Doxygenated function declarations.
2009-07-01   sa  Redesign NVM System.
2009-05-07   sa  Support for EFS NVM Mode.
2008-11-14   sa  Support for NV automation.
2008-03-04   sa  Initial version
===============================================================================*/

#ifndef UNDER_CE
#include "customer.h" /* pull in featurization macros */
#endif

#include "btqsocnvm.h"

#ifdef __cplusplus
extern "C"
{
#endif 

/**
  @defgroup btqsocvmprivate Bluetooth NVM System Private Header File
  @{
*/

/*--------------------------------------------------------------
Classes
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Type definitions
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Enumerated types
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Structure definitions
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Union definitions
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Macros
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Constant values
--------------------------------------------------------------*/

/* Defined in AMSS's soc/btsoc.h for SOC targets */
#define BT_SOC_MAX_RX_PKT_SIZE       0x0208  /* 520d */

/* BT_MTP_MAX_HC_LEN is also defined in AMSS's bti.h for SOC targets.
 * This value is computed to be the max payload size of a 3-DH5 packet.
 * Any changes to this number may have drastic effects on throughput,
 * and should be considered carefully. */
#define BT_MTP_MAX_HC_LEN            1021

/*=========================================================================*/
/*         Macros for BTS Buffer numbers and sizes                         */
/*=========================================================================*/

/* Max HCI buffer length for BTS. Must be 8 bytes larger than what */
#define BT_QSOC_MAX_HC_TX_LEN        (BT_MTP_MAX_HC_LEN + 0x8)

#define BT_QSOC_MAX_HC_TX_LEN_LSB    (BT_QSOC_MAX_HC_TX_LEN & 0xFF) /* LSB */
/* MSB */
#define BT_QSOC_MAX_HC_TX_LEN_MSB    (uint8)((uint16)BT_QSOC_MAX_HC_TX_LEN >> 0x8)

#define BT_QSOC_MAX_HC_RX_LEN        BT_SOC_MAX_RX_PKT_SIZE
#define BT_QSOC_MAX_HC_RX_LEN_LSB    (BT_QSOC_MAX_HC_RX_LEN & 0xFF) /* LSB */
/* MSB */
#define BT_QSOC_MAX_HC_RX_LEN_MSB    (uint8)((uint16)BT_QSOC_MAX_HC_RX_LEN >> 0x8)

#define BT_QSOC_MAX_HC_NUM_RX_BUF    0x0B /* 11d */
#define BT_QSOC_MAX_HC_NUM_TX_BUF    0x05 /* 5d */
#define BT_QSOC_SCO_BUF_SIZE         0x5A /* 90d */

/* Rx/Tx Buf Numbers for SOC_REDIRECT_LOGGING */
#define BT_QSOC_MAX_HC_NUM_RX_BUF_SOC_REDIRECT_LOGGING 0x03 /* 3d */
#define BT_QSOC_MAX_HC_NUM_TX_BUF_SOC_REDIRECT_LOGGING 0x03 /* 3d */
/** The "all" set of SOC types.

  This set contains the SOC types that are enabled by default on
  most targets (because most targets define FEATURE_BT_QSOC_ALL).
*/
#ifdef FEATURE_BT_QSOC_ALL
  #define FEATURE_BT_QSOC_BTS4020_R3
  #define FEATURE_BT_QSOC_BTS4021_B1
  #define FEATURE_BT_QSOC_BTS4020_BDB0
  #define FEATURE_BT_QSOC_BTS4020_BDB1
  #define FEATURE_BT_QSOC_BTS4025_B0
  #define FEATURE_BT_QSOC_BTS4025_B1
  #define FEATURE_BT_QSOC_BTS4025_B2
  #define FEATURE_BT_QSOC_BTS4025_B3
  #define FEATURE_BT_QSOC_MARIMBA_A0
  #define FEATURE_BT_QSOC_MARIMBA_B0
  #define FEATURE_BT_QSOC_MARIMBA_B1
  #define FEATURE_BT_QSOC_BAHAMA_A0
  #define FEATURE_BT_QSOC_BAHAMA_B0
  #define FEATURE_BT_QSOC_BAHAMA_B1
#endif /* FEATURE_BT_QSOC_ALL */

/*--------------------------------------------------------------
Global declarations
--------------------------------------------------------------*/

/**
  @defgroup btqsocvmprivate_string_externs Bluetooth NVM System SOC Strings
  @ingroup  btqsocvmprivate
  @{
*/

/** BTS4020 R3 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4020_R3_19P2Mhz;

/** BTS4020 R3 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4020_R3_32Mhz;

/** BTS4020 BD B0 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4020_BDB0_19P2Mhz;

/** BTS4020 BD B0 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4020_BDB0_32Mhz;

/** BTS4020 BD B1 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4020_BDB1_19P2Mhz;

/** BTS4020 BD B1 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4020_BDB1_32Mhz;

/** BTS4021 B1 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4021_B1_19P2Mhz;

/** BTS4021 B1 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4021_B1_32Mhz;

/** BTS4025 B0 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B0_19P2Mhz;

/** BTS4025 B0 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B0_32Mhz;

/** BTS4025 B1 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B1_19P2Mhz;

/** BTS4025 B1 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B1_32Mhz;

/** BTS4025 B2 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B2_19P2Mhz;

/** BTS4025 B2 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B2_32Mhz;

/** BTS4025 B3 19.2 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B3_19P2Mhz;

/** BTS4025 B3 32 MHz NVM data string */
extern const void * bt_qsoc_nvm_BTS4025_B3_32Mhz;

/** Marimba A0 NVM data string */
extern const void * bt_qsoc_nvm_MARIMBA_A0;

/** Marimba B0 NVM data string */
extern const void * bt_qsoc_nvm_MARIMBA_B0;

/** Marimba B1 NVM data string */
extern const void * bt_qsoc_nvm_MARIMBA_B1;

/** Bahama A0 NVM data string */
extern const void * bt_qsoc_nvm_BAHAMA_A0;

/** Bahama B0 NVM data string */
extern const void * bt_qsoc_nvm_BAHAMA_B0;

/** Bahama B1 NVM data string */
extern const void * bt_qsoc_nvm_BAHAMA_B1;


/** @} */ /* end of btqsocvmprivate_string_externs group */

/*--------------------------------------------------------------
Function declarations
--------------------------------------------------------------*/

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_fs_mode
==============================================================*/
/**
  Initialize the parser for reading a NVM file.

  @see bt_qsoc_enum_type
  @see bt_qsoc_config_params_struct_type
  
  @return  boolean: True: If initialized successfully.
                    False: otherwise.

  @sideeffects Undetermined.
*/
extern boolean bt_qsoc_nvm_init_fs_mode(void);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_fixed_n_runtime_tbl
==============================================================*/
/**
  Initialize the fixed and runtime data for a given SOC type.

  @see bt_qsoc_enum_type
  @see bt_qsoc_config_params_struct_type
  
  @return  boolean: True: If initialized successfully.
                    False: otherwise.

  @sideeffects Undetermined.
*/
boolean bt_qsoc_nvm_init_fixed_n_runtime_tbl 
(
  /** [in] SOC version */
  bt_qsoc_enum_type bt_qsoc_type, 
  
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_auto_mode
==============================================================*/
/**
  Initialize the parser for reading an auto-mode NVM string.

  @see bt_qsoc_enum_type
  @see bt_qsoc_config_params_struct_type
  
  @return  boolean: True: If initialized successfully.
                    False: otherwise.

  @sideeffects Undetermined.
*/
boolean bt_qsoc_nvm_init_auto_mode 
(
  /** [in] SOC version */
  bt_qsoc_enum_type bt_qsoc_type, 
  
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_hybrid_mode
==============================================================*/
/**
  Initialize the NVM tag list for hybrid mode.

  Since the NVM tag list ignores duplicates the hybrid mode list
  must be built in a particular order to preserve tag priority.
  The order is fixed/runtime tags, EFS tags, and auto-mode tags.
  This order ensures that the fixed and runtime tags have the 
  highest priority.  It also ensures that a tag in the EFS file
  will take precedence over a tag in the auto-mode string.
  
  @see bt_qsoc_enum_type
  @see bt_qsoc_config_params_struct_type
  
  @return  boolean: True: If initialized successfully.
                    False: otherwise.

  @sideeffects Undetermined.
*/
boolean bt_qsoc_nvm_init_hybrid_mode 
(
  /** [in] SOC version */
  bt_qsoc_enum_type bt_qsoc_type, 
  
  /** [in] Parameters that determine the run time configuration */
  bt_qsoc_config_params_struct_type *param_ptr
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_close_nvm
==============================================================*/
/**
  Reset the NVM parser state.

  In Auto Mode this function resets the parse state information.
  In EFS Mode this function closes the file handle to nvm. 
  In Hybrid mode this function does both of the above operations.
  
  @see bt_qsoc_nvm_init_auto_mode
  @see bt_qsoc_enum_nvm_mode
  
  @return  boolean: True: If closed successfully.
                    False: otherwise.

  @sideeffects Undetermined.
*/
boolean bt_qsoc_nvm_close_nvm 
(
  /** [in] BT QSOC NVM Mode */
  bt_qsoc_enum_nvm_mode mode
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_get_next_nvm_tag
==============================================================*/
/**
  This function returns the next tag which needs to be sent to SoC.

  @see bt_qsoc_nvm_init_auto_mode

  @return  boolean: True:  If tag extracted
                    False: If no more tags in nvm file

  @sideeffects Undetermined.
*/
boolean bt_qsoc_nvm_get_next_nvm_tag 
(
  /** [out] pointer to storage for the returned tag */
  uint8** soc_nvm_ptr
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_set_tag_sent
==============================================================*/
/**
  This function marks the given tag number as sent to SOC.

  @return  none

  @sideeffects Undetermined.
*/
void bt_qsoc_nvm_set_tag_sent
(
  /** [in] tag number to mark */
  uint8 tag_no
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_init_parser
==============================================================*/
/**
  This function initializes the internal parser structures.

  This function detects hybrid mode and doesn't clober previously
  parsed NVM data.

  @return  boolean: TRUE:  If the parser initialized sucessfully.
                    FALSE: If the parser failed to initialize.

  @sideeffects Undetermined.
*/
boolean bt_qsoc_nvm_init_parser (void);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_parse_nvm_file
==============================================================*/
/**
  This function parses NVM file data into binary data.

  This function detects hybrid mode and doesn't clober previously
  parsed NVM data.

  @see bt_qsoc_nvm_init_auto_mode
    @see bt_qsoc_nvm_init_fs_mode
    @see bt_qsoc_nvm_init_hybrid_mode

  @return  boolean: TRUE:  If parsing finished sucessfully.
                    FALSE: If parsing failed.

  @sideeffects Undetermined.
*/
boolean bt_qsoc_nvm_parse_nvm_file 
(
  /** [in] pointer to NVM file data */
  const char* nvm_data_ptr
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_add_element
==============================================================*/
/**
  This function adds a NVM element to the end of the list.  If
  an element already exists for this tag number it is overwritten
  instead.

  @return none.
*/
void bt_qsoc_nvm_add_element
(
  /** NVM data to add to the list */
  uint8 *nvm_string,

  /** True if string was dynamically allocated */
  boolean is_on_heap
);

/*------------------------------------------------------------------------------
Notes
------------------------------------------------------------------------------*/
 
/** @} */ /* end of btqsocvmprivate group */

#ifdef __cplusplus
};
#endif 

#endif /* #ifndef _BTQSOCNVM_PRIVATE_H_ */


