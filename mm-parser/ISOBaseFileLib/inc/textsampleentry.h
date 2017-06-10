#ifndef __TextSampleEntry_H__
#define __TextSampleEntry_H__
/* =======================================================================
                               textsampleentry.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2011-2012 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/textsampleentry.h#8 $
$DateTime: 2012/07/03 01:18:14 $
$Change: 2556419 $


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

#include "oscl_file_io.h"
#include "sampleentry.h"
#include "fontrecord.h"
#include "boxrecord.h"
#include "stylerecord.h"
#include "fonttableatom.h"

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
#define PVTEXT_MAX_TEXT_COLORS 4
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
  TextSampleEntry

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
class TextSampleEntry : public SampleEntry
{

public:
  TextSampleEntry (OSCL_FILE *fp, DataT eDataType); // Default constructor
  virtual ~TextSampleEntry (); // Destructor

  uint32 getDisplayFlags()
  {
    return _displayFlags;
  }

  int8 getHorzJustification()
  {
    return _horzJustification;
  }

  int8 getVertJustification()
  {
    return _vertJustification;
  }

  uint8 *getBackgroundColourRGBA ()
  {
    return _pBackgroundRGBA;
  }

  int16 getBoxTop ()
  {
    if ( _pBoxRecord != NULL )
    {
      return(_pBoxRecord->getBoxTop());
    }
    else
    {
      return -1;
    }
  }

  int16 getBoxLeft ()
  {
    if ( _pBoxRecord != NULL )
    {
      return(_pBoxRecord->getBoxLeft());
    }
    else
    {
      return -1;
    }
  }

  int16 getBoxBottom ()
  {
    if ( _pBoxRecord != NULL )
    {
      return(_pBoxRecord->getBoxBottom());
    }
    else
    {
      return -1;
    }
  }

  int16 getBoxRight ()
  {
    if ( _pBoxRecord != NULL )
    {
      return(_pBoxRecord->getBoxRight());
    }
    else
    {
      return -1;
    }
  }

  uint16 getStartChar()
  {
    if ( _pStyleRecord != NULL )
    {
      return _pStyleRecord-> getStartChar();
    }
    else
    {
      return 0;
    }
  }

  uint16 getEndChar()
  {
    if ( _pStyleRecord != NULL )
    {
      return _pStyleRecord-> getEndChar();
    }
    else
    {
      return 0;
    }
  }

  uint16 getFontID()
  {
    if ( _pStyleRecord != NULL )
    {
      return _pStyleRecord-> getFontID();
    }
    else
    {
      return 0;
    }
  }

  uint8 getFontStyleFlags()
  {
    if ( _pStyleRecord != NULL )
    {
      return _pStyleRecord-> getFontStyleFlags();
    }
    else
    {
      return 0;
    }
  }

  uint8 getfontSize()
  {
    if ( _pStyleRecord != NULL )
    {
      return _pStyleRecord->getfontSize();
    }
    else
    {
      return 0;
    }
  }

  uint8 *getTextColourRGBA ()
  {
    if ( _pStyleRecord != NULL )
    {
      return _pStyleRecord-> getTextColourRGBA();
    }
    else
    {
      return NULL;
    }
  }

  uint16  getFontListSize()
  {
    if ( _pFontTableAtom != NULL )
    {
      return _pFontTableAtom-> getFontListSize();
    }
    else
    {
      return 0;
    }
  }

  FontTableAtom *getFontTable()
  {
    return _pFontTableAtom;
  }

  FontRecord   *getFontRecordAt(uint32 index)
  {
    if ( _pFontTableAtom != NULL )
    {
      return _pFontTableAtom->getFontRecordAt(index);
    }
    else
    {
      return NULL;
    }
  }
 /*****************************************************************************
  @brief      Get NameSpace

  @details    This function is used to get XML namespaces to which the
              sample documents confirm

  @return     FILESOURCE_STRING

  @note       Specific to SMPTE-TT based subtitle
  *****************************************************************************/
  FILESOURCE_STRING getNameSpace() {return szNameSpace;}

 /*****************************************************************************
  @brief      Get Schema location

  @details    This function is used to get XML schema(s) to which sample
              documents confirm

  @return     FILESOURCE_STRING

  @note       Specific to SMPTE-TT based subtitle
  *****************************************************************************/
  FILESOURCE_STRING getSchemaLocation() {return szSchemaName;}

 /*****************************************************************************
  @brief      Get Mime Type

  @details    This function is used to get media type of all images and fonts
              if present stored as subtitle sub samples

  @return     FILESOURCE_STRING

  @note       Specific to SMPTE-TT based subtitle
  *****************************************************************************/
  FILESOURCE_STRING getMimeType() {return szMimeType;}

private:
  uint32         _displayFlags;
  uint8          _horzJustification;
  uint8          _vertJustification;
  uint8         *_pBackgroundRGBA;
  BoxRecord     *_pBoxRecord;
  StyleRecord   *_pStyleRecord;
  FontTableAtom *_pFontTableAtom;
  FILESOURCE_STRING szNameSpace;
  FILESOURCE_STRING szSchemaName;
  FILESOURCE_STRING szMimeType;
};


#endif
