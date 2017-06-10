# Add ChinaTelecom Apps
PRODUCT_PACKAGES += \
    Mail189 \
    CtWallpaper \
    CtLauncherRes \
    CtBrowserQuick \
    CtBrowserRes \
    CtWallpaper \
    com.qrd.ctuseragent \
    com.qrd.ctuseragent.xml \
    CtStkRes \
    CtEmailRes \
    CtFrameworksRes \
    CtMultisimSettingsRes \
    CtTeleServiceRes \
    CtMmsRes \
    CtSettingsRes \
    CtSettingsProviderRes \
    CtDialerRes \
    CtSystemUIRes \
    CtPhoneFeaturesRes \
    CtKeyguardRes \
    CtRoamingSettings \
    CtUniversalDownload \
    CtTelephonyProviderRes \
    AutoRegistration \
    CtTrebuchetRes \
    CtSimContactsRes \
    CtCalLocalAccountRes

# Marked as couldn't build sucess now.
#PRODUCT_PACKAGES += \
#    ApnSettings \
#    CustomerService \

LOCAL_PATH := vendor/qcom/proprietary/qrdplus/ChinaTelecom

# include the BootAnimation's products
-include $(LOCAL_PATH)/apps/BootAnimation/products.mk

# include the Cacerts's products
-include $(LOCAL_PATH)/apps/CaCerts/products.mk
