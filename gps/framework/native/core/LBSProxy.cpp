/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_LBSProxy"

#include <unistd.h>
#include <dlfcn.h>
#ifdef _HAS_LOC_V02_
#include <LBSApiV02.h>
#elif _HAS_LOC_RPC_
#include <LBSApiRpc.h>
#endif
#include <LBSProxy.h>
#include <log_util.h>
#include <loc_cfg.h>
#include <LBSAdapterBase.h>
#include <MsgTask.h>

#define IZAT_CONF_FILE "/etc/izat.conf"

using namespace loc_core;
using namespace lbs_core;

namespace lbs_core {

pthread_mutex_t LBSProxy::mLock = PTHREAD_MUTEX_INITIALIZER;
UlpProxyBase* LBSProxy::mUlp = NULL;
LocAdapterBase* LBSProxy::mAdapter = NULL;
unsigned long LBSProxy::mCapabilities = 0;
const char * LBSProxy::COM_QUALCOMM_LOCATION_APK_FILE_PATH =
    "/system/priv-app/com.qualcomm.location/com.qualcomm.location.apk";
const char * LBSProxy::FFOS_LOCATION_EXTENDED_CLIENT =
    "/system/b2g/distribution/bundles/location/location.so";
const bool LBSProxy::mLocationExtendedClientExists =
    LBSProxy::checkIfLocationExtendedClientExists();

LocApiBase* LBSProxy::getLocApi(const MsgTask* msgTask,
                                LOC_API_ADAPTER_EVENT_MASK_T exMask,
                                ContextBase *context) const {
#ifdef _HAS_LOC_V02_
    void *handle = NULL;
    if((handle = dlopen("libizat_api_pds.so", RTLD_NOW)) != NULL) {
        LOC_LOGD("%s:%d]: libizat_api_pds.so is present. Now trying to load IzatApiPds.", __func__, __LINE__);
        getLocApi_t* getter = (getLocApi_t*)dlsym(handle, "getLocApi");
        if(getter != NULL) {
            LocApiBase* locApi = (*getter)(msgTask, exMask, context);
            if ( locApi != NULL ) {
                return locApi;
            }
        }
    }
    return new LBSApiV02(msgTask, exMask, context);
#elif _HAS_LOC_RPC_
    return new LBSApiRpc(msgTask, exMask, context);
#endif
    return NULL;
}

bool LBSProxy::hasAgpsExtendedCapabilities() const {

    return mLocationExtendedClientExists;
}

bool LBSProxy::hasCPIExtendedCapabilities() const {

    return mLocationExtendedClientExists;
}

void LBSProxy::locRequestUlp(LocAdapterBase* adapter,
                              unsigned long capabilities) {
    pthread_mutex_lock(&LBSProxy::mLock);

    if (NULL == LBSProxy::mUlp) {
        LOC_LOGV("%s] %p", __func__, LBSProxy::mUlp);
    } else {
        LOC_LOGV("%s] ulp %p adapter %p", __func__,
                 LBSProxy::mUlp, adapter);
        LBSProxy::mUlp->setAdapter(adapter);
        LBSProxy::mUlp->setCapabilities(capabilities);
        adapter->setUlpProxy(LBSProxy::mUlp);
    }
    LBSProxy::mAdapter = adapter;
    LBSProxy::mCapabilities = capabilities;

    pthread_mutex_unlock(&LBSProxy::mLock);
}

void LBSProxy::ulpRequestLoc(UlpProxyBase* ulp) {
    pthread_mutex_lock(&LBSProxy::mLock);

    if (NULL == LBSProxy::mAdapter) {
        LOC_LOGV("%s] %p", __func__, LBSProxy::mAdapter);
    } else {
        LOC_LOGV("%s] ulp %p adapter %p", __func__, ulp,
                 LBSProxy::mAdapter);
        ulp->setAdapter(LBSProxy::mAdapter);
        ulp->setCapabilities(LBSProxy::mCapabilities);
        LBSProxy::mAdapter->setUlpProxy(ulp);
    }
    LBSProxy::mUlp = ulp;

    pthread_mutex_unlock(&LBSProxy::mLock);
}

bool LBSProxy::checkIfLocationExtendedClientExists()
{

    // To differentiate between PDK and non PDK builds
    if((access(COM_QUALCOMM_LOCATION_APK_FILE_PATH, F_OK) != -1) ||
        (access(FFOS_LOCATION_EXTENDED_CLIENT, F_OK) != -1)) {
        LOC_LOGD("%s] %s",__func__, "File Exists");
        return true;
    } else {
        LOC_LOGD("%s] %s",__func__, "File does not Exist");
        return false;
    }
}

bool LBSProxy :: reportPositionToUlp(UlpLocation &location,
                                     GpsLocationExtended &locationExtended,
                                     void* locationExt,
                                     enum loc_sess_status status,
                                     LocPosTechMask techMask)
{
    LOC_LOGD("%s:%d]: Enter", __func__, __LINE__);
    if(mUlp != NULL) {
        LOC_LOGD("%s:%d]: Exit. Sent to ulp", __func__, __LINE__);
        return mUlp->reportPosition(location, locationExtended, locationExt, status,
                                    techMask);
    }
    else {
        LOC_LOGD("%s:%d]: Exit", __func__, __LINE__);
        return false;
    }
}
void LBSProxy::injectFeatureConfig(ContextBase* context) const
{
    class InjectAdapter:public LBSAdapterBase {
    public:
        InjectAdapter(ContextBase *context):
            LBSAdapterBase(0, context) {}
        virtual void injectFeatureConfig() const
        {
            e_premium_mode sap_mode=DISABLED;
            e_premium_mode gtp_cell_mode=DISABLED;
            char conf_feature_gtp_cell_proc[32];
            char conf_feature_gtp_cell[32];
            char conf_feature_sap[32];
            loc_param_s_type izat_conf_feature_table[] =
            {
                {"GTP_CELL_PROC", &conf_feature_gtp_cell_proc, NULL, 's'},
                {"GTP_CELL",      &conf_feature_gtp_cell,      NULL, 's'},
                {"SAP",           &conf_feature_sap,           NULL, 's'}
            };
            UTIL_READ_CONF(IZAT_CONF_FILE, izat_conf_feature_table);

            //GTP CELL
            if(strcmp(conf_feature_gtp_cell_proc, "AP") == 0) {
                LOC_LOGD("%s:%d]: GTP_CELL_PROC set to AP", __func__, __LINE__);
            }
            //Defaulting to modem if it is set to MODEM or any other
            //random string
            else {
                LOC_LOGD("%s:%d]: GTP_CELL_PROC set to MODEM", __func__, __LINE__);
                if(strcmp(conf_feature_gtp_cell, "BASIC") == 0) {
                    LOC_LOGD("%s:%d]: GTP CELL mode set to BASIC", __func__, __LINE__);
                    gtp_cell_mode = BASIC;
                }
                else if(strcmp(conf_feature_gtp_cell, "PREMIUM") == 0) {
                    LOC_LOGE("%s:%d]: GTP CELL does not support PREMIUM mode."\
                             " Setting to default mode: BASIC", __func__, __LINE__);
                    gtp_cell_mode = BASIC;
                }
                else if(strcmp(conf_feature_gtp_cell, "DISABLED") == 0) {
                    LOC_LOGD("%s:%d]: GTP CELL mode set to DISABLED", __func__, __LINE__);
                }
                else {
                    LOC_LOGE("%s:%d]: Unrecognized value for GTP CELL Mode." \
                             " Setting GTP CELL to default mode: BASIC", __func__, __LINE__);
                    gtp_cell_mode = BASIC;
                }
            }

            //SAP
            if(strcmp(conf_feature_sap, "BASIC") == 0) {
                sap_mode = BASIC;
                LOC_LOGD("%s:%d]: Setting SAP to mode: BASIC", __func__, __LINE__);
            }
            else if(strcmp(conf_feature_sap, "PREMIUM") == 0) {
                LOC_LOGD("%s:%d]: Setting SAP to mode: PREMIUM", __func__, __LINE__);
                sap_mode = PREMIUM;
            }
            else if(strcmp(conf_feature_sap, "DISABLED") == 0) {
                LOC_LOGD("%s:%d]: Setting SAP to mode: DISABLED", __func__, __LINE__);
            }
            else {
                LOC_LOGE("%s:%d]: Unrecognized value for SAP Mode."     \
                         " Setting SAP to default mode: BASIC", __func__, __LINE__);
                sap_mode = BASIC;
            }

            mLBSApi->injectFeatureConfig(sap_mode, gtp_cell_mode);
        }
    };

    struct LBSProxyInjectFeatureConfig : public LocMsg {
        InjectAdapter mAdapter;
        inline LBSProxyInjectFeatureConfig(ContextBase *context) :
            LocMsg(), mAdapter(context)
        {
            locallog();
        }
        virtual void proc() const {
            mAdapter.injectFeatureConfig();
        }
        virtual void locallog()
        {
            LOC_LOGD("%s:%d]:LBSProxyInjectFeatureConfig", __func__, __LINE__);
        }
    };
    LOC_LOGD("%s:%d]: Enter", __func__, __LINE__);
    context->sendMsg(new LBSProxyInjectFeatureConfig(context));
    LOC_LOGD("%s:%d]: Exit", __func__, __LINE__);
}

bool LBSProxy :: reportPositionsToUlp(GpsExtLocation * locations,
                                      int32_t number_of_locations,
                                      enum loc_sess_status status,
                                      LocPosTechMask techMask)
{
    LOC_LOGD("%s:%d]: Enter", __func__, __LINE__);
    if(mUlp != NULL) {
        LOC_LOGD("%s:%d]: Exit. Sent to ulp", __func__, __LINE__);
        return mUlp->reportPositions(locations,
                                     number_of_locations,
                                     status,
                                     techMask);
    }
    else {
        LOC_LOGD("%s:%d]: Exit", __func__, __LINE__);
        return false;
    }
}

bool LBSProxy :: reportBatchingSessionToUlp(GpsExtBatchOptions &options,
                                            bool active)
{
    LOC_LOGD("%s:%d]: Enter", __func__, __LINE__);
    if(mUlp != NULL) {
        LOC_LOGD("%s:%d]: Exit. Sent to ulp", __func__, __LINE__);
        return mUlp->reportBatchingSession(options, active);
    }
    else {
        LOC_LOGD("%s:%d]: Exit", __func__, __LINE__);
        return false;
    }
}

void LBSProxy :: informShutdown()
{
    if(mAdapter) {
        LOC_LOGD("%s:%d]: Informing adapter of shutdown", __func__, __LINE__);
        mAdapter->shutdown();
    }
    else {
        LOC_LOGE("%s:%d]: Adapter not present", __func__, __LINE__);
    }
}

}  // namespace lbs_core

extern "C" {
LBSProxyBase* getLBSProxy() {
    return new lbs_core::LBSProxy();
}
}
