LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := qdp_test.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp


LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_02.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp


LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_02
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_03.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp


LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_03
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_04.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp


LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_04
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_05.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp


LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_05
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_06.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp


LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_MODULE := qdp_test_06
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_07.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp
LOCAL_SHARED_LIBRARIES += libqmi

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_07
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_08.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp
LOCAL_SHARED_LIBRARIES += libqmi

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_08
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_09.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp


LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_MODULE := qdp_test_09
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := qdp_test_10.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp
LOCAL_SHARED_LIBRARIES += libqmi

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_10
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test

include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := qdp_test_create_profile.c

#LOCAL_CFLAGS := -DFEATURE_DSI_TEST

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqdp
LOCAL_SHARED_LIBRARIES += libqmi

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dsutils/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qdp/src
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(call is-board-platform,msm7630_fusion),true)
LOCAL_CFLAGS += -DFEATURE_QDP_FUSION
endif

LOCAL_MODULE := qdp_test_create_profile
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)
