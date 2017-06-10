/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_LocUlpProxy"
#include <log_util.h>
#include <LocUlpProxy.h>
#include <ulp_engine.h>
#include <msg_q.h>
#include <ulp_internal.h>

using namespace loc_core;

LocUlpProxy::LocUlpProxy() :
    UlpProxyBase(), mAdapter(NULL), mCapabilities(0),
    mQ((void*)msg_q_init2())
{
}

LocUlpProxy::~LocUlpProxy()
{
    msg_q_destroy(&mQ);
}
/*===========================================================================
FUNCTION    setAdapter

DESCRIPTION
   Set the value of adapter to be used

DEPENDENCIES
   None

RETURN VALUE

SIDE EFFECTS
   N/A
===========================================================================*/
void LocUlpProxy::setAdapter(LocAdapterBase* adapter)
{
    LOC_LOGV("%s] %p", __func__, adapter);
    mAdapter = adapter;
}

/*===========================================================================
FUNCTION    sendStartFix

DESCRIPTION
   Send msg to ULP queue to start a fix

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::sendStartFix()
{
    ulp_msg *msg(new ulp_msg(this, ULP_MSG_START_FIX));
    msg_q_snd(mQ, msg, ulp_msg_free);
    return true;
}

/*===========================================================================
FUNCTION    sendStopFix

DESCRIPTION
   Send msg to ULP queue to stop fix

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::sendStopFix()
{
    ulp_msg *msg(new ulp_msg(this, ULP_MSG_STOP_GNSS_FIX));
    msg_q_snd(mQ, msg, ulp_msg_free);
    return true;
}

/*===========================================================================
FUNCTION    sendFixMode

DESCRIPTION
   Send msg to ULP queue to set position mode

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::sendFixMode(LocPosMode &params)
{
    ulp_msg_position_mode *msg(
        new ulp_msg_position_mode(&ulp_data, params));
    msg_q_snd(mQ, msg, ulp_msg_free);
    return true;
}

/*===========================================================================
FUNCTION    reportPosition

DESCRIPTION
   Send msg to ULP queue to with information about the position fix

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::reportPosition(UlpLocation &location,
                                 GpsLocationExtended &locationExtended,
                                 void* locationExt,
                                 enum loc_sess_status status,
                                 LocPosTechMask loc_technology_mask)
{
    ulp_msg_report_position *msg(
        new ulp_msg_report_position(&ulp_data,
                                    location,
                                    locationExtended,
                                    locationExt,
                                    status,
                                    loc_technology_mask));
    msg_q_snd(mQ, msg, ulp_msg_free);
    LOC_LOGV("%s] %d", __func__, location.position_source);
    return true;
}


/*===========================================================================
FUNCTION    reportPositions

DESCRIPTION
   Send msg to ULP queue to with information about the position fix

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::reportPositions(GpsExtLocation * locations,
                                 int32_t number_of_locations,
                                 enum loc_sess_status status,
                                 LocPosTechMask loc_technology_mask)
{
    ulp_msg_report_positions *msg(
        new ulp_msg_report_positions(&ulp_data,
                                    locations,
                                    number_of_locations,
                                    status,
                                    loc_technology_mask));
    msg_q_snd(mQ, msg, ulp_msg_free);
    return true;
}

/*===========================================================================
FUNCTION    reportBatchingSession

DESCRIPTION
   Send msg to ULP queue to with information about the Batching session

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::reportBatchingSession(GpsExtBatchOptions & options,
                                        bool active)
{
    ulp_msg_report_batching_session *msg(
        new ulp_msg_report_batching_session(&ulp_data, options, active));
    msg_q_snd(mQ, msg, ulp_msg_free);
    LOC_LOGV("%s]", __func__);
    LOC_LOGV("Max Power Allocatioin: %f",options.max_power_allocation_mW);
    LOC_LOGV("Sources to use: %d",options.sources_to_use);
    LOC_LOGV("Flags: %d",options.flags);
    LOC_LOGV("Period_ns: %lld",options.period_ns);
    LOC_LOGV("Batching Active: %d", active);
    return true;
}

/*===========================================================================
FUNCTION    reportSv

DESCRIPTION
   Send msg to ULP queue to with information about visible satellites
   as SV report

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::reportSv(GpsSvStatus &svStatus,
                           GpsLocationExtended &locationExtended,
                           void* svExt)
{
    ulp_msg_report_sv *msg(
        new ulp_msg_report_sv(&ulp_data, svStatus,
                              locationExtended, svExt));
    msg_q_snd(mQ, msg, ulp_msg_free);
    return true;
}

/*===========================================================================
FUNCTION    reportStatus

DESCRIPTION
   Send msg to ULP queue about GPS engine status

DEPENDENCIES
   None

RETURN VALUE
   true

SIDE EFFECTS
   N/A
===========================================================================*/
bool LocUlpProxy::reportStatus(GpsStatusValue status)
{
    ulp_msg_report_status *msg(
        new ulp_msg_report_status(&ulp_data, status));
    msg_q_snd(mQ, msg, ulp_msg_free);
    return true;
}
