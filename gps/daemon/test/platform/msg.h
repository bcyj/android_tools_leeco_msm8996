/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef MSG_H
#define MSG_H

/* enable to debug call */
/* #define LOG_DEBUG_CALL */

#ifndef LOG_DEBUG_CALL
#define MSG_LOW( str, a, b, c )
#define MSG_MED( str, a, b, c )
#define MSG_ERROR( str, a, b, c )
#define MSG_HIGH( str, a, b, c )
#define ERR_FATAL( str, a, b, c )
#else
#define MSG_LOW( str, a, b, c ) fprintf(stderr, "%s:%d] %s\n", __func__, __LINE__, str)
#define MSG_MED( str, a, b, c ) fprintf(stderr, "%s:%d] %s\n", __func__, __LINE__, str)
#define MSG_ERROR( str, a, b, c ) fprintf(stderr, "%s:%d] %s\n", __func__, __LINE__, str)
#define MSG_HIGH( str, a, b, c ) fprintf(stderr, "%s:%d] %s\n", __func__, __LINE__, str)
#define ERR_FATAL( str, a, b, c ) fprintf(stderr, "%s:%d] %s\n", __func__, __LINE__, str)
#endif

#endif /* MSG_H */
