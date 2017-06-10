/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifdef _ANDROID_
#include <log/log.h>
#endif

#include "mmcam_log_utils.h"

namespace mmcam_utils
{

void PrintByteStream(std::string s, size_t size, std::string comment)
{
    const char *val = s.data();

    MMCAM_LOGV("%s Size: %zu", comment.c_str(), size);

    for (size_t i = 0; i < size; i++) {
        MMCAM_LOGB("%hhu ", val[i]);
    }

    MMCAM_LOGB("\n\n");
}

};
