#ifndef __cencAtoms_H__
#define __cencAtoms_H__
/* =======================================================================
                              cencAtoms.h
DESCRIPTION
  This file consist the class and functions details require for
  parsing of DRM Atoms PSSH,FRMA,SCHM,SCHI,TENC,SINF which are defined in
  "Common Encryption in ISO-BFF ISO/IEC 23001-7(2011-E): MPEG-DASH Encryption".

  Copyright(c) 2012 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/cencatoms.h#2 $
$DateTime: 2012/08/08 02:58:32 $
$Change: 2672368 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "fullatom.h"
#include "atomdefs.h"
#include "isucceedfail.h"
#include "atomutils.h"
#include "filesourcetypes.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
**                          Macro Definitions
** ----------------------------------------------------------------------- */

#define CENC_DRM 0x63656E63

/* =======================================================================
**                        Class Declarations
** ======================================================================= */


/* ======================================================================
CLASS
  CPsshAtom

DESCRIPTION
  Parse the PSSH Atom and update PSSH DataSize,PSSH Offset,KID DataSize,
  KID DataOffset and SystemID.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

========================================================================== */
class CPsshAtom : public FullAtom
{

  public:
    CPsshAtom(OSCL_FILE *fp); //! Constructor
    virtual ~CPsshAtom();
    uint32  GetPsshData(uint8* pBuf, uint32 dwSize, uint32 dwOffset);

    //! Get PSSH box size
    uint32 GetPsshDataSize()
    {
      return m_ulPsshDataSize;
    }

    uint32  m_ulPsshDataBufferSize;    //! marlin specific
    uint64  m_ulPsshDataBufferOffset;  //! marlin specific
    uint32  m_ulPsshDataSize;
    uint64  m_ulPsshDataOffset;
    uint32  m_ulKidDataSize;
    uint64  m_ulKidDataOffset;
    uint8   m_ucSystemID[MAX_SYSTEMID_SIZE];
    uint32  m_ulKidCount;

  private:
    uint8   m_ucPsshVersion;
    uint8 * m_pucPsshData;
    uint8 * m_pucPsshDataStartPtr;
};

/* ======================================================================
CLASS
  CFrmaAtom

DESCRIPTION
  Parse the FRMA Atom and update relevant field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
class CFrmaAtom : public Atom
{

  public:
    CFrmaAtom(uint8 *pBuf); //! Constructor
    virtual ~CFrmaAtom();
    uint32 GetFrmaData(uint8* pBuf, uint32 dwSize, uint32 dwOffset);

    //! Get FRMA box size
    uint32 GetFrmaDataSize()
    {
      return m_ulFrmaDataSize;
    }
    uint32  m_ulDataFormat;

  private:
    uint8 * m_pucFrmaData;
    uint32  m_ulFrmaDataSize;
};
/* ======================================================================
CLASS
  CSchmAtom

DESCRIPTION
  Parse the SCHM Atom and update relevant field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
class CSchmAtom : public FullAtom
{

  public:
    CSchmAtom(uint8 *pBuf); //! Constructor
    virtual ~CSchmAtom();
    uint32 GetSchmData(uint8* pBuf, uint32 dwSize, uint32 dwOffset);
    //! Get SCHM box size
    uint32 GetSchmDataSize()
    {
      return m_ulSchmDataSize;
    }
    uint32  m_ulSchemeType;
    uint32  m_ulSchemeVersion;
    uint8   m_ucSchemeUri[DEFAULT_MAX_URI_SIZE];
    FileSourceDrmType m_drmType;

  private:
    uint8 * m_pucSchmData;
    uint32  m_ulSchmDataSize;
    uint32  m_ulByteConsumed;
};
/* ======================================================================
CLASS
  CSchiAtom

DESCRIPTION
  Parse the SCHI Atom and update relevant field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
class CSchiAtom : public Atom
{

  public:
    CSchiAtom(uint8 *pBuf); //! Constructor
    virtual ~CSchiAtom();
    uint32  GetSchiData(uint8* pBuf, uint32 dwSize, uint32 dwOffset);
    //! Get SCHI box size
    uint32 GetSchiDataSize()
    {
      return m_ulSchiDataSize;
    }
  private:
    uint8 * m_pucSchiData;
    uint32  m_ulSchiDataSize;
};
/* ======================================================================
CLASS
  CTencAtom

DESCRIPTION
  Parse the Tenc Atom and update relevant field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
class CTencAtom : public FullAtom
{

  public:
    CTencAtom(uint8 *pBuf); //! Constructor
    virtual ~CTencAtom();
    uint32 GetTencData(uint8* pBuf, uint32 dwSize, uint32 dwOffset);
    //! Get TENC box size
    uint32 GetTencDataSize()
    {
      return m_ulTencDataSize;
    }
    uint16 m_usIsEncrypted;
    uint8  m_ucIVSize;
    uint8  m_ucKeyID[MAX_KID_SIZE];

  private:
    uint8 * m_pucTencData;
    uint32  m_ulTencDataSize;
    uint32  m_ulByteConsumed;
};
/* ======================================================================
CLASS
  CSinfAtom

DESCRIPTION
  Parse the SINF Atom and update relevant field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
class CSinfAtom : public Atom
{

  public:
    CSinfAtom(OSCL_FILE *fp); //! Constructor
    virtual ~CSinfAtom();

    void Parse();
    uint32 GetSinfData(uint8* pBuf, uint32 dwSize, uint32 dwOffset);
    //! Get SINF box size
    uint32 GetSinfDataSize()
    {
      return m_ulSinfDataSize;
    }

    CFrmaAtom * m_pFrmaAtom; //! FRMA atom
    CSchmAtom * m_pSchmAtom; //! SCHM atom
    CSchiAtom * m_pSchiAtom; //! SCHI atom
    CTencAtom * m_pTencAtom; //! TENC atom

  private:
    uint8 * m_pucSinfData;
    uint32  m_ulSinfDataSize;
};
#endif /* __cencAtoms_H__ */


