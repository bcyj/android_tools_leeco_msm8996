LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MODEM_CONGIF_FILE := mcfg_sw.mbn
MODEM_CONGIF_FOLDER := $(TARGET_OUT)/vendor/LatamOpenAMX/data/modem_config
$(shell mkdir -p $(MODEM_CONGIF_FOLDER))
$(shell cp -r $(LOCAL_PATH)/$(MODEM_CONGIF_FILE) $(MODEM_CONGIF_FOLDER)/$(MODEM_CONGIF_FILE))
