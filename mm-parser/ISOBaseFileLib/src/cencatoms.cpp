/* =======================================================================
                              cencAtoms.cpp
DESCRIPTION
  This file consist the class and functions implementation details of
  DRM Atoms PSSH,FRMA,SCHM,SCHI,TENC,SINF which are defined in
  "Common Encryption in ISO-BFF ISO/IEC 23001-7(2011-E): MPEG-DASH Encryption".

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

  Copyright (c) 2012-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/cencatoms.cpp#6 $
$DateTime: 2013/01/07 03:15:37 $
$Change: 3197733 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "cencatoms.h"
#include "atomdefs.h"
#include "videofmt_common.h"
#include "filebase.h"

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */


/* ======================================================================
FUNCTION
  CPsshAtom::CPsshAtom

DESCRIPTION
  PSSH atom constructor.Parse the PSSH Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CPsshAtom::CPsshAtom(OSCL_FILE *fp): FullAtom(fp)
{
  m_ulPsshDataBufferSize = 0;
  m_ulPsshDataBufferOffset = 0;
  m_ulPsshDataSize = 0;
  m_ulPsshDataOffset = 0;
  m_ulKidDataSize =0;
  m_ulKidDataOffset = 0;
  m_pucPsshData = NULL;
  m_ulKidCount = 0;
  m_ucPsshVersion = 0;
  m_pucPsshDataStartPtr = NULL;

  if ( _success )
  {
    if( ATOM_VERSION_ONE == getVersion())
    {
      m_ucPsshVersion = 1;
    }

    //! Update m_ulPsshDataBufferSize with complete PSSH Atom Size.
    m_ulPsshDataBufferSize = Atom::getSize();
    //! Update m_ulPsshDataBufferOffset with PSSH Atom start offset.
    m_ulPsshDataBufferOffset = Atom::getOffsetInFile();

    //! Skip 8 byte atom header and 4 bytes ver/flag to get data size.
    m_ulPsshDataSize =  m_ulPsshDataBufferSize - DEFAULT_ATOM_SIZE
                        - DEFAULT_ATOM_VERSION_SIZE - DEFAULT_ATOM_FLAG_SIZE;

    //! getOffsetInFile return the absolute address of atom,
    //! Skip size,type,ver/flag.
    m_ulPsshDataOffset = m_ulPsshDataBufferOffset + DEFAULT_ATOM_SIZE
                        + DEFAULT_ATOM_VERSION_SIZE + DEFAULT_ATOM_FLAG_SIZE;

    if(m_ulPsshDataSize)
    {
      m_pucPsshData = (uint8*)MM_Malloc(m_ulPsshDataSize);
      if(m_pucPsshData )
      {
        if ( !AtomUtils::readByteData(fp, m_ulPsshDataSize, m_pucPsshData) )
        {
          m_ulPsshDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "CPsshAtom::CPsshAtom Read is failed ");
          return;
        }

        //! Save the start pointer of PSSH Data.
        m_pucPsshDataStartPtr = m_pucPsshData;

        //!copy m_ucSystemID(16 byte).
        memcpy ((uint8 *) &m_ucSystemID,m_pucPsshData,MAX_SYSTEMID_SIZE);

        m_pucPsshData += 16;
        m_ulPsshDataOffset += 16;

        if(DEFAULT_ATOM_VERSION == m_ucPsshVersion)
        {
          //!Copy the PSSH DataSize which is 4 byte long.
          copyByteSwapData((uint8*)&m_ulPsshDataSize,4, m_pucPsshData, 1, 4);
          m_pucPsshData += 4;
          m_ulPsshDataOffset += 4;
        }
        else
        {
          //!Copy the KID Count which is 4 byte long.
          copyByteSwapData ((uint8 *) &m_ulKidCount,4,m_pucPsshData,1,4);
          m_pucPsshData += 4;
          m_ulPsshDataOffset += 4;

          //! KID Data Size would be sizeof(KID) X KID Count.
          m_ulKidDataSize = MAX_KID_SIZE * m_ulKidCount;
          m_ulKidDataOffset = m_ulPsshDataOffset;

          //! PSSH Data Size will be pssh data offset + kid dataSize.
          copyByteSwapData ((uint8 *) &m_ulPsshDataSize,4,
                            m_pucPsshData+m_ulKidDataSize,1,4);
          m_pucPsshData += m_ulKidDataSize;
          m_ulPsshDataOffset += m_ulKidDataSize;
        }
        //! Assign the start pointer to PSSH Data.
        m_pucPsshData = m_pucPsshDataStartPtr;
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "CPsshAtom::CPsshAtom Memory allocation failed.");
        m_ulPsshDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "CPsshAtom::CPsshAtom _success is false before PSSH atom");
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  CPsshAtom::~CPsshAtom

DESCRIPTION
  PSSH atom destructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CPsshAtom::~CPsshAtom()
{
  if(m_pucPsshData)
  {
    MM_Free(m_pucPsshData);
    m_pucPsshData =  NULL;
  }
}
/* ======================================================================
FUNCTION:
  CPsshAtom::getPsshData

DESCRIPTION:
  copies the PSSH data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 CPsshAtom::GetPsshData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(pBuf && m_ulPsshDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, m_ulPsshDataSize-dwOffset);
    memcpy(pBuf, m_pucPsshData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  SinfAtom

DESCRIPTION
  SINF atom constructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CSinfAtom::CSinfAtom(OSCL_FILE *fp): Atom(fp)
{
  m_pucSinfData    = NULL;
  m_ulSinfDataSize = 0;
  m_pFrmaAtom      = NULL;
  m_pSchmAtom      = NULL;
  m_pSchiAtom      = NULL;
  m_pTencAtom      = NULL;

  if ( _success )
  {
    //! skip 8 bytes atom header size to get sinf data size.
    m_ulSinfDataSize = Atom::getSize() - DEFAULT_ATOM_SIZE;

    if(m_ulSinfDataSize)
    {
      m_pucSinfData = (uint8*)MM_Malloc(m_ulSinfDataSize);
      if(m_pucSinfData)
      {
        if ( !AtomUtils::readByteData(fp, m_ulSinfDataSize, m_pucSinfData) )
        {
          m_ulSinfDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "CSinfAtom::CSinfAtom Read is failed ");
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "CSinfAtom::CSinfAtom Memory allocation failed.");
        m_ulSinfDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "CSinfAtom::CSinfAtom _success is false before SINF atom");
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  CSinfAtom::~CSinfAtom

DESCRIPTION
  PSSH atom destructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */

CSinfAtom::~CSinfAtom()
{
  if(m_pFrmaAtom)
  {
    MM_Delete(m_pFrmaAtom);
    m_pFrmaAtom = NULL;
  }
  if(m_pSchmAtom)
  {
    MM_Delete(m_pSchmAtom);
    m_pSchmAtom = NULL;
  }
  if(m_pSchiAtom)
  {
    MM_Delete(m_pSchiAtom);
    m_pSchiAtom = NULL;
  }
  if(m_pTencAtom)
  {
    MM_Delete(m_pTencAtom);
    m_pTencAtom = NULL;
  }

  // De-allocate the SINF Data Buffer
  if(m_pucSinfData)
  {
    MM_Free(m_pucSinfData);
    m_pucSinfData = NULL;
  }
}

/* ======================================================================
FUNCTION
  SinfAtom

DESCRIPTION
  Parse the SINF Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
void CSinfAtom::Parse()
{
  uint8 * pucSinfDataStartPtr = NULL;
  uint32  ulByteConsumed = 0;
  uint32  ulAtomType = 0,ulAtomSize = 0;
  uint32  ulSinfDataOffset = 0;

  //! ulSinfDataOffset keep track of Atom offset.
  ulSinfDataOffset = 0;

  //! m_pucSinfData point to start of frma data.
  pucSinfDataStartPtr = m_pucSinfData;

  //! Check if sufficient buffer is available or not
  while(ulSinfDataOffset + DEFAULT_ATOM_SIZE < m_ulSinfDataSize )
  {
    pucSinfDataStartPtr =  m_pucSinfData + ulSinfDataOffset;
    ulByteConsumed = 0;
    //! Copy the Atom Size from SINF buffer
    copyByteSwapData((uint8*)&ulAtomSize,4, pucSinfDataStartPtr, 1, 4);
    ulByteConsumed +=4;

    //! Copy the Atom Type from SINF buffer.
    copyByteSwapData((uint8*)&ulAtomType,4,
                     pucSinfDataStartPtr+ulByteConsumed,1,4);
    ulByteConsumed +=4;

    if(FRMA_TYPE == ulAtomType)
    {
      //! Pass the SINF buffer to FRMA constructor.
      m_pFrmaAtom= MM_New_Args( CFrmaAtom, (pucSinfDataStartPtr) );

      //! ulSinfDataOffset will point to next atom.
      ulSinfDataOffset += ulAtomSize;
    }
    else if(SCHM_TYPE == ulAtomType)
    {
      //! Pass the SINF buffer to SCHM parse function.
      m_pSchmAtom= MM_New_Args( CSchmAtom, (pucSinfDataStartPtr) );

      //! ulSinfDataOffset will point to next atom.
      ulSinfDataOffset += ulAtomSize;
    }
    else if(SCHI_TYPE == ulAtomType)
    {
      //!Pass the SINF buffer to SCHI parse function.
      m_pSchiAtom= MM_New_Args( CSchiAtom, (pucSinfDataStartPtr) );

      //! ulSinfDataOffset will point to Tenc atom, so add the SCHI header size.
      ulSinfDataOffset += DEFAULT_ATOM_SIZE;
    }
    else if(TENC_TYPE == ulAtomType)
    {
      //! Pass the SINF buffer to SCHI parse function.
      m_pTencAtom = MM_New_Args( CTencAtom, (pucSinfDataStartPtr) );

      //! ulSinfDataOffset will point to next atom.
      ulSinfDataOffset += ulAtomSize;
    }
    //! Unsupported/Unknown atom. Skip this
    else
    {
      //! ulSinfDataOffset will point to next atom.
      ulSinfDataOffset += ulAtomSize;
    }
  } //while(ulSinfDataOffset + DEFAULT_ATOM_SIZE < m_ulSinfDataSize )
}
/* ======================================================================
FUNCTION:
  CSinfAtom::getSinfData

DESCRIPTION:
  copies the Sinf data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 CSinfAtom::GetSinfData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(pBuf && m_ulSinfDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, m_ulSinfDataSize-dwOffset);
    memcpy(pBuf, m_pucSinfData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
CLASS
  FrmaAtom

DESCRIPTION
  FRMA atom constructor.Parse the FRMA Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CFrmaAtom::CFrmaAtom(uint8* pBuf): Atom(pBuf)
{
  m_pucFrmaData    = NULL;
  m_ulFrmaDataSize = 0;
  m_ulDataFormat   = 0;

  if ( _success )
  {
    //! Skip 8 byte atom header to get frma data size.
    m_ulFrmaDataSize = Atom::getSize() - DEFAULT_ATOM_SIZE;
    if(m_ulFrmaDataSize)
    {
      m_pucFrmaData = (uint8*)MM_Malloc(m_ulFrmaDataSize);
      if(m_pucFrmaData)
      {
        if ( !AtomUtils::readByteData(pBuf, m_ulFrmaDataSize, m_pucFrmaData) )
        {
          m_ulFrmaDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "CFrmaAtom::CFrmaAtom Read is failed ");
          return;
        }
        copyByteSwapData ((uint8 *) &m_ulDataFormat,4,m_pucFrmaData,1,4);
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "CFrmaAtom::CFrmaAtom Memory allocation failed.");
        m_ulFrmaDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
    else
    {
      _fileErrorCode = PARSER_ErrorReadFail;
      _success = false;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "CFrmaAtom::CFrmaAtom _success is false before FRMA atom");
      return;
    }
  }
}

/* ======================================================================
FUNCTION
  CFrmaAtom::~CFrmaAtom

DESCRIPTION
  FRMA atom destructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CFrmaAtom::~CFrmaAtom()
{
  if(m_pucFrmaData)
  {
    MM_Free(m_pucFrmaData);
    m_pucFrmaData = NULL;
  }
}
/* ======================================================================
FUNCTION:
  CFrmaAtom::getFrmaData

DESCRIPTION:
  copies the Frma data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 CFrmaAtom::GetFrmaData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(pBuf && m_ulFrmaDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, m_ulFrmaDataSize-dwOffset);
    memcpy(pBuf, m_pucFrmaData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
CLASS
  CSchmAtom

DESCRIPTION
  SCHM atom constructor.Parse the SCHM Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CSchmAtom::CSchmAtom(uint8* pBuf): FullAtom(pBuf)
{
  m_pucSchmData     = NULL;
  m_ulSchmDataSize  = 0;
  m_ulSchemeType    = 0;
  m_ulSchemeVersion = 0;
  m_ulByteConsumed  = 0;
  m_drmType         = FILE_SOURCE_NO_DRM;

  if ( _success )
  {
    //! Skip 8 byte atom header and 1 bytes ver to get data size.
    m_ulSchmDataSize = Atom::getSize() - DEFAULT_ATOM_SIZE
                      - DEFAULT_ATOM_VERSION_SIZE - DEFAULT_ATOM_FLAG_SIZE;

    if(m_ulSchmDataSize)
    {
      m_pucSchmData = (uint8*)MM_Malloc(m_ulSchmDataSize);
      if(m_pucSchmData)
      {
        if ( !AtomUtils::readByteData(pBuf, m_ulSchmDataSize, m_pucSchmData) )
        {
          m_ulSchmDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                       "CSchmAtom::CSchmAtom Read is failed.");
          return;
        }

        //! Copy the scheme type which is 4 byte long.
        copyByteSwapData ((uint8 *) &m_ulSchemeType,4,
                           m_pucSchmData+m_ulByteConsumed,1,4);
        m_ulByteConsumed +=4;

        //! Copy the scheme Version which is 4 byte long.
        copyByteSwapData ((uint8 *) &m_ulSchemeVersion,4,
                           m_pucSchmData + m_ulByteConsumed,1,4);
        m_ulByteConsumed +=4;

        if(getFlags())
        {
          //! copy Scheme URI.
          memcpy ((uint8 *) &m_ucSchemeUri,m_pucSchmData + m_ulByteConsumed,
                   DEFAULT_MAX_URI_SIZE);
        }

        switch(m_ulSchemeType)
        {
        case CENC_DRM:
          m_drmType = FILE_SOURCE_CENC_DRM;
          break;
        default:
          m_drmType = FILE_SOURCE_NO_DRM;
          break;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                     "CSchmAtom::CSchmAtom Memory allocation failed.");
        m_ulSchmDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
    else
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                   "CSchmAtom::CSchmAtom m_ulSchmDataSize is ZERO");
      return;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "CSchmAtom::CSchmAtom _success is false before SCHM atom");
  }
}
/* ======================================================================
FUNCTION
  CSchmAtom::~CSchmAtom

DESCRIPTION
  SCHM atom destructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CSchmAtom::~CSchmAtom()
{
  if(m_pucSchmData)
  {
    MM_Free(m_pucSchmData);
    m_pucSchmData = NULL;
  }
}

  /* ======================================================================
FUNCTION:
  CSchmAtom::getSchmData

DESCRIPTION:
  copies the Schm data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 CSchmAtom::GetSchmData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(pBuf && m_ulSchmDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, m_ulSchmDataSize-dwOffset);
    memcpy(pBuf, m_pucSchmData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
CLASS
  CSchiAtom

DESCRIPTION
  SCHI atom constructor.Parse the SCHI Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CSchiAtom::CSchiAtom(uint8* pBuf): Atom(pBuf)
{
  m_pucSchiData    = NULL;
  m_ulSchiDataSize = 0;

  if ( _success )
  {
    //! Skip 8 byte atom header to get frma data size.
    m_ulSchiDataSize = Atom::getSize() - DEFAULT_ATOM_SIZE;
    if(m_ulSchiDataSize)
    {
      m_pucSchiData = (uint8*)MM_Malloc(m_ulSchiDataSize);
      if(m_pucSchiData)
      {
        if ( !AtomUtils::readByteData(pBuf, m_ulSchiDataSize, m_pucSchiData) )
        {
          m_ulSchiDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "CSchiAtom::CSchiAtom Read failed.");
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
          "CSchiAtom::CSchiAtom Memory allocation failed.");
        m_ulSchiDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
    else
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "CSchiAtom::CSchiAtom m_ulSchiDataSize value is ZERO");
      return;
    }
  }
}
/* ======================================================================
FUNCTION
  CSchiAtom::~CSchiAtom

DESCRIPTION
  SCHI atom destructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CSchiAtom::~CSchiAtom()
{
  if(m_pucSchiData)
  {
    MM_Free(m_pucSchiData);
    m_pucSchiData = NULL;
  }
}
/* ======================================================================
FUNCTION:
  CSchiAtom::getSchiData

DESCRIPTION:
  copies the Schi data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 CSchiAtom::GetSchiData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(pBuf && m_ulSchiDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, m_ulSchiDataSize-dwOffset);
    memcpy(pBuf, m_pucSchiData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
CLASS
  CTencAtom

DESCRIPTION
  Tenc atom constructor.Parse the TENC Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CTencAtom::CTencAtom(uint8* pBuf): FullAtom(pBuf)
{
  m_pucTencData    = NULL;
  m_ulTencDataSize = 0;
  m_ulByteConsumed = 0;
  m_usIsEncrypted  = 0;
  m_ucIVSize       = 0;

  if ( _success )
  {
    //! Skip 8 byte atom header and 4 bytes ver/flag to get data size.
    m_ulTencDataSize = Atom::getSize() - DEFAULT_ATOM_SIZE
      - DEFAULT_ATOM_VERSION_SIZE - DEFAULT_ATOM_FLAG_SIZE;

    if(m_ulTencDataSize)
    {
      m_pucTencData = (uint8*)MM_Malloc(m_ulTencDataSize);
      if(m_pucTencData)
      {
        if ( !AtomUtils::readByteData(pBuf, m_ulTencDataSize, m_pucTencData) )
        {
          m_ulTencDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "CTencAtom::CTencAtom Read failed.");
          return;
        }

        //! Copy the default IsEncrypted.
        copyByteSwapData ((uint8 *) &m_usIsEncrypted, 3,
          m_pucTencData + m_ulByteConsumed ,1,3);
        m_ulByteConsumed +=3;

        //! Copy the default IVSize
        copyByteSwapData ((uint8 *) &m_ucIVSize,1,
          m_pucTencData + m_ulByteConsumed,1,1);
        m_ulByteConsumed +=1;

        //!Copy the default KeyID
        copyByteSwapData ((uint8 *) &m_ucKeyID,MAX_KID_SIZE,
          m_pucTencData + m_ulByteConsumed,0,MAX_KID_SIZE);
        m_ulByteConsumed += MAX_KID_SIZE;
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
          "CTencAtom::CTencAtom Memory allocation failed.");
        m_ulTencDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
    else
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "CTencAtom::CTencAtom m_ulTencDataSize value is ZERO.");
      return;
    }
  }
}
/* ======================================================================
FUNCTION
  CTencAtom::~CTencAtom

DESCRIPTION
  TENC atom destructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CTencAtom::~CTencAtom()
{
  if(m_pucTencData)
  {
    MM_Free(m_pucTencData);
    m_pucTencData = NULL;
  }
}
  /* ======================================================================
FUNCTION:
  CTencAtom::getTencData

DESCRIPTION:
  copies the Tenc data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 CTencAtom::GetTencData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(pBuf && m_ulTencDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, m_ulTencDataSize-dwOffset);
    memcpy(pBuf, m_pucTencData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

