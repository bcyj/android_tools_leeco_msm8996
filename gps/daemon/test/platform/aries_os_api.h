/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef ARIES_OS_API_H
#define ARIES_OS_API_H

#if 1

#include <pthread.h>
typedef pthread_mutex_t os_MutexBlockType;
#define os_MutexLock(z_bit_mutex) pthread_mutex_lock(z_bit_mutex)
#define os_MutexUnlock(z_bit_mutex) pthread_mutex_unlock(z_bit_mutex)
#define os_MutexInit(z_bit_mutex, context) (!pthread_mutex_init(z_bit_mutex, NULL))

#else

typedef uint8_t os_MutexBlockType;
#define os_MutexLock(z_bit) (*z_bit)++
#define os_MutexUnlock(z_bit) (*z_bit)++
#define os_MutexInit(z_bit, context) (1)

#endif

#define MUTEX_DATA_ONLY_CONTEXT 0

#endif /* ARIES_OS_API_H */

