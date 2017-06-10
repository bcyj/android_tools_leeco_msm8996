#ifndef _CBFUNC_H
#define _CBFUNC_H

#include "vdm_pl_types.h"
#include "dm_pl_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

IS32 Get_DevInfoDevId_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevInfoMan_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevInfoMod_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevInfoDmV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevInfoLang_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevDetailDevTyp_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 Get_DevDetailOEM_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevDetailFwV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevDetailSwV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevDetailHwV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 Get_DevDetailLrgObj_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 Get_ServerAddr_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);

IS32 ServerNonceGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 ServerNonceReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 ClientNonceGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 ClientNonceReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

IS32 GPRSCmnetAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmnetAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 GPRSCmnetUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmnetUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 GPRSCmnetPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmnetPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 GPRSCmnetProxyAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmnetProxyAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 GPRSCmnetProxyPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 GPRSCmnetProxyPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);

IS32 GPRSCmwapAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmwapAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 GPRSCmwapUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmwapUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 GPRSCmwapPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmwapPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 GPRSCmwapProxyAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 GPRSCmwapProxyAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 GPRSCmwapProxyPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 GPRSCmwapProxyPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);

IS32 WAPConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 WAPConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 WAPProxyAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 WAPProxyAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 WAPProxyPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 WAPProxyPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 WAPStartPageGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 WAPStartPageReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 WAPUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 WAPUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 WAPPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 WAPPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 WAPAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 WAPAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

IS32 MMSMMSCGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 MMSMMSCReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 MMSConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 MMSConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

IS32 SRSRNumberGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 SRSRNumberReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 SRSRPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 SRSRPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

IS32 DMAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 DMAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 DMConProGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 DMConProReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 DMUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 DMUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 DMPasswordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 DMPasswordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 DMProxyGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 DMProxyReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 DMPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 DMPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

IS32 PIMAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PIMAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 PIMConProGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PIMConProReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

IS32 PIMAddressBookURIGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PIMAddressBookURIReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 PIMCalendarURIGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PIMCalendarURIReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 PIMPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PIMPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 PIMUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PIMUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 PIMPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PIMPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

IS32 StreamingNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 StreamingNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 StreamingProxyGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 StreamingProxyReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 StreamingProxyPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 StreamingProxyPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 StreamingConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 StreamingConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 StreamingNetInfoGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 StreamingNetInfoReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 StreamingMinUdpPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 StreamingMinUdpPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 StreamingMaxUdpPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 StreamingMaxUdpPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);

//add  for push mail
IS32 PushMailSendUseSecConGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 PushMailSendUseSecConReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 PushMailSendServerGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PushMailSendServerReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 PushMailSendPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PushMailSendPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 PushMailRecvProGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PushMailRecvProReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 PushMailRecvUseSecConGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 PushMailRecvUseSecConReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 PushMailRecvServerGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PushMailRecvServerReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 PushMailRecvPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 PushMailRecvPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 PushMailConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize);
IS32 PushMailConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize);
IS32 AgpsConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 AgpsConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);
IS32 AgpsServerPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
IS32 AgpsServerPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize);

#ifdef __cplusplus
}
#endif
#endif

