/*!
  @file
  qcril_log.h

  @brief
  Wrapps logging macros for Android

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_log.h#6 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
06/26/09   fc      Changes in call flow log packet.
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
04/14/09   fc      Changes on debug messages logging.
04/05/09   fc      Cleanup log macros, keep MSC messages in system log, and 
                   move QCRIL messages to radio log.
01/26/08   fc      Logged assertion info.
05/21/08   tml     Moved MMGSDI/GSTK log macro to corresponding files
05/19/08   tml     added MMGSDI and GSTK logging tag


===========================================================================*/

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
#else
#include <msgcfg.h>
#include <msg.h>
#include <diag_lsm.h>
#include <diag/include/log.h>
#endif /* FEATURE_UNIT_TEST */


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* Persistent System Property that controls whether QCRIL messages to be logged to ADB */ 
#define QCRIL_LOG_ADB_ON                      "persist.radio.adb_log_on" 

/* Maximum length of log message */
#define QCRIL_MAX_LOG_MSG_SIZE                512

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

#ifndef FEATURE_UNIT_TEST
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
const char *qcril_log_lookup_event_name( int event_id );
const char *qcril_log_lookup_errno_name( int errno_id );
int qcril_log_get_token_id( RIL_Token t );
void qcril_format_log_msg( char *buf_ptr, int buf_size, char *fmt, ... );
void qcril_log_call_flow_packet( qcril_call_flow_subsystem_e_type src_subsystem, 
                                 qcril_call_flow_subsystem_e_type dest_subsystem,
                                 qcril_call_flow_arrow_e_type arrow_type, char *label );
void qcril_log_msg_to_adb( int  lvl, char *msg_ptr );
void qcril_log_msg_to_adb_main( char *msg_ptr );

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
#else
/* Log message to Diag */
#define QCRIL_LOG_MSG( lvl, fmt, ... )                                           \
  {                                                                              \
    char buf[ QCRIL_MAX_LOG_MSG_SIZE ];                                          \
                                                                                 \
    qcril_format_log_msg( buf, QCRIL_MAX_LOG_MSG_SIZE, fmt, ##__VA_ARGS__ );       \
    qcril_log_msg_to_adb( lvl, buf );                                            \
    MSG_SPRINTF_1( MSG_SSID_ANDROID_QCRIL, lvl, "%s", buf );                     \
  }

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

/* Log error level message */
#define QCRIL_LOG_ERROR( fmt, ... )   QCRIL_LOG_MSG( MSG_LEGACY_ERROR, fmt, ##__VA_ARGS__ )

/* Log fatal level message */
#define QCRIL_LOG_FATAL( fmt, ... )   QCRIL_LOG_MSG( MSG_LEGACY_FATAL, fmt, ##__VA_ARGS__ )

/* Log debug level message */
#define QCRIL_LOG_DEBUG( fmt, ... )   QCRIL_LOG_MSG( MSG_LEGACY_HIGH, fmt, ##__VA_ARGS__ )

/* Log info level message */
#define QCRIL_LOG_INFO( fmt, ...  )   QCRIL_LOG_MSG( MSG_LEGACY_MED, fmt, ##__VA_ARGS__ )

/* Log verbose level message */
#define QCRIL_LOG_VERBOSE( fmt, ... ) QCRIL_LOG_MSG( MSG_LEGACY_LOW, fmt, ##__VA_ARGS__ )

/* Log function entry message */
#define QCRIL_LOG_FUNC_ENTRY()   QCRIL_LOG_MSG( MSG_LEGACY_LOW, "function entry",0,0,0 )

/* Log function exit message */
#define QCRIL_LOG_FUNC_RETURN()  QCRIL_LOG_MSG( MSG_LEGACY_LOW, "function exit",0,0,0 )

/* Log events and callback messages from MODEM to adb main buffer */
#define QCRIL_LOG_ADB_MAIN( fmt, ... )  \
{                                       \
	char buf1[ QCRIL_MAX_LOG_MSG_SIZE ]; \
                                        \
    qcril_format_log_msg( buf1, QCRIL_MAX_LOG_MSG_SIZE, fmt, ##__VA_ARGS__ ); \
    qcril_log_msg_to_adb_main( buf1 ); \
}

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
