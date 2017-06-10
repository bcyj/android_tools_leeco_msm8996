#ifndef MMDEBUGMSG_H
#define MMDEBUGMSG_H
/*===========================================================================
                          V i d e o   W r a p p e r
                    f o r   V i d e o   D e b u g   M s g

*//** @file MMDebugMsg.h
  This file defines a methods that can be used to output debug messages

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Common/OSAbstraction/rel/1.0/inc/VideoDebugMsg.h#4 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/02/08   gkapalli    Created file.

============================================================================*/


/*===========================================================================
 Include Files
============================================================================*/
#include <stdio.h>
#ifndef LOG_TAG
#define LOG_NDEBUG 0
#define LOG_TAG "MM_OSAL"
#include <utils/Log.h>

#include "common_log.h"

#endif

//#include <Windows.h>

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Data Declarations
** ----------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */
extern int video_msg_dummy;
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */


/* -----------------------------------------------------------------------
** Macro Declarations
** ----------------------------------------------------------------------- */
/* Default master category for applications from diag*/
#define MSG_SSID_APPS                      6000
#define MSG_SSID_APPS_APPMGR               6001
#define MSG_SSID_APPS_UI                   6002
#define MSG_SSID_APPS_QTV                  6003
#define MSG_SSID_APPS_QVP                  6004
#define MSG_SSID_APPS_QVP_STATISTICS       6005
#define MSG_SSID_APPS_QVP_VENCODER         6006
#define MSG_SSID_APPS_QVP_MODEM            6007
#define MSG_SSID_APPS_QVP_UI               6008
#define MSG_SSID_APPS_QVP_STACK            6009
#define MSG_SSID_APPS_QVP_VDECODER         6010
#define MSG_SSID_APPS_ACM                  6011
#define MSG_SSID_APPS_HEAP_PROFILE         6012
#define MSG_SSID_APPS_QTV_GENERAL          6013
#define MSG_SSID_APPS_QTV_DEBUG            6014
#define MSG_SSID_APPS_QTV_STATISTICS       6015
#define MSG_SSID_APPS_QTV_UI_TASK          6016
#define MSG_SSID_APPS_QTV_MP4_PLAYER       6017
#define MSG_SSID_APPS_QTV_AUDIO_TASK       6018
#define MSG_SSID_APPS_QTV_VIDEO_TASK       6019
#define MSG_SSID_APPS_QTV_STREAMING        6020
#define MSG_SSID_APPS_QTV_MPEG4_TASK       6021
#define MSG_SSID_APPS_QTV_FILE_OPS         6022
#define MSG_SSID_APPS_QTV_RTP              6023
#define MSG_SSID_APPS_QTV_RTCP             6024
#define MSG_SSID_APPS_QTV_RTSP             6025
#define MSG_SSID_APPS_QTV_SDP_PARSE        6026
#define MSG_SSID_APPS_QTV_ATOM_PARSE       6027
#define MSG_SSID_APPS_QTV_TEXT_TASK        6028
#define MSG_SSID_APPS_QTV_DEC_DSP_IF       6029
#define MSG_SSID_APPS_QTV_STREAM_RECORDING 6030
#define MSG_SSID_APPS_QTV_CONFIGURATION    6031
#define MSG_SSID_APPS_FILE_GENERAL         6032
#define MSG_SSID_APPS_QSM                  6033
#define MSG_SSID_APPS_QTA                  6034
#define MSG_SSID_APPS_ALL                  6035
#define MSG_SSID_APPS_LAST                 6036

// Video Message Apps
#define VIDEO_GENERAL           (MSG_SSID_APPS_QTV_GENERAL)
#define VIDEO_DEBUG             (MSG_SSID_APPS_QTV_DEBUG)
#define VIDEO_STATISTICS        (MSG_SSID_APPS_QTV_STATISTICS)
#define VIDEO_UI_TASK           (MSG_SSID_APPS_QTV_UI_TASK)
#define VIDEO_MP4_PLAYER        (MSG_SSID_APPS_QTV_MP4_PLAYER)
#define VIDEO_AUDIO_TASK        (MSG_SSID_APPS_QTV_AUDIO_TASK)
#define VIDEO_VIDEO_TASK        (MSG_SSID_APPS_QTV_VIDEO_TASK)
#define VIDEO_STREAMING         (MSG_SSID_APPS_QTV_STREAMING)
#define VIDEO_HTTP_STREAMING    (MSG_SSID_APPS_QTV_STREAMING)
#define VIDEO_MPEG4_TASK        (MSG_SSID_APPS_QTV_MPEG4_TASK)
#define VIDEO_FILE_OPS          (MSG_SSID_APPS_QTV_FILE_OPS)
#define VIDEO_HTTP_STACK        (MSG_SSID_APPS_QTV_RTP)
#define VIDEO_RTP               (MSG_SSID_APPS_QTV_RTP)
#define VIDEO_RTCP              (MSG_SSID_APPS_QTV_RTCP)
#define VIDEO_RTSP              (MSG_SSID_APPS_QTV_RTSP)
#define VIDEO_SDP_PARSE         (MSG_SSID_APPS_QTV_SDP_PARSE)
#define VIDEO_ATOM_PARSE        (MSG_SSID_APPS_QTV_ATOM_PARSE)
#define VIDEO_TEXT_TASK         (MSG_SSID_APPS_QTV_TEXT_TASK)
#define VIDEO_DEC_DSP_IF        (MSG_SSID_APPS_QTV_DEC_DSP_IF)
#define VIDEO_STREAM_RECORDING  (MSG_SSID_APPS_QTV_STREAM_RECORDING)
#define VIDEO_CONFIGURATION     (MSG_SSID_APPS_QTV_CONFIGURATION)
#define VIDEO_BCAST_FLO         (MSG_SSID_APPS_QTV_BCAST_FLO)

#define MM_GENERAL           (MSG_SSID_APPS_QTV_GENERAL)
#define MM_DEBUG             (MSG_SSID_APPS_QTV_DEBUG)
#define MM_STATISTICS        (MSG_SSID_APPS_QTV_STATISTICS)
#define MM_UI_TASK           (MSG_SSID_APPS_QTV_UI_TASK)
#define MM_MP4_PLAYER        (MSG_SSID_APPS_QTV_MP4_PLAYER)
#define MM_AUDIO_TASK        (MSG_SSID_APPS_QTV_AUDIO_TASK)
#define MM_VIDEO_TASK        (MSG_SSID_APPS_QTV_VIDEO_TASK)
#define MM_STREAMING         (MSG_SSID_APPS_QTV_STREAMING)
#define MM_HTTP_STREAMING    (MSG_SSID_APPS_QTV_STREAMING)
#define MM_MPEG4_TASK        (MSG_SSID_APPS_QTV_MPEG4_TASK)
#define MM_FILE_OPS          (MSG_SSID_APPS_QTV_FILE_OPS)
#define MM_HTTP_STACK        (MSG_SSID_APPS_QTV_RTP)
#define MM_RTP               (MSG_SSID_APPS_QTV_RTP)
#define MM_RTCP              (MSG_SSID_APPS_QTV_RTCP)
#define MM_RTSP              (MSG_SSID_APPS_QTV_RTSP)
#define MM_SDP_PARSE         (MSG_SSID_APPS_QTV_SDP_PARSE)
#define MM_ATOM_PARSE        (MSG_SSID_APPS_QTV_ATOM_PARSE)
#define MM_TEXT_TASK         (MSG_SSID_APPS_QTV_TEXT_TASK)
#define MM_DEC_DSP_IF        (MSG_SSID_APPS_QTV_DEC_DSP_IF)
#define MM_STREAM_RECORDING  (MSG_SSID_APPS_QTV_STREAM_RECORDING)
#define MM_CONFIGURATION     (MSG_SSID_APPS_QTV_CONFIGURATION)
#define MM_BCAST_FLO         (MSG_SSID_APPS_QTV_BCAST_FLO)
#define MM_FILE_GENERAL      (MSG_SSID_APPS_FILE_GENERAL)
#define MM_QSM               (MSG_SSID_APPS_QSM)
#define MM_QTA               (MSG_SSID_APPS_QTA)
#define MM_ALL               (MSG_SSID_APPS_ALL)

// MM Message Priorities
#ifndef MM_PRIO_LOW
#define MM_PRIO_LOW      1
#endif
#ifndef MM_PRIO_MEDIUM
#define MM_PRIO_MEDIUM   2
#endif
#ifndef MM_PRIO_HIGH
#define MM_PRIO_HIGH     4
#endif
#ifndef MM_PRIO_ERROR
#define MM_PRIO_ERROR    8
#endif
#ifndef MM_PRIO_FATAL
#define MM_PRIO_FATAL    16
#endif
#ifndef MM_PRIO_DEBUG
#define MM_PRIO_DEBUG    32
#endif

// Log Pkt Codes Apps
#define LOG_1X_BASE_C                                   ((uint16) 0x1000)

/* (0x173+LOG_1X_BASE_C) is the last reserved log code for Qtv log code space1.
    The following is the allocated second log code space for Qtv (432-480 Qtv reserved logs)
 #define LOG_QTV2_RESERVED_CODES_BASE_C                 (0x1B0 + LOG_1X_BASE_C)
 #define LOG_QTV2_LAST_C                  (48 + LOG_QTV2_RESERVED_CODES_BASE_C)
*/

#define LOG_QTV_PLAYER_TIMED_TEXT_C                    (0x1B1 + LOG_1X_BASE_C)
#define LOG_QTV_FRAME_DECODE_C                         (0x1B2 + LOG_1X_BASE_C)
#define LOG_QTV_FRAME_RENDER_C                         (0x1B3 + LOG_1X_BASE_C)
#define LOG_QTV_AV_SYNC_C                              (0x1B4 + LOG_1X_BASE_C)
#define LOG_QTV_PDS2_STATS                             (0x1B6 + LOG_1X_BASE_C)
#define LOG_QTV_PDS2_GET_REQUEST                       (0x1B7 + LOG_1X_BASE_C)
#define LOG_QTV_PDS2_GET_RESP_HEADER                   (0x1B8 + LOG_1X_BASE_C)
#define LOG_QTV_PDS2_GET_RESP_PCKT                     (0x1B9 + LOG_1X_BASE_C) //deprecated

#define LOG_QTV_CMX_AUDIO_DATA_C                       (0x1BA + LOG_1X_BASE_C)
#define LOG_QTV_RTSP_OPTIONS_C                         (0x1BB + LOG_1X_BASE_C)
#define LOG_QTV_RTSP_GET_PARAMETER_C                   (0x1BC + LOG_1X_BASE_C)
#define LOG_QTV_RTSP_SET_PARAMETER_C                   (0x1BD + LOG_1X_BASE_C)
#define LOG_ARM_VIDEO_DECODE_STATS                     (0x1BF + LOG_1X_BASE_C)
#define LOG_QTV_CMD_LOGGING_C                          (0x1C1 + LOG_1X_BASE_C)

#define LOG_QTV_AUDIO_FRAME_PTS_INFO_C                 (0x1C2 + LOG_1X_BASE_C)
#define LOG_QTV_VIDEO_FRAME_DECODE_INFO_C              (0x1C3 + LOG_1X_BASE_C)
#define LOG_QTV_RTCP_COMPOUND_RR_C                     (0x1C4 + LOG_1X_BASE_C)
#define LOG_QTV_FRAME_BUFFER_RELEASE_REASON_C          (0x1C5 + LOG_1X_BASE_C)
#define LOG_QTV_AUDIO_CHANNEL_SWITCH_FRAME_C           (0x1C6 + LOG_1X_BASE_C)
/*
typedef struct
{
  unsigned char header[12]; //log header is 12 bytes long
} log_hdr_type;
*/

typedef struct
{
  unsigned short len;
  unsigned short code;
  unsigned long long ts;
} mm_log_hdr_type;


#define  log_hdr_type  mm_log_hdr_type


// Event codes
typedef enum
{
  EVENT_DROP_ID = 0,

  EVENT_QTV_CLIP_STARTED = 904,              /* 7 byte payload */
  EVENT_QTV_CLIP_ENDED,                      /* 5 byte payload */
  EVENT_QTV_SDP_PARSER_REJECT,               /* No payload */
  EVENT_QTV_CLIP_PAUSE,                      /* 4 byte payload */
  EVENT_QTV_CLIP_REPOSITIONING,              /* 4 byte payload */
  EVENT_QTV_CLIP_ZOOM_IN,                    /* No payload */
  EVENT_QTV_CLIP_ZOOM_OUT,                   /* No payload */
  EVENT_QTV_CLIP_ROTATE,                     /* 4 byte payload */
  EVENT_QTV_CLIP_PAUSE_RESUME,               /* 4 byte payload */
  EVENT_QTV_CLIP_REPOSITION_RESUME,          /* 4 byte payload */
  EVENT_QTV_DSP_INIT,                        /* No payload */
  EVENT_QTV_STREAMING_SERVER_URL,            /* 22 byte payload */
  EVENT_QTV_SERVER_PORTS_USED,               /* 4 byte payload */
  EVENT_QTV_USING_PROXY_SERVER,              /* 6 byte payload */
  EVENT_QTV_STREAMER_STATE_IDLE,             /* No payload */
  EVENT_QTV_STREAMER_STATE_CONNECTING,       /* No payload */
  EVENT_QTV_STREAMER_STATE_CONNECTED,        /* No payload */
  EVENT_QTV_STREAMER_STATE_SETTING_TRACKS,   /* No payload */
  EVENT_QTV_STREAMER_STATE_STREAMING,        /* No payload */
  EVENT_QTV_STREAMER_STATE_PAUSED,           /* No payload */
  EVENT_QTV_STREAMER_STATE_SUSPENDED,        /* No payload */
  EVENT_QTV_STREAMER_STATE_CLOSING,          /* Np payload */
  EVENT_QTV_STREAMER_CONNECTED,              /* No payload */
  EVENT_QTV_STREAMER_INITSTREAM_FAIL,        /* No payload */
  EVENT_QTV_BUFFERING_STARTED,               /* 5 byte payload */
  EVENT_QTV_BUFFERING_ENDED,                 /* 5 byte payload */
  EVENT_QTV_CLIP_FULLSCREEN,                 /* No payload */
  EVENT_QTV_PS_DOWNLOAD_STARTED,             /* 8 byte payload */
  EVENT_QTV_PSEUDO_STREAM_STARTED,           /* No Payload */
  EVENT_QTV_PS_PLAYER_STATE_PSEUDO_PAUSE,    /* No payload */
  EVENT_QTV_PS_PLAYER_STATE_PSEUDO_RESUME,   /* 4 byte payload */
  EVENT_QTV_PARSER_STATE_READY,              /* 14 byte payload */
  EVENT_QTV_FRAGMENT_PLAYBACK_BEGIN,         /* 2 byte payload */
  EVENT_QTV_FRAGMENT_PLAYBACK_COMPLETE,      /* 2 byte payload */
  EVENT_QTV_PARSER_STATE_PSEUDO_PAUSE,       /* No payload */
  EVENT_QTV_PLAYER_STATE_PSEUDO_PAUSE,       /* No payload */
  EVENT_QTV_PARSER_STATE_PSEUDO_RESUME,      /* 4 byte payload */
  EVENT_QTV_PLAYER_STATE_PSEUDO_RESUME,      /* 4 byte payload */
  EVENT_QTV_FRAGMENTED_FILE_DECODE_START,    /* 2 byte payload */
  EVENT_QTV_FRAGMENTED_FILE_END_SUCCESS,     /* 2 byte payload */
  EVENT_QTV_DOWNLOAD_DATA_REPORT,            /* 4 byte payload */
  EVENT_QTV_VDEC_DIAG_DECODE_CALLBACK,       /* 5 byte payload */
  EVENT_QTV_URL_PLAYED_IS_MULTICAST,         /* No payload */
  EVENT_QTV_VDEC_DIAG_STATUS,                /* 4 byte payload */
  EVENT_QTV_STREAMING_URL_OPEN,              /* 4 byte payload */
  EVENT_QTV_STREAMING_URL_OPENING,           /* No payload */
  EVENT_QTV_CLIP_ENDED_VER2,                 /* 13 byte payload */
  EVENT_QTV_SILENCE_INSERTION_STARTED,       /* No payload */
  EVENT_QTV_SILENCE_INSERTION_ENDED,         /* 8 byte payload */
  EVENT_QTV_AUDIO_CHANNEL_SWITCH_FRAME,      /* 8 byte payload */
  EVENT_QTV_FIRST_VIDEO_FRAME_RENDERED,      /* No payload */
  EVENT_QTV_FIRST_VIDEO_I_FRAME_RENDERED,    /* No payload */
  EVENT_QTV_SDP_SELECTED,                    /* No payload */
  EVENT_QTV_DIAG_PLAYER_STATUS,              /* 12 byte payload */
  EVENT_QTV_SILENCE_INSERTION_DURATION,      /* 4 byte payload */
  EVENT_QTV_UNDEFINED_957,
  EVENT_QTV_UNDEFINED_958,
  EVENT_QTV_UNDEFINED_959,
  EVENT_QTV_UNDEFINED_960,
  EVENT_QTV_UNDEFINED_961,
  EVENT_QTV_UNDEFINED_962,
  EVENT_QTV_UNDEFINED_963,
  EVENT_QTV_UNDEFINED_964,
  EVENT_QTV_UNDEFINED_965,
  EVENT_QTV_UNDEFINED_966,
  EVENT_QTV_UNDEFINED_967,

  EVENT_RSVD_START = 0x0800,
  EVENT_RSVD_END   = 0x083F,
  EVENT_LAST_ID    = 0x083F,

  EVENT_MAX_ID     = 0x0FFF
} event_id_enum_type;

/*---------------------------------------------------------------------------
  This macro asserts an expression and produces a DebugBreak if the
  expression is FALSE.
---------------------------------------------------------------------------*/
#ifdef _DEBUG
#define VIDEO_ASSERT(exp) ((exp)?1:DebugBreak())
#else
#define VIDEO_ASSERT(exp)
#endif


/*---------------------------------------------------------------------------
  MM_Log_Alloc(logPktCode, logPktSize)
  This is the macro for allocating log packet memory (from diag).
---------------------------------------------------------------------------*/
#define MM_Log_Alloc(logPktCode, logPktSize) NULL

/*---------------------------------------------------------------------------
  MM_Log_Commit(logPkt)
  This is the macro for committing log packet (to diag).
---------------------------------------------------------------------------*/
#define MM_Log_Commit(logPkt)

/*---------------------------------------------------------------------------
  MM_Event_Report(eventID, eventPayloadSize, eventPayload)
  This is the macro for reporting an event with/without payload (to diag).
---------------------------------------------------------------------------*/
#define MM_Event_Report(eventID, eventPayloadSize, eventPayload) NULL

/////////////////////////////////////////////////////////////////////////////////
#define VDEC_LOG_DEBUG 1
#define VDEC_LOG_HIGH 1
#define VDEC_LOG_ERROR 1
    #ifdef VDEC_LOG_DEBUG    // should be LOGI but LOGI is suppressed in some builds so keeping LOGW
        #define LOG_MM_PRIO_LOW(a...)        LOGE(a)
        #define LOG_MM_PRIO_MEDIUM(a...)        LOGE(a)
        #define LOG_MM_PRIO_DEBUG(a...)        LOGE(a)
        #define DEBUG_PRINT(a...)        LOGE(a)
    #else
        #define LOG_MM_PRIO_LOW(a...)        while(0) {}
        #define LOG_MM_PRIO_MEDIUM(a...)        while(0) {}
        #define LOG_MM_PRIO_DEBUG(a...)        while(0) {}
        #define DEBUG_PRINT(a...)        while(0) {}
    #endif

    #ifdef VDEC_LOG_HIGH
        #define LOG_MM_PRIO_HIGH(a...)        LOGE(a)
    #else
        #define LOG_MM_PRIO_HIGH(a...)        while(0) {}
    #endif

    #ifdef VDEC_LOG_ERROR
        #define LOG_MM_PRIO_ERROR(a...)        LOGE(a)
        #define LOG_MM_PRIO_FATAL(a...)        LOGE(a)
        #define DEBUG_PRINT_ERROR(a...)        LOGE(a)
    #else
        #define LOG_MM_PRIO_ERROR(a...)        while(0) {}
        #define LOG_MM_PRIO_FATAL(a...)        while(0) {}
        #define DEBUG_PRINT_ERROR(a...)        while(0) {}
    #endif

    #ifdef PROFILE_DECODER       //should be LOGI but keeping LOGE
        #define QTV_PERF(a)
        #define QTV_PERF_MSG_PRIO(a,b,c...)     LOGW(c)
        #define QTV_PERF_MSG_PRIO1(a,b,c...)    LOGW(c)
        #define QTV_PERF_MSG_PRIO2(a,b,c...)    LOGW(c)
        #define QTV_PERF_MSG_PRIO3(a,b,c...)    LOGW(c)
    #else
        #define QTV_PERF(a)
        #define QTV_PERF_MSG_PRIO(a,b,c)        while(0){}
        #define QTV_PERF_MSG_PRIO1(a,b,c,d)        while(0){}
        #define QTV_PERF_MSG_PRIO2(a,b,c,d,e)        while(0){}
        #define QTV_PERF_MSG_PRIO3(a,b,c,d,e,f)        while(0){}
    #endif


/////////////////////////////////////////////////////////////////////////////////

#define MM_MSG_PRIO(a,b,c)                   (((b & GetLogMask(a)))?(LOGE(c)), 1 : 0)
#define MM_MSG_PRIO1(a,b,c,d)                (((b & GetLogMask(a)))?(LOGE((c),(d))), 1 : 0)
#define MM_MSG_PRIO2(a,b,c,d,e)              (((b & GetLogMask(a)))?(LOGE((c),(d),(e))), 1 : 0)
#define MM_MSG_PRIO3(a,b,c,d,e,f)            (((b & GetLogMask(a)))?(LOGE((c),(d),(e),(f))), 1 : 0)
#define MM_MSG_PRIO4(a,b,c,d,e,f,g)          (((b & GetLogMask(a)))?(LOGE((c),(d),(e),(f),(g))), 1 : 0)
#define MM_MSG_PRIO5(a,b,c,d,e,f,g,h)        (((b & GetLogMask(a)))?(LOGE((c),(d),(e),(f),(g),(h))), 1 : 0)
#define MM_MSG_PRIO6(a,b,c,d,e,f,g,h,i)      (((b & GetLogMask(a)))?(LOGE((c),(d),(e),(f),(g),(h),(i))), 1 : 0)
#define MM_MSG_PRIO7(a,b,c,d,e,f,g,h,i,j)    (((b & GetLogMask(a)))?(LOGE((c),(d),(e),(f),(g),(h),(i),(j))), 1 : 0)
#define MM_MSG_PRIO8(a,b,c,d,e,f,g,h,i,j,k)  (((b & GetLogMask(a)))?(LOGE((c),(d),(e),(f),(g),(h),(i),(j),(k))), 1 : 0)

#define ERR(a, b, c, d)        while(0){}
#define ERR_FATAL(a, b, c, d)        while(0){}

//#define MSG_ERROR(a, b)            while(0){}
#define MSG_ERROR(a, b, c, d)        while(0){}
#define MSG_HIGH(a, b, c, d)         (MM_PRIO_HIGH & GetLogMask(MM_ALL))?(LOGE((a),(b),(c),(d)))
#define MSG_LOW(a, b, c, d)          (MM_PRIO_LOW & GetLogMask(MM_ALL))?(LOGV((a),(b),(c),(d)))
#define MSG_MED(a, b, c, d)          (MM_PRIO_MED & GetLogMask(MM_ALL))?(LOGV((a),(b),(c),(d)))

#define VIDEO_MSG(a,b)                   while(0) {}
#define VIDEO_MSG1(a,b,c)                while(0) {}
#define VIDEO_MSG2(a,b,c,d)              while(0) {}
#define VIDEO_MSG3(a,b,c,d,e)            while(0) {}
#define VIDEO_MSG4(a,b,c,d,e,f)          while(0) {}


#define MM_MSG(a,b)                   MM_MSG_PRIO(a,MM_PRIO_MEDIUM,b)
#define MM_MSG1(a,b,c)                MM_MSG_PRIO1(a,MM_PRIO_MEDIUM,b,c)
#define MM_MSG2(a,b,c,d)              MM_MSG_PRIO2(a,MM_PRIO_MEDIUM,b,c,d)
#define MM_MSG3(a,b,c,d,e)            MM_MSG_PRIO3(a,MM_PRIO_MEDIUM,b,c,d,e)
#define MM_MSG4(a,b,c,d,e,f)          MM_MSG_PRIO4(a,MM_PRIO_MEDIUM,b,c,d,e,f)

#define MM_MSG_SPRINTF_PRIO(a,b,c)           MM_MSG_PRIO(a,b,c)
#define MM_MSG_SPRINTF_PRIO_1(a,b,c,d)       MM_MSG_PRIO1(a,b,c,d)
#define MM_MSG_SPRINTF_PRIO_2(a,b,c,d,e)     MM_MSG_PRIO2(a,b,c,d,e)

#define MM_MSG_SPRINTF_1(a,b,c)       while(0) {}
#define MM_MSG_SPRINTF_2(a,b,c,d)       while(0) {}
#define MM_MSG_SPRINTF_3(a,b,c,d,e)       while(0) {}
#define MM_MSG_SPRINTF_4(a,b,c,d,e,f)       while(0) {}



/* =======================================================================
**                        Class & Function Definations
** ======================================================================= */

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
 * Initializes the Diag Interface
 *
 * @return 0 value on success else failure
 */
int MM_Debug_Initialize();

/**
 * De-Initializez the Diag Interface
 *
 * @return 0 value on success else failure
 */
int MM_Debug_Deinitialize();

/**
 * Get the log mask for the specified system ID from the config file
 *
 * @return log mask
 */
int GetLogMask(const unsigned int nSysID);



#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */


#endif//MMDEBUGMSG_H


