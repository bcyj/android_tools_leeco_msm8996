/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <pthread.h>
#include "rfsa_common.h"
#include "rfsa_event.h"

typedef struct rfsa_int_event_t
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cvar;
	int		set;
	int		abort;
} rfsa_int_event_t;


int32_t rfsa_event_create(rfsa_event_t *event)
{
	int ret;
	int do_once = 0;
	int cleanup_errors = 0;
	rfsa_int_event_t *the_event;

	if (event == NULL) {
		return RFSA_EFAILED;
	}

	the_event = ((rfsa_int_event_t*) malloc(sizeof(rfsa_int_event_t)));
	if (the_event == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_init(&the_event->mutex, NULL);
	if (ret != 0) {
		goto rfsa_event_create_error;
	}

	ret = pthread_cond_init(&the_event->cvar, NULL);
	if (ret != 0) {
		goto rfsa_event_create_error;
	}

	the_event->set = 0;
	the_event->abort = 0;

	*event = ((rfsa_event_t*)the_event);

	goto rfsa_event_create_ok;

rfsa_event_create_error:
	(void)pthread_mutex_destroy(&the_event->mutex);

	free((void*)the_event);
	return RFSA_EFAILED;

rfsa_event_create_ok:
	return RFSA_EOK;
}

int32_t rfsa_event_destroy(rfsa_event_t event)
{
	int ret;
	int cleanup_errors = 0;
	rfsa_int_event_t *the_event = ((rfsa_int_event_t*)event);

	if (the_event == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_cond_destroy(&the_event->cvar);
	if (ret) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_destroy(&the_event->mutex);
	if (ret) {
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}

int32_t rfsa_event_wait(rfsa_event_t event)
{
	int ret;
	int errors = 0;
	rfsa_int_event_t *the_event = ((rfsa_int_event_t*)event);

	if (the_event == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_lock(&the_event->mutex);
	if (ret != 0) {
		return RFSA_EFAILED;
	}

	if ((the_event->set == 0) && (the_event->abort == 0)) {
		ret = pthread_cond_wait(&the_event->cvar, &the_event->mutex);
		if (ret) {
			pthread_mutex_unlock(&the_event->mutex);
			return RFSA_EFAILED;
		}
	}

	the_event->set = 0;
	if (the_event->abort != 0) {
		pthread_mutex_unlock(&the_event->mutex);
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_unlock(&the_event->mutex);
	if (ret) {
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}


int32_t rfsa_event_signal(rfsa_event_t event)
{
	int ret;
	int errors = 0;
	rfsa_int_event_t *the_event = ((rfsa_int_event_t*)event);

	if (the_event == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_lock(&the_event->mutex);
	if (ret != 0) {
		return RFSA_EFAILED;
	}

	if (the_event->set == 0) {
		the_event->set = 1;
		ret = pthread_cond_signal(&the_event->cvar);
		if (ret) {
			pthread_mutex_unlock(&the_event->mutex);
			return RFSA_EFAILED;
		}
	}

	ret = pthread_mutex_unlock(&the_event->mutex);
	if (ret) {
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}


int32_t rfsa_event_signal_abortall(rfsa_event_t event)
{
	int ret;
	int errors = 0;
	rfsa_int_event_t *the_event = ((rfsa_int_event_t*)event);

	if (the_event == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_lock(&the_event->mutex);
	if (ret != 0) {
		return RFSA_EFAILED;
	}

	if (the_event->abort == 0) {
		the_event->abort = 1;
		ret = pthread_cond_broadcast(&the_event->cvar);
		if (ret != 0) {
			pthread_mutex_unlock(&the_event->mutex);
			return RFSA_EFAILED;
		}
	}

	ret = pthread_mutex_unlock(&the_event->mutex);
	if (ret != 0) {
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}


int32_t rfsa_event_cancel_abortall(rfsa_event_t event)
{
	int ret;
	int errors = 0;
	rfsa_int_event_t *the_event = ((rfsa_int_event_t*)event);

	if (the_event == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_mutex_lock(&the_event->mutex);
	if (ret != 0) {
		return RFSA_EFAILED;
	}

	the_event->abort = 0;

	ret = pthread_mutex_unlock(&the_event->mutex);
	if (ret != 0) {
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}
