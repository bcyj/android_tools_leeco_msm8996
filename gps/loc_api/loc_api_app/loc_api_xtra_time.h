/******************************************************************************
  @file:  loc_api_xtra_time.h
  @brief: loc_api_test XTRA time services header

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
01/10/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_xtra_time.h#3 $
======================================================================*/

#ifndef LOC_API_XTRA_TIME_H_
#define LOC_API_XTRA_TIME_H_

#define SECSINSEVENTYYEARS 2208988800ll         // from 1 Jan 1900 to 1970 (start of Unix time)
#define SECSINTENYEARS     315964800            // from 1970 to 1980 (Start of GPS time)

#define SNTP_PORT_NUM      123
#define SNTP_VN_NUM        4
#define SNTP_CLIENT_MODE   3
#define SNTP_SERVER_MODE   4
#define SNTP_TIMEOUT_SEC   5
#define SNTP_TIMEOUT_USEC  0
#define SNTP_RECV_BUF_SIZE 128

//#define MIN( a, b ) ( (a>b)?(b):(a) )
#define SNTP_LI(x)   ( (x>>6) & 3 )
#define SNTP_VN(x)   ( (x>>3) & 7 )
#define SNTP_MODE(x) (  x     & 7 )

typedef struct {
   unsigned int sec;
   unsigned int fsec;
} gps_xtra_time_s_type;

typedef struct
{
   unsigned char             li_vn_mode;       /* leap indicator, version and mode */
   unsigned char             stratum;          /* peer stratum */
   unsigned char             poll;             /* peer poll interval */
   char                      precision;        /* peer clock precision */
   unsigned int              root_delay;       /* distance to primary clock */
   unsigned int              root_dispersion;  /* clock dispersion */
   unsigned int              ref_id;           /* reference clock ID */
   gps_xtra_time_s_type      ref_time;         /* time peer clock was last updated */
   gps_xtra_time_s_type      orig_time;        /* originate time stamp */
   gps_xtra_time_s_type      recv_time;        /* receive time stamp */
   gps_xtra_time_s_type      tx_time;          /* transmit time stamp */
} gps_xtra_sntp_pkt_s_type;

extern int loc_xtra_download_sntp_time(rpc_loc_assist_data_time_s_type *assist_time);
extern void loc_xtra_get_sys_time( rpc_loc_assist_data_time_s_type *assist_time );

#endif /* LOC_API_XTRA_TIME_H_ */
