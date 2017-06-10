/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_LBSApiRpc"
#include <log_util.h>
#include <loc_api_fixup.h>
#include <LBSApiRpc.h>
#include <IzatApiRpc.h>
using namespace loc_core;

namespace lbs_core {

// Conversion factor for Latitude and Longitude
// Convert from degrees to units required by locapi
#define COARSE_LAT_SCALE_FACTOR 23860929.4222
#define COARSE_LON_SCALE_FACTOR 11930464.7111

#define LAT_DEGREE_TO_RPC(x) ((int)(x*COARSE_LAT_SCALE_FACTOR))
#define LONG_DEGREE_TO_RPC(x) ((int)(x*COARSE_LON_SCALE_FACTOR))

int LBSApiRpc::locEventCB(rpc_loc_client_handle_type handle,
                          rpc_loc_event_mask_type event,
                          const rpc_loc_event_payload_u_type* payload)
{
    int ret = -1;
    if (event & RPC_LOC_EVENT_WPS_NEEDED_REQUEST) {
        WpsEvent(payload->rpc_loc_event_payload_u_type_u.qwip_request);
        ret = 0;
    }
    else if(((LocApiProxyRpc *)mLocApiProxy)->locEventCB(handle, event, payload)) {
        ret = 0;
    }
    else if(LocApiRpc::locEventCB(handle, event, payload)) {
        ret = 0;
    }
    return ret;
}

void LBSApiRpc::WpsEvent(const rpc_loc_qwip_request_s_type& wiperReq)
{
    enum WifiRequestType type;

    switch (wiperReq.request_type) {
    case RPC_LOC_QWIP_START_PERIODIC_HI_FREQ_FIXES:
        type = HIGH;
        break;
    case RPC_LOC_QWIP_START_PERIODIC_KEEP_WARM:
        type = LOW;
        break;
    case RPC_LOC_QWIP_STOP_PERIODIC_FIXES:
        type = STOP;
        break;
    default:
        type = UNKNOWN;
    }

    requestWps(type);
}

int LBSApiRpc::wifiStatusInform()
{
    rpc_loc_ioctl_data_u_type         ioctl_data;
    int                               ret_val;

    LOC_LOGV("%s:%d] informing wifi status ...\n", __func__, __LINE__);

    ioctl_data.rpc_loc_ioctl_data_u_type_u.wiper_status = RPC_LOC_WIPER_STATUS_AVAILABLE;

    ret_val = loc_eng_ioctl (client_handle,
                             RPC_LOC_IOCTL_NOTIFY_WIPER_STATUS,
                             &ioctl_data,
                             1000,  // time out in ms
                             NULL /* No output information is expected*/);

    return (ret_val == 0);
}

int LBSApiRpc::injectWifiPosition(const WifiLocation &wifiInfo)
{
    int                                   ret_val;
    rpc_loc_ioctl_data_u_type             ioctl_data;
    rpc_loc_wiper_position_report_s_type* report =
        &(ioctl_data.rpc_loc_ioctl_data_u_type_u.wiper_pos);

    memset(&ioctl_data, 0, sizeof(ioctl_data));

    // time - 0x1
    // pos  - 0x2
    // ap   - 0x4
    report->wiper_valid_info_flag = 0x3;

    report->wiper_fix_position.lat = LAT_DEGREE_TO_RPC(wifiInfo.latitude);
    report->wiper_fix_position.lon = LONG_DEGREE_TO_RPC(wifiInfo.longitude);
    report->wiper_fix_position.HEPE = (uint16_t)wifiInfo.accuracy;
    report->wiper_fix_position.num_of_aps_used = (uint8_t)wifiInfo.numApsUsed;
    report->wiper_fix_position.fix_error_code = (uint8_t)wifiInfo.fixError;

    report->wiper_fix_time.slow_clock_count = 0xFFFFFFFF;//cputime_to_msecs(jiffies)

    if(wifiInfo.apInfoValid){
        report->wiper_valid_info_flag |= 0x4;
        report->wiper_ap_set.num_of_aps = (uint8_t)wifiInfo.numApsUsed;

        for(int i=0;i<MAX_REPORTED_APS && i < 50;i++)
        {
            report->wiper_ap_set.ap_info[i].mac_addr[0] = 0;
            report->wiper_ap_set.ap_info[i].mac_addr[1] = 0;
            report->wiper_ap_set.ap_info[i].mac_addr[2] = 0;
            report->wiper_ap_set.ap_info[i].mac_addr[3] = 0;
            report->wiper_ap_set.ap_info[i].mac_addr[4] = 0;
            report->wiper_ap_set.ap_info[i].mac_addr[5] = 0;
            report->wiper_ap_set.ap_info[i].rssi = wifiInfo.apInfo[i].rssi;
            report->wiper_ap_set.ap_info[i].channel =
                             (uint16_t)wifiInfo.apInfo[i].channel;
            report->wiper_ap_set.ap_info[i].ap_qualifier = 0;
            LOC_LOGV("%s:%d] rssi[%d] = %d and channel[%d] = %d ...\n",
                     __func__, __LINE__, i ,
                     wifiInfo.apInfo[i].rssi,i, wifiInfo.apInfo[i].channel);
        }
    }

    ret_val = loc_eng_ioctl (client_handle,
                             RPC_LOC_IOCTL_SEND_WIPER_POSITION_REPORT,
                             &ioctl_data,
                             1000,  // time out in ms
                             NULL /* No output information is expected*/);

    return (ret_val == 0);
}

LBSApiRpc :: LBSApiRpc(const MsgTask* msgTask,
                       LOC_API_ADAPTER_EVENT_MASK_T exMask) :
    LocApiRpc(msgTask, exMask),
    LBSApiBase(new LocApiProxyRpc(this)),
{
    LOC_LOGD("%s:%d]: LBSApiRpc created. lbsapi: %p, izatapi: %p\n",
             __func__, __LINE__, this, mIzatApi);
}

}  // namespace lbs_core
