ifneq ($(TARGET_USES_AOSP),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(LOCAL_PATH)/../QSEEComAPI \
                    $(LOCAL_PATH)/../tzcommon/inc \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        liblog \
        libdl \
        libQSEEComAPI \
        libdrmfs \

LOCAL_MODULE := qseecomd
LOCAL_SRC_FILES := qseecomd.c
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

###############Compile QSEECOMFSD library #################################

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(LOCAL_PATH)/../QSEEComAPI \
                    $(LOCAL_PATH)/../tzcommon/inc \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_LDLIBS := -llog
LOCAL_LDLIB +=- landroid
LOCAL_MODULE := qseecomfsd
LOCAL_SRC_FILES := qseecomfsd.c
LOCAL_STATIC_LIBRARIES := libdrmMinimalfs \
                          libQSEEComAPIStatic \
                          libc \
                          liblog \
                          libcutils \

LOCAL_MODULE_PATH := $(TARGET_RECOVERY_OUT)
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

##############Compile QSEECOMD_STATIC ####################################

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(LOCAL_PATH)/../QSEEComAPI \
                    $(LOCAL_PATH)/../tzcommon/inc \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_STATIC_LIBRARIES := \
        libc \
        libcutils \
        liblog \
        libssdStatic \
        librpmbStatic \
	libQSEEComAPIStatic

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := qseecomd_static
LOCAL_SRC_FILES := qseecomd_static.c
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS) -static
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
