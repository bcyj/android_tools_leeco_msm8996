#ifdef WIN32
#include "std_header.h"
#endif
#include "libmem.h"
#include "smlerr.h"
#include "dm_task.h"
#include "dm_pl_tree.h"

extern task_protocol_step_type* dm_protocol_step;
extern task_protocol_step_type* dm_protocol_step_priv;

extern task_relay_info_type* dm_task_relay_info;

extern status_cmd_queue_type* dm_status_cmd_que;
extern results_cmd_queue_type* dm_results_cmd_que;
extern const int DM_ALERT_CONFIRM_DIALOG;

#ifdef DM_LOG_FILE
static uint32 dm_logFileCount = 0;
#endif

/*************************************************************************/
//Local SyncML API functions
/*************************************************************************/
#ifdef __USE_DEVINF__
static void dm_smlFreeDevInfDatastore(SmlDevInfDatastorePtr_t data);
static void dm_smlFreeDevInfDatastoreList(SmlDevInfDatastoreListPtr_t data);
static void dm_smlFreeDevInfXmitList(SmlDevInfXmitListPtr_t data);
static void dm_smlFreeDevInfXmit(SmlDevInfXmitPtr_t data);
static void dm_smlFreeDevInfDSMem(SmlDevInfDSMemPtr_t data);
static void dm_smlFreeDevInfSynccap(SmlDevInfSyncCapPtr_t data);
static void dm_smlFreeDevInfExt(SmlDevInfExtPtr_t data);
static void dm_smlFreeDevInfExtList(SmlDevInfExtListPtr_t data);
static void dm_smlFreeDevInfCTData(SmlDevInfCTDataPtr_t data);
static void dm_smlFreeDevInfCTDataList(SmlDevInfCTDataListPtr_t data);
static void dm_smlFreeDevInfCTDataProp(SmlDevInfCTDataPropPtr_t data);
static void dm_smlFreeDevInfCTDataPropList(SmlDevInfCTDataPropListPtr_t data);
static void dm_smlFreeDevInfCTCap(SmlDevInfCTCapPtr_t data);
static void dm_smlFreeDevInfCtcapList(SmlDevInfCtcapListPtr_t data);
static void dm_smlFreeDevInfDevInf(SmlDevInfDevInfPtr_t data);
#endif

static void dm_smlFreePcdataList(SmlPcdataListPtr_t list);

static void dm_smlFreeTargetRefList(SmlTargetRefListPtr_t pTargetRefList);

static void dm_smlFreeSourceRefList(SmlSourceRefListPtr_t pSourceRefList);
/*****************************************************************************/
//  Description :frees all allocated memory of a smlProtoElement
//  Global resource dependence : none
//  Function: dm_smlFreeProtoElement
/*****************************************************************************/

short dm_smlFreeProtoElement(void * pProtoElement) {
    if (!pProtoElement) {
        return (SML_ERR_OK);
    }
    switch (((SmlUnknownProtoElementPtr_t) pProtoElement)->elementType) {

    case SML_PE_HEADER:
        dm_smlFreeSyncHdr((SmlSyncHdrPtr_t) pProtoElement);
        break;

    case SML_PE_SYNC_START:
        dm_smlFreeSync((SmlSyncPtr_t) pProtoElement);
        break;

    case SML_PE_ADD:
    case SML_PE_COPY:
    case SML_PE_REPLACE:
    case SML_PE_DELETE:
    case SML_PE_GENERIC:
        dm_smlFreeGeneric((SmlGenericCmdPtr_t) pProtoElement);
        break;

    case SML_PE_ALERT:
        dm_smlFreeAlert((SmlAlertPtr_t) pProtoElement);
        break;

    case SML_PE_ATOMIC_START:
    case SML_PE_SEQUENCE_START:
    case SML_PE_CMD_GROUP:
        dm_smlFreeAtomic((SmlAtomicPtr_t) pProtoElement);
        break;

#if (defined EXEC_SEND || defined EXEC_RECEIVE)
    case SML_PE_EXEC:
        dm_smlFreeExec((SmlExecPtr_t) pProtoElement);
        break;
#endif

    case SML_PE_PUT:
    case SML_PE_GET:
    case SML_PE_PUT_GET:
        dm_smlFreeGetPut((SmlPutPtr_t) pProtoElement);
        break;

    case SML_PE_MAP:
        dm_smlFreeMap((SmlMapPtr_t) pProtoElement);
        break;

    case SML_PE_RESULTS:
        dm_smlFreeResults((SmlResultsPtr_t) pProtoElement);
        break;

    case SML_PE_STATUS:
        dm_smlFreeStatus((SmlStatusPtr_t) pProtoElement);
        break;

    default:
        return (SML_ERR_A_UTI_UNKNOWN_PROTO_ELEMENT);
    }

    return (SML_ERR_OK);
}

/*****************************************************************************/
//  Description : frees the Memory of an allocated Pcdata memory object
//  In:        SmlPcdataPtr_t       A Pointer to a PcData structure, which should be freed
//  Function: dm_smlFreePcdata
/*****************************************************************************/
void dm_smlFreePcdata(const char * file, int line, SmlPcdataPtr_t pPcdata) {
    if (!pPcdata) {
        return;
    }
    if (pPcdata->contentType == SML_PCDATA_EXTENSION) {
        switch ((int) pPcdata->extension) {
#ifdef __USE_METINF__
        case SML_EXT_METINF:
            dm_smlFreeMetinfMetinf(pPcdata->content);
            dm_smlLibFree(pPcdata);
            break;
#endif
#ifdef __USE_DEVINF__
        case SML_EXT_DEVINF:
            dm_smlFreeDevInfDevInf(pPcdata->content);
            dm_smlLibFree(pPcdata);
            break;
#endif
        default:
            break;
        }
        return;
    }

    if (pPcdata->content) {
        dm_smlLibFree((pPcdata->content));
    }
    dm_smlLibFree(pPcdata);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated PcdataList memory object
//  In:        SmlPcdataListPtr_t
//  Function: dm_smlFreePcdata
/*****************************************************************************/
static void dm_smlFreePcdataList(SmlPcdataListPtr_t list) {
    if (!list) {
        return;
    }
    dm_smlFreePcdataList(list->next);
    dm_smlFreePcdata(__FILE__, __LINE__, list->data);
    dm_smlLibFree(list);
    return;
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated SourceTargetPtr memory object
//  In:        SmlSourcePtr_t
//  Function: dm_smlFreeSourceTargetPtr
/*****************************************************************************/
void dm_smlFreeSourceTargetPtr(SmlSourcePtr_t pSourceTarget) {
    if (!pSourceTarget) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pSourceTarget->locURI);
    dm_smlFreePcdata(__FILE__, __LINE__, pSourceTarget->locName);

    dm_smlLibFree(pSourceTarget);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated CredPtr memory object
//  In:        SmlCredPtr_t
//  Function: dm_smlFreeCredPtr
/*****************************************************************************/
void dm_smlFreeCredPtr(SmlCredPtr_t pCred) {
    if (!pCred) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pCred->meta);
    dm_smlFreePcdata(__FILE__, __LINE__, pCred->data);

    dm_smlLibFree(pCred);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated SyncHdr memory object
//  In:        SmlSyncHdrPtr_t
//  Function: dm_smlFreeSyncHdr
/*****************************************************************************/
void dm_smlFreeSyncHdr(SmlSyncHdrPtr_t pSyncHdr) {
    if (!pSyncHdr) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pSyncHdr->version);
    dm_smlFreePcdata(__FILE__, __LINE__, pSyncHdr->proto);
    dm_smlFreePcdata(__FILE__, __LINE__, pSyncHdr->sessionID);
    dm_smlFreePcdata(__FILE__, __LINE__, pSyncHdr->msgID);
    dm_smlFreePcdata(__FILE__, __LINE__, pSyncHdr->respURI);
    dm_smlFreePcdata(__FILE__, __LINE__, pSyncHdr->meta);

    dm_smlFreeSourceTargetPtr(pSyncHdr->source);
    dm_smlFreeSourceTargetPtr(pSyncHdr->target);

    dm_smlFreeCredPtr(pSyncHdr->cred);

    dm_smlLibFree(pSyncHdr);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated Sync memory object
//  In:        SmlSyncPtr_t
//  Function: dm_smlFreeSync
/*****************************************************************************/
void dm_smlFreeSync(SmlSyncPtr_t pSync) {
    if (!pSync) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pSync->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pSync->meta);
    dm_smlFreePcdata(__FILE__, __LINE__, pSync->noc);

    dm_smlFreeSourceTargetPtr(pSync->source);
    dm_smlFreeSourceTargetPtr(pSync->target);

    dm_smlFreeCredPtr(pSync->cred);

    dm_smlLibFree(pSync);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated ItemPtr memory object
//  In:        SmlItemPtr_t
//  Function: dm_smlFreeItemPtr
/*****************************************************************************/
void dm_smlFreeItemPtr(SmlItemPtr_t pItem) {
    if (!pItem) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pItem->meta);
    dm_smlFreePcdata(__FILE__, __LINE__, pItem->data);

    dm_smlFreeSourceTargetPtr(pItem->source);
    dm_smlFreeSourceTargetPtr(pItem->target);

    dm_smlLibFree(pItem);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated ItemList memory object
//  In:        SmlItemListPtr_t
//  Function: dm_smlFreeItemList
/*****************************************************************************/
void dm_smlFreeItemList(SmlItemListPtr_t pItemList) {
    SmlItemListPtr_t pTmp;

    while (pItemList) {
        pTmp = pItemList->next;
        dm_smlFreeItemPtr(pItemList->item);
        dm_smlLibFree(pItemList);
        pItemList = pTmp;
    }
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated Generic memory object
//  In:        SmlGenericCmdPtr_t
//  Function: dm_smlFreeGeneric
/*****************************************************************************/
void dm_smlFreeGeneric(SmlGenericCmdPtr_t pGenericCmd) {
    if (!pGenericCmd) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pGenericCmd->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pGenericCmd->meta);

    dm_smlFreeCredPtr(pGenericCmd->cred);

    dm_smlFreeItemList(pGenericCmd->itemList);

    dm_smlLibFree(pGenericCmd);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated Alert memory object
//  In:        SmlAlertPtr_t
//  Function: dm_smlFreeAlert
/*****************************************************************************/
void dm_smlFreeAlert(SmlAlertPtr_t pAlert) {
    if (!pAlert) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pAlert->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pAlert->data);

    dm_smlFreeCredPtr(pAlert->cred);

    dm_smlFreeItemList(pAlert->itemList);

    dm_smlLibFree(pAlert);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated Status memory object
//  In:        SmlStatusPtr_t
//  Function: dm_smlFreeStatus
/*****************************************************************************/
void dm_smlFreeStatus(SmlStatusPtr_t pStatus) {
    if (!pStatus) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pStatus->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pStatus->msgRef);
    dm_smlFreePcdata(__FILE__, __LINE__, pStatus->cmdRef);
    dm_smlFreePcdata(__FILE__, __LINE__, pStatus->cmd);
    dm_smlFreePcdata(__FILE__, __LINE__, pStatus->data);

    dm_smlFreeCredPtr(pStatus->cred);
    dm_smlFreeChalPtr(pStatus->chal);

    dm_smlFreeTargetRefList(pStatus->targetRefList);
    dm_smlFreeSourceRefList(pStatus->sourceRefList);

    dm_smlFreeItemList(pStatus->itemList);

    dm_smlLibFree(pStatus);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated ChalPtr memory object
//  In:        SmlChalPtr_t
//  Function: dm_smlFreeChalPtr
/*****************************************************************************/
void dm_smlFreeChalPtr(SmlChalPtr_t pChal) {
    if (!pChal)
        return;

    dm_smlFreePcdata(__FILE__, __LINE__, pChal->meta);

    dm_smlLibFree(pChal);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated TargetRefList memory object
//  In:       SmlTargetRefListPtr_t
//  Function: dm_smlFreeTargetRefList
/*****************************************************************************/
static void dm_smlFreeTargetRefList(SmlTargetRefListPtr_t pTargetRefList) {
    SmlTargetRefListPtr_t pTmp;

    while (pTargetRefList) {
        pTmp = pTargetRefList->next;
        dm_smlFreePcdata(__FILE__, __LINE__, pTargetRefList->targetRef);
        dm_smlLibFree(pTargetRefList);
        pTargetRefList = pTmp;
    }
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated SourceRefList memory object
//  In:       SmlSourceRefListPtr_t
//  Function: dm_smlFreeSourceRefList
/*****************************************************************************/
static void dm_smlFreeSourceRefList(SmlSourceRefListPtr_t pSourceRefList) {
    SmlSourceRefListPtr_t pTmp;

    while (pSourceRefList) {
        pTmp = pSourceRefList->next;
        dm_smlFreePcdata(__FILE__, __LINE__, pSourceRefList->sourceRef);
        dm_smlLibFree(pSourceRefList);
        pSourceRefList = pTmp;
    }
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated GetPut  memory object
//  In:       SmlPutPtr_t
//  Function: dm_smlFreeGetPut
/*****************************************************************************/
void dm_smlFreeGetPut(SmlPutPtr_t pGetPut) {
    if (!pGetPut) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, pGetPut->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pGetPut->meta);
    dm_smlFreePcdata(__FILE__, __LINE__, pGetPut->lang);

    dm_smlFreeCredPtr(pGetPut->cred);

    dm_smlFreeItemList(pGetPut->itemList);

    dm_smlLibFree(pGetPut);
}

#if (defined ATOMIC_RECEIVE || defined SEQUENCE_RECEIVE)
/*****************************************************************************/
//  Description : frees the Memory of  allocated Atomic  memory object
//  In:       SmlAtomicPtr_t
//  Function: dm_smlFreeAtomic
/*****************************************************************************/
void dm_smlFreeAtomic(SmlAtomicPtr_t pAtomic) {
    if (!pAtomic) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, pAtomic->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pAtomic->meta);

    dm_smlLibFree(pAtomic);
}
#endif

#if (defined EXEC_SEND || defined EXEC_RECEIVE)

/*****************************************************************************/
//  Description : frees the Memory of  allocated Exec  memory object
//  In:       SmlExecPtr_t
//  Function: dm_smlFreeExec
/*****************************************************************************/
void dm_smlFreeExec(SmlExecPtr_t pExec) {
    if (!pExec)
        return;

    dm_smlFreePcdata(__FILE__, __LINE__, pExec->cmdID);

    dm_smlFreeCredPtr(pExec->cred);

    dm_smlFreeItemPtr(pExec->item);

    dm_smlLibFree(pExec);
}

#endif

#ifdef MAP_RECEIVE
/*****************************************************************************/
//  Description : frees the Memory of  allocated Map  memory object
//  In:       SmlMapPtr_t
//  Function: dm_smlFreeMap
/*****************************************************************************/
void dm_smlFreeMap(SmlMapPtr_t pMap) {
    if (!pMap) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, pMap->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pMap->meta);

    dm_smlFreeCredPtr(pMap->cred);

    dm_smlFreeSourceTargetPtr(pMap->source);
    dm_smlFreeSourceTargetPtr(pMap->target);

    dm_smlFreeMapItemList(pMap->mapItemList);

    dm_smlLibFree(pMap);
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated MapItemList   memory object
//  In:       SmlMapItemListPtr_t
//  Function: dm_smlFreeMapItemList
/*****************************************************************************/
void dm_smlFreeMapItemList(SmlMapItemListPtr_t pMapItemList) {
    SmlMapItemListPtr_t pTmp;

    while (pMapItemList) {
        pTmp = pMapItemList->next;
        dm_smlFreeMapItemPtr(pMapItemList->mapItem);
        dm_smlLibFree(pMapItemList);
        pMapItemList = pTmp;
    }
}

/*****************************************************************************/
//  Description : frees the Memory of  allocated MapItemPtr  memory object
//  In:       SmlMapItemPtr_t
//  Function: dm_smlFreeMapItemPtr
/*****************************************************************************/
void dm_smlFreeMapItemPtr(SmlMapItemPtr_t pMapItem) {
    if (!pMapItem) {
        return;
    }
    dm_smlFreeSourceTargetPtr(pMapItem->source);
    dm_smlFreeSourceTargetPtr(pMapItem->target);

    dm_smlLibFree(pMapItem);
}
#endif

#ifdef RESULT_RECEIVE
/*****************************************************************************/
//  Description : frees the Memory of  allocated Results  memory object
//  In:      SmlResultsPtr_t
//  Function: dm_smlFreeResults
/*****************************************************************************/
void dm_smlFreeResults(SmlResultsPtr_t pResults) {
    if (!pResults) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, pResults->cmdID);
    dm_smlFreePcdata(__FILE__, __LINE__, pResults->msgRef);
    dm_smlFreePcdata(__FILE__, __LINE__, pResults->cmdRef);
    dm_smlFreePcdata(__FILE__, __LINE__, pResults->meta);
    dm_smlFreePcdata(__FILE__, __LINE__, pResults->targetRef);
    dm_smlFreePcdata(__FILE__, __LINE__, pResults->sourceRef);

    dm_smlFreeItemList(pResults->itemList);

    dm_smlLibFree(pResults);
}
#endif

SmlPcdataPtr_t dm_concatPCData(SmlPcdataPtr_t pDat1, const SmlPcdataPtr_t pDat2) {
    if (pDat1->contentType != pDat2->contentType) {
        return NULL ;
    }
    switch (pDat1->contentType) {
    case SML_PCDATA_STRING:
        pDat1->content = (void*) dm_smlLibStrcat(pDat1->content,
                pDat2->content);
        pDat1->length += pDat2->length;
        break;
    case SML_PCDATA_OPAQUE:
        if ((pDat1->content = dm_smlLibRealloc(pDat1->content,
                pDat1->length + pDat2->length)) == NULL ) {
            return NULL ;
        }
        dm_smlLibMemmove(((unsigned char*) pDat1->content) + pDat1->length,
                pDat2->content, pDat2->length);
        pDat1->length += pDat2->length;
        break;
    default:
        return NULL ;
    }
    return pDat1;
}

#ifdef __USE_METINF__

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup MetaInf  structures
//Function :   dm_smlFreeDevInfDevInf
/******************************************************************/
void dm_smlFreeMetinfMetinf(SmlMetInfMetInfPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->format);
    dm_smlFreePcdata(__FILE__, __LINE__, data->type);
    dm_smlFreePcdata(__FILE__, __LINE__, data->mark);
    dm_smlFreePcdata(__FILE__, __LINE__, data->size);
    dm_smlFreePcdata(__FILE__, __LINE__, data->version);
    dm_smlFreePcdata(__FILE__, __LINE__, data->nextnonce);
    dm_smlFreePcdata(__FILE__, __LINE__, data->maxmsgsize);
    /* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
    dm_smlFreePcdata(__FILE__, __LINE__, data->maxobjsize);
    dm_smlFreeMetinfAnchor(data->anchor);
    dm_smlFreeMetinfMem(data->mem);
    dm_smlFreePcdataList(data->emi);
    dm_smlLibFree(data);
    return;
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup MetinfAnchor   structures
//Function :   dm_smlFreeDevInfDevInf
/******************************************************************/
void dm_smlFreeMetinfAnchor(SmlMetInfAnchorPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->last);
    dm_smlFreePcdata(__FILE__, __LINE__, data->next);
    dm_smlLibFree(data);
    return;
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInf DTD structures
//Function :   dm_smlFreeDevInfDevInf
/******************************************************************/
void dm_smlFreeMetinfMem(SmlMetInfMemPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->shared);
    dm_smlFreePcdata(__FILE__, __LINE__, data->free);
    dm_smlFreePcdata(__FILE__, __LINE__, data->freeid);
    dm_smlLibFree(data);
    return;
}
#endif

#ifdef __USE_DEVINF__
/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInf  structures
//Function :   dm_smlFreeDevInfDevInf
/******************************************************************/
static void dm_smlFreeDevInfDevInf(SmlDevInfDevInfPtr_t data) {
    if (!data) {
        return;
    }

    dm_smlFreePcdata(__FILE__, __LINE__, data->verdtd);
    dm_smlFreePcdata(__FILE__, __LINE__, data->man);
    dm_smlFreePcdata(__FILE__, __LINE__, data->mod);
    dm_smlFreePcdata(__FILE__, __LINE__, data->oem);
    dm_smlFreePcdata(__FILE__, __LINE__, data->fwv);
    dm_smlFreePcdata(__FILE__, __LINE__, data->hwv);
    dm_smlFreePcdata(__FILE__, __LINE__, data->swv);
    dm_smlFreePcdata(__FILE__, __LINE__, data->devid);
    dm_smlFreePcdata(__FILE__, __LINE__, data->devtyp);
    dm_smlFreeDevInfDatastoreList(data->datastore);
    dm_smlFreeDevInfExtList(data->ext);
    dm_smlFreeDevInfCtcapList(data->ctcap);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInf DTD structures
//Function :   dm_smlFreeDevInfDatastore
/******************************************************************/
static void dm_smlFreeDevInfDatastore(SmlDevInfDatastorePtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->sourceref);
    dm_smlFreePcdata(__FILE__, __LINE__, data->displayname);
    dm_smlFreePcdata(__FILE__, __LINE__, data->maxguidsize);
    dm_smlFreeDevInfXmit(data->rxpref);
    dm_smlFreeDevInfXmit(data->txpref);
    dm_smlFreeDevInfXmitList(data->rx);
    dm_smlFreeDevInfXmitList(data->tx);
    dm_smlFreeDevInfDSMem(data->dsmem);
    dm_smlFreeDevInfSynccap(data->synccap);
    dm_smlLibFree(data);
    return;
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DatastoreList structures
//Function :   dm_smlFreeDevInfDatastoreList
/******************************************************************/
static void dm_smlFreeDevInfDatastoreList(SmlDevInfDatastoreListPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreeDevInfDatastore(data->data);
    dm_smlFreeDevInfDatastoreList(data->next);
    dm_smlLibFree(data);
    return;
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfXmitList
//Function :   dm_smlFreeDevInfXmitList
/******************************************************************/
static void dm_smlFreeDevInfXmitList(SmlDevInfXmitListPtr_t data) {
    if (!data)
        return;
    dm_smlFreeDevInfXmit(data->data);
    dm_smlFreeDevInfXmitList(data->next);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfXmit
//Function :   dm_smlFreeDevInfXmit
/******************************************************************/
static void dm_smlFreeDevInfXmit(SmlDevInfXmitPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->cttype);
    dm_smlFreePcdata(__FILE__, __LINE__, data->verct);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfDSMem
//Function :   dm_smlFreeDevInfDSMem
/******************************************************************/
static void dm_smlFreeDevInfDSMem(SmlDevInfDSMemPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->maxmem);
    dm_smlFreePcdata(__FILE__, __LINE__, data->maxid);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfSynccap
//Function :   dm_smlFreeDevInfSynccap
/******************************************************************/
static void dm_smlFreeDevInfSynccap(SmlDevInfSyncCapPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdataList(data->synctype);
    dm_smlLibFree(data);
}

static void dm_smlFreeDevInfExt(SmlDevInfExtPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->xnam);
    dm_smlFreePcdataList(data->xval);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfExtList
//Function :  dm_smlFreeDevInfExtList
/******************************************************************/
static void dm_smlFreeDevInfExtList(SmlDevInfExtListPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreeDevInfExt(data->data);
    dm_smlFreeDevInfExtList(data->next);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfCTData
//Function :   dm_smlFreeDevInfCTData
/******************************************************************/
static void dm_smlFreeDevInfCTData(SmlDevInfCTDataPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->name);
    dm_smlFreePcdata(__FILE__, __LINE__, data->dname);
    dm_smlFreePcdataList(data->valenum);
    dm_smlFreePcdata(__FILE__, __LINE__, data->datatype);
    dm_smlFreePcdata(__FILE__, __LINE__, data->size);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfCTDataProp
//Function :   dm_smlFreeDevInfCTDataProp
/******************************************************************/
static void dm_smlFreeDevInfCTDataProp(SmlDevInfCTDataPropPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreeDevInfCTData(data->prop);
    dm_smlFreeDevInfCTDataList(data->param);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfCTDataList
//Function :   dm_smlFreeDevInfCTDataList
/******************************************************************/
static void dm_smlFreeDevInfCTDataList(SmlDevInfCTDataListPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreeDevInfCTData(data->data);
    dm_smlFreeDevInfCTDataList(data->next);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfCTDataPropList
//Function :   dm_smlFreeDevInfCTDataPropList
/******************************************************************/
static void dm_smlFreeDevInfCTDataPropList(SmlDevInfCTDataPropListPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreeDevInfCTDataProp(data->data);
    dm_smlFreeDevInfCTDataPropList(data->next);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfCTCap
//Function :   dm_smlFreeDevInfCTCap
/******************************************************************/
static void dm_smlFreeDevInfCTCap(SmlDevInfCTCapPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreePcdata(__FILE__, __LINE__, data->cttype);
    dm_smlFreeDevInfCTDataPropList(data->prop);
    dm_smlLibFree(data);
}

/******************************************************************/
//Decr :  Subfunctions to dm_smlFreePcdata, to freeup DevInfCtcapList
//Function :  dm_smlFreeDevInfCtcapList
/******************************************************************/
static void dm_smlFreeDevInfCtcapList(SmlDevInfCtcapListPtr_t data) {
    if (!data) {
        return;
    }
    dm_smlFreeDevInfCTCap(data->data);
    dm_smlFreeDevInfCtcapList(data->next);
    dm_smlLibFree(data);
}
#endif

/******************************************************************/
//Decr :  copy a string into a Pcdata structure
//Function :   dm_smlString2Pcdata
/*******************************************************************/
SmlPcdataPtr_t dm_smlString2Pcdata(const char * file, int line, char* str) {
    /* Definitions */
    SmlPcdataPtr_t pcdata;

    /* Invalid Input */
    if (!str) {
        return NULL ;
    }

    // Allocate the PcData Structure
    pcdata = (SmlPcdataPtr_t) dm_smlLibMalloc(file, line,
            (long) sizeof(SmlPcdata_t));
    if (!pcdata) {
        return NULL ;
    }
    dm_smlLibMemset(pcdata, 0, (long) sizeof(SmlPcdata_t));

    //Set the PcData Structure
    pcdata->contentType = SML_PCDATA_STRING;
    pcdata->extension = SML_EXT_UNDEFINED;
    pcdata->length = dm_smlLibStrlen(str);
    pcdata->content = (void*) dm_smlLibStrdup(str);

    return pcdata;
}

/******************************************************************/
//Decr :  copy a metainfo into a Pcdata structure
//Function : dm_smlmeta2extPcdata
/*******************************************************************/
SmlPcdataPtr_t dm_smlmeta2extPcdata(const char * file, int line,
        SmlMetInfMetInfPtr_t meta) {
    /* Definitions */
    SmlPcdataPtr_t pcdata;

    /* Invalid Input */
    if (!meta) {
        return NULL ;
    }
    // Allocate the PcData Structure
    pcdata = (SmlPcdataPtr_t) dm_smlLibMalloc(file, line,
            (long) sizeof(SmlPcdata_t));
    if (!pcdata) {
        return NULL ;
    }
    dm_smlLibMemset(pcdata, 0, (long) sizeof(SmlPcdata_t));
    // Set the PcData Structure
    pcdata->contentType = SML_PCDATA_EXTENSION;
    pcdata->extension = SML_EXT_METINF;
    pcdata->length = sizeof(meta);
    pcdata->content = (void*) (meta);

    return pcdata;
}

/****************************************************************]**/
//Decr :  copy a device info into a Pcdata structure
//Function :  dm_smldevinfo2extPcdata
/*******************************************************************/
SmlPcdataPtr_t dm_smldevinfo2extPcdata(const char * file, int line,
        SmlDevInfDevInfPtr_t devinfo) {
    // Definitions
    SmlPcdataPtr_t pcdata;

    //Invalid Input
    if (!devinfo) {
        return NULL ;
    }
    // Allocate the PcData Structure
    pcdata = (SmlPcdataPtr_t) dm_smlLibMalloc(file, line,
            (long) sizeof(SmlPcdata_t));
    if (!pcdata) {
        return NULL ;
    }
    dm_smlLibMemset(pcdata, 0, (long) sizeof(SmlPcdata_t));
    //Set the PcData Structure
    pcdata->contentType = SML_PCDATA_EXTENSION;
    pcdata->extension = SML_EXT_DEVINF;
    pcdata->length = sizeof(devinfo);
    pcdata->content = (void*) (devinfo);

    return pcdata;
}

/*************************************************************/
//Decr :  copy a Pcdata structure into a string
//Function :   dm_smlPcdata2String
/*******************************************************************/
char* dm_smlPcdata2String(const char * file, int line, SmlPcdataPtr_t pcdata) {
    //Definitions
    char* str;

    // Invalid Input
    if (!pcdata) {
        return NULL ;
    }
    //Allocate the String
    syncml_cb_message("MMIDM @@dm_smlPcdata2String size=%d",pcdata->length);

    str = (char*) dm_smlLibMalloc(file, line, (long) (pcdata->length + 1));
    if (str == NULL ) {
        return NULL ;
    }

    // Copy the string into the allocated data structure
    dm_smlLibMemcpy((unsigned char*) str, (unsigned char*) pcdata->content,
            pcdata->length);
    *(str + pcdata->length) = '\0';
    return str;
}

/*************************************************************/
//Decr :  copy an unsighed int  into a string
//Function :   dm_smlunsignedInt2String
/*******************************************************************/
char* dm_smlunsignedInt2String(const char * file, int line,
        unsigned int integer) {
    // Definitions
    char* str = NULL;
    char onechar = 0;
    int value = 0;
    int left = 0;
    int i = 0;
    int j = 0;
    // Allocate the String
    str = (char*) dm_smlLibMalloc(file, line, (long) 10);
    if (str == NULL ) {
        return NULL ;
    }
    value = integer;

    //get the Decimal number in reverse order
    do {
        left = value % 10;
        value = value / 10;
        onechar = (char) (0x30 + left);
        *(str + j) = onechar;
        j++;
    } while (value != 0);
    j--;
    //modify the error when it's two digits
    for (i = 0; i < j / 2 + 1; i++) {
        onechar = *(str + i);
        *(str + i) = *(str + j - i);
        *(str + j - i) = onechar;
    }
    *(str + j + 1) = '\0';
    return str;
}

/*************************************************************/
//Decr :  copy string to unsighed int
//Function :   dm_smlunsignedInt2String
/*******************************************************************/
unsigned int dm_smlString2unsignedInt(char* str) {
    int i;
    unsigned int temp;
    char achar;
    unsigned int ret = 0;
    syncml_cb_message("MMIDM @@dm_smlString2unsignedInt =%s",str);

    for (i = 0; i < dm_smlLibStrlen(str); i++) {
        achar = *(str + i);
        switch (achar) {
        case '0': {
            temp = 0;
        }
            break;
        case '1': {
            temp = 1;
        }
            break;
        case '2': {
            temp = 2;
        }
            break;
        case '3': {
            temp = 3;
        }
            break;
        case '4': {
            temp = 4;
        }
            break;
        case '5': {
            temp = 5;
        }
            break;
        case '6': {
            temp = 6;
        }
            break;
        case '7': {
            temp = 7;
        }
            break;
        case '8': {
            temp = 8;
        }
            break;
        case '9': {
            temp = 9;
        }
            break;
        default: {
            return 0;
        }
        }
        ret = ret * 10 + temp;/*lint !e737*/
    }
    syncml_cb_message("MMIDM  @@ leave dm_smlString2unsignedInt =%d",ret);

    return ret;
}

/*******************************************************************/
//Decr :  callback handling <SyncHdr> command
//See :   dm_smlInitInstance
//Function :   dm_myHandleStartMessage
/*******************************************************************/
short dm_myHandleStartMessage(short id, void* userData,
        SmlSyncHdrPtr_t pSyncHdr) {
    SmlStatusPtr_t status;
    SmlTargetRefListPtr_t targetlist;
    SmlSourceRefListPtr_t sourcelist;
    SmlMetInfMetInfPtr_t chal_meta = NULL;
    SmlChalPtr_t chal = NULL;
    char nextnonce[80] = { 0 };
    char *temp_char = NULL;
    int rand_num = 0;
    int i = 0;
    syncml_cb_message("MMIDM @@dm_myHandleStartMessage@@ !");
    status = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlStatus_t));
    if (NULL == status) {
        return dm_smlFreeProtoElement(pSyncHdr);
    }

    targetlist = (SmlTargetRefListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlTargetRefList_t));
    if (NULL == targetlist) {
        dm_smlLibFree(status);
        return dm_smlFreeProtoElement(pSyncHdr);
    }
    sourcelist = (SmlSourceRefListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlSourceRefList_t));
    if (NULL == sourcelist) {
        dm_smlLibFree(status);
        dm_smlLibFree(targetlist);
        return dm_smlFreeProtoElement(pSyncHdr);
    }
    dm_smlLibMemset(status, 0, (long) sizeof(SmlStatus_t));
    status->targetRefList = targetlist;
    status->sourceRefList = sourcelist;

    //dm_status_cmd_que->msgref=pSyncHdr->msgID;
    dm_status_cmd_que->msgref = dm_smlString2Pcdata(__FILE__, __LINE__,
            pSyncHdr->msgID->content);

    status->elementType = SML_PE_STATUS;
    //status->msgRef=dm_smlString2Pcdata(dm_status_cmd_que->msgref->content);
    status->msgRef = dm_smlString2Pcdata(__FILE__, __LINE__,
            pSyncHdr->msgID->content);
    status->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__, "0"); //sync header is considered as use default cmdid 0
    status->cmd = dm_smlString2Pcdata(__FILE__, __LINE__, "SyncHdr");
    // targetlist->targetRef=pSyncHdr->target->locURI;
    targetlist->targetRef = dm_smlString2Pcdata(__FILE__, __LINE__,
            pSyncHdr->target->locURI->content);
    targetlist->next = NULL;
    //sourcelist->sourceRef=pSyncHdr->source->locURI;
    sourcelist->sourceRef = dm_smlString2Pcdata(__FILE__, __LINE__,
            pSyncHdr->source->locURI->content);
    sourcelist->next = NULL;

    //add the management for RespURI
    if (NULL != pSyncHdr->respURI) {
        dm_smlLibMemset(dm_task_relay_info->syncml_connect_addr, 0,
                MMIDM_FULLPATH_LEN);
        memcpy(dm_task_relay_info->syncml_connect_addr,
                pSyncHdr->respURI->content,
                MIN(pSyncHdr->respURI->length,MMIDM_FULLPATH_LEN));
    }
    rand_num = rand();
    syncml_cb_message("MMIDM @@dm_myHandleStartMessage, rand_num=%d",rand_num);

    for (i = 0; i < 16; i++) {
        nextnonce[i] = (rand_num % 2 == 1) ? '1' : '0';
        rand_num = rand_num >> 1;
    }

    syncml_cb_message("MMIDM @@dm_myHandleStartMessage, nextnonce=%s",nextnonce);
    dm_calc_b64_cred(nextnonce, xppStrlen(nextnonce));
    syncml_cb_message("MMIDM @@dm_myHandleStartMessage, nextnonce_base64=%s",nextnonce);

    //xppStrcpy(nextnonce,"qpzVmCO/OBDIjswqK5shug==");

    chal_meta = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlMetInfMetInf_t));
    chal = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlChal_t));
    if(chal_meta != NULL) {
        dm_smlLibMemset(chal_meta, 0, sizeof(SmlMetInfMetInf_t));
        chal_meta->format = dm_smlString2Pcdata(__FILE__, __LINE__, "b64");
        chal_meta->type = dm_smlString2Pcdata(__FILE__, __LINE__,
                "syncml:auth-MAC");
        //chal_meta->nextnonce=dm_smlString2Pcdata(__FILE__, __LINE__, "qpzVmCO/OBDIjswqK5shug==");
        chal_meta->nextnonce = dm_smlString2Pcdata(__FILE__, __LINE__, nextnonce);
    }
    if(chal != NULL) {
        dm_smlLibMemset(chal, 0, sizeof(SmlChal_t));
        chal->meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, chal_meta);
        status->chal = chal;
    }

    status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");

    //keep this Nonce, In order to test the  next package's integration
    xppMemcpy(dm_task_relay_info->client_nextnonce, nextnonce,
            xppStrlen(nextnonce));
    dm_task_relay_info->client_nextnonce[xppStrlen(nextnonce)] = 0;

    dm_myAddstatustoCQ(status);
    return dm_smlFreeProtoElement(pSyncHdr);
}

short dm_myHandleEndMessage(short id, void * userData, unsigned char final) {
    //do nothing
    return SML_ERR_OK;
}

/*******************************************************************/
//Decr :  callback handling <Sync> command
//See :   dm_smlInitInstance
//Function :   dm_myHandleStartSync
/*******************************************************************/
short dm_myHandleStartSync(short id, void * userData, SmlSyncPtr_t pSync) {
    SmlStatusPtr_t status;
    SmlTargetRefListPtr_t targetlist;
    SmlSourceRefListPtr_t sourcelist;
    syncml_cb_message("MMIDM @@dm_myHandleStartSync@@ !");

    status = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlStatus_t));

    targetlist = (SmlTargetRefListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlTargetRefList_t));

    sourcelist = (SmlSourceRefListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlSourceRefList_t));
    if(status != NULL) {
        dm_smlLibMemset(status, 0, (long) sizeof(SmlStatus_t));
        status->targetRefList = targetlist;
        status->sourceRefList = sourcelist;

        status->elementType = SML_PE_STATUS;
        status->msgRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                dm_status_cmd_que->msgref->content);
        //status->cmdRef=pSync->cmdID;
        status->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                pSync->cmdID->content); // memory 20080223
        status->cmd = dm_smlString2Pcdata(__FILE__, __LINE__, "Sync");
    }
    //targetlist->targetRef=pSync->target->locURI;

    if(targetlist != NULL) {
        targetlist->targetRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                pSync->target->locURI->content); // memory 20080223
        targetlist->next = NULL;
    }
    //sourcelist->sourceRef=pSync->source->locURI;

    if(sourcelist != NULL) {
        sourcelist->sourceRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                pSync->source->locURI->content); // memory 20080223
        sourcelist->next = NULL;
    }

    if(status != NULL) {
        status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
    }
    dm_myAddstatustoCQ(status);

    return dm_smlFreeProtoElement(pSync); // SML_ERR_OK; //  20080223
}

short dm_myHandleEndSync(short id, void * userData) {
    //do nothing
    return SML_ERR_OK;
}

#ifdef ATOMIC_RECEIVE
/* these callbacks are NOT included in the Toolkit lite version */
short dm_myHandlestartAtomic(short id, void * userData, SmlAtomicPtr_t pContent) {
    return SML_ERR_OK;
}

short dm_myHandleendAtomic(short id, void * userData) {
    return SML_ERR_OK;
}
#endif

#ifdef SEQUENCE_RECEIVE
short dm_myHandlestartSequence(short id, void * userData,
        SmlSequencePtr_t pContent) {
    return SML_ERR_OK;
}

short dm_myHandleendSequence(short id, void * userData) {
    return SML_ERR_OK;
}
#endif

/*******************************************************************/
//Decr :  callback handling <Add> command
//See :   dm_smlInitInstance
//Function :    dm_myHandleAdd
/*******************************************************************/
short dm_myHandleAdd(short id, void* userData, SmlAddPtr_t pContent) {

    SmlStatusPtr_t status;
    syncml_cb_message("MMIDM @@ dm_myHandleAdd @@");

    status = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlStatus_t));
    if(status != NULL) {
        dm_smlLibMemset(status, 0, (long) sizeof(SmlStatus_t));

        status->elementType = SML_PE_STATUS;
        status->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                pContent->cmdID->content);
        status->cmd = dm_smlString2Pcdata(__FILE__, __LINE__, "Add");
        status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
    }
    dm_myAddstatustoCQ(status);

    dm_task_set_ctrl_type(SYNC_CONTROL_CONTINUE);
    return dm_smlFreeProtoElement(pContent);

}

/*******************************************************************/
//Effect:          No
//date  :         2009-8-11
//Function :    dm_myHandleget
/*******************************************************************/
short dm_myHandlealert(short id, void* userData, SmlAlertPtr_t pContent) {
    SmlStatusPtr_t status;
    char* alertdata;
    //Need to intialise to 0 if alertdata is NULL
    //alertcode will become uninitialised
    int alertcode = 0;

    syncml_cb_message("MMIDM @@dm_myHandlealert@@ !");
    status = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlStatus_t));
    if(status != NULL) {
        dm_smlLibMemset(status, 0, (long) sizeof(SmlStatus_t));

        status->elementType = SML_PE_STATUS;

        status->msgRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                dm_status_cmd_que->msgref->content);
        status->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                pContent->cmdID->content); //  20080223
        status->cmd = dm_smlString2Pcdata(__FILE__, __LINE__, "Alert");
    }
    //here is the server alert  data  process
    syncml_cb_message("MMIDM @@handle alert@@");
    alertdata = dm_smlPcdata2String(__FILE__, __LINE__, pContent->data);
    syncml_cb_message("MMIDM @@dm_myHandlealert@@ alertdata: %s", alertdata);
    if(alertdata != NULL) {
        alertcode = atoi(alertdata);
        dm_smlLibFree(alertdata);
    }
    syncml_cb_message("MMIDM @@dm_myHandlealert@@ alertcode: %d", alertcode);
    dm_task_set_ctrl_type(SYNC_CONTROL_WAIT_USER_OPE);
    switch (alertcode) {
    //the alert message sended by DM server, only for show,users needn't operate
    case 1100: {
        syncml_cb_message("MMIDM @@handle alert: 1100");
        if(status != NULL) {
            status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
        }
    }
        break;

        // the alert message sended by DM server,users should do a choice,
        //if chosing no,devices should send th emessage to DM server
    case 1101: {
        VDM_MMI_ScreenContext_t inScreenContext;
        if (pContent->itemList->item->data) {
            char * temp_start = NULL;
            char * temp_end = NULL;
            char mindt[10] = { 0 };
            temp_start = xppStrstr(pContent->itemList->item->data->content,"MINDT=");
            temp_end = xppStrstr(pContent->itemList->item->data->content,"&");
            temp_start = temp_start + strlen("MINDT=");
            if (temp_start && temp_end) {
                xppMemcpy(mindt, temp_start, MIN(9,temp_end-temp_start));
                inScreenContext.minDisplayTime = atoi(mindt);
            }
            temp_start = NULL;
            temp_start = xppStrstr(pContent->itemList->item->data->content,"MAXDT=");
            if (temp_start) {
                inScreenContext.maxDisplayTime = atoi(
                        temp_start + strlen("MAXDT="));
            }

        }
        if (pContent->itemList->next) {
            inScreenContext.displayText =
                    pContent->itemList->next->item->data->content;
        }
        dmTaskComm_displayDialog(DM_ALERT_CONFIRM_DIALOG,
                inScreenContext.displayText, inScreenContext.maxDisplayTime);
        syncml_cb_message("MMIDM @@handle alert: 1101");
        if(status != NULL) {
            status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
        }
    }
        break;

        /*the notification sended by DM platform from DM server to Client , Client can input messages;
         This text message will be returned to  the DM server as a part of the state message. */
    case 1102: {
        syncml_cb_message("MMIDM @@handle alert: 1102");
    }
        break;

        //Alert from DM server to client,it contains many choices,but client could choose only one
    case 1103: {
        syncml_cb_message("MMIDM @@handle alert: 1103");
        //status->data=dm_smlString2Pcdata(__FILE__, __LINE__, "200");
    }
        break;

        //Alert from DM server to client,it contains many choices,Client could choose  one or more
    case 1104: {
        syncml_cb_message("MMIDM @@handle alert: 1104");
    }
        break;

        //normally it couldn't be sented,only the package of client  is not completed
    case 1222: {
        syncml_cb_message("MMIDM @@handle alert 1222@@alert for more");
        if(status != NULL) {
            status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
        }
    }
        break;

    case 1223: {
        syncml_cb_message("MMIDM @@handle alert 1223@@alert for server abort.finish process!");
        if(status != NULL) {
            status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
        }
        *dm_protocol_step = STEP_SYNC_ERROR;
    }
        break;

    default: {
        if(status != NULL) {
            status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "405");
        }
    }
        break;
    }

    dm_myAddstatustoCQ(status);

    return dm_smlFreeProtoElement(pContent);
}

/*******************************************************************/
//Effect:          No
//date  :         2011-3-14
//Function :    dm_myHandleget
/*******************************************************************/
short dm_myHandledelete(short id, void* userData, SmlDeletePtr_t pContent) {

    SmlStatusPtr_t status;
    syncml_cb_message("MMIDM @@handle dm_myHandledelete @@");

    status = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlStatus_t));
    if(status != NULL) {
        dm_smlLibMemset(status, 0, (long) sizeof(SmlStatus_t));
        status->elementType = SML_PE_STATUS;
        status->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                pContent->cmdID->content);
        status->cmd = dm_smlString2Pcdata(__FILE__, __LINE__, "Delete");
        status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
    }
    dm_myAddstatustoCQ(status);

    dm_task_set_ctrl_type(SYNC_CONTROL_CONTINUE);
    return dm_smlFreeProtoElement(pContent);

}

/*******************************************************************/
//Effect:          No
//date  :         2011-3-14
//Function :    dm_myHandleget
/*******************************************************************/
short dm_myHandleget(short id, void* userData, SmlGetPtr_t pContent) {

    char * locuri = NULL;
    char * data = NULL;
    int max_size = 0;
    SmlStatusPtr_t status = NULL;
    SmlTargetRefListPtr_t targetlist = NULL;
    SmlTargetRefListPtr_t targetlistall = NULL;
    SmlTargetRefListPtr_t targetlistnext = NULL;

    SmlSourcePtr_t source_uri = NULL;
    SmlMetInfMetInfPtr_t meta_uri = NULL;
    SmlItemListPtr_t itemList = NULL;
    SmlItemListPtr_t itemListall = NULL;
    SmlItemListPtr_t itemListnext = NULL;
    SmlItemPtr_t item_uri = NULL;
    SmlResultsPtr_t results = NULL;
    IBOOL ret = FALSE;
    SmlItemListPtr_t tmp = pContent->itemList;
    int head = 0;
    int headtarget = 0;
    int getItemErr = 1;

    syncml_cb_message(("MMIDM @@handle dm_myHandleget @@"));

// Modify liuhongxing for one command with multi-items

    status = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlStatus_t));
    if(status != NULL) {
        dm_smlLibMemset(status, 0, (long) sizeof(SmlStatus_t));
    }else {
        // No need to continue if status is NULL
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    status->elementType = SML_PE_STATUS;
    status->msgRef = dm_smlString2Pcdata(__FILE__, __LINE__,
            dm_status_cmd_que->msgref->content);
    status->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__,
            pContent->cmdID->content);
    status->cmd = dm_smlString2Pcdata(__FILE__, __LINE__, "Get");
    status->targetRefList = NULL;

    while (tmp != NULL && getItemErr == 1) {
        if (tmp->item) {
            syncml_cb_message("MMIDM @@handle dm_myHandleget tmp->item:%s", tmp->item->target->locURI->content);

            if (headtarget == 0) {
                targetlist = (SmlTargetRefListPtr_t) dm_smlLibMalloc(__FILE__,
                        __LINE__, (long) sizeof(SmlTargetRefList_t));
                if(targetlist != NULL) {
                    dm_smlLibMemset(targetlist, 0, (long) sizeof(SmlTargetRefList_t));
                    targetlist->targetRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                        tmp->item->target->locURI->content);
                    targetlist->next = NULL;
                    targetlistall = targetlist;
                    targetlistnext = targetlistall;
                    headtarget++;
                }
            } else {
                targetlist = (SmlTargetRefListPtr_t) dm_smlLibMalloc(__FILE__,
                        __LINE__, (long) sizeof(SmlTargetRefList_t));
                if(targetlist != NULL) {
                    dm_smlLibMemset(targetlist, 0, (long) sizeof(SmlTargetRefList_t));
                    targetlist->targetRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                        tmp->item->target->locURI->content);
                    targetlist->next = NULL;
                    targetlistnext->next = targetlist;
                    targetlistnext = targetlist;
                }
            }
        }

//     if(pContent->itemList->item)
        if (tmp->item) {
            locuri = tmp->item->target->locURI->content;
        }

        if (locuri) {
            //call the interface of native tree's management
            ret = dmTreeParam_getNodeData(locuri, &data, &max_size);
        }

        if (ret) {
             status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
        } else {
             status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "404");
             getItemErr = 0;
        }
//     dm_myAddstatustoCQ(status);

        results = (SmlResultsPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
                (long) sizeof(SmlResults_t));
        if(results  != NULL) {
            dm_smlLibMemset(results, 0, (long) sizeof(SmlResults_t));
            results->elementType = SML_PE_STATUS;
            results->msgRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                    dm_status_cmd_que->msgref->content);
            results->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                    pContent->cmdID->content);
        }
        if (head == 0) {
            itemListall = dm_smlLibMalloc(__FILE__, __LINE__,
                    sizeof(SmlItemList_t));
            if(itemListall != NULL) {
                dm_smlLibMemset(itemListall, 0, (long) sizeof(SmlItemList_t));
                itemListnext = itemListall;
                itemList = itemListall;
                head++;
            }
        } else {
            itemList = dm_smlLibMalloc(__FILE__, __LINE__,
                    sizeof(SmlItemList_t));
            if(itemList != NULL){
                dm_smlLibMemset(itemList, 0, (long) sizeof(SmlItemList_t));
                itemListnext->next = itemList;
                itemListnext = itemList;
            } else {
                tmp = tmp->next;
                continue;
            }
        }

        source_uri = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlSource_t));
        if(source_uri != NULL) {
        dm_smlLibMemset(source_uri, 0, (long) sizeof(SmlSource_t));
        //  source_uri->locURI=dm_smlString2Pcdata(__FILE__, __LINE__, pContent->itemList->item->target->locURI->content);
        if (tmp->item) {
            source_uri->locURI = dm_smlString2Pcdata(__FILE__, __LINE__,
                    tmp->item->target->locURI->content);
        } else {
           source_uri->locURI = NULL;
        }
        }
        meta_uri = dm_smlLibMalloc(__FILE__, __LINE__,
                sizeof(SmlMetInfMetInf_t));
        if(meta_uri != NULL) {
            dm_smlLibMemset(meta_uri, 0, sizeof(SmlMetInfMetInf_t));
            meta_uri->format = dm_smlString2Pcdata(__FILE__, __LINE__, "chr");
        }

        item_uri = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(SmlItem_t));
        if(item_uri != NULL) {
            dm_smlLibMemset(item_uri, 0, (long) sizeof(SmlItem_t));
            item_uri->source = source_uri;
            item_uri->meta = dm_smlmeta2extPcdata(__FILE__, __LINE__, meta_uri);
            item_uri->data = dm_smlString2Pcdata(__FILE__, __LINE__, data);
        }
        if(itemList != NULL) {
            itemList->item = item_uri;
            itemList->next = NULL;
        }
        tmp = tmp->next;
        if (NULL != data) {
            free(data);
            data = NULL;
        }
    }
//      results->itemList=itemList;
    if(results != NULL) {
        results->itemList = itemListall;
    }
    status->targetRefList = targetlistall;
    dm_myAddstatustoCQ(status);
    dm_myAddResultstoCQ(results);
    if (NULL != data) {
        free(data);
        data = NULL;
    }
    //server function
    dm_task_set_ctrl_type(SYNC_CONTROL_CONTINUE);
    return dm_smlFreeProtoElement(pContent);
}

short dm_myHandleput(short id, void* userData, SmlPutPtr_t pContent) {
    //server function
    return dm_smlFreeProtoElement(pContent);
}

#ifdef MAP_RECEIVE
short dm_myHandlemap(short id, void* userData, SmlMapPtr_t pContent) {
    //server function
    return dm_smlFreeProtoElement(pContent);
}
#endif

#ifdef RESULT_RECEIVE
short dm_myHandleresults(short id, void* userData, SmlResultsPtr_t pContent) {
    //server function
    //call the interface of native tree's management
    return dm_smlFreeProtoElement(pContent);
}
#endif

/*******************************************************************/
//Effect:          No
//date  :         2009-7-16
//Function :    dm_myHandlestatus
/*******************************************************************/
short dm_myHandlestatus(short id, void* userData, SmlStatusPtr_t pContent) {
    SmlStatusPtr_t status;
    //Need to intialise to 0 if statusdata is NULL
    //statuscode will become uninitialised
    int statuscode = 0;
    char* cmd;
    char* itemchar;
    unsigned int rec_id;
    SmlItemListPtr_t templist;
    //uint32 buffersize=1000;
    char* statusdata = NULL;
    char * status_auth_nextnonce = NULL;
    char * status_auth_type = NULL;

    status = pContent;

    statusdata = dm_smlPcdata2String(__FILE__, __LINE__, status->data);
    if(statusdata  != NULL) {
        statuscode = atoi(statusdata);
        dm_smlLibFree(statusdata);
    }
    cmd = dm_smlPcdata2String(__FILE__, __LINE__, status->cmd);
    syncml_cb_message("MMIDM @@status cmd is %s@@",cmd);
    syncml_cb_message("MMIDM @@handle status, status is %d@@",statuscode);

    if (status->chal) // SyncHdr
    {
        status_auth_type = dm_smlPcdata2String(__FILE__, __LINE__,
                ((SmlMetInfMetInf_t*) (status->chal->meta->content))->type);
        syncml_cb_message("MMIDM @@status_auth_type is %s@@",status_auth_type);

        //if (0 == xppStrcmp(status_auth_type, "syncml:auth-md5"))
        {
            status_auth_nextnonce =
                    dm_smlPcdata2String(__FILE__, __LINE__,
                            ((SmlMetInfMetInf_t*) (status->chal->meta->content))->nextnonce);
            if(status_auth_nextnonce  != NULL) {
                xppMemcpy(dm_task_relay_info->server_nextnonce,
                        status_auth_nextnonce, xppStrlen(status_auth_nextnonce));
                dm_task_relay_info->server_nextnonce[xppStrlen(status_auth_nextnonce)] =
                        0;
            }
            dm_smlLibFree(status_auth_nextnonce);
        }
    }

    switch (statuscode) {
    case 201:     //succeed
    case 200:     //data added succeed

        break;

    case 212:     //clients has the authority
    {
        if (0 == dm_task_relay_info->authored) {
            dm_task_relay_info->authored = 1;
        }
    }
        break;

    case 222:      //send next database package
    {
        syncml_http_message("MMIDM ** send next message of the package");
    }
        break;

    case 303:

        break;

        //4XX     the class of client's state
    case 401:      //client send the request of authority again
    {
        if (status->chal) // SyncHdr
        {
            if(status_auth_type != NULL) {
                if (0 == xppStrcmp(status_auth_type, "syncml:auth-md5")) {
                    syncml_cb_message("MMIDM ** dm_myHandlestatus  syncml:auth-md5 AUTH_MD5 authenticate failer");
                    *dm_protocol_step = STEP_CLIENT_REINIT;
                    dm_task_set_ctrl_type(SYNC_CONTROL_CONTINUE);
                } else {
                    syncml_cb_message("MMIDM ** dm_myHandlestatus  status_auth_type is error");
                    *dm_protocol_step = STEP_SYNC_ERROR;
                }
            }
        }
    }
        break;

        //exception states,need client sync and give the  relevant alert
    case 400:     //because of  grammar error,can't execute, sync failed

    case 407:     // please input the user name and password

    case 412:     // sync failed ,because of uncompleted sync order

    case 415:

        *dm_protocol_step = STEP_SYNC_ERROR;

        break;

    case 424:
        break;

        //exception states,need server sync and give the  relevant alert

    case 500:     //sync failed ,the server happened unknown error.

    case 501: // sync failed ,the server don't support the request of  client's

    case 503:     // the server is busy ,please try again latter

    case 505:       // sync failed , don't support SyncML DTD's version number

    case 511:      //sync failed , the server's bug

        *dm_protocol_step = STEP_SYNC_ERROR;

        break;

    default:  //default
        break;
    }

    if (status->chal) // SyncHdr
    {
        dm_smlLibFree(status_auth_type);
    }
    dm_smlLibFree(cmd);
    return dm_smlFreeProtoElement(pContent);
}

short dm_myHandlereplace(short id, void* userData, SmlReplacePtr_t pContent) {

    SmlStatusPtr_t status;
    SmlTargetRefListPtr_t targetlist = NULL;
    SmlTargetRefListPtr_t targetlistall = NULL;
    SmlTargetRefListPtr_t targetlistnext = NULL;
    SmlItemListPtr_t itemList = NULL;
    int replaceItemErr = 1;
    IBOOL ret = FALSE;

    syncml_cb_message("MMIDM @@handle dm_myHandlereplace @@");

    status = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(SmlStatus_t));

    SmlItemListPtr_t tmp = pContent->itemList;
    int head = 0;
//    if(pContent->itemList)
    while (tmp != NULL && replaceItemErr == 1) {

        if (head == 0) {
            targetlist = (SmlTargetRefListPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, (long) sizeof(SmlTargetRefList_t));
            if(targetlist != NULL) {
                dm_smlLibMemset(targetlist, 0, (long) sizeof(SmlTargetRefList_t));
                targetlist->targetRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                        tmp->item->target->locURI->content);
                targetlist->next = NULL;
                targetlistall = targetlist;
                targetlistnext = targetlistall;
                head++;
            }
        } else {
            targetlist = (SmlTargetRefListPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, (long) sizeof(SmlTargetRefList_t));
            if(targetlist != NULL) {
                dm_smlLibMemset(targetlist, 0, (long) sizeof(SmlTargetRefList_t));
                targetlist->targetRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                       tmp->item->target->locURI->content);
                targetlist->next = NULL;
                targetlistnext->next = targetlist;
                targetlistnext = targetlist;
            }
        }

        if (tmp->item) {
            syncml_cb_message("MMIDM @@handle dm_myHandleget tmp->item:%s", tmp->item->target->locURI->content);
        } else {
            syncml_cb_message("MMIDM @@handle dm_myHandleget tmp->item NULL");
            break;
        }
        if (tmp->item->data) {
            int content_len = 0;

            //don't get the length of string directly,because there may be "null" in server
            syncml_cb_message (" len is %d=null", tmp->item->data->length);
            if (tmp->item->data->length > 0) {
                content_len = xppStrlen(tmp->item->data->content);
            } else {
                content_len = tmp->item->data->length;
                syncml_cb_message ("%s=null",tmp->item->target->locURI->content);

            }
            ret = dmTreeParam_ReplaceNodeData(
                    tmp->item->target->locURI->content,
                    tmp->item->data->content, content_len);
            if (ret == FALSE) {
                replaceItemErr = 0;
            }
        }
        tmp = tmp->next;
    }
    if(status != NULL) {
        dm_smlLibMemset(status, 0, (long) sizeof(SmlStatus_t));
        status->targetRefList = targetlistall;
        status->elementType = SML_PE_STATUS;
        status->msgRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                dm_status_cmd_que->msgref->content);
        status->cmdRef = dm_smlString2Pcdata(__FILE__, __LINE__,
                pContent->cmdID->content);
        status->cmd = dm_smlString2Pcdata(__FILE__, __LINE__, "Replace");
        if (ret) {
            status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "200");
        } else {
            status->data = dm_smlString2Pcdata(__FILE__, __LINE__, "404");
        }
    }
    dm_myAddstatustoCQ(status);
    dm_task_set_ctrl_type(SYNC_CONTROL_CONTINUE);

    return dm_smlFreeProtoElement(pContent);

}

/**************************************************************/
//these callbacks are NOT included in the Toolkit lite version
/**************************************************************/
#ifdef COPY_RECEIVE
short dm_myHandlecopy(short id, void* userData, SmlCopyPtr_t param) {
    return dm_smlFreeProtoElement(param); // SML_ERR_OK; //  20080223
}
#endif

#ifdef EXEC_RECEIVE
short dm_myHandleexec(short id, void* userData, SmlExecPtr_t pContent) {
    return dm_smlFreeProtoElement(pContent); // SML_ERR_OK; //  20080223
}
#endif

#ifdef SEARCH_RECEIVE
short dm_myHandlesearch(short id, void* userData, SmlSearchPtr_t pContent)
{
    return dm_smlFreeProtoElement(pContent); // SML_ERR_OK; //  20080223
}
#endif

short dm_myHandleFinal(void) {
    syncml_cb_message("^@@^MMIDM dm_myHandleFinal !");
    *dm_protocol_step_priv = *dm_protocol_step;

    return SML_ERR_OK;
}

short dm_myHandlehandleError(short id, void* userData) {
    return SML_ERR_OK;
}

short dm_myHandletransmitChunk(short id, void* userData) {
    return SML_ERR_OK;
}

/*******************************************************************/
//Decr:           add the status in status command queue
//Effect:         No
//date  :         2011-3-14
//Function :    dm_myAddstatustoCQ
/*******************************************************************/
short dm_myAddstatustoCQ(SmlStatusPtr_t status) {
    status_element_type* temp = PNULL;
    status_element_type* temptr = PNULL;

    temp = (status_element_type*) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(status_element_type));
    if (NULL == temp) {
        //dm_smlFreeStatus(temptr->status);
        return SML_ERR_OK;
    }

    temp->status = status;

    temp->next = NULL;

    temptr = dm_status_cmd_que->queue;

    if (dm_status_cmd_que->totalnumber == 0) {
        dm_status_cmd_que->queue = temp;
        dm_status_cmd_que->totalnumber++;
        return SML_ERR_OK;
    }

    while (temptr->next != NULL ) {
        temptr = temptr->next;
    }

    temptr->next = temp;
    dm_status_cmd_que->totalnumber++;

    return SML_ERR_OK;
}

/*******************************************************************/
//Decr:           free the status in status of command queue
//Effect:          No
//date  :         2011-3-14
//Function :    dm_myFreestatusofCQ
/*******************************************************************/
short dm_myFreestatusofCQ(void) {
    status_element_type* temptr;
    status_element_type* next;
    temptr = dm_status_cmd_que->queue;
    next = temptr->next;
    while ((next != NULL )&&(dm_status_cmd_que->totalnumber>0)){
    temptr->next=next->next;  //break one node
    dm_smlFreeStatus(next->status);
    dm_smlLibFree(next);
    next=temptr->next;
    dm_status_cmd_que->totalnumber--;
}

    if (dm_status_cmd_que->totalnumber > 0) {
        dm_smlFreeStatus(temptr->status);
        dm_smlLibFree(temptr);
        dm_status_cmd_que->totalnumber--;
        dm_smlFreePcdata(__FILE__, __LINE__, dm_status_cmd_que->msgref);
    }
    return SML_ERR_OK;
}

/*******************************************************************/
//Decr:           add  the result to command queue
//Effect:          No
//date  :         2011-3-14
//Function :    dm_myAddResultsCQ
/*******************************************************************/
short dm_myAddResultstoCQ(SmlResultsPtr_t results) {
    results_element_type* temp = PNULL;
    results_element_type* temptr = PNULL;

    temp = (results_element_type*) dm_smlLibMalloc(__FILE__, __LINE__,
            (long) sizeof(results_element_type));
    if (NULL == temp) {
        //dm_smlFreeStatus(temptr->status);
        return SML_ERR_OK;
    }

    temp->results = results;

    temp->next = NULL;

    temptr = dm_results_cmd_que->queue;

    if (dm_results_cmd_que->totalnumber == 0) {
        dm_results_cmd_que->queue = temp;
        dm_results_cmd_que->totalnumber++;
        return SML_ERR_OK;
    }

    while (temptr->next != NULL ) {
        temptr = temptr->next;
    }

    temptr->next = temp;
    dm_results_cmd_que->totalnumber++;

    return SML_ERR_OK;
}

/*******************************************************************/
//Decr:           free the results of command queue
//Effect:          No
//date  :         2011-3-14
//Function :    dm_myFreeResultsofCQ
/*******************************************************************/
short dm_myFreeResultsofCQ(void) {
    results_element_type* temptr;
    results_element_type* next;
    temptr = dm_results_cmd_que->queue;
    next = temptr->next;
    while ((next != NULL )&&(dm_results_cmd_que->totalnumber>0)){
    temptr->next=next->next;  //break one node
    dm_smlFreeResults(next->results);
    dm_smlLibFree(next);
    next=temptr->next;
    dm_results_cmd_que->totalnumber--;
}

    if (dm_results_cmd_que->totalnumber > 0) {
        dm_smlFreeResults(temptr->results);
        dm_smlLibFree(temptr);
        dm_results_cmd_que->totalnumber--;
        dm_smlFreePcdata(__FILE__, __LINE__, dm_results_cmd_que->msgref);
    }
    return SML_ERR_OK;
}

#ifdef DM_LOG_FILE

/*******************************************************************/
//Decr  :         write the xml document in to storage
//Effect:          No
//date  :         2009-2-5
//Function :    dm_myPrintLogFiles
/*******************************************************************/
static void dm_myPrintLogFiles(unsigned int sessionID, unsigned int messageID, char* filename, char* buffer, uint32 length)
{
    MMIFILE_HANDLE file_handle = PNULL;
    uint32 opt_mode=0;
    uint32 charswrite = 0;
    uint16 path_filename_len = 0;
    uint16 path_filename[100] = {0};
    uint16 i = 0;
    uint16 offset = 0;
    uint32 free_space_high = 0;
    uint32 free_space_low = 0;

    uint16 path_name[] = {0x0044, 0x003A, 0x005C, 0x0044, 0x006D, 0x004C, 0x006F, 0x0067, 0x005C, 0x00, 0x00};

    dm_logFileCount++;
    syncml_cb_message("MMIDM ----dm_myPrintLogFiles----%d.xml----", dm_logFileCount);

    // resolve the question that if there are 1000 logs the system halted
    if(1000==dm_logFileCount)
    {
        dm_logFileCount = 0;
    }
    if (!MMIFILE_IsFolderExist((uint16 *)path_name, 10))
    {
        // make file
        MMIFILE_CreateDir((uint16 *)path_name, 10);
    }
    // get the name of new XML document
    dm_smlLibMemcpy(path_filename, path_name, 20);

    offset = 10;

    path_filename[offset] =(uint16)( 0x30 + (dm_logFileCount / 100);
            offset+=1;
            path_filename[offset] = (uint16)(0x30 + ((dm_logFileCount % 100) / 10);
                    offset+=1;
                    path_filename[offset] = (uint16)(0x30 + (dm_logFileCount % 10);
                            offset+=1;
                            path_filename[offset] = 0x5F;// '_'
                            offset+=1;

                            path_filename[offset] = (uint16)(0x30 + (sessionID / 100);
                                    offset+=1;
                                    path_filename[offset] = (uint16)(0x30 + ((sessionID % 100) / 10);
                                            offset+=1;
                                            path_filename[offset] = (uint16)(0x30 + (sessionID % 10);
                                                    offset+=1;
                                                    path_filename[offset] = 0x5F;// '_'
                                                    offset+=1;

                                                    path_filename[offset] = (uint16)(0x30 + (messageID / 100);
                                                            offset+=1;
                                                            path_filename[offset] =(uint16)(0x30 + ((messageID % 100) / 10);
                                                                    offset+=1;
                                                                    path_filename[offset] = (uint16)(0x30 + (messageID % 10);
                                                                            offset+=1;
                                                                            path_filename[offset] = 0x5F;// '_'
                                                                            offset+=1;

                                                                            for(i = 0; i < dm_smlLibStrlen(filename); i++)
                                                                            {
                                                                                path_filename[offset] = filename[i];
                                                                                offset+=1;
                                                                            }

                                                                            path_filename[offset] = 0x2E; // '.'
                                                                            offset+=1;
                                                                            path_filename[offset] = 0x78;// 'x'
                                                                            offset+=1;
                                                                            path_filename[offset] = 0x6D;// 'm'
                                                                            offset+=1;
                                                                            path_filename[offset] = 0x6C;// 'l'

                                                                            if (MMIFILE_IsFileExist((uint16 *)path_filename, path_filename_len))
                                                                            {
                                                                                MMIFILE_DeleteFile((uint16 *)path_filename, NULL);
                                                                            }

                                                                            MMIFILE_GetDeviceFreeSpace((uint16 *)MMIFILE_DEVICE_UDISK,MMIFILE_DEVICE_UDISK_LEN,&free_space_high, &free_space_low);

                                                                            if(length < free_space_low || 0 != free_space_high)
                                                                            {
                                                                                opt_mode = SFS_MODE_CREATE_NEW | SFS_MODE_WRITE;/*lint !e655*/
                                                                                file_handle= MMIFILE_CreateFile((uint16 *)path_filename,
                                                                                        opt_mode,
                                                                                        PNULL,
                                                                                        PNULL
                                                                                );
                                                                                MMIFILE_WriteFile(file_handle, buffer, length, &charswrite, NULL);
                                                                                MMIFILE_CloseFile(file_handle);
                                                                            }
                                                                        }

#endif

/*******************************************************************/
//Effect:          No
//date  :         2009-7-10
//Function :    dm_myPrintbuffer
/*******************************************************************/
void dm_myPrintbuffer(char *pbuffer, int buf_lenth) {
#ifdef FEATURE_DM_DEBUG

    int i = 0;
    syncml_efs_message("MMIDM &&&buf_lenth=%d",buf_lenth);
    for (i = 0; i < 60; i++) //  20080129 15-->20
            {
        char temp_print_buffer[400] = { 0 };
        dm_smlLibMemcpy(temp_print_buffer, pbuffer + i * 230, 230);
        syncml_efs_message("MMIDM &&& content=%s",temp_print_buffer); //  20080129 250-->229
    }
#endif
    return;
}

/*******************************************************************/
//Decr  :          print the created XML or WBXML document to  output.xml
//Export:        null
//date  :         2005-5-10
//Function : dm_myPrintxmlorwbxml
/*******************************************************************/
void dm_myPrintxmlorwbxml(char* filename, short id) {

#ifdef FEATURE_DM_DEBUG
    WsmSmGlobals_t wsmsmtemp;
    uint16 offset = 0;
    syncml_efs_message("MMIDM &&& step=%s,id=%d ",filename,id);
    /*Start of   2009.9.22 for if encodetype=wbxml,not print */
    if (dm_task_relay_info->encodetype != SML_XML) {
        return;
    }
    /*End   of   2009.9.22 for if encodetype=wbxml,not print */
    if (dm_pGlobalAnchor != NULL ) {
        wsmsmtemp = dm_pGlobalAnchor->wsmGlobals->wsmSm;
        while (wsmsmtemp->memH != id) {
            wsmsmtemp = wsmsmtemp->next;
            if (wsmsmtemp == NULL ) {
                syncml_efs_message("error occurs! reason: wrong wsmglobal id");
                return;
            }
        }
        offset = id - 1;
        if (offset < MAX_WSM_BUFFERS) {
            dm_myPrintbuffer(wsmsmtemp->winH,
                    dm_pGlobalAnchor->wsmGlobals->wsmBuf[offset].usedBytes);
        }
#ifdef DM_LOG_FILE
        dm_myPrintLogFiles(dm_task_relay_info->sessionID, dm_task_relay_info->messageID - 1, filename, wsmsmtemp->winH,dm_pGlobalAnchor->wsmGlobals->wsmBuf[id-1].usedBytes);
#endif
    }
#endif
    return;
}

/*******************************************************************/
//Decr  :          print the interactived http document
//Export:        null
//date  :         2006-6-6
//Function : dm_myPrintxmlorwbxml
/*******************************************************************/
void dm_myhttplogs(char* buffer, uint32 length) {
#ifdef FEATURE_DM_DEBUG
    syncml_efs_message("MMIDM &&& dm_myhttplogs");
    if (buffer != NULL ) {
        dm_myPrintbuffer((char*) buffer, length);
    }
#endif
    return;
}
