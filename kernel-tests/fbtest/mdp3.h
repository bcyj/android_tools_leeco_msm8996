/******************************************************************************
  @file  mdp3.h
  @brief This file contains test code to verify all functionalities of msm_fb

  DESCRIPTION
  fbtest is msm framebuffer test program.  It opens all frambuffers (/dev/fb*)
  and executes the msm specific fb ioctls as well as the standard linux fb
  ioctls.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

 -----------------------------------------------------------------------------
 Copyright (c) 2011, 2013 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
 -----------------------------------------------------------------------------

******************************************************************************/
#ifndef MDP3_H
#define MDP3_H

#define FACE_WIDTH 128
#define FACE_HEIGHT 128

#define SQUARE_WIDTH 30
#define SQUARE_HEIGHT 30
#define INVALID_OVERLAY -1

#if 1
#define FACE_TRANSPMASK_RGB565 0xdc5e
#else
#define FACE_TRANSPMASK_RGB565 0x00
#endif
#define FACE_FORMAT MDP_RGB_565

#define YUV_WIDTH 176
#define YUV_HEIGHT 144
#define YUV_TRANSPMASK 0xffffff
#define YUV_FORMAT MDP_Y_CBCR_H2V2

#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

// Tests

//UTF: Below functions added for fbtest enhancement.
double rand_val(int seed);
double expon(double x);
int poisson(double x);
void get_rand4(int *pois_rv, int rv_max1, int rv_max2, int seed);

int allMDP3Test(void);
int drawBG(void);
int fpsVsyncTest(void);
int calcFPS(void);
int pppTest(void);
int videoPlayTest(void);
int pppTestVideo(void);
int adversarialTest(void);
int allcolorFormatPPPTest(void);
int stressTest(void);
int mddi_partial_update_test(void);
int rotateTest(void);
int scaleTest(void);
int formatTest(void);
int blendTest(void);
int cropTest(void);
int multiopTest(void);
int moveTest(void);
int ditherTest(void);
int overlayTest(void);
int colorkeyTest(void);
int CSCTest(void);
void add_delay_in_user_mode();
static int postprocTest(void);
int getMaxUpScale(int  mdp_version);
int getMaxDownScale(int  mdp_version);
#endif
