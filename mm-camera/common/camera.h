/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/select.h>
#include <media/msm_camera.h>
#if defined(_V4L2_BASED_CAM_)
#include <media/msm_isp.h>
#endif
#include <poll.h>
#include <linux/videodev2.h>
#include "jpege.h"
//#include "exif.h"
#include "camera_defs_i.h"
#include "QCamera_Intf.h"

#ifdef _ANDROID_
  #define MSM_CAMERA_SERVER "/dev/video100"
  #define MSM_CAMERA_CONTROL "/dev/msm_camera/control%d"
  #define MSM_CAMERA_CONFIG  "/dev/msm_camera/config%d"
  #define MSM_CAMERA_FRAME   "/dev/msm_camera/frame%d"
  #define MSM_CAMERA_PIC     "/dev/msm_camera/pic%d"
#else
  #define MSM_CAMERA_SERVER "/dev/video100"
  #define MSM_CAMERA_CONTROL "/dev/control%d"
  #define MSM_CAMERA_CONFIG  "/dev/config%d"
  #define MSM_CAMERA_FRAME   "/dev/frame%d"
  #define MSM_CAMERA_PIC     "/dev/pic%d"
#endif

#define NATIVE_CAMERA_INTERFACE   1

#define DOUBLE_2_FLOAT(X,Y,OUT,IN) ({ \
  for(i=0; i<X; i++) \
    for(j=0; j<Y; j++) \
      OUT[i][j] = IN[i][j]; \
})

#define FIND_STEREO_SIZE_FACTOR(PACK, FULL_SCALE, W_SC, H_SC) ({ \
  switch (PACK) { \
    case TOP_DOWN_FULL: \
      W_SC = 1; \
      H_SC = 2; \
      break; \
    case TOP_DOWN_HALF: \
      if(FULL_SCALE) { \
        W_SC = 1; \
        H_SC = 1; \
      } else { \
        W_SC = 1; \
        H_SC = 2; \
      } \
      break; \
    case SIDE_BY_SIDE_FULL: \
      W_SC = 2; \
      H_SC = 1; \
      break; \
    case SIDE_BY_SIDE_HALF: \
      if(FULL_SCALE) { \
        W_SC = 1; \
        H_SC = 1; \
      } else { \
        W_SC = 2; \
        H_SC = 1; \
      } \
      break; \
    default: \
      CDBG_HIGH("FIND_STEREO_SIZE_FACTOR: Invalid pack\n"); \
  } \
})

#undef __FD_SET
#define __FD_SET(fd, fdsetp) \
                (((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] |= (1<<((fd) & 31)))
#undef  __FD_ISSET
#define __FD_ISSET(fd, fdsetp) \
                ((((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] & (1<<((fd) & 31))) != 0)

#undef  __FD_ZERO
#define __FD_ZERO(fdsetp) \
                (memset (fdsetp, 0, sizeof (*(fd_set *)(fdsetp))))


#define ANDROID_FB0 "/dev/graphics/fb0"
#define LE_FB0 "/dev/fb0"

#define MAX_DEV_NAME_LEN 50
#define MAX_ZSL_SNAPSHOT_SUPPORTED (MAX_SNAPSHOT_BUFFERS-2)
#define MAX_OUTPUT_SUPPORTED 5 /* Max Number of scalers in VFE */
#define AF_COLLECTION_POINTS       50

/* Error codes */
#define CAMERA_SUCCESS                   0
#define CAMERA_ERR_GENERAL              -1
#define CAMERA_ERR_NO_MEMORY            -2
#define CAMERA_ERR_NOT_SUPPORTED        -3
#define CAMERA_ERR_INVALID_INPUT        -4
#define CAMERA_ERR_INVALID_OPERATION    -5
#define CAMERA_ERR_NO_DATA              -6
#define CAMERA_ERR_TIMEOUT              -7
#define CAMERA_ERR_NOT_FOUND            -8
#define CAMERA_GET_FRAME_FAILED         -9
#define CAMERA_ERR_OUT_OF_BOUNDS       -10

#define CAM_VALIDATE(rc, ptr) rc = ((ptr == NULL) ? -EINVAL : 0)

typedef int camera_status_t;

typedef enum
{
  AFD_OFF = 0,
  AFD_REGULAR_EXPOSURE_TABLE, /* 1 */
  AFD_60HZ_EXPOSURE_TABLE,  /* 2 */
  AFD_50HZ_EXPOSURE_TABLE,  /* 3 */
  AFD_50HZ_AUTO_EXPOSURE_TABLE, /* 4 */
  AFD_60HZ_AUTO_EXPOSURE_TABLE, /* 5 */
} afd_status_t;

typedef enum {
  CAMERA_PREVIEW_MODE_SNAPSHOT,
  CAMERA_PREVIEW_MODE_MOVIE,
  CAMERA_PREVIEW_MODE_MOVIE_120FPS,
  CAMERA_MAX_PREVIEW_MODE
} cam_preview_mode_t;

typedef struct {
  uint16_t dispWidth;
  uint16_t dispHeight;
} cam_ctrl_disp_t;

typedef enum msm_st_frame_packing cam_3d_frame_format_t;

typedef struct {
  cam_frame_type_t frame_type;
  cam_3d_frame_format_t format;
}camera_3d_frame_t;

#define CAM_ROTATION_0         0
#define CAM_ROTATION_90        90
#define CAM_ROTATION_180       180
#define CAM_ROTATION_270       270

typedef struct {
  int32_t x;
  int32_t y;
  uint32_t frame_id;
	uint16_t extra_pad_w;
	uint16_t extra_pad_h;
} cam_dis_info_t;

typedef struct {
  int pic_width;
  int pic_hight;
  int prev_width;
  int prev_hight;
  int rotation;
} cam_ctrl_set_dimension_data_t;

#define CAMERA_FPS_DENOMINATOR 1000

/*============================================================================
*                         DATA DECLARATIONS
============================================================================*/
/* to select no vignette correction, luma vignette correction */
/* or bayer vignette correction */
typedef enum {
  CAMERA_NO_VIGNETTE_CORRECTION,
  CAMERA_LUMA_VIGNETTE_CORRECTION,
  CAMERA_BAYER_VIGNETTE_CORRECTION
} camera_vc_mode_type;

typedef enum {
  CAMERA_BVCM_DISABLE,
  CAMERA_BVCM_RAW_CAPTURE,
  CAMERA_BVCM_OFFLINE_CAPTURE
} camera_bvcm_capture_type;

typedef struct {
  float min_fps;
  float max_fps;
} cam_sensor_fps_range_t;

/* Sensor position type, used for CAMERA_PARM_SENSOR_POSITION */
typedef enum {
  CAMERA_SP_NORMAL = 0,
  CAMERA_SP_REVERSE,
} camera_sp_type;

/* Exposure type, used for CAMERA_PARM_EXPOSURE */
typedef enum {
  CAMERA_EXPOSURE_MIN_MINUS_1,
  CAMERA_EXPOSURE_AUTO = 1,  /* This list must match aeecamera.h */
  CAMERA_EXPOSURE_DAY,
  CAMERA_EXPOSURE_NIGHT,
  CAMERA_EXPOSURE_LANDSCAPE,
  CAMERA_EXPOSURE_STRONG_LIGHT,
  CAMERA_EXPOSURE_SPOTLIGHT,
  CAMERA_EXPOSURE_PORTRAIT,
  CAMERA_EXPOSURE_MOVING,
  CAMERA_EXPOSURE_MAX_PLUS_1
} camera_exposure_type;

typedef enum {
  CAMERA_NIGHTSHOT_MODE_OFF,
  CAMERA_NIGHTSHOT_MODE_ON,
  CAMERA_MAX_NIGHTSHOT_MODE
} camera_nightshot_mode_type;

typedef enum {
  CAMERA_EXIT_CB_ABORT = -2,     /* AF is aborted */
  CAMERA_EXIT_CB_FAILED = -1,    /* AF is failed or rejected */
  CAMERA_EXIT_CB_DONE = 0,       /* AF is sucessful */
  CAMERA_CB_MAX,
} camera_af_done_type;

/*Keeping the legacy error code for older targets*/
typedef enum {
  CAMERA_INVALID_STATE = 1,
  CAMERA_INVALID_PARM,
  CAMERA_INVALID_FORMAT,
  CAMERA_NO_SENSOR,
  CAMERA_NO_MEMORY,
  CAMERA_NOT_SUPPORTED,
  CAMERA_FAILED,
  CAMERA_INVALID_STAND_ALONE_FORMAT,
  CAMERA_MALLOC_FAILED_STAND_ALONE,
  CAMERA_RET_CODE_MAX
} camera_ret_code_type;

typedef enum {
  CAMERA_RAW,
  CAMERA_JPEG,
  CAMERA_PNG,
  CAMERA_YCBCR_ENCODE,
  CAMERA_ENCODE_TYPE_MAX
} camera_encode_type;

#if !defined FEATURE_CAMERA_ENCODE_PROPERTIES && defined FEATURE_CAMERA_V7
typedef enum {
  CAMERA_SNAPSHOT,
  CAMERA_RAW_SNAPSHOT
} camera_snapshot_type;
#endif /* nFEATURE_CAMERA_ENCODE_PROPERTIES && FEATURE_CAMERA_V7 */

typedef enum {
  /* YCbCr, each pixel is two bytes. Two pixels form a unit.
   * MSB is Y, LSB is CB for the first pixel and CR for the second pixel. */
  CAMERA_YCBCR,
#ifdef FEATURE_CAMERA_V7
  CAMERA_YCBCR_4_2_0,
  CAMERA_YCBCR_4_2_2,
  CAMERA_H1V1,
  CAMERA_H2V1,
  CAMERA_H1V2,
  CAMERA_H2V2,
  CAMERA_BAYER_8BIT,
  CAMERA_BAYER_10BIT,
#endif /* FEATURE_CAMERA_V7 */
  /* RGB565, each pixel is two bytes.
   * MS 5-bit is red, the next 6-bit is green. LS 5-bit is blue. */
  CAMERA_RGB565,
  /* RGB666, each pixel is four bytes.
   * MS 14 bits are zeros, the next 6-bit is red, then 6-bit of green.
   * LS 5-bit is blue. */
  CAMERA_RGB666,
  /* RGB444, each pixel is 2 bytes. The MS 4 bits are zeros, the next
   * 4 bits are red, the next 4 bits are green. The LS 4 bits are blue. */
  CAMERA_RGB444,
  /* Bayer, each pixel is 1 bytes. 2x2 pixels form a unit.
   * First line: first byte is blue, second byte is green.
   * Second line: first byte is green, second byte is red. */
  CAMERA_BAYER_BGGR,
  /* Bayer, each pixel is 1 bytes. 2x2 pixels form a unit.
   * First line: first byte is green, second byte is blue.
   * Second line: first byte is red, second byte is green. */
  CAMERA_BAYER_GBRG,
  /* Bayer, each pixel is 1 bytes. 2x2 pixels form a unit.
   * First line: first byte is green, second byte is red.
   * Second line: first byte is blue, second byte is green. */
  CAMERA_BAYER_GRBG,
  /* Bayer, each pixel is 1 bytes. 2x2 pixels form a unit.
   * First line: first byte is red, second byte is green.
   * Second line: first byte is green, second byte is blue. */
  CAMERA_BAYER_RGGB,
  /* RGB888, each pixel is 3 bytes. R is 8 bits, G is 8 bits,
   * B is 8 bits*/
  CAMERA_RGB888
} camera_format_type;

typedef enum {
  CAMERA_ORIENTATION_LANDSCAPE,
  CAMERA_ORIENTATION_PORTRAIT
} camera_orientation_type;

typedef enum {
  CAMERA_DESCRIPTION_STRING,
  CAMERA_USER_COMMENT_STRING,
  CAMERA_GPS_AREA_INFORMATION_STRING
} camera_string_type;

typedef struct {
  int32_t  buffer[256];       /* buffer to hold data */
  int32_t  max_value;
} camera_preview_histogram_info;

typedef enum {
  CAM_STATS_TYPE_HIST,
  CAM_STATS_TYPE_MAX
}camstats_type;

typedef struct {
  uint32_t timestamp;  /* seconds since 1/6/1980          */
  double   latitude;   /* degrees, WGS ellipsoid */
  double   longitude;  /* degrees                */
  int16_t  altitude;   /* meters                          */
} camera_position_type;

typedef struct {
  /* Format of the frame */
  camera_format_type format;

  /* For pre-V7, Width and height of the picture.
   * For V7:
   *   Snapshot:     thumbnail dimension
   *   Raw Snapshot: not applicable
   *   Preview:      not applicable
   */
  uint16_t dx;
  uint16_t dy;
  /* For pre_V7: For BAYER format, RAW data before scaling.
   * For V7:
   *   Snapshot:     Main image dimension
   *   Raw snapshot: raw image dimension
   *   Preview:      preview image dimension
   */
  uint16_t captured_dx;
  uint16_t captured_dy;
  /* it indicates the degree of clockwise rotation that should be
   * applied to obtain the exact view of the captured image. */
  uint16_t rotation;

#ifdef FEATURE_CAMERA_V7
  /* Preview:      not applicable
   * Raw shapshot: not applicable
   * Snapshot:     thumbnail image buffer
   */
  uint8_t *thumbnail_image;
#endif /* FEATURE_CAMERA_V7 */

  /* For pre-V7:
   *   Image buffer ptr
   * For V7:
   *   Preview: preview image buffer ptr
   *   Raw snapshot: Raw image buffer ptr
   *   Shapshot:     Main image buffer ptr
   */
  uint8_t  *buffer;
} camera_frame_type;

typedef struct {
  uint16_t  uMode;        // Input / Output AAC mode
  uint32_t  dwFrequency;  // Sampling Frequency
  uint16_t  uQuality;     // Audio Quality
  uint32_t  dwReserved;   // Reserved for future use
} camera_aac_encoding_info_type;

typedef enum {
  TIFF_DATA_BYTE = 1,
  TIFF_DATA_ASCII = 2,
  TIFF_DATA_SHORT = 3,
  TIFF_DATA_LONG = 4,
  TIFF_DATA_RATIONAL = 5,
  TIFF_DATA_UNDEFINED = 7,
  TIFF_DATA_SLONG = 9,
  TIFF_DATA_SRATIONAL = 10
} tiff_data_type;

typedef struct {
  /* What is the ID for this sensor */
  uint16_t sensor_id;
  /* Sensor model number, null terminated, trancate to 31 characters */
  char sensor_model[32];
  /* Width and height of the sensor */
  uint16_t sensor_width;
  uint16_t sensor_height;
  /* Frames per second */
  uint16_t fps;
  /* Whether the device driver can sense when sensor is rotated */
  int8_t  sensor_rotation_sensing;
  /* How the sensor are installed */
  uint16_t default_rotation;
  camera_orientation_type default_orientation;
  /*To check antibanding support */
  int8_t  support_auto_antibanding;
} camera_info_type;

typedef struct {
  int32_t                quality;
  camera_encode_type     format;
  int32_t                file_size;
} camera_encode_properties_type;

typedef enum {
  CAMERA_DEVICE_MEM,
  CAMERA_DEVICE_EFS,
  CAMERA_DEVICE_MAX
} camera_device_type;

#define MAX_JPEG_ENCODE_BUF_NUM 4
#define MAX_JPEG_ENCODE_BUF_LEN (1024*8)

typedef struct {
  uint32_t buf_len;/* Length of each buffer */
  uint32_t used_len;
  int8_t   valid;
  uint8_t  *buffer;
} camera_encode_mem_type;

typedef struct {
  camera_device_type     device;
#ifndef FEATURE_CAMERA_ENCODE_PROPERTIES
  int32_t                quality;
  camera_encode_type     format;
#endif /* nFEATURE_CAMERA_ENCODE_PROPERTIES */
  int32_t                encBuf_num;
  camera_encode_mem_type encBuf[MAX_JPEG_ENCODE_BUF_NUM];
} camera_handle_mem_type;

#ifdef FEATURE_EFS
typedef struct {
  camera_device_type     device;
  #ifndef FEATURE_CAMERA_ENCODE_PROPERTIES
  int32_t                quality;
  camera_encode_type     format;
  #endif /* nFEATURE_CAMERA_ENCODE_PROPERTIES */
  char                   filename[FS_FILENAME_MAX_LENGTH_P];
} camera_handle_efs_type;
#endif /* FEATURE_EFS */

typedef enum {
  CAMERA_PARM_FADE_OFF,
  CAMERA_PARM_FADE_IN,
  CAMERA_PARM_FADE_OUT,
  CAMERA_PARM_FADE_IN_OUT,
  CAMERA_PARM_FADE_MAX
} camera_fading_type;

typedef union {
  camera_device_type      device;
  camera_handle_mem_type  mem;
} camera_handle_type;

typedef enum {
  CAMERA_AUTO_FOCUS,
  CAMERA_MANUAL_FOCUS
} camera_focus_e_type;

/* AEC: Frame average weights the whole preview window equally
   AEC: Center Weighted weights the middle X percent of the window
   X percent compared to the rest of the frame
   AEC: Spot metering weights the very center regions 100% and
   discounts other areas                                        */
typedef enum {
  CAMERA_AEC_FRAME_AVERAGE,
  CAMERA_AEC_CENTER_WEIGHTED,
  CAMERA_AEC_SPOT_METERING,
  CAMERA_AEC_SMART_METERING,
  CAMERA_AEC_USER_METERING,
  CAMERA_AEC_SPOT_METERING_ADV,
  CAMERA_AEC_CENTER_WEIGHTED_ADV,
  CAMERA_AEC_MAX_MODES
} camera_auto_exposure_mode_type;

/* Auto focus mode, used for CAMERA_PARM_AF_MODE */
typedef enum {
  AF_MODE_UNCHANGED = -1,
  AF_MODE_NORMAL    = 0,
  AF_MODE_MACRO,
  AF_MODE_AUTO,
  AF_MODE_CAF,
  AF_MODE_INFINITY,
  AF_MODE_MAX
} isp3a_af_mode_t;

/* VFE Focus Region:
 *  VFE input image dimension
 * VFE Focus Window:
 *  A rectangle in the VFE Focus Region. VFE Focus Window
 *  area must not exceed one quarter of the area of VFE Focus
 *  Region.
 * Display Focus Region:
 *  Diplay dimensions (not LCD dimensions)
 * Display Focus Window:
 *  A rectangle in Display Focus Region
 *  Focus Window Freedom: Movement of Focus Window in x and y direction.
 */

typedef  struct {
  /* Focus Window dimensions, could be negative. */
  int16_t x;
  int16_t y;
  int16_t dx;
  int16_t dy;

  /* Focus Window Freedom granularity in x-direction */
  int16_t dx_step_size;

  /* Focus Window Freedom granularity in y-direction */
  int16_t dy_step_size;

  /*  Focus Window can only move within this Focus Region and
   *  the maximum Focus Window area must not exceed one quarter of the
   *  Focus Region.
   */
  int16_t min_x;
  int16_t min_y;
  int16_t max_x;
  int16_t max_y;
} camera_focus_window_type;

typedef unsigned int Offline_Input_PixelSizeType;
typedef uint16_t Offline_Snapshot_PixelSizeType;
typedef uint16_t Offline_Thumbnail_PixelSizeType;
typedef uint16_t Offline_NumFragments_For_Input;
typedef uint16_t Offline_NumFragments_For_Output;

typedef  struct {
  /* Focus Window dimensions */
  int16_t x_upper_left;
  int16_t y_upper_left;
  int16_t width;
  int16_t height;
} camera_focus_rectangle_dimensions_type;

typedef  struct {
  int16_t focus_window_count;
  camera_focus_rectangle_dimensions_type *windows_list;
} camera_focus_window_rectangles_type;

typedef enum Offline_InputFormatType {
  CAMERA_BAYER_G_B,
  CAMERA_BAYER_B_G,
  CAMERA_BAYER_G_R,
  CAMERA_BAYER_R_G,
  CAMERA_YCbCr_Y_Cb_Y_Cr,
  CAMERA_YCbCr_Y_Cr_Y_Cb,
  CAMERA_YCbCr_Cb_Y_Cr_Y,
  CAMERA_YCbCr_Cr_Y_Cb_Y,
  CAMERA_YCbCr_4_2_2_linepacked,
  CAMERA_YCbCr_4_2_0_linepacked,
  CAMERA_NumberOf_InputFormatType   /* Used for count purposes only */
} Offline_InputFormatType;

typedef enum Offline_YCbCr_InputCositingType {
  CAMERA_CHROMA_NOT_COSITED,
  CAMERA_CHROMA_COSITED,
  CAMERA_NumberOf_YCbCr_InputCositingType   /* Used for count purposes only */
} Offline_YCbCr_InputCositingType;

typedef enum Offline_Input_PixelDataWidthType {
  CAMERA_8Bit,
  CAMERA_10Bit,
  CAMERA_NumberOf_PixelDataWidthType /* Used for count purposes only */
} Offline_Input_PixelDataWidthType;

typedef struct OfflineInputConfigurationType {
  Offline_YCbCr_InputCositingType  YCbCrCositing    ;
  Offline_InputFormatType          format           ;
  Offline_Input_PixelDataWidthType dataWidth        ;
  Offline_Input_PixelSizeType      height           ;
  Offline_Input_PixelSizeType      width            ;
  Offline_Thumbnail_PixelSizeType  thumbnail_width  ;
  Offline_Thumbnail_PixelSizeType  thumbnail_height ;
  Offline_Snapshot_PixelSizeType   snapshot_width   ;
  Offline_Snapshot_PixelSizeType   snapshot_height  ;
  char*                            file_name        ;
  Offline_NumFragments_For_Input   input_fragments  ;
  Offline_NumFragments_For_Output  output_fragments ;
} OfflineInputConfigurationType;

/* Enum Type for bracketing support */
typedef enum {
  CAMERA_BRACKETING_OFF,
  CAMERA_BRACKETING_EXPOSURE,
  CAMERA_BRACKETING_MAX
} camera_bracketing_mode_type;

/* Enum Type for denoise modes */
typedef enum {
  WAVELET_DENOISE_YCBCR_PLANE,
  WAVELET_DENOISE_CBCR_ONLY,
  WAVELET_DENOISE_STREAMLINE_YCBCR,
  WAVELET_DENOISE_STREAMLINED_CBCR,
} wd_process_plane_t;

/*
 * Best Shot Modes
 *
 * DESCRIPTION
 *  When best shot mode is enabled in the service layer, if the
 *  current mode is on, then it will remember the active parameter
 *  values. When best shot mode is disabled, the service layer will
 *  restore parameters that were overwritten when best shot mode was
 *  enabled.
 *
 *  For example, white balance was set to CAMERA_WB_INCANDESCENT,
 *  and LANDSCAPE best shot mode is now active, so the active WB
 *  mode is OUTDOOR:
 *
 *  While LANDSCAPE best shot mode is active:
 *
 *  If application sets WB to FLUORESCENT, then the remembered WB
 *  mode is FLUORESCENT, but the active WB is still OUTDOOR. When
 *  best shot mode is disabled, WB is restored to FLUORESCENT.
 *
 *  When the application gets WB parm, the service layer returns
 *  INCANDESCENT, not OUTDOOR.
 *
 *  SPECIAL EXCEPTIONS
 *  The service layer will always set EV to 0 upon changing a best shot
 *  mode. This includes enabling, disabling, and switching bes tshot modes.
 *  The UI should reflect this setting. The user can then adjust EV.
 *
 *  The service layer will also always set Contrast to the default setting when
 *  changing best shot modes. This includes enabling, disabling and switching
 *  best shot modes.
 *
 *  It is recommended that the UI also update EV and Contrast to the default
 *  values when changing best shot modes.
 *
 *  When the best shot mode specifies AUTO WB the service layer will accept
 *  and apply any manual WB. When the best shot mode specifies OUTDOOR WB the
 *  service layer will accept and apply CLOUDY or DAYLIGHT manual WB settings.
 *  Any other WB settings will be remembered and restored when best shot is
 *  disabled.
 *
 *  Exposure metering settings may be over written when entering a best shot
 *  mode. Any user set exposure metering will be applied immediately.
 *
 *  ISO and hand jitter reduction can not be used with a strobe flash. If
 *  a flash is to be used these features will be disabled.
 *
 *  For BEST SHOT if HJR is ON or a specific ISO setting is specified then
 *  no ISO/HJR settings will be applied while the best shot mode is
 *  enabled. If HJR is OFF any ISO setting besides HJR will be accepted.
 *  If HJR is KEEP CURRENT any ISO setting may be applied provided that
 *  the best shot ISO setting is auto.
 */

/* This list must match the best shot modes defined in
 * camera_bestshot_config.h
 */
typedef enum {
  CAMERA_BESTSHOT_OFF = 0,
  CAMERA_BESTSHOT_AUTO = 1,
  CAMERA_BESTSHOT_LANDSCAPE = 2,
  CAMERA_BESTSHOT_SNOW,
  CAMERA_BESTSHOT_BEACH,
  CAMERA_BESTSHOT_SUNSET,
  CAMERA_BESTSHOT_NIGHT,
  CAMERA_BESTSHOT_PORTRAIT,
  CAMERA_BESTSHOT_BACKLIGHT,
  CAMERA_BESTSHOT_SPORTS,
  CAMERA_BESTSHOT_ANTISHAKE,
  CAMERA_BESTSHOT_FLOWERS,
  CAMERA_BESTSHOT_CANDLELIGHT,
  CAMERA_BESTSHOT_FIREWORKS,
  CAMERA_BESTSHOT_PARTY,
  CAMERA_BESTSHOT_NIGHT_PORTRAIT,
  CAMERA_BESTSHOT_THEATRE,
  CAMERA_BESTSHOT_ACTION,
  CAMERA_BESTSHOT_AR,
  CAMERA_BESTSHOT_MAX
} camera_bestshot_mode_type;


typedef struct la_config {
  /* Pointer in input image - input */
  uint8_t *data_in;
  /* Luma re-mapping curve - output */
  uint16_t *la_curve;
  /* Detect - if 1 image needs LA , if 0 does not - output */
  int16_t detect;
  /* input image size - input */
  uint32_t size;
  /* Old gamma correction LUT - input */
  uint8_t *gamma;
  /* New gamma correction LUT - output */
  uint8_t *gamma_new;
  /* Chroma scale value - output */
  uint32_t chroma_scale_Q20;

  /* Detection related parameters - inputs*/
  /****************************************/
  uint8_t low_range;
  uint16_t low_perc_Q11;
  uint8_t high_range;
  uint16_t high_perc_Q11;

  /* Capping of the histogram - input*/
  uint16_t cap_Q10;

  /* The range to test if diffusion shift is needed - input*/
  uint8_t diff_range;
  /* The percentile to test if diffusion shift is needed - input*/
  uint16_t diff_tail_threshold_Q8;

  /* Scale how much to include contrast tranform
     0 - no contrast transform, 32 - all is contrast transform   - input*/
  uint16_t scale;

  /* Cap high final re-mapping function - input */
  uint16_t cap_high_Q2;
  /* Cap low final re-mapping function - input */
  uint16_t cap_low_Q2;
  /* Number of itterations for the diffusion */
  uint16_t numIt;

} la_config;

#if defined CAMERA_WB_AUTO
#undef CAMERA_WB_AUTO
#endif

#if defined CAMERA_WB_CUSTOM
#undef CAMERA_WB_CUSTOM
#endif

#if defined  CAMERA_WB_INCANDESCENT
#undef CAMERA_WB_INCANDESCENT
#endif

#if defined CAMERA_WB_FLUORESCENT
#undef CAMERA_WB_FLUORESCENT
#endif

#if defined CAMERA_WB_DAYLIGHT
#undef CAMERA_WB_DAYLIGHT
#endif

#if defined CAMERA_WB_CLOUDY_DAYLIGHT
#undef CAMERA_WB_CLOUDY_DAYLIGHT
#endif

#if defined CAMERA_WB_TWILIGHT
#undef CAMERA_WB_TWILIGHT
#endif

#if defined CAMERA_WB_SHADE
#undef CAMERA_WB_SHADE
#endif

typedef enum {
  LED_MODE_OFF,
  LED_MODE_AUTO,
  LED_MODE_ON,
  LED_MODE_TORCH,

  /*new mode above should be added above this line*/
  LED_MODE_MAX
} led_mode_t;

typedef enum {
  STROBE_FLASH_MODE_OFF,
  STROBE_FLASH_MODE_AUTO,
  STROBE_FLASH_MODE_ON,
  STROBE_FLASH_MODE_MAX,
} strobe_flash_mode_t;

/* Clockwise */
typedef enum {
  CAMERA_ENCODING_ROTATE_0,
  CAMERA_ENCODING_ROTATE_90,
  CAMERA_ENCODING_ROTATE_180,
  CAMERA_ENCODING_ROTATE_270
} camera_encoding_rotate_t;

typedef enum {
  MOTION_ISO_OFF,
  MOTION_ISO_ON
} motion_iso_t;

typedef enum {
  FPS_MODE_AUTO,
  FPS_MODE_FIXED,
} fps_mode_t;

typedef struct {
  int32_t minimum_value; /* Minimum allowed value */
  int32_t maximum_value; /* Maximum allowed value */
  int32_t step_value;    /* step value */
  int32_t default_value; /* Default value */
  int32_t current_value; /* Current value */
} cam_parm_info_t;

typedef struct {
  struct msm_ctrl_cmd ctrlCmd;
  int fd;
  void (*af_cb)(int8_t );
  int8_t is_camafctrl_thread_join;
  isp3a_af_mode_t af_mode;
} cam_af_ctrl_t;

typedef enum {
  AUTO = 1,
  SPOT,
  CENTER_WEIGHTED,
  AVERAGE
} cam_af_focusrect_t;

typedef enum {
  CAF_OFF,
  CAF_ON
} caf_ctrl_t;

extern camera_iso_mode_type max_camera_iso_type;


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
#define MP12_WIDTH      4000
#define MP12_HEIGHT     3000


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

#define CAMERA_MIN_BRIGHTNESS  0
#define CAMERA_DEF_BRIGHTNESS  3
#define CAMERA_MAX_BRIGHTNESS  6
#define CAMERA_BRIGHTNESS_STEP 1

#define CAMERA_MIN_CONTRAST    0
#define CAMERA_DEF_CONTRAST    5
#define CAMERA_MAX_CONTRAST    10
#define CAMERA_CONTRAST_STEP   1

#define CAMERA_MIN_SCE_FACTOR    -100
#define CAMERA_DEF_SCE_FACTOR    0
#define CAMERA_MAX_SCE_FACTOR    100

/* No saturation for default */
#define CAMERA_MIN_SATURATION  0
#define CAMERA_DEF_SATURATION  5
#define CAMERA_MAX_SATURATION  10
#define CAMERA_SATURATION_STEP 1

/* No hue for default. */
#define CAMERA_MIN_HUE         0
#define CAMERA_DEF_HUE         0
#define CAMERA_MAX_HUE         300
#define CAMERA_HUE_STEP        60

/* No sharpness for default */
#define CAMERA_MIN_SHARPNESS   0
#define CAMERA_DEF_SHARPNESS   10
#define CAMERA_MAX_SHARPNESS   30
#define CAMERA_SHARPNESS_STEP  5

#define CAMERA_MIN_ZOOM  0
#define CAMERA_DEF_ZOOM  0
#define CAMERA_MAX_ZOOM  0x31
#define CAMERA_ZOOM_STEP 0x3

typedef struct video_frame_info {
  /* NOTE !!!! Important:  It's recommended to make sure both
     w/h of the buffer & image are 32x. When rotation is needed,
     the values in this data structure is POST-rotation. */

  uint32_t               y_buffer_width;     /* y plane */
  uint32_t               cbcr_buffer_width;  /* cbcr plane */
  uint32_t               image_width;        /**< original image width.   */
  uint32_t               image_height;       /**< original image height. */
  uint32_t               color_format;
} video_frame_info;

typedef enum camera_rotation_type {
  ROT_NONE               = 0,
  ROT_CLOCKWISE_90       = 1,
  ROT_CLOCKWISE_180      = 6,
  ROT_CLOCKWISE_270      = 7,
} camera_rotation_type;

typedef struct video_rotation_param_ctrl_t {
  camera_rotation_type rotation; /* 0 degree = rot disable. */
} video_rotation_param_ctrl_t;

typedef struct video_dis_param_ctrl_t {
  uint32_t dis_enable;       /* DIS feature: 1 = enable, 0 = disable.
                               when enable, caller makes sure w/h are 10% more. */
  uint32_t video_rec_width;  /* video frame width for recording */
  uint32_t video_rec_height; /* video frame height for recording */
  uint32_t output_cbcr_offset;
} video_dis_param_ctrl_t;

typedef struct {
  uint32_t               dis_enable;  /* DIS feature: 1 = enable, 0 = disable.
                                           when enable, caller makes sure w/h are 10% more. */
  camera_rotation_type         rotation;        /* when rotation = 0, that also means it is disabled.*/
  video_frame_info       input_frame;
  video_frame_info       output_frame;
} video_param_ctrl_t;

typedef enum {
  CAM_CLIENT_AEC_SETTLED,
  CAM_CLIENT_UPDATE_INST_HANDLE,
} cam_mctl_client_params_t;

typedef struct {
  /* Notifications for the ctrl commands sent to mctl through the pipe */
  int (*ctrl_cmd_done)(void* handle, struct msm_ctrl_cmd *);
  /* Parameter updates */
  void (*update_params)(void* handle, cam_mctl_client_params_t, void*);
  /* client handle */
  void* handle;
} camera_mctl_client_ops_t;

/* config thread utility functions */
struct config_thread_arguments {
  char config_name[MAX_DEV_NAME_LEN];
  char mctl_node_name[MAX_DEV_NAME_LEN];
  int server_fd;
  int read_fd;
  int write_fd;
  pthread_t thread_id;
  int vnode_id;
  int ez_client_fd;
  int ez_read_fd;
  int ez_write_fd;
  int ez_prev_client_fd;
  int ez_prev_read_fd;
  int ez_prev_write_fd;
  camera_mctl_client_ops_t *p_client_ops;
};

void *cam_conf (void *data);
void *cam_conf_v4l2 (void *data);
int launch_cam_conf_thread(void);
int launch_v4l2_conf_thread(struct config_thread_arguments* arg);
int wait_cam_conf_ready(void);
int release_cam_conf_thread(void);
int release_v4l2_cam_conf_thread(void);
void set_config_start_params(config_params_t*);
void *create_v4l2_conf_thread(struct config_thread_arguments* arg);
int destroy_v4l2_cam_conf_thread(void *handle);
int launch_camafctrl_thread(cam_af_ctrl_t *pAfctrl);
/* Stats */
typedef enum {
  CAM_STATS_MSG_HIST,
  CAM_STATS_MSG_EXIT
}camstats_msg_type;

/* Stats messages */
typedef struct {
  camstats_msg_type msg_type;
  union {
    camera_preview_histogram_info hist_data;
  } msg_data;
} camstats_msg;
/*cam stats thread*/
int launch_camstats_thread(void);
void release_camstats_thread(void);
int8_t send_camstats(camstats_type msg_type, void* data, int size);
int8_t send_camstats_msg(camstats_type stats_type, camstats_msg* p_msg);
int is_camstats_thread_running(void);

/* cam frame*/
typedef struct {
  cam_preview_mode_t m;
  camera_mode_t cammode;
} cam_frame_start_parms;

struct v4l2_frame_buffer {
  struct v4l2_buffer buffer;
  unsigned long addr[VIDEO_MAX_PLANES];
  uint32_t size;
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
};

typedef struct {
  int camfdP;        /* preview fd */
  int numFramesP;   /* preview buf */
  int numFramesV;   /* video buf */
  struct v4l2_frame_buffer *framesP;
  int camfdV;
  int camfdS;
  int camfdT;
  int numFramesS;   /* snap shot */
  int numFramesT;   /* thumb image */
  struct v4l2_frame_buffer *framesS;
  struct v4l2_frame_buffer *framesV;
  struct v4l2_frame_buffer *framesT;
  struct v4l2_crop *crop;
  cam_format_t image_format;
} cam_frame_v4l2_start_parms;

int launch_camframe_thread(cam_frame_start_parms* parms);
void release_camframe_thread(void);
void release_camframe_v4l2_thread(void);
void camframe_terminate(void);
int launch_camframe_v4l2_thread(cam_frame_v4l2_start_parms* parms);
void *cam_frame(void *data);
void *cam_frame_set_exit_flag(int flag);
/*sends the free frame of given type to mm-camera*/
int8_t camframe_add_frame(cam_frame_type_t, struct msm_frame*);
/*release all frames of given type*/
int8_t camframe_release_all_frames(cam_frame_type_t);

enum focus_distance_index{
  FOCUS_DISTANCE_NEAR_INDEX,  /* 0 */
  FOCUS_DISTANCE_OPTIMAL_INDEX,
  FOCUS_DISTANCE_FAR_INDEX,
  FOCUS_DISTANCE_MAX_INDEX
};

typedef struct {
  float focus_distance[FOCUS_DISTANCE_MAX_INDEX];
} focus_distances_info_t;

struct msm_frame* get_frame(struct fifo_queue* queue);
int8_t add_frame(struct fifo_queue* queue, struct msm_frame *p);
void flush_queue (struct fifo_queue* queue);
void wait_queue (struct fifo_queue* queue);
void signal_queue (struct fifo_queue* queue);
void flush_and_destroy_queue (struct fifo_queue* queue);
/* queue iterator */
struct msm_frame* begin (void** iterator, struct fifo_queue* queue);
struct msm_frame* next (void** iterator);
struct msm_frame* end (struct fifo_queue* queue);

/* Display */
typedef struct {
    uint16_t user_input_display_width;
    uint16_t user_input_display_height;
} USER_INPUT_DISPLAY_T;
int launch_camframe_fb_thread(void);
void release_camframe_fb_thread(void);
void use_overlay_fb_display_driver(void);
int v4l2_render(int frame_fd, struct v4l2_buffer *vb, struct v4l2_crop *crop);
int cam_get_snapshot_images(void *data, cam_ctrl_dimension_t *dimension);
void v4l2_test_app_jpeg_fwrite(uint8_t *buf_ptr, uint32_t buf_size);
void v4l2_test_app_jpeg_fclose(void);
int cam_get_raw_images(int fd, struct v4l2_frame_buffer *buffers, int buffer_num);
int cam_get_thumbnail_images(int fd, struct v4l2_frame_buffer *buffers, int buffer_num);

int qcamsvr_start(void);
extern void mmcamera_util_profile(const char *str);

/* This structure defines the format of an image */
typedef struct yuv_image_struct {
    int  dx;                /* Number of pixels in the x dirctn or in a row*/
    int  dy;                /* Number of pixels in the y dirctn or in a col*/
    //ipl_col_for_type cFormat;  /* Color Format for image                      */
    unsigned char* imgPtr;     /* Pointer to the image data                   */
    int * clrPtr;             /* Pointer to the Color data                   */
} yuv_image_type;

/*
 * the APP event related defines
*/
typedef enum {
  MM_CAMERA_STATS_EVT_HISTO,
  MM_CAMERA_STATS_EVT_MAX
} mm_camera_stats_event_type_t;

typedef enum {
  MM_CAMERA_INFO_EVT_ROI,
  MM_CAMERA_INFO_EVT_MAX
} mm_camera_info_event_type_t;

/* where to put? */
typedef struct {
  int width;
  int height;
  int stride0; /* plane 1 stride, width */
  int stride1; /* plan 2, height */
	cam_format_t fmt;
} mm_vpe_window_type;

typedef struct {
  int x;
  int y;
  int width;
  int height;
} mm_vpe_roi_type;

typedef struct {
  mm_vpe_window_type input;
  mm_vpe_roi_type input_roi;
  mm_vpe_window_type output;
  int rot; /* ROT_NONE, enum camera_rotation_type */
} mm_vpe_pipe_config_parm_type;

struct video_crop_t{
    uint32_t  in1_w;
    uint32_t  out1_w;
    uint32_t  in1_h;
    uint32_t  out1_h;
    uint32_t  in2_w;
    uint32_t  out2_w;
    uint32_t  in2_h;
    uint32_t  out2_h;
    uint8_t update_flag;
};

typedef enum {
  CAM_SENSOR_MODE_PREVIEW,
  CAM_SENSOR_MODE_VIDEO,
  CAM_SENSOR_MODE_SNAPSHOT,
} cam_sensor_mode_type_t;

typedef struct {
  /* input variables */
  cam_sensor_mode_type_t type;
  camera_hfr_mode_t hfr_mode;
  /* output variables */
  int width;
  int height;
} cam_sensor_dim_t;

typedef enum {
  CAM3A_CONV_TYPE_SLOW,
  CAM3A_CONV_TYPE_MEDIUM,
  CAM3A_CONV_TYPE_FAST,
} cam_3a_conv_type_t;

typedef struct {
  cam_3a_conv_type_t conv_type;
  /* 0 - low 1 - med 2 - fast */
  int awb_aggressiveness;
  /* 0.0 to 1.0 */
  float aec_conv_speed;
  /* 2 - 8 */
  long aec_luma_tolerance;
  /*if 0, the default params in each type is set*/
  int force;
} cam_3a_conv_info_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  cam_format_t format;
  uint32_t image_mode;
  uint32_t inst_handle;
  /* If app needs to request for a particular cid,
   * this flag needs to be set and the cid value
   * filled in cid_val. */
  uint8_t req_cid;
  /* per sensor, CID 0 is the default. */
  uint8_t cid_val;
  cam_pad_format_t padding_format; /* default word */
  cam_frame_len_offset_t frame_offset;
  uint8_t active;
} cam_stream_info_def_t;

typedef enum {
  TARGET_DEFAULT,
  TARGET_MSM8930,
} target_type_t;

#endif /* __CAMERA_H__ */
