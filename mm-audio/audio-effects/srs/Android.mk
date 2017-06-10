SRS_PROCESSING_ROOT := $(call my-dir)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO := mm-audio/audio-effects
LOCAL_COPY_HEADERS := TruMedia/srs_processing.h
LOCAL_COPY_HEADERS += TruMedia/postpro_patch.h
include $(BUILD_COPY_HEADERS)

