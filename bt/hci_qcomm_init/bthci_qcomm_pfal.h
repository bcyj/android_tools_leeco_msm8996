#ifndef _BTHCI_QCOMM_PFAL_H_
#define _BTHCI_QCOMM_PFAL_H_
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

	   B L U E T O O T H    P F A L  B T S   D R I V E R    H E A D E R    F I L E

GENERAL DESCRIPTION
  Platform level abstraction 
EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2007-2012, 2014 Qualcomm Technologies, Inc. All Rights Reserved.  Qualcomm Technologies Proprietary and Confidential.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*==============================================================================
							  Edit History
when         who        what, where, why
----------   ----       ----------------------------------------------------------
02/29/2012   rr         Added/Modified API to get LE & BR/EDR SoC power class
08/17/2011   an         Added command to send HCI reset without changing baud rate
07/18/2011   bn         Added support for 8960 for sending NVM tags to RIVA.
06/09/2009   JMF        Merged up to #14 - VU_WCONNECT_BTHOST_WM_HCI.00.00.31.
05/11/2009   RH       Added RETAIL debugzone support, configurable via registry
01/31/2009    JMF       Merged up to aswp401 VU_WCONNECT_BTHOST_WM_HCI.00.00.20
2008-07-10    jmf       Merge aswp401 change 699599: hci_qcomm_init hang fix, cleanup.
2008-04-07    jmf       ported to .../LINUX/common/apps/hci_qcomm_init/inc
01/17/2008   RH       Merged in damu's 19.2Mhz/BT-Coexist code
11/09/2007   DR       Added BT Coex support
===============================================================================*/

/*===========================================================================
						 INCLUDE FILES FOR MODULE
===========================================================================*/

#include "btqsocnvm.h"  /* for bt_qsoc_enum_wlan_type */

/*=========================================================================*/
/*                               CONSTANTS                                 */
/*=========================================================================*/


/* enable only ZONE_ERROR by default for linux - want silent success */
#define DEFAULT_BT_ZONEMASK      0x000B

#define ZONE_ERROR      0
#define ZONE_WARNING    1
#define ZONE_FUNCTION   2
#define ZONE_INIT       2
#define ZONE_INFO       1
#define ZONE_IST        2
#define ZONE_VERBOSE    2
#define ZONE_DEBUG      2


/*=========================================================================*/
/*                               TYPEDEFS                                  */
/*=========================================================================*/

/*===========================================================================
			 Tables for NVM configuration during bootup
===========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif 

/*=========================================================================*/
/*                           DATA DECLARATIONS                             */
/*=========================================================================*/
extern unsigned int g_CelogEnabled;

typedef enum
{
  BT_HCI_UART,
  BT_HCI_SMD,
  BT_HCI_NONE = 0xFF
} bt_hci_transport_enum_type;

typedef struct
{
  /*transport type can be SMD/UART*/
  bt_hci_transport_enum_type type;

  /*hci cmd/event packet is required to carry the Packet indicator for UART interfaces only
    and not required for smd */
  int pkt_ind;

  /*transport device can be UART/SMD*/
  char *name;

} bt_hci_transport_device_type;

/*=========================================================================*/
/*                                MACROS                                   */
/*=========================================================================*/
//#define BTHCI_QCOMM_TRACE(x)      RETAILMSG(TRUE, (x))

#define DEBUGMSG(lev,format_etc...) \
if (verbose >= (lev))                \
{                                    \
  fprintf (stderr, format_etc); \
  fprintf (stderr, "%s", "\n");      \
  fflush (stderr);                   \
}

#define BTHCI_QCOMM_ERROR printf

#if defined(__linux__)
extern int verbose;
#endif /* __linux__ */

/*=========================================================================*/
/*                           FUNCTION INTERFACES                           */
/*=========================================================================*/
/*===========================================================================
FUNCTION   bt_hci_set_transport

DESCRIPTION
 sets the type of transport based on the msm type

DEPENDENCIES
  NIL

RETURN VALUE
sets the type of transport and device
SIDE EFFECTS
  None

===========================================================================*/
bt_hci_transport_device_type bt_hci_set_transport
(
  void
);

/*===========================================================================
FUNCTION   bt_hci_pfal_init_transport

DESCRIPTION
  Platform specific routine to intialise the UART/SMD resources.

PLATFORM SPECIFIC DESCRIPTION
  Opens the TTY/SMD device file descriptor, congiures the TTY/SMD device for CTS/RTS
  flow control,sets 115200 for TTY as the default baudrate and starts the Reader
  Thread

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  STATUS_SUCCESS if SUCCESS, else other reasons

SIDE EFFECTS
  None

===========================================================================*/


extern int bt_hci_pfal_init_transport
(
  bt_hci_transport_device_type bt_hci_transport_device
);

/*===========================================================================
FUNCTION   bt_hci_pfal_deinit_transport

DESCRIPTION
  Platform specific routine to de-intialise the UART/SMD resource.

PLATFORM SPECIFIC DESCRIPTION
  Closes the TTY/SMD file descriptor and sets the descriptor value to -1

DEPENDENCIES
  NIL

RETURN VALUE
  RETURN VALUE
  STATUS_SUCCESS if SUCCESS, else other reasons

SIDE EFFECTS
  The Close of the descriptor will trigger a failure in the Reader Thread
  and hence cause a Deinit of the ReaderThread

===========================================================================*/
boolean bt_hci_pfal_deinit_transport();

/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_changebaudrate

  DESCRIPTION    This routine changes SoC baud rate

  DEPENDENCIES   None.

  PARAMETERS     uint32 uibaudrate:  new baud rate
                 (if uibaudrate is 0, then skip baud rate change)

  RETURN VALUE   boolean: TRUE if baud rate change is successful
				 
  SIDE EFFECTS   SoC baud rate will be changed

==========================================================================*/
    boolean bt_hci_qcomm_pfal_changebaudrate( unsigned long uibaudrate );

/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_hcireset_baudratechange

  DESCRIPTION    This routine does HCI Reset and SoC baud rate change

  DEPENDENCIES   None.

  PARAMETERS     uint32 duibaudrate:  new baud rate

  RETURN VALUE   boolean: TRUE if Reset and baud rate change is successful
				 
  SIDE EFFECTS   SoC will be reset and baud rate will be changed

==========================================================================*/
    boolean bt_hci_qcomm_pfal_hcireset_baudratechange(  uint32 uibaudrate );

/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_hcireset_smd

  DESCRIPTION    This routine does HCI Reset when using SMD as the trsnasport

  DEPENDENCIES   None

  PARAMETERS     None

  RETURN VALUE   boolean: TRUE if Reset

  SIDE EFFECTS   SoC will be reset

==========================================================================*/
    boolean bt_hci_qcomm_pfal_hcireset_smd( void  );

/*==========================================================================
  FUNCTION       bt_hci_qcomm_pfal_powerup

  DESCRIPTION    This routine performs the powerup procedure for the 4020 
				 SOC chip and does the 4020/MSM sleep/wakeup setup

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE if the powerup sequence is successful
				 
  SIDE EFFECTS   EXT_WAKE and HOST_WAKE pins will be altered

==========================================================================*/
	boolean bt_hci_qcomm_pfal_powerup( void );

/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_shutdown

  DESCRIPTION    This routine performs the shutdown procedure for the 4020 
				 SOC chip and does all the sleep/wakeup cleanup

  DEPENDENCIES   None.

  RETURN VALUE   None
				 
  SIDE EFFECTS   4020 SoC's baud rate will be reset to 115200

==========================================================================*/
	void bt_hci_qcomm_pfal_shutdown( void );


/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_wake

  DESCRIPTION    This routine wakes up the 4020 SOC

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE if wakeup is successful, otherwise FALSE
				 
  SIDE EFFECTS   MSM and 4020 SOC will be awake

==========================================================================*/
	boolean bt_hci_qcomm_pfal_wake( void );


/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_sleep

  DESCRIPTION    This routine enables 4020 SoC to go to sleep

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE if successful
				 
  SIDE EFFECTS   4020 SOC will be allowed to sleep

==========================================================================*/
	boolean bt_hci_qcomm_pfal_sleep( void );

/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_vs_sendcmd

  DESCRIPTION    This routine sends Vendor specific HCI Command to the SOC
				 and read back event

  DEPENDENCIES   None.

  PARAMETERS     uint16 opcode:  command opcode
				 uint8 pCmdBuffer: command buffer
				 uint8 nSize: command length

  RETURN VALUE   boolean: TRUE if the command is successful sent and event is 
				 received.

  SIDE EFFECTS   None

==========================================================================*/
	boolean bt_hci_qcomm_pfal_vs_sendcmd( /* [IN]*/ unsigned short opcode, /* [IN]*/ unsigned char *pCmdBuffer,	/* [IN]*/ unsigned char nSize );


/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_get_bdaddress

  DESCRIPTION    This routine reads the NV BD Address using an RPC Wrapper

  DEPENDENCIES   NV Item being present and a RPC wrapper being present.

  PARAMETERS     uint8 pCmdBuffer: buffer to place the BD Address
				 

  RETURN VALUE   boolean: TRUE if the NV Read is successful sent

  SIDE EFFECTS   None

==========================================================================*/
    boolean bt_hci_qcomm_pfal_get_bdaddress( uint8 *pCmdBuffer );


/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_waitfor_commandcomplete

  DESCRIPTION    This routine waits until Command Complete event is received.

  DEPENDENCIES   None.

  PARAMETERS     boolean logEvent: log the event using celog if TRUE

  RETURN VALUE   boolean: TRUE if HCI event is read successfully
				 
  SIDE EFFECTS   None

==========================================================================*/
	boolean bt_hci_qcomm_pfal_waitfor_commandcomplete ( boolean logEvent );


/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_go_to_bt_dutmode

  DESCRIPTION    This routine sends down all the HCI Commands to the SOC, so
				 any BT Tester can connect to the SOC without requiring a Host
				 to perform BT RF/Baseband tests.

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE if all the commands were sent successfully.

  RETURN VALUE   None.

  SIDE EFFECTS   SOC will be in auto connect accept mode with Authentication
				 and Encryption enabled with Page&Inquiry Scans enabled and 
				 DUT Mode enabled.

==========================================================================*/
	boolean bt_hci_qcomm_pfal_go_to_bt_dutmode( void );


#ifdef FEATURE_BT_WLAN_COEXISTENCE
/*==========================================================================

  FUNCTION       bt_hci_qcomm_wm_btcoex_init

  DESCRIPTION    This routine performs the initialization related to BT Coex

  DEPENDENCIES   None.

  RETURN VALUE   TRUE if initialization succeeded else FALSE
				 
  SIDE EFFECTS   None

==========================================================================*/
	boolean bt_hci_qcomm_wm_btcoex_init( void );

/*==========================================================================

  FUNCTION       bt_hci_qcomm_wm_btcoex_deinit

  DESCRIPTION    This routine does a clean up related to BT Coex

  DEPENDENCIES   None.

  RETURN VALUE   None
				 
  SIDE EFFECTS   None

==========================================================================*/
	void bt_hci_qcomm_wm_btcoex_deinit( void );
#endif /* FEATURE_BT_WLAN_COEXISTENCE */


#ifdef FEATURE_BT_DYNAMIC_TLMM

    // once UART is configuring TLMM for their pins, we don't need to do it in BT
    boolean bt_hci_qcomm_pfal_config_uart_tlmm( void );
    boolean bt_hci_qcomm_pfal_unconfig_uart_tlmm( void );
    boolean bt_hci_qcomm_pfal_discharge_uart_pins( void );

/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_config_bt_tlmm

  DESCRIPTION    This routine does dynamic TLMM config for BT pins

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE if config is successful, otherwise FALSE
				 
  SIDE EFFECTS   BT pins will be configured (not lowest power consumption mode)

==========================================================================*/
    boolean bt_hci_qcomm_pfal_config_bt_tlmm( void );

/*==========================================================================

  FUNCTION       bt_hci_qcomm_pfal_unconfig_bt_tlmm

  DESCRIPTION    This routine unconfigure TLMM BT pins

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE if config is successful, otherwise FALSE
				 
  SIDE EFFECTS   BT pins should be unconfigured back to lowest power state

==========================================================================*/
    boolean bt_hci_qcomm_pfal_unconfig_bt_tlmm( void );
#endif  /* FEATURE_BT_DYNAMIC_TLMM */


/* These helper functions will return the SOC setting for clock, clock sharing and logging */
    boolean bt_hci_qcomm_pfal_is_refclock_19P2MHz( void );

    boolean bt_hci_qcomm_pfal_is_clocksharing_enabled( void );

    boolean bt_hci_qcomm_pfal_is_soclogging_enabled( void ); // SOC's logging enabled? (for debugging)

    boolean bt_hci_qcomm_pfal_is_lisbon_enabled( void );  // SOC's BT 2.1(Lisbon) should be enabled or not?

    //RF power class in BR/EDR BT SoC
    bt_qsoc_device_class_type bt_hci_qcomm_pfal_get_bredr_dev_class_type( void );

    //RF power class in LE BT SoC
    bt_qsoc_device_class_type bt_hci_qcomm_pfal_get_le_dev_class_type( void );

/*==============================================================
 * FUNCTION:  bt_hci_qcomm_pfal_get_wlan_type
 * ==============================================================*/
/**
 *   Return the WLAN type for SOCCFG NVM parameters.
 *
 *     @see bt_qsoc_nvm_open
 *
 *       @return  bt_qsoc_enum_wlan_type.
 *       */
bt_qsoc_enum_wlan_type bt_hci_qcomm_pfal_get_wlan_type( void );

/*==============================================================
 * FUNCTION:  bt_hci_qcomm_pfal_is_xo_enabled
 * ==============================================================*/
/**
 *   Return 0/1 implies no/yes.
 *
 *       @return  uint8
 *       */
uint8 bt_hci_qcomm_pfal_is_xo_enabled( void );


#ifdef __cplusplus
}
#endif 

#endif /* _BTHCI_QCOMM_PFAL_H_ */


