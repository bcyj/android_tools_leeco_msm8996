//source/qcom/qct/wconnect/bthost/wm/hci/transport/main/latest/src/bthci_qcomm.h#3 - edit change 1157287 (ktext)
#ifndef _BTHCI_QCOMM_H_
#define _BTHCI_QCOMM_H_


/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

	   B L U E T O O T H   COMMON   D R I V E R    H E A D E R    F I L E

GENERAL DESCRIPTION
EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2007-2011 by Qualcomm Technologies, Inc. All Rights Reserved.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*==============================================================================
							  Edit History
when        who  what, where, why
----------  ---  ----------------------------------------------------------
09/28/2011  rrr  Moved the implementation to CPP, for having BD address being
                 programmed twice if previous BD address was random generated.
08/17/2011   an  Add macro for HCI reset cmd over SMD transport
01/27/2011  dgh  Added prototype for GetFilterType().
11/26/2008   RH  Updated function descriptions
01/17/2008   RH  Merged in damu's 19.2Mhz/BT-Coexist code
11/09/2007   DR  Added BT Coex support
===============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
						 INCLUDE FILES FOR MODULE
===========================================================================*/
#include <comdef.h>  

/*=========================================================================*/
/*                               CONSTANTS                                 */
/*=========================================================================*/
/* define to support 4020 & MSM sleep/wakeup */
//#define FEATURE_BT_QSOC_SLEEP -> moved to sources

/* define to de-assert EXT_WAKE after HCI_WritePacket() finish writing, no timeout waiting */
//#define FEATURE_BT_QSOC_SLEEP_EXTWAKE_NOTIMEOUT -> moved to sources

/* define to support BT WLAN Coexistence */
//#define FEATURE_BT_WLAN_COEXISTENCE -> moved to sources

/* define to support BT WLAN Coexistence and send AFH Channel Classification */
//#define FEATURE_BT_WLAN_COEX_CHAN_CLASS -> moved to sources

#define HCI_DUT_MODE_ENABLED  (0x01)


/* Vendor Specific command codes */
#define BT_QSOC_EDL_CMD_OPCODE             (0xFC00)
#define BT_QSOC_NVM_ACCESS_OPCODE          (0xFC0B)

#define BT_QSOC_EDL_CMD_CODE             (0x00)
#define BT_QSOC_NVM_ACCESS_CODE          (0x0B)
#define BT_QSOC_VS_EDL_APPVER_RESP   	(0x02)

/*
* things that were in old bthci_qcomm.h
*/

#ifndef BT_QSOC_NVM_ACCESS_OPCODE
#define BT_QSOC_NVM_ACCESS_OPCODE (0xFC0B)
#endif /* BT_QSOC_NVM_ACCESS_OPCODE */

#ifndef BT_QSOC_INIT_BAUDRATE
#define BT_QSOC_INIT_BAUDRATE  (115200)
#endif /* BT_QSOC_INIT_BAUDRATE */

#ifdef FEATURE_BT_SOC_BITRATE_460800
#define BT_QSOC_APP_BAUDRATE             (460800)
#else /* FEATURE_BT_SOC_BITRATE_460800 */
#define BT_QSOC_APP_BAUDRATE             (3000000)
#endif /* !FEATURE_BT_SOC_BITRATE_460800 */

#ifndef HCI_CMD_HDR_SIZE
#define HCI_CMD_HDR_SIZE  4
#endif /* HCI_CMD_HDR_SIZE */

#ifndef HCI_EVT_HDR_SIZE
#define HCI_EVT_HDR_SIZE  3
#endif /* HCI_EVT_HDR_SIZE */

#ifndef RESET_CMD_SIZE_UART
#define RESET_CMD_SIZE_UART  4
#endif /* RESET_CMD_SIZE_UART */

#ifndef RESET_CMD_SIZE_SMD
#define RESET_CMD_SIZE_SMD  3
#endif /* RESET_CMD_SIZE_SMD */

#ifndef HC_VS_MAX_CMD_EVENT
#define HC_VS_MAX_CMD_EVENT  260
#endif /* HC_VS_MAX_CMD_EVENT */

#define BT_QSOC_MAX_NVM_CMD_SIZE     0x64  /* Maximum size config (NVM) cmd  */

typedef struct
{
  uint8  vs_cmd_len;
  uint8  vs_cmd_data[BT_QSOC_MAX_NVM_CMD_SIZE];
} bt_qsoc_cfg_tbl_struct_type;

#define TEXT(string...) string

/*=========================================================================*/
/*                           DATA DECLARATIONS                             */
/*=========================================================================*/

/*=========================================================================*/
/*                                MACROS                                   */
/*=========================================================================*/


/*=========================================================================*/
/*                           FUNCTION INTERFACES                           */
/*=========================================================================*/


/*==========================================================================

  FUNCTION       bt_hci_qcomm_init

  DESCRIPTION    This routine performs the SOC Initialization by sending
				 the Vendor specific HCI Command to the SOC.

  DEPENDENCIES   None.

  PARAMETERS     unsigned int 	uiInitBaudRate: init at this baud rate
                 unsigned int   uiRetryBaudRate: retry at this baud rate
                 (if it's 0, no retry)

  RETURN VALUE   boolean: TRUE if the initialization sequence was successful.

  RETURN VALUE   None.

  SIDE EFFECTS   bt_qsoc_init_state are updated.

==========================================================================*/
	boolean bt_hci_qcomm_init
	(
	unsigned int 	uiInitBaudRate, 
    unsigned int    uiRetryBaudRate
	);

/*==========================================================================

  FUNCTION       bt_hci_qcomm_powerup

  DESCRIPTION    This routine invokes the platform specific routine that
				 powers up BTS.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   boolean: Currently always TRUE

  SIDE EFFECTS   None.

==========================================================================*/
	boolean bt_hci_qcomm_powerup
	(
	void 
	);

/*==========================================================================

  FUNCTION       bt_hci_qcomm_shutdown

  DESCRIPTION    This routine invokes the platform specific routine that
				 shuts down BTS.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
	void    bt_hci_qcomm_shutdown
	(
	void 
	);

/*==========================================================================

  FUNCTION       bt_hci_qcomm_sleep

  DESCRIPTION    This routine invokes the platform specific routine that
				 allows the SOC to enter deep sleep.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   boolean: Currently always TRUE

  SIDE EFFECTS   SIO pins will be altered.

==========================================================================*/
	boolean bt_hci_qcomm_sleep
	(
	void 
	);

/*==========================================================================

  FUNCTION       bt_hci_qcomm_wake

  DESCRIPTION    This routine invokes the platform specific routine to wake
				 up SOC from Deep Sleep.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
	boolean bt_hci_qcomm_wake
	(
	void 
	);

/*==========================================================================

  FUNCTION       bt_hci_qcomm_send_shutdown_cmds

  DESCRIPTION    This routine sends down the Vendor specific commands that are
				 part of the Shutdown sequence.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   boolean: TRUE is the sequence was sent successfully.

  SIDE EFFECTS   The BTS SOC will be returned to the Baud rate 115200 from 3.2MBaud

==========================================================================*/
	boolean bt_hci_qcomm_send_shutdown_cmds
	(
	void 
	);

/*==========================================================================

  FUNCTION       bt_hci_qcomm_vs_event

  DESCRIPTION    This routine handles the Vendor specific
				 event returned by the SOC.

  DEPENDENCIES   None.

  PARAMETERS     const uint8 *pEventBuffer: Event returned.
				 uint8 nLength: Event length

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
	void bt_hci_qcomm_vs_event
	(
	const uint8* pEventBuffer, 
	uint8 nLength 
	);


#if defined( FEATURE_BT_WLAN_COEXISTENCE ) && defined( FEATURE_BT_WLAN_COEX_CHAN_CLASS )
/*==========================================================================

  FUNCTION       bt_wlan_coex_soc_calc_chan_class

  DESCRIPTION    Called to calculate the channel classification from the 
	               WLAN Channel number.

  PARAMETERS     uint8 wlan_chan_num:  WLAN channel number		
	             uint8 *chan_class_data_ptr: return buffer (pointing to 10 bytes)
  
  RETURN VALUE   None.

  SIDE EFFECTS   chan_class_data_ptr updated with channel classification..

==========================================================================*/
	void bt_wlan_coex_soc_calc_chan_class
	(
	uint8    wlan_chan_num,		/*Input */
	uint8    *chan_class_data_ptr  /*Input/Output */         
	);
#endif /* FEATURE_BT_WLAN_COEXISTENCE  && FEATURE_BT_WLAN_COEX_CHAN_CLASS */


/*==========================================================================

  FUNCTION       bt_hci_qcomm_is_soc_initialized

  DESCRIPTION    This routine provides the state if the SOC Initialization.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   boolean: TRUE is the SOC Init was done successfully, else FALSE

  SIDE EFFECTS   None

==========================================================================*/
boolean bt_hci_qcomm_is_soc_initialized
	(
	void 
	);

#if 0
/*==============================================================
FUNCTION:  GetFilterType
==============================================================*/
/**
  Return the filter type based on the WLAN driver registry values.

  @note Using default parameters here so that code remains compatible
        with old function signature.
  
  @return  BTFILTER_TYPE.
*/
BTFILTER_TYPE GetFilterType( void );
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BTHCI_QCOMM_H_ */


