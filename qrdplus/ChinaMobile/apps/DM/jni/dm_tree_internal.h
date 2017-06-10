#ifndef _DM_TREE_INTERNAL_H_
#define _DM_TREE_INTERNAL_H_

#include "dm_tree.h"
#include "dm_pl_fs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM_TREE_URI_MAX_DEPTH       7
#define DM_TREE_MAX_URI_SIZE        128
#define DM_TREE_MAX_NAME_STR        64

#define DM_MAKEULONG(h,l)        (unsigned long)(((unsigned short)(l)) | ((unsigned long)(h)<<16))
#define DM_MAKEWORD(h,l)         (unsigned short)(((unsigned char)(l)) | ((unsigned short)(h)<<8))
#define DM_HIWORD(d)             (unsigned short)(((unsigned long)(d) >> 16) & 0xFFFF)
#define DM_LOWORD(d)             (unsigned short)((unsigned long)(d) & 0xFFFF)

/*
 *  DMTREE replace type define
 */
#define DMTREE_REPLACE_NEWADD               0x1000

/*
 *  DMTREE Item
 */
typedef struct _DMTREE_INTERNAL_ITEM {
    struct _DMTREE_INTERNAL_ITEM* parent;
    struct _DMTREE_INTERNAL_ITEM* child;
    struct _DMTREE_INTERNAL_ITEM* prev;
    struct _DMTREE_INTERNAL_ITEM* next;

    DMTREE_ITEM* data;
    IU32 modMask;

} DMTREE_INTERNAL_ITEM;

void dmTreeFreeAclServerList(DMTREE_ACLSVRLIST *lpAclList);
DMTREE_ACLSVRLIST* dmTreeCopyAclServerList(DMTREE_ACLSVRLIST *lpAclList);
IBOOL dmTreeCheckDataFormat(const IS8* nodeFormat, const IS8* nodeData,
        IS32 dataSize);
void dmTreeDeleteItemData(DMTREE_INTERNAL_ITEM* lpItem);

/**
 *  @brief  Auth type change.
 *
 *  @param  none.
 *
 *  @return none.
 *
 *  @note   none.
 */
const IS8* dmTree_AuthType_etos(DMTREE_AUTH_TYPE eAuthType);
DMTREE_AUTH_TYPE dmTree_AuthType_stoe(const IS8* sAuthType);

/**
 *  @brief  This function checks a URI string for validity.
 *
 *  @param  nodeUri  [I]URI to be checked.
 *
 *  @return DMTREE_ERR_OK or Error code;
 *
 *  @note   none.
 */
IU32 dmTreeInternal_checkValidUri(const IS8 *nodeUri);

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
 *               DMTREE_ERR_NOMEMORY
 *
 *  @note    This function only add a new child-node under parent-node.
 *
 */
DMTREE_INTERNAL_ITEM* dmTreeInternal_insertItem(
        DMTREE_INTERNAL_ITEM* parentHndl, const DMTREE_ITEM* newItem);

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
 *               DMTREE_ERR_NOT_SUPPORTED
 *               DMTREE_ERR_NOMEMORY
 *               DMTREE_ERR_ALREADY_EXIST
 *
 *  @note    The treeHndl identifies the item, and the replaceMask parameters specifies which attributes to set.
 */
IS32 dmTreeInternal_replaceItem(DMTREE_INTERNAL_ITEM* replaceHndl,
        IU32 replaceMask, const DMTREE_ITEM* replaceItem);

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
        const IS8* aclBuf);

/**
 *  @brief   Removes an item and all its children from the DM tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTreeInternalCB_deleteItem(DM_TREE_HANDLE treeHndl,
        DMTREE_ITEM* treeItem, IS32 param);

/**
 *  @brief   Free the DM Tree resource;
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTreeInternalCB_freeItem(DM_TREE_HANDLE treeHndl, DMTREE_ITEM* treeItem,
        IS32 param);

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
 *               DMTREE_ERR_NOMEMORY
 *               DMTREE_ERR_CMD_FAILED
 *
 *  @note    none
 */
DMTREE_INTERNAL_ITEM* dmTreeInternal_readItem(DM_TREE_HANDLE treeHndl,
        IU8* buffer, IU32 size);

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
        IU32 size);

/**
 *  @brief   This function saves the management tree item to the files.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to save.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param. The file handle.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTreeInternalCB_saveItem(DM_TREE_HANDLE treeHndl, DMTREE_ITEM* treeItem,
        IS32 param);

/**
 *  @brief   This function print the management tree item.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to save.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param. The file handle.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTreeInternalCB_printItem(DM_TREE_HANDLE treeHndl, DMTREE_ITEM* treeItem,
        IS32 param);

/**
 *  @brief   This function saves the DMAcc info.
 *
 *  @param   fileHndl  [I]file handle to the item to save.
 *  @param   dmAccInfo [I]The DmAcc info.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTreeInternal_saveDmAccItem(DM_FILE_HANDLE fileHndl,
        DMTREE_ACC_INFO* dmAccInfo);

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
        const IS8* nodeName, const IS8* serverID);

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
        const IS8* serverID);

/**
 *  @brief   Set DMAcc server list ServerID value.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *  @param   treeItem  [I]The tree item pointer.
 *  @param   param     [I]The application param. serverID pointer.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTreeInternalCB_replaceDMAccServerList(DM_TREE_HANDLE treeHndl,
        DMTREE_ITEM* treeItem, IS32 param);

#ifdef __cplusplus
}
#endif

#endif // _DMTREE_INTERNAL_H
