include $(CLEAR_VARS)

LOCAL_SRC_FILES+=                                    \
    Android_adaptation/src/QCExtractor.cpp

LOCAL_C_INCLUDES:=                                   \
    $(LOCAL_PATH)/../common/inc                      \

LOCAL_SHARED_LIBRARIES :=       \
        libutils                \
        libcutils               \
        libdl                   \
        libstagefright          \

LOCAL_MODULE:= libExtendedExtractor

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

