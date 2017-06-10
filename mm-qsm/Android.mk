# ---------------------------------------------------------------------------------
#                 QSM Interface
# ---------------------------------------------------------------------------------
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(call is-board-platform,copper),true)

ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)

LOCAL_COPY_HEADERS_TO := mm-qsm/include
LOCAL_COPY_HEADERS    := inc/IEnhancedStreamSwitchManager.h
LOCAL_COPY_HEADERS    += inc/IDataStateProvider.h
LOCAL_COPY_HEADERS    += inc/IStreamSource.h
LOCAL_COPY_HEADERS    += inc/QsmTypes.h

include $(BUILD_COPY_HEADERS)

endif  #is-board-platform
endif  #is-vendor-board-platform
