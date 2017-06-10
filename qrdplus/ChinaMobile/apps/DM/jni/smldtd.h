#ifndef  HEADER_FILE_SMLDTD
#define  HEADER_FILE_SMLDTD

#include "comdef.h"

/**
 * ===========================
 * Common used SyncML Elements
 * ===========================
 */

/**
 * SyncML Protocol Management and Command Elements (PE)
 **/
typedef enum {
    SML_PE_UNDEF = 0,
    SML_PE_ERROR,
    SML_PE_ADD,
    SML_PE_ALERT,
    SML_PE_ATOMIC_START,
    SML_PE_ATOMIC_END,
    SML_PE_COPY,
    SML_PE_DELETE,
    SML_PE_EXEC,
    SML_PE_GET,
    SML_PE_MAP,
    SML_PE_PUT,
    SML_PE_RESULTS,
    SML_PE_SEARCH,
    SML_PE_SEQUENCE_START,
    SML_PE_SEQUENCE_END,
    SML_PE_STATUS,
    SML_PE_SYNC_START,
    SML_PE_SYNC_END,
    SML_PE_REPLACE,
    SML_PE_HEADER,
    SML_PE_PUT_GET,
    SML_PE_CMD_GROUP,
    SML_PE_GENERIC,
    SML_PE_FINAL
} SmlProtoElement_t;

/**
 * Various flags which are actually declared and (EMPTY) elements in
 * SyncML. This assumes at least a 16-bit architecture for the
 * underlying OS. We need to review this if that is deemed a problem.
 **/
#define SmlArchive_f       0x8000         // Delete flags
#define SmlSftDel_f        0x4000         // Delete flags
#define SmlMoreData_f      0x0400         // MoreData flag
#define SmlNoResults_f     0x0200         // No Results flag
#define SmlNoResp_f        0x0100         // No Response flag
#define SmlFinal_f         0x0001         // Header flag
#define SmlMetInfSharedMem_f 0x0002         // MetInf Shared Memory Flag
#define SmlDevInfSharedMem_f 0x0004         // DevInf Shared Memory Flag
/* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
#define SmlDevInfUTC_f          0x0001      // DevInf utc Flag
#define SmlDevInfNOfM_f         0x0002      // DevInf support n of m Flag
#define SmlDevInfLargeObject_f  0x0008      // DevInf support large object Flag

/**
 * Chal
 **/
typedef struct sml_chal_s {
    SmlPcdataPtr_t meta;
}*SmlChalPtr_t, SmlChal_t;

/**
 * Credentials
 **/
typedef struct sml_cred_s {
    SmlPcdataPtr_t meta;       // opt.
    SmlPcdataPtr_t data;
}*SmlCredPtr_t, SmlCred_t;

/**
 * Source or target location
 **/
typedef struct sml_source_or_target_s {
    SmlPcdataPtr_t locURI;
    SmlPcdataPtr_t locName;    // opt.
}*SmlSourcePtr_t, SmlSource_t, *SmlTargetPtr_t, SmlTarget_t;

typedef struct sml_source_list_s {
    SmlSourcePtr_t source;
    struct sml_source_list_s *next;
}*SmlSourceListPtr_t, SmlSourceList_t;

/**
 * ==============================
 * SyncML Message Header Elements
 * ==============================
 **/

/**
 * SyncML header
 * As the header is needed for each SyncML message, it's also the parameter
 * of the startMessage call.
 **/
typedef struct sml_sync_hdr_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t version;
    SmlPcdataPtr_t proto;
    SmlPcdataPtr_t sessionID;
    SmlPcdataPtr_t msgID;
    unsigned int flags;      // NoResp
    SmlTargetPtr_t target;
    SmlSourcePtr_t source;
    SmlPcdataPtr_t respURI;    // opt.
    SmlCredPtr_t cred;       // opt.
    SmlPcdataPtr_t meta;     // opt.
}*SmlSyncHdrPtr_t, SmlSyncHdr_t;

/**
 * =========================
 * Data description elements
 * =========================
 **/

/**
 * Data in SyncML is encapsulated in an "item" element.
 **/
typedef struct sml_item_s {
    SmlTargetPtr_t target;     // opt.
    SmlSourcePtr_t source;     // opt.
    SmlPcdataPtr_t meta;       // opt.
    SmlPcdataPtr_t data;       // opt.
    unsigned int flags;      // opt. for MoreData
}*SmlItemPtr_t, SmlItem_t;

typedef struct sml_item_list_s {
    SmlItemPtr_t item;
    struct sml_item_list_s *next;
}*SmlItemListPtr_t, SmlItemList_t;

/**
 * ==============================================
 * SyncML Commands (Protocol Management Elements)
 * ==============================================
 **/

/**
 * Generic commands:
 * Add, Copy, Replace, Delete
 **/
typedef struct sml_generic_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    unsigned int flags;      // NoResp, Archive (Delete), SftDel (Delete)
    SmlCredPtr_t cred;       // opt.
    SmlPcdataPtr_t meta;       // opt.
    SmlItemListPtr_t itemList;
}*SmlAddPtr_t, SmlAdd_t, *SmlCopyPtr_t, SmlCopy_t, *SmlReplacePtr_t,
        SmlReplace_t, *SmlDeletePtr_t, SmlDelete_t, *SmlGenericCmdPtr_t,
        SmlGenericCmd_t;

/**
 * Alert command:
 **/
typedef struct sml_alert_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    unsigned int flags;      // NoResp
    SmlCredPtr_t cred;       // opt.
    SmlPcdataPtr_t data;       // opt.
    SmlItemListPtr_t itemList;
}*SmlAlertPtr_t, SmlAlert_t;

/**
 * Atomic/Sequence command:
 **/
typedef struct sml_atomic_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    unsigned int flags;      // NoResp
    SmlPcdataPtr_t meta;       // opt.
}*SmlAtomicPtr_t, SmlAtomic_t, *SmlSequencePtr_t, SmlSequence_t;

/**
 * Sync command:
 **/
typedef struct sml_sync_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    unsigned int flags;      // NoResp
    SmlCredPtr_t cred;       // opt.
    SmlTargetPtr_t target;     // opt.
    SmlSourcePtr_t source;     // opt.
    SmlPcdataPtr_t meta;       // opt.
    SmlPcdataPtr_t noc;        // opt. (SyncML 1.1)
}*SmlSyncPtr_t, SmlSync_t;

/**
 * Exec command:
 **/
typedef struct sml_exec_s {
    SmlProtoElement_t elementType;
    SmlPcdataPtr_t cmdID;
    unsigned int flags;      // NoResp
    SmlCredPtr_t cred;       // opt.
    SmlPcdataPtr_t meta;       // opt.
    SmlItemPtr_t item;
}*SmlExecPtr_t, SmlExec_t;

/**
 * Get and Put command:
 **/
typedef struct sml_get_put_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    unsigned int flags;      // NoResp
    SmlPcdataPtr_t lang;       // opt.
    SmlCredPtr_t cred;       // opt.
    SmlPcdataPtr_t meta;       // opt.
    SmlItemListPtr_t itemList;
}*SmlPutPtr_t, SmlPut_t, *SmlGetPtr_t, SmlGet_t;

/**
 * Map command:
 **/
#define MMIDM_MAX_LOCAL_ID_LEN 50

typedef struct sml_map_item_s {
    SmlTargetPtr_t target;
    SmlSourcePtr_t source;
}*SmlMapItemPtr_t, SmlMapItem_t;

typedef struct sml_map_item_list_s {
    SmlMapItemPtr_t mapItem;
    struct sml_map_item_list_s *next;
}*SmlMapItemListPtr_t, SmlMapItemList_t;

typedef struct sml_map_s {
    SmlProtoElement_t elementType; // InternalToolkit Field
    SmlPcdataPtr_t cmdID;
    SmlTargetPtr_t target;
    SmlSourcePtr_t source;
    SmlCredPtr_t cred;       // opt.
    SmlPcdataPtr_t meta;       // opt.
    SmlMapItemListPtr_t mapItemList;
}*SmlMapPtr_t, SmlMap_t;

/**
 * Results command:
 **/
typedef struct sml_results_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    SmlPcdataPtr_t msgRef;     // opt.
    SmlPcdataPtr_t cmdRef;
    SmlPcdataPtr_t meta;       // opt.
    SmlPcdataPtr_t targetRef;  // opt.
    SmlPcdataPtr_t sourceRef;  // opt.
    SmlItemListPtr_t itemList;
}*SmlResultsPtr_t, SmlResults_t;

/**
 * Search command:
 **/
typedef struct sml_search_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    unsigned int flags;      // NoResp, NoResults
    SmlCredPtr_t cred;       // opt.
    SmlTargetPtr_t target;     // opt.
    SmlSourceListPtr_t sourceList;
    SmlPcdataPtr_t lang;       // opt.
    SmlPcdataPtr_t meta;
    SmlPcdataPtr_t data;
}*SmlSearchPtr_t, SmlSearch_t;

/**
 * Status command:
 **/

typedef struct sml_target_ref_list_s {
    SmlPcdataPtr_t targetRef;
    struct sml_target_ref_list_s *next;
}*SmlTargetRefListPtr_t, SmlTargetRefList_t;

typedef struct sml_source_ref_list_s {
    SmlPcdataPtr_t sourceRef;
    struct sml_source_ref_list_s *next;
}*SmlSourceRefListPtr_t, SmlSourceRefList_t;

typedef struct sml_status_s {
    SmlProtoElement_t elementType; // Internal Toolkit Field
    SmlPcdataPtr_t cmdID;
    SmlPcdataPtr_t msgRef; // Opt.
    SmlPcdataPtr_t cmdRef;
    SmlPcdataPtr_t cmd;
    SmlTargetRefListPtr_t targetRefList;  // opt.
    SmlSourceRefListPtr_t sourceRefList;  // opt.
    SmlCredPtr_t cred;           // opt.
    SmlChalPtr_t chal;           // opt.
    SmlPcdataPtr_t data;
    SmlItemListPtr_t itemList;       // opt.
}*SmlStatusPtr_t, SmlStatus_t;

/**
 * a little helper for typecasting
 **/
typedef struct sml_unknown_proto_element_s {
    SmlProtoElement_t elementType;  // Internal Toolkit Field
}*SmlUnknownProtoElementPtr_t, SmlUnknownProtoElement_t;

#endif
