/*==========================================================================

                     FTM Main Task Source File

Description
  Unit test component file for regsitering the routines to Diag library
  for BT and FTM commands

# Copyright (c) 2010-2014 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/18/10   rakeshk  Created a source file to implement routines for
                    registering the callback routines for FM and BT FTM
                    packets
07/06/10   rakeshk  changed the name of FM common header file in inclusion
07/07/10   rakeshk  Removed the sleep and wake in the main thread loop
===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <net/if.h>

#include <dirent.h>
#include <pwd.h>
#include <cutils/sched_policy.h>
#include <ctype.h>
#include <getopt.h>

/* Diag related header files */
#include "event.h"
#include "msg.h"
#include "log.h"
#include "diag_lsm.h"
#include "diagpkt.h"
#include "diagcmd.h"
#include "diag.h"

#include "ftm_dbg.h"
#include "ftm_bt.h"
#include "ftm_wlan.h"
#include "ftm_fm_common.h"
#include "ftm_common.h"
#include "ftm_ant_common.h"

#include "ftm_nfc.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#endif

int boardtype = 8660;
int first_ant_command;

static char *progname = NULL;

unsigned int g_dbg_level = FTM_DBG_DEFAULT;

#define SHOW_PRIO 1
#define SHOW_TIME 2
#define SHOW_POLICY 4
#define SHOW_CPU  8
#define SHOW_MACLABEL 16


#ifdef CONFIG_FTM_BT
/* Semaphore to monitor the completion of
* the queued command before sending down the
* next HCI cmd
*/
sem_t semaphore_cmd_complete;
/* Semaphore to monitor whether a command
* is queued before proceeding to dequeue
* the HCI packet
*/
sem_t semaphore_cmd_queued;
/* Callback declaration for BT FTM packet processing */
PACK(void *) bt_ftm_diag_dispatch (PACK(void *)req_pkt,
  uint16 pkt_len);
/* Diag pkt table for BT */
static const diagpkt_user_table_entry_type bt_ftm_diag_func_table[] =
{
  {FTM_BT_CMD_CODE, FTM_BT_CMD_CODE, bt_ftm_diag_dispatch},
};
#endif /* CONFIG_FTM_BT */


#ifdef CONFIG_FTM_FM
/* Callback declaration for BT FTM packet processing */
PACK(void *) fm_ftm_diag_dispatch (PACK(void *)req_pkt,
  uint16 pkt_len);

/* Diag pkt table for FM */
static const diagpkt_user_table_entry_type fm_ftm_diag_func_table[] =
{
  {FTM_FM_CMD_CODE, FTM_FM_CMD_CODE, fm_ftm_diag_dispatch},
};
#endif /* CONFIG_FTM_FM */

#ifdef CONFIG_FTM_ANT
/* Callback declaration for ANT FTM packet processing */
PACK(void *) ant_ftm_diag_dispatch (PACK(void *)req_pkt,
  uint16 pkt_len);

/*Diag pkt table for ANT */
static const diagpkt_user_table_entry_type ant_ftm_diag_func_table[] =
{
  {FTM_ANT_CMD_CODE, FTM_ANT_CMD_CODE, ant_ftm_diag_dispatch}
};
#endif /* CONFIG_FTM_ANT */

#ifdef CONFIG_FTM_WLAN
/* Callback declaration for WLAN FTM packet processing */
PACK(void *) wlan_ftm_diag_dispatch (PACK(void *)req_pkt,
  uint16 pkt_len);

/* Diag pkt table for WLAN */
static const diagpkt_user_table_entry_type wlan_ftm_diag_func_table[] =
{
  {FTM_WLAN_CMD_CODE, FTM_WLAN_CMD_CODE, wlan_ftm_diag_dispatch}
};
#endif

#ifdef CONFIG_FTM_NFC
/* Callback declaration for NFC FTM packet processing */
PACK(void *) nfc_ftm_diag_dispatch (PACK(void *)req_pkt,
  uint16 pkt_len);

/*Diag pkt table for NFC */
static const diagpkt_user_table_entry_type nfc_ftm_diag_func_table[] =
{
  {FTM_NFC_CMD_CODE, FTM_NFC_CMD_CODE, nfc_ftm_diag_dispatch}
};
#endif /* CONFIG_FTM_NFC */

#ifdef CONFIG_FTM_FM
/*=========================================================================
FUNCTION   fm_ftm_diag_dispatch

DESCRIPTION
  Processes the request packet and sends it to the FTM FM layer for further
  processing

DEPENDENCIES
  NIL

RETURN VALUE
  pointer to FTM FM Response packet

SIDE EFFECTS
  None

===========================================================================*/
PACK(void *) fm_ftm_diag_dispatch
(
  PACK(void *)req_pkt,
  uint16 pkt_len
)
{
 PACK(void *)rsp = NULL;

 DPRINTF(FTM_DBG_TRACE, "FM I2C Send Response = %d\n",pkt_len);

 // Allocate the same length as the request.
 rsp = ftm_fm_dispatch(req_pkt,pkt_len);
 return rsp;
}
#endif /* CONFIG_FTM_FM */

#ifdef CONFIG_FTM_ANT
/*===========================================================================
FUNCTION   ant_ftm_diag_dispatch

DESCRIPTION
  Processes the request packet and sends it to the FTM ANT layer for further
  processing
DEPENDENCIES
 NIL

RETURN VALUE
  pointer to FTM ANT  Response packet

SIDE EFFECTS
  None

===========================================================================*/
PACK(void *) ant_ftm_diag_dispatch
(
  PACK(void *)req_pkt,
  uint16 pkt_len
)
{
 PACK(void *)rsp = NULL;

 DPRINTF(FTM_DBG_TRACE, "ANT diag dispatch send response = %d\n", pkt_len);

// Allocate the same length as the request.
 rsp = ftm_ant_dispatch(req_pkt,pkt_len);
 return rsp;
}
#endif /* CONFIG_FTM_ANT */

#ifdef CONFIG_FTM_BT
/*===========================================================================
FUNCTION   bt_ftm_diag_dispatch

DESCRIPTION
  Processes the request packet and sends it to the FTM BT layer for further
  processing

DEPENDENCIES
  NIL

RETURN VALUE
  pointer to FTM BT Response packet

SIDE EFFECTS
  None

===========================================================================*/
PACK(void *) bt_ftm_diag_dispatch
(
  PACK(void *)req_pkt,
  uint16 pkt_len
)
{
  PACK(void *)rsp = NULL;
  boolean status = TRUE;

  DPRINTF(FTM_DBG_TRACE, "Send Response = %d\n",pkt_len);

  // Allocate the same length as the request.
  rsp = diagpkt_subsys_alloc (DIAG_SUBSYS_FTM, FTM_BT_CMD_CODE, pkt_len);

  if (rsp != NULL)
  {
    memcpy ((void *) rsp, (void *) req_pkt, pkt_len);
  }
  /* Spurious incoming request packets are occasionally received
   * by DIAG_SUBSYS_FTM which needs to be ignored and accordingly responded.
   * TODO: Reason for these spurious incoming request packets is yet to be
   *       found, though its always found to be corresponding to this majic
   *       length of 65532.
   */
  if (pkt_len == 65532)
  {
    printf("\nIgnore spurious DIAG packet processing & respond immediately");
  }
  else
  {

    DPRINTF(FTM_DBG_TRACE, "Insert BT packet = %d\n", pkt_len);

    /* add the BT packet into the Cmd Queue
     * and notify the main thread its queued
     */
    status = qinsert_cmd((ftm_bt_pkt_type *)req_pkt);
    if(status == TRUE)
      sem_post(&semaphore_cmd_queued);

    DPRINTF(FTM_DBG_TRACE, "Insert BT packet done\n");

  }
  return (rsp);
}
#endif /* CONFIG_FTM_BT */

#ifdef CONFIG_FTM_WLAN
/*===========================================================================
FUNCTION   wlan_ftm_diag_dispatch

DESCRIPTION
  Processes the request packet and sends it to the FTM WLAN layer for further
  processing

DEPENDENCIES
  NIL

RETURN VALUE
  pointer to FTM WLAN Response packet

SIDE EFFECTS
  None

===========================================================================*/

PACK(void *) wlan_ftm_diag_dispatch
(
  PACK(void *)req_pkt,
  uint16 pkt_len
)
{
    PACK(void *)rsp = NULL;

    DPRINTF(FTM_DBG_TRACE, "WLAN Send Response = %d\n", pkt_len);

    rsp = ftm_wlan_dispatch(req_pkt, pkt_len);

    return rsp;
}
#endif /* CONFIG_FTM_WLAN */

#ifdef CONFIG_FTM_NFC
/*===========================================================================
FUNCTION   nfcs_ftm_diag_dispatch

DESCRIPTION
  Processes the request packet and sends it to the FTM NFC layer for further
  processing
DEPENDENCIES
 NIL

RETURN VALUE


SIDE EFFECTS


===========================================================================*/
PACK(void *) nfc_ftm_diag_dispatch
(
  PACK(void *)req_pkt,
  uint16 pkt_len
)
{
    PACK(void *)rsp = NULL;
    boolean status = TRUE;

    DPRINTF(FTM_DBG_TRACE, " NFC Send Response = %d\n",pkt_len);

    /*now send the incoming nfc diag command packet to the nfc ftm layer to
      get it processed*/
    rsp = ftm_nfc_dispatch(req_pkt, pkt_len);

    /* send same response as recieved back*/
    return rsp;
}
#endif /* CONFIG_FTM_NFC */

static void usage(void)
{
    fprintf(stderr, "\nusage: %s [options] \n"
            "   -n, --nodaemon      do not run as a daemon\n"
            "   -d                  show more debug messages (-dd for even more)\n"
#ifdef CONFIG_FTM_BT
            "   -b, --board-type    Board Type\n"
#endif
#ifdef CONFIG_FTM_WLAN
            "   -i <wlan interface>\n"
            "       --interface=<wlan interface>\n"
            "                       wlan adapter name (wlan, eth, etc.) default wlan\n"
#endif
            "       --help          display this help and exit\n"
            , progname);
    exit(EXIT_FAILURE);
}

/*===========================================================================
FUNCTION   main

DESCRIPTION
  Initialises the Diag library and registers the PKT table for FM and BT
  and daemonises

DEPENDENCIES
  NIL

RETURN VALUE
  NIL, Error in the event buffer will mean a NULL App version and Zero HW
  version

SIDE EFFECTS
  None

===========================================================================*/

int main(int argc, char *argv[])
{
    int c;
    static struct option options[] =
    {
        {"help", no_argument, NULL, 'h'},
#ifdef CONFIG_FTM_WLAN
        {"interface", required_argument, NULL, 'i'},
#endif
#ifdef CONFIG_FTM_BT
        {"board-type", required_argument, NULL, 'b'},
#endif
        {"nodaemon", no_argument, NULL, 'n'},
        {0, 0, 0, 0}
    };
    int daemonize = 1;

    progname = argv[0];

    while (1)
    {
        c = getopt_long(argc, argv, "hdi:nb:", options, NULL);

        if (c < 0)
            break;

        switch (c)
        {
#ifdef CONFIG_FTM_WLAN
        case 'i':
            strlcpy(g_ifname, optarg, IFNAMSIZ);
            break;
#endif
        case 'n':
            daemonize = 0;
            break;
        case 'd':
#ifdef DEBUG
            g_dbg_level = g_dbg_level << 1 | 0x1;
#else
            printf("Debugging disabled, please build with -DDEBUG option\n");
            exit(EXIT_FAILURE);
#endif
            break;

#ifdef CONFIG_FTM_BT
        case 'b':
            boardtype = atoi(optarg);
            break;
#endif

        case 'h':
        default:
            usage();
            break;
        }
    }

    if (optind < argc)
        usage();

    if (daemonize && daemon(0, 0))
    {
        perror("daemon");
        exit(EXIT_FAILURE);
    }

    DPRINTF(FTM_DBG_TRACE, "FTM Daemon calling LSM init\n");

    if (!Diag_LSM_Init(NULL))
    {
        DPRINTF(FTM_DBG_ERROR, "FTM Daemon: Diag_LSM_Init() failed\n");
        exit(EXIT_FAILURE);
    }

    DPRINTF(FTM_DBG_TRACE, "FTMDaemon: Diag_LSM_Init succesful\n");

#ifdef CONFIG_FTM_FM
    DIAGPKT_DISPATCH_TABLE_REGISTER( DIAG_SUBSYS_FTM, fm_ftm_diag_func_table);
#endif

#ifdef CONFIG_FTM_WLAN
    DIAGPKT_DISPATCH_TABLE_REGISTER( DIAG_SUBSYS_FTM, wlan_ftm_diag_func_table);
#endif

#ifdef CONFIG_FTM_ANT
    DIAGPKT_DISPATCH_TABLE_REGISTER( DIAG_SUBSYS_FTM, ant_ftm_diag_func_table);
#endif

#ifdef CONFIG_FTM_NFC
    DIAGPKT_DISPATCH_TABLE_REGISTER( DIAG_SUBSYS_FTM, nfc_ftm_diag_func_table);
#endif

#ifdef CONFIG_FTM_BT
    DIAGPKT_DISPATCH_TABLE_REGISTER( DIAG_SUBSYS_FTM, bt_ftm_diag_func_table);


    sem_init(&semaphore_cmd_complete,0, 1);
    sem_init(&semaphore_cmd_queued,0,0);
    first_ant_command = 0;

    DPRINTF(FTM_DBG_TRACE, "Initialised the BT FTM cmd queue handlers \n");

    do
    {
        struct timespec ts;
        int sem_status;
        /* We have the freedom to send the first request without wating
         * for a command complete
         */
        DPRINTF(FTM_DBG_TRACE,
                "Wait on cmd complete from the previous command\n");
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            printf("get clock_gettime error");
        ts.tv_sec += 5;
        /*we wait for 5 secs for a command already queued for
         * transmision
         */
        sem_status = sem_timedwait(&semaphore_cmd_complete,&ts);
        if(sem_status == -1)
        {
            printf("Command complete timed out\n");
            ftm_bt_err_timedout();
        }

        DPRINTF(FTM_DBG_TRACE, "Waiting on next Cmd to be queued\n");

        sem_wait(&semaphore_cmd_queued);
        dequeue_send();
    }
    while(1);
#else /* CONFIG_FTM_BT */
    while (1);
#endif

    DPRINTF(FTM_DBG_TRACE, "\nFTMDaemon Deinit the LSM\n");

    /* Clean up before exiting */
    Diag_LSM_DeInit();

    exit(EXIT_SUCCESS);
}

