# ---------------------------------------------------------------------------------
#                 Source
# ---------------------------------------------------------------------------------
ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

LOCAL_COPY_HEADERS_TO := mm-http/include
LOCAL_COPY_HEADERS := inc/DASHMediaPeriodHandler.h
LOCAL_COPY_HEADERS += inc/DASHMediaPlayGroup.h
LOCAL_COPY_HEADERS += inc/DASHMediaRepresentationHandler.h
LOCAL_COPY_HEADERS += inc/DASHMediaSegmentHandler.h
LOCAL_COPY_HEADERS += inc/HTTPAPI.h
LOCAL_COPY_HEADERS += inc/HTTPBandwidthEstimator.h
LOCAL_COPY_HEADERS += inc/HTTPBase.h
LOCAL_COPY_HEADERS += inc/HTTPCmdQueue.h
LOCAL_COPY_HEADERS += inc/HTTPCommon.h
LOCAL_COPY_HEADERS += inc/HTTPController.h
LOCAL_COPY_HEADERS += inc/HTTPControllerHelper.h
LOCAL_COPY_HEADERS += inc/HTTPDASHAdaptor.h
LOCAL_COPY_HEADERS += inc/HTTPDataInterface.h
LOCAL_COPY_HEADERS += inc/HTTPDataManager.h
LOCAL_COPY_HEADERS += inc/HTTPDataStoreBase.h
LOCAL_COPY_HEADERS += inc/HTTPDiagInterfaceHandler.h
LOCAL_COPY_HEADERS += inc/HTTPDownloadHelper.h
LOCAL_COPY_HEADERS += inc/HTTPDownloader.h
LOCAL_COPY_HEADERS += inc/HTTPHeapManager.h
LOCAL_COPY_HEADERS += inc/HTTPResolver.h
LOCAL_COPY_HEADERS += inc/HTTPResource.h
LOCAL_COPY_HEADERS += inc/HTTPResourceManager.h
LOCAL_COPY_HEADERS += inc/HTTPSegmentDataStoreContainer.h
LOCAL_COPY_HEADERS += inc/HTTPSegmentDataStoreManager.h
LOCAL_COPY_HEADERS += inc/HTTPSegmentDataStoreStructs.h
LOCAL_COPY_HEADERS += inc/HTTPSessionInfo.h
LOCAL_COPY_HEADERS += inc/HTTPSource.h
LOCAL_COPY_HEADERS += inc/MPDParser.h
LOCAL_COPY_HEADERS += inc/PlaylistDownloadHelper.h
LOCAL_COPY_HEADERS += inc/PlaylistDownloader.h
LOCAL_COPY_HEADERS += inc/PlaylistParser.h
LOCAL_COPY_HEADERS += inc/SegmentDownloader.h
LOCAL_COPY_HEADERS += inc/httpInternalDefs.h

LOCAL_CFLAGS :=             \
    -D_ANDROID_

LOCAL_SRC_FILES += src/HTTPBandwidthEstimator.cpp
LOCAL_SRC_FILES += src/HTTPCmdQueue.cpp
LOCAL_SRC_FILES += src/HTTPCommon.cpp
LOCAL_SRC_FILES += src/HTTPController.cpp
LOCAL_SRC_FILES += src/HTTPControllerHelper.cpp
LOCAL_SRC_FILES += src/HTTPDataManager.cpp
LOCAL_SRC_FILES += src/HTTPDataStoreBase.cpp
LOCAL_SRC_FILES += src/HTTPDiagInterfaceHandler.cpp
LOCAL_SRC_FILES += src/HTTPDownloader.cpp
LOCAL_SRC_FILES += src/HTTPDownloadHelper.cpp
LOCAL_SRC_FILES += src/HTTPSessionInfo.cpp
LOCAL_SRC_FILES += src/PlaylistParser.cpp
LOCAL_SRC_FILES += src/PlaylistDownloadHelper.cpp
LOCAL_SRC_FILES += src/PlaylistDownloader.cpp
LOCAL_SRC_FILES += src/HTTPSegmentDataStoreStructs.cpp
LOCAL_SRC_FILES += src/HTTPSegmentDataStoreManager.cpp
LOCAL_SRC_FILES += src/HTTPSegmentDataStoreContainer.cpp
LOCAL_SRC_FILES += src/HTTPResolver.cpp
LOCAL_SRC_FILES += src/HTTPHeapManager.cpp
LOCAL_SRC_FILES += src/DASHMediaPeriodHandler.cpp
LOCAL_SRC_FILES += src/DASHMediaPlayGroup.cpp
LOCAL_SRC_FILES += src/DASHMediaRepresentationHandler.cpp
LOCAL_SRC_FILES += src/DASHMediaSegmentHandler.cpp
LOCAL_SRC_FILES += src/HTTPDashAdaptor.cpp
LOCAL_SRC_FILES += src/HTTPResource.cpp
LOCAL_SRC_FILES += src/HTTPResourceManager.cpp
LOCAL_SRC_FILES += src/MPDParser.cpp
LOCAL_SRC_FILES += src/SegmentDownloader.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Protocol/HTTP/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../MMI/HTTP/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/StreamUtils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/Network/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/PVXParser/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Common/SDPParser/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/HTTPSource/MAPI08/API
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/MP3ParserLib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/ID3Lib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/SeekLib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/SeekTableLib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/FileBaseLib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/FileSource/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/MP2ParserLib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/SIDXParserLib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/VideoFMTReaderLib/inc
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-parser/ISOBaseFileLib/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-qsm/include
LOCAL_C_INCLUDES += $(TOP)/external/tinyxml
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/mmi
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += libtinyxml
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += libmmparser
LOCAL_SHARED_LIBRARIES += libmmipstreamnetwork
LOCAL_SHARED_LIBRARIES += libmmipstreamutils
LOCAL_SHARED_LIBRARIES += libmmhttpstack
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libmmipstreamsourcehttp

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

