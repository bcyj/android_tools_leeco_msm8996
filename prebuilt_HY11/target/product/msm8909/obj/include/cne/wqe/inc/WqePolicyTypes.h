#ifndef _WQE_POLICY_TYPES_H
#define _WQE_POLICY_TYPES_H

/**----------------------------------------------------------------------------
@file WqePolicyTypes.h


-----------------------------------------------------------------------------*/


/*=============================================================================
           Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
           All Rights Reserved.
           Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*=============================================================================
EDIT HISTORY FOR MODULE

when        who     what, where, why
----------  ---     ---------------------------------------------------------
01-30-2012  mtony   First revision.
=============================================================================*/


/*----------------------------------------------------------------------------
* Include Files
* -------------------------------------------------------------------------*/
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "CneDefs.h"

/*----------------------------------------------------------------------------
* Preprocessor Definitions and Constants
* -------------------------------------------------------------------------*/

#define SWIM_MODULES_CFG_UPD_MASK           0x0002
#define SWIM_WIFI_CFG_UPD_MASK              0x0004
#define SWIM_LINK_PATH_CFG_UPD_MASK           0x0008
#define SWIM_IF_MANAGER_CFG_UPD_MASK         0x0020
#define SWIM_CQE_CFG_UPD_MASK               0x0040
#define SWIM_IF_SELECTION_CFG_UPD_MASK         0x0080
#define CNE_TQE_CFG_UPD_MASK               0x0200

//Update this mask upon adding or deleting above defined masks
#define SWIM_ANY_UPD_MASK             0x00FF

/* Default values */
#define SWIM_DEFAULT_WIFI_STATE                true
#define SWIM_DEFAULT_HYSTERESIS_TIMER        10 //secs
#define SWIM_DEFAULT_LINK_PATH_HISTORY        365 //days
#define SWIM_DEFAULT_T_PASSIVE_BQE_DURATION            5 //msecs
#define SWIM_DEFAULT_T_BLACKLIST_BQE        900 //secs (15 min)
#define SWIM_DEFAULT_T_BLACKLIST_ICD        900 //secs
#define SWIM_DEFAULT_WLAN_POLL_INTERVAL            60 //msecs
#define SWIM_DEFAULT_WLAN_BURST_DURATION            2 // multiple of poll intervals
#define SWIM_DEFAULT_WLAN_TOP_N                            4 // top N sample elements for passive BQE bitrate
#define SWIM_DEFAULT_WWAN_POLL_INTERVAL            70 //msecs
#define SWIM_DEFAULT_WWAN_BURST_DURATION            2 // multiple of poll intervals
#define SWIM_DEFAULT_WWAN_TOP_N                            4 // top N sample elements for passive BQE bitrate
#define SWIM_DEFAULT_ALPHA                            0.8 // alpha factor for exponential averaging for active BQE
#define SWIM_DEFAULT_MBW                                40000000 // bps
#define SWIM_DEFAULT_MSS                                1360 // bytes
#define SWIM_DEFAULT_RTT                                250 // s
#define SWIM_DEFAULT_CONG_TO_SLOW                1
#define SWIM_DEFAULT_NUM_PARALLEL_STREAM 1
#define SWIM_DEFAULT_BQE_PAST_SIZE            7
#define SWIM_DEFAULT_BQE_VALIDITY_SHORT_TIMER        10800 // s
#define SWIM_DEFAULT_BQE_GOOD_VALIDITY    10080 // minutes
#define SWIM_DEFAULT_BQE_BAD_VALIDITY    1440 // minutes
#define SWIM_DEFAULT_BQE_ACTIVE_BQE_DELAY 0 //milli seconds
#define SWIM_DEFAULT_BQE_ACTIVE_BQE_URI "http://cne.qualcomm.com/cne/v1/bqe/traffic"
#define SWIM_DEFAULT_BQE_ACTIVE_BQE_POSTURI "https://cne-post.qualcomm.com/cne/v1/bqe/findings"
#define SWIM_DEFAULT_ICD_URI "https://cne-ssl.qualcomm.com/cne/v1/icd/wifi-data"
#define SWIM_DEFAULT_ICD_HTTP_TURI "http://cne.qualcomm.com/cne/v1/icd"
#define SWIM_DEFAULT_ICD_CONFIG_TYPE "disabled"
#define SWIM_DEFAULT_RSSI_ADD_THRESHOLD            -86 // dbm
#define SWIM_DEFAULT_RSSI_DROP_THRESHOLD    -91 // dbm
#define SWIM_DEFAULT_RSSI_MODEL_THRESHOLD    -82 // dbm
#define SWIM_DEFAULT_RSSI_MAC_TIMER_THRESHOLD   -91 // dbm
#define SWIM_DEFAULT_RSSI_AVERAGING_INTERVAL    5 //secs
#define SWIM_DEFAULT_CQE_PERIODIC_TIMER        2 //secs
                                                 //
#define SWIM_MAX_RSSI_AVERAGING_INTERVAL    3600 //secs
#define SWIM_MAX_CQE_PERIODIC_TIMER         3600 //secs
#define SWIM_MIN_RSSI_AVERAGING_INTERVAL    1 //secs
#define SWIM_MIN_CQE_PERIODIC_TIMER         0 //secs
                                                 //
#define SWIM_DEFAULT_MAC_HYSTERESIS_TIMER    20 //secs
#define SWIM_DEFAULT_MAC_STATS_AVERAGING_ALPHA  (float)0.3 //Acceptable range [0, 1]
#define SWIM_DEFAULT_FRAME_COUNT_THRESHOLD    2//frames
#define SWIM_DEFAULT_COLD_START_THRESHOLD    2//frames, Acceptable range [0, 1000]
#define SWIM_DEFAULT_MAC_MIB_THRESHOLD_2A       (float)0.35
#define SWIM_DEFAULT_MAC_MIB_THRESHOLD_2B       (float)0.4
#define SWIM_DEFAULT_RETRY_METRIC_WEIGHT_2A     (float)0.2061 //Acceptable range [-1, 1]
#define SWIM_DEFAULT_RETRY_METRIC_WEIGHT_2B     (float)0.4318 //Acceptable range [-1, 1]
#define SWIM_DEFAULT_MULTI_RETRY_METRIC_WEIGHT_2A  (float)0.1984 //Acceptable range [-1, 1]
#define SWIM_DEFAULT_MULTI_RETRY_METRIC_WEIGHT_2B  (float)0.4156 //Acceptable range [-1, 1]
#define SWIM_DEFAULT_BPS_THRESHOLD            0

#define SWIM_DEFAULT_RMP_THR                  0.3 //Acceptable range [0, 1]
#define SWIM_DEFAULT_RMP_CNT_THR              10 //Acceptable range [0, 65k]
#define SWIM_DEFAULT_RX_MCS_THR               0 //Acceptable range [0, 10]
#define SWIM_DEFAULT_RX_BW_THR                0 //Acceptable range [0, 3]
#define SWIM_DEFAULT_TMD_THR                  0.1 //Acceptable range [0, 1]
#define SWIM_DEFAULT_TMD_CNT_THR              2 //Acceptable range [0, 65k]
#define SWIM_DEFAULT_TMR_THR                  0.7 //Acceptable range [0, 1]
#define SWIM_DEFAULT_TMR_CNT_THR              4 //Acceptable range [0, 65k]
#define SWIM_DEFAULT_TX_MCS_THR               0 //Acceptable range [0, 10]
#define SWIM_DEFAULT_TX_BW_THR                0 //Acceptable range [0, 3]
#define SWIM_DEFAULT_SUBRAT_CDMA              100000
#define SWIM_DEFAULT_SUBRAT_EVDO_0            200000
#define SWIM_DEFAULT_SUBRAT_EVDO_A            400000
#define SWIM_DEFAULT_SUBRAT_EVDO_B            600000
#define SWIM_DEFAULT_SUBRAT_EDGE              100000
#define SWIM_DEFAULT_SUBRAT_UMTS              200000
#define SWIM_DEFAULT_SUBRAT_HSPA              600000
#define SWIM_DEFAULT_SUBRAT_HSDPA             600000
#define SWIM_DEFAULT_SUBRAT_GPRS              50000
#define SWIM_DEFAULT_SUBRAT_LTE               1000000
#define SWIM_DEFAULT_SUBRAT_EHRPD             600000
#define SWIM_DEFAULT_SUBRAT_HSPAP             600000

#define SWIM_WIFI_VALID_CHANNEL_NUM_MIN        1
#define SWIM_WIFI_VALID_CHANNEL_NUM_MAX        13

#define SWIM_ACTIVE_BQE_URI_STRLEN          100 + 1 //including null character.
#define SWIM_POST_BQE_URI_STRLEN            100 + 1 //including null character.
#define SWIM_ICD_URI_STRLEN                          100 + 1 //including null character.

#define SWIM_RAT_SUBTYPE_MAX                17

/* INET-related macros */
#define SWIM_IN_ARE_ADDR_EQUAL(a, b)        (a.s_addr == b.s_addr)
#define SWIM_INET_ADDRSTRLEN                17 //including null character.

static const bool DEFAULT_ICD_DISABLED=true;
static const uint8_t DEFAULT_ICD_PAST_SIZE=7;
static const uint32_t DEFAULT_ICD_VALIDITY_TIMEOUT_SEC=3600;
static const uint32_t DEFAULT_ICD_TRANSACTION_TIMEOUT_SEC=4;
static const float DEFAULT_ICD_HIGH_PROBABILITY= (float)0.05;
static const uint32_t DEFAULT_ICD_MAX_AUTH_TIME=60;
static const uint32_t DEFAULT_ICD_RETEST_TIME=3;

static const bool DEFAULT_TQE_DISABLED=true;
static const uint16_t DEFAULT_TQE_DNS_BAD_THRESHOLD = 20000; //msecs
static const uint16_t DEFAULT_TQE_SOCK_ACTIVE_THRESH = 3000; //msecs
static const uint16_t DEFAULT_TQE_DGIM_THRESH = 400; //msecs
static const uint32_t DEFAULT_TQE_TPUT_THRESH = 1000000; //bps
static const uint16_t DEFAULT_TQE_TIME_WINDOW = 20; //sec
static const uint16_t DEFAULT_TQE_COUNT_THRESH = 2;
static const uint16_t DEFAULT_TQE_OVERALL_COUNT_THRESH = 2; //samples
static const uint16_t DEFAULT_T_BLACKLIST_TQE = 3600; //secs (60 min)
static const float DEFAULT_TQ_THRESH = (float)0.15;
static const float DEFAULT_TQE_RATIO_THRESH = (float)0.2;
static const float DEFAULT_TQE_ALPHA = (float)0.8;

/*----------------------------------------------------------------------------
* Types
* -------------------------------------------------------------------------*/

/* BQE parameters */
typedef struct SwimBqeParametersType
{
    SwimBqeParametersType()
    {
        bpsThreshold = SWIM_DEFAULT_BPS_THRESHOLD;
    }
    uint32_t bpsThreshold;
} SwimBqeParametersType;


/*---------------------------------------------------------------------------
* Type Description: A structure to mantain configuration related to
*                     WiFiManager module of NIMS class.
*-------------------------------------------------------------------------*/
struct SwimWiFiMgrConfigType
{
    SwimWiFiMgrConfigType()
    {
        DefaultState = SWIM_DEFAULT_WIFI_STATE;
    }

    /* A default Power ON or OFF state of WLAN. false = OFF, true = ON */
    bool DefaultState;

    bool operator== (SwimWiFiMgrConfigType &cfg1)
    {
        return ((*this).DefaultState == cfg1.DefaultState);
    }

    bool operator!= ( SwimWiFiMgrConfigType &cfg1)
    {
        return !(*this == cfg1);
    }

};

/*---------------------------------------------------------------------------
* Type Description: A structure to mantain configuration related to
*                     LinkPathManager module of NIMS class.
*
*-------------------------------------------------------------------------*/
struct SwimLinkPathMgrConfigType
{
    SwimLinkPathMgrConfigType()
    {
        LinkPathHistory = SWIM_DEFAULT_LINK_PATH_HISTORY;
    }

    /* The number of days till the history of LinkPath module needs
    be maintained. */
    uint16_t LinkPathHistory;

    bool operator== ( SwimLinkPathMgrConfigType &cfg1)
    {
        return ((*this).LinkPathHistory == cfg1.LinkPathHistory);
    }

    bool operator!= ( SwimLinkPathMgrConfigType &cfg1)
    {
        return !(*this == cfg1);
    }
};

/*---------------------------------------------------------------------------
* Type Description: A structure to mantain configuration related to
*                     Interface Selection module of NIMS class.
*
*-------------------------------------------------------------------------*/
struct SwimInterfaceSelectionConfigType
{
    SwimInterfaceSelectionConfigType()
    {
        HysteresisTimer = SWIM_DEFAULT_HYSTERESIS_TIMER;
    }

    /* Hysteresis Timer represents delay in reassignment of a traffic flow
    to a different interface after an assignment of that flow
    to some interface has just been completed. HysteresisTimer is
    specified in milliseconds. */
    uint16_t HysteresisTimer;

    bool operator== ( SwimInterfaceSelectionConfigType &cfg1)
    {
        return ((*this).HysteresisTimer == cfg1.HysteresisTimer );
    }

    bool operator!= ( SwimInterfaceSelectionConfigType &cfg1)
    {
        return !(*this == cfg1);
    }
};

/*---------------------------------------------------------------------------
* Type Description: A structure to mantain configuration related to
*                     Interface Manager module of NIMS class.
*
*-------------------------------------------------------------------------*/
struct SwimInterfaceMgrConfigType
{
    SwimInterfaceMgrConfigType()
    {
        bqePassiveTestDuration = SWIM_DEFAULT_T_PASSIVE_BQE_DURATION;
        bqeTBlacklist = SWIM_DEFAULT_T_BLACKLIST_BQE;
        icdTBlacklist = SWIM_DEFAULT_T_BLACKLIST_ICD;
    }

    // Duration of BQE test in milliseconds
    uint16_t bqePassiveTestDuration;

    // Duration of BQE Blacklisting in seconds
    uint16_t bqeTBlacklist;

    // Duration of ICD Blacklisting in seconds
    uint16_t icdTBlacklist;

    // list of authentication apps
    std::vector<std::string> authApps;

    bool operator== (SwimInterfaceMgrConfigType &cfg1)
    {
        return ( ( *this ).bqePassiveTestDuration == cfg1.bqePassiveTestDuration &&
            ( *this ).bqeTBlacklist == cfg1.bqeTBlacklist &&
            ( *this ).icdTBlacklist == cfg1.icdTBlacklist );
    }

    bool operator!= ( SwimInterfaceMgrConfigType &cfg1)
    {
        return !(*this == cfg1);
    }
};

/* poll parameters */
typedef struct SwimBeePollParametersType
{
    SwimBeePollParametersType()
    {
        pollInterval = SWIM_DEFAULT_WLAN_POLL_INTERVAL;
        burstDuration = SWIM_DEFAULT_WLAN_BURST_DURATION;
        topN = SWIM_DEFAULT_WLAN_TOP_N;
        alpha = SWIM_DEFAULT_ALPHA;
    }
    uint16_t pollInterval;
    uint8_t burstDuration;
    uint16_t topN;
    double alpha;
} SwimBeePollParametersType;

typedef struct SwimActiveBqeParametersType
{
    SwimActiveBqeParametersType()
    {
        memset(activeBqeUri, 0, SWIM_ACTIVE_BQE_URI_STRLEN);
        memset(postBqeUri, 0, SWIM_POST_BQE_URI_STRLEN);
        strlcpy(activeBqeUri, SWIM_DEFAULT_BQE_ACTIVE_BQE_URI, SWIM_ACTIVE_BQE_URI_STRLEN);
        strlcpy(postBqeUri, SWIM_DEFAULT_BQE_ACTIVE_BQE_POSTURI, SWIM_POST_BQE_URI_STRLEN);
        activeBqeDelay = SWIM_DEFAULT_BQE_ACTIVE_BQE_DELAY;
        defaultMbw = SWIM_DEFAULT_MBW;
        defaultRtt = SWIM_DEFAULT_RTT;
        congToSlow = SWIM_DEFAULT_CONG_TO_SLOW;
        bqePastSize = SWIM_DEFAULT_BQE_PAST_SIZE;
        bqeValidityShortTimer = SWIM_DEFAULT_BQE_VALIDITY_SHORT_TIMER;
        bqeGoodValidity = SWIM_DEFAULT_BQE_GOOD_VALIDITY;
        bqeBadValidity = SWIM_DEFAULT_BQE_BAD_VALIDITY;
        defaultMss = SWIM_DEFAULT_MSS;
    }

    char activeBqeUri[SWIM_ACTIVE_BQE_URI_STRLEN];
    char postBqeUri[SWIM_POST_BQE_URI_STRLEN];
    int activeBqeDelay;
    uint32_t defaultMbw;
    uint32_t defaultMss;
    uint32_t defaultRtt;
    uint8_t  congToSlow;
    uint8_t  bqePastSize;
    uint32_t bqeValidityShortTimer;
    uint32_t  bqeGoodValidity;
    uint32_t  bqeBadValidity;
} SwimActiveBqeParametersType;

/*---------------------------------------------------------------------------
* Type Description: A structure to maintain the mapping of BEE Poll parameters
* for a given radio interface type.
*-------------------------------------------------------------------------*/
typedef std::map<CneRatType, SwimBeePollParametersType*> SwimRatBeePollParametersMapType;

/*---------------------------------------------------------------------------
* Type Description: A structure to mantain configuration related to
*                     Bitrate Estimation Manager module of NIMS class.
*
*-------------------------------------------------------------------------*/
struct SwimBeeMgrConfigType
{
    SwimBeeMgrConfigType()
    {
        bqeDisabled = true;
    }

    /* A map of BEE poll parameters for a given RAT */
    bool bqeDisabled;
    SwimRatBeePollParametersMapType ratPollParametersMap;
    SwimActiveBqeParametersType activeBqeParameters;

    bool operator== (SwimBeeMgrConfigType &cfg1)
    {
        return (( *this ).bqeDisabled == cfg1.bqeDisabled &&
            (*this).activeBqeParameters.activeBqeDelay == cfg1.activeBqeParameters.activeBqeDelay);
    }

    bool operator!= ( SwimBeeMgrConfigType &cfg1)
    {
        return !(*this == cfg1);
    }
};

/*---------------------------------------------------------------------------
* Type Description: A structure to mantain configuration related to
*                     Internet Connectivity Detection module of NIMS class.
*
*-------------------------------------------------------------------------*/
struct SwimIcdMgrConfigType
{
    SwimIcdMgrConfigType():
    icdDisabled(DEFAULT_ICD_DISABLED),
    icdPastSize(DEFAULT_ICD_PAST_SIZE),
    icdValidityTimeout(DEFAULT_ICD_VALIDITY_TIMEOUT_SEC),
    icdTransactionTimeout(DEFAULT_ICD_TRANSACTION_TIMEOUT_SEC),
    icdHighProbability(DEFAULT_ICD_HIGH_PROBABILITY),
    icdMaxAuthTime(DEFAULT_ICD_MAX_AUTH_TIME),
    icdRetestTime(DEFAULT_ICD_RETEST_TIME)
{
    memset(icdUri, 0, SWIM_ICD_URI_STRLEN);
    memset(icdHttpUri, 0, SWIM_ICD_URI_STRLEN);
    strlcpy(icdUri, SWIM_DEFAULT_ICD_URI, SWIM_ICD_URI_STRLEN);
    strlcpy(icdHttpUri, SWIM_DEFAULT_ICD_HTTP_TURI, SWIM_ICD_URI_STRLEN);
    icdConfigType = SWIM_DEFAULT_ICD_CONFIG_TYPE;
}

bool icdDisabled;
uint8_t icdPastSize;
uint32_t icdValidityTimeout;
uint32_t icdTransactionTimeout;
float icdHighProbability;
char icdUri[SWIM_ICD_URI_STRLEN];
char icdHttpUri[SWIM_ICD_URI_STRLEN];
uint32_t icdMaxAuthTime;
uint32_t icdRetestTime;
std::string icdConfigType;
// list of apIds for ICD
std::map<std::string, bool> icdApIds;
};

/*---------------------------------------------------------------------------
 * Type Description: A structure to mantain configuration related to
 *                     TQE module of NIMS class.
 *
 *-------------------------------------------------------------------------*/
struct CneTQEConfigType
{
    CneTQEConfigType()
    {
        bbdDisabled = DEFAULT_TQE_DISABLED;
        dbdDisabled = DEFAULT_TQE_DISABLED;
        sockActiveThresh = DEFAULT_TQE_SOCK_ACTIVE_THRESH;
        dgimThresh = DEFAULT_TQE_DGIM_THRESH;
        dbdTputThresh = DEFAULT_TQE_TPUT_THRESH;
        tqeTimeWindow = DEFAULT_TQE_TIME_WINDOW;
        tqeCountThresh = DEFAULT_TQE_COUNT_THRESH;
        tqeOverallCountThresh = DEFAULT_TQE_OVERALL_COUNT_THRESH;
        tqeTBlacklist = DEFAULT_T_BLACKLIST_TQE;
        tqThresh = DEFAULT_TQ_THRESH;
        ratioThresh = DEFAULT_TQE_RATIO_THRESH;
        alphaTqe = DEFAULT_TQE_ALPHA;
    }

    bool bbdDisabled;
    bool dbdDisabled;
    uint16_t sockActiveThresh;
    uint16_t dgimThresh;
    uint32_t dbdTputThresh;
    uint16_t tqeTimeWindow;
    uint16_t tqeCountThresh;
    uint16_t tqeOverallCountThresh;
    uint16_t tqeTBlacklist;
    float tqThresh;
    float ratioThresh;
    float alphaTqe;

    bool operator== (CneTQEConfigType &cfg1)
    {
        return( (*this).bbdDisabled == cfg1.dbdDisabled &&
            (*this).dbdDisabled == cfg1.dbdDisabled &&
            (*this).sockActiveThresh == cfg1.sockActiveThresh &&
            (*this).dgimThresh == cfg1.dgimThresh &&
            (*this).dbdTputThresh == cfg1.dbdTputThresh &&
            (*this).tqeTimeWindow == cfg1.tqeTimeWindow &&
            (*this).tqeCountThresh == cfg1.tqeCountThresh &&
            (*this).tqeOverallCountThresh == cfg1.tqeOverallCountThresh &&
            (*this).tqeTBlacklist == cfg1.tqeTBlacklist &&
            (*this).tqThresh == cfg1.tqThresh &&
            (*this).ratioThresh == cfg1.ratioThresh &&
            (*this).alphaTqe == cfg1.alphaTqe );
    }

    bool operator!= ( CneTQEConfigType &cfg1)
    {
        return !(*this == cfg1);
    }
};

/*---------------------------------------------------------------------------
* Type Description: A structure to mantain configuration related to
*                  CQE module of NIMS class. CQE module is part of
*              SwimWlanLinkMgr class.
*
*
*-------------------------------------------------------------------------*/
struct SwimCQEProntoConfigType
{

    int RSSIModelThreshold;
    unsigned int FrameCntThreshold;
    unsigned int ColdStartThreshold;
    float MACMibThreshold2a;
    float MACMibThreshold2b;
    float RetryMetricWeight2a;
    float RetryMetricWeight2b;
    float MultiRetryMetricWeight2a;
    float MultiRetryMetricWeight2b;
    float FailMetricWeight2a;
    float FailMetricWeight2b;

    bool operator== (SwimCQEProntoConfigType &cfg1)
    {
            return( (*this).RSSIModelThreshold == cfg1.RSSIModelThreshold &&
            (*this).FrameCntThreshold == cfg1.FrameCntThreshold &&
            (*this).ColdStartThreshold == cfg1.ColdStartThreshold &&
            (*this).MACMibThreshold2a == cfg1.MACMibThreshold2a  &&
            (*this).MACMibThreshold2b == cfg1.MACMibThreshold2b  &&
            (*this).RetryMetricWeight2a == cfg1.RetryMetricWeight2a  &&
            (*this).RetryMetricWeight2b == cfg1.RetryMetricWeight2b  &&
            (*this).MultiRetryMetricWeight2a == cfg1.MultiRetryMetricWeight2a &&
            (*this).MultiRetryMetricWeight2b == cfg1.MultiRetryMetricWeight2b );
    }

    bool operator!= ( SwimCQEProntoConfigType &cfg1)
    {
        return !(*this == cfg1);
    }

    float computeFailMetricWeight( float a_retry, float a_multi_retry)
    {
        return (float)sqrt(1 - (a_retry * a_retry) - \
                ( (a_retry + a_multi_retry) * (a_retry + a_multi_retry) ) );
    }
};
struct SwimCQERomeConfigType
{
    float Rmp_Thr;
    unsigned int Rmp_Cnt_Thr;
    unsigned int Rx_Mcs_Thr;
    unsigned int Rx_Bw_Thr;
    float Tmd_Thr;
    unsigned int Tmd_Cnt_Thr;
    float Tmr_Thr;
    unsigned int Tmr_Cnt_Thr;
    unsigned int Tx_Mcs_Thr;
    unsigned int Tx_Bw_Thr;
    bool operator== (SwimCQERomeConfigType &cfg1)
    {
            return( (*this).Rmp_Thr == cfg1.Rmp_Thr &&
            (*this).Rmp_Cnt_Thr == cfg1.Rmp_Cnt_Thr &&
            (*this).Rx_Mcs_Thr == cfg1.Rx_Mcs_Thr  &&
            (*this).Rx_Bw_Thr == cfg1.Rx_Bw_Thr  &&
            (*this).Tmd_Thr == cfg1.Tmd_Thr  &&
            (*this).Tmd_Cnt_Thr == cfg1.Tmd_Cnt_Thr  &&
            (*this).Tmr_Thr == cfg1.Tmr_Thr  &&
            (*this).Tmr_Cnt_Thr == cfg1.Tmr_Cnt_Thr &&
            (*this).Tx_Mcs_Thr == cfg1.Tx_Mcs_Thr &&
            (*this).Tx_Bw_Thr == cfg1.Tx_Bw_Thr );
    }
    bool operator!= ( SwimCQERomeConfigType &cfg1)
    {
        return !(*this == cfg1);
    }
};
struct SwimCQEConfigType
{
    SwimCQEConfigType()
    {
        RSSIAddThreshold = SWIM_DEFAULT_RSSI_ADD_THRESHOLD;
        RSSIDropThreshold = SWIM_DEFAULT_RSSI_DROP_THRESHOLD;
        RSSIMacTimerThreshold = SWIM_DEFAULT_RSSI_MAC_TIMER_THRESHOLD;
        RSSIAveragingInterval = SWIM_DEFAULT_RSSI_AVERAGING_INTERVAL;
        CQEPeriodicTimer = SWIM_DEFAULT_CQE_PERIODIC_TIMER;
        MACHysteresisTimer = SWIM_DEFAULT_MAC_HYSTERESIS_TIMER;
        MACStatsAveragingAlpha = SWIM_DEFAULT_MAC_STATS_AVERAGING_ALPHA;
    }
    int RSSIAddThreshold;
    int RSSIDropThreshold;
    int RSSIMacTimerThreshold;
    unsigned int RSSIAveragingInterval;
    unsigned int CQEPeriodicTimer;
    unsigned int MACHysteresisTimer;
    float MACStatsAveragingAlpha;
    bool isWlanChipsetTypeRome;
    union
    {
       SwimCQEProntoConfigType prontoConfigType;
       SwimCQERomeConfigType romeConfigType;
    };
    bool operator== (SwimCQEConfigType &cfg1)
    {
        bool retVal = false;
        if((*this).isWlanChipsetTypeRome)
        {
            retVal = ((*this).romeConfigType == cfg1.romeConfigType);
        }
        else
        {
            retVal = ((*this).prontoConfigType == cfg1.prontoConfigType);
        }
        return( retVal && (*this).RSSIAddThreshold == cfg1.RSSIAddThreshold &&
            (*this).RSSIDropThreshold == cfg1.RSSIDropThreshold &&
            (*this).RSSIMacTimerThreshold == cfg1.RSSIMacTimerThreshold &&
            (*this).RSSIAveragingInterval == cfg1.RSSIAveragingInterval &&
            (*this).MACHysteresisTimer == cfg1.MACHysteresisTimer &&
            (*this).CQEPeriodicTimer == cfg1.CQEPeriodicTimer &&
            (*this).MACStatsAveragingAlpha == cfg1.MACStatsAveragingAlpha );
    }
    bool operator!= ( SwimCQEConfigType &cfg1)
    {
        return !(*this == cfg1);
    }
};


/*---------------------------------------------------------------------------
* Type Description: A high-level structure encompassing configurations
*                      related to all modules of NIMS package.
*-------------------------------------------------------------------------*/
struct SwimModulesConfigType
{
    SwimWiFiMgrConfigType WiFiCfg;
    SwimLinkPathMgrConfigType LinkPathCfg;
    SwimInterfaceSelectionConfigType IfSelectionCfg;
    SwimInterfaceMgrConfigType IfManagerCfg;
    SwimBeeMgrConfigType BEEManagerCfg;
    SwimIcdMgrConfigType ICDManagerCfg;
    CneTQEConfigType TQECfg;
    SwimCQEConfigType CQECfg;
};

/*---------------------------------------------------------------------------
* Type Description: A structure mapping CneRATSubType to their string representation
* for CneOEMPolicy XML parsing
*-------------------------------------------------------------------------*/
struct SwimRATSubType
{
    CneRatSubType subtype;
    const char *SubTypeStr;
};

#endif /*_WQE_POLICY_TYPES_H */
