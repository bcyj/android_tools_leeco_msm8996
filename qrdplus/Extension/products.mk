#Add the common apps.
PRODUCT_PACKAGES += \
    FileExplorer \
    DeviceInfo \
    init.carrier.rc \
    HomeLocation \
    LEDFlashlight \
    StopTimer \
    WorldClock \
    LunarInfoProvider \
    CalendarWidget \
    DataMonitor \
    AGPSTestMode \
    SimContacts \
    TouchPal_Global \
    ArabicPack \
    BengaliPack \
    CangjiePack \
    ChtPack \
    HindiPack \
    IndonesianPack \
    MarathiPack \
    PortuguesebrPack \
    RussianPack \
    SpanishLatinPack \
    TagalogPack \
    TamilPack \
    TeluguPack \
    ThaiPack \
    VietnamPack \
    TouchPal_V5.5.0_59680_Global \
    BatteryGuruSystemApp \
    CalendarLocalAccount \
    smartsearch \
    com.qualcomm.qti.smartsearch.xml \
    qcril.db \
    libapkscanner \
    CTAFrameworksRes \
    CTATeleServiceRes \
    CTASettingsRes \
    CTASettingsProviderRes \
    CTASystemUIRes \
    CTACamera2Res \
    CTAMmsRes \
    CTABrowserRes\
    CarrierConfigure \
    CarrierLoadService \
    CarrierCacheService \
    PhoneFeatures \
    com.qrd.wappush \
    com.qrd.wappush.xml \
    OmaDownload \
    GestureMgr \
    gestureservice \
    wrapper-updater \
    TimerSwitch \
    MSDC_UI \
    QAS_DVC_MSP \
    EngineerTool \
    EngineerToolOp \
    CTASimContactsRes \
    libmsp

ifeq ($(call is-board-platform,msm8610),true)
PRODUCT_PACKAGES += \
   Launcher2LayoutRes
endif
