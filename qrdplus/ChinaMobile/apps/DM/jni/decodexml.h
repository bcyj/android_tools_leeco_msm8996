#ifndef  HEADER_FILE_DECODE_XML
#define  HEADER_FILE_DECODE_XML
#include "codec.h"

/**
 * Private Interface for the XML scanner.
 */
typedef struct xmlScannerPriv_s xmlScannerPriv_t, *xmlScannerPrivPtr_t;
struct xmlScannerPriv_s {
    /* public */
    short (*nextTok)(const char*, int, XltDecScannerPtr_t);
    short (*destroy)(XltDecScannerPtr_t);
    short (*pushTok)(XltDecScannerPtr_t);
    void (*setBuf)(XltDecScannerPtr_t pScanner, unsigned char* pBufStart,
            unsigned char* pBufEnd); // delete the const symbol for reduce compiling warnings!
    unsigned char* (*getPos)(XltDecScannerPtr_t pScanner);

    XltDecTokenPtr_t curtok; /* current token */
    long charset; /* 0 */
    char* charsetStr; /* character set */
    long pubID; /* 0 */
    char* pubIDStr; /* document public identifier */
    SmlPcdataExtension_t ext; /* which is the actual open namespace ? */
    SmlPcdataExtension_t prev_ext; /* which is the previous open namespace ? */
    XltTagID_t ext_tag; /* which tag started the actual namespace ? */
    XltTagID_t prev_ext_tag; /* which tag started the previous open namespace ? */
    char* nsprefix; /* prefix used for active namespace (if any) */
    unsigned char nsprelen; /* how long is the prefix ? (to save dm_smlLibStrlen calls) */
    unsigned int finished;

    /* private */
    unsigned char* pos; /* current position */
    unsigned char* bufend; /* end of buffer */
};

short dm_xltDecXmlInit(unsigned char* pBufEnd, unsigned char* *ppBufStart,
        XltDecScannerPtr_t *ppScanner); // delete the const symbol for reduce compiling warnings!
#endif
