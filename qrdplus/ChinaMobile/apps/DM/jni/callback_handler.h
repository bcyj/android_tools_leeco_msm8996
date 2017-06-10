#ifndef  HEADER_FILE_CALLBACK_HANDLER
#define  HEADER_FILE_CALLBACK_HANDLER

#include "metainfodtd.h"
#include "devinfodtd.h"
#include "smldtd.h"
#include  "sci_types.h"

#ifndef NULL                        // a NULL pointer
#define NULL (void*) 0
#endif

typedef struct status_element_type {
    SmlStatusPtr_t status;
    struct status_element_type* next;
} status_element_type;

typedef struct {
    status_element_type* queue;
    long totalnumber;
    SmlPcdataPtr_t msgref;
} status_cmd_queue_type;

typedef struct results_element_type {
    SmlResultsPtr_t results;
    struct results_element_type* next;
} results_element_type;

typedef struct {
    results_element_type* queue;
    long totalnumber;
    SmlPcdataPtr_t msgref;
} results_cmd_queue_type;

short dm_smlFreeProtoElement(void * pProtoElement);

void dm_smlFreePcdata(const char * file, int line, SmlPcdataPtr_t pPcdata);

void dm_smlFreeSourceTargetPtr(SmlSourcePtr_t pSourceTarget);

void dm_smlFreeCredPtr(SmlCredPtr_t pCred);

void dm_smlFreeSyncHdr(SmlSyncHdrPtr_t pSyncHdr);

void dm_smlFreeSync(SmlSyncPtr_t pSync);

void dm_smlFreeItemPtr(SmlItemPtr_t pItem);

void dm_smlFreeItemList(SmlItemListPtr_t pItemList);

void dm_smlFreeGeneric(SmlGenericCmdPtr_t pGenericCmd);

void dm_smlFreeAlert(SmlAlertPtr_t pAlert);

void dm_smlFreeStatus(SmlStatusPtr_t pStatus);

void dm_smlFreeChalPtr(SmlChalPtr_t pChal);

void dm_smlFreeGetPut(SmlPutPtr_t pGetPut);

#if (defined ATOMIC_RECEIVE || defined SEQUENCE_RECEIVE)
void dm_smlFreeAtomic(SmlAtomicPtr_t pAtomic);
#endif

#if (defined EXEC_SEND || defined EXEC_RECEIVE)
void dm_smlFreeExec(SmlExecPtr_t pExec);
#endif

#ifdef MAP_RECEIVE
void dm_smlFreeMap(SmlMapPtr_t pMap);
void dm_smlFreeMapItemList(SmlMapItemListPtr_t pMapItemList);
void dm_smlFreeMapItemPtr(SmlMapItemPtr_t pMapItem);
#endif

#ifdef RESULT_RECEIVE
void dm_smlFreeResults(SmlResultsPtr_t pResults);
#endif

SmlPcdataPtr_t dm_concatPCData(SmlPcdataPtr_t pDat1,
        const SmlPcdataPtr_t pDat2);

#ifdef __USE_METINF__
void dm_smlFreeMetinfAnchor(SmlMetInfAnchorPtr_t data);
void dm_smlFreeMetinfMem(SmlMetInfMemPtr_t data);
void dm_smlFreeMetinfMetinf(SmlMetInfMetInfPtr_t data);
#endif

SmlPcdataPtr_t dm_smlString2Pcdata(const char * file, int line, char* str);

SmlPcdataPtr_t dm_smlmeta2extPcdata(const char * file, int line,
        SmlMetInfMetInfPtr_t meta);

SmlPcdataPtr_t dm_smldevinfo2extPcdata(const char * file, int line,
        SmlDevInfDevInfPtr_t devinfo);

char* dm_smlPcdata2String(const char * file, int line, SmlPcdataPtr_t pcdata);

char* dm_smlunsignedInt2String(const char * file, int line,
        unsigned int integer);

unsigned int dm_smlString2unsignedInt(char* str);

/***********************************************************/

short dm_myHandleStartMessage(short id, void* userData,
        SmlSyncHdrPtr_t pSyncHdr);

short dm_myHandleEndMessage(short id, void* userData, unsigned char final);

short dm_myHandleStartSync(short id, void* userData, SmlSyncPtr_t pSync);

short dm_myHandleEndSync(short id, void* userData);

#ifdef ATOMIC_RECEIVE  /* these callbacks are NOT included in the Toolkit lite version */
short dm_myHandlestartAtomic(short id, void * userData,
        SmlAtomicPtr_t pContent);

short dm_myHandleendAtomic(short id, void * userData);
#endif

#ifdef SEQUENCE_RECEIVE
short dm_myHandlestartSequence(short id, void * userData,
        SmlSequencePtr_t pContent);

short dm_myHandleendSequence(short id, void * userData);
#endif

short dm_myHandleAdd(short id, void* userData, SmlAddPtr_t pContent);

short dm_myHandlealert(short id, void* userData, SmlAlertPtr_t pContent);

short dm_myHandledelete(short id, void* userData, SmlDeletePtr_t pContent);

short dm_myHandleget(short id, void* userData, SmlGetPtr_t pContent);

short dm_myHandleput(short id, void* userData, SmlPutPtr_t pContent);

#ifdef MAP_RECEIVE
short dm_myHandlemap(short id, void* userData, SmlMapPtr_t pContent);
#endif

#ifdef RESULT_RECEIVE
short dm_myHandleresults(short id, void* userData, SmlResultsPtr_t pContent);
#endif

short dm_myHandlestatus(short id, void* userData, SmlStatusPtr_t pContent);

short dm_myHandlereplace(short id, void* userData, SmlReplacePtr_t pContent);

#ifdef COPY_RECEIVE  /* these callbacks are NOT included in the Toolkit lite version */
short dm_myHandlecopy(short id, void* userData, SmlCopyPtr_t param);
#endif

#ifdef EXEC_RECEIVE
short dm_myHandleexec(short id, void* userData, SmlExecPtr_t pContent);
#endif

#ifdef SEARCH_RECEIVE
short dm_myHandlesearch(short id, void* userData, SmlSearchPtr_t pContent);
#endif

short dm_myHandleFinal(void);

short dm_myHandlehandleError(short id, void* userData);

short dm_myHandletransmitChunk(short id, void* userData);

short dm_myAddstatustoCQ(SmlStatusPtr_t status);

short dm_myFreestatusofCQ(void);

short dm_myAddResultstoCQ(SmlResultsPtr_t results);

short dm_myFreeResultsofCQ(void);

/*****************************************************************/
void dm_myPrintxmlorwbxml(char* filename, short id);
void dm_myhttplogs(char* buffer, uint32 length);
void dm_myPrintbuffer(char *pbuffer, int buf_lenth);

#endif
