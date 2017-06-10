#ifndef _DM_PL_TREE_H_
#define _DM_PL_TREE_H_

#include "vdm_pl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* DM_TREE_HANDLE;

#define SUPPORT_OMA_DM_1_2

#define DMTREE_ID_MAXSIZE               32
#define DMTREE_NAME_MAXSIZE             64
#define DMTREE_URI_MAXSIZE              128
#define DMTREE_URL_MAXSIZE              512
#define DMTREE_TYPE_MAXSIZE             64

/*
 *  Error code
 */
#define DMTREE_ERR_OK                   0
#define DMTREE_ERR_UNDEF                1 //
#define DMTREE_ERR_NOMEMORY             2
#define DMTREE_ERR_INVALID_PARAM        3 //
#define DMTREE_ERR_NOT_FOUND            404
#define DMTREE_ERR_NOT_ALLOWED          405
#define DMTREE_ERR_NOT_SUPPORTED        406
#define DMTREE_ERR_URI_TOO_LONG         414
#define DMTREE_ERR_ALREADY_EXIST        418
#define DMTREE_ERR_PERMISSION_DENIED    425
#define DMTREE_ERR_CMD_FAILED           500

/*
 *  DMTREE data type define
 */
#define DMTREE_DATATYPE_DATA            0x0000
#define DMTREE_DATATYPE_FILE            0x0001

/**
 DMTREE scope enum define
 */
typedef enum _DMTREE_SCOPE {
    DMTREE_SCOPE_DYNAMIC,       ///< dynamic node
    DMTREE_SCOPE_PERMANENT      ///< permanent node

} DMTREE_SCOPE;

/**
 DMTREE occurrence enum define
 */
typedef enum _DMTREE_OCCURRENCE {
    DMTREE_OCCUR_One,           ///< only one node is allowed
    DMTREE_OCCUR_ZeroOrOne,     ///< zero or one node is allowed
    DMTREE_OCCUR_ZeroOrMore,    ///< zero or more than one node is allowed
    DMTREE_OCCUR_OneOrMore,     ///< one or more than one node is allowed
    DMTREE_OCCUR_ZeroOrN,       ///< zero or N nodes is allowed
    DMTREE_OCCUR_OneOrN         ///< one or N nodes is allowed

} DMTREE_OCCURRENCE;

/*
 *  DMTREE replace type define
 */
#define DMTREE_REPLACE_NODENAME      0x0001
//#define DMTREE_REPLACE_NODETYPE      0x0002
//#define DMTREE_REPLACE_NODEFORMAT    0x0004
#define DMTREE_REPLACE_NODEDATA      0x0008
#define DMTREE_REPLACE_ACLLIST       0x0020

/*
 *  DMTREE access type macro define
 */
#define DMTREE_ACCESS_NONE           0x0000
#define DMTREE_ACCESS_ADD            0x0001
#define DMTREE_ACCESS_COPY           0x0002
#define DMTREE_ACCESS_DELETE         0x0004
#define DMTREE_ACCESS_EXEC           0x0008
#define DMTREE_ACCESS_GET            0x0010
#define DMTREE_ACCESS_REPLACE        0x0020
#define DMTREE_ACCESS_ALL            0x003F

typedef unsigned int DMTREE_ACCESS_TYPE;

/**
 DMTREE acl server list struct
 */
typedef struct _DMTREE_ACLSVRLIST {
    IS8 serverID[DMTREE_ID_MAXSIZE];        ///< server ID
    IU32 accType;                            ///< access type

    struct _DMTREE_ACLSVRLIST* next;            ///< the next item pointer

} DMTREE_ACLSVRLIST;

/*
 *  DMTREE Access function type for the command operation
 */
typedef IS32 (*DMTREE_ADDFUNC)(DM_TREE_HANDLE treeHndl, IS8* data, IS32 size);
typedef IS32 (*DMTREE_DELETEFUNC)(DM_TREE_HANDLE treeHndl);
typedef IS32 (*DMTREE_GETFUNC)(DM_TREE_HANDLE treeHndl, IS32 offset, IS8** data,
        IS32* maxSize);
typedef IS32 (*DMTREE_REPLACEFUNC)(DM_TREE_HANDLE treeHndl, IS32 offset,
        IS8* data, IS32 size);
typedef IS32 (*DMTREE_EXECFUNC)(DM_TREE_HANDLE treeHndl, const IS8* params,
        const IS8* correlator, IS32 callType);

/**
 DMTREE item struct define
 */
typedef struct _DMTREE_ITEM {
    IBOOL staticData; ///< specifies whether the node is static data or dynamic data
    IS8 nodeName[DMTREE_NAME_MAXSIZE];        ///< node name
    IS8 nodeUri[DMTREE_URI_MAXSIZE];      ///< node URI name
    IS8 nodeType[DMTREE_TYPE_MAXSIZE];        ///< node type
    IS8 nodeFormat[8];                        ///< node format type
    IS32 dataSize;                         ///< data value length
    IS32 dataType;                         ///< data type value
    IS8* nodeData;                         ///< node data value

    DMTREE_OCCURRENCE occurrence;     ///< specifies the allowed number of node
    DMTREE_SCOPE scope;                   ///< node type, dynamic or permanent
    DMTREE_ACLSVRLIST* aclSvrList;              ///< ACL server list
    DMTREE_ACCESS_TYPE allSrvAcl;               ///< all server ACL
//  DMTREE_ACLSVRLIST  access;          // node access type
    DMTREE_ADDFUNC addFunc;                 ///< add callback function
    DMTREE_DELETEFUNC deleteFunc;              ///< delete callback function
    DMTREE_GETFUNC getFunc;                 ///< get callback function
    DMTREE_REPLACEFUNC replaceFunc;             ///< replace callback function
    DMTREE_EXECFUNC execFunc;                ///< exec callback function

} DMTREE_ITEM;

/**
 DMTREE account address type
 */
typedef enum _DMTREE_AddrType {
    DMTREE_ADDRTYPE_HTTP = 1,   ///< HTTP
    DMTREE_ADDRTYPE_WSP,        ///< WSP
    DMTREE_ADDRTYPE_OBEX        ///< OBEX

} DMTREE_AddrType;

/**
 DMTREE account auth type define
 */
typedef enum _DMTREE_AUTH_TYPE {
    DMTREE_AUTHTYPE_NONE,       ///< no authentication
    DMTREE_AUTHTYPE_BASIC,      ///< BASIC authentication
    DMTREE_AUTHTYPE_MD5,        ///< MD5 authentication
    DMTREE_AUTHTYPE_HMAC        ///< HMAC authentication

} DMTREE_AUTH_TYPE;

/**
 DMTREE Account info define
 */
typedef struct _DMTREE_ACC_INFO {
    IS8 accNodeName[DMTREE_NAME_MAXSIZE];       ///< node name

    IS8 accAppID[DMTREE_ID_MAXSIZE];            ///< application ID
    IS8 accServerId[DMTREE_ID_MAXSIZE];         ///< server ID
    IS8 accName[DMTREE_NAME_MAXSIZE];           ///< server name
    IS8 accPrefConRef[DMTREE_ID_MAXSIZE];       ///< preferred connectivity
    IS8 accConRef[DMTREE_ID_MAXSIZE]; ///< the linkage to connectivity parameters

    IS8 accAddr[DMTREE_URL_MAXSIZE];            ///< Management Server address
    IS8 accAddrType[16];                    ///< Management Server address type.
    IS32 accPortNbr;                             ///< port number

    DMTREE_AUTH_TYPE accAuthPref;                    ///< authentication types

    IS8 accServerAuthLevel[DMTREE_ID_MAXSIZE]; ///< server authentication level
    DMTREE_AUTH_TYPE accServerAuthType;          ///< server authentication type
    IS8 accServerAuthName[DMTREE_NAME_MAXSIZE]; ///< server authentication name
    IS8 accServerPw[DMTREE_ID_MAXSIZE];         ///< server password
    IU8 accServerNonce[DMTREE_NAME_MAXSIZE];    ///< server nonce
    IU8 accPrevServerNonce[DMTREE_NAME_MAXSIZE]; ///< server nonce of the previous session

    IS8 accClientAuthLevel[DMTREE_ID_MAXSIZE];  ///< client authentication level
    DMTREE_AUTH_TYPE accClientAuthType;          ///< client authentication type
    IS8 accUserName[DMTREE_NAME_MAXSIZE];       ///< use name
    IS8 accUserPw[DMTREE_ID_MAXSIZE];           ///< user password
    IU8 accClientNonce[DMTREE_NAME_MAXSIZE];    ///< client nonce

} DMTREE_ACC_INFO;

/**
 the type ID of the node name
 */
typedef enum DMTREE_ACC_NODE_NAME_TAG {
    dmAccNodeName_None,                 ///< the type of node name is unknown
    dmAccNodeName_AppID,                ///< AppID
    dmAccNodeName_ServerID,             ///< ServerID
    dmAccNodeName_Name,                 ///< Name
    dmAccNodeName_PrefConRef,           ///< PrefConRef
    dmAccNodeName_ConRef,               ///< ConRef

    dmAccNodeName_Addr,                 ///< Addr
    dmAccNodeName_AddrType,             ///< AddrType
    dmAccNodeName_PortNbr,              ///< PortNbr

    dmAccNodeName_AuthPref,             ///< AuthPref

    dmAccNodeName_ServerAuthLevel,      ///< ServerAuthLevel
    dmAccNodeName_ServerAuthType,       ///< ServerAuthType
    dmAccNodeName_ServerAuthName,       ///< ServerAuthName
    dmAccNodeName_ServerPw,             ///< ServerPw
    dmAccNodeName_ServerNonce,          ///< ServerNonce
    dmAccNodeName_PrevServerNonce,      ///< PrevServerNonce

    dmAccNodeName_ClientAuthLevel,      ///< ClientAuthLevel
    dmAccNodeName_ClientAuthType,       ///< ClientAuthType
    dmAccNodeName_UserName,             ///< UserName
    dmAccNodeName_UserPw,               ///< UserPw
    dmAccNodeName_ClientNonce,          ///< ClientNonce

    dmAccNodeName_End                   ///<

} DMTREE_ACC_NODE_NAME_E;

#ifdef SUPPORT_OMA_DM_1_2

#define DMTREE_DMACC_PATH          "./DMAcc"

#else

#define DMTREE_DMACC_PATH          "./SyncML/DMAcc"
#define DMTREE_DMCON_PATH          "./SyncML/Con"

#endif

/*
 *  OMA Standard table item
 */
extern const DMTREE_ITEM g_stInitStdDataTable[];

/*
 *  OMA Account standard table item
 */
extern const DMTREE_ITEM g_stAccStdDataTable[];

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
const DMTREE_ACC_INFO* dmTreeParam_getDmAccInfo(void);

/**
 *  @brief  This function Get the details of a DM account in the DM tree.
 *
 *  @param  eNodeName  [I]Account infor node name enum.
 *  @param  szNodeUri  [O]the Node URI path; this param can is NULL;
 *  @param  szNodeName [O]node name; this param can is NULL;
 *
 *  @return DM_OK    - Successful.   \n
 *          DM_ERROR - Failed!
 *
 *  @note   none.
 */
IS32 dmTreeParam_getDmAccNodeName(DMTREE_ACC_NODE_NAME_E eNodeName,
        IS8** szNodeUri, IS8** szNodeName);

IBOOL dmTreeParam_ReplaceNodeData(const IS8* nodeUri,    //in
        IS8* data,       //in
        IS32 maxSize     //in
        );

IBOOL dmTreeParam_getNodeData(const IS8* nodeUri,    //in
        IS8** data,       //out
        IS32* maxSize     //out
        );
#ifdef __cplusplus
}
#endif

#endif // _DMTREEWRAPPER_H_
