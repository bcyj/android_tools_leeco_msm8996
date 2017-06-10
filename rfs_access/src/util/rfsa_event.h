#ifndef __RFSA_EVENT_H__
#define __RFSA_EVENT_H__

/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

typedef void *rfsa_event_t;

int32_t rfsa_event_create(rfsa_event_t *ret_event);
int32_t rfsa_event_destroy(rfsa_event_t event);
int32_t rfsa_event_wait(rfsa_event_t event);
int32_t rfsa_event_signal(rfsa_event_t event);
int32_t rfsa_event_signal_abortall(rfsa_event_t event);
int32_t rfsa_event_cancel_abortall(rfsa_event_t event);

#endif /* __RFSA_EVENT_H__ */
