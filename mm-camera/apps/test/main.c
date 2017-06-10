/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
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
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <dlfcn.h>

#include "camera.h"
#include "camera_dbg.h"
#include "cam_mmap.h"
#include "jpeg_encoder.h"
#include "common_cam.h"
#include "camaf_ctrl.h"
#include "liveshot.h"
#include "mm_camera_interface.h"

#include "testsuite.h"

#include "display_dimensions.h"

#define DUMP_SNAPSHOT_YUV
#define VIDEO_BUFFER_SIZE       (PREVIEW_WIDTH * PREVIEW_HEIGHT * 3/2)
#define THUMBNAIL_BUFFER_SIZE   (THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 3/2)
#define SNAPSHOT_BUFFER_SIZE    (PICTURE_WIDTH * PICTURE_HEIGHT * 3/2)

/*===========================================================================
 * Macro
 *===========================================================================*/
#define PREVIEW_FRAMES_NUM  4
#define VIDEO_FRAMES_NUM  4
#define BUFF_SIZE_100 100
#define BUFF_SIZE_20 20

/*===========================================================================
 * Defines
 *===========================================================================*/
typedef struct {
  Camera_main_menu_t main_menu;
  char * menu_name;
} CAMERA_MAIN_MENU_TBL_T;

const CAMERA_MAIN_MENU_TBL_T camera_main_menu_tbl[] = {
  {TAKE_YUV_PICTURE,              "Take a YUV snapshot without JPEG encoding."},
  {TAKE_JPEG_PICTURE,             "Take a JPEG snapshot."},
  {STOP_CAMERA,                   "Stop preview/video and exit camera."},
  {ZOOM_GOTO_SUBMENU,             "Smooth zoom changes."},
  {CONTRAST_GOTO_SUBMENU,         "Contrast changes."},
  {SATURATION_GOTO_SUBMENU,       "Saturation changes."},
  {SPECIAL_EFFECT,                "Toggle special effect mode. Default Off."},
  {BRIGHTNESS_GOTO_SUBMENU,       "Brightness changes."},
  {EV_GOTO_SUBMENU,               "EV changes."},
  {ANTI_BANDING,                  "Set Anti Flicker in sequence: Off/50Hz/60Hz. Default Off."},
  {SET_WHITE_BALANCE,             "Set Auto White Balance in sequence: Auto/Daylight/Incandescent/Fluorescent. Default Auto."},
  {AEC_MODE,                      "Toggle exposure mode(Average/Weighted/Metering). Default Weighted"},
  {ISO_GOTO_SUBMENU,              "ISO changes."},
  {TAKE_RAW_PICTURE,              "Take Raw Picture."},
  {SHARPNESS_GOTO_SUBMENU,        "Sharpness changes."},
  {SET_HJR,                       "HJR changes:"},
  {SET_AUTO_FOCUS,                "Do Auto focus - Macro/Normal."},
  {SET_LENS_SHADING,              "Lens Shading:"},
  {SET_LED_MODE,                  "Toggle LED mode(Off/Auto/On/Torch). Default Off."},
  {GET_SHARPNESS_AF,              "AF sharpness."},
  {PREVIEW_VIDEO_RESOLUTION,      "Preview/Video Resolution: SQCIF/QCIF/QVGA/CIF/VGA/WVGA... Default WVGA."},
  {SNAPSHOT_RESOLUTION,           "SnapShot Resolution: MP1/MP2/MP3/MP5/MP12... Default 1MP."},
  {MOTION_ISO,                    "Toggle motion iso (Off/On). Default On."},
  {TOGGLE_HUE,                    "Toggle Hue (0/60/120/180/240/300). Default 0."},
  {CANCEL_AUTO_FOCUS,             "Cancel Auto Focus."},
  {GET_AF_STEP,                   "Get AF step."},
  {SET_AF_STEP,                   "Set AF step."},
  {ENABLE_AFD,                    "Enable AFD."},
  {SET_FPS_MODE,                  "Auto/Fix FPS."},
  {SET_FPS,                       "Set FPS."},
  {START_RECORDING,               "Start Video Recording."},
  {STOP_RECORDING,                "Stop Video Recording."},
  {LUMA_ENABLE_DISABLE,           "LUMA Enable/Disable."},
  {ENABLE_CAF,                    "Enable CAF.(This feature is enabled only during video recording)"},
  {AEC_ROI_GOTO_SUBMENU,          "Simulate touch AEC - Enable ROI based AEC and test starting from region index 0. Default Off."},
  {SET_BL_N_SNOW_DETECTION,       "Set Backlight or Snow Scene Detection."},
  {TOGGLE_JPEG_ROTATION,          "Toggle Jpeg rotation 0/90/180/270... Default 0."},
  {SET_BESTSHOT_MODE,             "Set Best Shot mode in sequence"},
  {SET_AF_ROI,                    "Set AF ROI for Touch AF / Face detection AF."},
  {VIDEO_DIS_CONFIG,              "Video DIS configuration"},
  {VIDEO_ROT_CONFIG,              "Video Rotation configuration"},
  {START_LIVESNAPSHOT,            "Start live snapshot."},
  {SET_FOCUS_RECTANGLE,           "Set selectable zone AF. Spot/Center-weighted/Average/Auto"},
  {SCE_ADJUST,                    "Skin Color Enhancement Adjustment "},
  {TOGGLE_STROBE_FLASH_MODE,      "Toggle strobe flash mode OFF/ON... Default OFF."},
  {TOGGLE_FACE_DETECTION,         "Toggle Face Detection... Default 0(Off)."},
  {QUERY_FLASH4SNAP,              "Query is flash is needed for snapshot (0:No, 1:Strb, 2:LED)."},
  {GET_FOCUS_DISTANCES,           "Get Focus Distances."},
  {START_ZSL,                     "Start zero shutter lag."},
  {STOP_ZSL,                      "Stop zero shutter lag"},
  {CAPTURE_ZSL,                   "Start zero shutter lag capture"},
  {GET_FOCAL_LENGTH,              "Get Focal Length"},
  {GET_HORIZONTAL_VIEW_ANGLE,     "Get Horizontal View Angle"},
  {GET_FOCAL_LENGTH,              "Get Vertical View Angle"},
  {TOGGLE_CAMERA_MODE,            "Toggle Camera Mode 2D/3D."},
  {RESET_LENS_TO_INFINITY,        "Reset Focus to infinity"},
  {GET_SNAPSHOT_DATA,             "Get snapshot data"},
  {STEREO_DISPLAY_DISTANCE,       "Set 3D display distance"},
  {STEREO_VIEW_ANGLE,             "Set 3D view angle"},
  {UPDATE_PREVIEW_FORMAT,         "Set preview format"},
  {STEREO_CONV_CONTROL,           "Modify 3D convergence control settings"},
};

typedef struct {
  Camera_Resolution cs_id;
  uint16_t width;
  uint16_t  height;
  char * name;
  char * str_name;
} SNAPSHOT_DIMENSION_TBL_T;

const SNAPSHOT_DIMENSION_TBL_T snapshot_dimension_tbl[] = {
  { SQCIF,  SQCIF_WIDTH,  SQCIF_HEIGHT,  "SQCIF",         "SnapShot Resolution: SQCIF <128x96>"},
  {  QCIF,   QCIF_WIDTH,   QCIF_HEIGHT,   "QCIF",         "SnapShot Resolution: QCIF <176x144>"},
  {  QVGA,   QVGA_WIDTH,   QVGA_HEIGHT,   "QVGA",         "SnapShot Resolution: QVGA <320x240>"},
  {   CIF,    CIF_WIDTH,    CIF_HEIGHT,    "CIF",          "SnapShot Resolution: CIF <352x288>"},
  {   VGA,    VGA_WIDTH,    VGA_HEIGHT,    "VGA",          "SnapShot Resolution: VGA <640x480>"},
  {  WVGA,   WVGA_WIDTH,   WVGA_HEIGHT,   "WVGA",         "SnapShot Resolution: WVGA <800x480>"},
  {  SVGA,   SVGA_WIDTH,   SVGA_HEIGHT,   "SVGA",         "SnapShot Resolution: SVGA <800x600>"},
  {   XGA,    XGA_WIDTH,    XGA_HEIGHT,    "XGA",         "SnapShot Resolution: XGA <1024x768>"},
  { HD720,  HD720_WIDTH,  HD720_HEIGHT,  "HD720",       "SnapShot Resolution: HD720 <1280x720>"},
  {  WXGA,   WXGA_WIDTH,   WXGA_HEIGHT,   "WXGA",        "SnapShot Resolution: WXGA <1280x768>"},
  {   MP1,    MP1_WIDTH,    MP1_HEIGHT,    "1MP",         "SnapShot Resolution: MP1 <1280x960>"},
  {   MP2,    MP2_WIDTH,    MP2_HEIGHT,    "2MP", "SnapShot Resolution: MP2 (UXGA) <1600x1200>"},
  {HD1080, HD1080_WIDTH, HD1080_HEIGHT, "HD1080",     "SnapShot Resolution: HD1080 <1920x1080>"},
  {   MP3,    MP3_WIDTH,    MP3_HEIGHT,    "3MP", "SnapShot Resolution: MP3 (QXGA) <2048x1536>"},
  {   MP5,    MP5_WIDTH,    MP5_HEIGHT,    "5MP",        "SnapShot Resolution: MP5 <2592x1944>"},
  {   MP12,  MP12_WIDTH,   MP12_HEIGHT,   "12MP",        "SnapShot Resolution:MP12 <4000x3000>"},
};

const SNAPSHOT_DIMENSION_TBL_T preview_video_dimension_tbl[] = {
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
  Camera_Zoom cz_id;
  char * zoom_name;
} CAMERA_ZOOM_TBL_T;

const CAMERA_ZOOM_TBL_T camera_zoom_tbl[] = {
  {     INCREASE_ZOOM,       "Increase smooth zoom."},
  {     DECREASE_ZOOM,       "Decrease smooth zoom."},
  {INCREASE_STEP_ZOOM,  "Increase step smooth zoom."},
  {DECREASE_STEP_ZOOM,  "Decrease step smooth zoom."},
  {     PRINT_MAXZOOM,             "Print max zoom."},
  {     PRINT_ZOOMRATIOS,       "Print zoom ratios."},
};

typedef struct {
  Camera_Luma cl_id;
  char * luma_name;
}CAMERA_LA_TBL_T;

typedef struct {
  Rot_Config cl_id;
  char * Conf_name;
}ROT_CONF_TBL_T;

typedef struct {
  Dist_Config ds_id;
  char *name;
}DIST_CONF_TBL_T;

typedef struct {
  Angle_Config an_id;
  char *name;
}ANGLE_CONF_TBL_T;

typedef struct {
  Stereo_Conv_Ctrl_Config an_id;
  char *name;
}ST_CONV_CTRL_TBL_T;

typedef struct {
  Dis_Config cl_id;
  char * Conf_name;
}DIS_CONF_TBL_T;

const CAMERA_LA_TBL_T camera_la_tbl[] = {
  {ENABLE_LA,  "Enable Luma Adaptation."},
  {DISABLE_LA, "Disable Luma Adaptation."},
};

const ROT_CONF_TBL_T rot_conf_tbl[] = {
  {ROT_0,   "No ROTATION."},
  {ROT_90,  "90 degree ROTATION."},
  {ROT_180, "180 degree ROTATION."},
  {ROT_270, "270 degree ROTATION."}
};

const DIST_CONF_TBL_T dist_conf_tbl[] = {
  {DIST_0,   "Zero Distance"},
  {DIST_10,   "10 meters"},
  {DIST_20,   "20 meters"},
  {DIST_30,   "30 meters"},
};

const ANGLE_CONF_TBL_T angle_conf_tbl[] = {
  {ANGLE_0,   "Zero degrees"},
  {ANGLE_30 ,  "30 degrees"},
  {ANGLE_60,   "60 degrees"},
  {ANGLE_90,   "90 degrees"},
};

const ST_CONV_CTRL_TBL_T st_conv_ctrl_tbl[] = {
  {ST_AUTO_CONV,   "Auto convergence"},
  {ST_MANUAL_CONV, "Manual convergence"},
  {ST_EXIT_CONV,   "Exit convergence settings"},
};

const DIS_CONF_TBL_T dis_conf_tbl[] = {
  {NO_DIS, "Disable DIS."},
  {EN_DIS, "Enable DIS."},
};

typedef struct {
  Camera_Sce csce_id;
  char * sce_name;
}CAMERA_SCE_TBL_T;

const CAMERA_SCE_TBL_T camera_sce_tbl[] = {
  {PALER_SKIN,        "Make the skin color paler"},
  {TANNER_SKIN,       "Make the skin color tanner"},
};

typedef struct {
  Camera_Contrast_changes cc_id;
  char * contrast_name;
} CAMERA_CONTRST_TBL_T;

const CAMERA_CONTRST_TBL_T camera_contrast_tbl[] = {
  {INC_CONTRAST, "Increase Contrast by one step."},
  {DEC_CONTRAST, "Decrease Contrast by one step."},
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
  Camera_Brightness_changes bc_id;
  char * brightness_name;
} CAMERA_BRIGHTNESS_TBL_T;

const CAMERA_BRIGHTNESS_TBL_T camera_brightness_tbl[] = {
  {INC_BRIGHTNESS, "Increase Brightness by one step."},
  {DEC_BRIGHTNESS, "Decrease Brightness by one step."},
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
  Camera_ISO_changes ic_id;
  char * ISO_name;
} CAMERA_ISO_TBL_T;

const CAMERA_ISO_TBL_T camera_ISO_tbl[] = {
  {INC_ISO, "Increase ISO."},
  {DEC_ISO, "Decrease ISO."},
};

typedef struct {
  Camera_Sharpness_changes bc_id;
  char * sharpness_name;
} CAMERA_SHARPNESS_TBL_T;

const CAMERA_SHARPNESS_TBL_T camera_sharpness_tbl[] = {
  {INC_SHARPNESS, "Increase Sharpness."},
  {DEC_SHARPNESS, "Decrease Sharpness."},
};

uint32_t camera_fps_tbl[] = {
  30,
  20,
  15,
  12,
  10,
  5,
  30 + (15<<16),
  20 + (15<<16),
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
  Camera_AECROI_changes rc_id;
  char * roiChange_name;
} CAMERA_AECROI_TBL_T;

const CAMERA_AECROI_TBL_T camera_aecroi_tbl[] = {
  {SET_ROI_INDEX,            "Set ROI index."},
  {SET_ROI_LOCATION_INDEX,   "Set ROI location."},
  {SET_ROI_DISABLE_INDEX,    "Disable Aec ROI."},
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

/*===========================================================================
 * Forward declarations
 *===========================================================================*/
int camera_testsuite_main(int argc, char *argv);
static int set_fps(uint32_t fps);
static int toggle_Strobe_Flash_Mode(void);
static int8_t register_buffers(int width, int height,
  uint32_t offset, int y_off, int fd, uint8_t * buf,
  int type, int8_t unregister, int8_t active);
/*===========================================================================
 * Static global variables
 *===========================================================================*/
struct sigaction sigact;
extern USER_INPUT_DISPLAY_T input_display;
extern void test_app_camframe_callback(struct msm_frame *frame);
static int camframe_status = 0;
static struct termios oldtio, newtio;

static struct msm_frame zsl_thumb_frame[MAX_SNAPSHOT_BUFFERS];
static struct msm_frame zsl_main_frame[MAX_SNAPSHOT_BUFFERS];
static uint32_t zsl_thumb_size;
static uint32_t zsl_main_size;
static uint32_t zsl_thumb_width, zsl_thumb_height;
static uint32_t zsl_picture_width, zsl_picture_height;
int8_t lens_shading_enabled = TRUE;
int8_t hjr_status = FALSE;
static int8_t external_yuv_snap_pp = FALSE;
static int8_t external_yuv_snap_pp_burst = TRUE;
static int8_t wait_for_capture_done;
extern int8_t dump_restart_flag;

#ifdef _ANDROID_
char *sdcard_path = "/data";
#else
char *sdcard_path = ".";
#endif

int ZoomDump = 0;
int recordingOn = 0;

int camfd = 0;
int main_ion_fd = 0;
int ctrlfd;
int interfaceType;
int preview_video_resolution_flag = 0;
int effect = CAMERA_EFFECT_OFF;
int brightness = CAMERA_DEF_BRIGHTNESS;
int contrast = CAMERA_DEF_CONTRAST;
int saturation = CAMERA_DEF_SATURATION;
int sharpness = CAMERA_DEF_SHARPNESS;
int32_t SCE_Adj = CAMERA_DEF_SCE_FACTOR;
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
unsigned char *thumbnail_buf = NULL, *main_img_buf = NULL, *raw_img_buf = NULL;
struct ion_allocation_data thumbnail_ion_alloc, main_ion_alloc, raw_ion_alloc;
struct ion_fd_data thumbnail_fd_data, main_fd_data, raw_fd_data;
int32_t *sharpness_AF = NULL;
int16_t *zoomRatios = NULL;
cam_ctrl_dimension_t *dimension = NULL;
cam_ctrl_dimension_t *zsl_dimension = NULL;
struct crop_info cropInfo;
common_crop_t cropInfo_s;
mm_camera_notify mCamNotify;
mm_camera_ops mCamOps;
mm_camera_config mCfgControl;
cam_3d_frame_format_t snapshot_format_3d;
cam_3d_frame_format_t video_format_3d;
camera_mode_t mode;
mm_camera_buffer_t output_buffer[MAX_SNAPSHOT_BUFFERS];
uint32_t g_num_captures;
uint32_t g_current_capture_count;
static int camera_mode;
static int prev_camera_mode;
static cam_frame_start_parms frame_parms;
camera_mode_t modes_supported;
camera_mode_t current_mode = CAMERA_MODE_2D;
uint32_t stereo_conv_range;
int8_t jps_format = 0;

interface_ctrl_t intrfcCtrl;
config3a_wb_t autoWB = CAMERA_WB_AUTO;
isp3a_af_mode_t af_mode = AF_MODE_NORMAL;
cam_af_focusrect_t afFocusRect = AUTO;

cam_af_ctrl_t af_ctrl;
camera_iso_mode_type iso = CAMERA_ISO_AUTO;
camera_antibanding_type antibanding = CAMERA_ANTIBANDING_OFF;
camera_auto_exposure_mode_type aec_mode = CAMERA_AEC_CENTER_WEIGHTED;
led_mode_t led_mode = LED_MODE_OFF;
motion_iso_t motion_iso = MOTION_ISO_OFF;
int32_t hue = CAMERA_DEF_HUE;
fps_mode_t fps_mode = FPS_MODE_AUTO;
caf_ctrl_t caf_ctrl = CAF_OFF;
strobe_flash_mode_t strobe_flash_mode = STROBE_FLASH_MODE_OFF;
int32_t jpeg_rotation_value = 0;
roi_info_t roi_ctrl;
uint8_t face_detection = 0;

int32_t position = 36, steps = -1;
Camera_Resolution Resolution;
int32_t g_camParmInfo_current_value = 0;
unsigned long preview_frames_buf;
extern void test_app_mmcamera_videoframe_callback(struct msm_frame *frame); // video_cam.c
extern void test_receive_shutter_callback(common_crop_t *crop);
extern int test_app_launch_video_frame_thread(void* parms); // video_cam.c
extern int test_app_release_video_frame_thread(void); // video_cam.c
extern int test_app_release_video(void);

static int8_t afctrl_join = FALSE;
void *libqcamera = NULL;

void  (**LINK_mmcamera_shutter_callback)(common_crop_t *crop);

/* Forward declarations */
int disable_la(void);
int set_af_focusrect(void);
int toggle_Jpeg_Rotation(void);
int startliveshot(void);
int video_dis_configuration(uint32_t);
int query_aec_flash_for_snapshot(void);

/* int set_snow_detection(uint8_t);
int set_bl_detection(uint8_t);
int enable_la(); */


struct msm_frame frames[PREVIEW_FRAMES_NUM], frame_holding;
struct msm_frame video_frames[VIDEO_FRAMES_NUM+VIDEO_FRAMES_NUM+1], video_frame_holding;
pthread_t frame_thread;

pthread_mutex_t snapshot_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t snapshot_cond = PTHREAD_COND_INITIALIZER;
static int snapshotindex = 0;
static int snapshotyuvindex = 0;
static int snapshotrawindex = 0;
mm_camera_ops_type_t current_ops_type;

static uint8_t* liveshot_ptr;

static struct fifo_queue g_busy_previewframe_queue =
{0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, "preview_busy_q"};

int8_t test_app_on_event(mm_camera_event* evt)
{
  int rc = 0;
  switch(evt->event_type) {
  case SNAPSHOT_DONE: {
    CDBG("%s: SNAPSHOT_DONE", __func__);
    if (CAMERA_OPS_CAPTURE == current_ops_type) {
      char file_name[BUFF_SIZE_100];
      snprintf(file_name, BUFF_SIZE_100, "%s/postview%d.yuv",
        sdcard_path, snapshotyuvindex);
      FILE *fp = fopen(file_name, "wb");
      if (fp && evt->event_data.yuv_frames[0]) {
        int size = dimension->ui_thumbnail_width * dimension->ui_thumbnail_height;
        char *postview_buf = (char *)evt->event_data.yuv_frames[0]->buffer;
        fwrite(postview_buf, 1,size, fp);
        fwrite(postview_buf + evt->event_data.yuv_frames[0]->planar1_off, 1,size/2, fp);
        fclose(fp);
      }
      snprintf(file_name, BUFF_SIZE_100, "%s/mainimg%d.yuv",
        sdcard_path, snapshotyuvindex);
      fp = fopen(file_name, "wb");
      if (fp && evt->event_data.yuv_frames[1]) {
        int size = dimension->orig_picture_dx * dimension->orig_picture_dy;
        char *snapshot_buf = (char *)evt->event_data.yuv_frames[1]->buffer;
        fwrite(snapshot_buf, 1,size, fp);
        fwrite(snapshot_buf + evt->event_data.yuv_frames[1]->planar1_off, 1,size/2, fp);
        fclose(fp);
        snapshotyuvindex++;
      }
    } else if (CAMERA_OPS_RAW_CAPTURE == current_ops_type) {
      char file_name[BUFF_SIZE_100];
      snprintf(file_name, BUFF_SIZE_100, "%s/mainimg%d.raw",
        sdcard_path, snapshotrawindex);
      FILE *fp = fopen(file_name, "wb");
      if (fp && evt->event_data.raw_frame) {
        fwrite((void *)evt->event_data.raw_frame->buffer, 1,
          raw_snapshot_buffer_size, fp);
        fclose(fp);
        snapshotrawindex++;
      }
    }
    if (wait_for_capture_done) {
      g_current_capture_count++;
      CDBG("%s: SNAPSHOT_DONE current_count %d num captures %d", __func__,
        g_current_capture_count, g_num_captures);
      if (g_current_capture_count >= g_num_captures) {
        pthread_cond_signal(&snapshot_cond);
      }
    }
    break;
  }
  case SNAPSHOT_FAILED: {
    CDBG("%s: SNAPSHOT_FAILED", __func__);
    break;
  }
  case JPEG_ENC_DONE: {
    CDBG("%s: JPEG_ENC_DONE size %d", __func__,
      evt->event_data.encoded_frame->filled_size);
    char file_name[BUFF_SIZE_100] = "";
    snprintf(file_name, BUFF_SIZE_100, "%s/snapshot%d.jpg",
      sdcard_path, snapshotindex);
    FILE *jpege_fout = fopen(file_name, "wb");
    if (jpege_fout) {
      fwrite(evt->event_data.encoded_frame->ptr, 1,
        evt->event_data.encoded_frame->filled_size, jpege_fout);
      fclose(jpege_fout);
      snapshotindex++;
    }

    g_current_capture_count++;
    CDBG("%s: JPEG_ENC_DONE current_count %d num captures %d", __func__,
      g_current_capture_count, g_num_captures);
    if (g_current_capture_count >= g_num_captures) {
      pthread_cond_signal(&snapshot_cond);
    }
    break;
  }
  case JPEG_ENC_FAILED: {
    CDBG("%s: JPEG_ENC_FAILED", __func__);
    pthread_cond_signal(&snapshot_cond);
    break;
  }
  default: {
    CDBG("%s: invalid message type", __func__);
    rc = -1;
    break;
  }
  }
  return rc;
}


void test_app_liveshot_done(liveshot_status status, uint32_t jpeg_size)
{
  CDBG("test_app_liveshot_done status %d jpeg_size %u liveshot_ptr %p\n", status, jpeg_size, liveshot_ptr);
  if (LIVESHOT_SUCCESS == status) {
    static int ls_count = 0;
    char filename[BUFF_SIZE_100];
    snprintf(filename, BUFF_SIZE_100,
      "/data/liveshot/liveshot%d.jpg", ls_count);
    FILE* fp = fopen(filename,"w+");
    if (fp) {
      int size_written = fwrite(liveshot_ptr, 1, jpeg_size, fp);
      CDBG("test_app_liveshot_done size written %d", size_written);
      fclose(fp);
      ls_count++;
    }
    if (liveshot_ptr) {
      free(liveshot_ptr);
    }
  }
}

void test_app_camframe_error_callback(camera_error_type err)
{
  if ((err == CAMERA_ERROR_TIMEOUT) || (err == CAMERA_ERROR_ESD)) {
    camframe_status = -1;
  }
}

void test_app_mmcamera_videoframe_return(struct msm_frame * p)
{
  CDBG("!!test_app_mmcamera_videoframe_return p = %x\n",(unsigned int)p);
  // get free video frame
  if (p) {
    camframe_add_frame(CAM_VIDEO_FRAME, p);
  }
  return;
}
/*===========================================================================
 * FUNCTION    - keypress_to_event -
 *
 * DESCRIPTION:
 *==========================================================================*/
int keypress_to_event(char keypress)
{
  char out_buf = INVALID_KEY_PRESS;
  if (keypress >= 'A' && keypress <= 'Z') {
    out_buf = keypress - 'A' + 1;
  } else if (keypress >= '1' && keypress <= '9') {
    out_buf = keypress - '1' + BASE_OFFSET_NUM;
  } else if (keypress >= 'a' && keypress <= 'z') {
    out_buf = keypress - 'a' + BASE_OFFSET_NUM + 9;
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
      next_menu_id = MENU_ID_BL_N_SNOW_SET;
      break;
  }
  return next_menu_id;
}

int next_menu(menu_id_change_t current_menu_id, char keypress,
  camera_action_t * action_id_ptr, int * action_param)
{
  char output_to_event;
  menu_id_change_t next_menu_id = MENU_ID_INVALID;
  * action_id_ptr = ACTION_NO_ACTION;

  output_to_event = keypress_to_event(keypress);
  switch (current_menu_id) {
    case MENU_ID_MAIN:
      switch (output_to_event) {
        case TAKE_YUV_PICTURE:
          * action_id_ptr = ACTION_TAKE_YUV_PICTURE;
          break;

        case TAKE_JPEG_PICTURE:
          * action_id_ptr = ACTION_TAKE_JPEG_PICTURE;
          break;

        case STOP_CAMERA:
          * action_id_ptr = ACTION_STOP_CAMERA;
          break;

        case START_RECORDING:
          * action_id_ptr = ACTION_START_RECORDING;
          break;

        case STOP_RECORDING:
          * action_id_ptr = ACTION_STOP_RECORDING;
          break;

        case ZOOM_GOTO_SUBMENU:
          next_menu_id = MENU_ID_ZOOMCHANGE;
          break;

        case LUMA_ENABLE_DISABLE:
          next_menu_id = MENU_ID_LUMACHANGE;
          break;

        case VIDEO_DIS_CONFIG:
          if (recordingOn) {
            printf("\nCannot change DIS while recording is On.\n");
            next_menu_id = MENU_ID_MAIN;
            break;
          }
          next_menu_id = MENU_ID_VIDEO_DIS_CONFIG;
          break;

        case VIDEO_ROT_CONFIG:
          if (recordingOn) {
            printf("\nCannot change Rotation while recording is On.\n");
            next_menu_id = MENU_ID_MAIN;
            break;
          }
          next_menu_id = MENU_ID_VIDEO_ROT_CONFIG;
          break;

        case SCE_ADJUST:
          next_menu_id = MENU_ID_SCECHANGE;
          break;

        case SPECIAL_EFFECT:
          * action_id_ptr = ACTION_SPECIAL_EFFECT;
          break;

        case CONTRAST_GOTO_SUBMENU:
          next_menu_id = MENU_ID_CONTRASTCHANGE;
          break;

        case SATURATION_GOTO_SUBMENU:
          next_menu_id = MENU_ID_SATURATIONCHANGE;
          break;

        case BRIGHTNESS_GOTO_SUBMENU:
          next_menu_id = MENU_ID_BRIGHTNESSCHANGE;
          break;

        case EV_GOTO_SUBMENU:
          next_menu_id = MENU_ID_EVCHANGE;
          break;

        case ANTI_BANDING:
          * action_id_ptr = ACTION_ANTI_BANDING;
          break;

        case SET_WHITE_BALANCE:
          * action_id_ptr = ACTION_SET_WHITE_BALANCE;
          break;

        case AEC_MODE:
          * action_id_ptr = ACTION_AEC_MODE;
          break;

        case AEC_ROI_GOTO_SUBMENU:
          next_menu_id = MENU_ID_AECROICHANGE;
          break;

        case ISO_GOTO_SUBMENU:
          next_menu_id = MENU_ID_ISOCHANGE;
          break;

        case TAKE_RAW_PICTURE:
          * action_id_ptr = ACTION_TAKE_RAW_PICTURE;
          break;

        case SHARPNESS_GOTO_SUBMENU:
          next_menu_id = MENU_ID_SHARPNESSCHANGE;
          break;

        case SET_HJR:
          * action_id_ptr = ACTION_SET_HJR;
          break;

        case SET_AUTO_FOCUS:
          * action_id_ptr = ACTION_SET_AUTO_FOCUS;
          break;

        case SET_LENS_SHADING:
          * action_id_ptr = ACTION_SET_LENS_SHADING;
          break;

        case SET_LED_MODE:
          * action_id_ptr = ACTION_SET_LED_MODE;
          break;

        case GET_SHARPNESS_AF:
          * action_id_ptr = ACTION_GET_SHARPNESS_AF;
          break;

        case PREVIEW_VIDEO_RESOLUTION:
          next_menu_id = MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE;
          break;

        case SNAPSHOT_RESOLUTION:
          next_menu_id = MENU_ID_SNAPSHOTRESOLUTIONCHNAGE;
          break;

        case MOTION_ISO:
          * action_id_ptr = ACTION_MOTION_ISO;
          break;

        case TOGGLE_HUE:
          *action_id_ptr = ACTION_TOGGLE_HUE;
          break;

        case CANCEL_AUTO_FOCUS:
          *action_id_ptr = ACTION_CANCEL_AUTO_FOCUS;
          break;

        case GET_AF_STEP:
          *action_id_ptr = ACTION_GET_AF_STEP;
          break;

        case SET_AF_STEP:
          *action_id_ptr = ACTION_SET_AF_STEP;
          break;

        case ENABLE_AFD:
          *action_id_ptr = ACTION_ENABLE_AFD;
          break;

        case SET_FPS_MODE:
          *action_id_ptr = ACTION_SET_FPS_MODE;
          break;

        case SET_FPS:
          next_menu_id = MENU_ID_FPSCHANGE;
          break;

        case SET_FOCUS_RECTANGLE:
          *action_id_ptr = ACTION_SET_AF_FOCUSRECT;
          break;


        case SET_BESTSHOT_MODE:
          next_menu_id = MENU_ID_SET_BESTSHOT_MODE;
          break;
        case ENABLE_CAF:
          *action_id_ptr = ACTION_ENABLE_CAF;
          break;

        case SET_BL_N_SNOW_DETECTION:
          next_menu_id = MENU_ID_BL_N_SNOW_SET;
          break;

        case TOGGLE_STROBE_FLASH_MODE:
          *action_id_ptr = ACTION_TOGGLE_STROBE_FLASH_MODE;
          break;

        case TOGGLE_JPEG_ROTATION:
          *action_id_ptr = ACTION_TOGGLE_JPEG_ROTATION;
          break;

        case TOGGLE_CAMERA_MODE:
          *action_id_ptr = ACTION_TOGGLE_CAMERA_MODE;
          break;

        case TOGGLE_FACE_DETECTION:
          *action_id_ptr = ACTION_TOGGLE_FACE_DETECTION;
          break;

        case SET_AF_ROI:
          *action_id_ptr = ACTION_SET_AF_ROI;
          break;

        case START_LIVESNAPSHOT:
          *action_id_ptr = ACTION_START_LIVESHOT;
          break;

      case QUERY_FLASH4SNAP:
        *action_id_ptr = ACTION_QUERY_FLASH_4_SNAP;
        break;

        case GET_FOCUS_DISTANCES:
          *action_id_ptr = ACTION_GET_FOCUS_DISTANCES;
          break;

        case START_ZSL:
          *action_id_ptr = ACTION_START_ZSL;
           break;

        case STOP_ZSL:
          *action_id_ptr = ACTION_STOP_ZSL;
           break;

        case CAPTURE_ZSL:
          *action_id_ptr = ACTION_CAPTURE_ZSL;
          break;

        case GET_FOCAL_LENGTH:
            *action_id_ptr = ACTION_GET_FOCAL_LENGTH;
            break;

        case GET_HORIZONTAL_VIEW_ANGLE:
            *action_id_ptr = ACTION_GET_HORIZONTAL_VIEW_ANGLE;
            break;

        case GET_VERTICAL_VIEW_ANGLE:
            *action_id_ptr = ACTION_GET_VERTICAL_VIEW_ANGLE;
            break;

        case RESET_LENS_TO_INFINITY:
            *action_id_ptr = ACTION_RESET_LENS_TO_INFINITY;
            break;

        case GET_SNAPSHOT_DATA:
            *action_id_ptr = ACTION_GET_SNAPSHOT_DATA;
            break;

        case STEREO_DISPLAY_DISTANCE:
          next_menu_id = MENU_ID_3D_DISPLAY_DISTANCE;
          break;

        case STEREO_VIEW_ANGLE:
          next_menu_id = MENU_ID_3D_VIEW_ANGLE;
          break;

        case STEREO_CONV_CONTROL:
          next_menu_id = MENU_ID_3D_CONV_CONTROL;
          break;

        case UPDATE_PREVIEW_FORMAT:
          *action_id_ptr = ACTION_UPDATE_PREVIEW_FORMAT;
          break;

        default:
          next_menu_id = MENU_ID_MAIN;
          break;
      }
      break;

    case MENU_ID_ZOOMCHANGE:
      switch (output_to_event) {
        case INCREASE_ZOOM:
          * action_id_ptr = ACTION_ZOOM_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case DECREASE_ZOOM:
          * action_id_ptr = ACTION_ZOOM_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case INCREASE_STEP_ZOOM:
          * action_id_ptr = ACTION_ZOOM_STEP_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case DECREASE_STEP_ZOOM:
          * action_id_ptr = ACTION_ZOOM_STEP_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case PRINT_MAXZOOM:
          * action_id_ptr = ACTION_PRINT_MAXZOOM;
          next_menu_id = MENU_ID_MAIN;
          break;

        case PRINT_ZOOMRATIOS:
          * action_id_ptr = ACTION_PRINT_ZOOMRATIOS;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_ZOOMCHANGE;
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
    case MENU_ID_VIDEO_ROT_CONFIG:
      switch (output_to_event) {
        case ROT_0:
          * action_id_ptr = ACTION_VPE_ROT_0;
          next_menu_id = MENU_ID_MAIN;
          break;
        case ROT_90:
          * action_id_ptr = ACTION_VPE_ROT_90;
          next_menu_id = MENU_ID_MAIN;
          break;
        case ROT_180:
          * action_id_ptr = ACTION_VPE_ROT_180;
          next_menu_id = MENU_ID_MAIN;
          break;
        case ROT_270:
          * action_id_ptr = ACTION_VPE_ROT_270;
          next_menu_id = MENU_ID_MAIN;
          break;
        default:
          next_menu_id = MENU_ID_VIDEO_ROT_CONFIG;
          break;
      }
      break;
    case MENU_ID_VIDEO_DIS_CONFIG:
      switch (output_to_event) {
        case NO_DIS:
          * action_id_ptr = ACTION_VPE_NO_DIS;
          next_menu_id = MENU_ID_MAIN;
          break;
        case EN_DIS:
          * action_id_ptr = ACTION_VPE_EN_DIS;
          next_menu_id = MENU_ID_MAIN;
          break;
        default:
          next_menu_id = MENU_ID_VIDEO_DIS_CONFIG;
          break;
      }
      break;

    case MENU_ID_SCECHANGE:
      switch (output_to_event) {
        case PALER_SKIN:
          * action_id_ptr = ACTION_PALER_SKIN;
          next_menu_id = MENU_ID_MAIN;
          break;

        case TANNER_SKIN:
          * action_id_ptr = ACTION_TANNER_SKIN;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_SCECHANGE;
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
          next_menu_id = MENU_ID_SATURATIONCHANGE;
          break;
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

    case MENU_ID_ISOCHANGE:
      switch (output_to_event) {
        case INC_ISO:
          * action_id_ptr = ACTION_ISO_INCREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        case DEC_ISO:
          * action_id_ptr = ACTION_ISO_DECREASE;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_ISOCHANGE;
          break;
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

    case MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE:
      * action_id_ptr = ACTION_PREVIEW_VIDEO_RESOLUTION;
      if (output_to_event > RESOLUTION_PREVIEW_VIDEO_MAX ||
        output_to_event < RESOLUTION_MIN) {
        CDBG("Please select correct resolution");
        next_menu_id = current_menu_id;
      } else {
        next_menu_id = MENU_ID_MAIN;
        * action_param = output_to_event;
      }
      break;

    case MENU_ID_SNAPSHOTRESOLUTIONCHNAGE:
      * action_id_ptr = ACTION_SNAPSHOT_RESOLUTION;
      if (output_to_event > RESOLUTION_MAX || output_to_event < RESOLUTION_MIN) {
        next_menu_id = current_menu_id;
      } else {
        next_menu_id = MENU_ID_MAIN;
        * action_param = output_to_event;
      }
      break;

    case MENU_ID_FPSCHANGE:
      if (output_to_event > 0 &&
        output_to_event <= sizeof(camera_fps_tbl)/sizeof(camera_fps_tbl[0])) {
        * action_id_ptr = ACTION_SET_FPS;
        * action_param = camera_fps_tbl[output_to_event-1];
      }
      next_menu_id = MENU_ID_MAIN;
      break;
    case MENU_ID_SET_BESTSHOT_MODE:
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

    case MENU_ID_AECROICHANGE:
      switch (output_to_event) {
        case SET_ROI_INDEX:
          * action_id_ptr = ACTION_ROI_SET_INDEX;
          next_menu_id = MENU_ID_MAIN;
          break;

        case SET_ROI_LOCATION_INDEX:
          printf("DECREASE_STEP_ROI_INDEX\n");
          * action_id_ptr = ACTION_ROI_SET_LOCATION;
          next_menu_id = MENU_ID_MAIN;
          break;

        case SET_ROI_DISABLE_INDEX:
          printf("Disable aec ROI\n");
          * action_id_ptr = ACTION_ROI_DISABLE;
          next_menu_id = MENU_ID_MAIN;
          break;

        default:
          next_menu_id = MENU_ID_AECROICHANGE;
          break;
      }
      break;

    case MENU_ID_BL_N_SNOW_SET:
      next_menu_id = bl_n_snow_detection_change_menu_process(
        output_to_event,
        action_id_ptr);
      break;

    case MENU_ID_3D_DISPLAY_DISTANCE:
      switch (output_to_event) {
        case DIST_0:
          * action_id_ptr = ACTION_3D_DIST_0;
          next_menu_id = MENU_ID_MAIN;
          break;
        case DIST_10:
          * action_id_ptr = ACTION_3D_DIST_10;
          next_menu_id = MENU_ID_MAIN;
          break;
        case DIST_20:
          * action_id_ptr = ACTION_3D_DIST_20;
          next_menu_id = MENU_ID_MAIN;
          break;
        case DIST_30:
          * action_id_ptr = ACTION_3D_DIST_30;
          next_menu_id = MENU_ID_MAIN;
          break;
        default:
          next_menu_id = MENU_ID_3D_DISPLAY_DISTANCE;
          break;
      }
      break;

    case MENU_ID_3D_VIEW_ANGLE:
      switch (output_to_event) {
        case ANGLE_0:
          * action_id_ptr = ACTION_3D_ANGLE_0;
          next_menu_id = MENU_ID_MAIN;
          break;
        case ANGLE_30:
          * action_id_ptr = ACTION_3D_ANGLE_30;
          next_menu_id = MENU_ID_MAIN;
          break;
        case ANGLE_60:
          * action_id_ptr = ACTION_3D_ANGLE_60;
          next_menu_id = MENU_ID_MAIN;
          break;
        case ANGLE_90:
          * action_id_ptr = ACTION_3D_ANGLE_90;
          next_menu_id = MENU_ID_MAIN;
          break;
        default:
          next_menu_id = MENU_ID_3D_VIEW_ANGLE;
          break;
      }
      break;

    case MENU_ID_3D_CONV_CONTROL:
      switch (output_to_event) {
        case ST_AUTO_CONV:
          * action_id_ptr = ACTION_3D_AUTO_CONV;
          next_menu_id = MENU_ID_3D_CONV_CONTROL;
          break;
        case ST_MANUAL_CONV:
          * action_id_ptr = ACTION_3D_MANUAL_CONV;
          next_menu_id = MENU_ID_3D_CONV_CONTROL;
          break;
        case ST_EXIT_CONV:
          next_menu_id = MENU_ID_MAIN;
          break;
        default:
          printf("\n\nInvalid selection\n");
          next_menu_id = MENU_ID_3D_CONV_CONTROL;
          break;
      }
      break;

    default:
      CDBG("menu id is wrong: %d\n", current_menu_id);
      break;
  }

  return next_menu_id;
}

/*===========================================================================
 * FUNCTION     - print_clear_term -
 *
 * DESCRIPTION:
 * ===========================================================================*/
static void print_clear_term(void)
{
  fprintf(stdout, "\033[2J");
  fprintf(stdout, "\033[1;1H");
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
    } else if (menuNum == '9' + 1) {
      menuNum = 'a';
    }

    if (i == SET_HJR) {
      if (hjr_status == TRUE)
        printf("    HJR is Enabled\n");
      else
        printf("    HJR is Disabled\n");
    }

    if (i == SET_LENS_SHADING) {
      if (lens_shading_enabled == TRUE)
        printf("    Lens Shading is Disable\n");
      else
        printf("    Lens Shading is Enable\n");
    }

    printf("%c.  %s\n", menuNum, camera_main_menu_tbl[i].menu_name);
    menuNum++;
  }

  printf("\nPlease enter your choice: ");

  return;
}

static void camera_snapshot_resolution_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in snapshot resolution mode       \n");
  printf("===========================================\n\n");

  char snapshotmenuNum = 'A';
  for (i = 0; i < sizeof(snapshot_dimension_tbl)/sizeof(snapshot_dimension_tbl[0]); i++) {
    printf("%c.  %s\n", snapshotmenuNum, snapshot_dimension_tbl[i].str_name);
    snapshotmenuNum++;
  }

  printf("\nPlease enter your choice for Snapshot Resolution: ");
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

char * set_snapshot_dimension_tbl(Camera_Resolution cs_id, uint16_t * width, uint16_t * height)
{
  unsigned int i;
  char * ptr = NULL;
  for (i = 0; i < sizeof(snapshot_dimension_tbl)/sizeof(snapshot_dimension_tbl[0]); i++) {
    if (cs_id == snapshot_dimension_tbl[i].cs_id) {
      if (snapshot_dimension_tbl[i].width > dimension->raw_picture_width ||
        snapshot_dimension_tbl[i].height > dimension->raw_picture_height) {
        printf("snapshot resolution input is wrong, fail to select!!!\n");
        printf("sensor doesn't support higher snapshot dimension\n");
      } else {
        *width = snapshot_dimension_tbl[i].width;
        *height = snapshot_dimension_tbl[i].height;
        ptr = snapshot_dimension_tbl[i].name;
      }
      break;
    }
  }
  return ptr;
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

static void camera_zoom_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in zoom change mode now       \n");
  printf("===========================================\n\n");

  char zoommenuNum = 'A';
  for (i = 0; i < sizeof(camera_zoom_tbl)/sizeof(camera_zoom_tbl[0]); i++) {
    printf("%c.  %s\n", zoommenuNum, camera_zoom_tbl[i].zoom_name);
    zoommenuNum++;
  }

  printf("\nPlease enter your choice for zoom changes: ");
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

static void video_dis_config_print() {
  uint32_t i;
  printf("\n");
  printf("===========================================\n");
  printf("             DIS Configuration             \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(dis_conf_tbl)/sizeof(dis_conf_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, dis_conf_tbl[i].Conf_name);
    submenuNum++;
  }
  printf("\nPlease enter your choice for DIS config: ");
  return;
}

static void video_rot_config_print() {
  uint32_t i;
  printf("\n");
  printf("===========================================\n");
  printf("     Video Rotation Configurations         \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(rot_conf_tbl)/sizeof(rot_conf_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, rot_conf_tbl[i].Conf_name);
    submenuNum++;
  }
  printf("\nPlease enter your choice for VPE Rotation: ");
  return;
}

static void camera_sce_table() {
  uint32_t i;
  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in SCE Adujstment mode now       \n");
  printf("===========================================\n\n");

  char scesubmenuNum = 'A';
  for (i = 0; i < sizeof(camera_sce_tbl)/sizeof(camera_sce_tbl[0]); i++) {
    printf("%c.  %s\n", scesubmenuNum, camera_sce_tbl[i].sce_name);
    scesubmenuNum++;
  }
  printf("\nPlease enter your choice for SCE Adjustment: ");
  return;
}

static void camera_contrast_change_tbl(void) {
  unsigned int i;
  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in contrast change mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_contrast_tbl)/sizeof(camera_contrast_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_contrast_tbl[i].contrast_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for contrast changes: ");
  return;
}

static void camera_saturation_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in saturation change mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_saturation_tbl)/sizeof(camera_saturation_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_saturation_tbl[i].saturation_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for saturation changes: ");
  return;
}

static void camera_brightness_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in brightness change mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_brightness_tbl)/sizeof(camera_brightness_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_brightness_tbl[i].brightness_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for brightness changes: ");
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

static void camera_ISO_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in ISO change mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_ISO_tbl)/sizeof(camera_ISO_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_ISO_tbl[i].ISO_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for ISO changes: ");
  return;
}

static void camera_sharpness_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in sharpness change mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_sharpness_tbl)/sizeof(camera_sharpness_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_sharpness_tbl[i].sharpness_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for sharpness changes: ");
  return;
}

static void camera_fps_change_tbl(void) {
  unsigned int i;
  uint16_t max_fps, min_fps;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in fps change mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_fps_tbl)/sizeof(camera_fps_tbl[0]); i++) {
    max_fps = (uint16_t)(camera_fps_tbl[i] & 0x0000ffff);
    min_fps = (uint16_t)((camera_fps_tbl[i]>>16) & 0x0000ffff);
    if (min_fps != 0)
      printf("%c.  %d -- %d max--min FPS\n", submenuNum, max_fps, min_fps);
    else
      printf("%c.  %d FPS\n", submenuNum, max_fps);
    submenuNum++;
  }

  printf("\nPlease enter your choice for fps changes: ");
  return;
}

static void camera_aecroi_change_tbl(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in touch AEC test mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_aecroi_tbl)/sizeof(camera_aecroi_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_aecroi_tbl[i].roiChange_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for AEC ROI changes: ");
  return;
}

static void camera_set_bestshot_mode(void) {
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in bestshot mode now       \n");
  printf("===========================================\n\n");

  char submenuNum = 'A';
  for (i = 0; i < sizeof(camera_aecroi_tbl)/sizeof(camera_aecroi_tbl[0]); i++) {
    printf("%c.  %s\n", submenuNum, camera_aecroi_tbl[i].roiChange_name);
    submenuNum++;
  }

  printf("\nPlease enter your choice for AEC ROI changes: ");
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

static void camera_3d_display_distance(void)
{
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in 3D mode display distance now \n");
  printf("===========================================\n\n");


  char DsmenuNum = 'A';
  for (i = 0; i < sizeof(dist_conf_tbl)/sizeof(dist_conf_tbl[0]); i++) {
    printf("%c.  %s\n", DsmenuNum,
      dist_conf_tbl[i].name);
    DsmenuNum++;
  }

  printf("\nPlease enter your choice of 3D display distance: ");
  return;
}

static void camera_3d_angle_of_view(void)
{
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf("      Camera is in 3D mode view angle now       \n");
  printf("===========================================\n\n");


  char AnmenuNum = 'A';
  for (i = 0; i < sizeof(angle_conf_tbl)/sizeof(angle_conf_tbl[0]); i++) {
    printf("%c.  %s\n", AnmenuNum,
      angle_conf_tbl[i].name);
    AnmenuNum++;
  }

  printf("\nPlease enter your choice of 3D view angle: ");
  return;
}

static void camera_3d_conv_control(void)
{
  unsigned int i;

  printf("\n");
  printf("===========================================\n");
  printf(" Camera is in 3D convergence control menu  \n");
  printf("===========================================\n\n");


  char AnmenuNum = 'A';
  for (i = 0; i < sizeof(st_conv_ctrl_tbl)/sizeof(st_conv_ctrl_tbl[0]); i++) {
    printf("%c.  %s\n", AnmenuNum,
      st_conv_ctrl_tbl[i].name);
    AnmenuNum++;
  }

  printf("\nPlease enter your choice of 3D convergence control: ");
  return;
}

static int submain();


/*===========================================================================
 * FUNCTION    - afStatus_ctrl -
 *
 * DESCRIPTION:
 *==========================================================================*/
void afStatus_ctrl(int8_t status)
{
  switch (status) {
    case CAMERA_EXIT_CB_DONE:
      printf("\n%s: AF is sucessful\n", __func__);
      break;

    case CAMERA_EXIT_CB_FAILED:
      printf("\n%s: AF is failed or rejected\n", __func__);
      break;

    case CAMERA_EXIT_CB_ABORT:
      printf("\n%s: AF is aborted\n", __func__);
      break;

    default:
      printf("\n%s: Invalid AF status\n", __func__);
      break;
  }
}

/*===========================================================================
 * FUNCTION    - cam_pos_to_string -
 *
 * DESCRIPTION:
 *==========================================================================*/
char* cam_pos_to_string(cam_position_t pos)
{
  switch(pos) {
  case FRONT_CAMERA:
    return "front camera";
  case BACK_CAMERA:
    return "back camera";
  }
  return "none";
}

/*===========================================================================
 * FUNCTION    - start_camera_interface -
 *
 * DESCRIPTION:
 *==========================================================================*/
static mm_camera_status_t start_camera_interface()
{
  camera_info_t* p_camera_info = NULL;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int num_cameras = 0;
  int i = 0;
  camera_mode_t mode;
  int user_input = 1;
  char input[BUFF_SIZE_20];
  status = mCfgControl.mm_camera_query_parms (CAMERA_PARM_CAMERA_INFO,
    (void **)&p_camera_info, (void *)&num_cameras);
  if (status)
    goto ERROR;

  if (num_cameras > 1) {
    printf("\nSelect the camera");
    for (i=0; i<num_cameras; i++) {
      printf("\n%d. %s", i+1, cam_pos_to_string(p_camera_info[i].position));
    }
    fgets(input, BUFF_SIZE_20, stdin);
    user_input = atoi(input);
    if ((user_input > num_cameras) || (user_input < 1)){
      printf("\nInvalid entry selected selecting option 1 as default");
      user_input = 1;
    }
  }

  uint8_t deviceID = user_input-1;
  status = mCfgControl.mm_camera_set_parm (CAMERA_PARM_CAMERA_ID,
    (void *)&deviceID);
  if (status)
    goto ERROR;

  if (p_camera_info[deviceID].modes_supported & CAMERA_MODE_3D) {
    modes_supported = p_camera_info[deviceID].modes_supported;
    printf("\nSelect the camera mode\n1. 2D mode\n2. 3D mode\n");
    fgets(input, BUFF_SIZE_20, stdin);
    user_input = atoi(input);

    if (2 == user_input) {
      printf("\nSelected 3D mode");
      mode = CAMERA_MODE_3D;
    } else {
      printf("\nSelected 2D mode");
      mode = CAMERA_MODE_2D;
    }
    current_mode = mode;
    status = mCfgControl.mm_camera_set_parm(CAMERA_PARM_MODE,
      (void *)&mode);
    if (status)
      goto ERROR;
  }

  status = mm_camera_exec();
  if (status) {
    CDBG_ERROR("%s: mm_camera_exec failed:", __func__);
  } else
    CDBG("%s: succeeded: %d", __func__, status);

  if (CAMERA_MODE_3D == current_mode) {
      camera_3d_frame_t format;
      format.frame_type = CAM_VIDEO_FRAME;
      status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_3D_FRAME_FORMAT,
        (void *)&format);
      if (status)
        goto ERROR;
      video_format_3d = format.format;
      format.frame_type = CAM_SNAPSHOT_FRAME;
      status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_3D_FRAME_FORMAT,
        (void *)&format);
      if (status)
        goto ERROR;
      snapshot_format_3d = format.format;
      CDBG("%s: 3d format video %d snapshot %d", __func__, video_format_3d,
          snapshot_format_3d);
    }

  return status;

ERROR:
  CDBG_ERROR("%s: failed: %d", __func__, status);
  return status;
}
/*===========================================================================
 * FUNCTION    - main -
 *
 * DESCRIPTION:
 *==========================================================================*/
int main(int argc, char **argv)
{
  int keep_on_going = 1;
  int c;
  int run_testsuite = 0;
  char * autotest_script = NULL;
  char * server_ip = "127.0.0.1";
  char * server_port = "9009";
  char input[BUFF_SIZE_20];

#ifdef _ANDROID_
  libqcamera = dlopen("liboemcamera.so", RTLD_NOW);
#else
  libqcamera = dlopen("libmm-camera.so", RTLD_NOW);
#endif
  if (!libqcamera) {
    CDBG("FATAL ERROR: could not dlopen liboemcamera.so: %s", dlerror());
    return -1;
  }

  *(void **)&LINK_mmcamera_shutter_callback = dlsym(libqcamera, "mmcamera_shutter_callback");
  if (!LINK_mmcamera_shutter_callback) {
    CDBG("FATAL ERROR: could not dlsym mmcamera_shutter_callback");
    return -1;
  }
  *LINK_mmcamera_shutter_callback = test_receive_shutter_callback;

  interfaceType = NATIVE_CAMERA_INTERFACE;
  while ((c = getopt(argc, argv, "vhod:ta:p:f:")) != -1) {
    switch (c) {
      case 'f':
        autotest_script = optarg;

      case 't':
        run_testsuite = 1;
        break;

      case 'a':
        server_ip = optarg;
        break;

      case 'p':
        server_port = optarg;
        break;

      case 'd':
        sdcard_path = optarg;
        break;

      case 'o':
        use_overlay_fb_display_driver();
        break;

      case 'h':
      default:
        printf("usage: %s [-l|-h-t] [-d <dir name>] [-f <autotest.txt>] [-a <ip address>] [-p <port>]\n", argv[0]);
        printf("-t:   Using automation testsuite instead.\n");
        printf("-f:   Using automation test script file.\n");
        printf("-a:   server ip address. By default it is 127.0.0.1\n");
        printf("-p:   server port number. By default it is 9009\n");
        printf("-h:   Show this usage message.                   \n");
        printf("-d:   specify path to store file, by default the file'll be stored into /data (flash memory)\n");
        printf("-o:   use overlay fb display driver\n");
        printf("run adb port forwarding for android target:\n");
        printf("adb forward tcp:9009 tcp:9009\n");
        exit(0);
    }
  }

  CDBG("\nCamera Test Application\n");

  struct timeval tdBeforePreviewVideo, tdStopCamera;
  struct timezone tz;

  gettimeofday(&tdBeforePreviewVideo, &tz);

  CDBG("Profiling: Start Camera timestamp = %ld ms\n",
    (tdBeforePreviewVideo.tv_sec * 1000) + (tdBeforePreviewVideo.tv_usec/1000));

  switch (interfaceType) {
    case NATIVE_CAMERA_INTERFACE:
      CDBG("main: NATIVE_CAMERA_INTERFACE\n");
      break;

    default:
      break;
  }

  mCamNotify.on_event = test_app_on_event;
  mCamNotify.camstats_cb = NULL;
  mCamNotify.jpegfragment_cb = NULL;
  mCamNotify.on_error_event = test_app_camframe_error_callback;
  mCamNotify.on_jpeg_event = NULL;
  mCamNotify.preview_frame_cb = test_app_camframe_callback;
  mCamNotify.video_frame_cb = test_app_mmcamera_videoframe_callback;
  mCamNotify.on_liveshot_event = test_app_liveshot_done;
  if (MM_CAMERA_SUCCESS != mm_camera_init(&mCfgControl, &mCamNotify,
    &mCamOps, 1)) {
    CDBG_ERROR("main: mm_camera_init failed:");
    return FALSE;
  }
  if (MM_CAMERA_SUCCESS != start_camera_interface()) {
    CDBG_ERROR("main: start_camera_interface failed:");
    return FALSE;
  }
#ifdef USE_ION
  main_ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (main_ion_fd < 0) {
    CDBG_ERROR("Ion dev open failed\n");
    CDBG_ERROR("Error is %s\n", strerror(errno));
    return FALSE;
  }
#endif
  if (native_interface_init(&intrfcCtrl, &camfd) <= 0)
    return -1;

  if (run_testsuite != 0) {

    printf("server_ip: %s\n", server_ip);
    printf("server_port: %s\n", server_port);

    testsuite_server_manager(server_ip, server_port);

  } else {
    do {
      if (current_mode == CAMERA_MODE_3D) {
        camera_mode = 1; /* Video Mode only */
      } else {
        printf("\nSelect the camera mode\n1. Video mode \n2. ZSL mode\n");
        fgets(input, BUFF_SIZE_20, stdin);
        camera_mode = atoi(input);
      }
      keep_on_going = submain();
    } while (keep_on_going);
  }

  /* Clean up and exit. */
  CDBG("Exiting the app\n");


  if (interfaceType == NATIVE_CAMERA_INTERFACE) {
    if (MM_CAMERA_SUCCESS != mm_camera_deinit())
      CDBG("%s: mm_camera_deinit failure!\n", __func__);
    else {
      CDBG("%s: mm_camera_deinit success\n", __func__);
      if (MM_CAMERA_SUCCESS != mm_camera_destroy())
        CDBG("%s: mm_camera_destroy success", __func__);
    }
  }

  close(camfd);
#ifdef USE_ION
  close(main_ion_fd);
#endif

  gettimeofday(&tdStopCamera, &tz);
  CDBG("Profiling: Stop camera end timestamp = %ld ms\n",
    (tdStopCamera.tv_sec * 1000) + (tdStopCamera.tv_usec/1000));

  ERROR:
  if (libqcamera) {
    unsigned ref = dlclose(libqcamera);
    CDBG("dlclose(libqcamera) refcount %d", ref);
  }

  return 0;
}

/*===========================================================================
 * FUNCTION    - unreg_vidbuf -
 *
 * DESCRIPTION:
 *==========================================================================*/
int unreg_vidbuf(int BufCount, int vpe_en)
{
  uint32_t buff_size = 0;
  int cnt = 0;
  int rc = -1;
  uint32_t offset=0;

  /* free video buffer after frame thread exits. */
  buff_size = dimension->video_width * dimension->video_height * 3/2;
  if (!vpe_en) {
    for (cnt = 0; cnt < BufCount; ++cnt) {
      CDBG("unreg_vidbuf: unregisterVideoBuf %d\n", cnt);
      if (video_frames[cnt].buffer == 0)
        continue;

      offset = buff_size;
      intrfcCtrl.unregisterVideoBuf(camfd,
        dimension, offset, video_frames[cnt].fd, (uint8_t *) video_frames[cnt].buffer, 0);
      CDBG("unreg_vidbuf: do_munmap video buffer %d, fd=%d, video_buf=0x%lx, size=%d\n",
        cnt, video_frames[cnt].fd, video_frames[cnt].buffer, buff_size);
#ifdef USE_ION
      rc = do_munmap_ion(main_ion_fd, &(video_frames[cnt].fd_data), 
                         (void *) video_frames[cnt].buffer,
                          video_frames[cnt].ion_alloc.len);
#else
      rc = do_munmap(video_frames[cnt].fd, (void *) video_frames[cnt].buffer, buff_size);
#endif
      CDBG("unreg_vidbuf: do_munmap done with return value %d\n", rc);
      video_frames[cnt].buffer = 0;
    }
  } else {
    CDBG("unreg_vidbuf: unregisterVPEBuf %d\n", cnt);
    //if (video_frames[cnt].buffer == 0)
    //  continue;

    offset = buff_size;
    intrfcCtrl.unregisterVideoBuf(camfd,
      dimension, offset, video_frames[cnt].fd, (uint8_t *) video_frames[cnt].buffer, 1);
    CDBG("unreg_vidbuf: do_munmap video buffer %d, fd=%d, video_buf=0x%lx, size=%d\n",
      cnt, video_frames[cnt].fd, video_frames[cnt].buffer, buff_size);
#ifdef USE_ION
    rc = do_munmap_ion(main_ion_fd, &(video_frames[cnt].fd_data), 
                         (void *) video_frames[cnt].buffer,
                          video_frames[cnt].ion_alloc.len);
#else
    rc = do_munmap(video_frames[cnt].fd, (void *) video_frames[cnt].buffer, buff_size);
#endif
    CDBG("unreg_vidbuf: do_munmap done with return value %d\n", rc);
    video_frames[cnt].buffer = 0;
  }
  return rc;
}

/*===========================================================================
 * FUNCTION    - reg_vid_vpe_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
int reg_vid_vpe_buf(void)
{
  int cnt = 0;
  uint32_t video_buff_size;
  int8_t activeBuffer;
  unsigned long video_frame_buf = 0;
  int preview_vid_fd = -1;
  uint32_t offset=0;

  CDBG("%s: current_mode %d\n", __func__, current_mode);
  if (CAMERA_MODE_3D == current_mode)
    video_buff_size = (uint32_t)(PAD_TO_2K(dimension->video_width *
      dimension->video_height) * 3/2);
  else
    video_buff_size = (uint32_t)(dimension->video_width *
      dimension->video_height * 3/2);

  for (cnt = 0; cnt < VIDEO_FRAMES_NUM; cnt++) {
    CDBG("Print the cnt: %d\n", cnt);
    video_frames[cnt].ion_alloc.len = video_buff_size;
    video_frames[cnt].ion_alloc.flags = 0;
    video_frames[cnt].ion_alloc.heap_mask =
      (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
    video_frames[cnt].ion_alloc.align = 4096;
#ifdef USE_ION
    video_frame_buf = (unsigned long) do_mmap_ion(main_ion_fd,
      &(video_frames[cnt].ion_alloc),
      &(video_frames[cnt].fd_data),
      &(preview_vid_fd));
#else
    video_frame_buf = (unsigned long) do_mmap(video_buff_size, &preview_vid_fd);
#endif
    video_frames[cnt].fd = preview_vid_fd;
    video_frames[cnt].planar0_off = 0;

  if (CAMERA_MODE_3D == current_mode)
    video_frames[cnt].planar1_off =
      PAD_TO_2K(dimension->video_width * dimension->video_height);
  else
    video_frames[cnt].planar1_off =
      dimension->video_width * dimension->video_height;

    CDBG("video_frames[%d].cbcr_off = %d \n",cnt,
      dimension->video_width * dimension->video_height);

    if (cnt == VIDEO_FRAMES_NUM - 1) {
      activeBuffer = FALSE;
    } else {
      activeBuffer = TRUE;
    }
    video_frames[cnt].path = OUTPUT_TYPE_V;
    video_frames[cnt].buffer = video_frame_buf;

    CDBG("do_mmap pbuf = 0x%lx, pmem_fd = %d, active = %d\n",
      video_frames[cnt].buffer, video_frames[cnt].fd, activeBuffer);

    if (video_frames[cnt].buffer == 0) {
      CDBG("main error: malloc_video failed!\n");
      unreg_vidbuf(cnt, 0);
      break;
    }

    intrfcCtrl.registerVideoBuf(camfd, dimension, offset,
      &video_frames[cnt], activeBuffer, 0);
  }

  /* init the video free quue and lunch the video thread!*/
  /* Video buffers */
  for (cnt = VIDEO_FRAMES_NUM; cnt < (VIDEO_FRAMES_NUM+VIDEO_FRAMES_NUM); cnt++) {
    CDBG("Print the cnt: %d\n", cnt);
    video_frames[cnt].ion_alloc.len = video_buff_size;
    video_frames[cnt].ion_alloc.flags = 0;
    video_frames[cnt].ion_alloc.heap_mask =
      (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
    video_frames[cnt].ion_alloc.align = 4096;
#ifdef USE_ION
    video_frame_buf = (unsigned long) do_mmap_ion(main_ion_fd,
      &(video_frames[cnt].ion_alloc),
      &(video_frames[cnt].fd_data),
      &(preview_vid_fd));
#else
    video_frame_buf = (unsigned long) do_mmap(video_buff_size, &preview_vid_fd);
#endif
    video_frames[cnt].fd = preview_vid_fd;
    video_frames[cnt].planar0_off = 0;

  if (CAMERA_MODE_3D == current_mode)
    video_frames[cnt].planar1_off =
      PAD_TO_2K(dimension->video_width * dimension->video_height);
  else
    video_frames[cnt].planar1_off =
      dimension->video_width * dimension->video_height;

    video_frames[cnt].path = OUTPUT_TYPE_V;
    video_frames[cnt].buffer = video_frame_buf;

    CDBG("do_mmap pbuf = 0x%lx, pmem_fd = %d, active = %d\n",
      video_frames[cnt].buffer, video_frames[cnt].fd, activeBuffer);

    if (video_frames[cnt].buffer == 0) {
      CDBG("main error: malloc_video failed on %d!\n", cnt);
      unreg_vidbuf(cnt, 0);
      break;
    }

    // register them kernel, but all are not active
    intrfcCtrl.registerVideoBuf(camfd, dimension, offset,
      &video_frames[cnt], FALSE, 0);
  }

  /* VPE buffer */
    video_frames[cnt].ion_alloc.len = video_buff_size;
    video_frames[cnt].ion_alloc.flags = 0;
    video_frames[cnt].ion_alloc.heap_mask =
      (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
    video_frames[cnt].ion_alloc.align = 4096;
#ifdef USE_ION
    video_frame_buf = (unsigned long) do_mmap_ion(main_ion_fd,
      &(video_frames[cnt].ion_alloc),
      &(video_frames[cnt].fd_data),
      &(preview_vid_fd));
#else
  video_frame_buf = (unsigned long) do_mmap(video_buff_size, &preview_vid_fd);
#endif
  video_frames[cnt].fd = preview_vid_fd;
  video_frames[cnt].planar0_off = 0;
  if (CAMERA_MODE_3D == current_mode) {
    video_frames[cnt].planar1_off =
      PAD_TO_2K(dimension->video_width * dimension->video_height);
  } else {
  video_frames[cnt].planar1_off =
    dimension->video_width * dimension->video_height;
  }
  video_frames[cnt].path = OUTPUT_TYPE_V;
  video_frames[cnt].buffer = video_frame_buf;

  CDBG("do_mmap pbuf = 0x%lx, pmem_fd = %d, active = %d\n",
    video_frames[cnt].buffer, video_frames[cnt].fd, activeBuffer);

  if (video_frames[cnt].buffer == 0) {
    CDBG("main error: malloc_video failed on %d!\n", cnt);
    unreg_vidbuf(cnt, 1);
  }

  // register them kernel, but all are not active
  intrfcCtrl.registerVideoBuf(camfd, dimension, offset,
    &video_frames[cnt], TRUE, 1);

  // now start the video simulation
  if (video_frame_buf) {
    for (cnt = VIDEO_FRAMES_NUM; cnt < (VIDEO_FRAMES_NUM + VIDEO_FRAMES_NUM); cnt++) {
      /* add the fram to free queue first, at start there will be VIDEO_FRAMES_NUM frames in it */
      test_app_mmcamera_videoframe_return (&(video_frames[cnt]));
    }
    test_app_launch_video_frame_thread (0);
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - zsl_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
int zsl_init()
{
  int rc;
  uint8_t is_zsl = 1;
  mm_camera_status_t status;
  int i=0, index;
  char dim;

  status = mCfgControl.mm_camera_set_parm(CAMERA_PARM_ZSL_ENABLE,
    (void *)&is_zsl);
  if (status) {
    CDBG("%s: set ZSL enable failed %d\n", __func__, status);
    return -1;
  }

  dimension = malloc(sizeof(cam_ctrl_dimension_t));
  CDBG_ERROR("%s, called\n", __func__);
  if (!dimension) {
    CDBG("%s: malloc failed!\n", __func__);
    return -1;
  }

  memset(dimension, 0, sizeof(cam_ctrl_dimension_t));
  dimension->picture_width = MP1_WIDTH;
  dimension->picture_height = MP1_HEIGHT;
  dimension->ui_thumbnail_width = 512;
  dimension->ui_thumbnail_height = 384;
  dimension->display_width = 512;
  dimension->display_height = 384;
  dimension->main_img_format = CAMERA_YUV_420_NV21;
  dimension->thumb_format = CAMERA_YUV_420_NV21;
  zsl_dimension = dimension;

  zsl_thumb_width = zsl_dimension->ui_thumbnail_width;
  zsl_thumb_height = zsl_dimension->ui_thumbnail_height;
  zsl_dimension->ui_thumbnail_width = zsl_dimension->display_width;
  zsl_dimension->ui_thumbnail_height = zsl_dimension->display_height;
  if (intrfcCtrl.setDimension(camfd, zsl_dimension) == TRUE) {
    CDBG ("%s setDimension: camfd = %d done", __func__, camfd);
  }
  CDBG_ERROR ("%s setDimension: new %dx%d", __func__,
    zsl_dimension->ui_thumbnail_width, zsl_dimension->ui_thumbnail_height);

  status = mCfgControl.mm_camera_set_parm(CAMERA_PARM_DIMENSION,
    (void *)dimension);
  if (status) {
    CDBG("%s: set dimension failed %d\n", __func__, status);
    return -1;
  }

  int8_t activeBuffer;
  int preview_vid_fd = -1;
  uint32_t offset=0, cnt;

  for (cnt = 0; cnt < PREVIEW_FRAMES_NUM; cnt++) {
    CDBG("%s: register preview %d\n", __func__, cnt);
    frames[cnt].ion_alloc.len = dimension->display_width *
      dimension->display_height *
      3/2;
    frames[cnt].ion_alloc.flags = 0;
    frames[cnt].ion_alloc.heap_mask =
      (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
    frames[cnt].ion_alloc.align = 4096;
#ifdef USE_ION
    preview_frames_buf = (unsigned long) do_mmap_ion(main_ion_fd,
      &(frames[cnt].ion_alloc),
      &(frames[cnt].fd_data),
      &(preview_vid_fd));
#else
    preview_frames_buf = (unsigned long) do_mmap(dimension->display_width *
      dimension->display_height *
      3/2,
      &preview_vid_fd);
#endif
    frames[cnt].fd = preview_vid_fd;
    frames[cnt].planar0_off = 0;
    frames[cnt].planar1_off =
      dimension->display_width * dimension->display_height;

    if (cnt == PREVIEW_FRAMES_NUM - 1) {
      activeBuffer = FALSE;
    } else {
      activeBuffer = TRUE;
    }
    frames[cnt].path = OUTPUT_TYPE_P;

    frames[cnt].buffer = preview_frames_buf;
    CDBG("%s: do_mmap pbuf = 0x%lx, pmem_fd = %d, active = %d\n",
      __func__, frames[cnt].buffer, frames[cnt].fd, activeBuffer);
    if (frames[cnt].buffer == 0) {
      CDBG("%s: malloc_preview failed!\n", __func__);
      goto ERROR;
    }

    intrfcCtrl.registerPreviewBuf(camfd, dimension, offset,
      &frames[cnt], activeBuffer);
  }
  camframe_add_frame(CAM_PREVIEW_FRAME, &frames[PREVIEW_FRAMES_NUM - 1]);

  if (launch_camframe_thread(&frame_parms)) {
    CDBG("%s: launch_camframe_thread failed!\n", __func__);
    goto ERROR;
  }

  return 0;

ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION    - zsl_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void zsl_deinit()
{
  uint8_t is_zsl = 0, cnt;
  mm_camera_status_t status;
  uint32_t offset = 0;
  int rc, i;

  release_camframe_thread();
  CDBG("%s: release_camframe_thread\n", __func__);
  camframe_release_all_frames(CAM_PREVIEW_FRAME);

  /* free preview buffer after frame thread exits. */
  for (cnt = 0; cnt < PREVIEW_FRAMES_NUM; ++cnt) {
    CDBG("%s: unregisterPreviewBuf %d\n", __func__, cnt);
    offset = 0;
    intrfcCtrl.unregisterPreviewBuf(camfd,
      dimension, offset, frames[cnt].fd, (uint8_t *) frames[cnt].buffer);
    CDBG("%s: do_munmap preview buffer %d, fd=%d, prev_buf=0x%lx, size=%d\n",
      __func__, cnt, frames[cnt].fd, frames[cnt].buffer,
      dimension->display_width * dimension->display_height * 3/2);
#ifdef USE_ION
      rc = do_munmap_ion(main_ion_fd, &(frames[cnt].fd_data), 
                         (void *) frames[cnt].buffer,
                          frames[cnt].ion_alloc.len);
#else
    rc =
      do_munmap(frames[cnt].fd, (void *) frames[cnt].buffer,
      dimension->display_width * dimension->display_height * 3/2);
#endif
  }

  /* unregister and delete ZSL buffers */
  for (i=0; i<MAX_SNAPSHOT_BUFFERS; i++) {
    register_buffers(dimension->display_width,
      dimension->display_height, 0, 0,
      zsl_thumb_frame[i].fd,
      (void *)zsl_thumb_frame[i].buffer,
      MSM_PMEM_THUMBNAIL, TRUE, TRUE);
#ifdef USE_ION
    rc = do_munmap_ion(main_ion_fd, &(zsl_thumb_frame[i].fd_data), 
                         (void *) zsl_thumb_frame[i].buffer,
                          zsl_thumb_frame[i].ion_alloc.len);
#else
    rc = do_munmap(zsl_thumb_frame[i].fd, (void *)zsl_thumb_frame[i].buffer,
      zsl_thumb_size);
#endif
    CDBG("do_munmap thumbnail pbuf = %p, pmem_fd = %d\n",
      (void *)zsl_thumb_frame[i].buffer, zsl_thumb_frame[i].fd);
  }

  for (i=0; i<MAX_SNAPSHOT_BUFFERS; i++) {
    register_buffers(dimension->orig_picture_dx,
      dimension->orig_picture_dy, 0, 0,
      zsl_main_frame[i].fd,
      (void *)zsl_main_frame[i].buffer,
      MSM_PMEM_MAINIMG, TRUE, TRUE);
#ifdef USE_ION
    rc = do_munmap_ion(main_ion_fd, &(zsl_main_frame[i].fd_data), 
                         (void *) zsl_main_frame[i].buffer,
                          zsl_main_frame[i].ion_alloc.len);
#else
    rc = do_munmap(zsl_main_frame[i].fd, (void *)zsl_main_frame[i].buffer,
      zsl_main_size);
#endif
    CDBG("do_munmap thumbnail pbuf = %p, pmem_fd = %d\n",
      (void *)zsl_main_frame[i].buffer, zsl_main_frame[i].fd);
  }

  status = mCfgControl.mm_camera_set_parm(CAMERA_PARM_ZSL_ENABLE,
    (void *)&is_zsl);

  if(dimension){
    free(dimension);
    dimension = NULL;
  }
}

/*===========================================================================
 * FUNCTION    - system_video_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
int stereo_start_preview(void)
{
  int cnt = 0;
  int rc;
  int8_t vpe_flag = 0;
  uint32_t video_buff_size;
  unsigned long video_frame_buf = 0;
  int8_t activeBuffer;
  int preview_vid_fd = -1;
  uint32_t offset=0;

  /* register them kernel, but all are not active */
  cnt = VIDEO_FRAMES_NUM+VIDEO_FRAMES_NUM;
  intrfcCtrl.registerVideoBuf(camfd, dimension, offset,
    &video_frames[cnt], TRUE, 1);

  return 0;
}

/*===========================================================================
 * FUNCTION    - system_video_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
int system_init(void)
{
  int cnt = 0;
  int rc;
  int8_t vpe_flag = 0;
  uint32_t video_buff_size;
  unsigned long video_frame_buf = 0;
  CDBG("!! system_video_init!\n");

  /*  Variable Initialization */
  memset(&frames, 0, PREVIEW_FRAMES_NUM * sizeof(struct msm_frame));
  memset(&video_frames, 0, (VIDEO_FRAMES_NUM + VIDEO_FRAMES_NUM + 1) *
         sizeof(struct msm_frame));
  memset(&cropInfo_s, 0, sizeof(common_crop_t));

  sharpness_AF = (int32_t *)malloc(sizeof(int32_t));
  if (!sharpness_AF) {
    CDBG("main: malloc_ failed!\n");
    goto ERROR;
  }
  memset(sharpness_AF, 0, sizeof(int32_t));

  cropInfo.len = sizeof(common_crop_t);
  cropInfo.info = &cropInfo_s;

  dimension = malloc(sizeof(cam_ctrl_dimension_t));
  if (!dimension) {
    CDBG("main: malloc failed!\n");
    goto ERROR;
  }

  memset(dimension, 0, sizeof(cam_ctrl_dimension_t));

  dimension->picture_width = MP1_WIDTH;
  dimension->picture_height = MP1_HEIGHT;
  dimension->ui_thumbnail_width = QVGA_WIDTH;
  dimension->ui_thumbnail_height = QVGA_HEIGHT;

#if defined(_VFE_31_)
  dimension->prev_format = CAMERA_YUV_420_NV21;
  dimension->prev_format = CAMERA_YUV_420_NV21;
  dimension->main_img_format = CAMERA_YUV_420_NV21;
  dimension->thumb_format = CAMERA_YUV_420_NV21;
#endif
  if (preview_video_resolution_flag == 1) {
    dimension->orig_video_width = input_display.user_input_display_width;
    dimension->orig_video_width = CEILING32(dimension->orig_video_width);
    dimension->orig_video_height = input_display.user_input_display_height;
    dimension->display_width = input_display.user_input_display_width;
    dimension->display_height = input_display.user_input_display_height;
  } else {
    dimension->orig_video_width = HD720_WIDTH;
    dimension->orig_video_width = CEILING32(dimension->orig_video_width);
    dimension->orig_video_height = HD720_HEIGHT;
    if (current_mode != CAMERA_MODE_3D) {
      dimension->display_width = WVGA_WIDTH;
      dimension->display_height = WVGA_HEIGHT;
    } else {
      dimension->picture_width = HD720_WIDTH;//HD1080_WIDTH;
      dimension->picture_height = HD720_HEIGHT;//HD1080_HEIGHT;
      dimension->ui_thumbnail_width = 768;
      dimension->ui_thumbnail_height = 432;
    }
#if defined(_TARGET_7X27_) || defined(_TARGET_7X27A_)
    input_display.user_input_display_width = WVGA_WIDTH;
    input_display.user_input_display_height = WVGA_HEIGHT;
#else
    input_display.user_input_display_width = QVGA_WIDTH;
    input_display.user_input_display_height = QVGA_HEIGHT;
#endif
  }

  CDBG("Dimension: picture_width = %d\n, picture_height = %d\n, display_width = %d\n, "
    "display_height = %d\n, ui_thumbnail_width = %d\n, ui_thumbnail_height = %d\n",
    dimension->picture_width, dimension->picture_height, dimension->display_width,
    dimension->display_height, dimension->ui_thumbnail_width, dimension->ui_thumbnail_height);

  CDBG("Before setDimension v_width = %d, video_height = %d\n",
    dimension->video_width, dimension->video_height);

  CDBG ("setDimension: camfd = %d", camfd);
  if (MM_CAMERA_SUCCESS == mCfgControl.mm_camera_set_parm(CAMERA_PARM_DIMENSION,
    (void *)dimension)) {
    CDBG ("setDimension: camfd = %d done", camfd);
    CDBG("After setDimension v_width = %d, video_height = %d\n",
      dimension->video_width, dimension->video_height);

    int8_t activeBuffer;
    int preview_vid_fd = -1;
    uint32_t offset=0;

    CDBG ("do_mmap: camfd = %d done", camfd);
    CDBG ("do_mmap: preview_fd = %d done", preview_vid_fd);
    CDBG("setDimenstion for new display resolution\n");

    if (current_mode != CAMERA_MODE_3D) {
      for (cnt = 0; cnt < PREVIEW_FRAMES_NUM; cnt++) {
        CDBG("Print the cnt: %d\n", cnt);
        frames[cnt].ion_alloc.len = dimension->display_width *
                                         dimension->display_height *
                                         3/2;
        frames[cnt].ion_alloc.flags = 0;
        frames[cnt].ion_alloc.heap_mask =
          (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
        frames[cnt].ion_alloc.align = 4096;
#ifdef USE_ION
        preview_frames_buf = (unsigned long) do_mmap_ion(main_ion_fd,
         &(frames[cnt].ion_alloc),
         &(frames[cnt].fd_data),
         &(preview_vid_fd));
#else
        preview_frames_buf = (unsigned long) do_mmap(dimension->display_width *
          dimension->display_height *
          3/2,
          &preview_vid_fd);
#endif
        frames[cnt].fd = preview_vid_fd;
        frames[cnt].planar0_off = 0;
        frames[cnt].planar1_off =
          dimension->display_width * dimension->display_height;

        if (cnt == PREVIEW_FRAMES_NUM - 1) {
          activeBuffer = FALSE;
        } else {
          activeBuffer = TRUE;
        }
        frames[cnt].path = OUTPUT_TYPE_P;

        frames[cnt].buffer = preview_frames_buf;
        CDBG("do_mmap pbuf = 0x%lx, pmem_fd = %d, active = %d\n",
          frames[cnt].buffer, frames[cnt].fd, activeBuffer);
        if (frames[cnt].buffer == 0) {
          CDBG_ERROR("main: malloc_preview failed!\n");
          goto ERROR;
        }

        intrfcCtrl.registerPreviewBuf(camfd, dimension, offset,
          &frames[cnt], activeBuffer);
      }
    }

#if !defined(_TARGET_7X27_) && !defined(_TARGET_7X27A_)
    reg_vid_vpe_buf();
#endif

    /* Set RAW Snapshot buffer size */
    /* Assign extra 3 Lines for HJR processing */
    raw_snapshot_buffer_size =
      dimension->raw_picture_height * dimension->raw_picture_width;
    CDBG("raw_snapshot_buffer_size = %d, raw_picture_height = %d, raw_picture_width = %d",
      raw_snapshot_buffer_size, dimension->raw_picture_height,
      dimension->raw_picture_width);
  } else {
    CDBG("%d setDimension failed!\n", __LINE__);
    goto ERROR;
  }

  if (current_mode != CAMERA_MODE_3D) {
    camframe_add_frame(CAM_PREVIEW_FRAME, &frames[PREVIEW_FRAMES_NUM - 1]);
  }
  camframe_add_frame(CAM_VIDEO_FRAME, &video_frames[VIDEO_FRAMES_NUM - 1]);

  frame_parms.cammode = current_mode;
  if (interfaceType == NATIVE_CAMERA_INTERFACE) {
    if (launch_camframe_fb_thread()) {
      CDBG("main: launch_camframe_fb_thread failed!\n");
      goto ERROR;
    }

    if (launch_camframe_thread(&frame_parms)) {
      CDBG("main: launch_camframe_thread failed!\n");
      goto ERROR;
    }
  } else
    pthread_create(&frame_thread, NULL, intrfcCtrl.frameThread, &frame_parms);

  return 0;

  ERROR:
  /* @todo need to handle cleanup for ERRORs */
  return -1;
}

/*===========================================================================
 * FUNCTION    - stereo_stop_preview -
 *
 * DESCRIPTION:
 *==========================================================================*/
int stereo_stop_preview(void)
{
  int cnt = 0;
  int rc;
  int offset = 0;
  uint32_t buff_size = 0;

  /* for vpe: during unregister, use pmem type = MSM_PMEM_VIDOE
   * not MSM_PMEM_VIDEO_VPE because during VPE AXI config this VPE buffer
   * is converted to VIDEO buffer type. */
  CDBG("unregisterVPEBuf %d\n", cnt);
  offset = 0;
  cnt = VIDEO_FRAMES_NUM + VIDEO_FRAMES_NUM;
  intrfcCtrl.unregisterVideoBuf(camfd, dimension, offset, video_frames[cnt].fd,
                                (uint8_t *) video_frames[cnt].buffer, 0);

  CDBG("do_munmap video buffer %d, fd=%d, video_buf=0x%lx, size=%d\n",
    cnt, video_frames[cnt].fd, video_frames[cnt].buffer, buff_size);
  return 0;
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
  if (interfaceType == NATIVE_CAMERA_INTERFACE) {
    release_camframe_thread();
    CDBG("Called release_camframe_thread!!!\n");
    release_camframe_fb_thread();
    CDBG("Called release_camframe_fb_thread!!!\n");
    /* close the video thread and flush the busy queue */
    CDBG("Called test_app_release_video_frame_thread!!!\n");
    test_app_release_video_frame_thread();
    /* now flush the video buffer free queue */
    CDBG("Calling cam_frame_flush_free_video!!!\n");
    camframe_release_all_frames(CAM_VIDEO_FRAME);
    camframe_release_all_frames(CAM_PREVIEW_FRAME);
  }

  if (current_mode != CAMERA_MODE_3D) {
    /* free preview buffer after frame thread exits. */
    for (cnt = 0; cnt < PREVIEW_FRAMES_NUM; ++cnt) {
      CDBG("unregisterPreviewBuf %d\n", cnt);
      offset = 0;
      intrfcCtrl.unregisterPreviewBuf(camfd,
        dimension, offset, frames[cnt].fd, (uint8_t *) frames[cnt].buffer);
      CDBG("do_munmap preview buffer %d, fd=%d, prev_buf=0x%lx, size=%d\n",
        cnt, frames[cnt].fd, frames[cnt].buffer,
        dimension->display_width * dimension->display_height * 3/2);
#ifdef USE_ION
      rc = do_munmap_ion(main_ion_fd, &(frames[cnt].fd_data), 
                         (void *) frames[cnt].buffer,
                          frames[cnt].ion_alloc.len);
#else
      rc =
        do_munmap(frames[cnt].fd, (void *) frames[cnt].buffer,
        dimension->display_width * dimension->display_height * 3/2);
#endif
    }
  }

#if !defined(_TARGET_7X27_) && !defined(_TARGET_7X27A_)
  /* free video buffer after frame thread exits. */
#if 0
  unreg_vidbuf(VIDEO_FRAMES_NUM + VIDEO_FRAMES_NUM, 0);
#endif
  buff_size = dimension->video_width * dimension->video_height * 3/2;
  for (cnt = 0; cnt < VIDEO_FRAMES_NUM + VIDEO_FRAMES_NUM; ++cnt) {
    CDBG("unregisterVideoBuf %d\n", cnt);
    if (video_frames[cnt].buffer == 0)
      continue;

    offset = 0;
    intrfcCtrl.unregisterVideoBuf(camfd,
      dimension, offset, video_frames[cnt].fd, (uint8_t *) video_frames[cnt].buffer, 0);
    CDBG("do_munmap video buffer %d, fd=%d, video_buf=0x%lx, size=%d\n",
      cnt, video_frames[cnt].fd, video_frames[cnt].buffer, buff_size);
#ifdef USE_ION
    rc = do_munmap_ion(main_ion_fd, &(video_frames[cnt].fd_data), 
                         (void *) video_frames[cnt].buffer,
                          video_frames[cnt].ion_alloc.len);
#else
    rc = do_munmap(video_frames[cnt].fd, (void *) video_frames[cnt].buffer, buff_size);
#endif
    CDBG("do_munmap done with return value %d\n", rc);
  }
  /* for vpe: during unregister, use pmem type = MSM_PMEM_VIDOE
   * not MSM_PMEM_VIDEO_VPE because during VPE AXI config this VPE buffer
   * is converted to VIDEO buffer type. */
#if 0
  unreg_vidbuf(cnt, 1);
#endif
  CDBG("unregisterVPEBuf %d\n", cnt);
  offset = 0;
  intrfcCtrl.unregisterVideoBuf(camfd,
    dimension, offset, video_frames[cnt].fd, (uint8_t *) video_frames[cnt].buffer, 0);

  CDBG("do_munmap video buffer %d, fd=%d, video_buf=0x%lx, size=%d\n",
    cnt, video_frames[cnt].fd, video_frames[cnt].buffer, buff_size);
#ifdef USE_ION
 rc = do_munmap_ion(main_ion_fd, &(video_frames[cnt].fd_data), 
                         (void *) video_frames[cnt].buffer,
                          video_frames[cnt].ion_alloc.len);
#else
  rc = do_munmap(video_frames[cnt].fd, (void *) video_frames[cnt].buffer, buff_size);
#endif
  CDBG("do_munmap done with return value %d\n", rc);
#endif
  free(dimension);
  free(sharpness_AF);
  test_app_release_video();
  return 0;
}

/*===========================================================================
 * FUNCTION     - start_zsl_mode -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int start_zsl_mode(void)
{
  int rc = 0, i;
  mm_camera_status_t status;
  zsl_params_t params;

  /* fill parameters*/
  memset(&params, 0, sizeof(zsl_params_t));
  params.picture_width = zsl_dimension->picture_width;
  params.picture_height = zsl_dimension->picture_height;
  params.preview_width = zsl_dimension->ui_thumbnail_width;
  params.preview_height = zsl_dimension->ui_thumbnail_height;
  params.useExternalBuffers = TRUE;
  CDBG ("%s Pic ht*W = %dx%d, ZSL dim = %d*%d", __func__,
    zsl_dimension->picture_height, zsl_dimension->picture_width,
    zsl_dimension->picture_height, zsl_dimension->picture_width);
  status = mCamOps.mm_camera_init(CAMERA_OPS_STREAMING_ZSL,
    (void *)&params, NULL);
  if (MM_CAMERA_SUCCESS != status) {
    printf("\n%s init MM_CAMERA_STREAMING_ZSL failed %d\n", __func__, status);
    rc = -1;
    return rc;
  }

  /* create and register ZSL buffers */
  if (params.useExternalBuffers) {
    zsl_thumb_size = dimension->display_width *
      CEILING16(dimension->display_height) * 3/2;
    for (i=0; i<MAX_SNAPSHOT_BUFFERS; i++) {
        zsl_thumb_frame[i].ion_alloc.len = zsl_thumb_size;
        zsl_thumb_frame[i].ion_alloc.flags = 0;
        zsl_thumb_frame[i].ion_alloc.heap_mask =
          (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
        zsl_thumb_frame[i].ion_alloc.align = 4096;
#ifdef USE_ION
        zsl_thumb_frame[i].buffer = (unsigned long) do_mmap_ion(main_ion_fd,
         &(zsl_thumb_frame[i].ion_alloc),
         &(zsl_thumb_frame[i].fd_data),
         &(zsl_thumb_frame[i].fd));
#else
      zsl_thumb_frame[i].buffer = (unsigned long)do_mmap(zsl_thumb_size,
        &(zsl_thumb_frame[i].fd));
#endif
      CDBG("do_mmap thumbnail pbuf = %p, pmem_fd = %d\n",
        (void *)zsl_thumb_frame[i].buffer, zsl_thumb_frame[i].fd);
      register_buffers(dimension->display_width,
        dimension->display_height, 0, 0,
        zsl_thumb_frame[i].fd,
        (void *)zsl_thumb_frame[i].buffer,
        MSM_PMEM_THUMBNAIL, FALSE, i<3);
    }

    zsl_main_size = dimension->orig_picture_dx *
      CEILING16(dimension->orig_picture_dy) * 3/2;
    for (i=0; i<MAX_SNAPSHOT_BUFFERS; i++) {
        zsl_main_frame[i].ion_alloc.len = zsl_main_size;
        zsl_main_frame[i].ion_alloc.flags = 0;
        zsl_main_frame[i].ion_alloc.heap_mask =
          (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
        zsl_main_frame[i].ion_alloc.align = 4096;
#ifdef USE_ION
        zsl_main_frame[i].buffer = (unsigned long) do_mmap_ion(main_ion_fd,
         &(zsl_main_frame[i].ion_alloc),
         &(zsl_main_frame[i].fd_data),
         &(zsl_main_frame[i].fd));
#else
      zsl_main_frame[i].buffer = (unsigned long)do_mmap(zsl_main_size,
        &(zsl_main_frame[i].fd));
#endif
      CDBG("do_mmap main pbuf = %p, pmem_fd = %d\n",
        (void *)zsl_main_frame[i].buffer, zsl_main_frame[i].fd);
      register_buffers(dimension->orig_picture_dx,
        dimension->orig_picture_dy, 0, 0,
        zsl_main_frame[i].fd,
        (void *)zsl_main_frame[i].buffer,
        MSM_PMEM_MAINIMG, FALSE, i<3);
    }
  }

  status = mCamOps.mm_camera_start(CAMERA_OPS_STREAMING_ZSL,
    NULL, NULL);
  if (MM_CAMERA_SUCCESS != status) {
    printf("\n%s start MM_CAMERA_STREAMING_ZSL failed %d\n", __func__, status);
    rc = -1;
    return rc;
  }

  printf("\n%s exit success %d \n", __func__, rc);
  return rc;

}

/*===========================================================================
 * FUNCTION     - zsl_pp_capture -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int zsl_pp_capture(int num_capt)
{
  int rc = 0, i=0, index, num_encode = 0;
  mm_camera_status_t status;
  zsl_capture_params_t capt_params;
  encode_params_t encode_params;
  char input[BUFF_SIZE_20];
  wait_for_capture_done = TRUE;

  /* fill parameters*/
  memset(&capt_params, 0, sizeof(zsl_capture_params_t));
  capt_params.thumbnail_width = zsl_thumb_width;
  capt_params.thumbnail_height = zsl_thumb_height;
  capt_params.num_captures = num_capt;
  g_num_captures = num_capt;
  g_current_capture_count = 0;

  status = mCamOps.mm_camera_start(CAMERA_OPS_CAPTURE,
    (void *)&capt_params, NULL);
  if (MM_CAMERA_SUCCESS != status) {
    printf("\n%s MM_CAMERA_CAPTURE failed %d\n", __func__, status);
    rc = -1;
    goto zsl_burst_capture_end;
  }

  pthread_mutex_lock(&snapshot_mutex);
  pthread_cond_wait(&snapshot_cond, &snapshot_mutex);
  pthread_mutex_unlock(&snapshot_mutex);
  printf("\nCapture completed");

  memset(&encode_params, 0, sizeof(encode_params_t));
  printf("\nSelect the indexes for encoding 0..%d.\n"
    "enter -1 at the end\n", num_capt-1);
  while(1) {
    fgets(input, BUFF_SIZE_20, stdin);
    index = atoi(input);
    if ((index < 0) || (index >= num_capt))
      break;
    encode_params.encodeBitMask |= (1<<index);
    num_encode++;
  };
  encode_params.exif_data = NULL;
  encode_params.exif_numEntries = 0;
  encode_params.p_output_buffer = output_buffer;
  for (i=0; i<num_encode; i++) {
    encode_params.p_output_buffer[i].offset = 0;
    encode_params.p_output_buffer[i].size = zsl_dimension->picture_width
      * zsl_dimension->picture_height * 3/2;
    encode_params.p_output_buffer[i].ptr = (uint8_t *)malloc(
      encode_params.p_output_buffer[i].size);
    if ( NULL == encode_params.p_output_buffer[i].ptr ) {
      printf("\n%s failed output_buffer->ptr is NULL\n", __func__);
      rc = -1;
      goto zsl_burst_capture_end;
    }
  }

  g_num_captures = num_encode;
  g_current_capture_count = 0;
  status = mCamOps.mm_camera_start(CAMERA_OPS_ENCODE,
    (void *)&encode_params, NULL);
  if (MM_CAMERA_SUCCESS != status) {
    printf("\n%s MM_CAMERA_ENCODE failed %d\n", __func__, status);
    rc = -1;
    goto zsl_burst_capture_end;
  }

  pthread_mutex_lock(&snapshot_mutex);
  pthread_cond_wait(&snapshot_cond, &snapshot_mutex);
  pthread_mutex_unlock(&snapshot_mutex);
  printf("\nEncode completed");

zsl_burst_capture_end:

  for (i=0; i<num_capt; i++) {
    if (output_buffer[i].ptr) {
      free(output_buffer[i].ptr);
      output_buffer[i].ptr = NULL;
    } else
      break;
  }
  wait_for_capture_done = false;

  CDBG("\n%s exit %d\n", __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION     - capture_zsl -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int capture_zsl(void)
{
  int rc = 0, num_capt, i=0;
  mm_camera_status_t status;
  zsl_capture_params_t capt_params;
  encode_params_t encode_params;
  char input[BUFF_SIZE_20];

  printf("Enter the number of snapshots to be taken (1..%d)",
    MAX_ZSL_SNAPSHOT_SUPPORTED);
  fgets(input, BUFF_SIZE_20, stdin);
  num_capt = atoi(input);
  if ((num_capt < 1) || (num_capt > MAX_ZSL_SNAPSHOT_SUPPORTED)) {
    printf("\nInvalid number of captures");
    rc = -1;
    goto capture_zsl_end;
  }

  if ((external_yuv_snap_pp && (num_capt == 1))
    || (external_yuv_snap_pp_burst && (num_capt > 1))) {
    return zsl_pp_capture(num_capt);
  }

  /* fill parameters*/
  memset(&capt_params, 0, sizeof(zsl_capture_params_t));
  capt_params.thumbnail_width = zsl_thumb_width;
  capt_params.thumbnail_height = zsl_thumb_height;
  capt_params.num_captures = num_capt;
  memset(&encode_params, 0, sizeof(encode_params_t));
  encode_params.exif_data = NULL;
  encode_params.exif_numEntries = 0;
  encode_params.p_output_buffer = output_buffer;
  g_num_captures = num_capt;
  g_current_capture_count = 0;
  for (i=0; i<num_capt; i++) {
    encode_params.p_output_buffer[i].offset = 0;
    encode_params.p_output_buffer[i].size = zsl_dimension->picture_width
      * zsl_dimension->picture_height * 3/2;
    encode_params.p_output_buffer[i].ptr = (uint8_t *)malloc(
      encode_params.p_output_buffer[i].size);
    if ( NULL == encode_params.p_output_buffer[i].ptr ) {
      printf("\n%s failed output_buffer->ptr is NULL\n", __func__);
      rc = -1;
      goto capture_zsl_end;
    }
  }

  status = mCamOps.mm_camera_start(CAMERA_OPS_CAPTURE_AND_ENCODE,
    (void *)&capt_params, &encode_params);
  if (MM_CAMERA_SUCCESS != status) {
    printf("\n%s MM_CAMERA_CAPTURE failed %d\n", __func__, status);
    rc = -1;
    goto capture_zsl_end;
  }

  pthread_mutex_lock(&snapshot_mutex);
  pthread_cond_wait(&snapshot_cond, &snapshot_mutex);
  pthread_mutex_unlock(&snapshot_mutex);

capture_zsl_end:

  for (i=0; i<num_capt; i++) {
    if (output_buffer[i].ptr) {
      free(output_buffer[i].ptr);
      output_buffer[i].ptr = NULL;
    } else
      break;
  }

  printf("\n%s exit %d\n", __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION     - stop_zsl -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int stop_zsl_mode(void)
{
  int rc = 0;
  mm_camera_status_t status;

  status = mCamOps.mm_camera_stop(CAMERA_OPS_STREAMING_ZSL,
    NULL, NULL);
  if (MM_CAMERA_SUCCESS != status) {
    printf("\n%s mm_camera_stop failed %d\n", __func__, status);
    rc = -1;
  }

  status = mCamOps.mm_camera_deinit(CAMERA_OPS_STREAMING_ZSL,
    NULL, NULL);

  if (MM_CAMERA_SUCCESS != status) {
    printf("\n%s mm_camera_deinit failed %d\n", __func__, status);
    rc = -1;
  }
  printf("\n %s exit %d\n", __func__, rc);
  return rc;
}


/*===========================================================================
 * FUNCTION     - submain -
 *
 * DESCRIPTION:
 * ===========================================================================*/
static int submain()
{
  int back_mainflag = 0;
  int optind, optopt;
  int rc;
  char tc_buf[3];

  menu_id_change_t current_menu_id = MENU_ID_MAIN, next_menu_id;
  camera_action_t action_id;
  static camera_action_t next_action_id = ACTION_NO_ACTION;
  int action_param;

  printf("next_action_id = %d\n", next_action_id);

  struct timeval tdStopCamera;
  struct timezone tz;

  CDBG_ERROR("%s, camera_mode = %d\n", __func__, camera_mode);
  switch (camera_mode) {
    case CAMERA_MODE_ZSL: {
      if (zsl_init() < 0) {
        goto ERROR;
      }
      if (start_zsl_mode() < 0) {
        goto ERROR;
      }
      break;
    }
    case CAMERA_MODE_VIDEO:
    default: {
      if (system_init() < 0) {
        goto ERROR;
      }

      if (start_video() < 0) {
        goto ERROR;
      }
      break;
    }
  }

  do {
    print_current_menu (current_menu_id);
    fgets(tc_buf, 3, stdin);
    next_menu_id = next_menu(current_menu_id, tc_buf[0], & action_id, & action_param);
    if (next_menu_id != MENU_ID_INVALID)
      current_menu_id = next_menu_id;

    if (action_id == ACTION_NO_ACTION)
      continue;

    if (camframe_status == -1) {
      printf("Preview/Video ERROR condition reported Closing Camera APP\n");
      break;
    }

    switch (action_id) {
      case ACTION_TAKE_YUV_PICTURE:
      case ACTION_TAKE_JPEG_PICTURE:
        CDBG("Take Picture\n");
        if (mCamOps.mm_camera_start(CAMERA_OPS_PREPARE_SNAPSHOT, NULL, NULL)
          != MM_CAMERA_SUCCESS)
          goto ERROR;
        if (stop_video() != 0)
          goto ERROR;
        if (current_mode == CAMERA_MODE_3D) {
          stereo_stop_preview();
          take_picture(action_id);
          stereo_start_preview();
        } else
          take_picture (action_id);
        usleep(100*1000);
        if (start_video() != 0)
          goto ERROR;
        break;

      case ACTION_TAKE_RAW_PICTURE:
        CDBG("Take Raw Picture\n");
        if (stop_video() != 0) {
          goto ERROR;
        }
        take_raw_picture ();
        if (start_video() == -1) {
          goto ERROR;
        }
        break;

      case ACTION_STOP_CAMERA:

        gettimeofday(&tdStopCamera, &tz);
        CDBG("Profiling: Stop Camera start timestamp = %ld ms\n",
          (tdStopCamera.tv_sec * 1000) + (tdStopCamera.tv_usec/1000));

        CDBG("Selection is for stop preview/video");
        if(camera_mode == CAMERA_MODE_VIDEO){
          if (stop_video() != 0)
            goto ERROR;
        } else if(camera_mode == CAMERA_MODE_ZSL) {
          if(stop_zsl_mode() != 0)
            goto ERROR;
        }
        break;

      case ACTION_START_RECORDING:
        CDBG("Selection is for start video recording \n");
        if (start_recording() != 0) {
          CDBG("Error in start_recording \n");
          goto ERROR;
        }
        break;

      case ACTION_STOP_RECORDING:
        CDBG("Selection is for stop video recording \n");
        if (stop_recording() != 0) {
          CDBG("Error in stop_recording \n");
          goto ERROR;
        }
        break;

      case ACTION_ZOOM_INCREASE:
        zoom_increase(1);
        break;

      case ACTION_ZOOM_STEP_INCREASE:
        zoom_increase(0);
        break;

      case ACTION_ZOOM_DECREASE:
        zoom_decrease(1);
        break;

      case ACTION_ZOOM_STEP_DECREASE:
        zoom_decrease(0);
        break;

      case ACTION_PRINT_MAXZOOM:
        print_maxzoom();
        break;

      case ACTION_PRINT_ZOOMRATIOS:
        print_zoomratios();
        break;

      case ACTION_CONTRAST_INCREASE:
        CDBG("Selection for the contrast increase\n");
        increase_contrast ();
        break;

      case ACTION_CONTRAST_DECREASE:
        CDBG("Selection for the contrast decrease\n");
        decrease_contrast ();
        break;

      case ACTION_SATURATION_INCREASE:
        CDBG("Selection for the saturation increase\n");
        increase_saturation ();
        break;

      case ACTION_SATURATION_DECREASE:
        CDBG("Selection for the saturation decrease\n");
        decrease_saturation ();
        break;

      case ACTION_SPECIAL_EFFECT:
        CDBG("Selection for Special Effect changes\n");
        SpecialEffect ();
        break;

      case ACTION_BRIGHTNESS_INCREASE:
        CDBG("Selection for the brightness increase\n");
        increase_brightness ();
        break;

      case ACTION_BRIGHTNESS_DECREASE:
        CDBG("Selection for the brightness decrease\n");
        decrease_brightness ();
        break;

      case ACTION_EV_INCREASE:
        CDBG("Selection for the EV increase\n");
        increase_EV ();
        break;

      case ACTION_EV_DECREASE:
        CDBG("Selection for the EV decrease\n");
        decrease_EV ();
        break;

      case ACTION_ANTI_BANDING:
        CDBG("Selection for the anti banding change\n");
        set_antibanding ();
        break;

      case ACTION_SET_WHITE_BALANCE:
        CDBG("Selection for the White Balance changes\n");
        set_whitebalance ();
        break;

      case ACTION_AEC_MODE:
        CDBG("Selection for the AEC mode changes\n");
        AEC_mode_change ();
        break;

      case ACTION_ISO_INCREASE:
        CDBG("Selection for the ISO increase\n");
        increase_ISO ();
        break;

      case ACTION_ISO_DECREASE:
        CDBG("Selection for the ISO decrease\n");
        decrease_ISO ();
        break;

      case ACTION_SHARPNESS_INCREASE:
        CDBG("Selection for the sharpness increase\n");
        increase_sharpness ();
        break;

      case ACTION_SHARPNESS_DECREASE:
        CDBG("Selection for the sharpness decrease\n");
        decrease_sharpness ();
        break;

      case ACTION_SET_AUTO_FOCUS:
        CDBG("Selection for the Snapshot AF\n");
        set_auto_focus ();
        break;

      case ACTION_SET_HJR:
        CDBG("Selection for the set HJR changes\n");
        set_hjr ();
        intrfcCtrl.setIso(camfd, iso);
        break;

      case ACTION_SET_LENS_SHADING:
        CDBG("Selection for the set Lens Shading changes\n");
        LensShading ();
        break;

      case ACTION_SET_LED_MODE:
        CDBG("Selection for the set LED mode\n");
        LED_mode_change ();
        break;
      case ACTION_SET_BESTSHOT_MODE:
        set_bestshotmode(action_param);
        break;

      case ACTION_TOGGLE_CAMERA_MODE:
        back_mainflag = 1;
        toggle_camera_mode( );
        break;

      case ACTION_TOGGLE_FACE_DETECTION:
        toggle_facedetection( );
        break;

      case ACTION_GET_SHARPNESS_AF:
        CDBG("Selection for the get sharpness mode\n");
        set_sharpness_AF ();
        break;

      case ACTION_SNAPSHOT_RESOLUTION:
        CDBG("Selection for the snapshot resolution change\n");
        if(camera_mode == CAMERA_MODE_ZSL) {
          if (stop_zsl_mode() != 0) {
            goto ERROR;
          }
          zsl_deinit();
          if (zsl_init() < 0) {
            goto ERROR;
          }
          snapshot_resolution (action_param);

          if (start_zsl_mode() != 0) {
            goto ERROR;
          }
        } else if (camera_mode == CAMERA_MODE_VIDEO) {
          if (stop_video() != 0) {
            goto ERROR;
          }
          system_destroy();
          if (system_init() < 0) {
            goto ERROR;
          }
          snapshot_resolution (action_param);
          if (start_video() != 0) {
            goto ERROR;
          }
        }
        break;

      case ACTION_PREVIEW_VIDEO_RESOLUTION:
        back_mainflag = 1;
        CDBG ("stopP: camfd = %d", camfd);
        if(camera_mode == CAMERA_MODE_ZSL) {
          if (stop_zsl_mode() != 0)
            goto ERROR;
        } else if (camera_mode == CAMERA_MODE_VIDEO){
          if (stop_video() != 0) {
            goto ERROR;
          }
        }

        CDBG("Selection for the preview/video resolution change\n");
        preview_video_resolution (action_param);
        break;

      case ACTION_MOTION_ISO:
        CDBG ("Selection for ISO motion change.");
        set_MotionIso ();
        break;

      case ACTION_TOGGLE_HUE:
        CDBG("Selection for toggling hue.\n");
        toggle_hue();
        break;

      case ACTION_CANCEL_AUTO_FOCUS:
        CDBG("Selection for cancelling auto focus.\n");
        cancel_af();
        break;

      case ACTION_GET_AF_STEP:
        CDBG("Selection for getting auto focus step.\n");
        get_af_step();
        break;

      case ACTION_SET_AF_STEP:
        CDBG("Selection for setting auto focus step.\n");
        set_af_step();
        break;

      case ACTION_ENABLE_AFD:
        CDBG("Selection for enabling auto flicker detection.\n");
        enable_afd();
        break;

      case ACTION_SET_FPS_MODE:
        CDBG("Selection for setting fps mode.\n");
        set_fps_mode();
        break;

      case ACTION_SET_FPS:
        CDBG("Selection for setting fps.\n");
        set_fps((uint32_t) action_param);
        break;

      case ACTION_ENABLE_LA:
        enable_la();
        break;

      case ACTION_DISABLE_LA:
        disable_la();
        break;

      case ACTION_PALER_SKIN:
        make_skin_paler();
        break;

      case ACTION_TANNER_SKIN:
        make_skin_tanner();
        break;

      case ACTION_SET_AF_FOCUSRECT:
        CDBG("Selection for setting AF focus rectangle.\n");
        afFocusRect = action_param;
        set_af_focusrect();
        break;

      case ACTION_ROI_SET_INDEX:
        roiIndex_set();
        break;

      case ACTION_ROI_SET_LOCATION:
        roiIndex_set_location( );
        break;

      case ACTION_ROI_DISABLE:
        aec_roi_disable( );
        break;

      case ACTION_ENABLE_CAF:
        set_CAF();
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

      case ACTION_TOGGLE_STROBE_FLASH_MODE:
        CDBG("Selection for toggling Jpeg rotation.\n");
        toggle_Strobe_Flash_Mode();
        break;

        case ACTION_TOGGLE_JPEG_ROTATION:
        CDBG("Selection for toggling Jpeg rotation.\n");
        toggle_Jpeg_Rotation();
        break;

      case ACTION_SET_AF_ROI:
        CDBG("Selection for AF ROI.\n");
        setAF_ROI();
        break;

      case ACTION_START_LIVESHOT:
        startliveshot();
        break;

      case ACTION_VPE_ROT_0:
        CDBG("Configuartion for ACTION_VPE_ROT_0.\n");
        video_rotation_config(0);
        break;

      case ACTION_VPE_ROT_90:
        CDBG("Configuartion for ACTION_VPE_ROT_90.\n");
        video_rotation_config(90);
        break;

      case ACTION_VPE_ROT_180:
        CDBG("Configuartion for ACTION_VPE_ROT_180.\n");
        video_rotation_config(180);
        break;

      case ACTION_VPE_ROT_270:
        CDBG("Configuartion for ACTION_VPE_ROT_270.\n");
        video_rotation_config(270);
        break;

      case ACTION_VPE_NO_DIS:
        back_mainflag = 1;
        action_id = ACTION_STOP_CAMERA;
        CDBG ("stop: camfd = %d", camfd);
        if (stop_video() != 0)
          goto ERROR;
        CDBG("Configuartion for ACTION_VPE_NO_DIS.\n");
        video_dis_configuration(0);
        break;

      case ACTION_VPE_EN_DIS:
        back_mainflag = 1;
        action_id = ACTION_STOP_CAMERA;
        CDBG ("stop: camfd = %d", camfd);
        if (stop_video() != 0)
          goto ERROR;
        CDBG("Configuartion for ACTION_VPE_EN_DIS.\n");
        video_dis_configuration(1);
        break;

      case ACTION_QUERY_FLASH_4_SNAP:
        query_aec_flash_for_snapshot( );
        break;

      case ACTION_GET_FOCUS_DISTANCES:
        CDBG("Selection for getting focus distances\n");
        get_focus_distances();
        break;

      case ACTION_GET_FOCAL_LENGTH:
        CDBG("Get Focal Length\n");
        get_focal_length();
        break;

      case ACTION_GET_HORIZONTAL_VIEW_ANGLE:
        CDBG("Get Horizontal View Angle");
        get_horizontal_view_angle();
        break;

      case ACTION_GET_VERTICAL_VIEW_ANGLE:
        CDBG("Get Vertical View Angle");
        get_vertical_view_angle();
        break;

      case ACTION_RESET_LENS_TO_INFINITY:
        CDBG("Reset Lens to infinity\n");
        reset_lens_infinity();
        break;

      case ACTION_GET_SNAPSHOT_DATA:
        CDBG("Get snapshot data\n");
        get_snapshot_data();
        break;

      case ACTION_3D_DIST_0:
        CDBG("Set 3D Display distance\n");
        set_3d_display_distance(0);
        break;

      case ACTION_3D_DIST_10:
        CDBG("Set 3D Display distance\n");
        set_3d_display_distance(10);
        break;

      case ACTION_3D_DIST_20:
        CDBG("Set 3D Display distance\n");
        set_3d_display_distance(20);
        break;

      case ACTION_3D_DIST_30:
        CDBG("Set 3D Display distance\n");
        set_3d_display_distance(30);
        break;

      case ACTION_3D_ANGLE_0:
        CDBG("Set 3D View angle\n");
        set_3d_view_angle(0);
        break;

      case ACTION_3D_ANGLE_30:
        CDBG("Set 3D View angle\n");
        set_3d_view_angle(30);
        break;

      case ACTION_3D_ANGLE_60:
        CDBG("Set 3D View angle\n");
        set_3d_view_angle(60);
        break;

      case ACTION_3D_ANGLE_90:
        CDBG("Set 3D View angle\n");
        set_3d_view_angle(90);
        break;

      case ACTION_UPDATE_PREVIEW_FORMAT:
        CDBG("Update preview format\n");
        update_preview_format();
        break;

      case ACTION_3D_AUTO_CONV:
        CDBG("Use Auto Convergence\n");
        set_3d_conv_control(TRUE);
        break;

      case ACTION_3D_MANUAL_CONV:
        CDBG("Use Manual Convergence\n");
        set_3d_conv_control(FALSE);
          break;

      case ACTION_NO_ACTION:
        printf("Go back to main menu");
        break;

      case ACTION_START_ZSL:
        if (CAMERA_MODE_VIDEO == camera_mode) {
          CDBG_ERROR("%s, ACTION_START_ZSL\n", __func__);
          back_mainflag = 1;
          if (stop_video() != 0)
            goto ERROR;
          prev_camera_mode = camera_mode;
          camera_mode = CAMERA_MODE_ZSL;
        }
        break;

      case ACTION_STOP_ZSL:
        if (CAMERA_MODE_ZSL == camera_mode) {
          CDBG_ERROR("%s, ACTION_STOP_ZSL\n", __func__);
          back_mainflag = 1;
          if (stop_zsl_mode() != 0)
            goto ERROR;
          prev_camera_mode = camera_mode;
          camera_mode = CAMERA_MODE_VIDEO;
          printf("Need to call system_int\n");
        }
        break;

      case ACTION_CAPTURE_ZSL:
        if (CAMERA_MODE_ZSL == camera_mode) {
          if(query_aec_flash_for_snapshot( )) {
              if (mCamOps.mm_camera_start(CAMERA_OPS_PREPARE_SNAPSHOT,
                NULL, NULL) != MM_CAMERA_SUCCESS)
                goto ERROR;
              if (stop_zsl_mode() != 0)
                goto ERROR;
              zsl_deinit();
              if (system_init() != 0)
                goto ERROR;
              take_picture (TAKE_JPEG_PICTURE);
              system_destroy();
              camera_mode = CAMERA_MODE_ZSL;

              CDBG_ERROR("NAG_DBG: flash, true start_zsl\n");
              if(zsl_init() != 0)
                goto ERROR;
              if(start_zsl_mode() != 0)
                goto ERROR;
          } else {
            CDBG_ERROR("NAG_DBG: flash, false capture_ZSL\n");
            capture_zsl();
          }
        }
        break;

      default:
        printf("\n\n!!!!!WRONG INPUT: %d!!!!\n", action_id);
        break;
    }
    usleep(1000 * 1000);
    CDBG("action_id = %d\n", action_id);
    camframe_status = 0;
  } while ((action_id != ACTION_STOP_CAMERA) &&
    (action_id != ACTION_PREVIEW_VIDEO_RESOLUTION) &&
    (action_id != ACTION_START_ZSL) &&
    (action_id != ACTION_STOP_ZSL) );

  if (prev_camera_mode == CAMERA_MODE_VIDEO)
    system_destroy();
  else if (prev_camera_mode == CAMERA_MODE_ZSL)
    zsl_deinit();
  else if (prev_camera_mode == CAMERA_MODE_NONE) {
    if (camera_mode == CAMERA_MODE_VIDEO)
      system_destroy();
    else if (camera_mode == CAMERA_MODE_ZSL)
      zsl_deinit();
  }

  action_id = ACTION_NO_ACTION;

  return back_mainflag;

  ERROR:
  back_mainflag = 0;
  return back_mainflag;
}

/*===========================================================================
 * FUNCTION    - reg_unreg_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t register_buffers(int width, int height,
  uint32_t offset, int y_off, int fd, uint8_t * buf,
  int type, int8_t unregister, int8_t active)
{
  struct msm_pmem_info pmemBuf;
  int ret;

  pmemBuf.type = type;
  pmemBuf.fd = fd;
  pmemBuf.vaddr = buf;
  pmemBuf.planar0_off = y_off;
  pmemBuf.active = active;
  pmemBuf.offset = offset;

  if (type == MSM_PMEM_RAW_MAINIMG) {
    pmemBuf.planar1_off = 0;
    pmemBuf.len    = width * height;
  } else if (type == MSM_PMEM_MAINIMG) {
    cam_buf_info_t buf_info;
    buf_info.resolution.width = width;
    buf_info.resolution.height = height;
    mCfgControl.mm_camera_get_parm(CAMERA_PARM_BUFFER_INFO, (void *)&buf_info);
    pmemBuf.planar0_off = buf_info.yoffset;
    pmemBuf.planar1_off = buf_info.cbcr_offset;
    pmemBuf.len = buf_info.size;
    CDBG("%s: pmemBuf.cbcr_off %d \n", __func__, pmemBuf.planar1_off);
  } else {
    uint32_t y_size;
    y_size = width * height;
    pmemBuf.planar1_off = PAD_TO_WORD(y_size);
    pmemBuf.len    = width * height * 3/2;
  }

  ret = mCamOps.mm_camera_start(unregister ? CAMERA_OPS_UNREGISTER_BUFFER :
    CAMERA_OPS_REGISTER_BUFFER ,(void *)&pmemBuf, NULL);
  CDBG("%s: MSM_CAM_IOCTL_(UN)REGISTER_PMEM ret %d", __func__, ret);
  return ret;
}

/*===========================================================================
 * FUNCTION     - take_picture -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int take_picture (char menu_item) {
  int rc;
  struct timeval tdBeforeSnapshot, tdStopPreviewVideo, tdStartSnapshot;
  struct timezone tz;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  current_ops_type = (menu_item == TAKE_JPEG_PICTURE) ?
    CAMERA_OPS_CAPTURE_AND_ENCODE : CAMERA_OPS_CAPTURE;

  g_num_captures = 1;
  g_current_capture_count = 0;
  int w_scale_factor = (snapshot_format_3d == SIDE_BY_SIDE_FULL) ?
    2 : 1;

  gettimeofday(&tdBeforeSnapshot, &tz);

  CDBG("Profiling: Before snapshot timestamp = %ld ms\n",
    (tdBeforeSnapshot.tv_sec * 1000) + (tdBeforeSnapshot.tv_usec/1000));
  gettimeofday(&tdStopPreviewVideo, &tz);
  CDBG("Profiling: Transition from preview/video to snapshot: %ld ms\n",
    (tdStopPreviewVideo.tv_sec - tdBeforeSnapshot.tv_sec) * 1000000 +
    (tdStopPreviewVideo.tv_usec - tdBeforeSnapshot.tv_usec));

  status = mCamOps.mm_camera_init(current_ops_type, NULL, NULL);
  CDBG("%s: mm_camera_init status %d", __func__, status);
  if(status)
    goto ERROR;
  thumbnail_ion_alloc.len = dimension->ui_thumbnail_width * w_scale_factor *
    CEILING16(dimension->ui_thumbnail_height) * 3/2;
  thumbnail_ion_alloc.flags = 0;
  thumbnail_ion_alloc.heap_mask = (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  thumbnail_ion_alloc.align = 4096;
#ifdef USE_ION
  thumbnail_buf = do_mmap_ion(main_ion_fd,
    &(thumbnail_ion_alloc),
    &(thumbnail_fd_data),
    &(pmemThumbnailfd));
#else
  thumbnail_buf = do_mmap(dimension->ui_thumbnail_width * w_scale_factor *
    CEILING16(dimension->ui_thumbnail_height) * 3/2,
    &pmemThumbnailfd);
#endif
  if (thumbnail_buf == NULL)
    goto ERROR;
  CDBG("do_mmap thumbnail pbuf = %p, pmem_fd = %d\n",
    thumbnail_buf, pmemThumbnailfd);
  main_ion_alloc.len = dimension->picture_width * w_scale_factor *
    CEILING16(dimension->picture_height) * 3/2;
  main_ion_alloc.flags = 0;
  main_ion_alloc.heap_mask = (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  main_ion_alloc.align = 4096;
#ifdef USE_ION
  main_img_buf = do_mmap_ion(main_ion_fd,
    &(main_ion_alloc),
    &(main_fd_data),
    &(pmemSnapshotfd));
#else
  main_img_buf = do_mmap(dimension->picture_width * w_scale_factor *
    CEILING16(dimension->picture_height) * 3/2,
    &pmemSnapshotfd);
#endif
  if (main_img_buf == NULL)
    goto ERROR;
  CDBG("do_mmap snapshot pbuf = %p, pmem_fd = %d\n", main_img_buf,
    pmemSnapshotfd);

  if ((thumbnail_buf != NULL) && (pmemThumbnailfd != 0)) {
    register_buffers(dimension->ui_thumbnail_width * w_scale_factor,
      dimension->ui_thumbnail_height, 0, 0,
      pmemThumbnailfd, thumbnail_buf, MSM_PMEM_THUMBNAIL, FALSE, TRUE);
  }

  /* For original snapshot */
  register_buffers(dimension->picture_width * w_scale_factor,
    dimension->picture_height, 0, 0,
    pmemSnapshotfd, main_img_buf, MSM_PMEM_MAINIMG, FALSE, TRUE);

  output_buffer[0].ptr = NULL;
  if (menu_item == TAKE_JPEG_PICTURE) {
    CDBG("main:%d  calling jpegEncode\n", __LINE__);
    capture_params_t capture_parms;
    capture_parms.num_captures = 1;
    capture_parms.picture_height = dimension->orig_picture_dy;
    capture_parms.picture_width = dimension->orig_picture_dx * w_scale_factor;
    capture_parms.postview_height = dimension->display_height;
    capture_parms.postview_width = dimension->display_width * w_scale_factor;
    capture_parms.thumbnail_height = dimension->ui_thumbnail_height;
    capture_parms.thumbnail_width = dimension->ui_thumbnail_width *
      w_scale_factor;
    encode_params_t encode_parms;
    encode_parms.cbcr_offset = -1;
    encode_parms.y_offset = 0;
    encode_parms.rotation = jpeg_rotation_value;
    encode_parms.quality = 85;
    encode_parms.p_output_buffer = output_buffer;
    encode_parms.exif_data = NULL;
    encode_parms.exif_numEntries = 0;
    encode_parms.format3d = 0;
    encode_parms.output_picture_height = 0;
    encode_parms.output_picture_width = 0;
    output_buffer[0].size = dimension->picture_width * w_scale_factor *
      CEILING16(dimension->picture_width) * 3/2;
    output_buffer[0].ptr = (uint8_t *)malloc(output_buffer[0].size);
    output_buffer[0].fd = -1;
    output_buffer[0].offset = 0;
    status = mCamOps.mm_camera_start(current_ops_type,(void *)&capture_parms,
      (void *)&encode_parms);
    CDBG("%s: mm_camera_start status %d", __func__, status);
    if(status)
      goto ERROR;

    pthread_mutex_lock(&snapshot_mutex);
    pthread_cond_wait(&snapshot_cond, &snapshot_mutex);
    pthread_mutex_unlock(&snapshot_mutex);
    CDBG("%s: snapshot done ", __func__);
  }

  if (menu_item == TAKE_YUV_PICTURE) {
    capture_params_t capture_parms;
    capture_parms.num_captures = 1;
    capture_parms.picture_height = dimension->orig_picture_dy;
    capture_parms.picture_width = dimension->orig_picture_dx * w_scale_factor;
    capture_parms.postview_height = dimension->display_height;
    capture_parms.postview_width = dimension->display_width * w_scale_factor;
    capture_parms.thumbnail_height = dimension->ui_thumbnail_height;
    capture_parms.thumbnail_width = dimension->ui_thumbnail_width *
      w_scale_factor;
    status = mCamOps.mm_camera_start(current_ops_type,(void *)&capture_parms,
      NULL);
    CDBG("%s: mm_camera_start status %d", __func__, status);
    if(status)
      goto ERROR;
  }

#if 0
    char postviewfile[50];
    sprintf(postviewfile, "%s/thumbnail_%d.yuv", sdcard_path,
      ++thumbnailCntr);
    FILE* fp = fopen(postviewfile, "w+");
    CDBG("%s: fp %p", __func__, fp);
    if (fp) {
      int rc = fwrite(thumbnail_buf, 1, dimension->ui_thumbnail_width *
        w_scale_factor * dimension->ui_thumbnail_height * 3/2, fp);
      CDBG("%s: rc %d", __func__, rc);
      fclose(fp);
    }
#endif

  gettimeofday(&tdStartSnapshot, &tz);
  CDBG("Profiling: Snapshot capturing latency: %ld microseconds\n",
    (tdStartSnapshot.tv_sec - tdStopPreviewVideo.tv_sec) * 1000000 +
    (tdStartSnapshot.tv_usec - tdStopPreviewVideo.tv_usec));

  status = mCamOps.mm_camera_deinit(current_ops_type, NULL, NULL);
  CDBG("%s: mm_camera_init status %d", __func__, status);
  if(status)
    goto ERROR;

  if (output_buffer[0].ptr) {
    free(output_buffer[0].ptr);
    output_buffer[0].ptr = NULL;
  }
  if ((thumbnail_buf != NULL) && (pmemThumbnailfd != 0)) {
    register_buffers(dimension->thumbnail_width * w_scale_factor,
      dimension->thumbnail_height, 0, 0,
      pmemThumbnailfd, thumbnail_buf, MSM_PMEM_THUMBNAIL, TRUE, TRUE);
  }

  /* For original snapshot */
  register_buffers(dimension->orig_picture_dx * w_scale_factor,
    dimension->orig_picture_dy, 0, 0,
    pmemSnapshotfd, main_img_buf, MSM_PMEM_MAINIMG, TRUE, TRUE);
#ifdef USE_ION
    rc = do_munmap_ion(main_ion_fd, &(thumbnail_fd_data), 
                         (void *) thumbnail_buf,
                          thumbnail_ion_alloc.len);
#else
  rc = do_munmap(pmemThumbnailfd, thumbnail_buf,
    dimension->display_width * w_scale_factor *
    CEILING16(dimension->display_height) * 3/2);
#endif
  CDBG("do_munmap thumbnail buffer return value: %d\n", rc);
#ifdef USE_ION
    rc = do_munmap_ion(main_ion_fd, &(main_fd_data), 
                         (void *) main_img_buf,
                          main_ion_alloc.len);
#else
  rc = do_munmap(pmemSnapshotfd, main_img_buf, dimension->picture_width *
    w_scale_factor * CEILING16(dimension->picture_height) * 3/2);
#endif

  CDBG("do_munmap main image buffer return value: %d\n", rc);

  return 0;

ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - take_raw_picture -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int take_raw_picture (void) {
  int rc;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  current_ops_type = CAMERA_OPS_RAW_CAPTURE;
  raw_capture_params_t raw_capture_params;

  if (hjr_status) {
    printf("RAWSNAPSHOT not supported with HJR ON!!!!\n");
    usleep(1000);
    return 0;
  }

  status = mCamOps.mm_camera_init(current_ops_type, NULL, NULL);
  CDBG("%s: mm_camera_init status %d", __func__, status);
  if(status)
    goto ERROR;
  raw_ion_alloc.len = raw_snapshot_buffer_size;
  raw_ion_alloc.flags = 0;
  raw_ion_alloc.heap_mask = (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  raw_ion_alloc.align = 4096;
#ifdef USE_ION
  raw_img_buf = do_mmap_ion(main_ion_fd,
    &(raw_ion_alloc),
    &(raw_fd_data),
    &(pmemRawSnapshotfd));
#else
  raw_img_buf = do_mmap(raw_snapshot_buffer_size, &pmemRawSnapshotfd);
#endif
  CDBG("do_mmap snapshot pbuf = %p, pmem_fd = %d\n", raw_img_buf,
    pmemRawSnapshotfd);
  if (raw_img_buf == NULL)
    goto ERROR;

  raw_capture_params.num_captures = 1;
  raw_capture_params.raw_picture_height = dimension->raw_picture_height;
  raw_capture_params.raw_picture_width = dimension->raw_picture_width;
  /* TODO update interface only to support only snapshot */
  register_buffers(dimension->raw_picture_width,
    dimension->raw_picture_height, 0, 0,
    pmemRawSnapshotfd, raw_img_buf, MSM_PMEM_RAW_MAINIMG, FALSE, TRUE);

  status = mCamOps.mm_camera_start(current_ops_type, &raw_capture_params,
    NULL);
  CDBG("%s: mm_camera_start status %d", __func__, status);
  if(status)
    goto ERROR;

  status = mCamOps.mm_camera_deinit(current_ops_type, NULL, NULL);
  CDBG("%s: mm_camera_start status %d", __func__, status);
  if(status)
    goto ERROR;

  /* TODO update interface only to support only snapshot */
  register_buffers(dimension->raw_picture_width,
    dimension->raw_picture_height, 0, 0,
    pmemRawSnapshotfd, raw_img_buf, MSM_PMEM_RAW_MAINIMG, TRUE, TRUE);
#ifdef USE_ION
    rc = do_munmap_ion(main_ion_fd, &(raw_fd_data), 
                         (void *) raw_img_buf,
                          raw_ion_alloc.len);
#else
  rc = do_munmap(pmemRawSnapshotfd, raw_img_buf,
    raw_snapshot_buffer_size);
#endif
  CDBG("do_munmap main image buffer return value: %d\n", rc);

  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - start preview -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int start_preview (void) {
  CDBG("main:start_preview!\n");
  if (mCamOps.mm_camera_start(CAMERA_OPS_STREAMING_PREVIEW, NULL, NULL)) {
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
  if (mCamOps.mm_camera_stop(CAMERA_OPS_STREAMING_PREVIEW, NULL, NULL)) {
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
int start_video (void) {

  if (current_mode == CAMERA_MODE_3D) {
    mm_camera_status_t status;
    status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_3D_MANUAL_CONV_RANGE,
      (void *)&stereo_conv_range);
    if (status != MM_CAMERA_SUCCESS)
      CDBG("%s: CAMERA_PARM_3D_MANUAL_CONV_RANGE Failed...", __func__);
  }

  CDBG("main:start_video!\n");
  if (mCamOps.mm_camera_start(CAMERA_OPS_STREAMING_VIDEO, NULL, NULL)) {
    CDBG("main:%d start_video failed!\n", __LINE__);
    goto ERROR;
  }
  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - stop_video -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int stop_video (void) {
  CDBG("main:stop_video!\n");
  if (mCamOps.mm_camera_stop(CAMERA_OPS_STREAMING_VIDEO, NULL, NULL)) {
    CDBG("main:%d stop_video failed!\n", __LINE__);
    goto ERROR;
  }
  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - start_recording -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int start_recording (void) {
  if (!recordingOn)
    recordingOn = 1;

  if (mCamOps.mm_camera_start(CAMERA_OPS_VIDEO_RECORDING, NULL, NULL)) {
    CDBG("main:%d start_recording failed!\n", __LINE__);
    goto ERROR;
  }
  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - stop_recording -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int stop_recording (void) {
  if (recordingOn)
    recordingOn = 0;

  if (mCamOps.mm_camera_stop(CAMERA_OPS_VIDEO_RECORDING, NULL, NULL)) {
    CDBG("main:%d stop_recroding failed!\n", __LINE__);
    goto ERROR;
  }
  return 0;

  ERROR:
  return -1;
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

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_CONTRAST, (void *)&contrast);
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

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_CONTRAST, (void *)&contrast);
  return 0;
}

/*===========================================================================
 * FUNCTION     - increase_saturation -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_saturation (void) {
  ++saturation;
  if (saturation > CAMERA_MAX_SATURATION) {
    saturation = CAMERA_MAX_SATURATION;
    printf("Reached max SATURATION. \n");
  } else
    printf("Increase SATURATION to %d\n", saturation);

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_SATURATION, (void *)&saturation);
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
    printf("Reached min SATURATION. \n");
  } else
    printf("Decrease SATURATION to %d\n", saturation);
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_SATURATION, (void *)&saturation);
  return 0;
}

/*===========================================================================
 * FUNCTION     - SpecialEffect -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int SpecialEffect (void) {
  effect++;
  if (effect == CAMERA_EFFECT_MAX)
    effect = CAMERA_EFFECT_OFF;

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_EFFECT, (void *)&effect);

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

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_BRIGHTNESS, (void *)&brightness);
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
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_BRIGHTNESS, (void *)&brightness);
  return 0;
}

/*===========================================================================
 * FUNCTION     - increase_EV -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_EV (void) {
  if (++ev_num <= 12) {
    int32_t ev = (ev_num << 16) | 6;
    printf("Increase EV to %d\n", ev_num);
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_EXPOSURE_COMPENSATION,
    (void *)&ev);
  } else
    printf("Reached max EV. \n");
  return 0;
}

/*===========================================================================
 * FUNCTION     - decrease_EV -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_EV (void) {
  if (--ev_num > -12) {
    int32_t ev = (ev_num << 16) | 6;
    printf("Decrease EV to %d\n", ev_num);
    mCfgControl.mm_camera_set_parm(CAMERA_PARM_EXPOSURE_COMPENSATION,
      (void *)&ev);
  } else
    printf("Reached min EV. \n");
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_antibanding -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_antibanding (void) {
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
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_ANTIBANDING,
    (void *)&antibanding);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_whitebalance -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_whitebalance (void) {
  switch (autoWB) {
    case CAMERA_WB_AUTO:
      printf("Set auto white balance to daylight.\n");
      autoWB = CAMERA_WB_DAYLIGHT;
      break;
    case CAMERA_WB_DAYLIGHT:
      printf("Set auto white balance to incandescent\n");
      autoWB = CAMERA_WB_INCANDESCENT;
      break;
    case CAMERA_WB_INCANDESCENT:
      printf("Set auto white balance to fluorescent\n");
      autoWB = CAMERA_WB_FLUORESCENT;
      break;
    case CAMERA_WB_FLUORESCENT:
      printf("Set auto white balance to auto\n");
      autoWB = CAMERA_WB_AUTO;
      break;
    default:
      break;
  }
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_WHITE_BALANCE,
    (void *)&autoWB);
  return 0;
}

/*===========================================================================
 * FUNCTION     - AEC_mode_change -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int AEC_mode_change (void) {
  aec_mode++;
  if (aec_mode == CAMERA_AEC_MAX_MODES)
    aec_mode = CAMERA_AEC_FRAME_AVERAGE;

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_EXPOSURE, (void *)&aec_mode);
  return 0;
}

/*===========================================================================
 * FUNCTION     - increase_ISO -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_ISO (void) {
  iso++;
  if (iso >= max_camera_iso_type)
    iso = max_camera_iso_type;

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_ISO, (void *)&iso);
  printf("Increase ISO to %d\n", iso);
  return 0;
}

/*===========================================================================
 * FUNCTION     - decrease_ISO -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_ISO (void) {
  if (iso > CAMERA_ISO_AUTO)
    iso--;
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_ISO, (void *)&iso);
  printf("Decrease ISO to %d\n", iso);
  return 0;
}

/*===========================================================================
 * FUNCTION     - increase_sharpness -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int increase_sharpness (void) {
  sharpness += CAMERA_SHARPNESS_STEP;
  if (sharpness > CAMERA_MAX_SHARPNESS) {
    sharpness = CAMERA_MAX_SHARPNESS;
    printf("Reached max SHARPNESS. \n");
  } else
    printf("Increase SHARPNESS to %d\n", sharpness);

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_SHARPNESS, (void *)&sharpness);
  return 0;
}

/*===========================================================================
 * FUNCTION     - decrease_sharpness -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int decrease_sharpness (void) {
  sharpness -= CAMERA_SHARPNESS_STEP;
  if (sharpness < CAMERA_MIN_SHARPNESS) {
    sharpness = CAMERA_MIN_SHARPNESS;
    printf("Reached min SHARPNESS. \n");
  } else
    printf("Decrease SHARPNESS to %d\n", sharpness);

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_SHARPNESS, (void *)&sharpness);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_auto_focus -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_auto_focus (void) {

  switch (af_mode) {
    case AF_MODE_MACRO:
      af_mode = AF_MODE_NORMAL;
      printf("Auto focus: Normal Mode.\n");
      break;

    case AF_MODE_NORMAL:
      af_mode = AF_MODE_MACRO;
      printf("Auto focus: Macro Mode.\n");
      break;

    default:
      break;
  }

  af_ctrl.af_cb = afStatus_ctrl;
  af_ctrl.af_mode = af_mode;

  CDBG("%s: Spawning new thread for AF \n", __func__);
  if (launch_camafctrl_thread(&af_ctrl)) {
    CDBG("main: launch_camframe_fb_thread failed!\n");
  }
  CDBG("Exiting AF\n");
  return 0;
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
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_HJR, (void *)&hjr_status);
  return 0;
}

/*===========================================================================
 * FUNCTION     - LensShading -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int LensShading (void) {
  if (lens_shading_enabled == TRUE)
    lens_shading_enabled = FALSE;
  else
    lens_shading_enabled = TRUE;

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_ROLLOFF,
    (void *)&lens_shading_enabled);
  return 0;
}

/*===========================================================================
 * FUNCTION     - LED_mode_change -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int LED_mode_change (void) {
  led_mode ++;
  if (led_mode >= LED_MODE_MAX)
    led_mode = LED_MODE_OFF;
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_LED_MODE, (void *)&led_mode);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_sharpness_AF -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_sharpness_AF (void) {
  intrfcCtrl.getSharpness_AF(camfd, sharpness_AF);
  printf("\n\n AF sharpness = %d\n", *sharpness_AF);
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
    & dimension->display_width,
    & dimension->display_height);

  CDBG("Selected preview/video resolution is %s\n", resolution_name);

  if (resolution_name == NULL) {
    CDBG("main:%d set_preview_dimension failed!\n", __LINE__);
    goto ERROR;
  }

  CDBG("Selected Preview Resolution: display_width = %d, display_height = %d\n",
    dimension->display_width, dimension->display_height);

  input_display.user_input_display_width = dimension->display_width;
  input_display.user_input_display_height = dimension->display_height;

  preview_video_resolution_flag = 1;
  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - snapshot_resolution -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int snapshot_resolution (int snapshot_action_param) {
  char * resolution_name;
  CDBG("Selecting the action for snapshot resolution = %d \n", snapshot_action_param);
  resolution_name = set_snapshot_dimension_tbl(snapshot_action_param, & dimension->picture_width, & dimension->picture_height);

  if (resolution_name == NULL) {
    CDBG("main:%d set_snapshot_dimension failed!\n", __LINE__);
    goto ERROR;
  }

  CDBG("Selected snapshot resolution is %s\n", resolution_name);

  CDBG("Selected snapshot resolution: dimension->picture_width %d, dimension->picture_height %d\n",
    dimension->picture_width, dimension->picture_height);

  if (dimension->picture_width == SQCIF_WIDTH &&
    dimension->picture_height == SQCIF_HEIGHT) {
    dimension->ui_thumbnail_width = SQCIF_WIDTH;
    dimension->ui_thumbnail_height = SQCIF_HEIGHT;
  } else if (dimension->picture_width == QCIF_WIDTH &&
    dimension->picture_height == QCIF_HEIGHT) {
    dimension->ui_thumbnail_width = QCIF_WIDTH;
    dimension->ui_thumbnail_height = QCIF_HEIGHT;
  } else if (dimension->picture_width == CIF_WIDTH &&
    dimension->picture_height == CIF_HEIGHT) {
    dimension->ui_thumbnail_width = QCIF_WIDTH;
    dimension->ui_thumbnail_height = QCIF_HEIGHT;
  } else if ((dimension->picture_width == HD1080_WIDTH &&
    dimension->picture_height == HD1080_HEIGHT) ||
    (dimension->picture_width == HD720_WIDTH &&
    dimension->picture_height == HD720_HEIGHT)) {
    dimension->ui_thumbnail_width = HD_THUMBNAIL_WIDTH;
    dimension->ui_thumbnail_height = HD_THUMBNAIL_HEIGHT;
  } else if (dimension->picture_width == MP12_WIDTH &&
    dimension->picture_height == MP12_HEIGHT) {
    dimension->ui_thumbnail_width = 512;
    dimension->ui_thumbnail_height = 384;
  } else {
    dimension->ui_thumbnail_width = 512;
    dimension->ui_thumbnail_height = 384;
  }

  if (mCfgControl.mm_camera_set_parm(CAMERA_PARM_DIMENSION,
    (void *)dimension)) {
    CDBG("main:%d setDimension failed!\n", __LINE__);
    goto ERROR;
  } else {
    CDBG("Set the dimesion for SnapShot Resolution\n");
  }
  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - set_MotionIso -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_MotionIso(void) {
  if (motion_iso == MOTION_ISO_OFF)
    motion_iso = MOTION_ISO_ON;
  else
    motion_iso = MOTION_ISO_OFF;
  intrfcCtrl.setMotionIso(camfd, motion_iso);
  printf("\n\n Motion iso = %d\n", motion_iso);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_fps_mode -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_fps_mode(void) {
  if (fps_mode == FPS_MODE_AUTO)
    fps_mode = FPS_MODE_FIXED;
  else
    fps_mode = FPS_MODE_AUTO;
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_FPS_MODE, (void *)&fps_mode);
  printf("\n\n Fps mode = %d,(AUTO=0, FIXED=1)\n", fps_mode);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_fps -
 *
 * DESCRIPTION: set fps when fixed fps mode, or set max fps when AFR enabled
 * ===========================================================================*/
static int set_fps(uint32_t fps) {
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_FPS, (void *)&fps);
  printf("\n\n Set fps = 0x%x\n", fps);
  return 0;
}

/*===========================================================================
 * FUNCTION     - toggle_hue -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int toggle_hue(void)
{
  hue += CAMERA_HUE_STEP;
  if (hue > CAMERA_MAX_HUE)
    hue = CAMERA_MIN_HUE;
  intrfcCtrl.setHue(camfd, hue);
  printf("\n\n Hue = %d\n", hue);
  return 0;
}
/*===========================================================================
     * FUNCTION     - set_bestshot_mode -
     *
     * DESCRIPTION:
* ===========================================================================*/
int set_bestshotmode (uint8_t bestshot_mode)
{
  intrfcCtrl.setBestShotMode(camfd, bestshot_mode);
  return 0;
}

/*===========================================================================
     * FUNCTION     - set_facedetection -
     *
     * DESCRIPTION:
* ===========================================================================*/
int toggle_facedetection(void)
{
  face_detection = (!face_detection);
  CDBG("toggle_facedetection %d\n", face_detection);
  intrfcCtrl.setFaceDetection(camfd, face_detection);
  CDBG("toggle_facedetection exitn");
  return 0;
}

/*===========================================================================
     * FUNCTION     - toggle_camera_mode -
     *
     * DESCRIPTION:
* ===========================================================================*/
int toggle_camera_mode(void)
{
  camera_mode_t mode;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("%s: modes_supported %d current mode %d\n", __func__, modes_supported,
    current_mode);
  if (!(modes_supported & CAMERA_MODE_3D)) {
    printf("Cannot switch modes");
    goto EXIT;
  }
  if (CAMERA_MODE_3D == current_mode) {
    mode = CAMERA_MODE_2D;
  } else {
    mode = CAMERA_MODE_3D;
  }

  if (stop_video() != 0) {
    CDBG("%s: stop video fail", __func__);
    goto EXIT;
  }

  status = mm_camera_deinit();
  if (status) {
    CDBG_ERROR("%s: mm_camera_deinit fail", __func__);
    goto EXIT;
  }

  status = mCfgControl.mm_camera_set_parm(CAMERA_PARM_MODE,
    (void *)&mode);
  if (status) {
    CDBG_HIGH("%s: set CAMERA_PARM_MODE fail", __func__);
    goto EXIT;
  }

  status = mm_camera_exec();
  if (status) {
    CDBG_ERROR("%s: mm_camera_deinit fail", __func__);
    goto EXIT;
  }

  if (start_video() != 0) {
    CDBG_ERROR("%s: start_video fail", __func__);
    goto EXIT;
  }

  current_mode = mode;
EXIT:
  CDBG("%s: Exit %d", __func__, current_mode);
  return 0;
}

/*===========================================================================
 * FUNCTION     - toggle_Jpeg_Rotation -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int toggle_Jpeg_Rotation(void)
{
  jpeg_rotation_value = (jpeg_rotation_value+90)%360;
  intrfcCtrl.setJpegRotation(jpeg_rotation_value);
  printf("\n\n Jpeg Rotation = %d\n", jpeg_rotation_value);
  return 0;
}

/*===========================================================================
 * FUNCTION     - query_aec_flash_for_snapshot -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int query_aec_flash_for_snapshot(void)
{
  int is_flash_needed = 0;
  mm_camera_status_t status;
  status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_QUERY_FALSH4SNAP,
        (void *)&is_flash_needed);
  printf("\n\n query flash  status= %d, flash=%d\n", status, is_flash_needed);

  if(is_flash_needed) {
    return true;
  }
  return 0;
} /* query_aec_flash_for_snapshot */

/*===========================================================================
 * FUNCTION     - startliveshot -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int startliveshot(void)
{
  int rc = 0;
  uint32_t maxjpegsize =  dimension->video_width * dimension->video_height * 1.5;
  liveshot_ptr = (uint8_t*)malloc(maxjpegsize);
  if (!liveshot_ptr) {
    printf("\n startliveshot failed no memory\n");
    return -1;
  }
  printf("\n startliveshot w %d h %d \n", dimension->video_width, dimension->video_height);
  rc = intrfcCtrl.startLiveShot(camfd, liveshot_ptr, maxjpegsize, dimension->video_width,
    dimension->video_height);
  printf("\n startliveshot = %d \n", rc);
  return 0;
}

/*===========================================================================
 * FUNCTION     - toggle_Strobe_Flash_Mode -
 *
 * DESCRIPTION:
 * ===========================================================================*/
static int toggle_Strobe_Flash_Mode(void)
{
  strobe_flash_mode++;
  if(strobe_flash_mode == STROBE_FLASH_MODE_MAX) {
    strobe_flash_mode = STROBE_FLASH_MODE_OFF;
  }
  intrfcCtrl.setStrobeFlashMode(camfd, strobe_flash_mode);
  printf("\n\n Strobe_Flash_Mode = %d\n", strobe_flash_mode);
  return 0;
}

/*===========================================================================
 * FUNCTION     - cancel_af -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int cancel_af(void)
{
  printf("Cancel Auto Focus\n");
  intrfcCtrl.cancelAF(camfd);
  return 0;
}

/*===========================================================================
 * FUNCTION     - get_af_step -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int get_af_step(void)
{
  int32_t afStep = 0;
  intrfcCtrl.getAfStep(camfd, &afStep);
  printf("\n\n Get AF step = %d\n", afStep);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_af_step -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_af_step(void)
{
  if (steps == -1) {
    if (--position <= 0)
      steps = 1;
  } else {
    if (++position >= 36)
      steps = -1;
  }
  intrfcCtrl.setAfStep(camfd, steps);
  printf("\n\n Moved to %d, step is %d\n", position, steps);
  return 0;
}

/*===========================================================================
 * FUNCTION     - enable_afd -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int enable_afd(void)
{
  intrfcCtrl.enableAFD(camfd);
  return 0;
}

/*===========================================================================
 * FUNCTION     - print_maxzoom -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int print_maxzoom()
{
  cam_parm_info_t zoomInfo;

  intrfcCtrl.getMaxZoom(camfd, &(zoomInfo.maximum_value));
  printf("main: Max Zoom value is: %d \n", zoomInfo.maximum_value);

  return 0;
}

/*===========================================================================
 * FUNCTION     - print_zoomratios -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int print_zoomratios()
{
  cam_parm_info_t zoomInfo;
  int32_t i;

  intrfcCtrl.getMaxZoom(camfd, &(zoomInfo.maximum_value));

  zoomRatios = (int16_t *) malloc (sizeof(int16_t) * zoomInfo.maximum_value);
  if (!zoomRatios) {
    printf("Failed to get zoomratios...report upper layers accordingly");
    return 0;
  }
  memset(zoomRatios, 0, sizeof(int16_t) * zoomInfo.maximum_value);

  if (intrfcCtrl.getZoomRatio(camfd, zoomInfo.maximum_value, (void *) zoomRatios) != true) {
    printf("Failed to get zoomratios...report upper layers accordingly");
  }

  for (i=0; i<zoomInfo.maximum_value; i++) {
    printf("ZoomRatios[%d] = %d \n", i, zoomRatios[i]);
  }

  free(zoomRatios);

  return 0;
}
/*===========================================================================
 * FUNCTION     - zoom_increase -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int zoom_increase(int zoomflag)
{
  cam_parm_info_t zoomInfo;

  zoomInfo.default_value = 0;
  zoomInfo.minimum_value = 0;
  zoomInfo.step_value    = 3;
  zoomInfo.current_value  = g_camParmInfo_current_value;

  intrfcCtrl.getMaxZoom(camfd, &(zoomInfo.maximum_value));
  CDBG("main:Zoom value is: %d %d\n", zoomInfo.current_value,
    zoomInfo.maximum_value);

  zoomInfo.current_value += zoomInfo.step_value;
  CDBG("Current value %d\n", zoomInfo.current_value);
  if (zoomflag == 1) {
    while (zoomInfo.current_value <= zoomInfo.maximum_value) {
      if (mCfgControl.mm_camera_set_parm(CAMERA_PARM_ZOOM,
        (void *)&zoomInfo.current_value)
        != MM_CAMERA_SUCCESS) {
        CDBG("main:%d setZoom failed!\n", __LINE__);
        goto ERROR;
      }
      printf("Increase zoom: value is %d \n", zoomInfo.current_value);
      usleep(100 * 1000);
      zoomInfo.current_value += zoomInfo.step_value;
      g_camParmInfo_current_value = zoomInfo.current_value;
    }
  } else {
    if (zoomInfo.current_value <= zoomInfo.maximum_value) {
      if (mCfgControl.mm_camera_set_parm(CAMERA_PARM_ZOOM,
        (void *)&zoomInfo.current_value)
        != MM_CAMERA_SUCCESS) {
        CDBG("main:%d setZoom failed!\n", __LINE__);
        goto ERROR;
      }
      printf("Increase zoom: value is %d \n", zoomInfo.current_value);
      usleep(100 * 1000);
      g_camParmInfo_current_value = zoomInfo.current_value;
    } else {
      printf("Increase zoom: the value is maximum now\n");
      goto ERROR;
    }
  }

  ZoomDump = 1;

  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - zoom_decrease -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int zoom_decrease(int zoomflag)
{
  cam_parm_info_t zoomInfo;

  zoomInfo.default_value = 0;
  zoomInfo.minimum_value = 0;
  zoomInfo.step_value    = 3;
  zoomInfo.current_value  = g_camParmInfo_current_value;

  intrfcCtrl.getMaxZoom(camfd, &(zoomInfo.maximum_value));
  CDBG("main:Zoom value is: %d %d %d \n", zoomInfo.current_value,
    zoomInfo.minimum_value, zoomInfo.maximum_value);

  if (!g_camParmInfo_current_value) {
    zoomInfo.current_value = zoomInfo.maximum_value;
    CDBG("Current value %d\n", zoomInfo.current_value);
  }

  zoomInfo.current_value -= zoomInfo.step_value;
  CDBG("Current value %d\n", zoomInfo.current_value);
  if (zoomflag == 1) {
    while (zoomInfo.current_value >= zoomInfo.minimum_value) {
      if (mCfgControl.mm_camera_set_parm(CAMERA_PARM_ZOOM,
        (void *)&zoomInfo.current_value)
        != MM_CAMERA_SUCCESS) {
        CDBG("main:%d setZoom failed!\n", __LINE__);
        goto ERROR;
      }
      printf("Decrease zoom: value is %d \n", zoomInfo.current_value);
      usleep(100 * 1000);
      zoomInfo.current_value -= zoomInfo.step_value;
      g_camParmInfo_current_value = zoomInfo.current_value;
    }
  } else {
    if (zoomInfo.current_value >= zoomInfo.minimum_value) {
      if (mCfgControl.mm_camera_set_parm(CAMERA_PARM_ZOOM,
        (void *)&zoomInfo.current_value)
        != MM_CAMERA_SUCCESS) {
        CDBG("main:%d setZoom failed!\n", __LINE__);
        goto ERROR;
      }
      printf("Decrease zoom: value is %d \n", zoomInfo.current_value);
      usleep(100 * 1000);
      g_camParmInfo_current_value = zoomInfo.current_value;
    } else {
      printf("Decrease zoom: the value is minimum now\n");
      goto ERROR;
    }
  }
  ZoomDump = 1;

  return 0;

  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - roi_set_by_index -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int roi_set_by_index(uint32_t  roc_index)
{
  int rc;
  cam_set_aec_roi_t cam_set_aec_roi;

  cam_set_aec_roi.aec_roi_enable = TRUE;
  cam_set_aec_roi.aec_roi_type = AEC_ROI_BY_INDEX;
  cam_set_aec_roi.aec_roi_position.aec_roi_idx = roc_index;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_AEC_ROI,
    (void *)&cam_set_aec_roi) == MM_CAMERA_SUCCESS);

  return rc;
} /* roi_set_by_index */

/*===========================================================================
 * FUNCTION     - roi_set_by_location -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int roi_set_by_location(uint32_t  x, uint32_t y)
{
  int rc;
  cam_set_aec_roi_t cam_set_aec_roi;

  cam_set_aec_roi.aec_roi_enable = TRUE;
  cam_set_aec_roi.aec_roi_type = AEC_ROI_BY_COORDINATE;
  cam_set_aec_roi.aec_roi_position.coordinate.x = x;
  cam_set_aec_roi.aec_roi_position.coordinate.y = y;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_AEC_ROI,
    (void *)&cam_set_aec_roi) == MM_CAMERA_SUCCESS);

  return rc;
} /* roi_set_by_location */

/*===========================================================================
 * FUNCTION     - roi_disable -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int aec_roi_disable( void )
{
  int rc;
  cam_set_aec_roi_t cam_set_aec_roi;

  cam_set_aec_roi.aec_roi_enable = FALSE;
  /*below setting does not matter*/
  cam_set_aec_roi.aec_roi_type = 0;
  cam_set_aec_roi.aec_roi_position.aec_roi_idx = 0;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_AEC_ROI,
    (void *)&cam_set_aec_roi) == MM_CAMERA_SUCCESS);

  usleep(100 * 1000);
  return rc;
} /* roi_disable */

/*===========================================================================
 * FUNCTION     - roiIndex_set -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int roiIndex_set()
{
  int32_t CurrROIindx = 255;
  char input[BUFF_SIZE_20];
  printf("Enter the ROI Index (Between 0 and 255):");
  fgets(input, BUFF_SIZE_20, stdin);
  CurrROIindx = atoi(input);
  printf("CurrROIindx is %d\n", CurrROIindx);
  if (CurrROIindx < 0 || CurrROIindx > 255) {
    printf("Incorrect index entered, returning..\n");
    return -1;
  }
  if (roi_set_by_index(CurrROIindx) < 0) {
    CDBG("main:%d setAecROI failed!\n", __LINE__);
    goto ERROR;
  }
  usleep(500 * 1000);
  return 0;
  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - roiIndex_set_location -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int roiIndex_set_location()
{
  int32_t x = 0;
  int32_t y = 0;
  char input[BUFF_SIZE_20];
  printf("Enter the ROI x (Between 0 and %d):", dimension->display_width);
  fgets(input, BUFF_SIZE_20, stdin);
  x = atoi(input);
  printf("x is %d\n", x);
  if (x < 0 || x > dimension->display_width) {
    printf("Incorrect x entered, returning..\n");
    return -1;
  }
  printf("Enter the ROI y (Between 0 and %d):", dimension->display_height);
  fgets(input, BUFF_SIZE_20, stdin);
  y = atoi(input);
  printf("y is %d\n", y);
  if (y < 0 || y > dimension->display_height) {
    printf("Incorrect y entered, returning..\n");
    return -1;
  }
  if (roi_set_by_location(x, y) < 0) {
    CDBG("main:%d setAecROI failed!\n", __LINE__);
    goto ERROR;
  }
  usleep(500 * 1000);
  return 0;
  ERROR:
  return -1;
}

/*===========================================================================
 * FUNCTION     - set_af_focusrect -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_af_focusrect(void)
{
  switch (afFocusRect) {
    case AUTO:
      printf("Set focus rectanlge to SPOT.\n");
      afFocusRect = SPOT;
      break;

    case SPOT:
      printf("Set focus rectanlge to CENTER_WEIGHTED.\n");
      afFocusRect = CENTER_WEIGHTED;
      break;

    case CENTER_WEIGHTED:
      printf("Set focus rectanlge to AVERAGE.\n");
      afFocusRect = AVERAGE;
      break;

    case AVERAGE:
      printf("Set focus rectanlge to AUTO.\n");
      afFocusRect = AUTO;
      break;

    default:
      break;
  }
  printf("Set focus rectanlge to %d.\n", afFocusRect);
  mCfgControl.mm_camera_set_parm(CAMERA_PARM_FOCUS_RECT,
    (void *)&afFocusRect);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_CAF -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_CAF(void)
{
  int rc = 0;
  if (caf_ctrl == CAF_OFF)
    caf_ctrl = CAF_ON;
  else
    caf_ctrl = CAF_OFF;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_CONTINUOUS_AF,
    (void *)&caf_ctrl) == MM_CAMERA_SUCCESS);
  printf("\n\n CAF = %d rc %d\n", caf_ctrl, rc);
  return 0;
}

/*===========================================================================
 * FUNCTION     - setAF_ROI -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int setAF_ROI(void)
{
  uint8_t i = 0;
  int32_t num_roi = 0, x = 0, y = 0, dx = 0, dy = 0;
  char input[BUFF_SIZE_20];

  printf("Enter number of AF ROIs (max is %d):", MAX_ROI);
  fgets(input, BUFF_SIZE_20, stdin);
  num_roi = atoi(input);
  printf("num_roi=%d\n", num_roi);
  if (num_roi > MAX_ROI) {
    printf("Max. numer of ROIs are geater than %d \n", MAX_ROI);
    return -1;
  }

  roi_ctrl.num_roi = (uint16_t)num_roi;
  for (i = 0; i < (int)num_roi; i++) {
    /* X */
    printf("Enter X for AF ROI Index (Between 0 and %d):",
      dimension->display_width);
    fgets(input, BUFF_SIZE_20, stdin);
    x = atoi(input);
    printf("x is %d\n", x);
    if (x < 0 || x > dimension->display_width) {
      printf("Entered Y is incorrect, returning..\n");
      return -1;
    }

    /* Y */
    printf("Enter Y for AF ROI Index (Between 0 and %d):",
      dimension->display_height);
    fgets(input, BUFF_SIZE_20, stdin);
    y = atoi(input);
    printf("y is %d\n", y);
    if (y < 0 || y > dimension->display_height) {
      printf("Entered Y is incorrect, returning..\n");
      return -1;
    }

    /* Delta X */
    printf("Enter Delta X for AF ROI Index (Between %d and %d):",
      x, dimension->display_width);
    fgets(input, BUFF_SIZE_20, stdin);
    dx = atoi(input);
    printf("dx is %d\n", dx);
    if (dx < x ||
      dx > dimension->display_width) {
      printf("Entered DX is incorrect, returning..\n");
      return -1;
    }

    /* Delta Y */
    printf("Enter Delta Y for AF ROI Index (Between %d and %d):",
      y, dimension->display_height);
    fgets(input, BUFF_SIZE_20, stdin);
    dy = atoi(input);
    printf("dy is %d\n", dy);
    if (dy < y ||
      dy > dimension->display_height) {
      printf("Entered DY is incorrect, returning..\n");
      return -1;
    }

    roi_ctrl.roi[i].x = (uint16_t)x;
    roi_ctrl.roi[i].y = (uint16_t)y;
    roi_ctrl.roi[i].dx = (uint16_t)dx;
    roi_ctrl.roi[i].dy = (uint16_t)dy;
    printf("i=%d, x=%d, y=%d, dx=%d, dy=%d \n", i, roi_ctrl.roi[i].x,
      roi_ctrl.roi[i].y, roi_ctrl.roi[i].dx, roi_ctrl.roi[i].dy);
  }

  if (mCfgControl.mm_camera_set_parm(CAMERA_PARM_AF_ROI,
    (void *)&roi_ctrl) != MM_CAMERA_SUCCESS) {
    CDBG("%s: failed", __func__);
  }
  return 0;
}



/*===========================================================================
 * FUNCTION     - enable_la -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int enable_la()
{
  uint32_t LA_Enable = TRUE;
  intrfcCtrl.enableLA(camfd, &LA_Enable);
  return 0;
}

/*===========================================================================
 * FUNCTION     - disable_la -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int disable_la()
{
  uint32_t LA_Enable = FALSE;
  intrfcCtrl.disableLA(camfd,&LA_Enable);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_bl_detection -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_bl_detection(uint8_t bl_detection_enable)
{
  int rc;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_SNOW_DETECTION,
    (void *)&bl_detection_enable) == MM_CAMERA_SUCCESS);
  return rc;
}

/*===========================================================================
 * FUNCTION     - set_snow_detection -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_snow_detection(uint8_t snow_detection_enable)
{
  int rc;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_SNOW_DETECTION,
    (void *)&snow_detection_enable) == MM_CAMERA_SUCCESS);
  return rc;
}

/*===========================================================================
 * FUNCTION     - get_af_step -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int get_focus_distances(void)
{
  focus_distances_info_t focusDistanceInfo;
  mm_camera_status_t status;

  //intrfcCtrl.getFocusDistances(camfd, &focusDistanceInfo);
  status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_FOCUS_DISTANCES,
    (void *)&focusDistanceInfo);
  if (status == MM_CAMERA_SUCCESS)
    printf("\n\n Get Focus Distances = %f %f %f\n",
      focusDistanceInfo.focus_distance[FOCUS_DISTANCE_NEAR_INDEX],
      focusDistanceInfo.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX],
      focusDistanceInfo.focus_distance[FOCUS_DISTANCE_FAR_INDEX]);
  else
    printf("\n\n%s: Failed...", __func__);
  return 0;
}

/*===========================================================================
 * FUNCTION     - get_focal_length -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int get_focal_length()
{
  mm_camera_status_t status;
  float focalLength = 0.0f;
  status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_FOCAL_LENGTH,
    (void *)&focalLength);
  printf("\n\n Focal Length %f \n", focalLength);
  return 0;
}

/*===========================================================================
 * FUNCTION     - get_horizontal_view_angle -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int get_horizontal_view_angle()
{
  mm_camera_status_t status;
  float horizontalViewAngle = 0.0f;
  status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_HORIZONTAL_VIEW_ANGLE,
    (void *)&horizontalViewAngle);
  printf("\n\n Horizontal View Angle %f \n", horizontalViewAngle);
  return 0;
}

/*===========================================================================
 * FUNCTION     - get_vertical_view_angle -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int get_vertical_view_angle()
{
  mm_camera_status_t status;
  float verticalViewAngle = 0.0f;
  status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_VERTICAL_VIEW_ANGLE,
    (void *)&verticalViewAngle);
  printf("\n\n Vertical View Angle %f \n", verticalViewAngle);
  return 0;
}

/*===========================================================================
 * FUNCTION     - reset_lens_to_infinity -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int reset_lens_infinity()
{
    mm_camera_status_t status;
    status = mCfgControl.mm_camera_set_parm(CAMERA_PARM_RESET_LENS_TO_INFINITY,
            NULL);
    if(status == MM_CAMERA_SUCCESS){
        printf("\n\n reset lens to infinity success ");
    } else {
        printf("\n\n reset lens to infinity failure ");
    }
    return 0;
}

/*===========================================================================
 * FUNCTION     - get_snapshot_data -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int get_snapshot_data()
{
  mm_camera_status_t status;
  snapshotData_info_t snapshot_data;
  status = mCfgControl.mm_camera_get_parm(CAMERA_PARM_SNAPSHOTDATA,
    (void *)&snapshot_data);
  printf("\n\n Snapshot data:\n AF:\n Focus position = %d\n \
AEC:\n Line Count=%d\t Luma target=%d\t exposure time=%f\t exposure mode=%d\t \
exposure program=%d\n \
AWB:\n R_gain=%d\t B_gain=%d\t G_gain=%d\n",
    snapshot_data.focus_position,
    snapshot_data.line_count, snapshot_data.luma_target,
    snapshot_data.exposure_time, snapshot_data.exposure_mode,
    snapshot_data.exposure_program,
    snapshot_data.r_gain, snapshot_data.g_gain, snapshot_data.b_gain);
  return 0;
}

/*===========================================================================
 * FUNCTION     - set_3d_display_distance -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_3d_display_distance(uint32_t distance)
{
  int rc;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_3D_DISPLAY_DISTANCE,
    (void *)&distance) == MM_CAMERA_SUCCESS);
  return rc;
}

/*===========================================================================
 * FUNCTION     - update_preview_format -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int update_preview_format()
{
  int rc;
  int preview_format;
  cam_format_t format;
  char input[BUFF_SIZE_20];
  printf("\n Enter the preview format 1.NV12 2.NV21");
  fgets(input, BUFF_SIZE_20, stdin);
  preview_format = atoi(input);
  format = (preview_format == 1) ? CAMERA_YUV_420_NV12 :
    CAMERA_YUV_420_NV21;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_PREVIEW_FORMAT,
    (void *)&format) == MM_CAMERA_SUCCESS);
  /* wait for hundred ms to make sure that format is updated*/
  usleep(100000);
  dump_restart_flag = true;
  return rc;
}

/*===========================================================================
 * FUNCTION     - set_3d_view_angle -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int set_3d_view_angle(uint32_t angle)
{
  int rc;
  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_3D_VIEW_ANGLE,
    (void *)&angle) == MM_CAMERA_SUCCESS);
  return rc;
}

/*===========================================================================
 * FUNCTION     - set_3d_conv_control -
 *
 * DESCRIPTION:
 * ===========================================================================*/
void set_3d_conv_control(int auto_conv)
{
  static int manual_conv = 0, conv_value = 0;
  mm_camera_status_t status;
  char input[BUFF_SIZE_20];

  if (auto_conv && manual_conv) {
    manual_conv = 0;
    status =
      mCfgControl.mm_camera_set_parm(CAMERA_PARM_ENABLE_3D_MANUAL_CONVERGENCE,
      (void *)&manual_conv);
    if (status != MM_CAMERA_SUCCESS)
      CDBG_HIGH("%s: Auto conv control failed with status = %d\n",
        __func__, status);
  } else if (!auto_conv) {
    if (!manual_conv) {
      manual_conv = 1;
      status =
        mCfgControl.mm_camera_set_parm(CAMERA_PARM_ENABLE_3D_MANUAL_CONVERGENCE,
        (void *)&manual_conv);
      if (status != MM_CAMERA_SUCCESS) {
        CDBG_HIGH("%s: Manual conv control failed with status = %d\n",
          __func__, status);
        return;
      }
    }

    printf("Enter the new convergence value (Between 0 and %d): ",
      stereo_conv_range);
    fgets(input, BUFF_SIZE_20, stdin);
    conv_value = atoi(input);
    printf("\nYou entered convergence value = %d\n", conv_value);

    status =
      mCfgControl.mm_camera_set_parm(CAMERA_PARM_3D_MANUAL_CONV_VALUE,
      (void *)&conv_value);
    if (status != MM_CAMERA_SUCCESS) {
      CDBG_HIGH("%s: Setting Manual conv value failed with status = %d\n",
        __func__, status);
      return;
    }
  }
}

/*===========================================================================
 * FUNCTION     - make_skin_paler -
 *
 * DESCRIPTION:
 * ==========================================================================*/
int make_skin_paler()
{
  SCE_Adj = SCE_Adj + 10;
  if (SCE_Adj > CAMERA_MAX_SCE_FACTOR) {
    SCE_Adj = CAMERA_MAX_SCE_FACTOR;
    printf("SCE adjustment factor is %d\n", SCE_Adj);
    printf("Reached Max SCE factor. \n");
  } else
    printf("SCE adjustment factor is %d\n", SCE_Adj);

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_SCE_FACTOR,
    (void *)&SCE_Adj);
  return 0;
}

/*===========================================================================
 * FUNCTION     - make_skin_tanner -
 *
 * DESCRIPTION:
 * ==========================================================================*/
int make_skin_tanner()
{
  SCE_Adj = SCE_Adj - 10;
  if (SCE_Adj < CAMERA_MIN_SCE_FACTOR) {
    SCE_Adj = CAMERA_MIN_SCE_FACTOR;
    printf("SCE adjustment factor is %d\n", SCE_Adj);
    printf("Reached Min SCE factor. \n");
  } else
    printf("SCE adjustment factor is %d\n", SCE_Adj);

  mCfgControl.mm_camera_set_parm(CAMERA_PARM_SCE_FACTOR,
    (void *)&SCE_Adj);
  return 0;

}

/*===========================================================================
 * FUNCTION     - video_rotation_config -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int video_rotation_config(uint16_t rotation)
{
  int rc;
  video_rotation_param_ctrl_t rotCtrl;
  switch (rotation) {
    case 0:
      rotCtrl.rotation = ROT_NONE;
      break;
    case 90:
      rotCtrl.rotation = ROT_CLOCKWISE_90;
      break;
    case 180:
      rotCtrl.rotation = ROT_CLOCKWISE_180;
      break;
    case 270:
      rotCtrl.rotation = ROT_CLOCKWISE_270;
      break;
    default:
      printf("\nInvalid rotation value\n");
      return -1;
  }

  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_VIDEO_ROT,
    (void *)&rotCtrl) == MM_CAMERA_SUCCESS);
  return rc;
}

/*===========================================================================
 * FUNCTION     - video_dis_configuration -
 *
 * DESCRIPTION:
 * ===========================================================================*/
int video_dis_configuration(uint32_t dis_enable)
{
  int rc;
  video_dis_param_ctrl_t disCtrl;

  disCtrl.dis_enable = dis_enable;
  disCtrl.video_rec_width = dimension->orig_video_width;
  disCtrl.video_rec_height = dimension->orig_video_height;
  disCtrl.output_cbcr_offset = dimension->orig_video_width * dimension->orig_video_height;

  rc = (mCfgControl.mm_camera_set_parm(CAMERA_PARM_VIDEO_DIS,
    (void *)&disCtrl) == MM_CAMERA_SUCCESS);
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
  } else if (current_menu_id == MENU_ID_ZOOMCHANGE) {
    camera_zoom_change_tbl ();
  } else if (current_menu_id == MENU_ID_CONTRASTCHANGE) {
    camera_contrast_change_tbl ();
  } else if (current_menu_id == MENU_ID_SATURATIONCHANGE) {
    camera_saturation_change_tbl ();
  } else if (current_menu_id == MENU_ID_BRIGHTNESSCHANGE) {
    camera_brightness_change_tbl ();
  } else if (current_menu_id == MENU_ID_EVCHANGE) {
    camera_EV_change_tbl ();
  } else if (current_menu_id == MENU_ID_ISOCHANGE) {
    camera_ISO_change_tbl ();
  } else if (current_menu_id == MENU_ID_SHARPNESSCHANGE) {
    camera_sharpness_change_tbl ();
  } else if (current_menu_id == MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE) {
    camera_preview_video_resolution_change_tbl ();
  } else if (current_menu_id == MENU_ID_SNAPSHOTRESOLUTIONCHNAGE) {
    camera_snapshot_resolution_change_tbl ();
  } else if (current_menu_id == MENU_ID_FPSCHANGE) {
    camera_fps_change_tbl();
  } else if (current_menu_id == MENU_ID_LUMACHANGE) {
    camera_la_table();
  } else if (current_menu_id == MENU_ID_AECROICHANGE) {
    camera_aecroi_change_tbl();
  } else if (current_menu_id == MENU_ID_BL_N_SNOW_SET) {
    camera_bl_n_snow_detection_tbl();
  } else if (current_menu_id == MENU_ID_SET_BESTSHOT_MODE) {
    camera_set_bestshot_tbl();
  } else if (current_menu_id == MENU_ID_VIDEO_DIS_CONFIG) {
    video_dis_config_print();
  } else if (current_menu_id == MENU_ID_VIDEO_ROT_CONFIG) {
    video_rot_config_print();
  } else if (current_menu_id == MENU_ID_SCECHANGE) {
    camera_sce_table();
  } else if (current_menu_id == MENU_ID_3D_DISPLAY_DISTANCE) {
    camera_3d_display_distance();
  } else if (current_menu_id == MENU_ID_3D_VIEW_ANGLE) {
    camera_3d_angle_of_view();
  } else if (current_menu_id == MENU_ID_3D_CONV_CONTROL) {
    camera_3d_conv_control();
  }
  return 0;
}

