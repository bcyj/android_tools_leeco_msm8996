/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <media/msm_gemini.h>
#include "gemini_lib.h"
#include "gemini_app.h"

struct test_gemini_param test_param = {
	0, // int yuv_w;
	0, // int yuv_h;
	"gemini_input.yuv", // char *input_yuv_filename;
	"gemini_output.bs", // char *output_bs_filename;
	NULL, // char *output_jpeg_filename;

	0, // int input_byte_ordering;
	0, // int output_byte_ordering;
	0, // int cbcr_ordering;
	0, // int zero_y;
	0, // int zero_cbcr;
	100, // int output_quality;

	0, // gmn_obj_t gmn_obj;
	-1, // int gmnfd;
};

void *gemini_app (void *context);

/* below is test control */
int test_console (int argc, char *argv[], gmn_obj_t gmn_obj, int gmnfd)
{
	int test_loop_exit = 0;
	int result;
	int n = 0;
	char tc_buf[4];
	struct msm_gemini_ctrl_cmd gmnCtrlCmd;
	struct msm_gemini_hw_cmd *hw_cmd_p;
	struct msm_gemini_hw_cmd hw_cmds[] = {
		/*
		 * type, n (max 0xFFF/4095), offset, mask, data/pdata
		 */
		{MSM_GEMINI_HW_CMD_TYPE_READ, 1, 0x4, 0xFF00FF00, {0x0}},	/* 30 */
		{MSM_GEMINI_HW_CMD_TYPE_READ, 1, 0x4, 0xFFFFFFFF, {0x0}},	/* 31 */
		{MSM_GEMINI_HW_CMD_TYPE_READ, 4, 0x14C, 0xFF00FF00, {0x0}},	/* 32 */
		{MSM_GEMINI_HW_CMD_TYPE_READ, 4, 0x14C, 0xFFFFFFFF, {0x0}},	/* 33 */
		{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, 0x4, 0xFF00FF00, {0xFFFFFFFF}},	/* 34 */
		{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, 0x4, 0xFFFFFFFF, {0xFFFFFFFF}},	/* 35 */
		{MSM_GEMINI_HW_CMD_TYPE_WRITE, 4, 0x14C, 0xFF00FF00, {0xFFFFFFFF}},	/* 36 */
		{MSM_GEMINI_HW_CMD_TYPE_WRITE, 4, 0x14C, 0xFFFFFFFF, {0xFFFFFFFF}},	/* 37 */
		{MSM_GEMINI_HW_CMD_TYPE_UWAIT, 4095, 0x4, 0xFF00FF00, {0x0}},	/* 38 */
		{MSM_GEMINI_HW_CMD_TYPE_UWAIT, 4095, 0x4, 0xFFFFFFFF, {0x0}},	/* 39 */
		{MSM_GEMINI_HW_CMD_TYPE_MWAIT, 4095, 0x14C, 0xFF00FF00, {0xFFFFFFFF}},	/* 40 */
		{MSM_GEMINI_HW_CMD_TYPE_MWAIT, 4095, 0x14C, 0xFFFFFFFF, {0xFFFFFFFF}},	/* 41 */
		{MSM_GEMINI_HW_CMD_TYPE_UDELAY, 4095, 0x0, 0x0, {0x0}},	/* 42 */
		{MSM_GEMINI_HW_CMD_TYPE_MDELAY, 4095, 0x0, 0x0, {0x0}},	/* 43 */
		{MSM_GEMINI_HW_CMD_TYPE_WRITE, 1, 0x24, 0xFFFFFFFF, {0x1}},	/* 37 */
		{MSM_GEMINI_HW_CMD_TYPE_READ, 1, 0x28, 0xFFFFFFFF, {0x0}},	/* 37 */
	};

	do {
		printf ("	0-15 MSM_GMN_IOCTL_HW_CMD\n");
		printf ("	90 - MSM_GMN_IOCTL_EVT_GET_UNBLOCK\n");
		printf ("	91 - MSM_GMN_IOCTL_INPUT_GET_UNBLOCK\n");
		printf ("	92 - MSM_GMN_IOCTL_OUTPUT_GET_UNBLOCK\n");
		printf ("	93 - MSM_GMN_IOCTL_RESET\n");
		printf ("	94 - MSM_GMN_IOCTL_TEST_DUMP_REGION\n");
		printf ("	99 - TEST EXIT\n");
		printf ("\nPlease type a command:\n");

		do {
			fgets (tc_buf, 4, stdin);
			n = atoi (tc_buf);
			if (strlen (tc_buf) > 1)
				break;
			else
				printf ("\nPlease input a command:\n");
		} while (1);

		printf ("Your command is %d\n", n);
		switch (n) {
		case 90:
			result = ioctl (gmnfd,
					MSM_GMN_IOCTL_EVT_GET_UNBLOCK);
			printf ("ioctl MSM_GMN_IOCTL_EVT_GET_UNBLOCK: %d\n", result);
			break;

		case 91:
			result = ioctl (gmnfd, MSM_GMN_IOCTL_INPUT_GET_UNBLOCK);
			printf ("ioctl MSM_GMN_IOCTL_INPUT_GET_UNBLOCK: %d\n",
				result);
			break;

		case 92:
			result = ioctl (gmnfd,
					MSM_GMN_IOCTL_OUTPUT_GET_UNBLOCK);
			printf ("ioctl MSM_GMN_IOCTL_OUTPUT_GET_UNBLOCK: %d\n",
				result);
			break;

		case 93:
			result = ioctl (gmnfd, MSM_GMN_IOCTL_RESET,
					&gmnCtrlCmd);
			printf ("ioctl rc = %d\n", result);
			break;

		case 94:
			result = ioctl (gmnfd, MSM_GMN_IOCTL_TEST_DUMP_REGION,
					(0x0150));
			printf ("ioctl MSM_GMN_IOCTL_TEST_DUMP_REGION: %d\n",
				result);
			break;

		case 99:
			test_loop_exit = 1;
			printf ("test loop exit %d\n", test_loop_exit);
			break;

		default:
			hw_cmd_p = &hw_cmds[n];
            if((n < 0) || (n > 15))
              break;
			result = ioctl (gmnfd, MSM_GMN_IOCTL_HW_CMD, hw_cmd_p);
			printf ("type:%d, n:%d, offset:0x%08x, mask:0x%08x, data/pdata:0x%08x\n",
				hw_cmd_p->type, hw_cmd_p->n, hw_cmd_p->offset, hw_cmd_p->mask, hw_cmd_p->data);
			printf ("ioctl MSM_GMN_IOCTL_HW_CMD: %d\n", result);
			break;
		}
	} while (!test_loop_exit);
	return 0;
}

int main (int argc, char *argv[])
{
	int c;
	pthread_t app_thread_id;

	while ((c = getopt (argc, argv, "czZq:B:b:o:i:w:h:j:?")) != -1) {
		switch (c) {
		case 'j':
			test_param.output_jpeg_filename = optarg;
			break;

		case 'z':
			test_param.zero_y = 1;
			break;

		case 'Z':
			test_param.zero_cbcr = 1;
			break;

		case 'c':
			test_param.cbcr_ordering = 1;
			break;

		case 'q':
			test_param.output_quality = atoi (optarg);
			break;

		case 'B':
			test_param.output_byte_ordering = atoi (optarg);
			break;

		case 'b':
			test_param.input_byte_ordering = atoi (optarg);
			break;

		case 'i':
			test_param.input_yuv_filename = optarg;
			break;

		case 'o':
			test_param.output_bs_filename = optarg;
			break;

		case 'w':
			test_param.yuv_w = atoi (optarg);
			break;

		case 'h':
			test_param.yuv_h = atoi (optarg);
			break;

		case '?':
		default:
			printf ("\nusage: %s [-hczZ] [-i <yuv input>] [-o <bit stream output>]\n", argv[0]);
			printf ("          [-b <0-7>] [-B <0-7>]\n");
			printf ("          [-j <jpeg file name>]\n");
			printf ("          [-q <0 - 100>]\n");
			printf ("	-j: output jpeg file name, it has to be existing with jpeg header\n");
			printf ("	-z: zero y data\n");
			printf ("	-Z: zero cbcr data\n");
			printf ("	-c: input cbcr ordering, default value 0\n");
			printf ("	-q <0-100>: quality control, default value 100\n");
			printf ("	-b <0-7>: input byte ordering\n");
			printf ("	-B <0-7>: output byte ordering\n");
			printf ("	-i <filename>: yuv input data file name\n");
			printf ("	-o <filename>: jpeg bit stream output data file name\n");
			printf ("	-w <width>: width of yuv input data file\n");
			printf ("	-h <height>: height of yuv input data file\n");
			printf ("	-?: print this help\n");
			exit (0);
		}
	}

	pthread_create (&app_thread_id, NULL, gemini_app, NULL);

	while (test_param.gmnfd < 0 || !test_param.gmn_obj) {
		printf ("Waiting for application ready...\n");
		sleep (1);
	}

	test_console (argc, argv, test_param.gmn_obj, test_param.gmnfd);

	printf ("Waiting for application exit...\n");
	if (pthread_join (app_thread_id, NULL) != 0) {
		printf ("%s: failed %d\n", __func__, __LINE__);
	}
	return 0;
}
