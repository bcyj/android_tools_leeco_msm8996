#ifndef  HEADER_FILE_COMDEF
#define  HEADER_FILE_COMDEF

/* support WBXML code switch */
#define __SML_WBXML__
/*support XML code switch*/
#define __SML_XML__

/*support Sub DTD extensions */
#define __USE_EXTENSIONS__

/* use Metainformation DTD  */
#define __USE_METINF__
/* use Device Info DTD  */
#define __USE_DEVINF__

/*support syncml1.1 switch*/
#define __SML_1_1__

/*support syncml1.2  switch*/
//#define __SML_1_2__

/* which of the following optional commands should be included ? */
#define ADD_SEND

#define COPY_SEND
#define COPY_RECEIVE

#define ATOMIC_SEND
#define ATOMIC_RECEIVE

#define SEQUENCE_SEND
#define SEQUENCE_RECEIVE

#define EXEC_SEND

#define SEARCH_SEND

#define MAP_RECEIVE

#define RESULT_RECEIVE

#define EXEC_RECEIVE

#define HTTPLOGNAME  "httplogs"

/*data struct state*/
/**
 * PCDATA - types of synchronization data which SyncML supports
 **/
typedef enum {
    SML_PCDATA_UNDEFINED = 0, SML_PCDATA_STRING,                  // String type
    SML_PCDATA_OPAQUE,                   // Opaque type
    SML_PCDATA_EXTENSION,     // Extention type - specified by PcdataExtension_t
    SML_PCDATA_CDATA                     // XML CDATA type
} SmlPcdataType_t;

/**
 * PCDATA - types of extensions for PCData elements
 */
typedef enum {
    SML_EXT_UNDEFINED = 0, SML_EXT_METINF, // Meta Information
    SML_EXT_DEVINF, // Device Information
    SML_EXT_LAST,    // last codepage, needed for loops!
    SML_EXT_MAX = 254
} SmlPcdataExtension_t;

/**
 * PCDATA - into this structure SyncML wraps the synchronization data itself
 **/
typedef struct sml_pcdata_s {
    SmlPcdataType_t contentType; // The type of data which a PCDATA structure contains
    SmlPcdataExtension_t extension;     // PCData Extension type
    long length;        // length of the data in this PCDATA structure
    void* content;       // Pointer to the data itself
}*SmlPcdataPtr_t, SmlPcdata_t;

/* generic list of PCData elements */
typedef struct sml_pcdata_list_s {
    SmlPcdataPtr_t data;
    struct sml_pcdata_list_s *next;
}*SmlPcdataListPtr_t, SmlPcdataList_t;

#endif
