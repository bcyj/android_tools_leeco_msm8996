#ifndef __RFSA_LOCK_H__
#define __RFSA_LOCK_H__

/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

typedef void *rfsa_lock_t;

int32_t rfsa_lock_create(rfsa_lock_t *ret_lock);
int32_t rfsa_lock_destroy(rfsa_lock_t lock);
int32_t rfsa_lock_enter(rfsa_lock_t lock);
int32_t rfsa_lock_leave(rfsa_lock_t lock);

#endif /* __RFSA_LOCK_H__ */
