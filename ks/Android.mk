ifeq ($(TARGET_ARCH),arm)
ifeq ($(call is-board-platform-in-list,msm8660 msm8960 msm8974 apq8084),true)

LOCAL_PATH := $(call my-dir)
commonSources :=
commonSharedLibraries := libc libcutils libutils

include $(CLEAR_VARS)
LOCAL_MODULE := ks
LOCAL_C_INCLUDES := kickstart_utils.h \
                   $(TARGET_OUT_HEADERS)/common/inc/

#LIBS = -lrt -lpthread 

LOCAL_SRC_FILES += $(commonSources) kickstart.c comm.c kickstart_log.c kickstart_utils.c sahara_protocol.c
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/capability.h
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
ifeq ($(call is-board-platform,msm8660),true)
    LOCAL_CFLAGS += -DFUSION2
endif
ifeq ($(call is-board-platform-in-list,msm8960 msm8974 apq8084),true)
    LOCAL_CFLAGS += -DFUSION3
endif
LOCAL_MODULE := qcks
LOCAL_C_INCLUDES := kickstart_utils.h \
                   $(TARGET_OUT_HEADERS)/common/inc/

#LIBS = -lrt -lpthread 

LOCAL_SRC_FILES += $(commonSources) qcks.c
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

ifeq ($(call is-board-platform-in-list,msm8960 msm8974 apq8084),true)
include $(CLEAR_VARS)

LOCAL_MODULE := efsks
LOCAL_C_INCLUDES := kickstart_utils.h \
                   $(TARGET_OUT_HEADERS)/common/inc/


LOCAL_SRC_FILES += $(commonSources) efsks.c
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif

LOCAL_CFLAGS+=-Werror

endif
endif
