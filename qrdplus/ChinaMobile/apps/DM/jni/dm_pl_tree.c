#include "dm_pl_tree.h"
#include "dm_error.h"
#include "dm_pl_debug.h"
#include "dm_tree.h"

/*
 *  DmAccount Information define
 */
DMTREE_ACC_INFO g_DmAccInfo = { "TDAcc",                // Node Name

        "w7",                   // AppID
        "OMADM",                // Server ID
        "CMCCTest",             // Name
        "",                     // PrefConRef
        "",                     // ConRef

        "http://218.206.176.97:7001",   // Addr URI
        "IPv4",                 // AddrType
        7001,                   // Port Number

        DMTREE_AUTHTYPE_MD5,    // Auth Pref

        "SRVCRED",              // ServerAuthLevel
        DMTREE_AUTHTYPE_MD5,    // ServerAuthType
        "OMADM",                // ServerAuthName
        "mvpdm",                // Server Psw
        "ZtexnABuWuR7El4aJffQSw==", // Server nonce
        "MTEx",                 // Prev server nonce

        "CLCRED",               // ClientAuthLevel
        DMTREE_AUTHTYPE_MD5,    // ClientAuthType
        "mvpdm",                // User name
        "mvpdm",                // User psw
        "JE48dmgiOyogPzxXPkdUJQ=="  // Client nonce
        };

/**
 *  @brief  This function is used for getting information from the device
 *          relating to the DMAcc managed object.
 *
 *  @param  none.
 *
 *  @return The pointer to DMTREE_accInfo struct.
 *
 *  @note   This function is called if the device is pre-configured at
 *          the factory (rather than bootstrapped Over-The-Air).
 */
const DMTREE_ACC_INFO* dmTreeParam_getDmAccInfo(void) {
    return (const DMTREE_ACC_INFO*) &g_DmAccInfo;
}

/**
 *  @brief  This function Get the details of a DM account in the DM tree.
 *
 *  @param  eNodeName  [I]Account infor node name enum.
 *  @param  szNodeUri  [O]the Node URI path; this param can is NULL;
 *  @param  szNodeName [O]node name; this param can is NULL;
 *
 *  @return DMTREE_ERR_OK    - Successful.   \n
 *          DMTREE_ERR_UNDEF - Failed!
 *
 *  @note   none.
 */
IS32 dmTreeParam_getDmAccNodeName(DMTREE_ACC_NODE_NAME_E eNodeName,
        IS8** szNodeUri, IS8** szNodeName) {
    IS8* nodeName = NULL;
    IS8* nodeUri = NULL;

#ifndef SUPPORT_OMA_DM_1_2

    /* Copy the node name and the URI */
    switch ( eNodeName )
    {
        case dmAccNodeName_Addr:
        nodeUri = "Addr";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_AddrType:
        nodeUri = "AddrType";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_PortNbr:
        nodeUri = "PortNbr";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_ConRef:
        nodeUri = "ConRef";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_ServerID:
        nodeUri = "ServerId";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_ServerPw:
        nodeUri = "ServerPW";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_ServerNonce:
        nodeUri = "ServerNonce";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_PrevServerNonce:
        nodeUri = "PrevServerNonce";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_UserName:
        nodeUri = "UserName";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_UserPw:
        nodeUri = "ClientPW";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_ClientNonce:
        nodeUri = "ClientNonce";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_AuthPref:
        nodeUri = "AuthPref";
        nodeName = nodeUri;
        break;
        case dmAccNodeName_Name:
        nodeUri = "Name";
        nodeName = nodeUri;
        break;
        default:
        return DMTREE_ERR_UNDEF;
    }

#else // ifdef SUPPORT_OMA_DM_1_2
    /* Copy the node name and the URI */
    switch (eNodeName) {
    case dmAccNodeName_AppID:
        nodeUri = "AppID";
        nodeName = nodeUri;
        break;
    case dmAccNodeName_ServerID:
        nodeUri = "ServerID";
        nodeName = nodeUri;
        break;
    case dmAccNodeName_Name:
        nodeUri = "Name";
        nodeName = nodeUri;
        break;
    case dmAccNodeName_PrefConRef:
        nodeUri = "PrefConRef";
        nodeName = nodeUri;
        break;
    case dmAccNodeName_ConRef:
        nodeUri = "ToConRef/ConRef";
        nodeName = nodeUri + 9;
        break;

    case dmAccNodeName_Addr:
        nodeUri = "AppAddr/Addr";
        nodeName = nodeUri + 8;
        break;
    case dmAccNodeName_AddrType:
        nodeUri = "AppAddr/AddrType";
        nodeName = nodeUri + 8;
        break;
    case dmAccNodeName_PortNbr:
        nodeUri = "AppAddr/Port/PortNbr";
        nodeName = nodeUri + 13;
        break;

    case dmAccNodeName_AuthPref:
        nodeUri = "AAuthPref";
        nodeName = nodeUri;
        break;

    case dmAccNodeName_ServerAuthLevel:
        nodeUri = "AppAuth/Server/AAuthLevel";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_ServerAuthType:
        nodeUri = "AppAuth/Server/AAuthType";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_ServerAuthName:
        nodeUri = "AppAuth/Server/AAuthName";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_ServerPw:
        nodeUri = "AppAuth/Server/AAuthSecret";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_ServerNonce:
        nodeUri = "AppAuth/Server/AAuthData";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_PrevServerNonce:
        nodeUri = "AppAuth/Server/PrevAAuthData";
        nodeName = nodeUri + 15;
        break;

    case dmAccNodeName_ClientAuthLevel:
        nodeUri = "AppAuth/Client/AAuthLevel";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_ClientAuthType:
        nodeUri = "AppAuth/Client/AAuthType";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_UserName:
        nodeUri = "AppAuth/Client/AAuthName";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_UserPw:
        nodeUri = "AppAuth/Client/AAuthSecret";
        nodeName = nodeUri + 15;
        break;
    case dmAccNodeName_ClientNonce:
        nodeUri = "AppAuth/Client/AAuthData";
        nodeName = nodeUri + 15;
        break;
    default:
        return DMTREE_ERR_UNDEF;
    }

#endif // SUPPORT_OMA_DM_1_2
    if (szNodeUri != NULL ) {
        *szNodeUri = nodeUri;
    }
    if (szNodeName != NULL ) {
        *szNodeName = nodeName;
    }

    return DMTREE_ERR_OK;
}

IBOOL dmTreeParam_getNodeData(const IS8* nodeUri,    //in
        IS8** data,       //out
        IS32* maxSize     //out
        ) {
    DM_TREE_HANDLE tree_handle = NULL;
    DMTREE_ITEM *item_ptr = NULL;
    IS32 result = 0;
    IBOOL isSuccess = FALSE;

    dm_debug_trace(DM_DEBUG_TREE_MASK, "dmTree_getNodeData: nodeUri = %s \r\n",
            nodeUri);

    if ((NULL == nodeUri) || (NULL == data)) {
        dmTree_setErrorCode(DMTREE_ERR_INVALID_PARAM);
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTree_getNodeData: Invalid param! \r\n");
        return FALSE;
    }

    item_ptr = dmTree_getItemEx(tree_handle, nodeUri);
    if (NULL != item_ptr) {
        result = item_ptr->getFunc(tree_handle, 0, data, maxSize);
        if (DM_OK == result) {
            dmTree_setErrorCode(DMTREE_ERR_OK);
            isSuccess = TRUE;
        } else {
            isSuccess = TRUE;
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "dmTree_getNodeData: item_ptr->getFunc return error! \r\n");
        }
    } else {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTree_getNodeData: Get item fail! \r\n");
        isSuccess = FALSE;
    }

    if (NULL != *data)
        dm_debug_trace(DM_DEBUG_TREE_MASK, "dmTree_getNodeData: data = %s \r\n",
                *data);
    if (NULL != maxSize)
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTree_getNodeData: maxSize = %d \r\n", *maxSize);

    return isSuccess;
}

/*****************************************************************************/
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
IBOOL dmTreeParam_ReplaceNodeData(const IS8* nodeUri,    //in
        IS8* data,       //in
        IS32 maxSize     //in
        ) {
    DM_TREE_HANDLE tree_handle = NULL;
    DMTREE_ITEM *item_ptr = NULL;
    IS32 result = 0;
    IBOOL isSuccess = FALSE;

    // if ((NULL == nodeUri) || (NULL == data) || (0 == maxSize))
    if (NULL == nodeUri) {
        dmTree_setErrorCode(DMTREE_ERR_INVALID_PARAM);
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeParam_ReplaceNodeData: Invalid param! \r\n");
        return FALSE;
    }

    dm_debug_trace(DM_DEBUG_TREE_MASK,
            "dmTreeParam_ReplaceNodeData: nodeUri = %s \r\n", nodeUri);
    if (NULL == data)
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeParam_ReplaceNodeData: data = NULL \r\n");
    else
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeParam_ReplaceNodeData: data = %s \r\n", data);
    dm_debug_trace(DM_DEBUG_TREE_MASK,
            "dmTreeParam_ReplaceNodeData: maxSize = %d \r\n", maxSize);

    item_ptr = dmTree_getItemEx(tree_handle, nodeUri);
    if (NULL != item_ptr) {
        result = item_ptr->replaceFunc(tree_handle, 0, data, maxSize);
        if (DM_OK == result) {
            dmTree_setErrorCode(DMTREE_ERR_OK);
            isSuccess = TRUE;
        } else {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "dmTreeParam_ReplaceNodeData: item_ptr->replaceFunc return error! \r\n");
        }
    } else {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeParam_ReplaceNodeData: Get item fail! \r\n");
        isSuccess = FALSE;
    }

    dm_debug_trace(DM_DEBUG_TREE_MASK,
            "dmTreeParam_ReplaceNodeData: Success! \r\n");

    return isSuccess;
}

/*****************************************************************************/
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
DMTREE_AUTH_TYPE dmTreeParam_GetAuthType() {
    dm_debug_trace(DM_DEBUG_TREE_MASK,
            "dmTreeParam_GetAuthType: g_DmAccInfo.accAuthPref = %d \r\n",
            g_DmAccInfo.accAuthPref);
    return g_DmAccInfo.accAuthPref;
}

