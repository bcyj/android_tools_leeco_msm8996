ifeq ($(call is-vendor-board-platform,QCOM),true)
REMOTEFS_DIR := $(call my-dir)

RMTFS_RPC_LIST := msm7627a
RMTFS_RPC_LIST += msm8625
RMTFS_RPC_LIST += msm7630_surf
RMTFS_RPC_LIST += msm8660_surf

ifeq ($(call is-board-platform-in-list,$(RMTFS_RPC_LIST)),true)
	include $(REMOTEFS_DIR)/rpc_rmt_storage/Android.mk
else
	include $(REMOTEFS_DIR)/qmi_rmt_storage/Android.mk
endif
endif #(is-vendor-board-platform,QCOM)
