/* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

#ifndef __COMMON_CAM_H__
#define __COMMON_CAM_H__

#include "camera.h"

typedef enum
{
  STOP_CAMERA = 1,
  PREVIEW_VIDEO_RESOLUTION = 2,
  SET_WHITE_BALANCE = 3,
  SET_EXP_METERING = 4,
  GET_CTRL_VALUE = 5,
  TOGGLE_AFR = 6,
  SET_ISO = 7,
  BRIGHTNESS_GOTO_SUBMENU = 8,
  CONTRAST_GOTO_SUBMENU = 9,
  EV_GOTO_SUBMENU = 10,
  SATURATION_GOTO_SUBMENU = 11,
  SET_ZOOM = 12,
  SET_SHARPNESS = 13,
  TAKE_YUV_SNAPSHOT = 14,
  ANTI_BANDING = 15,
  SPECIAL_EFFECT = 16,
  SET_HJR = 17,
  LUMA_ENABLE_DISABLE = 18,
  AUTO_FOCUS = 19,
  BEST_SHOT = 20,
  BL_SNOW = 21,
  TAKE_RAW_SNAPSHOT = 22,
  TOGGLE_STROBE_FLASH_MODE = 23,
  SET_LED_MODE = 24,
  TAKE_422_SNAPSHOT =25,
  TAKE_INLINE_SNAPSHOT =26,
  SET_HDR_GOTO_SUBMENU = 27,
} Camera_main_menu_t;

typedef enum
{
  ACTION_NO_ACTION,
  ACTION_STOP_CAMERA,
  ACTION_PREVIEW_VIDEO_RESOLUTION,
  ACTION_SET_WHITE_BALANCE,
  ACTION_SET_EXP_METERING,
  ACTION_GET_CTRL_VALUE,
  ACTION_TOGGLE_AFR,
  ACTION_SET_ISO,
  ACTION_BRIGHTNESS_INCREASE,
  ACTION_BRIGHTNESS_DECREASE,
  ACTION_CONTRAST_INCREASE,
  ACTION_CONTRAST_DECREASE,
  ACTION_EV_INCREASE,
  ACTION_EV_DECREASE,
  ACTION_SATURATION_INCREASE,
  ACTION_SATURATION_DECREASE,
  ACTION_SET_ZOOM,
  ACTION_SHARPNESS_INCREASE,
  ACTION_SHARPNESS_DECREASE,
  ACTION_TAKE_YUV_SNAPSHOT,
  ACTION_ANTI_BANDING,
  ACTION_SPECIAL_EFFECT,
  ACTION_SET_HJR,
  ACTION_ENABLE_LA,
  ACTION_DISABLE_LA,
  ACTION_AUTO_FOCUS,
  ACTION_SET_BESTSHOT_MODE,
  ACTION_BL_DETECTION_ON,
  ACTION_BL_DETECTION_OFF,
  ACTION_SNOW_DETECTION_ON,
  ACTION_SNOW_DETECTION_OFF,
  ACTION_TAKE_RAW_SNAPSHOT,
  ACTION_TOGGLE_STROBE_FLASH_MODE,
  ACTION_SET_LED_MODE,
  ACTION_TAKE_422_SNAPSHOT,
  ACTION_TAKE_INLINE_SNAPSHOT,
  ACTION_HDR_MODE,
} camera_action_t;

#define INVALID_KEY_PRESS 0
#define BASE_OFFSET  ('Z' - 'A' + 1)
#define BASE_OFFSET_NUM  ('Z' - 'A' + 2)
#define PAD_TO_WORD(a)  (((a)+3)&~3)

#define SQCIF_WIDTH     128
#define SQCIF_HEIGHT     96
#define QCIF_WIDTH      176
#define QCIF_HEIGHT     144
#define QVGA_WIDTH      320
#define QVGA_HEIGHT     240
#define HD_THUMBNAIL_WIDTH      256
#define HD_THUMBNAIL_HEIGHT     144
#define CIF_WIDTH       352
#define CIF_HEIGHT      288
#define VGA_WIDTH       640
#define VGA_HEIGHT      480
#define WVGA_WIDTH      800
#define WVGA_HEIGHT     480

#define MP1_WIDTH      1280
#define MP1_HEIGHT      960
#define MP2_WIDTH      1600
#define MP2_HEIGHT     1200
#define MP3_WIDTH      2048
#define MP3_HEIGHT     1536
#define MP5_WIDTH      2592
#define MP5_HEIGHT     1944

#define SVGA_WIDTH      800
#define SVGA_HEIGHT     600
#define XGA_WIDTH      1024
#define XGA_HEIGHT      768
#define HD720_WIDTH    1280
#define HD720_HEIGHT    720
#define WXGA_WIDTH     1280
#define WXGA_HEIGHT     768
#define HD1080_WIDTH   1920
#define HD1080_HEIGHT  1080

typedef enum
{
  RESOLUTION_MIN         = 1,
  SQCIF                  = RESOLUTION_MIN,
  QCIF                   = 2,
  QVGA                   = 3,
  CIF                    = 4,
  VGA                    = 5,
  WVGA                   = 6,
  SVGA                   = 7,
  XGA                    = 8,
  HD720                  = 9,
  RESOLUTION_PREVIEW_VIDEO_MAX = HD720,
  WXGA                   = 10,
  MP1                    = 11,
  MP2                    = 12,
  HD1080                 = 13,
  MP3                    = 14,
  MP5                    = 15,
  RESOLUTION_MAX         = MP5,
} Camera_Resolution;

typedef enum {
  WHITE_BALANCE_AUTO         = 1,
  WHITE_BALANCE_OFF          = 2,
  WHITE_BALANCE_DAYLIGHT     = 3,
  WHITE_BALANCE_INCANDESCENT = 4,
  WHITE_BALANCE_FLUORESCENT  = 5,
} White_Balance_modes;

typedef enum {
    WHITE_BALANCE_STATE = 1,
    WHITE_BALANCE_TEMPERATURE = 2,
    BRIGHTNESS_CTRL = 3,
    EV = 4,
    CONTRAST_CTRL = 5,
    SATURATION_CTRL = 6,
    SHARPNESS_CTRL = 7,
} Get_Ctrl_modes;

typedef enum {
	EXP_METERING_FRAME_AVERAGE   = 1,
	EXP_METERING_CENTER_WEIGHTED = 2,
  EXP_METERING_SPOT_METERING   = 3,
} Exp_Metering_modes;

typedef enum {
  ISO_AUTO = 1,
  ISO_DEBLUR = 2,
  ISO_100 = 3,
  ISO_200 = 4,
  ISO_400 = 5,
  ISO_800 = 6,
  ISO_1600 = 7,
} ISO_modes;

typedef enum
{
  MENU_ID_MAIN,
  MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE,
  MENU_ID_WHITEBALANCECHANGE,
  MENU_ID_EXPMETERINGCHANGE,
  MENU_ID_GET_CTRL_VALUE,
  MENU_ID_TOGGLEAFR,
  MENU_ID_ISOCHANGE,
  MENU_ID_BRIGHTNESSCHANGE,
  MENU_ID_CONTRASTCHANGE,
  MENU_ID_EVCHANGE,
  MENU_ID_SATURATIONCHANGE,
  MENU_ID_ZOOMCHANGE,
  MENU_ID_SHARPNESSCHANGE,
  MENU_ID_LUMACHANGE,
  MENU_ID_BESTSHOT,
  MENU_ID_BL_SNOW,
  MENU_ID_HDR_CHANGE,
  MENU_ID_INVALID,
} menu_id_change_t;

typedef enum
{
  INCREASE_ZOOM      = 1,
  DECREASE_ZOOM      = 2,
  INCREASE_STEP_ZOOM = 3,
  DECREASE_STEP_ZOOM = 4,
} Camera_Zoom;

typedef enum
{
  INC_CONTRAST = 1,
  DEC_CONTRAST = 2,
} Camera_Contrast_changes;

typedef enum
{
  INC_BRIGHTNESS = 1,
  DEC_BRIGHTNESS = 2,
} Camera_Brightness_changes;

typedef enum
{
  INCREASE_EV = 1,
  DECREASE_EV = 2,
} Camera_EV_changes;

typedef enum {
  INC_SATURATION = 1,
  DEC_SATURATION = 2,
} Camera_Saturation_changes;

typedef enum
{
  INC_ISO = 1,
  DEC_ISO = 2,
} Camera_ISO_changes;

typedef enum
{
  INC_SHARPNESS = 1,
  DEC_SHARPNESS = 2,
} Camera_Sharpness_changes;

typedef enum {
  ENABLE_LA   = 1,
  DISABLE_LA  = 2,
}Camera_Luma;

typedef enum {
  ZOOM_IN = 1,
  ZOOM_OUT = 2,
  ZOOM_IN_SMOOTH = 3,
  ZOOM_OUT_SMOOTH = 4,
} Zoom_direction;


typedef enum {
  BL_DETECTION_ON       = 1,
  BL_DETECTION_OFF      = 2,
  SNOW_DETECTION_ON     = 3,
  SNOW_DETECTION_OFF    = 4,
} BL_n_Snow_changes;

typedef enum {
  HDR_MODE_ON       = 1,
  HDR_MODE_OFF      = 2,
  EXPOSURE_METERING_ON     = 3,
} HDR_mode_changes;

typedef struct {
  void    *frameThread;
  int8_t (*setDimension)(int , void *);
  int8_t (*setDefaultParams)(int );
  int8_t (*registerPreviewBuf)(int , void *, uint32_t, struct msm_frame *, int8_t );
  int8_t (*unregisterPreviewBuf)(int , void *, uint32_t, int , unsigned char *);
  int8_t (*registerVideoBuf)(int , void *, uint32_t, struct msm_frame *, int8_t );
  int8_t (*unregisterVideoBuf)(int , void *, uint32_t, int , unsigned char *);
  int8_t (*startPreview)(int );
  int8_t (*stopPreview)(int );
  int8_t (*startVideo)(int );
  int8_t (*stopVideo)(int );
  int8_t (*startRecording)(int );
  int8_t (*stopRecording)(int );
  int8_t (*startSnapshot)(int );
  int8_t (*startRawSnapshot)(int );

  int8_t (*registerSnapshotBuf)(int , void *, int , int ,
    unsigned char *, unsigned char *);

  int8_t (*registerRawSnapshotBuf)(int , void *, int , unsigned char *);

  int8_t (*unregisterSnapshotBuf)(int , void *, int , int ,
    unsigned char *, unsigned char *);

  int8_t (*unregisterRawSnapshotBuf)(int , void *, int , unsigned char *);
  int8_t (*getPicture)(int fd, struct crop_info *cropInfo );
  int8_t (*stopSnapshot)(int );
  int8_t (*jpegEncode)(const char *path, void *, int, int , unsigned char *,
    unsigned char *, void *, camera_encoding_rotate_t rotate);
  int8_t (*setZoom)(int , void *);
  int8_t (*getMaxZoom)(int fd, void *pZm);
  int8_t (*setSpecialEffect)(int, int effect);
  int8_t (*setBrightness)(int, int);
  int8_t (*setContrast)(int, int);
  int8_t (*setSaturation)(int, int);
  int8_t (*setEV)(int , int );
  int8_t (*setAntiBanding)(int , int32_t antibanding);
  int8_t (*setWhiteBalance)(int , int32_t );
  int8_t (*setAecMode)(int , camera_auto_exposure_mode_type );  
  int8_t (*setIso)(int , camera_iso_mode_type );
  int8_t (*setSharpness)(int , int );
  int8_t (*setAutoFocus)(int , isp3a_af_mode_t, cam_af_ctrl_t *);
  int8_t (*sethjr) (int fd, int8_t hjr_status);
  int8_t (*setLensShading) (int fd, int8_t rolloff_status);
  int8_t (*setLedMode) (int fd, led_mode_t led_mode);
  int8_t (*getSharpness_AF) (int fd, int32_t *sharpness);
  int8_t (*setMotionIso) (int fd, motion_iso_t motion_iso);
  int8_t (*setHue) (int fd, int32_t hue);
  int8_t (*cancelAF) (int fd);
  int8_t (*getAfStep) (int fd, int32_t *afStep);
  int8_t (*setAfStep) (int fd, int32_t afStep);
  int8_t (*enableAFD) (int fd);
  int8_t (*prepareSnapshot) (int fd);
  int8_t (*setFpsMode) (int fd, fps_mode_t fps_mode);
  int8_t (*setFps) (int fd, uint16_t fps);
  int8_t (*setAFFocusRect) (int fd, cam_af_focusrect_t af_focus_rect);
} interface_ctrl_t;

int8_t native_interface_init(interface_ctrl_t *intrfcCtrl, int *camfd);
int8_t v4l2_interface_init(interface_ctrl_t *intrfcCtrl, int *videofd);

int set_zoom (int zoom_action_param);
int set_hjr (void);
int LensShading (void);
int decrease_contrast (void);
int increase_contrast (void);
int decrease_saturation (void);
int increase_saturation (void);
int decrease_brightness (void);
int increase_brightness (void);
int enable_la(void);
int disable_la(void);
int decrease_EV (void);
int increase_EV (void);
int set_iso (int iso_action_param);
int decrease_sharpness (void);
int increase_sharpness (void);
int SpecialEffect (void);
int set_exp_metering (int exp_metering_action_param);
int LED_mode_change (void);
int set_sharpness_AF (void);
int set_auto_focus (void);
int set_antibanding (void);
int set_whitebalance (int wb_action_param);
int take_picture (char );
int take_raw_picture (void);
int print_current_menu ();
int set_MotionIso (void);
int start_preview (void);
int stop_preview (void);
static int start_video (void);
static int stop_video (void);
int start_recording (void);
int stop_recording (void);
int snapshot_resolution (int);
int preview_video_resolution (int);
int system_init(void);
int system_destroy(void);
int toggle_hue(void);
int cancel_af(void);
int get_af_step();
int set_af_step();
int enable_afd();
int prepare_snapshot();
int set_fps_mode(void);
int get_ctrl_value (int ctrl_value_mode_param);
int toggle_afr ();
int take_yuv_snapshot();
int set_bestshot_mode(int bs_param);
#endif /* __COMMON_CAM_H__ */
