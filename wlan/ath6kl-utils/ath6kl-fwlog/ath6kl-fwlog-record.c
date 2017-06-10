/*
 * ath6kl fwlog recorder
 * Copyright (c) 2012, Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define HDRLEN 8
#define ATH6KL_FWLOG_PAYLOAD_SIZE 1500
#define RECLEN (HDRLEN + ATH6KL_FWLOG_PAYLOAD_SIZE)

static FILE *fwlog_in;
static FILE *fwlog_res;
static FILE *log_out;
const char *fwlog_res_file;
unsigned char buf[RECLEN];
int max_records;
int record;

static unsigned int get_le32(const unsigned char *pos)
{
	return pos[0] | (pos[1] << 8) | (pos[2] << 16) | (pos[3] << 24);
}

static size_t capture(FILE *out_log, FILE *in_log)
{
	size_t res;

	while ((res = fread(buf, RECLEN, 1, in_log)) == 1)  {
		printf("Read record timestamp=%u length=%u\n",
		       get_le32(buf), get_le32(&buf[4]));
		fseek(out_log, record * RECLEN, SEEK_SET);
		if (fwrite(buf, RECLEN, res, out_log) != res)
			perror("fwrite");
		record++;
		if (record == max_records)
			record = 0;
	}

	return res;
}

static size_t get_residual(FILE *out_log, FILE *in_log)
{
	int fd = fileno(in_log);

	/* just read as much from this file as possible since we can't make any
	 * assumptions about its size  */
	/* must align to RECLEN, so only write / seek whole packets */
	memset(buf, 0, RECLEN);
	while (read(fd, buf, RECLEN) > 0) {
		printf("Read record timestamp=%u length=%u\n",
		       get_le32(buf), get_le32(&buf[4]));
		fseek(out_log, record * RECLEN, SEEK_SET);
		if (fwrite(buf, RECLEN, 1, out_log) != 1)
			perror("fwrite");
		record++;
		if (record == max_records)
			record = 0;
		memset(buf, 0, RECLEN);
	}

	return 0;
}
static void cleanup(void) {
	fclose(fwlog_in);
	fwlog_res = fopen(fwlog_res_file, "r");
	if (fwlog_res == NULL) {
		perror("Failed to open residual fwlog file");
		fclose(fwlog_res);
		goto out;
	}

	get_residual(log_out, fwlog_res);
out:
	fclose(fwlog_res);
	fclose(log_out);
}

static void stop(int signum)
{
	printf("Recording stopped\n");
	cleanup();
	exit(0);
}

int main(int argc, char *argv[])
{
	int max_len;
	size_t res;

	if (argc != 4) {
		fprintf(stderr, "usage:\n"
			"%s <path to fwlog_block> <path to log file> \\\n"
			"    <max length>\n"
			"for example:\n"
			"ath6kl-fwlog-record /sys/kernel/debug/ieee80211/"
			"phy0/ath6kl/fwlog_block \\\n"
			"    /tmp/ath6kl-fwlog 1000000\n",
			argv[0]);
		return -1;
	}

	max_len = atoi(argv[3]);
	if (max_len < RECLEN) {
		fprintf(stderr, "Too small maximum length (has to be >= %d)\n",
			RECLEN);
		return -1;
	}
	max_records = max_len / RECLEN;
	printf("Storing last %d records\n", max_records);

	fwlog_in = fopen(argv[1], "r");
	if (fwlog_in == NULL) {
		perror("Failed to open fwlog_block file");
		return -1;
	}

	log_out = fopen(argv[2], "w");
	if (log_out == NULL) {
		perror("Failed to create output file");
		fclose(fwlog_in);
		return -1;
	}

	fprintf(stderr, "%s", strtok(argv[1], "_"));
	fwlog_res_file = strtok(argv[1], "_");

	signal(SIGINT, stop);
	signal(SIGTERM, stop);

	res = capture(log_out, fwlog_in);

	printf("Incomplete read: %d bytes\n", (int) res);

	cleanup();

	return 0;
}
