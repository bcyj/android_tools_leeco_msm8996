
include $(CLEAR_VARS)
include $(LOCAL_PATH)/build_common.mk

# ---------------------------------------------------------------------------------
#                 Exporting MM-Parser include files to public folder(s)
# ---------------------------------------------------------------------------------
LOCAL_COPY_HEADERS_TO := mm-parser/include
LOCAL_COPY_HEADERS := FileBaseLib/inc/isucceedfail.h
LOCAL_COPY_HEADERS += FileBaseLib/inc/parserdatadef.h
LOCAL_COPY_HEADERS += FileBaseLib/inc/parserinternaldefs.h
LOCAL_COPY_HEADERS += FileBaseLib/inc/oscl_file_io.h
LOCAL_COPY_HEADERS += FileBaseLib/inc/filesourcestring.h
LOCAL_COPY_HEADERS += FileBaseLib/inc/zrex_string.h
LOCAL_COPY_HEADERS += HTTPSource/MAPI08/API/DataSourcePort.h
LOCAL_COPY_HEADERS += HTTPSource/MAPI08/API/SourceBase.h
LOCAL_COPY_HEADERS += FileSource/inc/filesourcetypes.h
LOCAL_COPY_HEADERS += FileSource/inc/filesource.h

LOCAL_CFLAGS +=             \
    -DFEATURE_FILESOURCE_DIVX_DRM

LOCAL_SRC_FILES+=                                    \
    AVIParserLib/src/Qtv_DivxRegistration.cpp

LOCAL_C_INCLUDES+=                                   \
    $(LOCAL_PATH)/Api/DivxDRMLib_API

#update the mm-parser-bin if there are new targets added
ifeq ($(call is-chipset-prefix-in-board-platform,msm7627),true)
LOCAL_C_INCLUDES+=                                  \
    $(TARGET_OUT_HEADERS)/mm-video/DivxDrmDecrypt
endif

ifeq ($(call is-chipset-in-board-platform,msm7630),true)
LOCAL_C_INCLUDES+=                                  \
    $(TARGET_OUT_HEADERS)/mm-video/DivxDrmDecrypt
endif

ifeq ($(call is-board-platform,msm8660),true)
LOCAL_C_INCLUDES+=                                  \
    $(TARGET_OUT_HEADERS)/mm-video/DivxDrmDecrypt
endif
ifeq ($(call is-board-platform,msm8960),true)
LOCAL_C_INCLUDES+=                                  \
    $(TARGET_OUT_HEADERS)/mm-video/DivxDrmDecrypt
endif
ifeq ($(call is-board-platform,msm8610),true)
LOCAL_C_INCLUDES+=                                  \
    $(TARGET_OUT_HEADERS)/mm-video/DivxDrmDecrypt
endif
ifeq ($(call is-board-platform,msm8226),true)
LOCAL_C_INCLUDES+=                                  \
    $(TARGET_OUT_HEADERS)/mm-video/DivxDrmDecrypt
endif

LOCAL_SHARED_LIBRARIES +=   \
    libDivxDrm

LOCAL_MODULE:= libmmparser

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
