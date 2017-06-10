/******************************************************************************
 -----------------------------------------------------------------------------
  Copyright (c) 2010-2011,2013-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------------
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/msm_adc.h>

#define NUM_THREADS 6
#define NUM_ITERATIONS 5
#define MSM_ADC_DEVICE_NAME "/dev/msm_adc"

#define MAX_CHANNELS_TARGET 50

#define PROC_CPUINFO "/proc/cpuinfo"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#ifndef ANDROID
#include <sys/syscall.h>
#define gettid() ((pid_t) syscall(SYS_gettid))
#endif

struct adc_channels_t {
	char *chan_name[MAX_CHANNELS_TARGET];
	uint32_t num_channels;
	uint32_t num_xoadc_channels;
	uint32_t num_epm_channels;
};

struct adc_channels_t adc_channels[] = {
	{
		.chan_name = {
			/* msm7x30 Fluid LTC ADC channels */
			"I_OPTICAL_MOUSE",
			"I_CAMERA_ANALOG",
			"I_BT",
			"I_ARM11",
			"I_SCORPION",
			"I_MARIMBA_2P2V",
			"I_DDR2_CORE2",
			"I_TOUCH_SCREEN",
			"I_VEE",
			"I_BACKLIGHT",
			"I_LCD",
			"I_MARIMBA_1P8V",
			"I_MARIMBA_1P2V",
			"I_DDR2_IO",
			"I_EBI",
			"I_NAND",
			"I_DDR2_CORE1",
			"I_CAMERA_DIGITAL",
			"I_PLLS",
			"I_PLL2",
			"I_PAD3",
		},
		.num_channels = 21,
		.num_xoadc_channels = 0,
		.num_epm_channels = 21,
	},
	{
		.chan_name = {
			/* msm8x60 xoadc channels */
			"vbatt",
			"vcoin",
			"vcharger_channel",
			"charger_current_monitor",
			"vph_pwr",
			"usb_vbus",
			"pmic_therm",
			"pmic_therm_4K",
			"xo_therm",
			"xo_therm_4K",
			"hdset_detect",
			"chg_batt_amon",
			"msm_therm",
			"batt_therm",
			"batt_id",

		/* msm8x60 Fluid ADSADC channels */
			"I_AUDIO_DSP",
			"I_MAIN_POWER",
			"I_VREG_L16A",
			"I_MICRO_SD",
			"I_CODEC_IO",
			"I_CODEC_VDDCX_1",
			"I_CODEC_ANALOG",
			"I_TOUCH_SCREEN",
			"I_SCORPION_CORE0",
			"I_IMEM",
			"I_SCORPION_CORE1",
			"I_EMMC_VCC",
			"I_DIGITAL_CORE",
			"I_MSM_ISM_VDD2",
			"I_MSM_IO_PAD3",
			"I_IO_PAD2",
			"I_HAPTICS",
			"I_VDDPX1_LPDDR2",
			"I_DRAM_VDD1",
			"I_AMBIENT_LIGHT",
			"I_AMLOED_ELVDD",
			"I_AMLOED_IO",
			"I_AMLOED_MEMORY",
			"I_EMMC_VCCQ",
			"I_HSMI_5V",
			"I_VGREG_L21A",
			"I_VGREG_L13A",
			"I_CAMERA_IO",
			"I_CAMERA_DIGITAL",
			"I_CAMERA_ANALOG",
			"I_DRAM_VDD2",
		},
		.num_channels = 46,
		.num_xoadc_channels = 15,
		.num_epm_channels = 31,
	},
	{
		.chan_name = {
			/* msm8960 xoadc channels */
			"vcoin",
			"vbat",
			"dcin",
			"ichg",
			"vph_pwr",
			"ibat",
			"m4",
			"m5",
			"batt_therm",
			"batt_id",
			"usbin",
			"pmic_therm",
			"625mv",
			"125v",
			"chg_temp",
		},
		.num_channels = 15,
		.num_xoadc_channels = 15,
		.num_epm_channels = 0,
	}
};

#define MSM_ADC_DEV_MAX_XOADC_INFLIGHT 0X15

enum test_types {
	NOMINAL,
	ADVERSARIAL,
	STRESS,
	REPEAT,
	RELEASE,
};

enum verbosity_level {
	PRINT_ERROR = 0,
	PRINT_WARNING,
	PRINT_INFO,
};

static int verbosity = 3;
static int test_epm;

enum target_type {
	MSM_7x30 = 0,
	MSM_8x60,
	MSM_8960
};

enum board_type {
	BOARD_SURF_FFA = 0,
	BOARD_FLUID,
	BOARD_CDP,
	BOARD_MTP
};

static uint32_t chan_start_index, chan_end_index;
static int32_t target_id, board_type;
#define pr_err(msg, args...) do { 					 \
			if (verbosity >= PRINT_ERROR)			 \
				printf("[tid %d] "msg, gettid(), ##args); \
		} while (0)

#define pr_warn(msg, args...) do { 					 \
			if (verbosity >= PRINT_WARN)			 \
				printf("[tid %d] "msg, gettid(), ##args); \
		} while (0)


#define pr_info(msg, args...) do { 					 \
			if (verbosity >= PRINT_INFO)			 \
				printf("[tid %d] "msg, gettid(), ##args); \
		} while (0)

int channel_lookup(int fd, uint32_t *chan_num_table)
{
	struct msm_adc_lookup lookup;
	int rc = 0;
	uint32_t cpsize, i;

	for (i = chan_start_index; i <= chan_end_index; i++) {

		/* make sure we copy the NULL terminator */
		cpsize = strlen(adc_channels[target_id].chan_name[i]) + 1;
		if (cpsize > MSM_ADC_MAX_CHAN_STR) {
			pr_err("%s: channel string too long\n", __func__);
			return -EINVAL;
		}
		strncpy(lookup.name, adc_channels[target_id].chan_name[i], cpsize);

		rc = ioctl(fd, MSM_ADC_LOOKUP, &lookup);
		if (rc < 0) {
			pr_err("lookup request failed for chan %s\n", adc_channels[target_id].chan_name[i]);
			perror("ioctl");
			rc++;
			continue;
		}

		pr_info("channel: %s hwmon index: %u\n", lookup.name, lookup.chan_idx);
		chan_num_table[(i - chan_start_index)] = lookup.chan_idx;
	}

	return rc;
}

static int table_request_blocking_conversions(int fd,
		              uint32_t *chan_num_table, uint32_t nchans)
{
	int rc, errors = 0;
	uint32_t i;
	struct adc_chan_result conv;

	for (i = 0; i < nchans; i++) {
		conv.chan = chan_num_table[i];
		rc = ioctl(fd, MSM_ADC_REQUEST, &conv);
		if (rc) {
      if (errno == EBUSY)
        continue;
			pr_err("conversion request failed for chan %u\n",
								conv.chan);
			perror("ioctl");
			/* The modem driver often takes i2c errors and
			 * reports invalid data. Let's not treat this
			 * as an error for now.*/
			if (errno != ENODATA) {
				printf("%s: ENODATA received\n", __func__);
				errors++;
			}
			continue;

		}
		pr_info("blocking request for chan %u result %lld\n",
							conv.chan, conv.physical);
	}

	return errors;
}

static int queue_max_conversions_chan(int fd, uint32_t chan)
{
	int rc, errors = 0;
	uint32_t i;
	struct adc_chan_result conv;

	for (i = 0; i < MSM_ADC_DEV_MAX_XOADC_INFLIGHT; i++) {
		conv.chan = chan;
		rc = ioctl(fd, MSM_ADC_AIO_REQUEST, &conv);
		if (rc) {
			pr_err("conversion request failed for chan %u\n", conv.chan);
			perror("ioctl");
			errors++;
		}
		pr_info("non-blocking request for chan %u\n", conv.chan);
	}

	return errors;
}

static int poll_read(int fd, uint32_t queued)
{
	int rc = 0;
	uint32_t completed, j, i = 0, errors = 0;
	struct adc_chan_result result;

	pr_info("total expected number of completed transfers: %u\n", queued);

	while (i < queued) {
		rc = ioctl(fd, MSM_ADC_AIO_POLL, &completed);
		if (rc) {
			pr_err("poll failed\n");
			perror("ioctl");
			errors++;
			break;
		}

		pr_info("ready to read after poll: %u\n", completed);

		for (j = 0; j < completed; j++) {
			rc = ioctl(fd, MSM_ADC_AIO_READ, &result);
			if (rc) {
				pr_err("aio read failed\n");
				perror("ioctl");
				/* The modem driver often takes i2c errors and
				 * reports invalid data. Let's not treat this
				 * as an error for now.*/
				if (errno != ENODATA)
					errors++;
				i++;
				continue;
			}
			pr_info("chan: %u result: %lld i: %d\n", result.chan,
				result.physical, i);
			i++;
		}
	}

	return errors;
}

static int table_request_nonblocking_conversions(int fd,
						 uint32_t block_res,
				                 uint32_t *chan_num_table, uint32_t nchans)
{
	int rc;
	uint32_t i, j, completed, errors = 0, queued = 0;
	struct adc_chan_result conv;

	for (i = 0; (i < nchans && i < adc_channels[target_id].num_xoadc_channels); i++) {
		conv.chan = chan_num_table[i];
		rc = ioctl(fd, block_res ? MSM_ADC_AIO_REQUEST_BLOCK_RES :
					   MSM_ADC_AIO_REQUEST, &conv);
		/* 
		 * We have pending requests and there are no more resources,
		 * so we aren't allowed to block since we might deadlock. :( 
		 */
		if (rc && errno == EBUSY && block_res == 1) {
			if (queued == 0) {
				pr_info("queue filled by other threads\n");
				break;
			}

			pr_info("No resources after queueing %u requests,"
					" so draining pipeline\n", queued);

			rc = poll_read(fd, queued);
			if (rc) {
				errors += rc;
				break;
			}
			queued = 0;
		}
		else if (rc) {
			pr_err("conversion request failed for chan %u\n", conv.chan);
			perror("ioctl");
			errors++;
			continue;
		}
		else {
			pr_info("non-blocking request for chan %u queued\n", conv.chan);
			queued++;
		}
	}

	pr_info("queued all channels - now waiting on %u replies\n", queued);

	if (queued) {
		rc = poll_read(fd, queued);
		errors += rc;
	}

	return errors;
}

static int platform_detect()
{
	FILE *fp = NULL;
	char platform_str[100];

	target_id = board_type = -1;

	fp = fopen(PROC_CPUINFO, "r");
	if(!fp) {
		pr_err("fopen failed\n");
		return errno;
	}
	while (fgets(platform_str, sizeof(platform_str), fp)) {
		if(strncmp(platform_str, "Hardware", strlen("Hardware")) == 0) {
			printf("\nString : %s\n", platform_str);
			if(strcasestr(platform_str, "7X30") != NULL)
				target_id = MSM_7x30;
			else if(strcasestr(platform_str, "8X60") != NULL)
				target_id = MSM_8x60;
			else if(strcasestr(platform_str, "8960") != NULL)
				target_id = MSM_8960;

			if(strcasestr(platform_str, "FFA") != NULL || 
					strcasestr(platform_str, "SURF") != NULL)
				board_type = BOARD_SURF_FFA;
			else if(strcasestr(platform_str, "FLUID") != NULL)
				board_type = BOARD_FLUID;
			else if(strcasestr(platform_str, "CDP") != NULL)
				board_type = BOARD_CDP;
			else if(strcasestr(platform_str, "MTP") != NULL)
				board_type = BOARD_MTP;
			break;
		}
	}
	fclose(fp);
	return 0;
}

static int find_channel_range(int fd, uint32_t *num_chans)
{
	int ret = 0;
	int32_t res = 0;
	if (test_epm) {
		ret = ioctl(fd, MSM_ADC_FLUID_INIT, &res);
		if (res == -EINVAL) {
			printf("\nEPM initialization failed\n");
			ret = -1;
		}
	}
	chan_start_index = 0;
	chan_end_index = (test_epm) ? (adc_channels[target_id].num_channels - 1) : 
		(adc_channels[target_id].num_xoadc_channels - 1);

	*num_chans = chan_end_index - chan_start_index + 1;
	return ret;
}

static int nominal_test(char *name)
{
	int fd, rc;
	uint32_t chan_num_table[MAX_CHANNELS_TARGET];
	uint32_t res;
	uint32_t num_chans = 0;
	uint32_t errors = 0;

	pr_info("Running nominal test\n");

	fd = open(MSM_ADC_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
		/* This failure will happen for 7x30 Surf/FFA always */
		perror("open failed\n");
		return fd;
	}

	if (find_channel_range(fd, &num_chans)) {
		errors++;
	}

	pr_info("\nnum_chans = %u", num_chans);

	rc = channel_lookup(fd, chan_num_table);
	if (rc) {
		errors += rc;
		goto nominal_test_err;
	}

	rc = table_request_blocking_conversions(fd, chan_num_table, num_chans);
	if (rc) {
		errors += rc;
		goto nominal_test_err;
	}

	if (chan_start_index == 0) {
		rc = table_request_nonblocking_conversions(fd, 0, chan_num_table, num_chans);
		if (rc)
			errors += rc;
	}

nominal_test_err:
	rc = ioctl(fd, MSM_ADC_FLUID_DEINIT, &res);
	close(fd);

	return errors;
}

static int adversarial_test(char *name)
{
	int fd, rc;
	uint32_t chan_num_table[MAX_CHANNELS_TARGET];
	uint32_t num_chans = 0, res;
	uint32_t completed, errors = 0;
	struct adc_chan_result conv;

	pr_info("Running adversarial test\n");

	fd = open(MSM_ADC_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
		/* This failure will happen for 7x30 Surf/FFA always */
		perror("open failed\n");
		return fd;
	}

	if (find_channel_range(fd, &num_chans)) {
		errors++;
	}

	pr_info("\nnum_chans = %u", num_chans);

	if (chan_start_index != 0)
		return 0;

	rc = channel_lookup(fd, chan_num_table);
	if (rc) {
		errors += rc;
		goto adversarial_test_err;
	}

	/*
	 * At this point we should have 0 outstanding transactions.
	 * POLL should not be useable in such situations.
	 */
	rc = ioctl(fd, MSM_ADC_AIO_POLL, &completed);
	if (!rc || errno != EDEADLK) {
		pr_err("poll didn't fail even though it should have, returned %d\n", rc);
		perror("ioctl");
		errors++;
	}

	rc = queue_max_conversions_chan(fd, chan_num_table[0]);
	if (rc)
		errors++;

	/*
	 * We should be completely out of slot resources at this point,
	 * so this call should fail.
	 */
	conv.chan = chan_num_table[0];
	rc = ioctl(fd, MSM_ADC_AIO_REQUEST, &conv);
	if (!rc || errno != EBUSY) {
		pr_err("conversion request should be out of resources %d\n", errno);
		perror("ioctl");
		errors++;
	}

	/*
	 * Verify we fail if we attempt to block on resource availability.
	 * This is not permitted since we have pending requests, and
	 * blocking could lead to deadlocks.
	 */
	rc = ioctl(fd, MSM_ADC_AIO_REQUEST_BLOCK_RES, &conv);
	if (!rc || errno != EBUSY) {
		pr_err("block res conversion request didn't fail when"
		       " it should have. %d\n", errno);
		perror("ioctl");
		errors++;
	}
adversarial_test_err:
	rc = ioctl(fd, MSM_ADC_FLUID_DEINIT, &res);
	close(fd);

	return errors;
}

static void *stress_test_thread(void *arg)
{
	static int thread_cnt = 0;
	uint32_t thread_num = (uint32_t)arg;
	uint32_t chan_num_table[MAX_CHANNELS_TARGET];
	int fd, rc, i;
	uint32_t res, num_chans = (uint32_t)arg;
	uint32_t errors = 0;

	thread_num = ++thread_cnt;

	pr_info("Thread %d starting\n", thread_num);

	for (i = 0; i < NUM_ITERATIONS; i++)
	{
		fd = open(MSM_ADC_DEVICE_NAME, O_RDWR);
	  if (fd < 0) {
	 	  perror("open failed\n");
		  pthread_exit((void *)1);
	  }

		rc = channel_lookup(fd, chan_num_table);
		if (rc) {
			errors = rc;
			goto stress_test_thread_err;
		}

		/* Divy up the work so we test multiple modes at once */
		switch (thread_num % 2) {
		case 0:
			rc = table_request_blocking_conversions(fd,
							chan_num_table, num_chans);
			if (rc) {
				errors += rc;
			}
			break;
		case 1:
			if (chan_start_index == 0) {
				rc = table_request_nonblocking_conversions(fd, 1,
								chan_num_table, num_chans);
				if (rc)
					errors += rc;
			}
			break;
		}

	  close(fd);

		if (errors) {
			pr_err("Thread %d failed at iteration %d\n",
					thread_num, i);
			break;
		}
	}

	pr_info("Thread %d exiting with %d errors\n", thread_num, errors);
stress_test_thread_err:
	pthread_exit((void *)errors);

	return NULL;
}

static int stress_test(char *name)
{
	int i, rc, thread_errs, errors = 0;
	static pthread_t threads[NUM_THREADS];
	int fd;
	uint32_t num_chans = 0, res;

  /* Initialize the channel range  */
	fd = open(MSM_ADC_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
		/* This failure will happen for 7x30 Surf/FFA always */
		perror("open failed\n");
		return fd;
	}

	if (find_channel_range(fd, &num_chans)) {
		errors++;
	}

	rc = ioctl(fd, MSM_ADC_FLUID_DEINIT, &res);
	close(fd);

	for (i = 0; i < NUM_THREADS; i++) {
		rc = pthread_create(&threads[i], NULL, stress_test_thread,
								(void *)num_chans);
		if (rc) {
			pr_err("Couldn't create thread %d\n", i + 1);
			return 1;
		}

	}

	for (i = 0; i < NUM_THREADS; i++) {
		rc = pthread_join(threads[i], (void **)&thread_errs);
		if (rc) {
			pr_err("Couldn't join thread %d\n", i + 1);
			return 1;
		}
		errors += thread_errs;

	}

	return errors;
}

static int release_test(char *name)
{
	int errors;

	errors = nominal_test(name);
	if (errors)
		return errors;

	errors = adversarial_test(name);
	if (errors)
		return errors;

	errors = stress_test(name);
	if (errors)
		return errors;

	return 0;
}

static int repeat_test(char *name)
{
	int i, errors = 0;

	for (i = 0; i < NUM_ITERATIONS; i++) {
		errors = nominal_test(name);
		if (errors)
			break;
		errors = adversarial_test(name);
		if (errors)
			break;
	}

	return errors;
}

static int (*test_func[]) (char *) = {
	[NOMINAL] = nominal_test,
	[ADVERSARIAL] = adversarial_test,
	[STRESS] = stress_test,
	[RELEASE] = release_test,
	[REPEAT] = repeat_test,
};

static void usage(int ret)
{
	printf("Usage: msm_adc_test [OPTIONS] [TEST_TYPE]...\n"
	       "Runs the user space tests specified by the TEST_TYPE\n"
	       "parameters.  If no TEST_TYPE is specified, then the release\n"
	       " test is run.\n"
	       "\n"
	       "OPTIONS can be:\n"
	       "  -v, --verbose         run with debug messages on (NOTE:\n"
	       "                           currently has no effect).\n"
	       "\n"
	       "TEST_TYPE can be:\n"
	       "  -n, --nominal         run standard functionality tests\n"
	       "  -a, --adversarial     run tests that try to break the \n"
	       "                          driver\n"
	       "  -s, --stress          run tests that try to maximize the \n"
	       "                          capacity of the driver\n"
	       "  -r, --release         run one iteration of the nominal, \n"
	       "                          adversarial and stress tests\n"
	       "  -p, --repeatability   run 10 iterations of both the \n"
	       "                          nominal and adversarial tests\n"
	       "  -e, --epm	        Include epm channels for testing \n"
	       "  -h, --help            print this help message and exit\n");

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
		{"release", no_argument, NULL, 'r'},
		{"repeatability", no_argument, NULL, 'p'},
		{"epm", no_argument, NULL, 'e'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0},
	};

	while ((command = getopt_long(argc, argv, "v:nasrpeh", longopts,
				      NULL)) != -1) {
		switch (command) {
		case 'v':
			verbosity = (int)(strtol(optarg, NULL, 0));
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
			ret |= 1 << RELEASE;
			break;
		case 'p':
			ret |= 1 << REPEAT;
			break;
		case 'e':
			test_epm = 1;
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
	int rc = 0, num_tests_failed = 0, i;
	uint32_t test_mask = 0;

	platform_detect();
	if(target_id == -1 || board_type == -1)	{
		printf("Error: Platform detection failed\n");
		return 0;
	}

	printf("\ntarget_id = %d, board_type = %d\n", target_id, board_type);

	test_mask = parse_command(argc, argv);

	if (test_epm && (board_type == BOARD_SURF_FFA || 
				target_id == MSM_8960)) {
		printf("Target board does not support EPM channels\n");
		return -1;
	}

	if (target_id == MSM_7x30) {
		if (board_type == BOARD_SURF_FFA) {
			printf("No ADC channels supported on this board\n");
			return -1;
		}
		else if (board_type == BOARD_FLUID && !test_epm) {
			printf("No XoADC channels on this board." 
				"Please enable EPM channels testing\n");
			return -1;
		}
	}

	/* Run the nominal case if none is specified */
	if (test_mask == 0)
		test_mask = 1 << NOMINAL;


	for (i = 0; i < (int)ARRAY_SIZE(test_func); i++) {
		/* Look for the test that was selected */
		if (!(test_mask & (1U << i)))
			continue;

		/* This test was selected, so run it */
		rc = test_func[i] (argv[argc - 1]);

		if (rc) {
			fprintf(stderr, "Test failed! rc: %d\n", rc);
			num_tests_failed++;
		} else {
			printf("Test passed\n");
		}
	}

	printf("%d TESTS FAILED\n", num_tests_failed);

	return -num_tests_failed;
}
