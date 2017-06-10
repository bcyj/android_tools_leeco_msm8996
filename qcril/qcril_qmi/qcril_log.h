/******************************************************************************
  @file    qcril_log.h
  @brief   qcril qmi - logging utilities

  DESCRIPTION
    Wrapps logging macros for Android

  ---------------------------------------------------------------------------

  Copyright (c) 2008-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_LOG_H
#define QCRIL_LOG_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "assert.h"
#include "comdef.h"
#include <stdio.h>
#include <string.h>
#include "qcrili.h"
#ifdef FEATURE_UNIT_TEST
#include <assert.h>
#include <msg.h>
#elif defined QMI_RIL_UTF
#include <msg.h>
#include "ril_utf_log.h"
#else
#include <msgcfg.h>
#include <msg.h>
#include <diag_lsm.h>
#include <diag/include/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#endif /* FEATURE_UNIT_TEST */
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <stdarg.h>

typedef enum
{
  IPC_MESSAGE_RADIO_POWER = 1,
  IPC_MESSAGE_AM_CALL_STATE = 2
}ipc_message_id_type;

int qcril_multiple_rild_ipc_send_func(ipc_message_id_type message_id, void * payload, int payload_length, int dest_rild_instance_id);

#ifdef QMI_RIL_UTF
//#define MSG_SPRINTF_1(ignore_1, ignore_2, xx_fmt, xx_arg1)   printf(xx_fmt,xx_arg1 )
#endif


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* Persistent System Property that controls whether QCRIL messages to be logged to ADB */
#define QCRIL_LOG_ADB_ON                      "persist.radio.adb_log_on"

/* Persistent System Property that controls whether QCRIL messages to be dumped to QXDM log */
#define QCRIL_LOG_RIL_PAYLOAD_LOG_ON          "persist.radio.ril_payload_on"

/* Maximum length of log message */
#define QCRIL_MAX_IPC_PAYLOAD_SIZE            600
#define QCRIL_MAX_LOG_MSG_SIZE                512
#define QMI_RIL_THREAD_NAME_MAX_SIZE          64
#define QMI_RIL_THREAD_INFO_MAX_SIZE          64

#define MSG_INTERNAL_LEGACY_BASE             ( MSG_LEGACY_LOW + MSG_LEGACY_MED + MSG_LEGACY_HIGH + MSG_LEGACY_ERROR + MSG_LEGACY_FATAL )
#define MSG_LEGACY_ESSENTIAL                 ( MSG_INTERNAL_LEGACY_BASE + 1 )

extern FILE *rild_fp;
extern char log_buf[ QCRIL_MAX_LOG_MSG_SIZE ];
extern char log_fmt[ QCRIL_MAX_LOG_MSG_SIZE ];
extern pthread_mutex_t log_lock_mutex;
extern char thread_name[ QMI_RIL_THREAD_NAME_MAX_SIZE ];
extern boolean diag_init_complete;
extern boolean qcril_log_adb_on;

/* Subsystem type of the call flow */
typedef enum
{
  LOG_QCRIL_SUBSYSTEM_NONE, /* Used for banners e.g. state change that go across all subsystems */
  LOG_QCRIL_SUBSYSTEM_AMSS,
  LOG_QCRIL_SUBSYSTEM_QCRIL,
  LOG_QCRIL_SUBSYSTEM_ANDROID
} qcril_call_flow_subsystem_e_type;

/* Arrow type of the call flow */
typedef enum
{
  /* Bit 2 - Line type [0=Solid line, 1=Dashed line],
     Bit 1 - Arrow direction [0=One direction, 1=Bi-directional],
     Bit 0 - Arrow head type [0=Solid arrow head, 1=Open arrow head]
  */
  LOG_QCRIL_S_LINE_S_HEAD_S_DIR = 0x00, /* Solid line with solid arrow head pointing at destination */
  LOG_QCRIL_S_LINE_O_HEAD_S_DIR = 0x01, /* Solid line with open arrow head pointing at destination */
  LOG_QCRIL_S_LINE_S_HEAD_B_DIR = 0x02, /* Solid line with solid arrow head pointing at both source and destination */
  LOG_QCRIL_S_LINE_O_HEAD_B_DIR = 0x03, /* Solid line with open arrow head pointing at both source and destination */
  LOG_QCRIL_D_LINE_S_HEAD_S_DIR = 0x04, /* Dashed line with solid arrow head pointing at destination */
  LOG_QCRIL_D_LINE_O_HEAD_S_DIR = 0x05, /* Dashed line with open arrow head pointing at destination */
  LOG_QCRIL_D_LINE_S_HEAD_B_DIR = 0x06, /* Dashed line with solid arrow head pointing at both source and destination */
  LOG_QCRIL_D_LINE_O_HEAD_B_DIR = 0x07  /* Dashed line with open arrow head pointing at both source and destination */
} qcril_call_flow_arrow_e_type;

typedef struct
{
  int    event;
  char * event_name;
}qcril_qmi_event_log_type;

typedef enum _RIL__MsgType {
  RIL__MSG_TYPE__REQUEST = 1,
  RIL__MSG_TYPE__RESPONSE = 2,
  RIL__MSG_TYPE__UNSOL_RESPONSE = 3
} RIL__MsgType;

#if (!defined FEATURE_UNIT_TEST) && (!defined QMI_RIL_UTF)
/* Call flow log packet */
typedef struct
{
  log_hdr_type hdr;     /* Log header (length, code, timestamp) */
  uint8 src_subsystem;  /* Subsystem generating this call flow event */
  uint8 dest_subsystem; /* Subsystem this call flow event is being sent to */
  uint8 arrow;          /* Bit mask describing the line and arrow type to be drawn on the call flow */
  uint8 label[ 1 ];     /* Used to locate the first character of the Text to be displayed for the banner or arrow */
} qcril_call_flow_log_packet_type;
#endif /* FEATURE_UNIT_TEST */

void qcril_log_init( void );
void qcril_log_timer_init( void );
void qcril_log_cancel_log_timer();
void qcril_log_cleanup( void );
const char *qcril_log_lookup_event_name( int event_id );
int qcril_log_get_token_id( RIL_Token t );
void qcril_format_log_msg( char *buf_ptr, int buf_size, char *fmt, ... );
void qcril_log_call_flow_packet( qcril_call_flow_subsystem_e_type src_subsystem,
                                 qcril_call_flow_subsystem_e_type dest_subsystem,
                                 qcril_call_flow_arrow_e_type arrow_type, char *label );
void qcril_log_msg_to_adb( int  lvl, char *msg_ptr );
void qcril_ipc_init();
void qcril_ipc_release();
void qcril_multiple_rild_ipc_radio_power_propagation_helper_func(int is_genuine_signal);
void qcril_qmi_nas_handle_multiple_rild_radio_power_state_propagation(int is_genuine_signal);

inline void qcril_log_print_ril_message_payload(const char* format, ...);
void qcril_log_print_ril_message(int message_id, RIL__MsgType message_type,
                                  void* data, size_t datalen, RIL_Errno error);

#ifdef QMI_RIL_UTL
uint32 qcril_get_time_milliseconds();
#endif

#ifdef FEATURE_UNIT_TEST
/* Output message to Screen */
#define QCRIL_LOG_MSG( lvl, fmt, ... )                                           \
   {                                                                             \
     char buf[ QCRIL_MAX_LOG_MSG_SIZE ];                                         \
     char *p_front = __FILE__;                                                   \
     char *p_end = p_front + strlen( p_front );                                  \
     while ( p_end != p_front )                                                  \
     {                                                                           \
       if ( ( *p_end == '\\' ) || ( *p_end == ':' ) || ( *p_end == '/') )        \
       {                                                                         \
         p_end++;                                                                \
         break;                                                                  \
       }                                                                         \
       p_end--;                                                                  \
     }                                                                           \
                                                                                 \
     /* Format message for logging */                                            \
     qcril_format_log_msg( buf, QCRIL_MAX_LOG_MSG_SIZE, fmt, ##__VA_ARGS__ );      \
     (void) printf( "%s %d %s\n", p_end, __LINE__, buf );                        \
   }

#define QCRIL_LOG_ASSERT( xx_exp )  { if((xx_exp) == 0)  assert(0); }
#elif defined QMI_RIL_UTF
extern char log_buf_raw[ QCRIL_MAX_LOG_MSG_SIZE ];
/* Log message to Diag */
#define QCRIL_LOG_MSG( lvl, fmt, ... )                                           \
  {                                                                              \
    pthread_mutex_lock(&log_lock_mutex);                                         \
    uint32 t_milliseconds = qcril_get_time_milliseconds();\
    snprintf(log_buf_raw, QCRIL_MAX_LOG_MSG_SIZE, fmt, ##__VA_ARGS__ );\
    if(TRUE == qmi_ril_get_thread_name(pthread_self(), thread_name))\
    {\
        strlcpy(log_fmt, "\n%lu %s %d RIL[%d][%s] %s: %s", sizeof(log_fmt));\
        qcril_format_log_msg( log_buf, QCRIL_MAX_LOG_MSG_SIZE, log_fmt, t_milliseconds, __FILE__, __LINE__, (int)qmi_ril_get_process_instance_id(), thread_name, __func__, log_buf_raw);\
    }\
    else\
    {\
        strlcpy(log_fmt, "\n%lu %s %d RIL[%d] %s: %s", sizeof(log_fmt));\
        qcril_format_log_msg( log_buf, QCRIL_MAX_LOG_MSG_SIZE, log_fmt, t_milliseconds, __FILE__, __LINE__, (int)qmi_ril_get_process_instance_id(), __func__, log_buf_raw);\
    }\
    {                                                                                                                           \
        qcril_log_msg_to_adb( lvl, log_buf );                                                                                   \
        if( MSG_LEGACY_ESSENTIAL == lvl )          \
        {                                                                                                                       \
          RIL_UTF_SIMPLE_LOG("%s", log_buf); \
        }                                                                                                                       \
        else                                                                                                                    \
        {                                                                                                                       \
          RIL_UTF_SIMPLE_LOG("%s", log_buf); \
        }                                                                                                                       \
    }                                                                                                                           \
    (void) pthread_mutex_unlock(&log_lock_mutex);                                                                               \
  }                                                                                                                             \

#define QCRIL_LOG_MSG_LVL_BUF( lvl, buf )                                                                                       \
  {                                                                                                                             \
          RIL_UTF_SIMPLE_LOG("%s", log_buf); \
  }                                                                                                                             \

/* Log assertion level message */
#define QCRIL_LOG_ASSERT( xx_exp )                                         \
  if ( !( xx_exp ) )                                                       \
  {                                                                        \
    QCRIL_LOG_MSG( MSG_LEGACY_FATAL, "%s", "*****ASSERTION FAILED*****" ); \
    QCRIL_LOG_MSG( MSG_LEGACY_FATAL, "Cond: %s", #xx_exp );                \
    QCRIL_LOG_MSG( MSG_LEGACY_FATAL, "%s", "**************************" ); \
    assert( 0 );                                                           \
  }

#else
/* Log message to Diag */
#define QCRIL_LOG_MSG( lvl, fmt, ... )                                           \
  {                                                                              \
    if(diag_init_complete == TRUE || qcril_log_adb_on)\
    {\
      pthread_mutex_lock(&log_lock_mutex);                                         \
      if(TRUE == qmi_ril_get_thread_name(pthread_self(), thread_name))\
      {\
        strlcpy(log_fmt, "RIL[%d][%s] %s: ", sizeof(log_fmt));\
        strlcat(log_fmt, fmt, sizeof(log_fmt));\
        qcril_format_log_msg( log_buf, QCRIL_MAX_LOG_MSG_SIZE, log_fmt, (int)qmi_ril_get_process_instance_id(), thread_name, __func__, ##__VA_ARGS__ );\
      }\
      else\
      {\
        strlcpy(log_fmt, "RIL[%d] %s: ", sizeof(log_fmt));\
        strlcat(log_fmt, fmt, sizeof(log_fmt));\
        qcril_format_log_msg( log_buf, QCRIL_MAX_LOG_MSG_SIZE, log_fmt, (int)qmi_ril_get_process_instance_id(), __func__, ##__VA_ARGS__ );\
      }\
      if (diag_init_complete == TRUE)\
      {\
        if( MSG_LEGACY_ESSENTIAL == lvl )\
        {\
          MSG_SPRINTF_1( MSG_SSID_ANDROID_QCRIL, MSG_LEGACY_HIGH, "%s", log_buf );\
        }\
        else\
        {\
          MSG_SPRINTF_1( MSG_SSID_ANDROID_QCRIL, lvl, "%s", log_buf );\
        }\
      }\
      qcril_log_msg_to_adb( lvl, log_buf );\
      if(rild_fp)\
      {\
          fprintf(rild_fp, "%s\n", log_buf);\
      }\
      (void) pthread_mutex_unlock(&log_lock_mutex);\
    }\
  }\

#define QCRIL_LOG_MSG_LVL_BUF( lvl, buf )                                                                                       \
  {                                                                                                                             \
        MSG_SPRINTF_1( MSG_SSID_ANDROID_QCRIL, lvl, "%s", buf );                                                                \
  }                                                                                                                             \

/* Log assertion level message */
#define QCRIL_LOG_ASSERT( xx_exp )                                         \
  if ( !( xx_exp ) )                                                       \
  {                                                                        \
    QCRIL_LOG_MSG( MSG_LEGACY_FATAL, "%s", "*****ASSERTION FAILED*****" ); \
    QCRIL_LOG_MSG( MSG_LEGACY_FATAL, "Cond: %s", #xx_exp );                \
    QCRIL_LOG_MSG( MSG_LEGACY_FATAL, "%s", "**************************" ); \
    assert( 0 );                                                           \
  }
#endif /* FEATURE_UNIT_TEST */

/* Log warning level message */
#define QCRIL_LOG_WARN( ... )   QCRIL_LOG_MSG( MSG_LEGACY_HIGH, __VA_ARGS__ )

/* Log error level message */
#define QCRIL_LOG_ERROR( ... )   QCRIL_LOG_MSG( MSG_LEGACY_ERROR, __VA_ARGS__ )

/* Log fatal level message */
#define QCRIL_LOG_FATAL( ... )   QCRIL_LOG_MSG( MSG_LEGACY_FATAL, __VA_ARGS__ )

/* Log Essential messages */
#define QCRIL_LOG_ESSENTIAL( ... ) QCRIL_LOG_MSG( MSG_LEGACY_ESSENTIAL, __VA_ARGS__)

/* Log debug level message */
#define QCRIL_LOG_DEBUG( ... )   QCRIL_LOG_MSG( MSG_LEGACY_HIGH, __VA_ARGS__ )

/* Log info level message */
#define QCRIL_LOG_INFO( ...  )   QCRIL_LOG_MSG( MSG_LEGACY_MED, __VA_ARGS__ )

/* Log verbose level message */
#define QCRIL_LOG_VERBOSE( ... ) QCRIL_LOG_MSG( MSG_LEGACY_LOW, __VA_ARGS__ )

/* Log function entry message */
#define QCRIL_LOG_FUNC_ENTRY()   QCRIL_LOG_MSG( MSG_LEGACY_LOW, "function entry" )

/* Log function exit message */
#define QCRIL_LOG_FUNC_RETURN()  QCRIL_LOG_MSG( MSG_LEGACY_LOW, "function exit" )

/* Log function exit message */
#define QCRIL_LOG_FUNC_RETURN_WITH_RET( ... ) QCRIL_LOG_MSG( MSG_LEGACY_LOW, "function exit with ret %"PRIdPTR, __VA_ARGS__ )

#define QCRIL_LOG_QMI_RESP_STATUS(qmi_client_error, qmi_service_response_ptr, ril_error) \
  QCRIL_LOG_MSG( MSG_LEGACY_LOW, "QMI response status - qmi_client_error: %d, " \
                "qmi_serv_resp.result: %d, qmi_serv_resp.error: %d, ril_error: %d", \
                qmi_client_error, (qmi_service_response_ptr)->result, (qmi_service_response_ptr)->error, ril_error)

/* Log RPC messages */

#define QCRIL_LOG_RPC( modem_id, func_name, info_description, info_value )                                                               \
  {                                                                                                                                      \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL --- %s ---> AMSS [%s %d (0x%x)]\n", func_name, info_description, info_value, info_value ); */ \
    QCRIL_LOG_CF_PKT_MODEM_API( modem_id, func_name );                                                                                   \
  }

#define QCRIL_LOG_RPC2( modem_id, func_name, details )                                        \
  {                                                                                           \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL --- %s ---> AMSS [%s]\n", func_name, details ); */ \
    QCRIL_LOG_CF_PKT_MODEM_API( modem_id, func_name );                                        \
  }

#define QCRIL_LOG_RPC2A( modem_id, func_name, details )                                       \
  {                                                                                           \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL --- %s ---> AMSS [%s]\n", func_name, details ); */ \
    {                                                                                         \
      char label[ 300 ];                                                                      \
      QCRIL_SNPRINTF( label, sizeof( label ), "%s - %s", func_name, details );                \
      QCRIL_LOG_CF_PKT_MODEM_API( modem_id, label );                                          \
    }                                                                                         \
  }

#define QCRIL_LOG_RPC_SS( modem_id, func_name, ss_name, ss_ref_val, bsg_type_val, bsg_code_val )             \
  {                                                                                                          \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL --- %s ---> AMSS [%s, ss_ref %d, bsg_type %d, bsg_code %d]\n", */ \
    /*                func_name, ss_name, ss_ref_val, bsg_type_val, bsg_code_val ); */                       \
    QCRIL_LOG_CF_PKT_MODEM_API( modem_id, func_name );                                                       \
  }

/* Log QMI messages */
#define QCRIL_LOG_QMI( modem_id, cmd_name, details )                                          \
  {                                                                                           \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL --- %s ---> AMSS [%s]\n", cmd_name, details ); */  \
    char label[ 300 ];                                                                        \
    QCRIL_SNPRINTF( label, sizeof( label ), "%s - %s", cmd_name, details );                   \
    QCRIL_LOG_CF_PKT_MODEM_API( modem_id, label );                                            \
  }


/* Log call flow packets */

/* Log AMSS event */
#define QCRIL_LOG_CF_PKT_MODEM_EVT( modem_id, label )                                                                          \
  {                                                                                                                            \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "AMSS=>RIL [ label = \"%s\" ];", label ); */                                             \
    if ( modem_id == QCRIL_DEFAULT_MODEM_ID )                                                                                  \
    {                                                                                                                          \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_AMSS, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_S_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                          \
    else                                                                                                                       \
    {                                                                                                                          \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_AMSS, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                          \
  }

/* Log RPC to AMSS function */
#define QCRIL_LOG_CF_PKT_MODEM_API( modem_id, label )                                                                          \
  {                                                                                                                            \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL=>AMSS [ label = \"%s\" ];", label ); */                                             \
    if ( modem_id == QCRIL_DEFAULT_MODEM_ID )                                                                                  \
    {                                                                                                                          \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_AMSS, LOG_QCRIL_S_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                          \
    else                                                                                                                       \
    {                                                                                                                          \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_AMSS, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                          \
  }

/* Log RIL request */
#define QCRIL_LOG_CF_PKT_RIL_REQ( instance_id, label )                                                                            \
  {                                                                                                                               \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "Android=>RIL [ label = \"%s\" ];", label ); */                                             \
    if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )                                                                               \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_S_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                             \
    else                                                                                                                          \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                             \
  }

#define QCRIL_LOG_CF_PKT_RIL_REQ2( instance_id, label )                                                                           \
  {                                                                                                                               \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "Android:>RIL [ label = \"%s\" ];", label ); */                                             \
    if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )                                                                               \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_S_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                             \
    else                                                                                                                          \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                             \
  }

/* Log response to RIL request */
#define QCRIL_LOG_CF_PKT_RIL_RES( instance_id, label )                                                                            \
  {                                                                                                                               \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL=>Android [ label = \"%s\" ];", label ); */                                             \
    if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )                                                                               \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_S_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                             \
    else                                                                                                                          \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                             \
  }

/* Log unsolicited RIL response */
#define QCRIL_LOG_CF_PKT_RIL_UNSOL_RES( instance_id, label )                                                                      \
  {                                                                                                                               \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL=>>Android [ label = \"%s\" ];", label ); */                                            \
    if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )                                                                               \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_S_LINE_O_HEAD_S_DIR, label ); \
    }                                                                                                                             \
    else                                                                                                                          \
    {                                                                                                                             \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_D_LINE_O_HEAD_S_DIR, label ); \
    }                                                                                                                             \
  }

/* Log internal RIL event */
#define QCRIL_LOG_CF_PKT_RIL_EVT( instance_id, label )                                                                          \
  {                                                                                                                             \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "RIL=>RIL [ label = \"%s\" ];", label ); */                                               \
    if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )                                                                             \
    {                                                                                                                           \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_S_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                           \
    else                                                                                                                        \
    {                                                                                                                           \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_SUBSYSTEM_QCRIL, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                           \
  }

/* Log Android call to RIL functions */
#define QCRIL_LOG_CF_PKT_RIL_FN( instance_id, label )                                                                               \
  {                                                                                                                                 \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "Android=>Android [ label = \"%s\" ];", label ); */                                           \
    if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )                                                                                 \
    {                                                                                                                               \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_S_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                               \
    else                                                                                                                            \
    {                                                                                                                               \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_SUBSYSTEM_ANDROID, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                               \
  }

/* Log state change of RIL */
#define QCRIL_LOG_CF_PKT_RIL_ST_CHG( instance_id, label )                                                                     \
  {                                                                                                                           \
    /* QCRIL_LOG_MSG( MSG_LEGACY_MED, "--- [ label = \"%s\" ];\n", label ); */                                                \
    if ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )                                                                           \
    {                                                                                                                         \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_NONE, LOG_QCRIL_SUBSYSTEM_NONE, LOG_QCRIL_D_LINE_S_HEAD_S_DIR, label ); \
    }                                                                                                                         \
    else                                                                                                                      \
    {                                                                                                                         \
      qcril_log_call_flow_packet( LOG_QCRIL_SUBSYSTEM_NONE, LOG_QCRIL_SUBSYSTEM_NONE, LOG_QCRIL_D_LINE_O_HEAD_S_DIR, label ); \
    }                                                                                                                         \
  }

#endif /* QCRIL_LOG_H */
