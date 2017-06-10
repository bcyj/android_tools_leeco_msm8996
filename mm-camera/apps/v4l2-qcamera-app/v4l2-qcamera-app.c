/* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#ifdef _ANDROID_
#include <cutils/log.h>
#endif
#include <dlfcn.h>

#include "camera.h"
#include "cam_mmap.h"
#include "v4l2_cam.h"
#include "camaf_ctrl.h"
#include "camera_dbg.h"
#include "../test/display_dimensions.h"
#include "camera_defs_i.h"

#define VIDEO_BUFFER_SIZE       (PREVIEW_WIDTH * PREVIEW_HEIGHT * 3/2)
#define THUMBNAIL_BUFFER_SIZE   (THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 3/2)
#define SNAPSHOT_BUFFER_SIZE    (PICTURE_WIDTH * PICTURE_HEIGHT * 3/2)

/*===========================================================================
 * Macro
 *===========================================================================*/
#define PREVIEW_FRAMES_NUM    4
#define VIDEO_FRAMES_NUM      4
#define THUMBNAIL_FRAMES_NUM  1
#define SNAPSHOT_FRAMES_NUM   1
#define MAX_THUMBNAIL_FRAMES_NUM  3
#define MAX_SNAPSHOT_FRAMES_NUM   3
#define RAW_FRAMES_NUM        1
#define MAX_NUM_FORMAT        32
#define BUFF_SIZE_64          64
/*===========================================================================
 * Defines
 *===========================================================================*/
typedef struct{
    Camera_main_menu_t main_menu;
    char * menu_name;
} CAMERA_MAIN_MENU_TBL_T;

const CAMERA_MAIN_MENU_TBL_T camera_main_menu_tbl[] = {
  {STOP_CAMERA,                   "Stop preview/video and exit camera."},
  {PREVIEW_VIDEO_RESOLUTION,      "Preview/Video Resolution: SQCIF/QCIF/"
                              "QVGA/CIF/VGA/WVGA... Default WVGA."},
  {SET_WHITE_BALANCE,          "Set white balance mode: Auto/Off/Daylight/Incandescent/Fluorescent. Default Auto."},
  {SET_EXP_METERING,          "Set exposure metering mode: FrameAverage/CenterWeighted/SpotMetering. Default CenterWeighted"},
  {GET_CTRL_VALUE,              "Get control value menu"},
  {TOGGLE_AFR,                 "Toggle auto frame rate. Default fixed frame rate"},
  {SET_ISO,                 "ISO changes."},
  {BRIGHTNESS_GOTO_SUBMENU,                               "Brightness changes."},
  {CONTRAST_GOTO_SUBMENU,                                 "Contrast changes."},
  {EV_GOTO_SUBMENU,                                       "EV changes."},
  {SATURATION_GOTO_SUBMENU,                               "Saturation changes."},
  {SET_ZOOM,          "Set Digital Zoom."},
  {SET_SHARPNESS,          "Set Sharpness."},
  {TAKE_YUV_SNAPSHOT,       "Take a YUV 420 snapshot"},
  {ANTI_BANDING,      "Set Anti Flicker in sequence: Off/50Hz/60Hz. Default Off."},
  {SPECIAL_EFFECT,                     "Toggle special effect mode. Default Off."},
  {SET_HJR,                                                        "HJR changes:"},
  {LUMA_ENABLE_DISABLE,                                    "LUMA Enable/Disable."},
  {AUTO_FOCUS,      "Set auto focus"},
  {BEST_SHOT,       "Set best-shot mode"},
  {BL_SNOW,         "Set backlight/snow detection mode"},
  {TAKE_RAW_SNAPSHOT,        "Take Raw snapshot"},
  {TOGGLE_STROBE_FLASH_MODE,      "Toggle strobe flash mode OFF/ON... Default OFF."},
  {SET_LED_MODE,                  "Toggle LED mode(Off/Auto/On/Torch). Default Off."},
  {TAKE_422_SNAPSHOT,             "Take a snapshot in YUV422 format."},
  {TAKE_INLINE_SNAPSHOT,          "Take Inline Snapshot."},
  {SET_HDR_GOTO_SUBMENU,                  "Set HDR mode."},
};

typedef struct{
    Camera_Resolution cs_id;
    uint16_t width;
    uint16_t  height;
    char * name;
    char * str_name;
} PREVIEW_DIMENSION_TBL_T;

typedef struct {
  White_Balance_modes wb_id;
  char * wb_name;
} WHITE_BALANCE_TBL_T;

typedef struct {
  Get_Ctrl_modes get_ctrl_id;
  char * get_ctrl_name;
} GET_CTRL_TBL_T;

typedef struct{
  Exp_Metering_modes exp_metering_id;
  char * exp_metering_name;
} EXP_METERING_TBL_T;

typedef struct {
  ISO_modes iso_modes;
  char *iso_modes_name;
} ISO_TBL_T;

typedef struct {
  Zoom_direction zoom_direction;
  char * zoom_direction_name;
} ZOOM_TBL_T;

typedef struct {
  Camera_Sharpness_changes sharpness_change;
  char *sharpness_change_name;
} SHARPNESS_TBL_T;

const PREVIEW_DIMENSION_TBL_T preview_video_dimension_tbl[] = {
   { SQCIF, SQCIF_WIDTH, SQCIF_HEIGHT, "SQCIF",  "Preview/Video Resolution: SQCIF <128x96>"},
   {  QCIF,  QCIF_WIDTH,  QCIF_HEIGHT,  "QCIF",  "Preview/Video Resolution: QCIF <176x144>"},
   {  QVGA,  QVGA_WIDTH,  QVGA_HEIGHT,  "QVGA",  "Preview/Video Resolution: QVGA <320x240>"},
   {   CIF,   CIF_WIDTH,   CIF_HEIGHT,   "CIF",  "Preview/Video Resolution: CIF <352x288>"},
   {   VGA,   VGA_WIDTH,   VGA_HEIGHT,   "VGA",  "Preview/Video Resolution: VGA <640x480>"},
   {  WVGA,  WVGA_WIDTH,  WVGA_HEIGHT,  "WVGA",  "Preview/Video Resolution: WVGA <800x480>"},
   {  SVGA,  SVGA_WIDTH,  SVGA_HEIGHT,  "SVGA",  "Preview/Video Resolution: SVGA <800x600>"},
   {   XGA,   XGA_WIDTH,   XGA_HEIGHT,    "XGA", "Preview/Video Resolution: XGA <1024x768>"},
   { HD720, HD720_WIDTH, HD720_HEIGHT,  "HD720", "Preview/Video Resolution: HD720 <1280x720>"},
};

typedef struct {
  Camera_Brightness_changes bc_id;
  char * brightness_name;
} CAMERA_BRIGHTNESS_TBL_T;

const CAMERA_BRIGHTNESS_TBL_T brightness_change_tbl[] = {
  {INC_BRIGHTNESS, "Increase Brightness by one step."},
  {DEC_BRIGHTNESS, "Decrease Brightness by one step."},
};

typedef struct {
  Camera_Contrast_changes cc_id;
  char * contrast_name;
} CAMERA_CONTRST_TBL_T;

const CAMERA_CONTRST_TBL_T contrast_change_tbl[] = {
  {INC_CONTRAST, "Increase Contrast by one step."},
  {DEC_CONTRAST, "Decrease Contrast by one step."},
};

typedef struct {
  Camera_EV_changes ec_id;
  char * EV_name;
} CAMERA_EV_TBL_T;

const CAMERA_EV_TBL_T camera_EV_tbl[] = {
  {INCREASE_EV, "Increase EV by one step."},
  {DECREASE_EV, "Decrease EV by one step."},
};

typedef struct {
  Camera_Saturation_changes sc_id;
  char * saturation_name;
} CAMERA_SATURATION_TBL_T;

const CAMERA_SATURATION_TBL_T camera_saturation_tbl[] = {
  {INC_SATURATION, "Increase Satuation by one step."},
  {DEC_SATURATION, "Decrease Satuation by one step."},
};

typedef struct {
  Camera_Sharpness_changes bc_id;
  char * sharpness_name;
} CAMERA_SHARPNESS_TBL_T;

typedef struct {
  Camera_Luma cl_id;
  char * luma_name;
}CAMERA_LA_TBL_T;

const CAMERA_SHARPNESS_TBL_T camera_sharpness_tbl[] = {
  {INC_SHARPNESS, "Increase Sharpness."},
  {DEC_SHARPNESS, "Decrease Sharpness."},
};

const CAMERA_LA_TBL_T camera_la_tbl[] = {
  {ENABLE_LA,  "Enable Luma Adaptation."},
  {DISABLE_LA, "Disable Luma Adaptation."},
};

const WHITE_BALANCE_TBL_T white_balance_tbl[] = {
  { 	WHITE_BALANCE_AUTO,         "White Balance - Auto"},
  { 	WHITE_BALANCE_OFF,          "White Balance - Off"},
  {   WHITE_BALANCE_DAYLIGHT,     "White Balance - Daylight"},
  {   WHITE_BALANCE_INCANDESCENT, "White Balance - Incandescent"},
  {   WHITE_BALANCE_FLUORESCENT,  "White Balance - Fluorescent"},
};

const GET_CTRL_TBL_T get_ctrl_tbl[] = {
  {     WHITE_BALANCE_STATE,           "Get white balance state (auto/off)"},
  {     WHITE_BALANCE_TEMPERATURE,      "Get white balance temperature"},
  {     BRIGHTNESS_CTRL,      "Get brightness value"},
  {     EV,      "Get exposure value"},
  {     CONTRAST_CTRL,      "Get contrast value"},
  {     SATURATION_CTRL,      "Get saturation value"},
  {     SHARPNESS_CTRL,      "Get sharpness value"},
};

const EXP_METERING_TBL_T exp_metering_tbl[] = {
  {   EXP_METERING_FRAME_AVERAGE,      "Exposure Metering - Frame Average"},
  {   EXP_METERING_CENTER_WEIGHTED,    "Exposure Metering - Center Weighted"},
  {   EXP_METERING_SPOT_METERING,      "Exposure Metering - Spot Metering"},
};

const ISO_TBL_T iso_tbl[] = {
  {   ISO_AUTO, "ISO: Auto"},
  {   ISO_DEBLUR, "ISO: Deblur"},
  {   ISO_100, "ISO: 100"},
  {   ISO_200, "ISO: 200"},
  {   ISO_400, "ISO: 400"},
  {   ISO_800, "ISO: 800"},
  {   ISO_1600, "ISO: 1600"},
};

const ZOOM_TBL_T zoom_tbl[] = {
  {   ZOOM_IN, "Zoom In one step"},
  {   ZOOM_OUT, "Zoom Out one step"},
  {   ZOOM_IN_SMOOTH, "Zoom In all the way smoothly"},
  {   ZOOM_OUT_SMOOTH, "Zoom Out all the way smoothly"},
};

typedef struct {
  camera_bestshot_mode_type bs_id;
  char *name;
} BESTSHOT_MODE_TBT_T;

const BESTSHOT_MODE_TBT_T bestshot_mode_tbl[] = {
  {CAMERA_BESTSHOT_OFF,            "Bestshot Mode: Off"},
  {CAMERA_BESTSHOT_LANDSCAPE,      "Bestshot Mode: Landscape"},
  {CAMERA_BESTSHOT_BEACH,          "Bestshot Mode: Beach"},
  {CAMERA_BESTSHOT_SNOW,           "Bestshot Mode: Snow"},
  {CAMERA_BESTSHOT_SUNSET,         "Bestshot Mode: Sunset"},
  {CAMERA_BESTSHOT_NIGHT,          "Bestshot Mode: Night"},
  {CAMERA_BESTSHOT_PORTRAIT,       "Bestshot Mode: Portrait"},
  {CAMERA_BESTSHOT_BACKLIGHT,      "Bestshot Mode: Backlight"},
  {CAMERA_BESTSHOT_SPORTS,         "Bestshot Mode: Sports"},
  {CAMERA_BESTSHOT_ANTISHAKE,      "Bestshot Mode: Antishake"},
  {CAMERA_BESTSHOT_FLOWERS,        "Bestshot Mode: Flowers"},
  {CAMERA_BESTSHOT_CANDLELIGHT,    "Bestshot Mode: Candlelight"},
  {CAMERA_BESTSHOT_FIREWORKS,      "Bestshot Mode: Fireworks"},
  {CAMERA_BESTSHOT_PARTY,          "Bestshot Mode: Party"},
  {CAMERA_BESTSHOT_NIGHT_PORTRAIT, "Bestshot Mode: Night_Portrait"},
  {CAMERA_BESTSHOT_THEATRE,        "Bestshot Mode: Theatre"},
  {CAMERA_BESTSHOT_ACTION,         "Bestshot Mode: Action"},
  {CAMERA_BESTSHOT_AR,             "Bestshot Mode: Augmented Reality"},
};

typedef struct {
  BL_n_Snow_changes rc_id;
  char * bl_snow_name;
} BL_N_SNOW_DETECT_TBL_T;

const BL_N_SNOW_DETECT_TBL_T bl_n_snow_detect_tbl[] = {
  {BL_DETECTION_ON,    "Backlight Detection on."},
  {BL_DETECTION_OFF,   "Backlight Detection off."},
  {SNOW_DETECTION_ON,  "Snow Scene Detection on."},
  {SNOW_DETECTION_OFF, "Snow Scene Detection off."},
};

typedef struct {
  HDR_mode_changes rc_id;
  char * hdr_mode_name;
} HDR_MODE_TBL_T;

const HDR_MODE_TBL_T hdr_mode_tbl[] = {
  {HDR_MODE_ON,    "Turn HDR on."},
  {HDR_MODE_OFF,   "Turn HDR off."},
  {EXPOSURE_METERING_ON,  "Turn Exposure Metering on."},
};

struct v4l2_fmtdesc enumfmtdesc[MAX_NUM_FORMAT];
int numfmts;
struct v4l2_format current_fmt;

/*===========================================================================
 * Forward declarations
 *===========================================================================*/
static int set_fps(int fps);
static int start_snapshot (void);
static int stop_snapshot (void);
/*===========================================================================
 * Static global variables
 *===========================================================================*/
extern USER_INPUT_DISPLAY_T input_display;
static int camframe_status = 0;
char camera_dev_node[BUFF_SIZE_64];
struct v4l2_capability v4l2_cap;

int8_t hjr_status = FALSE;

#ifdef _ANDROID_
char *sdcard_path = "/data";
#else
char *sdcard_path = ".";
#endif

#ifndef DISABLE_JPEG_ENCODING
void *libqcamera = NULL;
void (**LINK_jpegfragment_callback)(uint8_t * buff_ptr , uint32_t buff_size);
void (**LINK_jpeg_callback)(void);
#endif /* DISABLE_JPEG_ENCODING */

int eventfd = 0;
int camfd = 0;
int videofd = 0;
int snapshotfd = 0;
int thumbnailfd = 0;
int rawfd = 0;
int ionfd = 0;
int ctrlfd;
int video_on = 0;  /* in test we capture video recording also */
int snapshot_on = 0; /* config the snapshot test */
int restart_preview = 0; /* stop preview and then restart preview */
int num_supported_fmts = 0;
int memoryType = V4L2_MEMORY_MMAP; /* default */
int bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE; /* default */
uint32_t ker_prev_format = V4L2_PIX_FMT_NV21; /* default */
uint32_t ker_enc_format = V4L2_PIX_FMT_NV21; /* default */
uint32_t ker_main_format = V4L2_PIX_FMT_NV21; /* default */
uint32_t ker_thumb_format = V4L2_PIX_FMT_NV21; /* default */
cam_format_t usr_prev_format = CAMERA_YUV_420_NV21; /* default */
cam_format_t usr_enc_format = CAMERA_YUV_420_NV21; /* default */
cam_format_t usr_main_format = CAMERA_YUV_420_NV21; /* default */
cam_format_t usr_thumb_format = CAMERA_YUV_420_NV21; /* default */
int preview_video_resolution_flag = 0;
int effect = CAMERA_EFFECT_OFF;
int brightness = CAMERA_DEF_BRIGHTNESS;
int contrast = CAMERA_DEF_CONTRAST;
int saturation = CAMERA_DEF_SATURATION;
int sharpness = CAMERA_DEF_SHARPNESS;
int32_t ev_num = 0;
uint8_t ezTune = FALSE;
int pmemThumbnailfd = 0;
int pmemSnapshotfd = 0;
int pmemRawSnapshotfd = 0;
int fdSnapshot = 0;
int fdThumbnail = 0;
char snapshotBuf[256] = { 0};
char thumbnailBuf[256] = { 0};
uint32_t snapshot_buff_size = 0;
uint32_t raw_snapshot_buffer_size = 0;
static int thumbnailCntr = 0, snapshotCntr = 0;
int thumbnail_num = THUMBNAIL_FRAMES_NUM;
int snapshot_num = SNAPSHOT_FRAMES_NUM;
unsigned char *thumbnail_buf = NULL, *main_img_buf = NULL, *raw_img_buf = NULL;
int32_t *sharpness_AF = NULL;
cam_ctrl_dimension_t *dimension = NULL;

int preview_bytesused[3];
int video_bytesused[3];

interface_ctrl_t intrfcCtrl;
config3a_wb_t autoWB = CAMERA_WB_AUTO;
isp3a_af_mode_t af_mode = AF_MODE_AUTO;
cam_af_focusrect_t afFocusRect = AUTO;

cam_af_ctrl_t af_ctrl;
camera_iso_mode_type iso = CAMERA_ISO_AUTO;
camera_antibanding_type antibanding = CAMERA_ANTIBANDING_OFF;
camera_auto_exposure_mode_type aec_mode = CAMERA_AEC_CENTER_WEIGHTED;
led_mode_t led_mode = LED_MODE_OFF;
strobe_flash_mode_t strobe_flash_mode = STROBE_FLASH_MODE_OFF;
motion_iso_t motion_iso = MOTION_ISO_OFF;
int32_t hue = CAMERA_DEF_HUE;
fps_mode_t fps_mode = FPS_MODE_AUTO;

struct v4l2_cropcap cropcap;
struct v4l2_crop crop;
struct v4l2_queryctrl zoom_queryctrl;
struct v4l2_queryctrl sharpness_queryctrl;
int zoom_level;

Camera_Resolution Resolution;
int32_t g_camParmInfo_current_value = 0;
extern unsigned long preview_frames_buf;
extern void test_app_mmcamera_videoframe_callback(struct msm_frame *frame); // video_cam.c

/* To flush free video buffers queue */
void (*LINK_cam_frame_flush_free_video)(void);
static int submain();

struct v4l2_frame_buffer frames[PREVIEW_FRAMES_NUM];
struct v4l2_frame_buffer video_frames[VIDEO_FRAMES_NUM];
struct v4l2_frame_buffer mainimg_frames[MAX_SNAPSHOT_FRAMES_NUM];
struct v4l2_frame_buffer thumbnail_frames[MAX_THUMBNAIL_FRAMES_NUM];
struct v4l2_frame_buffer raw_frames[RAW_FRAMES_NUM];

pthread_t frame_thread;

void test_app_camframe_timeout_callback(void)
{
  camframe_status = -1;
}
static int set_media_inst_ext_mode(int fd, int extmode)
{
  int rc = 0;
  struct v4l2_streamparm s_parm;
  s_parm.type = bufType;
  s_parm.parm.capture.extendedmode=extmode;
  CDBG("%s::fd=%d,extmode=%d\n",__func__,fd, extmode);
  rc = ioctl(fd, VIDIOC_S_PARM, &s_parm);
  if (rc < 0) {
    CDBG("error: ioctl VIDIOC_S_PARM failed: %s\n", strerror(errno));
  }
  return rc;
}
static int set_video_op_mode(int fd, int opmode)
{
  int rc = 0;
  struct v4l2_control s_ctrl;
  s_ctrl.id = MSM_V4L2_PID_CAM_MODE;
  s_ctrl.value = opmode;
  CDBG(":%s::s_ctrl,id=%d,value=%d\n",__func__,
    s_ctrl.id,s_ctrl.value);
  rc = ioctl(fd, VIDIOC_S_CTRL, &s_ctrl);
  if (rc < 0)
    CDBG("error: ioctl VIDIOC_S_CTRL failed: %s\n", strerror(errno));
  return rc;
}


/*===========================================================================
 * FUNCTION    - keypress_to_event -
 *
 * DESCRIPTION:
 *==========================================================================*/
int keypress_to_event(char keypress)
{
  char out_buf = INVALID_KEY_PRESS;
  if ((keypress >= 'A' && keypress <= 'Z') ||
    (keypress >= 'a' && keypress <= 'z')) {
    out_buf = tolower(keypress);
    out_buf = out_buf - 'a' + 1;
  } else if (keypress >= '1' && keypress <= '9') {
    out_buf = keypress;
    out_buf = keypress - '1' + BASE_OFFSET_NUM;
  }
  return out_buf;
}

int bl_n_snow_detection_change_menu_process(char event,
  camera_action_t * action_id_ptr)
{
  menu_id_change_t next_menu_id;
  switch (event) {
    case BL_DETECTION_ON:
      printf("BL_DETECTION_ON\n");
      * action_id_ptr = ACTION_BL_DETECTION_ON;
      next_menu_id = MENU_ID_MAIN;
      break;
    case BL_DETECTION_OFF:
      printf("BL_DETECTION_OFF\n");
      * action_id_ptr = ACTION_BL_DETECTION_OFF;
      next_menu_id = MENU_ID_MAIN;
      break;
    case SNOW_DETECTION_ON:
      printf("SNOW_DETECTION_ON\n");
      * action_id_ptr = ACTION_SNOW_DETECTION_ON;
      next_menu_id = MENU_ID_MAIN;
      break;
    case SNOW_DETECTION_OFF:
      printf("SNOW_DETECTION_OFF\n");
      * action_id_ptr = ACTION_SNOW_DETECTION_OFF;
      next_menu_id = MENU_ID_MAIN;
      break;
    default:
      printf("Invalidate Id\n");
      next_menu_id = MENU_ID_BL_SNOW;
      break;
  }
  return next_menu_id;
}

int next_menu(menu_id_change_t current_menu_id, char keypress, camera_action_t * action_id_ptr, int * action_param)
{
  char output_to_event;
  menu_id_change_t next_menu_id = MENU_ID_INVALID;
  * action_id_ptr = ACTION_NO_ACTION;

  output_to_event = keypress_to_event(keypress);
  CDBG("current_menu_id=%d\n",current_menu_id);
  CDBG("output_to_event=%d\n",output_to_event);
  switch(current_menu_id) {
    case MENU_ID_MAIN:
      switch(output_to_event) {
        case STOP_CAMERA:
          * action_id_ptr = ACTION_STOP_CAMERA;
          CDBG("STOP_CAMERA\n");
          break;

        case PREVIEW_VIDEO_RESOLUTION:
          next_menu_id = MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE;
          CDBG("next_menu_id = MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE = %d\n", next_menu_id);
          break;

        case SET_WHITE_BALANCE:
          next_menu_id = MENU_ID_WHITEBALANCECHANGE;
          CDBG("next_menu_id = MENU_ID_WHITEBALANCECHANGE = %d\n", next_menu_id);
          break;

        case SET_EXP_METERING:
          next_menu_id = MENU_ID_EXPMETERINGCHANGE;
          CDBG("next_menu_id = MENU_ID_EXPMETERINGCHANGE = %d\n", next_menu_id);
          break;

        case GET_CTRL_VALUE:
          next_menu_id = MENU_ID_GET_CTRL_VALUE;
          CDBG("next_menu_id = MENU_ID_GET_CTRL_VALUE = %d\n", next_menu_id);
          break;

        case BRIGHTNESS_GOTO_SUBMENU:
          next_menu_id = MENU_ID_BRIGHTNESSCHANGE;
          CDBG("next_menu_id = MENU_ID_BRIGHTNESSCHANGE = %d\n", next_menu_id);
          break;

        case CONTRAST_GOTO_SUBMENU:
          next_menu_id = MENU_ID_CONTRASTCHANGE;
          break;

        case EV_GOTO_SUBMENU:
          next_menu_id = MENU_ID_EVCHANGE;
          break;

        case SATURATION_GOTO_SUBMENU:
          next_menu_id = MENU_ID_SATURATIONCHANGE;
          break;

        case TOGGLE_AFR:
          * action_id_ptr = ACTION_TOGGLE_AFR;
          CDBG("next_menu_id = MENU_ID_TOGGLEAFR = %d\n", next_menu_id);
          break;

        case SET_ISO:
          next_menu_id = MENU_ID_ISOCHANGE;
          CDBG("next_menu_id = MENU_ID_ISOCHANGE = %d\n", next_menu_id);
          break;

        case SET_ZOOM:
          next_menu_id = MENU_ID_ZOOMCHANGE;
          CDBG("next_menu_id = MENU_ID_ZOOMCHANGE = %d\n", next_menu_id);
          break;

        case SET_SHARPNESS:
          next_menu_id = MENU_ID_SHARPNESSCHANGE;
          CDBG("next_menu_id = MENU_ID_SHARPNESSCHANGE = %d\n", next_menu_id);
          break;

        case TAKE_YUV_SNAPSHOT:
          * action_id_ptr = ACTION_TAKE_YUV_SNAPSHOT;
          CDBG("Taking YUV snapshot\n");
          break;

        case ANTI_BANDING:
          * action_id_ptr = ACTION_ANTI_BANDING;
          CDBG("next_menu_id = MENU_ID_ANTIBANDING = %d\n", next_menu_id);
          break;

        case SPECIAL_EFFECT:
          * action_id_ptr = ACTION_SPECIAL_EFFECT;
          CDBG("next_menu_id = MENU_ID_SPECIAL_EFFECT = %d\n", next_menu_id);
          break;

        case TOGGLE_STROBE_FLASH_MODE:
          CDBG("Strobe flash mode action\n");
          * action_id_ptr = ACTION_TOGGLE_STROBE_FLASH_MODE;
          break;

        case SET_LED_MODE:
          CDBG("Set LED Mode action\n");
          * action_id_ptr = ACTION_SET_LED_MODE;
          break;

        case SET_HJR:
          * action_id_ptr = ACTION_SET_HJR;
          break;

        case LUMA_ENABLE_DISABLE:
          next_menu_id = MENU_ID_LUMACHANGE;
          break;

        case AUTO_FOCUS:
          * action_id_ptr = ACTION_AUTO_FOCUS;
          break;

        case BEST_SHOT:
          next_menu_id = MENU_ID_BESTSHOT;
          break;

        case BL_SNOW:
          next_menu_id = MENU_ID_BL_SNOW;
          break;

        case TAKE_RAW_SNAPSHOT:
          * action_id_ptr = ACTION_TAKE_RAW_SNAPSHOT;
          CDBG("next_menu_id = ACTION_TAKE_RAW_SNAPSHOT = %d\n", next_menu_id);
          break;
        case TAKE_422_SNAPSHOT:
          * action_id_ptr = ACTION_TAKE_422_SNAPSHOT;
          CDBG("Taking YUV422 snapshot\n");
          break;

        case TAKE_INLINE_SNAPSHOT:
          * action_id_ptr = ACTION_TAKE_INLINE_SNAPSHOT;
          break;
        case SET_HDR_GOTO_SUBMENU:
          next_menu_id = MENU_ID_HDR_CHANGE;
          CDBG("next_menu_id = MENU_ID_HDR_CHANGE = %d\n", next_menu_id);
          break;

        default:
          next_menu_id = MENU_ID_MAIN;
          CDBG("next_menu_id = MENU_ID_MAIN = %d\n", next_menu_id);
          break;
      }
      break;

    case MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE:
      printf("MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE\n");
      * action_id_ptr = ACTION_PREVIEW_VIDEO_RESOLUTION;
      if (output_to_event > RESOLUTION_PREVIEW_VIDEO_MAX ||
        output_to_event < RESOLUTION_MIN) {
          next_menu_id = current_menu_id;
      }
      else {
        next_menu_id = MENU_ID_MAIN;
        * action_param = output_to_event;
      }
      break;

    case MENU_ID_WHITEBALANCECHANGE:
      printf("MENU_ID_WHITEBALANCECHANGE\n");
      * action_id_ptr = ACTION_SET_WHITE_BALANCE;
      if (output_to_event > 0 &&
        output_to_event <= sizeof(white_balance_tbl)/sizeof(white_balance_tbl[0])) {
          next_menu_id = MENU_ID_MAIN;
          * action_param = output_to_event;
      }
      else {
        next_menu_id = current_menu_id;
      }
      break;

    case MENU_ID_EXPMETERINGCHANGE:
      printf("MENU_ID_EXPMETERINGCHANGE\n");
      * action_id_ptr = ACTION_SET_EXP_METERING;
      if (output_to_event > 0 &&
        output_to_event <= sizeof(exp_metering_tbl)/sizeof(exp_metering_tbl[0])) {
          next_menu_id = MENU_ID_MAIN;
          * action_param = output_to_event;
      }
      else {
        next_menu_id = current_menu_id;
      }
      break;

    case MENU_ID_GET_CTRL_VALUE:
      printf("MENU_ID_GET_CTRL_VALUE\n");
      * action_id_ptr = ACTION_GET_CTRL_VALUE;
      if (output_to_event > 0 &&
        output_to_event <= sizeof(get_ctrl_tbl)/sizeof(get_ctrl_tbl[0])) {
          next_menu_id = MENU_ID_MAIN;
          * action_param = output_to_event;
      }
      else {
        next_menu_id = current_menu_id;
      }
      break;

    case MENU_ID_BRIGHTNESSCHANGE:
      switch (output_to_event) {
        case INC_BRIGHTNESS:
          * action_id_ptr = ACTION_BRIGHTNESS_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case DEC_BRIGHTNESS:
          * action_id_ptr = ACTION_BRIGHTNESS_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_BRIGHTNESSCHANGE;
          break;
      }
      break;

    case MENU_ID_CONTRASTCHANGE:
      switch (output_to_event) {
        case INC_CONTRAST:
          * action_id_ptr = ACTION_CONTRAST_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case DEC_CONTRAST:
          * action_id_ptr = ACTION_CONTRAST_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_CONTRASTCHANGE;
          break;
      }
      break;

    case MENU_ID_EVCHANGE:
      switch (output_to_event) {
        case INCREASE_EV:
          * action_id_ptr = ACTION_EV_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case DECREASE_EV:
          * action_id_ptr = ACTION_EV_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_EVCHANGE;
          break;
      }
      break;

    case MENU_ID_SATURATIONCHANGE:
      switch (output_to_event) {
        case INC_SATURATION:
          * action_id_ptr = ACTION_SATURATION_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case DEC_SATURATION:
          * action_id_ptr = ACTION_SATURATION_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_EVCHANGE;
          break;
      }
      break;

    case MENU_ID_ISOCHANGE:
      printf("MENU_ID_ISOCHANGE\n");
      * action_id_ptr = ACTION_SET_ISO;
      if (output_to_event > 0 &&
        output_to_event <= sizeof(iso_tbl)/sizeof(iso_tbl[0])) {
          next_menu_id = MENU_ID_MAIN;
          * action_param = output_to_event;
      } else {
        next_menu_id = current_menu_id;
      }
      break;

    case MENU_ID_ZOOMCHANGE:
      * action_id_ptr = ACTION_SET_ZOOM;
      if (output_to_event > 0 &&
        output_to_event <= sizeof(zoom_tbl)/sizeof(zoom_tbl[0])) {
          next_menu_id = MENU_ID_MAIN;
          * action_param = output_to_event;
      } else {
        next_menu_id = current_menu_id;
      }
      break;

    case MENU_ID_SHARPNESSCHANGE:
      switch (output_to_event) {
        case INC_SHARPNESS:
          * action_id_ptr = ACTION_SHARPNESS_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;
        case DEC_SHARPNESS:
          * action_id_ptr = ACTION_SHARPNESS_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;
        default:
          next_menu_id = MENU_ID_SHARPNESSCHANGE;
          break;
      }
      break;

    case MENU_ID_LUMACHANGE:
      switch (output_to_event) {
        case ENABLE_LA:
          * action_id_ptr = ACTION_ENABLE_LA;
          next_menu_id = MENU_ID_MAIN;
          break;
        case DISABLE_LA:
          *action_id_ptr = ACTION_DISABLE_LA;
          next_menu_id = MENU_ID_MAIN;
          break;
        default:
          next_menu_id = MENU_ID_LUMACHANGE;
          break;
      }
      break;

    case MENU_ID_BESTSHOT:
      /* outpur event starts from 1, camera besthot modes start from 0*/
      output_to_event = output_to_event - 1;
      if (output_to_event >= CAMERA_BESTSHOT_MAX) {
        next_menu_id = current_menu_id;
        * action_id_ptr = ACTION_NO_ACTION;
      } else {
        next_menu_id = MENU_ID_MAIN;
        * action_id_ptr = ACTION_SET_BESTSHOT_MODE;
        * action_param = output_to_event;
      }
      break;

    case MENU_ID_BL_SNOW:
      next_menu_id = bl_n_snow_detection_change_menu_process(
        output_to_event,
        action_id_ptr);
      break;

    case MENU_ID_HDR_CHANGE:
      CDBG("MENU_ID_HDR_CHANGE\n");
      * action_id_ptr = ACTION_HDR_MODE;
      if (output_to_event > 0 &&
        output_to_event <= sizeof(hdr_mode_tbl)/sizeof(hdr_mode_tbl[0])) {
          next_menu_id = MENU_ID_MAIN;
          * action_param = output_to_event;
      } else {
        next_menu_id = current_menu_id;
      }
      break;
    default:
      CDBG("menu id is wrong: %d\n", current_menu_id);
      break;
  }

  return next_menu_id;
}

/*===========================================================================
 * FUNCTION    - print_menu_preview_video -
 *
 * DESCRIPTION:
 * ===========================================================================*/
static void print_menu_preview_video(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in preview/video mode now        \n");
  printf("===========================================\n\n");

  char menuNum = 'A';
  for (i = 0; i < sizeof(camera_main_menu_tbl)/sizeof(camera_main_menu_tbl[0]); i++) {
    if (i == BASE_OFFSET) {
      menuNum = '1';
    }

    printf("%c.  %s\n", menuNum, camera_main_menu_tbl[i].menu_name);
    menuNum++;
  }

  printf("\nPlease enter your choice: ");

  return;
}

static void camera_preview_video_resolution_change_tbl(void) {
    unsigned int i;

    printf("\n");
    printf("==========================================================\n");
    printf("      Camera is in preview/video resolution mode       \n");
    printf("==========================================================\n\n");

    char previewVideomenuNum = 'A';
    for (i = 0; i < sizeof(preview_video_dimension_tbl) /
      sizeof(preview_video_dimension_tbl[0]); i++) {
        printf("%c.  %s\n", previewVideomenuNum,
          preview_video_dimension_tbl[i].str_name);
        previewVideomenuNum++;
    }

    printf("\nPlease enter your choice for Preview/Video Resolution: ");
    return;
}

static void camera_preview_video_wb_change_tbl(void) {
  unsigned int i;
  printf("\n");
  printf("==========================================================\n");
  printf("      Camera is in white balance change mode       \n");
  printf("==========================================================\n\n");

  char submenuNum = 'A';
  for (i = 0 ; i < sizeof(white_balance_tbl) /
                   sizeof(white_balance_tbl[0]); i++) {
        printf("%c.  %s\n", submenuNum, white_balance_tbl[i].wb_name);
        submenuNum++;
  }
  printf("\nPlease enter your choice for White Balance modes: ");
  return;
}

static void camera_preview_video_get_ctrl_value_tbl(void) {
  unsigned int i;
  printf("\n");
  printf("==========================================================\n");
  printf("      Camera is in get control value mode       \n");
  printf("==========================================================\n\n");

  char submenuNum = 'A';
  for (i = 0 ; i < sizeof(get_ctrl_tbl) /
                   sizeof(get_ctrl_tbl[0]); i++) {
        printf("%c.  %s\n", submenuNum, get_ctrl_tbl[i].get_ctrl_name);
        submenuNum++;
  }
  printf("\nPlease enter your choice for control value you want to get: ");
  return;
}

static void camera_preview_video_exp_metering_change_tbl(void) {
  unsigned int i;
  printf("\n");
  printf("==========================================================\n");
  printf("      Camera is in exposure metering change mode       \n");
  printf("==========================================================\n\n");

  char submenuNum = 'A';
  for (i = 0 ; i < sizeof(exp_metering_tbl) /
                   sizeof(exp_metering_tbl[0]); i++) {
        printf("%c.  %s\n", submenuNum, exp_metering_tbl[i].exp_metering_name);
        submenuNum++;
  }
  printf("\nPlease enter your choice for exposure metering modes: ");
  return;
}

static void camera_contrast_change_tbl(void) {
    unsigned int i;

    printf("\n");
    printf("==========================================================\n");
    printf("      Camera is in change contrast resolution mode       \n");
    printf("==========================================================\n\n");

    char contrastmenuNum = 'A';
    for (i = 0; i < sizeof(contrast_change_tbl) /
                    sizeof(contrast_change_tbl[0]); i++) {
        printf("%c.  %s\n", contrastmenuNum,
                            contrast_change_tbl[i].contrast_name);
        contrastmenuNum++;
    }

    printf("\nPlease enter your choice for contrast Change: ");
    return;
}

static void camera_EV_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in EV change mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_EV_tbl)/sizeof(camera_EV_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_EV_tbl[i].EV_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for EV changes: ");
  return;
}

static void camera_preview_video_zoom_change_tbl(void) {
  unsigned int i;
  struct v4l2_control ctrl;

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;

  if (ioctl(camfd, VIDIOC_G_CTRL, &ctrl) >= 0) {
    zoom_level = ctrl.value;
    printf("\n");
    printf("==========================================================\n");
    printf("      Camera is in zoom change mode: %d,  [%d..%d]        \n",
        ctrl.value, zoom_queryctrl.minimum, zoom_queryctrl.maximum);
    printf("==========================================================\n\n");

    char submenuNum = 'A';
    for (i = 0 ; i < sizeof(zoom_tbl) /
                   sizeof(zoom_tbl[0]); i++) {
        printf("%c.  %s\n", submenuNum, zoom_tbl[i].zoom_direction_name);
        submenuNum++;
    }
    printf("\nPlease enter your choice for zoom change direction: ");
  } else {
    printf("\nVIDIOC_G_CTRL error: %d\n", errno);
  }
  return;
}

static void camera_brightness_change_tbl(void) {
    unsigned int i;

    printf("\n");
    printf("==========================================================\n");
    printf("      Camera is in change brightness mode       \n");
    printf("==========================================================\n\n");

    char brightnessmenuNum = 'A';
    for (i = 0; i < sizeof(brightness_change_tbl) /
                    sizeof(brightness_change_tbl[0]); i++) {
        printf("%c.  %s\n", brightnessmenuNum,
                            brightness_change_tbl[i].brightness_name);
        brightnessmenuNum++;
    }

    printf("\nPlease enter your choice for Brightness Change: ");
    return;
}

static void camera_saturation_change_tbl(void) {
    unsigned int i;

    printf("\n");
    printf("==========================================================\n");
    printf("      Camera is in change saturation mode       \n");
    printf("==========================================================\n\n");

    char saturationmenuNum = 'A';
    for (i = 0; i < sizeof(camera_saturation_tbl) /
                    sizeof(camera_saturation_tbl[0]); i++) {
        printf("%c.  %s\n", saturationmenuNum,
                            camera_saturation_tbl[i].saturation_name);
        saturationmenuNum++;
    }

    printf("\nPlease enter your choice for Saturation Change: ");
    return;
}

char * set_preview_video_dimension_tbl(Camera_Resolution cs_id, uint16_t * width, uint16_t * height)
{
  unsigned int i;
  char * ptr = NULL;
  for (i = 0; i < sizeof(preview_video_dimension_tbl) /
    sizeof(preview_video_dimension_tbl[0]); i++) {
      if (cs_id == preview_video_dimension_tbl[i].cs_id) {
        *width = preview_video_dimension_tbl[i].width;
        *height = preview_video_dimension_tbl[i].height;
        ptr = preview_video_dimension_tbl[i].name;
        break;
      }
  }
  return ptr;
}

static void camera_preview_video_iso_change_tbl(void) {
  unsigned int i;
  printf("\n");
  printf("==========================================================\n");
  printf("      Camera is in ISO change mode       \n");
  printf("==========================================================\n\n");

  char submenuNum = 'A';
  for (i = 0 ; i < sizeof(iso_tbl) /
                   sizeof(iso_tbl[0]); i++) {
        printf("%c.  %s\n", submenuNum, iso_tbl[i].iso_modes_name);
        submenuNum++;
  }
  printf("\nPlease enter your choice for iso modes: ");
  return;
}

static void camera_preview_video_sharpness_change_tbl(void) {
  unsigned int i;
  printf("\n");
  printf("==========================================================\n");
  printf("      Camera is in sharpness change mode       \n");
  printf("==========================================================\n\n");

  char submenuNum = 'A';
  for (i = 0 ; i < sizeof(camera_sharpness_tbl) /
                   sizeof(camera_sharpness_tbl[0]); i++) {
        printf("%c.  %s\n", submenuNum, camera_sharpness_tbl[i].sharpness_name);
        submenuNum++;
  }
  printf("\nPlease enter your choice for sharpness modes: ");
  return;
}

static void camera_la_table() {
  uint32_t i;
  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in Luma Adaptation change mode now       \n");
  printf("===========================================\n\n");

  char lumasubmenuNum = 'A';
  for (i = 0; i < sizeof(camera_la_tbl)/sizeof(camera_la_tbl[0]); i++) {
    printf("%c.  %s\n", lumasubmenuNum, camera_la_tbl[i].luma_name);
    lumasubmenuNum++;
  }
  printf("\nPlease enter your choice for Luma Adaptation changes: ");
  return;
}

static void camera_set_bestshot_tbl(void)
{
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in set besthot mode now       \n");
  printf("===========================================\n\n");


  char bsmenuNum = 'A';
  for (i = 0; i < sizeof(bestshot_mode_tbl)/sizeof(bestshot_mode_tbl[0]); i++) {
    printf("%c.  %s\n", bsmenuNum,
      bestshot_mode_tbl[i].name);
    bsmenuNum++;
  }

  printf("\nPlease enter your choice of Bestshot Mode: ");
  return;
}

static void camera_bl_n_snow_detection_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===================================================\n");
  printf("   Camera is in bl_n_snow_detection change mode now\n");
  printf("===================================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(bl_n_snow_detect_tbl)/sizeof(bl_n_snow_detect_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, bl_n_snow_detect_tbl[i].bl_snow_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice: ");
  return;
}

static void camera_hdr_mode_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===================================================\n");
  printf("   Camera is in HDR change mode now\n");
  printf("===================================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(hdr_mode_tbl)/sizeof(hdr_mode_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, hdr_mode_tbl[i].hdr_mode_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice: ");
  return;
}

/*===========================================================================
 * FUNCTION     - increase_contrast -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_contrast (void) {
  ++contrast;
  if (contrast > CAMERA_MAX_CONTRAST) {
    contrast = CAMERA_MAX_CONTRAST;
    printf("Reached max CONTRAST. \n");
  } else
    printf("Increase CONTRAST to %d\n", contrast);

  /*intrfcCtrl.setContrast(camfd, contrast);*/

  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_CONTRAST;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_contrast is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_contrast is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_CONTRAST;
    /* Decreasing the contrast */
    control.value = contrast;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - decrease_contrast -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_contrast (void) {
  --contrast;
  if (contrast < CAMERA_MIN_CONTRAST) {
    contrast = CAMERA_MIN_CONTRAST;
    printf("Reached min CONTRAST. \n");
  } else
    printf("Decrease CONTRAST to %d\n", contrast);

  /*intrfcCtrl.setContrast(camfd, contrast);*/
  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_CONTRAST;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_contrast is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_contrast is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_CONTRAST;
    /* Decreasing the contrast */
    control.value = contrast;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - decrease_brightness -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_brightness (void) {
  brightness -= CAMERA_BRIGHTNESS_STEP;
  if (brightness < CAMERA_MIN_BRIGHTNESS) {
    brightness = CAMERA_MIN_BRIGHTNESS;
    printf("Reached min BRIGHTNESS. \n");
  } else
    printf("Decrease BRIGHTNESS to %d\n", brightness);

  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_BRIGHTNESS;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_BRIGHTNESS is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_BRIGHTNESS is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_BRIGHTNESS;
    /* Decreasing the Brightness */
    control.value = brightness;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - increase_brightness -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_brightness (void) {
  brightness += CAMERA_BRIGHTNESS_STEP;
  if (brightness > CAMERA_MAX_BRIGHTNESS) {
    brightness = CAMERA_MAX_BRIGHTNESS;
    printf("Reached max BRIGHTNESS. \n");
  } else
    printf("Increase BRIGHTNESS to %d\n", brightness);

  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_BRIGHTNESS;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_BRIGHTNESS is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_BRIGHTNESS is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_BRIGHTNESS;
    /* Increasing the Brightness */
    control.value = brightness;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - increase_EV -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_EV (void) {
  int32_t ev = 0;
  if (++ev_num <= 12) {
     ev = (ev_num << 16) | 6;
    printf("Increase EV to %d\n", ev_num);
  } else {
    printf("Reached max EV. \n");
    ev = ev_num;
  }

  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_EXPOSURE;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_EXPOSURE is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_EXPOSURE is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_EXPOSURE;
    /* Increasing the EV*/
    control.value = ev;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - decrease_EV -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_EV (void) {
  int32_t ev = 0;
  if (--ev_num > -12) {
    ev = (ev_num << 16) | 6;
    printf("Decrease EV to %d\n", ev_num);
  } else {
    printf("Reached min EV. \n");
    ev = ev_num;
  }

  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_EXPOSURE;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_EXPOSURE is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_EXPOSURE is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_EXPOSURE;
    /* Increasing the EV*/
    control.value = ev;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - increase_contrast -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_saturation (void) {
  ++saturation;
  if (saturation > CAMERA_MAX_SATURATION) {
    saturation = CAMERA_MAX_SATURATION;
    printf("Reached max saturation. \n");
  } else
    printf("Increase saturation to %d\n", saturation);

  /*intrfcCtrl.setContrast(camfd, contrast);*/

  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_SATURATION;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_saturation is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_saturation is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_SATURATION;
    /* Decreasing the contrast */
    control.value = saturation;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - decrease_saturation -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_saturation (void) {
  --saturation;
  if (saturation < CAMERA_MIN_SATURATION) {
    saturation = CAMERA_MIN_SATURATION;
    printf("Reached min saturation. \n");
  } else
    printf("Decrease saturation to %d\n", saturation);

  /*intrfcCtrl.setContrast(camfd, contrast);*/
  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_SATURATION;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_saturation is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_saturation is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = V4L2_CID_SATURATION;
    /* Decreasing the contrast */
    control.value = saturation;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_antibanding -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_antibanding (void) {
  int rc = 0;
  struct v4l2_control ctrl;
  switch (antibanding) {
    case CAMERA_ANTIBANDING_OFF:
      printf("Set anti flicking 50Hz\n");
      antibanding = CAMERA_ANTIBANDING_50HZ;
      break;
    case CAMERA_ANTIBANDING_50HZ:
      printf("Set anti flicking 60Hz\n");
      antibanding = CAMERA_ANTIBANDING_60HZ;
      break;
    case CAMERA_ANTIBANDING_60HZ:
      printf("Turn off anti flicking\n");
      antibanding = CAMERA_ANTIBANDING_OFF;
      break;
    default:
      break;
  }
  ctrl.id = V4L2_CID_POWER_LINE_FREQUENCY;
  ctrl.value = antibanding;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

/*===========================================================================
 * FUNCTION     - SpecialEffect -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int SpecialEffect (void) {
  struct v4l2_control ctrl;
  effect++;
  if (effect == CAMERA_EFFECT_MAX)
    effect = CAMERA_EFFECT_OFF;
  int rc = 0;

  ctrl.id = MSM_V4L2_PID_EFFECT;
  ctrl.value = effect;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

/*===========================================================================
 * FUNCTION     - toggle_Strobe_Flash_Mode -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int toggle_Strobe_Flash_Mode (void) {
  struct v4l2_control ctrl;
  int rc = 0;
  strobe_flash_mode++;
  if(strobe_flash_mode == STROBE_FLASH_MODE_MAX) {
    strobe_flash_mode = STROBE_FLASH_MODE_OFF;
  }

  ctrl.id = MSM_V4L2_PID_STROBE_FLASH;
  ctrl.value = strobe_flash_mode;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

/*===========================================================================
 * FUNCTION     - LED_mode_change -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int LED_mode_change (void) {
  struct v4l2_control ctrl;
  int rc = 0;
  led_mode ++;
  if (led_mode >= LED_MODE_MAX)
    led_mode = LED_MODE_OFF;

  ctrl.id = MSM_V4L2_PID_LED_MODE;
  ctrl.value = led_mode;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

/*===========================================================================
 * FUNCTION     - set_hjr -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_hjr (void) {

  if (hjr_status) {
    hjr_status = FALSE;
    CDBG("HJR status = %d\n", hjr_status);
    iso = CAMERA_ISO_AUTO;
  } else {
    hjr_status = TRUE;
    iso = CAMERA_ISO_DEBLUR;
    CDBG("HJR status = %d \n", hjr_status);
  }
  int rc = 0;
  struct v4l2_control ctrl;

  ctrl.id = MSM_V4L2_PID_HJR;
  ctrl.value = hjr_status;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

/*===========================================================================
 * FUNCTION     - enable_la -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int enable_la()
{
  uint32_t LA_Enable = TRUE;
  fprintf(stderr, "Enabling Luma Adaptation\n");
  int rc = 0;
  struct v4l2_control ctrl;
  ctrl.id = MSM_V4L2_PID_LUMA_ADAPTATION;
  ctrl.value = LA_Enable;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

/*===========================================================================
 * FUNCTION     - disable_la -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int disable_la()
{
  uint32_t LA_Enable = FALSE;
  fprintf(stderr, "Disabling Luma Adaptation\n");
  int rc = 0;
  struct v4l2_control ctrl;
  ctrl.id = MSM_V4L2_PID_LUMA_ADAPTATION;
  ctrl.value = LA_Enable;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

int set_auto_focus (void)
{
  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control;

  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = V4L2_CID_FOCUS_AUTO;

  if (-1 == ioctl (camfd, VIDIOC_QUERYCTRL, &queryctrl)) {
    if (errno != EINVAL) {
      perror ("VIDIOC_QUERYCTRL");
      exit (EXIT_FAILURE);
    } else {
      printf ("V4L2_CID_FOCUS_AUTO is not supported\n");
    }
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    printf ("V4L2_CID_FOCUS_AUTO is not supported\n");
  } else {
    memset (&control, 0, sizeof (control));
    control.id = MSM_V4L2_PID_FOCUS_MODE;
    control.value = AF_MODE_AUTO;

    if (-1 == ioctl (camfd, VIDIOC_S_CTRL, &control)) {
      perror ("VIDIOC_S_CTRL");
      return -1;
    }
  }
  return 0;
}

int set_bestshot_mode(int bs_param)
{
  int rc = 0;
  struct v4l2_control ctrl;
  ctrl.id = MSM_V4L2_PID_BEST_SHOT;
  ctrl.value = bs_param;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

int set_bl_detection(int bl_param)
{
  int rc = 0;
  struct v4l2_control ctrl;
  ctrl.id = MSM_V4L2_PID_BL_DETECTION;
  ctrl.value = bl_param;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

int set_snow_detection(int snow_param)
{
  int rc = 0;
  struct v4l2_control ctrl;
  ctrl.id = MSM_V4L2_PID_SNOW_DETECTION;
  ctrl.value = snow_param;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  return rc;
}

/*===========================================================================
 * FUNCTION    - main -
 *
 * DESCRIPTION:
 *==========================================================================*/
int main(int argc, char **argv)
{
  int keep_on_going = 1;
  int c, rc, tmp_fd;
  int camera_id;
  struct v4l2_event_subscription sub;
  struct msm_ctrl_cmd ctrl_cmd;
  cam_prop_t cam_prop;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  video_on = 0;
  snapshot_on = 1;
  restart_preview = 0;
  rc = 0;
  /* get v4l2 params - memory type etc */
  while ((c = getopt(argc, argv, "mouhsf")) != -1) {
    switch (c) {
      case 'm':
        memoryType = V4L2_MEMORY_MMAP;
        break;

      case 'o':
        /*use_overlay_fb_display_driver();*/
        break;

      case 's':
        bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        break;

      case 'u':
        memoryType = V4L2_MEMORY_USERPTR;
        break;

      case 'f':
        ker_prev_format = V4L2_PIX_FMT_YUV420M;
        usr_prev_format = CAMERA_YUV_420_YV12;
        printf(" YV12 format selected ");
        break;

      case 'h':
      default:
        printf("usage: %s [-m] [-u] [-o] [-f]\n", argv[0]);
        printf("-f:   YV12 Image Format \n");
        printf("-m:   V4L2_MEMORY_MMAP.      \n");
        printf("-o:   use overlay fb display driver\n");
        printf("-s:   V4L2_BUF_TYPE_VIDEO_CAPTURE (1-plane)\n");
        printf("-u:   V4L2_MEMORY_USERPTR\n");
        exit(0);
    }
  }

#ifndef DISABLE_JPEG_ENCODING
  libqcamera = dlopen("liboemcamera.so", RTLD_NOW);
  if (!libqcamera) {
    CDBG("FATAL ERROR: could not dlopen liboemcamera.so: %s", dlerror());
    return -1;
  }

  *(void **)&LINK_jpegfragment_callback = dlsym(libqcamera, "mmcamera_jpegfragment_callback");
  if (!LINK_jpegfragment_callback) {
    CDBG("FATAL ERROR: could not dlsym jpegfragment_callback\n");
    rc = -1;
    goto error;
  }
  *LINK_jpegfragment_callback = v4l2_test_app_jpeg_fwrite;
  *(void **)&LINK_jpeg_callback = dlsym(libqcamera, "mmcamera_jpeg_callback");
  if (!LINK_jpeg_callback) {
    CDBG("FATAL ERROR: could not dlsym jpeg_callback\n");
    rc = -1;
    goto error;
  }
  *LINK_jpeg_callback = v4l2_test_app_jpeg_fclose;
#endif /* DISABLE_JPEG_ENCODING */

  CDBG("\nV4L2 Camera Test Application\n");

  struct timeval tdBeforePreviewVideo, tdStopCamera;
  struct timezone tz;

  gettimeofday(&tdBeforePreviewVideo, &tz);

  CDBG("Profiling: Start Camera timestamp = %ld ms\n",
    (tdBeforePreviewVideo.tv_sec * 1000) + (tdBeforePreviewVideo.tv_usec/1000));

  printf("Please select camera(0-back, 1-front):\n");
  camera_id = getchar() - '0';
  if (camera_id < 0 || camera_id > 1) {
    printf("Only camera 0 and camera 1 are supported!\n");
    rc = -1;
    goto error;
  }
  snprintf(camera_dev_node, BUFF_SIZE_64, "/dev/video%d", camera_id);

  eventfd = open(camera_dev_node,O_RDWR | O_NONBLOCK);
  if (eventfd < 0) {
    CDBG("v4l2_camera open failed!\n");
    rc = -1;
    goto error;
  }
#ifdef USE_ION
  ionfd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (ionfd < 0) {
    CDBG_ERROR("Ion device open failed\n");
    rc = -1;
    goto error_ionfd_open;
  }
#endif

  /* subscribe to event */
  sub.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_DONE_EVENT;
  rc = ioctl(eventfd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    CDBG_ERROR("error: ioctl VIDIOC_SUBSCRIBE_EVENT failed : %s\n",
      strerror(errno));
    goto error_subscribe_event;
  }
  /* get the device capabilities */
  rc = ioctl(eventfd, VIDIOC_QUERYCAP, &v4l2_cap);
  if (rc < 0) {
    CDBG("error: ioctl VIDIOC_S_FMT failed: %s\n", strerror(errno));
    goto error_querycap;
  }
  /* print capabilities and check if they are apporpriate */
  CDBG("v4l2 capabilities = %d\n", v4l2_cap.capabilities);
  if(!(v4l2_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
     !(v4l2_cap.capabilities & V4L2_CAP_STREAMING)) {
    CDBG("V4L2 device not capable of streaming, return\n");
    goto error_querycap;
  }

  memset(&v4l2_ioctl, 0, sizeof(v4l2_ioctl));
  memset(&ctrl_cmd, 0, sizeof(ctrl_cmd));

  ctrl_cmd.type = CAMERA_GET_CAPABILITIES;
  ctrl_cmd.length = sizeof(cam_prop_t);;
  ctrl_cmd.value = &cam_prop;
  ctrl_cmd.timeout_ms = 1000;
  ctrl_cmd.status = CAM_CTRL_SUCCESS;
  v4l2_ioctl.id = MSM_V4L2_PID_CTRL_CMD;
  v4l2_ioctl.ioctl_ptr = &ctrl_cmd;

  rc = ioctl(eventfd, MSM_CAM_V4L2_IOCTL_PRIVATE_S_CTRL, &v4l2_ioctl);
  if (rc < 0)
    CDBG_ERROR("error: ioctl CAMERA_GET_CAPABILITIES failed: %s\n",
         strerror(errno));

  system_get_sensor_fmt(eventfd);

  /* main loop doing the work*/
  do {
    keep_on_going = submain();
  } while ( keep_on_going );

  /* Clean up and exit. */
  CDBG("Exiting the app\n");

error_querycap:
  ioctl(eventfd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
error_subscribe_event:
#ifdef USE_ION
  close(ionfd);
#endif
error_ionfd_open:
  close(eventfd);
error:
  gettimeofday(&tdStopCamera, &tz);
  CDBG("Exiting application\n");
  CDBG("Profiling: Stop camera end timestamp = %ld ms\n",
      (tdStopCamera.tv_sec * 1000) + (tdStopCamera.tv_usec/1000));
#ifndef DISABLE_JPEG_ENCODING
  dlclose(libqcamera);
#endif /* DISABLE_JPEG_ENCODING */

  return rc;
}

/*===========================================================================
 * FUNCTION    - v4l2_free_buffers -
 *
 * DESCRIPTION:
 *==========================================================================*/

void v4l2_free_buffers(int fd, int mem_type, struct v4l2_frame_buffer *in_frames,
  int buffer_count)
{
  struct v4l2_requestbuffers bufreq;
  int rc = 0, i = 0;
  int cnt = 0;

  CDBG("%s: fd = %d, memtype = %d, buffer_count = %d\n",
     __func__, fd, mem_type, buffer_count);

  bufreq.count = 0;
  bufreq.type  = bufType;
  bufreq.memory =  mem_type;
  rc = ioctl(fd, VIDIOC_REQBUFS, &bufreq);
  if (rc < 0) {
    CDBG("%s: error: fd = %d, ioctl VIDIOC_REQBUFS failed: %s\n",
      __func__, fd, strerror(errno));
  }

  for (cnt = 0; cnt < buffer_count; cnt++) {
    switch (memoryType) {
    case V4L2_MEMORY_MMAP:
      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        for (i = 0; i < in_frames[cnt].buffer.length; i++) {
          CDBG(" FREEING UP BUFFER %p Size %d ", (void *)in_frames[cnt].addr[i], in_frames[cnt].buffer.m.planes[i].length);
          rc = munmap((void *)in_frames[cnt].addr[i], in_frames[cnt].buffer.m.planes[i].length);
          if(rc < 0) {
            CDBG("%s: fd = %d, munmap() failed: idx = %d, errno = %s|%d\n",
              __func__, fd, cnt,strerror(errno), errno);
          }
        }
        free(in_frames[cnt].buffer.m.planes);
      } else {
        rc = munmap((void *)in_frames[cnt].addr[0], in_frames[cnt].buffer.length);
        if(rc < 0) {
          CDBG("%s: fd = %d, munmap() failed: idx = %d, errno = %s|%d\n",
            __func__, fd, cnt,strerror(errno), errno);
        }
      }
      break;
    case V4L2_MEMORY_USERPTR:
      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        for (i = 0; i < in_frames[cnt].buffer.length; i++) {
          CDBG(" FREEING UP BUFFER fd 0x%lx, %d Size %d ", in_frames[cnt].fd_data[i].fd,
               (int)in_frames[cnt].addr[i], in_frames[cnt].buffer.m.planes[i].length);
#ifdef USE_ION
          rc = do_munmap_ion(ionfd, &(in_frames[cnt].fd_data[i]), (void *)in_frames[cnt].addr[i],
                   in_frames[cnt].ion_alloc[i].len);
#else
          rc = do_munmap(in_frames[cnt].buffer.m.planes[i].m.userptr, (void *)in_frames[cnt].addr[i],
                         in_frames[cnt].buffer.m.planes[i].length);
#endif
          if (rc < 0) {
            CDBG("do_munmap() failed: %s %d\n", strerror(errno), errno);
          }
        }
        free(in_frames[cnt].buffer.m.planes);
      } else {
#ifdef USE_ION
        rc = do_munmap_ion(ionfd, &(in_frames[cnt].fd_data[0]), (void *)in_frames[cnt].addr[0],
            in_frames[cnt].ion_alloc[0].len);
#else
        rc = do_munmap(in_frames[cnt].buffer.m.userptr, (void *)in_frames[cnt].addr[0],
                       in_frames[cnt].buffer.length);
#endif
        if (rc < 0) {
          CDBG("do_munmap() failed: %s %d\n", strerror(errno), errno);
        }
     }
     break;
    }
  }

  return;
}
void fill_buffer_info(struct v4l2_buffer *buffer, int extendedmode)
{
       switch(extendedmode) {
         case MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW:
         case MSM_V4L2_EXT_CAPTURE_MODE_VIDEO:
           buffer->m.planes[0].length =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height);
           buffer->m.planes[0].bytesused = buffer->m.planes[0].length;
           if (usr_prev_format == CAMERA_YUV_420_YV12 &&
               extendedmode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
             buffer->m.planes[1].length =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/4);
             buffer->m.planes[2].length =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/4);
             buffer->m.planes[2].bytesused =
               buffer->m.planes[2].length;
           } else {
             buffer->m.planes[1].length =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/2);
           }
           buffer->m.planes[1].bytesused =
             buffer->m.planes[1].length;
         break;
         case MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL:
           buffer->m.planes[0].length =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                              CEILING16(current_fmt.fmt.pix_mp.height));
           buffer->m.planes[0].bytesused = buffer->m.planes[0].length;
           if (usr_thumb_format == CAMERA_YUV_422_NV61 || usr_thumb_format == CAMERA_YUV_422_NV16)
             buffer->m.planes[1].length =
                 PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                                CEILING16(current_fmt.fmt.pix_mp.height));
           else
             buffer->m.planes[1].length =
                 PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                                CEILING16(current_fmt.fmt.pix_mp.height)/2);
           buffer->m.planes[1].bytesused = buffer->m.planes[1].length;
         break;
         case MSM_V4L2_EXT_CAPTURE_MODE_MAIN:
           buffer->m.planes[0].length =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                              CEILING16(current_fmt.fmt.pix_mp.height));
           buffer->m.planes[0].bytesused = buffer->m.planes[0].length;
           if (usr_main_format == CAMERA_YUV_422_NV61 || usr_main_format == CAMERA_YUV_422_NV16) {
             buffer->m.planes[1].length =
                 PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                                CEILING16(current_fmt.fmt.pix_mp.height));
           } else
             buffer->m.planes[1].length =
                 PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                                CEILING16(current_fmt.fmt.pix_mp.height)/2);
           buffer->m.planes[1].bytesused = buffer->m.planes[1].length;
         break;
         case MSM_V4L2_EXT_CAPTURE_MODE_RAW:
           buffer->m.planes[0].length =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width * current_fmt.fmt.pix_mp.height);
           buffer->m.planes[0].bytesused = buffer->m.planes[0].length;
           buffer->length = 1;
         break;
         default:
           CDBG_ERROR(" Should not come here. ");
         break;
      }
}

void fill_buffer_info2(struct v4l2_buffer *buffer, int extendedmode)
{
       switch(extendedmode) {
       case MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW:
            preview_bytesused[0] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height);
           if (usr_prev_format == CAMERA_YUV_420_YV12) {
             preview_bytesused[1] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/4);
             preview_bytesused[2] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/4);
           } else {
             preview_bytesused[1] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/2);
           }
         break;
         case MSM_V4L2_EXT_CAPTURE_MODE_VIDEO:
           video_bytesused[0] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height);
           if (usr_prev_format == CAMERA_YUV_420_YV12) {
             video_bytesused[1] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/4);
             video_bytesused[2] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/4);
           } else {
             video_bytesused[1] =
               PAD_TO_WORD(current_fmt.fmt.pix_mp.width *
                           current_fmt.fmt.pix_mp.height/2);
           }
         break;
         case MSM_V4L2_EXT_CAPTURE_MODE_RAW:
           buffer->length = 1;
         break;
         default:
           CDBG_ERROR(" NOOP ");
         break;
      }
}

int v4l2_alloc_buffers(int fd, int mem_type, struct v4l2_frame_buffer *in_frames,
                              int buffer_count, int extendedmode)
{
  struct v4l2_requestbuffers bufreq;
  int rc = 0;
  int cnt = 0, i;
  int *ret;
  int preview_vid_fd = -1;

  bufreq.count = buffer_count;
  bufreq.type  = bufType;
  bufreq.memory =  mem_type;

  rc = ioctl(fd, VIDIOC_REQBUFS, &bufreq);
  if (rc < 0) {
    CDBG_ERROR("%s:error: fd=%d, ioctl VIDIOC_REQBUFS failed: %s\n",
      __func__, fd, strerror(errno));
    return rc;
  }

  switch (mem_type) {
  case V4L2_MEMORY_MMAP:
    for (cnt = 0; cnt < buffer_count; cnt++) {
      /* get the buffer length, offset etc to be mmapped */
      CDBG("Print the cnt: %d\n", cnt);
      in_frames[cnt].buffer.type = bufType;
      in_frames[cnt].buffer.memory= mem_type;
      in_frames[cnt].buffer.index = cnt;

      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        if (usr_prev_format == CAMERA_YUV_420_YV12)
          in_frames[cnt].buffer.length = 3; //Y, Cb, Cr.
        else
          in_frames[cnt].buffer.length = 2; //Y, CbCr.
        in_frames[cnt].buffer.m.planes = (struct v4l2_plane *)
          malloc(in_frames[cnt].buffer.length * sizeof(struct v4l2_plane));
      }
      rc = ioctl(fd, VIDIOC_QUERYBUF, &in_frames[cnt].buffer);
      if (rc < 0) {
        CDBG_ERROR("error: ioctl VIDIOC_QUERYBUF failed: %s\n", strerror(errno));
        v4l2_free_buffers(fd, mem_type, in_frames, cnt);
        return -EINVAL;
      }
      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        fill_buffer_info2(&in_frames[cnt].buffer, extendedmode);
        for (i = 0; i < in_frames[cnt].buffer.length ; i++) {
          CDBG("%s: Buff_len = %u, buff_offset = %u\n", __func__,
            in_frames[cnt].buffer.m.planes[i].length,
            in_frames[cnt].buffer.m.planes[i].m.mem_offset);

          /* now mmap */
          ret = mmap(NULL,
            in_frames[cnt].buffer.m.planes[i].length,
            PROT_READ  | PROT_WRITE,
            MAP_SHARED,
            fd,
            in_frames[cnt].buffer.m.planes[i].m.mem_offset);
          if(ret == MAP_FAILED) {
            CDBG_ERROR("mmap() failed: %s (%d)\n", strerror(errno), errno);
            v4l2_free_buffers(fd, mem_type, in_frames, cnt);
            return -ENOMEM;
          } else {
            in_frames[cnt].addr[i] = (unsigned long)ret;
            CDBG("index %d, mmapped buffer address = 0x%x, size = %d\n",
              cnt, (uint32_t)in_frames[cnt].addr[i],
              in_frames[cnt].buffer.m.planes[i].length);
          }
        }
      } else {
        CDBG("%s: fd = %d, Buff_len = %u,buff_offset = %u\n", __func__, fd,
          in_frames[cnt].buffer.length, in_frames[cnt].buffer.m.offset);
        /* now mmap */
        ret = mmap(NULL,
          in_frames[cnt].buffer.length,
          PROT_READ  | PROT_WRITE,
          MAP_SHARED,
          fd,
          in_frames[cnt].buffer.m.offset);
        if(ret == MAP_FAILED) {
          CDBG_ERROR("mmap() failed: %s (%d)\n", strerror(errno), errno);
          v4l2_free_buffers(fd, mem_type, in_frames, cnt);
          return -ENOMEM;
        } else {
          in_frames[cnt].addr[0] = (unsigned long)ret;
          CDBG("index %d, mmapped buffer address = 0x%x, size = %d\n",
            cnt, (uint32_t)in_frames[cnt].addr, in_frames[cnt].buffer.length);
        }
      }
    }
    break;

  case V4L2_MEMORY_USERPTR:
    for (cnt = 0; cnt < buffer_count; cnt++) {
      /* get the buffer length, offset etc to be mmapped */
      CDBG("Print the cnt: %d\n", cnt);
      in_frames[cnt].buffer.type = bufType;
      in_frames[cnt].buffer.memory= mem_type;
      in_frames[cnt].buffer.index = cnt;

      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        if ((usr_prev_format == CAMERA_YUV_420_YV12) &&
            (extendedmode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW))
          in_frames[cnt].buffer.length = 3; //Y, Cb, Cr.
        else
          in_frames[cnt].buffer.length = 2; //Y, CbCr.
        in_frames[cnt].buffer.m.planes = (struct v4l2_plane *)
          malloc(in_frames[cnt].buffer.length * sizeof(struct v4l2_plane));
      }
      /* query buffer to get the length */
      rc = ioctl(fd, VIDIOC_QUERYBUF, &in_frames[cnt].buffer);

      if (rc < 0) {
        CDBG("error: ioctl VIDIOC_QUERYBUF failed: %s\n", strerror(errno));
        v4l2_free_buffers(fd, mem_type, in_frames, cnt);
        return -EINVAL;
      }
      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        in_frames[cnt].buffer.reserved = 0;
        fill_buffer_info(&in_frames[cnt].buffer, extendedmode);
        for (i = 0; i < in_frames[cnt].buffer.length ; i++) {
          CDBG("userptr buffer length = %d\n", in_frames[cnt].buffer.m.planes[i].length);
#ifdef USE_ION
          in_frames[cnt].ion_alloc[i].len = in_frames[cnt].buffer.m.planes[i].length;
          in_frames[cnt].ion_alloc[i].flags = 0;
          in_frames[cnt].ion_alloc[i].heap_mask =
            (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
          in_frames[cnt].ion_alloc[i].align = 4096;
          in_frames[cnt].addr[i] = (unsigned long) do_mmap_ion(ionfd,
                       &(in_frames[cnt].ion_alloc[i]), &(in_frames[cnt].fd_data[i]),
                       &preview_vid_fd);
#else
          in_frames[cnt].addr[i] = (unsigned long) do_mmap(
                       in_frames[cnt].buffer.m.planes[i].length, &preview_vid_fd);
#endif
          if (!in_frames[cnt].addr[i]) {
            v4l2_free_buffers(fd, mem_type, in_frames, cnt);
            return -ENOMEM;
          }
          /* Put fd in userptr, and offset in reserved */
          in_frames[cnt].buffer.m.planes[i].m.userptr = preview_vid_fd;
          in_frames[cnt].buffer.m.planes[i].reserved[0] = 0;
          CDBG("userptr buffer address/fd = 0x%lx, size = %d, offset = %d\n",
            in_frames[cnt].buffer.m.planes[i].m.userptr,
            in_frames[cnt].buffer.m.planes[i].length,
            in_frames[cnt].buffer.m.planes[i].reserved[0]);
        }
     } else {
       in_frames[cnt].buffer.length = current_fmt.fmt.pix.width *
                          current_fmt.fmt.pix.height * 1.5;
       CDBG("userptr buffer length = %d\n", in_frames[cnt].buffer.length);
#ifdef USE_ION
       in_frames[cnt].ion_alloc[0].len = in_frames[cnt].buffer.length;
       in_frames[cnt].ion_alloc[0].flags = 0;
       in_frames[cnt].ion_alloc[0].heap_mask =
         (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
       in_frames[cnt].ion_alloc[0].align = 4096;
       in_frames[cnt].addr[0] = (unsigned long) do_mmap_ion(ionfd,
                       &(in_frames[cnt].ion_alloc[0]), &(in_frames[cnt].fd_data[0]),
                       &preview_vid_fd);
#else
       in_frames[cnt].addr[0] = (unsigned long) do_mmap(
         in_frames[cnt].buffer.length, &preview_vid_fd);
#endif
       if (!in_frames[cnt].addr[0]) {
          v4l2_free_buffers(fd, mem_type, in_frames, cnt);
          return -ENOMEM;
       }
       /* Put fd in userptr, and offset in reserved */
       in_frames[cnt].buffer.m.userptr = preview_vid_fd;
       in_frames[cnt].buffer.reserved = 0;
       CDBG("userptr buffer address/fd = 0x%lx, size = %d, offset = %d\n",
         in_frames[cnt].buffer.m.userptr, in_frames[cnt].buffer.length,
         in_frames[cnt].buffer.reserved);
     }
    }
    break;
  default:
    CDBG("It can not be happening - incorrect memory type\n");
    return -EINVAL;
  }

  return 0;
}
void send_native_ctrl_cmd(int fd, cam_ctrl_type type,
                    uint32_t length, void *value)
{
  int rc = 0;
  struct msm_ctrl_cmd ctrl_cmd;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  ctrl_cmd.type = type;
  ctrl_cmd.length = length;
  ctrl_cmd.timeout_ms = 1000;
  ctrl_cmd.value = value;
  memset(&v4l2_ioctl, 0, sizeof(v4l2_ioctl));
  v4l2_ioctl.id = MSM_V4L2_PID_CTRL_CMD;
  v4l2_ioctl.ioctl_ptr = &ctrl_cmd;

  rc = ioctl (fd, MSM_CAM_V4L2_IOCTL_PRIVATE_S_CTRL, &v4l2_ioctl);
  if(rc)
    CDBG_ERROR("%s: fd=%d, S_CTRL, id=0x%x, value = 0x%x, rc = %ld\n",
      __func__, fd, MSM_V4L2_PID_CTRL_CMD, (uint32_t)value, rc);
}

void system_dimension_init(int extendedmode)
{
 /* now that we have something supported, its time to negotiate
     dimensions using set_fmt call */
  if (!dimension) {
    dimension = malloc(sizeof(cam_ctrl_dimension_t));
    if (NULL == dimension) {
      CDBG_ERROR("%s: Error in malloc\n", __func__);
      return;
    }
    memset(dimension, 0, sizeof(cam_ctrl_dimension_t));
  }

  dimension->picture_width = MP1_WIDTH;
  dimension->picture_height = MP1_HEIGHT;
  dimension->ui_thumbnail_width = VGA_WIDTH;
  dimension->ui_thumbnail_height = VGA_HEIGHT;
  dimension->thumbnail_width = VGA_WIDTH;
  dimension->thumbnail_height = VGA_HEIGHT;
  dimension->orig_picture_dx = dimension->picture_width;
  dimension->orig_picture_dy = dimension->picture_height;
  dimension->prev_format = usr_prev_format;
  dimension->enc_format = usr_enc_format;
  dimension->thumb_format = usr_thumb_format;
  dimension->main_img_format = usr_main_format;

  if (preview_video_resolution_flag == 0) {
    dimension->video_width = WVGA_WIDTH;
    dimension->video_width = CEILING32(dimension->video_width);
    input_display.user_input_display_width = WVGA_WIDTH;
    input_display.user_input_display_height = WVGA_HEIGHT;
    dimension->video_height = WVGA_HEIGHT;
    dimension->orig_video_width = WVGA_WIDTH;
    dimension->orig_video_height = WVGA_HEIGHT;
    dimension->display_width = WVGA_WIDTH;
    dimension->display_height = WVGA_HEIGHT;
  } else {
    dimension->video_width = input_display.user_input_display_width;
    dimension->video_width = CEILING32(dimension->video_width);
    dimension->video_height = input_display.user_input_display_height;
    dimension->orig_video_width = dimension->video_width;
    dimension->orig_video_height = dimension->video_height;
    dimension->display_width = dimension->video_width;
    dimension->display_height = dimension->video_height;
  }

  current_fmt.type = bufType;
  if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
    current_fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    if ((usr_prev_format == CAMERA_YUV_420_YV12) &&
        (extendedmode == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW))
      current_fmt.fmt.pix_mp.num_planes = 3;
    else
      current_fmt.fmt.pix_mp.num_planes = 2;
  } else {
    current_fmt.fmt.pix.field = V4L2_FIELD_NONE;
  }

  switch(extendedmode) {
  case MSM_V4L2_EXT_CAPTURE_MODE_DEFAULT:
  case MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW:
    if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
      current_fmt.fmt.pix_mp.width = dimension->display_width;
      current_fmt.fmt.pix_mp.height= dimension->display_height;
      current_fmt.fmt.pix_mp.pixelformat = ker_prev_format;
    } else {
      current_fmt.fmt.pix.width = dimension->display_width;
      current_fmt.fmt.pix.height= dimension->display_height;
      current_fmt.fmt.pix.pixelformat = ker_prev_format;
    }
    break;
  case MSM_V4L2_EXT_CAPTURE_MODE_VIDEO:
    if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
      current_fmt.fmt.pix_mp.width = dimension->video_width;
      current_fmt.fmt.pix_mp.height= dimension->video_height;
      current_fmt.fmt.pix_mp.pixelformat = ker_enc_format;
    } else {
      current_fmt.fmt.pix.width = dimension->video_width;
      current_fmt.fmt.pix.height= dimension->video_height;
      current_fmt.fmt.pix.pixelformat = ker_enc_format;
    }
    break;
  case MSM_V4L2_EXT_CAPTURE_MODE_MAIN:
    if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
      current_fmt.fmt.pix_mp.width = dimension->picture_width;
      current_fmt.fmt.pix_mp.height= dimension->picture_height;
      current_fmt.fmt.pix_mp.pixelformat = ker_main_format;
    } else {
      current_fmt.fmt.pix.width = dimension->picture_width;
      current_fmt.fmt.pix.height= dimension->picture_height;
      current_fmt.fmt.pix.pixelformat = ker_main_format;
    }
    break;
  case MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL:
    if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
      current_fmt.fmt.pix_mp.width = dimension->ui_thumbnail_width;
      current_fmt.fmt.pix_mp.height= dimension->ui_thumbnail_height;
      current_fmt.fmt.pix_mp.pixelformat = ker_thumb_format;
    } else {
      current_fmt.fmt.pix.width = dimension->ui_thumbnail_width;
      current_fmt.fmt.pix.height= dimension->ui_thumbnail_height;
      current_fmt.fmt.pix.pixelformat = ker_thumb_format;
    }
    break;
  case MSM_V4L2_EXT_CAPTURE_MODE_RAW:
    /* Raw snapshot dimension is returned from camera server */
    if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
      current_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SBGGR10;
      current_fmt.fmt.pix_mp.num_planes = 1;
    }
    else
      current_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR10;
    break;
  default:
    break;
  }
}

void system_get_sensor_fmt(int fd)
{
  int i, rc;
  for (i = 0; i < MAX_NUM_FORMAT; i++) {
    enumfmtdesc[i].index = i;
    enumfmtdesc[i].type  = bufType;
    rc = ioctl(fd, VIDIOC_ENUM_FMT, &enumfmtdesc[i]);
    if( rc < 0) {
      CDBG("error: no more supported formats"
        "numfmts = %d, rc = %d\n", i, rc);
      numfmts = i;
      break;
    } else
      CDBG("Supported format at index i = %d with"
        "fourcc = %d\n", i, enumfmtdesc[i].pixelformat);
  }
}
int check_supported(uint32_t format)
{
  int i;
  for (i = 0; i < numfmts; i++) {
    CDBG("i %d, format %d, searched format %d\n", i, format, enumfmtdesc[i].pixelformat);
    if (enumfmtdesc[i].pixelformat == format)
      break;
  }
  if (i < numfmts) {
    CDBG("Found \n");
    return 0;
  }
  CDBG_ERROR("Format not found\n");
  return -1;
}

int check_preview_formats_supported(void)
{
  if (!check_supported(ker_prev_format) && !check_supported(ker_enc_format)){
    return 0;
  }
  return -1;
}

int check_snapshot_formats_supported(void)
{
  if (!check_supported(ker_main_format) && !check_supported(ker_thumb_format)){
    return 0;
  }
  return -1;
}

/*===========================================================================
 * FUNCTION    - system_video_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
int system_init()
{
  int rc;
  int result;
  int i = 0;
  struct v4l2_requestbuffers bufreq;
  unsigned long video_frame_buf = 0;
  CDBG("!! system_video_init!\n");
  cam_frame_v4l2_start_parms v4l2_frame_parms;

  if(check_preview_formats_supported()) {
    CDBG_ERROR("Invalid formats\n");
    rc = -1;
    goto ERROR;
  }

  /* now open the v4l2 device to get capabilities
     and then negotiate buffer sizes*/
  camfd = open(camera_dev_node,O_RDWR | O_NONBLOCK);
  if (camfd < 0) {
    CDBG("v4l2_camera open failed!\n");
    rc = -1;
    goto ERROR;
  }
  videofd = open(camera_dev_node,O_RDWR | O_NONBLOCK);
  if (videofd < 0) {
    CDBG("v4l2_camera open failed!\n");
    rc = -1;
    close(camfd);
    goto ERROR;
  }

  rc = set_media_inst_ext_mode(camfd,MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW);
  if (rc < 0) {
    CDBG("%s:error: ioctl VIDIOC_S_PARM failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }
  set_video_op_mode(camfd, MSM_V4L2_CAM_OP_PREVIEW);
  rc = set_media_inst_ext_mode(videofd, MSM_V4L2_EXT_CAPTURE_MODE_VIDEO);
  if (rc < 0) {
    CDBG("%s:error: ioctl VIDIOC_S_PARM failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }
  set_video_op_mode(videofd, MSM_V4L2_CAM_OP_VIDEO);

  CDBG("%s:v4l2_camera open camfd=%d!\n", __func__, camfd);
  system_dimension_init(MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW);
  send_native_ctrl_cmd(camfd, CAMERA_SET_PARM_DIMENSION,
           sizeof(cam_ctrl_dimension_t), (void *)dimension);
  rc = ioctl(camfd, VIDIOC_S_FMT, &current_fmt);
  if (rc < 0) {
    CDBG("error: ioctl VIDIOC_S_FMT failed: %s\n", strerror(errno));
    return rc;
  }
  rc = v4l2_alloc_buffers(camfd, memoryType, frames, PREVIEW_FRAMES_NUM,
                MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW);
  if (rc < 0) {
    CDBG("%s: v4l2_alloc_buffers preview failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }

  CDBG("%s:v4l2_camera open videofd=%d!\n", __func__, videofd);

  system_dimension_init(MSM_V4L2_EXT_CAPTURE_MODE_VIDEO);
  send_native_ctrl_cmd(videofd, CAMERA_SET_PARM_DIMENSION,
           sizeof(cam_ctrl_dimension_t), (void *)dimension);
  rc = ioctl(videofd, VIDIOC_S_FMT, &current_fmt);
  if (rc < 0) {
    CDBG("error: ioctl VIDIOC_S_FMT failed: %s\n", strerror(errno));
    return rc;
  }
  rc = v4l2_alloc_buffers(videofd, memoryType, video_frames, VIDEO_FRAMES_NUM,
                MSM_V4L2_EXT_CAPTURE_MODE_VIDEO);
  if (rc < 0) {
    CDBG("%s: v4l2_alloc_buffers video recording failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }

  /* Re-set input_display size to video size */
  input_display.user_input_display_width = dimension->video_width;
  input_display.user_input_display_height = dimension->video_height;

  memset(&crop, 0, sizeof(struct v4l2_crop));
  crop.type = bufType;

  v4l2_frame_parms.camfdP = camfd;
  v4l2_frame_parms.numFramesP = PREVIEW_FRAMES_NUM;
  v4l2_frame_parms.framesP = &frames[0];
  v4l2_frame_parms.camfdV = videofd;
  v4l2_frame_parms.numFramesV = VIDEO_FRAMES_NUM;
  v4l2_frame_parms.framesV = &video_frames[0];
  v4l2_frame_parms.crop = &crop;
  v4l2_frame_parms.image_format = usr_prev_format;

  // use overlay for 7x30
  use_overlay_fb_display_driver();
  if (launch_camframe_fb_thread()) {
    CDBG("main: launch_camframe_fb_thread failed!\n");
    goto ERROR;
  }
  if (launch_camframe_v4l2_thread(&v4l2_frame_parms)) {
      CDBG("main: launch_camframe_v4l2_thread failed!\n");
      goto ERROR;
  }
  return 0;

ERROR:
  /* @todo need to handle cleanup for ERRORs */
  return -1;
}

/*===========================================================================
 * FUNCTION    - system_destroy -
 *
 * DESCRIPTION:
 *==========================================================================*/
int system_destroy(void)
{
  int cnt = 0;
  int rc;
  int offset = 0;
  uint32_t buff_size = 0;

  CDBG("system_destroy!!!\n");
  release_camframe_v4l2_thread();
  CDBG("Called release_camframe_thread!!!\n");
  release_camframe_fb_thread();
  CDBG("Called release_camframe_fb_thread!!!\n");

  v4l2_free_buffers(camfd, memoryType, frames, PREVIEW_FRAMES_NUM);
  v4l2_free_buffers(videofd, memoryType, video_frames, VIDEO_FRAMES_NUM);

  if (videofd > 0)
  close(videofd);
  if (camfd > 0)
  close(camfd);

  return 0;
}

int v4l2_queryctrls()
{
  int rc = 0;

  /* query zoom controls */
  memset(&zoom_queryctrl, 0, sizeof(zoom_queryctrl));
  zoom_queryctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
  rc = ioctl(camfd, VIDIOC_QUERYCTRL, &zoom_queryctrl);
  CDBG("%s: zoom_queryctrl.minimum = %d, maximum = %d, step = %d, default = %d\n",
    __func__, zoom_queryctrl.minimum, zoom_queryctrl.maximum,
    zoom_queryctrl.step, zoom_queryctrl.default_value);
  if (rc == -1) {
    CDBG_ERROR("error: ioctl VIDIOC_QUERYCTRL failed: %s\n", strerror(errno));
    return rc;
  }

  /* query sharpness control */
  memset(&sharpness_queryctrl, 0, sizeof(sharpness_queryctrl));
  sharpness_queryctrl.id = V4L2_CID_SHARPNESS;
#if 1
  sharpness_queryctrl.step = CAMERA_SHARPNESS_STEP;
  sharpness_queryctrl.minimum = CAMERA_MIN_SHARPNESS;
  sharpness_queryctrl.maximum = CAMERA_MAX_SHARPNESS;
  sharpness_queryctrl.default_value = CAMERA_DEF_SHARPNESS;
#else
  rc = ioctl(camfd, VIDIOC_QUERYCTRL, &sharpness_queryctrl);
  if (rc == -1) {
    CDBG("error: ioctl VIDIOC_QUERYCTRL failed: %s\n", strerror(errno));
    return rc;
  }
#endif

  /* query cropcap */
  memset(&cropcap, 0, sizeof(cropcap));
  cropcap.type = bufType;
  rc = ioctl(camfd, VIDIOC_CROPCAP, &cropcap);
  if (rc == -1) {
    CDBG("error: ioctl VIDIOC_CROPCAP failed: %s\n", strerror(errno));
    return rc;
  } else {
    CDBG("VIDIOC_CROPCAP bounds: %d, %d, %d, %d\n", cropcap.bounds.left,
        cropcap.bounds.top, cropcap.bounds.width, cropcap.bounds.height);
    CDBG("VIDIOC_CROPCAP defrect: %d, %d, %d, %d\n", cropcap.defrect.left,
        cropcap.defrect.top, cropcap.defrect.width, cropcap.defrect.height);
    CDBG("VIDIOC_CROPCAP pixelaspect: %d, %d\n", cropcap.pixelaspect.numerator,
        cropcap.pixelaspect.denominator);
  }


  return rc;
}

int snapshot_init(cam_frame_v4l2_start_parms *pv4l2_frame_parms)
{
  int rc;
  int result;
  int i = 0;
  struct v4l2_requestbuffers bufreq;

  uint32_t buff_size;
  unsigned long video_frame_buf = 0;
  CDBG("!! system_video_init!\n");

  if(check_snapshot_formats_supported()) {
    CDBG_ERROR("Invalid formats\n");
    rc = -1;
    goto ERROR;
  }

  snapshotfd = open(camera_dev_node,O_RDWR | O_NONBLOCK);
  if (snapshotfd < 0) {
    CDBG("v4l2_camera open failed!\n");
    rc = -1;
    goto ERROR;
  }
  thumbnailfd = open(camera_dev_node,O_RDWR | O_NONBLOCK);
  if (thumbnailfd < 0) {
    CDBG("v4l2_camera open failed!\n");
    rc = -1;
    close(snapshotfd);
    goto ERROR;
  }
  rc = set_media_inst_ext_mode(thumbnailfd,
    MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL);
  if (rc < 0) {
    CDBG("%s:error: ioctl VIDIOC_S_PARM failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }
  rc = set_media_inst_ext_mode(snapshotfd, MSM_V4L2_EXT_CAPTURE_MODE_MAIN);
  if (rc < 0) {
    CDBG("%s:error: ioctl VIDIOC_S_PARM failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }
  rc = set_video_op_mode(snapshotfd, MSM_V4L2_CAM_OP_CAPTURE);
  CDBG("%s:set_video_op_mode videofd=%d,rc=%d\n", __func__, videofd, rc);
  if(rc < 0) {
    return rc;
  }

  system_dimension_init(MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL);
  send_native_ctrl_cmd(thumbnailfd, CAMERA_SET_PARM_DIMENSION,
           sizeof(cam_ctrl_dimension_t), (void *)dimension);
  rc = ioctl(thumbnailfd, VIDIOC_S_FMT, &current_fmt);
  if (rc < 0) {
    CDBG("error: ioctl VIDIOC_S_FMT failed: %s\n", strerror(errno));
    return rc;
  }
  rc = v4l2_alloc_buffers(thumbnailfd, memoryType, thumbnail_frames,
    thumbnail_num, MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL);
  if (rc < 0) {
    CDBG("%s: v4l2_alloc_buffers thumbnail failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }
  if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
    CDBG("%s:thumbnail current_fmt.fmt.pix_mp.width = %d,\
      current_fmt.fmt.pix_mp.height = %d\n",
      __func__, current_fmt.fmt.pix_mp.width, current_fmt.fmt.pix_mp.height);
  else
    CDBG("%s:thumbnail current_fmt.fmt.pix.width = %d,\
      current_fmt.fmt.pix.height = %d\n",
      __func__, current_fmt.fmt.pix.width, current_fmt.fmt.pix.height);
  system_dimension_init(MSM_V4L2_EXT_CAPTURE_MODE_MAIN);
  send_native_ctrl_cmd(snapshotfd, CAMERA_SET_PARM_DIMENSION,
           sizeof(cam_ctrl_dimension_t), (void *)dimension);
  rc = ioctl(snapshotfd, VIDIOC_S_FMT, &current_fmt);
  if (rc < 0) {
    CDBG("error: ioctl VIDIOC_S_FMT failed: %s\n", strerror(errno));
    return rc;
  }
  rc = v4l2_alloc_buffers(snapshotfd, memoryType, mainimg_frames,
    snapshot_num, MSM_V4L2_EXT_CAPTURE_MODE_MAIN);
  if (rc < 0) {
    CDBG("%s: v4l2_alloc_buffers thumbnail failed: %s\n", __func__,
      strerror(errno));
    return rc;
  }
  if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
    CDBG("%s:snapshot current_fmt.fmt.pix_mp.width = %d,\
      current_fmt.fmt.pix_mp.height = %d\n",
      __func__, current_fmt.fmt.pix_mp.width, current_fmt.fmt.pix_mp.height);
  else
    CDBG("%s:snapshot current_fmt.fmt.pix.width = %d,\
      current_fmt.fmt.pix.height = %d\n",
      __func__, current_fmt.fmt.pix.width, current_fmt.fmt.pix.height);
  pv4l2_frame_parms->camfdT = thumbnailfd;
  pv4l2_frame_parms->camfdS = snapshotfd;
  pv4l2_frame_parms->numFramesT = thumbnail_num;
  pv4l2_frame_parms->numFramesS = snapshot_num;
  pv4l2_frame_parms->framesT = &thumbnail_frames[0];
  pv4l2_frame_parms->framesS = &mainimg_frames[0];
  pv4l2_frame_parms->image_format = CAMERA_YUV_420_NV21; //Set to default.

  return 0;
ERROR:
  /* @todo need to handle cleanup for ERRORs */
  return -1;
}

int snapshot_destroy(void)
{
  int cnt = 0;
  int rc;
  int offset = 0;
  uint32_t buff_size = 0;

  v4l2_free_buffers(thumbnailfd, memoryType, thumbnail_frames, thumbnail_num);
  v4l2_free_buffers(snapshotfd, memoryType, mainimg_frames,
    snapshot_num);

  if (thumbnailfd > 0)
  close(thumbnailfd);
  if (snapshotfd > 0)
  close(snapshotfd);
  return 0;
}

int take_snapshot(void)
{
  int rc = 0;
  cam_frame_v4l2_start_parms v4l2_frame_parms;
  rc = snapshot_init(&v4l2_frame_parms);
  CDBG("%s: snapshot_init return %d\n", __func__, rc);
  if(rc) {
    goto end;
  }
  rc = start_snapshot();
  CDBG("%s: start_snapshot return %d\n", __func__, rc);
  if(rc) {
    goto end;
  }
  rc = cam_get_snapshot_images(&v4l2_frame_parms, dimension);
  CDBG("%s: cam_get_snapshot_images return %d\n", __func__, rc);
  if(rc) {
    goto end;
  }
  rc = stop_snapshot ();
  CDBG("%s:stop_snapshot return %d\n", __func__, rc);
  if(rc) {
    goto end;
  }
  rc = snapshot_destroy();
  CDBG("%s:stop_snapshot return %d\n", __func__, rc);
end:
  CDBG("%s: end, rc==%d\n", __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION     - take YUV snapshot
 *
 * DESCRIPTION:
 * ===========================================================================*/
int take_yuv_snapshot()
{
  int rc = 0;
  int i;
  CDBG("%s: stop_video\n", __func__);
  if (stop_video() != 0) {
    goto ERROR;
  }
  CDBG("%s: system_destory\n", __func__);
  if (system_destroy() < 0) {
    goto ERROR;
  }

  rc = take_snapshot();
  CDBG("%s: take_snapshot rc=%d\n", __func__, rc);
  if (rc)
    goto ERROR;

  if (system_init() < 0)
    goto ERROR;
  if (start_video() != 0) {
    goto ERROR;
  }

  printf ("Your snapshot is stored as /data/s_*.yuv and snapshot_*.jpg\n");
  return 0;
ERROR:
  return -1;
}

static int take_raw(void)
{
  int rc = 0;
  int i;
  enum v4l2_buf_type buf_type = bufType;

  rawfd = open(camera_dev_node, O_RDWR | O_NONBLOCK);
  if (rawfd < 0) {
    CDBG_ERROR("v4l2_camera open failed!\n");
    rc = -1;
    return rc;
  }
  /* Initialization */
  rc = set_media_inst_ext_mode(rawfd,
    MSM_V4L2_EXT_CAPTURE_MODE_MAIN);
  if (rc < 0) {
    CDBG_ERROR("%s: set_media_inst_ext_mode failed: %s", __func__,
      strerror(errno));
    return rc;
  }
  rc = set_video_op_mode(rawfd, MSM_V4L2_CAM_OP_RAW);
  if (rc < 0) {
    CDBG_ERROR("%s: set_video_op_mode failed: %s\n", __func__, strerror(errno));
    return rc;
  }
  system_dimension_init(MSM_V4L2_EXT_CAPTURE_MODE_RAW);
  send_native_ctrl_cmd(rawfd, CAMERA_SET_PARM_DIMENSION,
           sizeof(cam_ctrl_dimension_t), (void *)dimension);
  rc = ioctl(rawfd, VIDIOC_S_FMT, &current_fmt);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_S_FMT failed: %s\n", __func__, strerror(errno));
    return rc;
  }
  /* Now that we have the dimension information for raw snapshot,
   * call S_FMT again to pass it down to driver */
  current_fmt.fmt.pix.width = dimension->raw_picture_width;
  current_fmt.fmt.pix.height = dimension->raw_picture_height;
  CDBG_ERROR("raw width = %d, raw height = %d\n", current_fmt.fmt.pix.width, current_fmt.fmt.pix.height);
  rc = ioctl(rawfd, VIDIOC_S_FMT, &current_fmt);
  if (rc < 0) {
    CDBG_ERROR("%s(%d): ioctl VIDIOC_S_FMT failed: %s\n", __func__, __LINE__, strerror(errno));
    return rc;
  }

  rc = v4l2_alloc_buffers(rawfd, memoryType, raw_frames, RAW_FRAMES_NUM,
                    MSM_V4L2_EXT_CAPTURE_MODE_RAW);
  if (rc < 0) {
    CDBG_ERROR("%s: v4l2_alloc_buffers failed: %s\n", __func__, strerror(errno));
    return rc;
  }

  /* Take raw snapshot */
  for (i = 0 ; i < RAW_FRAMES_NUM; i++) {
    raw_frames[i].buffer.m.planes[0].bytesused = dimension->raw_picture_height *
      dimension->raw_picture_width;
    rc = ioctl(rawfd, VIDIOC_QBUF, &raw_frames[i].buffer);
    if (rc < 0) {
      CDBG_ERROR("%s: ioctl VIDIOC_QBUF failed: %s\n", __func__, strerror(errno));
      goto end;
    }
  }
  rc = ioctl(rawfd, VIDIOC_STREAMON, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMON failed: %s\n", __func__, strerror(errno));
    goto end;
  }
  rc = cam_get_raw_images(rawfd, &raw_frames[0], RAW_FRAMES_NUM);
  if (rc < 0) {
    CDBG_ERROR("%s: cam_get_raw_images failed: %s\n", __func__, strerror(errno));
    /* fall through */
  }
  /* stop raw snapshot */
  rc = ioctl(rawfd, VIDIOC_STREAMOFF, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMOFF failed: %s\n", __func__, strerror(errno));
    goto end;
  }

end:
  v4l2_free_buffers(rawfd, memoryType, raw_frames, RAW_FRAMES_NUM);
  if (rawfd > 0)
  close(rawfd);
  return 0;
}

/*===========================================================================
 * FUNCTION     - take raw snapshot
 *
 * DESCRIPTION:
 * ===========================================================================*/
int take_raw_snapshot()
{
  int rc = 0;
  int i;
  CDBG("%s: stop_video\n", __func__);
  if (stop_video() != 0) {
    goto ERROR;
  }
  CDBG("%s: system_destory\n", __func__);
  if (system_destroy() < 0) {
    goto ERROR;
  }

  rc = take_raw();
  CDBG("%s: take_raw_snapshot rc=%d\n", __func__, rc);
  if (rc)
    goto ERROR;

  if (system_init() < 0)
    goto ERROR;
  if (start_video() != 0) {
    goto ERROR;
  }

  printf ("Your raw snapshot is stored as /data/*.raw\n");
  return 0;
ERROR:
  return -1;
}

static int take_inline(void)
{
  int rc = 0;
  int i;
  enum v4l2_buf_type buf_type = bufType;

  thumbnailfd = open(camera_dev_node, O_RDWR | O_NONBLOCK);
  if (thumbnailfd < 0) {
    CDBG_ERROR("v4l2_camera open failed!\n");
    rc = -1;
    return rc;
  }
  /* Initialization */
  rc = set_media_inst_ext_mode(thumbnailfd,
    MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL);
  if (rc < 0) {
    CDBG_ERROR("%s: set_media_inst_ext_mode failed: %s", __func__,
      strerror(errno));
    return rc;
  }
  rc = set_video_op_mode(thumbnailfd, MSM_V4L2_CAM_OP_JPEG_CAPTURE);
  if (rc < 0) {
    CDBG_ERROR("%s: set_video_op_mode failed: %s\n", __func__, strerror(errno));
    return rc;
  }
  system_dimension_init(MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL);
  send_native_ctrl_cmd(thumbnailfd, CAMERA_SET_PARM_DIMENSION,
           sizeof(cam_ctrl_dimension_t), (void *)dimension);
  rc = ioctl(thumbnailfd, VIDIOC_S_FMT, &current_fmt);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_S_FMT failed: %s\n", __func__, strerror(errno));
    return rc;
  }

  rc = v4l2_alloc_buffers(thumbnailfd, memoryType, thumbnail_frames, THUMBNAIL_FRAMES_NUM,
                    MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL);
  if (rc < 0) {
    CDBG_ERROR("%s: v4l2_alloc_buffers failed: %s\n", __func__, strerror(errno));
    return rc;
  }

  CDBG_ERROR("%s: %d\n", __func__, __LINE__);
  CDBG_ERROR("%s:Configuration for Inline encode\n", __func__);
#ifndef DISABLE_JPEG_ENCODING
  jpeg_encoder_hw_inline_config();
#endif
  CDBG_ERROR("%s:picture_width=%d picture_height=%d\n", __func__,
    dimension->picture_width, dimension->picture_height);
  /* Take snapshot */
  for (i = 0 ; i < THUMBNAIL_FRAMES_NUM; i++) {
      CDBG("%s: inline jpeg VIDIOC_QBUF\n", __func__);
      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        thumbnail_frames[i].buffer.m.planes[0].bytesused = dimension->thumbnail_width *
                                        dimension->thumbnail_height;
        thumbnail_frames[i].buffer.m.planes[1].bytesused = dimension->thumbnail_width *
                                        dimension->thumbnail_height/2;
      } else
        CDBG("%s: frames[%d].buffer.type = %d, m.userptr = 0x%x"
          "length = %d\n", __func__, i, thumbnail_frames[i].buffer.type,
          (uint32_t)thumbnail_frames[i].buffer.m.userptr,
          thumbnail_frames[i].buffer.length);

    rc = ioctl(thumbnailfd, VIDIOC_QBUF, &thumbnail_frames[i].buffer);
    if (rc < 0) {
      CDBG_ERROR("%s: ioctl VIDIOC_QBUF failed: %s\n", __func__, strerror(errno));
      goto end;
    }
  }
  CDBG_ERROR("%s: %d\n", __func__, __LINE__);
  rc = ioctl(thumbnailfd, VIDIOC_STREAMON, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMON failed: %s\n", __func__, strerror(errno));
    goto end;
  }
  CDBG_ERROR("%s: %d\n", __func__, __LINE__);
  rc = cam_get_thumbnail_images(thumbnailfd, &thumbnail_frames[0], THUMBNAIL_FRAMES_NUM);
  if (rc < 0) {
    CDBG_ERROR("%s: cam_get_thumbnail_images failed: %s\n", __func__, strerror(errno));
    /* fall through */
  }

  /* wait for jpeg encoder to finish */
  CDBG_ERROR("%s: %d: Waiting for Gemini to done\n", __func__, __LINE__);
#ifndef DISABLE_JPEG_ENCODING
  jpeg_encoder_hw_inline_wait_done();
#endif
  CDBG_ERROR("%s: %d Gemini is done....\n", __func__, __LINE__);
  /* stop raw snapshot */
  rc = ioctl(thumbnailfd, VIDIOC_STREAMOFF, &buf_type);
  if (rc < 0) {
    CDBG_ERROR("%s: ioctl VIDIOC_STREAMOFF failed: %s\n", __func__, strerror(errno));
    goto end;
  }
  CDBG_ERROR("%s: %d\n", __func__, __LINE__);

end:
  v4l2_free_buffers(thumbnailfd, memoryType, thumbnail_frames, THUMBNAIL_FRAMES_NUM);
  if (thumbnailfd > 0)
  close(thumbnailfd);
  return 0;
}

/*===========================================================================
 * FUNCTION     - take inline snapshot
 *
 * DESCRIPTION:
 * ===========================================================================*/
int take_inline_snapshot()
{
  int rc = 0;
  int i;
  CDBG_ERROR("%s: stop_video\n", __func__);
  if (stop_video() != 0) {
    goto ERROR;
  }
  CDBG_ERROR("%s: system_destory\n", __func__);
  if (system_destroy() < 0) {
    goto ERROR;
  }

  rc = take_inline();
  CDBG_ERROR("%s: take_inline_snapshot rc=%d\n", __func__, rc);
  if (rc)
    goto ERROR;

  if (system_init() < 0)
    goto ERROR;
  if (start_video() != 0) {
    goto ERROR;
  }

  return 0;
ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - submain -
 *
 * DESCRIPTION:
 * ===========================================================================*/
static int submain()
{
  int rc = 0;
  int back_mainflag = 0;
  char tc_buf[3];
  int stop_preview = 1;
  menu_id_change_t current_menu_id = MENU_ID_MAIN, next_menu_id;
  camera_action_t action_id;
  int action_param;

  struct timeval tdStopCamera;
  struct timezone tz;

  CDBG("%s: system_init\n", __func__);
  if (system_init() < 0)
    goto ERROR;
  CDBG("%s: start_video\n", __func__);
  if (start_video() != 0) {
    goto ERROR;
  }
  /* Because the config thread doesn't have a lot of control information until video is started,
   * query controls here */
  v4l2_queryctrls();


  do {
    print_current_menu (current_menu_id);
    fgets(tc_buf, 3, stdin);
    next_menu_id = next_menu(current_menu_id, tc_buf[0], & action_id, & action_param);
    if (next_menu_id != MENU_ID_INVALID) {
      current_menu_id = next_menu_id;
    }

    if (action_id == ACTION_NO_ACTION) {
      continue;
    }
    if(camframe_status == -1) {
      printf("Preview/Video ERROR condition reported Closing Camera APP\n");
      break;
    }

    switch(action_id) {
      case ACTION_STOP_CAMERA:
        gettimeofday(&tdStopCamera, &tz);
        CDBG("Profiling: Stop Camera start timestamp = %ld ms\n",
          (tdStopCamera.tv_sec * 1000) + (tdStopCamera.tv_usec/1000));
        CDBG("Selection is for stop preview/video");
        if (stop_video() != 0) {
          goto ERROR;
        }
        break;

      case ACTION_PREVIEW_VIDEO_RESOLUTION:
        back_mainflag = 1;
        CDBG ("stopP: camfd = %d", camfd);
        if (stop_video() != 0) {
          goto ERROR;
        }
        CDBG("Selection for the preview/video resolution change\n");
        preview_video_resolution (action_param);
        break;

      case ACTION_SET_WHITE_BALANCE:
        CDBG("Selection for the White Balance changes\n");
        set_whitebalance(action_param);
        break;

      case ACTION_SET_EXP_METERING:
        CDBG("Selection for the Exposure Metering changes\n");
        set_exp_metering(action_param);
        break;

      case ACTION_GET_CTRL_VALUE:
        CDBG("Selection for getting control value\n");
        get_ctrl_value(action_param);
        break;

      case ACTION_BRIGHTNESS_INCREASE:
        printf("Increase brightness\n");
        increase_brightness();
        break;

      case ACTION_BRIGHTNESS_DECREASE:
        printf("Decrease brightness\n");
        decrease_brightness();
        break;

      case ACTION_CONTRAST_INCREASE:
        CDBG("Selection for the contrast increase\n");
        increase_contrast ();
        break;

      case ACTION_CONTRAST_DECREASE:
        CDBG("Selection for the contrast decrease\n");
        decrease_contrast ();
        break;

      case ACTION_EV_INCREASE:
        CDBG("Selection for the EV increase\n");
        increase_EV ();
        break;

      case ACTION_EV_DECREASE:
        CDBG("Selection for the EV decrease\n");
        decrease_EV ();
        break;

      case ACTION_SATURATION_INCREASE:
        CDBG("Selection for the EV increase\n");
        increase_saturation ();
        break;

      case ACTION_SATURATION_DECREASE:
        CDBG("Selection for the EV decrease\n");
        decrease_saturation ();
        break;

      case ACTION_TOGGLE_AFR:
        CDBG("Select for auto frame rate toggling\n");
        toggle_afr();
        break;

      case ACTION_SET_ISO:
        CDBG("Select for ISO changes\n");
        set_iso(action_param);
        break;

      case ACTION_SET_ZOOM:
        CDBG("Selection for the zoom direction changes\n");
        set_zoom(action_param);
        break;

      case ACTION_SHARPNESS_INCREASE:
        CDBG("Selection for sharpness increase\n");
        increase_sharpness();
        break;

      case ACTION_SHARPNESS_DECREASE:
        CDBG("Selection for sharpness decrease\n");
        decrease_sharpness();
        break;

      case ACTION_TAKE_YUV_SNAPSHOT:
        CDBG("Take YUV snapshot\n");
        ker_main_format = V4L2_PIX_FMT_NV21;
        usr_main_format = CAMERA_YUV_420_NV21;
        ker_thumb_format = V4L2_PIX_FMT_NV21;
        usr_thumb_format = CAMERA_YUV_420_NV21;
        if (take_yuv_snapshot() < 0)
          goto ERROR;
        break;

      case ACTION_ANTI_BANDING:
        CDBG("Selection for the anti banding change\n");
        set_antibanding();
        break;

      case ACTION_SPECIAL_EFFECT:
        CDBG("Selection for Special Effect changes\n");
        SpecialEffect();
        break;

      case ACTION_TOGGLE_STROBE_FLASH_MODE:
        CDBG("Selection for toggle strobe flash mode.\n");
        toggle_Strobe_Flash_Mode();
        break;

      case ACTION_SET_LED_MODE:
        CDBG("Selection for the set LED mode\n");
        LED_mode_change();
        break;

      case ACTION_SET_HJR:
        CDBG("Selection for the set HJR changes\n");
        set_hjr();
        set_iso(iso + 1);
        break;

      case ACTION_ENABLE_LA:
        enable_la();
        break;

      case ACTION_DISABLE_LA:
        disable_la();
        break;

      case ACTION_AUTO_FOCUS:
        set_auto_focus();
        break;

      case ACTION_SET_BESTSHOT_MODE:
        set_bestshot_mode(action_param);
        break;

      case ACTION_BL_DETECTION_ON:
        set_bl_detection(TRUE);
        break;

      case ACTION_BL_DETECTION_OFF:
        set_bl_detection(FALSE);
        break;

      case ACTION_SNOW_DETECTION_ON:
        set_snow_detection(TRUE);
        break;

      case ACTION_SNOW_DETECTION_OFF:
        set_snow_detection(FALSE);
        break;

      case ACTION_TAKE_RAW_SNAPSHOT:
        CDBG("Take Raw snapshot\n");
        if (take_raw_snapshot() < 0)
          goto ERROR;
        break;

      case ACTION_TAKE_422_SNAPSHOT:
        CDBG("Take YUV422 snapshot\n");
        ker_main_format = V4L2_PIX_FMT_NV61;
        usr_main_format = CAMERA_YUV_422_NV61;
        ker_thumb_format = V4L2_PIX_FMT_NV21;
        usr_thumb_format = CAMERA_YUV_420_NV21;
        if (take_yuv_snapshot() < 0)
            goto ERROR;
        break;

      case ACTION_TAKE_INLINE_SNAPSHOT:
	CDBG_ERROR("%s: %d\n", __func__, __LINE__);
        if (take_inline_snapshot() < 0)
          goto ERROR;
        break;

      case ACTION_HDR_MODE:
        CDBG("Change HDR mode\n");
        set_hdr_mode(action_param);
        break;

      case ACTION_NO_ACTION:
        printf("Go back to main menu");
        break;

      default:
        printf("\n\n!!!!!WRONG INPUT: %d!!!!\n", action_id);
        break;
    }
    usleep(1000 * 1000);
    CDBG("action_id = %d\n", action_id);
    camframe_status = 0;
  } while ((action_id != ACTION_STOP_CAMERA) &&
      (action_id != ACTION_PREVIEW_VIDEO_RESOLUTION));
  action_id = ACTION_NO_ACTION;

  system_destroy();

  return back_mainflag;

ERROR:
  back_mainflag = 0;
  return back_mainflag;
}

/*===========================================================================
 * FUNCTION     - start preview -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int start_preview (void) {
  CDBG("main:start_preview!\n");
  if (intrfcCtrl.startPreview(camfd) == FALSE) {
    CDBG("main:%d start_preview failed!\n", __LINE__);
    goto ERROR;
  }
  return 0;

ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - stop_preview -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int stop_preview (void) {
  if (intrfcCtrl.stopPreview(camfd) == FALSE) {
    CDBG("main:%d stop_preview failed!\n", __LINE__);
    goto ERROR;
    }
  return 0;

ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - start_video -
 *
 * DESCRIPTION:
 * ===========================================================================*/
static int start_video (void) {
  int rc = 0;
  int i = 0, j;
  enum v4l2_buf_type buf_type;
  CDBG("main:start_video!\n");

  /* queue the first three buffers only */
  switch (memoryType) {
  case V4L2_MEMORY_MMAP:
    if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
      for(i = 0; i < PREVIEW_FRAMES_NUM; i++) {
        frames[i].buffer.m.planes[0].bytesused = preview_bytesused[0];
        frames[i].buffer.m.planes[1].bytesused = preview_bytesused[1];
        frames[i].buffer.m.planes[2].bytesused = preview_bytesused[2];
      }
      for(i = 0; i < VIDEO_FRAMES_NUM; i++) {
        video_frames[i].buffer.m.planes[0].bytesused = video_bytesused[0];
        video_frames[i].buffer.m.planes[1].bytesused = video_bytesused[1];
        video_frames[i].buffer.m.planes[2].bytesused = video_bytesused[2];
      }
    }
  case V4L2_MEMORY_USERPTR:
    /* now queue buffer for both mmapped as well as userspace buffers */
    for(i = 0; i < (PREVIEW_FRAMES_NUM) - 1; i++) {
      if (frames[i].buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        CDBG("start_video: preview VIDIOC_QBUF\n");
        for (j = 0; j < frames[i].buffer.length; j++)
          CDBG("start_video: frames[%d].buffer.type = %d, m.userptr = 0x%d"
            "length = %d\n", i, frames[i].buffer.type,
            (uint32_t)frames[i].buffer.m.planes[j].m.userptr,
            frames[i].buffer.m.planes[j].length);
      }
      rc = ioctl(camfd, VIDIOC_QBUF, &frames[i].buffer);
      if (rc < 0) {
        CDBG("error: ioctl VIDIOC_QBUF failed: %s\n", strerror(errno));
        return rc;
      }
    }

/*To do allocated buffer for video frames*/
#if 0
    for(i = 0; i < (VIDEO_FRAMES_NUM) - 1; i++) {
      CDBG("start_video: video VIDIOC_QBUF\n");
      if (video_frames[i].buffer.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        CDBG("start_video: frames[%d].buffer.type = %d, m.userptr = 0x%x"
          "length = %d\n", i, video_frames[i].buffer.type,
          (uint32_t)video_frames[i].buffer.m.userptr,
          video_frames[i].buffer.length);
      }
      rc = ioctl(videofd, VIDIOC_QBUF, &video_frames[i].buffer);
      if (rc < 0) {
        CDBG("error: ioctl VIDIOC_QBUF failed: %s\n", strerror(errno));
        return rc;
      }
    }
#endif
    buf_type = bufType;
    rc = ioctl(camfd, VIDIOC_STREAMON, &buf_type);
    if (rc < 0) {
        CDBG("error: ioctl VIDIOC_STREAMON failed: %s\n",
          strerror(errno));
        return rc;
    }
    if(video_on) {
      buf_type = bufType;
      rc = ioctl(videofd, VIDIOC_STREAMON, &buf_type);
      if (rc < 0) {
        CDBG("error: video ioctl VIDIOC_STREAMON failed: %s\n",
          strerror(errno));
        return rc;
      }
    }
    break;

  default:
    CDBG("start_video: incorrect memory type\n");
    return -EINVAL;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION     - stop_video -
 *
 * DESCRIPTION:
 * ===========================================================================*/
static int stop_video (void) {
  int rc;
  enum v4l2_buf_type buf_type;

  CDBG("main:stop_video!\n");

  buf_type = bufType;

  switch (memoryType) {
  case V4L2_MEMORY_MMAP:
  case V4L2_MEMORY_USERPTR:
    buf_type = bufType;
    if (video_on) {
      rc = ioctl(videofd, VIDIOC_STREAMOFF, &buf_type);
      if (rc < 0) {
        CDBG("error: video ioctl VIDIOC_STREAMOFF failed: %s\n",
          strerror(errno));
        return rc;
      }
    }

    rc = ioctl(camfd, VIDIOC_STREAMOFF, &buf_type);
    if (rc < 0) {
      CDBG("error: preview ioctl VIDIOC_STREAMOFF failed: %s\n",
        strerror(errno));
      return rc;
    }
    break;

  default:
    CDBG("start_video: incorrect memory type\n");
    return -EINVAL;
  }

  return 0;
}
static int start_snapshot (void) {
  int rc = 0;
  int i = 0;
  enum v4l2_buf_type buf_type;
  struct msm_ctrl_cmd ctrlCmd;
  struct v4l2_control control;
  CDBG("main:start_video!\n");

  /* queue the first three buffers only */
  switch (memoryType) {
  case V4L2_MEMORY_MMAP:
  case V4L2_MEMORY_USERPTR:
    /* now queue buffer for both mmapped as well as userspace buffers */
    for(i = 0; i < thumbnail_num; i++) {
      CDBG("%s: preview VIDIOC_QBUF\n", __func__);
      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        thumbnail_frames[i].buffer.m.planes[0].bytesused = dimension->thumbnail_width *
                                        dimension->thumbnail_height;
        if (usr_thumb_format == CAMERA_YUV_422_NV61 || usr_thumb_format == CAMERA_YUV_422_NV16)
          thumbnail_frames[i].buffer.m.planes[1].bytesused = dimension->thumbnail_width *
                                        dimension->thumbnail_height;
        else
          thumbnail_frames[i].buffer.m.planes[1].bytesused = dimension->thumbnail_width *
                                        dimension->thumbnail_height/2;
      } else
        CDBG("%s: frames[%d].buffer.type = %d, m.userptr = 0x%x"
          "length = %d\n", __func__, i, thumbnail_frames[i].buffer.type,
          (uint32_t)thumbnail_frames[i].buffer.m.userptr,
          thumbnail_frames[i].buffer.length);
      rc = ioctl(thumbnailfd, VIDIOC_QBUF, &thumbnail_frames[i].buffer);
      if (rc < 0) {
        CDBG("error: ioctl VIDIOC_QBUF failed: %s\n", strerror(errno));
        goto end;
      }
    }
    for(i = 0; i < snapshot_num; i++) {
      CDBG("%s: video VIDIOC_QBUF\n", __func__);
      if (bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        mainimg_frames[i].buffer.m.planes[0].bytesused = dimension->picture_width *
                                        dimension->picture_height;
        if (usr_main_format == CAMERA_YUV_422_NV61 || usr_main_format == CAMERA_YUV_422_NV16)
          mainimg_frames[i].buffer.m.planes[1].bytesused = dimension->picture_width *
                                        dimension->picture_height;
        else
          mainimg_frames[i].buffer.m.planes[1].bytesused = dimension->picture_width *
                                        dimension->picture_height/2;
      } else
        CDBG("%s: frames[%d].buffer.type = %d, m.userptr = 0x%x"
          "length = %d\n", __func__, i, mainimg_frames[i].buffer.type,
          (uint32_t)mainimg_frames[i].buffer.m.userptr,
          mainimg_frames[i].buffer.length);

      rc = ioctl(snapshotfd, VIDIOC_QBUF, &mainimg_frames[i].buffer);
      if (rc < 0) {
        CDBG("error: ioctl VIDIOC_QBUF failed: %s\n", strerror(errno));
        goto end;
      }
    }
    ctrlCmd.timeout_ms  = 1000;
    ctrlCmd.type        = CAMERA_PREPARE_SNAPSHOT;
    ctrlCmd.length      = 0;
    ctrlCmd.value        = NULL;
    ctrlCmd.resp_fd     = camfd;
    control.id          = MSM_V4L2_PID_PREP_SNAPSHOT;
    control.value = (int32_t)&ctrlCmd;
    rc = ioctl(thumbnailfd, VIDIOC_S_CTRL, &control);
    if (rc < 0) {
        CDBG("%s:error: ioctl VIDIOC_S_CTRL failed: %s\n", __func__,
          strerror(errno));
        goto end;
    }
    buf_type = bufType;
    rc = ioctl(thumbnailfd, VIDIOC_STREAMON, &buf_type);
    if (rc < 0) {
      CDBG("error: ioctl VIDIOC_STREAMON failed: %s\n",
        strerror(errno));
      goto end;
    }
    buf_type = bufType;
    rc = ioctl(snapshotfd, VIDIOC_STREAMON, &buf_type);
    if (rc < 0) {
      CDBG("error: video ioctl VIDIOC_STREAMON failed: %s\n",
        strerror(errno));
      goto end;
    }
    break;

  default:
    CDBG("start_video: incorrect memory type\n");
    rc = -EINVAL;
    break;
  }
end:
  return 0;
}

static int stop_snapshot (void)
{
  int rc;
  enum v4l2_buf_type buf_type;

  CDBG("main:stop_video!\n");

  buf_type = bufType;

  switch (memoryType) {
  case V4L2_MEMORY_MMAP:
  case V4L2_MEMORY_USERPTR:
    rc = ioctl(thumbnailfd, VIDIOC_STREAMOFF, &buf_type);
    if (rc < 0) {
      CDBG("%s:error: preview ioctl VIDIOC_STREAMOFF failed: %s\n",
        __func__, strerror(errno));
      return rc;
    }
    buf_type = bufType;
    rc = ioctl(snapshotfd, VIDIOC_STREAMOFF, &buf_type);
    if (rc < 0) {
      CDBG("%s:error: video ioctl VIDIOC_STREAMOFF failed: %s\n",
        __func__, strerror(errno));
      return rc;
    }
    break;

  default:
    CDBG("%s: incorrect memory type\n", __func__);
    return -EINVAL;
  }

  return 0;
}
/*===========================================================================
 * FUNCTION     - preview_resolution -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int preview_video_resolution (int preview_video_action_param) {
  char * resolution_name;
  CDBG("Selecting the action for preview/video resolution = %d \n", preview_video_action_param);
  resolution_name = set_preview_video_dimension_tbl(preview_video_action_param,
                      & input_display.user_input_display_width,
                      & input_display.user_input_display_height);

  CDBG("Selected preview/video resolution is %s\n", resolution_name);

  if (resolution_name == NULL) {
    CDBG("main:%d set_preview_dimension failed!\n", __LINE__);
    goto ERROR;
  }

  CDBG("Selected Preview Resolution: display_width = %d, display_height = %d\n",
    input_display.user_input_display_width, input_display.user_input_display_height);

  preview_video_resolution_flag = 1;
  return 0;

ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - set_whitebalance -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_whitebalance (int wb_action_param) {

	int rc = 0;
	struct v4l2_control ctrl;

  if (wb_action_param == WHITE_BALANCE_AUTO) {
		ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
		ctrl.value = TRUE;
		rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);

	} else if ( wb_action_param == WHITE_BALANCE_OFF) {
		ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
		ctrl.value = FALSE;
		rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);

  } else {
		int temperature = 6500;

		switch (wb_action_param) {
			case WHITE_BALANCE_DAYLIGHT:
				temperature = 6500;
				break;
			case WHITE_BALANCE_INCANDESCENT:
				temperature = 2800;
				break;
			case WHITE_BALANCE_FLUORESCENT:
				temperature = 4200;
				break;
			default:
				temperature = 4200;
				break;
		}

		ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
		ctrl.value = temperature;
		rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
	}

DONE:
	return rc;
}

int set_hdr_mode(int hdr_action_param)
{
  int rc = 0;
	struct v4l2_control ctrl;
  exp_bracketing_t exp;
  switch (hdr_action_param) {
    case HDR_MODE_ON:
      CDBG("%s Setting HDR mode on\n", __func__);
      exp.hdr_enable = 1;
      exp.mode = HDR_MODE;
      exp.total_frames = 3;
      exp.total_hal_frames = 2;
    break;

    case HDR_MODE_OFF:
      CDBG("%s Setting HDR mode off\n", __func__);
      exp.hdr_enable = 0;
      exp.mode = HDR_BRACKETING_OFF;
      exp.total_frames = 1;
      exp.total_hal_frames = 1;
    break;

    case EXPOSURE_METERING_ON:
      CDBG("%s Setting exposure metering on\n", __func__);
      exp.hdr_enable = 0;
      exp.mode = EXP_BRACKETING_MODE;
      exp.total_frames = 3;
      exp.total_hal_frames = 3;
    break;

    default:
      exp.total_hal_frames = 1;
    break;
  }
  thumbnail_num = exp.total_hal_frames;
  snapshot_num = exp.total_hal_frames;

  send_native_ctrl_cmd(camfd, CAMERA_SET_PARM_HDR,
    sizeof(exp_bracketing_t), (void *)&exp);
  return rc;
}


/*===========================================================================
 * FUNCTION     - set_exp_metering -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_exp_metering (int exp_metering_action_param) {

	int rc = 0;
	struct v4l2_control ctrl;

  ctrl.id = MSM_V4L2_PID_EXP_METERING;
  ctrl.value = exp_metering_action_param - 1;
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);

  return rc;
}

int get_ctrl_value (int ctrl_value_mode_param){

    int rc = 0;
    struct v4l2_control ctrl;

    if (ctrl_value_mode_param == WHITE_BALANCE_STATE) {
        printf("You chose WHITE_BALANCE_STATE\n");
        ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
    }
    else if (ctrl_value_mode_param == WHITE_BALANCE_TEMPERATURE) {
        printf("You chose WHITE_BALANCE_TEMPERATURE\n");
        ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
    }
    else if (ctrl_value_mode_param == BRIGHTNESS_CTRL) {
        printf("You chose brightness value\n");
        ctrl.id = V4L2_CID_BRIGHTNESS;
    }
    else if (ctrl_value_mode_param == EV) {
        printf("You chose exposure value\n");
        ctrl.id = V4L2_CID_EXPOSURE;
    }
    else if (ctrl_value_mode_param == CONTRAST_CTRL) {
        printf("You chose contrast value\n");
        ctrl.id = V4L2_CID_CONTRAST;
    }
    else if (ctrl_value_mode_param == SATURATION_CTRL) {
        printf("You chose saturation value\n");
        ctrl.id = V4L2_CID_SATURATION;
    } else if (ctrl_value_mode_param == SHARPNESS_CTRL) {
        printf("You chose sharpness value\n");
        ctrl.id = V4L2_CID_SHARPNESS;
    }

    rc = ioctl(camfd, VIDIOC_G_CTRL, &ctrl);
    printf("The chosen control feature's value is %d\n", ctrl.value);

    return rc;
}

/*===========================================================================
 * FUNCTION     - toggle_afr -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int toggle_afr () {
  int rc = 0;
  struct v4l2_control ctrl;

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_EXPOSURE_AUTO;
  rc = ioctl(camfd, VIDIOC_G_CTRL, &ctrl);
  if (rc == -1) {
    CDBG("%s: VIDIOC_G_CTRL V4L2_CID_EXPOSURE_AUTO failed: %s\n",
        __func__, strerror(errno));
    return rc;
  }

  /* V4L2_CID_EXPOSURE_AUTO needs to be AUTO or SHUTTER_PRIORITY */
  if (ctrl.value != V4L2_EXPOSURE_AUTO &&
    ctrl.value != V4L2_EXPOSURE_SHUTTER_PRIORITY) {
    CDBG("%s: V4L2_CID_EXPOSURE_AUTO needs to be AUTO/SHUTTER_PRIORITY\n",
        __func__);
    return -1;
  }

  /* Get V4L2_CID_EXPOSURE_AUTO_PRIORITY */
  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_EXPOSURE_AUTO_PRIORITY;
  rc = ioctl(camfd, VIDIOC_G_CTRL, &ctrl);
  if (rc == -1) {
    CDBG("%s: VIDIOC_G_CTRL V4L2_CID_EXPOSURE_AUTO_PRIORITY failed: %s\n",
        __func__, strerror(errno));
    return rc;
  }

  ctrl.value = !ctrl.value;
  printf("V4L2_CID_EXPOSURE_AUTO_PRIORITY changed to %d\n", ctrl.value);
  rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
  if (rc == -1) {
    CDBG("%s: VIDIOC_S_CTRL V4L2_CID_EXPOSURE_AUTO_PRIORITY failed: %s\n",
      __func__, strerror(errno));
  }
  return rc;
}

int set_zoom (int zoom_action_param) {
    int rc = 0;
    struct v4l2_control ctrl;
    struct v4l2_event v4l2_evt;
     mm_camera_event_t *ev;

    switch (zoom_action_param) {
    case ZOOM_IN:
      zoom_level += zoom_queryctrl.step;
      if (zoom_level > zoom_queryctrl.maximum)
        zoom_level = zoom_queryctrl.maximum;
      ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
      ctrl.value = zoom_level;
      rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
      break;
    case ZOOM_OUT:
      zoom_level -= zoom_queryctrl.step;
      if (zoom_level < zoom_queryctrl.minimum)
        zoom_level = zoom_queryctrl.minimum;
      ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
      ctrl.value = zoom_level;
      rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
      break;
    case ZOOM_IN_SMOOTH:
      while (zoom_level < zoom_queryctrl.maximum) {
        zoom_level += zoom_queryctrl.step;
        if (zoom_level > zoom_queryctrl.maximum)
          zoom_level = zoom_queryctrl.maximum;
        ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
        ctrl.value = zoom_level;
        rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
        if (rc < 0)
          CDBG_ERROR("%s: VIDIOC_S_CTRL failed %s\n",
            __func__, strerror(errno));

        while (1)  {
          rc = ioctl(eventfd, VIDIOC_DQEVENT, &v4l2_evt);
          if (rc < 0)
            CDBG_ERROR("%s: VIDIOC_DQEVENT failed %s\n",
              __func__, strerror(errno));
            ev = (mm_camera_event_t *)v4l2_evt.u.data;
            if (ev->event_type == MM_CAMERA_EVT_TYPE_CTRL &&
              ev->e.ctrl.evt == MM_CAMERA_CTRL_EVT_ZOOM_DONE) {
              CDBG("ZOOM in notification: %d\n", ev->e.ctrl.status);
              break;
            }
	      }
        rc = ioctl(camfd, VIDIOC_G_CROP, &crop);
        if (!rc) {
          CDBG("Crop Rect is (%d, %d), (%d, %d)\n", crop.c.left, crop.c.top,
            crop.c.width, crop.c.height);
          usleep(200000);
        } else {
          CDBG_ERROR("VIDIOC_G_CROP failed %s\n", strerror(errno));
        }
      }
      break;
    case ZOOM_OUT_SMOOTH:
      while (zoom_level > zoom_queryctrl.minimum) {
        zoom_level -= zoom_queryctrl.step;
        if (zoom_level < zoom_queryctrl.minimum)
          zoom_level = zoom_queryctrl.minimum;
        ctrl.id = V4L2_CID_ZOOM_ABSOLUTE;
        ctrl.value = zoom_level;
        rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);
        if (rc < 0)
          CDBG_ERROR("%s: VIDIOC_S_CTRL failed %s\n",
            __func__, strerror(errno));

        while (1)  {
          rc = ioctl(eventfd, VIDIOC_DQEVENT, &v4l2_evt);
          if (rc < 0)
            CDBG_ERROR("%s: VIDIOC_DQEVENT failed %s\n",
              __func__, strerror(errno));
          ev = (mm_camera_event_t *)v4l2_evt.u.data;
          if (ev->event_type == MM_CAMERA_EVT_TYPE_CTRL &&
              ev->e.ctrl.evt == MM_CAMERA_CTRL_EVT_ZOOM_DONE) {
              CDBG("ZOOM in notification: %d\n", ev->e.ctrl.status);
              break;
          }
	    }
      rc = ioctl(camfd, VIDIOC_G_CROP, &crop);
      if (!rc) {
          CDBG("Crop Rect is (%d, %d), (%d, %d)\n", crop.c.left, crop.c.top,
            crop.c.width, crop.c.height);
          usleep(200000);
        } else {
          CDBG_ERROR("VIDIOC_G_CROP failed %s\n", strerror(errno));
        }
      }
      break;
    default:
      CDBG("%s: Invalid zoom_action_param value\n", __func__);
      return -EINVAL;
    }
    return rc;
}

/*===========================================================================
 * FUNCTION     - set_iso -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_iso (int iso_action_param) {
    int rc = 0;
    struct v4l2_control ctrl;

    ctrl.id = MSM_V4L2_PID_ISO;
    ctrl.value = iso_action_param - 1;
    rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);

    return rc;
}

/*===========================================================================
 * FUNCTION     - increase_sharpness -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_sharpness () {
    int rc = 0;
    struct v4l2_control ctrl;

    sharpness += sharpness_queryctrl.step;
    if (sharpness > sharpness_queryctrl.maximum)
        sharpness = sharpness_queryctrl.maximum;

    ctrl.id = V4L2_CID_SHARPNESS;
    ctrl.value = sharpness;
    rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);

    return rc;
}

/*===========================================================================
 * FUNCTION     - decrease_sharpness -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_sharpness () {
    int rc = 0;
    struct v4l2_control ctrl;

    sharpness -= sharpness_queryctrl.step;
    if (sharpness < sharpness_queryctrl.minimum)
        sharpness = sharpness_queryctrl.minimum;

    ctrl.id = V4L2_CID_SHARPNESS;
    ctrl.value = sharpness;
    rc = ioctl(camfd, VIDIOC_S_CTRL, &ctrl);

    return rc;
}

/*===========================================================================
 * FUNCTION     - print_current_menu -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int print_current_menu (menu_id_change_t current_menu_id) {
  if (current_menu_id == MENU_ID_MAIN) {
    print_menu_preview_video ();
  } else if (current_menu_id == MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE) {
    camera_preview_video_resolution_change_tbl ();
  } else if (current_menu_id == MENU_ID_WHITEBALANCECHANGE) {
    camera_preview_video_wb_change_tbl();
  } else if (current_menu_id == MENU_ID_EXPMETERINGCHANGE) {
    camera_preview_video_exp_metering_change_tbl();
  } else if (current_menu_id == MENU_ID_GET_CTRL_VALUE) {
    camera_preview_video_get_ctrl_value_tbl();
  } else if (current_menu_id == MENU_ID_ISOCHANGE) {
    camera_preview_video_iso_change_tbl();
  } else if (current_menu_id == MENU_ID_BRIGHTNESSCHANGE) {
    camera_brightness_change_tbl ();
  } else if (current_menu_id == MENU_ID_CONTRASTCHANGE) {
    camera_contrast_change_tbl ();
  } else if (current_menu_id == MENU_ID_EVCHANGE) {
    camera_EV_change_tbl ();
  } else if (current_menu_id == MENU_ID_SATURATIONCHANGE) {
    camera_saturation_change_tbl ();
  } else if (current_menu_id == MENU_ID_ZOOMCHANGE) {
    camera_preview_video_zoom_change_tbl();
  } else if (current_menu_id == MENU_ID_SHARPNESSCHANGE) {
    camera_preview_video_sharpness_change_tbl();
  } else if (current_menu_id == MENU_ID_LUMACHANGE) {
    camera_la_table();
  } else if (current_menu_id == MENU_ID_BESTSHOT) {
    camera_set_bestshot_tbl();
  } else if (current_menu_id == MENU_ID_BL_SNOW) {
    camera_bl_n_snow_detection_tbl();
  } else if (current_menu_id == MENU_ID_HDR_CHANGE) {
    camera_hdr_mode_tbl();
  }
  return 0;
}

