#ifndef  HEADER_FILE_SMLDEF
#define  HEADER_FILE_SMLDEF

#include "codec.h"

/**
 * Application callback function displaying output strings to the user
 **/
typedef void (*smlPrintFunc)(char* outputString);

/**
 * ====================================================
 * Callback Functions to be implemented by the Receiver
 * ====================================================
 **/

/* Protocol Management */
typedef short (*smlStartMessageFunc)(short id, void* userData,
        SmlSyncHdrPtr_t pContent);
typedef short (*smlEndMessageFunc)(short id, void* userData,
        unsigned char final);
typedef short (*smlStartSyncFunc)(short id, void* userData,
        SmlSyncPtr_t pContent);
typedef short (*smlEndSyncFunc)(short id, void* userData);

typedef short (*smlStartAtomicFunc)(short id, void* userData,
        SmlAtomicPtr_t pContent);
typedef short (*smlEndAtomicFunc)(short id, void* userData);

typedef short (*smlStartSequenceFunc)(short id, void* userData,
        SmlSequencePtr_t pContent);
typedef short (*smlEndSequenceFunc)(short id, void* userData);

/* Sync Commands */
typedef short (*smlAddCmdFunc)(short id, void* userData, SmlAddPtr_t pContent);
typedef short (*smlAlertCmdFunc)(short id, void* userData,
        SmlAlertPtr_t pContent);
typedef short (*smlDeleteCmdFunc)(short id, void* userData,
        SmlDeletePtr_t pContent);
typedef short (*smlGetCmdFunc)(short id, void* userData, SmlGetPtr_t pContent);
typedef short (*smlPutCmdFunc)(short id, void* userData, SmlPutPtr_t pContent);

typedef short (*smlMapCmdFunc)(short id, void* userData, SmlMapPtr_t pContent);

typedef short (*smlResultsCmdFunc)(short id, void* userData,
        SmlResultsPtr_t pContent);

typedef short (*smlStatusCmdFunc)(short id, void* userData,
        SmlStatusPtr_t pContent);
typedef short (*smlReplaceCmdFunc)(short id, void* userData,
        SmlReplacePtr_t pContent);

typedef short (*smlCopyCmdFunc)(short id, void* userData, SmlCopyPtr_t param);

typedef short (*smlExecCmdFunc)(short id, void* userData,
        SmlExecPtr_t pContent);

typedef short (*smlSearchCmdFunc)(short id, void* userData,
        SmlSearchPtr_t pContent);

typedef short (*smlFinalCmdFunc)(void);

typedef short (*smlHandleErrorFunc)(short id, void* userData);
typedef short (*smlTransmitChunkFunc)(short id, void* userData);

/**
 * Structure defining references to the applications callback implementations
 **/
typedef struct sml_callbacks_s {
    /* Protocol Management Callbacks */
    smlStartMessageFunc startMessageFunc;
    smlEndMessageFunc endMessageFunc;
    smlStartSyncFunc startSyncFunc;
    smlEndSyncFunc endSyncFunc;

    smlStartAtomicFunc startAtomicFunc;
    smlEndAtomicFunc endAtomicFunc;

    smlStartSequenceFunc startSequenceFunc;
    smlEndSequenceFunc endSequenceFunc;

    /* Sync Command callbacks */
    smlAddCmdFunc addCmdFunc;
    smlAlertCmdFunc alertCmdFunc;
    smlDeleteCmdFunc deleteCmdFunc;
    smlGetCmdFunc getCmdFunc;
    smlPutCmdFunc putCmdFunc;

    smlMapCmdFunc mapCmdFunc;

    smlResultsCmdFunc resultsCmdFunc;

    smlStatusCmdFunc statusCmdFunc;
    smlReplaceCmdFunc replaceCmdFunc;

    smlCopyCmdFunc copyCmdFunc;

    smlExecCmdFunc execCmdFunc;

    smlSearchCmdFunc searchCmdFunc;

    smlFinalCmdFunc FinalFunc;

    /* Other Callbacks */
    smlHandleErrorFunc handleErrorFunc;
    smlTransmitChunkFunc transmitChunkFunc;
    //smlPrintFunc           printFunc;
}*SmlCallbacksPtr_t, SmlCallbacks_t;

/**
 * Current instance status
 **/
typedef enum {
    MGR_IDLE,          // instance is idle (available for usage by applications)
    MGR_USED,                      // instance is in use, but currently inactive
    MGR_RECEIVE,          // actively used for receiving (locked by application)
    MGR_SEND,               // actively used for sending (locked by application)
    MGR_ENCODING,               // actively used for encoding (locked by SyncML)
    MGR_DECODING                // actively used for decoding (locked by SyncML)
} InstanceStatus_t;

/**
 * Requested buffer pointer position
 **/
typedef enum {
    SML_FIRST_DATA_ITEM = 0, SML_FIRST_FREE_ITEM
} SmlBufPtrPos_t;

/**
 * Processing modes
 **/
typedef enum {
    SML_DO_NOTHING = 0,
    SML_FIRST_COMMAND,
    SML_NEXT_COMMAND,
    SML_NEXT_MESSAGE,
    SML_ALL_COMMANDS
} SmlProcessMode_t;

/**
 * structure describing the options and setting of this syncml process
 **/
typedef struct sml_options_s {
    smlPrintFunc defaultPrintFunc; // default application callback for displaying strings,
    long maxWorkspaceAvailMem; // size which all workspaces in total MUST not exceed
}*SmlOptionsPtr_t, SmlOptions_t;

/**
 * structure describing the options of an instance,
 **/
typedef struct sml_instance_options_s {
    SmlEncoding_t encoding;               // Used encoding type,
    long workspaceSize; // size of the workspace to allocate (instance buffer size if NOWSM defined)
    char* workspaceName;          // name of the workspace
}*SmlInstanceOptionsPtr_t, SmlInstanceOptions_t;

/**
 * structure describing the current status of an instance,
 **/
typedef struct instance_info_s {
    short id;                // unique ID of the instance
    unsigned char* workspaceHandle; // handle to the  first position of the assigned workspace memory
    InstanceStatus_t status;            // current internal state of instance
    SmlCallbacksPtr_t callbacks; // Defined callback refererences for this Instance
    SmlInstanceOptionsPtr_t instanceOptions; // Defined options for this Instance (e.g. encoding type)
    void* userData; // Pointer to a structure, which is passed to the invoked callback functions
    void* workspaceState; // Pointer to a structure defining the current workspace status
    void* encoderState; // Pointer to a structure defining the current encoder status
    void* decoderState; // Pointer to a structure defining the current decoder status
    struct instance_info_s* nextInfo; // Pointer to next Instance Info in a list
}*InstanceInfoPtr_t, InstanceInfo_t;

/* Pointers to store the global Tag tables */
typedef struct tokeninfo_s {
    TagPtr_t SyncML;
    TagPtr_t MetInf;
    TagPtr_t DevInf;
}*TokenInfoPtr_t, TokenInfo_t;

#endif
