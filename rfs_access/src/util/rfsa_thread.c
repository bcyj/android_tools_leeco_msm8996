/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <pthread.h>
#include "rfsa_common.h"
#include "rfsa_thread.h"

typedef struct rfsa_thread_internal_t
{
	pthread_t		handle;
	rfsa_thread_fn_t	fn;
	void			*param;
	int32_t			result;
} rfsa_thread_internal_t;


static void *rfsa_thread_wrapper(void *param)
{
	rfsa_thread_internal_t *the_thread = ((rfsa_thread_internal_t*)param);

	if (the_thread != NULL) {
		the_thread->result = the_thread->fn(the_thread->param);
	}

	return NULL;
}

int32_t rfsa_thread_create(rfsa_thread_t *ret_thread,
				const char *name, /* Not implemented in SIM. */
				uint32_t priority, /* Not implemented in SIM. */
				uint8_t *stack_base, /* Not implemented in SIM. */
				uint32_t stack_size, /* Not implemented in SIM. */
				rfsa_thread_fn_t thread_fn, void *thread_param)
{
	int32_t ret;
	int check_point = 0;
	int cleanup_error = 0;
	rfsa_thread_internal_t *the_thread;
	pthread_attr_t attr;
	void *ret_val;

	(void)name;
	(void)priority;
	(void)stack_base;
	(void)stack_size;

	if (ret_thread == NULL) {
		return RFSA_EFAILED;
	}

	the_thread = ((rfsa_thread_internal_t*) malloc(sizeof(rfsa_thread_internal_t)));
	if (the_thread == NULL) {
		return RFSA_EFAILED;
	}

	ret = pthread_attr_init(&attr);
	if (ret) {
		goto rfsa_thread_create_error;
	}
	check_point = 1;

	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (ret) {
		goto rfsa_thread_create_error;
	}
	check_point = 2;

	the_thread->fn = thread_fn;
	the_thread->param = thread_param;
	the_thread->result = RFSA_EOK;
	ret = pthread_create(&the_thread->handle, &attr, rfsa_thread_wrapper, ((void*)the_thread));
	if (ret) {
		goto rfsa_thread_create_error;
	}
	check_point = 3;

	ret = pthread_attr_destroy(&attr);
	if (ret) {
		goto rfsa_thread_create_error;
	}
	check_point = 4;

	*ret_thread = ((rfsa_thread_t*)the_thread);

	return RFSA_EOK;

rfsa_thread_create_error:
	/* Perform error clean-up. */
	switch (check_point) {
		case 3:
			ret = pthread_join(the_thread->handle, &ret_val);
			if (ret) {
				cleanup_error = 1;
			}
			/* -fallthru */
		case 2:
			/* -fallthru */
		case 1:
			ret = pthread_attr_destroy( &attr );
			if (ret) {
				cleanup_error = 1;
			}
			/* -fallthru */
		case 0:
			free(the_thread);
			/* -fallthru */
		default:
			break;
	}

	return RFSA_EFAILED;
}

int32_t rfsa_thread_destroy(rfsa_thread_t thread)
{
	int32_t ret;
	rfsa_thread_internal_t *the_thread = ((rfsa_thread_internal_t*)thread);
	int cleanup_error = 0;
	void *ret_val;

	if (the_thread == NULL) {
		return RFSA_EFAILED;
	}

	(void) pthread_kill(the_thread->handle, SIGUSR1);

	ret = pthread_join(the_thread->handle, &ret_val);
	if (ret) {
		cleanup_error = 1;
	}

	ret = the_thread->result;
	free(the_thread);

	if (cleanup_error) {
		/* Clean-up can't fail but something did. */
		return RFSA_EFAILED;
	}

	return ret;
}
