/***************************************************************************
Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
***************************************************************************/
#ifndef __EZTUNE_INTERNAL_TYPES_H__
#define __EZTUNE_INTERNAL_TYPES_H__

#define WAIT_TIME_MILLISECONDS 1000

typedef struct {
    //event_id is of EztuneNotify type and will be casted
    //appropriately. event_id is defined as uint32_t for cleaner
    //header seperation & inclusion
    uint32_t event_id;
    uint32_t size;
    void *data;
} ProcessThreadMessage;

typedef struct {
    uint32_t type;
    uint32_t payload_size;
    void *payload_ptr;
} InterfaceThreadMessage;

#if defined(__cplusplus)
extern "C" {
#endif

const uint8_t kEztuneMinFps = 15;
const uint32_t kEztuneScaledWidth = 320;
const uint32_t kEztuneScaledHeight = 240;
const uint32_t kEztuneScaledLumaSize = (320 * 240);
const uint32_t kEztuneScaledSize = (320 * 240 * 3) >> 1;

#if defined(__cplusplus)
}
#endif

#endif /* __EZTUNE_INTERNAL_TYPES_H__ */
