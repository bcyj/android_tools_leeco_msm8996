ifneq ($(TARGET_USES_AOSP),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#             Make the Executable isdbtmmtest
# ---------------------------------------------------------------------------------

commonIncludes := bionic \
				  bionic/libstdc++/include \
                  external/stlport/stlport \
				  external/gtest/include \
				  $(LOCAL_PATH)/inc \
                  $(LOCAL_PATH)/src \
                  $(LOCAL_PATH)/../../isdbtmm/inc \
                  $(LOCAL_PATH)/../../../../securemsm/drm/isdbtmm/inc \
                  $(LOCAL_PATH)/../../../../securemsm/tzcommon/inc \
                  $(LOCAL_PATH)/../../../../securemsm/QSEEComAPI \
                  $(TARGET_OUT_HEADERS)/common/inc \
                  $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                  $(LOCAL_PATH)/../drm/isdbtmm/inc \


commonStaticLibraries := \
                         libgtest \
                         libgtest_main \
						 libisdbtmm

commonSharedLibraries := libcutils \
                         libdl \
                         liblog \
                         libutils \
                         libstlport \
                         libz \
                         libQSEEComAPI


sourceFiles := \
               src/isdbtmm_test.cpp


LOCAL_SRC_FILES := $(sourceFiles)
LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) <=19 ))" )))
LOCAL_LDLIBS += -lpthread
endif
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE:= isdbtmmtest
LOCAL_SHARED_LIBRARIES := $(commonSharedLibraries)
LOCAL_STATIC_LIBRARIES := $(commonStaticLibraries)
LOCAL_STATIC_LIBRARIES += libqsappsver

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
