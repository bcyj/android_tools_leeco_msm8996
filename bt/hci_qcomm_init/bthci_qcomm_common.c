//source/qcom/qct/wconnect/bthost/wm/hci/transport/main/latest/src/bthci_qcomm_common.cpp#2 - edit change 1157287 (ktext)
/**
 * Copyright (c) 2007-2012 by Qualcomm Technologies, Inc.  All Rights
 * Reserved.
 *
 * @file bthci_qcomm_common.cxx
 *  This file implements Bluetooth Host Controller Common
 *  interface (UART, GPIO, Power, AUX_PCM)
 * 
 * 
 */

/*==============================================================================
                              Edit History
when        who  what, where, why
----------  ---  ----------------------------------------------------------
10/01/2012  dv   Added support for wcn2243 2.1 SOC.
02/29/2012  rr   Added/Modified configuration of LE & BR/EDR power class
08/17/2011  an   Send HCI reset after NVM commands for SMD transport
2011-07-18  bn   Added support for 8960 for sending NVM tags to RIVA.
2011-02-14  nkr  Added retry for hci_reset and baudrate change if fails for the first time.
2011-02-07  bhr  Adding -g and -o options for dumping the hci commands to a file.
2010-01-27  dgh  Converted file to a C++ file.  Initialized the wlan_type value.  
11/14/2009  MP	 Integrated P3 fixes
2009-10-19  DGH  Added support for SOCCFG mainline interface.
2009-10-05  DGH  Added support for SOCCFG branch 26.
05/26/2009  RH   Added EFS mode support for WM
12/17/2008  NS   Added NVM automation support  
09/30/2008  RH   Added 4025 A0/B0/B1 SOC support
06/04/2008  RH   Added uiInitBaudRate/uiRetryBaudRate for qcomm_init 
04/18/2008  RH   Added support for BT init retry at 3.2Mpbs
03/12/2008  RH   Added 4021 B1 support
02/05/2008  RH   Added 4020BD B0 support
01/31/2008  RH   Some fixes after merging in bretth's update, added default BT address 
01/17/2008  RH   Merged in damu's 19.2Mhz/BT-Coexist code
11/21/2007  DR   Sync NVM to AMSS //depot/asic/msmshared/services/bt/dep/2007.11/services/bt/soc/btqsoc.c#4 (2007/11/21)
11/09/2007  DR   Sync NVM to AMSS btqsoc.c#29 (2007/11/06)
11/09/2007  SK   Route DBGMSG to Celog
10/30/2007  RH   Bug fix in pfal_powerup, return value is ignored
10/15/2007  RH   Changed the default BT address back to all 0xAA
10/10/2007  RH   Code cleanup, added function header/description
10/01/2007  DR   Added support for unknown SOC(non R3/R3BD/R4) 
                 (Synced with AMSS Btqsoc.c: #2111 Sep 2007)
10/01/2007  RH   Initial version
===============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "AEEstd.h" /* LIBSTD header file */
#include "bthci_qcomm.h"
#include "bthci_qcomm_pfal.h"
#include "btqsocnvmprivate.h"
#include "bthci_qcomm_linux.h"

/* Structure for storing BT QSOC related information. */
typedef struct
{
  bt_qsoc_enum_type bt_qsoc_type;
} bt_qsoc_struct_type;

/* BTS initialization states */
typedef enum
{
  BT_QSOC_INIT_STARTED = 0,
  BT_QSOC_DETECT_FW,
  BT_QSOC_POKE8_TBL_INIT,
  BT_QSOC_FIXED_TBL_INIT,
  BT_QSOC_DYN_TBL_INIT,
  BT_QSOC_NV_TBL_INIT,
  BT_QSOC_CUST_TBL_INIT,
  BT_QSOC_HCI_RESET,
  BT_QSOC_INIT_COMPLETED,
  BT_QSOC_SHUTDOWN
} bt_qsoc_init_st_enum_type;

/*===========================================================================
                                                            Globals
===========================================================================*/
bt_qsoc_init_st_enum_type bt_qsoc_init_state;

/* Structure to hold any BT QSOC specific information */
static bt_qsoc_struct_type       bt_qsoc;

/* Variables to identify the trasport used */
bt_hci_transport_device_type bt_hci_transport_device;

/* Default Bluetooth address if read from NV fails, same in AMSS */
static const uint8  default_bt_bd_addr[] =
{
    0x34, 0x12, 0x78, 0x56, 0xBC, 0x9A
};

char binary_filename[MAX_FILE_NAME];
/*****************************************************************************/
/*****************     S H U T D O W N   V S   C M D S     *******************/
/*****************************************************************************/
/* BTS's NVM configuration that needs to be performed to shut down required only
** on platforms where Power On/Off is not available (example 7500/7200 SURFS) 
*/
static bt_qsoc_cfg_element_type bt_qsoc_cfg_tag_17_baud_115200[] = 
{
    /* set running baud rate to 115.2K */
    0x08, 0x01, 0x11, 0x05,
          0x0A, 0x01, BT_QSOC_BAUD_115200, 0x00, 0x00
};
#define CFG_TAG_17_BAUD_115200  (bt_qsoc_cfg_element_type*)(&bt_qsoc_cfg_tag_17_baud_115200[0])

static boolean bt_hci_qcomm_vs_getappversion
(
void
);

static boolean bt_hci_qcomm_vs_gethwversion
(
void
);
static boolean bt_hci_qcomm_vs_poke
(
  void
);
/* Default hw version register contents for 4020BD B0, if it's different than it's 4020BD B1 */
static uint8 bt_soc_hw_version[] =
{
    0x05, 0x00, 0x00, 0x00
};

/* pointer to the SOC version string */
static uint8 *bt_soc_app_version_string = NULL;

/*********************************************************************************************************/

/*==========================================================================

  FUNCTION       bt_hci_qcomm_powerup

  DESCRIPTION    This routine invokes the platform specific routine that
                 powers up BTS.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   boolean: TRUE if powerup is successful

  SIDE EFFECTS   None.

==========================================================================*/
boolean bt_hci_qcomm_powerup
(
  void 
)
{
    return bt_hci_qcomm_pfal_powerup();
}

/*==========================================================================

  FUNCTION       bt_hci_qcomm_shutdown

  DESCRIPTION    This routine invokes the platform specific routine that
                 shuts down BTS.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
void bt_hci_qcomm_shutdown
(
  void 
)
{
    bt_hci_qcomm_pfal_shutdown();
}

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
)
{
	uint8 * nvm_ptr=NULL;
    boolean returnValue = TRUE;

	DEBUGMSG(ZONE_FUNCTION,TEXT("bt_hci_qcomm_send_shutdown_cmds"));

    if ( bt_qsoc_init_state == BT_QSOC_INIT_COMPLETED)
    {
        /* Send VS Command to switch over to 115200 */
		nvm_ptr = (uint8*)(CFG_TAG_17_BAUD_115200);
		if(nvm_ptr != NULL)
		{
			if ( (bt_hci_qcomm_pfal_vs_sendcmd(
					BT_QSOC_NVM_ACCESS_OPCODE, 
					(uint8 *)(&nvm_ptr[1]), 
					(uint8)(nvm_ptr[0]))
				  ) != TRUE ) 
			{
				DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_shutdown Failed NV VS Set Cmds"));
				returnValue = FALSE;
			}
		}

        /* TODO: As RIVA doesn't require HCI RESET to be sent after NVM
           configuration, this call needs to be skipped for RIVA.
           Currently this code is unreachable. */
        returnValue = bt_hci_qcomm_pfal_hcireset_baudratechange( BT_QSOC_INIT_BAUDRATE );

        (void)bt_hci_qcomm_sleep();

        bt_qsoc_init_state = BT_QSOC_SHUTDOWN;
    }
    return returnValue;
}

/*==========================================================================

  FUNCTION       bt_hci_qcomm_sleep

  DESCRIPTION    This routine invokes the platform specific routine that
                 allows the SOC to enter Deep sleep.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   boolean: Currently always TRUE

  SIDE EFFECTS   SIO pins will be altered.

==========================================================================*/
boolean bt_hci_qcomm_sleep
(
  void 
)
{
    boolean return_value = TRUE;
    return_value = bt_hci_qcomm_pfal_sleep();
    return return_value;
}

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
)
{
    boolean return_value = TRUE;
    return_value = bt_hci_qcomm_pfal_wake();
    return return_value;
}

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
)
{
    if ( nLength > HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind )
    {
        if ( ( pEventBuffer[ 0]  ==  0xFF)  /* VS Event */
             && (pEventBuffer[1] > 5)   /* VS Length > 5*/
             && (pEventBuffer[2] == BT_QSOC_EDL_CMD_CODE)   /* EDL VS Cmd 0x00 */
             && (pEventBuffer[3] == BT_QSOC_VS_EDL_APPVER_RESP) /* VS Application version Response 0x02 */
           )
        {
			if( NULL != bt_soc_app_version_string )
			{
				free(bt_soc_app_version_string);
				bt_soc_app_version_string = NULL;
			}
			bt_soc_app_version_string = (uint8 *)malloc(nLength-5);
			if( NULL != bt_soc_app_version_string )
			{
				std_memmove(bt_soc_app_version_string,&pEventBuffer[5],nLength-5);
			}
			snprintf(binary_filename, MAX_FILE_NAME, "%c%c%c%c%c%c_%c%c%c%c", pEventBuffer[17], pEventBuffer[18],
					pEventBuffer[19], pEventBuffer[20], pEventBuffer[21], pEventBuffer[22], pEventBuffer[31], pEventBuffer[32],
					pEventBuffer[33], pEventBuffer[34]);

			printf("Binary file: %s", binary_filename);
        }
        else if ( nLength > 12  // make sure we have enough event bytes
             && (pEventBuffer[0] ==  0xFF)  /* VS Event */
             && (pEventBuffer[1] == 0x0B)   /* VS Length = 0x0B */
             && (pEventBuffer[2] == BT_QSOC_EDL_CMD_CODE)   /* EDL VS Cmd 0x00 */
             && (pEventBuffer[3] == 0x01)
             && (pEventBuffer[4] == 0x34) /* 0x8C000034 hw version reg address */
             && (pEventBuffer[5] == 0x00) 
             && (pEventBuffer[6] == 0x00) 
             && (pEventBuffer[7] == 0x8C) 
             && (pEventBuffer[8] == 0x04) ) /* response len=0x04 */
        {
             // FF 0B 00 01 34 00 00 8C 04 05 00 00 00  0x00000005 is hw version
             bt_soc_hw_version[0] = pEventBuffer[9];
             bt_soc_hw_version[1] = pEventBuffer[10];
             bt_soc_hw_version[2] = pEventBuffer[11];
             bt_soc_hw_version[3] = pEventBuffer[12];
        }
    }
} /* bt_hci_qcomm_vs_event */

/*==========================================================================

  FUNCTION       bt_hci_qcomm_vs_getappversion

  DESCRIPTION    This routine sends the Vendor specific Application Get Version
                 HCI Command to the SOC.

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE is the sequence was sent successfully.

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
static boolean bt_hci_qcomm_vs_getappversion
(
  void 
)
{
    uint8 getStatusCmd[] = {0x06};
    if ( TRUE == bt_hci_qcomm_pfal_vs_sendcmd(
                                             BT_QSOC_EDL_CMD_OPCODE, 
                                             getStatusCmd, 
                                             1) )
    {
        DEBUGMSG(ZONE_FUNCTION,TEXT("bt_hci_qcomm_vs_getappversion : Succeeded"));
        return TRUE;
    }
    return FALSE;
} /* bt_hci_qcomm_vs_getappversion */

/*==========================================================================

  FUNCTION       bt_hci_qcomm_vs_gethwversion

  DESCRIPTION    This routine sends the Vendor specific Read Hardware Version Reg
                 HCI Command to the SOC.

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE is the sequence was sent successfully.

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
static boolean bt_hci_qcomm_vs_gethwversion
(
  void 
)
{
    uint8 getHWVerRegCmd[] = {0x0D, 0x34, 0x00, 0x00, 0x8C, 0x04 }; // 6 bytes, read 4 bytes at address 0x8C000034

    if ( TRUE == bt_hci_qcomm_pfal_vs_sendcmd(
                                             BT_QSOC_EDL_CMD_OPCODE,  // opcode=0xFC00
                                             getHWVerRegCmd, 
                                             6) )  
    {
        DEBUGMSG(ZONE_FUNCTION,TEXT("bt_hci_qcomm_vs_gethwversion : Succeeded"));
        return TRUE;
    }
    return FALSE;
} /* bt_hci_qcomm_vs_gethwversion */


/*==========================================================================

  FUNCTION       bt_hci_qcomm_vs_poke

  DESCRIPTION    This routine sends poke command to kick start NVM
                 initialisation.

  DEPENDENCIES   None.

  RETURN VALUE   boolean: TRUE is the sequence was sent successfully.

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
static boolean bt_hci_qcomm_vs_poke
(
  void
)
{
    //Poke32 0xA4000038 0x03030101
    uint8 pokeCmd[] = {0x0C, 0x08, 0x38, 0x00, 0x00, 0xA4, 0x01, 0x01, 0x03, 0x03};

   if ( TRUE == bt_hci_qcomm_pfal_vs_sendcmd(
                                           BT_QSOC_EDL_CMD_OPCODE,  // opcode=0xFC00
                                           pokeCmd,
                                           10) )
    {
        DEBUGMSG(ZONE_FUNCTION,TEXT("bt_hci_qcomm_vs_poke : Succeeded"));
        return TRUE;
    }
    return FALSE;
} /* bt_hci_qcomm_vs_poke */

/*==========================================================================

  FUNCTION       bt_hci_qcomm_download_nvm

  DESCRIPTION    This routine downloads BT SOC NVMs 

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   boolean: TRUE if the NVM download was successful.

  SIDE EFFECTS   BT SOC will receive downoaded NVMs

==========================================================================*/
static boolean bt_hci_qcomm_download_nvm()
{
	boolean returnStatus=TRUE;
	bt_qsoc_nvm_status nvm_status;
	uint8 * nvm_ptr=NULL;
	const int TAG_NUM_OFFSET = 2;
	const int TAG_LEN_OFFSET = 0;

	do
	{ 
		/* Get the next NVM "string" */
		nvm_status = bt_qsoc_nvm_get_next_cmd(&nvm_ptr);
		if( BT_QSOC_NVM_IS_STATUS_SUCCESS(nvm_status) ) 
		{ 
			if( nvm_ptr != NULL )	
			{	/* if status == success this check should never 
			       fail but we check anyway to be safe. */
				DEBUGMSG(ZONE_INFO,TEXT("bt_hci_qcomm_download_nvm - Sending NVM tag #%d (size %d)"), 
							(int)nvm_ptr[TAG_NUM_OFFSET], (int)nvm_ptr[TAG_LEN_OFFSET]);
				
				returnStatus = bt_hci_qcomm_pfal_vs_sendcmd( BT_QSOC_NVM_ACCESS_OPCODE, 
							  (uint8 *)(&nvm_ptr[1]), 
							  (uint8)(nvm_ptr[0]) );
				
				if (FALSE == returnStatus)    
				{
					DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_download_nvm - Failed to send dynamic VSCmd"));
					break;
				}
			}
			else
			{
				DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_download_nvm - NVM string is NULL!"));
				break;
			}
		}
	} while( BT_QSOC_NVM_IS_STATUS_SUCCESS(nvm_status) );

	if( BT_QSOC_NVM_IS_STATUS_COMPLETE(nvm_status) )
	{
		DEBUGMSG(ZONE_INFO,TEXT("bt_hci_qcomm_download_nvm - Finished sending NVM data."));
		returnStatus = TRUE;
	}
	else
	{
		DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_download_nvm - Aborted sending NVM data."));
		returnStatus = FALSE;
	}

    return returnStatus;
}

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
boolean bt_hci_qcomm_init(
	unsigned int 	uiInitBaudRate,
    unsigned int    uiRetryBaudRate
)
{
    uint8 * nvm_ptr=NULL;
    bt_qsoc_config_params_struct_type run_time_params;
    bt_qsoc_lookup_param soc_data;
    bt_qsoc_nvm_status nvm_status;
    uint32 loopCount=0;
    boolean returnStatus=TRUE;
    boolean bResult;

    DEBUGMSG(ZONE_FUNCTION,TEXT("bt_hci_qcomm_init : Started"));

    /* Power Off and then Power On the SOC to start fresh */
    bt_qsoc_init_state = BT_QSOC_DETECT_FW;
    bt_qsoc.bt_qsoc_type = BT_QSOC_NONE;
    std_memset(&run_time_params, 0x00, sizeof(run_time_params));

    DEBUGMSG(ZONE_INIT,TEXT("bt_hci_qcomm_init: Detecting BT SOC firmware at baud rate %d\r\n"), uiInitBaudRate );
    returnStatus = bt_hci_qcomm_vs_getappversion();

    if(BT_HCI_UART == bt_hci_transport_device.type)
    {  /* the following is enabled to retry next baud rate at 115.2Kbps or 3.2Mbps,
        uiInitBaudRate is current baud rate (tried and failed) comment this out to disable this feature */

      /* If for some reason, BT SOC is not at correct baud rate, bt_hci_qcomm_vs_getappversion() will fail
         So we try to reset SOC to different baud rate and try again */
      if (returnStatus == FALSE && uiRetryBaudRate != 0)
      {
        nvm_ptr = (uint8*)(CFG_TAG_17_BAUD_115200);

        /* COM port will be switched to new baud rate to send down command */
        bt_hci_qcomm_pfal_changebaudrate(uiRetryBaudRate); /* switch COM port to new baud rate */

        /* Send VS Command to switch over to 115200 */
        returnStatus = bt_hci_qcomm_pfal_vs_sendcmd (BT_QSOC_NVM_ACCESS_OPCODE, (uint8 *)(&nvm_ptr[1]), (uint8)(nvm_ptr[0]) );

        if ( returnStatus != FALSE )
        {
           /* Change to 115200 Baud rate and retry firmware detection */
           DEBUGMSG(ZONE_INIT,TEXT("bt_hci_qcomm_init: Retry Detecting BT SOC firmware at baud rate %d\r\n"), BT_QSOC_INIT_BAUDRATE );
           returnStatus = bt_hci_qcomm_pfal_hcireset_baudratechange( BT_QSOC_INIT_BAUDRATE );
        }

        if ( returnStatus != FALSE )
        {
          returnStatus = bt_hci_qcomm_vs_getappversion();  // try again at 115.2Kbps
        }
       }
    }

    if(returnStatus == TRUE && generate_override_mode == TRUE)	{
	/*In case of generate override mode
	Just get the app version to form the file name and
	stop further communication with the SoC*/
	return TRUE;
    }

    /* Read the hardware version */
    if(BT_HCI_UART == bt_hci_transport_device.type)
       bResult = bt_hci_qcomm_vs_gethwversion();


    /* Determine the SOC type */
    soc_data.app_ver_str = (char *)bt_soc_app_version_string;
    soc_data.hw_ver_str = (char *)bt_soc_hw_version;
    bt_qsoc.bt_qsoc_type = bt_qsoc_type_look_up(&soc_data);

    if (bt_qsoc.bt_qsoc_type == BT_QSOC_R2B)
    {
        DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_init Failed R2B Not supported"));
        returnStatus = FALSE;
    }
    else if (bt_qsoc.bt_qsoc_type == BT_QSOC_R2C)
    {
        DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_init Failed R2C Not supported"));
        returnStatus = FALSE;
    }
    else
    {
        DEBUGMSG(ZONE_FUNCTION,TEXT("\nbt_hci_qcomm_init : Found QSoC type %d.\n"), bt_qsoc.bt_qsoc_type);
    }

    /* After the Firmware is detected, start intializing the Poke table */
    if ( returnStatus != FALSE)
    {
        /* Patch: Pokes Only specific to R3 */
        bt_qsoc_init_state = BT_QSOC_POKE8_TBL_INIT;
        if ( bt_qsoc.bt_qsoc_type == BT_QSOC_R3 )
        {
            DEBUGMSG(ZONE_INFO,TEXT("bt_hci_qcomm_init  - Initialize R3 Poke table"));
            for ( loopCount = 0; loopCount <BT_QSOC_R3_POKETBL_COUNT; loopCount++ )
            {
                if ( (bt_hci_qcomm_pfal_vs_sendcmd(
                        BT_QSOC_EDL_CMD_OPCODE, 
                        (uint8 *)bt_qsoc_vs_poke8_tbl_r3[loopCount].vs_poke8_data,
                        bt_qsoc_vs_poke8_tbl_r3[loopCount].vs_poke8_data_len)
                      ) !=  TRUE)
                {
                    DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_init Failed Poke VS Set Cmds"));
                    returnStatus = FALSE;
                    break;
                }
            }
        }
        else if ((bt_qsoc.bt_qsoc_type == BT_QSOC_BHA_B0) ||
                 (bt_qsoc.bt_qsoc_type == BT_QSOC_BHA_B1))
        {
          DEBUGMSG(ZONE_INFO,TEXT("bt_hci_qcomm_init  - Poke Bahama B0 to start NVM download"));
          if ( bt_hci_qcomm_vs_poke() != TRUE)
          {
            DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_init Failed Poke Bahama B0 to start NVM download"));
            returnStatus = FALSE;
          }
        }
    }

    // default run-time parameters for SOC
    run_time_params.refclock_type = BT_SOC_REFCLOCK_32MHZ;
    run_time_params.clock_sharing =BT_SOC_CLOCK_SHARING_DISABLED;
    run_time_params.soc_logging = 0;
    run_time_params.bt_2_1_lisbon_disabled = 0;
    /* BR/EDR & LE SoC power class defaulted to CLASS1 and CLASS2
     * respectively
     */
    run_time_params.bt_qsoc_bredr_dev_class = BT_QSOC_DEV_CLASS1;
    run_time_params.bt_qsoc_le_dev_class = BT_QSOC_DEV_CLASS2;
    run_time_params.is_xo_support_enabled = 0;

    if (returnStatus != FALSE)
    {
        if (bt_hci_qcomm_pfal_get_bdaddress((uint8*)(&run_time_params.bd_address[0])) == TRUE)
        {
        }
        else
        {
            DEBUGMSG(ZONE_WARNING,TEXT("bt_hci_qcomm_init Failed to read BD Address from NV default to %02X:%02X:%02X:%02X:%02X:%02X"),
                (int)default_bt_bd_addr[0],(int)default_bt_bd_addr[1],(int)default_bt_bd_addr[2],
                (int)default_bt_bd_addr[3],(int)default_bt_bd_addr[4],(int)default_bt_bd_addr[5]  );

            // using default BT address
            std_memmove((uint8*)(&run_time_params.bd_address[0]), (const uint8 *)(&(default_bt_bd_addr[0])),BT_QSOC_MAX_BD_ADDRESS_SIZE);
        }
        run_time_params.bt_qsoc_bredr_dev_class = bt_hci_qcomm_pfal_get_bredr_dev_class_type();
        run_time_params.bt_qsoc_le_dev_class = bt_hci_qcomm_pfal_get_le_dev_class_type();

        if(BT_QSOC_RIVA != bt_qsoc.bt_qsoc_type)
        {
          if (TRUE == bt_hci_qcomm_pfal_is_refclock_19P2MHz())
          {
            run_time_params.refclock_type = BT_SOC_REFCLOCK_19P2MHZ;
          }
          if (TRUE == bt_hci_qcomm_pfal_is_clocksharing_enabled())
          {
            run_time_params.clock_sharing =BT_SOC_CLOCK_SHARING_ENABLED;
          }
          if (TRUE == bt_hci_qcomm_pfal_is_soclogging_enabled())
          {
            run_time_params.soc_logging = 1;  // enable BT SOC logging NVM  (added in latest btqsocnvm.c)
          }

          run_time_params.wlan_type = bt_hci_qcomm_pfal_get_wlan_type();
          run_time_params.is_xo_support_enabled = bt_hci_qcomm_pfal_is_xo_enabled();
        }

        if (FALSE == bt_hci_qcomm_pfal_is_lisbon_enabled()) {
            run_time_params.bt_2_1_lisbon_disabled = 1;  // If Lisbon is disabled on SOC, we'll make it a BT 2.0 controller
        }
        DEBUGMSG(ZONE_INIT,TEXT("bt_hci_qcomm_init - Clock=%sMhz ClockSharing=%s Logging=%s SOC=BT%s xo_support=%s"),
                                  run_time_params.refclock_type == BT_SOC_REFCLOCK_19P2MHZ ? TEXT("19.2") : TEXT("32"), 
                                  run_time_params.clock_sharing == BT_SOC_CLOCK_SHARING_ENABLED ? TEXT("Yes") : TEXT("No"), 
                                  run_time_params.soc_logging == 1 ? TEXT("Yes") : TEXT("No"), 
                                  run_time_params.bt_2_1_lisbon_disabled == 1 ? TEXT("2.0") : TEXT("2.1"),
                                  run_time_params.is_xo_support_enabled == 1 ? TEXT("Yes") : TEXT("No") );

		/* TODO - make the NVM mode configurable at runtime */
		nvm_status = bt_qsoc_nvm_open(bt_qsoc.bt_qsoc_type, nvm_mode, &run_time_params);

		if( BT_QSOC_NVM_IS_STATUS_SUCCESS(nvm_status)  )	  
		{
			DEBUGMSG(ZONE_INIT,TEXT("bt_hci_qcomm_init - Opened NVM module."));
		}
		else
		{
			DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_init - Failed to open NVM module.  status = %d"),nvm_status);
		}

		/* Send all the NVM data to the SOC */
		returnStatus = bt_hci_qcomm_download_nvm();

		/* Close NVM module (without regard to status of bt_hci_qcomm_download_nvm()) */
		nvm_status = bt_qsoc_nvm_close();
		if( FALSE == BT_QSOC_NVM_IS_STATUS_SUCCESS(nvm_status) )
		{
			returnStatus = FALSE;
			DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_init - bt_qsoc_nvm_close failed!"));
		}
    }

    /* After all the NVM VS Set commands, then send the HCI Reset command for those
    ** NV Parameters to be used. If required also change to the Application UART Baud rate
    */
    if ( returnStatus != FALSE)
    {
        DEBUGMSG(ZONE_INFO,TEXT("bt_hci_qcomm_init - NVM download completed, doing HCI Reset..."));

        if(BT_HCI_UART == bt_hci_transport_device.type)
        {
           bt_qsoc_init_state = BT_QSOC_HCI_RESET;
           returnStatus = bt_hci_qcomm_pfal_hcireset_baudratechange(BT_QSOC_APP_BAUDRATE);
           if ( returnStatus == FALSE)
           {
              returnStatus = bt_hci_qcomm_pfal_hcireset_baudratechange(BT_QSOC_APP_BAUDRATE);
           }
        }
	else if(BT_HCI_SMD == bt_hci_transport_device.type)
	{
	   bt_qsoc_init_state = BT_QSOC_HCI_RESET;
           returnStatus = bt_hci_qcomm_pfal_hcireset_smd();
           if ( returnStatus == FALSE)
           {
              returnStatus = bt_hci_qcomm_pfal_hcireset_smd();
           }
	}
    }

    if ( returnStatus != FALSE)
    {
        bt_qsoc_init_state = BT_QSOC_INIT_COMPLETED;
        DEBUGMSG(ZONE_INFO,TEXT("bt_hci_qcomm_init - Successful"));
    }
    else
    {
        DEBUGMSG(ZONE_ERROR,TEXT("bt_hci_qcomm_init - Failed"));
    }
    return returnStatus;
}

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
)
{
    boolean bInitialized = FALSE;

    if ( bt_qsoc_init_state == BT_QSOC_INIT_COMPLETED)
    {
			bInitialized = TRUE;
    }
    return (bInitialized);
}



#if defined( FEATURE_BT_WLAN_COEXISTENCE ) && \
    defined( FEATURE_BT_WLAN_COEX_CHAN_CLASS )
/* Numeric value of WLAN and BT channel frequency is not used by SW 
 * anywhere else .. so they are defined here for use by this function.
 * Refer : ANSI/IEEE Std 802.11 1999, Section 15.4.6.2 and
 *         Bluetooth Specification v2.0+EDR Vol 2, Part A, Section 2
 * Bluetooth channel spacing is 1 MHZ
 *         
*/
#define            WLAN_80211_RF_CH_1_MHZ       2412
#define            WLAN_80211_RF_CH_SPACING_MHZ 5
#define            BT_RF_CHANNEL_0_MHZ          2402    
uint16 bt_dc_afh_ch_exclude = 20 ;

/*==========================================================================

  FUNCTION       bt_wlan_coex_soc_calc_chan_class

  DESCRIPTION    Called to calculate the channel classification from the 
	               WLAN Channel number.

  DEPENDENCIES   chan_class_data_ptr has to point to 10 bytes.

  RETURN VALUE   None.

  SIDE EFFECTS   chan_class_data_ptr updated with channel classification..

==========================================================================*/
void bt_wlan_coex_soc_calc_chan_class
(
  uint8    wlan_chan_num,         /*Input */
  uint8    *chan_class_data_ptr   /*Input/Output */         
)
{
  uint16             wl_freq, bt_c_channel;
  uint8              i;

  /*-------------------------------------------------------------------------*/
  /* HCI Command:     Set_AFH_Host_Channel_Classification (7.3.50)           */
  /* Command Type:    BT_CMD_HC_SET_AFH_HOST_CHAN_CLASS                      */
  /* Command struct:  bt_cmd_hc_set_afh_host_chan_class_type                 */
  /* Return struct:   bt_hc_rp_status_type                                   */
  /*-------------------------------------------------------------------------*/
  /* bt_afh_ch_map_type.bytes[ 10 ]; ==> */
  /* MSB of byte 9 is not used and should be 0 */
  /* then one bit per channel 0..78            */
  /* 0 = bad, 1 = unknown                      */

  /*
   * Send Set_AFH_Host_Channel_Classification HCI cmd.
   * WiFi channel number is in wlan_chan_num.
   * TODO: What do we do about channel 14 (japan) - it is further away, 
   * so probably will casue very little interfernce
  */

	DEBUGMSG(ZONE_INFO,TEXT("BT Chan Class : chan %d data %x"), wlan_chan_num, chan_class_data_ptr);

  for ( i = 0; i < 10; i++ )
  {                        
    /* Initialize channel map with all channels set to "unknown" */ 
    /* 1==> channel indicated by bit position is unknown */
    chan_class_data_ptr[ i ] = 0xFF;
  }

  /* bit 79 of 80 bit mask is reserved and should be set to 0 */
  chan_class_data_ptr[ 9 ] = 0x7F; 

  if( wlan_chan_num > 13 )
  {
    /* Force channel classification to be reverted to default */
    wlan_chan_num = 0;
  }
    
  if( wlan_chan_num != 0 )
  {                
    /* Convert to freqeuncy */
    wl_freq = WLAN_80211_RF_CH_1_MHZ + 
              ( WLAN_80211_RF_CH_SPACING_MHZ * ( wlan_chan_num - 1 ) );

    /* Convert to channel */
    bt_c_channel = wl_freq - BT_RF_CHANNEL_0_MHZ;
    DEBUGMSG(ZONE_INFO,TEXT("BT Chan Class: WLAN ch %d Block freq %d, BT chan centre %d"), 
                  wlan_chan_num, wl_freq, bt_c_channel );
    DEBUGMSG(ZONE_INFO,TEXT("BT Chan Class %d channels each side of ch %d "), 
                  bt_dc_afh_ch_exclude, bt_c_channel, 0 );
        
    /* Fill in the bitmap */
    for( i=0; i<=78; i++ )
    {
      if( abs( i - bt_c_channel ) <= bt_dc_afh_ch_exclude )
      {
        /* BT_MSG_DEBUG("BT_DC: Marking channel %x bad", i, 0, 0 ); */
        /* 0 ==> channel indicated by bit position is bad */
        chan_class_data_ptr[ i/8 ] &= ~( 1 << ( i % 8 ) );
      }
      else
      {
        /* BT_MSG_DEBUG("BT_DC: Marking channel %x unknown", i, 0, 0 ); */
      }
    }            
  }
  else
  {
     DEBUGMSG(ZONE_INFO,TEXT("BT Chan Class: Revert default AFH chan class map.\r\n"));
  }
  
  DEBUGMSG(ZONE_INFO,TEXT("BT DC: New AFH map bytes: %.2X, %.2X %.2X"),
               chan_class_data_ptr[ 9 ], 
               chan_class_data_ptr[ 8 ], 
               chan_class_data_ptr[ 7 ] );
  DEBUGMSG(ZONE_INFO,TEXT("BT DC: New AFH map bytes: %.2X, %.2X %.2X"),
               chan_class_data_ptr[ 6 ], 
               chan_class_data_ptr[ 5 ], 
               chan_class_data_ptr[ 4 ] );
  DEBUGMSG(ZONE_INFO,TEXT("BT DC: New AFH map bytes: %.2X, %.2X %.2X"),
               chan_class_data_ptr[ 3 ], 
							 chan_class_data_ptr[ 2 ], 
							 chan_class_data_ptr[ 1 ]);
  DEBUGMSG(ZONE_INFO,TEXT("BT DC: New AFH map bytes: %.2X"),
               chan_class_data_ptr[ 0 ]);

}
#endif /* FEATURE_BT_WLAN_COEX_CHAN_CLASS && FEATURE_BT_WLAN_COEX_CHAN_CLASS*/
