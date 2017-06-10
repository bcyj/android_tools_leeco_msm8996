#ifdef WIN32
#include "std_header.h"
#endif

#include "decodewbxml.h"
#include "smlerr.h"
#include "libmem.h"

#ifdef __SML_WBXML__
/* various checks about wbxml token */
#define HAS_ATTRIBUTES(tag) (*tag & 0x80)
#define HAS_CONTENT(tag) (*tag & 0x40)
#define IDENTITY(tag) (*tag & 0x3F)

#define IS_SWITCH(tok)  (*(tok) == 0x00)
#define IS_END_(tok)     (*(tok) == 0x01)
#define IS_ENTITY(tok)  (*(tok) == 0x02)
#define IS_STR_I(tok)   (*(tok) == 0x03)
#define IS_LITERAL(tok) (IDENTITY(tok) == 0x04)
// Note: gcc cannot parse multi-line macros when file has DOS line ends
#define IS_EXT_I(tok)  ((*(tok) == 0x40) || (*(tok) == 0x41) || (*(tok) == 0x42))
#define IS_PI(tok)      (*(tok) == 0x43)
#define IS_EXT_T(tok)  ((*(tok) == 0x80) || (*(tok) == 0x81) || (*(tok) == 0x82))
#define IS_STR_T(tok)   (*(tok) == 0x83)
#define IS_EXT(tok)    ((*(tok) == 0xC0) || (*(tok) == 0xC1) || (*(tok) == 0xC2))
#define IS_OPAQUE(tok)  (*(tok) == 0xC3)
#define IS_STRING(tok) (IS_STR_I(tok) || IS_STR_T(tok))
#define IS_EXTENSION(tok) (IS_EXT_I(tok) || IS_EXT_T(tok) || IS_EXT(tok))

#define IS_ATTRIBUTE_VALUE(tok) (*(tok) & 0x80)
#define IS_ATTRIBUTE_START(tok) (~IS_ATTRIBUTE_VALUE(tok))

/**
 * Public methods of the scanner interface.
 *
 * Description see XLTDecCom.h.
 */
static short dm_destroy(XltDecScannerPtr_t);
static short dm_nextTok(const char*, int, XltDecScannerPtr_t);
static short dm_pushTok(XltDecScannerPtr_t);
static void dm_setBuf(XltDecScannerPtr_t pScanner, unsigned char*,
        unsigned char*); // delete the const symbol for reduce compiling warnings!
static unsigned char* dm_getPos(XltDecScannerPtr_t);

/**
 * FUNCTION: dm_wbxmlHeader, dm_wbxmlVersion, dm_wbxmlPublicID, dm_wbxmlCharset
 *
 * These functions are used for decoding the WBXML document header.
 * dm_wbxmlHeader is a short wrapper that calls the other four functions in
 * the right order to scan the header. dm_wbxmlStrtbl makes a copy of the
 * string table.
 */
static short dm_wbxmlHeader(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlVersion(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlPublicID(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlCharset(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlStrtbl(wbxmlScannerPrivPtr_t pScanner);

/**
 * FUNCTION: dm_readBytes
 *
 * Advance the current position pointer after checking whether the end of
 * the buffer has been reached. If the end of the buffer has been reached
 * the scanner's finished flag is set.

 * RETURNS:        0, if end of buffer has been reached
 *                 1 otherwise
 */
static unsigned char dm_readBytes(wbxmlScannerPrivPtr_t pScanner, long bytes);

/**
 * FUNCTION: dm_parseInt
 *
 * Decodes multi-byte integers.
 *
 * PRE-Condition:
 *                 pScanner->pos points to the first byte of the mb_int.
 *
 * POST-Condition:
 *                 pScanner->pos points to the last byte of the mb_int.
 */
static short dm_parseInt(wbxmlScannerPrivPtr_t pScanner, long *mbi);

/**
 * FUNCTION: wbxmlXXXToken
 *
 * WBXML extensions, entities, processing instructions and attributes are
 * not supported by this scanner. If one is found it is skipped and
 * processing continues afterwards.
 */
static short dm_wbxmlSkipExtension(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlSkipEntity(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlSkipPI(wbxmlScannerPrivPtr_t);
static short dm_wbxmlSkipAttribute(wbxmlScannerPrivPtr_t);

/**
 * FUNCTION: wbxmlXXXToken
 *
 * Scan the document for the next valid XML/WBXML token as defined in the
 * XLTDecCom header file (e.g. TOK_TAG_START).
 *
 * PRE-Condition:
 *                 pScanner->pos points to the first byte of a valid WBXML
 *                 element (String, Tag, etc.)
 *
 * POST-Condition:
 *                 pScanner->pos points to the last byte of the WBXML
 *                 element;
 *                 pScanner->curtok contains type and tagid or pcdata of
 *                 the token
 */
static short dm_wbxmlStringToken(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlOpaqueToken(wbxmlScannerPrivPtr_t pScanner);
static short dm_wbxmlTagToken(wbxmlScannerPrivPtr_t pScanner);

/**
 * FUNCTION: dm_wbxmlSwitchPage
 *
 * Switch WBXML code page
 */
static short dm_wbxmlSwitchPage(wbxmlScannerPrivPtr_t pScanner);

/* feature func*/

/**
 * FUNCTION: XltDecWbxmlInit
 *
 * Create and initialize a new WBXML scanner. Description see XLTDec.h.
 */
short dm_xltDecWbxmlInit(unsigned char* pBufEnd, unsigned char* *ppBufPos,
        XltDecScannerPtr_t *ppScanner) // delete the const symbol for reduce compiling warnings!
{
    wbxmlScannerPrivPtr_t pScanner;
    short rc;

    /* initialize new WBXML scanner */
    if ((pScanner = (wbxmlScannerPrivPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(wbxmlScannerPriv_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pScanner, 0, sizeof(wbxmlScannerPriv_t));
    pScanner->bufend = pBufEnd;
    pScanner->pos = *ppBufPos;
    if ((pScanner->curtok = (XltDecTokenPtr_t) dm_smlLibMalloc(__FILE__,
            __LINE__, sizeof(XltDecToken_t))) == NULL) {
        dm_smlLibFree(pScanner);
        *ppScanner = NULL;
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    pScanner->curtok->pcdata = NULL;
    if ((rc = dm_xltUtilCreateStack(&pScanner->tagstack, 10)) != SML_ERR_OK) {
        dm_smlLibFree((pScanner->curtok));
        dm_smlLibFree(pScanner);
        *ppScanner = NULL;
        return rc;
    }
    pScanner->state = TAG_STATE;

    /* point public/private methods to the right implementation */
    pScanner->nextTok = dm_nextTok;
    pScanner->destroy = dm_destroy;
    pScanner->pushTok = dm_pushTok;
    pScanner->setBuf = dm_setBuf;
    pScanner->getPos = dm_getPos;

    /* decode WBXML header */
    if ((rc = dm_wbxmlHeader(pScanner)) != SML_ERR_OK) {
        pScanner->destroy((XltDecScannerPtr_t) pScanner);
        *ppScanner = NULL;
        return rc;
    }

    *ppScanner = (XltDecScannerPtr_t) pScanner;

    return SML_ERR_OK;
}

/**
 * FUNCTION: destroy
 *
 * Free memory. Description see XltDecAll.h.
 */
static short dm_destroy(XltDecScannerPtr_t pScanner) {
    wbxmlScannerPrivPtr_t pScannerPriv;

    if (pScanner == NULL)
        return SML_ERR_OK;

    pScannerPriv = (wbxmlScannerPrivPtr_t) pScanner;
    if (pScannerPriv->tagstack != NULL)
        pScannerPriv->tagstack->destroy(pScannerPriv->tagstack);
    dm_smlLibFree((pScannerPriv->curtok));
    dm_smlLibFree((pScannerPriv->strtbl));
    dm_smlLibFree(pScannerPriv);

    return SML_ERR_OK;
}

/**
 * FUNCTION: nextTok
 *
 * Get next token.
 */
static short dm_nextTok(const char * file, int line,
        XltDecScannerPtr_t pScanner) {
    wbxmlScannerPrivPtr_t pScannerPriv;
    short rc;

    pScannerPriv = (wbxmlScannerPrivPtr_t) pScanner;
    // T.K.: chanched Ptr_t to _t
    dm_smlLibMemset(pScanner->curtok, 0, sizeof(XltDecToken_t));
    pScannerPriv->curtok->start = pScannerPriv->pos;

    /* keep going until we find a "supported" element */
    rc = SML_ERR_OK;
    while (rc == SML_ERR_OK) {
        /* skip PIs, extensions and entities... */
        if (IS_PI(pScannerPriv->pos)) {
            rc = dm_wbxmlSkipPI(pScannerPriv);
        } else if (IS_EXTENSION(pScannerPriv->pos)) {
            rc = dm_wbxmlSkipExtension(pScannerPriv);
        } else if (IS_ENTITY(pScannerPriv->pos)) {
            rc = dm_wbxmlSkipEntity(pScannerPriv);

            /* ... decode strings, opaque data and tags */
        } else if (IS_STRING(pScannerPriv->pos)) {
            rc = dm_wbxmlStringToken(pScannerPriv);
            break;
        } else if (IS_OPAQUE(pScannerPriv->pos)) {
            rc = dm_wbxmlOpaqueToken(pScannerPriv);
            break;
        } else {
            rc = dm_wbxmlTagToken(pScannerPriv);
            break;
        }
    }

    return rc;
}

/**
 * FUNCTION: pushTok
 *
 * Reset the scanner to the starting position of the current token within
 * the buffer.
 */
static short dm_pushTok(XltDecScannerPtr_t pScanner) {
    wbxmlScannerPrivPtr_t pScannerPriv;
    XltUtilStackPtr_t pTagStack;
    XltTagID_t tagid;
    short rc = 0;

    pScannerPriv = (wbxmlScannerPrivPtr_t) pScanner;
    pTagStack = pScannerPriv->tagstack;

    if (pScannerPriv->curtok->start == NULL)
        return SML_ERR_WRONG_USAGE;

    /* reset scanner to position where tok begins */
    pScannerPriv->pos = pScannerPriv->curtok->start;

    /* correct the tag stack */
    if (pScannerPriv->curtok->type == TOK_TAG_START) {
        rc = pTagStack->pop(pTagStack, &tagid);
    } else if (pScannerPriv->curtok->type == TOK_TAG_END) {
        tagid = pScannerPriv->curtok->tagid;
        rc = pTagStack->push(pTagStack, tagid);
    }
    if (rc)
        return rc;

    /* invalidate curtok */
    /* T.K. Possible Error. pScannerPriv->curtok is of type XltDecToken_t NOT ...Ptr_t */
    // OrigLine:
    // dm_smlLibMemset(pScannerPriv->curtok, 0, sizeof(XltDecTokenPtr_t);
    pScannerPriv->curtok->type = (XltTokType_t) 0;

    return SML_ERR_OK;
}

static void dm_setBuf(XltDecScannerPtr_t pScanner, unsigned char* pBufStart,
        unsigned char* pBufEnd) // delete the const symbol for reduce compiling warnings!
{
    wbxmlScannerPrivPtr_t pScannerPriv = (wbxmlScannerPrivPtr_t) pScanner;
    pScannerPriv->pos = pBufStart;
    pScannerPriv->bufend = pBufEnd;
}

static unsigned char* dm_getPos(XltDecScannerPtr_t pScanner) {
    return ((wbxmlScannerPrivPtr_t) pScanner)->pos;
}

/**
 * FUNCTION: dm_wbxmlHeader
 *
 * DM_Decode the WBXML header containing version number, document public
 * identifier, character set and a string table.
 */
static short dm_wbxmlHeader(wbxmlScannerPrivPtr_t pScanner) {
    short rc;

    /* decode the WBXML header */
    if ((rc = dm_wbxmlVersion(pScanner)) != SML_ERR_OK)
        return rc;
    if ((rc = dm_wbxmlPublicID(pScanner)) != SML_ERR_OK)
        return rc;
    if ((rc = dm_wbxmlCharset(pScanner)) != SML_ERR_OK)
        return rc;
    if ((rc = dm_wbxmlStrtbl(pScanner)) != SML_ERR_OK)
        return rc;
    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_wbxmlVersion
 *
 * DM_Decode WBXML version. The scanner returns an error if the major version
 * of the document differs from the major version this scanner supports or
 * if the minor version of the document is larger than the minor version
 * the scanner supports.
 */
static short dm_wbxmlVersion(wbxmlScannerPrivPtr_t pScanner) {
    unsigned char major, minor;

    minor = ((unsigned char) (*pScanner->pos & 0x0F));
    major = ((unsigned char) ((*pScanner->pos >> 4) + 1));

    if (major != _MAJOR_VERSION || minor > _MINOR_VERSION)
        return SML_ERR_XLT_INCOMP_WBXML_VERS;

    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_wbxmlPublicID
 *
 * Decodes WBXML Document Public Identifier.
 */
static short dm_wbxmlPublicID(wbxmlScannerPrivPtr_t pScanner) {
    long tmp;
    short rc;

    if (*pScanner->pos != 0) {
        /* pre-defined numeric identifier */
        if ((rc = dm_parseInt(pScanner, &tmp)) != SML_ERR_OK)
            return rc;
        pScanner->pubID = tmp;
        pScanner->pubIDIdx = 0;
    } else {
        /* public id is given as string table entry (which we
         haven't read at this point so we'll save the reference
         for later) */
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
        if ((rc = dm_parseInt(pScanner, &tmp)) != SML_ERR_OK)
            return rc;
        pScanner->pubID = 0;
        pScanner->pubIDIdx = tmp;
    }
    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;
    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_wbxmlCharset
 *
 * DM_Decode WBXML Charset.
 */
static short dm_wbxmlCharset(wbxmlScannerPrivPtr_t pScanner) {
    /* TODO: if charset iformation has to be processed
     it can be done here. For the moment only UTF-8 is used by SyncML */
    long mibenum;
    short rc;

    /* charset is given as a single IANA assigned MIBEnum value */
    if ((rc = dm_parseInt(pScanner, &mibenum)) != SML_ERR_OK)
        return rc;
    pScanner->charset = mibenum;

    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_wbxmlStrtbl
 *
 * Keep a copy of the string table.
 */
static short dm_wbxmlStrtbl(wbxmlScannerPrivPtr_t pScanner) {
    long len;
    short rc;

    if ((rc = dm_parseInt(pScanner, &len)) != SML_ERR_OK)
        return rc;
    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;
    pScanner->strtbllen = len;
    if (len > 0) {
        if (pScanner->pos + len > pScanner->bufend)
            return SML_ERR_XLT_END_OF_BUFFER;
        if ((pScanner->strtbl = dm_smlLibMalloc(__FILE__, __LINE__, len))
                == NULL) {
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
        dm_smlLibMemcpy(pScanner->strtbl, pScanner->pos, len);
        dm_readBytes(pScanner, len);
    } else {
        pScanner->strtbl = NULL;
    }

    /* if the public ID was given as a string table reference save a
     reference to the corresponding string for later */
    if (pScanner->pubID == 0) {
        if (pScanner->pubIDIdx > pScanner->strtbllen)
            return SML_ERR_XLT_INVAL_WBXML_DOC;
        pScanner->pubIDStr = (char*) (pScanner->strtbl + pScanner->pubIDIdx);
    }

    return SML_ERR_OK;
}

/*******************************************************/
// FUNCTION: dm_readBytes
// Advance the position pointer. Description see above.
/*******************************************************/
static unsigned char dm_readBytes(wbxmlScannerPrivPtr_t pScanner, long bytes) {
    if (pScanner->pos + bytes > pScanner->bufend) {
        pScanner->finished = 1;
        return 0;
    }
    pScanner->pos += bytes;
    return 1;
}

static short dm_parseInt(wbxmlScannerPrivPtr_t pScanner, long *mbi) {
    *mbi = 0;
    /* accumulate byte value until continuation flag (MSB) is zero */
    for (;;) {
        *mbi = *mbi << 7;
        *mbi += *(pScanner->pos) & 0x7F;
        if (!(*pScanner->pos & 0x80))
            break;
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
    }
    return SML_ERR_OK;
}

/*******************************************************/
// FUNCTION: dm_wbxmlSkipPI
//Handle XML processing instructions. PIs are not supported but the
// scanner recognizes and skips over them.
/********************************************************/
static short dm_wbxmlSkipPI(wbxmlScannerPrivPtr_t pScanner) {
    /* PIs are just like tag attributes with a special PI token instead
     * of the attribute start token */
    return dm_wbxmlSkipAttribute(pScanner);
}

/*******************************************************/
// FUNCTION: dm_wbxmlSkipAttribute
// Handle attributes. Attributes are not supported but the
// scanner recognizes and skips over them.
/*******************************************************/
static short dm_wbxmlSkipAttribute(wbxmlScannerPrivPtr_t pScanner) {
    XltDecTokenPtr_t oldtok;
    long tmp;
    short rc = 0;

    /* skipping attributes shouldn't change the current token so we
     make a copy... */
    if ((oldtok = (XltDecTokenPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(XltDecToken_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemcpy(oldtok, pScanner->curtok, sizeof(XltDecToken_t));

    /* ... skip until attribute end tag... */
    while (!IS_END_(pScanner->pos)) {
        if (IS_STRING(pScanner->pos)) {
            rc = dm_wbxmlStringToken(pScanner);
            /* avoid memory leak due to this ugly workaround of
             skipping attributes */
            dm_smlLibFree((pScanner->curtok->pcdata));
            pScanner->curtok->pcdata = NULL;
        } else if (IS_EXTENSION(pScanner->pos)) {
            rc = dm_wbxmlSkipExtension(pScanner);
        } else if (IS_ENTITY(pScanner->pos)) {
            rc = dm_wbxmlSkipEntity(pScanner);
        } else if (IS_OPAQUE(pScanner->pos)) {
            rc = dm_wbxmlOpaqueToken(pScanner);
            /* avoid memory leak due to this ugly workaround of
             skipping attributes */
            dm_smlLibFree((pScanner->curtok->pcdata));
            pScanner->curtok->pcdata = NULL;
        } else if (IS_LITERAL(pScanner->pos)) {
            if (!dm_readBytes(pScanner, 1))
                return SML_ERR_XLT_END_OF_BUFFER;
            rc = dm_parseInt(pScanner, &tmp);
            if (!dm_readBytes(pScanner, 1))
                return SML_ERR_XLT_END_OF_BUFFER;
        } else if (IS_SWITCH(pScanner->pos)) {
            rc = dm_wbxmlSwitchPage(pScanner);
        } else {
            if (!dm_readBytes(pScanner, 1))
                return SML_ERR_XLT_END_OF_BUFFER;
        }

        if (rc != SML_ERR_OK) {
            dm_smlLibFree(oldtok);
            return rc;
        }

    }
    /* ... then skip the end tag itself... */
    dm_readBytes(pScanner, 1);

    /* ... and finaly restore our copy of curtok */
    dm_smlLibMemcpy(pScanner->curtok, oldtok, sizeof(XltDecToken_t));
    dm_smlLibFree(oldtok);

    return SML_ERR_OK;
}

/*******************************************************/
// FUNCTION: dm_wbxmlSkipExtension
// DM_Decode WBXML extensions. Skips the extension but doesn't do anything
// useful with it.
/*******************************************************/
static short dm_wbxmlSkipExtension(wbxmlScannerPrivPtr_t pScanner) {
    long tmp;
    short rc;

    if (IS_EXT(pScanner->pos)) {
        /* single byte extension token */
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
    } else if (IS_EXT_I(pScanner->pos)) {
        /* inline string extension token */
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
        if (!dm_readBytes(pScanner, dm_smlLibStrlen((char*) pScanner->pos) + 1))
            return SML_ERR_XLT_END_OF_BUFFER;
    } else {
        /* inline integer extension token */
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
        if ((rc = dm_parseInt(pScanner, &tmp)) != SML_ERR_OK)
            return rc;
        if (!dm_readBytes(pScanner, tmp + 1))
            return SML_ERR_XLT_END_OF_BUFFER;
    }
    return SML_ERR_OK;
}

/*******************************************************/
// FUNCTION: dm_wbxmlSkipEntity
//Skips entities but doesn't do anything useful yet.
/*******************************************************/
static short dm_wbxmlSkipEntity(wbxmlScannerPrivPtr_t pScanner) {
    long tmp;
    short rc;

    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;
    if ((rc = dm_parseInt(pScanner, &tmp)) != SML_ERR_OK)
        return rc;
    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;

    return SML_ERR_OK;
}

static short dm_wbxmlStringToken(wbxmlScannerPrivPtr_t pScanner) {
    SmlPcdataPtr_t pPcdata;
    short rc;

    if ((pPcdata = (SmlPcdataPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlPcdata_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    /* copy the string into the new PCdata struct */
    if (IS_STR_I(pScanner->pos)) {
        /* inline string */
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
        pPcdata->extension = SML_EXT_UNDEFINED;
        pPcdata->contentType = SML_PCDATA_STRING;
        pPcdata->length = dm_smlLibStrlen((char*) pScanner->pos);
        if (pScanner->pos + pPcdata->length + 1 > pScanner->bufend) {
            dm_smlLibFree(pPcdata);
            return SML_ERR_XLT_END_OF_BUFFER;
        }
        if ((pPcdata->content = dm_smlLibMalloc(__FILE__, __LINE__,
                pPcdata->length + 1)) == NULL) {
            dm_smlLibFree(pPcdata);
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
        dm_smlLibStrncpy(pPcdata->content, (char*) pScanner->pos,
                pPcdata->length + 1);
        dm_readBytes(pScanner, pPcdata->length + 1);

    } else {
        /* string table reference */
        long offset; /* offset into string table */
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
        if ((rc = dm_parseInt(pScanner, &offset)) != SML_ERR_OK) {
            dm_smlLibFree(pPcdata);
            return rc;
        }
        if (offset >= pScanner->strtbllen) {
            dm_smlLibFree(pPcdata);
            return SML_ERR_XLT_INVAL_WBXML_DOC;
        }
        pPcdata->contentType = SML_PCDATA_STRING;
        pPcdata->length = dm_smlLibStrlen((char*) (pScanner->strtbl + offset));
        if ((pPcdata->content = dm_smlLibMalloc(__FILE__, __LINE__,
                pPcdata->length + 1)) == NULL) {
            dm_smlLibFree(pPcdata);
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
        dm_smlLibStrncpy(pPcdata->content, (char*) (pScanner->strtbl + offset),
                pPcdata->length + 1);
        dm_readBytes(pScanner, 1);
    }

    pScanner->curtok->pcdata = pPcdata;

    pScanner->curtok->type = TOK_CONT;

    return SML_ERR_OK;
}

static short dm_wbxmlOpaqueToken(wbxmlScannerPrivPtr_t pScanner) {
    SmlPcdataPtr_t pPcdata = NULL;
    long len;
    short rc;

    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;

    /* a mbi indicates the length of the opaque data block that we'll
     copy into new PCdata struct */
    if ((rc = dm_parseInt(pScanner, &len)) != SML_ERR_OK)
        return rc;
    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;
    if (pScanner->pos + len > pScanner->bufend)
        return SML_ERR_XLT_END_OF_BUFFER;
    if ((pPcdata = (SmlPcdataPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlPcdata_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    pPcdata->extension = SML_EXT_UNDEFINED;
    pPcdata->contentType = SML_PCDATA_OPAQUE;
    pPcdata->length = len;
    /* Modification 2001-07-03 by Luz %%%%%:
     * made sure that content is one null byte longer
     * than indicated opaque content, such that strings that are coded as
     * opaque (happens to be the case with Nokia 9210) can still be read
     * as C-string without need for an intermediate buffer
     */
    if ((pPcdata->content = dm_smlLibMalloc(__FILE__, __LINE__, len + 1))
            == NULL) {
        dm_smlLibFree(pPcdata);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    ((char *) pPcdata->content)[len] = 0; /* make sure there is a c-string terminator */
    /* end modification */

    dm_smlLibMemcpy(pPcdata->content, pScanner->pos, len);
    pScanner->curtok->pcdata = pPcdata;

    dm_readBytes(pScanner, len);

    pScanner->curtok->type = TOK_CONT;

    return SML_ERR_OK;
}

/*******************************************************/
// FUNCTION: dm_wbxmlSwitchPage
// Switch WBXML code page.
/*******************************************************/
/* T.K. 06.02.01
 * We need to enhance this as soon as we introduce
 * Sub DTD's with more than one WBXML codepage. But till then
 * there is only one case where WBXML codepages can occure, and
 * this is the MetInf Sub DTD. So in case we find a codepage switch
 * to something other than codepage zero, we set the active extension
 * to metinf.
 * In future versions the pScanner needs to be enhanced, to translate
 * codepageswitches context sensitive to the active extension.
 */
static short dm_wbxmlSwitchPage(wbxmlScannerPrivPtr_t pScanner) {
    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;
    if (pScanner->state == TAG_STATE)
        pScanner->cptag = (SmlPcdataExtension_t) *pScanner->pos;
    else
        pScanner->cpattr = *pScanner->pos;
    dm_readBytes(pScanner, 1);
    /* T.K. this needs to be adjusted as described above */
    if (pScanner->cpattr != 0 || pScanner->cptag != 0)
        pScanner->activeExt = SML_EXT_METINF;
    else
        pScanner->activeExt = SML_EXT_UNDEFINED;
    return SML_ERR_OK;
}

static short dm_wbxmlTagToken(wbxmlScannerPrivPtr_t pScanner) {
    XltTagID_t tagid;
    unsigned char has_cont, has_attr;
    short rc;

    if (IS_SWITCH(pScanner->pos)) {
        if ((rc = dm_wbxmlSwitchPage(pScanner)) != SML_ERR_OK)
            return rc;
    }

    /* we have to look at the top of the tagstack to see which
     start tag an end tag belongs to */
    if (IS_END_(pScanner->pos)) {
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
        pScanner->curtok->type = TOK_TAG_END;
        rc = pScanner->tagstack->pop(pScanner->tagstack, &tagid);
        if (rc == SML_ERR_WRONG_USAGE)
            return SML_ERR_XLT_INVAL_WBXML_DOC;
        else if (rc)
            return rc;
        pScanner->curtok->tagid = tagid;
        return SML_ERR_OK;
    }

    /* look at the two MSB: does this tag have content or attributes? */

    has_cont = ((unsigned char) (HAS_CONTENT(pScanner->pos)));
    has_attr = ((unsigned char) (HAS_ATTRIBUTES(pScanner->pos)));

    /* look up tag ID either by string or by number */
    if (IS_LITERAL(pScanner->pos)) {
        long offset; /* offset into the string table */
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
        if ((rc = dm_parseInt(pScanner, &offset)) != SML_ERR_OK)
            return rc;
        if (offset > pScanner->strtbllen)
            return SML_ERR_XLT_INVAL_WBXML_DOC;

        rc = (short) dm_getTagIDByStringAndExt(
                (char*) (pScanner->strtbl + offset), pScanner->activeExt,
                &tagid);
        if ((tagid == TN_UNDEF) || (rc != SML_ERR_OK))
            return rc;

    } else {
        rc = (short) dm_getTagIDByByteAndExt(
                (unsigned char) IDENTITY(pScanner->pos), pScanner->activeExt,
                &tagid);
        if ((tagid == TN_UNDEF) || (rc != SML_ERR_OK))
            return rc;

    }

    /* we know everything we need to know */
    pScanner->curtok->tagid = tagid;
    pScanner->curtok->type = has_cont ? TOK_TAG_START : TOK_TAG_EMPTY;
#ifdef __USE_METINF__
    pScanner->curtok->ext =
            pScanner->cptag == 0 ? SML_EXT_UNDEFINED : SML_EXT_METINF;
#else
    pScanner->curtok->ext = SML_EXT_UNDEFINED;
#endif

    if (!dm_readBytes(pScanner, 1))
        return SML_ERR_XLT_END_OF_BUFFER;

    /* push tag onto tagstack unless this tag is empty */
    if (has_cont) {
        if ((rc = pScanner->tagstack->push(pScanner->tagstack, tagid))
                != SML_ERR_OK)
            return rc;
    }

    /* skip attributes */
    if (has_attr) {
        pScanner->state = ATTRIBUTE_STATE;
        if ((rc = dm_wbxmlSkipAttribute(pScanner)) != SML_ERR_OK)
            return rc;
        pScanner->state = TAG_STATE;
    }

    return SML_ERR_OK;
}

#ifdef __USE_EXTENSIONS__
/*******************************************************/
//This function tries to decode an inlined WBXML document inside
//an PCDATA element.
//In case of failing to decode it the PCDATA element isn't changed
// at all.
/*******************************************************/

void dm_subdtdDecodeWbxml(XltDecoderPtr_t pDecoder, SmlPcdataPtr_t *ppPcdata) {
    short _err = SML_ERR_OK;
    unsigned char* pSubBuf = NULL;
    SmlPcdataPtr_t pSubPcdata = NULL;
    XltDecoderPtr_t pSubDecoder = NULL;
#ifdef __USE_DEVINF__
    wbxmlScannerPrivPtr_t pScannerPriv = NULL;
#endif

    /* some sanity checks at first */

    if (*ppPcdata == NULL) {
        if (pDecoder) /* use this rare case to remove warning */
        {
        }
        return;
    }

    if ((*ppPcdata)->contentType != SML_PCDATA_OPAQUE)
        return;

    // now create a sub buffer
    pSubBuf = (unsigned char*) dm_smlLibMalloc(__FILE__, __LINE__,
            (*ppPcdata)->length);
    if (pSubBuf == NULL)
        return;
    dm_smlLibMemset(pSubBuf, 0x00, (*ppPcdata)->length);
    dm_smlLibMemmove(pSubBuf, (*ppPcdata)->content, (*ppPcdata)->length);

    /* ok looks fine sofar - now lets decode the rest */
    /* now lets create a decoder, but without parsing the SyncML
     * start tags (because it's not there) and skip the XML
     * part as we don't need it.
     */
    pSubDecoder = (XltDecoderPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(XltDecoder_t));
    if (pSubDecoder == NULL) {
        dm_smlLibFree(pSubBuf);
        return;
    }
    pSubDecoder->finished = 0;
    pSubDecoder->final = 0;
    pSubDecoder->scanner = NULL;
    if (dm_xltUtilCreateStack(&pSubDecoder->tagstack, 10) != SML_ERR_OK) {
        dm_smlLibFree(pSubDecoder);
        dm_smlLibFree(pSubBuf);
        return;
    }
    if (dm_xltDecWbxmlInit(pSubBuf + (*ppPcdata)->length, &pSubBuf,
            &pSubDecoder->scanner) != SML_ERR_OK) {
        dm_xltDecTerminate(pSubDecoder);
        dm_smlLibFree(pSubBuf);
        return;
    }
    pSubDecoder->charset = pSubDecoder->scanner->charset;
    pSubDecoder->charsetStr = NULL;

    pSubPcdata = (SmlPcdataPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlPcdata_t));
    if (pSubPcdata == NULL) {
        dm_xltDecTerminate(pSubDecoder);
        dm_smlLibFree(pSubPcdata);
        dm_smlLibFree(pSubBuf);
        return;
    }
    /* T.K.
     * In the future we need to check the WBXML stringtable and
     * switch into the right Sub DTD. But sofar only DevInf is
     * supported so we can save time and space
     */
    /* T.K.
     * To prevent buffer corruption when __USE_DEVINF__ is not used
     * we initialize _err with any errorcode != OK, and this way
     * force the function to exit without modifying the ppPcdata
     */
    _err = SML_ERR_UNSPECIFIC;
#ifdef __USE_DEVINF__
    pSubPcdata->contentType = SML_PCDATA_EXTENSION;
    pSubPcdata->extension = SML_EXT_DEVINF;
    pSubPcdata->length = 0;
    pSubPcdata->content = NULL;

    pScannerPriv = (wbxmlScannerPrivPtr_t) pSubDecoder->scanner;
    pScannerPriv->activeExt = SML_EXT_DEVINF;
    pScannerPriv->cpattr = 0;
    pScannerPriv->cptag = (SmlPcdataExtension_t) 0;
    dm_smlLibMemset(pScannerPriv->curtok, 0, sizeof(XltDecToken_t));

    _err = dm_buildDevInfDevInfCmd(pSubDecoder, (void*) &pSubPcdata->content);
#endif

    if (_err != SML_ERR_OK) {
        dm_xltDecTerminate(pSubDecoder);
        dm_smlLibFree(pSubPcdata);
        dm_smlLibFree(pSubBuf);
        return;
    }

    /* parsing is done, now lets anchor it within the original PCDATA element */
    dm_smlFreePcdata(__FILE__, __LINE__, *ppPcdata);
    *ppPcdata = pSubPcdata;

    /* we are done */
    dm_xltDecTerminate(pSubDecoder);
    dm_smlLibFree(pSubBuf);

    return;
}

#endif

#endif
