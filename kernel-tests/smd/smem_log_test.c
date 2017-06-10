/******************************************************************************
  @file	 smem_log_test.c
  @brief Test program for SMEM log

  DESCRIPTION
  This tests the user SMEM log functionality

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  -----------------------------------------------------------------------------
  Copyright (c) 2008, 2011 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEBUG
#undef DEBUG

struct smem_log_item {
	unsigned int identifier;
	unsigned int timetick;
	unsigned int data1;
	unsigned int data2;
	unsigned int data3;
};

#define SMEM_LOG_NUM_ENTRIES 2000

#define SMEM_LOG_BASE 0x30

#define SMIOC_SETMODE _IOW(SMEM_LOG_BASE, 1, int)
#define SMIOC_SETLOG _IOW(SMEM_LOG_BASE, 2, int)

#define SMIOC_TEXT 0x00000001
#define SMIOC_BINARY 0x00000002
#define SMIOC_LOG 0x00000003
#define SMIOC_STATIC_LOG 0x00000004

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

#ifdef DEBUG
#define D(x...) printf(x)
#else
#define D(x...) do {} while (0)
#endif

static int enable_test[] = {
	1, /* t0_bin_write_read_check */
	1, /* t1_bin_write_read_check_multiple */
	1, /* t2_write_read_check */
	1, /* t3_write_read_check_multiple */
	1, /* t4_write_all_log */
	1, /* t5_all_tests_on_static */
};

static int log_dest = SMIOC_LOG;

static int read_entry(int fd, char *buf, int len)
{
	int ret;

	D("%s read %i\n", __func__, len);

	ret = read(fd, buf, len);
	if (ret < 0) {
		printf("ERROR: %s: read returned %i\n", __func__, ret);
		return -1;
	}
	return 0;
}

static int write_entry(int fd, char *buf, int len)
{
	int ret;

	D("%s write %i\n", __func__, len);

	ret = write(fd, buf, len);
	if (ret != len) {
		printf("ERROR: %s: write returned %i\n", __func__, ret);
		return -1;
	}
	return 0;
}

static int bin_read_entry(int fd, struct smem_log_item *item, int num)
{
	int ret;

	ret = read(fd, item, sizeof(*item)*num);
	if ((unsigned)ret != sizeof(*item)*num) {
		printf("ERROR: %s: read returned %i\n", __func__, ret);
		return -1;
	}
	return 0;
}

static int bin_write_entry(int fd, struct smem_log_item *item, int num)
{
	int ret;

	ret = write(fd, item, sizeof(*item)*num);
	if ((unsigned)ret != sizeof(*item)*num) {
		printf("ERROR: %s: write returned %i\n", __func__, ret);
		return -1;
	}
	return 0;
}

static int is_equal_bin(struct smem_log_item *e0, struct smem_log_item *e1)
{
	return (e0->identifier == e1->identifier &&
		e0->data1 == e1->data1 &&
		e0->data2 == e1->data2 &&
		e0->data3 == e1->data3);
}

static int str_to_smem_log_item(char *elm, struct smem_log_item *item,
				char **end)
{
	const char delimiters[] = " ,;";
	unsigned int val[5] = {0};
	unsigned int vals = 0;
	char *token;
	char *tail;
	char *running;

	running = elm;

	token = strsep(&running, delimiters);
	while (token && vals < ARRAY_SIZE(val)) {
		if (*token != '\0') {
			D("%s: ", __func__);
			D("token = %s\n", token);
			val[vals++] = strtoul(token, &tail, 0);
		}
		token = strsep(&running, delimiters);
	}

	item->identifier = val[0];
	item->data1 = val[2];
	item->data2 = val[3];
	item->data3 = val[4];

	if (end != 0)
		*end = running;

	return 0;
}

static int is_equal(char *i0, char *i1)
{

	int ret;
	struct smem_log_item e0;
	struct smem_log_item e1;

	ret = str_to_smem_log_item(i0, &e0, 0);
	if (ret < 0)
		return -1;

	ret = str_to_smem_log_item(i1, &e1, 0);
	if (ret < 0)
		return -1;

	return is_equal_bin(&e0, &e1);
}

static int is_equal_multiple(char *i0, char *i1)
{

	int ret;
	struct smem_log_item e0[2];
	struct smem_log_item e1[2];
	char *end;

	/* records come out in reverse */
	ret = str_to_smem_log_item(i0, &e0[1], &end); i0 = end;
	if (ret < 0)
		return -1;

	ret = str_to_smem_log_item(i0, &e0[0], 0);
	if (ret < 0)
		return -1;


	/* records come out in reverse */
	ret = str_to_smem_log_item(i1, &e1[1], &end); i1 = end;
	if (ret < 0)
		return -1;

	ret = str_to_smem_log_item(i1, &e1[0], 0);
	if (ret < 0)
		return -1;

	return is_equal_bin(&e0[0], &e1[0]) &&
		is_equal_bin(&e0[1], &e1[1]);
}

static int t0_bin_write_read_check(void)
{
	int ret = 0;
	int fd;

	struct smem_log_item read_item;
	struct smem_log_item write_item = {
		.identifier = 0x01234567,
		.timetick = 0,
		.data1 = 1,
		.data2 = 2,
		.data3 = 3,
	};

	if (!enable_test[0])
		return 0;

	fd = open("/dev/smem_log", O_RDWR);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	ret = ioctl(fd, SMIOC_SETMODE, SMIOC_BINARY);
	if (ret == -1) {
        perror("ioctl(SMIOC_BINARY)");
		printf("ERROR %s:%i ioctl(SMIOC_BINARY): %s\n",
		       __func__,
		       __LINE__,
		       strerror(errno));
		goto free_resources;
	}

	ret = ioctl(fd, SMIOC_SETLOG, log_dest);
	if (ret == -1) {
        perror("ioctl(SMIOC_SETLOG)");
		printf("ERROR %s:%i ioctl(SMIOC_SETLOG, %#x): %s\n",
		       __func__,
		       __LINE__,
		       log_dest,
		       strerror(errno));
		goto free_resources;
	}

	ret = bin_write_entry(fd, &write_item, 1);
	if (ret == -1)
		goto free_resources;

	ret = bin_read_entry(fd, &read_item, 1);
	if (ret == -1)
		goto free_resources;

	ret = is_equal_bin(&read_item, &write_item);
	if (ret == -1)
		goto free_resources;

 free_resources:
	close(fd);

	return ret;
}

static int t1_bin_write_read_check_multiple(void)
{
	int ret;
	int fd;

	int num = 10;
	int i = 0;
	int data = 0;

	struct smem_log_item *read_item;
	struct smem_log_item *write_item;

	if (!enable_test[1])
		return 0;

	fd = open("/dev/smem_log", O_RDWR);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	read_item = malloc(sizeof(struct smem_log_item) * num);
	if (read_item == NULL) {
		ret = -1;
		goto free_resources;
	}

	write_item = malloc(sizeof(struct smem_log_item) * num);
	if (write_item == NULL) {
		ret = -1;
		goto free_read;
	}

	ret = ioctl(fd, SMIOC_SETMODE, SMIOC_BINARY);
	if (ret == -1) {
		printf("ERROR %s:%i ioctl(SMIOC_BINARY): %s\n",
		       __func__,
		       __LINE__,
		       strerror(errno));
		goto free_write;
	}

	read_item[0].identifier = write_item[0].identifier = 0x76543210;
	for (i = 0; i < num; ++i) {
		write_item[i].data1 = data++;
		write_item[i].data2 = data++;
		write_item[i].data3 = data++;
	}

	ret = bin_write_entry(fd, write_item, num);
	if (ret == -1)
		goto free_write;

	ret = bin_read_entry(fd, read_item, num);
	if (ret == -1)
		goto free_write;

	for (i = 0; i < num; ++i) {
		ret = is_equal_bin(&read_item[i], &write_item[i]);
		if (ret == -1)
			goto free_write;
	}

	close(fd);

	return ret;


 free_write:
	free(write_item);
 free_read:
	free(read_item);
 free_resources:
	close(fd);

	return ret;
}

static int t2_write_read_check(void)
{
	int ret = 0;
	int fd;

	char write_item[] =
		"0x11223344, "
		"0x00000000, "
		"0x10000000, "
		"0x20000000, "
		"0x30000000 ";
	char read_item[sizeof(write_item)];

	if (!enable_test[2])
		return 0;

	fd = open("/dev/smem_log", O_RDWR);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	ret = ioctl(fd, SMIOC_SETMODE, SMIOC_TEXT);
	if (ret == -1) {
		printf("ERROR %s:%i ioctl(SMIOC_BINARY): %s\n",
		       __func__,
		       __LINE__,
		       strerror(errno));
		goto free_resources;
	}

	ret = write_entry(fd, write_item, sizeof(write_item));
	if (ret == -1)
		goto free_resources;

	ret = read_entry(fd, read_item, sizeof(read_item));
	if (ret == -1)
		goto free_resources;

	ret = is_equal(read_item, write_item);
	if (ret == -1) {
		read_item[sizeof(read_item)-1] = '\0';
		write_item[sizeof(write_item)-1] = '\0';
		printf("ERROR:%s:%i\n", __func__, __LINE__-4);
		printf("read_item = %s\n", read_item);
		printf("write_item = %s\n", write_item);
		goto free_resources;
	}

 free_resources:
	close(fd);

	return ret;
}

static int t3_write_read_check_multiple(void)
{
	int ret = 0;
	int fd;

	char write_item[] =
		"0x44332211, "
		"0x00000000, "
		"0xA0000000, "
		"0xB0000000, "
		"0xC0000000, "
		"0x00000000, "
		"0x00000000, "
		"0xD0000000, "
		"0xE0000000, "
		"0xF0000000 ";
	char read_item[sizeof(write_item)];

	if (!enable_test[3])
		return 0;

	fd = open("/dev/smem_log", O_RDWR);
	if (fd < 0) {
		perror("open");
		return -1;
	}

	ret = ioctl(fd, SMIOC_SETMODE, SMIOC_TEXT);
	if (ret == -1) {
		printf("ERROR %s:%i ioctl(SMIOC_BINARY): %s\n",
		       __func__,
		       __LINE__,
		       strerror(errno));
		goto free_resources;
	}

	ret = write_entry(fd, write_item, sizeof(write_item));
	if (ret == -1)
		goto free_resources;

	ret = read_entry(fd, read_item, sizeof(read_item));
	if (ret == -1)
		goto free_resources;

	D("read_item = %.*s\n", sizeof(read_item), read_item);
	D("write_item = %.*s\n", sizeof(write_item), write_item);

	ret = is_equal_multiple(read_item, write_item);

	if (ret == -1) {
		printf("ERROR:%s:%i\n", __func__, __LINE__-4);
		goto free_resources;
	}

 free_resources:
	close(fd);

	return ret;
}

static int t4_write_all_log(void)
{
	int i = 0;
	int ret;
	int num = SMEM_LOG_NUM_ENTRIES * 2.5;

	if (!enable_test[4])
		return 0;

	for (i = 0; i < num; ++i) {
		ret = t0_bin_write_read_check();
		if (ret < 0) {
			printf("FAILED t0_bin_write_read_check "
				"on i = %i of %i\n", i, num);
			return -1;
		}
	}

	for (i = 0; i < num; ++i) {
		ret = t1_bin_write_read_check_multiple();
		if (ret < 0) {
			printf("FAILED t1_bin_write_read_check_multiple "
				"on i = %i of %i\n", i, num);
			return -1;
		}
	}

	for (i = 0; i < num; ++i) {
		ret = t2_write_read_check();
		if (ret < 0) {
			printf("FAILED t2_write_read_check "
				"on i = %i of %i\n", i, num);
			return -1;
		}
	}

	for (i = 0; i < num; ++i) {
		ret = t3_write_read_check_multiple();
		if (ret < 0) {
			printf("FAILED t3_write_read_check_multiple "
				"on i = %i of %i\n", i, num);
			return -1;
		}
	}

	return 0;
}

/* Tests if log is disabled.
 *
 * @returns 0  - log not disabled
 *          1  - log is disabled
 *          -1 - error
 */
static int log_disabled(int log)
{
	int ret = 0;
	int fd;

	fd = open("/dev/smem_log", O_RDWR);
	if (fd < 0)
		return -1;

	ret = ioctl(fd, SMIOC_SETLOG, log);
	close(fd);

	if (ret < 0) {
		if (errno == ENODEV)
			ret = 1;
		else
			ret = -1;
	}
	return ret;
}


static int t5_all_tests_on_static(void)
{
	if (!enable_test[5])
		return 0;

	log_dest = SMIOC_STATIC_LOG;

	if (log_disabled(log_dest)) {
		printf("INFO %s:%i Static log disabled, skipping tests.\n",
			__func__,
			__LINE__);

		return 0;
	}

	return t4_write_all_log();
}

int main(int argc, char **argv)
{
	int ret;

	ret = t0_bin_write_read_check();
	if (ret < 0) {
		printf("%s:%i FAILED ret = %i\n", __FILE__, __LINE__-2, ret);
		goto test_fail;
	}

	ret = t1_bin_write_read_check_multiple();
	if (ret < 0) {
		printf("%s:%i FAILED ret = %i\n", __FILE__, __LINE__-2, ret);
		goto test_fail;
	}

	ret = t2_write_read_check();
	if (ret < 0) {
		printf("%s:%i FAILED ret = %i\n", __FILE__, __LINE__-2, ret);
		goto test_fail;
	}

	ret = t3_write_read_check_multiple();
	if (ret < 0) {
		printf("%s:%i FAILED ret = %i\n", __FILE__, __LINE__-2, ret);
		goto test_fail;
	}

	ret = t4_write_all_log();
	if (ret < 0) {
		printf("%s:%i FAILED ret = %i\n", __FILE__, __LINE__-2, ret);
		goto test_fail;
	}

	ret = t5_all_tests_on_static();
	if (ret < 0) {
		printf("%s:%i FAILED ret = %i\n", __FILE__, __LINE__-2, ret);
		goto test_fail;
	}

	printf("%s: PASSED\n", __FILE__);

 test_fail:
	exit(ret);
}
