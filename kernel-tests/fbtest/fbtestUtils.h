/******************************************************************************
  @file  fbtestUtils.h
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
#ifndef FBTESTUTILS_H
#define FBTESTUTILS_H

#include <linux/fb.h>
#include <linux/msm_mdp.h>
#ifdef ENABLE_POSTPROC
#include "lib-postproc.h"
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef MAX
#define  MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define  MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#define FBTEST_MSG_DEBUG(msg, ...)\
	if (fbtest_msg_debug > 0)\
		printf(msg, ## __VA_ARGS__)

#define MAX_TESTS 35
#define MAX_FILE_PATH 256
#define MAX_FB_NAME 8

/****************************************************************/
// Global Constants
/****************************************************************/
#define COLOR_BLACK_RGB_888 0x000000
#define COLOR_BLACK_RGB_565 0x0000
#define COLOR_BLACK_YCBYCR_H2V1 0x80108010

#define COLOR_WHITE_RGB_888 0xFFFFFF
#define COLOR_WHITE_RGB_565 0xFFFF
#define COLOR_WHITE_YCBYCR_H2V1 0x80EB80EB

#define COLOR_GREY_RGB_888 0x888888
#define COLOR_GREY_RGB_565 0x8410
#define COLOR_GREY_YCBYCR_H2V1 0x807E807E

#define COLOR_YELLOW_RGB_565	0xFFE0		/* bit pattern RGB */
#define COLOR_YELLOW_BGR_565	0x07FF		/* bit pattern BGR */
#define COLOR_YELLOW_RGB_888	0xFFFF00	/* bit pattern RGB */
#define COLOR_YELLOW_BGR_888	0x00FFFF	/* bit pattern RGB */
#define COLOR_YELLOW_XRGB_8888	0xFFFF0000	/* bit pattern ARGB */
#define COLOR_YELLOW_ARGB_8888	0xFFFF0000	/* bit pattern ARGB */
#define COLOR_YELLOW_BGRA_8888	0x00FFFF00	/* bit pattern ARGB */
#define COLOR_YELLOW_RGBA_8888	0x0000FFFF	/* bit pattern ABGR */
#define COLOR_YELLOW_RGBX_8888	0x0000FFFF	/* bit pattern ABGR */

#define COLOR_WHITE_XRGB_8888	0x00FFFFFF	/* bit pattern ARGB */
#define COLOR_WHITE_ARGB_8888	0x00FFFFFF	/* bit pattern ARGB */
#define COLOR_WHITE_BGRA_8888	0x00FFFFFF	/* bit pattern ARGB */
#define COLOR_WHITE_RGBA_8888	0x00FFFFFF	/* bit pattern ABGR */
#define COLOR_WHITE_RGBX_8888	0x00FFFFFF	/* bit pattern ABGR */

#define COLOR_BLACK_XRGB_8888	0x00000000	/* bit pattern BGRA */
#define COLOR_BLACK_ARGB_8888	0x00000000	/* bit pattern ARGB */
#define COLOR_BLACK_BGRA_8888	0x00000000	/* bit pattern ARGB */
#define COLOR_BLACK_RGBA_8888	0x00000000	/* bit pattern ABGR */
#define COLOR_BLACK_RGBX_8888	0x00000000	/* bit pattern ABGR */

/* modify these values to get yellow */
#define COLOR_YELLOW_YCBYCR_H2V1 0x807E807E
#define COLOR_YELLOW_CBCR		0x7E7E /*bit pattern CbCr, Cb in MSB*/
#define COLOR_YELLOW_CRCB		0x7E7E /*bit pattern CbCr, Cr in MSB*/
#define COLOR_YELLOW_Y		0x80
#define COLOR_YELLOW_CB		0x7E
#define COLOR_YELLOW_CR		0x7E

#define DEFAULT_ROTATE_FLIP (0x00)
#define MOVE_SKIP_INTERVAL (50)
#define ALIGN(x, align) (((x) + ((align)-1)) & ~((align)-1))
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

enum COLOR {BLACK, WHITE, GREY, YELLOW,BLEND};

//UTF: Below declaratins added for fbtest enhancement.
enum errorValues {
	NO_ERROR,
	FBDEV_NOT_FOUND,
	PMEMDEV_NOT_FOUND,
	FBDEV_OPEN_FAILED,
	PMEMDEV_OPEN_FAILED,
	IONDEV_OPEN_FAILED,
	VSCREEN_GET_INFO_FAILED,
	FSCREEN_GET_INFO_FAILED,
	MMAP_FAILED,
	INVALID_FORMAT,
	INVALID_PATH,
	PARSE_FAILED,
	DIROPEN_FAILED,
	DIRCLOSE_FAILED,
	INVALID_PARAMS,
	FOPEN_FAILED,
	FILE_NOT_FOUND,
	INCORRECT_FILE_SIZE,
	PMEMALLOC_FAILED,
	MALLOC_FAILED,
	SYSTEM_CMD_FAILED,
	RES_OUT_OF_BOUND,
	FBIOPAN_DISPLAY_FAILED,
	OVERLAY_SET_FAILED,
	OVERLAY_PLAY_FAILED,
	OVERLAY_UNSET_FAILED,
	INAVALID_CROP_PARAMS,
	INVALID_COLORKEY_IMAGE_FORMAT,
	ROT_OPEN_FAILED,
	ROT_START_FAILED,
	ROT_FAILED,
	ROT_FINISH_FAILED,
	ROT_HxW_OUT_OF_BOUND_0_180,
	ROT_HxW_OUT_OF_BOUND_90_270,
	OFFSET_OUT_OF_BOUND,
	INVALID_OFFSET,
	INVALID_BLEND_BG_FORMAT,
	MAX_YUV_SUPPORT_REACHED,
	FREAD_FAILED,
	MSMFB_CURSOR_FAILED,
	MSMFB_DISPLAY_COMMIT_FAILED,
	BLIT_FAILED,
	INVALID_OVERLAY,
	CRC_ENTRY_NOT_FOUND,
	CRC_ENTRY_NOT_MATCHING,
	CRC_DB_NOT_FOUND,
	CRC_ENTRY_FOUND,
	CRC_ENTRY_ADD,
};

enum successValues {
	VALID_OVERLAY = 0,
	OVERLAY_UNSET_PASS = 0,
	OVERLAY_SET_PASS = 0,
	BLIT_PASS = 0,
	MSMFB_DISPLAY_COMMIT_PASS = 0,
	TEST_ALIGNMENT_CKECK_PASS = 0,
};
//UTF: Above define is added for fbtest enhancement.
enum SCALING {
	UPSCALE,
	DOWNSCALE,
	ARBITRARY,
};
enum ROTATE_DEGREE {
	ROT_0,
	ROT_90,
	ROT_180,
	ROT_270,
};
enum ROTATE_FLIP {
	FLIP_NOP,
	FLIP_LR,
	FLIP_UD,
	FLIP_LRUD,
};
enum CROPPING {
	P_25,
	P_50,
	P_75,
	C_BOTTOM,
	C_TOP,
	C_LEFT,
	C_RIGHT,
	C_RAND,//for random cropping  with user specified starting cordinate(x,y) & width-height
};
enum IMAGE_TRAVERSAL {
	USER_DEFINED,
	HORIZONTAL,
	VERTICAL,
        BOUNDARY,
	DIAGONALS,
	RANDOM,
};

enum brightness_type {
    BRIGHTNESS_STORE_NEW_LEVEL,
    BRIGHTNESS_RESTORE_PREV_LEVEL
};

struct blendParams{
	char ImagePath[4][256];
	int ImgAlphaVal[4];
	unsigned int isPixBlend;
	unsigned int ispremul;
};
struct Video {
	char name[FILE_NAME_STR_LEN_MAX];
	int width;
	int height;
	int format;
	int frames;
	int fps;
};
struct imageOffset
{
	int startX;
	int startY;
	int endX;
	int endY;
};
struct resolution
{
	unsigned int width;
	unsigned int height;
};
struct coordinate
{
	int x;
	int y;
};
struct inputFileParams
{
	char filenamePath[256];
	struct resolution inResolution;
	int inputFormat;//Input color format
	char charFormat[25];//user readable form of input format
	char filename[50];//filename of input file used for naming output dump file
};
struct cropParams //cropping parameters struct
{
	struct coordinate src_rect;
	struct coordinate dst_rect;
	struct resolution resolution;
};

struct fbtest_pp_data {
	int postproc_use_file;
	char postproc_file[256];
};

struct fbtest_params
{
	int fBuffer;
	int testMode;
	unsigned int inputFileCount;
	struct inputFileParams fileParams;
	struct resolution outResolution;
	struct coordinate coordinate;
	struct resolution cropresolution;
	struct imageOffset imgOffset;
	struct blendParams blend;
	struct fbtest_pp_data pp_data;
	int isStepScale;
	int rot_fd;
	int rotateDegree;
	int rotateFlip;
	int crop;
	char path[256];
	char dumpPath[50];
	int fps;/*fps for video file */
	int frameNum;/*Frame Number of video file */
	int freePipes;
	unsigned int transparency;/* transparency for color key */
	unsigned int iteration;/* iteration value for fps test */
	int moveType;
	char *testname;
	int crcmode;
	int misr;
	char crcPath[256];
	int decimation;
};

typedef struct lookup_string_t {
	char const *string;
	int key;
	int val;
} lookup_string_t;

#ifdef USE_ION
struct ion_handle_data handle_data;
#ifdef MDP3_FLAG
struct ion_handle_data handle_data_src;
#endif
#endif
extern lookup_string_t format_name_table[];
unsigned char prev_brightness, curr_brightness;

struct fbtest_params FBTEST;
struct cropParams CROPTEST;
unsigned int testEnable;
unsigned int verbose;
unsigned int interactiveSleep;
unsigned int interactive;
unsigned int async;
unsigned int dump;
unsigned int deInterlaceFlag;
unsigned int ditherFlag;
unsigned int colorkeyflag;
unsigned int multiopflag;
int curr_overlay_id[4]; /* UTF: added for cleanup code addition. */
/* UTF: Above declaratins added for fbtest enhancement. */


/* Helper Functions */
int allocMEM(unsigned int);
void memset16(void *, uint16_t, int);
void memset24(void *, uint32_t, int);
void memset32(void *, uint32_t, int);
float getFormatBpp(int);
int lookup_key(lookup_string_t const *ptable, char const *string, int default_value);
char const *lookup_string(lookup_string_t const *ptable, int key, char const *default_value);
int lookup_key_case_insensitive( lookup_string_t const * ptable, char const *string, int default_result);
int lookup_val( lookup_string_t const *ptable, int key, int default_result);
int brightness_change(enum brightness_type type);
void showError(int, int);
int paintBuffer(unsigned char *, int, int,
			unsigned int, unsigned int, unsigned int);
int drawBG(void);
int getFrameSize(int width,int height,int format);
void showUsage(void);
int parse_opt(int, char **);
int parseFileName(const char *file, struct inputFileParams *thisInputFileParams);
void parse_config_file(char config_path[MAX_FILE_PATH], int *ptr_num_line, char *argv_array[][MAX_FILE_PATH], int *argc_array);
int rand_range(int, int);
int gcd(int, int);
int cleanup_overlay(void);		/* UTF: added for cleanup code addition. */
void close_devices(void);		/* UTF: added for cleanup code addition. */
void signal_callback_handler(int);	/* UTF: added for cleanup code addition. */
void turnScreenOn(void);
void *keepScreenOn(void *);
void get_rand4(int *pois_rv, int rv_max1, int rv_max2, int seed);
int poisson(double x);
double expon(double x);
double rand_val(int seed);
void dump_img(unsigned char *buf, int size, const char *filename);
void date_timestamp_string(char *name);
int setOverlay(struct mdp_overlay* ptr_overlay, struct fbDev* ptr_fb,
                        uint32_t format, int32_t src_w, int32_t src_h,
                        int32_t src_x, int32_t src_y, int32_t dst_w,
                        int32_t dst_h, int32_t dst_x, int32_t dst_y,
                        int32_t z_order, uint32_t transp_mask, uint32_t flags,
						int32_t id, uint32_t offset);
int is_valid_overlay (struct mdp_overlay *overlay);
int unsetOverlay(struct fbDev* ptr_fb, uint32_t* overlay_id);
int getMaxScale(int  mdp_version);
int doCommit(struct fbDev* hdl, struct mdp_display_commit* commit_info);
int doPlay(struct fbDev* hdl, struct msmfb_overlay_data* ptr_ovdata);
int set_brightness(void);
int crcGen(long int);
int crcValidation(long int);
int powerCycleDisplay(void);
int misr(int misrid);
int postProcCRC(void);
int getMisr (int);
int setMisr (int);
int crcGenerate(long int crc);
int crcValidate(long int crc);
long int get_histogram_crc(struct mdp_histogram_data *data, int extra_bins);
void showCRCError(int errorno, char *crcfb_file);
#endif
