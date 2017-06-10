/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/


#ifndef __FACEPROC_H__
#define __FACEPROC_H__

#include "img_common.h"

/**
 * CONSTANTS and MACROS
 **/

#define FACE_CONTOUR_DETECT 1
#define FACE_RECOGNITION 1
#define FACE_RECOGNIZE_TEST_REGISTER 0
#define FACE_PART_DETECT 1
#define FACE_BGS_DETECT 1
#define FACE_SMILE_DETECT 1

#define MAX_FACE_ROI 5

#define MAX_FACE_PART 12
#define MAX_CONTOUR_PT 45

/* Histogram size */
#define FD_PIXEL_BIT_WIDTH      8
#define FD_HIST_SIZE            (1<<FD_PIXEL_BIT_WIDTH)

#define FD_ROT_RANGE_MAX_SUPPORTED    0xffffffff

/* Configurations to FD - potentially stored as chromatix later */
#define MAX_FACES_TO_DETECT   MAX_FACE_ROI
#define MIN_FACE_SIZE         50
#define MAX_FACE_SIZE         500
#define DEFAULT_FACE_ANGLE    45
#define FACE_ANGLE_TO_DETECT  FD_ROT_RANGE_MAX_SUPPORTED

#define DEFAULT_DT_MODE  3
#define REGISTER_DT_MODE 1

/* Debug Face detection feature mask */
/* Face detection enable */
#define FACE_PROP_ENABLE_FD     (1 << 1)
/* Face detection facial parts on (smile, eyes */
#define FACE_PROP_PARTS_ON      (1 << 2)
/* Face angle on/off */
#define FACE_PROP_ANGLE_ON      (1 << 3)
/* Downscaling enable  */
#define FACE_PROP_DOWNSCALE_ON  (1 << 4)
/* Number of faces if set it will be 10 otherwise 5 */
#define FACE_PROP_MAX_FACES     (1 << 5)
/* Frame dump enable */
#define FACE_PROP_DUMP_FRAMES   (1 << 6)
/* Min face size in pixels 1- 25, 2 - 50, 3 - 100*/
#define FACE_PROP_MIN_FACE_SIZE_SHIFT 7
#define FACE_PROP_MIN_FACE_SIZE (3 << FACE_PROP_MIN_FACE_SIZE_SHIFT)

#define FACE_PROP_DEFAULT \
  (FACE_PROP_ENABLE_FD | FACE_PROP_ANGLE_ON \
  | (0<<FACE_PROP_MIN_FACE_SIZE_SHIFT) \
  | FACE_PROP_PARTS_ON)



/** faceproc_frame_cfg_t
 *   @max_width: maximum width
 *   @max_height: maximum height
 *
 *   faceproc frame configuration
 **/
typedef struct {
  uint32_t max_width;  /* Frame Width */
  uint32_t max_height;  /* Frame Height */
} faceproc_frame_cfg_t;

/** faceproc_orientation_t
 *   FD_FACE_ORIENTATION_0: 0 degreee orientation
 *   FD_FACE_ORIENTATION_90: 90 degree orientation
 *   FD_FACE_ORIENTATION_180: 180 degree orientation
 *   FD_FACE_ORIENTATION_270: 270 degree orientation
 *
 *   face orientation
 **/
typedef enum {
  FD_FACE_ORIENTATION_0 = 0,
  FD_FACE_ORIENTATION_90,
  FD_FACE_ORIENTATION_180,
  FD_FACE_ORIENTATION_270,
  FD_FACE_ORIENTATION_UNKNOWN,
} faceproc_orientation_t;

/** faceproc_frame_cfg_t
 *   @min_face_size: minimum face size in pixels
 *   @max_face_size: maximum face size in pixels
 *   @max_num_face_to_detect: maximum number of faces to detect
 *   @face_orientation_hint: Hint on orientation of face w.r.t.
 *                         the frames. It's useful optional
 *                         information to the FD algorithms
 *                         that's usually derived from the
 *                         camera's sensor orientation together
 *                         with the data from orientation
 *                         sensor. If no hint is available, set
 *                         it to FD_FACE_ORIENTATION_UNKNOWN.
 *                         Algorithms not supporting this hint
 *                         will return error otherwise.
 *   @rotation_range: Faces inclined at this angle (front-facing
 *                  plane) on either side will be detected. In
 *                  units of degree. Use
 *                  FD_ROT_RANGE_MAX_SUPPORTED to request the
 *                  maximum supported range. Algorithms not
 *                  supporting specific ranges will return an
 *                  error during configuration if a specific
 *                  range other than FD_ROT_RANGE_MAX_SUPPORTED
 *                  is requested.
 *
 *   faceproc face configuration
 **/
typedef struct {
  uint32_t min_face_size;
  uint32_t max_face_size;
  uint32_t max_num_face_to_detect;
  faceproc_orientation_t face_orientation_hint;
  uint32_t rotation_range;
  uint32_t detection_mode;
} faceproc_face_cfg_t;

/** faceproc_config_t
 *   @frame_cfg: frame configuration
 *   @face_cfg: face configuration
 *   @histogram_enable: flag to indicate if the histogram is
 *                    enabled
 *   @fd_feature_mask: need for debug bitmask with different
 *   combinations of enabled features.
 *
 *   faceproc configuration
 **/
typedef struct {
  faceproc_frame_cfg_t frame_cfg;
  faceproc_face_cfg_t face_cfg;
  uint32_t histogram_enable;
  uint32_t fd_feature_mask;
} faceproc_config_t;

/** fd_hist_t
 *   @bin: histogram bin
 *   @num_samples: number of samples
 *
 *   faceproc histogram
 **/
typedef struct {
  uint32_t bin[FD_HIST_SIZE];
  uint32_t num_samples;
} fd_hist_t;

/** fd_pixel_t
 *   @x: x cordinate
 *   @y: y cordinate
 *
 *   faceproc pixel information
 **/
typedef struct {
  int32_t x;
  int32_t y;
} fd_pixel_t;


/** face_part_detect
 *
 *   faceproc part type.
 *   Note:- this enumeration matches the Engine enumeration
 *   value. Any update in Engine APIs needs to be matched here.
 *
 **/
typedef enum {
  FACE_PART_LEFT_EYE = 0,  /* Center of left eye        */
  FACE_PART_RIGHT_EYE,     /* Center of right eye       */
  FACE_PART_MOUTH,         /* Mouth Center              */
  FACE_PART_LEFT_EYE_IN,   /* Inner corner of left eye  */
  FACE_PART_LEFT_EYE_OUT,  /* Outer corner of left eye  */
  FACE_PART_RIGHT_EYE_IN,  /* Inner corner of right eye */
  FACE_PART_RIGHT_EYE_OUT, /* Outer corner of right eye */
  FACE_PART_MOUTH_LEFT,    /* Left corner of mouth      */
  FACE_PART_MOUTH_RIGHT,   /* Right corner of mouth     */
  FACE_PART_NOSE_LEFT,     /* Left Nostril              */
  FACE_PART_NOSE_RIGHT,    /* Right Nostril             */
  FACE_PART_MOUTH_UP,      /* Mouth top                 */
} face_part_type_t;

/** face_part_detect
 *   @face_pt: cordinates for face part
 *   @confidence: confidence value for face parts
 *   @direction_up_down: up down degree
 *   @direction_left_right: left right degree
 *   @direction_roll: roll degree
 *
 *   faceproc part information
 **/
typedef struct {
  fd_pixel_t face_pt[MAX_FACE_PART];
  int confidence[MAX_FACE_PART];
  int direction_up_down; // -180 to 179
  int direction_left_right;// -180 to 179
  int direction_roll; //-180 to 179
} face_part_detect;

/** fd_smile_detect
 *   @smile_degree: smile degree
 *   @confidence: confidence of the smile
 *
 *   faceproc smile information
 **/
typedef struct {
 int smile_degree;  //0 - 100
 int confidence;  // 0 -1000
} fd_smile_detect;

/** contour_detect
 *   @contour_pt: countour points
 *
 *   contour information
 **/
typedef struct {
  fd_pixel_t contour_pt[MAX_CONTOUR_PT];
} contour_detect;

/** fd_rect_t
 *   @x: left cordinate
 *   @y: top cordinate
 *   @dx: width of window
 *   @dy: height of the window
 *
 *   faceproc rect information
 **/
typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
} fd_rect_t;

/** faceproc_info_t
 *   @face_boundary: face boundary
 *   @fp: face part information
 *   @ct: contour cordinates
 *   @sm: smile detec information
 *   @is_face_recognised: flag to indicate whether the face is
 *                      recognised
 *   @histogram: histogram for the image
 *   @blink_detected: flag to indicate whether the blink is
 *                  detected
 *   @left_blink: value for the left blink
 *   @right_blink: value of the right blink
 *   @left_right_gaze: value of left-right gaze
 *   @top_bottom_gaze: value of top bottom gaze
 *   @unique_id: unique id for the faceproc output
 *   @gaze_angle: angle of gaze
 *   @fd_confidence: confidence of face detection
 *
 *   faceproc information
 **/
typedef struct {
  fd_rect_t              face_boundary;
  face_part_detect       fp;
  contour_detect         ct;
  fd_smile_detect        sm;
  int                    is_face_recognised;  /* 1 or 0 */
  fd_hist_t              histogram;
  int                    blink_detected; /* 1 or 0 */
  int                    left_blink; // 0 - 1000
  int                    right_blink; // 0 - 1000

  /* left gaze gives +ve value; right gaze gives -ve value */
  int                    left_right_gaze;
  /*top gaze gives -ve, bottom gaze give +ve value */
  int                    top_bottom_gaze;
  int                    unique_id;
  int                    gaze_angle;// -90, -45, 0 , 45 ,90
  int                    fd_confidence;
} faceproc_info_t;

/** faceproc_result_t
 *   @roi: result for each region of interest
 *   @num_faces_detected: number of faces detected
 *   @trans_info: translation info for the face cordinates
 *   @frame_id: frame id
 *
 *   faceproc information
 **/
typedef struct {
  faceproc_info_t roi[MAX_FACE_ROI];
  uint32_t num_faces_detected;
  img_trans_info_t trans_info;
  int frame_id;
} faceproc_result_t;

/** faceproc_mode_t
 *   FACE_DETECT_OFF: face detection is off
 *   FACE_DETECT: face detection mode
 *   FACE_RECOGNIZE: face recognition mode
 *   FACE_REGISTER: face registration mode
 *   CLEAR_ALBUM: album clear mode
 *
 *   faceproc information
 **/
typedef enum {
  FACE_DETECT_OFF,
  FACE_DETECT,
  FACE_RECOGNIZE,
  FACE_REGISTER,
  CLEAR_ALBUM,
}faceproc_mode_t;

/** QWD_FACEPROC_CFG
 *
 *   datastructure passed needs to be of type faceproc_config_t
 **/
#define QWD_FACEPROC_CFG     (QIMG_FACEPROC_PARAM_OFF +  1)

/** QWD_FACEPROC_RESULT
 *
 *   datastructure passed needs to be of type faceproc_result_t
 **/
#define QWD_FACEPROC_RESULT  (QIMG_FACEPROC_PARAM_OFF +  2)

/** QWD_FACEPROC_MODE
 *
 *   datastructure passed needs to be of type faceproc_mode_t
 **/
#define QWD_FACEPROC_MODE    (QIMG_FACEPROC_PARAM_OFF +  3)

/** QWD_FACEPROC_CHROMATIX
 *
 *   datastructure passed needs to be of type fd_chromatix_t
 **/
#define QWD_FACEPROC_CHROMATIX   (QIMG_FACEPROC_PARAM_OFF +  4)

/** FD_PRINT_RECT:
   *  @p: fd rect
   *
   *  prints the crop region
   **/
#define FD_PRINT_RECT(p) ({ \
  IDBG_MED("%s:%d] FD crop info (%d %d %d %d)", __func__, __LINE__, \
    (p)->x, \
    (p)->y, \
    (p)->dx, \
    (p)->dy); \
})


#endif //__FACEPROC_H__
