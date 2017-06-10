#ifndef  HEADER_FILE_WSM
#define  HEADER_FILE_WSM

// use default value of 4 (not much for a multi-connection server)

#define MAX_WSM_BUFFERS 2
#define MAX_WSM_BUFFER_SIZE (100*1024)
#define ONE_WORK_SPACE_SIZE  (30*1024)

typedef struct WsmOptions_s {
    long maxAvailMem;  // maximum amount of memory available for all wsm buffers
} WsmOptions_t;

/** WSM internal buffer structure */
typedef struct WsmBuf_s {
    char* bufName;     // external name of buffer
    short memH;        // memory handle
    unsigned char* pFirstFree;  // pointer to first free element in buffer
    unsigned char* pFirstData;  // pointer to first data element in buffer
    long size;        // size of buffer
    long usedBytes;   // used bytes in buffer
    unsigned char flags;
} WsmBuf_t;

/* sbuffer list */
typedef struct smWinList_s {
    char *memName;      // name of buffer
    char *winH;         // reference to memory block
    short memH;         // handle of memory block
    unsigned char locked;       // is handle locked?
    long memSize;      // size of memory block
    struct smWinList_s *next;         // next list item
} smWinList_t;

typedef smWinList_t *WsmSmGlobals_t;

/** WSM globals for use with global Anchor */
typedef struct WsmGlobals_s {
    short wsmRet;          // last WSM return code
    unsigned char initWasCalled;   // was dm_wsmInit() called?
    WsmBuf_t wsmBuf[MAX_WSM_BUFFERS];
    short wsmIndex;        // Index of actual buffer
    WsmSmGlobals_t wsmSm;           // WSM_SM global; device dependent!
}*WsmGlobalsPtr_t, WsmGlobals_t;

//private functions
short dm_getNextFreeEntry(void);
unsigned char dm_isMemAvailable(long memToAlloc);
short dm_resetBufferGlobals(short memH);

short dm_nameToHandle(char* name);
unsigned char dm_isValidMemH(short memH);
unsigned char dm_isLockedMemH(short memH);
unsigned char dm_locateH(short memH, smWinList_t **p);
short dm_deleteBufferHandle(short memH);

unsigned char dm_newListEle(const char *name, smWinList_t **newEle,
        short *newHandle);
unsigned char dm_locateEle(const char *eleName, smWinList_t **p);
void dm_removeEle(const char *eleName);

//public functions
void dm_createDataStructs(void);

InstanceInfoPtr_t dm_findInfo(short id);

short dm_addInfo(InstanceInfoPtr_t pInfo);

short dm_removeInfo(short id);

short dm_smCreate(char* memName, long memSize, short *memH);
short dm_smOpen(char* memName, short *memH);
short dm_smClose(short memH);
short dm_smDestroy(char* memName);
short dm_smSetSize(short memH, long newSize);
short dm_smLock(short memH, unsigned char **pMem);
short dm_smUnlock(short memH);

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
 *          <LI> tbd
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
short dm_wsmInit(const WsmOptions_t *wsmOpts);

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
short dm_wsmCreate(char* bufName, long bufSize, short *wsmH);

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
short dm_wsmClose(short wsmH);

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
short dm_wsmDestroy(char* bufName);

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
short dm_wsmTerminate(void);

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
short dm_wsmLockH(short wsmH, SmlBufPtrPos_t requestedPos,
        unsigned char **pMem);
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
short dm_wsmGetFreeSize(short wsmH, long *freeSize);

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
short dm_wsmSetUsedSize(short wsmH, long usedSize);

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
short dm_wsmGetUsedSize(short wsmH, long *usedSize);

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
short dm_wsmProcessedBytes(short wsmH, long noBytes);

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
short dm_wsmUnlockH(short wsmH);

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
short dm_pim_wsmReset(short wsmH);

#endif
