#ifndef  HEADER_FILE_METAINFODTD
#define  HEADER_FILE_METAINFODTD

#include "comdef.h"

#ifdef __USE_METINF__
typedef struct sml_metinf_anchor_s {
    SmlPcdataPtr_t last; /* optional */
    SmlPcdataPtr_t next;
}*SmlMetInfAnchorPtr_t, SmlMetInfAnchor_t;

typedef struct sml_metinf_mem_s {
    SmlPcdataPtr_t shared; /* optional */
    SmlPcdataPtr_t free;
    SmlPcdataPtr_t freeid;
}*SmlMetInfMemPtr_t, SmlMetInfMem_t;

typedef struct sml_metinf_metinf_s {
    SmlPcdataPtr_t format; /* opt. */
    SmlPcdataPtr_t type; /* opt. */
    SmlPcdataPtr_t mark; /* opt. */
    SmlPcdataPtr_t size; /* opt. */
    SmlPcdataPtr_t nextnonce; /* opt. */
    SmlPcdataPtr_t version;
    SmlPcdataPtr_t maxmsgsize; /* optional */
    /* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
    SmlPcdataPtr_t maxobjsize; /* optional */
    SmlMetInfMemPtr_t mem; /* optional */
    SmlPcdataListPtr_t emi; /* optional */
    SmlMetInfAnchorPtr_t anchor; /* opt. */
}*SmlMetInfMetInfPtr_t, SmlMetInfMetInf_t;

#endif

/*  全局变量声明*/

/*  功能函数声明*/

#endif
