/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                  LocUlpProxy header file

GENERAL DESCRIPTION
  This file contains the data structure and variables used for handling requests
  sent by GPS HAL. This acts as a proxy for the ULP module and generally forwards
  the requests to the ULP msg queue
=============================================================================*/

#ifndef LOC_ULP_PROXY_H
#define LOC_ULP_PROXY_H

#include <LocAdapterBase.h>

using namespace loc_core;

class LocUlpProxy : public UlpProxyBase {
    LocAdapterBase* mAdapter;
public:
    unsigned long mCapabilities;
    void* mQ;
    LocUlpProxy();
    virtual ~LocUlpProxy();
    virtual bool sendStartFix();
    virtual bool sendStopFix();
    virtual bool sendFixMode(LocPosMode &params);
    virtual bool reportPosition(UlpLocation &location,
                                GpsLocationExtended &locationExtended,
                                void* locationExt,
                                enum loc_sess_status status,
                                LocPosTechMask loc_technology_mask);
    virtual bool reportPositions(GpsExtLocation * locations,
                                int32_t number_of_locations,
                                enum loc_sess_status status,
                                LocPosTechMask techMask);
    virtual bool reportBatchingSession(GpsExtBatchOptions & options,
                                       bool active);
    virtual bool reportSv(GpsSvStatus &svStatus,
                          GpsLocationExtended &locationExtended,
                          void* svExt);
    virtual bool reportStatus(GpsStatusValue status);
    inline virtual void setAdapter(LocAdapterBase* adapter);
    inline virtual void setCapabilities(unsigned long capabilities) {
        mCapabilities = capabilities;
    }

    inline void sendLocMsg(LocMsg* msg) {
        if (NULL != mAdapter)
            mAdapter->sendMsg(msg);
    }

    inline LocAdapterBase* getAdapter() { return mAdapter; }
};

#endif /* LOC_ULP_PROXY_H */
