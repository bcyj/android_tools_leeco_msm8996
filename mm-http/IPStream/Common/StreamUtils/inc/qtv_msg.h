#ifndef QTV_OPEN_MM_DBG_MESSAGES
#define QTV_OPEN_MM_DBG_MESSAGES

/************************************************************************* */
/**
 * qtv_msg.h
 *
 * @brief Debug Interface for IPStream modules
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/qtv_msg.h#11 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
#include <MMDebugMsg.h>
#include <AEEstd.h>

#define QTVDIAG_GENERAL           MM_GENERAL
#define QTVDIAG_DEBUG             MM_DEBUG
#define QTVDIAG_STATISTICS        MM_STATISTICS
#define QTVDIAG_UI_TASK           MM_UI_TASK
#define QTVDIAG_MP4_PLAYER        MM_MP4_PLAYER
#define QTVDIAG_AUDIO_TASK        MM_AUDIO_TASK
#define QTVDIAG_VIDEO_TASK        MM_VIDEO_TASK
#define QTVDIAG_STREAMING         MM_STREAMING
#define QTVDIAG_HTTP_STREAMING    MM_HTTP_STREAMING
#define QTVDIAG_MPEG4_TASK        MM_MPEG4_TASK
#define QTVDIAG_FILE_OPS          MM_FILE_OPS
#define QTVDIAG_HTTP_STACK        MM_HTTP_STACK
#define QTVDIAG_RTP               MM_RTP
#define QTVDIAG_RTCP              MM_RTCP
#define QTVDIAG_RTSP              MM_RTSP
#define QTVDIAG_SDP_PARSE         MM_SDP_PARSE
#define QTVDIAG_ATOM_PARSE        MM_ATOM_PARSE
#define QTVDIAG_TEXT_TASK         MM_TEXT_TASK
#define QTVDIAG_DEC_DSP_IF        MM_DEC_DSP_IF
#define QTVDIAG_STREAM_RECORDING  MM_STREAM_RECORDING
#define QTVDIAG_CONFIGURATION     MM_CONFIGURATION
#define QTVDIAG_BCAST_FLO         MM_BCAST_FLO

/* QTV Message Priorities */
#define QTVDIAG_PRIO_LOW      MM_PRIO_LOW
#define QTVDIAG_PRIO_MEDIUM   MM_PRIO_MEDIUM
#define QTVDIAG_PRIO_MED      MM_PRIO_MEDIUM
#define QTVDIAG_PRIO_HIGH     MM_PRIO_HIGH
#define QTVDIAG_PRIO_ERROR    MM_PRIO_ERROR
#define QTVDIAG_PRIO_FATAL    MM_PRIO_FATAL
#define QTVDIAG_PRIO_DEBUG    MM_PRIO_DEBUG

#ifndef log_hdr_type
#define log_hdr_type mm_log_hdr_type
#endif

#ifndef LOG_1X_BASE_C
#define LOG_1X_BASE_C MM_LOG1X_BASE_C
#endif

/*---------------------------------------------------------------------------
  This is the macro for messages with no parameters  and a default priority.
---------------------------------------------------------------------------*/
#define QTV_MSG(xx_ss_mask, xx_fmt) MM_MSG(xx_ss_mask, xx_fmt)

/*---------------------------------------------------------------------------
  This is the macro for messages with no parameters and a specific priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO(xx_ss_mask, xx_prio, xx_fmt) \
          MM_MSG_PRIO(xx_ss_mask, xx_prio, xx_fmt)
/*---------------------------------------------------------------------------
  Macro for messages with 1 parameter  and a default priority
---------------------------------------------------------------------------*/
#define QTV_MSG1(xx_ss_mask, xx_fmt, xx_arg1) \
          MM_MSG1(xx_ss_mask, xx_fmt, xx_arg1)

/*---------------------------------------------------------------------------
  This is the macro for messages with 1 parameters and a specific priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO1(xx_ss_mask, xx_prio, xx_fmt, xx_arg1) \
          MM_MSG_PRIO1(xx_ss_mask, xx_prio, xx_fmt, xx_arg1)

/*---------------------------------------------------------------------------
  Macro for messages with 2 parameters  and a default priority
---------------------------------------------------------------------------*/
#define QTV_MSG2(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2) \
          MM_MSG2(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2)
/*---------------------------------------------------------------------------
  This is the macro for messages with 2 parameters and a specific priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO2(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2) \
          MM_MSG_PRIO2(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2)

/*---------------------------------------------------------------------------
  This is the macro for messages with 3 parameters  and a default priority
---------------------------------------------------------------------------*/
#define QTV_MSG3(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
          MM_MSG3(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3)

/*---------------------------------------------------------------------------
  This is the macro for messages with 3 parameters and a specific priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO3(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, xx_arg3) \
          MM_MSG_PRIO3(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, xx_arg3)

/*---------------------------------------------------------------------------
  This is the macro for messages with 4 parameters  and a default priority. In
  this case the function called needs to have more than 4 parameters so it is
  going to be a slow function call.  So for this case the  msg_send_var() uses
  var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG4(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4) \
          MM_MSG4(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4)

/*---------------------------------------------------------------------------
  This is the macro for messages with 4 parameters and a specific priority.
  msg_send_var() uses var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO4(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, xx_arg3, \
                      xx_arg4) \
          MM_MSG_PRIO4(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, \
                          xx_arg3, xx_arg4)

/*---------------------------------------------------------------------------
  This is the macro for messages with 5 parameters  and a default priority.
  msg_send_var() uses var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG5(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, \
                 xx_arg5) \
          MM_MSG5(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, \
                     xx_arg5)

/*---------------------------------------------------------------------------
  This is the macro for messages with 5 parameters and a specific priority.
  msg_send_var() uses var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO5(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, xx_arg3, \
                      xx_arg4, xx_arg5) \
          MM_MSG_PRIO5(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, \
                          xx_arg3, xx_arg4, xx_arg5)

/*---------------------------------------------------------------------------
  This is the macro for messages with 6 parameters  and a default priority.
  msg_send_var() uses var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG6(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, \
                                            xx_arg4, xx_arg5, xx_arg6) \
          MM_MSG6(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, \
                     xx_arg4, xx_arg5, xx_arg6)

/*---------------------------------------------------------------------------
  This is the macro for messages with 6 parameters and a specific priority.
  msg_send_var() uses var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO6(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, xx_arg3, \
                      xx_arg4, xx_arg5, xx_arg6) \
          MM_MSG_PRIO6(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, \
                          xx_arg3, xx_arg4, xx_arg5, xx_arg6)
/*---------------------------------------------------------------------------
  This is the macro for messages with 7 parameters  and a default priority.
  msg_send_var() uses var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG7(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, \
                 xx_arg5, xx_arg6, xx_arg7) \
          MM_MSG7(xx_ss_mask, xx_fmt, xx_arg1, xx_arg2, xx_arg3, xx_arg4, \
                     xx_arg5, xx_arg6, xx_arg7)

/*---------------------------------------------------------------------------
  This is the macro for messages with 7 parameters and a specific priority.
  msg_send_var() uses var arg list supported by the compiler.
---------------------------------------------------------------------------*/
#define QTV_MSG_PRIO7(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, xx_arg3, \
                      xx_arg4, xx_arg5, xx_arg6, xx_arg7) \
           MM_MSG_PRIO7(xx_ss_mask, xx_prio, xx_fmt, xx_arg1, xx_arg2, \
                      xx_arg3, xx_arg4, xx_arg5, xx_arg6, xx_arg7)

/*---------------------------------------------------------------------------
  This is the macro for messages with 1 string argumant  and a default priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_SPRINTF_1(xx_ss_mask,xx_fmt,xx_arg1) \
          MM_MSG_SPRINTF_1(xx_ss_mask,xx_fmt,xx_arg1)

/*---------------------------------------------------------------------------
  This is the macro for messages with 2 string arguments  and a default priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_SPRINTF_2(xx_ss_mask,xx_fmt,xx_arg1,arg2) \
          MM_MSG_SPRINTF_2(xx_ss_mask,xx_fmt,xx_arg1,arg2)

/*---------------------------------------------------------------------------
  This is the macro for messages with 3 string arguments  and a default priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_SPRINTF_3(xx_ss_mask,xx_fmt,xx_arg1,arg2,arg3) \
          MM_MSG_SPRINTF_3(xx_ss_mask,xx_fmt,xx_arg1,arg2,arg3)

/*---------------------------------------------------------------------------
  This is the macro for messages with 1 string arguments and a specific priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_SPRINTF_PRIO_1( xx_ss_mask, xx_prio,xx_fmt,arg1) \
          MM_MSG_SPRINTF_PRIO_1( xx_ss_mask, xx_prio,xx_fmt,arg1)

/*---------------------------------------------------------------------------
  This is the macro for messages with 2 string argumant and a specific priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_SPRINTF_PRIO_2( xx_ss_mask, xx_prio,xx_fmt,arg1,arg2) \
          MM_MSG_SPRINTF_PRIO_2( xx_ss_mask, xx_prio,xx_fmt,arg1,arg2)

/*---------------------------------------------------------------------------
  This is the macro for messages with 3 string argumant and a specific priority.
---------------------------------------------------------------------------*/
#define QTV_MSG_SPRINTF_PRIO_3( xx_ss_mask, xx_prio,xx_fmt,arg1,arg2,arg3) \
          MM_MSG_SPRINTF_PRIO_3( xx_ss_mask, xx_prio,xx_fmt,arg1,arg2,arg3)

#ifndef ASSERT
#if defined(_DEBUG)
#define ASSERT(x) ((x) || (DebugBreak(), FALSE))
#else
#define ASSERT(x) while(0)
#endif //_DEBUG
#endif // ASSERT

#endif//QTV_OPEN_MM_DBG_MESSAGES
