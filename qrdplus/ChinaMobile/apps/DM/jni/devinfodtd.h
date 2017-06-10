#ifndef  HEADER_FILE_DEVINFODTD
#define  HEADER_FILE_DEVINFODTD
#include"comdef.h"

#ifdef __USE_DEVINF__

typedef struct sml_devinf_synccap_s {
    SmlPcdataListPtr_t synctype;
}*SmlDevInfSyncCapPtr_t, SmlDevInfSyncCap_t;

typedef struct sml_devinf_dsmem_s {
    unsigned int flags; /* %%% luz:2003-04-28, mad flag, was PCData (completely wrong) */
    SmlPcdataPtr_t maxmem; /* optional */
    SmlPcdataPtr_t maxid; /* optional */
}*SmlDevInfDSMemPtr_t, SmlDevInfDSMem_t;

typedef struct sml_devinf_ext_s {
    SmlPcdataPtr_t xnam;
    SmlPcdataListPtr_t xval; /* optional */
}*SmlDevInfExtPtr_t, SmlDevInfExt_t;

typedef struct sml_devinf_xmit_s {
    SmlPcdataPtr_t cttype;
    SmlPcdataPtr_t verct;
}*SmlDevInfXmitPtr_t, SmlDevInfXmit_t;

typedef struct sml_devinf_ctdata_s {
    SmlPcdataPtr_t name;
    SmlPcdataPtr_t dname; /* optional, display name */

    SmlPcdataListPtr_t valenum;
    SmlPcdataPtr_t datatype;
    SmlPcdataPtr_t size;
}*SmlDevInfCTDataPtr_t, SmlDevInfCTData_t;

typedef struct sml_devinf_xmitlist_s {
    SmlDevInfXmitPtr_t data;
    struct sml_devinf_xmitlist_s *next;
}*SmlDevInfXmitListPtr_t, SmlDevInfXmitList_t;

typedef struct sml_devinf_datastore_s {
    SmlPcdataPtr_t sourceref;
    SmlPcdataPtr_t displayname; /* optional */
    SmlPcdataPtr_t maxguidsize; /* optional */
    SmlDevInfXmitPtr_t rxpref;
    SmlDevInfXmitListPtr_t rx; /* optional */
    SmlDevInfXmitPtr_t txpref;
    SmlDevInfXmitListPtr_t tx; /* optional */
    SmlDevInfDSMemPtr_t dsmem; /* optional */
    SmlDevInfSyncCapPtr_t synccap;
}*SmlDevInfDatastorePtr_t, SmlDevInfDatastore_t;

typedef struct sml_devinf_datastorelist_s {
    SmlDevInfDatastorePtr_t data;
    struct sml_devinf_datastorelist_s *next;
}*SmlDevInfDatastoreListPtr_t, SmlDevInfDatastoreList_t;

typedef struct sml_devinf_ctdatalist_s {
    SmlDevInfCTDataPtr_t data;
    struct sml_devinf_ctdatalist_s *next;
}*SmlDevInfCTDataListPtr_t, SmlDevInfCTDataList_t;

typedef struct sml_devinf_ctdataprop_s {
    SmlDevInfCTDataPtr_t prop;
    SmlDevInfCTDataListPtr_t param;
}*SmlDevInfCTDataPropPtr_t, SmlDevInfCTDataProp_t;

typedef struct sml_devinf_ctdataproplist_s {
    SmlDevInfCTDataPropPtr_t data;
    struct sml_devinf_ctdataproplist_s *next;
}*SmlDevInfCTDataPropListPtr_t, SmlDevInfCTDataPropList_t;

typedef struct sml_devinf_ctcap_s {
    SmlPcdataPtr_t cttype;
    SmlDevInfCTDataPropListPtr_t prop;
}*SmlDevInfCTCapPtr_t, SmlDevInfCTCap_t;

typedef struct sml_devinf_extlist_s {
    SmlDevInfExtPtr_t data;
    struct sml_devinf_extlist_s *next;
}*SmlDevInfExtListPtr_t, SmlDevInfExtList_t;

typedef struct sml_devinf_ctcaplist_s {
    SmlDevInfCTCapPtr_t data;
    struct sml_devinf_ctcaplist_s *next;
}*SmlDevInfCtcapListPtr_t, SmlDevInfCtcapList_t;

typedef struct sml_devinf_devinf_s {
    SmlPcdataPtr_t verdtd;
    SmlPcdataPtr_t man; /* optional */
    SmlPcdataPtr_t mod; /* optional */
    SmlPcdataPtr_t oem; /* optional */
    SmlPcdataPtr_t fwv; /* optional */
    SmlPcdataPtr_t swv; /* optional */
    SmlPcdataPtr_t hwv; /* optional */
    SmlPcdataPtr_t devid;
    SmlPcdataPtr_t devtyp;
    SmlDevInfDatastoreListPtr_t datastore;
    SmlDevInfCtcapListPtr_t ctcap;
    SmlDevInfExtListPtr_t ext;
    /* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
    unsigned int flags;
}*SmlDevInfDevInfPtr_t, SmlDevInfDevInf_t;
#endif

/*  全局变量声明*/

/*  功能函数声明*/

#endif
