#ifdef WIN32
#include "std_header.h"
#endif
#include "decodexml.h"
#include "smlerr.h"
#include "libmem.h"

/**
 * Public methods of the scanner interface.
 *
 * Description see XLTDecCom.h.
 */
static short dm_destroy(XltDecScannerPtr_t);
static short dm_nextTok(const char*, int, XltDecScannerPtr_t);
static short dm_pushTok(XltDecScannerPtr_t);
static void dm_setBuf(XltDecScannerPtr_t, unsigned char*, unsigned char*); // delete the const symbol for reduce compiling warnings!
static unsigned char* dm_getPos(XltDecScannerPtr_t);

/**
 * FUNCTION: dm_readBytes
 *
 * Advance the current position pointer after checking whether the end of
 * the buffer has been reached. If the end of the buffer has been reached
 * the scanner's finished flag is set.
 *
 * PRE-Condition:
 * POST-Condition:
 *
 * IN:             bytes, read this many bytes
 *
 * IN/OUT:         pScanner, the scanner
 *
 * RETURNS:        1, if end of buffer has not been reached
 *                 0 otherwise
 */
static unsigned char dm_readBytes(xmlScannerPrivPtr_t pScanner, long bytes);

/**
 * Skip whitespaces.
 */
static void dm_skipS(xmlScannerPrivPtr_t pScanner);

static short dm_xmlTag(xmlScannerPrivPtr_t pScanner, unsigned char endtag);
static short dm_xmlName(xmlScannerPrivPtr_t pScanner, char* *name);
static short dm_xmlCharData(xmlScannerPrivPtr_t pScanner);
static short dm_xmlProlog(xmlScannerPrivPtr_t pScanner);
static short dm_xmlDocTypeDecl(xmlScannerPrivPtr_t pScanner);
static short dm_xmlXMLDecl(xmlScannerPrivPtr_t pScanner);
static short dm_xmlAttribute(xmlScannerPrivPtr_t pScanner, char* *name,
        char* *value);
static short dm_xmlStringConst(xmlScannerPrivPtr_t pScanner, char* *value);

static short dm_xmlSkipPCDATA(const char * file, int line,
        xmlScannerPrivPtr_t pScanner);
static short dm_xmlSkipComment(xmlScannerPrivPtr_t pScanner);
static short dm_xmlSkipAttributes(xmlScannerPrivPtr_t pScanner);
static short dm_xmlSkipPI(xmlScannerPrivPtr_t pScanner);
static short dm_xmlCDATA(xmlScannerPrivPtr_t pScanner);
static unsigned char dm_isPcdata(XltTagID_t tagid);

/*************************************************************************/
/* External Functions                                                    */
/*************************************************************************/

short dm_xltDecXmlInit(unsigned char* pBufEnd, unsigned char* *ppBufStart,
        XltDecScannerPtr_t *ppScanner) // delete the const symbol for reduce compiling warnings!
{
    xmlScannerPrivPtr_t pScanner;
    short rc;

    pScanner = (xmlScannerPrivPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(xmlScannerPriv_t));
    if (pScanner == NULL) {
        *ppScanner = NULL;
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    pScanner->finished = 0;
    pScanner->pos = *ppBufStart;
    pScanner->bufend = pBufEnd;
    pScanner->curtok = (XltDecTokenPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(XltDecToken_t));
    if (pScanner->curtok == NULL) {
        dm_smlLibFree(pScanner);
        *ppScanner = NULL;
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    pScanner->curtok->pcdata = NULL;
    pScanner->curtok->tagid = TN_UNDEF;
    pScanner->pubID = 0;
    pScanner->pubIDStr = NULL;
    pScanner->charset = 0;
    pScanner->charsetStr = NULL;
    pScanner->ext = SML_EXT_UNDEFINED;
    pScanner->prev_ext = (SmlPcdataExtension_t) 255;
    pScanner->ext_tag = TN_UNDEF;
    pScanner->prev_ext_tag = TN_UNDEF;
    pScanner->nsprelen = 0;
    pScanner->nsprefix = NULL;

    /* point public/private methods to the right implementation */
    pScanner->nextTok = dm_nextTok;
    pScanner->destroy = dm_destroy;
    pScanner->pushTok = dm_pushTok;
    pScanner->setBuf = dm_setBuf;
    pScanner->getPos = dm_getPos;

    if ((rc = dm_xmlProlog(pScanner)) != SML_ERR_OK) {
        dm_smlLibFree((pScanner->curtok));
        dm_smlLibFree(pScanner);
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
    xmlScannerPrivPtr_t pScannerPriv;

    if (pScanner == NULL)
        return SML_ERR_OK;

    pScannerPriv = (xmlScannerPrivPtr_t) pScanner;
#if 0
    if (NULL != pScannerPriv->curtok)
    {
        dm_smlFreePcdata(__FILE__, __LINE__, pScannerPriv->curtok->pcdata);
        pScannerPriv->curtok->pcdata = NULL;
        //      dm_smlLibFree(pScannerPriv->curtok->start);
    }
#endif
    dm_smlLibFree((pScannerPriv->curtok));
    dm_smlLibFree((pScannerPriv->charsetStr));
    dm_smlLibFree((pScannerPriv->pubIDStr));
    dm_smlLibFree(pScannerPriv);

    return SML_ERR_OK;
}

/**
 * FUNCTION: nextTok
 *
 * Get next token. Description see XltDecAll.h.
 */
static short dm_nextTok(const char * file, int line,
        XltDecScannerPtr_t pScanner) {
    xmlScannerPrivPtr_t pScannerPriv;
    short rc;
    pScannerPriv = (xmlScannerPrivPtr_t) pScanner;
    pScannerPriv->curtok->start = pScannerPriv->pos;
    dm_skipS(pScannerPriv);
    /* skip unsupported elements until we find a supported one */
    rc = 0;

    while (!rc) {
        if (dm_smlLibStrncmp((char*) pScannerPriv->pos, "<!--", 4) == 0) {
            rc = dm_xmlSkipComment(pScannerPriv);
        } else if (dm_smlLibStrncmp((char*) pScannerPriv->pos, "<?", 2) == 0) {
            rc = dm_xmlSkipPI(pScannerPriv);
        } else if (dm_smlLibStrncmp((char*) pScannerPriv->pos, "</", 2) == 0) {
            rc = dm_xmlTag(pScannerPriv, 1);
            break;
        } else if (dm_smlLibStrncmp((char*) pScannerPriv->pos, "<![CDATA[", 9)
                == 0) {
            rc = dm_xmlCDATA(pScannerPriv);
            break;
        } else if ((dm_isPcdata(pScannerPriv->curtok->tagid))
                && (pScannerPriv->curtok->type != TOK_TAG_END)) {
            rc = dm_xmlSkipPCDATA(file, line, pScannerPriv);
            break;
        } else if (dm_smlLibStrncmp((char*) pScannerPriv->pos, "<", 1) == 0) {
            rc = dm_xmlTag(pScannerPriv, 0);
            break;
        } else {
            rc = dm_xmlCharData(pScannerPriv);
            break;
        }
    }
    if (rc)
        return rc;

    return SML_ERR_OK;
}

/**
 * FUNCTION: pushTok
 *
 * Reset the scanner to the starting position of the current token within
 * the buffer. Description see XltDecAll.h.
 */
static short dm_pushTok(XltDecScannerPtr_t pScanner) {
    xmlScannerPrivPtr_t pScannerPriv;

    pScannerPriv = (xmlScannerPrivPtr_t) pScanner;
    pScannerPriv->pos = pScannerPriv->curtok->start;

    /* invalidate curtok */
    /* T.K. Possible Error. pScannerPriv->curtok is of type XltDecToken_t NOT ...Ptr_t */
    // OrigLine:
    // dm_smlLibMemset(pScannerPriv->curtok, 0, sizeof(XltDecTokenPtr_t);
    pScannerPriv->curtok->type = (XltTokType_t) 0;

    return SML_ERR_OK;
}

/**
 * FUNCTION: setBuf
 *
 * Set the working buffer of the scanner.
 */
static void dm_setBuf(XltDecScannerPtr_t pScanner, unsigned char* pBufStart,
        unsigned char* pBufEnd) // delete the const symbol for reduce compiling warnings!
{
    xmlScannerPrivPtr_t pScannerPriv = (xmlScannerPrivPtr_t) pScanner;
    pScannerPriv->pos = pBufStart;
    pScannerPriv->bufend = pBufEnd;
}

/**
 * FUNCTION: getPos
 *
 * Get the current position of the scanner within its working buffer.
 */
static unsigned char* dm_getPos(XltDecScannerPtr_t pScanner) {
    return ((xmlScannerPrivPtr_t) pScanner)->pos;
}

/*************************************************************************/
/* Internal Functions                                                    */
/*************************************************************************/

/**
 * FUNCTION: dm_readBytes
 *
 * Advance the position pointer. Description see above.
 */
static unsigned char dm_readBytes(xmlScannerPrivPtr_t pScanner, long bytes) {
    if (pScanner->pos + bytes > pScanner->bufend) {
        pScanner->finished = 1;
        return 0;
    }
    pScanner->pos += bytes;
    return 1;
}

/**
 * FUNCTION: dm_skipS
 *
 * Skip whitespace.
 */
static void dm_skipS(xmlScannerPrivPtr_t pScanner) {
    for (;;) {
        switch (*pScanner->pos) {
        case 9: /* tab stop */
        case 10: /* line feed */
        case 13: /* carriage return */
        case 32: /* space */
            // %%% luz: 2001-07-03: added exit from loop if no more bytes
            if (!dm_readBytes(pScanner, 1))
                return;
            break;
        default:
            return;
        }
    }
}

/**
 * FUNCTION: dm_xmlTag
 *
 * Handle XML Tags
 */
static short dm_xmlTag(xmlScannerPrivPtr_t pScanner, unsigned char endtag) {
    short rc;
    char* name;
    char* attname = NULL;
    char* value = NULL;
    char* nsprefix = NULL;
    unsigned char nsprelen = 0;
    XltTagID_t tagid;
    SmlPcdataExtension_t ext;

    if (endtag) {
        if (!dm_readBytes(pScanner, 2))
            return SML_ERR_XLT_END_OF_BUFFER;
    } else {
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;
    }

    if ((rc = dm_xmlName(pScanner, &name)) != SML_ERR_OK) {
        if (rc != SML_ERR_NOT_ENOUGH_SPACE) {
            return SML_ERR_XLT_INVAL_XML_DOC;
        } else {
            return rc;
        }

    }

    ext = pScanner->ext;
    if (!endtag) {
        /* Namespaces can only be defined on start, never on endtags
         * but we have to make sure we close a namespace on the corrsponding endtag.
         * Thats why we a) only open a namespace when it differs from the previous one, and
         * b) record the tag_id that opend the namespace so we can close it when the
         * corresponding endtag is reached.
         */

        if ((rc = dm_xmlAttribute(pScanner, &attname, &value)) == SML_ERR_OK) {
            if (dm_smlLibStrncmp(attname, "xmlns", 5) == 0) {
                /* Heureka we found a Namespace :-) */
                /* It's save to check attname[5] here, as it contains at least the terminating '\000' */
                if (attname[5] == ':') { /* we found a namespace prefixdefinition */
                    nsprelen = (unsigned char) dm_smlLibStrlen(&attname[6]);
                    nsprefix = dm_smlLibMalloc(__FILE__, __LINE__,
                            nsprelen + 1);
                    if (nsprefix == NULL) {
                        dm_smlLibFree(attname);
                        dm_smlLibFree(value);
                        dm_smlLibFree(name);
                        return SML_ERR_NOT_ENOUGH_SPACE;
                    }
                    dm_smlLibStrcpy(nsprefix, &attname[6]);
                }
                ext = dm_getExtByName(value);
                if (ext == SML_EXT_MAX) {/*lint !e650 !e774 */
                    dm_smlLibFree(nsprefix); /* doesn't harm, even when empty */
                    dm_smlLibFree(attname);
                    dm_smlLibFree(value);
                    dm_smlLibFree(name);
                    return SML_ERR_XLT_INVALID_CODEPAGE;
                }
            } else {/*lint !e774*/
#if 0//del  2009.12.19  moidfy for warning
                if (rc == SML_ERR_NOT_ENOUGH_SPACE) {/*lint !e774*/
                    dm_smlLibFree(attname);
                    dm_smlLibFree(value);
                    dm_smlLibFree(name);
                    return SML_ERR_NOT_ENOUGH_SPACE;
                }
                else
#endif
                {
                    /* we found an unknown attribute -> bail out */
                    dm_smlLibFree(attname);
                    dm_smlLibFree(value);
                    /* nsprefix is empty here so we save us a function call */
                    dm_smlLibFree(name);
                    return SML_ERR_XLT_INVAL_XML_DOC;
                }
            }

        } else if (rc != SML_ERR_XLT_MISSING_CONT) {
            /* dm_xmlAttribute returns an SML_ERR_XLT_MISSING_CONT error when
             * no attribute was found. This is not an error, but everything else is.
             */
            dm_smlLibFree(value);
            dm_smlLibFree(name);
            return rc;
        }

    }

    if (pScanner->ext == ext) {
        /* no new Namespace found - lets proceed with the active one */

        /* first lets check wether a tag is in the right namespace, in case
         * we are using namespaces with prefix notation ('mi:Format' instead of
         * 'Format nsattr="...").
         * If we are and the token is not in this namespace -> bail out
         */
        if (pScanner->nsprelen > 0
                && dm_smlLibStrlen(name) > pScanner->nsprelen + 1) {
            if (name[pScanner->nsprelen] != ':'
                    || dm_smlLibStrncmp(name, pScanner->nsprefix,
                            pScanner->nsprelen) != 0) {
                dm_smlLibFree(name);
                dm_smlLibFree(attname);
                dm_smlLibFree(value);
                dm_smlLibFree(nsprefix);
                return SML_ERR_XLT_NO_MATCHING_CODEPAGE;
            }
        }
        /* Strip off namespace prefixes and ':' to find the tag.
         * If no prefix is defined (pScanner->nsprelen == 0) take the whole tagname.
         */
        if (pScanner->nsprelen > 0)
            rc = dm_getTagIDByStringAndExt(&name[0 + pScanner->nsprelen + 1],
                    pScanner->ext, &tagid);
        else
            rc = dm_getTagIDByStringAndExt(name, pScanner->ext, &tagid);
    } else {
        /* we have a new Namespace */
        if (nsprelen > 0 && dm_smlLibStrlen(name) > nsprelen + 1) {
            if (name[nsprelen] != ':'
                    || dm_smlLibStrncmp(name, nsprefix, nsprelen) != 0) {
                dm_smlLibFree(name);
                dm_smlLibFree(attname);
                dm_smlLibFree(value);
                dm_smlLibFree(nsprefix);
                return SML_ERR_XLT_NO_MATCHING_CODEPAGE;
            }
        }
        /* Strip off namespace prefixes and ':' to find the tag.
         * If no prefix is defined (pScanner->nsprelen == 0) take the whole tagname.
         */
        if (nsprelen > 0)
            rc = dm_getTagIDByStringAndExt(&name[nsprelen + 1], ext, &tagid);
        else
            rc = dm_getTagIDByStringAndExt(name, ext, &tagid);
    }
    /* free temporary buffers */
    dm_smlLibFree(name);
    dm_smlLibFree(attname);
    dm_smlLibFree(value);

    if ((tagid == TN_UNDEF) || (rc != SML_ERR_OK)) {
        dm_smlLibFree(nsprefix);
        return rc;
    }

    /* remember the old extension including the corresponding start tag if we found a new one */
    if (ext != pScanner->ext) { /* namespace changed */
        pScanner->prev_ext = pScanner->ext; /* remember the old ext */
        pScanner->prev_ext_tag = pScanner->ext_tag; /* and the corresponding start tag */
        pScanner->ext = ext;
        pScanner->ext_tag = tagid;
        dm_smlLibFree((pScanner->nsprefix));
        pScanner->nsprefix = nsprefix;
        pScanner->nsprelen = nsprelen;
    }

    pScanner->curtok->tagid = tagid;
    pScanner->curtok->ext = pScanner->ext;
    dm_skipS(pScanner);

    if (endtag) {
        /* found end tag */
        if (dm_smlLibStrncmp((char*) pScanner->pos, ">", 1) != 0)
            return SML_ERR_XLT_INVAL_XML_DOC;
        pScanner->curtok->type = TOK_TAG_END;
        dm_readBytes(pScanner, 1);
        /* in case of an endtag we might need to close the current CP */
        if (tagid == pScanner->ext_tag) {
            pScanner->ext_tag = pScanner->prev_ext_tag;
            pScanner->ext = pScanner->prev_ext;
            pScanner->prev_ext = SML_EXT_UNDEFINED;
            pScanner->prev_ext_tag = TN_UNDEF;
            pScanner->nsprelen = 0;
            dm_smlLibFree((pScanner->nsprefix));
            pScanner->nsprefix = NULL;
        }
    } else {
        /* Attributes are not supported in SyncML -> skip them*/
        if ((rc = dm_xmlSkipAttributes(pScanner)) != SML_ERR_OK)
            return rc;

        if (dm_smlLibStrncmp((char*) pScanner->pos, "/>", 2) == 0) {
            /* found empty tag */
            pScanner->curtok->type = TOK_TAG_EMPTY;
            dm_readBytes(pScanner, 2);
        } else if (dm_smlLibStrncmp((char*) pScanner->pos, ">", 1) == 0) {
            pScanner->curtok->type = TOK_TAG_START;
            dm_readBytes(pScanner, 1);
        } else {
            return SML_ERR_XLT_INVAL_XML_DOC;
        }
    }

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlName
 *
 * Handle Name Elements
 */
static short dm_xmlName(xmlScannerPrivPtr_t pScanner, char* *name) {
    unsigned char* begin;
    char* tmp;
    int len;

    begin = pScanner->pos;
    while (((*pScanner->pos >= 'a') && (*pScanner->pos <= 'z'))
            || ((*pScanner->pos >= 'A') && (*pScanner->pos <= 'Z'))
            || ((*pScanner->pos >= '0') && (*pScanner->pos <= '9'))
            || (*pScanner->pos == '.') || (*pScanner->pos == '-')
            || (*pScanner->pos == '_') || (*pScanner->pos == ':'))
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;

    len = pScanner->pos - begin;
    /* T.K. bail out if len is zero without modifying name */
    if (len == 0)
        return SML_ERR_OK;

    tmp = (char*) dm_smlLibMalloc(__FILE__, __LINE__, len + 1);
    if (tmp == NULL) {
        *name = NULL;
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(tmp, 0, len + 1);
    dm_smlLibStrncpy(tmp, (char*) begin, len);
    *name = tmp;
    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlCharData
 *
 * Handle Pcdata character data content
 */
static short dm_xmlCharData(xmlScannerPrivPtr_t pScanner) {
    SmlPcdataPtr_t pPCData;
    unsigned char* begin;
    int len;

    pPCData = (SmlPcdataPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlPcdata_t));
    if (pPCData == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    pPCData->contentType = SML_PCDATA_UNDEFINED;
    pPCData->length = 0;
    pPCData->content = NULL;

    begin = pScanner->pos;

    if (*pScanner->pos >= *pScanner->bufend) {
        pPCData->content = NULL;
        pPCData->contentType = SML_PCDATA_UNDEFINED;
        pPCData->extension = SML_EXT_UNDEFINED;
        pPCData->length = 0;
        pScanner->curtok->type = TOK_CONT;
        pScanner->curtok->pcdata = pPCData;
        //dm_smlLibFree(pPCData);
        return SML_ERR_XLT_END_OF_BUFFER;
    }

    while (*pScanner->pos != '<') /* && (*pScanner->pos != '&') */
    {
        if (pScanner->pos >= pScanner->bufend) {
            dm_smlLibFree(pPCData);
            return SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (!dm_readBytes(pScanner, 1)) {
            dm_smlLibFree(pPCData);
            return SML_ERR_XLT_END_OF_BUFFER;
        }

    }
    len = pScanner->pos - begin;
    pPCData->content = dm_smlLibMalloc(__FILE__, __LINE__, len + 1);
    if (pPCData->content == NULL) {
        dm_smlLibFree(pPCData);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(pPCData->content, 0, len + 1);
    dm_smlLibMemcpy(pPCData->content, begin, len);
    pPCData->contentType = SML_PCDATA_STRING;
    pPCData->length = len;

    pScanner->curtok->type = TOK_CONT;
    pScanner->curtok->pcdata = pPCData;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlProlog
 *
 * Scan the XML prolog (might be empty...).
 */
static short dm_xmlProlog(xmlScannerPrivPtr_t pScanner) {
    short rc;

    if (pScanner->pos + 5 > pScanner->bufend)
        return SML_ERR_OK;
    if (dm_smlLibStrncmp((char*) pScanner->pos, "<?xml", 5) == 0)
        if ((rc = dm_xmlXMLDecl(pScanner)) != SML_ERR_OK)
            return rc;

    dm_skipS(pScanner);

    while ((pScanner->pos + 4 <= pScanner->bufend)
            && ((dm_smlLibStrncmp((char*) pScanner->pos, "<!--", 4) == 0)
                    || (dm_smlLibStrncmp((char*) pScanner->pos, "<?", 2) == 0))) {
        if (dm_smlLibStrncmp((char*) pScanner->pos, "<!--", 4) == 0)
            rc = dm_xmlSkipComment(pScanner);
        else
            rc = dm_xmlSkipPI(pScanner);
        if (rc != SML_ERR_OK)
            return rc;
        dm_skipS(pScanner);
    }

    if ((pScanner->pos + 9 <= pScanner->bufend)
            && (dm_smlLibStrncmp((char*) pScanner->pos, "<!DOCTYPE", 9) == 0))
        if ((rc = dm_xmlDocTypeDecl(pScanner)) != SML_ERR_OK)
            return rc;

    dm_skipS(pScanner);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlDocTypeDecl
 *
 * Part of the Prolog scanning
 */
static short dm_xmlDocTypeDecl(xmlScannerPrivPtr_t pScanner) {
    short rc;
    char* name = NULL;
    char* syslit = NULL;
    char* publit = NULL;

    dm_readBytes(pScanner, 9);
    dm_skipS(pScanner);
    if ((rc = dm_xmlName(pScanner, &name)) != SML_ERR_OK) {
        dm_smlLibFree(name);
        return rc;
    }
    dm_skipS(pScanner);

    /* parse ExternalID */
    if ((pScanner->pos + 6 <= pScanner->bufend)
            && (dm_smlLibStrncmp((char*) pScanner->pos, "SYSTEM", 6) == 0)) {
        dm_readBytes(pScanner, 6);
        dm_skipS(pScanner);
        if ((rc = dm_xmlStringConst(pScanner, &syslit)) != SML_ERR_OK) {
            dm_smlLibFree(name);
            dm_smlLibFree(syslit);
            return rc;
        }
    } else if ((pScanner->pos + 6 <= pScanner->bufend)
            && (dm_smlLibStrncmp((char*) pScanner->pos, "PUBLIC", 6) == 0)) {
        dm_readBytes(pScanner, 6);
        dm_skipS(pScanner);
        if ((rc = dm_xmlStringConst(pScanner, &publit)) != SML_ERR_OK) {
            dm_smlLibFree(name);
            dm_smlLibFree(publit);
            return rc;
        }
        dm_skipS(pScanner);
        if ((rc = dm_xmlStringConst(pScanner, &syslit)) != SML_ERR_OK) {
            dm_smlLibFree(name);
            dm_smlLibFree(syslit);
            dm_smlLibFree(publit);
            return rc;
        }
    }

    dm_smlLibFree(name);
    dm_smlLibFree(syslit);
    dm_smlLibFree(publit);

    dm_skipS(pScanner);

    if (*pScanner->pos != '>')
        return SML_ERR_XLT_INVAL_XML_DOC;
    dm_readBytes(pScanner, 1);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlXMLDecl
 *
 * Part of the Prolog scanning
 */
static short dm_xmlXMLDecl(xmlScannerPrivPtr_t pScanner) {
    char* name;
    char* value;
    short rc;

    dm_readBytes(pScanner, 5);
    dm_skipS(pScanner);

    /* mandatory version info */
    if ((rc = dm_xmlAttribute(pScanner, &name, &value)) != SML_ERR_OK)
        return rc;
    if (dm_smlLibStrcmp(name, "version") != 0) {
        dm_smlLibFree(name);
        dm_smlLibFree(value);
        return SML_ERR_XLT_INVAL_XML_DOC;
    }
    dm_smlLibFree(name);
    dm_smlLibFree(value);

    dm_skipS(pScanner);

    /* optional attributes are encoding and standalone */
    while ((pScanner->pos + 2 <= pScanner->bufend)
            && (dm_smlLibStrncmp((char*) pScanner->pos, "?>", 2) != 0)) {
        if ((rc = dm_xmlAttribute(pScanner, &name, &value)) != SML_ERR_OK)
            return rc;
        dm_smlLibFree(name);
        dm_smlLibFree(value);
        dm_skipS(pScanner);
    }

    if (pScanner->pos + 2 > pScanner->bufend)
        return SML_ERR_XLT_END_OF_BUFFER;

    dm_readBytes(pScanner, 2);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlAttribute
 *
 * Handle Attributes //function can be used if attributes get necessary
 */
static short dm_xmlAttribute(xmlScannerPrivPtr_t pScanner, char**name,
        char**value) {
    short rc;

    dm_skipS(pScanner);

    if ((rc = dm_xmlName(pScanner, name)) != SML_ERR_OK)
        return rc;

    dm_skipS(pScanner);

    /* no attributes found, because this tag has none -> bail out */
    if (*pScanner->pos == '>') {
        return SML_ERR_XLT_MISSING_CONT;
    }
    if (dm_smlLibStrncmp((char*) pScanner->pos, "/>", 2) == 0) {
        return SML_ERR_XLT_MISSING_CONT;
    }

    if (*pScanner->pos != '=') {
        dm_smlLibFree((*name));
        *name = NULL;
        *value = NULL;
        return SML_ERR_XLT_INVAL_XML_DOC;
    }
    dm_readBytes(pScanner, 1);

    dm_skipS(pScanner);

    if ((rc = dm_xmlStringConst(pScanner, value)) != SML_ERR_OK) {
        dm_smlLibFree((*name));
        *name = NULL;
        *value = NULL;
        return rc;
    }

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlStringConst
 *
 * Handle Pcdata String Constants
 */
static short dm_xmlStringConst(xmlScannerPrivPtr_t pScanner, char* *value) {
    char* end;
    int len;
    char del;

    if ((*pScanner->pos != '"') && (*pScanner->pos != '\'')) {
        *value = NULL;
        return SML_ERR_XLT_INVAL_XML_DOC;
    }
    del = *pScanner->pos;
    dm_readBytes(pScanner, 1);

    if ((end = dm_smlLibStrchr((char*) pScanner->pos, del)) == NULL) {
        *value = NULL;

        return SML_ERR_XLT_END_OF_BUFFER;
    }
    len = end - (char*) pScanner->pos;
    if ((*value = (char*) dm_smlLibMalloc(__FILE__, __LINE__, len + 1)) == NULL) {

        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    dm_smlLibMemset(*value, 0, len + 1);
    dm_smlLibStrncpy(*value, (char*) pScanner->pos, len);
    dm_readBytes(pScanner, len + 1);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlSkipPCDATA
 *
 * Read over a Pcdata content
 */
static short dm_xmlSkipPCDATA(const char * file, int line,
        xmlScannerPrivPtr_t pScanner) {
    SmlPcdataPtr_t pPCData;
    unsigned char* begin;
    int len;
    short rc;
    char* _tagString = NULL;
    char* _tagString2 = NULL;

    /* Check wether this PCData might contain a subdtd.
     ** We assume a Sub DTD starts with '<' as first char.
     ** If this char is present start further processing else
     ** take it as pure String data. If the scanning returns an
     ** error we reject the file, as '<' is not a valid char inside
     ** PCData elements.
     */
    if (dm_smlLibStrncmp((char*) pScanner->pos, "<", 1) == 0) {
        rc = dm_xmlTag(pScanner, 0);
        return rc;
    }

    _tagString = dm_smlLibMalloc(__FILE__, __LINE__, XML_MAX_TAGLEN);
    if (_tagString == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    if ((rc = dm_getTagString(pScanner->curtok->tagid, _tagString,
            pScanner->curtok->ext)) != SML_ERR_OK) {
        dm_smlLibFree(_tagString);
        return rc;
    }

    _tagString2 = dm_smlLibMalloc(__FILE__, __LINE__,
            dm_smlLibStrlen(_tagString) + 4 + (pScanner->nsprelen + 1));

    // build a end tag String to compate (e.g. </Meta>)
    // beware of possible namespace prefixes
    if (_tagString2 == NULL) {
        dm_smlLibFree(_tagString);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    _tagString2 = dm_smlLibStrcpy(_tagString2, "</");
    if (pScanner->nsprelen > 0) {
        _tagString2 = dm_smlLibStrcat(_tagString2, pScanner->nsprefix);
        _tagString2 = dm_smlLibStrcat(_tagString2, ":");
    }
    _tagString2 = dm_smlLibStrcat(_tagString2, _tagString);
    _tagString2 = dm_smlLibStrcat(_tagString2, ">");
    dm_smlLibFree(_tagString);

    pPCData = (SmlPcdataPtr_t) dm_smlLibMalloc(file, line, sizeof(SmlPcdata_t));

    if (pPCData == NULL) {
        dm_smlLibFree(_tagString2);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    pPCData->contentType = SML_PCDATA_UNDEFINED;
    pPCData->extension = SML_EXT_UNDEFINED;
    pPCData->length = 0;
    pPCData->content = NULL;

    begin = pScanner->pos;

    // read Pcdata content until end tag appears
    while (dm_smlLibStrncmp((char*) pScanner->pos, _tagString2,
            dm_smlLibStrlen(_tagString2)) != 0) {
        if (pScanner->pos >= pScanner->bufend) {
            dm_smlLibFree(_tagString2);
            return SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;

    }

    dm_smlLibFree(_tagString2);

    len = pScanner->pos - begin;
    pPCData->content = dm_smlLibMalloc(__FILE__, __LINE__, len + 1);
    if (pPCData->content == NULL) {
        dm_smlLibFree(pPCData);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    dm_smlLibMemset(pPCData->content, 0, len + 1);
    dm_smlLibMemcpy(pPCData->content, begin, len);
    pPCData->contentType = SML_PCDATA_STRING;
    pPCData->length = len;

    pScanner->curtok->type = TOK_CONT;
    //  if (pScanner->curtok->pcdata)
    //  {
    //      dm_smlLibFree(pScanner->curtok->pcdata->content);
    //      dm_smlLibFree(pScanner->curtok->pcdata);
    //     pScanner->curtok->pcdata = NULL;
    //  }
    pScanner->curtok->pcdata = pPCData;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlSkipComment
 *
 * Skip comments
 */
static short dm_xmlSkipComment(xmlScannerPrivPtr_t pScanner) {
    dm_readBytes(pScanner, 4);

    while ((pScanner->pos + 3 <= pScanner->bufend)
            && (dm_smlLibStrncmp((char*) pScanner->pos, "-->", 3) != 0))
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;

    if (pScanner->pos + 3 > pScanner->bufend)
        return SML_ERR_XLT_END_OF_BUFFER;

    if (!dm_readBytes(pScanner, 3))
        return SML_ERR_XLT_END_OF_BUFFER;

    dm_skipS(pScanner);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlSkipAttributes
 *
 * Skip attributes -> they are not supported in SyncML
 */
static short dm_xmlSkipAttributes(xmlScannerPrivPtr_t pScanner) {

    while ((pScanner->pos + 1 <= pScanner->bufend)
            && (dm_smlLibStrncmp((char*) pScanner->pos, ">", 1))
            && (dm_smlLibStrncmp((char*) pScanner->pos, "/>", 2)))
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;

    if (pScanner->pos + 1 > pScanner->bufend)
        return SML_ERR_XLT_END_OF_BUFFER;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xmlSkipPI
 *
 * Skip PI elements
 */
static short dm_xmlSkipPI(xmlScannerPrivPtr_t pScanner) {
    if (pScanner) { /* Get rid of warning, this should not be called anyway */
    }

    return SML_ERR_UNSPECIFIC;
}

/**
 * FUNCTION: dm_xmlCDATA
 *
 * Handle a CDATA content
 */
static short dm_xmlCDATA(xmlScannerPrivPtr_t pScanner) {
    SmlPcdataPtr_t pPCData;
    unsigned char* begin;
    int len;

    dm_readBytes(pScanner, 9);

    pPCData = (SmlPcdataPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlPcdata_t));
    if (pPCData == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    pPCData->contentType = SML_PCDATA_UNDEFINED;
    pPCData->length = 0;
    pPCData->content = NULL;

    begin = pScanner->pos;
    while (!((pScanner->pos[0] == ']') && (pScanner->pos[1] == ']')
            && (pScanner->pos[2] == '>')))
        if (!dm_readBytes(pScanner, 1))
            return SML_ERR_XLT_END_OF_BUFFER;

    len = pScanner->pos - begin;
    pPCData->content = dm_smlLibMalloc(__FILE__, __LINE__, len + 1);
    if (pPCData->content == NULL) {
        dm_smlLibFree(pPCData);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    dm_smlLibMemset(pPCData->content, 0, len + 1);
    dm_smlLibMemcpy(pPCData->content, begin, len);
    pPCData->contentType = SML_PCDATA_CDATA;
    pPCData->length = len;

    pScanner->curtok->type = TOK_CONT;
//    if (pScanner->curtok->pcdata)
//    {
    //      dm_smlLibFree(pScanner->curtok->pcdata->content);
    //      dm_smlLibFree(pScanner->curtok->pcdata);
    //      pScanner->curtok->pcdata = NULL;
    //   }
    pScanner->curtok->pcdata = pPCData;

    dm_readBytes(pScanner, 3);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_isPcdata
 *
 * Check if the current tag id represents a Pcdata element
 */
static unsigned char dm_isPcdata(XltTagID_t tagid) {
    switch (tagid) {
    case TN_CMD:
    case TN_CMDID:
    case TN_CMDREF:
    case TN_LANG:
    case TN_LOCNAME:
    case TN_LOCURI:
    case TN_MSGID:
    case TN_MSGREF:
    case TN_RESPURI:
    case TN_SESSIONID:
    case TN_SOURCEREF:
    case TN_TARGETREF:
    case TN_VERSION:
    case TN_PROTO:
    case TN_DATA:
    case TN_META:
    case TN_NUMBEROFCHANGES:
#ifdef __USE_METINF__
    case TN_METINF_EMI:
    case TN_METINF_FORMAT:
    case TN_METINF_FREEID:
    case TN_METINF_FREEMEM:
    case TN_METINF_LAST:
    case TN_METINF_MARK:
    case TN_METINF_MAXMSGSIZE:
        /* SCTSTK - 18/03/2002 S.H. 2002-04-05 : SyncML 1.1 */
    case TN_METINF_MAXOBJSIZE:
    case TN_METINF_NEXT:
    case TN_METINF_NEXTNONCE:
    case TN_METINF_SIZE:
    case TN_METINF_TYPE:
    case TN_METINF_VERSION:
#endif
#ifdef __USE_DEVINF__
    case TN_DEVINF_MAN:
    case TN_DEVINF_MOD:
    case TN_DEVINF_OEM:
    case TN_DEVINF_FWV:
    case TN_DEVINF_SWV:
    case TN_DEVINF_HWV:
    case TN_DEVINF_DEVID:
    case TN_DEVINF_DEVTYP:
    case TN_DEVINF_MAXGUIDSIZE:
    case TN_DEVINF_SOURCEREF:
    case TN_DEVINF_DISPLAYNAME:
    case TN_DEVINF_CTTYPE:
    case TN_DEVINF_DATATYPE:
    case TN_DEVINF_SIZE:
    case TN_DEVINF_PROPNAME:
    case TN_DEVINF_VALENUM:
    case TN_DEVINF_PARAMNAME:
    case TN_DEVINF_SYNCTYPE:
    case TN_DEVINF_XNAM:
    case TN_DEVINF_XVAL:
    case TN_DEVINF_MAXMEM:
    case TN_DEVINF_MAXID:
    case TN_DEVINF_VERCT:
    case TN_DEVINF_VERDTD:
#endif
        return 1;
    default:
        return 0;
    }
}

