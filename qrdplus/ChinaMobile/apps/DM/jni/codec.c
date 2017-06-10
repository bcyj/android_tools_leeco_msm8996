#ifdef WIN32
#include "std_header.h"
#endif

#include "smlcore.h"
#include "smlerr.h"
#include "libmem.h"
#include "decodexml.h"
#include "decodewbxml.h"

struct ArrayStack_s;
typedef struct ArrayStack_s *ArrayStackPtr_t, ArrayStack_t;
struct ArrayStack_s {
    /* public */
    short (*top)(const XltUtilStackPtr_t, XltUtilStackItem_t *);
    short (*pop)(XltUtilStackPtr_t, XltUtilStackItem_t *);
    short (*push)(XltUtilStackPtr_t, const XltUtilStackItem_t);
    short (*destroy)(XltUtilStackPtr_t);

    /* private */
    long topidx;          // index of the top of the stack
    long size;            // size of the stack (multiple of chunksize)
    long chunksize;       // size of memory chunks allocated at a time
    XltUtilStackItem_t *array;     // the stack itself
};

typedef struct PEBuilder_s {
    XltTagID_t tagid;
    SmlProtoElement_t type;
    short (*build)(XltDecoderPtr_t pDecoder, void* *ppElem);
} PEBuilder_t, *PEBuilderPtr_t;

// %%% luz:2003-07-31: added SyncML namespace tables
static const char * const dm_SyncMLNamespacesPim[SML_NUM_VERS] = { "???",
        "SYNCML:SYNCML1.0", "SYNCML:SYNCML1.1", "SYNCML:SYNCML1.2" };

static const char * const dm_SyncMLDevInfFPIPim[SML_NUM_VERS] = { "???",
        "-//SYNCML//DTD DevInf 1.0//EN", "-//SYNCML//DTD DevInf 1.1//EN",
        "-//SYNCML//DTD DevInf 1.2//EN" };

static const char * const dm_SyncMLFPIPim[SML_NUM_VERS] = { "???",
        "-//SYNCML//DTD SyncML 1.0//EN", "-//SYNCML//DTD SyncML 1.1//EN",
        "-//SYNCML//DTD SyncML 1.2//EN" };

/*feature function*/

/* if the commands are not defined we let the functions point to NULL */
#ifndef RESULT_RECEIVE
#define dm_buildResults NULL
#endif

#ifndef MAP_RECEIVE
#define dm_buildMap NULL
#endif

#ifndef EXEC_RECEIVE
#define dm_buildExec NULL
#endif

#if !defined(ATOM_RECEIVE) && !defined(SEQUENCE_RECEIVE)
#define dm_buildAtomOrSeq NULL
#endif

#ifndef SEARCH_RECEIVE
#define buildSearch NULL
#endif

/**
 * FUNCTION: dm_xltGenerateTag
 *
 * Generates a (WB)XML tag
 *
 * PRE-Condition:   valis parameters
 *
 * POST-Condition:  the buffer contains a new tag
 *
 * IN:              tagId, the tag ID
 *                  TagType, the tag type (begin tag, end tag, ...)
 *                  enc, the encoding constant (SML_WBXML or SML_XML)
 *                  attFlag, indicates if the encoded tag contain Attributes in namespace extensions
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_xltGenerateTag(XltTagID_t tagId, XltTagType_t TagType,
        SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag);
static short dm_xltEncPcdata(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag);

/**
 * FUNCTION: dm_xltEncList
 *
 * Generates a list element which is not directly related to a tag
 *
 * PRE-Condition:   pList holds a valid list structure
 *                  listId contains a valid SyncML list ID
 *
 * POST-Condition:  the (WB)XML buffer in the pBufMgr structure contains the
 *                  encoded (WB)XML list
 *
 * IN:              listId, the ID of the list to generate (e.g. TARGET_LIST, ...)
 *                  pList, reference to the list to process
 *                  enc, the encoding constant (SML_WBXML or SML_XML)
 *                  attFlag, indicates if the encoded tag contain Attributes in namespace extensions
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_xltEncList(XltListType_t listId, XltRO_t reqOptFlag,
        void* pList, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag);

/**
 * FUNCTION: dm_xltEncBlock
 *
 * Generates a (WB)XML Block for a given tag ID and a given content
 *
 * PRE-Condition:   pContent holds a valid content structure
 *                  tagId contains a valid SyncML tag ID
 *
 * POST-Condition:  the (WB)XML buffer in the pBufMgr structure contains the
 *                  encoded (WB)XML block
 *
 * IN:              tagId, the ID for the tag to generate (TN_ADD, ...)
 *                  reqOptFlag, flag if the block is required or optional
 *                  pContent, the content structure of the block
 *                  enc, the encoding constant (SML_WBXML or SML_XML)
 *                  attFlag, indicates if the encoded tag contain Attributes in namespace extensions
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_xltEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag);

/**
 * FUNCTION: dm_getExtName
 *
 * Returns the official name for a given extention/sub-DTD
 * and stored it in 'name'. If not found name isn't modified
 */
// %%% luz:2003-04-24: added syncmlvers parameter
// %%% luz:2003-07-31: changed to vers enum
static short dm_getExtName(SmlPcdataExtension_t ext, char* *name,
        SmlVersion_t vers);

static TagPtr_t dm_getTagTable(SmlPcdataExtension_t ext);

static void dm_freeDtdTable(DtdPtr_t tbl);

static DtdPtr_t dm_getDtdTable(void);
static short dm_getTagByte(XltTagID_t tagID, SmlPcdataExtension_t ext,
        unsigned char *pTagByte);

static PEEncPtr_t dm_getPEEncTable(void);
static short dm_getTNbyPE(SmlProtoElement_t pE, XltTagID_t *tagID);

/**
 * Gets the next token from the scanner.
 * Checks if the current tag is an end tag and if so, whether the last
 * open start tag has the same tag id as the current end tag. An open start
 * tag is one which matching end tag has not been seen yet.
 * If the current tag is a start tag its tag ID will be pushed onto the
 * tag stack.
 * If the current tag is an empty tag or not a tag at all nothing will be
 * done.
 */
static short dm_nextToken(const char * file, int line, XltDecoderPtr_t pDecoder);
/**
 * FUNCTION: dm_wbxmlGetGlobToken
 *
 * Converts a element type into its wbxml token
 *
 * PRE-Condition:   valid element type
 *
 * POST-Condition:  return of wbxml token
 *
 * IN:              elType, element type
 *
 * OUT:             wbxml token
 *
 * RETURN:          wbxml token
 *                  0, if no matching wbxml token
 */
static unsigned char dm_wbxmlGetGlobToken(XltElementType_t elType);

/**
 * FUNCTION: dm_wbxmlOpaqueSize2Buf
 *
 * Converts a Long_t opaque size to a wbxml mb_u_int32 and adds it to the buffer
 *
 * PRE-Condition:   size of the content to be written as opaque datatype
 *
 * POST-Condition:  the size is converted to the mb_u_int32 representation and added
 *                  to the buffer
 *
 * IN:              size, length of the opaque data
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_wbxmlOpaqueSize2Buf(long size, BufferMgmtPtr_t pBufMgr);

#ifdef __USE_EXTENSIONS__
/* Entrypoint for SubDTD's. If we reached this point we already know
 * a) we have data fora sub-DTD to encode and
 * b) we know which sub-DTD should be encoded.
 * So just call the appropriate sub-DTD encoder and thats it.
 */
static short dm_xltBuildExtention(SmlPcdataExtension_t extId,
        XltRO_t reqOptFlag, void* pContent, SmlEncoding_t enc,
        BufferMgmtPtr_t pBufMgr);
#ifdef __SML_WBXML__
/* Sub DTD's need a special treatment when used together with WBXML.
 * We need to eoncode them as a complete WBXML message including headers and stuff
 * and store the result within an SML_PCDATA_OPAQUE datafield.
 * To archieve this we create a new encoder, encode the message and finally
 * copy the result into the allready existing encoder.
 */
static short dm_subdtdEncWBXML(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag);

#endif
#endif

static short dm_top(const XltUtilStackPtr_t, XltUtilStackItem_t *);
static short dm_pop(XltUtilStackPtr_t, XltUtilStackItem_t *);
static short dm_push(XltUtilStackPtr_t, const XltUtilStackItem_t);
static short dm_destroy(XltUtilStackPtr_t);

static short dm_appendItemList(XltDecoderPtr_t pDecoder,
        SmlItemListPtr_t *ppItemList);
static short dm_appendTargetRefList(XltDecoderPtr_t pDecoder,
        SmlTargetRefListPtr_t *ppTargetRefList);
static short dm_appendSourceRefList(XltDecoderPtr_t pDecoder,
        SmlSourceRefListPtr_t *ppSourceRefList);

static PEBuilderPtr_t dm_getPETable(void);

#define IS_START(tok) ((tok)->type == TOK_TAG_START)
#define IS_END(tok) ((tok)->type == TOK_TAG_END)
#define IS_EMPTY(tok) ((tok)->type == TOK_TAG_EMPTY)
#define IS_TAG(tok) (IS_START(tok) || IS_EMPTY(tok) || IS_END(tok))
#define IS_START_OR_EMPTY(tok) (IS_START(tok) || IS_EMPTY(tok))
#define IS_CONTENT(tok) ((tok)->type == TOK_CONT)

/**
 * FUNCTION: smlXltEncInit
 *
 * Initializes an XML buffer; Creates XML code for the SyncHdr
 * and appends it to the buffer.
 * Returns 0 if operation was successful.
 *
 * PRE-Condition:   no memory should be allocated for ppEncoder (should be NULL)
 *                  pHeader has to contain a valid SyncHdr structure
 *                  pBufEnd must point to the end of the (WB)XML buffer
 *                  ppBufPos has to be initialized to the start point of the
 *                  (WB)XML buffer.
 *
 *
 * POST-Condition:  After the function call ppBufPos points to the
 *                  first free byte in the buffer behind the (WB)XML document
 *
 * IN:              enc, the encoding constant (SML_WBXML or SML_XML)
 *                  pHeader, the SyncML header structure
 *                  pBufEnd, pointer to the end of the buffer to write on
 *
 * IN/OUT:          ppBufPos, current position of the bufferpointer
 *                  ppEncoder, the encoder object
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 *                  Possible Error Codes:
 *                  SML_ERR_XLT_MISSING_CONT
 *                  SML_ERR_XLT_BUF_ERR
 *                  SML_ERR_XLT_INVAL_ELEM_TYPE
 *                  SML_ERR_XLT_INVAL_LIST_TYPE
 *                  SML_ERR_XLT_INVAL_TAG_TYPE
 *                  SML_ERR_XLT_ENC_UNK
 *                  SML_ERR_XLT_INVAL_PROTO_ELEM
 */
short dm_xltEncInit(SmlEncoding_t enc, const SmlSyncHdrPtr_t pHeader,
        const unsigned char* pBufEnd, unsigned char* *ppBufPos,
        XltEncoderPtr_t *ppEncoder, SmlVersion_t vers) {
    // Return variable
    short _err;

    XltEncoderPtr_t _pEncoder;

    //Structure containing buffer pointers, length and written bytes
    BufferMgmtPtr_t _pBufMgr;

#ifdef __SML_WBXML__
    unsigned char _stablen = 0x1D; //XLT_STABLEN;
    unsigned char _wbxmlver = XLT_WBXMLVER;
    unsigned char _charset = XLT_CHARSET;
    unsigned char _pubident1 = XLT_PUBIDENT1;
    unsigned char _pubident2 = XLT_PUBIDENT2;
    // %%% luz:2003-07-31: now uses FPI according to syncml version
    const char *_syncmldtd = dm_SyncMLFPIPim[vers];
#endif

#ifdef __SML_XML__
    unsigned char* _tmpStr;
    unsigned char* _xmlver = (unsigned char*) XML_VERSION;
    unsigned char* _xmlenc = (unsigned char*) XML_ENCODING;
    unsigned char _begpar = XML_BEGPAR;
    unsigned char _endpar = XML_ENDPAR;
#endif

    //MemByte_t _tmp = 0x00;CURRENTLY NOT USED

    if ((_pEncoder = (XltEncoderPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(XltEncoder_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    if ((_pBufMgr = (BufferMgmtPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(BufferMgmt_t))) == NULL) {
        dm_smlLibFree(_pEncoder);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    //set the encoding
    _pEncoder->enc = enc;

    // %%% luz:2003-07-31: added version
    _pEncoder->vers = vers;

    _pEncoder->cur_ext = (SmlPcdataExtension_t) SML_EXT_UNDEFINED;
    _pEncoder->last_ext = (SmlPcdataExtension_t) SML_EXT_UNDEFINED;
    _pEncoder->end_tag_size = 0;
    _pEncoder->space_evaluation = NULL;

    _pBufMgr->smlXltBufferP = *ppBufPos;
    _pBufMgr->smlXltBufferLen = pBufEnd - *ppBufPos;
    _pBufMgr->smlXltStoreBufP = _pBufMgr->smlXltBufferP;
    _pBufMgr->smlXltWrittenBytes = 0;
    _pBufMgr->smlCurExt = _pEncoder->cur_ext;
    _pBufMgr->smlLastExt = _pEncoder->last_ext;
    _pBufMgr->smlActiveExt = SML_EXT_UNDEFINED;
    _pBufMgr->switchExtTag = TN_UNDEF;
    _pBufMgr->spaceEvaluation = 0;
    _pBufMgr->vers = vers;
    _pBufMgr->endTagSize = 0;

    switch (enc) {

#ifdef __SML_WBXML__
    case SML_WBXML: {

        // Set the WBXML Header Values
        // WBXML Version
        if ((_err = dm_wbxmlWriteTypeToBuffer((unsigned char*) (&_wbxmlver),
                TAG, 1, _pBufMgr)) != SML_ERR_OK)
            break;
        // Public Idetifier - default unknown
        if ((_err = dm_wbxmlWriteTypeToBuffer((unsigned char*) (&_pubident1),
                TAG, 1, _pBufMgr)) != SML_ERR_OK)
            break;
        if ((_err = dm_wbxmlWriteTypeToBuffer((unsigned char*) (&_pubident2),
                TAG, 1, _pBufMgr)) != SML_ERR_OK)
            break;
        // Character set - not yet implemented
        if ((_err = dm_wbxmlWriteTypeToBuffer((unsigned char*) (&_charset), TAG,
                1, _pBufMgr)) != SML_ERR_OK)
            break;
        // Sting table length -  not yet implemented
        if ((_err = dm_wbxmlWriteTypeToBuffer((unsigned char*) (&_stablen), TAG,
                1, _pBufMgr)) != SML_ERR_OK)
            break;
        // FPI - %%% luz:2003-07-31: not constant any more, varies according to SyncML version
        if ((_err = dm_xltAddToBuffer((unsigned char*) (_syncmldtd),
                dm_smlLibStrlen((char*) _syncmldtd), _pBufMgr)) != SML_ERR_OK)
            break;

        break;
    }
#endif

#ifdef __SML_XML__
    case SML_XML: {

        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_begpar), 1, _pBufMgr))
                != SML_ERR_OK)
            break;
        _tmpStr = (unsigned char*) "?xml version=\"";
        if ((_err = dm_xltAddToBuffer(_tmpStr, dm_smlLibStrlen((char*) _tmpStr),
                _pBufMgr)) != SML_ERR_OK)
            break;
        _tmpStr = _xmlver;
        if ((_err = dm_xltAddToBuffer(_tmpStr, dm_smlLibStrlen((char*) _tmpStr),
                _pBufMgr)) != SML_ERR_OK)
            break;
        _tmpStr = (unsigned char*) "\" encoding=\"";
        if ((_err = dm_xltAddToBuffer(_tmpStr, dm_smlLibStrlen((char*) _tmpStr),
                _pBufMgr)) != SML_ERR_OK)
            break;
        _tmpStr = _xmlenc;
        if ((_err = dm_xltAddToBuffer(_tmpStr, dm_smlLibStrlen((char*) _tmpStr),
                _pBufMgr)) != SML_ERR_OK)
            break;
        _tmpStr = (unsigned char*) "\"?";
        if ((_err = dm_xltAddToBuffer(_tmpStr, dm_smlLibStrlen((char*) _tmpStr),
                _pBufMgr)) != SML_ERR_OK)
            break;
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_endpar), 1, _pBufMgr))
                != SML_ERR_OK)
            break;

        break;
    }
#endif

    default: {
        _err = SML_ERR_XLT_ENC_UNK;
    }
    }
    if (_err != SML_ERR_OK) {
        dm_smlLibFree(_pBufMgr);
        dm_smlLibFree(_pEncoder);
        return _err;
    }

    // SyncML Tag
    if ((_err = dm_xltGenerateTag(TN_SYNCML, TT_BEG, enc, _pBufMgr,
            SML_EXT_UNDEFINED)) != SML_ERR_OK) {
        dm_smlLibFree(_pBufMgr);
        dm_smlLibFree(_pEncoder);
        return _err;
    }

    // Generate SmlSyncHdr

    if ((_err = dm_xltEncBlock(TN_SYNCHDR, REQUIRED, pHeader, enc, _pBufMgr,
            SML_EXT_UNDEFINED)) != SML_ERR_OK) {
        dm_smlLibFree(_pBufMgr);
        dm_smlLibFree(_pEncoder);
        return _err;
    }

    // SyncBody Tag
    if ((_err = dm_xltGenerateTag(TN_SYNCBODY, TT_BEG, enc, _pBufMgr,
            SML_EXT_UNDEFINED)) != SML_ERR_OK) {
        dm_smlLibFree(_pBufMgr);
        dm_smlLibFree(_pEncoder);
        return _err;
    }

    _pEncoder->cur_ext = _pBufMgr->smlCurExt;
    _pEncoder->last_ext = _pBufMgr->smlLastExt;
    _pEncoder->end_tag_size = _pBufMgr->endTagSize;

    *ppBufPos = _pBufMgr->smlXltBufferP;

    dm_smlLibFree(_pBufMgr);

    _pEncoder->final = 0;

    *ppEncoder = (XltEncoderPtr_t) _pEncoder;

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xltEncList
 *
 * Generates a list element which is not directly related to a tag
 *
 * PRE-Condition:   pList holds a valid list structure
 *                  listId contains a valid SyncML list ID
 *
 * POST-Condition:  the (WB)XML buffer in the pBufMgr structure contains the
 *                  encoded (WB)XML list
 *
 * IN:              listId, the ID of the list to generate (e.g. TARGET_LIST, ...)
 *                  pList, reference to the list to process
 *                  enc, the encoding constant (SML_WBXML or SML_XML)
 *                  attFlag, indicates if the encoded tag contain Attributes in namespace extensions
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_xltEncList(XltListType_t listId, XltRO_t reqOptFlag,
        void* pList, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag) {
    //Return variable
    short _err;

    //check if list is required or not
    if ((reqOptFlag == REQUIRED) && (pList == NULL ))
        return SML_ERR_XLT_MISSING_CONT;
    else if (pList == NULL)
        return SML_ERR_OK;

    //encode the different list types
    switch ((int) listId) {
    case ITEM_LIST: {
        do {
            if ((_err = dm_xltEncBlock(TN_ITEM, _OPTIONAL,
                    ((SmlItemListPtr_t) pList)->item, enc, pBufMgr, attFlag))
                    != SML_ERR_OK)
                return _err;
            pList = ((SmlItemListPtr_t) pList)->next;
        } while ((SmlItemListPtr_t) pList != NULL );

        break;
    }
    case SOURCE_LIST: {
        do {
            if ((_err = dm_xltEncBlock(TN_SOURCE, _OPTIONAL,
                    ((SmlSourceListPtr_t) pList)->source, enc, pBufMgr, attFlag))
                    != SML_ERR_OK)
                return _err;
            pList = ((SmlSourceListPtr_t) pList)->next;
        } while ((SmlSourceListPtr_t) pList != NULL );

        break;
    }
    case TARGETREF_LIST: {
        do {
            if ((_err = dm_xltEncBlock(TN_TARGETREF, _OPTIONAL,
                    ((SmlTargetRefListPtr_t) pList)->targetRef, enc, pBufMgr,
                    attFlag)) != SML_ERR_OK)
                return _err;
            pList = ((SmlTargetRefListPtr_t) pList)->next;
        } while ((SmlTargetRefListPtr_t) pList != NULL );

        break;
    }
    case SOURCEREF_LIST: {
        do {
            if ((_err = dm_xltEncBlock(TN_SOURCEREF, _OPTIONAL,
                    ((SmlSourceRefListPtr_t) pList)->sourceRef, enc, pBufMgr,
                    attFlag)) != SML_ERR_OK)
                return _err;
            pList = ((SmlSourceRefListPtr_t) pList)->next;
        } while ((SmlSourceRefListPtr_t) pList != NULL );

        break;
    }
    case MAPITEM_LIST: {
        do {
            if ((_err = dm_xltEncBlock(TN_MAPITEM, _OPTIONAL,
                    ((SmlMapItemListPtr_t) pList)->mapItem, enc, pBufMgr,
                    attFlag)) != SML_ERR_OK)
                return _err;
            pList = ((SmlMapItemListPtr_t) pList)->next;
        } while ((SmlMapItemListPtr_t) pList != NULL );

        break;
    }
    default:
        return SML_ERR_XLT_INVAL_LIST_TYPE;
    }

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xltEncBlock
 *
 * Generates a (WB)XML Block for a given tag ID and a given content
 *
 * PRE-Condition:   pContent holds a valid content structure
 *                  tagId contains a valid SyncML tag ID
 *
 * POST-Condition:  the (WB)XML buffer in the pBufMgr structure contains the
 *                  encoded (WB)XML block
 *
 * IN:              tagId, the ID for the tag to generate (TN_ADD, ...)
 *                  reqOptFlag, flag if the block is required or optional
 *                  pContent, the content structure of the block
 *                  enc, the encoding constant (SML_WBXML or SML_XML)
 *                  attFlag, indicates if the encoded tag contain Attributes in namespace extensions
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_xltEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag) {

    //Return variable
    short _err;

    //Check if pContent of a required field is missing
    if ((reqOptFlag == REQUIRED) && (pContent == NULL )) {
        switch ((int) tagId) {
        case TN_ATOMIC_END:
        case TN_SYNC_END:
        case TN_SEQUENCE_END:
            break;
        default:
            return SML_ERR_XLT_MISSING_CONT;
        }
    }
    //Check if pContent of a optional field is missing
    else if ((pContent == NULL ) && (tagId != TN_SYNC_END)
            && (tagId != TN_ATOMIC_END) && (tagId != TN_SEQUENCE_END))
        return SML_ERR_OK;
    // syncml_message("MMIDM dm_xltEncBlock tag id=%d ",tagId);

    //Generate the commands -> see DTD
    switch ((int) tagId) {
    case TN_SYNCHDR:
        // SyncHdr Begin Tag
        if ((_err = dm_xltGenerateTag(TN_SYNCHDR, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // Version
        if ((_err = dm_xltEncBlock(TN_VERSION, REQUIRED,
                ((SmlSyncHdrPtr_t) pContent)->version, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Proto
        if ((_err = dm_xltEncBlock(TN_PROTO, REQUIRED,
                ((SmlSyncHdrPtr_t) pContent)->proto, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // SessionID
        if ((_err = dm_xltEncBlock(TN_SESSIONID, REQUIRED,
                ((SmlSyncHdrPtr_t) pContent)->sessionID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // MsgID
        if ((_err = dm_xltEncBlock(TN_MSGID, REQUIRED,
                ((SmlSyncHdrPtr_t) pContent)->msgID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Target
        if ((_err = dm_xltEncBlock(TN_TARGET, REQUIRED,
                ((SmlSyncHdrPtr_t) pContent)->target, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Source
        if ((_err = dm_xltEncBlock(TN_SOURCE, REQUIRED,
                ((SmlSyncHdrPtr_t) pContent)->source, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // RespURI?
        if ((_err = dm_xltEncBlock(TN_RESPURI, _OPTIONAL,
                ((SmlSyncHdrPtr_t) pContent)->respURI, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlSyncHdrPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlSyncHdrPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlSyncHdrPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // SyncHdr End Tag
        if ((_err = dm_xltGenerateTag(TN_SYNCHDR, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_CRED:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_CRED, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlCredPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Data
        if ((_err = dm_xltEncBlock(TN_DATA, REQUIRED,
                ((SmlCredPtr_t) pContent)->data, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_CRED, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_SOURCE:
    case TN_TARGET:
        // Begin tag
        if ((_err = dm_xltGenerateTag(tagId, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // LocURI
        if ((_err = dm_xltEncBlock(TN_LOCURI, REQUIRED,
                ((SmlSourcePtr_t) pContent)->locURI, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // LocName?
        if ((_err = dm_xltEncBlock(TN_LOCNAME, _OPTIONAL,
                ((SmlSourcePtr_t) pContent)->locName, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(tagId, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_ITEM:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_ITEM, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // Target?
        if ((_err = dm_xltEncBlock(TN_TARGET, _OPTIONAL,
                ((SmlItemPtr_t) pContent)->target, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Source?
        if ((_err = dm_xltEncBlock(TN_SOURCE, _OPTIONAL,
                ((SmlItemPtr_t) pContent)->source, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlItemPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Data?
        if ((_err = dm_xltEncBlock(TN_DATA, _OPTIONAL,
                ((SmlItemPtr_t) pContent)->data, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // MoreData?
        if ((_err = dm_xltEncBlock(TN_MOREDATA, _OPTIONAL,
                &(((SmlItemPtr_t) pContent)->flags), enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_ITEM, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
#if (defined ADD_SEND || defined COPY_SEND)
    case TN_ADD:
    case TN_COPY:
        // Begin tag
        if ((_err = dm_xltGenerateTag(tagId, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlGenericCmdPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlGenericCmdPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlGenericCmdPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlGenericCmdPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item+
        if ((_err = dm_xltEncList(ITEM_LIST, REQUIRED,
                ((SmlGenericCmdPtr_t) pContent)->itemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(tagId, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
#endif
    case TN_ALERT:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_ALERT, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlAlertPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlAlertPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlAlertPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Data?
        if ((_err = dm_xltEncBlock(TN_DATA, _OPTIONAL,
                ((SmlAlertPtr_t) pContent)->data, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item*
        if ((_err = dm_xltEncList(ITEM_LIST, _OPTIONAL,
                ((SmlAlertPtr_t) pContent)->itemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_ALERT, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
#if (defined ATOMIC_SEND || defined SEQUENCE_SEND)
    case TN_ATOMIC:
    case TN_SEQUENCE:
        // Begin tag
        if ((_err = dm_xltGenerateTag(tagId, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlAtomicPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlAtomicPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlAtomicPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        //End tag in TN_ATOMIC_END

        break;
    case TN_ATOMIC_END:
        // End tag
        if ((_err = dm_xltGenerateTag(TN_ATOMIC, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        break;
    case TN_SEQUENCE_END:
        // End tag
        if ((_err = dm_xltGenerateTag(TN_SEQUENCE, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        break;
#endif
    case TN_DELETE:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_DELETE, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlDeletePtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlDeletePtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Archive?
        if ((_err = dm_xltEncBlock(TN_ARCHIVE, _OPTIONAL,
                &(((SmlDeletePtr_t) pContent)->flags), enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // SftDel?
        if ((_err = dm_xltEncBlock(TN_SFTDEL, _OPTIONAL,
                &(((SmlDeletePtr_t) pContent)->flags), enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlDeletePtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlDeletePtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item+
        if ((_err = dm_xltEncList(ITEM_LIST, REQUIRED,
                ((SmlDeletePtr_t) pContent)->itemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_DELETE, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
#ifdef EXEC_SEND
    case TN_EXEC:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_EXEC, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlExecPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlExecPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlExecPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlExecPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item
        if ((_err = dm_xltEncBlock(TN_ITEM, REQUIRED,
                ((SmlExecPtr_t) pContent)->item, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_EXEC, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
#endif
    case TN_GET:
    case TN_PUT:
        // Begin tag
        if ((_err = dm_xltGenerateTag(tagId, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlGetPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlGetPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Lang?
        if ((_err = dm_xltEncBlock(TN_LANG, _OPTIONAL,
                ((SmlGetPtr_t) pContent)->lang, enc, pBufMgr, SML_EXT_UNDEFINED))
                != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlGetPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED))
                != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlGetPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED))
                != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item+
        if ((_err = dm_xltEncList(ITEM_LIST, REQUIRED,
                ((SmlGetPtr_t) pContent)->itemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(tagId, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_MAP:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_MAP, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlMapPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Target
        if ((_err = dm_xltEncBlock(TN_TARGET, REQUIRED,
                ((SmlMapPtr_t) pContent)->target, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Source
        if ((_err = dm_xltEncBlock(TN_SOURCE, REQUIRED,
                ((SmlMapPtr_t) pContent)->source, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlMapPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED))
                != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlMapPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED))
                != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Mapitemlist
        if ((_err = dm_xltEncList(MAPITEM_LIST, REQUIRED,
                ((SmlMapPtr_t) pContent)->mapItemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_MAP, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_MAPITEM:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_MAPITEM, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // Target
        if ((_err = dm_xltEncBlock(TN_TARGET, REQUIRED,
                ((SmlMapItemPtr_t) pContent)->target, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Source
        if ((_err = dm_xltEncBlock(TN_SOURCE, REQUIRED,
                ((SmlMapItemPtr_t) pContent)->source, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_MAPITEM, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_RESULTS:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_RESULTS, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlResultsPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // MsgRef?
        if ((_err = dm_xltEncBlock(TN_MSGREF, _OPTIONAL,
                ((SmlResultsPtr_t) pContent)->msgRef, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // CmdRef
        if ((_err = dm_xltEncBlock(TN_CMDREF, REQUIRED,
                ((SmlResultsPtr_t) pContent)->cmdRef, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlResultsPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // TargetRef?
        if ((_err = dm_xltEncBlock(TN_TARGETREF, _OPTIONAL,
                ((SmlResultsPtr_t) pContent)->targetRef, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // SourceRef?
        if ((_err = dm_xltEncBlock(TN_SOURCEREF, _OPTIONAL,
                ((SmlResultsPtr_t) pContent)->sourceRef, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item+
        if ((_err = dm_xltEncList(ITEM_LIST, REQUIRED,
                ((SmlResultsPtr_t) pContent)->itemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_RESULTS, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_CHAL:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_CHAL, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // Meta
        if ((_err = dm_xltEncBlock(TN_META, REQUIRED,
                ((SmlChalPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_CHAL, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
#ifdef SEARCH_SEND
    case TN_SEARCH:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_SEARCH, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlSearchPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlSearchPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResults?
        if ((_err = dm_xltEncBlock(TN_NORESULTS, _OPTIONAL,
                &((SmlSearchPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlSearchPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Target?
        if ((_err = dm_xltEncBlock(TN_TARGET, _OPTIONAL,
                ((SmlSearchPtr_t) pContent)->target, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Source List
        if ((_err = dm_xltEncList(SOURCE_LIST, REQUIRED,
                ((SmlSearchPtr_t) pContent)->sourceList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Lang?
        if ((_err = dm_xltEncBlock(TN_LANG, _OPTIONAL,
                ((SmlSearchPtr_t) pContent)->lang, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta
        if ((_err = dm_xltEncBlock(TN_META, REQUIRED,
                ((SmlSearchPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Dsta
        if ((_err = dm_xltEncBlock(TN_DATA, REQUIRED,
                ((SmlSearchPtr_t) pContent)->data, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_SEARCH, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
#endif
    case TN_STATUS:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_STATUS, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlStatusPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // MsgRef?
        if ((_err = dm_xltEncBlock(TN_MSGREF, REQUIRED,
                ((SmlStatusPtr_t) pContent)->msgRef, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // CmdRef
        if ((_err = dm_xltEncBlock(TN_CMDREF, REQUIRED,
                ((SmlStatusPtr_t) pContent)->cmdRef, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cmd
        if ((_err = dm_xltEncBlock(TN_CMD, REQUIRED,
                ((SmlStatusPtr_t) pContent)->cmd, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // TargetRefList?
        if ((_err = dm_xltEncList(TARGETREF_LIST, _OPTIONAL,
                ((SmlStatusPtr_t) pContent)->targetRefList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // SourceRefList?
        if ((_err = dm_xltEncList(SOURCEREF_LIST, _OPTIONAL,
                ((SmlStatusPtr_t) pContent)->sourceRefList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlStatusPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Chal?
        if ((_err = dm_xltEncBlock(TN_CHAL, _OPTIONAL,
                ((SmlStatusPtr_t) pContent)->chal, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Data
        if ((_err = dm_xltEncBlock(TN_DATA, REQUIRED,
                ((SmlStatusPtr_t) pContent)->data, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item*
        if ((_err = dm_xltEncList(ITEM_LIST, _OPTIONAL,
                ((SmlStatusPtr_t) pContent)->itemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(TN_STATUS, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_SYNC:
        // Begin tag
        if ((_err = dm_xltGenerateTag(TN_SYNC, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlSyncPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlSyncPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlSyncPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Target?
        if ((_err = dm_xltEncBlock(TN_TARGET, _OPTIONAL,
                ((SmlSyncPtr_t) pContent)->target, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;/*lint !e613*/
        // Source?
        if ((_err = dm_xltEncBlock(TN_SOURCE, _OPTIONAL,
                ((SmlSyncPtr_t) pContent)->source, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlSyncPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NumberOfChanges?
        if ((_err = dm_xltEncBlock(TN_NUMBEROFCHANGES, _OPTIONAL,
                ((SmlSyncPtr_t) pContent)->noc, enc, pBufMgr, SML_EXT_UNDEFINED))
                != SML_ERR_OK)
            return _err;/*lint !e613*/
        // End tag in TN_SYNC_END

        break;
    case TN_SYNC_END:
        //End tag
        if ((_err = dm_xltGenerateTag(TN_SYNC, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_REPLACE:
        // Begin tag
        if ((_err = dm_xltGenerateTag(tagId, TT_BEG, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;
        // CmdID
        if ((_err = dm_xltEncBlock(TN_CMDID, REQUIRED,
                ((SmlGenericCmdPtr_t) pContent)->cmdID, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // NoResp?
        if ((_err = dm_xltEncBlock(TN_NORESP, _OPTIONAL,
                &((SmlGenericCmdPtr_t) pContent)->flags, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Cred?
        if ((_err = dm_xltEncBlock(TN_CRED, _OPTIONAL,
                ((SmlGenericCmdPtr_t) pContent)->cred, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Meta?
        if ((_err = dm_xltEncBlock(TN_META, _OPTIONAL,
                ((SmlGenericCmdPtr_t) pContent)->meta, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // Item+
        if ((_err = dm_xltEncList(ITEM_LIST, REQUIRED,
                ((SmlGenericCmdPtr_t) pContent)->itemList, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err; /*lint !e613*/
        // End tag
        if ((_err = dm_xltGenerateTag(tagId, TT_END, enc, pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK)
            return _err;

        break;
    case TN_ARCHIVE:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((unsigned int*) pContent)) & (SmlArchive_f)) {/*lint !e613*/
            if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                    SML_EXT_UNDEFINED)) != SML_ERR_OK)
                return _err;
        }
        break;
    case TN_SFTDEL:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((unsigned int *) pContent)) & (SmlSftDel_f)) {/*lint !e613*/
            if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                    SML_EXT_UNDEFINED)) != SML_ERR_OK)
                return _err;
            break;
            case TN_MOREDATA:
            //set the flag in the (WB)XML document if the flag is in the pContent
            if ((*((unsigned int *) pContent)) & (SmlMoreData_f)) {/*lint !e613*/
                if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                        SML_EXT_UNDEFINED)) != SML_ERR_OK)
                    return _err;
            }
            break;
            case TN_NORESULTS:
            //set the flag in the (WB)XML document if the flag is in the pContent
            if ((*((unsigned int *) pContent)) & (SmlNoResults_f)) {/*lint !e613*/
                if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                        SML_EXT_UNDEFINED)) != SML_ERR_OK)
                    return _err;
            }
            break;
            case TN_NORESP:
            //set the flag in the (WB)XML document if the flag is in the pContent
            if ((*((unsigned int *) pContent)) & (SmlNoResp_f)) {/*lint !e613*/
                if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                        SML_EXT_UNDEFINED)) != SML_ERR_OK)
                    return _err;
            }
            break;
            case TN_FINAL:
            //set the flag in the (WB)XML document if the flag is in the pContent
            if ((*((unsigned int *) pContent)) & (SmlFinal_f)) {/*lint !e613*/
                if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                        SML_EXT_UNDEFINED)) != SML_ERR_OK)
                    return _err;
            }
            break;
            default:
            // all leaf nodes (PCDATA#)
            return dm_xltEncPcdata(tagId, reqOptFlag, pContent, enc, pBufMgr,
                    attFlag);/*lint !e613*/
        }
    }
    return SML_ERR_OK;
}

static short dm_xltEncPcdata(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag) {
    //Return variable
    short _err;

    //generate PCDATA begin tag
    if ((_err = dm_xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, attFlag))
            != SML_ERR_OK)
        return _err;

    //write the pContent to the buffer according the encoding type
    //syncml_message("MMIDM dm_xltEncPcdata enc=%d ",enc);
    if (pContent == NULL) {
        return SML_ERR_XLT_INVAL_PCDATA_TYPE;
    }

    switch ((int) enc) {
#ifdef __SML_WBXML__
    case SML_WBXML:
        switch (((SmlPcdataPtr_t) pContent)->contentType) {
        case SML_PCDATA_STRING:
            if ((_err = dm_wbxmlWriteTypeToBuffer(
                    ((SmlPcdataPtr_t) pContent)->content, STR_I,
                    ((SmlPcdataPtr_t) pContent)->length, pBufMgr)) != SML_ERR_OK)
                return _err;
            break;
            // Note: SML_PCDATA_CDATA case added by luz to allow direct translation from XML to WBXML
        case SML_PCDATA_CDATA:
        case SML_PCDATA_OPAQUE:
            if ((_err = dm_wbxmlWriteTypeToBuffer(
                    ((SmlPcdataPtr_t) pContent)->content, _OPAQUE,
                    ((SmlPcdataPtr_t) pContent)->length, pBufMgr)) != SML_ERR_OK)
                return _err;
            break;
#ifdef __USE_EXTENSIONS__
        case SML_PCDATA_EXTENSION:
            if ((_err = dm_xltBuildExtention(
                    ((SmlPcdataPtr_t) pContent)->extension, reqOptFlag,
                    ((SmlPcdataPtr_t) pContent)->content, enc, pBufMgr))
                    != SML_ERR_OK)
                return _err;
            break;
#endif
        default:
            // 2003-11-24: Tomy to deal with pcdata empty extensions (for example <Meta></Meta> which is valid)
            // refer to xltdec.c to see that empty extensions result in SmlPcdataPtr_t with all fields (data) set to 0
            if (((SmlPcdataPtr_t) pContent)->contentType
                    != SML_PCDATA_UNDEFINED||
                    ((SmlPcdataPtr_t)pContent)->extension != SML_EXT_UNDEFINED ||
                    ((SmlPcdataPtr_t)pContent)->length != 0 ||
                    ((SmlPcdataPtr_t)pContent)->content != NULL)return SML_ERR_XLT_INVAL_PCDATA_TYPE;
            //          return SML_ERR_XLT_INVAL_PCDATA_TYPE;
            // end modified by Tomy
        }; // eof switch(contenttype)
        break;
#endif  // eof WBXML
#ifdef __SML_XML__
                    case SML_XML:
                    switch (((SmlPcdataPtr_t)pContent)->contentType) {
                        case SML_PCDATA_CDATA: {
                            unsigned char* _tmpStr;
                            _tmpStr = (unsigned char*) "<![CDATA[";
                            if ((_err = dm_xltAddToBuffer(_tmpStr, dm_smlLibStrlen((char*)_tmpStr), pBufMgr)) != SML_ERR_OK) return _err;
                            if ((_err = dm_xltAddToBuffer(((SmlPcdataPtr_t)pContent)->content, ((SmlPcdataPtr_t)pContent)->length, pBufMgr)) != SML_ERR_OK) return _err;
                            _tmpStr = (unsigned char*) "]]>";
                            if ((_err = dm_xltAddToBuffer(_tmpStr, dm_smlLibStrlen((char*)_tmpStr), pBufMgr)) != SML_ERR_OK) return _err;
                            break;
                        }
                        // Note: SyncFest #5 shows that <![CDATA[ is not correctly parsed by the RTK
                        //       so we don't use it and risk the danger of failing on payload which has
                        //       XML in it.
                        case SML_PCDATA_OPAQUE:
                        case SML_PCDATA_STRING:
                        if ((_err = dm_xltAddToBuffer(((SmlPcdataPtr_t)pContent)->content, ((SmlPcdataPtr_t)pContent)->length, pBufMgr)) != SML_ERR_OK) return _err;
                        break;
#ifdef __USE_EXTENSIONS__
                    case SML_PCDATA_EXTENSION:
                    if ((_err = dm_xltBuildExtention(((SmlPcdataPtr_t)pContent)->extension, reqOptFlag, ((SmlPcdataPtr_t)pContent)->content, enc, pBufMgr)) != SML_ERR_OK) return _err;
                    break;
#endif
                    default:
                    // 2003-11-24: Tomy to deal with pcdata empty extensions (for example <Meta></Meta> which is valid)
                    // refer to xltdec.c to see that empty extensions result in SmlPcdataPtr_t with all fields (data) set to 0
                    if (((SmlPcdataPtr_t)pContent)->contentType != SML_PCDATA_UNDEFINED ||
                            ((SmlPcdataPtr_t)pContent)->extension != SML_EXT_UNDEFINED ||
                            ((SmlPcdataPtr_t)pContent)->length != 0 ||
                            ((SmlPcdataPtr_t)pContent)->content != NULL)
                    return SML_ERR_XLT_INVAL_PCDATA_TYPE;
                    //          return SML_ERR_XLT_INVAL_PCDATA_TYPE;
                    // end modified by Tomy
                }
                break;
#endif // eof XML
                    default:
                    return SML_ERR_XLT_ENC_UNK;
                } // eof switch(enc)

            //generate PCDATA END tag
    if ((_err = dm_xltGenerateTag(tagId, TT_END, enc, pBufMgr, attFlag))
            != SML_ERR_OK)
        return _err;
    return SML_ERR_OK;
}

short dm_xltEncReset(XltEncoderPtr_t pEncoder) {
    if ((pEncoder) && (pEncoder->space_evaluation)) {
        dm_smlLibFree((pEncoder->space_evaluation));
        pEncoder->space_evaluation = NULL;
    }
    dm_smlLibFree(pEncoder);
    return SML_ERR_OK;
}

/**
 * FUNCTION: smlXltEncAppend
 *
 * Generates (WB)XML code and appends it to the XML buffer.
 *
 * PRE-Condition:   pEncoder holds the initialized encoder structure.
 *                  the initialization takes place in the dm_xltEncAppend function
 *                  pContent has to contain a valid content structure structure
 *                  pBufEnd must point to the end of the (WB)XML buffer
 *                  ppBufPos has to be initialized to the start point of the
 *                  (WB)XML buffer.
 *
 *
 * POST-Condition:  After the function call ppBufPos points to the
 *                  first free byte in the buffer behind the (WB)XML document
 *
 * IN:              pEncoder, the encoder object
 *                  pe, the protocol element (PE_ADD, ...)
 *                  pBufEnd, pointer to the end of the buffer to write on
 *                  pContent, the content to append to the SyncML document
 *
 * IN/OUT:          ppBufPos, current position of the bufferpointer
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 *                  Possible Error Codes:
 *                  SML_ERR_XLT_MISSING_CONT
 *                  SML_ERR_XLT_BUF_ERR
 *                  SML_ERR_XLT_INVAL_ELEM_TYPE
 *                  SML_ERR_XLT_INVAL_LIST_TYPE
 *                  SML_ERR_XLT_INVAL_TAG_TYPE
 *                  SML_ERR_XLT_ENC_UNK
 *                  SML_ERR_XLT_INVAL_PROTO_ELEM
 */
short dm_xltEncAppend(const XltEncoderPtr_t pEncoder, SmlProtoElement_t pe,
        const unsigned char* pBufEnd, const void* pContent,
        unsigned char* *ppBufPos) {
    // Return variable
    short _err;

    XltTagID_t tagID = TN_UNDEF;

    // encoding type
    SmlEncoding_t _enc;

    //Structure containing buffer pointers, length and written bytes
    BufferMgmtPtr_t _pBufMgr;

    if ((_pBufMgr = (BufferMgmtPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(BufferMgmt_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(_pBufMgr, 0, sizeof(BufferMgmt_t));

    //get the encoding type
    _enc = pEncoder->enc;

    _pBufMgr->vers = pEncoder->vers; // %%% luz:2003-07-31: pass SyncML version to bufmgr
    _pBufMgr->smlXltBufferP = *ppBufPos;
    _pBufMgr->smlXltBufferLen = pBufEnd - *ppBufPos;
    _pBufMgr->smlXltStoreBufP = _pBufMgr->smlXltBufferP;
    _pBufMgr->smlXltWrittenBytes = 0;
    _pBufMgr->smlActiveExt = SML_EXT_UNDEFINED;
    _pBufMgr->switchExtTag = TN_UNDEF;
    _pBufMgr->spaceEvaluation = (unsigned char) (
            (pEncoder->space_evaluation == NULL ) ? 0 : 1);
    // %%% luz 2002-09-03: evaluation may not mess with encoder state
    if (_pBufMgr->spaceEvaluation) {
        // spaceEval state
        _pBufMgr->smlCurExt = pEncoder->space_evaluation->cur_ext;
        _pBufMgr->smlLastExt = pEncoder->space_evaluation->last_ext;
    } else {
        // normal encoder state
        _pBufMgr->smlCurExt = pEncoder->cur_ext;
        _pBufMgr->smlLastExt = pEncoder->last_ext;
    }

    _pBufMgr->endTagSize = 0;

    _err = dm_getTNbyPE(pe, &tagID);

    _err = dm_xltEncBlock(tagID, REQUIRED, pContent, _enc, _pBufMgr,
            SML_EXT_UNDEFINED);
    if (_err != SML_ERR_OK) {
        dm_smlLibFree(_pBufMgr);
        return _err;
    }

    if (pEncoder->space_evaluation != NULL) {
        // Only calculating size
        pEncoder->space_evaluation->written_bytes +=
                _pBufMgr->smlXltWrittenBytes;
        pEncoder->space_evaluation->end_tag_size += _pBufMgr->endTagSize;
        // save it only into evaluation state
        pEncoder->space_evaluation->cur_ext = _pBufMgr->smlCurExt;
        pEncoder->space_evaluation->last_ext = _pBufMgr->smlLastExt;
    } else {
        // really generating data
        pEncoder->end_tag_size += _pBufMgr->endTagSize;
        // save it into encoder state
        pEncoder->cur_ext = _pBufMgr->smlCurExt;
        pEncoder->last_ext = _pBufMgr->smlLastExt;
    }

    *ppBufPos = _pBufMgr->smlXltBufferP;

    dm_smlLibFree(_pBufMgr);

    return SML_ERR_OK;
}

/**
 * FUNCTION: smlXltEncTerminate
 *
 * Filnalizes the (WB)XML document and returns the size of written bytes to
 * the workspace module
 *
 * PRE-Condition:   pEncoder holds the initialized encoder structure.
 *                  the initialization takes place in the dm_xltEncAppend function
 *                  pBufEnd must point to the end of the (WB)XML buffer
 *                  ppBufPos has to be initialized to the start point of the
 *                  (WB)XML buffer.
 *
 * POST-Condition:  After the function call ppBufPos points to the
 *                  first free byte in the buffer behind the (WB)XML document
 *
 * IN:              pEncoder, the encoder object
 *                  pBufEnd, pointer to the end of the buffer to write on
 *
 * IN/OUT:          ppBufPos, current position of the bufferpointer
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 *                  Possible Error Codes:
 *                  SML_ERR_XLT_BUF_ERR
 *                  SML_ERR_XLT_MISSING_CONT
 *                  SML_ERR_XLT_INVAL_ELEM_TYPE
 *                  SML_ERR_XLT_INVAL_LIST_TYPE
 *                  SML_ERR_XLT_INVAL_TAG_TYPE
 *                  SML_ERR_XLT_ENC_UNK
 *                  SML_ERR_XLT_INVAL_PROTO_ELEM
 */
short dm_xltEncTerminate(const XltEncoderPtr_t pEncoder,
        const unsigned char* pBufEnd, unsigned char* *ppBufPos) {
    // Return variable
    short _err;

    // encoding type
    SmlEncoding_t _enc;

    //Structure containing buffer pointers, length and written bytes
    BufferMgmtPtr_t _pBufMgr;

    //get the encoding type
    _enc = pEncoder->enc;

    //Initialize buffer variables
    if ((_pBufMgr = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(BufferMgmt_t)))
            == NULL) {
        dm_smlLibFree(pEncoder);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    _pBufMgr->vers = pEncoder->vers; // %%% luz:2003-07-31: pass SyncML version to bufmgr
    _pBufMgr->smlXltWrittenBytes = 0;
    _pBufMgr->smlXltBufferP = *ppBufPos;
    _pBufMgr->smlXltStoreBufP = _pBufMgr->smlXltBufferP;
    _pBufMgr->smlXltBufferLen = pBufEnd - *ppBufPos;
    _pBufMgr->smlCurExt = pEncoder->cur_ext;
    _pBufMgr->smlLastExt = pEncoder->last_ext;
    _pBufMgr->smlActiveExt = pEncoder->cur_ext;
    _pBufMgr->switchExtTag = TN_UNDEF;
    _pBufMgr->spaceEvaluation = (unsigned char) (
            (pEncoder->space_evaluation == NULL ) ? 0 : 1);
    _pBufMgr->endTagSize = 0;

    if (pEncoder->final == 1) {
        // Final Flag
        if ((_err = dm_xltGenerateTag(TN_FINAL, TT_ALL, _enc, _pBufMgr,
                SML_EXT_UNDEFINED)) != SML_ERR_OK) {
            dm_smlLibFree(_pBufMgr);
            dm_xltEncReset(pEncoder);
            return _err;
        }
    }

    // SyncBody End Tag
    if ((_err = dm_xltGenerateTag(TN_SYNCBODY, TT_END, _enc, _pBufMgr,
            SML_EXT_UNDEFINED)) != SML_ERR_OK) {
        dm_smlLibFree(_pBufMgr);
        dm_xltEncReset(pEncoder);
        return _err;
    }

    // SyncML End Tag
    if ((_err = dm_xltGenerateTag(TN_SYNCML, TT_END, _enc, _pBufMgr,
            SML_EXT_UNDEFINED)) != SML_ERR_OK) {
        dm_smlLibFree(_pBufMgr);
        dm_xltEncReset(pEncoder);
        return _err;
    }

    pEncoder->cur_ext = _pBufMgr->smlCurExt;
    pEncoder->last_ext = _pBufMgr->smlLastExt;

    *ppBufPos = _pBufMgr->smlXltBufferP;

    dm_smlLibFree(_pBufMgr);

    dm_xltEncReset(pEncoder);

    return SML_ERR_OK;
}

short dm_xltDecInit(const SmlEncoding_t enc,
        unsigned char* pBufEnd, // delete the const symbol for reduce compiling warnings!
        unsigned char **ppBufPos, XltDecoderPtr_t *ppDecoder,
        SmlSyncHdrPtr_t *ppSyncHdr) {
    XltDecoderPtr_t pDecoder;
    short rc;
    syncml_comm_message("MMIDM  *_*dm_xltDecInit 1,enc=%d",enc);

    /* create new decoder object */
    if ((pDecoder = (XltDecoderPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(XltDecoder_t))) == NULL) {
        syncml_comm_message("SML_ERR_NOT_ENOUGH_SPACE 5555");
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    pDecoder->finished = 0;
    pDecoder->final = 0;
    pDecoder->scanner = NULL;
    if ((rc = dm_xltUtilCreateStack(&pDecoder->tagstack, 10)) != SML_ERR_OK) {
        dm_xltDecTerminate(pDecoder);
        return rc;
    }
    syncml_comm_message("MMIDM  *_*dm_xltDecInit 1.5");

#ifdef __SML_WBXML__
    if (enc == SML_WBXML) {
        rc = dm_xltDecWbxmlInit(pBufEnd, ppBufPos, &pDecoder->scanner);
        if (rc == SML_ERR_OK) {
            syncml_comm_message("dm_xltDecWbxmlInit  =SML_ERR_OK");
            pDecoder->charset = pDecoder->scanner->charset;
            pDecoder->charsetStr = NULL;
        }
    } else
#endif

#ifdef __SML_XML__
    if (enc == SML_XML) {

        rc = dm_xltDecXmlInit(pBufEnd, ppBufPos, &pDecoder->scanner);
        if (rc == SML_ERR_OK) {
            syncml_comm_message("dm_xltDecXmlInit  =SML_ERR_OK");
            pDecoder->charset = 0;
            pDecoder->charsetStr = pDecoder->scanner->charsetStr;
        }
    } else
#endif

    {
        rc = SML_ERR_XLT_ENC_UNK;
    }

    if (rc != SML_ERR_OK) {
        dm_xltDecTerminate((XltDecoderPtr_t) pDecoder);
        return rc;
    }
    syncml_comm_message("MMIDM  *_*dm_xltDecInit 2");
    /* try to find SyncHdr element, first comes the SyncML tag... */
    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_xltDecTerminate((XltDecoderPtr_t) pDecoder);
        return rc;
    }
    syncml_comm_message("MMIDM  *_*dm_xltDecInit 2.5");
    if (!IS_START(pDecoder->scanner->curtok)
            || (pDecoder->scanner->curtok->tagid != TN_SYNCML)) {
        dm_smlFreePcdata(__FILE__, __LINE__, pDecoder->scanner->curtok->pcdata);
        dm_xltDecTerminate((XltDecoderPtr_t) pDecoder);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }
    syncml_comm_message("MMIDM  *_*dm_xltDecInit 3");
    /* ... then the SyncHdr */
    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_xltDecTerminate((XltDecoderPtr_t) pDecoder);
        return rc;
    }
    if ((rc = dm_buildSyncHdr(pDecoder, (void*) ppSyncHdr)) != SML_ERR_OK) {
        dm_xltDecTerminate((XltDecoderPtr_t) pDecoder);
        return rc;
    }

    *ppBufPos = pDecoder->scanner->getPos(pDecoder->scanner);

    *ppDecoder = (XltDecoderPtr_t) pDecoder;

    return SML_ERR_OK;
}

/**
 * Description see XLTDec.h header file.
 */
short dm_xltDecNext(XltDecoderPtr_t pDecoder, const unsigned char* pBufEnd,
        unsigned char* *ppBufPos, SmlProtoElement_t *pe, void* *ppContent) {
    XltDecoderPtr_t pDecPriv = (XltDecoderPtr_t) pDecoder;
    XltDecScannerPtr_t pScanner = pDecPriv->scanner;
    XltTagID_t tagid;
    short rc;
    int i;

    pScanner->setBuf(pScanner, *ppBufPos, pBufEnd);

    /* if we are still outside the SyncBody, look for SyncBody start tag */
    if ((rc = pDecPriv->tagstack->top(pDecPriv->tagstack, &tagid)) != SML_ERR_OK)
        return rc;
    if (tagid == TN_SYNCML) {
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecPriv)) != SML_ERR_OK)) {
            return rc;
        }
        if (!((IS_START(pScanner->curtok))
                && (pScanner->curtok->tagid == TN_SYNCBODY))) {
            return SML_ERR_XLT_INVAL_PROTO_ELEM;
        }
    }

    if ((rc = dm_nextToken(__FILE__, __LINE__, pDecPriv)) != SML_ERR_OK)
        return rc;

    /* if we find a SyncML protocol element build the corresponding
     data structure */
    if ((IS_START_OR_EMPTY(pScanner->curtok))
            && (pScanner->curtok->tagid != TN_FINAL)) {

        PEBuilderPtr_t pPEs = dm_getPETable();
        if (pPEs == NULL) {
            dm_smlLibFree(pPEs);
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
        i = 0;
        while (((pPEs + i)->tagid) != TN_UNDEF) {
            if (((pPEs + i)->tagid) == pScanner->curtok->tagid) {
                *pe = ((pPEs + i)->type);
                if ((rc = (pPEs + i)->build(pDecPriv, ppContent)) != SML_ERR_OK) {
                    dm_smlLibFree(pPEs);
                    return rc;
                }
                /* T.K. adjust the SML_PE_ for 'generic' structures */
                if (*pe == SML_PE_GENERIC) {
                    SmlGenericCmdPtr_t g = *ppContent;
                    switch ((int) ((pPEs + i)->tagid)) {
                    case TN_ADD:
                        g->elementType = SML_PE_ADD;
                        break;
                    case TN_COPY:
                        g->elementType = SML_PE_COPY;
                        break;
                    case TN_DELETE:
                        g->elementType = SML_PE_DELETE;
                        break;
                    case TN_REPLACE:
                        g->elementType = SML_PE_REPLACE;
                        break;
                    default:
                        break;
                    }
                }
                break;
            }
            i++;
        }
        if (((pPEs + i)->tagid) == TN_UNDEF) {
            *pe = SML_PE_UNDEF;
            *ppContent = NULL;
            dm_smlLibFree(pPEs);
            return SML_ERR_XLT_INVAL_PROTO_ELEM;
        }
        dm_smlLibFree(pPEs);
    } else {

        /* found end tag */
        switch (pScanner->curtok->tagid) {
        case TN_ATOMIC:
            *pe = SML_PE_ATOMIC_END;
            *ppContent = NULL;
            break;
        case TN_SEQUENCE:
            *pe = SML_PE_SEQUENCE_END;
            *ppContent = NULL;
            break;
        case TN_SYNC:
            *pe = SML_PE_SYNC_END;
            *ppContent = NULL;
            break;
        case TN_FINAL:
            *pe = SML_PE_FINAL;
            *ppContent = NULL;
            pDecPriv->final = 1;
            break;
        case TN_SYNCBODY:
            /* next comes the SyncML end tag, then we're done */
            if ((rc = dm_nextToken(__FILE__, __LINE__, pDecPriv)) != SML_ERR_OK)
                return rc;
            if ((pScanner->curtok->type == TOK_TAG_END)
                    && (pScanner->curtok->tagid == TN_SYNCML)) {
                *pe = SML_PE_UNDEF;
                *ppContent = NULL;
                pDecPriv->finished = 1;
            } else {
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            }
            break;
        default:
            return SML_ERR_XLT_INVAL_PROTO_ELEM;
        }
    }

    *ppBufPos = pScanner->getPos(pScanner);

    return SML_ERR_OK;
}

/**
 * FUNCTION: dm_xltDecTerminate
 *
 * Frees the memory allocated by the decoder.
 *
 * PRE-Condition:
 *                 pDecoder points to a decoder status object initialized
 *                 by dm_xltDecInit
 *
 * POST-Condition:
 *                 all memory allocated by the decoder status object is
 *                 freed
 *
 * IN:             pDecoder, the decoder
 *
 * RETURN:         SML_ERR_OK, if the memory could be freed
 *                 else error code
 */
short dm_xltDecTerminate(XltDecoderPtr_t pDecoder) {
    XltDecoderPtr_t pDecPriv;

    if (pDecoder == NULL)
        return SML_ERR_OK;

    pDecPriv = (XltDecoderPtr_t) pDecoder;
    if (pDecPriv->scanner != NULL)
        pDecPriv->scanner->destroy(pDecPriv->scanner);
    if (pDecPriv->tagstack != NULL)
        pDecPriv->tagstack->destroy(pDecPriv->tagstack);
    dm_smlLibFree(pDecPriv);

    return SML_ERR_OK;
}

short dm_xltDecReset(XltDecoderPtr_t pDecoder) {
    return dm_xltDecTerminate(pDecoder);
}

/**
 * FUNCTION: dm_xltGenerateTag
 *
 * Generates a (WB)XML tag
 *
 * PRE-Condition:   valis parameters
 *
 * POST-Condition:  the buffer contains a new tag
 *
 * IN:              tagId, the tag ID
 *                  TagType, the tag type (begin tag, end tag, ...)
 *                  enc, the encoding constant (SML_WBXML or SML_XML)
 *                  attFlag, indicates if the encoded tag contain Attributes in namespace extensions
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_xltGenerateTag(XltTagID_t tagId, XltTagType_t TagType,
        SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag) {

#ifdef __SML_WBXML__
    unsigned char _switchpage = XLT_SWITCHPAGE;
#endif

    switch ((int) enc) {
#ifdef __SML_WBXML__
    case SML_WBXML:
        /* in WBXML codepage switches are done for starting tags only */
        if (TagType != TT_END) {
            //codepage switching with wbxml instead of namespace
            if (dm_getCodePage(attFlag) != dm_getCodePage(pBufMgr->smlCurExt)) {
                unsigned char _newcp = dm_getCodePage(attFlag);
                short _err;
                if ((_err = dm_wbxmlWriteTypeToBuffer(
                        (unsigned char*) (&_switchpage), TAG, 1, pBufMgr))
                        != SML_ERR_OK)
                    return _err;
                if ((_err = dm_wbxmlWriteTypeToBuffer(
                        (unsigned char*) (&_newcp), TAG, 1, pBufMgr))
                        != SML_ERR_OK)
                    return _err;
            }

            if (attFlag != pBufMgr->smlCurExt) {
                pBufMgr->switchExtTag = tagId;
                pBufMgr->smlLastExt = pBufMgr->smlCurExt;
                pBufMgr->smlCurExt = attFlag;
            }
        } // for TagType
        return dm_wbxmlGenerateTag(tagId, TagType, pBufMgr);
#endif
#ifdef __SML_XML__
    case SML_XML:

        if (attFlag != pBufMgr->smlCurExt) {
            pBufMgr->switchExtTag = tagId;
            pBufMgr->smlLastExt = pBufMgr->smlCurExt;
            pBufMgr->smlCurExt = attFlag;
        }
        return dm_xmlGenerateTag(tagId, TagType, pBufMgr, attFlag);
#endif
    default:
        return SML_ERR_XLT_ENC_UNK;
    }

    //return SML_ERR_XLT_ENC_UNK;NOT NEEDED
}

/**
 * FUNCTION: dm_xltAddToBuffer
 *
 * Add a string to the global buffer
 *
 * PRE-Condition:  pContent contains some content bytes to write to the (WB) XML buffer
 *
 * POST-Condition: content is written to the buffer
 *
 * IN:             pContent, the character pointer referencing the content to
 *                           write to the buffer
 *                 size, the content length
 *
 * IN/OUT:         pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:         shows error codes of function,
 *                 0, if OK
 */
short dm_xltAddToBuffer(const unsigned char* pContent, long size,
        BufferMgmtPtr_t pBufMgr) {
    // if we are doing a space evaluation, do not write the data physically - just remember its length
    if (!pBufMgr->spaceEvaluation) {
        //check if buffersize is to small to write the content
        if ((size + pBufMgr->smlXltWrittenBytes) > pBufMgr->smlXltBufferLen) {
            return SML_ERR_XLT_BUF_ERR;
        }

        if (!(dm_smlLibMemcpy((void*) pBufMgr->smlXltBufferP, (void*) pContent,
                (long) size))) {
            return SML_ERR_XLT_BUF_ERR;
        }
        pBufMgr->smlXltBufferP += size;
    }
    pBufMgr->smlXltWrittenBytes += size;

    return SML_ERR_OK;
}

short dm_xltUtilCreateStack(XltUtilStackPtr_t *ppStack, const long size) {
    ArrayStackPtr_t pStack;

    if (size <= 0)
        return SML_ERR_WRONG_PARAM;
    if ((pStack = (ArrayStackPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(ArrayStack_t))) == NULL) {
        *ppStack = NULL;

        syncml_comm_message("return SML_ERR_NOT_ENOUGH_SPACE1");
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    pStack->top = dm_top;
    pStack->pop = dm_pop;
    pStack->push = dm_push;
    pStack->destroy = dm_destroy;
    pStack->topidx = -1;
    pStack->size = size;
    pStack->chunksize = size;
    pStack->array = NULL;
    if ((pStack->array = (XltUtilStackItem_t*) dm_smlLibMalloc(__FILE__,
            __LINE__, size * sizeof(XltUtilStackItem_t))) == NULL) {/*lint !e737*/
        *ppStack = NULL;
        dm_smlLibFree(pStack);
        syncml_comm_message("return SML_ERR_NOT_ENOUGH_SPACE2");
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    *ppStack = (XltUtilStackPtr_t) pStack;

    return SML_ERR_OK;
}

#ifdef __USE_EXTENSIONS__

/* Entrypoint for SubDTD's. If we reached this point we already know
 * a) we have data fora sub-DTD to encode and
 * b) we know which sub-DTD should be encoded.
 * So just call the appropriate sub-DTD encoder and thats it.
 */
static short dm_xltBuildExtention(SmlPcdataExtension_t extId,
        XltRO_t reqOptFlag, void* pContent, SmlEncoding_t enc,
        BufferMgmtPtr_t pBufMgr) {

    switch (extId) {
#ifdef __USE_METINF__
    case SML_EXT_METINF:
        /* a metaInf DTD always starts with this token */
        return dm_metinfEncBlock(TN_METINF_METINF, reqOptFlag, pContent, enc,
                pBufMgr, SML_EXT_METINF);
        break;/*lint !e527*/
#endif
#ifdef __USE_DEVINF__
    case SML_EXT_DEVINF:
        /* a deviceInf DTD always starts with this token */
        /* we have to choose, wether we have to encode the DevInf as XML or WBXML */
        /* in the latter case, we need a special treatment of this sub-dtd, as we have */
        /* to put it into a SML_PCDATA_OPAQUE field ... */
        if (enc == SML_XML)
            return dm_devinfEncBlock(TN_DEVINF_DEVINF, reqOptFlag, pContent,
                    enc, pBufMgr, SML_EXT_DEVINF);
        else
            return dm_subdtdEncWBXML(TN_DEVINF_DEVINF, reqOptFlag, pContent,
                    SML_WBXML, pBufMgr, SML_EXT_DEVINF);
        break;/*lint !e527*/
#endif
        /* oops - we don not know about that extension -> bail out */
    default:
        return SML_ERR_XLT_INVAL_EXT;
    }
    //return SML_ERR_OK;CAN NOT BE REACHED
}

#ifdef __SML_WBXML__
/* Sub DTD's need a special treatment when used together with WBXML.
 * We need to eoncode them as a complete WBXML message including headers and stuff
 * and store the result within an SML_PCDATA_OPAQUE datafield.
 * To archieve this we create a new encoder, encode the message and finally
 * copy the result into the allready existing encoder.
 */

static short dm_subdtdEncWBXML(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag) {
#ifdef __USE_DEVINF__
    short _err = SML_ERR_OK;
#endif

    short SubBufSize = 12000; // for starters we use 12kB for each sub DTD to encode in WBXML
    BufferMgmtPtr_t pSubBufMgr = NULL;

    // %%% luz 2003-07-31: ensured that we send the right version here!
    const char *FPIstring = dm_SyncMLDevInfFPIPim[pBufMgr->vers];
    short FPIsize = (short) dm_smlLibStrlen(FPIstring);

    // first create a sub buffer
    pSubBufMgr = (BufferMgmtPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(BufferMgmt_t));
    if (pSubBufMgr == NULL) {
        if (enc && pContent && reqOptFlag && tagId) {
        }
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    dm_smlLibMemset(pSubBufMgr, 0, sizeof(BufferMgmt_t));
    pSubBufMgr->smlXltBufferLen = SubBufSize;
    pSubBufMgr->smlXltBufferP = (unsigned char*) dm_smlLibMalloc(__FILE__,
            __LINE__, SubBufSize);
    if (pSubBufMgr->smlXltBufferP == NULL) {
        dm_smlLibFree(pSubBufMgr);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }

    dm_smlLibMemset(pSubBufMgr->smlXltBufferP, 0, SubBufSize);
    pSubBufMgr->smlXltStoreBufP = pSubBufMgr->smlXltBufferP;
    pSubBufMgr->smlXltWrittenBytes = 0;
    pSubBufMgr->smlActiveExt = pBufMgr->smlActiveExt;
    pSubBufMgr->smlCurExt = pBufMgr->smlCurExt;
    pSubBufMgr->smlLastExt = pBufMgr->smlLastExt;
    pSubBufMgr->spaceEvaluation = pBufMgr->spaceEvaluation;

    // in case of space evaluation, just count the number of written bytes
    if (pSubBufMgr->spaceEvaluation == 0) {
        // create the WBXML header
        pSubBufMgr->smlXltBufferP[0] = 0x02; // WBXML Version 1.2
        pSubBufMgr->smlXltBufferP[1] = 0x00; // use Stringtable for ID
        pSubBufMgr->smlXltBufferP[2] = 0x00; // empty/unknown public ID
        pSubBufMgr->smlXltBufferP[3] = 0x6A; // charset encoding UTF-8
        pSubBufMgr->smlXltBufferP[4] = 0x1D; // lenght of stringtable
        pSubBufMgr->smlXltBufferP += 5;
        // Generate FPI
        // %%% luz 2003-07-31: ensured that we send the right version here!
        dm_smlLibMemmove(pSubBufMgr->smlXltBufferP, FPIstring, FPIsize);
        pSubBufMgr->smlXltBufferP += FPIsize;
    }
    pSubBufMgr->smlXltWrittenBytes = 5 + FPIsize;

    // do the encoding
    switch (attFlag) {
#ifdef __USE_DEVINF__
    case SML_EXT_DEVINF:
        if ((_err = dm_devinfEncBlock(TN_DEVINF_DEVINF, reqOptFlag, pContent,
                enc, pSubBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) {
            dm_smlLibFree((pSubBufMgr->smlXltStoreBufP));
            dm_smlLibFree(pSubBufMgr);
            return _err;
        }
        break;
#endif
        /* oops - we don not know about that extension -> bail out */
    default:
        dm_smlLibFree((pSubBufMgr->smlXltStoreBufP));
        dm_smlLibFree(pSubBufMgr);
        return SML_ERR_XLT_INVAL_EXT;
    }

#ifdef __USE_DEVINF__
    // move it to the 'real' encoder buffer
    // now set up the OPAQUE field
    if (pBufMgr->spaceEvaluation == 0) {
        pBufMgr->smlXltBufferP[0] = 0xC3; // OPAQUE data identifier
        pBufMgr->smlXltBufferP += 1;

        dm_wbxmlOpaqueSize2Buf(pSubBufMgr->smlXltWrittenBytes, pBufMgr);

        dm_smlLibMemmove(pBufMgr->smlXltBufferP, pSubBufMgr->smlXltStoreBufP,
                pSubBufMgr->smlXltWrittenBytes);
        pBufMgr->smlXltBufferP += pSubBufMgr->smlXltWrittenBytes;
        pBufMgr->smlXltWrittenBytes += pSubBufMgr->smlXltWrittenBytes;
    } else {
        pBufMgr->smlXltWrittenBytes++;
        dm_wbxmlOpaqueSize2Buf(pSubBufMgr->smlXltWrittenBytes, pBufMgr);
        pBufMgr->smlXltWrittenBytes += pSubBufMgr->smlXltWrittenBytes;
    }

    // clean up the temporary stuff
    dm_smlLibFree((pSubBufMgr->smlXltStoreBufP));
    dm_smlLibFree(pSubBufMgr);

    return _err;
#endif
}

#endif

#ifdef __USE_METINF__

short dm_metinfEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag) {
    //Return variable
    short _err;
    SmlPcdataListPtr_t pList = NULL;
    //Check if pContent of a required field is missing
    if ((reqOptFlag == REQUIRED) && (pContent == NULL ))
        return SML_ERR_XLT_MISSING_CONT;
    //Check if pContent of a optional field is missing -> if yes we are done
    else if (pContent == NULL)
        return SML_ERR_OK;

    //Generate the commands -> see DTD
    switch (tagId) {
    case TN_METINF_ANCHOR:
        if ((_err = dm_xltGenerateTag(TN_METINF_ANCHOR, TT_BEG, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_LAST, _OPTIONAL,
                ((SmlMetInfAnchorPtr_t) pContent)->last, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_NEXT, REQUIRED,
                ((SmlMetInfAnchorPtr_t) pContent)->next, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_xltGenerateTag(TN_METINF_ANCHOR, TT_END, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        break;
    case TN_METINF_MEM:
        if ((_err = dm_xltGenerateTag(TN_METINF_MEM, TT_BEG, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_SHAREDMEM, _OPTIONAL,
                ((SmlMetInfMemPtr_t) pContent)->shared, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_FREEMEM, REQUIRED,
                ((SmlMetInfMemPtr_t) pContent)->free, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_FREEID, REQUIRED,
                ((SmlMetInfMemPtr_t) pContent)->freeid, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_xltGenerateTag(TN_METINF_MEM, TT_END, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        break;
    case TN_METINF_SHAREDMEM:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((unsigned int *) pContent)) & (SmlMetInfSharedMem_f))
            if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                    SML_EXT_METINF)) != SML_ERR_OK)
                return _err;
        break;
    case TN_METINF_METINF:
        //if ((_err = dm_xltGenerateTag(TN_METINF_METINF, TT_BEG, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_FORMAT, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->format, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_TYPE, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->type, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_MARK, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->mark, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_SIZE, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->size, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_ANCHOR, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->anchor, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_VERSION, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->version, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_NEXTNONCE, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->nextnonce, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_metinfEncBlock(TN_METINF_MAXMSGSIZE, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->maxmsgsize, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        // %%% luz 2003-04-24: added maxobjsize generation (was missing = bug in original RTK 4.1)
        if ((_err = dm_metinfEncBlock(TN_METINF_MAXOBJSIZE, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->maxobjsize, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        pList = ((SmlMetInfMetInfPtr_t) pContent)->emi;
        while (pList != NULL ) {
            if ((_err = dm_xltEncBlock(TN_METINF_EMI, _OPTIONAL, pList->data,
                    enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK)
                return _err;
            pList = pList->next;
        }
        ;

        if ((_err = dm_metinfEncBlock(TN_METINF_MEM, _OPTIONAL,
                ((SmlMetInfMetInfPtr_t) pContent)->mem, enc, pBufMgr,
                SML_EXT_METINF)) != SML_ERR_OK)
            return _err;
        //if ((_err = dm_xltGenerateTag(TN_METINF_METINF, TT_END, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        break;
    default: { // all leaf nodes (PCDATA#)
        return dm_xltEncPcdata(tagId, reqOptFlag, pContent, enc, pBufMgr,
                attFlag);
    } /* eof default statement from switch tagid */
    } /* eof switch tagid */
    return SML_ERR_OK;
}

short dm_buildMetInfMetInfCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlMetInfMetInfPtr_t pMeta;
    short rc;
    int foundWrapper = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pMeta = (SmlMetInfMetInfPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlMetInfMetInf_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pMeta, 0, sizeof(SmlMetInfMetInf_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pMeta;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlFreeMetinfMetinf(pMeta);
        //dm_smlLibFree(pMeta);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        case TN_METINF_METINF: /* ignore - it's just the wrapper tag */
            foundWrapper = 1;
            break;
        case TN_METINF_FORMAT:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->format);
            break;
        case TN_METINF_TYPE:
            syncml_codec_message("MMIDM dm_buildMetInfMetInfCmd --TN_METINF_TYPE  file: %s , line: %d", __FILE__, __LINE__);

            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->type);
            break;
        case TN_METINF_MARK:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->mark);
            break;
        case TN_METINF_SIZE:
            syncml_codec_message("MMIDM dm_buildMetInfMetInfCmd --TN_METINF_SIZE  file: %s , line: %d", __FILE__, __LINE__);

            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->size);
            break;
        case TN_METINF_VERSION:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->version);
            break;
        case TN_METINF_NEXTNONCE:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->nextnonce);
            break;
        case TN_METINF_ANCHOR:
            rc = dm_buildMetInfAnchorCmd(pDecoder, (void*) &pMeta->anchor);
            break;
        case TN_METINF_MAXMSGSIZE:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->maxmsgsize);
            break;
            /* SCTSTK - 18/03/2002 S.H. 2002-04-05: SyncML 1.1 */
        case TN_METINF_MAXOBJSIZE:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMeta->maxobjsize);
            break;
        case TN_METINF_MEM:
            rc = dm_buildMetInfMemCmd(pDecoder, (void*) &pMeta->mem);
            break;
        case TN_METINF_EMI:
            rc = dm_buildPCDataList(pDecoder, (void*) &pMeta->emi);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeMetinfMetinf(pMeta);
            //dm_smlLibFree(pMeta);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeMetinfMetinf(pMeta);
            //dm_smlLibFree(pMeta);
            return rc;
        }
    }

    if (foundWrapper) {
        /* Optional Metinf root tag was used in this message.
         * The actual token is the closing root tag.
         * It is required that the scanner points to the first tag _after_
         * <MetInf>...</MetInf>, so we just skip to the next token and continue.
         */
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeMetinfMetinf(pMeta);
            //dm_smlLibFree(pMeta);
            return rc;
        }
    }
    *ppElem = pMeta;

    return SML_ERR_OK;
}

/* decoder callbacks */
short dm_buildMetInfAnchorCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlMetInfAnchorPtr_t pAnchor;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pAnchor = (SmlMetInfAnchorPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlMetInfAnchor_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pAnchor, 0, sizeof(SmlMetInfAnchor_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pAnchor;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlFreeMetinfAnchor(pAnchor);
        //dm_smlLibFree(pAnchor);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        /* PCDATA elements */
        case TN_METINF_LAST:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pAnchor->last);
            break;
        case TN_METINF_NEXT:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pAnchor->next);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeMetinfAnchor(pAnchor);
            //dm_smlLibFree(pAnchor);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeMetinfAnchor(pAnchor);
            //dm_smlLibFree(pAnchor);
            return rc;
        }
    }
    *ppElem = pAnchor;

    return SML_ERR_OK;
}

short dm_buildMetInfMemCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlMetInfMemPtr_t pMem;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pMem = (SmlMetInfMemPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlMetInfMem_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pMem, 0, sizeof(SmlMetInfMem_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pMem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlFreeMetinfMem(pMem);
        //dm_smlLibFree(pMem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        /* PCDATA elements */
        case TN_METINF_SHAREDMEM:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMem->shared);
            break;
        case TN_METINF_FREEMEM:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMem->free);
            break;
        case TN_METINF_FREEID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMem->freeid);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeMetinfMem(pMem);
            //dm_smlLibFree(pMem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeMetinfMem(pMem);
            //dm_smlLibFree(pMem);
            return rc;
        }
    }
    *ppElem = pMem;

    return SML_ERR_OK;
}

#endif

#ifdef __USE_DEVINF__
/* see xltenc.c:XltEncBlock for description of parameters */
short dm_devinfEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag,
        const void* pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr,
        SmlPcdataExtension_t attFlag) {
    //Return variable
    short _err;
    SmlPcdataListPtr_t pList = NULL, p2List = NULL;
    SmlDevInfDatastoreListPtr_t dsList = NULL;
    SmlDevInfCtcapListPtr_t ctList = NULL;
    SmlDevInfExtListPtr_t exList = NULL;
    SmlDevInfXmitListPtr_t xmList = NULL;
    SmlDevInfCTDataPropListPtr_t propList = NULL;
    SmlDevInfCTDataListPtr_t paramList = NULL;

    //Check if pContent of a required field is missing
    if ((reqOptFlag == REQUIRED) && (pContent == NULL ))
        return SML_ERR_XLT_MISSING_CONT;
    //Check if pContent of a optional field is missing -> if yes we are done
    else if (pContent == NULL)
        return SML_ERR_OK;

    //Generate the commands -> see DTD
    syncml_message("MMIDM dm_devinfEncBlock tag id=%d ",tagId);
    switch (tagId) {
    case TN_DEVINF_EXT:
        if ((_err = dm_xltGenerateTag(TN_DEVINF_EXT, TT_BEG, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_XNAM, REQUIRED,
                ((SmlDevInfExtPtr_t) pContent)->xnam, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        pList = ((SmlDevInfExtPtr_t) pContent)->xval;
        while (pList != NULL ) {
            if ((_err = dm_devinfEncBlock(TN_DEVINF_XVAL, _OPTIONAL,
                    pList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
            pList = pList->next;
        }
        ;
        if ((_err = dm_xltGenerateTag(TN_DEVINF_EXT, TT_END, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        break;
    case TN_DEVINF_SYNCCAP:
        if ((_err = dm_xltGenerateTag(TN_DEVINF_SYNCCAP, TT_BEG, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        pList = ((SmlDevInfSyncCapPtr_t) pContent)->synctype;
        while (pList != NULL ) {
            if ((_err = dm_devinfEncBlock(TN_DEVINF_SYNCTYPE, _OPTIONAL,
                    pList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
            pList = pList->next;
        }
        ;
        if ((_err = dm_xltGenerateTag(TN_DEVINF_SYNCCAP, TT_END, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        break;
    case TN_DEVINF_SHAREDMEM:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((unsigned int *) pContent)) & (SmlDevInfSharedMem_f))
            if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                    SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
        break;
        // %%% luz:2003-04-28 added missing 1.1 devinf tags here
    case TN_DEVINF_UTC:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((unsigned int *) pContent)) & (SmlDevInfUTC_f)) {
            if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                    SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
        }
        break;
    case TN_DEVINF_NOFM:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((unsigned int *) pContent)) & (SmlDevInfNOfM_f)) {
            if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                    SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
        }
        break;
    case TN_DEVINF_LARGEOBJECT:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((unsigned int *) pContent)) & (SmlDevInfLargeObject_f)) {
            if ((_err = dm_xltGenerateTag(tagId, TT_ALL, enc, pBufMgr,
                    SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
        }
        break;
        // %%% end luz
    case TN_DEVINF_CTCAP:
        if ((_err = dm_xltGenerateTag(TN_DEVINF_CTCAP, TT_BEG, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        ctList = ((SmlDevInfCtcapListPtr_t) pContent);
        // if (ctList == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA; //del 2009.12.19 for del warning
        while (ctList != NULL ) {
            if (ctList->data == NULL)
                return SML_ERR_XLT_INVAL_INPUT_DATA;
            if ((_err = dm_devinfEncBlock(TN_DEVINF_CTTYPE, _OPTIONAL,
                    ctList->data->cttype, enc, pBufMgr, SML_EXT_DEVINF))
                    != SML_ERR_OK)
                return _err;
            /* now the propList */
            // %%% luz 2002-11-27: made property list optional (e.g. text/message of P800 has none)
            propList = ctList->data->prop;
            // %%% original: if (propList == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA;
            while (propList != NULL ) {
                if (propList->data == NULL)
                    return SML_ERR_XLT_INVAL_INPUT_DATA;
                if (propList->data->prop == NULL)
                    return SML_ERR_XLT_INVAL_INPUT_DATA;
                /* -- Propname */
                if ((_err = dm_devinfEncBlock(TN_DEVINF_PROPNAME, REQUIRED,
                        propList->data->prop->name, enc, pBufMgr,
                        SML_EXT_DEVINF)) != SML_ERR_OK)
                    return _err;
                /* -- (ValEnum+ | (Datatype, Size?))? */
                //if (propList->data->prop->valenum == NULL && propList->data->prop->datatype == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA;
                if (propList->data->prop->valenum != NULL
                        && propList->data->prop->datatype != NULL)
                    return SML_ERR_XLT_INVAL_INPUT_DATA;
                if (propList->data->prop->valenum != NULL) {
                    // ValEnum+
                    pList = propList->data->prop->valenum;
                    while (pList != NULL ) {
                        if ((_err = dm_devinfEncBlock(TN_DEVINF_VALENUM,
                                REQUIRED, pList->data, enc, pBufMgr,
                                SML_EXT_DEVINF)) != SML_ERR_OK)
                            return _err;
                        pList = pList->next;
                    };
                } else if (propList->data->prop->datatype != NULL) {
                    // Datatype, Size?
                    if ((_err = dm_devinfEncBlock(TN_DEVINF_DATATYPE, REQUIRED,
                            propList->data->prop->datatype, enc, pBufMgr,
                            SML_EXT_DEVINF)) != SML_ERR_OK)
                        return _err;
                    if ((_err = dm_devinfEncBlock(TN_DEVINF_SIZE, _OPTIONAL,
                            propList->data->prop->size, enc, pBufMgr,
                            SML_EXT_DEVINF)) != SML_ERR_OK)
                        return _err;
                }
                /* -- DisplayName ? */
                if ((_err = dm_devinfEncBlock(TN_DEVINF_DISPLAYNAME, _OPTIONAL,
                        propList->data->prop->dname, enc, pBufMgr,
                        SML_EXT_DEVINF)) != SML_ERR_OK)
                    return _err;
                /* -- now the paramList */
                paramList = propList->data->param;
                while (paramList != NULL ) {
                    if ((_err = dm_devinfEncBlock(TN_DEVINF_PARAMNAME, REQUIRED,
                            paramList->data->name, enc, pBufMgr, SML_EXT_DEVINF))
                            != SML_ERR_OK)
                        return _err;
                    /* -- (ValEnum+ | (Datatype, Size?))? */
                    //if (paramList->data->valenum == NULL && paramList->data->datatype == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA;
                    if (paramList->data->valenum != NULL
                            && paramList->data->datatype != NULL)
                        return SML_ERR_XLT_INVAL_INPUT_DATA;
                    if (paramList->data->valenum != NULL) {
                        // ValEnum+
                        p2List = paramList->data->valenum;
                        while (p2List != NULL ) {
                            if ((_err = dm_devinfEncBlock(TN_DEVINF_VALENUM,
                                    REQUIRED, p2List->data, enc, pBufMgr,
                                    SML_EXT_DEVINF)) != SML_ERR_OK)
                                return _err;
                            p2List = p2List->next;
                        };
                    } else if (paramList->data->datatype != NULL) {
                        // Datatype, Size?
                        if ((_err = dm_devinfEncBlock(TN_DEVINF_DATATYPE,
                                REQUIRED, paramList->data->datatype, enc,
                                pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
                            return _err;
                        if ((_err = dm_devinfEncBlock(TN_DEVINF_SIZE, _OPTIONAL,
                                paramList->data->size, enc, pBufMgr,
                                SML_EXT_DEVINF)) != SML_ERR_OK)
                            return _err;
                    }
                    /* -- DisplayName ? */
                    if ((_err = dm_devinfEncBlock(TN_DEVINF_DISPLAYNAME,
                            _OPTIONAL, paramList->data->dname, enc, pBufMgr,
                            SML_EXT_DEVINF)) != SML_ERR_OK)
                        return _err;
                    paramList = paramList->next;
                }
                propList = propList->next;
            }
            /* eof propList */
            ctList = ctList->next;
        }
        ;

        if ((_err = dm_xltGenerateTag(TN_DEVINF_CTCAP, TT_END, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        break;
    case TN_DEVINF_DSMEM:
        if ((_err = dm_xltGenerateTag(TN_DEVINF_DSMEM, TT_BEG, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_SHAREDMEM, _OPTIONAL,
                &(((SmlDevInfDSMemPtr_t) pContent)->flags), enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_MAXMEM, _OPTIONAL,
                ((SmlDevInfDSMemPtr_t) pContent)->maxmem, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_MAXID, _OPTIONAL,
                ((SmlDevInfDSMemPtr_t) pContent)->maxid, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_xltGenerateTag(TN_DEVINF_DSMEM, TT_END, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        break;
        // special case, the following 4 have the same structure, only the tag name differs
    case TN_DEVINF_RX:
    case TN_DEVINF_TX:
    case TN_DEVINF_RXPREF:
    case TN_DEVINF_TXPREF:
        if ((_err = dm_xltGenerateTag(tagId, TT_BEG, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_CTTYPE, REQUIRED,
                ((SmlDevInfXmitPtr_t) pContent)->cttype, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_VERCT, REQUIRED,
                ((SmlDevInfXmitPtr_t) pContent)->verct, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_xltGenerateTag(tagId, TT_END, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        break;
    case TN_DEVINF_DATASTORE:
        if ((_err = dm_xltGenerateTag(TN_DEVINF_DATASTORE, TT_BEG, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_SOURCEREF, REQUIRED,
                ((SmlDevInfDatastorePtr_t) pContent)->sourceref, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_DISPLAYNAME, _OPTIONAL,
                ((SmlDevInfDatastorePtr_t) pContent)->displayname, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_MAXGUIDSIZE, _OPTIONAL,
                ((SmlDevInfDatastorePtr_t) pContent)->maxguidsize, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_RXPREF, REQUIRED,
                ((SmlDevInfDatastorePtr_t) pContent)->rxpref, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        xmList = ((SmlDevInfDatastorePtr_t) pContent)->rx;
        while (xmList != NULL ) {
            if ((_err = dm_devinfEncBlock(TN_DEVINF_RX, _OPTIONAL, xmList->data,
                    enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
            xmList = xmList->next;
        }
        ;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_TXPREF, REQUIRED,
                ((SmlDevInfDatastorePtr_t) pContent)->txpref, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        xmList = ((SmlDevInfDatastorePtr_t) pContent)->tx;
        while (xmList != NULL ) {
            if ((_err = dm_devinfEncBlock(TN_DEVINF_TX, _OPTIONAL, xmList->data,
                    enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
            xmList = xmList->next;
        }
        ;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_DSMEM, _OPTIONAL,
                ((SmlDevInfDatastorePtr_t) pContent)->dsmem, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_SYNCCAP, REQUIRED,
                ((SmlDevInfDatastorePtr_t) pContent)->synccap, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_xltGenerateTag(TN_DEVINF_DATASTORE, TT_END, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        break;
    case TN_DEVINF_DEVINF:
        if ((_err = dm_xltGenerateTag(TN_DEVINF_DEVINF, TT_BEG, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_VERDTD, REQUIRED,
                ((SmlDevInfDevInfPtr_t) pContent)->verdtd, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_MAN, _OPTIONAL,
                ((SmlDevInfDevInfPtr_t) pContent)->man, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_MOD, _OPTIONAL,
                ((SmlDevInfDevInfPtr_t) pContent)->mod, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_OEM, _OPTIONAL,
                ((SmlDevInfDevInfPtr_t) pContent)->oem, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_FWV, _OPTIONAL,
                ((SmlDevInfDevInfPtr_t) pContent)->fwv, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_SWV, _OPTIONAL,
                ((SmlDevInfDevInfPtr_t) pContent)->swv, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_HWV, _OPTIONAL,
                ((SmlDevInfDevInfPtr_t) pContent)->hwv, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_DEVID, REQUIRED,
                ((SmlDevInfDevInfPtr_t) pContent)->devid, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_DEVTYP, REQUIRED,
                ((SmlDevInfDevInfPtr_t) pContent)->devtyp, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        // %%% luz:2003-04-28 added missing SyncML 1.1 devinf tags
        if ((_err = dm_devinfEncBlock(TN_DEVINF_UTC, _OPTIONAL,
                &(((SmlDevInfDevInfPtr_t) pContent)->flags), enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_NOFM, _OPTIONAL,
                &(((SmlDevInfDevInfPtr_t) pContent)->flags), enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_LARGEOBJECT, _OPTIONAL,
                &(((SmlDevInfDevInfPtr_t) pContent)->flags), enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        // %%% end luz

        dsList = ((SmlDevInfDevInfPtr_t) pContent)->datastore;
        if (dsList == NULL)
            return SML_ERR_XLT_MISSING_CONT;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_DATASTORE, REQUIRED,
                dsList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        dsList = dsList->next;
        while (dsList != NULL ) {
            if ((_err = dm_devinfEncBlock(TN_DEVINF_DATASTORE, _OPTIONAL,
                    dsList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
            dsList = dsList->next;
        }
        ;
        if ((_err = dm_devinfEncBlock(TN_DEVINF_CTCAP, _OPTIONAL,
                ((SmlDevInfDevInfPtr_t) pContent)->ctcap, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        exList = ((SmlDevInfDevInfPtr_t) pContent)->ext;
        while (exList != NULL ) {
            if ((_err = dm_devinfEncBlock(TN_DEVINF_EXT, _OPTIONAL,
                    exList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK)
                return _err;
            exList = exList->next;
        }
        ;

        if ((_err = dm_xltGenerateTag(TN_DEVINF_DEVINF, TT_END, enc, pBufMgr,
                SML_EXT_DEVINF)) != SML_ERR_OK)
            return _err;
        break;

    default: { // all leaf nodes (PCDATA#)
        return dm_xltEncPcdata(tagId, reqOptFlag, pContent, enc, pBufMgr,
                attFlag);
    } //* eof default statement from switch tagid
    } // eof switch tagid
    return SML_ERR_OK;
}

short dm_buildDevInfDevInfCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDevInfPtr_t pElem = NULL;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pElem);
        return rc;
    }

    switch (pScanner->curtok->tagid) {
    case TN_DEVINF_DEVINF:
        rc = dm_buildDevInfDevInfContent(pDecoder, (void*) &pElem);
        break;
    default:
        rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
    }
    if (rc != SML_ERR_OK) {
        dm_smlLibFree(pElem);
        return rc;
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

short dm_buildDevInfDevInfContent(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDevInfPtr_t pElem;
    short rc;
    /* Modified by Tomy to allow <UTC></UTC>, <SupportNumberOfChanges></SupportNumberOfChanges> and <SupportLargeObjs></SupportLargeObjs> */
    SmlPcdataPtr_t tmp_ptr;
    /* End modified by Tomy */

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pElem = (SmlDevInfDevInfPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfDevInf_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfDevInf_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        case TN_DEVINF_VERDTD:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->verdtd);
            break;
        case TN_DEVINF_MAN:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->man);
            break;
        case TN_DEVINF_MOD:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->mod);
            break;
        case TN_DEVINF_OEM:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->oem);
            break;
        case TN_DEVINF_FWV:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->fwv);
            break;
        case TN_DEVINF_SWV:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->swv);
            break;
        case TN_DEVINF_HWV:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->hwv);
            break;
        case TN_DEVINF_DEVID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->devid);
            break;
        case TN_DEVINF_DEVTYP:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->devtyp);
            break;
        case TN_DEVINF_DATASTORE:
            rc = dm_buildDevInfDataStoreList(pDecoder,
                    (void*) &pElem->datastore);
            break;
        case TN_DEVINF_CTCAP:
            rc = dm_buildDevInfCtcap(pDecoder, (void*) &pElem->ctcap);
            break;
        case TN_DEVINF_EXT:
            rc = dm_buildDevInfExtList(pDecoder, (void*) &pElem->ext);
            break;
            /* SCTSTK - 18/03/2002 S.H. 2002-04-05 : SyncML 1.1 */
        case TN_DEVINF_UTC:
            pElem->flags |= SmlDevInfUTC_f;
            /* Modified by Tomy to allow <UTC></UTC> */
            tmp_ptr = NULL;
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder, (void*) &tmp_ptr);
            if (tmp_ptr->contentType
                    != SML_PCDATA_UNDEFINED&& tmp_ptr->extension != SML_EXT_UNDEFINED && tmp_ptr->length != 0 && tmp_ptr->content != NULL)rc
                = SML_ERR_XLT_INVAL_SYNCML_DOC;
            /* End modified by Tomy */
            dm_smlFreePcdata(__FILE__, __LINE__, tmp_ptr);
            break;
        case TN_DEVINF_NOFM:
            pElem->flags |= SmlDevInfNOfM_f;
            /* Modified by Tomy to allow <SupportNumberOfChanges></SupportNumberOfChanges> */
            tmp_ptr = NULL;
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder, (void*) &tmp_ptr);
            if (tmp_ptr->contentType
                    != SML_PCDATA_UNDEFINED&& tmp_ptr->extension != SML_EXT_UNDEFINED && tmp_ptr->length != 0 && tmp_ptr->content != NULL)rc
                = SML_ERR_XLT_INVAL_SYNCML_DOC;
            /* End modified by Tomy */
            dm_smlFreePcdata(__FILE__, __LINE__, tmp_ptr);
            break;
        case TN_DEVINF_LARGEOBJECT:
            pElem->flags |= SmlDevInfLargeObject_f;
            /* Modified by Tomy to allow <SupportLargeObjs></SupportLargeObjs> */
            tmp_ptr = NULL;
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder, (void*) &tmp_ptr);
            if (tmp_ptr->contentType
                    != SML_PCDATA_UNDEFINED&& tmp_ptr->extension != SML_EXT_UNDEFINED && tmp_ptr->length != 0 && tmp_ptr->content != NULL)rc
                = SML_ERR_XLT_INVAL_SYNCML_DOC;
            /* End modified by Tomy */
            dm_smlFreePcdata(__FILE__, __LINE__, tmp_ptr);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlLibFree(pElem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

short dm_buildDevInfDataStoreList(XltDecoderPtr_t pDecoder, void* *ppElem) {
    SmlDevInfDatastoreListPtr_t pElem = NULL, pPrev = NULL;

    pElem = (SmlDevInfDatastoreListPtr_t) *ppElem;

    /* advance to the end of the list, and create ther an empty list element */
    while (pElem != NULL ) {
        pPrev = pElem;
        pElem = pPrev->next;
    }
    if ((pElem = (SmlDevInfDatastoreListPtr_t) dm_smlLibMalloc(__FILE__,
            __LINE__, sizeof(SmlDevInfDatastoreList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfDatastoreList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
        pPrev->next = pElem;
    else
        /* nope we created a new list */
        *ppElem = pElem;
    pElem->data = NULL;
    /* at this point pElem should point to an valid list element */
    return dm_buildDevInfDataStoreCmd(pDecoder, (void*) &pElem->data);
}

short dm_buildDevInfDataStoreCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDatastorePtr_t pElem;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pElem = (SmlDevInfDatastorePtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfDatastore_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfDatastore_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        /* PCDATA elements */
        case TN_DEVINF_SOURCEREF:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->sourceref);
            break;
        case TN_DEVINF_DISPLAYNAME:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->displayname);
            break;
        case TN_DEVINF_MAXGUIDSIZE:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->maxguidsize);
            break;
        case TN_DEVINF_RXPREF:
            rc = dm_buildDevInfXmitCmd(pDecoder, (void*) &pElem->rxpref);
            break;
        case TN_DEVINF_TXPREF:
            rc = dm_buildDevInfXmitCmd(pDecoder, (void*) &pElem->txpref);
            break;
        case TN_DEVINF_RX:
            rc = dm_buildDevInfXmitList(pDecoder, (void*) &pElem->rx);
            break;
        case TN_DEVINF_TX:
            rc = dm_buildDevInfXmitList(pDecoder, (void*) &pElem->tx);
            break;
        case TN_DEVINF_DSMEM:
            rc = dm_buildDevInfDSMemCmd(pDecoder, (void*) &pElem->dsmem);
            break;
        case TN_DEVINF_SYNCCAP:
            rc = dm_buildDevInfSyncCapCmd(pDecoder, (void*) &pElem->synccap);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlLibFree(pElem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

short dm_buildDevInfXmitCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfXmitPtr_t pXmit;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pXmit = (SmlDevInfXmitPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfXmit_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pXmit, 0, sizeof(SmlDevInfXmit_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pXmit;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pXmit);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        /* PCDATA elements */
        case TN_DEVINF_CTTYPE:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pXmit->cttype);
            break;
        case TN_DEVINF_VERCT:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pXmit->verct);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlLibFree(pXmit);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlLibFree(pXmit);
            return rc;
        }
    }
    *ppElem = pXmit;

    return SML_ERR_OK;
}

short dm_buildDevInfXmitList(XltDecoderPtr_t pDecoder, void* *ppElem) {
    SmlDevInfXmitListPtr_t pElem = NULL, pPrev = NULL;

    pElem = (SmlDevInfXmitListPtr_t) *ppElem;

    /* advance to the end of the list, and create ther an empty list element */
    while (pElem != NULL ) {
        pPrev = pElem;
        pElem = pPrev->next;
    }
    if ((pElem = (SmlDevInfXmitListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfXmitList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfXmitList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
        pPrev->next = pElem;
    else
        /* nope we created a new list */
        *ppElem = pElem;
    pElem->data = NULL;
    /* at this point pElem should point to an valid list element */
    return dm_buildDevInfXmitCmd(pDecoder, (void*) &pElem->data);
}

short dm_buildDevInfDSMemCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDSMemPtr_t pElem;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pElem = (SmlDevInfDSMemPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfDSMem_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfDSMem_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        /* PCDATA elements */
        case TN_DEVINF_SHAREDMEM:
            // %%% luz:2003-04-28: made work as a flag
            pElem->flags |= SmlDevInfSharedMem_f;
            // rc = dm_buildPCData(pDecoder, (VoidPtr_t)&pElem->shared);
            break;
        case TN_DEVINF_MAXMEM:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->maxmem);
            break;
        case TN_DEVINF_MAXID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->maxid);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlLibFree(pElem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

short dm_buildDevInfSyncCapCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfSyncCapPtr_t pElem;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pElem = (SmlDevInfSyncCapPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfSyncCap_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfSyncCap_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        case TN_DEVINF_SYNCTYPE:
            rc = dm_buildPCDataList(pDecoder, (void*) &pElem->synctype);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlLibFree(pElem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

short dm_buildDevInfCtcap(XltDecoderPtr_t pDecoder, void* *ppElem) {
    SmlDevInfCtcapListPtr_t pCtcap = NULL, pPrev = NULL;
    SmlDevInfCTDataPropListPtr_t pOldProp = NULL, pProp = NULL;
    SmlDevInfCTDataListPtr_t pOldParam = NULL, pParam = NULL;
    SmlDevInfCtcapListPtr_t pElem = NULL;
    XltDecScannerPtr_t pScanner;
    short rc;

    pElem = (SmlDevInfCtcapListPtr_t) *ppElem;
    pScanner = pDecoder->scanner;

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        case TN_DEVINF_CTTYPE:
            pCtcap = (SmlDevInfCtcapListPtr_t) *ppElem;
            /* advance to the end of the list, and create ther an empty list element */
            while (pCtcap != NULL ) {
                pPrev = pCtcap;
                pCtcap = pPrev->next;
            }
            if ((pCtcap = (SmlDevInfCtcapListPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, sizeof(SmlDevInfCtcapList_t))) == NULL)
                return SML_ERR_NOT_ENOUGH_SPACE;
            dm_smlLibMemset(pCtcap, 0, sizeof(SmlDevInfCtcapList_t));
            if (pPrev != NULL) /* we already had some entries in the list */
                pPrev->next = pCtcap;
            else
                /* nope we created a new list */
                *ppElem = pCtcap;
            pCtcap->data = (SmlDevInfCTCapPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, sizeof(SmlDevInfCTCap_t));
            if (pCtcap->data == NULL)
                return SML_ERR_NOT_ENOUGH_SPACE;
            dm_smlLibMemset(pCtcap->data, 0, sizeof(SmlDevInfCTCap_t));
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pCtcap->data->cttype);
            break;
        case TN_DEVINF_PROPNAME:
            pCtcap = (SmlDevInfCtcapListPtr_t) *ppElem;
            if (pCtcap == NULL)
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            while (pCtcap->next != NULL ) {
                pPrev = pCtcap;
                pCtcap = pCtcap->next;
            }
            /* here we are at the latest defined DevInfCTCapPtr_t */
            /* now we need to create a new DevInfCTDataPtr_t element, tostore the properties name */
            pOldProp = NULL;
            pProp = pCtcap->data->prop;
            while (pProp != NULL ) {
                pOldProp = pProp;
                pProp = pProp->next;
            }
            pProp = (SmlDevInfCTDataPropListPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, sizeof(SmlDevInfCTDataPropList_t));
            if (pProp == NULL)
                return SML_ERR_NOT_ENOUGH_SPACE;
            dm_smlLibMemset(pProp, 0, sizeof(SmlDevInfCTDataPropList_t));
            if (pOldProp != NULL)
                pOldProp->next = pProp;
            else
                pCtcap->data->prop = pProp;
            pProp->data = (SmlDevInfCTDataPropPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, sizeof(SmlDevInfCTDataProp_t));
            if (pProp->data == NULL)
                return SML_ERR_NOT_ENOUGH_SPACE;
            dm_smlLibMemset(pProp->data, 0, sizeof(SmlDevInfCTDataProp_t));
            pProp->data->prop = (SmlDevInfCTDataPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, sizeof(SmlDevInfCTData_t));
            if (pProp->data->prop == NULL)
                return SML_ERR_NOT_ENOUGH_SPACE;
            dm_smlLibMemset(pProp->data->prop, 0, sizeof(SmlDevInfCTData_t));
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pProp->data->prop->name);
            break;
        case TN_DEVINF_PARAMNAME:
            pCtcap = (SmlDevInfCtcapListPtr_t) *ppElem;
            if (pCtcap == NULL)
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            while (pCtcap->next != NULL ) {
                pPrev = pCtcap;
                pCtcap = pCtcap->next;
            }
            /* here we are at the latest defined DevInfCTCapPtr_t */
            pProp = pCtcap->data->prop;
            if (pProp == NULL)
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            while (pProp->next != NULL ) {
                pProp = pProp->next;
            }
            /* here we are at the latest defined PropList Element in the latest defined CTCap element */
            /* now lets insert a new Param element into this property */
            pOldParam = NULL;
            pParam = pProp->data->param;
            while (pParam != NULL ) {
                pOldParam = pParam;
                pParam = pParam->next;
            }
            pParam = (SmlDevInfCTDataListPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, sizeof(SmlDevInfCTDataList_t));
            if (pParam == NULL)
                return SML_ERR_NOT_ENOUGH_SPACE;
            dm_smlLibMemset(pParam, 0, sizeof(SmlDevInfCTDataList_t));
            if (pOldParam != NULL)
                pOldParam->next = pParam;
            else
                pProp->data->param = pParam;
            pParam->data = (SmlDevInfCTDataPtr_t) dm_smlLibMalloc(__FILE__,
                    __LINE__, sizeof(SmlDevInfCTData_t));
            if (pParam->data == NULL)
                return SML_ERR_NOT_ENOUGH_SPACE;
            dm_smlLibMemset(pParam->data, 0, sizeof(SmlDevInfCTData_t));
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pParam->data->name);
            break;
        case TN_DEVINF_DISPLAYNAME:
        case TN_DEVINF_VALENUM:
        case TN_DEVINF_DATATYPE:
        case TN_DEVINF_SIZE:
            /* The code for the above 4 is basically the same.
             * The hardpart is finding the right SmlDevInfCTDataPtr_t
             * struct, as it can be either within a Property or an Parameter.
             */
            pCtcap = (SmlDevInfCtcapListPtr_t) *ppElem;
            if (pCtcap == NULL)
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            while (pCtcap->next != NULL ) {
                pCtcap = pCtcap->next;
            }
            /* here we are at the latest defined DevInfCTCapPtr_t */
            pProp = pCtcap->data->prop;
            if (pProp == NULL)
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            while (pProp->next != NULL ) {
                pProp = pProp->next;
            }

            if (pProp->data == NULL)
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            if (pProp->data->prop == NULL)
                return SML_ERR_XLT_INVAL_SYNCML_DOC;
            if (pProp->data->param == NULL) {
                /* No Param's yet so we have Property fields to fill */
                switch (pScanner->curtok->tagid) {
                case TN_DEVINF_DISPLAYNAME:
                    rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                            (void*) &pProp->data->prop->dname);
                    break;
                case TN_DEVINF_VALENUM:
                    rc = dm_buildPCDataList(pDecoder,
                            (void*) &pProp->data->prop->valenum);
                    break;
                case TN_DEVINF_DATATYPE:
                    rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                            (void*) &pProp->data->prop->datatype);
                    break;
                case TN_DEVINF_SIZE:
                    rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                            (void*) &pProp->data->prop->size);
                    break;
                default:
                    break;
                }
            } else {
                pParam = pProp->data->param;
                while (pParam->next != NULL ) {
                    pParam = pParam->next;
                }
                if (pParam->data == NULL)
                    return SML_ERR_XLT_INVAL_SYNCML_DOC;
                switch (pScanner->curtok->tagid) {
                case TN_DEVINF_DISPLAYNAME:
                    rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                            (void*) &pParam->data->dname);
                    break;
                case TN_DEVINF_VALENUM:
                    rc = dm_buildPCDataList(pDecoder,
                            (void*) &pParam->data->valenum);
                    break;
                case TN_DEVINF_DATATYPE:
                    rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                            (void*) &pParam->data->datatype);
                    break;
                case TN_DEVINF_SIZE:
                    rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                            (void*) &pParam->data->size);
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlLibFree(pElem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlLibFree(pElem);
            return rc;
        }
    } /* eof while */
    pElem = *ppElem;
    return SML_ERR_OK;
}

short dm_buildDevInfExtList(XltDecoderPtr_t pDecoder, void* *ppElem) {
    SmlDevInfExtListPtr_t pElem = NULL, pPrev = NULL;

    pElem = (SmlDevInfExtListPtr_t) *ppElem;

    /* advance to the end of the list, and create ther an empty list element */
    while (pElem != NULL ) {
        pPrev = pElem;
        pElem = pPrev->next;
    }
    if ((pElem = (SmlDevInfExtListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfExtList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfExtList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
        pPrev->next = pElem;
    else
        /* nope we created a new list */
        *ppElem = pElem;
    pElem->data = NULL;
    /* at this point pElem should point to an valid list element */
    return dm_buildDevInfExtCmd(pDecoder, (void*) &pElem->data);
}

short dm_buildDevInfExtCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfExtPtr_t pElem;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pElem = (SmlDevInfExtPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlDevInfExt_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pElem, 0, sizeof(SmlDevInfExt_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
        case TN_DEVINF_XNAM:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pElem->xnam);
            break;
        case TN_DEVINF_XVAL:
            rc = dm_buildPCDataList(pDecoder, (void*) &pElem->xval);
            break;
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlLibFree(pElem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

#endif

#endif

/*******************codec base functions************************/

/**
 * FUNCTION: dm_getCodePage
 *
 * Returns the code page which belongs to a certain PCDATA extension type.
 *
 * PRE-Condition:   valid PCDATA extension type
 *
 * POST-Condition:  the code page is returned
 *
 * IN:              ext, the PCDATA extension type
 *
 * RETURN:          the code page
 */
unsigned char dm_getCodePage(SmlPcdataExtension_t ext) {
#ifdef __USE_METINF__
    if (ext == SML_EXT_METINF)
        return 1;
#endif
#ifdef __USE_DEVINF__
    if (ext == SML_EXT_DEVINF)
        return 0;
#endif
    return 0;
}

/**
 * FUNCTION: getTagIDByStringAndCodepage
 *
 * Returns the tag ID which belongs to a tag string in a certain codepage
 *
 * PRE-Condition:   valid tag string, valid code page
 *
 * POST-Condition:  tag id is returned
 *
 * IN:              tag, the string representation of the tag
 *                  cp, code page group for the tag
 *                  pTagID, the tag id of the tag
 *
 * RETURN:          0, if OK
 */

short dm_getTagIDByStringAndExt(char* tag, SmlPcdataExtension_t ext,
        XltTagID_t *pTagID) {
    int i = 0;
    TagPtr_t pTags = dm_getTagTable(ext);
    if (pTags == NULL) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    for (i = 0; ((pTags + i)->id) != TN_UNDEF; i++) {
        if (*(pTags + i)->xml != *tag)
            continue; // if the first char doesn't match we skip the strcmp to speed things up
        if (dm_smlLibStrcmp(((pTags + i)->xml), tag) == 0) {
            *pTagID = (pTags + i)->id;
            return SML_ERR_OK;
        }
    }
    *pTagID = TN_UNDEF;
    return SML_ERR_XLT_INVAL_PROTO_ELEM;
}

/**
 * FUNCTION: getTagIDByByteAndCodepage
 *
 * Returns the tag ID which belongs to a tag byte in a certain codepage
 *
 * PRE-Condition:   valid tag byte, valid code page
 *
 * POST-Condition:  tag id is returned
 *
 * IN:              tag, the byte representation of the tag
 *                  cp, code page group for the tag
 *                  pTagID, the tag id of the tag
 *
 * RETURN:          0, if OK
 */
short dm_getTagIDByByteAndExt(unsigned char tag, SmlPcdataExtension_t ext,
        XltTagID_t *pTagID) {

    int i = 0;
    TagPtr_t pTags = dm_getTagTable(ext);
    if (pTags == NULL) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    while (((pTags + i)->id) != TN_UNDEF) {
        if (((pTags + i)->wbxml) == tag) {
            *pTagID = (pTags + i)->id;
            return SML_ERR_OK;
        }
        i++;
    }
    *pTagID = TN_UNDEF;
    return SML_ERR_XLT_INVAL_PROTO_ELEM;
}

/**
 * FUNCTION: dm_getTagString
 *
 * Returns a tag string which belongs to a tag ID.
 * This function is needed for the XML encoding
 *
 * PRE-Condition:   valid tag ID, the tagSring has to be allocated
 *
 * POST-Condition:  tag string is returned
 *
 * IN:              tagId, the ID for the tag
 *
 * IN/OUT:          tagString, allocated string into which the XML
 *                             tag string will be written
 *
 * RETURN:          0,if OK
 */
#ifdef __SML_XML__
short dm_getTagString(XltTagID_t tagID, char* tagString,
        SmlPcdataExtension_t ext) {
    int i = 0;
    TagPtr_t pTags = dm_getTagTable(ext);
    if (pTags == NULL) {
        tagString[0] = '\0';
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    while (((pTags + i)->id) != TN_UNDEF) {
        if ((((pTags + i)->id) == tagID)) {
            char* _tmp = (pTags + i)->xml;
            dm_smlLibStrcpy(tagString, _tmp);
            return SML_ERR_OK;
        }
        i++;
    }
    tagString[0] = '\0';
    return SML_ERR_XLT_INVAL_PROTO_ELEM;
}
#endif

static TagPtr_t dm_getTagTable(SmlPcdataExtension_t ext) {
    int mySize = 0;
    TagPtr_t _tmpTagPtr;
    SyncMLInfoPtr_t pGA = NULL;

    /* standard SyncML codepage */
    static const Tag_t syncml[] = { { _TOKEN(TN_ADD, 0x05, "Add") }, {
            _TOKEN(TN_ALERT, 0x06, "Alert") }, {
            _TOKEN(TN_ARCHIVE, 0x07, "Archive") }, {
            _TOKEN(TN_ATOMIC, 0x08, "Atomic") }, {
            _TOKEN(TN_CHAL, 0x09, "Chal") }, { _TOKEN(TN_CMD, 0x0A, "Cmd") }, {
            _TOKEN(TN_CMDID, 0x0B, "CmdID") }, {
            _TOKEN(TN_CMDREF, 0x0C, "CmdRef") }, {
            _TOKEN(TN_COPY, 0x0D, "Copy") }, { _TOKEN(TN_CRED, 0x0E, "Cred") },
            { _TOKEN(TN_DATA, 0x0F, "Data") }, {
                    _TOKEN(TN_DELETE, 0x10, "Delete") }, {
                    _TOKEN(TN_EXEC, 0x11, "Exec") }, {
                    _TOKEN(TN_FINAL, 0x12, "Final") }, {
                    _TOKEN(TN_GET, 0x13, "Get") }, {
                    _TOKEN(TN_ITEM, 0x14, "Item") }, {
                    _TOKEN(TN_LANG, 0x15, "Lang") }, {
                    _TOKEN(TN_LOCNAME, 0x16, "LocName") }, {
                    _TOKEN(TN_LOCURI, 0x17, "LocURI") }, {
                    _TOKEN(TN_MAP, 0x18, "Map") }, {
                    _TOKEN(TN_MAPITEM, 0x19, "MapItem") }, {
                    _TOKEN(TN_META, 0x1A, "Meta") }, {
                    _TOKEN(TN_MSGID, 0x1B, "MsgID") }, {
                    _TOKEN(TN_MSGREF, 0x1C, "MsgRef") }, {
                    _TOKEN(TN_NORESP, 0x1D, "NoResp") }, {
                    _TOKEN(TN_NORESULTS, 0x1E, "NoResults") }, {
                    _TOKEN(TN_PUT, 0x1F, "Put") }, {
                    _TOKEN(TN_REPLACE, 0x20, "Replace") }, {
                    _TOKEN(TN_RESPURI, 0x21, "RespURI") }, {
                    _TOKEN(TN_RESULTS, 0x22, "Results") }, {
                    _TOKEN(TN_SEARCH, 0x23, "Search") }, {
                    _TOKEN(TN_SEQUENCE, 0x24, "Sequence") }, {
                    _TOKEN(TN_SESSIONID, 0x25, "SessionID") }, {
                    _TOKEN(TN_SFTDEL, 0x26, "SftDel") }, {
                    _TOKEN(TN_SOURCE, 0x27, "Source") }, {
                    _TOKEN(TN_SOURCEREF, 0x28, "SourceRef") }, {
                    _TOKEN(TN_STATUS, 0x29, "Status") }, {
                    _TOKEN(TN_SYNC, 0x2A, "Sync") }, {
                    _TOKEN(TN_SYNCBODY, 0x2B, "SyncBody") }, {
                    _TOKEN(TN_SYNCHDR, 0x2C, "SyncHdr") }, {
                    _TOKEN(TN_SYNCML, 0x2D, "SyncML") }, {
                    _TOKEN(TN_TARGET, 0x2E, "Target") }, {
                    _TOKEN(TN_TARGETREF, 0x2F, "TargetRef") }, {
                    _TOKEN(TN_VERSION, 0x31, "VerDTD") }, {
                    _TOKEN(TN_PROTO, 0x32, "VerProto") }, {
                    _TOKEN(TN_NUMBEROFCHANGES,0x33, "NumberOfChanges") }, {
                    _TOKEN(TN_MOREDATA, 0x34, "MoreData") }, {
                    _TOKEN(TN_UNDEF, 0x00, NULL) } };

#ifdef __USE_METINF__
    static const Tag_t metinf[] = {
            { _TOKEN(TN_METINF_ANCHOR, 0x05, "Anchor") }, {
                    _TOKEN(TN_METINF_EMI, 0x06, "EMI") }, {
                    _TOKEN(TN_METINF_FORMAT, 0x07, "Format") }, {
                    _TOKEN(TN_METINF_FREEID, 0x08, "FreeID") }, {
                    _TOKEN(TN_METINF_FREEMEM, 0x09, "FreeMem") }, {
                    _TOKEN(TN_METINF_LAST, 0x0A, "Last") }, {
                    _TOKEN(TN_METINF_MARK, 0x0B, "Mark") }, {
                    _TOKEN(TN_METINF_MAXMSGSIZE, 0x0C, "MaxMsgSize") }, {
                    _TOKEN(TN_METINF_MEM, 0x0D, "Mem") }, {
                    _TOKEN(TN_METINF_METINF, 0x0E, "MetInf") }, {
                    _TOKEN(TN_METINF_NEXT, 0x0F, "Next") }, {
                    _TOKEN(TN_METINF_NEXTNONCE, 0x10, "NextNonce") }, {
                    _TOKEN(TN_METINF_SHAREDMEM, 0x11, "SharedMem") }, {
                    _TOKEN(TN_METINF_SIZE, 0x12, "Size") }, {
                    _TOKEN(TN_METINF_TYPE, 0x13, "Type") }, {
                    _TOKEN(TN_METINF_VERSION, 0x14, "Version") }, {
                    _TOKEN(TN_METINF_MAXOBJSIZE, 0x15, "MaxObjSize") }, {
                    _TOKEN(TN_UNDEF, 0x00, NULL) } };
#endif

#ifdef __USE_DEVINF__
    static const Tag_t devinf[] = { { _TOKEN(TN_DEVINF_CTCAP, 0x05, "CTCap") },
            { _TOKEN(TN_DEVINF_CTTYPE, 0x06, "CTType") }, {
                    _TOKEN(TN_DEVINF_DATASTORE, 0x07, "DataStore") }, {
                    _TOKEN(TN_DEVINF_DATATYPE, 0x08, "DataType") }, {
                    _TOKEN(TN_DEVINF_DEVID, 0x09, "DevID") }, {
                    _TOKEN(TN_DEVINF_DEVINF, 0x0A, "DevInf") }, {
                    _TOKEN(TN_DEVINF_DEVTYP, 0x0B, "DevTyp") }, {
                    _TOKEN(TN_DEVINF_DISPLAYNAME, 0x0C, "DisplayName") }, {
                    _TOKEN(TN_DEVINF_DSMEM, 0x0D, "DSMem") }, {
                    _TOKEN(TN_DEVINF_EXT, 0x0E, "Ext") }, {
                    _TOKEN(TN_DEVINF_FWV, 0x0F, "FwV") }, {
                    _TOKEN(TN_DEVINF_HWV, 0x10, "HwV") }, {
                    _TOKEN(TN_DEVINF_MAN, 0x11, "Man") }, {
                    _TOKEN(TN_DEVINF_MAXGUIDSIZE, 0x12, "MaxGUIDSize") }, {
                    _TOKEN(TN_DEVINF_MAXID, 0x13, "MaxID") }, {
                    _TOKEN(TN_DEVINF_MAXMEM, 0x14, "MaxMem") }, {
                    _TOKEN(TN_DEVINF_MOD, 0x15, "Mod") }, {
                    _TOKEN(TN_DEVINF_OEM, 0x16, "OEM") }, {
                    _TOKEN(TN_DEVINF_PARAMNAME, 0x17, "ParamName") }, {
                    _TOKEN(TN_DEVINF_PROPNAME, 0x18, "PropName") }, {
                    _TOKEN(TN_DEVINF_RX, 0x19, "Rx") }, {
                    _TOKEN(TN_DEVINF_RXPREF, 0x1A, "Rx-Pref") }, {
                    _TOKEN(TN_DEVINF_SHAREDMEM, 0x1B, "SharedMem") }, {
                    _TOKEN(TN_DEVINF_SIZE, 0x1C, "Size") }, {
                    _TOKEN(TN_DEVINF_SOURCEREF, 0x1D, "SourceRef") }, {
                    _TOKEN(TN_DEVINF_SWV, 0x1E, "SwV") }, {
                    _TOKEN(TN_DEVINF_SYNCCAP, 0x1F, "SyncCap") }, {
                    _TOKEN(TN_DEVINF_SYNCTYPE, 0x20, "SyncType") }, {
                    _TOKEN(TN_DEVINF_TX, 0x21, "Tx") }, {
                    _TOKEN(TN_DEVINF_TXPREF, 0x22, "Tx-Pref") }, {
                    _TOKEN(TN_DEVINF_VALENUM, 0x23, "ValEnum") }, {
                    _TOKEN(TN_DEVINF_VERCT, 0x24, "VerCT") }, {
                    _TOKEN(TN_DEVINF_VERDTD, 0x25, "VerDTD") }, {
                    _TOKEN(TN_DEVINF_XNAM, 0x26, "XNam") }, {
                    _TOKEN(TN_DEVINF_XVAL, 0x27, "XVal") },
            // %%% luz:2003-04-28 : added these, they were missing
            { _TOKEN(TN_DEVINF_UTC, 0x28, "UTC") }, {
                    _TOKEN(TN_DEVINF_NOFM, 0x29, "SupportNumberOfChanges") }, {
                    _TOKEN(TN_DEVINF_LARGEOBJECT, 0x2A, "SupportLargeObjs") },
            // %%% end luz
            { _TOKEN(TN_UNDEF, 0x00, NULL) } };
#endif

    _tmpTagPtr = NULL;
    pGA = dm_pGlobalAnchor;
    if (pGA == NULL)
        return NULL ;

    /* get the correct codepage */
    if (ext == SML_EXT_UNDEFINED) {
        _tmpTagPtr = pGA->tokTbl->SyncML;
        if (_tmpTagPtr == NULL) {
            mySize = sizeof(syncml);
            pGA->tokTbl->SyncML = (TagPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
                    mySize);
            if (pGA->tokTbl->SyncML == NULL)
                return NULL ;
            dm_smlLibMemcpy(pGA->tokTbl->SyncML, &syncml, mySize);
            _tmpTagPtr = pGA->tokTbl->SyncML;
        }

    }

#ifdef __USE_METINF__
    else if (ext == SML_EXT_METINF) {
        _tmpTagPtr = pGA->tokTbl->MetInf;
        if (_tmpTagPtr == NULL) {
            mySize = sizeof(metinf);
            pGA->tokTbl->MetInf = (TagPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
                    mySize);
            if (pGA->tokTbl->MetInf == NULL)
                return NULL ;
            dm_smlLibMemcpy(pGA->tokTbl->MetInf, &metinf, mySize);
            _tmpTagPtr = pGA->tokTbl->MetInf;
        }

    }
#endif

#ifdef __USE_DEVINF__
    else if (ext == SML_EXT_DEVINF) {
        _tmpTagPtr = pGA->tokTbl->DevInf;
        if (_tmpTagPtr == NULL) {
            mySize = sizeof(devinf);
            pGA->tokTbl->DevInf = (TagPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
                    mySize);
            if (pGA->tokTbl->DevInf == NULL)
                return NULL ;
            dm_smlLibMemcpy(pGA->tokTbl->DevInf, &devinf, mySize);
            _tmpTagPtr = pGA->tokTbl->DevInf;
        }

    }
#endif
    return _tmpTagPtr;
}

/**
 * FUNCTION: dm_getExtName
 *
 * Returns the official name for a given extention/sub-DTD
 * and stored it in 'name'. If not found name isn't modified
 */
// %%% luz:2003-04-24: added syncmlvers parameter
// %%% luz:2003-07-31: changed to vers enum
static short dm_getExtName(SmlPcdataExtension_t ext, char* *name,
        SmlVersion_t vers) {
    DtdPtr_t dtdhead = dm_getDtdTable();
    DtdPtr_t dtd = dtdhead;
    const char *dtdname;
    if (!dtdhead)
        return -1;
    for (; dtd->ext != SML_EXT_LAST; dtd++) {
        if (!dtd->name)
            continue; /* skip empty names (should not appear but better be on the safe side) */
        if (dtd->ext == ext) {
            char* _tmp;
            // this is the default
            dtdname = dtd->name;
            // %%% luz:2003-04-24: added dynamic generation of namespace according to SyncML version
            if (ext == SML_EXT_UNDEFINED && vers != SML_VERS_UNDEF) {
                // this requests SyncML namespace
                dtdname = dm_SyncMLNamespacesPim[vers];
            }
            _tmp = dm_smlLibMalloc(__FILE__, __LINE__,
                    dm_smlLibStrlen(dtdname) + 1);
            if (!_tmp) {
                dm_freeDtdTable(dtdhead);
                return SML_ERR_NOT_ENOUGH_SPACE;
            }
            dm_smlLibStrcpy(_tmp, dtdname);
            dm_freeDtdTable(dtdhead);
            *name = _tmp;
            return SML_ERR_OK;
        }
    }
    dm_freeDtdTable(dtdhead);
    return -1;
}

/**
 * FUNCTION: getCpByName
 *
 * Returns the codepage constant assoziated with the name stored in 'ns'
 *
 * RETURN:             a SmlPcdataExtension_t representing the corresponding codepage id.
 *                     If no corresponding codepage is found -1 is returned.
 */
SmlPcdataExtension_t dm_getExtByName(char* ns) {
    DtdPtr_t dtdhead = dm_getDtdTable();
    DtdPtr_t dtd = dtdhead;
    SmlPcdataExtension_t ext = (SmlPcdataExtension_t) 255;
    if (!dtdhead)
        return SML_EXT_UNDEFINED;
    for (; dtd->ext != SML_EXT_LAST; dtd++) {
        const char *dtdname = dtd->name;
        if (!dtdname)
            continue; /* skip empty names (should not appear but better be on the safe side) */
        if (dtd->ext == SML_EXT_UNDEFINED
                && dm_smlLibStrncmp("SYNCML:SYNCML", ns, 13) == 0) {
            // SyncML namespace is ok without checking version!
            ext = SML_EXT_UNDEFINED;
            break;
        } else if (dm_smlLibStrcmp(dtdname, ns) == 0) {
            ext = dtd->ext;
            break;
        }
    }
    dm_freeDtdTable(dtdhead);
    return ext;
}

/**
 * FUNCTION: dm_getTagByte
 *
 * Returns a WBXML byte which belongs to a tag ID in a defined codepage.
 * This function is needed for the WBXML encoding
 *
 * PRE-Condition:   valid tag ID, valid code page
 *
 * POST-Condition:  tag byte is returned
 *
 * IN:              tagId, the ID for the tag
 *                  cp, code page group for the tag
 *                  pTagByte, the byte representation of the tag
 *
 * RETURN:          0, if OK
 */
static short dm_getTagByte(XltTagID_t tagID, SmlPcdataExtension_t ext,
        unsigned char *pTagByte) {
    int i = 0;
    TagPtr_t pTags = dm_getTagTable(ext);
    if (pTags == NULL) {
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    while (((pTags + i)->id) != TN_UNDEF) {
        if (((pTags + i)->id) == tagID) {
            *pTagByte = (pTags + i)->wbxml;
            return SML_ERR_OK;
        }
        i++;
    }
    *pTagByte = 0;
    return SML_ERR_XLT_INVAL_PROTO_ELEM;
}

static DtdPtr_t dm_getDtdTable() {

    // WSM method wasting a lot of memory
    DtdPtr_t _tmpPtr;

    Dtd_t XltDtdTbl[] = { { "SYNCML:SYNCML1.0", SML_EXT_UNDEFINED }, {
            "syncml:metinf", SML_EXT_METINF },
            { "syncml:devinf", SML_EXT_DEVINF }, { NULL, SML_EXT_LAST } };
    _tmpPtr = NULL;
    _tmpPtr = (DtdPtr_t) dm_smlLibMalloc(__FILE__, __LINE__, sizeof(XltDtdTbl));
    if (_tmpPtr == NULL)
        return NULL ;
    dm_smlLibMemcpy(_tmpPtr, &XltDtdTbl, sizeof(XltDtdTbl));
    return _tmpPtr;
}

// free table obtained with dm_getDtdTable()
static void dm_freeDtdTable(DtdPtr_t tbl) {
    // only with WSM this is an allocated table
    dm_smlLibFree(tbl);
    return;
}

static PEBuilderPtr_t dm_getPETable(void) {
    PEBuilderPtr_t _tmpPEPtr;
    PEBuilder_t PE[] = { { TN_ADD, SML_PE_ADD, dm_buildGenericCmd }, { TN_ALERT,
            SML_PE_ALERT, dm_buildAlert }, { TN_ATOMIC, SML_PE_ATOMIC_START,
            dm_buildAtomOrSeq }, { TN_COPY, SML_PE_COPY, dm_buildGenericCmd }, {
            TN_DELETE, SML_PE_DELETE, dm_buildGenericCmd }, { TN_EXEC,
            SML_PE_EXEC, dm_buildExec },
            { TN_GET, SML_PE_GET, dm_buildPutOrGet }, { TN_MAP, SML_PE_MAP,
                    dm_buildMap }, { TN_PUT, SML_PE_PUT, dm_buildPutOrGet }, {
                    TN_RESULTS, SML_PE_RESULTS, dm_buildResults }, { TN_SEARCH,
                    SML_PE_SEARCH, buildSearch }, { TN_SEQUENCE,
                    SML_PE_SEQUENCE_START, dm_buildAtomOrSeq }, { TN_STATUS,
                    SML_PE_STATUS, dm_buildStatus }, { TN_SYNC,
                    SML_PE_SYNC_START, dm_buildSync }, { TN_REPLACE,
                    SML_PE_REPLACE, dm_buildGenericCmd }, { TN_UNDEF,
                    SML_PE_UNDEF, 0 } };

    _tmpPEPtr = dm_smlLibMalloc(__FILE__, __LINE__, sizeof(PE));
    if (_tmpPEPtr == NULL)
        return NULL ;
    dm_smlLibMemcpy(_tmpPEPtr, &PE, sizeof(PE));
    return _tmpPEPtr;
}

static PEEncPtr_t dm_getPEEncTable(void) {
    PEEncPtr_t _tmpPEEncPtr;
    PEEnc_t const PE[] = { { TN_ADD, SML_PE_ADD }, { TN_ALERT, SML_PE_ALERT }, {
            TN_ATOMIC, SML_PE_ATOMIC_START },
            { TN_ATOMIC_END, SML_PE_ATOMIC_END }, { TN_COPY, SML_PE_COPY }, {
                    TN_DELETE, SML_PE_DELETE }, { TN_EXEC, SML_PE_EXEC }, {
                    TN_GET, SML_PE_GET }, { TN_MAP, SML_PE_MAP }, { TN_PUT,
                    SML_PE_PUT }, { TN_RESULTS, SML_PE_RESULTS }, { TN_SEARCH,
                    SML_PE_SEARCH }, { TN_SEQUENCE, SML_PE_SEQUENCE_START }, {
                    TN_SEQUENCE_END, SML_PE_SEQUENCE_END }, { TN_STATUS,
                    SML_PE_STATUS }, { TN_SYNC, SML_PE_SYNC_START }, {
                    TN_SYNC_END, SML_PE_SYNC_END },
            { TN_REPLACE, SML_PE_REPLACE }, { TN_UNDEF, SML_PE_UNDEF } };
    _tmpPEEncPtr = (PEEncPtr_t) dm_smlLibMalloc(__FILE__, __LINE__, sizeof(PE));
    if (_tmpPEEncPtr == NULL)
        return NULL ;
    dm_smlLibMemcpy(_tmpPEEncPtr, &PE, sizeof(PE));
    return _tmpPEEncPtr;
}

static short dm_getTNbyPE(SmlProtoElement_t pE, XltTagID_t *tagID) {
    int i = 0;
    PEEncPtr_t pPETbl = dm_getPEEncTable();
    if (pPETbl == NULL) {
        dm_smlLibFree(pPETbl);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    while (((pPETbl + i)->type) != SML_PE_UNDEF) {
        if (((pPETbl + i)->type) == pE) {
            *tagID = (pPETbl + i)->tagid;
            dm_smlLibFree(pPETbl);
            return SML_ERR_OK;
        }
        i++;
    }
    dm_smlLibFree(pPETbl);
    *tagID = TN_UNDEF;
    return SML_ERR_XLT_INVAL_PROTO_ELEM;
}

/**
 * FUNCTION: dm_wbxmlWriteTypeToBuffer
 *
 * Write a content of a certain WBXML element type (e.g. STR_I) to the global buffer
 *
 * PRE-Condition:   valid parameters
 *
 * POST-Condition:  the content is written to the wbxml buffer with the leading
 *                  bytes for the opaque data type or the STR_I data type
 *
 * IN:              pContent, the character pointer referencing the content to
 *                            write to the buffer
 *                  elType, the element type to write to the buffer (e.g. STR_I)
 *                  size, the content length
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
short dm_wbxmlWriteTypeToBuffer(const unsigned char* pContent,
        XltElementType_t elType, long size, BufferMgmtPtr_t pBufMgr) {
    short _err;

    unsigned char _termstr = XLT_TERMSTR;
    unsigned char _tmp;

    switch ((int) elType) {
    case TAG: {

        return (dm_xltAddToBuffer(pContent, size, pBufMgr));

    }
    case STR_I: {
        _tmp = (unsigned char) dm_wbxmlGetGlobToken(STR_I);
        if (!_tmp)
            return SML_ERR_XLT_WBXML_UKN_TOK;

        //add the STR_I identifier
        if ((_err = dm_xltAddToBuffer(&_tmp, 1, pBufMgr)) != SML_ERR_OK)
            return _err;

        //add the string to the buffer
        if ((_err = dm_xltAddToBuffer(pContent,
                (!pContent) ? 0 : dm_smlLibStrlen((char*) pContent), pBufMgr))
                != SML_ERR_OK)
            return _err;

        //add the string terminator '\0'
        if ((_err = dm_xltAddToBuffer(&_termstr, 1, pBufMgr)) != SML_ERR_OK)
            return _err;

        return SML_ERR_OK;
    }
    case _OPAQUE: {
        _tmp = (unsigned char) dm_wbxmlGetGlobToken(_OPAQUE);
        if (!_tmp)
            return SML_ERR_XLT_WBXML_UKN_TOK;

        //add the OPAQUE identifier
        if ((_err = dm_xltAddToBuffer(&_tmp, 1, pBufMgr)) != SML_ERR_OK)
            return _err;

        //add the pContent length
        if ((_err = dm_wbxmlOpaqueSize2Buf(size, pBufMgr)) != SML_ERR_OK)
            return _err;

        //add the string buffer
        if ((_err = dm_xltAddToBuffer(pContent, size, pBufMgr)) != SML_ERR_OK)
            return _err;

        return SML_ERR_OK;
    }
    default:
        return SML_ERR_XLT_INVAL_PCDATA_TYPE;
    }

//  return SML_ERR_OK;unreachable
}

/**
 * FUNCTION: dm_wbxmlGetGlobToken
 *
 * Converts a element type into its wbxml token
 *
 * PRE-Condition:   valid element type
 *
 * POST-Condition:  return of wbxml token
 *
 * IN:              elType, element type
 *
 * OUT:             wbxml token
 *
 * RETURN:          wbxml token
 *                  0, if no matching wbxml token
 */
static unsigned char dm_wbxmlGetGlobToken(XltElementType_t elType) {

    typedef struct GlobTok_s {
        XltElementType_t id;
        unsigned char wbxml;
    } GlobTok_t;

    // encoding of global tokens; related to the type XML_ElementType_t
    GlobTok_t globtoken[] = { { END, 0x01 },  //Tag End
            { STR_I, 0x03 },  //Inline string
            { _OPAQUE, 0xC3 },  //Opaque Data
            { UNDEF, 0x00 } };

    int i = -1;
    while (globtoken[++i].id != UNDEF)
        if (globtoken[i].id == elType)
            return globtoken[i].wbxml;
    return 0;

}

/**
 * FUNCTION: dm_wbxmlOpaqueSize2Buf
 *
 * Converts a Long_t opaque size to a wbxml mb_u_int32 and adds it to the buffer
 *
 * PRE-Condition:   size of the content to be written as opaque datatype
 *
 * POST-Condition:  the size is converted to the mb_u_int32 representation and added
 *                  to the buffer
 *
 * IN:              size, length of the opaque data
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
static short dm_wbxmlOpaqueSize2Buf(long size, BufferMgmtPtr_t pBufMgr) {
    long _thresholdcount = 1;
    long _bytesNeeded = 0;
    unsigned char* _byteArray;
    unsigned char* _tmpByteArray;
    int32 i = 0;
    int32 j = 0;
    short _err;

    //j max = number of bytes of size
    for (j = 1; j <= sizeof(size); j++)/*lint !e737 !e574*/
    {
        //if the size of the content is smaller than the power of 128,j ->
        //one more byte is needed in the mb_u_int32 representation of WBXML
        _thresholdcount = _thresholdcount * 128;
        if (size < _thresholdcount) {
            _bytesNeeded = j;
            break;
        }
    }

    if (pBufMgr->spaceEvaluation == 0) {
        //allocate number of bytes needed by the mb_u_int32 data type
        if ((_byteArray = dm_smlLibMalloc(__FILE__, __LINE__, _bytesNeeded))
                == NULL)
            return SML_ERR_NOT_ENOUGH_SPACE;

        _tmpByteArray = _byteArray;

        //process al bytes in the mb_u_int32 data type
        for (i = 1; i <= _bytesNeeded; i++) {
            // the lowest byte needs a 0 in its highest bit -> no | 0x80
            if ((_bytesNeeded - i) == 0) {
                *_tmpByteArray = ((unsigned char) (size & 0x7F));
            }
            // all the other byte needs a 1 in its highest bit -> | 0x80
            else {
                // only the seven lower bits contain the size value -> >> 7
                *_tmpByteArray = ((unsigned char) (((size
                        >> (7 * (_bytesNeeded - i))) & 0x7F) | 0x80));
                _tmpByteArray++;
            }
        }

        _err = dm_xltAddToBuffer(_byteArray, _bytesNeeded, pBufMgr);

        dm_smlLibFree(_byteArray);
    } else {
        pBufMgr->smlXltWrittenBytes += _bytesNeeded;
        // %%% luz 2002-09-03: return value was missing here.
        _err = SML_ERR_OK;
    }

    return _err;
}

/**
 * FUNCTION: dm_wbxmlGenerateTag
 *
 * Generates a tag for a given tag ID and a given tag type
 *
 * PRE-Condition:   valid parameters
 *
 * POST-Condition:  a new wbxml tag is written to the buffer
 *
 * IN:              tagId, the ID for the tag to generate (TN_ADD, ...)
 *                  tagType, the tag type (e.g. Begin Tag -> TT_BEG, ...)
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
short dm_wbxmlGenerateTag(XltTagID_t tagId, XltTagType_t tagType,
        BufferMgmtPtr_t pBufMgr) {

    short _err = SML_ERR_OK;
    unsigned char _tmp = 0x00;

    //check if content byte has to be added to the tag
    switch (tagType) {
    //set the end tag
    case TT_END: {
        _tmp = (unsigned char) dm_wbxmlGetGlobToken(END);
        if (!_tmp)
            return SML_ERR_XLT_INVAL_TAG_TYPE;
        _err = dm_xltAddToBuffer((&_tmp), 1, pBufMgr);
        // remember the number of byte that must follow for the according  end-tag
        if (_err == SML_ERR_OK)
            pBufMgr->endTagSize -= 1;
        return _err;
    }
        //Begin and End Tag in one
    case TT_ALL: {
        _err = (unsigned char) dm_getTagByte(tagId, pBufMgr->smlCurExt, &_tmp);
        if ((!_tmp) || (_err != SML_ERR_OK))
            return _err;
        return dm_xltAddToBuffer((unsigned char*) (&_tmp), 1, pBufMgr);
    }
        //Only Begin Tag -> content follows -> content byte has to be added
    case TT_BEG: {
        _err = (unsigned char) dm_getTagByte(tagId, pBufMgr->smlCurExt, &_tmp);
        if ((!_tmp) || (_err != SML_ERR_OK))
            return _err;

        _tmp = ((unsigned char) (_tmp | XLT_CONTBYTE));
        _err = dm_xltAddToBuffer(&_tmp, 1, pBufMgr);
        // remember the number of byte that must follow for the according  end-tag
        if (_err == SML_ERR_OK)
            pBufMgr->endTagSize += 1;
        return _err;
    }
    default:
        return SML_ERR_XLT_INVAL_TAG_TYPE;
    }

    // return SML_ERR_OK;Unreachable
}

/**
 * FUNCTION: dm_xmlGenerateTag
 *
 * Generates a XML tag
 *
 * PRE-Condition:   valid parameters
 *
 * POST-Condition:  the XML tag is written to the XML buffer
 *
 * IN:              tagId, the ID for the tag to generate (TN_ADD, ...)
 *                  tagType, the tag type (e.g. Begin Tag -> TT_BEG, ...)
 *                  attFlag, indicates if the encoded tag contain Attributes in namespace extensions
 *
 * IN/OUT:          pBufMgr, pointer to a structure containing buffer management elements
 *
 * RETURN:          shows error codes of function,
 *                  0, if OK
 */
short dm_xmlGenerateTag(XltTagID_t tagId, XltTagType_t tagType,
        BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag) {
    short _err;

    unsigned char _begpar = XML_BEGPAR;
    unsigned char _tagdel = XML_TAGDEL;
    unsigned char _endpar = XML_ENDPAR;
    unsigned char _nstagstart[] = XML_NSSTART;
    unsigned char _nstagend[] = XML_NSEND;

    char* _tagstr;
    char* _tagnsattr = NULL;

    if ((_tagstr = (char*) dm_smlLibMalloc(__FILE__, __LINE__, XML_MAX_TAGLEN))
            == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;

    if ((_err = dm_getTagString(tagId, _tagstr, attFlag)) != SML_ERR_OK) {
        dm_smlLibFree(_tagstr);
        return _err;
    }

#if 0//del   2009.12.19 for delete warning
    if (!_tagstr) {/*lint !e774*/ // check again as _tagstr might be alterd in dm_getTagString
        dm_smlLibFree(_tagstr);
        return SML_ERR_XLT_INVAL_TAG_TYPE;
    }
#endif

    /* the <SyncML> tag _must_ have an xmlns attribute */
    if (attFlag != pBufMgr->smlActiveExt || tagId == TN_SYNCML) {
        // %%% luz:2003-07-31: now uses namespace from table according to version
        if (dm_getExtName(attFlag, &_tagnsattr, pBufMgr->vers) != SML_ERR_OK) {
            dm_smlLibFree(_tagstr);
            return SML_ERR_XLT_INVAL_TAG_TYPE;
        }
    }
    pBufMgr->smlActiveExt = attFlag;
    //check if content byte has to be added to the tag
    switch (tagType) {
    // set the end tag
    case TT_END: {
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_begpar), 1, pBufMgr))
                != SML_ERR_OK)
            break;
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_tagdel), 1, pBufMgr))
                != SML_ERR_OK)
            break;
        if ((_err = dm_xltAddToBuffer((unsigned char*) _tagstr,
                dm_smlLibStrlen(_tagstr), pBufMgr)) != SML_ERR_OK)
            break;
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_endpar), 1, pBufMgr))
                != SML_ERR_OK)
            break;
        if (tagId == pBufMgr->switchExtTag) {
            pBufMgr->smlActiveExt = pBufMgr->smlLastExt;
            pBufMgr->smlCurExt = pBufMgr->smlLastExt;
            pBufMgr->smlLastExt = attFlag;
        }
        // just forget the stored number ob bytes for this end-tag since written now
        pBufMgr->endTagSize -= (3 + dm_smlLibStrlen(_tagstr));
        break;
    }
        //Empty tag
    case TT_ALL: {
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_begpar), 1, pBufMgr))
                != SML_ERR_OK)
            break;
        if ((_err = dm_xltAddToBuffer((unsigned char*) _tagstr,
                dm_smlLibStrlen(_tagstr), pBufMgr)) != SML_ERR_OK)
            break;
        if (_tagnsattr) {
            if ((_err = dm_xltAddToBuffer((unsigned char*) (&_nstagstart), 8,
                    pBufMgr)) != SML_ERR_OK)
                break;
            if ((_err = dm_xltAddToBuffer((unsigned char*) _tagnsattr,
                    dm_smlLibStrlen(_tagnsattr), pBufMgr)) != SML_ERR_OK)
                break;
            if ((_err = dm_xltAddToBuffer((unsigned char*) &_nstagend, 1,
                    pBufMgr)) != SML_ERR_OK)
                break;
        }
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_tagdel), 1, pBufMgr))
                != SML_ERR_OK)
            break;
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_endpar), 1, pBufMgr))
                != SML_ERR_OK)
            break;

        break;
    }
        //Only Begin Tag -> content follows -> content byte has to be added
    case TT_BEG: {
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_begpar), 1, pBufMgr))
                != SML_ERR_OK)
            break;
        if ((_err = dm_xltAddToBuffer((unsigned char*) _tagstr,
                dm_smlLibStrlen(_tagstr), pBufMgr)) != SML_ERR_OK)
            break;
        if (_tagnsattr) {
            if ((_err = dm_xltAddToBuffer((unsigned char*) &_nstagstart, 8,
                    pBufMgr)) != SML_ERR_OK)
                break;
            if ((_err = dm_xltAddToBuffer((unsigned char*) _tagnsattr,
                    dm_smlLibStrlen(_tagnsattr), pBufMgr)) != SML_ERR_OK)
                break;
            if ((_err = dm_xltAddToBuffer((unsigned char*) &_nstagend, 1,
                    pBufMgr)) != SML_ERR_OK)
                break;
        }
        if ((_err = dm_xltAddToBuffer((unsigned char*) (&_endpar), 1, pBufMgr))
                != SML_ERR_OK)
            break;

        // remember the number of byte that must follow for the according  end-tag
        pBufMgr->endTagSize += (3 + dm_smlLibStrlen(_tagstr));
        break;
    }
    default: {
        dm_smlLibFree(_tagstr);
        dm_smlLibFree(_tagnsattr);
        return SML_ERR_XLT_INVAL_TAG_TYPE;
    }
    }
    dm_smlLibFree(_tagstr);
    dm_smlLibFree(_tagnsattr);
    return _err;
}

/**
 * Gets the next token from the scanner.
 * Checks if the current tag is an end tag and if so, whether the last
 * open start tag has the same tag id as the current end tag. An open start
 * tag is one which matching end tag has not been seen yet.
 * If the current tag is a start tag its tag ID will be pushed onto the
 * tag stack.
 * If the current tag is an empty tag or not a tag at all nothing will be
 * done.
 */
static short dm_nextToken(const char * file, int line, XltDecoderPtr_t pDecoder) {
    XltUtilStackPtr_t pTagStack;
    XltDecTokenPtr_t pToken;
    short rc;

    if ((rc = pDecoder->scanner->nextTok(file, line, pDecoder->scanner))
            != SML_ERR_OK)
        return rc;

    pToken = pDecoder->scanner->curtok;
    pTagStack = pDecoder->tagstack;

    if (IS_START(pToken)) {
        if (pTagStack->push(pTagStack, pToken->tagid))
            return SML_ERR_UNSPECIFIC;
    } else if (IS_END(pToken)) {
        XltTagID_t lastopen;
        if (pTagStack->pop(pTagStack, &lastopen))
            return SML_ERR_UNSPECIFIC;
        if (pToken->tagid != lastopen)
            return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }
    return SML_ERR_OK;
}

short dm_discardToken(XltDecoderPtr_t pDecoder) {
    short rc;
    XltTagID_t tmp;
    if ((rc = pDecoder->scanner->pushTok(pDecoder->scanner)) != SML_ERR_OK)
        return rc;
    if ((rc = pDecoder->tagstack->pop(pDecoder->tagstack, &tmp)) != SML_ERR_OK)
        return rc;
    return SML_ERR_OK;
}

short dm_buildPCData(const char * file, int line, XltDecoderPtr_t pDecoder,
        void* *ppPCData) {
    XltDecScannerPtr_t pScanner;
    SmlPcdataPtr_t pPCData = 0;
    SmlPcdataExtension_t ext;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppPCData != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if (IS_EMPTY(pScanner->curtok)) {
        syncml_codec_message("MMIDM dm_buildPCData --  1 file: %s , line: %d", __FILE__, __LINE__);
        if ((pPCData = (SmlPcdataPtr_t) dm_smlLibMalloc(file, line,
                sizeof(SmlPcdata_t))) == NULL)
            return SML_ERR_NOT_ENOUGH_SPACE;

        dm_smlLibMemset(pPCData, 0, sizeof(SmlPcdata_t));

        *ppPCData = pPCData;
        return SML_ERR_OK;
    }

    pPCData = NULL;

    if (((rc = dm_nextToken(file, line, pDecoder)) != SML_ERR_OK)) {
        syncml_codec_message("MMIDM dm_buildPCData --  2 file: %s , line: %d", __FILE__, __LINE__);
        if (rc == SML_ERR_XLT_INVAL_SYNCML_DOC) { /* leaks if dtd failed */
            pPCData = pScanner->curtok->pcdata;
            *ppPCData = pPCData;
        }

        return rc;
    }

    if (IS_CONTENT(pScanner->curtok)) {
        syncml_codec_message("MMIDM dm_buildPCData --  3 file: %s , line: %d", __FILE__, __LINE__);
        /* PCData element has a regular string or opaque content */
        while (pScanner->curtok->type == TOK_CONT) {
            if (pPCData == NULL) {
                syncml_codec_message("MMIDM dm_buildPCData --  4 file: %s , line: %d", __FILE__, __LINE__);
                pPCData = pScanner->curtok->pcdata; //////////////////////////////////////////////////////////////////////
            } else {
                syncml_codec_message("MMIDM dm_buildPCData --  5 file: %s , line: %d", __FILE__, __LINE__);
                if (pScanner->curtok->pcdata != NULL) {
                    pPCData = dm_concatPCData(pPCData,
                            pScanner->curtok->pcdata);
                    dm_smlLibFree((pScanner->curtok->pcdata->content));
                }
                dm_smlLibFree((pScanner->curtok->pcdata));
                pScanner->curtok->pcdata = NULL;

                if (pPCData == NULL)
                    return SML_ERR_XLT_INVAL_PCDATA;
            }

            syncml_codec_message("MMIDM dm_buildPCData --  6 file: %s , line: %d", __FILE__, __LINE__);
            if (((rc = dm_nextToken(file, line, pDecoder)) != SML_ERR_OK)) {
                syncml_codec_message("MMIDM dm_buildPCData --  7 file: %s , line: %d", __FILE__, __LINE__);
                *ppPCData = pPCData;
                return rc;
            }
        }
    } else if (IS_START_OR_EMPTY(pScanner->curtok)) {
        syncml_codec_message("MMIDM dm_buildPCData --  8 file: %s , line: %d", __FILE__, __LINE__);
        /* PCData element contains an XML dokument that is handled by an
         extension mechanism  */
        ext = pScanner->curtok->ext;
        if ((rc = dm_discardToken(pDecoder)) != SML_ERR_OK) {
            syncml_codec_message("MMIDM dm_buildPCData --  9 file: %s , line: %d", __FILE__, __LINE__);
            return rc;
        }
        if ((pPCData = (SmlPcdataPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
                sizeof(SmlPcdata_t))) == NULL)
            return SML_ERR_NOT_ENOUGH_SPACE;
        dm_smlLibMemset(pPCData, 0, sizeof(SmlPcdata_t));
        pPCData->contentType = SML_PCDATA_EXTENSION;
        pPCData->extension = ext;
        switch (ext) {
#ifdef __USE_METINF__
        case SML_EXT_METINF:

            syncml_codec_message("MMIDM dm_buildPCData --  10 file: %s , line: %d", __FILE__, __LINE__);
            if ((rc = dm_buildMetInfMetInfCmd(pDecoder,
                    (void*) &pPCData->content)) != SML_ERR_OK) {
                dm_smlLibFree(pPCData);
                return rc;
            }
            break;
#endif
#ifdef __USE_DEVINF__
        case SML_EXT_DEVINF:

            syncml_codec_message("MMIDM dm_buildPCData --  11 file: %s , line: %d", __FILE__, __LINE__);
            if ((rc = dm_buildDevInfDevInfCmd(pDecoder,
                    (void*) &pPCData->content)) != SML_ERR_OK) {

                dm_smlLibFree(pPCData);
                return rc;
            }

            syncml_codec_message("MMIDM dm_buildPCData --  12 file: %s , line: %d", __FILE__, __LINE__);
            /* the scanner must point to the closing PCDATA tag */
            if (((rc = dm_nextToken(file, line, pDecoder)) != SML_ERR_OK)) {
                syncml_codec_message("MMIDM dm_buildPCData --  13 file: %s , line: %d", __FILE__, __LINE__);
                dm_smlLibFree(pPCData);
                return rc;
            }
            break;
#endif
        default:
            syncml_codec_message("MMIDM dm_buildPCData --  14 file: %s , line: %d", __FILE__, __LINE__);
            dm_smlFreePcdata(__FILE__, __LINE__, pPCData);
            return SML_ERR_XLT_INVAL_EXT;
        }

    } else if (IS_END(pScanner->curtok)) {
        syncml_codec_message("MMIDM dm_buildPCData --  15 file: %s , line: %d", __FILE__, __LINE__);
        /* PCData element is empty */
    } else {
        syncml_codec_message("MMIDM dm_buildPCData --  16 file: %s , line: %d", __FILE__, __LINE__);
        return SML_ERR_XLT_INVAL_PCDATA;
    }

    if (pScanner->curtok->type != TOK_TAG_END) {
        syncml_codec_message("MMIDM dm_buildPCData --  17 file: %s , line: %d", __FILE__, __LINE__);
        return SML_ERR_XLT_INVAL_PCDATA;
    }

    if (pPCData == NULL) {
        syncml_codec_message("MMIDM dm_buildPCData --  18 file: %s , line: %d", __FILE__, __LINE__);
        if ((pPCData = (SmlPcdataPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
                sizeof(SmlPcdata_t))) == NULL)
            return SML_ERR_NOT_ENOUGH_SPACE;
        dm_smlLibMemset(pPCData, 0, sizeof(SmlPcdata_t));
    }

    *ppPCData = pPCData;

    return SML_ERR_OK;
}

short dm_buildPCDataList(XltDecoderPtr_t pDecoder, void* *ppPCData) {
    SmlPcdataListPtr_t pPCDataList = NULL, pPrev = NULL;

    pPCDataList = (SmlPcdataListPtr_t) *ppPCData;

    /* advance to the end of the list, and create ther an empty list element */
    while (pPCDataList != NULL ) {
        pPrev = pPCDataList;
        pPCDataList = pPrev->next;
    }
    if ((pPCDataList = (SmlPcdataListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlPcdataList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pPCDataList, 0, sizeof(SmlPcdataList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
        pPrev->next = pPCDataList;
    else
        /* nope we created a new list */
        *ppPCData = pPCDataList;
    pPCDataList->data = NULL;
    /* at this point pPCDataList should point to an valid list element */
    return dm_buildPCData(__FILE__, __LINE__, pDecoder,
            (void*) &pPCDataList->data);
}

short dm_buildTargetOrSource(XltDecoderPtr_t pDecoder, void* *ppTarget) {
    XltDecScannerPtr_t pScanner;
    SmlTargetPtr_t pTarget;
    long locuri = 0;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppTarget != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pTarget = (SmlTargetPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlTarget_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pTarget, 0, sizeof(SmlTarget_t));

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pTarget);
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pTarget);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_LOCURI:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pTarget->locURI);
            locuri++;
            break;
        case TN_LOCNAME:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pTarget->locName);
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeSourceTargetPtr(pTarget);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeSourceTargetPtr(pTarget);
            return rc;
        }
    }

    if (locuri == 0) {
        dm_smlFreeSourceTargetPtr(pTarget);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppTarget = pTarget;

    return SML_ERR_OK;
}

short dm_buildCred(XltDecoderPtr_t pDecoder, void* *ppCred) {
    XltDecScannerPtr_t pScanner;
    SmlCredPtr_t pCred;
    short rc;
    long data = 0;

    pScanner = pDecoder->scanner;

    if (*ppCred != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pCred = (SmlCredPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlCred_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pCred, 0, sizeof(SmlCred_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppCred = pCred;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pCred);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_DATA:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pCred->data);
            data++;
            break;
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pCred->meta);
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeCredPtr(pCred);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeCredPtr(pCred);
            return rc;
        }
    }

    if (data == 0) {
        dm_smlFreeCredPtr(pCred);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppCred = pCred;

    return SML_ERR_OK;
}

short dm_buildItem(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlItemPtr_t pItem;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pItem = (SmlItemPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlItem_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pItem, 0, sizeof(SmlItem_t));

    /* Item might be empty */
    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pItem;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pItem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pItem->meta);
            break;
        case TN_DATA:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pItem->data);
#ifdef __USE_EXTENSIONS__
#ifdef __SML_WBXML__
            if (pItem->data && pItem->data->contentType == SML_PCDATA_OPAQUE)
                dm_subdtdDecodeWbxml(pDecoder, (SmlPcdataPtr_t*) &pItem->data);
#endif
#endif
            break;
            /* child tags */
        case TN_TARGET:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pItem->target);
            break;
        case TN_SOURCE:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pItem->source);
            break;

            /* flags */
        case TN_MOREDATA:
            pItem->flags |= SmlMoreData_f;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeItemPtr(pItem);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeItemPtr(pItem);
            return rc;
        }
    }

    *ppElem = pItem;

    return SML_ERR_OK;
}

short dm_buildSyncHdr(XltDecoderPtr_t pDecoder, void* *ppSyncHdr) {
    XltDecScannerPtr_t pScanner;
    SmlSyncHdrPtr_t pSyncHdr;
    short rc;
    long sessionid = 0, msgid = 0, source = 0, target = 0, version = 0, proto =
            0;

    /* shortcut to the scanner object */
    pScanner = pDecoder->scanner;

    /* if ppSyncHdr is not NULL we've already
     found a SyncHdr before! */
    if (*ppSyncHdr != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    /* initialize new SmlSyncHdr */
    if ((pSyncHdr = (SmlSyncHdrPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlSyncHdr_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pSyncHdr, 0, sizeof(SmlSyncHdr_t));

    /* initialize the element type field */
    pSyncHdr->elementType = SML_PE_HEADER;

    /* empty SmlSyncHdr is possible */
    if (IS_EMPTY(pScanner->curtok)) {
        *ppSyncHdr = pSyncHdr;
        return SML_ERR_OK;
    }

    /* get next Token */
    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pSyncHdr);
        return rc;
    }

    /* parse child elements until we find a matching end tag */
    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_VERSION:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSyncHdr->version);
            version++;
            break;
        case TN_PROTO:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSyncHdr->proto);
            proto++;
            break;
        case TN_SESSIONID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSyncHdr->sessionID);
            sessionid++;
            break;
        case TN_MSGID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSyncHdr->msgID);
            msgid++;
            break;
        case TN_RESPURI:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSyncHdr->respURI);
            break;

            /* child tags */
        case TN_TARGET:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pSyncHdr->target);
            target++;
            break;
        case TN_SOURCE:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pSyncHdr->source);
            source++;
            break;
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pSyncHdr->cred);
            break;
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSyncHdr->meta);
            break;

            /* flags (empty tags) */
        case TN_NORESP:
            pSyncHdr->flags |= SmlNoResp_f;
            syncml_task_message("MMIDM  ,dm_buildSyncHdr ,for TN_NORESP  parse process!");
            rc = dm_nextToken(__FILE__, __LINE__, pDecoder);
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }

        /* decoding of child element went ok? */
        if (rc != SML_ERR_OK) {
            dm_smlFreeSyncHdr(pSyncHdr);

            return rc;
        }

        /* get next token */
        if ((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK) {
            dm_smlFreeSyncHdr(pSyncHdr);
            return rc;
        }
    }

    if ((sessionid == 0) || (msgid == 0) || (target == 0) || (source == 0)
            || (version == 0) || (proto == 0)) {
        dm_smlFreeSyncHdr(pSyncHdr);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppSyncHdr = pSyncHdr;

    return SML_ERR_OK;
}

short dm_buildGenericCmd(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlGenericCmdPtr_t pGenCmd;
    short rc;
    long items = 0, cmdid = 0;

    pScanner = pDecoder->scanner;
    syncml_codec_message("MMIDM dm_buildGenericCmd --  , file: %s , line: %d", __FILE__, __LINE__);

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    /* initialize a new GenericCmd */
    if ((pGenCmd = (SmlGenericCmdPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlGenericCmd_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pGenCmd, 0, sizeof(SmlGenericCmd_t));

    /* initialize the element type field */
    pGenCmd->elementType = SML_PE_GENERIC;

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pGenCmd);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pGenCmd);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pGenCmd->cmdID);
            cmdid++;
            break;
        case TN_META:
            syncml_codec_message("MMIDM dm_buildGenericCmd --  TN_META, file: %s , line: %d", __FILE__, __LINE__);

            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pGenCmd->meta);
            break;

            /* child tags */
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pGenCmd->cred);
            break;

            /* flags (empty tags) */
        case TN_NORESP:
            pGenCmd->flags |= SmlNoResp_f;
            syncml_task_message("MMIDM  ,dm_buildGenericCmd, for TN_NORESP  parse process!");
            rc = dm_nextToken(__FILE__, __LINE__, pDecoder);
            break;
        case TN_ARCHIVE:
            pGenCmd->flags |= SmlArchive_f;
            break;
        case TN_SFTDEL:
            pGenCmd->flags |= SmlSftDel_f;
            break;

            /* Lists */
        case TN_ITEM:
            rc = dm_appendItemList(pDecoder, &pGenCmd->itemList);
            items++;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeGeneric(pGenCmd);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeGeneric(pGenCmd);
            return rc;
        }
    }

    if ((items == 0) || (cmdid == 0)) {
        dm_smlFreeGeneric(pGenCmd);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppElem = pGenCmd;

    return SML_ERR_OK;
}

short dm_buildAlert(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlAlertPtr_t pAlert;
    short rc;
    long cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pAlert = (SmlAlertPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlAlert_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pAlert, 0, sizeof(SmlAlert_t));

    /* initialize the element type field */
    pAlert->elementType = SML_PE_ALERT;

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pAlert);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pAlert);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pAlert->cmdID);
            cmdid++;
            break;
        case TN_DATA:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pAlert->data);
            break;

            /* child tags */
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pAlert->cred);
            break;

            /* flags (empty tags) */
        case TN_NORESP:
            pAlert->flags |= SmlNoResp_f;
            syncml_task_message("MMIDM   ,dm_buildAlert,for TN_NORESP  parse process!");
            rc = dm_nextToken(__FILE__, __LINE__, pDecoder);
            break;

            /* Lists */
        case TN_ITEM:
            rc = dm_appendItemList(pDecoder, &pAlert->itemList);
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeAlert(pAlert);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeAlert(pAlert);
            return rc;
        }
    }

    if (cmdid == 0) {
        dm_smlFreeAlert(pAlert);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppElem = pAlert;

    return SML_ERR_OK;
}

short dm_buildSync(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlSyncPtr_t pSync;
    short rc;
    long cmdid = 0;

    /* stop decoding the Sync when we find a SyncML command */
    unsigned char break_sync = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    /* initialize a new Sync */
    if ((pSync = (SmlSyncPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlSync_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pSync, 0, sizeof(SmlSync_t));

    /* initialize the element type field */
    pSync->elementType = SML_PE_SYNC_START;

    if (IS_EMPTY(pScanner->curtok)) {

        dm_smlLibFree(pSync);
        return SML_ERR_OK;
    }

    /* get next token */
    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pSync);
        return rc;
    }

    /* parse child elements until we find a matching end tag
     or until we find a TN_ADD, TN_ATOMIC, etc. start tag */
    while ((pScanner->curtok->type != TOK_TAG_END) && !break_sync) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSync->cmdID);
            cmdid++;
            break;
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSync->meta);
            break;
        case TN_NUMBEROFCHANGES:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pSync->noc);
            break;

            /* child tags */
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pSync->cred);
            break;
        case TN_TARGET:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pSync->target);
            break;
        case TN_SOURCE:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pSync->source);
            break;

            /* flags */
        case TN_NORESP:
            pSync->flags |= SmlNoResp_f;
            syncml_task_message("MMIDM   ,dm_buildSync ,for TN_NORESP  parse process!");
            rc = dm_nextToken(__FILE__, __LINE__, pDecoder);
            break;

            /* quit if we find an Add, Atomic, etc.
             element */
        case TN_ADD:
        case TN_ATOMIC:
        case TN_COPY:
        case TN_DELETE:
        case TN_SEQUENCE:
        case TN_REPLACE:
            break_sync = 1;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeSync(pSync);
            return rc;
        }
        if (!break_sync) {
            /* get next token and continue as usual */
            if ((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK) {
                dm_smlFreeSync(pSync);
                return rc;
            }
        } else {
            /* we've found a SyncML command - we need to go
             back one token and correct the tagstack */
            if ((rc = dm_discardToken(pDecoder)) != SML_ERR_OK) {
                dm_smlFreeSync(pSync);
                return rc;
            }
        }
    }

    if (!break_sync) {
        if ((pScanner->curtok->tagid) != TN_SYNC) {
            dm_smlFreeSync(pSync);
            return SML_ERR_XLT_INVAL_SYNCML_DOC;
        } else {
            if (pDecoder->tagstack->push(pDecoder->tagstack,
                    pScanner->curtok->tagid)) {
                dm_smlFreeSync(pSync);
                return SML_ERR_UNSPECIFIC;
            }
            if ((rc = pDecoder->scanner->pushTok(pDecoder->scanner))
                    != SML_ERR_OK) {
                dm_smlFreeSync(pSync);
                return rc;
            }
        }
    }

    *ppElem = pSync;

    return SML_ERR_OK;
}

short dm_buildStatus(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlStatusPtr_t pStatus;
    short rc;
    long cmd = 0, data = 0, cmdid = 0;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    pScanner = pDecoder->scanner;

    if ((pStatus = (SmlStatusPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlStatus_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pStatus, 0, sizeof(SmlStatus_t));

    /* initialize the element type field */
    pStatus->elementType = SML_PE_STATUS;

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pStatus);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pStatus);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCData elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pStatus->cmdID);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_CMDID");
                pStatus->cmdID = NULL;
            }
            cmdid++;
            break;
        case TN_MSGREF:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pStatus->msgRef);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_MSGREF");
                pStatus->msgRef = NULL;
            }
            break;
        case TN_CMDREF:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pStatus->cmdRef);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_CMDREF");
                pStatus->cmdRef = NULL;
            }
            break;
        case TN_CMD:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pStatus->cmd);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_CMD");
                pStatus->cmd = NULL;
            }
            cmd++;
            break;
        case TN_DATA:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pStatus->data);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_DATA");
                pStatus->data = NULL;
            }
            data++;
            break;
        case TN_CHAL:
            rc = dm_buildChal(pDecoder, (void*) &pStatus->chal);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_CHAL");
                pStatus->chal = NULL;
            }
            break;
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pStatus->cred);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_CRED");
                pStatus->cred = NULL;
            }
            break;

            /* Lists */
        case TN_ITEM:
            rc = dm_appendItemList(pDecoder, (void*) &pStatus->itemList);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_ITEM");
                pStatus->itemList = NULL;
            }
            break;
        case TN_TARGETREF:
            rc = dm_appendTargetRefList(pDecoder,
                    (void*) &pStatus->targetRefList);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_TARGETREF");
                pStatus->targetRefList = NULL;
            }
            break;
        case TN_SOURCEREF:
            rc = dm_appendSourceRefList(pDecoder,
                    (void*) &pStatus->sourceRefList);
            if (rc != SML_ERR_OK) {
                syncml_task_message("MMIDM  ----------------------------------------TN_SOURCEREF");
                pStatus->sourceRefList = NULL;
            }
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            syncml_task_message("MMIDM-----------------------------------------dm_smlFreeStatus  11111111 rc = %4x", rc);
            pStatus = NULL;
            dm_smlFreeStatus(pStatus);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            syncml_task_message("MMIDM-----------------------------------------dm_smlFreeStatus  22222222 rc = %4x", rc);
            pStatus = NULL;
            dm_smlFreeStatus(pStatus);
            return rc;
        }
    }

    if ((cmd == 0) || (data == 0) || (cmdid == 0)) {
        syncml_task_message("MMIDM-----------------------------------------dm_smlFreeStatus  3333333 rc = %4x", rc);
        pStatus = NULL;
        dm_smlFreeStatus(pStatus);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppElem = pStatus;

    return SML_ERR_OK;
}

short dm_buildChal(XltDecoderPtr_t pDecoder, void* *ppChal) {
    XltDecScannerPtr_t pScanner;
    SmlChalPtr_t pChal;
    long meta = 0;
    short rc;

    pScanner = pDecoder->scanner;

    if (*ppChal != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pChal = (SmlChalPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlChal_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pChal, 0, sizeof(SmlChal_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppChal = pChal;
        return SML_ERR_OK;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pChal);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pChal->meta);
            meta++;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeChalPtr(pChal);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeChalPtr(pChal);
            return rc;
        }
    }

    if (meta == 0) {
        dm_smlFreeChalPtr(pChal);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppChal = pChal;

    return SML_ERR_OK;
}

short dm_buildPutOrGet(XltDecoderPtr_t pDecoder, void**ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlGetPtr_t pGet;
    short rc;
    long items = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pGet = (SmlGetPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlGet_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pGet, 0, sizeof(SmlGet_t));

    /* initialize the element type field */
    pGet->elementType = SML_PE_PUT_GET;

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pGet);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pGet);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pGet->cmdID);
            cmdid++;
            break;
        case TN_LANG:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pGet->lang);
            break;
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pGet->meta);
            break;

            /* child tags */
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pGet->cred);
            break;

            /* flags */
        case TN_NORESP:
            pGet->flags |= SmlNoResp_f;
            syncml_task_message("MMIDM   ,dm_buildPutOrGet,for TN_NORESP  parse process!");
            rc = dm_nextToken(__FILE__, __LINE__, pDecoder);
            break;

            /* Lists */

        case TN_ITEM:
            rc = dm_appendItemList(pDecoder, &pGet->itemList);
            items++;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeGetPut(pGet);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeGetPut(pGet);
            return rc;
        }
    }

    if ((items == 0) || (cmdid == 0)) {
        dm_smlFreeGetPut(pGet);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppElem = pGet;

    return SML_ERR_OK;
}

#if (defined ATOMIC_RECEIVE || defined SEQUENCE_RECEIVE)
short dm_buildAtomOrSeq(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlAtomicPtr_t pAoS; /* SmlAtomicPtr_t and SequencePtr_t are pointer
     to the same structure! */
    short rc;
    unsigned char break_aos = 0; /* stop decoding the Atomic when we find a
     SyncML command */
    long cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pAoS = (SmlAtomicPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlAtomic_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pAoS, 0, sizeof(SmlAtomic_t));

    /* initialize the element type field */
    pAoS->elementType = SML_PE_CMD_GROUP;

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pAoS);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    /* get next token */
    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pAoS);
        return rc;
    }

    /* parse child elements until we find a matching end tag
     or until we find a TN_ADD, TN_ATOMIC, etc. start tag */
    while ((pScanner->curtok->type != TOK_TAG_END) && !break_aos) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pAoS->cmdID);
            cmdid++;
            break;
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pAoS->meta);
            break;

            /* flags */
        case TN_NORESP:
            pAoS->flags |= SmlNoResp_f;
            break;

            /* quit if we find an Add, Atomic, etc.
             element */
        case TN_ADD:
        case TN_REPLACE:
        case TN_DELETE:
        case TN_COPY:
        case TN_ATOMIC:
        case TN_MAP:
        case TN_SYNC:
        case TN_GET:
        case TN_ALERT:
        case TN_EXEC:
            break_aos = 1;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeAtomic(pAoS);
            return rc;
        }
        if (!break_aos) {
            if ((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK) {
                dm_smlFreeAtomic(pAoS);
                return rc;
            }
        } else {
            /* we've found a SyncML command - we need to go
             back one token and correct the tagstack */
            if ((rc = dm_discardToken(pDecoder)) != SML_ERR_OK) {
                dm_smlFreeAtomic(pAoS);
                return rc;
            }
        }
    }

    if (!break_aos) {
        /* Atomic/Sequence must contain at least one SyncML command */
        dm_smlFreeAtomic(pAoS);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (cmdid == 0) {
        dm_smlFreeAtomic(pAoS);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppElem = pAoS;

    return SML_ERR_OK;
}
#endif

#ifdef EXEC_RECEIVE
short dm_buildExec(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlExecPtr_t pExec;
    short rc;
    long items = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pExec = (SmlExecPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlExec_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pExec, 0, sizeof(SmlExec_t));

    /* initialize the element type field */
    pExec->elementType = SML_PE_EXEC;

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pExec);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pExec);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCData */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pExec->cmdID);
            cmdid++;
            break;

        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pExec->meta);
            break;

            /* child tags */
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pExec->cred);
            break;

        case TN_ITEM:
            rc = dm_buildItem(pDecoder, (void*) &pExec->item);
            items++;
            break;

            /* flags */
        case TN_NORESP:
            pExec->flags |= SmlNoResp_f;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeExec(pExec);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeExec(pExec);
            return rc;
        }
    }

    if ((items == 0) || (cmdid == 0)) {
        dm_smlFreeExec(pExec);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppElem = pExec;

    return SML_ERR_OK;
}
#endif

#ifdef MAP_RECEIVE
short dm_buildMap(XltDecoderPtr_t pDecoder, void* *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlMapPtr_t pMap;
    short rc;
    long target = 0, source = 0, cmdid = 0;
#ifdef MAPITEM_RECEIVE
    long mapitems = 0;
#endif

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pMap = (SmlMapPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlMap_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pMap, 0, sizeof(SmlMap_t));

    /* initialize the element type field */
    pMap->elementType = SML_PE_MAP;

    /* Source is required */
    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pMap);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pMap);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMap->cmdID);
            cmdid++;
            break;
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pMap->meta);
            break;

            /* child tags */
        case TN_CRED:
            rc = dm_buildCred(pDecoder, (void*) &pMap->cred);
            break;
        case TN_SOURCE:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pMap->source);
            source++;
            break;
        case TN_TARGET:
            rc = dm_buildTargetOrSource(pDecoder, (void*) &pMap->target);
            target++;
            break;
#ifdef MAPITEM_RECEIVE
            /* Lists */
            case TN_MAPITEM:
            rc = appendMapItemList(pDecoder, &pMap->mapItemList);
            mapitems++;
            break;
#endif
        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeMap(pMap);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeMap(pMap);
            return rc;
        }
    }

    // if ((source == 0) || (mapitems == 0) || (target == 0) || (cmdid == 0))
    {
        dm_smlFreeMap(pMap);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppElem = pMap;/*lint !e527*/

    return SML_ERR_OK;
}
#endif

#ifdef RESULT_RECEIVE
short dm_buildResults(XltDecoderPtr_t pDecoder, void** ppResults) {
    XltDecScannerPtr_t pScanner;
    SmlResultsPtr_t pResults;
    short rc;
    long cmdref = 0, items = 0, cmdid = 0;

    if (*ppResults != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    pScanner = pDecoder->scanner;

    if ((pResults = (SmlResultsPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlResults_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pResults, 0, sizeof(SmlResults_t));

    /* initialize the element type field */
    pResults->elementType = SML_PE_RESULTS;

    if (IS_EMPTY(pScanner->curtok)) {
        dm_smlLibFree(pResults);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
        dm_smlLibFree(pResults);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

        /* PCDATA elements */
        case TN_CMDID:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pResults->cmdID);
            cmdid++;
            break;
        case TN_MSGREF:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pResults->msgRef);
            break;
        case TN_CMDREF:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pResults->cmdRef);
            cmdref++;
            break;
        case TN_META:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pResults->meta);
            break;
        case TN_TARGETREF:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pResults->targetRef);
            break;
        case TN_SOURCEREF:
            rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
                    (void*) &pResults->sourceRef);
            break;

            /* Lists */
        case TN_ITEM:
            rc = dm_appendItemList(pDecoder, &pResults->itemList);
            items++;
            break;

        default:
            rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            dm_smlFreeResults(pResults);
            return rc;
        }
        if (((rc = dm_nextToken(__FILE__, __LINE__, pDecoder)) != SML_ERR_OK)) {
            dm_smlFreeResults(pResults);
            return rc;
        }
    }

    if ((cmdref == 0) || (items == 0) || (cmdid == 0)) {
        dm_smlFreeResults(pResults);
        return SML_ERR_XLT_INVAL_SYNCML_DOC;
    }

    *ppResults = pResults;

    return SML_ERR_OK;
}

#endif

/*************************************************************************/
/* Internal Functions                                                    */
/*************************************************************************/

static short dm_top(const XltUtilStackPtr_t pStack, XltUtilStackItem_t *itemPtr) {
    ArrayStackPtr_t pStackPriv = (ArrayStackPtr_t) pStack;

    if (pStackPriv->topidx == -1)
        return SML_ERR_WRONG_USAGE;

    *itemPtr = pStackPriv->array[pStackPriv->topidx];

    return SML_ERR_OK;
}

static short dm_pop(XltUtilStackPtr_t pStack, XltUtilStackItem_t *itemPtr) {
    ArrayStackPtr_t pStackPriv = (ArrayStackPtr_t) pStack;
    XltUtilStackItem_t item;

    if (pStackPriv->topidx == -1)
        return SML_ERR_WRONG_USAGE;

    item = pStackPriv->array[pStackPriv->topidx];
    pStackPriv->topidx--;

    if ((pStackPriv->topidx >= 0)
            && (pStackPriv->topidx < pStackPriv->size - pStackPriv->chunksize)) {
        unsigned long newsize;
        XltUtilStackItem_t *newarray;

        newsize = pStackPriv->size - pStackPriv->chunksize;
        if ((newarray = (XltUtilStackItem_t*) dm_smlLibRealloc(
                pStackPriv->array, newsize * sizeof(XltUtilStackItem_t)))
                != NULL) {/*lint !e737*/
            pStackPriv->size = newsize;
            pStackPriv->array = newarray;
        } else {
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
    }

    *itemPtr = item;

    return SML_ERR_OK;
}

static short dm_push(XltUtilStackPtr_t pStack, const XltUtilStackItem_t item) {
    ArrayStackPtr_t pStackPriv = (ArrayStackPtr_t) pStack;

    if (pStackPriv->topidx == pStackPriv->size - 1) {
        unsigned long newsize;
        XltUtilStackItem_t *newarray;

        newsize = pStackPriv->size + pStackPriv->chunksize;
        if ((newarray = (XltUtilStackItem_t*) dm_smlLibRealloc(
                pStackPriv->array, newsize * sizeof(XltUtilStackItem_t)))
                != NULL) {/*lint !e737*/
            pStackPriv->size = newsize;
            pStackPriv->array = newarray;
        } else {
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
    }

    pStackPriv->topidx++;
    pStackPriv->array[pStackPriv->topidx] = item;

    return SML_ERR_OK;
}

static short dm_destroy(XltUtilStackPtr_t pStack) {
    ArrayStackPtr_t pStackPriv;

    if (pStack == NULL)
        return SML_ERR_OK;

    pStackPriv = (ArrayStackPtr_t) pStack;

    dm_smlLibFree((pStackPriv->array));
    dm_smlLibFree(pStackPriv);
    return SML_ERR_OK;
}

static short dm_appendItemList(XltDecoderPtr_t pDecoder,
        SmlItemListPtr_t *ppItemList) {
    SmlItemListPtr_t pNewItemList;
    SmlItemListPtr_t pItemList;
    short rc;

    pItemList = *ppItemList;
    if (pItemList != NULL)
        while (pItemList->next != NULL )
            pItemList = pItemList->next;

    if ((pNewItemList = (SmlItemListPtr_t) dm_smlLibMalloc(__FILE__, __LINE__,
            sizeof(SmlItemList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pNewItemList, 0, sizeof(SmlItemList_t));

    if ((rc = dm_buildItem(pDecoder, (void*) &pNewItemList->item)) != SML_ERR_OK) {
        dm_smlLibFree(pNewItemList);
        return rc;
    }

    if (pItemList == NULL)
        *ppItemList = pNewItemList;
    else
        pItemList->next = pNewItemList;

    return SML_ERR_OK;
}

static short dm_appendTargetRefList(XltDecoderPtr_t pDecoder,
        SmlTargetRefListPtr_t *ppTargetRefList) {
    SmlTargetRefListPtr_t pNewTargetRefList;
    SmlTargetRefListPtr_t pTargetRefList;
    short rc;

    pTargetRefList = *ppTargetRefList;
    if (pTargetRefList != NULL)
        while (pTargetRefList->next != NULL )
            pTargetRefList = pTargetRefList->next;

    if ((pNewTargetRefList = (SmlTargetRefListPtr_t) dm_smlLibMalloc(__FILE__,
            __LINE__, sizeof(SmlTargetRefList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pNewTargetRefList, 0, sizeof(SmlTargetRefList_t));

    if ((rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
            (void*) &pNewTargetRefList->targetRef)) != SML_ERR_OK) {
        dm_smlFreePcdata(__FILE__, __LINE__, pNewTargetRefList->targetRef);
        dm_smlLibFree(pNewTargetRefList);
        return rc;
    }

    if (pTargetRefList == NULL)
        *ppTargetRefList = pNewTargetRefList;
    else
        pTargetRefList->next = pNewTargetRefList;

    return SML_ERR_OK;
}

static short dm_appendSourceRefList(XltDecoderPtr_t pDecoder,
        SmlSourceRefListPtr_t *ppSourceRefList) {
    SmlSourceRefListPtr_t pNewSourceRefList;
    SmlSourceRefListPtr_t pSourceRefList;
    short rc;

    pSourceRefList = *ppSourceRefList;
    if (pSourceRefList != NULL)
        while (pSourceRefList->next != NULL )
            pSourceRefList = pSourceRefList->next;

    if ((pNewSourceRefList = (SmlSourceRefListPtr_t) dm_smlLibMalloc(__FILE__,
            __LINE__, sizeof(SmlSourceRefList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    dm_smlLibMemset(pNewSourceRefList, 0, sizeof(SmlSourceRefList_t));

    if ((rc = dm_buildPCData(__FILE__, __LINE__, pDecoder,
            (void*) &pNewSourceRefList->sourceRef)) != SML_ERR_OK) {
        dm_smlFreePcdata(__FILE__, __LINE__, pNewSourceRefList->sourceRef);
        dm_smlLibFree(pNewSourceRefList);
        return rc;
    }

    if (pSourceRefList == NULL)
        *ppSourceRefList = pNewSourceRefList;
    else
        pSourceRefList->next = pNewSourceRefList;

    return SML_ERR_OK;
}

