/******************************************************************************
  @file  rotator.c
  @brief This file contains test code to verify all functionality of
  msm_rotator

  DESCRIPTION
  rotator.c is a test program to verify the msm_rotator kernel driver.  It
  opens the rotator driver and rotates various image formats.  It can be run
  interactively for more test options.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

 -----------------------------------------------------------------------------
 Copyright (c) 2009 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
 -----------------------------------------------------------------------------

******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <getopt.h>
#include <errno.h>

#include <linux/msm_mdp.h>
#include "linux/msm_rotator.h"
#include <linux/fb.h>

#include "yellow_face.zif"
#include "lb.zif"
#include "mdp11_Y_10F.zif"
#include "mdp11_CbCr_10F.zif"
#include "ycrcbh2v1.zif"
#include "ycrcb_tile_image"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ALIGN16(x) (((x) + 15) & ~15)
#define MAX_IMG_W 320
#define MAX_IMG_H 240
#define MAX_IMG_BPP 4

#define MAX_SESSIONS 16

#define LB_WIDTH 100
#define LB_HEIGHT 100

#define PG_WIDTH 128
#define PG_HEIGHT 128

#define YUV_WIDTH 176
#define YUV_HEIGHT 144

#define FB_NAME_SIZE 64

static const char *fb_devs[] = {
	"/dev/graphics/fb0",
	"/dev/fb0",
};
#define NUM_FB_DEVS ARRAY_SIZE(fb_devs)

static int fb_fd;
static unsigned char *fb_mmap;
static unsigned char *fb_mmap_back;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;


static uint32_t test_set;
static uint8_t testid;
static uint8_t interactive;
static uint8_t dumpimg;
static uint8_t verbose;

/*
 * The yellow face issue (CR 289961: blank or half red garbled screen)
 * is caused by painting 16 bit RGB_565 yellow face data directly on
 * the 32 bit ABGR_8888 framebuffer.  This can be proven by converting
 * rgb_565 to abgr_8888 format first then garbled display gose away.
 * overlay_play_rotation function is added to have visual results for
 * all other test caces and test images.
 */

static int overlay_play_rotation(int fb_fd,
                                 int pmem_fd,
                                 struct msm_rotator_img_info *src,
                                 struct msm_rotator_data_info *dst);

static void bg_tile_draw(int fb_fd)
{
	unsigned char *srcp;
	unsigned char *dstp;
	unsigned int i, j, k;
	unsigned int width, height;
	struct fb_var_screeninfo vinfo;
	struct fb_var_screeninfo *var;
	unsigned char *outp = fb_mmap;

	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0)  {
		printf("can't retrieve vscreenInfo!\n");
		return;
	}

	var = &vinfo;

	for (i = 0; i < var->yres; i += LB_HEIGHT) {
		for (j = 0; j < var->xres; j += LB_WIDTH) {
			if ((j+LB_WIDTH) > var->xres)
				width = var->xres - j;
			else
				width = LB_WIDTH;

			if ((i+LB_HEIGHT) > var->yres)
				height = var->yres - i;
			else
				height = LB_HEIGHT;

			srcp = (unsigned char *)lb_data;
			dstp = outp;
			dstp += (j+i*var->xres)*2;

			for (k = 0; k < height; k++) {
				memcpy(dstp, srcp, width*2);
				dstp += var->xres*2;
				srcp += LB_WIDTH*2;
			}
		}
	}
}


void dump_img(unsigned char *buf, int size, char *filename)
{
	static FILE *fp;

	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open output file: %s\n", filename);
		return;
	}

	fwrite(buf, 1, size, fp);
	fclose(fp);
}

char *rotation_text(int r)
{
	switch (r) {
	case MDP_ROT_90:
		return "90 degrees";
	case MDP_FLIP_UD:
		return "Up/Down";
	case MDP_FLIP_LR:
		return "Left/Right";
	case MDP_FLIP_UD|MDP_FLIP_LR:
		return "Rotate U/D & L/R";
	case MDP_FLIP_UD|MDP_ROT_90:
		return "Rotate 90 & U/D";
	case MDP_FLIP_LR|MDP_ROT_90:
		return "Rotate 90 & L/R";
	case MDP_ROT_90|MDP_FLIP_UD|MDP_FLIP_LR:
		return "Rotate 90 & U/D & L/R";
	default:
		return "<no rotate>";
	}
}

static void interactive_inputs(struct msm_rotator_img_info *rinfo)
{

	int input = -1;
	int partial = 0;
	uint32_t temp;

	while ((input < '0') || (input > '7')) {
		printf("choose rotation (bitwise OR/add in decimal ");
		printf("for multiple):\n");
		printf("  0 : none\n");
		printf("  1 : left-right\n");
		printf("  2 : up-down\n");
		printf("  4 : 90 degrees\n");
		printf("> ");
		input = getchar();
		while (getchar() != '\n')
			;

		if ((input < '0') || (input > '7'))
			printf("Error: invalid choice!\n");
		else
			rinfo->rotations = input - '0';
	}

	input = -1;
	while ((input < '0') || (input > '1')) {
		printf("Choose portion of image to rotate\n");
		printf("  0 : Entire image\n");
		printf("  1 : Partial image\n");
		printf("> ");
		input = getchar();
		while (getchar() != '\n')
			;

		if ((input < '0') || (input > '1'))
			printf("Error: invalid choice!\n");
		else
			partial = input - '0';
	}
	if (partial) {
		rinfo->src_rect.h = rinfo->src.height/2;
		rinfo->src_rect.w = rinfo->src.width/4;
	} else {
		/* whole image */
		rinfo->src_rect.h = rinfo->src.height;
		rinfo->src_rect.w = rinfo->src.width;
		if ((rinfo->rotations & MDP_ROT_90) &&
		    (rinfo->dst.width != rinfo->dst.height)) {
			while ((input != 'Y') && (input != 'N')) {
				printf("Rotation of non-square whole image ");
				printf("will fail, swap destination width & ");
				printf(" height?\n(y/n) > ");
				input = toupper(getchar());
				while (getchar() != '\n')
					;

				if (input == 'Y') {
					temp = rinfo->dst.height;
					rinfo->dst.height = rinfo->dst.width;
					rinfo->dst.width = temp;
				} else if (input != 'N')
					printf("Error: invalid choice!\n");
			}
		}
	}

	if (partial) {
		input = -1;
		while ((input < '0') || (input > '1')) {
			printf("Choose offset into image to rotate\n");
			printf("  0 : none (0,0) \n");
			printf("  1 : height/4,width/4\n");
			printf("> ");
			input = getchar();
			while (getchar() != '\n')
				;

			if (input == '0') {
				rinfo->src_rect.x = 0;
				rinfo->src_rect.y = 0;
			} else if (input == '1') {
				rinfo->src_rect.x = rinfo->src.width/4;
				rinfo->src_rect.y = rinfo->src.height/4;
			} else
				printf("Error: invalid choice!\n");
		}
	} else {
		rinfo->src_rect.x = 0;
		rinfo->src_rect.y = 0;
	}
}

static void default_rect(struct msm_rotator_img_info *rinfo)
{
	rinfo->src_rect.h = rinfo->src.height;
	rinfo->src_rect.w = rinfo->src.width;
	rinfo->src_rect.x = 0;
	rinfo->src_rect.y = 0;
}

int rgb565_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	unsigned int i;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	struct timeval t1, t2, t3 = {0, 0};
	void *out_virt = NULL;

	if (verbose)
		printf("Test #1 : %dx%d RGB565 image\n", PG_WIDTH, PG_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));
	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = sizeof(yellow_face_data);
	out_virt = (unsigned char *)in_virt + sizeof(yellow_face_data);
	memcpy(in_virt, yellow_face_data, PG_WIDTH*PG_HEIGHT*2);
	memcpy(out_virt, yellow_face_data, PG_WIDTH*PG_HEIGHT*2);
	if (dumpimg)
		dump_img((unsigned char *)in_virt,
			 sizeof(yellow_face_data),
			 "rotator_rgb565_test_before.bin");

	iinfo.src.format = iinfo.dst.format = MDP_RGB_565;
	iinfo.src.height = iinfo.dst.height = PG_HEIGHT;
	iinfo.src.width = iinfo.dst.width = PG_WIDTH;
	if (interactive)
		interactive_inputs(&iinfo);
	else {
		default_rect(&iinfo);
		iinfo.rotations = MDP_ROT_90;
	}
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;
	iinfo.enable = 1;
	iinfo.secure = 0;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		printf("rotator start ioctl failed result = %d\n", result);
		return result;
	}
	dinfo.session_id = iinfo.session_id;
	if (verbose) {
		printf("rotating RGB565 image:  src: %dx%d rect: %dx%d ",
		       iinfo.src.width, iinfo.src.height,
		       iinfo.src_rect.w, iinfo.src_rect.h);
		printf("offset %d,%d %s\n",
		       iinfo.src_rect.x, iinfo.src_rect.y,
		       rotation_text(iinfo.rotations));
		gettimeofday(&t1, NULL);
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (verbose) {
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
	}
	if (result < 0) {
		printf("rotator ioctl failed result = %d\n", result);
		return result;
	} else {
		if (verbose)
			printf("RGB565 rotation passed in %f sec\n",
			       (double)t3.tv_sec +
			       (double)t3.tv_usec/1000000.0);
		if (dumpimg)
			dump_img(out_virt,
				 iinfo.dst.height*iinfo.dst.width*2,
				 "rotator_rgb565_test_after.bin");
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0)
		printf("rotator finish ioctl failed result = %d\n",
		       result);

	if (interactive && (result >= 0)) {
                result = overlay_play_rotation(fb_fd, pmem_id, &iinfo, &dinfo);
                if ( result < 0) {
                        printf("overlay play failed.\n");
                }
	}

	return result;
}


int bgr565_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	unsigned int i;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	struct timeval t1, t2, t3 = {0, 0};
	void *out_virt = NULL;
	unsigned short *sp;

	if (verbose)
		printf("Test #2 : %dx%d BGR565 image\n", PG_WIDTH, PG_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = sizeof(yellow_face_data);
	out_virt = (unsigned char *)in_virt + sizeof(yellow_face_data);
	sp = (unsigned short *)in_virt;
	for (i = 0; i < PG_WIDTH*PG_HEIGHT; i++)
		/* swap red & blue positions */
		sp[i] = ((yellow_face_data[i] & 0x001f) << 11) |
			(yellow_face_data[i] & 0x07e0)        |
			((yellow_face_data[i] & 0xf100) >> 11);
	if (dumpimg)
		dump_img((unsigned char *)in_virt,
			 sizeof(yellow_face_data),
			 "rotator_bgr565_test_before.bin");

	iinfo.src.format = iinfo.dst.format = MDP_BGR_565;
	iinfo.src.height = iinfo.dst.height = PG_HEIGHT;
	iinfo.src.width = iinfo.dst.width = PG_WIDTH;
	if (interactive)
		interactive_inputs(&iinfo);
	else {
		default_rect(&iinfo);
		iinfo.rotations = MDP_ROT_90;
	}
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;
	iinfo.enable = 1;
	iinfo.secure = 0;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		printf("rotator start ioctl failed result = %d\n", result);
		return result;
	}
	dinfo.session_id = iinfo.session_id;
	if (verbose) {
		printf("***FB driver does not support display of this ");
		printf("format yet...\n");
		printf("rotating BGR565 image:  src: %dx%d rect: %dx%d ",
		       iinfo.src.width, iinfo.src.height,
		       iinfo.src_rect.w, iinfo.src_rect.h);
		printf("offset %d,%d\n", iinfo.src_rect.x, iinfo.src_rect.y);
		gettimeofday(&t1, NULL);
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (verbose) {
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
	}
	if (result < 0) {
		printf("rotator ioctl failed result = %d\n", result);
		return result;
	} else {
		if (verbose)
			printf("BGR565 rotation passed in %f sec\n",
			       (double)t3.tv_sec +
			       (double)t3.tv_usec/1000000.0);
		if (dumpimg)
			dump_img(out_virt,
				 iinfo.dst.height*iinfo.dst.width*2,
				 "rotator_bgr565_test_after.bin");
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0)
		printf("rotator finish ioctl failed result = %d\n",
		       result);

	if (interactive && (result >= 0)) {
                result = overlay_play_rotation(fb_fd, pmem_id, &iinfo, &dinfo);
                if ( result < 0) {
                        printf("overlay play failed.\n");
                }
	}

	return result;
}

static int rgb888_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	int i;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	struct timeval t1, t2, t3 = {0, 0};
	void *out_virt = NULL;
	unsigned char *cp;

	if (verbose)
		printf("Test #3 : %dx%d RGB888 image\n", PG_WIDTH, PG_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = PG_WIDTH*PG_HEIGHT*3;
	out_virt = (unsigned char *)in_virt + PG_WIDTH*PG_HEIGHT*3;
	cp = (unsigned char *)in_virt;
	for (i = 0; i < PG_WIDTH*PG_HEIGHT; i++) {
		/* make all components into 8 bit values */
		*cp = (yellow_face_data[i] & 0x001f) << 3;
		cp++;
		*cp = (yellow_face_data[i] & 0x07e0) >> 3;
		cp++;
		*cp = (yellow_face_data[i] & 0xf100) >> 8;
		cp++;
	}
	if (dumpimg)
		dump_img((unsigned char *)in_virt,
			 PG_WIDTH*PG_HEIGHT*3,
			 "rotator_rgb888_test_before.bin");

	iinfo.src.format = iinfo.dst.format = MDP_RGB_888;
	iinfo.src.height = iinfo.dst.height = PG_HEIGHT;
	iinfo.src.width = iinfo.dst.width = PG_WIDTH;
	if (interactive)
		interactive_inputs(&iinfo);
	else {
		default_rect(&iinfo);
		iinfo.rotations = MDP_ROT_90;
	}
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;
	iinfo.enable = 1;
	iinfo.secure = 0;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		printf("rotator start ioctl failed result = %d\n", result);
		return result;
	}
	dinfo.session_id = iinfo.session_id;
	if (verbose) {
		printf("***FB driver does not support display of this format");
		printf(" yet...\n");
		printf("rotating RGB888 image:  src: %dx%d rect: %dx%d offset",
		       iinfo.src.width, iinfo.src.height,
		       iinfo.src_rect.w, iinfo.src_rect.h);
		printf(" %d,%d\n", iinfo.src_rect.x, iinfo.src_rect.y);
		gettimeofday(&t1, NULL);
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (verbose) {
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
	}
	if (result < 0) {
		printf("rotator ioctl failed result = %d\n", result);
		return result;
	} else {
		if (verbose)
			printf("RGB888 rotation passed in %f sec\n",
			       (double)t3.tv_sec +
			       (double)t3.tv_usec/1000000.0);
		if (dumpimg)
			dump_img(out_virt,
				 iinfo.dst.height*iinfo.dst.width*3,
				 "rotator_rgb888_test_after.bin");
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0)
		printf("rotator finish ioctl failed result = %d\n",
		       result);

	if (interactive && (result >= 0)) {
                result = overlay_play_rotation(fb_fd, pmem_id, &iinfo, &dinfo);
                if ( result < 0) {
                        printf ("overlay set failed. \n");
                }
	}

	return result;
}


int y_cr_cb_gh2v2_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
    int i;
    int result = 0;
    struct msm_rotator_img_info iinfo;
    struct msm_rotator_data_info dinfo;
    int partial_size;
    struct timeval t1, t2, t3 = {0, 0};
    void *out_virt = NULL;
    unsigned char *yptr;
    unsigned char *uvptr;
    FILE *imgRAW;
    int framesize, err;

    printf("\nOpening video\n");
    imgRAW = fopen("yv12_qcif.yuv", "rb");

    if (imgRAW == NULL) {
        printf("ERROR! Cannot open raw video file!\n");
        return -1;
    }

    framesize = ((YUV_WIDTH * YUV_HEIGHT) + (ALIGN16(YUV_WIDTH/2) * YUV_HEIGHT));
    err = fread (in_virt, sizeof(unsigned char), framesize, imgRAW);

    if (err != framesize) {
        printf("\nERROR! RAW image read failed (read %d of %d bytes)\n", err, framesize);
        return -1;
    }

        if (verbose)
                printf("Test #5 : %dx%d YV12 image\n",
                       YUV_WIDTH, YUV_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

        dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
        dinfo.src.flags = 0;
        dinfo.dst.flags = 0;
        dinfo.src.offset = 0;
        dinfo.dst.offset = framesize;//YUV_WIDTH*YUV_HEIGHT*2;
        out_virt = (unsigned char *)in_virt + dinfo.dst.offset;

    if (dumpimg)
        dump_img(in_virt, framesize, "rotator_yv12_test_before.bin");
    iinfo.src.format = MDP_Y_CR_CB_GH2V2;
    iinfo.dst.format = MDP_Y_CRCB_H2V2;

        iinfo.src.height = YUV_HEIGHT;
        iinfo.src.width = YUV_WIDTH;
        iinfo.dst.height = YUV_WIDTH;
        iinfo.dst.width = YUV_HEIGHT;

        if (interactive)
                interactive_inputs(&iinfo);
        else {
                default_rect(&iinfo);
                iinfo.rotations = MDP_ROT_90;//MDP_FLIP_UD;
        }
        iinfo.dst_x = iinfo.src_rect.x;
        iinfo.dst_y = iinfo.src_rect.y;
        iinfo.enable = 1;
        iinfo.secure = 0;
        result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
        if (result < 0) {
                printf("rotator start ioctl failed result = %d\n", result);
                return result;
        }
        dinfo.session_id = iinfo.session_id;
        if (verbose) {
                printf("***FB driver does not support display of YUV yet...\n");
                printf("rotating YV12 image  ");
                printf("src: %dx%d rect: %dx%d src offset %d,%d dst ",
                       iinfo.src.width, iinfo.src.height,
                       iinfo.src_rect.w, iinfo.src_rect.h,
                       iinfo.src_rect.x, iinfo.src_rect.y);
                printf("offset %d,%d\n",
                       iinfo.dst_x, iinfo.dst_y);
                gettimeofday(&t1, NULL);
        }
        result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
        if (verbose) {
                gettimeofday(&t2, NULL);
                timersub(&t2, &t1, &t3);
        }
        if (result < 0) {
                printf("rotator ioctl failed result = %d\n", result);
                return result;
        } else {
                if (verbose)
                        printf("YCbCr rotation passed in %f sec\n",
                               (double)t3.tv_sec +
                               (double)t3.tv_usec/1000000.0);
                if (dumpimg)
                        dump_img(out_virt,
                                 iinfo.dst.height*iinfo.dst.width +
                                 (iinfo.dst.height*iinfo.dst.width/2),
                                 "rotator_yv12_test_after.bin");
        }
        result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
        if (result < 0)
                printf("rotator finish ioctl failed result = %d\n",
                       result);
        if (interactive) {
                printf("Press enter to continue...\n");
                getchar();
        }

        return result;
}


int ycbcr_h2v2_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	int i;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	int partial_size;
	struct timeval t1, t2, t3 = {0, 0};
	void *out_virt = NULL;
	unsigned char *yptr;
	unsigned char *uvptr;

	if (verbose)
		printf("Test #4 : %dx%d YCbCr H2V2 image\n",
		       YUV_WIDTH, YUV_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = YUV_WIDTH*YUV_HEIGHT*2;
	out_virt = (unsigned char *)in_virt + dinfo.dst.offset;
	yptr = (unsigned char *)in_virt;
	uvptr = (unsigned char *)in_virt + YUV_WIDTH*YUV_HEIGHT;
	memcpy(yptr, (unsigned char *)&mdp11_Y[0], YUV_WIDTH*YUV_HEIGHT);
	memcpy(uvptr, (unsigned char *)&mdp11_CbCr[0],
	       (YUV_WIDTH*YUV_HEIGHT)/2);
	if (dumpimg)
		dump_img(in_virt,
			 YUV_WIDTH*YUV_HEIGHT+(YUV_WIDTH*YUV_HEIGHT/2),
			 "rotator_ycbcr_h2v2_test_before.bin");
	iinfo.src.format = iinfo.dst.format = MDP_Y_CBCR_H2V2;
	iinfo.src.height = iinfo.dst.height = YUV_HEIGHT;
	iinfo.src.width = iinfo.dst.width = YUV_WIDTH;
	if (interactive)
		interactive_inputs(&iinfo);
	else {
		default_rect(&iinfo);
		iinfo.rotations = MDP_FLIP_UD;
	}
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;
	iinfo.enable = 1;
	iinfo.secure = 0;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		printf("rotator start ioctl failed result = %d\n", result);
		return result;
	}
	dinfo.session_id = iinfo.session_id;
	if (verbose) {
		printf("***FB driver does not support display of YUV yet...\n");
		printf("rotating YCbCr H2V2 image  ");
		printf("src: %dx%d rect: %dx%d src offset %d,%d dst ",
		       iinfo.src.width, iinfo.src.height,
		       iinfo.src_rect.w, iinfo.src_rect.h,
		       iinfo.src_rect.x, iinfo.src_rect.y);
		printf("offset %d,%d\n",
		       iinfo.dst_x, iinfo.dst_y);
		gettimeofday(&t1, NULL);
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (verbose) {
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
	}
	if (result < 0) {
		printf("rotator ioctl failed result = %d\n", result);
		return result;
	} else {
		if (verbose)
			printf("YCbCr rotation passed in %f sec\n",
			       (double)t3.tv_sec +
			       (double)t3.tv_usec/1000000.0);
		if (dumpimg)
			dump_img(out_virt,
				 iinfo.dst.height*iinfo.dst.width +
				 (iinfo.dst.height*iinfo.dst.width/2),
				 "rotator_ycbcr_h2v2_test_after.bin");
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0)
		printf("rotator finish ioctl failed result = %d\n",
		       result);

	if (interactive && (result >= 0)) {
                result = overlay_play_rotation(fb_fd, pmem_id, &iinfo, &dinfo);
                if ( result < 0) {
                        printf ("overlay play failed. \n");
                }
	}

	return result;
}

int ycbcr_h2v2_tile_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	int i;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	int partial_size;
	struct timeval t1, t2, t3 = {0, 0};
	void *out_virt = NULL;
	unsigned char *yptr;
	unsigned char *uvptr;

	struct mdp_overlay overlay;
	struct msmfb_overlay_data ovdata;
	int overlay_id;

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

	bg_tile_draw(fb_fd);

	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = (MAX_IMG_W*MAX_IMG_H*3)/2;


	out_virt = (unsigned char *)in_virt + dinfo.dst.offset;
	yptr = (unsigned char *)in_virt;
	memcpy(yptr, (unsigned char *)&tile_image[0],
		   (MAX_IMG_W*MAX_IMG_H*3)/2);


	iinfo.src.format = MDP_Y_CRCB_H2V2_TILE;
	iinfo.dst.format = MDP_Y_CRCB_H2V2;
	iinfo.src.height = iinfo.dst.width = MAX_IMG_H;
	iinfo.src.width = iinfo.dst.height = MAX_IMG_W;
	iinfo.src_rect.w = iinfo.src.width;
	iinfo.src_rect.h = iinfo.src.height;
	iinfo.src_rect.x = 0;
	iinfo.src_rect.y = 0;
	iinfo.rotations = MDP_ROT_90|MDP_FLIP_LR;
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;

	iinfo.enable = 1;
	iinfo.secure = 0;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		printf("rotator start ioctl failed result = %d\n", result);
		return result;
	}

	dinfo.session_id = iinfo.session_id;

	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);

	if (result < 0) {
		printf("rotator ioctl failed result = %d\n", result);
		return result;
	}

	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0)
		printf("rotator finish ioctl failed result = %d\n",
		       result);

	printf(" Image rotated : preparing for display...\n");

	overlay.src.width  = MAX_IMG_H;
	overlay.src.height = MAX_IMG_W;
	overlay.src.format = MDP_Y_CRCB_H2V2;
	overlay.src_rect.x = 0;
	overlay.src_rect.y = 0;
	overlay.src_rect.w = MAX_IMG_H;
	overlay.src_rect.h = MAX_IMG_W;

	overlay.dst_rect.x = 0;
	overlay.dst_rect.y = 0;
	overlay.dst_rect.w = vinfo.xres;;
	overlay.dst_rect.h = vinfo.yres;


	overlay.z_order = 0;
	overlay.alpha = MDP_ALPHA_NOP;
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.flags = MDP_FLIP_LR | MDP_FLIP_UD;
	overlay.is_fg = 1;
	overlay.id = MSMFB_NEW_REQUEST;


	overlay_id = ioctl(fb_fd, MSMFB_OVERLAY_SET, &overlay);

	if (overlay_id < 0) {
		printf("\n Ioctl MSMFB_OVERLAY_SET failed");
		return -1;
	}

	overlay_id = overlay.id;
	ovdata.id = overlay_id;
	ovdata.data.offset = (MAX_IMG_W*MAX_IMG_H*3)/2;
	ovdata.data.memory_id = pmem_id;


	if (ioctl(fb_fd, MSMFB_OVERLAY_PLAY, &ovdata)) {
		printf("\n Ioctl MSMFB_OVERLAY_PLAY failed \n");
		return -1;
	}

	sleep(2);

	if (ioctl(fb_fd, MSMFB_OVERLAY_UNSET, &overlay_id)) {
		printf("\n Ioctl MSMFB_OVERLAY_UNSET failed \n");
		return -1;
	}

	if (ioctl(fb_fd, FBIOPAN_DISPLAY, &vinfo) < 0) {
			printf("Ioctl FBIOPAN_DISPLAY: failed \n");
			return -1;
	}

	return result;
}


int ycrcb_h2v1_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	int i;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	int partial_size;
	struct timeval t1, t2, t3 = {0, 0};
	void *out_virt = NULL;
	unsigned char *yptr;
	unsigned char *uvptr;

	if (verbose)
		printf("Test #5 : %dx%d YCrCb H2V1 image\n",
		       YUV_WIDTH, YUV_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = YCRCBH2V1_WIDTH*YCRCBH2V1_HEIGHT*2;
	out_virt = (unsigned char *)in_virt + dinfo.dst.offset;
	memcpy(in_virt, (unsigned char *)&ycrcb_h2v1,
	       YCRCBH2V1_HEIGHT*YCRCBH2V1_WIDTH*2);
	if (dumpimg)
		dump_img(in_virt, YCRCBH2V1_HEIGHT*YCRCBH2V1_WIDTH*2,
			 "rotator_ycrcb_h2v1_test_before.bin");
	iinfo.src.format = iinfo.dst.format = MDP_Y_CBCR_H2V1;
	iinfo.src.height = iinfo.dst.height = YCRCBH2V1_HEIGHT;
	iinfo.src.width = iinfo.dst.width  = YCRCBH2V1_WIDTH;
	if (interactive)
		interactive_inputs(&iinfo);
	else {
		default_rect(&iinfo);
		iinfo.rotations = MDP_FLIP_UD;
	}
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;
	iinfo.enable = 1;
	iinfo.secure = 0;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		printf("rotator start ioctl failed result = %d\n", result);
		return result;
	}
	dinfo.session_id = iinfo.session_id;
	if (verbose) {
		printf("***FB driver does not support display of this ");
		printf("format yet...\n");
		printf("rotating YCrCb H2V1 image  ");
		printf("src: %dx%d rect: %dx%d src offset %d,%d dst ",
		       iinfo.src.width, iinfo.src.height,
		       iinfo.src_rect.w, iinfo.src_rect.h,
		       iinfo.src_rect.x, iinfo.src_rect.y);
		printf("offset %d,%d\n",
		       iinfo.dst_x, iinfo.dst_y);
		gettimeofday(&t1, NULL);
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (verbose) {
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
	}
	if (result < 0) {
		printf("rotator ioctl failed result = %d\n", result);
		return result;
	} else {
		if (verbose)
			printf("YCrCb H2V1 rotation passed in %f sec\n",
			       (double)t3.tv_sec +
			       (double)t3.tv_usec/1000000.0);
		if (dumpimg)
			dump_img(out_virt,
				 iinfo.dst.height*iinfo.dst.width*2,
				 "rotator_ycrcb_h2v1_test_after.bin");
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0)
		printf("rotator finish ioctl failed result = %d\n",
		       result);

	if (interactive && (result >= 0)) {
                result = overlay_play_rotation(fb_fd, pmem_id, &iinfo, &dinfo);
                if ( result < 0) {
                        printf ("overlay play failed. \n");
                }
	}

	return result;
}

static int rgb565_incr_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	unsigned int i, j;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	struct timeval t1, t2, t3 = {0, 0};
	unsigned int pg_height, pg_width;
	unsigned short *sp;

	if (verbose)
		printf("Test #6 : %dx%d RGB565 incremental image\n",
		       PG_WIDTH, PG_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = sizeof(yellow_face_data);
	iinfo.src.format = iinfo.dst.format = MDP_RGB_565;
	iinfo.rotations = MDP_FLIP_UD;
	iinfo.dst_x = 0;
	iinfo.dst_y = 0;

	j = pg_height = pg_width = 1;
	while ((pg_height <= PG_HEIGHT) && (pg_width <= PG_WIDTH)) {
		sp = (unsigned short *)in_virt;
		for (i = 0; i < pg_height; i++) {
			memcpy(sp, &yellow_face_data[i*PG_WIDTH], pg_width);
			sp += pg_width;
		}

		iinfo.src.height = iinfo.dst.height = pg_height;
		iinfo.src.width = iinfo.dst.width = pg_width;
		iinfo.enable = 1;
		iinfo.secure = 0;
		default_rect(&iinfo);
		result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
		if (result < 0) {
			printf("rotator start ioctl failed result = %d\n",
			       result);
			goto rgb565_incr_exit;
		}
		dinfo.session_id = iinfo.session_id;
		if (verbose) {
			printf("rotating RGB565 image: src: %dx%d ",
			       iinfo.src.width, iinfo.src.height);
			printf("rect: %dx%d ",
			       iinfo.src_rect.w, iinfo.src_rect.h);
			printf("offset %d,%d %s\n",
			       iinfo.src_rect.x, iinfo.src_rect.y,
			       rotation_text(iinfo.rotations));
			gettimeofday(&t1, NULL);
		}
		result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
		if (verbose) {
			gettimeofday(&t2, NULL);
			timersub(&t2, &t1, &t3);
		}
		if (result < 0) {
			printf("rotator ioctl failed result = %d\n", result);
			goto rgb565_incr_exit;
		} else if (verbose)
				printf("RGB565 rotation passed in %f sec\n",
				       (double)t3.tv_sec +
				       (double)t3.tv_usec/1000000.0);
		result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH,
			       &dinfo.session_id);
		if (result < 0)
			printf("rotator finish ioctl failed result = %d\n",
			       result);
		if ((j++ % 2) == 0)
			pg_height++;
		else
			pg_width++;
	}

rgb565_incr_exit:
	return result;
}

#define MULTI_TIMES 10
int rgb565_multi_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	unsigned int i, j;
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	struct timeval t1, t2, t3 = {0, 0};
	void *out_virt = NULL;

	if (verbose)
		printf("Test #7 : %dx%d RGB565 image multiple times, one id\n",
		       PG_WIDTH, PG_HEIGHT);

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));

	dinfo.src.memory_id = dinfo.dst.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.offset = sizeof(yellow_face_data);
	out_virt = (unsigned char *)in_virt + sizeof(yellow_face_data);
	memcpy(in_virt, yellow_face_data, PG_WIDTH*PG_HEIGHT*2);
	memcpy(out_virt, yellow_face_data, PG_WIDTH*PG_HEIGHT*2);
	iinfo.src.format = iinfo.dst.format = MDP_RGB_565;
	iinfo.src.height = iinfo.dst.height = PG_HEIGHT;
	iinfo.src.width = iinfo.dst.width = PG_WIDTH;
	iinfo.enable = 1;
	iinfo.secure = 0;
	default_rect(&iinfo);
	iinfo.rotations = MDP_ROT_90;
	iinfo.dst_x = iinfo.src_rect.x;
	iinfo.dst_y = iinfo.src_rect.y;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		printf("rotator start ioctl failed result = %d\n", result);
		return result;
	}
	dinfo.session_id = iinfo.session_id;
	for (j = 0; j < MULTI_TIMES; j++) {
		if (verbose) {
			printf("rotating RGB565 image:  src: %dx%d ",
			       iinfo.src.width, iinfo.src.height);
			printf("rect: %dx%d offset %d,%d %s\n",
			       iinfo.src_rect.w, iinfo.src_rect.h,
			       iinfo.src_rect.x, iinfo.src_rect.y,
			       rotation_text(iinfo.rotations));
			gettimeofday(&t1, NULL);
		}
		result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
		if (verbose) {
			gettimeofday(&t2, NULL);
			timersub(&t2, &t1, &t3);
		}
		if (result < 0) {
			printf("rotator ioctl failed result = %d\n", result);
			return result;
		} else {
			if (verbose)
				printf("RGB565 rotation passed in %f sec\n",
				       (double)t3.tv_sec +
				       (double)t3.tv_usec/1000000.0);
		}
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0)
		printf("rotator finish ioctl failed result = %d\n",
		       result);

	if (interactive && (result >= 0)) {
                result = overlay_play_rotation(fb_fd, pmem_id, &iinfo, &dinfo);
                if ( result < 0) {
                        printf("overlay play failed.\n");
                }
	}

	return result;
}

typedef int (*test_t)(int fb_fd, int pmem_id, void *in_virt, int rfd);

test_t sub_tests[] = {
	rgb565_test,
	bgr565_test,
	rgb888_test,
	ycbcr_h2v2_test,
	ycrcb_h2v1_test,
	rgb565_incr_test,
	rgb565_multi_test,
	y_cr_cb_gh2v2_test,
};

int nominal_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	int result = 0;
	unsigned int i = 0;

	if (verbose)
		printf("Nominal test\n");

	while ((result == 0) && (i < ARRAY_SIZE(sub_tests))) {
		result = sub_tests[i](fb_fd, pmem_id, in_virt, rfd);
		i++;
	}
	return result;
}

#define ADVERSARIAL_IMG_W 10
#define ADVERSARIAL_IMG_H 10
int adversarial_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	int result = 0;
	struct msm_rotator_img_info iinfo;
	struct msm_rotator_data_info dinfo;
	int i, s[MAX_SESSIONS];

	if (verbose)
		printf("Adversarial test\n");

	memset(&iinfo, 0, sizeof(iinfo));
	memset(&dinfo, 0, sizeof(dinfo));
	iinfo.src.format = MDP_RGB_565;
	iinfo.src.height = ADVERSARIAL_IMG_H;
	iinfo.src.width = ADVERSARIAL_IMG_W;
	iinfo.dst.format = MDP_RGB_565;
	iinfo.dst.height = ADVERSARIAL_IMG_H;
	iinfo.dst.width = ADVERSARIAL_IMG_W;
	iinfo.dst_x = 0;
	iinfo.dst_y = 0;
	iinfo.src_rect.x = 0;
	iinfo.src_rect.y = 0;
	iinfo.src_rect.h = ADVERSARIAL_IMG_H;
	iinfo.src_rect.w = ADVERSARIAL_IMG_W;
	iinfo.rotations = MDP_FLIP_UD;
	iinfo.enable = 1;
	iinfo.secure = 0;

	dinfo.src.memory_id = pmem_id;
	dinfo.src.flags = 0;
	dinfo.dst.flags = 0;
	dinfo.src.offset = 0;
	dinfo.dst.memory_id = pmem_id;
	dinfo.dst.offset = ADVERSARIAL_IMG_H*ADVERSARIAL_IMG_W*2;

	/* first make sure it works with good values */
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		fprintf(stderr, "%s(): start ioctl failed with good values\n",
			__func__);
		goto adversarial_exit;
	}
	dinfo.session_id = iinfo.session_id;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (result < 0) {
		fprintf(stderr, "%s(): failed with good values\n",
			__func__);
		goto adversarial_exit;
	}

	/* bad pmem id */
	dinfo.src.memory_id = pmem_id * 100;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad pmem id\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	dinfo.src.memory_id = pmem_id;

	/* bad format */
	iinfo.src.format = MDP_IMGTYPE_LIMIT2 + 100;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad format\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.src.format = MDP_RGB_565;

	/* bad dst offset */
	iinfo.dst_x = ADVERSARIAL_IMG_W + 1;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad dst_x\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.dst_x = 0;

	/* bad dst offset */
	iinfo.dst_y = ADVERSARIAL_IMG_H + 1;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad dst_y\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.dst_y = 0;

	/* bad src_rect offset */
	iinfo.src_rect.x = ADVERSARIAL_IMG_W + 1;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad src_rect.x\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.src_rect.x = 0;

	/* bad src_rect offset */
	iinfo.src_rect.y = ADVERSARIAL_IMG_H + 1;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad src_rect.y\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.src_rect.y = 0;

	/* bad src_rect size */
	iinfo.src_rect.h = ADVERSARIAL_IMG_H + 1;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad src_rect.h\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.src_rect.h = ADVERSARIAL_IMG_H;

	/* bad src_rect size */
	iinfo.src_rect.w = ADVERSARIAL_IMG_W + 1;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad src_rect.w\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.src_rect.w = ADVERSARIAL_IMG_W;

	/* bad src_rect size */
	iinfo.rotations = 0xff;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with bad rotations\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.rotations = MDP_FLIP_UD;

	/* invalid session id */
	s[0] = dinfo.session_id;
	dinfo.session_id = -1;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with invalid session id\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	dinfo.session_id = s[0];

	/* fail when session not enabled */
	iinfo.enable = 0;
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result < 0) {
		fprintf(stderr, "%s(): start ioctl failed with good values\n",
			__func__);
		goto adversarial_exit;
	}
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail when session not enabled\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}
	iinfo.enable = 1;

	/* finish this session, so we can use it again when it's not valid */
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &dinfo.session_id);
	if (result < 0) {
		printf("rotator finish ioctl failed result = %d\n",
		       result);
		goto adversarial_exit;
	}

	/* use unused session id */
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): did NOT fail with unused session id\n",
			__func__);
		result = -1;
		goto adversarial_exit;
	}

	/* grab all session ids */
	for (i = 0; i < MAX_SESSIONS; i++) {
		result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
		if (result < 0) {
			fprintf(stderr, "%s(): start ioctl failed with ",
				__func__);
			fprintf(stderr, "good values\n");
			goto adversarial_exit;
		}
		s[i] = dinfo.session_id = iinfo.session_id;
		result = ioctl(rfd, MSM_ROTATOR_IOCTL_ROTATE, &dinfo);
		if (result < 0) {
			fprintf(stderr, "%s(): failed with good values\n",
				__func__);
			goto adversarial_exit;
		}
	}

	/* this should fail since there aren't any sessions left */
	result = ioctl(rfd, MSM_ROTATOR_IOCTL_START, &iinfo);
	if (result == 0) {
		fprintf(stderr, "%s(): start ioctl did NOT fail ",
			__func__);
		fprintf(stderr, "when all sessions used\n");
		goto adversarial_exit;
	}

	/* now finish them all */
	for (i = 0; i < MAX_SESSIONS; i++) {
		result = ioctl(rfd, MSM_ROTATOR_IOCTL_FINISH, &s[i]);
		if (result < 0)
			printf("rotator finish ioctl failed result = %d\n",
			       result);
	}

adversarial_exit:
	return result;
}

int stress_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	fprintf(stderr, "%s() not implemented yet\n", __func__);
	return 0;
}

int repeat_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	int i = 0;
	int result = 0;

	while ((result == 0) && (i < 200)) {
		result = nominal_test(fb_fd, pmem_id, in_virt, rfd);
		result |= adversarial_test(fb_fd, pmem_id, in_virt, rfd);
		i++;
	}

	return result;
}

int single_test(int fb_fd, int pmem_id, void *in_virt, int rfd)
{
	if ((0 < testid) && (testid <= ARRAY_SIZE(sub_tests))) {
		if (verbose)
			printf("Single test %d\n", testid);
		return sub_tests[testid-1](fb_fd, pmem_id, in_virt, rfd);
	} else {
		fprintf(stderr, "Invalid test id\n");
		return -EINVAL;
	}
}

enum test_types {
	nominal,
	adversarial,
	stress,
	repeat,
	single,
};

test_t main_tests[] = {
	nominal_test,
	adversarial_test,
	stress_test,
	repeat_test,
	single_test
};

static int parse_args(int argc, char **argv)
{
	struct option longopts[] = {
		{ "nominal",     no_argument,       NULL, 'n'},
		{ "adversarial", no_argument,       NULL, 'a'},
		{ "stress",      no_argument,       NULL, 's'},
		{ "repeat",      no_argument,       NULL, 'r'},
		{ "tnum",        required_argument, NULL, 't'},
		{ "dumpimgs",    no_argument,       NULL, 'd'},
		{ "interactive", no_argument,       NULL, 'i'},
		{ "verbose",     no_argument,       NULL, 'v'},
		{ NULL,          0,                 NULL,  0},
	};
	int command;

	while ((command = getopt_long(argc, argv, "nasrt:div", longopts,
				      NULL)) != -1) {
		switch (command) {
		case 'n':
			test_set |= 1 << nominal;
			break;
		case 'a':
			test_set |= 1 << adversarial;
			break;
		case 's':
			test_set |= 1 << stress;
			break;
		case 'r':
			test_set |= 1 << repeat;
			break;
		case 't':
			testid = atoi(optarg);
			test_set |= 1 << single;
			break;
		case 'd':
			dumpimg = 1;
			break;
		case 'i':
			interactive = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "Invalid argument: %c\n", command);
			return -1;
		}
	}

	/* Run nominal in default testcase */
	if (!test_set)
		test_set |= 1 << nominal;

	return 0;
}

static void print_help(void)
{
  printf("=============================\n");
  printf("rotator [-nasrt:div]\n");
  printf("=============================\n\n");
  printf("	-n, --nominal		Run nominal tests\n");
  printf("	-a, --adversarial	Run adversarial tests\n");
  printf("	-s, --stress		Run stress tests\n");
  printf("	-r, --repeat		Run repeat tests\n");
  printf("	-i, --interactive       Pauses after drawing each image\n");
  printf("				Interactively queries of ");
  printf("more options\n");
  printf("	-d, --dumpimgs          Dump raw images to files before & ");
  printf("after each test\n");
  printf("	-t [num], --tnum	Run single test\n");
  printf("	-v, --verbose		Run with debug messages\n");
  printf("=============================\n");
  printf("\n");
}



int main(int argc, char **argv)
{
	int result = 0;
	uint32_t i;
	uint32_t fbSize;
	int rfd;
	int pmem_id;
	void *in_virt;
	size_t map_size = MAX_IMG_W*MAX_IMG_H*MAX_IMG_BPP*2;

	if (parse_args(argc, argv)) {
		print_help();
		return -1;
	}

	/* setup framebuffer */
	i = 0;
	while (i < NUM_FB_DEVS) {
		fb_fd = open(fb_devs[i], O_RDWR);
		if (fb_fd >= 0) {
			if (verbose)
				printf("%s opened successfully!\n",
				       fb_devs[i]);
			break;
		} else if (verbose)
			printf("warning...failed opening %s\n", fb_devs[i]);
		i++;
	}
	if (i >= NUM_FB_DEVS) {
		printf("failed to open a framebuffer device!\n");
		return -1;
	}
	result = ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	if (result < 0) {
		printf("can't retrieve vscreenInfo!\n");
		goto main_close;
	}
	result = ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
	if (result < 0) {
		printf("can't retrieve fscreenInfo!\n");
		goto main_close;
	}
	if ((LB_WIDTH > vinfo.xres) || (LB_HEIGHT > vinfo.yres)) {
		printf("display resolution too small!\n");
		result = -1;
		goto main_close;
	}
	if (verbose)
		printf("xres = %d yres = %d\n", vinfo.xres, vinfo.yres);
	vinfo.activate = FB_ACTIVATE_VBL;

	/*
         * using line_length, not xres to calculate frame buffer size
         */
        fbSize = finfo.line_length * vinfo.yres;

	fb_mmap = fb_mmap_back = mmap(NULL,
				      finfo.smem_len,
				      PROT_READ|PROT_WRITE,
				      MAP_SHARED,
				      fb_fd,
				      0);
        if (interactive) {
                memset(fb_mmap, 0, finfo.smem_len);
        }
	if (fb_mmap == MAP_FAILED) {
		printf("fb map failed!\n");
		result = -1;
		goto main_close2;
	} else if (verbose)
		printf("framebuffer mmap successful...\n");
	if (finfo.smem_len < (2*fbSize)) {
		printf("no back buffer!\n");
		result = -1;
		goto main_close2;
	}
	fb_mmap_back += fbSize;
	pmem_id = open("/dev/pmem_adsp", O_RDWR | O_SYNC);
	if (pmem_id < 0) {
		fprintf(stderr, "Could not open pmem device!\n");
		result = -1;
		goto main_close2;
	}
	in_virt = mmap(NULL, map_size, PROT_READ | PROT_WRITE,
		       MAP_SHARED, pmem_id, 0);
	if (in_virt == MAP_FAILED) {
		fprintf(stderr, "%s(): pmem mmap() failed\n", __func__);
		result = -1;
		goto main_close3;
	}
	/* open rotator */
	rfd = open("/dev/msm_rotator", O_RDWR);
	if (rfd < 0) {
		fprintf(stderr, "Could not open rotator device!\n");
		result = -1;
		goto main_close4;
	}

	for (i = 0; i < ARRAY_SIZE(main_tests); i++) {
		if (test_set & (1U << i)) {
			result = main_tests[i](fb_fd, pmem_id, in_virt, rfd);
			if (result)
				goto main_close5;
		}
	}

main_close5:
	close(rfd);
main_close4:
	munmap(in_virt, map_size);

main_close3:
	close(pmem_id);
main_close2:
	munmap(fb_mmap, finfo.smem_len);

main_close:
	close(fb_fd);

	if (result)
		printf("Tests failed\n");
	else
		printf("Tests passed\n");
	return result;
}

static int overlay_play_rotation(int fb_fd, int pmem_fd, struct msm_rotator_img_info *img, struct msm_rotator_data_info *rot ) {

        int result = 0;
        int overlay_id_org;
        int overlay_id_rot;

	struct mdp_overlay        overlay;
	struct msmfb_overlay_data ovdata_org;
	struct msmfb_overlay_data ovdata_rot;
        /*
         * Use fb back buffer to display results, so the results can
         * be seen even when android is not stopped.
         */
        memset(fb_mmap_back, 0, finfo.line_length*vinfo.yres);
        vinfo.yoffset = vinfo.yres;
        result = ioctl(fb_fd, FBIOPAN_DISPLAY, &vinfo);
        if (result < 0)
                printf("FBIOPAN_DISPLAY ioctl failed error = %d\n", result);

	overlay.src.width   = img->src.width;
	overlay.src.height  = img->src.height;
	overlay.src.format  = img->src.format;
	overlay.src_rect.x  = 0;
	overlay.src_rect.y  = 0;
	overlay.src_rect.w  = img->src.width;
	overlay.src_rect.h  = img->src.height;
	overlay.dst_rect.x  = 0;
	overlay.dst_rect.y  = 0;
	overlay.dst_rect.w  = img->src.width;
	overlay.dst_rect.h  = img->src.height;
	overlay.z_order     = 0;
	overlay.alpha       = 0;
	overlay.transp_mask = 0;
	overlay.flags       = 0;
        overlay.is_fg       = 0;
	overlay.id          = MSMFB_NEW_REQUEST;
	result              = ioctl(fb_fd, MSMFB_OVERLAY_SET, &overlay);
        if (result < 0) {
		printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n",__LINE__, result);
                return result;
	}
        overlay_id_org = overlay.id;

	overlay.src.width   = img->dst.width;
	overlay.src.height  = img->dst.height;
	overlay.src.format  = img->dst.format;

        if ( img->dst.width == img->src_rect.x ) { /* whole image roation */
                overlay.src_rect.x  = 0;
                overlay.src_rect.y  = 0;
                overlay.src_rect.w  = img->dst.width;
                overlay.src_rect.h  = img->dst.height;
                overlay.dst_rect.x  = 0;
                overlay.dst_rect.y  = img->src.height;
                overlay.dst_rect.w  = img->dst.width;
                overlay.dst_rect.h  = img->dst.height;
        }
        else {                  /* partial image rotation */

                overlay.src_rect.x  = img->src_rect.x;
                overlay.src_rect.y  = img->src_rect.y;

                /*
                 * if rotation is 90 and w and h are not equal, w and h need
                 * to be swapped.
                 */
                if ( img->rotations & MDP_ROT_90 ) {
                        overlay.src_rect.w  = img->src_rect.h;
                        overlay.src_rect.h  = img->src_rect.w;
                        overlay.dst_rect.w  = img->src_rect.h;
                        overlay.dst_rect.h  = img->src_rect.w;
                }
                else {
                        overlay.src_rect.w  = img->src_rect.w;
                        overlay.src_rect.h  = img->src_rect.h;
                        overlay.dst_rect.w  = img->src_rect.w;
                        overlay.dst_rect.h  = img->src_rect.h;
                }

                overlay.dst_rect.x  = img->src_rect.x;
                overlay.dst_rect.y  = img->src_rect.y + img->src.height;

        }
	overlay.z_order     = 1;
	overlay.alpha       = 0;
	overlay.transp_mask = 0;
	overlay.flags       = 0;
        overlay.is_fg       = 0;
	overlay.id          = MSMFB_NEW_REQUEST;
	result              = ioctl(fb_fd, MSMFB_OVERLAY_SET, &overlay);
        if (result < 0) {
		printf("ERROR: MSMFB_OVERLAY_SET failed! line=%d err=%d\n",__LINE__, result);
                return result;
	}
        overlay_id_rot = overlay.id;

	ovdata_org.id             = overlay_id_org;
	ovdata_org.data.flags     = 0;
	ovdata_org.data.offset    = 0;
	ovdata_org.data.memory_id = pmem_fd;
        result                    = ioctl(fb_fd, MSMFB_OVERLAY_PLAY, &ovdata_org);
	if (result < 0) {
		printf("ERROR: MSMFB_OVERLAY_PLAY failed! Line=%d err=%d\n", __LINE__, result);
                return result;
	}

	ovdata_rot.id             = overlay_id_rot;
	ovdata_rot.data.flags     = 0;
	ovdata_rot.data.offset    = rot->dst.offset;
	ovdata_rot.data.memory_id = pmem_fd;
        result                    = ioctl(fb_fd, MSMFB_OVERLAY_PLAY, &ovdata_rot);
	if (result < 0) {
		printf("ERROR: MSMFB_OVERLAY_PLAY failed! Line=%d err=%d\n", __LINE__, result);
                return result;
	}

        printf("Press Enter to continue...\n");
        getchar();

        result = ioctl(fb_fd, MSMFB_OVERLAY_UNSET, &ovdata_org);
	if ( result < 0 ) {
		printf("ERROR: MSMFB_OVERLAY_UNSET failed! Line %d err=%d\n", __LINE__, result);
                return result;
	}

        result = ioctl(fb_fd, MSMFB_OVERLAY_UNSET, &ovdata_rot);
	if ( result < 0 ) {
		printf("ERROR: MSMFB_OVERLAY_UNSET failed! Line %d err=%d\n", __LINE__, result);
                return result;
	}

        return result;
}
