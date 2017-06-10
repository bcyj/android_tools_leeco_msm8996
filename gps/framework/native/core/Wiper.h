 /*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef WIPER_H
#define WIPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#define MAX_REPORTED_APS 50
#define MAC_ADDRESS_LENGTH 6
#define SSID_LENGTH 32

#define WIFI_APDATA_MASK_AP_RSSI 0x10
#define WIFI_APDATA_MASK_AP_CHANNEL 0x20

enum PositionSourceType
{
    GNSS = 0,
    CELLID = 1,
    ENH_CELLID = 2,
    WIFI = 3,
    TERRESTRIAL = 4,
    GNSS_TERRESTRIAL_HYBRID = 5,
    OTHER = 6
};

enum ReliabilityValue
{
    RELIABILITY_NOT_SET = 0,
    RELIABILITY_VERY_LOW = 1,
    RELIABILITY_LOW = 2,
    RELIABILITY_MEDIUM = 3,
    RELIABILITY_HIGH = 4
};

typedef struct {

    uint8_t latitudeValid;  /* Must be set to true if latitude is being passed */
    double latitude;

    uint8_t longitudeValid;  /* Must be set to true if longitude is being passed */
    double longitude;

    uint8_t horUncCircularValid;  /* Must be set to true if horUncCircular is being passed */
    float horUncCircular;

    uint8_t horConfidenceValid;  /* Must be set to true if horConfidence is being passed */
    uint8_t horConfidence;

    uint8_t horReliabilityValid;  /* Must be set to true if horReliability is being passed */
    ReliabilityValue horReliability;

    uint8_t altitudeWrtEllipsoidValid;  /* Must be set to true if altitudeWrtEllipsoid is being passed */
    float altitudeWrtEllipsoid;

    uint8_t altitudeWrtMeanSeaLevelValid;  /* Must be set to true if altitudeWrtMeanSeaLevel is being passed */
    float altitudeWrtMeanSeaLevel;

    uint8_t vertUncValid;  /* Must be set to true if vertUnc is being passed */
    float vertUnc;

    uint8_t vertConfidenceValid;  /* Must be set to true if vertConfidence is being passed */
    uint8_t vertConfidence;

    uint8_t vertReliabilityValid;  /* Must be set to true if vertReliability is being passed */
    ReliabilityValue vertReliability;

    uint8_t timestampUtcValid;  /* Must be set to true if timestampUtc is being passed */
    uint64_t timestampUtc;

    uint8_t timestampAgeValid;  /* Must be set to true if timestampAge is being passed */
    int32_t timestampAge;

    uint8_t positionSrcValid;  /* Must be set to true if positionSrc is being passed */
    PositionSourceType positionSrc;

} CoarsePositionInfo;

typedef struct {

  /*  Wifi SSID string */
  char ssid[SSID_LENGTH+1];
} WifiApSsidInfo;

typedef struct {

    int apLen;
    /* Represents whether access point len*/
    uint8_t mac_address[MAX_REPORTED_APS * MAC_ADDRESS_LENGTH];
    /*  Represents mac address of the wifi access point*/
    uint32_t rssi[MAX_REPORTED_APS];
    /*  Represents received signal strength indicator of the wifi access point*/
    uint16_t channel[MAX_REPORTED_APS];
    /*  Represents  wiFi channel on which a beacon was received of the wifi access point */

} WifiApInfo;

typedef struct {

    unsigned char positionValid;
    /* Represents info on whether position is valid */
    double latitude;
    /* Represents latitude in degrees */
    double longitude;
    /* Represents longitude in degrees */
    float accuracy;
    /* Represents expected accuracy in meters */
    uint8_t horConfidence;
    /* Represents expected hor confidence in percentage */
    int fixError;
    /* Represents Wifi position error code */
    unsigned char numApsUsed;
    /* Represents  number of Access Points (AP) used to generate a fix */
    unsigned char apInfoValid;
    /* Represents whether access point info is valid*/
    WifiApInfo apInfo;
    /* Represents access point information */
    uint8_t  wifiApSsidValid;
    /* Represents info on whether ap SSID is valid */
    uint32_t wifiApSsidInfoLen;
    /* Represents info on ap SSID length */
    WifiApSsidInfo wifiApSsidInfo[MAX_REPORTED_APS];
    /* Represent Wifi SSID string */

} WifiLocation;

typedef struct {

    int attachState;
    /* Represents whether access point attach state*/
    unsigned char apMacAddressValid;
    /* Represents info on whether ap mac address is valid */
    uint8_t apMacAddress[MAC_ADDRESS_LENGTH];
    /* Represents mac address of the wifi access point*/
    uint8_t  wifiApSsidValid;
    /* Represents info on whether ap SSID is valid */
    char ssid[SSID_LENGTH+1];
    /* Represents Wifi SSID string*/
} WifiSupplicantInfo;

/* Wifi request types from modem*/
enum WifiRequestType
{
    HIGH = 0,
    LOW = 1,
    STOP = 2,
    UNKNOWN
};

#ifdef __cplusplus
}
#endif

#endif /* WIPER_H */
