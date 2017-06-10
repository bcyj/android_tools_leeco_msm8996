/*==========================================================================

                     FTM WLAN Source File

# Copyright (c) 2011, 2013-2014 by Qualcomm Technologies, Inc.
# All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who       what, where, why
--------   ---       ----------------------------------------------------------
07/11/11   karthikm  Wrapper that contains routines for directing FTM commands
                     sent from host to the IOCTL calls of Atheros driver.
*/

/*
 * Copyright (c) 2006 Atheros Communications Inc.
 * All rights reserved.
 *
 *
// The software source and binaries included in this development package are
// licensed, not sold. You, or your company, received the package under one
// or more license agreements. The rights granted to you are specifically
// listed in these license agreement(s). All other rights remain with Atheros
// Communications, Inc., its subsidiaries, or the respective owner including
// those listed on the included copyright notices.  Distribution of any
// portion of this package must be in strict compliance with the license
// agreement(s) terms.
// </copyright>
//
// <summary>
//      FTM_WLAN_TCMD
//  Based on athtestcmd.c from AR6003 drop
// </summary>
//
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/limits.h>

#include "comdef.h"
#include "diagcmd.h"

#include "testcmd.h"
#include "libtcmd.h"
#include "ftm_wlan.h"
#include "ftm_dbg.h"

#define INVALID_FREQ    0
#define A_RATE_NUM      28
#define G_RATE_NUM      28
#define RATE_STR_LEN    20

#ifdef CONFIG_FTM_WLAN_AUTOLOAD
#define MODULE_FILE             "/proc/modules"
#define DRIVER_MODULE_TAG       "wlan"
#define WLAN_CFG_FILE           "/persist/wlan_mac.bin"
/* Offset for set mac address cmd */
#define MAIN_CMD_OFFSET         16
#define SUB_CMD_OFFSET          24
#define LENGTH_OFFSET           32
#define NMAC_OFFSET             48
#define PMAC_OFFSET             49

#if defined(ANDROID)
#if defined(BOARD_HAS_ATH_WLAN_AR6320)
#if BOARD_HAS_ATH_WLAN_AR6320
#define FTM_WLAN_LOAD_CMD       "/system/bin/insmod " \
                                "/system/lib/modules/wlan.ko con_mode=5"
#define FTM_WLAN_UNLOAD_CMD     "/system/bin/rmmod wlan"
#endif
#endif
#elif defined(MDM9635_LE)
#define FTM_WLAN_LOAD_CMD       "/etc/init.d/wlan start_ftm"
#define FTM_WLAN_UNLOAD_CMD     "/etc/init.d/wlan stop"
#else
#warning "Load and Unload driver may not work!"
#endif

typedef enum {
    SUBCMD_DRIVER_LOAD      = 'L',
    SUBCMD_DRIVER_UNLOAD    = 'U',
    SUBCMD_DRIVER_AUTO_MODE = 'A',
} sub_cmds;

static int load_wifi_driver_testmode(void);
static int unload_wifi_driver(void);
static bool is_wifi_driver_loaded(char *mod_tag);
static bool flag_driver_auto_load = false;
#endif /* CONFIG_FTM_WLAN_AUTOLOAD */

static void rxReport(void *buf);
bool ifs_init[32]  = {false};
char g_ifname[IFNAMSIZ] = "wlan";

void print_uchar_array(uint8_t *addr, int len)
{
   int i;
   for (i = 0;i< len; i++)
      DPRINTF(FTM_DBG_TRACE, "%02X ", addr[i]);
   DPRINTF(FTM_DBG_TRACE, "\n");
}

void print_uint16_array(uint16_t *addr, int len)
{
   int i;
   for (i = 0;i< len; i++)
      DPRINTF(FTM_DBG_TRACE, "%02X %02X ", addr[i]>>8, addr[i]&0xFF);
   DPRINTF(FTM_DBG_TRACE, "\n");
}

/*===========================================================================
FUNCTION   rxReport

DESCRIPTION
  Quick debug routine that will print all the receive statistics

DEPENDENCIES
  NIL

RETURN VALUE
  NIL

SIDE EFFECTS
  NONE

===========================================================================*/
static void rxReport(void *buf)
{
    uint32 pkt;
    int  rssi;
    uint32 crcError;
    uint32 secErr;
    uint16 rateCnt[TCMD_MAX_RATES];
    uint16 rateCntShortGuard[TCMD_MAX_RATES];

    pkt = *(uint32 *)buf;
    rssi = (int)(*((uint32 *)buf + 1));
    crcError = *((uint32 *)buf + 2);
    secErr = *((uint32 *)buf + 3);

    DPRINTF(FTM_DBG_TRACE, "total pkt %lu, crcError pkt %lu, secErr pkt %lu, average rssi %d\n",
            pkt, crcError, secErr, (int)( pkt ? (rssi / (int)pkt) : 0));


    memcpy(rateCnt, ((unsigned char *)buf) + (4 * sizeof(uint32)), sizeof(rateCnt));
    memcpy(rateCntShortGuard, ((unsigned char *)buf) + (4 * sizeof(uint32)) +
            (TCMD_MAX_RATES * sizeof(uint16)), sizeof(rateCntShortGuard));

    DPRINTF(FTM_DBG_INFO, "1Mbps     %d\n", rateCnt[0]);
    DPRINTF(FTM_DBG_INFO, "2Mbps     %d\n", rateCnt[1]);
    DPRINTF(FTM_DBG_INFO, "5.5Mbps   %d\n", rateCnt[2]);
    DPRINTF(FTM_DBG_INFO, "11Mbps    %d\n", rateCnt[3]);
    DPRINTF(FTM_DBG_INFO, "6Mbps     %d\n", rateCnt[4]);
    DPRINTF(FTM_DBG_INFO, "9Mbps     %d\n", rateCnt[5]);
    DPRINTF(FTM_DBG_INFO, "12Mbps    %d\n", rateCnt[6]);
    DPRINTF(FTM_DBG_INFO, "18Mbps    %d\n", rateCnt[7]);
    DPRINTF(FTM_DBG_INFO, "24Mbps    %d\n", rateCnt[8]);
    DPRINTF(FTM_DBG_INFO, "36Mbps    %d\n", rateCnt[9]);
    DPRINTF(FTM_DBG_INFO, "48Mbps    %d\n", rateCnt[10]);
    DPRINTF(FTM_DBG_INFO, "54Mbps    %d\n", rateCnt[11]);
    DPRINTF(FTM_DBG_INFO, "\n");
    DPRINTF(FTM_DBG_INFO, "HT20 MCS0 6.5Mbps   %d (SGI: %d)\n", rateCnt[12], rateCntShortGuard[12]);
    DPRINTF(FTM_DBG_INFO, "HT20 MCS1 13Mbps    %d (SGI: %d)\n", rateCnt[13], rateCntShortGuard[13]);
    DPRINTF(FTM_DBG_INFO, "HT20 MCS2 19.5Mbps  %d (SGI: %d)\n", rateCnt[14], rateCntShortGuard[14]);
    DPRINTF(FTM_DBG_INFO, "HT20 MCS3 26Mbps    %d (SGI: %d)\n", rateCnt[15], rateCntShortGuard[15]);
    DPRINTF(FTM_DBG_INFO, "HT20 MCS4 39Mbps    %d (SGI: %d)\n", rateCnt[16], rateCntShortGuard[16]);
    DPRINTF(FTM_DBG_INFO, "HT20 MCS5 52Mbps    %d (SGI: %d)\n", rateCnt[17], rateCntShortGuard[17]);
    DPRINTF(FTM_DBG_INFO, "HT20 MCS6 58.5Mbps  %d (SGI: %d)\n", rateCnt[18], rateCntShortGuard[18]);
    DPRINTF(FTM_DBG_INFO, "HT20 MCS7 65Mbps    %d (SGI: %d)\n", rateCnt[19], rateCntShortGuard[19]);
    DPRINTF(FTM_DBG_INFO, "\n");
    DPRINTF(FTM_DBG_INFO, "HT40 MCS0 13.5Mbps    %d (SGI: %d)\n", rateCnt[20], rateCntShortGuard[20]);
    DPRINTF(FTM_DBG_INFO, "HT40 MCS1 27.0Mbps    %d (SGI: %d)\n", rateCnt[21], rateCntShortGuard[21]);
    DPRINTF(FTM_DBG_INFO, "HT40 MCS2 40.5Mbps    %d (SGI: %d)\n", rateCnt[22], rateCntShortGuard[22]);
    DPRINTF(FTM_DBG_INFO, "HT40 MCS3 54Mbps      %d (SGI: %d)\n", rateCnt[23], rateCntShortGuard[23]);
    DPRINTF(FTM_DBG_INFO, "HT40 MCS4 81Mbps      %d (SGI: %d)\n", rateCnt[24], rateCntShortGuard[24]);
    DPRINTF(FTM_DBG_INFO, "HT40 MCS5 108Mbps     %d (SGI: %d)\n", rateCnt[25], rateCntShortGuard[25]);
    DPRINTF(FTM_DBG_INFO, "HT40 MCS6 121.5Mbps   %d (SGI: %d)\n", rateCnt[26], rateCntShortGuard[26]);
    DPRINTF(FTM_DBG_INFO, "HT40 MCS7 135Mbps     %d (SGI: %d)\n", rateCnt[27], rateCntShortGuard[27]);
}

ftm_wlan_rx_rsp_type *g_rsp = NULL;
TCMD_ID tcmd = TCMD_CONT_RX_ID;
uint32 mode = 0;

/*===========================================================================
FUNCTION   ftm_wlan_tcmd_rx

DESCRIPTION
   Call back handler

DEPENDENCIES
  NIL

RETURN VALUE
  NONE

SIDE EFFECTS
  NONE

===========================================================================*/

void ftm_wlan_tcmd_rx(void *buf, int len)
{
    void *data = NULL;
    int data_len = 0;
    struct TCMD_CONT_RX_REPORT *report = NULL;
    TC_CMDS *tCmd = NULL;

    DPRINTF(FTM_DBG_TRACE, "Rx call back received with len %d\n", len);

    /* Build the response to be sent */
    switch(tcmd) {
    case TCMD_CONT_RX_ID:
        report = &((TCMD_CONT_RX *) buf)->u.report;

        if (mode == TCMD_CONT_RX_REPORT) {
            rxReport((void*)report);
        } else if (mode == TCMD_CONT_RX_GETMAC) {
            tCmd = (TC_CMDS *)buf;

            DPRINTF(FTM_DBG_TRACE, "length %d version %d act %d\n",
                    tCmd->hdr.u.parm.length, tCmd->hdr.u.parm.version,
                    tCmd->hdr.act);

            DPRINTF(FTM_DBG_INFO,
                    "MAC address : %02x:%02x:%02x:%02x:%02x:%02x\n",
                    tCmd->buf[0], tCmd->buf[1], tCmd->buf[2],
                    tCmd->buf[3], tCmd->buf[4], tCmd->buf[5]);
        }

        data = report;
        data_len = sizeof(struct TCMD_CONT_RX_REPORT);
        break;

    case TC_CMDS_ID:
        tCmd = (TC_CMDS *)buf;

        DPRINTF(FTM_DBG_TRACE, "length %d version %d act %d\n",
                tCmd->hdr.u.parm.length, tCmd->hdr.u.parm.version,
                tCmd->hdr.act);
        data = buf;
        data_len = sizeof(TC_CMDS);

        if ( mode == TC_CMDS_READTHERMAL )
            DPRINTF(FTM_DBG_INFO, "Chip Thermal value: %d\n", tCmd->buf[0]);
        break;

    case TC_CMD_TLV_ID:
        data = buf;
        data_len = len;
        print_uchar_array(buf, len);

        DPRINTF(FTM_DBG_TRACE, "tcmd_rx TC_CMD_TLV_ID length %d\n", len);
        break;

    default:
        data_len = 0;
        data = NULL;
        DPRINTF(FTM_DBG_TRACE, "Unknown TCMD response\n");
        break;
    }

    g_rsp = (ftm_wlan_rx_rsp_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM,
            FTM_WLAN_CMD_CODE, (sizeof(ftm_wlan_gen_rsp_type) + data_len));

    if (g_rsp == NULL) {
        DPRINTF(FTM_DBG_ERROR, "Failed to allocate diag packet! tcmd: %d",
                tcmd);
        return;
    }

    if (data && data_len != 0)
        memcpy(g_rsp->data, data, data_len);

    g_rsp->result = FTM_ERR_CODE_PASS;
}

/*===========================================================================
FUNCTION   isResponseNeeded

DESCRIPTION
   Do we need a response for the command

DEPENDENCIES
  NIL

RETURN VALUE
  boolean response required/not

SIDE EFFECTS
  NONE

===========================================================================*/
static bool isResponseNeeded(void *buf)
{
    bool respNeeded = false;

    tcmd = *((uint32 *) buf);
    mode = *((uint32 *) buf + 1);

    /// Insert commands which need response
    switch (tcmd)
    {
    case TC_CMD_TLV_ID:
        respNeeded = true;
        break;
    case TCMD_CONT_RX_ID:
        switch (mode)
        {
        case TCMD_CONT_RX_REPORT:
        case TCMD_CONT_RX_GETMAC:
            respNeeded = true;
            break;
        }
        break;
    case TC_CMDS_ID:
        switch (mode)
        {
        case TC_CMDS_READTHERMAL:
        case TC_CMDS_EFUSEDUMP:
        case TC_CMDS_EFUSEWRITE:
        case TC_CMDS_OTPSTREAMWRITE:
        case TC_CMDS_OTPDUMP:
            respNeeded = true; //TC_CMDS_EFUSEDUMP, TC_CMDS_EFUSEWRITE, TC_CMDS_OTPSTREAMWRITE, TC_CMDS_OTPDUMP, TC_CMDS_READTHERMAL
            break;
        }
        break;
    default:
        break;
    }

    if (respNeeded)
    {
        DPRINTF(FTM_DBG_TRACE, "cmdID %d response needed\n", tcmd);
    }
    else
    {
        DPRINTF(FTM_DBG_TRACE, "cmdID %d response not needed\n", tcmd);
    }

    return respNeeded;
}

/*===========================================================================
FUNCTION   ftm_wlan_dispatch

DESCRIPTION
  WLAN FTM dispatch routine. Main entry point routine for WLAN FTM for
  AR6003

DEPENDENCIES
  NIL

RETURN VALUE
  Returns back buffer that is meant to be passed to the diag callback

SIDE EFFECTS
  NONE

===========================================================================*/
void* ftm_wlan_dispatch(ftm_wlan_pkt_type *wlan_ftm_pkt, int pkt_len)
{
   unsigned int cmd = 0;
   ftm_wlan_gen_rsp_type *rsp;
   int data_len = pkt_len - sizeof(diagpkt_subsys_header_v2_type) - 4;
   char ifname[IFNAMSIZ];
   bool resp = false;

   snprintf(ifname, sizeof(ifname), "%s%d", g_ifname,
           wlan_ftm_pkt->wlandeviceno);
   if (data_len <= 0) {
       DPRINTF(FTM_DBG_ERROR, "Invalid data_len: %d\n", data_len);
       return NULL;
   }

   if (wlan_ftm_pkt->data == NULL) {
       DPRINTF(FTM_DBG_ERROR, "Invalid Data Reference\n");
       return NULL;
   }
   cmd = *((uint32*)wlan_ftm_pkt->data);
   DPRINTF(FTM_DBG_TRACE, "Command ID rec'd: 0x%X length %d\n", cmd, data_len);

   print_uchar_array((uint8_t*)(wlan_ftm_pkt->data), data_len);

   g_rsp = NULL;

   rsp = (ftm_wlan_gen_rsp_type*)diagpkt_subsys_alloc( DIAG_SUBSYS_FTM
                   , FTM_WLAN_CMD_CODE
                   , sizeof(ftm_wlan_gen_rsp_type)
                   );

   if (rsp == NULL) {
           DPRINTF(FTM_DBG_ERROR, "Failed to allocate Diag packet: %p\n", rsp);
           goto err_out;
   }

   rsp->result = FTM_ERR_CODE_PASS;

#ifdef CONFIG_FTM_WLAN_AUTOLOAD
   if (cmd == TCMD_LOAD_DRIVER) {
       /* Get sub-cmd */
       unsigned int sub_cmd = 0;
       if (data_len >= 2*sizeof(uint32))
           sub_cmd = *((uint32*)wlan_ftm_pkt->data + 1);

       /* execute sub_cmd */
       if (sub_cmd == SUBCMD_DRIVER_LOAD) {
           load_wifi_driver_testmode();
       } else if (sub_cmd == SUBCMD_DRIVER_UNLOAD) {
           unload_wifi_driver();
       } else if (sub_cmd == SUBCMD_DRIVER_AUTO_MODE) {
           flag_driver_auto_load = true;
       }

       /* These commands will not go to firmware */
       return rsp;
   }
   else if ((data_len > PMAC_OFFSET) && *(wlan_ftm_pkt->data + MAIN_CMD_OFFSET) == OP_GENERIC_NART_CMD) {
       if (*(wlan_ftm_pkt->data + SUB_CMD_OFFSET) == TCMD_SET_MAC_ADDR) {
           int i = 0;
           uint8_t length = 0, nMac = 0, *pMac = NULL;
           length = *(wlan_ftm_pkt->data + LENGTH_OFFSET);
           nMac = *(wlan_ftm_pkt->data + NMAC_OFFSET);
           pMac = (wlan_ftm_pkt->data + PMAC_OFFSET);

           DPRINTF(FTM_DBG_TRACE, "Handling WLAN request of setting Mac address\n");

           if (!nMac || (length < (5 + 6*nMac))) {
               DPRINTF(FTM_DBG_ERROR, "invalid data, nMac = %u, length = %u\n",
                       nMac, length);
               rsp->result = FTM_ERR_CODE_IOCTL_FAIL;
               goto err_out;
           }

           FILE *fp = fopen(WLAN_CFG_FILE, "wb");
           if (fp == NULL) {
               DPRINTF(FTM_DBG_INFO, "Can't open file %s\n", WLAN_CFG_FILE);
               rsp->result = FTM_ERR_CODE_IOCTL_FAIL;
               goto err_out;
           }

           for (i = 0; i < nMac; i++) {
               fprintf(fp, "Intf%dMacAddress=%02X%02X%02X%02X%02X%02X\n", i,
                       pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
               pMac = pMac + 6;
           }
           fprintf(fp, "END");
           fclose(fp);
           /* This command will not go to the firmware */
           return rsp;
       }
   }

   if ((flag_driver_auto_load == true) \
       && (!is_wifi_driver_loaded(DRIVER_MODULE_TAG))) {
       /* load the driver to testmode */
       load_wifi_driver_testmode();
   }
#endif

   /*
    * As a 8994 bring up hack we are initializing the nl
    * socket fd for every msg from the QRCT. We are investigating
    * why the sendmsg fails with ECONNREFUSED. This is a temp fix
    * till the issue is triaged.
    */
   ifs_init[wlan_ftm_pkt->wlandeviceno] = false;
   if (!ifs_init[wlan_ftm_pkt->wlandeviceno])
   {
       DPRINTF(FTM_DBG_TRACE, "Initializing Interface: %s\n", ifname);

       if (tcmd_tx_init(ifname, ftm_wlan_tcmd_rx))
       {
           DPRINTF(FTM_DBG_ERROR, "Couldn't init tcmd transport!\n");
           rsp->result = FTM_ERR_CODE_IOCTL_FAIL;
           goto err_out;
       }

       DPRINTF(FTM_DBG_TRACE, "tcmd: Initialized Interface: %s\n", ifname);
       ifs_init[wlan_ftm_pkt->wlandeviceno] = true;
   }

   resp = isResponseNeeded( (void*)wlan_ftm_pkt->data);

   if (tcmd_tx(wlan_ftm_pkt->data, data_len, resp))
   {
        DPRINTF(FTM_DBG_ERROR, "TCMD timed out!\n");
        rsp->result = FTM_ERR_CODE_IOCTL_FAIL;
        goto err_out;
   }

   if (resp)
   {
       if (g_rsp)
       {
          diagpkt_free(rsp);
          return (void *) g_rsp;
       }
       else
       {
           DPRINTF(FTM_DBG_ERROR, "No response got probably timing out.... \n");
           rsp->result = FTM_ERR_CODE_IOCTL_FAIL;
       }
  }

err_out:
    return (void *) rsp;
}

#ifdef CONFIG_FTM_WLAN_AUTOLOAD
/*===========================================================================
FUNCTION
  load_wifi_driver_testmode

DESCRIPTION
  Use system call to load driver

DEPENDENCIES
  NIL

RETURN VALUE
  NONE

SIDE EFFECTS
  NONE
===========================================================================*/

static int load_wifi_driver_testmode(void)
{
    int ret = 0;

    /* clean-up the driver state */
    if ((ret = unload_wifi_driver())) {
        DPRINTF(FTM_DBG_ERROR, "%s: Unload driver failed: %d\n", __func__, ret);
        return ret;
    }

#ifdef FTM_WLAN_LOAD_CMD
    if ((ret = system(FTM_WLAN_LOAD_CMD))) {
        DPRINTF(FTM_DBG_ERROR, "WLAN driver load failed!\n");
        return ret;
    }
#else
#error "FTM_WLAN_LOAD_CMD is not defined!"
#endif

    DPRINTF(FTM_DBG_TRACE, "WLAN driver loaded in FTM mode successfully!\n");

    return ret;
}
/*===========================================================================
FUNCTION
  Unload_wifi_driver if the drvier is detected existing already

DESCRIPTION
  Use system call to unload driver

DEPENDENCIES
  NIL

RETURN VALUE
  NONE

SIDE EFFECTS
  NONE
===========================================================================*/
static int unload_wifi_driver(void)
{
    int ret = 0;

    if (is_wifi_driver_loaded(DRIVER_MODULE_TAG)) {
#ifdef FTM_WLAN_UNLOAD_CMD
        if ((ret = system(FTM_WLAN_UNLOAD_CMD))) {
            DPRINTF(FTM_DBG_ERROR, "WLAN driver unload failed!\n");
            return ret;
        }
#else
#error "FTM_WLAN_UNLOAD_CMD is not defined!"
#endif
    }

    DPRINTF(FTM_DBG_TRACE, "WLAN driver unloaded successfully!\n");

    return ret;
}

/*===========================================================================
FUNCTION
  is_wifi_driver_loaded

DESCRIPTION
  Check if WLAN driver is loaded or not

DEPENDENCIES
  NIL

RETURN VALUE
  Returns true if driver already loaded, false if driver not loaded

SIDE EFFECTS
  NONE
===========================================================================*/
static bool is_wifi_driver_loaded(char *mod_tag)
{
    FILE *proc = NULL;
    char cmd[NAME_MAX + 64]; /* File name max + room for command */
    bool ret = false;

    snprintf(cmd, sizeof(cmd), "cat /proc/modules | grep %s", mod_tag);

    proc = popen(cmd, "r");

    if (proc == NULL) {
        DPRINTF(FTM_DBG_ERROR, "%s failed!\n", __func__);
        return ret;
    }

    if (fread (cmd, 1, sizeof(cmd), proc) > 0)
        ret = true;

    fclose(proc);

    return ret;
}
#endif /* CONFIG_FTM_WLAN_AUTOLOAD */
