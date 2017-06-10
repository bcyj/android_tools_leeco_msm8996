/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#ifndef IZAT_ADAPTER_BASE_H
#define IZAT_ADAPTER_BASE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <string.h>
#include <LocAdapterBase.h>
#include <geofence.h>
#include <LocAdapterProxyBase.h>

using namespace loc_core;

namespace izat_core {

class IzatApiBase;

class IzatAdapterBase : public LocAdapterProxyBase {
protected:
    IzatApiBase* mIzatApi;
    IzatAdapterBase(const LOC_API_ADAPTER_EVENT_MASK_T mask,
                    ContextBase* context);
    virtual ~IzatAdapterBase();
    const MsgTask* mMsgTask;
public:
    virtual bool gfBreachEvent(int32_t hwId, GpsExtLocation& gpsLocation, int32_t transition);
    virtual bool gfStatusEvent(uint64_t status);
    virtual bool gfResponse(GeofenceResp resp, int32_t status, uint32_t hwId);
    virtual bool saveGeofenceItem(uint32_t hwId, GeoFenceData& geofenceData);
    virtual bool removeGeofenceItem(uint32_t hwId);
    virtual bool pauseGeofenceItem(uint32_t hwId);
    virtual bool resumeGeofenceItem(uint32_t hwId);

    inline void sendMsg(const LocMsg* msg) const {
        mMsgTask->sendMsg(msg);
    }
    inline void sendMsg(const LocMsg* msg) {
        mMsgTask->sendMsg(msg);
    }
    virtual bool handleReportedLocations(const GpsExtLocation* location,
                                         int32_t number_query,
                                         int32_t number_read,
                                         LocBatchingReportedType reportType);
    virtual bool getLocations(int32_t last_n_locations, LocBatchingReportedType reportType);

    virtual bool setUserPreference(bool user_pref);
    virtual void handleEngineUpEvent();
    virtual void handleEngineDownEvent();
    virtual bool updateGfBreachLocation(GpsExtLocation& gpsLocation);
    virtual bool reportPosition(UlpLocation &location,
                                enum loc_sess_status status,
                                LocPosTechMask loc_technology_mask);
    // for GDT AP
    virtual bool onGdtUploadBeginEvent(int32_t service,
                                       int32_t session,
                                       const char* filePath,
                                       uint32_t filePath_len);
    virtual bool onGdtUploadEndEvent(int32_t service, int32_t session, int32_t status);
#ifdef ON_TARGET_TEST
#endif
};

}  // namespace izat_core

#ifdef __cplusplus
}
#endif

#endif /* IZAT_ADAPTER_BASE_H */
