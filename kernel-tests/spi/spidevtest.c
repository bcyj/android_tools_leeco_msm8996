/*
 * User-space unit test application for the SPI driver using the userspace
 * spidev.
 *
 * Copyright (c) 2009, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/types.h>
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include <linux/spi/spidev.h>

#include <linux/types.h>
#include <linux/ioctl.h>

#define SPIDEVTEST_BUFLEN	16   /* one block */
#define SPIDEVTEST_BUFLEN2	(16*2) /* two blocks */
#define SPIDEVTEST_BUFLEN3	(16*3) /* three blocks */
#define SPIDEVTEST_BUFLEN4	(16*4) /* fifo size */
#define SPIDEVTEST_BUFLEN5	(16+8) /* fifo size + something */
#define SPIDEVTEST_1K_BUFLEN	(1024)
#define SPIDEVTEST_2K_BUFLEN	(1024*2)
#define SPIDEVTEST_3K_BUFLEN	(1024*3)
#define SPIDEVTEST_4K_BUFLEN	(1024*4)
#define SPIDEVTEST_11K_BUFLEN	(1024*11)
#define SPIDEVTEST_12K_BUFLEN	(1024*12)
#define SPIDEVTEST_13K_BUFLEN	(1024*13)
#define SPIDEVTEST_15K_BUFLEN	(1024*15)
#define SPIDEVTEST_17K_BUFLEN	(1024*17)
#define SPIDEVTEST_3K_BUFLEN_UN (1024*3-32)
#define SPIDEVTEST_5K_BUFLEN	(5*1024)
#define SPIDEVTEST_6K_BUFLEN	(6*1024)
#define SPIDEVTEST_7K_BUFLEN	(7*1024)
#define SPIDEVTEST_32K_BUFLEN_UN (32*1024+8*5) /* largest size */

#define SPIDEVTEST_DEVLEN	50

#define SPIDEVTEST_INVALID_CS	5

#define ARRAY_SIZE(a) 		(int)(sizeof(a) / sizeof((a)[0]))
#define ALIGN(x, a)		(((x) + (a) - 1) & ~((a) - 1))
#define SPI_REPEAT_COUNT	10
#define USHORT_MAX 		(~0U)

static uint8_t saved_mode;
static uint8_t req_mode;
static uint8_t verbose;
static uint8_t suppress_errors;
static uint8_t loop;
static uint8_t aardvark;
static uint8_t testid = -1;
static uint32_t test_set;
static uint32_t max_speed;
static uint8_t continuous;

struct test_t {
	char *desc;
	int (*func)(int);
};

enum test_types {
	spi_nominal,
	spi_adversarial,
	spi_stress,
	spi_repeat,
	spi_single
};

static int nominal_test(int fd);
static int adversarial_test(int fd);
static int stress_test(int fd);
static int repeat_test(int fd);
static int single_test(int fd);

struct test_t tests[] = {
	[spi_nominal] = { "nominal test", nominal_test },
	[spi_adversarial] = { "adversarial test", adversarial_test },
	[spi_repeat] = { "repeat test", repeat_test },
	[spi_stress] = { "stress test", stress_test },
	[spi_single] = { "single test", single_test },
};

static void hexdump(const uint8_t *buf, unsigned len, const char *name)
{
	unsigned i;

	fprintf(stdout, "%s =\n", name);
	for (i = 0; i < len; i++) {
		fprintf(stdout, "%02x ", buf[i]);
		if ((i+1)%10 == 0)
			fprintf(stdout, "\n");
	}
	fprintf(stdout, "\n");
}

static int dev_open(char *dev_name)
{
	int fd;
	char buf[SPIDEVTEST_DEVLEN];

	snprintf(buf, sizeof(buf), "/dev/%s", dev_name);
	fd = open(buf, O_RDWR);
	if (fd <= 0)
		fprintf(stderr, "Could not open device %s\n", buf);
	return fd ;
}

static int set_mode(int fd, uint8_t spi_mode)
{
	int rc;

	if (verbose)
		fprintf(stdout, "Changing to spi mode=%#x\n", spi_mode);
	rc = ioctl(fd, SPI_IOC_WR_MODE, &spi_mode);
	if (rc && !suppress_errors)
		fprintf(stderr, "Error in SPI_IOC_WR_MODE ioctl ret=%d\n", rc);

	return rc;
}

static int save_mode(int fd)
{
	int rc = 0;

	rc = ioctl(fd, SPI_IOC_RD_MODE, &saved_mode);
	if (rc)
		fprintf(stderr, "Error in SPI_IOC_RD_MODE ioctl ret=%d\n", rc);

	if (verbose)
		fprintf(stdout, "Saved spi mode=%#x\n", saved_mode);
	return rc;
}

static int set_bpw(int fd, uint8_t bpw)
{
	int rc;

	if (verbose)
		fprintf(stdout, "Changing to bpw=%#x\n", bpw);
	rc = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, bpw);
	if (rc && !suppress_errors)
		fprintf(stderr, "Error in SPI_IOC_WR_BITS_PER_WORD ioctl "
			"ret=%d\n", rc);

	return rc;
}

static void fill_random_buffer(uint8_t *buf, int len)
{
	int i;
	srand(time(NULL));
	for (i = 0; i < len; i++)
		buf[i] = (uint8_t)(rand() & 0xFF);
}

static int send_buffer(int fd, struct spi_ioc_transfer *data,
		       uint8_t spi_mode)
{
	int rc;

	rc = set_mode(fd, spi_mode);
	if (rc)
		goto do_exit;
	if (max_speed)
		data->speed_hz = max_speed;

	/* Do full-duplex transfer */
	rc = ioctl(fd, SPI_IOC_MESSAGE(1), data);
	if (rc <= 0) {
		fprintf(stdout, "Error in SPI_IOC_MESSAGE ioctl ret=%d\n", rc);
		rc = -1;
		goto do_exit;
	}

	if (rc != (int)data->len) {
		if (verbose)
			fprintf(stdout, "Error: We sent %d instead %d", rc,
				data->len);
		goto do_exit;
	}

	if (verbose) {
		if (data->tx_buf) {
			fprintf(stdout, "Wrote %d characters\n", rc);
			hexdump((uint8_t *)(uintptr_t)data->tx_buf, data->len,
				"tx_buf");
		}
		if (data->rx_buf) {
			fprintf(stdout, "Recieved %d characters\n", rc);
			hexdump((uint8_t *)(uintptr_t)data->rx_buf, data->len,
				"rx_buf");
		}
	}
	return 0;

do_exit:
	return rc;
}

static int send_loopback_buffer(int fd, struct spi_ioc_transfer *data)
{
	int rc;

	rc = send_buffer(fd, data, req_mode | SPI_LOOP);
	if (rc)
		goto do_exit;

	/* Check rx buffer is equal to tx buffer */
	rc = memcmp((void *)(uintptr_t)data->rx_buf,
		    (void *)(uintptr_t)data->tx_buf, data->len);
	if (verbose)
		fprintf(stdout, "Comparing rx buffer ret=%d\n", rc);
do_exit:
	return rc;
}

/* Send all possible sizes and modes */
static int send_all_loopback_buffer(int fd, struct spi_ioc_transfer *data)
{
	uint8_t bits_per_word[] = { 8, 16, 32 };
	uint8_t modes[] = { SPI_MODE_0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3,
			    SPI_MODE_0 | SPI_CS_HIGH, SPI_MODE_1 | SPI_CS_HIGH,
			    SPI_MODE_2 | SPI_CS_HIGH, SPI_MODE_3 | SPI_CS_HIGH
		};
	uint32_t i, j;
	int rc, test_result = 0;

	for (i = 0; i < ARRAY_SIZE(bits_per_word); i++) {
		for (j = 0; j < ARRAY_SIZE(modes); j++) {
			req_mode = modes[j];
			data->bits_per_word = bits_per_word[i];
			data->len = ALIGN(data->len, data->bits_per_word/8);
			if (verbose) {
				fprintf(stdout, "Sending len=%d bits per "
					"word=%d mode=%#x\n", data->len,
					bits_per_word[i], req_mode);
			}
			rc = send_loopback_buffer(fd, data);
			test_result |= rc;
			if (verbose)
				fprintf(stdout, "Result:%s\n", rc ?
					"Failed" : "Passed");
			else if (rc && !continuous)
				break;
		}
	}
	return test_result;
}

/* Configure device to use loopback mode and transfer data */
static int test_loopback(int fd)
{
	uint8_t tx_buf[SPIDEVTEST_BUFLEN] = {0};
	uint8_t rx_buf[ARRAY_SIZE(tx_buf)] = {0};
	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = ARRAY_SIZE(tx_buf),
	};

	fill_random_buffer(tx_buf, ARRAY_SIZE(tx_buf));
	return send_all_loopback_buffer(fd, &data);
}

/* Configure device to use loopback mode and transfer data */
static int test_loopback_all_sizes(int fd, int *sizes, int num_sizes)
{
	uint8_t tx_buf[SPIDEVTEST_32K_BUFLEN_UN];
	uint8_t rx_buf[SPIDEVTEST_32K_BUFLEN_UN];
	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = ARRAY_SIZE(tx_buf),
	};
	int i, rc;

	fill_random_buffer(tx_buf, ARRAY_SIZE(tx_buf));
	for (i = 0; i < num_sizes; i++) {
		if (verbose) {
			fprintf(stdout, "Sending buffer of %d size\n",
				sizes[i]);
		}
		data.len = sizes[i];
		rc = send_all_loopback_buffer(fd, &data);
		if (rc)
			return rc;
	}
	return 0;
}

static int test_loopback_aligned(int fd)
{
	int sizes[] = {
		1, 4, 8, 16, 24, 32, 64, 128, SPIDEVTEST_1K_BUFLEN,
		SPIDEVTEST_2K_BUFLEN, SPIDEVTEST_3K_BUFLEN,
		SPIDEVTEST_4K_BUFLEN, SPIDEVTEST_5K_BUFLEN
	};

	return test_loopback_all_sizes(fd, sizes, ARRAY_SIZE(sizes));
}

static int test_loopback_unaligned(int fd)
{
	int sizes[] = {
		41, SPIDEVTEST_BUFLEN + 8, SPIDEVTEST_3K_BUFLEN + 1,
		SPIDEVTEST_BUFLEN2 + 2, SPIDEVTEST_BUFLEN3 + 3,
		SPIDEVTEST_BUFLEN4 + 4, SPIDEVTEST_1K_BUFLEN,
		SPIDEVTEST_3K_BUFLEN, SPIDEVTEST_1K_BUFLEN + 16,
		SPIDEVTEST_1K_BUFLEN + 15, SPIDEVTEST_1K_BUFLEN + 8*16,
		SPIDEVTEST_1K_BUFLEN + 8,
	};

	return test_loopback_all_sizes(fd, sizes, ARRAY_SIZE(sizes));
}

static int test_loopback_large(int fd)
{
	int sizes[] = {
		SPIDEVTEST_4K_BUFLEN, SPIDEVTEST_5K_BUFLEN,
		SPIDEVTEST_11K_BUFLEN, SPIDEVTEST_12K_BUFLEN,
		SPIDEVTEST_13K_BUFLEN, SPIDEVTEST_15K_BUFLEN,
		SPIDEVTEST_17K_BUFLEN
	};

	return test_loopback_all_sizes(fd, sizes, ARRAY_SIZE(sizes));
}

static int test_loopback_large_unaligned(int fd)
{
	int sizes[] = {
		SPIDEVTEST_4K_BUFLEN+8, SPIDEVTEST_5K_BUFLEN+9,
		SPIDEVTEST_11K_BUFLEN+8, SPIDEVTEST_12K_BUFLEN+12,
		SPIDEVTEST_13K_BUFLEN+18, SPIDEVTEST_15K_BUFLEN,
		SPIDEVTEST_32K_BUFLEN_UN
	};

	return test_loopback_all_sizes(fd, sizes, ARRAY_SIZE(sizes));
}

static int test_aardvark_all_sizes(int fd)
{
	uint8_t tx_buf[SPIDEVTEST_3K_BUFLEN];
	uint8_t rx_buf[SPIDEVTEST_3K_BUFLEN];
	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = ARRAY_SIZE(tx_buf),
	};
	int i, rc, sizes[] = {
		SPIDEVTEST_BUFLEN, SPIDEVTEST_BUFLEN2, SPIDEVTEST_BUFLEN3,
		SPIDEVTEST_BUFLEN4, SPIDEVTEST_1K_BUFLEN, SPIDEVTEST_3K_BUFLEN,
		SPIDEVTEST_1K_BUFLEN + 16, SPIDEVTEST_1K_BUFLEN + 15
	};

	fill_random_buffer(tx_buf, ARRAY_SIZE(tx_buf));
	for (i = 0; i < ARRAY_SIZE(sizes); i++) {
		if (verbose) {
			fprintf(stdout, "Sending buffer of %d size\n",
				sizes[i]);
		}
		data.len = sizes[i];
		rc =  send_buffer(fd, &data, saved_mode);
		if (rc)
			return rc;
	}
	return 0;
}

static int test_aardvark_rx_only(int fd)
{
	uint8_t rx_buf[SPIDEVTEST_BUFLEN] = {0};
	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)NULL,
		.rx_buf = (unsigned long)rx_buf,
		.bits_per_word = 8,
		.cs_change = 0,
		.len = ARRAY_SIZE(rx_buf),
	};

	return send_buffer(fd, &data, saved_mode);
}

static int test_aardvark_tx_only(int fd)
{
	uint8_t tx_buf[50] = {
		0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1C, 0x1C, 0x1D,
		0x1E, 0x1F, 0x1A, 0x1C, 0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1B,
		0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1F, 0x1C, 0x1D, 0x1E, 0x1F,
		0x1A, 0x2A, 0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1B, 0x3B, 0x1D,
		0x1E, 0x1F, 0x2F, 0x5A, 0x8B, 0x1F, 0x1A, 0x1B, 0x3B, 0x1D
	};
	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)NULL,
		.bits_per_word = 8,
		.cs_change = 0,
		.len = ARRAY_SIZE(tx_buf),
	};

	return send_buffer(fd, &data, saved_mode);
}

static int test_aardvark_large(int fd)
{
	uint8_t tx_buf[SPIDEVTEST_3K_BUFLEN],
		rx_buf[SPIDEVTEST_3K_BUFLEN] = {0};

	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.bits_per_word = 8,
		.len = ARRAY_SIZE(tx_buf),
	};

	fill_random_buffer(tx_buf, ARRAY_SIZE(tx_buf));
	return send_buffer(fd, &data, saved_mode);
}

static int test_aardvark(int fd, uint8_t mode)
{
	uint8_t tx_buf[66] = {
		0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1C, 0x1C, 0x1D,
		0x1E, 0x1F, 0x1A, 0x1C, 0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1B,
		0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1F, 0x1C, 0x1D, 0x1E, 0x1F,
		0x1A, 0x2A, 0x1C, 0x1D, 0x1E, 0x1F, 0x1A, 0x1B, 0x3B, 0x1D,
		0x1E, 0x1F, 0x2F, 0x5A, 0x8B, 0x1F, 0x1A, 0x1B, 0x3B, 0x1D,
		0x1E, 0x1F, 0x2F, 0x5A, 0x8B, 0x1F, 0x1A, 0x1B, 0x3B, 0x1D,
		0x1E, 0x1F, 0x2F, 0x5A, 0x8B, 0x1F,
	};

	uint8_t rx_buf[ARRAY_SIZE(tx_buf)] = {0};
	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.bits_per_word = 8,
		.cs_change = 0,
		.len = ARRAY_SIZE(tx_buf),
	};

	return send_buffer(fd, &data, mode&~SPI_CS_HIGH);
}

static int test_aardvark_mode3(int fd)
{
	return test_aardvark(fd, SPI_MODE_3);
}

static int test_aardvark_mode2(int fd)
{
	return test_aardvark(fd, SPI_MODE_2);
}
static int test_aardvark_mode0(int fd)
{
	return test_aardvark(fd, SPI_MODE_0);
}

static int test_aardvark_mode1(int fd)
{
	return test_aardvark(fd, SPI_MODE_1);
}

struct test_t loopback_tests[] = {
	{ "Verify loopback mode data read", test_loopback },
	{ "Verify loopback mode data read with aligned to block size",
		test_loopback_aligned },
	{ "Verify loopback mode data read with unaligned data",
		test_loopback_unaligned },
	{ "Verify loopback large aligned chunks (>=4K)", test_loopback_large },
	{ "Verify loopback large unaligned chunks (>=4K)",
		test_loopback_large_unaligned },
};

/* Manual testcases: these testcases can be used with aardvark device connected
   to the bus. They cannot run automatically, but can be used during
   development. */
struct test_t aardvark_tests[] = {
	{ "Verify regular mode data read mode0", test_aardvark_mode0 },
	{ "Verify regular mode data read mode1", test_aardvark_mode1 },
	{ "Verify regular mode data read mode2", test_aardvark_mode2 },
	{ "Verify regular mode data read mode3", test_aardvark_mode3 },
	{ "Verify aardvark rx only data read", test_aardvark_rx_only },
	{ "Verify aardvark tx only data read", test_aardvark_tx_only },
	{ "Verify large aadvark transaction", test_aardvark_large },
};

int nominal_test(int fd)
{
	int rc;

	rc = test_loopback(fd);

	return rc;
}

/* Test spi_setup for invalid modes */
static int unsupported_modes(int fd)
{
	int i, rc;
	uint8_t unsupported_modes[] = { SPI_LSB_FIRST, SPI_3WIRE };

	for (i = 0; i < ARRAY_SIZE(unsupported_modes); i++) {
			rc = set_mode(fd, unsupported_modes[i]);
			if (verbose)
				fprintf(stdout, "Unsupported mode %#x: rc: "
					"%d\n", unsupported_modes[i], rc);
			if (!rc)
				return -1;
	}

	return 0;
}

/* Test spi_setup for invalid bpw */
static int unsupported_bpw(int fd)
{
	int i, rc;
	uint8_t unsupported_bpw[] = { 3, 33 };

	for (i = 0; i < ARRAY_SIZE(unsupported_bpw); i++) {
			rc = set_bpw(fd, unsupported_bpw[i]);
			if (verbose)
				fprintf(stdout, "Unsupported bpw %#x: rc: %d\n",
					unsupported_bpw[i],  rc);
			if (!rc)
				return -1;
	}

	return 0;
}

/* Test spi_transfer for all kind of invalid values */
static int adversarial_spi_transfer(int fd)
{
	uint8_t tx_buf[SPIDEVTEST_BUFLEN] = {0};
	uint8_t rx_buf[ARRAY_SIZE(tx_buf)] = {0};
	struct spi_ioc_transfer data = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = ARRAY_SIZE(tx_buf),
	};
	int rc;

	fill_random_buffer(tx_buf, ARRAY_SIZE(tx_buf));

	data.len = ARRAY_SIZE(tx_buf);
	data.tx_buf = data.rx_buf = (unsigned long)NULL;
	rc = ioctl(fd, SPI_IOC_MESSAGE(1), data);
	if (verbose)
		fprintf(stdout, "NULL buffers rc: %d\n", rc);
	if (!rc)
		return -1;

	data.tx_buf = (unsigned long)tx_buf;
	data.rx_buf = (unsigned long)rx_buf;
	data.bits_per_word = 3;
	rc = ioctl(fd, SPI_IOC_MESSAGE(1), data);
	if (verbose)
		fprintf(stdout, "Invalid bpw rc: %d\n", rc);
	if (!rc)
		return -1;

	return 0;
}

struct test_t adversarial_tests[] = {
	{ "Verify unsupported modes for spi_setup", unsupported_modes },
	{ "Verify unsupported bpw for spi_setup", unsupported_bpw },
	{ "Verify unsupported values for spi_transfer",
		adversarial_spi_transfer },
};

int adversarial_test(int fd)
{
	int i, rc;

	suppress_errors = 1;
	for (i = 0; i < ARRAY_SIZE(adversarial_tests); i++) {
		if (verbose)
			fprintf(stdout, "Running test: %s\n",
					 adversarial_tests[i].desc);
		rc = adversarial_tests[i].func(fd);
		if (rc)
			goto do_exit;
	}

do_exit:
	suppress_errors = 0;
	return rc;
}

int stress_test(int fd)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(loopback_tests); i++) {
		if (verbose)
			fprintf(stdout, "Running test: %s\n",
					 loopback_tests[i].desc);
		rc = loopback_tests[i].func(fd);
		if (rc)
			return rc;
	}
	return rc;
}

int repeat_test(int fd)
{
	int i, j, rc;

	for (j = 0; j < SPI_REPEAT_COUNT; j++) {
		for (i = 0; i < ARRAY_SIZE(loopback_tests); i++) {
			if (verbose)
				fprintf(stdout, "Running test: %s\n",
						 loopback_tests[i].desc);
			rc = loopback_tests[i].func(fd);
			if (rc)
				return rc;
		}
	}
	return rc;
}

/* Manual tests */
int single_test(int fd)
{
	struct test_t *cur_test = loop ? &loopback_tests[testid] :
					 &aardvark_tests[testid];
	int rc;

	if (verbose)
		fprintf(stdout, "Running test: %s\n", cur_test->desc);
	rc = cur_test->func(fd);

	return rc;
}

static void usage(void);
static int parse_args(int argc, char **argv);

int main(int argc, char **argv)
{
	char *dev_name;
	int fd, rc, i;

	if (argc < 2) {
		usage();
		return 1;
	}
	dev_name = argv[1];
	if (parse_args(argc-1, argv+1)) {
		usage();
		return 1;
	}

	fd = dev_open(dev_name);
	if (fd <= 0)
		return 1;
	rc = save_mode(fd);
	if (rc)
		goto do_exit;

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		if (!(test_set & (1U << i)))
			continue;
		if (verbose)
			fprintf(stdout, "Running test: %s\n",
					tests[i].desc);
		rc = tests[i].func(fd);
		if (rc)
			goto do_exit;
	}

do_exit:
	rc |= set_mode(fd, saved_mode);
	if (fd > 0)
		close(fd);
	if (rc)
		fprintf(stdout, "Test failed=%d\n", rc);
	else
		fprintf(stdout, "Test passed\n");

	return rc;
}

static void usage(void)
{
	int i;

	fprintf(stdout, "Syntax: spidevtest device [-nasrldt:vz:]\n");
	fprintf(stdout, "device is the device name for spidev.\n");
	fprintf(stdout, "in the /dev directory, for example: spidev0.0\n");
	fprintf(stdout, "Note that the spidevtest.sh script determines this "
		"automatically.\n\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "	-n, --nominal		Run nominal tests\n");
	fprintf(stdout, "	-a, --adversarial	Run adversarial "
							"tests\n");
	fprintf(stdout, "	-s, --stress		Run stress tests\n");
	fprintf(stdout, "	-r, --repeat		Run repeat tests\n");

	fprintf(stdout, "	-t [num], --tnum	Run single test\n");
	fprintf(stdout, "	-l, --loopback		Run loopback tests\n");
	fprintf(stdout, "	-d, --aardvark		Run aardvark tests\n");
	fprintf(stdout, "	-z, --max_speed		Change maxspeed\n");
	fprintf(stdout, "	-c, --continue		Continue running after"
							" failed test\n");
	fprintf(stdout, "	-v, --verbose		Run with debug messages"
							"on\n");
	fprintf(stdout, "The currently implemented tests are:\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Looback: Test number	Test \n");
	for (i = 0; i < ARRAY_SIZE(loopback_tests); i++)
		fprintf(stdout, "    %d	%s\n", i, loopback_tests[i].desc);
	fprintf(stdout, "Aardvark: Test number	Test \n");
	for (i = 0; i < ARRAY_SIZE(aardvark_tests); i++)
		fprintf(stdout, "    %d	%s\n", i, aardvark_tests[i].desc);
}

static int parse_args(int argc, char **argv)
{
	struct option longopts[] = {
		{ "nominal",     no_argument,       NULL, 'n'},
		{ "adversarial", no_argument,       NULL, 'a'},
		{ "stress",      no_argument,       NULL, 's'},
		{ "repeat",      no_argument,       NULL, 'r'},

		{ "tnum",        required_argument, NULL, 't'},
		{ "loopback",    no_argument,       NULL, 'l'},
		{ "aardvark",    no_argument,       NULL, 'd'},
		{ "max_speed",   required_argument, NULL, 'z'},

		{ "continue",    no_argument,       NULL, 'c'},
		{ "verbose",     no_argument,       NULL, 'v'},

		{ NULL,          0,                 NULL,  0},
	};
	int command;

	while ((command = getopt_long(argc, argv, "nasrldt:vcz:", longopts,
				      NULL))
		!= -1) {
		switch (command) {
		/* These are automatic tests and will run in loopback mode */
		case 'n':
			test_set |= 1 << spi_nominal;
			break;
		case 'a':
			test_set |= 1 << spi_adversarial;
			break;
		case 's':
			test_set |= 1 << spi_stress;
			break;
		case 'r':
			test_set |= 1 << spi_repeat;
			break;

		/* Those tests can be run manually, upon need */
		case 'l':
			loop = 1;
			break;
		case 'd':
			aardvark = 1;
			break;
		case 't':
			testid = atoi(optarg);
			test_set |= 1 << spi_single;
			break;
		case 'z':
			max_speed = atoi(optarg);
			break;

		case 'c':
			continuous = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Invalid argument: %c\n", command);
			return -1;
		}
	}

	/* Run nominal in default testcase */
	if (!test_set && !loop && !aardvark) {
		test_set |= 1 << spi_nominal;
		return 0;
	}

	/* If not otherwise stated, we run in loopback mode */
	if (!loop && !aardvark)
		loop = 1;

	if (loop && aardvark) {
		fprintf(stderr, "Invalid argument: %c (Loopback cannot be used"
			"with aardvark option\n", command);
		usage();
		return -1;
	}

	/* In case of single test, let's check testid number */
	if ((test_set & 1 << spi_single) &&
	    ((loop && testid >= ARRAY_SIZE(loopback_tests)) ||
	     (aardvark && testid >= ARRAY_SIZE(aardvark_tests)))) {
		fprintf(stderr, "Invalid testid\n");
		usage();
		return -1;
	}

	return 0;
}

