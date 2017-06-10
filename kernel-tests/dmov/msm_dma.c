/**********************************************************************
 * MSM DMA driver tester.
 *
 * Copyright (c) 2008 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dma_test.h"

#define BUFFER_SIZE 16384

int buf[2];

char tmp_buffer[2][BUFFER_SIZE];

void set_pattern(char *buf, unsigned seed)
{
	int i;
	unsigned *lbuf = (unsigned *) buf;
	
	assert(((long) buf & 3) == 0);

	for (i = 0; i < BUFFER_SIZE/4; i++) {
		lbuf[i] = seed;
		if (seed & 1)
			seed = (seed >> 1) ^ 0x80000057;
		else
			seed >>= 1;
	}
}

void check_pattern(char *buf, unsigned seed)
{
	int i;
	unsigned *lbuf = (unsigned *) buf;
	
	assert(((long) buf & 3) == 0);

	for (i = 0; i < BUFFER_SIZE/4; i++) {
		if (lbuf[i] != seed) {
			printf("Buffer comparison mismatch\n");
			exit(1);
		}
		if (seed & 1)
			seed = (seed >> 1) ^ 0x80000057;
		else
			seed >>= 1;
	}
}

int alloc_buffer(int fd, int size)
{
	int res;
	struct msm_dma_alloc_req req;

	req.size = size;
	res = ioctl(fd, MSM_DMA_IOALLOC, &req);
	if (res != 0) {
		perror(__FUNCTION__);
		exit(1);
	}
	
	return req.bufnum;
}

void free_all(int fd)
{
	int res;

	res = ioctl(fd, MSM_DMA_IOFREEALL, NULL);
	if (res != 0) {
		perror(__FUNCTION__);
		exit(1);
	}
}

void write_data(int fd, int bufnum, void *data, int size)
{
	int res;
	struct msm_dma_bufxfer req;

	req.data = data;
	req.size = size;
	req.bufnum = bufnum;

	res = ioctl(fd, MSM_DMA_IOWBUF, &req);
	if (res != 0) {
		perror(__FUNCTION__);
		exit(1);
	}
}

void read_data(int fd, int bufnum, void *data, int size)
{
	int res;
	struct msm_dma_bufxfer req;

	req.data = data;
	req.size = size;
	req.bufnum = bufnum;

	res = ioctl(fd, MSM_DMA_IORBUF, &req);
	if (res != 0) {
		perror(__FUNCTION__);
		exit(1);
	}
}

void copy_buffer(int fd, int src, int dest, int size)
{
	struct msm_dma_scopy req;

	req.srcbuf = src;
	req.destbuf = dest;
	req.size = size;
	if (ioctl(fd, MSM_DMA_IOSCOPY, &req) != 0) {
		perror(__FUNCTION__);
		exit(1);
	}
}

int main(int argc, char **argv)
{
	int fd;
	int i;

	fd = open("/dev/msmdma", O_RDWR);
	if (fd < 0) {
		perror("Unable to open /dev/msmdma");
		exit(1);
	}
	
	free_all(fd);
	
	for (i = 0; i < 2; i++) {
		buf[i] = alloc_buffer(fd, BUFFER_SIZE);
		printf("Got user buffer %d for %d\n", buf[i], i);
	}

	set_pattern(tmp_buffer[0], 1);
	write_data(fd, 0, tmp_buffer[0], BUFFER_SIZE);
	
	set_pattern(tmp_buffer[1], 2);
	write_data(fd, 1, tmp_buffer[1], BUFFER_SIZE);
	
	memset(tmp_buffer[0], 0, BUFFER_SIZE);
	memset(tmp_buffer[1], 0, BUFFER_SIZE);
	
	read_data(fd, 0, tmp_buffer[0], BUFFER_SIZE);
	check_pattern(tmp_buffer[0], 1);
	
	read_data(fd, 1, tmp_buffer[0], BUFFER_SIZE);
	check_pattern(tmp_buffer[0], 2);
	
	memset(tmp_buffer[0], 0, BUFFER_SIZE);
	memset(tmp_buffer[1], 0, BUFFER_SIZE);
	
	copy_buffer(fd, 0, 1, BUFFER_SIZE);
	read_data(fd, 0, tmp_buffer[0], BUFFER_SIZE);
	check_pattern(tmp_buffer[0], 1);
	read_data(fd, 1, tmp_buffer[1], BUFFER_SIZE);
	check_pattern(tmp_buffer[1], 1);

	close (fd);
	
	exit(0);
}
