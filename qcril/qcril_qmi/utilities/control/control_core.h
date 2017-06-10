/***************************************************************************************************
    @file
    cri_core.h

    @brief
    Supports utility functions for dealing with control events trigerred by HLOS interface or
    CRI.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CONTROL_CORE
#define CONTROL_CORE

#include "utils_common.h"

typedef struct control_core_control_event_data_type
{
    unsigned long event_id;
    void *data;
    size_t data_len;
}control_core_control_event_data_type;


#endif
