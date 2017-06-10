/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
QCEDEV driver test app.
*********************************************************************/
#include "qcedev_test.h"


#define NUM_ITERATIONS 10000
#define MAX_PACKET_DATA_KB 65
#define NUM_PTHREADS 8
#define _128K	0x20000
#define _32K	0x8000
#define _16K	0x4000

#define KEY_SIZE_16    16
#define KEY_SIZE_32    32
#define KEY_SIZE_64    64
int key_sz = 32;

static int verbose;
static int bringup;
static int internal_test_enabled;
static int pmem_enabled;

unsigned char test_128K[_128K];
unsigned char test_128K_verify[_128K];

enum test_types {
	NOMINAL,
	ADVERSARIAL,
	REPEAT,
	STRESS,
	PERFORMANCE,
	LAST_TEST,
	INTERNAL = LAST_TEST,
};

struct option testopts[] = {
	{"Nominal", no_argument, NULL, 'n'},
	{"Adversarial", no_argument, NULL, 'a'},
	{"Repeatability", no_argument, NULL, 'r'},
	{"Stress", no_argument, NULL, 's'},
	{"Performance", no_argument, NULL, 'p'},
	{"Verbose", no_argument, NULL, 'v'},
	{"Help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
};

FILE *fp;

/*********************************************************************
**  Data types for Performance Testing
*********************************************************************/
static unsigned int enc_time_record[NUM_ITERATIONS];
static unsigned int dec_time_record[NUM_ITERATIONS];
static unsigned int sha_time_record[NUM_ITERATIONS];

/*********************************************************************
**  Driver handles (qcedev and pmem)
*********************************************************************/
static int qcedev_fd;
static int pmem_fd;
void *pmem_src;
pthread_mutex_t plock;

/*********************************************************************
**  print_text() - print the text into the dmesg buffer.
*********************************************************************/
static void print_text(char *intro_message, unsigned char *text_addr,
						unsigned int size)
{
	unsigned int   i;

	printf("%s @ address = 0x%p\n", intro_message, text_addr);
	for (i = 0;  i < size;  i++) {
		printf("%2x ", text_addr[i]);
		if ((i & 0xf) == 0xf)
			printf("\n");
	}
	printf("\n");
}

/*********************************************************************
** _time_diff() - Calculate the time difference in micro sec.
*********************************************************************/
static int _time_diff(struct timespec *stp, struct timespec *etp)
{
	int diff;

	if (etp->tv_sec !=  stp->tv_sec) {
		diff = (etp->tv_nsec + 1000000000 - stp->tv_nsec)/1000;
		etp->tv_sec--;
		diff += (etp->tv_sec -  stp->tv_sec) * 1000000;
	} else {
		diff = (etp->tv_nsec - stp->tv_nsec)/1000;
	}
	return diff;
};

static int qcedev_cipher_vbuf_speedtest(int fd, int packet_size,
					enum qcedev_cipher_alg_enum alg,
					enum qcedev_cipher_mode_enum mode,
					uint32_t key_sz)

{

	unsigned long long total_time = 0;
	int i, j;
	struct qcedev_cipher_op_req  req;
	struct timespec stp;
	struct timespec etp;
	unsigned int  usec;
	int first_time = 1;
	static unsigned char iv[16] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	printf("\n\n");
	printf("---------------------------------------------------\n");
	printf("cipher_vbuf_speedtest packet_size  = 0x%x\n", packet_size);
	printf("---------------------------------------------------\n");

	for (i = 0; i < 16; i++)
		iv[i] = (1 + i) % NUM_ITERATIONS;

	for (i = 0; i < (_32K * 4); i++)
		test_128K[i] = i % 255;


	for (i = 0; i < NUM_ITERATIONS; i++) {
		enc_time_record[i] = 0;
		dec_time_record[i] = 0;
	}

	memset(req.enckey, 0, 32);
	memset(req.iv, 0, 32);
	memcpy(req.enckey, key, key_sz);
	memcpy(req.iv, iv, 16);
	req.ivlen = 16;

	req.byteoffset  = 0;
	req.data_len = packet_size;
	req.encklen  = key_sz;
	req.alg = alg;
	req.mode = mode;

	req.use_pmem = 0;

	req.vbuf.src[0].vaddr =  &test_128K[0];
	req.vbuf.src[0].len =  packet_size;
	req.vbuf.dst[0].vaddr =  &test_128K[0];
	req.vbuf.dst[0].len =  packet_size;

	req.in_place_op = 1;
	req.entries = 1;

	req.op = QCEDEV_OPER_ENC;
	printf("Encryption packet size %d\n", packet_size);

	for (i = 0; i < NUM_ITERATIONS; i++) {
		if (((i % 200) == 0) && (i > 0))
			printf("X");
		if ((verbose == 1) && (first_time == 1))
			print_text("Original data", &test_128K[0],
					(unsigned int)(packet_size/64));


		if (clock_gettime(CLOCK_MONOTONIC, &stp) < 0) {
			printf("can't get time\n");
			return 0;
		}

		ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);

		if (clock_gettime(CLOCK_MONOTONIC, &etp) < 0) {
			printf("can't get time\n");
			return 0;
		}

		usec = _time_diff(&stp, &etp);
		enc_time_record[i] = usec;
		if ((verbose == 1) && (first_time == 1)) {
			print_text("Encrypted data", &test_128K[0],
							packet_size/64);
			printf("\nTime for encrypting packet (0x%x) = %d usec\n"
					, (uint32_t)packet_size
					, (uint32_t)usec);
		}

		memcpy(req.iv, iv, 16);
		req.op = QCEDEV_OPER_DEC;
		if (clock_gettime(CLOCK_MONOTONIC, &stp) < 0) {
			printf("can't get time\n");
			return 0;
		}

		ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);

		if (clock_gettime(CLOCK_MONOTONIC, &etp) < 0) {
			printf("can't get time\n");
			return 0;
		}
		usec = _time_diff(&stp, &etp);
		dec_time_record[i] = usec;
		if ((verbose == 1) && (first_time == 1)) {
			printf("Time for decrypting packet (0x%x) = %d usec\n",
					(uint32_t)packet_size, (uint32_t)usec);
			print_text("Decrypted data", &test_128K[0],
							packet_size/64);
			first_time = 0;
		}

		for (j = 0; j < (_32K * 4); j++)
			test_128K[j] = j % 255;

		memcpy(req.iv, iv, 16);
		req.alg = QCEDEV_ALG_AES;
		req.mode = QCEDEV_AES_MODE_CTR;

		req.vbuf.src[0].vaddr = &test_128K[0];
		req.vbuf.src[0].len = packet_size;
		req.vbuf.dst[0].vaddr = &test_128K[0];
		req.vbuf.dst[0].len = packet_size;

		req.data_len = packet_size;
		req.op = QCEDEV_OPER_ENC;
		req.use_pmem = 0;
		req.in_place_op = 1;
		req.byteoffset = 0;
		req.entries = 1;

		/* Reset first time to 0 at the end of first iteration*/
		if (i == 0)
			first_time = 0;
	}
	total_time = 0;
	for (i = 0; i < NUM_ITERATIONS; i++)
		total_time += (unsigned long)enc_time_record[i];
	printf("\nTotal enc time for %d iteration of packet (0x%x b)= %d us\n",
		NUM_ITERATIONS, packet_size, (unsigned int)total_time);
	printf("Average enc time for %d iteration  = %d usec\n",
		NUM_ITERATIONS, (unsigned int)(total_time/NUM_ITERATIONS));

	fprintf(fp, "Enc::PacketSize = %d:: AverageTime = %d\n", packet_size,
		(unsigned int)(total_time/NUM_ITERATIONS));

	total_time = 0;
	for (i = 0; i < NUM_ITERATIONS; i++)
		total_time += (unsigned long)dec_time_record[i];
	printf("\nTotal dec time for %d iteration of packet (0x%x b)= %d us\n",
		NUM_ITERATIONS, packet_size, (unsigned int)total_time);
	printf("Average dec time for %d iteration  = %d usec\n",
		NUM_ITERATIONS, (unsigned int)(total_time/NUM_ITERATIONS));

	fprintf(fp, "Dec::PacketSize = %d:: AverageTime = %d\n", packet_size,
		(unsigned int)(total_time/NUM_ITERATIONS));
	printf("\n------------------------------------------------\n");

	return 1;
}


static int qcedev_cipher_pmem_speedtest(int fd, int packet_size,
					enum qcedev_cipher_alg_enum alg,
					enum qcedev_cipher_mode_enum mode,
					uint32_t key_sz)
{

	unsigned long total_time = 0;
	int i, j;
	struct qcedev_cipher_op_req  req;
	struct timespec stp;
	struct timespec etp;
	unsigned int  usec;
	int first_time = 1;
	unsigned char *pdata;
	void *data = pmem_src;

	printf("\n\n");
	printf("---------------------------------------------------\n");
	printf("cipher_pmem_speedtest packet_size  = 0x%x\n", packet_size);
	printf("pmem_fd = 0x%x   src = 0x%p\n", pmem_fd, pmem_src);
	printf("---------------------------------------------------\n");


	data = pmem_src;
	for (i = 0; i < 16; i++)
		iv[i] = (1 + i) % NUM_ITERATIONS;

	pdata = (unsigned char *)data;
	for (i = 0; i < packet_size; i++) {
		*pdata = i % 255;
		pdata++;
	}

	for (i = 0; i < NUM_ITERATIONS; i++) {
		enc_time_record[i] = 0;
		dec_time_record[i] = 0;
	}

	memset(req.enckey, 0, 32);
	memset(req.iv, 0, 32);
	memcpy(req.enckey, key, key_sz);
	memcpy(req.iv, iv, 16);

	req.byteoffset  = 0;
	req.data_len = packet_size;
	req.encklen = key_sz;
	req.ivlen = 16;
	req.alg = alg;
	req.mode = mode;
	req.op = QCEDEV_OPER_ENC;
	req.use_pmem = 1;

	req.pmem.fd_src =  pmem_fd;
	req.pmem.src[0].offset = 0;
	req.pmem.src[0].len = packet_size;
	req.in_place_op = 1;
	req.pmem.fd_dst =  pmem_fd;
	req.pmem.dst[0].offset = 0;
	req.pmem.dst[0].len = packet_size;
	req.entries = 1;

	req.op = QCEDEV_OPER_ENC;

	printf("Encryption packet size %d\n", packet_size);

	for (i = 0; i < NUM_ITERATIONS; i++) {
		if (((i % 200) == 0) && (i > 0))
			printf("X");

		if ((verbose == 1) && (first_time == 1))
			print_text("Original data", data, packet_size/64);

		if (clock_gettime(CLOCK_MONOTONIC, &stp) < 0) {
			printf("can't get time\n");
			return 0;
		}

		ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);


		if (clock_gettime(CLOCK_MONOTONIC, &etp) < 0) {
			printf("can't get time\n");
			return 0;
		}
		usec = _time_diff(&stp, &etp);
		enc_time_record[i] = usec;

		if ((verbose == 1) && (first_time == 1)) {
			print_text("Encrypted data",
					(unsigned char *)data, packet_size/64);
			printf("Time for encrypting packet (0x%x)= %d usec\n",
					(uint32_t)packet_size, (uint32_t)usec);
		}

		memcpy(req.iv, iv, 16);
		req.op = QCEDEV_OPER_DEC;
		if (clock_gettime(CLOCK_MONOTONIC, &stp) < 0) {
			printf("can't get time\n");
			return 0;
		}

		ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);

		if (clock_gettime(CLOCK_MONOTONIC, &etp) < 0) {
			printf("can't get time\n");
			return 0;
		}
		usec = _time_diff(&stp, &etp);
		dec_time_record[i] = usec;

		if ((verbose == 1) && (first_time == 1)) {
			printf("Time for decrypting packet (0x%x) = %d usec\n",
					(uint32_t)packet_size, (uint32_t)usec);
			print_text("Decrypted data", (unsigned char *)data,
					packet_size/64);
			first_time = 0;
		}

		pdata = (unsigned char *)data;
		for (j = 0; j < packet_size; j++) {
			*pdata = j % 255;
			pdata++;
		}

		memcpy(req.iv, iv, 16);
		req.op = QCEDEV_OPER_ENC;
		req.alg = QCEDEV_ALG_AES;
		req.mode = QCEDEV_AES_MODE_CTR;

		req.use_pmem = 1;

		req.pmem.fd_src =  pmem_fd;
		req.pmem.src[0].offset = 0;
		req.pmem.src[0].len = packet_size;
		req.in_place_op = 1;
		req.byteoffset = 0;
		req.pmem.fd_dst = pmem_fd;
		req.pmem.dst[0].offset = 0;
		req.pmem.dst[0].len = packet_size;
		req.entries = 1;

		/* Reset first time to 0 at the end of first iteration*/
		if (i == 0)
			first_time = 0;
	}
	total_time = 0;
	for (i = 0; i < NUM_ITERATIONS; i++)
		total_time += (unsigned long)enc_time_record[i];
	printf("\nTotal enc time for %d iteration of pkt (0x%x B)= %d us\n",
		NUM_ITERATIONS, packet_size, (unsigned int)total_time);
	printf("Average enc time for %d iteration  = %d usec\n",
		NUM_ITERATIONS, (unsigned int)(total_time/NUM_ITERATIONS));

	fprintf(fp, "Enc::PacketSize = %d:: AverageTime = %d\n", packet_size,
		(unsigned int)(total_time/NUM_ITERATIONS));

	total_time = 0;
	for (i = 0; i < NUM_ITERATIONS; i++)
		total_time += (unsigned long)dec_time_record[i];
	printf("\nTotal dec time for %d iteration of pkt (0x%x B)= %d us\n",
		NUM_ITERATIONS, packet_size, (unsigned int)total_time);
	printf("Average dec time for %d iteration  = %d usec\n",
		NUM_ITERATIONS, (unsigned int)(total_time/NUM_ITERATIONS));

	fprintf(fp, "Dec::PacketSize = %d:: AverageTime = %d\n", packet_size,
		(unsigned int)(total_time/NUM_ITERATIONS));
	printf("\n------------------------------------------------\n");

	return 1;
}

static int qcedev_sha_vbuf_speedtest(int fd, int packet_size,
					enum qcedev_sha_alg_enum alg, int iuf)
{
	unsigned long long total_time = 0;
	int i, j;
	struct qcedev_sha_op_req  req;
	struct timespec stp;
	struct timespec etp;
	unsigned int  usec;
	int first_time = 1;


	printf("\n\n");
	printf("---------------------------------------------------\n");
	printf("sha1_vbuf_speedtest packet_size  = 0x%x\n", packet_size);
	printf("---------------------------------------------------\n");

	for (i = 0; i < _128K; i++)
		test_128K[i] = i % 255;

	for (i = 0; i < NUM_ITERATIONS; i++)
		sha_time_record[i] = 0;

	req.data[0].vaddr = &test_128K[0];
	req.data[0].len = packet_size;
	req.data_len = packet_size;
	req.entries = 1;
	memset(req.digest, 0, QCEDEV_MAX_SHA_DIGEST);
	req.alg = alg;

	printf("Input packet size %d bytes\n", packet_size);
	for (i = 0; i < NUM_ITERATIONS; i++) {
		if (((i % 200) == 0) && (i > 0))
			printf("X");
		if ((verbose == 1) && (first_time == 1))
			print_text("Original data", &test_128K[0],
					(unsigned int)(packet_size/64));

		if (clock_gettime(CLOCK_MONOTONIC, &stp) < 0) {
			printf("can't get time\n");
			return 0;
		}

		if (iuf) {
			ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req);
			ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req);
			ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req);
		} else
			ioctl(fd, QCEDEV_IOCTL_GET_SHA_REQ, &req);

		if (clock_gettime(CLOCK_MONOTONIC, &etp) < 0) {
			printf("can't get time\n");
			return 0;
		}

		usec = _time_diff(&stp, &etp);
		sha_time_record[i] = usec;
		if ((verbose == 1) && (first_time == 1)) {
			print_text("Calculated Digest", (unsigned char *)
							&req.digest,
							req.diglen);
			printf("\nTime for creating digest on packet"
				" (0x%x) = %d usec\n", (uint32_t)packet_size,
				(uint32_t)usec);
			printf("Digest Length is %x\n", req.diglen);
		}

		req.data[0].vaddr = &test_128K[0];
		req.data[0].len = packet_size;
		req.data_len = packet_size;
		req.entries = 1;
		memset(req.digest, 0, QCEDEV_MAX_SHA_DIGEST);
		req.alg = alg;

		/* Reset first time to 0 at the end of first iteration*/
		if (i == 0)
			first_time = 0;
	}

	total_time = 0;
	for (i = 0; i < NUM_ITERATIONS; i++)
		total_time += (unsigned long)sha_time_record[i];
	printf("\nTotal SHA time for %d iteration of packet (0x%x b)= %d us\n",
		NUM_ITERATIONS, packet_size, (unsigned int)total_time);
	printf("Average SHA time for %d iteration  = %d usec\n",
		NUM_ITERATIONS, (unsigned int)(total_time/NUM_ITERATIONS));

	fprintf(fp, "SHA::PacketSize = %d:: AverageTime = %d\n", packet_size,
		(unsigned int)(total_time/NUM_ITERATIONS));
	printf("\n------------------------------------------------\n");

	return 1;
}

static int qcedev_set_iv(struct qcedev_cipher_op_req *req, unsigned char *iv,
				unsigned int ivlen,
				enum qcedev_cipher_mode_enum mode)
{
	memset(req->iv, 0, 32);
	if ((mode == QCEDEV_AES_MODE_ECB) || (mode == QCEDEV_DES_MODE_ECB))
		req->ivlen  = 0;
	else {
		memcpy(req->iv, iv, ivlen);
		req->ivlen  = ivlen;
	}
	return 0;
}

static int qcedev_cipher_aes_basic_test(int fd)
{
	int i, num_tv;
	struct qcedev_cipher_op_req  req;
	int errors = 0;
	unsigned char *lvbuf_src;
	unsigned char *lvbuf_dst;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_aes_basic_test fd  =0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}

	lvbuf_src = malloc(0x100);
	if (!lvbuf_src) {
		printf("Failed to allocate memory for src buffer\n");
		return -1;
	}
	lvbuf_dst = malloc(0x100);
	if (!lvbuf_dst) {
		printf("Failed to allocate memory for dst buffer\n");
		free(lvbuf_src);
		return -1;
	}
	num_tv = (sizeof(cipher_aes_tv))/(sizeof(struct test_vector));

	for (i = 0; i < num_tv; i++) {
		memset(req.enckey, 0, 32);
		memcpy(req.enckey, cipher_aes_tv[i].key,
					cipher_aes_tv[i].klen);
		qcedev_set_iv(&req, (unsigned char *)cipher_aes_tv[i].iv,
					cipher_aes_tv[i].ivlen,
					cipher_aes_tv[i].mode);
		req.encklen  = cipher_aes_tv[i].klen;
		req.byteoffset  = 0;
		req.data_len = cipher_aes_tv[i].ilen;
		req.alg = cipher_aes_tv[i].c_alg;
		req.mode = cipher_aes_tv[i].mode;

		memset(&lvbuf_dst[0], 0xFF, 0x100);
		memset(&lvbuf_src[0], 0xFF, 0x100);
		memcpy(&lvbuf_src[0], cipher_aes_tv[i].input,
					cipher_aes_tv[i].ilen);

		req.vbuf.src[0].vaddr = &lvbuf_src[0];
		req.vbuf.src[0].len = cipher_aes_tv[i].ilen;
		req.vbuf.dst[0].vaddr = &lvbuf_dst[0];
		req.vbuf.dst[0].len = cipher_aes_tv[i].rlen;

		req.use_pmem = 0;
		req.in_place_op = 0;
		req.entries = 1;

		req.op = QCEDEV_OPER_ENC;
		ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);
		if (!(memcmp(&lvbuf_dst[0], cipher_aes_tv[i].result,
					cipher_aes_tv[i].rlen) == 0)) {
			errors += 1;
			if (verbose) {
				printf("AES mode %d Encryption Failed: tv %d\n",
						cipher_aes_tv[i].mode, i);
				print_text("Input data",
						(unsigned char *)
						cipher_aes_tv[i].input,
						cipher_aes_tv[i].ilen);
				print_text("Encrypted Data", &lvbuf_dst[0],
						cipher_aes_tv[i].ilen);
				print_text("Expected encrypted data",
						(unsigned char *)
						cipher_aes_tv[i].result,
						cipher_aes_tv[i].rlen);
			}
		}
	}

	for (i = 0; i < num_tv; i++) {
		memset(req.enckey, 0, 32);
		memcpy(req.enckey, cipher_aes_tv[i].key,
						cipher_aes_tv[i].klen);
		qcedev_set_iv(&req, (unsigned char *)cipher_aes_tv[i].iv,
						cipher_aes_tv[i].ivlen,
						cipher_aes_tv[i].mode);
		req.encklen  = cipher_aes_tv[i].klen;
		req.byteoffset  = 0;
		req.data_len = cipher_aes_tv[i].rlen;
		req.alg = cipher_aes_tv[i].c_alg;
		req.mode = cipher_aes_tv[i].mode;

		memset(&lvbuf_dst[0], 0xFF, 0x100);
		memset(&lvbuf_src[0], 0xFF, 0x100);
		memcpy(&lvbuf_src[0], cipher_aes_tv[i].result,
					cipher_aes_tv[i].rlen);

		req.vbuf.src[0].vaddr = &lvbuf_src[0];
		req.vbuf.src[0].len = cipher_aes_tv[i].rlen;
		req.vbuf.dst[0].vaddr = &lvbuf_dst[0];
		req.vbuf.dst[0].len = cipher_aes_tv[i].rlen;

		req.use_pmem = 0;
		req.in_place_op = 0;
		req.entries = 1;
		req.op = QCEDEV_OPER_DEC;
		ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);
		if (!(memcmp(&lvbuf_dst[0], cipher_aes_tv[i].input,
					cipher_aes_tv[i].ilen) == 0)) {
			errors += 1;
			if (verbose) {
				printf("AES mode %d Decryption Failed: tv=%d\n",
						cipher_aes_tv[i].mode, i);
				print_text("Input data",
						(unsigned char *)
						cipher_aes_tv[i].result,
						cipher_aes_tv[i].rlen);
				print_text("Decrypted Data", &lvbuf_dst[0],
						cipher_aes_tv[i].ilen);
				print_text("Expected decrypted data",
						(unsigned char *)
						cipher_aes_tv[i].input,
						cipher_aes_tv[i].ilen);
			}
		}
	}

	if (verbose || internal_test_enabled) {
		if (errors  == 0)
			printf("Basic AES Cipher Test: PASS!!!\n");
		else
			printf("Basic AES Cipher Test: FAILED %d Test(s) "
				"out of %d\n", errors, num_tv);
	}

	free(lvbuf_src);
	free(lvbuf_dst);

	return errors;
}

static int qcedev_cipher_des_basic_test(int fd)
{
	int i, num_tv;
	struct qcedev_cipher_op_req  req;
	int errors = 0;
	unsigned char *lvbuf_src;
	unsigned char *lvbuf_dst;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_des_basic_test fd  =0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}
	lvbuf_src = malloc(0x100);
	if (!lvbuf_src) {
		printf("Failed to allocate memory for src buffer\n");
		return -1;
	}
	lvbuf_dst = malloc(0x100);
	if (!lvbuf_dst) {
		printf("Failed to allocate memory for dst buffer\n");
		free(lvbuf_src);
		return -1;
	}

	num_tv = (sizeof(cipher_des_tv))/(sizeof(struct test_vector));

	for (i = 0; i < num_tv; i++) {
		memset(req.enckey, 0, 32);
		memcpy(req.enckey, cipher_des_tv[i].key,
					cipher_des_tv[i].klen);
		qcedev_set_iv(&req, (unsigned char *)cipher_des_tv[i].iv,
					cipher_des_tv[i].ivlen,
					cipher_des_tv[i].mode);
		req.encklen  = cipher_des_tv[i].klen;
		req.byteoffset  = 0;
		req.data_len = cipher_des_tv[i].ilen;
		req.alg = cipher_des_tv[i].c_alg;
		req.mode = cipher_des_tv[i].mode;

		memset(&lvbuf_dst[0], 0xFF, 0x100);
		memset(&lvbuf_src[0], 0xFF, 0x100);
		memcpy(&lvbuf_src[0], cipher_des_tv[i].input,
					cipher_des_tv[i].ilen);

		req.vbuf.src[0].vaddr = &lvbuf_src[0];
		req.vbuf.src[0].len = cipher_des_tv[i].ilen;
		req.vbuf.dst[0].vaddr = &lvbuf_dst[0];
		req.vbuf.dst[0].len = cipher_des_tv[i].rlen;

		req.use_pmem = 0;
		req.in_place_op = 0;
		req.entries = 1;

		req.op = QCEDEV_OPER_ENC;
		ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);
		if (!(memcmp(&lvbuf_dst[0], cipher_des_tv[i].result,
					cipher_des_tv[i].rlen) == 0)) {
			errors += 1;
			if (verbose) {
				printf("DES mode %d Encryption Failed: tv %d\n",
						cipher_des_tv[i].mode, i);
				print_text("Input data",
						(unsigned char *)
						cipher_des_tv[i].input,
						cipher_des_tv[i].ilen);
				print_text("Encrypted Data", &lvbuf_dst[0],
						cipher_des_tv[i].ilen);
				print_text("Expected encrypted data",
						(unsigned char *)
						cipher_des_tv[i].result,
						cipher_des_tv[i].rlen);
			}
		}
	}

	for (i = 0; i < num_tv; i++) {
		memset(req.enckey, 0, 32);
		memcpy(req.enckey, cipher_des_tv[i].key,
						cipher_des_tv[i].klen);
		qcedev_set_iv(&req, (unsigned char *)cipher_des_tv[i].iv,
						cipher_des_tv[i].ivlen,
						cipher_des_tv[i].mode);
		req.encklen  = cipher_des_tv[i].klen;
		req.byteoffset  = 0;
		req.data_len = cipher_des_tv[i].rlen;
		req.alg = cipher_des_tv[i].c_alg;
		req.mode = cipher_des_tv[i].mode;

		memset(&lvbuf_dst[0], 0xFF, 0x100);
		memset(&lvbuf_src[0], 0xFF, 0x100);
		memcpy(&lvbuf_src[0], cipher_des_tv[i].result,
					cipher_des_tv[i].rlen);

		req.vbuf.src[0].vaddr = &lvbuf_src[0];
		req.vbuf.src[0].len = cipher_des_tv[i].rlen;
		req.vbuf.dst[0].vaddr = &lvbuf_dst[0];
		req.vbuf.dst[0].len = cipher_des_tv[i].rlen;

		req.use_pmem = 0;
		req.in_place_op = 0;
		req.entries = 1;
		req.op = QCEDEV_OPER_DEC;
		ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);
		if (!(memcmp(&lvbuf_dst[0], cipher_des_tv[i].input,
					cipher_des_tv[i].ilen) == 0)) {
			errors += 1;
			if (verbose) {
				printf("DES mode %d Decryption Failed: tv=%d\n",
						cipher_des_tv[i].mode, i);
				print_text("Input data",
						(unsigned char *)
						cipher_des_tv[i].result,
						cipher_des_tv[i].rlen);
				print_text("Decrypted Data", &lvbuf_dst[0],
						cipher_des_tv[i].ilen);
				print_text("Expected decrypted data",
						(unsigned char *)
						cipher_des_tv[i].input,
						cipher_des_tv[i].ilen);
			}
		}
	}

	if (verbose || internal_test_enabled) {
		if (errors  == 0)
			printf("Basic DES Cipher Test: PASS!!!\n");
		else
			printf("Basic DES Cipher Test: FAILED %d Test(s) "
				"out of %d\n", errors, num_tv);
	}
	free(lvbuf_src);
	free(lvbuf_dst);
	return errors;
}

static int qcedev_cipher_basic_test(int fd)
{
	int errors = 0;
	errors += qcedev_cipher_aes_basic_test(fd);
	errors += qcedev_cipher_des_basic_test(fd);
	return errors;
}

static int qcedev_cipher_pmem_single_sg_test(int fd,
		enum qcedev_cipher_alg_enum alg,
		enum qcedev_cipher_mode_enum mode, int pmem_fd, void *src,
		unsigned char *data_src, unsigned char *data_dst,
		unsigned int len, unsigned char *key, unsigned char *iv,
		int no_key,
		uint32_t key_sz)
{
	int i;
	int usec;
	int pass = 0;
	struct qcedev_cipher_op_req  req;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_pmem_single_test fd=0x%x\n", fd);
		printf("pmem_fd=0x%x src=0x%p\n", pmem_fd, src);
		printf("---------------------------------------------------\n");
	}
	/* copy original data for verification of enc/dec */
	memcpy(&vbuf_src[0], data_src, len);

	memset(req.enckey, 0, 32);
	memcpy(req.enckey, key, key_sz);
	qcedev_set_iv(&req, iv, 16, mode);

	req.byteoffset  = 0;
	req.data_len = len;
	req.encklen  = key_sz;
	req.alg = alg;
	req.mode = mode;
	req.use_pmem = 1;
	req.in_place_op = 1;
	req.entries = 1;

	req.op = QCEDEV_OPER_ENC;

	memcpy(src, data_src, len);
	req.pmem.fd_src =  pmem_fd;
	req.pmem.src[0].offset = 0;
	req.pmem.src[0].len = len;
	req.entries = 1;

	req.pmem.fd_dst =  pmem_fd;
	req.pmem.dst[0].offset = 0;
	req.pmem.dst[0].len = len;

	ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);

	/* copy encrypted data for verification of enc/dec */
	memcpy(&vbuf_dst[0], src, len);

	if (no_key) {
		req.op = QCEDEV_OPER_DEC_NO_KEY;
		req.encklen = 0;
		memset(req.enckey, 0, 32);
	}

	req.pmem.fd_src =  pmem_fd;
	req.pmem.src[0].offset = 0;
	req.pmem.src[0].len = len;
	req.entries = 1;
	qcedev_set_iv(&req, iv, 16, mode);
	req.pmem.fd_dst =  pmem_fd;
	req.pmem.dst[0].offset = 0;
	req.pmem.dst[0].len = len;

	req.byteoffset = 0;
	req.data_len = len;
	req.op = QCEDEV_OPER_DEC;
	ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);

	pass = memcmp(&vbuf_src[0], (uint8_t *)src, len);

	if (((pass != 0) || (bringup == 1)) && (verbose == 1)) {
		print_text("Original data", &vbuf_src[0], len);
		print_text("Encrypted data", &vbuf_dst[0], len);
		print_text("Decrypted data", (unsigned char *)src, len);
	}

	if (pass == 0) {
		if (verbose)
			printf("Encryption/decryption single-sgP (alg %x, mode"
				" %x) PASSED\n", (uint32_t)alg, (uint32_t)mode);
		return 1;
	} else {
		printf("Encryption/decryption single-sgP (alg %x, mode %x)"
				"FAILED\n", (uint32_t)alg, (uint32_t)mode);
		return 0;
	}
}

static int qcedev_cipher_pmem_mult_sg_test(int fd,
			enum qcedev_cipher_alg_enum alg,
			enum qcedev_cipher_mode_enum mode, int pmem_fd,
			void *src, unsigned char *key, unsigned char *iv,
			int no_key,
			uint32_t key_sz)
{
	int i;
	struct qcedev_cipher_op_req  req;
	int sg_size0, sg_size1, sg_size2;
	int pass = 0;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_pmem_mult_sg_test fd=0x%x\n", fd);
		printf("pmem_fd=0x%x src=0x%p\n", pmem_fd, src);
		printf("---------------------------------------------------\n");
	}
	memset(req.enckey, 0, 32);
	memcpy(req.enckey, key, key_sz);
	qcedev_set_iv(&req, iv, 16, mode);

	if ((alg == QCEDEV_ALG_AES) &&
		(mode == QCEDEV_AES_MODE_CTR)) {
		sg_size0 = 58;
		sg_size1 = 42;
		sg_size2 = 28;
	} else {
		sg_size0 = 0x40;
		sg_size1 = 0x30;
		sg_size2 = 0x10;
	}

	req.byteoffset = 0;
	req.data_len = sg_size0 + sg_size1 + sg_size2;
	req.encklen = key_sz;
	req.alg = alg;
	req.mode = mode;
	req.use_pmem = 1;
	req.in_place_op = 1;
	req.entries = 1;

	req.op = QCEDEV_OPER_ENC;

	memset(src, 0xFF, 0x1000);
	memcpy(src, &PlainText_90[0] , sg_size0);
	memcpy((char *)src + 100, &PlainText_60[0] , sg_size1);
	memcpy((char *)src + 200, &PlainText_90[0] , sg_size2);

	/* copy original data for verification */
	memset(&vbuf_src[0], 0xFF, 0x1000);
	memcpy(&vbuf_src[0], src, 200 + sg_size2);

	req.pmem.fd_src =  pmem_fd;
	req.pmem.src[0].offset = 0;
	req.pmem.src[0].len = sg_size0;
	req.pmem.src[1].offset = 100;
	req.pmem.src[1].len = sg_size1;
	req.pmem.src[2].offset = 200;
	req.pmem.src[2].len = sg_size2;
	req.entries = 3;

	req.pmem.fd_dst =  pmem_fd;
	req.pmem.dst[0].offset = 0;
	req.pmem.dst[0].len = sg_size0;
	req.pmem.dst[1].offset = 100;
	req.pmem.dst[1].len = sg_size1;
	req.pmem.dst[2].offset = 200;
	req.pmem.dst[2].len = sg_size2;

	ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);

	/* copy encrypted data for verification */
	memset(&vbuf_dst[0], 0xFF, 0x1000);
	memcpy(&vbuf_dst[0], src, 200 + sg_size2);

	if (no_key) {
		req.op = QCEDEV_OPER_DEC_NO_KEY;
		req.encklen  = 0;
		memset(req.enckey, 0, 32);
	}

	qcedev_set_iv(&req, iv, 16, mode);
	req.byteoffset  = 0;
	req.op = QCEDEV_OPER_DEC;
	ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);


	pass = memcmp(&vbuf_src[0], (uint8_t *)src, 200 + sg_size2);

	if (((pass != 0) || (bringup == 1)) && (verbose == 1)) {
		print_text("Original data", &vbuf_src[0], 200 + sg_size2);
		print_text("Encrypted data", &vbuf_dst[0], 200 + sg_size2);
		print_text("Decrypted data", (unsigned char *)src, 400);
	}

	if (pass == 0) {
		if (verbose)
			printf("Encryption/Decryption multi-sgP (alg %x, mode "
				"%x) PASSED\n", (uint32_t)alg, (uint32_t)mode);
		return 1;
	} else {
			printf("Encryption/Decryption multi-sgP (alg %x, mode "
				"%x) FAILED\n", (uint32_t)alg, (uint32_t)mode);
		return 0;
	}

}

static int qcedev_cipher_pmem_mult_sg_large_buf_test(int fd,
				enum qcedev_cipher_alg_enum alg,
				enum qcedev_cipher_mode_enum mode, int pmem_fd,
				void *src, unsigned char *key,
				unsigned char *iv, int no_key, uint32_t key_sz)
{
	int i;
	struct qcedev_cipher_op_req  req;
	int pass = 0;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_pmem_mult_sg_large_buf_test fd=0x%x\n",
								fd);
		printf("pmem_fd=0x%x src=0x%p\n", pmem_fd, src);
		printf("---------------------------------------------------\n");
	}
	memset(req.enckey, 0, 32);
	memcpy(req.enckey, key, key_sz);
	qcedev_set_iv(&req, iv, 16, mode);

	req.byteoffset = 0;
	req.encklen = key_sz;
	req.alg = alg;
	req.mode = mode;
	req.use_pmem = 1;
	req.in_place_op = 1;

	memcpy((uint8_t *)src, &test_128K[0],  _32K * 4);

	req.pmem.fd_src =  pmem_fd;
	req.pmem.src[0].offset = 0;
	req.pmem.src[0].len = _32K * 2;
	req.pmem.src[1].offset = _32K * 3;
	req.pmem.src[1].len = _32K;
	req.entries = 2;

	req.pmem.dst[0].offset = 0;
	req.pmem.dst[0].len = _32K * 2;
	req.pmem.dst[1].offset = _32K * 3;
	req.pmem.dst[1].len = _32K;

	req.data_len = _32K * 3;

	req.op = QCEDEV_OPER_ENC;

	ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);

	memcpy(&test_128K_verify[0], (uint8_t *)src, (_32K * 4));

	if (memcmp(&test_128K[0], (uint8_t *)src, (_32K * 4)) == 0) {
		print_text("\nEncrypted data ", (unsigned char *)src,
				(_32K)/200);
		print_text("\nEncrypted data at offset of  +_32K*3",
				((unsigned char *)src + (_32K*3)), (_32K)/200);
		printf("Encryption of 32KB_plus PMEM buffer  FAIL!!!\n");
		return 0;
	} else {
		if (verbose)
			printf("Encryption of 32KB+ PMEM buffer PASS!!!\n");
	}

	if (no_key) {
		req.op = QCEDEV_OPER_DEC_NO_KEY;
		req.encklen = 0;
		memset(req.enckey, 0, 32);
	}

	req.byteoffset = 0;
	qcedev_set_iv(&req, iv, 16, mode);
	req.op = QCEDEV_OPER_DEC;
	ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);

	pass = memcmp(&test_128K[0], (uint8_t *)src, _32K*4);
	if (((pass != 0) || (bringup == 1)) && (verbose == 1)) {
		print_text("\nOriginal data src", &test_128K[0], (_32K)/200);
		print_text("\nOriginal data src +_32K*3", &test_128K[_32K*3],
								(_32K)/200);
		print_text("\nEncrypted data src", &test_128K_verify[0],
								(_32K)/200);
		print_text("\nEncrypted dat src +_32K*3",
				&test_128K_verify[_32K*3], (_32K)/200);
		print_text("\nDecrypted data src", (unsigned char *)src,
				(_32K)/200);
		print_text("\nDecrypted dat src + _32K*3",
				((unsigned char *)src + (_32K*3)), (_32K)/200);
	}

	if (pass == 0) {
		if (verbose)
			printf("Decryption of 32KB_plus packet P (alg %x, mode"
				"%x) PASSED\n", (uint32_t)alg, (uint32_t)mode);
		return 1;
	} else {
		printf("Decryption of 32KB_plus packet P (alg %x, mode %x) "
				"FAILED\n", (uint32_t)alg, (uint32_t)mode);
		return 0;
	}
}

static int qcedev_cipher_vbuf_mult_sg_large_buf_test(int fd,
				enum qcedev_cipher_alg_enum alg,
				enum qcedev_cipher_mode_enum mode,
				unsigned char *key, unsigned char * iv,
				int no_key, uint32_t key_sz)
{
	int i;
	struct qcedev_cipher_op_req  req;
	int dec_offset = 0;
	int pass = 0;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_vbuf_mult_sg_large_buf_test fd=0x%x\n",
									fd);
		printf("---------------------------------------------------\n");
	}
	memset(req.enckey, 0, 32);
	memcpy(req.enckey, key, key_sz);
	qcedev_set_iv(&req, iv, 16, mode);

	req.byteoffset  = 0;
	req.encklen  = key_sz;
	req.alg = alg;
	req.mode = mode;
	req.use_pmem = 0;
	req.in_place_op = 1;

	for (i = 0; i < (_32K * 4); i++) {
		test_128K[i] = i % 255;
		test_128K_verify[i] = test_128K[i];
	}
	req.vbuf.src[0].vaddr = &test_128K[0];
	req.vbuf.src[0].len = _32K * 2;
	req.vbuf.src[1].vaddr = &test_128K[_32K * 3];
	req.vbuf.src[1].len = _32K;
	req.entries = 2;

	req.vbuf.dst[0].vaddr = &test_128K[0];
	req.vbuf.dst[0].len = _32K * 2;
	req.vbuf.dst[1].vaddr = &test_128K[_32K * 3];
	req.vbuf.dst[1].len = _32K;

	req.data_len = _32K * 3;
	req.op = QCEDEV_OPER_ENC;

	ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);
	if (bringup == 1) {
		print_text("\nEncrypted data src", &test_128K[0], (_32K)/200);
		print_text("\nEncrypted data src +_32K*3", &test_128K[_32K*3],
								(_32K)/200);
	}
	if (no_key) {
		req.op = QCEDEV_OPER_DEC_NO_KEY;
		req.encklen = 0;
		memset(req.enckey, 0, 32);
	}

	qcedev_set_iv(&req, iv, 16, mode);
	req.byteoffset  = dec_offset;
	req.vbuf.src[0].vaddr = &test_128K[dec_offset];
	req.vbuf.src[0].len = (_32K * 2) - dec_offset;
	req.vbuf.src[1].vaddr = &test_128K[_32K * 3];
	req.vbuf.src[1].len = _32K;
	req.entries = 2;

	req.vbuf.dst[0].vaddr = &test_128K[dec_offset];
	req.vbuf.dst[0].len = (_32K * 2) - dec_offset;
	req.vbuf.dst[1].vaddr = &test_128K[_32K * 3];
	req.vbuf.dst[1].len = _32K;
	req.op = QCEDEV_OPER_DEC;
	req.data_len = (_32K * 3) - dec_offset;
	ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);

	pass = memcmp(&test_128K[0], &test_128K_verify[0], (_32K * 4));
	if (((pass != 0) || (bringup == 1)) && (verbose == 1)) {
		print_text("\nOriginal data src", &test_128K_verify[0],
								(_32K)/200);
		print_text("\nOriginal data src +_32K*3",
				&test_128K_verify[(_32K * 3)], (_32K)/200);
		print_text("\nDecrypted data src", &test_128K[0], (_32K)/200);
		print_text("\nDecrypted data src +_32K*3", &test_128K[_32K*3],
								(_32K)/200);
	}

	if (pass == 0) {
		if (verbose)
			printf("Decryption of 32KB_plus packet (alg %x, mode"
				"%x) PASSED\n", (uint32_t)alg, (uint32_t)mode);
		return 1;
	} else {
		printf("Decryption of 32KB_plus packet (alg %x, mode %x) "
				"FAILED\n", (uint32_t)alg, (uint32_t)mode);
		return 0;
	}
}

static int qcedev_cipher_vbuf_mult_sg_test(int fd,
			enum qcedev_cipher_alg_enum alg,
			enum qcedev_cipher_mode_enum mode, unsigned char *key,
			unsigned char *iv, int no_key, uint32_t key_sz)
{
	int i;
	struct qcedev_cipher_op_req  req;
	int sg_size0, sg_size1, sg_size2;
	int pass = 0;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_vbuf_mult_sg_test fd =0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}
	memset(req.enckey, 0, 32);
	memcpy(req.enckey, key, key_sz);
	qcedev_set_iv(&req, iv, 16, mode);

	if ((alg == QCEDEV_ALG_AES) &&
		(mode == QCEDEV_AES_MODE_CTR)) {
		sg_size0 = 58;
		sg_size1 = 42;
		sg_size2 = 28;
	} else {
		sg_size0 = 0x40;
		sg_size1 = 0x30;
		sg_size2 = 0x10;
	}
	req.byteoffset  = 0;
	req.data_len = sg_size0 + sg_size1 + sg_size2;
	req.encklen = key_sz;
	req.alg = alg;
	req.mode = mode;
	req.use_pmem = 0;
	req.in_place_op = 1;

	memset(&vbuf_src[0], 0xFF , 0x1000);
	memcpy(&vbuf_src[0], &PlainText_90[0], sg_size0);
	memcpy(&vbuf_src[0] + 100, &PlainText_60[0], sg_size1);
	memcpy(&vbuf_src[0] + 200, &PlainText_90[0], sg_size2);

	req.vbuf.src[0].vaddr = &vbuf_src[0];
	req.vbuf.src[0].len = sg_size0;
	req.vbuf.src[1].vaddr = &vbuf_src[100];
	req.vbuf.src[1].len = sg_size1;
	req.vbuf.src[2].vaddr = &vbuf_src[200];
	req.vbuf.src[2].len = sg_size2;
	req.entries = 3;

	req.vbuf.dst[0].vaddr = &vbuf_src[0];
	req.vbuf.dst[0].len = sg_size0;
	req.vbuf.dst[1].vaddr = &vbuf_src[100];
	req.vbuf.dst[1].len = sg_size1;
	req.vbuf.dst[2].vaddr = &vbuf_src[200];
	req.vbuf.dst[2].len = sg_size2;

	/* copy original data for verification */
	memset(&test_128K[0], 0xFF, 0x1000);
	memcpy(&test_128K[0], &vbuf_src[0], 200 + sg_size2);

	req.op = QCEDEV_OPER_ENC;
	ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);

	/* copy encrypted data for verification */
	memset(&test_128K_verify[0], 0xFF, 0x1000);
	memcpy(&test_128K_verify[0], &vbuf_src[0], 200 + sg_size2);


	qcedev_set_iv(&req, iv, 16, mode);
	req.byteoffset = 0;
	req.op = QCEDEV_OPER_DEC;
	ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);

	pass = memcmp(&test_128K[0], &vbuf_src[0], 200 + sg_size2);

	if (((pass != 0) || (bringup == 1)) && (verbose == 1)) {
		print_text("Original data", &test_128K[0], 200 + sg_size2);
		print_text("Encrypted data", &test_128K_verify[0],
							200 + sg_size2);
		print_text("Decrypted data", &vbuf_src[0], 400);
	}

	if (pass == 0) {
		if (verbose)
			printf("Encryption/Decryption multi-sg (alg %x, mode "
				"%x) PASSED\n", (uint32_t)alg, (uint32_t)mode);
		return 1;
	} else {
			printf("Encryption/Decryption multi-sg(alg %x, mode "
				"%x) FAILED\n", (uint32_t)alg, (uint32_t)mode);
		return 0;
	}

}

static int qcedev_cipher_vbuf_single_sg_test(int fd,
			enum qcedev_cipher_alg_enum alg,
			enum qcedev_cipher_mode_enum mode,
			unsigned char *data_src,
			unsigned char *data_dst, unsigned int len,
			unsigned char *key, unsigned char* iv, int no_key,
			uint32_t offset,
			uint32_t key_sz)
{
	int i;
	struct qcedev_cipher_op_req  req;
	int pmem_fd;
	void *data;
	int use_pmem = 0;
	offset = 0;
	int pass = 0;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_cipher_vbuf_single_sg_test fd  =0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}
	memset(req.enckey, 0, 32);
	memcpy(req.enckey, key, key_sz);
	qcedev_set_iv(&req, iv, 16, mode);

	req.byteoffset  = 0;
	req.data_len = len;
	req.encklen  = key_sz;

	req.alg = alg;
	req.mode = mode;

	req.vbuf.src[0].vaddr =  data_src;
	req.vbuf.src[0].len =  len;
	req.vbuf.dst[0].vaddr =  data_dst;
	req.vbuf.dst[0].len =  len;

	req.use_pmem = 0;
	req.in_place_op = 1;
	req.entries = 1;

	req.op = QCEDEV_OPER_ENC;

	/* copy original data for verification of enc/dec */
	memset(&vbuf_src[0], 0xFF, 0x1000);
	memcpy(&vbuf_src[0], data_src, len);

	ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req);

	/* copy encrypted data for verification of enc/dec */
	memset(&vbuf_dst[0], 0xFF, 0x1000);
	memcpy(&vbuf_dst[0], data_dst, len);

	if (no_key) {
		req.op = QCEDEV_OPER_DEC_NO_KEY;
		req.encklen = 0;
		memset(req.enckey, 0, 32);
	}

	req.data_len -= offset;
	req.byteoffset = offset;

	req.vbuf.src[0].vaddr = data_dst + offset;
	req.vbuf.src[0].len = req.data_len;
	req.vbuf.dst[0].vaddr = req.vbuf.src[0].vaddr ;
	req.vbuf.dst[0].len = req.data_len;

	if (offset) {
		if (verbose) {
			printf("Encrypted data starts at offset 0x%x\n",
								offset);
			print_text("Modified Encrypted data\n",
				(unsigned char *)data_dst, req.data_len);
		}
	}
	req.op = QCEDEV_OPER_DEC;
	qcedev_set_iv(&req, iv, 16, mode);
	ioctl(fd, QCEDEV_IOCTL_DEC_REQ, &req);

	pass = memcmp((data_dst + offset), (uint8_t *)&vbuf_src[0] + offset,
							req.data_len);

	if (((pass != 0) || (bringup == 1)) && (verbose == 1)) {
		print_text("\nOriginal data src", &vbuf_src[0] + offset,
						req.data_len);
		print_text("\nEncrypted data ", &vbuf_dst[0] +
						offset,	req.data_len);
		print_text("\nDecrypted data ",
				(unsigned char *)(data_dst + offset),
				req.data_len);
	}

	if (pass == 0) {
		if (verbose)
			printf("Encryption single-sg(alg %x, mode %x) PASSED\n",
						(uint32_t)alg, (uint32_t)mode);
		return 1;
	} else {
		printf("Decryption single-sg (alg %x, mode %x) FAILED\n",
						(uint32_t)alg, (uint32_t)mode);
		return 0;
	}
}

static int qcedev_get_SHA(struct qcedev_sha_op_req *req, int fd, int alg)
{
	uint8_t digest[QCEDEV_MAX_SHA_DIGEST];
	memset(&digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	memset(&req->digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req->alg = alg;

	ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, req);
	ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, req);
	ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, req);

	memcpy(&digest[0], &req->digest[0], QCEDEV_MAX_SHA_DIGEST);
	memset(&req->digest[0], 0, QCEDEV_MAX_SHA_DIGEST);

	ioctl(fd, QCEDEV_IOCTL_GET_SHA_REQ, req);
	if (verbose || (internal_test_enabled == 1))
		print_text("Calculated Digest", (unsigned char *)&req->digest,
						req->diglen);

	if (memcmp(&req->digest[0], &digest[0], QCEDEV_MAX_SHA_DIGEST) == 0) {
		return 1;
	} else {
		printf("FAILED to calculate hash (qcedev_get_SHA)alg = %d\n",
									alg);
		return 0;
	}
}

static int qcedev_sha_basic_test(int fd, int alg)
{
	struct qcedev_sha_op_req  req;
	struct test_vector *curr_test_vector;
	int errors = 0;
	unsigned int i, num_tv;

	if (verbose == 1) {
		printf("\n\n");
		printf("----------------------------------------------\n");
		printf(" sha_basic_test: fd = 0x%x\n", fd);
	}

	req.alg = alg;
	switch (req.alg) {
	case QCEDEV_ALG_SHA1:
		if (verbose == 1)
			printf(" SHA1 Algorithm test\n");
		num_tv = (sizeof(sha1_tv))/(sizeof(struct test_vector));
		curr_test_vector = &sha1_tv[0];
		break;

	case QCEDEV_ALG_SHA256:
		if (verbose == 1)
			printf(" SHA256 Algorithm test\n");
		num_tv = (sizeof(sha256_tv))/(sizeof(struct test_vector));
		curr_test_vector = &sha256_tv[0];
		break;

	case QCEDEV_ALG_SHA1_HMAC:
		if (verbose == 1)
			printf(" SHA1-HMAC Algorithm test\n");
		num_tv = (sizeof(hmac_sha1_tv))/(sizeof(struct test_vector));
		curr_test_vector = &hmac_sha1_tv[0];
		break;

	case QCEDEV_ALG_SHA256_HMAC:
		if (verbose == 1)
			printf(" SHA256-HMAC Algorithm test\n");
		num_tv = (sizeof(hmac_sha256_tv))/(sizeof(struct test_vector));
		curr_test_vector = &hmac_sha256_tv[0];
		break;
	default:
		printf("Invalid algorithm = %d\n", alg);
		return 1;
	}

	if (verbose)
		printf("----------------------------------------------\n");

	for (i = 0; i < num_tv; i++) {
		req.data[0].vaddr = (uint8_t *)curr_test_vector[i].input;
		req.data[0].len = curr_test_vector[i].ilen;
		req.data_len = curr_test_vector[i].ilen;
		req.diglen = curr_test_vector[i].diglen;
		req.entries = 1;

		if (req.alg == QCEDEV_ALG_SHA1_HMAC ||
				req.alg == QCEDEV_ALG_SHA256_HMAC) {
			req.authkey = (uint8_t *)curr_test_vector[i].key;
			req.authklen =	curr_test_vector[i].klen;
			req.diglen = curr_test_vector[i].diglen;
		}

		memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
		qcedev_get_SHA(&req, fd, req.alg);

		if (!(memcmp(&req.digest[0], curr_test_vector[i].digest,
			curr_test_vector[i].diglen) == 0)) {
			errors += 1;
			if (verbose == 1) {
				printf("SHA test failed\n");
				print_text("input data",
						(unsigned char *)
						curr_test_vector[i].input,
						curr_test_vector[i].ilen);
				print_text("Calculated Digest",
						(unsigned char *)&req.digest[0],
						curr_test_vector[i].diglen);
				print_text("Expected Digest",
						(unsigned char *)
						curr_test_vector[i].digest,
						curr_test_vector[i].diglen);
			}
		}
	}

	if (verbose || internal_test_enabled) {
		if (errors == 0)
			printf("SHA TEST: PASS !!!\n");
		else
			printf("SHA TEST: FAILED %d TESTS OUT OF %d\n",
					errors, num_tv);
	}

	return errors;
}

static int qcedev_SHA1_SHA256_basic_test(int fd)
{
	int errors = 0;

	if (verbose == 1) {
		printf("\n\n");
		printf("----------------------------------------------\n");
		printf(" qcedev_SHA1_SHA256_basic_test: fd = 0x%x\n", fd);
		printf("----------------------------------------------\n");
	}

	errors = qcedev_sha_basic_test(fd, QCEDEV_ALG_SHA1);
	errors += qcedev_sha_basic_test(fd, QCEDEV_ALG_SHA256);
#ifdef CRYPTO_HMAC_SUPPORT
	errors += qcedev_sha_basic_test(fd, QCEDEV_ALG_SHA1_HMAC);
	errors += qcedev_sha_basic_test(fd, QCEDEV_ALG_SHA256_HMAC);
#endif

	if ((errors == 0) && (verbose == 1))
#ifdef CRYPTO_HMAC_SUPPORT
		printf("SHA(sha1,sha256,sha1-hmac,sha1-256)Basic Test: PASS\n");
#else
		printf("SHA(sha1,sha256)Basic Test: PASS\n");
#endif
	if (errors > 0)
		return 0;
	else
		return 1;
}

#ifdef CRYPTO_CMAC_SUPPORT
static int qcedev_CMAC_basic_test(int fd)
{
	struct qcedev_sha_op_req  req;
	int errors = 0;
	int i, num_tv;
	if (verbose == 1) {
		printf("\n\n");
		printf("----------------------------------------------\n");
		printf(" qcedev_CMAC_basic_test: fd = 0x%x\n", fd);
		printf("----------------------------------------------\n");
	}
	num_tv = (sizeof(aes_cmac_tv))/(sizeof(struct test_vector));

	for (i = 0; i < num_tv; i++) {
		req.alg = QCEDEV_ALG_AES_CMAC ;
		req.data[0].vaddr = (uint8_t *)aes_cmac_tv[i].input;
		req.data[0].len = aes_cmac_tv[i].ilen;
		req.data_len = aes_cmac_tv[i].ilen;
		req.authkey = (uint8_t *)aes_cmac_tv[i].key;
		req.authklen = aes_cmac_tv[i].klen;
		req.diglen = 16;
		req.entries = 1;

		memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
		ioctl(fd, QCEDEV_IOCTL_GET_CMAC_REQ, &req);

		if (!(memcmp(&req.digest[0], aes_cmac_tv[i].digest,
					aes_cmac_tv[i].diglen) == 0)) {
			errors += 1;
			if (verbose) {
				printf("CMAC test failed\n");
				print_text("Calculated CMAC :",	&req.digest[0],
							aes_cmac_tv[i].diglen);
				print_text("Expected CMAC :",
							(unsigned char *)
							aes_cmac_tv[i].digest,
							aes_cmac_tv[i].diglen);
			}
		}
	}
	if (verbose || internal_test_enabled) {
		if (errors  == 0)
			printf("CMAC Basic Test: PASS !!!\n");
		else
			printf("CMAC BASIC TEST: FAILED %d TESTS"
				"OUT OF %d\n", errors, num_tv);
	}
	return errors;
}
#else
static int qcedev_CMAC_basic_test(int fd)
{
	printf("CMAC is not supported on this device\n");
	return 0;
}
#endif

static int qcedev_SHA1_SHA256_multiple_of_64b(int fd)
{
	struct qcedev_sha_op_req  req;

	if (verbose) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf(" qcedev_SHA1_SHA256_x64: fd = 0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}
	req.data[0].vaddr = &PlainText_90[0];
	req.data[0].len = 0x80;
	req.data_len = 0x80;
	req.diglen = 0;
	req.entries = 1;

	qcedev_get_SHA(&req, fd, QCEDEV_ALG_SHA1);
	qcedev_get_SHA(&req, fd, QCEDEV_ALG_SHA256);

	return 1;
}

static int qcedev_SHA1_SHA256_multi_updates(int fd)
{
	struct qcedev_sha_op_req  req;
	struct qcedev_sha_op_req  req0;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf("qcedev_SHA1_SHA256_xupdates: fd = 0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}
	memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req.data[0].vaddr = &PlainText_90[0];
	req.data[0].len = 0x50;
	req.data_len = 0x50;
	req.diglen = 0;
	req.entries = 1;
	req.alg = QCEDEV_ALG_SHA1;

	ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req);

	ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req);
	req.data[0].vaddr = &PlainText_90[0x50];
	req.data[0].len = 0x40;
	req.data_len = 0x40;

	ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req);

	ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req);

	memset(&req0.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req0.data[0].vaddr = &PlainText_90[0];
	req0.data[0].len = 0x90;
	req0.data_len = 0x90;
	req0.diglen = 0;
	req0.entries = 1;
	req0.alg = QCEDEV_ALG_SHA1;

	ioctl(fd, QCEDEV_IOCTL_GET_SHA_REQ, &req0);

	if (bringup == 1) {
		print_text("\nOriginal data", &PlainText_90[0], 0x90);
		print_text("\nSHA1 Digest ", &req.digest[0], 0x14);
	}
	if (memcmp(&req.digest[0], &req0.digest[0], 0x14) == 0) {
		if (verbose || internal_test_enabled)
			printf("SHA multi updates PASS!!!\n");
		return 1;
	} else {
		if (verbose || internal_test_enabled) {
			printf("SHA multi updates FAILED!!!\n");
			print_text("\nExpected SHA1 Digest", &req0.digest[0],
								0x14);
		}
		return 0;
	}
}

static int qcedev_SHA_test_32KB_plus(int fd, int alg)
{
	struct qcedev_sha_op_req  req;
	struct qcedev_sha_op_req  req0;
	int i = 0;
	int pass = 1;
	char digest[QCEDEV_MAX_SHA_DIGEST];

	if (verbose) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf(" qcedev_SHA_test_32KB_plus: fd = 0x%x, algo = %d\n",
								fd, alg);
		printf("---------------------------------------------------\n");
		printf("Max Data PAcket Size = 0x%x\n", (uint32_t)_32K * 4);
	}
	/* Testing large buffer >32K*/
	for (i = 0; i < (_32K * 4); i++)
		test_128K[i] = i % 255;

	/*********************************************************************/
	memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req.data[0].vaddr = &test_128K[0];
	req.data[0].len = 38;
	req.data[1].vaddr = &test_128K[38];
	req.data[1].len = 34525;
	req.data_len = 38 + 34525;
	req.alg = alg;
	req.diglen = 0;
	req.entries = 2;

	qcedev_get_SHA(&req, fd, alg);

	memcpy(&digest[0], &req.digest[0], QCEDEV_MAX_SHA_DIGEST);

	/*********************************************************************/
	memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req.data[0].vaddr = &test_128K[0];
	req.data[0].len = 34525 ;
	req.data[1].vaddr = &test_128K[34525];
	req.data[1].len = 38;
	req.data_len  = 38 + 34525;
	req.diglen = 0;
	req.entries = 2;

	qcedev_get_SHA(&req, fd, alg);

	if (memcmp(&digest[0], &req.digest[0], QCEDEV_MAX_SHA_DIGEST) == 0)
		pass = 1;
	else {
		pass = 0;
		printf("SHA1 of large buffer failed\n");
	}

	/*********************************************************************/
	memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req.data[0].vaddr = &test_128K[0];
	req.data[0].len = _32K + 30 ;
	req.data[1].vaddr = &test_128K[_32K + 30];
	req.data[1].len = _32K + 90;
	req.data[2].vaddr = &test_128K[(2 * _32K) + 120];
	req.data[2].len = (2 * _32K) - 120;
	req.data_len = _32K * 4;
	req.diglen = 0;
	req.entries = 3;
	qcedev_get_SHA(&req, fd, alg);

	memcpy(&digest[0], &req.digest[0], QCEDEV_MAX_SHA_DIGEST);

	/*********************************************************************/
	memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req.data[0].vaddr = &test_128K[0];
	req.data[0].len = _32K + 30 ;
	req.data[1].vaddr = &test_128K[_32K + 30];
	req.data[1].len = 90;
	req.data[2].vaddr = &test_128K[_32K + 120];
	req.data[2].len = 50;
	req.data[3].vaddr = &test_128K[_32K + 170];
	req.data[3].len = (2 * _32K) + 90;
	req.data[4].vaddr = &test_128K[(3 * _32K) + 260];
	req.data[4].len = _32K - 260;
	req.data_len = _32K * 4;
	req.diglen = 0;
	req.entries = 5;

	qcedev_get_SHA(&req, fd, alg);

	if (memcmp(&digest[0], &req.digest[0], QCEDEV_MAX_SHA_DIGEST) == 0)
		pass = 1;
	else {
		pass = 0;
		if (verbose) {
			printf("SHA of 128 KB buffer in 5 sg entries failed\n");
			print_text("SHA Digest w/3 entries\n",
						(unsigned char *)&digest[0],
						QCEDEV_MAX_SHA_DIGEST);
			print_text("SHA Digest w/5 entries\n",
						(unsigned char *)&req.digest[0],
						QCEDEV_MAX_SHA_DIGEST);
		}
	}

	/*********************************************************************/
	memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req.data[0].vaddr = &test_128K[0];
	req.data[0].len = _32K * 4 ;
	req.data_len = _32K * 4;
	req.diglen = 0;
	req.entries = 1;
	qcedev_get_SHA(&req, fd, alg);

	if (memcmp(&digest[0], &req.digest[0], QCEDEV_MAX_SHA_DIGEST) == 0)
		pass = 1;
	else {
		pass = 0;
		if (verbose) {
			printf("SHA of 128 KB buffer (in one entry) failed\n");
			print_text("SHA Digest\n", &req.digest[0],
						QCEDEV_MAX_SHA_DIGEST);
		}
	}

	/*********************************************************************/
	req0.data[0].vaddr = &test_128K[0];
	req0.data[0].len = _16K;
	req0.data_len = _16K;
	req.diglen = 0;
	req0.entries = 1;
	req0.alg = alg;

	memset(&req0.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req0);

	for (i = 0; i < 8; i++) {
		req0.data[0].vaddr = &test_128K[_16K * i];
		req0.data[0].len = _16K;
		req0.data_len = _16K;
		req.diglen = 0;
		req0.entries = 1;
		req0.alg = alg;

		ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req0);
	}

	ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req0);

	if (memcmp(&digest[0], &req0.digest[0], QCEDEV_MAX_SHA_DIGEST) == 0)
		pass = 1;
	else {
		pass = 0;
		printf("SHA of 128 KB buffer failed (_16KB chunks)\n");
	}
	/*********************************************************************/

	if (pass == 1) {
		if (verbose || internal_test_enabled)
			printf("SHA Digest (128 KB) as expected: PASS!!!!\n");
	} else {
		if (verbose)
			printf("SHA Digest (128 KB ) comparison FAILED !!!!\n");
	}
	return pass;
}

static int qcedev_sha_adversarial_test(int fd)
{
	struct qcedev_sha_op_req  req;
	struct qcedev_sha_op_req  req0;
	int errors = 0;
	int ret;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf(" qcedev_sha_adverserial_test: fd = 0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}
	memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);
	req.data[0].vaddr =  &PlainText_90[0];
	req.data[0].len = 0x90;
	req.data_len = 0x00;
	req.diglen = 0;
	req.entries = 1;
	req.alg = QCEDEV_ALG_SHA1;

	/* Check for data-len set incorrectly to 0 */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req))
		errors++;

	req.data_len = 0x90;
	req.entries = 0;
	req.alg = QCEDEV_ALG_SHA1;

	/* Check for entries set incorrectly to 0 */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req))
		errors++;

	req.entries = 1;
	req.alg = 6;

	/* Check for alg set incorrectly to an invalid value */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req))
		errors++;

	req.alg = QCEDEV_ALG_SHA1;

	if (ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req))
		errors++;

	/**********************************************************/
	req.data_len = 0x00;

	/* Check for data-len set incorrectly to 0 */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req))
		errors++;

	req.data_len = 0x90;
	req.entries = 0;
	req.alg = QCEDEV_ALG_SHA1;

	/* Check for entries set incorrectly to 0 */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req))
		errors++;

	req.entries = 1;
	req.alg = 6;

	/* Check for alg set incorrectly to an invalid value */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req))
		errors++;

	req.alg = QCEDEV_ALG_SHA1;

	if (ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req))
		errors++;
	/***********************************************************/
	req.data_len = 0x00;

	/* Check for data-len set incorrectly to 0 */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req))
		errors++;

	req.data_len = 0x90;
	req.entries = 0;
	req.alg = QCEDEV_ALG_SHA1;

	/* Check for entries set incorrectly to 0 */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req))
		errors++;

	req.entries = 1;
	req.alg = 6;

	/* Check for alg set incorrectly to an invalid value */
	if (!ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req))
		errors++;

	req.alg = QCEDEV_ALG_SHA1;

	if (ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req))
		errors++;
	/***********************************************************/

	return errors;
}

static int qcedev_sha_test_suite(int sha_fd)
{
	int errors = 0;

	if ((verbose == 1) && (internal_test_enabled == 1)) {
		printf("\n\n");
		printf("===================================================\n");
		printf(" SHA TEST for sha_fd = 0x%x\n", sha_fd);
		printf("===================================================\n");
	}
	if (!qcedev_SHA1_SHA256_basic_test(sha_fd))
		errors++;
	if (!qcedev_SHA_test_32KB_plus(sha_fd, QCEDEV_ALG_SHA1))
		errors++;
	if (!qcedev_SHA_test_32KB_plus(sha_fd, QCEDEV_ALG_SHA256))
		errors++;
	if (!qcedev_SHA1_SHA256_multi_updates(sha_fd))
		errors++;
	if (!qcedev_SHA1_SHA256_multiple_of_64b(sha_fd))
		errors++;
	if (errors)
		printf("NUMBER OF FAILURES IN SHA TEST SUITE: 0x%x\n", errors);
	if (verbose || internal_test_enabled)
		printf("\n=============SHA TEST COMPLETE=============\n\n\n");
	return errors;
}

static int qcedev_cipher_adversarial_test(int fd)
{
	int errors = 0;
	struct qcedev_cipher_op_req  req;

	if (verbose == 1) {
		printf("\n\n");
		printf("---------------------------------------------------\n");
		printf(" qcedev_cipher_adversarial_test: fd = 0x%x\n", fd);
		printf("---------------------------------------------------\n");
	}

	memset(req.enckey, 0, 32);
	memset(req.iv, 0, 32);
	memcpy(req.enckey, key, key_sz);
	memcpy(req.iv, iv, 16);
	req.ivlen  = 16;

	req.byteoffset  = 0;
	req.data_len = 0x90;
	req.encklen  = key_sz;

	req.alg = QCEDEV_ALG_AES;
	req.mode = QCEDEV_AES_MODE_CTR;

	req.vbuf.src[0].vaddr = &PlainText_90[0];
	req.vbuf.src[0].len = 0x90;
	req.vbuf.dst[0].vaddr = &PlainText_90[0];
	req.vbuf.dst[0].len = 0x90;

	req.use_pmem = 0;
	req.in_place_op = 1;
	req.entries = 1;

	req.op = QCEDEV_OPER_ENC;

	/* Case 1: keylength is 0 but the key is a non zero array*/
	req.encklen  = 0;
	if (!ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req))
		errors++;
	req.encklen  = key_sz;

	/* Case 2: ivlen = 0  for ctr mode*/
	req.ivlen  = 0;
	if (!ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req))
		errors++;
	req.ivlen  = 16;

	/* Case 3: ivlen > 0 and mode == _ECB*/
	req.mode = QCEDEV_AES_MODE_ECB;
	if (!ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req))
		errors++;

	/* Case 4: byteoffset > 0 and mode ==  _ECB*/
	req.byteoffset  = 16;
	if (!ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req))
		errors++;
	req.mode = QCEDEV_AES_MODE_CTR;

	/* Case 5: byteoffset > 0 and use_pmem == 1*/
	req.byteoffset  = 16;
	req.use_pmem  = 1;
	if (!ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req))
		errors++;
	req.mode = QCEDEV_AES_MODE_CTR;
	req.byteoffset  = 0;

	/* Case 6:  use_pmem == 1 and  in_palce_op == 0*/
	req.in_place_op  = 0;
	if (!ioctl(fd, QCEDEV_IOCTL_ENC_REQ, &req))
		errors++;
	req.use_pmem  = 0;
	req.in_place_op  = 1;

	return errors;
}

static int qcedev_cipher_pmem_test(int cipher_fd,
					enum qcedev_cipher_alg_enum alg,
					enum qcedev_cipher_mode_enum  mode,
					uint32_t key_sz)
{
	int i = 0;
	int errors = 0;

	if (verbose) {
		printf("\n\n\n");
		printf("**************************************************\n");
		printf("       CIPHER TEST CASES WITH PMEM ALLOCATED MEMORY\n");
		printf("qcedev_cipher_pmem_test: alg = 0x%x, mode = 0x%x\n",
								alg, mode);
		printf("**************************************************\n");
	}

	memset(pmem_src, 0 , (_32K * 4));
	for (i = 0; i < (_32K * 4); i++)
		test_128K[i] = i % 255;

	memcpy((uint8_t *)pmem_src, &test_128K[0], _32K * 4);

	if (!(qcedev_cipher_pmem_single_sg_test(cipher_fd, alg, mode, pmem_fd,
					pmem_src,
					&PlainText_90[0], &PlainText_90[0],
					0x80, key, &iv[0], 0, key_sz)))
		errors++;
	if (!(qcedev_cipher_pmem_mult_sg_test(cipher_fd, alg, mode, pmem_fd,
					pmem_src, key, &iv[0], 0, key_sz)))
		errors++;
	if (!(qcedev_cipher_pmem_mult_sg_large_buf_test(cipher_fd, alg, mode,
					pmem_fd, pmem_src, key, &iv[0], 0,
					key_sz)))
			errors++;

	return errors;
}

static int qcedev_cipher_vbuf_test(int cipher_fd,
					enum qcedev_cipher_alg_enum alg,
					enum qcedev_cipher_mode_enum  mode,
					uint32_t key_sz)
{
	int errors = 0;
	if (verbose == 1) {
		printf("\n\n\n");
		printf("***************************************************\n");
		printf("       CIPHER TEST CASES WITH VBUF ALLOCATED MEMORY\n");
		printf("qcedev_cipher_vbuf_test: alg=0x%x, mode=0x%x\n",
								alg, mode);
		printf("***************************************************\n");
	}
	if (alg != QCEDEV_ALG_AES) {
		if (!(qcedev_cipher_vbuf_single_sg_test(cipher_fd, alg, mode,
				&PlainText_90[0], &PlainText_90[0], 0x10, key,
				iv, 0, 0, key_sz)))
			errors++;
	} else {
		if (mode == QCEDEV_AES_MODE_CTR) {
			/* test the byteoffset case */
			if (!(qcedev_cipher_vbuf_single_sg_test(cipher_fd, alg,
				mode, &PlainText_90[0], &PlainText_90[0],
				0x80, key, &iv[0], 0, 10, key_sz)))
				errors++;
		} else {
			if (!(qcedev_cipher_vbuf_single_sg_test(cipher_fd, alg,
				mode, &PlainText_90[0], &PlainText_90[0],
				0x80, key, &iv[0], 0, 0, key_sz)))
				errors++;
		}
	}
	if (!(qcedev_cipher_vbuf_mult_sg_test(cipher_fd, alg,
						mode, key, &iv[0], 0, key_sz)))
			errors++;
	if (!(qcedev_cipher_vbuf_mult_sg_large_buf_test(cipher_fd, alg,
						mode, key, &iv[0], 0, key_sz)))
			errors++;
	return errors;
}

static int qcedev_cipher_pmem_test_suite(int cipher_fd)
{
	int errors = 0;
	if (verbose == 1) {
		printf("\n\n");
		printf("==================================================\n");
		printf(" PMEM CIPHER TEST for  cipher_fd = 0x%x\n", cipher_fd);
		printf("==================================================\n");
	}

	if (verbose == 1)
		printf("\n AES-128 (USE PMEM)\n");

	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CBC, KEY_SIZE_16);
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_ECB, KEY_SIZE_16);
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CTR, KEY_SIZE_16);
	if (verbose == 1)
		printf("\n AES-256 (USE PMEM)\n");
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CBC, KEY_SIZE_32);
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_ECB, KEY_SIZE_32);
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CTR, KEY_SIZE_32);

	if (verbose == 1)
		printf("\n DES (USE PMEM)\n");
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_DES,
					QCEDEV_DES_MODE_CBC, KEY_SIZE_16);
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_DES,
					QCEDEV_DES_MODE_ECB, KEY_SIZE_16);

	if (verbose == 1)
		printf("\n 3DES (USE PMEM)\n");
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_3DES,
					QCEDEV_DES_MODE_CBC, KEY_SIZE_32);
	errors += qcedev_cipher_pmem_test(cipher_fd, QCEDEV_ALG_3DES,
					QCEDEV_DES_MODE_ECB, KEY_SIZE_32);

	if (errors)
		printf("NUMBER OF FAILURES IN PMEM CIPHER TEST SUITE: 0x%x\n",
								errors);
	if ((verbose == 1) || internal_test_enabled)
		printf("\n===========PMEM CIPHER TEST COMPLETE============\n");
	return errors;
}

static int qcedev_cipher_vbuf_test_suite(int cipher_fd)
{
	int errors = 0;
	if (verbose == 1) {
		printf("\n\n");
		printf("==================================================\n");
		printf(" CIPHER TEST for  cipher_fd = 0x%x\n", cipher_fd);
		printf("==================================================\n");
	}

	if (verbose == 1)
		printf("\n AES-128 (USE VBUF)\n");
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CBC, KEY_SIZE_16);
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_ECB, KEY_SIZE_16);
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CTR, KEY_SIZE_16);

	if (verbose == 1)
		printf("\n AES-256 (USE VBUF)\n");
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CBC, KEY_SIZE_32);
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_ECB, KEY_SIZE_32);
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_CTR, KEY_SIZE_32);

#ifdef CRYPTO_AES_XTS_SUPPORT
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_XTS, 2*KEY_SIZE_32);
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_AES,
					QCEDEV_AES_MODE_XTS, KEY_SIZE_32);
#endif

	if (verbose == 1)
		printf("\n DES (USE VBUF)\n");
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_DES,
					QCEDEV_DES_MODE_CBC, KEY_SIZE_16);
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_DES,
					QCEDEV_DES_MODE_ECB, KEY_SIZE_16);

	if (verbose == 1)
		printf("\n 3DES (USE VBUF)\n");
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_3DES,
					QCEDEV_DES_MODE_CBC, KEY_SIZE_32);
	errors += qcedev_cipher_vbuf_test(cipher_fd, QCEDEV_ALG_3DES,
					QCEDEV_DES_MODE_ECB, KEY_SIZE_32);

	if (errors)
		printf("NUMBER OF FAILURES IN VBUF CIPHER TEST SUITE: 0x%x\n",
								errors);
	if ((verbose == 1) || internal_test_enabled)
		printf("\n==============CIPHER TEST COMPLETE==============\n");
	return errors;
}

static int qcedev_cipher_test_suite(int cipher_fd)
{
	int errors = 0;
	if (verbose == 1) {
		printf("\n\n");
		printf("==================================================\n");
		printf(" CIPHER TEST for  cipher_fd = 0x%x\n", cipher_fd);
		printf("==================================================\n");
	}

	if (pmem_enabled)
		errors += qcedev_cipher_pmem_test_suite(qcedev_fd);

	errors += qcedev_cipher_vbuf_test_suite(cipher_fd);

	if (errors)
		printf("NUMBER OF FAILURES IN CIPHER TEST SUITE: 0x%x\n",
								errors);
	if (verbose || internal_test_enabled)
		printf("\n==============CIPHER TEST COMPLETE==============\n");
	return errors;
}

static int qcedev_test_suite(int qcedev_fd)
{
	int errors = 0;

	if (verbose == 1) {
		printf("\n\n\n\n\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                                 X\n");
		printf("X      QCEDEV TEST SUITE: qcedev_fd = 0x%x        X\n",
								qcedev_fd);
		printf("X                                                 X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	}
	errors = qcedev_sha_test_suite(qcedev_fd);

	errors += qcedev_cipher_test_suite(qcedev_fd);

	if (verbose == 1) {
		if (errors) {
			printf("\n\n");
			printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
			printf("X                                         X\n");
			printf("X	    QCEDEV TEST SUITE FAILED      X\n");
			printf("X                                         X\n");
			printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
			printf("NUMBER OF FAILURES IN TEST SUITE: 0x%x\n",
				errors);
		} else {
			printf("\n\n");
			printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
			printf("X                                         X\n");
			printf("X QCEDEV TEST SUITE PASSED ALL TEST CASES X\n");
			printf("X                                         X\n");
			printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		}
	}

	return 1;
}

static int nominal(void)
{
	int errors = 0;
	errors = qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA1);
	errors += qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA256);
	errors += qcedev_cipher_basic_test(qcedev_fd);
#ifdef CRYPTO_HMAC_SUPPORT
	errors += qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA1_HMAC);
	errors += qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA256_HMAC);
#endif
	errors += qcedev_CMAC_basic_test(qcedev_fd);
	return errors;
}

static int nominal_test(void)
{
	int errors;

	printf("\n\nRunning Nominal Test......\n");

	errors = nominal();

	printf("\n\n");
	if (errors) {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X    QCEDEV FAILED NOMINAL TEST CASES     X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("Number of failures in %s test: 0x%x\n",
					testopts[NOMINAL].name, errors);
	} else {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X   QCEDEV PASSED NOMINAL TEST CASE(S)	  X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	}
	return errors;
}

static int adverse(void)
{
	int sha_errors = 0;
	int cipher_errors = 0;

	sha_errors = qcedev_sha_adversarial_test(qcedev_fd);
	if ((sha_errors) && (verbose == 1))
		printf("SHA ADVERSARIAL TESTS ERRORS = 0x%x\n",
					(uint32_t)sha_errors);

	cipher_errors += qcedev_cipher_adversarial_test(qcedev_fd);
	if ((cipher_errors) && (verbose == 1))
		printf("CIPHER ADVERSARIAL TESTS ERRORS = 0x%x\n",
						(uint32_t)cipher_errors);

	return cipher_errors + sha_errors;
}

static int adversarial_test(void)
{
	int errors;

	printf("\n\nRunning Adversarial Test......\n");

	errors = adverse();
	printf("\n\n");
	if (errors) {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X   QCEDEV FAILED ADVERSE TEST CASE(S)	  X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("Number of failures in %s test: 0x%x\n",
				testopts[ADVERSARIAL].name, errors);
	} else {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X   QCEDEV PASSED ADVERSE TEST CASE(S)	  X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	}
	return errors;
}

static int repeat_test(void)
{
	int errors = 0;
	int i;

	printf("\n\nRunning Repeatabilty Test......\n");
	for (i = 0; i < 100; i++) {
		errors += nominal();
		errors += adverse();
	}
	printf("\n\n");
	if (errors) {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X  QCEDEV FAILED REPEAT TEST CASE(S)	  X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("Number of failures in %s test: 0x%x\n",
					testopts[REPEAT].name, errors);
	} else {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X    QCEDEV PASSED REPEAT TEST CASE(S)    X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	}
	return errors;
}

static int qcedev_sha1_pthread_test(int fd)
{
	struct qcedev_sha_op_req  *req;
	int errors = 0;
	unsigned int i, num_tv;
	if (verbose == 1) {
		printf("\n\n");
		printf("----------------------------------------------\n");
		printf(" sha1_basic_test: fd = 0x%x\n", fd);
		printf("----------------------------------------------\n");
	}
	req = malloc(sizeof(struct qcedev_sha_op_req));
	if (!req) {
		printf("Failed to allocate memory for req buffer\n");
		return -1;
	}

	num_tv = (sizeof(sha1_tv))/(sizeof(struct test_vector));
	for (i = 0; i < num_tv; i++) {

		req->alg = QCEDEV_ALG_SHA1;
		req->data[0].vaddr = (uint8_t *)sha1_tv[i].input;
		req->data[0].len = sha1_tv[i].ilen;
		req->data_len = sha1_tv[i].ilen;
		req->entries = 1;

		memset(&req->digest[0], 0, QCEDEV_MAX_SHA_DIGEST);

		ioctl(fd, QCEDEV_IOCTL_GET_SHA_REQ, req);
		if (!(memcmp(&req->digest[0], sha1_tv[i].digest,
					sha1_tv[i].diglen) == 0)) {
			errors += 1;
			if (verbose) {
				printf("SHA1 FAILED\n");
				print_text("SHA1 input data",
						(unsigned char *)
						sha1_tv[i].input,
						sha1_tv[i].ilen);
				print_text("Calculated SHA1 Digest",
						(unsigned char *)
						&req->digest[0],
						sha1_tv[i].diglen);
				print_text("Expected SHA1 Digest",
						(unsigned char *)
						sha1_tv[i].digest,
						sha1_tv[i].diglen);
			}
		}
	}

	if (verbose) {
		if (errors  == 0)
			printf("SHA1 Basic Test: PASS !!!\n");
		else
			printf("SHA1 BASIC TEST: FAILED %d TESTS"
				"OUT OF %d\n", errors, num_tv);
	}
	free(req);
	return errors;
}

static int basic_thread_test(int fd, int thread, uint32_t *iteration)
{
	int errors = 0;
	int i;

	for (i = 0; i < 500; i++)
		if (thread % 2)
			errors += qcedev_sha1_pthread_test(fd);
		else
			errors += qcedev_cipher_basic_test(fd);

	if (errors) {
		printf("\nNumber of failures in thread %d stress test: 0x%x\n",
					thread, errors);
	} else {
		printf("	Thread %d stress test: PASSED\n", thread);
	}
	*iteration = i;
	return errors;
}

static void *test_thread(void *arg)
{
	uint32_t thread_num = *(uint32_t *)arg;
	int fd;
	uint32_t iteration, errors = 0;

	printf("    Thread %d starting...\n", thread_num);
	fd = open("/dev/qce", O_RDWR | O_SYNC);
	if (fd < 0) {
		printf("Fail to Open QCEDEV Device fd (%d)\n", fd);
		pthread_exit((char *)1);
	} else {
		if (verbose == 1)
			printf("     Thread %d fd %d\n", thread_num, fd);
	}

	errors =  basic_thread_test(fd, thread_num, &iteration);
	if (errors)
		pthread_exit(&errors);

	close(fd);

	if (errors)
		printf("Thread %d failed at iteration %d\n",
				thread_num, iteration);

	pthread_exit(&errors);

	return NULL;
}

static int stress_test_thread(char *name)
{
	int i, rc, thread_errs, errors = 0;
	static pthread_t threads[8];

	if (verbose == 1) {
		printf("\n\n");
		printf("----------------------------------------------\n");
		printf(" Running Multi-Thread Stress Test...\n");
		printf("----------------------------------------------\n");
	}
	pthread_mutex_init(&plock, 0);

	for (i = 0; i < 8; i++) {
		rc = pthread_create(&threads[i], NULL, test_thread,
							(void *) &i);
		if (rc) {
			printf("Couldn't create thread %d\n", i + 1);
			return 1;
		}
	}

	for (i = 0; i < 8; i++) {
		rc = pthread_join(threads[i], (void **)&thread_errs);
		if (rc) {
			printf("Couldn't join thread %d\n", i + 1);
			return 1;
		}
		errors += thread_errs;
	}

	return errors;
}

static int stress_test(void)
{
	int errors = 0;
	int i;

	printf("\n\nRunning Stress Test......\n");

	printf("  Running Single Thread Stress Test...\n");
	for (i = 1; i < 501; i++) {
		errors += qcedev_sha_test_suite(qcedev_fd);
		if (pmem_enabled)
			errors += qcedev_cipher_pmem_test_suite(qcedev_fd);
		errors += qcedev_cipher_vbuf_test_suite(qcedev_fd);
		if (!(i % 50))
			printf("\n  *   Completed %d runs...\n", i);
	}
	printf("\n\n");
	if (errors) {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X  QCEDEV FAILED STRESS TEST		  X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("Number of failures in %s test: 0x%x\n",
					testopts[STRESS].name, errors);
	} else {
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		printf("X                                         X\n");
		printf("X    QCEDEV PASSED STRESS TEST            X\n");
		printf("X                                         X\n");
		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	}
	return errors;
}

static int performance_test(void)
{
	int i;

	printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	printf("X                                         X\n");
	printf("X  QCEDEV PERFORMANCE TEST RESULTS	  X\n");
	printf("X                                         X\n");
	printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	fp = fopen("test.txt", "w+");
	printf("\n\nRunning Performance Test......\n");
	printf("\nCipher Performance Test");
	printf("\n\talg= AES, mode = CBC, key size = 128bit\n");
	/* Performance test is run with the following parameters
	 * packet sizes = 8K, 16K, 32K, 64K
	 * alg = aes
	 * mode = cbc
	 * key size = 128 bit
	*/
	for (i = 8; i < MAX_PACKET_DATA_KB; i *= 2)
		qcedev_cipher_vbuf_speedtest(qcedev_fd, 1024 * i,
						QCEDEV_ALG_AES,
						QCEDEV_AES_MODE_CBC,
						16);

	/* Performance test is run with the following parameters
	 * packet sizes = 8K, 16K, 32K, 64K.
	 * alg = SHA256.
	 * iuf = false.
	*/
	printf("\nSHA Performance Test");
	printf("\n\talg= SHA256, DIGEST\n");
	for (i = 8; i < MAX_PACKET_DATA_KB; i *= 2)
		qcedev_sha_vbuf_speedtest(qcedev_fd, 1024 * i,
						QCEDEV_ALG_SHA256, 0);
	fclose(fp);
	return 0;
}

void internal_test_menu(void)
{
	printf("\n\n");
	printf("The currently implemented tests are:\n");
	printf("\n");
	printf("    Input    1	TEST CASE\n");
	printf("   -----------	----------\n");
	printf("    0 ------->  Dispaly Menu\n");
	printf("    1 ------->  SHA1/SHA256 Test Suite\n");
	printf("    2 ------->  CIPHER Test Suite\n");
	printf("    3 ------->  Basic SHA1 Test\n");
	printf("    4 ------->  Basic SHA256 Test\n");
	printf("    5 ------->  Basic CMAC Test\n");
	printf("    6 ------->  Basic Cipher AES test\n");
	printf("    7 ------->  Basic Cipher DES test\n");
	printf("    8 ------->  SHA1/SHA256 block size multiple pkt\n");
	printf("    9 ------->  SHA1/SHA256 32 KB Plus data\n");
	printf("   10 ------->  CIPHER (PMEM) Test Suite\n");
	printf("   11 ------->  CIPHER (VBUF) Test Suite\n");
	printf("   12 ------->  Cipher using VBUF (PERFORMANCE)\n");
	printf("   13 ------->  Cipher using PMEM (PERFORMANCE)\n");
	printf("   14 ------->  SHA using VBUF (PERFORMANCE)\n");
	printf("   15 ------->  Multi Thread Stress Test\n");
	printf("   16 ------->  hmac-sha1 basic test\n");
	printf("   17 ------->  hmac-sha256 basic test\n");

	printf("\n  Input   2	CIPHER/SHA ALGORITHM\n");
	printf("   -----------	-----------------\n");
	printf("    0 ------->  QCEDEV_ALG_DES/SHA1\n");
	printf("    1 ------->  QCEDEV_ALG_3DES/SHA256\n");
	printf("    2 ------->  QCEDEV_ALG_AES\n\n");

	printf("\n  Input   3	AES/DES MODE\n");
	printf("   -----------	-----------------\n");
	printf("    0 ------->  QCEDEV_AES_MODE_CBC (AES/DES/DES3)\n");
	printf("    1 ------->  QCEDEV_AES_MODE_ECB (AES/DES/DES3)\n");
	printf("    2 ------->  QCEDEV_AES_MODE_CTR\n");
	printf("    3 ------->  QCEDEV_AES_MODE_XTS\n");

	printf("\n  Input   4	KEY SIZE 16/32\n");
	printf("   -----------	-----------------\n");
	printf("    16 ------->   128 bit\n");
	printf("    32 ------->   256 bit\n");
	printf("    64 ------->   512 bit\n");

	printf("\n  Input   5	SHA1/SHA256 MODE\n");
	printf("   -----------	-----------------\n");
	printf("    0 ------->   using SHA Digest\n");
	printf("    1 ------->   using init/update/final\n");


	printf("    Test Cases  Input1 Input2 Input3 Input4 Input5\n");
	printf("    ----------  ------ ------ ------ ------ -------\n");
	printf("     0 - 8        x      -       -      -	-\n");
	printf("       9          x      x       -      -	-\n");
	printf("     10 - 13      x      x       x      x	-\n");
	printf("      14          x      x       -      x	x\n");
	printf("      15          x      -       -      -	-\n");
}

int internal_test(int argc, char **argv)
{
	int test_number;
	int i = 0;
	int iuf = 0;
	enum qcedev_cipher_alg_enum alg = 0;
	enum qcedev_cipher_alg_enum sha_alg = 0;
	enum qcedev_cipher_mode_enum mode = 0;


	if (!argv[0]) {
		internal_test_menu();
		return 0;
	}
	fp = fopen("test.txt", "w+");
	if (!fp) {
		printf("Could not open test.txt file\n");
		return -errno;
	}

	test_number = atoi(argv[0]);

	/*All internal tests > 9 and < 14 need algo and mode options*/
	if (test_number == 9) {
		alg = atoi(argv[1]);
	} else	if ((test_number > 9) && (test_number < 13)) {
		alg = atoi(argv[1]);
		mode = atoi(argv[2]);
		key_sz = atoi(argv[3]);
		if (!((key_sz == KEY_SIZE_16) || (key_sz == KEY_SIZE_32) ||
			(key_sz == KEY_SIZE_64))) {
				printf("Wrong key size\n");
				exit(-1);
		}
	} else if (test_number == 14) {
		sha_alg = atoi(argv[1]);
		iuf = atoi(argv[2]);
	}

	if (verbose)
		printf("Running test %d\n", test_number);

	switch (test_number) {
	case 0:
		internal_test_menu();
		break;

	case 1:
		qcedev_sha_test_suite(qcedev_fd);
		break;

	case 2:
		qcedev_cipher_test_suite(qcedev_fd);
		break;

	case 3:
		qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA1);
		break;

	case 4:
		qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA256);
		break;
	case 5:
		qcedev_CMAC_basic_test(qcedev_fd);
		break;
	case 6:
		qcedev_cipher_aes_basic_test(qcedev_fd);
		break;

	case 7:
		qcedev_cipher_des_basic_test(qcedev_fd);
		break;

	case 8:
		qcedev_SHA1_SHA256_multiple_of_64b(qcedev_fd);
		break;

	case 9:
		qcedev_SHA_test_32KB_plus(qcedev_fd, alg);
		break;

	case 10:
		if (pmem_enabled)
			qcedev_cipher_pmem_test(qcedev_fd, alg, mode, key_sz);
		else
			printf("PMEM is not enabled on this device\n");
		break;

	case 11:
		qcedev_cipher_vbuf_test(qcedev_fd, alg, mode, key_sz);
		break;

	case 12:
		for (i = 1; i < MAX_PACKET_DATA_KB; i *= 2)
			qcedev_cipher_vbuf_speedtest(qcedev_fd, 1024 * i, alg,
							mode, key_sz);
		break;

	case 13:
		if (pmem_enabled) {
			for (i = 1; i < MAX_PACKET_DATA_KB; i *= 2)
				qcedev_cipher_pmem_speedtest(qcedev_fd,
								1024 * i,
								alg, mode,
								key_sz);
		} else
			printf("PMEM is not enabled on this device\n");
		break;

	case 14:
		for (i = 1; i < MAX_PACKET_DATA_KB; i *= 2)
			qcedev_sha_vbuf_speedtest(qcedev_fd, 1024 * i, sha_alg,
							iuf);
		break;

	case 15:
		stress_test_thread("MultiThread");
		break;

	case 16:
#ifdef CRYPTO_HMAC_SUPPORT
		qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA1_HMAC);
#else
		printf("HMAC not supported on this target\n");
#endif
		break;

	case 17:
#ifdef CRYPTO_HMAC_SUPPORT
		qcedev_sha_basic_test(qcedev_fd, QCEDEV_ALG_SHA256_HMAC);
#else
		printf("HMAC not supported on this target\n");
#endif
		break;

	default:
		internal_test_menu();
		break;
	}

	close(qcedev_fd);
	fclose(fp);
	exit(0);
}

static int (*test_func[]) (void) = {
	[NOMINAL] = nominal_test,
	[ADVERSARIAL] = adversarial_test,
	[STRESS] = stress_test,
	[REPEAT] = repeat_test,
	[PERFORMANCE] = performance_test,
};

static void usage(int ret)
{
	printf("\n\n---------------------------------------------------------\n"
		"Usage: qcedev_test -[OPTION] -[TEST_TYPE0]..-[TEST_TYPEn]\n\n"
		"Runs the user space tests specified by the TEST_TYPE\n"
		"parameter(s).\n"
		"\n\n"
		"OPTION can be:\n"
		"  -v, --verbose         run with debug messages on\n\n"
		"TEST_TYPE can be:\n"
		"  -n, --nominal         run standard functionality tests\n"
		"  -a, --adversarial     run tests that try to break the\n"
		"                          driver\n"
		"  -r, --repeatability   run 200 iterations of both the\n"
		"                          nominal and adversarial tests\n"
		"  -s, --stress          run tests that try to maximize the\n"
		"                          capacity of the driver\n"
		"  -p, --performance	 run cipher and sha256 performance\n"
		"			   tests\n"
		"  -h, --help            print this help message and exit\n\n\n"
		" - 'adb push out/target/product/<target>/obj/KERNEL_OBJ/drivers"
		"/crypto/msm/qce.ko /data/kernel-tests/'\n"
		" - 'adb push out/target/product/<target>/obj/KERNEL_OBJ/drivers"
		"/crypto/msm/qcedev.ko /data/kernel-tests/'\n\n"
		" - Connect to device: From command shell, do 'adb shell'\n"
		" - Once in the shell: do 'cd /data/kernel-tests'\n"
		" - Load QCE module: do 'insmod qce.ko (or qce40.ko)'\n"
		" - Load QCEDEV module: do 'insmod qcedev.ko'\n"
		" - Change permission for qcedev_test: "
		"   do 'chmod 777 qcedev_test'\n"
		" - Run qcedev_test:\n"
		"      do './qcedev_test -<OPTION> -<TEST_TYPE0> -<TEST_TYPE1>"
		" ......-<TEST_TYPEn>'\n"
		"         [see OPTION & TEST_TYPE described above]\n"
		"---------------------------------------------------------\n");

	exit(ret);
}

static unsigned int parse_command(int argc, char *const argv[])
{
	int command = 0;
	unsigned int ret = 0;

	verbose = 0;
	bringup = 0;

	while ((command = getopt_long(argc, argv, "bvnasrpih", testopts,
				      NULL)) != -1) {
		switch (command) {
		case 'b':
			bringup = 1;
			verbose = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'n':
			ret |= 1 << NOMINAL;
			break;
		case 'a':
			ret |= 1 << ADVERSARIAL;
			break;
		case 'r':
			ret |= 1 << REPEAT;
			break;
		case 's':
			ret |= 1 << STRESS;
			break;
		case 'p':
			ret |= 1 << PERFORMANCE;
			break;
		case 'i':
			ret = 1 << INTERNAL;
			break;
		case 'h':
			usage(0);
		default:
			usage(-1);
		}
	}
	/* When only verbose option is given*/
	if (ret == 0)
		usage(-1);
	return ret;
}

int main(int argc, char **argv)
{
	int rc = 0, num_tests_failed = 0, i;
	internal_test_enabled = 0;
	int error = -1;
	unsigned int test_mask = parse_command(argc, argv);

	qcedev_fd = open("/dev/qce", O_RDWR | O_SYNC);

	if (qcedev_fd < 0) {
		printf("Fail to Open QCEDEV Device (dev/qce) qcedev_fd (%d)\n",
								qcedev_fd);
		printf("Test failed\n");
		return error;
	}
	pmem_fd = open("/dev/pmem_adsp", O_RDWR | O_SYNC);
	if (pmem_fd < 0) {
		printf("cannot open pmem device for pmem_fd\n");
		pmem_enabled = 0;
	} else {
		pmem_src = mmap(NULL, _128K, PROT_READ | PROT_WRITE,
						MAP_SHARED, pmem_fd, 0);

		if (pmem_src == MAP_FAILED) {
			printf("pmem mmap for src failed");
			close(pmem_fd);
			pmem_fd = -1;
			printf("PMEM test cases will not be run\n");
			pmem_enabled = 0;
			/* Increase the test fail count by 1 */
			num_tests_failed++;
		} else {
			memset(pmem_src, 0 , (_128K));
			pmem_enabled = 1;
		}
	}

	if (verbose)
		printf("QCEDEV Device Opened SUCCESSFUL qcedev_fd (%d)\n",
								qcedev_fd);

	/* Condition to check if it is a internal test or ABAIT test */
	if ((test_mask & (1 << INTERNAL)) == (1U << INTERNAL)) {
		internal_test(argc-2, &argv[2]);
	} else {
		for (i = 0; i < LAST_TEST; i++) {
			/* Look for the test that was selected */
			if (!(test_mask & (1U << i)))
				continue;

			/* This test was selected, so run it */
			rc = test_func[i]();

			if (rc) {
				printf("%s test case FAILED! rc:%d\n",
					testopts[i].name, rc);
				num_tests_failed++;
			}
		}
	}

	if (num_tests_failed)
		printf("NUMBER OF TESTS FAILED: %d\n", num_tests_failed);

	if (pmem_enabled) {
		close(pmem_fd);
		pmem_src = NULL;
	}
	close(qcedev_fd);

	return -num_tests_failed;
}
