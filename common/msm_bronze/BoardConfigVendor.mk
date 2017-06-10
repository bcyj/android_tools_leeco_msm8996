# BoardConfigVendor.mk
# Qualcomm Technologies proprietary product specific compile-time definitions.

BOARD_USES_GENERIC_AUDIO := true
USE_CAMERA_STUB := false

BOARD_USES_QCOM_HARDWARE := true
BOARD_USES_ADRENO := true
HAVE_ADRENO_SOURCE := true
HAVE_ADRENO_SC_SOURCE := true
HAVE_ADRENO_FIRMWARE := true
DYNAMIC_SHARED_LIBV8SO := true
#ENABLE_WEBGL := true
#BOARD_USE_QCOM_LLVM_CLANG_RS := true
TARGET_USES_ION := true
#USE_OPENGL_RENDERER := true
#TARGET_USES_C2D_COMPOSITION := true
#BOARD_USES_ALSA_AUDIO := true
#TARGET_USES_QCOM_MM_AUDIO := true
#MM_AUDIO_OMX_ADEC_EVRC_DISABLED := true
MM_AUDIO_OMX_ADEC_QCELP13_DISABLED := true
#MM_AUDIO_FTM_DISABLED := false
#MM_AUDIO_MEASUREMENT_DISABLED := true
#MM_AUDIO_VOEM_DISABLED := true
#MM_AUDIO_SRS_DISABLED := false
#BOARD_USES_SRS_TRUEMEDIA := true
#BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := default
#BOARD_USES_QCNE := true
TARGET_USES_ASHMEM := true
TARGET_USES_SECURITY_BRIDGE := true
TARGET_USES_POST_PROCESSING := true
#DOLBY_DAP := true

BOARD_DISK_ANDROID_IMG := true

ifneq ($(BUILD_TINY_ANDROID), true)
  #BOARD_HAS_QCOM_WLAN := true
  #BOARD_HAS_ATH_WLAN_AR6004 := true
  #BOARD_HAVE_BLUETOOTH := true
  #BOARD_HAS_QCA_BT_AR3002 := true
  #BOARD_HAVE_QCOM_FM := true
  #BOARD_ANT_WIRELESS_DEVICE := "vfs-prerelease"

  ifeq ($(BOARD_HAVE_BLUETOOTH), true)
    # Comment the following flag to enable bluedriod
    BOARD_HAVE_BLUETOOTH_BLUEZ := true
    ifneq ($(BOARD_HAVE_BLUETOOTH_BLUEZ), true)
      BOARD_HAVE_BLUETOOTH_QCOM := true
      QCOM_BT_USE_SMD_TTY := true
    endif # BOARD_HAVE_BLUETOOTH_BLUEZ
  endif # BOARD_HAVE_BLUETOOTH

  ifeq ($(findstring true,$(BOARD_HAS_ATH_WLAN_AR6004) $(BOARD_HAS_QCOM_WLAN)),true)
    BOARD_WLAN_DEVICE := qcwcn
    BOARD_WPA_SUPPLICANT_DRIVER := NL80211
    BOARD_HOSTAPD_DRIVER := NL80211
    WIFI_DRIVER_MODULE_PATH := "/system/lib/modules/wlan.ko"
    WIFI_DRIVER_MODULE_NAME := "wlan"
    WIFI_DRIVER_MODULE_ARG := ""
    WPA_SUPPLICANT_VERSION := VER_0_8_X
    HOSTAPD_VERSION := VER_0_8_X
    WIFI_DRIVER_FW_PATH_STA := "sta"
    WIFI_DRIVER_FW_PATH_AP  := "ap"
    WIFI_DRIVER_FW_PATH_P2P := "p2p"
    ifeq ($(call is-platform-sdk-version-at-least,17),true)
      # JB MR1 or later
      BOARD_HAS_CFG80211_KERNEL3_4 := true
      # KitKat
      ifeq ($(call is-platform-sdk-version-at-least,19),true)
        BOARD_HAS_CFG80211_KERNEL3_4 := false
        BOARD_HAS_CFG80211_KERNEL3_10 := true
      endif
      BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
      BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
    else
      # JB or earlier
      WIFI_CFG80211_DRIVER_MODULE_PATH := "/system/lib/modules/cfg80211.ko"
      WIFI_CFG80211_DRIVER_MODULE_NAME := "cfg80211"
      WIFI_CFG80211_DRIVER_MODULE_ARG  := ""
      WIFI_DRIVER_DEF_CONF_FILE := "/persist/WCNSS_qcom_cfg.ini"
      WIFI_DRIVER_CONF_FILE := "/data/misc/wifi/WCNSS_qcom_cfg.ini"
    endif
  endif
endif   # !BUILD_TINY_ANDROID
