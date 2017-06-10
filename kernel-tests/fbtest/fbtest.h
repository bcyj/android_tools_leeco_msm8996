/******************************************************************************
  @file  fbtest.h
  @brief This file contains test code to verify all functionalities of msm_fb

  DESCRIPTION
  fbtest is msm framebuffer test program.  It opens all frambuffers (/dev/fb*)
  and executes the msm specific fb ioctls as well as the standard linux fb
  ioctls.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

 -----------------------------------------------------------------------------
 Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
 -----------------------------------------------------------------------------

******************************************************************************/
#ifndef FBTEST_H
#define FBTEST_H
#ifdef USE_GLIB
#include "stringl.h"
#else
#include <string.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <errno.h>
#include <linux/fs.h>
#include <math.h>
#include <time.h>

//UTF: Below header file are added for fbtest enhancement.
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <signal.h> 	//UTF: addded for cleanup code addition.
#include "linux/msm_rotator.h" //UTF: addded for cleanup code addition.
#ifdef USE_ION
#include "linux/msm_ion.h"
#endif
//UTF: Above header file are added for fbtest enhancement.

/* Use GLIB for g_strlcpy and strlCat*/
#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif

#define MDP_V_UNKNOWN 0
#define MDP_V2_2 220
#define MDP_V3_0 300
#define MDP_V3_0_3 303
#define MDP_V3_1 310
#define MDP_V4_0 400
#define MDP_V4_1 410
#define MDP_V4_2 420
#define MDP_V4_3 430
#define MDP_V4_4 440
#define MDP_V5 500

/* MDP5 HW version */
#define MDSS_MDP_HW_REV_100             0x10000000
#define MDSS_MDP_HW_REV_102             0x10020000
#define MDSS_MDP_HW_REV_103             0x10030000

//Panel values directly reflect definitions in msm_fb_panel.h
#define MDDI_PANEL     '1'
#define EBI2_PANEL     '2'
#define LCDC_PANEL     '3'
#define EXT_MDDI_PANEL '4'
#define TV_PANEL       '5'
#define DTV_PANEL      '7'
#define MIPI_VIDEO_PANEL '8'
#define MIPI_CMD_PANEL  '9'
#define WRITEBACK_PANEL  'a'
#define LVDS_PANEL  'b'
#define EDP_PANEL   'c'

#define PANEL_UNKNOWN 0
#define PANEL_PRIMARY_LCD 1
#define PANEL_SECONDARY_LCD 2
#define PANEL_TVOUT 3

/****************************************************************/
// Global Config
/***************************************************************/

#define MSM_STR_SIZE 8 			// msmfbxx = 9 chars + 1 null

#define FB_NAME_STR_LE "/dev/fb0"
#define FB_NAME_STR_LA "/dev/graphics/fb0"
#define FB_NAME_STR_LEN sizeof(FB_NAME_STR_LA)
#define FB_NUM_MAX 4
#define FB_NUM_PANEL 0
#define FB_NUM_TVOUT 1
#define FB_NUM_DEFAULT FB_NUM_PANEL
#define FB_FMT_PANEL MDP_RGB_565
#define FB_FMT_DEFAULT MDP_FB_FORMAT
#define FB_FMT_TVOUT MDP_RGB_565
#define FB_OVERLAY_NUM_MAX 4
#define FB_OVERLAY_RGB_0 0
#define FB_OVERLAY_RGB_1 0
#define FB_OVERLAY_VID_0 0
#define FB_OVERLAY_VID_1 2

#define MEM_NAME_STR_LEN  (25)
#define MEM_NUM_DEFAULT 0
#ifdef USE_ION
#define ION_NAME_STR "/dev/ion"
#define ION_NAME_STR_LEN sizeof(ION_NAME_STR)
#define ION_NUM_MAX 1
#define ION_NUM_DEFAULT 0
#else
#define PMEM_NAME_STR "/dev/pmem_adsp"
#define PMEM_NAME_STR2 "/dev/pmem"
#define PMEM_NAME_STR_LEN sizeof(PMEM_NAME_STR)
#define PMEM_NUM_MAX 4
#define PMEM_NUM_DEFAULT 0
#define PMEM_ALLOCATE _IOW('p', 5, unsigned int)
#endif

#define TEST_RESULT_PASS 0
#define TEST_RESULT_SKIP 1
#define TEST_RESULT_FAIL -1

#define TEST_LEVEL_DEFAULT 0
#define TEST_LEVEL_MAX 4
#define TEST_QUICKMODE_DEFAULT 0
#define TEST_NAME_MAXLEN 50
#define TEST_LIST_MAXLEN 30

#define TEST_DELAY_SHORT 500000
#define TEST_DELAY_MEDIUM 2000000
#define TEST_DELAY_LONG 5000000

#define FILE_NAME_STR_LEN_MAX 30

//UTF: Below define is added for fbtest enhancement.
#define VERSION    "fbtest v1.0"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define VPRINT(a, ...) a ? printf( __VA_ARGS__) : 0;
//UTF: Above define is added for fbtest enhancement.
#define FB_CONCURRENT_NUM 2
#define TRUE 1
#define FALSE 0
/*****************************************************/
// Structures and Globals
/*****************************************************/
// MDP
int mdp_version;
unsigned int mdp_rev;
int panel_type;
int writeback_flag;
int testflag;//for multi-opt test check in mixer-2 case

struct fbDev * FB;

struct memDev * MEM;
#ifdef USE_ION
struct memDev * ION;
#else
struct memDev * PMEM;
#endif
struct fbDev *FBLIST_GLOBAL[FB_CONCURRENT_NUM];
int FBLISTSIZE_GLOBAL;
// Framebuffers
struct fbDev {
	struct fb_var_screeninfo fb_vinfo;
	struct fb_fix_screeninfo fb_finfo;
	int fb_fd;
	int fb_num;
	char fb_name[FB_NAME_STR_LEN];
	char fb_msm_str[MSM_STR_SIZE];
	int fb_fmt;
	unsigned int fb_size;
	unsigned char *fb_buf;
	unsigned char *fb_buf_back;

} fb[FB_NUM_MAX];

extern struct fbDev * FB;

struct memDev {
	int fd;
	int mem_fd;
	char mem_name[MEM_NAME_STR_LEN];
	int mem_page_size;
	int mem_size;
	unsigned char *mem_buf;
#ifdef MDP3_FLAG
	int mem_fd_src;
	int mem_size_src;
	unsigned char *mem_buf_src;
#endif
};

extern struct memDev * MEM;
#ifdef USE_ION
struct memDev ion[ION_NUM_MAX];
extern struct memDev * ION;
#else
struct memDev pmem[PMEM_NUM_MAX];
extern struct memDev * PMEM;
#endif

enum VERBOSE {
	DISABLED,
	ENABLED,
};
enum FRAMEBUFFER {
	FB0,
	FB1,
	FB2,
	FB01 = FB_NUM_MAX,
};
enum MODE {
	USER,
	AUTO,
};
enum PATH{
	DIRPATH,
	REGFILE,
	OTHER,
};

/*******************************************************/
// Function Prototypes
/******************************************************/
// Helper Functions
int read_int(void);
void swapPointers(unsigned char **, unsigned char **);
// Framebuffer functions
int chooseFBDev(int);
int chooseFBFmt(int, struct fbDev *, int);
int openFBDev(struct fbDev *);
int getFBInfo(struct fbDev *);
int getMDPInfo(struct fbDev *);
int getPanelInfo(struct fbDev *);
int mapFB(struct fbDev *);
int closeFBDev(struct fbDev *);
void reset_fbtest(void);

// MEM functions
int chooseMEMDev(void);
int openMEMDev(void);

typedef int (*tests)(void);

#endif
