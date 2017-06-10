/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MERCURY_DBG_H__
#define __MERCURY_DBG_H__

#define LOG_DEBUG
#define _ANDROID_

#undef MCR_DBG
#undef MCR_PR_ERR

#ifndef LOG_DEBUG
    #ifdef _ANDROID_
        #define LOG_NIDEBUG 0
        #define LOG_TAG "mercury"
        #include <utils/Log.h>
        #define MCR_DBG(fmt, args...) do{}while(0)
	#ifdef NEW_LOG_API
            #define MCR_PR_ERR(fmt, args...) ALOGE(fmt, ##args)
	#else
            #define MCR_PR_ERR(fmt, args...) LOGE(fmt, ##args)
	#endif
    #else
        #define MCR_DBG(fmt, args...) do{}while(0)
        #define MCR_PR_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #endif
#else
    #ifdef _ANDROID_
        #define LOG_NIDEBUG 0
        #define LOG_TAG "mercury"
        #include <utils/Log.h>
	#ifdef NEW_LOG_API
            #define MCR_DBG(fmt, args...) ALOGE(fmt, ##args)
            #define MCR_PR_ERR(fmt, args...) ALOGE(fmt, ##args)
	#else
            #define MCR_DBG(fmt, args...) LOGE(fmt, ##args)
            #define MCR_PR_ERR(fmt, args...) LOGE(fmt, ##args)
	#endif
    #else
        #define MCR_DBG(fmt, args...) do{}while(0)
        #define MCR_DBG(fmt, args...) fprintf(fmt, ##args)
        #define MCR_PR_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
    #endif
#endif

#endif /* __MERCURY_DBG_H__ */
