/* =======================================================================
                               textsamplemodifiers.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.  Include any initialization and synchronizing
  requirements.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/textsamplemodifiers.cpp#17 $
$DateTime: 2013/09/19 20:13:39 $
$Change: 4465453 $


========================================================================== */
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "textsamplemodifiers.h"
#include "atomutils.h"
#include "atomdefs.h"

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
  TextStyleBox:: TextStyleBox

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
TextStyleBox:: TextStyleBox (uint8  *buf) : Atom (buf)
{
  _pparent         = NULL;

  _pStyleRecordVec = NULL;
  _entryCount      = 0;

  if ( _success && AtomUtils::read16( buf, _entryCount) )
  {
    _pStyleRecordVec = MM_New( ZArray<StyleRecord*> );

    if (_pStyleRecordVec)
    {
      uint16 numStyleRecordsAdded;
      for ( numStyleRecordsAdded = 0;
            numStyleRecordsAdded < _entryCount;
            numStyleRecordsAdded++ )
      {
        StyleRecord *pStyleRecord = MM_New_Args( StyleRecord , (buf) );

        if (!pStyleRecord)
        {
          break; // fall through
        }
        else if ( !pStyleRecord->FileSuccess() )
        {
          _success = false;
          _fileErrorCode = pStyleRecord->GetFileError();
          MM_Delete(pStyleRecord);
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextStyleBox pStyleRecord is failed ");
          return;
        }
        else
        {
          (*_pStyleRecordVec)+=(pStyleRecord);
        }
      }

      // No error only if all StyleRecords were added.
      if (numStyleRecordsAdded == _entryCount)
        return;
    }
  }

  _success = false;
  _fileErrorCode = PARSER_ErrorReadFail;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
    "TextStyleBox::TextStyleBox is failed ");
  return;
}

/* ======================================================================
FUNCTION
  TextStyleBox::~TextStyleBox

DESCRIPTION
  Destructor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TextStyleBox::~TextStyleBox()
{
  if ( _pStyleRecordVec != NULL )
  {
    for ( uint32 i=0; i<_pStyleRecordVec->GetLength(); i++ )
    {
      MM_Delete( (*_pStyleRecordVec)[i] );
    }
    MM_Delete( _pStyleRecordVec );
    _pStyleRecordVec = NULL;
  }
}

/* ======================================================================
FUNCTION
  TextHighlightBox::TextHighlightBox

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
TextHighlightBox::TextHighlightBox (uint8  *buf) : Atom(buf)
{
  _startCharOffset = 0;
  _endCharOffset   = 0;

  if ( _success && !AtomUtils::read16( buf, _startCharOffset) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if ( _success && !AtomUtils::read16( buf, _endCharOffset) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (!_success)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "TextHighlightBox::TextHighlightBox read failed ");
  }
  return;
}

/* ======================================================================
FUNCTION
  TextHilightColorBox::TextHilightColorBox

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
TextHilightColorBox::TextHilightColorBox (uint8  *buf) : Atom(buf)
{
  _pHighlightColorRGBA = NULL;

  _pHighlightColorRGBA = (uint8*)MM_Malloc(4);
  if ( !_pHighlightColorRGBA )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorMemAllocFail;
  }
  if( _success && !AtomUtils::readByteData(buf, 4, _pHighlightColorRGBA) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (!_success)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "TextHilightColorBox::TextHilightColorBox read failed ");
  }
}

/* ======================================================================
FUNCTION
  TextHilightColorBox::~TextHilightColorBox

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
TextHilightColorBox::~TextHilightColorBox()
{
  if ( _pHighlightColorRGBA != NULL )
  {
    MM_Free(_pHighlightColorRGBA);
    _pHighlightColorRGBA = NULL;
  }
}

/* ======================================================================
FUNCTION
  TextKaraokeBox::TextKaraokeBox

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
TextKaraokeBox::TextKaraokeBox (uint8  *buf) : Atom(buf)
{
  _pHighLightEndTimeVec = NULL;
  _pStartCharOffsetVec  = NULL;
  _highLightStartTime = 0;
  _entryCount = 0;

  if (_success && !AtomUtils::read32( buf, _highLightStartTime) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (_success && !AtomUtils::read16( buf, _entryCount) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  uint32 endTime;
  uint16 startOffset;

  if ( _entryCount > 0 && _success)
  {
    _pHighLightEndTimeVec = (uint32*)MM_Malloc(_entryCount);
    _pStartCharOffsetVec = (uint16*)MM_Malloc(_entryCount);
    if ( !_pHighLightEndTimeVec || !_pStartCharOffsetVec)
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorMemAllocFail;
    }

    for ( uint16 i = 0; i < _entryCount && _success; i++ )
    {
      if ( !AtomUtils::read32(buf, endTime) )
      {
        _success = false;
        _fileErrorCode = PARSER_ErrorReadFail;
      }
      if ( _success && !AtomUtils::read16(buf, startOffset) )
      {
        _success = false;
        _fileErrorCode = PARSER_ErrorReadFail;
      }

      _pHighLightEndTimeVec[i] = endTime;
      _pStartCharOffsetVec[i]  = startOffset;
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (!_success)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "TextKaraokeBox::TextKaraokeBox read failed ");
  }
}

/* ======================================================================
FUNCTION
  TextKaraokeBox::~TextKaraokeBox

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
TextKaraokeBox::~TextKaraokeBox()
{
  if ( _pHighLightEndTimeVec != NULL )
  {
    MM_Free(_pHighLightEndTimeVec);
    _pHighLightEndTimeVec = NULL;
  }

  if ( _pStartCharOffsetVec != NULL )
  {
    MM_Free(_pStartCharOffsetVec);
    _pStartCharOffsetVec = NULL;
  }
}

/* ======================================================================
FUNCTION
  TextScrollDelay::TextScrollDelay

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
TextScrollDelay::TextScrollDelay (uint8  *buf) : Atom(buf)
{
  _scrollDelay = 0;
  if ( _success && !AtomUtils::read32( buf, _scrollDelay))
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (!_success)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "TextScrollDelay::TextScrollDelay read failed ");
  }
}

/* ======================================================================
FUNCTION
  TextHyperTextBox::TextHyperTextBox

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
TextHyperTextBox::TextHyperTextBox (uint8  *buf) : Atom(buf)
{
  _startCharOffset = 0;
  _endCharOffset = 0;
  _urlLength = 0;
  _pURL = NULL;
  _altLength = 0;
  _pAltString = NULL;

  if(_success && !AtomUtils::read16( buf, _startCharOffset) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if(_success && !AtomUtils::read16( buf, _endCharOffset) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if(_success && !AtomUtils::read8( buf, _urlLength) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (_success)
  {
    _pURL = (uint8*)MM_Malloc(_urlLength);
    if (!_pURL)
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorMemAllocFail;
    }
    if ( _pURL && !AtomUtils::readByteData(buf, _urlLength, _pURL) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
    }
  }

  if (_success && !AtomUtils::read8( buf, _altLength) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  _pAltString = (uint8*)MM_Malloc(_altLength);

  if (_pAltString && _success &&
      !AtomUtils::readByteData(buf, _altLength, _pAltString) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    MM_Free(_pAltString);
  }
  else
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorMemAllocFail;
  }
  if (!_success)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "TextHyperTextBox::TextHyperTextBox read failed ");
  }
  return;
}

/* ======================================================================
FUNCTION
  TextHyperTextBox::~TextHyperTextBox

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
TextHyperTextBox::~TextHyperTextBox()
{
  if ( _pURL != NULL )
  {
    MM_Free(_pURL);
    _pURL = NULL;
  }

  if ( _pAltString != NULL )
  {
    MM_Free(_pAltString);
    _pAltString = NULL;
  }
}

/* ======================================================================
FUNCTION
  TextBoxBox::TextBoxBox

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
TextBoxBox::TextBoxBox (uint8  *buf) : Atom(buf)
{
  _pBoxRecord = NULL;

  if ( _success )
  {
    _pBoxRecord = MM_New_Args( BoxRecord, (buf) );

    if(!_pBoxRecord || !_pBoxRecord->FileSuccess())
    {
      _success = false;
      if (_pBoxRecord)
      {
        _fileErrorCode = _pBoxRecord->GetFileError();
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (!_success)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "TextBoxBox::TextBoxBox  read failed ");
  }
  return;
}

/* ======================================================================
FUNCTION
  TextBoxBox::~TextBoxBox

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
TextBoxBox::~TextBoxBox()
{
  if ( _pBoxRecord != NULL )
  {
    MM_Delete( _pBoxRecord );
    _pBoxRecord = NULL;
  }
}

/* ======================================================================
FUNCTION
  BlinkBox::BlinkBox

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
BlinkBox::BlinkBox (uint8  *buf) : Atom(buf)
{
  _endCharOffset = 0;
 _startCharOffset = 0;
  if ( _success )
  {
    if ( !AtomUtils::read16( buf, _startCharOffset) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
    }

    if ( _success && !AtomUtils::read16( buf, _endCharOffset) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (!_success)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "BlinkBox::BlinkBox read failed ");
  }
  return;
}

/* ======================================================================
FUNCTION
  TextSampleModifiers::parseTextModifiers

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
TextSampleModifiers*
TextSampleModifiers::parseTextModifiers(uint8* buf, uint32 size)
{
  TextSampleModifiers *pTextSampleModifiers =
    MM_New_Args( TextSampleModifiers, (buf, size) );

  if (pTextSampleModifiers && !pTextSampleModifiers->FileSuccess() )
  {
    MM_Delete(pTextSampleModifiers);
    return NULL;
  }

  return(pTextSampleModifiers);
}

/* ======================================================================
FUNCTION
  TextSampleModifiers:: TextSampleModifiers

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
TextSampleModifiers:: TextSampleModifiers (uint8  *buf, uint32 size)
{
  _pTextStyleBoxVec       = NULL;
  _pTextHighlightBoxVec   = NULL;
  _pTextKaraokeBoxVec     = NULL;
  _pTextHyperTextBoxVec   = NULL;
  _pBlinkBoxVec           = NULL;
  _pTextHilightColorBox   = NULL;
  _pTextScrollDelay       = NULL;
  _pTextBoxBox            = NULL;

  _success = true;

  _pTextStyleBoxVec     = MM_New( ZArray<TextStyleBox*> );
  _pTextHighlightBoxVec = MM_New( ZArray<TextHighlightBox*> );
  _pTextKaraokeBoxVec   = MM_New( ZArray<TextKaraokeBox*> );
  _pTextHyperTextBoxVec = MM_New( ZArray<TextHyperTextBox*> );
  _pBlinkBoxVec         = MM_New( ZArray<BlinkBox*> );
  _fileErrorCode        = PARSER_ErrorNone;

  if ( _success )
  {
    uint32 count    = 0;
    uint32 atomType = AtomUtils::getNextAtomType(buf);

    while ( ((atomType == TEXT_STYLE_BOX) ||
             (atomType == TEXT_HIGHLIGHT_BOX) ||
             (atomType == TEXT_HILIGHT_COLOR_BOX) ||
             (atomType == TEXT_KARAOKE_BOX)||
             (atomType == TEXT_SCROLL_DELAY_BOX) ||
             (atomType == TEXT_HYPER_TEXT_BOX) ||
             (atomType == TEXT_OVER_RIDE_BOX) ||
             (atomType == TEXT_BLINK_BOX)) &&
            (count < size) )
    {
      if ( atomType == TEXT_STYLE_BOX )
      {
        TextStyleBox *pStyleBox = MM_New_Args( TextStyleBox, (buf) );

        if( ( !pStyleBox ) || (!pStyleBox->FileSuccess() ) )
        {
          _success = false;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers pStyleBox is NULL or failed ");
          if (pStyleBox )
          {
            _fileErrorCode = pStyleBox->GetFileError();
            MM_Delete( pStyleBox );
          }
          return;
        }

        if(_pTextStyleBoxVec)
        {
          (*_pTextStyleBoxVec) += pStyleBox;
          count += pStyleBox->getSize();
          buf   += pStyleBox->getSize();
        }
        else
        {
          _fileErrorCode = PARSER_ErrorMemAllocFail;
          _success = false;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "TextSampleModifiers:_pTextStyleBoxVec malloc failed ");
          MM_Delete( pStyleBox );
        }
      }
      else if ( atomType == TEXT_HIGHLIGHT_BOX )
      {
        TextHighlightBox *pHighlightBox = MM_New_Args( TextHighlightBox, (buf) );

        if( ( !pHighlightBox ) || ( !pHighlightBox->FileSuccess() ))
        {
          _success = false;
          if(pHighlightBox)
          {
            _fileErrorCode = pHighlightBox->GetFileError();
            MM_Delete( pHighlightBox );
          }
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers pHighlightBox is NULL or failed ");
          return;
        }

        if(_pTextHighlightBoxVec)
        {
          (*_pTextHighlightBoxVec) += pHighlightBox;
          count += pHighlightBox->getSize();
          buf   += pHighlightBox->getSize();
        }
        else
        {
          _fileErrorCode = PARSER_ErrorMemAllocFail;
          _success = false;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                   "TextSampleModifiers:_pTextHighlightBoxVec malloc failed ");
          MM_Delete( pHighlightBox );
        }
      }
      else if ( atomType == TEXT_HILIGHT_COLOR_BOX )
      {
        if ( _pTextHilightColorBox == NULL )
        {
          _pTextHilightColorBox = MM_New_Args( TextHilightColorBox, (buf) );

          if((!_pTextHilightColorBox)||(!_pTextHilightColorBox->FileSuccess()))
          {
            _success = false;
            if(_pTextHilightColorBox)
            {
              _fileErrorCode = _pTextHilightColorBox->GetFileError();
              MM_Delete(_pTextHilightColorBox);
            }
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "TextSampleModifiers _pTextHilightColorBox is NULL or failed ");
            return;
          }

          count += _pTextHilightColorBox->getSize();
          buf   += _pTextHilightColorBox->getSize();
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers _pTextHilightColorBox is not NULL ");
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else if ( atomType == TEXT_KARAOKE_BOX )
      {
        TextKaraokeBox *pKaraokeBox = MM_New_Args( TextKaraokeBox, (buf) );

        if( (!pKaraokeBox) || ( !pKaraokeBox->FileSuccess() ))
        {
          _success = false;
          if(pKaraokeBox)
          {
            _fileErrorCode = pKaraokeBox->GetFileError();
            MM_Delete(pKaraokeBox);
          }
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers pKaraokeBox is NULL or failed ");
          return;
        }
        if(_pTextKaraokeBoxVec)
        {
          (*_pTextKaraokeBoxVec) += pKaraokeBox;
          count += pKaraokeBox->getSize();
          buf   += pKaraokeBox->getSize();
        }
        else
        {
          _fileErrorCode = PARSER_ErrorMemAllocFail;
          _success = false;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                     "TextSampleModifiers:_pTextKaraokeBoxVec malloc failed ");
          MM_Delete( pKaraokeBox );
        }
      }
      else if ( atomType == TEXT_SCROLL_DELAY_BOX )
      {
        if ( _pTextScrollDelay == NULL )
        {
          _pTextScrollDelay = MM_New_Args( TextScrollDelay, (buf) );

          if((!_pTextScrollDelay) || ( !_pTextScrollDelay->FileSuccess() ))
          {
            _success = false;
            if(_pTextScrollDelay)
            {
              _fileErrorCode = _pTextScrollDelay->GetFileError();
              MM_Delete(_pTextScrollDelay);
            }
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "TextSampleModifiers _pTextScrollDelay is NULL or failed ");
            return;
          }
          count += _pTextScrollDelay->getSize();
          buf   += _pTextScrollDelay->getSize();
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers _pTextScrollDelay is not NULL ");
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else if ( atomType == TEXT_HYPER_TEXT_BOX )
      {
        TextHyperTextBox *pHyperTextBox = MM_New_Args( TextHyperTextBox, (buf) );

        if((!pHyperTextBox) ||( !pHyperTextBox->FileSuccess() ))
        {
          _success = false;
          if (pHyperTextBox)
          {
            _fileErrorCode = pHyperTextBox->GetFileError();
            MM_Delete(pHyperTextBox);
          }
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers pHyperTextBox is NULL or failed ");
          return;
        }

        if(_pTextHyperTextBoxVec)
        {
          (*_pTextHyperTextBoxVec) += pHyperTextBox;
          count += pHyperTextBox->getSize();
          buf   += pHyperTextBox->getSize();
        }
        else
        {
          _fileErrorCode = PARSER_ErrorMemAllocFail;
          _success = false;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                   "TextSampleModifiers:_pTextHyperTextBoxVec malloc failed ");
          MM_Delete( pHyperTextBox );
        }
      }
      else if ( atomType == TEXT_OVER_RIDE_BOX )
      {
        if ( _pTextBoxBox == NULL )
        {
          _pTextBoxBox = MM_New_Args( TextBoxBox, (buf) );

          if((!_pTextBoxBox) || ( !_pTextBoxBox->FileSuccess() ))
          {
            _success = false;
            if (_pTextBoxBox)
            {
              _fileErrorCode = _pTextBoxBox->GetFileError();
              MM_Delete(_pTextBoxBox);
            }
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "TextSampleModifiers _pTextBoxBox is NULL or failed ");
            return;
          }
          count += _pTextBoxBox->getSize();
          buf   += _pTextBoxBox->getSize();
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers _pTextBoxBox is not NULL ");
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else if ( atomType == TEXT_BLINK_BOX )
      {
        BlinkBox *pBlinkBox = MM_New_Args( BlinkBox, (buf) );

        if((!pBlinkBox) || ( !pBlinkBox->FileSuccess() ))
        {
          _success = false;
          if (pBlinkBox)
          {
            _fileErrorCode = pBlinkBox->GetFileError();
            MM_Delete(pBlinkBox);
          }
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "TextSampleModifiers pBlinkBox is NULL or failed ");
          return;
        }

        if(_pBlinkBoxVec)
        {
          (*_pBlinkBoxVec) += pBlinkBox;
          count += pBlinkBox->getSize();
          buf   += pBlinkBox->getSize();
        }
        else
        {
          _fileErrorCode = PARSER_ErrorMemAllocFail;
          _success = false;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "TextSampleModifiers:_pBlinkBoxVec malloc failed ");
          MM_Delete( pBlinkBox );
        }
      }
      atomType = AtomUtils::getNextAtomType(buf);
    }
  }
  else
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "TextSampleModifiers _success is already false ");
    return;
  }
}

/* ======================================================================
FUNCTION
  TextSampleModifiers::~TextSampleModifiers

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
TextSampleModifiers::~TextSampleModifiers()
{
  if ( _pTextStyleBoxVec != NULL )
  {
    for ( uint32 i=0; i<_pTextStyleBoxVec->GetLength(); i++ )
    {
      MM_Delete( (*_pTextStyleBoxVec)[i] );
    }
    MM_Delete( _pTextStyleBoxVec );
    _pTextStyleBoxVec = NULL;
  }
  if ( _pTextHighlightBoxVec != NULL )
  {
    for ( uint32 i=0; i<_pTextHighlightBoxVec->GetLength(); i++ )
    {
      MM_Delete( (*_pTextHighlightBoxVec)[i] );
    }
    MM_Delete( _pTextHighlightBoxVec );
    _pTextHighlightBoxVec = NULL;
  }
  if ( _pTextKaraokeBoxVec != NULL )
  {
    for ( uint32 i=0; i<_pTextKaraokeBoxVec->GetLength(); i++ )
    {
      MM_Delete( (*_pTextKaraokeBoxVec)[i] );
    }
    MM_Delete( _pTextKaraokeBoxVec );
    _pTextKaraokeBoxVec = NULL;
  }
  if ( _pTextHyperTextBoxVec != NULL )
  {
    for ( uint32 i=0; i<_pTextHyperTextBoxVec->GetLength(); i++ )
    {
      MM_Delete( (*_pTextHyperTextBoxVec)[i] );
    }
    MM_Delete( _pTextHyperTextBoxVec );
    _pTextHyperTextBoxVec = NULL;
  }
  if ( _pBlinkBoxVec != NULL )
  {
    for ( uint32 i=0; i<_pBlinkBoxVec->GetLength(); i++ )
    {
      MM_Delete( (*_pBlinkBoxVec)[i] );
    }
    MM_Delete( _pBlinkBoxVec );
    _pBlinkBoxVec = NULL;
  }

  if ( _pTextHilightColorBox != NULL )
  {
    MM_Delete( _pTextHilightColorBox );
    _pTextHilightColorBox = NULL;
  }
  if ( _pTextScrollDelay != NULL )
  {
    MM_Delete( _pTextScrollDelay );
    _pTextScrollDelay = NULL;
  }
  if ( _pTextBoxBox != NULL )
  {
    MM_Delete( _pTextBoxBox );
    _pTextBoxBox = NULL;
  }
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
