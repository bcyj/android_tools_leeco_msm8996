/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef IZAT_API_RPC_H
#define IZAT_API_RPC_H

#include <LocApiRpc.h>
#include <IzatApiBase.h>

namespace lbs_core {
    class LBSApiRpc;
};

using namespace loc_core;
using namespace lbs_core;

namespace izat_core {

class IzatApiRpc : public IzatApiBase {

public:
    LBSApiRpc *mLBSApi;
    inline IzatApiRpc(LocApiProxyRpc* locApiProxy);
    inline ~IzatApiRpc() {}

    virtual int locEventCB(rpc_loc_client_handle_type handle,
                   rpc_loc_event_mask_type event,
                   const rpc_loc_event_payload_u_type* payload);
    virtual int setUserPref(bool pref);
};

}  // namespace izat_core

#endif //IZAT_API_RPC_H
