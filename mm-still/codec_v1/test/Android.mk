
LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)

JPEG_PATH := $(LOCAL_DIR_PATH)/../../jpeg2

jpeg_test_defines:= -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h \

jpeg_test_defines += -D_ANDROID_
jpeg_test_defines += -DLOGE=ALOGE
jpeg_test_defines += -DCODEC_V1

ifeq ($(strip $(FACT_VER)),codecB)
jpeg_test_defines += -DCODEC_B
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
# remove the following after reimplementing the exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += hardware/qcom/camera/mm-image-codec/qexif


jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/videodev2.h

ifeq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3 4.4.4))
jpeg_test_defines += -include bionic/libc/kernel/uapi/asm-arm/asm/byteorder.h
jpeg_test_defines += -include bionic/libc/kernel/uapi/linux/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/uapi/linux/types.h
else
jpeg_test_defines += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/arch-arm/asm/byteorder.h
jpeg_test_defines += -include bionic/libc/kernel/common/linux/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/common/linux/types.h
endif

LOCAL_CFLAGS := $(jpeg_test_defines)

LOCAL_SRC_FILES := QJpegEncoderTest.cpp

LOCAL_SHARED_LIBRARIES:= \
	libmmqjpeg_codec libcutils libdl

LOCAL_MODULE:= mm-qjpeg-enc-test
LOCAL_32_BIT_ONLY       := true

LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)

################# decoder #####################################
ifeq ($(strip $(FACT_VER)),codecB)

include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)

JPEG_PATH := $(LOCAL_DIR_PATH)/../../jpeg2

jpeg_test_defines:= -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h

ifneq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3))
    jpeg_test_defines += -Werror
endif
jpeg_test_defines += -D_ANDROID_
jpeg_test_defines += -DLOGE=ALOGE
jpeg_test_defines += -DCODEC_V1

ifeq ($(strip $(FACT_VER)),codecB)
jpeg_test_defines += -DCODEC_B
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
# remove the following after reimplementing the exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += hardware/qcom/camera/mm-image-codec/qexif


jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/videodev2.h

ifeq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3 4.4.4))
jpeg_test_defines += -include bionic/libc/kernel/uapi/asm-arm/asm/byteorder.h
jpeg_test_defines += -include bionic/libc/kernel/uapi/linux/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/uapi/linux/types.h
else
jpeg_test_defines += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/arch-arm/asm/byteorder.h
jpeg_test_defines += -include bionic/libc/kernel/common/linux/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/common/linux/types.h
endif

LOCAL_CFLAGS := $(jpeg_test_defines)

LOCAL_SRC_FILES := QJpegDecoderTest.cpp

LOCAL_SHARED_LIBRARIES:= \
	libmmqjpeg_codec libcutils libdl

LOCAL_MODULE:= mm-qjpeg-dec-test
LOCAL_32_BIT_ONLY       := true

LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)
endif

################# makernote data extractor #####################################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_DIR_PATH)

JPEG_PATH := $(LOCAL_DIR_PATH)/../../jpeg2

jpeg_test_defines:= -g -O2 \
       -D_ANDROID_ \
       -include QIDbg.h

ifneq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3))
jpeg_test_defines += -Werror
endif

jpeg_test_defines += -D_ANDROID_
jpeg_test_defines += -DLOGE=ALOGE
jpeg_test_defines += -DCODEC_V1

OPENSSL_HEADER_DIR := external/openssl/include

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
# remove the following after reimplementing the exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(OPENSSL_HEADER_DIR)
LOCAL_C_INCLUDES += hardware/qcom/camera/mm-image-codec/qexif
LOCAL_C_INCLUDES  += $(LOCAL_PATH)/../qcrypt
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
jpeg_test_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/videodev2.h

ifeq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3))
jpeg_test_defines += -include bionic/libc/kernel/uapi/asm-arm/asm/byteorder.h
jpeg_test_defines += -include bionic/libc/kernel/uapi/linux/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/uapi/linux/types.h
else
jpeg_test_defines += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/arch-arm/asm/byteorder.h
jpeg_test_defines += -include bionic/libc/kernel/common/linux/posix_types.h
jpeg_test_defines += -include bionic/libc/kernel/common/linux/types.h
endif

LOCAL_CFLAGS := $(jpeg_test_defines)

LOCAL_SRC_FILES := QJpegMakernoteExtract.cpp

LOCAL_SHARED_LIBRARIES:= \
	libmmqjpeg_codec libcutils libdl

LOCAL_MODULE:= qmakernote-xtract
LOCAL_32_BIT_ONLY       := true

LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)
