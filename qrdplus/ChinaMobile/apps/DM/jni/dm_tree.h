#ifndef _DM_TREE_H_
#define _DM_TREE_H_

#include "vdm_pl_types.h"
#include "dm_pl_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  DMTREE traverse callback function define
 */
typedef IBOOL (*DMTREE_TRAVERSEFUNC)(DM_TREE_HANDLE treeHndl,
        DMTREE_ITEM* treeItem, IS32 param);

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
DM_TREE_HANDLE dmTree_create(void);

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
DM_TREE_HANDLE dmTree_searchItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri);

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
        const DMTREE_ITEM* newItem);

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
        const DMTREE_ITEM* replaceItem);

/**
 *  @brief   Retrieves all of a tree item's attributes.
 *
 *  @param   treeHndl    [I]Handle to the parent item. NULL handle is Root Handle.
 *  @param   nodeUri     [I]The uri to get. This param can be NULL.
 *  @param   getItem     [O]Address of a DMTREE_ITEM structure that specifies the attributes of
 *                       the new tree item.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    The return data-pointer can only be read,you cann't release it or modify it.
 */
IBOOL dmTree_getItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri,
        DMTREE_ITEM* getItem);

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
const DMTREE_ITEM* dmTree_getItemEx(DM_TREE_HANDLE treeHndl,
        const IS8* nodeUri);

/**
 *  @brief   Removes an item and all its children from the DM tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *                     If treeHndl is set to NULL, all items are deleted.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTree_deleteItem(DM_TREE_HANDLE treeHndl);

/**
 *  @brief   Free an item and all its children from the DM tree.
 *
 *  @param   treeHndl  [I]DM_TREE_HANDLE handle to the item to delete.
 *                     If treeHndl is set to NULL, all items are deleted.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    Once an item is deleted, its handle is invalid and cannot be used.
 */
IBOOL dmTree_freeItem(DM_TREE_HANDLE treeHndl);

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
 *  @return  If the function traverse the tree over, the return value is BF_TRUE. \n
 *           Otherwise, the return value is BF_FALSE. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    If you want ternimated during traversing Tree,you should return BF_FALSE in the callback function.
 *           If startHndl is not NULL, the searching will start from the position stopped last time.
 */
IBOOL dmTree_preTraverseTree(DM_TREE_HANDLE treeHndl, DM_TREE_HANDLE startHndl,
        DMTREE_TRAVERSEFUNC func, IS32 param);

/**
 *  @brief   This function traverses the tree in post-order and do the
 *           operation (callback function pointer) on the traversed nodes. \n
 *           Pre-order: Parent is processed prior to its children nodes.
 *
 *  @param   treeHndl  [I]Pointer to the starting node of the sub tree.
 *  @param   startHndl [I]Start pointer handle. It can been NULL. this handle max is treeHandl's child node.
 *  @param   func      [I]Callback function pointer for the operation on the node.
 *  @param   param     [I]Specifies an user defined value passed to the callback function.
 *
 *  @return  If the function traverse the tree over, the return value is BF_TRUE. \n
 *           Otherwise, the return value is BF_FALSE. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    If you want ternimated during traversing Tree,you should return BF_FALSE in the callback function.
 *           If startHndl is not NULL, the searching will start from the position stopped last time.
 */
IBOOL dmTree_postTraverseTree(DM_TREE_HANDLE treeHndl, DM_TREE_HANDLE startHndl,
        DMTREE_TRAVERSEFUNC func, IS32 param);

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
DM_TREE_HANDLE dmTree_getParent(DM_TREE_HANDLE treeHndl);

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
DM_TREE_HANDLE dmTree_getChild(DM_TREE_HANDLE treeHndl);

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
DM_TREE_HANDLE dmTree_getPrevious(DM_TREE_HANDLE treeHndl);

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
DM_TREE_HANDLE dmTree_getNext(DM_TREE_HANDLE treeHndl);

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
        IU32 aclType);

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
        const DMTREE_ITEM* newItem, const IS8* serverID);

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
        const DMTREE_ITEM* newItem, const IS8* serverID);

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
        const IS8* serverID);

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
 *               DMTREE_ERR_NOT_FOUND
 *               DMTREE_ERR_NOT_ALLOWED
 *
 *  @note    none.
 */
DM_TREE_HANDLE dmTreeSml_getItem(DM_TREE_HANDLE treeHndl, const IS8* nodeUri,
        const IS8* serverID);

/**
 *  @brief   This function sets some or all of a tree item's attributes. check ACL auth.
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
        const DMTREE_ITEM* replaceItem, const IS8* serverID);

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
        const IS8* nodeUri, IS32 param, const IS8* serverID);

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
        IS32 callType);

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
DM_TREE_HANDLE dmTreeDmAcc_create(const IS8* nodeName, const IS8* serverID);

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
IS8* dmTreeDmAcc_getServerList(void);

/**
 *  @brief   This function add or replace the DM Account info and the related Con
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
IS32 dmTreeDmAcc_addReplaceAccount(const DMTREE_ACC_INFO* accInfo);

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
IS32 dmTreeDmAcc_getAccountInfo(const IS8* serverID, DMTREE_ACC_INFO* accInfo);

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
IS32 dmTreeDmAcc_deleteAccount(const IS8* serverID);

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
DM_TREE_HANDLE dmTree_read(void);

/**
 *  @brief   This function saves the management tree to the files.
 *
 *  @param   none.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTree_save(void);

/**
 *  @brief   This function print the tree.
 *
 *  @param   none.
 *
 *  @return  Returns BF_TRUE if successful, or BF_FALSE otherwise. \n
 *           To get extended error information, call dmTree_getErrorCode().
 *
 *  @note    none
 */
IBOOL dmTree_print(void);

/**
 *  @brief   The function retrieves the last-error code value.
 *
 *  @param   none.
 *
 *  @return  The return value is the last-error code value.
 *
 *  @note    none
 */
IU32 dmTree_getErrorCode(void);

/**
 *  @brief   The function set the last-error code value.
 *
 *  @param   The error code.
 *
 *  @return  none.
 *
 *  @note    none
 */
void dmTree_setErrorCode(IU32 errorCode);

#ifdef __cplusplus
}
#endif

#endif
