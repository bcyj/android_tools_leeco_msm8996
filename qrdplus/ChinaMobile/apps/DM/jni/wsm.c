#ifdef WIN32
#include "std_header.h"
#endif
#define _WSM_C_

#include "string.h"
#include "libmem.h"
#include "smlcore.h"
#include "smlerr.h"

/** WSM buffer flags */
#define  WSM_VALID_F     (unsigned char) 0x01
#define  WSM_LOCKED_F    (unsigned char) 0x02

#define  WSM_MEMH_UNUSED   -1
#define  INDEX_INVALID   -1

/* defines for convient use of global anchor */
#define wsmRet          dm_pGlobalAnchor->wsmGlobals->wsmRet
#define initWasCalled   dm_pGlobalAnchor->wsmGlobals->initWasCalled
#define maxWsmAvailMem  dm_pGlobalAnchor->syncmlOptions->maxWorkspaceAvailMem
#define wsmBuf          dm_pGlobalAnchor->wsmGlobals->wsmBuf
#define wsmIndex        dm_pGlobalAnchor->wsmGlobals->wsmIndex
#define wsmSm                  dm_pGlobalAnchor->wsmGlobals->wsmSm

#define freeDataStructs() dm_smlLibFree((dm_pGlobalAnchor->wsmGlobals))

static short dm_lookup(short memH);

/***********************static basic functions***************************/
void dm_createDataStructs() {
    if (dm_pGlobalAnchor->wsmGlobals == NULL) {
        if ((dm_pGlobalAnchor->wsmGlobals = dm_smlLibMalloc(__FILE__, __LINE__,
                sizeof(WsmGlobals_t))) == 0) {
            return;
        }
        dm_smlLibMemset(dm_pGlobalAnchor->wsmGlobals, 0, sizeof(WsmGlobals_t));
        wsmRet = 0;
        initWasCalled = 0;
        wsmIndex = 0;
        wsmSm = NULL;
    }
}

/**
 * FUNCTION:
 * Searches an element with the given InstanceID in the list
 *
 * IN:        InstanceID_t
 *            ID of the InstanceInfo structure to be retrieved
 *
 * RETURN:    InstanceInfoPtr_t
 *            Pointer to the InstanceInfo structure with the given ID
 *            NULL, if no InstanceInfo with the given ID has been found
 */
InstanceInfoPtr_t dm_findInfo(short id) {

    InstanceInfoPtr_t _pTmp;                // A helper pointer

    /* go through the list until end */
    for (_pTmp = dm_pGlobalAnchor->instanceListAnchor; _pTmp != NULL ; _pTmp =
            _pTmp->nextInfo) {
        if (_pTmp->id == id) {
            return _pTmp;              // STOP, we've found the info, return!
        }
    }
    return NULL ;                            // Info was not found, return NULL
}

/**
 * FUNCTION:
 * Adds a new element to the list
 *
 * IN:        InstanceInfoPtr_t
 *            pointer to the structure to be be added to list
 *
 * RETURN:    Return value,
 *            SML_ERR_OK if element was added successfully
 */
short dm_addInfo(InstanceInfoPtr_t pInfo) {

    if (pInfo != NULL) {
        InstanceInfoPtr_t _pTmp;

        /* Remember old beginning of the list */
        _pTmp = dm_pGlobalAnchor->instanceListAnchor;

        /* insert element immediately after anchor */
        dm_pGlobalAnchor->instanceListAnchor = pInfo; // anchor of list points now to new info element
        pInfo->nextInfo = _pTmp;    // Next info element is the prior first one.
        return SML_ERR_OK;

    } else {                     // Invalid InstanceInfo pointer was used (NULL)

        return SML_ERR_MGR_INVALID_INSTANCE_INFO;
    }

}

/**
 * FUNCTION:
 * Removes an element with the given InstanceID from the list
 *
 * IN:        InstanceID_t
 *            ID of the InstanceInfo structure to be removed
 *
 * RETURN:    Return value,
 *            SML_ERR_OK if element was removed successfully
 */
short dm_removeInfo(short id) {

    InstanceInfoPtr_t _pTmp;               // A helper pointer
    InstanceInfoPtr_t _pRemember;          // A helper pointer

    /* Remember current anchor */
    _pRemember = dm_pGlobalAnchor->instanceListAnchor;

    /* special check, if list is empty */
    if (_pRemember == NULL) {
        syncml_task_message("MMIDM dm_removeInfo 1");
        return SML_ERR_MGR_INVALID_INSTANCE_INFO;
    }

    /* special check, if first element should be removed */
    if (_pRemember->id == id) {
        // It's the first element, update anchor!
        dm_pGlobalAnchor->instanceListAnchor = _pRemember->nextInfo;
        syncml_task_message("MMIDM dm_removeInfo 2");
        return SML_ERR_OK;                    // return
    }

    /* go through the list until end */
    for (_pTmp = _pRemember->nextInfo; _pTmp != NULL ; _pTmp =
            _pTmp->nextInfo) {
        if (_pTmp->id == id)                  // STOP, we've found the info
                {
            _pRemember->nextInfo = _pTmp->nextInfo;
            syncml_task_message("MMIDM dm_removeInfo 3");
            return SML_ERR_OK;                  // return

        } else {

            _pRemember = _pTmp;                   // update helper pointer
        }
    }

    syncml_task_message("MMIDM dm_removeInfo 4");
    return SML_ERR_MGR_INVALID_INSTANCE_INFO;  // Info wasn't found

}

/**
 * Get next free buffer entry.
 * Returns index of next free entry, or -1 if buffer table is full.
 */
short dm_getNextFreeEntry() {
    short i;

    for (i = 0; i < MAX_WSM_BUFFERS; ++i)
        if (wsmBuf[i].memH == WSM_MEMH_UNUSED)
            return i;

    return -1;
}

unsigned char dm_isMemAvailable(long memToAlloc) {
    int i;
    long actMem = memToAlloc;
    if (maxWsmAvailMem == 0)
        return 1;  // no memsize restrictions
    for (i = 0; i < MAX_WSM_BUFFERS; ++i) {
        if (wsmBuf[i].memH != WSM_MEMH_UNUSED)
            actMem += wsmBuf[i].size;
    }
    return ((unsigned char) (actMem <= maxWsmAvailMem));
}

/**
 * Reset values in buffer table for entry memH.
 * If memH doesn't exist create an entry.
 * Return index to memH in buffer table,
 * or -1 if table is full
 */
short dm_resetBufferGlobals(short memH) {
    if ((wsmIndex = dm_lookup(memH)) < 0) {
        // create new one
        if ((wsmIndex = dm_getNextFreeEntry()) < 0)
            return -1;  // buffer table full
        wsmBuf[wsmIndex].memH = memH;
    } else
        // use existing one, which has to be reset prior usage
        dm_smlLibFree(wsmBuf[wsmIndex].bufName);   // free mem

    // reset the values
    wsmBuf[wsmIndex].pFirstFree = NULL;
    wsmBuf[wsmIndex].pFirstData = NULL;
    wsmBuf[wsmIndex].size = 0;
    wsmBuf[wsmIndex].usedBytes = 0;
    wsmBuf[wsmIndex].flags = WSM_VALID_F;
    wsmBuf[wsmIndex].bufName = NULL;

    return wsmIndex;
}

/**
 * Get buffer table index for memH.
 * Returns -1 if memH not found.
 */
static short dm_lookup(short memH) {
    short i;

    // first check cache
    if (wsmBuf[wsmIndex].memH == memH)
        return wsmIndex;

    // search through buffer
    for (i = 0; (i < MAX_WSM_BUFFERS) && (wsmBuf[i].memH != memH); ++i) {
        syncml_core_message("dm_lookup  wsmBuf[%d].memH=%d",i,wsmBuf[i].memH );
    };
    if (i < MAX_WSM_BUFFERS) {
        wsmIndex = i;
        return i;
    } else {
        return -1;     // memH not found
    }
}

/**
 * Find memory handle corresponding to name.
 * Return WSM_MEMH_UNUSED, if name not found in wsmBuf.
 */
short dm_nameToHandle(char* name) {
    int i;

    // first check cache
    if ((wsmBuf[wsmIndex].bufName != NULL )
            && (dm_smlLibStrcmp(wsmBuf[wsmIndex].bufName, name) == 0))
        return wsmBuf[wsmIndex].memH;

    // search through buffer
    for (i = 0;
            ((i < MAX_WSM_BUFFERS)
                    && (wsmBuf[i].bufName == NULL ?
                            1 : dm_smlLibStrcmp(wsmBuf[i].bufName, name) != 0));
            ++i)
        ;
    if (i < MAX_WSM_BUFFERS)
        return wsmBuf[i].memH;
    else {
        return WSM_MEMH_UNUSED;     // name not found
    }
}

unsigned char dm_isValidMemH(short memH) {
    short wsm_index = dm_lookup(memH);
    if (wsm_index >= 0 && wsm_index < MAX_WSM_BUFFERS) {
        return (unsigned char) (wsmBuf[wsm_index].flags & WSM_VALID_F );

    } else {
        return 0;
    }
}

unsigned char dm_isLockedMemH(short memH) {
    short wsm_index = dm_lookup(memH);
    if (wsm_index >= 0 && wsm_index < MAX_WSM_BUFFERS) {
        return (unsigned char) (wsmBuf[wsm_index].flags & WSM_LOCKED_F );

    } else {
        return 0;
    }
}

/**
 * search for buffer with memHandle memH and return pointer to it in p.
 * return == 0 if not found; 1 if found
 */
// luz %%% NOTE: called only from routines which lock the toolkit already,
//               no separate lock required here
unsigned char dm_locateH(short memH, smWinList_t **p) {
    *p = wsmSm;
    while ((*p != NULL ) && ((*p)->memH != memH)) {
        *p = (*p)->next;
    }
    if (*p == NULL)
        return 0;
    else
        return 1;
}

/**
 * Delete memory handle from buffer.
 * Return -1, if handle not found.
 */
short dm_deleteBufferHandle(short memH) {
    if ((wsmIndex = dm_lookup(memH)) < 0)
        return -1;  // handle not found

    // reset the values
    wsmBuf[wsmIndex].memH = WSM_MEMH_UNUSED;
    wsmBuf[wsmIndex].pFirstFree = NULL;
    wsmBuf[wsmIndex].pFirstData = NULL;
    wsmBuf[wsmIndex].size = 0;
    wsmBuf[wsmIndex].usedBytes = 0;
    wsmBuf[wsmIndex].flags = ((unsigned char) ~WSM_VALID_F );
    dm_smlLibFree(wsmBuf[wsmIndex].bufName);   // free mem
    wsmBuf[wsmIndex].bufName = NULL;

    return 0;
}

/************************sm functions!*****************************/
/**
 * FUNCTION: dm_smCreate
 *
 * Creates a new memory block with name memName and size memSize.
 *
 * PRE-Condition:   OS does not know memName; memSize > 0
 *
 * POST-Condition:  memName exists with size memSize;
 *                  memH refers to new memory block.
 *
 * IN:      memName
 *          Name of new memory block
 * IN:      memSize
 *          Size of new memory block
 *
 * OUT:     memH
 *          Handle to new memory block
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_USAGE, if memName is already known to the OS
 *          SML_ERR_INVALID_SIZE, if memSize <= 0
 *          SML_ERR_NOT_ENOUGH_SPACE, if available memory < memSize
 *
 * @see  dm_smDestroy
 */
short dm_smCreate(char* memName, long memSize, short *memH) {
    smWinList_t *pEle;     // pointer to new buffer

    if (memSize <= 0) {
        syncml_task_message("MMIDM dm_smCreate -- 1");
        return SML_ERR_INVALID_SIZE;
    }
    if (dm_locateEle(memName, &pEle)) {
        syncml_task_message("MMIDM  dm_smCreate -- 2");
        return SML_ERR_WRONG_USAGE;
    }

    // create new element in buffer list
    if (!dm_newListEle(memName, &pEle, memH)) {
        syncml_task_message("MMIDM dm_smCreate -- 3");
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    // create memory
    if (pEle->winH) {
        dm_smlLibFree(pEle);
        syncml_task_message("MMIDM  dm_smCreate,the memory is not pnull,if malloc onece ,will lead memory leak!!!");

    }
    if ((pEle->winH = dm_smlLibMalloc(__FILE__, __LINE__, memSize)) == 0) {
        dm_smlLibFree((pEle->memName));
        dm_smlLibFree(pEle);
        syncml_task_message("MMIDM  dm_smCreate -- 4");
        return SML_ERR_NOT_ENOUGH_SPACE;
    } else //add   for init memory
    {
        syncml_message("MMIDM  init buffer memory!addr=0x%x",pEle->winH);
        dm_smlLibMemset(pEle->winH, 0, memSize);
    }
    // set new values
    pEle->locked = 0;
    pEle->memH = *memH;
    pEle->memSize = memSize;
    pEle->next = NULL;

    syncml_task_message("MMIDM  dm_smCreate -- 5");
    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_smOpen
 *
 * Open connection to memory block with name memName.
 *
 * PRE-Condition:   OS does know memName
 *
 * POST-Condition:  memH refers to memory block memName
 *
 * IN:      memName
 *          Name of memory block to open<BR>
 *          Windows version: Name is ignored
 *
 * OUT:     memH
 *          Handle to opened memory block
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_PARAM, if memName is unknown
 *
 * @see  dm_smClose
 */
short dm_smOpen(char* memName, short *memH) {
    smWinList_t *pEle;     // pointer to buffer element

    if (!dm_locateEle(memName, &pEle)) {
        return SML_ERR_WRONG_PARAM;
    }

    *memH = pEle->memH;
    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_smClose
 *
 * Close link to memory block.
 *
 * PRE-Condition:   memH is a valid memory block handle; memH is unlocked;
 *                  no pointers to records are in use
 *
 * POST-Condition:  memH is not valid anymore
 *
 * IN:      memH
 *          Handle to close
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_USAGE, if memH is locked or unknown
 *
 * @see  dm_smOpen
 */
short dm_smClose(short memH) {
    smWinList_t *pEle;     // pointer to buffer element
    syncml_task_message("MMIDM  dm_smClose , memH=%d", memH);
    if (!dm_locateH(memH, &pEle)) {
        return SML_ERR_WRONG_USAGE;
    }
    syncml_task_message("MMIDM _mem  dm_smClose ,free  winH!addr=0x%x ",pEle->winH );

    // reset handle
    dm_smlLibFree((pEle->winH));
    pEle->memH = 0;
    pEle->locked = 0;
    pEle->memSize = 0;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_smDestroy
 *
 * Remove memory block memName within OS.
 *
 * PRE-Condition:   memName is a valid memory block name;
 *                  memory block is not in use (i.e. no handles and
 *                  pointers to this memory block are in use)
 *
 * POST-Condition:  memName is not a valid memory block name anymore
 *
 * IN:      memName
 *          Name of memory block to remove
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_PARAM, if memName is unknown
 *          SML_ERR_WRONG_USAGE, if memory block is still locked
 *
 * @see  dm_smCreate
 */
short dm_smDestroy(char* memName) {
    smWinList_t *pEle;     // pointer to buffer element

    if (!dm_locateEle(memName, &pEle)) {
        return SML_ERR_WRONG_PARAM;
    }
    if (pEle->locked) {
        return SML_ERR_WRONG_USAGE;
    }

    // remove memory buffer
    dm_removeEle(memName);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_smSetSize
 *
 * Set size of memory block memH to newSize.
 *
 * PRE-Condition:   memH is a valid handle; newSize > 0;
 *                  memory block is unlocked
 *
 * POST-Condition:  memory block size = newSize
 *
 * IN:      memH
 *          Handle to memory block
 * IN:      newSize
 *          New size of memory block
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_PARAM, if memH is unknown
 *          SML_ERR_WRONG_USAGE, if memH is locked
 *          SML_ERR_INVALID_SIZE, if newSize <= 0
 *          SML_ERR_NOT_ENOUGH_SPACE, if available memory < newSize
 *
 * @see  smGetSize
 */
short dm_smSetSize(short memH, long newSize) {
    smWinList_t *pEle;     // pointer to buffer element

    if (!dm_locateH(memH, &pEle)) {
        return SML_ERR_WRONG_PARAM;
    }
    if (pEle->locked) {
        return SML_ERR_WRONG_USAGE;
    }
    if (newSize <= 0) {
        return SML_ERR_INVALID_SIZE;
    }
    syncml_task_message("MMIDM _mem  dm_smSetSize ,free  winH!addr=0x%x ",pEle->winH );

    dm_smlLibFree((pEle->winH));
    if ((pEle->winH = dm_smlLibMalloc(__FILE__, __LINE__, newSize)) == 0) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    pEle->memSize = newSize;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_smLock
 *
 * Map memory block memH to local address space.
 *
 * PRE-Condition:   memH is a valid handle; memory block is not locked
 *
 * POST-Condition:  pMem points to memory block memH;
 *                  memory block is locked
 *
 * IN:      memH
 *          Handle to memory block
 *
 * OUT:     pMem
 *          Pointer to memory block memH mapped in local address space
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_PARAM, if memH is unknown
 *          SML_ERR_WRONG_USAGE, if memH was already locked
 *          SML_ERR_UNSPECIFIC, if lock failed
 *
 * @see  dm_smUnlock
 */
short dm_smLock(short memH, unsigned char **pMem) {
    smWinList_t *pEle;     // pointer to buffer element

    if (!dm_locateH(memH, &pEle)) {
        return SML_ERR_WRONG_PARAM;
    }
    if (pEle->locked) {
        return SML_ERR_WRONG_USAGE;
    }

    *pMem = (unsigned char*) pEle->winH;
    pEle->locked = 1;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_smUnlock
 *
 * Free pointer mapped to memH memory block.
 *
 * PRE-Condition:   memH is a valid handle; memory block is locked
 *
 * POST-Condition:  memory block is unlocked
 *
 * IN:      memH
 *          Handle to memory block
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_PARAM, if memH is unknown
 *          SML_ERR_WRONG_USAGE, if memH was already unlocked
 *          SML_ERR_UNSPECIFIC, if unlock failed
 *
 * @see  dm_smLock
 */
short dm_smUnlock(short memH) {
    smWinList_t *pEle;     // pointer to buffer element

    if (!dm_locateH(memH, &pEle)) {
        return SML_ERR_WRONG_PARAM;
    }
    if (!pEle->locked) {
        return SML_ERR_WRONG_USAGE;
    }

    pEle->locked = 0;

    return SML_ERR_OK;
}

/**********************sm static basic functions*******************/

/** create new buffer element and assign name to it
 * return pointer to new element and handle of new element
 */
/*
 SCTSTK - 16/03/2002 S.H. 2002-04-05 : fixed so that it works even if the sequence of buffer termination
 is not in the reverse order of buffer creation
 */

// luz %%% NOTE: called only from routines which lock the toolkit already,
//               no separate lock required here
unsigned char dm_newListEle(const char *name, smWinList_t **newEle,
        short *newHandle) {
    smWinList_t *p;
    short i;
    for (i = 0; *newHandle < MAX_WSM_BUFFERS && wsmBuf[i].memH != -1; ++i) {
    };

    if (i == MAX_WSM_BUFFERS)
        return 0;
    *newHandle = (short) (i + 1);

    if (((*newEle) = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(smWinList_t)))
            == 0)
        return 0;  // no more memory
    dm_smlLibMemset(*newEle, 0, sizeof(smWinList_t));
    if (((*newEle)->memName = dm_smlLibMalloc(__FILE__, __LINE__,
            strlen(name) + 1)) == 0) {
        dm_smlLibFree((*newEle));
        return 0;  // no more memory
    }

    memcpy((*newEle)->memName, name, strlen(name));
    (*newEle)->memName[strlen(name)] = '\0';
    if (wsmSm == 0)
        wsmSm = *newEle;
    else {
        p = wsmSm;
        while (p->next != NULL )
            p = p->next;
        p->next = *newEle;
    }
    return 1;
}

/**
 * search for buffer with name eleName and return pointer to it in p.
 * return == 0 if not found; 1 if found
 */
// luz %%% NOTE: called only from routines which lock the toolkit already,
//               no separate lock required here
unsigned char dm_locateEle(const char *eleName, smWinList_t **p) {
    *p = wsmSm;
    while ((*p != NULL ) && (strcmp((*p)->memName, eleName) != 0)) {
        *p = (*p)->next;
    }
    if (*p == NULL)
        return 0;
    else
        return 1;
}

/**
 * remove buffer with name eleName from smWinList.
 */
// luz %%% NOTE: called only from routines which lock the toolkit already,
//               no separate lock required here
void dm_removeEle(const char *eleName) {
    smWinList_t *act, *old;

    old = act = wsmSm;
    while ((act != NULL ) && (strcmp(act->memName, eleName) != 0)) {
        old = act;
        act = act->next;
    }
    if (act != NULL) {
        if (old == act)   // delete first list ele
            wsmSm = act->next;
        else
            old->next = act->next;
        dm_smlLibFree((act->memName));
        dm_smlLibFree(act);
    }
}

/******************************wsm functions****************************/

/**
 * FUNCTION: dm_wsmInit
 *
 * Initializes all Workspace Manager related resources.<BR>
 * Should only be called once!
 *
 * PRE-Condition:   This is the first function call to WSM
 *
 * POST-Condition:  All WSM resources are initialized
 *
 * IN:      wsmOpts
 *          WSM options, valid options are:
 *          <UL>
 *          <LI> maxAvailMem<BR>
 *               Maximal amount of memory which wsm can use for the buffers<BR>
 *               0 == no limitation
 *          </UL>
 *
 * OUT:     wsmH
 *          Handle to new buffer
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_OPTIONS, if wsmOpts is not valid
 *          SML_ERR_NOT_ENOUGH_SPACE, if not enough available memory
 *          SML_ERR_WRONG_USAGE, if dm_wsmInit was already called
 */
short dm_wsmInit(const WsmOptions_t *wsmOpts) {
    int i;

    // create global datastructs
    dm_createDataStructs();

    if (NULL == dm_pGlobalAnchor->wsmGlobals) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    // check if init was already called
    if (initWasCalled)
        return SML_ERR_WRONG_USAGE;

    // check options
    if (wsmOpts != NULL) {
        if (wsmOpts->maxAvailMem > 0) {
            maxWsmAvailMem = wsmOpts->maxAvailMem;
        }
    }

    // init resources
    for (i = 0; i < MAX_WSM_BUFFERS; ++i)
        wsmBuf[i].memH = WSM_MEMH_UNUSED;
    wsmIndex = 0;
    initWasCalled = (unsigned char) 1;

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmCreate
 *
 * Creates and opens a new buffer with name bufName and size bufSize.<BR>
 * If a buffer with name bufName already exists, the existing buffer
 * is resized to bufSize.
 *
 * PRE-Condition:   bufSize > 0
 *
 * POST-Condition:  handle refers to buffer bufName; BufferSize = size
 *
 * IN:      bufName
 *          Name of buffer to be created
 * IN:      bufSize
 *          Size of buffer to be created
 *
 * OUT:     wsmH
 *          Handle to new buffer
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_SIZE, if bufSize <= 0
 *          SML_ERR_NOT_ENOUGH_SPACE, if available memory < bufSize
 *          SML_ERR_WSM_BUF_TABLE_FULL, if buffer table is full
 *          SML_ERR_WRONG_USAGE, if dm_wsmInit wasn't called before
 *
 * @see  dm_wsmDestroy
 */
short dm_wsmCreate(char* bufName, long bufSize, short *wsmH) {

    *wsmH = 0;    // 0 in case of error

    if (!initWasCalled) {
        syncml_task_message("MMIDM  dm_wsmCreate 1");
        return SML_ERR_WRONG_USAGE;
    }

    // check buffer space
    if (dm_getNextFreeEntry() == -1) {
        syncml_task_message("MMIDM dm_wsmCreate 2");
        return wsmRet = SML_ERR_WSM_BUF_TABLE_FULL;
    }
    // check for maxMemAvailable
    if (!dm_isMemAvailable(bufSize)) {
        syncml_task_message("MMIDM dm_wsmCreate 3");
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    // create buffer
    if ((wsmRet = dm_smCreate(bufName, bufSize, wsmH)) != SML_ERR_OK) {
        syncml_task_message("MMIDM dm_wsmCreate 4");
        if (wsmRet == SML_ERR_WRONG_USAGE) {    // buffer already exists
            // resize existing buffer
            // open buffer
            syncml_task_message("MMIDM dm_wsmCreate 5");
            if ((wsmRet = dm_smOpen(bufName, wsmH)) != SML_ERR_OK) {
                syncml_task_message("MMIDM dm_wsmCreate 6");
                return wsmRet = SML_ERR_NOT_ENOUGH_SPACE;
            }
            // resize buffer
            if ((wsmRet = dm_smSetSize(*wsmH, bufSize)) != SML_ERR_OK) {
                syncml_task_message("MMIDM dm_wsmCreate 7");
                return wsmRet = SML_ERR_NOT_ENOUGH_SPACE;
            }
        } else {
            syncml_task_message("MMIDM dm_wsmCreate 8");
            return wsmRet;
        }
    }

    syncml_task_message("MMIDM dm_wsmCreate 9");
    // reset buffer vars
    wsmIndex = dm_resetBufferGlobals(*wsmH);

    // set buffer vars
    if (wsmIndex != INDEX_INVALID) {
        wsmBuf[wsmIndex].size = bufSize;
        wsmBuf[wsmIndex].bufName = dm_smlLibStrdup(bufName);

        if (wsmBuf[wsmIndex].bufName == NULL) {
            dm_smClose(*wsmH);
            dm_smDestroy(bufName);
            syncml_task_message("MMIDM dm_wsmCreate 10");
            return wsmRet = SML_ERR_NOT_ENOUGH_SPACE;
        }
    }
    syncml_task_message("MMIDM dm_wsmCreate 11");

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmClose
 *
 * Close an open buffer.
 *
 * PRE-Condition:   handle is valid; handle is unlocked
 *
 * POST-Condition:  handle is not known to WSM any more
 *
 * IN:      wsmH
 *          Handle to the open buffer
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_HANDLE, if handle was invalid
 *          SML_ERR_WRONG_USAGE, if handle was still locked
 *
 * @see  wsmOpen
 */
short dm_wsmClose(short wsmH) {

    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        syncml_task_message("MMIDM dm_wsmClose 1");
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }

    // close handle
    if ((wsmRet = dm_smClose(wsmH)) != SML_ERR_OK) {
        syncml_task_message("MMIDM  dm_wsmClose 2");
        return wsmRet;
    }
    wsmRet = dm_deleteBufferHandle(wsmH);

    syncml_task_message("MMIDM  dm_wsmClose 3 wsmRet = 0x%4x", wsmRet);
    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmDestroy
 *
 * Destroy existing buffer with name bufName.
 *
 * PRE-Condition:   WSM knows bufName; handle is unlocked
 *
 * POST-Condition:  buffer is not known to WSM any more; all resources
 *                  connected to this buffer are freed
 *
 * IN:      bufName
 *          Name of buffer to be opened
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_PARAM, if bufName is unknown to WSM
 *          SML_ERR_WRONG_USAGE, if handle was still locked
 *
 * @see  dm_wsmCreate
 */
short dm_wsmDestroy(char* bufName) {

    // free resources
    if ((wsmRet = dm_wsmClose(dm_nameToHandle(bufName))) != SML_ERR_OK) {
        syncml_task_message("MMIDM  dm_wsmDestroy 1");
        return wsmRet;
    }

    // free buffer
    if ((wsmRet = dm_smDestroy(bufName)) != SML_ERR_OK) {
        syncml_task_message("MMIDM  dm_wsmDestroy 2");
        return wsmRet;
    }

    syncml_task_message("MMIDM  dm_wsmDestroy 3");
    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmTerminate
 *
 * Terminate WSM; free all buffers and resources.
 *
 * PRE-Condition: all handles must be unlocked
 *
 * POST-Condition: all resources are freed
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_WRONG_USAGE, if a handle was still locked
 *
 */
short dm_wsmTerminate(void) {
    int i;

    // free all WSM resources
    for (i = 0; i < MAX_WSM_BUFFERS; ++i) {
        if (wsmBuf[i].memH != WSM_MEMH_UNUSED)
            if (dm_wsmDestroy(wsmBuf[i].bufName) == SML_ERR_WRONG_USAGE) {
                return SML_ERR_WRONG_USAGE;
            }
    }

    // free global DataStructs
    freeDataStructs();

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmLockH
 *
 * Locks handle wsmH and get a pointer to the contents of wsmH. <BR>
 * RequestedPos describes the position in the buffer to which the returned
 * pointer should point. Valid values are:
 * <UL>
 *   <LI> SML_FIRST_DATA_ITEM
 *   <LI> SML_FIRST_FREE_ITEM
 * </UL>
 *
 * PRE-Condition:   handle is unlocked; handle is valid
 *
 * POST-Condition:  handle is locked; points to first data item,
 *                  or first free item.
 *
 * IN:      wsmH
 *          Handle to the open buffer
 * IN:      requestedPos
 *          Requested position of the returned pointer
 *          <UL>
 *            <LI> SML_FIRST_DATA_ITEM : points to first data entry
 *            <LI> SML_FIRST_FREE_ITEM : points to first free entry
 *          </UL>
 *
 * OUT:     pMem
 *          Pointer to requested memory
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_HANDLE, if handle was invalid
 *          SML_ERR_WRONG_USAGE, if handle was still locked
 *          SML_ERR_UNSPECIFIC, if requested position is unknown, or lock failed
 *
 * @see  dm_wsmUnlockH
 */
short dm_wsmLockH(short wsmH, SmlBufPtrPos_t requestedPos, unsigned char **pMem) {

    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }
    // check if handle is locked
    if (dm_isLockedMemH(wsmH)) {
        return wsmRet = SML_ERR_WRONG_USAGE;
    }

    // lock
    if ((wsmRet = dm_smLock(wsmH, pMem)) != SML_ERR_OK) {
        return wsmRet = SML_ERR_UNSPECIFIC;
    }

    // set local pointers
    wsmIndex = dm_lookup(wsmH);
    if (wsmIndex != INDEX_INVALID) {
        wsmBuf[wsmIndex].pFirstData = *pMem;
        wsmBuf[wsmIndex].pFirstFree = *pMem + wsmBuf[wsmIndex].usedBytes;
        wsmBuf[wsmIndex].flags |= WSM_LOCKED_F;
    }

    switch (requestedPos) {
    case SML_FIRST_DATA_ITEM:
        if (wsmIndex != INDEX_INVALID) {
            *pMem = wsmBuf[wsmIndex].pFirstData;
        }
        break;
    case SML_FIRST_FREE_ITEM:
        if (wsmIndex != INDEX_INVALID) {
            *pMem = wsmBuf[wsmIndex].pFirstFree;
        }
        break;
    default:
        return wsmRet = SML_ERR_UNSPECIFIC;
    }

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmGetFreeSize
 *
 * Returns the remaining unused bytes in the buffer.
 *
 * PRE-Condition:   handle is valid
 *
 * POST-Condition:  dm_wsmGetFreeSize = BufferSize - dm_wsmGetUsedSize
 *
 * IN:      wsmH
 *          Handle to the open buffer
 *
 * OUT:     freeSize
 *          Number of bytes which are unused in this buffer
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_HANDLE, if handle was invalid
 *
 * @see  dm_wsmGetUsedSize
 * @see  dm_wsmProcessedBytes
 */
short dm_wsmGetFreeSize(short wsmH, long *freeSize) {

    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }

    wsmIndex = dm_lookup(wsmH);

    if (wsmIndex != INDEX_INVALID) {
        *freeSize = wsmBuf[wsmIndex].size - wsmBuf[wsmIndex].usedBytes;
    }

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmSetUsedSize
 *
 * Tell Workspace how many data were written into buffer.
 *
 * PRE-Condition:   handle is valid; usedSize <= dm_wsmGetFreeSize; handle is
 *                  locked
 *
 * POST-Condition:  dm_wsmGetUsedSize += usedSize; dm_wsmGetFreeSize -= usedSize;
 *                  instancePtr += usedSize;
 *
 * IN:      wsmH
 *          Handle to the open buffer
 * IN:      usedSize
 *          Number of bytes which were written into buffer
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_HANDLE, if handle was invalid
 *          SML_ERR_INVALID_SIZE, if usedSize <= dm_wsmGetFreeSize
 *
 * @see  dm_wsmGetUsedSize
 */
short dm_wsmSetUsedSize(short wsmH, long usedSize) {

    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }
    // check if handle is unlocked
    if (!dm_isLockedMemH(wsmH)) {
        return wsmRet = SML_ERR_WRONG_USAGE;
    }

    wsmIndex = dm_lookup(wsmH);

    // usedSize > freeSize?
    if (wsmIndex != INDEX_INVALID) {
        if (usedSize > (wsmBuf[wsmIndex].size - wsmBuf[wsmIndex].usedBytes)) {
            return wsmRet = SML_ERR_INVALID_SIZE;
        }

        // adapt usedSize
        wsmBuf[wsmIndex].usedBytes += usedSize;

        // move pFirstFree
        wsmBuf[wsmIndex].pFirstFree += usedSize;
    }

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmGetUsedSize
 *
 * Returns the number of bytes used in the buffer.
 *
 * PRE-Condition:   handle is valid
 *
 * POST-Condition:  usedSize = BufferSize - dm_wsmGetFreeSize
 *
 * IN:      wsmH
 *          Handle to the open buffer
 *
 * OUT:     usedSize
 *          Number of bytes which are already used in this buffer
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_HANDLE, if handle was invalid
 *
 * @see  dm_wsmGetFreeSize
 * @see  dm_wsmSetUsedSize
 */
short dm_wsmGetUsedSize(short wsmH, long *usedSize) {
    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }

    wsmIndex = dm_lookup(wsmH);
    if (wsmIndex != INDEX_INVALID) {
        *usedSize = wsmBuf[wsmIndex].usedBytes;
    }
    syncml_core_message("MMIDM  dm_wsmGetUsedSize usedsize=%d",*usedSize );

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmProcessedBytes
 *
 * Tell Workspace Manager the number of bytes already processed.
 *
 * PRE-Condition:   handle is locked; handle is valid;
 *                  noBytes <= dm_wsmGetUsedSize
 *
 * POST-Condition:  noBytes starting at wsmGetPtr() position are deleted;
 *                  remaining bytes are copied to
 *                  wsmGetPtr(SML_FIRST_FREE_ITEM) position;
 *                  dm_wsmGetUsedSize -= noBytes; dm_wsmGetFreeSize += noBytes
 *
 * IN:      wsmH
 *          Handle to the open buffer
 * IN:      noBytes
 *          Number of bytes already processed from buffer.
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_HANDLE, if handle was invalid
 *          SML_ERR_WRONG_USAGE, if handle was not locked
 *          SML_ERR_INVALID_SIZE, if noBytes > dm_wsmGetUsedSize
 *
 * @see  dm_wsmGetFreeSize
 */
short dm_wsmProcessedBytes(short wsmH, long noBytes) {
    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        syncml_core_message("invalid  wsmH=%d",wsmH );
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }
    // check if handle is unlocked
    if (!dm_isLockedMemH(wsmH)) {
        syncml_core_message("isLockedMemH  wsmH=%d",wsmH );
        return wsmRet = SML_ERR_WRONG_USAGE;
    }

    wsmIndex = dm_lookup(wsmH);

    if (wsmIndex != INDEX_INVALID) {
        if (noBytes > wsmBuf[wsmIndex].usedBytes) {
            syncml_core_message("SML_ERR_INVALID_SIZE  noBytes=%d;usedBytes=%d",noBytes, wsmBuf[wsmIndex].usedBytes );
            syncml_core_message("wsmIndex  =%d",wsmIndex );
            return wsmRet = SML_ERR_INVALID_SIZE;
        }

        // adapt usedSize
        wsmBuf[wsmIndex].usedBytes -= noBytes;

        // move memory
        // check return ?????
        dm_smlLibMemmove(wsmBuf[wsmIndex].pFirstData,
                (wsmBuf[wsmIndex].pFirstData + noBytes),
                wsmBuf[wsmIndex].usedBytes);

        // move pFirstFree
        wsmBuf[wsmIndex].pFirstFree -= noBytes;
    }

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_wsmUnlockH
 *
 * Unlock handle wsmH. <BR>
 * After this call all pointers to this memory handle are invalid
 * and should no longer be used.
 *
 * PRE-Condition:   handle is locked; handle is valid
 *
 * POST-Condition:  handle is unlocked
 *
 * OUT:     wsmH
 *          Handle to unlock
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *          SML_ERR_INVALID_HANDLE, if handle was invalid
 *          SML_ERR_WRONG_USAGE, if handle was not locked
 *          SML_ERR_UNSPECIFIC, unlock failed
 *
 * @see  dm_wsmLockH
 */
short dm_wsmUnlockH(short wsmH) {

    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }
    // check if handle is already unlocked
    if (!dm_isLockedMemH(wsmH)) {
        return wsmRet = SML_ERR_WRONG_USAGE;
    }

    // unlock
    if ((wsmRet = dm_smUnlock(wsmH)) != SML_ERR_OK) {
        return wsmRet = SML_ERR_UNSPECIFIC;
    }

    // set local pointers
    wsmIndex = dm_lookup(wsmH);
    if (wsmIndex != INDEX_INVALID) {
        wsmBuf[wsmIndex].pFirstData = NULL;
        wsmBuf[wsmIndex].pFirstFree = NULL;
        wsmBuf[wsmIndex].flags &= ~WSM_LOCKED_F;
    }

    return wsmRet = SML_ERR_OK;
}

/**
 * FUNCTION: dm_pim_wsmReset
 *
 * Reset the Workspace
 *
 * PRE-Condition:   -
 *
 * POST-Condition:  all data is lost. The FirstFree Position equals
 * the First Data position
 *
 * IN:      wsmH
 *          Handle to the open buffer
 *
 * RETURN:  SML_ERR_OK, if O.K.
 *
 */
short dm_pim_wsmReset(short wsmH) {

    // check if handle is invalid
    if (!dm_isValidMemH(wsmH)) {
        return wsmRet = SML_ERR_INVALID_HANDLE;
    }

    wsmIndex = dm_lookup(wsmH);

    if (wsmIndex != INDEX_INVALID) {
        wsmBuf[wsmIndex].pFirstFree = wsmBuf[wsmIndex].pFirstFree
                - wsmBuf[wsmIndex].usedBytes;
        wsmBuf[wsmIndex].pFirstData = wsmBuf[wsmIndex].pFirstFree;
        wsmBuf[wsmIndex].usedBytes = 0;
    }

    return SML_ERR_OK;
}
