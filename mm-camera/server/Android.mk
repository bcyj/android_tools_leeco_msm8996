#************* libmmcamera_server_assembly Start ************#
LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

ifeq ($(FEATURE_FACE_PROC),true)
  include $(CLEAR_VARS)
  LOCAL_PATH := $(LOCAL_DIR_PATH)/frameproc/face_proc/engine/
  LOCAL_MODULE := libmmcamera_faceproc
  LOCAL_SRC_FILES := libmmcamera_faceproc.so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_TAGS := optional eng
  LOCAL_MODULE_OWNER := qcom 
  LOCAL_32_BIT_ONLY := true
  LOCAL_PROPRIETARY_MODULE := true

  include $(BUILD_PREBUILT)
endif
#************* libmmcamera_target Start ************#

LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= -Werror \
  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  $(USE_SERVER_TREE) \
  -include camera_defs_i.h \
  -DMSM_CAMERA_BIONIC

LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/types.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/in.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/un.h
LOCAL_CFLAGS += -include $(LOCAL_PATH)/chromatix/$(CHROMATIX_VERSION)/chromatix.h
LOCAL_CFLAGS += -DCHROMATIX_VERSION_$(CHROMATIX_VERSION)

ifeq ($(strip $(TARGET_USES_ION)),true)
  LOCAL_CFLAGS += -DUSE_ION
  ifeq ($(MSM_VERSION), 8974)
    LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
  else ifeq ($(MSM_VERSION), 8960)
    LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
  else ifeq ($(MSM_VERSION),7x27A)
    LOCAL_CFLAGS += -DTARGET_7x27A
    LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CAMERA_HEAP_ID
  else
    LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CAMERA_HEAP_ID
  endif
endif

ifeq ($(VFE_VERS),vfe40)
  LOCAL_CFLAGS += -DVFE_40
else ifeq ($(VFE_VERS),vfe32)
  LOCAL_CFLAGS += -DVFE_32
  ifeq ($(FEATURE_GYRO), true)
    LOCAL_CFLAGS += -DFEATURE_GYRO
  endif
else ifeq ($(VFE_VERS),vfe31)
  LOCAL_CFLAGS += -DVFE_31
  ifeq ($(MSM_VERSION), 7x30)
    LOCAL_CFLAGS += -DVFE_31_7x30
  else
    LOCAL_CFLAGS += -DVFE_31_8x60
  endif
else ifeq ($(VFE_VERS),vfe2x)
  LOCAL_CFLAGS += -DVFE_2X
endif

#ifeq ($(MM_DEBUG),true) # does not compile is not defined
LOCAL_CFLAGS+= -DIPL_DEBUG_STANDALONE
#endif

#for jpeg
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES+= .
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/adreno/
#inlcude GYRO sensor API header location
LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/sensors/inc
LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/qmi/inc

LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/common/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/$(VFE_VERS)/$(MSM_VERSION)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/$(VFE_VERS)/common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/$(VFE_VERS)/common/eztune
#LOCAL_C_INCLUDES+= hardware/qcom/display/libgralloc
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../hardware/qcom/camera

#all sub folders
ifeq ($(MSM_VERSION),7x27A)
  LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/camif/DSP_CAMIF
  LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/axi/DSP_AXI
else
  LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/camif/ARM_CAMIF
  LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/axi/ARM_AXI
endif


# This has to be removed. VERY VERY BAD
LOCAL_C_INCLUDES+= $(shell find $(LOCAL_PATH) -type d -print )
LOCAL_SHARED_LIBRARIES:= libC2D2 libcutils libdl

LOCAL_COPY_HEADERS_TO := mm-camera
LOCAL_COPY_HEADERS := ../common/camera_defs_i.h
LOCAL_COPY_HEADERS += ../common/camera.h
LOCAL_COPY_HEADERS += ../common/cam_mmap.h
LOCAL_COPY_HEADERS += ../common/cam_fifo.h
LOCAL_COPY_HEADERS += ../common/cam_list.h
LOCAL_COPY_HEADERS += core/mctl/camera_mctl_interface.h
LOCAL_COPY_HEADERS += include/camera_plugin_intf.h
LOCAL_COPY_HEADERS += include/tgtcommon.h
LOCAL_COPY_HEADERS += include/vfe_stats_def.h

ifeq ($(BUILD_CAM_FD),1)
  LOCAL_CFLAGS+= -DMM_CAMERA_FD
  CAM_FD_FILES:= \
    frameproc/facial/face_detection.c \
    frameproc/facial/pp_face_detection.c
else
  CAM_FD_FILES :=
endif

ifeq ($(FEATURE_FACE_PROC),true)
  LOCAL_CFLAGS+= -DMM_CAMERA_FD
  LOCAL_C_INCLUDES+= $(LOCAL_PATH)/frameproc/face_proc/engine
endif

include $(LOCAL_PATH)/../local_additional_dependency.mk

#add all common files
LOCAL_SRC_FILES:= \
  ../common/cam_mmap.c \
  ../common/stacktrace.c

LOCAL_HW_DIR :=$(LOCAL_PATH)/hardware

#add core framework
LOCAL_SRC_DIR := $(LOCAL_PATH)/core
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add features
LOCAL_SRC_DIR := $(LOCAL_PATH)/features/bestshot
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_SRC_DIR := $(LOCAL_PATH)/features/effects
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_SRC_DIR := $(LOCAL_PATH)/features/zoom
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_SRC_DIR := $(LOCAL_PATH)/features/hdr
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_SRC_DIR := $(LOCAL_PATH)/plugin
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_SRC_DIR := $(LOCAL_PATH)/features/live_snapshot
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add vpe/ccp
LOCAL_CCP_DIR :=$(LOCAL_HW_DIR)/vpe1
LOCAL_SRC_FILES += $(shell find $(LOCAL_CCP_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add flash
LOCAL_FLASH_DIR :=$(LOCAL_HW_DIR)/flash
LOCAL_SRC_FILES += $(shell find $(LOCAL_FLASH_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add camif
LOCAL_CAMIF_DIR :=$(LOCAL_HW_DIR)/camif
LOCAL_SRC_FILES += $(shell find $(LOCAL_CAMIF_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add csi
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/csi

LOCAL_CSI_IF_DIR :=hardware/csi
LOCAL_SRC_FILES +=$(LOCAL_CSI_IF_DIR)/csi_interface.c

ifeq ($(MSM_VERSION),8960)
LOCAL_CSI2_DIR :=$(LOCAL_HW_DIR)/csi/csi2
LOCAL_SRC_FILES += $(shell find $(LOCAL_CSI2_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
else ifeq ($(MSM_VERSION),8974)
LOCAL_CSI2_DIR :=$(LOCAL_HW_DIR)/csi/csi2
LOCAL_SRC_FILES += $(shell find $(LOCAL_CSI2_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
else
LOCAL_CSIC_DIR :=$(LOCAL_HW_DIR)/csi/csic
LOCAL_SRC_FILES += $(shell find $(LOCAL_CSIC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
endif

ifeq ($(MSM_VERSION),7x27A)
  LOCAL_SRC_FILES += $(shell find $(LOCAL_CAMIF_DIR)/DSP_CAMIF -name '*.c' | sed s:^$(LOCAL_PATH)::g )
else
  LOCAL_SRC_FILES += $(shell find $(LOCAL_CAMIF_DIR)/ARM_CAMIF -name '*.c' | sed s:^$(LOCAL_PATH)::g )
endif

#add ispif
LOCAL_ISPIF_DIR :=$(LOCAL_HW_DIR)/ispif
LOCAL_SRC_FILES += $(shell find $(LOCAL_ISPIF_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add axi
LOCAL_AXI_DIR :=$(LOCAL_HW_DIR)/axi
LOCAL_SRC_FILES += $(shell find $(LOCAL_AXI_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )
ifeq ($(MSM_VERSION),7x27A)
  LOCAL_SRC_FILES += $(shell find $(LOCAL_AXI_DIR)/DSP_AXI -name '*.c' | sed s:^$(LOCAL_PATH)::g )
else
  LOCAL_SRC_FILES += $(shell find $(LOCAL_AXI_DIR)/ARM_AXI -name '*.c' | sed s:^$(LOCAL_PATH)::g )
endif

#add c2d client
LOCAL_C2D_CLIENT_DIR :=$(LOCAL_HW_DIR)/c2d_client
LOCAL_SRC_FILES += $(shell find $(LOCAL_C2D_CLIENT_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add actuator
LOCAL_ACTUATOR_DIR :=$(LOCAL_HW_DIR)/actuator
LOCAL_SRC_FILES += $(shell find $(LOCAL_ACTUATOR_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add eeprom
LOCAL_EEPROM_DIR :=$(LOCAL_HW_DIR)/eeprom
LOCAL_SRC_FILES += $(shell find $(LOCAL_EEPROM_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add sensor
LOCAL_SENSOR_DIR :=$(LOCAL_HW_DIR)/sensor
LOCAL_SRC_FILES += $(shell find $(LOCAL_SENSOR_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#add dsps
ifeq ($(FEATURE_GYRO), true)
LOCAL_DSPS_DIR :=$(LOCAL_HW_DIR)/dsps
LOCAL_SRC_FILES += $(shell find $(LOCAL_DSPS_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
endif

#add vfe
LOCAL_VFE_IF_DIR_A :=$(LOCAL_HW_DIR)/vfe/vfe_interface
LOCAL_SRC_FILES += $(shell find $(LOCAL_VFE_IF_DIR_A) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_VFE_COMMON_DIR :=$(LOCAL_HW_DIR)/vfe/common
LOCAL_SRC_FILES += $(shell find $(LOCAL_VFE_COMMON_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_VFE_DIR :=$(LOCAL_HW_DIR)/vfe/$(VFE_VERS)

# No source files present in MSM_VERSION dir. as of now.
# If source files are added then please remove this comment.
LOCAL_SRC_FILES += $(shell find $(LOCAL_VFE_DIR)/$(MSM_VERSION) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_VFE_MSM_COMMON_DIR :=$(LOCAL_VFE_DIR)/common
LOCAL_SRC_FILES += $(shell find $(LOCAL_VFE_MSM_COMMON_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )
#LOCAL_SRC_FILES += $(shell find $(LOCAL_VFE_MSM_COMMON_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

ifeq ($(FEATURE_VFE_TEST_VEC),true)
  LOCAL_CFLAGS+= -DFEATURE_VFE_TEST_VECTOR
  LOCAL_VFE_TEST_DIR :=$(LOCAL_VFE_DIR)/test
  LOCAL_SRC_FILES += $(shell find $(LOCAL_VFE_TEST_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
endif

#add eztune vfe util
  LOCAL_VFE_EZTUNE_DIR :=$(LOCAL_VFE_MSM_COMMON_DIR)/eztune
  LOCAL_SRC_FILES += $(shell find $(LOCAL_VFE_EZTUNE_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

#basic VFE modules
LOCAL_VFE_MO_DIR_A :=hardware/vfe/vfe_modules
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demux/demux.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/wb/wb.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/abf/abf.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/gamma/gamma.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/rolloff.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/mesh_rolloff.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/mesh_rolloff_v4.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/pca_rolloff.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/mlro_to_plro/mlro.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/mlro_to_plro/mlro_utils.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/mlro_to_plro/matrix_utils.c
LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/parser/stats_parser.c

ifneq ($(VFE_VERS),vfe2x)
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/aec_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/af_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/awb_stats.c
else
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/aec_awb_7x27a.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/af_7x27a.c
endif

ifeq ($(VFE_VERS),vfe40)
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/colorxform/colorxform.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demosaic/demosaic_v3.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demosaic/demosaic_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bcc_v3.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bcc_v3_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bpc_v3.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bpc_v3_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/linearization/linearization_v1.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/linearization/linearization_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clf/clf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clf/clf_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/mce/mce_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/mce/mce.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_suppress/chroma_suppress.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_suppress/chroma_suppress_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/sce/sce.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/sce/sce_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/luma_adaptation/luma_adaptation.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/luma_adaptation/luma_adaptation_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/scaler/scaler_v4.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/scaler/scaler_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/fovcrop/fovcrop_v4.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/fovcrop/fovcrop_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demux/demux_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clamp/clamp_v4.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clamp/clamp_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/colorcorrect/colorcorrect.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/colorcorrect/colorcorrect_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_enhan/chroma_enhan.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_enhan/chroma_enhan_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/wb/wb_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/abf/abf_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/gamma/gamma_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/rolloff_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/awb_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/awb_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/rs_cs_stats_v4.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/rs_cs_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/ihist_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/ihist_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/be_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/be_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bg_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bg_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bf_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bf_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bhist_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bhist_stats_intf.c
else ifeq ($(VFE_VERS),vfe32)
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/asf/asf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/frame_skip/frame_skip.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/frame_skip/frame_skip_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demosaic/demosaic_v3.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demosaic/demosaic_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bcc_v3.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bcc_v3_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bpc_v3.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bpc_v3_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/linearization/linearization_v1.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/linearization/linearization_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clf/clf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clf/clf_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bg_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bf_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bhist_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/mce/mce_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/mce/mce.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_suppress/chroma_suppress.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_suppress/chroma_suppress_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/sce/sce.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/sce/sce_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/luma_adaptation/luma_adaptation.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/luma_adaptation/luma_adaptation_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/rs_cs_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/ihist_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/scaler/scaler.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/scaler/scaler_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/fovcrop/fovcrop.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/fovcrop/fovcrop_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demux/demux_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clamp/clamp.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clamp/clamp_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/colorcorrect/colorcorrect.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/colorcorrect/colorcorrect_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_enhan/chroma_enhan.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_enhan/chroma_enhan_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_subsample/chroma_subsample.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_subsample/chroma_subsample_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/wb/wb_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/abf/abf_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/gamma/gamma_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/asf/asf_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/rolloff/rolloff_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/aec_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/af_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/awb_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/rs_cs_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/ihist_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bg_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bf_stats_intf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/bhist_stats_intf.c
else ifeq ($(VFE_VERS),vfe2x)
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/asf/asf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_enhan/chroma_enhan.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clamp/clamp.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/colorcorrect/colorcorrect.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/fovcrop/fovcrop.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_subsample/chroma_subsample.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demosaic/demosaic_v3.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/black_level/black_level_v1.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bpc_v3_7x27a.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/frame_skip/frame_skip_v1.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/active_crop/active_crop.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/color_proc/color_proc.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/scaler/scaler_v1.c
else
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/asf/asf.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_enhan/chroma_enhan.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/clamp/clamp.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/colorcorrect/colorcorrect.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/fovcrop/fovcrop.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_subsample/chroma_subsample.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/frame_skip/frame_skip.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/black_level/black_level.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/demosaic/demosaic.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/bpc/bpc.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/mce/mce.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/chroma_suppress/chroma_suppress.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/sce/sce.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/luma_adaptation/luma_adaptation.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/scaler/scaler.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/rs_cs_stats.c
  LOCAL_SRC_FILES +=$(LOCAL_VFE_MO_DIR_A)/stats/config/ihist_stats.c
endif
#other VFE moduels
#TBD

LOCAL_MODULE:=libmmcamera_target
LOCAL_MODULE_TAGS := optional

include $(LOCAL_PATH)/../malloc_wrap_static_library.mk
#************* libmmcamera_target End ************#

#************* libmmcamera_statsproc31 Start ************#
include $(CLEAR_VARS)
LOCAL_CFLAGS:= -Werror \
  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  $(USE_SERVER_TREE) \
  -include camera_defs_i.h \
  -DMSM_CAMERA_BIONIC

ifeq ($(strip $(TARGET_USES_ION)),true)
  LOCAL_CFLAGS += -DUSE_ION
endif

LOCAL_CFLAGS += -include $(LOCAL_PATH)/chromatix/0208/chromatix.h
LOCAL_CFLAGS += -include $(LOCAL_PATH)/hardware/actuator/0208/af_tuning.h

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/$(VFE_VERS)/common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/include
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/$(VFE_VERS)/$(MSM_VERSION)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/actuator/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/statsproc
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg
#LOCAL_C_INCLUDES += bionic/libc/include
#LOCAL_C_INCLUDES += bionic/libc/kernel/common/linux
#LOCAL_C_INCLUDES += bionic/libc/kernel/common/media

include $(LOCAL_PATH)/../local_additional_dependency.mk
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../hardware/qcom/camera

LOCAL_SRC_FILES:= \
  statsproc/stats_proc.c \
  statsproc/aec/aec_interface.c \
  statsproc/aec/aec.c \
  statsproc/aec/aec_util.c \
  statsproc/aec/aec_snapshot.c \
  statsproc/aec/aec_slow_conv.c \
  statsproc/awb/awb_interface.c \
  statsproc/awb/awb.c \
  statsproc/awb/awb_util.c \
  statsproc/awb/awb_self_cal.c \
  statsproc/awb/awb_agw.c \
  statsproc/asd/asd_interface.c \
  statsproc/asd/asd.c \
  statsproc/af/af_interface.c \
  statsproc/af/af_util.c \
  statsproc/af/af.c \
  statsproc/af/af_exhstv.c \
  statsproc/af/af_hillclimb.c \
  statsproc/af/af_slopepredictive.c \
  statsproc/afd/afd_interface.c \
  statsproc/afd/afd.c \
  statsproc/is/is_interface.c \
  statsproc/is/dis/dis_interface.c \
  statsproc/is/eis/eis_interface.c

LOCAL_MODULE           := libmmcamera_statsproc31
LOCAL_SHARED_LIBRARIES := libcutils libmmcamera_image_stab
LOCAL_PRELINK_MODULE   := false
LOCAL_MODULE_TAGS := optional

ifeq ($(MM_DEBUG),true)
  LOCAL_SHARED_LIBRARIES += liblog
endif
LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
#************* libmmcamera_statsproc31 End ************#

#************* libmmcamera_frameproc Start ************#
include $(CLEAR_VARS)
LOCAL_CFLAGS:= -Werror \
  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  $(USE_SERVER_TREE) \
  -include camera_defs_i.h \
  -DMSM_CAMERA_BIONIC


ifeq ($(strip $(TARGET_USES_ION)),true)
  LOCAL_CFLAGS += -DUSE_ION
endif

ifeq ($(FEATURE_FACE_PROC),true)
  LOCAL_CFLAGS+= -DMM_CAMERA_FD
endif

ifeq ($(TARGET_NEON_ENABLED),true)
  LOCAL_CFLAGS+= -DMM_CAMERA_NEON_ENABLED
endif

ifeq ($(VFE_VERS),vfe2x)
  LOCAL_CFLAGS += -DVFE_2X
endif

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/$(VFE_VERS)/common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/include
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/$(VFE_VERS)/$(MSM_VERSION)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/frameproc
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg
#LOCAL_C_INCLUDES += bionic/libc/include
#LOCAL_C_INCLUDES += bionic/libc/kernel/common/linux
#LOCAL_C_INCLUDES += bionic/libc/kernel/common/media
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../hardware/qcom/camera

ifeq ($(FEATURE_FACE_PROC),true)
  LOCAL_C_INCLUDES+= $(LOCAL_PATH)/frameproc/face_proc/engine
  LOCAL_CFLAGS+= -DMM_CAMERA_FD
endif

include $(LOCAL_PATH)/../local_additional_dependency.mk

LOCAL_SRC_FILES:= \
  frameproc/hjr/hjr_interface.c \
  frameproc/hjr/hjr.c \
  frameproc/wavelet_denoise/wavelet_denoise_interface.c \
  frameproc/hdr/hdr_interface.c \


ifeq ($(FEATURE_FACE_PROC),true)
  LOCAL_SRC_FILES += frameproc/frameproc.c
ifeq ($(MSM_VERSION),7x27A)
  LOCAL_SRC_FILES += frameproc/afd/afd_interface.c
  LOCAL_SRC_FILES += frameproc/afd/afd.c
endif
  LOCAL_SRC_FILES += frameproc/face_proc/face_proc_interface.c
  LOCAL_SRC_FILES += frameproc/face_proc/face_proc_util.c
  LOCAL_SRC_FILES += frameproc/face_proc/engine/faceproc_engine.c
endif

LOCAL_MODULE           := libmmcamera_frameproc
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libdl libcutils
LOCAL_SHARED_LIBRARIES += libmmcamera_wavelet_lib libmmcamera_hdr_lib
LOCAL_MODULE_TAGS := optional
ifeq ($(MM_DEBUG),true)
  LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
#************* libmmcamera_frameproc End ************#



#************* libcpp Start ************#
LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_SRC_FILES:= \
        hardware/cpp/cpp.c hardware/cpp/cpp_params.c

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/include
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/cpp/
LOCAL_C_INCLUDES+= hardware/qcom/camera

LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg

LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES:= libcutils libdl

LOCAL_MODULE:= libmmcamera_cpp

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
#************* libcpp End************#

#************* libcameraplugin Start ************#
LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(CLEAR_VARS)

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_SRC_FILES:= \
        plugin/camera_plugin.c
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/include
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/plugin/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/modules/
LOCAL_C_INCLUDES+= hardware/qcom/camera
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/include/mm-still/jpeg
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/common/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/vfe_interface/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/vfe32/8960/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/vfe_modules/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/hardware/vfe/vfe32/test/ \
$(LOCAL_PATH)/include \
$(LOCAL_PATH)/common \
$(LOCAL_PATH)/hardware/vfe/vfe_interface \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/abf \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/asf \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/black_level \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/bpc \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/chroma_enhan \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/chroma_subsample \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/chroma_suppress \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/clamp \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/clf \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/color_proc \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/colorcorrect \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/colorxform \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/demosaic \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/demux \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/fovcrop \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/frame_skip \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/gamma \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/linearization \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/luma_adaptation \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/mce \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/rolloff \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/scaler \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/sce \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/stats/config \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/stats/parser \
$(LOCAL_PATH)/hardware/vfe/vfe_modules/wb

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES:= libcutils libdl

LOCAL_MODULE:= libmmcamera_plugin

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
#************* libcameraplugin End************#

include $(LOCAL_PATH)/chromatix/$(CHROMATIX_VERSION)/libchromatix/Android.mk
include $(call all-subdir-makefiles)

LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/hardware/sensor_libs/Android.mk
include $(call all-subdir-makefiles)

LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/unit_test/Android.mk
include $(call all-subdir-makefiles)

