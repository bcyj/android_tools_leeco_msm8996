/* 
 * Copyright (c) 2008-2011 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include <pthread.h>

#include "testsuite.h"

#include "camera.h"
#include "camera_dbg.h"
#include "cam_mmap.h"
#include "jpeg_encoder.h"
#include "common_cam.h"

static int fd = -1;

extern int camfd;
extern interface_ctrl_t intrfcCtrl;

static char * device_str = "/dev/video20";

cam_ctrl_dimension_t *dimension;
common_crop_t *scaling_params;
extern int pmemThumbnailfd, pmemSnapshotfd;
uint8_t * thumbnail_buf, *main_img_buf;
pthread_t cam_conf_thread, frame_thread;
struct msm_frame frames[4];

extern char *sdcard_path;

static int msm_v4l2_vidioc_g_fmt()
{
    struct v4l2_format fmt;

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) {
        printf("%s failed\n", __FUNCTION__ );
        return -1;
    }
    else {
        printf("type:%d\n", fmt.type);
        printf("width:%d, height:%d, pixelformat:0x%X, field:%d\n",
               fmt.fmt.pix.width, fmt.fmt.pix.height,
               fmt.fmt.pix.pixelformat, fmt.fmt.pix.field);
        printf("%s success\n", __FUNCTION__);
    }

    return 0;
}

static int msm_v4l2_vidioc_s_fmt()
{
    struct v4l2_format fmt;

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (ioctl (fd, VIDIOC_S_FMT, &fmt) == -1) {
         printf("%s failed\n", __FUNCTION__);
         return -1;
    }
    else {
         printf("%s success\n", __FUNCTION__);
    }
    return 0;
}

static int msm_v4l2_vidioc_cropcap()
{
    struct v4l2_cropcap cropcap;

    memset( &cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_CROPCAP, &cropcap) == -1) {
        printf("%s failed\n", __FUNCTION__ );
        return -1;
    }
    else {
        printf("type:%d\n", cropcap.type);
        printf("bounds: left-%d, top-%d, width-%d, height-%d\n",
               cropcap.bounds.left, cropcap.bounds.top,
               cropcap.bounds.width, cropcap.bounds.height);
        printf("defrect: left-%d, top-%d, width-%d, height-%d\n",
               cropcap.defrect.left, cropcap.defrect.top,
               cropcap.defrect.width, cropcap.defrect.height);
        printf("pixelaspect: numerator-%d, denominator-%d\n",
               cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);
        printf("%s success\n", __FUNCTION__);
    }
		return TRUE;
}

static int msm_v4l2_vidioc_g_crop()
{
    struct v4l2_crop crop;

    memset( &crop, 0, sizeof(crop));
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_G_CROP, &crop) == -1) {
        printf("%s failed\n", __FUNCTION__ );
        return -1;
    }
    else {
        printf("type:%d\n", crop.type);
        printf("left-%d, top-%d, width-%d, height-%d\n",
               crop.c.left, crop.c.top, crop.c.width, crop.c.height);
        printf("%s success\n", __FUNCTION__);
    }

    return 0;
}

static int msm_v4l2_vidioc_s_crop()
{
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;

    memset( &cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_CROPCAP, &cropcap) == -1) {
        printf("%s failed\n", __FUNCTION__ );
        return -1;
    }
    else {
        printf("%s success\n", __FUNCTION__);
    }

    memset( &crop, 0, sizeof(crop));
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;

    if (ioctl(fd, VIDIOC_S_CROP, &crop) == -1) {
        printf("%s failed\n", __FUNCTION__ );
        return -1;
    }
    else {
        printf("%s success\n", __FUNCTION__);
    }

    return 0;
}

static int msm_v4l2_vidioc_queryctrl(char * args);

struct v4l2_queryctrl queryctrl;
static int msm_v4l2_vidioc_querymenu(char * args)
{
    int min, max;
    char * pstr_id, * pstr_min, * pstr_max;
    struct v4l2_querymenu querymenu;
    char *prev_value;

    memset( &querymenu, 0, sizeof(querymenu));

    if (args != NULL) {
        pstr_id = strtok_r(args, ", ", &prev_value);
        if (!pstr_id) {
          printf("msm_v4l2_vidioc_querymenu: Invalid Args\n");
          return -1;
        }
        pstr_min = strtok_r(NULL, ", ", &prev_value);
        pstr_max = strtok_r(NULL, ", ", &prev_value);
        querymenu.id = string_to_id(pstr_id);
        if (msm_v4l2_vidioc_queryctrl(args) == -1)
        {
          printf("%s not supported\n", args);
          return -1;
        }
        if (pstr_min == NULL) {
          min = queryctrl.minimum;
          max = queryctrl.maximum;
        } else {
          min = atoi(pstr_min);
          if (!pstr_max) {
            printf("msm_v4l2_vidioc_querymenu: Invalid Args\n");
            return -1;
          }

          max = atoi(pstr_max);
        }
    } else {
        // in this case queryctrl id has been set;
        querymenu.id = queryctrl.id;
        min = queryctrl.minimum;
        max = queryctrl.maximum;
    }

    for(querymenu.index = min;
        querymenu.index < (unsigned int) max;
        querymenu.index ++) {
        if ( ioctl(fd, VIDIOC_QUERYMENU, &querymenu)) {
            printf("%s: %s\n", __FUNCTION__, querymenu.name);
        } else {
            printf("%s failed\n", __FUNCTION__);
            return -1;
        }
    }

    return 0;
}

static int msm_v4l2_vidioc_queryctrl(char * args)
{
    int result = 0;
    int id_start, id_end;
    char * pstr_id;
    char *prev_value;
    memset(&queryctrl, 0, sizeof (queryctrl));

    if ( args == NULL ) {
        id_start = V4L2_CID_BASE;
        id_end = V4L2_CID_LASTP1 - 1;
    } else {
        pstr_id = strtok_r(args, ", ", &prev_value);
        id_start = string_to_id(args);
        id_end = id_start;
    }

    for (queryctrl.id = id_start;
         queryctrl.id <= (unsigned int) id_end;
         queryctrl.id++)
    {
        if (ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
            printf ("%s: Control 0x%X %s", __FUNCTION__, queryctrl.id, queryctrl.name);
            if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                printf (" disabled\n");
                continue;
            }
            printf (" OK\n");
            if (queryctrl.type == V4L2_CTRL_TYPE_MENU) {
                msm_v4l2_vidioc_querymenu(NULL);
            }
        } else {
            printf ("%s failed id 0x%X \n", __FUNCTION__, queryctrl.id);
            result = -1;
        }
    }
    return result;
}

static int msm_v4l2_vidioc_s_ctrl(char * args)
{
    int id;
    struct v4l2_control control;
    char * pstr_of_id;
    char * pstr_of_value;
    char *prev_value;

    pstr_of_id = strtok_r(args, ", ", &prev_value);
    if (!pstr_of_id) {
      printf("%s: invalid args\n", __FUNCTION__);
      return -1;
    }

    pstr_of_value = strtok_r(NULL, ", ", &prev_value);

    memset( &control, 0, sizeof(control));
    control.id = id = string_to_id(pstr_of_id);
    if (id < 0) {
        printf("%s: invalid ID\n", __FUNCTION__);
        return -1;
    }

    if (pstr_of_value == NULL) {
        msm_v4l2_vidioc_queryctrl(args);
        control.value = queryctrl.default_value;
    } else {
        control.value = atoi(pstr_of_value);
    }

    if ( ioctl(fd, VIDIOC_S_CTRL, & control) == -1) {
        printf("%s failed\n", __FUNCTION__);
        return -1;
    } else {
        printf("%s successed\n", __FUNCTION__);
    }

    return 0;
}

static int msm_v4l2_vidioc_g_ctrl(char * args)
{
    int id;
    struct v4l2_control control;
    char * pstr_of_id = args;

    memset( &control, 0, sizeof(control));
    control.id = id = string_to_id(pstr_of_id);
    if (id < 0) {
        printf("%s: invalid ID\n", __FUNCTION__);
        return -1;
    }

    if ( ioctl(fd, VIDIOC_G_CTRL, & control) == -1) {
        printf("%s failed\n", __FUNCTION__);
        return -1;
    } else {
        printf("%s successed\n", __FUNCTION__);
    }

    return 0;
}

static int testsuite_snapshot_stop()
{
           if (intrfcCtrl.stopSnapshot(camfd) == FALSE) {
            CDBG("main:%d  stop_snapshot failed!\n", __LINE__);
            return -1;
          }

    return 0;
}

int command_dispatcher(int command_id, char * args, char * result_str)
{
    int result = 0;
    int arg_param;
 
    switch(command_id)
    {
        case TS_PREVIEW_START:
            result = start_preview();
            break;

        case TS_PREVIEW_STOP:
            result = stop_preview();
            break;

	    case TS_VIDEO_START:
            result = start_video();
            break;

        case TS_VIDEO_STOP:
            result = stop_video();
            break;

        case TS_SNAPSHOT_YUV_PICTURE:
            result = take_picture(ACTION_TAKE_YUV_PICTURE);
            break;

        case TS_SNAPSHOT_JPEG_PICTURE:
            result = take_picture(ACTION_TAKE_JPEG_PICTURE);
            break;

        case TS_SNAPSHOT_RAW_PICTURE:
            result = take_raw_picture();
            break;

        case TS_SNAPSHOT_STOP:
            result = testsuite_snapshot_stop();
            break;

        case TS_SYSTEM_INIT:
            result = system_init();
            break;

        case TS_SYSTEM_DESTROY:
            result = system_destroy();
            break;

        case TS_PRINT_MAXZOOM:
            result = print_maxzoom();
            break;

        case TS_PRINT_ZOOMRATIOS:
            result = print_zoomratios();
            break;

        case TS_ZOOM_INCREASE:
            result = zoom_increase(1);
            break;

        case TS_ZOOM_DECREASE:
            result = zoom_decrease(1);
            break;

        case TS_ZOOM_STEP_INCREASE:
            result = zoom_increase(0);
            break;

        case TS_ZOOM_STEP_DECREASE:
            result = zoom_decrease(0);
            break;

        case TS_CONTRAST_INCREASE:
            result = increase_contrast();
            break;

        case TS_CONTRAST_DECREASE:
            result = decrease_contrast();
            break;

        case TS_SATURATION_INCREASE:
            result = increase_saturation();
            break;

        case TS_SATURATION_DECREASE:
            result = decrease_saturation();
            break;

        case TS_SPECIAL_EFFECT:
            result = SpecialEffect();
            break;

        case TS_BRIGHTNESS_INCREASE:
            result = increase_brightness();
            break;

        case TS_BRIGHTNESS_DECREASE:
            result = decrease_brightness();
            break;

        case TS_EV_INCREASE:
            result = increase_EV();
            break;

        case TS_EV_DECREASE:
            result = decrease_EV();
            break;

        case TS_ANTI_BANDING:
            result = set_antibanding();
            break;

        case TS_SET_WHITE_BALANCE:
            result = set_whitebalance();
            break;

        case TS_AEC_MODE:
            result = AEC_mode_change();
            break;

        case TS_ISO_INCREASE:
            result = increase_ISO();
            break;

        case TS_ISO_DECREASE:
            result = decrease_ISO();
            break;

        case TS_SHARPNESS_INCREASE:
            result = increase_sharpness();
            break;

        case TS_SHARPNESS_DECREASE:
            result = decrease_sharpness();
            break;

        case TS_SET_AUTO_FOCUS:
            result = set_auto_focus();
            break;

        case TS_SET_HJR:
            result = set_hjr();
            break;

        case TS_SET_LENS_SHADING:
            result = LensShading();
            break;

        case TS_SET_LED_MODE:
            result = LED_mode_change();
            break;

        case TS_GET_SHARPNESS_AF:
            result = set_sharpness_AF();
            break;

        case TS_SNAPSHOT_RESOLUTION:
            arg_param = atoi(args);
            result = snapshot_resolution(arg_param);
            break;

        case TS_PREVIEW_RESOLUTION:
            arg_param = atoi(args);
            result = preview_video_resolution (arg_param);
            break;

        case TS_MOTION_ISO:
            result = set_MotionIso();
            break;

        case TS_TOGGLE_HUE:
            result = toggle_hue();
            break;

        case TS_CANCEL_AUTO_FOCUS:
            result = cancel_af();
            break;

        case TS_GET_AF_STEP:
            result = get_af_step();
            break;

        case TS_SET_AF_STEP:
            result = set_af_step();
            break;

        case TS_ENABLE_AFD:
            result = enable_afd();
            break;

        case TEST_VIDIOC_G_FMT:
            result = msm_v4l2_vidioc_g_fmt();
            break;

        case TEST_VIDIOC_S_FMT:
            result = msm_v4l2_vidioc_s_fmt();
            break;

        case TEST_VIDIOC_CROPCAP:
            result = msm_v4l2_vidioc_cropcap();
            break;

        case TEST_VIDIOC_G_CROP:
            result = msm_v4l2_vidioc_g_crop();
            break;

        case TEST_VIDIOC_S_CROP:
            result = msm_v4l2_vidioc_s_crop();
            break;

        case TEST_VIDIOC_QUERYMENU:
            result = msm_v4l2_vidioc_querymenu(args);
            break;

        case TEST_VIDIOC_QUERYCTRL:
            result = msm_v4l2_vidioc_queryctrl(NULL);
            break;

        case TEST_VIDIOC_S_CTRL:
            result = msm_v4l2_vidioc_s_ctrl(args);
            break;

        case TEST_VIDIOC_G_CTRL:
            result = msm_v4l2_vidioc_g_ctrl(args);
            break;

        default:
            break;
    }
    return result;
}

