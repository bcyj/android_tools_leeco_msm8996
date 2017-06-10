/*
 * User-space unit test application for the MSM I2C driver.
 *
 * This test uses the EEPROM as the supported slave device.
 *
 * Copyright (c) 2009, 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define pr_fmt(fmt) "#%d " fmt, __LINE__
#define pr_err(fmt, ...) do {\
	if (errno)\
		fprintf(stderr, "Error " pr_fmt(fmt " err#:%d:%s\n")\
		, ##__VA_ARGS__, errno , strerror(errno));\
	else \
		fprintf(stderr, "Error " pr_fmt(fmt), ##__VA_ARGS__);\
	} while (0)
#define pr_verbose(fmt, ...) do {\
		if (verbose)\
			printf(fmt, ##__VA_ARGS__);\
	} while (0)

#define DATA_SIZE	74
#define TEST_OFFSET	59
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(*a))

#define LOG_LINE_MAX_LEN		(512)
#define MSM_EEPROM_ADDRESS		0x52
#define MSM_EEPROM_PAGE_SIZE		64
#define MSM_EEPROM_PAGE_SIZE_MASK	(MSM_EEPROM_PAGE_SIZE - 1)
#define DUMP_BYTES_PER_LINE		(20) /* x4 char per byte = 80 chars */

#define GET_PAGE_END(off) \
	(((off) + MSM_EEPROM_PAGE_SIZE_MASK) & ~MSM_EEPROM_PAGE_SIZE_MASK)

#define IS_PAGE_OFFSET(off) (0 == ((off) & MSM_EEPROM_PAGE_SIZE_MASK))

static inline uint32_t min(uint32_t a, uint32_t b) {
   return (a < b) ? a : b;
}

static inline int get_trans_len(uint32_t offset, uint32_t len) {
   if (!IS_PAGE_OFFSET(offset)) {
      return min(GET_PAGE_END(offset) - offset, len);
   }
   return min(MSM_EEPROM_PAGE_SIZE, len);
}

static int test_scan_bus(int fd);
static int test_eeprom_n_nack(int fd);
static int test_eeprom(int fd);
static int probe_eeprom(int fd);
static int parse_args(int argc, char **argv);
static void dump_test_config(void);

static bool verbose;
static bool ascending;
static int  read_bc;
static int  write_bc;
static int  rd_wr_cnt  = 1;
static int  iterations = 1;
static int  num_blocks = 20;
static int  block_size = DATA_SIZE;
static int  test_offset = TEST_OFFSET;
static uint16_t slave_address = MSM_EEPROM_ADDRESS;
static int (*test_func)(int) = test_eeprom_n_nack;
static const char *device_name = "/dev/i2c-0";

static int do_rdwr(int fd, struct i2c_msg *msgs, int nmsgs)
{
	struct i2c_rdwr_ioctl_data msgset = {
		.msgs = msgs,
		.nmsgs = nmsgs,
	};

	if (msgs == NULL || nmsgs <= 0)
		return -1;

	if (ioctl(fd, I2C_RDWR, &msgset) < 0)
		return -1;

	return 0;
}

/**
 * do_basic_read: Read from device a single buffer
 * @return one on success or negative error code
 */
static int do_basic_read(int fd, uint16_t address, uint8_t *buf, uint16_t count)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr = address,
			.flags = I2C_M_RD,
			.buf = (void *)buf,
			.len = count,
		},
	};
	struct i2c_rdwr_ioctl_data msgset = {
		.msgs = msgs,
		.nmsgs = ARRAY_SIZE(msgs),
	};

	ret = ioctl(fd, I2C_RDWR, &msgset);
	pr_verbose("Reading:%d bytes at addr:0x%x ret:%d\n",
			count, slave_address, ret);

	return ret;
}

static double log_get_line_timestamp(const char *line)
{
	double timestamp = 0.0;
	/*
	 * offset 4 (line + 4) is here:
	 *     |
	 *     V
	 * <3>[ 6475.962278] msm_ipc_load_default_node: Failed to load modem
	 */
	sscanf(line + 4, "%lf", &timestamp);

	return timestamp;
}

/*
 * log_are_strs_in_line: search for strings in the line
 *
 * @find_str_arr array of arrays of string to match against a log line.
 *       the array is null terminated. The internal arrays are null terminated.
 * @return positive int if all the string in the array are found in line
 */
static int log_are_strs_in_line(const char **find_str_arr[], const char *line)
{
	int i2c_drvr = 0;

	if (!find_str_arr)
		return 1;

	for (; find_str_arr[i2c_drvr]; ++i2c_drvr) {
		const char *found_str = line;
		int  i = 0;
		bool all_found = true;

		for (; find_str_arr[i2c_drvr][i]; ++i) {
			found_str = strstr(found_str,
						find_str_arr[i2c_drvr][i]);
			if (!found_str) {
				all_found = false;
				break;
			}
		}
		if (all_found)
			return 1;
	}

	return 0;
}

/*
 * log_cnt_matches_after_timestamp: count log lines containing search strings
 *
 * @find_str_arr array of arrays of string to match against a log line.
 *       the array is null terminated. The internal arrays are null terminated.
 * @search_after_timestamp start searching after this timestamp
 * @log_last_timestamp out param, last timestamp found in log
 * @return number of matching lines or negative error value
 */
static int log_cnt_matches_after_timestamp(
		double search_after_timestamp, double *log_last_timestamp,
		const char **find_str_arr[])
{
	FILE  *log;
	bool   is_popen = true;
	char   line[LOG_LINE_MAX_LEN];
	int    ret       = 0;
	int    match_cnt = 0;
	double cur_timestamp = 0.0;

	/* Works on APT test env */
	log = popen("dmesg", "re");
	if (!log) {
		/* Works on KDev test env */
		log =  popen("/bin/dmesg", "re");
		if (!log) {
			/* Works on target with Android */
			system("dmesg > klog-i2c-test");
			log =  fopen("klog-i2c-test", "r");
			is_popen = false;

			if (!log) {
				pr_err("failed to open kernel log for reading\n");
				return errno;
			}
		}
	}

	while (!feof(log)) {
		if (!fgets(line, sizeof(line), log))
			break;

		cur_timestamp = log_get_line_timestamp(line);
		if (cur_timestamp <= search_after_timestamp)
			continue;

		if (log_are_strs_in_line(find_str_arr, line))
			++match_cnt;
	}

	*log_last_timestamp = cur_timestamp;
	if (is_popen)
		pclose(log);
	else
		fclose(log);
	log = NULL;

	return (ret < 0) ? ret : match_cnt;
}

/*
 * @timestamp out param, last timestamp in kernel log
 * @return zero on success or negative error code
 */
static int log_get_last_timestamp(double *timestamp)
{
	int ret = log_cnt_matches_after_timestamp(0, timestamp, NULL);
	if (!ret)
		return -ENOENT;
	return (ret > 0) ? 0 : ret;
}

/*
 * test_eeprom_n_nack: Run EEPROM test, if it fails run NACK test
 *
 * 1. Finds the last line in the kernel log.
 * 2. Executes EEPROM test, if it passes, exits with success
 * 3. If it fails, search for a
 * Searches the kernel-log for nack signature after the given timesatmp.
 * Log lines are searched for nack signatures of both i2c_qup (v1) and
 * i2c-msm-v2 (v2) drivers.
 *
 * @return zero on success (nack observed) or negative error value
 */
static int test_eeprom_n_nack(int fd)
{
	int ret;
	double timestamp;
	double last_log_timestamp;
	const char *find_str_arr_driver_v1[] = {
			"I2C slave addr:", "not connected", NULL};
	const char *find_str_arr_driver_v2[] = {
			"slave:", "is not responding (I2C-NACK) ensure the slave is powered and out of reset", NULL};
	const char **find_str_arr[] = {
			find_str_arr_driver_v1, find_str_arr_driver_v2, NULL};

	pr_verbose("EEPROM/NACK test\n");

	if (verbose)
		dump_test_config();

	ret = log_get_last_timestamp(&timestamp);
	if (ret) {
		pr_err("Error on reading last timestamp form kernel log\n");
		return ret;
	}

	ret = test_eeprom(fd);
	if (!ret) {
		pr_verbose("EEPROM-Test passed\n");
		return 0;
	}

	ret = log_cnt_matches_after_timestamp(timestamp, &last_log_timestamp,
								find_str_arr);
	if (ret > 0) {
		pr_verbose("NACK-Test passed\n");
		return 0;
	}

	pr_verbose("EEPROM/NACK-Test failed\n");
	return (ret < 0) ? ret : -EIO;
}

static int test_scan_bus(int fd)
{
	int i;
	uint8_t data[1];

	printf("Scanning the bus...\n");

	for (i = 8; i < 120; ++i) {
		slave_address = i;
		if (!do_basic_read(fd, slave_address, data, sizeof(data)))
			printf("Found device at address 0x%x (0x%x/0x%x)\n",
				i, i*2, (i*2)+1);
	}

	return 0;
}

static int read_eeprom(int fd, uint16_t offset, uint8_t *buf, uint16_t count)
{
	uint8_t offset_data[] =  {offset >> 8, offset & 0xFF};

	struct i2c_msg msgs[] = {
		[0] = {
			.addr = slave_address,
			.flags = 0,
			.buf = (void *)offset_data,
			.len = ARRAY_SIZE(offset_data),
		},
		[1] = {
			.addr = slave_address,
			.flags = I2C_M_RD,
			.buf = (void *)buf,
			.len = count,
		},
	};

	pr_verbose("Reading %d bytes from slave-addr:0x%x offset:0x%x\n"
						, count, slave_address, offset);

	if (do_rdwr(fd, msgs, ARRAY_SIZE(msgs))) {
		return -1;
	}

	return 0;
}

static
int write_eeprom_page(int fd, uint16_t offset, const uint8_t *buf, int len)
{
	uint8_t *data = (uint8_t *)malloc((2 + len) * sizeof(*data));

	struct i2c_msg msgs[] = {
		[0] = {
			.addr = slave_address,
			.flags = 0,
			.buf = (void *)data,
			.len = (2 + len) * sizeof(*data),
		},
	};

	if (!data) {
		len = -1;
		goto err_malloc;
	}

	pr_verbose("Writing %d bytes to slave-addr:0x%x offset:0x%x\n"
						, len, slave_address, offset);

	data[0] = offset >> 8;
	data[1] = offset & 0xFF;
	memcpy(data + 2, buf, len);

	if (do_rdwr(fd, msgs, ARRAY_SIZE(msgs))) {
		len = -1;
		goto err_do_rdwr;
	}

	usleep(20000);

err_do_rdwr:
	free(data);
err_malloc:
	return len;
}

static int write_eeprom(int fd, uint16_t offset, uint8_t *buf, uint16_t count)
{
	uint16_t len = count;
	int rc = 0;

	while (len > 0) {
		uint16_t this_trans_len = get_trans_len(offset, len);

		rc = write_eeprom_page(fd, offset, buf, this_trans_len);
		if (rc < 0)
			return rc;

		buf += this_trans_len;
		offset += this_trans_len;
		len -= this_trans_len;
	}

	return 0;
}

static void dump_buffers(uint8_t *write_buf, uint8_t *read_buf, int len)
{
	static const int space_per_byte = 5; /* strlen("0x00 ") */
	static const int screen_size    = 80;
	const int        bytes_per_row  = screen_size / space_per_byte;
	int cur, wr_cnt = 0, rd_cnt = 0;

	printf("Dump of read and write buffers:\n");

	while (wr_cnt < len) {
		for (cur = 0; (cur < bytes_per_row) && (wr_cnt < len);
							++cur, ++wr_cnt) {
			if (!cur)
				printf("Wr: ");
			printf("0x%2x ", write_buf[cur]);
		}
		printf("\n");
		for (cur = 0; (cur < bytes_per_row) && (rd_cnt < len);
							++cur, ++rd_cnt) {
			if (!cur)
				printf("Rd: ");
			printf("0x%2x ", read_buf[cur]);
		}
		printf("\n\n");
	}
}

static int test_eeprom(int fd)
{
	int i;
	int j;
	int k;
	int rc = 0;
	int offset = test_offset;

	uint8_t *read_buf1 = malloc(block_size);
	uint8_t *read_buf2 = malloc(block_size);
	uint8_t *write_buf = malloc(block_size);

	if (!read_buf1 || !read_buf2 || !write_buf) {
		rc = -1;
		pr_err("failed to allocate space for data buffers\n");
		goto err_alloc;
	}

	if (ascending)
		for (k = 0; k < block_size; k++)
			write_buf[k] = (k & 0xFF);
	else
		srand(time(NULL));

	for (j = 0; j < iterations; j++) {

		offset = test_offset;

		for (i = 0; i < num_blocks; i++, offset += block_size) {

			/* Initialize write buffer with random data */
			if (!ascending)
				for (k = 0; k < block_size; k++)
					write_buf[k] = (uint8_t)(rand() & 0xFF);

			rc = read_eeprom(fd, offset, read_buf1, block_size);
			if (rc < 0) {
				if (test_func == test_eeprom)
					pr_err("Could not read eeprom before "
						"writing\n");
				goto err_exit;
			}

			rc = write_eeprom(fd, offset, write_buf, block_size);
			if (rc < 0) {
				if (test_func == test_eeprom)
					pr_err(
					     "Could not write random buffer\n");
				goto err_exit;
			}

			rc = read_eeprom(fd, offset, read_buf2, block_size);
			if (rc < 0) {
				if (test_func == test_eeprom)
					pr_err("Could not read eeprom after "
						"writing\n");
				goto restore_eeprom;
			}

			if (memcmp(write_buf, read_buf2, block_size)) {
				if (test_func == test_eeprom)
					pr_err("Buffers differ after writing "
						"and reading\n");
				if (verbose)
					dump_buffers(write_buf, read_buf2,
								block_size);
				rc = -1;
				goto restore_eeprom;
			}

			/* Restore */
			rc = write_eeprom(fd, offset, read_buf1, block_size);
			if (rc < 0) {
				pr_err(
				"Could not restore eeprom to original state\n");
				goto err_exit;
			}
		}
	}
	goto done;

restore_eeprom:
	if (write_eeprom(fd, offset, read_buf1, block_size) < 0)
		pr_err("Could not restore eeprom to original state\n");

done:
err_exit:
	free(read_buf1);
	free(read_buf2);
	free(write_buf);
err_alloc:
	return rc;
}

/**
 * test_wr_rd: iterate over a single write followed by a read
 *
 * @return zero on success
 *
 * Default number of iteration is 1. Thus the default case is a single write
 * followed by a single read.
 * Lnegth of write is write_bc. if write_bc is zero no write is issued.
 * Lnegth of read is read_bc. if read_bc is zero no read is issued.
 * write buffer content is defaulted to random values or to ascending values
 * when the asending flag is set.
 */
static int test_wr_rd(int fd)
{
	int i, j, k;
	int step;
	int n_msgs ;
	int rc = 0;
	/* read_buf includes multiple read buffers */
	uint8_t        *read_buf  = NULL;
	/* single write_buf is used for all writes */
	uint8_t        *write_buf = NULL;
	struct i2c_msg *msgs      = NULL;
	const char      *action = "";

	printf("*ONLY* use with an i2c-bus *ANALYSER/EMULATOR* device\n");

	if (verbose)
		dump_test_config();

	if (read_bc) {
		if (write_bc) {
			action = "write and read";
			step   = 2;
			n_msgs = 2 * rd_wr_cnt;
		} else {
			action = "read";
			step   = 1;
			n_msgs = rd_wr_cnt;
		}
	} else {
		if (write_bc) {
			action = "write";
			step   = 1;
			n_msgs = rd_wr_cnt;
		} else {
			return 0;
		}
	}

	msgs = malloc(sizeof(*msgs) * n_msgs);
	if (!msgs) {
		pr_err("failed to allocated array of %d i2c_msg structs",
									n_msgs);
		return -ENOMEM;
	}

	/* allocate read buffer */
	if (read_bc) {
		read_buf = malloc(read_bc * rd_wr_cnt);
		if (!read_buf) {
			rc = -ENOMEM;
			pr_err("failed to allocate %d bytes for read buffer\n",
								read_bc);
			goto err_exit;
		}
	}
	/* allocate write buffer */
	if (write_bc) {
		write_buf = malloc(write_bc);
		if (!write_buf) {
			rc = -ENOMEM;
			pr_err("failed to allocate %d bytes for write buffer\n",
								write_bc);
			goto err_exit;
		}

		/* populate write buffer */
		if (ascending) {
			for (i = 0; i < write_bc; i++)
				write_buf[i] = (i & 0xFF);
		} else {
			srand(time(NULL));
			for (i = 0; i < write_bc; i++)
				write_buf[i] = (uint8_t)(rand() & 0xFF);
		}
	}

	pr_verbose("read_bc:%d write_bc:%d step:%d\n", read_bc, write_bc, step);

	/* populate i2c_msgs with write buffers */
	if (write_bc)
		for (i = 0 ; i < n_msgs; i += step)
			msgs[i] = (struct i2c_msg) {
				.addr  = slave_address,
				.len   = write_bc,
				.buf   = write_buf,
			};

	/* populate i2c_msgs with read buffers */
	if (read_bc) {
		uint8_t *cur_read_buf = read_buf;

		for (i = (write_bc ? 1 : 0); i < n_msgs; i += step,
							cur_read_buf += read_bc)
			msgs[i] = (struct i2c_msg) {
				.addr  = slave_address,
				.flags = I2C_M_RD,
				.len   = read_bc,
				.buf   = cur_read_buf,
			};
	}

	for (i = 0 ; i < n_msgs; ++i)
		pr_verbose("msg[%d] addr:0x%x len:%d is_inp:0x%x\n",
			i, msgs[i].addr, msgs[i].len,
			msgs[i].flags & I2C_M_RD);

	for (j = 0; j < iterations; j++) {
		rc = do_rdwr(fd, msgs, n_msgs);
		if (rc < 0) {
			pr_err("Could not %s buffers\n", action);
			goto err_exit;
		}

		if (read_bc && verbose) {
			uint8_t *cur_read_buf = read_buf;

			for (k = (write_bc ? 1 : 0); k < n_msgs; k += step,
						cur_read_buf += read_bc) {
				printf("Read buf:");
				for (i = 0; i < read_bc; ++i) {
					if (!(i % DUMP_BYTES_PER_LINE))
						printf("\n");
					printf("0x%x ", cur_read_buf[i]);
				}
				printf("\n");
			}
		}
	}

err_exit:
	free(read_buf);
	free(write_buf);
	free(msgs);
	return rc;
}

static int probe_eeprom(int fd)
{
	uint8_t data[1];

	/* Do a simple read to verify that the device is present */
	int rc = read_eeprom(fd, 0, &data[0], sizeof(data));

	if (rc < 0) {
		pr_err("Could not read from EEPROM at address 0x%x\n",
				slave_address);
		return -ENODEV;
	}

	return 0;
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-Dsbnoiealv]\n", prog);
	puts(
	"  -D --device      device to use (default /dev/i2c-0)\n"
	"  -s --slave_addr  address of the i2c slave device\n"
	"  -p --probe       checks if the device is present\n"
	"  -b --block_size  size of data blocks for read/writes (bytes)\n"
	"  -n --num_blocks  the number of data blocks to read/write\n"
	"  -o --offset      address offset from the start of the device\n"
	"  -i --iter        number of times to repeat the test\n"
	"  -e --eeprom      write and read back from the EEPROM. Original\n"
	"                   data is restored\n"
	"  -a --asc         fill write buffer with ascending values (0..0xff)\n"
	"                   rather then the default random values.\n"
	"  -l --scan        scans the i2c bus for slaves (lists devices)\n"
	"  -v --verbose     verbose\n"
	"\n"
	"*ONLY* use with an i2c-bus *ANALYSER/EMULATOR* device:\n"
	"  -w --write       number of bytes to write for write-then-read\n"
	"                   test (not eeprom test). default is no write.\n"
	"  -r --read        number of bytes to read for write-then-read\n"
	"                   test (not eeprom test). default is no read.\n"
	"  -c --rd_wr_cnt   when --write is set this option sets the\n"
	"                   number of writes. When --read is set, this\n"
	"                   option sets the number of reads. When both\n"
	"                   are set, this option creates a mix array\n"
	"                   such that wr-0,rd-0,wr-1,rd-1,...wr-n,rd-n.\n"
	"                   When none is set, this option has no effect.\n");
	exit(1);
}

static int parse_args(int argc, char **argv)
{
	struct option lopts[] = {
		{ "device",      required_argument, NULL, 'D'},
		{ "slave_addr",  required_argument, NULL, 's'},
		{ "block_size",  required_argument, NULL, 'b'},
		{ "num_blocks",  required_argument, NULL, 'n'},
		{ "offset",      required_argument, NULL, 'o'},
		{ "iterations",  required_argument, NULL, 'i'},
		{ "probe",       no_argument,       NULL, 'p'},
		{ "scan",        no_argument,       NULL, 'l'},
		{ "write",       required_argument, NULL, 'w'},
		{ "eeprom",      required_argument, NULL, 'e'},
		{ "read",        required_argument, NULL, 'r'},
		{ "rd_wr_cnt",   required_argument, NULL, 'c'},
		{ "asc",         no_argument,       NULL, 'a'},
		{ "verbose",     no_argument,       NULL, 'v'},
		{ "help",        no_argument,       NULL, 'h'},
		{ NULL,          0,                 NULL,  0},
	};
	int command;
	const char *optstr = "D:s:b:n:o:i:plw:r:c:avhe";

	while ((command = getopt_long(argc, argv, optstr, lopts, NULL)) != -1) {
		switch (command) {
		case 'D':
			device_name = optarg;
			break;
		case 's':
			slave_address = (uint16_t)(strtol(optarg, NULL, 0));
			break;
		case 'b':
			block_size = atoi(optarg);
			break;
		case 'n':
			num_blocks = atoi(optarg);
			break;
		case 'o':
			test_offset = atoi(optarg);
			break;
		case 'i':
			iterations = atoi(optarg);
			break;
		case 'e':
			test_func = test_eeprom;
			break;
		case 'p':
			test_func = probe_eeprom;
			break;
		case 'l':
			test_func = test_scan_bus;
			break;
		case 'w':
			write_bc = atoi(optarg);
			if (write_bc > 0) {
				test_func = test_wr_rd;
			} else {
				pr_err("%s is invalid num of bytes to write",
				       optarg);
				write_bc = 0;
			}
			break;
		case 'r':
			read_bc = atoi(optarg);
			if (read_bc > 0) {
				test_func = test_wr_rd;
			} else {
				pr_err("%s is invalid num of bytes to read",
				       optarg);
				read_bc = 0;
			}
			break;
		case 'c':
			rd_wr_cnt = atoi(optarg);
			if (rd_wr_cnt < 0) {
				pr_err("%s is invalid num of bytes to read",
				       optarg);
				rd_wr_cnt = 1;
			}
			break;
		case 'a':
			ascending = true;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			print_usage(argv[0]);
			break;
		}
	}

	return 0;
}

static void dump_test_config(void)
{
	const char *type = "NULL";

	/* dump verbose info */
	if (test_func == test_eeprom)
		type = "EEPROM-Test";
	else if (test_func == probe_eeprom)
		type = "Probe-EEPROM";
	else if (test_func == test_scan_bus)
		type = "Bus-Scan";
	else if (test_func == test_wr_rd)
		type = "Write and Read Test";
	else if (test_func == test_eeprom_n_nack)
		type = "EEPROM-Test with fallback to NACK-Test";

	pr_verbose("I2C unit test configuration summery:\n");
	pr_verbose("test type       : %s\n", type);
	pr_verbose("device          : %s\n", device_name);
	pr_verbose("address         : 0x%x\n", slave_address);
	if ((test_func == test_eeprom) || (test_func == probe_eeprom)) {
		pr_verbose("offset          : %d\n", test_offset);
		pr_verbose("block size      : %d\n", block_size);
		pr_verbose("number of blocks: %d\n", num_blocks);
	} else if (test_func == test_wr_rd) {
		pr_verbose("write size      : %d\n", write_bc);
		pr_verbose("read size       : %d\n", read_bc);
		pr_verbose("rd_wr_cnt       : %d\n", rd_wr_cnt);
	};
	pr_verbose("iterations      : %d\n", iterations);
	pr_verbose("values          : %s\n", ascending ? "ascending" :
								"random");
}

int main(int argc, char **argv)
{
	int fd;
	int rc = 0;

	if (parse_args(argc, argv))
		return 1;

	fd = open(device_name, O_RDWR);
	if (-1 == fd) {
		rc = -1;
		pr_err("Could not open device %s\n", device_name);
		goto err_open;
	}

	rc = (*test_func)(fd);

	close(fd);

err_open:
	return rc;
}
