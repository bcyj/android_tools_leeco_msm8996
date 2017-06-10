#ifndef __KDDIUserDataAtom_H__
#define __KDDIUserDataAtom_H__
/* =======================================================================
                              kddiuserdataatom.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2008-11, 2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/kddiuserdataatom.h#7 $
$DateTime: 2011/11/24 00:50:52 $
$Change: 2061573 $


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

#ifdef FEATURE_MP4_CUSTOM_META_DATA
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

/* ======================================================================
CLASS
  KDDIDrmAtom

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
class KDDIDrmAtom : public FullAtom
{

public:
    KDDIDrmAtom(OSCL_FILE *fp); // Constructor
    virtual ~KDDIDrmAtom() {};

    uint32 getCopyProhibitionFlag() { return _copyGaurdAttribute; }
    uint32 getValidityEffectiveDate() { return _limitDate; }
    uint32 getValidityPeriod() { return _limitPeriod; }
    uint32 getNumberofAllowedPlayBacks() { return _limitCount; }

private:
    uint32 _copyGaurdAttribute;
    uint32 _limitDate;
    uint32 _limitPeriod;
    uint32 _limitCount;
};

/* ======================================================================
CLASS
  KDDIContentPropertyAtom

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
class KDDIContentPropertyAtom : public FullAtom
{

public:
    KDDIContentPropertyAtom(OSCL_FILE *fp); // Constructor
    virtual ~KDDIContentPropertyAtom() {};

    FILESOURCE_STRING getContentPropertyTitle() { return _title; }
    FILESOURCE_STRING getContentPropertyCopyRight() { return _copyRight; }
    FILESOURCE_STRING getContentPropertyAuthor() { return _author; }
    FILESOURCE_STRING getContentPropertyMemo() { return _memo; }
    uint32      getAuthorDLLVersion() { return _dllVersion; }

private:
    FILESOURCE_STRING _title;
    FILESOURCE_STRING _copyRight;
    FILESOURCE_STRING _author;
    FILESOURCE_STRING _memo;
    uint32      _dllVersion;
};

/* ======================================================================
CLASS
  KDDIMovieMailAtom

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
class KDDIMovieMailAtom : public FullAtom
{
public:
    KDDIMovieMailAtom(OSCL_FILE *fp); // Constructor
    virtual ~KDDIMovieMailAtom() {};

    uint32 getEditFlags() { return _editFlags; }
    uint8  getRecordingMode() { return _recordingMode; }
    uint32 getRecordingDate() { return _recDate; }

private:
    uint32       _editFlags;
    uint8        _recordingMode;
    uint32       _recDate;
};

/* ======================================================================
CLASS
  KDDIEncoderInformationAtom

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
class KDDIEncoderInformationAtom : public FullAtom
{
public:
    KDDIEncoderInformationAtom(OSCL_FILE *fp); // Constructor
    virtual ~KDDIEncoderInformationAtom();

    uint8*  getDeviceName() const { return _deviceName; }
    uint8*  getModelName() const { return _modelName; }
    uint8*  getEncoderInformation() const { return _encoderInformation; }
    uint8*  getMuxInformation() const { return _muxInformation; }

private:
    uint8     *_deviceName;
    uint8     *_modelName;
    uint8     *_encoderInformation;
    uint8     *_muxInformation;
};

#define KDDI_GPS_IFD_TAG_VERSION          0x00
#define KDDI_GPS_IFD_TAG_LATITUDE_REF     0x01
#define KDDI_GPS_IFD_TAG_LATITUDE         0x02
#define KDDI_GPS_IFD_TAG_LONGITUDE_REF    0x03
#define KDDI_GPS_IFD_TAG_LONGITUDE        0x04
#define KDDI_GPS_IFD_TAG_ALTITUDE_REF     0x05
#define KDDI_GPS_IFD_TAG_ALTITUDE         0x06
#define KDDI_GPS_IFD_TAG_TIMESTAMP        0x07
#define KDDI_GPS_IFD_TAG_GEODETIC_SRV     0x12
#define KDDI_GPS_IFD_TAG_POS_SYSTEM       0x1B
#define KDDI_GPS_IFD_TAG_POS_POINT_NAME   0x1C
#define KDDI_GPS_IFD_TAG_DATESTAMP        0x1D

/* ======================================================================
CLASS
  KDDIGPSIFDEntry

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
class KDDIGPSIFDEntry
{
public:
    KDDIGPSIFDEntry(OSCL_FILE *fp);
    virtual ~KDDIGPSIFDEntry() {};

    uint16 getTagID() { return _tagID; }
    uint16 getTypeValue() { return _typeValue; }
    uint32 gatDataCount() { return _dataCount; }
    uint32 getValue() { return _value; }
    bool   getSuccess() { return  _success; }

private:
    uint16      _tagID;
    uint16      _typeValue;
    uint32      _dataCount;
    uint32      _value;
    bool        _success;
};

/* ======================================================================
CLASS
  KDDIGPSExtensionAtom

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
class KDDIGPSExtensionAtom : public FullAtom
{
public:
    KDDIGPSExtensionAtom(OSCL_FILE *fp); // Constructor
    virtual ~KDDIGPSExtensionAtom() {};

    uint64 getMapScaleInfo() { return _mapScaleInfo; }

private:
    uint64 _mapScaleInfo;
};


/* ======================================================================
CLASS
  KDDIGPSAtom

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
class KDDIGPSAtom : public FullAtom
{
public:
    KDDIGPSAtom(OSCL_FILE *fp); // Constructor
    virtual ~KDDIGPSAtom();

    uint16 getGPSByteOrder()
    {
        return _byteOrder;
    }

    uint32  getVersionID() { return _versionID; }

    uint32  getLatitudeRef() { return _latitudeRef; }
    uint64 *getGPSLatitudeArray() { return _gpsLatitude; }

    uint32  getLongitudeRef() { return _longitudeRef; }
    uint64 *getGPSLongitudeArray(){ return _gpsLongitude; }

    uint32  getAltitudeRef() { return _altitudeRef; }
    uint64 getGPSAltitude()  { return _gpsAltitude; }

    uint64 *getGPSTimeArray(){ return _gpsTime; }

    FILESOURCE_STRING getGPSSurveyData() { return _gpsGeodeticSurveyData; }

    FILESOURCE_STRING getPositoningMethod() { return _positioningMethod; }

    FILESOURCE_STRING getPositioningName() { return _positioningPointName; }

    FILESOURCE_STRING getGPSDate() { return _gpsDate; }

    uint64 getGPSExtensionMapScaleInfo()
    {
      if (_kddiGPSExtensionAtom != NULL)
      {
        return (_kddiGPSExtensionAtom->getMapScaleInfo());
      }
      return 0;
    }

private:

    uint16                   _byteOrder;
    uint16                   _numIFD;
    ZArray<KDDIGPSIFDEntry*> *_gpsIFDEntryVec;

    uint32                   _versionID;
    uint32                   _latitudeRef;
    uint64                   _gpsLatitude[QTV_MAX_GPS_LATITUDE];
    uint32                   _longitudeRef;
    uint64                   _gpsLongitude[QTV_MAX_GPS_LONGITUDE];
    uint32                   _altitudeRef;
    uint64                   _gpsAltitude;
    uint64                   _gpsTime[QTV_MAX_GPS_TIME];
    FILESOURCE_STRING              _gpsGeodeticSurveyData;
    FILESOURCE_STRING              _positioningMethod;
    FILESOURCE_STRING              _positioningPointName;
    FILESOURCE_STRING              _gpsDate;

    uint32                   _nextIFDOffset;
    KDDIGPSExtensionAtom     *_kddiGPSExtensionAtom;

    uint16 getNumIFDEntries()
    {
        return (_numIFD);
    }

    KDDIGPSIFDEntry *getIFDEntryAt(int32 index);
};

/* ======================================================================
CLASS
  KDDITelopAtom

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
class KDDITelopAtom : public FullAtom
{
public:
    KDDITelopAtom(OSCL_FILE *fp); // Constructor
    virtual ~KDDITelopAtom() ;    // Destructor

    uint8* getTelopInformationString()
    {
        return _pKddiTelopText;
    }

    uint32 getTelopInformationSize()
    {
        return (_telopSize);
    }

private:
    uint8*  _pKddiTelopText;
    uint32  _telopSize;
};
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
#endif
