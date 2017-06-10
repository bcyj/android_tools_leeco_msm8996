/******************************************************************************
  @file  vfptest.c
  @brief This file contains test code for the VFP unit

  DESCRIPTION
  vfptest is a basic program for sanity-testing floating-point operations

  INITIALIZATION AND SEQUENCING REQUIREMENTS

 -----------------------------------------------------------------------------
 Copyright (c) 2011,2012 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
 -----------------------------------------------------------------------------

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <pthread.h>

#define BUF_SIZE 256

double buf[BUF_SIZE];
int buf_ptr = 0;
static pthread_mutex_t buf_lock;

void store(double x)
{
	pthread_mutex_lock(&buf_lock);
	buf[buf_ptr++] = x;
	buf_ptr &= (BUF_SIZE - 1);
	pthread_mutex_unlock(&buf_lock);
}


int run_test(int n)
{
	int i;

	for (i = 0; i < n; i++) {
		double a, b, c, d, e, f, g, h;

		/* (a^2 - b^2) = (a - b)(a + b) */

		if (i & 1) {
			a = i * 100 + 39;
			b = (3000000 - i * 6);
		} else {
			a = (2800000 - i * 7);
			b = i * 121 + 45;
		}

		store(a);
		store(b);

		a = a / 23456;
		b = b / 12345;
		store(a);
		store(b);

		/* (a^2 - b^2) */
		c = a * a;
		store(c);
		d = b * b;
		store(d);
		e = c - d;
		store(e);

		/* (a - b)(a + b) */
		f = a + b;
		store(f);
		g = a - b;
		store(g);
		h = f * g;
		store(h);

		if (e - h > 0.000001) {
			printf("Mismatch: a=%f, b=%f:  e=%f, h=%f, delta=%f\n",
				a, b, e, h, e-h);
			printf("VFP test FAILED!\n");
			return -1;
		}
	}

	return 0;
}

static void *run_thread_test(void *args)
{
	return (void *)(long)run_test((long)args);
}

static int start_test(int nthreads, int niter)
{
	pthread_t *threads;
	int i, j;
	int ret, res;

	pthread_mutex_init(&buf_lock, NULL);
	if (!nthreads)
		goto run_once;

	threads = malloc(nthreads * sizeof(threads));
	if (!threads)
		goto run_once;
	for (i = 0; i < nthreads; i++) {
		ret = pthread_create(&threads[i], NULL, run_thread_test,
					(void *)(long)niter);
		if (ret) {
			printf("Failed to create %d thread\n", i);
			for (j = 0; j < i; j++)
				pthread_join(threads[j], NULL);
			goto bail_out;
		}
	}

	for (i = 0; i < nthreads; i++) {
		pthread_join(threads[i], (void **)&res);
		ret |= res;
	}
bail_out:
	free(threads);
	pthread_mutex_destroy(&buf_lock);
	return ret;

run_once:
	ret = run_test(niter);
	pthread_mutex_destroy(&buf_lock);
	return ret;
}

int main(int argc, char** argv)
{
	int nthreads = 0, niter;

	if (argc < 2) {
		printf("Usage: %s [iterations] [threads]\n", argv[0]);
		return -1;
	}

	niter = atoi(argv[1]);
	if (argc > 2)
		nthreads = atoi(argv[2]);
	printf("Running test with %d threads (%d iterations)\n",
		nthreads, niter);
	return start_test(nthreads, niter);
}
