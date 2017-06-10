/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef IZAT_API_BASE_H
#define IZAT_API_BASE_H

#include <Wiper.h>
#include <LocApiBase.h>
#include <geofence.h>
#include <LBSApiBase.h>

using namespace loc_core;
using namespace lbs_core;

namespace izat_core {

enum IzatApiReturnCode {
    IZAT_SUCCESS,
    IZAT_FAIL,
    IZAT_INVALID_ARGUMENTS,
};

class IzatAdapterBase;

class IzatApiBase {
    IzatAdapterBase* mIzatAdapters[MAX_ADAPTERS];
protected:
    LocApiProxyBase *mLocApiProxy;
public:
    IzatApiBase(LocApiProxyBase *locApiProxy);
    inline virtual ~IzatApiBase() {}

    void addAdapter(IzatAdapterBase* adapter);
    void removeAdapter(IzatAdapterBase* adapter);

    void saveGeofenceItem(uint32_t hwId, GeoFenceData& geofenceInfo);
    void removeGeofenceItem(uint32_t hwId);
    void pauseGeofenceItem(uint32_t hwId);
    void resumeGeofenceItem(uint32_t hwId);

    void geofenceBreach(int32_t hwId, GpsExtLocation& gpsLocation, int32_t transition);
    void geofenceStatus(uint64_t status);
    void geofenceResponse(GeofenceResp resp, int32_t status, uint32_t hwId);
    virtual int addGeofence(GeoFenceData geofenceInfo, bool needsResponse);
    virtual int removeGeofence(uint32_t hwId,int32_t afwId);
    virtual int pauseGeofence(uint32_t hwId,int32_t afwId);
    virtual int resumeGeofence(uint32_t hwId, int32_t afwId, uint32_t breachMask);
    virtual int modifyGeofence(uint32_t hwId, GeoFenceData data);
    virtual bool isMessageSupported(LocCheckingMessagesID msgID);
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
    virtual int getBatchedLocation(int32_t lastNLocations, LocBatchingReportedType reportType);
    virtual int injectLocation(GpsExtLocation location);
    void reportLocations(GpsExtLocation* location,
                        int32_t number_query,
                        int32_t last_n_locations,
                        LocBatchingReportedType reportType);
    void readLocationsFromModem(int32_t number, LocBatchingReportedType reportType);
    void geofenceBreachLocation(GpsExtLocation &gpsLocation);

    // for TDP
    virtual int sendGtpEnhancedCellConfigBlocking(e_premium_mode mode);
    virtual int sendGtpCellConfigurationsBlocking(const char* data, uint32_t len);

    virtual int sendGdtUploadBeginResponse(int32_t service, int32_t session, int32_t status);
    virtual int sendGdtUploadEndResponse(int32_t service, int32_t session, int32_t status);

protected:
    void handleGdtUploadBeginEvent(int32_t service,
                                   int32_t session,
                                   const char* filePath,
                                   uint32_t filePath_len);
    void handleGdtUploadEndEvent(int32_t service, int32_t session, int32_t status);
};

}  // namespace izat_core

#endif //IZAT_API_BASE_H
