LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO   := common/inc
LOCAL_COPY_HEADERS      := ../inc/armasm.h
LOCAL_COPY_HEADERS      += ../inc/comdef.h
LOCAL_COPY_HEADERS      += ../inc/common_log.h
LOCAL_COPY_HEADERS      += ../inc/customer.h
LOCAL_COPY_HEADERS      += ../inc/msm_ipc.h
LOCAL_COPY_HEADERS      += ../inc/rex.h
LOCAL_COPY_HEADERS      += ../inc/stringl.h
LOCAL_COPY_HEADERS      += ../inc/target.h
LOCAL_COPY_HEADERS      += ../inc/qsocket.h
LOCAL_COPY_HEADERS      += ../inc/qsocket_ipcr.h

include build/core/copy_headers.mk
