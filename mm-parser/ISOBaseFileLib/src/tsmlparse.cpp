/* =======================================================================
                               tsmlparse.cpp
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

Copyright 2008-2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/tsmlparse.cpp#18 $
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
#include "telop.h"

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
  TsmlHandle :: TsmlHandle

DESCRIPTION
  Constructor for the TsmlHandle Class

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TsmlHandle :: TsmlHandle (TsmlAtom * pthis)
{
  wordPointer   = NULL;
  stringPointer = NULL;
  sizeOfWord    = 0;
  Sample        = 0;
  Offset        = 0;
  errorName     = 0;

  tsmlFlag   = 0;
  headFlag   = 0;
  layoutFlag = 0;
  regionFlag = 0;
  fontFlag   = 0;

  bodyFlag  = 0;
  telopFlag = 0;

  T = NULL;
  S = NULL;

  /* TelopHeader related */
  objTsmlAtom = pthis;
  objTsmlAtom -> currentIndex = -1;

  /* Telop Element related */
  beginFlag = 0;
  beginTime = 0;
  endFlag = 0;
  endTime = 0;
  previousEndFlag = 1;
  previousEndTime = 0;
  contentTime  = 0;

  /* STRING related */
  bodyFontFlag = 0;
  stringFontColor = 0;
  underLineFlag = 0;
  lineFeedFlag = 0;
  revFlag = 0;
  stringFlag = 0;

  /* Link related */
  aFlag = 0;
  CurLinkInfo.linkSize = 0;
  CurLinkInfo.linkValue = NULL;

  unsupportTag = 0;
  errorFlag = 0;
}

/* ======================================================================
FUNCTION
  TsmlHandle :: Parse

DESCRIPTION
  This function makes word from the stream.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 TsmlHandle :: Parse (char * string, int32 size, int32 atomContentTime)
{
  int32 strcounter;

  errorFlag = SUCCESSFUL;

  strcounter = 0;

  contentTime = atomContentTime;

  while ( string [strcounter] == ' ' )
  {
    strcounter ++;
    Sample ++;
    Offset ++;
  }

  wordPointer = & string [strcounter];

  if ( size > 1 )
  {
    for ( ; strcounter < size; strcounter ++, Sample ++, Offset ++ )
    {
      if ( errorFlag != 0 )
      {
        break;
      }

      switch ( string [strcounter] )
      {
      case '\0':          /* To handle line break or end of file */
        if ( sizeOfWord>1 )
        {
          IdentifyWord ();
          sizeOfWord = 0;
        }
        break;

      case '>':           /* The word has been completed */
        sizeOfWord++;
        if ( sizeOfWord>1 )
        {
          IdentifyWord ();
          sizeOfWord = 0;
        }
        break;

      case '<':           /* New word has been started */
        if ( sizeOfWord>=1 )
        {
          IdentifyWord ();
          sizeOfWord = 0;
        }
        wordPointer = &string[strcounter];
        sizeOfWord++;
        break;

      case ' ': /* Either the word is completed, or the word requires '>' to complete */
        if(sizeOfWord == 0)
        {
          wordPointer = &string[strcounter];
        }
        sizeOfWord++;
        strcounter++;Sample++;Offset++;
        while ( string[strcounter]==' ' && strcounter<=size )
        {
          strcounter++;
          Sample++;
          Offset++;
          sizeOfWord++;
        }
        if ( string[strcounter]=='>' )  /* word completed */
        {
          sizeOfWord++;
        }
        else
        {
          if ( string[strcounter]=='<' )  /* beginning of new word */
          {
            wordPointer = &string[strcounter];
            sizeOfWord = 0;
          }
          strcounter--;
          Sample--;
          Offset--;
        }
        if ( sizeOfWord>1 )
        {
          IdentifyWord ();
          sizeOfWord = 0;
        }
        break;

      default:
        if ( sizeOfWord==0 )
        {
          wordPointer = &string[strcounter];
        }
        sizeOfWord ++;
        break;
      }
    }
  }

  return errorFlag;
}

/* ======================================================================
FUNCTION
  TsmlHandle :: IdentifyWord

DESCRIPTION
  This function identifies whether tsml atom has been started or not? If started
  tsmlFlag is set and the word is passed to the TsmlInfo() funtion.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: IdentifyWord ()
{
  if ( ! tsmlFlag )
  {
    if ( StringCompare ("<tsml") )
    {
      tsmlFlag = 1;
    }
    else if ( StringCompare ("</tsml") )
    {
      errorFlag = ERROR_endTSML_BEFORE_startTSML;
    }
  }
  else
  {
    if ( StringCompare ("</tsml") )
    {
      if ( !previousEndFlag && T)
      {
        if ( beginFlag )
        {
          T->duration = contentTime - beginTime;
        }
        else
        {
          T->duration = contentTime - previousEndTime;
        }
      }
      else
      {
        tsmlFlag = 0;
      }
    }
    else if ( StringCompare ("<tsml") )
    {
      errorFlag = ERROR_startTSML_BEFORE_endTSML;
    }
    else
      TsmlInfo ();
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: TsmlInfo

DESCRIPTION
  This function parses the word to the HeadInfo() function or BodyInfo() function
  depending upon the status of headFlag and bodyFlag.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: TsmlInfo ()
{
  ExtractInfo1 (& headFlag, "<head", "</head", HEADINFO);

  ExtractInfo1 (& bodyFlag, "<body", "</body", BODYINFO);
}

/* ======================================================================
FUNCTION
  TsmlHandle :: HeadInfo

DESCRIPTION
  This function extracts the layout information from the word.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: HeadInfo ()
{
  /* Layout Element */
  ExtractInfo1 (& layoutFlag, "<layout", "</layout", LAYOUTINFO);
}

/* ======================================================================
FUNCTION
  TsmlHandle :: LayoutInfo

DESCRIPTION
  This function stores the region information and font information in the telop
  header.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: LayoutInfo ()
{
  /* Extract Region Info */
  objTsmlAtom -> telopHeader . backGroundColor =
  ExtractInfo2(& regionFlag,"<region","</region",REGIONINFO);

  /* Extract Font Info */
  objTsmlAtom -> telopHeader . defaultFontColor =
  ExtractInfo2(&fontFlag,"<font","</font",FONTINFO);

  stringFontColor = objTsmlAtom -> telopHeader . defaultFontColor;
}

/* ======================================================================
FUNCTION
  TsmlHandle :: RegionInfo

DESCRIPTION
  This function extracts the background color information from the word.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 TsmlHandle :: RegionInfo ()
{
  return ExtractPartInfo ("background-color=\"#", "\">", 1);
}

/* ======================================================================
FUNCTION
  TsmlHandle :: FontInfo

DESCRIPTION
  This function extracts the font color information from the word.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 TsmlHandle :: FontInfo ()
{
  return ExtractPartInfo ("color=\"#", "\">", 1);
}

/* ======================================================================
FUNCTION
  TsmlHandle :: BodyInfo

DESCRIPTION
  This function directs the body information to the further functions. Once
  telop start tag comes telop flag is set and the word is passed to the TelopInfo()
  function. When end of telop tag comes, telop flag is reset.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: BodyInfo ()
{
  if ( ! telopFlag )
  {
    if ( StringCompare ("<telop") )
    {
      T = MM_New( TelopElement );

      /* reset all old telop string attributes */
      ResetSubStringAttributes();

      if ( T )
      {
        objTsmlAtom -> TelopElementTable += T;
      }
      else
      {
        errorFlag = ERROR_IN_ALLOCATING_MEMORY_FOR_TELOP_ELEMENT;
      }

      objTsmlAtom -> SampleOffsetTable += (Offset - 6);
      objTsmlAtom -> currentIndex ++;
      Sample = 7;
      telopFlag = 1;
      beginFlag = 0;
      endFlag = 0;
    }
    else if ( StringCompare ("</telop") )
    {
      errorFlag = ERROR_endTELOP_BEFORE_startTELOP;
    }
  }
  else
  {
    if ( StringCompare ("</telop") )
    {
      if ( beginFlag ) /* current Telop has beginTime */
      {
        if ( !previousEndFlag ) /*  Previous telop has no endtime, but as cureent  */
                                /* has begintime. Previous endtime should be set   */
                                /* with the current begin time                     */
        {
          /* Do for the past---PART A */
          TelopElement *previousTelopElement;
          previousTelopElement = objTsmlAtom->GetTelopElementByIndex(objTsmlAtom->currentIndex-1);
          if( previousTelopElement != NULL)
          {
            previousTelopElement->duration = beginTime - objTsmlAtom->TimeStampTable.ElementAt(objTsmlAtom->currentIndex-1);
          }
        }
        else if ( previousEndTime > beginTime )
        {
          TelopElement *previousTelopElement;
          previousTelopElement = objTsmlAtom->GetTelopElementByIndex(objTsmlAtom->currentIndex-1);
          if(previousTelopElement != NULL)
          {
            previousTelopElement->duration = beginTime - objTsmlAtom->TimeStampTable.ElementAt(objTsmlAtom->currentIndex-1);
          }
        }

        /* Do for the current */
        objTsmlAtom->TimeStampTable += beginTime;
        T -> beginTime = beginTime;
        if ( endFlag )  /* current telop has both beginTime and endTime --> */
                        /* normal setup -->previousEndFlag = 1 ---PART B    */
        {
          T->duration = endTime - beginTime;
          previousEndFlag = 1;
          previousEndTime = endTime;
        }
        else      /* current telop has no endTime, so should wait for the  */
                  /* next telop to come and set the endTime information */
                  /* previousEndFlag = 0; ---PART C */
        {
          previousEndFlag = 0;
        }
      }

      else /* current Telop has no beginTime */
      {
        if ( !previousEndFlag )   /* previous has no endTime and current has no */
                                  /* beginTime --> ERROR ---PART D */
        {
          errorFlag = ERROR_PREVIOUS_HAS_NO_ENDTIME_AND_CURRENT_HAS_NO_BEGINTIME;
        }
        else  /* previous has endTime so no need to worry about the past */
              /* just do for the present                                 */
        {
          /* Do for the current */
          if ( objTsmlAtom->currentIndex == 0 )  /* ---PART G */
          {
            objTsmlAtom->TimeStampTable += 0;
            T->beginTime = 0;
          }
          else  /* ---PART H */
          {
            objTsmlAtom->TimeStampTable += previousEndTime;
            T->beginTime = previousEndTime;
          }

          if ( endFlag )  /* current telop has no beginTime and but endTime --> */
                          /* previousEndFlag = 1                                */
          {
            if ( objTsmlAtom->currentIndex == 0 )  /* ---PART E */
            {
              T->duration = endTime;
            }
            else  /* ---PART F */
            {
              T->duration = endTime - previousEndTime;
            }
            previousEndTime = endTime;
            previousEndFlag = 1;
          }
          else      /* current telop has no beginTime and endTime, so should wait  */
                    /* for the next telop to come and set the endTime information  */
                    /* previousEndFlag = 0;                                        */
          {
            previousEndFlag = 0;
          }
        }
      }

      objTsmlAtom -> SampleSizeTable += Sample;
      T->telopSize = Sample;
      telopFlag = 0;
      SetStringPara ();
    }

    else if ( StringCompare ("<telop") )
    {
      errorFlag = ERROR_startTELOP_BEFORE_endTELOP;
    }
    else
    {
      TelopInfo ();
    }
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: TelopInfo

DESCRIPTION
  This function analyse the telop element.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: TelopInfo ()
{
  /* Begin Element */
  if ( StringCompare ("begin=") )
  {
    beginFlag = 1;
    beginTime = ExtractPartInfo ("begin=\"", "\"", 0);
  }
  /* End Element */
  else if ( StringCompare ("end=") )
  {
    endFlag = 1;
    endTime = ExtractPartInfo ("end=\"", "\">", 0);
  }

  /* Check for Wrap Flag */
  else if ( (StringCompare ("wrap=\"true\"")) || (StringCompare ("wrap=true")) )
  {
    T -> oWrapFlag = 1;
  }
  else if ( (StringCompare ("wrap=\"false\"")) || (StringCompare ("wrap=false")) )
  {
    T -> oWrapFlag = 0;
  }

  /* ///////////////////////////////////////////////// */
  /* Font Element */
  else if ( StringCompare ("<font") && ! bodyFontFlag )
  {
    SetStringPara ();
    bodyFontFlag=1;
  }
  else if ( StringCompare ("</font") && ! bodyFontFlag )
  {
    errorFlag = ERROR_endFONT_BODY_BEFORE_startFONT_BODY;
  }

  else if ( StringCompare ("</font") && bodyFontFlag )
  {
    SetStringPara ();
    bodyFontFlag=0;
    stringFontColor = objTsmlAtom -> telopHeader . defaultFontColor;
  }
  else if ( StringCompare ("<font") && bodyFontFlag )
  {
    errorFlag = ERROR_startFONT_BODY_BEFORE_endFONT_BODY;
  }
  else if ( StringCompare ("color=") && bodyFontFlag )
  {
    stringFontColor = ExtractPartInfo ("color=\"#", "\">", 1);
  }

  /* ///////////////////////////////////////////////////////// */
  /*  Line feed */
  else if ( StringCompare ("<br/") || StringCompare ("<br>") )
  {
    SetStringPara();
    lineFeedFlag = 1;
  }

  /* ////////////////////////////////////////////////////////////// */
  /* Underline */
  else if ( StringCompare ("<u") && ! underLineFlag )
  {
    SetStringPara ();
    underLineFlag=1;
  }
  else if ( StringCompare ("</u") && ! underLineFlag )
  {
    errorFlag = ERROR_endUNDERLINE_BEFORE_startUNDERLINE;
  }
  else if ( StringCompare ("</u") && underLineFlag )
  {
    SetStringPara ();
    underLineFlag = 0;
  }
  else if ( StringCompare ("<u") && underLineFlag )
  {
    errorFlag = ERROR_startUNDERLINE_BEFORE_endUNDERLINE;
  }
  /* ///////////////////////////////////////////////////////////// */
  /* Rev element */
  else if ( StringCompare ("<rev") && ! revFlag )
  {
    SetStringPara ();
    revFlag=1;
  }
  else if ( StringCompare ("</rev") && ! revFlag )
  {
    errorFlag = ERROR_endREVERSAL_BEFORE_startREVERSAL;
  }
  else if ( StringCompare ("</rev") && revFlag )
  {
    SetStringPara ();
    revFlag=0;
  }
  else if ( StringCompare ("<rev") && revFlag )
  {
    errorFlag = ERROR_startREVERSAL_BEFORE_endREVERSAL;
  }

  /* ///////////////////////////////////////// */
  /* 'a' Element */
  else if ( StringCompare ("<a") && ! aFlag )
  {
    SetStringPara ();
    aFlag = 1;
  }
  else if ( StringCompare ("</a") && ! aFlag )
  {
    errorFlag = ERROR_endA_ELEMENT_BEFORE_startA_ELEMENT;
  }
  else if ( StringCompare ("</a") && aFlag )
  {
    SetStringPara ();
    aFlag=0;
  }
  else if ( StringCompare ("<a") && aFlag )
  {
    errorFlag = ERROR_startA_ELEMENT_BEFORE_endA_ELEMENT;
  }
  else if ( StringCompare ("href=") && ! aFlag )
  {
    errorFlag = ERROR_WITHOUT_A_ELEMENT_HREF;
  }
  else if ( StringCompare ("href=") && aFlag )
  {
    LinkInfo ();
  }
  else if ( StringCompare ("</") )
  {
    /* end of un-supported tag */
    SetStringPara ();
    unsupportTag = 0;
  }
  else if ( StringCompare ("<") )
  {
    /* unsupported tag */
    SetStringPara ();
    unsupportTag = 1;
  }
  else if ( unsupportTag && wordPointer[sizeOfWord-1] == '>'  )
  {
    /* ignore the word and it is end of un-supported tag */
    unsupportTag = 0;
  }
  else
  {
    StringInfo ();
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: ResetSubStringAttributes

DESCRIPTION
  resets all the telop sub string attributes

DEPENDENCIES

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: ResetSubStringAttributes(void)
{
  stringFontColor = objTsmlAtom->telopHeader.defaultFontColor;
  underLineFlag = 0;
  lineFeedFlag = 0;
  revFlag = 0;
  aFlag = 0;
  CurLinkInfo.linkSize = 0;
  if(CurLinkInfo.linkValue)
  {
    MM_Free(CurLinkInfo.linkValue);
    CurLinkInfo.linkValue = NULL;
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: CopyLinkInfo

DESCRIPTION
  This function copies the link information to the current sub string.

DEPENDENCIES
  LinkInfo should have valid information.

RETURN VALUE
  NONE

SIDE EFFECTS
  NONE

========================================================================== */
void TsmlHandle :: CopyLinkInfo ()
{
  if (S){
    S->linkSize = CurLinkInfo.linkSize;
    S->linkValue = (char*)MM_Malloc((CurLinkInfo.linkSize) + 1);
    if(S->linkValue)
    {
#ifdef _ANDROID_
      strlcpy(S->linkValue,CurLinkInfo.linkValue, (CurLinkInfo.linkSize) + 1);
#else
      std_strlcpy(S->linkValue,CurLinkInfo.linkValue, (CurLinkInfo.linkSize) + 1);
#endif
    }
    S->oLinking = 1;
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: LinkInfo

DESCRIPTION
  This function extracts the link information from the telop element.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: LinkInfo ()
{
  if(CurLinkInfo.linkValue)
  {
    MM_Free(CurLinkInfo.linkValue);
  }

  if ( (CurLinkInfo.linkValue = (char*)MM_Malloc((sizeOfWord-8) + 1)) == NULL )
  {
    errorFlag = ERROR_IN_ALLOCATING_MEMORY_FOR_LINKVALUE;
  }
  else
  {
    for ( CurLinkInfo.linkSize = 0; CurLinkInfo.linkSize < (sizeOfWord - 8); CurLinkInfo.linkSize ++ )
    {
      CurLinkInfo.linkValue [CurLinkInfo.linkSize] = *(wordPointer + CurLinkInfo.linkSize + 6);
    }

    CurLinkInfo.linkValue [CurLinkInfo.linkSize] = '\0';
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: StringInfo

DESCRIPTION
  This function stores the information in the string. If string flag is not set
  it adds the word in a new string and if string flag is set the word is appended
  to the existing string.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: StringInfo ()
{
  if( !unsupportTag )
  {
    if ( ! stringFlag )
    {
      AddNewString ();
      stringFlag = 1;
      stringPointer = wordPointer;
      if(S)
      {
         S -> sizeofTextSampleInBytes = sizeOfWord;
      }
    }
    else
    {
      if(S)
      {
         S -> sizeofTextSampleInBytes += sizeOfWord;
      }
    }
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: AddNewString

DESCRIPTION
  This function adds a new string in the telop element. This also initializes
  various parameters of a new string.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: AddNewString ()
{
  S = (SubStrings*)MM_Malloc(sizeof(SubStrings));

  if ( S )
  {
    memset(S, 0, sizeof(SubStrings));

    if ( T )
    {
      T -> subStringVector += S;
      T -> numSubStrings++;
    }

    S -> sizeofTextSampleInBytes = 0;
    S -> linkSize = 0;
    S -> oLineFeed = 0;
    S -> textSubString = 0;
    S -> linkValue = 0;
  }
  else
  {
    errorFlag = ERROR_IN_ALLOCATING_MEMORY_FOR_SUBSTRINGS;
  }

}

/* ======================================================================
FUNCTION
  TsmlHandle :: SetStringPara

DESCRIPTION
  When string is ended, this function sets the various string parameters.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: SetStringPara ()
{
  if ( stringFlag && S)
  {
    S -> fontColor = stringFontColor;
    S -> oUnderLine = underLineFlag;
    S -> oReversal = revFlag;
    S -> oLineFeed = lineFeedFlag;
    /* line feed flag is used only once, then reset it. */
    lineFeedFlag = 0;

    if ( (S->textSubString = (char*)MM_Malloc(S->sizeofTextSampleInBytes + 1)) == NULL )
    {
      errorFlag = ERROR_IN_ALLOCATING_MEMORY_FOR_STRING_MEMBER;
    }
    else
    {
#ifdef _ANDROID_
     strlcpy(S->textSubString,stringPointer,(S->sizeofTextSampleInBytes + 1));
#else
     std_strlcpy(S->textSubString,stringPointer,(S->sizeofTextSampleInBytes + 1));
#endif
     S->textSubString[S->sizeofTextSampleInBytes] = '\0';
    }
    stringFlag = 0;
    S -> oLinking = aFlag;
    if(aFlag && !S->linkValue)
    {
      /* these words are still between "<a" and "<\a>.
         So copy link info to these sub-strings as well */
      CopyLinkInfo();
    }
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: ExtractInfo1

DESCRIPTION
  This function sets or resets or parses the word to the HeadInfo(), BodyInfo()
  or LayoutInfo() depending upon the enum value.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlHandle :: ExtractInfo1 (bool *pFlag, const char *startPart, const char *endPart,
                                 INFO info)
{
  if ( ! * pFlag )
  {
    if ( StringCompare (startPart) )
    {
      * pFlag = 1;
    }
    else if ( StringCompare (endPart) )
    {
      if ( info == HEADINFO )
      {
        errorFlag = ERROR_endHEAD_BEFORE_startHEAD;
      }
      else if ( info == BODYINFO )
      {
        errorFlag = ERROR_endBODY_BEFORE_startBODY;
      }
      else if ( info == LAYOUTINFO )
      {
        errorFlag = ERROR_endLAYOUT_BEFORE_startLAYOUT;
      }
    }
  }
  else
  {
    if ( StringCompare (endPart) )
    {
      * pFlag = 0;
    }
    else if ( StringCompare (startPart) )
    {
      if ( info == HEADINFO )
      {
        errorFlag = ERROR_startHEAD_BEFORE_endHEAD;
      }
      else if ( info == BODYINFO )
      {
        errorFlag = ERROR_startBODY_BEFORE_endBODY;
      }
      else if ( info == LAYOUTINFO )
      {
        errorFlag = ERROR_startLAYOUT_BEFORE_endLAYOUT;
      }
    }
    else
    {
      if ( info == HEADINFO )
      {
        HeadInfo ();
      }
      else if ( info == BODYINFO )
      {
        BodyInfo ();
      }
      else if ( info == LAYOUTINFO )
      {
        LayoutInfo ();
      }
    }
  }
}

/* ======================================================================
FUNCTION
  ExtractInfo2

DESCRIPTION
  This function sets or resets or parses the word to the RegionInfo(), or
  FontInfo() depending upon the enum value.
  Returns the background color or font color.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 TsmlHandle :: ExtractInfo2(bool *pFlag, const char *startPart, const char *endPart,
                                 INFO info)
{
  int32 returnValue = SUCCESSFUL;

  if ( info == REGIONINFO )
  {
    returnValue = objTsmlAtom -> telopHeader . backGroundColor;
  }
  else if ( info == FONTINFO )
  {
    returnValue = objTsmlAtom -> telopHeader . defaultFontColor;
  }

  if ( ! * pFlag )
  {
    if ( StringCompare (startPart) )
    {
      * pFlag = 1;
    }
    else if ( StringCompare (endPart) )
    {
      if ( info == REGIONINFO )
      {
        errorFlag = ERROR_endREGION_BEFORE_startREGION;
      }
      else if ( info == FONTINFO )
      {
        errorFlag = ERROR_endFONT_HEAD_BEFORE_startFONT_HEAD;
      }
    }
  }

  else
  {
    if ( StringCompare (endPart) )
    {
      * pFlag = 0;
    }
    else if ( StringCompare (startPart) )
    {
      if ( info == REGIONINFO )
      {
        errorFlag = ERROR_startREGION_BEFORE_endREGION;
      }
      else if ( info == FONTINFO )
      {
        errorFlag = ERROR_startFONT_HEAD_BEFORE_endFONT_HEAD;
      }
    }
    else
    {
      if ( info == REGIONINFO )
      {
        returnValue = RegionInfo ();
      }
      else if ( info == FONTINFO )
      {
        returnValue = FontInfo ();
      }
    }
  }

  return returnValue;
}

/* ======================================================================
FUNCTION
  TsmlHandle :: ExtractPartInfo

DESCRIPTION
  This function extracts part of the word and returns it directy or its int32
  value depending upon the flag.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 TsmlHandle :: ExtractPartInfo (const char *startWord, const char *endWord, bool Flag)
{
  char return_char [8], * p;
  uint32 i, startLength, endLength;

  startLength = (uint32)strlen (startWord);
  endLength = (uint32)strlen (endWord);

  for ( i = 0; i < (sizeOfWord - startLength - endLength); i ++ )
  {
    return_char [i] = wordPointer [i + startLength];
  }

  return_char [i]='\0';

  if ( Flag )   /* hex to int32 coversion is required */
  {
    return(strtol (return_char, &p, 16));
  }
  else      /* string to int32 conversion is required */
  {
    return(atol (return_char));
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: StringCompare

DESCRIPTION
  This function compare the word and return the bool result true or false.
  The strncmp function compares the two strings and returns a value that
  is   less than, equal, or larger than zero, depending on whether the
  compared   portion of the first string is less than, equal, or greater
  than the compared portion of the second string.

DEPENDENCIES
  The function compares the given string pointed to by wordPointer, which
  is a private member of the class TsmlHandle.

RETURN VALUE
  true - when the referenceWord is found
  false - when the referenceWord is not found

SIDE EFFECTS
  None

========================================================================== */
bool TsmlHandle :: StringCompare(const char *referenceWord)
{
  if ( strncmp(wordPointer,referenceWord,strlen(referenceWord)) )
  {
    return false; /* referenceWord NOT found */
  }
  else
  {
    return true; /* referenceWord found */
  }
}

/* ======================================================================
FUNCTION
  TsmlHandle :: ~TsmlHandle

DESCRIPTION
  free the memory taken by it.

DEPENDENCIES

RETURN VALUE
  None
SIDE EFFECTS
  None

========================================================================== */
TsmlHandle :: ~TsmlHandle()
{
  if(CurLinkInfo.linkValue)
  {
    MM_Free(CurLinkInfo.linkValue);
  }
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
