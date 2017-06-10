/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef IZAT_API_V02_H
#define IZAT_API_V02_H

#include <LocApiV02.h>
#include <IzatApiBase.h>
#include <geofence.h>

namespace lbs_core {
    class LocApiProxyV02;
};

using namespace loc_core;
using namespace lbs_core;

namespace izat_core {

class IzatApiV02 : public IzatApiBase {
    uint32_t mBatchSize;
    uint32_t mDesiredBatchSize;
    bool mBatchingSupported;
    bool mTrackingInProgress;
    bool mBatchingInProgress;
    int startTracking(int64_t period_ns,
                      int32_t accuracy);
    int startModemBatching(uint32_t flags,
                           int64_t period_ns,
                           int32_t accuracy,
                           int32_t timeout);
    int stopTracking(int32_t id);
    int stopModemBatching(int32_t id);
    int queryBatchBufferOnModem(int32_t size);
public:
    IzatApiV02(LocApiProxyV02* locApiProxy);
    inline virtual ~IzatApiV02() {}

    inline virtual void* getSibling() { return (IzatApiBase*)this; }

    /* event callback registered with the loc_api v02 interface */
    virtual int eventCb(locClientHandleType client_handle,
                         uint32_t loc_event_id,
                         locClientEventIndUnionType loc_event_payload);
    // For Geofence
    void GeofenceBreach(const qmiLocEventGeofenceBreachIndMsgT_v02* breachInfo);
    void batchGeofenceBreach(const qmiLocEventGeofenceBatchedBreachIndMsgT_v02* batchedBreachInfo);
    void GeofenceStatus(const qmiLocEventGeofenceGenAlertIndMsgT_v02* alertInfo);
    virtual int addGeofence(GeoFenceData geofenceData, bool needsResponse);
    virtual int removeGeofence(uint32_t hwId, int32_t afwId);
    virtual int pauseGeofence(uint32_t hwId, int32_t afwId);
    virtual int resumeGeofence(uint32_t hwId, int32_t afwId, uint32_t breachMask);
    virtual int modifyGeofence(uint32_t hwId, GeoFenceData geofenceData);
    // set user preference
    virtual int setUserPref(bool user_pref);

    virtual void setBatchingSupport(bool supported);
    virtual int cleanUpBatching();
    virtual void setBatchSize(int32_t size);
    virtual int startBatching(int32_t id,
                              const GpsExtBatchOptions& options,
                              int32_t accuracy,
                              int32_t timeout);
    virtual int updateBatching(int32_t id,
                               const GpsExtBatchOptions& options,
                               int32_t accuracy,
                               int32_t timeout);
    virtual int stopBatching(int32_t id);
    virtual int getBatchedLocation(int32_t lastNlocations,
                                   LocBatchingReportedType reportType);
    virtual int injectLocation(GpsExtLocation location);
    virtual bool isMessageSupported(LocCheckingMessagesID msgID);
    void readModemLocations(GpsExtLocation* pLocationPiece,
                            int32_t lastNlocations,
                            int32_t& numbOfEntries);

    // for TDP
    virtual int sendGtpEnhancedCellConfigBlocking(e_premium_mode mode);
    virtual int sendGtpCellConfigurationsBlocking(const char* data, uint32_t len);

    virtual int sendGdtUploadBeginResponse(int32_t service, int32_t session, int32_t status);
    virtual int sendGdtUploadEndResponse(int32_t service, int32_t session, int32_t status);

private:
    void onGdtUploadBeginEvent(const qmiLocEventGdtUploadBeginStatusReqIndMsgT_v02* pInfo);
    void onGdtUploadEndEvent(const qmiLocEventGdtUploadEndReqIndMsgT_v02* pInfo);
};

}  // namespace izat_core

#endif //IZAT_API_V02_H
