#ifndef _BTQSOCNVM_H_
#define _BTQSOCNVM_H_

/**
  @file btqsocnvm.h
  
  Public declarations for the NVM system.  
*/

/*--------------------------------------------------------------
  Copyright (c) 2007-2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Qualcomm Technologies Confidential and Proprietary
--------------------------------------------------------------*/

/*==============================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/wconnect/bthost/soccfg/main/latest/inc/btqsocnvm.h#11 $
  $DateTime: 2011/06/28 08:37:56 $
  $Author: rkorukon $

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2012-02-29  rr   Added/Modified LE & BR/EDR related config structure element
2011-06-22  rk   Added poke command for BAHAMA B0
2010-09-30  gbm  changed default WLAN device type to Libra
2010-05-31  akg  InBand Sleep Disabled in FTM Mode. This causes FTM mode fail on SCMM BMP
2010-03-26   rr  Fixed complier warning because of 32-bit max enum value for bt_qsoc_enum_wlan_type.
2010-02-05  dgh  Updated some documentation.
2010-01-28  dgh  Added new power class to support custom power settings on Hagrid
                 daugthercard.
2010-01-27  dgh  Added runtime configuration for selecting WLAN coexistence tags.
2009-12-09   rd  Comment changes only, elaborate on API desriptions
2009-11-23  sbk  Changed aeestddef.h to AEEStdDef.h to work with linux build
                 environment
2009-11-06  dgh  Added the bt_qsoc_nvm_tot_hc_tx_len() function.  Made tag #71
                 a runtime tag.  Moved bt_qsoc_type_look_up(), bt_qsoc_lookup_param 
                 and bt_qsoc_enum_type back into btqsocnvmutils.h.
2009-10-07   sp  Added support for Marimba B0
2009-09-02  dgh  Doxygenated function declarations.
                 Added hybrid mode enum.
2009-07-09   sa  Adding the Header, DateTime, Author information.
2009-07-01   sa  Redesign NVM System.
2009-06-24   ks  Define VS DataLog event Sub Command.
2009-05-14   rb  Added support for Marimba bluetooth hardware core
2009-05-07   sa  Support for EFS NVM Mode.
2009-03-14   sa  Added Support for CLASS 2 device.
2008-11-14   sa  Support for NV automation.
2008-10-27   sa  Added functionality to read SOC hw version register
                 (Currently this is used to differentiate BT 4020 BD B1 from 
                 BT 4020 DB B0, the cmd BT_QSOC_VS_EDL_GETAPPVER_REQ 
                 response alone cannot distinguish BT 4020 BD B1 from B1).
2008-10-27   sa  Added support to read BT 2.1 support from a nv item in 
                 NV browser at run-time.
2008-08-30   rb  Added preliminay support for 4020 BD B1.
2008-08-20   rb  Added support to 4025 B1
2008-07-28   sa  Added support to read Clk type,Clk sharing,in-band sleep mode, 
                 soc logging enabled, BT Wlan co-ext, BTS power mgmt mode  
                 from NV browser at run-time. 
2008-05-27   rb  Fix compiler warnings
2008-05-15   rb  Support for 4025 B0 Beta
2008-05-06   rb  Added support for 4025 A0
2008-03-13   rb  Added support for 4021 B1 and 4020 BD B0. 
2008-01-31   bretth Branched and renamed file from WM
2008-01-31   ruih   Merged in bretth's latest update
2007-12-21   damu   Initial version
===============================================================================*/

#include "AEEStdDef.h"
#include "btqsocnvmutils.h"  /* For bt_qsoc_enum_type */
 
#ifdef __cplusplus
extern "C"
{
#endif 

/**
  @mainpage Bluetooth NVM System
*/

/**
  @defgroup btqsocnvm Bluetooth NVM System
  @{
*/

/*--------------------------------------------------------------
Classes
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Type definitions
--------------------------------------------------------------*/

/** Basic type for a constant NVM element */
typedef const uint8 bt_qsoc_cfg_cst_element_type;

/** Basic type for an NVM element */
typedef uint8 bt_qsoc_cfg_element_type;

/*--------------------------------------------------------------
Enumerated types
--------------------------------------------------------------*/

/**
  NVM System Error Codes
*/
typedef enum
{
  BT_QSOC_NVM_STATUS_SUCCESS                = 0,  /**< Function Call Succeeded */
  BT_QSOC_NVM_STATUS_OK                     = 0,  /**< alias for 
                                                   BT_QSOC_NVM_STATUS_SUCCESS */
  BT_QSOC_NVM_STATUS_COMPLETE               = 1,  /**< Job/task is complete */
  BT_QSOC_NVM_STATUS_INVALID_PARAMETERS     = 101,/**< Invalid Func Parameter */
  BT_QSOC_NVM_STATUS_NOT_IMPLEMENTED        = 102,/**< Not Implemented */
  BT_QSOC_NVM_STATUS_DATA_NOT_INITIALIZED   = 103,/**< Data not initialized */
  BT_QSOC_NVM_STATUS_INIT_FAIILED           = 104,/**< Initialization failed */ 
  BT_QSOC_NVM_STATUS_FAIL                   = 105,/**< Generic Failure */ 
  BT_QSOC_NVM_STATUS_ALREADY_INITIALIZED    = 106,/**< Already Initialized */
  BT_QSOC_NVM_SYS_STATUS_PARSE_ERROR        = 106,/**< NVM Parse Error */
  BT_QSOC_NVM_SYS_STATUS_FILE_OPEN_ERROR    = 107,/**< NVM File Open Error */
  BT_QSOC_NVM_SYS_STATUS_FILE_READ_ERROR    = 108 /**< NVM File Read Error */
} bt_qsoc_nvm_status;

/** Enum for the Reference Clk Information */
typedef enum
{
  BT_SOC_REFCLOCK_32MHZ = 0,
  BT_SOC_REFCLOCK_19P2MHZ
} bt_soc_refclock_type;

/** Enum for the Clk Sharing Information */
typedef enum
{
  BT_SOC_CLOCK_SHARING_DISABLED = 0,
  BT_SOC_CLOCK_SHARING_ENABLED
} bt_soc_clock_sharing_type;

/** Enum for the Device Class Information */
typedef enum
{
  /** device class 1 */
  BT_QSOC_DEV_CLASS1 = 0,
  /** device class 2 */
  BT_QSOC_DEV_CLASS2,
  /** device class enum for custom power levels 
          
      @note This should be used sparingly!
  */
  BT_QSOC_DEV_CLASS_CUSTOM
} bt_qsoc_device_class_type;

/** NVM Mode 

  Default is NVM_AUTO_MODE.
  
  @see bt_qsoc_nvm_open
*/
typedef enum
{
  /** Automatic NVM generation based on SOC type.
  
    Auto-mode generates NVM data based on the SOC type and
    input parameters passed via a bt_qsoc_config_params_struct_type
    structure.  Three classes of NVM data are created: fixed, runtime,
    and dynamic.  The fixed data consists of NVM flags that are common 
    for a particular SOC type or family.  The runtime data consists of 
    flags that are tweaked based on configurations parameters that are
    different for each mobile unit.  For example, the bluetooth address.
    The dynamic data is parsed from a release NVM string and contains 
    patches and other configuration data related to a particular SOC
    type.
    
    After the three classes of NVM data are generated they are sent to
    the SOC in a particular order.  First the fixed data, followed by
    the runtime data, and finally the dynamic data.  If a NVM item exists
    in the dynamic data and the fixed or runtime data it will be ignored;
    the fixed and runtime data has higher priority.  If the same NVM item 
    is found in both the fixed and runtime data the runtime tag will be 
    ignored.
  */
  NVM_AUTO_MODE = 0,
  
  /** NVM data parsed from file.
  
    In EFS-mode all of the NVM items are generated based on a file.  The
    NVM items are parsed and sent to the SOC in the same order that they
    are found in the file.
  */
  NVM_EFS_MODE,
  
  /** Hybrid mode merges auto-mode and EFS-mode data.
  
    Hybrid mode combines the auto- and EFS-mode operation.  Three classes
    of data are generated like they are in auto-mode.  The dynamic data
    generated from the auto-mode string is overriden by the data found in
    the EFS file.  A tag in the EFS file will trump the same tag in the 
    atuo-mode string but the fixed and runtime data will trump them both.
    For example, if there is a Bluetooth address tag in the auto-mode string
    and the EFS file, they are both ignored and the Bluetooth address tag in
    the runtime table is used instead.
  */
  NVM_HYBRID_MODE
} bt_qsoc_enum_nvm_mode;

/** Parser Error Codes */
typedef enum
{
  BTQSOCNVM_NO_ERROR = 0,
  BTQSOCNVM_NUMBER_FORMAT_ERR,
  BTQSOCNVM_FILE_READ_ERR,
  BTQSOCNVM_QSOC_NOT_SUPPORTED
} btqsocnvm_parser_err_type;

/** SOC Sleep Method */
typedef enum
{
  BT_QSOC_HW_INBAND_SLEEP = 0,
  BT_QSOC_SW_INBAND_SLEEP
} bt_qsoc_inband_sleep_type;

/** Bluetooth QSOC Baud Rate Codes */
typedef enum
{
  BT_QSOC_BAUD_115200      = 0x0,
  BT_QSOC_BAUD_57600       = 0x1,
  BT_QSOC_BAUD_38400       = 0x2,
  BT_QSOC_BAUD_19200       = 0x3,
  BT_QSOC_BAUD_9600        = 0x4,
  BT_QSOC_BAUD_230400      = 0x5,
  BT_QSOC_BAUD_250000      = 0x6,
  BT_QSOC_BAUD_460800      = 0x7,
  BT_QSOC_BAUD_500000      = 0x8,
  BT_QSOC_BAUD_720000      = 0x9,
  BT_QSOC_BAUD_921600      = 0xA,
  BT_QSOC_BAUD_1000000     = 0xB,
  BT_QSOC_BAUD_125000      = 0xC,
  BT_QSOC_BAUD_2000000     = 0xD,
  BT_QSOC_BAUD_3000000     = 0xE,
  BT_QSOC_BAUD_4000000     = 0xF,
  BT_QSOC_BAUD_1600000     = 0x10,
  BT_QSOC_BAUD_3200000     = 0x11,
  BT_QSOC_BAUD_3500000     = 0x12,
  BT_QSOC_BAUD_Auto        = 0xFE,
  BT_QSOC_BAUD_UserDefined = 0xFF 
} bt_qsoc_enum_baud_rate;

/** WLAN Coexistence Type.

  Default is BT_QSOC_WLAN_LIBRA.
  
  @see bt_qsoc_nvm_open
*/
typedef enum
{
  /** Use the default coexistence tags.

    The default has a value of zero so that memset can be used to 
    initialize the bt_qsoc_config_params_struct.
  */
  /** Force the compiler to keep this enum a minimum of 32-bits. */
  BT_QSOC_WLAN_DEFAULT = 0x00000000,

  /** Use Libra coexistence tags.

    The default mode is to use the tags for Libra.
  */
  BT_QSOC_WLAN_LIBRA = BT_QSOC_WLAN_DEFAULT,
  
  /** Use Atheros coexistence tags.

    This mode will use the Atheros coexistence tags that were updated for Libra.
  */
  BT_QSOC_WLAN_ATHEROS
} bt_qsoc_enum_wlan_type;

/*--------------------------------------------------------------
Structure definitions
--------------------------------------------------------------*/

#define BT_QSOC_MAX_BD_ADDRESS_SIZE  0x06  /**< Length of BT Address */

/** Structure used to configure the BT QSOC NVM subsystem 

    @see    bt_qsoc_nvm_open

    These configuration values will be reflected in the contents of
    various NVM tags returned by bt_qsoc_nvm_get_next_cmd()
*/
typedef struct 
{
  uint8                       bd_address[BT_QSOC_MAX_BD_ADDRESS_SIZE];
  uint8                       soc_logging;
  uint8                       bt_2_1_lisbon_disabled;
  bt_soc_refclock_type        refclock_type;
  bt_soc_clock_sharing_type   clock_sharing;
  bt_qsoc_device_class_type   bt_qsoc_bredr_dev_class;
  bt_qsoc_device_class_type   bt_qsoc_le_dev_class;
  
  /** The following structure elements provide control over size and number
    of buffers in the BT SOC.  There is also a mechanism to indicate that
    defaults values are to be used for these elements:
    
    if num_rx_buf is 0, 
        default values will be used for num_rx_buf and max_hc_rx_len
    
    if num_tx_buf is 0, 
        default values will be used for num_tx_buf and max_hc_tx_len
        
    if sco_buf_size is 0,
        a default value will be used for sco_buf_size
  */
  uint16                      max_hc_rx_len;
  uint16                      max_hc_tx_len;
  uint8                       num_rx_buf;
  uint8                       num_tx_buf;
  uint8                       sco_buf_size;
  /** Type of WLAN coexistence tags to output. */
  bt_qsoc_enum_wlan_type      wlan_type;
  /** FTM Mode enabled or Disabled **/
  uint8	                      is_ftm_mode;
  uint8                       is_xo_support_enabled;
  
} bt_qsoc_config_params_struct_type;

#define BT_QSOC_MAX_VS_POKE8_SIZE    0xF0  /**< Maximum length of poke8 payload */

/** Poke table structure */
typedef struct
{
  uint8  vs_poke8_data_len;
  uint8  vs_poke8_data[BT_QSOC_MAX_VS_POKE8_SIZE];
} bt_qsoc_poke8_tbl_struct_type;

/*--------------------------------------------------------------
Union definitions
--------------------------------------------------------------*/

/*--------------------------------------------------------------
Macros
--------------------------------------------------------------*/

/**
  Macros for testing status/error codes.
  
  @see bt_qsoc_nvm_status
*/
#define BT_QSOC_NVM_IS_STATUS_OK(x)        ((x) == BT_QSOC_NVM_STATUS_OK)
#define BT_QSOC_NVM_IS_STATUS_SUCCESS(x)   ((x) == BT_QSOC_NVM_STATUS_SUCCESS)
#define BT_QSOC_NVM_IS_STATUS_COMPLETE(x)  ((x) == BT_QSOC_NVM_STATUS_COMPLETE) 

/** Default bit rate featurization 

  @see bt_qsoc_enum_baud_rate
*/
#ifdef FEATURE_BT_SOC_BITRATE_460800

#define BT_QSOC_BAUD_RATE  BT_QSOC_BAUD_460800

#elif defined FEATURE_BT_SOC_BITRATE_115200

#define BT_QSOC_BAUD_RATE  BT_QSOC_BAUD_115200

#else

#define BT_QSOC_BAUD_RATE  BT_QSOC_BAUD_3200000

#endif /* FEATURE_BT_SOC_BITRATE_460800 */

/*--------------------------------------------------------------
Constant values
--------------------------------------------------------------*/

#define BT_QSOC_SHUTDOWNBL_COUNT     1

#ifdef FEATURE_BT_QSOC_INBAND_SLEEP
#define BT_QSOC_R3_POKETBL_COUNT     2
#else
#define BT_QSOC_R3_POKETBL_COUNT     1
#endif /* FEATURE_BT_QSOC_INBAND_SLEEP */

#define   BT_QSOC_BHA_B0_POKETBL_COUNT    1

#define   HCI_VS_NULL_CHAR (0x20)

#define BT_QSOC_SOC_LOGGING_ENABLED(x)    ((x) == 1)
#define BT_QSOC_IS_FTM_MODE(x)            ((x) == 1)
#define BT_QSOC_IS_XO_SUPPORT_ENABLED(x)  ((x) == 1)
#define BT_2_1_FEATURE_SUPPORTED(x)       ((x) != 1)

/*--------------------------------------------------------------
Function declarations
--------------------------------------------------------------*/

/*==============================================================
FUNCTION:  bt_qsoc_nvm_open
==============================================================*/
/**
  This function initializes the NVM system.

  @see bt_qsoc_nvm_get_next_cmd
  @see bt_qsoc_nvm_close
  @see bt_qsoc_enum_nvm_mode
  
  @return  Returns the enum bt_qsoc_nvm_status.
           BT_QSOC_NVM_STATUS_SUCCESS if function call succeeded.
           BT_QSOC_NVM_INIT_FAIILED if function call failed.
           BT_QSOC_NVM_STATUS_NOT_IMPLEMENTED if functionality is not implemented 
           (e.g caller selected an unknown nvm mode).
           BT_QSOC_NVM_STATUS_INVALID_PARAMETERS if invalid parameter are passed.
           BT_QSOC_NVM_ALREADY_INITIALIZED the open function called.
           
  @sideeffects Undetermined.
*/
extern bt_qsoc_nvm_status bt_qsoc_nvm_open
(
  /** [in] QSOC type. */
  bt_qsoc_enum_type     qsoc_type,
  
  /** [in] NVM mode. */
  bt_qsoc_enum_nvm_mode mode,
  
  /** [in] Runtime configuration parameters. */
  bt_qsoc_config_params_struct_type *bt_qsoc_vs_nvm_config  
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_get_next_cmd
==============================================================*/
/**
  The function returns the next NVM command to be sent to SOC.
  
  bt_qsoc_nvm_open() should be called before calling this function.  
  
  The NVM tag string that is returned consists of a length byte followed
  by one or more payload bytes.

  @see bt_qsoc_nvm_open
  @see bt_qsoc_nvm_close
  
  @return  Returns the enum bt_qsoc_nvm_status.
           BT_QSOC_NVM_STATUS_SUCCESS if function call succeeded.
           BT_QSOC_NVM_STATUS_COMPLETE exhausted no more tags left to deliver. 
           BT_QSOC_NVM_STATUS_FAIL NVM subsystem internal failure.
           BT_QSOC_NVM_STATUS_NOT_IMPLEMENTED if functionality is not implemented.
           BT_QSOC_NVM_STATUS_INVALID_PARAMETERS if NVM pointer is invalid.
           
  @sideeffects Undetermined.
*/
extern bt_qsoc_nvm_status bt_qsoc_nvm_get_next_cmd
(
  /** Pointer to a pointer to a NVM command string. */
  uint8 **soc_nvm_ptr
);

/*==============================================================
FUNCTION:  bt_qsoc_nvm_close
==============================================================*/
/**
  This function cleans up any memory allocated or modified by the NVM system.
  

  @see bt_qsoc_nvm_open
  @see bt_qsoc_nvm_get_next_cmd
  
  @return  Returns the enum bt_qsoc_nvm_status.
           BT_QSOC_NVM_STATUS_SUCCESS if function call succeeded.           
           
  @sideeffects Undetermined.
*/
extern bt_qsoc_nvm_status bt_qsoc_nvm_close(void); 

/*==============================================================
FUNCTION:  bt_qsoc_nvm_tot_hc_tx_len
==============================================================*/
/**
  This function returns the length of the SOC transmit buffer.
  
  @see bt_qsoc_tot_hc_tx_len
  @see bt_soc_tot_hc_tx_len
  
  @return  Returns the enum bt_qsoc_nvm_status.
           BT_QSOC_NVM_STATUS_SUCCESS if function call succeeded.           
           BT_QSOC_NVM_STATUS_INVALID_PARAMETERS if length pointer is invalid.
           
  @sideeffects Undetermined.
*/
extern bt_qsoc_nvm_status bt_qsoc_nvm_tot_hc_tx_len(uint16 *tx_len_ptr); 

/*------------------------------------------------------------------------------
Globals
------------------------------------------------------------------------------*/
/** Fixed poke table for BTS' VS Poke8 command. The elements of this 
    table are pretty much hardcoded.  
*/
extern const bt_qsoc_poke8_tbl_struct_type bt_qsoc_vs_poke8_tbl_r3[];
extern const bt_qsoc_poke8_tbl_struct_type bt_qsoc_vs_poke8_tbl_bha_b0[];

/*------------------------------------------------------------------------------
Notes
------------------------------------------------------------------------------*/
 
/** @} */ /* end of btqsocvm group */

#ifdef __cplusplus
};
#endif 

#endif /* #ifndef _BTQSOCNVM_H_ */
