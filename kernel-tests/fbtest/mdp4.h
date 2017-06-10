/******************************************************************************
  @file  mdp4.h
  @brief This file contains test code to verify all functionalities of msm_fb

  DESCRIPTION
  fbtest is msm framebuffer test program.  It opens all frambuffers (/dev/fb*)
  and executes the msm specific fb ioctls as well as the standard linux fb
  ioctls.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

 -----------------------------------------------------------------------------
 Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
 -----------------------------------------------------------------------------

******************************************************************************/
#ifndef MDP4_H
#define MDP4_H
#include "cursor.zif"
#include "yellow_face.zif"

#ifdef VENUS_COLOR_FORMAT
#include "media/msm_media_info.h"
#define WRITEBACK_SIZE VENUS_BUFFER_SIZE(COLOR_FMT_NV12, 1920, 1080)//mdss buf req
#define WRITEBACK_WIDTH(w) VENUS_Y_STRIDE(COLOR_FMT_NV12, w)
#define WRITEBACK_HEIGHT(h) VENUS_Y_SCANLINES(COLOR_FMT_NV12, h)
#else
#define WRITEBACK_SIZE (1280*720*1.5)//width*height*BPP
#define WRITEBACK_WIDTH(w) 1280
#define WRITEBACK_HEIGHT(h) 720
#endif

////UTF: Below define is added for fbtest enhancement.
#define MAX_YUV_PIPE_SUPPORT (2)
#define MAX_FILE_TO_BLEND (MAX_MDP4_OVERLAY_PIPE)
//#define MAX_FILE_TO_BLEND (4) //temporary define is in 8660 during driver initialization 2 pipes
				//get allocated. Will use the above define once issue is resolved.
//UTF: Above define is added for fbtest enhancement.

#define SQUARE_WIDTH 30
#define SQUARE_HEIGHT 30
#define FACE_WIDTH 128
#define FACE_HEIGHT 128
#define FACE_FORMAT MDP_RGB_565
#define INVALID_OVERLAY -1
#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

enum MDP4_OVERLAY_PIPES{
	BG_PIPE,
	FG1_PIPE,
	FG2_PIPE,
	FG3_PIPE,
	MAX_MDP4_OVERLAY_PIPE,
};
struct blend_thread_data {
    struct fbDev* ptr_fb;
    int* ptr_result;
    int src_min_width;
    int src_min_height;
    int src_width_increment;
    int src_height_increment;
    uint32_t overlay_id;
    uint32_t offset;
    uint32_t flags;
    int fcount;
};
struct scale_thread_data {
    struct fbDev* ptr_fb;
    int* ptr_result;
    float max_scaledown;
    float max_scaleup;
    float scale_increment;
    uint32_t src_min_width;
    uint32_t src_min_height;
    uint32_t offset;
    uint32_t overlay_id;
    uint32_t flags;
    int fcount;
};

/* UTF: Below functions added for fbtest enhancement. */
int is_valid_overlay(struct mdp_overlay *overlay);
int writeback_init_start(int framesize);
int writeback_queueBuff(struct msmfb_data *fbdata, int offset);
int writeback_dqueue_dump(struct msmfb_data *fbdata, int offset, char *testname);
int writeback_stop_terminate(struct msmfb_data *fbdata);
int docursor(struct fb_image *cursor_image, int i);
int displayBG();
int allMDP4Test(void);
int scaleTest(void);
int doScale(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
int rotateTest(void);
int doRotate(unsigned int, unsigned int);
int moveTest(void);
int doImageMove(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
int traverseImage(unsigned int startX, unsigned int startY, unsigned int endX, unsigned int endY, struct mdp_overlay*, struct msmfb_data *fbdata);
int blendTest(void);
int colorkeyTest(void);
int doImageBlend(struct inputFileParams *, unsigned int, int);
int formatTest(void);
int deInterlaceTest(void);
int ditherTest(void);
int multiopTest(void);
int doCrop(void);
int cropCalculation(void);
int cropTest(void);
int videoPlayTest(void);
int fpsVsyncTest(void);
int openRotator(void);
int MDP4adversarialTest(void);
int hwCursorTest(void);
int updateCursorPos(struct fb_cursor *cursor_control, unsigned int col, unsigned int row);
int CSCTest(void);
void *doScaleptr(void* arg);
int overlayScaleTest(void);
static void *doAlphaBlend(void *arg);
int overlayAlphaTest(void);
int unsetOverlay(struct fbDev* ptr_fb, uint32_t* overlay_id);
int playOverlay(struct msmfb_overlay_data* ptr_ovdata,struct fbDev* ptr_fb, int overlay_id,uint32_t flags, uint32_t offset, int memory_id);
int setOverlay(struct mdp_overlay* ptr_overlay, struct fbDev* ptr_fb,uint32_t format, int32_t src_w, int32_t src_h,int32_t src_x, int32_t src_y, int32_t dst_w,\
    int32_t dst_h, int32_t dst_x, int32_t dst_y,int32_t z_order, uint32_t transp_mask, uint32_t flags,int32_t id, uint32_t offset);
uint32_t fibonacci(uint32_t n);
int drawBGAlpha(struct fbDev* ptr_fb, int color);
static int overlayStressTest(void);
static int allcolorFormatOverlayTest(void);
static int overlayARGBTest(void);
static int overlayTest(void);
static int postprocTest(void);
static int overlayPPTest(void);
int getMaxUpScale(int  mdp_version);
int getMaxDownScale(int  mdp_version);
int getHistogram(unsigned int block);
long int get_histogram_crc(struct mdp_histogram_data *data, int extra_bins);
int doDisplayUpdate(void);
int doPanDisplay(void);
void add_delay_in_user_mode();
int setup_borderfill_pipe();
#endif
