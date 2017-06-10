# Sample usage of build/core/utils.mk
#
# How to use this .mk
#   $ source build/envsetup.sh
#   $ choosecombo
#   $ cd vendor/qcom/proprietary/common/build
#   $ make -f utils_sample_usage.mk

# Contents of this .mk
#   - initialize vars, taken from a live example
#   - qcom target specific featurization examples
#     covering all current usage
#   - qcom proprietary dependent aosp featurization examples
#   - other board,target,product featurization examples
#
# For each of featurizations categories mentioned above, we
# enlist existing usage samples and recommended usage

# TARGET_BOARD_PLATFORM now takes the value of TARGET_PRODUCT
# for all qcom targets. For example, the possible values for
# TARGET_BOARD_PLATFORM are
#
#     TARGET_BOARD_PLATFORM := msm8960
#     TARGET_BOARD_PLATFORM := msm8660
#     TARGET_BOARD_PLATFORM := msm7627a
#     TARGET_BOARD_PLATFORM := msm7627_surf
#     TARGET_BOARD_PLATFORM := msm7630_surf
#     TARGET_BOARD_PLATFORM := msm7630_fusion
#
# This will also enable usage of ro.board.platform
# at runtime for qcom target specific featurization
# instead of having to use ro.product.device which
# is overwritten by oems

include $(ANDROID_BUILD_TOP)/build/core/utils.mk

# the following vars represent values for TARGET_PRODUCT
# msm7627_surf
TARGET_PRODUCT := msm7627_surf
TARGET_DEVICE  := msm7627_surf
TARGET_BOARD_PLATFORM := msm7627_surf
TARGET_BOOTLOADER_BOARD_NAME := msm7627_surf

# When the release for qcom TARGET_PRODUCT
# msm7627_surf reaches the customer, the following
# would be an example set of TARGET_PRODUCT values
# as defined by the customer for their device
#
#   TARGET_PRODUCT := full_passion
#   TARGET_DEVICE  := passion
#   TARGET_BOARD_PLATFORM := msm7627_surf
#   TARGET_BOOTLOADER_BOARD_NAME := mahimahi
#
# And so, any qcom specific featurizations based on
# variables other than TARGET_BOARD_PLATFORM, will
# require manual changes on part of the oem to
# build and integrate. Given that, we limit our
# featurizations to using TARGET_BOARD_PLATFORM
# using the utilities, as described below.

# globals defined in device/qcom/common/common.mk
QCOM_BOARD_PLATFORMS := msm7627_surf
QCOM_BOARD_PLATFORMS += msm7627a
QCOM_BOARD_PLATFORMS += msm7630_surf
QCOM_BOARD_PLATFORMS += msm7630_fusion
QCOM_BOARD_PLATFORMS += msm8660
QCOM_BOARD_PLATFORMS += msm8960

MSM7K_BOARD_PLATFORMS := msm7630_surf
MSM7K_BOARD_PLATFORMS += msm7630_fusion
MSM7K_BOARD_PLATFORMS += msm7627_surf
MSM7K_BOARD_PLATFORMS += msm7627a
MSM7K_BOARD_PLATFORMS += msm7k

QSD8K_BOARD_PLATFORMS := qsd8k

# QCOM TARGET SPECIFIC FEATURIZATION

# The following use cases list examples on
# how to use the utils.mk to do various
# types of featurizations for qcom chipsets
# and targets

# CASE 1: build conditionally for a qcom target
#

# Existing usage
ifeq ($(TARGET_BOARD_PLATFORM),msm8660)
endif
ifeq ($(QCOM_TARGET_PRODUCT),msm8660_surf)
endif
ifeq "$(findstring msm8660,$(QCOM_TARGET_PRODUCT))" "msm8660"
endif

# Recommended usage
ifeq ($(call is-board-platform,msm7627_surf),true)
$(warning expected match)
else
$(warning unexpected result)
endif

ifeq ($(call is-not-board-platform,msm8960),true)
$(warning expected match)
else
$(warning unexpected result)
endif

# Existing Usage
ifneq (, $(filter msm7630_fusion msm7630_surf, $(QCOM_TARGET_PRODUCT)))
endif

ifneq (, $(filter msm7627_surf msm7627_ffa, $(QCOM_TARGET_PRODUCT)))
endif

# Recommended usage
ifeq ($(call is-chipset-in-board-platform,msm7627),true)
$(warning expected match)
else
$(warning unexpected result)
endif

# Existing Usage
ifeq "$(findstring msm7627,$(QCOM_TARGET_PRODUCT))" "msm7627"
endif

ifneq (, $(filter msm7627_surf msm7627a, $(QCOM_TARGET_PRODUCT)))
endif

# Recommended usage
ifeq ($(call is-chipset-prefix-in-board-platform,msm7627),true)
$(warning expected match)
else
$(warning unexpected result)
endif

#-----
# CASE 2: build conditionally for a subset of qcom targets

# Existing usage
ifneq (, $(filter msm7630_surf msm7630_1x msm8660_surf msm8660_csfb msm7630_fusion, $(QCOM_TARGET_PRODUCT)))
endif

# Recommended usage
# BOARD_PLATFORMS_LIST here is a local var to be filled
# in with appropriate subset in your .mk, using values
# from TARGET_BOARD_PLATFORM
BOARD_PLATFORMS_LIST := msm7627_surf
BOARD_PLATFORMS_LIST += msm7627a
BOARD_PLATFORMS_LIST += msm8660
BOARD_PLATFORMS_LIST += msm8960

ifeq ($(call is-board-platform-in-list,$(BOARD_PLATFORMS_LIST)),true)
$(warning expected match)
else
$(warning unexpected result)
endif

ifeq ($(call is-board-platform-in-list,$(MSM7K_BOARD_PLATFORMS)),true)
$(warning expected match)
else
$(warning unexpected result)
endif

TARGET_BOARD_PLATFORM := msm8660
ifneq ($(call is-board-platform-in-list,$(MSM7K_BOARD_PLATFORMS)),true)
$(warning expected match)
else
$(warning unexpected result)
endif
# reset target_board_platform to original value set at top
TARGET_BOARD_PLATFORM := msm7627_surf

#----
# CASE 3: build conditionally for all qcom targets

# Existing Usage
# Yes, it should wrap in 80 chars, but that's how it
# is in the original usage :)
ifneq (, $(filter msm7627a msm7627_ffa msm7627_surf msm7630_surf msm7630_1x msm7630_fusion msm8660_surf msm8960 msm8660_csfb qsd8250_ffa qsd8250_surf qsd8650a_st1x, $(QCOM_TARGET_PRODUCT)))
endif

# Recommended Usage
ifeq ($(call is-vendor-board-platform,QCOM),true)
$(warning expected match)
else
$(warning unexpected result)
endif

# TARGET_BOARD_PLATFORM is defined in BoardConfig.mk
# It's initialized here for a demo only
#
TARGET_BOARD_PLATFORM := generic
ifneq ($(call is-vendor-board-platform,QCOM),true)
$(warning expected match)
else
$(warning unexpected result)
endif
# reset target_board_platform to original value set at top
TARGET_BOARD_PLATFORM := msm7627_surf

#---
# CASE 4: build conditionally for android code name

# Existing Usage
ifeq ($(BUILD_ID),GINGERBREAD)
endif

# Recommended usage
# PLATFORM_SDK_VERSION is predefined in build/core and shouldn't
# be changed. It's being defined here only for because this is
# a standalone .mk

PLATFORM_SDK_VERSION := 9
ifeq ($(call is-android-codename,GINGERBREAD),true)
$(warning expected match)
else
$(warning unexpected result)
endif
PLATFORM_SDK_VERSION := 11
ifneq ($(call is-android-codename,GINGERBREAD),true)
$(warning expected match)
else
$(warning unexpected result)
endif

PLATFORM_SDK_VERSION := 8
ifeq ($(call is-android-codename-in-list,FROYO GINGERBREAD),true)
$(warning expected match)
else
$(warning unexpected result)
endif
ifneq ($(call is-android-codename-in-list,GINGERBREAD HONEYCOMB),true)
$(warning expected match)
else
$(warning unexpected result)
endif

# QCOM PROPRIETARY DEPENDENT AOSP FEATURISATION
# Existing usage, for featurizing external/webkit
# for additions provided by proprietary webkit
ifeq ($(strip $(QC_PROP)),true)
endif

# Recommended usage
# Declare TARGET_HAVE_QCOM_WEBKIT_ACCEL in BoardConfig.mk
ifeq ($(strip $(TARGET_HAVE_QCOM_WEBKIT_ACCEL)),true)
endif

# OTHER BOARD TARGET PRODUCT featurization examples
# Please do not do any of the following for qcom
# specific featurizations. These are examples
# of incorrect usage, and should be fixed as
# recommended.

# Existing/bad usage
ifeq ($(TARGET_PRODUCT),msm7627_surf)
endif
ifeq ($(TARGET_DEVICE),msm7627_surf)
endif
ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),msm7627_surf)
endif
ifeq ($(TARGET_DEVICE),msm8960)
endif

# Recommended usage
ifeq ($(call is-board-platform,msm8660_surf),true)
endif

# QCOM AOSP DEPENDENT PROPRIETARY FEATURIZATION
# ? (Ideally we shouldn't have any of this)

all:

# Additional documentation
#
# Target in context of build, vendor, and android
# have different meanings.
#
# 1. Target in context of build using make is a gnu
#    make 'target'
# 2. Target in context of vendor can be it's chipset
# 3. Target in context of android buildsystem is what's
#    being built (specified by variables TARGET_PRODUCT
#    and the associated variables in the product and
#    board configuration for that product)
#
# Note: The android buildsystem seems to differentiate between
# target hardware and make targets by the way in which it uses
# TARGET/target. For hardware vars naming, it's used as a prefix,
# for gnu make target names it's used as a suffix, or in lower case.
#
