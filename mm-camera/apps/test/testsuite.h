/*
 * Copyright (c) 2008 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */

#include <linux/types.h>
#include <linux/videodev2.h>

#define TS_CMD_FILENAME_MAX         32
#define TS_CMD_STR_MAX              128

typedef enum
{
  TS_CMD_ID_INVALID = -1,
  /* FUNCTION TEST */
  TS_SERVER_EXIT = 1,               // Used as msg id, has to be greater than 0
  TS_CLIENT_EXIT,                   // TEST CLIENT EXIT, SERVER 

  TS_CMD_FILENAME,                  // This one is for command line only,
                                    // cannot be used in command file.
  TS_MSLEEP,
  TS_SLEEP,
  TS_COUNT,
  TS_COUNT_RESET,
  TS_TIME,
  TS_TIME_MARKER,
  TS_TIME_SINCE_MARKER,
  TS_LOOP_START,
  TS_LOOP_END,
  TS_SYSTEM_INIT,
  TS_SYSTEM_DESTROY,
  TS_PREVIEW_START,
  TS_PREVIEW_STOP,
  TS_SNAPSHOT_YUV_PICTURE,
  TS_SNAPSHOT_JPEG_PICTURE,
  TS_SNAPSHOT_RAW_PICTURE,
  TS_SNAPSHOT_STOP,

  TS_PRINT_MAXZOOM,
  TS_PRINT_ZOOMRATIOS,
  TS_ZOOM_INCREASE,
  TS_ZOOM_DECREASE,
  TS_ZOOM_STEP_INCREASE,
  TS_ZOOM_STEP_DECREASE,
  TS_CONTRAST_INCREASE,
  TS_CONTRAST_DECREASE,
  TS_SATURATION_INCREASE,
  TS_SATURATION_DECREASE,
  TS_SPECIAL_EFFECT,
  TS_BRIGHTNESS_INCREASE,
  TS_BRIGHTNESS_DECREASE,
  TS_EV_INCREASE,
  TS_EV_DECREASE,
  TS_ANTI_BANDING,
  TS_SET_WHITE_BALANCE,
  TS_AEC_MODE,
  TS_ISO_INCREASE,
  TS_ISO_DECREASE,
  TS_SHARPNESS_INCREASE,
  TS_SHARPNESS_DECREASE,
  TS_SET_AUTO_FOCUS,
  TS_SET_HJR,
  TS_SET_LENS_SHADING,
  TS_SET_LED_MODE,
  TS_GET_SHARPNESS_AF,
  TS_SNAPSHOT_RESOLUTION,
  TS_PREVIEW_RESOLUTION,
  TS_MOTION_ISO,
  TS_TOGGLE_HUE,
  TS_CANCEL_AUTO_FOCUS,
  TS_GET_AF_STEP,
  TS_SET_AF_STEP,
  TS_ENABLE_AFD,
  TS_VIDEO_START,
  TS_VIDEO_STOP,

  /* API TEST */
  TEST_VIDIOC_QUERYMENU,
  TEST_VIDIOC_QUERYCTRL,
  TEST_VIDIOC_S_CTRL,
  TEST_VIDIOC_G_CTRL,
  TEST_VIDIOC_CROPCAP,
  TEST_VIDIOC_G_CROP,
  TEST_VIDIOC_S_CROP,                     // CAMERA_SET_PARM_DIMENSION,
                                          // CAMERA_SET_PARM_ZOOM
  TEST_VIDIOC_G_FMT,
  TEST_VIDIOC_S_FMT,
  TEST_V4L2_CID_FOCUS_ABSOLUTE,           // CAMERA_SET_PARM_FOCUS_RECT,
                                          // CAMERA_SET_PARM_FOCUS_STEP
  TEST_V4L2_CID_FOCUS_RELATIVE, 
  TEST_V4L2_CID_FOCUS_AUTO, 
  TEST_V4L2_CID_CONTRAST,                 // CAMERA_SET_PARM_LUMA_ADAPTATION, 
                                          // CAMERA_SET_PARM_CONTRAST
  TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_SPATIAL_FILTER_TYPE,
  TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_BOTTOM,
  TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_TOP,
  TEST_V4L2_CID_BRIGHTNESS,               // CAMERA_SET_PARM_BRIGHTNESS
  TEST_V4L2_CID_EXPOSURE,                 // CAMERA_SET_PARM_EXPOSURE_COMPENSATION
                                          // CAMERA_SET_PARM_EXPOSURE
  TEST_V4L2_CID_EXPOSURE_AUTO,            // CAMERA_SET_PARM_AUTO_EXPOSURE_MODE
  TEST_V4L2_CID_EXPOSURE_ABSOLUTE,
  TEST_V4L2_CID_EXPOSURE_AUTO_PRIORITY,
  TEST_V4L2_CID_SHARPNESS,                // CAMERA_SET_PARM_SHARPNESS
  TEST_V4L2_CID_HUE_AUTO,                 // CAMERA_SET_PARM_HUE
  TEST_V4L2_CID_SATURATION,               // CAMERA_SET_PARM_SATURATION
  TEST_V4L2_CID_AUTO_WHITE_BALANCE_AUTO,  // CAMERA_SET_PARM_WB
  TEST_V4L2_CID_DO_WHITE_BALANCE,
  TEST_V4L2_CID_WHITE_BALANCE_TEMPERATURE,

  TEST_VIDIOC_ENUM_FRAMEINTERVALS,        // CAMERA_SET_PARM_FPS,
                                              // 24/25/30/50/60,
                                              // struct v4l2_buffer->struct v4lw_timecode->type
                                              // CAMERA_SET_PARM_FPS_LIST, 
                                              // CAMERA_SET_PARM_PREVIEW_FPS,
  TEST_VIDIOC_STREAMON,                   // CAMERA_SET_PARM_PREVIEW_MODE
                                              // CAMERA_START_PREVIEW
  TEST_VIDIOC_STREAMOFF,                  // CAMERA_STOP_PREVIEW
  TEST_VIDIOC_ENCODER_CMD,                // CAMERA_SET_PARM_ENCODE_ROTATION
  TEST_VIDIOC_TRY_ENCODER_CMD,

  TEST_VIDIOC_G_PARAM,                    // CAMERA_SET_PARM_FRAME_TIMESTAMP
                                              // v4l2_buffer->timestamp
  TEST_VIDIOC_S_PARAM,

  /* Might not related to V4L2 or private V4L2 API: */
  //  CAMERA_SET_PARM_DISPLAY_INFO,  
  //  CAMERA_SET_PARM_SENSOR_POSITION,
  //  CAMERA_SET_PARM_EFFECT, // Suspended
  //  CAMERA_SET_PARM_FLASH, // ???
  //  CAMERA_SET_PARM_NIGHTSHOT_MODE, // ???
  //  CAMERA_SET_PARM_REFLECT, // ???
  //  CAMERA_SET_PARM_ANTIBANDING, // ???
  //  CAMERA_SET_PARM_RED_EYE_REDUCTION, // ???
  //  CAMERA_SET_PARM_ISO, // ???
  //  CAMERA_SET_PARM_BESTSHOT_MODE, // ???
  //  CAMERA_SET_PARM_EXPOSURE_METERING, // ???
  //  CAMERA_SET_PARM_AF_MODE, // ???
  //  CAMERA_SET_PARM_HISTOGRAM,  /* 30 */ // ???
  //  CAMERA_SET_PARM_FLASH_STATE, // ???
  //  CAMERA_SET_PARM_STROBE_FLASH, // ???
  //  CAMERA_START_RAW_SNAPSHOT, // ???
  TEST_V4L2_CID_START_SNAPSHOT,            //  CAMERA_START_SNAPSHOT
  TEST_V4L2_CID_STOP_SNAPSHOT,             //  CAMERA_STOP_SNAPSHOT

  /* THIS IS THE END */
  TS_CMD_ID_MAX
} TS_CMD_ID;

typedef struct
{
    char * string;
    int id;
} STRING_TO_ID;

int string_to_id( char * str);
void print_help(void);
int read_a_line(char * test_command_filename, char * test_command_buf);
int parse_command( char * command_str, char ** p_command_args, char ** p_command_result);


int testsuite_send(int socket_inet, const char * text);
int testsuite_receive(int socket_inet, char ** text);
int testsuite_server_manager(const char * const server_ip_str, const char * const server_port_str);

