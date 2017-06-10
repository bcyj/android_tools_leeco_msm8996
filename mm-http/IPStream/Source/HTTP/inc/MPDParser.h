#ifndef __MPDPARSER_H__
#define __MPDPARSER_H__
/************************************************************************* */
/**
 * MPDParser.h
 * @brief Header file for MPDParser.
 *
 * COPYRIGHT 2011-2013 QUALCOMM Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/MPDParser.h#75 $
$DateTime: 2013/07/28 21:38:43 $
$Change: 4175800 $

========================================================================== */
/* =======================================================================
**               Include files for MPDParser.h
** ======================================================================= */
#include "PlaylistParser.h"
#include "tinyxml.h"
#include "MMTime.h"
#include "IPStreamSourceUtils.h"
#include "StreamSourceTimeUtils.h"

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
  class MPDParser;
/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
/* This is the MPD Parser iFace which can be used to extract information about
 * all the elements in MPD
 */
//TODO: Add function headers and comments
class iMPDParser
{
public:
  virtual ~iMPDParser() { };

  virtual bool RegisterMPDUpdateNotification(void (*fNotification)(void *), void *pPrivData) = 0;

  virtual HTTPDownloadStatus GetNextPeriod(PeriodInfo*, bool& bEOS)=0;

  virtual bool GetCurrentPeriod(PeriodInfo&)=0;


  virtual bool GetRepresentationByKey(uint64 key,RepresentationInfo*&)=0;

  virtual HTTPDownloadStatus GetAllRepresentationsForGroup(RepresentationInfo*,uint32& numReps,uint64 GrpKey)=0;

  virtual HTTPDownloadStatus GetAllRepGroupForPeriod(RepresentationGroup*, uint32& numGroups,uint64 period_key, bool bReturnSelectedOnly = true)=0;

  virtual HTTPDownloadStatus GetMaxBitrateForPeriod(uint64 period_key, uint32& maxBitRate) = 0;


  virtual HTTPDownloadStatus GetAllSegmentsForRepresentationRange(uint64*, uint32& numSegments,uint64 repKey, uint64 segStartTime, uint64 segEndTime,double &firstAvailableSegmentStartTime)=0;

  virtual HTTPDownloadStatus GetLastAvailableSegmentTimeForRepresentation(uint64 repKey, uint64& nLastAvailableSegmentStartTime, uint64& nLastAvailableSegmentEndTime)=0;

  virtual HTTPDownloadStatus GetLastSegmentKeyForRepresentation(uint64*, uint64 repKey)=0;

  virtual bool GetSegmentInfoByKey(uint64 key,SegmentInfo&)=0;

  virtual bool GetTotalDuration(uint64& start,uint64& end)=0;

  virtual bool GetEndTime(uint64& end) = 0;

  virtual bool IsLive() = 0;

  virtual bool IsLiveContentHostedLocally() = 0;

  virtual uint32 GetTimeShiftBufferDepth() const = 0;

  virtual MPDProfileType GetMPDProfile() = 0;

  virtual double GetMinBufferTime() = 0;

  virtual bool IsSegmentAvailable(uint64 nKey) = 0;

  virtual bool IsLastPeriod(uint64 nKey) = 0;

  /**
   * Check if key from PeriodInfo corresponds to the last period.
   */
  virtual bool IsLastSegment(uint64 nKey) = 0;

  virtual double GetDuration(const uint64 nRepKey) = 0;

  virtual void ForceRefreshPlaylist() = 0;

  virtual void GetMPDText(char *pMPDtext, uint32& size) = 0;

  /**
   * Constructs an xml string of buffer size 'size' of the
   * properties in the mpd. 2 ways to use this function:
   *  (i) Set pString to null to query the reqd buffer size
   *  'size'.
   *  (ii) Pass non-null pString of buffer-size 'size' to populate
   *    pString.
   *  To filter a subset of property-names, include the optional
   *  argument 'propertyNames' as a string containing
   *  property-names delimited by '#'. Example
   *  "ContentProtection#Rating". This is a TO DO and current
   *  support is for "ContentProtection" only.
   */
  virtual void GetPropertiesXML(char *pString, int& size, const char *propertyNames = "") = 0;

  /**
   * Parses the xml and marks all the specified representations as
   * selectable. And then, marks a subset of the 'selectable'
   * representations as 'selected'.
   */
  virtual bool SetSelectionsXML(const char *pXMLSelectedString) = 0;


  virtual HTTPDownloadStatus GetCurrentPlayingRepInfo(int periodKey,
                                                      int grpKeyAudio, int grpKeyVideo, int grpKeyText,
                                                      int repKeyAudio, int repKeyVideo, int repKeyText,
                                                      int &currRepSize, char *currRepSring) = 0;

  virtual void FillPeriodXMLProperties(PeriodInfo &periodInfo,
                                       int periodKey,
                                       int &currRepSize,
                                       char *currRepSring) = 0;

  virtual void FillGrpXMLProperties(RepresentationGroup &repGrpInfo,
                                    int &currRepSize,
                                    char *currRepSring) = 0;

  virtual void FillRepXMLProperties(RepresentationInfo &repInfo,
                                    int &currRepSize,
                                    char *currRepSring) = 0;



  virtual bool GetRepGrpKeysForAdaptationSetChange(uint64 nPeriodKey,
                                                   const char *pXMLSelectedString,
                                                   IPStreamList<uint64>& repGrpXMLKeySet) = 0;

  /**
   * Modify the track selections due to dynamic adaptation-set
   * changes.
   */
  virtual bool ModifySelectedRepresentations(uint32 nPeriodIdentifer,
                                             uint32 nAdaptationSetIdentifier,
                                             IPStreamList<uint32>& listRepIds,
                                             bool bIsSelected) = 0;

  /**
   * Determine if the rep associated with nRepKey is of VOD
   * profile.
   */
  virtual bool IsRepVODProfile(uint64 nRepKey) = 0;
};

/* This is the MPD Handler iFace used by DashDownloadHelper */
class iMPDHandler
{
public:
  virtual ~iMPDHandler() { };
  virtual void Open(char *url) = 0;
  virtual void Close() = 0;
  /* Returns the MPDParser iFace which can be used to extract information
   *  about MPD elements */
  virtual iMPDParser* GetMPDParserIface() = 0;
};

class MPD
{
public:
  MPD();
  MPD& operator=(const MPD&);
  MPD(const MPD &);
  ~MPD();
  void Copy(const MPD &);
  PeriodInfo* getPeriodInfo(int& numPeriods);

  /**
   * Retrieve the period info from the period key
   */
  PeriodInfo* InitializePeriodInfo(int numperiods=0);
  bool AddPeriod(int& PeriodIndex);
  bool RemovePeriod();
  bool CommitPeriodInfo();
  bool IsValidMPD();
  uint32 getNumPeriods() const;
  double getDuration() const;
  double getMinimumUpdatePeriod() const;
  double getMinBufferTime() const;
  double getTimeShiftBufferDepth() const;
  double GetAvailabilityStartTime() const;

  void SetMPDInfo(double duration, double bufferTime, double timeShiftBufferDepth,
                  double minUpdatePeriod,double availStartTime);

  void setMinimumUpdatePeriod(double minUpdatePeriod);
  void setTimeShiftBufferDepth(double timeShiftBufferDepth);

  //To cache and retrieve the orignal values (non-capped) of minUpdatePeriod and timeShiftBufferDepth

  void setMinimumUpdatePeriodCache(double minUpdatePeriod);
  void setTimeShiftBufferDepthCache(double timeShiftBufferDepth);

  double getMininumUpdatePeriodCache() const;
  double getTimeShiftBufferDepthCache() const;

  uint64 getStartTime();

  /**
   * Check if key from PeriodInfo corresponds to the last period.
   */
  bool IsLastPeriod(uint64 nPeriodKey);

  /**
   * True if first time parsing mpd.
   */
  bool IsMpdValid() const;

  /**
   * Set the flag that mpd is live.
   */
  void SetLive();

  /**
   * Check if mpd is live.
   */
  bool IsLive() const;

private:
  static const int MIN_PERIOD_ARR_SIZE = 10;
  PeriodInfo *m_pPeriodInfo;
  PeriodInfoCache *m_pPeriodInfoCache;
  bool ResizePeriodInfo(int newSize);
  int m_nNumPeriods;
  double m_nDuration;
  double m_nMinBufferTime;
  double m_nTimeShiftBufferDepth;
  double m_nTimeShiftBufferDepthCache;
  double m_nMinimumUpdatePeriod;
  double m_nMinimumUpdatePeriodCache;
  int m_nPeriodArrSize;
  double m_AvailabilityStartTime;
  bool m_bIsLive;
};

class MPDParser:public PlaylistParser,public iMPDHandler, public iMPDParser
{
public:

  bool CheckSegmentAvailability(SegmentInfo& rSegInfo, double currMSeconds);

  /*
   *  c'tor Create an instance of MPDParser
   * @param Scheduler
  */
  MPDParser(HTTPSessionInfo& sessionInfo,Scheduler& pScheduler, HTTPStatusHandlerInterface* pStackNotificationHandler, uint32 nRequestIDfromResolver,
            HTTPStackInterface& HTTPStack);
  /*
    *  d'tor destroys mpd parser instance
  */
  virtual ~MPDParser();

  /**
   * Store the time the mpd is fetched from the server.
   */
  void SetFetchTime();

  /**
   * Retrive the time the MPD was last fetched from the server.
   */
  MM_Time_DateTime GetFetchTime() const;

  virtual bool GetTotalDuration(uint64& start,uint64& end);
  virtual bool GetEndTime(uint64& end);

  iMPDHandler* GetMPDHandler()
  {
    return static_cast<iMPDHandler *>(this);
  }

  virtual iMPDParser* GetMPDParserIface()
  {
    return static_cast<iMPDParser *>(this);
  }

  virtual void Open(char *url);

  /*
   * @brief - Sets the url for MPDParser
   * @param[in] - url of the playlist file
  */
  virtual void SetURL(char *url);

  /*Closes MPDParser */
  virtual void Close();

  virtual bool RegisterMPDUpdateNotification(void (*fNotification)(void *), void *pPrivData);

  virtual HTTPDownloadStatus GetNextPeriod(PeriodInfo*, bool& bEOS);

  virtual bool GetCurrentPeriod(PeriodInfo&);

  /**
   * Calculate the offset from the mpd@availabilityStartTime that
   * should be used to start playback.
   */
  virtual double GetOffsetFromAvailabilityTimeToStartPlayback(bool bStartOfPlayback);

  /**
   * Calculate the offset from the mpd@availabilityStartTime.
   */
  virtual double GetOffsetFromAvailabilityTime();

  /**
   * @brief
   *  Search the playlist for the periodhich falls within
   *  starttime and update the current period to that.
   *
   * @return HTTPDownloadStatus
   */
  virtual HTTPDownloadStatus InitializePlaylistForStartTime(
    PeriodInfo& periodInfo, bool& bEOS, int64 startTime, bool bSearchFromStartOfPlaylist);


  virtual bool GetRepresentationByKey(uint64 key,RepresentationInfo*&);

  virtual HTTPDownloadStatus GetAllRepresentationsForGroup(RepresentationInfo*,uint32& numReps,uint64 GrpKey);

  virtual HTTPDownloadStatus GetAllRepGroupForPeriod(RepresentationGroup*, uint32& numGroups, uint64 period_key, bool bReturnSelectedOnly = true);

  virtual HTTPDownloadStatus GetMaxBitrateForPeriod(uint64 period_key, uint32& maxBitRate);


  virtual HTTPDownloadStatus GetAllSegmentsForRepresentationRange(uint64*, uint32& numSegments,uint64 repKey , uint64 segStartTime, uint64 segEndTime,double &firstAvailableSegmentStartTime);

  virtual HTTPDownloadStatus GetLastSegmentKeyForRepresentation(uint64*, uint64 repKey);

  virtual HTTPDownloadStatus GetLastAvailableSegmentTimeForRepresentation(uint64  repKey, uint64& nLastAvailableSegmentStartTime, uint64& nLastAvailableSegmentEndTime);

  virtual bool GetSegmentInfoByKey(uint64 key,SegmentInfo&);

  virtual bool IsLastPeriod(uint64 nKey);

  virtual bool IsLastSegment(uint64 nKey);

  double GetDuration(const uint64 nRepKey);

  virtual bool IsLive()
  {
    bool val = false;
    MM_CriticalSection_Enter(m_pParserDataLock);
    val = mpd ? mpd->IsLive(): false;
    MM_CriticalSection_Leave(m_pParserDataLock);
    return val;
  };

  virtual bool IsLiveContentHostedLocally();

  virtual MPDProfileType GetMPDProfile()
  {
    return  m_MpdProfile;
  }

  virtual uint32 GetTimeShiftBufferDepth() const
  {
    return (uint32) (mpd ? mpd->getTimeShiftBufferDepth() : 0);
  };

  virtual double GetMinBufferTime()
  {
    return (mpd ? mpd->getMinBufferTime() : 0.0);
  };

  virtual bool IsSegmentAvailable(uint64 nKey);

  /**
   * Override the refresh interval for playlist downloading, so
   * that the next playlist is downloaded immediately.
   */
  virtual void ForceRefreshPlaylist();

   virtual void GetMPDText(char *pMPDtext, uint32& size);

  /**
   * Constructs an xml string of buffer size 'size' of the
   * properties in the mpd. 2 ways to use this function:
   *  (i) Set pString to null to query the reqd buffer size
   *  'size'.
   *  (ii) Pass non-null pString of buffer-size 'size' to populate
   *    pString.
   *  To filter a subset of property-names, include the optional
   *  argument 'propertyNames' as a string containing
   *  property-names delimited by '#'. Example
   *  "ContentProtection#Rating". This is a TO DO and current
   *  support is for "ContentProtection" only.
   */
  virtual void GetPropertiesXML(char *ppString, int& size,  const char *propertyNames = "");

  /**
   * Parses the xml and marks all the specified representations as
   * selectable. And then, marks a subset of the 'selectable'
   * representations as 'selected'.
   */
  virtual bool SetSelectionsXML(const char *pXMLSelectedString);

  virtual bool GetRepGrpKeysForAdaptationSetChange(
     uint64 nPeriodKey, const char *pXMLSelectedString, IPStreamList<uint64>& repGrpXMLKeySet);

  /**
   * Modify the track selections due to dynamic adaptation-set
   * changes.
   */
  virtual bool ModifySelectedRepresentations(uint32 nPeriodIdentifer,
                                             uint32 nAdaptationSetIdentifier,
                                             IPStreamList<uint32>& listRepIds,
                                             bool bIsSelected);

  /**
   * Determine if the rep associated with nRepKey is of VOD
   * profile.
   */
  virtual bool IsRepVODProfile(uint64 nRepKey);

  /**
   * Get the first available segment start and end times
   * across all periods.
   */
  HTTPDownloadStatus GetFirstAvailableSegmentTimeForPlayback
  (
    bool bStartOfPlayback,
    uint64& nFirstAvailableSegmentStartTime,
    uint64& nFirstAvailableSegmentEndTime
  );

  /**
   * Get the start time of the first available segment start time
   * for period.
   */
  HTTPDownloadStatus GetFirstAvailableSegmentTimeForPeriod
  (
    uint64 nPeriodKey,
    bool bStartOfPlayback,
    uint64& nFirstAvailableSegmentStartTime,
    uint64& nFirstAvailableSegmentEndTime
  );

  /**
   * Get the last available segment start and end times
   * across all periods.
   */
  HTTPDownloadStatus GetLastAvailableSegmentTimeForPlayback
  (
    uint64& nLastAvailableSegmentStartTime,
    uint64& nLastAvailableSegmentEndTime
  );

  /**
   * Get the last available segment start and end times
   * for period.
   */
  HTTPDownloadStatus GetLastAvailableSegmentTimeForPeriod
  (
    uint64 nPeriodKey,
    uint64& nLastAvailableSegmentStartTime,
    uint64& nLastAvailableSegmentEndTime
  );

  /**
   * Get the end time of the last available segment time for given
   * period. Used to calculate availability of periods at a given time for live case.
   */
  HTTPDownloadStatus GetLastLargestSegmentDurationForPeriod(PeriodInfo& rPeriodInfo, uint64 &nLastMaxSegDuration);

  bool IsMPDAvailable() const;

  bool IsMPDValid() const;

  HTTPDownloadStatus GetContentProtectElem(HTTPDrmType &drmType,uint32 &contentProtectionInfoSize,
                                          unsigned char* contentProtectionData);

  HTTPDownloadStatus GetCurrentPlayingContentProtectElem(HTTPMediaType mediaType,
                                          HTTPDrmType &drmType,
                                          HTTPContentStreamType &streamType,
                                          int &contentProtectionInfoSize,
                                          char* contentProtectionData,
                                          int currPeriodkey, int currGrpKey,
                                          int currRepKey);


  HTTPDownloadStatus GetCurrentPlayingRepInfo(int periodKey,
                                              int grpKeyAudio, int grpKeyVideo, int grpKeyText,
                                              int repKeyAudio, int repKeyVideo, int repKeyText,
                                              int &currRepSize, char *currRepSring);

  HTTPDownloadStatus GetTrackEncodingInfo(FileSourceMnMediaType& audio,
                                          FileSourceMnMediaType& video,
                                          FileSourceMnMediaType& other);

  void FillPeriodXMLProperties(PeriodInfo &periodInfo,
                               int periodKey,
                               int &currRepSize,
                               char *currRepSring);

  void FillGrpXMLProperties(RepresentationGroup &repGrpInfo,
                            int &currRepSize,
                            char *currRepSring);

  void FillRepXMLProperties(RepresentationInfo &repInfo,
                            int &currRepSize,
                            char *currRepSring);


  /**
   * For debugging.
   */
  void PrintMPD();

  void AdjustSegmentKeys(
       MPD& rNewMPD, MPD& rOldMPD, int& nTimeOffsetSegment, int correspondingOldPeriodIdx);

private:
  MPDParser();
  MPDParser(const MPDParser&);
  MPDParser& operator=(const MPDParser&);
  class PlaylistDownloaderTaskParam : public SchedulerTaskParamBase
  {
    public:
    PlaylistDownloaderTaskParam()
    {
      pCbSelf = NULL;
      nStartTime = 0;

    };
    virtual ~PlaylistDownloaderTaskParam(){ };

    void* pCbSelf;
    uint32 nStartTime;
  };
  /*
  * @brief
  * Parses the mpd file and stores the information about programs and representations.
  * @param[in] Representation text - contains the text for the presentation file
  */
  virtual bool Parse(char* PresentationText);

  /**
   * Used to update the unique keys of the new mpd aftercomapring
   * with the previously stored mpd.
   */
  void AdjustKeysForNewMPD(MPD& rNewMPD, MPD& rOldMPD, bool bIsOnTime);

  /**
   * Adjusts the availability time of the mpd to that of the
   * previous mpd in case of dynamic mpd if both the mpd's can be
   * related, that is, there is a period in the new mpd that
   * corresponds to the old one.
   */
  static void AdjustNewMPD(MPD& rNewMPD, MPD& rOldMPD,
                           int& nTimeOffsetPeriods,
                           int& correspondingOldPeriodIdx);

  /**
   * Updates if needed the period keys of new mpd after comparing
   * with the old mpd.
   */
  static void AdjustPeriodKeys(MPD& rNewMPD, MPD& rOldMPD,
                               int nTimeOffsetPeriods,
                               int correspondingOldPeriodIdx);

  /*
   * Scheduler Task to download and update playlist
   */
  static int TaskDownloadAndUpdateMPD( void* pTaskParam);

  void Print();

  static void ConvertDurationTypeToSeconds(char* duration,double& duration_in_sec);

  bool IsEndReached()
  {
    return m_bEndReached;
  }

  bool ParsePeriod(MPD *pMpd, TiXmlElement* PeriodElement, char **baseURLs, char **baseURLByteRangeAttrs, int& numUrls,uint64 PeriodKey,
                   double& prevPeriodStartTime, double& prevPeriodDuration, double currPeriodStartTime, double currPeriodDuration, bool bIsFirstPeriod);
  bool preScanPeriods(MPD *pMpd, TiXmlElement* MPDChildElement, PeriodInfoCache** pPeriodArray, int& numPeriods);
  bool ResizeCachePeriodInfo(PeriodInfoCache** PeriodArray, int& numPeriodElements);
  bool UpdatePeriodInfoFromHead(MPD *pMpd, TiXmlElement* PeriodElement, double& prevPeriodStartTime, double& prevPeriodDuration,
                                PeriodInfoCache* pPeriodInfo, bool bIsFirstPeriod, bool& isPeriodRevisitReq, uint64 PeriodKey);
  bool UpdatePeriodInfoFromTail(MPD *pMpd, PeriodInfoCache* pPeriodInfo, int numPeriods);
  bool ParseGroup(MPD *pMpd, TiXmlElement* GroupElement, char **baseURLs, char **baseURLByteRangeAttrs, int& numUrls,uint64 GrpKey);
  bool ParseRepresentation(MPD *pMpd, TiXmlElement* RepElement, char **baseURLs, char **baseURLByteRangeAttrs, int& numUrls,
                           uint64 repKey,int& bRepType, bool &bBWPresent);
  static bool GetResolvedURL(char* base_url, char *url, char *newUrl,int& new_url_len);
  bool ParseGroupAttributes(MPD& rMpd, TiXmlElement* GroupElement,uint64 GroupKey,RepresentationGroup* repGrp,
                            bool& bXlinkHrefFound,bool& bXlinkActuateFound);
  bool ParseRepAttrs(MPD& rMpd, TiXmlElement* RepElement,uint64 RepKey,RepresentationInfo* rep,
                     RepresentationGroup repGrp,int& bRepType, bool &bBWPresent);
  static bool ParseSegmentInfo(MPD *pMpd, TiXmlElement* SegmentElement, char **baseURLs, int& numUrls,uint64 repKey);
  bool GetSegmentTimeInfo(uint64 segmentKey,double& nStartTime,double& nDuration);
  static bool ConvertStringVectorToList(char* strVector,int& numElements, double* list);

  bool ParseDescriptorTypeElements(TiXmlElement* GroupElement,
                                   ContentDescriptorContainer& rCdc,
                                   const char *adaptationSetDescriptorName);

  void ParseContentProtection(TiXmlElement* GroupElement,
                              ContentProtectionType& rContentProtection);

  bool ParseGetNumberSegmentTimeline(TiXmlElement* Element, int&numSegmentTimeline);
  bool ParseSegmentTimeline(TiXmlElement* Element, int& numSegmentTimeline, SegmentTimelineStuct *segmentTimeline);
  bool ParseSegmentBase(MPD *pMpd, TiXmlElement* Element, uint64 key, ElementType elementType);
  bool ParseMultipleSegmentBaseInfo(TiXmlElement* Element, uint32& segmentDuration, uint32& startNumber);
  bool ParseSegmentBaseInfo(TiXmlElement* Element, uint32& timeScale,
                       uint64& presentationTimeOffset, char** indexRangeBase,
                       bool&  indexRangeExact, URLType **initialisation,
                       URLType **representationIndex);
  bool ParseSegmentList(MPD *pMpd, TiXmlElement* Element, char **baseURLs,uint64 key, ElementType elementType);
  bool ParseSegmentTemplate(MPD *pMpd, TiXmlElement* Element, uint64 key, ElementType elementType);
  void InvokeCallbacks();

  bool IsNextPeriodPresent(TiXmlElement* Element, double &nextPeriodStartTime, double &nextPeriodDuration);
  bool IsNextRepresentationPresent(TiXmlElement* Element);
  void ResetCurrentPeriod();

  /**
   * Returns the offset from avalablity time taking
   * time-shift-buffer into account for the period. If the current
   * time adjusted for tsb is after the availability time, the
   * offset is returned, else zero is returned.
   *
   * @param rMpd
   * @param rPeriodInfo
   *
   * @return double
   */
  double GetOffsetFromAvailabilityTimeForPeriod(const MPD& rMpd, const PeriodInfo& rPeriodInfo, double timeShiftBufferDepth);

  /**
   * replaces $RepresentationID$,$Bandwidth$
   * $Number$, $Time$ with suitable string for
   * template
   */

  void ReplaceIdentifierwithValueInUrl(char *targetUrl, const char *idToReplace, char *replaceString, int origUrlLen);

  /**
   * replaces $RepresentationID%[width]d$,$Bandwidth%[width]d$
   * $Number%[width]d$, $Time%[width]d$ with suitable string for
   * template
   */

  void FormatandReplaceIdentifierwithValueInUrl(char *targetUrl, const char *idToReplace, int value, int origUrlLen);

  /**
   * To check $RepresentationID$,$Bandwidth$
   * $Number$, $Time$ if present
   */
  bool IsTemplateTagPresent( char *targetUrl, const char *idToReplace);

  /**
   * @brief
   *  Update numSegment and segmentIndex based on segment
   *  availability and minimumUpdatePeriod.
   *
   * @param numSegment    out-argument
   * @param segmentIndex  out-argument
   * @param startNumber
   * @param rMpd
   * @param rPeriodInfo
   * @param segDuration
   * @param maxSegments optional argument. If set, then the
   *                    calculated value of numsegment cannot
   *                    exceed this.
   */
  void UpdateNumSegmentsAndSegmentIndex(
    int& numSegment, int& segmentIndex,
    int startNumber, const MPD& rMpd, const PeriodInfo& rPeriodInfo, RepresentationInfo& rRepInfo, double segDuration,
    int maxSegments = -1);

  /**
   * @brief
   *  Number of available segment withing availability limits
   *
   * @param numSegment    out-argument
   * @param rMpd
   * @param rPeriodInfo
   * @param rRepInfo
   * @param timeScale
   * @param maxSegments the
   *                    calculated value of numsegment cannot
   *                    exceed the value of this adjusted for
   *                    availability time.
   */
  void GetNumAvailableSegment(
     int& numSegment, const MPD& rMpd,
     const PeriodInfo& rPeriodInfo,  RepresentationInfo& rRepInfo,
     uint32 timeScale,int maxSegments, int &startNumber, bool bSegmentTemplate = true);

  /**
   * This calculates the amount of time behind the current time
   * for which playback can start.
   */
  uint32 GetTsbToUseAtStartUp() const;
  /**
   * Delegate some initial checks for GetAllSegmentsForRepresentationRange
   * to this function.
   */
  HTTPDownloadStatus GetAllSegmentsForRepresentationRangePreProcess(
    PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, uint64 segStartTime, double currMSeconds);

  /**
   *  Gets the index of the firstAvailableSegmentIndex and
   *  lastAvailableSegmentIndex for segment array for the live
   *  case.
   */
  void GetFirstAndLastAvailableSegmentInfo(int& firstAvailableSegmentIndex,
                                           int& lastAvailableSegmentIndex,
                                           uint32& numberOfAvailableSegments,
                                           SegmentInfo* pSegment,
                                           uint32 nNumSegments,
                                           uint64 periodStartTime,
                                           double currMSeconds,
                                           double minUpdatePeriod,
                                           double timeShiftBufferDepth);

  bool GetRepMajorType(RepresentationInfo& sRep, uint32& nMajorType);

  /**
   * Get the major-type of the adaptation-set.
   */
  bool GetGroupMajorType(RepresentationGroup& sRepGroup,
                         uint32& nMajorType);

  /**
   * This sets the m_IsSelectable flag for each adaptation set as
   * specified by the arg 'bIsSelectable'. The m_bIsSelectable
   * flag in each representation takes on this value as well. Each
   * representation in marked not-selected as well.
   */
  void MarkAllRepGrpsAsSelectable(bool bIsSelectable);

  /**
   * This sets the m_IsSelectable flag for each adaptation set as
   * specified by the arg 'bIsSelectable'. The m_bIsSelectable
   * flag in each representation takes on this value as well. Each
   * representation in marked not-selected as well.
   * Text is not selected as default selection
   */
  void MarkDefaultRepGrpsAsSelectable(bool bIsSelectable);


  /**
   * Helper function for MarkDefaultRepGrpsAsSelectable at period
   * level.
   */
  void MarkDefaultRepGrpsAsSelectableForPeriod(PeriodInfo& rPeriod,
                                               bool bIsSelectable);

  /**
   * Marks the representation specified by
   * (nPeriodIdentifer,nAdaptationSetIdentifier,nRepresentationIdentifier)
   * as selectable.
   */
  void MarkRepresentationAsSelectable(
    uint32 nPeriodIdentifer, uint32 nAdaptationSetIdentifier, uint32 nRepresentationIdentifier);

  /**
   * If any period is found unselected by user choice, this
   * ensures that period is marked for default selection. The
   * application should not be allowed to de-select a period thus
   * leaving a hole in terms of presentation-time. To skip, media
   * the app should be allowed only via SEEK.
   */
  void EnsureAllPeriodsSelected();

  /**
   * This looks at all the representations that are marked
   * selectable and selects at most one audio, video, text
   * adaptation-sets.
   */
  void DoContentSelection();

  class KeyStruct
  {
  public:
    static const int MAGIC_VAL = 0x7fffffff;
    KeyStruct();
    ~KeyStruct();

    KeyStruct(const KeyStruct& rKey);
    KeyStruct& operator=(const KeyStruct& rKey);

  private:
    int m_PeriodKey;
    int m_AdaptationSetKey;
    int m_RepresentationKey;

    void Assign(const KeyStruct& rKey);
    friend class MPDParser;
  };

  /**
   * Parses the xml-element associated with 'MPD' and populates
   * the representation-identifiers into a list.
   */
  bool ParseKeysFromXML(TiXmlElement *elem, IPStreamList<KeyStruct>& keyList);

  /**
   * Parse the value of the "Key" attribute and return the 'int'
   * value.
   */
  bool GetIntValueForKey(TiXmlElement *elem, int& keyValue);

  /**
   * For debugging.
   */
  void PrintSelectedReps();

  void StoreMPDText(const char *MPDText);


  /**
   * Indicates if the URL is LocalHost URL
   */
  bool IsLocalHostURL(const char* url);

  int GetMaxBufferSizeForStringTemplate(RepresentationInfo& rRepInfo, char *urlTemplate);
  void SetBufferForStringTemplate(RepresentationInfo& rRepInfo,
                                  char *urlTemplate, char *dst, int dstBufferSize,
                                  int64 startNumberOrTime = -1);
  int GetNumOccurencesOfTemplateTag(char *urlTemplate, const char *tag);

  MPD *mpd;
  uint64 current_period_key;
  bool m_bEndReached;
  bool m_bClosePending;
  bool mpdAvailable;
  bool m_bIsMpdValid;
  bool m_bBaseURLElementPresent;
  StreamSourceClock* m_pSourceClock;

  /**
   * Fetch time of the last mpd request.
   */
  MM_Time_DateTime m_FetchTime;

  /**
   * Callback function.
   * TO DO: In case, the use case arises that there may be more
   * than one callback regiter'er, then this can be changed to be
   * a queue of callbacks.
   */
  void (*m_fCallback)(void *);
  void *m_fCallbackPrivData;
  HTTPDrmType m_MpdDrmType;
  char *m_sMPDText;
  // includes NULL terminating character
  uint32 m_nMPDTextLen;

  static int SupportedAudioChannelValues[];
  static const char* AdapSet_DescriptorNames[];
  static const char* Common_AdapSet_Rep_SubRep_DescriptorNames[];

  //Lock to synchronize mpd data access
  MM_HANDLE m_pParserDataLock;

  friend class RepresentationInfo::SegmentFuncTemplate;
  friend class RepresentationInfo::SegmentFuncDefault;

  // Used for unit-testing live only.
  double m_nLiveTest_FakeAvailStartTime;

  friend class MPDParserTest;
};
}/* namespace video */
#endif /* __M3U8PARSER_H__ */
