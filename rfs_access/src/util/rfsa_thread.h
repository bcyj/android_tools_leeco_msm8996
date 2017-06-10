#ifndef __RFSA_THREAD_H__
#define __RFSA_THREAD_H__

/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdint.h>

typedef int32_t (*rfsa_thread_fn_t) (void *param);

typedef void *rfsa_thread_t;

int32_t rfsa_thread_create(rfsa_thread_t *ret_thread, const char *name,
				uint32_t priority, uint8_t *stack_base,
				uint32_t stack_size, rfsa_thread_fn_t thread_fn,
				void *thread_param);

int32_t rfsa_thread_destroy(rfsa_thread_t thread);

#endif /* __RFSA_THREAD_H__ */
