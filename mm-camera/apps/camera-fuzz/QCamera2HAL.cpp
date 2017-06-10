/* ========================================================================= *
   Purpose:  Shared object library used for fuzzing Camera HAL layer APIs

           -------------------------------------------------------
        Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
               Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */
#define LOG_TAG "QCamera2HAL"

#include <sys/stat.h>
#include <binder/IMemory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/RefBase.h>
#include <hardware/hardware.h>
#include <dlfcn.h>
#include "QCamera2HAL.h"

static hw_module_t * module;
struct hw_device_t hw_dev;
struct hw_device_t * hw_device = & hw_dev;
static int error;
static void *libptr;
camera_memory_t * handle;
char * param_string = NULL; //temporary define as global.

int (*fuzz_get_number_of_cameras)();
int (*fuzz_camera_device_open)(const struct hw_module_t* module, const char* id,
      struct hw_device_t** device);
int (*fuzz_close_camera_device)( hw_device_t *);
int (*fuzz_start_preview)( struct camera_device *);
int (*fuzz_stop_preview)( struct camera_device *);
int (*fuzz_set_CallBacks) (struct camera_device *,
          camera_notify_callback notify_cb,
          camera_data_callback data_cb,
          camera_data_timestamp_callback data_cb_timestamp,
          camera_request_memory get_memory,
          void *user);
int (*fuzz_set_preview_window)(struct camera_device *,
          struct preview_stream_ops *window);
int (*fuzz_enable_msg_type) (struct camera_device *, int32_t msg_type);
int (*fuzz_disable_msg_type) (struct camera_device *, int32_t msg_type);
int (*fuzz_msg_type_enabled) (struct camera_device *, int32_t msg_type);
int (*fuzz_preview_enabled) (struct camera_device *);
int (*fuzz_store_meta_data_in_buffers)(struct camera_device *, int enable);
int (*fuzz_start_recording)(struct camera_device *);
int (*fuzz_stop_recording)(struct camera_device *);
int (*fuzz_recording_enabled)(struct camera_device *);
int (*fuzz_release_recording_frame)(struct camera_device *,
                const void *opaque);
int (*fuzz_auto_focus)(struct camera_device *);
int (*fuzz_cancel_auto_focus)(struct camera_device *);
int (*fuzz_take_picture)(struct camera_device *);
int (*fuzz_cancel_picture)(struct camera_device *);
int (*fuzz_set_parameters)(struct camera_device *, const char *parms);
char* (*fuzz_get_parameters)(struct camera_device *);
int (*fuzz_put_parameters)(struct camera_device *, char *);
int (*fuzz_send_command)(struct camera_device *,
            int32_t cmd, int32_t arg1, int32_t arg2);
int (*fuzz_release)(struct camera_device *);


void __notify_cb(int32_t msg_type, int32_t ext1,
                         int32_t ext2, void *user);
void __data_cb(int32_t msg_type,
                         const camera_memory_t *data, unsigned int index,
                         camera_frame_metadata_t *metadata,
                         void *user);
void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                         const camera_memory_t *data, unsigned index,
                         void *user);
camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
                         void *user __attribute__((unused)));

void * mapfd(int fd, size_t size);

int Fuzzer_HAL_init() {
  libptr = dlopen("hw/camera.msm8974.so", RTLD_NOW);
  if (!libptr) {
    ALOGE("%s: Error loading library\n", __func__);
    error = TRUE;
    return -1;
  }
  *(void **)&(fuzz_get_number_of_cameras) = dlsym(libptr, "get_number_of_cameras");
  *(void **)&(fuzz_camera_device_open) = dlsym(libptr, "camera_device_open");
  *(void **)&(fuzz_close_camera_device) = dlsym(libptr, "close_camera_device");
  *(void **)&(fuzz_start_preview) = dlsym(libptr, "start_preview");
  *(void **)&(fuzz_stop_preview) = dlsym(libptr, "stop_preview");
  *(void **)&(fuzz_set_CallBacks) = dlsym(libptr, "set_CallBacks");
  *(void **)&(fuzz_set_preview_window) = dlsym(libptr, "set_preview_window");
  *(void **)&(fuzz_enable_msg_type) = dlsym(libptr, "enable_msg_type");
  *(void **)&(fuzz_disable_msg_type) = dlsym(libptr, "disable_msg_type");
  *(void **)&(fuzz_msg_type_enabled) = dlsym(libptr, "msg_type_enabled");
  *(void **)&(fuzz_preview_enabled) = dlsym(libptr, "preview_enabled");
  *(void **)&(fuzz_store_meta_data_in_buffers) = dlsym(libptr, "store_meta_data_in_buffers");
  *(void **)&(fuzz_start_recording) = dlsym(libptr, "start_recording");
  *(void **)&(fuzz_stop_recording) = dlsym(libptr, "stop_recording");
  *(void **)&(fuzz_recording_enabled) = dlsym(libptr, "recording_enabled");
  *(void **)&(fuzz_release_recording_frame) = dlsym(libptr, "release_recording_frame");
  *(void **)&(fuzz_auto_focus) = dlsym(libptr, "auto_focus");
  *(void **)&(fuzz_cancel_auto_focus) = dlsym(libptr, "cancel_auto_focus");
  *(void **)&(fuzz_take_picture) = dlsym(libptr, "take_picture");
  *(void **)&(fuzz_cancel_picture) = dlsym(libptr, "cancel_picture");
  *(void **)&(fuzz_set_parameters) = dlsym(libptr, "set_parameters");
  *(void **)&(fuzz_get_parameters) = dlsym(libptr, "get_parameters");
  *(void **)&(fuzz_put_parameters) = dlsym(libptr, "put_parameters");
  *(void **)&(fuzz_send_command) = dlsym(libptr, "send_command");
  *(void **)&(fuzz_release) = dlsym(libptr, "release");

  if (!fuzz_get_number_of_cameras || !fuzz_camera_device_open ||
      !fuzz_close_camera_device || !fuzz_start_preview ||
      !fuzz_stop_preview || !fuzz_set_CallBacks ||
      !fuzz_set_preview_window || !fuzz_enable_msg_type ||
      !fuzz_disable_msg_type || !fuzz_msg_type_enabled ||
      !fuzz_preview_enabled || !fuzz_store_meta_data_in_buffers ||
      !fuzz_start_recording || !fuzz_stop_recording ||
      !fuzz_recording_enabled || !fuzz_release_recording_frame ||
      !fuzz_auto_focus  || !fuzz_cancel_auto_focus ||
      !fuzz_take_picture  || !fuzz_cancel_picture ||
      !fuzz_set_parameters  || !fuzz_get_parameters ||
      !fuzz_put_parameters  || !fuzz_send_command ||
      !fuzz_release ) {

      ALOGE("%s: Error loading symbols\n", __func__);
      error = TRUE;
      return -1;
  }
  int num_cam = fuzz_get_number_of_cameras();
  ALOGV("%s: %d\n", __func__, num_cam);
  return 0;
}

int Fuzzer_HAL_deinit() {
  ALOGV("%s \n", __func__);
  if (libptr) {
      dlclose(libptr);
  }
  return 0;
}

int camera_device_open(const char* id) {
  ALOGV("In %s \n", __func__);
  if (error) {
    ALOGE("In %s error\n", __func__);
    return -1;
    }

  const char * Id = id;
  if (hw_get_module(CAMERA_HARDWARE_MODULE_ID,
              (const hw_module_t **)&module) < 0) {
      ALOGE("Could not load camera HAL module");
  }
  //module->name = "Qcamera";
  fuzz_camera_device_open(module, Id, &hw_device);
  ALOGV("X %s \n", __func__);
  return 0;
}

int close_camera_device() {
  ALOGV("In %sn", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_close_camera_device( hw_device);
  ALOGV("X %s ret = %d\n", __func__, ret);
  return 0;
}

int start_preview() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_start_preview((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return 0;
}

int stop_preview() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  fuzz_stop_preview((camera_device *)hw_device);
  ALOGV("X %s \n", __func__);
  return 0;
}

int set_callbacks(int cam_id) {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_set_CallBacks((struct camera_device *)hw_device,
                                   __notify_cb,
                                   __data_cb,
                                   __data_cb_timestamp,
                                   __get_memory,
                                   (void *)cam_id);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int set_preview_window() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_set_preview_window((struct camera_device *)hw_device,
          /*struct preview_stream_ops *window*/ NULL);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int enable_msg_type(int msg_type) {
  ALOGV("E %s msg_type %d\n", __func__,msg_type);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_enable_msg_type((struct camera_device *)hw_device, msg_type);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int disable_msg_type(int msg_type) {
  ALOGV("E %s msg_type %d\n", __func__,msg_type);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_disable_msg_type((struct camera_device *)hw_device, msg_type);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int msg_type_enabled(int msg_type) {
  ALOGV("E %s msg_type %d\n", __func__,msg_type);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_msg_type_enabled((struct camera_device *)hw_device, msg_type);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int preview_enabled() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_preview_enabled((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__, ret);
  return ret;
}

int store_meta_data_in_buffers(int enable) {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_store_meta_data_in_buffers((struct camera_device *)hw_device,enable);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int start_recording() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_start_recording((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int stop_recording() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_stop_recording((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int recording_enabled() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_recording_enabled((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int release_recording_frame(const void *opaque) {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_release_recording_frame((struct camera_device *)hw_device,opaque);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int auto_focus() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_auto_focus((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int cancel_auto_focus() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_cancel_auto_focus((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int take_picture() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_take_picture((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int cancel_picture() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_cancel_picture((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int set_parameters(const char *parms, int use_default) {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  const char * str = "ae-bracket-hdr=Off;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-distances=1.229898,2.159076,8.830284;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;lensshade-values=enable,disable;luma-adaptation=3;max-contrast=10;max-exposure-compensation=12;max-num-detected-faces-hw=2;max-num-focus-areas=1;max-num-metering-areas=1;max-saturation=10;max-sharpness=30;max-zoom=59;mce=enable;mce-values=enable,disable;metering-areas=(0, 0, 0, 0, 0);min-exposure-compensation=-12;no-display-mode=0;num-snaps-per-shutter=1;overlay-format=265;picture-format=jpeg;picture-format-values=jpeg,raw;picture-size=1920x1088;picture-size-values=4000x3000,3200x2400,2592x1944,2048x1536,1920x1088,1600x1200,1;power-mode=Normal_Power;power-mode-supported=true;preferred-preview-size-for-video=1920x1088;preview-format=yuv420sp;preview-format-values=yuv420sp,yuv420sp-adreno,yuv420p,yuv420p,nv12;preview-fps-range=5000,121000;preview-fps-range-values=(5000,121000);preview-frame-rate=121;preview-frame-rate-mode=frame-rate-auto;preview-frame-rate-modes=frame-rate-auto,frame-rate-fixed;preview-frame-rate-values=5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121;preview-size=1920x1088;preview-size-values=1920x1088,1280x720,800x480,768x432,720x480,640x480,576x432,480x320,384x288,352x288,320x240,240x160,176x144;redeye-reduction=disable;redeye-reduction-values=enable,disable;saturation=5;scene-detect=off;scene-detect,landscape,night,night-portrait,theatre,beach,snow,sunset,steadyphoto,fireworks,sports,party,candlelight,backlight,flowers,AR;selectable-zone-af=auto;selectable-zone-af-values=auto,spot-metering,center-weighted,frame-average;sharpness=10;skinToneEnhancement=0;skinToneEnhancement-values=enable,disable;strtextures=OFF;touch-af-aec=touch-off;touch-af-aec-values=touch-off,touch-on;touchAfAec-dx=100;touchAfAec-dy=100;vertical-view-angle=42.5;video-frame-format=yuv420sp;video-hfr=off;video-hfr-values=off,60,90,120;video-size=1920x1088;video-size-values=1920x1088,1280x720,800x480,720x480,640x480,480x320,352x288,320x240,176x144;video-snapshot-supported=true;video-zoom-support=true;whitebalance=incandescent;whitebalance-values=auto,incandescent,fluorescent,daylight,cloudy-daylight;zoom=0;zoom-ratios=100,102,104,107,109,112,114,117,120,123,125,128,131,135,138,141,144,148,151,155,158,162,166,170,174,178,182,186,190,195,200,204,209,214,219,224,229,235,240,246,251,257,263,270,276,282,289,296,303,310,317,324,332,340,348,356,364,ff;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-detection-values=;flash-mode=off;flash-mode-values=off,auto,on,torch;focal-length=4.6;focus-areas=(0, 0, 0, 0, 0);focus-distances=1.231085,2.162743,8.892049;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;";
  if( use_default == 0) {
    if(strcmp(parms,"NULL")) {
        ALOGV("String passed from test xml is set\n"); str = parms;
    } else {
        ALOGV("String passed from test xml is NULL\n");
    }
  } else if (use_default == 2 && NULL != param_string) {
    ALOGV("Keeping same parameters as previous\n");
    str = param_string;
  } else if (use_default == 3) {
    ALOGV(" HDR\n");
    str = "ae-bracket-hdr=HDR;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-distances=1.229898,2.159076,8.830284;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;lensshade-values=enable,disable;luma-adaptation=3;max-contrast=10;max-exposure-compensation=12;max-num-detected-faces-hw=2;max-num-focus-areas=1;max-num-metering-areas=1;max-saturation=10;max-sharpness=30;max-zoom=59;mce=enable;mce-values=enable,disable;metering-areas=(0, 0, 0, 0, 0);min-exposure-compensation=-12;no-display-mode=0;num-snaps-per-shutter=1;overlay-format=265;picture-format=jpeg;picture-format-values=jpeg,raw;picture-size=1920x1088;picture-size-values=4000x3000,3200x2400,2592x1944,2048x1536,1920x1088,1600x1200,1;power-mode=Normal_Power;power-mode-supported=true;preferred-preview-size-for-video=1920x1088;preview-format=yuv420sp;preview-format-values=yuv420sp,yuv420sp-adreno,yuv420p,yuv420p,nv12;preview-fps-range=5000,121000;preview-fps-range-values=(5000,121000);preview-frame-rate=121;preview-frame-rate-mode=frame-rate-auto;preview-frame-rate-modes=frame-rate-auto,frame-rate-fixed;preview-frame-rate-values=5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121;preview-size=1920x1088;preview-size-values=1920x1088,1280x720,800x480,768x432,720x480,640x480,576x432,480x320,384x288,352x288,320x240,240x160,176x144;redeye-reduction=disable;redeye-reduction-values=enable,disable;saturation=5;scene-detect=off;scene-detect,landscape,night,night-portrait,theatre,beach,snow,sunset,steadyphoto,fireworks,sports,party,candlelight,backlight,flowers,AR;selectable-zone-af=auto;selectable-zone-af-values=auto,spot-metering,center-weighted,frame-average;sharpness=10;skinToneEnhancement=0;skinToneEnhancement-values=enable,disable;strtextures=OFF;touch-af-aec=touch-off;touch-af-aec-values=touch-off,touch-on;touchAfAec-dx=100;touchAfAec-dy=100;vertical-view-angle=42.5;video-frame-format=yuv420sp;video-hfr=off;video-hfr-values=off,60,90,120;video-size=1920x1088;video-size-values=1920x1088,1280x720,800x480,720x480,640x480,480x320,352x288,320x240,176x144;video-snapshot-supported=true;video-zoom-support=true;whitebalance=incandescent;whitebalance-values=auto,incandescent,fluorescent,daylight,cloudy-daylight;zoom=0;zoom-ratios=100,102,104,107,109,112,114,117,120,123,125,128,131,135,138,141,144,148,151,155,158,162,166,170,174,178,182,186,190,195,200,204,209,214,219,224,229,235,240,246,251,257,263,270,276,282,289,296,303,310,317,324,332,340,348,356,364,ff;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-detection-values=;flash-mode=off;flash-mode-values=off,auto,on,torch;focal-length=4.6;focus-areas=(0, 0, 0, 0, 0);focus-distances=1.231085,2.162743,8.892049;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;";
  } else if (use_default == 4) {
    ALOGV("Wavelet denoise\n");
    str = "ae-bracket-hdr=Off;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-on;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-distances=1.229898,2.159076,8.830284;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;lensshade-values=enable,disable;luma-adaptation=3;max-contrast=10;max-exposure-compensation=12;max-num-detected-faces-hw=2;max-num-focus-areas=1;max-num-metering-areas=1;max-saturation=10;max-sharpness=30;max-zoom=59;mce=enable;mce-values=enable,disable;metering-areas=(0, 0, 0, 0, 0);min-exposure-compensation=-12;no-display-mode=0;num-snaps-per-shutter=1;overlay-format=265;picture-format=jpeg;picture-format-values=jpeg,raw;picture-size=1920x1088;picture-size-values=4000x3000,3200x2400,2592x1944,2048x1536,1920x1088,1600x1200,1;power-mode=Normal_Power;power-mode-supported=true;preferred-preview-size-for-video=1920x1088;preview-format=yuv420sp;preview-format-values=yuv420sp,yuv420sp-adreno,yuv420p,yuv420p,nv12;preview-fps-range=5000,121000;preview-fps-range-values=(5000,121000);preview-frame-rate=121;preview-frame-rate-mode=frame-rate-auto;preview-frame-rate-modes=frame-rate-auto,frame-rate-fixed;preview-frame-rate-values=5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121;preview-size=1920x1088;preview-size-values=1920x1088,1280x720,800x480,768x432,720x480,640x480,576x432,480x320,384x288,352x288,320x240,240x160,176x144;redeye-reduction=disable;redeye-reduction-values=enable,disable;saturation=5;scene-detect=off;scene-detect,landscape,night,night-portrait,theatre,beach,snow,sunset,steadyphoto,fireworks,sports,party,candlelight,backlight,flowers,AR;selectable-zone-af=auto;selectable-zone-af-values=auto,spot-metering,center-weighted,frame-average;sharpness=10;skinToneEnhancement=0;skinToneEnhancement-values=enable,disable;strtextures=OFF;touch-af-aec=touch-off;touch-af-aec-values=touch-off,touch-on;touchAfAec-dx=100;touchAfAec-dy=100;vertical-view-angle=42.5;video-frame-format=yuv420sp;video-hfr=off;video-hfr-values=off,60,90,120;video-size=1920x1088;video-size-values=1920x1088,1280x720,800x480,720x480,640x480,480x320,352x288,320x240,176x144;video-snapshot-supported=true;video-zoom-support=true;whitebalance=incandescent;whitebalance-values=auto,incandescent,fluorescent,daylight,cloudy-daylight;zoom=0;zoom-ratios=100,102,104,107,109,112,114,117,120,123,125,128,131,135,138,141,144,148,151,155,158,162,166,170,174,178,182,186,190,195,200,204,209,214,219,224,229,235,240,246,251,257,263,270,276,282,289,296,303,310,317,324,332,340,348,356,364,ff;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-on;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-detection-values=;flash-mode=off;flash-mode-values=off,auto,on,torch;focal-length=4.6;focus-areas=(0, 0, 0, 0, 0);focus-distances=1.231085,2.162743,8.892049;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;";
  } else if (use_default == 5) {
    ALOGV("flash on ZSL\n");
    str = "ae-bracket-hdr=Off;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=1;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-distances=1.229898,2.159076,8.830284;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;lensshade-values=enable,disable;luma-adaptation=3;max-contrast=10;max-exposure-compensation=12;max-num-detected-faces-hw=2;max-num-focus-areas=1;max-num-metering-areas=1;max-saturation=10;max-sharpness=30;max-zoom=59;mce=enable;mce-values=enable,disable;metering-areas=(0, 0, 0, 0, 0);min-exposure-compensation=-12;no-display-mode=0;num-snaps-per-shutter=1;overlay-format=265;picture-format=jpeg;picture-format-values=jpeg,raw;picture-size=1920x1088;picture-size-values=4000x3000,3200x2400,2592x1944,2048x1536,1920x1088,1600x1200,1;power-mode=Normal_Power;power-mode-supported=true;preferred-preview-size-for-video=1920x1088;preview-format=yuv420sp;preview-format-values=yuv420sp,yuv420sp-adreno,yuv420p,yuv420p,nv12;preview-fps-range=5000,121000;preview-fps-range-values=(5000,121000);preview-frame-rate=121;preview-frame-rate-mode=frame-rate-auto;preview-frame-rate-modes=frame-rate-auto,frame-rate-fixed;preview-frame-rate-values=5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121;preview-size=1920x1088;preview-size-values=1920x1088,1280x720,800x480,768x432,720x480,640x480,576x432,480x320,384x288,352x288,320x240,240x160,176x144;redeye-reduction=disable;redeye-reduction-values=enable,disable;saturation=5;scene-detect=off;scene-detect,landscape,night,night-portrait,theatre,beach,snow,sunset,steadyphoto,fireworks,sports,party,candlelight,backlight,flowers,AR;selectable-zone-af=auto;selectable-zone-af-values=auto,spot-metering,center-weighted,frame-average;sharpness=10;skinToneEnhancement=0;skinToneEnhancement-values=enable,disable;strtextures=OFF;touch-af-aec=touch-off;touch-af-aec-values=touch-off,touch-on;touchAfAec-dx=100;touchAfAec-dy=100;vertical-view-angle=42.5;video-frame-format=yuv420sp;video-hfr=off;video-hfr-values=off,60,90,120;video-size=1920x1088;video-size-values=1920x1088,1280x720,800x480,720x480,640x480,480x320,352x288,320x240,176x144;video-snapshot-supported=true;video-zoom-support=true;whitebalance=incandescent;whitebalance-values=auto,incandescent,fluorescent,daylight,cloudy-daylight;zoom=0;zoom-ratios=100,102,104,107,109,112,114,117,120,123,125,128,131,135,138,141,144,148,151,155,158,162,166,170,174,178,182,186,190,195,200,204,209,214,219,224,229,235,240,246,251,257,263,270,276,282,289,296,303,310,317,324,332,340,348,356,364,ff;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=1;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-detection-values=;flash-mode=on;flash-mode-values=off,auto,on,torch;focal-length=4.6;focus-areas=(0, 0, 0, 0, 0);focus-distances=1.231085,2.162743,8.892049;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;";
  } else {
    ALOGV("Using default params with flash ON\n");
    str = "ae-bracket-hdr=Off;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-distances=1.229898,2.159076,8.830284;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;lensshade-values=enable,disable;luma-adaptation=3;max-contrast=10;max-exposure-compensation=12;max-num-detected-faces-hw=2;max-num-focus-areas=1;max-num-metering-areas=1;max-saturation=10;max-sharpness=30;max-zoom=59;mce=enable;mce-values=enable,disable;metering-areas=(0, 0, 0, 0, 0);min-exposure-compensation=-12;no-display-mode=0;num-snaps-per-shutter=1;overlay-format=265;picture-format=jpeg;picture-format-values=jpeg,raw;picture-size=1920x1088;picture-size-values=4000x3000,3200x2400,2592x1944,2048x1536,1920x1088,1600x1200,1;power-mode=Normal_Power;power-mode-supported=true;preferred-preview-size-for-video=1920x1088;preview-format=yuv420sp;preview-format-values=yuv420sp,yuv420sp-adreno,yuv420p,yuv420p,nv12;preview-fps-range=5000,121000;preview-fps-range-values=(5000,121000);preview-frame-rate=121;preview-frame-rate-mode=frame-rate-auto;preview-frame-rate-modes=frame-rate-auto,frame-rate-fixed;preview-frame-rate-values=5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121;preview-size=640x480;preview-size-values=1920x1088,1280x720,800x480,768x432,720x480,640x480,576x432,480x320,384x288,352x288,320x240,240x160,176x144;redeye-reduction=disable;redeye-reduction-values=enable,disable;saturation=5;scene-detect=off;scene-detect,landscape,night,night-portrait,theatre,beach,snow,sunset,steadyphoto,fireworks,sports,party,candlelight,backlight,flowers,AR;selectable-zone-af=auto;selectable-zone-af-values=auto,spot-metering,center-weighted,frame-average;sharpness=10;skinToneEnhancement=0;skinToneEnhancement-values=enable,disable;strtextures=OFF;touch-af-aec=touch-off;touch-af-aec-values=touch-off,touch-on;touchAfAec-dx=100;touchAfAec-dy=100;vertical-view-angle=42.5;video-frame-format=yuv420sp;video-hfr=off;video-hfr-values=off,60,90,120;video-size=1920x1088;video-size-values=1920x1088,1280x720,800x480,720x480,640x480,480x320,352x288,320x240,176x144;video-snapshot-supported=true;video-zoom-support=true;whitebalance=incandescent;whitebalance-values=auto,incandescent,fluorescent,daylight,cloudy-daylight;zoom=0;zoom-ratios=100,102,104,107,109,112,114,117,120,123,125,128,131,135,138,141,144,148,151,155,158,162,166,170,174,178,182,186,190,195,200,204,209,214,219,224,229,235,240,246,251,257,263,270,276,282,289,296,303,310,317,324,332,340,348,356,364,ff;ae-bracket-hdr-values=Off,HDR,AE-Bracket;antibanding=off;antibanding-values=off,50hz,60hz,auto;auto-exposure=frame-average;auto-exposure-lock=false;auto-exposure-lock-supported=true;auto-exposure-values=frame-average,center-weighted,spot-metering;auto-whitebalance-lock=false;auto-whitebalance-lock-supported=true;camera-mode=0;camera-mode-values=0,1;capture-burst-captures-values=2;capture-burst-exposures=;capture-burst-exposures-values=-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12;capture-burst-interval=1;capture-burst-interval-max=10;capture-burst-interval-min=1;capture-burst-interval-supported=true;capture-burst-retroactive=0;capture-burst-retroactive-max=2;contrast=5;denoise=denoise-off;denoise-values=denoise-off,denoise-on;effect=none;effect-values=none,mono,negative,solarize,sepia,posterize,whiteboard,blackboard,aqua,emboss,sketch,neon;exposure-compensation=0;exposure-compensation-step=0.166667;face-detection=off;face-detection-values=;flash-mode=on;flash-mode-values=off,auto,on,torch;focal-length=4.6;focus-areas=(0, 0, 0, 0, 0);focus-distances=1.231085,2.162743,8.892049;focus-mode=auto;focus-mode-values=auto,infinity,normal,macro,continuous-picture,continuous-video;hfr-size-values=800x480,640x480;histogram=disable;histogram-values=enable,disable;horizontal-view-angle=54.8;iso=auto;iso-values=auto,ISO_HJR,ISO100,ISO200,ISO400,ISO800,ISO1600;jpeg-quality=85;jpeg-thumbnail-height=384;jpeg-thumbnail-quality=90;jpeg-thumbnail-size-values=512x288,480x288,432x288,512x384,352x288,0x0;jpeg-thumbnail-width=512;lensshade=enable;";
  }

  ret = fuzz_set_parameters((struct camera_device *)hw_device,str);
  ALOGV("X %s ret = %d \n", __func__, ret);
  return ret;
}

char* get_parameters() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return NULL;
  }
  //char * ret = NULL;
  param_string = fuzz_get_parameters((struct camera_device *)hw_device);
  ALOGV("X %s \n Parameters string : %s \n", __func__, param_string);
  ALOGV("%s \n",(param_string+1096));
  ALOGV("%s \n",(param_string+2192));
  ALOGV("%s \n",(param_string+3288));
  ALOGV("%s \n",(param_string+4384));
  ALOGV("%s \n",(param_string+5480));
  ALOGV("%s \n",(param_string+6576));
  return param_string;
}

int put_parameters(char *parms) {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_put_parameters((struct camera_device *)hw_device, param_string); //temp using global
  ALOGV("X %s ret = %d \n", __func__, ret);
  return ret;
}

int send_command(int cmd, int arg1, int arg2) {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_send_command((struct camera_device *)hw_device, cmd, arg1, arg2);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

int release() {
  ALOGV("E %s \n", __func__);
  if (error) {
      ALOGE("In %s error\n", __func__);
      return -1;
  }
  int ret = -1;
  ret = fuzz_release((struct camera_device *)hw_device);
  ALOGV("X %s ret= %d\n", __func__,ret);
  return ret;
}

  /* Internal Helper Functions*/
void __notify_cb(int32_t msg_type, int32_t ext1,
                                       int32_t ext2, void *user) {
  ALOGV("%s\n", __FUNCTION__);
  ALOGV("msg_type %d, ext1 %d ext2 %d \n",  msg_type, ext1, ext2);
}

void __data_cb(int32_t msg_type, const camera_memory_t *data,
                          unsigned int index,
                          camera_frame_metadata_t *metadata,
                          void *user) {
  ALOGV("%s\n", __FUNCTION__);
  ALOGV("msg_type %d, data %p, metadata %p index %d \n",
            msg_type, (unsigned int *)data, (unsigned int *)metadata, index);
  }

void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                                                 const camera_memory_t *data, unsigned int index,
                                                 void *user) {
  ALOGV("%s\n", __FUNCTION__);
  ALOGV("timestamp %ld msg_type %d, data %p,  index %d \n",
                          (long) timestamp, msg_type, (unsigned int *)data, index);
  }

static void __put_memory(camera_memory_t *data) {

  ALOGV("E %s data :%p \n", __FUNCTION__, (unsigned int *)data);
  if (!data)
        return;
  //free(data->data);
  free(data);
  data = NULL;
  ALOGV("X %s\n", __FUNCTION__);
  }

camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
                                       void *user __attribute__((unused))) {
  ALOGV("%s fd:%d buffsize: %d num_bufs %d\n", __FUNCTION__, fd, (int)buf_size, num_bufs);
  handle = (camera_memory_t *)malloc (sizeof(camera_memory_t));
  //handle->data = (void *)malloc(buf_size * num_bufs);
  const size_t pagesize = getpagesize();
  ALOGV("pagesize: %d\n",(int)pagesize);
  buf_size = ((buf_size + pagesize-1) & ~(pagesize-1));
  ALOGV("new buf_size: %d caling mapfd\n", buf_size);
  handle->data = mapfd(dup(fd), buf_size);
  ALOGV("after mapfd: %p\n", (int *)handle->data);
  handle->size = buf_size * num_bufs;
  handle->handle = NULL;
  handle->release = __put_memory;
  ALOGV("%s handle :%p \n", __FUNCTION__,(unsigned int *) handle);
  return handle;
  }

void * mapfd(int fd, size_t size) {
  ALOGV("E %s fd %d size %d\n", __FUNCTION__, fd, (int)size);
  int offset = 0;
  void* base = NULL;
  if (size == 0) {
    // try to figure out the size automatically
    #ifdef HAVE_ANDROID_OS
    // first try the PMEM ioctl
    ALOGV("first try the PMEM ioctl\n");
    pmem_region reg;
    int err = ioctl(fd, PMEM_GET_TOTAL_SIZE, &reg);
    if (err == 0)
      size = reg.len;
    #endif
    if (size == 0) { // try fstat
      struct stat sb;
      if (fstat(fd, &sb) == 0)
          size = sb.st_size;
    }
    // if it didn't work, let mmap() fail.
  }
    ALOGV("calling mmap\n");
    base = (uint8_t*)mmap(0, size,
                            PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
    if (base == MAP_FAILED) {
      ALOGV("mmap(fd=%d, size=%u) failed",
              fd, uint32_t(size));
      close(fd);
      return NULL;
    }
  ALOGV("mmap success base %p\n", (int *) base);
  return base;
}
