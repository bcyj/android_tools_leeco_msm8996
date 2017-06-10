#ifndef __udtaAtoms_H__
#define __udtaAtoms_H__
/* =======================================================================
                              udtaAtoms.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2008-2014 by QUALCOMM Technologies Inc, Incorporated. All Rights Reserved
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/udtaatoms.h#10 $
$DateTime: 2013/05/14 02:43:43 $
$Change: 3759752 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "fullatom.h"
#include "atomdefs.h"
#include "isucceedfail.h"
#include "atomutils.h"
#include "id3.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

 /* Encoder and Padding delays will be available in same metadata
    section :iTunSMPB". One example of iTunSMPB metadata:
    " 00000000 00000840 000001CA 00000000003F31F6 00000000 00000000
    00000000 00000000 00000000 00000000 00000000 00000000"
    In the above data,
    0x840 indicates Encoder delay and
    0x1CA indicates Padding delay
    0x3F31F6 indicates total number of samples
    Metadata will be stored in following format
    <Space><8bytes ZERO Padding><Space><8bytes Encoder Delay>
    <Space><8bytes Padding delay><Space>
 */
#define ENCODER_DELAY_OFFSET (10) //Skip 2 spaces and one 8byte data field
#define PADDING_DELAY_OFFSET (19) //Skip 3 spaces and two 8byte data fields
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
  ITUNES_IMPLICIT = 0,
  ITUNES_UTF8,
  ITUNES_UTF16,
  ITUNES_JIS, //Special Japanese Characters
  ITUNES_HTML = 6,
  ITUNES_XML,
  ITUNES_UUID,
  ITUNES_ISRC,
  ITUNES_MI3P,
  ITUNES_GIF = 12,
  ITUNES_JPEG,
  ITUNES_PNG,
  ITUNES_URL,
  ITUNES_DURATION,
  ITUNES_DATE_TIME,
  ITUNES_GENRE,
  ITUNES_INTEGER = 21,
  ITUNES_RIAA_PA = 24,
  ITUNES_UPC,
  ITUNES_BMP     = 27,

}ITUNES_DATA_TYPE;

typedef struct _Itunes_meta_data_
{
  FileSourceMetaDataType eMetaType;
  ITUNES_DATA_TYPE       eDataType;
  uint32                 ulLocaleIndicator;
  uint32                 ulMetaDataLen;
  uint8*                 pucMetaData;
} ItunesMetaData;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ======================================================================
CLASS
  UdtaMidiAtom

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaMidiAtom : public FullAtom
{

public:
    UdtaMidiAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaMidiAtom();

    uint32 getUdtaMidiDataSize();
    uint32 getUdtaMidiData(uint8* pBuf, uint32 dwSize, uint32 dwOffset);

private:
    uint32  _midiDataSize;
    uint8 * _midiData;
};

/* ======================================================================
CLASS
  UdtaLinkAtom

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaLinkAtom : public FullAtom
{

public:
    UdtaLinkAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaLinkAtom();

    uint32 getUdtaLinkDataSize();
    uint32 getUdtaLinkData(uint8* pBuf, uint32 dwSize);

private:
    uint32  _linkDataSize;
    uint8 * _linkData;
};

/* ======================================================================
CLASS
  FtypAtom

DESCRIPTION
  File Type atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class FtypAtom : public Atom
{

public:
    FtypAtom(OSCL_FILE *fp); // Constructor
    virtual ~FtypAtom();

    uint32 getFtypDataSize();
    uint32 getFtypData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _ftypDataSize;
    uint8 * _ftypData;
};


/* ======================================================================
CLASS
  DrefAtom

DESCRIPTION
  Data Reference atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class DrefAtom : public FullAtom
{

public:
    DrefAtom(OSCL_FILE *fp); // Constructor
    virtual ~DrefAtom();

    uint32 getDrefDataSize();
    uint32 getDrefData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _drefDataSize;
    uint8 * _drefData;
};
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
/* ======================================================================
CLASS
  CSubsAtom

DESCRIPTION
  Sub Sample atom.

DEPENDENCIES
  None

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  None.

========================================================================== */
class CSubsAtom : public FullAtom
{

public:
  CSubsAtom(OSCL_FILE *fp); // Constructor
  virtual ~CSubsAtom();
  //! Get subs box size
  uint32 GetSubsDataSize()
  {
    return ulSubsDataSize;
  }
  //! Get entry count in table
  uint32 GetEntryCount()
  {
    return ulEntryCount;
  }
  //! Get Sample number having subsamples
  uint32 GetSampleDelta()
  {
    return ulSampleDelta;
  }
  //! Get subsample count in current sample
  uint16 GetSubSampleCount()
  {
    return usSubSampleCount;
  }
  //! Get subs data including version & flag
  uint32 GetSubsData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
  //! Sub sample box size along with version & flag information
  uint32 ulSubsDataSize;
  //! Number of entry count in following table. In case of subtitle
  //! entry_count = 1
  uint32 ulEntryCount;
  //! Sample number of sample having sub sample structure
  uint32 ulSampleDelta;
  //! Number of sub-sample in the current sample
  uint16 usSubSampleCount;
  // Subs box data including version & flag information
  uint8 * pSubsData;
};

/* ======================================================================
CLASS
  UdtaCprtAtom

DESCRIPTION
  Copyright atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaCprtAtom : public FullAtom
{

public:
    UdtaCprtAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaCprtAtom();

    uint32 getUdtaCprtDataSize();
    uint32 getUdtaCprtData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _cprtDataSize;
    uint8 * _cprtData;
};


/* ======================================================================
CLASS
  UdtaAuthAtom

DESCRIPTION
  Author atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaAuthAtom : public FullAtom
{

public:
    UdtaAuthAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaAuthAtom();

    uint32 getUdtaAuthDataSize();
    uint32 getUdtaAuthData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _authDataSize;
    uint8 * _authData;
};


/* ======================================================================
CLASS
  UdtaTitlAtom

DESCRIPTION
  Title atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaTitlAtom : public FullAtom
{

public:
    UdtaTitlAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaTitlAtom();

    uint32 getUdtaTitlDataSize();
    uint32 getUdtaTitlData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _titlDataSize;
    uint8 * _titlData;
};


/* ======================================================================
CLASS
  UdtaAlbumAtom

DESCRIPTION
  Album atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaAlbumAtom : public FullAtom
{

public:
    UdtaAlbumAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaAlbumAtom();

    uint32 getUdtaAlbumDataSize();
    uint32 getUdtaAlbumData(uint8* pBuf, uint32 dwSize, uint32 offset);
    uint8* getUdtaTrackNumStr() {return _TrackNum;};

private:
    uint32  _albumDataSize;
    uint8 * _albumData;
    uint8   _TrackNum[4];
};

/* ======================================================================
CLASS
  UdtaYrrcAtom

DESCRIPTION
  Yrcc atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaYrrcAtom : public FullAtom
{

public:
    UdtaYrrcAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaYrrcAtom();

    uint32 getUdtaYrrcDataSize();
    uint32 getUdtaYrrcData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _yrrcDataSize;
    uint8 * _yrrcData;
};

/* ======================================================================
CLASS
  UdtaDscpAtom

DESCRIPTION
  Description atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaDscpAtom : public FullAtom
{

public:
    UdtaDscpAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaDscpAtom();

    uint32 getUdtaDscpDataSize();
    uint32 getUdtaDscpData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _dscpDataSize;
    uint8 * _dscpData;
};

/* ======================================================================
CLASS
  UdtaRtnAtom

DESCRIPTION
  Rating atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaRtngAtom : public FullAtom
{

public:
    UdtaRtngAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaRtngAtom();

    uint32 getUdtaRtngDataSize();
    uint32 getUdtaRtngData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _rtngDataSize;
    uint8 * _rtngData;
};
/* ======================================================================
CLASS
  UdtaRtngAtom

DESCRIPTION
  Rating atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaGnreAtom : public FullAtom
{

public:
    UdtaGnreAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaGnreAtom();

    uint32 getUdtaGnreDataSize();
    uint32 getUdtaGnreData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _gnreDataSize;
    uint8 * _gnreData;
};
/* ======================================================================
CLASS
  UdtaPerfAtom

DESCRIPTION
  Performance atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaPerfAtom : public FullAtom
{

public:
    UdtaPerfAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaPerfAtom();

    uint32 getUdtaPerfDataSize();
    uint32 getUdtaPerfData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _perfDataSize;
    uint8 * _perfData;
};

/* ======================================================================
CLASS
  UdtaclsfAtom

DESCRIPTION
  Classification atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaClsfAtom : public FullAtom
{

public:
    UdtaClsfAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaClsfAtom();

    uint32 getUdtaClsfDataSize();
    uint32 getUdtaClsfData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _clsfDataSize;
    uint8 * _clsfData;
};

/* ======================================================================
CLASS
  UdtaKywdAtom

DESCRIPTION
  Keyword atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaKywdAtom : public FullAtom
{

public:
    UdtaKywdAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaKywdAtom();

    uint32 getUdtaKywdDataSize();
    uint32 getUdtaKywdData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _kywdDataSize;
    uint8 * _kywdData;
};

/* ======================================================================
CLASS
  UdtaLociAtom

DESCRIPTION
  Location atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaLociAtom : public FullAtom
{

public:
    UdtaLociAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaLociAtom();

    uint32 getUdtaLociDataSize();
    uint32 getUdtaLociData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _lociDataSize;
    uint8 * _lociData;
};

/* ======================================================================
CLASS
  UdtaiLstAtom

DESCRIPTION
  Atom used to store i-tunes related metadata.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaiLstAtom : public Atom
{
public:
    UdtaiLstAtom(uint8 *pBuf); // Constructor
    virtual ~UdtaiLstAtom();

    uint32 getUdtaiLstDataSize(){return m_uliLstDataSize;}
    uint32 getUdtaiLstData(uint8* pBuf, uint32 dwSize, uint32 ulOffset);
    void   Parse();
    ItunesMetaData* ParseFreeAtom(uint8* pBuf, uint32 ulFreeAtomSize);

    //! Declaration of ZArray of Metadata Atoms
    ZArray<ItunesMetaData*> m_aMetaAtomEntryArray;
    uint32 m_ulMetaAtomCount;

private:
    uint32  m_uliLstDataSize;
    uint8 * m_puciLstData;
};

/* ======================================================================
CLASS
  UdtaMetaAtom

DESCRIPTION
  Location atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class UdtaMetaAtom : public FullAtom
{

public:
    UdtaMetaAtom(OSCL_FILE *fp); // Constructor
    virtual ~UdtaMetaAtom();

    uint32 getUdtaMetaDataSize();
    uint32 getUdtaMetaData(uint8* pBuf, uint32 dwSize, uint32 offset);
    void   Parse();

    //! Declaration of ZArray of iTunes related Atoms
    ZArray<UdtaiLstAtom*> m_aiLstAtomEntryArray;
    uint32 m_uliLstAtomCount;
    ZArray<metadata_id3v2_type*> m_aID3AtomArray;
    uint32 m_ulID3AtomCount;
private:
    uint32  _metaDataSize;
    uint8 * _metaData;
};

#endif /* __udtaAtoms_H__ */

