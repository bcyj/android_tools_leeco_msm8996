/******************************************************************************
  @file:  loc_api_xtra_time.c
  @brief: loc_api_test XTRA time services

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
03/17/09   dx       Android version
01/10/09   dx       Initial version, derived from cl's GPS XTRA client

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_xtra_time.c#5 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>         /* struct sockaddr_in */
#include <netdb.h>              /* struct hostent     */
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_xtra_time.h"
#include "loc_api_xtra_bin.h"

/*===========================================================================

FUNCTION loc_xtra_get_sys_time

DESCRIPTION
   Fills system time in loc_api assistance time struct

DEPENDENCIES
   n/a

RETURN VALUE
   none

SIDE EFFECTS
   n/a

===========================================================================*/
void loc_xtra_get_sys_time( rpc_loc_assist_data_time_s_type *assist_time )
{
   struct timeval  time_val;

   gettimeofday(&time_val, (void *)0);

   assist_time->time_utc = (unsigned int) time_val.tv_sec; /* secs since 00:00 UTC 1/1/1970 */
         // (unsigned int)(70 * 365 + 17) * (24 * 60 * 60));
   assist_time->time_utc *= 1000;

   assist_time->uncertainty = 20 /* sec */ * 1000;
}

/*===========================================================================

FUNCTION gps_xtra_get_sys_time

DESCRIPTION
   Fills system time in gps_xtra_time_s_type struct
   
DEPENDENCIES
   n/a

RETURN VALUE
   none

SIDE EFFECTS
   n/a

===========================================================================*/
static void gps_xtra_get_sys_time( gps_xtra_time_s_type *time_ptr )
{
  struct timeval  time_val;

  gettimeofday(&time_val, (void *)0);
    
  time_ptr->sec = htonl((unsigned int)time_val.tv_sec + (unsigned int)(70 * 365 + 17) * (24 * 60 * 60));
  time_ptr->fsec = htonl((unsigned int)(4294967296. * (double)time_val.tv_usec/1000000. +.5));
}

/*===========================================================================

FUNCTION loc_xtra_create_sntp_request_pkt

DESCRIPTION

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
static int loc_xtra_create_sntp_request_pkt(gps_xtra_sntp_pkt_s_type *pkt_ptr)
{
   if(pkt_ptr == NULL)
   {
      return -1;
   }
   pkt_ptr->li_vn_mode = (SNTP_VN_NUM <<3) + SNTP_CLIENT_MODE;
   gps_xtra_get_sys_time(&pkt_ptr->tx_time);
   return 0;
}

/*===========================================================================

FUNCTION loc_xtra_wait_resp

DESCRIPTION

DEPENDENCIES
   n/a

RETURN VALUE
   0   if successful
  -1   error

SIDE EFFECTS
   n/a

===========================================================================*/
static int loc_xtra_wait_resp
( 
      int                       sock_fd,
      gps_xtra_sntp_pkt_s_type *resp_pkt
)
{
   struct sockaddr_in dst;
   int dst_len;
   int len, ret;
   char recv_buf[SNTP_RECV_BUF_SIZE];
   fd_set rfds;
   struct timeval tv;

   dst_len = sizeof(dst);
   bzero( &dst, dst_len );
   FD_ZERO(&rfds);
   FD_SET( sock_fd, &rfds);
   tv.tv_sec  = SNTP_TIMEOUT_SEC;
   tv.tv_usec = SNTP_TIMEOUT_USEC;
   ret = select( sock_fd + 1, &rfds, NULL, NULL, &tv );

   if( ret )
   {
      len = recvfrom(sock_fd,
            recv_buf,
            SNTP_RECV_BUF_SIZE,
            0,
            (struct sockaddr *)&dst,
            &dst_len);

      if(ntohs(dst.sin_port) != SNTP_PORT_NUM)
      {
         return -1;
      }
   }
   else
   {
      loc_write_error("SNTP receive time out\n" );
      return -1;
   }

   if(len < (signed) sizeof(*resp_pkt))
   {
      loc_write_error("Packet size error.\n");
   }

   memcpy( resp_pkt, recv_buf, sizeof(*resp_pkt));

   if( SNTP_VN(resp_pkt->li_vn_mode) != SNTP_VN_NUM
         ||SNTP_MODE(resp_pkt->li_vn_mode) != SNTP_SERVER_MODE)
   {
      loc_write_error("Illegal packet.\n");
   }

   return 0;
}

/*===========================================================================

FUNCTION timeval_diff

DESCRIPTION
   Calculates the difference between time intervals 

DEPENDENCIES
   n/a

RETURN VALUE
   result - time difference in seconds and microseconds
   return: if y is newer

SIDE EFFECTS
   n/a

===========================================================================*/
static int timeval_diff
(
      struct timeval *result, 
      struct timeval *x, 
      struct timeval *y
)
{
    int y_is_newer;
    int diff_sec;
    int diff_usec;

    diff_sec  = y->tv_sec  - x->tv_sec;
    diff_usec = y->tv_usec - x->tv_usec;

    /* in case usec is greater than 1 second */
    diff_sec += diff_usec / 1000000;
    diff_usec = diff_usec % 1000000;

    /* offset the negative value for usec */
    if (diff_usec > 0) {
        diff_sec  += 1;
        diff_usec -= 1000000;
    }

    /* x is newer or y is newer? */
    if ( (diff_sec >  0)                        ||
         (diff_sec == 0 && diff_usec > 0)   ) {
        /* y is newer */
        y_is_newer = 1;
    } else if ( diff_sec == 0 && diff_usec == 0 ) {
        /* same time */
        y_is_newer = 0;
    } else {
        /* y is older */
        y_is_newer = 0;
    }

    result->tv_sec  = -diff_sec;
    result->tv_usec = -diff_usec;

    return y_is_newer;
}

/*===========================================================================

FUNCTION loc_xtra_download_sntp_time

DESCRIPTION
   Download time from SNTP server

DEPENDENCIES
   n/a

RETURN VALUE
   0            if successful.
  -1            error

SIDE EFFECTS
   n/a

===========================================================================*/
int loc_xtra_download_sntp_time(rpc_loc_assist_data_time_s_type *assist_time)
{
   struct sockaddr_in         peer_addr;
   gps_xtra_sntp_pkt_s_type   sntp_req_pkt_info;
   gps_xtra_sntp_pkt_s_type   sntp_resp_pkt_info;
   struct timeval             sntp_req_send_time;  
   struct timeval             sntp_resp_recv_time;
   struct timeval             time_diff;  
   unsigned long long         time_seconds;
   int                        sntp_socket_fd;
   int                        rc;
   
   /* DNS query */
   loc_write_msg("Resolving hostname for SNTP server...\n");
   
   memset(&peer_addr,0, sizeof(peer_addr));
   peer_addr.sin_family = AF_INET;
   peer_addr.sin_port = htons(SNTP_PORT_NUM);   
   if (!resolve_in_addr(param.xtra_server_addr, &peer_addr.sin_addr))
   {
      loc_write_error("Cannot resolve hostname name: %s\n", param.xtra_server_addr);
      return -1;
   }
   
   /* open the socket */
   loc_write_msg("Connecting to SNTP server: %s\n", inet_ntoa(peer_addr.sin_addr));
   sntp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
   if(sntp_socket_fd < 0)
   {
      loc_write_error("Socket failed.\n");
      return -1;
   }

   bzero(&sntp_req_pkt_info, sizeof(sntp_req_pkt_info));
   if(loc_xtra_create_sntp_request_pkt(&sntp_req_pkt_info) == -1)
   {
      loc_write_error("SNTP pkt request creation failed.\n");
      if(sntp_socket_fd) { close(sntp_socket_fd); }
      return -1;
   }

   gettimeofday(&sntp_req_send_time, (void *)0);
   /* Send SNTP Request */
   sendto(sntp_socket_fd,
         (void *)&sntp_req_pkt_info,
         sizeof(sntp_req_pkt_info),
         0,
         (struct sockaddr *)&peer_addr,
         sizeof(peer_addr));

   /* Wait SNTP Resp */
   rc = loc_xtra_wait_resp(sntp_socket_fd, &sntp_resp_pkt_info);
   if (rc != 0)
   {
      if (sntp_socket_fd) { close(sntp_socket_fd); }
      return -1; /* timed out, exit */
   }
   loc_write_msg("Received SNTP Resp\n");
   close(sntp_socket_fd);

   /* Fill Time Inject Info */
   gettimeofday(&sntp_resp_recv_time, (void *)0);
   timeval_diff(&time_diff, &sntp_resp_recv_time, &sntp_req_send_time);
   assist_time->uncertainty = time_diff.tv_sec * 1000 + time_diff.tv_usec / 1000;
   time_seconds = ntohl(sntp_resp_pkt_info.tx_time.sec);
   assist_time->time_utc = 1000 * (time_seconds - SECSINSEVENTYYEARS);

   return 0; /* successful */
}
