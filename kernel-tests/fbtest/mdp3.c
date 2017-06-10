
/******************************************************************************
  @file  mdp3.c
  @brief This file contains test code to verify all functionalities of msm_fb

  DESCRIPTION
  fbtest is msm framebuffer test program.  It opens all frambuffers (/dev/fb*)
  and executes the msm specific fb ioctls as well as the standard linux fb
  ioctls.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

 -----------------------------------------------------------------------------
 Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------------

******************************************************************************/

#include "fbtest.h"
#include "fbtestUtils.h"
#include "mdp3.h"
#include "poll.h"
#include <pthread.h>
#include <stdlib.h>
#ifdef ENABLE_POSTPROC
#include "postproctest.h"
#endif
int testLevel = TEST_LEVEL_DEFAULT;		/* Needs to be unsigned int (-1 used for menu-mode-only tests) */
int testQuickMode = TEST_QUICKMODE_DEFAULT;

#include "yellow_face.zif"
#include "mdp11_Y_10F.zif"
#include "mdp11_CbCr_10F.zif"
#include "cursor.zif"
#define DEST_BUFFERS 2
#define DEST_BUFFER_SIZE  ((FB->fb_vinfo.xres * FB->fb_vinfo.yres* getFormatBpp(FB->fb_fmt)) * DEST_BUFFERS)
int swap = 0;
tests MDP3testFunctions[] = {
	allMDP3Test,
	formatTest,
	scaleTest,
	rotateTest,
	cropTest,
	moveTest,
	ditherTest,
	videoPlayTest,
	fpsVsyncTest,
	blendTest,
	colorkeyTest,
	multiopTest,
	adversarialTest,
	overlayTest,
	CSCTest,
#ifdef ENABLE_POSTPROC
	postprocTest,
#endif
	drawBG,
	calcFPS,
	pppTest,
	allcolorFormatPPPTest,
	pppTestVideo,
	stressTest,
	mddi_partial_update_test,
};

int MDP3TestFuncArraySize = (sizeof(MDP3testFunctions));
extern int fbtest_msg_debug;
extern struct inputFileParams ImgFileParams[4];
void interactiveDelay(void);

struct lookup_string_t MDP3testNameLUT[] =  {
	{ "allmdp3",	0, 0 },
	{ "format",	1, 1 },
	{ "scale",	2, 2 },
	{ "rotate",	3, 3 },
	{ "crop",	4, 4 },
	{ "move",	5, 5 },
	{ "dither",	6, 6 },
	{ "videoplay",	7,7 },
	{ "fpsvsync",	8, 8 },
	{ "blend",	9, 9 },
	{ "colorkey",	10, 10 },
	{ "multiop",	11, 11 },
	{ "adversarial",12, 12 },
	{ "overlay",13,13},
	{ "csc",14,14},
#ifdef ENABLE_POSTPROC
	{ "postproc",15,15},
#endif
/* Legacy MDP3 tests are moved */
	{ "drawBG",		16, 16 },
	{ "CalcFPS",		17, 17 },
	{ "PPP",		18, 18 },
	{ "allColorformatPPP",	19, 19 },
	{ "pppTestVideo",	20, 20 },
	{ "stress",		21, 21 },
	{ "mddiPartial",	22, 22 },
};

int mdp3AllocMEM(unsigned int srcSize, unsigned int destSize);

int MDP3TestNameArraySize = (sizeof(MDP3testNameLUT));

int allMDP3Test(void) {
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	fpsVsyncTest();
	videoPlayTest();
	adversarialTest();
	mddi_partial_update_test();
	rotateTest();
	scaleTest();
	formatTest();
	blendTest();
	cropTest();
	multiopTest();
	moveTest();
	return TEST_RESULT_PASS;
}

int getMaxUpScale(int  mdp_version) {
	int scale = 0;
	if (mdp_version >= MDP_V3_1)
		scale = 8;
	else
		scale = 4;
	return scale;
}
int getMaxDownScale(int  mdp_version) {
	int scale = 0;
	if (mdp_version >= MDP_V3_1)
		scale = 8;
	else
		scale = 4;
	return scale;
}
pthread_mutex_t mutex;
pthread_cond_t cond;
int triggered;

void *readVsync(void *ptr) {
	int i = 0, fd, max_data = 64;
	struct pollfd *pfd;
	char timestamp[64];
	struct timeval time_start, time_stop;
	pthread_t id = pthread_self();
	fd = open(ptr, O_RDONLY);
	if (fd < 0) {
		/* Make sure fb device is opened before starting this thread*/
		VPRINT(verbose, "\n%s:not able to open vsync node, %s",
			__FUNCTION__, strerror(errno));
		return NULL;
	}

	pfd = (struct pollfd *)malloc(sizeof(struct pollfd));
	if (pfd == NULL)
		VPRINT(verbose, " %s Malloc failed\n", __FUNCTION__);
	pfd->fd = fd;
	VPRINT(verbose, "fd : %d\n", pfd->fd);
	if (pfd->fd >= 0)
		pfd->events = POLLPRI | POLLERR;
	do {
		gettimeofday(&time_start,NULL);
		int err = poll(pfd, 1, -1);
		if(err > 0) {
			if (pfd->revents & POLLPRI) {
				VPRINT(verbose, "return event %d\n", pfd->revents);
				int len = pread(pfd->fd, timestamp, max_data, 0);
				if (!strncmp(timestamp, "VSYNC=", strlen("VSYNC="))) {
					VPRINT(verbose, "timestamp = %llu\n",
						strtoull(timestamp + strlen("VSYNC="), NULL, 0));
				}
				pthread_mutex_lock(&mutex);
				triggered = 1;
				pthread_cond_signal(&cond);
				pthread_mutex_unlock(&mutex);
			}
			gettimeofday(&time_stop,NULL);
			VPRINT(verbose, "\n*****Poll Timestamp = %f fps*********\n",
			(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
				time_start.tv_sec*1000-time_start.tv_usec/1000));
		}
	} while(1);
	return NULL;
}

void doPoll(void) {
	int  iret;
	pthread_t vsyncPoll;
	char const*const SYSFS
		= "/sys/class/graphics/fb0/vsync_event";
	pthread_mutex_init(&mutex, 0);
	pthread_cond_init(&cond, 0);
	triggered = 0;
	iret = pthread_create(&vsyncPoll, NULL, readVsync, (void*)SYSFS);
}

/* Check for video format alignment to avoid MDP hang */
int checkVideoFormatAlignment (void) {
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	if ((thisFBTEST->fileParams.inputFormat==MDP_Y_CBCR_H2V2) ||
		(thisFBTEST->fileParams.inputFormat==MDP_Y_CRCB_H2V2)) {
		if (thisFBTEST->fileParams.inResolution.width%2 ||
			thisFBTEST->fileParams.inResolution.height%2 ||
			thisFBTEST->outResolution.width%2 ||
			thisFBTEST->outResolution.height%2) {
			if (thisFBTEST->fileParams.inResolution.width % 2)
				thisFBTEST->fileParams.inResolution.width -= 1;
			if (thisFBTEST->fileParams.inResolution.height % 2)
				thisFBTEST->fileParams.inResolution.height -= 1;
			if (thisFBTEST->outResolution.width % 2)
				thisFBTEST->outResolution.width -= 1;
			if (thisFBTEST->outResolution.height % 2)
				thisFBTEST->outResolution.height -= 1;
			FBTEST_MSG_DEBUG("Both source and destination dimensions\
					are adjusted to be even for %d Format \n",
					thisFBTEST->fileParams.inputFormat );
		}
	}
	return TEST_ALIGNMENT_CKECK_PASS;
}

/* Read RAW input image from file system and loads in ION buffer */
void loadImage (void) {
	int result, image_size, bytesRead = 0;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	FILE *fptr = NULL;
	FBTEST_MSG_DEBUG("[LoadImage] open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);

	image_size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,
			thisFBTEST->fileParams.inputFormat);

	fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
	if (fptr == NULL) {
		FBTEST_MSG_DEBUG("[LoadImage] Cannot open raw Image file %s !\n",
				thisFBTEST->fileParams.filenamePath);
		showError(-FOPEN_FAILED, 1);
	}
	FBTEST_MSG_DEBUG("[LoadImage] Open raw Image file %s !\n",
			thisFBTEST->fileParams.filenamePath);

	/* allocate ION buffer */
	if (mdp3AllocMEM(image_size, DEST_BUFFER_SIZE)) {
		FBTEST_MSG_DEBUG("[FORMAT] mdp3mdp3AllocMEM failed! (Line %d)\n",__LINE__);
		showError(-PMEMALLOC_FAILED, 1);
	} else {
		result = TEST_RESULT_FAIL;
	}
	/* copy image data to ION buffer */
	FBTEST_MSG_DEBUG("[FORMAT] mdp3AllocMEM SUCCESS! (Line %d)\n",__LINE__);

	/* read image in buffer */
	bytesRead  = fread(MEM->mem_buf_src,1, image_size,fptr);
	/* close the file */
	fclose(fptr);
	if (bytesRead == 0 || bytesRead != image_size) {
		FBTEST_MSG_DEBUG("[FORMAT] fread failed! (Line %d) for file %s\n",
				__LINE__, thisFBTEST->fileParams.filenamePath);
		showError(-FREAD_FAILED, 1);
	}
	FBTEST_MSG_DEBUG("[FORMAT] LOAD IMG OVER\n");
}

/* Calls MSMFB_OVERLAY_PLAY and MSMFB_DISPLAY_COMMIT ioctl */
int commit_overlay(struct msmfb_overlay_data* ptr_ovdata) {
	struct mdp_display_commit commit_info;
	int result;
	memset(&commit_info, 0, sizeof(struct mdp_display_commit));
	commit_info.flags = MDP_DISPLAY_COMMIT_OVERLAY;
	VPRINT(verbose,"offset %u", ptr_ovdata->data.offset);

	result = doPlay(FB, ptr_ovdata);
	if (result < 0)
		return result;
	result = doCommit(FB, &commit_info);
	if (result < 0)
		return result;
	return MSMFB_DISPLAY_COMMIT_PASS;
}
/* Calls MSMFB_BLIT ioctl */
int doBlit(struct mdp_blit_req_list *req_list) {
	struct timeval blit_start, blit_stop;
	struct mdp_buf_sync *bufsync;
	int acq_fen_fd[MDP_MAX_FENCE_FD], rel_fen_fd = -1;
	gettimeofday(&blit_start,NULL);
	if (!async) {
		VPRINT(verbose, "Sync blit");
		if (ioctl(FB->fb_fd, MSMFB_BLIT, req_list) < 0)
		return -BLIT_FAILED;
	}
	else {
		union {
		char dummy[sizeof(struct mdp_async_blit_req_list) + sizeof(struct mdp_blit_req)*1];
		struct mdp_async_blit_req_list list;
		} imgFrame;
		struct mdp_async_blit_req_list *list = &(imgFrame.list);
		struct mdp_blit_req *blitReq, *req;
		imgFrame.list.count = 1;
		/* Setting Blit req parameters */
		blitReq = &imgFrame.list.req[0];
		req = &req_list->req[0];
		memcpy(blitReq, req, sizeof(struct mdp_blit_req));

		/* Setting the fence parameters */
		bufsync = &imgFrame.list.sync;
		bufsync->flags = MDP_BUF_SYNC_FLAG_WAIT;
		bufsync->acq_fen_fd_cnt = 0;
		bufsync->acq_fen_fd = acq_fen_fd;
		bufsync->rel_fen_fd = &rel_fen_fd;
		VPRINT(verbose, "ASync blit");
		if (ioctl(FB->fb_fd, MSMFB_ASYNC_BLIT, list) < 0)
		return -BLIT_FAILED;
	}
	gettimeofday(&blit_stop,NULL);
	VPRINT(verbose, "[BLIT Time:%f]  Start = %f Stop = %f\n",
		(float)(blit_stop.tv_sec*1000+blit_stop.tv_usec/1000 -
		blit_start.tv_sec*1000-blit_start.tv_usec/1000),
		(float)(blit_start.tv_usec/1000),
		(float)(blit_stop.tv_usec/1000));
	return BLIT_PASS;
}

char *constructDumpFileName(void) {
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	char *filename, height[20], width[20], format[20];
	int length, h, w, fmt, test;
	snprintf(height, sizeof(int), "%d", FB->fb_vinfo.yres);
	h = strlen(height);
	VPRINT(verbose, "height = %d", h);
	snprintf(width,  sizeof(int),"%d", FB->fb_vinfo.xres);
	w = strlen(width);
	snprintf(format, sizeof(int), "%d", FB->fb_fmt);
	fmt = strlen(format);
	test = strlen(thisFBTEST->testname);
	length = strlen("data/destDump_") + test + strlen("_") + w + strlen("x") + h +
			 strlen("_") + fmt + strlen(".rgb");
	VPRINT(verbose, "\ndump length = %d\n", length);
	filename = (char *)malloc(length);
	if(!filename) {
		VPRINT(verbose, "%s Malloc Failed", __func__);
		return NULL;
	}
	memset(filename, 0x00, length);
	strncat(filename, "data/destDump_", strlen("data/destDump_"));
	strncat(filename, thisFBTEST->testname, test);
	strncat(filename, "_", strlen("_"));
	strncat(filename, width, w);
	strncat(filename, "x", strlen("x"));
	strncat(filename, height, h);
	strncat(filename, "_", strlen("_"));
	strncat(filename, format, fmt);
	strncat(filename, ".rgb", strlen(".rgb"));
	return filename;
}
void doDestinationCopy(int size) {
	char *filename;
	static int call_count = 0;
	 if (dump) {
		 filename = constructDumpFileName();
		 VPRINT(verbose, "filename = %s", filename);
		  if (!filename) {
			VPRINT(verbose, "Failed to calculate Dump File name\n");
			showError(TEST_RESULT_FAIL, 1);
		 }
		 FILE* fd;
		 if (call_count == 0) {
			 fd = fopen(filename, "w");
			 fwrite(MEM->mem_buf + size,
					 FB->fb_vinfo.xres * FB->fb_vinfo.yres * getFormatBpp(FB->fb_fmt),
					 1, fd);
			 fclose(fd);
		 }else {
			 fd = fopen(filename, "a+");
			 fwrite(MEM->mem_buf + size,
					 FB->fb_vinfo.xres * FB->fb_vinfo.yres * getFormatBpp(FB->fb_fmt),
					 1, fd);
			 fclose(fd);
		 }
	 }
	++call_count;
}
/* Calls commit_overlay and MISR functions */
int doDisplayUpdate(void) {
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int result=TEST_RESULT_PASS;
	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	int total,size, test_num;
	struct mdp_display_commit commit_info;

	thisFBTEST = &FBTEST;
	total = MEM->mem_size;
	ovdata.id = MSMFB_NEW_REQUEST;
	ovdata.data.flags = 0;
	int offset;
	if (swap % DEST_BUFFERS) {
		size = total-((FB->fb_vinfo.xres * FB->fb_vinfo.yres * getFormatBpp(FB->fb_fmt)) * DEST_BUFFERS);
		offset = total-(FB->fb_vinfo.xres * FB->fb_vinfo.yres * getFormatBpp(FB->fb_fmt));
	} else {
		size = total-(FB->fb_vinfo.xres * FB->fb_vinfo.yres * getFormatBpp(FB->fb_fmt));
		offset = total-((FB->fb_vinfo.xres * FB->fb_vinfo.yres * getFormatBpp(FB->fb_fmt)) * DEST_BUFFERS);
	}
	VPRINT(verbose, "swap in image update = %d\n", swap);
	swap = !swap;

	ovdata.data.offset = size;
	ovdata.data.memory_id = MEM->mem_fd;
	memset(&commit_info, 0, sizeof(struct mdp_display_commit));
	commit_info.flags = MDP_DISPLAY_COMMIT_OVERLAY;
	VPRINT(verbose,"offset %u\n", ovdata.data.offset);

	result = doPlay(FB, &ovdata);
	if (result < 0)
		return result;
	result = doCommit(FB, &commit_info);
	if(result < 0)
		return result;

	VPRINT(verbose, "Commit Successful\n");
	add_delay_in_user_mode();
	if(thisFBTEST->misr) {
		misr(DISPLAY_MISR_DSI0);
	}
	if (isdigit(thisFBTEST->testname[0])) {
		test_num = atoi(thisFBTEST->testname);
		VPRINT(verbose,"look up val for %s is %d \n", thisFBTEST->testname,
			test_num);
	} else {
		test_num = lookup_key_case_insensitive (MDP3testNameLUT,
				thisFBTEST->testname, -1);
	}

	doDestinationCopy(size);
	/*Do not clear destination buffer for videoplay / fpsvsync / blend test */
	switch (test_num)
	{
		case 7: /* videoplay */
		case 8: /* fpsvsync */
		case 9: /* blend */
		case 10: /*colorkey*/
			break;
		default :
			memset(MEM->mem_buf + offset, 0x00, DEST_BUFFER_SIZE/2);
			VPRINT(verbose,"Calling memset for %s \n",
				thisFBTEST->testname);
			 break;
	}
	return result;
}

void add_delay_in_user_mode () {
	struct fbtest_params *thisFBTEST;
	int test_num = -1;
	thisFBTEST = &FBTEST;
	if (interactiveSleep) {
		interactiveDelay();
	} else {
		if (isdigit(thisFBTEST->testname[0])) {
			test_num = atoi(thisFBTEST->testname);
			VPRINT(verbose,"look up val for %s is %d \n",
				thisFBTEST->testname, test_num);
		} else {
			test_num = lookup_key_case_insensitive(MDP3testNameLUT,
					thisFBTEST->testname, -1);
		}
		switch(test_num) {
			/* No delay required for videoplay / fpsvsync */
			case 7: /* videoplay */
			case 8: /* fpsvsync */
				break;
			case 21: /* stress */
				usleep(16000);
				VPRINT(verbose,"Calling usleep for %s \n",
					thisFBTEST->testname);
				break;
			default :
				usleep(TEST_DELAY_SHORT);
				VPRINT(verbose,"Calling usleep for %s \n",
					thisFBTEST->testname);
				break;
		}
	}
}

/* sleep for n seconds specified with -kn (-k2 sleep for 2 second) */
void interactiveDelay() {
	VPRINT(verbose, "Sleep for(%d sec) ...\n",interactiveSleep);
	sleep(interactiveSleep);
}

void setBlitParam(struct mdp_blit_req *blitReq, struct inputFileParams fileParams) {
	blitReq->src.width  = fileParams.inResolution.width;
	blitReq->src.height = fileParams.inResolution.height;
	blitReq->src.format = fileParams.inputFormat;
	blitReq->src.offset = 0;
	blitReq->src.memory_id = MEM->mem_fd_src;
	blitReq->dst.width  = FB->fb_vinfo.xres;
	blitReq->dst.height = FB->fb_vinfo.yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
				getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
	blitReq->dst.memory_id = MEM->mem_fd;
	blitReq->transp_mask = MDP_TRANSP_NOP;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = fileParams.inResolution.width;
	blitReq->dst_rect.h = fileParams.inResolution.height;
	blitReq->src_rect.w = fileParams.inResolution.width;
	blitReq->src_rect.h = fileParams.inResolution.height;
}

int colorkeyTest (void) {
	int result = TEST_RESULT_PASS, bg_set = 0, max_img_size = 0;
	unsigned int i = 0, j = 0, index = 0;
	unsigned int inFileCnt = 0;
	FILE *fptr = NULL;
	size_t bytesRead = 0;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	struct inputFileParams tmpFileParams;
	int ImgFrameSize[2];
	unsigned char *ImgBuff[2];
	int size;
	int ImgDataOffset[2] = {0,0};

	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);

	if (thisFBTEST->testMode == AUTO) {
		VPRINT(verbose, "Color keying doesn't support Auto mode as it \
				required manually setting with exact transparency value\n");
		return TEST_RESULT_SKIP;
	}

	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;

	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;

	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	for (i = 0; i < 2; i++) {
		if (strncmp(thisFBTEST->blend.ImagePath[i], "",strlen("")+1)) {
			result = parseFileName(thisFBTEST->blend.ImagePath[i],
				&ImgFileParams[inFileCnt++]);
			if (result < 0)
				showError(result, 1);
		}
	}

	for (i = 0; i < inFileCnt; i++) {
		if (strncmp(thisFBTEST->blend.ImagePath[i], "",strlen("")+1)) {
			ImgFrameSize[i] = getFrameSize(ImgFileParams[i].inResolution.width,\
					ImgFileParams[i].inResolution.height,
					ImgFileParams[i].inputFormat);
			size = ImgFrameSize[i];
			if (max_img_size < size)
				max_img_size = size;
			/* allocate buffer for reading data from input image */
			ImgBuff[i] = (unsigned char *)malloc(ImgFrameSize[i]);
			if (ImgBuff[i] == NULL)
				showError(-MALLOC_FAILED, 1);
			/* open image file */
			fptr = fopen(ImgFileParams[i].filenamePath, "rb");

			if (fptr == NULL) {
				FBTEST_MSG_DEBUG("[Blend Test] Cannot open raw Image file!\n");
				for (j = 0; j <= i; j++)
					free(ImgBuff[i]);
				return -FOPEN_FAILED;
			}
			/* read image in buffer */
			bytesRead  = fread(ImgBuff[i], 1, ImgFrameSize[i], fptr);

			if (bytesRead == 0) {
				FBTEST_MSG_DEBUG("[Blend Test] fread failed! (Line %d)\n",__LINE__);
				for (j = 0; j <= i; j++)
					free(ImgBuff[i]);
				return -FREAD_FAILED;
			}
			/* close the file */
			fclose(fptr);
		}
	}
	if (mdp3AllocMEM(max_img_size, DEST_BUFFER_SIZE)) {
		FBTEST_MSG_DEBUG("[Color Key] mdp3AllocMEM failed! (Line %d)\n", __LINE__);
        for (i = 0; i < inFileCnt; i++)
            free(ImgBuff[i]);
		return -PMEMALLOC_FAILED;
	}

	/* for BG color keying,choose BG image which is of frame buffer format */
	if (colorkeyflag && (thisFBTEST->fBuffer == FB0)) {
		if (ImgFileParams[0].inputFormat == FB->fb_fmt) {
			bg_set = 1;

		} else if ((ImgFileParams[1].inputFormat == FB->fb_fmt) && !bg_set) {
			FBTEST_MSG_DEBUG("swapping file for BG color keying \n");
			memcpy(&ImgFileParams[1], &tmpFileParams,
				sizeof(struct inputFileParams));
		} else {
			FBTEST_MSG_DEBUG("for BG color keying one test image\
					 should have frame buffer format\n");
			return -INVALID_COLORKEY_IMAGE_FORMAT;
		}
	} else {
		index = 1;/* increment the index of file next to play */
		sleep(3);
		//drawBG();
		ImgDataOffset[1] = ImgFrameSize[0];
		/* setting transparency value */
	}

	int dst_w, dst_h;
	struct mdp_overlay overlay;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	/* set & play blitReqs  */
	for(i = 0;i < inFileCnt;i++) {
		/* copy image data to PMEM buffer  */
		memset(MEM->mem_buf_src, 0x00, max_img_size);
		memcpy(MEM->mem_buf_src, ImgBuff[i], (int)(ImgFrameSize[i]));
		memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
		blitReq->src.memory_id = MEM->mem_fd_src;
		blitReq->dst.width  = vinfo->xres;
		blitReq->dst.height = vinfo->yres;
		blitReq->dst.format = FB->fb_fmt;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres * getFormatBpp(FB->fb_fmt)) * DEST_BUFFERS);
		blitReq->dst.memory_id = MEM->mem_fd;
		blitReq->transp_mask = MDP_TRANSP_NOP;
		blitReq->src.width  = ImgFileParams[i].inResolution.width;
		blitReq->src.height = ImgFileParams[i].inResolution.height;
		blitReq->src.format = ImgFileParams[i].inputFormat;
		blitReq->src_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->src_rect.h = ImgFileParams[i].inResolution.height;
		blitReq->dst_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->dst_rect.h = ImgFileParams[i].inResolution.height;
		blitReq->alpha = 0xff;/* made complete opaque  */
		if (i==0)
			blitReq->transp_mask=MDP_TRANSP_NOP;
		else if (i==1)
			blitReq->transp_mask = thisFBTEST->transparency;
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		sleep(2);
	}
	VPRINT(verbose, "Colorkey test over\n");
	unsetOverlay(FB, &overlay.id);
	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Color Key blitReq Test: Pass \n");
	} else {
		VPRINT(verbose, "Color Key blitReq Test: Fail \n");
        }
	free(ImgBuff[0]);
	free(ImgBuff[1]);
	return result;
}

int overlayTest (void) {
	int result, overlay_id, buf_size;
	struct mdp_overlay overlay;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	struct fb_var_screeninfo *vinfo;
	vinfo = &(FB->fb_vinfo);
	size_t bytesRead = 0;

	FILE *fptr = NULL;
	unsigned char *buff;
	buf_size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,thisFBTEST->fileParams.inputFormat);

	/* open image file */
	VPRINT(verbose, "file=%s\n", thisFBTEST->fileParams.filenamePath);
	fptr = fopen(thisFBTEST->fileParams.filenamePath, "rb");
	if (fptr == NULL) {
		FBTEST_MSG_DEBUG("[FORMAT] Cannot open raw Image file!\n");
		return -FOPEN_FAILED;
	}

	/* allocate PMEM buffer */
	if (mdp3AllocMEM(buf_size, DEST_BUFFER_SIZE)) {
		FBTEST_MSG_DEBUG("[FORMAT] mdp3AllocMEM failed! (Line %d)\n", __LINE__);
		return -PMEMALLOC_FAILED;
		result = TEST_RESULT_PASS;
	}
	else
		result = TEST_RESULT_FAIL;

	/* read image in buffer */
	bytesRead  = fread(MEM->mem_buf_src,1, buf_size, fptr);
	if (bytesRead == 0) {
		FBTEST_MSG_DEBUG("[FORMAT] fread failed! (Line %d)\n", __LINE__);
		return -FREAD_FAILED;
	}

	/* close the file  */
	fclose(fptr);


	memset(&overlay, 0, sizeof(struct mdp_overlay));
	/* drawBG(); */
	VPRINT(verbose, "overlayTest: addr=%x size=%d\n", (int)MEM->mem_buf, FB->fb_size);

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
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.flags = 0;
	overlay.id = MSMFB_NEW_REQUEST;

	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;

	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;

	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	setBlitParam(blitReq, thisFBTEST->fileParams);

	result = TEST_RESULT_PASS;
	if ((result = doBlit(list)) < 0) {
		VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
		return -BLIT_FAILED;
	}

	if (FB->fb_vinfo.bits_per_pixel == 32)
		overlay.is_fg = 0;
	else {
		overlay.is_fg = 1;
		overlay.alpha = 0xFF;
	}
	overlay.id = 1;
	overlay_id = ioctl(FB->fb_fd, MSMFB_OVERLAY_SET, &overlay);

	VPRINT(verbose, "MSMFB_OVERLAY_SET: id=%d\n", overlay.id);

	if (overlay_id < 0) {
		VPRINT(verbose, "ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n",__LINE__, overlay_id);
		return TEST_RESULT_FAIL;
	}

	overlay_id = overlay.id;
	result = TEST_RESULT_PASS;
	result = doDisplayUpdate();

	if (result == TEST_RESULT_PASS)
	{
		VPRINT(verbose, "Overlay Test: Successful \n");
	} else {
		VPRINT(verbose, "Overlay Test: Fail \n");
	}
	return result;
}

int ditherTest(void) {
	int size, dst_w, dst_h, result= TEST_RESULT_PASS;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	struct mdp_overlay overlay;
	thisFBTEST = &FBTEST;
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,
			thisFBTEST->fileParams.inputFormat);
	if (checkVideoFormatAlignment()) {
		return result;
	}
	vinfo = &(FB->fb_vinfo);
	loadImage();
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	setBlitParam(blitReq, thisFBTEST->fileParams);
	blitReq->flags = MDP_DITHER;

	result = TEST_RESULT_PASS;
	result = ioctl(FB->fb_fd, MSMFB_BLIT, list);

	if ((result = doBlit(list)) < 0) {
		VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
		return -BLIT_FAILED;
	}

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	result = setOverlay(&overlay,FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	result = doDisplayUpdate();
	unsetOverlay(FB, &overlay.id);
	return result;
}

int fpsVsyncTest(void) {
	int iterations, inFileCnt = 0, i = 0, j = 0, size[2], max_size=0, result = TEST_RESULT_PASS, ImgFrameSize[2], max_img_size;
	struct mdp_overlay overlay;
	FILE *fptr = NULL;
	unsigned char * yptr, *uvptr;
	int fd;
	unsigned char *ImgBuff[2];
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	struct inputFileParams tmpFileParams;
	struct timeval time_start, time_stop,time_cur,time_cur1;
	struct fb_var_screeninfo vinfo_bak;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	size_t bytesRead = 0;
	if (checkVideoFormatAlignment())
		return result;

	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);

	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 50;

	if (strncmp(thisFBTEST->blend.ImagePath[0], "yellow_face.zif",
		strlen("yellow_face.zif") + 1)) {
		max_size = FACE_WIDTH * FACE_HEIGHT * getFormatBpp(FACE_FORMAT);
		ImgBuff[0] = (unsigned char *)malloc(max_size);
		ImgFileParams[0].inResolution.width = FACE_WIDTH;
		ImgFileParams[0].inResolution.height = FACE_HEIGHT;
		ImgFileParams[0].inputFormat = FACE_FORMAT;
		size[0] = ImgFrameSize[0] = max_size;
		memcpy(ImgBuff[0], yellow_face_data, max_size);

		max_size = YUV_WIDTH * YUV_HEIGHT * getFormatBpp(YUV_FORMAT);
		ImgBuff[1] = (unsigned char *)malloc(max_size);
		memcpy(ImgBuff[1], yellow_face_data, max_size);

		ImgFileParams[1].inResolution.width = YUV_WIDTH;
                ImgFileParams[1].inResolution.height = YUV_HEIGHT;
                ImgFileParams[1].inputFormat = YUV_FORMAT;
		size[1] = ImgFrameSize[1] = max_size;

                yptr = uvptr = (unsigned char *)ImgBuff[1];
                uvptr += YUV_WIDTH*YUV_HEIGHT;
                memcpy(yptr, (unsigned char *)&mdp11_Y[0] + j*(YUV_WIDTH*YUV_HEIGHT), YUV_WIDTH*YUV_HEIGHT);
                memcpy(uvptr, (unsigned char *)&mdp11_CbCr[0] + j*((YUV_WIDTH*YUV_HEIGHT)/2), (YUV_WIDTH*YUV_HEIGHT)/2);

	} else {
		for(i=0; i < 2; i++) {
			if (strncmp(thisFBTEST->blend.ImagePath[i], "",strlen("")+1)) {
				result = parseFileName(thisFBTEST->blend.ImagePath[i], &ImgFileParams[inFileCnt++]);
				if (result < 0)
					showError(result, 1);
			}
		}

		for (i=0; i < inFileCnt; i++) {
			if (strncmp(thisFBTEST->blend.ImagePath[i], "",strlen("")+1)) {
				ImgFrameSize[i] = getFrameSize(ImgFileParams[i].inResolution.width,\
						ImgFileParams[i].inResolution.height,ImgFileParams[i].inputFormat);
				size[i] = ImgFrameSize[i];
				VPRINT(verbose, "reading size[i]=%d\n",size[i]);
				if (max_size<size[i])
					max_size=size[i];
				/* allocate buffer for reading data from input image */
				ImgBuff[i] = (unsigned char *)malloc(ImgFrameSize[i]);
				if (ImgBuff[i] == NULL)
					showError(-MALLOC_FAILED, 1);
				/* open image file */
				fptr = fopen(ImgFileParams[i].filenamePath, "rb");
				if (fptr == NULL) {
					FBTEST_MSG_DEBUG("[Blend Test] Cannot open raw Image file!\n");
					for (j = 0; j <= i; j++)
						free(ImgBuff[j]);
					return -FOPEN_FAILED;
				}
				/* read image in buffer */
				bytesRead  = fread(ImgBuff[i], 1, ImgFrameSize[i], fptr);
			if (bytesRead == 0) {
				FBTEST_MSG_DEBUG("[Blend Test] fread failed! (Line %d)\n",__LINE__);
				for (j = 0; j <= i; j++)
					free(ImgBuff[j]);
				return -FREAD_FAILED;
			}
			/* close the file */
			fclose(fptr);
			}
		}
	}
	max_img_size = max_size;
	if (mdp3AllocMEM(max_img_size, DEST_BUFFER_SIZE)) {
		FBTEST_MSG_DEBUG("[FORMAT] mdp3AllocMEM failed! (Line %d)\n",__LINE__);
		for (i = 0; i < inFileCnt; i++)
			free(ImgBuff[i]);
		return -PMEMALLOC_FAILED;
	}
	vinfo_bak = FB->fb_vinfo;
	vinfo = &vinfo_bak;
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	VPRINT(verbose, "CalcFPS test\n");
	/* FPS with VSync ON */
	vinfo->activate = FB_ACTIVATE_VBL;
	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_VSYNC_CTRL,&vinfo->activate))
		VPRINT(verbose,"Vsync ioctl failed\n");
	gettimeofday(&time_start,NULL);

	if (async)
		doPoll();
	for(j=0;j<iterations;j++) {
		i=j%2;
		if (async) {
			pthread_mutex_lock(&mutex);
			while (!triggered)
				pthread_cond_wait(&cond, &mutex);

			pthread_mutex_unlock(&mutex);
		}
		memcpy(MEM->mem_buf_src, ImgBuff[i], (int)(size[i]));
		thisFBTEST->fileParams.inResolution.width = ImgFileParams[i].inResolution.width;
		thisFBTEST->fileParams.inResolution.height = ImgFileParams[i].inResolution.height;
		thisFBTEST->fileParams.inputFormat = ImgFileParams[i].inputFormat;
		blitReq->src.width  = ImgFileParams[i].inResolution.width;
		blitReq->src.height = ImgFileParams[i].inResolution.height;
		blitReq->src.format = ImgFileParams[i].inputFormat;
		blitReq->src.memory_id = MEM->mem_fd_src;
		blitReq->dst.width  = vinfo->xres;
		blitReq->dst.height = vinfo->yres;
		blitReq->dst.format = FB->fb_fmt;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		blitReq->dst.memory_id = MEM->mem_fd;
		blitReq->transp_mask = MDP_TRANSP_NOP;
		blitReq->alpha  = MDP_TRANSP_NOP; /* thisFBTEST->blend.ImgAlphaVal[i]; */
		blitReq->dst_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->dst_rect.h = ImgFileParams[i].inResolution.height;
		blitReq->src_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->src_rect.h = ImgFileParams[i].inResolution.height;

		result = TEST_RESULT_PASS;
		thisFBTEST->frameNum = i;
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		if (j==0)
			setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
				thisFBTEST->fileParams.inResolution.height, 0,
				0, blitReq->dst_rect.w, blitReq->dst_rect.h, 0, 0,
				FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0, MSMFB_NEW_REQUEST, 0);
		result = doDisplayUpdate();
		memset(MEM->mem_buf_src,0x00,(int)(size[i]));
		if (async) {
			pthread_mutex_lock(&mutex);
			triggered = 0;
			pthread_mutex_unlock(&mutex);
		}
		// VPRINT(verbose, "\nTime for each iteration = %f fps\n",
			// (float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
				// time_start.tv_sec*1000-time_start.tv_usec/1000))
	}
	gettimeofday(&time_stop,NULL);
	VPRINT(verbose, "Average FPS with VSync ON = %.1f fps\n",
			(float)(iterations*1000)/
			(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
				time_start.tv_sec*1000-time_start.tv_usec/1000));
	sleep(2);
	/*FPS with VSync OFF*/
	vinfo->activate = FB_ACTIVATE_NOW;

	memset(MEM->mem_buf, 0x00, DEST_BUFFER_SIZE);
	gettimeofday(&time_start,NULL);
	for (j = 0;j < iterations;j++) {
		i=j%2;
		memcpy(MEM->mem_buf_src, ImgBuff[i], (int)(size[i]));
		thisFBTEST->fileParams.inResolution.width = ImgFileParams[i].inResolution.width;
		thisFBTEST->fileParams.inResolution.height = ImgFileParams[i].inResolution.height;
		thisFBTEST->fileParams.inputFormat = ImgFileParams[i].inputFormat;
		blitReq->src.width  = ImgFileParams[i].inResolution.width;
		blitReq->src.height = ImgFileParams[i].inResolution.height;
		blitReq->src.format = ImgFileParams[i].inputFormat;
		blitReq->src.memory_id = MEM->mem_fd_src;
		blitReq->dst.width  = vinfo->xres;
		blitReq->dst.height = vinfo->yres;
		blitReq->dst.format = FB->fb_fmt;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
						getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
		blitReq->dst.memory_id = MEM->mem_fd;
		blitReq->transp_mask = MDP_TRANSP_NOP;
		blitReq->alpha  = MDP_TRANSP_NOP;/*thisFBTEST->blend.ImgAlphaVal[i];*/
		blitReq->dst_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->dst_rect.h = ImgFileParams[i].inResolution.height;
		blitReq->src_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->src_rect.h = ImgFileParams[i].inResolution.height;

		result = TEST_RESULT_PASS;
		thisFBTEST->frameNum = i;
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		memset(MEM->mem_buf_src, 0x00, (int)(size[i]));
	}
	gettimeofday(&time_stop,NULL);
	VPRINT(verbose, "Average FPS with VSync OFF = %.1f fps\n",
			(float)(iterations*1000)/
			(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
				time_start.tv_sec*1000-time_start.tv_usec/1000));
	FB->fb_vinfo = vinfo_bak;
	unsetOverlay(FB, &overlay.id);
	for (i = 0; i < inFileCnt; i++)
		free(ImgBuff[i]);
	return TEST_RESULT_PASS;
}

int doCrop(void) {
	int result = TEST_RESULT_PASS, dst_w, dst_h;
	struct fb_var_screeninfo *vinfo;
	FILE *fptr = NULL;
	size_t bytesRead = 0;
	char *buff;
	struct cropParams *thisCrop;
	struct fbtest_params *thisFBTEST;
	unsigned int h, w;
	union
	{
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	struct mdp_overlay overlay;

	thisCrop = &CROPTEST;
	thisFBTEST = &FBTEST;
	vinfo = &(FB->fb_vinfo);
	loadImage();
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	setBlitParam(blitReq, thisFBTEST->fileParams);

	/* For CRC capture without crop */
	w = thisCrop->resolution.width;
	h = thisCrop->resolution.height;
	thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width;
	thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height;

	if ((result = doBlit(list)) < 0) {
		VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
		return -BLIT_FAILED;
	}
	result = doDisplayUpdate();
	/* drawBG(); */

	result = TEST_RESULT_PASS;

	/********starting crop test**************************/
	/* For CRC capture with crop */
	thisCrop->resolution.width = w;
	thisCrop->resolution.height = h;

	blitReq->src.width  = thisFBTEST->fileParams.inResolution.width;
	blitReq->src.height = thisFBTEST->fileParams.inResolution.height;
	blitReq->src.format = thisFBTEST->fileParams.inputFormat;
	blitReq->src.memory_id = MEM->mem_fd_src;
	blitReq->dst.width  = vinfo->xres;
	blitReq->dst.height = vinfo->yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
				getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
	blitReq->dst.memory_id = MEM->mem_fd;
	blitReq->transp_mask = MDP_TRANSP_NOP;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.x = thisCrop->dst_rect.x;
	blitReq->dst_rect.y = thisCrop->dst_rect.y;
	blitReq->dst_rect.w = thisCrop->resolution.width;
	blitReq->dst_rect.h = thisCrop->resolution.height;
	blitReq->src_rect.x = thisCrop->src_rect.x;
	blitReq->src_rect.y = thisCrop->src_rect.y;
	blitReq->src_rect.w = thisCrop->resolution.width;
	blitReq->src_rect.h = thisCrop->resolution.height;

	if ((result = doBlit(list)) < 0) {
		VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
		return -BLIT_FAILED;
	}

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, blitReq->dst_rect.w, blitReq->dst_rect.h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	result = doDisplayUpdate();
	unsetOverlay(FB, &overlay.id);
	return result;
}

int cropCalculation(void) {
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
	} else if((crop_percentage & (1 << C_TOP))) {
		VPRINT(verbose, "\nCropping Top-Half of Input Image\n");
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height * .5;
	} else if((crop_percentage & (1 << C_LEFT))) {
		VPRINT(verbose, "\nCropping Left-Half of Input Image\n");
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width * .5;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height;
	} else if((crop_percentage & (1 << C_RIGHT))){
		VPRINT(verbose, "\nCropping Right-Half of Input Image\n");
		thisCrop->src_rect.x = thisFBTEST->fileParams.inResolution.width * .5;
		thisCrop->resolution.width = thisFBTEST->fileParams.inResolution.width * .5;
		thisCrop->resolution.height = thisFBTEST->fileParams.inResolution.height;
	} else if((crop_percentage & (1 << C_RAND))) {
		VPRINT(verbose, "\nRandom Cropping of Input Image\n");
		if ((thisFBTEST->coordinate.x == 0) && (thisFBTEST->coordinate.y == 0)) {}
		else {
			if ((thisFBTEST->coordinate.x <= (int) thisFBTEST->fileParams.inResolution.width) &&\
					(thisFBTEST->coordinate.y <= (int) thisFBTEST->fileParams.inResolution.height)){
				/* starting coordinate for cropping is non-zero(x,y) */
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
		sleep(2);
	}
	return result;
}

int cropTest(void) {
	int dst_w, dst_h,i;
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST;
	struct fb_var_screeninfo *vinfo;
	struct mdp_overlay overlay;

	thisFBTEST = &FBTEST;
	i = P_25;
	vinfo = &(FB->fb_vinfo);
	if (checkVideoFormatAlignment()) {
		return result;
	}
	VPRINT(verbose, "\nPerforming Crop Test...\n");
	switch(thisFBTEST->testMode)
	{
		case USER:
			result = cropCalculation();
			if (!result)
				result = doCrop();
			break;
		case AUTO:
			for (i; i<= C_RAND;i++) {
				thisFBTEST->crop = 0;
				thisFBTEST->crop = (1 << i);
				if (thisFBTEST->crop & (1 << C_RAND)) {
					srand(1);
					thisFBTEST->coordinate.x = rand_range(0, thisFBTEST->fileParams.inResolution.width);
					thisFBTEST->coordinate.y = rand_range(0, thisFBTEST->fileParams.inResolution.height);
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
	VPRINT(verbose, "Crop test over\n");
	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nCrop Test: PASS\n");
	} else {
		VPRINT(verbose, "\nCrop Test: FAIL\n");
	}
	return result;
}

int videoPlayTest(void) {
	struct mdp_blit_req* blitReq;
	FILE *imgRAW;
	struct timeval t_start, t_end;
	int result= TEST_RESULT_PASS, frame,filesize, framesize, err, uwait, i;
	float scaleX, scaleY, fps, fps_avg;
	int maxScale;
	unsigned char *vid_buf_front, *vid_buf_back;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	if (checkVideoFormatAlignment())
		return result;

	struct Video video;
	struct stat buf;
	struct mdp_overlay overlay;

	/* calculating size of file */
	if (!stat(thisFBTEST->fileParams.filenamePath,&buf)) {
		filesize = buf.st_size;
	} else {
		FBTEST_MSG_DEBUG("Error in filesize calculation\n");
		return -INCORRECT_FILE_SIZE;
	}

	VPRINT(verbose, "\nPerforming Video Test...\n");
	maxScale = getMaxUpScale(mdp_version);
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
	video.frames = filesize/framesize;

	/* opening video file */
	FBTEST_MSG_DEBUG("\nOpening video (%s)\n",thisFBTEST->fileParams.filenamePath );
	if ((imgRAW = fopen(thisFBTEST->fileParams.filenamePath,"rb")) == NULL) {
		FBTEST_MSG_DEBUG("[Video] Cannot open raw video file!\n");
		return -FOPEN_FAILED;
	}

	scaleX = (float)FB->fb_vinfo.xres / video.width;
	scaleY = (float)FB->fb_vinfo.yres / video.height;

	if (scaleX < (1/maxScale) || scaleX > maxScale) {
		VPRINT(verbose, "SKIP: Cannot support QVGA video \
			playback (scaleX = %.2f, scaleY = %.2f)\n", scaleX, scaleY);
		return TEST_RESULT_SKIP;
	}

	if (mdp3AllocMEM((framesize * 2), DEST_BUFFER_SIZE))
		return TEST_RESULT_FAIL;

	union {
		char dummy[sizeof(struct mdp_blit_req_list) + sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} imgFrame;
	struct mdp_blit_req_list *list = &(imgFrame.list);
	imgFrame.list.count = 1;
	blitReq = &imgFrame.list.req[0];
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	blitReq->src.width  = video.width;
	blitReq->src.height = video.height;
	blitReq->src.format = video.format;
	blitReq->src.memory_id = MEM->mem_fd_src;
	blitReq->dst.width  = FB->fb_vinfo.xres;
	blitReq->dst.height = FB->fb_vinfo.yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.memory_id = MEM->mem_fd;
	blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
				getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
	blitReq->alpha  = 0xFF;
	blitReq->dst_rect.w = FB->fb_vinfo.xres;
	blitReq->dst_rect.h = FB->fb_vinfo.yres;
	blitReq->src_rect.w = video.width;
	blitReq->src_rect.h = video.height;


	setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, blitReq->dst_rect.w, blitReq->dst_rect.h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	/* opening video file */
	FBTEST_MSG_DEBUG("\nOpening video (%s)\n",thisFBTEST->fileParams.filenamePath );
	if ((imgRAW = fopen(thisFBTEST->fileParams.filenamePath,"rb")) == NULL) {
		FBTEST_MSG_DEBUG("[Video] Cannot open raw video file!\n");
		return -FOPEN_FAILED;
	}
	result = TEST_RESULT_PASS;
	fps_avg = 0.0;
	uwait = 0;
	vid_buf_front = MEM->mem_buf_src;
	vid_buf_back = MEM->mem_buf_src + framesize;
	VPRINT(verbose, "Start Playback for (Frame %d)....\n",video.frames);
	for(frame = 0; frame < video.frames; frame++) {
		i=frame%2;
		thisFBTEST->frameNum = frame;
		gettimeofday(&t_start,NULL);
		if (fseek(imgRAW, framesize * frame, SEEK_SET) != 0) /* Fix: rewind to start of file */
			break;
		err = fread (vid_buf_back, sizeof(unsigned char), framesize, imgRAW);
			VPRINT(verbose,"frame size : %d, err :%d", framesize, err);
		swapPointers(&vid_buf_front, &vid_buf_back);
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
		blitReq->src.offset = vid_buf_front - MEM->mem_buf_src;
		if (err != framesize) {
			VPRINT(verbose, "\nERROR! RAW image read failed at frame %d (read %d of %d \
				bytes)\n", frame, err, framesize);
			result = TEST_RESULT_FAIL;
			break;
		}
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}

		doDisplayUpdate();
		if (video.frames == 1)
			usleep(160000);
		gettimeofday(&t_end,NULL);
		fps = (float)(1000.0)/(float)(t_end.tv_sec*1000+t_end.tv_usec/1000-t_start.tv_sec*1000-t_start.tv_usec/1000);

		if (video.fps != 0.0) {
			uwait = (int)( 1000000.0 / video.fps - 1000000.0 / fps );
			uwait = (int)( 1000000.0 / video.fps - 1000000.0 / fps );
			if (uwait > 0.0)
				usleep(uwait);
		}

		gettimeofday(&t_end,NULL);
		fps = (float)(1000.0)/(float)(t_end.tv_sec*1000+t_end.tv_usec/1000-t_start.tv_sec*1000-t_start.tv_usec/1000);

		fps_avg += fps;
	}

	if (result == TEST_RESULT_PASS) {
		fps_avg /= video.frames;
		VPRINT(verbose, "FPS = %.2f\n", fps_avg);
	}
	VPRINT(verbose, "\nClose Video");
	unsetOverlay(FB, &overlay.id);
	fclose(imgRAW);

	return result;
}

int CSCTest(void) {
	int size;
	struct fbtest_params *thisFBTEST = &FBTEST;
	VPRINT(verbose, "testname ==%s\n",thisFBTEST->testname);
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
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,
			thisFBTEST->fileParams.inputFormat);
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

	VPRINT(verbose,"\n\n\tORIGINAL VALUES:");

	for (i = 0; i < 9; i++)
		VPRINT(verbose, "\n csc_mv[%d] = 0x%x", i,
				mdp_pp_cfg.data.csc_cfg_data.csc_data.csc_mv[i]);
	VPRINT(verbose, "\n");
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

int rotateTest(void) {
	int iterations, i, result = TEST_RESULT_PASS, remainder;
	unsigned int w, h;
	int testresult, format, overlay_id, dst_w, dst_h;
	int rotations = 0;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	struct mdp_overlay overlay;
	unsigned int cmdRotFlip = 0, cmdRotDeg = 0;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;

	thisFBTEST = &FBTEST;
	cmdRotFlip = thisFBTEST->rotateFlip;
	cmdRotDeg = thisFBTEST->rotateDegree;
	memset(&overlay, 0, sizeof(struct mdp_overlay));
	testresult = TEST_RESULT_PASS;
	if (checkVideoFormatAlignment()) {
		return result;
	}
	vinfo = &(FB->fb_vinfo);
	loadImage();

	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	setBlitParam(blitReq, thisFBTEST->fileParams);

	/* Rotation Test */
	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 1;

	result = TEST_RESULT_PASS;
	w = blitReq->dst_rect.w;
	h = blitReq->dst_rect.h;
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	switch (thisFBTEST->rotateDegree) {
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
	switch (thisFBTEST->rotateFlip) {
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

	if (rotations != 0) {
		result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
				thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
				FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, blitReq->flags,
				MSMFB_NEW_REQUEST, 0);
		thisFBTEST->rotateFlip = 0;
		thisFBTEST->rotateDegree = 0;
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		thisFBTEST->rotateFlip = cmdRotFlip;
		thisFBTEST->rotateDegree = cmdRotDeg;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		blitReq->flags = rotations;
		if ((thisFBTEST->rotateDegree == ROT_90) ||
			(thisFBTEST->rotateDegree == ROT_270)) {
			blitReq->dst_rect.w = MIN(h, FB->fb_vinfo.xres);
			blitReq->dst_rect.h = MIN(w, FB->fb_vinfo.yres);
		}
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
	} else {
		VPRINT(verbose, "\nNo Rotate and Flip options Specified\n");
		blitReq->flags = 0;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
			return -BLIT_FAILED;
		}
		result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
				thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
				FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, blitReq->flags,
				MSMFB_NEW_REQUEST, 0);
		thisFBTEST->rotateFlip = FLIP_NOP;
		thisFBTEST->rotateDegree = ROT_0;
		result = doDisplayUpdate();

		blitReq->flags |= MDP_ROT_90;
		thisFBTEST->rotateDegree = ROT_90;
		blitReq->dst_rect.w = MIN(h, FB->fb_vinfo.xres);
		blitReq->dst_rect.h = MIN(w, FB->fb_vinfo.yres);

		VPRINT(verbose, " Dest Rect W=%d H=%d Dst W=%d H=%d \n",
			blitReq->dst_rect.w, blitReq->dst_rect.h,
			blitReq->dst.width, blitReq->dst.height);
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
				return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		swap = 0;
		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		VPRINT(verbose, "\n Rotation 90 : %s \n",
			(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");

		blitReq->flags = 0;
		thisFBTEST->rotateDegree = ROT_180;
		blitReq->flags |= MDP_ROT_180;
		blitReq->dst_rect.w = w;
		blitReq->dst_rect.h = h;

		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}

		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		VPRINT(verbose, "\n Rotation 180 : %s \n",
			(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");

		thisFBTEST->rotateDegree = ROT_270;
		blitReq->flags |= MDP_ROT_270;
		blitReq->dst_rect.w = MIN(h, FB->fb_vinfo.xres);
		blitReq->dst_rect.h = MIN(w, FB->fb_vinfo.yres);

		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		VPRINT(verbose, "\n Rotation 270 : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");

		thisFBTEST->rotateDegree = ROT_0;
		thisFBTEST->rotateFlip = FLIP_LR;
		blitReq->flags = MDP_FLIP_LR;
		blitReq->dst_rect.w = w;
		blitReq->dst_rect.h = h;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		VPRINT(verbose, "\n Flip LR : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");

		thisFBTEST->rotateFlip = FLIP_UD;
		blitReq->flags = MDP_FLIP_UD;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		VPRINT(verbose, "\n Flip UD : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");

		thisFBTEST->rotateFlip = FLIP_LRUD;
		blitReq->flags = MDP_FLIP_UD | MDP_FLIP_LR;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % 2));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		VPRINT(verbose, "\n FLIP LRUD : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");
	}
	unsetOverlay(FB, &overlay.id);
	return testresult;
}

int traverseImage(unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY) {
	int  fb_fd,i = 0, factor = 0,loopCnt = 0,xOrdinate = 0, yOrdinate = 0,result = TEST_RESULT_FAIL;
	fb_fd=FB->fb_fd;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	vinfo = &(FB->fb_vinfo);
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	thisFBTEST = &FBTEST;
	setBlitParam(blitReq, thisFBTEST->fileParams);

	result = TEST_RESULT_PASS;

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

	blitReq->dst_rect.x = startX;
	blitReq->dst_rect.y = startY;

	loopCnt = factor -1;


	for (i = 0; i <= loopCnt; i++) {
		if (startX < endX)
			blitReq->dst_rect.x += (((endX-startX)/factor));
		else
			blitReq->dst_rect.x -= (((startX-endX)/factor));

		if (startY < endY)
			blitReq->dst_rect.y += (((endY-startY)/factor));
		else
			blitReq->dst_rect.y -= (((startY-endY)/factor));

		thisFBTEST->imgOffset.startX = blitReq->dst_rect.x;
		thisFBTEST->imgOffset.startY = blitReq->dst_rect.y;

		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
	}
	return TEST_RESULT_PASS;
}

int doImageMove(unsigned int startX, unsigned int startY, unsigned int endX,
		unsigned int endY, unsigned int traversal) {
	int size, result = TEST_RESULT_FAIL, factor = 0;
	int loopCnt = 0, xOrdinate = 0, yOrdinate = 0;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	unsigned int i = 0;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;

	thisFBTEST = &FBTEST;
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,
			thisFBTEST->fileParams.inputFormat);
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	/* Get file params */
	unsigned int iWidth = thisFBTEST->fileParams.inResolution.width;
	unsigned int iHeight = thisFBTEST->fileParams.inResolution.height;
	unsigned int oWidth = thisFBTEST->fileParams.inResolution.width;
	unsigned int oHeight = thisFBTEST->fileParams.inResolution.height;
	unsigned int iFormat = thisFBTEST->fileParams.inputFormat;

	vinfo = &(FB->fb_vinfo);
	blitReq->src.memory_id = MEM->mem_fd_src;
	blitReq->dst.width  = vinfo->xres;
	blitReq->dst.height = vinfo->yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
				getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
	blitReq->dst.memory_id = MEM->mem_fd;
	blitReq->src.width  = iWidth;
	blitReq->src.height = iHeight;
	blitReq->src.format = iFormat;
	blitReq->transp_mask = MDP_TRANSP_NOP;
	blitReq->alpha = 0xFF;
	blitReq->src_rect.w = iWidth;
	blitReq->src_rect.h = iHeight;
	blitReq->dst_rect.x = startX;
	blitReq->dst_rect.y = startY;
	blitReq->dst_rect.w = oWidth;
	blitReq->dst_rect.h = oHeight;
	blitReq->dst_rect.w = iWidth;
	blitReq->dst_rect.h = iHeight;

	switch (traversal) {
		case USER_DEFINED:
			VPRINT(verbose, "User Defined Move: [%dx%d] --> [%dx%d]\n", startX,
				startY, endX, endY);
			traverseImage(startX, startY, endX, endY);
			break;
		case HORIZONTAL:
			startX=0;
			startY=0;
			endX = vinfo->xres - iWidth;
			endY = 0;
			if (startX < endX)
				xOrdinate = endX -startX;
			else
				xOrdinate = startX -endX;

			if (startY < endY)
				yOrdinate = endY -startY;
			else
				yOrdinate = startY -endY;

			factor = gcd(xOrdinate, yOrdinate);

			blitReq->dst_rect.x = startX;
			blitReq->dst_rect.y = startY;

			loopCnt = factor -1;

			for (i = 0; i <endX; i++) {
				if (startX < endX)
					blitReq->dst_rect.x += (((endX-startX)/factor));
				else
					blitReq->dst_rect.x -= (((startX-endX)/factor));

				if (startY < endY)
					blitReq->dst_rect.y += (((endY-startY)/factor));
				else
					blitReq->dst_rect.y -= (((startY-endY)/factor));

				/* drawBG(); */
			if ((result = doBlit(list)) < 0) {
				VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
				return -BLIT_FAILED;
			}
			result = doDisplayUpdate();
			}
			break;
		case VERTICAL:
			startX=0;
			startY=0;
			endX=0;
			endY = vinfo->yres - iHeight;
			if (startX < endX)
				xOrdinate = endX -startX;
			else
				xOrdinate = startX -endX;

			if (startY < endY)
				yOrdinate = endY -startY;
			else
				yOrdinate = startY -endY;

			factor = gcd(xOrdinate, yOrdinate);

			blitReq->dst_rect.x = startX;
			blitReq->dst_rect.y = startY;

			loopCnt = factor -1;

			for (i = 0; i <endY; i++) {
				if (startX < endX)
					blitReq->dst_rect.x += (((endX-startX)/factor));
				else
					blitReq->dst_rect.x -= (((startX-endX)/factor));

				if (startY < endY)
					blitReq->dst_rect.y += (((endY-startY)/factor));
				else
					blitReq->dst_rect.y -= (((startY-endY)/factor));
				/* drawBG(); */
				if ((result = doBlit(list)) < 0) {
					VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
						__LINE__, result);
					return -BLIT_FAILED;
				}
				result = doDisplayUpdate();
			}
			break;
		case BOUNDARY:
			startX=0;
			startY=0;
			endX = vinfo->xres - iWidth;
			endY = 0;
			if (startX < endX)
				xOrdinate = endX -startX;
			else
				xOrdinate = startX -endX;
			if (startY < endY)
				yOrdinate = endY -startY;
			else
				yOrdinate = startY -endY;

			factor = gcd(xOrdinate, yOrdinate);

			blitReq->dst_rect.x = startX;
			blitReq->dst_rect.y = startY;

			loopCnt = factor -1;

			for (i = 0; i < endX; i++) {
				if (startX < endX)
					blitReq->dst_rect.x += (((endX-startX)/factor));
				else
					blitReq->dst_rect.x -= (((startX-endX)/factor));

				if (startY < endY)
					blitReq->dst_rect.y += (((endY-startY)/factor));
				else
					blitReq->dst_rect.y -= (((startY-endY)/factor));

				if ((result = doBlit(list)) < 0) {
					VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
						__LINE__, result);
					return -BLIT_FAILED;
				}
				result = doDisplayUpdate();
			}

			startX = vinfo->xres - iWidth;
			startY=0;
			endX = vinfo->xres - iWidth;
			endY = vinfo->yres - iHeight;
			if (startX < endX)
				xOrdinate = endX -startX;
			else
				xOrdinate = startX -endX;

			if (startY < endY)
				yOrdinate = endY -startY;
			else
				yOrdinate = startY -endY;

			factor = gcd(xOrdinate, yOrdinate);

			blitReq->dst_rect.x = startX;
			blitReq->dst_rect.y = startY;

			loopCnt = factor -1;

			for (i = 0; i < endY; i++) {

				if (startX < endX)
					blitReq->dst_rect.x += (((endX-startX)/factor));
				else
					blitReq->dst_rect.x -= (((startX-endX)/factor));

				if (startY < endY)
					blitReq->dst_rect.y += (((endY-startY)/factor));
				else
					blitReq->dst_rect.y -= (((startY-endY)/factor));

				if ((result = doBlit(list)) < 0) {
					VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
						__LINE__, result);
					return -BLIT_FAILED;
				}
				result = doDisplayUpdate();
			}
			startX = vinfo->xres - iWidth;
			startY = vinfo->yres - iHeight;
			endX=0;
			endY = vinfo->yres - iHeight;
			if (startX < endX)
				xOrdinate = endX -startX;
			else
				xOrdinate = startX -endX;

			if (startY < endY)
				yOrdinate = endY -startY;
			else
				yOrdinate = startY -endY;

			factor = gcd(xOrdinate, yOrdinate);

			blitReq->dst_rect.x = startX;
			blitReq->dst_rect.y = startY;

			loopCnt = factor -1;

			for (i = 0; i < startX; i++) {
				if (startX < endX)
					blitReq->dst_rect.x += (((endX-startX)/factor));
				else
					blitReq->dst_rect.x -= (((startX-endX)/factor));

				if (startY < endY)
					blitReq->dst_rect.y += (((endY-startY)/factor));
				else
					blitReq->dst_rect.y -= (((startY-endY)/factor));

				if ((result = doBlit(list)) < 0) {
					VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
						__LINE__, result);
					return -BLIT_FAILED;
				}
				result = doDisplayUpdate();
			}
			startX=0;
			startY = vinfo->yres - iHeight;
			endX=0;
			endY=0;
			if (startX < endX)
				xOrdinate = endX -startX;
			else
				xOrdinate = startX -endX;

			if (startY < endY)
				yOrdinate = endY -startY;
			else
				yOrdinate = startY -endY;

			factor = gcd(xOrdinate, yOrdinate);
			blitReq->dst_rect.x = startX;
			blitReq->dst_rect.y = startY;
			loopCnt = factor -1;
			for (i = 0; i < startY; i++) {
				if (startX < endX)
					blitReq->dst_rect.x += (((endX-startX)/factor));
				else
					blitReq->dst_rect.x -= (((startX-endX)/factor));

				if (startY < endY)
					blitReq->dst_rect.y += (((endY-startY)/factor));
				else
					blitReq->dst_rect.y -= (((startY-endY)/factor));

				if ((result = doBlit(list)) < 0) {
					VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
						__LINE__, result);
					return -BLIT_FAILED;
				}
				result = doDisplayUpdate();
			}
			break;
		case DIAGONALS:
			endX = vinfo->xres - iWidth;
			endY = vinfo->yres - iHeight;
			traverseImage(0, 0, endX, endY);
			startX = vinfo->xres - iWidth;
			endY = vinfo->yres - iHeight;
			traverseImage(startX,0, 0, endY);
			break;
		case RANDOM:
			VPRINT(verbose, "Random\n");
			/* Take random values between 1/8 and oWidth */
			startX = rand_range(0, vinfo->xres - iWidth);
			startY = rand_range(0, vinfo->yres - iHeight);
			endX = rand_range(0,vinfo->xres - iWidth);
			endY = rand_range(0,vinfo->yres - iHeight);
			VPRINT(verbose, "Random Move: [x1 = %d y1 = %d  x2 = %d y2 = %d]\n",
				startX, startY, endX, endY);
			traverseImage(startX, startY, endX, endY);
			break;
	}
	return TEST_RESULT_PASS;
}

int move(void) {
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST=&FBTEST;
	struct fb_var_screeninfo *vinfo;

	unsigned int startX = thisFBTEST->imgOffset.startX;
	unsigned int startY = thisFBTEST->imgOffset.startY;
	unsigned int endX = thisFBTEST->imgOffset.endX;
	unsigned int endY = thisFBTEST->imgOffset.endY;

	vinfo = &(FB->fb_vinfo);
	loadImage();
	if (thisFBTEST->imgOffset.startX < 0 ||
			thisFBTEST->imgOffset.startY < 0 ||
			thisFBTEST->imgOffset.endX < 0 ||
			thisFBTEST->imgOffset.endY < 0) {

		FBTEST_MSG_DEBUG("\n[Move] Invalid Offsets! (Line %d)\n",	__LINE__);
		return -INVALID_OFFSET;
	}
	/* Check if offsets are valid as per MDP3 support */
	if ((startX > (vinfo->xres- thisFBTEST->fileParams.inResolution.width)) ||
			(startY > (vinfo->yres- thisFBTEST->fileParams.inResolution.height))||
			(endX > (vinfo->xres- thisFBTEST->fileParams.inResolution.width)) ||
			(endY > (vinfo->yres- thisFBTEST->fileParams.inResolution.height)))
	{
		FBTEST_MSG_DEBUG("\n[Move] Offsets(%d, %d) or (%d, %d) OutofBound! \
							(Line %d)\n", startX + thisFBTEST->fileParams.inResolution.width,
							 startY + thisFBTEST->fileParams.inResolution.height,\
							 endX + thisFBTEST->fileParams.inResolution.width,
							 endY + thisFBTEST->fileParams.inResolution.height, __LINE__);
		showError(-OFFSET_OUT_OF_BOUND, 0);
	}

	VPRINT(verbose, "\nPerforming Image Move Test...\n");
	switch(thisFBTEST->testMode) {
		case USER:
			if ((endX) || (endY) ||(startX) || (startY))
				result = doImageMove(startX, startY, endX, endY, USER_DEFINED);
			else
				result = doImageMove(startX, startY, endX, endY, DIAGONALS);
			break;
		case AUTO:
			if ((endX > 0) || (endY > 0))
				thisFBTEST->moveType = USER_DEFINED;
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
					VPRINT(verbose, "IMAGE TRAVERSAL IN USER_DEFINED\n");
					break;
				default:
					VPRINT(verbose, "IMAGE TRAVERSAL IS NOT DEFINED, USE RANDOM\n");
					thisFBTEST->moveType = RANDOM;
					break;
			}
			result = doImageMove(startX, startY, endX, endY, thisFBTEST->moveType);
			break;
		default:
			VPRINT(verbose, "Error: Not a valid mode of operation\n");
			break;
	}

	VPRINT(verbose, "Image Move test over\n");
	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "Image Move Test: Pass \n");
	} else {
		VPRINT(verbose, "Image move Test: Fail \n");
	}
	return result;
}

int moveTest(void) {
	int result= TEST_RESULT_PASS, overlay_id, dst_w, dst_h;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	union {
	char dummy[sizeof(struct mdp_blit_req_list)
		+ sizeof(struct mdp_blit_req)*1];
	struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	struct mdp_overlay overlay;


	thisFBTEST = &FBTEST;
	if (checkVideoFormatAlignment()) {
		return result;
	}
	vinfo = &(FB->fb_vinfo);
	loadImage();
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	setBlitParam(blitReq, thisFBTEST->fileParams);
	result = TEST_RESULT_PASS;

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	move();
	unsetOverlay(FB, &overlay.id);
	return result;
}

#ifdef ENABLE_POSTPROC
int postprocTest(void) {
	int result = TEST_RESULT_FAIL;
	struct fbtest_params *thisFBTEST;

	thisFBTEST = &FBTEST;

	if (thisFBTEST->pp_data.postproc_use_file) {
		VPRINT(verbose, "\n postproc_file: %s ",
				thisFBTEST->pp_data.postproc_file);
		result = parsePPFrameworkFile(thisFBTEST->pp_data.postproc_file);
		thisFBTEST->pp_data.postproc_use_file = 0;
	} else {
		VPRINT(verbose, "\n Running default postproc cfg file : %s ",
				DEFAULT_POSTPROC_FILE_PATH);
		result = parsePPFrameworkFile(DEFAULT_POSTPROC_FILE_PATH);
	}

	VPRINT(verbose, "\nPost Processing Test: %s\n",result?"FAIL":"PASS");
	return result;
}
#endif

int formatTest(void) {
	int fb_fd,result= TEST_RESULT_PASS, size = 0, dst_w,dst_h;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	struct timeval time_start, time_stop;
	struct mdp_overlay overlay;

	thisFBTEST = &FBTEST;
	if (checkVideoFormatAlignment()) {
		return result;
	}
	vinfo = &(FB->fb_vinfo);
        if (!(strncmp(thisFBTEST->path, "dummy",
                strlen("dummy") + 1))) {
                size = FACE_WIDTH * FACE_HEIGHT * getFormatBpp(FACE_FORMAT);
		thisFBTEST->fileParams.inResolution.width = FACE_WIDTH;
		thisFBTEST->fileParams.inResolution.height = FACE_HEIGHT;
		thisFBTEST->fileParams.inputFormat = FACE_FORMAT;
		if (mdp3AllocMEM(size, DEST_BUFFER_SIZE)) {
			FBTEST_MSG_DEBUG("[FORMAT] mdp3AllocMEM failed! (Line %d)\n",__LINE__);
			showError(-PMEMALLOC_FAILED, 1);
		} else {
			result = TEST_RESULT_FAIL;
		}
		memset(MEM->mem_buf_src, 0x00, size);
		memcpy(MEM->mem_buf_src, yellow_face_data, size);
        } else {
		size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
				thisFBTEST->fileParams.inResolution.height,
				thisFBTEST->fileParams.inputFormat);
		loadImage();
	}
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	setBlitParam(blitReq, thisFBTEST->fileParams);
	if (thisFBTEST->fileParams.inResolution.width > vinfo->xres)
		blitReq->dst_rect.w = vinfo->xres;
	else
		blitReq->dst_rect.w = thisFBTEST->fileParams.inResolution.width;
	if (thisFBTEST->fileParams.inResolution.height > vinfo->yres)
		blitReq->dst_rect.h = vinfo->yres;
	else
		blitReq->dst_rect.h = thisFBTEST->fileParams.inResolution.height;
	result = TEST_RESULT_PASS;
	gettimeofday(&time_start,NULL);
	if ((result = doBlit(list)) < 0) {
		VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
		return -BLIT_FAILED;
	}
	gettimeofday(&time_stop,NULL);
	float f=(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
			time_start.tv_sec*1000-time_start.tv_usec/1000);
	VPRINT(verbose, " ### f=%f\n\n",f);

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	result = doDisplayUpdate();
	unsetOverlay(FB, &overlay.id);
	return result;
}

int doScale(unsigned int iWidth, unsigned int iHeight, unsigned int oWidth,
	unsigned int oHeight, unsigned int iFormat) {
	int iterations, w, h, result, remainder, d_w_1, s_w_1, size;
	unsigned int scale = 4;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;

	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	thisFBTEST = &FBTEST;
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,
			thisFBTEST->fileParams.inputFormat);
	vinfo = &(FB->fb_vinfo);
	if ((oWidth > vinfo->xres) || (oHeight > vinfo->yres))
		showError(-RES_OUT_OF_BOUND, 1);

	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	blitReq->src.width  = iWidth;
	blitReq->src.height = iHeight;
	blitReq->src.format = iFormat;
	blitReq->src.memory_id = MEM->mem_fd_src;
	blitReq->dst.width  = vinfo->xres;
	blitReq->dst.height = vinfo->yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
				getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
	blitReq->dst.memory_id = MEM->mem_fd;
	blitReq->transp_mask = MDP_TRANSP_NOP;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = oWidth;
	blitReq->dst_rect.h = oHeight;
	blitReq->src_rect.w = iWidth;
	blitReq->src_rect.h = iHeight;

	scale = getMaxUpScale(mdp_version);
	/* drawBG(); */
	result = TEST_RESULT_PASS;
	w = blitReq->dst_rect.w;
	h = blitReq->dst_rect.h;

	blitReq->dst_rect.w = iWidth;
	blitReq->dst_rect.h = iHeight;
	if (iHeight < oHeight)
		blitReq->dst_rect.h = iHeight - 1;
	if (iHeight > oHeight)
		blitReq->dst_rect.h = iHeight + 1;
	if (iWidth < oWidth)
		blitReq->dst_rect.w = iWidth - 1;
	if (iWidth > oWidth)
		blitReq->dst_rect.w = iWidth + 1;
	if (blitReq->dst_rect.w > vinfo->xres)
		blitReq->dst_rect.w = vinfo->xres;
	if (blitReq->dst_rect.h > vinfo->yres)
		blitReq->dst_rect.h = vinfo->yres;
	while ((blitReq->dst_rect.w != oWidth) ||
			(blitReq->dst_rect.h != oHeight )) {
		if ((iFormat == MDP_YCRYCB_H2V1) && (getFormatBpp(FB->fb_fmt) == 4)) {
			remainder = blitReq->dst_rect.w % 16;
			if (remainder == 14 || remainder == 6) {
				d_w_1 = (blitReq->dst_rect.w / 2) - 1;
				s_w_1 = (blitReq->src_rect.w / 2) - 1;
				if (d_w_1 > (4 * s_w_1)) {
					if (blitReq->dst_rect.w < oWidth)
						blitReq->dst_rect.w++;
					if (blitReq->dst_rect.w > oWidth)
						blitReq->dst_rect.w--;
					if (blitReq->dst_rect.h < oHeight)
						blitReq->dst_rect.h++;
					if (blitReq->dst_rect.h > oHeight)
						blitReq->dst_rect.h--;
					continue;
				}
			}
		}
		/* drawBG(); */
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
		VPRINT(verbose, "swap in scale = %d\n", swap);
		thisFBTEST->outResolution.width = blitReq->dst_rect.w;
		thisFBTEST->outResolution.height = blitReq->dst_rect.h;
		if ((result = doBlit(list)) < 0) {
			VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
			return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		if (blitReq->dst_rect.w < oWidth)
			blitReq->dst_rect.w++;
		if (blitReq->dst_rect.w > oWidth)
			blitReq->dst_rect.w--;
		if (blitReq->dst_rect.h < oHeight)
			blitReq->dst_rect.h++;
		if (blitReq->dst_rect.h > oHeight)
			blitReq->dst_rect.h--;
	}
	VPRINT(verbose, "\n Scaling Test : %s \n",
			(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");
	return result;
}

int scaleTest(void) {
	int iterations, w, h, result = TEST_RESULT_PASS, d_w_1, s_w_1;
	int i, size, overlay_id, dst_w, dst_h;
	unsigned int scale = 4;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST = &FBTEST;
	struct mdp_overlay overlay;
	unsigned int iWidth = thisFBTEST->fileParams.inResolution.width;
	unsigned int iHeight = thisFBTEST->fileParams.inResolution.height;
	unsigned int oWidth = thisFBTEST->outResolution.width;
	unsigned int oHeight = thisFBTEST->outResolution.height;
	unsigned int iFormat = thisFBTEST->fileParams.inputFormat;
	size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
			thisFBTEST->fileParams.inResolution.height,
			thisFBTEST->fileParams.inputFormat);
	if (checkVideoFormatAlignment()) {
		return result;
	}
	vinfo = &(FB->fb_vinfo);
	loadImage();

	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 1;
	result = TEST_RESULT_PASS;

	scale = getMaxUpScale(mdp_version);

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	switch(thisFBTEST->testMode)
	{
		case USER:
			result = doScale( iWidth, iHeight, oWidth, oHeight, iFormat);
			break;
		case AUTO:
			for(i = UPSCALE; i <= ARBITRARY; i++) {
				if (i == UPSCALE)  {

					oWidth  = (((iWidth*scale) < vinfo->xres) ? (iWidth*scale) :vinfo->xres);
					oHeight  = (((iHeight*scale) < vinfo->yres) ? (iHeight*scale) :vinfo->yres);

					VPRINT(verbose, "UPSCALING TO: %dx%d\n",oWidth, oHeight);
					result = doScale( iWidth, iHeight, oWidth, oHeight, iFormat);
				}
				if (i == DOWNSCALE)  {

					oWidth  =  (iWidth/scale);
					oHeight  = (iHeight/scale);

					VPRINT(verbose, "DOWNSCALING TO: %dx%d\n",oWidth, oHeight);
					result = doScale( iWidth, iHeight, oWidth, oHeight, iFormat);
				}

				if (i == ARBITRARY) {
					srand(1);

					oWidth  = (((iWidth*scale) < vinfo->xres) ? (iWidth*scale) :vinfo->xres);
					oHeight  = (((iHeight*scale) < vinfo->yres) ? (iHeight*scale) :vinfo->yres);

					/*Take random values between 1/scale and oWidth */
					oWidth = rand_range(iWidth/scale, oWidth);
					oHeight = rand_range(iHeight/scale, oHeight);

					VPRINT(verbose, "ARBITRARY SCALING TO: %dx%d\n",oWidth, oHeight);
					result = doScale( iWidth, iHeight, oWidth, oHeight, iFormat);
				}
			}
			break;
		default:
			VPRINT(verbose, "Error: Not a valid mode of operation\n");
			break;
	}
	unsetOverlay(FB, &overlay.id);
	return result;
}

int blendTest(void)
{
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	int overlay_id, tmp_result, max_img;
	struct mdp_overlay overlay;

	unsigned char *ImgBuff[2];
	struct fb_var_screeninfo *vinfo;
	struct inputFileParams tmpFileParams;
	unsigned int i = 0,inFileCnt = 0;
	int ImgFrameSize[2], size[2], max_size=0;
	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	FILE *fptr = NULL;
	size_t bytesRead = 0;

	if (checkVideoFormatAlignment()) {
		return result;
	}
	vinfo = &(FB->fb_vinfo);
	thisFBTEST->blend.ImgAlphaVal[0] = (thisFBTEST->blend.ImgAlphaVal[0] >= 0 &&
		thisFBTEST->blend.ImgAlphaVal[0] <= 0xFF) ?
		thisFBTEST->blend.ImgAlphaVal[0] : 0x1f;
	thisFBTEST->blend.ImgAlphaVal[1] = (thisFBTEST->blend.ImgAlphaVal[1] >= 0 &&
		thisFBTEST->blend.ImgAlphaVal[1] <= 0xFF)?
		thisFBTEST->blend.ImgAlphaVal[1] : 0xa5;

	VPRINT(verbose, "\nPerforming Image Blend Test...\n");

	for(i=0; i < 2; i++) {
		VPRINT(verbose, "thisFBTEST->blend.ImagePath[i]=%s\n",thisFBTEST->blend.ImagePath[i]);
		if (strcmp(thisFBTEST->blend.ImagePath[i], "")) {
			result = parseFileName(thisFBTEST->blend.ImagePath[i], &ImgFileParams[inFileCnt++]);
			if (result < 0)
				showError(result, 1);
		}
	}

	for(i=0; i < inFileCnt; i++) {
		if (strcmp(ImgFileParams[i].filenamePath, "")) {
			ImgFrameSize[i] = getFrameSize(ImgFileParams[i].inResolution.width,\
				ImgFileParams[i].inResolution.height,ImgFileParams[i].inputFormat);
			size[i]=ImgFrameSize[i];
			if (max_size<size[i])
				max_size=size[i];
			/* allocate buffer for reading data from input image */
			ImgBuff[i] = (unsigned char *)malloc(ImgFrameSize[i]);
			if (ImgBuff[i] == NULL)
				showError(-MALLOC_FAILED, 1);
			/* open image file */
			fptr = fopen(ImgFileParams[i].filenamePath, "rb");
			if (fptr == NULL) {
				FBTEST_MSG_DEBUG("[Blend Test] Cannot open raw Image file!\n");
				return -FOPEN_FAILED;
			}
			/* read image in buffer */
			bytesRead  = fread(ImgBuff[i], 1, ImgFrameSize[i], fptr);
			if (bytesRead == 0) {
				FBTEST_MSG_DEBUG("[Blend Test] fread failed! (Line %d)\n",__LINE__);
				return -FREAD_FAILED;
			}
			/* close the file */
			fclose(fptr);
		}
	}

	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));

	if (mdp3AllocMEM(max_size, DEST_BUFFER_SIZE)) {
		FBTEST_MSG_DEBUG("[FORMAT] mdp3AllocMEM failed! (Line %d)\n",__LINE__);
		return -PMEMALLOC_FAILED;
		result = TEST_RESULT_PASS;
	}

	memset(MEM->mem_buf_src, 0x00, max_size);
	memset(MEM->mem_buf, 0x00, DEST_BUFFER_SIZE);
	tmp_result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, FB->fb_vinfo.xres, FB->fb_vinfo.yres, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);
	for (i = 0;i < inFileCnt;i++) {
		VPRINT(verbose, "\n\n\nimages =%s\n",ImgFileParams[i].filenamePath);
		VPRINT(verbose, "Doing a mem copy for size %d",size[i]);
		memcpy(MEM->mem_buf_src, ImgBuff[i], (int)size[i]);
		thisFBTEST->fileParams.inResolution.width = ImgFileParams[i].inResolution.width;
		thisFBTEST->fileParams.inResolution.height = ImgFileParams[i].inResolution.height;
		thisFBTEST->fileParams.inputFormat = ImgFileParams[i].inputFormat;

		blitReq->src.width  = ImgFileParams[i].inResolution.width;
		blitReq->src.height = ImgFileParams[i].inResolution.height;
		blitReq->src.format = ImgFileParams[i].inputFormat;
		blitReq->src.memory_id = MEM->mem_fd_src;
		blitReq->dst.width  = vinfo->xres;
		blitReq->dst.height = vinfo->yres;
		blitReq->dst.format = FB->fb_fmt;
		blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
					getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
		blitReq->dst.memory_id = MEM->mem_fd;
		blitReq->transp_mask = MDP_TRANSP_NOP;
		blitReq->alpha  = thisFBTEST->blend.ImgAlphaVal[i];
		VPRINT(verbose, "alpha=%x \n",blitReq->alpha);
		blitReq->dst_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->dst_rect.h = ImgFileParams[i].inResolution.height;
		blitReq->src_rect.w = ImgFileParams[i].inResolution.width;
		blitReq->src_rect.h = ImgFileParams[i].inResolution.height;

		result = TEST_RESULT_PASS;
		if ((result = doBlit(list)) < 0) {
				VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
					__LINE__, result);
				return -BLIT_FAILED;
		}
		result = doDisplayUpdate();
		swap = 0;
	}
	unsetOverlay(FB, &overlay.id);
	free(ImgBuff[0]);
	free(ImgBuff[1]);
	return result;
}

int multiopTest(void) {
	int iterations, w, h,i,remainder,testresult, format, size;
	int overlay_id, dst_w, dst_h;
	int rotations = 0;
	struct fb_var_screeninfo *vinfo;
	int result = TEST_RESULT_PASS,tempswap;
	struct fbtest_params *thisFBTEST;
	struct cropParams *thisCrop;
	thisFBTEST = &FBTEST;
	thisCrop = &CROPTEST;
	vinfo = &(FB->fb_vinfo);
	int mode = USER;
	unsigned int cmdRotDeg;
	testresult = TEST_RESULT_PASS;
	if (checkVideoFormatAlignment()) {
		return result;
	}

	union {
		char dummy[sizeof(struct mdp_blit_req_list)
			+ sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;

	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req *blitReq;
	struct mdp_overlay overlay;
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	if (!(strncmp(thisFBTEST->path, "dummy",
			strlen("dummy") + 1))) {
		size = FACE_WIDTH * FACE_HEIGHT * getFormatBpp(FACE_FORMAT);
		printf("\nsize =  %d\n", size);
		thisFBTEST->fileParams.inResolution.width = FACE_WIDTH;
		thisFBTEST->fileParams.inResolution.height = FACE_HEIGHT;
		thisFBTEST->fileParams.inputFormat = FACE_FORMAT;
		if (mdp3AllocMEM(size, DEST_BUFFER_SIZE)) {
			FBTEST_MSG_DEBUG("[FORMAT] mdp3AllocMEM failed! (Line %d)\n",__LINE__);
				showError(-PMEMALLOC_FAILED, 1);
		} else {
			result = TEST_RESULT_FAIL;
		}

		memset(MEM->mem_buf_src, 0x00, size);
		memset(MEM->mem_buf, 0x00, DEST_BUFFER_SIZE);
		memcpy(MEM->mem_buf_src, yellow_face_data, size);
		/* set the test as rotate-move-crop-scale */
		thisFBTEST->rotateDegree = ROT_270; /* 270 deg rotation */
		thisFBTEST->imgOffset.endX = thisFBTEST->fileParams.inResolution.height * 0.25; /* move 'x' coordinate */
		thisFBTEST->imgOffset.endY = thisFBTEST->fileParams.inResolution.width * 0.25; /* move 'y' coordinate */
		/* 2x scaling and swapped width-height */
		thisFBTEST->outResolution.width = thisFBTEST->fileParams.inResolution.height * 0.5;
		thisFBTEST->outResolution.height = thisFBTEST->fileParams.inResolution.width * 0.5;
		thisFBTEST->crop |= 1<< P_25;/* set 25 percentage cropping */
        } else {
		size = getFrameSize(thisFBTEST->fileParams.inResolution.width,\
				thisFBTEST->fileParams.inResolution.height,
				thisFBTEST->fileParams.inputFormat);
		loadImage();
	}
	setBlitParam(blitReq, thisFBTEST->fileParams);
	w=blitReq->dst_rect.w;
	h=blitReq->dst_rect.h;
	if (thisFBTEST->testMode == AUTO) {
		multiopflag = 1; /* for auto mode set the test as rotate-move-crop-scale */
		thisFBTEST->rotateDegree = ROT_270; /* 270 deg rotation */
		thisFBTEST->imgOffset.endX = thisFBTEST->fileParams.inResolution.height * 0.25; /* move 'x' coordinate */
		thisFBTEST->imgOffset.endY = thisFBTEST->fileParams.inResolution.width * 0.25; /* move 'y' coordinate */
		/* 2x scaling and swapped width-height */
		thisFBTEST->outResolution.width = thisFBTEST->fileParams.inResolution.height * 0.5;
		thisFBTEST->outResolution.height = thisFBTEST->fileParams.inResolution.width * 0.5;
		thisFBTEST->crop |= 1<< P_25;/* set 25 percentage cropping */
		mode = AUTO;
		/* setting mode user so that rotation,crop,move & scale work in user mode */
		thisFBTEST->testMode = USER;
	}

	if (multiopflag == 0) {
		VPRINT(verbose, "Performing Move-Crop-Scale test...\n");
		/* test is move-crop-scale */
		/* formatTest(); */
	}
	blitReq->dst_rect.w = thisFBTEST->fileParams.inResolution.width;
	blitReq->dst_rect.h = thisFBTEST->fileParams.inResolution.height;

	if (multiopflag) {
		VPRINT(verbose, "Performing Rotate-Move-Crop-Scale test...\n");
		cmdRotDeg = thisFBTEST->rotateDegree;
		/* call rotate test */
		blitReq->alpha  = 0xff;
		blitReq->dst_rect.w = thisFBTEST->fileParams.inResolution.width;
		blitReq->dst_rect.h = thisFBTEST->fileParams.inResolution.height;
		blitReq->src_rect.w = thisFBTEST->fileParams.inResolution.width;
		blitReq->src_rect.h = thisFBTEST->fileParams.inResolution.height;
		if (thisFBTEST->iteration)
			iterations = thisFBTEST->iteration;
		else
			iterations = 1;
		result = TEST_RESULT_PASS;
		switch (cmdRotDeg) {
			case ROT_0:
				rotations = 0;
				thisFBTEST->rotateDegree=ROT_0;
				break;
			case ROT_90:
				rotations |= MDP_ROT_90;
				blitReq->dst_rect.w = h;
				blitReq->dst_rect.h = w;
				thisFBTEST->rotateDegree=ROT_90;
				break;
			case ROT_180:
				rotations |= MDP_ROT_180;
				thisFBTEST->rotateDegree=ROT_180;
				break;
			case ROT_270:
				rotations |= MDP_ROT_270;
				blitReq->dst_rect.w = h;
				blitReq->dst_rect.h = w;
				thisFBTEST->rotateDegree=ROT_270;
				break;
			default:
				VPRINT(verbose, "[ROTATE] Rotation degree is not proper\n");
		}
		switch (thisFBTEST->rotateFlip) {
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
                blitReq->flags = rotations;
	}
	blitReq->src.width  = thisFBTEST->fileParams.inResolution.width;
	blitReq->src.height = thisFBTEST->fileParams.inResolution.height;
	blitReq->src.format = thisFBTEST->fileParams.inputFormat;
	if (!cropCalculation()) {
		blitReq->src_rect.x = thisCrop->src_rect.x;/* for crop */
		blitReq->src_rect.y = thisCrop->src_rect.y;/* for crop */
		blitReq->src_rect.w = thisCrop->resolution.width;/* for crop */
		blitReq->src_rect.h = thisCrop->resolution.height;/* for crop */
	}

	/* check move coordinate within panel width/height */
	if (((thisFBTEST->imgOffset.endX + blitReq->src.width) <= FB->fb_vinfo.xres)
			&& ((thisFBTEST->imgOffset.endY + blitReq->src.height) <= FB->fb_vinfo.yres)) {
		blitReq->dst_rect.x =  thisFBTEST->imgOffset.endX;/* for move */
		blitReq->dst_rect.y =  thisFBTEST->imgOffset.endY;/* for move */
	} else {
		thisFBTEST->testMode = mode;
		FBTEST_MSG_DEBUG("Error:X-Y coordinate to move the image is not in range\n");
		return -OFFSET_OUT_OF_BOUND;
	}
	if (thisFBTEST->outResolution.width != 0)
		blitReq->dst_rect.w = thisFBTEST->outResolution.width;/* for scale */
	if (thisFBTEST->outResolution.height != 0)
		blitReq->dst_rect.h = thisFBTEST->outResolution.height;/* for scale */

	if ((blitReq->dst_rect.x + blitReq->dst_rect.w) > vinfo->xres) {
		FBTEST_MSG_DEBUG("sum of output start coordinate x and display width is\
				greater than display panel width\n");
		return -RES_OUT_OF_BOUND;
	}
	if ((blitReq->dst_rect.y + blitReq->dst_rect.h) > vinfo->yres) {
		FBTEST_MSG_DEBUG("sum of output start coordinate y and display height is\
				greater than display panel height\n");
		return -RES_OUT_OF_BOUND;
	}
	if ((blitReq->src_rect.w > vinfo->xres)|| (blitReq->src_rect.h > vinfo->yres)||\
			(blitReq->dst_rect.w > vinfo->xres)||(blitReq->dst_rect.h > vinfo->yres))
	{
		FBTEST_MSG_DEBUG("\n[MULTIOP] Resolution OutofBound! (Line %d)\n",	__LINE__);
		return -RES_OUT_OF_BOUND;
	}
	blitReq->alpha = 0xFF;
	blitReq->transp_mask = MDP_TRANSP_NOP;
	if ((result = doBlit(list)) < 0) {
		VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
			__LINE__, result);
		return -BLIT_FAILED;
	}
	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (thisFBTEST->fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = thisFBTEST->fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}

	if (thisFBTEST->fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = thisFBTEST->fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	result = setOverlay(&overlay, FB, FB->fb_fmt, thisFBTEST->fileParams.inResolution.width,
			thisFBTEST->fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);

	result = doDisplayUpdate();
	unsetOverlay(FB, &overlay.id);

	if (result == TEST_RESULT_PASS) {
		VPRINT(verbose, "\nMultiop Test: PASS\n");
	}else
		VPRINT(verbose, "\nMultiop Test: FAIL\n");
	return result;

}

/* Legacy MDP3 tests */
int calcFPS(void) {
	int iterations, i;;
	struct timeval time_start, time_stop;
	struct fb_var_screeninfo * vinfo;
	struct fb_var_screeninfo vinfo_bak;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 50;

	paintBuffer(FB->fb_buf, BLACK, FB->fb_fmt, FB->fb_vinfo.xres,
			FB->fb_vinfo.yres, FB->fb_finfo.line_length);
	paintBuffer(FB->fb_buf_back, WHITE, FB->fb_fmt, FB->fb_vinfo.xres,
			FB->fb_vinfo.yres, FB->fb_finfo.line_length);

	vinfo_bak = FB->fb_vinfo;
	vinfo = &vinfo_bak;
	VPRINT(verbose, "CalcFPS test\n");
	// FPS with VSync ON
	vinfo->activate = FB_ACTIVATE_VBL;
	gettimeofday(&time_start,NULL);
	for (i = 0; i < (iterations); i++) {
		if (FB->fb_buf == FB->fb_buf_back)
			vinfo->yoffset = 0;
		else
			vinfo->yoffset = vinfo->yres ;
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed, vsync ON,"
					"yoffset=%d. line=%d\n", vinfo->yoffset,
					__LINE__);
			return TEST_RESULT_FAIL;
		}

		vinfo->yoffset = 0;
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed, vsync ON,"
					"yoffset=%d. line=%d\n", vinfo->yoffset,
					__LINE__);
			return TEST_RESULT_FAIL;
		}
	}
	gettimeofday(&time_stop,NULL);
	printf("Average FPS with VSync ON = %.1f fps\n",
			(float)(iterations*1000)/
			(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
				time_start.tv_sec*1000-time_start.tv_usec/1000));
	sleep(2);
	/*FPS with VSync OFF*/
	vinfo->activate = FB_ACTIVATE_NOW;

	gettimeofday(&time_start,NULL);
	for (i = 0; i < (iterations); i++) {
		if (FB->fb_buf == FB->fb_buf_back)
			vinfo->yoffset = 0;
		else
			vinfo->yoffset = vinfo->yres;

		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed, vsync OFF, yoffset=%d\n",
					vinfo->yoffset);
			return TEST_RESULT_FAIL;
		}

		vinfo->yoffset = 0;
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed, vsync OFF, yoffset=%d\n",
					vinfo->yoffset);
			return TEST_RESULT_FAIL;
		}
	}
	gettimeofday(&time_stop,NULL);
	printf("Average FPS with VSync OFF = %.1f fps\n",
			(float)(iterations*1000)/
			(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
				time_start.tv_sec*1000-time_start.tv_usec/1000));


	FB->fb_vinfo = vinfo_bak;

	return TEST_RESULT_PASS;
}

int pppTest(void)
{
	int samples, iterations, w, h, x, y, i, j;
	int result, testresult, fps_print;
	unsigned char * yptr, *uvptr;
	struct timeval time_start, time_stop;
	float ppp_time, dma_time;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;

	thisFBTEST = &FBTEST;
	VPRINT(verbose, "PPP test\n");

	testresult = TEST_RESULT_PASS;
	fps_print = (testLevel > 1) ? 1 : 0;

	vinfo = &(FB->fb_vinfo);
	memcpy(FB->fb_buf_back, yellow_face_data, (int)(FACE_WIDTH*FACE_HEIGHT*getFormatBpp(FACE_FORMAT)));

	union {
		char dummy[sizeof(struct mdp_blit_req_list) + sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;

	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req* blitReq;

	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	blitReq->src.width  = FACE_WIDTH;
	blitReq->src.height = FACE_HEIGHT;
	blitReq->src.format = FACE_FORMAT;
	blitReq->src.offset = FB->fb_buf_back - FB->fb_buf;
	blitReq->src.memory_id = FB->fb_fd;
	blitReq->dst.width  = vinfo->xres;
	blitReq->dst.height = vinfo->yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.memory_id = FB->fb_fd;
	blitReq->transp_mask = FACE_TRANSPMASK_RGB565;

	/* Rotation Test */
	/**********************************************************/
	blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = FACE_WIDTH;
	blitReq->dst_rect.h = FACE_HEIGHT;
	blitReq->src_rect.w = FACE_WIDTH;
	blitReq->src_rect.h = FACE_HEIGHT;

	drawBG();

	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 1;
	result = TEST_RESULT_PASS;

	for (i = 0, ppp_time = 0.0, dma_time = 0.0, samples = 0,
			w = blitReq->dst_rect.w, h = blitReq->dst_rect.h;
			i< iterations;
			i++) {
		blitReq->flags |= 0;
		blitReq->dst_rect.w = w;
		blitReq->dst_rect.h = h;
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
			printf("ERROR: MSM_FBIOBLT failed in rotation test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		samples++;
		ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed in rotation test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}

		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		usleep(TEST_DELAY_SHORT);

		blitReq->flags |= MDP_ROT_90;
		blitReq->dst_rect.w = h;
		blitReq->dst_rect.h = w;
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
			printf("ERROR: MSM_FBIOBLT failed in rotation test! line=%d\n",__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		samples++;
		ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		gettimeofday(&time_start,NULL);

		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed in rotation test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		usleep(TEST_DELAY_SHORT);

		blitReq->flags |= MDP_ROT_180;
		blitReq->dst_rect.w = w;
		blitReq->dst_rect.h = h;
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
			printf("ERROR: MSM_FBIOBLT failed in rotation test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		samples++;
		ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed in rotation test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		usleep(TEST_DELAY_SHORT);

		blitReq->flags |= MDP_ROT_270;
		blitReq->dst_rect.w = h;
		blitReq->dst_rect.h = w;
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
			printf("ERROR: MSM_FBIOBLT failed in rotation test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		samples++;
		ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed in rotation test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		usleep(TEST_DELAY_SHORT);
	}

	if (result == TEST_RESULT_FAIL)
		testresult = TEST_RESULT_FAIL;
	printf("Rotation Test : %s\n",
			(result == TEST_RESULT_FAIL) ? "FAIL" : "PASS");
	if (fps_print && (result == TEST_RESULT_PASS)) {
		printf("Average Blit (PPP) time   = %.3f us\n",
				(float)ppp_time*1000/samples);
		printf("Average Update (DMA) time = %.3f us\n",
				(float)dma_time*1000/samples);
		printf("Average FPS               = %.2f fps\n",
				(float)samples*1000/(ppp_time+dma_time));
	}

	/* Scaling Test */
	/**********************************************************/
	blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = FACE_WIDTH;
	blitReq->dst_rect.h = FACE_HEIGHT;
	blitReq->src_rect.w = FACE_WIDTH;
	blitReq->src_rect.h = FACE_HEIGHT;

	drawBG();

	result = TEST_RESULT_PASS;

	ppp_time = 0.0;
	dma_time = 0.0;
	samples = 0;
	w = blitReq->dst_rect.w;
	h = blitReq->dst_rect.h;

	blitReq->dst_rect.w = w/4+1;
	blitReq->dst_rect.h = h/4+1;

	while (((blitReq->dst_rect.w/2)*2 < ((FACE_WIDTH/2)*2*4)) &&
			((blitReq->dst_rect.h/2)*2 < ((FACE_HEIGHT/2)*2*4)) &&
			(blitReq->dst_rect.w < vinfo->xres) &&
			(blitReq->dst_rect.h < vinfo->yres)) {
		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
			printf("ERROR: MSM_FBIOBLT failed in scaling test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		samples++;
		ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
		blitReq->dst_rect.w++;
		blitReq->dst_rect.h++;


		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed in scaling test! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
		}
		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
	}
	usleep(TEST_DELAY_MEDIUM);	// Add visible delay
	if (result == TEST_RESULT_FAIL)
		testresult = TEST_RESULT_FAIL;
	printf("Scaling Test : %s\n",
			(result == TEST_RESULT_FAIL) ? "FAIL" : "PASS");
	if (fps_print && (result == TEST_RESULT_PASS)) {
		printf("Average Blit (PPP) time   = %.3f us\n",
				(float)ppp_time*1000 / samples);
		printf("Average Update (DMA) time = %.3f us\n",
				(float)dma_time*1000);
		printf("Average FPS               = %.2f fps\n",
				(float)samples*1000 / (ppp_time+dma_time*samples));
	}

	/* Blit Performance Test */
	/**********************************************************/
	blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = FACE_WIDTH;
	blitReq->dst_rect.h = FACE_HEIGHT;
	blitReq->src_rect.w = FACE_WIDTH;
	blitReq->src_rect.h = FACE_HEIGHT;

	drawBG();

	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 200;

	result = TEST_RESULT_PASS;

	for (i = 0, ppp_time = 0.0, dma_time = 0.0, samples = 0,
			w = blitReq->dst_rect.w, h = blitReq->dst_rect.h;
			i< iterations;
			i++) {

		x = (rand() % vinfo->xres) - w;
		if (x < 0) x = 0;
		blitReq->dst_rect.x = x;

		y = (rand() % vinfo->yres) - h;
		if (y < 0) y = 0;
		blitReq->dst_rect.y = y;

		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
			printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}
		gettimeofday(&time_stop,NULL);
		samples++;
		ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-
				time_start.tv_usec/1000);

		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
		}
		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-
				time_start.tv_usec/1000);
	}
	usleep(TEST_DELAY_MEDIUM);	// Add visible delay
	if (result == TEST_RESULT_FAIL)
		testresult = TEST_RESULT_FAIL;
	printf("Blit Performance Test : %s\n",
			(result == TEST_RESULT_FAIL) ? "FAIL" : "PASS");
	if (fps_print && (result == TEST_RESULT_PASS)) {
		printf("Average Blit (PPP) time   = %.3f us\n",
				(float)ppp_time*1000/samples);
		printf("Average Update (DMA) time = %.3f us\n",
				(float)dma_time*1000);
		printf("Average FPS               = %.2f fps\n",
				(float)samples*1000/(ppp_time+dma_time*samples));
	}

	/* Blend Performance Test */
	/************************************************************/
	blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = 0x80;
	blitReq->dst_rect.w = FACE_WIDTH;
	blitReq->dst_rect.h = FACE_HEIGHT;
	blitReq->src_rect.w = FACE_WIDTH;
	blitReq->src_rect.h = FACE_HEIGHT;

	drawBG();

	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 200;

	result = TEST_RESULT_PASS;

	for (i = 0, ppp_time = 0.0, dma_time = 0.0, samples = 0,
			w = blitReq->dst_rect.w, h = blitReq->dst_rect.h;
			i< iterations;
			i++) {
		x = (rand() % vinfo->xres) - w;
		if (x < 0) x = 0;
		blitReq->dst_rect.x = x;

		y = (rand() % vinfo->yres) - h;
		if (y < 0) y = 0;
		blitReq->dst_rect.y = y;

		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
			printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
			break;
		}

		gettimeofday(&time_stop,NULL);
		samples++;
		ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-
				time_start.tv_usec/1000);

		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
					__LINE__);
			result = TEST_RESULT_FAIL;
		}
		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/
				1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
	}
	usleep(TEST_DELAY_MEDIUM);	// Add visible delay
	if (result == TEST_RESULT_FAIL)
		testresult = TEST_RESULT_FAIL;
	printf("Blend Performance Test : %s\n",
			(result == TEST_RESULT_FAIL) ? "FAIL" : "PASS");
	if (fps_print && (result == TEST_RESULT_PASS)) {
		printf("Average Blit (PPP) time   = %.3f us\n",
				(float)ppp_time*1000/samples);
		printf("Average Update (DMA) time = %.3f us\n",
				(float)dma_time*1000);
		printf("Average FPS               = %.2f fps\n",
				(float)samples*1000/(ppp_time+dma_time*samples));
	}

	/* YUV Color Conversion Test */
	/*****************************************************************/
	blitReq->src.width  = YUV_WIDTH;
	blitReq->src.height = YUV_HEIGHT;
	blitReq->src.format = YUV_FORMAT;
	blitReq->src.offset = FB->fb_buf_back - FB->fb_buf;
	blitReq->src.memory_id = FB->fb_fd;
	blitReq->dst.width  = vinfo->xres;
	blitReq->dst.height = vinfo->yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.memory_id = FB->fb_fd;
	blitReq->transp_mask = YUV_TRANSPMASK;
	blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = YUV_WIDTH;
	blitReq->dst_rect.h = YUV_HEIGHT;
	blitReq->src_rect.w = YUV_WIDTH;
	blitReq->src_rect.h = YUV_HEIGHT;

	drawBG();

	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 1;

	yptr = uvptr = (unsigned char *)FB->fb_buf_back;
	uvptr += YUV_WIDTH*YUV_HEIGHT;

	ppp_time = 0.0;
	dma_time = 0.0;
	samples = 0;

	result = TEST_RESULT_PASS;

	while ((blitReq->dst_rect.w < (YUV_WIDTH*4-1)) &&
			(blitReq->dst_rect.h < (YUV_HEIGHT*4-1)) &&
			(blitReq->dst_rect.w < vinfo->xres) &&
			(blitReq->dst_rect.h < vinfo->yres)) {
		for (i = 0; i < iterations; i++) {
			for (j = 0; j < 10; j++) {
				memcpy(yptr, (unsigned char *)&mdp11_Y[0] + j*(YUV_WIDTH*YUV_HEIGHT), YUV_WIDTH*YUV_HEIGHT);
				memcpy(uvptr, (unsigned char *)&mdp11_CbCr[0] + j*((YUV_WIDTH*YUV_HEIGHT)/2), (YUV_WIDTH*YUV_HEIGHT)/2);
				gettimeofday(&time_start,NULL);
				if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
					printf("ERROR: MSM_FBIOBLT failed! line=%d\n",__LINE__);
					result = TEST_RESULT_FAIL;
					break;
				}

				gettimeofday(&time_stop,NULL);
				samples++;
				ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
			}
			if (result != TEST_RESULT_PASS) break;

			for (j = 8; j >= 0; j--) {
				memcpy(yptr, (unsigned char *)&mdp11_Y[0] + j*(YUV_WIDTH*YUV_HEIGHT), YUV_WIDTH*YUV_HEIGHT);
				memcpy(uvptr, (unsigned char *)&mdp11_CbCr[0] + j*((YUV_WIDTH*YUV_HEIGHT)/2), (YUV_WIDTH*YUV_HEIGHT)/2);
				gettimeofday(&time_start,NULL);
				if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
					printf("ERROR: MSM_FBIOBLT failed! line=%d\n",__LINE__);
					result = TEST_RESULT_FAIL;
					break;
				}

				gettimeofday(&time_stop,NULL);
				samples++;
				ppp_time += (time_stop.tv_sec*1000+time_stop.tv_usec/1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
			}
			if (result != TEST_RESULT_PASS) break;
		}
		if (result != TEST_RESULT_PASS)
			break;

		blitReq->dst_rect.w +=  YUV_WIDTH/4;
		blitReq->dst_rect.h +=  YUV_HEIGHT/4;

		gettimeofday(&time_start,NULL);
		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed in blend performance test! line=%d\n", __LINE__);
			result = TEST_RESULT_FAIL;
		}
		gettimeofday(&time_stop,NULL);
		dma_time += (time_stop.tv_sec*1000+time_stop.tv_usec/1000-time_start.tv_sec*1000-time_start.tv_usec/1000);
	}
	usleep(TEST_DELAY_MEDIUM);	// Add visible delay
	if (result == TEST_RESULT_FAIL)
		testresult = TEST_RESULT_FAIL;
	printf("YUV Test : %s\n", (result == TEST_RESULT_FAIL) ? "FAIL" : "PASS");
	if (fps_print && (result == TEST_RESULT_PASS)) {
		printf("Average Blit (PPP) time   = %.3f us\n", (float)ppp_time*1000/samples);
		printf("Average Update (DMA) time = %.3f us\n", (float)dma_time*1000);
		printf("Average FPS               = %.2f fps\n", (float)samples*1000/(ppp_time+dma_time*samples));
	}
	return testresult;
}

int pppTestVideo(void)
{
	struct mdp_blit_req* blitReq;
	FILE *imgRAW;
	struct timeval t_start, t_end;
	int iterations, result, frame, framesize, err, uwait, i;
	float scaleX, scaleY, fps, fps_avg;
	int maxScale;
	unsigned char *vid_buf_front, *vid_buf_back;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	/* QVGA Video Playback */

	struct Video video;

	VPRINT(verbose, "PPP Video test\n");
	maxScale = getMaxUpScale(mdp_version);
	strcpy(video.name, "vidQVGA.yuv");
	video.width = 320;
	video.height = 240;
	video.format = MDP_Y_CBCR_H2V2;
	video.frames = 100;
	video.fps = 15.0;

	framesize = video.width*video.height*getFormatBpp(video.format);
	scaleX = (float)FB->fb_vinfo.xres / video.width;
	scaleY = (float)FB->fb_vinfo.yres / video.height;

	if (scaleX < (1/maxScale) || scaleX > maxScale) {
		printf("SKIP: Cannot support QVGA video playback (scaleX = %.2f, scaleY = %.2f)\n", scaleX, scaleY);
		return TEST_RESULT_SKIP;
	}

	if (allocMEM(framesize*2))
		return TEST_RESULT_FAIL;


	if (thisFBTEST->iteration)
		iterations = thisFBTEST->iteration;
	else
		iterations = 1;

	union {
		char dummy[sizeof(struct mdp_blit_req_list) + sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} imgFrame;

	imgFrame.list.count = 1;

	blitReq = &imgFrame.list.req[0];
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	blitReq->src.width  = video.width;
	blitReq->src.height = video.height;
	blitReq->src.format = video.format;
	blitReq->src.memory_id = MEM->mem_fd_src;

	blitReq->dst.width  = FB->fb_vinfo.xres;;
	blitReq->dst.height = FB->fb_vinfo.yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.memory_id = FB->fb_fd;

	blitReq->transp_mask = 0xF81F;
	blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = 0xFF;
	blitReq->dst_rect.w = FB->fb_vinfo.xres;
	blitReq->dst_rect.h = FB->fb_vinfo.yres;;
	blitReq->src_rect.w = video.width;
	blitReq->src_rect.h = video.height;

	printf("\nOpening video (%s)\n", video.name);
	imgRAW = fopen(video.name, "r");
	if (imgRAW == NULL) {
		printf("ERROR! Cannot open raw video file vidQVGA.yuv!\n");
		printf("File is located at \\\\sun\\hydlnx\\displayLinux\\YUV" \
				"or baskin\\\\happyfeet\\users\\c_neekum\\video_files \n");
		return TEST_RESULT_FAIL;
	}

	result = TEST_RESULT_PASS;
	fps_avg = 0.0;
	uwait = 0;
	vid_buf_front = MEM->mem_buf;
	vid_buf_back = MEM->mem_buf + framesize;
	printf("           Playback ....");

	for (i = 0; i < iterations; i++) {

		for(frame = 0; frame < video.frames; frame++) {
			gettimeofday(&t_start,NULL);

			if (fseek(imgRAW, framesize * frame, SEEK_SET) != 0)// Fix: rewind to start of file
				break;

			err = fread (vid_buf_back, sizeof(unsigned char), framesize, imgRAW);
			swapPointers(&vid_buf_front, &vid_buf_back);
			blitReq->src.offset = vid_buf_front - MEM->mem_buf;

			if (err != framesize) {
				printf("\nERROR! RAW image read failed at frame %d (read %d of %d bytes)\n", frame, err, framesize);
				result = TEST_RESULT_FAIL;
				break;
			}

			if (ioctl(FB->fb_fd, MSMFB_BLIT, &imgFrame.list)) {
				printf("\nERROR! MSMFB_BLIT failed at frame %d! (Line %d)\n", frame, __LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
				printf("\nERROR! FBIOPAN_DISPLAY failed at frame %d! (Line %d)\n", frame, __LINE__);
				result = TEST_RESULT_FAIL;
			}


			gettimeofday(&t_end,NULL);
			fps = (float)(1000.0)/(float)(t_end.tv_sec*1000+t_end.tv_usec/1000-t_start.tv_sec*1000-t_start.tv_usec/1000);

			if (video.fps != 0.0) {
				uwait = (int)( 1000000.0 / video.fps - 1000000.0 / fps );
				if (uwait > 0.0)
					usleep(uwait);
			}

			gettimeofday(&t_end,NULL);
			fps = (float)(1000.0)/(float)(t_end.tv_sec*1000+t_end.tv_usec/1000-t_start.tv_sec*1000-t_start.tv_usec/1000);

			fps_avg += fps;
		}
	}
	if (result == TEST_RESULT_PASS) {
		fps_avg /= (video.frames * iterations);
		printf(" Done. FPS = %.2f", fps_avg);
	}

	printf("\nClose Video");
	fclose(imgRAW);

	return result;
}

int allcolorFormatPPPTest(void)
{
	int iterations, w, h, x, y, i, result;
	int testresult, fps_print, format;
	unsigned int scale = 4;
	struct fb_var_screeninfo *vinfo;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int remainder,d_w_1,s_w_1;

	testresult = TEST_RESULT_PASS;
	fps_print = (testLevel > 1) ? 1 : 0;

	vinfo = &(FB->fb_vinfo);
	VPRINT(verbose, "All Color format PPP test\n");

	for( format = MDP_RGB_565; format < MDP_Y_CRCB_H2V2_TILE; format++) {

		switch(format) {
			case MDP_RGB_565:
				printf("\nTesting color RGB_565 with YELLOW pattern");
				break;
			case MDP_XRGB_8888:
				printf("\nTesting color XRGB_8888 with YELLOW pattern");
				break;
			case MDP_Y_CBCR_H2V2:
				printf("\nTesting color Y_CBCR_H2V2 with GREY pattern");
				break;
			case MDP_ARGB_8888:
				printf("\nTesting color ARGB_8888 with YELLOW pattern");
				break;
			case MDP_RGB_888:
				printf("\nTesting color RGB_888 with YELLOW pattern");
				break;
			case MDP_Y_CRCB_H2V2:
				printf("\nTesting color Y_CRCB_H2V2 with GREY pattern");
				break;
			case MDP_YCRYCB_H2V1:
				printf("\nTesting color YCRYCB_H2V1 with GREY pattern");
				break;
			case MDP_Y_CBCR_H2V1:
				printf("\nTesting color Y_CBCR_H2V1 with GREY pattern");
				break;
			case MDP_Y_CRCB_H2V1:
				printf("\nTesting color format Y_CRCB_H2V1 with GREY pattern");
				break;
			case MDP_RGBA_8888:
				printf("\nTesting color RGBA_8888 with YELLOW pattern");
				break;
			case MDP_BGRA_8888:
				printf("\nTesting color BGRA_8888 with YELLOW pattern");
				break;
			case MDP_RGBX_8888:
				printf("\nTesting color RGBX_8888 with YELLOW pattern");
				break;
			default:
				printf("\n Error! Format:%d not supported in test", format);
				continue;
		}

		paintBuffer(FB->fb_buf_back, YELLOW, format, SQUARE_WIDTH,
				SQUARE_HEIGHT, SQUARE_WIDTH * getFormatBpp(format));

		union {
			char dummy[sizeof(struct mdp_blit_req_list)
				+ sizeof(struct mdp_blit_req)*1];
			struct mdp_blit_req_list list;
		} image;

		struct mdp_blit_req_list *list = &(image.list);
		struct mdp_blit_req *blitReq;

		image.list.count = 1;
		blitReq = &(image.list.req[0]);
		memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
		blitReq->src.width  = SQUARE_WIDTH;
		blitReq->src.height = SQUARE_HEIGHT;
		blitReq->src.format = format;
		blitReq->src.offset = FB->fb_buf_back - FB->fb_buf;
		blitReq->src.memory_id = FB->fb_fd;
		blitReq->dst.width  = vinfo->xres;
		blitReq->dst.height = vinfo->yres;
		blitReq->dst.format = FB->fb_fmt;
		blitReq->dst.memory_id = FB->fb_fd;
		blitReq->transp_mask = MDP_TRANSP_NOP;

		/* Rotation Test */
		/****************************************************************/
		blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
		blitReq->alpha  = 0xff;
		blitReq->dst_rect.w = SQUARE_WIDTH;
		blitReq->dst_rect.h = SQUARE_HEIGHT;
		blitReq->src_rect.w = SQUARE_WIDTH;
		blitReq->src_rect.h = SQUARE_HEIGHT;

		drawBG();
		if (thisFBTEST->iteration)
			iterations = thisFBTEST->iteration;
		else
			iterations = 1;
		result = TEST_RESULT_PASS;

		for (i = 0, w = blitReq->dst_rect.w, h = blitReq->dst_rect.h;
				i < iterations;
				i++) {
			blitReq->flags = 0;
			blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
			blitReq->dst_rect.w = w;
			blitReq->dst_rect.h = h;

			if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
				printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
				printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			usleep(TEST_DELAY_SHORT);

			blitReq->flags |= MDP_ROT_90;
			blitReq->dst_rect.w = h;
			blitReq->dst_rect.h = w;

			if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
				printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
				printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}
			usleep(TEST_DELAY_SHORT);

			blitReq->flags |= MDP_ROT_180;
			blitReq->dst_rect.w = w;
			blitReq->dst_rect.h = h;

			if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
				printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
				printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}
			usleep(TEST_DELAY_SHORT);

			blitReq->flags |= MDP_ROT_270;
			blitReq->dst_rect.w = h;
			blitReq->dst_rect.h = w;
			if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
				printf("\n ERROR: MSM_FBIOBLT failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}
			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
				printf("\n ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}
			usleep(TEST_DELAY_SHORT);
		}

		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		printf("\n Rotation Test : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");

		/* Scaling Test */
		/************************************************************************************************************/
		blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
		blitReq->alpha  = 0xff;
		blitReq->dst_rect.w = SQUARE_WIDTH;
		blitReq->dst_rect.h = SQUARE_HEIGHT;
		blitReq->src_rect.w = SQUARE_WIDTH;
		blitReq->src_rect.h = SQUARE_HEIGHT;

		scale = getMaxUpScale(mdp_version);

		drawBG();

		result = TEST_RESULT_PASS;
		w = blitReq->dst_rect.w;
		h = blitReq->dst_rect.h;

		blitReq->dst_rect.w = w/scale+1;
		blitReq->dst_rect.h = h/scale+1;

		while (blitReq->dst_rect.w < scale*SQUARE_WIDTH  &&
				blitReq->dst_rect.h < scale*SQUARE_HEIGHT &&
				(blitReq->dst_rect.w < vinfo->xres) &&
				(blitReq->dst_rect.h < vinfo->yres)) {
			/* MDP width split workaround */
			if ((format == MDP_YCRYCB_H2V1)
					&& (getFormatBpp(FB->fb_fmt) == 4)) {
				remainder = blitReq->dst_rect.w % 16;
				if (remainder == 14 || remainder == 6) {
					d_w_1 =  (blitReq->dst_rect.w / 2) - 1;
					s_w_1 =  (blitReq->src_rect.w / 2) - 1;
					if (d_w_1 > (4 * s_w_1)) {
						blitReq->dst_rect.w++;
						blitReq->dst_rect.h++;
						continue;
					}
				}
			}
			if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
				printf("\n ERROR: MSM_FBIOBLT failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
				printf("\n ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
			}
			blitReq->dst_rect.w++;
			blitReq->dst_rect.h++;

		}

		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		printf("\n Scaling Test : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL":"PASS");


		/* Blit Performance Test */
		/************************************************************************************************************/
		blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
		blitReq->alpha  = 0xff;
		blitReq->dst_rect.w = SQUARE_WIDTH;
		blitReq->dst_rect.h = SQUARE_HEIGHT;
		blitReq->src_rect.w = SQUARE_WIDTH;
		blitReq->src_rect.h = SQUARE_HEIGHT;

		drawBG();

		if (thisFBTEST->iteration)
			iterations = thisFBTEST->iteration;
		else
			iterations = 20;
		result = TEST_RESULT_PASS;
		for (i = 0, w = blitReq->dst_rect.w, h = blitReq->dst_rect.h;
				i < iterations;
				i++) {

			x = (rand() % vinfo->xres) - w;
			if (x < 0) x = 0;
			blitReq->dst_rect.x = x;

			if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
				printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			y = (rand() % vinfo->yres) - h;
			if (y < 0) y = 0;
			blitReq->dst_rect.y = y;


			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
				printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
			}

		}

		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		printf("\n Blit Performance Test : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL" : "PASS");

		/* Blend Performance Test */
		/***************************************************/

		blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
		blitReq->alpha  = 0xff;
		blitReq->dst_rect.w = SQUARE_WIDTH;
		blitReq->dst_rect.h = SQUARE_HEIGHT;
		blitReq->src_rect.w = SQUARE_WIDTH;
		blitReq->src_rect.h = SQUARE_HEIGHT;

		drawBG();

		if (thisFBTEST->iteration)
			iterations = thisFBTEST->iteration;
		else
			iterations = 20;
		result = TEST_RESULT_PASS;

		for (i = 0, w = blitReq->dst_rect.w, h = blitReq->dst_rect.h;
				i < iterations;
				i++) {

			x = (rand() % vinfo->xres) - w;
			if (x < 0) x = 0;
			blitReq->dst_rect.x = x;

			if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
				printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
				break;
			}

			y = (rand() % vinfo->yres) - h;
			if (y < 0) y = 0;
			blitReq->dst_rect.y = y;

			if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
				printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
						__LINE__);
				result = TEST_RESULT_FAIL;
			}

		}

		if (result == TEST_RESULT_FAIL)
			testresult = TEST_RESULT_FAIL;
		printf("\nBlend Performance Test : %s \n",
				(result == TEST_RESULT_FAIL) ? "FAIL" : "PASS");
	}
	return testresult;
}

int adversarialTest(void)
{
	int format = MDP_RGB_565;
	int size;
	struct fb_var_screeninfo *vinfo = &(FB->fb_vinfo);

	union {
		char dummy[sizeof(struct mdp_blit_req_list) +
			sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;

	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req* blitReq;

	VPRINT(verbose, "Adversarial test\n");

	size = YUV_WIDTH * YUV_HEIGHT * getFormatBpp(YUV_FORMAT);
	if (mdp3AllocMEM(size, DEST_BUFFER_SIZE)) {
		FBTEST_MSG_DEBUG("[FORMAT] mdp3AllocMEM failed! (Line %d)\n",__LINE__);
		return -PMEMALLOC_FAILED;
	}
	memcpy(MEM->mem_buf, yellow_face_data, (int)(YUV_WIDTH * YUV_HEIGHT * getFormatBpp(YUV_FORMAT)));

	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	blitReq->src.width  = SQUARE_WIDTH;
	blitReq->src.height = SQUARE_HEIGHT;
	blitReq->src.format = format;
	//blitReq->src.offset = FB->fb_buf_back - FB->fb_buf;
	blitReq->src.memory_id = MEM->mem_fd_src;
	blitReq->dst.width  = vinfo->xres;
	blitReq->dst.height = vinfo->yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.memory_id = MEM->mem_fd;
	blitReq->transp_mask = MDP_TRANSP_NOP;
	//blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = MDP_ALPHA_NOP;
	blitReq->dst_rect.w = SQUARE_WIDTH;
	blitReq->dst_rect.h = SQUARE_HEIGHT;
	blitReq->src_rect.w = SQUARE_WIDTH;
	blitReq->src_rect.h = SQUARE_HEIGHT;
    blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
                                        getFormatBpp(FB->fb_fmt)));

	/* illegal source format test */
	printf("\n Executing illegal source format test");
	blitReq->src.format = MDP_IMGTYPE_LIMIT2 + 1;
	if (ioctl(FB->fb_fd, MSMFB_BLIT, list) >= 0) {
		printf("\n ERROR: MSM_FBIOBLT failed illegal format test! line=%d",
				__LINE__);
		return TEST_RESULT_FAIL;
	}
	blitReq->src.format = format;
	printf("\n Passed illegal source format test");

	/* illegal destination format test */
	printf("\n Executing illegal destination format test");
	blitReq->dst.format = MDP_IMGTYPE_LIMIT2 + 1;
	if (ioctl(FB->fb_fd, MSMFB_BLIT, list) >= 0) {
		printf("\n ERROR: MSM_FBIOBLT failed illegal format test! line=%d",
				__LINE__);
		return TEST_RESULT_FAIL;
	}
	blitReq->dst.format = format;
	printf("\n Passed illegal destination format test");

	/* illegal source rectangle coordinate test */
	printf("\n Executing illegal source rectangle coordinate test");
	blitReq->src_rect.x = SQUARE_WIDTH;
	blitReq->src_rect.y = SQUARE_HEIGHT;
	if (ioctl(FB->fb_fd, MSMFB_BLIT, list) >= 0) {
		printf("\n ERROR: MSM_FBIOBLT failed illegal source coordinates test!"
				"line=%d", __LINE__);
		return TEST_RESULT_FAIL;
	}
	blitReq->src_rect.x = 0;
	blitReq->src_rect.y = 0;
	printf("\n Passed illegal source rectangle coordinate test");

	/* illegal source rectangle height and width test */
	printf("\n Executing illegal source rectangle height and width test");
	blitReq->src_rect.w = vinfo->xres + 2;
	blitReq->src_rect.h = vinfo->yres + 2;
	if (ioctl(FB->fb_fd, MSMFB_BLIT, list) >= 0) {
		printf("\n ERROR: MSM_FBIOBLT failed illegal source rectangle "
				"test! line=%d\n", __LINE__);
		return TEST_RESULT_FAIL;
	}
	blitReq->src_rect.w = SQUARE_WIDTH;
	blitReq->src_rect.h = SQUARE_HEIGHT;
	printf("\n Passed illegal source rectangle height and width test");

	/* illegal destination rectangle coordinate test */
	printf("\n Executing illegal destination rectangle coordinate test");
	blitReq->dst_rect.x = vinfo->xres;
	blitReq->dst_rect.y = vinfo->yres;
	if (ioctl(FB->fb_fd, MSMFB_BLIT, list) >= 0) {
		printf("\n ERROR: MSM_FBIOBLT failed illegal destination "
				"coordinates test! line=%d\n", __LINE__);
		return TEST_RESULT_FAIL;
	}
	blitReq->dst_rect.x = 0;
	blitReq->dst_rect.y = 0;
	printf("\n Passed illegal destination rectangle coordinate test");


	/* illegal destination rectangle height and width test */
	printf("\n Executing illegal destination rectangle height and width test");
	blitReq->dst_rect.w = vinfo->xres+2;
	blitReq->dst_rect.h = vinfo->yres+2;
	if (ioctl(FB->fb_fd, MSMFB_BLIT, list) >= 0) {
		printf("\n ERROR: MSM_FBIOBLT failed illegal destination "
				"rectangle test! line=%d\n", __LINE__);
		return TEST_RESULT_FAIL;
	}
	blitReq->dst_rect.w = 0;
	blitReq->dst_rect.h = 0;
	printf("\n Passed illegal destination rectangle height and width test");

	return TEST_RESULT_PASS;
}

int stressTest(void)
{

	int src_roi[4];
	int dst_roi[4];
	unsigned int dst_w, dst_h;
	unsigned int rotate, scale = 4;
	unsigned long count = 0;
	unsigned int line_length, xres_aligned, framesize;

	union
	{
		char dummy[sizeof(struct mdp_blit_req_list) +
			sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;

	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req* blitReq;
	struct timeval time_start, time_stop;
	int result;
	struct mdp_overlay overlay;
	struct mdp_buf_sync data;
	int releaseFd = -1;

	memset(&data, 0, sizeof(data));
	data.rel_fen_fd = &releaseFd;
	data.acq_fen_fd_cnt = 0;
	data.flags = MDP_BUF_SYNC_FLAG_RETIRE_FENCE;

	VPRINT(verbose, "Stress test\n");
	FB->fb_vinfo.yoffset = 0;
	xres_aligned = FB->fb_vinfo.xres;
	framesize = xres_aligned * FB->fb_vinfo.yres *
		getFormatBpp(MDP_RGB_565);

	FBTEST.fileParams.inResolution.width = FB->fb_vinfo.xres;
	FBTEST.fileParams.inResolution.height = FB->fb_vinfo.yres;
	FBTEST.fileParams.inputFormat = MDP_RGB_565;
	if(mdp3AllocMEM((framesize*2), DEST_BUFFER_SIZE)){
		printf("\n MEM allocation failed");
		return TEST_RESULT_FAIL;
	}
	memset(MEM->mem_buf_src, 0x00, framesize*2);

	line_length = xres_aligned * getFormatBpp(MDP_RGB_565);
	paintBuffer(MEM->mem_buf_src, YELLOW, MDP_RGB_565, FB->fb_vinfo.xres,
			FB->fb_vinfo.yres, line_length);
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	blitReq->src.width  = FBTEST.fileParams.inResolution.width;
	blitReq->src.height = FBTEST.fileParams.inResolution.height;
	blitReq->src.format = FBTEST.fileParams.inputFormat;
	blitReq->src.offset = 0;
	blitReq->src.memory_id = MEM->mem_fd_src;
	blitReq->dst.width  = FB->fb_vinfo.xres;;
	blitReq->dst.height = FB->fb_vinfo.yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
				getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
	blitReq->dst.memory_id = MEM->mem_fd;
	blitReq->transp_mask = MDP_TRANSP_NOP;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = FBTEST.fileParams.inResolution.width;
	blitReq->dst_rect.h = FBTEST.fileParams.inResolution.height;
	blitReq->src_rect.w = FBTEST.fileParams.inResolution.width;
	blitReq->src_rect.h = FBTEST.fileParams.inResolution.height;

	//scale = getMaxUpScale(mdp_version);
	scale =4;
	printf("\n Starting stress test");
	gettimeofday(&time_start,NULL);
	if ((result = doBlit(list)) < 0) {
		VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
				__LINE__, result);
		return -BLIT_FAILED;
	}
	gettimeofday(&time_stop,NULL);
	float f=(float)(time_stop.tv_sec*1000+time_stop.tv_usec/1000-
			time_start.tv_sec*1000-time_start.tv_usec/1000);
	VPRINT(verbose, " ### f=%f\n\n",f);

	memset(&overlay, 0, sizeof(struct mdp_overlay));
	if (FBTEST.fileParams.inResolution.width <= FB->fb_vinfo.xres) {
		dst_w = FBTEST.fileParams.inResolution.width;
	} else {
		dst_w = FB->fb_vinfo.xres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	if (FBTEST.fileParams.inResolution.height <= FB->fb_vinfo.yres) {
		dst_h = FBTEST.fileParams.inResolution.height;
	} else {
		dst_h = FB->fb_vinfo.yres;
		if (lookup_val(format_name_table, overlay.src.format, 0))
			overlay.flags = MDP_OV_PIPE_SHARE;
	}
	result = setOverlay(&overlay, FB, FB->fb_fmt, FBTEST.fileParams.inResolution.width,
			FBTEST.fileParams.inResolution.height, 0, 0, dst_w, dst_h, 0, 0,
			FB_OVERLAY_RGB_1, MDP_TRANSP_NOP, 0,
			MSMFB_NEW_REQUEST, 0);
	if (ioctl(FB->fb_fd, MSMFB_BUFFER_SYNC, &data) < 0){
		FBTEST_MSG_DEBUG("WARNING: Cannot enable BUF SYNC.\n");
	}
	result = doDisplayUpdate();

	while (1) {
		data.acq_fen_fd = data.rel_fen_fd;
		data.acq_fen_fd_cnt = 1;
		if (ioctl(FB->fb_fd, MSMFB_BUFFER_SYNC, &data) < 0){
			FBTEST_MSG_DEBUG("WARNING: Cannot enable BUF SYNC.\n");
		}
		/*drawBG();*/
		get_rand4(src_roi, FB->fb_vinfo.xres,
				FB->fb_vinfo.yres, 1);
		get_rand4(dst_roi, FB->fb_vinfo.xres,
				FB->fb_vinfo.yres/2, 2);

		blitReq->src_rect.x = src_roi[0];
		blitReq->src_rect.y = src_roi[1];
		blitReq->src_rect.w = src_roi[2];
		blitReq->src_rect.h = src_roi[3];

		blitReq->dst_rect.x = dst_roi[0];
		blitReq->dst_rect.y = dst_roi[1];

		srand((unsigned)time(NULL));
		rotate = rand() % 4;

		switch (rotate) {
		case 0:
		case 2:
			if (rotate == 0)
				blitReq->flags = MDP_ROT_90;
			else
				blitReq->flags = MDP_ROT_270;

			blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;

			for ( dst_h = blitReq->src_rect.w/scale + 1;
					dst_h < scale * blitReq->src_rect.w &&
					dst_h + blitReq->dst_rect.y < blitReq->dst.height;
					dst_h++) {

				if (dst_h == 1)
					continue;

				for ( dst_w = blitReq->src_rect.h/scale + 1;
						dst_w < scale * blitReq->src_rect.h &&
						dst_w + blitReq->dst_rect.x < blitReq->dst.width;
						dst_w++) {

					if (dst_w == 1)
						continue;

					blitReq->dst_rect.w = dst_w;
					blitReq->dst_rect.h = dst_h;

					printf("\n %dx%d %d,%d -> %dx%d %d,%d  rot=%s",
							src_roi[2], src_roi[3], src_roi[0], src_roi[1],
							dst_w, dst_h, dst_roi[0], dst_roi[1],
							rotate == 0 ? "90" : "270");

					blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
								getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
					gettimeofday(&time_start,NULL);
					printf("blitReq->dst.offset 0x%x ", blitReq->dst.offset);
					if ((result = doBlit(list)) < 0) {
						VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
								__LINE__, result);
						return -BLIT_FAILED;
					} else {
						if (++count%1000 == 0)
							fprintf(stderr, "%ld blits\n", count);
					}
					result = doDisplayUpdate();

				}
			}
			break;
		case 1:
		default:
			if (rotate == 1)
				blitReq->flags = MDP_ROT_180;
			else
				blitReq->flags = MDP_ROT_NOP;

			blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;

			for ( dst_h = blitReq->src_rect.h/scale + 1;
					dst_h < scale * blitReq->src_rect.h &&
					dst_h + blitReq->dst_rect.y < blitReq->dst.height;
					dst_h++) {

				if (dst_h == 1)
					continue;

				for ( dst_w = blitReq->src_rect.w/scale + 1;
						dst_w < scale * blitReq->src_rect.w &&
						dst_w + blitReq->dst_rect.x < blitReq->dst.width;
						dst_w++) {

					if (dst_w == 1)
						continue;

					blitReq->dst_rect.w = dst_w;
					blitReq->dst_rect.h = dst_h;

					printf("\n %dx%d %d,%d -> %dx%d %d,%d  rot=%s",
								src_roi[2], src_roi[3], src_roi[0], src_roi[1],
								dst_w, dst_h, dst_roi[0], dst_roi[1],
								rotate == 1 ? "180" : "0");


					blitReq->dst.offset = MEM->mem_size - ((FB->fb_vinfo.xres * FB->fb_vinfo.yres *
								getFormatBpp(FB->fb_fmt)) * (1 + swap % DEST_BUFFERS));
					gettimeofday(&time_start,NULL);
					if ((result = doBlit(list)) < 0) {
						VPRINT(verbose, "ERROR: MSM_FBIOBLT failed! line=%d result = %d\n",
								__LINE__, result);
						return -BLIT_FAILED;
					} else {
						if (++count%1000 == 0)
							fprintf(stderr, "%ld blits\n", count);
					}
					result = doDisplayUpdate();
				}
			}
			break;
		}
	}
}

int mddi_partial_update_test(void)
{
	int result;
	struct fb_var_screeninfo *vinfo;
	vinfo = &(FB->fb_vinfo);

	drawBG();
	memcpy(FB->fb_buf_back, yellow_face_data,
			(int)(FACE_WIDTH*FACE_HEIGHT*getFormatBpp(FACE_FORMAT)));

	union {
		char dummy[sizeof(struct mdp_blit_req_list) + sizeof(struct mdp_blit_req)*1];
		struct mdp_blit_req_list list;
	} image;
	struct mdp_blit_req_list *list = &(image.list);
	struct mdp_blit_req* blitReq;

	VPRINT(verbose, "MDDI Partial test\n");
	image.list.count = 1;
	blitReq = &(image.list.req[0]);
	memset(blitReq, 0x00, sizeof(struct mdp_blit_req));
	blitReq->src.width  = FACE_WIDTH;
	blitReq->src.height = FACE_HEIGHT;
	blitReq->src.format = FACE_FORMAT;
	blitReq->src.offset = FB->fb_buf_back - FB->fb_buf;
	blitReq->src.memory_id = FB->fb_fd;
	blitReq->dst.width  = vinfo->xres;
	blitReq->dst.height = vinfo->yres;
	blitReq->dst.format = FB->fb_fmt;
	blitReq->dst.memory_id = FB->fb_fd;
	blitReq->transp_mask = FACE_TRANSPMASK_RGB565;
	blitReq->flags |= MDP_MEMORY_ID_TYPE_FB;
	blitReq->alpha  = 0xff;
	blitReq->dst_rect.w = FACE_WIDTH;
	blitReq->dst_rect.h = FACE_HEIGHT;
	blitReq->src_rect.w = FACE_WIDTH;
	blitReq->src_rect.h = FACE_HEIGHT;

	if (ioctl(FB->fb_fd, MSMFB_BLIT, list) < 0) {
		printf("ERROR: MSM_FBIOBLT failed! line=%d\n",
				__LINE__);
		return TEST_RESULT_FAIL;
	}

	printf("Executing partial update for incomplete happy face\n");
	vinfo->reserved[0] = 0x54445055;
	vinfo->reserved[1] = 0;
	vinfo->reserved[2] = (__u32)(FACE_WIDTH/2) << 16 | (FACE_HEIGHT/2);
	if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
		printf("\n ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
				__LINE__);
		result = TEST_RESULT_FAIL;
	}
	usleep(TEST_DELAY_SHORT);

	printf("Executing partial update for full happy face\n");
	vinfo->reserved[0] = 0x54445055;
	vinfo->reserved[1] = 0;
	vinfo->reserved[2] = (__u32)FACE_WIDTH << 16 | FACE_HEIGHT;
	if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
		printf("\n ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
				__LINE__);
		result = TEST_RESULT_FAIL;
	}
	usleep(TEST_DELAY_SHORT);

	printf("Executing full screen update for happy face\n");
	vinfo->reserved[0] = 0;
	vinfo->reserved[1] = 0;
	vinfo->reserved[2] = (__u32)0x64 << 16 | 0x64;
	if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo) < 0) {
		printf("\n ERROR: FBIOPAN_DISPLAY failed! line=%d\n",
				__LINE__);
		result = TEST_RESULT_FAIL;
	}
	usleep(TEST_DELAY_SHORT);

	return result;
}


int mdp3AllocMEM(unsigned int srcSize, unsigned int destSize) {
	int result = -ENOMEM;

	if (MEM == NULL)
		return result;
	if (srcSize == 0) {
		FBTEST_MSG_DEBUG("Src Buff size is zero\n");
		return -1;
	}
#ifdef USE_ION
	struct ion_fd_data fd_data;
	struct ion_allocation_data ionAllocData;
	fd_data.fd = 0;

	if ((unsigned int)MEM->mem_size_src >= srcSize && MEM->mem_buf_src) {
		memset(MEM->mem_buf_src, 0x00, MEM->mem_size_src);
		FBTEST_MSG_DEBUG("MEM reuse successful (%d bytes at %p)\n", MEM->mem_size, MEM->mem_buf);
		return 0;
	}

	// If unable to reuse current MEM, unmap and close the current MEM
	// and create a clean one
	if(MEM->mem_size_src > 0){
		FBTEST_MSG_DEBUG("MEM->mem_size>0\n");
		munmap(MEM->mem_buf_src, MEM->mem_size_src);
		ioctl(MEM->fd, ION_IOC_FREE, &handle_data_src);
		ioctl(MEM->fd, ION_IOC_FREE, &handle_data);
		close(MEM->mem_fd);
		close(MEM->fd);

		result = chooseMEMDev();
		if(result < 0)
			showError(result, 1);

		result = openMEMDev();
		if(result < 0)
			showError(result, 1);
	}

	// First Allocate MEM
	MEM->mem_page_size = sysconf(_SC_PAGESIZE);
	MEM->mem_size = destSize;
	MEM->mem_size = (MEM->mem_size + MEM->mem_page_size - 1) & (~(MEM->mem_page_size - 1));
	ionAllocData.len = MEM->mem_size;
	ionAllocData.align = sysconf(_SC_PAGESIZE);
	ionAllocData.heap_id_mask =
		ION_HEAP(ION_IOMMU_HEAP_ID) |
		ION_HEAP(ION_CP_MM_HEAP_ID) |
		ION_HEAP(ION_CP_WB_HEAP_ID) |
		ION_HEAP(ION_SF_HEAP_ID);
	ionAllocData.flags = 0;

	result = ioctl(MEM->fd, ION_IOC_ALLOC,  &ionAllocData);
	if(result){

		FBTEST_MSG_DEBUG("ERROR! MEM_ALLOCATE failed.\n");
		close_devices();        //UTF: added for cleanup code addition.
		// Close MEM Dev <----------------------------------------------
		return -MALLOC_FAILED;

	} else {
		fd_data.handle = ionAllocData.handle;
		handle_data.handle = ionAllocData.handle;

		if(ioctl(MEM->fd, ION_IOC_MAP, &fd_data)){
			FBTEST_MSG_DEBUG("ERROR! ION_IOC_MAP failed.\n");
			close_devices();
		}else
		{
			MEM->mem_buf = mmap(NULL, MEM->mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
			MEM->mem_fd = fd_data.fd;
			if (MEM->mem_buf == MAP_FAILED) {
				FBTEST_MSG_DEBUG("ERROR: MEM MMAP failed!\n");
				close_devices();        //UTF: added for cleanup code addition.
				// Deallocate <------------------------------------------
				return -MMAP_FAILED;
			}

			memset(MEM->mem_buf, 0x00, MEM->mem_size);
			FBTEST_MSG_DEBUG("MEM Allocation successful (%d bytes at %p)\n", MEM->mem_size, MEM->mem_buf);
		}
	}
	// First Allocate MEM
	MEM->mem_page_size = sysconf(_SC_PAGESIZE);
	MEM->mem_size_src = srcSize;
	MEM->mem_size_src = (MEM->mem_size_src + MEM->mem_page_size - 1) & (~(MEM->mem_page_size - 1));
	ionAllocData.len = MEM->mem_size_src;
	ionAllocData.align = sysconf(_SC_PAGESIZE);
	ionAllocData.heap_id_mask =
		ION_HEAP(ION_IOMMU_HEAP_ID) |
		ION_HEAP(ION_CP_MM_HEAP_ID) |
		ION_HEAP(ION_CP_WB_HEAP_ID) |
		ION_HEAP(ION_SF_HEAP_ID);
	ionAllocData.flags = 0;

	result = ioctl(MEM->fd, ION_IOC_ALLOC,  &ionAllocData);
	if(result){

		FBTEST_MSG_DEBUG("ERROR! MEM_ALLOCATE failed.\n");
		close_devices();        //UTF: added for cleanup code addition.
		// Close MEM Dev <----------------------------------------------
		return -MALLOC_FAILED;

	} else {
		fd_data.handle = ionAllocData.handle;
		handle_data_src.handle = ionAllocData.handle;

		if(ioctl(MEM->fd, ION_IOC_MAP, &fd_data)){
			FBTEST_MSG_DEBUG("ERROR! ION_IOC_MAP failed.\n");
			close_devices();
		}else
		{
			MEM->mem_buf_src = mmap(NULL, MEM->mem_size_src, PROT_READ | PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
			MEM->mem_fd_src = fd_data.fd;
			if (MEM->mem_buf_src == MAP_FAILED) {
				FBTEST_MSG_DEBUG("ERROR: MEM MMAP failed!\n");
				close_devices();        //UTF: added for cleanup code addition.
				// Deallocate <------------------------------------------
				return -MMAP_FAILED;
			}

			memset(MEM->mem_buf_src, 0x00, MEM->mem_size_src);
			FBTEST_MSG_DEBUG("SRC BUFF MEM Allocation successful (%d bytes at %p)\n", MEM->mem_size_src, MEM->mem_buf_src);
		}
	}
#endif
	return 0;
}
