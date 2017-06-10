/**
-----------------------------------------------------------------------------
Copyright (c) 2007-2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------------
 *
 * @file bthci_qcomm_linux.cpp
 * This file contains the Vendor Specific initializations
 * for Bluetooth QUALCOMM BTS402x Linux interfaces.
 *
 */
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //linux/pkgs/proprietary/bt/main/source/hci_qcomm_init/bthci_qcomm_linux.c#13 $
  $DateTime: 2009/01/29 20:43:14 $
  $Author: mfeldman $


  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2012-05-10  rk   Added runtime detection for WLAN type using property
  2012-02-29  rrr  Added/Modified API to get LE & BR/EDR SoC power class
  2011-09-28  rrr  Moved the implementation to CPP, for having BD address being
                   programmed twice if previous BD address was random generated.
  2011-08-17  an   HCI reset changes due to SMD transport
  2011-07-18  bn   Added support for 8960 for sending NVM tags to RIVA.
  2011-02-07  bhr  Adding -g and -o options for dumping the hci commands to a file.
  2010-04-08  jmf  New Architecture updates.
  2010-03-07  jmf  Enable Software In-Band Sleep if FEATURE on.
  2010-01-14  jmf  Don't clobber sleep protocol type when changing baud.
  2009-12-04  jmf  Add Marimba B0, lisbon disable option now defaults off.
  2009-10-10  jmf  Add/fix run-time option arguments for sleep, class2, debug.
  2009-05-19  jmf  Correct length to common HCI event parser.
  2009-05-07  jmf  Always disable Lisbon; don't get from NV.
  2009-01-07  jmf  Add getting ref clock, clock sharing from NV.
  2008-09-12  jmf  Power via bluetooth_power kernel mod; support WLAN coexistance.
  2008-08-08  jmf  Do select for all reads; pass in default arg vals from config.
  2008-07-10  jmf  Merge aswp401 change 699599: hci_qcomm_init hang fix, cleanup.
  2008-06-12  jmf  Correct BT power management plus misc cleanup.
  2008-03-13  jmf  Add new edit history format and P4 version control string.
  2007-09-06   jn  Adapted from AMSS/WM version.
===========================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <cutils/properties.h>

#include "comdef.h"
#include "bthci_qcomm.h"
#include "bthci_qcomm_pfal.h"
#include "bthci_qcomm_linux.h"
#include "bt_qmi_dms_client.h"

#ifdef BT_QSOC_GET_ITEMS_FROM_PROPERTY
#include <utils/Log.h>
#define BASE_16 16
#endif

#include "bt_nv.h"

/* Global for opened file descriptor. This is made global in order
 * to maintain same interfaces between different QC platforms
 */
int fd;
/* Run-time options for SoC initialization. */

#define BT_QSOC_REF_CLK_19_2MHZ 19200000
#define BT_QSOC_REF_CLK_32_0MHZ 32000000

#ifdef BT_QSOC_REF_CLOCK
unsigned long ref_clk_speed = BT_QSOC_REF_CLOCK;
#else
unsigned long ref_clk_speed = BT_QSOC_REF_CLK_32_0MHZ;
#endif /* BT_QSOC_REF_CLOCK */

boolean ref_clk_arg_entered = FALSE;

#ifdef BT_QSOC_ENABLE_CLOCK_SHARING
boolean clock_sharing_enabled = TRUE;
#else
boolean clock_sharing_enabled = FALSE;
#endif /* BT_QSOC_ENABLE_CLOCK_SHARING */

boolean clk_sharing_arg_entered = FALSE;

unsigned long starting_baud = BT_QSOC_INIT_BAUDRATE;

#ifdef BT_QSOC_HCI_BAUD_RATE
unsigned long ending_baud = BT_QSOC_HCI_BAUD_RATE; /* differs based on legacy vs HS uart */
#else
unsigned long ending_baud = BT_QSOC_APP_BAUDRATE;  /* differs based on legacy vs HS uart */
#endif /* BT_QSOC_HCI_BAUD_RATE */

#ifdef BT_QSOC_DISABLE_SLEEP_MODE
boolean sleep_mode = FALSE;
#else
boolean sleep_mode = TRUE;
#endif /* BT_QSOC_DISABLE_SLEEP_MODE */

boolean sleep_mode_arg_entered = FALSE;

boolean soc_logging_enabled = FALSE;
boolean soc_logging_arg_entered = FALSE;

boolean disable_lisbon_arg_entered = FALSE;
boolean disable_lisbon = FALSE;  /* Disable Bluetooth 2.1 features in SoC */

bt_qsoc_device_class_type bredr_pwr_class = BT_QSOC_DEV_CLASS1;
bt_qsoc_device_class_type le_pwr_class    = BT_QSOC_DEV_CLASS2;

boolean dont_reset = FALSE;
boolean just_reset = FALSE;

boolean force_hw_sleep = FALSE;

boolean no_soc_download_mode = FALSE;
boolean generate_override_mode = FALSE;
int binary_fd = -1;
char *path_to_binary = NULL;
char *path_to_override = NULL;

#ifdef BT_QSOC_GET_ITEMS_FROM_PROPERTY
#define BLUETOOTH_MAC_ADDR_BOOT_PROPERTY "ro.boot.btmacaddr"
bool validate_tok(char* bdaddr_tok) {
    int i = 0;
    bool ret;
    int len = strlen(bdaddr_tok);
    if (len > 2) {
        ret = FALSE;
        ALOGE("Invalid token length");
    } else {
        ret = TRUE;
        for (i = 0; i < len; i++) {
            if ((bdaddr_tok[i] >= '0' && bdaddr_tok[i] <= '9') ||
                (bdaddr_tok[i] >= 'A' && bdaddr_tok[i] <= 'F') ||
                (bdaddr_tok[i] >= 'a' && bdaddr_tok[i] <= 'f')) {
                ret = TRUE;
                ALOGV("%s: tok %s @ %d is good", __func__, bdaddr_tok, i);
            } else {
                ret = FALSE;
                ALOGE("invalid character in tok: %s at ind: %d", bdaddr_tok, i);
                break;
            }
        }
    }
    return ret;
}
#endif


/* Variables to identify the platform */
bt_hci_transport_device_type bt_hci_transport_device;
#ifdef BT_QSOC_HCI_DEVICE
const char *device_name = BT_QSOC_HCI_DEVICE;
#else
char *device_name = "/dev/ttyHS0";
#endif

char *BD_addr_string = NULL;
/* BD_ADDR is little_endian reverse of network byte order */
uint8 BD_addr[] = { 0x3A, 0x1D, 0xFE, 0xC6, 0xA0, 0x00 }; /* IEEE Standard OUI for Qualcomm */

char *firmware_file_name = NULL;
bt_qsoc_enum_nvm_mode nvm_mode = NVM_AUTO_MODE; /* default if no firmware file argument */

int verbose = 0;

bt_qsoc_device_class_type bt_hci_qcomm_pfal_get_bredr_dev_class_type( void )
{
  return bredr_pwr_class;

} /* bt_hci_qcomm_pfal_get_bredr_dev_class_type */

bt_qsoc_device_class_type bt_hci_qcomm_pfal_get_le_dev_class_type( void )
{
  return le_pwr_class;

} /* bt_hci_qcomm_pfal_get_le_dev_class_type */

boolean bt_hci_qcomm_pfal_is_soclogging_enabled (void)
{
    /* Do not use NV_BT_SOC_LOGGING_ENABLED_I until libnv.so supports. */
    return soc_logging_enabled;
}

boolean bt_hci_qcomm_pfal_is_lisbon_enabled( void )
{
    /* Do not use NV_BT_LISBON_DISABLED_I until libnv.so supports. */
    /* Enable Lisbon (BT 2.1) since Android eclair and both BlueZ 4.47 and BM3 can do mandatory 2.1 features. */
    return disable_lisbon? FALSE: TRUE;
}

boolean bt_hci_qcomm_pfal_is_refclock_19P2MHz
(
void
)
{
#if defined(BT_QSOC_GET_ITEMS_FROM_NV)
    if((TRUE == is_modem_bt_nv_supported(MODEM_NV_BT_SOC_REFCLOCK_TYPE)) &&
       (TRUE == qmi_dms_get_refclock(&ref_clk_speed)))
    {
        fprintf (stderr, "Read MODEM_NV_BT_SOC_REFCLOCK_TYPE from modem NV\n");
    }
    else
    {
        /* get NV items from persist */
        if (FALSE == ref_clk_arg_entered)
        {
            static nv_persist_item_type my_nv_item;
            static nv_persist_stat_enum_type cmd_result;

            cmd_result = (nv_persist_stat_enum_type)bt_nv_cmd(NV_READ_F,  NV_BT_SOC_REFCLOCK_TYPE_I, &my_nv_item);

            if (NV_SUCCESS  != cmd_result)
            {
                if (verbose > 0)
                {
                    fprintf (stderr, "bt_nv_cmd failed to get NV_BT_SOC_REFCLOCK_TYPE_I from NV, code %d\n", cmd_result);
                }
            }
            else
            {
                ref_clk_speed = ( *((nv_ps_bt_soc_refclock_enum_type *) &my_nv_item.bt_soc_refclock_type) ==
                                 NV_PS_BT_SOC_REFCLOCK_19P2MHZ)? \
                                BT_QSOC_REF_CLK_19_2MHZ: BT_QSOC_REF_CLK_32_0MHZ;
                if (verbose > 0)
                {
                    fprintf (stderr, "bt_nv_cmd got NV_BT_SOC_REFCLOCK_TYPE_I from NV: 0x%x\n",
                            (unsigned int) my_nv_item.bt_soc_refclock_type);
                }
            }
        }
    }
#elif defined(BT_QSOC_GET_ITEMS_FROM_PERSIST)
  if (FALSE == ref_clk_arg_entered)
  {
     static nv_persist_item_type my_nv_item;
     static nv_persist_stat_enum_type cmd_result;

     cmd_result = (nv_persist_stat_enum_type)bt_nv_cmd(NV_READ_F,  NV_BT_SOC_REFCLOCK_TYPE_I, &my_nv_item);

     if (NV_SUCCESS  != cmd_result)
     {
          if (verbose > 0)
          {
              fprintf (stderr, "bt_nv_cmd failed to get NV_BT_SOC_REFCLOCK_TYPE_I from NV, code %d\n", cmd_result);
          }
     }
     else
     {
          ref_clk_speed = ( *((nv_ps_bt_soc_refclock_enum_type *) &my_nv_item.bt_soc_refclock_type) == NV_PS_BT_SOC_REFCLOCK_19P2MHZ)? \
          BT_QSOC_REF_CLK_19_2MHZ: BT_QSOC_REF_CLK_32_0MHZ;
          if (verbose > 0)
          {
              fprintf (stderr, "bt_nv_cmd got NV_BT_SOC_REFCLOCK_TYPE_I from NV: 0x%x\n",
                  (unsigned int) my_nv_item.bt_soc_refclock_type);
          }
     }
  }

#endif /* defined(BT_QSOC_GET_ITEMS_FROM_NV/PERSIST) */
    if(BT_QSOC_REF_CLK_19_2MHZ == ref_clk_speed)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

boolean bt_hci_qcomm_pfal_is_clocksharing_enabled
(
void
)
{
#if defined(BT_QSOC_GET_ITEMS_FROM_NV)
    if((TRUE == is_modem_bt_nv_supported(MODEM_NV_BT_SOC_CLK_SHARING_TYPE)) &&
       (TRUE == qmi_dms_get_clk_sharing(&clock_sharing_enabled)))
    {
        fprintf (stderr, "Read MODEM_NV_BT_SOC_CLK_SHARING_TYPE from modem NV\n");
    }
    else
    {
        /* get NV items from persist */
        static nv_persist_item_type my_nv_item; /* nv_ps_bt_soc_refclock_enum_type not in nv.h ... */
        static nv_persist_stat_enum_type cmd_result;

        cmd_result = (nv_persist_stat_enum_type)bt_nv_cmd(NV_READ_F,  NV_BT_SOC_CLK_SHARING_TYPE_I, &my_nv_item);

        if (NV_SUCCESS  != cmd_result)
        {
            if (verbose > 0)
            {
                fprintf (stderr, "bt_nv_cmd failed to get NV_BT_SOC_CLK_SHARING_TYPE_I from NV, code %d\n", cmd_result);
            }
        }
        else
        {
            clock_sharing_enabled =
                ( *((nv_ps_bt_soc_clock_sharing_enum_type *) &my_nv_item.bt_soc_clk_sharing_type) ==
                                NV_PS_BT_SOC_CLOCK_SHARING_ENABLED);
            if (verbose > 0)
            {
                fprintf (stderr, "bt_nv_cmd got NV_BT_SOC_CLK_SHARING_TYPE_I from NV: 0x%x\n",
                        (unsigned int) my_nv_item.bt_soc_clk_sharing_type);
            }
        }

    }
#elif defined (BT_QSOC_GET_ITEMS_FROM_PERSIST)
    static nv_persist_item_type my_nv_item; /* nv_ps_bt_soc_refclock_enum_type not in nv.h ... */
    static nv_persist_stat_enum_type cmd_result;

    cmd_result = (nv_persist_stat_enum_type)bt_nv_cmd(NV_READ_F,  NV_BT_SOC_CLK_SHARING_TYPE_I, &my_nv_item);

    if (NV_SUCCESS  != cmd_result)
    {
        if (verbose > 0)
        {
            fprintf (stderr, "bt_nv_cmd failed to get NV_BT_SOC_CLK_SHARING_TYPE_I from NV, code %d\n", cmd_result);
        }
    }
    else
    {
        clock_sharing_enabled =
           ( *((nv_ps_bt_soc_clock_sharing_enum_type *) &my_nv_item.bt_soc_clk_sharing_type) == NV_PS_BT_SOC_CLOCK_SHARING_ENABLED);
        if (verbose > 0)
        {
           fprintf (stderr, "bt_nv_cmd got NV_BT_SOC_CLK_SHARING_TYPE_I from NV: 0x%x\n",
                             (unsigned int) my_nv_item.bt_soc_clk_sharing_type);
        }
    }

#endif /* defined(BT_QSOC_GET_ITEMS_FROM_NV) */
    return clock_sharing_enabled;
}


/* JN: Make sure code formatting is correct */
static boolean bt_hci_qcomm_pfal_sendresetcmd ( void )
{
  boolean status = FALSE;
  uint8   bthResetCmd[RESET_CMD_SIZE_UART] = {0x01, 0x03, 0x0C, 0x00};

  /* The protocol byte is not required if SMD is used as transport*/
  if (bt_hci_transport_device.type == BT_HCI_SMD)
	status = (bt_hci_qcomm_nwrite((uint8*)(&bthResetCmd[1]), (int)RESET_CMD_SIZE_SMD) == RESET_CMD_SIZE_SMD)? TRUE: FALSE;
  else
	status = (bt_hci_qcomm_nwrite((uint8*)(&bthResetCmd[0]), (int)RESET_CMD_SIZE_UART) == RESET_CMD_SIZE_UART)? TRUE: FALSE;

  if (FALSE == status)
  {
    BTHCI_QCOMM_ERROR(("bt_hci_qcomm_SendResetCmd: Error->Send Cmd failed\n"));
    return status;
  }

  return status;
}

#if defined(BT_QSOC_POWER_ONOFF_PATH)
static char bt_qsoc_power_off_flag = '0';
static char bt_qsoc_power_on_flag  = '1';
static char *bt_qsoc_power_path = BT_QSOC_POWER_ONOFF_PATH;
static int power_fd;
#endif /* defined(BT_QSOC_POWER_ONOFF_PATH) */

boolean bt_hci_qcomm_pfal_powerup ( void )
{
  boolean ret_value = TRUE;

#if defined(BT_QSOC_POWER_ONOFF_PATH)
  struct timespec sleeptime;

  power_fd = open(bt_qsoc_power_path, (O_RDWR | O_NOCTTY));  /* may want to read before write */

  if (-1 == power_fd)
  {
    BTHCI_QCOMM_ERROR("bt_hci_qcomm_pfal_powerup: Cannot open %s: %s\n", bt_qsoc_power_path, strerror(errno));
  }
  else if (sizeof (bt_qsoc_power_on_flag) != write (power_fd, &bt_qsoc_power_on_flag, sizeof (bt_qsoc_power_on_flag)))
  {
      fprintf (stderr, "bt_hci_qcomm_pfal_powerup: failed to power up BT SoC:%s)\n", strerror(errno));
  }
  else if (verbose > 1)
  {
      fprintf (stderr, "bt_hci_qcomm_pfal_powerup: succeeded to power up BT QSoC\n");
  }

  sleeptime.tv_sec = 0;
  sleeptime.tv_nsec = 10*1000*1000; /* 10 milliseconds to let VREG stabilize */
  if (nanosleep (&sleeptime, NULL) != 0)
  {
    perror ("bt_hci_qcomm_pfal_powerup: nanosleep:");
  }

#else /* do not try power on/off - not on PC */
  if (verbose > 1)
  {
      fprintf (stderr, "bt_hci_qcomm_pfal_powerup: not configured to power up BT QSoC\n");
  }
#endif /* do power on/off - not on PC */

  return ret_value;
}


boolean bt_hci_qcomm_pfal_sleep ( void )
{
  boolean ret_value = TRUE;
  DEBUGMSG(2, "bt_hci_qcomm_pfal_sleep : DOING NOTHING.\n");
  return ret_value;
}


boolean bt_hci_qcomm_pfal_wake ( void )
{
  boolean ret_value = TRUE;
  DEBUGMSG(2, "bt_hci_qcomm_pfal_wake : DOING NOTHING.\n");
  return ret_value;
}


void bt_hci_qcomm_pfal_shutdown ( void )
{
  (void) tcflush(fd, TCIOFLUSH); /* so UART doesn't hang when we close */
#if defined(BT_QSOC_POWER_ONOFF_PATH)

  power_fd = open(bt_qsoc_power_path, (O_RDWR | O_NOCTTY));  /* may want to read before write */

  if (-1 == power_fd)
  {
    BTHCI_QCOMM_ERROR("bt_hci_qcomm_pfal_powerup: Cannot open %s: %s\n", bt_qsoc_power_path, strerror(errno));
  }
  else if (sizeof (bt_qsoc_power_off_flag) != write (power_fd, &bt_qsoc_power_off_flag, sizeof (bt_qsoc_power_off_flag)))
  {
      fprintf (stderr, "bt_hci_qcomm_pfal_powerup: failed to power up BT SoC:%s)\n", strerror(errno));
  }
  else if (verbose > 1)
  {
      fprintf (stderr, "bt_hci_qcomm_pfal_powerup: succeeded to power up BT QSoC\n");
  }
#else /* do not try power on/off - not on PC */
  if (verbose > 1)
  {
      fprintf (stderr, "bt_hci_qcomm_pfal_powerup: BT QSoC power control configured to be outside of hci_qcomm_init.\n");
  }
#endif /* not do power on/off -- on PC */
}


boolean bt_hci_qcomm_pfal_get_bdaddress ( uint8 *pCmdBuffer )
{
  boolean bd_set = FALSE;

  if (verbose > 2)
  {
    fprintf (stderr, "get_bdaddr: before: 0x%02x.0x%02x.0x%02x.0x%02x.0x%02x.0x%02x\n",
             BD_addr[0], BD_addr[1], BD_addr[2], BD_addr[3], BD_addr[4], BD_addr[5]);
  }

  if (BD_addr_string != NULL) /* BD_ADDR was passed in as -b arg */
  {
    /* convert ASCII HEX representation to 48 bits */
    /* BD_addr is little-endian, but string is as printed */
    BD_addr[5] = strtol (&BD_addr_string[ 0], 0, 16);
    BD_addr[4] = strtol (&BD_addr_string[ 3], 0, 16);
    BD_addr[3] = strtol (&BD_addr_string[ 6], 0, 16);
    BD_addr[2] = strtol (&BD_addr_string[ 9], 0, 16);
    BD_addr[1] = strtol (&BD_addr_string[12], 0, 16);
    BD_addr[0] = strtol (&BD_addr_string[15], 0, 16);
    bd_set = TRUE;

  }
  else /* get BD_ADDR from NV or set it to RANDOM */
  {
#if defined(BT_QSOC_GET_ITEMS_FROM_NV)
    if((TRUE == is_modem_bt_nv_supported(MODEM_NV_BD_ADDR)) &&
       (TRUE == qmi_dms_get_bt_address(BD_addr)))
    {
        bd_set = TRUE;
        fprintf (stderr, "Read MODEM_NV_BD_ADDR from modem NV\n");
    }
    else
    {
        /* get NV items from persist */
        static nv_persist_item_type my_nv_item; /* nv_bd_addr_type */
        static nv_persist_stat_enum_type cmd_result;
        cmd_result = (nv_persist_stat_enum_type)bt_nv_cmd(NV_READ_F,  NV_BD_ADDR_I, &my_nv_item);

        if (NV_SUCCESS  != cmd_result)
        {
            if (verbose > 0)
            {
                fprintf (stderr, "bt_nv_cmd failed to get BD_ADDR from NV, code %d\n", cmd_result);
            }
        }
        else
        {
            /* copy bytes */
            BD_addr[5] = my_nv_item.bd_addr[5];
            BD_addr[4] = my_nv_item.bd_addr[4];
            BD_addr[3] = my_nv_item.bd_addr[3];
            BD_addr[2] = my_nv_item.bd_addr[2];
            BD_addr[1] = my_nv_item.bd_addr[1];
            BD_addr[0] = my_nv_item.bd_addr[0];
            bd_set = TRUE;
        }
    }
#elif defined(BT_QSOC_GET_ITEMS_FROM_PERSIST)

    static nv_persist_item_type my_nv_item; /* nv_bd_addr_type */
    static nv_persist_stat_enum_type cmd_result;
    cmd_result = (nv_persist_stat_enum_type)bt_nv_cmd(NV_READ_F,  NV_BD_ADDR_I, &my_nv_item);

    if (NV_SUCCESS  != cmd_result)
    {
        if (verbose > 0)
        {
            fprintf (stderr, "bt_nv_cmd failed to get BD_ADDR from NV, code %d\n", cmd_result);
        }
    }
    else
    {
        /* copy bytes */
        BD_addr[5] = my_nv_item.bd_addr[5];
        BD_addr[4] = my_nv_item.bd_addr[4];
        BD_addr[3] = my_nv_item.bd_addr[3];
        BD_addr[2] = my_nv_item.bd_addr[2];
        BD_addr[1] = my_nv_item.bd_addr[1];
        BD_addr[0] = my_nv_item.bd_addr[0];
        bd_set = TRUE;
    }

#elif defined(BT_QSOC_GET_ITEMS_FROM_PROPERTY)

    int i = 0;
    static char bd_addr[PROPERTY_VALUE_MAX];
    char* tok;
    static nv_persist_item_type my_nv_item; /* nv_bd_addr_type */
    static nv_persist_stat_enum_type cmd_result;

    if (property_get(BLUETOOTH_MAC_ADDR_BOOT_PROPERTY, bd_addr, NULL)) {
        ALOGE("BD address read from Boot property: %s\n", bd_addr);
        tok =  strtok(bd_addr, ":");
        while (tok != NULL) {
            ALOGV("bd add [%d]: %d ", i, strtol(tok, NULL, BASE_16));
            if (i >= 6) {
                ALOGE("bd property of invalid length");
                bd_set = FALSE;
                break;
            }
            if (!validate_tok(tok)) {
                ALOGE("Invalid token in BD address");
                bd_set = FALSE;
                break;
            }
            BD_addr[5-i] = strtol(tok, NULL, BASE_16);
            tok = strtok(NULL, ":");
            bd_set = TRUE;
            i++;
        }
        if (bd_set && (i < 6)) {
            ALOGE("bd property length is less than 6 bytes");
            bd_set = FALSE;
        }
    }

    if(!bd_set)
    {
        ALOGE("Error with BD address property, falling back to read from persist \n");

        cmd_result = (nv_persist_stat_enum_type)bt_nv_cmd(NV_READ_F,  NV_BD_ADDR_I, &my_nv_item);

        if (NV_SUCCESS  != cmd_result)
        {
            if (verbose > 0)
            {
                fprintf (stderr, "bt_nv_cmd failed to get BD_ADDR from NV, code %d\n", cmd_result);
            }
        }
        else
        {
            /* copy bytes */
            BD_addr[5] = my_nv_item.bd_addr[5];
            BD_addr[4] = my_nv_item.bd_addr[4];
            BD_addr[3] = my_nv_item.bd_addr[3];
            BD_addr[2] = my_nv_item.bd_addr[2];
            BD_addr[1] = my_nv_item.bd_addr[1];
            BD_addr[0] = my_nv_item.bd_addr[0];
            bd_set = TRUE;
        }
    }
#else /* not BT_QSOC_GET_ITEMS_FROM_NV - running on PC not MSM */

    /* then set the non-manufacturer part to random */
    BTHCI_QCOMM_TRACE ("bt_hci_qcomm_pfal_get_bdaddress: BD_ADDR not avialable from NV.\n");

#endif /* not on PC */
  }

  if (!bd_set)
  {
   int seed;
   struct timespec sTime;

   if (-1 == clock_gettime (CLOCK_REALTIME, &sTime))
   {
	perror ("clock_gettime\n" );
	return (FALSE);
   }

   seed = sTime.tv_nsec;
   srand ((unsigned int) seed);

   BD_addr[0] = (rand() & 0x0FF00000) >> 20;
   BD_addr[1] = (rand() & 0x0FF00000) >> 20;
   BD_addr[2] = (rand() & 0x0FF00000) >> 20;
  }

  memcpy( pCmdBuffer, BD_addr, sizeof(BD_addr) );

  if (verbose > 2)
  {
    fprintf (stderr, "get_bdaddr: after: %02x.%02x.%02x.%02x.%02x.%02x\n",
             BD_addr[0], BD_addr[1], BD_addr[2], BD_addr[3], BD_addr[4], BD_addr[5]);
  }
  return TRUE; /* for now -- need error checking? */
}

bt_qsoc_enum_wlan_type bt_hci_qcomm_pfal_get_wlan_type( void )
{
  static char ath6kl_supported[PROPERTY_VALUE_MAX];
  bt_qsoc_enum_wlan_type rval = BT_QSOC_WLAN_DEFAULT;

  property_get("wlan.driver.ath", ath6kl_supported, 0);

  if(*ath6kl_supported == '1')
  {
    rval = BT_QSOC_WLAN_ATHEROS;
  }
  else
  {
    rval = BT_QSOC_WLAN_LIBRA;
  }

  fprintf(stderr, "bt_hci_qcomm_pfal_get_wlan_type: %d\n", rval);
  fflush (stderr);

  return rval;
} /* bt_hci_qcomm_pfal_get_wlan_type */

uint8 bt_hci_qcomm_pfal_is_xo_enabled( void )
{
  uint8 rval = 0; //Default XO disabled

  /* no nv support */
#ifdef FEATURE_BT_SYSTEM_CLOCK_XO_SUPPORT
    rval = 1;
#endif

  return rval;
} /* bt_hci_qcomm_pfal_is_xo_enabled */



boolean bt_hci_qcomm_get_event (uint8 *pEventBuf)
{
  boolean status = FALSE;
  boolean long_event = FALSE;
  int     rx_bytes = 0;
  int event_bytes = 0;
  int i;

  /* Get the first three bytes (protocol id, event code and len of event */
  /* JMF: this assumes that only HCI-UART events can and do occur. */
  /* JMF: with FEATURE_BT_QSOC_INBAND_SLEEP, we can get 1 byte IBS messages. */
  if ((HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind) >= 0)
    rx_bytes = bt_hci_qcomm_nread(pEventBuf, HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind);

  if (rx_bytes != HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind)
  {
    fprintf(stderr, "bt_hci_qcomm_get_event: VERY SHORT READ!\n");
    fflush (stderr);
    return status;
  }

  /* else get rest of the packet */
  if ((HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind - 1) >= 0)
    event_bytes = pEventBuf[HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind - 1];

  if (HC_VS_MAX_CMD_EVENT < event_bytes)
  {
    fprintf(stderr, "bt_hci_qcomm_get_event: LONG EVENT!\n");
    fflush (stderr);
    long_event = TRUE;
    event_bytes = HC_VS_MAX_CMD_EVENT;
  }
  if ((HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind) >= 0)
    rx_bytes = bt_hci_qcomm_nread(&(pEventBuf[HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind]), event_bytes);

  if (rx_bytes == event_bytes)
  {
    status = TRUE;
  }
  else
  {
    fprintf(stderr, "bt_hci_qcomm_get_event: SHORT READ!\n");
    fflush (stderr);
  }

  if (verbose > 2)
  {
    event_bytes += HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind; /* to account for HCI header */
    fprintf(stderr, "EVT:");
    for (i = 0; i < event_bytes; i++)
    {
      fprintf(stderr, " %02X", pEventBuf[i]);
    }

    fprintf(stderr, "%s", long_event? " ...\n": "\n");
    fflush (stderr);
  }

  return status;
}

/*
* Send the reset command when using SMD as the transport
*/

boolean bt_hci_qcomm_pfal_hcireset_smd ( void )
{
  boolean status = TRUE;

  status = bt_hci_qcomm_pfal_sendresetcmd();


  /* Get the Command Complete for the HCI Reset at the new Baud rate */
  if(status)
  {
    uint8 pEventBuffer[HC_VS_MAX_CMD_EVENT];

    /* Get COMMAND COMPLETE event */
    if (TRUE != bt_hci_qcomm_get_event(pEventBuffer))
    {
	BTHCI_QCOMM_ERROR("Failed to receive cmd cmplt event \n");
	status = FALSE;
    }
  }

  return status;
}

/*
* Change baud rate of both BTS chip and local UART.
*/

boolean bt_hci_qcomm_pfal_hcireset_baudratechange ( uint32 baud_rate )
{
  boolean status = TRUE;
  int     flags;

  if (sleep_mode_arg_entered) /* first case of tag override */
  {
    /* assume that sleep_mode_arg_entered means disable sleep in SOC */
    static bt_qsoc_cfg_tbl_struct_type bt_qsoc_tag27 =
    {
	0x04,   {0x01, 0x1B, 0x01, 0x00}
    };

    if ( (bt_hci_qcomm_pfal_vs_sendcmd (
		BT_QSOC_NVM_ACCESS_OPCODE,
		(uint8 *)(bt_qsoc_tag27.vs_cmd_data),
		bt_qsoc_tag27.vs_cmd_len )
	      ) != TRUE )
    {
       fprintf (stderr, "bthci_qcomm_hcireset_baudratechange Failed NV VS Set Cmd to disable sleep mode");
       return (FALSE);  /* only on error */
    }
  }

  /*
   * Debuging option to allow hci_qcomm_init to be run in steps.
   */
  if (dont_reset)
  {
    return status;
  }

  /*
  * bt_hci_qcomm_init() in bthci_qcomm_common.c has a compiled-in
  * ending baud rate that cannot be changed at runtime, but it does
  * call us with the value.  Here we ignore the default ending baud.
  * We can overwrite the SOC's NVM baud entry.
  * Defines adapted from AMSS //depot/asic/msmshared/services/bt/soc/btqsoc.c:
  */
#define  BT_QSOC_BAUD_115200      0x0
#define  BT_QSOC_BAUD_57600       0x1
#define  BT_QSOC_BAUD_38400       0x2
#define  BT_QSOC_BAUD_19200       0x3
#define  BT_QSOC_BAUD_9600        0x4
#define  BT_QSOC_BAUD_230400      0x5
#define  BT_QSOC_BAUD_250000      0x6
#define  BT_QSOC_BAUD_460800      0x7
#define  BT_QSOC_BAUD_500000      0x8
#define  BT_QSOC_BAUD_720000      0x9
#define  BT_QSOC_BAUD_921600      0xA
#define  BT_QSOC_BAUD_1000000     0xB
#define  BT_QSOC_BAUD_125000      0xC
#define  BT_QSOC_BAUD_2000000     0xD
#define  BT_QSOC_BAUD_3000000     0xE
#define  BT_QSOC_BAUD_4000000     0xF
#define  BT_QSOC_BAUD_1600000     0x10
#define  BT_QSOC_BAUD_3200000     0x11
#define  BT_QSOC_BAUD_3500000     0x12
#define  BT_QSOC_BAUD_Auto        0xFE
#define  BT_QSOC_BAUD_UserDefined 0xFF

#define BT_QSOC_TAG_17_CMD_SZ_IBS 0x08
#define BT_QSOC_TAG_17_TAG_SZ_IBS 0x05
#define BT_QSOC_SLEEP_TYPE_IBS 0x4A
#define BT_QSOC_TAG_17_RX_SCO_BUFS_IBS 0x00
#define BT_QSOC_TAG_17_TX_SCO_BUFS_IBS 0x00

#define BT_QSOC_TAG_17_CMD_SZ_SWIBS 0x0B
#define BT_QSOC_TAG_17_TAG_SZ_SWIBS 0x08
#define BT_QSOC_SLEEP_TYPE_SWIBS 0x8A
#define BT_QSOC_TAG_17_RX_SCO_BUFS_SWIBS 0x00
#define BT_QSOC_TAG_17_TX_SCO_BUFS_SWIBS 0x00
#define BT_QSOC_TAG_17_RETRANS_SWIBS 0x04
#define BT_QSOC_TAG_17_IDLE_TO_SWIBS 0x0A
#define BT_QSOC_TAG_17_WAKEREQ_SWIBS 0x01  /* 0x00 to disable glitch avoidance */

#define BT_QSOC_TAG_17_CMD_SZ_HWS 0x08
#define BT_QSOC_TAG_17_TAG_SZ_HWS 0x05
#define BT_QSOC_SLEEP_TYPE_HWS 0x0A
#define BT_QSOC_TAG_17_RX_SCO_BUFS_HWS 0x00
#define BT_QSOC_TAG_17_TX_SCO_BUFS_HWS 0x00


#if defined(FEATURE_BT_QSOC_INBAND_SLEEP)
  static bt_qsoc_cfg_tbl_struct_type bt_qsoc_tag17_ibs =
  {
      BT_QSOC_TAG_17_CMD_SZ_IBS,   {0x01, 0x11, BT_QSOC_TAG_17_TAG_SZ_IBS,
           BT_QSOC_SLEEP_TYPE_IBS, 0x01, BT_QSOC_BAUD_115200,
           BT_QSOC_TAG_17_RX_SCO_BUFS_IBS, BT_QSOC_TAG_17_TX_SCO_BUFS_IBS
      }
  };
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
  static bt_qsoc_cfg_tbl_struct_type bt_qsoc_tag17_swibs =
  {
      BT_QSOC_TAG_17_CMD_SZ_SWIBS,   {0x01, 0x11, BT_QSOC_TAG_17_TAG_SZ_SWIBS,
           BT_QSOC_SLEEP_TYPE_SWIBS, 0x01, BT_QSOC_BAUD_115200,
           BT_QSOC_TAG_17_RX_SCO_BUFS_SWIBS, BT_QSOC_TAG_17_TX_SCO_BUFS_SWIBS,
           BT_QSOC_TAG_17_RETRANS_SWIBS, BT_QSOC_TAG_17_IDLE_TO_SWIBS,
           BT_QSOC_TAG_17_WAKEREQ_SWIBS
      }
  };
#endif /*FEATURE_BT_QSOC_INBAND_SLEEP*/
  static bt_qsoc_cfg_tbl_struct_type bt_qsoc_tag17_hws =
  {
      BT_QSOC_TAG_17_CMD_SZ_HWS,   {0x01, 0x11, BT_QSOC_TAG_17_TAG_SZ_HWS,
           BT_QSOC_SLEEP_TYPE_HWS, 0x01, BT_QSOC_BAUD_115200,
           BT_QSOC_TAG_17_RX_SCO_BUFS_HWS, BT_QSOC_TAG_17_TX_SCO_BUFS_HWS
      }
  };

  static bt_qsoc_cfg_tbl_struct_type *bt_qsoc_tag17 =
#if defined(FEATURE_BT_QSOC_INBAND_SLEEP)
    &bt_qsoc_tag17_ibs;
#elif defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
    &bt_qsoc_tag17_swibs;
#else /* not FEATURE_BT_QSOC_INBAND_SLEEP or FEATURE_BT_QSOC_SW_INBAND_SLEEP */
    &bt_qsoc_tag17_hws;
#endif

  if (force_hw_sleep)
  {
     bt_qsoc_tag17 = &bt_qsoc_tag17_hws;
  }

  /* To be safe, we should read tag17 from the SOC before writing it!!! */
  switch (ending_baud)
  {
     case 115200: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_115200; break;
     case 57600: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_57600; break;
     case 38400: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_38400; break;
     case 19200: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_19200; break;
     case 9600: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_9600; break;
     case 230400: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_230400; break;
     case 250000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_250000; break;
     case 460800: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_460800; break;
     case 500000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_500000; break;
     case 720000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_720000; break;
     case 921600: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_921600; break;
     case 1000000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_1000000; break;
     case 125000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_125000; break;
     case 2000000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_2000000; break;
     case 3000000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_3000000; break;
     case 4000000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_4000000; break;
     case 1600000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_1600000; break;
     case 3200000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_3200000; break;
     case 3500000: bt_qsoc_tag17->vs_cmd_data[5] = BT_QSOC_BAUD_3500000; break;

     default:
       fprintf (stderr, "bthci_qcomm_hcireset_baudratechange: QSOC does not support %d baud\n", (int) ending_baud);
       return (FALSE);  /* only on error */
  }

  if ( (bt_hci_qcomm_pfal_vs_sendcmd(
		BT_QSOC_NVM_ACCESS_OPCODE,
		(uint8 *)(bt_qsoc_tag17->vs_cmd_data),
		bt_qsoc_tag17->vs_cmd_len )
	      ) != TRUE )
  {
     fprintf (stderr, "bthci_qcomm_hcireset_baudratechange Failed NV VS Set Cmds");
     return (FALSE);  /* only on error */
  }

  /* Flow-off the UART so that response to HCI reset is received
  ** at higher baud rate once host baud rate has been changed.
  */

  (void) ioctl(fd, TIOCMGET, &flags);

  flags &= ~TIOCM_RTS;
  if (ioctl(fd, TIOCMSET, &flags) < 0)
  {
    status = FALSE;
    perror("hcireset_baudratechange: HW Flow-off error::");
  }
  else if (verbose)
  {
    printf("Flowing-off the SOC...\n");
  }

  /* Send the HCI Reset Command to the Controller at the old baud rate */
  if(FALSE != status)
  {
    status = bt_hci_qcomm_pfal_sendresetcmd();
  }

  /* After the HCI Command is completely sent to the Controller,
  ** change the Baud rate to the new baud rate
  */
  if(FALSE != status)
  {
    if (verbose &&  (baud_rate == starting_baud))
    {
       fprintf(stderr, "bt_hci_qcomm_pfal_hcireset_baudratechange: this must be shutdown: %d\n", (int) ending_baud);
    }

    if (starting_baud != ending_baud)
    {
      status = bt_hci_qcomm_pfal_changebaudrate (ending_baud); /* local uart */
    }
    else if (verbose)
    {
       fprintf(stderr, "bt_hci_qcomm_pfal_hcireset_baudratechange: starting and ending baud same: %d\n", (int) ending_baud);
    }

    if (FALSE == status)
    {
       fprintf(stderr, "bt_hci_qcomm_pfal_hcireset_baudratechange: changing local speed to %d failed.\n", (int) ending_baud);
    }
  }

  if(status)
  {
    /* Flow-on BTS402x */
    flags |= TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &flags) < 0)
    {
      perror("hcireset_baudratechange:HW Flow-on error::");
      status = FALSE;
    }
  }

  /* Get the Command Complete for the HCI Reset at the new Baud rate */
  if(status)
  {
    uint8 pEventBuffer[HC_VS_MAX_CMD_EVENT];

    /* Get COMMAND COMPLETE event */
    if (TRUE != bt_hci_qcomm_get_event(pEventBuffer))
    {
      BTHCI_QCOMM_ERROR("Failed to receive cmd cmplt event \n");
      status = FALSE;
    }
  }
  fflush (stderr);
  return status;
}
/* This generates the Override file in the given location
Currently, only BD address is the only configurable paramter
*/
boolean bt_hci_qcomm_generate_override_file()
{

  int override_fd = 0, n = 0;
  boolean retStat = FALSE;
  char override_filename[MAX_FILE_NAME];
  bt_qsoc_cfg_element_type bd_hci_cmd[] =  {
        0x09, 0x01, 0x02, 0x06,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
        };

  if (BD_addr_string != NULL) /* BD_ADDR was passed in as -b arg */
  {
    /* convert ASCII HEX representation to 48 bits */
    /* BD_addr is little-endian, but string is as printed */
    BD_addr[5] = strtol (&BD_addr_string[ 0], 0, 16);
    BD_addr[4] = strtol (&BD_addr_string[ 3], 0, 16);
    BD_addr[3] = strtol (&BD_addr_string[ 6], 0, 16);
    BD_addr[2] = strtol (&BD_addr_string[ 9], 0, 16);
    BD_addr[1] = strtol (&BD_addr_string[12], 0, 16);
    BD_addr[0] = strtol (&BD_addr_string[15], 0, 16);

   /*Make sure that the Directory exists
   and creat if there is nothing
   */
   if(mkdir(path_to_override, 0644) < 0){
     if(errno != EEXIST){
       BTHCI_QCOMM_ERROR("Not able to create the override directory\n");
       return FALSE;
     }
     printf("Errno: %s", strerror(errno));
   }

    /*If is is no soc download mode
    form the HCI command for BD address and
    create a override file*/

    snprintf(override_filename, MAX_FILE_NAME, "%s%s_override.bin", path_to_override, binary_filename);
    printf("Override file: %s", override_filename);
    override_fd = open(override_filename, O_CREAT|O_WRONLY, 0644);
    if(override_fd < 0){
      BTHCI_QCOMM_ERROR("Failed to open the override file\n");
      return FALSE;
    }
    /*Form the BD address*/
    memcpy(&bd_hci_cmd[4], &BD_addr[0], 6);

    n = write(override_fd, bd_hci_cmd, 10);
    if(n < 0) {
      BTHCI_QCOMM_ERROR("Failed to write to override file\n");
      retStat = FALSE;
    }
    else {
      retStat = TRUE;
    }
    close(override_fd);
  }
  else
  {
    printf("Info: no BD address to override\n");
  }

  return retStat;
}

boolean bt_hci_qcomm_pfal_vs_sendcmd
(
  uint16 opcode,
  uint8  *pCmdBuffer,
  uint8  nSize
)
{
  uint8 cmd[HC_VS_MAX_CMD_EVENT];   //JN: change this
  uint8 pEventBuffer[HC_VS_MAX_CMD_EVENT];
  boolean status = FALSE;
  char filename[MAX_FILE_NAME];
  int nwrite;
  int hci_cmd_header = 0;
  if(BT_HCI_UART == bt_hci_transport_device.type)
    cmd[hci_cmd_header++] = 0x01; // JN: bluetooth header files in linux has a define
                      // HCI_COMMAND_PKT for this but do we want to use
                      // something thats command between QC platforms.
  cmd[hci_cmd_header++] = (uint8)(opcode & 0xFF);
  cmd[hci_cmd_header++] = (uint8)( (opcode>>8) & 0xFF);
  cmd[hci_cmd_header] = (uint8)(nSize);

  memcpy(&cmd[HCI_CMD_HDR_SIZE - bt_hci_transport_device.pkt_ind], pCmdBuffer, nSize);


  /*If no_soc_download_mode is on and command is APP version and Hw Version read,
  Just dump the command to binary file and Indicate Success*/
  if(no_soc_download_mode == TRUE && pCmdBuffer[0] != 0x06 &&  pCmdBuffer[0] != 0x0D) {
    printf("In no soc download mode\n");
    if(binary_fd < 0)	{
     /*Make sure that the Directory exists
     and creat if there is nothing
     */
      if(mkdir(path_to_binary, 0644) < 0){
        if(errno != EEXIST){
          BTHCI_QCOMM_ERROR("Not able to create the directory\n");
          return FALSE;
        }
        printf("Errno: %s\n", strerror(errno));
      }

      snprintf(filename, MAX_FILE_NAME, "%s%s.bin", path_to_binary, binary_filename);
      binary_fd = open(filename,O_CREAT|O_WRONLY|O_EXCL, 0664);
      if(binary_fd < 0){
        if(errno == EEXIST){
          printf("File exists: don't need to generate\n");
        }
        BTHCI_QCOMM_ERROR("Failed to open the binary file\n");
        return FALSE;
      }
    }
    nwrite = write(binary_fd,(&cmd[0]), (HCI_CMD_HDR_SIZE+nSize));
    if(nwrite<0){
      BTHCI_QCOMM_ERROR("Failed to write to binary file\n");
      return FALSE;
    }
    /*Indicate success*/
    return TRUE;
  }

  nwrite = bt_hci_qcomm_nwrite((&cmd[0]), (HCI_CMD_HDR_SIZE - bt_hci_transport_device.pkt_ind+ nSize));

  if (nwrite != (HCI_CMD_HDR_SIZE - bt_hci_transport_device.pkt_ind + nSize))
  {
    fprintf(stderr, "Error->Send Header failed:: %d\n", nwrite);
    perror ("Header send failed");
    return status;
  }

  /* Get VS event */
  while(1)
  {
    memset(&pEventBuffer[0],0,HC_VS_MAX_CMD_EVENT);
    if (TRUE != bt_hci_qcomm_get_event(pEventBuffer))
    {
      fprintf(stderr, "Failed to receive VS event \n");
      fflush (stderr);
      return status;
    }
    if(bt_hci_transport_device.type == BT_HCI_SMD)
    {
       if(pEventBuffer[0] == 0xff)
         break;
    }
    else
    {
      if(pEventBuffer[1] == 0xff)
         break;
    }
     fprintf(stderr, "unknown event recvd\n");
  }

  /*TODO :Need to make the below implentation generic so as to allow any size of header*/
  bt_hci_qcomm_vs_event(&pEventBuffer[!bt_hci_transport_device.pkt_ind],
                        pEventBuffer[HCI_EVT_HDR_SIZE - bt_hci_transport_device.pkt_ind - 1]);
                        /* include opcode and payload length */

  /* Get COMMAND COMPLETE event */
  if (TRUE != bt_hci_qcomm_get_event(pEventBuffer))
  {
    fprintf(stderr, "Failed to receive cmd cmplt event \n");
    fflush (stderr);
    return status;
  }

  status = TRUE;

  /*If it Override generation mode
  Create the override file in the desired location with the given runtime parameters*/
  if (generate_override_mode == TRUE) {

    status = bt_hci_qcomm_generate_override_file();
  }
  return status;
}

boolean set_bt_status(boolean state)
{
	int rfkill_fd, i = 0, nBytes = 0;
	int bt_rfkill_index = 0;
	char val;
	char rfkill_driver_path[MAX_FILE_NAME];
	char type[16];
	int retStat = FALSE;

	do {
		snprintf(rfkill_driver_path, MAX_FILE_NAME, "/sys/class/rfkill/rfkill%d/type", i);

		rfkill_fd = open(rfkill_driver_path, O_RDONLY);
		if(rfkill_fd < 0){
			printf("failed to open the rfkill driver: %d\n", i);
			return FALSE;
		}
		/*Read the type*/
		nBytes = read(rfkill_fd, &type, sizeof(type));
		if(nBytes < 0)	{
                        printf("failed to read from the rfkill driver: %d\n", i);
			close(rfkill_fd);
                        return FALSE;
		}
		//Close the fd
		close(rfkill_fd);
		if(nBytes >= 9 && strncmp(type, "bluetooth", 9) == 0) {
			printf("Bluetooth driver found\n");
			bt_rfkill_index = i;
			break;
		}
		i++;
	}while(1);

	snprintf(rfkill_driver_path, MAX_FILE_NAME, "/sys/class/rfkill/rfkill%d/state", bt_rfkill_index);
	rfkill_fd = open(rfkill_driver_path, O_RDWR);
	if(rfkill_fd < 0){
		printf("Error while opening rfkill node\n");
		return FALSE;
	}
	if(read(rfkill_fd, &val, 1) < 0) {
		printf("Error while reading the status: %s\n", strerror(errno));
		retStat = FALSE;
	}
	else {
		if ((state == TRUE && val == '0')||(state == FALSE && val == '1')) {

			val = state ? '1':'0';

			if(write(rfkill_fd, &val ,1) < 0)     {
				printf("Error while writing to rfkill: %s\n", strerror(errno));
				retStat = FALSE;
			}
			else {
				retStat = TRUE;
			}
		}
	}
	close(rfkill_fd);

	return retStat;
}

int main(int argc, char *argv[])
{
  int c;
  int option_index = 0;
  boolean print_env_strings = FALSE;
  FILE *mystdout_fd = NULL;
  uint32 uval = 0;

  while (TRUE)
  {
      static struct option long_options[] =
      {
	   {"board-address",   required_argument, NULL, 'b'},
	   {"device",          required_argument, NULL, 'd'},
	   {"firmware-file",   required_argument, NULL, 'f'},
	   {"full-firmware-file",   required_argument, NULL, 'F'},
	   {"generate-binary-file",   required_argument, NULL, 'g'},
	   {"generate-override-file",   required_argument, NULL, 'o'},
	   {"initial-speed",   required_argument, NULL, 'i'},
	   {"reference-clock", required_argument, NULL, 'r'},
	   {"speed",           required_argument, NULL, 's'},
	   {"enable-clock-sharing",  no_argument, NULL, 'c'},
	   {"enable-soc-logging",    no_argument, NULL, 'l'},
	   {"disable-lisbon-2.1",    no_argument, NULL, 'L'},
	   {"bredr-pwr-class",       required_argument, NULL, 'p'},
	   {"le-pwr-class",          required_argument, NULL, 'P'},
	   {"force-hw-sleep",        no_argument, NULL, 'H'},
	   {"no-sleep-mode",         no_argument, NULL, 'w'},
	   {"no-reset-mode",         no_argument, NULL, 'N'},
	   {"just-reset-mode",       no_argument, NULL, 'J'},
	   {"print-env",             no_argument, NULL, 'e'},
	   {"verbose",               no_argument, NULL, 'v'},
	   {"help",                  no_argument, NULL, 'h'},
	   {0, 0, 0, 0}
      };

      c = getopt_long (argc, argv, ":b:cd:ef:F:g:o:hHi:lp:P:r:s:vw2NJL",
		long_options, &option_index);
      if (c == -1)
	   break;

      switch (c)
      {
      case 0:
	   printf ("option %s", long_options[option_index].name);
	   if (optarg)
	       printf (" with arg %s", optarg);
	   printf ("\n");
	   break;

      case 'v':
	   verbose++;
	   break;

      case 'H':
	   force_hw_sleep = TRUE;  /* Do not enable either In-Band-Sleep protocol */
	   break;

      case 'N':
	   dont_reset = TRUE;  /* Run all of SoC initializations, but do not reset or change baud */
	   break;

      case 'J':
	   just_reset = TRUE;  /* Do not do any SoC initialization, just reset the SoC and change baud */
	   break;

      case 'w':
           sleep_mode_arg_entered = TRUE;
	   sleep_mode = FALSE;  /* !sleep = wake */
	   break;

      case 'l':
           soc_logging_arg_entered = TRUE;
	   soc_logging_enabled = TRUE;  /* enable SoC logging to local port */
	   break;

      case 'L':
           disable_lisbon_arg_entered = TRUE;
	   disable_lisbon = TRUE;  /* Disable Bluetooth 2.1 features in SoC */
	   break;

      case 'p':
           if(optarg==NULL)
           {
             fprintf (stderr, "%s: option 'p' (--bredr-pwr-class) requires value %d or %d or %d.\n",
                      argv[0], BT_QSOC_DEV_CLASS1, BT_QSOC_DEV_CLASS2, BT_QSOC_DEV_CLASS_CUSTOM);
             return 1;
           }
	   switch (uval = atoi (optarg))
           {
             case BT_QSOC_DEV_CLASS1:
             case BT_QSOC_DEV_CLASS2:
             case BT_QSOC_DEV_CLASS_CUSTOM:
               bredr_pwr_class = (bt_qsoc_device_class_type)uval;
               break;

             default:
               fprintf (stderr, "%s: option 'p' (--bredr-pwr-class) requires value %d or %d or %d.\n",
                        argv[0], BT_QSOC_DEV_CLASS1, BT_QSOC_DEV_CLASS2, BT_QSOC_DEV_CLASS_CUSTOM);
               return 1;
           }
	   break;

      case 'P':
           if(optarg==NULL)
           {
             fprintf (stderr, "%s: option 'P' (--le-pwr-class) requires value %d or %d or %d.\n",
                      argv[0], BT_QSOC_DEV_CLASS1, BT_QSOC_DEV_CLASS2, BT_QSOC_DEV_CLASS_CUSTOM);
             return 1;
           }
	   switch (uval = atoi (optarg))
           {
             case BT_QSOC_DEV_CLASS1:
             case BT_QSOC_DEV_CLASS2:
             case BT_QSOC_DEV_CLASS_CUSTOM:
               le_pwr_class = (bt_qsoc_device_class_type)uval;
               break;

             default:
               fprintf (stderr, "%s: option 'P' (--le-pwr-class) requires value %d or %d or %d.\n",
                        argv[0], BT_QSOC_DEV_CLASS1, BT_QSOC_DEV_CLASS2, BT_QSOC_DEV_CLASS_CUSTOM);
               return 1;
           }
	   break;

      case 'b':
           if (optarg==NULL || (17 > strlen (optarg))
           ||  (!isxdigit(optarg[ 0]))
           ||  (!isxdigit(optarg[ 1]))
           ||  ( isxdigit(optarg[ 2]))
           ||  (!isxdigit(optarg[ 3]))
           ||  (!isxdigit(optarg[ 4]))
           ||  ( isxdigit(optarg[ 5]))
           ||  (!isxdigit(optarg[ 6]))
           ||  (!isxdigit(optarg[ 7]))
           ||  ( isxdigit(optarg[ 8]))
           ||  (!isxdigit(optarg[ 9]))
           ||  (!isxdigit(optarg[10]))
           ||  ( isxdigit(optarg[11]))
           ||  (!isxdigit(optarg[12]))
           ||  (!isxdigit(optarg[13]))
           ||  ( isxdigit(optarg[14]))
           ||  (!isxdigit(optarg[15]))
           ||  (!isxdigit(optarg[16]))
           ) {
               fprintf (stderr, "%s: option 'b' (--board-address) requires 6 hexidecimal pairs: xx.xx.xx.xx.xx.xx (%s)\n",
                        argv[0], (optarg == NULL)? "NULL": optarg);
               return 1;
           }
           BD_addr_string = optarg;
	   break;

      case 'r':
           if(optarg==NULL)
           {
             fprintf (stderr, "%s: option 'r' (--reference-clock) requires value %d or %d.\n",
                      argv[0], BT_QSOC_REF_CLK_19_2MHZ, BT_QSOC_REF_CLK_32_0MHZ);
             return 1;
           }
	   switch (ref_clk_speed = atoi (optarg))
           {
           case BT_QSOC_REF_CLK_32_0MHZ:
           case BT_QSOC_REF_CLK_19_2MHZ:
                 ref_clk_arg_entered = TRUE;
                 break;
           default:
               fprintf (stderr, "%s: option 'r' (--reference-clock) requires value %d or %d.\n",
                        argv[0], BT_QSOC_REF_CLK_19_2MHZ, BT_QSOC_REF_CLK_32_0MHZ);
               return 1;
           }
	   break;

      case 'c':
           clk_sharing_arg_entered = TRUE;
	   clock_sharing_enabled = TRUE;
	   break;

      case 's':
           if(optarg)
           {
               ending_baud = atoi (optarg);
           }
	   break;

      case 'i':
           if(optarg)
           {
               starting_baud = atoi (optarg);
           }
	   break;

      case 'e':
	   print_env_strings = TRUE;
	   break;

      case 'd':
           if(optarg)
           {
               device_name = optarg;
           }
	   break;

      case 'f':
      case 'F':
	   firmware_file_name = optarg;
	   nvm_mode = isupper(c)? NVM_EFS_MODE: NVM_HYBRID_MODE;
	   break;
      case 'g':
	  path_to_binary = optarg;
	  no_soc_download_mode = TRUE;
          if(set_bt_status(TRUE) == FALSE) {
		return -1;
	  }
	  break;
      case 'o':
          path_to_override = optarg;
          generate_override_mode = TRUE;
	  if(set_bt_status(TRUE) == FALSE) {
		return -1;
	  }
	  break;
      case ':':
          fprintf (stderr, "%s: option %c requires an argument.\n", argv[0], optopt);
          return 1;

      default:
          printf ("%s: Unrecognized option: %c\n\n", argv[0], optopt);
	  /* fall through */
      case 'h':
          printf ("Initialize BTS 402x Bluetooth System-on-Chip prior to BT HCI attachment\n\n");

#if defined(BT_QSOC_POWER_ONOFF_PATH)
          printf ("This version attempts power on/off via %s.\n\n", bt_qsoc_power_path);
#endif /* BT_QSOC_POWER_ONOFF_PATH */
#if defined(BT_QSOC_GET_ITEMS_FROM_NV)
          printf ("This version attempts NV read of BD_ADDR, refclock, clock-sharing via RPC.\n\n");
#endif /* BT_QSOC_GET_ITEMS_FROM_NV */
#if defined(FEATURE_BT_WLAN_COEXISTENCE)
          printf ("This version configures on WLAN coexistence.\n\n");
#endif /* FEATURE_BT_WLAN_COEXISTENCE */

          printf ("Usage: %s [options]\n", argv[0]);
          printf ("\noptional arguments:\n");
          printf ("   flag\tlong option\t  cur/dflt\tdescription\n\n");
          printf ("    -b\t--board-address\t  NV447 or rand\tBluetooth MAC address to use\n");
          printf ("    -d\t--device\t  %s\tUART device to BTSoC\n", device_name);
          printf ("    -f\t--firmware-file  (none)\t\tfirmware file NVM format, hybrid mode\n");
          printf ("    -F\t--full-firmware-file  (none)\tfirmware file NVM format, EFS mode\n");
	  printf ("    -g\t--generate-binary-file  (none)\tpath where binary file to be generated\n");
          printf ("    -o\t--generate-override-file  (none)\tpath where override file to be generated\n");
          printf ("    -i\t--initial-speed\t  %lu\tinitial baudrate to BTSoC\n", starting_baud);
          printf ("    -r\t--reference-clock %d\tBTS reference clock Hz\t  %d\n",
			  BT_QSOC_REF_CLK_32_0MHZ, BT_QSOC_REF_CLK_19_2MHZ);
          printf ("    -s\t--speed\t\t  %lu\tfinal baudrate to BTSoC\n", ending_baud);
          printf ("\noption flags:\n");
          printf ("    -c\t--enable-clock-sharing\t%s\tenable  clock sharing in BTSoC\n",
			 clock_sharing_enabled? "on": "off");
          printf ("    -w\t--no-sleep-mode\t\t%s\tdisable low-power protocol in BTSoC\n",
			 sleep_mode? "off": "on" );
          printf ("    -p\t--bredr-pwr-class\t  %d\tRF power class by BR/EDR BTSoC (if supported): %d or %d or %d\n",
			 bredr_pwr_class, BT_QSOC_DEV_CLASS1, BT_QSOC_DEV_CLASS2, BT_QSOC_DEV_CLASS_CUSTOM);
          printf ("    -P\t--le-pwr-class\t  %d\tRF power class by LE BTSoC (if supported): %d or %d or %d\n",
			 le_pwr_class, BT_QSOC_DEV_CLASS1, BT_QSOC_DEV_CLASS2, BT_QSOC_DEV_CLASS_CUSTOM);
          printf ("    -H\t--force_hw_sleep\t\t%s\tuse hardware sleep, not In-Band Sleep\n",
			 dont_reset? "on": "off" );
          printf ("    -N\t--no-reset\t\t%s\tdo not reset/change baud after init\n",
			 dont_reset? "on": "off" );
          printf ("    -J\t--just-reset\t\t%s\tdo not init SoC; only reset/change baud\n",
			 just_reset? "on": "off" );
          printf ("    -l\t--soc-logging\t\t%s\tenable debug logging from BTSoC\n",
			 soc_logging_enabled? "on": "off" );
          printf ("    -L\t--disable_lisbon-2.1\t%s\tdisable BT 2.1 (lisbon) features in BTSoC\n",
			 disable_lisbon? "on": "off" );
          printf ("    -e\t--print-env\t\t%s\tprint environment variables for shell\n",
			 print_env_strings? "on": "off");
          printf ("    -v\t--verbose\t\t%d\tincrement debug output level\n", verbose);
          printf ("    -h\t--help\t\t\t\tprint this usage message\n");
          return 1;

      }
  }

  if (verbose > 0)
  {
     fprintf (stderr, "%s -d %s -s %lu -i %lu -r %lu%s%s%s%s -p %d -P %d%s%s%s%s%s%s%s\n",
             argv[0],
             device_name,
             ending_baud,
             starting_baud,
             ref_clk_speed,
             clock_sharing_enabled? " -c": "",
             sleep_mode_arg_entered? " -w": "",
             soc_logging_enabled? " -l": "",
             disable_lisbon? " -L": "",
             bredr_pwr_class,
             le_pwr_class,
             force_hw_sleep? " -H": "",
             dont_reset? " -N": "",
             just_reset? " -J": "",
             BD_addr_string? " -b ": "",
             BD_addr_string? BD_addr_string: "",
	     (firmware_file_name != NULL) & (nvm_mode != NVM_AUTO_MODE)?
		nvm_mode == NVM_EFS_MODE? " -F ": " -f ":
		   "",
	     firmware_file_name? firmware_file_name: ""
    );
  }

  if (just_reset && dont_reset)
  {
     fprintf (stderr, "%s: both --no-reset and --just-reset does nothing, exiting.\n", argv[0]);
     return 0;
  }

  bt_hci_transport_device = bt_hci_set_transport();

  if(BT_HCI_UART == bt_hci_transport_device.type)
  {
  /* Powerup BTS402x */
    if (!bt_hci_qcomm_powerup()) /* success */
       return 1;
  }

      fd = bt_hci_pfal_init_transport(bt_hci_transport_device);

      if (fd < 0)
      {
        bt_hci_qcomm_shutdown();
        close (fd);
        if(no_soc_download_mode || generate_override_mode) {
           set_bt_status(FALSE);
        }
        return 1;
      }

#if defined(BT_QSOC_GET_ITEMS_FROM_NV)

  if (bt_qmi_dms_init() == FALSE)
  {
     fprintf (stderr, "%s: No remote access to NV items on modem.\n", argv[0]);
  }
  else
  {
     fprintf (stderr, "%s: Modem available for remote access to NV items.\n", argv[0]);
  }
#endif    /* BT_QSOC_GET_ITEMS_FROM_NV */

    /* Perform any Vendor specific initialization stuff before starting */

    /* At this point just close the UART and let an external Bluetooth
     * utility open the UART for HCI use.
     */
    if (!just_reset && bt_hci_qcomm_init(starting_baud, ending_baud) == FALSE)
    {
      extern int bt_qsoc_init_state;
      bt_hci_qcomm_shutdown();
      BTHCI_QCOMM_ERROR("BTS402x Initialization Failed in bt_qsoc_init_state %d\n", bt_qsoc_init_state);
      close (fd);
      if(no_soc_download_mode || generate_override_mode) {
        set_bt_status(FALSE);
      }
#if defined(BT_QSOC_GET_ITEMS_FROM_NV)
      cleanup_qmi_dms();
#endif
      return 1;
    }
#if defined(BT_QSOC_GET_ITEMS_FROM_NV)
    cleanup_qmi_dms();
#endif

    /* TODO: As RIVA doesn't require HCI RESET to be sent after NVM
       configuration, this call needs to be skipped for RIVA.
       Following call is invoked only when NVM configuration is
       done in split phases. This functionality is for test purpose
       and not used in normal mode where NVM configuration is done
       in one-shot. Hence currently this call is unreachable. */

    if (just_reset && bt_hci_qcomm_pfal_hcireset_baudratechange (ending_baud) == FALSE)
    {
      BTHCI_QCOMM_ERROR("BTS402x Reset Failed\n");
      close (fd);
      if(no_soc_download_mode || generate_override_mode) {
        set_bt_status(FALSE);
      }
      return 1;
    }
    /*closing the hci transport device */
    bt_hci_pfal_deinit_transport();

    DEBUGMSG(ZONE_INFO, "BTS402x initialized successfully %s\n", dont_reset? "without reset": "");

#if defined(FEATURE_BT_QSOC_SW_INBAND_SLEEP)
#define BTS_TYPE_STRING "qualcomm-ibs"
#else
#define BTS_TYPE_STRING "qualcomm"
#endif

  if (print_env_strings)
  {
    const char *bts_type_string = force_hw_sleep? "qualcomm": BTS_TYPE_STRING;
    mystdout_fd = fdopen (dup (STDOUT_FILENO), "a");
    (void) dup2 (STDERR_FILENO, STDOUT_FILENO);  /* so "printf error messages really go to stderr */
    if (mystdout_fd == NULL)
      return 1;
    fprintf (mystdout_fd,
           "BTS_DEVICE=%s;\nBTS_BAUD=%d;\nBTS_TYPE=%s;\nBTS_ADDRESS=%02x:%02x:%02x:%02x:%02x:%02x;\n",
            device_name,
            (int) ending_baud,
            bts_type_string,
            BD_addr[5], BD_addr[4], BD_addr[3], BD_addr[2], BD_addr[1], BD_addr[0]
    );
    fprintf (mystdout_fd, "export BTS_DEVICE BTS_BAUD BTS_TYPE BTS_ADDRESS;\n");
    fclose(mystdout_fd);
  }

  if(binary_fd != -1)
    close(binary_fd);
  if(no_soc_download_mode || generate_override_mode) {
    set_bt_status(FALSE);
  }
  return 0;
}
