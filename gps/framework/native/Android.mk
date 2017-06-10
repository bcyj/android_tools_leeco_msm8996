#=#====#====#====#====#====#====#====#====#====#====#====#====#====#====#====#
#
#        Location Service module
#
# GENERAL DESCRIPTION
#   Location Service module makefile
#
#=============================================================================
ifneq ($(BUILD_TINY_ANDROID),true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := liblocationservice

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
    jni/DeviceContext_jni.cpp \
    jni/LocationService_jni.cpp \
    jni/RilInfoMonitor_jni.cpp \
    jni/UlpEngine_jni.cpp \
    jni/Wiper_jni.cpp \
    adapter/LBSAdapter.cpp \
    service/loc_extended.cpp \
    geofence/com_qualcomm_services_location_GeoFenceKeeper.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libandroid_runtime \
    libnativehelper \
    libloc_core \
    libizat_core \
    libloc_eng \
    libgps.utils \
    libhardware \
    libhardware_legacy \
    libgeofence \
    libulp2 \
    liblbs_core

LOCAL_C_INCLUDES := \
    $(JNI_H_INCLUDE) \
    $(LOCAL_PATH)/service \
    $(LOCAL_PATH)/adapter \
    $(TOP)/frameworks/base/include/android_runtime \
    $(TOP)/hardware/qcom/gps/loc_api/ulp/inc \
    $(TARGET_OUT_HEADERS)/gps.utils \
    $(TARGET_OUT_HEADERS)/libloc_eng \
    $(TARGET_OUT_HEADERS)/libloc_core \
    $(TARGET_OUT_HEADERS)/libizat_core \
    $(TOP)/vendor/qcom/proprietary/gps-noship/internal/ulp2/inc \
    $(TARGET_OUT_HEADERS)/liblbs_core

LOCAL_CFLAGS+=$(GPS_FEATURES) \
    -D_ANDROID_ \
    -DON_TARGET_TEST

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := liblocationservice
LOCAL_COPY_HEADERS := \
    service/ulp_service.h \
    service/loc_extended.h \
    adapter/LBSAdapter.h \
    geofence/GeoFencerProxyMsgs.h

include $(BUILD_COPY_HEADERS)

include $(addsuffix /Android.mk, $(addprefix $(LOCAL_PATH)/,core))

endif # not BUILD_TINY_ANDROID
