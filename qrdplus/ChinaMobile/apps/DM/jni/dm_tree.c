#include "dm_tree.h"
#include "dm_tree_internal.h"
#include "dm_pl_debug.h"
#include "dm_error.h"
//#include <utils/Log.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"DMLIB ==>",__VA_ARGS__)

//#define LOGE
static IU32 g_nErrorCode = DMTREE_ERR_OK;
static DM_TREE_HANDLE g_hTreeRootHndl = NULL;

extern DMTREE_ITEM g_dmTreeTmpItem;
extern DMTREE_ACC_INFO g_dmTreeTmpDmAcc;
extern IBOOL g_dmTreeObjSave;

/**
 *  @brief   Create a tree from standard DDFTable.
 *
 *  @param   none.
 *
 *  @return  If successful, the return value is the handle to the new item. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    The first item max is root item. "./"
 */
DM_TREE_HANDLE dmTree_create(void) {
    const DMTREE_ITEM* lpStdItem;
    DMTREE_INTERNAL_ITEM* lpRootItem;

    /*
     *  Create Root item
     */
    lpStdItem = g_stInitStdDataTable;
    if ((lpRootItem = dmTreeInternal_insertItem(NULL, lpStdItem)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "Create tree failed! no memory! \r\n");
        return NULL ;
    }

    /*
     *  Insert other item
     */
    lpStdItem++;
    while (lpStdItem->nodeName[0] != '\0') {
        if (dmTree_insertItem(lpRootItem, lpStdItem) == NULL ) {
            dmTree_deleteItem((DM_TREE_HANDLE) lpRootItem);
            return NULL ;
        }

        lpStdItem++;
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return (DM_TREE_HANDLE) lpRootItem;
}

/**
 *  @brief   Search the item of the nodeUri, from the DM Tree.
 *
 *  @param   treeHndl    [I]Handle to the search start item. NULL handle is Root Handle.
 *  @param   nodeUri     [I]The URI to search. This param can be NULL.
 *
 *  @return  If successful, the return value is the handle to the found item. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *               DMTREE_ERR_CMD_FAILED
 *
 *  @note    treeHndl can be the non-root node, use the exact node handle can accelerate the searching speed.
 *           URI can be absolute path(./DevDetail), or the relative path(Ext/FUMO).
 */
DM_TREE_HANDLE dmTree_searchItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri) {
    DMTREE_INTERNAL_ITEM* lpTreeHndl;
    DMTREE_INTERNAL_ITEM* lpTmpItem;
    IS8 uri[DMTREE_URI_MAXSIZE];
    IS8 *puri, *ptmp;
    const IS8* nuri;
    IS32 len, lenUri;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;
    lpTreeHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;

    /*
     *  Is current path
     */
    if (nodeUri == NULL || nodeUri[0] == '\0'
            || strcmp(lpTreeHndl->data->nodeUri, nodeUri) == 0) {
        dmTree_setErrorCode(DMTREE_ERR_OK);
        return treeHndl;
    }

    /*
     *  Set Init uri
     */
    strcpy(uri, lpTreeHndl->data->nodeUri);
    if (strlen(uri) >= (DMTREE_URI_MAXSIZE-1)) {
        dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
        return NULL;
    }
    strcat(uri, "/");
    lenUri = strlen(uri);
    puri = uri + lenUri;

    if (nodeUri[0] == '.') {
        /*
         *  Full path item
         */
        if (strncmp(uri, nodeUri, lenUri) != 0) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE search item failed! Invalid URI! %s \r\n", nodeUri);
            dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
            return NULL ;
        }
        nuri = (nodeUri + lenUri);
    } else {
        nuri = (nodeUri[0] == '/' ? (nodeUri + 1) : nodeUri);
    }

    /*
     *  Check path
     */
    do {
        ptmp = strchr(nuri, '/');
        if (ptmp != NULL ) {
            len = (IS32) (ptmp - nuri);
            strncpy(puri, nuri, len);
            puri[len] = '\0';
        } else {
            len = strlen(nuri);
            strcpy(puri, nuri);
        }

        /*
         *  Find the node
         */
        lpTmpItem = lpTreeHndl->child;
        while (lpTmpItem != NULL ) {
            if (strcmp(lpTmpItem->data->nodeUri + lenUri, puri) == 0)
                break;

            lpTmpItem = lpTmpItem->next;
        }
        if (lpTmpItem == NULL ) {
            return NULL ;
        }
        lpTreeHndl = lpTmpItem;

        if (ptmp != NULL ) {
            strcat(puri, "/");
            puri += len + 1;
            lenUri += len + 1;
            nuri = ptmp + 1;
        }
    } while (ptmp != NULL );

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return (DM_TREE_HANDLE) lpTreeHndl;
}

/**
 *  @brief   Inserts a new child item in the tree handle.
 *
 *  @param   treeHndl  [I]Handle to the parent item. NULL handle is Root Handle.
 *  @param   newItem   [I]Address of a DMTREE_ITEM structure that specifies
 *                     the attributes of the new tree item.
 *
 *  @return  If successful, the return value is the handle to the new item. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *               DMTREE_ERR_NOT_FOUND
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    You can use this function to add a new child-node,the parent-node must be exist to do this.
 *           The URI of the new node can be absolute path or relative path.
 *
 */
DM_TREE_HANDLE dmTree_insertItem(DM_TREE_HANDLE treeHndl,
        const DMTREE_ITEM* newItem) {
    DM_TREE_HANDLE insHndl;
    const IS8* pszTmp;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search the URI
     */
    if ((pszTmp = strrchr(newItem->nodeUri, '/')) != NULL ) {
        IS8 szNodeUri[DMTREE_URI_MAXSIZE];
        IS32 nLen;

        // Make URI
        nLen = (IS32) (pszTmp - newItem->nodeUri);
        strncpy(szNodeUri, newItem->nodeUri, nLen);
        szNodeUri[nLen] = '\0';

        // Search URI node
        if ((insHndl = dmTree_searchItem(treeHndl, szNodeUri)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE insertItem failed! not found the nodeURI: %s \r\n",
                    szNodeUri);
            dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
            return NULL ;
        }

        pszTmp++;
    } else {
        insHndl = treeHndl;
        pszTmp = newItem->nodeUri;
    }

    /*
     *  Is Node
     */
    if (strcmp(((DMTREE_INTERNAL_ITEM*) insHndl)->data->nodeFormat, "node")
            != 0) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE insert item failed! Leaf can't insert item! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOT_SUPPORTED);
        return NULL ;
    }

    /*
     **  Check data format
     */
    if (newItem->nodeData != NULL ) {
        if (!dmTreeCheckDataFormat(newItem->nodeFormat, newItem->nodeData,
                newItem->dataSize)) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE insert item failed! Data format error! \r\n");
            dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
            return NULL ;
        }
    }

    /*
     *  Exist the node name
     */
    {
        DMTREE_INTERNAL_ITEM* lpTmp = (DMTREE_INTERNAL_ITEM*) insHndl;
        IS32 nLen;

        nLen = strlen(lpTmp->data->nodeUri) + 1;
        lpTmp = lpTmp->child;
        while (lpTmp != NULL ) {
            if (strcmp(lpTmp->data->nodeName, newItem->nodeName) == 0) {
                dm_debug_trace(DM_DEBUG_TREE_MASK,
                        "DMTREE insert item failed! The node name have exist! %s \r\n",
                        newItem->nodeName);
                dmTree_setErrorCode(DMTREE_ERR_ALREADY_EXIST);
                return NULL ;
            } else if (strcmp(lpTmp->data->nodeUri + nLen, pszTmp) == 0) {
                dm_debug_trace(DM_DEBUG_TREE_MASK,
                        "DMTREE insert item failed! The node URI have exist! %s \r\n",
                        newItem->nodeUri);
                dmTree_setErrorCode(DMTREE_ERR_ALREADY_EXIST);
                return NULL ;
            }

            lpTmp = lpTmp->next;
        }
    }

    // Insert new Item
    if ((insHndl = (DM_TREE_HANDLE) dmTreeInternal_insertItem(
            (DMTREE_INTERNAL_ITEM*) insHndl, newItem)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE insert item failed! no memory!\r\n");
        return NULL ;
    }

    return insHndl;
}

/**
 *  @brief   This function sets some or all of a tree item's attributes.
 *
 *  @param   treeHndl    [I]Handle to the replace item. NULL handle is Root Handle.
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
 *               DMTREE_ERR_NOT_SUPPORTED
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    The treeHndl identifies the item, and the replaceMask parameters specifies which attributes to set.
 */
IS32 dmTree_replaceItem(DM_TREE_HANDLE treeHndl, IU32 replaceMask,
        const DMTREE_ITEM* replaceItem) {
    DM_TREE_HANDLE repHndl;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search the URI
     */
    if ((repHndl = dmTree_searchItem(treeHndl, replaceItem->nodeUri)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE replaceItem failed! not found the nodeURI: %s \r\n",
                replaceItem->nodeUri);
        dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
        return DMTREE_ERR_NOT_FOUND;
    }

    /*
     *  Replace Item
     */
    return dmTreeInternal_replaceItem((DMTREE_INTERNAL_ITEM*) repHndl,
            replaceMask, replaceItem);
}

/**
 *  @brief   Retrieves all of a tree item's attributes.
 *
 *  @param   treeHndl    [I]Handle to the parent item. NULL handle is Root Handle.
 *  @param   nodeUri     [I]The uri to get. This param can be NULL.
 *  @param   getItem     [I]Address of a DMTREE_ITEM structure that specifies the attributes of
 *                       the new tree item.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    The return data-pointer can only be read,you cann't release it or modify it.
 */
IBOOL dmTree_getItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri,
        DMTREE_ITEM* getItem) {
    DM_TREE_HANDLE getHndl;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search the URI
     */
    if (nodeUri != NULL ) {
        if ((getHndl = dmTree_searchItem(treeHndl, nodeUri)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE getItem failed! not found the nodeURI: %s \r\n",
                    nodeUri);
            dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
            return FALSE;
        }
    } else {
        getHndl = treeHndl;
    }

    /*
     *  Copy param
     */
    memcpy(getItem, ((DMTREE_INTERNAL_ITEM*) getHndl)->data,
            sizeof(DMTREE_ITEM));

    return TRUE;
}

/**
 *  @brief   Retrieves all of a tree item's attributes.
 *
 *  @param   treeHndl    [I]Handle to the parent item. NULL handle is Root Handle.
 *  @param   nodeUri     [I]The uri to get. This param can been NULL.
 *
 *  @return  Returns the pointer DMTREE_ITEM structure. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    The return data-pointer can only be read,you cann't release it or modify it.
 */
const DMTREE_ITEM* dmTree_getItemEx(DM_TREE_HANDLE treeHndl, const IS8* nodeUri) {
    DM_TREE_HANDLE getHndl;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search the URI
     */
    if (nodeUri != NULL ) {
        if ((getHndl = dmTree_searchItem(treeHndl, nodeUri)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE getItem failed! not found the nodeURI: %s \r\n",
                    nodeUri);
            dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
            return NULL ;
        }
    } else {
        getHndl = treeHndl;
    }

    return ((DMTREE_INTERNAL_ITEM*) getHndl)->data;
}

/**
 *  @brief   Removes an item and all its children from the DM tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *                     If treeHndl is set to NULL, all items are deleted.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTree_deleteItem(DM_TREE_HANDLE treeHndl) {
    DMTREE_INTERNAL_ITEM* lpTreeHndl;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL ) {
        treeHndl = g_hTreeRootHndl;
        g_hTreeRootHndl = NULL;
    }
    lpTreeHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;

    if (lpTreeHndl->prev != NULL ) {
        lpTreeHndl->prev->next = lpTreeHndl->next;
    } else if (lpTreeHndl->parent != NULL ) {
        lpTreeHndl->parent->child = lpTreeHndl->next;
    }
    if (lpTreeHndl->next != NULL ) {
        lpTreeHndl->next->prev = lpTreeHndl->prev;
    }

    return dmTree_postTraverseTree(treeHndl, NULL, dmTreeInternalCB_deleteItem,
            0);
}

/**
 *  @brief   Free an item and all its children from the DM tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *                     If treeHndl is set to NULL, all items are deleted.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTree_freeItem(DM_TREE_HANDLE treeHndl) {
    DMTREE_INTERNAL_ITEM* lpTreeHndl;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL ) {
        treeHndl = g_hTreeRootHndl;
        g_hTreeRootHndl = NULL;
    }
    lpTreeHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;

    if (lpTreeHndl->prev != NULL ) {
        lpTreeHndl->prev->next = lpTreeHndl->next;
    } else if (lpTreeHndl->parent != NULL ) {
        lpTreeHndl->parent->child = lpTreeHndl->next;
    }
    if (lpTreeHndl->next != NULL ) {
        lpTreeHndl->next->prev = lpTreeHndl->prev;
    }

    return dmTree_postTraverseTree(treeHndl, NULL, dmTreeInternalCB_freeItem, 0);
}

/**
 *  @brief   This function traverses the tree in pre-order and do the
 *           operation (callback function pointer) on the traversed nodes. \n
 *           Pre-order: Parent is processed prior to its children nodes.
 *
 *  @param   treeHndl  [I]Pointer to the starting node of the sub tree.
 *  @param   startHndl [I]Start pointer handle. It can been NULL. this handle max is treeHandl's child node.
 *  @param   func      [I]Callback function pointer for the operation on the node.
 *  @param   param     [I]Specifies an user defined value passed to the callback function.
 *
 *  @return  If the function traverse the tree over, the return value is TRUE. \n
 *           Otherwise, the return value is FALSE. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    If you want ternimated during traversing Tree,you should return FALSE in the callback function.
 *           If startHndl is not NULL, the searching will start from the position stopped last time.
 */
IBOOL dmTree_preTraverseTree(DM_TREE_HANDLE treeHndl, DM_TREE_HANDLE startHndl,
        DMTREE_TRAVERSEFUNC func, IS32 param) {
    DMTREE_INTERNAL_ITEM* lpRootHndl;
    DMTREE_INTERNAL_ITEM* lpStartHndl;
    DMTREE_INTERNAL_ITEM* lpPos;
    IBOOL done;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Init value
     */
    lpRootHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;
    lpStartHndl = (DMTREE_INTERNAL_ITEM*) startHndl;
    done = FALSE;
    if (lpStartHndl == NULL )
        lpStartHndl = lpRootHndl;

    lpPos = lpStartHndl;

    while (!done) {
        /*
         *  Do operation first.
         */
        if (!func((DM_TREE_HANDLE) lpPos, lpPos->data, param)) {
            return FALSE;
        }

        /*
         *  Operation on child node gets the precedence over sibling node
         */
        if (lpPos->child != NULL ) {
            lpPos = lpPos->child;
        } else if (lpPos->next != NULL ) {
            lpPos = lpPos->next;
        } else {
            do {
                lpPos = lpPos->parent;
                // Stop when it came back to the root node
                if (lpPos == lpRootHndl)
                    done = TRUE;
            } while (lpPos->next == NULL && !done);

            // Sibling exists, update the node and continue with traversing
            if (lpPos->next != NULL )
                lpPos = lpPos->next;
        }
    }

    return TRUE;
}

/**
 *  @brief   This function traverses the tree in post-order and do the
 *           operation (callback function pointer) on the traversed nodes. \n
 *
 *  @param   treeHndl  [I]Pointer to the starting node of the sub tree.
 *  @param   startHndl [I]Start pointer handle. It can been NULL. this handle max is treeHandl's child node.
 *  @param   func      [I]Callback function pointer for the operation on the node.
 *  @param   param     [I]Specifies an user defined value passed to the callback function.
 *
 *  @return  If the function traverse the tree over, the return value is TRUE. \n
 *           Otherwise, the return value is FALSE. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    If you want ternimated during traversing Tree,you should return FALSE in the callback function.
 *           If startHndl is not NULL, the searching will start from the position stopped last time.
 */
IBOOL dmTree_postTraverseTree(DM_TREE_HANDLE treeHndl, DM_TREE_HANDLE startHndl,
        DMTREE_TRAVERSEFUNC func, IS32 param) {
    DMTREE_INTERNAL_ITEM* lpRootHndl;
    DMTREE_INTERNAL_ITEM* lpStartHndl;
    DMTREE_INTERNAL_ITEM* lpPos, *lpNextItem;
    IBOOL done;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Init value
     */
    lpRootHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;
    lpStartHndl = (DMTREE_INTERNAL_ITEM*) startHndl;
    done = FALSE;

    if (lpStartHndl == NULL ) {
        lpStartHndl = lpRootHndl;
        while (lpStartHndl->child != NULL )
            lpStartHndl = lpStartHndl->child;
    }

    lpPos = lpStartHndl;

    while (lpPos != lpRootHndl) {
        while (lpPos->next != NULL ) {
            // get a hold of the next sibling
            lpNextItem = lpPos->next;

            // Do the operation one the current node
            if (!func((DM_TREE_HANDLE) lpPos, lpPos->data, param)) {
                return FALSE;
            }

            // goto the next sibling
            lpPos = lpNextItem;

            while (lpPos->child != NULL )
                lpPos = lpPos->child;
        }

        // go back to the parent node
        lpNextItem = lpPos->parent;

        // Do the operation one the current node
        if (!func((DM_TREE_HANDLE) lpPos, lpPos->data, param)) {
            return FALSE;
        }

        lpPos = lpNextItem;
    }

    /*
     *  Do parent item operation
     */
    if (!func((DM_TREE_HANDLE) lpRootHndl, lpRootHndl->data, param)) {
        return FALSE;
    }

    return TRUE;
}

/**
 *  @brief   Retrieves the parent of the specified item.
 *
 *  @param   treeHndl  [I]Handle to an item.
 *
 *  @return  Returns the handle to the item if successful. For most cases, the message
 *           returns a NULL value to indicate an error. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    This function will return NULL if the item no exist.
 */
DM_TREE_HANDLE dmTree_getParent(DM_TREE_HANDLE treeHndl) {
    return (DM_TREE_HANDLE) (((DMTREE_INTERNAL_ITEM*) treeHndl)->parent);
}

/**
 *  @brief   Retrieves the first child item of the specified item.
 *
 *  @param   treeHndl  [I]Handle to an item.
 *
 *  @return  Returns the handle to the item if successful. For most cases, the message
 *           returns a NULL value to indicate an error. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    This function will return NULL if the item no exist.
 */
DM_TREE_HANDLE dmTree_getChild(DM_TREE_HANDLE treeHndl) {
    return (DM_TREE_HANDLE) (((DMTREE_INTERNAL_ITEM*) treeHndl)->child);
}

/**
 *  @brief   Retrieves the previous sibling item.
 *
 *  @param   treeHndl  [I]Handle to an item.
 *
 *  @return  Returns the handle to the item if successful. For most cases, the message
 *           returns a NULL value to indicate an error. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    This function will return NULL if the item no exist.
 */
DM_TREE_HANDLE dmTree_getPrevious(DM_TREE_HANDLE treeHndl) {
    return (DM_TREE_HANDLE) (((DMTREE_INTERNAL_ITEM*) treeHndl)->prev);
}

/**
 *  @brief   Retrieves the next sibling item.
 *
 *  @param   treeHndl  [I]Handle to an item.
 *
 *  @return  Returns the handle to the item if successful. For most cases, the message
 *           returns a NULL value to indicate an error. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    This function will return NULL if the item no exist.
 */
DM_TREE_HANDLE dmTree_getNext(DM_TREE_HANDLE treeHndl) {
    return (DM_TREE_HANDLE) (((DMTREE_INTERNAL_ITEM*) treeHndl)->next);
}

/**
 *  @brief  Check the ACL.
 *
 *  @param  treeHndl  [I]Handle to the node item.
 *  @param  serverID  [I]Server Id to check ACL.
 *  @param  aclType   [I]Check the ACL type;
 *
 *  @return DMTREE_ERR_OK   - Check OK!  \n
 *          DMTREE_ERR_NOT_ALLOWED - support the operation.
 *
 *  @note   none.
 */
IS32 dmTreeSml_checkACL(DM_TREE_HANDLE treeHndl, const IS8* serverID,
        IU32 aclType) {
    DMTREE_INTERNAL_ITEM* lpTreeHndl = (DMTREE_INTERNAL_ITEM*) treeHndl;
    DMTREE_ACLSVRLIST* lpAclList;

    if (serverID == NULL )
        return DMTREE_ERR_OK;

    /*
     *  Check All server
     */
    if ((lpTreeHndl->data->allSrvAcl & aclType) != aclType) {
        /*
         **  No acl server list
         */
        if (lpTreeHndl->data->aclSvrList == NULL ) {
            return DMTREE_ERR_NOT_ALLOWED;
        }
        /*
         *  Check server list
         */
        lpAclList = lpTreeHndl->data->aclSvrList;
        while (lpAclList != NULL ) {
            if (strcmp(lpAclList->serverID, serverID) == 0) {
                if ((lpAclList->accType & aclType) != aclType) {
                    return DMTREE_ERR_NOT_ALLOWED;
                }
                break;
            }

            lpAclList = lpAclList->next;
        }
    }

    return DMTREE_ERR_OK;
}

/**
 *  @brief   Inserts a new child item in the tree handle.
 *
 *  @param   treeHndl  [I]Handle to the parent item. NULL handle is Root Handle.
 *  @param   newItem   [I]Address of a DMTREE_ITEM structure that specifies
 *                     the attributes of the new tree item.
 *  @param   serverID  [I]Server Id to check ACL. This param can be NULL.
 *
 *  @return  If successful, the return value is the handle to the new item. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *               DMTREE_ERR_NOT_FOUND
 *               DMTREE_ERR_NOT_ALLOWED
 *               DMTREE_ERR_NOT_SUPPORTED
 *               DMTREE_ERR_CMD_FAILED
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    This function only add a child-node under the parent-node.
 *
 */
DM_TREE_HANDLE dmTreeSml_insertItem(DM_TREE_HANDLE treeHndl,
        const DMTREE_ITEM* newItem, const IS8* serverID) {
    DM_TREE_HANDLE insHndl;
    IS8* pszTmp;
    IS32 ret;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Check URI
     */
    if ((ret = dmTreeInternal_checkValidUri(newItem->nodeUri)) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "Insert Item failed! Invalid URI: %s \r\n", newItem->nodeUri);
        return NULL ;
    }

    /*
     *  Search the URI
     */
    if ((pszTmp = strrchr(newItem->nodeUri, '/')) != NULL ) {
        IS8 szNodeUri[DMTREE_URI_MAXSIZE];
        IS32 nLen;

        // Make URI
        nLen = (IS32) (pszTmp - newItem->nodeUri);
        strncpy(szNodeUri, newItem->nodeUri, nLen);
        szNodeUri[nLen] = '\0';

        // Search URI node
        if ((insHndl = dmTree_searchItem(treeHndl, szNodeUri)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "Insert Item failed! Invalid URI: %s \r\n",
                    newItem->nodeUri);
            dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
            return NULL ;
        }
    } else {
        insHndl = treeHndl;
    }

    /*
     *  check ACL auth
     */
    if (dmTreeSml_checkACL(insHndl, serverID,
            DMTREE_ACCESS_ADD) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE insertItem failed! check ACL Failed! \r\n",
                newItem->nodeUri);
        /*  if (strncmp(newItem->nodeUri, "./Application", 13)) // Jianghong Song for bug 9433 20070111
         {
         dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
         }
         else
         {
         dmTree_setErrorCode(DMTREE_ERR_PERMISSION_DENIED);
         }*/
        dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
        return NULL ;
    }

    // Insert new Item
    if ((insHndl = (DM_TREE_HANDLE) dmTree_insertItem(insHndl, newItem))
            == NULL ) {
        return NULL ;
    }

    g_dmTreeObjSave = TRUE;
    ((DMTREE_INTERNAL_ITEM*) insHndl)->modMask |= DMTREE_REPLACE_NEWADD;
    return insHndl;
}

/**
 *  @brief   Inserts a new child item in the tree handle.
 *
 *  @param   treeHndl  [I]Handle to the parent item. NULL handle is Root Handle.
 *  @param   newItem   [I]Address of a DMTREE_ITEM structure that specifies
 *                     the attributes of the new tree item.
 *  @param   serverID  [I]Server Id to check ACL. This param can be NULL.
 *
 *  @return  If successful, the return value is the handle to the new item. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *               DMTREE_ERR_NOT_FOUND
 *               DMTREE_ERR_NOT_ALLOWED
 *               DMTREE_ERR_NOT_SUPPORTED
 *               DMTREE_ERR_CMD_FAILED
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    A child-node will be added under the parent-node. If the path(URI) of the
 *           new node doesn't exist, this path will be added,like implicit add operation .
 */
DM_TREE_HANDLE dmTreeSml_insertItemEx(DM_TREE_HANDLE treeHndl,
        const DMTREE_ITEM* newItem, const IS8* serverID) {
    DMTREE_INTERNAL_ITEM* lpParentItem;
    DMTREE_INTERNAL_ITEM* lpInteItem;
    IS8 uri[DMTREE_URI_MAXSIZE];
    IS8 *puri, *ptmp;
    const IS8 *nuri;
    IS32 len, lenUri;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Check URI
     */
    if (dmTreeInternal_checkValidUri(newItem->nodeUri) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "Insert Item failed! Invalid URI: %s \r\n", newItem->nodeUri);
        return NULL ;
    }

    lpParentItem = (DMTREE_INTERNAL_ITEM*) treeHndl;
    strcpy(uri, lpParentItem->data->nodeUri);
    if (strlen(uri) >= (DMTREE_URI_MAXSIZE-1)) {
        dmTree_setErrorCode(DMTREE_ERR_URI_TOO_LONG);
        return NULL;
    }
    strcat(uri, "/");
    lenUri = strlen(uri);
    puri = uri + lenUri;

    if (newItem->nodeUri[0] == '.') {
        /*
         *  Full path item
         */
        if (strncmp(uri, newItem->nodeUri, lenUri) != 0) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "DMTREE InsertItemEx failed! Invalid URI! %s \r\n",
                    newItem->nodeUri);
            dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
            return NULL ;
        }
        nuri = newItem->nodeUri + lenUri;
    } else {
        nuri = ((newItem->nodeUri[0] == '/') ?
                (newItem->nodeUri + 1) : newItem->nodeUri);
    }

    /*
     *  Check path
     */
    do {
        ptmp = strchr(nuri, '/');
        if (ptmp != NULL ) {
            /*
             *  Insert item path
             */
            len = (IS32) (ptmp - nuri);
            strncpy(puri, nuri, len);
            puri[len] = '\0';

            /*
             *  Find the node
             */
            lpInteItem = lpParentItem->child;
            while (lpInteItem != NULL ) {
                if (strcmp(lpInteItem->data->nodeUri + lenUri, puri) == 0)
                    break;

                lpInteItem = lpInteItem->next;
            }
            /*
             *  Insert child path
             */
            if (lpInteItem == NULL ) {
                memset(&g_dmTreeTmpItem, 0, sizeof(DMTREE_ITEM));
                strcpy(g_dmTreeTmpItem.nodeName, puri);
                strcpy(g_dmTreeTmpItem.nodeUri, uri);
                strcpy(g_dmTreeTmpItem.nodeFormat, "node");
                g_dmTreeTmpItem.occurrence = newItem->occurrence;
                g_dmTreeTmpItem.scope = newItem->scope;
                g_dmTreeTmpItem.allSrvAcl = newItem->allSrvAcl;
                g_dmTreeTmpItem.aclSvrList = newItem->aclSvrList;

                if ((lpInteItem = dmTreeSml_insertItem(lpParentItem,
                        &g_dmTreeTmpItem, serverID)) == NULL )
                    return NULL ;
            }
            lpParentItem = lpInteItem;

            if (ptmp != NULL ) {
                strcat(puri, "/");
                puri += len + 1;
                lenUri += len + 1;
                nuri = ptmp + 1;
            }
        }
    } while (ptmp != NULL );

    /*
     *  Insert new item
     */
    if ((lpInteItem = dmTreeSml_insertItem(lpParentItem, newItem, serverID))
            == NULL )
        return NULL ;

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return (DM_TREE_HANDLE) lpInteItem;
}

/**
 *  @brief   Removes an item and all its children from the DM tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete. NULL handle is Root Handle.
 *                     If treeHndl is set to root handle, all items are deleted.
 *  @param   nodeUri   [I]The delete node uri. This param can be NULL.
 *  @param   serverID  [I]Server Id to check ACL. This param can be NULL.
 *
 *  @return  DMTREE_ERR_OK   - Check OK!  \n
 *           DMTREE_ERR_NOT_ALLOWED - support the operation.
 *               DMTREE_ERR_NOT_ALLOWED
 *               DMTREE_ERR_NOT_FOUND
 *               DMTREE_ERR_CMD_FAILED
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IS32 dmTreeSml_deleteItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri,
        const IS8* serverID) {
    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search NodeURi item
     */
    if (nodeUri != NULL ) {
        if ((treeHndl = dmTree_searchItem(treeHndl, nodeUri)) == NULL ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "dmTreeSml_deleteItem Found the Uri failed!\r\n");
            dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
            return DMTREE_ERR_NOT_FOUND;
        }
    }

    /*
     *  check ACL auth
     */
    if (dmTreeSml_checkACL(treeHndl, serverID,
            DMTREE_ACCESS_DELETE) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeSml_deleteItem check ACL Failed!\r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
        return DMTREE_ERR_NOT_ALLOWED;
    }

    /*
     *  Delete item
     */
    g_dmTreeObjSave = TRUE;
    if (!dmTree_deleteItem(treeHndl)) {
        return DMTREE_ERR_CMD_FAILED;
    }

    return DMTREE_ERR_OK;
}

/**
 *  @brief   Retrieves all of a tree item's attributes.
 *
 *  @param   treeHndl    [I]Handle to the root item. NULL handle is Root Handle.
 *  @param   nodeUri     [I]The get node uri. This param can be NULL.
 *  @param   serverID    [I]Server Id to check ACL. This param can be NULL.
 *
 *  @return  If successful, the return value is the handle to the new item. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none.
 */
DM_TREE_HANDLE dmTreeSml_getItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri,
        const IS8* serverID) {
    DM_TREE_HANDLE getHndl;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search node URI item
     */
    if ((getHndl = dmTree_searchItem(treeHndl, nodeUri)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE getItem failed! Search URI Failed: %s \r\n", nodeUri);
        dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
        return NULL ;
    }

    /*
     *  Check ACL
     */
    if (dmTreeSml_checkACL(getHndl, serverID,
            DMTREE_ACCESS_GET) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeSml_getItem check ACL Failed!\r\n");

        dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);

        return NULL ;
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return getHndl;
}

/**
 *  @brief   This function sets some or all of a tree item's attributes.
 *
 *  @param   treeHndl    [I]Handle to the parent item. NULL handle is Root Handle.
 *  @param   replaceMask [I]Array of flags that indicate which of the other structure members
 *                       contain valid data. This member can be one or more of the following values. \n
 *                         DMTREE_REPLACE_NODENAME     \n
 *                         DMTREE_REPLACE_NODEDATA     \n
 *                         DMTREE_REPLACE_ACCESSTYPE
 *  @param   replaceItem [I]Address of a DMTREE_ITEM structure that specifies the attributes of
 *                       the new tree item.
 *  @param   serverID    [I]Server Id to check ACL. This param can be NULL.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *               DMTREE_ERR_NOT_SUPPORTED
 *               DMTREE_ERR_NOT_ALLOWED
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    The treeHndl identifies the item, and the replaceMask parameters specifies which attributes to set.
 */
IS32 dmTreeSml_replaceItem(DM_TREE_HANDLE treeHndl, IU32 replaceMask,
        const DMTREE_ITEM* replaceItem, const IS8* serverID) {
    DM_TREE_HANDLE repHndl;
    IS32 ret;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search node URI item
     */
    if ((repHndl = dmTree_searchItem(treeHndl, replaceItem->nodeUri)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE replaceItem failed! Search URI Failed: %s \r\n",
                replaceItem->nodeUri);
        dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
        return DMTREE_ERR_NOT_FOUND;
    }

    /*
     *  check ACL auth
     */
    if (dmTreeSml_checkACL(repHndl, serverID,
            DMTREE_ACCESS_REPLACE) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeSml_replaceItem check ACL Failed!\r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
        return DMTREE_ERR_NOT_ALLOWED;
    }

    /*
     *  Replace item
     */
    if ((ret = dmTreeInternal_replaceItem((DMTREE_INTERNAL_ITEM*) repHndl,
            replaceMask, replaceItem)) != DMTREE_ERR_OK) {
        return ret;
    }

    g_dmTreeObjSave = TRUE;
    ((DMTREE_INTERNAL_ITEM*) repHndl)->modMask |= replaceMask;
    return DMTREE_ERR_OK;
}

/**
 *  @brief   This function sets some or all of a tree item's attributes.
 *
 *  @param   treeHndl    [I]Handle to the tree item. NULL handle is Root Handle.
 *  @param   replaceMask [I]Array of flags that indicate which of the other structure members
 *                       contain valid data. This member can be one or more of the following values. \n
 *                         DMTREE_REPLACE_NODENAME     \n
 *                         DMTREE_REPLACE_NODEDATA     \n
 *                         DMTREE_REPLACE_ACCESSTYPE
 *  @param   param       [I]The param.
 *  @param   serverID    [I]Server Id to check ACL. This param can be NULL.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *               DMTREE_ERR_NOT_SUPPORTED
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    The treeHndl identifies the item, and the replaceMask parameters specifies which attributes to set.
 */
IS32 dmTreeSml_replaceItemEx(DM_TREE_HANDLE treeHndl, IU32 replaceMask,
        const IS8* nodeUri, IS32 param, const IS8* serverID) {
    DM_TREE_HANDLE repHndl;
    IS32 ret;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *  Search node URI item
     */
    if ((repHndl = dmTree_searchItem(treeHndl, nodeUri)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE replaceItemEx failed! Search URI Failed: %s \r\n",
                nodeUri);
        dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
        return DMTREE_ERR_NOT_FOUND;
    }

    /*
     *  check ACL auth
     */
    if (dmTreeSml_checkACL(repHndl, serverID,
            DMTREE_ACCESS_REPLACE) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "dmTreeSml_replaceItemEx check ACL Failed!\r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
        return DMTREE_ERR_NOT_ALLOWED;
    }

    // node name
    if (replaceMask == DMTREE_REPLACE_NODENAME) {
        strcpy(g_dmTreeTmpItem.nodeName, (IS8*) param);
    }
    // node data
    else if (replaceMask == DMTREE_REPLACE_NODEDATA) {
        g_dmTreeTmpItem.dataType = DMTREE_DATATYPE_DATA;
        g_dmTreeTmpItem.dataSize = -1;
        g_dmTreeTmpItem.nodeData = (IS8*) param;
    }
    // acl list
    else if (replaceMask == DMTREE_REPLACE_ACLLIST) {
        /*
         *  Replace acl server list
         */
        if ((ret = dmTreeInternal_replaceAcl((DMTREE_INTERNAL_ITEM*) repHndl,
                (const IS8*) param)) != DMTREE_ERR_OK) {
            return ret;
        }
        g_dmTreeObjSave = TRUE;
        ((DMTREE_INTERNAL_ITEM*) repHndl)->modMask |= replaceMask;
        return DMTREE_ERR_OK;
    } else {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE ReplaceItemEx failed! Invalid Replace Mask!\r\n");
        return DMTREE_ERR_CMD_FAILED;
    }

    /*
     *  Replace item
     */
    if ((ret = dmTreeInternal_replaceItem((DMTREE_INTERNAL_ITEM*) repHndl,
            replaceMask, &g_dmTreeTmpItem)) != DMTREE_ERR_OK) {
        return ret;
    }

    g_dmTreeObjSave = TRUE;
    ((DMTREE_INTERNAL_ITEM*) repHndl)->modMask |= replaceMask;
    return DMTREE_ERR_OK;
}

/**
 *  @brief   Exec node.
 *
 *  @param   treeHndl    [I]Handle to the tree item. NULL handle is Root Handle.
 *  @param   nodeUri     [I]The node uri. This param can be NULL.
 *  @param   serverID    [I]Server Id to check ACL. This param can be NULL.
 *  @param   param       [I]The Exec command param value. This param can be NULL.
 *  @param   correlator  [I]The correlator string for AsyncExec. This param can be NULL.
 *  @param   callType    [I]Exec command callback function call type.
 *                          1 DM Call, 0 Other Call.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *               DMTREE_ERR_NOT_SUPPORTED
 *
 *  @note   none.
 */
IS32 dmTreeSml_execItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri,
        const IS8* serverId, const IS8* param, const IS8* correlator,
        IS32 callType) {
    DM_TREE_HANDLE execHndl;
    DMTREE_INTERNAL_ITEM* lpExecItem;
    IS32 ret = DMTREE_ERR_OK;

    /*
     **  Is Root handle
     */
    if (treeHndl == NULL )
        treeHndl = g_hTreeRootHndl;

    /*
     *   Search node URI item
     */
    if ((execHndl = dmTree_searchItem(treeHndl, nodeUri)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE execItem failed! Search URI Failed: %s \r\n", nodeUri);
        dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
        return DMTREE_ERR_NOT_FOUND;
    }

    /*
     *   check ACL auth
     */
    if (dmTreeSml_checkACL(execHndl, serverId,
            DMTREE_ACCESS_EXEC) != DMTREE_ERR_OK) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMTREE execItem check ACL Failed!\r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOT_ALLOWED);
        return DMTREE_ERR_NOT_ALLOWED;
    }

    lpExecItem = (DMTREE_INTERNAL_ITEM*) execHndl;

    /*
     *   Exec item
     */

    if (lpExecItem->data->execFunc != NULL ) {
        ret = lpExecItem->data->execFunc(execHndl, param, correlator, callType);
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return ret;
}

/**
 *  @brief   This function create one DM Account all node tree.
 *
 *  @param   nodeName  [I]The DM Account node name.
 *  @param   serverID  [I]The DM Account ServerID.
 *
 *  @return  If successful, the return value is the handle to the new DM Account node. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none.
 */
DM_TREE_HANDLE dmTreeDmAcc_create(const IS8* nodeName, const IS8* serverID) {
    DM_TREE_HANDLE treeHndl = g_hTreeRootHndl;
    DMTREE_INTERNAL_ITEM* dmAccHndl;
    DMTREE_INTERNAL_ITEM* lpAccNode;

    IS8* serverIDUri;

    /*
     *  Check param
     */
    if (treeHndl == NULL || nodeName == NULL || serverID == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Invalidate param! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return NULL ;
    }

    // get serverID node URI
    dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerID, &serverIDUri, NULL );

    /*
     *  Search DM ACC Path
     */
    if ((dmAccHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(treeHndl,
            DMTREE_DMACC_PATH)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Search DMAcc Path failed! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return NULL ;
    }

    /*
     *  Search ServerID node
     */
    if ((lpAccNode = dmTreeInternal_searchDMAcc(dmAccHndl, serverID)) != NULL ) {
        /*
         *  Node name is same?
         */
        if (strcmp(lpAccNode->data->nodeName, nodeName) == 0) {
            /*
             *  Have same ACC info
             */
            return (DM_TREE_HANDLE) lpAccNode;
        } else {
            /*
             *  Delete the acc info
             */
            dmTree_deleteItem((DM_TREE_HANDLE) lpAccNode);
        }
    }

    /*
     *  Search Node name
     */
    if ((lpAccNode = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
            (DM_TREE_HANDLE) dmAccHndl, nodeName)) != NULL ) {
        /*
         *  Replace the ServerID node
         */
        dmTreeSml_replaceItemEx((DM_TREE_HANDLE) lpAccNode,
                DMTREE_REPLACE_NODEDATA, serverIDUri, (IS32) serverID, NULL );

        // Replace server list ServerID value
        dmTree_preTraverseTree((DM_TREE_HANDLE) lpAccNode, NULL,
                dmTreeInternalCB_replaceDMAccServerList, (IS32) serverID);
    } else {
        /*
         *  Create DmAcc node
         */
        if ((lpAccNode = dmTreeInternal_createDMAcc(dmAccHndl,
                g_stAccStdDataTable, nodeName, serverID)) == NULL ) {
            return NULL ;
        }
    }

    return (DM_TREE_HANDLE) lpAccNode;
}

/**
 *  @brief   This function lists all the servers ID in the DM tree.
 *
 *  @param   none.
 *
 *  @return  If successful, the return value is the pointer to the server ID list. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    If function return successful, you mast free the server ID buffer.
 */
IS8* dmTreeDmAcc_getServerList(void) {
    DM_TREE_HANDLE treeHndl = g_hTreeRootHndl;
    DMTREE_INTERNAL_ITEM* dmAccHndl;
    DMTREE_INTERNAL_ITEM* nodeHndl;
    DMTREE_INTERNAL_ITEM* idHndl;
    IS8* serverIDUri;
    IS8* srvLst;
    IS8* tmp;
    IS32 len;

    /*
     *  Check param
     */
    if (treeHndl == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Invalidate param! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return NULL ;
    }

    /*
     *  Find DMAcc
     */
    if ((dmAccHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(treeHndl,
            DMTREE_DMACC_PATH)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Search DMAcc Path failed! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return NULL ;
    }

    /*
     *  Count total length
     */
    dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerID, &serverIDUri, NULL );
    nodeHndl = dmAccHndl->child;
    len = 0;
    while (nodeHndl != NULL ) {
        idHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, serverIDUri);
        if (idHndl != NULL && idHndl->data->nodeData != NULL )
            len += idHndl->data->dataSize + 1;

        nodeHndl = nodeHndl->next;
    }
    if (len == 0) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "No DMAccount info! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return NULL ;
    }

    /*
     *  Malloc buffer
     */
    if ((srvLst = malloc(len + 1)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "Malloc server list buffer failed! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOMEMORY);
        return NULL ;
    }
    memset(srvLst, 0, len + 1);

    /*
     *  Get server List
     */
    nodeHndl = dmAccHndl->child;
    tmp = srvLst;
    while (nodeHndl != NULL ) {
        idHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, serverIDUri);
        if (idHndl != NULL && idHndl->data->nodeData != NULL ) {
            strcpy(tmp, idHndl->data->nodeData);
            tmp += idHndl->data->dataSize + 1;
        }

        nodeHndl = nodeHndl->next;
    }

    return srvLst;
}

static char* itoa(char *s, int n) {
    int sign;
    char *ptr;
    ptr = s;
    if ((sign = n) < 0) /* record sign */
        n = -n; /* make n positive */
    do { /* generate digits in reverse order */
        *ptr++ = n % 10 + '0'; /* get next digit */
    } while ((n = n / 10) > 0); /* delete it */
    if (sign < 0)
        *ptr++ = '-';
    *ptr = '\0';
    return s;
}

/**
 *  @brief   This function add the DM Account info and the related Con
 *           info from the tree using the serverId.
 *
 *  @param   accInfo   [I]Structure to store DM Account Info.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IS32 dmTreeDmAcc_addReplaceAccount(const DMTREE_ACC_INFO* accInfo) {
    DM_TREE_HANDLE treeHndl = g_hTreeRootHndl;
    DMTREE_INTERNAL_ITEM* nodeHndl;
    DMTREE_INTERNAL_ITEM* itemHndl;
    IS8* itemUri;

    /*
     *  Check param
     */
    if (treeHndl == NULL || accInfo == NULL || accInfo->accNodeName[0] == '\0'
            || accInfo->accServerId[0] == '\0') {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Invalidate param! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return DMTREE_ERR_CMD_FAILED;
    }

    /*
     *  Create Dm acc
     */
    if ((nodeHndl = dmTreeDmAcc_create(accInfo->accNodeName,
            accInfo->accServerId)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DMAcc create failed: NodeName: %s ServerID: %s \r\n",
                accInfo->accNodeName, accInfo->accServerId);
        return dmTree_getErrorCode();
    }

    /*
     *  Replace dm acc item
     */
    g_dmTreeTmpItem.dataType = DMTREE_DATATYPE_DATA;
    g_dmTreeTmpItem.dataSize = -1;
    g_dmTreeTmpItem.nodeUri[0] = '\0';

    // App ID
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_AppID, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData, accInfo->accAppID)
                                != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accAppID;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Server ID
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerID, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accServerId) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accServerId;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Name
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_Name, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData, accInfo->accName)
                                != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accName;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // PrefConRef
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_PrefConRef, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accPrefConRef) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accPrefConRef;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Con Ref
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ConRef, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData, accInfo->accConRef)
                                != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accConRef;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Addr
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_Addr, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData, accInfo->accAddr)
                                != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accAddr;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // AddrType
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_AddrType, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accAddrType) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accAddrType;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Port Number
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_PortNbr, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL ) {
            IS8 num[8];
            itoa(num, accInfo->accPortNbr);
            g_dmTreeTmpItem.nodeData = num;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Auth Pref
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_AuthPref, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                dmTree_AuthType_etos(accInfo->accAuthPref)) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) dmTree_AuthType_etos(
                    accInfo->accAuthPref);
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Server AuthLevel
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerAuthLevel, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accServerAuthLevel) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accServerAuthLevel;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Server Auth Type
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerAuthType, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                dmTree_AuthType_etos(
                                        accInfo->accServerAuthType)) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) dmTree_AuthType_etos(
                    accInfo->accServerAuthType);
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Server Auth Type
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerAuthName, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accServerAuthName) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accServerAuthName;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Server Psw
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerPw, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accServerPw) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accServerPw;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Server Nonce
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerNonce, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                (const IS8 *) accInfo->accServerNonce) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accServerNonce;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Prev Server Nonce
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_PrevServerNonce, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                (const IS8 *) accInfo->accPrevServerNonce) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accPrevServerNonce;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Client Auth Level
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ClientAuthLevel, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accClientAuthLevel) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accClientAuthLevel;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Client Auth Type
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ClientAuthType, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                dmTree_AuthType_etos(
                                        accInfo->accClientAuthType)) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) dmTree_AuthType_etos(
                    accInfo->accClientAuthType);
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // User Name
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_UserName, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                accInfo->accUserName) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accUserName;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // User Psw
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_UserPw, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData, accInfo->accUserPw)
                                != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accUserPw;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    // Client Nonce
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ClientNonce, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL
                && (itemHndl->data->nodeData == NULL
                        || strcmp(itemHndl->data->nodeData,
                                (const IS8 *) accInfo->accClientNonce) != 0)) {
            g_dmTreeTmpItem.nodeData = (IS8*) accInfo->accClientNonce;
            dmTree_replaceItem((DM_TREE_HANDLE) itemHndl,
                    DMTREE_REPLACE_NODEDATA, &g_dmTreeTmpItem);
        }
    }

    g_dmTreeObjSave = TRUE;
    dmTree_setErrorCode(DMTREE_ERR_OK);
    return DMTREE_ERR_OK;
}

/**
 *  @brief   This function get the DM Account info and the related Con
 *           info from the tree using the serverId.
 *
 *  @param   serverID  [I]Server Id to find the corresponding DM Account.
 *  @param   accInfo   [O]Structure to store DM Account Info.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IS32 dmTreeDmAcc_getAccountInfo(const IS8* serverID, DMTREE_ACC_INFO* accInfo) {
    DM_TREE_HANDLE treeHndl = g_hTreeRootHndl;
    DMTREE_INTERNAL_ITEM* nodeHndl;
    DMTREE_INTERNAL_ITEM* itemHndl;
    IS8* itemUri;

    /*
     *  Check param
     */
    if (treeHndl == NULL || serverID == NULL || accInfo == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Invalidate param! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return DMTREE_ERR_CMD_FAILED;
    }

    /*
     *  Search server ID
     */
    if ((nodeHndl = dmTreeInternal_searchDMAcc((DMTREE_INTERNAL_ITEM*) treeHndl,
            serverID)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Search DMAcc path failed! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
        return DMTREE_ERR_NOT_FOUND;
    }

    /*
     *  Get item info
     */
    memset(accInfo, 0, sizeof(DMTREE_ACC_INFO));

    // Node name
    strcpy(accInfo->accNodeName, nodeHndl->data->nodeName);

    // App ID
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_AppID, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accAppID, itemHndl->data->nodeData);
    }

    // Server ID
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerID, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accServerId, itemHndl->data->nodeData);
    }

    // Name
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_Name, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accName, itemHndl->data->nodeData);
    }

    // PrefConRef
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_PrefConRef, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accPrefConRef, itemHndl->data->nodeData);
    }

    // Con Ref
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ConRef, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accConRef, itemHndl->data->nodeData);
    }

    // Addr
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_Addr, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accAddr, itemHndl->data->nodeData);
    }

    // AddrType
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_AddrType, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accAddrType, itemHndl->data->nodeData);
    }

    // Port Number
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_PortNbr, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            accInfo->accPortNbr = atoi(itemHndl->data->nodeData);
    }

    // Auth Pref
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_AuthPref, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            accInfo->accAuthPref = dmTree_AuthType_stoe(
                    itemHndl->data->nodeData);
    }

    // Server AuthLevel
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerAuthLevel, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accServerAuthLevel, itemHndl->data->nodeData);
    }

    // Server AuthType
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerAuthType, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            accInfo->accServerAuthType = dmTree_AuthType_stoe(
                    itemHndl->data->nodeData);
    }

    // Server AuthName
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerAuthName, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accServerAuthName, itemHndl->data->nodeData);
    }

    // Server Psw
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerPw, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accServerPw, itemHndl->data->nodeData);
    }

    // Server Nonce
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ServerNonce, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy((IS8*) accInfo->accServerNonce, itemHndl->data->nodeData);
    }

    // Prev Server Nonce
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_PrevServerNonce, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy((IS8*) accInfo->accPrevServerNonce,
                    itemHndl->data->nodeData);
    }

    // Client AuthLevel
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ClientAuthLevel, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accClientAuthLevel, itemHndl->data->nodeData);
    }

    // Client AuthType
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ClientAuthType, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            accInfo->accClientAuthType = dmTree_AuthType_stoe(
                    itemHndl->data->nodeData);
    }

    // User Name
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_UserName, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accUserName, itemHndl->data->nodeData);
    }

    // User Psw
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_UserPw, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy(accInfo->accUserPw, itemHndl->data->nodeData);
    }

    // Client Nonce
    if (dmTreeParam_getDmAccNodeName(dmAccNodeName_ClientNonce, &itemUri,
            NULL ) == DMTREE_ERR_OK) {
        itemHndl = (DMTREE_INTERNAL_ITEM*) dmTree_searchItem(
                (DM_TREE_HANDLE) nodeHndl, itemUri);
        if (itemHndl != NULL && itemHndl->data->nodeData != NULL )
            strcpy((IS8*) accInfo->accClientNonce, itemHndl->data->nodeData);
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return DMTREE_ERR_OK;
}

/**
 *  @brief   This function delete the DM Account info and the related Con
 *           info from the tree using the serverId.
 *
 *  @param   serverID  [I]Server Id to find the corresponding DM Account.
 *
 *  @return  Returns DMTREE_ERR_OK if successful,
 *           Otherwise, return error code. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IS32 dmTreeDmAcc_deleteAccount(const IS8* serverID) {
    DM_TREE_HANDLE treeHndl = g_hTreeRootHndl;
    DMTREE_INTERNAL_ITEM* nodeHndl;

    /*
     *  Check param
     */
    if (treeHndl == NULL || serverID == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Invalidate param! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return DMTREE_ERR_CMD_FAILED;
    }

    /*
     *  Search server ID
     */
    if ((nodeHndl = dmTreeInternal_searchDMAcc((DMTREE_INTERNAL_ITEM*) treeHndl,
            serverID)) == NULL ) {
        dm_debug_trace(DM_DEBUG_TREE_MASK, "Search DMAcc path failed! \r\n");
        dmTree_setErrorCode(DMTREE_ERR_NOT_FOUND);
        return DMTREE_ERR_NOT_FOUND;
    }

    /*
     *  Delete server ID
     */
    g_dmTreeObjSave = TRUE;
    if (!dmTree_deleteItem((DM_TREE_HANDLE) nodeHndl)) {
        dm_debug_trace(DM_DEBUG_TREE_MASK,
                "DeleteAccount failed! delete node failed!\r\n");
        dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
        return DMTREE_ERR_CMD_FAILED;
    }

    dmTree_save();      // Jianghong Song for bug 9313 20061230

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return DMTREE_ERR_OK;
}

/**
 *  @brief   This function reads the tree from tree files and builds the
 *           whole tree.
 *
 *  @param   none.
 *
 *  @return  If successful, the return value is the handle to the new tree. \n
 *           Otherwise, the return value is NULL. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
DM_TREE_HANDLE dmTree_read(void) {
    DM_TREE_HANDLE treeHndl;
    DM_FILE_HANDLE fileHndl;
    IU32 size;
    IU8* buffer;

    LOGE("dm_tree, dmTree_read start");
    /*
     **  The tree have been opened?
     */
    if (g_hTreeRootHndl != NULL ) {
        return g_hTreeRootHndl;
    }

    /*
     *  Create standard tree
     */
    if ((treeHndl = dmTree_create()) == NULL ) {
        return NULL ;
    }
    LOGE("dm_tree, dmTree_read dmTree_create end");
    g_hTreeRootHndl = treeHndl;

    if ((buffer = (IU8*) malloc(sizeof(DMTREE_ACC_INFO))) != NULL ) {
        /*
         *  Read Dm obj item
         */
        if ((fileHndl = devFS_openFile(devFS_getObjTreeFileName(),
                DM_FOM_OPEN_FOR_READ)) != DM_INVALID_FILE_HANDLE ) {
            LOGE("dm_tree, dmTree_read devFS_openFile OK");
            while (1) {
                if (devFS_readFile(fileHndl, &size, sizeof(IU32))
                        != sizeof(IU32)) {
                    LOGE("dm_tree, dmTree_read devFS_readFile size: %d", size);
                    break;
                }
                if (devFS_readFile(fileHndl, buffer, size) != size) {
                    LOGE("dm_tree, dmTree_read devFS_readFile buffer: %s",
                            buffer);
                    break;
                }

                dmTreeInternal_readItem(treeHndl, buffer, size);
                LOGE(
                        "dm_tree, dmTree_read devFS_readFile dmTreeInternal_readItem end");
            }

            // close file
            devFS_closeFile(fileHndl);
        } else {
            LOGE("dm_tree, dmTree_read devFS_readFile devFS_openFile failed");
        }

        /*
         *  Read DMAcc item
         */
        if ((fileHndl = devFS_openFile(devFS_getAccTreeFileName(),
                DM_FOM_OPEN_FOR_READ)) != DM_INVALID_FILE_HANDLE ) {
            LOGE("dm_tree, dmTree_read devFS_openFile OK2");
            while (1) {
                if (devFS_readFile(fileHndl, &size, sizeof(IU32))
                        != sizeof(IU32))
                    break;
                if (devFS_readFile(fileHndl, buffer, size) != size)
                    break;

                dmTreeInternal_readDmAccItem(treeHndl, buffer, size);

            }

            // close file
            devFS_closeFile(fileHndl);
        } else {
            /*
             *  Create default DMAccount info
             */
            LOGE("dm_tree, dmTree_read devFS_readFile devFS_openFile failed2");
            const DMTREE_ACC_INFO* lpDmAccInfo;
            if ((lpDmAccInfo = dmTreeParam_getDmAccInfo()) != NULL ) {
                LOGE("dm_tree, dmTree_read dmTreeParam_getDmAccInfo ok");
                dmTreeDmAcc_addReplaceAccount(lpDmAccInfo);
                LOGE("dm_tree, dmTree_read dmTreeParam_getDmAccInfo ok1");
                dmTree_save();
                LOGE("dm_tree, dmTree_read dmTreeParam_getDmAccInfo ok2");
            }
        }

        // free buffer
        free(buffer);
    } else {
        /*
         **  Create default DMAccount info
         */
        const DMTREE_ACC_INFO* lpDmAccInfo;
        if ((lpDmAccInfo = dmTreeParam_getDmAccInfo()) != NULL ) {
            dmTreeDmAcc_addReplaceAccount(lpDmAccInfo);
            dmTree_save();
        }
    }

    g_dmTreeObjSave = FALSE;
    return treeHndl;
}

/**
 *  @brief   This function saves the management tree to the files.
 *
 *  @param   none.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTree_save(void) {
    DMTREE_INTERNAL_ITEM* lpTreeHndl = (DMTREE_INTERNAL_ITEM*) g_hTreeRootHndl;
    DM_FILE_HANDLE fileHndl;
    IS8* sSvrLst;

    if (lpTreeHndl == NULL ) {
        return FALSE;
    }

    if (g_dmTreeObjSave) {
        /*
         *  Save DMObj info
         */
        if ((fileHndl = devFS_openFile(devFS_getObjTreeFileName(),
                DM_FOM_OPEN_FOR_WRITE)) == DM_INVALID_FILE_HANDLE ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "Create Tree file failed! %s \r\n",
                    devFS_getObjTreeFileName());
            dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
            return FALSE;
        }
        // Traverse and Save tree item
        lpTreeHndl = lpTreeHndl->child->next;
        while (lpTreeHndl != NULL ) {
            if (!dmTree_preTraverseTree((DM_TREE_HANDLE) lpTreeHndl, NULL,
                    dmTreeInternalCB_saveItem, (IS32) fileHndl)) {
                break;
            }

            lpTreeHndl = lpTreeHndl->next;
        }
        // Close file
        devFS_closeFile(fileHndl);

        /*
         *  Save DMAcc info
         */
        if ((fileHndl = devFS_openFile(devFS_getAccTreeFileName(),
                DM_FOM_OPEN_FOR_WRITE)) == DM_INVALID_FILE_HANDLE ) {
            dm_debug_trace(DM_DEBUG_TREE_MASK,
                    "Create DMAcc file failed! %s \r\n",
                    devFS_getAccTreeFileName());
            dmTree_setErrorCode(DMTREE_ERR_CMD_FAILED);
            return FALSE;
        }
        // Get Server List
        if ((sSvrLst = dmTreeDmAcc_getServerList()) != NULL ) {
            DMTREE_ACC_INFO* dmAccInfo;
            IS8* svr = sSvrLst;

            // malloc dmAcc info
            if ((dmAccInfo = (DMTREE_ACC_INFO*) malloc(sizeof(DMTREE_ACC_INFO)))
                    != NULL ) {
                while (*svr != '\0') {
                    // Get DmAcc infor
                    if (dmTreeDmAcc_getAccountInfo(svr,
                            dmAccInfo) == DMTREE_ERR_OK) {
                        // Save to file
                        dmTreeInternal_saveDmAccItem(fileHndl, dmAccInfo);
                    }
                    svr += (strlen(svr) + 1);
                }
                // free dmACCInfo
                free(dmAccInfo);
            }
            // free buffer
            free(sSvrLst);
        }
        // Close File
        devFS_closeFile(fileHndl);
    }

    dmTree_setErrorCode(DMTREE_ERR_OK);
    return TRUE;
}

/**
 *  @brief   This function print the tree.
 *
 *  @param   none.
 *
 *  @return  Returns TRUE if successful, or FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTree_print(void) {
    return dmTree_preTraverseTree(g_hTreeRootHndl, NULL,
            dmTreeInternalCB_printItem, 0);
}

/**
 *  @brief   The function retrieves the last-error code value.
 *
 *  @param   none.
 *
 *  @return  The return value is the last-error code value.
 *
 *  @note    none
 */
IU32 dmTree_getErrorCode(void) {
    return g_nErrorCode;
}

/**
 *  @brief   The function set the last-error code value.
 *
 *  @param   The error code.
 *
 *  @return  none.
 *
 *  @note    none
 */
void dmTree_setErrorCode(IU32 errorCode) {
    g_nErrorCode = errorCode;
}

