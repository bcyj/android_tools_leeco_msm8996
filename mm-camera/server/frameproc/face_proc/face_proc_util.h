/**********************************************************************
* Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef FACE_DETECTION_H
#define FACE_DETECTION_H

#include <pthread.h>
#include <stdint.h>
#include "frameproc.h"
#include "camera_dbg.h"

#if(FRAME_PROC_FACE_DETECT_DEBUG)
  #ifdef _ANDROID_
  #include <utils/Log.h>
  #endif
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-FACE_DETECT"
  #define CDBG_FD(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_FD(fmt, args...) do{}while(0)
#endif

/* Helpful macros */
#define FD_SUCCEEDED(rc)     (rc == FD_RET_SUCCESS)
#define FD_FAILED(rc)        (rc != FD_RET_SUCCESS)


//================================================================
//   Enumerations
//================================================================

/* Return result for the Common Interface API calls */
#define   FD_RET_SUCCESS         0
#define   FD_RET_FAILURE         -1
#define   FD_RET_NO_RESOURCE     2
#define   FD_RET_INVALID_PARM    3
#define   FD_RET_BUSY            4
#define   FD_RET_INVALID_STATE   5
#define   FD_RET_UNSUPPORTED     6
#define   FD_RET_UNCHANGED       7

#define MIN2(a,b)      ((a<b)?a:b)
#define MIN4(a,b,c,d)  (MIN2(MIN2(a,b),MIN2(c,d)))
#define MAX2(a,b)      ((a>b)?a:b)
#define MAX4(a,b,c,d)  (MAX2(MAX2(a,b),MAX2(c,d)))
#define CLIP(x, lower, upper)  {x = ((x < lower) ? lower : \
                               ((x > upper) ? upper : x)); }
/* Configurations to FD - potentially stored as chromatix later */
#define MAX_FACES_TO_DETECT   MAX_ROI
#define MIN_FACE_SIZE         50
#define MAX_FACE_SIZE         500
#define FACE_ANGLE_TO_DETECT  FD_ROT_RANGE_MAX_SUPPORTED

/* Feature enable flags in fd_config_t */
#define   FD_ENABLE_HISTOGRAM_GEN        0x01
#define   FD_ENABLE_RECT_RENDERING       0x02

/* Histogram size */
#define   FD_PIXEL_BIT_WIDTH      8
#define   FD_HIST_SIZE            (1<<FD_PIXEL_BIT_WIDTH)

/* A special value for 'rotation_range' under fd_face_cfg_t to
 * indicate maximum supported */
#define   FD_ROT_RANGE_MAX_SUPPORTED    0xffffffff
extern const char FACE_ALBUM[];

typedef enum {
  FD_FRAME_FORMAT_8BIT_GRAYSCALE,
} fd_frame_format_t;

typedef enum {
  FD_FACE_ORIENTATION_0 = 0,
  FD_FACE_ORIENTATION_90,
  FD_FACE_ORIENTATION_180,
  FD_FACE_ORIENTATION_270,
  FD_FACE_ORIENTATION_UNKNOWN,
} fd_face_orientation_t;


/* Specification of frame */
typedef struct {
  uint32_t            width;  /* Frame Width */
  uint32_t            height;  /* Frame Height */
  uint32_t            row_incr;  /* Row Increment in case of frame padding */
  fd_frame_format_t   format;  /* Frame Format */
} fd_frame_cfg_t;

/* Preferences in detection of a face */
typedef struct {
  /* Min  & Max face size in # of pixels in one dimension */
  uint32_t                min_face_size;
  uint32_t                max_face_size;
  /* Max # of faces to detect */
  uint32_t                max_num_face_to_detect;

  /* Hint on orientation of face w.r.t. the frames
   *  It's useful optional information to the FD algorithms
   * that's usually derived from the camera's sensor
   * orientation together with the data from orientation
   * sensor. If no hint is available, set it to
   * FD_FACE_ORIENTATION_UNKNOWN. Algorithms not supporting
   * this hint will return error otherwise.
   */
  fd_face_orientation_t   face_orientation_hint;

  /* Faces inclined at this angle (front-facing plane)
   * on either side will be detected. In units of degree.
   * Use FD_ROT_RANGE_MAX_SUPPORTED to request the maximum
   * supported range. Algorithms not supporting specific ranges
   * will return an error during configuration if a specific
   * range other than FD_ROT_RANGE_MAX_SUPPORTED is requested.
   */
  uint32_t                rotation_range;
} fd_face_cfg_t;

/* To configure the FD module for the next session */
typedef struct {
  fd_frame_cfg_t          frame_cfg;
  fd_face_cfg_t           face_cfg;

  uint32_t                histogram_enable;  /* Bit mask e.g histogram */
} fd_config_t;

/* Information of current frame being processed in FD */
typedef struct {
  uint8_t                *buffer;
  uint32_t                frame_id;  /* Assigned by framework in fd_run */
} fd_frame_t;

/* FD states */
typedef enum {
  FD_STATE_ON,
  FD_STATE_INIT,
  FD_STATE_BUSY,
  FD_STATE_DESTROYING,
  FD_STATE_OFF,
} fd_state_t;

/* Internal FD control structure */

typedef struct {
  void * local_fd_obj;
  int fd_lib_loaded;
  /* State */
  fd_state_t      fd_state;  //State wrt fd_interface
  /* Config */
  fd_config_t        config;
  /* Frame currently being processed */
  fd_frame_t         current_frame;
  /* User data */
  uint32_t           prev_frame_id;
  fd_mode_t            mode;
  uint32_t  num_faces_total;
  frame_proc_t * frame_ctrl;
} fd_ctrl_t;

/*============================================================================
                        EXTERNAL API DECLARATIONS
============================================================================*/
int fd_util_init(void *Ctrl, fd_ctrl_t  *fdCtrl);
int32_t fd_util_execute(void *Ctrl, fd_ctrl_t *fdCtrl);
int32_t fd_util_exit(void *Ctrl, fd_ctrl_t  *fdCtrl);
#endif /* #define FACE_DETECTION_H */
