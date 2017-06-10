/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __LBS_PROXY_H__
#define __LBS_PROXY_H__
#include <LBSProxyBase.h>
#include <LocAdapterBase.h>
#include <pthread.h>

using namespace loc_core;

namespace lbs_core {

class LBSProxy : public LBSProxyBase {
    static pthread_mutex_t mLock;
    static UlpProxyBase* mUlp;
    static LocAdapterBase* mAdapter;
    static unsigned long mCapabilities;
    static const char * COM_QUALCOMM_LOCATION_APK_FILE_PATH;
    static const char * FFOS_LOCATION_EXTENDED_CLIENT;
    static const bool mLocationExtendedClientExists;
public:
    inline LBSProxy() : LBSProxyBase() {}
    inline ~LBSProxy() {}
    static void informShutdown();
    inline virtual void requestUlp(LocAdapterBase* adapter,
                                   unsigned long capabilities) const {
        locRequestUlp(adapter, capabilities);
    }
    virtual LocApiBase* getLocApi(const MsgTask* msgTask,
                                  LOC_API_ADAPTER_EVENT_MASK_T exMask,
                                  ContextBase *context) const;
    inline virtual bool hasAgpsExtendedCapabilities() const;
    inline virtual bool hasCPIExtendedCapabilities() const;

    static void locRequestUlp(LocAdapterBase* adapter,
                              unsigned long capabilities);
    static void ulpRequestLoc(UlpProxyBase* ulp);
    static bool reportPositionToUlp(UlpLocation &location,
                                    GpsLocationExtended &locationExtended,
                                    void* locationExt,
                                    enum loc_sess_status status,
                                    LocPosTechMask techMask);
    static bool reportBatchingSessionToUlp(GpsExtBatchOptions &options,
                                            bool active);
    static bool reportPositionsToUlp(GpsExtLocation * locations,
                                     int32_t number_of_locations,
                                     enum loc_sess_status status,
                                     LocPosTechMask techMask);
    virtual void injectFeatureConfig(ContextBase* context) const;
private:
    inline static bool checkIfLocationExtendedClientExists();
};

}  // namespace lbs_core
#endif //__LBS_PROXY_H__
