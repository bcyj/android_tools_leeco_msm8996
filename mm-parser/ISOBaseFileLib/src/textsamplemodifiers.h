#ifndef __TextSampleModifiers_H__
#define __TextSampleModifiers_H__
/* =======================================================================
                               textsamplemodifiers.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2008-2013 QUALCOMM Technologies Inc., All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/textsamplemodifiers.h#8 $
$DateTime: 2013/09/02 04:12:20 $
$Change: 4367443 $


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
#include "oscl_file_io.h"
#include "atom.h"
#include "atomutils.h"
#include "atomdefs.h"
#include "boxrecord.h"
#include "stylerecord.h"
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
  TextStyleBox

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
class TextStyleBox : public Atom
{

public:
  TextStyleBox (uint8  *buf); // Default constructor
  virtual ~TextStyleBox (); // Destructor

  int32 getNumStyleRecordEntries()
  {
    return((int32)(_entryCount));
  }

  StyleRecord* getStyleRecordAt(uint32 index)
  {
    if ( (_pStyleRecordVec->GetLength() == 0) ||
         (index >= (_pStyleRecordVec->GetLength())) )
    {
      return NULL;
    }

    return(*_pStyleRecordVec)[index];
  }

private:
  uint16                   _entryCount;
  ZArray<StyleRecord*>  *_pStyleRecordVec;
};

/* ======================================================================
CLASS
  TextHighlightBox

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
class TextHighlightBox : public Atom
{

public:
  TextHighlightBox (uint8  *buf);

  virtual ~TextHighlightBox ()
  {
  };

  uint16 getStartCharOffset()
  {
    return(_startCharOffset);
  }

  uint16 getEndCharOffset()
  {
    return(_endCharOffset);
  }

private:
  uint16                  _startCharOffset;
  uint16                  _endCharOffset;
};

/* ======================================================================
CLASS
  TextHilightColorBox

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
class TextHilightColorBox : public Atom
{
public:
  TextHilightColorBox (uint8  *buf);

  virtual ~TextHilightColorBox ();

  uint8 *getHighLightColorRGBA()
  {
    return(_pHighlightColorRGBA);
  }

private:
  uint8   *_pHighlightColorRGBA;
};


/* ======================================================================
CLASS
  TextKaraokeBox

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
class TextKaraokeBox : public Atom
{
public:
  TextKaraokeBox (uint8  *buf);

  virtual ~TextKaraokeBox ();

  uint32 getHighLightStartTime()
  {
    return(_highLightStartTime);
  }

  uint16 getNumKaraokeEntries()
  {
    return(_entryCount);
  }

  uint32 *getHighLightEndTimeVec()
  {
    return(_pHighLightEndTimeVec);
  }

  uint16 *getStartCharOffsetVec()
  {
    return(_pStartCharOffsetVec);
  }

private:
  uint32  _highLightStartTime;
  uint16  _entryCount;

  uint32  *_pHighLightEndTimeVec;
  uint16  *_pStartCharOffsetVec;
};

/* ======================================================================
CLASS
  TextScrollDelay

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
class TextScrollDelay : public Atom
{
public:
  TextScrollDelay (uint8  *buf);

  virtual ~TextScrollDelay ()
  {
  };

  uint32 getScrollDelay()
  {
    return(_scrollDelay);
  }

private:
  uint32  _scrollDelay;
};

/* ======================================================================
CLASS
  TextHyperTextBox

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
class TextHyperTextBox : public Atom
{
public:
  TextHyperTextBox (uint8  *buf);

  virtual ~TextHyperTextBox ();

  uint16 getStartCharOffset()
  {
    return(_startCharOffset);
  }

  uint16 getEndCharOffset()
  {
    return(_endCharOffset);
  }

  uint8 getUrlLength()
  {
    return(_urlLength);
  }

  uint8* getURL()
  {
    return(_pURL);
  }

  uint8 getAltStringLength()
  {
    return(_altLength);
  }

  uint8* getAltString()
  {
    return(_pAltString);
  }

private:
  uint16  _startCharOffset;
  uint16  _endCharOffset;
  uint8   _urlLength;
  uint8   *_pURL;
  uint8   _altLength;
  uint8   *_pAltString;
};


/* ======================================================================
CLASS
  TextBoxBox

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
class TextBoxBox : public Atom
{
public:
  TextBoxBox (uint8  *buf);

  virtual ~TextBoxBox ();

  BoxRecord* getBoxRecord()
  {
    return(_pBoxRecord);
  }

private:
  BoxRecord*  _pBoxRecord;
};

/* ======================================================================
CLASS
  BlinkBox

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
class BlinkBox : public Atom
{
public:
  BlinkBox (uint8  *buf);

  virtual ~BlinkBox ()
  {
  };

  uint16 getStartCharOffset()
  {
    return(_startCharOffset);
  }
  uint16 getEndCharOffset()
  {
    return(_endCharOffset);
  }

private:
  uint16  _startCharOffset;
  uint16  _endCharOffset;
};


/* ======================================================================
CLASS
  TextSampleModifiers

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
class TextSampleModifiers : public ISucceedFail
{
public:
  static TextSampleModifiers* parseTextModifiers(uint8* buf, uint32 size);

  TextSampleModifiers(uint8* buf, uint32 size);
  virtual ~TextSampleModifiers();

  int32 getNumTextStyleBoxes()
  {
    return(_pTextStyleBoxVec->GetLength());
  }

  TextStyleBox* getTextStyleBoxAt(uint32 index)
  {
    if ( (_pTextStyleBoxVec->GetLength() == 0) ||
         (index >= (_pTextStyleBoxVec->GetLength())) )
    {
      return NULL;
    }

    return(*_pTextStyleBoxVec)[index];
  }

  int32 getNumTextHighlightBoxes()
  {
    return(_pTextHighlightBoxVec->GetLength());
  }

  TextHighlightBox* getTextHighlightBoxAt(uint32 index)
  {
    if ( (_pTextHighlightBoxVec->GetLength() == 0) ||
         (index >= (_pTextHighlightBoxVec->GetLength())) )
    {
      return NULL;
    }

    return(*_pTextHighlightBoxVec)[index];
  }

  int32 getNumTextKaraokeBoxes()
  {
    return(_pTextKaraokeBoxVec->GetLength());
  }

  TextKaraokeBox* getTextKaraokeBoxAt(uint32 index)
  {
    if ( (_pTextKaraokeBoxVec->GetLength() == 0) ||
         (index >= (_pTextKaraokeBoxVec->GetLength())) )
    {
      return NULL;
    }

    return(*_pTextKaraokeBoxVec)[index];
  }

  int32 getNumTextHyperTextBoxes()
  {
    return(_pTextHyperTextBoxVec->GetLength());
  }

  TextHyperTextBox* getTextHyperTextBoxAt(uint32 index)
  {
    if ( (_pTextHyperTextBoxVec->GetLength() == 0) ||
         (index >= (_pTextHyperTextBoxVec->GetLength())) )
    {
      return NULL;
    }

    return(*_pTextHyperTextBoxVec)[index];
  }

  int32 getNumTextBlinkBoxes()
  {
    return(_pBlinkBoxVec->GetLength());
  }

  BlinkBox* getTextBlinkBoxAt(uint32 index)
  {
    if ( (_pBlinkBoxVec->GetLength() == 0) ||
         (index >= (_pBlinkBoxVec->GetLength())) )
    {
      return NULL;
    }

    return(*_pBlinkBoxVec)[index];
  }

  TextHilightColorBox* getTextHilightColorBox()
  {
    return _pTextHilightColorBox;
  }

  TextScrollDelay* getTextScrollDelayBox()
  {
    return _pTextScrollDelay;
  }

  TextBoxBox* getTextOverideBox()
  {
    return _pTextBoxBox;
  }

  bool PopulateTextSampleModifierParams()
  {
    return FALSE;
  }

private:
  ZArray<TextStyleBox*>       *_pTextStyleBoxVec;
  ZArray<TextHighlightBox*>   *_pTextHighlightBoxVec;
  ZArray<TextKaraokeBox*>     *_pTextKaraokeBoxVec;
  ZArray<TextHyperTextBox*>   *_pTextHyperTextBoxVec;
  ZArray<BlinkBox*>           *_pBlinkBoxVec;
  TextHilightColorBox         *_pTextHilightColorBox;
  TextScrollDelay             *_pTextScrollDelay;
  TextBoxBox                  *_pTextBoxBox;
};
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
#endif
