#include "sci_types.h"
#include "libmem.h"
#include "dm_cbfunc.h"
#include "dm_error.h"
#include "dm_pl_tree.h"
#include "dm_tree.h"

/*****************************************************************************/
//  Description : get vdm info string
//  Global resource dependence : none
//  Author:
//  Note:
/*****************************************************************************/

//add  for node handler
const int DEVID_IO_HANDLER = 0;
const int FROMFILE_IO_HANDLER = 1;
const int MODEL_IO_HANDLER = 2;
const int MAN_IO_HANDLER = 3;
const int OEM_IO_HANDLER = 4;

const int LANG_IO_HANDLER = 5;
const int DMVERSION_IO_HANDLER = 6;

const int FWVERSION_IO_HANDLER = 7;
const int SWVERSION_IO_HANDLER = 8;
const int HWVERSION_IO_HANDLER = 9;
const int SERVER_ADDR_IO_HANDLER = 10;

//DM Setting
const int DM_CONN_PROFILE_IO_HANDLER = 11;
const int DM_APN_IO_HANDLER = 12;
const int DM_PROXY_IO_HANDLER = 13;
const int DM_PORT_IO_HANDLER = 14;

//GPRS-CMNET Setting
const int GPRS_CMNET_APN_IO_HANDLER = 15;
const int GPRS_CMNET_PROXY_IO_HANDLER = 16;
const int GPRS_CMNET_PORT_IO_HANDLER = 17;
const int GPRS_CMNET_USERNAME_IO_HANDLER = 18;
const int GPRS_CMNET_PASSWORD_IO_HANDLER = 19;

//GPRS-CMWAP Setting
const int GPRS_CMWAP_APN_IO_HANDLER = 20;
const int GPRS_CMWAP_PROXY_IO_HANDLER = 21;
const int GPRS_CMWAP_PORT_IO_HANDLER = 22;
const int GPRS_CMWAP_USERNAME_IO_HANDLER = 23;
const int GPRS_CMWAP_PASSWORD_IO_HANDLER = 24;

//WAP Setting
const int WAP_CONNPROFILE_IO_HANDLER = 25;
const int WAP_HOMEPAGE_IO_HANDLER = 26;
const int WAP_PROXY_IO_HANDLER = 27;
const int WAP_PORT_IO_HANDLER = 28;
const int WAP_USERNAME_IO_HANDLER = 29;
const int WAP_PASSWORD_IO_HANDLER = 30;

//MMS Setting
const int MMS_CONNPROFILE_IO_HANDLER = 31;
const int MMS_MMSC_IO_HANDLER = 32;

//PIM Setting
const int PIM_CONNPROFILE_URI_IO_HANDLER = 33;
const int PIM_SERVER_ADDR_IO_HANDLER = 34;
const int PIM_ADDRESS_BOOK_URI_IO_HANDLER = 35;
const int PIM_CALENDAR_URI_IO_HANDLER = 36;

//PushMail Setting
const int MAIL_CONNPROFILE_IO_HANDER = 37;
const int MAIL_SEND_SERVER_IO_HANDER = 38;
const int MAIL_SEND_PORT_IO_HANDER = 39;
const int MAIL_SEND_USE_SEC_CON_IO_HANDER = 40;
const int MAIL_RECV_SERVER_IO_HANDER = 41;
const int MAIL_RECV_PORT_IO_HANDER = 42;
const int MAIL_RECV_USE_SEC_CON_IO_HANDER = 43;
const int MAIL_RECV_PROTOCAL_IO_HANDER = 44;

//Streaming Setting
const int STREAMING_CONNPROFILE_IO_HANDLER = 45;
const int STREAMING_NAME_IO_HANDLER = 46;
const int STREAMING_MAX_UDP_PORT_IO_HANDLER = 47;
const int STREAMING_MIN_UDP_PORT_IO_HANDLER = 48;
const int STREAMING_NET_INFO_IO_HANDLER = 49;

//AGPS Setting
const int AGPS_CONNPROFILE_IO_HANDLER = 50;
const int AGPS_SERVER_IO_HANDLER = 51;
const int AGPS_SERVER_NAME_IO_HANDLER = 52;
const int AGPS_IAPID_IO_HANDLER = 53;
const int AGPS_PORT_IO_HANDLER = 54;
const int AGPS_PROVIDER_ID_IO_HANDLER = 55;

IS32 DM_GetFormatString(char* string,    //in
        IS32 offset,    //in
        IS8** data,      //out
        IS32* maxSize    //out
        ) {
    if (string == NULL || SCI_STRLEN(string) == 0) {
        DM_TRACE("MMIDM==> DM_GetFormatString: string is null!");
        *data = NULL;
        *maxSize = 0;
        return DM_ERROR;
    }

    DM_TRACE("MMIDM==> DM_GetFormatString: string is %s", string);
    *data = (IS8 *) SCI_ALLOC(SCI_STRLEN(string) + 1);
    if (NULL != *data) {
        strlcpy(*data, string, sizeof(*data));
        DM_TRACE("MMIDM==> DM_GetFormatString: *data is %s", *data);
        if (NULL != maxSize) {
            *maxSize = SCI_STRLEN(string);
        }
    } else {
        DM_TRACE("MMIDM==> DM_GetFormatString: malloc fail!");
        return DM_ERROR;
    }

    return DM_OK;
}

/*This is the callback function of node :DevId.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevInfoDevId_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevInfoDevId_CBFunc ");
//  string = DM_GetDevImeiInfo();
    string = MMIDM_GetCBFunc(DEVID_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :Man.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevInfoMan_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevInfoMan_CBFunc ");
//  string = MMIDM_GetMan();
    string = MMIDM_GetCBFunc(MAN_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :Mod.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevInfoMod_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevInfoMod_CBFunc ");
//  string = MMIDM_GetModel();
    string = MMIDM_GetCBFunc(MODEL_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :DmV.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevInfoDmV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevInfoDmV_CBFunc ");
//  string = MMIDM_GetDMVersion();
    string = MMIDM_GetCBFunc(DMVERSION_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :Lang.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevInfoLang_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevInfoLang_CBFunc ");
//  string = MMIDM_GetLanguage();
    string = MMIDM_GetCBFunc(LANG_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :DevTyp.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevDetailDevTyp_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = "phone";

    DM_TRACE("MMIDM==> Get_DevDetailDevTyp_CBFunc ");

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :OEM.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevDetailOEM_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevDetailOEM_CBFunc ");
//  string = MMIDM_GetOEM();
    string = MMIDM_GetCBFunc(OEM_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :FwV.*/
//Do Not Modified Unless necessarily!!!
IS32 Get_DevDetailFwV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevDetailFwV_CBFunc ");
//  string = MMIDM_GetFirmwareVersion();
    string = MMIDM_GetCBFunc(FWVERSION_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :SwV.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevDetailSwV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevDetailSwV_CBFunc ");
//  string = MMIDM_GetSoftwareVersion();
    string = MMIDM_GetCBFunc(SWVERSION_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :HwV.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevDetailHwV_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_DevDetailHwV_CBFunc ");
//  string = MMIDM_GetHardwareVersion();
    string = MMIDM_GetCBFunc(HWVERSION_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :LrgObj.*/
/*DO NOT MODIFY THIS FUNCTION UNELESS NECESSORYILY!!!*/
IS32 Get_DevDetailLrgObj_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = "false";

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 Get_ServerAddr_CBFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> Get_ServerAddr_CBFunc ");
//  string = MMIDM_GetSrvAddURL();

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 ServerNonceGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

IS32 ServerNonceReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

IS32 ClientNonceGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

IS32 ClientNonceReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*This is the callback function of node :APN.*/
IS32 GPRSCmnetAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmnetAPNGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_APN,DM_CMNET);
    string = MMIDM_GetCBFunc(GPRS_CMNET_APN_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :APN.*/
IS32 GPRSCmnetAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmnetAPNReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_APN,(char*)data,maxSize,DM_CMNET))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmnetAPNReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMNET_APN_IO_HANDLER, offset, (char*) data, maxSize);

    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 GPRSCmnetUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmnetUserNameGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_USER_NAME,DM_CMNET);
    string = MMIDM_GetCBFunc(GPRS_CMNET_USERNAME_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :UserName.*/
IS32 GPRSCmnetUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmnetUserNameReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_USER_NAME,(char*)data,maxSize,DM_CMNET))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmnetUserNameReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMNET_USERNAME_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :PassWord.*/
IS32 GPRSCmnetPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmnetPassWordGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_PW,DM_CMNET);
    string = MMIDM_GetCBFunc(GPRS_CMNET_PASSWORD_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :PassWord.*/
IS32 GPRSCmnetPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmnetPassWordReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_PW,(char*)data,maxSize,DM_CMNET))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmnetPassWordReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMNET_PASSWORD_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ProxyAddr.*/
IS32 GPRSCmnetProxyAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmnetProxyAddrGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_PROXY_ADDR,DM_CMNET);
    string = MMIDM_GetCBFunc(GPRS_CMNET_PROXY_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ProxyAddr.*/
IS32 GPRSCmnetProxyAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmnetProxyAddrReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_PROXY_ADDR,(char*)data,maxSize,DM_CMNET))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmnetProxyAddrReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMNET_PROXY_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ProxyPortNbr.*/
IS32 GPRSCmnetProxyPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmnetProxyPortNbrGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_PORT_NUM,DM_CMNET);
    string = MMIDM_GetCBFunc(GPRS_CMNET_PORT_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ProxyPortNbr.*/
IS32 GPRSCmnetProxyPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmnetProxyPortNbrReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_PORT_NUM,(char*)data,maxSize,DM_CMNET))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmnetProxyPortNbrReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMNET_PORT_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :APN.*/
IS32 GPRSCmwapAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmwapAPNGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_APN,DM_CMWAP);
    string = MMIDM_GetCBFunc(GPRS_CMWAP_APN_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :APN.*/
IS32 GPRSCmwapAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmwapAPNReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_APN,(char*)data,maxSize,DM_CMWAP))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmwapAPNReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMWAP_APN_IO_HANDLER, offset, (char*) data, maxSize);

    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 GPRSCmwapUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmwapUserNameGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_USER_NAME,DM_CMWAP);
    string = MMIDM_GetCBFunc(GPRS_CMWAP_USERNAME_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :UserName.*/
IS32 GPRSCmwapUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmwapUserNameReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_USER_NAME,(char*)data,maxSize,DM_CMWAP))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmwapUserNameReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMWAP_USERNAME_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :PassWord.*/
IS32 GPRSCmwapPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmwapPassWordGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_PW,DM_CMWAP);
    string = MMIDM_GetCBFunc(GPRS_CMWAP_PASSWORD_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :PassWord.*/
IS32 GPRSCmwapPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmwapPassWordReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_PW,(char*)data,maxSize,DM_CMWAP))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmwapPassWordReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMWAP_PASSWORD_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ProxyAddr.*/
IS32 GPRSCmwapProxyAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmwapProxyAddrGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_PROXY_ADDR,DM_CMWAP);
    string = MMIDM_GetCBFunc(GPRS_CMWAP_PROXY_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ProxyAddr.*/
IS32 GPRSCmwapProxyAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmwapProxyAddrReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_PROXY_ADDR,(char*)data,maxSize,DM_CMWAP))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmwapProxyAddrReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMWAP_PROXY_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ProxyPortNbr.*/
IS32 GPRSCmwapProxyPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> GPRSCmwapProxyPortNbrGetFunc ");
//  string = MMIDM_GetGPRSInfo(DM_WAP_PORT_NUM,DM_CMWAP);
    string = MMIDM_GetCBFunc(GPRS_CMWAP_PORT_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ProxyPortNbr.*/
IS32 GPRSCmwapProxyPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> GPRSCmwapProxyPortNbrReplaceFunc ");
//    if (MMIDM_SetGprsInfo(DM_WAP_PORT_NUM,(char*)data,maxSize,DM_CMWAP))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> GPRSCmwapProxyPortNbrReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(GPRS_CMWAP_PORT_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ConnProfile.*/
IS32 WAPConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> WAPConnProfileGetFunc ");
//  string = DM_GetWAPInfo(DM_WAP_APN);
    string = MMIDM_GetCBFunc(WAP_CONNPROFILE_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ConnProfile.*/
IS32 WAPConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> WAPConnProfileReplaceFunc ");
//    if (DM_SetWAPInfo(DM_WAP_APN,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> WAPConnProfileReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(WAP_CONNPROFILE_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ProxyAddr.*/
IS32 WAPProxyAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :ProxyAddr.*/
IS32 WAPProxyAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :ProxyPortNbr.*/
IS32 WAPProxyPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :ProxyPortNbr.*/
IS32 WAPProxyPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :StartPage.*/
IS32 WAPStartPageGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> WAPStartPageGetFunc ");
//  string = DM_GetWAPInfo(DM_WAP_HOME_PAGE);
    string = MMIDM_GetCBFunc(WAP_HOMEPAGE_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :StartPage.*/
IS32 WAPStartPageReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> WAPStartPageReplaceFunc ");
//    if (DM_SetWAPInfo(DM_WAP_HOME_PAGE,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> WAPStartPageReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(WAP_HOMEPAGE_IO_HANDLER, offset, (char*) data, maxSize);

    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 WAPUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 WAPUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :PassWord.*/
IS32 WAPPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :PassWord.*/
IS32 WAPPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :APN.*/
IS32 WAPAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :APN.*/
IS32 WAPAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :MMSC.*/
IS32 MMSMMSCGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> MMSMMSCGetFunc ");
//  string = MMIDM_GetMMSCInfo(DM_MMS_MMSC);
    string = MMIDM_GetCBFunc(MMS_MMSC_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :MMSC.*/
IS32 MMSMMSCReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> MMSMMSCReplaceFunc ");
//    if (MMIDM_SetMMSCInfo(DM_MMS_MMSC,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> MMSMMSCReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(MMS_MMSC_IO_HANDLER, offset, (char*) data, maxSize);

    return DM_OK;
}

/*This is the callback function of node :ConnProfile.*/
IS32 MMSConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> MMSConnProfileGetFunc ");
//  string = MMIDM_GetMMSCInfo(DM_MMS_CONN_PROFILE);
    string = MMIDM_GetCBFunc(MMS_CONNPROFILE_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ConnProfile.*/
IS32 MMSConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> MMSConnProfileReplaceFunc ");
//    if (MMIDM_SetMMSCInfo(DM_MMS_CONN_PROFILE,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> MMSConnProfileReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(MMS_CONNPROFILE_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :SRNumber.*/
IS32 SRSRNumberGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :SRNumber.*/
IS32 SRSRNumberReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :SRPort.*/
IS32 SRSRPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :SRPort.*/
IS32 SRSRPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :APN.*/
IS32 DMAPNGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> DMAPNGetFunc ");
//  string = MMIDM_GetDmProfile();
    string = MMIDM_GetCBFunc(DM_APN_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :APN.*/
IS32 DMAPNReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> DMAPNReplaceFunc ");
//    if (MMIDM_SetDmProfile((char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> DMAPNReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(DM_APN_IO_HANDLER, offset, (char*) data, maxSize);

    return DM_OK;
}
/*This is the callback function of node :ConnProfile.*/
IS32 DMConProGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> DMCONPROGetFunc ");
//  string = MMIDM_GetDmProfile();
    string = MMIDM_GetCBFunc(DM_CONN_PROFILE_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ConnProfile.*/
IS32 DMConProReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> DMCONPROReplaceFunc ");
//    if (MMIDM_SetDmProfile((char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> DMAPNReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(DM_CONN_PROFILE_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 DMUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 DMUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :Password.*/
IS32 DMPasswordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :Password.*/
IS32 DMPasswordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :Proxy.*/
IS32 DMProxyGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;
    DM_TRACE("MMIDM==> DMProxyGetFunc ");
    string = MMIDM_GetCBFunc(DM_PROXY_IO_HANDLER, offset);
    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :Proxy.*/
IS32 DMProxyReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> DMProxyReplaceFunc ");
    MMIDM_ReplaceFunc(DM_PROXY_IO_HANDLER, offset, (char*) data, maxSize);
    return DM_OK;

}

/*This is the callback function of node :Port.*/
IS32 DMPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> DMPortGetFunc ");
    string = MMIDM_GetCBFunc(DM_PORT_IO_HANDLER, offset);
    return DM_GetFormatString(string, offset, data, maxSize);

}

/*This is the callback function of node :Port.*/
IS32 DMPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> DMPortReplaceFunc ");
    MMIDM_ReplaceFunc(DM_PORT_IO_HANDLER, offset, (char*) data, maxSize);
    return DM_OK;

}

/*This is the callback function of node :Addr.*/
IS32 PIMAddrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PIMAddrGetFunc ");
//  string = MMIDM_GetPIMInfo(DM_PIM_ADDR);
    string = MMIDM_GetCBFunc(PIM_SERVER_ADDR_IO_HANDLER, offset);

    DM_TRACE("MMIDM==> PIMAddrGetFunc : string = %s", string);
    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :Addr.*/
IS32 PIMAddrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> PIMAddrReplaceFunc ");
//    if (MMIDM_SetPIMInfo(DM_PIM_ADDR,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> PIMAddrReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }

    MMIDM_ReplaceFunc(PIM_SERVER_ADDR_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}
/*This is the callback function of node :ConnProfile.*/
IS32 PIMConProGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PIMConProGetFunc ");
//  string = MMIDM_GetPIMInfo(DM_PIM_ADDR);
    string = MMIDM_GetCBFunc(PIM_CONNPROFILE_URI_IO_HANDLER, offset);

    DM_TRACE("MMIDM==> PIMConProGetFunc : string = %s", string);
    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ConnProfile.*/
IS32 PIMConProReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> PIMConProReplaceFunc ");
//    if (MMIDM_SetPIMInfo(DM_PIM_ADDR,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> PIMAddrReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }

    MMIDM_ReplaceFunc(PIM_CONNPROFILE_URI_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :AddressBookURI.*/
IS32 PIMAddressBookURIGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PIMAddressBookURIGetFunc ");
//  string = MMIDM_GetPIMInfo(DM_PIM_ADDRESSBOOKURL);
    string = MMIDM_GetCBFunc(PIM_ADDRESS_BOOK_URI_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :AddressBookURI.*/
IS32 PIMAddressBookURIReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PIMAddressBookURIReplaceFunc ");
//    if (MMIDM_SetPIMInfo(DM_PIM_ADDRESSBOOKURL,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> PIMAddressBookURIReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(PIM_ADDRESS_BOOK_URI_IO_HANDLER, offset, (char*) data,
            maxSize);
    return DM_OK;
}

/*This is the callback function of node :CalendarURI.*/
IS32 PIMCalendarURIGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PIMCalendarURIGetFunc ");
//  string = MMIDM_GetPIMInfo(DM_PIM_CALENDARURL);
    string = MMIDM_GetCBFunc(PIM_CALENDAR_URI_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :CalendarURI.*/
IS32 PIMCalendarURIReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> PIMCalendarURIReplaceFunc ");
//    if (MMIDM_SetPIMInfo(DM_PIM_CALENDARURL,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> PIMCalendarURIReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(PIM_CALENDAR_URI_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :PortNbr.*/
IS32 PIMPortNbrGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :PortNbr.*/
IS32 PIMPortNbrReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 PIMUserNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :UserName.*/
IS32 PIMUserNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :PassWord.*/
IS32 PIMPassWordGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    return DM_OK;
}

/*This is the callback function of node :PassWord.*/
IS32 PIMPassWordReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    return DM_OK;
}

/*This is the callback function of node :Name.*/
IS32 StreamingNameGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingNameGetFunc ");
//  string = MMIDM_GetStreamingInfo(DM_ST_NAME);
    string = MMIDM_GetCBFunc(STREAMING_NAME_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :Name.*/
IS32 StreamingNameReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingNameReplaceFunc ");
//    if (MMIDM_SetStreamingInfo(DM_ST_NAME,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> StreamingNameReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(STREAMING_NAME_IO_HANDLER, offset, (char*) data, maxSize);

    return DM_OK;
}

/*This is the callback function of node :Proxy.*/
IS32 StreamingProxyGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingProxyGetFunc ");
//  string = MMIDM_GetStreamingInfo(DM_ST_PROXY);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :Proxy.*/
IS32 StreamingProxyReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingProxyReplaceFunc ");
//    if (MMIDM_SetStreamingInfo(DM_ST_PROXY,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> StreamingProxyReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    return DM_OK;
}

/*This is the callback function of node :ProxyPort.*/
IS32 StreamingProxyPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingProxyPortGetFunc ");
//  string = MMIDM_GetStreamingInfo(DM_ST_PROXYPORT);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ProxyPort.*/
IS32 StreamingProxyPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingProxyPortReplaceFunc ");
//    if (MMIDM_SetStreamingInfo(DM_ST_PROXYPORT,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> StreamingProxyPortReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    return DM_OK;
}

/*This is the callback function of node :ConnProfile.*/
IS32 StreamingConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingConnProfileGetFunc ");
//  string = MMIDM_GetStreamingInfo(DM_ST_APN);
    string = MMIDM_GetCBFunc(STREAMING_CONNPROFILE_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ConnProfile.*/
IS32 StreamingConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingConnProfileReplaceFunc ");
//    if (MMIDM_SetStreamingInfo(DM_ST_APN,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> StreamingConnProfileReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(STREAMING_CONNPROFILE_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :NetInfo.*/
IS32 StreamingNetInfoGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingNetInfoGetFunc ");
//  string = MMIDM_GetStreamingInfo(DM_ST_NETINFO);
    string = MMIDM_GetCBFunc(STREAMING_NET_INFO_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :NetInfo.*/
IS32 StreamingNetInfoReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingNetInfoReplaceFunc ");
//    if (MMIDM_SetStreamingInfo(DM_ST_NETINFO,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> StreamingNetInfoReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(STREAMING_NET_INFO_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :MinUdpPort.*/
IS32 StreamingMinUdpPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingMinUdpPortGetFunc ");
//  string = MMIDM_GetStreamingInfo(DM_ST_MIN_UDP_PORT);
    string = MMIDM_GetCBFunc(STREAMING_MIN_UDP_PORT_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :MinUdpPort.*/
IS32 StreamingMinUdpPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingMinUdpPortReplaceFunc ");
//    if (MMIDM_SetStreamingInfo(DM_ST_MIN_UDP_PORT,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> StreamingMinUdpPortReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(STREAMING_MIN_UDP_PORT_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :MaxUdpPort.*/
IS32 StreamingMaxUdpPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingMaxUdpPortGetFunc ");
//  string = MMIDM_GetStreamingInfo(DM_ST_MAX_UDP_PORT);
    string = MMIDM_GetCBFunc(STREAMING_MAX_UDP_PORT_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :MaxUdpPort.*/
IS32 StreamingMaxUdpPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingMaxUdpPortReplaceFunc ");
//    if (MMIDM_SetStreamingInfo(DM_ST_MAX_UDP_PORT,(char*)data,maxSize))
//    {
//        return DM_OK;
//    }
//    else
//    {
//        DM_TRACE("MMIDM==> StreamingMaxUdpPortReplaceFunc: Set fail! ");
//        return DM_ERROR;
//    }
    MMIDM_ReplaceFunc(STREAMING_MAX_UDP_PORT_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

//add for push mail

IS32 PushMailSendUseSecConGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PushMailSendUseSecConGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_SEND_USE_SEC_CON_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailSendUseSecConReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailSendUseSecConReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_SEND_USE_SEC_CON_IO_HANDER, offset, (char*) data,
            maxSize);

    return DM_OK;
}
IS32 PushMailSendServerGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PushMailSendServerGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_SEND_SERVER_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailSendServerReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailSendServerReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_SEND_SERVER_IO_HANDER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

IS32 PushMailSendPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PushMailSendPortGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_SEND_PORT_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailSendPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailSendPortReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_SEND_PORT_IO_HANDER, offset, (char*) data, maxSize);

    return DM_OK;
}

IS32 PushMailRecvProGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PushMailRecvProGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_RECV_PROTOCAL_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailRecvProReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailRecvProReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_RECV_PROTOCAL_IO_HANDER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

IS32 PushMailRecvUseSecConGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PushMailRecvUseSecConGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_RECV_USE_SEC_CON_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailRecvUseSecConReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailRecvUseSecConReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_RECV_USE_SEC_CON_IO_HANDER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

IS32 PushMailRecvServerGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> PushMailRecvServerGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_RECV_SERVER_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailRecvServerReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailRecvServerReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_RECV_SERVER_IO_HANDER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

IS32 PushMailRecvPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;
    DM_TRACE("MMIDM==> PushMailRecvPortGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_RECV_PORT_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailRecvPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailRecvPortReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_RECV_PORT_IO_HANDER, offset, (char*) data, maxSize);

    return DM_OK;
}
IS32 PushMailConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8** data, IS32* maxSize) {
    char* string = PNULL;
    DM_TRACE("MMIDM==> PushMailConnProfileGetFunc ");
    string = MMIDM_GetCBFunc(MAIL_CONNPROFILE_IO_HANDER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

IS32 PushMailConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 maxSize) {
    DM_TRACE("MMIDM==> PushMailConnProfileReplaceFunc ");

    MMIDM_ReplaceFunc(MAIL_CONNPROFILE_IO_HANDER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ConnProfile.*/
IS32 AgpsConnProfileGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingConnProfileGetFunc ");
    string = MMIDM_GetCBFunc(AGPS_CONNPROFILE_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ConnProfile.*/
IS32 AgpsConnProfileReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingConnProfileReplaceFunc ");
    MMIDM_ReplaceFunc(AGPS_CONNPROFILE_IO_HANDLER, offset, (char*) data,
            maxSize);

    return DM_OK;
}

/*This is the callback function of node :ConnProfile.*/
IS32 AgpsServerPortGetFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize) {
    char* string = PNULL;

    DM_TRACE("MMIDM==> StreamingConnProfileGetFunc ");
    string = MMIDM_GetCBFunc(AGPS_SERVER_IO_HANDLER, offset);

    return DM_GetFormatString(string, offset, data, maxSize);
}

/*This is the callback function of node :ConnProfile.*/
IS32 AgpsServerPortReplaceFunc(DM_TREE_HANDLE treeHndl, IS32 offset, IS8* data,
        IS32 maxSize) {
    DM_TRACE("MMIDM==> StreamingConnProfileReplaceFunc ");
    MMIDM_ReplaceFunc(AGPS_SERVER_IO_HANDLER, offset, (char*) data, maxSize);

    return DM_OK;
}

