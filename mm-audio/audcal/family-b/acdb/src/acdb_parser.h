#ifndef __ACDB_PARSER_H__
#define __ACDB_PARSER_H__
/*===========================================================================
    @file   acdb_parser.h

    The interface of the Acdb Parse project.

    This file will handle the parsing of an ACDB file. It will issue callbacks
    when encountering useful client information in the ACDB file.

                    Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_parser.h#6 $ */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_parser.h#6 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2013-06-07  avi     Support Voice Volume boost feature

========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_os_includes.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

#define ACDB_PARSE_COMPRESSED 1
#define ACDB_PARSE_SUCCESS 0
#define ACDB_PARSE_FAILURE -1
#define ACDB_PARSE_CHUNK_NOT_FOUND -2
#define ACDB_PARSE_INVALID_FILE -3

#define ACDB_CHUNKID_DATE		            0x4445494649444f4dULL //  MODIFIED
#define ACDB_CHUNKID_SWNAME					0x20454d414e505753ULL //  SWPNAME
#define ACDB_CHUNKID_SWVERS					0x2053524556505753ULL //  SWPVERS
#define ACDB_CHUNKID_OEMINFO				0x204f464e494d454fULL //  OEMINFO
#define ACDB_CHUNKID_DEVCAT					0x4e49544143564544ULL //  DEVCATIN
#define ACDB_CHUNKID_AUDPROCLUT				0x3054554c49474341ULL //  ACGILUT0
#define ACDB_CHUNKID_AUDPROCDFT				0x5446444349474341ULL //  ACGICDFT
#define ACDB_CHUNKID_AUDPROCDOT				0x544f444349474341ULL //  ACGICDOT
#define ACDB_CHUNKID_DATAPOOL				0x4c4f4f5041544144ULL //  DATAPOOL
#define ACDB_CHUNKID_VOCPROCLUT				0x3054554c49474356ULL //  VCGILUT0
#define ACDB_CHUNKID_VOCPROCDFT				0x5446444349474356ULL //  VCGICDFT
#define ACDB_CHUNKID_VOCPROCDOT				0x544f444349474356ULL //  VCGICDOT
#define ACDB_CHUNKID_AUDPROGAINDEPCLUT		0x3054554c44474341ULL //  ACGDLUT0
#define ACDB_CHUNKID_AUDPROGAINDEPCDFT		0x5446444344474341ULL //  ACGDCDFT
#define ACDB_CHUNKID_AUDPROGAINDEPCDOT		0x544f444344474341ULL //  ACGDCDOT
#define ACDB_CHUNKID_AUDVOLCLUT				0x3054554c4c4f5641ULL //  AVOLLUT0
#define ACDB_CHUNKID_AUDVOLCDFT				0x544644434c4f5641ULL //  AVOLCDFT
#define ACDB_CHUNKID_AUDVOLCDOT				0x544f44434c4f5641ULL //  AVOLCDOT
#define ACDB_CHUNKID_VOCVOLCLUT				0x3054554c44474356ULL //  VCGDLUT0
#define ACDB_CHUNKID_VOCVOLCDFT				0x5446444344474356ULL //  VCGDCDFT
#define ACDB_CHUNKID_VOCVOLCDOT				0x544f444344474356ULL //  VCGDCDOT
#define ACDB_CHUNKID_AFECLUT				0x3054554c20454641ULL //  AFE LUT0
#define ACDB_CHUNKID_AFECDFT				0x5446444320454641ULL //  AFE CDFT
#define ACDB_CHUNKID_AFECDOT				0x544f444320454641ULL //  AFE CDOT
#define ACDB_CHUNKID_AFECMNCLUT				0x3054554c43454641ULL //  AFECLUT0
#define ACDB_CHUNKID_AFECMNCDFT				0x5446444343454641ULL //  AFECCDFT
#define ACDB_CHUNKID_AFECMNCDOT				0x544f444343454641ULL //  AFECCDOT
#define ACDB_CHUNKID_AUDSTREAMCLUT			0x3054554c4d545341ULL //  ASTMLUT0
#define ACDB_CHUNKID_AUDSTREAMCDFT			0x544644434d545341ULL //  ASTMCDFT
#define ACDB_CHUNKID_AUDSTREAMCDOT			0x544f44434d545341ULL //  ASTMCDOT
#define ACDB_CHUNKID_VOCSTREAMCLUT			0x3054554c4d545356ULL //  VSTMLUT0
#define ACDB_CHUNKID_VOCSTREAMCDFT			0x544644434d545356ULL //  VSTMCDFT
#define ACDB_CHUNKID_VOCSTREAMCDOT			0x544f44434d545356ULL //  VSTMCDOT

#define ACDB_CHUNKID_ADIEANCLUT				0x3054554c20434e41ULL //  ANC LUT0
#define ACDB_CHUNKID_ADIELUT				0x203054554c434443ULL //  CDCLUT0
#define ACDB_CHUNKID_DEVPROPLUT				0x54554c504f525044ULL //  DPROPLUT
#define ACDB_CHUNKID_GLOBALPROPLUT			0x54554c504f525047ULL //  GPROPLUT
#define ACDB_CHUNKID_GLOBALLUT				0x2054554c4c424c47ULL //  GLBLLUT

//pervocoder changes Need to revisit
#define ACDB_CHUNKID_VOCPROCDYNLUT				0x3054554c59445056ULL //  VCGILUT0
#define ACDB_CHUNKID_VOCPROCDYNLUTCVD			0x3044564359445056    //  VCGILUT0
#define ACDB_CHUNKID_VOCPROCDYNLUTOFFSET			0x5453464f59445056  //  VCGILUT0
#define ACDB_CHUNKID_VOCPROCSTATLUT				0x3054554c54535056ULL //  VCGILUT0
#define ACDB_CHUNKID_VOCPROCSTATLUTCVD				0x3044564354535056 //  VCGILUT0
#define ACDB_CHUNKID_VOCPROCSTATLUTOFFSET				0x5453464f54535056  //  VCGILUT0
#define ACDB_CHUNKID_VOCPROCDYNDFT				0x5446444359445056ULL //  VCGICDFT
#define ACDB_CHUNKID_VOCPROCSTATDOT				0x544f444354535056ULL //  VCGICDOT
#define ACDB_CHUNKID_VOCPROCSTATDFT				0x5446444354535056ULL //  VCGICDFT
#define ACDB_CHUNKID_VOCPROCDYNDOT				0x544f444359445056ULL //  VCGICDOT
#define ACDB_CHUNKID_VOCSTREAM2CLUT			0x3054554c32545356ULL //  VSTMLUT0
#define ACDB_CHUNKID_VOCSTREAM2CDFT			0x5446444332545356ULL //  VSTMCDFT
#define ACDB_CHUNKID_VOCSTREAM2CDOT			0x544f444332545356ULL //  VSTMCDOT

#define ACDB_CHUNKID_DEVPAIRCFGCLUT				0x3054554c49504456ULL //  VDPILUT0
#define ACDB_CHUNKID_DEVPAIRCFGCDFT				0x5446444349504456ULL //  VDPICDFT
#define ACDB_CHUNKID_DEVPAIRCFGCDOT				0x544f444349504456ULL //  VDPICDOT

#define ACDB_CHUNKID_LSMLUT				    0x3054554c434d534cULL // LSMCLUT0
#define ACDB_CHUNKID_LSMCDFT			    0x54464443434d534cULL // LSMCCDFT
#define ACDB_CHUNKID_LSMCDOT			    0x544f4443434d534cULL // LSMCCDOT
#define ACDB_CHUNKID_CDCFEATUREDATALUT		0x3054554c53464443ULL // CDFSLUT0

#define ACDB_CHUNKID_ADSTLUT				0x3054554c54534441ULL // ADSTCLUT0
#define ACDB_CHUNKID_ADSTCDFT			    0x5446444354534441ULL // ADSTCCDFT
#define ACDB_CHUNKID_ADSTCDOT			    0x544f444354534441ULL // ADSTCCDOT

#define ACDB_CHUNKID_AANCLUT				0x3054554c434e4141ULL // AANCLUT0
#define ACDB_CHUNKID_AANCCDFT			    0x54464443434e4141ULL // AANCCDFT
#define ACDB_CHUNKID_AANCCDOT			    0x544f4443434e4141ULL // AANCCDOT

#define ACDB_CHUNKID_ACSWVERS			    0x5352455657534341ULL // ACSWVERS
#define ACDB_CHUNKID_ADSPVERS			    0x5352455650534441ULL // ADSPVERS

#define ACDB_CHUNKID_VOCVOL2CLUT				0x3054554c32444756ULL // VCD2LUT0
#define ACDB_CHUNKID_VOCVOL2CDFT			    0x5446444332444356ULL // VCD2CDFT
#define ACDB_CHUNKID_VOCVOL2CDOT			    0x544f444332444356ULL // VCD2CDOT

#define ACDB_CHUNKID_VOICEVP3CLUT				0x3054554c33505656ULL // VVP3LUT0
#define ACDB_CHUNKID_VOICEVP3CDFT			    0x5446444333505656ULL // VVP3CDFT
#define ACDB_CHUNKID_VOICEVP3CDOT			    0x544f444333505656ULL // VVP3CDOT

#define ACDB_CHUNKID_AUDIORECVP3CLUT				0x3054554c33505641ULL // AVP3LUT0
#define ACDB_CHUNKID_AUDIORECVP3CDFT			    0x5446444333505641ULL // AVP3CDFT
#define ACDB_CHUNKID_AUDIORECVP3CDOT			    0x544f444333505641ULL // AVP3CDOT

#define ACDB_CHUNKID_AUDIORECECVP3CLUT				0x3054554c33505645ULL // EVP3LUT0
#define ACDB_CHUNKID_AUDIORECECVP3CDFT			    0x5446444333505645ULL // EVP3CDFT
#define ACDB_CHUNKID_AUDIORECECVP3CDOT			    0x544f444333505645ULL // EVP3CDOT

#define ACDB_CHUNKID_MINFOLUT				0x54554c4f464e494dULL // MINFOLUT

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

int32_t AcdbFileGetChunkData(const uint8_t *pFileBuf,const uint32_t nFileBufLen,uint64_t chunkID,uint8_t **pChkBuf,uint32_t *pChkLen);
int32_t IsAcdbFileValid(const uint8_t *pFileBuf,const uint32_t nFileSize);
int32_t IsAcdbFileZipped(const uint8_t *pFileBuf,const uint32_t nFileSize);
int32_t AcdbFileGetVersion(const uint8_t *pFileBuf,const uint32_t nFileSize,uint32_t *pMajVer,uint32_t *pMinVer, uint32_t *pRevVer);
int32_t IsCodecFileType(const uint8_t *pFileBuf,const uint32_t nFileSize);
int32_t IsGlobalFileType(const uint8_t *pFileBuf,const uint32_t nFileSize);
int32_t IsAVFileType(const uint8_t *pFileBuf,const uint32_t nFileSize);
int32_t AcdbFileGetSWVersion(const uint8_t *pFileBuf,const uint32_t nFileSize,uint32_t *pMajVer,uint32_t *pMinVer, uint32_t *pRevVer);

#endif /* __ACDB_PARSER_H__ */

