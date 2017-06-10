/*==========================================================================

                     WDS Main Task Source File

Description
  Added wds daemon to enable testing of Host Controller Interface (HCI)
  communication with stack layers bypassed.
  1. Acts as a communication bridge between PC based tester tool
  communicating over UART and target host side transport as SMD (Shared Memory).
  Here the stack layers are bypassed.
  2. Used to test exchange of BT-FM HCI commands, events and ACL data packets
  between host and controller.

# Copyright (c) 2012 by Qualcomm Atheros, Inc..
# All Rights Reserved.
# Qualcomm Atheros Confidential and Proprietary.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/18/11  subrsrin  Created a source file to process the FM and BT HCI
                    packets
5/8/12    ankurn    Added support for ANT+ HCI commands
===========================================================================*/
#include "stdio.h"
#include <unistd.h>
#include <pthread.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "wds_hci_pfal.h"
#include <sys/time.h>
#include <getopt.h>
#include <strings.h>
#include <termios.h>
#include <math.h>

#define WDS_DEBUG 0

int fd_wds_port;
int option_selected;
typedef unsigned char uint8;

/*===========================================================================
FUNCTION   get_pkt_type

DESCRIPTION
  Routine to get the packet type from the data bytes received

DEPENDENCIES
  NIL

RETURN VALUE
  Packet type for the data bytes received

SIDE EFFECTS
  None

===========================================================================*/
int get_pkt_type(uint8 pkt_indicator)
{
  int packet_type;

  if(pkt_indicator == BT_CMD_PKT_ID)
    packet_type = BT_CMD_PKT;
  else if(pkt_indicator == FM_CMD_PKT_ID)
    packet_type = FM_CMD_PKT;
  else if(pkt_indicator == BT_ACL_DATA_PKT_ID)
    packet_type = BT_ACL_PKT;
  else if(pkt_indicator == ANT_CMD_PKT_ID)
    packet_type = ANT_CMD_PKT;
  else if(pkt_indicator == ANT_DATA_PKT_ID)
    packet_type = ANT_DATA_PKT;

  return(packet_type);
}

/*===========================================================================

FUNCTION   main

DESCRIPTION
  Reads Command/ACL packets from UART interface and sends them to SMD
  interface

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
  int retval, i;
  int total_length, para_acl_length, total_acl_length;
  static uint8 bufArr[UART_BUF_SIZE];
  uint8 bufWrite[UART_BUF_SIZE];
  uint8 length_msb1, length_msb2, length_lsb1, length_lsb2;
  fd_set readfds;
  int ntotal, pkt_type, nbytes, bytes_count_processed = 0;
  int bytes_count = 0;
  char *uart_tty_interface;

  uart_tty_interface = BT_HS_NMEA_DEVICE;
  //set port properties for UART port
  fd_wds_port = init_uart(uart_tty_interface);

  if (argc == 2) {
    switch(argv[1][1])
    {
      case 'a':
        printf("Opening ANT SMD Channels\n");
        option_selected = ANT_ONLY;
        break;
      case 'b':
        printf("Opening BT SMD Channels\n");
        option_selected = BT_ONLY;
        break;
      case 'f':
        printf("Opening FM SMD Channel\n");
        option_selected = FM_ONLY;
        break;
#ifdef BT_SOC_TYPE_ROME
      case 'u':
        printf("Opening UART BT Channel\n");
        option_selected = UART_BT_ONLY;
        break;
      case 'n':
         printf("Opening UART ANT Channel\n");
         option_selected = UART_ANT_ONLY;
         break;
#endif
      case 'h':
        printf("By Default, it will open all SMD channels\n");
        printf("Use -a for opening only ANT Channels\n");
        printf("Use -b for opening only BT Channels\n");
        printf("Use -f for opening only FM Channels\n");
#ifdef BT_SOC_TYPE_ROME
        printf("Use -u for opening only UART Channel for BT (ROME)\n");
        printf("Use -n for opening only UART Channel for ANT (ROME)\n");
#endif
        return 0;
        break;
      default:
        option_selected = ALL_SMD_CHANNELS;
    }
  } else  if (argc > 2){
    //If more than one is provided we exit
    printf("Invalid number of arguments\n");
    printf("Exiting\n");
    return -1;
  } else {
    printf("Opening All SMD Channels\n");
    option_selected = ALL_SMD_CHANNELS;
  }

  //initiate reader thread to read smd1, smd2 and smd3, smd5, smd6 interfaces
  wds_initiate_thread();
  do
  {
    FD_ZERO(&readfds);
    FD_SET(fd_wds_port, &readfds);
    retval = select(fd_wds_port + 1, &readfds, NULL, NULL, NULL);
    if (retval == -1) {
      if(WDS_DEBUG)
        printf("select failed\n");
      break;
    }
    if (FD_ISSET(fd_wds_port,&readfds)) {
      nbytes = 0;
      ntotal = 0;
      do
      {
         ntotal += nbytes;
         nbytes = read(fd_wds_port, (char *)(&(bufArr[ntotal])), 1);
      } while ((nbytes > 0) && (ntotal < sizeof(bufArr)));

      if (nbytes < 0) {
        if(WDS_DEBUG) {
          printf("\n Error while reading %d\n", nbytes);
          perror("The error is");
          printf("errno = %d.\n", errno);
        }
        if (errno == EINVAL)
          continue;
      }
      if (WDS_DEBUG) {
         printf("\n The data read from wds commnader ntotal(%d)\n", ntotal);
        for (i=0; i < ntotal; i++)
          printf(" %02X", bufArr[i]);
        printf("\n");
      }

      if (ntotal > 0) {
        while (ntotal > bytes_count_processed) {
           int st = 1;
           if (option_selected == UART_BT_ONLY || option_selected == UART_ANT_ONLY) st = 0;

          //BT Command pkt and FM Command pkt
          if ((bufArr[0]== BT_CMD_PKT_ID) || (bufArr[0]== FM_CMD_PKT_ID)) {

            bytes_count = 0;
            total_length = BT_FM_PKT_UART_HDR_LEN + bufArr[3];
            bytes_count_processed += total_length;
            pkt_type = get_pkt_type(bufArr[0]);

            //process the data obtained from UART
            for (i = st; i<total_length; i++) {
              bufWrite[bytes_count++] = bufArr[i];
            }

            wds_pkt_dispatch(bufWrite, bytes_count, pkt_type);
            if (ntotal > bytes_count_processed) {
              for (i = 0; i < ntotal; i++)
                bufArr[i] = bufArr[i+total_length];
            }
          }
          //ACL packet
          else if(bufArr[0] == BT_ACL_DATA_PKT_ID) {
            bytes_count=0;
            //calculate total length of ACL pkt
            para_acl_length = get_acl_pkt_length(bufArr[4], bufArr[3]);
            total_acl_length = ACL_PKT_UART_HDR_LEN + para_acl_length;
            bytes_count_processed += total_acl_length;
            pkt_type = get_pkt_type(bufArr[0]);

            //process the data obtained from UART
            for (i = st; i < total_acl_length; i++)
              bufWrite[bytes_count++] = bufArr[i];

            wds_pkt_dispatch(bufWrite, bytes_count, pkt_type);
            if (ntotal > bytes_count_processed) {
              for (i = 0;i < ntotal; i++)
                bufArr[i] = bufArr[i+total_acl_length];
            }
          }
          //ANT CMD pkt or ANT DATA pkt
          else if (bufArr[0] == ANT_CMD_PKT_ID || bufArr[0] == ANT_DATA_PKT_ID) {
            bytes_count = 0;
            total_length = ANT_CMD_DATA_PKT_UART_HDR_LEN + bufArr[1];
            bytes_count_processed += total_length;
            pkt_type = get_pkt_type(bufArr[0]);

            printf("ANT CMD: ");
            for (i=0; i < ntotal; i++)
               printf(" %02X", bufArr[i]);
            printf("\n");
            //process the data obtained from UART
            for (i = st; i<total_length; i++)
              bufWrite[bytes_count++] = bufArr[i];
            wds_pkt_dispatch(bufWrite, bytes_count, pkt_type);
            if (ntotal > bytes_count_processed) {
                for (i = 0; i < ntotal; i++) {
                    bufArr[i] = bufArr[i+total_length];
              }
            }
          }

        }
        bytes_count_processed = 0;
      }
    }
  }
  while(1);
  printf("\n Main Thread exited \n");
  return 0;
}
