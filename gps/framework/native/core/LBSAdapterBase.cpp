/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 1
#define LOG_TAG "locSvc_LBSAdapterBase"

#include <log_util.h>
#include <LBSAdapterBase.h>

using namespace loc_core;

namespace lbs_core {

LBSAdapterBase :: LBSAdapterBase(const LOC_API_ADAPTER_EVENT_MASK_T mask,
                                 ContextBase *context) :
        LocAdapterBase(mask, context),
        mLBSApi((LBSApiBase *)context->getLocApi()->getSibling())
{
    mLBSApi->addAdapter(this);
    LOC_LOGD("%s:%d]: LBSAdapterBase created: %p\n", __func__, __LINE__, this);
}

LBSAdapterBase :: ~LBSAdapterBase()
{
    mLBSApi->removeAdapter(this);
}

bool LBSAdapterBase::requestWps(enum WifiRequestType type)
DEFAULT_IMPL(false)

bool LBSAdapterBase::requestWifiApData()
DEFAULT_IMPL(false)

bool LBSAdapterBase::requestSensorData(const SensorRequest &request)
DEFAULT_IMPL(false)

bool LBSAdapterBase::requestPedometerData(const PedometerRequest &request)
DEFAULT_IMPL(false)

bool LBSAdapterBase::requestMotionData(const SensorRequest &request)
DEFAULT_IMPL(false)

bool LBSAdapterBase::requestTimeData(const TimeRequest &timeRequest)
DEFAULT_IMPL(false)

bool LBSAdapterBase::requestSPIStatus(const SensorRequest &request)
DEFAULT_IMPL(false)

};
