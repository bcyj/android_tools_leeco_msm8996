ifeq ($(TARGET_USES_QCA_NFC),true)
NFC_D := true

PRODUCT_PACKAGES += \
    libqnfc-nci \
    libqnfc_nci_jni \
    nfc_nci.$(TARGET_BOARD_PLATFORM) \
    QNfc \
    Tag \
    GsmaNfcService \
    com.gsma.services.nfc \
    com.gsma.services.utils \
    com.gsma.services.nfc.xml \
    com.android.nfc_extras \
    com.android.qcom.nfc_extras \
    com.android.qcom.nfc_extras.xml \
    com.android.nfc.helper \
    com.android.nfc.helper.xml \
    SmartcardService \
    org.simalliance.openmobileapi \
    org.simalliance.openmobileapi.xml \
    com.vzw.nfc \
    com.vzw.nfc.xml \
    libassd

# file that declares the MIFARE NFC constant
# Commands to migrate prefs from com.android.nfc3 to com.android.nfc
# NFC access control + feature files + configuration
PRODUCT_COPY_FILES += \
        packages/apps/Nfc/migrate_nfc.txt:system/etc/updatecmds/migrate_nfc.txt \
        frameworks/native/data/etc/com.nxp.mifare.xml:system/etc/permissions/com.nxp.mifare.xml \
        frameworks/native/data/etc/com.android.nfc_extras.xml:system/etc/permissions/com.android.nfc_extras.xml \
        frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
        frameworks/native/data/etc/android.hardware.nfc.hce.xml:system/etc/permissions/android.hardware.nfc.hce.xml

ADDITIONAL_BUILD_PROPERTIES += persist.nfc.smartcard.config=SIM1,SIM2,eSE1
endif # TARGET_USES_QCA_NFC


