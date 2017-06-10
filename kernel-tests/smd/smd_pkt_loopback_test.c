/******************************************************************************
  @file  smd_pkt_loopback_test.c
  @brief Test program for SMD PKt Driver

  DESCRIPTION
  Program to test the working of SMD loopback port between APPS and MODEM
  SMD drivers.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  -----------------------------------------------------------------------------
  Copyright (c) 2010-2011, 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>

#define SMD_PKT_IOCTL_MAGIC (0xC2)

#define SMD_PKT_IOCTL_BLOCKING_WRITE \
	_IOR(SMD_PKT_IOCTL_MAGIC, 0, unsigned int)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

enum test_types {
	NOMINAL,
	ADVERSARIAL,
	STRESS,
	REPEAT,
};

static pthread_t rx_pthread;
static pthread_t tx_pthread;

/* conditional wait to keep the parent thread from returning */
pthread_cond_t  smd_test_cond_sync   = PTHREAD_COND_INITIALIZER;
pthread_mutex_t smd_test_mutex_sync  = PTHREAD_MUTEX_INITIALIZER;


/* This value corresonds to the maximum memory transaction that SMEM
   will currently perform.   When this number is no longer relavent
   we can just make this value really big */
#define TX_TEST_BUF_SZ  2048

#define SMD_DEV_NM   "/dev/smd_pkt_loopback"
static char *device_nm = SMD_DEV_NM;

static int smd_dev_p;
static int data_check = 1;
static int smd_verbose;
static int smd_num_tests = 23;
static int smd_test_result;
static int test_completed;
static int blocking_write;
static int user_iterations = 1;
static int modem_in_reset;
static int modem_restart_delay;

static unsigned char tx_test_buf[TX_TEST_BUF_SZ];
static unsigned char rx_test_buf[TX_TEST_BUF_SZ];

typedef struct{
	int buf_size;
	unsigned char pattern;
	double data_rate;
	int write_attempts;
	int read_attempts;
	struct timespec start_time;
	struct timespec end_time;
	double rtt;
}smd_pkt_loopback_test_type;

smd_pkt_loopback_test_type smd_tests_type1[23] = {
	{ 1, 0xfa, -1, 0, 0, {0, 0}, {0, 0}, 0 },   /* 1 */
	{ 2, 0xdd, -1, 0, 0, {0, 0}, {0, 0}, 0 },   /* 2 */
	{ 4, 0x12, -1, 0, 0, {0, 0}, {0, 0}, 0 },   /* 3 */
	{ 8, 0x62, -1, 0, 0, {0, 0}, {0, 0}, 0 },  /* 4 */
	{ 16, 0x51, -1, 0, 0, {0, 0}, {0, 0}, 0 },  /* 5 */
	{ 32, 0x17, -1, 0, 0, {0, 0}, {0, 0}, 0 },  /* 6 */
	{ 64, 0xb2, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 7 */
	{ 128, 0xa1, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 8 */
	{ 256, 0x89, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 9 */
	{ 512, 0x64, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 10 */
	{ 1024, 0x55, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 11 */
	{ 2 * 1024, 0x94, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 12 */
	{ 4 * 1024, 0x35, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 13 */
	{ 8111, 0x21, -1, 0, 0, {0, 0}, {0, 0}, 0 },    /* 14 */
	{ 8111, 0x46, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 15 */
	{ 8111, 0x91, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 16 */
	{ 8111, 0x25, -1, 0, 0, {0, 0}, {0, 0}, 0 },/* 17 */
	{ 8111, 0x77, -1, 0, 0, {0, 0}, {0, 0}, 0 },/* 18 */
	{ 8111, 0x58, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 19 */
	{ 8111, 0xef, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 20 */
	{ 8111, 0xea, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 21 */
	{ 8111, 0x0a, -1, 0, 0, {0, 0}, {0, 0}, 0 }, /* 22 */
	{ 16 * 1024, 0x35, -1, 0, 0, {0, 0}, {0, 0}, 0 } /* 23 */
};

double time_diff(const struct timespec start, const struct timespec end)
{
	double diff_time = 0;
	struct timespec temp;
	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	diff_time = (double)temp.tv_sec + ((double)(temp.tv_nsec)/(double)1000000000);
	return diff_time;
}


static void rx_test(int test_num)
{
	int bytes_left;
	int bytes_read;
	int temp;
	int iterations = 0;

	bytes_left = smd_tests_type1[test_num].buf_size;

	while (bytes_left) {
		temp = (bytes_left > TX_TEST_BUF_SZ) ? TX_TEST_BUF_SZ : bytes_left;

		if (modem_in_reset)
			return;

		bytes_read = read(smd_dev_p, rx_test_buf, TX_TEST_BUF_SZ);
		if (bytes_read != temp) {
			if (errno == ENETRESET) {
				/* modem in reset */
				printf("%s: modem in reset\n", __func__);
				modem_in_reset |= 1;
				return;
			}

			perror("rx_test");
			printf("Err: Read %d bytes than requested %d bytes\n", bytes_read, temp);
			smd_test_result |= -1;
			clock_gettime(CLOCK_REALTIME,&smd_tests_type1[test_num].end_time);
			return;
		}
		else if (data_check) {
			if (memcmp(rx_test_buf, tx_test_buf, bytes_read)) {
				printf("Err: RX data cmp failed\n");
				smd_test_result |= -1;
				clock_gettime(CLOCK_REALTIME,&smd_tests_type1[test_num].end_time);
				return;
			}
		}
		bytes_left -= bytes_read;
		iterations++;
	}
	clock_gettime(CLOCK_REALTIME,&smd_tests_type1[test_num].end_time);
	smd_tests_type1[test_num].rtt = time_diff(smd_tests_type1[test_num].start_time, smd_tests_type1[test_num].end_time);
	smd_tests_type1[test_num].data_rate = smd_tests_type1[test_num].buf_size/smd_tests_type1[test_num].rtt;
	smd_tests_type1[test_num].data_rate = (smd_tests_type1[test_num].data_rate * 8 * 2)/ 1000000;
	smd_tests_type1[test_num].read_attempts = iterations;
}

static void tx_test(int test_num)
{
	unsigned char pattern;
	int bytes_left;
	int bytes_written = 0;
	int temp;
	int iterations = 0;
	int write_attempts = 0;

	pattern = smd_tests_type1[test_num].pattern;
	bytes_left = smd_tests_type1[test_num].buf_size;
	memset(tx_test_buf, pattern, TX_TEST_BUF_SZ);

	clock_gettime(CLOCK_REALTIME,&smd_tests_type1[test_num].start_time);
	while (bytes_left) {
		temp = (bytes_left > TX_TEST_BUF_SZ) ? TX_TEST_BUF_SZ : bytes_left;

		if (modem_in_reset)
			return;

		bytes_written = write(smd_dev_p, tx_test_buf, temp);

		if (bytes_written == -1) {
			if (errno == ENETRESET) {
				/* modem in reset */
				printf("%s: modem in reset\n", __func__);
				modem_in_reset |= 1;
				return;
			}

			if (write_attempts < 60) {
				++write_attempts;
				bytes_written = 0;
				sleep(1);
			} else {
				perror("tx_test");
				printf("Err: Unable to write to buffer for a"
						" minute\n");
				smd_test_result |= -1;
				return;
			}
		} else if (bytes_written != temp) {
			perror("tx_test");
			printf("Err: Wrote %d bytes than requested %d bytes\n", bytes_written, temp);
			smd_test_result |= -1;
			return;
		}
		bytes_left -= bytes_written;
		iterations++;
	}
	smd_tests_type1[test_num].write_attempts = iterations;
}

static void *rx_test_body(void *arg)
{
	int test_num;
	int iterations = user_iterations;

	if (smd_verbose)
		printf("Test\tSize\tPattern\tRTT(ms)\t\tData Rate(Mbps)\t# of Packets\tResult\n");
	else
		printf("Test\tSize\t\tRTT(ms)\tData Rate(Mbps)\t\tResult\n");

	while (iterations-- > 0) {

		for (test_num = 0; test_num < smd_num_tests; test_num++) {
			rx_test(test_num);

			if (modem_in_reset) {
				printf("%s: Modem in reset, sleeping\n", __func__);
				sleep(1);
			} else if (smd_verbose) {
				printf("%2d\t%6d\t0x%2x\t%6.3lf\t\t%3.6lf\t\t%4d\t%s\n",
						(test_num + 1),
						smd_tests_type1[test_num].buf_size,
						smd_tests_type1[test_num].pattern,
						(smd_tests_type1[test_num].rtt * 1000),
						smd_tests_type1[test_num].data_rate,
						smd_tests_type1[test_num].write_attempts,
						(smd_test_result) ? "FAIL" : "PASS");
			} else {
				printf("%2d\t%6d\t\t%6.3lf\t\t%3.6lf\t%s\n",
						(test_num + 1),
						smd_tests_type1[test_num].buf_size,
						(smd_tests_type1[test_num].rtt * 1000),
						smd_tests_type1[test_num].data_rate,
						(smd_test_result) ? "FAIL" : "PASS");
			}

			pthread_mutex_lock(&smd_test_mutex_sync);
			test_completed = 1;
			pthread_mutex_unlock(&smd_test_mutex_sync);
			pthread_cond_signal(&smd_test_cond_sync);

			if (modem_in_reset)
				test_num--;

			/* wait for TX thread to receive signal */
			pthread_mutex_lock(&smd_test_mutex_sync);
			while (test_completed)
				pthread_cond_wait(&smd_test_cond_sync, &smd_test_mutex_sync);
			pthread_mutex_unlock(&smd_test_mutex_sync);
		}

		if (modem_restart_delay && iterations == user_iterations/2) {
			printf("Delaying for modem restart - restart modem now!\n");
			sleep(modem_restart_delay);
			printf("Continuing with test\n");
		}
	}

	return NULL;
}

static void *tx_test_body(void *arg)
{
	int test_num;
	int iterations = user_iterations;

	while (iterations-- > 0) {
		for (test_num = 0; test_num < smd_num_tests; test_num++) {

			smd_test_result = 0;

			tx_test(test_num);

			/* wait for RX thread to complete test */
			pthread_mutex_lock(&smd_test_mutex_sync);
			while (!test_completed) {
				pthread_cond_wait(&smd_test_cond_sync, &smd_test_mutex_sync);
			}

			if (modem_in_reset) {
				test_num--;
				modem_in_reset = 0;
			}

			/* signal RX thread to continue */
			test_completed = 0;
			pthread_mutex_unlock(&smd_test_mutex_sync);
			pthread_cond_signal(&smd_test_cond_sync);
		}
	}

	return NULL;
}

static int nominal_test(char *argv)
{
	int ret_val = 0;
	int *status;

	if ((smd_dev_p = open(device_nm, O_RDWR)) <= 0) {
		perror("Open");
		printf("cannot open file %s", device_nm);
		return -1;
	}

	if (blocking_write)
		ioctl(smd_dev_p, SMD_PKT_IOCTL_BLOCKING_WRITE, &blocking_write);
	ret_val = pthread_create (&tx_pthread , NULL, tx_test_body, NULL);
	if (ret_val) {
		printf("cannot spawn thread: ERR %d\n", ret_val);
		exit(1);
	}

	ret_val = pthread_create (&rx_pthread , NULL, rx_test_body, NULL);
	if (ret_val) {
		printf("cannot spawn thread: ERR %d\n", ret_val);
		exit(1);
	}

	pthread_join(tx_pthread, (void *)&status);
	if (status)
		ret_val = -1;
	pthread_join(rx_pthread, (void *)&status);
	if (status)
		ret_val = -1;

	close(smd_dev_p);

	return ret_val;
}

static int (*test_func[]) (char *) = {
	[NOMINAL] = nominal_test,
	[ADVERSARIAL] = NULL,
	[STRESS] = NULL,
	[REPEAT] = NULL,
};

static void usage(int ret)
{
	printf("Usage: smd_pkt_loopback_test [OPTIONS] [PARAMS] [TEST_TYPE]...\n"
			"Runs the loopback test with parameters specified\n"
			"over a SMD PKT device.  /dev/smd_pkt_loopback will be"
			"used if a device is not specified\n"
			"\n"
			"OPTIONS can be:\n"
			"  -v, --verbose         run with debug messages on (NOTE:\n"
			"                           currently has no effect).\n"
			"  -h, --help            print this help message and exit\n"
			"  -c, --nocheck         donot check the integrity of data\n"
			"  -w, --blocking_write use blocking writes\n"
			"\nPARAMS can be:\n"
			"  -b, --bytes           write/read buffer size (default 2 bytes)\n"
			"  -i, --iterations      number of test iterations (default 1)\n"
			"  -p, --pattern         pattern to be written (default 0xfa)\n"
			"  -d, --delay TIME      delay after iterations/2 by TIME seconds\n"
			"  -f, --file            The character device to use (default\n"
			"                           /dev/smd_pkt_loopback)\n"
			"\n"
			"TEST_TYPE can be:\n"
			"  -n, --nominal         run standard functionality tests\n"
			"  -a, --adversarial     run tests that try to break the\n"
			"                          driver, not supported currently\n"
			"  -s, --stress          run tests that try to maximize the\n"
			"                          capacity of the driver,\n"
			"                          not supported currently\n"
			"  -r, --repeatability   run 10 iterations of both the\n"
			"                          nominal and adversarial tests,\n"
			"                          not supported currently\n");

	exit(ret);
}

static uint32_t parse_command(int argc, char *const argv[])
{
	int command;
	unsigned ret = 0;

	struct option longopts[] = {
		{"verbose", no_argument, NULL, 'v'},
		{"nominal", no_argument, NULL, 'n'},
		{"adversarial", no_argument, NULL, 'a'},
		{"stress", no_argument, NULL, 's'},
		{"repeatability", no_argument, NULL, 'r'},
		{"help", no_argument, NULL, 'h'},
		{"nocheck", no_argument, NULL, 'c'},
		{"blocking_write", no_argument, NULL, 'w'},
		{"bytes", required_argument, NULL, 'b'},
		{"iterations", required_argument, NULL, 'i'},
		{"pattern", required_argument, NULL, 'p'},
		{"delay", required_argument, NULL, 'd'},
		{"file", required_argument, NULL, 'f'},
		{NULL, 0, NULL, 0},
	};

	while ((command = getopt_long(argc, argv, "v:nasrhcwb:p:i:d:f:", longopts,
					NULL)) != -1) {
		switch (command) {
		case 'v':
			smd_verbose = 1;
			break;
		case 'n':
			ret |= 1 << NOMINAL;
			break;
		case 'a':
			ret |= 1 << ADVERSARIAL;
			break;
		case 's':
			ret |= 1 << STRESS;
			break;
		case 'r':
			ret |= 1 << REPEAT;
			break;
		case 'b':
			smd_tests_type1[0].buf_size = atoi(optarg);
			smd_num_tests = 1;
			break;
		case 'i':
			user_iterations = atoi(optarg);
			break;
		case 'p':
			smd_tests_type1[0].pattern = (unsigned char)atoi(optarg);
			smd_num_tests = 1;
			break;
		case 'c':
			data_check = 0;
			break;
		case 'd':
			modem_restart_delay = atoi(optarg);
			break;
		case 'h':
			usage(0);
			break;
		case 'w':
			blocking_write = 1;
			break;
		case 'f':
			device_nm = optarg;
			break;
		default:
			usage(-1);
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	int rc = 0, num_tests_failed = 0, i;
	uint32_t test_mask = parse_command(argc, argv);

	/* Run the nominal case if none is specified */
	if (test_mask == 0)
		test_mask = 1 << NOMINAL;


	for (i = 0; i < (int)ARRAY_SIZE(test_func); i++) {
		/* Look for the test that was selected */
		if (!(test_mask & (1U << i)))
			continue;

		/* This test was selected, so run it */
		if (test_func[i])
			rc = test_func[i](argv[argc - 1]);
		else {
			printf("Sorry, Test not supported\n");
			rc = -1;
		}

		if (rc) {
			fprintf(stderr, "Test failed! rc: %d\n", rc);
			num_tests_failed++;
		} else {
			printf("Test passed\n");
		}
	}

	return -num_tests_failed;
}
