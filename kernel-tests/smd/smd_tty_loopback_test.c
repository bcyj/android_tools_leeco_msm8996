/******************************************************************************
  @file  smd_tty_loopback_test.c
  @brief Test program for SMD

  DESCRIPTION
  Program to test the working of SMD loopback port between APPS and MODEM
  SMD drivers.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  -----------------------------------------------------------------------------
  Copyright (c) 2008-2011, 2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <poll.h>

/* only enable subsystem restart if proper TIOCM bits are defined */
#if defined(TIOCM_OUT1) && defined(TIOCM_OUT2)
#define HAVE_MODEM_RESTART
#endif

static pthread_t rx_pthread;
static pthread_t tx_pthread;

/* conditional wait to keep the parent thread from returning */
static pthread_cond_t  smd_test_rx_sig   = PTHREAD_COND_INITIALIZER;
static int smd_test_rx_done = 0;
static pthread_cond_t  smd_test_tx_sig   = PTHREAD_COND_INITIALIZER;
static int smd_test_tx_done = 0;
static pthread_mutex_t smd_test_mutex_sync  = PTHREAD_MUTEX_INITIALIZER;


/* This value corresonds to the maximum memory transaction that SMEM
   will currently perform.   When this number is no longer relavent
   we can just make this value really big */
#define TX_TEST_BUF_SZ  2048

#define SMD_DEV_NM   "/dev/smd36"

#define D(x...)	\
	do { \
	if (smd_verbose) { \
		printf("%s:%d ", __FUNCTION__, __LINE__); \
		printf(x); \
	} \
	} while(0)

static int smd_dev_wp, smd_dev_rp;
static int data_check = 1;
static int smd_verbose = 0;
static int smd_max_attempts = 2000;
static int smd_num_tests = 26;
static int smd_test_result = 0;
static int user_iterations = 1;
static volatile int modem_in_reset;
static int test_number;

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
}smd_tty_loopback_test_type;

smd_tty_loopback_test_type smd_tests_type1[26] = {
	{ 2, 0xfa, 0, 0, 0, {0, 0}, {0, 0}, 0 },   /* 1 */
	{ 4, 0xdd, 0, 0, 0, {0, 0}, {0, 0}, 0 },   /* 2 */
	{ 8, 0x12, 0, 0, 0, {0, 0}, {0, 0}, 0 },   /* 3 */
	{ 16, 0x62, 0, 0, 0, {0, 0}, {0, 0}, 0 },  /* 4 */
	{ 32, 0x51, 0, 0, 0, {0, 0}, {0, 0}, 0 },  /* 5 */
	{ 64, 0x17, 0, 0, 0, {0, 0}, {0, 0}, 0 },  /* 6 */
	{ 128, 0xb2, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 7 */
	{ 256, 0xa1, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 8 */
	{ 512, 0x89, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 9 */
	{ 1024, 0x64, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 10 */
	{ 2048, 0x55, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 11 */
	{ 2044, 0x94, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 12 */
	{ 2044 * 90, 0x35, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 13 */
	{ 1, 0x21, 0, 0, 0, {0, 0}, {0, 0}, 0 },    /* 14 */
	{ 3, 0x46, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 15 */
	{ 9, 0x91, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 16 */
	{ 27, 0x25, 0, 0, 0, {0, 0}, {0, 0}, 0 },/* 17 */
	{ 81, 0x77, 0, 0, 0, {0, 0}, {0, 0}, 0 },/* 18 */
	{ 243, 0x58, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 19 */
	{ 729, 0xef, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 20 */
	{ 729 *3, 0xea, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 21 */
	{ 729 *9, 0xb1, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 22 */
	{ 729 *27, 0x3c, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 23 */
	{ 729 *81, 0xb7, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 24 */
	{ 729 *243, 0x2f, 0, 0, 0, {0, 0}, {0, 0}, 0 }, /* 25 */
	{ 729 * 729, 0x1f, 0, 0, 0, {0, 0}, {0, 0}, 0 } /* 26 */
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

#ifdef HAVE_MODEM_RESTART
static pthread_mutex_t reset_state_mutex = PTHREAD_MUTEX_INITIALIZER;
int update_modem_in_reset(int fd)
{
	int serial_bits;
	struct pollfd fds;
	struct timespec tv;
	int ret;

	pthread_mutex_lock(&reset_state_mutex);
	ioctl(fd, TIOCMGET, &serial_bits);
	if (serial_bits & TIOCM_OUT2) {
		if (serial_bits & TIOCM_OUT1) {
			printf("%s: modem entered reset\n", __func__);
			modem_in_reset = 1;

			/* verify poll error code */
			fds.fd = fd;
			fds.events = 0;
			fds.revents = 0;
			ret = poll(&fds, 1, 0);
			if (ret < 0 || !(fds.revents & POLLHUP))
				printf("%s: ERROR - poll returned %d; revents 0x%x\n", __func__, ret, fds.revents);
			else
				printf("%s: OK - poll returned %d; revents 0x%x\n", __func__, ret, fds.revents);
		}
		else {
			if (modem_in_reset) {
				printf("%s: modem exited reset\n", __func__);

				/* verify poll OK */
				fds.fd = fd;
				fds.events = 0;
				fds.revents = 0;
				ret = poll(&fds, 1, 0);
				if (ret < 0 || fds.revents & POLLHUP)
					printf("%s: ERROR - poll returned %d; revents 0x%x\n", __func__, ret, fds.revents);
				else
					printf("%s: OK - poll returned %d; revents 0x%x\n", __func__, ret, fds.revents);
			}
			modem_in_reset = 0;
		}
	}
	pthread_mutex_unlock(&reset_state_mutex);
	return modem_in_reset;
}
#else
int update_modem_in_reset(int fd)
{
	return modem_in_reset;
}
#endif


static int rx_test(int test_num)
{
	int bytes_left;
	int bytes_read;
	int temp;
	int test_iterations = 0;

	bytes_left = smd_tests_type1[test_num].buf_size;
	while (bytes_left && (test_iterations <= smd_max_attempts)) {
		temp = (bytes_left > TX_TEST_BUF_SZ) ? TX_TEST_BUF_SZ : bytes_left;
		test_iterations++;

		bytes_read = read(smd_dev_rp, rx_test_buf, temp);

		if (update_modem_in_reset(smd_dev_rp)) {
			return 0;
		}

		if (bytes_read > temp) {
			printf("Err: Read more bytes than requested\n");
			bytes_left -= temp;
			smd_test_result |= -1;
			break;
		}
		else if (bytes_read >= 0) {
			bytes_left -= bytes_read;

			if (data_check) {
				if (memcmp(rx_test_buf, tx_test_buf, bytes_read)) {
					printf("Err: RX data cmp failed\n");
					smd_test_result |= -1;
					break;
				}
			}
		}
	}

	if (bytes_left) {
		printf("Err: Couldnot read all bytes\n");
		smd_test_result |= -1;
	}

	clock_gettime(CLOCK_REALTIME,&smd_tests_type1[test_num].end_time);
	smd_tests_type1[test_num].rtt = time_diff(smd_tests_type1[test_num].start_time, smd_tests_type1[test_num].end_time);
	smd_tests_type1[test_num].data_rate = smd_tests_type1[test_num].buf_size/smd_tests_type1[test_num].rtt;
	smd_tests_type1[test_num].data_rate = (smd_tests_type1[test_num].data_rate * 8 * 2)/1000000;
	smd_tests_type1[test_num].read_attempts = test_iterations;
	return 1;
}

static int tx_test(int test_num)
{
	int bytes_left;
	unsigned char pattern;
	int bytes_written = 0;
	int test_iterations = 0;
	int temp;

	pattern = smd_tests_type1[test_num].pattern;
	bytes_left = smd_tests_type1[test_num].buf_size;

	temp = (bytes_left > TX_TEST_BUF_SZ) ? TX_TEST_BUF_SZ : bytes_left;
	memset(tx_test_buf, pattern, temp);

	clock_gettime(CLOCK_REALTIME,&smd_tests_type1[test_num].start_time);
	while (bytes_left && (test_iterations <= smd_max_attempts)) {
		temp = (bytes_left > TX_TEST_BUF_SZ) ? TX_TEST_BUF_SZ : bytes_left;
		test_iterations++;

		bytes_written = write(smd_dev_wp, tx_test_buf, temp);

		if (update_modem_in_reset(smd_dev_wp))
			return 0;

		if (bytes_written > temp) {
			printf("Err: Wrote more bytes than requested\n");
			bytes_left -= temp;
			smd_test_result |= -1;
			break;
		}
		else if (bytes_written >= 0)
			bytes_left -= bytes_written;
		else
			usleep(1000);
	}

	if (bytes_left) {
		printf("Err: Couldnot write all bytes\n");
		smd_test_result |= -1;
	}

	smd_tests_type1[test_num].write_attempts = test_iterations;
	return 1;
}


static int reset_test_num(void)
{
	test_number = 0;
	return test_number;
}

static int get_test_num(void)
{
	return test_number;
}

static int next_test(void)
{
	return ++test_number;
}

static void *rx_test_body(void *arg)
{
	int test_num;
	int iterations = user_iterations;

	if (smd_verbose)
		printf("Test\tSize\tPattern\tRTT(ms)\tData Rate(Mbps)\tRead Attempts\tWrite Attempts\tResult\n");
	else
		printf("Test\tSize\t\tRTT(ms)\tData Rate(Mbps)\t\tResult\n");

	if ((smd_dev_rp = open(SMD_DEV_NM, O_RDWR | O_APPEND)) < 0) {
		printf("%s: cannot open file %s\n", __func__, SMD_DEV_NM);
		return NULL;
	}

	while (iterations-- > 0) {
		D("iteration %d\n", iterations);
		for (test_num = 0; test_num < smd_num_tests;) {
			D("test_num=%d\n", test_num);
			if (rx_test(test_num)) {
				if (smd_verbose) {
					printf("%2d\t%6d\t0x%2x\t%6.3lf\t%3.6lf\t\t%4d\t\t%4d\t%s\n",
						(test_num + 1),
						smd_tests_type1[test_num].buf_size,
						smd_tests_type1[test_num].pattern,
						(smd_tests_type1[test_num].rtt * 1000),
						smd_tests_type1[test_num].data_rate,
						smd_tests_type1[test_num].read_attempts,
						smd_tests_type1[test_num].write_attempts,
						(smd_test_result)? "FAIL" : "PASS");
				} else {
					printf("%2d\t%6d\t\t%6.3lf\t\t%3.6lf\t%s\n",
						(test_num + 1),
						smd_tests_type1[test_num].buf_size,
						(smd_tests_type1[test_num].rtt * 1000),
						smd_tests_type1[test_num].data_rate,
						(smd_test_result)? "FAIL" : "PASS");
				}
			} else {
				printf("Test aborted due to modem reset\n");

				/* wait for modem to come out of reset */
				while (update_modem_in_reset(smd_dev_rp)) {
					usleep(500000);
					printf("Waiting for modem to come back\n");
				}
			}

			/* notify TX thread of completion */
			D("pthread_cond_signal\n");
			pthread_mutex_lock(&smd_test_mutex_sync);
			test_num = next_test();
			smd_test_rx_done = 1;
			pthread_cond_signal(&smd_test_rx_sig);
			pthread_mutex_unlock(&smd_test_mutex_sync);
		}

		/* wait for TX thread to complete test loop */
		pthread_mutex_lock(&smd_test_mutex_sync);
		while (!smd_test_tx_done)
			pthread_cond_wait(&smd_test_tx_sig, &smd_test_mutex_sync);
		D("reset_test_num\n");
		smd_test_tx_done = 0;
		reset_test_num();
		pthread_mutex_unlock(&smd_test_mutex_sync);
	}
	D("exiting\n");
	close(smd_dev_rp);

	return NULL;
}

static void *tx_test_body(void *arg)
{
	int test_num;
	int iterations = user_iterations;

	if ((smd_dev_wp = open(SMD_DEV_NM, O_RDWR | O_APPEND)) < 0) {
		printf("%s cannot open file %s\n", __func__, SMD_DEV_NM);
		return NULL;
	}

	while (iterations-- > 0) {
		D("iteration %d\n", iterations);
		for (test_num = 0; test_num < smd_num_tests; ) {
			D("test_num=%d\n", test_num);

			smd_test_result = 0;

			tx_test(test_num);

			/* wait for RX thread to receive data */
			D("pthread_cond_wait\n");
			pthread_mutex_lock(&smd_test_mutex_sync);
			while (!smd_test_rx_done)
				pthread_cond_wait(&smd_test_rx_sig, &smd_test_mutex_sync);
			smd_test_rx_done = 0;
			test_num = get_test_num();
			pthread_mutex_unlock(&smd_test_mutex_sync);
		}

		/* signal RX thread that we are done */
		D("pthread_cond_signal\n");
		pthread_mutex_lock(&smd_test_mutex_sync);
		smd_test_tx_done = 1;
		pthread_cond_signal(&smd_test_tx_sig);
		pthread_mutex_unlock(&smd_test_mutex_sync);
	}
	D("exiting\n");
	close(smd_dev_wp);

	return NULL;
}

static void usage(int ret)
{
	printf("Usage: smd_tty_loopback_test [OPTIONS] [PARAMS]...\n\n"
	       "Runs the SMD loopback test with parameters specified.\n"
	       "If neither size or pattern is specified,\n"
	       "then the default tests are run.\n"
	       "\nOPTIONS can be:\n"
	       "  -v, --verbose         run with debug messages on\n"
	       "  -h, --help            print this help message and exit\n"
	       "  -n, --nocheck         Donot check the integrity of data\n"
	       "\nPARAMS can be:\n"
	       "  -s, --size            write/read buffer size (default 2 bytes)\n"
	       "  -p, --pattern         pattern to be written (default 0xfa)\n"
	       "  -a, --attempts        max number of attempts (default 2000)\n"
	       "  -i, --iterations      number of test iterations (default 1)\n"
		  );

	exit(ret);
}

static void parse_command(int argc, char *const argv[])
{
	int command;

	struct option longopts[] = {
		{"verbose", no_argument, NULL, 'v'},
		{"help", no_argument, NULL, 'h'},
		{"nocheck", no_argument, NULL, 'n'},
		{"size", required_argument, NULL, 's'},
		{"pattern", required_argument, NULL, 'p'},
		{"attempts", required_argument, NULL, 'a'},
		{"iterations", required_argument, NULL, 'i'},
		{NULL, 0, NULL, 0},
	};

	while ((command = getopt_long(argc, argv, "vhns:p:a:i:", longopts,
				      NULL)) != -1) {
		switch (command) {
		case 'v':
			smd_verbose = 1;
			break;
		case 's':
			smd_tests_type1[0].buf_size = atoi(optarg);
			smd_num_tests = 1;
			break;
		case 'a':
			smd_max_attempts = atoi(optarg);
			break;
		case 'p':
			smd_tests_type1[0].pattern = (unsigned char)atoi(optarg);
			smd_num_tests = 1;
			break;
		case 'n':
			data_check = 0;
			break;
		case 'i':
			user_iterations = atoi(optarg);
			break;
		case 'h':
			usage(0);
		default:
			usage(-1);
		}
	}
}

int main(int argc, char **argv)
{
	int ret_val;

	parse_command(argc, argv);

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

	pthread_join(tx_pthread, NULL);
	pthread_join(rx_pthread, NULL);

	return 0;
}
