/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef LBS_API_RPC_H
#define LBS_API_RPC_H
#include <LocApiRpc.h>
#include <LBSApiBase.h>

namespace izat_core {
    class IzatApiRpc;
};

using namespace loc_core;
using namespace izat_core;
namespace lbs_core {

class LBSApiRpc : public LocApiRpc, public LBSApiBase {
    void WpsEvent(const rpc_loc_qwip_request_s_type& event);
public:
    //Declaring LocApiProxyRpc as a friend class so that it
    //can have access to the client handle
    friend class LocApiProxyRpc;
    LBSApiRpc(const MsgTask* msgTask,
              LOC_API_ADAPTER_EVENT_MASK_T exMask);
    inline ~LBSApiRpc() {}

    inline virtual void* getSibling() { return (LBSApiBase*)this; }
    virtual int locEventCB(rpc_loc_client_handle_type handle,
                   rpc_loc_event_mask_type event,
                   const rpc_loc_event_payload_u_type* payload);

    virtual int wifiStatusInform();
    virtual int injectWifiPosition(const WifiLocation &wifiInfo);
};

}  // namespace lbs_core

#endif //LBS_API_RPC_H
