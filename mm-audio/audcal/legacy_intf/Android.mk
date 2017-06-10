ifeq ($(call is-board-platform-in-list,msm8960 msm8930 msm8660),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libaudcal-def := -g -O3
libaudcal-def += -D_ANDROID_
libaudcal-def += -D_ENABLE_QC_MSG_LOG_

libaudcal-inc     := $(TARGET_OUT_HEADERS)/diag/include
libaudcal-inc     += $(TARGET_OUT_HEADERS)/common/inc
libaudcal-inc     += $(AUDIO_ROOT)/audio-acdb-util/acdb-mapper/inc


# ---------------------------------------------------------------------------------
#             Include 8960/8660
# ---------------------------------------------------------------------------------

ifeq ($(call is-board-platform,msm8960),true)
libaudcal-def     += -D_NO_ACDB_MAPPER_LIB_
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acdb/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acdb/src
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/actp/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/audtp/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/audtp/src
libaudcal-inc     += $(LOCAL_PATH)/8960/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acph/src

else ifeq ($(call is-board-platform,msm8930),true)
libaudcal-def     += -D_NO_ACDB_MAPPER_LIB_
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acdb/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acdb/src
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/actp/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/audtp/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/audtp/src
libaudcal-inc     += $(LOCAL_PATH)/8930/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/common_8960/acph/src

else ifeq ($(call is-board-platform,msm8660),true)
libaudcal-inc     += $(LOCAL_PATH)/common/acdb/inc
libaudcal-inc     += $(LOCAL_PATH)/common/acdb/src
libaudcal-inc     += $(LOCAL_PATH)/common/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/common/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/common/actp/inc
libaudcal-inc     += $(LOCAL_PATH)/common/audtp/inc
libaudcal-inc     += $(LOCAL_PATH)/common/audtp/src
libaudcal-inc     += $(LOCAL_PATH)/8660/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/8660/acph/src

endif

# ---------------------------------------------------------------------------------
#             Make the Shared library (libaudcal)
# ---------------------------------------------------------------------------------


LOCAL_MODULE            := libaudcal
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libaudcal-def)
LOCAL_C_INCLUDES        := $(libaudcal-inc)
LOCAL_C_INCLUDES	+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog libdiag

ifneq "$(findstring NO_ACDB_MAPPER_LIB,$(LOCAL_CFLAGS))" "NO_ACDB_MAPPER_LIB"
LOCAL_SHARED_LIBRARIES  += libacdbmapper
endif

LOCAL_COPY_HEADERS_TO   := mm-audio/audcal

ifeq ($(call is-board-platform,msm8960),true)
LOCAL_COPY_HEADERS      := common_8960/acdb/inc/acdb.h
LOCAL_COPY_HEADERS      += common_8960/acph/inc/acph.h
LOCAL_COPY_HEADERS      += common_8960/acdb/inc/acdb_deprecate.h
LOCAL_COPY_HEADERS      += 8960/acdbdata/inc/acdb_includes.h
else ifeq ($(call is-board-platform,msm8930),true)
LOCAL_COPY_HEADERS      := common_8960/acdb/inc/acdb.h
LOCAL_COPY_HEADERS      += common_8960/acph/inc/acph.h
LOCAL_COPY_HEADERS      += common_8960/acdb/inc/acdb_deprecate.h
LOCAL_COPY_HEADERS      += 8930/acdbdata/inc/acdb_includes.h
else ifeq ($(call is-board-platform,msm8660),true)
LOCAL_COPY_HEADERS      := common/acdb/inc/acdb.h
LOCAL_COPY_HEADERS      += common/acph/inc/acph.h
LOCAL_COPY_HEADERS      += 8660/acdbdata/inc/acdb_includes.h
endif

ifeq ($(call is-board-platform,msm8960),true)
LOCAL_SRC_FILES         := common_8960/acdb/src/acdb.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_command.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_init.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_override.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_parser.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_linked_list.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_translation.c
LOCAL_SRC_FILES         += common_8960/acdb/la/src/acdb_init_utility.c
LOCAL_SRC_FILES         += common_8960/actp/src/actp.c
LOCAL_SRC_FILES         += common_8960/audtp/src/audtp.c
LOCAL_SRC_FILES         += 8960/acdbdata/src/acdb_default_data.c
LOCAL_SRC_FILES         += common_8960/acph/src/acph.c
LOCAL_SRC_FILES         += common_8960/acph/src/rtc_apps_intf.c
LOCAL_SRC_FILES         += common_8960/acph/src/rtc_q6_intf.c
else ifeq ($(call is-board-platform,msm8930),true)
LOCAL_SRC_FILES         := common_8960/acdb/src/acdb.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_command.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_init.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_override.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_parser.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_linked_list.c
LOCAL_SRC_FILES         += common_8960/acdb/src/acdb_translation.c
LOCAL_SRC_FILES         += common_8960/acdb/la/src/acdb_init_utility.c
LOCAL_SRC_FILES         += common_8960/actp/src/actp.c
LOCAL_SRC_FILES         += common_8960/audtp/src/audtp.c
LOCAL_SRC_FILES         += 8930/acdbdata/src/acdb_default_data.c
LOCAL_SRC_FILES         += common_8960/acph/src/acph.c
LOCAL_SRC_FILES         += common_8960/acph/src/rtc_apps_intf.c
LOCAL_SRC_FILES         += common_8960/acph/src/rtc_q6_intf.c
else ifeq ($(call is-board-platform,msm8660),true)
LOCAL_SRC_FILES         := common/acdb/src/acdb.c
LOCAL_SRC_FILES         += common/acdb/src/acdb_command.c
LOCAL_SRC_FILES         += common/acdb/src/acdb_init.c
LOCAL_SRC_FILES         += common/acdb/src/acdb_override.c
LOCAL_SRC_FILES         += common/acdb/src/acdb_parser.c
LOCAL_SRC_FILES         += common/acdb/src/acdb_linked_list.c
LOCAL_SRC_FILES         += common/acdb/src/acdb_translation.c
LOCAL_SRC_FILES         += common/acdb/la/src/acdb_init_utility.c
LOCAL_SRC_FILES         += common/actp/src/actp.c
LOCAL_SRC_FILES         += common/audtp/src/audtp.c
LOCAL_SRC_FILES         += 8660/acdbdata/src/acdb_default_data.c
LOCAL_SRC_FILES         += 8660/acph/src/acph.c
LOCAL_SRC_FILES         += 8660/acph/src/rtc_apps_intf.c
LOCAL_SRC_FILES         += 8660/acph/src/rtc_q6_intf.c
endif

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

ifeq ($(call is-board-platform-in-list,msm8660),true)
# ---------------------------------------------------------------------------------
#             Make the apps-test (acdbtest)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

acdbtest-inc := $(LOCAL_PATH)/common/acdb/inc
acdbtest-inc += $(AUDIO_ROOT)/audio-alsa/inc

LOCAL_MODULE            := acdbtest
LOCAL_MODULE_TAGS       := optional
LOCAL_C_INCLUDES        := $(acdbtest-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libaudcal
LOCAL_SHARED_LIBRARIES  += libaudioalsa
LOCAL_CFLAGS            := $(libaudcal-def)

LOCAL_SRC_FILES         := common/test/acdbtest.c

include $(BUILD_EXECUTABLE)

else ifeq ($(call is-board-platform-in-list,msm8960),true)
# ---------------------------------------------------------------------------------
#             Make the apps-test (mm-audio-send-cal)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

sendcal-inc := $(AUDIO_ROOT)/audio-acdb-util/acdb-loader/inc/8960

LOCAL_MODULE            := mm-audio-send-cal
LOCAL_MODULE_TAGS       := optional
LOCAL_C_INCLUDES        := $(sendcal-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libacdbloader

LOCAL_SRC_FILES         := common_8960/test/sendcal.c

include $(BUILD_EXECUTABLE)

endif

endif

else ifeq ($(call is-chipset-in-board-platform,msm7630),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libaudcal-def := -g -O3
libaudcal-def += -D_ANDROID_
libaudcal-def += -D_ENABLE_QC_MSG_LOG_

# ---------------------------------------------------------------------------------
#             Make the Shared library (libaudcal)
# ---------------------------------------------------------------------------------

libaudcal-inc     := $(LOCAL_PATH)/common/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/8660/acph/src
libaudcal-inc     += $(LOCAL_PATH)/7x30/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/7x30/acph/src
libaudcal-inc     += $(LOCAL_PATH)/common/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/8660/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/common/actp/inc
libaudcal-inc     += $(LOCAL_PATH)/common/audtp/inc
libaudcal-inc     += $(LOCAL_PATH)/common/audtp/src
libaudcal-inc     += $(TARGET_OUT_HEADERS)/common/inc
libaudcal-inc     += $(TARGET_OUT_HEADERS)/diag/include
libaudcal-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_MODULE            := libaudcal
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libaudcal-def)
LOCAL_C_INCLUDES        := $(libaudcal-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog libdiag
LOCAL_COPY_HEADERS_TO   := mm-audio/audcal
LOCAL_COPY_HEADERS      := /7x30/acph/inc/initialize_audcal7x30.h

LOCAL_SRC_FILES         := common/actp/src/actp.c
LOCAL_SRC_FILES         += common/audtp/src/audtp.c
LOCAL_SRC_FILES         += 7x30/acph/src/acph7x30.c
LOCAL_SRC_FILES         += 7x30/acph/src/rtc_q5_intf7x30.c

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID

else ifeq ($(call is-chipset-in-board-platform,msm7627a),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libaudcal-def := -g -O3
libaudcal-def += -D_ANDROID_
libaudcal-def += -D_ENABLE_QC_MSG_LOG_

# ---------------------------------------------------------------------------------
#             Make the Shared library (libaudcal)
# ---------------------------------------------------------------------------------

libaudcal-inc     := $(LOCAL_PATH)/common/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/8660/acph/src
libaudcal-inc     += $(LOCAL_PATH)/8x25/acph/inc
libaudcal-inc     += $(LOCAL_PATH)/8x25/acph/src
libaudcal-inc     += $(LOCAL_PATH)/common/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/8660/acdbdata/inc
libaudcal-inc     += $(LOCAL_PATH)/common/actp/inc
libaudcal-inc     += $(LOCAL_PATH)/common/audtp/inc
libaudcal-inc     += $(LOCAL_PATH)/common/audtp/src
libaudcal-inc     += $(TARGET_OUT_HEADERS)/common/inc
libaudcal-inc     += $(TARGET_OUT_HEADERS)/diag/include
libaudcal-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_MODULE            := libaudcal
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libaudcal-def)
LOCAL_C_INCLUDES        := $(libaudcal-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog libdiag
LOCAL_COPY_HEADERS_TO   := mm-audio/audcal
LOCAL_COPY_HEADERS      := /8x25/acph/inc/initialize_audcal8x25.h

LOCAL_SRC_FILES         := common/actp/src/actp.c
LOCAL_SRC_FILES         += common/audtp/src/audtp.c
LOCAL_SRC_FILES         += 8x25/acph/src/acph8x25.c
LOCAL_SRC_FILES         += 8x25/acph/src/rtc_q5_intf8x25.c

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
endif

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

