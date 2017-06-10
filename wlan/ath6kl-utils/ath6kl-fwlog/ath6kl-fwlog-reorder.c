/*
 * ath6kl fwlog reorderer
 * Copyright (c) 2012, Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <stdlib.h>
#include <stdio.h>

#define HDRLEN 8
#define ATH6KL_FWLOG_PAYLOAD_SIZE 1500
#define RECLEN (HDRLEN + ATH6KL_FWLOG_PAYLOAD_SIZE)

static FILE *log_in;
static FILE *log_out;

static unsigned int get_le32(const unsigned char *pos)
{
	return pos[0] | (pos[1] << 8) | (pos[2] << 16) | (pos[3] << 24);
}

int main(int argc, char *argv[])
{
	unsigned char buf[RECLEN];
	size_t res;
	unsigned int timestamp, min_timestamp = -1;
	int pos = 0, min_pos = 0;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <log in> <log out>\n", argv[0]);
		return -1;
	}

	log_in = fopen(argv[1], "r");
	if (log_in == NULL) {
		perror("Failed to open input file");
		return -1;
	}

	log_out = fopen(argv[2], "w");
	if (log_out == NULL) {
		perror("Failed to create output file");
		fclose(log_in);
		return -1;
	}

	pos = 0;
	while ((res = fread(buf, RECLEN, 1, log_in)) == 1) {
		timestamp = get_le32(buf);
		if (timestamp < min_timestamp) {
			min_timestamp = timestamp;
			min_pos = pos;
		}
		pos++;
	}
	printf("First record at position %d\n", min_pos);

	fseek(log_in, min_pos * RECLEN, SEEK_SET);
	while ((res = fread(buf, RECLEN, 1, log_in)) == 1) {
		printf("Read record timestamp=%u length=%u\n",
		       get_le32(buf), get_le32(&buf[4]));
		if (fwrite(buf, RECLEN, res, log_out) != res)
			perror("fwrite");
	}

	fseek(log_in, 0, SEEK_SET);
	pos = min_pos;
	while (pos > 0 && (res = fread(buf, RECLEN, 1, log_in)) == 1) {
		pos--;
		printf("Read record timestamp=%u length=%u\n",
		       get_le32(buf), get_le32(&buf[4]));
		if (fwrite(buf, RECLEN, res, log_out) != res)
			perror("fwrite");
	}

	fclose(log_in);
	fclose(log_out);

	return 0;
}
