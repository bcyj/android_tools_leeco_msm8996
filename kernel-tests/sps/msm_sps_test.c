/******************************************************************************
 SPS/BAM driver Unit-Test: User-Space part

 -----------------------------------------------------------------------------
  Copyright (c) 2011 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------------
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "msm_sps_test.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

enum test_types {
	NOMINAL,
	ADVERSARIAL,
	REPEAT,
	RELEASE,
};

enum verbosity_level {
	PRINT_ERROR = 0,
	PRINT_WARNING,
	PRINT_INFO,
	PRINT_DEBUG,
};

static int verbosity;
static int iteration_number = 5;
static int interval_ms = 10;
static int failure_number;
int fd;

#define pr_err(msg, args...) do {					\
			if (verbosity >= (int)PRINT_ERROR)			\
				printf("\n"msg, ##args);\
		} while (0)

#define pr_warn(msg, args...) do {					\
			if (verbosity >= (int)PRINT_WARN)			\
				printf("\n"msg, ##args);\
		} while (0)

#define pr_info(msg, args...) do {					\
			if (verbosity >= (int)PRINT_INFO)			\
				printf("\n"msg, ##args);\
		} while (0)

#define pr_debug(msg, args...) do {					\
			if (verbosity >= (int)PRINT_DEBUG)			\
				printf("\n"msg, ##args);\
		} while (0)

static void showerrorandexit(char *message)
{
	printf("%s\n", message);
	failure_number++;
	if (verbosity < (int)PRINT_DEBUG)
		exit(-1);
}

static void nominal_test()
{
	int ret;
	int test_type = 1;

	pr_info("Start running nominal test\n");

	ret = ioctl(fd, MSM_SPS_TEST_TYPE, &test_type);

	if (ret == -ENOTTY || ret == -EFAULT || ret == -EINVAL) {
		pr_err("Fail to start testing; Error Code:%d\n", ret);
		exit(-1);
	}
	else if (ret < 0) {
		pr_info("Nominal test failed\n");
		if (verbosity < (int)PRINT_DEBUG)
		exit(-1);
		else
			failure_number++;
	}
	else if (verbosity < (int)PRINT_DEBUG)
		pr_info("Nominal test passed\n");
}

static void adversarial_test()
{
	int ret;
	int test_type = 2;

	pr_info("Start running adversarial test\n");

	ret = ioctl(fd, MSM_SPS_TEST_TYPE, &test_type);

	if (ret == -ENOTTY || ret == -EFAULT || ret == -EINVAL) {
		pr_err("Fail to start testing; Error Code:%d\n", ret);
		exit(-1);
	}
	else if (ret < 0) {
		pr_info("Adversarial test failed\n");
		if (verbosity < (int)PRINT_DEBUG)
			exit(-1);
		else
			failure_number++;
	}
	else if (verbosity < (int)PRINT_DEBUG)
		pr_info("Adversarial test passed\n");
}

static void repeatability_test()
{
	int i;

	pr_info("Start running repeatability test\n");
	for (i = 1; i <= iteration_number; i++) {
		pr_info("Iteration %d of repeatability test:\n", i);
		nominal_test();
		adversarial_test();
		usleep(interval_ms*1000);
	}

	if (verbosity < (int)PRINT_DEBUG)
		pr_info("Repeatability test passed\n");
}

static void release_test()
{
	pr_info("Start running release test\n");

	nominal_test();
	adversarial_test();
	repeatability_test();

	if (verbosity < (int)PRINT_DEBUG)
		pr_info("Release test passed\n");
}

static int (*test_func[]) () = {
	[NOMINAL] = nominal_test,
	[ADVERSARIAL] = adversarial_test,
	[REPEAT] = repeatability_test,
	[RELEASE] = release_test,
};

static void usage(int ret)
{
	printf("Usage: msm_sps_test [OPTIONS] [TEST_TYPE]...\n"
		"Runs the user space tests specified by the TEST_TYPE\n"
		"parameters.  If no TEST_TYPE is specified, then the release\n"
		" test is run.\n"
		"\n"
		"OPTIONS can be:\n"
		"  -v, --verbose         run with debug messages on\n"
		"                           (debugging level: 0-3).\n"
		"  -i, --iteration       iteration number for repeatability\n"
		"  -t, --interval        interval (ms) between two iterations\n"
		"\n"
		"TEST_TYPE can be:\n"
		"  -n, --nominal         run standard functionality tests\n"
		"  -a, --adversarial     run tests that try to break the \n"
		"                          driver\n"
		"  -p, --repeatability   run 5 iterations of both the \n"
		"                          nominal and adversarial tests\n"
		"  -r, --release         run one iteration of the nominal, \n"
		"                        adversarial and repeatability tests\n"
		"  -h, --help            print this help message and exit\n");

	exit(ret);
}

static unsigned int parse_command(int argc, char *const argv[])
{
	int command;
	unsigned ret = 0;

	struct option longopts[] = {
		{"verbose", required_argument, NULL, 'v'},
		{"iteration", required_argument, NULL, 'i'},
		{"interval", required_argument, NULL, 't'},
		{"nominal", no_argument, NULL, 'n'},
		{"adversarial", no_argument, NULL, 'a'},
		{"repeatability", no_argument, NULL, 'p'},
		{"release", no_argument, NULL, 'r'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0},
	};

	while ((command = getopt_long(argc, argv, "v:i:t:naprh", longopts,
				      NULL)) != -1) {
		switch (command) {
		case 'v':
			verbosity = (int)(strtol(optarg, NULL, 0));
			break;
		case 'i':
			iteration_number = (int)(strtol(optarg, NULL, 0));
			break;
		case 't':
			interval_ms = (int)(strtol(optarg, NULL, 0));
			break;
		case 'n':
			ret |= 1 << NOMINAL;
			break;
		case 'a':
			ret |= 1 << ADVERSARIAL;
			break;
		case 'p':
			ret |= 1 << REPEAT;
			break;
		case 'r':
			ret |= 1 << RELEASE;
			break;
		case 'h':
			usage(0);
		default:
			usage(-1);
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	int i;
	unsigned int test_mask = parse_command(argc, argv);
	int ignore_failure = 0;

	/* Run the release case if none is specified */
	if (test_mask == 0)
		test_mask = 1 << RELEASE;

	fd = open("/dev/sps_test", O_RDWR);
	if (fd < 0)
		showerrorandexit("Unable to open /dev/sps_test\n"
						"Testing exits!\n");

	if (verbosity == (int)PRINT_DEBUG)
		ignore_failure = 1;

	if (ioctl(fd, MSM_SPS_TEST_IGNORE_FAILURE, &ignore_failure))
		showerrorandexit("Fail to set the "
						"IGNORE_FAILURE parameter!\n");

	for (i = 0; i < (int)ARRAY_SIZE(test_func); i++) {
		/* Look for the test that was selected */
		if (!(test_mask & (1U << i)))
			continue;

		/* This test was selected, so run it */
		test_func[i] ();
	}

	if (failure_number == 0) {
		printf("All tests have been passed.\n");
		return 0;
	}
	else
		return -1;
}
