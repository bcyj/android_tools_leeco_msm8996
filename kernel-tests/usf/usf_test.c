/******************************************************************************
 -----------------------------------------------------------------------------
  Copyright (c) 2011 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------------
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <usf.h>

static const unsigned short PortSamplesDataSize = 768;


#define NUM_ITERATIONS 5
#define	NUM_ITERATIONS_STRESS	100

#define USF_DEVICE_NAME "/dev/usf1"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

enum test_types {
	NOMINAL,
	ADVERSARIAL,
	STRESS,
	REPEAT,
};

typedef enum {false, true} bool;

// Whether the USF is supported on the current platform
// If the usf device can't be oppened, it's assumed, the USF isn't supported
static bool s_usf_supported = true;

// "IF with the USF" file descriptor
static int s_usf_fd = -1;

// One buffer (in the cyclic queue) is for one group of the US frames
// The US data buffer size (bytes) in the cyclic queue
static unsigned int  s_tx_buf_size = 0;
// Number of the buffers in the cyclic queue
static unsigned short  s_tx_buf_number = 0;

// TX US data memory (shared with the USF) size
static unsigned int s_tx_alloc_size = 0;
// TX US data memory, shared with the USF
static unsigned char* s_tx_user_addr = NULL;

// Free region (in the cyclic queue) for writing by the USF
// Pointer (read index) to the end of available region
// in the shared US data memory
static	unsigned short	s_tx_read_index = 0;

// Ready region (in the cyclic queue) with US data for a client
// Pointer (write index) to the end of ready US data region
// in the shared memory
static	unsigned short	s_tx_write_index = 0;

// TX configuration
static us_tx_info_type	s_tx_info;

// TX configuration
static us_rx_info_type	s_rx_info;

///////////////////////////////////////////////////////////////////////
// One buffer (in the cyclic queue) is for one group of the US frames
// The US data buffer size (bytes) in the cyclic queue
static unsigned int  s_rx_buf_size = 0;
// Number of the buffers in the cyclic queue
static unsigned short  s_rx_buf_number = 0;

// RX US data memory (shared with the USF) size
static unsigned int s_rx_alloc_size = 0;
// RX US data memory, shared with the USF
static unsigned char* s_rx_user_addr = NULL;

// Free region (in the cyclic queue) for writing by client
static	unsigned short	s_rx_read_index = 0;
// Ready region (in the cyclic queue) for US data transmition
static	unsigned short	s_rx_write_index = 0;

typedef void (*set_xx_configuration)(void);
// Valid TX configurations
static void set_valid_tx_configuration1();

set_xx_configuration set_valid_tx_configuration[] =
{
	set_valid_tx_configuration1
};

// Valid RX configurations
static void set_valid_rx_configuration1();

set_xx_configuration set_valid_rx_configuration[] =
{
	set_valid_rx_configuration1
};


typedef bool (*configure_xx_invalid)(void);

static bool configure_tx_invalid1();
static bool configure_tx_invalid2();

configure_xx_invalid configure_tx_invalid[] =
{
	configure_tx_invalid1,
	configure_tx_invalid2
};

// Invalid RX configurations
static bool configure_rx_invalid1();

configure_xx_invalid configure_rx_invalid[] =
{
	configure_rx_invalid1
};


// Valid configurations implementations
static void set_valid_tx_configuration1()
{
	const unsigned short FrameHdrSizeBytes = 12;

	typedef struct {
	  unsigned short skipFactor;
	  unsigned short groupFactor;
	  unsigned int frameSize;
	} TransparentDataTxType;
	static TransparentDataTxType transparentTxData;
	transparentTxData.skipFactor = 1;
	transparentTxData.groupFactor = 2;
	transparentTxData.frameSize = PortSamplesDataSize;

	s_tx_info.us_xx_info.client_name = "tester";
	s_tx_info.us_xx_info.dev_id = 0;
	s_tx_info.us_xx_info.stream_format = USF_RAW_FORMAT;
	s_tx_info.us_xx_info.sample_rate = 96000;
	s_tx_info.us_xx_info.buf_num = 8;
	s_tx_info.us_xx_info.port_cnt = 3;
	s_tx_info.us_xx_info.port_id[0] = 1;
	s_tx_info.us_xx_info.port_id[1] = 2;
	s_tx_info.us_xx_info.port_id[2] = 5;
	s_tx_info.us_xx_info.bits_per_sample = 16;

	s_tx_info.us_xx_info.params_data_size = sizeof(TransparentDataTxType);
	s_tx_info.us_xx_info.params_data = (unsigned char*)&transparentTxData;

	unsigned short frame_size = PortSamplesDataSize*
				(s_tx_info.us_xx_info.bits_per_sample/8) *
				s_tx_info.us_xx_info.port_cnt +
				FrameHdrSizeBytes;
	s_tx_info.us_xx_info.buf_size = frame_size *
					transparentTxData.groupFactor;

	s_tx_info.input_info.event_types = USF_ALL_EVENTS;
	s_tx_info.input_info.tsc_x_dim[0] = 0;
	s_tx_info.input_info.tsc_x_dim[1] = 480;
	s_tx_info.input_info.tsc_y_dim[0] = 0;
	s_tx_info.input_info.tsc_y_dim[1] = 800;

	s_tx_info.input_info.tsc_x_fuzz = 0;
	s_tx_info.input_info.tsc_y_fuzz = 0;

	s_tx_info.input_info.tsc_pressure[0] = 0;
	s_tx_info.input_info.tsc_pressure[1] = 1;

} // set_valid_tx_configuration1

static void set_valid_rx_configuration1()
{
	typedef struct {
	  unsigned short frameSize;
	  unsigned short groupFactor;
	} TransparentDataRxType;
	static TransparentDataRxType transparentRxData;
	unsigned short frame_size = 0;

	transparentRxData.frameSize = PortSamplesDataSize;
	transparentRxData.groupFactor = 1;

	s_rx_info.us_xx_info.client_name = "tester";
	s_rx_info.us_xx_info.dev_id = 0;
	s_rx_info.us_xx_info.stream_format = USF_RAW_FORMAT;
	s_rx_info.us_xx_info.sample_rate = 96000;
	s_rx_info.us_xx_info.buf_num = 3;
	s_rx_info.us_xx_info.port_cnt = 1;
	s_rx_info.us_xx_info.port_id[0] = 1;
	s_rx_info.us_xx_info.bits_per_sample = 16;
	s_rx_info.us_xx_info.params_data_size = sizeof(TransparentDataRxType);
	s_rx_info.us_xx_info.params_data = (unsigned char*)&transparentRxData;

	frame_size = PortSamplesDataSize *
				(s_rx_info.us_xx_info.bits_per_sample/8) *
				s_rx_info.us_xx_info.port_cnt;
	// group size
	s_rx_info.us_xx_info.buf_size = frame_size *
					transparentRxData.groupFactor;
} // set_valid_rx_configuration1

bool configure_tx_valid(unsigned short config_ind)
{
	// US TX device (& stream) configuration
	bool rc = true;
	int ret = 0;

	printf("%s: start; config_ind=%d\n",
	       __FUNCTION__,
	       config_ind);
	if ((config_ind < ARRAY_SIZE(set_valid_tx_configuration)) &&
	    (set_valid_tx_configuration[config_ind] != NULL)) {
		(*set_valid_tx_configuration[config_ind])();
	}
	else {
		printf ("%s: wrong config_ind=%d\n",
			__FUNCTION__,
			config_ind);
		return false;
	}

	ret = ioctl (s_usf_fd, US_SET_TX_INFO, &s_tx_info);
	if (ret < 0) {
		printf ("%s: ioctl(US_SET_TX_INFO) failed. ret=%d err=%d\n",
			__FUNCTION__,
			ret, errno);
		return false;
	}

	s_tx_buf_size = s_tx_info.us_xx_info.buf_size;
	s_tx_buf_number = s_tx_info.us_xx_info.buf_num;
	s_tx_alloc_size = s_tx_buf_size * s_tx_buf_number;
	if (s_tx_alloc_size > 0) { // Data path is required
		// request memory mapping
		s_tx_user_addr=(unsigned char*)(mmap(0, s_tx_alloc_size,
						     PROT_READ, MAP_SHARED,
						     s_usf_fd, 0));
		if (s_tx_user_addr == MAP_FAILED) {
			printf("%s: mmap failed(); err=%d\n",
			       __FUNCTION__, errno);
			s_tx_user_addr = NULL;
			s_tx_alloc_size = 0;
			s_tx_buf_size = 0;
			s_tx_buf_number = 0;
			rc = false;
		}
	}

	return rc;
} // configure_tx_valid

static bool configure_rx_valid(unsigned short config_ind)
{
	bool rc = true;
	int ret = 0;

	printf("%s: start; config_ind=%d\n", __FUNCTION__, config_ind);
	if ((config_ind < ARRAY_SIZE(set_valid_rx_configuration)) &&
	    (set_valid_rx_configuration[config_ind] != NULL)) {
		(*set_valid_rx_configuration[config_ind])();
	}
	else {
		printf("%s: wrong config_ind=%d\n", __FUNCTION__, config_ind);
		return false;
	}

	ret = ioctl (s_usf_fd, US_SET_RX_INFO, &s_rx_info);
	if (ret < 0) {
		printf("%s: ioctl(US_SET_RX_INFO) failed. ret=%d err=%d\n",
		       __FUNCTION__,
		       ret, errno);
		return false;
	}

	s_rx_buf_size = s_rx_info.us_xx_info.buf_size;
	s_rx_buf_number = s_rx_info.us_xx_info.buf_num;
	s_rx_alloc_size = s_rx_buf_size * s_rx_buf_number;
	if (s_rx_alloc_size > 0) { // Data path is required
		// request memory mapping
		s_rx_user_addr=(unsigned char*)(mmap(0,s_rx_alloc_size,
						PROT_WRITE, MAP_SHARED,
						s_usf_fd, 0));

		if (s_rx_user_addr == MAP_FAILED) {
			printf("%s: mmap failed(); err=%d\n",
				 __FUNCTION__, errno);
			s_rx_user_addr = NULL;
			s_rx_alloc_size = 0;
			s_rx_buf_size = 0;
			s_rx_buf_number = 0;
			rc = false;
		}
	}

	return rc;
} // configure_rx_valid

// Invalid configurations implementations
bool configure_tx_invalid1()
{
	// US TX device (& stream) configuration
	bool rc = true;
	int ret = 0;

	printf("%s: start; \n", __FUNCTION__);
	set_valid_tx_configuration1();

	s_tx_info.us_xx_info.port_cnt = 10;
	printf("%s:  invalid port_cnt(%d)was set \n",
		__FUNCTION__, s_tx_info.us_xx_info.port_cnt);

	ret = ioctl (s_usf_fd, US_SET_TX_INFO, &s_tx_info);
	if (ret < 0) {
		printf( "%s: US_SET_TX_INFO failed as supposed; err=%d\n",
			  __FUNCTION__, errno);
		return false;
	}

	return rc;
} // configure_tx_invalid1

// Invalid configurations implementations
bool configure_tx_invalid2()
{
	// US TX device (& stream) configuration
	bool rc = true;
	int ret = 0;

	printf("%s: start; \n", __FUNCTION__);
	ret = ioctl (s_usf_fd, US_SET_TX_INFO, NULL);
	printf("%s:  invalid NULL ptr; ret=%d \n", __FUNCTION__, ret);
	if (ret < 0) {
		printf( "%s: US_SET_TX_INFO failed as supposed; err=%d\n",
			  __FUNCTION__, errno);
		return false;
	}

	return rc;
} // configure_tx_invalid2

static bool configure_rx_invalid1()
{
	bool rc = true;
	int ret = 0;

	printf("%s: start\n", __FUNCTION__);
	ret = ioctl (s_usf_fd, US_SET_RX_INFO, NULL);
	printf("%s:  invalid NULL ptr; ret=%d \n", __FUNCTION__, ret);
	if (ret < 0) {
		printf( "%s: US_SET_RX_INFO failed as supposed; err=%d\n",
			  __FUNCTION__, errno);
		return false;
	}

	return rc;
} // configure_rx_invalid1

static bool start_TX()
{
	bool  rc = true;

	printf("%s: start\n", __FUNCTION__);
	if (s_usf_fd == -1 ) {
		printf("%s: UAL isn't opened\n", __FUNCTION__);
		rc = false;
	}
	else {
		int ret = ioctl (s_usf_fd, US_START_TX, NULL);
		rc = (ret >= 0);
	}

	return rc;
} // start_TX

static bool start_RX()
{
	bool  rc = true;

	printf("%s: start\n", __FUNCTION__);
	if (s_usf_fd == -1 ) {
		printf("%s: UAL isn't opened\n", __FUNCTION__);
		rc = false;
	}
	else {
		int ret = ioctl (s_usf_fd, US_START_RX, NULL);
		rc = (ret >= 0);
	}

	return rc;
} // start_RX

static bool stop_TX()
{
	bool  rc = true;

	printf("%s: start\n", __FUNCTION__);
	if (s_usf_fd == -1 ) {
		printf("%s: UAL isn't opened\n", __FUNCTION__);
		rc = false;
	}
	else {
		int ret = ioctl (s_usf_fd, US_STOP_TX, NULL);
		if ((ret < 0) &&
			(errno != EBADFD) ) {
			printf("%s: (US_STOP_TX) failed. ret=%d\n",
				 __FUNCTION__, ret);
			rc = false;
		}

		// Release TX shared US data memory
		if (s_tx_user_addr != NULL) {
			ret = munmap (s_tx_user_addr, s_tx_alloc_size);
			if (ret != 0) {
				printf("%s: (TX munmap) failed. ret=%d\n",
					 __FUNCTION__, ret);
				rc = false;
			}
		}

		s_tx_user_addr = NULL;
		s_tx_alloc_size = 0;
		s_tx_buf_number = 0;
		s_tx_buf_size = 0;
		s_tx_read_index = 0;
		s_tx_write_index = 0;
	}

	return rc;
} // stop_TX

static bool stop_RX()
{
	bool  rc = true;

	printf("%s: start\n", __FUNCTION__);
	if (s_usf_fd == -1 ) {
		printf("%s: UAL isn't opened\n", __FUNCTION__);
		rc = false;
	}
	else {
		int ret = ioctl (s_usf_fd, US_STOP_RX, NULL);
		if ((ret < 0) &&
			(errno != EBADFD) ) {
			printf("%s: (US_STOP_RX) failed. ret=%d\n",
				 __FUNCTION__, ret);
			rc = false;
		}

		// Release RX shared US data memory
		if (s_rx_user_addr != NULL) 	{
			ret = munmap (s_rx_user_addr, s_rx_alloc_size);
			if (ret != 0) {
				printf("%s: (RX munmap) failed. ret=%d\n",
					 __FUNCTION__, ret);
				rc = false;
			}
		}
		s_rx_user_addr = NULL;
		s_rx_alloc_size = 0;
		s_rx_buf_number = 0;
		s_rx_buf_size = 0;
		s_rx_read_index = 0;
		s_rx_write_index = 0;
	}

	return rc;
} // stop_RX

static bool usf_close()
{
	bool  rc = true;

	printf("%s: start\n", __FUNCTION__);
	if (s_usf_fd == -1 ) {
		printf("%s: UAL isn't opened\n", __FUNCTION__);
		rc = false;
	}
	else {
		rc = stop_TX();
		rc = stop_RX() && rc;
	}

	return rc;
} // usf_close


static bool usf_write(unsigned char* data, unsigned short data_size)
{
	us_rx_update_info_type update_info;
	bool rc = true;
	int ret = 0;

	printf("%s: start\n", __FUNCTION__);
	if ((s_usf_fd == -1 ) ||
		(data == NULL) ||
		(data_size > s_rx_buf_size) ) {
		printf("%s: invalid parameters: data=0x%p; size=%d; fd[%d]\n",
		       __FUNCTION__, data, data_size, s_usf_fd);
		return false;
	}

	update_info.ready_region = s_rx_write_index;
	update_info.params_data = NULL;
	update_info.params_data_size = 0;

	ret = ioctl(s_usf_fd, US_SET_RX_UPDATE, &update_info);
	if (ret < 0) {
		printf("%s: (US_SET_RX_UPDATE) failed. ret=%d\n",
			 __FUNCTION__, ret);
		rc = false;
	}
	else {
		s_rx_read_index = update_info.free_region;
	}

	return rc;
} // usf_write

static bool usf_read()
{
	us_tx_update_info_type update_info;
	bool rc = true;
	int ret = 0;

	printf("%s: start\n", __FUNCTION__);
	if (s_usf_fd == -1 ) {
		printf("%s: invalid parameters: UAL not opened[%d]\n",
		       __FUNCTION__, s_usf_fd);
		return false;
	}

	// Input Events injection isn't tested here
	update_info.event = NULL;
	update_info.event_counter = 0;

	// ual_read() call means, the region from the previous call
	// may be released (that is, the queue is empty)
	s_tx_read_index = s_tx_write_index;

	update_info.params_data = NULL;
	update_info.params_data_size = 0;
	update_info.ready_region = 0;
	update_info.free_region = s_tx_read_index;

	ret = ioctl (s_usf_fd, US_GET_TX_UPDATE, &update_info);
	if ((ret < 0) &&
		(errno != ETIME) ) {
		printf("%s: (US_GET_TX_UPDATE) failed. ret=%d\n",
			 __FUNCTION__, ret);
		rc = false;
	}
	else {
		s_tx_write_index =  update_info.ready_region;
	}

	return rc;
} // usf_read

static int open_usf(void)
{
	int rc = 0;

	s_usf_fd = open (USF_DEVICE_NAME, O_RDWR);
	if (s_usf_fd <= 0) {
		if (errno == ENOENT) {
			s_usf_supported = false;
			printf("%s: usf driver is not supported\n",
				__FUNCTION__);
		}
		else {
			rc = 1;
			printf("%s: open usf device failed; errno=%d\n",
				__FUNCTION__, errno);
		}
	}

	return rc;
} // open_usf

//////////////////////////////////////////////////////////////////////////////
static int nominal_test(void)
{
	bool rc = false;
	int  errors = 0;
	static unsigned char pattern[PortSamplesDataSize * sizeof(unsigned short)];
	unsigned short ind_tx = 0;
	unsigned short ind_rx = 0;

	printf("Running nominal test\n");
	errors = open_usf();

	if (errors || !s_usf_supported)
		return errors;

	for (ind_tx = 0; ind_tx < ARRAY_SIZE(set_valid_tx_configuration);
		 ++ind_tx ) {
		rc = configure_tx_valid(ind_tx);
		if (!rc)
			++errors;

		for (ind_rx = 0; ind_rx < ARRAY_SIZE(set_valid_rx_configuration);
			  ++ind_rx ) {
			rc = configure_rx_valid(ind_rx);
			if (!rc)
				++errors;
	
			rc = start_TX();
			if (!rc)
				++errors;
	
			rc = start_RX();
			if (!rc)
				++errors;
	
			rc = usf_write(pattern, sizeof(pattern));
			if (!rc)
				++errors;
	
			rc = usf_read();
	
			if (!rc)
				++errors;
	
			rc = stop_TX();
			if (!rc)
				++errors;
	
			rc = stop_RX();
			if (!rc)
				++errors;
	
			rc = usf_close();
			if (!rc)
				++errors;
		} // // valid rx configuration loop
	} // valid tx configuration loop

	// Release USF resources
	if (s_usf_fd >= 0 ) {
		close(s_usf_fd);
		s_usf_fd = -1;
	}

	return errors;
}

static int adversarial_test(void)
{
	bool rc = false;
	int  errors = 0;
	unsigned short ind = 0;

	printf("Running adversarial test\n");
	errors = open_usf();
	if (errors || !s_usf_supported)
		return errors;

	for (ind = 0; ind < ARRAY_SIZE(configure_tx_invalid); ++ind ) {
		if (configure_tx_invalid[ind] != NULL) {
			rc = (*configure_tx_invalid[ind])();
			if (rc) {
				printf("%s: no tx_config failure as supposed; ind=%d\n",
					   __FUNCTION__, ind);
				++errors;
			}
		}
	}

	for (ind = 0; ind < ARRAY_SIZE(configure_rx_invalid); ++ind ) {
		if (configure_rx_invalid[ind] != NULL) {
			rc = (*configure_rx_invalid[ind])();
			if (rc) {
				printf("%s: no rx_config failure as supposed; ind=%d\n",
					   __FUNCTION__, ind);
				++errors;
			}
		}
	}

	(void)usf_close();

	// Release USF resources
	if (s_usf_fd >= 0 ) {
		close(s_usf_fd);
		s_usf_fd = -1;
	}

	return errors;
} // adversarial_test

static int repeat_test_utility(int iterations)
{
	int ind = 0;
	int errors = 0;

	for (ind = 1; ind <= iterations; ++ind) {
		printf("Run:  %d\n", ind);
		errors = nominal_test();
		if (errors || !s_usf_supported)
			break;
		errors = adversarial_test();
		if (errors)
			break;
	}

	return errors;
} // repeat_test_utility

static int stress_test(void)
{
	// Run Nominal & adversarial tests for a long time - 1000 iterations
	// System stress - in a future
	// The current USF driver supports work with only one thread
	int errors = repeat_test_utility(NUM_ITERATIONS_STRESS);

	return errors;
} // stress_test

static int repeat_test(void)
{
	int errors = repeat_test_utility(NUM_ITERATIONS);
	return errors;
} // repeat_test

static int (*test_func[]) (void) = {
	nominal_test,
	adversarial_test,
	stress_test,
	repeat_test
};

static void usage(int ret)
{
	printf("Usage: usf_test [TEST_TYPE]...\n"
	       "Runs the user space tests specified by the TEST_TYPE\n"
	       "parameters.  If no TEST_TYPE is specified, then the nominal\n"
	       " test is run.\n"
	       "\n"
	       "TEST_TYPE can be:\n"
	       "  -n, --nominal         run standard functionality tests\n"
	       "  -a, --adversarial     run tests that try to break the \n"
	       "                          driver\n"
	       "  -s, --stress          run tests that try to maximize the \n"
	       "                          capacity of the driver\n"
	       "  -r, --repeatability   run 5 iterations of both the \n"
	       "                          nominal and adversarial tests\n"
	       "  -h, --help            print this help message and exit\n");

	exit(ret);
} // usage

static unsigned int parse_command(int argc, char *const argv[])
{
	int command;
	unsigned ret = 0;

	struct option longopts[] = {
		{"nominal", no_argument, NULL, 'n'},
		{"adversarial", no_argument, NULL, 'a'},
		{"stress", no_argument, NULL, 's'},
		{"repeatability", no_argument, NULL, 'r'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0},
	};

	while ((command = getopt_long(argc, argv, "v:nasrph", longopts,
				      NULL)) != -1) {
		switch (command) {
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
		case 'h':
			usage(0);
		default:
			usage(-1);
		}
	}

	return ret;
} // parse_command

int main(int argc, char **argv)
{
	int rc = 0;
	int num_tests_failed = 0;
	int i = 0;
	unsigned int test_mask = parse_command(argc, argv);

	/* Run the nominal case if none is specified */
	if (test_mask == 0)
		test_mask = 1 << NOMINAL;

	for (i = 0; i < (int)ARRAY_SIZE(test_func); i++) {
		/* Look for the test that was selected */
		if (!(test_mask & (1U << i)))
			continue;

		/* This test was selected, so run it */
		rc = test_func[i] ();

		if (rc) {
			printf("Test failed! rc: %d\n", rc);
			num_tests_failed++;
		} else {
			printf("Test passed\n");
		}
	}

	if (num_tests_failed > 0)
		printf("USF tests failed: num=%d\n", num_tests_failed);
	else
		printf("USF tests passed\n");

	return -num_tests_failed;
} // main
