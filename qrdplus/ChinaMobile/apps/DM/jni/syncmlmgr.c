#ifdef WIN32
#include "std_header.h"
#endif
#define _SYNCMLMGR_C_

#include "libmem.h"
#include "base64.h"
#include "md5.h"
#include "smlerr.h"
#include "dm_task.h"

extern task_relay_info_type* dm_task_relay_info;
extern status_cmd_queue_type* dm_status_cmd_que;
extern results_cmd_queue_type* dm_results_cmd_que;

/*******************************************************************
 **FUNC : dm_calc_b64_cred
 **Description : count the cred that used by b64 authentication
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
void dm_calc_b64_cred(char* creddata, unsigned long cbLength) {
    /**************************************/
    /* Basic authentication:              */
    /* calculate a base64-encoding of the */
    /* string <user>:<password>           */
    /**************************************/

    char abSave[4];
    int cbCopied = 0;
    unsigned long cbDigestSize = 80;
    unsigned long cbOffset = 0;
    unsigned char* outputdata;
    abSave[0] = abSave[1] = abSave[2] = abSave[3] = '\0';
    if (0 == cbLength) {
        creddata[0] = '\0';
        return;
    }
    outputdata = xppMalloc(cbLength * 2 + 4);
    if(outputdata == NULL)
       return;
    cbCopied = dm_base64Encode((unsigned char*) outputdata, cbDigestSize,
            (unsigned char*) creddata, &cbLength, &cbOffset, 1,
            (unsigned char *) abSave);
    outputdata[cbCopied] = '\0';
    xppMemcpy(creddata, outputdata, cbCopied);
    creddata[cbCopied] = '\0';
    xppFree(outputdata);
}

/*******************************************************************
 **FUNC : dm_calc_md5_cred
 **Description : count the cred that used by md5 authenticationb
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
char* dm_calc_md5_cred(char* creddata) {
    /*******************************************/
    /* Digest authentication:                  */
    /* compute the digest according to RFC2617 */
    /*******************************************/
    MD5_CTX Md5Ctx;
    HASH HA1;
    //int i = 0;
    DM_MD5Init(&Md5Ctx);
    DM_MD5Update(&Md5Ctx, creddata, xppStrlen(creddata));
    DM_MD5Final(HA1, &Md5Ctx);
    xppMemcpy(creddata, HA1, HASHLEN);
    creddata[HASHLEN] = 0;
    return creddata;
}

/*******************************************************************
 **FUNC : dm_syncml_init
 **Description : sync init
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_init() {
    SmlOptions_t options;

    /* Set Toolkit options structure */
    options.defaultPrintFunc = NULL;
    options.maxWorkspaceAvailMem = MAX_WSM_BUFFER_SIZE;

    /* Initialize SyncML Toolkit */
    return dm_smlInit(&options);
}

/*******************************************************************
 **FUNC : dm_syncml_initance
 **Description : sync init workspace
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_initance(short * pID, char* workspacename) {
    SmlCallbacks_t callbacks;
    SmlInstanceOptions_t options;

    /* Set instance */
    options.encoding = dm_task_relay_info->encodetype;
    options.workspaceName = workspacename;
    options.workspaceSize = ONE_WORK_SPACE_SIZE;

    /* Allocate for callbacks structure */
    callbacks.startMessageFunc = dm_myHandleStartMessage;
    callbacks.endMessageFunc = dm_myHandleEndMessage;

    callbacks.startSyncFunc = dm_myHandleStartSync;
    callbacks.endSyncFunc = dm_myHandleEndSync;

#ifdef ATOMIC_RECEIVE
    callbacks.startAtomicFunc = dm_myHandlestartAtomic;
    callbacks.endAtomicFunc = dm_myHandleendAtomic;
#else
    callbacks.startAtomicFunc = NULL;
    callbacks.endAtomicFunc = NULL;
#endif

#ifdef SEQUENCE_RECEIVE
    callbacks.startSequenceFunc = dm_myHandlestartSequence;
    callbacks.endSequenceFunc = dm_myHandleendSequence;
#else
    callbacks.startSequenceFunc = NULL;
    callbacks.endSequenceFunc = NULL;
#endif

    callbacks.addCmdFunc = dm_myHandleAdd;
    callbacks.alertCmdFunc = dm_myHandlealert;
    callbacks.deleteCmdFunc = dm_myHandledelete;
    callbacks.getCmdFunc = dm_myHandleget;
    callbacks.putCmdFunc = dm_myHandleput;

#ifdef MAP_RECEIVE
    callbacks.mapCmdFunc = dm_myHandlemap;
#else
    callbacks.mapCmdFunc = NULL;
#endif

#ifdef RESULT_RECEIVE
    callbacks.resultsCmdFunc = dm_myHandleresults;
#else
    callbacks.resultsCmdFunc = NULL;
#endif

    callbacks.statusCmdFunc = dm_myHandlestatus;
    callbacks.replaceCmdFunc = dm_myHandlereplace;

#ifdef COPY_RECEIVE
    callbacks.copyCmdFunc = dm_myHandlecopy;
#else
    callbacks.copyCmdFunc = NULL;
#endif

#ifdef EXEC_RECEIVE
    callbacks.execCmdFunc = dm_myHandleexec;
#else
    callbacks.execCmdFunc = NULL;
#endif

#ifdef SEARCH_RECEIVE
    callbacks.searchCmdFunc = dm_myHandlesearch;
#else
    callbacks.searchCmdFunc = NULL;
#endif

    callbacks.FinalFunc = dm_myHandleFinal;

    callbacks.handleErrorFunc = dm_myHandlehandleError;
    callbacks.transmitChunkFunc = dm_myHandletransmitChunk;

    return dm_smlInitInstance(&callbacks, &options, NULL, pID);
}

/*******************************************************************
 **FUNC : dm_syncml_startmessage
 **Description : found sync header
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_startmessage(short id) {
    SmlSyncHdr_t hdr;
    SmlSource_t source;
    SmlTarget_t target;
    SmlCred_t cred;
    SmlMetInfMetInf_t metainfo;
    SmlMetInfMetInf_t metainfo2;
    char creddata[80];
    char* sessionid;
    char* messageid;
    short returnval = SML_ERR_OK;
    char * max_msg_size = NULL;
    char * max_obj_size = NULL;

    syncml_core_message("MMIDM $$ dm_syncml_startmessage  anth_type=%d ",dm_task_relay_info->authtype);
    dm_smlLibMemset(&metainfo, 0, sizeof(metainfo));
    dm_smlLibMemset(&metainfo2, 0, sizeof(metainfo2));
    dm_smlLibMemset(&cred, 0, sizeof(cred));

    /* Create SyncML proto element for message header */
    hdr.elementType = SML_PE_HEADER;
    hdr.version = dm_smlString2Pcdata(__FILE__, __LINE__, "1.2");
    hdr.proto = dm_smlString2Pcdata(__FILE__, __LINE__, "DM/1.2");

    sessionid = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->sessionID);
    messageid = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->messageID);
    hdr.sessionID = dm_smlString2Pcdata(__FILE__, __LINE__, sessionid);
    hdr.msgID = dm_smlString2Pcdata(__FILE__, __LINE__, messageid);
    hdr.flags = 0;

#if 1
    target.locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            dm_task_relay_info->syncml_connect_addr);
#else
    target.locURI = dm_smlString2Pcdata(__FILE__, __LINE__,"http://218.206.176.97:7001");
#endif
    target.locName = NULL;
    hdr.target = &target;

#if 1
    source.locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            dm_task_relay_info->syncml_dm_client_imei);
    source.locName = dm_smlString2Pcdata(__FILE__, __LINE__,
            dm_task_relay_info->syncml_dm_username);
#else
    source.locURI = dm_smlString2Pcdata(__FILE__, __LINE__, "IMEI:860206000003973");
    source.locName=dm_smlString2Pcdata(__FILE__, __LINE__, "mvpdm");
#endif
    hdr.source = &source;
    hdr.respURI = NULL;
    syncml_core_message("MMIDM $$ dm_syncml_startmessage  authtype=%d!",dm_task_relay_info->authtype);
    if ((dm_task_relay_info->authtype == AUTH_B64)
            && (dm_task_relay_info->authored == 0)
            && (0
                    != (xppStrlen(dm_task_relay_info->syncml_dm_username)
                            + xppStrlen(dm_task_relay_info->syncml_dm_password)))) {
        metainfo.type = dm_smlString2Pcdata(__FILE__, __LINE__,
                "syncml:auth-basic");
        if (0 != xppStrlen(dm_task_relay_info->syncml_dm_password))
            sprintf(creddata, "%s:%s", dm_task_relay_info->syncml_dm_username,
                    dm_task_relay_info->syncml_dm_password);
        else
            sprintf(creddata, "%s", dm_task_relay_info->syncml_dm_username);
        dm_calc_b64_cred(creddata, xppStrlen(creddata));
        syncml_core_message("MMIDM $$ dm_syncml_startmessage  creddata=%s ",creddata);
        cred.data = dm_smlString2Pcdata(__FILE__, __LINE__, creddata);
        cred.meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, &metainfo);
        hdr.cred = &cred;
    } else if ((dm_task_relay_info->authtype == AUTH_MD5)
            && (dm_task_relay_info->authored == 0)) {
        char nonce_tem[30] = { 0 };
        int nonce_len = 0;
        metainfo.type = dm_smlString2Pcdata(__FILE__, __LINE__,
                "syncml:auth-md5");
        metainfo.format = dm_smlString2Pcdata(__FILE__, __LINE__, "b64");
#if 0
        sprintf(creddata, "%s", "Bruce2:OhBehave");
#else
        syncml_core_message("MMIDM $$ dm_syncml_startmessage  username=%s, password=%s",
                dm_task_relay_info->syncml_dm_username,dm_task_relay_info->syncml_dm_password);

        sprintf(creddata, "%s", dm_task_relay_info->syncml_dm_username);
        xppStrcat(creddata, ":");
        xppStrcat(creddata, dm_task_relay_info->syncml_dm_password);
#endif
        dm_calc_b64_cred(dm_calc_md5_cred(creddata), HASHLEN);
        xppStrcat(creddata, ":");
        syncml_core_message("MMIDM $$ dm_syncml_startmessage  server_nextnonce=%s ",dm_task_relay_info->server_nextnonce);
        nonce_len = strlen(dm_task_relay_info->server_nextnonce);
        dm_base64Decode(nonce_tem, 30, dm_task_relay_info->server_nextnonce,
                &nonce_len);
        syncml_core_message("MMIDM $$ dm_syncml_startmessage  dec_nextnonce=%s ",nonce_tem);
        xppStrcat(creddata, nonce_tem);
        dm_calc_b64_cred(dm_calc_md5_cred(creddata), HASHLEN);
        syncml_core_message("MMIDM $$ dm_syncml_startmessage  creddata=%s ",creddata);

        cred.data = dm_smlString2Pcdata(__FILE__, __LINE__, creddata);
        cred.meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, &metainfo);
        hdr.cred = &cred;
    } else {
        syncml_core_message("MMIDM $$ dm_syncml_startmessage  authtype is error!");
        hdr.cred = NULL;
    }

#if 1
    max_msg_size = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->maxmsg_size);
#else
    max_msg_size=dm_smlunsignedInt2String(__FILE__, __LINE__, "9000");
#endif
    metainfo2.maxmsgsize = dm_smlString2Pcdata(__FILE__, __LINE__,
            max_msg_size);

#if 1
    max_obj_size = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->maxobj_size);
#else
    max_obj_size=dm_smlunsignedInt2String(__FILE__, __LINE__, "524288");
#endif
    metainfo2.maxobjsize = dm_smlString2Pcdata(__FILE__, __LINE__,
            max_obj_size);

    hdr.meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, &metainfo2);
    returnval = dm_smlStartMessageExt(id, &hdr, SML_VERS_1_2);

    dm_smlFreePcdata(__FILE__, __LINE__, hdr.version);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.proto);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.sessionID);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.msgID);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.respURI);
    dm_smlLibFree(hdr.meta);
    dm_smlFreePcdata(__FILE__, __LINE__, metainfo2.maxmsgsize);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.source->locURI);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.source->locName);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.target->locURI);
    dm_smlFreePcdata(__FILE__, __LINE__, hdr.target->locName);
    if (NULL != hdr.cred) {
        dm_smlLibFree((hdr.cred->meta));
        dm_smlFreePcdata(__FILE__, __LINE__, metainfo.type);
        dm_smlFreePcdata(__FILE__, __LINE__, metainfo.format);
        dm_smlFreePcdata(__FILE__, __LINE__, hdr.cred->data);
    }
    dm_smlLibFree(sessionid);
    dm_smlLibFree(messageid);
    dm_smlLibFree(max_msg_size);
    dm_smlLibFree(max_obj_size);

    return returnval;
}

/*******************************************************************
 **FUNC : dm_syncml_AlertCmd
 **Description : add sync order ALERT
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_AlertCmd(short id, char* cmd) {
    SmlAlert_t alert;
    char * temp = NULL;
    short returnval = SML_ERR_OK;

    dm_smlLibMemset(&alert, 0, sizeof(alert));

    /*Alert command*/
    temp = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->cmdID);
    alert.cmdID = dm_smlString2Pcdata(__FILE__, __LINE__, temp);
    dm_smlLibFree(temp);
    dm_task_relay_info->cmdID++;
    alert.data = dm_smlString2Pcdata(__FILE__, __LINE__, cmd);

    returnval = dm_smlAlertCmd(id, &alert);

    dm_smlFreePcdata(__FILE__, __LINE__, alert.cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, alert.data);
    return returnval;
}

/*******************************************************************
 **FUNC : dm_syncml_PutCmd
 **Description : add sync order PUT
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_PutCmd(short id) {
    return SML_ERR_OK;
}

/*******************************************************************
 **FUNC : dm_syncml_GetCmd
 **Description : add sync order GET
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_GetCmd(short id) {
    char* temp;
    SmlGet_t get;
    SmlMetInfMetInf_t metainfo;
    SmlItemList_t itemlist;
    SmlItem_t item;
    SmlTarget_t target;
    short returnval = SML_ERR_OK;

    dm_smlLibMemset(&get, 0, sizeof(get));
    dm_smlLibMemset(&metainfo, 0, sizeof(metainfo));
    dm_smlLibMemset(&item, 0, sizeof(item));
    dm_smlLibMemset(&target, 0, sizeof(target));

    temp = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->cmdID);
    get.cmdID = dm_smlString2Pcdata(__FILE__, __LINE__, temp);
    dm_smlLibFree(temp);
    dm_task_relay_info->cmdID++;
    if (dm_task_relay_info->encodetype == SML_XML) {
        metainfo.type = dm_smlString2Pcdata(__FILE__, __LINE__,
                "application/vnd.syncml-devinf+xml");
    } else if (dm_task_relay_info->encodetype == SML_WBXML) {
        metainfo.type = dm_smlString2Pcdata(__FILE__, __LINE__,
                "application/vnd.syncml-devinf+wbxml");
    }
    get.meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, &metainfo);

    itemlist.next = NULL;
    itemlist.item = &item;
    itemlist.item->target = &target;
    itemlist.item->target->locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            "./devinf11");

    get.itemList = &itemlist;

    returnval = dm_smlGetCmd(id, &get);

    dm_smlFreePcdata(__FILE__, __LINE__, get.cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, metainfo.type);
    dm_smlLibFree(get.meta);
    dm_smlFreePcdata(__FILE__, __LINE__, itemlist.item->target->locURI);
    return returnval;
}

/*******************************************************************
 * function     : dm_Syncml_GetAddItems
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/14/2011
 *******************************************************************/
SmlItemListPtr_t dm_Syncml_GetAddItems(void) {
    SmlItemListPtr_t itemList = NULL;
    return itemList;

}

/*******************************************************************
 **FUNC : dm_syncml_AddCmd
 **Description : add sync order ADD
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/

short dm_syncml_AddCmd(short id) {
    char* temp;
    SmlAdd_t add;
    SmlMetInfMetInf_t meta;
    short returnval = SML_ERR_OK;
    SmlItemPtr_t item = NULL;
    SmlItemListPtr_t itemList = NULL;
    SmlSourcePtr_t source = NULL;

    add.itemList = NULL;

    add.itemList = dm_Syncml_GetAddItems();

    if (NULL == add.itemList) /*there is no items need to be added.*/
    {
        return returnval;
    }

    /* Add cmd */
    add.elementType = SML_PE_ADD;
    temp = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->cmdID);
    dm_task_relay_info->cmdID++;
    add.cmdID = dm_smlString2Pcdata(__FILE__, __LINE__, temp);
    add.flags = 0;
    add.cred = NULL;
    dm_smlLibMemset(&meta, 0, (long) sizeof(SmlMetInfMetInf_t));
    meta.type = dm_smlString2Pcdata(__FILE__, __LINE__, "text/x-vcard");
    add.meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, &meta);

    returnval = dm_smlAddCmd(id, &add);

    //free drynamic memorys.
    while (add.itemList != NULL ) {
        itemList = add.itemList;
        add.itemList = itemList->next;
        item = itemList->item;
        source = item->source;

        if (NULL != item) {
            dm_smlFreePcdata(__FILE__, __LINE__, item->meta);
            dm_smlFreePcdata(__FILE__, __LINE__, item->data);
        }
        if (NULL != source) {
            dm_smlFreePcdata(__FILE__, __LINE__, source->locName);
            dm_smlFreePcdata(__FILE__, __LINE__, source->locURI);
        }

        dm_smlLibFree(source);
        source = NULL;
        dm_smlLibFree(item);
        item = NULL;
        dm_smlLibFree(itemList);
        itemList = NULL;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, add.cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, meta.type);
    dm_smlLibFree(add.meta);
    dm_smlLibFree(temp);

    return returnval;

}

/*******************************************************************
 * function     : dm_Syncml_GetDeleteItems
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/14/2011
 *******************************************************************/
SmlItemListPtr_t dm_Syncml_GetDeleteItems(void) {
    SmlItemListPtr_t itemList = NULL;
    return itemList;

}

/*******************************************************************
 **FUNC : dm_syncml_DeleteCmd
 **Description : add sync order DELETE
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_DeleteCmd(short id) {
    char* temp;
    SmlDelete_t del;
    short returnval = SML_ERR_OK;
    SmlItemListPtr_t itemList = NULL;
    SmlItemPtr_t item = NULL;
    SmlSourcePtr_t source = NULL;

    del.itemList = NULL;
    syncml_core_message("MMIDM  $$enter dm_syncml_DeleteCmd $$");

    del.itemList = dm_Syncml_GetDeleteItems();

    syncml_core_message("MMIDM  $$ del.itemList=%d",del.itemList);
    if (NULL == del.itemList) /*there is no items to be deleted*/
    {
        return returnval;
    }

    /* Delete cmd */
    del.elementType = SML_PE_DELETE;
    temp = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->cmdID);
    dm_task_relay_info->cmdID++;
    del.cmdID = dm_smlString2Pcdata(__FILE__, __LINE__, temp);
    del.flags = 0;
    del.cred = NULL;
    del.meta = NULL;

    returnval = dm_smlDeleteCmd(id, &del);

    //free drynamic memorys.
    while (del.itemList != NULL ) {
        itemList = del.itemList;
        del.itemList = itemList->next;
        item = itemList->item;
        source = item->source;
        if (NULL != item) {
            dm_smlFreePcdata(__FILE__, __LINE__, item->meta);
            dm_smlFreePcdata(__FILE__, __LINE__, item->data);
        }
        if (NULL != source) {
            dm_smlFreePcdata(__FILE__, __LINE__, source->locName);
            dm_smlFreePcdata(__FILE__, __LINE__, source->locURI);
        }
        dm_smlLibFree(source);
        source = NULL;
        dm_smlLibFree(item);
        item = NULL;
        dm_smlLibFree(itemList);
        itemList = NULL;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, del.cmdID);
    dm_smlLibFree(temp);
    return returnval;
}

/*******************************************************************
 **FUNC : dm_syncml_StatusCmd
 **Description : add sync order Status
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_StatusCmd(short id) {
    char* temp;
    short rc = SML_ERR_OK;
    long counter;
    status_element_type* queueptr;
    if (NULL == dm_status_cmd_que->queue) {
        return rc;
    }

    queueptr = dm_status_cmd_que->queue;
    for (counter = 0; counter < dm_status_cmd_que->totalnumber; counter++) {
        temp = dm_smlunsignedInt2String(__FILE__, __LINE__,
                dm_task_relay_info->cmdID);

        queueptr->status->cmdID = dm_smlString2Pcdata(__FILE__, __LINE__, temp);
        dm_task_relay_info->cmdID++;
        rc |= dm_smlStatusCmd(id, queueptr->status);
        queueptr = queueptr->next;
        dm_smlLibFree(temp);
    }
    rc |= dm_myFreestatusofCQ();
    return rc;
}

/*==========================================================
 * function     : dm_syncml_ResultsCmd
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/14/2011
 ==========================================================*/
short dm_syncml_ResultsCmd(short id) {
    char* temp;
    short rc = SML_ERR_OK;
    long counter;
    results_element_type* queueptr;
    syncml_core_message("MMIDM  $$enter dm_syncml_ResultsCmd $$");
    if (NULL == dm_results_cmd_que->queue) {
        return rc;
    }
    queueptr = dm_results_cmd_que->queue;
    for (counter = 0; counter < dm_results_cmd_que->totalnumber; counter++) {
        temp = dm_smlunsignedInt2String(__FILE__, __LINE__,
                dm_task_relay_info->cmdID);

        queueptr->results->cmdID = dm_smlString2Pcdata(__FILE__, __LINE__,
                temp);
        dm_task_relay_info->cmdID++;
        rc |= dm_smlResultsCmd(id, queueptr->results);
        queueptr = queueptr->next;
        dm_smlLibFree(temp);
    }
    rc |= dm_myFreeResultsofCQ();
    syncml_core_message("MMIDM  $$ return dm_syncml_ResultsCmd $$");
    return rc;
}

/*==========================================================
 * function     : dm_Syncml_GettReplacedItems
 * decr     :
 * parameter    :
 * return       :
 * effect       : NO
 * history  :created  3/10/2011
 ==========================================================*/
SmlItemListPtr_t dm_Syncml_GetReplacedItems(void) {
    SmlItemListPtr_t itemList = NULL;

    SmlItemListPtr_t tmptr = NULL;

    SmlSourcePtr_t source_mod = NULL;
    SmlSourcePtr_t source_man = NULL;
    SmlSourcePtr_t source_lang = NULL;
    SmlSourcePtr_t source_dmv = NULL;
    SmlSourcePtr_t source_devid = NULL;
    SmlMetInfMetInfPtr_t meta_devid = NULL;

    SmlItemPtr_t item_mod = NULL;
    SmlItemPtr_t item_man = NULL;
    SmlItemPtr_t item_lang = NULL;
    SmlItemPtr_t item_dmv = NULL;
    SmlItemPtr_t item_devid = NULL;

    char * data = NULL;

    //DevId
    itemList = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItemList_t));
    if(itemList == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(itemList, 0, (long) sizeof(SmlItemList_t));

    source_devid = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlSource_t));
    if(source_devid == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(source_devid, 0, (long) sizeof(SmlSource_t));
    source_devid->locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            "./DevInfo/DevId");

    meta_devid = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlMetInfMetInf_t));
    if(meta_devid == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(meta_devid, 0, sizeof(SmlMetInfMetInf_t));
    meta_devid->format = dm_smlString2Pcdata(__FILE__, __LINE__, "chr");

    item_devid = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItem_t));
    if(item_devid == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(item_devid, 0, (long) sizeof(SmlItem_t));
    item_devid->source = source_devid;
    item_devid->meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, meta_devid);
#if 1
    item_devid->data = dm_smlString2Pcdata(__FILE__, __LINE__,
            dm_task_relay_info->syncml_dm_client_imei);
#else
    item_devid->data=dm_smlString2Pcdata(__FILE__, __LINE__, "IMEI:860206000003973");
#endif

    itemList->item = item_devid;
    itemList->next = tmptr;
    tmptr = itemList;

    //DmV
    itemList = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItemList_t));
    if(itemList == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(itemList, 0, (long) sizeof(SmlItemList_t));

    source_dmv = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlSource_t));
    if(source_dmv == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(source_dmv, 0, (long) sizeof(SmlSource_t));
    source_dmv->locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            "./DevInfo/DmV");

    item_dmv = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItem_t));
    if(item_dmv == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(item_dmv, 0, (long) sizeof(SmlItem_t));
    item_dmv->source = source_dmv;
    //item_dmv->data=dm_smlString2Pcdata(__FILE__, __LINE__, "1.2");

    dmTreeParam_getNodeData("./DevInfo/DmV", &data, NULL );
    item_dmv->data = dm_smlString2Pcdata(__FILE__, __LINE__, data);
    if (data) {
        free(data);
        data = NULL;
    }

    itemList->item = item_dmv;
    itemList->next = tmptr;
    tmptr = itemList;

    //Lang
    itemList = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItemList_t));
    if(itemList == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(itemList, 0, (long) sizeof(SmlItemList_t));

    source_lang = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlSource_t));
    if(source_lang == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(source_lang, 0, (long) sizeof(SmlSource_t));
    source_lang->locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            "./DevInfo/Lang");

    item_lang = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItem_t));
    if(item_lang == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(item_lang, 0, (long) sizeof(SmlItem_t));
    item_lang->source = source_lang;
    // item_lang->data=dm_smlString2Pcdata(__FILE__, __LINE__, "en-US");

    dmTreeParam_getNodeData("./DevInfo/Lang", &data, NULL );
    item_lang->data = dm_smlString2Pcdata(__FILE__, __LINE__, data);
    if (data) {
        free(data);
        data = NULL;
    }

    itemList->item = item_lang;
    itemList->next = tmptr;
    tmptr = itemList;

    //Man
    itemList = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItemList_t));
    if(itemList == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(itemList, 0, (long) sizeof(SmlItemList_t));

    source_man = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlSource_t));
    if(source_man == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(source_man, 0, (long) sizeof(SmlSource_t));
    source_man->locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            "./DevInfo/Man");

    item_man = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItem_t));
    if(item_man == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(item_man, 0, (long) sizeof(SmlItem_t));
    item_man->source = source_man;
    dmTreeParam_getNodeData("./DevInfo/Man", &data, NULL );
    //item_man->data=dm_smlString2Pcdata(__FILE__, __LINE__, "Hisense");
    item_man->data = dm_smlString2Pcdata(__FILE__, __LINE__, data);
    if (data) {
        free(data);
        data = NULL;
    }

    itemList->item = item_man;
    itemList->next = tmptr;
    tmptr = itemList;

    //Mod
    itemList = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItemList_t));
    if(itemList == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(itemList, 0, (long) sizeof(SmlItemList_t));

    source_mod = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlSource_t));
    if(source_mod == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(source_mod, 0, (long) sizeof(SmlSource_t));
    source_mod->locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
            "./DevInfo/Mod");

    item_mod = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItem_t));
    if(item_mod == NULL)
       return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(item_mod, 0, (long) sizeof(SmlItem_t));
    item_mod->source = source_mod;
    dmTreeParam_getNodeData("./DevInfo/Mod", &data, NULL );
    item_mod->data = dm_smlString2Pcdata(__FILE__, __LINE__, data);
    if (data) {
        free(data);
        data = NULL;
    }

    itemList->item = item_mod;
    itemList->next = tmptr;
    tmptr = itemList;

    return itemList;

}

/*******************************************************************
 **FUNC : dm_syncml_ReplaceCmd
 **Description : add sync order Replace
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_ReplaceCmd(short id) {
    char* temp;
    SmlReplace_t replace;
    short returnval = SML_ERR_OK;
    SmlItemListPtr_t itemList = NULL;
    SmlItemPtr_t item = NULL;
    SmlSourcePtr_t source = NULL;
    syncml_core_message("MMIDM  $$ enter dm_syncml_ReplaceCmd!$$");
    replace.itemList = NULL;

    replace.itemList = dm_Syncml_GetReplacedItems();

    if (NULL == replace.itemList) /*there is no items to be deleted*/
    {
        syncml_core_message("MMIDM  $$ there is no items to be replaced!$$");
        return returnval;
    }

    /* replace cmd */
    replace.elementType = SML_PE_REPLACE;
    temp = dm_smlunsignedInt2String(__FILE__, __LINE__,
            dm_task_relay_info->cmdID);
    dm_task_relay_info->cmdID++;
    replace.cmdID = dm_smlString2Pcdata(__FILE__, __LINE__, temp);
    dm_smlLibFree(temp);
    replace.flags = 0;
    replace.cred = NULL;
    replace.meta = NULL;

    returnval = dm_smlReplaceCmd(id, &replace);

    //free drynamic memorys.
    while (replace.itemList != NULL ) {
        itemList = replace.itemList;
        replace.itemList = itemList->next;
        item = itemList->item;

        if (NULL != item) {
            source = item->source;
            dm_smlFreePcdata(__FILE__, __LINE__, item->meta);
            dm_smlFreePcdata(__FILE__, __LINE__, item->data);
        }
        if (NULL != source) {
            dm_smlFreePcdata(__FILE__, __LINE__, source->locName);
            dm_smlFreePcdata(__FILE__, __LINE__, source->locURI);
        }

        dm_smlLibFree(source);
        source = NULL;
        dm_smlLibFree(item);
        item = NULL;
        dm_smlLibFree(itemList);
        itemList = NULL;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, replace.cmdID);
    dm_smlLibFree(replace.meta);
    return returnval;
}

/*******************************************************************
 **FUNC : dm_syncml_EndSync
 **Description : add sync </SyncBody>
 **out : the state of succeed or not
 **Date : 2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_EndSync(short id) {
    syncml_core_message("MMIDM  $$ return dm_syncml_EndSync $$");
    return dm_smlEndSync(id);
}

/*******************************************************************
 **FUNC : dm_syncml_EndMessage
 **Description : add sync </Sync>
 **out : the state of succeed or not
 **Date : 2007-11-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_EndMessage(short id) {
    /* This ends the SyncML document ... it has been assembled */
    /* SmlFinal_f says this is the last message in the SyncML package */
    /* (since it's the only one) */

    syncml_core_message("MMIDM  $$ return dm_syncml_EndMessage $$");
    return dm_smlEndMessage(id, SmlFinal_f);
}

/*******************************************************************
 **FUNC : dm_syncml_EndMessageWithoutFinal
 **Description : add sync </Sync>
 **out : the state of succeed or not
 **Date : 2007-11-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_EndMessageWithoutFinal(short id) {
    /* This ends the SyncML document ... it has been assembled */
    /*No SmlFinal_f says this is not the last message in the SyncML package */
    /* the server should send 222 to ask more data in the next message */
    //dm_myFreestatusofCQ();
    return dm_smlEndMessage(id, 0x0);
}
/*******************************************************************
 **FUNC : dm_syncml_ReceiveData
 **Description : add sync </Sync> reciver the message deal founction of sever
 **out : the state of succeed or not
 **Date :2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_ReceiveData(short id) {
    void* userData = NULL;
    short rc;
    /*  Prepare the callback parameter userData here!
     userData is a void pointer handed over to every callback function
     as one of the function arguments. The Toolkit doesn't touch the
     content of this structure. For instance, this mechanism can be used
     by the application to pass data to the callback routines. */syncml_task_message("MMIDM  dm_syncml_ReceiveData ,id=%d",id);
    rc = dm_smlSetUserData(id, &userData);
    if (rc != SML_ERR_OK) {
        return rc;
    }
    /* --- Parse commands & invoke callback routines of the application -- */
    rc = dm_smlProcessData(id, SML_ALL_COMMANDS);
    if (rc != SML_ERR_OK) {
        return rc;
    }
    return rc;
}

/*******************************************************************
 **FUNC : dm_syncml_TerminateInstance
 **Description : stop instance,realse the memory that applyed by instance
 **out : the state of succeed or not
 **Date :2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_TerminateInstance(short id) {
    return dm_smlTerminateInstance(id);
}

/*******************************************************************
 **FUNC :dm_syncml_TerminateAllExitInstance
 **Description : stop instance,realse all  memory that applyed by instance
 **out : the state of succeed or not
 **Date :2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_TerminateAllExitInstance(void) {
    short rc = 0;
    short id;
    for (id = 1; id < 5; id++) {
        rc |= dm_smlTerminateInstance(id);
        if (rc == SML_ERR_MGR_INVALID_INSTANCE_INFO) {
            syncml_core_message("MMIDM  $$ release all instances,instance NO.%d is NULL!$$",id);
        } else if (rc == SML_ERR_OK) {
            syncml_core_message("MMIDM  $$release all instances,instance NO.%d release OK!$$",id);
        } else {
            syncml_core_message("MMIDM  $$release all instances,instance NO.%d release error!$$",id);
        }
    }
    if (dm_pGlobalAnchor->instanceListAnchor != NULL ) {
        dm_smlLibFree((dm_pGlobalAnchor->instanceListAnchor));
        dm_pGlobalAnchor->instanceListAnchor = NULL;
    }
    return rc;
}

/*******************************************************************
 **FUNC :dm_syncml_Terminate
 **Description : stop pGlobalAnchor ,realse  memory
 **out : the state of succeed or not
 **Date :2005-5-10
 **Version: ver 1.0
 *******************************************************************/
short dm_syncml_Terminate(void) {
    short ret = SML_ERR_OK;

    syncml_core_message("MMIDM  $$Delete all deleted pb items error!$$");

    ret = dm_smlTerminate();
    return ret;
}
