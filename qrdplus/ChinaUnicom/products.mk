# Add ChinaUnicom Apps
PRODUCT_PACKAGES += \
    CuBrowserRes \
    CuLauncherRes \
    GsmTuneAway \
    com.qrd.cuuseragent \
    com.qrd.cuuseragent.xml \
    CuWallpaper \
    CuStkRes \
    CuFrameworksRes \
    CuSettingsRes \
    CuSettingsProviderRes \
    CuSystemUIRes \
    CuGallery2Res \
    CuBrowserQuick \
    CuKeyguardRes \
    CuTeleServiceRes \
    CuTrebuchetRes \
    CuSimContactsRes \
    CuCalLocalAccountRes

# Marked as couldn't build sucess now.
#PRODUCT_PACKAGES += \
#    WoRead

# CSVT
PRODUCT_PACKAGES += \
    videocallapi \
    videocallapi.xml \
    com.qti.videocall.permissions.xml \
    VideoCall \
    vtremoteservice \
    libcamerahandler_jni \
    libqcom_omx \
    libomx_sharedlibrary \
    libomx_amrenc_sharedlibrary \
    libomx_amrdec_sharedlibrary \
    libcsvt_jni \
    libvt_engine \
    qcom.cfg

LOCAL_PATH := vendor/qcom/proprietary/qrdplus/ChinaUnicom

# include the BootAnimation's products
-include $(LOCAL_PATH)/apps/BootAnimation/products.mk
