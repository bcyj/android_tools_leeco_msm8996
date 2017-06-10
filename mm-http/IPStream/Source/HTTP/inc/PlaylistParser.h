#ifndef __PLAYLISTPARSER_H__
#define __PLAYLISTPARSER_H__
/************************************************************************* */
/**
 * PlaylistParser.h
 * @brief Header file for PlaylistParser.
 *
 * COPYRIGHT 2011-2013 QUALCOMM Technologies, Inc.
.* All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/PlaylistParser.h#49 $
$DateTime: 2013/07/26 04:54:56 $
$Change: 4168608 $

========================================================================== */
/* =======================================================================
**               Include files for ProgressiveDownloadHelper.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "Scheduler.h"
#include <HTTPStackInterface.h>
#include "PlaylistDownloader.h"
#include "oscl_string_utils.h"
#include "HTTPBandwidthEstimator.h"
#include "MMCriticalSection.h"
namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

#define PROGRAM_MASK           0xFFF8000000000000ULL
#define PERIOD_MASK            0x0007FFFFC0000000ULL
#define REPRESENTATION_MASK    0x000000003FE00000ULL
#define SEGMENT_MASK           0x00000000001FFFFFULL
#define MAX_AVAILIBILITY_TIME  0xFFFFFFFF

#define MPD_PERIOD_MASK   0xFF00000000000000ULL
#define MPD_REPGRP_MASK   0x00FE000000000000ULL
#define MPD_REPR_MASK     0x0001FF0000000000ULL
#define MPD_SEGMENT_MASK  0x00000000FFFFFFFFULL

#define MPD_PERIOD_SHIFT_COUNT  56
#define MPD_REPGRP_SHIFT_COUNT  49
#define MPD_REPR_SHIFT_COUNT    40

#define MAX_VIEW_POINTS 2
#define MAX_RATINGS 2
#define MAX_CONTENT_PROTECTION 3
#define MAX_ACCESSIBILITY 3

// maxumium template identifier length
#define MAX_TEMPLATE_ID_LEN           64
#define MAX_TEMPLATE_ID_BANDWIDTH_LEN 16
#define MAX_TEMPLATE_ID_TIMELINE_LEN  32
#define MAX_TEMPLATE_ID_NUMBER_LEN    16
//TODO: Add function headers and comments
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class PeriodInfo;
class RepresentationGroup;
class RepresentationInfo;
class SegmentInfo;
class MPDParser;



class SegmentBaseType;
class MultipleSegmentBaseType;
class SegmentTimelineType;
class SegmentTemplateType;
class SegmentListType;
class SegmentURLType;


/*
 * This structure provides  basic URL type
*/
typedef struct URLType
{
  char *sourceURL;
  char *range;
}URLType;


typedef struct MediaStreamStructId
{
  uint32 id;
}MediaStreamStructId;

typedef struct SegmentTimelineStuct
{
  uint64 starTime;
  uint64 duration;
  int32 repeatCount;
}SegmentTimelineStuct;

typedef struct SegmentURLStruct
{
  char *mediaUrl;
  char *mediaRange;
  char *indexUrl;
  char *indexRange;
}SegmentURLStruct;

enum MPDProfileType
{
  DASH_PROFILE_NO_PROFILE = -1,
  DASH_PROFILE_ISO_FULL,
  DASH_PROFILE_ISO_ON_DEMAND,
  DASH_PROFILE_ISO_LIVE,
  DASH_PROFILE_ISO_MAIN,
  DASH_PROFILE_MP2T_MAIN,
  DASH_PROFILE_MP2T_SIMPLE
};

enum ElementType
{
  MPD_PERIOD = 0,
  MPD_ADAPTATIONSET,
  MPD_REPRESENTATION
};

/*
This class stores the information about period. It contains the list of
representation for the period.

*/
class PeriodInfo
{
public:
  /*
  * @brief
  * Returns the period Id
  */
  PeriodInfo();
  PeriodInfo& operator=(const PeriodInfo&);
  PeriodInfo(const PeriodInfo &);
  ~PeriodInfo();
  void Copy(const PeriodInfo &);
  char *getPeriodIdentifier();

  /**
   * Update on an mpd-update
   */
  void SetPeriodKey(uint64 key);

  /**
   * Get the unique period key
   */
  uint64 getPeriodKey() const;

  /*
  * @brief
  * Returns the start time for the period (in seconds)
  */
  uint64 getStartTime() const;
  double getDuration() const;
  uint32 getNumRepGroups();

  void setStartTime(uint64 starttime);

  void setDuration(double duration);

  void SetPeriodInfo(uint64 key,uint64 start_time, double duration, char* periodIdent);

  void SetLastPeriod()
  {
    m_bLastPeriod = true;
  }

  RepresentationGroup* InitializeGroupInfo(int numGroups=0);

  /**
   * Retrieve representationInfo from the representation
   * identifier in MPD.
   */
  RepresentationInfo *GetRepresentationByIdentifier(const char *repIdentifier);

  RepresentationGroup* getRepGrpInfo(int& numRepgrps);

  bool AddRepGroup(int& RepGrpIndex);

  bool RemoveRepGroup();

  bool CommitGroupInfo();

  bool GetGrpKeyForID(uint32 grpID,uint64& grpKey);

  bool InitialiseSegmentBase();
  SegmentBaseType* GetSegmentBase();


  bool InitialiseSegmentTemplate();

  bool IsValidPeriod();

  bool IsSegmentTemplateFound()
  {
    return m_bSegmentTemplateFound;
  }

  bool IsLastPeriod()
  {
    return m_bLastPeriod;
  }
  bool InitializeSegmentTimeline(int numSEntry);
  bool InitialiseSegmentList();
  bool InitialiseSegmentUrl(int numSegUrl);
  SegmentListType* GetSegmentList();

  SegmentTemplateType *GetSegmentTemplate()
  {
    return m_pSegmentTemplate;
  }
  void SetSegmentBaseInfo(uint32 timeScale, uint64 presentationTimeOffset,
                     char *indexRange,bool indexRangeExact,
                     URLType *initialisation, URLType *representationIndex);

  void SetSegmentTemplateInfo(char *mediaTemplate,char *indexTemplate,
                         char *initialisationTemplate, char *bitstreamSwitchingTemplate);
  void SetMultiSegmentBaseInfo( uint32 segmentDuration, uint32 startNumber,
                                uint32 timeScale, uint64 presentationTimeOffset,
                                char *indexRange,bool indexRangeExact,
                                URLType *initialisation, URLType *representationIndex);
  void SetInitialisation(URLType *initialisation);
  bool SetSegmentTimeline(uint32 segTimelineIndex, uint64 startTime,
                          uint64 duration, int32 repeatCount);
  bool SetSegmentUrl(uint32 urlIndex, char *mediaUrl,
                         char *mediaRange, char *indexUrl, char *indexRange);
  bool IsSegmentBaseFound()
  {
    return m_bSegmentBaseFound;
  }

  bool IsSegmentListFound()
  {
    return m_bSegmentListFound;
  }

  uint32 GetNumSegURL()
  {
    return m_nNumSegURL;
  }

private:
  static const int MIN_REP_ARRAY_SIZE = 10;
  static const int MIN_REP_GRP_ARRAY_SIZE = 10;
  RepresentationInfo *m_pRepresentationInfo;
  RepresentationGroup *m_pRepresentationGroup;
  int m_nNumRepGroups;
  char *m_PeriodIdentifier;
  int m_nNumRepresentations;
  uint64 m_nPeriodStartTime;
  double m_nPeriodDuration;
  bool m_bLastPeriod;
  int m_nRepArraySize;
  int m_nRepGrpArrSize;
  uint64 m_nPeriodKey;
  bool ResizeGrpInfo(int newSize);
  SegmentTemplateType *m_pSegmentTemplate;
  bool m_bSegmentTemplateFound;
  SegmentBaseType *m_pSegmentBase;
  bool m_bSegmentBaseFound;
  SegmentListType *m_pSegmentList;
  uint32 m_nNumSegURL;
  bool m_bSegmentListFound;
};

/*
 * This class stores the prescan information about periods.
*/
class PeriodInfoCache
{
public:
  /*
  * @brief
  * Returns the period Id
  */
  PeriodInfoCache()
  {
    m_PeriodIdentifier = NULL;
    m_nPeriodStartTime = 0;
    m_nPeriodDuration = 0;
    m_nPeriodKey = 0;
  }

  ~PeriodInfoCache()
  {
    if(m_PeriodIdentifier)
    {
      QTV_Free(m_PeriodIdentifier);
      m_PeriodIdentifier = NULL;
    }
  }

  /* @brief: Copy constructor
   */
  PeriodInfoCache(const PeriodInfoCache &period_info)
  {
    m_PeriodIdentifier = NULL;
    m_nPeriodStartTime = 0;
    m_nPeriodDuration = 0;
    m_nPeriodKey = 0;
    Copy(period_info);
  }

  /* @brief: Copy function will copy all atributes except
  Representation Info array
  */
  void Copy(const PeriodInfoCache& period_info)
  {
    m_nPeriodKey = period_info.m_nPeriodKey;
    m_nPeriodStartTime = period_info.m_nPeriodStartTime;
    m_nPeriodDuration = period_info.m_nPeriodDuration;
    if(period_info.m_PeriodIdentifier)
    {
      if(m_PeriodIdentifier)
      {
        QTV_Free(m_PeriodIdentifier);
        m_PeriodIdentifier = NULL;
      }
      m_PeriodIdentifier = (char*)QTV_Malloc(std_strlen(period_info.m_PeriodIdentifier)+1);
      if(m_PeriodIdentifier)
      {
        (void)std_strlcpy(m_PeriodIdentifier,period_info.m_PeriodIdentifier,
                              std_strlen(period_info.m_PeriodIdentifier)+1);
      }
    }
    return;
  }

  /* @brief: Assignment operator overloading
  */
  PeriodInfoCache& operator=(const PeriodInfoCache& period_info)
  {
    Copy(period_info);
    return *this;
  }

  char *getPeriodIdentifier()
  {
    return m_PeriodIdentifier;
  }

  /**
   * Update on an mpd-update
   */
  void SetPeriodKey(uint64 key)
  {
    m_nPeriodKey = key;
  }

  /**
   * Get the unique period key
   */
  uint64 getPeriodKey() const
  {
    return m_nPeriodKey;
  }

  /*
  * @brief
  * Returns the start time for the period (in seconds)
  */
  uint64 getStartTime() const
  {
    return m_nPeriodStartTime;
  }

  double getDuration() const
  {
    return m_nPeriodDuration;
  }


  void setStartTime(uint64 starttime)
  {
    m_nPeriodStartTime = starttime;
  }

  void setDuration(double duration)
  {
    m_nPeriodDuration = duration;
  }

  void SetPeriodInfo(uint64 key,uint64 start_time, double duration, char* periodIdent)
  {
    m_nPeriodKey = key;
    m_nPeriodStartTime = start_time;
    m_nPeriodDuration = duration;

    if(m_PeriodIdentifier)
    {
      QTV_Free(m_PeriodIdentifier);
      m_PeriodIdentifier = NULL;
    }

    if(periodIdent)
    {
      m_PeriodIdentifier = (char*)QTV_Malloc(std_strlen(periodIdent)+1);
      if(m_PeriodIdentifier)
      {
        std_strlcpy(m_PeriodIdentifier,periodIdent,std_strlen(periodIdent)+1);
      }
    }
  }

private:
  char   *m_PeriodIdentifier;
  uint64 m_nPeriodStartTime;
  double m_nPeriodDuration;
  uint64 m_nPeriodKey;
};

typedef struct Resolution
{
  uint32 width;
  uint32 height;
}Resolution;

enum MjMediaType
{
  MJ_TYPE_UNKNOWN = 0,
  MJ_TYPE_AUDIO = 1,
  MJ_TYPE_VIDEO = 2,
  MJ_TYPE_TEXT = 4
};

enum MnMediaType
{
  MN_TYPE_UNKNOWN,
  MN_TYPE_AVC,
  MN_TYPE_HVC,
  MN_TYPE_MPEG4,
  MN_TYPE_MP3,
  MN_TYPE_MP2,
  MN_TYPE_MPEG2,
  MN_TYPE_HE_AAC,
  MN_TYPE_AAC_LC,
  MN_TYPE_AC3,
  MN_TYPE_VORBIS,
  MN_TYPE_TIMED_TEXT
};

typedef struct CodecInfo
{
  MjMediaType majorType;
  MnMediaType minorType;
  int profile;
  int level;

  void Reset()
  {
    majorType = MJ_TYPE_UNKNOWN;
    minorType = MN_TYPE_UNKNOWN;
    profile = 0;
    level = 0;
  }

}CodecInfo;

typedef struct Codecs
{
  int numcodecs;
  CodecInfo *mcodecs;
}Codecs;

/**
 * Stores the values associated with the ContentDescriptorType.
 */
class ContentDescriptorType
{
public:
  ContentDescriptorType();
  ~ContentDescriptorType();

  bool SetDesc(const char *descName, const char *schemeURI, const char* value);
  void GetDesc(const char **ppDescName, const char **ppSchemeURI, const char **ppValue);
  void SetInherited(bool bIsInherited);
  bool IsInherited() const;
  bool Copy(const ContentDescriptorType& rCdt);
  void Print();

protected:
  ContentDescriptorType(const ContentDescriptorType&);
  ContentDescriptorType& operator=(const ContentDescriptorType&);

  char *m_DescName;
  char *m_SchemeURI;
  char *m_SchemeInfo;

  // false if present in the mpd at this level, true if inherited from upper
  // level in MPD.
  bool m_bIsInherited;

  friend class PlaylistParser;
};

class ContentProtectionType : public ContentDescriptorType
{
public:
  ContentProtectionType();
  ~ContentProtectionType();

  enum ContentProtectionSource
  {
    CONTENT_PROTECTION_NONE,
    CONTENT_PROTECTION_DASH,
    CONTENT_PROTECTION_MARLIN,
    CONTENT_PROTECTION_PLAYREADY,
    CONTENT_PROTECTION_MAX
  };

  static const char* MARLIN_SCHEME_URI;
  static const char* MARLIN_CONTENTIDS;
  static const char* MARLIN_CONTENTID;
  static const char* MARLIN_FORMAT_VERSION;

  static const char* PLAYREADY_SCHEME_URI;
  static const char* CENC_MP4_SCHEME_URI;
  static const char* CENC_MP2TS_SCHEME_URI;

  void SetProtectionSource(ContentProtectionSource s)
  {
    m_ContentProtectionSource = s;
  }

  ContentProtectionSource GetContentProtectionSource() const;

  void SetPlayReadyContentProtection(const char* prContentProtectoin);
  const char *GetPlayReadyContentProtection() const;

  void SetMarlinContentProtection(const char* mrlnContentProtectoin);
  const char *GetMarlinContentProtection() const;


  void SetCENCContentProtection(const char* cencContentProtectoin);
  const char *GetCENCContentProtection() const;


  void Print();

private:

  ContentProtectionSource m_ContentProtectionSource;
  char *m_PRContentProtectoin;
  char *m_MrlnContentProtectoin;
  char *m_CencContentProtectoin;
};

/**
 * Contains instances of ContentDescriptorType
 */
class ContentDescriptorContainer
{
public:
  ContentDescriptorContainer();
  ~ContentDescriptorContainer();

  bool CopyDescs(const ContentDescriptorContainer& cdc, bool bIsInherited = false);

  /**
   * Add a content descriptor to the internal array. There is no
   * check of whether the params conform to the standard.
   */
  bool AddDescriptorType(const char *descName, const char *schemeURI, const char *value);

  /**
   * Get a pointer to the array of ContentDescriptorType.
   */
  void GetContentDescriptors(ContentDescriptorType** ppContentDescs, int& numDescs);

  /**
   * For debugging
   */
  void PrintDescs();

private:
  ContentDescriptorType *m_Array;
  int m_SizeUsed;
  int m_ArraySize;
};

class StringValue
{
public:
  StringValue();
  ~StringValue();

  bool SetString(const char *name, const char *value);
  void SetInherited(bool bIsInherited);
  bool Copy(const StringValue&);
  void GetStringValue(char **ppName, char **ppValue);

private:
  StringValue(const StringValue&);
  StringValue& operator=(const StringValue&);

  char *m_Name;
  char *m_Value;
  bool m_bIsInherited;
};
class StringValueContainer
{
public:
  StringValueContainer();
  ~StringValueContainer();

  bool AddString(const char *pName, const char *pValue);
  bool CopyStringValues(const StringValueContainer& rCvc, bool bIsInherited = false);
  void GetStringValues(StringValue **ppStringValues, int& size);
  const char *FindString(const char *pName);

  void PrintStringValues();

private:
  StringValue *m_Array;
  int m_SizeUsed;
  int m_ArraySize;
};
class RepresentationGroup
{
public:
  RepresentationGroup();
  RepresentationGroup& operator=(const RepresentationGroup&);
  RepresentationGroup(const RepresentationGroup &);
  ~RepresentationGroup();

  void Copy(const RepresentationGroup& );

  const char *GetGrpIdentifer() const;

  char* getLanguage();

  bool getCodec(CodecInfo* pCodecs,int& numCodecs);

  double getFrameRate();

  uint32 getParX();

  uint32 getParY();

  bool IsSubSegmentAlignment() { return m_bSubSegmentAlignment;}

  uint64 getKey();
  uint32 getGrpID();

  Resolution* getResolution();

  bool AddRepresentation(int &repIndex);

  bool CommitRepInfo();

  bool IsValidRepGroup();

  RepresentationInfo* InitializeRepInfo(int numRepresentations=0);

  void SetGroupInfo(const char *repGrpId,
                    Resolution *pResolution,uint32 parX,uint32 parY,
                                        char *language, char *mimeType, Codecs* codec,uint64 key,
                                        bool subSegmentAlignment,
                                        uint32 frameRate,int numChannels,
                                        double *pChannels, int numSamplingRates,
                                        double *pSamplingRates,
                                        bool bSegmentAligned, int sapVal,
                                        int subSegmentSAPVal,
                                        uint32 grpID = 0);

  RepresentationInfo* getRepInfo(int& numRepresentations);
  void SetCodecInfo(Codecs* codec);
  HTTPBandwidthEstimator *m_bEstimator;
  void UpdatePeriodKey(uint64 periodKey);
  bool InitialiseSegmentBase();
  SegmentBaseType* GetSegmentBase();
  bool InheritSegmentBaseInfo(SegmentBaseType* segment_base);
  bool InitialiseSegmentTemplate();
  bool InheritSegmentTemplate(SegmentTemplateType* segment_template);
  bool InheritSegmentList(SegmentListType *segment_list);
  bool InitializeSegmentTimeline(int numSEntry);
  bool InitialiseSegmentList();
  bool InitialiseSegmentUrl(int numSegUrl);
  SegmentListType* GetSegmentList();
  void SetSegmentBaseInfo(uint32 timeScale, uint64 presentationTimeOffset,
                     char *indexRange,bool indexRangeExact,
                     URLType *initialisation, URLType *representationIndex);

  SegmentTemplateType *GetSegmentTemplate();
  void SetSegmentTemplateInfo(char *mediaTemplate,char *indexTemplate,
                         char *initialisationTemplate, char *bitstreamSwitchingTemplate);
  void SetMultiSegmentBaseInfo(uint32 segmentDuration, uint32 startNumber,
                               uint32 timeScale, uint64 presentationTimeOffset,
                               char *indexRange,bool indexRangeExact,
                               URLType *initialisation, URLType *representationIndex);
  void SetInitialisation(URLType *initialisation);
  void SetInitialisationSegmentUrl(char *base_url, char *url);
  void SetInitialisationSegmentRange(int64 nStartOffset, int64 nEndOffset, char *range);
  char* GetInitialisationSegmentUrl();
  char* GetInitialisationSegmentRange();
  void GetRangeInitialisationSegment(int64& nStartOffset, int64& nEndOffset);

  void SetRepresentationIndex(URLType *representationIndex);
  bool SetSegmentTimeline(uint32 segTimelineIndex, uint64 startTime,
                          uint64 duration, int32 repeatCount);
  bool SetSegmentUrl(uint32 urlIndex, char *mediaUrl,
                         char *mediaRange, char *indexUrl, char *indexRange);
  bool CompareStreamStructId(RepresentationInfo *repInfo1, RepresentationInfo * repInfo2);
  bool CompareMediaStreamStruct();
  bool IsMediaStreamStructIdPresentAllRep();
  bool RemoveRepresentation();
  bool RemoveRepresentationByIndex(int repIndex);
  bool IsSegmentBaseFound()
  {
    return m_bSegmentBaseFound;
  }
  bool IsSegmentTemplateFound()
  {
    return m_bSegmentTemplateFound;
  }
  bool IsSegmentListFound()
  {
    return m_bSegmentListFound;
  }
  int GetNumRepresentations()
  {
    return m_nNumRepresentations;
  }
  void SetSegmentAlinged(bool bIsSegmentAlinged)
  {
    m_bSegmentAlinged = bIsSegmentAlinged;
  }
  bool IsSegmentAlinged()
  {
    return m_bSegmentAlinged;
  }
  void SetSubSegStartWithSAPVal(int subsegSAPVal)
  {
    m_nSubSegmentStartsWithSAP = subsegSAPVal;
  }
  int GetSubSegStartWithSAPVal()
  {
    return m_nSubSegmentStartsWithSAP;
  }
  void SetStartWithSAPVal(int sapVal)
  {
    m_nStartWithSAPVal = sapVal;
  }
  int GetStartWithSAPVal()
  {
    return m_nStartWithSAPVal;
  }
  uint32 GetNumSegURL()
  {
    return m_nNumSegURL;
  }

  /**
   * Get a reference to the ContentDescriptors that can be
   * specified at the adaptation-set level (eg "Accessibility").
   */
  ContentDescriptorContainer& GetAdaptationSetSpecificContentDescriptors();

  /**
   * Get a reference to the ContentDescriptors that can occur at
   * the AdaptationSet level as well as the representation level.
   * Eg. "AudioChannelConfiguration"
   */
  ContentDescriptorContainer& GetCommonContentDescriptors();

  ContentProtectionType& GetContentProtection();

  /**
   * Get a reference to the string-values that can be
   * specified at the adaptation-set level (eg "contentType").
   */
  StringValueContainer& GetAdaptationSetSpecificStringValueContainer();

  /**
   * Get a reference to the string-values that can be
   * specified at the adaptation-set level (eg "mimeType").
   */
  StringValueContainer& GetCommonStringValueContainer();

  /**
   * Check if any representation in this adaptation set is
   * selectable.
   */
  bool IsAnyRepSelectable();

  /**
   * All representations that were marked as selectable are marked
   * as selected.
   */
  void MarkAllSelectableRepsAsSelected();

  /**
   * Check if any representation in this adaptation-set is
   * selected.
   */
  bool IsAnyRepSelected();

private:
  static const int MIN_REP_ARRAY_SIZE = 10;
  RepresentationInfo *m_pRepresentationInfo;
  int m_nNumRepresentations;
  char *m_GrpIdentifier;
  Codecs *m_codec;
  Resolution* m_resolution;
  double m_nFrameRate;
  uint32 *m_SamplingRate;
  int m_nNumSamplingRates;
  double *m_Channels;
  int m_nNumChannels;
  uint32 m_nParX;
  uint32 m_nParY;
  char *m_language;
  char *m_MimeType;
  bool m_bSubSegmentAlignment;
  uint64 m_nKey;
  int m_nRepArraySize;
  bool ResizeRepInfo(int newSize);
  uint32 m_nGrpID;
  bool m_bSegmentAlinged;
  int m_nSubSegmentStartsWithSAP;
  SegmentTemplateType *m_pSegmentTemplate;
  bool m_bSegmentTemplateFound;
  int m_nStartWithSAPVal;
  int m_nMajorType;
  SegmentBaseType *m_pSegmentBase;
  bool m_bSegmentBaseFound;
  SegmentListType *m_pSegmentList;
  uint32 m_nNumSegURL;
  bool m_bSegmentListFound;
  int64 m_nStartOffsetInitSeg;
  int64 m_nEndOffsetInitSeg;
  char* m_InitialisationSegmentUrl;
  char* m_InitialisationSegmentRange;

  ContentDescriptorContainer m_AdaptationSetSpecificDescs;
  ContentDescriptorContainer m_CommonDescs;
  ContentProtectionType m_ContentProtection;

  StringValueContainer m_AdaptationSetStringValues;
  StringValueContainer m_CommonStringValues;
};
/*
This class stores the information about representations. It contains the list of
all segments for the representation.

*/
class RepresentationInfo
{
public:
  class SegmentFuncBase
  {
  public:
    enum Type
    {
      SEGMENT_TYPE_DEFAULT,
      SEGMENT_TYPE_TEMPLATE_WITHOUT_TIMELINE,
    };

    SegmentFuncBase() { }
    virtual ~SegmentFuncBase() { }

    virtual HTTPDownloadStatus GetFirstAvailableSegmentTimeForRepresentation(
       MPDParser *pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo,
       double nCurrMSeconds, bool bStartOfPlayback, uint64& nFirstAvailableSegmentStartTime, uint64 &nFirstAvailableSegmentEndTime) = 0;

    virtual HTTPDownloadStatus GetLastAvailableSegmentTimeForRepresentation(
       MPDParser *pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, double nCurrMSeconds, uint64& nLastAvailableSegmentStartTime, uint64& nLastAvailableSegmentEndTime) = 0;

    virtual HTTPDownloadStatus GetAllSegmentsForRepresentationRange(
      MPDParser* rMPDParser, double currMSeconds, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, uint64* pSegmentInfo, uint32& numSegments,
      uint64 segStartTime,uint64 segEndTime, double &firstAvailableSegmentStartTime) = 0;

    virtual HTTPDownloadStatus GetLastSegmentKeyForRepresentation(
       MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, uint64* pSegmentInfo, RepresentationInfo* pRepInfo ) = 0;

    virtual void AdjustSegmentKeys(RepresentationInfo* pOldRep, RepresentationInfo* pNewRep, uint64 newPeriodKey, int& nTimeOffsetSegment, bool bFirstNewPeriod) = 0;

    virtual Type GetType() const = 0;

    virtual void PrintMPDInfoForRepresentation(MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo) = 0;

    virtual HTTPDownloadStatus GetSegDurationForRepresentation(MPDParser *pMPDParser, RepresentationInfo* pRepInfo, uint64 nSegKey, uint64& segDuration) = 0;

  protected:
    Type m_Type;

  };

  class SegmentFuncTemplate : public SegmentFuncBase
  {
  public:
    SegmentFuncTemplate() { m_Type = SegmentFuncBase::SEGMENT_TYPE_TEMPLATE_WITHOUT_TIMELINE; }
    virtual ~SegmentFuncTemplate() { }

    virtual HTTPDownloadStatus GetFirstAvailableSegmentTimeForRepresentation(
       MPDParser *pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo,
       double nCurrMSeconds, bool bStartOfPlayback, uint64& nFirstAvailableSegmentStartTime, uint64 &nFirstAvailableSegmentEndTime);

    virtual HTTPDownloadStatus GetLastAvailableSegmentTimeForRepresentation(
       MPDParser *pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, double nCurrMSeconds, uint64& nLastAvailableSegmentStartTime, uint64& nLastAvailableSegmentEndTime);

    virtual HTTPDownloadStatus GetAllSegmentsForRepresentationRange(
      MPDParser* rMPDParser, double currMSeconds, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, uint64* pSegmentInfo, uint32& numSegments,
      uint64 segStartTime,uint64 segEndTime, double &firstAvailableSegmentStartTime);

    virtual HTTPDownloadStatus GetLastSegmentKeyForRepresentation(
       MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, uint64* pSegmentInfo, RepresentationInfo* pRepInfo );

    virtual void AdjustSegmentKeys(RepresentationInfo* pOldRep, RepresentationInfo* pNewRep, uint64 newPeriodKey, int& nTimeOffsetSegment, bool bFirstNewPeriod);

    virtual Type GetType() const;

    virtual void PrintMPDInfoForRepresentation(MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo);

    virtual HTTPDownloadStatus GetSegDurationForRepresentation(MPDParser *pMPDParser, RepresentationInfo* pRepInfo, uint64 nSegKey, uint64& segDuration);

    bool GenerateSegmentInfoFromTemplate(
       MPDParser* pMPDParser, SegmentInfo* pSegmentInfo, RepresentationInfo* pRepInfo,PeriodInfo* pPeriodInfo, uint64 nSegMPDKey);

  private:
    /**
     * @param tsbToUse      How much back from the current time to
     *                      go.
     *
     * @return HTTPDownloadStatus
     */
    HTTPDownloadStatus GetAvailabilityBoundsForSegDurationFromTemplate(
      MPDParser* rMPDParser,
      int64& firstAvailableSegmentIndex,
      int64& lastAvailableSegmentIndex,
      PeriodInfo* pPeriodInfo,
      double segDuration, // ms
      double currMSeconds,
      double minUpdatePeriod,
      double tsbToUse);

    double CalculateSegmentDurationFromStoredTemplate(RepresentationInfo* rRepInfo);


  };


  class SegmentFuncDefault : public SegmentFuncBase
  {
  public:
    SegmentFuncDefault() { m_Type = SegmentFuncBase::SEGMENT_TYPE_DEFAULT; }
    virtual ~SegmentFuncDefault() { }

   virtual HTTPDownloadStatus GetFirstAvailableSegmentTimeForRepresentation(
       MPDParser *pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo,
       double nCurrMSeconds, bool bStartOfPlayback, uint64& nFirstAvailableSegmentStartTime, uint64 &nFirstAvailableSegmentEndTime);

    virtual HTTPDownloadStatus GetLastAvailableSegmentTimeForRepresentation(
       MPDParser *pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, double nCurrMSeconds, uint64& nLastAvailableSegmentStartTime, uint64& nLastAvailableSegmentEndTime);

    virtual HTTPDownloadStatus GetAllSegmentsForRepresentationRange(
      MPDParser* rMPDParser, double currMSeconds, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, uint64* pSegmentInfo, uint32& numSegments,
      uint64 segStartTime,uint64 segEndTime, double &firstAvailableSegmentStartTime);

    virtual HTTPDownloadStatus GetLastSegmentKeyForRepresentation(
       MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, uint64* pSegmentInfo, RepresentationInfo* pRepInfo );

    virtual void AdjustSegmentKeys(RepresentationInfo* pOldRep, RepresentationInfo* pNewRep, uint64 newPeriodKey, int& nTimeOffsetSegment, bool bFirstNewPeriod);

    virtual Type GetType() const;

    virtual void PrintMPDInfoForRepresentation(MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo);

    virtual HTTPDownloadStatus GetSegDurationForRepresentation(MPDParser *pMPDParser, RepresentationInfo* pRepInfo, uint64 nSegKey, uint64& segDuration);
  };


  SegmentFuncBase* GetSegmentFunc()
  {
    return m_pSegmentFunc;
  }

  void SetSegmentFuncToTemplate(bool bVal)
  {
    if(bVal)
    {
      m_pSegmentFunc = &m_SegmentFuncTemplate;
      m_bIsSegmentFuncInitialized = true;
    }
    else
    {
      m_pSegmentFunc = &m_SegmentFuncDefault;
      m_bIsSegmentFuncInitialized = false;
    }
  }

  bool IsSegmentFuncInitializedToTemplate()
  {
    return m_bIsSegmentFuncInitialized;
  }

  /*
  * @brief
  * Returns the bandwidth for the representation(bps)
  */
  RepresentationInfo();
  RepresentationInfo& operator=(const RepresentationInfo&);
  RepresentationInfo(const RepresentationInfo &);
  void Copy(const RepresentationInfo &rep_info);

  ~RepresentationInfo();

  double getFrameRate();

  uint64 getKey();

  char *getRepIdentifier();

  uint32 getBandwidth();

  /*
  * @brief
  * Returns the audio/video codec information for the representation
  * Returns null if information is not present.
  */
  bool getCodec(CodecInfo* pCodecs,int& numCodecs);

  /*
  * @brief
  * Returns the resolution information for the representation.
  * Returns NULL if codec information is not present.
  */
  Resolution* getResolution();


  void setSegmentInfo(SegmentInfo* pSegmentInfo,uint32 numSegments,uint32 size);

  SegmentInfo* getSegmentInfo(uint32& numsegments,uint32& size);

  /**
   * Retrieve the segmentInfo from the segment url.
   */
  SegmentInfo* GetSegmentInfoByURL(const char *url);

  /**
   * Retrieves segment info from the segmentStartTime. Since,
   * segment start times are computed by dividing by timescale,
   * there is a possibility of rounding error. So, for comparing
   * against 2 startTimes, equality is assumed if the difference
   * between them is less than 0.001ms. That is a fragment will
   * not be of duration less than 0.001ms.
   */
  SegmentInfo* GetSegmentInfoByStartTime(double segmentStartTime);

  char* GetByteRangeURLTemplate();

  void SetByteRangeURLTemplate(char* pByteRangeTemplate);

  void SetInitialisationSegmentUrl(char *base_url, char *url);

  void SetInitialisationSegmentRange(int64 nStartOffset, int64 nEndOffset, char *range);

  void SetIndexSegmentRange(char *indexUrlRange);

  char* GetIndexSegmentRange();

  char* GetInitialisationSegmentUrl();

  char* GetInitialisationSegmentRange();

  void GetRangeInitialisationSegment(int64& nStartOffset, int64& nEndOffset);

  void setRepInfo(char *ident,uint32 bandwidth,Codecs* codec,
                  const char *mimeType,
                  Resolution *presolution,char *language,
                  uint32 parX,uint32 parY,uint64 key,
                  uint32 frameRate,int numChannels,
                  double *pChannels, int numSamplingRates,
                  double *pSamplingRates,
                  bool bSegmentAligned, int sapVal,int subSegSAP,
                  char*  mediaStreamStructureId);
  void UpdatePeriodKey(uint64 periodKey);
  bool InitialiseSegmentBase();
  SegmentBaseType* GetSegmentBase();
  bool InitialiseSegmentTemplate();
  bool InitializeSegmentTimeline(int numSEntry);
  bool InitialiseSegmentList();
  bool InitialiseSegmentUrl(int numSegUrl);
  SegmentListType* GetSegmentList();
  bool InheritSegmentBaseInfo(SegmentBaseType* segment_base);
  bool InheritSegmentTemplate(SegmentTemplateType* segment_template);
  bool InheritSegmentList(SegmentListType *segment_list);
  void SetSegmentBaseInfo(uint32 timeScale, uint64 presentationTimeOffset,
                     char *indexRange,bool indexRangeExact,
                     URLType *initialisation, URLType *representationIndex);
  void SetSegmentTemplateInfo(char *mediaTemplate,char *indexTemplate,
                         char *initialisationTemplate, char *bitstreamSwitchingTemplate);
  void SetMultiSegmentBaseInfo( uint32 segmentDuration, uint32 startNumber,
                                uint32 timeScale, uint64 presentationTimeOffset,
                                char *indexRange,bool indexRangeExact,
                                URLType *initialisation, URLType *representationIndex);
  void SetRepresentationIndex(URLType *representationIndex);
  bool SetSegmentTimeline(uint32 segTimelineIndex, uint64 startTime,
                          uint64 duration, int32 repeatCount);
  bool SetSegmentUrl(uint32 urlIndex, char *mediaUrl,
                         char *mediaRange, char *indexUrl, char *indexRange);
  bool SetMediaStreamStructureId(char *mediaStructID);
  uint32 GetMediaStreamStructureId(uint32 index);
  bool CompareStreamStructId(char * structId1,char * structId2);
  SegmentTemplateType *GetSegmentTemplate();
  int GetNumSegmentTimelineEntry();
  uint32 GetNumMediaStreamStructEntry();

  bool IsSegmentTemplateFound()
  {
    return m_bSegmentTemplateFound;
  }
  void SetSegmentAlinged(bool bIsSegmentAlinged)
  {
    m_bSegmentAlinged = bIsSegmentAlinged;
  }
  bool IsSegmentAlinged()
  {
    return m_bSegmentAlinged;
  }
  void SetMediaStreamStructMatchedWithOtherRep(bool bIsMatched)
  {
    m_bMediaStreamStructMatchedWithOtherRep = bIsMatched || m_bMediaStreamStructMatchedWithOtherRep;
  }
  bool IsMediaStreamStructMatchedWithOtherRep()
  {
    char *repId = m_RepId;
    return m_bMediaStreamStructMatchedWithOtherRep;
  }
  void SetStartWithSAPVal(int sapVal)
  {
    m_nStartWithSAPVal = sapVal;
  }
  int GetStartWithSAPVal()
  {
    return m_nStartWithSAPVal;
  }
  void SetSubSegStartWithSAPVal(int subsegSAPVal)
  {
    m_nSubSegmentStartsWithSAP = subsegSAPVal;
  }
  int GetSubSegStartWithSAPVal()
  {
    return m_nSubSegmentStartsWithSAP;
  }
  bool IsSegmentBaseFound()
  {
    return m_bSegmentBaseFound;
  }
  bool IsSegmentListFound()
  {
    return m_bSegmentListFound;
  }

  uint64 GetPTSOffset()
  {
    return m_CachedPTSOffset;
  }

  void SetPTSOffset(double ptsOffset)
  {
    m_CachedPTSOffset = (uint64)(ptsOffset*1000);
  }

  bool GetContentDescriptor(ContentDescriptorType& rCdc, const char *descName);

  /**
   * Get a reference to the ContentDescriptors that may be
   * specified at the adaptation-set level as well as the
   * representation-level eg "AudioChannelConfiguration").
   */
  ContentDescriptorContainer& GetCommonDescriptors();

  ContentProtectionType& GetContentProtection();

  /**
   * Get a reference to the StringValues that may be
   * specified at the adaptation-set level as well as the
   * representation-level eg "mimeType").
   */
  StringValueContainer& GetCommonStringValues();

  /**
   * Set the 'selectable' flag on this representation. Reset the
   * 'selected' flag as marking a representation as selectable
   * means we are preparing to select representations.
   */
  void MarkSelectable(bool bIsSelectable);

  /**
   * Check if a representation is 'selectable'.
   */
  bool IsMarkedSelectable() const;

  /**
   * Marks the representation as 'selected'. The caller should
   * have ensured that the representation was already marked
   * 'selectable'.
   */
  void MarkSelected(bool bSelectRep);

  /**
   * Check if the representation is marked as 'selected'.
   */
  bool IsMarkedSelected() const;

  static const int INVALID_REPRESENTATION_ID = -1;
  HTTPBandwidthEstimator *m_bEstimator;


  /*
  * Set server response i.e. if server honors byte range header
  * as part of original url or uses a byterangeURL since it
  * dishonors byte range header (for http1.0)
  */
  void SetByteRangeURLResp(const HTTPCommon::ByteRangeURLRespState eByteRangeURLrsp);

  /*
  * Get server response for byterangeURL used/not used
  */
  HTTPCommon::ByteRangeURLRespState getByteRangeURLResp();

  bool SetBaseURL(const char *baseUrl)
  {
    bool rslt = false;

    if (baseUrl)
    {
      if (m_BaseURL)
      {
        QTV_Free(m_BaseURL);
      }

      size_t reqdSize = 1 + std_strlen(baseUrl);
      m_BaseURL = (char *)QTV_Malloc(reqdSize * sizeof(char));
      if (m_BaseURL)
      {
        std_strlcpy(m_BaseURL, baseUrl, reqdSize);
        rslt = true;
      }
    }

    return rslt;
  }

  char *GetBaseURL() const
  {
    return m_BaseURL;
  }

  double GetMinUpdatePeriod()
  {
    return m_nMinUpdatePeriod;
  }

  void SetMinUpdatePeriod(double minUpdatePeriod)
  {
    m_nMinUpdatePeriod = minUpdatePeriod;
  }

  double GetTimeShiftBufferDepth()
  {
    return m_nTimeShiftBufferDepth;
  }

  void SetTimeShiftBufferDepth(double timeShiftBufferDepth)
  {
    m_nTimeShiftBufferDepth = timeShiftBufferDepth;
  }

private:
  char **m_url;
  int m_nNumurls;
  uint32 m_nbandwidth;
  Codecs *m_codec;
  Resolution* m_resolution;
  uint32 m_nNumSegments;
  double m_nRefreshTime;
  uint32 m_nSize;
  SegmentInfo *m_pSegmentInfo;
  bool m_bIsAvailable;
  double m_nFrameRate;
  uint32 *m_SamplingRate;
  int m_nNumSamplingRates;
  double *m_Channels;
  int m_nNumChannels;
  uint32 m_nParX;
  uint32 m_nParY;
  char *m_language;
  char *m_MimeType;
  uint32 m_nRepIndex;
  uint64 m_nKey;
  char *m_RepId;
  char *m_InitialisationSegmentUrl;
  char *m_InitialisationSegmentRange;
  int64 m_nStartOffsetInitSeg;
  int64 m_nEndOffsetInitSeg;
  char* m_pIndexSegRange;
  bool m_bSegmentAlinged;
  bool m_bMediaStreamStructMatchedWithOtherRep;
  SegmentTemplateType *m_pSegmentTemplate;
  bool m_bSegmentTemplateFound;
  MediaStreamStructId *m_MediaStreamStructureId;
  uint32 m_nNumMediaStreamStructEntry;
  int m_nStartWithSAPVal;
  int m_nSubSegmentStartsWithSAP;
  SegmentBaseType *m_pSegmentBase;
  bool m_bSegmentBaseFound;
  SegmentListType *m_pSegmentList;
  uint32 m_nNumSegURL;
  bool m_bSegmentListFound;
  uint64 m_CachedPTSOffset;

  ContentDescriptorContainer m_CommonDescs;
  ContentProtectionType m_ContentProtection;
  StringValueContainer m_CommonStringValues;

  // true if representation may be selected
  bool m_bIsSelectable;

  // true if the representation is selected for use.
  bool m_bIsSelected;
  char *m_pByteRangeURLTemplate;
  char *m_BaseURL;

  SegmentFuncTemplate m_SegmentFuncTemplate;
  SegmentFuncDefault  m_SegmentFuncDefault;
  SegmentFuncBase* m_pSegmentFunc;
  bool m_bIsSegmentFuncInitialized;

  friend class SegmentFuncTemplate;
  friend class SegmentFuncDefault;

  double m_nTimeShiftBufferDepth;
  double m_nMinUpdatePeriod;

};
/*
This class stores the information about media segments.
*/
class SegmentInfo
{
public:
  SegmentInfo();
  SegmentInfo& operator=(const SegmentInfo& segment_info);
  ~SegmentInfo();
  void Copy(const SegmentInfo &segment_info);
  /*
  * @brief
  * Returns the current url for the segment
  */
  char* GetURL();

  /*
  * @brief
  * Returns therange of current url for the segment
  */
  char* GetUrlRange();

  /*
  * @brief
  * Retunrs the Index Url for the segment
  */
  char* GetIndexURL();

  bool IsIndexURLPresent()
  {
    return m_bIsIndexURLPresent;
  }

  bool IsIndexRangeExact()
  {
    return m_bIsIndexRangeExact;
  }

  bool IsInitURLPresent()
  {
    return m_bIsInitURLPresent;
  }

  /*
  * @brief
  * Retunrs the Range of Index Url for the segment
  */
  char* GetIndexUrlRange();

  char* GetInitUrlRange();

  /*
  * @brief
  * Returns the start time for the segment(in seconds)
  */

  double getStartTime();
  /*
  * @brief
  * Returns the duration for the segment (in seconds)
  */
  double getDuration();
  /*
  * @brief
  * Returns the availability time for the segment
  */
  double getAvailabilitytime();
  /*
  * @brief
  * Returns true if there is a discontinuity between this segment and the previous segment
  */
  bool isDiscontinuous();
 /*
  * @brief
  * Returns key for the current segment
  */
  uint64 getKey();

  void SetStartTime(uint32 time);
  void SetAvailabilityTime(double time);
  void MarkLastSegment();
  void MarkDeleted();
  bool IsAvailableForUse();
  void MarkRemovedFromServer();
  void MarkAvailableForUse();
  bool IsLastSegment();
  bool IsAvailableOnServer();

  void SetContentLength(const int64 nContentLength);
  int64 GetContentLength();

  void SetInfo(char* url,double duration,uint64 key,double start_time,
                 bool isdiscontinuous=false,
                 bool isencrypted=false);

  void SetInfoBase(char* url, double duration,uint64 key,double start_time,
               char* mediaRange = NULL, char* indexUrl = NULL, bool isIndexURLPresent = false,
               char* indexRange = NULL, bool isIndexRangeExact = false, bool isInitURLPresent = false, char* initRange = NULL,
               bool isdiscontinuous=false, bool isencrypted=false);

  bool SetInfoURL(char* base_url, char *url, char* mediaRange,
                  char* indexUrl, bool isIndexURLPresent, char* indexRange, bool isIndexRangeExact,
                  bool isInitURLPresent, char* initRange, double duration,uint64 key,
                  double start_time,bool isdiscontinuous=false,
                  bool isencrypted=false);

  void UpdatePeriodKey(uint64 periodKey);

  /**
   * Update the internal key (unique key for segments in the rep)
   * on an mpd update.
   */
  void SetKey(uint64 key);

  void Print();
  HTTPBandwidthEstimator *m_bEstimator;

   /*
  * Set server response i.e. if server honors byte range header
  * as part of original url or uses a byterangeURL since it
  * dishonors byte range header (for http1.0)
  */
  void SetByteRangeURLResp(const HTTPCommon::ByteRangeURLRespState eByteRangeURLrsp);

  /*
  * Get server response for byterangeURL used/not used
  */
  HTTPCommon::ByteRangeURLRespState GetByteRangeURLResp();

  void SetByteRangeURL(char* pByteRangeURL);

  char* GetByteRangeURL();

private:
  double m_nStarttime;
  double m_nDuration;
  double m_nAvailabilitytime;
  char *m_url;
  char* m_pMediaRange;
  char* m_pIndexUrl;
  char* m_pIndexRange;
  char* m_pInitRange;
  uint64 m_key;
  bool m_bdiscontinuous;
  /* Indicates whether the segment is available on server or not */
  bool m_bIsAvailableOnServer;
  /*Indicates whether client issued a delete on the segment */
  bool m_bIsDeleted;
  /* Indicates if the segment is the last segment in the representation */
  bool m_bIsLastSegment;
  bool m_bIsAvailableForUse;
  bool m_bAvailibilityTimeSet;
  bool m_bIsInitializationSegment;
  int64 m_nContentLength;
  MM_HANDLE m_pSegDataLock;
  char* m_pByteRangeUrl;
  HTTPCommon::ByteRangeURLRespState m_eByteRangeURLRespState;
  bool m_bIsIndexURLPresent;
  bool m_bIsIndexRangeExact;
  bool m_bIsInitURLPresent;
};

class PlaylistParser
{
public:
  PlaylistParser( HTTPSessionInfo& pSessionInfo,Scheduler& pScheduler);
  virtual ~PlaylistParser();

  static bool ResolveURI(char *base_uri,char *relative_path,char *segment_url,int& segment_url_len);

protected:
  PlaylistParser();
  PlaylistParser(const PlaylistParser&);
  PlaylistParser& operator=(const PlaylistParser&);
  Scheduler& m_pScheduler;
  HTTPSessionInfo& m_sessionInfo;

  /* @brief: This function will parse the playlist file */
  virtual bool Parse(char* RepresentationText)=0;

  /*
  * @brief - Gets the url of the playlist parser
  * @return - url of the playlist file
  */
  const char* GetURL() const
  {
    return m_url;
  }
  static bool ParseCodecInfo(char* str, Codecs* codec_info,int& majorTypes);
  static bool ParseCodecInfo(char* str, Codecs* codec_info);
  static bool IsHevcCodecPresent(Codecs* codec_info);
  int m_nNumPrograms;
  int current_selected_program;
  int current_selected_representation;
  int current_selected_period;
  char *m_url;
  bool m_bAbortSet;
  bool m_bRepsAvailable;
  PlaylistDownloader *m_pDownloader;
  char *m_pNamespaceKey;
  char *m_pNamespaceVal;

  MPDProfileType m_MpdProfile;
  char *m_MpdProfileStr;
};

/*
 * This class stores the information about SegmentBase.
 */

class SegmentBaseType
{
public:
  SegmentBaseType();
  virtual ~SegmentBaseType();
  void SetTimeScale(uint32 timescale);
  void SetPresentationOffset(uint64 presentationoffset);
  void SetIndexRange(char *indexrange);
  void SetIndexRangeExact(bool indexrangeexact);
  void SetInitialisation(URLType *initialisation);
  void SetRepresentationIndex(URLType *repindex);
  uint32 GetTimeScale();
  uint64 GetPresentationOffset();
  char* GetIndexRange();
  bool GetIndexRangeExact();
  URLType* GetInitialisation();
  URLType* GetRepresentationIndex();
  void InheritSegmentBase(SegmentBaseType *segment_base);
  void Copy(SegmentBaseType *pRHS);
private:
  uint32 m_nTimeScale;
  uint64 m_nPresentationTimeOffset;
  char  *m_pIndexRange;
  bool  m_bIndexRangeExact;
  URLType *m_pInitialisation;
  URLType *m_pRepresentationIndex;
};

/*
 * This class stores the information about MultipleSegmentBase.
 */

class MultipleSegmentBaseType : public SegmentBaseType
{
public:
  MultipleSegmentBaseType();
  virtual ~MultipleSegmentBaseType();
  void SetSegmentTimeline( uint32 segmentTimelineIndex,
                           uint64 startTime,
                           uint64 duration,
                           int32 repeatCount);
  bool InitializeSegmentTimeline(int numSEntry);
  bool IsSegmentTimelineFound();
  int GetNumSegmentTimelineEntry();
  void SetDuration(uint32 duration);
  void SetStartNumber(uint32 startnumber);
  uint32 GetDuration();
  uint32 GetStartNumber();
  uint64 GetSegTimeLineStartTime(uint32 segmentTimelineIndex);
  uint64 GetSegTimeLineDuration(uint32 segmentTimelineIndex);
  int32 GetSegTimeLineRepeatCount(uint32 segmentTimelineIndex);

  void InheritSegmentBaseInfo(SegmentTemplateType *segment_template);
  void InheritSegmentBaseInfo(SegmentListType *segment_list);
 private:
   uint32 m_nDuration;
   uint32 m_nStartNumber;
   SegmentTimelineType *m_pSegmentTimeline;
   int m_nNumSegmentTimeline;
   URLType *m_pBitstreamSwitching;
   bool m_bSegmentTimelineFound;
};

/*
 * This class stores the information about SegmentTimeline.
 */

class SegmentTimelineType
{
public:
  SegmentTimelineType();
  ~SegmentTimelineType();
  void SetStartTime(uint64 starttime);
  void SetDuration(uint64  duration64);
  void SetRepeatCount(int32 repeatcount);
  uint64 GetStartTime();
  uint64 GetDuration();
  int32 GetRepeatCount();
private:
  uint64 m_nStartTime;
  uint64 m_nDuration;
  int32 m_nRepeatCount;
};

/*
 * This class stores the information about SegmentTemplate.
 */

class SegmentTemplateType : public MultipleSegmentBaseType
{
public:
  SegmentTemplateType();
  virtual ~SegmentTemplateType();
  void SetMediaTemplate(char *media);
  void SetIndexTemplate(char *index);
  void SetInitialisationTemplate(char *initialisation);
  void SetBSSwitchingTemplate(char *bsSwiching);
  char *GetMediaTemplate();
  char *GetIndexTemplate();
  char *GetInitialisationTemplate();
  char *GetBSSwitchingTemplate();

  void InheritSegmentTemplateInfo(SegmentTemplateType *segment_template);
  void InheritMultiSegmentBaseInfo(SegmentTemplateType *segment_template);

  void Copy(SegmentTemplateType *pRHS);

private:
  char *m_pMedia;
  char *m_pIndex;
  char *m_pInitialisation;
  char *m_pBitStreamSwitching;
  bool m_bIsSegmentTimelineFound;
};


class SegmentURLType
{
public:
  SegmentURLType();
  ~SegmentURLType();
  void SetMediaUrl(char *mediaUrl);
  void SetMediaRange(char *mediaRange);
  void SetIndexUrl(char *indexUrl);
  void SetIndexRange(char *indexRange);
  char* GetMediaUrl();
  char* GetMediaRange();
  char* GetIndexUrl();
  char* GetIndexRange();
private:
  char* m_pMediaUrl;
  char* m_pMediaRange;
  char* m_pIndexUrl;
  char* m_pIndexRange;
};

/*
 * This class stores the information about SegmentList.
 */

class SegmentListType : public MultipleSegmentBaseType
{
public:
  SegmentListType();
  ~SegmentListType();
  bool InitialiseSegmentUrl(int numSegmentUrl);
  void InheritMultiSegmentBaseInfo(SegmentListType *segment_list);
  void InheritSegmentUrl(SegmentListType* segment_list, int segUrlIndex);
  void SetSegmentUrl(uint32 segUrlIndex, char* mediaUrl,
                     char* mediaRange,char* indexUrl,char* indexRange);
  SegmentURLType *GetSegmentUrl(int urlIndex);
  int GetNumSegmentUrl()
  {
    return m_nNumSegmentUrl;
  }
  void SetNumSegmentUrl(int numSegmentUrl)
  {
  m_nNumSegmentUrl = numSegmentUrl;
  }
  void Copy(SegmentListType *pRHS);

private:
  SegmentURLType *m_pSegmentUrl;
  int m_nNumSegmentUrl;
};

}/* namespace video */
#endif /* __PLAYLISTPARSER_H__ */
