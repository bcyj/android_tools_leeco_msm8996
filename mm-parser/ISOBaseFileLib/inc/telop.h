#ifndef __TELOP_H__
#define __TELOP_H__
/* =======================================================================
                               telop.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/telop.h#6 $
$DateTime: 2011/11/24 00:50:52 $
$Change: 2061573 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "ztl.h"
#include "tsmlerror.h"
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Global Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Type Declarations
** ----------------------------------------------------------------------- */
enum TIME
{
  CURRENT,
  PAST,
  FUTURE
};

/* Structure of the Header.  */
struct TelopHeader
{
  int32 backGroundColor;
  int32 defaultFontColor;
};

/* Structure for the Substrings. */
struct SubStrings
{
  bool oLineFeed;
  bool oUnderLine;
  bool oReversal;
  bool oLinking;
  int32 sizeofTextSampleInBytes;
  int32 fontColor;
  uint32 linkSize;
  char * textSubString;
  char * linkValue;
};

struct LinkInfoStrct
{
  uint32 linkSize;
  char * linkValue;
};

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* ==========================================================================

                        MACRO DECLARATIONS

========================================================================== */

/* =======================================================================
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* ==========================================================================

                      FUNCTION/CLASS DECLARATIONS

========================================================================== */

/* ======================================================================
CLASS
  TelopElement

DESCRIPTION
  This class stores the information for the telop element.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class TelopElement
{
private:
  bool oWrapFlag;
  int32 numSubStrings;
  int32 beginTime;
  int32 duration;
  int32 telopSize;
  ZArray <SubStrings *> subStringVector;

  friend class TsmlHandle;

public:
  TelopElement ();
  TelopElement (TelopElement & srcTelop,  int32 * constructionresult);
  bool GetWrapFlag()const;/*madeconstant*/
  int32 GetNumSubStrings ()
  {
    return numSubStrings;
  };
  int32 GetBeginTime()
  {
    return beginTime;
  };
  int32 GetDuration ()
  {
    return duration;
  };
  int32 GetTelopSize ()
  {
    return telopSize;
  };
  SubStrings * GetSubStringStructAt (int32 index)
  {
    return subStringVector .ElementAt (index);
  };
  ~TelopElement();
};

/* ======================================================================
CLASS
  TsmlAtom

DESCRIPTION
  This class stores the information about the tsmlatom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class TsmlAtom
{
private:
  int32 currentIndex;

  TelopHeader telopHeader;
  ZArray <TelopElement *> TelopElementTable;
  ZArray <int32> SampleSizeTable;
  ZArray <int32> TimeStampTable;
  ZArray <int32> SampleOffsetTable;

  friend class TsmlHandle;
  TsmlAtom (char * string, int32 size, int32 contenttime, int32 *parsingResult);

public:
  static TsmlAtom * ParseTelopElementText (char * string, int32 size, int32 contenttime,
                                           int32 *parsingResult);

  TelopHeader * GetTelopHeader()
  {
    return & telopHeader;
  };
  ZArray <int32> * GetSampleSizeTable()
  {
    return & SampleSizeTable;
  };
  ZArray <int32> * GetTimeStampTable()
  {
    return & TimeStampTable;
  };
  ZArray <int32> * GetSampleOffsetTable()
  {
    return & SampleOffsetTable;
  };
  ZArray <TelopElement *> * GetTelopElementVector()
  {
    return & TelopElementTable;
  };
  TelopElement * GetTelopElementByIndex(uint32 index);
  TelopElement * GetTelopElementByTime(int32 time, TIME flag);
  void ResetTelopVectorIndex();
  bool ResetTelopVectorIndexByTime(int32 time);
  int32 GetCurrentIndex();
  TelopElement * GetNextTelopElement();
  uint32 GetTelopTrackDuration();

  ~TsmlAtom();
};

/* ======================================================================
CLASS
  TsmlHandle

DESCRIPTION
  This class is used to parse the tsml information
  and store it in the Tsml Atom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class TsmlHandle
{
private:
  int32 errorName;
  bool tsmlFlag;
  bool headFlag;
  bool layoutFlag;
  bool regionFlag;
  bool fontFlag;
  bool bodyFlag;
  bool telopFlag;

  int32 Sample;
  int32 Offset;

  char * stringPointer;
  char * wordPointer;
  uint32 sizeOfWord;

  TsmlAtom * objTsmlAtom;
  TelopElement * T;
  SubStrings * S;

  //TelopElement Related
  bool beginFlag;
  bool endFlag;
  bool previousEndFlag;
  int32 beginTime;
  int32 endTime;
  int32 previousEndTime;
  int32 contentTime;

  //STRING related
  bool bodyFontFlag;
  bool underLineFlag;
  bool lineFeedFlag;
  bool revFlag;
  bool stringFlag;
  int32 stringFontColor;

  //Link related
  bool aFlag;
  enum INFO
  {
    HEADINFO, BODYINFO, LAYOUTINFO, REGIONINFO, FONTINFO
  };
  LinkInfoStrct CurLinkInfo;
  void CopyLinkInfo();

  bool unsupportTag;
  void ResetSubStringAttributes(void);

public:
  int32 errorFlag;
  TsmlHandle(TsmlAtom *pthis);
  ~TsmlHandle();
  void IdentifyWord();
  void TsmlInfo();
  bool StringCompare(const char *referenceWord);

  //Head Information Functions
  void HeadInfo();
  void LayoutInfo();
  int32 RegionInfo();
  int32 FontInfo();

  //Body Information Functions
  void BodyInfo();
  void TelopInfo();

  //String related
  void AddNewString();
  void StringInfo();
  void SetStringPara();
  void LinkInfo();

  //General Functions
  void ErrorTypeOdd(int32 errorNo,char *tag);
  void ErrorTypeEven(int32 errorNo,char *tag);
  void ExtractInfo1(bool *pFlag, const char *startPart, const char *endPart, INFO info);
  int32 ExtractInfo2(bool *pFlag, const char *startPart, const char *endPart, INFO info);
  int32 ExtractPartInfo(const char *startWord,const char *endWord,bool Flag);
  void ExitCode();

  //Main API
  int32 Parse (char *string, int32 size, int32 atomContentTime);
};
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
#endif
