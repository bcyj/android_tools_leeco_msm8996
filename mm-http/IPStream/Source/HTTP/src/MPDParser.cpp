/************************************************************************* */
/**
* MPDParser.cpp
* @brief implementation of MPDParser.
*  MPDParser is a specialization of playlistparser.
*  It parses the playlist file and stores the information about media files which
*  is used by DASHDownloadHelper
*
* COPYRIGHT 2011-2013 QUALCOMM Technologies, Inc.
* All rights reserved. Qualcomm Technologies proprietary and confidential.
*
************************************************************************* */
/* =======================================================================
Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/MPDParser.cpp#165 $
$DateTime: 2013/08/08 05:19:16 $
$Change: 4242765 $

========================================================================== */
/* =======================================================================
**               Include files for MPDParser.cpp
** ======================================================================= */
//TODO: Add function headers and comments
#include "MPDParser.h"
#include "StreamSourceTimeUtils.h"
#include "math.h"

namespace video
{

int MPDParser::SupportedAudioChannelValues[] =
{
  1,
  2,
  -1, // end of array marker. Unused value.
};

/**
 * Content Descriptors that are specific to adaptation-set.
 */
const char *MPDParser::AdapSet_DescriptorNames[] =
{
  "Accessibility",
  "Role",
  NULL
};

/**
 * Content-Descriptors that may be specified at adaptation-set
 * level, as well as representation level.
 */
const char *MPDParser::Common_AdapSet_Rep_SubRep_DescriptorNames[] =
{
  "AudioChannelConfiguration",
  NULL
};

/**
 * Helper class to extract properties and create xml string out
 * of mpd properties.
 */
class MPDParserXMLPropertiesHelper
{
public:
  /**
   * Helper to print part of the xml string that was just
   * appended.
   */
  class PrintXMLStringForDebug
  {
  public:
    PrintXMLStringForDebug(char *pString) :
      m_Start(pString ? (int)std_strlen(pString) : 0), m_pString(pString)
    {
    }

    ~PrintXMLStringForDebug()
    {
      if(m_pString)
      {
        int appendedLen = (int)std_strlen(m_pString + m_Start);

        // In MMDebugMsg.c:
        // #define MM_MAX_DEBUG_BUFFER  512
        if(appendedLen >= 500)
        {
          int pos = 0;
          static const int MAX_LEN = 50;
          char tmpBuf[1 + MAX_LEN];

          while(appendedLen > 0)
          {
            int len = QTV_MIN(appendedLen, MAX_LEN);
            tmpBuf[len] = 0;
            memcpy(tmpBuf, m_pString + pos, len);
            QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                   "xml: %s", tmpBuf);
            pos += len;
            appendedLen -= MAX_LEN;
          }
        }
        else
        {
          if(appendedLen > 0)
          {
            QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                   "xml: %s", m_pString + m_Start);
          }
        }
      }
    }

  private:
    int m_Start;
    char *m_pString;
  };

  class XMLGeneratorForRootElem
  {
  public:
    XMLGeneratorForRootElem(char *pString, int& size, const char * /*profiles */) :
      m_pString(pString), m_Size(size)
    {
      static const char* str1 =
        "<MPD "
        "xs:schemaLocation=\"urn:mpeg:DASH:schema:MPD:2011 DashPropertiesSchema.xsd\" "
        "xmlns=\"urn:mpeg:DASH:schema:MPD:2011\" "
        "xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xmlns:mas=\"urn:marlin:mas:1-0:services:schemas:mpd\" >";

        if(NULL == m_pString)
        {
          m_Size += (int)std_strlen(str1);
        }
        else
        {
          std_strlcat(m_pString, str1, m_Size);
        }
    }

    ~XMLGeneratorForRootElem()
    {
      static const char* str1 = "</MPD>";
      if(NULL == m_pString)
      {
        m_Size += (int)std_strlen(str1);
      }
      else
      {
        std_strlcat(m_pString, str1, m_Size);
      }

      if(NULL == m_pString)
      {
        m_Size += 1; // space for terminating null
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "xml string reqd bufsize %d", m_Size);
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      " xml string len %d", (int)std_strlen(m_pString));
      }
    }

    private:
      char *m_pString;
      int& m_Size;
  };

  /**
   * Helper class to add opening and closing tags for "Period",
   * "AdaptationSet", "Representation"
   */
  class XMLKeyTag
  {
  public:
    XMLKeyTag(char *pString, int& size, const char *tag, int periodKey) :
        m_pString(pString), m_Size(size), m_Tag(tag)
    {
      if(m_Tag)
      {
        char tmpBuf[20];
        std_strlprintf(tmpBuf, sizeof(tmpBuf), "%d", periodKey);

        static const char *str1 = "<";
        static const char* str2 = " Key=\"";
        static const char *str3 = "\">";

        if(NULL == m_pString)
        {
          m_Size += (int)std_strlen(str1);
          m_Size += (int)std_strlen(m_Tag);
          m_Size += (int)std_strlen(str2);

          // add the key value
          m_Size += (int)std_strlen(tmpBuf);

          m_Size += (int)std_strlen(str3);
        }
        else
        {
          std_strlcat(m_pString, str1, m_Size);
          std_strlcat(m_pString, m_Tag, m_Size);
          std_strlcat(m_pString, str2, m_Size);

          // add the key value
          std_strlcat(m_pString, tmpBuf, m_Size);

          std_strlcat(m_pString, str3, m_Size);
        }
      }
    }

    ~XMLKeyTag()
    {
      if(m_Tag)
      {
        static const char* str1 = "</";
        static const char* str2 = ">";
        if(NULL == m_pString)
        {
          m_Size += (int)std_strlen(str1);
          m_Size += (int)std_strlen(m_Tag);
          m_Size += (int)std_strlen(str2);
        }
        else
        {
          std_strlcat(m_pString, str1, m_Size);
          std_strlcat(m_pString, m_Tag, m_Size);
          std_strlcat(m_pString, str2, m_Size);
        }
      }
    }
  private:
    // disallow
    XMLKeyTag();
    XMLKeyTag(const XMLKeyTag&);
    XMLKeyTag& operator=(const XMLKeyTag&);

    char *m_pString;
    int &m_Size;
    const char* m_Tag;
  };

  class XMLGeneratorForPropertiesString
  {
  public:
    XMLGeneratorForPropertiesString(char *pString, int& size, const char *pPropertiesString) :
      m_pString(pString), m_Size(size), m_pPropertiesString(pPropertiesString)
    {
      static const char *str1 = "<";
      static const char *str2 = ">";
      if(NULL == m_pString)
      {
        m_Size += (int)std_strlen(str1);
        m_Size += (int)std_strlen(m_pPropertiesString);
        m_Size += (int)std_strlen(str2);
      }
      else
      {
        std_strlcat(m_pString, str1, m_Size);
        std_strlcat(m_pString, m_pPropertiesString, m_Size);
        std_strlcat(m_pString, str2, m_Size);
      }
    }

    ~XMLGeneratorForPropertiesString()
    {
      static const char *str1 = "</";
      static const char *str2 = ">";

      if(NULL == m_pString)
      {
        m_Size += (int)std_strlen(str1);
        m_Size += (int)std_strlen(m_pPropertiesString);
        m_Size += (int)std_strlen(str2);
      }
      else
      {
        std_strlcat(m_pString, str1, m_Size);
        std_strlcat(m_pString, m_pPropertiesString, m_Size);
        std_strlcat(m_pString, str2, m_Size);
      }
    }

  private:
    char *m_pString;
    int& m_Size;
    const char* m_pPropertiesString;
  };

  /**
   * Helper class to append <tagname><value></> to pString.
   */
  class CreateXMLForString
  {
  public:
    CreateXMLForString(char *pString, int& size, const char *tagName, const char *value) :
      m_pString(pString), m_Size(size), m_TagName(tagName), m_Value(value)
    {
      if(m_TagName && m_Value)
      {
        static const char *str1 = "<";
        static const char *str2 = ">";
        static const char *str4 = "/";

        if(NULL == pString)
        {
          size += (int)std_strlen(str1);
          size += (int)std_strlen(tagName);
          size += (int)std_strlen(str2);
          size += (int)std_strlen(m_Value);
          size += (int)std_strlen(str1);
          size += (int)std_strlen(str4);
          size += (int)std_strlen(tagName);
          size += (int)std_strlen(str2);
        }
        else
        {
          std_strlcat(m_pString, str1, size);
          std_strlcat(m_pString, m_TagName, size);
          std_strlcat(m_pString, str2, size);
          std_strlcat(m_pString, m_Value, size);
          std_strlcat(m_pString, str1, size);
          std_strlcat(m_pString, str4, size);
          std_strlcat(m_pString, m_TagName, size);
          std_strlcat(m_pString, str2, size);
        }
      }
    }

    ~CreateXMLForString()
    {

    }

  private:
    CreateXMLForString();
    CreateXMLForString(const CreateXMLForString&);
    CreateXMLForString& operator=(const CreateXMLForString&);

    char *m_pString;
    int &m_Size;
    const char *m_TagName;
    const char *m_Value;
  };

  /**
   * Helper class to construct xml-string from a
   * StringValueContainer and append to pString.
   */
  class XMLGeneratorForStringValueContainer
  {
  public:
    XMLGeneratorForStringValueContainer(
      char *pString, int& size, StringValueContainer& rStringValueContainer) :
        m_pString(NULL)
    {
      StringValue *pStringValueArray = NULL;
      int arrSize = 0;
      rStringValueContainer.GetStringValues(&pStringValueArray, arrSize);

      if(pStringValueArray)
      {
        for(int i = 0; i < arrSize; ++i)
        {
          char* pTagName = NULL, *pValue = NULL;
          pStringValueArray[i].GetStringValue(&pTagName, &pValue);
          MPDParserXMLPropertiesHelper::CreateXMLForString xmlStringValue(
            pString,size,pTagName,pValue);
        }
      }
    }

    char *m_pString;
  };

  class XMLGeneratorForContentDescriptorContainer
  {
  public:
    XMLGeneratorForContentDescriptorContainer(
      char *pString, int& size, ContentDescriptorContainer& rContentDescContainer) :
        m_pString(pString)
    {
      // Populate content descriptors like Accessibility, Role, Rating
      ContentDescriptorType *pContentDescs = NULL;
      int numDescs = 0;
      rContentDescContainer.GetContentDescriptors(&pContentDescs, numDescs);

      if(pContentDescs)
      {
        for(int k = 0; k < numDescs; ++k)
        {
          if(false == pContentDescs[k].IsInherited())
          {
            const char *pDescName = NULL, *pSchemeURI = NULL, *pValue = NULL;
            int reqdSize = (int)std_strlen("<");
            pContentDescs[k].GetDesc(&pDescName, &pSchemeURI, &pValue);
            reqdSize += (pDescName ? (int)std_strlen(pDescName) : 0);
            reqdSize += (int)std_strlen(" schemeIdUri=\"");
            reqdSize += (pSchemeURI ? (int)std_strlen(pSchemeURI) : 0);
            reqdSize += (int)std_strlen("\" value=\"");
            reqdSize += (pValue ? (int)std_strlen(pValue) : 0);
            reqdSize += (int)std_strlen("\"/>");

            if(NULL == pString)
            {
              size += reqdSize;
            }
            else
            {
              std_strlcat(pString, "<", size);
              (pDescName ? std_strlcat(pString, pDescName, size) : 0);
              std_strlcat(pString, " schemeIdUri=\"", size);
              (pSchemeURI ? std_strlcat(pString, pSchemeURI, size) : 0);
              std_strlcat(pString, "\" value=\"", size);
              (pValue ? std_strlcat(pString, pValue, size) : 0);
              std_strlcat(pString, "\"/>", size);
            }
          }
          else
          {
            const char *pDescName = NULL, *pSchemeURI = NULL, *pValue = NULL;
            pContentDescs[k].GetDesc(&pDescName, &pSchemeURI, &pValue);
            QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                          "Skipping inherited property %s %s %s",
                          pDescName, pSchemeURI, pValue);
          }
        }
      }
    }

    ~XMLGeneratorForContentDescriptorContainer()
    {

    }

  private:
    char *m_pString;
  };
  class XMLGeneratorForCP
  {
  public:
    XMLGeneratorForCP(char *pString, int& size, const char*pContentProtection)
    :m_pString(pString), m_Size(size)
      {
        if(NULL == pString)
        {
        if(pContentProtection)
        {
          m_Size += (int)std_strlen(pContentProtection);
          }

        }
        else
        {
        if (pContentProtection)
            {
          std_strlcat(m_pString, pContentProtection, m_Size);
            }
          }
        }

    ~XMLGeneratorForCP()
        {
    }
  private:
    char *m_pString;
    int& m_Size;
  };

  class XMLGeneratorForContentProtectionElement
  {
  public:
      XMLGeneratorForContentProtectionElement(char *pString, int& size, ContentProtectionType& rCpt) :
      m_pString(pString), m_Size(size), m_rCpt(rCpt)
    {
        switch(m_rCpt.GetContentProtectionSource())
        {
        case video::ContentProtectionType::CONTENT_PROTECTION_DASH:
          {
          XMLGeneratorForCP(pString,size, m_rCpt.GetCENCContentProtection());
          }
          break;
      case video::ContentProtectionType::CONTENT_PROTECTION_MARLIN:
          {
          XMLGeneratorForCP(pString,size, m_rCpt.GetMarlinContentProtection());
            }
        break;
      case video::ContentProtectionType::CONTENT_PROTECTION_PLAYREADY:
        {
          XMLGeneratorForCP(pString,size,m_rCpt.GetPlayReadyContentProtection());
          }
          break;
        default:
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Unknown contentProtection source");
          break;
        } // end switch
      }


      ~XMLGeneratorForContentProtectionElement()
        {

    }

  private:
    char *m_pString;
    int& m_Size;
    ContentProtectionType& m_rCpt;
  };
};

/* =======================================================================
**                             MPD Class
** ======================================================================= */
/* @brief - Constructor for the MPD class.*/
MPD::MPD():
  m_pPeriodInfo(NULL),
  m_nNumPeriods(0),
  m_nDuration(0),
  m_nMinBufferTime(0),
  m_nTimeShiftBufferDepth(0),
  m_nTimeShiftBufferDepthCache(0),
  m_nMinimumUpdatePeriod(0.0),
  m_nMinimumUpdatePeriodCache(0),
  m_nPeriodArrSize(0),
  m_AvailabilityStartTime(0.0),
  m_bIsLive(false)
{

}
MPD::~MPD()
{
  if(m_pPeriodInfo)
  {
    QTV_Delete_Array(m_pPeriodInfo);
    m_pPeriodInfo = NULL;
  }
}
PeriodInfo* MPD::getPeriodInfo(int& numPeriods)
{
  numPeriods = m_nNumPeriods;
  return m_pPeriodInfo;
}

PeriodInfo* MPD::InitializePeriodInfo(int numPeriods)
{
  if(numPeriods == 0)
  {
    m_nPeriodArrSize = MIN_PERIOD_ARR_SIZE;
  }
  else
  {
    m_nPeriodArrSize = numPeriods;
  }
  m_nNumPeriods = numPeriods;
  if(m_pPeriodInfo)
  {
    QTV_Delete_Array(m_pPeriodInfo);
    m_pPeriodInfo = NULL;
  }
  m_pPeriodInfo = QTV_New_Array(PeriodInfo,m_nPeriodArrSize);
  return m_pPeriodInfo;
}
bool MPD::AddPeriod(int& PeriodIndex)
{
  bool bOk = true;
  if(m_nNumPeriods >= m_nPeriodArrSize)
  {
    bOk = ResizePeriodInfo(2*m_nPeriodArrSize);
  }
  if(bOk)
  {
    PeriodIndex = m_nNumPeriods;
    m_nNumPeriods++;
  }
  return bOk;
}

bool MPD::RemovePeriod()
{
  bool bOk = true;
  m_nNumPeriods--;
  return bOk;
}

bool MPD::CommitPeriodInfo()
{
  bool bOk = true;
  if(m_nPeriodArrSize > m_nNumPeriods)
  {
    bOk = ResizePeriodInfo(m_nNumPeriods);
  }
  if(bOk)
  {
    if(m_pPeriodInfo)
    {
      if(m_nNumPeriods > 0)
      {
        m_pPeriodInfo[m_nNumPeriods-1].SetLastPeriod();
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "LastPeriod set on period %d", m_nNumPeriods-1);
      }
    }
  }
  return bOk;
}

bool MPD:: IsValidMPD()
{
  bool bOk = false;
  if (m_nNumPeriods >  0)
  {
    bOk = true;
  }
  return bOk;
}

bool MPD::ResizePeriodInfo(int newSize)
{
  bool bOk=false;
  if(m_nPeriodArrSize < newSize)
  {
    PeriodInfo* temp;
    temp = QTV_New_Array(PeriodInfo,(m_nPeriodArrSize));
    if(temp)
    {
      for(int i=0;i<m_nNumPeriods;i++)
      {
        temp[i]=m_pPeriodInfo[i];
      }
      QTV_Delete_Array(m_pPeriodInfo);
      m_pPeriodInfo = NULL;
      m_pPeriodInfo =  QTV_New_Array(PeriodInfo,(newSize));
      if(m_pPeriodInfo)
      {
        for(int i=0;i<m_nNumPeriods;i++)
        {
          m_pPeriodInfo[i]=temp[i];
        }
        bOk = true;
        m_nPeriodArrSize = newSize;
      }
      QTV_Delete_Array(temp);
      temp=NULL;
    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}
void MPD::SetMPDInfo(double duration,
                     double bufferTime,
                     double timeShiftBufferDepth,
                     double minUpdatePeriod,
                     double availStartTime)
{
  m_nDuration = duration;
  m_nMinBufferTime = bufferTime;

  m_nTimeShiftBufferDepth = timeShiftBufferDepth;
  m_nTimeShiftBufferDepthCache = m_nTimeShiftBufferDepth;
  if (m_nTimeShiftBufferDepth > MAX_TSB_BUFFER_SECS)
  {
    m_nTimeShiftBufferDepth = MAX_TSB_BUFFER_SECS;
  }

  m_nMinimumUpdatePeriod = minUpdatePeriod;
  m_nMinimumUpdatePeriodCache = m_nMinimumUpdatePeriod;
  if (m_nMinimumUpdatePeriod > MAX_MINUPDATE_PERIOD_SECS)
  {
    m_nMinimumUpdatePeriod = MAX_MINUPDATE_PERIOD_SECS;
  }

  m_AvailabilityStartTime = availStartTime;
}

void MPD::setMinimumUpdatePeriodCache(double minUpdatePeriod)
{
  m_nMinimumUpdatePeriodCache = minUpdatePeriod;
}

void MPD::setTimeShiftBufferDepthCache(double timeShiftBufferDepth)
{
  m_nTimeShiftBufferDepthCache = timeShiftBufferDepth;
}

void MPD::setMinimumUpdatePeriod(double minUpdatePeriod)
{
  m_nMinimumUpdatePeriod = minUpdatePeriod;
}

void MPD::setTimeShiftBufferDepth(double timeShiftBufferDepth)
{
  m_nTimeShiftBufferDepth = timeShiftBufferDepth;
}

uint32 MPD::getNumPeriods() const
{
  return m_nNumPeriods;
}

double MPD::getDuration() const
{
  return m_nDuration;
}

double MPD::getMinimumUpdatePeriod() const
{
  return m_nMinimumUpdatePeriod;
}

double MPD::getMininumUpdatePeriodCache() const
{
  return m_nMinimumUpdatePeriodCache;
}

double MPD::getTimeShiftBufferDepthCache() const
{
  return m_nTimeShiftBufferDepthCache;
}


double MPD::getMinBufferTime() const
{
  return m_nMinBufferTime;
}

double MPD::getTimeShiftBufferDepth() const
{
  return m_nTimeShiftBufferDepth;
}

double MPD::GetAvailabilityStartTime() const
{
  return m_AvailabilityStartTime;
}

uint64 MPD::getStartTime()
{
  uint64 start_time = 0;
  if(m_nNumPeriods > 0)
  {
    start_time = m_pPeriodInfo[0].getStartTime();
  }
  return start_time;
}

/**
 * Check if key from PeriodInfo corresponds to the last period.
 */
bool MPD::IsLastPeriod(uint64 nPeriodKey)
{
  bool rslt = false;
  if(m_nNumPeriods > 0)
  {
    PeriodInfo& rLastPeriodInArray = m_pPeriodInfo[m_nNumPeriods-1];
    if (rLastPeriodInArray.IsLastPeriod() && nPeriodKey == rLastPeriodInArray.getPeriodKey())
    {
      rslt = true;
    }
  }

  return rslt;
}

/**
 * True if comparison needs to be done between newly arrived mpd
 * and old mpd.
 */
bool MPD::IsMpdValid() const
{
  return m_pPeriodInfo ? true : false;
}

/**
 * Set the flag that mpd is live.
 */
void MPD::SetLive()
{
  m_bIsLive = true;
}

/**
 * Check if mpd is live.
 */
bool MPD::IsLive() const
{
  return m_bIsLive;
}

/**
 * For debugging.
 */
void MPDParser::PrintMPD()
{
  QTV_NULL_PTR_CHECK(mpd, RETURN_VOID);
  int numPeriods = 0;
  PeriodInfo *pPeriodArray = mpd->getPeriodInfo(numPeriods);
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "PrintMPD numPeriods %d  ---------------------", numPeriods);

  MM_CriticalSection_Enter(m_pParserDataLock);

  for (int i = 0; i < numPeriods; ++i)
  {
    PeriodInfo& rPeriodInfo = pPeriodArray[i];
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "PrintMPD period idx 'key' %u periodStart %u, periodDuration %u",
                  (uint32)((uint64)(rPeriodInfo.getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT),
                  (uint32)rPeriodInfo.getStartTime(),
                  (uint32)rPeriodInfo.getDuration() * 1000);

    int numRepGrps = 0;
    RepresentationGroup *newRepGrp = rPeriodInfo.getRepGrpInfo(numRepGrps);

    if (newRepGrp)
    {
      for (int j = 0; j < numRepGrps; ++j)
      {
        int numNewReps = 0;
        RepresentationInfo *repArray = newRepGrp[j].getRepInfo(numNewReps);

        if (repArray)
        {
          for (int k = 0; k < numNewReps; ++k)
          {
            // This is mandatory and unique across a rep.
            RepresentationInfo& newRep = repArray[k];
            newRep.GetSegmentFunc()->PrintMPDInfoForRepresentation(this, &rPeriodInfo, &newRep);
          }
        }
      }
    }
  }

  MM_CriticalSection_Leave(m_pParserDataLock);

}


void RepresentationInfo::SegmentFuncTemplate::PrintMPDInfoForRepresentation(MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo)
{
  QTV_NULL_PTR_CHECK(pMPDParser, RETURN_VOID);
  QTV_NULL_PTR_CHECK(pPeriodInfo, RETURN_VOID);
  QTV_NULL_PTR_CHECK(pRepInfo, RETURN_VOID);

  MM_Time_DateTime currentTime;
  MM_Time_GetUTCTime(&currentTime);

  double currMSeconds =
    StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);
  if(pRepInfo->GetSegmentTemplate())
  {
    double segDuration = CalculateSegmentDurationFromStoredTemplate(pRepInfo);
    char *newRepID = pRepInfo->getRepIdentifier();

    if (newRepID)
    {
      QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "PrintMPD repID '%s'. First and last segment info:",
        newRepID);
    }

    int64 nEarliestOffsetStartNumber = -1, nLatestOffsetStartNumber = -1;
    HTTPDownloadStatus status = ((RepresentationInfo::SegmentFuncTemplate*)(pRepInfo->GetSegmentFunc()))->GetAvailabilityBoundsForSegDurationFromTemplate(
      pMPDParser,
      nEarliestOffsetStartNumber, nLatestOffsetStartNumber,
      pPeriodInfo,
      segDuration,
      currMSeconds,
      pRepInfo->GetMinUpdatePeriod(),
      (double)pMPDParser->GetTimeShiftBufferDepth()*1000);

    if(HTTPCommon::HTTPDL_SUCCESS == status)
    {
      SegmentInfo firstSegmentInfo, lastSegmentInfo;
      if (true == ((RepresentationInfo::SegmentFuncTemplate*)(pRepInfo->GetSegmentFunc()))->GenerateSegmentInfoFromTemplate(
        pMPDParser, &firstSegmentInfo, pRepInfo, pPeriodInfo, (uint32)nEarliestOffsetStartNumber) &&
        (true == ((RepresentationInfo::SegmentFuncTemplate*)(pRepInfo->GetSegmentFunc()))->GenerateSegmentInfoFromTemplate(
        pMPDParser, &lastSegmentInfo, pRepInfo, pPeriodInfo, (uint32)nLatestOffsetStartNumber)))
      {

        QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "PrintMPD First segment '%s'", firstSegmentInfo.GetURL());
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "PrintMPD First segment key %llu, startTime %llu",
          firstSegmentInfo.getKey(), (uint64)firstSegmentInfo.getStartTime());
        QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "PrintMPD last segment '%s'", lastSegmentInfo.GetURL());
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "PrintMPD last segment key %llu, startTime %llu",
          lastSegmentInfo.getKey(), (uint64)lastSegmentInfo.getStartTime());
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "PrintMPD failed to generate segmentinfo from template");
      }

    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "PrintMPD: SegmentTemplate waiting");
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "PrintMPD: Unexpected error NULL segmenttemplate for rep");
  }
}


void RepresentationInfo::SegmentFuncDefault::PrintMPDInfoForRepresentation(MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo)
{

  QTV_NULL_PTR_CHECK(pMPDParser, RETURN_VOID);
  QTV_NULL_PTR_CHECK(pPeriodInfo, RETURN_VOID);
  QTV_NULL_PTR_CHECK(pRepInfo, RETURN_VOID);

  char *newRepID = pRepInfo->getRepIdentifier();

  uint32 numSegments = 0, tmpSize = 0;
  SegmentInfo *segmentArray = pRepInfo->getSegmentInfo(numSegments, tmpSize);

  if (newRepID)
  {
    QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "PrintMPD repID '%s'. NumSegmentsInArray %d. First and last segment info:",
      newRepID, (int)numSegments);
  }

  if (numSegments > 0)
  {
    QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "PrintMPD First segment '%s'", segmentArray[0].GetURL());
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "PrintMPD First segment key %d, startTime %d",
      (int)segmentArray[0].getKey(),(int)segmentArray[0].getStartTime());
    QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "PrintMPD last segment '%s'", segmentArray[numSegments-1].GetURL());
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "PrintMPD last segment key %d, startTime %d",
      (int)segmentArray[numSegments-1].getKey(),
      (int)segmentArray[numSegments-1].getStartTime());
  }
}

/* =======================================================================
**                             MPDParser Class
** ======================================================================= */
/* @brief: MPDParser constructor
 * @param - Scheduler
 * @param - pStackNotificationHandler - Stack status notification handler
 */
MPDParser::MPDParser(HTTPSessionInfo& pSessionInfo,Scheduler& pScheduler, HTTPStatusHandlerInterface* pStackNotificationHandler, uint32 nRequestIDfromResolver,
                       HTTPStackInterface& HTTPStack)
  :PlaylistParser(pSessionInfo,pScheduler),
    current_period_key(0),
    m_bEndReached(false),
  m_bClosePending(false),
  mpdAvailable(false),
  m_bIsMpdValid(false),
  m_bBaseURLElementPresent(false),
  m_fCallback(NULL),
  m_fCallbackPrivData(NULL),
  m_MpdDrmType(HTTPCommon::NO_DRM),
  m_sMPDText(NULL),
  m_nMPDTextLen(0),
  m_pParserDataLock(NULL),
  m_nLiveTest_FakeAvailStartTime(MAX_UINT32_VAL)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "MPDParser::MPDParser" );
  bool bOk = false;
  m_pSourceClock = NULL;
  mpd=QTV_New(MPD);
  if(mpd == NULL)
  {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: mpd creation failed" );
  }

  bOk = (MM_CriticalSection_Create(&m_pParserDataLock) == 0);
  if(!bOk)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Unable to create m_pParserDataLock" );
  }
  else
  {
    m_pSourceClock = QTV_New_Args(StreamSourceClock, (bOk));
    if (m_pSourceClock == NULL || !bOk)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: Stream source clock creation failed" );
    }
    else
    {
      m_pDownloader = QTV_New_Args(PlaylistDownloader,(pSessionInfo,
                                   m_pSourceClock, pStackNotificationHandler, nRequestIDfromResolver,
                                   HTTPStack));
      if(m_pDownloader == NULL)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "MPDParser::MPDParser Could not create Downloader" );
      }
    }
  }

  // just for intialization. Will get overwritten anyways.
  MM_Time_GetUTCTime(&m_FetchTime);
}
/* @brief : MPDParser Destructor
*/
MPDParser::~MPDParser()
{
  if(mpd)
  {
    QTV_Delete(mpd);
    mpd = NULL;
  }
  if( m_pDownloader)
  {
    QTV_Delete( m_pDownloader);
    m_pDownloader = NULL;
  }
  if(m_pSourceClock)
  {
    QTV_Delete(m_pSourceClock);
    m_pSourceClock = NULL;
  }
  if(m_pParserDataLock)
  {
    (void)MM_CriticalSection_Release(m_pParserDataLock);
    m_pParserDataLock = NULL;
  }
  if (m_sMPDText)
  {
    QTV_Delete(m_sMPDText);
    m_sMPDText = NULL;
    m_nMPDTextLen = 0;
  }
}

/**
 * Store the time the mpd is fetched from the server.
 */
void MPDParser::SetFetchTime()
{
  // Reset the fetch time to start timing the next download.
  MM_Time_GetUTCTime(&m_FetchTime);
  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "m_FetchTime set as hr:%u, min %u, sec %u",
                (uint32)m_FetchTime.m_nHour, (uint32)m_FetchTime.m_nMinute, (uint32)m_FetchTime.m_nSecond);
}

/**
 * Retrive the time the MPD was last fetched from the server.
 */
MM_Time_DateTime MPDParser::GetFetchTime() const
{
  return m_FetchTime;
}

/* @brief - Gives total duration for the mpd
 * @param - start - start time of the current mpd
 * @param - end - end time of the current mpd
 * @return - true if duration can be retrieved
    false otherwise
 */
bool MPDParser::GetTotalDuration(uint64& start,uint64& end)
{
  start = 0;
  end = 0;
  QTV_NULL_PTR_CHECK(mpd,false);

  if (!IsLive())
  {
    start = mpd->getStartTime();

    int numPeriods = 0;
    PeriodInfo *pPeriodInfo = mpd->getPeriodInfo(numPeriods);
    if (pPeriodInfo && numPeriods > 0)
    {
      end = pPeriodInfo[numPeriods - 1].getStartTime() +
        (uint32)(1000.0 * pPeriodInfo[numPeriods - 1].getDuration());
    }
  }

  return true;
}

/* @brief - Gives end time for the current mpd
 * @param - end - end time of the current mpd
 * @return - true if end time can be retrieved
    false otherwise
 */
bool MPDParser::GetEndTime(uint64& end)
{
  end = 0;
  QTV_NULL_PTR_CHECK(mpd, false);

  int numPeriods = 0;
  PeriodInfo *pPeriodInfo = mpd->getPeriodInfo(numPeriods);
  if (pPeriodInfo && numPeriods > 0)
  {
    end = pPeriodInfo[numPeriods - 1].getStartTime() +
      (uint32)(1000.0 * pPeriodInfo[numPeriods - 1].getDuration());
  }

  return true;
}

/* @brief - This function converts a String vector to a list of double
            variables.
 * @param[in] - strVector to be converted
 * @param[in/out] - number of elements in the list
 * @param[out] - list - list of elements
 * @return - true if conversion is successful
             false otherwise
 */
bool MPDParser::ConvertStringVectorToList(char* strVector,int& numElements, double* list)
{
  QTV_NULL_PTR_CHECK(strVector,false);
  bool bOk=false;
  int cnt=0;
  if(numElements == 0)
  {
    if(strVector)
    {
      numElements=1;
    }
    bOk=true;
    while(strVector && *strVector)
    {
      if(*strVector==' ')
      {
        numElements++;
        while(strVector && *strVector==' ')
          strVector++;
      }
      else
      {
        strVector++;
      }
    }

  }
  else if(list)
  {
    bOk=true;
    while(strVector && *strVector)
    {
      if(*strVector==' ')
      {
        while(strVector && *strVector==' ')
          strVector++;
      }
      else
      {
        if(cnt < numElements &&  (isdigit(*strVector) || *strVector=='.'))
        {
          list[cnt++]=atof(strVector);
          while(strVector && (isdigit(*strVector) || *strVector=='.'))
          {
            strVector++;
          }
        }
        strVector++;
      }
    }
  }
  return bOk;
}
/* @brief - This function converts a duration type(xml) value to duration in seconds
            variables.
 * @param[in] - duration - duration type to be converted
 * @param[out] - duration in seconds
 * @return - void
 */
 void MPDParser::ConvertDurationTypeToSeconds(char* duration,double& duration_in_sec)
  {
    double total_duration =0;

    if(duration)
    {
      bool timeSection = false;
      double num=0;
      while(duration && *duration)
      {
        if(timeSection == false)
        {
          //If timesection indicated by 'T' did not start yet then
          //Y signifies year, M - Month and D - Day
          if(*duration == 'Y')
          {
            timeSection = false;
            total_duration+= num*365*24*3600;
            num = 0;
          }
          else if(*duration == 'M')
          {
            timeSection = false;
            total_duration+= num*30*24*3600;
            num = 0;
          }
          else if(*duration == 'D')
          {
            timeSection = false;
            total_duration+= num*24*3600;
            num = 0;
          }
          else if(*duration == 'T')
          {
            timeSection = true;
          }
        }
        else
        {
          //After the start of timesection H indicates Hour, M - Minute
          //and S second
          if(*duration=='H')
          {
            total_duration+=num*3600;
            num = 0;
          }
          else if(*duration == 'M')
          {
            total_duration+=num*60;
            num = 0;
          }
          else if(*duration == 'S')
          {
            total_duration+=num;
            num = 0;
          }
        }
        if(isdigit(*duration) || *duration=='.')
        {
          num = atof(duration);
          while(duration && (isdigit(*duration) || *duration=='.'))
          {
            duration++;
          }
          continue;
        }
        duration++;
      }

    }
    duration_in_sec = total_duration;
  }

/* @brief - Scheduler task to download and update the MPD
 * @param[in] - Task parameter
 * @return - int
 */
int MPDParser::TaskDownloadAndUpdateMPD
(
 void* pTaskParam
 )
{

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  PlaylistDownloaderTaskParam* parameter =
    (PlaylistDownloaderTaskParam*) pTaskParam;
  bool delete_task = false;
  if (parameter == NULL || parameter->pCbSelf == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Invalid taskParam" );
  }
  else
  {
    MPDParser* pSelf = (MPDParser*)parameter->pCbSelf;
    StreamSourceClock* pSourceClock = pSelf->m_pSourceClock;
    PlaylistDownloader* pDownloader = pSelf->m_pDownloader;
    if (pSourceClock == NULL || pDownloader == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: pSourceClock or pDownloader is NULL" );
    }
    else
    {
      if(pSelf->m_bClosePending)
      {
        status = HTTPCommon::HTTPDL_INTERRUPTED;
      }
      else
      {
        status = pDownloader->DownloadAndUpdatePlaylist();
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "DownloadAndUpdatePlaylist status %d ", status );

        if (HTTPSUCCEEDED(status))
        {
          char *pMPDText;
          pMPDText = pDownloader->GetRepresentationText();
          if(!pMPDText)
          {
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
          }
          else
          {
            if(!pSelf->Parse(pMPDText))
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Playlist Parse Error");
              status = HTTPCommon::HTTPDL_ERROR_ABORT;
              delete_task = true;
              pSelf->m_bIsMpdValid = false;
              pSelf->mpdAvailable = true;
            }
            else
            {
              pSelf->m_bIsMpdValid = true;
              pSelf->mpdAvailable = true;
              pSelf->Print();

              pSelf->SetFetchTime();

              pDownloader->SetUpdateAndRefreshTime(
                NULL,
                pSelf->m_pSourceClock->GetTickCount(),
                (uint32)(1000 * pSelf->mpd->getMinimumUpdatePeriod()));

              parameter->nStartTime = pSelf->m_pSourceClock->GetTickCount();

              if(pSelf->m_bEndReached)
              {
                delete_task = true;
              }
            }
          }
        }
      }

      if((!(status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_WAITING))
         || pSelf->m_bClosePending || delete_task == true)
      {
        if(!HTTPSUCCEEDED(status))
        {
          pSelf->m_bAbortSet = true;
        }
        pDownloader->CloseConnection();
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Deleting Playlist parser task" );
        //Delete DownloadAndUpdatePlaylist task
        if (!pSelf->m_pScheduler.DeleteTask(parameter->ntaskID))
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "DownloadAndUpdatePlaylist task could not be deleted" );
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
      }
    }
  }

  return (HTTPSUCCEEDED(status) ? 0 : -1);;
}

/*
* @brief - Opens the MPD Parser.
* @param[in] - url of the playlist file
*/

void MPDParser::Open(char* url)
{
  return SetURL(url);
}

/* @brief: Sets the close pending flag to true.
 *  DownloadAndUpdate playlist will check for this flag to find out
 *  if there was a close issued and will stop the task
 */

void MPDParser::Close()
{
  m_bClosePending = true;
  return;
}

/* @brief: Prints the complete MPD
 */

void MPDParser::Print()
{
  return;
}

/*
* @brief - Sets the url for MPDParser. Creates the task to download and
* update the playlist file.
* @param[in] - url of the playlist file
*/
void MPDParser::SetURL(char *url)
{
  if(m_url)
  {
    QTV_Free(m_url);
    m_url = NULL;
  }

  m_url = (char*)QTV_Malloc((std_strlen(url)+1));
  if(!m_url)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Memory allocation failed for m_url" );
  }
  else
  {
    (void)std_strlcpy(m_url,url,std_strlen(url)+1);
    if(!m_pDownloader)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Downloader is null" );
      m_bAbortSet = true;
    }
    else
    {
      //Set the url in downloader
      m_pDownloader->SetURL(url);
      //Create the scheduler task to download and update the playlist
      SchedulerTask pTask = TaskDownloadAndUpdateMPD;
      //Fill out the task param structure - to be passed to the task
      PlaylistDownloaderTaskParam* pTaskParam =
                QTV_New(PlaylistDownloaderTaskParam);
      if (pTaskParam == NULL)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "Error: Memory allocation failed for taskParam" );
        m_bAbortSet = true;
      }
      else
      {
        pTaskParam->pCbSelf = (void*)this;
        //Used as reference for timing out
        pTaskParam->nStartTime = m_pSourceClock->GetTickCount();
        int taskID = m_pScheduler.AddTask(pTask, pTaskParam);
        if (taskID)
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "Scheduler %d task added", taskID );
        }
        ((SchedulerTaskParamBase*)pTaskParam)->ntaskID = taskID;
      }
    }
  }
  return;

}

bool MPDParser::RegisterMPDUpdateNotification(void (*fNotification)(void *), void *pPrivData)
{
  bool rslt = false;

  if (m_fCallback)
  {
    // support not yet added to register more than one callback.
  }
  else
  {
    m_fCallback = fNotification;
    m_fCallbackPrivData = pPrivData;
    rslt = true;
  }

  return rslt;
}

/* This function will provide the client with the next period information.
 * The PeriodInfo outparam is copied with next period’s information.
 * @param[out] - pPeriodInfo - next period's information
 * @param[out] - bEOS - Set to true, if it is the last period in MPD.
 * @return - HTTPDL_SUCCESS on successful retrieval.
 *           HTTPDL_WAITING if period information is currently not present in MPD.
 *           HTTPDL_ERROR_ABORT is returned if period info can not be retrieved.
*/
HTTPDownloadStatus MPDParser::GetNextPeriod(PeriodInfo *pPeriodInfo, bool& bEOS)
{
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(mpd,status);
  int numPeriods = 0;
  PeriodInfo *period_Info = mpd->getPeriodInfo(numPeriods);
  uint32 periodKey = (uint32)((current_period_key & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  bEOS = false;

  if(m_bAbortSet)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "MPD task aborted");
    bEOS = true;
  }
  else if(!mpdAvailable)
  {
    status = HTTPCommon::HTTPDL_WAITING;
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Waiting to get the mpd information from server");
  }
  else if (period_Info)
  {
    uint32 periodIndex = periodKey - (uint32)(period_Info[0].getPeriodKey() >> MPD_PERIOD_SHIFT_COUNT);
    if(periodIndex >= (uint32)numPeriods)
    {
      if(numPeriods > 0)
      {
        if(period_Info[numPeriods-1].IsLastPeriod())
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "EndOfStream for MPDs reached on period %d", numPeriods-1);
          bEOS=true;
          status = HTTPCommon::HTTPDL_SUCCESS;
        }
        else
        {
          if(!m_bEndReached)
          {
            status = HTTPCommon::HTTPDL_WAITING;
          }
        }
      }
    }
    else
    {
      if(periodIndex < (uint32)numPeriods)
      {
        if (pPeriodInfo)
        {
          pPeriodInfo->Copy(period_Info[periodIndex]);
          current_period_key = ((uint64)(periodIndex+1)<<MPD_PERIOD_SHIFT_COUNT) + period_Info[0].getPeriodKey();
        }
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
    }
  }

  return status;
}

/* This function will provide the client with the current period’s information.
 * The PeriodInfo outparam is copied with current period’s information.
 * @param[out] - pPeriodInfo - Current Period's information
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::GetCurrentPeriod(PeriodInfo& pPeriodInfo)
{
  bool rslt = false;
  QTV_NULL_PTR_CHECK(mpd,rslt);
  int numPeriods = 0;
  PeriodInfo *period_Info = mpd->getPeriodInfo(numPeriods);
  uint32 period_index = (uint32)((uint64)(current_period_key & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  if(m_bAbortSet)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "MPD task aborted");
  }
  else if(!mpdAvailable)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Waiting to get the mpd information from server");
  }
  else if(period_index > 0 )
  {
    period_index--;

    uint32 periodArrayIdx =
      period_index - (uint32)((uint64)(period_Info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);

    if(periodArrayIdx < (uint32)numPeriods)
    {
      pPeriodInfo.Copy(period_Info[period_index]);
      rslt = true;
    }
  }

  return rslt;
}

/**
 * Calculate the offset from the mpd@availabilityStartTime that
 * should be used to start playback.
 */
double MPDParser::GetOffsetFromAvailabilityTimeToStartPlayback(bool bStartOfPlayback)
{
  QTV_NULL_PTR_CHECK(mpd, 0.0);

  MM_Time_DateTime sCurrTime;
  MM_Time_GetUTCTime(&sCurrTime);

  double nAvailabilityMSeconds = mpd->GetAvailabilityStartTime();
  double nCurrMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(sCurrTime);
  double offsetFromAvailTime = (nCurrMSeconds > nAvailabilityMSeconds
                                ? nCurrMSeconds - nAvailabilityMSeconds
                                : 0.0);

  double tsbToUse = bStartOfPlayback ? (double)GetTsbToUseAtStartUp() : (double)(GetTimeShiftBufferDepth()*1000);
  offsetFromAvailTime = (offsetFromAvailTime > tsbToUse
                         ? offsetFromAvailTime - tsbToUse
                         : 0.0);

  QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "MPDParser::GetOffsetFromAvailabilityTimeToStartPlayback(msec) availabilityStartTime %llu, currSysTime %llu, tsbToUse %llu, offsetFromAvailTime %llu: ",
                    (uint64)nAvailabilityMSeconds, (uint64)nCurrMSeconds, (uint64)tsbToUse, (uint64)offsetFromAvailTime);

  return (offsetFromAvailTime);
}

/**
 * Calculate the offset from the mpd@availabilityStartTime.
 *
 * @return time offset
 */
double MPDParser::GetOffsetFromAvailabilityTime()
{
  double offsetFromAvailTime = 0.0;

  MM_CriticalSection_Enter(m_pParserDataLock);

  if(NULL != mpd)
  {
    MM_Time_DateTime sCurrTime;
    MM_Time_GetUTCTime(&sCurrTime);

    double nAvailabilityMSeconds = mpd->GetAvailabilityStartTime();
    double nCurrMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(sCurrTime);
    offsetFromAvailTime = (nCurrMSeconds > nAvailabilityMSeconds
                                  ? nCurrMSeconds - nAvailabilityMSeconds
                                  : 0.0);
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "MPDParser::GetOffsetFromAvailabilityTime mpd is NULL");
  }

  MM_CriticalSection_Leave(m_pParserDataLock);

  return offsetFromAvailTime;
}

/**
 * @brief
 *  Search the playlist for the periodhich falls within
 *  starttime and update the current period to that.
 *
 * @return HTTPDownloadStatus
 */
HTTPDownloadStatus MPDParser::InitializePlaylistForStartTime(
  PeriodInfo& periodInfo, bool& bEOS, int64 startTime, bool bSearchFromStartOfPlaylist)
{
  bEOS = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(mpd,status);
  if (bSearchFromStartOfPlaylist)
  {
    ResetCurrentPeriod();
  }

  do
  {
    status = GetNextPeriod(NULL, bEOS);

    if ((HTTPCommon::HTTPDL_SUCCESS == status) && (false == bEOS))
    {
      bEOS = false;
      status = GetNextPeriod(&periodInfo, bEOS);

      if(!IsLive()) //static case
      {
      if (((uint64)startTime >= periodInfo.getStartTime()) &&
           ((uint64)startTime < periodInfo.getStartTime() + (uint64)(1000 * periodInfo.getDuration())))
      {
          break;
        }
      }
      else //dynamic case
      {
        if(0.0 == periodInfo.getDuration())
        {
          break; // Only last period can have unknown duration and that too for dynamic mpd only. Return this period.
        }

        uint64 nLastMaxSegDuration = 0;
        HTTPDownloadStatus ret = GetLastLargestSegmentDurationForPeriod(periodInfo, nLastMaxSegDuration);

        if(ret == HTTPCommon::HTTPDL_WAITING)
        {
          break; //Start time too early on period. Return this period.
        }

        /* If pause is issued at the end of the live stream (mpd duration is
          present) and is resumed if discontinuity is triggered i.e. seek will
          be issued and by the time segment is also past availability then ideally
          gracefull exit should happen
        */
        if (ret == HTTPCommon::HTTPDL_SUCCESS &&
            bSearchFromStartOfPlaylist &&
            mpd->getDuration() > 0 &&
            startTime >= (int64)mpd->getDuration()*1000)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM, "start time > duration for Live");
          status = HTTPCommon::HTTPDL_DATA_END;
          bEOS = true;
          break;
        }
        if(ret == HTTPCommon::HTTPDL_SUCCESS && (uint64)startTime < (periodInfo.getStartTime() + (uint64)(1000 * periodInfo.getDuration()) + nLastMaxSegDuration))
        {
          break;
        }
      }
    }

    if (true == bEOS)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "No period found for startTime %u, EOS reached on playlist parser",
                    (uint32)startTime);
      break;
    }
  } while (HTTPCommon::HTTPDL_SUCCESS == status);

  return status;
}
/* This function will provide the client with the representation’s information.
 * identified by the param key
 * @param[in] - key - representation's key
 * @param[out] - pRepresentation - Representation's information
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::GetRepresentationByKey(uint64 key,RepresentationInfo* &pRepresentation)
{
  bool rslt = false;

  if(m_bAbortSet)
  {
     QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "MPD parser task aborted");
  }
  else if(!mpdAvailable)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Waiting to get the mpd information from server");
  }
  else if(!mpd)
  {
     QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "mpd is NULL");
  }
  else
  {
    uint32 period_index = (uint32)((key & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
    uint32 grp_index  = (uint32)((key & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
    uint32 rep_index = (uint32)((key & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);

    int numPeriods = 0;
    PeriodInfo* pPeriodInfo=mpd->getPeriodInfo(numPeriods);

    uint32 start_period_index =
      (uint32)((uint64)(pPeriodInfo[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);

    if (period_index >= start_period_index)
    {
      period_index -= start_period_index;
    }

    if(pPeriodInfo && period_index < (uint32)numPeriods)
    {
      int numRepGrps = 0;
      RepresentationGroup* repgrpInfo=pPeriodInfo[period_index].getRepGrpInfo(numRepGrps);
      if(repgrpInfo && grp_index < (uint32)numRepGrps)
      {
        int numReps=0;
        RepresentationInfo *repInfo = repgrpInfo[grp_index].getRepInfo(numReps);
        if(repInfo && rep_index < (uint32)numReps)
        {
          pRepresentation = &repInfo[rep_index];
          rslt = true;
        }
      }
    }
  }

  return rslt;
}
/* This function will provide the client with all the representations for the
 * group identified by param GrpKey
 * @param[out] - pRepresentations - array of representations for the group
 * @param[in/out] - numReps - If the space is not sufficient numreps is
 *                  populated and sent to client.
 * @param[in] - Group key for which representation information is queried
 * @return - HTTPDownloadStatus
 * HTTPDL_SUCCESS - success
 * HTTPDL_WAITING - if update is going on.
 * HTTPDL_OUT_OF_MEMORY - if space is not sufficient
 * HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetAllRepresentationsForGroup(RepresentationInfo* pRepresentations,
                                                 uint32& numReps,uint64 GrpKey)
{
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(mpd,status);
  if(m_bAbortSet)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "MPD parser task aborted");
  }
  else if(!mpdAvailable)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Waiting to get the mpd information from server");
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
    uint32 period_index = (uint32)((GrpKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
    uint32 grp_index  = (uint32)((GrpKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
    int numPeriods = 0;
    PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);
    if(period_index < (uint32)numPeriods)
    {
      int numGrps = 0;
      RepresentationGroup* pRepGrp = period_info[period_index].getRepGrpInfo(numGrps);
      if(grp_index < (uint32)numGrps)
      {
        int numRepresentations = 0;
        RepresentationInfo *pReps = pRepGrp[grp_index].getRepInfo(numRepresentations);
        int numSelectedReps = 0;

        for(int i = 0; i < numRepresentations; ++i)
        {
          if(pReps[i].IsMarkedSelected())
          {
            ++numSelectedReps;
          }
        }

        int numSelectedCounter = 0;

        if(pRepresentations == NULL || numReps < (uint32)numSelectedReps )
        {
          status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
        }
        else if(numReps >= (uint32)numSelectedReps)
        {
          for(int i=0;i < numRepresentations;i++)
          {
            if(pReps[i].IsMarkedSelected())
            {
              pRepresentations[numSelectedCounter].Copy(pReps[i]);
              ++numSelectedCounter;
            }
          }
          status = HTTPCommon::HTTPDL_SUCCESS;
        }
        numReps = numSelectedReps;
      }
    }
  }
  return status;
}

/* This function will provide the client with sum of the Maximum bitrates for all the
 * representation groups for the period identified by param period_key
 * @param[out] - maxBitRate - Max Bitrate for the Period
 * @param[in] - period key for which Max bitrate information is queried
 * @return - HTTPDownloadStatus
 * HTTPDL_SUCCESS - success
 * HTTPDL_OUT_OF_MEMORY - if space is not sufficient
 * HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetMaxBitrateForPeriod(uint64 period_key, uint32& maxBitRate)
{
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  maxBitRate = 0;
  QTV_NULL_PTR_CHECK(mpd,status);
  if(m_bAbortSet)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "MPD parser task aborted");
  }
  else if(!mpdAvailable)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Waiting to get the mpd information from server");
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
    uint32 period_index = (uint32)((period_key & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
    int numPeriods = 0;
    PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);
    if (period_info)
    {
      int periodArrayIdx =
        (int)(period_index -
        (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

      if((periodArrayIdx >= 0) && (periodArrayIdx < numPeriods))
      {
        int numGrps = 0;
        uint32 maxAudioBandwidth = 0;
        uint32 maxVideoBandwidth = 0;
        uint32 maxTextBandwidth = 0;
        RepresentationGroup* rep_grps = period_info[periodArrayIdx].getRepGrpInfo(numGrps);
        if(NULL == rep_grps)
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
        else
        {
          for(int i=0;i<numGrps;i++)
          {
            if(rep_grps[i].IsAnyRepSelected())
            {
              {
                int numRepresentations = 0;
                RepresentationInfo *pReps = rep_grps[i].getRepInfo(numRepresentations);
                if(NULL == pReps)
                {
                  status = HTTPCommon::HTTPDL_ERROR_ABORT;
                }
                else
                {
                  for(int j=0;j<numRepresentations;j++)
                  {
                    if(pReps[j].IsMarkedSelected())
                    {
                      uint32 nMajorType = MJ_TYPE_UNKNOWN;
                      GetRepMajorType(pReps[j], nMajorType);
                      if(nMajorType != MJ_TYPE_UNKNOWN)
                      {
                        if(nMajorType & MJ_TYPE_VIDEO)
                        {
                          maxVideoBandwidth = STD_MAX(maxVideoBandwidth, pReps[j].getBandwidth());
                        }
                        else if(nMajorType & MJ_TYPE_AUDIO)
                        {
                          maxAudioBandwidth = STD_MAX(maxAudioBandwidth, pReps[j].getBandwidth());
                        }
                        else if(nMajorType & MJ_TYPE_TEXT)
                        {
                          maxTextBandwidth = STD_MAX(maxTextBandwidth, pReps[j].getBandwidth());
                        }

                        status = HTTPCommon::HTTPDL_SUCCESS;
                      }
                    }
                  }
                }
              }

              if(status != HTTPCommon::HTTPDL_SUCCESS)
              {
                break;
              }
            }
          }

          if(HTTPCommon::HTTPDL_SUCCESS == status)
          {
            maxBitRate = maxVideoBandwidth + maxAudioBandwidth + maxTextBandwidth;
          }
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "GetMaxBitrateForPeriod: Invalid periodArrayIdx %d",
                      periodArrayIdx);
      }
    }
  }
  return status;
}

/** @brief Gets major type for representation.
  *
  * @param[in] sRep - Reference to representation
  * @param[in] nMajorType - Major type bit mask
  * @return
  * TRUE - Success
  * FALSE - Failure
  */
bool MPDParser::GetRepMajorType
(
 RepresentationInfo& sRep,
 uint32& nMajorType
)
{
  bool bResult = false;
  int nNumCodecs = 0;

  nMajorType = MJ_TYPE_UNKNOWN;
  (void)sRep.getCodec(NULL, nNumCodecs);
  if (nNumCodecs > 0)
  {
    CodecInfo* pCodec = (CodecInfo *)QTV_Malloc(nNumCodecs * sizeof(CodecInfo));
    if (pCodec)
    {
      if (sRep.getCodec(pCodec, nNumCodecs))
      {
        bResult = true;
        for (int i = 0; i < nNumCodecs; i++)
        {
          nMajorType |= pCodec[i].majorType;
        }
      }
      QTV_Free(pCodec);
      pCodec = NULL;
    }
  }

  return bResult;
}

/* This function will provide the client with all the representation groups for the
 * period identified by param period_key
 * @param[out] - pRepGrps - array of representation grps for the period
 * @param[in/out] - numGroups - If the space is not sufficient numGroups is
 *                  populated and sent to client.
 * @param[in] - period key for which representation grp information is queried
 * @param[in] - bReturnSelectedOnly - return only repgrps that are marked selected.
 * @return - HTTPDownloadStatus
 * HTTPDL_SUCCESS - success
 * HTTPDL_WAITING - if update is going on.
 * HTTPDL_OUT_OF_MEMORY - if space is not sufficient
 * HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetAllRepGroupForPeriod(
   RepresentationGroup* pRepGrps, uint32& numGroups, uint64 period_key,
   bool bReturnSelectedOnly)
{
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(mpd,status);
  if(m_bAbortSet)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "MPD parser task aborted");
  }
  else if(!mpdAvailable)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Waiting to get the mpd information from server");
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
    uint32 period_index = (uint32)((period_key & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
    int numPeriods = 0;
    PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);
    if (period_info)
    {
      int periodArrayIdx =
        (int)(period_index -
        (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

      if((periodArrayIdx >= 0) && (periodArrayIdx < numPeriods))
      {
        int numGrps = 0;
        RepresentationGroup* rep_grps = period_info[periodArrayIdx].getRepGrpInfo(numGrps);

        int numSelectedRepGrps = 0;
        if(rep_grps)
        {
          for(int i = 0 ; i < numGrps; ++i)
          {
            if(rep_grps[i].IsAnyRepSelected() || (false == bReturnSelectedOnly))
            {
              ++numSelectedRepGrps;
            }
          }
        }

        if(pRepGrps == NULL || numGroups < (uint32)numSelectedRepGrps)
        {
          status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
        }
        else if(numGroups >= (uint32)numSelectedRepGrps)
        {
          int numSelectedCounter = 0;
          for(int i=0;i<numGrps;i++)
          {
            if(rep_grps[i].IsAnyRepSelected() || (false == bReturnSelectedOnly))
            {
              pRepGrps[numSelectedCounter].Copy(rep_grps[i]);
              ++numSelectedCounter;
            }
          }

          if (numSelectedCounter == numSelectedRepGrps)
          {
            status = HTTPCommon::HTTPDL_SUCCESS;
          }
          else
          {
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                          "GetAllRepGroupForPeriod: Sanity check failed %d != %d",
                          numSelectedCounter, numSelectedRepGrps);
          }
        }
        numGroups = numSelectedRepGrps;
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "GetAllRepGroupForPeriod: Invalid periodArrayIdx %d",
                      periodArrayIdx);
      }
    }
  }
  return status;
}


/**
 * Override the refresh interval for playlist downloading, so
 * that the next playlist is downloaded immediately.
 */
void MPDParser::ForceRefreshPlaylist()
{
  if (m_pDownloader)
  {
    m_pDownloader->UpdatePlaylist();
  }
}

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
void MPDParser::GetPropertiesXML(char *pString, int& size, const char *propertyNames)
{
  if(NULL == pString)
  {
    // query for size
    size = 0;
  }
  else
  {
    pString[0] = '\0';
  }

  bool bIncludeAll = false;

  if(propertyNames)
  {
    // will alwasys come here.
    bIncludeAll = (0 == std_stricmp(propertyNames, "") ? true : false);
  }

  bool bIncludeContentProtection = bIncludeAll;
  if(false == bIncludeAll && propertyNames)
  {
    if(std_strstri(propertyNames, "ContentProtection"))
    {
      bIncludeContentProtection = true;
    }
  }

  bool bIncludeID = bIncludeAll;
  if(false == bIncludeAll && propertyNames)
  {
    if(std_strstri(propertyNames, "id"))
    {
      bIncludeID = true;
    }
  }

  MPDParserXMLPropertiesHelper::PrintXMLStringForDebug tmpStr(pString);

  MPDParserXMLPropertiesHelper::XMLGeneratorForRootElem rootElem(pString, size, m_MpdProfileStr);

  // <xs:element name="MPDProperties" type="MPDPropertiesType" minOccurs="0" maxOccurs="unbounded"/>
  {
    MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpPropertiesString(
      pString,size,"MPDProperties");
    if(bIncludeAll)
    {
      MPDParserXMLPropertiesHelper::CreateXMLForString tmpProfiles(
        pString, size, "profiles", m_MpdProfileStr);
    }
  }

  int curPos = 0;

  int numPeriods = 0;
  PeriodInfo *period_Info = mpd->getPeriodInfo(numPeriods);

  if(period_Info && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      int numRepGrps = 0;
      RepresentationGroup *pRepGrps = period_Info[i].getRepGrpInfo(numRepGrps);

      if(pRepGrps && numRepGrps > 0)
      {
        int periodKey = (int)((uint64)(period_Info[i].getPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
        MPDParserXMLPropertiesHelper::XMLKeyTag periodTag(
          pString, size, "Period", periodKey);

        // <xs:element name="PeriodProperties" type="PeriodPropertiesType" minOccurs="0" maxOccurs="1"/>
        {
          MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpPropString(
                pString,size,"PeriodProperties");

          if(bIncludeID)
          {
            const char *id = period_Info[i].getPeriodIdentifier();
            if(id)
            {
              MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                pString,size,"id", id);
            }
          }
        }

        for(int j = 0; j < numRepGrps; ++j)
        {
          int numReps = 0;
          RepresentationInfo *repInfo = pRepGrps[j].getRepInfo(numReps);

          if(repInfo && numReps > 0)
          {
            int adaptationSetKey = (int)((uint64)(pRepGrps[j].getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
            MPDParserXMLPropertiesHelper::XMLKeyTag adaptationTag(
              pString, size, "AdaptationSet", adaptationSetKey);

            {
              MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpPropString(
                pString,size,"AdaptationSetProperties");

              if(bIncludeAll)
              {
                /*
                  <xs:element name="AudioChannelConfiguration" type="DescriptorType" minOccurs="0" />
                  <xs:element name="mimeType" type="xs:string" minOccurs="0"/>
                  <xs:element name="codecs" type="xs:string" maxOccurs="unbounded" minOccurs="0"/>
                  <xs:element name="width" type="xs:unsignedInt" minOccurs="0"/>
                  <xs:element name="height" type="xs:unsignedInt" minOccurs="0"/>
                  <xs:element name="ContentProtection" type="ContentProtectionType" minOccurs="0"/>
                */
                {
                  MPDParserXMLPropertiesHelper::XMLGeneratorForContentDescriptorContainer
                    generateAdaptionSetContentDescriptorsXML(
                      pString,size, pRepGrps[j].GetCommonContentDescriptors());
                }

                const char *pValue =
                  pRepGrps[j].GetCommonStringValueContainer().FindString("mimeType");

                if(pValue)
                {
                  MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                    pString,size,"mimeType", pValue);
                }

                pValue = pRepGrps[j].GetCommonStringValueContainer().FindString("codecs");
                if(pValue)
                {
                  MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                    pString,size,"codecs", pValue);
                }

                Resolution *res = pRepGrps[j].getResolution();
                if(res)
                {
                  char tmpBuf[20];

                  std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->width);
                  {
                    MPDParserXMLPropertiesHelper::CreateXMLForString
                      generateXMLForString(pString,size,"width", tmpBuf);
                  }

                  std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->height);
                  {
                    MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                      pString,size,"height", tmpBuf);
                  }
                }
              }

              if(bIncludeContentProtection)
              {
                MPDParserXMLPropertiesHelper::XMLGeneratorForContentProtectionElement
                  generateXMLGeneratorForContentProtectionElement(pString,size,pRepGrps[j].GetContentProtection());
              }

              /*
                    <xs:element name="id" type="xs:string" minOccurs="0" />
                    <xs:element name="Accessibility" type="DescriptorType" minOccurs="0" maxOccurs="unbounded"/>
                    <xs:element name="Role" type="DescriptorType" minOccurs="0" maxOccurs="unbounded"/>
                    <xs:element name="contentType" type="xs:string" minOccurs="0"/>
                    <xs:element name="lang" type="xs:language" minOccurs="0"/>
              */

              if(bIncludeID)
              {
                const char *id = pRepGrps[j].GetGrpIdentifer();
                if(id)
                {
                  MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                    pString,size,"id", id);
                }
              }

              if(bIncludeAll)
              {
                {
                  MPDParserXMLPropertiesHelper::XMLGeneratorForContentDescriptorContainer
                    generateAdaptionSetContentDescriptorsXML(
                      pString,size, pRepGrps[j].GetAdaptationSetSpecificContentDescriptors());
                }

                {
                  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                "StringValues at adap-set for repGrp with idx %d", j);
                  pRepGrps[j].GetAdaptationSetSpecificStringValueContainer().PrintStringValues();
                  MPDParserXMLPropertiesHelper::XMLGeneratorForStringValueContainer tmp(
                    pString,size,pRepGrps[j].GetAdaptationSetSpecificStringValueContainer());
                }

                {
                  MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                    pString,size,"lang", pRepGrps[j].getLanguage());
                }
              }

            }

            for(int k = 0; k < numReps; ++k)
            {
              int repKey = (int)((uint64)(repInfo[k].getKey() & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT);
              MPDParserXMLPropertiesHelper::XMLKeyTag representationTag(
                pString, size, "Representation", repKey);

              MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpProps(
                  pString,size,"RepresentationProperties");

              if(bIncludeAll)
              {
                /*
                <xs:element name="AudioChannelConfiguration" type="DescriptorType" minOccurs="0" />
                <xs:element name="mimeType" type="xs:string" minOccurs="0"/>
                <xs:element name="codecs" type="xs:string" maxOccurs="unbounded" minOccurs="0"/>
                <xs:element name="width" type="xs:unsignedInt" minOccurs="0"/>
                <xs:element name="height" type="xs:unsignedInt" minOccurs="0"/>
                <xs:element name="ContentProtection" type="ContentProtectionType" minOccurs="0"/>
                */

                {
                  // this returns only AudioChannelConfiguration for now.
                  MPDParserXMLPropertiesHelper::XMLGeneratorForContentDescriptorContainer
                    generateCommonContentDescriptorsXML(
                      pString,size, repInfo[k].GetCommonDescriptors());
                }

                {
                  const char *pValue = NULL;
                  pValue = repInfo[k].GetCommonStringValues().FindString("mimeType");
                  if(pValue)
                  {
                    MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                      pString,size,"mimeType", pValue);
                  }

                  pValue = repInfo[k].GetCommonStringValues().FindString("codecs");
                  if(pValue)
                  {
                    MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                      pString,size,"codecs", pValue);
                  }
                }

                Resolution *res = repInfo[k].getResolution();
                if(res)
                {
                  char tmpBuf[20];

                  std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->width);
                  {
                    MPDParserXMLPropertiesHelper::CreateXMLForString
                      generateXMLForString(pString,size,"width", tmpBuf);
                  }

                  std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->height);
                  {
                    MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                      pString,size,"height", tmpBuf);
                  }
                }
              }

              if(bIncludeContentProtection)
              {
                MPDParserXMLPropertiesHelper::XMLGeneratorForContentProtectionElement
                  XMLGeneratorForContentProtectionElement(pString,size,repInfo[k].GetContentProtection());
              }

                /*
                <xs:element name="id" type="xs:string" minOccurs="0"/>
                <xs:element name="bandwidth" type="xs:unsignedInt" minOccurs="0"/>
                */
              if(bIncludeID)
              {
                const char *id = repInfo[k].getRepIdentifier();
                if(id)
                {
                  MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                    pString,size,"id", id);
                }
              }

              if(bIncludeAll)
              {
                if(repInfo[k].getBandwidth())
                {
                  char tmpBuf[20];
                  std_strlprintf(tmpBuf, sizeof(tmpBuf), "%d", (int)repInfo[k].getBandwidth());
                  MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
                    pString,size,"bandwidth", tmpBuf);
                }
              }

            }
          }
          else
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "GetPropertiesXML: Failed to get repInfo");
          }
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "GetPropertiesXML: Failed to get repGrpInfo");
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "GetPropertiesXML: Failed to get periodInfo");
  }
}

/**
 * Parses the xml and marks all the specified representations as
 * selectable. And then, marks a subset of the 'selectable'
 * representations as 'selected'.
 */
bool MPDParser::SetSelectionsXML(const char *pXMLSelectedString)
{
  bool bOk = false;
  MarkAllRepGrpsAsSelectable(false);

  if(pXMLSelectedString)
  {
    TiXmlDocument xmlStream;
    const char *pRet = xmlStream.Parse(pXMLSelectedString);
    TiXmlElement* elem = xmlStream.FirstChildElement();

    // this should be the "MPD"
    if(elem)
    {
      IPStreamList<KeyStruct> keyList;
      bOk = ParseKeysFromXML(elem, keyList);

      for(IPStreamList<KeyStruct>::Iterator iter = keyList.Begin();
           iter != keyList.End();
           iter = iter.Next())
      {
        const KeyStruct& keyStruct = *iter;
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "SetSelectionsXML (%d,%d,%d)",
                      keyStruct.m_PeriodKey,
                      keyStruct.m_AdaptationSetKey,
                      keyStruct.m_RepresentationKey);

        MarkRepresentationAsSelectable(
          keyStruct.m_PeriodKey, keyStruct.m_AdaptationSetKey, keyStruct.m_RepresentationKey);
      }

      EnsureAllPeriodsSelected();
      DoContentSelection();
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "SetSelectionsXML Null elem");
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "NULL selected xml string");
  }

  PrintSelectedReps();

  return bOk;
}

/**
 * Given an xml string of mpd selections, extract the repgrpKeys
 * from it into repGrpXMLKeySet.
 */
bool MPDParser::GetRepGrpKeysForAdaptationSetChange(uint64 nPeriodKey,
                                                    const char *pXMLSelectedString,
                                                    IPStreamList<uint64>& repGrpXMLKeySet)
{
  bool bOk = true;

  static const int MAX_ARR_SZ = 10;
  int repGrpArray[MAX_ARR_SZ];
  int arraySizeInUse = 0;

  for(int i = 0; i < MAX_ARR_SZ; ++i)
  {
    repGrpArray[i] = 0;
  }

  if(pXMLSelectedString)
  {
    TiXmlDocument xmlStream;
    const char *pRet = xmlStream.Parse(pXMLSelectedString);
    TiXmlElement* elem = xmlStream.FirstChildElement();

    // this should be the "MPD"
    if(elem)
    {
      IPStreamList<KeyStruct> keyList;
      bOk = ParseKeysFromXML(elem, keyList);

      for(IPStreamList<KeyStruct>::Iterator iter = keyList.Begin();
           iter != keyList.End();
           iter = iter.Next())
      {
        const KeyStruct& keyStruct = *iter;

        if(nPeriodKey > MAX_INT32)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Invalid period key %llu", nPeriodKey);
          continue;
        }

        if(keyStruct.m_PeriodKey != (int)nPeriodKey)
        {
          continue;
        }

        bool bFound = false;
        for(int i = 0; i < arraySizeInUse; ++i)
        {
          if(repGrpArray[i] == keyStruct.m_AdaptationSetKey)
          {
            bFound = true;
            break;
          }
        }

        if(false == bFound)
        {
          repGrpArray[arraySizeInUse] = keyStruct.m_AdaptationSetKey;
          arraySizeInUse++;
        }

        if(MAX_ARR_SZ == arraySizeInUse)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "arraySizeInUse is MAX_ARR_SZ");
          break;
        }
      }
    }
  }

  for(int i = 0; i < arraySizeInUse; ++i)
  {
    repGrpXMLKeySet.Push(repGrpArray[i]);
  }

  return bOk;
}

/**
 * Modify the track selections due to dynamic adaptation-set
 * changes.
 */
bool MPDParser::ModifySelectedRepresentations(
  uint32 nPeriodIdentifer, uint32 nAdaptationSetIdentifier, IPStreamList<uint32>& listRepIds, bool bIsSelected)
{
  bool bIsModified = false;

  uint64 nTmpPeriodId = nPeriodIdentifer;
  uint64 nTmpAdaptationSetId = nAdaptationSetIdentifier;
  uint64 nAdaptationSetKey = (nTmpPeriodId << MPD_PERIOD_SHIFT_COUNT) || (nTmpAdaptationSetId << MPD_REPGRP_SHIFT_COUNT);

  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(periodInfo && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      if(nPeriodIdentifer == ((periodInfo[i].getPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT))
      {
        int numRepgrps = 0;
        RepresentationGroup *repGrpInfo = periodInfo[i].getRepGrpInfo(numRepgrps);
        if(repGrpInfo && numRepgrps > 0)
        {
          for(int j = 0; j < numRepgrps; ++j)
          {
            if(nAdaptationSetIdentifier == ((repGrpInfo[j].getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT))
            {
              int numReps = 0;
              RepresentationInfo *repInfo = repGrpInfo[j].getRepInfo(numReps);

              if(repInfo && numReps > 0)
              {
                bIsModified = true;

                // first de-select all representations for this adaptation-set.
                for(int k = 0; k < numReps; ++k)
                {
                    repInfo[k].MarkSelectable(false);
                    repInfo[k].MarkSelected(false);
                }

                if(bIsSelected)
                {
                  if(listRepIds.IsEmpty())
                  {
                    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "ModifySelectedRepresentations: Selected all reps for PeriodXmlId %u and AdaptationSetXmlId %u",
                      nPeriodIdentifer, nAdaptationSetIdentifier);

                    for(int k = 0; k < numReps; ++k)
                    {
                      repInfo[k].MarkSelectable(bIsSelected);
                      repInfo[k].MarkSelected(bIsSelected);
                    }
                  }
                  else
                  {
                    for(int k = 0; k < numReps; ++k)
                    {
                      IPStreamList<uint32>::Iterator iterEnd = listRepIds.End();

                      for(IPStreamList<uint32>::Iterator iter = listRepIds.Begin();
                           iter != iterEnd; iter = iter.Next())
                      {
                        uint32 nRepId = *iter;

                        if(((repInfo[k].getKey() & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT) == nRepId)
                        {
                          repInfo[k].MarkSelectable(bIsSelected);
                          repInfo[k].MarkSelected(bIsSelected);

                          QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                            "ModifySelectedRepresentations: Selected representation %s in with periodIdx %d, repGrpIdx %d",
                            repInfo[k].getRepIdentifier() ? repInfo[k].getRepIdentifier() : "Unknown",
                            (int)nPeriodIdentifer, (int)nRepId);
                        }
                      }
                    }
                  }
                }
              }
              else
              {
                QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Failed to find a representation in adapatation "
                  "set with periodId %d, repGrpId %d",
                (int)nPeriodIdentifer, (int)nAdaptationSetIdentifier);
              }

              break;
            }
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Did not find any valid adaptation set");
        }
        break;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Did not find any period");
  }

  return bIsModified;
}

/**
 * Determine if the rep associated with nRepKey is of VOD
 * profile.
 */
bool MPDParser::IsRepVODProfile(uint64 nRepKey)
{
  bool bRslt = false;

  uint32 period_index = (uint32)((nRepKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  uint32 grp_index  = (uint32)((nRepKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
  uint32 rep_index  = (uint32)((nRepKey & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
  int numPeriods = 0;
  PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);

  if(period_info && numPeriods > 0)
  {
    int periodArrayIdx =
      (int)(period_index - (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

    if (periodArrayIdx >= 0 && periodArrayIdx < numPeriods)
    {
      int numGrps = 0;
      PeriodInfo& rPeriodInfo = period_info[periodArrayIdx];
      RepresentationGroup* pRepGrp = rPeriodInfo.getRepGrpInfo(numGrps);

      if(pRepGrp && grp_index < (uint32)numGrps)
      {
        int numRepresentations = 0;
        RepresentationInfo *pReps = pRepGrp[grp_index].getRepInfo(numRepresentations);
        if(pReps && rep_index < (uint32)numRepresentations)
        {
          uint32 nNumSegments = 0,size=0;
          RepresentationInfo& rRepInfo = pReps[rep_index];

          if(rRepInfo.GetSegmentTemplate() == NULL && rRepInfo.GetSegmentList() == NULL)
          {
            // has only base URL and must have only one segment.
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "VOD profile for repkey %llu with base-url only", nRepKey);

            bRslt = true;
          }
        }
      }
    }
  }

  return bRslt;
}

/**
 * Get the major-type of the adaptation-set.
 */
bool MPDParser::GetGroupMajorType
(
 RepresentationGroup& sRepGroup,
 uint32& nMajorType
)
{
  bool bResult = false;
  int nNumCodecs = 0;

  nMajorType = MJ_TYPE_UNKNOWN;
  (void)sRepGroup.getCodec(NULL, nNumCodecs);
  if (nNumCodecs > 0)
  {
    CodecInfo* pCodec = (CodecInfo *)QTV_Malloc(nNumCodecs * sizeof(CodecInfo));
    if (pCodec)
    {
      if (sRepGroup.getCodec(pCodec, nNumCodecs))
      {
        bResult = true;
        for (int i = 0; i < nNumCodecs; i++)
        {
          if (pCodec[i].majorType == MJ_TYPE_AUDIO)
          {
            nMajorType |= MJ_TYPE_AUDIO;
          }
          else if (pCodec[i].majorType == MJ_TYPE_VIDEO)
          {
            nMajorType |= MJ_TYPE_VIDEO;
          }
          else if (pCodec[i].majorType == MJ_TYPE_TEXT)
          {
            nMajorType |= MJ_TYPE_TEXT;
          }
        }
      }
      QTV_Free(pCodec);
      pCodec = NULL;
    }
  }

  return bResult;
}


/* This function computes first available segment start time and end time for the
* representation.
* When segment information for representation is either of type segment base,
* segment list with/without timeline or segment template with timeline SegmentFuncDefault
* path API's are used for available segment times calculations where segment info
* structures are already available
*/
HTTPDownloadStatus RepresentationInfo::SegmentFuncDefault::GetFirstAvailableSegmentTimeForRepresentation
(
  MPDParser *pMPDParser,
  PeriodInfo* pPeriodInfo,
  RepresentationInfo* pRepInfo,
  double nCurrMSeconds,
  bool bStartOfPlayback,
  uint64& nFirstAvailableSegmentStartTime,
  uint64& nFirstAvailableSegmentEndTime
)
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  double tsbToUse = bStartOfPlayback ? (double)pMPDParser->GetTsbToUseAtStartUp() : (double)pMPDParser->GetTimeShiftBufferDepth()*1000;
  nCurrMSeconds = (nCurrMSeconds > tsbToUse ? nCurrMSeconds - tsbToUse : 0.0);

  MPD *mpd = pMPDParser->mpd;
  QTV_NULL_PTR_CHECK(mpd, HTTPCommon::HTTPDL_ERROR_ABORT);

  double nAvailabilityMSeconds = (mpd ? mpd->GetAvailabilityStartTime() : 0.0);

  SegmentInfo *segmentArray;

  uint32 numSegments = 0, tmpSize = 0;
  segmentArray = pRepInfo->getSegmentInfo(numSegments, tmpSize);

  if (segmentArray && numSegments > 0)
  {
    if(!pMPDParser->IsLive())
    {
      /*VOD case. First segment start time.*/
      nFirstAvailableSegmentStartTime = (uint64)segmentArray[0].getStartTime();
      nFirstAvailableSegmentEndTime = (uint64)segmentArray[0].getStartTime() + (uint64)segmentArray[0].getDuration();

      status = HTTPCommon::HTTPDL_SUCCESS;
    }
    else
    {
      uint32 index = 0;
      double availPlusPeriodStart = nAvailabilityMSeconds + (double)pPeriodInfo->getStartTime();

      if (nCurrMSeconds < (availPlusPeriodStart + segmentArray[0].getStartTime() + segmentArray[0].getDuration()))
      {
        status = HTTPCommon::HTTPDL_WAITING;
      }
      else if (nCurrMSeconds >=
        (availPlusPeriodStart + segmentArray[numSegments-1].getStartTime() + segmentArray[numSegments-1].getDuration()))
      {
        status = HTTPCommon::HTTPDL_DATA_END;
      }
      else
      {
        for (uint32 index = 0; index < numSegments; ++index)
        {
          double curSegmentAvailStartTime = availPlusPeriodStart + segmentArray[index].getStartTime() + segmentArray[index].getDuration();

          if ((curSegmentAvailStartTime <= nCurrMSeconds) &&
            (nCurrMSeconds < (curSegmentAvailStartTime + segmentArray[index].getDuration())))
          {
            nFirstAvailableSegmentStartTime = (uint64)segmentArray[index].getStartTime();
            nFirstAvailableSegmentEndTime = (uint64)segmentArray[index].getStartTime() + (uint64)segmentArray[index].getDuration();

            status = HTTPCommon::HTTPDL_SUCCESS;
            break;
          }
        }
      }
    }
  }

  return status;
}


/* This function provides first available segment start time and end time for the
* period identified by period key from the current system time
* @param[in] - nPeriodKey - period key for which segment information is queried
* @param[in] - bStartOfPlayback - flag indicates if api called during start of playback.
*              Accordingly uses actual TSB value or tsbToUseAtStartUp value
* @param[out] - nLastAvailableSegmentStartTime - Start time of the last available segment
* @param[out] - nLastAvailableSegmentEndTime - End time of the last available segment
* @return - HTTPDownloadStatus
* HTTPDL_SUCCESS - success
* HTTPDL_WAITING - if none of the segments are available
* HTTPDL_DATA_END - if all segments past their availability times
* HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetFirstAvailableSegmentTimeForPeriod
(
  uint64 nPeriodKey,
  bool bStartOfPlayback,
  uint64& nFirstAvailableSegmentStartTime,
  uint64& nFirstAvailableSegmentEndTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  uint32 nPeriodIdx = (uint32)((nPeriodKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  int nNumPeriods = 0;

  PeriodInfo* pPeriodInfo = mpd->getPeriodInfo(nNumPeriods);

  if (pPeriodInfo)
  {
    int nPeriodArrayIdx =
      (int)(nPeriodIdx - (uint32)((uint64)(pPeriodInfo[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

    if (nPeriodArrayIdx >=0 && nPeriodArrayIdx < nNumPeriods)
    {
      nFirstAvailableSegmentStartTime = MAX_UINT64;
      nFirstAvailableSegmentEndTime = MAX_UINT64;

      int numRepGrps = 0;
      RepresentationGroup *repGrpArray = pPeriodInfo->getRepGrpInfo(numRepGrps);

      if (repGrpArray && numRepGrps > 0)
      {
        MM_Time_DateTime sCurrTime;
        MM_Time_GetUTCTime(&sCurrTime);
        double nCurrMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(sCurrTime);

        bool bIsCurrentTimeTooEarlyOnAllReps = IsLive() ? true : false;
        bool bIsCurrentTimeTooLateOnAllReps = IsLive() ? true : false;

        for (int j = 0; j < numRepGrps; ++j)
        {
          int numReps = 0;
          RepresentationInfo *repArray = repGrpArray[j].getRepInfo(numReps);

          if (repArray && numReps > 0)
          {
            for (int k = 0; k < numReps; ++k)
            {
              uint64 nTmpStartTime = MAX_UINT64;
              uint64 nTmpEndTime = MAX_UINT64;

              HTTPDownloadStatus ret = repArray[k].GetSegmentFunc()->GetFirstAvailableSegmentTimeForRepresentation(this,
                &pPeriodInfo[nPeriodArrayIdx],
                &repArray[k],
                nCurrMSeconds,
                bStartOfPlayback,
                nTmpStartTime,
                nTmpEndTime);

              if(HTTPCommon::HTTPDL_ERROR_ABORT == ret)
              {
                continue; //Continue to other representations
              }

              if(HTTPCommon::HTTPDL_WAITING != ret)
              {
                bIsCurrentTimeTooEarlyOnAllReps = false;

                if(HTTPCommon::HTTPDL_DATA_END != ret)
                {
                  bIsCurrentTimeTooLateOnAllReps = false;
                  if (HTTPCommon::HTTPDL_SUCCESS == ret && (MAX_UINT64 == nFirstAvailableSegmentStartTime || (nTmpStartTime < nFirstAvailableSegmentStartTime)))
                  {
                    nFirstAvailableSegmentStartTime = nTmpStartTime;
                    nFirstAvailableSegmentEndTime = nTmpEndTime;
                    status = HTTPCommon::HTTPDL_SUCCESS;
                  }
                }
              }
            }

            if(true == bIsCurrentTimeTooEarlyOnAllReps)
            {
              nFirstAvailableSegmentStartTime = 0;
              nFirstAvailableSegmentEndTime = 0;
              status = HTTPCommon::HTTPDL_WAITING;
            }
            else if(true == bIsCurrentTimeTooLateOnAllReps)
            {
              status = HTTPCommon::HTTPDL_DATA_END;
            }
          }
        }
      }
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "MPDParser::GetFirstAvailableSegmentTimeForPeriod period key %lu, periodStartTime %llu, segmentStartTime %llu",
        (uint32)((nPeriodKey & (uint64)MPD_PERIOD_MASK) >> 56),  pPeriodInfo[nPeriodArrayIdx].getStartTime(), nFirstAvailableSegmentStartTime);
    }
  }

  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
    "MPDParser::GetFirstAvailableSegmentTimeForPeriod period key %lu, status %d",
    (uint32)((nPeriodKey & (uint64)MPD_PERIOD_MASK) >> 56), status);


  return status;
}


/* This function provides first available segment start time and end time across
* all periods
* @param[in] - bStartOfPlayback - flag indicates if api called during start of playback.
*              Accordingly uses actual TSB value or tsbToUseAtStartUp value
* @param[out] - nLastAvailableSegmentStartTime - Start time of the last available segment
* @param[out] - nLastAvailableSegmentEndTime - End time of the last available segment
* @return - HTTPDownloadStatus
* HTTPDL_SUCCESS - success
* HTTPDL_WAITING - if none of the segments are available
* HTTPDL_DATA_END - if all segments past their availability times
* HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetFirstAvailableSegmentTimeForPlayback
(
  bool bStartOfPlayback,
  uint64& nFirstAvailableSegmentStartTime,
  uint64& nFirstAvailableSegmentEndTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  MM_CriticalSection_Enter(m_pParserDataLock);

  if(NULL != mpd)
  {
    int numPeriods = 0;
    PeriodInfo *pPeriodInfo = mpd->getPeriodInfo(numPeriods);

    if(pPeriodInfo && numPeriods > 0)
    {
      int i = 0;
      for(i = 0; i < numPeriods; ++i)
      {
        status = GetFirstAvailableSegmentTimeForPeriod(pPeriodInfo[i].getPeriodKey(),
          bStartOfPlayback,
          nFirstAvailableSegmentStartTime,
          nFirstAvailableSegmentEndTime);

        if(status == HTTPCommon::HTTPDL_DATA_END)
        {
          continue;
        }
        else
        {
          break;
        }
      }

      if(HTTPCommon::HTTPDL_SUCCESS == status || HTTPCommon::HTTPDL_WAITING == status)
      {
        nFirstAvailableSegmentStartTime = pPeriodInfo[i].getStartTime() + nFirstAvailableSegmentStartTime;
        nFirstAvailableSegmentEndTime = pPeriodInfo[i].getStartTime() + nFirstAvailableSegmentEndTime;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "MPDParser::GetFirstAvailableSegmentTimeForPlayback mpd is NULL");
  }

  MM_CriticalSection_Leave(m_pParserDataLock);

  return status;
}


HTTPDownloadStatus MPDParser::GetLastLargestSegmentDurationForPeriod(PeriodInfo& rPeriodInfo, uint64 &nLastMaxSegDuration)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDownloadStatus retStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

  int numRepGrps = 0;
  RepresentationGroup *repGrpArray = rPeriodInfo.getRepGrpInfo(numRepGrps);


  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "MPDParser::GetLastLargestSegmentDurationForPeriod" );

  if (repGrpArray)
  {
    for (int j = 0; j < numRepGrps; ++j)
    {
      int numReps = 0;
      RepresentationInfo *repArray = repGrpArray[j].getRepInfo(numReps);

      if (repArray)
      {
        for (int k = 0; k < numReps; ++k)
        {
          uint64 nLastSegKey = MAX_UINT64_VAL;
          uint64 segDuration = 0;

          if(repArray[k].GetSegmentFunc())
          {
            status = repArray[k].GetSegmentFunc()->GetLastSegmentKeyForRepresentation(this, &rPeriodInfo, &nLastSegKey, &repArray[k]);

            if(status == HTTPCommon::HTTPDL_SUCCESS)
            {
              status = repArray[k].GetSegmentFunc()->GetSegDurationForRepresentation(this, &repArray[k], nLastSegKey, segDuration);
              nLastMaxSegDuration = QTV_MAX(nLastMaxSegDuration, segDuration);
            }

            if(status != HTTPCommon::HTTPDL_ERROR_ABORT)
            {
              retStatus = status;
            }
          }
        }
      }
    }
  }

  return retStatus;
}

/* This function returns true if the segment identified by param nKey is available
 * on server
 * @param[in] - nKey - Key for the segment
 * @return - bool
 * true if segment is available false otherwise
*/
bool MPDParser::IsSegmentAvailable(uint64 nKey)
{
  bool bRet = false;
  if(IsLive())
  {
    MM_Time_DateTime currentTime;
    MM_Time_GetUTCTime(&currentTime);

    MM_CriticalSection_Enter(m_pParserDataLock);

    double availabilityMSeconds = (mpd ? mpd->GetAvailabilityStartTime() : 0.0);
    double currMSeconds =
    StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);

    double mpdDuration = (mpd ? mpd->getDuration() * ((double)1000): 0.0);
    uint32 periodKey = (uint32)((nKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
    uint32 grp_index  = (uint32)((nKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
    uint32 rep_index = (uint32)((nKey & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
    uint32 segmentKey = (uint32)((nKey & MPD_SEGMENT_MASK));

    int numPeriods = 0;
    PeriodInfo* pPeriodInfo= (mpd ? mpd->getPeriodInfo(numPeriods) : NULL);

    if (pPeriodInfo)
    {
      int periodIndex = (int)(periodKey - ((pPeriodInfo[0].getPeriodKey()& MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT));

      if((periodIndex >= 0) && (periodIndex < numPeriods))
      {
        uint64 periodStartTime = pPeriodInfo[periodIndex].getStartTime();
        int numRepGrps = 0;
        RepresentationGroup* repgrpInfo=pPeriodInfo[periodIndex].getRepGrpInfo(numRepGrps);
        if(repgrpInfo && grp_index < (uint32)numRepGrps)
        {
          int numReps=0;
          RepresentationInfo *repInfo = repgrpInfo[grp_index].getRepInfo(numReps);
          if(repInfo && rep_index < (uint32)numReps)
          {
            SegmentInfo segInfo;
            bool bOk = GetSegmentInfoByKey(nKey, segInfo);

            if(bOk)
            {
              // Lower bound time to restrict accessible segments taking into account
              // the time shift buffer.
              double lowerBoundTime = currMSeconds - (repInfo[rep_index].GetTimeShiftBufferDepth() * 1000) - segInfo.getDuration() ;
              MM_Time_DateTime fetchTime = GetFetchTime();
              double checkTimeForMPD = (StreamSourceTimeUtils::ConvertSysTimeToMSec(fetchTime) +
                                         1000 * repInfo[rep_index].GetMinUpdatePeriod());

              // Upper bound time to restrict accessible segments taking into account
              // current time and mpd checkTime as defined in the spec under the title
              // "Media Segment list restrictions"
              double upperboundTime = (repInfo[rep_index].GetMinUpdatePeriod() > 0.0
                                       ? QTV_MIN(checkTimeForMPD, currMSeconds)
                                      : currMSeconds);

              double segAvailStartTime=availabilityMSeconds + segInfo.getDuration() + (double)periodStartTime + (double)segInfo.getStartTime();


              //Check segment start time for availability
              if((segAvailStartTime >= lowerBoundTime) && (segAvailStartTime <= upperboundTime))
              {
                bRet = true;
              }
              else
              {
                QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Segment with starttime %lf is past availability time %lf", segAvailStartTime,upperboundTime);
              }
            }
          }
        }
      }
    }

    MM_CriticalSection_Leave(m_pParserDataLock);
  }
  else
  {
   //For VOD case segments are always available
    bRet = true;
  }
  return bRet;
}

bool MPDParser::IsLastPeriod(uint64 nKey)
{
  bool bLast = false;

  if(mpd)
  {
    if(mpd->IsLastPeriod(nKey))
    {
      bLast = mpd->IsLastPeriod(nKey);
    }
  }

  return bLast;
}

/* This function returns true if the segment identified by param nKey is the last segment
 * in the mpd
 * @param[in] - nKey - Key for the segment
 * @return - bool
 * true if segment is last segment false otherwise
*/
bool MPDParser::IsLastSegment(uint64 nKey)
{
  bool rslt = false;
  uint32 periodKey = (uint32)((nKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  uint32 grp_index  = (uint32)((nKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
  uint32 rep_index = (uint32)((nKey & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
  uint32 segmentKey = (uint32)((nKey & MPD_SEGMENT_MASK));

  int numPeriods = 0;

  MM_CriticalSection_Enter(m_pParserDataLock);

  PeriodInfo* pPeriodInfo=mpd->getPeriodInfo(numPeriods);

  if (pPeriodInfo)
  {
    int periodIndex = (int)(periodKey - ((pPeriodInfo[0].getPeriodKey()& MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT));

    if((periodIndex >= 0) && (periodIndex < numPeriods))
    {
      SegmentInfo segInfo;
      bool bOk = GetSegmentInfoByKey(nKey, segInfo);

      if(bOk)
      {
        if ((pPeriodInfo[periodIndex].getDuration() > 0.0 &&
             (segInfo.getStartTime() + segInfo.getDuration())
             >= pPeriodInfo[periodIndex].getDuration()*1000))
        {
          QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "IsLastSegment true for period %u (idx %d) duration %u, segStart %u, segDuration %u",
            periodKey, periodIndex, (uint32)pPeriodInfo[periodIndex].getDuration(),
            (uint32)segInfo.getStartTime(),
            (uint32)segInfo.getDuration());

          rslt = true;
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "IsLastSegment: Failed to get segmentinfo for key %llu", nKey);
      }
    }
  }

  MM_CriticalSection_Leave(m_pParserDataLock);

  return rslt;
}

/* This function returns duration of the specified rep
 *
 * @param[in] - nKey - Key for the rep
 * @return - duration
*/
double MPDParser::GetDuration(const uint64 nRepKey)
{
  double duration = 0.0;
  uint32 periodKey = (uint32)((nRepKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  uint32 grp_index  = (uint32)((nRepKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
  uint32 rep_index = (uint32)((nRepKey & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);

  //Rep duration is the same as the containing period duration in concept. Note
  //that even for live with updates (and with possibly multiple periods), the
  //present period duration can be deduced given the start of ensuing period or
  //total presentation duration!
  int numPeriods = 0;

  MM_CriticalSection_Enter(m_pParserDataLock);
  PeriodInfo* pPeriodInfo=mpd->getPeriodInfo(numPeriods);

  if (pPeriodInfo)
  {
    int periodIndex = (int)(periodKey - ((pPeriodInfo[0].getPeriodKey()& MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT));

    if((periodIndex >= 0) && (periodIndex < numPeriods))
    {
      duration = 1000 * pPeriodInfo[periodIndex].getDuration();
    }
  }

  MM_CriticalSection_Leave(m_pParserDataLock);

  return duration;
}


HTTPDownloadStatus RepresentationInfo::SegmentFuncDefault::GetLastSegmentKeyForRepresentation(MPDParser* /*pMPDParser*/, PeriodInfo* pPeriodInfo, uint64* pSegmentInfo, RepresentationInfo* pRepInfo )
{
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  uint32 nNumSegments = 0,size=0;
  SegmentInfo* pSegment = pRepInfo->getSegmentInfo(nNumSegments,size);
  if(pSegment)
  {
    int32 lastAvailableSegmentIndex = -1;
    uint32 numberOfAvailableSegments = 0;

    numberOfAvailableSegments = nNumSegments;
    lastAvailableSegmentIndex = (int32)nNumSegments - 1;

    if(pSegmentInfo)
    {
      *pSegmentInfo = pSegment[lastAvailableSegmentIndex].getKey();
      status = HTTPCommon::HTTPDL_SUCCESS;
    }
  }//if(pSegment)
  else
  {
    // if segment array was not created, then non of the segments
    // fall in the availability window. If current time is past
    // the availability window, then eos is returned higher up.
    // If not, then we are waiting for mpd update.
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
      "Waiting for mpd update");
    status = video::HTTPCommon::HTTPDL_WAITING;
  }

  return status;
}

HTTPDownloadStatus RepresentationInfo::SegmentFuncTemplate::GetLastSegmentKeyForRepresentation(MPDParser* pMPDParser, PeriodInfo* pPeriodInfo, uint64* pSegmentInfo, RepresentationInfo* pRepInfo )
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pMPDParser->mpd, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  if(NULL == pRepInfo->GetSegmentTemplate())
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetLastSegmentKeyForRepresentation: Unexpected error. Null template");
  }
  else
  {
    uint32 startNumber = pRepInfo->GetSegmentTemplate()->GetStartNumber();
    double segDuration = CalculateSegmentDurationFromStoredTemplate(pRepInfo);

    if(segDuration > 0)
    {
      double latestSegOffset = 0.0;
      int64 nLatestOffsetStartNumber = -1;

      double absPeriodAvailEndTime = 1000.0 * pPeriodInfo->getDuration();

      if(absPeriodAvailEndTime == 0)
      {
        MM_Time_DateTime fetchTime = pMPDParser->GetFetchTime();
        absPeriodAvailEndTime = (StreamSourceTimeUtils::ConvertSysTimeToMSec(fetchTime) +
          1000.0 * (pMPDParser->mpd)->getMinimumUpdatePeriod());
      }

      nLatestOffsetStartNumber = (int64)((absPeriodAvailEndTime) / segDuration);

      int64 returnLastSegIdx = nLatestOffsetStartNumber - 1;

      if ((int)(absPeriodAvailEndTime) % (int)segDuration != 0)
      {
        // segEndTime does not line up with start of a new segment in template.
        returnLastSegIdx += 1;
      }

      if(pSegmentInfo && absPeriodAvailEndTime > 0)
      {
        uint64 mpdKey = 0;
        uint64 repKey = pRepInfo->getKey();
        mpdKey = repKey | returnLastSegIdx;
        *pSegmentInfo = mpdKey;
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Invalid segDuration 0");
    }
  }

  return status;
}

/* This function will provide the client with all the segments within a particular range for the
* representation identified by param repKey
* @param[out] - pSegmentInfo - array of segment keys for segments within that range
* @param[in/out] - numSegments - numSegments is populated and sent to client.
* @param[in] - repKey - Rep key for which segment information is queried
* @param[in] - segStartTime - Start of the range of queried segments
* @param[in] - segEndTime - End of the range for which segment information is requested
* @param[out] - firstAvailableSegmentStartTime - start time of the first available segment
* @return - HTTPDownloadStatus
* HTTPDL_SUCCESS - success
* HTTPDL_WAITING - if mpd is not available.
* HTTPDL_INSUFFICIENT_BUFFER - if space is not sufficient
* HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetAllSegmentsForRepresentationRange(uint64* pSegmentInfo,
    uint32& numSegments,uint64 repKey,uint64 segStartTime,uint64 segEndTime,double &firstAvailableSegmentStartTime)
{
  firstAvailableSegmentStartTime = -1;
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(mpd,status);

  MM_Time_DateTime currentTime;
  MM_Time_GetUTCTime(&currentTime);

  double currMSeconds =
    StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);

  uint32 period_index = (uint32)((repKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  uint32 grp_index  = (uint32)((repKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
  uint32 rep_index  = (uint32)((repKey & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
  int numPeriods = 0;
  PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);

  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "GetAllSegmentsForRepresentationRange period_index %u, segStartTime %u, segEndTime %u",
    period_index, (uint32)segStartTime, (uint32)segEndTime);

  // 1. Check if the segStartTime and end fall within valid boundaries of the periodStart and
  // end time. In MPD refresh case, the period end time is unknown if it falls in last period.
  // 2a. For VOD from this point should return success
  // 2b. For live, check to see if segStartTime and SegEndTimes fall within availability.

  if(period_info && numPeriods > 0)
  {
    int periodArrayIdx =
      (int)(period_index - (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

    if (periodArrayIdx >= 0 && periodArrayIdx < numPeriods)
    {
      int numGrps = 0;
      PeriodInfo& rPeriodInfo = period_info[periodArrayIdx];
      uint64 periodStartTime = rPeriodInfo.getStartTime();

      RepresentationGroup* pRepGrp = rPeriodInfo.getRepGrpInfo(numGrps);

      if(pRepGrp && grp_index < (uint32)numGrps)
      {
        int numRepresentations = 0;
        RepresentationInfo *pReps = pRepGrp[grp_index].getRepInfo(numRepresentations);
        if(pReps && rep_index < (uint32)numRepresentations)
        {

          RepresentationInfo& rRepInfo = pReps[rep_index];

          status = GetAllSegmentsForRepresentationRangePreProcess(&rPeriodInfo, &rRepInfo, segStartTime, currMSeconds);
          if(status == HTTPCommon::HTTPDL_SUCCESS)
          {

            status = rRepInfo.GetSegmentFunc()->GetAllSegmentsForRepresentationRange(this,
                                                                                     currMSeconds,
                                                                                     &rPeriodInfo,
                                                                                     &rRepInfo,
                                                                                     pSegmentInfo,
                                                                                     numSegments,
                                                                                     segStartTime,
                                                                                     segEndTime,
                                                                                     firstAvailableSegmentStartTime);
          }
        }//if(pReps && rep_index < numRepresentations)
      }// if(pRepGrp && grp_index < numGrps)
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Invalid periodArrayIdx %d", periodArrayIdx);
      status = HTTPCommon::HTTPDL_DATA_END;
    }
  }

  return status;

}

/* This function will provide the client with the last segment key for the
* representation identified by param repKey
* @param[out] - pSegmentInfo - array of segment keys for segments within that range
* @param[in] - repKey - Rep key for which segment information is queried
* @return - HTTPDownloadStatus
* HTTPDL_SUCCESS - success
* HTTPDL_WAITING - if mpd is not available.
* HTTPDL_INSUFFICIENT_BUFFER - if space is not sufficient
* HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetLastSegmentKeyForRepresentation(uint64* pSegmentInfo, uint64 repKey)
{
  MPD* mpd = this->mpd;
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(mpd,status);

  uint32 period_index = (uint32)((repKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  uint32 grp_index  = (uint32)((repKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
  uint32 rep_index  = (uint32)((repKey & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
  int numPeriods = 0;
  PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);

  if(period_info && numPeriods > 0)
  {
    int periodArrayIdx =
      (int)(period_index - (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

    if (periodArrayIdx >= 0 && periodArrayIdx < numPeriods)
    {
      int numGrps = 0;
      PeriodInfo& rPeriodInfo = period_info[periodArrayIdx];
      uint64 periodStartTime = rPeriodInfo.getStartTime();

      RepresentationGroup* pRepGrp = rPeriodInfo.getRepGrpInfo(numGrps);

      if(pRepGrp && grp_index < (uint32)numGrps)
      {
          int numRepresentations = 0;
          RepresentationInfo *pReps = pRepGrp[grp_index].getRepInfo(numRepresentations);
          if(pReps && rep_index < (uint32)numRepresentations)
          {
            uint32 nNumSegments = 0,size=0;
            RepresentationInfo& rRepInfo = pReps[rep_index];

            status = rRepInfo.GetSegmentFunc()->GetLastSegmentKeyForRepresentation(this, &rPeriodInfo, pSegmentInfo, &rRepInfo);

            if(status == HTTPCommon::HTTPDL_ERROR_ABORT)
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                "GetLastSegmentKeyForRepresentation: Unexpected error for repkey %llu", repKey);
            }

          }//if(pReps && rep_index < numRepresentations)
      }// if(pRepGrp && grp_index < numGrps)
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Invalid periodArrayIdx %d", periodArrayIdx);
    }
  }

  return status;
}


/* This function provides last available segment start time and end time across
* all periods
* @param[out] - nLastAvailableSegmentStartTime - Start time of the last available segment
* @param[out] - nLastAvailableSegmentEndTime - End time of the last available segment
* @return - HTTPDownloadStatus
* HTTPDL_SUCCESS - success
* HTTPDL_WAITING - if none of the segments are available
* HTTPDL_DATA_END - if all segments past their availability times
* HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetLastAvailableSegmentTimeForPlayback
(
  uint64 &nLastAvailableSegmentStartTime,
  uint64 &nLastAvailableSegmentEndTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  MM_CriticalSection_Enter(m_pParserDataLock);

  if(NULL != mpd)
  {
    int numPeriods = 0;
    PeriodInfo *pPeriodInfo = mpd->getPeriodInfo(numPeriods);

    if(pPeriodInfo && numPeriods > 0)
    {
      int i = numPeriods-1;

      for(i = numPeriods-1; i >= 0 ; --i)
      {
        status = GetLastAvailableSegmentTimeForPeriod(pPeriodInfo[i].getPeriodKey(),
                                                       nLastAvailableSegmentStartTime,
                                                       nLastAvailableSegmentEndTime);

        if(HTTPCommon::HTTPDL_WAITING == status)
        {
          continue;
        }
        else
        {
          break;
        }
      }

      if(HTTPCommon::HTTPDL_WAITING == status)
      {
        i = 0;
      }

      if(HTTPCommon::HTTPDL_SUCCESS == status || HTTPCommon::HTTPDL_WAITING == status)
      {
        nLastAvailableSegmentStartTime = pPeriodInfo[i].getStartTime() + nLastAvailableSegmentStartTime;
        nLastAvailableSegmentEndTime = pPeriodInfo[i].getStartTime() + nLastAvailableSegmentEndTime;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "MPDParser::GetLastAvailableSegmentTimeForPlayback mpd is NULL");
  }

  MM_CriticalSection_Leave(m_pParserDataLock);

  return status;
}


/* This function provides last available segment start time and end time for the
* period identified by period key from the current system time
* @param[in] - nPeriodKey - period key for which segment information is queried
* @param[out] - nLastAvailableSegmentStartTime - Start time of the last available segment
* @param[out] - nLastAvailableSegmentEndTime - End time of the last available segment
* @return - HTTPDownloadStatus
* HTTPDL_SUCCESS - success
* HTTPDL_WAITING - if none of the segments are available
* HTTPDL_DATA_END - if all segments past their availability times
* HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetLastAvailableSegmentTimeForPeriod
(
  uint64 nPeriodKey,
  uint64 &nLastAvailableSegmentStartTime,
  uint64 &nLastAvailableSegmentEndTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  uint32 nPeriodIdx = (uint32)((nPeriodKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
  int nNumPeriods = 0;

  PeriodInfo* pPeriodInfo = mpd->getPeriodInfo(nNumPeriods);

  if (pPeriodInfo)
  {
    int nPeriodArrayIdx =
      (int)(nPeriodIdx - (uint32)((uint64)(pPeriodInfo[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

    if (nPeriodArrayIdx >=0 && nPeriodArrayIdx < nNumPeriods)
    {
      nLastAvailableSegmentStartTime = MAX_UINT64;
      nLastAvailableSegmentEndTime = MAX_UINT64;

      int numRepGrps = 0;
      RepresentationGroup *repGrpArray = pPeriodInfo->getRepGrpInfo(numRepGrps);

      if (repGrpArray && numRepGrps > 0)
      {
        MM_Time_DateTime sCurrTime;
        MM_Time_GetUTCTime(&sCurrTime);
        double nCurrMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(sCurrTime);

        bool bIsCurrentTimeTooEarlyOnAllReps = IsLive() ? true : false;
        bool bIsCurrentTimeTooLateOnAllReps = IsLive() ? true : false;

        for (int j = 0; j < numRepGrps; ++j)
        {
          int numReps = 0;
          RepresentationInfo *repArray = repGrpArray[j].getRepInfo(numReps);

          if (repArray && numReps > 0)
          {
            for (int k = 0; k < numReps; ++k)
            {
              uint64 nTmpStartTime = MAX_UINT64;
              uint64 nTmpEndTime = MAX_UINT64;

              HTTPDownloadStatus ret = repArray[k].GetSegmentFunc()->GetLastAvailableSegmentTimeForRepresentation(this,
                &pPeriodInfo[nPeriodArrayIdx],
                &repArray[k],
                nCurrMSeconds,
                nTmpStartTime,
                nTmpEndTime);

              if(HTTPCommon::HTTPDL_ERROR_ABORT == ret)
              {
                continue; //Continue to other representations
              }

              if(HTTPCommon::HTTPDL_WAITING != ret)
              {
                bIsCurrentTimeTooEarlyOnAllReps = false;

                if(HTTPCommon::HTTPDL_DATA_END != ret)
                {
                  bIsCurrentTimeTooLateOnAllReps = false;
                  if (HTTPCommon::HTTPDL_SUCCESS == ret && (MAX_UINT64 == nLastAvailableSegmentEndTime || (nTmpEndTime > nLastAvailableSegmentEndTime)))
                  {
                    nLastAvailableSegmentStartTime = nTmpStartTime;
                    nLastAvailableSegmentEndTime = nTmpEndTime;
                    status = HTTPCommon::HTTPDL_SUCCESS;
                  }
                }
              }
            }

            if(true == bIsCurrentTimeTooEarlyOnAllReps)
            {
              nLastAvailableSegmentStartTime = 0;
              nLastAvailableSegmentEndTime = 0;
              status = HTTPCommon::HTTPDL_WAITING;
            }
            else if(true == bIsCurrentTimeTooLateOnAllReps)
            {
              status = HTTPCommon::HTTPDL_DATA_END;
            }
          }
        }
      }

      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "MPDParser::GetLastAvailableSegmentTimeForPeriod period key %lu, periodStartTime %llu, segmentEndTime %llu",
                 (uint32)((nPeriodKey & (uint64)MPD_PERIOD_MASK) >> 56),  pPeriodInfo[nPeriodArrayIdx].getStartTime(), nLastAvailableSegmentEndTime);
    }
  }


  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "MPDParser::GetLastAvailableSegmentTimeForPeriod period key %lu, status %d",
                 (uint32)((nPeriodKey & (uint64)MPD_PERIOD_MASK) >> 56), status);

  return status;
}


/* This function provides last available segment start time and end time for the
* representation identified by param repKey from the current system time
* @param[in] - repKey - Rep key for which segment information is queried
* @param[out] - nLastAvailableSegmentStartTime - Start time of the last available segment
* @param[out] - nLastAvailableSegmentEndTime - End time of the last available segment
* @return - HTTPDownloadStatus
* HTTPDL_SUCCESS - success
* HTTPDL_WAITING - if none of the segments are available
* HTTPDL_DATA_END - if all segments past their availability times
* HTTPDL_ERROR_ABORT - otherwise
*/
HTTPDownloadStatus MPDParser::GetLastAvailableSegmentTimeForRepresentation
(
  uint64  repKey,
  uint64  &nLastAvailableSegmentStartTime,
  uint64  &nLastAvailableSegmentEndTime
)
{
  int lastAvailableSegmentIndex = -1;
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(mpd, HTTPCommon::HTTPDL_ERROR_ABORT);

  if (false == m_bIsMpdValid)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Data end as mpd is no longer valid");
    status = video::HTTPCommon::HTTPDL_DATA_END;
  }
  else if(m_bAbortSet)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "MPD parser task aborted");
    status = video::HTTPCommon::HTTPDL_DATA_END;
  }
  else if(!mpdAvailable)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "Waiting to get the mpd information from server");
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
    uint32 period_index = (uint32)((repKey & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
    uint32 grp_index  = (uint32)((repKey & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
    uint32 rep_index  = (uint32)((repKey & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
    int numPeriods = 0;

    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
      "GetLastAvailableSegmentTimeForRepresentation period_index %u rep_index %u", period_index, rep_index);

    MM_Time_DateTime currentTime;
    MM_Time_GetUTCTime(&currentTime);

    MM_CriticalSection_Enter(m_pParserDataLock);

    double availabilityMSeconds = (mpd ? mpd->GetAvailabilityStartTime() : 0.0);
    double currMSeconds =
      StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);

    double mpdDuration = (mpd ? (mpd->getDuration() * ((double)1000)) : 0.0);

    //If the MPD is not yet available then it is a complete waiting.
    if(currMSeconds < availabilityMSeconds)
    {
      int tmpDiff = (int)((availabilityMSeconds - currMSeconds)/1000.0);
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Mpd is not available yet. currMSeconds %llu < availabilityMSeconds %llu. Ahead by %d seconds",
                     (uint64)currMSeconds, (uint64)availabilityMSeconds, tmpDiff);
      status = HTTPCommon::HTTPDL_WAITING;
    }
    //ToDo : If availabilityEndTime is present only the segments till availabilityEndTime
    // will be available irrespective of whether itis Live or OnDemand.
    else
    {
      PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);

      if(period_info && numPeriods > 0)
      {
        uint32 nMinPeriodKey = (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
        uint32 nRequestedPeriodKey = (uint32)((uint64)(repKey & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);

        if (nMinPeriodKey > nRequestedPeriodKey)
        {
          // this should be when the mpd refresh occured so long after the
          // previous refresh that the periodKey assosicated with request
          // is no longer relatable in the latest mpd refresh.
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "HTTPDL_DATA_END as nMinPeriodKey %u > nRequestedPeriodKey %u",
                        nMinPeriodKey, nRequestedPeriodKey);
          status = video::HTTPCommon::HTTPDL_DATA_END;
        }
        else
        {
          int periodArrayIdx =
            (int)(period_index - (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT));

          if (periodArrayIdx >= 0 && periodArrayIdx < numPeriods)
          {
            int numGrps = 0;
            uint64 periodStartTime = period_info[periodArrayIdx].getStartTime();

            RepresentationGroup* pRepGrp = period_info[periodArrayIdx].getRepGrpInfo(numGrps);

            if(pRepGrp && grp_index < (uint32)numGrps)
            {
              int numRepresentations = 0;
              RepresentationInfo *pReps = pRepGrp[grp_index].getRepInfo(numRepresentations);
              if(pReps && rep_index < (uint32)numRepresentations)
              {

                bool bCheckEos = false;

                if (period_info[periodArrayIdx].getDuration() > 0.0 || (IsLive() && (mpd) && mpd->getMinimumUpdatePeriod()))
                {
                  if (availabilityMSeconds > 0.0)
                  {
                    uint64 nLastSegKey = MAX_UINT64_VAL;
                    uint64 segDuration = 0;

                    //Will need to use last segment key duration also in calculation to check if past period end

                    if(pReps[rep_index].GetSegmentFunc())
                    {
                      status = pReps[rep_index].GetSegmentFunc()->GetLastSegmentKeyForRepresentation(this, &period_info[periodArrayIdx], &nLastSegKey, &pReps[rep_index]);

                      if(status == HTTPCommon::HTTPDL_SUCCESS)
                      {
                        status = pReps[rep_index].GetSegmentFunc()->GetSegDurationForRepresentation(this, &pReps[rep_index], nLastSegKey, segDuration);

                        if(status == HTTPCommon::HTTPDL_SUCCESS)
                        {
                    // If the current time has gone past the end of the period-end,
                    // then set status as end of stream. The segmentinfo array will
                    // not be created in this case.
                          double absPeriodAvailEndTime = 0.0;
                          if (period_info[periodArrayIdx].getDuration() > 0.0)
                          {
                            absPeriodAvailEndTime = availabilityMSeconds + (double)segDuration +
                                      (double)periodStartTime +  1000 * period_info[periodArrayIdx].getDuration();
                          }
                          else if (IsLive() && mpd->getMinimumUpdatePeriod())
                          {
                            MM_Time_DateTime fetchTime = GetFetchTime();
                            absPeriodAvailEndTime = (StreamSourceTimeUtils::ConvertSysTimeToMSec(fetchTime) +
                              1000.0 * mpd->getMinimumUpdatePeriod() + (double)segDuration);
                          }

                           double offsetOfPeriodEndFromCurrentTime =
                              ((currMSeconds -  pReps[rep_index].GetTimeShiftBufferDepth() * 1000) >= absPeriodAvailEndTime
                               ? (currMSeconds - pReps[rep_index].GetTimeShiftBufferDepth() * 1000) - absPeriodAvailEndTime : 0.0);

                           if (offsetOfPeriodEndFromCurrentTime > 0.0 && IsLive() )
                           {
                             QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "eos reached on period %u, peiodDuration %f, past period end by %f msecs",
                             period_index, period_info[periodArrayIdx].getDuration(), offsetOfPeriodEndFromCurrentTime);

                             status = video::HTTPCommon::HTTPDL_DATA_END;
                             bCheckEos = true;
                           }
                         }
                       }
                    }
                  }

                if(!bCheckEos)
                {
                  status =
                    pReps[rep_index].GetSegmentFunc()->GetLastAvailableSegmentTimeForRepresentation(
                    this, &period_info[periodArrayIdx], &pReps[rep_index],
                    currMSeconds, nLastAvailableSegmentStartTime, nLastAvailableSegmentEndTime);
                }
                }//period_info[periodArrayIdx].getDuration() > 0.0
              }//if(pReps && rep_index < numRepresentations)
            }// if(pRepGrp && grp_index < numGrps)
          }
          else
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                          "Invalid periodArrayIdx %d", periodArrayIdx);
          }
        }
      }//if(period_info && period_index < numPeriods)
    }

    MM_CriticalSection_Leave(m_pParserDataLock);
  }

  return status;
}


/* This function computes last available segment start time and end time for the
* representation.
* When segment information for representation is either of type segment base,
* segment list with/without timeline or segment template with timeline SegmentFuncDefault
* path API's are used for available segment times calculations where segment info
* structures are already available
*/
HTTPDownloadStatus RepresentationInfo::SegmentFuncDefault::GetLastAvailableSegmentTimeForRepresentation
(
  MPDParser *pMPDParser,
  PeriodInfo* pPeriodInfo,
  RepresentationInfo* pRepInfo,
  double nCurrMSeconds,
  uint64& nLatestSegmentStartTime,
  uint64& nLatestSegmentEndTime
 )
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  int64 lastAvailableSegmentIndex = -1;
  MPD *mpd = pMPDParser->mpd;
  QTV_NULL_PTR_CHECK(mpd, HTTPCommon::HTTPDL_ERROR_ABORT);

  double availabilityMSeconds = (mpd ? mpd->GetAvailabilityStartTime() : 0.0);
  uint64 periodStartTime = pPeriodInfo->getStartTime();

  double lowerBoundTime = nCurrMSeconds - pMPDParser->GetTimeShiftBufferDepth()*1000;
  double upperBoundTime = nCurrMSeconds;

  uint32 nNumSegments = 0,size=0;
  SegmentInfo* pSegment = pRepInfo->getSegmentInfo(nNumSegments,size);
  if(pSegment)
  {
    uint64 nSegStartTime = 0;
    uint64 nSegDuration = 0;
    int segmentCounter = 0, copyCounter = 0;

    //find out the first available segment
    for(uint32 i = 0;i < nNumSegments; i++)
    {
      //The number of available segments will actually be based on timeShiftBufferDepth
      //For the live case depending on this parameter those many number of segments will be available
      //and for the onDemand case numberOfAvailable segments is nNumSegments if current time
      //is past the availabilityStartTime
      if(!pMPDParser->IsLive())
      {
        lastAvailableSegmentIndex = nNumSegments - 1;
        break;
      }
      //find the available segments window for the live case.
      //The calculation takes into account timeShiftBuffer.

      double curSegmentAvailStartTime =
          availabilityMSeconds + (double)periodStartTime + pSegment[i].getDuration() + pSegment[i].getStartTime();


      if((curSegmentAvailStartTime <= upperBoundTime) &&
         (lowerBoundTime < (curSegmentAvailStartTime + pSegment[i].getDuration())))
      {
        lastAvailableSegmentIndex = i;
      }
    }//for(int i = 0;i < nNumSegments; i++)

    if (lastAvailableSegmentIndex >= 0)
    {
      status = HTTPCommon::HTTPDL_SUCCESS;
      nLatestSegmentStartTime = (uint64)pSegment[lastAvailableSegmentIndex].getStartTime();
      nLatestSegmentEndTime = (uint64)(pSegment[lastAvailableSegmentIndex].getStartTime() +
        pSegment[lastAvailableSegmentIndex].getDuration());
    }
    else
    {
      //No segment available currently. Check for DATA_END already occured before entering this function.
      status = HTTPCommon::HTTPDL_WAITING;
    }
  }//if(pSegment)
  else
  {
    // if segment array was not created, then non of the segments
    // fall in the availability window. If current time is past
    // the availability window, then eos is returned higher up.
    // If not, then we are waiting for mpd update.
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                 "Waiting for mpd update");
    status = video::HTTPCommon::HTTPDL_WAITING;
  }

  return status;
}


HTTPDownloadStatus RepresentationInfo::SegmentFuncDefault::GetSegDurationForRepresentation(MPDParser *pMPDParser, RepresentationInfo* pRepInfo, uint64 nSegKey, uint64& segDuration)
{
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  SegmentInfo segInfo;
  bool bOk = pMPDParser->GetSegmentInfoByKey(nSegKey, segInfo);

  if(bOk)
  {
    status = HTTPCommon::HTTPDL_SUCCESS;
    segDuration = (uint64)segInfo.getDuration();
  }

  return status;
}


HTTPDownloadStatus RepresentationInfo::SegmentFuncTemplate::GetSegDurationForRepresentation(MPDParser * /*pMPDParser*/, RepresentationInfo* pRepInfo, uint64 /*nSegKey*/, uint64& segDuration)
{
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  //All segments with the representation have same duration for template-without-timeline scenario
  segDuration = (uint64)CalculateSegmentDurationFromStoredTemplate(pRepInfo);

  return HTTPCommon::HTTPDL_SUCCESS;;
}




/* This function will provide the client with the segment’s information.
 * identified by the param key
 * @param[in] - key - segment's key
 * @param[out] - pSegmentInfo - Segment's information
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::GetSegmentInfoByKey(uint64 key,SegmentInfo& pSegmentInfo)
{
  MPDParser* pMPDParser = this;
  MPD *mpd = pMPDParser->mpd;
  QTV_NULL_PTR_CHECK(mpd, HTTPCommon::HTTPDL_ERROR_ABORT);
  bool rslt = false;

  if(pMPDParser->m_bAbortSet)
  {
     QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "MPD parser task aborted");
  }
  else if(!mpd)
  {
     QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "mpd is NULL");
  }
  else
  {
    uint32 periodKey = (uint32)((key & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
    uint32 grp_index  = (uint32)((key & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
    uint32 rep_index = (uint32)((key & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
    uint32 segmentKey = (uint32)((key & MPD_SEGMENT_MASK));

    int numPeriods = 0;
    PeriodInfo* pPeriodInfo=mpd->getPeriodInfo(numPeriods);

    if (pPeriodInfo)
    {
      int periodIndex = (int)(periodKey - (pPeriodInfo[0].getPeriodKey() >> MPD_PERIOD_SHIFT_COUNT));

      if((periodIndex >= 0) && (periodIndex < numPeriods))
      {
        int numRepGrps = 0;
        RepresentationGroup* repgrpInfo=pPeriodInfo[periodIndex].getRepGrpInfo(numRepGrps);
        if(repgrpInfo && grp_index < (uint32)numRepGrps)
        {
          int numReps=0;
          RepresentationInfo *repInfo = repgrpInfo[grp_index].getRepInfo(numReps);
          if(repInfo && rep_index < (uint32)numReps)
          {
            if(repInfo[rep_index].GetSegmentFunc()->GetType() == RepresentationInfo::SegmentFuncBase::SEGMENT_TYPE_DEFAULT)
            {
              uint32 numSegments =0,size =0;
              SegmentInfo* pSegment=repInfo[rep_index].getSegmentInfo(numSegments,size);
              if(pSegment)
              {
                int64 segmentIndex = segmentKey - (pSegment[0].getKey() & MPD_SEGMENT_MASK);
                if ((segmentIndex >= 0) && ((uint32)segmentIndex < numSegments))
                {
                  pSegmentInfo.Copy(pSegment[segmentIndex]);
                  rslt = true;
                 }
                 else
                 {
                   QTV_MSG_PRIO6(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Segment with key %u %u %u %u not found in seg array - "
                   "segmentIndex %lld, numSegs %u", periodKey, grp_index, rep_index,
                   segmentKey, segmentIndex, numSegments);
                 }
               }
            }
            else
            {
              rslt = ((RepresentationInfo::SegmentFuncTemplate*)(repInfo[rep_index].GetSegmentFunc()))
                          ->GenerateSegmentInfoFromTemplate(pMPDParser, &pSegmentInfo, &repInfo[rep_index], &pPeriodInfo[periodIndex], key);
            }
          }

        }
      }
    }
  }

  return rslt;
}


bool MPDParser::GetSegmentTimeInfo(uint64 /* segmentKey */,double& nStartTime,double& nDuration)
{
  bool bOk = false;
  QTV_NULL_PTR_CHECK(mpd,bOk);
  /* TODO: Parse the duration from segment info at all levels and take value
     from appropriate level. For phase-1 duration will only be present at mpd
     level and there will only be one sement, so safe to set start time to 0
     and duration to mpd duration */
  nStartTime = 0;
  nDuration = mpd->getDuration();
  return true;
}

/* This function will parse the Representation element of the MPD
 * and store in the representationinfo.
 * @param[in] - Representation Element - xml element for the representation
 * @param[in] - baseURLs - base urls to construct the absolute url
 * @param[in] - numUrls - number of base urls
 * @param[in] - repKey - unique key for the representation
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::ParseRepresentation(MPD *pMpd, TiXmlElement* RepElement,
                                    char **baseURLs, char **baseURLByteRangeAttrs, int& numUrls,
                                    uint64 repKey,int& bRepType,
                                    bool &bBWPresent)
{
  QTV_NULL_PTR_CHECK(pMpd,false);
  bool bOk = true;
  int numPeriods=0;
  PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
  RepresentationGroup* repGrps=NULL;
  RepresentationInfo* rep=NULL;
  int PeriodIndex = (int)((repKey & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
  int GroupIndex =(int)((repKey & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
  int RepIndex = (int)((repKey & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT);
  //SegmentTemplate
  char *bitstreamSwitchingTemplate = NULL;
  //MultipleSegmentBaseInformation
  uint32 startNumber = MAX_UINT32_VAL;
  //SegmentBase
  uint32 segBaseTimeScale = 1;
  uint64 segBasePresentationTimeOffset = 0;
  char *indexRangeBase = NULL;
  bool indexRangeExact = false;
  //SegmentList info
  char *mediaUrl = NULL;
  char *mediaRange = NULL;
  char *indexUrl = NULL;
  bool isIndexURLPresent = false;
  bool isInitURLPresent = false;
  char *indexRange = NULL;
  int numSegmentURL = 0;
  SegmentURLStruct *segmentURL = NULL;

  char elementSearchStr[100];
  int64 nStartOffset = 0;
  int64 nEndOffset = -1;

  uint32 timeScale = MAX_UINT32_VAL;
  uint64 presentationTimeOffset = 0;
  double ptsOffset = 0.0;


  URLType *initialisation = NULL;
  URLType *representationIndex = NULL;
  bBWPresent = false;
  if(pPeriodInfo)
  {
    int numRepGrps = 0;
    repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
    if(repGrps)
    {
      int numReps = 0;
      rep=repGrps[GroupIndex].getRepInfo(numReps);
      if(rep)
      {
        bOk = true;
      }
    }
  }

  rep[RepIndex].SetMinUpdatePeriod(pMpd->getMinimumUpdatePeriod());
  rep[RepIndex].SetTimeShiftBufferDepth(pMpd->getTimeShiftBufferDepth());

  bOk = ParseRepAttrs(*pMpd, RepElement,repKey,rep,repGrps[GroupIndex],bRepType, bBWPresent);

  //if segmentAlignment false then inherit
  if (bOk && !rep[RepIndex].IsSegmentAlinged())
  {
     rep[RepIndex].SetSegmentAlinged(repGrps[GroupIndex].IsSegmentAlinged());
  }

  // if SAP is not set then inherit
  if (bOk && rep[RepIndex].GetStartWithSAPVal() == -1)
  {
     rep[RepIndex].SetStartWithSAPVal(repGrps[GroupIndex].GetStartWithSAPVal());
  }
  //if subsegmentSAP is not set then inherit
  if (bOk && rep[RepIndex].GetSubSegStartWithSAPVal()== -1)
  {
     rep[RepIndex].SetSubSegStartWithSAPVal(repGrps[GroupIndex].GetSubSegStartWithSAPVal());
  }

  //Parsing BaseURL
  TiXmlElement* BaseURLElement = NULL;
  elementSearchStr[0] = '\0';
  int index = 0;
  char *repBaseURLs[10] = {NULL};
  char *repBaseURLByteRangeAttrs[10] = {NULL};

  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:BaseURL", m_pNamespaceKey);
    BaseURLElement = RepElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    BaseURLElement = RepElement->FirstChildElement("BaseURL");
  }

  if(bOk)
  {
    if (!BaseURLElement)
    {
      for(int i=0;i<numUrls;i++)
      {
        if(baseURLs[i])
        {
          repBaseURLs[i] = (char *)QTV_Malloc(std_strlen(baseURLs[i])+1);
          if (repBaseURLs[i])
          {
            std_strlcpy(repBaseURLs[i], baseURLs[i], std_strlen(baseURLs[i])+1);
            if(baseURLByteRangeAttrs[i])
            {
              repBaseURLByteRangeAttrs[i] = (char *)QTV_Malloc(std_strlen(baseURLByteRangeAttrs[i])+1);
              if (repBaseURLByteRangeAttrs[i])
              {
                std_strlcpy(repBaseURLByteRangeAttrs[i], baseURLByteRangeAttrs[i], std_strlen(baseURLByteRangeAttrs[i])+1);
              }
            }
          }
          index++;
        }
      }
    }
    while(bOk && BaseURLElement)
    {
      if ((char*)BaseURLElement->GetText())
      {
        if(repBaseURLs[index])
        {
          QTV_Free(repBaseURLs[index]);
          repBaseURLs[index] = NULL;
        }
        if(repBaseURLByteRangeAttrs[index])
        {
          QTV_Free(repBaseURLByteRangeAttrs[index]);
          repBaseURLByteRangeAttrs[index] = NULL;
        }
        int periodBaseURLLen = 0;
        char *url=(char*)BaseURLElement->GetText();
        char *tmpURL = NULL;
        if (baseURLs)
        {
          tmpURL = baseURLs[index];
        }
        bOk = GetResolvedURL(tmpURL,url,repBaseURLs[index],periodBaseURLLen);
        if (bOk)
        {
          repBaseURLs[index] = (char *)QTV_Malloc(periodBaseURLLen);
          if (repBaseURLs[index])
          {
            bOk = GetResolvedURL(tmpURL,url,repBaseURLs[index],periodBaseURLLen);
            if(bOk)
            {
              TiXmlAttribute* attrib=BaseURLElement->FirstAttribute();
              while(attrib)
              {
                char *attrib_name=(char*)skip_whitespace(attrib->Name());

                if(!std_strnicmp(attrib_name,"byteRange",9))
                {
                  repBaseURLByteRangeAttrs[index] = (char *)QTV_Malloc(std_strlen(attrib->Value())+1);
                  if (repBaseURLByteRangeAttrs[index])
                  {
                    std_strlcpy(repBaseURLByteRangeAttrs[index], attrib->Value(), std_strlen(attrib->Value())+1);
                  }
                  break;
                }
                attrib=attrib->Next();
              }

              if(repBaseURLByteRangeAttrs[index] == NULL && baseURLByteRangeAttrs[index])
              {
                repBaseURLByteRangeAttrs[index] = (char *)QTV_Malloc(std_strlen(baseURLByteRangeAttrs[index])+1);
                if (repBaseURLByteRangeAttrs[index])
                {
                  std_strlcpy(repBaseURLByteRangeAttrs[index], baseURLByteRangeAttrs[index], std_strlen(baseURLByteRangeAttrs[index])+1);
                }
              }
            }
          }
          else
          {
            bOk = false;
          }
        }
        if (bOk)
        {
          index++;
          if(!m_bBaseURLElementPresent)
          {
            m_bBaseURLElementPresent = true;
          }
        }
        if(!bOk || index >= 10)
        {
          //More than 10 alternates are not supported
          break;
        }
      }
      elementSearchStr[0] = '\0';
      if (m_pNamespaceKey)
      {
        std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:BaseURL", m_pNamespaceKey);
        BaseURLElement = BaseURLElement->NextSiblingElement(elementSearchStr);
      }
      else
      {
        BaseURLElement = BaseURLElement->NextSiblingElement("BaseURL");
      }
    }
    numUrls = index;
    rep[RepIndex].SetByteRangeURLTemplate(repBaseURLByteRangeAttrs[0]);
  }

  //Parsing SegmentBase
  TiXmlElement* SegmentBaseElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentBase", m_pNamespaceKey);
    SegmentBaseElement = RepElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentBaseElement =  RepElement->FirstChildElement("SegmentBase");
  }
  if (bOk && SegmentBaseElement)
  {
    bOk = ParseSegmentBase(pMpd, SegmentBaseElement, repKey, MPD_REPRESENTATION);
  }
  // inherit SegmentBase from  Group, if present
  if (bOk && repGrps[GroupIndex].IsSegmentBaseFound())
  {
    (void)rep[RepIndex].InheritSegmentBaseInfo(repGrps[GroupIndex].GetSegmentBase());
  }

  //Parsing SegmentList
  elementSearchStr[0] = '\0';
  TiXmlElement* SegmentListElement = NULL;
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentList", m_pNamespaceKey);
    SegmentListElement = RepElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentListElement = RepElement->FirstChildElement("SegmentList");
  }
  if (bOk && SegmentListElement)
  {
    bOk = ParseSegmentList(pMpd, SegmentListElement, baseURLs, repKey, MPD_REPRESENTATION);
  }
  // inherit SegmentList from  Group, if present
  if (bOk && repGrps[GroupIndex].IsSegmentListFound())
  {
    (void)rep[RepIndex].InheritSegmentList(repGrps[GroupIndex].GetSegmentList());
  }

  //Parsing SegmentTemplate
  TiXmlElement* SegmentTemplateElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentTemplate", m_pNamespaceKey);
    SegmentTemplateElement = RepElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentTemplateElement =  RepElement->FirstChildElement("SegmentTemplate");
  }

  if (bOk && SegmentTemplateElement)
  {
    bOk = ParseSegmentTemplate(pMpd, SegmentTemplateElement, repKey, MPD_REPRESENTATION);
  }

  // inherit SegmentTemplate from  Group, if present
  if (bOk && repGrps[GroupIndex].IsSegmentTemplateFound())
  {
    (void)rep[RepIndex].InheritSegmentTemplate(repGrps[GroupIndex].GetSegmentTemplate());
  }
  if (bOk && m_MpdProfile == DASH_PROFILE_ISO_LIVE  &&
      !rep[RepIndex].IsSegmentTemplateFound())
  {
    //for live stream if segmentTemplate is not present at any level,
    //(representation has segmenttemplate inherited for all the above level)
    //ignore this represenation
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Ignoring representation %d as segment template "
                  "is not present at any level",(int)RepIndex);
     bOk = false;
  }

  uint64 segmentKey=0;
  segmentKey = segmentKey|repKey;
  char *initialisationUrl = NULL;
  char *intialisationUrlRange = NULL;
  if (bOk && rep[RepIndex].IsSegmentBaseFound())
  {
    if(repBaseURLs[0])
    {
      (void)rep[RepIndex].SetBaseURL(repBaseURLs[0]);
    }

    if (rep[RepIndex].GetSegmentBase()->GetInitialisation())
    {
      initialisationUrl = rep[RepIndex].GetSegmentBase()->GetInitialisation()->sourceURL;
      intialisationUrlRange = rep[RepIndex].GetSegmentBase()->GetInitialisation()->range;
      //initialisation url
      if(initialisationUrl)
      {
        isInitURLPresent = true;
        if (repBaseURLs[0])
        {
          rep[RepIndex].SetInitialisationSegmentUrl(repBaseURLs[0],initialisationUrl);
        }
      }
      // Base url shall be used to download init segment using init range
      else if((NULL == initialisationUrl) && (intialisationUrlRange) && repBaseURLs[0])
      {
        isInitURLPresent = true;
      }
      else
      {
        // Self Initialized
      }

      //initialisation url range
      if (intialisationUrlRange)
      {
        parseByteRange(intialisationUrlRange, nStartOffset, nEndOffset);
      }
      rep[RepIndex].SetInitialisationSegmentRange(nStartOffset, nEndOffset, intialisationUrlRange);
    }
    segBaseTimeScale = rep[RepIndex].GetSegmentBase()->GetTimeScale();
    if (segBaseTimeScale == MAX_UINT32_VAL)
    {
      segBaseTimeScale = 1;
      rep[RepIndex].GetSegmentBase()->SetTimeScale(segBaseTimeScale);
    }
    //index range
    indexRangeBase = rep[RepIndex].GetSegmentBase()->GetIndexRange();
    if (indexRangeBase)
    {
     rep[RepIndex].SetIndexSegmentRange(indexRangeBase);
    }
    //retrieve presentationTimeOffset
    segBasePresentationTimeOffset = rep[RepIndex].GetSegmentBase()->GetPresentationOffset();
    ptsOffset = (segBaseTimeScale > 0 ? ((double)segBasePresentationTimeOffset/(double)segBaseTimeScale)
                 : (double)segBasePresentationTimeOffset);

    rep[RepIndex].SetPTSOffset(ptsOffset);
    indexRangeExact = rep[RepIndex].GetSegmentBase()->GetIndexRangeExact();
  }
  if (bOk && rep[RepIndex].IsSegmentListFound()) /*** Segment List ***/
  {
    if(repBaseURLs[0])
    {
      (void)rep[RepIndex].SetBaseURL(repBaseURLs[0]);
    }

    startNumber = rep[RepIndex].GetSegmentList()->GetStartNumber();
    /* start number not present so taking it as '1'*/
    if (startNumber == MAX_UINT32_VAL)
    {
      startNumber = 1;
      rep[RepIndex].GetSegmentList()->SetStartNumber(startNumber);
    }
    if (rep[RepIndex].GetSegmentList()->GetInitialisation())
    {
      initialisationUrl = rep[RepIndex].GetSegmentList()->GetInitialisation()->sourceURL;
      intialisationUrlRange = rep[RepIndex].GetSegmentList()->GetInitialisation()->range;
      //initialisation url
      if(initialisationUrl)
      {
        isInitURLPresent = true;
        if (repBaseURLs[0])
        {
          rep[RepIndex].SetInitialisationSegmentUrl(repBaseURLs[0],initialisationUrl);
        }
      }
      // Base url shall be used to download init segment using init range
      else if((NULL == initialisationUrl) && (intialisationUrlRange) && repBaseURLs[0])
      {
        isInitURLPresent = true;
      }
      else
      {
        // Self Initialized
      }
      //initialisation url range
      if (intialisationUrlRange)
      {
        parseByteRange(intialisationUrlRange, nStartOffset, nEndOffset);
      }
      rep[RepIndex].SetInitialisationSegmentRange(nStartOffset, nEndOffset, intialisationUrlRange);
    }
    timeScale = rep[RepIndex].GetSegmentList()->GetTimeScale();
    if (timeScale == MAX_UINT32_VAL)
    {
      timeScale = segBaseTimeScale;
      rep[RepIndex].GetSegmentList()->SetTimeScale(timeScale);
    }
    // index range
    if (rep[RepIndex].GetSegmentList()->GetIndexRange())
    {
      indexRangeBase = rep[RepIndex].GetSegmentList()->GetIndexRange();
    }

    //retrieve presentationTimeOffset
    presentationTimeOffset = rep[RepIndex].GetSegmentList()->GetPresentationOffset();
    if (presentationTimeOffset > 0)
    {
      ptsOffset = (timeScale > 0 ? ((double)presentationTimeOffset/(double)timeScale)
                 :(double)presentationTimeOffset);
    }
    rep[RepIndex].SetPTSOffset(ptsOffset);

    int numSegment = 0;
    int segmentIndex = (int)startNumber;
    SegmentInfo *pSegment=NULL;
    bool bIsSegmentTimelineProvided = rep[RepIndex].GetSegmentList()->IsSegmentTimelineFound();
    double segDuration = (double)rep[RepIndex].GetSegmentList()->GetDuration();
    segDuration = (timeScale > 0 ? (segDuration/(double)timeScale)*1000:segDuration*1000);
    //SegmentUrl Present
    numSegmentURL = rep[RepIndex].GetSegmentList()->GetNumSegmentUrl();
    if (numSegmentURL > 0)
    {
      numSegment = numSegmentURL;
      if (!bIsSegmentTimelineProvided) /** Segment List without timeline **/
      {
        double periodDuraion = 1000.0 * pPeriodInfo[PeriodIndex].getDuration();
        bool bIsSegDurationZero = false;
        if(0.0 == segDuration)
        {
          bIsSegDurationZero = true;
          if(1 == numSegmentURL && periodDuraion > 0.0)
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "SegDuration is zero. Setting to %d", (int)periodDuraion);
            segDuration = periodDuraion;
          }
          else
          {
            numSegment = 0;
            bOk = false;
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "SegmentDuration zero, but numSegmentsInLIst %d != 1, or "
              "periodDuration %d is unknown", numSegmentURL, (int)periodDuraion);
          }
        }
        else
        {
          UpdateNumSegmentsAndSegmentIndex(
            numSegment, segmentIndex,
            (int)startNumber, *pMpd, pPeriodInfo[PeriodIndex], rep[RepIndex], segDuration,
            numSegmentURL);
        }

        if(bOk)
        {
          if (numSegment > 0)
          {
            pSegment=(SegmentInfo*)QTV_New_Array(SegmentInfo,(numSegment));
            if(pSegment)
            {
              int tempIndex = 0;
              int indexOffsetFromStart = segmentIndex - 1;
              double segmentStartTime = (double)(indexOffsetFromStart*segDuration);
              while((tempIndex < numSegment) && bOk)
              {
                char *tmpUrl = NULL;
                tmpUrl = ((repBaseURLs[0]) ? repBaseURLs[0] : NULL);
                SegmentURLType *pSegmentUrl = rep[RepIndex].GetSegmentList()->GetSegmentUrl(tempIndex + indexOffsetFromStart);
                if (pSegmentUrl)
                {
                  mediaUrl   = pSegmentUrl->GetMediaUrl();
                  mediaRange = pSegmentUrl->GetMediaRange();
                  indexUrl   = pSegmentUrl->GetIndexUrl();
                  indexRange = pSegmentUrl->GetIndexRange();
                }
                if (!mediaUrl)
                {
                  // if media url not present any base url is mapped
                  mediaUrl = tmpUrl;
                }
                if (!indexUrl)
                {
                  indexUrl = mediaUrl;
                }
                else
                {
                  isIndexURLPresent = true;
                }

                if (!indexRange)
                {
                  if (indexRangeBase)
                  {
                    indexRange = indexRangeBase;
                  }
                }
                if (mediaUrl)
                {
                  bOk = pSegment[tempIndex].SetInfoURL(
                    tmpUrl,mediaUrl, mediaRange,
                    indexUrl, isIndexURLPresent, indexRange, indexRangeExact,
                    isInitURLPresent, intialisationUrlRange,
                    segDuration, segmentKey | ((uint64)segmentIndex  - 1),
                    segmentStartTime);
                  if (bOk && pMpd && pMpd->IsLive())
                  {
                    pSegment[tempIndex].SetAvailabilityTime(pMpd->GetAvailabilityStartTime()+
                                                            (double)pPeriodInfo[PeriodIndex].getStartTime()+ (double)segDuration +
                                                            segmentStartTime);
                  }
                }
                tempIndex++;
                segmentIndex++;
                segmentStartTime = ((double)(segmentIndex-1))*segDuration;
              }
              if(rep)
              {
                rep[RepIndex].setSegmentInfo(pSegment, (uint32)tempIndex, (uint32)tempIndex);
              }
              QTV_Delete_Array(pSegment);
              pSegment = NULL;
            }
          }
          else
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Did not create segmentInfoArray");
            bOk = false;
          }
        }
      }
      else /** Segment List with timeline **/
      {
        int numTimelineEntry = rep[RepIndex].GetNumSegmentTimelineEntry();
        int totalNumTimelineEntry = 0;
        int repeatCount = 0;
        double segmentStartTime = 0.0;
        int tempStartNumber = (int)startNumber;
        double offsetFromAvailTime = 0.0;
        if (pMpd && pMpd->GetAvailabilityStartTime() > 0.0 && pMpd->IsLive())
        {
          offsetFromAvailTime = GetOffsetFromAvailabilityTimeForPeriod(*pMpd,
            pPeriodInfo[PeriodIndex], rep[RepIndex].GetTimeShiftBufferDepth());
        }

        //Each segment availability time is now availabilityStartTime + periodStart + segDuration.
        //Hence later below will need last segment duration within the period to determine the segment availability within the period
        double lastSegmentDurationInPeriod = 0;
        if(numTimelineEntry > 0)
        {
          lastSegmentDurationInPeriod = (double)rep[RepIndex].GetSegmentList()->GetSegTimeLineDuration(numTimelineEntry-1);
          lastSegmentDurationInPeriod = (lastSegmentDurationInPeriod/(double)timeScale)*1000;
        }

        double periodDuration = 1000.0 * pPeriodInfo[PeriodIndex].getDuration();
        double relevantPeriodDuration = periodDuration;
        double minUpdatePeriod = rep[RepIndex].GetMinUpdatePeriod();
        if (minUpdatePeriod > 0.0)
        {
          relevantPeriodDuration =
            minUpdatePeriod * 1000 +
          1000 * rep[RepIndex].GetTimeShiftBufferDepth() +
          2000; // adding some delta.

          if (periodDuration > 0.0)
          {
            relevantPeriodDuration = QTV_MIN(relevantPeriodDuration, periodDuration);
          }

          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "periodWindow set as %d", (int)relevantPeriodDuration);
        }
        for (int i = 0; i < numTimelineEntry; i++)
        {
          repeatCount = (int)rep[RepIndex].GetSegmentList()->GetSegTimeLineRepeatCount(i);

          /*  Todo: Repeat count can be negative. Today just ignore the representation

          The value of the @r attribute of the S element may be negative value indicating
          that the duration indicated in @d is promised to repeat until the S@t of the next S element
          or if it is the last S element in the SegmentTimeline element until the end of the Period
          or the next update of the MPD i.e. it is treated in the same way as the @duration attribute for a full period.
          */

          if(repeatCount < 0)
          {
            bOk = false;
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "Ignore the representation since repeat count is negative");
            break;
          }

          totalNumTimelineEntry = totalNumTimelineEntry + repeatCount + 1;
        }

        if(bOk)
        {
        numSegment = QTV_MIN(totalNumTimelineEntry, numSegmentURL);
        GetNumAvailableSegment(numSegment, *pMpd, pPeriodInfo[PeriodIndex],
            rep[RepIndex], timeScale, numSegment, tempStartNumber, false);
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                      "numSegment = %d", numSegment);
        if(numSegment > 0)
        {
        pSegment=(SegmentInfo*)QTV_New_Array(SegmentInfo,(numSegment));
        if (pSegment)
        {
          int segIndex = 0;
          int repeatIndex = 0;
          int tempIndex = 0;
          uint64 replaceSegDuration = 0;
          double replaceSegStartTime = 0.0;
          bool bPeriodDurationSet = false;
          int startIndex = 0;
          int segmentIndex = 0;
          startIndex = (int)startNumber - 1;
              while(bOk && segmentIndex < numSegment)
          {
            char *tmpUrl = NULL;
            bool bSegmentAvail = false;
            if (repeatIndex == 0)
            {
              if (rep[RepIndex].GetSegmentList()->GetSegTimeLineStartTime(tempIndex) > replaceSegStartTime)
              {
                replaceSegStartTime = (double)rep[RepIndex].GetSegmentList()->GetSegTimeLineStartTime(tempIndex);
                segmentStartTime = (replaceSegStartTime/(double)timeScale)*1000;
                segmentStartTime -= (ptsOffset*1000);
              }
              replaceSegDuration = rep[RepIndex].GetSegmentList()->GetSegTimeLineDuration(tempIndex);
              segDuration = ((double)replaceSegDuration/(double)timeScale)*1000;
            }

                if ((segmentStartTime + 2*segDuration) > (offsetFromAvailTime)  &&
                  (segmentStartTime + segDuration) < (relevantPeriodDuration +
                                                     lastSegmentDurationInPeriod))
                  /*This factors in segDuration additionally in segment availability time calculations*/
            {
              bSegmentAvail = true;
              QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "SegmentStartTime %d, segDuration %d, "
                "availability window [%d - %d]",
                  (int)segmentStartTime, (int)segDuration, (int)offsetFromAvailTime,
                      (int)(relevantPeriodDuration + lastSegmentDurationInPeriod));
              if (periodDuration > 0.0 &&
                   periodDuration > segmentStartTime &&
                   !bPeriodDurationSet)
              {
                periodDuration -= segmentStartTime;
                bPeriodDurationSet = true;
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "ReCalculated period duration = %d", (int)periodDuration);
              }
            }
            tmpUrl = ((repBaseURLs[0]) ? repBaseURLs[0] : NULL);
            if (bSegmentAvail)
            {
              SegmentURLType *pSegmentUrl = rep[RepIndex].GetSegmentList()->GetSegmentUrl(segIndex);
              if (pSegmentUrl)
              {
                mediaUrl   = pSegmentUrl->GetMediaUrl();
                mediaRange = pSegmentUrl->GetMediaRange();
                indexUrl   = pSegmentUrl->GetIndexUrl();
                indexRange = pSegmentUrl->GetIndexRange();
              }

              if (!mediaUrl)
              {
                // if media url not present any base url is mapped
                mediaUrl = tmpUrl;
              }
              if (!indexUrl) // if index url is not present
              {
                if (!mediaUrl)
                {
                  // if media url not present any base url is mapped
                  indexUrl = tmpUrl;
                }
                else
                {
                  //media url is mapped
                  indexUrl = mediaUrl;
                }
              }
              else
              {
                isIndexURLPresent = true;
              }

              bOk = pSegment[segmentIndex].SetInfoURL(
                tmpUrl,mediaUrl, mediaRange,
                indexUrl, isIndexURLPresent, indexRange, indexRangeExact,
                isInitURLPresent, intialisationUrlRange,
                segDuration, segmentKey | ((uint64)segmentIndex),
                segmentStartTime);
              if (bOk && pMpd && pMpd->IsLive())
               {
                 pSegment[segmentIndex].SetAvailabilityTime(pMpd->GetAvailabilityStartTime()+
                                                          (double)pPeriodInfo[PeriodIndex].getStartTime()+ (double)segDuration +
                                                          segmentStartTime);
               }
              segmentIndex++;
            }
            segIndex++;
            repeatIndex++;
            if (repeatIndex ==
                 rep[RepIndex].GetSegmentList()->GetSegTimeLineRepeatCount(tempIndex) + 1)
            {
              repeatIndex = 0;
              tempIndex++;
            }
            segmentStartTime = segmentStartTime + segDuration;
            replaceSegStartTime = replaceSegStartTime + (double)replaceSegDuration;
          }
          if(rep)
          {
            rep[RepIndex].setSegmentInfo(pSegment, (uint32)segmentIndex, (uint32)segmentIndex);
          }
          QTV_Delete_Array(pSegment);
          pSegment = NULL;
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "No segments available for segment list with timeline");
      }
      }
    }
    }
    else
    {
      // if no segmetn url provided in segment list
      bOk = false;
    }
  }
  else if (bOk &&
      rep[RepIndex].IsSegmentTemplateFound()) /** Segment Template **/
  {

    if(repBaseURLs[0])
    {
      (void)rep[RepIndex].SetBaseURL(repBaseURLs[0]);
    }

    // Initialisation url from segmenet base
    if (rep[RepIndex].GetSegmentTemplate()->GetInitialisation())
    {
      initialisationUrl = rep[RepIndex].GetSegmentTemplate()->GetInitialisation()->sourceURL;
      intialisationUrlRange = rep[RepIndex].GetSegmentTemplate()->GetInitialisation()->range;
      // initialisation url
      if(initialisationUrl)
      {
        isInitURLPresent = true;
        if (repBaseURLs[0])
        {
          rep[RepIndex].SetInitialisationSegmentUrl(repBaseURLs[0],initialisationUrl);
        }
      }
      // Base url shall be used to download init segment using init range
      else if((NULL == initialisationUrl) && (intialisationUrlRange) && repBaseURLs[0])
      {
        isInitURLPresent = true;
      }
      else
      {
        // Self Initialized
      }
      // initialisation url range
      if (intialisationUrlRange)
      {
        parseByteRange(intialisationUrlRange, nStartOffset, nEndOffset);
      }
      rep[RepIndex].SetInitialisationSegmentRange(nStartOffset, nEndOffset, intialisationUrlRange);
    }

    // Initialisation Template from segment template
    else if (rep[RepIndex].GetSegmentTemplate()->GetInitialisationTemplate())
    {

      char *initUrlTemp = rep[RepIndex].GetSegmentTemplate()->GetInitialisationTemplate();
      if (initUrlTemp)
      {
        int origUrlLen = GetMaxBufferSizeForStringTemplate(rep[RepIndex], initUrlTemp);

        char *initUrlTemplate = (char *)QTV_Malloc(origUrlLen * sizeof(char));
        if (initUrlTemplate)
        {
          SetBufferForStringTemplate(rep[RepIndex], initUrlTemp, initUrlTemplate, origUrlLen);
        isInitURLPresent = true;

        //neither $number$ nor $time$ should be present in the initialisation template
        if (repBaseURLs[0])
        {
            rep[RepIndex].SetInitialisationSegmentUrl(repBaseURLs[0],initUrlTemplate);
        }
        QTV_Free(initUrlTemplate);
        initUrlTemplate = NULL;
        }
      }
    }
    else
    {
      //self initialised
    }

    // Index Range from segmentTemplate
    if (rep[RepIndex].GetSegmentTemplate()->GetIndexRange())
    {
      indexRange = rep[RepIndex].GetSegmentTemplate()->GetIndexRange();
    }
    else // Index Range from Segment Base
    {
      indexRange = indexRangeBase;
    }

    // duration from multiple segment base
    double segDuration = (double)rep[RepIndex].GetSegmentTemplate()->GetDuration();
    segDuration *= 1000;

    // start number from multiple segment base
    startNumber = rep[RepIndex].GetSegmentTemplate()->GetStartNumber();
    if (startNumber == MAX_UINT32_VAL)
    {
      startNumber = 1;
      rep[RepIndex].GetSegmentTemplate()->SetStartNumber(startNumber);
    }
    //time scale from segment template
    timeScale = rep[RepIndex].GetSegmentTemplate()->GetTimeScale();
    if (timeScale == MAX_UINT32_VAL)
    {
      timeScale = segBaseTimeScale;
      rep[RepIndex].GetSegmentTemplate()->SetTimeScale(timeScale);
    }

    //retrieve presentationTimeOffset
    presentationTimeOffset = rep[RepIndex].GetSegmentTemplate()->GetPresentationOffset();
    if (presentationTimeOffset > 0)
    {
      ptsOffset = (timeScale > 0 ? ((double)presentationTimeOffset/(double)timeScale)
                 :(double)presentationTimeOffset);
    }
    rep[RepIndex].SetPTSOffset(ptsOffset);

    int numSegment = 0;
    SegmentInfo *pSegment=NULL;
    bool bIsSegmentTimelineProvided = rep[RepIndex].GetSegmentTemplate()->IsSegmentTimelineFound();
    segDuration = segDuration/(double)timeScale;
    if (!bIsSegmentTimelineProvided) /** Segment Template without timeline **/
    {
      //Segment template present
      if(!rep[RepIndex].IsSegmentFuncInitializedToTemplate())
      {
        rep[RepIndex].SetSegmentFuncToTemplate(true);
      }

      rep[RepIndex].SetMinUpdatePeriod(pMpd->getMininumUpdatePeriodCache());
      rep[RepIndex].SetTimeShiftBufferDepth(pMpd->getTimeShiftBufferDepthCache());

      pMpd->setMinimumUpdatePeriod(pMpd->getMininumUpdatePeriodCache());
      pMpd->setTimeShiftBufferDepth(pMpd->getTimeShiftBufferDepthCache());

    }
    else /** Segment Template with timeline **/
    {
      int numTimelineEntry = rep[RepIndex].GetNumSegmentTimelineEntry();
      int totalNumTimelineEntry = 0;
      int repeatCount = 0;
      double segmentStartTime = 0.0;
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "startNumber = %d", (int)startNumber);
      int tempStartNumber = (int)startNumber;
      double offsetFromAvailTime = 0.0;
      if (pMpd && pMpd->GetAvailabilityStartTime() > 0.0 && pMpd->IsLive())
      {
        offsetFromAvailTime = GetOffsetFromAvailabilityTimeForPeriod(*pMpd,
                                                                 pPeriodInfo[PeriodIndex], rep[RepIndex].GetTimeShiftBufferDepth());
      }

      //Each segment availability time is now availabilityStartTime + periodStart + segDuration.
      //Hence later below will need last segment duration within the period to determine the segment availability within the period
      double lastSegmentDurationInPeriod = 0;
      if(numTimelineEntry > 0)
      {
        lastSegmentDurationInPeriod = (double)rep[RepIndex].GetSegmentTemplate()->GetSegTimeLineDuration(numTimelineEntry-1);
        lastSegmentDurationInPeriod = (lastSegmentDurationInPeriod/(double)timeScale)*1000;
      }

      double periodDuration = 1000.0 * pPeriodInfo[PeriodIndex].getDuration();
      double relevantPeriodDuration = periodDuration;

      double minUpdatePeriod = rep[RepIndex].GetMinUpdatePeriod();
      if (minUpdatePeriod > 0.0)
      {
        relevantPeriodDuration =
          minUpdatePeriod * 1000 +
          1000 * rep[RepIndex].GetTimeShiftBufferDepth() +
          2000; // adding some delta.

        if (periodDuration > 0.0)
        {
          relevantPeriodDuration = QTV_MIN(relevantPeriodDuration, periodDuration);
        }

        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "periodWindow set as %d", (int)relevantPeriodDuration);
      }

      for (int tempIndex = 0; tempIndex < numTimelineEntry; tempIndex++)
      {
        repeatCount = (int)rep[RepIndex].GetSegmentTemplate()->GetSegTimeLineRepeatCount(tempIndex);

        /*  Todo: Repeat count can be negative. Today just ignore the representation

        The value of the @r attribute of the S element may be negative value indicating
        that the duration indicated in @d is promised to repeat until the S@t of the next S element
        or if it is the last S element in the SegmentTimeline element until the end of the Period
        or the next update of the MPD i.e. it is treated in the same way as the @duration attribute for a full period.
        */

        if(repeatCount < 0)
        {
          bOk = false;
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "Ignore the representation since repeat count is negative");
          break;
        }

        totalNumTimelineEntry = totalNumTimelineEntry + repeatCount + 1;
      }

      if(bOk)
      {
      int numSegment = 0;
      GetNumAvailableSegment(numSegment, *pMpd, pPeriodInfo[PeriodIndex],
            rep[RepIndex], timeScale, totalNumTimelineEntry, tempStartNumber);
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "numSegment = %d", numSegment);

      if(numSegment > 0)
      {
      pSegment=(SegmentInfo*)QTV_New_Array(SegmentInfo,(numSegment));
      if(pSegment)
      {
        int segIndex = 0;
        int replaceStartNumber = (int)startNumber;
        int repeatIndex = 0;
        int tempTimelineIndex = 0;
        uint64 tempReplaceSegDuration = 0;
        double replaceSegDuration = 0.0;
        double replaceSegStartTime = 0.0;
        char *tmpUrl = NULL;
        bool bPeriodDurationSet = false;
        int startIndex = 0;
        bool bUpdateUsingStartNumber = true;
        while(bOk && tempTimelineIndex < numTimelineEntry && startIndex < numSegment)
        {
          bool bSegmentAvail = false;
          char *mediaUrlTemp = rep[RepIndex].GetSegmentTemplate()->GetMediaTemplate();
          char *mediaTemplate = NULL;
          char *indexTemplate = NULL;
          char *indexUrlTemp = NULL;
          int origMediaUrlLen = 0;
          if (mediaUrlTemp)
          {
            origMediaUrlLen = GetMaxBufferSizeForStringTemplate(rep[RepIndex], mediaUrlTemp);
            mediaTemplate = (char *) QTV_Malloc(origMediaUrlLen * sizeof(char));
          }
          if (mediaTemplate)
          {
            if (repeatIndex == 0)
            {
              if (rep[RepIndex].GetSegmentTemplate()->GetSegTimeLineStartTime(tempTimelineIndex) > replaceSegStartTime)
              {
                replaceSegStartTime = (double)rep[RepIndex].GetSegmentTemplate()->GetSegTimeLineStartTime(tempTimelineIndex);
                tempReplaceSegDuration = rep[RepIndex].GetSegmentTemplate()->GetSegTimeLineStartTime(tempTimelineIndex) ;
                segmentStartTime = (replaceSegStartTime/(double)timeScale)*1000;
                segmentStartTime -= (ptsOffset*1000);
              }
              replaceSegDuration = (double)rep[RepIndex].GetSegmentTemplate()->GetSegTimeLineDuration(tempTimelineIndex);
              segDuration = (replaceSegDuration/(double)timeScale)*1000;
            }

                if ((segmentStartTime + 2*segDuration) > (offsetFromAvailTime)  &&
                  (segmentStartTime + segDuration) < (relevantPeriodDuration +
                                                      lastSegmentDurationInPeriod))
                  /*This factors in segDuration additionally in segment availability time calculations*/
            {
              bSegmentAvail = true;
              QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                  "SegmentStartTime %d, segDuration %d, "
                  "availability window [%d - %d]",
                  (int)segmentStartTime, (int)segDuration, (int)offsetFromAvailTime,
                      (int)(relevantPeriodDuration + lastSegmentDurationInPeriod));
              if (periodDuration > 0.0 &&
                  periodDuration > segmentStartTime &&
                  !bPeriodDurationSet)
              {
                periodDuration -= segmentStartTime;
                bPeriodDurationSet = true;
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "ReCalculated period duration = %d", (int)periodDuration);
              }
            }
            if (bUpdateUsingStartNumber && (uint32)segIndex >= (startNumber - tempStartNumber))
            {
              if (startNumber == (uint32)tempStartNumber)
              {
                tempReplaceSegDuration = tempReplaceSegDuration + (uint32)(((double)tempStartNumber - 1)*replaceSegDuration);
              }
              else
              {
                tempReplaceSegDuration = tempReplaceSegDuration + (uint32)((double)tempStartNumber*replaceSegDuration);
              }
              bUpdateUsingStartNumber = false;
            }

            if (bSegmentAvail)
            {
              int64 startNumberOrTime = -1;
              if (IsTemplateTagPresent(mediaUrlTemp, "$Time"))
              {
                startNumberOrTime = tempReplaceSegDuration;
              }
              else if(IsTemplateTagPresent(mediaUrlTemp, "$Number"))
              {
                startNumberOrTime = replaceStartNumber;
              }

              SetBufferForStringTemplate(
                rep[RepIndex], mediaUrlTemp, mediaTemplate, origMediaUrlLen, startNumberOrTime);

              indexUrlTemp = rep[RepIndex].GetSegmentTemplate()->GetIndexTemplate();
              int origIndexUrlLen = 0;
              if (indexUrlTemp)
              {
                origIndexUrlLen = GetMaxBufferSizeForStringTemplate(rep[RepIndex], indexUrlTemp);
                indexTemplate  = (char *)QTV_Malloc(origIndexUrlLen * sizeof(char));
              }
              if (indexTemplate)
              {
                int64 startNumberOrTime = -1;
                if (IsTemplateTagPresent(mediaUrlTemp, "$Time"))
                {
                  startNumberOrTime = tempReplaceSegDuration;
                }
                else if(IsTemplateTagPresent(mediaUrlTemp, "$Number"))
                {
                      startNumberOrTime = replaceStartNumber;
                }

                SetBufferForStringTemplate(
                  rep[RepIndex],indexUrlTemp,indexTemplate,origIndexUrlLen, startNumberOrTime);
                isIndexURLPresent = true;
              }
              else
              {
                indexTemplate = mediaTemplate;
              }
              char *tmpUrl = ((repBaseURLs[0]) ? repBaseURLs[0] : NULL);
              if (bOk)
              {
                bOk = pSegment[startIndex].SetInfoURL(
                    tmpUrl, mediaTemplate, mediaRange,
                    indexTemplate, isIndexURLPresent, indexRange, indexRangeExact,
                    isInitURLPresent, intialisationUrlRange,
                    segDuration, segmentKey | ((uint64)startIndex),
                    segmentStartTime);
               if (bOk && pMpd && pMpd->IsLive())
               {
                 pSegment[startIndex].SetAvailabilityTime(pMpd->GetAvailabilityStartTime()+
                                                          (double)pPeriodInfo[PeriodIndex].getStartTime()+ (double)segDuration +
                                                          segmentStartTime);
               }
              }
              startIndex++;
            }
            segIndex++;
            replaceStartNumber++;
            repeatIndex++;
            if (repeatIndex ==
                 rep[RepIndex].GetSegmentTemplate()->GetSegTimeLineRepeatCount(tempTimelineIndex) + 1)
            {
              repeatIndex = 0;
              tempTimelineIndex++;
            }
            segmentStartTime = segmentStartTime + segDuration;
            replaceSegStartTime = replaceSegStartTime + replaceSegDuration;
            tempReplaceSegDuration = (uint32)replaceSegDuration + tempReplaceSegDuration;
            QTV_Free(mediaTemplate);
            mediaTemplate = NULL;
            if (indexUrlTemp)
            {
              QTV_Free(indexTemplate);
              indexTemplate  = NULL;
            }
          }
        }
        if(rep)
        {
          rep[RepIndex].setSegmentInfo(pSegment, (uint32)startIndex, (uint32)startIndex);
        }
        QTV_Delete_Array(pSegment);
        pSegment = NULL;
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "No segments available for segment template with timeline");
    }
    }
  }
  }
  else if (bOk &&
           m_bBaseURLElementPresent && numUrls > 0 )
  {
    int segIndex = 1;
    SegmentInfo *pSegment=NULL;
    pSegment = (SegmentInfo*)QTV_New_Array(SegmentInfo,(1));
    if (pSegment)
    {
      indexUrl = repBaseURLs[0];
      indexRange = rep[RepIndex].GetIndexSegmentRange();
      if (pPeriodInfo[PeriodIndex].getDuration() > 0.0)
      {
        double segDuration  = (pPeriodInfo[PeriodIndex].getDuration()/(double)segBaseTimeScale)*1000;
        double segmentStartTime = 0;
        pSegment[0].SetInfoBase(repBaseURLs[0], segDuration, segmentKey, segmentStartTime,
                                mediaRange, indexUrl, isIndexURLPresent, indexRange, indexRangeExact,
                                isInitURLPresent, intialisationUrlRange);
        if (pMpd && pMpd->IsLive())
         {
           pSegment[0].SetAvailabilityTime(pMpd->GetAvailabilityStartTime()+
                                           (double)pPeriodInfo[PeriodIndex].getStartTime()+ (double)segDuration +
                                           segmentStartTime);
         }
        if(rep)
        {
          rep[RepIndex].setSegmentInfo(pSegment, (uint32)segIndex, (uint32)segIndex);
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Period duration unknown");
        bOk = false;
      }
      QTV_Delete_Array(pSegment);
      pSegment = NULL;
    }
  }
  else
  {
    bOk = false;
  }
  if(repBaseURLs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(repBaseURLs[i])
      {
        QTV_Free(repBaseURLs[i]);
        repBaseURLs[i] = NULL;
      }
    }
  }
  if(repBaseURLByteRangeAttrs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(repBaseURLByteRangeAttrs[i])
      {
        QTV_Free(repBaseURLByteRangeAttrs[i]);
        repBaseURLByteRangeAttrs[i] = NULL;
      }
    }
  }
  return bOk;
}
/* This function will parse the Representation attributes for the representation
 * element of the MPD and store in the representationinfo.
 * @param[in] - Representation Element - xml element for the representation
 * @param[in] - RepKey - unique key for the representation
 * @param[in] - rep - Representation info object to store the attributes
 * @param[in] - repGrp - object for the group to which rep belongs
 * @param[out] - bRepType - Representation Type (audio/video/text)
 *               1 - audio 2- video  4- text. So if bRepType has value
                 3 it means it is av representation. If value is 6 then
                 it has video and text
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::ParseRepAttrs(MPD& /* rMpd */, TiXmlElement* RepElement,uint64 RepKey,RepresentationInfo* rep,
                              RepresentationGroup repGrp,int& bRepType, bool &bBWPresent)
{
  QTV_NULL_PTR_CHECK(RepElement,false);
  QTV_NULL_PTR_CHECK(rep,false);
  bool bOk = true;
  int RepIndex = (int)((RepKey & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT);
  TiXmlAttribute* attrib=RepElement->FirstAttribute();
  uint32 width=0,height=0,parX=0,parY=0,bandwidth=0,frameRate=0;
  char *language=NULL,*mimeType=NULL;
  char *cNumChannels=NULL,*cSamplingRate=NULL;
  char *rep_id=NULL;
  uint32 startWithSAPVal = 0;
  int sapVal = -1;
  bool bSAPPresent = false;
  bool  bSegmentAligned = false;
  char* mediaStreamStructureId = NULL;
  bool  bSubSegSAPPresent = false;
  uint32 subSegmentSAPVal = 0;
  int subSegSAP = -1;
  char *codecAttrib = NULL;
  char *profile = NULL;
  bBWPresent = false;

  // inherit the descriptor-set from the adaptation-set. Per spec, if the same element
  // is present at the adaptation-set, then it should not be present at the representation.

  ContentDescriptorType *pContentDescs = NULL;
  int numDescs = 0;
  rep[RepIndex].GetCommonDescriptors().CopyDescs(repGrp.GetCommonContentDescriptors(), true);

  for(const char **descName = Common_AdapSet_Rep_SubRep_DescriptorNames;
       *descName;
       ++descName)
  {
    bOk = ParseDescriptorTypeElements(RepElement, rep[RepIndex].GetCommonDescriptors(), *descName);

    if(false == bOk)
    {
      break;
    }
  }

  if(bOk)
  {
    ParseContentProtection(RepElement, rep[RepIndex].GetContentProtection());

    while(attrib)
    {
      char *attrib_name=(char*)skip_whitespace(attrib->Name());
      if(!std_strnicmp(attrib_name,"width",5))
      {
        width = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"height",std_strlen("height")))
      {
        height = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"parX",std_strlen("parX")))
      {
        parX= atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"parY",std_strlen("parY")))
      {
        parY = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"lang",std_strlen("language")))
      {
        language = (char*) attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"id",std_strlen("id")))
      {
        rep_id = (char*) attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"bandwidth",std_strlen("bandwidth")))
      {
        bandwidth = atoi(attrib->Value());
        bBWPresent = true;
      }
      else if(!std_strnicmp(attrib_name,"mimeType",std_strlen("mimeType")))
      {
        mimeType =(char*) attrib->Value();
        rep[RepIndex].GetCommonStringValues().AddString(attrib_name, attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"frameRate",std_strlen("frameRate")))
      {
        frameRate = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"numberOfChannels",std_strlen("numberOfChannels")))
      {
        cNumChannels = (char*) attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"audioSamplingRate",std_strlen("audioSamplingRate")))
      {
        cSamplingRate = (char*) attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"codecs",std_strlen("codecs")))
      {
        codecAttrib = (char*) attrib->Value();
        rep[RepIndex].GetCommonStringValues().AddString(attrib_name, attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"segmentAlignmentFlag",
                              std_strlen("segmentAlignmentFlag")))
      {
        if(std_strnicmp(attrib->Value(),"true",4) == 0)
        {
            bSegmentAligned = true;
        }
      }
      else if(!std_strnicmp(attrib_name,"segmentAlignment",
                                    std_strlen("segmentAlignment")))
      {
        if(std_strnicmp(attrib->Value(),"true",4) == 0)
        {
           bSegmentAligned = true;
        }
      }
      else if(!std_strnicmp(attrib_name,"startWithSAP",
                                  std_strlen("startWithSAP"))) //Mpeg-DASH
      {
        bSAPPresent = true;
        startWithSAPVal = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"mediaStreamStructureId",
                                  std_strlen("mediaStreamStructureId")))
      {
        mediaStreamStructureId = (char*) attrib->Value();
      }
      else if (!std_strnicmp(attrib_name,"bitStreamStructureId",
                                  std_strlen("bitStreamStructureId")))
      {
        mediaStreamStructureId = (char*) attrib->Value();
      }
      else if (!std_strnicmp(attrib_name,"subSegmentStartsWithSAP",
                                  std_strlen("subSegmentStartsWithSAP")))
      {
        bSubSegSAPPresent= true;
        subSegmentSAPVal = atoi(attrib->Value());
      }
      else if (!std_strnicmp(attrib_name,"subSegmentStartWithSAP",
                                  std_strlen("subSegmentStartWithSAP")))
      {
        bSubSegSAPPresent= true;
        subSegmentSAPVal = atoi(attrib->Value());
      }
      else if (!std_strnicmp(attrib_name,"profile",std_strlen("profile"))) //3GP-DASH
      {
        profile=(char*)attrib->Value();
      }
      else if (!std_strnicmp(attrib_name,"profiles",std_strlen("profiles"))) //DASH
      {
        profile=(char*)attrib->Value();
      }
      attrib=attrib->Next();
    }

    if (bSAPPresent)
    {
      sapVal = (int)startWithSAPVal;
    }
    if (bSubSegSAPPresent)
    {
      subSegSAP = (int)subSegmentSAPVal;
    }
    if (profile)
    {
      if (!std_strcmp(profile, "urn:mpeg:dash:profile:isoff-live:2011") ||
         !std_strcmp(profile, "urn:mpeg:dash:profile:iso-live"))
      {
        m_MpdProfile = DASH_PROFILE_ISO_LIVE;
      }
      else if (!std_strcmp(profile, "urn:mpeg:dash:profile:isoff-on-demand:2011") ||
         !std_strcmp(profile, "urn:mpeg:dash:profile:isoff-on-demand"))
      {
        m_MpdProfile = DASH_PROFILE_ISO_ON_DEMAND;
      }
      else if (!std_strcmp(profile, "urn:mpeg:dash:profile:mp2t-simple:2011") ||
         !std_strcmp(profile, "urn:mpeg:dash:profile:mp2t-simple"))
      {
        m_MpdProfile = DASH_PROFILE_MP2T_SIMPLE;
      }
    }

    double *pChannels=NULL;
    int numChannels=0;
    if(cNumChannels)
    {
      if(ConvertStringVectorToList(cNumChannels,numChannels,pChannels))
      {
        pChannels=(double*)QTV_Malloc(sizeof(double)*numChannels);
        if(pChannels)
        {
          ConvertStringVectorToList(cNumChannels,numChannels,pChannels);
        }
      }
    }
    double *pSamplingRate=NULL;
    int numSamplingRates=0;
    if(cSamplingRate)
    {
      if(ConvertStringVectorToList(cSamplingRate,numSamplingRates,pSamplingRate))
      {
        pSamplingRate=(double*)QTV_Malloc(sizeof(double)*numSamplingRates);
        if(pSamplingRate)
        {
          ConvertStringVectorToList(cSamplingRate,numSamplingRates,pSamplingRate);
        }
      }
    }
    Resolution *pRes=NULL;
    if(repGrp.getResolution())
    {
      Resolution *res = repGrp.getResolution();
      width = res->width;
      height = res->height;
    }
    if(repGrp.getLanguage())
    {
      language = repGrp.getLanguage();
    }
    if(!parX)
    {
      parX = repGrp.getParX();
    }
    if(!parY)
    {
      parY = repGrp.getParY();
    }
    if(!frameRate)
    {
      frameRate = (uint32)repGrp.getFrameRate();
    }

    // Filter out reps with bw greater than max supported bw.
    if(bBWPresent)
    {
      if(bandwidth > m_sessionInfo.GetMaxSupportedRepBandwidth())
      {
        QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "Filtering out rep with bw %u as it is > %u",
          bandwidth, m_sessionInfo.GetMaxSupportedRepBandwidth());
        bOk = false;
      }
    }

    Codecs *codec_info = NULL;
    if (bOk)
    {
      if (codecAttrib)
      {
        codec_info=(Codecs*)QTV_Malloc(sizeof(Codecs));
        if(codec_info)
        {
          //Initialize number of codecs to 0 and query for number of codecs.
          codec_info->numcodecs=0;
          if(ParseCodecInfo(codecAttrib,codec_info))
          {
            //After getting the number of codecs, allocate required memory.
            codec_info->mcodecs = (CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(codec_info->numcodecs));
            if(codec_info->mcodecs)
            {
              ParseCodecInfo(codecAttrib,codec_info,bRepType);
            }
          }
        }
      }
      else if(mimeType)
      {
        codec_info=(Codecs*)QTV_Malloc(sizeof(Codecs));
        if(codec_info)
        {
          //Initialize number of codecs to 0 and query for number of codecs.
          codec_info->numcodecs=1;
          codec_info->mcodecs = (CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(codec_info->numcodecs));

          if(codec_info->mcodecs)
          {
            mimeType=(char*)skip_whitespace(mimeType);
            codec_info->mcodecs[0].minorType = MN_TYPE_UNKNOWN;
            codec_info->mcodecs[0].profile = 0;
            codec_info->mcodecs[0].level = 0;

            if((!(std_strnicmp(mimeType,"video/mp4",std_strlen("video/mp4"))) ||
               (!(std_strnicmp(mimeType,"video/mp2t",std_strlen("video/mp2t"))))))
            {
              codec_info->mcodecs[0].majorType = MJ_TYPE_VIDEO;
              bRepType = 2;
            }
            else if(!(std_strnicmp(mimeType,"audio/mp4",std_strlen("audio/mp4"))))
            {
              codec_info->mcodecs[0].majorType = MJ_TYPE_AUDIO;
              bRepType = 1;
            }
            else if(!(std_strnicmp(mimeType,"application/mp4",std_strlen("application/mp4"))))
            {
              codec_info->mcodecs[0].majorType = MJ_TYPE_TEXT;
              bRepType = 4;
            }
          }
        }
      }
      else
      {
        int numCodecs = 0;
        if(repGrp.getCodec(NULL,numCodecs))
        {
          codec_info=(Codecs*)QTV_Malloc(sizeof(Codecs));
          if(codec_info)
          {
            //Initialize number of codecs to 0 and query for number of codecs.
            codec_info->numcodecs=numCodecs;
            codec_info->mcodecs = (CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(codec_info->numcodecs));

            if(codec_info->mcodecs)
            {
              repGrp.getCodec(codec_info->mcodecs,codec_info->numcodecs);
            }
          }
        }
        else
        {
          bOk = false;
        }
      }

      if(width > 0 && height > 0)
      {
        if (codec_info && IsHevcCodecPresent(codec_info))
        {
          m_sessionInfo.SetMaxSupportedRepResolution(m_sessionInfo.GetMaxSupportedHevcRepResolution());
          m_sessionInfo.SetMaxSupportedVideoBufferSize(m_sessionInfo.GetMaxSupportedHevcRepResolution());
        }
        //Ignore representation with resolution > MAX_RESOLUTION
        if((width*height) > (m_sessionInfo.GetMaxSupportedRepResolution()))
        {
          QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Ignoring Representation with width %u height %u "
                       "as resolution is more than max supported resolution %u ",
                       width, height, m_sessionInfo.GetMaxSupportedRepResolution());
          bOk = false;
        }
        if (bOk)
        {
          pRes = (Resolution*)QTV_Malloc(sizeof(Resolution));
          if(pRes)
          {
            pRes->width = width;
            pRes->height = height;
          }
        }
      }
      if (bOk)
      {
        rep[RepIndex].setRepInfo(rep_id,bandwidth,codec_info,mimeType,pRes,language,
                               parX,parY,RepKey,frameRate,numChannels,
                               pChannels,numSamplingRates,pSamplingRate,
                               bSegmentAligned,sapVal, subSegSAP, mediaStreamStructureId);
      }
    }
    if(pSamplingRate)
    {
      QTV_Free(pSamplingRate);
      pSamplingRate = NULL;
    }
    if(pChannels)
    {
      QTV_Free(pChannels);
      pChannels = NULL;
    }
    if(pRes)
    {
      QTV_Free(pRes);
      pRes = NULL;
    }
    if(codec_info)
    {
      if(codec_info->mcodecs)
      {
        QTV_Free(codec_info->mcodecs);
        codec_info->mcodecs=NULL;
      }
      QTV_Free(codec_info);
      codec_info=NULL;
    }
  }
  return bOk;

}

/* This function will parse the attributes for Group element of the
  * MPD and store in the repgrpinfo.
 * @param[in] - GroupElement - xml element for the Group
 * @param[in] - GroupKey - unique key for the Group
 * @param[in] - repGrp - Representation Group object which will store
 *              the attributes
 * @param[out] - bXlinkHrefFound - true if present
 * @param[out] - bXlinkActuateFound - true if present
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::ParseGroupAttributes(MPD& /* rMpd */, TiXmlElement* GroupElement,uint64 GroupKey,
                                     RepresentationGroup* repGrp,
                                     bool& bXlinkHrefFound,bool& bXlinkActuateFound )
{
  QTV_NULL_PTR_CHECK(GroupElement,false);
  QTV_NULL_PTR_CHECK(repGrp,false);
  int GroupIndex =(int)((GroupKey & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);

  bool bOk = true;

  for(const char **descName = Common_AdapSet_Rep_SubRep_DescriptorNames;
       *descName;
       ++descName)
  {
    bOk = ParseDescriptorTypeElements(
      GroupElement, repGrp[GroupIndex].GetCommonContentDescriptors(), *descName);

    if(false == bOk)
    {
      break;
    }
  }

  if(bOk)
  {
    for(const char **descName = AdapSet_DescriptorNames;
         *descName;
         ++descName)
    {
      bOk = ParseDescriptorTypeElements(
        GroupElement,repGrp[GroupIndex].GetAdaptationSetSpecificContentDescriptors() , *descName);

      if(false == bOk)
      {
        break;
      }
    }
  }

  if(bOk)
  {
    ParseContentProtection(GroupElement, repGrp[GroupIndex].GetContentProtection());

    TiXmlAttribute* attrib=GroupElement->FirstAttribute();
    char *repGrpId = NULL;
    uint32 width=0,height=0,parX=0,parY=0,frameRate=0;
    char *language=NULL,*mimeType=NULL;
    char *cNumChannels=NULL,*cSamplingRate=NULL;
    bXlinkHrefFound = false;
    bXlinkActuateFound = false;
    uint32 startWithSAPVal = 0;
    uint32 subSegmentSAPVal = 0;
    int sapVal = -1;
    int subSegSAPVal = -1;
    bool bSAPPresent = false;
    bool bSubSegSAPPresent = false;
    bool bSegmentAligned = false;
    bool bSubSegmentAlignment = false;
    char* mediaStreamStructureId = NULL;
    char *codecAttrib = NULL;
    char *profile = NULL;

    while(attrib)
    {
      char *attrib_name=(char*)skip_whitespace(attrib->Name());

      if(!std_strnicmp(attrib_name,"id",std_strlen("id")))
      {
        repGrpId = (char*)attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"width",5))
      {
        width = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"height",std_strlen("height")))
      {
        height = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"parX",std_strlen("parX")))
      {
        parX = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"parY",std_strlen("parY")))
      {
        parY = atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"lang",std_strlen("language")))
      {
        language =(char*) attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"mimeType",std_strlen("mimeType")))
      {
        mimeType =(char*) attrib->Value();
        repGrp[GroupIndex].GetCommonStringValueContainer().AddString(attrib_name, attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"contentType",std_strlen("contentType")))
      {
        repGrp[GroupIndex].GetAdaptationSetSpecificStringValueContainer().AddString(attrib_name, attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"frameRate",std_strlen("frameRate")))
      {
        frameRate= atoi(attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"numberOfChannels",std_strlen("numberOfChannels")))
      {
        cNumChannels = (char*) attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"audioSamplingRate",std_strlen("audioSamplingRate")))
      {
        cSamplingRate = (char*) attrib->Value();
      }
      else if(!std_strnicmp(attrib_name,"codecs",std_strlen("codecs")))
      {
        codecAttrib = (char*) attrib->Value();
        repGrp[GroupIndex].GetCommonStringValueContainer().AddString(attrib_name, attrib->Value());
      }
      else if(!std_strnicmp(attrib_name,"xlink:href",std_strlen("xlink:href")))
      {
        bXlinkHrefFound = true;
      }
      else if(!std_strnicmp(attrib_name,"xlink:actuate",std_strlen("xlink:actuate")))
      {
        bXlinkActuateFound = true;
      }
      else if(!std_strnicmp(attrib_name,"segmentAlignmentFlag",
                            std_strlen("segmentAlignmentFlag")))
      {
        if(std_strnicmp(attrib->Value(),"true",4) == 0)
        {
          bSegmentAligned = true;
        }
      }
      else if(!std_strnicmp(attrib_name,"segmentAlignment",
                                  std_strlen("segmentAlignment")))
      {
        if(std_strnicmp(attrib->Value(),"true",4) == 0)
        {
          bSegmentAligned = true;
        }
      }
      else if(!std_strnicmp(attrib_name,"subsegmentAlignment",
        std_strlen("subsegmentAlignment")))
      {
        if(std_strnicmp(attrib->Value(),"true",4) == 0)
        {
          bSubSegmentAlignment = true;
        }
      }
      else if(!std_strnicmp(attrib_name,"startWithSAP",
                                  std_strlen("startWithSAP")))
      {
        bSAPPresent = true;
        startWithSAPVal = atoi(attrib->Value());
      }
      else if (!std_strnicmp(attrib_name,"subSegmentStartsWithSAP",
                                  std_strlen("subSegmentStartsWithSAP")))
      {
        bSubSegSAPPresent= true;
        subSegmentSAPVal = atoi(attrib->Value());
      }
      else if (!std_strnicmp(attrib_name,"subSegmentStartWithSAP",
                                  std_strlen("subSegmentStartWithSAP")))
      {
        bSubSegSAPPresent= true;
        subSegmentSAPVal = atoi(attrib->Value());
      }
      else if (!std_strnicmp(attrib_name,"profile",std_strlen("profile"))) //3GP-DASH
      {
        profile=(char*)attrib->Value();
      }
      else if (!std_strnicmp(attrib_name,"profiles",std_strlen("profiles"))) //DASH
      {
        profile=(char*)attrib->Value();
      }

      attrib=attrib->Next();
    }
    if (bSAPPresent)
    {
      sapVal = (int)startWithSAPVal;
    }
    if (bSubSegSAPPresent)
    {
      subSegSAPVal = (int)subSegmentSAPVal;
    }
    if (profile)
    {
      if (!std_strcmp(profile, "urn:mpeg:dash:profile:isoff-live:2011") ||
         !std_strcmp(profile, "urn:mpeg:dash:profile:iso-live"))
      {
        m_MpdProfile = DASH_PROFILE_ISO_LIVE;
      }
      else if (!std_strcmp(profile, "urn:mpeg:dash:profile:isoff-on-demand:2011") ||
         !std_strcmp(profile, "urn:mpeg:dash:profile:isoff-on-demand"))
      {
        m_MpdProfile = DASH_PROFILE_ISO_ON_DEMAND;
      }
      else if (!std_strcmp(profile, "urn:mpeg:dash:profile:mp2t-simple:2011") ||
         !std_strcmp(profile, "urn:mpeg:dash:profile:mp2t-simple"))
      {
        m_MpdProfile = DASH_PROFILE_MP2T_SIMPLE;
      }
    }

    double *pChannels=NULL;
    int numChannels=0;
    if(cNumChannels)
    {
      if(ConvertStringVectorToList(cNumChannels,numChannels,pChannels))
      {
        pChannels=(double*)QTV_Malloc(sizeof(double)*numChannels);
        if(pChannels)
        {
          ConvertStringVectorToList(cNumChannels,numChannels,pChannels);
        }
      }
    }
    double *pSamplingRate=NULL;
    int numSamplingRates=0;
    if(cSamplingRate)
    {
      if(ConvertStringVectorToList(cSamplingRate,numSamplingRates,pSamplingRate))
      {
        pSamplingRate=(double*)QTV_Malloc(sizeof(double)*numSamplingRates);
        if(pSamplingRate)
        {
          ConvertStringVectorToList(cSamplingRate,numSamplingRates,pSamplingRate);
        }
      }
    }
    Codecs *codec_info = NULL;
    if (codecAttrib)
    {
      codec_info=(Codecs*)QTV_Malloc(sizeof(Codecs));
      if(codec_info)
      {
        //Initialize number of codecs to 0 and query for number of codecs.
        codec_info->numcodecs=0;
        if(ParseCodecInfo(codecAttrib,codec_info))
        {
          //After getting the number of codecs, allocate required memory.
          codec_info->mcodecs = (CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(codec_info->numcodecs));
          if(codec_info->mcodecs)
          {
            ParseCodecInfo(codecAttrib,codec_info);
          }
        }
      }
    }
    else if(mimeType)
    {
      codec_info=(Codecs*)QTV_Malloc(sizeof(Codecs));
      if(codec_info)
      {
        //Initialize number of codecs to 0 and query for number of codecs.
        codec_info->numcodecs=1;
        codec_info->mcodecs = (CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(codec_info->numcodecs));
        if(codec_info->mcodecs)
        {
          mimeType=(char*)skip_whitespace(mimeType);
          codec_info->mcodecs[0].minorType = MN_TYPE_UNKNOWN;
          codec_info->mcodecs[0].profile = 0;
          codec_info->mcodecs[0].level = 0;
          if((!(std_strnicmp(mimeType,"video/mp4",std_strlen("video/mp4"))) ||
             (!(std_strnicmp(mimeType,"video/mp2t",std_strlen("video/mp2t"))))))
          {
            codec_info->mcodecs[0].majorType = MJ_TYPE_VIDEO;
          }
          else if(!(std_strnicmp(mimeType,"audio/mp4",std_strlen("audio/mp4"))))
          {
            codec_info->mcodecs[0].majorType = MJ_TYPE_AUDIO;
          }
          else if(!(std_strnicmp(mimeType,"application/mp4",std_strlen("application/mp4"))))
          {
            codec_info->mcodecs[0].majorType = MJ_TYPE_TEXT;
          }
        }
      }
    }

    Resolution *pRes=NULL;
    if(width > 0 && height > 0)
    {
      if (codec_info && IsHevcCodecPresent(codec_info))
      {
        m_sessionInfo.SetMaxSupportedRepResolution(m_sessionInfo.GetMaxSupportedHevcRepResolution());
        m_sessionInfo.SetMaxSupportedVideoBufferSize(m_sessionInfo.GetMaxSupportedHevcRepResolution());
      }
      //Ignore adaptation set with resolution > MAX_RESOLUTION
      if((width*height) > (m_sessionInfo.GetMaxSupportedRepResolution()))
      {
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Ignoring adaptationset with width %u height %u "
                      "as resolution is more than max supported resolution %u ",
                       width, height, m_sessionInfo.GetMaxSupportedRepResolution());
        bOk =  false;
      }
      if (bOk)
      {
        pRes = (Resolution*)QTV_Malloc(sizeof(Resolution));
        if(pRes)
        {
          pRes->width = width;
          pRes->height = height;
        }
      }
    }
    if (bOk)
    {
      repGrp[GroupIndex].SetGroupInfo(repGrpId, pRes,parX,parY,language,mimeType, codec_info,
                                      GroupKey,bSubSegmentAlignment,frameRate,
                                      numChannels,pChannels,numSamplingRates,
                                      pSamplingRate, bSegmentAligned, sapVal, subSegSAPVal);
    }
    if(pSamplingRate)
    {
      QTV_Free(pSamplingRate);
      pSamplingRate = NULL;
    }
    if(pChannels)
    {
      QTV_Free(pChannels);
      pChannels = NULL;
    }
    if(pRes)
    {
      QTV_Free(pRes);
      pRes = NULL;
    }
    if(codec_info)
    {
      if(codec_info->mcodecs)
      {
        QTV_Free(codec_info->mcodecs);
        codec_info->mcodecs=NULL;
      }
      QTV_Free(codec_info);
      codec_info=NULL;
    }
  }

  return bOk;
}
/* This function will parse the Group element of the MPD
 * and store in the repgrpinfo.
 * @param[in] - GroupElement - xml element for the group
 * @param[in] - baseURLs - base urls to construct the absolute url
 * @param[in] - numUrls - number of base urls
 * @param[in] - GroupKey - unique key for the group
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::ParseGroup(MPD *pMpd,  TiXmlElement* GroupElement,
                           char **baseURLs, char **baseURLByteRangeAttrs,
                                       int& numUrls,
                                       uint64 GroupKey)
{
  QTV_NULL_PTR_CHECK(pMpd, false);
  QTV_NULL_PTR_CHECK(GroupElement,false);
  bool bOk = false;
  TiXmlElement *GroupChildElement=NULL;
  int numPeriods=0;
  bool bUpdateCodecInfo = true;
  PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
  RepresentationGroup* repGrps=NULL;
  int PeriodIndex = (int)((GroupKey & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
  int GroupIndex =(int)((GroupKey & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT)  ;
  uint64 grp  = (uint64)(GroupKey & MPD_REPGRP_MASK);
  int repIndex = 0;
  bool bXlinkHrefFound = false;
  bool bXlinkActuateFound = false;
  char elementSearchStr[100];


  if(pPeriodInfo)
  {
    int numRepGrps = 0;
    repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
    if(repGrps)
    {
      bOk = true;
    }
  }

  bOk = ParseGroupAttributes(*pMpd, GroupElement,GroupKey,repGrps,
                       bXlinkHrefFound, bXlinkActuateFound );

  if (bOk &&
      (m_MpdProfile == DASH_PROFILE_ISO_ON_DEMAND ||
       m_MpdProfile == DASH_PROFILE_ISO_LIVE) &&
       (bXlinkHrefFound ||bXlinkActuateFound ))
  {
    //ignore this adaptation set(or group)
    // for both live and on-demand
    bOk = false;
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Ignoring group %d for ondeamand/live profile as"
                  " bXlinkHrefFound %d bXlinkActuateFound %d",(int)GroupIndex,
                  (int)bXlinkHrefFound,(int)bXlinkActuateFound);

  }

  //Parsing SegmentBase
  TiXmlElement* SegmentBaseElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentBase", m_pNamespaceKey);
    SegmentBaseElement = GroupElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentBaseElement =  GroupElement->FirstChildElement("SegmentBase");
  }
  if (bOk && SegmentBaseElement)
  {
    bOk = ParseSegmentBase(pMpd, SegmentBaseElement, GroupKey, MPD_ADAPTATIONSET);
  }
  //inherit SegmentBase from period, if present
  if (bOk && pPeriodInfo[PeriodIndex].IsSegmentBaseFound())
  {
    (void)repGrps[GroupIndex].InheritSegmentBaseInfo(pPeriodInfo[PeriodIndex].GetSegmentBase());
  }

  //Parsing SegmentList
  TiXmlElement* SegmentListElement =  NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentList", m_pNamespaceKey);
    SegmentListElement = GroupElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentListElement =  GroupElement->FirstChildElement("SegmentList");
  }
  if(bOk && SegmentListElement)
  {
    bOk = ParseSegmentList(pMpd, SegmentListElement, baseURLs, GroupKey, MPD_ADAPTATIONSET);
  }
  //inherit SegmentList from period, if present
  if (bOk && pPeriodInfo[PeriodIndex].IsSegmentListFound())
  {
    (void)repGrps[GroupIndex].InheritSegmentList(pPeriodInfo[PeriodIndex].GetSegmentList());
  }

  //Parsing SegmentTemplate
  TiXmlElement* SegmentTemplateElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentTemplate", m_pNamespaceKey);
    SegmentTemplateElement = GroupElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentTemplateElement =  GroupElement->FirstChildElement("SegmentTemplate");
  }


  if (bOk && SegmentTemplateElement)
  {
    bOk = ParseSegmentTemplate(pMpd, SegmentTemplateElement, GroupKey, MPD_ADAPTATIONSET);
  } //inherit SegmentTemplate from  Period, if present
  if (bOk && pPeriodInfo[PeriodIndex].IsSegmentTemplateFound())
  {
    (void)repGrps[GroupIndex].InheritSegmentTemplate(pPeriodInfo[PeriodIndex].GetSegmentTemplate());
  }

  //Parsing BaseURL
  TiXmlElement *baseURLElement=NULL;
  int index = 0;
  char *adaptationBaseURLs[10] = {NULL};
  char *adaptationBaseURLByteRangeAttrs[10] = {NULL};
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:BaseURL", m_pNamespaceKey);
    baseURLElement = GroupElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    baseURLElement = GroupElement->FirstChildElement("BaseURL");
  }
  if(bOk)
  {
    if (!baseURLElement)
    {
      for(int i=0;i<numUrls;i++)
      {
        if(baseURLs[i])
        {
          adaptationBaseURLs[i] = (char *)QTV_Malloc(std_strlen(baseURLs[i])+1);
          if(adaptationBaseURLs[i])
          {
            std_strlcpy(adaptationBaseURLs[i], baseURLs[i], std_strlen(baseURLs[i])+1);
            if(baseURLByteRangeAttrs[i])
            {
              adaptationBaseURLByteRangeAttrs[i] = (char *)QTV_Malloc(std_strlen(baseURLByteRangeAttrs[i])+1);
              if (adaptationBaseURLByteRangeAttrs[i])
              {
                std_strlcpy(adaptationBaseURLByteRangeAttrs[i], baseURLByteRangeAttrs[i], std_strlen(baseURLByteRangeAttrs[i])+1);
              }
            }
          }
          index++;
        }
      }
    }
    while(bOk && baseURLElement)
    {
      if ((char*)baseURLElement->GetText())
      {
        if(adaptationBaseURLs[index])
        {
          QTV_Free(adaptationBaseURLs[index]);
          adaptationBaseURLs[index] = NULL;
        }
        if(adaptationBaseURLByteRangeAttrs[index])
        {
          QTV_Free(adaptationBaseURLByteRangeAttrs[index]);
          adaptationBaseURLByteRangeAttrs[index] = NULL;
        }
        int adaptationBaseURLLen = 0;
        char *url=(char*)baseURLElement->GetText();

        bOk =  (url != NULL) ? true: false;
        if(bOk)
        {
          char *tmpURL = NULL;
          if (baseURLs)
          {
            tmpURL = baseURLs[index];
          }
          bOk = GetResolvedURL(tmpURL,url,adaptationBaseURLs[index],adaptationBaseURLLen);
          if (bOk)
          {
            adaptationBaseURLs[index] = (char *)QTV_Malloc(adaptationBaseURLLen);
            if (adaptationBaseURLs[index])
            {
              bOk = GetResolvedURL(tmpURL,url,adaptationBaseURLs[index],adaptationBaseURLLen);
              if(bOk)
              {
                TiXmlAttribute* attrib=baseURLElement->FirstAttribute();
                while(attrib)
                {
                  char *attrib_name=(char*)skip_whitespace(attrib->Name());
                  if(!std_strnicmp(attrib_name,"byteRange",9))
                  {
                    adaptationBaseURLByteRangeAttrs[index] = (char *)QTV_Malloc(std_strlen(attrib->Value())+1);
                    if (adaptationBaseURLByteRangeAttrs[index])
                    {
                      std_strlcpy(adaptationBaseURLByteRangeAttrs[index], attrib->Value(), std_strlen(attrib->Value())+1);
                    }
                    break;
                  }
                  attrib=attrib->Next();
                }

                if(adaptationBaseURLByteRangeAttrs[index] == NULL && baseURLByteRangeAttrs[index])
                {
                  adaptationBaseURLByteRangeAttrs[index] = (char *)QTV_Malloc(std_strlen(baseURLByteRangeAttrs[index])+1);
                  if (adaptationBaseURLByteRangeAttrs[index])
                  {
                    std_strlcpy(adaptationBaseURLByteRangeAttrs[index], baseURLByteRangeAttrs[index], std_strlen(baseURLByteRangeAttrs[index])+1);
                  }
                }
              }
            }
            else
            {
              bOk = false;
            }
          }
        }
        if (bOk)
        {
          index++;
          if(!m_bBaseURLElementPresent)
          {
            m_bBaseURLElementPresent = true;
          }
        }
        if(!bOk || index >= 10)
        {
          //More than 10 alternates are not supported
          break;
        }
      }
      elementSearchStr[0] = '\0';
      if (m_pNamespaceKey)
      {
        std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:BaseURL", m_pNamespaceKey);
        baseURLElement = baseURLElement->NextSiblingElement(elementSearchStr);
      }
      else
      {
        baseURLElement = baseURLElement->NextSiblingElement("BaseURL");
      }
    }
    numUrls = index;
  }

  //Parsing Representation
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:Representation", m_pNamespaceKey);
    GroupChildElement = GroupElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    GroupChildElement = GroupElement->FirstChildElement("Representation");
  }
  if (bOk && GroupChildElement)
  {
    repGrps[GroupIndex].InitializeRepInfo();
  }

  // Check and set initialization url. It may be common initialization sement
  uint64 segmentKey=0;
  char *initialisationUrl = NULL;
  char *intialisationUrlRange = NULL;
  int64 nStartOffset = -1;
  int64 nEndOffset = -1;
  if (bOk && repGrps[GroupIndex].IsSegmentBaseFound())
  {
    if (repGrps[GroupIndex].GetSegmentBase()->GetInitialisation())//(rep[RepIndex].GetSegmentBase()->GetInitialisation())
    {
      initialisationUrl = repGrps[GroupIndex].GetSegmentBase()->GetInitialisation()->sourceURL;
      intialisationUrlRange = repGrps[GroupIndex].GetSegmentBase()->GetInitialisation()->range;
      //initialisation url
      if(initialisationUrl)
      {
        //isCommonInitURLPresent = true;
        if (adaptationBaseURLs[0])
        {
          repGrps[GroupIndex].SetInitialisationSegmentUrl(adaptationBaseURLs[0],initialisationUrl);
        }
      }
      else
      {
        // Might be Self Initialized or init present at rep level
      }

      //initialisation url range
      if (intialisationUrlRange)
      {
        parseByteRange(intialisationUrlRange, nStartOffset, nEndOffset);
      }
      repGrps[GroupIndex].SetInitialisationSegmentRange(nStartOffset, nEndOffset, intialisationUrlRange);
    }
  }
  if (bOk && repGrps[GroupIndex].IsSegmentListFound())
  {
    if (repGrps[GroupIndex].GetSegmentList()->GetInitialisation())
    {
      initialisationUrl = repGrps[GroupIndex].GetSegmentList()->GetInitialisation()->sourceURL;
      intialisationUrlRange = repGrps[GroupIndex].GetSegmentList()->GetInitialisation()->range;
      //initialisation url
      if(initialisationUrl)
      {
        //isCommonInitURLPresent = true;
        if (adaptationBaseURLs[0])
        {
          repGrps[GroupIndex].SetInitialisationSegmentUrl(adaptationBaseURLs[0],initialisationUrl);
        }
      }
      else
      {
        // Might be Self Initialized or init present at rep level
      }

      //initialisation url range
      if (intialisationUrlRange)
      {
        parseByteRange(intialisationUrlRange, nStartOffset, nEndOffset);
      }
      repGrps[GroupIndex].SetInitialisationSegmentRange(nStartOffset, nEndOffset, intialisationUrlRange);
    }
  }
  else if (bOk && repGrps[GroupIndex].IsSegmentTemplateFound())
  {
    // Initialisation url from segmenet base
    if (repGrps[GroupIndex].GetSegmentTemplate()->GetInitialisation())
    {
      char *initTemplate = repGrps[GroupIndex].GetSegmentTemplate()->GetInitialisation()->sourceURL;
      if(initTemplate && !IsTemplateTagPresent(initTemplate, "$"))
      {
        initialisationUrl = repGrps[GroupIndex].GetSegmentTemplate()->GetInitialisation()->sourceURL;
        intialisationUrlRange = repGrps[GroupIndex].GetSegmentTemplate()->GetInitialisation()->range;
        // initialisation url
        if(initialisationUrl)
        {
          //isInitURLPresent = true;
          if (adaptationBaseURLs[0])
          {
            repGrps[GroupIndex].SetInitialisationSegmentUrl(adaptationBaseURLs[0],initialisationUrl);
          }
        }
        else
        {
          // Self Initialized
        }
        // initialisation url range
        if (intialisationUrlRange)
        {
          parseByteRange(intialisationUrlRange, nStartOffset, nEndOffset);
        }
        repGrps[GroupIndex].SetInitialisationSegmentRange(nStartOffset, nEndOffset, intialisationUrlRange);
      }
    }
    // Initialisation url from segmenet base
    else if (repGrps[GroupIndex].GetSegmentTemplate()->GetInitialisationTemplate())
    {
      char *initTemplate = repGrps[GroupIndex].GetSegmentTemplate()->GetInitialisationTemplate();
      if(initTemplate && !IsTemplateTagPresent(initTemplate, "$"))
      {
        initialisationUrl = repGrps[GroupIndex].GetSegmentTemplate()->GetInitialisationTemplate();
        // initialisation url
        if(initialisationUrl)
        {
          //isInitURLPresent = true;
          if (adaptationBaseURLs[0])
          {
            repGrps[GroupIndex].SetInitialisationSegmentUrl(adaptationBaseURLs[0],initialisationUrl);
          }
        }
        else
        {
          // Self Initialized
        }
        // initialisation url range
        if (intialisationUrlRange)
        {
          parseByteRange(intialisationUrlRange, nStartOffset, nEndOffset);
        }
        repGrps[GroupIndex].SetInitialisationSegmentRange(nStartOffset, nEndOffset, intialisationUrlRange);
      }
    }
  }

  // prevreptype rightmost bit indicates audio is present, second bit
  // indicates video is present and third bit indicates text is present
  // So if prevreptype is 1 it is audio only, 2 is video only and 4 is
  // text only.
  // prevreptype = 3 indicates audio-video and so on.
  int prevRepType = -1,bRepType=-1;
  bool bBWPresent = false;
  while(bOk && GroupChildElement)
  {
    repGrps[GroupIndex].AddRepresentation(repIndex);
    uint64 repKey = (((uint64)repIndex) << MPD_REPR_SHIFT_COUNT);
    repKey = repKey | GroupKey;
    bOk = ParseRepresentation(pMpd, GroupChildElement,adaptationBaseURLs,adaptationBaseURLByteRangeAttrs,numUrls,repKey,bRepType, bBWPresent);
    if (!bOk )
    {
      bOk = repGrps[GroupIndex].RemoveRepresentation();
    }
    else
    {
      bOk = IsNextRepresentationPresent(GroupChildElement);
      if (bOk && !bBWPresent)
      {
         bOk = repGrps[GroupIndex].RemoveRepresentation();
      }
    }
    if(prevRepType == -1)
    {
      prevRepType = bRepType;
    }
    if(bRepType != prevRepType)
    {
      bUpdateCodecInfo = false;
    }
    bOk = true;
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:Representation", m_pNamespaceKey);
      GroupChildElement = GroupChildElement->NextSiblingElement(elementSearchStr);
    }
    else
    {
      GroupChildElement = GroupChildElement->NextSiblingElement("Representation");
    }
  }

  //mismatch in codec of any two representations
  if (bOk && !bUpdateCodecInfo)
  {
    bOk = false;
  }
  else if(bOk)
  {
    if(prevRepType >0 && prevRepType <=7)
    {
      Codecs *codec_info = NULL;
      codec_info=(Codecs*)QTV_Malloc(sizeof(Codecs));
      if(codec_info)
      {
        codec_info->numcodecs= (prevRepType&0x1) +(prevRepType & 0x2)/2+ (prevRepType&0x4)/4;
        codec_info->mcodecs = (CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(codec_info->numcodecs));
        if(codec_info->mcodecs)
        {
          for(int nCodec=0;nCodec < codec_info->numcodecs;nCodec++)
          {
            if(prevRepType&0x1)
            {
              codec_info->mcodecs[nCodec].majorType = MJ_TYPE_AUDIO;
              prevRepType-=1;
            }
            else if(prevRepType&0x2)
            {
              codec_info->mcodecs[nCodec].majorType = MJ_TYPE_VIDEO;
              prevRepType-=2;
            }
            else if(prevRepType&0x4)
            {
              codec_info->mcodecs[nCodec].majorType = MJ_TYPE_TEXT;
              prevRepType-=4;
            }
            codec_info->mcodecs[nCodec].minorType = MN_TYPE_UNKNOWN;
            codec_info->mcodecs[nCodec].profile = 0;
            codec_info->mcodecs[nCodec].level = 0;
          }
        }
      }
      repGrps[GroupIndex].SetCodecInfo(codec_info);
      if(codec_info)
      {
        if(codec_info->mcodecs)
        {
          QTV_Free(codec_info->mcodecs);
          codec_info->mcodecs=NULL;
        }
        QTV_Free(codec_info);
        codec_info=NULL;
      }
    }
  }
  Codecs *codec_info = NULL;
  int numCodecs = 0;
  bool bVideoRepGrp = false;
  if(bOk && repGrps[GroupIndex].getCodec(NULL,numCodecs))
  {
    codec_info=(Codecs*)QTV_Malloc(sizeof(Codecs));
    if(codec_info)
    {
      //Initialize number of codecs to 0 and query for number of codecs.
      codec_info->numcodecs=numCodecs;
      codec_info->mcodecs = (CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(codec_info->numcodecs));

      if(codec_info->mcodecs)
      {
        repGrps[GroupIndex].getCodec(codec_info->mcodecs,codec_info->numcodecs);
      }
      for ( int i = 0 ; i < codec_info->numcodecs && codec_info->mcodecs; i++)
      {
        if (codec_info->mcodecs[i].majorType == MJ_TYPE_VIDEO)
        {
          bVideoRepGrp = true;
          break;
        }
      }
      if(codec_info)
      {
        if(codec_info->mcodecs)
        {
          QTV_Free(codec_info->mcodecs);
          codec_info->mcodecs=NULL;
        }
        QTV_Free(codec_info);
        codec_info=NULL;
      }
    }
  }
  else if(bOk)
  {
    if (prevRepType == 2 || prevRepType == 3)
    {
      bVideoRepGrp = true;
    }
  }

  if (bOk)
  {
    //done parsing all the representation
    //compare mediaSttreamStructure ID of all the representations for
    // for video representation
    if (bVideoRepGrp &&
         repGrps[GroupIndex].IsMediaStreamStructIdPresentAllRep() &&
         repGrps[GroupIndex].GetNumRepresentations() > 1 &&
         !repGrps[GroupIndex].CompareMediaStreamStruct() &&
         (m_MpdProfile == DASH_PROFILE_ISO_ON_DEMAND ||
         (m_MpdProfile == DASH_PROFILE_ISO_LIVE)))
    {
      //no other Representation has the same value for @mediaStreamStructureId
      //remove this AdaptationSet/Group
      bOk = false;
    }
  }

  if (bOk && !repGrps[GroupIndex].IsValidRepGroup())
  {
    bOk = false;
  }

  if(bOk && pMpd && !pMpd->IsLive())
  {
     repGrps[GroupIndex].CommitRepInfo();
  }
  if(adaptationBaseURLs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(adaptationBaseURLs[i])
      {
        QTV_Free(adaptationBaseURLs[i]);
        adaptationBaseURLs[i] = NULL;
      }
    }
  }
  if(adaptationBaseURLByteRangeAttrs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(adaptationBaseURLByteRangeAttrs[i])
      {
        QTV_Free(adaptationBaseURLByteRangeAttrs[i]);
        adaptationBaseURLByteRangeAttrs[i] = NULL;
      }
    }
  }

  return bOk;
}


void MPDParser::InvokeCallbacks()
{
  if (m_fCallback)
  {
    (*m_fCallback)(m_fCallbackPrivData);
  }
}

/* This function will parse the Period element of the MPD
 * and store in the periodinfo.
 * @param[in] - PeriodElement - xml element for the period
 * @param[in] - baseURLs - base urls to construct the absolute url
 * @param[in] - numUrls - number of base urls
 * @param[in] - PeriodKey - unique key for the period
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::ParsePeriod(MPD *pMpd, TiXmlElement* PeriodElement, char **baseURLs, char **baseURLByteRangeAttrs, int& numUrls,uint64 PeriodKey,
                            double& prevPeriodStartTime, double& prevPeriodDuration, double currPeriodStartTime, double currPeriodDuration,
                            bool /* bIsFirstPeriod */)
{
  QTV_NULL_PTR_CHECK(pMpd, false);
  QTV_NULL_PTR_CHECK(PeriodElement, false);
  bool bOk = true;
  TiXmlElement *PeriodChildElement=NULL;
  int numPeriods=0;
  double periodDuration = 0;
  double periodStartTime = 0.0;
  double nextPeriodDuration = 0;
  double nextPeriodStartTime = 0.0;
  double mpdDuration = pMpd->getDuration();

  char *periodDurationChar = NULL;
  char *periodStartChar = NULL;
  PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
  QTV_NULL_PTR_CHECK(pPeriodInfo,false);
  int PeriodIndex = (int)((PeriodKey & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
  //Setting default values for start time and periodId.
  //TODO: Parse attributes and update. Not required for
  //Netflix simple profile.
  bool bXlinkHrefFound = false;
  bool bXlinkActuateFound = false;
  char elementSearchStr[100];
  char *periodId = NULL;

  TiXmlAttribute* attrib = PeriodElement->FirstAttribute();
  while(attrib)
  {
    char *attrib_name = (char*)skip_whitespace(attrib->Name());
    if(!std_strnicmp(attrib_name,"id",std_strlen("id")))
    {
      periodId = (char*)attrib->Value();
    }
    if(!std_strnicmp(attrib_name,"duration",std_strlen("duration")))
    {
      periodDurationChar = (char*)attrib->Value();
      if(periodDurationChar)
      {
        ConvertDurationTypeToSeconds(periodDurationChar,periodDuration);
      }
    }
    else if (!std_strnicmp(attrib_name,"start",std_strlen("start")))
    {
      periodStartChar = (char*)attrib->Value();
      if (periodStartChar)
      {
        ConvertDurationTypeToSeconds(periodStartChar,periodStartTime);
      }
    }
    else if(!std_strnicmp(attrib_name,"xlink:href",std_strlen("xlink:href")))
    {
       bXlinkHrefFound = true;
    }
    else if(!std_strnicmp(attrib_name,"xlink:actuate",std_strlen("xlink:actuate")))
    {
       bXlinkActuateFound = true;
    }
    attrib=attrib->Next();
  }
  if ((m_MpdProfile == DASH_PROFILE_ISO_ON_DEMAND ||
        m_MpdProfile == DASH_PROFILE_ISO_LIVE) &&
      (bXlinkHrefFound ||bXlinkActuateFound ))
  {
    // ignore this period for On-Demand Profile
    bOk = false;
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Ignoring period %d for ondeamnd/live profile as"
                  " bXlinkHrefFound %d bXlinkActuateFound %d",
                  (int)PeriodIndex,(int)bXlinkHrefFound,(int)bXlinkActuateFound);
  }

  if (bOk)
  {
    if (prevPeriodDuration > 0 && 0 == periodStartTime)
    {
      periodStartTime = prevPeriodStartTime + prevPeriodDuration;
    }

    if(pMpd && !pMpd->IsLive())
    {
        // For OmDemand case, use already cached value from pre scan
        periodDuration  = currPeriodDuration;
        periodStartTime = currPeriodStartTime;
    }
    else if (IsNextPeriodPresent(PeriodElement, nextPeriodStartTime, nextPeriodDuration))
    {
      // the first period should have the start-time set. If not set, assume 0.
      if (nextPeriodStartTime > 0)
      {
        if (0 == periodDuration)
        {
          periodDuration = nextPeriodStartTime - periodStartTime;
        }
      }
    }
    else if (m_bEndReached)
    {
      // there is no next period and end of mpd reached.
      if (0 == periodDuration)
      {
        periodDuration = (mpdDuration > periodStartTime
                          ? mpdDuration - periodStartTime
                          : 0.0);
      }
    }

    if (bOk)
    {
      pPeriodInfo[PeriodIndex].SetPeriodInfo(PeriodKey,(uint64)periodStartTime * 1000, periodDuration, periodId);
    }
    prevPeriodStartTime = periodStartTime;
    prevPeriodDuration = periodDuration;

  }


  //Parsing BaseURL
  TiXmlElement *baseURLElement=NULL;
  char *periodBaseURLs[10]={NULL};
  char *periodBaseURLByteRangeAttrs[10] = {NULL};
  int index = 0;

  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                    "%s:BaseURL", m_pNamespaceKey);
    baseURLElement = PeriodElement->FirstChildElement(elementSearchStr);
  }
  else
  {
   baseURLElement = PeriodElement->FirstChildElement("BaseURL");
  }
  if (!baseURLElement)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(baseURLs[i])
      {
        periodBaseURLs[i] = (char *)QTV_Malloc(std_strlen(baseURLs[i])+1);
        if(periodBaseURLs[i])
        {
          std_strlcpy(periodBaseURLs[i], baseURLs[i], std_strlen(baseURLs[i])+1);
          if(baseURLByteRangeAttrs[i])
          {
            periodBaseURLByteRangeAttrs[i] = (char *)QTV_Malloc(std_strlen(baseURLByteRangeAttrs[i])+1);
            if (periodBaseURLByteRangeAttrs[i])
            {
              std_strlcpy(periodBaseURLByteRangeAttrs[i], baseURLByteRangeAttrs[i], std_strlen(baseURLByteRangeAttrs[i])+1);
            }
          }
        }
        index++;
      }
    }
  }
  while(bOk && baseURLElement)
  {
    if ((char*)baseURLElement->GetText())
    {
      if(periodBaseURLs[index])
      {
        QTV_Free(periodBaseURLs[index]);
        periodBaseURLs[index] = NULL;
      }
      if(periodBaseURLByteRangeAttrs[index])
      {
        QTV_Free(periodBaseURLByteRangeAttrs[index]);
        periodBaseURLByteRangeAttrs[index] = NULL;
      }
      int periodBaseURLLen = 0;
      char *url=(char*)baseURLElement->GetText();
      bOk =  (url != NULL) ? true: false;
      if(bOk)
      {
        char *tmpURL = NULL;
        if (baseURLs)
        {
          tmpURL = baseURLs[index];
        }
        bOk = GetResolvedURL(tmpURL,url,periodBaseURLs[index],periodBaseURLLen);
        if (bOk)
        {
          periodBaseURLs[index] = (char *)QTV_Malloc(periodBaseURLLen);
          if (periodBaseURLs[index])
          {
            bOk = GetResolvedURL(tmpURL,url,periodBaseURLs[index],periodBaseURLLen);
            if(bOk)
            {
              TiXmlAttribute* attrib=baseURLElement->FirstAttribute();
              while(attrib)
              {
                char *attrib_name=(char*)skip_whitespace(attrib->Name());
                if(!std_strnicmp(attrib_name,"byteRange",9))
                {
                  periodBaseURLByteRangeAttrs[index] = (char *)QTV_Malloc(std_strlen(attrib->Value())+1);
                  if (periodBaseURLByteRangeAttrs[index])
                  {
                    std_strlcpy(periodBaseURLByteRangeAttrs[index], attrib->Value(), std_strlen(attrib->Value())+1);
                  }
                  break;
                }
                attrib=attrib->Next();
              }

              if(periodBaseURLByteRangeAttrs[index] == NULL && baseURLByteRangeAttrs[index])
              {
                periodBaseURLByteRangeAttrs[index] = (char *)QTV_Malloc(std_strlen(baseURLByteRangeAttrs[index])+1);
                if (periodBaseURLByteRangeAttrs[index])
                {
                  std_strlcpy(periodBaseURLByteRangeAttrs[index], baseURLByteRangeAttrs[index], std_strlen(baseURLByteRangeAttrs[index])+1);
                }
              }
            }
          }
          else
          {
            bOk = false;
          }
        }
      }
      if (bOk)
      {
        index++;
        if(!m_bBaseURLElementPresent)
        {
          m_bBaseURLElementPresent = true;
        }
      }
      if(!bOk || index >= 10)
      {
        //More than 10 alternates are not supported
        break;
      }
    }
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:BaseURL", m_pNamespaceKey);
      baseURLElement = baseURLElement->NextSiblingElement(elementSearchStr);
    }
    else
    {
      baseURLElement = baseURLElement->NextSiblingElement("BaseURL");
    }
  }
  numUrls = index;
  //Parsing SegmentBase
  TiXmlElement* SegmentBaseElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentBase", m_pNamespaceKey);
    SegmentBaseElement = PeriodElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentBaseElement =  PeriodElement->FirstChildElement("SegmentBase");
  }
  if (SegmentBaseElement)
  {
    ParseSegmentBase(pMpd, SegmentBaseElement, PeriodKey, MPD_PERIOD);
  }

  //Parsing SegmentList
  TiXmlElement* SegmentListElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentList", m_pNamespaceKey);
    SegmentListElement = PeriodElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentListElement =  PeriodElement->FirstChildElement("SegmentList");
  }
  if(SegmentListElement)
  {
    bOk = ParseSegmentList(pMpd, SegmentListElement, periodBaseURLs, PeriodKey, MPD_PERIOD);
  }

  //Parsing SegmentTemplate
  TiXmlElement* SegmentTemplateElement =  NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentTemplate", m_pNamespaceKey);
    SegmentTemplateElement = PeriodElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentTemplateElement =  PeriodElement->FirstChildElement("SegmentTemplate");
  }
  if (bOk && SegmentTemplateElement)
  {
      bOk = ParseSegmentTemplate(pMpd, SegmentTemplateElement, PeriodKey, MPD_PERIOD);
    }

  //Parsing AdaptationSet
  pPeriodInfo[PeriodIndex].InitializeGroupInfo();
  int GroupIndex = 0;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:AdaptationSet", m_pNamespaceKey);
    PeriodChildElement = PeriodElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    PeriodChildElement = PeriodElement->FirstChildElement("AdaptationSet");
  }
  while(bOk && PeriodChildElement)
  {
    pPeriodInfo[PeriodIndex].AddRepGroup(GroupIndex);
    uint64 GroupKey = ((uint64)GroupIndex << MPD_REPGRP_SHIFT_COUNT);
    GroupKey = PeriodKey | GroupKey;
    bOk = ParseGroup(pMpd, PeriodChildElement,periodBaseURLs, periodBaseURLByteRangeAttrs, numUrls,GroupKey);
     if (!bOk)
    {
       bOk = pPeriodInfo[PeriodIndex].RemoveRepGroup();
    }
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:AdaptationSet", m_pNamespaceKey);
      PeriodChildElement = PeriodChildElement->NextSiblingElement(elementSearchStr);
    }
    else
    {
      PeriodChildElement = PeriodChildElement->NextSiblingElement("AdaptationSet");
    }
  }
  if (!pPeriodInfo[PeriodIndex].IsValidPeriod())
  {
    bOk = false;
  }

  if(bOk && pMpd && !pMpd->IsLive())
  {
     pPeriodInfo[PeriodIndex].CommitGroupInfo();
  }


  if(periodBaseURLs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(periodBaseURLs[i])
      {
        QTV_Free(periodBaseURLs[i]);
        periodBaseURLs[i] = NULL;
      }
    }
  }

  if(periodBaseURLByteRangeAttrs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(periodBaseURLByteRangeAttrs[i])
      {
        QTV_Free(periodBaseURLByteRangeAttrs[i]);
        periodBaseURLByteRangeAttrs[i] = NULL;
      }
    }
  }

  return bOk;

}

/*
 * Pre scan the MPD period list to get the correct period start time and duration
 * if the perameters are missing from the period. Period start time and duration
 * are cached in a seperate period list and cached list is used to parse the period
 * list. In pre scanning of periods, to get period start time and duration
 * First period list is traversed in fordward direction and updates each period
 * start time and duration, if any period in a list misses the start time or duration
 * cached list is traversed in backword direction to get the correct period starttime
 * and duration
 */
bool MPDParser::preScanPeriods(MPD *pMpd, TiXmlElement* PeriodElement, PeriodInfoCache** PeriodArray, int& numPeriodElements)
{
  QTV_NULL_PTR_CHECK(pMpd, false);
  QTV_NULL_PTR_CHECK(PeriodElement, false);
  TiXmlElement* MPDChildElement = PeriodElement;
  bool bOk = true;
  bool isPeriodRevisitReq = false;
  double prevPeriodStartTime = 0;
  double prevPeriodDuration = 0;
  char elementSearchStr[100];
  PeriodInfoCache* tempPeriodArray = QTV_New_Array(PeriodInfoCache, 10);
  int tempPeriodArraySize = 10;
  QTV_NULL_PTR_CHECK(tempPeriodArray, false);

  int periodIndex = 0;
  bool bIsFirstPeriod = true;
  int numPeriods = 0;
  while(MPDChildElement)
  {
    numPeriods++;
    periodIndex = numPeriods - 1;
    uint64 PeriodKey = (((uint64)periodIndex) << MPD_PERIOD_SHIFT_COUNT);
    bOk = UpdatePeriodInfoFromHead(pMpd, MPDChildElement, prevPeriodStartTime, prevPeriodDuration,
                                   tempPeriodArray + periodIndex, bIsFirstPeriod, isPeriodRevisitReq, PeriodKey);

    bIsFirstPeriod = false;
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                     "%s:Period", m_pNamespaceKey);
       MPDChildElement = MPDChildElement->NextSiblingElement(elementSearchStr);
    }
    else
    {
      MPDChildElement = MPDChildElement->NextSiblingElement("Period");
    }

    double mediaPresentationDuration = pMpd->getDuration();

    if (mediaPresentationDuration > 0.0)
    {
      if (prevPeriodStartTime + prevPeriodDuration > mediaPresentationDuration)
      {
          tempPeriodArray[periodIndex].setDuration(
           mediaPresentationDuration - prevPeriodStartTime);
            break;
      }
    }

    if(numPeriods >= tempPeriodArraySize)
    {
      bOk = ResizeCachePeriodInfo(&tempPeriodArray, tempPeriodArraySize);
    }
  }

  if(bOk && isPeriodRevisitReq)
  {
    bOk = UpdatePeriodInfoFromTail(pMpd, tempPeriodArray, numPeriods);
  }

  *PeriodArray = tempPeriodArray;
  numPeriodElements = numPeriods;

  return bOk;
}
/*
 * Resizes the array size
 */
bool MPDParser::ResizeCachePeriodInfo(PeriodInfoCache** PeriodArray, int& numPeriodElements)
{
  bool bOk=false;
  QTV_NULL_PTR_CHECK(PeriodArray, false);
  PeriodInfoCache *m_pPeriodInfo = (*PeriodArray);
  QTV_NULL_PTR_CHECK(m_pPeriodInfo, false);

  PeriodInfoCache* temp;
  temp = QTV_New_Array(PeriodInfoCache,(numPeriodElements));
  if(temp)
  {
    for(int i=0;i<numPeriodElements;i++)
    {
      temp[i]=m_pPeriodInfo[i];
    }
    QTV_Delete_Array(m_pPeriodInfo);
    m_pPeriodInfo = NULL;
    m_pPeriodInfo =  QTV_New_Array(PeriodInfoCache,(numPeriodElements * 2));
    if(m_pPeriodInfo)
    {
      for(int i=0;i<numPeriodElements;i++)
      {
        m_pPeriodInfo[i]=temp[i];
      }
      bOk = true;
      (*PeriodArray) = m_pPeriodInfo;
      numPeriodElements = numPeriodElements * 2;
    }
    QTV_Delete_Array(temp);
    temp=NULL;
  }

  return bOk;
}

/* This function traverses the period list from the head to updated
 * the missing startTime and Duration of periods in the list
 * @param[in] - pMpd - MPD element
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::UpdatePeriodInfoFromHead(MPD *pMpd, TiXmlElement* PeriodElement, double& prevPeriodStartTime, double& prevPeriodDuration,
                                         PeriodInfoCache* pPeriodInfo, bool bIsFirstPeriod, bool& isPeriodRevisitReq, uint64 PeriodKey)
{
  QTV_NULL_PTR_CHECK(pMpd, false);
  QTV_NULL_PTR_CHECK(PeriodElement, false);
  bool bOk = true;
  TiXmlElement *PeriodChildElement=NULL;
  int numPeriods=0;
  double periodDuration = 0;
  double periodStartTime = 0.0;
  double nextPeriodDuration = 0;
  double nextPeriodStartTime = 0.0;
  double mpdDuration = pMpd->getDuration();

  char *periodDurationChar = NULL;
  char *periodStartChar = NULL;
  QTV_NULL_PTR_CHECK(pPeriodInfo,false);
  int PeriodIndex = (int)((PeriodKey & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
  bool bIsPreviousPeriodFirstPeriod = (PeriodIndex - 1) == 0 ? true : false;

  bool bXlinkHrefFound = false;
  bool bXlinkActuateFound = false;
  char *periodId = NULL;

  TiXmlAttribute* attrib = PeriodElement->FirstAttribute();
  while(attrib)
  {
    char *attrib_name = (char*)skip_whitespace(attrib->Name());
    if(!std_strnicmp(attrib_name,"id",std_strlen("id")))
    {
      periodId = (char*)attrib->Value();
    }
    if(!std_strnicmp(attrib_name,"duration",std_strlen("duration")))
    {
      periodDurationChar = (char*)attrib->Value();
      if(periodDurationChar)
      {
        ConvertDurationTypeToSeconds(periodDurationChar,periodDuration);
      }
    }
    else if (!std_strnicmp(attrib_name,"start",std_strlen("start")))
    {
      periodStartChar = (char*)attrib->Value();
      if (periodStartChar)
      {
        ConvertDurationTypeToSeconds(periodStartChar,periodStartTime);
      }
    }
    attrib=attrib->Next();
  }

  if (prevPeriodDuration > 0 && (prevPeriodStartTime > 0 || bIsPreviousPeriodFirstPeriod) && 0 == periodStartTime)
  {
    periodStartTime = prevPeriodStartTime + prevPeriodDuration;
  }

  if (IsNextPeriodPresent(PeriodElement, nextPeriodStartTime, nextPeriodDuration))
  {
    // the first period should have the start-time set. If not set, assume 0.
    if (nextPeriodStartTime > 0)
    {
      if (0 == periodDuration)
      {
        periodDuration = nextPeriodStartTime - periodStartTime;
      }
    }
  }
  else
  {
    // there is no next period and end of mpd reached.
    if (0 == periodDuration && (periodStartTime > 0 || bIsFirstPeriod))
    {
      periodDuration = (mpdDuration > periodStartTime
                        ? mpdDuration - periodStartTime
                        : 0.0);
    }
  }

  pPeriodInfo->SetPeriodInfo(PeriodKey,(uint64)periodStartTime, periodDuration, periodId);

  prevPeriodStartTime = periodStartTime;
  prevPeriodDuration = periodDuration;

  // If the period start time and duration not updated, these parameters could be updated
  // with revisiting period list from revers direction
  if(((!bIsFirstPeriod) && (0 == periodStartTime)) || (0 == periodDuration))
  {
    isPeriodRevisitReq = true;
  }

  return bOk;
}

/* This function traverses the period list from the tail to updated
 * the missing startTime and Duration of periods in the list
 * @param[in] - pMpd - MPD element
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::UpdatePeriodInfoFromTail(MPD *pMpd, PeriodInfoCache* pPeriodInfo, int numPeriods)
{
  bool bOk = true;

  if(bOk && pMpd && pPeriodInfo)
  {
    double nextPeriodDuration  = 0;
    double nextPeriodStartTime = 0;
    double currPeriodDuration  = 0;
    double currPeriodStartTime = 0;
    double prevPeriodDuration  = 0;
    double prevPeriodStartTime = 0;

    for(int PeriodIndex = numPeriods - 1 ; PeriodIndex >= 0 && bOk; --PeriodIndex)
    {
      bool isLastPeriod  = ((numPeriods - 1) == PeriodIndex) ? true : false;
      bool isFirstPeriod = (0 == PeriodIndex) ? true : false;

      nextPeriodDuration  = (!isLastPeriod) ?  pPeriodInfo[PeriodIndex  + 1].getDuration() : 0;
      nextPeriodStartTime = (!isLastPeriod) ?  (double)pPeriodInfo[PeriodIndex  + 1].getStartTime() : 0;

      prevPeriodDuration = (!isFirstPeriod)  ?  pPeriodInfo[PeriodIndex  - 1].getDuration() : 0;
      prevPeriodStartTime = (!isFirstPeriod) ?  (double)pPeriodInfo[PeriodIndex  - 1].getStartTime() : 0;

      currPeriodDuration  = pPeriodInfo[PeriodIndex].getDuration();
      currPeriodStartTime = (double)pPeriodInfo[PeriodIndex].getStartTime();

      if(0 == currPeriodStartTime && !isFirstPeriod)
      {
        if(prevPeriodDuration > 0 && prevPeriodStartTime > 0)
        {
          pPeriodInfo[PeriodIndex].setStartTime((uint64)(prevPeriodStartTime + prevPeriodDuration));
        }
        else if(currPeriodDuration > 0 && nextPeriodStartTime > 0)
        {
          pPeriodInfo[PeriodIndex].setStartTime((uint64)(nextPeriodStartTime - currPeriodDuration));
        }
        else if(currPeriodDuration > 0 && isLastPeriod && (pMpd->getDuration() > 0))
        {
          pPeriodInfo[PeriodIndex].setStartTime((uint64)(pMpd->getDuration() - currPeriodDuration));
        }
        else
        {
          bOk = false;
        }
      }

      if(bOk && (0 == currPeriodDuration))
      {
        if(nextPeriodStartTime > 0 && (currPeriodStartTime > 0 || isFirstPeriod))
        {
          pPeriodInfo[PeriodIndex].setDuration(nextPeriodStartTime - currPeriodStartTime);
        }
        else if(isLastPeriod && currPeriodStartTime > 0 && (pMpd->getDuration() > currPeriodStartTime))
        {
          pPeriodInfo[PeriodIndex].setDuration(pMpd->getDuration() - currPeriodStartTime);
        }
        else
        {
          bOk = false;
        }
      } //if(bOk && (0 == currPeriodDuration))
    } // for(PeriodIndex = numPeriods - 1 ; PeriodIndex >= 0 && bOk; --PeriodIndex)
  } // if(bOk)

  return bOk;
}

/* This function will provide the resolved url for the given base url
 * @param[in] - base_url - base url to be resolved against
 * @param[in] - url - relative path
 * @param[in] - newUrl - absolute url constructed
 * @param[in] - new_url_len - length of the constructed url
 * @return - bool
 * true on successful resolve false otherwise
 */
bool MPDParser::GetResolvedURL(char* base_url, char *url, char *newUrl,int& new_url_len)
{
  bool bOk = false;
  QTV_NULL_PTR_CHECK(url,false);
  if(std_strncmp(url,"http://",7))
  {
    bOk = ResolveURI(base_url,url,newUrl,new_url_len);

  }
  else
  {
    bOk = true;
    if(new_url_len > 0)
    {
        std_strlcpy(newUrl,url,new_url_len);
    }
    else
    {
      new_url_len = (int)std_strlen(url)+1;

    }
  }
  return bOk;
}
/*
* @brief
* Parses the mpd file and stores the information about periods,representation groups
* representations and segments
* @param[in] MPD text - contains the text for the MPD file
*/
bool MPDParser::Parse(char * MPDText)
{
  QTV_NULL_PTR_CHECK(mpd,false);
  StoreMPDText((const char*)MPDText);
  TiXmlDocument xmlStream;
  TiXmlAttribute* attrib = NULL;
  const char *pRet = xmlStream.Parse(MPDText);
  TiXmlElement* MPDElement;
  MPDElement = xmlStream.FirstChildElement();
  if(MPDElement)
  {
    attrib=MPDElement->FirstAttribute();
  }
  else
  {
    return false;
  }
  char *type=NULL,*mpdDuration=NULL,*mpdBufferTime=NULL,*mpdAvailabilityStartTime=NULL,*mpdTimeShiftBuffer=NULL,*minUpdatePeriodMPD=NULL;
  char *mpdURLs[10]={NULL};
  char *mpdURLByteRangeAttrs[10] = {NULL};
  char *profile = NULL;
  int numUrls=0;
  bool bOk = true;
  char elementSearchStr[100];
  mpdURLs[numUrls]=(char*)QTV_Malloc(std_strlen(m_url)+1);
  if(mpdURLs[numUrls])
  {
    std_strlcpy(mpdURLs[numUrls],m_url,std_strlen(m_url)+1);
  }
  while(attrib)
  {
    char *attrib_name = (char*)skip_whitespace(attrib->Name());
    if (!std_strnicmp(attrib_name,"xmlns",std_strlen("xmlns")))
    {
      char *tempNamespaceKey = std_strstr(attrib_name,":");
      bool bValidNamespace = false;
      if (tempNamespaceKey)
      {
        if (!std_strnicmp((char*)attrib->Value(),"urn:mpeg:dash:mpd",std_strlen("urn:mpeg:dash:mpd")))
        {
           bValidNamespace = true;
        }
        else if (!std_strnicmp((char*)attrib->Value(),"urn:3GPP:ns:PSS",std_strlen("urn:3GPP:ns:PSS")))
        {
          bValidNamespace = true;
        }
        else if (!std_strnicmp((char*)attrib->Value(),"urn:mpeg:dash:schema",std_strlen("urn:mpeg:dash:schema")))
        {
           bValidNamespace = true;
        }

        if(bValidNamespace)
        {
          m_pNamespaceVal = (char*)attrib->Value();
          tempNamespaceKey = tempNamespaceKey + 1;
          m_pNamespaceKey = tempNamespaceKey;
        }
      }
    }
    else if (!std_strnicmp(attrib_name,"profile",std_strlen("profile"))) //3GP-DASH
    {
      profile=(char*)attrib->Value();
    }
    else if (!std_strnicmp(attrib_name,"profiles",std_strlen("profiles"))) //DASH
    {
      profile=(char*)attrib->Value();
    }
    else if(!std_strnicmp(attrib_name,"type",4))
    {
      type=(char*)attrib->Value();
    }
    else if(!std_strnicmp(attrib_name,"mediaPresentationDuration",std_strlen("mediaPresentationDuration")))
    {
      mpdDuration=(char*)attrib->Value();
    }
    else if(!std_strnicmp(attrib_name,"minimumUpdatePeriodMPD",std_strlen("minimumUpdatePeriodMPD")))
    {
      minUpdatePeriodMPD=(char*)attrib->Value();
    }
    else if(!std_strnicmp(attrib_name,"availabilityStartTime",std_strlen("availabilityStartTime")))
    {
      mpdAvailabilityStartTime=(char*)attrib->Value();
    }
    else if(!std_strnicmp(attrib_name,"minBufferTime",std_strlen("minBufferTime")))
    {
      mpdBufferTime=(char*)attrib->Value();
    }
    else if(!std_strnicmp(attrib_name,"timeShiftBufferDepth",std_strlen("timeShiftBufferDepth")))
    {
      mpdTimeShiftBuffer=(char*)attrib->Value();
    }
    else if (!std_strnicmp(attrib_name,"minimumUpdatePeriod",std_strlen("minimumUpdatePeriod ")))
    {
      minUpdatePeriodMPD = (char*)attrib->Value();
    }
    attrib=attrib->Next();
  }
  double timeShiftBufferDepth = 0;
  if(mpdTimeShiftBuffer)
  {
    ConvertDurationTypeToSeconds(mpdTimeShiftBuffer, timeShiftBufferDepth);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "MPD Parsed timeshiftBufferDepth %d", (int)timeShiftBufferDepth);
  }


  m_bEndReached = true;
  if (m_MpdProfile == DASH_PROFILE_ISO_ON_DEMAND)
  {
    if(type)
    {
      if (!std_strnicmp(type,"dynamic",7))
      {
        bOk = false;
      }
    }
  }

  double duration=0,buffertime=0, minUpdatePeriod = 0.0;
  MM_Time_DateTime UTCTime ={0, 0, 0, 0, 0, 0, 0, 0};
  ConvertDurationTypeToSeconds(mpdDuration,duration);
  ConvertDurationTypeToSeconds(mpdBufferTime,buffertime);
  ConvertDurationTypeToSeconds(minUpdatePeriodMPD, minUpdatePeriod);

  //Check if mediaPresentationDuration present for static type (In case no type present default is static)
  if (NULL == type || !std_strnicmp(type,"static",6))
  {
    if(duration == 0.0)
    {
       QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "MPDParser::Parse error. No valid mediaPresentationDuration present for type='static'");
       bOk = false;
    }
  }

  if(type && !std_strnicmp(type,"dynamic",7))
  {
    if(mpdAvailabilityStartTime == NULL)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "MPDParser::Parse error. No availabilityStartTime present for type='dynamic'");
      bOk = false;
    }

    if(mpdDuration == NULL && minUpdatePeriodMPD == NULL)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "MPDParser::Parse error. Neither mediaPresentationDuration nor minimumUpdatePeriod are present for type='dynamic'");
      bOk = false;
    }
  }

  double availabilityStartTime = 0;

  if(mpdAvailabilityStartTime)
  {
    if(false == StreamSourceTimeUtils::GetUTCTimeInMsecsFromXMLDateTime(mpdAvailabilityStartTime, availabilityStartTime))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Unable to convert DateTimeType to UTC Time");
      bOk = false;
    }
  }

  #ifdef FEATURE_HTTP_TEST_LIVE_USING_FAKE_AVAILABILITY_TIME
  {
    if(std_strstri(GetURL(), "HTTPSourceDashUnitTest/"))
    {
      // Live unit test case
      if(MAX_UINT32_VAL == m_nLiveTest_FakeAvailStartTime)
      {
        MM_Time_DateTime tmpCurTime;
        MM_Time_GetUTCTime(&tmpCurTime);

        QTV_MSG_PRIO7( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "FEATURE_HTTP_TEST_LIVE_USING_FAKE_AVAILABILITY_TIME XML date time from which offset in mpd url is subtracted %d-%d-%d %d:%d:%d.%d",
          tmpCurTime.m_nYear, tmpCurTime.m_nMonth, tmpCurTime.m_nDay, tmpCurTime.m_nHour, tmpCurTime.m_nMinute, tmpCurTime.m_nSecond, tmpCurTime.m_nMilliseconds);

        double tmpCurMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(tmpCurTime);

        bool isNegative = std_strstri(GetURL(), "minus_");
        const char *offsetPos = NULL, *endPos = NULL;

        if (!isNegative)
        {
          offsetPos = std_strstri(GetURL(), "plus_");
          offsetPos = offsetPos ? offsetPos + std_strlen("plus_") : offsetPos;
        }
        else
        {
          offsetPos = std_strstri(GetURL(), "minus_");
          offsetPos = offsetPos ? offsetPos + std_strlen("minus_") : offsetPos;
        }

        if(offsetPos)
        {
          endPos = std_strstri(offsetPos, "_");
        }

        if(endPos)
        {
          int numToCopy = endPos - offsetPos;
          char tmpBuf[100];

          if(numToCopy > 0 && numToCopy < (int)sizeof(tmpBuf) - 1)
          {
            memcpy(tmpBuf, offsetPos, numToCopy);
            tmpBuf[numToCopy] = 0;

            double tmpDiff = atoi(tmpBuf);
            tmpDiff = (isNegative ? -tmpDiff : tmpDiff);

            m_nLiveTest_FakeAvailStartTime = tmpCurMSeconds - tmpDiff;
          }

        }
      }

      // ovverride the avail start time for unit testing if mpd@availabilityStartTime
      // exists in mpd
      if(mpdAvailabilityStartTime)
      {
        availabilityStartTime = m_nLiveTest_FakeAvailStartTime;
      }
    }
  }
#endif

  if (mpd->IsMpdValid())
  {
    if (mpd->GetAvailabilityStartTime() != availabilityStartTime)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Error: availabilityStartTime mismatch in new mpd");
      bOk = false;
    }
  }

  if (bOk)
  {
    MPD *pMpd = QTV_New(MPD);

    if (NULL == pMpd)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Failed to allocated MPD");
       bOk = false;
    }
    else
    {

      if (profile)
      {
        if (!std_strcmp(profile, "urn:mpeg:dash:profile:isoff-live:2011") ||
           !std_strcmp(profile, "urn:mpeg:dash:profile:iso-live"))
        {
          m_MpdProfile = DASH_PROFILE_ISO_LIVE;
        }
        else if (!std_strcmp(profile, "urn:mpeg:dash:profile:isoff-on-demand:2011") ||
           !std_strcmp(profile, "urn:mpeg:dash:profile:isoff-on-demand"))
        {
          m_MpdProfile = DASH_PROFILE_ISO_ON_DEMAND;
        }
        else if (!std_strcmp(profile, "urn:mpeg:dash:profile:mp2t-simple:2011") ||
           !std_strcmp(profile, "urn:mpeg:dash:profile:mp2t-simple"))
        {
          m_MpdProfile = DASH_PROFILE_MP2T_SIMPLE;
        }

        int reqdSize = (int)std_strlen(profile) + 1;
        m_MpdProfileStr = (char *)QTV_Malloc(reqdSize * sizeof(char));
        if(m_MpdProfileStr)
        {
          std_strlcpy(m_MpdProfileStr, profile, reqdSize);
        }
      }

      if(type)
      {
        if (!std_strnicmp(type,"dynamic",7))
        {
          pMpd->SetLive();

          if (!minUpdatePeriodMPD)
          {
            // this should be for the last dynamic mpd?
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Last dynamic MPD received");
            m_bEndReached = true;
          }
          else
          {
            m_bEndReached = false;

            if(duration > 0)
            {
              double presentationEndTime = availabilityStartTime + 1000.0 * duration;

              MM_Time_DateTime sCurrTime;
              MM_Time_GetUTCTime(&sCurrTime);
              double nCurrMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(sCurrTime);
              double maxTimeForNextRefresh = nCurrMSeconds + 1000.0 * minUpdatePeriod;
              double diff = maxTimeForNextRefresh - presentationEndTime;

              if(diff >= 0.0)
              {
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "End of mpd reached as max time for next refresh exceeds presentationEndTime by %d ms",
                  (int)diff);
                m_bEndReached = true;
              }
            }
          }
        }
      }

      pMpd->SetMPDInfo(duration,buffertime,timeShiftBufferDepth, minUpdatePeriod, availabilityStartTime);

      TiXmlElement *MPDChildElement=NULL;
      elementSearchStr[0] = '\0';
      if (m_pNamespaceKey)
      {
        std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:BaseURL", m_pNamespaceKey);
        MPDChildElement = MPDElement->FirstChildElement(elementSearchStr);
      }
      else
      {
        MPDChildElement = MPDElement->FirstChildElement("BaseURL");
      }

      while(MPDChildElement)
      {
        if(numUrls >= 10)
        {
          //More than 10 alternate urls. Ignoring rest of them
        }
        else if(MPDChildElement->GetText())
        {
          if(mpdURLs[numUrls])
          {
            QTV_Free(mpdURLs[numUrls]);
            mpdURLs[numUrls] = NULL;
          }
          if(mpdURLByteRangeAttrs[numUrls])
          {
            QTV_Free(mpdURLByteRangeAttrs[numUrls]);
            mpdURLByteRangeAttrs[numUrls] = NULL;
          }
          int mpdUrllen = 0;
          char *url=(char*)MPDChildElement->GetText();
          if (url)
          {
            bOk = GetResolvedURL(m_url,url,mpdURLs[numUrls],mpdUrllen);
          }
          if(bOk)
          {
            mpdURLs[numUrls]=(char*)QTV_Malloc(mpdUrllen);
            if(mpdURLs[numUrls])
            {
              bOk = GetResolvedURL(m_url,url,mpdURLs[numUrls],mpdUrllen);

              TiXmlAttribute* attrib=MPDChildElement->FirstAttribute();
              while(attrib)
              {
                char *attrib_name=(char*)skip_whitespace(attrib->Name());
                if(!std_strnicmp(attrib_name,"byteRange",9))
                {
                  mpdURLByteRangeAttrs[numUrls] = (char *)QTV_Malloc(std_strlen(attrib->Value())+1);
                  if (mpdURLByteRangeAttrs[numUrls])
                  {
                    std_strlcpy(mpdURLByteRangeAttrs[numUrls], attrib->Value(), std_strlen(attrib->Value())+1);
                  }
                  break;
                }
                attrib=attrib->Next();
              }
            }else
            {
              bOk = false;
            }
          }
          if(bOk)
          {
            numUrls++;
            if(!m_bBaseURLElementPresent)
            {
              m_bBaseURLElementPresent = true;
            }
          }
        }
        elementSearchStr[0] = '\0';
        if (m_pNamespaceKey)
        {
          std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:BaseURL", m_pNamespaceKey);
          MPDChildElement = MPDChildElement->NextSiblingElement(elementSearchStr);
        }
        else
        {
          MPDChildElement = MPDChildElement->NextSiblingElement("BaseURL");
        }
      }
      if(numUrls == 0)
      {
        //No base url or empty base url tag
        numUrls = 1;
      }
      elementSearchStr[0] = '\0';
      if (m_pNamespaceKey)
      {
        std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:Period", m_pNamespaceKey);
        MPDChildElement = MPDElement->FirstChildElement(elementSearchStr);
      }
      else
      {
        MPDChildElement = MPDElement->FirstChildElement("Period");
      }
      pMpd->InitializePeriodInfo();
      int PeriodIndex = 0;
      bool bIsFirstPeriod = true;
      double prevPeriodStartTime = 0;
      double prevPeriodDuration = 0;
      double currPeriodStartTime = 0;
      double currPeriodDuration = 0;
      PeriodInfoCache* cachedPeriodInfo = NULL;
      int numCachedPeriods = 0;
      int cachedPeriodInfoIndex = 0;

      if(pMpd && !pMpd->IsLive())
      {
        // Pre scan the MPD period list to get the correct period start time and duration
        // if the perameters are missing from the period. Period start time and duration
        // are cached in a seperate period list and cached list is used to parse the period
        // list. In pre scanning of periods, to get period start time and duration
        // First period list is traversed in fordward direction and updates each period
        // start time and duration, if any period in a list misses the start time or duration
        // cached list is traversed in backword direction to get the correct period starttime
        // and duration
        bOk = preScanPeriods(pMpd, MPDChildElement, &cachedPeriodInfo, numCachedPeriods);
      }

      if(bOk)
      {
        while(MPDChildElement)
        {
          (void)pMpd->AddPeriod(PeriodIndex);
          uint64 PeriodKey = (((uint64)PeriodIndex) << MPD_PERIOD_SHIFT_COUNT);

          //For On Demand case we already perScanned the period array for duration and start time
          //Hence startime and duration of the period can be fetched from cached period array
          if(pMpd && !pMpd->IsLive())
          {
            currPeriodDuration  = cachedPeriodInfo[cachedPeriodInfoIndex].getDuration();
            currPeriodStartTime = (double)cachedPeriodInfo[cachedPeriodInfoIndex].getStartTime();
            cachedPeriodInfoIndex++;
          }

          bOk = ParsePeriod(pMpd, MPDChildElement,mpdURLs,mpdURLByteRangeAttrs,numUrls,PeriodKey,
                        prevPeriodStartTime, prevPeriodDuration, currPeriodStartTime, currPeriodDuration,
                        bIsFirstPeriod);

          bIsFirstPeriod = false;
          if (!bOk)
          {
            bOk = pMpd->RemovePeriod();
          }
          if (m_pNamespaceKey)
          {
            std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                         "%s:Period", m_pNamespaceKey);
            MPDChildElement = MPDChildElement->NextSiblingElement(elementSearchStr);
          }
          else
          {
            MPDChildElement = MPDChildElement->NextSiblingElement("Period");
          }

          double mediaPresentationDuration = pMpd->getDuration();

          if (mediaPresentationDuration > 0.0)
          {
            if (prevPeriodStartTime >= mediaPresentationDuration)
            {
              pMpd->RemovePeriod();
              break;
            }
            else if (prevPeriodStartTime + prevPeriodDuration > mediaPresentationDuration)
            {
              int numPeriods = 0;
              PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);

              if (pPeriodInfo && numPeriods > 0)
              {
                pPeriodInfo[numPeriods - 1].setDuration(
                mediaPresentationDuration - prevPeriodStartTime);
              }
              else
              {
                // sanity check failed
                bOk = false;
              }

              break;
            }
          }
        } //while(MPDChildElement)
      } // if(bOk)

      if(cachedPeriodInfo)
      {
        QTV_Delete_Array(cachedPeriodInfo);
        cachedPeriodInfo = NULL;
      }

      elementSearchStr[0] = '\0';
      if(m_bEndReached)
      {
        pMpd->CommitPeriodInfo();
      }

      if (!pMpd->IsValidMPD())
      {
        bOk = false;
      }

      if ((mpd) && (mpd->IsMpdValid()))
      {
        // retrieved an mpd
        int64 minUpdatePeriod = (int)(1000 * mpd->getMinimumUpdatePeriod());
        int64 nElapsedTime = m_pDownloader->GetElapsedTimeForPlaylistUpdate();
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                      "MPD refresh check nElapsedTime %u, minUpdatePeriod %u",
                      (uint32)nElapsedTime, (uint32)minUpdatePeriod);

        AdjustKeysForNewMPD(*pMpd, *mpd, (nElapsedTime <= minUpdatePeriod));
      }
      else
      {
        // first time
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "prevMPD not valid");
      }

      if (mpd)
      {
        MM_CriticalSection_Enter(m_pParserDataLock);
        QTV_Delete(mpd);
        mpd = pMpd;
        MM_CriticalSection_Leave(m_pParserDataLock);
        InvokeCallbacks();

        PrintMPD();
      }
    }
  }

  if(mpdURLs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(mpdURLs[i])
      {
        QTV_Free(mpdURLs[i]);
        mpdURLs[i] = NULL;
      }
    }
  }

  if(mpdURLByteRangeAttrs != NULL)
  {
    for(int i=0;i<numUrls;i++)
    {
      if(mpdURLByteRangeAttrs[i])
      {
        QTV_Free(mpdURLByteRangeAttrs[i]);
        mpdURLByteRangeAttrs[i] = NULL;
      }
    }
  }

  if(bOk)
  {
    MarkDefaultRepGrpsAsSelectable(true);
    DoContentSelection();
  }
  return bOk;
}

/**
 * Used to update the unique keys of the new mpd aftercomapring
 * with the previously stored mpd.
 */
void MPDParser::AdjustKeysForNewMPD(MPD& rNewMPD, MPD& rOldMPD, bool bIsOnTime)
{
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "AdjustKeysForNewMPD: isMPDRefreshedOnTime %d", bIsOnTime);

  int nTimeOffsetPeriods = 0, correspondingOldPeriodIdx = -1;
  AdjustNewMPD(rNewMPD, rOldMPD, nTimeOffsetPeriods, correspondingOldPeriodIdx);

  if(correspondingOldPeriodIdx >= 0)
  {
    // The first period of the new mpd can be related to a period of the
    // previous mpd and correspondingOldPeriodIdx is its array idx in the
    // period_info array.
    AdjustPeriodKeys(rNewMPD,rOldMPD, nTimeOffsetPeriods, correspondingOldPeriodIdx);

    int numOldPeriods = 0, numNewPeriods = 0;
    PeriodInfo *oldPeriodInfoArray = rOldMPD.getPeriodInfo(numOldPeriods);
    PeriodInfo *newPeriodInfoArray = rNewMPD.getPeriodInfo(numNewPeriods);

    if (oldPeriodInfoArray && newPeriodInfoArray &&
        numOldPeriods > 0 && numNewPeriods > 0)
    {
      uint64 nOldMPDFirstPeriodKey = oldPeriodInfoArray[0].getPeriodKey();
      uint64 nNewMPDFirstPeriodKey = newPeriodInfoArray[0].getPeriodKey();

      AdjustSegmentKeys(rNewMPD,rOldMPD,nTimeOffsetPeriods,correspondingOldPeriodIdx);
    }
  }
  else
  {
    // MPD refresh must have occured too late, or the new mpd refresh occured at
    // period boundary.
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "First period in new MPD is not relatable to a period of  old mpd. IsOntime %d",
      bIsOnTime);

    int numOldPeriods = 0, numNewPeriods = 0;
    PeriodInfo *oldPeriodInfoArray = rOldMPD.getPeriodInfo(numOldPeriods);
    PeriodInfo *newPeriodInfoArray = rNewMPD.getPeriodInfo(numNewPeriods);

    if (oldPeriodInfoArray && newPeriodInfoArray &&
      numOldPeriods > 0 && numNewPeriods > 0)
    {
      static const uint64 one = 1;

      // The key corresponding to the 'periodKey' only
      uint64 newPeriodStartKey =
        (one << 56) + (oldPeriodInfoArray[numOldPeriods - 1].getPeriodKey() & MPD_PERIOD_MASK);

      for (uint64 i = 0; i < (uint64)numNewPeriods; ++i)
      {
        newPeriodInfoArray[i].SetPeriodKey((i << MPD_PERIOD_SHIFT_COUNT) + newPeriodStartKey);
      }

      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Updated period_index from %u to %u",
        (uint32)((uint64)((current_period_key & MPD_PERIOD_MASK >> MPD_PERIOD_SHIFT_COUNT))),
        (uint32)((uint64)((newPeriodStartKey & MPD_PERIOD_MASK >> MPD_PERIOD_SHIFT_COUNT))));

      int oldPeriodDuration = 1000 * (int)oldPeriodInfoArray[numOldPeriods - 1].getDuration();

      int maxOldPeriodDuration = (int)(newPeriodInfoArray[0].getStartTime() -
        oldPeriodInfoArray[numOldPeriods - 1].getStartTime());

      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Old Period Duration %d, Max allowed = %d",
        oldPeriodDuration, maxOldPeriodDuration);

      if (0 == oldPeriodDuration || oldPeriodDuration > maxOldPeriodDuration)
      {
        oldPeriodInfoArray[numOldPeriods - 1].setDuration(maxOldPeriodDuration);
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Updated old period duration to %d",
          maxOldPeriodDuration);
      }
    }
  }
}

/**
 * Adjusts the availability time of the mpd to that of the
 * previous mpd in case of dynamic mpd if both the mpd's can be
 * related, that is, there is a period in the new mpd that
 * corresponds to the old one.
 */
void MPDParser::AdjustNewMPD(MPD& rNewMPD, MPD& rOldMPD,
                             int& nTimeOffsetPeriods,
                             int& correspondingOldPeriodIdx)
{
  nTimeOffsetPeriods = 0;

  double oldMPDAvailabliltyTimeMsecs = rOldMPD.GetAvailabilityStartTime();
  double newMPDAvailabliltyTimeMsecs = rNewMPD.GetAvailabilityStartTime();

  int numOldPeriods = 0, numNewPeriods = 0;
  PeriodInfo* oldPeriodInfoArray = rOldMPD.getPeriodInfo(numOldPeriods);
  PeriodInfo* newPeriodInfoArray = rNewMPD.getPeriodInfo(numNewPeriods);

  correspondingOldPeriodIdx = -1;

  // Check to see if the first period of the new MPD exists in the old MPD.
  if (numNewPeriods > 0)
  {
    const char *pPeriodIdInNewMPD = newPeriodInfoArray[0].getPeriodIdentifier();
    if (!pPeriodIdInNewMPD)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Period@id not used. Treat as single period use case");

      correspondingOldPeriodIdx = 0;
    }
    else
    {
      // Period@id used. May be single for multiple period use case.
      for (int i = 0; i < numOldPeriods; ++i)
      {
        if (oldPeriodInfoArray[i].getPeriodIdentifier())
        {
          if (0 == std_strcmp(pPeriodIdInNewMPD,
                              oldPeriodInfoArray[i].getPeriodIdentifier()))
          {
            correspondingOldPeriodIdx = i;
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "Used period idntifiers to identify correspondingOldPeriodIdx %d",
                          correspondingOldPeriodIdx);
            break;
          }
          else
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "Skip over old period %d", i);
            continue;
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Sanity check failed");
        }
      }

      if (correspondingOldPeriodIdx >= 0)
      {
        // the first period in the new mpd corresponds to a period in the old mpd.
        uint64 nCorrespondingOldPeriodStartTime =
          oldPeriodInfoArray[correspondingOldPeriodIdx].getStartTime();

        nTimeOffsetPeriods =
          (int)(newPeriodInfoArray[0].getStartTime() -
                oldPeriodInfoArray[correspondingOldPeriodIdx].getStartTime());
         QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "AdjustNewMPD: nTimeOffsetPeriods %d", (int)nTimeOffsetPeriods);
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Did not find any corresponding period in old mpd");
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Invalid mpd received in refresh. Zero periods");
  }
}

/**
 * Updates if needed the period keys of new mpd after comparing
 * with the old mpd.
 */
void MPDParser::AdjustPeriodKeys(MPD& rNewMPD, MPD& rOldMPD, int nTimeOffsetPeriods, int correspondingOldPeriodIdx)
{
  // "Each Period has a conceptual start time PeriodStart in the Media
  // Presentation. Period elements shall be physically ordered in the
  // MPD in increasing order of their PeriodStart time."
  // @start "specifies the start time of the Period relative to the value
  // of MPD@availabilityStartTime for live services and relative to the
  // start of the first period for on-demand services."
  int numOldPeriods = 0, numNewPeriods = 0;
  PeriodInfo* oldPeriodInfoArray = rOldMPD.getPeriodInfo(numOldPeriods);
  PeriodInfo* newPeriodInfoArray = rNewMPD.getPeriodInfo(numNewPeriods);
  uint64 newPeriodStartKey = MAX_UINT64_VAL;

  if (correspondingOldPeriodIdx >= 0 && oldPeriodInfoArray && newPeriodInfoArray)
  {
    newPeriodStartKey = oldPeriodInfoArray[correspondingOldPeriodIdx].getPeriodKey();

    // Possible optimization:
    // If correspondingOldPeriodIdx == 0 && the top period keys in new and old
    // mpd's match up can skip to do the 'for' loop below.
    for (uint64 i = 0; i < (uint64)numNewPeriods; ++i)
    {
      if (0 == i)
      {
        // only the first new period needs adjusting wrt to times
        uint64 prevStartTime = newPeriodInfoArray[i].getStartTime();
        newPeriodInfoArray[i].setStartTime(prevStartTime - nTimeOffsetPeriods);
        uint32 prevDuration = (uint32)newPeriodInfoArray[i].getDuration();

        if (prevDuration > 0.0)
        {
          int64 adjustedPeriodDuration = nTimeOffsetPeriods/1000 + prevDuration;
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "AdjustPeriodKeys: Adjusted startTime %d, adjustedPeriodDuration %d",
                        (int)newPeriodInfoArray[i].getStartTime(), (int)adjustedPeriodDuration);
          newPeriodInfoArray[i].setDuration((double)adjustedPeriodDuration);
        }

        QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "AdjustPeriodKeys: Adjusted periodStart from %u to %u, duration from %u to %u",
                      (uint32)prevStartTime, (uint32)(prevStartTime - nTimeOffsetPeriods),
                      (uint32)prevDuration, (uint32)(prevDuration + nTimeOffsetPeriods/1000));
      }
      newPeriodInfoArray[i].SetPeriodKey((i << MPD_PERIOD_SHIFT_COUNT) + newPeriodStartKey);
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "AdjustPeriodKeys: Sanity check failed");
  }
}

/**
 * Updates if needed the unique segment keys of new mpd on
 * comparing with the old mpd.
 */
void MPDParser::AdjustSegmentKeys(
  MPD& rNewMPD, MPD& rOldMPD, int& nTimeOffsetSegment, int correspondingOldPeriodIdx)
{
  // All periods have adjusted keys at this point.
  int numOldPeriods = 0, numNewPeriods = 0;
  PeriodInfo *oldPeriodInfoArray = rOldMPD.getPeriodInfo(numOldPeriods);
  PeriodInfo *newPeriodInfoArray = rNewMPD.getPeriodInfo(numNewPeriods);

  bool bFirstNewPeriod = false;

  if(correspondingOldPeriodIdx >= 0 &&
    oldPeriodInfoArray && newPeriodInfoArray &&
    numOldPeriods > 0 && numNewPeriods > 0)
  {
    int maxNumOldPeriodsToCmp = numOldPeriods - correspondingOldPeriodIdx;
    int maxNumPeriodsToCmp = STD_MIN(maxNumOldPeriodsToCmp, numNewPeriods);

    for (int i = 0; i < maxNumPeriodsToCmp; ++i)
    {
      bFirstNewPeriod = (i==0) ? true : false;
      PeriodInfo& newPeriod = newPeriodInfoArray[i];
      uint64 newPeriodKey = newPeriod.getPeriodKey(); // this is already adjusted.

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "AdjustSegmentKeys: newPeriodIdx %u",
        (uint32)((uint64)((newPeriodKey & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT)));

      PeriodInfo *correspondingOldPeriod = &oldPeriodInfoArray[correspondingOldPeriodIdx + i];

      int numNewRepGrps = 0;
      RepresentationGroup *newRepGrp = newPeriod.getRepGrpInfo(numNewRepGrps);

      if (newRepGrp)
      {
        for (int j = 0; j < numNewRepGrps; ++j)
        {
          int numNewReps = 0;
          RepresentationInfo *newRepArray = newRepGrp[j].getRepInfo(numNewReps);

          if (newRepArray)
          {
            for (int k = 0; k < numNewReps; ++k)
            {
              // This is mandatory and unique across a rep.
              RepresentationInfo& newRep = newRepArray[k];
              char *newRepID = newRep.getRepIdentifier();
              RepresentationInfo *correspondingOldRep =
                correspondingOldPeriod->GetRepresentationByIdentifier(newRepID);
              if (correspondingOldRep && correspondingOldRep->GetSegmentFunc())
              {
                correspondingOldRep->GetSegmentFunc()->AdjustSegmentKeys(correspondingOldRep, &newRep, newPeriodKey, nTimeOffsetSegment, bFirstNewPeriod);
              }
              else
              {
                // nothing to do for this new rep. This is a rep which was not
                // present in the prev mpd which should never happen
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "AdjustSegmentKeys: Did not find newRep with ID '%s' in old MPD", newRepID);
              }
            }
          }
        }

      }
    }
  }
}

/**
 * Updates if needed the unique segment keys of new mpd on
 * comparing with the old mpd.
 */
void RepresentationInfo::SegmentFuncDefault::AdjustSegmentKeys(RepresentationInfo* pOldRep, RepresentationInfo* pNewRep, uint64 newPeriodKey, int& nTimeOffsetSegment, bool bFirstNewPeriod)
{
  QTV_NULL_PTR_CHECK(pOldRep, RETURN_VOID);
  QTV_NULL_PTR_CHECK(pNewRep, RETURN_VOID);

  uint32 numNewSegments = 0, numOldSegments = 0, tmpSize = 0;
  SegmentInfo *newSegmentArray = pNewRep->getSegmentInfo(numNewSegments, tmpSize);
  SegmentInfo *oldSegmentArray = pOldRep->getSegmentInfo(numOldSegments, tmpSize);

  if (oldSegmentArray && newSegmentArray)
  {
    uint64 newSegmentStartKey = MAX_UINT64_VAL;
    char *newRepFirstURL = newSegmentArray[0].GetURL();

    if (newRepFirstURL)
    {
      bool bAreSegmentsCommon = false;
      QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "AdjustSegmentKeys Lookup rep '%s' for segment with startTime %d",
        pNewRep->getRepIdentifier() ? pNewRep->getRepIdentifier() : "", (int)(nTimeOffsetSegment + newSegmentArray[0].getStartTime()));
      SegmentInfo *correspondingOldSegment = pOldRep->GetSegmentInfoByStartTime(nTimeOffsetSegment + newSegmentArray[0].getStartTime());

      if (correspondingOldSegment)
      {
        newSegmentStartKey = correspondingOldSegment->getKey();
        bAreSegmentsCommon = true;
         QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "AdjustSegmentKeys: url '%s' in old rep with key %llu starttime %f",
          correspondingOldSegment->GetURL(), newSegmentStartKey, correspondingOldSegment->getStartTime());
      }
      else
      {
        // refresh did not occur in time.
        newSegmentStartKey = 1 + oldSegmentArray[numOldSegments - 1].getKey();
        QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "AdjustSegmentKeys: Did not find url '%s' in old rep  and newSegmentStartKey is %llu ",
          newRepFirstURL, newSegmentStartKey);
      }

      uint64 newSegmentKey = newSegmentStartKey;
      newSegmentKey = newSegmentKey & (~MPD_PERIOD_MASK);
      newSegmentKey = newSegmentKey | (newPeriodKey & MPD_PERIOD_MASK);

      for (uint32 l = 0; l < numNewSegments; ++l)
      {
        newSegmentArray[l].SetKey(newSegmentKey);
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
          "AdjustSegmentKeys: SegmentKey period_idx %u, idx %u segkey %u",
          (uint32)((newSegmentKey & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT),
          l,
          (uint32)newSegmentArray[l].getKey());

        if (bFirstNewPeriod)
        {
          // Only the first new period needs adjusting wrt to times.
          double prevStartTime = newSegmentArray[l].getStartTime();
          newSegmentArray[l].SetStartTime((uint32)(prevStartTime + (double)nTimeOffsetSegment));
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
            "AdjustSegmentKeys: Adjusted segstartTime from %u to %u",
            (uint32)prevStartTime, (uint32)(prevStartTime + nTimeOffsetSegment));
        }
        ++newSegmentKey;

        // Sanity check. If this check fails, then the MPD violates the spec.
        SegmentInfo *oldSegment =
          pOldRep->GetSegmentInfoByURL(newSegmentArray[l].GetURL());

        if (oldSegment)
        {
          if (oldSegment->getKey() != newSegmentArray[l].getKey())
          {
            QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "Sanity check for segment key failed. oldSegmentKey %u, "
              "newSegmentKey %u, new segmentIdx %u", (uint32)oldSegment->getKey(),
              (uint32)newSegmentArray[l].getKey(), l);
          }
        }
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Unexpected error. First segment in new mpd has null url");
    }
  }

}


void RepresentationInfo::SegmentFuncTemplate::AdjustSegmentKeys(RepresentationInfo* pOldRep, RepresentationInfo* pNewRep, uint64 /* newPeriodKey */, int& /* nTimeOffsetSegment */, bool /* bFirstNewPeriod */)
{
  QTV_NULL_PTR_CHECK(pOldRep, RETURN_VOID);
  QTV_NULL_PTR_CHECK(pNewRep, RETURN_VOID);

  if (pOldRep->GetSegmentTemplate() &&
    pNewRep->GetSegmentTemplate())
  {
    uint32 nOldStartNumber = pOldRep->GetSegmentTemplate()->GetStartNumber();
    uint64 nOldPTSOffset = pOldRep->GetPTSOffset();

    QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "AdjustSegmentKeys: Restore StartNumber and ptsOffset for repId %s", pNewRep->getRepIdentifier());
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "AdjustSegmentKeys: Restore StartNumber from %u to %u",
      pNewRep->GetSegmentTemplate()->GetStartNumber(),nOldStartNumber);
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "AdjustSegmentKeys: Restore ptsOffset from %llu and %llu",
      pNewRep->GetPTSOffset(), nOldPTSOffset);

    pNewRep->GetSegmentTemplate()->SetStartNumber(nOldStartNumber);
    pNewRep->SetPTSOffset((double)nOldPTSOffset / 1000.0);
  }
}


bool MPDParser::ParseGetNumberSegmentTimeline(TiXmlElement *Element,int& numSegmentTimeline)
{
  QTV_NULL_PTR_CHECK(Element,false);
  bool bOk = true;
  char elementSearchStr[100];


  TiXmlElement *SegmentTimelineElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentTimeline", m_pNamespaceKey);
    SegmentTimelineElement = Element->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentTimelineElement = Element->FirstChildElement("SegmentTimeline");
  }
  if (bOk && SegmentTimelineElement)
  {
    TiXmlElement *SElement = NULL;
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:S", m_pNamespaceKey);
      SElement = SegmentTimelineElement->FirstChildElement(elementSearchStr);
    }
    else
    {
      SElement = SegmentTimelineElement->FirstChildElement("S");
    }
    while (bOk && SElement)
    {
      numSegmentTimeline++;
      elementSearchStr[0] = '\0';
      if (m_pNamespaceKey)
      {
        std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                          "%s:S", m_pNamespaceKey);
        SElement = SElement->NextSiblingElement(elementSearchStr);
      }
      else
      {
        SElement = SElement->NextSiblingElement("S");
      }
    }
  }
  return bOk;
}


/* This function will parse the Segment List info of the MPD
 * and store in the segmentinfo.
 * @param[in] - Element - xml
 * @param[out] - number of segment time line
 * @param[out] - segmentTimeline structure
 * @return - bool
 * true on successful retrieval false otherwise
 */


bool MPDParser::ParseSegmentTimeline(TiXmlElement* Element, int& numSegmentTimeline,
                                        SegmentTimelineStuct *segmentTimeline)
{
  QTV_NULL_PTR_CHECK(Element,false);
  bool bOk = true;
  char elementSearchStr[100];


  TiXmlElement *SegmentTimelineElement = NULL;


  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentTimeline", m_pNamespaceKey);
    SegmentTimelineElement = Element->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentTimelineElement = Element->FirstChildElement("SegmentTimeline");
  }
  if (bOk && SegmentTimelineElement)
  {
    TiXmlElement *SElement = NULL;
        int timelineIndex = 0;
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:S", m_pNamespaceKey);
      SElement = SegmentTimelineElement->FirstChildElement(elementSearchStr);
    }
    else
    {
      SElement = SegmentTimelineElement->FirstChildElement("S");
    }
    while (bOk && SElement)
    {
      TiXmlAttribute* attrib = SElement->FirstAttribute();
      segmentTimeline[timelineIndex].starTime = 0;
      segmentTimeline[timelineIndex].duration = 0;
      segmentTimeline[timelineIndex].repeatCount = 0;
      bool bDurationPresent = false;
      while(attrib)
      {

        char *attrib_name = (char *)skip_whitespace(attrib->Name());
        if (attrib_name)
        {
          if(!std_strnicmp(attrib_name,"t",std_strlen("t")))
          {
            const char *end_ptr = NULL;
            int nErr = 0;
            segmentTimeline[timelineIndex].starTime = std_scanul(attrib->Value(),0,&end_ptr,&nErr);
          }
          if(!std_strnicmp(attrib_name,"d",std_strlen("d")))
          {
            bDurationPresent = true;
            if (atoi(attrib->Value()) == 0)
            {
              timelineIndex--;
              numSegmentTimeline--;
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Segment duration specified as zero");
              bOk = false;
              break;
            }
            const char *end_ptr = NULL;
            int retCode = 0;
            segmentTimeline[timelineIndex].duration = std_scanul(attrib->Value(),0,
                                                          &end_ptr, &retCode);
          }
          if(!std_strnicmp(attrib_name,"r",std_strlen("r")))
          {
            segmentTimeline[timelineIndex].repeatCount = atoi(attrib->Value());
          }
        }
        attrib = attrib->Next();
      }

      if(false == bOk)
      {
        break;
      }

      if (!bDurationPresent)
      {
         timelineIndex--;
         numSegmentTimeline--;
      }
      timelineIndex++;
      elementSearchStr[0] = '\0';
      if (m_pNamespaceKey)
      {
        std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                          "%s:S", m_pNamespaceKey);
        SElement = SElement->NextSiblingElement(elementSearchStr);
      }
      else
      {
        SElement = SElement->NextSiblingElement("S");
      }
    }
  }

  return bOk;

}


/* This function will parse the multiple Segment Base attribute and element info of the MPD
 * and store in the segmentinfo.
 * @param[in] - Element - xml
 * @param[out] - segment duration
 * @param[out] - segment start time
 * @return - bool
 * true on successful retrieval false otherwise
 */
bool MPDParser::ParseMultipleSegmentBaseInfo(TiXmlElement* Element, uint32& segmentDuration,
                                             uint32& startNumber)
{
  QTV_NULL_PTR_CHECK(Element,false);
  bool bOk = true;

  TiXmlAttribute* attrib = Element->FirstAttribute();
  while(attrib)
  {
    char *attrib_name = (char*)skip_whitespace(attrib->Name());
    if (attrib_name)
    {
      //MultipleSegmentBaseInformation
      if(!std_strnicmp(attrib_name,"duration",std_strlen("duration")))
      {
        const char *end_ptr = NULL;
        int retCode = 0;
        segmentDuration = std_scanul(attrib->Value(),0,&end_ptr,&retCode);

      }
      //MultipleSegmentBaseInformation
      if(!std_strnicmp(attrib_name,"startNumber",std_strlen("startNumber")))
      {
        startNumber = atoi(attrib->Value());
      }
    }
    attrib=attrib->Next();
  }
  return bOk;

}
/* This function will parse the Segment Base attribute and element info of the MPD
 * and store in the segmentinfo.
 * @param[in] - Element - xml
 * @param[out] - time scale
 * @param[out] - presentation time offset
 * @param[out] - index range
 * @param[out] - index range exact
 * @param[out] - intialisation
 * @param[out] - representation Index
 * @return - bool
 * true on successful retrieval false otherwise
 */


bool MPDParser::ParseSegmentBaseInfo(TiXmlElement* Element, uint32& timeScale,
                       uint64& presentationTimeOffset, char** indexRange,
                       bool&  indexRangeExact, URLType **initialisation,
                       URLType **representationIndex)
{
  QTV_NULL_PTR_CHECK(Element,false);
  bool bOk = true;
  char elementSearchStr[100];
  if (bOk)
  {
    TiXmlAttribute* attrib = Element->FirstAttribute();
    while (attrib)
    {
      char *attrib_name = (char*)skip_whitespace(attrib->Name());
      if (attrib_name)
      {
        if(!std_strnicmp(attrib_name,"timeScale",std_strlen("timeScale")))
        {
          int retcode = 0;
          const char *end_ptr = NULL;
          timeScale = std_scanul(attrib->Value(),0,&end_ptr, &retcode);
        }
        if(!std_strnicmp(attrib_name,"presentationTimeOffset",std_strlen("presentationTimeOffset")))
        {
          int retcode = 0;
          const char *end_ptr = NULL;
          presentationTimeOffset = std_scanull(attrib->Value(), 0, &end_ptr, &retcode);
          if (retcode == STD_NODIGITS ||
          retcode == STD_NEGATIVE ||
          retcode == STD_OVERFLOW ||
          retcode == STD_BADPARAM)
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "invalid presentationTimeOffset");
          }
        }
        if(!std_stricmp(attrib_name,"indexRange"))
        {
          *indexRange = (char *)QTV_Malloc(std_strlen((char*)attrib->Value())+1);
          if (*indexRange)
          {
            std_strlcpy(*indexRange, (char*)attrib->Value(), std_strlen((char*)attrib->Value())+1);
          }
        }
        if(!std_stricmp(attrib_name,"indexRangeExact"))
        {
          if(std_strnicmp(attrib->Value(),"true",4) == 0)
          {
            indexRangeExact = true;
          }
        }
        attrib=attrib->Next();
      }
    }
    char *initSourceURL = NULL;
    char *initRange = NULL;
    TiXmlElement* InitInfoElement = NULL;
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:Initialization", m_pNamespaceKey);
      InitInfoElement = Element->FirstChildElement(elementSearchStr);
      if (!InitInfoElement)
      {
        elementSearchStr[0] = '\0';
        std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:Initialisation", m_pNamespaceKey);
        InitInfoElement = Element->FirstChildElement(elementSearchStr);
      }
    }
    else
    {
      InitInfoElement = Element->FirstChildElement("Initialization");
      if (!InitInfoElement)
      {
        InitInfoElement = Element->FirstChildElement("Initialisation");
      }
    }
    if ( bOk && InitInfoElement)
    {
      TiXmlAttribute* attrib = InitInfoElement->FirstAttribute();
      while (attrib)
      {
        char *attrib_name = (char *)skip_whitespace(attrib->Name());
        if(!std_strnicmp(attrib_name,"sourceURL",std_strlen("sourceURL")))
        {
          initSourceURL = (char*)attrib->Value();
        }
        if(!std_strnicmp(attrib_name,"range",std_strlen("range")))
        {
          initRange = (char*)attrib->Value();
        }
        attrib = attrib->Next();
      }
      if (initSourceURL || initRange)
      {
        *initialisation = (URLType *)QTV_Malloc(sizeof(URLType));
        if (*initialisation)
        {
          (*initialisation)->sourceURL = initSourceURL;
          (*initialisation)->range     = initRange;
        }
        else
        {
          bOk = false;
        }
      }
    }

    char *repIndexSourceURL = NULL;
    char *repIndexRange = NULL;
    TiXmlElement* RepIndexInfoElement = NULL;
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:RepresentationIndex", m_pNamespaceKey);
      RepIndexInfoElement = Element->FirstChildElement(elementSearchStr);
    }
    else
    {
      RepIndexInfoElement = Element->FirstChildElement("RepresentationIndex");
    }
    if ( bOk && RepIndexInfoElement)
    {
      TiXmlAttribute* attrib = RepIndexInfoElement->FirstAttribute();
      while (attrib)
      {
        char *attrib_name = (char *)skip_whitespace(attrib->Name());
        if(!std_strnicmp(attrib_name,"sourceURL",std_strlen("sourceURL")))
        {
          repIndexSourceURL = (char*)attrib->Value();
        }
        if(!std_strnicmp(attrib_name,"range",std_strlen("range")))
        {
          repIndexRange = (char*)attrib->Value();
        }
        attrib = attrib->Next();
      }
      if (repIndexSourceURL || repIndexRange)
      {
        *representationIndex = (URLType *)QTV_Malloc(sizeof(URLType));
        if (*representationIndex)
        {
          (*representationIndex)->sourceURL = repIndexSourceURL;
          (*representationIndex)->range     = repIndexRange;

        }
        else
        {
          bOk = false;
        }
      }
    }
  }
  return bOk;

}

/* This function will parse the Segment base info of the MPD
 * and store in the segmentinfo.
 * @param[in] - Element - xml
 * @param[in] - Key - key for the element
 * @param[in] - Element type MPD_PERIOD, MPD_ADAPTATIONSET,MPD_REPRESENTATION
 * @return - bool
 * true on successful retrieval false otherwise
 */

bool MPDParser::ParseSegmentBase(MPD *pMpd, TiXmlElement* SegementBaseElement, uint64 key, ElementType elementType)
{
  QTV_NULL_PTR_CHECK(SegementBaseElement,false);
  bool bOk = false;
  int numPeriods = 0;
  PeriodInfo *pPeriodInfo = NULL;
  RepresentationGroup* repGrps = NULL;
  RepresentationInfo* rep = NULL;
  //SegementBase Attributes and elements
  uint32 timeScale = MAX_UINT32_VAL;
  uint64 presentationTimeOffset = 0;
  char *indexRange = NULL;
  bool indexRangeExact = false;
  URLType *initialisation = NULL;
  URLType *representationIndex = NULL;


  (void)ParseSegmentBaseInfo(SegementBaseElement, timeScale,
                             presentationTimeOffset, &indexRange,
                             indexRangeExact, &initialisation,
                             &representationIndex);

  if (elementType == MPD_PERIOD)
  {
    PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);

    if(pPeriodInfo)
    {
      int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);

      bOk = pPeriodInfo[PeriodIndex].InitialiseSegmentBase();
      if (bOk)
      {
        pPeriodInfo[PeriodIndex].SetSegmentBaseInfo(timeScale, presentationTimeOffset,
                                                    indexRange, indexRangeExact,
                                                    initialisation, representationIndex);
      }
    }
  }
  else if (elementType == MPD_ADAPTATIONSET)
  {
    PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
    int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
    int GroupIndex =(int)((key & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
    if(pPeriodInfo)
    {
      int numRepGrps = 0;
      repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
      if(repGrps)
      {
        bOk = true;
      }
    }
    if (bOk)
    {
      bOk = repGrps[GroupIndex].InitialiseSegmentBase();
      if (bOk)
      {
        repGrps[GroupIndex].SetSegmentBaseInfo(timeScale, presentationTimeOffset,
                                               indexRange,indexRangeExact,
                                               initialisation, representationIndex);
      }
    }
  }
  else if (elementType == MPD_REPRESENTATION)
  {
    PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
    int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
    int GroupIndex =(int)((key & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
    int RepIndex = (int)((key & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT);
    if(pPeriodInfo)
    {
      int numRepGrps = 0;
      repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
      if(repGrps)
      {
        int numReps = 0;
        rep=repGrps[GroupIndex].getRepInfo(numReps);
        if(rep)
        {
          bOk = true;
        }
        if (bOk)
        {
          bOk = rep[RepIndex].InitialiseSegmentBase();
          if (bOk)
          {
            rep[RepIndex].SetSegmentBaseInfo(timeScale, presentationTimeOffset,
                                                    indexRange,indexRangeExact,
                                                    initialisation, representationIndex);
          }
        }
      }
    }
  }

  if (initialisation)
  {
    QTV_Free(initialisation);
    initialisation = NULL;
  }

  if (representationIndex)
  {
    QTV_Free(representationIndex);
    representationIndex = NULL;
  }

  if (indexRange)
  {
    QTV_Free(indexRange);
    indexRange = NULL;
  }
  return bOk;
}

/* This function will parse the Segment List info of the MPD
 * and store in the segmentinfo.
 * @param[in] - Element - xml
 * @param[in] - Key - key for the element
 * @param[in] - Element type MPD_PERIOD, MPD_ADAPTATIONSET,MPD_REPRESENTATION
 * @return - bool
 * true on successful retrieval false otherwise
 */

bool MPDParser::ParseSegmentList(MPD *pMpd, TiXmlElement* SegmentListElement, char ** /*baseURLs*/, uint64 key, ElementType elementType)
{
  QTV_NULL_PTR_CHECK(SegmentListElement,false);
  bool bOk = true;
  int numPeriods = 0;
  PeriodInfo *pPeriodInfo = NULL;
  RepresentationGroup* repGrps = NULL;
  RepresentationInfo* rep = NULL;
  //SegmentList info
  char *mediaUrl = NULL;
  char *mediaRange = NULL;
  char *indexUrl = NULL;
  char *indexRange = NULL;
  int numSegmentURL = 0;
  SegmentURLStruct *segmentURL = NULL;
  //multiple segment base
  uint32 segmentDuration = 0;
  uint32 startNumber = MAX_UINT32_VAL;
  //SegementBase Attributes and elements
  uint32 timeScale = MAX_UINT32_VAL;
  uint64 presentationTimeOffset = 0;
  char *indexRangeBase = NULL;
  bool indexRangeExact = false;
  URLType *initialisation = NULL;
  URLType *representationIndex = NULL;
  //segmentTimeline info
  int numSegmentTimeline = 0;
  SegmentTimelineStuct *segmentTimeline = NULL;

  char elementSearchStr[100];

  bOk = ParseMultipleSegmentBaseInfo(SegmentListElement, segmentDuration, startNumber );

  if(bOk)
  {
    (void)ParseSegmentBaseInfo(SegmentListElement, timeScale,
                               presentationTimeOffset, &indexRangeBase,
                               indexRangeExact, &initialisation,
                               &representationIndex);

    (void)ParseGetNumberSegmentTimeline(SegmentListElement, numSegmentTimeline);

    if (numSegmentTimeline  > 0)
    {
      segmentTimeline = (SegmentTimelineStuct *)QTV_New_Array(SegmentTimelineStuct, numSegmentTimeline);

      if(segmentTimeline)
      {
        bOk = ParseSegmentTimeline(SegmentListElement, numSegmentTimeline, segmentTimeline);
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Failed to allocate segmentTimeline");
        bOk = false;
      }
    }
  }


  //parsing SegmentURL
  TiXmlElement *SegmentURLElement = NULL;
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentURL", m_pNamespaceKey);
    SegmentURLElement = SegmentListElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentURLElement = SegmentListElement->FirstChildElement("SegmentURL");
  }

  while (bOk && SegmentURLElement)
  {
    numSegmentURL++;
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:SegmentURL", m_pNamespaceKey);
      SegmentURLElement = SegmentURLElement->NextSiblingElement(elementSearchStr);
    }
    else
    {
      SegmentURLElement = SegmentURLElement->NextSiblingElement("SegmentURL");
    }
  }
  if (numSegmentURL > 0)
  {
    segmentURL = (SegmentURLStruct *)QTV_New_Array(SegmentURLStruct,(numSegmentURL));
  }
  elementSearchStr[0] = '\0';
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                      "%s:SegmentURL", m_pNamespaceKey);
    SegmentURLElement = SegmentListElement->FirstChildElement(elementSearchStr);
  }
  else
  {
    SegmentURLElement = SegmentListElement->FirstChildElement("SegmentURL");
  }
  int urlListIndex = 0;
  while (bOk && SegmentURLElement && segmentURL)
  {
    segmentURL[urlListIndex].mediaUrl   = NULL;
    segmentURL[urlListIndex].mediaRange = NULL;
    segmentURL[urlListIndex].indexUrl   = NULL;
    segmentURL[urlListIndex].indexRange = NULL;
    TiXmlAttribute* attrib = SegmentURLElement->FirstAttribute();
    while(attrib)
    {
      char *attrib_name = (char*)skip_whitespace(attrib->Name());
      if (attrib_name)
      {
        if(!std_strnicmp(attrib_name,"mediaRange",std_strlen("mediaRange")))
        {
          segmentURL[urlListIndex].mediaRange= (char *)attrib->Value();
        }
        else if(!std_strnicmp(attrib_name,"media",std_strlen("media")))
        {
          segmentURL[urlListIndex].mediaUrl = (char *)attrib->Value();
        }
          else if(!std_strnicmp(attrib_name,"indexRange",std_strlen("indexRange")))
        {
          segmentURL[urlListIndex].indexRange = (char *)attrib->Value();
        }
        else if(!std_strnicmp(attrib_name,"index",std_strlen("index")))
        {
          segmentURL[urlListIndex].indexUrl= (char *)attrib->Value();
        }
      }
      attrib = attrib->Next();
    }
    urlListIndex++;
    elementSearchStr[0] = '\0';
    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                        "%s:SegmentURL", m_pNamespaceKey);
      SegmentURLElement = SegmentURLElement->NextSiblingElement(elementSearchStr);
    }
    else
    {
      SegmentURLElement = SegmentURLElement->NextSiblingElement("SegmentURL");
    }
  }


  if(bOk)
  {
    bOk = false;
    if (elementType == MPD_PERIOD)
    {
      PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);

      if(pPeriodInfo)
      {
        int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
        bOk = pPeriodInfo[PeriodIndex].InitialiseSegmentList();
        if (bOk)
        {
          if (numSegmentURL > 0)
          {
            bOk = pPeriodInfo[PeriodIndex].InitialiseSegmentUrl(numSegmentURL);
            if (bOk)
            {
              for (int i = 0; i < numSegmentURL && segmentURL; i++)
              {
                pPeriodInfo[PeriodIndex].SetSegmentUrl(i, segmentURL[i].mediaUrl,
                                                         segmentURL[i].mediaRange,
                                                         segmentURL[i].indexUrl,
                                                         segmentURL[i].indexRange);
              }
            }
          }
          pPeriodInfo[PeriodIndex].SetMultiSegmentBaseInfo(segmentDuration, startNumber,
                                                           timeScale, presentationTimeOffset,
                                                           indexRangeBase, indexRangeExact,
                                                           initialisation, representationIndex);
          if (numSegmentTimeline > 0)
          {
            bOk = pPeriodInfo[PeriodIndex].InitializeSegmentTimeline(numSegmentTimeline);
            if (bOk)
            {
              for (int i = 0; i < numSegmentTimeline; i++)
              {
                pPeriodInfo[PeriodIndex].SetSegmentTimeline(i, segmentTimeline[i].starTime,
                                                             segmentTimeline[i].duration,
                                                             segmentTimeline[i].repeatCount);
              }
            }
          }
        }
      }
    }
    else if (elementType == MPD_ADAPTATIONSET)
    {
      PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
      int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
      int GroupIndex =(int)((key & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
      if(pPeriodInfo)
      {
        int numRepGrps = 0;
        repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
        if(repGrps)
        {
          bOk = true;
        }
      }
      if (bOk)
      {
        bOk = repGrps[GroupIndex].InitialiseSegmentList();
        if (bOk)
        {
          if (numSegmentURL > 0 && segmentURL)
          {
            bOk = repGrps[GroupIndex].InitialiseSegmentUrl(numSegmentURL);
            if (bOk)
            {
              for (int i = 0; i < numSegmentURL; i++)
              {
                repGrps[GroupIndex].SetSegmentUrl(i, segmentURL[i].mediaUrl,
                                                    segmentURL[i].mediaRange,
                                                    segmentURL[i].indexUrl,
                                                    segmentURL[i].indexRange);
              }
            }
          }
          repGrps[GroupIndex].SetMultiSegmentBaseInfo(segmentDuration, startNumber,
                                                      timeScale, presentationTimeOffset,
                                                      indexRangeBase, indexRangeExact,
                                                      initialisation, representationIndex);
          if (numSegmentTimeline > 0)
          {
            bOk = repGrps[GroupIndex].InitializeSegmentTimeline(numSegmentTimeline);
            if (bOk)
            {
              for (int i = 0; i < numSegmentTimeline; i++)
              {
                 repGrps[GroupIndex].SetSegmentTimeline(i, segmentTimeline[i].starTime,
                                                           segmentTimeline[i].duration,
                                                           segmentTimeline[i].repeatCount);
              }
            }
          }
        }
      }
    }
    else if (elementType == MPD_REPRESENTATION)
    {
      PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
      int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
      int GroupIndex =(int)((key & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
      int RepIndex = (int)((key & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT);
      if(pPeriodInfo)
      {
        int numRepGrps = 0;
        repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
        if(repGrps)
        {
          int numReps = 0;
          rep=repGrps[GroupIndex].getRepInfo(numReps);
          if(rep)
          {
            bOk = true;
          }
          bOk = rep[RepIndex].InitialiseSegmentList();
          if (bOk)
          {
            if (numSegmentURL > 0 && segmentURL)
            {
              bOk = rep[RepIndex].InitialiseSegmentUrl(numSegmentURL);
              if (bOk)
              {
                for (int i = 0; i < numSegmentURL; i++)
                {
                  rep[RepIndex].SetSegmentUrl(i, segmentURL[i].mediaUrl,
                                                  segmentURL[i].mediaRange,
                                                  segmentURL[i].indexUrl,
                                                  segmentURL[i].indexRange);
                }
              }
            }
            rep[RepIndex].SetMultiSegmentBaseInfo(segmentDuration, startNumber,
                                                  timeScale, presentationTimeOffset,
                                                  indexRangeBase, indexRangeExact,
                                                  initialisation, representationIndex);
            if (numSegmentTimeline > 0)
            {
              bOk = rep[RepIndex].InitializeSegmentTimeline(numSegmentTimeline);
              if (bOk)
              {
                for (int i = 0; i < numSegmentTimeline; i++)
                {
                  rep[RepIndex].SetSegmentTimeline(i, segmentTimeline[i].starTime,
                                                    segmentTimeline[i].duration,
                                                    segmentTimeline[i].repeatCount);
                }
              }
            }
          }
        }
      }
    }
  }

  if (segmentTimeline)
  {
    QTV_Delete_Array(segmentTimeline);
    segmentTimeline = NULL;
  }

  if (segmentURL)
  {
    QTV_Delete_Array(segmentURL);
    segmentURL = NULL;
  }
  if (initialisation)
  {
    QTV_Free(initialisation);
    initialisation = NULL;
  }
  if (representationIndex)
  {
    QTV_Free(representationIndex);
    representationIndex = NULL;
  }
  if (indexRangeBase)
  {
    QTV_Free(indexRangeBase);
    indexRangeBase = NULL;
  }
  return bOk;
}


/* This function will parse the Segment Template info of the MPD
 * and store in the segmentinfo.
 * @param[in] - Element - xml
 * @param[in] - Key - key for the element
 * @param[in] - Element type MPD_PERIOD, MPD_ADAPTATIONSET,MPD_REPRESENTATION
 * @return - bool
 * true on successful retrieval false otherwise
 */

bool MPDParser::ParseSegmentTemplate(MPD *pMpd, TiXmlElement* SegmentTemplateElement, uint64 key, ElementType elementType)
{
  QTV_NULL_PTR_CHECK(SegmentTemplateElement,false);

  bool bOk = true;
  int numPeriods = 0;
  PeriodInfo *pPeriodInfo = NULL;
  RepresentationGroup* repGrps = NULL;
  RepresentationInfo* rep = NULL;
  //SegmentTemplate info
  char *mediaTemplate = NULL;
  char *indexTemplate = NULL;
  char *initialisationTemplate = NULL;
  char *bitstreamSwitchingTemplate = NULL;
  //multiple segment base
  uint32 segmentDuration = 0;
  uint32 startNumber = MAX_UINT32_VAL;
  //SegementBase Attributes and elements
  uint32 timeScale = MAX_UINT32_VAL;
  uint64 presentationTimeOffset = 0;
  char *indexRange = NULL;
  bool indexRangeExact = false;
  URLType *initialisation = NULL;
  URLType *representationIndex = NULL;
  //segmentTimeline info
  int numSegmentTimeline = 0;
  SegmentTimelineStuct *segmentTimeline = NULL;

  bOk = ParseMultipleSegmentBaseInfo(SegmentTemplateElement, segmentDuration, startNumber );

  if(bOk)
  {
    (void)ParseSegmentBaseInfo(SegmentTemplateElement, timeScale,
                               presentationTimeOffset, &indexRange,
                               indexRangeExact, &initialisation,
                               &representationIndex);
    (void)ParseGetNumberSegmentTimeline(SegmentTemplateElement, numSegmentTimeline);
    if (numSegmentTimeline > 0)
    {
      segmentTimeline = (SegmentTimelineStuct *)QTV_New_Array(SegmentTimelineStuct, numSegmentTimeline);

      if(segmentTimeline)
      {
        bOk = ParseSegmentTimeline(SegmentTemplateElement, numSegmentTimeline, segmentTimeline);
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Failed to allocate segmentTimeline");
        bOk = false;
      }
    }
  }

  //parsing segment template
  if (bOk)
  {
    TiXmlAttribute* attrib = SegmentTemplateElement->FirstAttribute();
    while(attrib)
    {
      char *attrib_name = (char*)skip_whitespace(attrib->Name());
      if (attrib_name)
      {
        //SegmentTemplate
        if(!std_strnicmp(attrib_name,"media",std_strlen("media")))
        {
          mediaTemplate = (char*)attrib->Value();
        }
        //SegmentTemplate
        if(!std_strnicmp(attrib_name,"index",std_strlen("index")) && attrib_name[std_strlen("index")] == '\0' )
        {
          indexTemplate = (char*)attrib->Value();
        }
        //SegmentTemplate
        if(!std_strnicmp(attrib_name,"initialization",std_strlen("initialization")))
        {
          initialisationTemplate = (char*)attrib->Value();
        }
        if(!std_strnicmp(attrib_name,"initialisation",std_strlen("initialisation")))
        {
          initialisationTemplate = (char*)attrib->Value();
        }
        //SegmentTemplate
        if(!std_strnicmp(attrib_name,"bitstreamSwitching",std_strlen("bitstreamSwitching")))
        {
          bitstreamSwitchingTemplate = (char*)attrib->Value();
        }
        attrib=attrib->Next();
      }
    }
  }


 if(bOk)
 {
    bOk = false;
    if (elementType == MPD_PERIOD)
    {
      PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
      if(pPeriodInfo)
      {
        int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);

        bOk = pPeriodInfo[PeriodIndex].InitialiseSegmentTemplate();
        if (bOk)
        {
          pPeriodInfo[PeriodIndex].SetMultiSegmentBaseInfo(segmentDuration, startNumber,
                                                           timeScale, presentationTimeOffset,
                                                           indexRange, indexRangeExact,
                                                           initialisation, representationIndex);
          if (numSegmentTimeline > 0 )
          {
            bOk = pPeriodInfo[PeriodIndex].InitializeSegmentTimeline(numSegmentTimeline);
            if (bOk)
            {
               for (int i = 0; i < numSegmentTimeline; i++)
               {
                 pPeriodInfo[PeriodIndex].SetSegmentTimeline(i, segmentTimeline[i].starTime,
                                                                segmentTimeline[i].duration,
                                                                segmentTimeline[i].repeatCount);
               }
            }
          }
          pPeriodInfo[PeriodIndex].SetSegmentTemplateInfo(mediaTemplate,indexTemplate,
                                                    initialisationTemplate,
                                                    bitstreamSwitchingTemplate);
        }
      }
    }
    else if (elementType == MPD_ADAPTATIONSET)
    {
      PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
      int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
      int GroupIndex =(int)((key & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
      if(pPeriodInfo)
      {
        int numRepGrps = 0;
        repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
        if(repGrps)
        {
          bOk = true;
        }
      }
      if (bOk)
      {
        bOk = repGrps[GroupIndex].InitialiseSegmentTemplate();
        if (bOk)
        {
          repGrps[GroupIndex].SetMultiSegmentBaseInfo(segmentDuration, startNumber,
                                                      timeScale, presentationTimeOffset,
                                                      indexRange, indexRangeExact,
                                                      initialisation, representationIndex);
          if (numSegmentTimeline > 0)
          {
            bOk = repGrps[GroupIndex].InitializeSegmentTimeline(numSegmentTimeline);
            if (bOk)
            {
               for (int i = 0; i < numSegmentTimeline; i++)
               {
                             repGrps[GroupIndex].SetSegmentTimeline(i, segmentTimeline[i].starTime,
                                                                segmentTimeline[i].duration,
                                                                segmentTimeline[i].repeatCount);
               }
            }
          }
          repGrps[GroupIndex].SetSegmentTemplateInfo(mediaTemplate,indexTemplate,
                                                  initialisationTemplate,
                                                  bitstreamSwitchingTemplate);
        }
      }
    }
    else if (elementType == MPD_REPRESENTATION)
    {
      PeriodInfo *pPeriodInfo = pMpd->getPeriodInfo(numPeriods);
      int PeriodIndex = (int)((key & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
      int GroupIndex =(int)((key & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
      int RepIndex = (int)((key & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT);
      if(pPeriodInfo)
      {
        int numRepGrps = 0;
        repGrps = pPeriodInfo[PeriodIndex].getRepGrpInfo(numRepGrps);
        if(repGrps)
        {
          int numReps = 0;
          rep=repGrps[GroupIndex].getRepInfo(numReps);
          if(rep)
          {
            bOk = true;
          }
          if (bOk)
          {
            bOk = rep[RepIndex].InitialiseSegmentTemplate();
            if (bOk)
            {
              rep[RepIndex].SetMultiSegmentBaseInfo(segmentDuration, startNumber,
                                                    timeScale, presentationTimeOffset,
                                                    indexRange, indexRangeExact,
                                                    initialisation, representationIndex);
              if (numSegmentTimeline > 0)
              {
                bOk = rep[RepIndex].InitializeSegmentTimeline(numSegmentTimeline);
                if (bOk)
                {
                   for (int i = 0; i < numSegmentTimeline; i++)
                   {
                                     rep[RepIndex].SetSegmentTimeline(i, segmentTimeline[i].starTime,
                                                         segmentTimeline[i].duration,
                                                         segmentTimeline[i].repeatCount);
                   }
                }
              }
              rep[RepIndex].SetSegmentTemplateInfo(mediaTemplate,indexTemplate,
                                                  initialisationTemplate,
                                                  bitstreamSwitchingTemplate);
            }
          }
        }
      }
    }
 }

  if (segmentTimeline)
  {
    QTV_Delete_Array(segmentTimeline);
    segmentTimeline = NULL;
  }
  if (initialisation)
  {
    QTV_Free(initialisation);
    initialisation = NULL;
  }
  if (representationIndex)
  {
    QTV_Free(representationIndex);
    representationIndex = NULL;
  }
  if (indexRange)
  {
    QTV_Free(indexRange);
    indexRange = NULL;
  }
  return bOk;
}

/**
 * check is next period present.
 * return next period starttime and duration if next period present
 */

bool MPDParser::IsNextPeriodPresent(TiXmlElement* Element, double &nextPeriodStartTime, double &nextPeriodDuration)
{
  bool bOk = false;
  char elementSearchStr[100];
  TiXmlElement* siblingElement = NULL;
  char *periodDurationChar = NULL;
  char *periodStartChar = NULL;
  nextPeriodDuration = 0;
  nextPeriodStartTime = 0.0;
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                    "%s:Period", m_pNamespaceKey);
    siblingElement = Element->NextSiblingElement(elementSearchStr);
  }
  else
  {
    siblingElement = Element->NextSiblingElement("Period");
  }
  if (siblingElement) // next period is present
  {
    bOk = true;
    TiXmlAttribute* attrib = siblingElement->FirstAttribute();
    while(attrib)
    {
      char *attrib_name = (char*)skip_whitespace(attrib->Name());
      if(!std_strnicmp(attrib_name,"duration",std_strlen("duration")))
      {
        periodDurationChar = (char*)attrib->Value();
        if(periodDurationChar)
        {
          ConvertDurationTypeToSeconds(periodDurationChar,nextPeriodDuration);
        }
      }
      else if (!std_strnicmp(attrib_name,"start",std_strlen("start")))
      {
        periodStartChar = (char*)attrib->Value();
        if (periodStartChar)
        {
          ConvertDurationTypeToSeconds(periodStartChar,nextPeriodStartTime);
        }
      }
      attrib=attrib->Next();
    }
  }
  else
  {
    bOk = false;
  }
  return bOk;
}


/**
 * check if next Representation present.
 */

bool  MPDParser::IsNextRepresentationPresent(TiXmlElement* Element)
{
  bool bOk = false;
  char elementSearchStr[100];
  TiXmlElement* siblingElement = NULL;
  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                    "%s:Representation", m_pNamespaceKey);
    siblingElement = Element->NextSiblingElement(elementSearchStr);
  }
  else
  {
    siblingElement = Element->NextSiblingElement("Representation");
  }
  if (siblingElement) // next rep is present
  {
    bOk = true;
  }
  return bOk;
}


bool MPDParser::IsMPDAvailable() const
{
  return mpdAvailable;
}

bool MPDParser::IsMPDValid() const
{
  return m_bIsMpdValid;
}

/**
 * Reset the playlist parser to start at the first period in the
 * in the current playlist.
 */
void MPDParser::ResetCurrentPeriod()
{
  current_period_key = 0;

  PeriodInfo *periodInfo = NULL;
  int numPeriods = 0;

  PeriodInfo *periodInfoArray = mpd->getPeriodInfo(numPeriods);

  if (periodInfoArray && numPeriods > 0)
  {
    current_period_key = periodInfoArray[0].getPeriodKey();
  }
}

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
 *                    exceed the value of this adjusted for
 *                    availability time.
 */
void MPDParser::UpdateNumSegmentsAndSegmentIndex(
  int& numSegment, int& segmentIndex,
  int startNumber, const MPD& rMpd, const PeriodInfo& rPeriodInfo, RepresentationInfo& rRepInfo, double segDuration,
  int maxSegments)
{
  bool bIsMaxSegmentsSet = (maxSegments > 0 ? true : false);
  double offsetFromAvailTime = 0.0;
  numSegment = 0;

  if (rMpd.GetAvailabilityStartTime() > 0.0 && rMpd.IsLive())
  {
    offsetFromAvailTime = GetOffsetFromAvailabilityTimeForPeriod(rMpd,
      rPeriodInfo, rRepInfo.GetTimeShiftBufferDepth());

    int incSegments = (int)((offsetFromAvailTime-segDuration /*First segment is available only after availabilityStartTime + periodStart + segDuration*/)/ segDuration);

    incSegments = incSegments < 0 ? 0 : incSegments;
    segmentIndex += incSegments;

    if (maxSegments > 0)
    {
      maxSegments = (maxSegments > incSegments ? maxSegments - incSegments : 0);
    }
  }

  if (bIsMaxSegmentsSet && 0 == maxSegments)
  {
    // segment list case
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Prevent reading from segment list");
  }
  else
  {
    double firstAvailSegmentStartTime = ((double)(segmentIndex - startNumber))*segDuration;
    double periodDuration = 1000.0 * rPeriodInfo.getDuration();

    if (periodDuration > 0.0 && periodDuration > firstAvailSegmentStartTime)
    {
      periodDuration -= firstAvailSegmentStartTime;
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "ReCalculated period duration = %d", (int)periodDuration);
    }

    double relevantPeriodDuration = periodDuration;

    if (rRepInfo.GetMinUpdatePeriod() > 0.0)
    {
      relevantPeriodDuration =
        rRepInfo.GetMinUpdatePeriod() * 1000 +
        1000 * rRepInfo.GetTimeShiftBufferDepth() +
        2000; // adding some delta.

      if (periodDuration > 0.0)
      {
        relevantPeriodDuration = QTV_MIN(relevantPeriodDuration, periodDuration);
      }

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "UpdateNumSegmentsAndSegmentIndex: periodWindow set as %d",
                    (int)relevantPeriodDuration);
    }

    numSegment = (int)ceil(relevantPeriodDuration / segDuration);
    if (maxSegments > 0 && numSegment >= maxSegments)
    {
      numSegment = maxSegments;
    }
  }
}

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

void MPDParser::GetNumAvailableSegment(
  int& numSegment, const MPD& rMpd, const PeriodInfo& rPeriodInfo,
   RepresentationInfo& rRepInfo, uint32 timeScale, int maxSegments, int &startNumber, bool bSegmentTemplate)
{
  bool bIsMaxSegmentsSet = (maxSegments > 0 ? true : false);
  double offsetFromAvailTime = 0.0;
  numSegment = 0;
  int repeatIndex = 0;
  uint32 tempTimelineIndex = 0;
  double replaceSegStartTime = 0.0;
  double replaceSegDuration = 0.0;
  double segmentStartTime = 0.0;
  double presentationTimeOffset = 0.0;
  double segDuration = 0.0;
  bool bPeriodDurationSet = false;
  int numTimelineEntry = rRepInfo.GetNumSegmentTimelineEntry();
  int repeatCount = 0;

  if (rMpd.GetAvailabilityStartTime() > 0.0 && rMpd.IsLive())
  {
    offsetFromAvailTime = GetOffsetFromAvailabilityTimeForPeriod(rMpd,
      rPeriodInfo, rRepInfo.GetTimeShiftBufferDepth());
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
             "offsetFromAvailTime = %d", (int)offsetFromAvailTime);
  }
  double periodDuration = 1000.0 * rPeriodInfo.getDuration();
  double relevantPeriodDuration = periodDuration;
  double minUpdatePeriod = rRepInfo.GetMinUpdatePeriod();

  double lastSegmentDurationInPeriod = 0;
  if(numTimelineEntry > 0)
  {
    if(bSegmentTemplate)
    {
      lastSegmentDurationInPeriod = (double)rRepInfo.GetSegmentTemplate()->GetSegTimeLineDuration(numTimelineEntry-1);
    }
    else
    {
      lastSegmentDurationInPeriod = (double)rRepInfo.GetSegmentList()->GetSegTimeLineDuration(numTimelineEntry-1);
    }
    lastSegmentDurationInPeriod = (lastSegmentDurationInPeriod/(double)timeScale)*1000;
  }

  if (minUpdatePeriod > 0.0)
  {
    relevantPeriodDuration =
      minUpdatePeriod * 1000 +
      1000 * rRepInfo.GetTimeShiftBufferDepth() +
      2000; // adding some delta.

    if (periodDuration > 0.0)
    {
      relevantPeriodDuration = QTV_MIN(relevantPeriodDuration, periodDuration);
    }

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "periodWindow set as %d", (int)relevantPeriodDuration);
  }

  while(tempTimelineIndex < (uint32)numTimelineEntry)
  {
    if (repeatIndex == 0)
    {
      if (bSegmentTemplate)
      {
        if (rRepInfo.GetSegmentTemplate()->GetSegTimeLineStartTime(tempTimelineIndex) > replaceSegStartTime)
        {
          replaceSegStartTime = (double)rRepInfo.GetSegmentTemplate()->GetSegTimeLineStartTime(tempTimelineIndex);
          presentationTimeOffset = (double)rRepInfo.GetSegmentTemplate()->GetPresentationOffset();
          segmentStartTime = replaceSegStartTime - presentationTimeOffset;
          segmentStartTime = (segmentStartTime/(double)timeScale)*1000;
        }
        replaceSegDuration = (double)rRepInfo.GetSegmentTemplate()->GetSegTimeLineDuration(tempTimelineIndex);
      }
      else
      {
        if (rRepInfo.GetSegmentList()->GetSegTimeLineStartTime(tempTimelineIndex) > replaceSegStartTime)
        {
          replaceSegStartTime = (double)rRepInfo.GetSegmentList()->GetSegTimeLineStartTime(tempTimelineIndex);
          presentationTimeOffset = (double)rRepInfo.GetSegmentList()->GetPresentationOffset();
          segmentStartTime = replaceSegStartTime - presentationTimeOffset;
          segmentStartTime = (segmentStartTime/(double)timeScale)*1000;
        }
        replaceSegDuration = (double)rRepInfo.GetSegmentList()->GetSegTimeLineDuration(tempTimelineIndex);
      }
      segDuration = (replaceSegDuration/(double)timeScale)*1000;
    }

    if ((segmentStartTime + 2*segDuration) > (offsetFromAvailTime)  &&
      (segmentStartTime + segDuration) < (relevantPeriodDuration +
                                         lastSegmentDurationInPeriod))
      /*This factors in segDuration additionally in segment availability time calculations*/
    {

       numSegment++;
       if (maxSegments > 0 && numSegment >= maxSegments)
       {
         break;
       }
    }

    repeatIndex++;
    if (bSegmentTemplate)
    {
      if (repeatIndex ==
         rRepInfo.GetSegmentTemplate()->GetSegTimeLineRepeatCount(tempTimelineIndex) + 1)
      {
        if (startNumber > repeatIndex)
        {
          startNumber -= repeatIndex;
           QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
             "returning startNumber = %d", (int)startNumber);
        }
        repeatIndex = 0;
        tempTimelineIndex++;
      }
    }
    else
    {
      if (repeatIndex ==
         rRepInfo.GetSegmentList()->GetSegTimeLineRepeatCount(tempTimelineIndex) + 1)
      {
        if (startNumber > repeatIndex)
        {
          startNumber -= repeatIndex;
           QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
             "returning startNumber = %d", (int)startNumber);
        }
        repeatIndex = 0;
        tempTimelineIndex++;
      }
    }
    segmentStartTime = segmentStartTime + segDuration;
    replaceSegStartTime = replaceSegStartTime + (double)replaceSegDuration;
  }
}

/**
 * Delegate some initial checks for GetAllSegmentsForRepresentationRange
 * to this function.
 */
HTTPDownloadStatus MPDParser::GetAllSegmentsForRepresentationRangePreProcess(
PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, uint64 segStartTime, double currMSeconds)
{
  QTV_NULL_PTR_CHECK(mpd, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;

  double availabilityMSeconds = mpd->GetAvailabilityStartTime();
  double mpdDuration = mpd->getDuration() * 1000.0;

  if (false == m_bIsMpdValid)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Data end as mpd is no longer valid");
    status = video::HTTPCommon::HTTPDL_DATA_END;
  }
  else if(m_bAbortSet)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "MPD parser task aborted");
    status = video::HTTPCommon::HTTPDL_DATA_END;
  }
  else if(!mpdAvailable)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "Waiting to get the mpd information from server");
    status = HTTPCommon::HTTPDL_WAITING;
  }

  if(HTTPCommon::HTTPDL_SUCCESS == status)
  {
    //If the MPD is not yet available then it is a complete waiting.
    if(currMSeconds < availabilityMSeconds)
    {
      int tmpDiff = (int)((availabilityMSeconds - currMSeconds)/1000.0);
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Mpd is not available yet. currMSeconds %llu < availabilityMSeconds %llu. Ahead by %d seconds",
                     (uint64)currMSeconds, (uint64)availabilityMSeconds, tmpDiff);
      status = HTTPCommon::HTTPDL_WAITING;
    }
  }

  if(HTTPCommon::HTTPDL_SUCCESS == status)
  {
    int numPeriods = 0;
    PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);

    uint64 periodStartTime = pPeriodInfo->getStartTime();

    if(period_info && numPeriods > 0)
    {
      uint32 nMinPeriodKey = (uint32)((uint64)(period_info[0].getPeriodKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
      uint32 nRequestedPeriodKey = (uint32)((uint64)(pPeriodInfo->getPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);

      if (nMinPeriodKey > nRequestedPeriodKey)
      {
        // this should be when the mpd refresh occured so long after the
        // previous refresh that the periodKey assosicated with request
        // is no longer relatable in the latest mpd refresh.
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "HTTPDL_DATA_END as nMinPeriodKey %u > nRequestedPeriodKey %u",
                      nMinPeriodKey, nRequestedPeriodKey);
        status = video::HTTPCommon::HTTPDL_DATA_END;
      }
      else
      {
        uint64 absSegStartTime = periodStartTime + segStartTime;

        if (pPeriodInfo->getDuration() > 0.0)
        {
          uint64 periodEndTime = periodStartTime + (uint64)(1000 * pPeriodInfo->getDuration());
          if (absSegStartTime >= periodEndTime)
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "eos reached on period %u", nRequestedPeriodKey);

            status = video::HTTPCommon::HTTPDL_DATA_END;
          }
        }
      }

      if(HTTPCommon::HTTPDL_SUCCESS == status)
      {
        if ((availabilityMSeconds > 0.0 && pPeriodInfo->getDuration() > 0) ||
            (IsLive() && (mpd) && mpd->getMinimumUpdatePeriod()))
        {
          uint64 nLastSegKey = MAX_UINT64_VAL;
          uint64 nLastsegDuration = 0;

          if(pRepInfo->GetSegmentFunc())
          {
            status = pRepInfo->GetSegmentFunc()->GetLastSegmentKeyForRepresentation(this, pPeriodInfo, &nLastSegKey, pRepInfo);

            if(status == HTTPCommon::HTTPDL_SUCCESS)
            {
              status = pRepInfo->GetSegmentFunc()->GetSegDurationForRepresentation(this, pRepInfo, nLastSegKey, nLastsegDuration);

              if(status == HTTPCommon::HTTPDL_SUCCESS)
        {
          // If the current time has gone past the end of the period-end,
          // then set status as end of stream. The segmentinfo array will
          // not be created in this case.
                double absPeriodAvailEndTime = 0.0;
                if(pPeriodInfo->getDuration() > 0.0)
                {
                  absPeriodAvailEndTime = availabilityMSeconds +
                                          (double)periodStartTime + (double)nLastsegDuration +
                  1000 * pPeriodInfo->getDuration();
                }
                else if (IsLive() &&  mpd->getMinimumUpdatePeriod() > 0.0)
                {
                  MM_Time_DateTime fetchTime = GetFetchTime();
                  absPeriodAvailEndTime = (StreamSourceTimeUtils::ConvertSysTimeToMSec(fetchTime) +
                              1000.0 * mpd->getMinimumUpdatePeriod() + (double)nLastsegDuration);
                }
                double offsetOfPeriodEndFromCurrentTime =
                  ((currMSeconds - pRepInfo->GetTimeShiftBufferDepth()*1000) >= absPeriodAvailEndTime
                  ? (currMSeconds - pRepInfo->GetTimeShiftBufferDepth()*1000 - absPeriodAvailEndTime)
             : 0.0);

          if (offsetOfPeriodEndFromCurrentTime > 0.0 && IsLive() )
          {
            QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "eos reached on period %u, peiodDuration %f, past period end by %f msecs",
                    nRequestedPeriodKey, pPeriodInfo->getDuration(), offsetOfPeriodEndFromCurrentTime);

            status = video::HTTPCommon::HTTPDL_DATA_END;
          }
        }
      }
    }
  }
      }
    }
  }

  return status;
}

/**
 *  Gets the index of the firstAvailableSegmentIndex and
 *  lastAvailableSegmentIndex for segment array for the live
 *  case.
 */
void MPDParser::GetFirstAndLastAvailableSegmentInfo(int& firstAvailableSegmentIndex,
                                                    int& lastAvailableSegmentIndex,
                                                    uint32& numberOfAvailableSegments,
                                                    SegmentInfo* pSegment,
                                                    uint32 nNumSegments,
                                                    uint64 periodStartTime,
                                                    double currMSeconds,
                                                    double minUpdatePeriod,
                                                    double timeShiftBufferDepth)
{
  firstAvailableSegmentIndex = -1;
  lastAvailableSegmentIndex = -1;
  QTV_NULL_PTR_CHECK(mpd, RETURN_VOID);

  // Lower bound time to restrict accessible segments taking into account
  // the time shift buffer.
  double lowerBoundTime = currMSeconds - (timeShiftBufferDepth * 1000);
  MM_Time_DateTime fetchTime = GetFetchTime();

  // Upper bound time to restrict accessible segments taking into account
  // current time and mpd checkTime as defined in the spec under the title
  // "Media Segment list restrictions"
  double upperboundTime = currMSeconds;
  if (minUpdatePeriod > 0.0)
  {
    double checkTimeForMPD = (StreamSourceTimeUtils::ConvertSysTimeToMSec(fetchTime) +
                              1000.0 * minUpdatePeriod);
    upperboundTime = STD_MIN(checkTimeForMPD, currMSeconds);
  }

  double availabilityMSeconds = mpd->GetAvailabilityStartTime();
  double availPlusPeriodStart = (double)periodStartTime + availabilityMSeconds;

  if(pSegment && nNumSegments > 0)
  {
    //find out the first available segment
    for(int i = 0;i < (int)nNumSegments; i++)
    {
      //The number of available segments will actually be based on timeShiftBufferDepth
      //For the live case depending on this parameter those many number of segments will be available
      //and for the onDemand case numberOfAvailable segments is nNumSegments if current time
      //is past the availabilityStartTime

      //find the available segments window for the live case.
      //The calculation takes into account timeShiftBuffer.
      double segAvailStartTime = availPlusPeriodStart + pSegment[i].getDuration() + pSegment[i].getStartTime();
      double segAvailEndTime = segAvailStartTime + pSegment[i].getDuration();

      if(segAvailStartTime <= upperboundTime && segAvailEndTime > lowerBoundTime)
      {
        if(-1 == firstAvailableSegmentIndex)
        {
          firstAvailableSegmentIndex = i;
        }
        lastAvailableSegmentIndex = i;
        numberOfAvailableSegments++;
      }

      if(segAvailEndTime > upperboundTime)
      {
        break;
      }
    }//for(int i = 0;i < nNumSegments; i++)
  }
}

/**
 * This calculates the amount of time behind the current time
 * for which playback can start.
 */
uint32 MPDParser::GetTsbToUseAtStartUp() const
{
  uint32 nTsbToUse = 0;

  if (m_sessionInfo.IsStartupLatencyImprovementEnabled())
  {
    uint32 nTsbDepth = 1000 * GetTimeShiftBufferDepth();
    uint32 nPrerollMs = m_sessionInfo.GetInitialPreroll();
    nTsbToUse = QTV_MIN(nTsbDepth, nPrerollMs);

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "TSB to use at startup = %u ms", nTsbToUse);
  }

  return nTsbToUse;
}

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
double MPDParser::GetOffsetFromAvailabilityTimeForPeriod(const MPD& rMpd, const PeriodInfo& rPeriodInfo, double timeShiftBufferDepth)
{
  MM_Time_DateTime sCurrTime;
  MM_Time_GetUTCTime(&sCurrTime);

  uint64 periodStartTime = rPeriodInfo.getStartTime();
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
             "periodStartTime = %d", (int)periodStartTime);
  double nAvailabilityMSeconds = rMpd.GetAvailabilityStartTime();
  double earliestAvailTime = nAvailabilityMSeconds + (double)periodStartTime;
  double nCurrMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(sCurrTime);

  double tsbDepth = 1000 * timeShiftBufferDepth;
  nCurrMSeconds = (nCurrMSeconds > tsbDepth ? nCurrMSeconds - tsbDepth : 0);

  double offsetFromAvailTime =
    (nCurrMSeconds > earliestAvailTime
     ? nCurrMSeconds - earliestAvailTime
     : 0.0);

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "OffsetFromAvailabilityTimeForPeriod for period = %d", (int)offsetFromAvailTime);

  return offsetFromAvailTime;
}

/**
 * replaces $RepresentationID$,$Bandwidth$
 * $Number$, $Time$ with suitable string for
 * template
 */
void MPDParser::ReplaceIdentifierwithValueInUrl(char *targetUrl, const char *idToReplace, char *replaceString, int origUrlLen)
{
  char *tmpUrl = (char *)QTV_Malloc(origUrlLen * sizeof(char));
  if (tmpUrl && targetUrl && idToReplace && replaceString)
  {
    int position;
    char *pos = std_strstr(targetUrl , idToReplace);
    if( pos != 0 )
    {
      position = (int)(pos - targetUrl);
    }
    else
    {
      position = -1;
    }
    while (position >= 0)
    {
      std_strlcpy(tmpUrl, targetUrl, position + 1);
      std_strlcat(tmpUrl, replaceString, origUrlLen);
      std_strlcat(tmpUrl, targetUrl + position + std_strlen(idToReplace),origUrlLen);
      std_strlcpy(targetUrl,tmpUrl,origUrlLen);
      tmpUrl[0] = '\0';
      pos = std_strstr(targetUrl , idToReplace);
      if (pos != 0)
      {
        position = (int)(pos - targetUrl);
      }
      else
      {
        position = -1;
      }
    }
  }
  if(tmpUrl)
  {
    QTV_Free(tmpUrl);
    tmpUrl = NULL;
  }
  return;
}

/**
 * replaces $Bandwidth%[width]d$ $Number%[width]d$,
 * $Time%[width]d$ with suitable string for template
 *
 */
void MPDParser::FormatandReplaceIdentifierwithValueInUrl(char *targetUrl, const char *idToReplace, int value, int origUrlLen)
{
  char actualIdtoReplace[MAX_TEMPLATE_ID_LEN] = {'\0'};
  char format[MAX_TEMPLATE_ID_LEN]            = {'\0'};
  char bwChar[MAX_TEMPLATE_ID_LEN]            = {'\0'};

  int position;
  char *pos = std_strstr(targetUrl , idToReplace);
  if(pos)
  {
    char *pos1 = std_strstr(pos+1 , "$");
    char *pos2 = std_strstr(pos+1 , "%");

    if(pos1 && pos2 && (STD_ARRAY_SIZE(actualIdtoReplace) >= pos1 - pos + 2) &&
                       (STD_ARRAY_SIZE(actualIdtoReplace) >= pos1 - pos2 + 1))
    {
      std_strlcpy(actualIdtoReplace, pos, pos1 - pos + 2);
      std_strlcpy(format, pos2, pos1 - pos2 + 1);
    }
  }

  if(format[0] != '\0' && actualIdtoReplace[0] != '\0')
  {
    std_strlprintf(bwChar, STD_ARRAY_SIZE(bwChar), format, value);
    ReplaceIdentifierwithValueInUrl(targetUrl, actualIdtoReplace, bwChar, origUrlLen);
  }

  return;
}

/**
 * To check $RepresentationID$,$Bandwidth$
 * $Number$, $Time$ if present
 */
bool MPDParser::IsTemplateTagPresent( char *targetUrl, const char *idToReplace)
{
  bool bOk = false;
  if (idToReplace && targetUrl)
  {
    int position;
    char *pos = std_strstr(targetUrl , idToReplace);
    if ( pos != 0 )
    {
      position = (int)(pos - targetUrl);
    }
    else
    {
      position = -1;
    }
    if (position >= 0)
    {
      bOk = true;
    }
  }
  return bOk;
}

HTTPDownloadStatus MPDParser::GetContentProtectElem(HTTPDrmType &drmType,
                                         uint32 &contentProtectionInfoSize,
                                         unsigned char* contentProtectionData)
{
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_SUCCESS;
  GetPropertiesXML((char *)contentProtectionData, (int&)contentProtectionInfoSize, "id#ContentProtection");
  drmType = m_MpdDrmType;
  return  status;
}


HTTPDownloadStatus MPDParser::GetCurrentPlayingContentProtectElem
(
  HTTPMediaType mediaType,
  HTTPDrmType &drmType,
  HTTPContentStreamType &streamType,
  int &contentProtectionInfoSize,
  char* contentProtectionData,
  int currPeriodkey, int currGrpKey,
  int currRepKey
)
{
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_SUCCESS;
  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(NULL == contentProtectionData)
  {
    // query for size
    contentProtectionData = 0;
  }
  else
  {
    contentProtectionData[0] = '\0';
  }

  drmType = m_MpdDrmType;

  streamType = HTTPCommon::MP4_STREAM;

  MPDParserXMLPropertiesHelper::PrintXMLStringForDebug tmpStr(contentProtectionData);

  //MPDParserXMLPropertiesHelper::XMLGeneratorForRootElem rootElem(contentProtectionData, contentProtectionInfoSize, m_MpdProfileStr);

  for(int i = 0; (periodInfo && numPeriods > 0) && i < numPeriods; ++i)
  {
    int numRepgrps = 0;
    if ((int)((uint64)(periodInfo[i].getPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT)
         == currPeriodkey)
    {
      RepresentationGroup *repGrpInfo = periodInfo[i].getRepGrpInfo(numRepgrps);
      for(int j = 0; (repGrpInfo && numRepgrps > 0) && j < numRepgrps; ++j)
      {
          uint32 nMajorType = MJ_TYPE_UNKNOWN;
          GetGroupMajorType(repGrpInfo[j], nMajorType);
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "mediaType %d, nMajorType %d",mediaType, nMajorType);

          if (currGrpKey == -1 && ((HTTPMediaType)nMajorType == mediaType))
          {
            currGrpKey =
             (int)((repGrpInfo[j].getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
          }

          if((int)((repGrpInfo[j].getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT)
              == currGrpKey)
          {
            const char *pMimeType =
                repGrpInfo[j].GetCommonStringValueContainer().FindString("mimeType");
            if (pMimeType && std_strstr(pMimeType, "mp2t"))
            {
              streamType = HTTPCommon::BBTS_STREAM;
            }
            uint32 nMajorType = MJ_TYPE_UNKNOWN;
            GetGroupMajorType(repGrpInfo[j], nMajorType);
            if ((HTTPMediaType)nMajorType == mediaType)
            {


              MPDParserXMLPropertiesHelper::XMLGeneratorForContentProtectionElement
              XMLGeneratorForContentProtectionElement(contentProtectionData,
                                              contentProtectionInfoSize,
                                              repGrpInfo[j].GetContentProtection());
              int numReps = 0;
              RepresentationInfo *repInfo = repGrpInfo[j].getRepInfo(numReps);
              for(int k = 0; (repInfo && numReps > 0) && k < numReps; ++k)
              {
                if (currRepKey == -1 && ((HTTPMediaType)nMajorType == mediaType))
                {
                  currRepKey =
                   (int)((repInfo[k].getKey() & MPD_REPR_MASK) >>
                                              MPD_REPR_SHIFT_COUNT);
                }

                if ((int)((repInfo[k].getKey() & MPD_REPR_MASK) >>
                                              MPD_REPR_SHIFT_COUNT) == currRepKey)
                {
                  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "GetCurrentPlayingRepInfo :: Current Period Key %d, "
                                           "Group Key %d Representation %d",
                       currPeriodkey, currGrpKey, currRepKey);

                   MPDParserXMLPropertiesHelper::XMLGeneratorForContentProtectionElement
                    XMLGeneratorForContentProtectionElement(
                    contentProtectionData, contentProtectionInfoSize,
                     repInfo[k].GetContentProtection());

                  break;
                }
              }
              break;
            }
          }
        }
      break;
    }
  }
  return status;
}

/*
 * method to Fill Period properties in XML format
 */


void MPDParser::FillPeriodXMLProperties(PeriodInfo &periodInfo,
                                        int /* periodKey */,
                                        int &currRepSize,
                                        char *currRepSring)
{
  {
    MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpPropString(
              currRepSring,currRepSize,"PeriodProperties");
    const char *id = periodInfo.getPeriodIdentifier();
    if(id)
    {
      MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
        currRepSring, currRepSize, "id", id);
    }
  }
}

/*
 * method to Fill AdaptationSet properties in XML format
 */
void MPDParser::FillGrpXMLProperties(RepresentationGroup &repGrpInfo,
                                      int &currRepSize,
                                      char *currRepSring)
{
  {
    MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpPropString(
      currRepSring,currRepSize,"AdaptationSetProperties");
    {
      MPDParserXMLPropertiesHelper::XMLGeneratorForContentDescriptorContainer
        generateAdaptionSetContentDescriptorsXML(
           currRepSring, currRepSize, repGrpInfo.GetCommonContentDescriptors());
      const char *pValue =
            repGrpInfo.GetCommonStringValueContainer().FindString("mimeType");

      if(pValue)
      {
        MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
          currRepSring,currRepSize,"mimeType", pValue);
      }

      pValue = repGrpInfo.GetCommonStringValueContainer().FindString("codecs");
      if(pValue)
      {
        MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
          currRepSring,currRepSize,"codecs", pValue);
      }
      Resolution *res = repGrpInfo.getResolution();
      if(res)
      {
        char tmpBuf[20];

        std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->width);
        {
          MPDParserXMLPropertiesHelper::CreateXMLForString
            generateXMLForString(currRepSring,currRepSize,"width", tmpBuf);
        }

        std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->height);
        {
          MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
            currRepSring,currRepSize,"height", tmpBuf);
        }
      }
      {
        MPDParserXMLPropertiesHelper::XMLGeneratorForContentProtectionElement
            XMLGeneratorForContentProtectionElement(currRepSring,currRepSize,repGrpInfo.GetContentProtection());
      }
      const char *id = repGrpInfo.GetGrpIdentifer();
      if(id)
      {
        MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
          currRepSring,currRepSize,"id", id);
      }
      {
        MPDParserXMLPropertiesHelper::XMLGeneratorForContentDescriptorContainer
          generateAdaptionSetContentDescriptorsXML(
            currRepSring,currRepSize, repGrpInfo.GetAdaptationSetSpecificContentDescriptors());
      }
      {
        repGrpInfo.GetAdaptationSetSpecificStringValueContainer().PrintStringValues();
        MPDParserXMLPropertiesHelper::XMLGeneratorForStringValueContainer tmp(
        currRepSring,currRepSize,repGrpInfo.GetAdaptationSetSpecificStringValueContainer());
      }
      {
        MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
        currRepSring,currRepSize,"lang", repGrpInfo.getLanguage());
      }
    }
  }
}

/*
 * method to Fill Representation properties in XML format
 */

 void MPDParser::FillRepXMLProperties(RepresentationInfo &repInfo,
                                      int &currRepSize,
                                      char *currRepSring)
 {
   MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpProps(
                  currRepSring,currRepSize,"RepresentationProperties");
   {
     {
       // this returns only AudioChannelConfiguration for now.
       MPDParserXMLPropertiesHelper::XMLGeneratorForContentDescriptorContainer
            generateCommonContentDescriptorsXML(
            currRepSring ,currRepSize, repInfo.GetCommonDescriptors());
     }
     const char *pValue = NULL;
     pValue = repInfo.GetCommonStringValues().FindString("mimeType");
     if(pValue)
     {
        MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
          currRepSring,currRepSize,"mimeType", pValue);
     }

     pValue = repInfo.GetCommonStringValues().FindString("codecs");
     if(pValue)
     {
       MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
          currRepSring, currRepSize,"codecs", pValue);
     }


     Resolution *res = repInfo.getResolution();
     if(res)
     {
       char tmpBuf[20];

       std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->width);
       {
         MPDParserXMLPropertiesHelper::CreateXMLForString
           generateXMLForString(currRepSring,currRepSize,"width", tmpBuf);
       }
       std_strlprintf(tmpBuf, sizeof(tmpBuf), "%u", res->height);
       {
         MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
           currRepSring,currRepSize,"height", tmpBuf);
       }
     }
   }
   {
     MPDParserXMLPropertiesHelper::XMLGeneratorForContentProtectionElement
        XMLGeneratorForContentProtectionElement(currRepSring, currRepSize, repInfo.GetContentProtection());
   }

   const char *id = repInfo.getRepIdentifier();
   if(id)
   {
     MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
        currRepSring, currRepSize, "id", id);
   }

   if(repInfo.getBandwidth())
   {
     char tmpBuf[20];
     std_strlprintf(tmpBuf, sizeof(tmpBuf), "%d", (int)repInfo.getBandwidth());
     MPDParserXMLPropertiesHelper::CreateXMLForString generateXMLForString(
        currRepSring, currRepSize, "bandwidth", tmpBuf);
   }
 }



/**
  * Retrieving current representation info from MPD
  * current Period, current group, current representation
  * in XML format
  */


HTTPDownloadStatus MPDParser::GetCurrentPlayingRepInfo(int periodKey,
                                              int grpKeyAudio, int grpKeyVideo, int grpKeyText,
                                              int repKeyAudio, int repKeyVideo, int repKeyText,
                                              int &currRepSize, char *currRepSring)
{
  HTTPDownloadStatus status= HTTPCommon::HTTPDL_SUCCESS;
  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(NULL == currRepSring)
  {
    // query for size
    currRepSize = 0;
  }
  else
  {
    currRepSring[0] = '\0';
  }

  MPDParserXMLPropertiesHelper::PrintXMLStringForDebug tmpStr(currRepSring);
  MPDParserXMLPropertiesHelper::XMLGeneratorForRootElem rootElem(currRepSring, currRepSize, m_MpdProfileStr);
  {
    MPDParserXMLPropertiesHelper::XMLGeneratorForPropertiesString tmpPropertiesString(
      currRepSring, currRepSize, "MPDProperties");
    {
      MPDParserXMLPropertiesHelper::CreateXMLForString tmpProfiles(
       currRepSring, currRepSize, "profiles", m_MpdProfileStr);
    }


    for(int i = 0; (periodInfo && numPeriods > 0) && i < numPeriods; ++i)
    {
      int numRepgrps = 0;
      if ((int)((uint64)(periodInfo[i].getPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT)
           == periodKey)
      {
        MPDParserXMLPropertiesHelper::XMLKeyTag periodTag(
            currRepSring, currRepSize, "Period", periodKey);

        FillPeriodXMLProperties(periodInfo[i], periodKey, currRepSize, currRepSring);
        uint32 lastGrpMajorType = MJ_TYPE_UNKNOWN;
        RepresentationGroup *repGrpInfo = periodInfo[i].getRepGrpInfo(numRepgrps);
        for(int j = 0; (repGrpInfo && numRepgrps > 0) && j < numRepgrps; ++j)
        {
          uint32 nMajorType = MJ_TYPE_UNKNOWN;
          GetGroupMajorType(repGrpInfo[j], nMajorType);
          if (!(lastGrpMajorType & nMajorType))
          {
            int grpKey = (int)((repGrpInfo[j].getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
            if  (nMajorType == MJ_TYPE_AUDIO && grpKeyAudio != -1)
            {
              grpKey = grpKeyAudio;
            }
            else if (nMajorType == MJ_TYPE_VIDEO && grpKeyVideo != -1)
            {
              grpKey = grpKeyVideo;
            }
            else if (nMajorType == MJ_TYPE_TEXT && grpKeyText != -1)
            {
               grpKey = grpKeyText;
            }
            if((int)((repGrpInfo[j].getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT)
                == grpKey)
            {
              MPDParserXMLPropertiesHelper::XMLKeyTag adaptationTag(
                  currRepSring, currRepSize, "AdaptationSet", grpKey);

              FillGrpXMLProperties(repGrpInfo[j], currRepSize, currRepSring);

              int numReps = 0;
              RepresentationInfo *repInfo = repGrpInfo[j].getRepInfo(numReps);
              for(int k = 0; (repInfo && numReps > 0) && k < numReps; ++k)
               {
                int repKey = (int)((repInfo[k].getKey() & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT);
                if  (nMajorType == MJ_TYPE_AUDIO && repKeyAudio != -1)
                {
                  repKey = repKeyAudio;
                }
                else if (nMajorType == MJ_TYPE_VIDEO && repKeyVideo != -1)
                {
                  repKey = repKeyVideo;
                }
                else if (nMajorType == MJ_TYPE_TEXT && repKeyText != -1)
                {
                  repKey = repKeyText;
                }
                if ((int)((repInfo[k].getKey() & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT) == repKey)
                {
                  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "GetCurrentPlayingRepInfo :: Current Period Key %d, Group Key %d Representation %d",
                       periodKey, grpKey, repKey);
                  MPDParserXMLPropertiesHelper::XMLKeyTag representationTag(
                     currRepSring, currRepSize, "Representation", repKey);
                  FillRepXMLProperties(repInfo[k],  currRepSize, currRepSring);
                    lastGrpMajorType = nMajorType | lastGrpMajorType;
                  break;
                }
              }
            }
          }
        }
        break;
      }
    }
  }
  return status;
}

/**
  * Retrieving codec type info from MPD, based on current selected
  * Representation
  */

HTTPDownloadStatus MPDParser::GetTrackEncodingInfo(FileSourceMnMediaType& audio,
                                                   FileSourceMnMediaType& video,
                                                   FileSourceMnMediaType& other)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  int numPeriods = 0;
  audio = video = other = FILE_SOURCE_MN_TYPE_UNKNOWN;
  PeriodInfo *periodInfo = mpd ? mpd->getPeriodInfo(numPeriods) : NULL;

  if(periodInfo && numPeriods > 0)
  {
    int numGrps = 0;
    RepresentationGroup* rep_grps = periodInfo[0].getRepGrpInfo(numGrps);
    bool alltracksfound = false;
    if(rep_grps && numGrps > 0)
    {
      for(int i = 0; i < numGrps; i++)
      {
        if(rep_grps[i].IsAnyRepSelected())
        {
          int numRepresentations = 0;
          RepresentationInfo* pReps = rep_grps[i].getRepInfo(numRepresentations);

          for(int j=0;j<numRepresentations;j++)
          {
            if(pReps[j].IsMarkedSelected())
            {
              int nNumCodecs = 0;

              (void)pReps[j].getCodec(NULL, nNumCodecs);
              if (nNumCodecs > 0)
              {
                CodecInfo* pCodec = (CodecInfo *)QTV_Malloc(nNumCodecs * sizeof(CodecInfo));
                if (pCodec)
                {
                  if (pReps[j].getCodec(pCodec, nNumCodecs))
                  {
                    for (int i = 0; i < nNumCodecs; i++)
                    {
                      switch(pCodec[i].minorType)
                      {
                       case MN_TYPE_AVC:
                         video = FILE_SOURCE_MN_TYPE_H264;
                       break;
                       case MN_TYPE_MPEG4:
                         video = FILE_SOURCE_MN_TYPE_MPEG4;
                       break;
                       case MN_TYPE_HVC:
                         video = FILE_SOURCE_MN_TYPE_HEVC;
                       break;
                       case MN_TYPE_MP3:
                         audio = FILE_SOURCE_MN_TYPE_MP3;
                       break;
                       case MN_TYPE_HE_AAC:
                       case MN_TYPE_AAC_LC:
                         audio = FILE_SOURCE_MN_TYPE_AAC;
                       break;
                       case MN_TYPE_AC3:
                         audio = FILE_SOURCE_MN_TYPE_AC3;
                       break;
                       case MN_TYPE_VORBIS:
                         audio = FILE_SOURCE_MN_TYPE_VORBIS;
                       break;
                       case MN_TYPE_TIMED_TEXT:
                         other = FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT;
                       break;
                       default:
                       // not supported
                       break;
                      }

                      if((audio != FILE_SOURCE_MN_TYPE_UNKNOWN) &&
                         (video != FILE_SOURCE_MN_TYPE_UNKNOWN) &&
                         (other != FILE_SOURCE_MN_TYPE_UNKNOWN))
                       {
                         alltracksfound = true;
                         break;
                       }
                    }
                  }
                  QTV_Free(pCodec);
                  pCodec = NULL;
                }
              }
            }

            if(alltracksfound)
            {
              break;
            }
          } // reps loop
        }

        if(alltracksfound)
        {
          break;
        }
      } // rep group loop
    }

    status = HTTPCommon::HTTPDL_SUCCESS;
  }

  return status;
}

/**
 * This sets the m_IsSelectable flag for each adaptation set as
 * specified by the arg 'bIsSelectable'. The m_bIsSelectable
 * flag in each representation takes on this value as well. Each
 * representation in marked not-selected as well.
 * Text is not selected as default selection
 */
void MPDParser::MarkDefaultRepGrpsAsSelectable(bool bIsSelectable)
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "MarkDefaultRepGrpsAsSelectable");
  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(periodInfo && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      MarkDefaultRepGrpsAsSelectableForPeriod(periodInfo[i],
                                              bIsSelectable);
    }
  }
}

/**
 * Helper function for MarkDefaultRepGrpsAsSelectable at period
 * level.
 */
void MPDParser::MarkDefaultRepGrpsAsSelectableForPeriod(PeriodInfo& rPeriod,
                                                        bool bIsSelectable)
{
  int numRepgrps = 0;
  RepresentationGroup *repGrpInfo = rPeriod.getRepGrpInfo(numRepgrps);
  if(repGrpInfo && numRepgrps > 0)
  {
    for(int j = 0; j < numRepgrps; ++j)
    {
      int numReps = 0;
      RepresentationInfo *pRepInfo = repGrpInfo[j].getRepInfo(numReps);

      if(pRepInfo && numReps > 0)
      {
        for(int k = 0; k < numReps; ++k)
        {
          uint32 nMajorType = MJ_TYPE_UNKNOWN;
          GetRepMajorType(pRepInfo[k], nMajorType);
          if(nMajorType != MJ_TYPE_TEXT)
          {
            pRepInfo[k].MarkSelectable(bIsSelectable);
          }
        }
      }
    }
  }
}

/**
 * This sets the m_IsSelectable flag for each adaptation set as
 * specified by the arg 'bIsSelectable'. The m_bIsSelectable
 * flag in each representation takes on this value as well. Each
 * representation in marked not-selected as well.
 */
void MPDParser::MarkAllRepGrpsAsSelectable(bool bIsSelectable)
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "MarkAllRepGrpsAsSelectable");
  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(periodInfo && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      int numRepgrps = 0;
      RepresentationGroup *repGrpInfo = periodInfo[i].getRepGrpInfo(numRepgrps);
      if(repGrpInfo && numRepgrps > 0)
      {
        for(int j = 0; j < numRepgrps; ++j)
        {
          int numReps = 0;
          RepresentationInfo *pRepInfo = repGrpInfo[j].getRepInfo(numReps);

          if(pRepInfo && numReps > 0)
          {
            for(int k = 0; k < numReps; ++k)
            {
              pRepInfo[k].MarkSelectable(bIsSelectable);
            }
          }
        }
      }
    }
  }
}

/**
 * Marks the representation specified by
 * (nPeriodIdentifer,nAdaptationSetIdentifier,nRepresentationIdentifier)
 * as selectable.
 */
void MPDParser::MarkRepresentationAsSelectable(
  uint32 nPeriodIdentifer, uint32 nAdaptationSetIdentifier, uint32 nRepresentationIdentifier)
{
  uint64 nTmpPeriodId = nPeriodIdentifer;
  uint64 nTmpAdaptationSetId = nAdaptationSetIdentifier;
  uint64 nAdaptationSetKey = (nTmpPeriodId << MPD_PERIOD_SHIFT_COUNT) || (nTmpAdaptationSetId << MPD_REPGRP_SHIFT_COUNT);

  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(periodInfo && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      if(nPeriodIdentifer == ((periodInfo[i].getPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT) ||
         (uint32)KeyStruct::MAGIC_VAL == nPeriodIdentifer)
      {
        int numRepgrps = 0;
        RepresentationGroup *repGrpInfo = periodInfo[i].getRepGrpInfo(numRepgrps);
        if(repGrpInfo && numRepgrps > 0)
        {
          for(int j = 0; j < numRepgrps; ++j)
          {
            if(nAdaptationSetIdentifier == ((repGrpInfo[j].getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT) ||
               (uint32)KeyStruct::MAGIC_VAL == nAdaptationSetIdentifier)
            {
              int numReps = 0;
              RepresentationInfo *repInfo = repGrpInfo[j].getRepInfo(numReps);

              if(repInfo && numReps > 0)
              {
                for(int k = 0; k < numReps; ++k)
                {
                  if(((repInfo[k].getKey() & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT) == nRepresentationIdentifier ||
                     (uint32)KeyStruct::MAGIC_VAL == nRepresentationIdentifier)
                  {
                    repInfo[k].MarkSelectable(true);

                    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Selected representation %s in with periodIdx %d, repGrpIdx %d",
                      repInfo[k].getRepIdentifier() ? repInfo[k].getRepIdentifier() : "Unknown",
                      (int)nPeriodIdentifer, (int)nRepresentationIdentifier);
                  }
                }
              }
              else
              {
                QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Failed to find a representation in adapatation "
                  "set with periodId %d, repGrpId %d",
                (int)nPeriodIdentifer, (int)nAdaptationSetIdentifier);
              }
            }
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Did not find any valid adaptation set");
        }
        break;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Did not find any period");
  }
}

/**
 * If any period is found unselected by user choice, this
 * ensures that period is marked for default selection. The
 * application should not be allowed to de-select a period thus
 * leaving a hole in terms of presentation-time. To skip, media
 * the app should be allowed only via SEEK.
 */
void MPDParser::EnsureAllPeriodsSelected()
{
  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(periodInfo && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      int numRepgrps = 0;
      RepresentationGroup *repGrpInfo = periodInfo[i].getRepGrpInfo(numRepgrps);

      if(repGrpInfo && numRepgrps > 0)
      {
        bool bFoundValidSelectionForPeriod = false;

        for(int j = 0; j < numRepgrps; ++j)
        {
          if(repGrpInfo[j].IsAnyRepSelectable())
          {
            bFoundValidSelectionForPeriod = true;
            break;
          }
        }

        if(!bFoundValidSelectionForPeriod)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "EnsureAllPeriodsSelected: Period with key %llu "
            "marked for default selection as no valid user selection for period",
            periodInfo[i].getPeriodKey());

          MarkDefaultRepGrpsAsSelectableForPeriod(periodInfo[i], true);
        }
      }
    }
  }


}

/**
 * This looks at all the representations that are marked
 * selectable and selects at most one audio, video, text
 * adaptation-sets.
 */
void MPDParser::DoContentSelection()
{
  int numPeriods = 0;
  PeriodInfo *periodInfo = mpd->getPeriodInfo(numPeriods);

  if(periodInfo && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      bool bFoundAudio = false, bFoundVideo = false, bFoundText = false;
      bool bIsInterleaved = false;
      int numRepgrps = 0;
      RepresentationGroup *repGrpInfo = periodInfo[i].getRepGrpInfo(numRepgrps);
      if(repGrpInfo && numRepgrps > 0)
      {
        for(int j = 0; j < numRepgrps; ++j)
        {
          if(!repGrpInfo[j].IsAnyRepSelectable())
          {
            // print msg
            continue;
          }

          uint32 mediaType;
          GetGroupMajorType(repGrpInfo[j], mediaType);

          if((mediaType & MJ_TYPE_AUDIO) &&
             (mediaType & MJ_TYPE_VIDEO) &&
             !bFoundAudio && !bFoundVideo)
          {
            // found interleaved audio,video
            bFoundAudio = true;
            bFoundVideo = true;

            if(mediaType & MJ_TYPE_TEXT)
            {
              bFoundText = true;
            }

            bIsInterleaved = true;

            repGrpInfo[j].MarkAllSelectableRepsAsSelected(); // mark adaptation-set as selected.
          }
          else if(!bFoundAudio &&
                  mediaType == MJ_TYPE_AUDIO)
          {
            bFoundAudio = true;
            repGrpInfo[j].MarkAllSelectableRepsAsSelected();
          }
          else if(!bFoundVideo &&
                  mediaType == MJ_TYPE_VIDEO)
          {
            bFoundVideo = true;
            repGrpInfo[j].MarkAllSelectableRepsAsSelected();
          }
        }
      }

      // Text is only allowed along with Audio or Video, No Group/Representations
      // will be selected for Text only period
      if((bFoundAudio || bFoundVideo) && ((bIsInterleaved && !bFoundText) ||
         !bIsInterleaved))
      {
        // check if there is any text track that exists in isolation in another adaptation-set.
        for(int j = 0; j < numRepgrps; ++j)
        {
          uint32 mediaType;
          GetGroupMajorType(repGrpInfo[j], mediaType);
          if(mediaType == MJ_TYPE_TEXT)
          {
            if(repGrpInfo[j].IsAnyRepSelectable())
            {
              bFoundText = true;
              repGrpInfo[j].MarkAllSelectableRepsAsSelected();
              break;
            }
          }
        }
      }
    }
  }
}

bool MPDParser::ParseDescriptorTypeElements(TiXmlElement* GroupElement,
                                            ContentDescriptorContainer& rCdc,
                                            const char *adaptationSetDescriptorName)
{
  QTV_NULL_PTR_CHECK(GroupElement, false);
  QTV_NULL_PTR_CHECK(adaptationSetDescriptorName, false);

  bool bOk = true;

  char elementSearchStr[100];
  std_strlcpy(elementSearchStr,
              adaptationSetDescriptorName,
              STD_ARRAY_SIZE(elementSearchStr));

  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                   "%s:%s", m_pNamespaceKey, adaptationSetDescriptorName);
  }

  TiXmlElement* descTypeElement = GroupElement->FirstChildElement(elementSearchStr);

  while (descTypeElement)
  {
    TiXmlAttribute* attrib=descTypeElement->FirstAttribute();
    char *tmpBuf1 = NULL, *tmpBuf2 = NULL;
    bool bOk1 = false, bOk2 = false;

    while(attrib)
    {
      char *attrib_name=(char*)skip_whitespace(attrib->Name());
      if (attrib_name)
      {
        if(!std_stricmp(attrib_name,"schemeIdUri"))
        {
          int reqBufSize = 1 + (int)std_strlen(attrib->Value());
          tmpBuf1 = (char *)QTV_Malloc(reqBufSize * sizeof(char));
          if(tmpBuf1)
          {
            std_strlcpy(tmpBuf1, attrib->Value(), reqBufSize);
            bOk1 = true;
          }
        }
        else if(!std_stricmp(attrib_name, "value"))
        {
          // this is optional
          int reqBufSize = 1 + (int)std_strlen(attrib->Value());
          tmpBuf2 = (char *)QTV_Malloc(reqBufSize * sizeof(char));
          if(tmpBuf2)
          {
            std_strlcpy(tmpBuf2, attrib->Value(), reqBufSize);
            bOk2 = true;
          }
        }
      }
      attrib=attrib->Next();
    }

    if(bOk1)
    {
      if(0 == std_stricmp(adaptationSetDescriptorName, "AudioChannelConfiguration"))
      {
        bOk = false;
        if(tmpBuf2)
        {
          int audioChannelValue = atoi(tmpBuf2);

          if(audioChannelValue >= 0)
          {
            uint32 maxSupportedAudioSpecificConfig =
              m_sessionInfo.GetMaxSupportedAudioSpecificConfigValue();

            if(maxSupportedAudioSpecificConfig > 0)
            {
              if((uint32)audioChannelValue <= maxSupportedAudioSpecificConfig)
              {
                bOk = true;
              }
              else
              {
                QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "Filtering out audio channel with ASC %d > %u",
                             audioChannelValue, maxSupportedAudioSpecificConfig);
              }
            }
            else
            {
              for(int *pAudioConfigVal = SupportedAudioChannelValues;
                   *pAudioConfigVal != -1;
                   ++pAudioConfigVal)
              {
                if(audioChannelValue == *pAudioConfigVal)
                {
                  bOk = true;
                  break;
                }
              }
            }
          }
          else
          {
            bOk = false;
          }

          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "AudioChannelConfiguration with value %d. IsSupported? %d",
                        audioChannelValue, bOk);
        }
      }

      if(bOk)
      {
        rCdc.AddDescriptorType(adaptationSetDescriptorName, tmpBuf1, (bOk2 ? tmpBuf2 : ""));
      }
    }

    if(tmpBuf1)
    {
      QTV_Free(tmpBuf1);
    }

    if(tmpBuf2)
    {
      QTV_Free(tmpBuf2);
    }

    descTypeElement = descTypeElement->NextSiblingElement(elementSearchStr);
  }

  return bOk;
}

void MPDParser::ParseContentProtection(TiXmlElement* GroupElement,
                                       ContentProtectionType& rContentProtection)
{
  QTV_NULL_PTR_CHECK(GroupElement, RETURN_VOID);
  static const char* adaptationSetDescriptorName = "ContentProtection";
  rContentProtection.SetProtectionSource(
        ContentProtectionType::CONTENT_PROTECTION_NONE);

  char elementSearchStr[100];
  std_strlcpy(elementSearchStr,
              adaptationSetDescriptorName,
              STD_ARRAY_SIZE(elementSearchStr));

  if (m_pNamespaceKey)
  {
    std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                   "%s:%s", m_pNamespaceKey, adaptationSetDescriptorName);
  }

  TiXmlElement* descTypeElement = GroupElement->FirstChildElement(elementSearchStr);

  while (descTypeElement)
  {
    TiXmlAttribute* attrib=descTypeElement->FirstAttribute();
    char *tmpBuf1 = NULL, *tmpBuf2 = NULL;
    bool bOk1 = false, bOk2 = false;

    while(attrib)
    {
      char *attrib_name=(char*)skip_whitespace(attrib->Name());
      if (attrib_name)
      {
        if(!std_stricmp(attrib_name,"schemeIdUri"))
        {
          int reqBufSize = 1 + (int)std_strlen(attrib->Value());
          tmpBuf1 = (char *)QTV_Malloc(reqBufSize * sizeof(char));
          if(tmpBuf1)
          {
            std_strlcpy(tmpBuf1, attrib->Value(), reqBufSize);
            if (std_strstr(tmpBuf1, MARLIN_SCHEMEID_URI))
            {
              m_MpdDrmType = HTTPCommon::MARLIN_DRM;
            }
            else if (std_strstr(tmpBuf1, PLAYREADY_SCHEMEID_URI))
            {
              m_MpdDrmType = HTTPCommon::PLAYREADY_DRM;
            }
            else if (std_strstr(tmpBuf1, CENC_MP4_SCHEMEID_URI) ||
                     std_strstr(tmpBuf1, CENC_MP2TS_SCHEMEID_URI))
            {
              if ( m_MpdDrmType == HTTPCommon::NO_DRM)
              {
                m_MpdDrmType = HTTPCommon::CENC_DRM;
              }
              else
              {
                bOk1 = false;
                break;
              }
            }
            bOk1 = true;
          }
        }
        else if(!std_stricmp(attrib_name, "value"))
        {
          // this is optional
          int reqBufSize = 1 + (int)std_strlen(attrib->Value());
          tmpBuf2 = (char *)QTV_Malloc(reqBufSize * sizeof(char));
          if(tmpBuf2)
          {
            std_strlcpy(tmpBuf2, attrib->Value(), reqBufSize);
            if (std_stricmp(tmpBuf2, "cenc"))
            {
              if ( m_MpdDrmType == HTTPCommon::NO_DRM)
              {
                m_MpdDrmType = HTTPCommon::CENC_DRM;
              }
            }
            bOk2 = true;
          }
        }
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DRM type set to %d", m_MpdDrmType);
      }
      attrib=attrib->Next();
    }

    if(tmpBuf1 && bOk1)
    {
      rContentProtection.SetDesc(adaptationSetDescriptorName, tmpBuf1, (bOk2 ? tmpBuf2 : ""));

      if(!std_stricmp(tmpBuf1, ContentProtectionType::MARLIN_SCHEME_URI))
      {
        rContentProtection.SetProtectionSource(
          ContentProtectionType::CONTENT_PROTECTION_MARLIN);
#ifdef WIN32
          TiXmlPrinter printer;
          printer.SetStreamPrinting();
          descTypeElement->Accept(&printer);
          rContentProtection.SetMarlinContentProtection(printer.CStr());
#else
          TIXML_OSTREAM outDesc ;
          outDesc << *descTypeElement;
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "MRLN:ContentProtection Element is %s",
                           outDesc.c_str());
          rContentProtection.SetMarlinContentProtection(outDesc.c_str());
#endif

          }
      else if(!std_stricmp(tmpBuf1, ContentProtectionType::PLAYREADY_SCHEME_URI))
          {
        rContentProtection.SetProtectionSource(
                           ContentProtectionType::CONTENT_PROTECTION_PLAYREADY);
#ifdef WIN32
          TiXmlPrinter printer;
          printer.SetStreamPrinting();
          descTypeElement->Accept(&printer);
          rContentProtection.SetPlayReadyContentProtection(printer.CStr());
#else
          TIXML_OSTREAM outDesc ;
          outDesc << *descTypeElement;
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "PR:ContentProtection Element is %s",
                           outDesc.c_str());
          rContentProtection.SetPlayReadyContentProtection(outDesc.c_str());
#endif

        }
      else if (!std_stricmp(tmpBuf1, ContentProtectionType::CENC_MP4_SCHEME_URI) ||
               !std_stricmp(tmpBuf1, ContentProtectionType::CENC_MP2TS_SCHEME_URI))
      {
        rContentProtection.SetProtectionSource(
          ContentProtectionType::CONTENT_PROTECTION_DASH);
#ifdef WIN32
          TiXmlPrinter printer;
          printer.SetStreamPrinting();
          descTypeElement->Accept(&printer);
          rContentProtection.SetCENCContentProtection(printer.CStr());
#else
          TIXML_OSTREAM outDesc ;
          outDesc << *descTypeElement;
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "CENC:ContentProtection Element is %s",
                           outDesc.c_str());
          rContentProtection.SetCENCContentProtection(outDesc.c_str());
#endif
      }
    }

    if(tmpBuf1)
    {
      QTV_Free(tmpBuf1);
    }

    if(tmpBuf2)
    {
      QTV_Free(tmpBuf2);
    }

    elementSearchStr[0] = '\0';
    std_strlcpy(elementSearchStr,
              adaptationSetDescriptorName,
              STD_ARRAY_SIZE(elementSearchStr));

    if (m_pNamespaceKey)
    {
      std_strlprintf(elementSearchStr, STD_ARRAY_SIZE(elementSearchStr),
                     "%s:%s", m_pNamespaceKey, adaptationSetDescriptorName);
    }
    descTypeElement = descTypeElement->NextSiblingElement(elementSearchStr);
  }

  rContentProtection.Print();
}

/**
 * Parses the xml-element associated with 'MPD' and populates
 * the representation-identifiers into a list.
 */
bool MPDParser::ParseKeysFromXML(TiXmlElement *elem,
                                 IPStreamList<KeyStruct>& keyList)
{
  static const int magicVal = 0x7fffffff;

  bool bOk = false;
  const char *periodName = "Period";
  const char *adaptationSetName = "AdaptationSet";
  const char *representationName = "Representation";

  TiXmlElement *periodElem = elem->FirstChildElement(periodName);

  if(periodElem)
  {
    while(periodElem)
    {
      int periodKey;
      bOk = GetIntValueForKey(periodElem, periodKey);

      if(true == bOk)
      {
        // parse adaptation set keys
        TiXmlElement *adaptationSetElem = periodElem->FirstChildElement(adaptationSetName);
        if(adaptationSetElem)
        {
          while(adaptationSetElem)
          {
            int adaptationSetKey;
            bOk = GetIntValueForKey(adaptationSetElem, adaptationSetKey);

            if(true == bOk)
            {
              TiXmlElement *repElem = adaptationSetElem->FirstChildElement(representationName);
              if(repElem)
              {
                while(repElem)
                {
                   int repKey;
                   bOk = GetIntValueForKey(repElem, repKey);

                   if(true == bOk)
                   {

                     KeyStruct keyStruct;
                     keyStruct.m_PeriodKey = periodKey;
                     keyStruct.m_AdaptationSetKey = adaptationSetKey;
                     keyStruct.m_RepresentationKey = repKey;
                     keyList.Push(keyStruct);

                     QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Extracted from app-xml as 'selectable' (Period %d, Adap %d, rep %d)",
                       periodKey, adaptationSetKey, repKey);
                   }
                   else
                   {
                     QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                                  "Failed to get key for rep elem");
                     break;
                   }

                   repElem = repElem->NextSiblingElement(representationName);
                }
              }
              else
              {
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Extracted from app-xml as 'selectable' all reps in "
                  "adaptationSet %d", adaptationSetKey);

                int repKey = magicVal;
                KeyStruct keyStruct;
                keyStruct.m_PeriodKey = periodKey;
                keyStruct.m_AdaptationSetKey = adaptationSetKey;
                keyStruct.m_RepresentationKey = repKey;
                keyList.Push(keyStruct);

                QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                              "Extracted from app-xml as 'selectable' (Period %d, Adap %d, rep %d)",
                              periodKey, adaptationSetKey, repKey);
              }
            }
            else
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "ParseKeysFromXML Failed to get key for adaptation-set elem");

              break;
            }


            adaptationSetElem = adaptationSetElem->NextSiblingElement(adaptationSetName);
          }
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "Extracted from app-xml as 'selectable' all adaptation-sets selected "
            "for period %d", periodKey);

          int adaptationSetKey = magicVal;
          int repKey = magicVal;

          KeyStruct keyStruct;
          keyStruct.m_PeriodKey = periodKey;
          keyStruct.m_AdaptationSetKey = adaptationSetKey;
          keyStruct.m_RepresentationKey = repKey;
          keyList.Push(keyStruct);

          QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "Extracted from app-xml as 'selectable'(Period %d, Adap %d, rep %d)",
            periodKey, adaptationSetKey, repKey);
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "ParseKeysFromXML: Failed to get key from period elem");
        break;
      }

      periodElem = periodElem->NextSiblingElement(periodName);
    }
  }
  else
  {
    // all periods selected.
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Extracted from app-xml as 'selectable' all periods");

    int periodKey = magicVal;
    int adaptationSetKey = magicVal;
    int repKey = magicVal;

    KeyStruct keyStruct;
    keyStruct.m_PeriodKey = periodKey;
    keyStruct.m_AdaptationSetKey = adaptationSetKey;
    keyStruct.m_RepresentationKey = repKey;
    keyList.Push(keyStruct);

    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "Extracted from app-xml as 'selectable' (Period %d, Adap %d, rep %d)",
      periodKey, adaptationSetKey, repKey);
  }
  return bOk;
}

/**
 * Parse the value of the "Key" attribute and return the 'int'
 * value.
 */
bool MPDParser::GetIntValueForKey(TiXmlElement *elem, int& keyValue)
{
  bool bOk = false;
  keyValue = -1;

  if(elem)
  {
    TiXmlAttribute* attrib = elem->FirstAttribute();

    if(attrib)
    {
      const char *attrib_name = skip_whitespace(attrib->Name());
      const char *attrib_value = skip_whitespace(attrib->Value());

      if (attrib_name && attrib_value)
      {
        if(0 == std_stricmp(attrib_name, "Key"))
        {
          keyValue = atoi(attrib_value);
          bOk = true;
        }
      }
      else
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "attrib_name %llu or attrib_value %llu is null", (uint64)attrib_name, (uint64)attrib_value);
      }
    }
  }

  return bOk;
}

/**
 * For debugging.
 */
void MPDParser::PrintSelectedReps()
{
  int numPeriods = 0;
  PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);

  if(numPeriods && numPeriods > 0)
  {
    for(int i = 0; i < numPeriods; ++i)
    {
      int numRepGrps = 0;
      RepresentationGroup *repGrps = period_info[i].getRepGrpInfo(numRepGrps);

      if(repGrps && numRepGrps > 0)
      {
        for(int j = 0; j < numRepGrps; ++j)
        {
          int numReps = 0;
          RepresentationInfo *repArray = repGrps[j].getRepInfo(numReps);

          if(repArray && numReps > 0)
          {
            for(int k = 0; k < numReps; ++k)
            {
              if(repArray[k].IsMarkedSelected())
              {
                uint64 key = repArray[k].getKey();
                uint32 period_index = (uint32)((key & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
                uint32 grp_index  = (uint32)((key & MPD_REPGRP_MASK)>>MPD_REPGRP_SHIFT_COUNT);
                uint32 rep_index = (uint32)((key & MPD_REPR_MASK)>>MPD_REPR_SHIFT_COUNT);
                QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "PrintSelectedReps Selected rep %s (%u,%u,%u) ",
                  repArray[k].getRepIdentifier() ? repArray[k].getRepIdentifier() : "Unknown",
                  period_index, grp_index, rep_index);
              }
            }
          }
        }
      }
    }
  }
}

MPDParser::KeyStruct::KeyStruct() :
  m_PeriodKey(MAGIC_VAL),
  m_AdaptationSetKey(MAGIC_VAL),
  m_RepresentationKey(MAGIC_VAL)
{

}

MPDParser::KeyStruct::KeyStruct(const KeyStruct& rKey)
{
  Assign(rKey);
}

MPDParser::KeyStruct::~KeyStruct()
{

}

MPDParser::KeyStruct& MPDParser::KeyStruct::operator=(const MPDParser::KeyStruct& rKey)
{
  Assign(rKey);
  return *this;
}

void MPDParser::KeyStruct::Assign(const KeyStruct& rKey)
{
  m_PeriodKey = rKey.m_PeriodKey ;
  m_AdaptationSetKey = rKey.m_AdaptationSetKey;
  m_RepresentationKey = rKey.m_RepresentationKey;
}

/**
 * @breif Store the original MPD text
 *
 * @param MPDText
 */
void MPDParser::StoreMPDText(const char *MPDText)
{
  if (MPDText)
  {
    size_t mpdTextLen = 0;
    // Delete previous MPD text valid for live case
    if (m_sMPDText)
    {
      QTV_Free(m_sMPDText);
      m_sMPDText = NULL;
      m_nMPDTextLen = 0;
    }

    mpdTextLen = std_strlen(MPDText)+ 1;
    m_sMPDText = (char*)QTV_Malloc( sizeof(char) * mpdTextLen);
    if (m_sMPDText)
    {
      memset(m_sMPDText, 0x0, mpdTextLen);
      std_strlcpy(m_sMPDText ,MPDText,mpdTextLen);
      m_nMPDTextLen = (uint32)mpdTextLen;
    }
  }
}

/**
 * @brief provides the MPDtext
 *
 * @param pMPDtext
 * @param size
 */
void MPDParser::GetMPDText(char *pMPDtext, uint32& size)
{
  if (pMPDtext && size >= m_nMPDTextLen)
  {
    std_strlcpy(pMPDtext,m_sMPDText,size);
  }
  else
  {
    size = m_nMPDTextLen;
  }
}

/**
 * Indicates if the URL is  localhost URL
 *
 * @param url
 *
 * @return true if localhost , false otherwise
 */
bool MPDParser::IsLocalHostURL(const char* url)
{
  bool bLocalHostURL = false;
  if (url)
  {
    const char *localHostIpAddStr = "http://127.0.0.1";
    const char *localHostNameStr =  "http://localhost";
    bLocalHostURL = true;

    if (std_strncmp(url,localHostIpAddStr,std_strlen(localHostIpAddStr)))
    {
      if (std_strncmp(url,localHostNameStr,std_strlen(localHostNameStr)))
      {
        bLocalHostURL = false;
      }
    }
  }

  return bLocalHostURL;
}

/**
 * @brief Indicates if the Media segments are referring to
 *        localHost
 *
 * @return True if locally available otherwise false
 */
bool MPDParser::IsLiveContentHostedLocally()
{
  bool isMediaLocallyAvailable = false;
  if (IsLive())
  {
    int numPeriods = 0;
    PeriodInfo *period_info = mpd->getPeriodInfo(numPeriods);
    isMediaLocallyAvailable = true;
    for(int period_index = 0;period_index < numPeriods && isMediaLocallyAvailable; period_index++)
    {
      int numGrps = 0;
      RepresentationGroup* pRepGrp = period_info[period_index].getRepGrpInfo(numGrps);
      for(int grp_index = 0; grp_index < numGrps && isMediaLocallyAvailable; grp_index++)
      {
         int numRepresentations = 0;
         RepresentationInfo *pReps = pRepGrp[grp_index].getRepInfo(numRepresentations);
         for(int rep_index = 0; rep_index < numRepresentations && isMediaLocallyAvailable; ++rep_index)
         {
           if (isMediaLocallyAvailable)
           {
             const char *InitURL = pReps->GetInitialisationSegmentUrl();
             if (InitURL && !std_strncmp(InitURL,"http://", 7))
             {
               if (!IsLocalHostURL(InitURL))
               {
                 isMediaLocallyAvailable = false;
                 break;
               }
             }
             if (isMediaLocallyAvailable)
             {
               uint32 nNumSegments =0,size =0;
               SegmentInfo* pSegment=pReps[rep_index].getSegmentInfo(nNumSegments,size);
               if (pSegment)
               {
                 for(uint32 seg_index = 0;seg_index < nNumSegments && isMediaLocallyAvailable; seg_index++)
                 {
                   const char* segURL = pSegment[seg_index].GetURL();
                   if (!segURL || (segURL && !IsLocalHostURL(segURL)))
                   {
                     isMediaLocallyAvailable = false;
                     break;
                   }
                 }
               }
               else if (pReps[rep_index].IsSegmentTemplateFound())
               {
                 SegmentTemplateType *pSegmentTemplate = pReps[rep_index].GetSegmentTemplate();
                 if (pSegmentTemplate)
                 {
                   URLType *initURLType = pSegmentTemplate->GetInitialisation();
                   if (initURLType)
                   {
                     const char *initialisationUrl = initURLType->sourceURL;
                     if (initialisationUrl && !std_strncmp(initialisationUrl,"http://", 7))
                     {
                       if (!IsLocalHostURL(initialisationUrl))
                       {
                         isMediaLocallyAvailable = false;
                         break;
                       }
                     }
                   }
                   if (isMediaLocallyAvailable)
                   {
                     URLType *indexURLType = pSegmentTemplate->GetRepresentationIndex();
                     if (indexURLType)
                     {
                       const char *IndexURL = indexURLType->sourceURL;
                       if (IndexURL && !std_strncmp(IndexURL,"http://", 7))
                       {
                         if (!IsLocalHostURL(IndexURL))
                         {
                           isMediaLocallyAvailable = false;
                           break;
                         }
                       }
                     }
                   }
                 }
               }
             }
           }
         }
      }
    }
  }
  return isMediaLocallyAvailable;
}

/**
 * Get an upper bound of the size needed for allocation of the
 * template string in mpd.
 */
int MPDParser::GetMaxBufferSizeForStringTemplate(RepresentationInfo& rRepInfo, char *urlTemplate)
{
  int reqdBufSize = 0;

  if (urlTemplate)
  {
    int count = 0;

    reqdBufSize = (int)std_strlen(urlTemplate) + 1;
    if (rRepInfo.getRepIdentifier())
    {
      count = GetNumOccurencesOfTemplateTag(urlTemplate, "$RepresentationID$");
      reqdBufSize += count * (int)std_strlen(rRepInfo.getRepIdentifier());
    }

    count = GetNumOccurencesOfTemplateTag(urlTemplate, "$Bandwidth$");
    reqdBufSize += count * MAX_TEMPLATE_ID_BANDWIDTH_LEN;

    count = GetNumOccurencesOfTemplateTag(urlTemplate, "$Bandwidth%");
    reqdBufSize += count * MAX_TEMPLATE_ID_LEN;

    count = GetNumOccurencesOfTemplateTag(urlTemplate, "$Time$");
    reqdBufSize += count * MAX_TEMPLATE_ID_TIMELINE_LEN;

    count = GetNumOccurencesOfTemplateTag(urlTemplate, "$Time%");
    reqdBufSize += count * MAX_TEMPLATE_ID_LEN;

    count = GetNumOccurencesOfTemplateTag(urlTemplate, "$Number$");
    reqdBufSize += count * MAX_TEMPLATE_ID_NUMBER_LEN;

    count = GetNumOccurencesOfTemplateTag(urlTemplate, "$Number%");
    reqdBufSize += count * MAX_TEMPLATE_ID_LEN;
  }

  return reqdBufSize;
}

/**
 * Populate dstBuffer from the mpd urlTemplate.
 */
void MPDParser::SetBufferForStringTemplate(
   RepresentationInfo& rRepInfo, char *urlTemplate,
   char *dstBuffer, int dstBufferSize, int64 startNumberOrTime)
{
  // Either $Number$ or $Time$ may be used but not both at the same time.
  if(urlTemplate)
  {
    std_strlcpy(dstBuffer, urlTemplate, dstBufferSize);

    //replace $RepresentationID$ with Representation@id
    if (IsTemplateTagPresent(dstBuffer, "$RepresentationID$"))
    {
      if(rRepInfo.getRepIdentifier())
      {
        ReplaceIdentifierwithValueInUrl(dstBuffer, "$RepresentationID$",
                                        rRepInfo.getRepIdentifier(), dstBufferSize);
      }
    }
    //replace $Bandwidth$ with Representation@bandWidth
    if (IsTemplateTagPresent(dstBuffer, "$Bandwidth$"))
    {
      char bwChar[MAX_TEMPLATE_ID_BANDWIDTH_LEN];
      std_strlprintf(bwChar, STD_ARRAY_SIZE(bwChar), "%u", rRepInfo.getBandwidth());
      ReplaceIdentifierwithValueInUrl(dstBuffer, "$Bandwidth$",
                                      bwChar, dstBufferSize);
    }

    if(IsTemplateTagPresent(dstBuffer, "$Bandwidth%"))
    {
      FormatandReplaceIdentifierwithValueInUrl(
         dstBuffer, "$Bandwidth%", (int)rRepInfo.getBandwidth(), dstBufferSize);
    }

    if(IsTemplateTagPresent(dstBuffer, "$Time$"))
    {
      //replace $Time$ with segmenttimeline@t
      char segmentStartTimeChar[MAX_TEMPLATE_ID_TIMELINE_LEN];
      segmentStartTimeChar[0] = '\0';
      std_strlprintf(segmentStartTimeChar,STD_ARRAY_SIZE(segmentStartTimeChar), "%lld", startNumberOrTime);
      ReplaceIdentifierwithValueInUrl(dstBuffer, "$Time$",
                                      segmentStartTimeChar, dstBufferSize);

    }

    if(IsTemplateTagPresent(dstBuffer, "$Time%"))
    {
      FormatandReplaceIdentifierwithValueInUrl(dstBuffer, "$Time%", (int)startNumberOrTime, dstBufferSize);
    }

    if(IsTemplateTagPresent(dstBuffer, "$Number$"))
    {
      //replace $Number$ with segment index
      char segIndexChar[MAX_TEMPLATE_ID_NUMBER_LEN];
      segIndexChar[0] = '\0';
      std_strlprintf(segIndexChar, STD_ARRAY_SIZE(segIndexChar), "%lld", startNumberOrTime);
      ReplaceIdentifierwithValueInUrl(dstBuffer, "$Number$",segIndexChar, dstBufferSize);
    }

    if(IsTemplateTagPresent(dstBuffer, "$Number%"))
    {
      //Format & replace $Number% with segment index
      FormatandReplaceIdentifierwithValueInUrl(
         dstBuffer, "$Number%",(int)startNumberOrTime, dstBufferSize);
    }
  }
}

/**
 * Return the number of occurances of of mpd template tag
 * 'urlTemplate'.
 */
int MPDParser::GetNumOccurencesOfTemplateTag(char *urlTemplate, const char *tag)
{
  int count = 0;

  if(urlTemplate && tag)
  {
    const int tagLen = (int)std_strlen(tag);
    const char *urlTemplateEnd = urlTemplate + std_strlen(urlTemplate);
    char *pos = std_strstr(urlTemplate , tag);

    while(pos && urlTemplate < urlTemplateEnd)
    {
      ++count;
      urlTemplate = pos + tagLen;
      pos = std_strstr(urlTemplate , tag);
    }
  }

  return count;
}


/* This function computes first available segment start time and end time for the
* representation.
* When segment information for representation is of type segment template without
* timeline SegmentFuncTemplate path API's are used for available segment times
* calculations where segment info is computed on fly
 */
HTTPDownloadStatus RepresentationInfo::SegmentFuncTemplate::GetFirstAvailableSegmentTimeForRepresentation
(
  MPDParser* pMPDParser,
  PeriodInfo* pPeriodInfo,
  RepresentationInfo* pRepInfo,
  double nCurrMSeconds,
  bool bStartOfPlayback,
  uint64& nFirstAvailableSegmentStartTime,
  uint64& nFirstAvailableSegmentEndTime
)
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  if(NULL == pRepInfo->GetSegmentTemplate())
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetFirstAvailableSegmentTimeForRepresentation: Unexpected error. Null segmenttemplate");
    status = video::HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    int64 nEarliestOffsetStartNumber = -1;
    int64 nLatestOffsetStartNumber = -1;
    double segDuration = CalculateSegmentDurationFromStoredTemplate(pRepInfo);

    if(segDuration > 0)
    {
      if(!pMPDParser->IsLive())
      {
        int64 nPeriodDurationMSecs = (int64)(1000 * pPeriodInfo->getDuration());
        nEarliestOffsetStartNumber = 0;
        nLatestOffsetStartNumber =
          (nPeriodDurationMSecs % (int64)segDuration == 0
          ? ((int64)((double)nPeriodDurationMSecs / segDuration) - 1)
          : ((int64)((double)nPeriodDurationMSecs / segDuration)));
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
      else
      {
        uint32 tsbToUse = bStartOfPlayback ? pMPDParser->GetTsbToUseAtStartUp() : pMPDParser->GetTimeShiftBufferDepth()*1000;

        status = GetAvailabilityBoundsForSegDurationFromTemplate(
          pMPDParser,
          nEarliestOffsetStartNumber, nLatestOffsetStartNumber,
          pPeriodInfo,
          segDuration,
          nCurrMSeconds,
          pRepInfo->GetMinUpdatePeriod(),
          (double)tsbToUse);
      }

      if(HTTPCommon::HTTPDL_SUCCESS == status)
      {
        int64 tmpMinStartTime = nEarliestOffsetStartNumber * (int64)segDuration;
        int64 tmpMaxEndTime = (1 + nLatestOffsetStartNumber) * (int64)segDuration;
        nFirstAvailableSegmentStartTime = (uint64)tmpMinStartTime;
        nFirstAvailableSegmentEndTime = nFirstAvailableSegmentStartTime + (int64)segDuration;
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "GetFirstAvailableSegmentTimeForRepresentation: segDuration is zero");
      status = video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }

  return status;
}


/* This function computes last available segment start time and end time for the
* representation.
* When segment information for representation is of type segment template without
* timeline SegmentFuncTemplate path API's are used for available segment times
* calculations where segment info is computed on fly
*/
HTTPDownloadStatus RepresentationInfo::SegmentFuncTemplate::GetLastAvailableSegmentTimeForRepresentation
(
  MPDParser *pMPDParser,
  PeriodInfo* pPeriodInfo,
  RepresentationInfo* pRepInfo,
  double nCurrMSeconds,
  uint64& nLastAvailableSegmentStartTime,
  uint64& nLastAvailableSegmentEndTime
)
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  if(NULL == pRepInfo->GetSegmentTemplate())
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetLastAvailableSegmentTimeForRepresentation: Unexpected error. Null segmenttemplate");
    status = video::HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    int64 nEarliestOffsetStartNumber = -1;
    int64 nLatestOffsetStartNumber = -1;

    if(!pMPDParser->IsLive())
    {
      int64 periodDurationMs = (int64)(1000 * pPeriodInfo->getDuration());
      int64 segDurationMSecs = (int64)CalculateSegmentDurationFromStoredTemplate(pRepInfo);

      if(segDurationMSecs > 0)
      {
        int numSegments = (0 == periodDurationMs % segDurationMSecs
          ? (int)(periodDurationMs / segDurationMSecs)
          : 1 + (int)(periodDurationMs / segDurationMSecs));

        nLastAvailableSegmentEndTime = (uint64) (numSegments * segDurationMSecs);
        nLastAvailableSegmentStartTime = nLastAvailableSegmentEndTime - (uint64) segDurationMSecs;
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Invalid segDurationMsecs 0");
      }
    }
    else
    {
      double segDuration = CalculateSegmentDurationFromStoredTemplate(pRepInfo);

      if(segDuration > 0)
      {
        status = GetAvailabilityBoundsForSegDurationFromTemplate(
          pMPDParser,
          nEarliestOffsetStartNumber, nLatestOffsetStartNumber,
          pPeriodInfo,
          segDuration,
          nCurrMSeconds,
          pRepInfo->GetMinUpdatePeriod(),
          (double)pMPDParser->GetTimeShiftBufferDepth()*1000);

        if(nLatestOffsetStartNumber >= 0)
        {
          nLastAvailableSegmentEndTime = (uint64)segDuration * (1+nLatestOffsetStartNumber);
          nLastAvailableSegmentStartTime = nLastAvailableSegmentEndTime - (uint64)segDuration;
          status = HTTPCommon::HTTPDL_SUCCESS;
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "GetLastAvailableSegmentTimeForRepresentation: segDuration is zero");
      }
    }
  }

  return status;
}


bool RepresentationInfo::SegmentFuncTemplate::GenerateSegmentInfoFromTemplate(
   MPDParser* pMPDParser, SegmentInfo* pSegmentInfo, RepresentationInfo* pRepInfo, PeriodInfo* pPeriodInfo, uint64 nKey)
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo->GetSegmentTemplate(), HTTPCommon::HTTPDL_ERROR_ABORT);
  bool bOk = false;

  MPD *mpd = pMPDParser->mpd;
  uint32 segmentKey = (uint32)((nKey & MPD_SEGMENT_MASK));

  uint32 startNumber = pRepInfo->GetSegmentTemplate()->GetStartNumber();
  double segDuration = CalculateSegmentDurationFromStoredTemplate(pRepInfo);

  if(segDuration > 0)
  {
    uint32 segmentIndex = segmentKey;
    double segmentStartTime = ((double)(segmentIndex))*segDuration;

    char *mediaUrlTemp = pRepInfo->GetSegmentTemplate()->GetMediaTemplate();
    char *mediaTemplate = NULL;
    int origMediaUrlLen = 0;
    if (mediaUrlTemp)
    {
      origMediaUrlLen = pMPDParser->GetMaxBufferSizeForStringTemplate(*pRepInfo, mediaUrlTemp);
      mediaTemplate = (char *)QTV_Malloc(origMediaUrlLen * sizeof(char));
    }

    bOk = true;

    if(mediaTemplate)
    {
      pMPDParser->SetBufferForStringTemplate(*pRepInfo, mediaUrlTemp, mediaTemplate, origMediaUrlLen, segmentIndex + startNumber);

      char *indexUrlTemp = pRepInfo->GetSegmentTemplate()->GetIndexTemplate();
      char *indexTemplate = NULL;
      bool isIndexURLPresent = false;
      int origIndexUrlLen = 0;
      if (indexUrlTemp)
      {
        origIndexUrlLen = (int)std_strlen(indexUrlTemp)+ 1;
        indexTemplate = (char *)QTV_Malloc(origIndexUrlLen * 2);
      }
      if (indexTemplate)
      {
        pMPDParser->SetBufferForStringTemplate(
           *pRepInfo, indexUrlTemp, indexTemplate, origIndexUrlLen, segmentIndex + startNumber);
        isIndexURLPresent = true;
      }
      else
      {
        indexTemplate = mediaTemplate;
      }

      char *initURLl = pRepInfo->GetSegmentTemplate()->GetInitialisationTemplate();
      char *initURL2 = NULL;
      char *initRange = NULL;

      if(pRepInfo->GetSegmentTemplate()->GetInitialisation())
      {
        initURL2  = pRepInfo->GetSegmentTemplate()->GetInitialisation()->sourceURL;
        initRange = pRepInfo->GetSegmentTemplate()->GetInitialisation()->range;
      }

      bool isInitURLPresent = (initURLl || initURL2) ? true : false;
      bool indexRangeExact  = pRepInfo->GetSegmentTemplate()->GetIndexRangeExact();
      char *indexRange      = pRepInfo->GetSegmentTemplate()->GetIndexRange();

      bOk = pSegmentInfo->SetInfoURL(
          pRepInfo->GetBaseURL(), mediaTemplate, NULL /*mediaRange*/,
         indexTemplate, isIndexURLPresent, indexRange, indexRangeExact,
         isInitURLPresent, initRange, segDuration, nKey /*segmentKey | ((uint64)segmentIndex - 1) */,
         segmentStartTime);

      if (bOk && mpd && mpd->IsLive())
      {
        pSegmentInfo->SetAvailabilityTime(mpd->GetAvailabilityStartTime()+
                                          (double)pPeriodInfo->getStartTime()+ segDuration +
                                         segmentStartTime);
      }

      QTV_Free(mediaTemplate);
      if(isIndexURLPresent && indexTemplate)
      {
        QTV_Free(indexTemplate);
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "GenerateSegmentInfoFromTemplate segDuration zero");
  }

  return bOk;
}

HTTPDownloadStatus RepresentationInfo::SegmentFuncTemplate::GetAllSegmentsForRepresentationRange(
  MPDParser* pMPDParser, double currMSeconds, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, uint64* pSegmentInfo, uint32& numSegments,
  uint64 segStartTime,uint64 segEndTime, double &firstAvailableSegmentStartTime)
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  uint32 saveNumSegments = numSegments;
  numSegments = 0;
  if(pRepInfo->GetSegmentTemplate())
  {
    uint32 startNumber = pRepInfo->GetSegmentTemplate()->GetStartNumber();
    double segDuration = CalculateSegmentDurationFromStoredTemplate(pRepInfo);

    if(segDuration > 0)
    {
      double latestSegOffset = 0.0;
      int64 nEarliestOffsetStartNumber = -1;
      int64 nLatestOffsetStartNumber = -1;

      if(!pMPDParser->IsLive())
      {
        status = HTTPCommon::HTTPDL_SUCCESS;
        nEarliestOffsetStartNumber = 0;
        // subtract 1ms from periodDuration.
        nLatestOffsetStartNumber = (int64)((1000.0 * pPeriodInfo->getDuration() - 1) / segDuration);
      }
      else
      {
        status = GetAvailabilityBoundsForSegDurationFromTemplate(
          pMPDParser,
          nEarliestOffsetStartNumber, nLatestOffsetStartNumber,
          pPeriodInfo,
          segDuration,
          currMSeconds,
          pRepInfo->GetMinUpdatePeriod(),
          pRepInfo->GetTimeShiftBufferDepth()*1000.0);
      }

      if(HTTPCommon::HTTPDL_SUCCESS == status)
      {
        double earliestSegStartTime = (double)nEarliestOffsetStartNumber * segDuration;
        double latestSegEndTime =  ((double)nLatestOffsetStartNumber + 1) * segDuration;

        firstAvailableSegmentStartTime = earliestSegStartTime;

        if(segStartTime >= latestSegEndTime)
        {
          status = HTTPCommon::HTTPDL_WAITING;
        }
        else
        {
          if (segEndTime <= earliestSegStartTime)
          {
            // there are segments available but the request is too late.
            status = HTTPCommon::HTTPDL_TIMEOUT;
          }
          else
          {
            // there are available segments corresponding to segStartTime
            // and segEndTime. (segStartTime < latestSegEndTime &&
            // segEndTime >= earliestSegStartTime) should always be true.

            // Adjust segStartTime, segEndTime to what will be returned to client.
            if(segStartTime < earliestSegStartTime)
            {
              segStartTime = (uint64)earliestSegStartTime;
            }

            if(segEndTime > latestSegEndTime)
            {
              segEndTime = (uint64)latestSegEndTime;
            }

            // returnFirstSegIdx is correct whether or not segStartTime
            // lines up with the the template start time.
            uint32 returnFirstSegIdx = (uint32)((double)segStartTime / segDuration);

            uint32 returnLastSegIdx = (uint32)((double)segEndTime / segDuration);


            if ((uint64)segEndTime % (uint64)segDuration != 0)
            {
              // segEndTime does not line up with start of a new segment in template.
              returnLastSegIdx += 1;

            }

            numSegments = 0;

            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
              "DEBUG: Template:offset-startNos %lld - %lld",
              nEarliestOffsetStartNumber, nLatestOffsetStartNumber);

            if(returnFirstSegIdx <= returnLastSegIdx)
            {
              numSegments = 1 + returnLastSegIdx - returnFirstSegIdx;
              if(saveNumSegments < numSegments)
              {
                status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
              }

              uint64 nSegmentOffset = returnFirstSegIdx;

              for(uint32 i = returnFirstSegIdx;
                i <= returnLastSegIdx && (i - returnFirstSegIdx) < saveNumSegments;
                ++i)
              {
                uint64 mpdKey = 0;
                uint64 repKey = pRepInfo->getKey();
                uint64 tmpSegKey = nSegmentOffset ;
                mpdKey = repKey | tmpSegKey;
                pSegmentInfo[i - returnFirstSegIdx] = mpdKey;
              }
            }
          }
        }
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "GetAllSegmentsForRepresentationRange SegDuration zero");
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetAllSegmentsForRepresentationRange: null segment template");
  }

  return status;

}



HTTPDownloadStatus RepresentationInfo::SegmentFuncDefault::GetAllSegmentsForRepresentationRange(
  MPDParser* pMPDParser, double currMSeconds, PeriodInfo* pPeriodInfo, RepresentationInfo* pRepInfo, uint64* pSegmentInfo, uint32& numSegments,
  uint64 segStartTime,uint64 segEndTime, double &firstAvailableSegmentStartTime)
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pRepInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  MPD *mpd = pMPDParser->mpd;
  QTV_NULL_PTR_CHECK(mpd,HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status= HTTPCommon::HTTPDL_ERROR_ABORT;
  uint32 nNumSegments = 0,size=0;
  SegmentInfo* pSegment = pRepInfo->getSegmentInfo(nNumSegments,size);
  double availabilityMSeconds = (mpd ? mpd->GetAvailabilityStartTime() : 0.0);
  uint64 periodStartTime = pPeriodInfo->getStartTime();
  if(pSegment)
  {
    int firstAvailableSegmentIndex = -1;
    int lastAvailableSegmentIndex = -1;
    uint32 numberOfAvailableSegments = 0;

    int segmentCounter = 0, copyCounter = 0;

    if(!pMPDParser->IsLive())
    {
      firstAvailableSegmentStartTime = pSegment[0].getStartTime();
      firstAvailableSegmentIndex = 0;
      numberOfAvailableSegments = nNumSegments;
      lastAvailableSegmentIndex = (int)(nNumSegments - 1);
    }
    else
    {
      pMPDParser->GetFirstAndLastAvailableSegmentInfo(
        firstAvailableSegmentIndex, lastAvailableSegmentIndex, numberOfAvailableSegments,
        pSegment, nNumSegments, periodStartTime, currMSeconds, pRepInfo->GetMinUpdatePeriod(), pRepInfo->GetTimeShiftBufferDepth());

      if(firstAvailableSegmentIndex >= 0 && (uint32)firstAvailableSegmentIndex < nNumSegments)
      {
        firstAvailableSegmentStartTime = pSegment[firstAvailableSegmentIndex].getStartTime();
      }
    }

    if(firstAvailableSegmentIndex >= 0 && lastAvailableSegmentIndex >= 0)
    {
      //Request end time falls below the start time of the first available segment.
      //Hence this is a complete failure.
      if(segEndTime <= firstAvailableSegmentStartTime)
      {
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Segment requested end range %d is past availability time of %d",
          (int)segEndTime, (int)firstAvailableSegmentStartTime );
        status = HTTPCommon::HTTPDL_TIMEOUT;
      }
      else if(segStartTime >= (pSegment[lastAvailableSegmentIndex].getStartTime() + pSegment[lastAvailableSegmentIndex].getDuration())&& (pMPDParser->IsLive()))
      {
        //Request start time falls beyond the end time of the last available segment. Hence this is waiting.
        status = HTTPCommon::HTTPDL_WAITING;
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "Segment with start time of %d is not available yet for the live case with availability time of %d",
          (int)segStartTime, (int)firstAvailableSegmentStartTime);
      }
      else
      {
        //check for segments within the specified range only after the first available segment
        for(int i = firstAvailableSegmentIndex; i < firstAvailableSegmentIndex + (int)numberOfAvailableSegments; i++)
        {
          if(segStartTime < (uint64)(pSegment[i].getStartTime() + pSegment[i].getDuration()) &&
            segEndTime > (uint64)pSegment[i].getStartTime())
          {
            if(pSegmentInfo && ((uint32)copyCounter < numSegments))
            {
              pSegmentInfo[copyCounter] = pSegment[i].getKey();
              copyCounter++;
            }
            segmentCounter++;
          }
        }//for(int i = firstAvailableSegmentKey; i < nNumSegments; i++)
      }
    }//if(firstAvailableSegmentIndex >= 0)
    //Current time is beyond current period duration. Hence this is data end.
    else
    {
      status = HTTPCommon::HTTPDL_DATA_END;

      // Check if its an early request
      if (nNumSegments > 0)
      {
        double firstSegmentStartTime = pSegment[0].getStartTime();

        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "GetAllSegmentsForRepresentationRange. Check Reuqest to early earliestSegmentStartTime %u > currMSeconds %u",
          (uint32)((int64)firstSegmentStartTime % 0x80000000),
          (uint32)((int64)currMSeconds % 0x80000000));

        if (segStartTime < firstSegmentStartTime)
        {
          // if the requested start time is earlier than the first segment
          // start time, then don't care about whether the first segment is
          // available or not, as either way, error needs to be reported so
          // that QSM receives a failure and quickly issues another request
          // so that the requst start time catches up with the first segment
          // start time, and 'availability' can be checked then.
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "RequestedSegmentStart %u < firstSegmentStartTime %u",
            (uint32)((int64)(segStartTime) % 0x80000000), (uint32)((int64)(firstSegmentStartTime)% 0x80000000));
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
        else
        {
          // the requested start time is greater than the first avaialble start
          // time. But the first segment and all others aren't 'available', else
          // it would have found an available segment and not entered this code segment.
          if ((availabilityMSeconds + (double)periodStartTime + firstSegmentStartTime + pSegment[0].getDuration()) > currMSeconds)
          {
            // the first segment is not yet available and the requested start
            // time is furthur ahead than the first segment start time.
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "GetAllSegmentsForRepresentationRange. Request too early");
            status = HTTPCommon::HTTPDL_WAITING;
          }
          else
          {
            // None of the segments are available and the requestedStart is greater
            // than all segments and the all the segments are too late to be played.
            // Then, if it is known that the last segment in the mpd corresponds to
            // the last segment in the playout, then this is end of stream from mpd
            // perspective, else wait for an mpd update which may result in segments
            // that are available.
            double lastSegmentEndTime =
              pSegment[nNumSegments - 1].getStartTime() +
              pSegment[nNumSegments - 1].getDuration();

            double periodDuration = 1000 * pPeriodInfo->getDuration();

            if (periodDuration > 0.0 && lastSegmentEndTime >= periodDuration)
            {
              // end of period is known based on period duration.
              uint32 period_index = (uint32)((pRepInfo->getKey() & MPD_PERIOD_MASK)>>MPD_PERIOD_SHIFT_COUNT);
              QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                "LastSegment in period idx %d as lastSegmentEndTime %d (%d+%d)",
                period_index, (int)lastSegmentEndTime,
                (int)pSegment[nNumSegments - 1].getStartTime(),
                (int)pSegment[nNumSegments - 1].getDuration());
              status = HTTPCommon::HTTPDL_DATA_END;
            }
            else
            {
              status = HTTPCommon::HTTPDL_WAITING;
            }
          }
        }
      }

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "GetAllSegmentsForRepresentationRange: request too late or early status %d", status);
    }

    if (segmentCounter > 0)
    {
      if (pSegmentInfo)
      {
        status = (copyCounter < segmentCounter) ? HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER : HTTPCommon::HTTPDL_SUCCESS;
      }
      else
      {
        status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
      }
    }
    //Request start time is beyond the end time of the last segment in
    //the current period. Hence this is data end.
    else if(nNumSegments >= 1)
    {
      if(segStartTime >= (uint64)(pSegment[nNumSegments-1].getStartTime() + pSegment[nNumSegments-1].getDuration()) &&
        pMPDParser->m_bEndReached)
      {
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Request start time %d is beyond the end time of the last segment %d + duration %d",
          (int)segStartTime, (int)pSegment[nNumSegments-1].getStartTime(), (int)(pSegment[nNumSegments-1].getDuration()));
        status = HTTPCommon::HTTPDL_DATA_END;
      }
    }
    numSegments = segmentCounter;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "%u segments are available within the given range", numSegments);

  }
  else
  {
    // if segment array was not created, then non of the segments
    // fall in the availability window. If current time is past
    // the availability window, then eos is returned higher up.
    // If not, then we are waiting for mpd update.
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
      "Waiting for mpd update");
    status = video::HTTPCommon::HTTPDL_WAITING;
  }

  return status;
}

/**
 * This finds the earliestAvailTime and latestAvailTime for a
 * period. There are 3 cases:
 *  (i) The availablity window is partially or wholly within the
 *  period.
 *  (ii) The availability window is before period start.
 *  (iii) The availability window is after the upperboundtime
 *        with the periodDuration unknown.
 *  (iv) The availability window is after the period end (period
 *       duration is known.
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS   case (i) earliestAvailTime and
 *                      latestAvailTime are valid.
 *  HTTPDL_WAITING  cases (ii) and (iii) as the a retry is
 *                  needed.
 *  HTTPDL_DATA_END   case (iv) no point retrying. EOS to QSM.
 */
HTTPDownloadStatus RepresentationInfo::SegmentFuncTemplate::GetAvailabilityBoundsForSegDurationFromTemplate(
  MPDParser* pMPDParser,
  int64& firstAvailableSegmentIndex,
  int64& lastAvailableSegmentIndex,
  PeriodInfo* pPeriodInfo,
  double segDuration, // ms
  double currMSeconds,
  double minUpdatePeriod,
  double tsbToUse)
{
  QTV_NULL_PTR_CHECK(pMPDParser, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(pPeriodInfo, HTTPCommon::HTTPDL_ERROR_ABORT);

  if(!(segDuration > 0))
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "SegDuration zero!!!");
    return HTTPCommon::HTTPDL_ERROR_ABORT;
  }

  MPD *mpd = pMPDParser->mpd;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  firstAvailableSegmentIndex = -1;
  lastAvailableSegmentIndex = -1;
  QTV_NULL_PTR_CHECK(mpd, HTTPCommon::HTTPDL_ERROR_ABORT);

  // Lower bound time to restrict accessible segments taking into account
  // the time shift buffer.
  double lowerBoundTime = currMSeconds - tsbToUse;
  MM_Time_DateTime fetchTime = pMPDParser->GetFetchTime();

  // Upper bound time to restrict accessible segments taking into account
  // current time and mpd checkTime as defined in the spec under the title
  // "Media Segment list restrictions"
  double upperboundTime = currMSeconds;
  if (minUpdatePeriod > 0.0)
  {
    double checkTimeForMPD = (StreamSourceTimeUtils::ConvertSysTimeToMSec(fetchTime) +
                              1000.0 * minUpdatePeriod);
    upperboundTime = QTV_MIN(checkTimeForMPD, currMSeconds);
  }

  double availabilityMSeconds = mpd ? mpd->GetAvailabilityStartTime() : 0.0;
  double availPlusPeriodStartPlusSegDuration = availabilityMSeconds + (double)pPeriodInfo->getStartTime() + segDuration;

  if(upperboundTime < availPlusPeriodStartPlusSegDuration)
  {
    // case (ii)
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
    double periodDuration = pPeriodInfo->getDuration();
    double absolutePeriodEndtime = 0.0;

    if(periodDuration > 0)
    {
      absolutePeriodEndtime = availPlusPeriodStartPlusSegDuration + 1000.0 * periodDuration;
    }
    else
    {
      // refresh case and this is the last period in the mpd.
      absolutePeriodEndtime = currMSeconds + 1000.0 * minUpdatePeriod;
    }

    if(lowerBoundTime >= absolutePeriodEndtime)
    {
      // case (iv)
      status = HTTPCommon::HTTPDL_DATA_END;
    }
    else
    {
      // some part of the availability falls within the limits of the period.
      // readjust lowerBoundTime and upperBoundTime so that lowerBoundTime is
      // not before the period start and upperBoundTime is not after the
      // periodEndTime.
      // Consider segments that have some part of the segment for which
      // lowerBoundTime <= t < upperBoundTime
      if(lowerBoundTime < availPlusPeriodStartPlusSegDuration)
      {
        lowerBoundTime = availPlusPeriodStartPlusSegDuration;
      }

      if(upperboundTime >= absolutePeriodEndtime)
      {
        upperboundTime = absolutePeriodEndtime - 1; // subtracting 1ms so that the end time is not included.
      }

      // now find the segment indexes corresponding to lowerBoundTime and
      // upperBoundTime.
      double offsetOfLowerBoundFromPeriodStart =
        lowerBoundTime - availPlusPeriodStartPlusSegDuration;

      double offsetOfUpperBoundFromPeriodStart =
        offsetOfLowerBoundFromPeriodStart + (upperboundTime - lowerBoundTime);

      if(offsetOfLowerBoundFromPeriodStart >= 0 && offsetOfUpperBoundFromPeriodStart >= 0)
      {
        firstAvailableSegmentIndex = (int64)(offsetOfLowerBoundFromPeriodStart / segDuration);
        lastAvailableSegmentIndex = (int64)(offsetOfUpperBoundFromPeriodStart / segDuration);

        status = HTTPCommon::HTTPDL_SUCCESS;
      }
      else
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "offsetOfLowerBoundFromPeriodStart %llu or offsetOfUpperBoundFromPeriodStart "
          "%llu is < 0", (uint64)offsetOfLowerBoundFromPeriodStart, (uint64)offsetOfUpperBoundFromPeriodStart);
      }

    }
  }

  return status;
}

/**
 * Calculate the segment duration in millisecs from the duration
 * and timescale stored in the segmentTemplate structure.
 */
double RepresentationInfo::SegmentFuncTemplate::CalculateSegmentDurationFromStoredTemplate(RepresentationInfo* pRepInfo)
{
  double segDuration = 0.0;

  if(pRepInfo->GetSegmentTemplate())
  {
    segDuration = 1000.0 * (double)pRepInfo->GetSegmentTemplate()->GetDuration();
    uint32 nTimescale = pRepInfo->GetSegmentTemplate()->GetTimeScale();
    if(nTimescale > 0 && nTimescale < MAX_UINT32_VAL)
    {
      segDuration = segDuration / (double)nTimescale;
    }
  }

  return segDuration;
}


RepresentationInfo::SegmentFuncTemplate::Type RepresentationInfo::SegmentFuncTemplate::GetType() const
{
  return m_Type;
}

RepresentationInfo::SegmentFuncDefault::Type RepresentationInfo::SegmentFuncDefault::GetType() const
{
  return m_Type;
}

}
