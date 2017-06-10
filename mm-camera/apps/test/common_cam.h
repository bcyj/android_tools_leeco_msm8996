/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __COMMON_CAM_H__
#define __COMMON_CAM_H__

#include "camera.h"

typedef enum {
  TAKE_YUV_PICTURE        = 1,
  TAKE_JPEG_PICTURE       = 2,
  STOP_CAMERA             = 3,
  ZOOM_GOTO_SUBMENU       = 4,
  CONTRAST_GOTO_SUBMENU   = 5,
  SATURATION_GOTO_SUBMENU = 6,
  SPECIAL_EFFECT          = 7,
  BRIGHTNESS_GOTO_SUBMENU = 8,
  EV_GOTO_SUBMENU         = 9,
  ANTI_BANDING            = 10,
  SET_WHITE_BALANCE       = 11,
  AEC_MODE                = 12,
  ISO_GOTO_SUBMENU        = 13,
  TAKE_RAW_PICTURE        = 14,
  SHARPNESS_GOTO_SUBMENU  = 15,
  SET_HJR                 = 16,
  SET_AUTO_FOCUS          = 17,
  SET_LENS_SHADING        = 18,
  SET_LED_MODE            = 19,
  GET_SHARPNESS_AF        = 20,
  PREVIEW_VIDEO_RESOLUTION = 21,
  SNAPSHOT_RESOLUTION     = 22,
  MOTION_ISO              = 23,
  TOGGLE_HUE              = 24,
  CANCEL_AUTO_FOCUS       = 25,
  GET_AF_STEP             = 26,
  SET_AF_STEP             = 27,
  ENABLE_AFD              = 28,
  SET_FPS_MODE            = 29,
  SET_FPS                 = 30,
  START_RECORDING         = 31,
  STOP_RECORDING          = 32,
  LUMA_ENABLE_DISABLE     = 33,
  ENABLE_CAF              = 34,
  AEC_ROI_GOTO_SUBMENU    = 35,
  SET_BL_N_SNOW_DETECTION = 36,
  TOGGLE_JPEG_ROTATION    = 37,
  SET_BESTSHOT_MODE       = 38,
  SET_AF_ROI              = 39,
  VIDEO_DIS_CONFIG        = 40,
  VIDEO_ROT_CONFIG        = 41,
  START_LIVESNAPSHOT      = 42,
  SET_FOCUS_RECTANGLE     = 43,
  SCE_ADJUST              = 44,
  TOGGLE_STROBE_FLASH_MODE = 45,
  TOGGLE_FACE_DETECTION   = 46,
  QUERY_FLASH4SNAP        = 47,
  GET_FOCUS_DISTANCES     = 48,
  START_ZSL               = 49,
  STOP_ZSL                = 50,
  CAPTURE_ZSL             = 51,
  GET_FOCAL_LENGTH        = 52,
  GET_HORIZONTAL_VIEW_ANGLE = 53,
  GET_VERTICAL_VIEW_ANGLE   = 54,
  TOGGLE_CAMERA_MODE      = 55,
  RESET_LENS_TO_INFINITY  = 56,
  GET_SNAPSHOT_DATA       = 57,
  STEREO_DISPLAY_DISTANCE = 58,
  STEREO_VIEW_ANGLE       = 59,
  UPDATE_PREVIEW_FORMAT   = 60,
  STEREO_CONV_CONTROL     = 61,
} Camera_main_menu_t;

typedef enum {
  ACTION_NO_ACTION,
  ACTION_TAKE_YUV_PICTURE,
  ACTION_TAKE_JPEG_PICTURE,
  ACTION_STOP_CAMERA,
  ACTION_PRINT_MAXZOOM,
  ACTION_PRINT_ZOOMRATIOS,
  ACTION_ZOOM_INCREASE,
  ACTION_ZOOM_DECREASE,
  ACTION_ZOOM_STEP_INCREASE,
  ACTION_ZOOM_STEP_DECREASE,
  ACTION_CONTRAST_INCREASE,
  ACTION_CONTRAST_DECREASE,
  ACTION_SATURATION_INCREASE,
  ACTION_SATURATION_DECREASE,
  ACTION_SPECIAL_EFFECT,
  ACTION_BRIGHTNESS_INCREASE,
  ACTION_BRIGHTNESS_DECREASE,
  ACTION_EV_INCREASE,
  ACTION_EV_DECREASE,
  ACTION_ANTI_BANDING,
  ACTION_SET_WHITE_BALANCE,
  ACTION_AEC_MODE,
  ACTION_ISO_INCREASE,
  ACTION_ISO_DECREASE,
  ACTION_TAKE_RAW_PICTURE,
  ACTION_SHARPNESS_INCREASE,
  ACTION_SHARPNESS_DECREASE,
  ACTION_SET_HJR,
  ACTION_SET_AUTO_FOCUS,
  ACTION_SET_LENS_SHADING,
  ACTION_SET_LED_MODE,
  ACTION_GET_SHARPNESS_AF,
  ACTION_PREVIEW_VIDEO_RESOLUTION,
  ACTION_SNAPSHOT_RESOLUTION,
  ACTION_MOTION_ISO,
  ACTION_TOGGLE_HUE,
  ACTION_CANCEL_AUTO_FOCUS,
  ACTION_GET_AF_STEP,
  ACTION_SET_AF_STEP,
  ACTION_ENABLE_AFD,
  ACTION_SET_FPS_MODE,
  ACTION_SET_FPS,
  ACTION_START_RECORDING,
  ACTION_STOP_RECORDING,
  ACTION_ENABLE_LA,
  ACTION_DISABLE_LA,
  ACTION_ROI_SET_INDEX,
  ACTION_ROI_SET_LOCATION,
  ACTION_ROI_DISABLE,
  ACTION_ENABLE_CAF,
  ACTION_BL_DETECTION_ON,
  ACTION_BL_DETECTION_OFF,
  ACTION_SNOW_DETECTION_ON,
  ACTION_SNOW_DETECTION_OFF,
  ACTION_TOGGLE_JPEG_ROTATION,
  ACTION_START_LIVESHOT,
  ACTION_SET_BESTSHOT_MODE,
  ACTION_TOGGLE_FACE_DETECTION,
  ACTION_SET_AF_ROI,
  ACTION_VPE_ROT_0,
  ACTION_VPE_ROT_90,
  ACTION_VPE_ROT_180,
  ACTION_VPE_ROT_270,
  ACTION_VPE_NO_DIS,
  ACTION_VPE_EN_DIS,
  ACTION_SET_AF_FOCUSRECT,
  ACTION_PALER_SKIN,
  ACTION_TANNER_SKIN,
  ACTION_TOGGLE_STROBE_FLASH_MODE,
  ACTION_QUERY_FLASH_4_SNAP,
  ACTION_GET_FOCUS_DISTANCES,
  ACTION_START_ZSL,
  ACTION_STOP_ZSL,
  ACTION_CAPTURE_ZSL,
  ACTION_GET_FOCAL_LENGTH,
  ACTION_GET_HORIZONTAL_VIEW_ANGLE,
  ACTION_GET_VERTICAL_VIEW_ANGLE,
  ACTION_TOGGLE_CAMERA_MODE,
  ACTION_RESET_LENS_TO_INFINITY,
  ACTION_GET_SNAPSHOT_DATA,
  ACTION_3D_DIST_0,
  ACTION_3D_DIST_10,
  ACTION_3D_DIST_20,
  ACTION_3D_DIST_30,
  ACTION_3D_ANGLE_0,
  ACTION_3D_ANGLE_30,
  ACTION_3D_ANGLE_60,
  ACTION_3D_ANGLE_90,
  ACTION_UPDATE_PREVIEW_FORMAT,
  ACTION_3D_AUTO_CONV,
  ACTION_3D_MANUAL_CONV,
} camera_action_t;

#define INVALID_KEY_PRESS 0
#define BASE_OFFSET  ('Z' - 'A' + 1)
#define BASE_OFFSET_NUM  ('Z' - 'A' + 2)
#define PAD_TO_WORD(a)  (((a)+3)&~3)

typedef enum {
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
  MP12                   = 16,
  RESOLUTION_MAX         = MP12,
} Camera_Resolution;

typedef enum {
  MENU_ID_MAIN,
  MENU_ID_ZOOMCHANGE,
  MENU_ID_CONTRASTCHANGE,
  MENU_ID_SATURATIONCHANGE,
  MENU_ID_BRIGHTNESSCHANGE,
  MENU_ID_EVCHANGE,
  MENU_ID_ISOCHANGE,
  MENU_ID_SHARPNESSCHANGE,
  MENU_ID_PREVIEWVIDEORESOLUTIONCHANGE,
  MENU_ID_SNAPSHOTRESOLUTIONCHNAGE,
  MENU_ID_FPSCHANGE,
  MENU_ID_LUMACHANGE,
  MENU_ID_AECROICHANGE,
  MENU_ID_BL_N_SNOW_SET,
  MENU_ID_SET_BESTSHOT_MODE,
  MENU_ID_VIDEO_DIS_CONFIG,
  MENU_ID_VIDEO_ROT_CONFIG,
  MENU_ID_SCECHANGE,
  MENU_ID_3D_DISPLAY_DISTANCE,
  MENU_ID_3D_VIEW_ANGLE,
  MENU_ID_3D_CONV_CONTROL,
  MENU_ID_INVALID,
} menu_id_change_t;

typedef enum {
  ENABLE_LA   = 1,
  DISABLE_LA  = 2,
}Camera_Luma;

typedef enum {
 PALER_SKIN = 1,
 TANNER_SKIN = 2,
}Camera_Sce;

typedef enum {
  ROT_0   = 1,
  ROT_90  = 2,
  ROT_180 = 3,
  ROT_270 = 4
}Rot_Config;

typedef enum {
  NO_DIS = 1,
  EN_DIS = 2
}Dis_Config;

typedef enum {
  INCREASE_ZOOM      = 1,
  DECREASE_ZOOM      = 2,
  INCREASE_STEP_ZOOM = 3,
  DECREASE_STEP_ZOOM = 4,
  PRINT_MAXZOOM      = 5,
  PRINT_ZOOMRATIOS   = 6,
} Camera_Zoom;

typedef enum {
  INC_CONTRAST = 1,
  DEC_CONTRAST = 2,
} Camera_Contrast_changes;

typedef enum {
  INC_SATURATION = 1,
  DEC_SATURATION = 2,
} Camera_Saturation_changes;

typedef enum {
  INC_BRIGHTNESS = 1,
  DEC_BRIGHTNESS = 2,
} Camera_Brightness_changes;

typedef enum {
  INCREASE_EV = 1,
  DECREASE_EV = 2,
} Camera_EV_changes;

typedef enum {
  INC_ISO = 1,
  DEC_ISO = 2,
} Camera_ISO_changes;

typedef enum {
  INC_SHARPNESS = 1,
  DEC_SHARPNESS = 2,
} Camera_Sharpness_changes;

typedef enum {
  SET_ROI_INDEX           = 1,
  SET_ROI_LOCATION_INDEX  = 2,
  SET_ROI_DISABLE_INDEX   = 3,
} Camera_AECROI_changes;

typedef enum {
  BL_DETECTION_ON       = 1,
  BL_DETECTION_OFF      = 2,
  SNOW_DETECTION_ON     = 3,
  SNOW_DETECTION_OFF    = 4,
} BL_n_Snow_changes;

typedef enum {
  DIST_0   = 1,
  DIST_10  = 2,
  DIST_20  = 3,
  DIST_30  = 4
}Dist_Config;

typedef enum {
  ANGLE_0   = 1,
  ANGLE_30  = 2,
  ANGLE_60  = 3,
  ANGLE_90  = 4
}Angle_Config;

typedef enum {
  ST_AUTO_CONV    = 1,
  ST_MANUAL_CONV  = 2,
  ST_EXIT_CONV    = 3,
}Stereo_Conv_Ctrl_Config;

typedef struct {
  void    *frameThread;
  int8_t (*setDimension)(int , void *);
  int8_t (*setDefaultParams)(int );
  int8_t (*registerPreviewBuf)(int , void *, uint32_t, struct msm_frame *, int8_t );
  int8_t (*unregisterPreviewBuf)(int , void *, uint32_t, int , unsigned char *);
  int8_t (*registerVideoBuf)(int , void *, uint32_t, struct msm_frame *, int8_t, int8_t);
  int8_t (*unregisterVideoBuf)(int , void *, uint32_t, int , unsigned char *, int8_t);
  int8_t (*startPreview)(int );
  int8_t (*stopPreview)(int );
  int8_t (*startVideo)(int );
  int8_t (*stopVideo)(int );
  int8_t (*startRecording)(int );
  int8_t (*stopRecording)(int );
  int8_t (*startSnapshot)(int );
  int8_t (*startRawSnapshot)(int );
  int8_t (*startLiveShot) (int, void* , int, uint32_t, uint32_t );
  int8_t (*registerSnapshotBuf)(int, int, int, int, int, int,
    unsigned char *, int);
  int8_t (*unregisterSnapshotBuf)(int, int, int, int, int, int,
    unsigned char *, int);
  int8_t (*registerRawSnapshotBuf)(int , void *, int , unsigned char *);
  int8_t (*unregisterRawSnapshotBuf)(int , void *, int , unsigned char *);
  int8_t (*getPicture)(int fd, struct crop_info *cropInfo );
  int8_t (*stopSnapshot)(int );
  int8_t (*jpegEncode)(const char *path, void *, int, int , unsigned char *,
    unsigned char *, void *, camera_encoding_rotate_t rotate);
  int8_t (*setJpegRotation)(int);
  int8_t (*setStrobeFlashMode)(int, strobe_flash_mode_t);
  int8_t (*setZoom)(int , void *);
  int8_t (*getMaxZoom)(int fd, void *pZm);
  int8_t (*getZoomRatio)(int fd, int maxZoom, void *pZm);
  int8_t (*setSpecialEffect)(int, int effect);
  int8_t (*setBrightness)(int, int);
  int8_t (*setContrast)(int, int);
  int8_t (*setSaturation)(int, int);
  int8_t (*setEV)(int , int );
  int8_t (*setAntiBanding)(int , int32_t antibanding);
  int8_t (*setWhiteBalance)(int , int32_t );
  int8_t (*setAecMode)(int , camera_auto_exposure_mode_type );
  int8_t (*setAecROI)(int , cam_set_aec_roi_t);
  int8_t (*setIso)(int , camera_iso_mode_type );
  int8_t (*setSharpness)(int , int );
  int8_t (*setAutoFocus)(int , isp3a_af_mode_t, cam_af_ctrl_t *);

  /* The commented declaration is incompatible with V4L2 method*/
  /*int8_t (*sethjr) (int fd, int8_t hjr_status);*/
  int8_t (*sethjr) (int fd, int hjr_status);
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
  int8_t (*setFps) (int fd, uint32_t fps);
  int8_t (*setAFFocusRect) (int fd, cam_af_focusrect_t af_focus_rect);
  int8_t (*enableLA)(int fd,void *value);
  int8_t (*disableLA)(int fd, void  *value);
  int8_t (*setCAF) (int fd, caf_ctrl_t caf);
  int8_t (*setBacklightDetection) (int fd, uint8_t bl_detection);
  int8_t (*setSnowSceneDetection) (int fd, uint8_t snow_detection);
  int8_t (*setBestShotMode) (int fd, uint8_t bestshotmode);
  int8_t (*setFaceDetection) (int fd, uint8_t face_detection);
  int8_t (*setAF_ROI) (int fd, roi_info_t roi_ctrl);
  int8_t (*video_rot_config)(int fd,void *value);
  int8_t (*video_dis_config)(int fd,void *value);
  int8_t (*setSCEfactor)(int, int);
  int8_t (*queryFlash4Snap)(int, int*);
  int8_t (*getFocusDistances) (int fd, focus_distances_info_t *focus_distances_info);
  int8_t (*getFocalLength) (int fd, float *value);
  int8_t (*getHorizontalViewAngle) (int fd, float *value);
  int8_t (*getVerticalViewAngle) (int fd, float *value);
  int8_t (*resetLensToInfinity) (int fd);
  int8_t (*getSnapshotdata) (int fd, snapshotData_info_t snapshotData);
} interface_ctrl_t;

int8_t native_interface_init(interface_ctrl_t *intrfcCtrl, int *camfd);

int print_maxzoom();
int print_zoomratios();
int zoom_increase(int);
int zoom_decrease(int);
int set_hjr (void);
int LensShading (void);
int decrease_contrast (void);
int increase_contrast (void);
int decrease_saturation (void);
int increase_saturation (void);
int decrease_brightness (void);
int increase_brightness (void);
int decrease_EV (void);
int increase_EV (void);
int decrease_ISO (void);
int increase_ISO (void);
int decrease_sharpness (void);
int increase_sharpness (void);
int SpecialEffect (void);
int AEC_mode_change (void);
int LED_mode_change (void);
int set_sharpness_AF (void);
int set_auto_focus (void);
int set_antibanding (void);
int set_whitebalance (void);
int take_picture (char );
int take_raw_picture (void);
int print_current_menu ();
int set_MotionIso (void);
int start_preview (void);
int stop_preview (void);
int start_video (void);
int stop_video (void);
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
int enable_la();
int disbale_la();
int roiIndex_set(void);
int aec_roi_disable( void );
int roiIndex_set_location(void);
int set_bl_detection(uint8_t);
int set_snow_detection(uint8_t);
int set_CAF(void);
int set_bestshotmode(uint8_t);
int setAF_ROI(void);
int video_rotation_config(uint16_t rotation);
int video_dis_config(uint8_t dis_enable);
int make_skin_paler();
int make_skin_tanner();
int toggle_facedetection (void);
int get_focus_distances(void);
int get_focal_length();
int get_horizontal_view_angle();
int get_vertical_view_angle();
int toggle_camera_mode(void);
int reset_lens_infinity();
int get_snapshot_data();
int set_3d_display_distance(uint32_t distance);
int set_3d_view_angle(uint32_t angle);
void set_3d_conv_control(int auto_conv);
int update_preview_format();
#endif /* __COMMON_CAM_H__ */
