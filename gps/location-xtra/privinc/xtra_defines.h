/******************************************************************************
  @file:  xtra_defines.h
  @brief: general definitions of xtra

  DESCRIPTION

  XTRA Definition
  -----------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/


#ifndef XTRA_DEFINES_H_
#define XTRA_DEFINES_H_

#ifdef _LINUX_
#include <stdint.h>
#else
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
#endif
typedef unsigned long DWORD;
typedef int64_t GpsUtcTime;

#define OK 1
#define FAIL 0




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

//unnecessary
//#define XTRA_DATA_BUFFER_SIZE 1024 //TODO: How much this is?

#define XTRA_MAX_FILE_PATH_LEN 255
#ifndef XTRA_DEFAULT_CFG_PATH
#define XTRA_DEFAULT_CFG_PATH "./xtra.cfg"
#endif

#define XTRA_URL_MAX_LEN          256
#define XTRA_HTTP_HOST_MAX_LEN    128
#define XTRA_HTTP_PATH_MAX_LEN    256
#define XTRA_HTTP_HTTPREQ_MAX_LEN 300
#define USER_AGENT_STRING_LEN     256

//default values
#define XTRA_MAX_REMOTE_FILE_SIZE (1024*1024)
#define XTRA_DEFAULT_SERVER1 "http://xtra1.gpsonextra.net/xtra2.bin"
#define XTRA_DEFAULT_SERVER2 "http://xtra2.gpsonextra.net/xtra2.bin"
#define XTRA_DEFAULT_SERVER3 "http://xtra3.gpsonextra.net/xtra2.bin"
#define XTRA_NTP_DEFAULT_SERVER1 "time.gpsonextra.net"

#define AMOUNT_NTP_SERVERS 1 //Only 1 !
#define AMOUNT_XTRA_SERVERS 3

#define MAX_LOG_MSG_LEN 255

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

typedef struct  {
   unsigned long long time_utc;
   unsigned long uncertainty;
}xtra_assist_data_time_s_type;


#endif /* LOC_API_XTRA_TIME_H_ */

typedef struct tagXTRASYSTEMTIME
{
   unsigned short wYear;
   unsigned short wMonth;
   unsigned short wDayOfWeek;
   unsigned short wDay;
   unsigned short wHour;
   unsigned short wMinute;
   unsigned short wSecond;
   unsigned short wMilliseconds;
} XTRA_SYSTEMTIME;

typedef char xtra_url_t[XTRA_URL_MAX_LEN];
typedef char xtra_path_t[XTRA_MAX_FILE_PATH_LEN];

typedef struct tagXtraCfgData
{
   unsigned char xtra_time_info_enabled;   //enable time injection
   unsigned char xtra_log_level;           //log level
   unsigned char xtra_downloading_enabled; //enable extra downlods
   xtra_url_t xtra_sntp_server_url[AMOUNT_NTP_SERVERS];    //time servers
   xtra_url_t xtra_server_url[AMOUNT_XTRA_SERVERS];        //xtra servers
   char user_agent_string[USER_AGENT_STRING_LEN];
    xtra_path_t xtra_server_stats_path;  //server statistics file
    unsigned char xtra_log_server_stats; //enable server statistics
}xtra_cfgdata_t;


typedef struct
{
   int last_used_xtra_server;  //index of last succesfully used xtra server
   int last_used_sntp_server;  //index of last succesfully used sntp server
}server_stats_t;

typedef struct
{
   unsigned long file_checksum;
   unsigned short file_length;
   xtra_cfgdata_t data;
}xtra_config_t;

typedef struct
{
   unsigned char   bTerminate;              // terminate daemon;
   unsigned char   bSave_config;            //testing feature: allow config saving
   xtra_config_t config;
   unsigned long     xtra_datalen;
   unsigned char*    xtra_data;
   xtra_assist_data_time_s_type assist_time; //NTP time storage
   server_stats_t server_statistics; //usage tracking
}globals_t;



#endif

