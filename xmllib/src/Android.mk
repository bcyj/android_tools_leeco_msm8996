LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := xmllib/inc
LOCAL_COPY_HEADERS      := ../inc/xmllib_common.h \
                           ../inc/xmllib_parser.h \
                           ../inc/xmllib_tok.h

# measurements show that the ARM version of ZLib is about x1.17 faster
# than the thumb one...
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
          $(LOCAL_PATH)/../inc \
          $(TARGET_OUT_HEADERS)/common/inc \
          $(TARGET_OUT_HEADERS)/diag/include

LOCAL_SRC_FILES:= \
          xmllib_decl.c\
          xmllibi_decl_utf8.c\
          xmllibi_utf8_util.c\
          xmllib_generator.c\
          xmllibi_tok_ascii.c\
          xmllib_parser.c\
          xmllibi_decl_ascii.c\
          xmllibi_tok_utf8.c\
          xmllib_tok.c

LOCAL_MODULE:= libxml
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libcutils \
                          libdiag

LOCAL_CFLAGS+= -DFEATURE_XMLLIB

# Enable one of the logging mechanisms
#LOCAL_CFLAGS+= -DFEATURE_XMLLIB_LOG_STDERR
LOCAL_CFLAGS+= -DFEATURE_XMLLIB_LOG_QXDM

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
