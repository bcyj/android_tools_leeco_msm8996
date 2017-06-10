# primitive utility to test build/core/utils.mk
#
# How to use
#   $ source build/envsetup.sh
#   $ choosecombo
#   $ cd vendor/qcom/proprietary/common/build
#   $ make -f utils_test.mk

include $(ANDROID_BUILD_TOP)/build/core/utils.mk

TARGET_BOARD_PLATFORM := msm7627_surf
TARGET_BOARD_PLATFORM_A := msm7627a
MSM7K_PLATFORMS := msm7627_surf msm7627a msm7630_fusion

PRODUCT_LIST = msm7627_surf
PRODUCT_LIST += msm7627a
PRODUCT_LIST += msm7630_surf
PRODUCT_LIST += msm7630_fusion
PRODUCT_LIST += msm8660_surf
PRODUCT_LIST += msm8660_csfb
PRODUCT_LIST += msm8960

QCOM_BOARD_PLATFORMS := $(PRODUCT_LIST)

$(warning word TARGET_BOARD_PLATFORM $(TARGET_BOARD_PLATFORM))
$(warning word TARGET_BOARD_PLATFORM_A $(TARGET_BOARD_PLATFORM_A))
$(warning list MSM7K_PLATFORMS $(MSM7K_PLATFORMS))

# test match-word
RESULT := $(call match-word," ","  ")
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call match-word,"qsd7627a",qsd7627a)
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call match-word,"qsd7627a","qsd7627a  ")
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call match-word,"qsd7627a","qsd7627a")
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call match-word,"\n"," ")
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call match-word,"qsd7627a","qsd7627a msm8660")
$(warning expected result --, result -$(RESULT)-)

# test match-word-in-list
RESULT := $(call match-word-in-list,"  msm7627a",$(MSM7K_PLATFORMS))
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call match-word-in-list,msm7627a,$(MSM7K_PLATFORMS))
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call match-word-in-list,   msm7627a ,$(MSM7K_PLATFORMS))
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call match-word-in-list,$(TARGET_BOARD_PLATFORM),$(MSM7K_PLATFORMS))
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call match-word-in-list,qsd7627a,$(MSM7K_PLATFORMS))
$(warning expected result --, result -$(RESULT)-)

# test find-word-in-list
RESULT := $(call find-word-in-list,qsd7627a,$(MSM7K_PLATFORMS))
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call find-word-in-list,msm7627_surf,$(MSM7K_PLATFORMS))
$(warning expected result -msm7627_surf-, result -$(RESULT)-)
RESULT := $(call find-word-in-list,msm7627a,$(MSM7K_PLATFORMS))
$(warning expected result -msm7627a-, result -$(RESULT)-)

# test match-prefix
RESULT := $(call match-prefix,msm7627,_,msm7627_surf msm7627a msm8960)
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call match-prefix,msm7627,a,msm7627_surf msm7627a msm8960)
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call match-prefix,msm7627,a,msm7627_surf msm8960)
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call match-prefix,msm8660,,msm7627_surf msm7627a msm8960)
$(warning expected result --, result --)
RESULT := $(call match-prefix,msm8660,,)
$(warning expected result --, result --)
RESULT := $(call match-prefix, ,,msm7627_surf msm7627a msm8960)
$(warning expected result --, result --)

# test get-vendor-board-platforms
XYZ_BOARD_PLATFORMS := msm8960 msm7627a msm7627_surf
RESULT := $(call get-vendor-board-platforms,XYZ)
$(warning expected result -$(XYZ_BOARD_PLATFORMS)-, result -$(RESULT)-)
XYZ_BOARD_PLATFORMS := msm8960 msm7627a
RESULT := $(call get-vendor-board-platforms,PQR)
$(warning expected result --, result -$(RESULT)-)

# test is-board-platform
RESULT := $(call is-board-platform,qsd7627a)
$(warning expected result --, result -$(RESULT)-)
RESULT := $(call is-board-platform,msm7627_surf)
$(warning expected result -true-, result -$(RESULT)-)

# test is-not-board-platform
RESULT := $(call is-not-board-platform,qsd7627a)
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call is-not-board-platform,msm7627_surf)
$(warning expected result --, result -$(RESULT)-)

# test is-board-platform-in-list
RESULT := $(call is-board-platform-in-list,$(MSM7K_PLATFORMS))
$(warning expected result -true-, result -$(RESULT)-)
TARGET_BOARD_PLATFORM := msm8660
RESULT := $(call is-board-platform-in-list,$(MSM7K_PLATFORMS))
$(warning expected result --, result -$(RESULT)-)
# reset target_board_platform to original value set at top
TARGET_BOARD_PLATFORM := msm7627_surf

# test is-vendor-board-platform
RESULT := $(call is-vendor-board-platform,QCOM)
$(warning expected result -true-, result -$(RESULT)-)
TARGET_BOARD_PLATFORM := msm8660
RESULT := $(call is-vendor-board-platform,QCOM)
$(warning expected result --, result -$(RESULT)-)
# reset target_board_platform to original value set at top
TARGET_BOARD_PLATFORM := msm7627_surf

# test is-chipset-in-board-platform
RESULT := $(call is-chipset-in-board-platform,msm7627)
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call is-chipset-in-board-platform,msm8960)
$(warning expected result --, result -$(RESULT)-)

# test is-chipset-prefix-in-board-platform
RESULT := $(call is-chipset-prefix-in-board-platform,msm7627)
$(warning expected result -true-, result -$(RESULT)-)
TARGET_BOARD_PLATFORM := msm7627a
RESULT := $(call is-chipset-prefix-in-board-platform,msm7627)
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call is-chipset-prefix-in-board-platform,msm8960)
$(warning expected result --, result -$(RESULT)-)
TARGET_BOARD_PLATFORM := msm7627_surf

# test is-android-codename
PLATFORM_SDK_VERSION := 9
RESULT := $(call is-android-codename,GINGERBREAD)
$(warning expected result -true-, result -$(RESULT)-)
PLATFORM_SDK_VERSION := 11
RESULT := $(call is-android-codename,GINGERBREAD)
$(warning expected result --, result -$(RESULT)-)

# test is-android-codename-in-list
PLATFORM_SDK_VERSION := 8
RESULT := $(call is-android-codename-in-list,FROYO GINGERBREAD)
$(warning expected result -true-, result -$(RESULT)-)
RESULT := $(call is-android-codename-in-list,GINGERBREAD HONEYCOMB)
$(warning expected result --, result -$(RESULT)-)

all:
