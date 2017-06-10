LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(TARGET_OUT_HEADERS)/common/inc
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils
LOCAL_MODULE := qfipsverify
LOCAL_SRC_FILES := fips.c
ifeq ($(FIPS_ENABLED),true)
	LOCAL_CFLAGS += -DFIPSENABLED
endif # FIPS_ENABLED
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

HMAC_KEY := b617318655057264e28bc0b6fb378c8ef146be
BOOTIMG_BINARY := $(TARGET_PREBUILT_KERNEL)
QFIPSVERYFY_BINARY := $(PRODUCT_OUT)/system/bin/qfipsverify

HMAC := $(PRODUCT_OUT)/system/usr/qfipsverify/bootimg.hmac
$(HMAC): $(TARGET_PREBUILT_KERNEL)
	$(shell mkdir -p $(PRODUCT_OUT)/system/usr/qfipsverify)
	$(shell openssl dgst -sha256 -mac HMAC -macopt hexkey:$(HMAC_KEY) $(BOOTIMG_BINARY) | awk '{ print $$2 }'  > $(HMAC))

ALL_DEFAULT_INSTALLED_MODULES += $(HMAC)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(HMAC)

HMAC_QFIPSVERIFY := $(PRODUCT_OUT)/system/usr/qfipsverify/qfipsverify.hmac
$(HMAC_QFIPSVERIFY): $(PRODUCT_OUT)/system/bin/qfipsverify
	$(shell mkdir -p $(PRODUCT_OUT)/system/usr/qfipsverify)
	$(shell openssl dgst -sha256 -mac HMAC -macopt hexkey:$(HMAC_KEY) $(QFIPSVERYFY_BINARY) | awk '{ print $$2 }'  > $(HMAC_QFIPSVERIFY))

ALL_DEFAULT_INSTALLED_MODULES += $(HMAC_QFIPSVERIFY)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(HMAC_QFIPSVERIFY)
