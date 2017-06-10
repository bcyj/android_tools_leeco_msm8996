#ifndef  HEADER_FILE_DECODE_WBXML
#define  HEADER_FILE_DECODE_WBXML

#include "codec.h"

/**
 * Private Interface for the WBXML scanner.
 *
 * The private scanner interface contains some additional member attributes
 * that are not listed in the public interface, e.g. a copy of the string
 * table and some other items that do not need to be known outside the
 * scanner module.
 */
typedef struct wbxmlScannerPriv_s wbxmlScannerPriv_t, *wbxmlScannerPrivPtr_t;
struct wbxmlScannerPriv_s {
    /* public methods */
    short (*nextTok)(const char*, int, XltDecScannerPtr_t);
    short (*destroy)(XltDecScannerPtr_t);
    short (*pushTok)(XltDecScannerPtr_t);
    void (*setBuf)(XltDecScannerPtr_t pScanner, unsigned char* pBufStart,
            unsigned char* pBufEnd); // delete the const symbol for reduce compiling warnings!
    unsigned char* (*getPos)(XltDecScannerPtr_t pScanner);

    /* public attributes */
    XltDecTokenPtr_t curtok; /* current token */
    long charset; /* character set as specified in the
     WBXML header */
    char* charsetStr; /* NULL */
    long pubID; /* document public identifier as
     specified in the WBXML header */
    char* pubIDStr; /* pubID as a string - valid only when
     pubID == 0 */
    unsigned int finished; /* set when end of buffer is reached */

    /* private attributes */
    unsigned char* pos; /* current buffer position */
    unsigned char* bufend; /* end of buffer */
    long pubIDIdx; /* strtbl index of the string
     version of the pubID - valid only
     when pubID == 0 */

    XltUtilStackPtr_t tagstack; /* stack of open start tags */

    unsigned char* strtbl; /* copy of the string table */
    long strtbllen; /* length of the string table */

    unsigned char state; /* tag state or attribute state */
    SmlPcdataExtension_t cptag; /* current codepage for tags */
    unsigned char cpattr; /* current codepage for attributes */
    SmlPcdataExtension_t activeExt; /* the active Sub DTD */
};

/* WBXML version that this parser knows */
#define _MAJOR_VERSION    1
#define _MINOR_VERSION    2

#define TAG_STATE 0
#define ATTRIBUTE_STATE 1

short dm_xltDecWbxmlInit(unsigned char* pBufEnd, unsigned char* *ppBufPos,
        XltDecScannerPtr_t *ppScanner); // delete the const symbol for reduce compiling warnings!

#ifdef __USE_EXTENSIONS__
/* prototype for function in xltdecwbxml.c */
void dm_subdtdDecodeWbxml(XltDecoderPtr_t pDecoder, SmlPcdataPtr_t *ppPcdata);
#endif

#endif
