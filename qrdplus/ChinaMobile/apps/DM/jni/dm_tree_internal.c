#include "dm_tree_internal.h"
#include "dm_pl_debug.h"
#include "dm_pl_fs.h"

DMTREE_ITEM g_dmTreeTmpItem;
DMTREE_ACC_INFO g_dmTreeTmpDmAcc;
IBOOL g_dmTreeObjSave = FALSE;

void dmTreeFreeAclServerList(DMTREE_ACLSVRLIST *lpAclList) {
    if (lpAclList->next != NULL )
        dmTreeFreeAclServerList(lpAclList->next);
    free(lpAclList);
}

static IU8* dmTreeCopyString(IU8* buf, const IS8* str) {
    *buf = (IU8) (strlen(str) + 1);
    strcpy((IS8*) (buf + 1), str);
    buf += (*buf) + 1;
    return buf;
}

static IU8* dmTreeGetString(IU8* buf, IS8* str) {
    IS32 len = *buf++;
    strcpy(str, (IS8*) buf);
    buf += len;
    return buf;
}

DMTREE_ACLSVRLIST* dmTreeCopyAclServerList(DMTREE_ACLSVRLIST *lpAclList) {
    DMTREE_ACLSVRLIST* retAcl;
    DMTREE_ACLSVRLIST* newAcl;
    DMTREE_ACLSVRLIST* curAcl;
    DMTREE_ACLSVRLIST* addAcl;

    retAcl = NULL;
    curAcl = NULL;
    addAcl = lpAclList;
    while (addAcl != NULL ) {
        /*
         *  Malloc new Acl item
         */
        newAcl = malloc(sizeof(DMTREE_ACLSVRLIST));
        if (newAcl == NULL ) {
            if (retAcl != NULL )
                dmTreeFreeAclServerList(retAcl);
            dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
            return NULL ;
        }

        /*
         *   copy acl value
         */
        strcpy(newAcl->serverID, addAcl->serverID);
        newAcl->accType = addAcl->accType;
        newAcl->next = NULL;

        /*
         *   insert list
         */
        if (curAcl == NULL ) {
            retAcl = newAcl;
            curAcl = newAcl;
        } else {
            curAcl->next = newAcl;
            curAcl = newAcl;
        }

        addAcl = addAcl->next;
    }

    return retAcl;
}

IBOOL dmTreeCheckDataFormat(const IS8* nodeFormat, const IS8* nodeData,
        IS32 dataSize) {
    IS8 i;

    if (nodeData == NULL || dataSize == 0)
        return TRUE;
    if (dataSize < 0)
        dataSize = strlen(nodeData);

    if (0 == strcmp("int", nodeFormat)) {
        // int
        for (i = 0; i < dataSize; i++) {
            if (!isdigit(nodeData[i]))
                return FALSE;
        }
    } else if (0 == strcmp("bool", nodeFormat)) {
        // bool
        if (strcmp("true", nodeData) != 0 && strcmp("false", nodeData) != 0) {
            return FALSE;
        }
    }

    return TRUE;
}

void dmTreeDeleteItemData(DMTREE_INTERNAL_ITEM* lpItem) {
    if (lpItem->data->nodeData != NULL ) {
        if (lpItem->data->dataType == DMTREE_DATATYPE_DATA) {
            free(lpItem->data->nodeData);
        } else if (lpItem->data->dataType == DMTREE_DATATYPE_FILE) {
            devFS_removeFile(lpItem->data->nodeData);
            free(lpItem->data->nodeData);
        } else {
            free(lpItem->data->nodeData);
        }

        lpItem->data->nodeData = NULL;
        lpItem->data->dataSize = 0;
        lpItem->data->dataType = 0;
    }
}

const IS8* AUTH_MD5_STR = "syncml:auth-md5";
const IS8* AUTH_BASIC_STR = "syncml:auth-basic";
const IS8* AUTH_HMAC_STR = "syncml:auth-MAC";
const IS8* AUTH_NONE_STR = "syncml:auth-none";

/**
 *  @brief  Auth type change.
 *
 *  @param  none.
 *
 *  @return none.
 *
 *  @note   none.
 */
const IS8* dmTree_AuthType_etos(DMTREE_AUTH_TYPE eAuthType) {
    const IS8* res;
    switch (eAuthType) {
    case DMTREE_AUTHTYPE_BASIC:
        res = AUTH_BASIC_STR;
        break;
    case DMTREE_AUTHTYPE_MD5:
        res = AUTH_MD5_STR;
        break;
    case DMTREE_AUTHTYPE_HMAC:
        res = AUTH_HMAC_STR;
        break;
    default:
        res = AUTH_NONE_STR;
    }
    return res;
}

DMTREE_AUTH_TYPE dmTree_AuthType_stoe(const IS8* sAuthType) {
    DMTREE_AUTH_TYPE res;
    if (strcmp(sAuthType, AUTH_BASIC_STR) == 0)
        res = DMTREE_AUTHTYPE_BASIC;
    else if (strcmp(sAuthType, AUTH_MD5_STR) == 0)
        res = DMTREE_AUTHTYPE_MD5;
    else if (strcmp(sAuthType, AUTH_HMAC_STR) == 0)
        res = DMTREE_AUTHTYPE_HMAC;
    else
        res = DMTREE_AUTHTYPE_NONE;
    return res;
}

/**
 *  @brief  This function checks a URI string for validity.
 *
 *  @param  nodeUri  [I]URI to be checked.
 *
 *  @return DMTREE_ERR_OK or Error code;
 *
 *  @note   none.
 */
IU32 dmTreeInternal_checkValidUri(const IS8 *nodeUri) {
    const IS8* uri = nodeUri;
    IS32 numDepth = 1;
    IS32 lenName;

    /*
     *  URI Length <= 127
     */
    if (strlen(nodeUri) >= DM_TREE_MAX_URI_SIZE) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE Check URI failed! URI size too long: %s \r\n", uri);
        dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
        return DMTREE_ERR_URI_TOO_LONG;
    }

    /*
     *  no beginning '/' or '\'
     */
    if (*nodeUri == '/' || *nodeUri == '\\') {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE Check URI failed! Invalid URI: %s \r\n", uri);
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return DMTREE_ERR_CMD_FAILED;
    }

    /*
     *  depth and NodeName length
     */
    while (*nodeUri != '\0') {
        /*
         **  Search name
         */
        lenName = 0;
        while (*nodeUri != '\0' && *nodeUri != '/' && *nodeUri != '\\') {
            lenName++;
            nodeUri++;
        }

        /*
         **  Check the nodeName length.
         */
        if (lenName == 0 || lenName >= DM_TREE_MAX_NAME_STR) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE Check URI failed! NodeName too long, URI: %s \r\n",
                    uri);
            dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
            return DMTREE_ERR_CMD_FAILED;
        }

        /*
         **  level of Depth <= 7
         */
        numDepth++;
        if (numDepth > DM_TREE_URI_MAX_DEPTH) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE Check URI failed! Too max Depth: %s \r\n", uri);
            dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
            return DMTREE_ERR_CMD_FAILED;
        }

        /*
         **  Bad URI "./A/b/"
         */
        if (nodeUri[0] != '\0' && nodeUri[1] == '\0') {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE Check URI failed! Invalid URI: %s \r\n", uri);
            dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
            return DMTREE_ERR_CMD_FAILED;
        }
        nodeUri++;
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return DMTREE_ERR_OK;
}

/**
 *  @brief   Inserts a new child item in the tree handle.
 *
 *  @param   parentHndl  [I]Handle to the parent item.
 *  @param   newItem     [I]Address of a DMTREE_ITEM structure that specifies
 *                       the attributes of the new tree item.
 *
 *  @return  If successful, the return value is the handle to the new item. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    This function only add a new child-node under parent-node.
 *
 */
DMTREE_INTERNAL_ITEM* dmTreeInternal_insertItem(
        DMTREE_INTERNAL_ITEM* parentHndl, const DMTREE_ITEM* newItem) {
    DMTREE_INTERNAL_ITEM* lpNewItem;
    IS8* tmpUri;
    IS32 size;

    if (newItem->staticData) {
        size = sizeof(DMTREE_INTERNAL_ITEM);
        /*
         *  Malloc one new TreeItem
         */
        if ((lpNewItem = malloc(size)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE InsertItem failed! no memory! %s \r\n",
                    newItem->nodeName);
            dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
            return NULL ;
        }
        memset(lpNewItem, 0, size);
        lpNewItem->data = (DMTREE_ITEM*) newItem;
    } else {
        size = sizeof(DMTREE_INTERNAL_ITEM) + sizeof(DMTREE_ITEM);
        /*
         *  Malloc one new TreeItem
         */
        if ((lpNewItem = malloc(size)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE InsertItem failed! no memory! %s \r\n",
                    newItem->nodeName);
            dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
            return NULL ;
        }
        memset(lpNewItem, 0, size);
        lpNewItem->data = (DMTREE_ITEM*) ((IU32) lpNewItem
                + sizeof(DMTREE_INTERNAL_ITEM));

        /*
         *  Set new item data value
         */
        if (newItem->nodeData != NULL ) {
            lpNewItem->data->dataType = newItem->dataType;
            lpNewItem->data->dataSize = newItem->dataSize;
            if (lpNewItem->data->dataSize < 0)
                lpNewItem->data->dataSize = strlen(newItem->nodeData);
            lpNewItem->data->nodeData = malloc(lpNewItem->data->dataSize + 1);
            if (lpNewItem->data->nodeData == NULL ) {
                dm_debug_trace(DM_DEBUG_TREE_MASK,
                        "DMTREE InsertItem failed! no memory! %s \r\n",
                        newItem->nodeName);
                free(lpNewItem);
                dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
                return NULL ;
            }
            strncpy(lpNewItem->data->nodeData, newItem->nodeData,
                    lpNewItem->data->dataSize + 1);
            lpNewItem->data->nodeData[lpNewItem->data->dataSize] = '\0';
        }

        /*
         *  Set server Acl list
         */
        if (newItem->aclSvrList != NULL ) {
            DMTREE_ACLSVRLIST* newAcl;
            DMTREE_ACLSVRLIST* curAcl;
            DMTREE_ACLSVRLIST* addAcl;

            curAcl = NULL;
            addAcl = newItem->aclSvrList;
            while (addAcl != NULL ) {
                /*
                 *  Malloc new Acl item
                 */
                newAcl = malloc(sizeof(DMTREE_ACLSVRLIST));
                if (newAcl == NULL ) {
                    dm_debug_trace(DM_DEBUG_TREE_MASK,
                            "DMTREE InsertItem failed! no memory! %s \r\n",
                            newItem->nodeName);
                    free(lpNewItem->data->nodeData);
                    free(lpNewItem);
                    dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
                    return NULL ;
                }

                /*
                 *  copy acl value
                 */
                strcpy(newAcl->serverID, addAcl->serverID);
                newAcl->accType = addAcl->accType;
                newAcl->next = NULL;

                /*
                 *  insert list
                 */
                if (curAcl == NULL ) {
                    lpNewItem->data->aclSvrList = newAcl;
                    curAcl = newAcl;
                } else {
                    curAcl->next = newAcl;
                    curAcl = newAcl;
                }

                addAcl = addAcl->next;
            }
        }

        /*
         *  Set New Item uri
         */
        if (newItem->nodeUri[0] == '.') {
            strcpy(lpNewItem->data->nodeUri, newItem->nodeUri);
        } else if (newItem->nodeUri[0] != '\0') {
            int newLen;
            if (parentHndl != NULL) {
                strcpy(lpNewItem->data->nodeUri, parentHndl->data->nodeUri);
            }
            newLen = strlen(lpNewItem->data->nodeUri);
            if (newLen >=  (DMTREE_URI_MAXSIZE-1) ) {
                    dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
                    while (lpNewItem->data->aclSvrList) {
                         DMTREE_ACLSVRLIST* newAcl = lpNewItem->data->aclSvrList;
                         lpNewItem->data->aclSvrList = lpNewItem->data->aclSvrList->next;
                         free (newAcl);
                    }
                    free(lpNewItem->data->nodeData);
                    free(lpNewItem);
                    return NULL;
            }
            strcat(lpNewItem->data->nodeUri, "/");
            newLen+=1;
            if ((tmpUri = strrchr(newItem->nodeUri, '/')) != NULL ){
                int appendLen = strlen(tmpUri + 1);
                if(appendLen+newLen >= DMTREE_URI_MAXSIZE){
                      dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
                      while (lpNewItem->data->aclSvrList) {
                           DMTREE_ACLSVRLIST* newAcl = lpNewItem->data->aclSvrList;
                           lpNewItem->data->aclSvrList = lpNewItem->data->aclSvrList->next;
                           free (newAcl);
                      }
                      free(lpNewItem->data->nodeData);
                      free(lpNewItem);
                      return NULL ;
                }
                strcat(lpNewItem->data->nodeUri, tmpUri + 1);
            }
            else{
                int appendLen = strlen(newItem->nodeUri);
                if(appendLen+newLen >= DMTREE_URI_MAXSIZE) {
                    dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
                      while (lpNewItem->data->aclSvrList) {
                           DMTREE_ACLSVRLIST* newAcl = lpNewItem->data->aclSvrList;
                           lpNewItem->data->aclSvrList = lpNewItem->data->aclSvrList->next;
                           free (newAcl);
                      }
                    free(lpNewItem->data->nodeData);
                    free(lpNewItem);
                    return NULL;
                }
                strcat(lpNewItem->data->nodeUri, newItem->nodeUri);
           }
        } else {
            int newLen=0, appendLen=0;
            if (parentHndl != NULL ) {
                strcpy(lpNewItem->data->nodeUri, parentHndl->data->nodeUri);
                newLen = strlen(lpNewItem->data->nodeUri);
                if(newLen >= (DMTREE_URI_MAXSIZE-1) ) {
                    dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
                    while (lpNewItem->data->aclSvrList) {
                         DMTREE_ACLSVRLIST* newAcl = lpNewItem->data->aclSvrList;
                         lpNewItem->data->aclSvrList = lpNewItem->data->aclSvrList->next;
                         free (newAcl);
                    }
                    free(lpNewItem->data->nodeData);
                    free(lpNewItem);
                    return NULL;
                }
                strcat(lpNewItem->data->nodeUri, "/");
                newLen+=1;
            }
            appendLen = strlen(newItem->nodeName);
            if((appendLen+newLen) >= DMTREE_URI_MAXSIZE) {
                 dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
                 while (lpNewItem->data->aclSvrList) {
                       DMTREE_ACLSVRLIST* newAcl = lpNewItem->data->aclSvrList;
                       lpNewItem->data->aclSvrList = lpNewItem->data->aclSvrList->next;
                       free (newAcl);
                 }
                 free(lpNewItem->data->nodeData);
                 free(lpNewItem);
                 return NULL;
            }
            strcat(lpNewItem->data->nodeUri, newItem->nodeName);
        }

        /*
         *  Set TreeItem value
         */
        strcpy(lpNewItem->data->nodeName, newItem->nodeName);
        strcpy(lpNewItem->data->nodeType, newItem->nodeType);
        strcpy(lpNewItem->data->nodeFormat, newItem->nodeFormat);
        lpNewItem->data->occurrence = newItem->occurrence;
        lpNewItem->data->scope = newItem->scope;
        lpNewItem->data->allSrvAcl = newItem->allSrvAcl;

        lpNewItem->data->addFunc = newItem->addFunc;
        lpNewItem->data->deleteFunc = newItem->deleteFunc;
        lpNewItem->data->getFunc = newItem->getFunc;
        lpNewItem->data->replaceFunc = newItem->replaceFunc;
        lpNewItem->data->execFunc = newItem->execFunc;
    }

    /*
     *  Insert list
     */
    if (parentHndl != NULL ) {
        lpNewItem->parent = parentHndl;
        if (parentHndl->child != NULL ) {
            parentHndl = parentHndl->child;
            while (parentHndl->next != NULL )
                parentHndl = parentHndl->next;
            lpNewItem->prev = parentHndl;
            parentHndl->next = lpNewItem;
        } else {
            parentHndl->child = lpNewItem;
        }
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return lpNewItem;
}

/**
 *  @brief   This function sets some or all of a tree item's attributes.
 *
 *  @param   replaceHndl [I]Handle to the replace item.
 *  @param   replaceMask [I]Array of flags that indicate which of the other structure members
 *                       contain valid data. This member can be one or more of the following values. \n
 *                         DMTREE_REPLACE_NODENAME     \n
 *                         DMTREE_REPLACE_NODEDATA     \n
 *                         DMTREE_REPLACE_ACCESSTYPE
 *  @param   replaceItem [I]Address of a DMTREE_ITEM structure that specifies the attributes of
 *                       the new tree item.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *               DMTREE_ERR_NOT_ALLOWED
 *               DMTREE_ERR_NOMEMORY
 *               DMTREE_ERR_ALREADY_EXIST
 *
 *  @note    The treeHndl identifies the item, and the replaceMask parameters specifies which attributes to set.
 */
IS32 dmTreeInternal_replaceItem(DMTREE_INTERNAL_ITEM* replaceHndl,
        IU32 replaceMask, const DMTREE_ITEM* replaceItem) {
    if (replaceMask & DMTREE_REPLACE_NODENAME) {
        DMTREE_INTERNAL_ITEM* lpTmp;
        /*
         *  Check exist
         */
        lpTmp = replaceHndl->prev;
        while (lpTmp != NULL ) {
            if (strcmp(lpTmp->data->nodeName, replaceItem->nodeName) == 0) {
                dm_debug_trace(DM_DEBUG_TREE_MASK,
                        "DMTREE ReplaceItem failed! Node name have exist!\r\n");
                dmTree_setErrorCode(DMTREE_ERR_ALREADY_EXIST);
                return DMTREE_ERR_ALREADY_EXIST;
            }
            lpTmp = lpTmp->prev;
        }
        lpTmp = replaceHndl->next;
        while (lpTmp != NULL ) {
            if (strcmp(lpTmp->data->nodeName, replaceItem->nodeName) == 0) {
                dm_debug_trace(DM_DEBUG_TREE_MASK,
                        "DMTREE ReplaceItem failed! Node name have exist!\r\n");
                dmTree_setErrorCode(DMTREE_ERR_ALREADY_EXIST);
                return DMTREE_ERR_ALREADY_EXIST;
            }
            lpTmp = lpTmp->next;
        }
        /*
         *  Replace node name
         */
        strcpy(replaceHndl->data->nodeName, replaceItem->nodeName);
    }
    if (replaceMask & DMTREE_REPLACE_NODEDATA) {
        /*
         *  replace node data
         */
        if (strcmp(replaceHndl->data->nodeFormat, "node") == 0) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE ReplaceItem failed! Node data don't support replace!\r\n");
            dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
            return DMTREE_ERR_NOT_ALLOWED;
        }
        /*
         **  Data format check
         */
        if (!dmTreeCheckDataFormat(replaceHndl->data->nodeFormat,
                replaceItem->nodeData, replaceItem->dataSize)) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE ReplaceItem failed! Node format error!\r\n");
            dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
            return DMTREE_ERR_NOT_ALLOWED;
        }
        /*
         **  Remove old data
         */
        if (replaceHndl->data->nodeData != NULL ) {
            dmTreeDeleteItemData(replaceHndl);
        }
        /*
         **  Replace data
         */
        if (replaceItem->nodeData != NULL ) {
            if (replaceHndl->data->replaceFunc != NULL ) {
                replaceHndl->data->replaceFunc(replaceHndl, 0,
                        replaceItem->nodeData,
                        strlen(replaceItem->nodeData) + 1);
            } else {
                replaceHndl->data->dataType = replaceItem->dataType;
                replaceHndl->data->dataSize = replaceItem->dataSize;
                if (replaceHndl->data->dataSize < 0)
                    replaceHndl->data->dataSize = strlen(replaceItem->nodeData);
                if (replaceHndl->data->dataSize > 0) {
                    replaceHndl->data->nodeData = malloc(
                            replaceHndl->data->dataSize + 1);
                    if (replaceHndl->data->nodeData == NULL ) {
                        dm_debug_trace(DM_DEBUG_TREE_MASK,
                                "DMTREE ReplaceItem failed! no memory!\r\n");
                        dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
                        return DMTREE_ERR_NOMEMORY;
                    }
                    strncpy(replaceHndl->data->nodeData, replaceItem->nodeData,
                            replaceHndl->data->dataSize + 1);
                    replaceHndl->data->nodeData[replaceHndl->data->dataSize] =
                            '\0';
                }
            }
        }
    }
    if (replaceMask & DMTREE_REPLACE_ACLLIST) {
        /*
         *  Replace acl server list
         */
        DMTREE_ACLSVRLIST* newAcl;

        if ((newAcl = dmTreeCopyAclServerList(replaceItem->aclSvrList))
                == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE ReplaceItem failed! Copy ACL List failed! no memory!\r\n");
            return DMTREE_ERR_NOMEMORY;
        }

        if (replaceHndl->data->aclSvrList != NULL ) {
            dmTreeFreeAclServerList(replaceHndl->data->aclSvrList);
            replaceHndl->data->aclSvrList = NULL;
        }
        replaceHndl->data->aclSvrList = newAcl;
        // all server acl
        replaceHndl->data->allSrvAcl = replaceItem->allSrvAcl;
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return DMTREE_ERR_OK;
}

/**
 *  @brief   Replace the node acl list.
 *
 *  @param   lpTreeHndl [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   aclBuf     [I]The pointer to ACL Data.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *               DMTREE_ERR_CMD_FAILED
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note   none.
 */
IS32 dmTreeInternal_replaceAcl(DMTREE_INTERNAL_ITEM* lpTreeHndl,
        const IS8* aclBuf) {
    const IS8 *dataBeginPtr;
    const IS8 *dataEndPtr;

    DMTREE_ACCESS_TYPE accType;
    DMTREE_ACCESS_TYPE allSrvAcl;
    DMTREE_ACLSVRLIST *aclSvrList;
    IS8 serverId[DMTREE_ID_MAXSIZE];

    /* Initialize the variables */
    dataBeginPtr = dataEndPtr = aclBuf;
    allSrvAcl = 0;
    aclSvrList = NULL;

    /**
     Go through the ACL syntax and build the server list as you parse the
     ACL string
     */
    while (*dataEndPtr != '\0') {
        /* TO DO : Change the comparison with strncmp */
        /* Parse the command: Get command name and advance the dataBeginPtr. */
        if (strncmp(dataBeginPtr, "Add=", 4) == 0) {
            dataBeginPtr += 4;
            accType = DMTREE_ACCESS_ADD;
        } else if (strncmp(dataBeginPtr, "Delete=", 7) == 0) {
            dataBeginPtr += 7;
            accType = DMTREE_ACCESS_DELETE;
        } else if (strncmp(dataBeginPtr, "Get=", 4) == 0) {
            dataBeginPtr += 4;
            accType = DMTREE_ACCESS_GET;
        } else if (strncmp(dataBeginPtr, "Replace=", 8) == 0) {
            dataBeginPtr += 8;
            accType = DMTREE_ACCESS_REPLACE;
        } else if (strncmp(dataBeginPtr, "Exec=", 5) == 0) {
            dataBeginPtr += 5;
            accType = DMTREE_ACCESS_EXEC;
        } else if (strncmp(dataBeginPtr, "Copy=", 5) == 0) {
            dataBeginPtr += 5;
            accType = DMTREE_ACCESS_COPY;
        } else {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE Replace Acl failed! Invalid data!\r\n");
            dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
            return DMTREE_ERR_CMD_FAILED;
        }

        /**
         Loop to extract the serverIds from the ACL string (just in case
         there are more than one serverId associated to the command).
         The delimiter for multiple serverId is '+'.
         The delimiter for the end of the command is '&'.
         */
        do {
            /**
             Find the delimiter of the serverId list: '+' for next server ID
             and '&' for end of the serverId list
             */
            /* TO DO : Put syntax error check, including ' ' and '=' */
            dataEndPtr = dataBeginPtr;
            while (*dataEndPtr != '+' && *dataEndPtr != '&'
                    && *dataEndPtr != '\0') {
                dataEndPtr++;
            }

            /* Copy the serverId from the ACL string */
            /* TO DO : Check the size of the serverId before strncpy or change this to malloc */
            strncpy(serverId, dataBeginPtr, (IU32) (dataEndPtr - dataBeginPtr));
            serverId[(IU32) (dataEndPtr - dataBeginPtr)] = '\0';

            /**
             Check to see if the serverId is "*". If so, add the acl into the
             allServerAcl
             */
            if (strcmp(serverId, "*") == 0) {
                allSrvAcl |= accType;
            } else {
                DMTREE_ACLSVRLIST* lpTmpAclList;
                /**
                 Search the list of the ACL serverID, and make a new one if this server
                 does not already exist in the list.
                 */
                lpTmpAclList = aclSvrList;
                while (lpTmpAclList != NULL ) {
                    if (strcmp(lpTmpAclList->serverID, serverId) == 0)
                        break;
                    lpTmpAclList = lpTmpAclList->next;
                }

                if (lpTmpAclList != NULL ) {
                    /* ServerId found. Add the ACL into the serverId's ACL. */
                    lpTmpAclList->accType |= accType;
                } else {
                    /* ServerId not found. Create a server and append it to the list. */
                    lpTmpAclList = (DMTREE_ACLSVRLIST*) malloc(
                            sizeof(DMTREE_ACLSVRLIST));
                    if (lpTmpAclList == NULL ) {
                        dm_debug_trace(DM_DEBUG_TREE_MASK,
                                "DMTREE Replace Acl failed! no memory!\r\n");
                        if (aclSvrList != NULL )
                            dmTreeFreeAclServerList(aclSvrList);
                        dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
                        return DMTREE_ERR_NOMEMORY;
                    }

                    /* Copy the serverId and acl */
                    strcpy(lpTmpAclList->serverID, serverId);
                    lpTmpAclList->accType = accType;
                    lpTmpAclList->next = NULL;

                    if (aclSvrList == NULL ) {
                        /* First item in the list */
                        aclSvrList = lpTmpAclList;
                    } else {
                        /* Append it to the list */
                        lpTmpAclList->next = aclSvrList;
                        aclSvrList = lpTmpAclList;
                    }
                }
            } /* End else if not all server */

            /* Update the dataPtr to the next serverId or command */
            if (!strncmp(dataEndPtr, "&amp;", 5)) //Jianghong Song modify 20061226 for bug 9200
                    {
                dataBeginPtr = dataEndPtr + 5;
            } else {
                dataBeginPtr = dataEndPtr + 1;
            }

        } while (*dataEndPtr == '+');

    } /* End while (!done) */

    if (lpTreeHndl->data->aclSvrList != NULL )
        dmTreeFreeAclServerList(lpTreeHndl->data->aclSvrList);
    lpTreeHndl->data->aclSvrList = aclSvrList;
    lpTreeHndl->data->allSrvAcl = allSrvAcl;

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return DMTREE_ERR_OK;
}

/**
 *  @brief   Removes an item and all its children from the DM tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTreeInternalCB_deleteItem(DM_TREE_HANDLE treeHndl,
        DMTREE_ITEM* treeItem, IS32 param) {
    DMTREE_INTERNAL_ITEM* lpItem = (DMTREE_INTERNAL_ITEM*) treeHndl;

    /*
     *  delete item
     */
    if (!lpItem->data->staticData) {
        if (lpItem->data->nodeData != NULL ) {
            dmTreeDeleteItemData(lpItem);
        }
        if (lpItem->data->aclSvrList != NULL ) {
            dmTreeFreeAclServerList(lpItem->data->aclSvrList);
        }
    }
    free(lpItem);

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return TRUE;
}

/**
 *  @brief   Free the DM Tree resource;
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTreeInternalCB_freeItem(DM_TREE_HANDLE treeHndl, DMTREE_ITEM* treeItem,
        IS32 param) {
    DMTREE_INTERNAL_ITEM* lpItem = (DMTREE_INTERNAL_ITEM*) treeHndl;

    /*
     *  delete item
     */
    if (!lpItem->data->staticData) {
        if (lpItem->data->nodeData != NULL ) {
            free(lpItem->data->nodeData);
        }
        if (lpItem->data->aclSvrList != NULL ) {
            dmTreeFreeAclServerList(lpItem->data->aclSvrList);
        }
    }
    free(lpItem);

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return TRUE;
}

/**
 *  @brief   This function reads the tree from tree files and builds the
 *           whole tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to read.
 *  @param   buffer    [I]Read the data from file.
 *  @param   size      [I]Buffer data size.
 *
 *  @return  If successful, the return value is the handle to the new tree. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
DMTREE_INTERNAL_ITEM* dmTreeInternal_readItem(DM_TREE_HANDLE treeHndl,
        IU8* buffer, IU32 size) {
    DMTREE_INTERNAL_ITEM* lpNewItem;
    IU8* buf = buffer;
    IU32 modMask;

    // modify mask
    //modMask = *((bf_u16*)buf);
    modMask = DM_MAKEWORD(buf[1], buf[0]);
    buf += 2;

    /*
     *  Get item info
     */
    memset(&g_dmTreeTmpItem, 0, sizeof(DMTREE_ITEM));

    // node name
    buf = dmTreeGetString(buf, g_dmTreeTmpItem.nodeName);
    // node Uri
    buf = dmTreeGetString(buf, g_dmTreeTmpItem.nodeUri);
    // node Type
    if ((modMask & DMTREE_REPLACE_NEWADD)) {
        buf = dmTreeGetString(buf, g_dmTreeTmpItem.nodeType);
    }
    // node format
    if ((modMask & DMTREE_REPLACE_NEWADD)) {
        buf = dmTreeGetString(buf, g_dmTreeTmpItem.nodeFormat);
    }
    // node Data
    if ((modMask & DMTREE_REPLACE_NODEDATA)
            || (modMask & DMTREE_REPLACE_NEWADD)) {
        g_dmTreeTmpItem.dataType = *buf++;
        if (*buf != 0) {
            g_dmTreeTmpItem.dataSize = -1;
            g_dmTreeTmpItem.nodeData = (IS8*) (buf + 1);
        }
        buf += (*buf) + 1;
    }
    // All Server access
    if ((modMask & DMTREE_REPLACE_NEWADD)) {
        g_dmTreeTmpItem.allSrvAcl = *buf++;
    }
    // acl server list
    if ((modMask & DMTREE_REPLACE_ACLLIST)
            || (modMask & DMTREE_REPLACE_NEWADD)) {
        if (*buf == sizeof(DMTREE_ACLSVRLIST)) {
            DMTREE_ACLSVRLIST* aclList = NULL;
            g_dmTreeTmpItem.aclSvrList = (DMTREE_ACLSVRLIST*) (buf + 1);
            while (*buf == sizeof(DMTREE_ACLSVRLIST)) {
                buf++;
                if (aclList != NULL )
                    aclList->next = (DMTREE_ACLSVRLIST*) buf;
                aclList = (DMTREE_ACLSVRLIST*) buf;
                aclList->next = NULL;

                buf += sizeof(DMTREE_ACLSVRLIST);
            }
        }
    }

    if (modMask & DMTREE_REPLACE_NEWADD) {
        /*
         *  Add New item
         */
        lpNewItem = dmTreeSml_insertItem(treeHndl, &g_dmTreeTmpItem, NULL );
    } else {
        /*
         *  Replace item
         */
        lpNewItem = dmTree_searchItem(treeHndl, g_dmTreeTmpItem.nodeUri);
        if (lpNewItem != NULL ) {
            dmTreeSml_replaceItem((DM_TREE_HANDLE) lpNewItem, modMask,
                    &g_dmTreeTmpItem, NULL );
        }
    }

    return lpNewItem;
}

/**
 *  @brief   This function reads the tree from tree files and builds the
 *           whole tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to read.
 *  @param   buffer    [I]Read the data from file.
 *  @param   size      [I]Buffer data size.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IS32 dmTreeInternal_readDmAccItem(DM_TREE_HANDLE treeHndl, IU8* buffer,
        IU32 size) {
    IS8* buf = (IS8*) buffer;

    memset(&g_dmTreeTmpDmAcc, 0, sizeof(DMTREE_ACC_INFO));

    // node name
    if (strlen(buf) < DMTREE_NAME_MAXSIZE) {
        strcpy(g_dmTreeTmpDmAcc.accNodeName, buf);
    }
    buf += strlen(buf) + 1;

    // AppID
    if (strlen(buf) < DMTREE_ID_MAXSIZE) {
        strcpy(g_dmTreeTmpDmAcc.accAppID, buf);
    }
    buf += strlen(buf) + 1;
    // ServerID
    if (strlen(buf) < DMTREE_ID_MAXSIZE) {
        strcpy(g_dmTreeTmpDmAcc.accServerId, buf);
    }
    buf += strlen(buf) + 1;
    // Name
    if (strlen(buf) < DMTREE_NAME_MAXSIZE) {
        strcpy(g_dmTreeTmpDmAcc.accName, buf);
    }
    buf += strlen(buf) + 1;
    // PrefConRef
    if (strlen(buf) < DMTREE_ID_MAXSIZE) {
        strcpy(g_dmTreeTmpDmAcc.accPrefConRef, buf);
    }
    buf += strlen(buf) + 1;
    // ConRef
    if (strlen(buf) < DMTREE_ID_MAXSIZE) {
        strcpy(g_dmTreeTmpDmAcc.accConRef, buf);
    }
    buf += strlen(buf) + 1;

    // Addr
    if (strlen(buf) < DMTREE_URL_MAXSIZE) {
        strcpy(g_dmTreeTmpDmAcc.accAddr, buf);
    }
    buf += strlen(buf) + 1;
    // AddrType
    if (strlen(buf) < 16) {
        strcpy(g_dmTreeTmpDmAcc.accAddrType, buf);
    }
    buf += strlen(buf) + 1;
    // Prot Number
    //g_dmTreeTmpDmAcc.accPortNbr = *((bf_u16*)buf);
    g_dmTreeTmpDmAcc.accPortNbr = DM_MAKEWORD(buf[1], buf[0]);
    buf += 2;

    // Auth type
    g_dmTreeTmpDmAcc.accAuthPref = (DMTREE_AUTH_TYPE) (*buf++);

    // Server AuthLevel
    strcpy(g_dmTreeTmpDmAcc.accServerAuthLevel, buf);
    buf += strlen(buf) + 1;
    // Server AuthType
    g_dmTreeTmpDmAcc.accServerAuthType = (DMTREE_AUTH_TYPE) (*buf++);
    // Server AuthName
    strcpy(g_dmTreeTmpDmAcc.accServerAuthName, buf);
    buf += strlen(buf) + 1;
    // Server Psw
    strcpy(g_dmTreeTmpDmAcc.accServerPw, buf);
    buf += strlen(buf) + 1;
    // Server Nonce
    strcpy((IS8*) g_dmTreeTmpDmAcc.accServerNonce, buf);
    buf += strlen(buf) + 1;
    // Prev Server Nonce
    strcpy((IS8*) g_dmTreeTmpDmAcc.accPrevServerNonce, buf);
    buf += strlen(buf) + 1;

    // Client AuthLevel
    strcpy(g_dmTreeTmpDmAcc.accClientAuthLevel, buf);
    buf += strlen(buf) + 1;
    // Client AuthType
    g_dmTreeTmpDmAcc.accClientAuthType = (DMTREE_AUTH_TYPE) (*buf++);
    // User Name
    strcpy(g_dmTreeTmpDmAcc.accUserName, buf);
    buf += strlen(buf) + 1;
    // User Psw
    strcpy(g_dmTreeTmpDmAcc.accUserPw, buf);
    buf += strlen(buf) + 1;
    // Client Nonce
    strcpy((IS8*) g_dmTreeTmpDmAcc.accClientNonce, buf);
    buf += strlen(buf) + 1;

    return dmTreeDmAcc_addReplaceAccount(&g_dmTreeTmpDmAcc);
}

/**
 *  @brief   This function saves the management tree item to the files.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param. The file handle.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTreeInternalCB_saveItem(DM_TREE_HANDLE treeHndl, DMTREE_ITEM* treeItem,
        IS32 param) {
    DMTREE_INTERNAL_ITEM* lpTreeHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;
    if (lpTreeHndl->modMask != 0) {
        IU8* buffer = (IU8*) (&g_dmTreeTmpDmAcc);
        IU8* buf = buffer;
        IS32 size;

        // modify mask
        //*((bf_u16*)buf) = (bf_u16)lpTreeHndl->modMask; buf += 2;
        *buf++ = (IU8) lpTreeHndl->modMask;
        *buf++ = (IU8) (lpTreeHndl->modMask >> 8);
        // node name
        buf = dmTreeCopyString(buf, treeItem->nodeName);
        // node Uri
        buf = dmTreeCopyString(buf, treeItem->nodeUri);
        // node Type
        if ((lpTreeHndl->modMask & DMTREE_REPLACE_NEWADD)) {
            buf = dmTreeCopyString(buf, treeItem->nodeType);
        }
        // node format
        if ((lpTreeHndl->modMask & DMTREE_REPLACE_NEWADD)) {
            buf = dmTreeCopyString(buf, treeItem->nodeFormat);
        }
        // node Data
        if ((lpTreeHndl->modMask & DMTREE_REPLACE_NODEDATA)
                || (lpTreeHndl->modMask & DMTREE_REPLACE_NEWADD)) {
            *buf++ = (IU8) treeItem->dataType;
            if (treeItem->nodeData != NULL )
                buf = dmTreeCopyString(buf, treeItem->nodeData);
            else
                *buf++ = 0;
        }
        // All Server access
        if ((lpTreeHndl->modMask & DMTREE_REPLACE_NEWADD)) {
            *buf++ = (IU8) treeItem->allSrvAcl;
        }
        // acl server list
        if ((lpTreeHndl->modMask & DMTREE_REPLACE_ACLLIST)
                || (lpTreeHndl->modMask & DMTREE_REPLACE_NEWADD)) {
            DMTREE_ACLSVRLIST* aclList = treeItem->aclSvrList;
            while (aclList != NULL ) {
                *buf++ = sizeof(DMTREE_ACLSVRLIST);
                memcpy(buf, aclList, sizeof(DMTREE_ACLSVRLIST));

                aclList = aclList->next;
            }
            *buf++ = 0;
        }

        // Save to file
        size = (IS32) (buf - buffer);
        devFS_writeFile((DM_FILE_HANDLE) param, &size, sizeof(IU32));
        devFS_writeFile((DM_FILE_HANDLE) param, buffer, size);
    }

    return TRUE;
}

/**
 *  @brief   This function print the management tree item.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to save.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param. The file handle.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTreeInternalCB_printItem(DM_TREE_HANDLE treeHndl, DMTREE_ITEM* treeItem,
        IS32 param) {
    DMTREE_ACLSVRLIST* aclList;
    /*
     *  Node name and nodeUri
     */
    dm_debug_trace(DM_DEBUG_TREE_MASK, "<%s> = <%s> ", treeItem->nodeName,
            treeItem->nodeUri);
    /*
     *  Node data
     */
    if (treeItem->nodeData != NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Data=<%s> ", treeItem->nodeData);
    }
    /*
     *  All server ACL
     */
    dm_debug_trace(DM_DEBUG_TREE_MASK, "AllSrvAcl=<0x%04X> ",
            treeItem->allSrvAcl);
    /*
     *  ACL Server list
     */
    aclList = treeItem->aclSvrList;
    while (aclList != NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "RunTimePropertyType->ServerID<%s> ACL<0x%04X> ",
                aclList->serverID, aclList->accType);
        aclList = aclList->next;
    }

    dm_debug_trace(DM_DEBUG_TREE_MASK, "\r\n");
    return TRUE;
}

/**
 *  @brief   This function saves the DMAcc info.
 *
 *  @param   fileHndl  [I]file handle to the item to save.
 *  @param   dmAccInfo [I]The DmAcc info.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTreeInternal_saveDmAccItem(DM_FILE_HANDLE fileHndl,
        DMTREE_ACC_INFO* dmAccInfo) {
    IS8* buffer = (IS8*) (&g_dmTreeTmpDmAcc);
    IS8* buf = buffer;
    IU32 size;

    // node name
    strcpy(buf, dmAccInfo->accNodeName);
    buf += strlen(buf) + 1;
    // AppID
    strcpy(buf, dmAccInfo->accAppID);
    buf += strlen(buf) + 1;
    // ServerID
    strcpy(buf, dmAccInfo->accServerId);
    buf += strlen(buf) + 1;
    // Name
    strcpy(buf, dmAccInfo->accName);
    buf += strlen(buf) + 1;
    // PrefConRef
    strcpy(buf, dmAccInfo->accPrefConRef);
    buf += strlen(buf) + 1;
    // ConRef
    strcpy(buf, dmAccInfo->accConRef);
    buf += strlen(buf) + 1;

    // Addr
    strcpy(buf, dmAccInfo->accAddr);
    buf += strlen(buf) + 1;
    // AddrType
    strcpy(buf, dmAccInfo->accAddrType);
    buf += strlen(buf) + 1;
    // Prot Number
    //*((bf_u16*)buf) = (bf_u16)dmAccInfo->accPortNbr; buf += 2;// alignment problem. the result is undefined in the arm device. Jianghong Song
    *buf++ = (IU8) dmAccInfo->accPortNbr;
    *buf++ = (IU8) (dmAccInfo->accPortNbr >> 8);

    // Auth type
    *buf++ = (IS8) dmAccInfo->accAuthPref;

    // Server AuthLevel
    strcpy(buf, dmAccInfo->accServerAuthLevel);
    buf += strlen(buf) + 1;
    // Server AuthType
    *buf++ = (IS8) dmAccInfo->accServerAuthType;
    // Server AuthName
    strcpy(buf, dmAccInfo->accServerAuthName);
    buf += strlen(buf) + 1;
    // Server Psw
    strcpy(buf, dmAccInfo->accServerPw);
    buf += strlen(buf) + 1;
    // Server Nonce
    strcpy(buf, (IS8*) dmAccInfo->accServerNonce);
    buf += strlen(buf) + 1;
    // Prev Server Nonce
    strcpy(buf, (IS8*) dmAccInfo->accPrevServerNonce);
    buf += strlen(buf) + 1;

    // Client AuthLevel
    strcpy(buf, dmAccInfo->accClientAuthLevel);
    buf += strlen(buf) + 1;
    // Client AuthType
    *buf++ = (IS8) dmAccInfo->accClientAuthType;
    // User Name
    strcpy(buf, dmAccInfo->accUserName);
    buf += strlen(buf) + 1;
    // User Psw
    strcpy(buf, dmAccInfo->accUserPw);
    buf += strlen(buf) + 1;
    // Client Nonce
    strcpy(buf, (IS8*) dmAccInfo->accClientNonce);
    buf += strlen(buf) + 1;

    // Save data to file
    size = (IU32) (buf - buffer);
    devFS_writeFile(fileHndl, &size, sizeof(IU32));
    devFS_writeFile(fileHndl, buffer, size);

    return TRUE;
}

/**
 *  @brief  Create DMAcc node.
 *
 *  @param   dmAccHndl [I]Handle to the DmAcc item.
 *  @param   nodeName  [I]The DM Account node name.
 *  @param   serverID  [I]The DM Account ServerID.
 *
 *  @return  If successful, the return value is the handle to the new DM Account node. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none.
 */
DMTREE_INTERNAL_ITEM* dmTreeInternal_createDMAcc(
        DMTREE_INTERNAL_ITEM* dmAccHndl, const DMTREE_ITEM* lpAccStdItem,
        const IS8* nodeName, const IS8* serverID) {
    const DMTREE_ITEM* lpStdItem;
    DMTREE_ACLSVRLIST stServerList;
    DMTREE_INTERNAL_ITEM* lpNodeItem;
    DMTREE_INTERNAL_ITEM* lpItem;

    /*
     *  Init value
     */
    lpStdItem = lpAccStdItem;
    strcpy(stServerList.serverID, serverID);
    stServerList.accType = DMTREE_ACCESS_ALL;
    stServerList.next = NULL;

    /*
     *  Create Node Name
     */
    memcpy(&g_dmTreeTmpItem, lpStdItem, sizeof(DMTREE_ITEM));
    strcpy(g_dmTreeTmpItem.nodeName, nodeName);
    strcpy(g_dmTreeTmpItem.nodeUri, nodeName);
    g_dmTreeTmpItem.aclSvrList = &stServerList;
    g_dmTreeTmpItem.allSrvAcl = DMTREE_ACCESS_NONE;
    stServerList.accType = lpStdItem->allSrvAcl;
    if ((lpNodeItem = dmTreeInternal_insertItem(dmAccHndl, &g_dmTreeTmpItem))
            == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "Create DMAcc failed! Insert Account Node name failed!\r\n");
        return NULL ;
    }

    lpStdItem++;
    /*
     *  Insert Other item
     */
    while (lpStdItem->nodeName[0] != '\0') {
        memcpy(&g_dmTreeTmpItem, lpStdItem, sizeof(DMTREE_ITEM));
        g_dmTreeTmpItem.aclSvrList = &stServerList;
        g_dmTreeTmpItem.allSrvAcl = DMTREE_ACCESS_NONE;
        stServerList.accType = lpStdItem->allSrvAcl;
        if ((lpItem = dmTree_insertItem(lpNodeItem, &g_dmTreeTmpItem)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "Create DMAcc failed! Insert item failed!\r\n");
            return NULL ;
        }

        lpStdItem++;
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return lpNodeItem;
}

/**
 *  @brief  Search DM Account Node.
 *
 *  @param  treeHndl  [I]Handle to the node item.
 *  @param  serverID  [I]Server Id to check ACL.
 *
 *  @return  If successful, the return value is the handle to the DM Account node. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note   none.
 */
DMTREE_INTERNAL_ITEM* dmTreeInternal_searchDMAcc(DMTREE_INTERNAL_ITEM* treeHndl,
        const IS8* serverID) {
    DMTREE_INTERNAL_ITEM* dmAccHndl;
    DMTREE_INTERNAL_ITEM* nodeHndl;
    DMTREE_INTERNAL_ITEM* idHndl;
    IS8* serverIDUri;

    /*
     *  Search DM Acc path
     */
    if ((dmAccHndl = dmTree_searchItem((DM_TREE_HANDLE) treeHndl,
            DMTREE_DMACC_PATH)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Search DMAcc path failed! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return NULL ;
    }

    /*
     *  Search serverID Node
     */
    dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerID, &serverIDUri, NULL );
    nodeHndl = dmAccHndl->child;
    while (nodeHndl != NULL ) {
        idHndl = dmTree_searchItem((DM_TREE_HANDLE) nodeHndl, serverIDUri);
        if (idHndl != NULL && idHndl->data->nodeData != NULL
                && strcmp(idHndl->data->nodeData, serverID) == 0)
            break;

        nodeHndl = nodeHndl->next;
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return nodeHndl;
}

/**
 *  @brief   Set DMAcc server list ServerID value.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param. serverID pointer.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTreeInternalCB_replaceDMAccServerList(DM_TREE_HANDLE treeHndl,
        DMTREE_ITEM* treeItem, IS32 param) {
    DMTREE_INTERNAL_ITEM* lpTreeHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;

    if (lpTreeHndl->data->aclSvrList != NULL )
        strcpy(lpTreeHndl->data->aclSvrList->serverID, (IS8*) param);

    return TRUE;
}
