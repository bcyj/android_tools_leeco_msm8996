#ifndef  HEADER_FILE_SMLCORE
#define  HEADER_FILE_SMLCORE

#include "smldef.h"
#include "wsm.h"

/**
 * structure describing the current status of the global syncml module
 * (holds all global variables within SyncML)
 **/
typedef struct syncml_info_s {
    InstanceInfoPtr_t instanceListAnchor; // Anchor of the global list of known SyncML instances
    SmlOptionsPtr_t syncmlOptions;     // Options valid for this SyncML Process
    WsmGlobalsPtr_t wsmGlobals;        // Workspace global variables
    TokenInfoPtr_t tokTbl;
}*SyncMLInfoPtr_t, SyncMLInfo_t;

extern SyncMLInfoPtr_t dm_pGlobalAnchor;

short dm_smlInit(SmlOptionsPtr_t pOptions);
short dm_smlInitInstance(SmlCallbacksPtr_t callbacks,
        SmlInstanceOptionsPtr_t pOptions, void* pUserData, short *pInstanceID);

short dm_smlStartMessageExt(short id, SmlSyncHdrPtr_t pContent,
        SmlVersion_t vers);
short dm_smlStartSync(short id, SmlSyncPtr_t pContent);
#ifdef ADD_SEND
short dm_smlAddCmd(short id, SmlAddPtr_t pContent);
#endif
short dm_smlDeleteCmd(short id, SmlDeletePtr_t pContent);
short dm_smlReplaceCmd(short id, SmlReplacePtr_t pContent);
short dm_smlAlertCmd(short id, SmlAlertPtr_t pContent);
short dm_smlPutCmd(short id, SmlPutPtr_t pContent);
short dm_smlGetCmd(short id, SmlGetPtr_t pContent);
short dm_smlStatusCmd(short id, SmlStatusPtr_t pContent);
short dm_smlMapCmd(short id, SmlMapPtr_t pContent);
short dm_smlEndSync(short id);
short dm_smlSetUserData(short id, void* pUserData);
short dm_smlProcessData(short id, SmlProcessMode_t mode);
short dm_smlEndMessage(short id, unsigned char final);
short dm_smlTerminateInstance(short id);
short dm_smlTerminate(void);

short dm_smlLockWriteBuffer(short id, unsigned char **pWritePosition,
        long *freeSize);
short dm_smlUnlockWriteBuffer(short id, long writtenBytes);
short dm_smlLockReadBuffer(short id, unsigned char* *pReadPosition,
        long *usedSize);
short dm_smlUnlockReadBuffer(short id, long processedBytes);

short dm_mgrProcessStartMessage(short id, InstanceInfoPtr_t pInstanceInfo);
short dm_mgrResetWorkspace(short id);
short dm_mgrProcessNextCommand(short id, InstanceInfoPtr_t pInstanceInfo);

#endif
