/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <pthread.h>
#include "rfsa_common.h"
#include "rfsa_lock.h"

typedef struct rfsa_int_lock_t {
	pthread_mutexattr_t	rfsa_mutex_attr;
	pthread_mutex_t		rfsa_mutex;
} rfsa_int_lock_t;

int32_t rfsa_lock_create(rfsa_lock_t *ret_lock)
{
	int32_t ret;
	int do_once = 0;
	rfsa_int_lock_t *the_lock;

	if (ret_lock == NULL) {
		return RFSA_EFAILED;
	}

	the_lock = ((rfsa_int_lock_t*) malloc(sizeof(rfsa_int_lock_t)));
	if (the_lock == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutexattr_init(&the_lock->rfsa_mutex_attr);
	if (ret) {
		goto rfsa_lock_create_error;
	}
	ret = pthread_mutexattr_settype(&the_lock->rfsa_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	if (ret) {
		goto rfsa_lock_create_error;
	}
	ret = pthread_mutex_init(&the_lock->rfsa_mutex, NULL);
	if (ret) {
		goto rfsa_lock_create_error;
	}

	*ret_lock = the_lock;

	goto rfsa_lock_create_ok;

rfsa_lock_create_error:
	ret = pthread_mutex_destroy(&the_lock->rfsa_mutex);
	ret = pthread_mutexattr_destroy(&the_lock->rfsa_mutex_attr);
	free(the_lock);

	return RFSA_EFAILED;

rfsa_lock_create_ok:
	return RFSA_EOK;
}

int32_t rfsa_lock_destroy(rfsa_lock_t lock)
{
	int32_t ret;
	int cleanup_errors = 0;
	rfsa_int_lock_t *the_lock = lock;

	if (the_lock == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_destroy(&the_lock->rfsa_mutex);
	if (ret) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutexattr_destroy(&the_lock->rfsa_mutex_attr);
	if (ret) {
		return RFSA_EFAILED;
	}

	free(the_lock);

	return RFSA_EOK ;
}

int32_t rfsa_lock_enter(rfsa_lock_t lock)
{
	int32_t ret;
	rfsa_int_lock_t *the_lock = lock;

	if (the_lock == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_lock(&the_lock->rfsa_mutex);
	if (ret) {
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}


int32_t rfsa_lock_leave(rfsa_lock_t lock)
{
	int32_t ret;
	rfsa_int_lock_t *the_lock = lock;

	if (the_lock == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_unlock(&the_lock->rfsa_mutex);
	if (ret) {
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}
