/*===========================================================================

                         Q C M A P_ S T A _ I N T E R F A C E . C

DESCRIPTION

  The Data Services Qualcomm Mobile Access Point STA interface source file.

EXTERNALIZED FUNCTIONS

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
/*===========================================================================

when        who    what, where, why
--------    ---    ---------------------------------------------------------- 
12/06/12    cp     Created module
01/11/14    sr     Added support for connected devices in SoftAP
===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include "comdef.h"
#include "qcmap_cm_api.h"

/* Socket used for STA events posting. */
unsigned int sta_qcmap_sockfd;

#define NUM_ARGS_EXPECTED_FOR_STA_MODE_EVENT 3
#define NUM_ARGS_EXPECTED_FOR_AP_EVENT  4

#define STA_ASSOCIATED "CONNECTED"
#define STA_DISASSOCIATED "DISCONNECTED"
#define AP_STA_ASSOCIATED "AP-STA-CONNECTED"
#define AP_STA_DISSOCIATED "AP-STA-DISCONNECTED"

int create_sta_socket(unsigned int *sockfd)
{

  if ((*sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if(fcntl(*sockfd, F_SETFD, FD_CLOEXEC) < 0)
  {
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);
  }

  return QCMAP_CM_ENOERROR;
}

int main(int argc, char **argv)
{
  qcmap_sta_buffer_t qcmap_sta_buffer;
  int numBytes=0, len;
  struct sockaddr_un sta_qcmap;

  if (argc == NUM_ARGS_EXPECTED_FOR_STA_MODE_EVENT)
  {
    LOG_MSG_INFO1("QCMAP STA Interface called by WPA_SUPPLICANT event",
                  0, 0, 0);
  }
  else if(argc == NUM_ARGS_EXPECTED_FOR_AP_EVENT)
  {
    LOG_MSG_INFO1("QCMAP STA Interface called by WLAN Client event",
                  0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("QCMAP STA Interface called with incorrect number"
                  "of arguements",0,0,0);
    exit(1);
  }

  LOG_MSG_INFO1("Got %s event on STA interface %s", argv[2], argv[1], 0);

  if ( strncmp(argv[2], STA_ASSOCIATED, strlen(STA_ASSOCIATED)) == 0 )
  {
    qcmap_sta_buffer.event = STA_CONNECTED;
  }
  else if ( strncmp(argv[2], STA_DISASSOCIATED,
            strlen(STA_DISASSOCIATED)) == 0 )
  {
    qcmap_sta_buffer.event = STA_DISCONNECTED;
  }
  else if ( strncmp(argv[2], AP_STA_ASSOCIATED,
            strlen(AP_STA_ASSOCIATED)) == 0 )
  {
    qcmap_sta_buffer.event = AP_STA_CONNECTED;

    if (!strncmp(argv[1],WLAN0,strlen(WLAN0)))
    {
      qcmap_sta_buffer.device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
    }
    else
    {
      qcmap_sta_buffer.device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
    }
    strlcpy ( qcmap_sta_buffer.mac_addr, argv[3],
              QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01);
  }
  else if ( strncmp(argv[2], AP_STA_DISSOCIATED,
            strlen(AP_STA_DISSOCIATED)) == 0 )
  {
    qcmap_sta_buffer.event = AP_STA_DISCONNECTED;

    if (!strncmp(argv[1],WLAN0,strlen(WLAN0)))
    {
      qcmap_sta_buffer.device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
    }
    else
    {
      qcmap_sta_buffer.device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
    }
    strlcpy ( &qcmap_sta_buffer.mac_addr, argv[3],
              QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01);
  }
  else
  {
    LOG_MSG_INFO1("QCMAP STA Interface: unsupported event %s",
                   argv[2], 0, 0);
    exit(1);
  }

  if ( create_sta_socket(&sta_qcmap_sockfd) != QCMAP_CM_ENOERROR )
  {
    LOG_MSG_ERROR("QCMAP STA Interface: failed to create client sta sockfd.",
                   0, 0, 0);
    exit(1);
  }

  /* Send the event to DS QCMAP server socket. */
  sta_qcmap.sun_family = AF_UNIX;
  strcpy(sta_qcmap.sun_path, QCMAP_STA_UDS_FILE);
  len = strlen(sta_qcmap.sun_path) + sizeof(sta_qcmap.sun_family);

  /* update the STA cookie. */
  qcmap_sta_buffer.sta_cookie = 0xDCDCDCDC;

  if ((numBytes = sendto(sta_qcmap_sockfd, (void *)&qcmap_sta_buffer, sizeof(qcmap_sta_buffer), 0,
             (struct sockaddr *)&sta_qcmap, len)) == -1)
  {
    LOG_MSG_ERROR("Send Failed from sta interface context", 0, 0, 0);
    close(sta_qcmap_sockfd);
    exit(1);
  }

  close(sta_qcmap_sockfd);

  return 0;

}

