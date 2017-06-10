/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_LBSApiBase"
#include <log_util.h>
#include <LBSApiBase.h>
#include <LBSAdapterBase.h>

using namespace loc_core;

namespace lbs_core {

#define TO_ALL_LBSADAPTERS(call) TO_ALL_ADAPTERS(mLBSAdapters, (call))
#define TO_1ST_HANDLING_LBSADAPTERS(call) TO_1ST_HANDLING_ADAPTER(mLBSAdapters, (call))

LBSApiBase::LBSApiBase(LocApiProxyBase *locApiProxy) :
    mLocApiProxy(locApiProxy)
{
    memset(mLBSAdapters, 0, sizeof(mLBSAdapters));
    LOC_LOGD("%s:%d]:LBSApiBase created. lbsApi:%p\n", __func__, __LINE__, this);
}

void LBSApiBase::addAdapter(LBSAdapterBase* adapter)
{
    for (int i = 0; i < MAX_ADAPTERS && mLBSAdapters[i] != adapter; i++) {
        if (mLBSAdapters[i] == NULL) {
            mLBSAdapters[i] = adapter;
            LOC_LOGD("%s:%d]: Adapter added: %p\n", __func__, __LINE__, adapter);
            break;
        }
    }
}

void LBSApiBase::removeAdapter(LBSAdapterBase* adapter)
{
    for (int i = 0;
         i < MAX_ADAPTERS && NULL != mLBSAdapters[i];
         i++) {
        if (mLBSAdapters[i] == adapter) {
            mLBSAdapters[i] = NULL;

            // shift the rest of the adapters up so that the pointers
            // in the array do not have holes.  This should be more
            // performant, because the array maintenance is much much
            // less frequent than event handlings, which need to linear
            // search all the adapters
            int j = i;
            while (++i < MAX_ADAPTERS && mLBSAdapters[i] != NULL);

            // i would be MAX_ADAPTERS or point to a NULL
            i--;
            // i now should point to a none NULL adapter within valid
            // range although i could be equal to j, but it won't hurt.
            // No need to check it, as it gains nothing.
            mLBSAdapters[j] = mLBSAdapters[i];
            // this makes sure that we exit the for loop
            mLBSAdapters[i] = NULL;
        }
    }
}

void LBSApiBase::requestWps(enum WifiRequestType type)
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestWps(type));
}

void LBSApiBase::requestWifiApData()
{
    // loop through adapters, and deliver to the first handling adapter.
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestWifiApData());
}

int LBSApiBase::cinfoInject(int cid, int lac, int mnc,
                int mcc, bool roaming)
DEFAULT_IMPL(0)

int  LBSApiBase::oosInform()
DEFAULT_IMPL(0)

int  LBSApiBase::niSuplInit(char* supl_init, int length)
DEFAULT_IMPL(0)

int  LBSApiBase::chargerStatusInject(int status)
DEFAULT_IMPL(0)

int  LBSApiBase::wifiStatusInform()
DEFAULT_IMPL(0)

int  LBSApiBase::wifiAttachmentStatusInject(
    const WifiSupplicantInfo &wifiSupplicantInfo)
DEFAULT_IMPL(0)

int  LBSApiBase::wifiEnabledStatusInject(int status)
DEFAULT_IMPL(0)

int LBSApiBase::injectCoarsePosition(const CoarsePositionInfo &cpInfo)
DEFAULT_IMPL(0)

int  LBSApiBase::injectWifiPosition(const WifiLocation &wifiInfo)
DEFAULT_IMPL(0)

int  LBSApiBase::injectWifiApInfo(const WifiApInfo &wifiApInfo)
DEFAULT_IMPL(0)

int  LBSApiBase::setWifiWaitTimeoutValue(int timeout)
DEFAULT_IMPL(0)

int LBSApiBase::shutdown()
DEFAULT_IMPL(0)

void LBSApiBase::requestSensorData(SensorRequest &request)
{
     TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestSensorData(request));
}

void LBSApiBase::requestPedometerData(PedometerRequest &request)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestPedometerData(request));
}

void LBSApiBase::requestMotionData(SensorRequest &request)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestMotionData(request));
}

void LBSApiBase::requestTimeData(TimeRequest &timeRequest)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestTimeData(timeRequest));
}

void LBSApiBase::requestSPIStatus(SensorRequest &request)
{
    TO_1ST_HANDLING_LBSADAPTERS(mLBSAdapters[i]->requestSPIStatus(request));
}

int LBSApiBase::injectSensorData(const SensorDataBundle &dataBundle)
DEFAULT_IMPL(0)

int LBSApiBase::injectMotionData(const MotionData &motionData)
DEFAULT_IMPL(0)

int LBSApiBase::injectPedometerData(const PedometerData &pedometerData)
DEFAULT_IMPL(0)

int LBSApiBase::injectTimeData(const TimeData &data)
DEFAULT_IMPL(0)

int LBSApiBase::injectSPIStatus(const SPIStatus &status)
DEFAULT_IMPL(0)

void LBSApiBase::injectFeatureConfig(e_premium_mode sap_mode, e_premium_mode gtp_cell_mode)
DEFAULT_IMPL()
}; //namespace lbs_core
