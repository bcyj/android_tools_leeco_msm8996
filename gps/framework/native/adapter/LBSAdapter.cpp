/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 1
#define LOG_TAG "locSvc_LBSAdapter"

#include <LocDualContext.h>
#include <LBSAdapter.h>

using namespace loc_core;
using namespace lbs_core;

LBSAdapter* LBSAdapter::mMe = NULL;

LBSAdapter* LBSAdapter::get(const LOC_API_ADAPTER_EVENT_MASK_T mask,
                            MsgTask::tCreate tCreator, LBSCallback* callbacks)
{
    if (NULL == mMe) {
        mMe = new LBSAdapter(mask, tCreator, callbacks);
    }
    return mMe;
}

LBSAdapter::LBSAdapter(const LOC_API_ADAPTER_EVENT_MASK_T mask,
                       MsgTask::tCreate tCreator, LBSCallback* callbacks) :
    LBSAdapterBase(mask,
                   LocDualContext::getLocFgContext(tCreator,
                                                   LocDualContext::mLocationHalName)),
    mLBSCallbacks(callbacks)
{
}

bool LBSAdapter::requestWps(enum WifiRequestType type)
{
    sendMsg(new WiperRequest(type));
    return true;
}

bool LBSAdapter::requestWifiApData()
{
    sendMsg(new WiperApDataRequest());
    return true;
}

void LBSAdapter::handleEngineUpEvent()
{
    sendMsg(new WiperSsrInform());
}

int LBSAdapter::cinfoInject(int cid, int lac, int mnc,
                                int mcc, bool roaming)
{
    struct CinfoInjectMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mCid, mLac, mMnc, mMcc;
        bool mRoaming;
        inline CinfoInjectMsg(LBSApiBase* lbsApi,
                              int cid, int lac,
                              int mnc, int mcc, bool roaming) :
            LocMsg(), mLBSApi(lbsApi), mCid(cid), mLac(lac),
            mMnc(mnc), mMcc(mcc), mRoaming(roaming) {}
        inline virtual void proc() const {
            mLBSApi->cinfoInject(mCid, mLac, mMnc, mMcc, mRoaming);
        }
    };
    sendMsg(new CinfoInjectMsg(mLBSApi, cid, lac, mnc, mcc, roaming));
    return true;
}

int LBSAdapter::oosInform()
{
    struct OosInformMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        inline OosInformMsg(LBSApiBase* lbsApi) :
            LocMsg(), mLBSApi(lbsApi) {}
        inline virtual void proc() const {
            mLBSApi->oosInform();
        }
    };
    sendMsg(new OosInformMsg(mLBSApi));
    return true;
}

int LBSAdapter::niSuplInit(char* supl_init, int length)
{
    struct NisuplInitMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mLen;
        char* mSuplInit;
        inline NisuplInitMsg(LBSApiBase* lbsApi, char* supl_init,
                             int len) :
            LocMsg(), mLBSApi(lbsApi), mLen(len),
            mSuplInit(new char[mLen]) {
            memcpy(mSuplInit, supl_init, mLen);
        }
        inline virtual ~NisuplInitMsg() { delete[] mSuplInit; }
        inline virtual void proc() const {
            mLBSApi->niSuplInit(mSuplInit, mLen);
        }
    };
    sendMsg(new NisuplInitMsg(mLBSApi, supl_init, length));
    return true;
}

int LBSAdapter::chargerStatusInject(int status)
{
    struct ChargerSatusMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mStatus;
        inline ChargerSatusMsg(LBSApiBase* lbsApi, int status) :
            LocMsg(), mLBSApi(lbsApi), mStatus(status) {}
        inline virtual void proc() const {
            mLBSApi->chargerStatusInject(mStatus);
        }
    };
    sendMsg(new ChargerSatusMsg(mLBSApi, status));
    return true;
}

int LBSAdapter::wifiStatusInform()
{
    struct WifiStatusMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        inline WifiStatusMsg(LBSApiBase* lbsApi) :
            LocMsg(), mLBSApi(lbsApi) {}
        inline virtual void proc() const {
            mLBSApi->wifiStatusInform();
        }
    };
    sendMsg(new WifiStatusMsg(mLBSApi));
    return true;
}

int LBSAdapter::injectCoarsePosition(CoarsePositionInfo &cpInfo)
{
    struct CoarsePosMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        CoarsePositionInfo mCoarsePos;
        inline CoarsePosMsg(LBSApiBase* lbsApi, CoarsePositionInfo &cpInfo) :
            LocMsg(), mLBSApi(lbsApi), mCoarsePos(cpInfo) {}
        inline virtual void proc() const {
            mLBSApi->injectCoarsePosition(mCoarsePos);
        }
    };
    sendMsg(new CoarsePosMsg(mLBSApi, cpInfo));
    return true;
}

int LBSAdapter::injectWifiPosition(WifiLocation &wifiInfo)
{
    struct WifiPosMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        WifiLocation mWifiPos;
        inline WifiPosMsg(LBSApiBase* lbsApi, WifiLocation &wifiInfo) :
            LocMsg(), mLBSApi(lbsApi), mWifiPos(wifiInfo) {}
        inline virtual void proc() const {
            mLBSApi->injectWifiPosition(mWifiPos);
        }
    };
    sendMsg(new WifiPosMsg(mLBSApi, wifiInfo));
    return true;
}

int LBSAdapter::injectWifiApInfo(WifiApInfo &wifiApInfo)
{
    struct WifiApDataMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        WifiApInfo mWifiApData;
        inline WifiApDataMsg(LBSApiBase* lbsApi, WifiApInfo &wifiApInfo) :
            LocMsg(), mLBSApi(lbsApi), mWifiApData(wifiApInfo) {}
        inline virtual void proc() const {
            mLBSApi->injectWifiApInfo(mWifiApData);
        }
    };
    sendMsg(new WifiApDataMsg(mLBSApi, wifiApInfo));
    return true;
}

int LBSAdapter::setWifiWaitTimeoutValue(int timeout)
{
    struct WifiSetTimeoutMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mTimeout;
        inline WifiSetTimeoutMsg(LBSApiBase* lbsApi, int timeout) :
            LocMsg(), mLBSApi(lbsApi), mTimeout(timeout) {}
        inline virtual void proc() const {
            mLBSApi->setWifiWaitTimeoutValue(mTimeout);
        }
    };
    sendMsg(new WifiSetTimeoutMsg(mLBSApi, timeout));
    return true;
}

int LBSAdapter::getZppFixRequest(enum zpp_fix_type type)
{
    struct LBSAdapterGetZppFixMsg : public LocMsg {
        LBSAdapter* mAdapter;
        enum zpp_fix_type mZppType;
        inline LBSAdapterGetZppFixMsg(LBSAdapter* adapter, enum zpp_fix_type type) :
            mAdapter(adapter), mZppType(type){}
        inline virtual void proc() const {
            mAdapter->getZppFixSync(mZppType);
        }
    };
    sendMsg(new LBSAdapterGetZppFixMsg(this, type));
    return true;
}

void LBSAdapter::getZppFixSync(enum zpp_fix_type type)
{
    GpsLocation gpsLocation;
    enum loc_api_adapter_err adapter_err;
    if(type == ZPP_FIX_BEST_AVAILABLE) {
        adapter_err = mLocApi->getBestAvailableZppFix(gpsLocation);
    }
    else {
        adapter_err = mLocApi->getWwanZppFix(gpsLocation);
    }
    if (adapter_err == LOC_API_ADAPTER_ERR_SUCCESS) {
        if (mLBSCallbacks) {
            mLBSCallbacks->locationReport(gpsLocation);
        }
    }
}

int LBSAdapter::wifiAttachmentStatusInject(
    WifiSupplicantInfo &wifiSupplicantInfo)
{
    struct WifiAttachmentStatusMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        WifiSupplicantInfo mWifiSupplicantInfo;
        inline WifiAttachmentStatusMsg(LBSApiBase* lbsApi,
                                       WifiSupplicantInfo &wifiSupplicantInfo) :
            LocMsg(), mLBSApi(lbsApi),
            mWifiSupplicantInfo(wifiSupplicantInfo) {}
        inline virtual void proc() const {
            mLBSApi->wifiAttachmentStatusInject(mWifiSupplicantInfo);
        }
    };
    sendMsg(new WifiAttachmentStatusMsg(mLBSApi, wifiSupplicantInfo));
    return true;
}

int LBSAdapter::wifiEnabledStatusInject(int status)
{
    struct WifiEnabledStatusMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mStatus;
        inline WifiEnabledStatusMsg(LBSApiBase* lbsApi, int status) :
            LocMsg(), mLBSApi(lbsApi), mStatus(status) {}
        inline virtual void proc() const {
            mLBSApi->wifiEnabledStatusInject(mStatus);
        }
    };
    sendMsg(new WifiEnabledStatusMsg(mLBSApi, status));
    return true;
}
