/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef MMCAM_LOG_UTILS_H
#define MMCAM_LOG_UTILS_H

#include <assert.h>
#include <unistd.h>
#include <string>
#include <stdio.h>

namespace mmcam_utils
{

#undef MMCAM_LOGE
#undef MMCAM_LOGW
#undef MMCAM_LOGI
#undef MMCAM_LOGV
#undef MMCAM_LOGB
#undef MMCAM_ASSERT
#undef MMCAM_ASSERT_FAILED
#undef MMCAM_ASSERT_PRE
#undef MMCAM_ASSERT_POST
#undef MMCAM_ASSERT_INVARIANT

#ifdef _ANDROID_

#undef LOG_NIDEBUG
#undef LOG_TAG
#define LOG_NIDEBUG 0
#define LOG_TAG "mm-camera-eztune"

#define MMCAM_LOGE(fmt, ...) ALOGE("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGW(fmt, ...) ALOGW("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGI(fmt, ...) ALOGI("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGV(fmt, ...) ALOGV("%s(%d): " #fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGB(fmt, ...) ALOGV(fmt ,##__VA_ARGS__)

#else

#define KMAG  "\x1B[35m"
#define KRED  "\x1B[31m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KWHT  "\x1B[37m"

#define MMCAM_LOGE(fmt, ...) printf("%s%s(%d): " #fmt "\n", KRED, __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGW(fmt, ...) printf("%s%s(%d): " #fmt "\n", KMAG, __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGI(fmt, ...) printf("%s%s(%d): " #fmt "\n", KYEL, __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGV(fmt, ...) printf("%s%s(%d): " #fmt "\n", KBLU, __FILE__, __LINE__, ##__VA_ARGS__)
#define MMCAM_LOGB(fmt, ...) printf("%s" fmt, KWHT, ##__VA_ARGS__)

#endif //_ANDROID_

//Uncomment required lines below to reduce log level
//#undef MMCAM_LOGI
//#undef MMCAM_LOGV
//#undef MMCAM_LOGB
//#define MMCAM_LOGI(fmt, args...) do{}while(0)
//#define MMCAM_LOGV(fmt, args...) do{}while(0)
//#define MMCAM_LOGB(fmt, args...) do{}while(0)

#define MMCAM_ASSERT(TST, fmt, ...) do { \
        if(!(TST)) { \
            MMCAM_LOGE(Assert Failed: #fmt, ##__VA_ARGS__); \
            abort(); \
        } \
    } while(0)

#define MMCAM_ASSERT_PRE(TST, fmt, ...) MMCAM_ASSERT (TST, fmt, ##__VA_ARGS__)

#ifndef NDEBUG
#define MMCAM_ASSERT_POST(TST, fmt, ...) MMCAM_ASSERT (TST, fmt, ##__VA_ARGS__)
#define MMCAM_ASSERT_INVARIANT(TST, fmt, ...) MMCAM_ASSERT (TST, fmt, ##__VA_ARGS__)
#else
#define MMCAM_ASSERT_POST(TST, ...) ((void)0)
#define MMCAM_ASSERT_INVARIANT(TST, ...) ((void)0)
#endif

void PrintByteStream(std::string s, size_t size, std::string comment);

};

#endif // MMCAM_LOG_UTILS_H
