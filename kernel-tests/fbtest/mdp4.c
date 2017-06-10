/******************************************************************************
@file  mdp4.c
@brief This file contains test code to verify all functionalities of msm_fb

DESCRIPTION
fbtest is msm framebuffer test program.  It opens all frambuffers (/dev/fb*)
and executes the msm specific fb ioctls as well as the standard linux fb
ioctls.

-----------------------------------------------------------------------------
Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------------

******************************************************************************/

#include <stdlib.h>

#include "fbtest.h"
#include "fbtestUtils.h"
#include "mdp4.h"
#ifdef ENABLE_POSTPROC
#include "postproctest.h"
#endif

extern int fbtest_msg_debug;	/* set non-zero for verbose debugging messages */
int THREAD_DONE_GLOBAL = 1;
unsigned char *buff;
void interactiveDelay(void);
int is_delay_required_fb1 = TRUE;
tests MDP4testFunctions[] = {
	allMDP4Test,
	formatTest,
	scaleTest,
	rotateTest,
	cropTest,
	moveTest,
	deInterlaceTest,
	ditherTest,
	videoPlayTest,
	fpsVsyncTest,
	blendTest,
	colorkeyTest,
	multiopTest,
	MDP4adversarialTest,
	hwCursorTest,
	overlayScaleTest,
	overlayAlphaTest,
	overlayStressTest,
	allcolorFormatOverlayTest,
	overlayARGBTest,
	overlayTest,
	CSCTest,
#ifdef ENABLE_POSTPROC
	postprocTest,
	overlayPPTest,
#endif
};

int getRotatorDstFormat(unsigned int format, unsigned int cmdRotDeg);

int MDP4TestFuncArraySize = (sizeof(MDP4testFunctions));

struct lookup_string_t MDP4testNameLUT[] = {
	{ "allmdp4",	0, 0 },
	{ "format",	1, 1 },
	{ "scale",	2, 2 },
	{ "rotate",	3, 3 },
	{ "crop",	4, 4 },
	{ "move",	5, 5 },
	{ "deinterlace",6, 6 },
	{ "dither",	7, 7 },
	{ "videoplay",	8, 8 },
	{ "fpsvsync",	9, 9 },
	{ "blend",	10, 10 },
	{ "colorkey",	11, 11 },
	{ "multiop",	12, 12 },
	{ "adversarial",13, 13 },
	{ "hwcursor",   14,14},
	{ "overlayscale",15, 15 },
	{ "overlayalpha",16, 16 },
	{ "overlayStress",17,17},
	{ "allcolorFormatOverlay",18,18},
	{ "overlayARGB",19,19},
	{ "overlay",20,20},
	{ "csc",21,21},
#ifdef ENABLE_POSTPROC
	{ "postproc",22,22},
	{ "overlayPP",23,23},
#endif
	{ NULL,	-1, -1},
};

int MDP4TestNameArraySize = (sizeof(MDP4testNameLUT));

int getMaxUpScale(int  mdp_version) {
	int scale = 1;
	if (mdp_version >= MDP_V4_1)
		scale = 20;
	else
		scale = 8;
	return scale;
}

int getMaxDownScale(int  mdp_version) {
	int scale = 1;
	if (mdp_version < MDP_V5)
		scale = 8;
	else
		scale = 4;
	return scale;
}

static int overlay_vsync_ctrl(int enable) {
	int ret = 0;
	int vsync_en = enable;
	if (writeback_flag == FALSE) {
		ret = ioctl(FB->fb_fd, MSMFB_OVERLAY_VSYNC_CTRL, &vsync_en);
		if (ret)
			FBTEST_MSG_DEBUG("\n MSMFB_OVERLAY_VSYNC_CTRL failed! (Line %d)\n",
					__LINE__);
	}
	return ret;
}

int writeback_init_start(int framesize)
{
	int result = TEST_RESULT_FAIL;
	if (testflag == FALSE) {//if test is not multiopt
		/* in mixer-2 write back mode kernel only support ION,PMEM is not supported.
			allocate ION buffer */
		FBTEST_MSG_DEBUG("[WRITEBACK_INIT_START] Buffer Allocation  \n");
		if (allocMEM(framesize + (WRITEBACK_SIZE * 2))) {
			FBTEST_MSG_DEBUG("[WRITEBACK_INIT_START] Buffer Allocation failed!\
			(Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	//call write back init ioctl
	FBTEST_MSG_DEBUG("[WRITEBACK_INIT_START] MSMFB_WRITEBACK_INIT \n");
	result = ioctl(FB->fb_fd,MSMFB_WRITEBACK_INIT,NULL);
	if (result < 0) {
		FBTEST_MSG_DEBUG("[WRITEBACK_INIT_START] MSMFB_WRITEBACK_INIT failed!\
		line=%d err=%d\n", __LINE__, result);
		return TEST_RESULT_FAIL;
	}

	//call write back start ioctl
	FBTEST_MSG_DEBUG("[WRITEBACK_INIT_START] MSMFB_WRITEBACK_START \n");
	result = ioctl(FB->fb_fd,MSMFB_WRITEBACK_START,NULL);
	if (result < 0) {
		FBTEST_MSG_DEBUG("[WRITEBACK_INIT_START] MSMFB_WRITEBACK_START failed!\
		line=%d err=%d\n", __LINE__, result);
		return TEST_RESULT_FAIL;
	}

	return 	TEST_RESULT_PASS;
}

int writeback_queueBuff(struct msmfb_data *fbdata,int offset)
{
	int result = TEST_RESULT_FAIL;
	setMisr(DISPLAY_MISR_MDP);
	fbdata->offset = offset;//farme size of input image
	fbdata->memory_id = MEM->mem_fd;
	fbdata->id = 0;
	fbdata->flags = 0;
	fbdata->iova = 0;//specify fbtest is using mixer-2
	/* reset the writback buffer for dumping */
	memset(MEM->mem_buf+offset,0x00,WRITEBACK_SIZE*2);
	/* call write back queue buffer ioctl */
	FBTEST_MSG_DEBUG("[WRITEBACK_QUEUE_BUFF] MSMFB_WRITEBACK_QUEUE_BUFFER \n");
	result = ioctl(FB->fb_fd,MSMFB_WRITEBACK_QUEUE_BUFFER,fbdata);
	if (result < 0) {
		FBTEST_MSG_DEBUG("[WRITEBACK_QUEUE_BUFF] MSMFB_WRITEBACK_QUEUE_BUFFER\
			failed! line=%d err=%d\n", __LINE__, result);
		return TEST_RESULT_FAIL;
	}
	return 	TEST_RESULT_PASS;
}

int writeback_dqueue_dump(struct msmfb_data *fbdata, int offset, char *testname)
{
	int result = TEST_RESULT_FAIL;
	char filename[200];
	char time[16];
	struct fb_var_screeninfo *vinfo = &(FB->fb_vinfo);
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	static int i;
	int count = 0;
	fbdata->flags = MSMFB_WRITEBACK_DEQUEUE_BLOCKING;
	//call write back de-queue buffer ioctl
	FBTEST_MSG_DEBUG("[WRITEBACK_DQUEUE_BUFF] MSMFB_WRITEBACK_DEQUEUE_BUFFER \n");
	result = ioctl(FB->fb_fd,MSMFB_WRITEBACK_DEQUEUE_BUFFER,fbdata);
	if (result < 0) {
		FBTEST_MSG_DEBUG("[WRITEBACK_DQUEUE_DUMP] MSMFB_WRITEBACK_DEQUEUE_BUFFER\
			failed! line=%d err=%d\n", __LINE__, result);
		return TEST_RESULT_FAIL;
	}
	getMisr(DISPLAY_MISR_MDP);
	FBTEST_MSG_DEBUG("Dequeue buffer from MDP with fd %u\toffset %u\t priv %u\n",
		fbdata->offset, fbdata->memory_id, fbdata->priv);
	//dump output frame
	date_timestamp_string(time);
	if (!strcmp(thisFBTEST->dumpPath,""))
	{
		VPRINT(verbose,"Please provide writeback dump path with '-D'\n");
		showError(-FILE_NOT_FOUND, 1);
	}
	if (!(testEnable & (1 << lookup_key_case_insensitive(MDP4testNameLUT, "videoplay", 0)))) {
		snprintf(filename, sizeof(filename), "%s/%s_%d%s_%s_%s_%dx%d_NV12.yuv", thisFBTEST->dumpPath, testname, i, time,
			thisFBTEST->fileParams.filename, thisFBTEST->fileParams.charFormat, WRITEBACK_WIDTH(vinfo->xres),
			WRITEBACK_HEIGHT(vinfo->yres));
		FBTEST_MSG_DEBUG("dumping writeback buffer\n");
		dump_img(MEM->mem_buf+offset, WRITEBACK_SIZE, filename);
	} else {
		count = WRITEBACK_SIZE * i;
		memcpy(buff + count, MEM->mem_buf + offset, WRITEBACK_SIZE);
	}
	i++;
	return TEST_RESULT_PASS;
}

int writeback_stop_terminate(struct msmfb_data *fbdata)
{
	int result = TEST_RESULT_FAIL;
	//call write back stop ioctl
	FBTEST_MSG_DEBUG("[WRITEBACK_STOP] MSMFB_WRITEBACK_STOP \n");
	result = ioctl(FB->fb_fd,MSMFB_WRITEBACK_STOP,NULL);
	if (result < 0) {
		FBTEST_MSG_DEBUG("[WRITEBACK_STOP] MSMFB_WRITEBACK_STOP failed!\
			line=%d err=%d\n", __LINE__, result);
		return TEST_RESULT_FAIL;
	}
	//call write back terminate ioctl
	FBTEST_MSG_DEBUG("[WRITEBACK_TERMINATE] MSMFB_WRITEBACK_TERMINATE \n");
	result = ioctl(FB->fb_fd,MSMFB_WRITEBACK_TERMINATE,NULL);
	if (result < 0) {
		FBTEST_MSG_DEBUG("[WRITEBACK_TERMINATE] MSMFB_WRITEBACK_TERMINATE\
		failed! line=%d err=%d\n", __LINE__, result);
		return TEST_RESULT_FAIL;
	}
	return 	TEST_RESULT_PASS;
}

int allMDP4Test(void)
{
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	if (thisFBTEST->testMode == USER) {
		VPRINT(verbose, "\nallMDP4 test is not supported in USER mode.\
		Try with Auto.\n");
		return TEST_RESULT_SKIP;
	}
	formatTest();
	scaleTest();
	rotateTest();
	cropTest();
	moveTest();
	deInterlaceTest();
	blendTest();
	multiopTest();
	CSCTest();
	MDP4adversarialTest();
	overlayAlphaTest();
	hwCursorTest();
	overlayTest();
	overlayARGBTest();
	allcolorFormatOverlayTest();
	overlayScaleTest();
	overlayStressTest();
	return 0;
}

int MDP4adversarialTest(void)
{
	int fb_fd = FB->fb_fd;
	int format = MDP_RGB_565;
	struct fb_var_screeninfo *vinfo = &(FB->fb_vinfo);


	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	overlay.src.width  = SQUARE_WIDTH;
	overlay.src.height = SQUARE_HEIGHT;
	overlay.src.format = format;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}
	overlay.id = MSMFB_NEW_REQUEST;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = SQUARE_WIDTH;
	overlay.src_rect.h = SQUARE_HEIGHT;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = SQUARE_WIDTH;
	overlay.dst_rect.h = SQUARE_HEIGHT;
	overlay.flags = 0;


	/* illegal source format test */
	printf("\n Executing illegal source format test...");
	overlay.src.format = MDP_IMGTYPE_LIMIT2 + 1;
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id > 0) {
		printf("ERROR: MSMFB_OVERLAY_SET illegal source format test "
			   "failed! \n");
		return TEST_RESULT_FAIL;
	}
	overlay.src.format = format;
	printf("\n Passed illegal source format test");


	/* illegal source rectangle coordinate test */
	printf("\n\n Executing illegal source rectangle coordinate test...");
	overlay.src_rect.x = SQUARE_WIDTH;
	overlay.src_rect.y = SQUARE_HEIGHT;
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id > 0) {
		printf("ERROR: MSMFB_OVERLAY_SET illegal source rectangle "
			   "coordinate test failed! \n");
		return TEST_RESULT_FAIL;
	}
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	printf("\n Passed illegal source rectangle coordinate test");


	/* illegal source rectangle height and width test */
	printf("\n\n Executing illegal source rectangle height and width test...");
	overlay.src_rect.w = vinfo->xres + 2;
	overlay.src_rect.h = vinfo->yres + 2;
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id > 0) {
		printf("ERROR: MSMFB_OVERLAY_SET illegal source rectangle height and "
			   "width test failed! \n");
		return TEST_RESULT_FAIL;
	}
	overlay.src_rect.w = SQUARE_WIDTH;
	overlay.src_rect.h = SQUARE_HEIGHT;
	printf("\n Passed illegal source rectangle height and width test");


	/* illegal destination rectangle coordinate test */
	printf("\n\n Executing illegal destination rectangle coordinate test...");
	overlay.dst_rect.x = vinfo->xres;
	overlay.dst_rect.y = vinfo->yres;
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id > 0) {
		printf("ERROR: MSMFB_OVERLAY_SET illegal destination coordinate "
			   "test failed! \n");
		return TEST_RESULT_FAIL;
	}
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	printf("\n Passed illegal destination rectangle coordinate test");


	/* illegal destination rectangle height and width test */
	printf("\n\n Executing illegal destination rectangle height and width test...");
	overlay.dst_rect.w = vinfo->xres+2;
	overlay.dst_rect.h = vinfo->yres+2;
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id > 0) {
		printf("\n ERROR: MSMFB_OVERLAY_SET illegal destination height "
			   "and width failed!");
		return TEST_RESULT_FAIL;
	}
	overlay.dst_rect.w = 0;
	overlay.dst_rect.h = 0;
	printf("\n\n Passed illegal destination rectangle height and width test");

	overlay_id = -1;
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = FB->fb_fd;

	/* Illegal overlay play call*/
	printf("\n\n Executing illegal overlay play call test...");
	if (!ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		printf("\nERROR! Illegal MSMFB_OVERLAY_PLAY call failed \n");
		return TEST_RESULT_FAIL;
	}
	printf("\n Passed illegal overlay play call test");


	/* Illegal overlay unset call*/
	printf("\n\n Executing illegal overlay unset call test...");
	if (!ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id)) {
		printf("\nERROR! illegal MSMFB_OVERLAY_UNSET  call failed \n");
		return TEST_RESULT_FAIL;
	}
	printf("\n Passed illegal overlay unset call test\n");

	return TEST_RESULT_PASS;

}

/* Color Space conversion test */
int CSCTest(void)
{
	int result = TEST_RESULT_PASS, i;

	struct msmfb_mdp_pp mdp_pp_cfg;

	struct mdp_csc matrix1 = {
		5, /*OVERLAY_PIPE_VG1*/
		{
			0x0254, 0x0000, 0x0331,
			0x0254, 0xff37, 0xfe60,
			0x0254, 0x0409, 0x0000,
		},
		{
			0xfff0, 0xff80, 0xff80,
		},
		{
			0, 0, 0,
		},
		{
			0, 0xff, 0, 0xff, 0, 0xff,
		},
		{
			0, 0xff, 0, 0xff, 0, 0xff,
		}

	};
	VPRINT(verbose,"Starting CSC Test\n");

	memcpy(&mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_mv,
			matrix1.csc_mv, sizeof(matrix1.csc_mv));

	memcpy(&mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_pre_bv,
			matrix1.csc_pre_bv, sizeof(matrix1.csc_pre_bv));

	memcpy(&mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_post_bv,
			matrix1.csc_post_bv, sizeof(matrix1.csc_post_bv));

	memcpy(&mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_pre_lv,
			matrix1.csc_pre_lv, sizeof(matrix1.csc_pre_lv));

	memcpy(&mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_post_lv,
			matrix1.csc_post_lv, sizeof(matrix1.csc_post_lv));

	mdp_pp_cfg.data.csc_cfg_data.csc_data.flags =
			MDP_CSC_FLAG_YUV_IN | MDP_CSC_FLAG_ENABLE ;

	mdp_pp_cfg.data.csc_cfg_data.block = MDP_BLOCK_VG_1;

	mdp_pp_cfg.op = mdp_op_csc_cfg;

	VPRINT(verbose,"\n\n\tORIGINAL VALUES:\n");

	for (i = 0; i < 9; i++)
		VPRINT(verbose, "csc_mv[%d] = 0x%x\n", i,
		mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_mv[i]);

	if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &mdp_pp_cfg)) {
		VPRINT(verbose, "MSMFB_MDP_PP failed\n");
		return TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "\n MSMFB_MDP_PP success.");
	}

	/* Calling Video play test with default CSC matrix value */
	result = videoPlayTest();

	VPRINT(verbose,"\n\n\tDISTORTED VALUES:");

	mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_mv[3] = 0;
	mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_mv[4] = 0;
	mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_mv[5] = 0;

	for (i = 0; i < 9; i++)
		VPRINT(verbose, "\n csc_mv[%d] = 0x%x", i,
		mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_mv[i]);

	if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &mdp_pp_cfg)) {
		VPRINT(verbose, "MSMFB_MDP_PP failed\n");
		return TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "\n MSMFB_MDP_PP success.");
	}

	VPRINT(verbose,"\n");
	/* Calling Video play test with distorted CSC matrix value */
	result = videoPlayTest();

	mdp_pp_cfg.data.csc_cfg_data.csc_data.flags = 0;

	if (ioctl(FB->fb_fd, MSMFB_MDP_PP, &mdp_pp_cfg)) {
		VPRINT(verbose, "MSMFB_MDP_PP failed\n");
		return TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "\n MSMFB_MDP_PP success.");
	}

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nCSC Test: PASS\n");
	} else
		VPRINT(verbose, "\nCSC Test: FAIL\n");
	return result;
}

#ifdef ENABLE_POSTPROC
int postprocTest(void)
{
	int result = TEST_RESULT_FAIL;
	struct fbtest_params *thisFBTEST;

	thisFBTEST = &FBTEST;

	VPRINT(verbose, "postprocTest Run format test first...\n");
	formatTest();

	if (thisFBTEST->pp_data.postproc_use_file) {
		printf("\n postproc_file: %s ",
			thisFBTEST->pp_data.postproc_file);
		result = parsePPFrameworkFile(thisFBTEST->pp_data.postproc_file);
		thisFBTEST->pp_data.postproc_use_file = 0;
	} else {
		printf("\n Running default postproc cfg file : %s ",
			DEFAULT_POSTPROC_FILE_PATH);
		result = parsePPFrameworkFile(DEFAULT_POSTPROC_FILE_PATH);
	}
	sleep(1);
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	sleep(1);
	printf("\nPost Processing Test: %s\n",result?"FAIL":"PASS");
	return result;
}
#endif

int ditherTest(void){
	int result = TEST_RESULT_PASS;
	int  fb_fd;
	struct fb_var_screeninfo *vinfo;
	FILE *fptr = NULL;
	size_t bytesRead = 0;
	char *buff;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int size;
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
		thisFBTEST->fileParams.inResolution.height,thisFBTEST->fileParams.inputFormat);

	//allocate buffer for reading data from input image
	buff = (char *)malloc(size);
	if (buff == NULL)
		showError(-MALLOC_FAILED, 1);

	vinfo = &(FB->fb_vinfo);
	fb_fd = FB->fb_fd;

	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	//writeback structure
	struct msmfb_data fbdata;

	VPRINT(verbose, "\nPerforming Dithering Test...\n");


	memset(&overlay, 0, sizeof(struct mdp_overlay));
	//open image file
	fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
	if (fptr == NULL) {
		FBTEST_MSG_DEBUG("[Dithering] Cannot open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}

	//read image in buffer
	bytesRead  = fread(buff, 1, size, fptr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[Dithering] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}

	//close the file
	fclose(fptr);
	if (writeback_flag == TRUE) {
		result = writeback_init_start(size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		drawBG();
		//allocate PMEM buffer
		if (allocMEM(FB->fb_size)) {
			FBTEST_MSG_DEBUG("[Dithering] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	//copy image data to PMEM buffer
	memcpy(MEM->mem_buf, buff, (int)(size));

	overlay.src.width  = thisFBTEST->fileParams.inResolution.width;
	overlay.src.height = thisFBTEST->fileParams.inResolution.height;
	overlay.src.format = thisFBTEST->fileParams.inputFormat;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = thisFBTEST->fileParams.inResolution.width;
	overlay.src_rect.h = thisFBTEST->fileParams.inResolution.height;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = thisFBTEST->fileParams.inResolution.width;
	overlay.dst_rect.h = thisFBTEST->fileParams.inResolution.height;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}
	overlay.transp_mask = MDP_TRANSP_NOP;
	if (ditherFlag == ENABLED) {

		//Setting Dithering flag on
		VPRINT(verbose, "Dither on\n");
		overlay.flags = MDP_DITHER;
	} else {

		VPRINT(verbose, "Dither off\n");
		overlay.flags = 0;
	}

	FBTEST_MSG_DEBUG("MDP_DITHER %x\n",overlay.flags);

	overlay.id = MSMFB_NEW_REQUEST;


	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[Dithering] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			   __LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	if (writeback_flag == TRUE) {
		result = writeback_queueBuff(&fbdata,size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	overlay_id = overlay.id;	/* return from mdp */
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[Dithering] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
			   __LINE__, overlay_id);
		return -OVERLAY_PLAY_FAILED;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	if (writeback_flag == TRUE) {
		result = writeback_dqueue_dump(&fbdata, size, "ditherTest");
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "Displaying Image on device...\n");
		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}
	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.

	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "Dither test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec),
		(time_stop.tv_usec-time_start.tv_usec));
	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nDither Test: PASS\n");
	} else {
		VPRINT(verbose, "\nDither Test: FAIL\n");
	}
	free(buff);
	return result;
}

int deInterlaceTest(void){
	int result = TEST_RESULT_PASS;
	int  fb_fd;
	struct fb_var_screeninfo *vinfo;
        FILE *fptr = NULL;
	size_t bytesRead = 0;
	char *buff;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int size;
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
		thisFBTEST->fileParams.inResolution.height,thisFBTEST->fileParams.inputFormat);

	//allocate buffer for reading data from input image
	buff = (char *)malloc(size);
	if (buff == NULL)
		showError(-MALLOC_FAILED, 1);

	vinfo = &(FB->fb_vinfo);
	fb_fd = FB->fb_fd;

	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	//writeback structure
	struct msmfb_data fbdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	VPRINT(verbose, "\nPerforming De-Interlace Test...\n");


	//open image file
	fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
	if (fptr == NULL) {
		FBTEST_MSG_DEBUG("[De-Interlace] Cannot open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}

	//read image in buffer
	bytesRead  = fread(buff, 1, size, fptr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[De-Interlace] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}

	//close the file
	fclose(fptr);
	if (writeback_flag == TRUE) {
		result = writeback_init_start(size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		drawBG();
		//allocate PMEM buffer
		if (allocMEM(FB->fb_size)) {
			FBTEST_MSG_DEBUG("[De-Interlace] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	//copy image data to PMEM buffer
	memcpy(MEM->mem_buf, buff, (int)(size));

	overlay.src.width  = thisFBTEST->fileParams.inResolution.width;
	overlay.src.height = thisFBTEST->fileParams.inResolution.height;
	overlay.src.format = thisFBTEST->fileParams.inputFormat;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = thisFBTEST->fileParams.inResolution.width;
	overlay.src_rect.h = thisFBTEST->fileParams.inResolution.height;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = thisFBTEST->fileParams.inResolution.width;
	overlay.dst_rect.h = thisFBTEST->fileParams.inResolution.height;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}
	overlay.transp_mask = MDP_TRANSP_NOP;
	if (deInterlaceFlag == ENABLED) {

		//Setting De-Interlace flag on
		VPRINT(verbose, "Deinterlace on\n");
		overlay.flags = MDP_DEINTERLACE | MDP_DEINTERLACE_ODD;
	} else {
		VPRINT(verbose, "Deinterlace off\n");
		overlay.flags = 0;
	}

	FBTEST_MSG_DEBUG("MDP_DEINTERLACE %x\n",overlay.flags);

	overlay.id = MSMFB_NEW_REQUEST;


	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[De-Interlace] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			   __LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	if (writeback_flag == TRUE) {
		result = writeback_queueBuff(&fbdata,size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	overlay_id = overlay.id;	/* return from mdp */
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;


	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[De-Interlace] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
			   __LINE__, overlay_id);
		return -OVERLAY_PLAY_FAILED;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	if (writeback_flag == TRUE) {
		result = writeback_dqueue_dump(&fbdata, size, "deinterlaceTest");
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "Displaying Image on device...\n");

		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}
	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.
	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "De-Interlace test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec),
		(time_stop.tv_usec-time_start.tv_usec));
	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nDe-Interlace Test: PASS\n");
	} else
		VPRINT(verbose, "\nDe-Interlace Test: FAIL\n");
	free(buff);
	return result;
}

/* Taken from old fbtest & modified for command line input */
int videoPlayTest(void){
	FILE *imgRAW;
	struct timeval t_start, t_end;
	int result  = TEST_RESULT_PASS, frame, framesize,filesize, err, uwait;
	float  fps, fps_avg;
	unsigned char *vid_buf_front, *vid_buf_back;
	int vid_buf_front_id;
	struct mdp_overlay overlay, *overlayp;
	struct msmfb_overlay_data ov_front, ov_back;
	struct msmfb_overlay_data *ovp_front, *ovp_back;
	struct Video video;
	struct stat buf;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	struct msmfb_data fbdata;
	char filename[200];

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	/* calculating size of file */
	if (!stat(thisFBTEST->fileParams.filenamePath,&buf)) {
		filesize = buf.st_size;
	} else {
		FBTEST_MSG_DEBUG("Error in filesize calculation\n");
		return -INCORRECT_FILE_SIZE;
	}
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	VPRINT(verbose, "\nPerforming Video Test...\n");

	/* Initialization of Video structure parameters */
	video.width = thisFBTEST->fileParams.inResolution.width;
	video.height = thisFBTEST->fileParams.inResolution.height;
	video.format = thisFBTEST->fileParams.inputFormat;
	if (thisFBTEST->fps == 0) {
		VPRINT(verbose, "Warning: FPS of video is not entered\n");
	}
	video.fps = thisFBTEST->fps;
	/* calculate framesize of video file */
	framesize = getFrameSize(video.width,video.height,video.format);

	/* calculating number of frames */
	if (filesize <= framesize) {
		VPRINT(verbose, "Frame size is less than file size, settign frame count as 1\n");
		video.frames = 1;
	} else {
		video.frames = filesize/framesize;
	}

	/* opening video file */
	FBTEST_MSG_DEBUG("\nOpening video (%s)\n", thisFBTEST->fileParams.filenamePath);
	if ((imgRAW = fopen(thisFBTEST->fileParams.filenamePath,"rb")) == NULL) {
		FBTEST_MSG_DEBUG("[Video] Cannot open raw video file %s!\n",
			thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}

	if (writeback_flag == TRUE) {
		if (allocMEM(framesize*2 + (WRITEBACK_SIZE * 2))) {
			FBTEST_MSG_DEBUG("[Video] WRITEBACK Buffer Allocation failed!\
				(Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
		buff = (unsigned char *)malloc(video.frames*WRITEBACK_SIZE);
		if (buff == NULL) {
			showError(-MALLOC_FAILED, 1);
		}
		memset(buff, 0x00, video.frames*WRITEBACK_SIZE);
	} else {
		/* drawing white background */
		drawBG();
		/* allocate PMEM */
		if (allocMEM(framesize*2)) {
			FBTEST_MSG_DEBUG("[Video] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	/* Setup vid_buf_front_id */
	overlayp = &overlay;
	overlayp->src.width  = video.width;
	overlayp->src.height = video.height;
	overlayp->src.format = video.format;
	overlayp->src_rect.x = 0;
	overlayp->src_rect.y = 0;
	overlayp->src_rect.w = video.width;
	overlayp->src_rect.h = video.height;
	overlayp->dst_rect.x = 0;
	overlayp->dst_rect.y = 0;
	if (writeback_flag == TRUE) {
		overlayp->dst_rect.w = FB->fb_vinfo.xres;
		overlayp->dst_rect.h = FB->fb_vinfo.yres;
	} else {
		overlayp->dst_rect.w = (((unsigned int) video.width*8) >
			FB->fb_vinfo.xres) ? FB->fb_vinfo.xres : ((unsigned int) video.width*8);
		overlayp->dst_rect.h = (((unsigned int)video.height*8) >
			FB->fb_vinfo.yres) ? FB->fb_vinfo.yres : ((unsigned int) video.height*8);
	}
	overlayp->z_order = FB_OVERLAY_VID_0;
	overlayp->alpha = 0xFF;
	overlayp->transp_mask = MDP_TRANSP_NOP;
	if (FB->fb_vinfo.bits_per_pixel == 32)
		overlay.is_fg = 0;
	else
		overlay.is_fg = 1;

	overlayp->id = MSMFB_NEW_REQUEST;

	if (deInterlaceFlag == ENABLED) {
		/* Setting De-Interlace flag on */
		VPRINT(verbose, "Deinterlace on\n");
		overlay.flags = MDP_DEINTERLACE | MDP_DEINTERLACE_ODD;
	} else {
		VPRINT(verbose, "Deinterlace off\n");
		overlay.flags = 0;
	}
	vid_buf_front_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, overlayp);
	if (vid_buf_front_id < 0) {
		FBTEST_MSG_DEBUG("[Video] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			   __LINE__, vid_buf_front_id);
		return -OVERLAY_SET_FAILED;
	}

	vid_buf_front_id = overlayp->id;

	ov_front.id = overlayp->id;
	curr_overlay_id[0] = ov_front.id;   /* UTF: added for cleanup code addition. */
	ov_front.data.flags = 0;
	ov_front.data.offset = 0;
	ov_front.data.memory_id = MEM->mem_fd;

	ov_back.id = overlayp->id;
	curr_overlay_id[0] = ov_back.id;   /* UTF: added for cleanup code addition. */
	ov_back.data.flags = 0;
	ov_back.data.offset = framesize;
	ov_back.data.memory_id = MEM->mem_fd;

	fps_avg = 0.0;
	uwait = 0;
	vid_buf_front = MEM->mem_buf;
	vid_buf_back = MEM->mem_buf + framesize;

	ovp_front = &ov_front;
	ovp_back = &ov_back;
	if (writeback_flag == TRUE) {
		result = writeback_init_start(framesize*2);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	VPRINT(verbose, "Playing video (Frame = %d)on device...\n", video.frames);
	/*Play Video file*/
	for (frame = 0; frame < video.frames; frame++) {
		if (writeback_flag == TRUE) {
			result = writeback_queueBuff(&fbdata, framesize*2);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		}
		thisFBTEST->frameNum = frame;
		gettimeofday(&t_start, NULL);/* for fps calculation start time */
		if (fseek(imgRAW, framesize * frame, SEEK_SET) != 0) /* Seek to the particualr frame */
			break;
		/* read the frame */
		err = fread (vid_buf_back, sizeof(unsigned char), framesize, imgRAW);
		if (err != framesize) {
			VPRINT(verbose, "[Video] ERROR! RAW image read failed at frame %d \
				(read %d of %d bytes)\n", frame, err, framesize);
			break;
		}
		/* swap the buffer to copy data in front buffer */
		swapPointers(&vid_buf_front, &vid_buf_back);
		swapPointers((unsigned char **)&ovp_front, (unsigned char **)&ovp_back);
		/* play the frame */
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, ovp_front)) {
			FBTEST_MSG_DEBUG("[Video] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
				__LINE__, overlayp->id);
			result = -OVERLAY_PLAY_FAILED;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		gettimeofday(&t_end,NULL);/* for fps calculation end time */
		if (writeback_flag == TRUE) {
			result = writeback_dqueue_dump(&fbdata, framesize*2, "videodeinterlaceTest");
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		}
		fps = (float)(1000.0)/(float)(t_end.tv_sec*1000+t_end.tv_usec/1000-t_start.tv_sec*1000-t_start.tv_usec/1000);
		/* wait calculation depending on fps & video fps */
		if (video.fps != 0) {
			uwait = (int)(1000000.0 / video.fps - 1000000.0 / fps);
			if (uwait > 0.0) usleep(uwait);
		}

		gettimeofday(&t_end,NULL);
		fps = (float)(1000.0)/(float)(t_end.tv_sec*1000+t_end.tv_usec/1000-t_start.tv_sec*1000-t_start.tv_usec/1000);

		fps_avg += fps;
	}
	if (writeback_flag == TRUE) {
		result = writeback_stop_terminate(&fbdata);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		if (!strncmp(thisFBTEST->dumpPath, "", sizeof(thisFBTEST->dumpPath))) {
			VPRINT(verbose, "Please provide writeback dump path with '-D'\n");
			showError(-FILE_NOT_FOUND, 1);
		}
		snprintf(filename, sizeof(filename), "%s/videodump_%dx%d_NV12.yuv", thisFBTEST->dumpPath,
					WRITEBACK_WIDTH(FB->fb_vinfo.xres),
					WRITEBACK_HEIGHT(FB->fb_vinfo.yres));
		dump_img(buff, video.frames*WRITEBACK_SIZE, filename);
	} else {
		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}

	if (cleanup_overlay())
		result =  -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST; 	/* UTF: added for cleanup code addition. */

	if (result == TEST_RESULT_PASS) {
		fps_avg /= video.frames;
		printf("FPS = %.2f\n", fps_avg);
	}
	/* close video file */
	FBTEST_MSG_DEBUG("Close Video\n");
	fclose(imgRAW);

	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "Video test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec),
		(time_stop.tv_usec-time_start.tv_usec));

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nVideo Test: PASS\n");
	} else {
		VPRINT(verbose, "\nVideo Test: FAIL\n");
	}
	free(buff);
	return result;

}

/* Taken from old fbtest */
int fpsVsyncTest(void){

	int result = TEST_RESULT_PASS,  iterations,i, fb_fd;;
	struct timeval time_start, time_stop;
	struct fb_var_screeninfo * vinfo;
	struct fb_var_screeninfo vinfo_bak;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	//writeback structure
	struct msmfb_data fbdata;
	struct timeval start,stop;
	float writeback_time = 0.0;
	unsigned int framesize;
	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 50;

	vinfo_bak = FB->fb_vinfo;
	vinfo = &FB->fb_vinfo;
	fb_fd = FB->fb_fd;

	VPRINT(verbose, "\nPerforming fps-vsync Test...\n");

	if (thisFBTEST->fBuffer == FB0) {

	paintBuffer(FB->fb_buf, BLACK, FB->fb_fmt, FB->fb_vinfo.xres,
		FB->fb_vinfo.yres, FB->fb_finfo.line_length);
	paintBuffer(FB->fb_buf_back, WHITE, FB->fb_fmt, FB->fb_vinfo.xres,
		FB->fb_vinfo.yres, FB->fb_finfo.line_length);

	/* FPS with VSync ON */
	vinfo->activate = FB_ACTIVATE_VBL;
	gettimeofday(&time_start, NULL);
	for (i = 0; i < iterations; i++) {
		if (i%2)
			vinfo->yoffset = 0;
		else
			vinfo->yoffset = vinfo->yres ;
		thisFBTEST->frameNum = i;
		result = doPanDisplay();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	gettimeofday(&time_stop, NULL);
	printf("Average FPS with VSync ON = %.1f fps\n",
		(float)(iterations*1000)/
		(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
		time_start.tv_sec*1000-time_start.tv_usec/1000));
	add_delay_in_user_mode();

	/*FPS with VSync OFF*/
	vinfo->activate = FB_ACTIVATE_NOW;

	gettimeofday(&time_start, NULL);
	for (i = 0; i < iterations; i++) {
		if (i%2)
			vinfo->yoffset = 0;
		else
			vinfo->yoffset = vinfo->yres ;
		result = doPanDisplay();
		thisFBTEST->frameNum = i;
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	gettimeofday(&time_stop, NULL);
	printf("Average FPS with VSync OFF = %.1f fps\n",
		(float)(iterations*1000)/
		(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
		time_start.tv_sec*1000-time_start.tv_usec/1000));

	} else {

	framesize = getFrameSize(FB->fb_vinfo.xres, FB->fb_vinfo.yres, FB->fb_fmt);
	if (writeback_flag == FALSE) {
		if (allocMEM(framesize * 2)) {
			FBTEST_MSG_DEBUG("[FPSTEST] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	} else {
		if (allocMEM(framesize * 2 + (WRITEBACK_SIZE * 2))) {
			FBTEST_MSG_DEBUG("[FPSTEST] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
		testflag = TRUE;
	}
	paintBuffer(MEM->mem_buf, BLACK, FB->fb_fmt, FB->fb_vinfo.xres,
		FB->fb_vinfo.yres, FB->fb_finfo.line_length);
	paintBuffer(MEM->mem_buf + framesize, WHITE, FB->fb_fmt, FB->fb_vinfo.xres,
		FB->fb_vinfo.yres, FB->fb_finfo.line_length);

	overlay.src.width  = FB->fb_vinfo.xres;
	overlay.src.height = FB->fb_vinfo.yres;
	overlay.src.format = FB->fb_fmt;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = FB->fb_vinfo.xres;
	overlay.src_rect.h = FB->fb_vinfo.yres;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = FB->fb_vinfo.xres;
	overlay.dst_rect.h = FB->fb_vinfo.yres;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.alpha = 0xFF;

	if (FB->fb_vinfo.bits_per_pixel == 32)
		overlay.is_fg = 0;
	else
		overlay.is_fg = 1;

	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.flags = 0;
	overlay.id = MSMFB_NEW_REQUEST;


	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[FPSTEST] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}

	overlay_id = overlay.id;	/* return from mdp */
	curr_overlay_id[0] = overlay_id;
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (writeback_flag == FALSE) {

		/* FPS with VSync ON */
		vinfo->activate = FB_ACTIVATE_VBL;
		gettimeofday(&time_start, NULL);
		for (i = 0; i < (iterations / 2); i++) {
			ovdata.data.offset = 0;
			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
				FBTEST_MSG_DEBUG("[FPSTEST] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
					__LINE__, overlay_id);
				return -OVERLAY_PLAY_FAILED;
			}
			thisFBTEST->frameNum = i;
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;

			ovdata.data.offset = framesize;
			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
				FBTEST_MSG_DEBUG("[FPSTEST] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
					__LINE__, overlay_id);
				return -OVERLAY_PLAY_FAILED;
			}
			thisFBTEST->frameNum = i + 1;
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		}
		gettimeofday(&time_stop,NULL);
		printf("Average FPS with VSync ON = %.1f fps\n",
			(float)(iterations*1000)/
			(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
			time_start.tv_sec*1000-time_start.tv_usec/1000));

		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}
	/* FPS with VSync OFF */
	vinfo->activate = FB_ACTIVATE_NOW;
	gettimeofday(&time_start, NULL);
	for (i = 0; i < (iterations / 2); i++) {
		if (writeback_flag == TRUE) {

			gettimeofday(&start,NULL);

			result = writeback_init_start(framesize * 2);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;

			result = writeback_queueBuff(&fbdata, framesize * 2);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;

			gettimeofday(&stop, NULL);
			writeback_time += (float)(stop.tv_sec*1000+stop.tv_usec/1000-
					start.tv_sec*1000-start.tv_usec/1000);
		}
		ovdata.data.offset = 0;
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			FBTEST_MSG_DEBUG("[FPSTEST] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
				__LINE__, overlay_id);
			return -OVERLAY_PLAY_FAILED;
		}
		thisFBTEST->frameNum = i;
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		if (writeback_flag == TRUE) {
			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
				FBTEST_MSG_DEBUG("[FPSTEST] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
					__LINE__, overlay_id);
				return -OVERLAY_PLAY_FAILED;
			}
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			gettimeofday(&start,NULL);

			result = writeback_dqueue_dump(&fbdata, framesize * 2, "fpsVsyncTest");
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;

			result = writeback_stop_terminate(&fbdata);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;

			result = writeback_init_start(framesize * 2);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;

			result = writeback_queueBuff(&fbdata, framesize * 2);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;

			gettimeofday(&stop,NULL);
			writeback_time += (float)(stop.tv_sec*1000+stop.tv_usec/1000-
					start.tv_sec*1000-start.tv_usec/1000);
		}

		ovdata.data.offset = framesize;
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			FBTEST_MSG_DEBUG("[FPSTEST] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
				__LINE__, overlay_id);
			return -OVERLAY_PLAY_FAILED;
		}
		thisFBTEST->frameNum = i + 1;
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		if (writeback_flag == TRUE) {
			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
				FBTEST_MSG_DEBUG("[FPSTEST] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
					__LINE__, overlay_id);
				return -OVERLAY_PLAY_FAILED;
			}
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			gettimeofday(&start, NULL);

			result = writeback_dqueue_dump(&fbdata, framesize * 2, "fpsVsyncTest");
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			result = writeback_stop_terminate(&fbdata);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			gettimeofday(&stop,NULL);

			writeback_time += (float)(stop.tv_sec*1000+stop.tv_usec/1000-
					start.tv_sec*1000-start.tv_usec/1000);
		}
	}
	gettimeofday(&time_stop,NULL);
	printf("Average FPS with VSync OFF = %.1f fps\n",
		(float)(iterations*1000)/
		(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
		time_start.tv_sec*1000-time_start.tv_usec/1000 - writeback_time));
	testflag = FALSE;
	if (cleanup_overlay())
		result = -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST;
	}

	FB->fb_vinfo = vinfo_bak;
	VPRINT(verbose, "fps-vsync test over\n");

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nfps-vsync Test: PASS\n");
	} else {
		VPRINT(verbose, "\nfps-vsync Test: FAIL\n");
	}


	return TEST_RESULT_PASS;
}

int formatTest(void)
{
	int result = TEST_RESULT_PASS;
	int  fb_fd;
	size_t bytesRead = 0;
	struct fb_var_screeninfo *vinfo;
        FILE *fptr = NULL;
	unsigned char *buff;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int size;
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
		thisFBTEST->fileParams.inResolution.height,thisFBTEST->fileParams.inputFormat);

	//allocate buffer for reading data from input image
	buff = (unsigned char *)malloc(size);
	if (buff == NULL)
		showError(-MALLOC_FAILED, 1);

	vinfo = &(FB->fb_vinfo);
	fb_fd = FB->fb_fd;

	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	//writeback structure
	struct msmfb_data fbdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	VPRINT(verbose, "\nPerforming Format Test...\n");


	//open image file
	FBTEST_MSG_DEBUG("[FORMAT] open raw Image file %s !\n",
		thisFBTEST->fileParams.filenamePath);
	fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
	if (fptr == NULL) {
		FBTEST_MSG_DEBUG("[FORMAT] Cannot open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}
	//read image in buffer
	bytesRead  = fread(buff, 1, size, fptr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[FORMAT] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}

	//close the file
	fclose(fptr);
	if (writeback_flag == TRUE) {
		FBTEST_MSG_DEBUG("[FORMAT] writeback_init_start (Line %d)\n", __LINE__);
		result = writeback_init_start(size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		drawBG();
		//allocate PMEM buffer
		if (allocMEM(size)) {
			FBTEST_MSG_DEBUG("[FORMAT] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	//copy image data to PMEM buffer
	memcpy(MEM->mem_buf, buff, (int)(size));
	free(buff);

	overlay.src.width  = thisFBTEST->fileParams.inResolution.width;
	overlay.src.height = thisFBTEST->fileParams.inResolution.height;
	overlay.src.format = thisFBTEST->fileParams.inputFormat;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = thisFBTEST->fileParams.inResolution.width;
	overlay.src_rect.h = thisFBTEST->fileParams.inResolution.height;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		overlay.dst_rect.w = thisFBTEST->fileParams.inResolution.width;
	} else {
		overlay.dst_rect.w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		overlay.dst_rect.h = thisFBTEST->fileParams.inResolution.height;
	} else {
		overlay.dst_rect.h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.flags = 0;
	overlay.id = MSMFB_NEW_REQUEST;

	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[FORMAT] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		printf("err %d\n", errno);
		return -OVERLAY_SET_FAILED;
	}
	if (writeback_flag == TRUE) {
		result = writeback_queueBuff(&fbdata,size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	overlay_id = overlay.id;	/* return from mdp */
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[FORMAT] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		return -OVERLAY_PLAY_FAILED;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	if (writeback_flag == TRUE) {
		result = writeback_dqueue_dump(&fbdata, size, "formatTest");
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "Displaying Image on device...\n");

		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}
	if (strcmp(thisFBTEST->testname, "postproc") != 0) {
		if (cleanup_overlay())
			result = -OVERLAY_UNSET_FAILED;

		curr_overlay_id[0] = MSMFB_NEW_REQUEST;
		/*UTF: added for cleanup code addition.*/

		VPRINT(verbose, "Format test over\n");

		if (result == TEST_RESULT_PASS) {
			VPRINT(verbose, "\nFormat Test: PASS\n");
		} else
			VPRINT(verbose, "\nFormat Test: FAIL\n");
	}
	return result;
}

int doCrop(void){
	int result = TEST_RESULT_PASS;
	int  fb_fd;
	struct fb_var_screeninfo *vinfo;
	FILE *fptr = NULL;
	size_t bytesRead = 0;
	char *buff;
	struct cropParams *thisCrop;
	thisCrop = &CROPTEST;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int size;

	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,thisFBTEST->fileParams.inputFormat);

	//allocate buffer for reading data from input image
	buff = (char *)malloc(size);
	if (buff == NULL)
		showError(-MALLOC_FAILED, 1);

	vinfo = &(FB->fb_vinfo);
	fb_fd = FB->fb_fd;

	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	//writeback structure
	struct msmfb_data fbdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));

	//open image file
	fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
	if (fptr == NULL) {
		FBTEST_MSG_DEBUG("[CROP] Cannot open raw Image file %s !\n",
				thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}

	//read image in buffer
	bytesRead  = fread(buff, 1, size, fptr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[CROP] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}
	//close the file
	fclose(fptr);
	if (writeback_flag == TRUE) {
		result = writeback_init_start(size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		drawBG();
		//allocate PMEM buffer
		if (allocMEM(size)) {
			FBTEST_MSG_DEBUG("[CROP] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	//copy image data to PMEM buffer
	memcpy(MEM->mem_buf, buff, (int)(size));

	overlay.src.width  = thisFBTEST->fileParams.inResolution.width;
	overlay.src.height = thisFBTEST->fileParams.inResolution.height;
	overlay.src.format = thisFBTEST->fileParams.inputFormat;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = thisFBTEST->fileParams.inResolution.width;
	overlay.src_rect.h = thisFBTEST->fileParams.inResolution.height;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.flags = 0;

	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		overlay.dst_rect.w = thisFBTEST->fileParams.inResolution.width;
	} else {
		overlay.dst_rect.w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		overlay.dst_rect.h = thisFBTEST->fileParams.inResolution.height;
	} else {
		overlay.dst_rect.h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.flags = 0;
	overlay.id = MSMFB_NEW_REQUEST;

	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[CROP] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
				__LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	overlay_id = overlay.id;	/* return from mdp */
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[CROP] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
				__LINE__, overlay_id);
		return -OVERLAY_PLAY_FAILED;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	if (writeback_flag == FALSE) {
		VPRINT(verbose, "Displaying Input Image on device...\n");

		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}

	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.

	/********starting crop test**************************/
	overlay.src_rect.x = thisCrop->src_rect.x;
	overlay.src_rect.y = thisCrop->src_rect.y;
	overlay.src_rect.w = thisCrop->resolution.width;
	overlay.src_rect.h = thisCrop->resolution.height;
	overlay.dst_rect.x = thisCrop->dst_rect.x;
	overlay.dst_rect.y = thisCrop->dst_rect.y;
	overlay.dst_rect.w = thisCrop->resolution.width;
	overlay.dst_rect.h = thisCrop->resolution.height;

	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.flags = 0;
	overlay.id = MSMFB_NEW_REQUEST;

	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[CROP] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			   __LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	if (writeback_flag == TRUE) {
		result = writeback_queueBuff(&fbdata,size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	overlay_id = overlay.id;	/* return from mdp */
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;
	result = TEST_RESULT_PASS;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[CROP] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		return -OVERLAY_PLAY_FAILED;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	if (writeback_flag == TRUE) {
		result = writeback_dqueue_dump(&fbdata, size, "cropTest");
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {

		VPRINT(verbose, "Displaying Cropped result on device...\n");

		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}

	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.
	/***********end crop test*******************************/
	free(buff);
	return result;
}

int cropCalculation(void){
	int result = 0;
	struct fbtest_params *thisFBTEST;
	struct cropParams *thisCrop;
	struct fb_var_screeninfo *vinfo;
	thisFBTEST = &FBTEST;
	thisCrop = &CROPTEST;
	vinfo = &(FB->fb_vinfo);
	int crop_percentage = thisFBTEST->crop;

	thisCrop->src_rect.x = 0;
	thisCrop->src_rect.y = 0;
	thisCrop->dst_rect.x = 0;
	thisCrop->dst_rect.y = 0;

	if ((crop_percentage & (1 << P_25))) {

		VPRINT(verbose, "\nCropping 25%% of Input Image\n");
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width * .25;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height * .25;

	} else if ((crop_percentage & (1 << P_50))) {

		VPRINT(verbose, "\nCropping 50%% of Input Image\n");
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width * .5;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height * .5;

	} else if ((crop_percentage & (1 << P_75))) {

		VPRINT(verbose, "\nCropping 75%% of Input Image\n");
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width * .75;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height * .75;

	} else if ((crop_percentage & (1 << C_BOTTOM))) {

		VPRINT(verbose, "\nCropping Bottom-Half of Input Image\n");
		thisCrop->src_rect.y = thisFBTEST->fileParams.inResolution.height * .5;
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height * .5;

	} else if ((crop_percentage & (1 << C_TOP))) {

		VPRINT(verbose, "\nCropping Top-Half of Input Image\n");
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height * .5;

	} else if ((crop_percentage & (1 << C_LEFT))) {

		VPRINT(verbose, "\nCropping Left-Half of Input Image\n");
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width * .5;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height;

	} else if ((crop_percentage & (1 << C_RIGHT))) {

		VPRINT(verbose, "\nCropping Right-Half of Input Image\n");
		thisCrop->src_rect.x = thisFBTEST->fileParams.inResolution.width * .5;
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width * .5;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height;

	} else if ((crop_percentage & (1 << C_RAND))) {

		VPRINT(verbose, "\nRandom Cropping of Input Image\n");
		if ((thisFBTEST->coordinate.x == 0) && (thisFBTEST->coordinate.y == 0)) {
		} else {
			if ((thisFBTEST->coordinate.x <= (int) thisFBTEST->fileParams.inResolution.width) &&\
				(thisFBTEST->coordinate.y <= (int) thisFBTEST->fileParams.inResolution.height)) {

				//starting coordinate for cropping is non-zero(x,y)
				thisCrop->src_rect.x = thisFBTEST->coordinate.x;
				thisCrop->src_rect.y = thisFBTEST->coordinate.y;
				thisCrop->dst_rect.x = thisFBTEST->coordinate.x;
				thisCrop->dst_rect.y = thisFBTEST->coordinate.y;

			} else {
				FBTEST_MSG_DEBUG("Error: Not a vaild cropping start co-ordinate\n");
				return -INAVALID_CROP_PARAMS;
			}
		}
		if ((thisFBTEST->cropresolution.width != 0) && (thisFBTEST->cropresolution.height != 0)) {
			if (((thisCrop->src_rect.x)+(thisFBTEST->cropresolution.width)) <= \
				thisFBTEST->fileParams.inResolution.width) {
				thisCrop->resolution.width = thisFBTEST->cropresolution.width;
			} else {
				FBTEST_MSG_DEBUG("Error: Not a vaild width for cropping\n");
				return -INAVALID_CROP_PARAMS;
			}
			if (((thisCrop->src_rect.y)+(thisFBTEST->cropresolution.height)) <= \
				thisFBTEST->fileParams.inResolution.height) {
				thisCrop->resolution.height = thisFBTEST->cropresolution.height;
			} else {
				FBTEST_MSG_DEBUG("Error: Not a vaild height for cropping\n");
				return -INAVALID_CROP_PARAMS;
			}

		} else {
			FBTEST_MSG_DEBUG("Error: width & height should be non-zero\n");
			return -INAVALID_CROP_PARAMS;
		}
	} else {
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height;
	}
	return result;
}

int cropTest(void){
	int result = -1;
	struct fbtest_params *thisFBTEST;

	struct fb_var_screeninfo *vinfo;
	thisFBTEST = &FBTEST;
	int i = P_25;
	vinfo = &(FB->fb_vinfo);
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	VPRINT(verbose, "\nPerforming Crop Test...\n");

	switch(thisFBTEST->testMode) {
		case USER:
			result = cropCalculation();

			if (!result)
				result = doCrop();
			break;
		case AUTO:
			srand(1);
			for (i; i <= C_RAND; i++) {
				thisFBTEST->crop = 0;
				thisFBTEST->crop = (1 << i);
				if (thisFBTEST->crop & (1 << C_RAND)) {
					thisFBTEST->coordinate.x = rand_range(0,thisFBTEST->fileParams.inResolution.width);
					thisFBTEST->coordinate.y = rand_range(0,thisFBTEST->fileParams.inResolution.height);
					thisFBTEST->cropresolution.width = rand_range(0, (thisFBTEST->fileParams.inResolution.width - thisFBTEST->coordinate.x));
					thisFBTEST->cropresolution.height = rand_range(0, (thisFBTEST->fileParams.inResolution.height - thisFBTEST->coordinate.y));
				}
				result = cropCalculation();

				if (!result)
					result = doCrop();
			}

			break;

		default:
			VPRINT(verbose, "Error: Not a valid mode of operation\n");
			break;
	}

	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "Crop test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec),
		(time_stop.tv_usec-time_start.tv_usec));

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nCrop Test: PASS\n");
	} else
		VPRINT(verbose, "\nCrop Test: FAIL\n");

	return result;
}

int multiopTest(void){
	int result = TEST_RESULT_PASS,swap;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	struct cropParams *thisCrop;
	thisFBTEST = &FBTEST;
	thisCrop = &CROPTEST;
	vinfo = &(FB->fb_vinfo);
	int mode = USER;
	unsigned int cmdRotDeg;
	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	//writeback structure
	struct msmfb_data fbdata;
	int size;
	int is_rgb = 0;
	int rotDstFormat;
	int bufferOffset = 0;
	int wbOffset = 0;
	int rotDstSize = 0;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	VPRINT(verbose, "\nPerforming Multi-operation Test...\n");

	is_rgb = lookup_val(format_name_table,
			thisFBTEST->fileParams.inputFormat, 0);

	if (thisFBTEST->testMode == AUTO) {

		multiopflag = 1;//for auto mode set the test as rotate-move-crop-scale
		thisFBTEST->rotateDegree = ROT_270;/* 270 deg rotation */
		thisFBTEST->rotateFlip = FLIP_LRUD;/* LR & UD flip */
		thisFBTEST->imgOffset.endX = thisFBTEST->fileParams.inResolution.height * 0.25;//move 'x' coordinate
		thisFBTEST->imgOffset.endY = thisFBTEST->fileParams.inResolution.width * 0.25;//move 'y' coordinate

		//2x scaling and swapped width-height
		thisFBTEST->outResolution.width = thisFBTEST->fileParams.inResolution.height * 0.5;
		thisFBTEST->outResolution.height = thisFBTEST->fileParams.inResolution.width * 0.5;
		thisFBTEST->crop |= 1 << P_25;/*set 25 percentage cropping*/
		mode = AUTO;

		//setting mode user so that rotation,crop,move & scale work in user mode
		thisFBTEST->testMode = USER;
	}

	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = thisFBTEST->fileParams.inResolution.width;
	overlay.dst_rect.h = thisFBTEST->fileParams.inResolution.height;

	//if test is rotate-move-crop-scale
	if (multiopflag) {

		VPRINT(verbose, "Performing Rotate-Move-Crop-Scale test...\n");

		cmdRotDeg = thisFBTEST->rotateDegree;

		//call rotate test
		rotateTest();
		size = getFrameSize(thisFBTEST->fileParams.inResolution.width,
				thisFBTEST->fileParams.inResolution.height,
				thisFBTEST->fileParams.inputFormat);
		bufferOffset = size;
		rotDstFormat = getRotatorDstFormat(
				thisFBTEST->fileParams.inputFormat, cmdRotDeg);
		overlay.src.format = rotDstFormat;
		rotDstSize = getFrameSize(
				thisFBTEST->fileParams.inResolution.width,
				thisFBTEST->fileParams.inResolution.height,
				rotDstFormat);
		//swap width & height of test image if rotation is 90 or 270
		if ((cmdRotDeg  == ROT_90) || (cmdRotDeg == ROT_270)) {
			swap = thisFBTEST->fileParams.inResolution.width;
			thisFBTEST->fileParams.inResolution.width = \
			thisFBTEST->fileParams.inResolution.height;
			thisFBTEST->fileParams.inResolution.height = swap;
		}
	} else {
		VPRINT(verbose, "Performing Move-Crop-Scale test...\n");
		/* test is move-crop-scale */
		formatTest();
		overlay.src.format = thisFBTEST->fileParams.inputFormat;
		bufferOffset = getFrameSize(overlay.src.width,
					overlay.src.height, overlay.src.format);
	}

	overlay.src.width  = thisFBTEST->fileParams.inResolution.width;
	overlay.src.height = thisFBTEST->fileParams.inResolution.height;
	overlay.src.format = thisFBTEST->fileParams.inputFormat;
	size = getFrameSize(overlay.src.width,overlay.src.height,overlay.src.format);
	if (!cropCalculation()) {
		overlay.src_rect.x = thisCrop->src_rect.x;//for crop
		overlay.src_rect.y = thisCrop->src_rect.y;//for crop
		overlay.src_rect.w = thisCrop->resolution.width;//for crop
		overlay.src_rect.h = thisCrop->resolution.height;//for crop
	}

	//check move coordinate within panel width/height
	if (((thisFBTEST->imgOffset.endX + overlay.src.width) <= FB->fb_vinfo.xres)\
		&& ((thisFBTEST->imgOffset.endY + overlay.src.height) <= FB->fb_vinfo.yres)) {
	    overlay.dst_rect.x =  thisFBTEST->imgOffset.endX;//for move
	    overlay.dst_rect.y =  thisFBTEST->imgOffset.endY;//for move
	} else {
		thisFBTEST->testMode = mode;
		FBTEST_MSG_DEBUG("Error:X-Y coordinate to move the image is not in range\n");
		return -OFFSET_OUT_OF_BOUND;
	}
	if (thisFBTEST->outResolution.width != 0)
		overlay.dst_rect.w = thisFBTEST->outResolution.width;//for scale
	if (thisFBTEST->outResolution.height != 0)
		overlay.dst_rect.h = thisFBTEST->outResolution.height;//for scale

	if (is_rgb && ((overlay.dst_rect.w < overlay.src_rect.w) ||
				(overlay.dst_rect.h < overlay.src_rect.h))) {
		overlay.flags = MDP_OV_PIPE_SHARE;
	} else {
		overlay.flags = 0;
	}

	//Check if WxH are valid as per MDP4 support
	if (is_valid_overlay(&overlay))
	{
		FBTEST_MSG_DEBUG("\n[MULTIOP] Resolution OutofBound! (Line %d)\n",	__LINE__);
		return -RES_OUT_OF_BOUND;
	}
	if ((overlay.dst_rect.x + overlay.dst_rect.w) > vinfo->xres) {
		FBTEST_MSG_DEBUG("sum of output start coordinate x and display width is\
		greater than display panel width\n");
		return -RES_OUT_OF_BOUND;
	}
	if ((overlay.dst_rect.y + overlay.dst_rect.h) > vinfo->yres) {
		FBTEST_MSG_DEBUG("sum of output start coordinate y and display height is\
		greater than display panel height\n");
		return -RES_OUT_OF_BOUND;
	}
	if ((overlay.src_rect.w > vinfo->xres) || (overlay.src_rect.h > vinfo->yres) ||\
		(overlay.dst_rect.w > vinfo->xres) || (overlay.dst_rect.h > vinfo->yres))
	{
		FBTEST_MSG_DEBUG("\n[MULTIOP] Resolution OutofBound! (Line %d)\n",	__LINE__);
		return -RES_OUT_OF_BOUND;
	}
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.id = MSMFB_NEW_REQUEST;

	if (writeback_flag == TRUE) {
		testflag = TRUE;
		wbOffset = bufferOffset + rotDstSize;
		result = writeback_init_start(wbOffset);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		result = writeback_queueBuff(&fbdata, wbOffset);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}

	if (!is_rgb) {
		/* odd res or src offsets are not supported on yuv formats */
		overlay.src_rect.w &= ~1;
		overlay.src_rect.h &= ~1;
		overlay.dst_rect.w &= ~1;
		overlay.dst_rect.h &= ~1;
		overlay.src_rect.x &= ~1;
		overlay.src_rect.y &= ~1;
	}
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[MULTIOP] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	overlay_id = overlay.id;	/* return from mdp */
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	if (rotDstSize)
		ovdata.data.offset = bufferOffset;
	else
		ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[MULTIOP] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		return -OVERLAY_PLAY_FAILED;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	if (writeback_flag == TRUE) {
		result = writeback_dqueue_dump(&fbdata, wbOffset,
				"multiopTest");
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "Displaying Image\n");
		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}

	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.

	thisFBTEST->testMode = mode;
	testflag = FALSE;
	VPRINT(verbose, "Multi-operation test over\n");

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nMulti-operation Test: PASS\n");
	} else
		VPRINT(verbose, "\nMulti-operation Test: FAIL\n");

	return result;
}

int colorkeyTest(void){
	int result = TEST_RESULT_PASS,bg_set =0;
	unsigned int i = 0,index =0;
	unsigned int inFileCnt = 0;
	FILE *fptr = NULL;
	size_t bytesRead = 0;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	struct inputFileParams ImgFileParams[2], tmpFileParams;
	int ImgFrameSize[2];
	char *ImgBuff[2];
	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);
	int size = 0;
	struct mdp_overlay overlay[2];
	struct msmfb_overlay_data ovdata[2];
	int overlay_id[2];
	int ImgDataOffset[2] = {0,0};
	int zOrderVal[2] = {FB_OVERLAY_RGB_1,FB_OVERLAY_RGB_1};
	int is_fgVal[2] = {0,0};
	//writeback structure
	struct msmfb_data fbdata;

	memset(overlay, 0, sizeof(struct mdp_overlay) * 2);
	VPRINT(verbose, "\nPerforming Colorkeying Test...\n");

	if (thisFBTEST->testMode == AUTO) {
		printf("Color keying doesn't support Auto mode as it \
		required manually setting with exact transparency value\n");
		return TEST_RESULT_SKIP;
	}

	//Parsing input file name
	for (i = 0; i < 2; i++)	{
		if (strcmp(thisFBTEST->blend.ImagePath[i], "")) {

			result = parseFileName(thisFBTEST->blend.ImagePath[i], &ImgFileParams[inFileCnt++]);
			if (result < 0)
				showError(result, 1);
		} else {
			FBTEST_MSG_DEBUG("Input file name for color keying is NULL\n");
			return -FILE_NOT_FOUND;
		}
	}

	//for BG color keying,choose BG image which is of frame buffer format
	if (colorkeyflag && (thisFBTEST->fBuffer == FB0)) {

		if (ImgFileParams[0].inputFormat == FB->fb_fmt) {

			bg_set = 1;

		} else if ((ImgFileParams[1].inputFormat == FB->fb_fmt) && !bg_set) {

			FBTEST_MSG_DEBUG("swapping file for BG color keying \n");
			memcpy(&tmpFileParams, &ImgFileParams[BG_PIPE], sizeof(struct inputFileParams));
			memcpy(&ImgFileParams[BG_PIPE], &ImgFileParams[1], sizeof(struct inputFileParams));
			memcpy(&ImgFileParams[1], &tmpFileParams, sizeof(struct inputFileParams));

		} else {
			FBTEST_MSG_DEBUG("for BG color keying one test image should have frame buffer format\n");
			return -INVALID_COLORKEY_IMAGE_FORMAT;
		}
	}

	//Reading the input files
	for (i = 0; i < inFileCnt; i++) {
		if (strcmp(ImgFileParams[i].filenamePath, "")) {

			ImgFrameSize[i] = getFrameSize(ImgFileParams[i].inResolution.width,\
				ImgFileParams[i].inResolution.height,ImgFileParams[i].inputFormat);
			//allocate buffer for reading data from input image
			ImgBuff[i] = (char *)malloc(ImgFrameSize[i]);
			if (ImgBuff[i] == NULL)
				showError(-MALLOC_FAILED, 1);
			//open image file
			fptr = fopen(ImgFileParams[i].filenamePath, "rb");
			if (fptr == NULL) {
				FBTEST_MSG_DEBUG("[Color Key] Cannot open raw Image file %s !\n",
					ImgFileParams[i].filenamePath);
				return -FOPEN_FAILED;
			}
			//read image in buffer
			bytesRead  = fread(ImgBuff[i], 1, ImgFrameSize[i], fptr);
			if (bytesRead == 0) {
				FBTEST_MSG_DEBUG("[Color Key] fread failed! (Line %d)\n", __LINE__);
				return -FREAD_FAILED;
			}
			//close the file
			fclose(fptr);
		}
	}

	if (inFileCnt < 2) {
		printf("Minimum 2 images required for color key operation\n");
		return -INVALID_PARAMS;
	} else {
		//for BG color keying
		if (colorkeyflag && (thisFBTEST->fBuffer == FB0)) {
			for (i = 0; i < ImgFileParams[BG_PIPE].inResolution.height; i++) {
				memcpy(&FB->fb_buf[i*vinfo->xres*(int)getFormatBpp(ImgFileParams\
				[BG_PIPE].inputFormat)], ((unsigned short *) (&ImgBuff[BG_PIPE][i*\
				ImgFileParams[BG_PIPE].inResolution.width*(int)getFormatBpp(\
				ImgFileParams[BG_PIPE].inputFormat)])),ImgFileParams[BG_PIPE].\
				inResolution.width*(int)getFormatBpp(ImgFileParams[BG_PIPE].inputFormat));
			}

			/* allocate PMEM buffer */
			if (allocMEM(FB->fb_size)) {
				FBTEST_MSG_DEBUG("[Color Key] Buffer Allocation failed! (Line %d)\n", __LINE__);
				return -PMEMALLOC_FAILED;
			}
			//setting transparency value
			overlay[1].transp_mask = thisFBTEST->transparency;
			index = 1;//increment the index of file next to play
			add_delay_in_user_mode();
		} else {
			if (writeback_flag == TRUE) {
				size = ImgFrameSize[0] + ImgFrameSize[1];
				result = writeback_init_start(size);
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			} else {
				drawBG();
				//allocate PMEM buffer
				if (allocMEM(FB->fb_size*2)) {
					FBTEST_MSG_DEBUG("[Color Key] Buffer Allocation failed! (Line %d)\n", __LINE__);
					return -PMEMALLOC_FAILED;
				}
			}
			//setting color keying specific value
			zOrderVal[1] = 1;
			is_fgVal[1] = 1;
			ImgDataOffset[1] = ImgFrameSize[0];
			//setting transparency value
			overlay[0].transp_mask = MDP_TRANSP_NOP;
			overlay[1].transp_mask = thisFBTEST->transparency;
		}
	}

	//set & play overlays
	for (i = index; i < inFileCnt; i++) {

		//copy image data to PMEM buffer
		memcpy(MEM->mem_buf+ImgDataOffset[i], ImgBuff[i], (int)(ImgFrameSize[i]));
		overlay[i].src.width  = ImgFileParams[i].inResolution.width;
		overlay[i].src.height = ImgFileParams[i].inResolution.height;
		overlay[i].src.format = ImgFileParams[i].inputFormat;
		overlay[i].src_rect.x = 0;
		overlay[i].src_rect.y = 0;
		overlay[i].src_rect.w = ImgFileParams[i].inResolution.width;
		overlay[i].src_rect.h = ImgFileParams[i].inResolution.height;
		overlay[i].dst_rect.x = 0;
		overlay[i].dst_rect.y = 0;
		overlay[i].dst_rect.w = ImgFileParams[i].inResolution.width;
		overlay[i].dst_rect.h = ImgFileParams[i].inResolution.height;

		overlay[i].is_fg = is_fgVal[i];
		overlay[i].alpha = 0xff;//made complete opaque
		overlay[i].z_order = zOrderVal[i];
		//For mdss, use DMA pipe for first layer
		if ((mdp_version >= MDP_V5) && (i == index) &&
			(lookup_val(format_name_table,
			ImgFileParams[i].inputFormat, 0)))
			overlay[i].flags = MDP_OV_PIPE_FORCE_DMA;
		else
			overlay[i].flags = 0;
		overlay[i].id = MSMFB_NEW_REQUEST;


		/*------------------------------------------*/
		FBTEST_MSG_DEBUG("transparency %x\n",overlay[i].transp_mask);
		//set overlay ioctl
		overlay_id[i] = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay[i]);
		if (overlay_id[i] < 0) {
		FBTEST_MSG_DEBUG("[Color Key] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",\
		__LINE__, overlay_id[i]);
			return -OVERLAY_SET_FAILED;
		}
		if (((inFileCnt-1) == i) && (writeback_flag == TRUE)) {
			result = writeback_queueBuff(&fbdata,size);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		}
		overlay_id[i] = overlay[i].id;	/* return from mdp */
		curr_overlay_id[i] = overlay_id[i];   //UTF: added for cleanup code addition.
		ovdata[i].id = overlay_id[i];
		ovdata[i].data.flags = 0;
		ovdata[i].data.offset = ImgDataOffset[i];
		ovdata[i].data.memory_id = MEM->mem_fd;

		//play overlay
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata[i])) {
			FBTEST_MSG_DEBUG("[Color key] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",\
			__LINE__, overlay_id[i]);
			return -OVERLAY_PLAY_FAILED;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}
	if (((inFileCnt-1) == i) && (writeback_flag == TRUE)) {
		result = writeback_dqueue_dump(&fbdata, size, "colorkeyTest");
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		add_delay_in_user_mode();
	}

	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	for (i = index; i < inFileCnt; i++) {
		curr_overlay_id[i] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.
	}

	VPRINT(verbose, "Colorkey test over\n");

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Color Key Overlay Test: Pass \n");
	} else
		VPRINT(verbose, "Color Key Overlay Test: Fail \n");
	free(ImgBuff[0]);
	free(ImgBuff[1]);
	return result;
}
extern struct inputFileParams ImgFileParams[MAX_FILE_TO_BLEND];
extern unsigned int istage;
int blendTest(void)
{
	int result = TEST_RESULT_PASS;
	unsigned int i = 0;
	unsigned int inFileCnt = 0;
	int isAutoMode = 0, bg_set =0, fg_yuv =0;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	struct inputFileParams tmpFileParams;
	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);

	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	if ((thisFBTEST->blend.isPixBlend) || !(thisFBTEST->blend.ImgAlphaVal[0] == -1)) {
		thisFBTEST->blend.ImgAlphaVal[0] = (thisFBTEST->blend.ImgAlphaVal[0] >= 0 && thisFBTEST->blend.ImgAlphaVal[0] <= 0xFF)? thisFBTEST->blend.ImgAlphaVal[0] : 0x2F;
		thisFBTEST->blend.ImgAlphaVal[1] = (thisFBTEST->blend.ImgAlphaVal[1] >= 0 && thisFBTEST->blend.ImgAlphaVal[1] <= 0xFF)? thisFBTEST->blend.ImgAlphaVal[1] : 0x7F;
		thisFBTEST->blend.ImgAlphaVal[2] = (thisFBTEST->blend.ImgAlphaVal[2] >= 0 && thisFBTEST->blend.ImgAlphaVal[2] <= 0xFF)? thisFBTEST->blend.ImgAlphaVal[2] : 0xAF;
		thisFBTEST->blend.ImgAlphaVal[3] = (thisFBTEST->blend.ImgAlphaVal[3] >= 0 && thisFBTEST->blend.ImgAlphaVal[3] <= 0xFF)? thisFBTEST->blend.ImgAlphaVal[3] : 0xF0;
	}

	VPRINT(verbose, "\nPerforming Image Blend Test...\n");

	memset(ImgFileParams, 0x00, MAX_FILE_TO_BLEND * sizeof(struct inputFileParams));

	switch (thisFBTEST->testMode) {
		case USER:

			for (i = 0; i < MAX_FILE_TO_BLEND; i++) {
				if (strcmp(thisFBTEST->blend.ImagePath[i], "")) {

					result = parseFileName(thisFBTEST->blend.ImagePath[i], &ImgFileParams[inFileCnt++]);
					if (result < 0)
						showError(result, 1);
				}
			}

			for (i = 0; i < inFileCnt; i++) {

				if (!lookup_val(format_name_table, ImgFileParams[i].inputFormat, 0)) {
					fg_yuv++;
					if (fg_yuv > MAX_YUV_PIPE_SUPPORT) {
						showError(-MAX_YUV_SUPPORT_REACHED, 0);
						strlcpy(thisFBTEST->blend.ImagePath[i], "",sizeof(thisFBTEST->blend.ImagePath[i]));
					}
				}
				if (thisFBTEST->fBuffer == FB0) {
					if ((ImgFileParams[i].inputFormat == FB->fb_fmt) && !bg_set) {
						memcpy(&tmpFileParams, &ImgFileParams[BG_PIPE], sizeof(struct inputFileParams));
						memcpy(&ImgFileParams[BG_PIPE], &ImgFileParams[i], sizeof(struct inputFileParams));
						memcpy(&ImgFileParams[i], &tmpFileParams, sizeof(struct inputFileParams));
						bg_set = 1;
					}
				}

			}
			if ((thisFBTEST->fBuffer == FB1) || (thisFBTEST->fBuffer == FB2))
				bg_set = 1;
			if (!bg_set) {
				if (thisFBTEST->fBuffer == FB0)
					showError(-INVALID_BLEND_BG_FORMAT, 0);

				if (inFileCnt == MAX_FILE_TO_BLEND) {
					memcpy(&ImgFileParams[MAX_FILE_TO_BLEND-1], &ImgFileParams[BG_PIPE], sizeof(struct inputFileParams));
				}else{
					memcpy(&ImgFileParams[inFileCnt], &ImgFileParams[BG_PIPE], sizeof(struct inputFileParams));
				}
			}

			VPRINT(verbose, "\nUSR Call doImageBlend Mode %d infile=%d\n", isAutoMode, inFileCnt);
			result = doImageBlend(ImgFileParams, inFileCnt, isAutoMode);

			break;

		case AUTO:
			isAutoMode = 1;
			strlcpy(thisFBTEST->blend.ImagePath[0], thisFBTEST->fileParams.filenamePath, sizeof(thisFBTEST->blend.ImagePath[0]));
			strlcpy(thisFBTEST->blend.ImagePath[1], thisFBTEST->fileParams.filenamePath, sizeof(thisFBTEST->blend.ImagePath[1]));
			strlcpy(thisFBTEST->blend.ImagePath[2], thisFBTEST->fileParams.filenamePath, sizeof(thisFBTEST->blend.ImagePath[2]));
			strlcpy(thisFBTEST->blend.ImagePath[3], thisFBTEST->fileParams.filenamePath, sizeof(thisFBTEST->blend.ImagePath[3]));

			for(i = 0; i < MAX_FILE_TO_BLEND; i++)
			{
				if(strcmp(thisFBTEST->blend.ImagePath[i], "")){


					result = parseFileName(thisFBTEST->blend.ImagePath[i], &ImgFileParams[i]);
					if(result < 0)
						showError(result, 1);

					if(!lookup_val(format_name_table, ImgFileParams[i].inputFormat, 0))
						fg_yuv++;
				}
			}
			if (fg_yuv>MAX_YUV_PIPE_SUPPORT) {
				showError(-MAX_YUV_SUPPORT_REACHED, 0);
				inFileCnt = MAX_YUV_PIPE_SUPPORT+1;
			} else {
				inFileCnt = MAX_FILE_TO_BLEND;
			}
			result = doImageBlend(ImgFileParams, inFileCnt, isAutoMode);
			break;

		default:
			VPRINT(verbose, "Error: Not a valid mode of operation\n");
			break;
	}

	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "Image Blend test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec),
		(time_stop.tv_usec-time_start.tv_usec));

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Image Blend Test: Pass \n");
	} else
		VPRINT(verbose, "Image Blend Test: Fail \n");
	return result;
}

int doImageBlend(struct inputFileParams *ptrImgFileParams, unsigned int inFileCnt, int isAutoMode)
{
	int result = TEST_RESULT_PASS;
	unsigned int i, j;
	int  fb_fd, overlayCnt = 0, overlaySetCnt = 0, overlayPlayCnt = 0;
	struct fb_var_screeninfo *vinfo;
	FILE *fptr = NULL;
	size_t bytesRead = 0;
	char *ImgBuff[MAX_FILE_TO_BLEND];
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int ImgFrameSize[MAX_FILE_TO_BLEND] = {0, 0, 0, 0};
	int ImgDataOffset[MAX_MDP4_OVERLAY_PIPE];
	int zOrderVal[MAX_MDP4_OVERLAY_PIPE];

	vinfo = &(FB->fb_vinfo);
	fb_fd = FB->fb_fd;

	int overlay_id[MAX_MDP4_OVERLAY_PIPE] = {MSMFB_NEW_REQUEST, MSMFB_NEW_REQUEST, MSMFB_NEW_REQUEST, MSMFB_NEW_REQUEST};
	struct mdp_overlay overlay[MAX_MDP4_OVERLAY_PIPE];
	struct msmfb_overlay_data ovdata[MAX_MDP4_OVERLAY_PIPE];
	struct msmfb_data fbdata;
	int size = 0;

	memset(overlay, 0, sizeof(struct mdp_overlay) * MAX_MDP4_OVERLAY_PIPE);
	for (i = 0; i < inFileCnt; i++) {
		if (strcmp(ImgFileParams[i].filenamePath, "")) {

			ImgFrameSize[i] = getFrameSize(ImgFileParams[i].inResolution.width,\
				ImgFileParams[i].inResolution.height,ImgFileParams[i].inputFormat);

			//allocate buffer for reading data from input image
			ImgBuff[i] = (char *)malloc(ImgFrameSize[i]);
			if (ImgBuff[i] == NULL)
				showError(-MALLOC_FAILED, 1);
			//open image file
			fptr = fopen(ImgFileParams[i].filenamePath, "rb");
			if (fptr == NULL) {
				FBTEST_MSG_DEBUG("[BLEND] Cannot open raw Image file %s !\n", ImgFileParams[i].filenamePath);
				return -FOPEN_FAILED;
			}
			//read image in buffer
			bytesRead = fread(ImgBuff[i], 1, ImgFrameSize[i], fptr);
			if (bytesRead == 0) {
				FBTEST_MSG_DEBUG("[BLEND] fread failed! (Line %d)\n", __LINE__);
				return -FREAD_FAILED;
			}
			FBTEST_MSG_DEBUG("[BLEND] Byte Read %d from Image file %s Expected byte %d!\n",
			bytesRead, ImgFileParams[i].filenamePath, ImgFrameSize[i]);
			//close the file
			fclose(fptr);
		}

		overlayCnt++;
		VPRINT(verbose, "Blending Image %d: %s  Img W=%d, H=%d\n",
			i, ImgFileParams[i].filenamePath,
                ImgFileParams[i].inResolution.width,
                ImgFileParams[i].inResolution.height);

	}
	ImgDataOffset[BG_PIPE] = 0;
	ImgDataOffset[FG1_PIPE] = ImgFrameSize[BG_PIPE];
	ImgDataOffset[FG2_PIPE] = ImgFrameSize[BG_PIPE] + ImgFrameSize[FG1_PIPE];
	ImgDataOffset[FG3_PIPE] = ImgFrameSize[BG_PIPE] + ImgFrameSize[FG1_PIPE] + ImgFrameSize[FG2_PIPE];

	size = ImgDataOffset[FG3_PIPE] + ImgFrameSize[FG3_PIPE];

	zOrderVal[BG_PIPE] = 0;
	zOrderVal[FG1_PIPE] = 1;
	zOrderVal[FG2_PIPE] = 2;
	zOrderVal[FG3_PIPE] = 3;

	overlaySetCnt = overlayPlayCnt = overlayCnt;
	if (writeback_flag == TRUE) {
		result = writeback_init_start(size);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		//allocate PMEM buffer
		if (allocMEM(FB->fb_size*4)) {
			FBTEST_MSG_DEBUG("[BLEND] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}

	if (thisFBTEST->blend.isPixBlend) {
		VPRINT(verbose, "Using Pixel Blend Method\n");
	} else {
		VPRINT(verbose, "Using Constant Blend Method\n");
	}
	if (thisFBTEST->fBuffer == FB0) {
		if (strncmp(ImgFileParams[BG_PIPE].filenamePath, "", sizeof(ImgFileParams[BG_PIPE].filenamePath))) {
			VPRINT(verbose, "Drawing BG with Image %d: %s (Alpha Value = 256)\n",
						BG_PIPE, ImgFileParams[BG_PIPE].filenamePath);
		} else {
			VPRINT(verbose, "Drawing BG with default white color (Alpha Value = 256)\n");
		}
	}

	for (i = BG_PIPE; i < inFileCnt; i++) {

		VPRINT(verbose, "Blending Image %d: %s (Alpha Value = %d)\n",i, ImgFileParams[i].filenamePath, thisFBTEST->blend.ImgAlphaVal[i]);

		for(j=0;j<=0xFF; j+= 16){
			if (j == 0x100)
				j = 0xFF;
			if (thisFBTEST->blend.isPixBlend)
				j = 0xFF;
			//copy image data to PMEM buffer
			memcpy(MEM->mem_buf+ImgDataOffset[i], ImgBuff[i], (int)(ImgFrameSize[i]));
			overlay[i].src.width  = ImgFileParams[i].inResolution.width;
			overlay[i].src.height = ImgFileParams[i].inResolution.height;
			overlay[i].src.format = ImgFileParams[i].inputFormat;
			overlay[i].src_rect.x = 0;
			overlay[i].src_rect.y = 0;
			overlay[i].src_rect.w  = (vinfo->xres > (ImgFileParams[i].inResolution.width + (i*20))) ?
				(ImgFileParams[i].inResolution.width) : vinfo->xres - (i*20);
			overlay[i].src_rect.h  = (vinfo->yres > (ImgFileParams[i].inResolution.height + (i*20))) ?
				(ImgFileParams[i].inResolution.height) : vinfo->yres - (i*20);
			overlay[i].dst_rect.w  = (vinfo->xres > (ImgFileParams[i].inResolution.width + (i*20))) ?
				(ImgFileParams[i].inResolution.width) : vinfo->xres - (i*20);
			overlay[i].dst_rect.h  = (vinfo->yres > (ImgFileParams[i].inResolution.height + (i*20))) ?
				(ImgFileParams[i].inResolution.height) : vinfo->yres - (i*20);
			overlay[i].dst_rect.x = i*20;
			overlay[i].dst_rect.y = i*20;

			VPRINT(verbose, "Dst Rect Param x=%d y=%d w=%d h=%d\n", overlay[i].dst_rect.x,
				overlay[i].dst_rect.y, overlay[i].dst_rect.w, overlay[i].dst_rect.h);
			VPRINT(verbose, "Var info Param w=%d h=%d\n", vinfo->xres, vinfo->yres);

                        overlay[i].z_order = zOrderVal[i];
                        overlay[i].is_fg = thisFBTEST->blend.isPixBlend? 0: 1;
                        if (isAutoMode) {
                                overlay[i].alpha = thisFBTEST->blend.isPixBlend? 0xFF: j;
                        } else {
                                if(thisFBTEST->blend.ImgAlphaVal[0] == -1) {
                                        overlay[i].alpha = thisFBTEST->blend.isPixBlend? 0xFF: j;
                                } else {
                                        overlay[i].alpha = thisFBTEST->blend.ImgAlphaVal[i];
                                        j = 0xFF;
                                }
                        }

			overlay[i].transp_mask = MDP_TRANSP_NOP;
			//For mdss, use DMA pipe for first layer
			if ((mdp_version >= MDP_V5) && (i == FG1_PIPE) &&
				(lookup_val(format_name_table,
				ImgFileParams[i].inputFormat, 0)))
				overlay[i].flags = MDP_OV_PIPE_FORCE_DMA;
			else
				overlay[i].flags = 0;

			overlay[i].id = overlay_id[i];
			overlay_id[i] = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay[i]);
			if (overlay_id[i] < 0) {
				FBTEST_MSG_DEBUG("[BLEND] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
					   __LINE__, overlay_id[i]);
				return -OVERLAY_SET_FAILED;
			}
			FBTEST_MSG_DEBUG("[BLEND] Set %d done id: %d\n",i, overlay_id[i]);
			FBTEST_MSG_DEBUG("play FG Image...\n");
			overlay_id[i] = overlay[i].id;	/* return from mdp */
			curr_overlay_id[i] = overlay_id[i];   //UTF: added for cleanup code addition.
			ovdata[i].id = overlay_id[i];
			ovdata[i].data.flags = 0;
			ovdata[i].data.offset = ImgDataOffset[i];
			ovdata[i].data.memory_id = MEM->mem_fd;
			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata[i])) {
				FBTEST_MSG_DEBUG("[BLEND] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
				__LINE__, overlay_id[i]);
				return -OVERLAY_PLAY_FAILED;
			}
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			add_delay_in_user_mode();
		}
		if (writeback_flag == TRUE) {
			result = writeback_queueBuff(&fbdata,size);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			result = writeback_dqueue_dump(&fbdata, size, "blendTest");
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		}
		sleep(2);
	}
	if (writeback_flag == TRUE) {
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	for (i = FG1_PIPE; i < inFileCnt; i++) {

		curr_overlay_id[i] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.
	}
	for (i = 0; i < inFileCnt; i++)
		free(ImgBuff[i]);
	return result;
}

int moveTest(void)
{
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	if (thisFBTEST->imgOffset.startX < 0 ||
		thisFBTEST->imgOffset.startY < 0 ||
		thisFBTEST->imgOffset.endX < 0 ||
		thisFBTEST->imgOffset.endY < 0) {

		FBTEST_MSG_DEBUG("\n[Move] Invalid Offsets! (Line %d)\n", __LINE__);
		return -INVALID_OFFSET;


	}
	unsigned int startX = thisFBTEST->imgOffset.startX;
	unsigned int startY = thisFBTEST->imgOffset.startY;
	unsigned int endX = thisFBTEST->imgOffset.endX;
	unsigned int endY = thisFBTEST->imgOffset.endY;

	//Check if offsets are valid as per MDP4 support
	if ((startX > (vinfo->xres - thisFBTEST->fileParams.inResolution.width)) ||
	(startY > (vinfo->yres- thisFBTEST->fileParams.inResolution.height))||
		(endX > (vinfo->xres - thisFBTEST->fileParams.inResolution.width)) ||
		(endY > (vinfo->yres - thisFBTEST->fileParams.inResolution.height)))
	{
		FBTEST_MSG_DEBUG("\n[Move] Offsets OutofBound! (Line %d)\n", __LINE__);
		return -OFFSET_OUT_OF_BOUND;
	}

	VPRINT(verbose, "\nPerforming Image Move Test...\n");

	switch(thisFBTEST->testMode) {
		case USER:
			result = doImageMove(startX, startY, endX, endY, thisFBTEST->moveType);
			break;

		case AUTO:
			switch (thisFBTEST->moveType) {
			case HORIZONTAL:
				VPRINT(verbose, "IMAGE TRAVERSAL IN HORIZONTAL DIRECTION\n");
				break;
			case VERTICAL:
				VPRINT(verbose, "IMAGE TRAVERSAL IN VERTICAL DIRECTION\n");
				break;
			case BOUNDARY:
				VPRINT(verbose, "IMAGE TRAVERSAL IN ACROSS BOUNDARY\n");
				break;
			case DIAGONALS:
				VPRINT(verbose, "IMAGE TRAVERSAL IN DIAGONALS\n");
				break;
			case RANDOM:
				VPRINT(verbose, "IMAGE TRAVERSAL IN RANDOM\n");
				break;
			case USER_DEFINED:
			default:
				VPRINT(verbose, "IMAGE TRAVERSAL IS NOT DEFINED, USE DIAGONALS\n");
				thisFBTEST->moveType = DIAGONALS;
			break;
			}
			result = doImageMove(startX, startY, endX, endY, thisFBTEST->moveType);
			break;

		default:
			VPRINT(verbose, "Error: Not a valid mode of operation\n");
			break;
	}

	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "Image Move test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec), (time_stop.tv_usec-time_start.tv_usec));

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Image Move Test: Pass \n");
	} else
		VPRINT(verbose, "Image move Test: Fail \n");

	return result;

}

//This function does up/down and arbitrary scaling.
int doImageMove(unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY, unsigned int traversal)
{
	int overlay_id, fb_fd;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	unsigned int i = 0;
	int result = TEST_RESULT_FAIL;
	thisFBTEST = &FBTEST;
	struct msmfb_data fbdata;
	int frameSize;
	FILE *imgPtr = NULL;
	size_t bytesRead = 0;
	int count = 0;//for traverseImage loop count
	//Get file params
	unsigned int iWidth = thisFBTEST->fileParams.inResolution.width;
	unsigned int iHeight = thisFBTEST->fileParams.inResolution.height;
	unsigned int oWidth = thisFBTEST->fileParams.inResolution.width;
	unsigned int oHeight = thisFBTEST->fileParams.inResolution.height;
	unsigned int iFormat = thisFBTEST->fileParams.inputFormat;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	fb_fd = FB->fb_fd;
	vinfo = &(FB->fb_vinfo);

	frameSize  = getFrameSize(iWidth,iHeight,iFormat);

	if ((imgPtr = fopen(thisFBTEST->fileParams.filenamePath,"rb")) == NULL) {
		FBTEST_MSG_DEBUG("[Move] Cannot open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}
	if (writeback_flag == TRUE) {
		result = writeback_init_start(frameSize);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		drawBG();
		//allocate PMEM buffer
		if (allocMEM(FB->fb_size)) {
			FBTEST_MSG_DEBUG("[Move] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}

	//read image in buffer
	bytesRead  = fread(MEM->mem_buf, 1, frameSize, imgPtr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[Move] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}

	fclose(imgPtr);

	//Set Overlay
	overlay.src.width  = iWidth;
	overlay.src.height = iHeight;
	overlay.src.format = iFormat;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}

	overlay.id = MSMFB_NEW_REQUEST;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = iWidth;
	overlay.src_rect.h = iHeight;
	overlay.dst_rect.x = startX;
	overlay.dst_rect.y = startY;
	overlay.dst_rect.w = oWidth;
	overlay.dst_rect.h = oHeight;
	overlay.dst_rect.w = iWidth;
	overlay.dst_rect.h = iHeight;
	overlay.flags = 0;



	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[Move] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			   __LINE__, overlay_id);
		showError(-OVERLAY_SET_FAILED,0);
	}

	overlay_id = overlay.id;
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;
	thisFBTEST->imgOffset.startX = startX;
	thisFBTEST->imgOffset.startY = startY;
	thisFBTEST->imgOffset.endX = endX;
	thisFBTEST->imgOffset.endY = endY;
	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("\n[Move] MSMFB_OVERLAY_PLAY failed! (Line %d)\n",
				__LINE__);
		return -OVERLAY_PLAY_FAILED;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	add_delay_in_user_mode();

	switch (traversal) {
		case USER_DEFINED:

			VPRINT(verbose, "User Defined Move: [%dx%d] --> [%dx%d]\n",
				startX, startY, endX, endY);
			traverseImage(startX, startY, endX, endY, &overlay, &fbdata);
		break;
		case HORIZONTAL:
			endX = vinfo->xres - iWidth;
			for (i = 0; i <= (vinfo->yres - iHeight); i = i + MOVE_SKIP_INTERVAL) {
				traverseImage(0, i, endX, i, &overlay, &fbdata);
			}
		break;

		case VERTICAL:
			endY = vinfo->yres - iHeight;
			for (i = 0; i <= (vinfo->xres - iWidth); i = i + MOVE_SKIP_INTERVAL) {
				traverseImage(i, 0, i, endY, &overlay, &fbdata);
			}
		break;

		case BOUNDARY:
			endX = vinfo->xres - iWidth;
			traverseImage(0, 0, endX, 0, &overlay, &fbdata);

			startX = vinfo->xres - iWidth;
			endX = vinfo->xres - iWidth;
			endY = vinfo->yres - iHeight;
			traverseImage(startX, 0, endX, endY, &overlay, &fbdata);

			startX = vinfo->xres - iWidth;
			startY = vinfo->yres - iHeight;
			endY = vinfo->yres - iHeight;
			traverseImage(startX, startY, 0, endY, &overlay, &fbdata);

			startY = vinfo->yres - iHeight;
			traverseImage(0, startY, 0,0, &overlay, &fbdata);
		break;

		case DIAGONALS:
			endX = vinfo->xres - iWidth;
			endY = vinfo->yres - iHeight;
			traverseImage(0, 0, endX, endY, &overlay, &fbdata);
			startX = vinfo->xres - iWidth;
			endY = vinfo->yres - iHeight;

			traverseImage(startX, 0, 0, endY, &overlay, &fbdata);

			break;

		case RANDOM:
			//Take random values between 1/8 and oWidth
			startX = rand_range(0, vinfo->xres - iWidth);
			startY = rand_range(0, vinfo->yres - iHeight);
			endX = rand_range(0,vinfo->xres - iWidth);
			endY = rand_range(0,vinfo->yres - iHeight);
			VPRINT(verbose, "Random Move: [%dx%d] --> [%dx%d]\n",
				startX, startY, endX, endY);
			traverseImage(startX, startY, endX, endY, &overlay, &fbdata);
		break;
	}

	if (interactive) {
		interactiveDelay();
	} else {
		add_delay_in_user_mode();
	}
	if (writeback_flag == TRUE) {
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST; 	//UTF: added for cleanup code addition.

	return TEST_RESULT_PASS;


}

int traverseImage(unsigned int startX, unsigned int startY,
		unsigned int endX, unsigned int endY, struct mdp_overlay * ptrOverlay,
		struct msmfb_data *fbdata) {
	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	struct fbtest_params *thisFBTEST;
	int loopCnt = 0;
	int xOrdinate = 0, yOrdinate = 0;
	int i = 0, factor = 0;
	int result = TEST_RESULT_FAIL;
	thisFBTEST = &FBTEST;
	memcpy(&overlay, ptrOverlay, sizeof(struct mdp_overlay));
	unsigned int iWidth;
	unsigned int iHeight;
	unsigned int iFormat;
	int tmp_startX = 0,tmp_startY = 0, tmp_endX = 0, tmp_endY = 0;
	int frameSize = 0;
	FILE *imgPtr = NULL;
	size_t bytesRead = 0;
	iWidth =  thisFBTEST->fileParams.inResolution.width;
	iHeight = thisFBTEST->fileParams.inResolution.height;
	iFormat = thisFBTEST->fileParams.inputFormat;
	frameSize  = getFrameSize(iWidth, iHeight, iFormat);
	//Get file params

	if (startX < endX)
		xOrdinate = endX -startX;
	else
		xOrdinate = startX -endX;

	if (startY < endY)
		yOrdinate = endY -startY;
	else
		yOrdinate = startY -endY;

	factor = gcd(xOrdinate, yOrdinate);

	FBTEST_MSG_DEBUG("FACTOR::::: %d start x,y:: %d,%d end x,y:: %d,%d\n",
		factor, startX, startY, endX, endY);

	overlay.dst_rect.x = startX;
	overlay.dst_rect.y = startY;

	loopCnt = factor -1;
	for (i = 0; i <= loopCnt; i++) {

		if (startX < endX)
			overlay.dst_rect.x += (((endX-startX)/factor));
		else
			overlay.dst_rect.x -= (((startX-endX)/factor));

		if (startY < endY)
			overlay.dst_rect.y += (((endY-startY)/factor));
		else
			overlay.dst_rect.y -= (((startY-endY)/factor));



		FBTEST_MSG_DEBUG(" dst off: x,y:: %d,%d\n",
			overlay.dst_rect.x, overlay.dst_rect.y);
		overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

		if (overlay_id < 0) {
			FBTEST_MSG_DEBUG("[Move] MSMFB_OVERLAY_SET failed!\
				line=%d err=%d\n", __LINE__, overlay_id);
				showError(-OVERLAY_SET_FAILED,0);
		}
		if ((writeback_flag == TRUE)) {
			result = writeback_queueBuff(fbdata, frameSize);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		}
		overlay_id = overlay.id;
		curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
		ovdata.id = overlay_id;
		ovdata.data.flags = 0;
		ovdata.data.offset = 0;
		ovdata.data.memory_id = MEM->mem_fd;

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			FBTEST_MSG_DEBUG("\n[Move] MSMFB_OVERLAY_PLAY failed! (Line %d)\n",
				__LINE__);
			return -OVERLAY_PLAY_FAILED;
		}

		tmp_startX = thisFBTEST->imgOffset.startX;
		tmp_startY = thisFBTEST->imgOffset.startY;
		tmp_endX = thisFBTEST->imgOffset.endX;
		tmp_endY = thisFBTEST->imgOffset.endY;

		thisFBTEST->imgOffset.startX = overlay.dst_rect.x;
		thisFBTEST->imgOffset.startY = overlay.dst_rect.y;
		thisFBTEST->imgOffset.endX = endX;
		thisFBTEST->imgOffset.endY = endY;

		result = doDisplayUpdate();
		thisFBTEST->imgOffset.startX =tmp_startX;
		thisFBTEST->imgOffset.startY = tmp_startY;
		thisFBTEST->imgOffset.endX = tmp_endX;
		thisFBTEST->imgOffset.endY = tmp_endY;

		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		if ((writeback_flag == TRUE)) {
			result = writeback_dqueue_dump(fbdata, frameSize, "moveTest");
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		}
	}
	return TEST_RESULT_PASS;
}

int rotateTest(void)
{
	int result = TEST_RESULT_PASS;
	unsigned int i = 0;
	unsigned int j = 0;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);

	unsigned int cmdRotFlip = thisFBTEST->rotateFlip;
	unsigned int cmdRotDeg = thisFBTEST->rotateDegree;

	result = openRotator();
	if (result < 0) {
		return -ROT_OPEN_FAILED;
	}
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	VPRINT(verbose, "\nPerforming Rotate Test...\n");

	switch(thisFBTEST->testMode) {
		case USER:
			result = doRotate(cmdRotFlip, cmdRotDeg);
			break;
		case AUTO:
			for (i = ROT_0; i <= ROT_270; i++) {
				if (i == ROT_0)
					VPRINT(verbose, "ROTATING for 0 degree:\n");
				if (i == ROT_90)
					VPRINT(verbose, "\nROTATING for 90 degree:\n");
				if (i == ROT_180)
					VPRINT(verbose, "\nROTATING for 180 degree:\n");
				if (i == ROT_270)
					VPRINT(verbose, "\nROTATING for 270 degree:\n");
				cmdRotDeg = i;
				for (j = FLIP_NOP; j <= FLIP_LRUD; j++) {
					cmdRotFlip = j;
					if (j == FLIP_NOP)
						VPRINT(verbose, " Flipping for NOFLIP:\n");
					if (j == FLIP_LR)
						VPRINT(verbose, " Flipping for LR:\n");
					if (j == FLIP_UD)
						VPRINT(verbose, " Flipping for UD:\n");
					if (j == FLIP_LRUD)
						VPRINT(verbose, " Flipping for LR+UD:\n");

					thisFBTEST->rotateFlip = cmdRotFlip;
					thisFBTEST->rotateDegree = cmdRotDeg;
					result = doRotate(cmdRotFlip, cmdRotDeg);
					if (result == TEST_RESULT_FAIL)
						break;
				}
				if (result == TEST_RESULT_FAIL)
				break;
			}

			break;

		default:
			VPRINT(verbose, "Error: Not a valid mode of operation\n");
			break;
	}

	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "Rotate test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec),
		(time_stop.tv_usec-time_start.tv_usec));

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Rotate Test: Pass \n");
	} else
		VPRINT(verbose, "Rotate Test: Fail \n");

	return result;

}

int doMdssRotate(unsigned int rotations, struct mdp_overlay *overlay,
			int dstOffset)
{
	struct fbtest_params *thisFBTEST = &FBTEST;
	struct msmfb_overlay_data ovdata;
	unsigned int ovSessionId = overlay->id;

	/* use overlay interface for rotator in mdss */
	overlay->flags = rotations;

	VPRINT(verbose, "[ROTATE] Rotating using MDSS rotator\n");

	overlay->id = MSMFB_NEW_REQUEST;
	overlay->flags |= MDSS_MDP_ROT_ONLY;

	if (overlay->flags & MDP_ROT_90) {
		/* Driver expects actual destination w, h */
		swap(overlay->dst_rect.w, overlay->dst_rect.h);
	}

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, overlay)) {
		FBTEST_MSG_DEBUG("[ROTATE] MSMFB_OVERLAY_SET failed! line=%d\n",
				__LINE__);
		return -OVERLAY_SET_FAILED;
	}

	curr_overlay_id[1] = overlay->id;   //UTF: added for cleanup code addition.
	memset(&ovdata, 0, sizeof(ovdata));
	ovdata.id = overlay->id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	ovdata.dst_data.flags = 0;
	ovdata.dst_data.offset = dstOffset;
	ovdata.dst_data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[ROTATE] MSMFB_OVERLAY_PLAY failed line=%d\n",
				__LINE__);
		return -OVERLAY_PLAY_FAILED;
	}

	/* prepare overlay for displaying rotated 90 degree output */
	overlay->id = ovSessionId;
	if (overlay->flags & MDP_ROT_90) {
		overlay->flags = MDP_SOURCE_ROTATED_90;
		/* Destination of rotator becomes the source for MDP
		We assume here that both were the same to begin with */
		swap(overlay->src.width, overlay->src.height);
		swap(overlay->src_rect.w, overlay->src_rect.h);
	} else {
		overlay->flags = 0;
	}

	return TEST_RESULT_PASS;
}

int getRotatorDstFormat(unsigned int format, unsigned int cmdRotDeg)
{
	unsigned int rotDstFormat;

	if (mdp_version >= MDP_V5) {
		if ((cmdRotDeg == ROT_90 || cmdRotDeg == ROT_270) &&
			((mdp_rev < MDSS_MDP_HW_REV_102) ||
			lookup_val(format_name_table, format, 0))) {
			switch (format) {
			case MDP_RGB_565:
			case MDP_BGR_565:
				rotDstFormat = MDP_RGB_888;
				break;
			case MDP_Y_CBCR_H2V2_VENUS:
			case MDP_Y_CBCR_H2V2:
			case MDP_Y_CB_CR_H2V2:
			case MDP_Y_CR_CB_GH2V2:
			case MDP_Y_CR_CB_H2V2:
				rotDstFormat = MDP_Y_CRCB_H2V2;
				break;
			default:
				rotDstFormat = format;
				break;
			}
		} else {
				rotDstFormat = format;
		}
	} else {
		if (cmdRotDeg == ROT_90 || cmdRotDeg == ROT_270) {
			switch (format) {
			case MDP_Y_CRCB_H2V1:
				rotDstFormat = MDP_Y_CRCB_H1V2;
				break;
			case MDP_Y_CBCR_H2V1:
				rotDstFormat = MDP_Y_CBCR_H1V2;
				break;
			case MDP_YCRYCB_H2V1:
				rotDstFormat = MDP_Y_CRCB_H1V2;
			default:
				rotDstFormat = format;
				break;
			}
		} else {
			switch (format) {
			case MDP_Y_CRCB_H2V2_TILE:
			case MDP_Y_CR_CB_H2V2:
			case MDP_Y_CR_CB_GH2V2:
				rotDstFormat = MDP_Y_CRCB_H2V2;
				break;
			case MDP_Y_CB_CR_H2V2:
			case MDP_Y_CBCR_H2V2_TILE:
				rotDstFormat = MDP_Y_CBCR_H2V2;
				break;
			case MDP_YCRYCB_H2V1:
				rotDstFormat = MDP_Y_CRCB_H2V1;
				break;
			default:
				rotDstFormat = format;
				break;
			}
		}
	}
	return rotDstFormat;
}

int doMdp4Rotate(unsigned int rotations, struct mdp_overlay *overlay,
			int dstOffset)
{
	struct fbtest_params *thisFBTEST = &FBTEST;;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;

	unsigned int iHeight = overlay->src.height;
	unsigned int iWidth = overlay->src.width;
	unsigned int iFormat = overlay->src.format;
	unsigned int oHeight = iHeight;
	unsigned int oWidth = iWidth;

	int result, rotateLoop = 0;
	int frameSize;
	void *out_virt = NULL;
	char filename[100];
	frameSize  = getFrameSize(iWidth,iHeight,iFormat);

	if (rotations & MDP_ROT_90) {
		oWidth = iHeight;
		oHeight = iWidth;
	}
	VPRINT(verbose, "[ROTATE MDP4] (W=%d,H=%d) format = %d frameSize = %d \n",
		iWidth, iHeight, iFormat, frameSize);

	/* Check if inuput TILE image resolution is multiples of 64x32 blocks
	 * checking only width because even if height is not multiple of 32,
	 * padding after luma pixel will take care of problem */
	if ((iFormat == MDP_Y_CRCB_H2V2_TILE) ||
			(iFormat == MDP_Y_CBCR_H2V2_TILE)) {
		if ((iWidth % 64) != 0) {
			VPRINT(verbose,"[ROTATE] TILE width not mult of 64\n");
			return TEST_RESULT_SKIP;
		}
	}
	VPRINT(verbose, "[ROTATE MDP4]3 (W=%d,H=%d) format = %d frameSize = %d \n",
		iWidth, iHeight, iFormat, frameSize);
	//Set up iinfo and dinfo structures for rotation ioctls.
	iinfo.downscale_ratio = 0;//Need to support rotator downscale in future
	iinfo.src.height = iHeight;
	iinfo.src.width  = iWidth;
	iinfo.src.format = iinfo.dst.format = iFormat;
	dinfo.src.memory_id = dinfo.dst.memory_id = MEM->mem_fd;
	dinfo.src.flags =0;
	dinfo.dst.flags =0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = dstOffset;
	out_virt = (unsigned char *)MEM->mem_buf + dinfo.dst.offset;

        iinfo.src_rect.x = 0;
        iinfo.src_rect.y = 0;
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;
	iinfo.dst.height = oWidth;
	iinfo.dst.width = oHeight;

	memcpy(out_virt, MEM->mem_buf, frameSize);
	iinfo.src_rect.h = iinfo.src.height;
	iinfo.src_rect.w = iinfo.src.width;
	iinfo.rotations = rotations;

	iinfo.enable = 1;
	iinfo.secure = 0;
	result = ioctl(thisFBTEST->rot_fd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		FBTEST_MSG_DEBUG("[ROTATE MDP4] rotator start ioctl failed result = %d\n", result);
		if (cleanup_overlay())
			return -OVERLAY_UNSET_FAILED;
		return -ROT_START_FAILED;
	} else {
		VPRINT(verbose, "[ROTATE MDP4] MSM_ROTATOR_IOCTL_START result = %d\n", result);
	}
	dinfo.session_id = iinfo.session_id;

	result = ioctl(thisFBTEST->rot_fd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);

	if (result < 0) {
		FBTEST_MSG_DEBUG("[ROTATE MDP4] rotator ioctl failed result = %d\n", result);
		if (cleanup_overlay()) { /* overlay cleanup */
			return -OVERLAY_UNSET_FAILED;
		}
		VPRINT(verbose, "[ROTATE MDP4] MSM_ROTATOR_IOCTL_ROTATE result = %d\n", result);
		/* rotator cleanup */
		showError(-ROT_FAILED, 0);
		result = ioctl(thisFBTEST->rot_fd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);

		if (result < 0)
			FBTEST_MSG_DEBUG("[ROTATE MDP4] rotator finish ioctl failed result = %d\n", result);
			return -ROT_FAILED;
		VPRINT(verbose, "[ROTATE MDP4] MSM_ROTATOR_IOCTL_FINISH result = %d\n", result);
	}
	memcpy(MEM->mem_buf, out_virt, frameSize);
	if (strncmp(thisFBTEST->dumpPath, "", sizeof(thisFBTEST->dumpPath))) {
		snprintf(filename, sizeof(filename), "%s/rotator_dump.raw", thisFBTEST->dumpPath);
		dump_img(MEM->mem_buf, frameSize, filename);
	}

	/* set destination format */
	overlay->src.format = iinfo.dst.format;
	overlay->src.width  = oWidth;
	overlay->src.height = oHeight;
	overlay->src_rect.w = oWidth;
	overlay->src_rect.h = oHeight;
	overlay->flags = 0;
	if (oWidth <= FB->fb_vinfo.xres) {
		overlay->dst_rect.w = oWidth;
	} else {
		overlay->dst_rect.w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay->src.format, 0))
			overlay->flags = MDP_OV_PIPE_SHARE;
	}
	if (oHeight <= FB->fb_vinfo.yres) {
		overlay->dst_rect.h = oHeight;
	} else {
		overlay->dst_rect.h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay->src.format, 0))
			overlay->flags = MDP_OV_PIPE_SHARE;
	}
	return TEST_RESULT_PASS;
}

int doRotate(unsigned int cmdRotFlip, unsigned int cmdRotDeg)
{
	int overlay_id, fb_fd;
	int result = TEST_RESULT_PASS;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	int rotations = 0;
	//writeback structure
	struct msmfb_data fbdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	thisFBTEST = &FBTEST;

	//Get file params
	unsigned int iHeight = thisFBTEST->fileParams.inResolution.height;
	unsigned int iWidth = thisFBTEST->fileParams.inResolution.width;
	unsigned int iFormat = thisFBTEST->fileParams.inputFormat;
	unsigned int oWidth = iWidth;
	unsigned int oHeight = iHeight;

	int frameSize, rotatedOffset;
	FILE *imgPtr = NULL;
	size_t bytesRead = 0;
	int rotDstSize = 0;
	unsigned int rotDstFormat;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	fb_fd = FB->fb_fd;
	vinfo = &(FB->fb_vinfo);

	//calculate framesize
	frameSize  = getFrameSize(iWidth, iHeight, iFormat);
	if ((imgPtr = fopen(thisFBTEST->fileParams.filenamePath,"rb")) == NULL) {
		FBTEST_MSG_DEBUG("[ROTATE] Cannot open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}

	rotDstFormat = getRotatorDstFormat(iFormat, cmdRotDeg);
	rotDstSize = getFrameSize(iWidth, iHeight, rotDstFormat);

	if (writeback_flag == TRUE) {
		result = writeback_init_start(frameSize + rotDstSize);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		/* Do not call drawBG() for 8226 targets as it would fail the
		rotation usecase with YUV planar input due to insufficient SMPs
		*/
		if(!(mdp_version == MDP_V5 && MDSS_GET_MAJOR(mdp_rev) == 1 &&
						MDSS_GET_MINOR(mdp_rev) == 1))
			drawBG();

		//allocate PMEM buffer
		if (allocMEM(frameSize + rotDstSize)) {
			FBTEST_MSG_DEBUG("[ROTATE] allocMEM failed! (Line %d)\n",__LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	//read image in buffer
	bytesRead  = fread(MEM->mem_buf, 1, frameSize, imgPtr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[ROTATE] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}

	fclose(imgPtr);

	//Set overlay so that we can show the input frame
	overlay.src.width  = iWidth;
	overlay.src.height = iHeight;
	overlay.src.format = iFormat;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.alpha = 0xFF;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}

	overlay.id = MSMFB_NEW_REQUEST;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = iWidth;
	overlay.src_rect.h = iHeight;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.flags = 0;
	if (iWidth <= FB->fb_vinfo.xres) {
		overlay.dst_rect.w = iWidth;
	} else {
		overlay.dst_rect.w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (iHeight <= FB->fb_vinfo.yres) {
		overlay.dst_rect.h = iHeight;
	} else {
		overlay.dst_rect.h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	VPRINT(verbose, "[ROTATE] format = %d frameSize = %d \n",
		iFormat, frameSize);
	VPRINT(verbose, "[ROTATE] SRC (X=%d,Y=%d)(W=%d,H=%d) \n",
		overlay.src_rect.x, overlay.src_rect.y, overlay.src_rect.w, overlay.src_rect.h);
	VPRINT(verbose, "[ROTATE] Dest(X=%d,Y=%d)(W=%d,H=%d) \n",
		overlay.dst_rect.x, overlay.dst_rect.y, overlay.dst_rect.w, overlay.dst_rect.h);


	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[ROTATE] MSMFB_OVERLAY_SET failed!\
			line=%d err=%d\n", __LINE__, overlay_id);
			return -OVERLAY_SET_FAILED;
	}
	VPRINT(verbose, "[ROTATE] MSMFB_OVERLAY_SET \n");

	overlay_id = overlay.id;
	curr_overlay_id[0] = overlay_id;
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[ROTATE] MSMFB_OVERLAY_PLAY failed!\
			line=%d err=%d\n", __LINE__, overlay_id);
		showError(-OVERLAY_PLAY_FAILED, 0);
		if (cleanup_overlay())
			return -OVERLAY_UNSET_FAILED;
	}

	if (thisFBTEST->testMode != AUTO) {
		VPRINT(verbose, "[ROTATE] MSMFB_OVERLAY_PLAY \n");
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		VPRINT(verbose, "[ROTATE] FBIOPAN_DISPLAY \n");
		VPRINT(verbose, "Displaying original image\n");
		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}

	switch (cmdRotDeg) {
	case ROT_270:
		rotations |= MDP_FLIP_LR|MDP_FLIP_UD;
	case ROT_90:
		rotations |= MDP_ROT_90;
		break;
	case ROT_180:
		rotations |= MDP_FLIP_LR|MDP_FLIP_UD;
	case ROT_0:
		break;
	default:
		VPRINT(verbose, "[ROTATE] Rotation degree is not proper\n");
		break;
	}
	switch (cmdRotFlip) {
	case FLIP_LR:
		rotations ^= MDP_FLIP_LR;
		break;
	case FLIP_UD:
		rotations ^= MDP_FLIP_UD;
		break;
	case FLIP_LRUD:
		rotations ^= MDP_FLIP_LR|MDP_FLIP_UD;
		break;
	case FLIP_NOP:
		break;
	default:
		VPRINT(verbose, "[ROTATE] Rotation Flip value is not proper\n");
		break;
	}

	rotatedOffset = frameSize;
	VPRINT(verbose, "[ROTATE] mdp_version = %d  \n", mdp_version);
	if (mdp_version < MDP_V5) {
		result = doMdp4Rotate(rotations, &overlay, rotatedOffset);
		VPRINT(verbose, "[ROTATE] doMdp4Rotate \n");
	} else {
		result = doMdssRotate(rotations, &overlay, rotatedOffset);
		VPRINT(verbose, "[ROTATE] doMdssRotate \n");
	}

	if (result < 0)
		return result;
	else if (result == TEST_RESULT_SKIP)
		rotatedOffset = 0;

	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;
	overlay.id = MSMFB_NEW_REQUEST;

	/* Down Scale Rotated surface when surface size is larger than panel */
	if (overlay.dst_rect.w > FB->fb_vinfo.xres)
		overlay.dst_rect.w = FB->fb_vinfo.xres;

	if(overlay.dst_rect.h > FB->fb_vinfo.yres)
		overlay.dst_rect.h = FB->fb_vinfo.yres;

	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[ROTATE] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	VPRINT(verbose, "[ROTATE] MSMFB_OVERLAY_SET \n");

	if (writeback_flag == TRUE) {
		result = writeback_queueBuff(&fbdata, frameSize + rotDstSize);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}

	overlay.src.format = rotDstFormat;

	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[ROTATE] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
				__LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	overlay_id = overlay.id;
	curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = rotatedOffset;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		FBTEST_MSG_DEBUG("[ROTATE] MSMFB_OVERLAY_PLAY failed! line=%d err=%d\n",
			__LINE__, overlay_id);
		return -OVERLAY_PLAY_FAILED;
	}

	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	if (writeback_flag == TRUE) {
		result = writeback_dqueue_dump(&fbdata,
					frameSize + rotDstSize, "rotateTest");
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {

		VPRINT(verbose, "Displaying rotated image\n");
		if (interactive) {
			interactiveDelay();
		} else {
			add_delay_in_user_mode();
		}
	}

	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	return TEST_RESULT_PASS;
}

int openRotator(void)
{
	int result = 0;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	if (mdp_version >= MDP_V5)
		return result;

	/* open rotator */
	thisFBTEST->rot_fd = open("/dev/msm_rotator", O_RDWR);
	if (thisFBTEST->rot_fd < 0) {
		fprintf(stderr, "Could not open rotator device!\n");
		result = -1;
	}
	return result;

}

int scaleTest(void)
{
	int result = TEST_RESULT_PASS;
	unsigned int i = 0;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);

	//Get file params
	unsigned int iWidth = thisFBTEST->fileParams.inResolution.width;
	unsigned int iHeight = thisFBTEST->fileParams.inResolution.height;
	unsigned int oWidth = thisFBTEST->outResolution.width;
	unsigned int oHeight = thisFBTEST->outResolution.height;
	unsigned int iFormat = thisFBTEST->fileParams.inputFormat;
	struct timeval time_start, time_stop;
	gettimeofday(&time_start, NULL);
	VPRINT(verbose, "\nPerforming Scale Test...\n");
	//Switch as per mode
	VPRINT(verbose, "\nPerforming Scale Test Mode %d\n", thisFBTEST->testMode);
	switch(thisFBTEST->testMode) {

		case USER:
			VPRINT(verbose, "\nPerforming Scale Test USER \n");
			if (0 == oWidth)
				oWidth  = vinfo->xres;
			if (0 == oHeight)
				oHeight  = vinfo->yres;

			result = doScale(iWidth, iHeight, oWidth, oHeight, iFormat);
		break;

		case AUTO:
			VPRINT(verbose, "\nPerforming Scale Test AUTO \n");
			//For Auto, we will do up/down and arbitrary scaling till full extent.
			srand(1);
			for (i = UPSCALE; i <= ARBITRARY; i++) {

				if (i == UPSCALE) {
					oWidth  = (((iWidth * getMaxUpScale(mdp_version)) < vinfo->xres) ?
						(iWidth * getMaxUpScale(mdp_version)) : vinfo->xres);
					oHeight  = (((iHeight * getMaxUpScale(mdp_version)) < vinfo->yres) ?
						(iHeight * getMaxUpScale(mdp_version)) : vinfo->yres);
					VPRINT(verbose, "UPSCALING TO: %dx%d\n",oWidth, oHeight);
					thisFBTEST->outResolution.width = oWidth;
					thisFBTEST->outResolution.height = oHeight;
					result = doScale(iWidth, iHeight, oWidth, oHeight, iFormat);
				}
				if (i == DOWNSCALE) {
					oWidth  =  (iWidth/getMaxDownScale(mdp_version));
					oHeight  = (iHeight/getMaxDownScale(mdp_version));
					VPRINT(verbose, "DOWNSCALING TO: %dx%d\n",oWidth, oHeight);
					thisFBTEST->outResolution.width = oWidth;
					thisFBTEST->outResolution.height = oHeight;
					result = doScale(iWidth, iHeight, oWidth, oHeight, iFormat);
				}

				if (i == ARBITRARY) {

					oWidth  = (((iWidth*8) < vinfo->xres) ? (iWidth*8) :vinfo->xres);
					oHeight  = (((iHeight*8) < vinfo->yres) ? (iHeight*8) :vinfo->yres);

					//Take random values between 1/8 and oWidth
					oWidth = rand_range(iWidth/8, oWidth);
					oHeight = rand_range(iHeight/8, oHeight);

					thisFBTEST->outResolution.width = oWidth;
					thisFBTEST->outResolution.height = oHeight;
					printf("ARBITRARY SCALING TO: %dx%d\n", oWidth, oHeight);
					result = doScale(iWidth, iHeight, oWidth, oHeight, iFormat);
				}
			}
			break;

		default:
			VPRINT(verbose, "Error: Not a valid mode of operation\n");
			break;
	}

	gettimeofday(&time_stop, NULL);
	VPRINT(verbose, "Scale test over %ld second %ld microsecond\n",
		(time_stop.tv_sec-time_start.tv_sec),
		(time_stop.tv_usec-time_start.tv_usec));

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nScale Test: PASS\n");
	} else
		VPRINT(verbose, "\nScale Test: FAIL\n");
	return result;

}

int setDecimationFactor(struct mdp_overlay* overlay) {
	float horDscale = 0, verDscale = 0;
	int minHorDeci = 0;
	struct fb_var_screeninfo *vinfo;
	vinfo = &(FB->fb_vinfo);
	if (overlay->src_rect.w > 2048) {
		//If the client sends us something > what a layer mixer supports
		//then it means it doesn't want to use split-pipe but wants us to
		//decimate. A minimum decimation of 2 will ensure that the width is
		//always within layer mixer limits.
		minHorDeci = 2;
	}
	verDscale = ceilf((float)overlay->src_rect.h /
			(float)overlay->dst_rect.h);

	horDscale = ceilf((float)overlay->src_rect.w /
			(float)overlay->dst_rect.w);

	//Next power of 2, if not already
	horDscale = powf(2.0f, ceilf(log2f(horDscale)));
	verDscale = powf(2.0f, ceilf(log2f(verDscale)));

	//Since MDP can do 1/4 dscale and has better quality, split the task
	//between decimator and MDP downscale
	horDscale /= 4.0f;
	verDscale /= 4.0f;

	if (horDscale < minHorDeci)
		horDscale = minHorDeci;
	if ((int)horDscale)
		overlay->horz_deci = (int)log2f(horDscale);
	else
		overlay->horz_deci = 0;
	if ((int)verDscale)
		overlay->vert_deci = (int)log2f(verDscale);
	else
		overlay->vert_deci = 0;
	if ((overlay->dst_rect.w > vinfo->xres) || (overlay->dst_rect.h > vinfo->yres))
	{
		FBTEST_MSG_DEBUG("\n[SCALE] Destination Resolution OutofBound! (Line %d)\n", __LINE__);
		return -RES_OUT_OF_BOUND;
	}
	return TEST_RESULT_PASS;
}

//This function does up/down and arbitrary scaling.
int doScale(unsigned int iWidth, unsigned int iHeight, unsigned int oWidth,
    unsigned int oHeight, unsigned int iFormat)
{

	int overlay_id, fb_fd, result = TEST_RESULT_PASS;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	struct msmfb_data fbdata;//writeback structure
	int count = 0;
	thisFBTEST = &FBTEST;
	int frameSize;
	FILE *imgPtr = NULL;
	size_t bytesRead = 0;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	fb_fd = FB->fb_fd;
	vinfo = &(FB->fb_vinfo);

	frameSize  = getFrameSize(iWidth,iHeight,iFormat);

	if ((imgPtr = fopen(thisFBTEST->fileParams.filenamePath,"rb")) == NULL) {
		FBTEST_MSG_DEBUG("[SCALE] Cannot open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);
		return -FOPEN_FAILED;
	}

	if (writeback_flag == TRUE) {
		thisFBTEST->isStepScale = 1;
		result = writeback_init_start(frameSize);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	} else {
		drawBG();
		//allocate PMEM buffer
		if (allocMEM(frameSize)) {
			FBTEST_MSG_DEBUG("[SCALE] Buffer Allocation failed! (Line %d)\n", __LINE__);
			return -PMEMALLOC_FAILED;
		}
	}
	//read image in buffer
	bytesRead  = fread(MEM->mem_buf, 1, frameSize, imgPtr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[SCALE] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}

	fclose(imgPtr);

	//Set Overlay
	overlay.src.width  = iWidth;
	overlay.src.height = iHeight;
	overlay.src.format = iFormat;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.alpha = 0xFF;

	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;
	} else {
		overlay.is_fg = 1;
	}

	overlay.id = MSMFB_NEW_REQUEST;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = iWidth;
	overlay.src_rect.h = iHeight;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = oWidth;
	overlay.dst_rect.h = oHeight;
	overlay.flags = 0;
	if (lookup_val(format_name_table, iFormat, 0) &&
		((iWidth > oWidth) || (iHeight > oHeight)) &&
		(mdp_version <= MDP_V4_1)) {
		overlay.flags = MDP_OV_PIPE_SHARE;
	}
	//Check if WxH are valid as per MDP4 support
	if (is_valid_overlay(&overlay))
	{
		FBTEST_MSG_DEBUG("\n[SCALE] Resolution OutofBound! (Line %d)\n",	__LINE__);
		return -RES_OUT_OF_BOUND;
	}

	if (overlay.src_rect.w > vinfo->xres) {
		iWidth = vinfo->xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (overlay.src_rect.h > vinfo->yres) {
		iHeight = vinfo->yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	overlay.dst_rect.w = iWidth;
	overlay.dst_rect.h = iHeight;


	VPRINT(verbose, "Scaling Image from %dx%d to %dx%d...\n", iWidth, iHeight, oWidth, oHeight);

	//This code makes the loop  to start with (first-1) loop. Required for doing
	//up/down/arbitrary scaling using common loop

	if (iHeight < oHeight)
		overlay.dst_rect.h = iHeight - 1;
	if (iHeight > oHeight)
		overlay.dst_rect.h = iHeight + 1;
	if (iWidth < oWidth)
		overlay.dst_rect.w = iWidth - 1;
	if (iWidth > oWidth)
		overlay.dst_rect.w = iWidth + 1;

	//Loop to up/down/arbitrary scale operation in steps
	while ((overlay.dst_rect.w != oWidth) ||
		(overlay.dst_rect.h != oHeight)) {

		if (overlay.dst_rect.w < oWidth)
				overlay.dst_rect.w++;
		if (overlay.dst_rect.w > oWidth)
				overlay.dst_rect.w--;
		if (overlay.dst_rect.h < oHeight)
				overlay.dst_rect.h++;
		if (overlay.dst_rect.h > oHeight)
				overlay.dst_rect.h--;

		if(setDecimationFactor(&overlay) != TEST_RESULT_PASS){
			result = TEST_RESULT_FAIL;
			break;
		}
		overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
		if (overlay_id < 0) {
			FBTEST_MSG_DEBUG("[Scale] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
				__LINE__, overlay_id);
			showError(-OVERLAY_SET_FAILED,0);
			result = TEST_RESULT_FAIL;
			break;
		}
		if ((writeback_flag == TRUE)) {
			if (thisFBTEST->isStepScale && overlay.dst_rect.w == oWidth &&
				overlay.dst_rect.h == oHeight) {
				result = writeback_queueBuff(&fbdata, frameSize);
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			}
			if (thisFBTEST->isStepScale == 0) {
				result = writeback_queueBuff(&fbdata, frameSize);
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			}
		}
		overlay_id = overlay.id;
		curr_overlay_id[0] = overlay_id;   //UTF: added for cleanup code addition.
		ovdata.id = overlay_id;
		ovdata.data.flags = 0;
		ovdata.data.offset = 0;
		ovdata.data.memory_id = MEM->mem_fd;

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
			FBTEST_MSG_DEBUG("\n[Scale] MSMFB_OVERLAY_PLAY failed! (Line %d)\n",
					__LINE__);
			return -OVERLAY_PLAY_FAILED;
		}

		thisFBTEST->outResolution.height = overlay.dst_rect.h;
		thisFBTEST->outResolution.width = overlay.dst_rect.w;
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;

		//If step is enabled, incre/decre-ment dst h,w
		if (thisFBTEST->isStepScale) {
			if ((overlay.dst_rect.w != oWidth) ||
				(overlay.dst_rect.h != oHeight)) {

				VPRINT(verbose, "Scaling in single step...\n");

				if (iHeight < oHeight)
					overlay.dst_rect.h = oHeight - 1;
				if (iHeight > oHeight)
					overlay.dst_rect.h = oHeight + 1;
				if (iWidth < oWidth)
					overlay.dst_rect.w = oWidth - 1;
				if (iWidth > oWidth)
					overlay.dst_rect.w = oWidth + 1;

			}

			if (interactive) {
				interactiveDelay();
			} else {
				add_delay_in_user_mode();
			}
		}
		if (writeback_flag == TRUE) {
			if (thisFBTEST->isStepScale && overlay.dst_rect.w == oWidth &&
				overlay.dst_rect.h == oHeight) {
				result = writeback_dqueue_dump(&fbdata, frameSize, "scaleTest");
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			}
			if (thisFBTEST->isStepScale == 0) {
				result = writeback_dqueue_dump(&fbdata, frameSize, "scaleTest");
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			}
		}
	}
	if (writeback_flag == TRUE) {
		result = writeback_stop_terminate(&fbdata);
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}
	if (cleanup_overlay())
		return -OVERLAY_UNSET_FAILED;

	curr_overlay_id[0] = MSMFB_NEW_REQUEST;	/* UTF: added for cleanup code addition. */

	return result;
}
int updateCursorPos(struct fb_cursor *cursor_control,
    unsigned int col, unsigned int row)
{
	int result;
	(*cursor_control).image.dx = col;
	(*cursor_control).image.dy = row;
	(*cursor_control).set = FB_CUR_SETPOS;
	result = ioctl(FB->fb_fd, MSMFB_CURSOR, cursor_control);
	if (result < 0) {
		FBTEST_MSG_DEBUG("\n[HWCURSOR] MSMFB_CURSOR failed! (Line %d)\n",\
					__LINE__);
		return -MSMFB_CURSOR_FAILED;
	}
	return TEST_RESULT_PASS;
}
int hwCursorTest(void)
{
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	const unsigned char *pSrc;
	unsigned int dstOffset, row, col, delay, alpha;
	struct fb_cursor cursor_control;
	struct fb_image cursor_image =
	{
		.dx = 0,
		.dy = 0,
		.width = 64,
		.height = 64,
		.fg_color = 0x00000000,
		.bg_color = 0x00FFFFFF,
		.depth = 32,
		.data = NULL
	};
	char * tempBuf;
	VPRINT(verbose, "Hw Cursor test\n");
	int  fb_fd;
	size_t bytesRead = 0;
	struct fb_var_screeninfo *vinfo;
	FILE *fptr = NULL;
	char *buff;
	int size;

	int overlay_id, i;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	struct msmfb_data fbdata;

	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
		thisFBTEST->fileParams.inResolution.height,
		thisFBTEST->fileParams.inputFormat);
	VPRINT(verbose, "Cursor Image File Name %s Path %s\n",
	thisFBTEST->fileParams.filenamePath, thisFBTEST->path);
	VPRINT(verbose, "Cursor Image Size (W = %d, H = %d) format = % d \n",
		thisFBTEST->fileParams.inResolution.width,
		thisFBTEST->fileParams.inResolution.height,
		thisFBTEST->fileParams.inputFormat);
	if (strcmp(thisFBTEST->path, "dummy") != 0) {
		cursor_image.width  = thisFBTEST->fileParams.inResolution.width;
		cursor_image.height = thisFBTEST->fileParams.inResolution.height;
		cursor_image.depth  = getFormatBpp(thisFBTEST->fileParams.inputFormat) * 8;
		cursor_image.depth  = 32;
	} else {
		cursor_image.width  = 64;
		cursor_image.height = 64;
		cursor_image.depth  = 32;
	}
	VPRINT(verbose, "Cursor Image Size (W = %d, H = %d) format = % d \n",
		cursor_image.width, cursor_image.height, cursor_image.depth);
	alpha = 0x63;
	printf("  ARGB Mode cursor (alpha = 0x%X) .. ", alpha);
	buff = (char *)malloc(size);
	if (buff == NULL)
		showError(-MALLOC_FAILED, 1);

	vinfo = &(FB->fb_vinfo);
	fb_fd = FB->fb_fd;

	VPRINT(verbose, "\nPerforming hwcursor Test...\n");
	if (strcmp(thisFBTEST->path, "dummy") != 0) {
		fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
		printf("\nFile name:%s", thisFBTEST->fileParams.filenamePath);
		if (fptr == NULL) {
			FBTEST_MSG_DEBUG("[HWCURSOR] Cannot open raw Image file!\n");
			return -FOPEN_FAILED;
		}
		bytesRead  = fread(buff, 1, size, fptr);
		if (bytesRead == 0) {
			FBTEST_MSG_DEBUG("[HWCURSOR] fread failed! (Line %d)\n", __LINE__);
			return -FREAD_FAILED;
		}

		fclose(fptr);
		if (writeback_flag == TRUE) {
			result = writeback_init_start(size);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
		} else {
			drawBG();
			if (allocMEM(FB->fb_size)) {
				FBTEST_MSG_DEBUG("[FORMAT] Buffer Allocation failed! (Line %d)\n", __LINE__);
				return -PMEMALLOC_FAILED;
			}
		}
		for (i = 0 ; i <= 0xFF; i += 32) {
			cursor_image.data = buff;
			if (thisFBTEST->blend.isPixBlend == 1) {
				VPRINT(verbose, "\nPerforming PER PIXEL belnding\n");
				i = 0xFF;
			} else {
				alpha = i;
				cursor_image.fg_color = i << 24;
				printf("Alpha:%d", cursor_image.fg_color>>24);
			}
			memcpy(MEM->mem_buf, buff, (int)(size));
			result = docursor(&cursor_image, i);
		}
	} else {
		for (i = 32; i <= 0xFF; i += 32) {
			if (thisFBTEST->blend.isPixBlend == 1) {
				VPRINT(verbose, "\nPerforming PER PIXEL belnding\n");
				i = 0xFF;
				alpha = 0x63;
			} else {
				alpha = i;
				cursor_image.fg_color = alpha << 24;
				printf("Alpha:%d", cursor_image.fg_color>>24);
			}
			tempBuf = (char *)malloc(CURSOR_WIDTH*CURSOR_HEIGHT*4);
			if (tempBuf == NULL) {
				fprintf(stderr, "ERROR! Cannot allocate buffer for cursor data.\n");
				return TEST_RESULT_FAIL;
			}
			pSrc = cursor_data;
			dstOffset = 0;
			while (dstOffset < CURSOR_WIDTH*CURSOR_HEIGHT*4) {
				if ((dstOffset % 4) == 3) {
					*(tempBuf+dstOffset) = alpha;
					dstOffset++;
				} else {
					*(tempBuf+dstOffset) = *pSrc;
					pSrc++;
					dstOffset++;
				}
			}
			if (writeback_flag == TRUE) {
			result = writeback_init_start(size);
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			} else {
			drawBG();
			if (allocMEM(FB->fb_size)) {
				FBTEST_MSG_DEBUG("[FORMAT] Buffer Allocation failed! (Line %d)\n", __LINE__);
				return -PMEMALLOC_FAILED;
			}
			}
			cursor_image.data = tempBuf ;
			memcpy(MEM->mem_buf, buff, (int)(size));
			result = docursor(&cursor_image, i);
			free(tempBuf);
		}
	}
	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nHW cursor Test Pass\n");
	} else {
		VPRINT(verbose, "\nHW cursor Test FAIL\n");
	}
	return result;
}
int docursor(struct fb_image *cursor_image, int i)
{
	unsigned int dstOffset, delay, alpha;
	unsigned int row, col;
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	struct fb_cursor cursor_control;
	if (thisFBTEST->fBuffer != FB0) {
		printf("hwCursorTest only supported on FB0\n");
		return TEST_RESULT_SKIP;
	}
	delay = 1000;	/* microseconds */

	memset32(FB->fb_buf, 0xFFEEEEEE, FB->fb_vinfo.xres * FB->fb_vinfo.yres);
	cursor_control.set = FB_CUR_SETIMAGE | FB_CUR_SETPOS;
	cursor_control.enable = 1;
	cursor_control.image = (*cursor_image);
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	usleep(delay);
	result = ioctl(FB->fb_fd, MSMFB_CURSOR, &cursor_control);
	if (result < 0) {
		FBTEST_MSG_DEBUG("\n[HWCURSOR] MSMFB_CURSOR failed! (Line %d)\n",\
					__LINE__);
		return -MSMFB_CURSOR_FAILED;
	}
	row = thisFBTEST->coordinate.x;
	for (col = thisFBTEST->coordinate.y; col < FB->fb_vinfo.xres - CURSOR_WIDTH; col++) {
		thisFBTEST->imgOffset.startX = row;
		thisFBTEST->imgOffset.startY = col;
		thisFBTEST->imgOffset.endX = row + cursor_image->width;
		thisFBTEST->imgOffset.endY = col + cursor_image->height;
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		usleep(delay);
		updateCursorPos(&cursor_control, col, row);
	}
	usleep(delay);
	col = FB->fb_vinfo.xres - CURSOR_WIDTH;
	for (row = 0; row < FB->fb_vinfo.yres - CURSOR_HEIGHT; row++) {
		thisFBTEST->imgOffset.startX = row;
		thisFBTEST->imgOffset.startY = col;
		thisFBTEST->imgOffset.endX = row + cursor_image->width;
		thisFBTEST->imgOffset.endY = col + cursor_image->height;
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		usleep(delay);
		updateCursorPos(&cursor_control, col, row);
	}
	usleep(delay);
	row = FB->fb_vinfo.yres - CURSOR_HEIGHT;
	for (col = FB->fb_vinfo.xres - CURSOR_WIDTH; col > 0 ; col--) {
		thisFBTEST->imgOffset.startX = row;
		thisFBTEST->imgOffset.startY = col;
		thisFBTEST->imgOffset.endX = row + cursor_image->width;
		thisFBTEST->imgOffset.endY = col + cursor_image->height;
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		usleep(delay);
		updateCursorPos(&cursor_control, col, row);
	}
	usleep(delay);
	col = 0;
	for (row = FB->fb_vinfo.yres - CURSOR_HEIGHT; row > 0; row--) {
		thisFBTEST->imgOffset.startX = row;
		thisFBTEST->imgOffset.startY = col;
		thisFBTEST->imgOffset.endX = row + cursor_image->width;
		thisFBTEST->imgOffset.endY = col + cursor_image->height;
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		usleep(delay);
		updateCursorPos(&cursor_control, col, row);
	}

	usleep(delay*1000);
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	usleep(delay);
	updateCursorPos(&cursor_control, FB->fb_vinfo.xres/2,
		FB->fb_vinfo.yres/2);

	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	printf("done.%d\n", i);
	usleep(delay*1000);
	cursor_control.enable = 0;
	result = ioctl(FB->fb_fd, MSMFB_CURSOR, &cursor_control);
	if (result < 0) {
		FBTEST_MSG_DEBUG("\n[HWCURSOR] MSMFB_CURSOR failed! (Line %d)\n",\
					__LINE__);
		return -MSMFB_CURSOR_FAILED;
	}
	drawBG();
	return result;
}

int playOverlay(struct msmfb_overlay_data* ptr_ovdata,
                        struct fbDev* ptr_fb, int overlay_id,
                        uint32_t flags, uint32_t offset, int memory_id)
{
	int result = 0;
	ptr_ovdata->id = overlay_id;
	ptr_ovdata->data.flags = flags;
	ptr_ovdata->data.offset = offset;
	ptr_ovdata->data.memory_id = memory_id;
	if (ioctl(ptr_fb->fb_fd, MSMFB_OVERLAY_PLAY, ptr_ovdata)) {
		FBTEST_MSG_DEBUG("ERROR! MSMFB_OVERLAY_PLAY failed! Line %d %s\n",
				__LINE__, strerror(errno));
		return 0 ;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	return 1;
}

int overlayAlphaTest(void)
{
	/*
	The framebuffer format is currently RGBA8888 and RGB565 on FB0
	and FB1, respectively. The framebuffers go through RGB1/RGB2
	pipes; however, blending only occurs for FB0.
	RGB images go through the remaining RGB pipe while YUV images
	go through the VG pipes by default.
	This function creates a separate thread to handle alpha
	blending on HDMI (FB1).
	*/

	//define the starting width of the foreground box
	const int src_min_width = 64;
	const int src_min_height = 64;
	//define the size increment of the foreground box
	const int src_width_increment = 64;
	const int src_height_increment = 64;

	struct mdp_overlay overlay;
	//struct msmfb_overlay_data ovdata;

	int result = TEST_RESULT_PASS;
	int* ptr_ret = &result;
	int fcount;
	//int width, height, tmp_result;
	//uint32_t scaled_width, scaled_height, offset;
	float fb_relative_w_scale[FBLISTSIZE_GLOBAL];
	float fb_relative_h_scale[FBLISTSIZE_GLOBAL];
	uint32_t pipes[2], pcount;
	uint32_t overlay_ids[FBLISTSIZE_GLOBAL];
	//unsigned int xres_aligned, line_length;
	unsigned int pmem_size = 0;
	//int counter = 0;
	//int FAILFLAG = 0;

	const int ref_xres = FBLIST_GLOBAL[0]->fb_vinfo.xres;
	const int ref_yres = FBLIST_GLOBAL[0]->fb_vinfo.yres;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	//Calculate the relative scales between framebuffers if both tested together
	//Calculate the total PMEM size required

	for (fcount = 0; fcount < FBLISTSIZE_GLOBAL; fcount++) {
		fb_relative_w_scale[fcount] =
			(float)(FBLIST_GLOBAL[fcount]->fb_vinfo.xres)/(float)ref_xres;
		fb_relative_h_scale[fcount] =
			(float)(FBLIST_GLOBAL[fcount]->fb_vinfo.yres)/(float)ref_yres;
		pmem_size += FBLIST_GLOBAL[fcount]->fb_size;
	}

	if (allocMEM(pmem_size))
		return TEST_RESULT_FAIL;

	pipes[0] = 0, pipes[1] = MDP_OV_PIPE_SHARE; //define the RGB and VG pipes

	//Invoke setOverlay on every framebuffer to allocate pipes.
	//The overlay id for each framebuffer is recorded to be used in subsequent
	//setOverlays every time the size changes.
	struct blend_thread_data btd_list[FBLISTSIZE_GLOBAL];
	for (fcount = 0; fcount < FBLISTSIZE_GLOBAL; fcount++) {
		if (setOverlay(&overlay, FBLIST_GLOBAL[fcount],
			FBLIST_GLOBAL[fcount]->fb_fmt, src_min_width, src_min_height,
			0, 0, src_min_width, src_min_height, 0, 0, FB_OVERLAY_RGB_1,
			0xffffffff, pipes[fcount], MSMFB_NEW_REQUEST, 0)) {
			return  TEST_RESULT_FAIL;
		}
		overlay_ids[fcount] = overlay.id;
		struct blend_thread_data btd;
		btd.ptr_fb = FBLIST_GLOBAL[fcount];
		btd.src_min_width = src_min_width*fb_relative_w_scale[fcount];
		btd.src_min_height = src_min_height*fb_relative_h_scale[fcount];
		btd.src_width_increment = src_width_increment*fb_relative_w_scale[fcount];
		btd.src_height_increment = src_height_increment*fb_relative_h_scale[fcount];
		btd.overlay_id = overlay_ids[fcount];
		btd.offset = fcount*FBLIST_GLOBAL[0]->fb_size;
		btd.flags = 0;
		btd.ptr_result = ptr_ret;
		btd.fcount = fcount;
		btd_list[fcount] = btd;
	}

	//Perform blending on HDMI in a new thread if both frambuffers
	//are selected
	for (pcount = 0; pcount < 2; pcount++) {
		VPRINT(verbose, "Alpha Blending in Pipe %d ...\n", pcount);
		btd_list[0].flags = pipes[pcount];

		if (FBLISTSIZE_GLOBAL > 1) {
			btd_list[1].flags = pipes[pcount];
			pthread_t pth;
			pthread_create(&pth, NULL, doAlphaBlend, &btd_list[1]);
			doAlphaBlend(&btd_list[0]);
			pthread_join(pth, NULL);
			if (*ptr_ret != TEST_RESULT_PASS) {
				result = TEST_RESULT_FAIL;
				break;
			}
		} else {
			doAlphaBlend(&btd_list[0]);
		}
		if (*ptr_ret != TEST_RESULT_PASS) {
			result = TEST_RESULT_FAIL;
			break;
		}
		VPRINT(verbose, "Alpha Blending in Pipe %d Done\n", pcount);
	}

	for (fcount = 0; fcount < FBLISTSIZE_GLOBAL; fcount++) {
		if (!unsetOverlay(FBLIST_GLOBAL[fcount],
			&(overlay_ids[fcount]))) {
			result = TEST_RESULT_FAIL;
		}
	}


	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Overlay Alpha Test: Successful\n");
	} else {
		VPRINT(verbose, "Overlay Alpha Test: Fail\n");
	}

	return result;
}

static void *doAlphaBlend(void *arg)
{
	struct blend_thread_data* ptr_btd = (struct blend_thread_data*)arg;
	struct fbDev* ptr_fb = ptr_btd->ptr_fb;
	int src_min_width = ptr_btd->src_min_width;
	int src_min_height = ptr_btd->src_min_height;
	int src_width_increment = ptr_btd->src_width_increment;
	int src_height_increment = ptr_btd->src_height_increment;
	uint32_t overlay_id = ptr_btd->overlay_id;
	uint32_t offset = ptr_btd->offset;
	uint32_t flags = ptr_btd->flags;
	int* ptr_ret = ptr_btd->ptr_result;
	int fcount = ptr_btd->fcount;

	//int result = TEST_RESULT_PASS;
	int width, height, tmp_result;
	unsigned int line_length;
	const int ref_xres = ptr_fb->fb_vinfo.xres;
	const int ref_yres = ptr_fb->fb_vinfo.yres;

	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	int counter = 0;
	//Vary the source width from a minimum size to the full screen
	//height resolution
	for (width = src_min_width; width <= ref_xres; width += src_width_increment) {
		//Vary the source height from a minimum size to the full
		//screen height resolution
		for (height = src_min_height; height <= ref_yres;
			height += src_height_increment) {
			line_length = ref_xres*
			getFormatBpp(ptr_fb->fb_fmt);
			paintBuffer(MEM->mem_buf+offset, WHITE,
			ptr_fb->fb_fmt,
			ref_xres,
			ref_yres,
			line_length);

			//Redraw background every 50 iterations
			if (counter % 50 == 0 || counter == 0)
				drawBGAlpha(ptr_fb, BLEND);

			//Invoke overlay set on the framebuffer
			tmp_result = setOverlay(&overlay, ptr_fb,
							ptr_fb->fb_fmt,
							width, height, 0, 0,
							width, height, 0, 0,
							FB_OVERLAY_RGB_1, 0xffffffff, flags,
							overlay_id, offset);
			if (tmp_result == INVALID_OVERLAY) {
				//Skip overlay play if overlay size is invalid
				continue;
			} else if (!tmp_result) {
				*ptr_ret = TEST_RESULT_FAIL;
				return &THREAD_DONE_GLOBAL;
			}

			//Mix the foreground with background by invoking
			//overlay play.
			if (!playOverlay(&ovdata, ptr_fb,
					overlay_id, 0, offset, MEM->mem_fd)) {
					*ptr_ret = TEST_RESULT_FAIL;
					return &THREAD_DONE_GLOBAL;
			}
			counter++;
		}
	}
	//Other thread might have failed. If not,
	//set return pointer value to Pass.
	if (*ptr_ret != TEST_RESULT_FAIL)
		*ptr_ret = TEST_RESULT_PASS;
	FBTEST_MSG_DEBUG("FB%d Done\n", fcount);
	return &THREAD_DONE_GLOBAL;
}
int overlayScaleTest(void)
{
	/*
	Both RGB and VG pipes support scaling. However, RGB pipes do not support
	downscaling. RGB and YUV images go through RGB and VG pipes by default.
	The MDP4 HDD states the hardware supports scaling factors between 1/8
	and 8.
	This test case creates a separate thread to handle scaling on HDMI
	(FB1).
	*/

	//define the starting width of the foreground box
	const uint32_t src_min_width = 64;
	const uint32_t src_min_height = 64;
	const float scale_increment = 0.5;
	const float max_scaledown = 0.5;
	const float max_scaleup = 8.0;

	struct mdp_overlay overlay;
	//struct msmfb_overlay_data ovdata;

	int result = TEST_RESULT_PASS;
	int* ptr_ret = &result;
	int fcount;
	//int tmp_result;
	//int FAILFLAG = 0; int counter = 0;
	//uint32_t width, height, scaled_width, scaled_height, dst_w, dst_h, offset;
	//unsigned int line_length;
	float fb_relative_w_scale[FBLISTSIZE_GLOBAL];
	float fb_relative_h_scale[FBLISTSIZE_GLOBAL];
	uint32_t pipes[2], pcount;
	const uint32_t ref_xres = FBLIST_GLOBAL[0]->fb_vinfo.xres;
	const uint32_t ref_yres = FBLIST_GLOBAL[0]->fb_vinfo.yres;
	unsigned int pmem_size = 0;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	//Calculate the relative scales between framebuffers if both tested together
	//Calculate the total PMEM size required
	for (fcount = 0; fcount < FBLISTSIZE_GLOBAL; fcount++) {
		fb_relative_w_scale[fcount] =
			(float)(FBLIST_GLOBAL[fcount]->fb_vinfo.xres)/(float)ref_xres;
		fb_relative_h_scale[fcount] =
			(float)(FBLIST_GLOBAL[fcount]->fb_vinfo.yres)/(float)ref_yres;
		pmem_size += FBLIST_GLOBAL[fcount]->fb_size;
	}
	if (allocMEM(pmem_size))
		return TEST_RESULT_FAIL;

	pipes[0] = 0, pipes[1] = MDP_OV_PIPE_SHARE; //define the RGB and VG pipes
	uint32_t overlay_ids[FBLISTSIZE_GLOBAL];

	//Invoke setOverlay on every framebuffer to allocate pipes.
	//The overlay id for each framebuffer is recorded to be used in subsequent
	//setOverlays every time the size changes.
	struct scale_thread_data std_list[FBLISTSIZE_GLOBAL];

	for (fcount = 0; fcount < FBLISTSIZE_GLOBAL; fcount++) {
		overlay.src.width  = FBLIST_GLOBAL[fcount]->fb_vinfo.xres;
		overlay.src.height = FBLIST_GLOBAL[fcount]->fb_vinfo.yres;
		overlay.src.format = FBLIST_GLOBAL[fcount]->fb_fmt;
		overlay.src_rect.x = 0;
		overlay.src_rect.y = 0;
		overlay.src_rect.w = src_min_width;
		overlay.src_rect.h = src_min_height;
		overlay.dst_rect.x = 0;
		overlay.dst_rect.y = 0;
		overlay.dst_rect.w = src_min_width;
		overlay.dst_rect.h = src_min_height;
		overlay.z_order = pipes[fcount];
		overlay.transp_mask = 0xffffffff;
		overlay.flags = 0;
		overlay.id = MSMFB_NEW_REQUEST;
		overlay.alpha= 0xFF;
		if(ioctl(FBLIST_GLOBAL[fcount]->fb_fd, MSMFB_OVERLAY_SET, &overlay))
			return  TEST_RESULT_FAIL;
		overlay_ids[fcount] = overlay.id;

		struct scale_thread_data std;
		std.ptr_fb = FBLIST_GLOBAL[fcount];
		std.max_scaledown = max_scaledown;
		std.max_scaleup = max_scaleup;
		std.scale_increment = scale_increment;
		std.src_min_width = src_min_width*fb_relative_w_scale[fcount];
		std.src_min_height = src_min_height*fb_relative_h_scale[fcount];
		std.offset = fcount*FBLIST_GLOBAL[0]->fb_size;
		std.overlay_id = overlay_ids[fcount];
		std.flags = 0;
		std.fcount = fcount;
		std.ptr_result = ptr_ret;
		std_list[fcount] = std;
	}

	//Perform scaling on HDMI in a separate thread if
	//both framebuffers are selected.
	for (pcount = 0; pcount < 2; pcount++) {
		VPRINT(verbose, "Scaling in Pipe %d ...\n", pcount);
		std_list[0].flags = pipes[pcount];

		if (FBLISTSIZE_GLOBAL > 1) {
			std_list[1].flags = pipes[pcount];
			pthread_t pth;
			pthread_create(&pth, NULL, doScaleptr, &std_list[1]);
			doScaleptr(&std_list[0]);
			pthread_join(pth, NULL);
		} else {
			doScaleptr(&std_list[0]);
		}
		if (*ptr_ret != TEST_RESULT_PASS) {
			result = TEST_RESULT_FAIL;
			break;
		}
		VPRINT(verbose, "Scaling in Pipe %d Done\n", pcount);
	}

	//Close the pipes allocated to each framebuffer by invoking overlay unset
	//on the appropriate overlay ids.
	for (fcount = 0; fcount < FBLISTSIZE_GLOBAL; fcount++) {
		if (!unsetOverlay(FBLIST_GLOBAL[fcount], &(overlay_ids[fcount]))) {
			result = TEST_RESULT_FAIL;
		}
	}

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Overlay Scale Test: Successful\n");
	} else {
		VPRINT(verbose, "Overlay Scale Test: Fail\n");
	}
	return result;
}

void *doScaleptr(void* arg)
{
	struct scale_thread_data* ptr_std = (struct scale_thread_data*) arg;
	struct fbDev* ptr_fb = ptr_std->ptr_fb;
	float max_scaledown = ptr_std->max_scaledown;
	float max_scaleup = ptr_std->max_scaleup;
	uint32_t src_min_width = ptr_std->src_min_width;
	uint32_t src_min_height = ptr_std->src_min_height;
	float  scale_increment = ptr_std->scale_increment;
	uint32_t offset = ptr_std->offset;
	uint32_t overlay_id = ptr_std->overlay_id;
	int fcount = ptr_std->fcount;
	uint32_t flags = ptr_std->flags;
	int* ptr_ret = ptr_std->ptr_result;

	//int result = TEST_RESULT_PASS;
	int tmp_result;

	float scalew = max_scaledown;
	float scaleh = max_scaledown;
	int width_count = 1;
	int width_increment = 1;
	int height_increment = 1;

	uint32_t width, height, dst_w, dst_h;
	unsigned int line_length;
	const uint32_t ref_xres = ptr_fb->fb_vinfo.xres;
	const uint32_t ref_yres = ptr_fb->fb_vinfo.yres;
	int counter = 0;

	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	//Vary the source width from a minimum size to the horizontal resolution
	//of the screen, in increments of the Fibonacci sequence
	for (width = src_min_width; width <= ref_xres; width += width_increment) {
		int height_count = 1;
		//Vary the source height from a minimum size to the vertical
		//resolution of the screen, in increments of the Fibonacci sequence
		for (height = src_min_height; height <= ref_yres;
			height += height_increment) {
			FBTEST_MSG_DEBUG("FB%d: Scaling source width, source height = %d, %d\n",
					fcount, width, height);
			dst_w = width*scalew;
			dst_h = height*scaleh;
			//Destination width and height are obtained by multiplying the
			//source by the scaling factor
			while(dst_w <= ref_xres && scalew <= max_scaleup) {
				while(dst_h <= ref_yres && scaleh <= max_scaleup) {
			//Only downscale if not using RGB pipe
			if (!(flags == 0 && (dst_w < width || dst_h < height))) {
					//Perform scaling on each framebuffer
					line_length = ref_xres*
					getFormatBpp(ptr_fb->fb_fmt);

					//Populate the PMEM with the foreground
					paintBuffer(MEM->mem_buf+offset, WHITE,
								ptr_fb->fb_fmt,
								ref_xres,
								ref_yres,
								line_length);

					if (counter % 50 == 0 || counter == 0)
						//Redraw the background every 50 iterations
						drawBGAlpha(ptr_fb, BLACK);

					//Invoke overlay set on the framebuffer
					overlay.src.width  = ptr_fb->fb_vinfo.xres;
					overlay.src.height = ptr_fb->fb_vinfo.yres;
					overlay.src.format = ptr_fb->fb_fmt;
					overlay.src_rect.x = 0;
					overlay.src_rect.y = 0;
					overlay.src_rect.w = width;
					overlay.src_rect.h = height;
					overlay.dst_rect.x = 0;
					overlay.dst_rect.y = 0;
					overlay.dst_rect.w = dst_w;
					overlay.dst_rect.h = dst_h;
					overlay.z_order = FB_OVERLAY_RGB_1;
					overlay.transp_mask = 0xffffffff;
					overlay.flags = flags;
					overlay.id = overlay_id;
					overlay.alpha= 0xFF;
					tmp_result = ioctl(FBLIST_GLOBAL[fcount]->fb_fd, MSMFB_OVERLAY_SET,
								 &overlay);

					if (tmp_result == INVALID_OVERLAY) {
						//Skip overlay play if size is invalid
						continue;
					} else if (tmp_result) {
						*ptr_ret = TEST_RESULT_FAIL;
						return &THREAD_DONE_GLOBAL;
					}
					//Mix the framebuffer with the PMEM by invoking
					//overlay play
					if (!playOverlay(&ovdata, ptr_fb,
						overlay_id, 0, offset, MEM->mem_fd)) {
						*ptr_ret = TEST_RESULT_FAIL;
						return &THREAD_DONE_GLOBAL;
					}
			}
			counter++;
					//update the vertical scaling factor
					scaleh += scale_increment;
					dst_h = height*scaleh;
				}
				//reset the vertical scaling factor
				scaleh = max_scaledown;
				dst_h = height*scaleh;
				//update the horizontal scaling factor
				scalew += scale_increment;
				dst_w = width*scalew;
			}
			//reset the horizontal scaling factor
			scalew = max_scaledown;
			dst_w = width*scalew;
			height_count += 1;
			//If the current source height plus the Fibonacci increment
			//exceeds the screen height, stop at the screen boundary.
			//Otherwise increment the source height by the Fibonacci number.
			if (ref_yres - height != 0)
				height_increment = (fibonacci(height_count) >
				ref_yres-height ? ref_yres-height : fibonacci(height_count));
			else
				height_increment = 1;
		} // end source height loop
		width_count += 1;
		//If the current source width plus the Fibonacci increment exceeds
		//the screen width, stop at the screen boundary. Otherwise
		//increment the source width by the Fibonacci number.
		if (ref_xres - width != 0)
			width_increment = (fibonacci(width_count) >
			ref_xres-width ? ref_xres-width : fibonacci(width_count));
		else
			width_increment = 1;
	} // end source width loop

	//Other thread might have failed. If not,
	//set return pointer value to Pass.
	if (*ptr_ret != TEST_RESULT_FAIL)
		*ptr_ret = TEST_RESULT_PASS;
	FBTEST_MSG_DEBUG("FB%d Done\n", fcount);
	return &THREAD_DONE_GLOBAL;
}

uint32_t fibonacci(uint32_t n)
{
	/*
	This function returns the n-th Fibonacci number.
	It is used to increment the width and height
	of the source rectangle in overlayScaleTest to
	cover a wide range of dimensions.
	*/
	if (n == 1 || n == 2)
		return 1;
	else
		return fibonacci(n-1) + fibonacci(n-2);
}
int drawBGAlpha(struct fbDev* ptr_fb, int color)
{
	int result;

	result = paintBuffer(ptr_fb->fb_buf, color, ptr_fb->fb_fmt,
				ptr_fb->fb_vinfo.xres, ptr_fb->fb_vinfo.yres,
				ptr_fb->fb_finfo.line_length);

	if (result != TEST_RESULT_PASS)
		return result;

	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	return TEST_RESULT_PASS;
	}
	static int overlayTest(void)
	{
	int result, overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;

		memset(&overlay, 0, sizeof(struct mdp_overlay));
	drawBG();

	if (allocMEM(FB->fb_size))
		return TEST_RESULT_FAIL;

	printf("overlayTest: addr=%x size=%d\n", MEM->mem_buf, FB->fb_size);

	memcpy(MEM->mem_buf, yellow_face_data, (int)(FACE_WIDTH*FACE_HEIGHT*getFormatBpp(FACE_FORMAT)));

	overlay.src.width  = FACE_WIDTH;
	overlay.src.height = FACE_HEIGHT;
	overlay.src.format = FACE_FORMAT;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = FACE_WIDTH;
	overlay.src_rect.h = FACE_HEIGHT;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 300;
	overlay.dst_rect.w = FACE_WIDTH;
	overlay.dst_rect.h = FACE_HEIGHT;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0x80;
	overlay.transp_mask = 0xffe0;
	overlay.flags = MDP_FLIP_LR | MDP_FLIP_UD;
	if (FB->fb_vinfo.bits_per_pixel == 32)
		overlay.is_fg = 0;
	else {
		overlay.is_fg = 1;
		overlay.alpha = 0xFF;
	}
	overlay.id = MSMFB_NEW_REQUEST;

	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

	printf("MSMFB_OVERLAY_SET: id=%d\n", overlay.id);

	if (overlay_id < 0) {
		printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n", __LINE__, overlay_id);
		return TEST_RESULT_FAIL;
	}
	overlay_id = overlay.id;	/* return from mdp */
	result = TEST_RESULT_PASS;
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		printf("\nERROR! MSMFB_OVERLAY_PLAY failed! (Line %d)\n", __LINE__);
		result = TEST_RESULT_FAIL;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	printf("MSMFB_OVERLAY_PLAY: play_done\n");

	if (interactive) {
		interactiveDelay();
	} else {
		add_delay_in_user_mode();
	}

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id)) {
		printf("\nERROR! MSMFB_OVERLAY_UNSET failed! (Line %d)\n", __LINE__);
		result = TEST_RESULT_FAIL;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	printf("MSMFB_OVERLAY_PLAY: unset\n");


	if (result == TEST_RESULT_PASS)
		printf("Overlay Test: Successful \n");
	else
		printf("Overlay Test: Fail \n");
	return result;
}
static int overlayARGBTest(void)
{
	int result, overlay_id, i, j;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	char *bp;

        memset(&overlay, 0, sizeof(struct mdp_overlay));
	drawBG();

	if (allocMEM(FB->fb_size))
		return TEST_RESULT_FAIL;

	drawBG();


	bp = (char *)MEM->mem_buf;

	for (j = 0; j < 512; j++) {
		for (i = 0; i < 256; i++) {
			*bp++ = j;	/* A */
			*bp++ = 0xff;	/* R */
			*bp++ = 0x0; 	/* G */
			*bp++ = 0x0;	/* B */
		}
	}

	overlay.src.width  = 256;
	overlay.src.height = 512;
	overlay.src.format = MDP_ARGB_8888;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = 256;
	overlay.src_rect.h = 512;
	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = 256;
	overlay.dst_rect.h = 512;
	overlay.z_order = FB_OVERLAY_RGB_1;
	overlay.alpha = 0xFF;
	overlay.transp_mask = 0xffffffff;
	overlay.flags = 0;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay.is_fg = 0;

	} else {
		overlay.is_fg = 1;
	}
	overlay.id = MSMFB_NEW_REQUEST;



	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

	printf("MSMFB_OVERLAY_SET: id=%d\n", overlay.id);

	if (overlay_id < 0) {
		printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n", __LINE__, overlay_id);
		return TEST_RESULT_FAIL;
	}

	overlay_id = overlay.id;	/* return from mdp */
	result = TEST_RESULT_PASS;
	ovdata.id = overlay_id;
	ovdata.data.flags = 0;
	ovdata.data.offset = 0;
	ovdata.data.memory_id = MEM->mem_fd;

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		printf("\nERROR! MSMFB_OVERLAY_PLAY failed! (Line %d)\n", __LINE__);
		result = TEST_RESULT_FAIL;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;
	printf("MSMFB_OVERLAY_PLAY: play_done\n");

	if (interactive) {
		interactiveDelay();
	} else {
		add_delay_in_user_mode();
	}

	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id)) {
		printf("\nERROR! MSMFB_OVERLAY_UNSET failed! (Line %d)\n", __LINE__);
		result = TEST_RESULT_FAIL;
	}
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;

	printf("MSMFB_OVERLAY_PLAY: unset\n");
	if (result == TEST_RESULT_PASS)
		printf("Overlay ARGB Test: Successful\n");
	else
		printf("Overlay ARGB Test: Fail\n");
	return result;
}
static int allcolorFormatOverlayTest(void)
{
	int samples, iterations, w, h, x, y, i;
	int fps_print, fb_fd, format;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	int result = 0;
	thisFBTEST = &FBTEST;

	fps_print = 1;

	vinfo = &(FB->fb_vinfo);
	fb_fd = FB->fb_fd;

	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;

        memset(&overlay, 0, sizeof(struct mdp_overlay));
	printf(" \n Starting all color format test \n");

	if (allocMEM(FB->fb_size)) {
		printf("\nERROR! allocPMEM failed! (Line %d)\n",
			__LINE__);
		return TEST_RESULT_FAIL;
	}


	for (format = MDP_RGB_565; format < MDP_Y_CRCB_H2V2_TILE; format++) {

		switch(format) {
			case MDP_RGB_565:
				printf("\nTesting color format RGB_565 with YELLOW pattern");
				break;
			case MDP_XRGB_8888:
				printf("\nTesting color format XRGB_8888 with YELLOW pattern");
				break;
			case MDP_Y_CBCR_H2V2:
				printf("\nTesting color format Y_CBCR_H2V2 with GREY pattern");
				break;
			case MDP_ARGB_8888:
				printf("\nTesting color format ARGB_8888 with YELLOW pattern");
				break;
			case MDP_RGB_888:
				printf("\nTesting color format RGB_888 with YELLOW pattern");
				break;
			case MDP_Y_CRCB_H2V2:
				printf("\nTesting color format Y_CRCB_H2V2 with GREY pattern");
				break;
			case MDP_YCRYCB_H2V1:
				printf("\nTesting color format YCRYCB_H2V1 with GREY pattern");
				break;
			case MDP_Y_CBCR_H2V1:
				printf("\nTesting color format Y_CBCR_H2V1 with GREY pattern");
				break;
			case MDP_Y_CRCB_H2V1:
				printf("\nTesting color format Y_CRCB_H2V1 with GREY pattern");
				break;
			case MDP_RGBA_8888:
				printf("\nTesting color format RGBA_8888 with YELLOW pattern");
				break;
			case MDP_BGRA_8888:
				printf("\nTesting color format BGRA_8888 with YELLOW pattern");
				break;
			case MDP_RGBX_8888:
				printf("\nTesting color format RGBX_8888 with YELLOW pattern");
				break;
			default:
				printf("\n Error! Format=%d not supported in test", format);
				continue;
		}

		paintBuffer(MEM->mem_buf, YELLOW, format, SQUARE_WIDTH+10,
				SQUARE_HEIGHT+10, (SQUARE_WIDTH+10)* getFormatBpp(format));

		/* Scaling Test */
		/*********************************************************************/

		overlay.src.width  = SQUARE_WIDTH;
		overlay.src.height = SQUARE_HEIGHT;
		overlay.src.format = format;
		overlay.z_order = FB_OVERLAY_RGB_1;
		overlay.transp_mask = MDP_TRANSP_NOP;
		overlay.alpha = 0xFF;
		if (FB->fb_vinfo.bits_per_pixel == 32) {
			overlay.is_fg = 0;
		} else {
			overlay.is_fg = 1;
		}
		overlay.id = MSMFB_NEW_REQUEST;
		overlay.src_rect.x = 0;
		overlay.src_rect.y = 0;
		overlay.src_rect.w = SQUARE_WIDTH;
		overlay.src_rect.h = SQUARE_HEIGHT;
		overlay.dst_rect.x = 0;
		overlay.dst_rect.y = 0;
		overlay.dst_rect.w = SQUARE_WIDTH;
		overlay.dst_rect.h = SQUARE_HEIGHT;
		overlay.flags = 0;

		drawBG();

		while (overlay.dst_rect.w < 8*SQUARE_WIDTH  &&
			   overlay.dst_rect.h < 8*SQUARE_HEIGHT &&
			   (overlay.dst_rect.w < vinfo->xres) &&
			   (overlay.dst_rect.h < vinfo->yres)) {

			if (is_valid_overlay(&overlay))
				continue;
			overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
			if (overlay_id < 0) {
				printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
					   __LINE__, overlay_id);
				return TEST_RESULT_FAIL;
			}

			overlay_id = overlay.id;
			ovdata.id = overlay_id;
			ovdata.data.flags = 0;
			ovdata.data.offset = 0;
			ovdata.data.memory_id = MEM->mem_fd;

			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
				printf("\nERROR! MSMFB_OVERLAY_PLAY failed! (Line %d)\n",
						__LINE__);
				return TEST_RESULT_FAIL;
			}
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			if (thisFBTEST->fBuffer == FB0) {
				result = doDisplayUpdate();
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			}
			overlay.dst_rect.w++;
			overlay.dst_rect.h++;

		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id)) {
			printf("\nERROR! MSMFB_OVERLAY_UNSET failed! (Line %d)\n",
					__LINE__);
			return TEST_RESULT_FAIL;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		printf("\n Scaling Test : PASS \n");

		/* Blit Performance Test */
		/************************************************************************************************************/
		overlay.src.width  = SQUARE_WIDTH;
		overlay.src.height = SQUARE_HEIGHT;
		overlay.src.format = format;
		overlay.z_order = FB_OVERLAY_RGB_1;
		overlay.transp_mask = MDP_TRANSP_NOP;
		overlay.alpha = 0xFF;
		if (FB->fb_vinfo.bits_per_pixel == 32) {
			overlay.is_fg = 0;
		} else {
			overlay.is_fg = 1;
		}
		overlay.id = MSMFB_NEW_REQUEST;

		overlay.src_rect.x = 0;
		overlay.src_rect.y = 0;
		overlay.src_rect.w = SQUARE_WIDTH;
		overlay.src_rect.h = SQUARE_HEIGHT;
		overlay.dst_rect.x = 0;
		overlay.dst_rect.y = 0;
		overlay.dst_rect.w = SQUARE_WIDTH;
		overlay.dst_rect.h = SQUARE_HEIGHT;
		overlay.flags = 0;

		drawBG();
		if (thisFBTEST->iteration)
			iterations = thisFBTEST->iteration;
		else
			iterations = 20;

		for (i = 0,w = overlay.dst_rect.w, h = overlay.dst_rect.h;
			 i < iterations;
			 i++) {

			x = (rand() % 200/*vinfo->xres*/) - w;
			if (x < 0)
				x = 0;
			overlay.dst_rect.x = x;

			y = (rand() % 200/*vinfo->yres*/) - h;
			if (y < 0)
				y = 0;
			overlay.dst_rect.y = y;

			if (is_valid_overlay(&overlay))
				continue;
			overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
			if (overlay_id < 0)
			{
				printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
						__LINE__, overlay_id);
				return TEST_RESULT_FAIL;
			}

			overlay_id = overlay.id;
			ovdata.id = overlay_id;
			ovdata.data.flags = 0;
			ovdata.data.offset = 0;
			ovdata.data.memory_id = MEM->mem_fd;

			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
				printf("\nERROR! MSMFB_OVERLAY_PLAY failed! (Line %d)\n",
					   __LINE__);
				return TEST_RESULT_FAIL;
			}
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			if (thisFBTEST->fBuffer == FB0) {
				result = doDisplayUpdate();
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			}
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id)) {
			printf("\nERROR! MSMFB_OVERLAY_UNSET failed! (Line %d)\n",
				   __LINE__);
			return TEST_RESULT_FAIL;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		printf("\n Blit Performance Test : PASS \n");

		/* Blend Performance Test */
		/*********************************************************************/
		overlay.src.width  = SQUARE_WIDTH;
		overlay.src.height = SQUARE_HEIGHT;
		overlay.src.format = format;
		overlay.z_order = FB_OVERLAY_RGB_1;
		overlay.transp_mask = MDP_TRANSP_NOP;
		if (FB->fb_vinfo.bits_per_pixel == 32)
			overlay.is_fg = 0;
		else
			overlay.is_fg = 1;
		overlay.id = MSMFB_NEW_REQUEST;

		overlay.src_rect.x = 0;
		overlay.src_rect.y = 0;
		overlay.src_rect.w = SQUARE_WIDTH;
		overlay.src_rect.h = SQUARE_HEIGHT;
		overlay.dst_rect.x = 0;
		overlay.dst_rect.y = 0;
		overlay.dst_rect.w = SQUARE_WIDTH;
		overlay.dst_rect.h = SQUARE_HEIGHT;
		overlay.alpha = 0x80;
		overlay.flags = 0;

		drawBG();

		if (thisFBTEST->iteration)
			iterations = thisFBTEST->iteration;
		else
			iterations = 20;


		for (i = 0, w = overlay.dst_rect.w, h = overlay.dst_rect.h;
			 i < iterations;
			 i++) {

			x = (rand() % 200/*vinfo->xres*/) - w;
			if (x < 0)
				x = 0;
			overlay.dst_rect.x = x;

			y = (rand() % 200/*vinfo->yres*/) - h;
			if (y < 0)
				y = 0;
			overlay.dst_rect.y = y;

			if (is_valid_overlay(&overlay))
				continue;
			overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
			if (overlay_id < 0) {
				printf("\n ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
						__LINE__, overlay_id);
				return TEST_RESULT_FAIL;
			}

			overlay_id = overlay.id;
			ovdata.id = overlay_id;
			ovdata.data.flags = 0;
			ovdata.data.offset = 0;
			ovdata.data.memory_id = MEM->mem_fd;

			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
				printf("\n ERROR! MSMFB_OVERLAY_PLAY failed! (Line %d)\n",
					   __LINE__);
				return TEST_RESULT_FAIL;
			}
			result = doDisplayUpdate();
			if (result == TEST_RESULT_FAIL)
				return TEST_RESULT_FAIL;
			if (thisFBTEST->fBuffer == FB0) {
				result = doDisplayUpdate();
				if (result == TEST_RESULT_FAIL)
					return TEST_RESULT_FAIL;
			}
		}

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id)) {
			printf("\nERROR! MSMFB_OVERLAY_UNSET failed! (Line %d)\n",
				   __LINE__);
			return TEST_RESULT_FAIL;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		printf("\n Blend Performance Test : PASS \n");
	}


	return TEST_RESULT_PASS;
}
static int overlayStressTest(void)
{
	int src_roi[4];
	int dst_roi[4];
	unsigned int dst_w, dst_h;
	unsigned int rotate, scale = 4;
	unsigned long count = 0;
	unsigned int line_length, xres_aligned, framesize;
	int remainder;
	struct fbtest_params *thisFBTEST;
	int result = 0;
	thisFBTEST = &FBTEST;

	struct fb_var_screeninfo *vinfo = &(FB->fb_vinfo);
	int overlay_id;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	xres_aligned = (FB->fb_vinfo.xres + 31) & (~31);

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	framesize = xres_aligned * FB->fb_vinfo.yres * 2;

	if (allocMEM(framesize*2)) {
		printf("\n PMEM allocation failed");
		return TEST_RESULT_FAIL;
	}

	line_length = xres_aligned * getFormatBpp(MDP_YCRYCB_H2V1);

	paintBuffer(MEM->mem_buf, GREY,MDP_YCRYCB_H2V1, FB->fb_vinfo.xres,
				FB->fb_vinfo.yres, line_length);

	paintBuffer(MEM->mem_buf + framesize, BLACK,MDP_YCRYCB_H2V1,
		FB->fb_vinfo.xres, FB->fb_vinfo.yres, line_length);

	drawBG();
	if (thisFBTEST->fBuffer == FB0)
		paintBuffer(FB->fb_buf, YELLOW, MDP_RGBA_8888, FB->fb_vinfo.xres,
				200, FB->fb_finfo.line_length);

	overlay.src.width  = FB->fb_vinfo.xres;
	overlay.src.height = FB->fb_vinfo.yres;
	overlay.src.format = MDP_YCRYCB_H2V1;
	overlay.z_order = FB_OVERLAY_VID_0;
	overlay.transp_mask = MDP_TRANSP_NOP;
	if (FB->fb_vinfo.bits_per_pixel == 32)
		overlay.is_fg = 0;
	else
		overlay.is_fg = 1;
	overlay.alpha = 0xD0;
	overlay.flags = 0;
	overlay.id = MSMFB_NEW_REQUEST;


	printf("\n Starting stress test: \n");
	srand(1);
	while (1) {
		get_rand4(src_roi, FB->fb_vinfo.xres,
			FB->fb_vinfo.yres, 1);
		get_rand4(dst_roi, FB->fb_vinfo.xres,
			FB->fb_vinfo.yres, 2);

		overlay.src_rect.x = src_roi[0] & ~1;
		overlay.src_rect.y = src_roi[1] & ~1;
		overlay.src_rect.w = src_roi[2] & ~1;
		overlay.src_rect.h = src_roi[3] & ~1;
		overlay.dst_rect.x = dst_roi[0];
		overlay.dst_rect.y = dst_roi[1];
		rotate = rand() % 4;

	printf("src: x=%d y=%d w=%d h=%d -> dst x=%d y=%d\n",
	src_roi[0], src_roi[1], src_roi[2], src_roi[3], dst_roi[0], dst_roi[1]);

		switch (rotate) {
		case 0:
		case 2:
#ifdef MDP4_FLIP
			if (rotate == 0)
				overlay.flags = MDP_ROT_90;
			else
				overlay.flags = MDP_ROT_270;
#endif

			for (dst_w = overlay.src_rect.h/scale + 1;
				  dst_w < scale * overlay.src_rect.h &&
				  dst_w + overlay.dst_rect.x < FB->fb_vinfo.xres;
				  dst_w++) {

				for (dst_h = overlay.src_rect.w/scale + 1;
					  dst_h < scale * overlay.src_rect.w &&
					  dst_h + overlay.dst_rect.y < FB->fb_vinfo.yres;
					  dst_h++) {

					overlay.dst_rect.w = dst_w  & ~1;
					overlay.dst_rect.h = dst_h  & ~1;

					if (is_valid_overlay(&overlay))
						continue;
					overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

					if (overlay_id < 0)
						continue;

					overlay_id = overlay.id;
					ovdata.id = overlay_id;
					ovdata.data.flags = 0;
					ovdata.data.offset = 0;
					ovdata.data.memory_id = MEM->mem_fd;

					if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
						printf("\n ERROR! MSMFB_OVERLAY_PLAY failed! count: %ld", count);
					}
					result = doDisplayUpdate();
					if (result == TEST_RESULT_FAIL)
						return TEST_RESULT_FAIL;
					if (++count%1000 == 0)
						fprintf(stderr, "%ld blits\n", count);
				}
			}
			break;
		case 1:
		default:
#ifdef MDP4_FLIP
			if (rotate == 1)
				overlay.flags = MDP_ROT_180;
			else
				overlay.flags = MDP_ROT_NOP;
#endif
			for (dst_w = overlay.src_rect.w/scale + 1;
				  dst_w < scale * overlay.src_rect.w &&
				  dst_w + overlay.dst_rect.x < FB->fb_vinfo.xres;
				  dst_w++) {
				for (dst_h = overlay.src_rect.h/scale + 1;
					  dst_h < scale * overlay.src_rect.h &&
					  dst_h + overlay.dst_rect.y < FB->fb_vinfo.yres;
					  dst_h++) {
					overlay.dst_rect.w = dst_w & ~1;
					overlay.dst_rect.h = dst_h & ~1;

					if (is_valid_overlay(&overlay))
						continue;
					overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

					if (overlay_id < 0)
						continue;

					overlay_id = overlay.id;
					ovdata.id = overlay_id;
					ovdata.data.flags = 0;
					ovdata.data.offset = 0;
					ovdata.data.memory_id = MEM->mem_fd;

					if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
					  printf("\n ERROR! MSMFB_OVERLAY_PLAY failed! count: %ld", count);
					}
					result = doDisplayUpdate();
					if (result == TEST_RESULT_FAIL)
						return TEST_RESULT_FAIL;
					if (++count%1000 == 0)
					  fprintf(stderr, "%ld blits\n", count);
				}
			}
			break;
		}
	}
	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id))
		printf("\n ERROR! MSMFB_OVERLAY_UNSET failed! count: %ld", count);
	result = doDisplayUpdate();
	if (result == TEST_RESULT_FAIL)
		return TEST_RESULT_FAIL;

	return 0;
}

#define COLOR_STEP 16
static void colorize(char *r, char *g, char *b, int row, int col,
						int row_off, int col_off)
{
	int i = 0, temp = 0;
	int dist_x = row - row_off;
	int dist_y = col - col_off;
	int dist = (dist_x * dist_x) + (dist_y * dist_y);
	unsigned int color[3];
	int color_from = 0;
	int color_to = 1;
	int step_size = 1;

	while (i <= dist/2) { //Lazy Square Root
		if (i*i < dist)
			temp = i;
		i++;
	}
	dist = temp;

	//Red = 255 @ dist = 0
	color[0] = 256;
	color[1] = 0;
	color[2] = 0;

	while (dist > 0) {
		color[color_from] -= COLOR_STEP;
		color[color_to] += COLOR_STEP;
		dist -= step_size;

		if (color[color_from] == 0) {
			color_from = (color_from + 1) % 3;
			color_to = (color_to + 1) % 3;

			if (color_from == 0)
				step_size *= 2;
		}
	}

	for (i = 0; i < 3; i++) {
		if (color[i] == 256)
			color[i] = 255;
	}

	*r = (char) color[0];
	*g = (char) color[1];
	*b = (char) color[2];
}
static int overlayPPTest(void)
{
	int result, overlay_id[2], i, j;
	struct mdp_overlay overlay[2];
	struct msmfb_overlay_data ovdata[2];
	char *bp;
	uint32_t * values;
	char r,g,b;
	uint32_t lv[6] = {0,255,0,255,0,255};
	uint32_t bv[3] = {0x0, 0x0, 0x0};
	uint32_t csc0[9] =	{0x0200,0x0000,0x0000,
				0x0000,0x0200,0x0000,
				0x0000,0x0000,0x0200};
	uint32_t csc1[9] =	{0x0200,0x0000,0x0000,
				0x0000,0x0180,0x0000,
				0x0000,0x0000,0x0180};
	uint32_t csc2[9] =	{0x0200,0x0000,0x0000,
				0x0000,0x0100,0x0000,
				0x0000,0x0000,0x0100};
	uint32_t csc3[9] =	{0x0200,0x0000,0x0000,
				0x0000,0x0080,0x0000,
				0x0000,0x0000,0x0080};
	uint32_t csc4[9] =	{0x0200,0x0000,0x0000,
				0x0000,0x0000,0x0000,
				0x0000,0x0000,0x0000};
	memset(overlay, 0, sizeof(struct mdp_overlay) * 2);
	drawBG();

	if (allocMEM(FB->fb_size))
		return TEST_RESULT_FAIL;

	drawBG();


	bp = (char *)MEM->mem_buf;

	for (j = 0; j < 512; j++) {
		for (i = 0; i < 256; i++) {
			*bp++ = 0xff;	/* A */
			colorize(&r, &g, &b, i, j, 256/4, 512/4);
			*bp++ = r;	/* R */
			*bp++ = g; 	/* G */
			*bp++ = b;	/* B */
		}
	}

	overlay[0].src.width  = 256;
	overlay[0].src.height = 512;
	overlay[0].src.format = MDP_ARGB_8888;
	overlay[0].src_rect.x = 0;
	overlay[0].src_rect.y = 0;
	overlay[0].src_rect.w = 256;
	overlay[0].src_rect.h = 512;
	overlay[0].dst_rect.x = 0;
	overlay[0].dst_rect.y = 0;
	overlay[0].dst_rect.w = 256;
	overlay[0].dst_rect.h = 512;
	overlay[0].z_order = 1;
	overlay[0].alpha = 0xFF;
	overlay[0].transp_mask = 0xffffffff;
	overlay[0].flags = MDP_OVERLAY_PP_CFG_EN;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay[0].is_fg = 0;

	} else {
		overlay[0].is_fg = 1;
	}

	overlay[1].src.width  = 256;
	overlay[1].src.height = 512;
	overlay[1].src.format = MDP_ARGB_8888;
	overlay[1].src_rect.x = 0;
	overlay[1].src_rect.y = 0;
	overlay[1].src_rect.w = 256;
	overlay[1].src_rect.h = 512;
	overlay[1].dst_rect.x = 257;
	overlay[1].dst_rect.y = 0;
	overlay[1].dst_rect.w = 256;
	overlay[1].dst_rect.h = 512;
	overlay[1].z_order = 0;
	overlay[1].alpha = 0xFF;
	overlay[1].transp_mask = 0xffffffff;
	overlay[1].flags = MDP_OVERLAY_PP_CFG_EN;
	if (FB->fb_vinfo.bits_per_pixel == 32) {
		overlay[1].is_fg = 0;

	} else {
		overlay[1].is_fg = 1;
	}

	values = malloc(sizeof(uint32_t)*2);
	if (!values)
		return -MALLOC_FAILED;

	for (i = 0; i <= 10; i++) {
		overlay[0].id = MSMFB_NEW_REQUEST;
		overlay[1].id = MSMFB_NEW_REQUEST;

		switch (i%5) {
		case 0:
			overlay[0].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[0].overlay_pp_cfg.csc_cfg.flags = 0;
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_mv, csc0, sizeof(uint32_t) * 9);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[0].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].len = 2;

			overlay[1].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[1].overlay_pp_cfg.csc_cfg.flags = MDP_CSC_FLAG_ENABLE;
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_mv, csc0, sizeof(uint32_t) * 9);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[1].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].len = 2;

			values[0] = 0x20000000;
			values[1] = 0x00000000;

			overlay[0].overlay_pp_cfg.qseed_cfg[0].data = values;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].data = values;
			break;
		case 1:
			overlay[0].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[0].overlay_pp_cfg.csc_cfg.flags = 0;
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_mv, csc1, sizeof(uint32_t) * 9);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[0].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].len = 2;

			overlay[1].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[1].overlay_pp_cfg.csc_cfg.flags = MDP_CSC_FLAG_ENABLE;
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_mv, csc1, sizeof(uint32_t) * 9);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[1].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].len = 2;

			values[0] = 0x22222222;
			values[1] = 0x22222222;

			overlay[0].overlay_pp_cfg.qseed_cfg[0].data = values;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].data = values;

			break;
		case 2:

			overlay[0].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[0].overlay_pp_cfg.csc_cfg.flags = 0;
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_mv, csc2, sizeof(uint32_t) * 9);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[0].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].len = 2;

			overlay[1].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[1].overlay_pp_cfg.csc_cfg.flags = MDP_CSC_FLAG_ENABLE;
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_mv, csc2, sizeof(uint32_t) * 9);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[1].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].len = 2;

			values[0] = 0x44444444;
			values[1] = 0x44444444;

			overlay[0].overlay_pp_cfg.qseed_cfg[0].data = values;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].data = values;
			break;
		case 3:

			overlay[0].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[0].overlay_pp_cfg.csc_cfg.flags = 0;
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_mv, csc2, sizeof(uint32_t) * 9);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[0].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].len = 2;

			overlay[1].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[1].overlay_pp_cfg.csc_cfg.flags = MDP_CSC_FLAG_ENABLE;
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_mv, csc2, sizeof(uint32_t) * 9);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[1].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].len = 2;

			values[0] = 0x66666666;
			values[1] = 0x66666666;

			overlay[0].overlay_pp_cfg.qseed_cfg[0].data = values;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].data = values;
			break;
		case 4:

			overlay[0].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[0].overlay_pp_cfg.csc_cfg.flags = 0;
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_mv, csc4, sizeof(uint32_t) * 9);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[0].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[0].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[0].overlay_pp_cfg.qseed_cfg[0].len = 2;

			overlay[1].overlay_pp_cfg.config_ops =
				MDP_OVERLAY_PP_CSC_CFG | MDP_OVERLAY_PP_QSEED_CFG;
			overlay[1].overlay_pp_cfg.csc_cfg.flags = MDP_CSC_FLAG_ENABLE;
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_mv, csc4, sizeof(uint32_t) * 9);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_lv, lv, sizeof(uint32_t) * 6);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_pre_bv, bv, sizeof(uint32_t) * 3);
			memcpy(overlay[1].overlay_pp_cfg.csc_cfg.csc_post_bv, bv, sizeof(uint32_t) * 3);

			overlay[1].overlay_pp_cfg.qseed_cfg[0].table_num = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].ops = 1;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].len = 2;

			values[0] = 0x88888888;
			values[1] = 0x88888888;

			overlay[0].overlay_pp_cfg.qseed_cfg[0].data = values;
			overlay[1].overlay_pp_cfg.qseed_cfg[0].data = values;
			break;
		}

		overlay_id[0] = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay[0]);
		overlay_id[1] = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay[1]);

		printf("MSMFB_OVERLAY_SET: id=%d\n", overlay[0].id);
		printf("MSMFB_OVERLAY_SET: id=%d\n", overlay[1].id);

		if (overlay_id[0] < 0) {
			printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n", __LINE__, overlay_id[0]);
			return TEST_RESULT_FAIL;
		}
		if (overlay_id[1] < 0) {
			printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n", __LINE__, overlay_id[1]);
			return TEST_RESULT_FAIL;
		}

		overlay_id[0] = overlay[0].id;	/* return from mdp */
		result = TEST_RESULT_PASS;
		ovdata[0].id = overlay_id[0];
		ovdata[0].data.flags = 0;
		ovdata[0].data.offset = 0;
		ovdata[0].data.memory_id = MEM->mem_fd;

		overlay_id[1] = overlay[1].id;	/* return from mdp */
		result = TEST_RESULT_PASS;
		ovdata[1].id = overlay_id[1];
		ovdata[1].data.flags = 0;
		ovdata[1].data.offset = 0;
		ovdata[1].data.memory_id = MEM->mem_fd;

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata[0])) {
			printf("\nERROR! MSMFB_OVERLAY_PLAY failed! (Line %d)\n", __LINE__);
			result = TEST_RESULT_FAIL;
		}
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, &ovdata[1])) {
			printf("\nERROR! MSMFB_OVERLAY_PLAY failed! (Line %d)\n", __LINE__);
			result = TEST_RESULT_FAIL;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
		printf("MSMFB_OVERLAY_PLAY: play_done\n");

		add_delay_in_user_mode();

		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id[1])) {
			printf("\nERROR! MSMFB_OVERLAY_UNSET failed! (Line %d)\n", __LINE__);
			result = TEST_RESULT_FAIL;
		}
		if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id[0])) {
			printf("\nERROR! MSMFB_OVERLAY_UNSET failed! (Line %d)\n", __LINE__);
			result = TEST_RESULT_FAIL;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			return TEST_RESULT_FAIL;
	}

	free(values);


	printf("MSMFB_OVERLAY_PLAY: unset\n");
	if (result == TEST_RESULT_PASS)
		printf("Overlay ARGB Test: Successful\n");
	else
		printf("Overlay ARGB Test: Fail\n");
	return result;
}

int getHistogram(unsigned int block) {
	struct mdp_histogram_start_req hist_req;
	struct mdp_histogram_data hist;
	int result, extra;
	hist_req.block = block;
	hist_req.frame_cnt = 1;
	hist_req.bit_mask = 0;
	hist_req.num_bins = 128;
	hist.bin_cnt    = hist_req.num_bins;
	hist.block      = block;
	hist_req.block  = block;

	hist.extra_info = calloc(4 , sizeof(uint32_t));
	hist.c0         = calloc(hist.bin_cnt , sizeof(uint32_t));
	hist.c1         = calloc(hist.bin_cnt , sizeof(uint32_t));
	hist.c2         = calloc(hist.bin_cnt , sizeof(uint32_t));
	extra = sizeof(hist.extra_info);

	VPRINT(verbose, "Hist Loop \nCalling ioctl for do_histogram!! - %d\n",
		hist_req.block);
	result = ioctl(FB->fb_fd, MSMFB_HISTOGRAM_START, &hist_req);
	if (result) {
		VPRINT(verbose, "Histogram Start FAILED!%d\n", result);
		result = TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "Histogram Start PASSED\n");
		get_histogram_crc(&hist, extra);
		result = TEST_RESULT_PASS;
	}

	result = ioctl(FB->fb_fd, MSMFB_HISTOGRAM, &hist);
	long int crc;
	if (result) {
		VPRINT(verbose, "Histogram FAILED! Error = %d\n", result);
		result = TEST_RESULT_FAIL;
		crc = get_histogram_crc(&hist, extra);
	} else {
		VPRINT(verbose, "Histogram PASSED\n");
		crc = get_histogram_crc(&hist, extra);
		result = TEST_RESULT_PASS;
	}

	VPRINT(verbose, "Return value = %d, block = %d\n", result, hist.block);
	VPRINT(verbose, "Histogram stop block = %d\n", hist.block);
	result = ioctl(FB->fb_fd, MSMFB_HISTOGRAM_STOP, &hist.block);
	if (result) {
		VPRINT(verbose, "Histogram stop FAILED! Error = %d\n", result);
		result = TEST_RESULT_FAIL;
	} else {
		VPRINT(verbose, "Histogram stop PASSED\n");
		result = TEST_RESULT_PASS;
	}
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int ret = 0;
	if (thisFBTEST->crcmode == 0)
		ret = crcGen(crc);
	else if (thisFBTEST->crcmode == 1)
		ret = crcValidation(crc);
	if (ret != 0)
		printf("File Access fail");

	return result;
}

long int get_histogram_crc(struct mdp_histogram_data *data, int extra_bins) {
	unsigned int i;
	long int ret = -1;
	long int crc = 0;
	int sumC0e = 0;
	int sumC1e = 0;
	int sumC2e = 0;
	int sumC0o = 0;
	int sumC1o = 0;
	int sumC2o = 0;
	int ravg   = 0;
	int gavg   = 0;
	int bavg   = 0;
	int rgbavg = 0;
	int tempe = 0;
	int tempo = 0;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	if (data) {
		if (data->c0) {
			for (i = 0; i < data->bin_cnt >> 1;) {
				sumC0e += data->c0[i];
				i++;
				sumC0o += data->c0[i];
				i++;
			}
		}
		if (data->block >= MDP_BLOCK_DMA_P) {
			if (data->c1) {
				for (i = 0; i < data->bin_cnt >> 1;) {
					sumC1e += data->c1[i];
					i++;
					sumC1o += data->c1[i];
					i++;
				}
			}
			if (data->c2) {
				for (i = 0; i < data->bin_cnt >> 1;) {
					if ((i % 4) == 0)
						sumC2e += data->c2[i];
					i++;
					sumC2o += data->c2[i];
					i++;
				}
			}
		}
		if ((thisFBTEST->fileParams.inputFormat == MDP_BGR_565)		||
		(thisFBTEST->fileParams.inputFormat == MDP_BGRA_8888)		||
		(thisFBTEST->fileParams.inputFormat == MDP_Y_CBCR_H2V1)		||
		(thisFBTEST->fileParams.inputFormat == MDP_Y_CBCR_H2V2_TILE)	||
		(thisFBTEST->fileParams.inputFormat == MDP_Y_CBCR_H1V1)		||
		(thisFBTEST->fileParams.inputFormat == MDP_Y_CBCR_H2V2)		||
		(thisFBTEST->fileParams.inputFormat == MDP_Y_CB_CR_H2V2)) {
			tempe = sumC0e;
			tempo = sumC0o;
			sumC0e = sumC2e;
			sumC0o = sumC2o;
			sumC2e = tempe;
			sumC2o = tempo;
			if ((sumC0e == sumC2e) && (sumC0o == sumC2o)) {
				sumC0e = (~sumC0e);
				sumC0o = (~sumC0o);
			}
		}

		crc = ((sumC0e + sumC0o) & 0xFF)		|
			(((sumC1e + sumC1o) & 0xFF) << 8)	|
			(((sumC2e + sumC2o) & 0xFF) << 16) ;
		ravg = ((data->extra_info[0] >> 16) & 0x7F) +
			((data->extra_info[0] >> 24) & 0x7F);
		bavg = ((data->extra_info[1] >> 16) & 0x7F) +
			((data->extra_info[1] >> 24) & 0x7F);
		gavg = ((data->extra_info[1] >> 8) & 0x7F) +
			((data->extra_info[1] >> 0) & 0x7F);

		rgbavg = ravg | (gavg << 8) | (bavg << 16);
		printf("rgbavg = 0x%08x \n", rgbavg);
		if (crc) {
			crc    = crc ^ rgbavg;
		} else {
			crc    = crc | rgbavg;
		}
		ret = crc;
		printf("CRC = 0x%08lx \n", ret);
	} else {
		printf("Error: Invalid Histogram Data\n");
	}
	return ret;
}

/***************************************************************/

int doDisplayUpdate (void) {
	int result = -1;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	struct mdp_display_commit commit;
	memset(&commit, 0, sizeof(struct mdp_display_commit));
	commit.flags = MDP_DISPLAY_COMMIT_OVERLAY;

	if (ioctl(FB->fb_fd, MSMFB_DISPLAY_COMMIT, &commit) < 0) {
		printf("ERROR: MSMFB_DISPLAY_COMMIT failed! line=%d\n",
				__LINE__);
		return TEST_RESULT_FAIL;
	}
	if ((thisFBTEST->fBuffer == FB1) && (is_delay_required_fb1)) {
		sleep(6);
		is_delay_required_fb1 = false;
	}
	result = TEST_RESULT_PASS;
	if (interactive) {
		interactiveDelay();
	} else {
		add_delay_in_user_mode();
	}

	if (thisFBTEST->fBuffer == FB0) {
		if (thisFBTEST->misr == 1)
			result = misr(DISPLAY_MISR_DSI0);
		else if (thisFBTEST->misr == 2)
			result = getHistogram((unsigned int) MDP_BLOCK_DMA_P);
	} else if (thisFBTEST->fBuffer == FB1 || thisFBTEST->fBuffer == FB2) {
		if (thisFBTEST->misr == 1) {
			if (writeback_flag != TRUE)
				result = misr(DISPLAY_MISR_HDMI);
		}
	}
	return result;
}

int doPanDisplay (void) {
	int result = -1;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	if (ioctl(FB->fb_fd, FBIOPUT_VSCREENINFO, &FB->fb_vinfo) < 0) {
		printf("ERROR: FBIOPUT_VSCREENINFO failed! line=%d\n",
				__LINE__);
		return TEST_RESULT_FAIL;
	}

	if ((thisFBTEST->fBuffer == FB1) && (is_delay_required_fb1)) {
		sleep(6);
		is_delay_required_fb1 = false;
	}
	result = TEST_RESULT_PASS;
	if (interactive) {
		interactiveDelay();
	} else {
		add_delay_in_user_mode();
	}

	if (thisFBTEST->fBuffer == FB0) {
		if (thisFBTEST->misr == 1)
			result = misr(DISPLAY_MISR_DSI0);
		else if (thisFBTEST->misr == 2)
			result = getHistogram((unsigned int) MDP_BLOCK_DMA_P);
	} else if (thisFBTEST->fBuffer == FB1 || thisFBTEST->fBuffer == FB2) {
		if (thisFBTEST->misr == 1) {
			if (writeback_flag != TRUE)
				result = misr(DISPLAY_MISR_HDMI);
		}
	}
	return result;
}

void add_delay_in_user_mode() {
	struct fbtest_params *thisFBTEST;
	int test_num = -1;
	thisFBTEST = &FBTEST;
	if (interactiveSleep) {
		interactiveDelay();
	} else {
		if (isdigit(thisFBTEST->testname[0])) {
			test_num = atoi(thisFBTEST->testname);
			VPRINT(verbose," look up val for %s is %d ",
				thisFBTEST->testname, test_num);
		} else {
			test_num = lookup_key_case_insensitive(MDP4testNameLUT,
					thisFBTEST->testname, -1);
		}
		switch(test_num) {
			/* No delay required for videoplay / fpsvsync */
            case 2: /* scale */
			case 8: /* videoplay */
			case 9: /* fpsvsync */
			case 14: /* hwCursor */
				break;
			default :
				usleep(TEST_DELAY_SHORT);
				VPRINT(verbose," scalling usleep for %s \n",
					thisFBTEST->testname);
				break;
		}
	}
}

void interactiveDelay() {
	printf("Sleep for(%d sec) ...\n",interactiveSleep);
	sleep(interactiveSleep);
}

int setup_borderfill_pipe() {
	struct mdp_overlay overlay;
	int overlay_id;
	struct mdp_display_commit commit_info;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	memset(&commit_info, 0, sizeof(struct mdp_display_commit));

	overlay.src.format = MDP_RGB_BORDERFILL;
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);
	if (overlay_id < 0) {
		FBTEST_MSG_DEBUG("[BORDERFILL] MSMFB_OVERLAY_SET failed! line=%d err=%d\n",
				__LINE__, overlay_id);
		return -OVERLAY_SET_FAILED;
	}
	FBTEST_MSG_DEBUG("[BORDERFILL] MSMFB_OVERLAY_SET SUCCESS id = %d\n", overlay_id);

	commit_info.flags = MDP_DISPLAY_COMMIT_OVERLAY;
	if (ioctl(FB->fb_fd, MSMFB_DISPLAY_COMMIT, &commit_info) == -1) {
		FBTEST_MSG_DEBUG("[BORDERFILL] MSMFB_DISPLAY_COMMIT for primary failed\
				line=%d err=%d\n", __LINE__, overlay_id);
		return -MSMFB_DISPLAY_COMMIT_FAILED;
	}
	FBTEST_MSG_DEBUG("[BORDERFILL] MSMFB_DISPLAY_COMMIT SUCCESS\n");
	return 0;
}
