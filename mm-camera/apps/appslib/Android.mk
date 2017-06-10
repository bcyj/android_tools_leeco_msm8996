BUILD_LIBMMCAMERA:=1
ifeq ($(BUILD_LIBMMCAMERA),1)

CAM_LIB_PATH := $(call my-dir)

# --------------------------------------------------------------
# BUILD_CAM_FD is defined in mm-camera/Android.mk
# --------------------------------------------------------------
ifeq ($(BUILD_CAM_FD),1)
# ---------------------------------------------------------------
#     Install the prebuilt libraries
# ---------------------------------------------------------------

include $(CLEAR_VARS)
# ---------------------------------------------------------------
#  put face detetection library path & name here, for example:

#  LOCAL_PATH := $(CAM_LIB_PATH)/../../targets/tgtcommon/postproc/facial/engine/qct/lib
LOCAL_PATH := $(CAM_LIB_PATH)/../../server/frameproc/face_proc/engine/
#  LOCAL_PREBUILT_LIBS := libQctFd.so
#  LOCAL_PREBUILT_LIBS += libQctFt.so
# ---------------------------------------------------------------
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif


#
# libmmcamera-core.a
#

include $(CLEAR_VARS)

LOCAL_PATH:= $(CAM_LIB_PATH)

LOCAL_SRC_FILES:= \
	cam_frame_q.c \
	camframe.c \
        camaf_ctrl.c \
        jpeg_encoder.c \
	liveshot.c \
	mm_camera_interface.c \
	cam_display.c \
	cam_stats.c \
	snapshot.c \
	yv12conversionroutines.c

ifeq ($(MSM_VERSION),8x60)
LOCAL_SRC_FILES+= mpo_encoder.c
endif 

LOCAL_C_INCLUDES:= $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../common


ifeq ($(MSM_VERSION),7x2x)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg/inc
LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/mm-still/gemini
else
ifeq ($(MSM_VERSION),7x27A)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg/inc
LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/mm-still/gemini
else
LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/mm-still/jpeg
LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/mm-still/gemini
endif
endif
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../../hardware/qcom/camera

ifeq ($(MSM_VERSION),8x60)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/mpo/src
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/mpo/inc
endif


LOCAL_CFLAGS:= -Werror \
	$(mmcamera_debug_cflags) \
	$(mmcamera_debug_defines) \
	$(USE_SERVER_TREE) \
  -include camera_defs_i.h

ifeq ($(MSM_VERSION), 8974)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else ifeq ($(MSM_VERSION), 8226)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else ifeq ($(MSM_VERSION), 8610)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else ifeq ($(MSM_VERSION), 8960)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CAMERA_HEAP_ID
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

ifeq ($(MSM_VERSION),7x2x)
  LOCAL_CFLAGS += -D_TARGET_7x2x_
endif

ifeq ($(MSM_VERSION),7x27A)
  LOCAL_CFLAGS += -D_TARGET_7x27A_
endif

ifeq ($(MSM_VERSION),8x60)
  LOCAL_CFLAGS += -D_TARGET_8660_
endif
ifeq ($(ARCH_ARM_HAVE_NEON),true)
    LOCAL_CFLAGS += -D__ARM_HAVE_NEON
endif
LOCAL_MODULE:=libmmcamera-core
LOCAL_MODULE_TAGS := optional

include $(LOCAL_PATH)/../../local_additional_dependency.mk
include $(LOCAL_PATH)/../../malloc_wrap_static_library.mk

#
# libqcamera.so
#

include $(CLEAR_VARS)

LOCAL_CFLAGS:= -Werror \
    $(mmcamera_debug_cflags) \
    $(mmcamera_debug_defines)

LOCAL_SRC_FILES:= override_malloc.c
ifeq ($(FEATURE_WAVELET_DENOISE),true)
  ifeq ($(MSM_VERSION),8960)
    LOCAL_WHOLE_STATIC_LIBRARIES:= libmmcamera-core libmmcamera_target
  else ifeq ($(BUILD_SERVER), true)
     ifeq ($(MSM_VERSION),7x30)
        LOCAL_WHOLE_STATIC_LIBRARIES:= libmmcamera-core libmmcamera_target
     else ifeq ($(MSM_VERSION),7x27A)
        LOCAL_WHOLE_STATIC_LIBRARIES:= libmmcamera-core libmmcamera_target
     else ifeq ($(MSM_VERSION),8x60)
        LOCAL_WHOLE_STATIC_LIBRARIES:= libmmcamera-core libmmcamera_target
     endif
  else
    LOCAL_WHOLE_STATIC_LIBRARIES:= libmmcamera-core libmmcamera_assembly libmmcamera_target
  endif
else
  LOCAL_WHOLE_STATIC_LIBRARIES:= libmmcamera-core libmmcamera_target
endif
ifeq ($(MSM_VERSION),8960)
  LOCAL_SHARED_LIBRARIES:= libdl libcutils $(mmcamera_debug_libs) libmmjpeg libgemini
  ifeq ($(FEATURE_GYRO),true)
    LOCAL_SHARED_LIBRARIES+=libsensor1
  endif
else ifeq ($(MSM_VERSION),8974)
  LOCAL_SHARED_LIBRARIES:= libdl libcutils $(mmcamera_debug_libs) libmmjpeg
   ifeq ($(FEATURE_GYRO),true)
    LOCAL_SHARED_LIBRARIES+=libsensor1
  endif
else ifeq ($(MSM_VERSION),8226)
  LOCAL_SHARED_LIBRARIES:= libdl libcutils $(mmcamera_debug_libs) libmmjpeg
   ifeq ($(FEATURE_GYRO),true)
    LOCAL_SHARED_LIBRARIES+=libsensor1
  endif
else ifeq ($(MSM_VERSION),8610)
  LOCAL_SHARED_LIBRARIES:= libdl libcutils $(mmcamera_debug_libs) libmmjpeg
   ifeq ($(FEATURE_GYRO),true)
    LOCAL_SHARED_LIBRARIES+=libsensor1
  endif
else
  ifeq ($(BUILD_SERVER), true)
    LOCAL_SHARED_LIBRARIES:= libdl libcutils $(mmcamera_debug_libs) libmmjpeg libgemini
  else
    LOCAL_SHARED_LIBRARIES:= libdl libcutils $(mmcamera_debug_libs) libmmjpeg libmmipl libgemini
  endif
endif
ifeq ($(MSM_VERSION),8x60)
LOCAL_SHARED_LIBRARIES+= libmmmpo libgemini
endif

ifeq ($(BUILD_CAM_FD),1)
# ---------------------------------------------------------------
#  put face detetection library path here, such as

#  LOCAL_SHARED_LIBRARIES += libQctFd libQctFt
# ---------------------------------------------------------------
endif

ifeq ($(MSM_VERSION),8x60)
  LOCAL_SHARED_LIBRARIES += libC2D2
  LOCAL_SHARED_LIBRARIES += libmmstereo
endif

LOCAL_PRELINK_MODULE:= true
LOCAL_MODULE:= liboemcamera
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

#
# mm-qcamera-daemon
#

include $(CLEAR_VARS)

LOCAL_CFLAGS:= \
 $(mmcamera_debug_defines) \
 $(mmcamera_debug_cflags) \
 -include camera_defs_i.h

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

LOCAL_SRC_FILES:= override_malloc.c \
 ../../common/stacktrace.c \
 camdaemon.c

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
 $(LOCAL_PATH)/../appslib \
 $(LOCAL_PATH)/../../common \
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../../hardware/qcom/camera

ifeq ($(MSM_VERSION),7x2x)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg/inc
else
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg2
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/jpeg2/inc
endif

ifeq ($(MSM_VERSION),8x60)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/mpo/src
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../mm-still/mpo/inc
endif

include $(LOCAL_PATH)/../../local_additional_dependency.mk

LOCAL_STATIC_LIBRARIES:= libmmcamera_target
ifeq ($(MSM_VERSION),8960)
  LOCAL_SHARED_LIBRARIES:= libcutils $(mmcamera_debug_libs) liboemcamera
else
  ifeq ($(BUILD_SERVER), true)
    LOCAL_SHARED_LIBRARIES:= libcutils $(mmcamera_debug_libs) liboemcamera
  else
    LOCAL_SHARED_LIBRARIES:= libcutils $(mmcamera_debug_libs) libmmipl liboemcamera
  endif
endif
LOCAL_MODULE:= mm-qcamera-daemon
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif
