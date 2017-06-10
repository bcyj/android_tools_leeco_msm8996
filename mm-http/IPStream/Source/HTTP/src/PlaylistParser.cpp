/************************************************************************* */
/**
 * PlaylistParser.cpp
 * @brief implementation of PlaylistParser.
 *  It parses the playlist file and stores the information about media files which
 *  is used by AdaptiveStreamHelper
 *
 * COPYRIGHT 2011-2013 QUALCOMM Technologies, Inc.
 * All rights reserved. QUALCOMM Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/PlaylistParser.cpp#72 $
$DateTime: 2013/12/03 01:05:22 $
$Change: 4888051 $

========================================================================== */
/* =======================================================================
**               Include files for PlaylistParser.cpp
** ======================================================================= */
//TODO: Add function headers and comments
#include "PlaylistParser.h"
#include <ctype.h>

namespace video {

const char* ContentProtectionType::MARLIN_SCHEME_URI = "urn:uuid:5E629AF5-38DA-4063-8977-97FFBD9902D4";
const char* ContentProtectionType::MARLIN_CONTENTIDS = "mas:MarlinContentIds";
const char* ContentProtectionType::MARLIN_CONTENTID = "mas:MarlinContentId";
const char* ContentProtectionType::MARLIN_FORMAT_VERSION = "mas:FormatVersion";

const char* ContentProtectionType::PLAYREADY_SCHEME_URI = "urn:uuid:9a04f079-9840-4286-ab92-e65be0885f95";
const char* ContentProtectionType::CENC_MP4_SCHEME_URI  = "urn:mpeg:dash:mp4protection:2011";
const char* ContentProtectionType::CENC_MP2TS_SCHEME_URI = "urn:mpeg:dash:13818:1:CA_descriptor:2011";




/* @brief - Playlist parser constructor
 * @param -Scheduler
 */
PlaylistParser::PlaylistParser
(
   HTTPSessionInfo& pSessionInfo,
   Scheduler& pScheduler
)
:m_pScheduler(pScheduler),
 m_sessionInfo(pSessionInfo),
 m_nNumPrograms(0),
 current_selected_program(0),
 current_selected_representation(0),
 current_selected_period(0),
 m_url(NULL),
 m_bAbortSet(false),
 m_bRepsAvailable(false),
 m_pDownloader(NULL),
 m_pNamespaceKey(NULL),
 m_pNamespaceVal(NULL),
 m_MpdProfile(DASH_PROFILE_NO_PROFILE),
 m_MpdProfileStr(NULL)
{
}
/* @brief - PlaylistParser Destructor
 */
PlaylistParser::~PlaylistParser()
{
  if(m_MpdProfileStr)
  {
    QTV_Free(m_MpdProfileStr);
    m_MpdProfileStr = NULL;
  }
  if(m_url)
  {
    QTV_Free(m_url);
    m_url = NULL;
  }
}
/* @brief - Parses Codec Information and populates the codec_info with
* all the codecs
* @param - str - string containing codecs in the format "[format][,format]*"
* where each format specifies a media sample type that is present in a
media file in the Playlist file. It is parsed to fill the out param codec_info
* @param[out] - codec_info
*/
bool PlaylistParser::ParseCodecInfo(char* str, Codecs* codec_info)
{
  int majorTypes=0;
  return ParseCodecInfo(str,codec_info,majorTypes);

}
/* @brief - Parses Codec Information and populates the codec_info with
* all the codecs
* @param - str - string containing codecs in the format "[format][,format]*"
* where each format specifies a media sample type that is present in a
media file in the Playlist file. It is parsed to fill the out param codec_info
* @param[out] - codec_info
*/
bool PlaylistParser::ParseCodecInfo(char* str, Codecs* codec_info,int& majorTypes)
{
  bool bOk=false;
  majorTypes = 0;
  if(str==NULL||codec_info==NULL)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,"codec string is null");
  }
  else
  {
    int len;
    char *pos = std_strchr(str,'"');
    if(pos != 0)
    {
      len = int(pos - str);
    }
    else
    {
      len = -1;
    }
    if (len >=0)
    {
      str=std_strchr(str,'"');
    }
    if (str)
    {
      len=(int)std_strlen(str);
    }
    //If number of codecs passed is 0. Populate the numcodecs. So that
    //required memory can be allocated by caller.
    if(codec_info->numcodecs==0)
    {
      int numcodecs = 1;
      for(int i=0;i<len && str;i++)
      {
        //Different codecs are seperated by ","
        if(str[i]==',')
          numcodecs++;
      }
      codec_info->numcodecs = numcodecs;
      bOk=true;
    }
    else if(codec_info->mcodecs==NULL)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,"codec_info->mcodecs is null");
    }
    else
    {
      bOk=true;
      if (str && str[0] == '"')
      {
        str=skip_whitespace(str+1);
      }
      else
      {
        str=skip_whitespace(str);
      }
      //Find the location of "," to find where the next codec starts
      char *tmp= str ? std_strchr(str,',') : NULL;
      int index=0;
      while(str)
      {
        //For avc codec the format string is either "avc1.[profile].[level]"
        //or "avc1.xxyyzz" where xx denotes profile number zz denotes level and
        // yy gives the compatible profiles. Right now only using xx and yy.
        //
        if(!std_strncmp(str,"avc1",4))
        {
          codec_info->mcodecs[index].majorType = MJ_TYPE_VIDEO;
          codec_info->mcodecs[index].minorType = MN_TYPE_AVC;
          majorTypes|=2;
          uint32 profile=0,level=0;
          if(std_strlen(str) >= 10)
          {
            //Checking if the format is "avc1.[profile].[level]"
            // or "avc1.xxyyzz"
            int nErr;
            if(*(str+7)=='.')
            {
               const char *temp = str+7;
               profile = std_scanul(str+5, 10, &temp, &nErr);
               temp = str+10;
               level = std_scanul(str+8, 10, &temp, &nErr);
            }
            else
            {
               const char *temp = str+7;
               profile = std_scanul(str+5, 16, &temp, &nErr);
               temp = str+11;
               level = std_scanul(str+9, 16, &temp, &nErr);
            }
          }

          codec_info->mcodecs[index].profile=(int)profile;
          codec_info->mcodecs[index].level=(int)level;
        }
        else if(!std_strncmp(str,"hvc1",4) || !std_strncmp(str,"hev1",4))
        {
          codec_info->mcodecs[index].majorType = MJ_TYPE_VIDEO;
          codec_info->mcodecs[index].minorType = MN_TYPE_HVC;
          majorTypes|=2;
        }
        //mp4v identifies mpeg4 video
        //mp4v.20 is mpeg4 part 2
        //mp4v.61 high profile
        //mp4v.64 main profile
        else if(!std_strncmp(str,"mp4v",4))
        {
          codec_info->mcodecs[index].majorType = MJ_TYPE_VIDEO;
          majorTypes|=2;
          if(!std_strncmp(str,"mp4v.20",7) ||
             !std_strncmp(str,"mp4v.61",7) ||
             !std_strncmp(str,"mp4v.64",7))
          {
            codec_info->mcodecs[index].minorType = MN_TYPE_MPEG4;
          }
          else
          {
             codec_info->mcodecs[index].minorType = MN_TYPE_UNKNOWN;
          }
          codec_info->mcodecs[index].profile=0;
          codec_info->mcodecs[index].level=0;
        }
        else if((!std_strncmp(str,"ec-3",4)) || (!std_strncmp(str,"ovrb",4)))
        {
          codec_info->mcodecs[index].majorType = MJ_TYPE_AUDIO;
          majorTypes|=1;
          if((!std_strncmp(str,"ovrb",4)))
          {
            codec_info->mcodecs[index].minorType = MN_TYPE_VORBIS;
          }
          else
          {
            codec_info->mcodecs[index].minorType = MN_TYPE_AC3;
          }
          codec_info->mcodecs[index].profile=0;
          codec_info->mcodecs[index].level=0;

        }
        //mp4a identifies mpeg4 audio codec
        //mp4a.40.2 is AAC_LC
        //mp4a.40.5 is HE_AAC
        //mp4a.40.32 is MP1
        //mp4a.40.33 is MP2
        //mp4a.40.34 is MP3
        else if(!std_strncmp(str,"mp4a",4))
        {
          majorTypes|=1;
          codec_info->mcodecs[index].majorType = MJ_TYPE_AUDIO;
          codec_info->mcodecs[index].minorType = MN_TYPE_UNKNOWN;
          codec_info->mcodecs[index].profile=0;
          codec_info->mcodecs[index].level=0;
          if(!std_strncmp(str,"mp4a.40",7))
          {
            if(std_strlen(str)>=9)
            {
              const char *object_type=str+8;
              uint32 objType=0;
              objType = atoi(object_type);
              if(objType==2)
              {
                codec_info->mcodecs[index].minorType = MN_TYPE_AAC_LC;
              }
              else if(objType==5)
              {
                codec_info->mcodecs[index].minorType = MN_TYPE_HE_AAC;
              }
              else if(objType==33)
              {
                codec_info->mcodecs[index].minorType = MN_TYPE_MP2;
              }
              else if(objType==34)
              {
                codec_info->mcodecs[index].minorType = MN_TYPE_MP3;
              }
            }
          }
          else if(!std_strncmp(str,"mp4a.0x40",9))
          {
            //TODO: Find out this. Temporary filling it with HE-AAC
            codec_info->mcodecs[index].minorType = MN_TYPE_HE_AAC;
          }

        }
        else if(!std_strncmp(str,"ttml",4) ||
                !std_strncmp(str,"smtt",4) ||
                !std_strncmp(str,"stpp",4))
        {
          codec_info->mcodecs[index].majorType = MJ_TYPE_TEXT;
          codec_info->mcodecs[index].minorType = MN_TYPE_TIMED_TEXT;
          majorTypes|=MJ_TYPE_TEXT;
          uint32 profile=0,level=0;
          codec_info->mcodecs[index].profile=(int)profile;
          codec_info->mcodecs[index].level=(int)level;
        }
        else if(*str && !isalpha(*str))
        {
          str++;
          continue;
        }
        else
        {
          //In any other case filling with unknown major and minor type
          codec_info->mcodecs[index].majorType = MJ_TYPE_UNKNOWN;
          codec_info->mcodecs[index].minorType = MN_TYPE_UNKNOWN;
          codec_info->mcodecs[index].profile=0;
          codec_info->mcodecs[index].level=0;
        }
        if(tmp)
        {
          str=skip_whitespace(tmp+1);
          tmp=std_strchr(str,',');
        }
        else
        {
          break;
        }
        index++;
      }
    }
  }
  return bOk;

}

/*
 * Identifying if Hevc codec is present in the Codec list
 */
bool PlaylistParser::IsHevcCodecPresent(Codecs* codec_info)
{
  bool  bOk = false;
  if (codec_info)
  {
    for (int codecIdx = 0; codecIdx < codec_info->numcodecs ; codecIdx++)
    {
      if (codec_info->mcodecs[codecIdx].minorType == MN_TYPE_HVC)
      {
        bOk = true;
        break;
      }
    }
  }
  return bOk;
}

/*
* @brief: Resolves a relative path wrt to a base uri. Populates segment url with the new
* absolute url. When called with segment url len 0, populates segment url len.
* @param: [in] base uri
* @param: [in] relative path to be resolved
* @param: [out] absolute url
* @param: [in/out] absolute url length
* @return: true if variant playlist else false
*/
bool PlaylistParser::ResolveURI(char *base_uri,char *relative_path,char *absolute_url,int& absolute_url_len)
{
  QTV_NULL_PTR_CHECK(base_uri,false);
  QTV_NULL_PTR_CHECK(relative_path,false);
  bool bOk = true;
  int temp_url_len= (int)std_strlen(base_uri)+1;
  char *temp_url=(char*)QTV_Malloc(temp_url_len);
  if(temp_url == NULL)
  {
    return false;
  }
  std_strlcpy(temp_url,base_uri,temp_url_len);
  int http_const_len=7;//Length of http://
  /** If base uri is http://a/b/c/test.html and relative path is //g it
  should be resolved as http://
  **/
  if(std_stribegins(relative_path,"//"))
  {
    int req_len=(int)std_strlen(relative_path)+(int)std_strlen("http:")+1;
    if(absolute_url_len >=req_len)
    {
      if(*(relative_path+2) == '/')
      {
        bOk = false;
      }
      else if(absolute_url)
      {
        std_strlcpy(absolute_url,"http://",absolute_url_len);
        std_strlcat(absolute_url,relative_path+2,absolute_url_len);
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,"PlaylistParser::ResolveURI absolute_url is null");
      }
    }
    else
    {
      absolute_url_len = req_len;
    }
  }
  /** If base uri is http://a/b/c/test.html and relative path is /g it
  should be resolved as http://a/g
  **/
  else if(relative_path[0]=='/')
  {
    char *firstslash =std_strchr(temp_url+http_const_len,'/');
    int firstslashindex = (firstslash ? ((int)std_strlen(temp_url)- (int)std_strlen(firstslash)) : 0);
    int req_len=firstslashindex + (int)std_strlen(relative_path)+1;
    if((absolute_url_len >=req_len) && (absolute_url))
    {
      char tmp=temp_url[firstslashindex];
      temp_url[firstslashindex]='\0';
      std_strlcpy(absolute_url,temp_url,absolute_url_len);
      std_strlcat(absolute_url,relative_path,absolute_url_len);
      temp_url[firstslashindex]=tmp;
    }
    else
    {
      absolute_url_len = req_len;
    }

  }
  /** If base uri is http://a/b/c/test.html and relative path is ./g it
  should be resolved as http://a/b/c/g. On the other hand if relative path
  is ../g it should be resolved as http://a/b/g. but if relative path is
  ../../../../g it will be resolved as http://a/../../g the ".." syntax
  cannot be used to change the authority component of a URI
  **/
  else if(relative_path[0]=='.')
  {

    char *lastslash =std_strrchr(temp_url,'/');
    int lastslashindex = (lastslash ? ((int)std_strlen(temp_url)-(int)std_strlen(lastslash)): 0);
    temp_url[lastslashindex]='\0';
    while(relative_path&& relative_path[0]=='.')
    {
      if(std_stribegins(relative_path,"./"))
      {
        relative_path+=2;
      }
      else if(std_stribegins(relative_path,"../"))
      {

        if((int)std_strlen(temp_url) > http_const_len &&
          std_strchr(temp_url+http_const_len,'/'))
        {
          lastslash =std_strrchr(temp_url,'/');
          lastslashindex = (int)std_strlen(temp_url)- (lastslash ? (int)std_strlen(lastslash) : 0);
          temp_url[lastslashindex]='\0';
          relative_path+=3;
        }
        else
        {
          break;
        }
      }
      else
      {
        break;
      }
    }
    int req_len=(relative_path ? ((int)std_strlen(temp_url)+(int)std_strlen(relative_path)+2) : 0 );
    if((req_len <= absolute_url_len) && (absolute_url))
    {
      std_strlcpy(absolute_url,temp_url,absolute_url_len);
      std_strlcat(absolute_url,"/",absolute_url_len);
      (relative_path ? std_strlcat(absolute_url,relative_path,absolute_url_len) : 0);
    }
    else
    {
      absolute_url_len = req_len;
    }
  }
  /** If base uri is http://a/b/c/test.html and relative path is g it
  should be resolved as http://a/b/c/g.
  **/
  else
  {
    char *lastslash =std_strrchr(temp_url,'/');
    int lastslashindex = (int)std_strlen(temp_url)-(lastslash ? (int)std_strlen(lastslash) : 0);
    temp_url[lastslashindex]='\0';
    int req_len=(int)std_strlen(temp_url)+(int)std_strlen(relative_path)+2;
    if((req_len <= absolute_url_len) && (absolute_url))
    {
      std_strlcpy(absolute_url,temp_url,absolute_url_len);
      std_strlcat(absolute_url,"/",absolute_url_len);
      std_strlcat(absolute_url,relative_path,absolute_url_len);
    }
    else
    {
      absolute_url_len = req_len;
    }
  }
  if(temp_url)
  {
    QTV_Free(temp_url);
    temp_url=NULL;
  }
  return bOk;

}

/* ***************************************************************************
Period Info Class
***************************************************************************
*/
/* @brief - PeriodInfo Constructor
*/
PeriodInfo::PeriodInfo()
:m_pRepresentationInfo(NULL),
m_pRepresentationGroup(NULL),
m_nNumRepGroups(0),
m_PeriodIdentifier(NULL),
m_nNumRepresentations(0),
m_nPeriodStartTime(0),
m_nPeriodDuration(0),
m_bLastPeriod(false),
m_nRepArraySize(0),
m_nRepGrpArrSize(0),
m_nPeriodKey(0),
m_pSegmentTemplate(NULL),
m_bSegmentTemplateFound(false),
m_pSegmentBase(NULL),
m_bSegmentBaseFound(false),
m_pSegmentList(NULL),
m_nNumSegURL(0),
m_bSegmentListFound(false)
{
}
/* @brief - PeriodInfo Destructor
*/
PeriodInfo::~PeriodInfo()
{
  if(m_pRepresentationInfo)
  {
    QTV_Delete_Array(m_pRepresentationInfo);
    m_pRepresentationInfo = NULL;
  }
  if(m_PeriodIdentifier)
  {
    QTV_Free(m_PeriodIdentifier);
    m_PeriodIdentifier = NULL;
  }
  if(m_pRepresentationGroup)
  {
    QTV_Delete_Array(m_pRepresentationGroup);
    m_pRepresentationGroup = NULL;
  }
  if (m_pSegmentBase)
  {
    QTV_Delete(m_pSegmentBase);
    m_pSegmentBase = NULL;
  }
  if (m_pSegmentList)
  {
    QTV_Delete(m_pSegmentList);
    m_pSegmentList = NULL;
  }
  if (m_pSegmentTemplate)
  {
    QTV_Delete(m_pSegmentTemplate);
    m_pSegmentTemplate = NULL;
  }
}
/* @brief: Copy constructor
*/
PeriodInfo::PeriodInfo(const PeriodInfo &period_info)
:m_pRepresentationInfo(NULL),
m_pRepresentationGroup(NULL),
m_nNumRepGroups(0),
m_PeriodIdentifier(NULL),
m_nNumRepresentations(0),
m_nPeriodStartTime(0),
m_nPeriodDuration(0),
m_bLastPeriod(false),
m_nRepArraySize(0),
m_nRepGrpArrSize(0),
m_nPeriodKey(0),
m_pSegmentTemplate(NULL),
m_bSegmentTemplateFound(false),
m_pSegmentBase(NULL),
m_bSegmentBaseFound(false),
m_pSegmentList(NULL),
m_nNumSegURL(0),
m_bSegmentListFound(false)
{
  Copy(period_info);
}
/* @brief: Copy function will copy all atributes except
Representation Info array
*/
void PeriodInfo::Copy(const PeriodInfo& period_info)
{
  m_nNumRepresentations = period_info.m_nNumRepresentations;
  m_nNumRepGroups = period_info.m_nNumRepGroups;
  m_nPeriodKey = period_info.m_nPeriodKey;
  m_nPeriodStartTime = period_info.m_nPeriodStartTime;
  m_nPeriodDuration = period_info.m_nPeriodDuration;
  m_bLastPeriod = period_info.m_bLastPeriod;
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
  if(m_pRepresentationInfo)
  {
    QTV_Delete_Array(m_pRepresentationInfo);
    m_pRepresentationInfo = NULL;
  }
  if(m_pRepresentationGroup)
  {
    QTV_Delete_Array(m_pRepresentationGroup);
    m_pRepresentationGroup = NULL;
  }
  if(period_info.m_pRepresentationInfo)
  {
    m_pRepresentationInfo = QTV_New_Array(RepresentationInfo,m_nNumRepresentations);
    for(int i=0;i<m_nNumRepresentations;i++)
    {
      m_pRepresentationInfo[i]=period_info.m_pRepresentationInfo[i];
    }
  }
  if(period_info.m_pRepresentationGroup)
  {
    m_pRepresentationGroup = QTV_New_Array(RepresentationGroup,m_nNumRepGroups);
    for(int i=0;i<m_nNumRepGroups;i++)
    {
      m_pRepresentationGroup[i]=period_info.m_pRepresentationGroup[i];
    }
  }
  return;
}
/* @brief: Assignment operator overloading
*/
PeriodInfo& PeriodInfo::operator=(const PeriodInfo& period_info)
{
  Copy(period_info);
  return *this;
}
char* PeriodInfo::getPeriodIdentifier()
{
  return m_PeriodIdentifier;
}

/**
 * Update on an mpd-update
 */
void PeriodInfo::SetPeriodKey(uint64 key)
{
  m_nPeriodKey = key;

  int numNewRepGrps = 0;
  RepresentationGroup *newRepGrp = getRepGrpInfo(numNewRepGrps);

  for (int i = 0; i < numNewRepGrps; ++i)
  {
    newRepGrp[i].UpdatePeriodKey(key);
  }
}

/**
 * Get the unique period key
 */
uint64 PeriodInfo::getPeriodKey() const
{
  return m_nPeriodKey;
}
/*
* @brief
* Returns the start time for the period (in seconds)
*/
uint64 PeriodInfo::getStartTime() const
{
  return m_nPeriodStartTime;
}

/*
* @brief
* Returns the duration for the period (in seconds)
*/
double PeriodInfo::getDuration() const
{
  return m_nPeriodDuration;
}

/* @brief: Sets the period start time */
void PeriodInfo::setStartTime(uint64 starttime)
{
  m_nPeriodStartTime = starttime;
  return;
}

/* @brief: Sets the period duration */
void PeriodInfo::setDuration(double duration)
{
  m_nPeriodDuration = duration;
  return;
}

void PeriodInfo::SetPeriodInfo(uint64 key,uint64 start_time, double duration, char* periodIdent)
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

/* @brief: Initializes the representation grp info for the period */
RepresentationGroup* PeriodInfo::InitializeGroupInfo(
  int numrepgrps)
{
  if(numrepgrps == 0)
  {
    m_nRepGrpArrSize = MIN_REP_GRP_ARRAY_SIZE;
  }
  else
  {
    m_nRepGrpArrSize = numrepgrps;
  }
  m_nNumRepGroups = numrepgrps;
  if(m_pRepresentationGroup)
  {
    QTV_Delete_Array(m_pRepresentationGroup);
    m_pRepresentationGroup = NULL;
  }
  m_pRepresentationGroup = QTV_New_Array(RepresentationGroup,
    m_nRepGrpArrSize);
  return m_pRepresentationGroup;
}


/**
 * Retrieve representationInfo from the representation
 * identifier in MPD.
 */
RepresentationInfo *PeriodInfo::GetRepresentationByIdentifier(const char *repIdentifier)
{
  RepresentationInfo *repInfo = NULL;
  int numRepGrps = 0;
  RepresentationGroup *repGrps = getRepGrpInfo(numRepGrps);

  if (repGrps)
  {
    for (int i = 0; i < numRepGrps; ++i)
    {
      int numReps = 0;
      RepresentationInfo *repArray = repGrps[i].getRepInfo(numReps);

      if (repIdentifier)
      {
        for (int j = 0; j < numReps; ++j)
        {
          if (std_stricmp(repArray[j].getRepIdentifier(), repIdentifier) == 0)
          {
            repInfo = &repArray[j];
            break;
          }
        }
      }

      if (repInfo)
      {
        break;
      }
    }
  }

  return repInfo;
}

/* @brief: Gets the representation info for the period */
RepresentationGroup* PeriodInfo::getRepGrpInfo(int& numRepGrps)
{
  numRepGrps = m_nNumRepGroups;
  return m_pRepresentationGroup;
}
bool PeriodInfo::AddRepGroup(int& RepGrpIndex)
{
   bool bOk = true;
   if(m_nNumRepGroups >= m_nRepGrpArrSize)
   {
     bOk = ResizeGrpInfo(2*m_nRepGrpArrSize);
   }
   if(bOk)
   {
     RepGrpIndex = m_nNumRepGroups;
     m_nNumRepGroups++;
   }
   return bOk;
}

/* @brief: remove the group. Decrement the count */
bool PeriodInfo::RemoveRepGroup()
{
  bool bOk = true;
  m_nNumRepGroups--;
  return bOk;
}

bool PeriodInfo::CommitGroupInfo()
{
   bool bOk = true;
   if(m_nRepGrpArrSize > m_nNumRepGroups)
   {
     bOk = ResizeGrpInfo(m_nNumRepGroups);
   }
   return bOk;
}
bool PeriodInfo::GetGrpKeyForID(uint32 grpID,uint64& grpKey)
{
  bool bRet = false;
  for(int i=0;i<m_nNumRepGroups;i++)
  {
    if(m_pRepresentationGroup[i].getGrpID() == grpID)
    {
      grpKey = m_pRepresentationGroup[i].getKey();
      bRet = true;
      break;
    }
  }
  return bRet;
}

bool PeriodInfo::ResizeGrpInfo(int newSize)
{
  bool bOk=false;
  if(m_nRepGrpArrSize < newSize)
  {
    RepresentationGroup* temp;
    temp = QTV_New_Array(RepresentationGroup,(m_nRepGrpArrSize));
    if(temp)
    {
      for(int i=0;i<m_nNumRepGroups;i++)
      {
        temp[i]=m_pRepresentationGroup[i];
      }
      QTV_Delete_Array(m_pRepresentationGroup);
      m_pRepresentationGroup = NULL;
      m_pRepresentationGroup =  QTV_New_Array(RepresentationGroup,(newSize));
      if(m_pRepresentationGroup)
      {
        for(int i=0;i<m_nNumRepGroups;i++)
        {
          m_pRepresentationGroup[i]=temp[i];
        }
        m_nRepGrpArrSize = newSize;
        bOk = true;
      }
      QTV_Delete_Array(temp);
      temp = NULL;
    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}

bool PeriodInfo::IsValidPeriod()
{
  bool bOk = true;
  if (m_nNumRepGroups == 0)
  {
    bOk = false;
  }
  return bOk;
}

bool PeriodInfo::InitialiseSegmentBase()
{
  bool bOk = false;
  if(m_pSegmentBase)
  {
    QTV_Delete(m_pSegmentBase);
    m_pSegmentBase = NULL;
  }
  m_pSegmentBase = QTV_New(SegmentBaseType);
  if (m_pSegmentBase)
  {
    bOk = true;
    m_bSegmentBaseFound = true;
  }
  return bOk;
}

SegmentBaseType *PeriodInfo::GetSegmentBase()
{
  return m_pSegmentBase;
}

/* @brief: Initialises Segment template */
bool PeriodInfo::InitialiseSegmentTemplate()
{
  bool bOk = false;
  if(m_pSegmentTemplate)
  {
    QTV_Delete(m_pSegmentTemplate);
    m_pSegmentTemplate = NULL;
  }
  m_pSegmentTemplate = QTV_New(SegmentTemplateType);
  if (m_pSegmentTemplate)
  {
    m_bSegmentTemplateFound = true;
    bOk = true;
  }
  return bOk;
}

/* @brief: Initialises Segment  */
bool PeriodInfo::InitializeSegmentTimeline(int numSEntry)
{
  bool bOk = false;
  if (numSEntry > 0)
  {
    if(m_pSegmentTemplate)
    {
      bOk = m_pSegmentTemplate->InitializeSegmentTimeline(numSEntry);
    }
    else if (m_pSegmentList)
    {
      bOk = m_pSegmentList->InitializeSegmentTimeline(numSEntry);
    }
  }
    return bOk;
}

/* @brief: Initialises Segment list */
bool PeriodInfo::InitialiseSegmentList()
{
  bool bOk = false;
  if(m_pSegmentList)
  {
    QTV_Delete(m_pSegmentList);
    m_pSegmentList = NULL;
  }

  m_pSegmentList=QTV_New(SegmentListType);
  if (m_pSegmentList)
  {
    m_bSegmentListFound = true;
    bOk = true;
  }
  return bOk;
}

bool PeriodInfo::InitialiseSegmentUrl(int numSegUrl)
{
  bool bOk = false;
  if (numSegUrl > 0)
  {
    if (m_pSegmentList)
    {
      m_pSegmentList->InitialiseSegmentUrl(numSegUrl);
      bOk = true;
    }
  }
  return bOk;
}


SegmentListType* PeriodInfo::GetSegmentList()
{
  return m_pSegmentList;
}


/* @brief: sets segment list
 */

bool PeriodInfo::SetSegmentUrl(uint32 urlIndex, char *mediaUrl,
                         char *mediaRange, char *indexUrl, char *indexRange)
{
  bool bOk = false;
  if (m_pSegmentList)
  {
    m_pSegmentList->SetSegmentUrl(urlIndex, mediaUrl,
                                  mediaRange, indexUrl, indexRange);
    bOk = true;
  }
  return bOk;
}

/* @brief: sets segment base related info
 */

void PeriodInfo::SetSegmentBaseInfo(uint32 timeScale, uint64 presentationTimeOffset,
                        char *indexRange,bool indexRangeExact,
                        URLType *initialisation, URLType *representationIndex)
{
  if (m_pSegmentBase)
  {
    m_pSegmentBase->SetTimeScale(timeScale);
    m_pSegmentBase->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentBase->SetIndexRange(indexRange);
    m_pSegmentBase->SetIndexRangeExact(indexRangeExact);
    m_pSegmentBase->SetInitialisation(initialisation);
    m_pSegmentBase->SetRepresentationIndex(representationIndex);
  }
}

/* @brief: sets segment template attributes
 */

void PeriodInfo::SetSegmentTemplateInfo(char *mediaTemplate,char *indexTemplate,
                                          char *initialisationTemplate, char *bitstreamSwitchingTemplate)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetMediaTemplate(mediaTemplate);
    m_pSegmentTemplate->SetIndexTemplate(indexTemplate);
    m_pSegmentTemplate->SetInitialisationTemplate(initialisationTemplate);
    m_pSegmentTemplate->SetBSSwitchingTemplate(bitstreamSwitchingTemplate);
  }
}

 /* @brief: sets multiple segmentbase attributes
 */
void PeriodInfo::SetMultiSegmentBaseInfo( uint32 segmentDuration, uint32 startNumber,
                                          uint32 timeScale, uint64 presentationTimeOffset,
                                                    char *indexRange,bool indexRangeExact,
                                                  URLType *initialisation, URLType *representationIndex)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetDuration(segmentDuration);
    m_pSegmentTemplate->SetStartNumber(startNumber);
    m_pSegmentTemplate->SetTimeScale(timeScale);
    m_pSegmentTemplate->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentTemplate->SetIndexRange(indexRange);
    m_pSegmentTemplate->SetIndexRangeExact(indexRangeExact);
    m_pSegmentTemplate->SetInitialisation(initialisation);
    m_pSegmentTemplate->SetRepresentationIndex(representationIndex);
  }
  else if (m_pSegmentList)
  {
    m_pSegmentList->SetDuration(segmentDuration);
    m_pSegmentList->SetStartNumber(startNumber);
    m_pSegmentList->SetTimeScale(timeScale);
    m_pSegmentList->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentList->SetIndexRange(indexRange);
    m_pSegmentList->SetIndexRangeExact(indexRangeExact);
    m_pSegmentList->SetInitialisation(initialisation);
    m_pSegmentList->SetRepresentationIndex(representationIndex);
  }
}


/* @brief: sets initilisation segment url
 */
void PeriodInfo::SetInitialisation(URLType *initialisation)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetInitialisation(initialisation);
  }
}


/* @brief: sets segment timeline
 */
bool PeriodInfo::SetSegmentTimeline(uint32 segTimelineIndex, uint64 startTime,
                         uint64 duration, int32 repeatCount)
{
  bool bOk = false;
  if (m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetSegmentTimeline(segTimelineIndex, startTime,
                                            duration, repeatCount);
    bOk = true;
  }
  else if (m_pSegmentList)
  {
    m_pSegmentList->SetSegmentTimeline(segTimelineIndex, startTime,
                                            duration, repeatCount);
    bOk = true;
  }
  return bOk;
}

/* ***************************************************************************
Representation Group Class
***************************************************************************
*/

/* @brief - RepresentationGroup Constructor
*/
RepresentationGroup::RepresentationGroup()
 :m_pRepresentationInfo(NULL),
  m_nNumRepresentations(0),
  m_GrpIdentifier(NULL),
  m_codec(NULL),
  m_resolution(NULL),
  m_nFrameRate(0),
  m_SamplingRate(NULL),
  m_nNumSamplingRates(0),
  m_Channels(NULL),
  m_nNumChannels(0),
  m_nParX(0),
  m_nParY(0),
  m_language(NULL),
  m_MimeType(NULL),
  m_bSubSegmentAlignment(false),
  m_nKey(0),
  m_nRepArraySize(0),
  m_nGrpID(0),
  m_bSegmentAlinged(false),
  m_nSubSegmentStartsWithSAP(-1),
  m_pSegmentTemplate(NULL),
  m_bSegmentTemplateFound(false),
  m_nStartWithSAPVal(-1),
  m_nMajorType(-1),
  m_pSegmentBase(NULL),
  m_bSegmentBaseFound(false),
  m_pSegmentList(NULL),
  m_nNumSegURL(0),
  m_bSegmentListFound(false),
  m_nStartOffsetInitSeg(-1),
  m_nEndOffsetInitSeg(-1),
  m_InitialisationSegmentUrl(NULL),
  m_InitialisationSegmentRange(NULL)
{
  m_bEstimator = NULL;
}
/* @brief - RepresentationGroup Destructor
*/
RepresentationGroup::~RepresentationGroup()
{
  if(m_GrpIdentifier)
  {
     QTV_Free(m_GrpIdentifier);
     m_GrpIdentifier = NULL;
  }

  if(m_codec)
  {
    if(m_codec->mcodecs)
    {
      QTV_Free(m_codec->mcodecs);
      m_codec->mcodecs=NULL;
    }
    QTV_Free(m_codec);
    m_codec=NULL;
  }
  if(m_resolution)
  {
    QTV_Free(m_resolution);
    m_resolution = NULL;
  }
  if(m_pRepresentationInfo)
  {
    QTV_Delete_Array(m_pRepresentationInfo);
    m_pRepresentationInfo = NULL;
  }
  if(m_SamplingRate)
  {
    QTV_Free(m_SamplingRate);
    m_SamplingRate = NULL;
  }
  if(m_Channels)
  {
    QTV_Free(m_Channels);
    m_Channels = NULL;
  }
  if(m_language)
  {
    QTV_Free(m_language);
    m_language = NULL;
  }
  if (m_pSegmentBase)
  {
    QTV_Delete(m_pSegmentBase);
    m_pSegmentBase = NULL;
  }
  if (m_pSegmentList)
  {
    QTV_Delete(m_pSegmentList);
    m_pSegmentList = NULL;
  }
  if (m_pSegmentTemplate)
  {
    QTV_Delete(m_pSegmentTemplate);
    m_pSegmentTemplate = NULL;
  }
  if(m_MimeType)
  {
    QTV_Free(m_MimeType);
    m_MimeType = NULL;
  }
  if(m_InitialisationSegmentUrl)
  {
    QTV_Free(m_InitialisationSegmentUrl);
    m_InitialisationSegmentUrl = NULL;
  }
  if(m_InitialisationSegmentRange)
  {
    QTV_Free(m_InitialisationSegmentRange);
    m_InitialisationSegmentRange = NULL;
  }
}
/* @brief: Copy function will copy all attributes except Segment info array
*/
void RepresentationGroup::Copy(const RepresentationGroup& repgrp_info)
{
  m_nNumRepresentations = repgrp_info.m_nNumRepresentations;
  m_nFrameRate = repgrp_info.m_nFrameRate;
  m_nNumSamplingRates = repgrp_info.m_nNumSamplingRates;
  m_nNumChannels = repgrp_info.m_nNumChannels;
  m_nParX = repgrp_info.m_nParX;
  m_nParY = repgrp_info.m_nParY;
  m_bSubSegmentAlignment = repgrp_info.m_bSubSegmentAlignment;
  m_nKey = repgrp_info.m_nKey;
  m_nRepArraySize = repgrp_info.m_nRepArraySize;
  m_nGrpID = repgrp_info.m_nGrpID;
  m_bEstimator = repgrp_info.m_bEstimator;
  m_bSegmentAlinged = repgrp_info.m_bSegmentAlinged;
  m_nSubSegmentStartsWithSAP = repgrp_info.m_nSubSegmentStartsWithSAP;
  m_nStartWithSAPVal = repgrp_info.m_nStartWithSAPVal;
  m_nMajorType = repgrp_info.m_nMajorType;
  m_nStartOffsetInitSeg = repgrp_info.m_nStartOffsetInitSeg;
  m_nEndOffsetInitSeg   = repgrp_info.m_nEndOffsetInitSeg;
  if(repgrp_info.m_InitialisationSegmentUrl)
  {
    if(m_InitialisationSegmentUrl)
    {
      QTV_Free(m_InitialisationSegmentUrl);
    }

    int bufSize = (int)std_strlen(repgrp_info.m_InitialisationSegmentUrl) + 1;
    m_InitialisationSegmentUrl = (char *)QTV_Malloc(bufSize * sizeof(char));

    if(m_InitialisationSegmentUrl)
    {
      std_strlcpy(m_InitialisationSegmentUrl, repgrp_info.m_InitialisationSegmentUrl, bufSize);
    }
  }

  if(repgrp_info.m_InitialisationSegmentRange)
  {
    if(m_InitialisationSegmentRange)
    {
      QTV_Free(m_InitialisationSegmentRange);
    }

    int bufSize = (int)std_strlen(repgrp_info.m_InitialisationSegmentRange) + 1;
    m_InitialisationSegmentRange = (char *)QTV_Malloc(bufSize * sizeof(char));

    if(m_InitialisationSegmentRange)
    {
      std_strlcpy(m_InitialisationSegmentRange, repgrp_info.m_InitialisationSegmentRange, bufSize);
    }
  }

  if(m_GrpIdentifier)
  {
    QTV_Free(m_GrpIdentifier);
    m_GrpIdentifier = NULL;
  }

  if(repgrp_info.m_GrpIdentifier)
  {
    int bufSz = 1 + (int)std_strlen(repgrp_info.m_GrpIdentifier);
    m_GrpIdentifier = (char *)QTV_Malloc(bufSz * sizeof(char));
    if(m_GrpIdentifier)
    {
      std_strlcpy(m_GrpIdentifier, repgrp_info.m_GrpIdentifier, bufSz);
    }
  }

  if(repgrp_info.m_language)
  {
    if(m_language)
    {
      QTV_Free(m_language);
      m_language = NULL;
    }
    m_language = (char*)QTV_Malloc(std_strlen(repgrp_info.m_language)+1);
    if(m_language)
    {
      std_strlcpy(m_language,repgrp_info.m_language,std_strlen(repgrp_info.m_language)+1);
    }
  }

  if(repgrp_info.m_MimeType)
  {
    if(m_MimeType)
    {
      QTV_Free(m_MimeType);
    }

    int bufSize = (int)std_strlen(repgrp_info.m_MimeType) + 1;
    m_MimeType = (char *)QTV_Malloc(bufSize * sizeof(char));

    if(m_MimeType)
    {
      std_strlcpy(m_MimeType, repgrp_info.m_MimeType, bufSize);
    }
  }
  if(repgrp_info.m_Channels)
  {
    if(m_Channels)
    {
      QTV_Free(m_Channels);
      m_Channels = NULL;
    }
    m_Channels = (double*)QTV_Malloc(sizeof(double)*m_nNumChannels);
    if(m_Channels)
    {
      for(int i=0;i<m_nNumChannels;i++)
      {
        m_Channels[i] = repgrp_info.m_Channels[i];
      }
    }
  }
  if(repgrp_info.m_SamplingRate)
  {
    if(m_SamplingRate)
    {
      QTV_Free(m_SamplingRate);
      m_SamplingRate = NULL;
    }
    m_SamplingRate = (uint32*)QTV_Malloc(sizeof(uint32)*m_nNumSamplingRates);
    if(m_SamplingRate)
    {
      for(int i=0;i<m_nNumSamplingRates;i++)
      {
        m_SamplingRate[i] = repgrp_info.m_SamplingRate[i];
      }
    }
  }
  if(repgrp_info.m_codec)
  {
    if(m_codec)
    {
      if(m_codec->mcodecs)
      {
        QTV_Free(m_codec->mcodecs);
        m_codec->mcodecs=NULL;
      }
      QTV_Free(m_codec);
      m_codec=NULL;
    }
    m_codec = (Codecs*)QTV_Malloc(sizeof(Codecs));
    if(m_codec)
    {
      m_codec->numcodecs=repgrp_info.m_codec->numcodecs;
      m_codec->mcodecs=(CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(m_codec->numcodecs));
      if(m_codec->mcodecs)
      {
        for(int i=0;i<m_codec->numcodecs;i++)
        {
          m_codec->mcodecs[i]=repgrp_info.m_codec->mcodecs[i];
        }
      }
    }
  }
  if(repgrp_info.m_resolution)
  {
    if(m_resolution)
    {
      QTV_Free(m_resolution);
      m_resolution=NULL;
    }
    m_resolution=(Resolution*)QTV_Malloc(sizeof(Resolution));
    if(m_resolution)
    {
      m_resolution->height = repgrp_info.m_resolution->height;
      m_resolution->width = repgrp_info.m_resolution->width;
    }
  }
  if(m_pRepresentationInfo)
  {
    QTV_Delete_Array(m_pRepresentationInfo);
    m_pRepresentationInfo = NULL;
  }

  m_AdaptationSetSpecificDescs.CopyDescs(repgrp_info.m_AdaptationSetSpecificDescs);
  m_CommonDescs.CopyDescs(repgrp_info.m_CommonDescs);

  m_AdaptationSetStringValues.CopyStringValues(repgrp_info.m_AdaptationSetStringValues);
  m_CommonStringValues.CopyStringValues(repgrp_info.m_CommonStringValues);
}
/* @brief Copy constructor
*/
RepresentationGroup::RepresentationGroup(const RepresentationGroup &repgrp_info)
 :m_pRepresentationInfo(NULL),
  m_nNumRepresentations(0),
  m_GrpIdentifier(NULL),
  m_codec(NULL),
  m_resolution(NULL),
  m_nFrameRate(0),
  m_SamplingRate(NULL),
  m_nNumSamplingRates(0),
  m_Channels(NULL),
  m_nNumChannels(0),
  m_nParX(0),
  m_nParY(0),
  m_language(NULL),
  m_MimeType(NULL),
  m_bSubSegmentAlignment(false),
  m_nKey(0),
  m_nRepArraySize(0),
  m_nGrpID(0),
  m_bSegmentAlinged(false),
  m_nSubSegmentStartsWithSAP(-1),
  m_pSegmentTemplate(NULL),
  m_bSegmentTemplateFound(false),
  m_nStartWithSAPVal(-1),
  m_nMajorType(-1),
  m_pSegmentBase(NULL),
  m_bSegmentBaseFound(false),
  m_pSegmentList(NULL),
  m_nNumSegURL(0),
  m_bSegmentListFound(false),
  m_nStartOffsetInitSeg(-1),
  m_nEndOffsetInitSeg(-1),
  m_InitialisationSegmentUrl(NULL),
  m_InitialisationSegmentRange(NULL)
{
  Copy(repgrp_info);
  if(repgrp_info.m_pRepresentationInfo)
  {
    m_pRepresentationInfo = QTV_New_Array(RepresentationInfo,m_nNumRepresentations);
    for(int i=0;i<m_nNumRepresentations;i++)
    {
      m_pRepresentationInfo[i] = repgrp_info.m_pRepresentationInfo[i];
    }
  }
}
/* @brief:  Assignment Operator overloading
*/
RepresentationGroup& RepresentationGroup::operator=(const RepresentationGroup& repgrp_info)
{
  Copy(repgrp_info);
  if(repgrp_info.m_pRepresentationInfo)
  {
    m_pRepresentationInfo = QTV_New_Array(RepresentationInfo,m_nNumRepresentations);
    for(int i=0;i< m_nNumRepresentations;i++)
    {
      m_pRepresentationInfo[i] = repgrp_info.m_pRepresentationInfo[i];
    }
  }
  return *this;
}
RepresentationInfo* RepresentationGroup::getRepInfo(int& numRepresentations)
{
  numRepresentations = m_nNumRepresentations;
  return m_pRepresentationInfo;
}

const char *RepresentationGroup::GetGrpIdentifer() const
{
  return m_GrpIdentifier;
}

/*
* @brief
* Returns the codecs for the representation
*/
bool RepresentationGroup::getCodec(CodecInfo* pCodecs,int& numCodecs)
{
  bool ret = false;
  if(m_codec && m_codec->numcodecs > 0)
  {
    ret = true;
    if(numCodecs < m_codec->numcodecs)
    {
      numCodecs = m_codec->numcodecs;
    }
    else if(pCodecs)
    {
      for(int i=0;i<m_codec->numcodecs;i++)
      {
        pCodecs[i]=m_codec->mcodecs[i];
      }
    }
  }
  return ret;
}
Resolution* RepresentationGroup::getResolution()
{
  return m_resolution;
}
char* RepresentationGroup::getLanguage()
{
  return m_language;
}

double RepresentationGroup::getFrameRate()
{
  return m_nFrameRate;
}
uint32 RepresentationGroup::getParX()
{
  return m_nParX;
}
uint32 RepresentationGroup::getParY()
{
  return m_nParY;
}

uint64 RepresentationGroup::getKey()
{
  return m_nKey;
}
uint32 RepresentationGroup::getGrpID()
{
  return m_nGrpID;
}
bool RepresentationGroup::AddRepresentation(int& RepIndex)
{
   bool bOk = true;
   if(m_nNumRepresentations >= m_nRepArraySize)
   {
     bOk = ResizeRepInfo(2*m_nRepArraySize);
   }
   if(bOk)
   {
     RepIndex = m_nNumRepresentations;
     m_nNumRepresentations++;
   }
   return bOk;
}
RepresentationInfo* RepresentationGroup::InitializeRepInfo(int numRepresentations)
{
  if(numRepresentations == 0)
  {
    m_nRepArraySize = MIN_REP_ARRAY_SIZE;
  }
  else
  {
    m_nRepArraySize = numRepresentations;
  }
  m_nNumRepresentations = numRepresentations;
  if(m_pRepresentationInfo)
  {
    QTV_Delete_Array(m_pRepresentationInfo);
    m_pRepresentationInfo = NULL;
  }
  m_pRepresentationInfo = QTV_New_Array(RepresentationInfo,
    m_nRepArraySize);
  return m_pRepresentationInfo;
}

bool RepresentationGroup::IsValidRepGroup()
{
  bool bOk = true;
  if (m_nNumRepresentations == 0)
  {
    bOk = false;
  }
  return bOk;
}

bool RepresentationGroup::CommitRepInfo()
{
   bool bOk = true;
   if(m_nRepArraySize > m_nNumRepresentations)
   {
     bOk = ResizeRepInfo(m_nNumRepresentations);
   }
   return bOk;
}
bool RepresentationGroup::ResizeRepInfo(int newSize)
{
  bool bOk=false;
  if(m_nRepArraySize < newSize)
  {
    RepresentationInfo* temp;
    temp = QTV_New_Array(RepresentationInfo,(m_nRepArraySize));
    if(temp)
    {
      for(int i=0;i<m_nNumRepresentations;i++)
      {
        temp[i]=m_pRepresentationInfo[i];
      }
      QTV_Delete_Array(m_pRepresentationInfo);
      m_pRepresentationInfo = NULL;
      m_pRepresentationInfo =  QTV_New_Array(RepresentationInfo,(newSize));
      if(m_pRepresentationInfo)
      {
        for(int i=0;i<m_nNumRepresentations;i++)
        {
          m_pRepresentationInfo[i]=temp[i];
        }
        m_nRepArraySize = newSize;
        bOk = true;
      }
      QTV_Delete_Array(temp);
      temp = NULL;

    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}

void RepresentationGroup::SetGroupInfo(const char *repGrpId,
                                       Resolution *pResolution,uint32 parX,uint32 parY,
                                       char *language, char *mimeType, Codecs* codec,uint64 grp_key,
                                       bool subSegmentAlignment,
                                       uint32 frameRate,int numChannels,double *pChannels,
                                       int numSamplingRates, double *pSamplingRates,
                                       bool bSegmentAligned, int sapVal,
                                       int subSegmentSAPVal, uint32 grpID)
{
  if(m_GrpIdentifier)
  {
    QTV_Free(m_GrpIdentifier);
    m_GrpIdentifier = NULL;
  }

  if(repGrpId)
  {
    int reqdBufSize = 1 + (int)std_strlen(repGrpId);
    m_GrpIdentifier = (char *)QTV_Malloc(reqdBufSize * sizeof(char));

    if(m_GrpIdentifier)
    {
      std_strlcpy(m_GrpIdentifier, repGrpId, reqdBufSize);
    }
  }

   m_nParX = parX;
   m_nParY = parY;
   m_nKey = grp_key;
   m_nFrameRate = (double)frameRate;
   m_nNumSamplingRates = numSamplingRates;
   m_nNumChannels = numChannels;
   m_nGrpID = grpID;
   m_bSegmentAlinged = bSegmentAligned;
   m_nSubSegmentStartsWithSAP = subSegmentSAPVal;
   m_nStartWithSAPVal = sapVal;
   if(m_SamplingRate)
   {
     QTV_Free(m_SamplingRate);
     m_SamplingRate = NULL;
   }
   if(pSamplingRates)
   {
     m_SamplingRate = (uint32*)QTV_Malloc(sizeof(uint32)*m_nNumSamplingRates);
     if(m_SamplingRate)
     {
       for(int i=0;i<m_nNumSamplingRates;i++)
       {
         m_SamplingRate[i] = (uint32)pSamplingRates[i];
       }
     }
   }
   if(m_Channels)
   {
     QTV_Free(m_Channels);
     m_Channels = NULL;
   }
   if(pChannels)
   {
     m_Channels = (double*)QTV_Malloc(sizeof(double)*m_nNumChannels);
     if(m_Channels)
     {
       for(int i=0;i<m_nNumChannels;i++)
       {
         m_Channels[i] = pChannels[i];
       }
     }
   }

   m_bSubSegmentAlignment = subSegmentAlignment;
   if(pResolution)
   {
     if(m_resolution)
     {
       QTV_Free(m_resolution);
       m_resolution = NULL;
     }
     m_resolution = (Resolution*)QTV_Malloc(sizeof(Resolution));
     if(m_resolution)
     {
       m_resolution->width = pResolution->width;
       m_resolution->height = pResolution->height;
     }
   }
   if(language)
   {
     if(m_language)
     {
       QTV_Free(m_language);
       m_language = NULL;
     }
     m_language=(char*)QTV_Malloc(std_strlen(language)+1);
     if(m_language)
     {
       std_strlcpy(m_language,language,std_strlen(language)+1);
     }
   }

   if(mimeType)
   {
     if(m_MimeType)
     {
       QTV_Free(m_MimeType);
     }

     int bufSize = (int)std_strlen(mimeType) + 1;
     m_MimeType = (char *)QTV_Malloc(bufSize * sizeof(char));

     if(m_MimeType)
     {
       std_strlcpy(m_MimeType, mimeType, bufSize);
     }
   }

   if(codec)
   {
     if(m_codec)
     {
       if(m_codec->mcodecs)
       {
         QTV_Free(m_codec->mcodecs);
         m_codec->mcodecs=NULL;
       }
       QTV_Free(m_codec);
       m_codec=NULL;
     }
     m_codec = (Codecs*)QTV_Malloc(sizeof(Codecs));
     if(m_codec)
     {
       m_codec->numcodecs=codec->numcodecs;
       m_codec->mcodecs=(CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(m_codec->numcodecs));
       if(m_codec->mcodecs)
       {
         for(int i=0;i<m_codec->numcodecs;i++)
         {
           m_codec->mcodecs[i].majorType = codec->mcodecs[i].majorType;
           m_codec->mcodecs[i].minorType = codec->mcodecs[i].minorType;
           m_codec->mcodecs[i].profile = codec->mcodecs[i].profile;
           m_codec->mcodecs[i].level = codec->mcodecs[i].level;
         }
       }
     }
   }
 }
 void RepresentationGroup::SetCodecInfo(Codecs* codec)
 {
   if(codec)
   {
     if(m_codec)
     {
       if(m_codec->mcodecs)
       {
         QTV_Free(m_codec->mcodecs);
         m_codec->mcodecs=NULL;
       }
       QTV_Free(m_codec);
       m_codec=NULL;
     }
     m_codec = (Codecs*)QTV_Malloc(sizeof(Codecs));
     if(m_codec)
     {
       m_codec->numcodecs=codec->numcodecs;
       m_codec->mcodecs=(CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(m_codec->numcodecs));
       if(m_codec->mcodecs)
       {
         for(int i=0;i<m_codec->numcodecs;i++)
         {
           m_codec->mcodecs[i].majorType = codec->mcodecs[i].majorType;
           m_codec->mcodecs[i].minorType = codec->mcodecs[i].minorType;
           m_codec->mcodecs[i].profile = codec->mcodecs[i].profile;
           m_codec->mcodecs[i].level = codec->mcodecs[i].level;
         }
       }
     }
   }
 }

 void RepresentationGroup::UpdatePeriodKey(uint64 periodKey)
 {
   int numReps = 0;
   RepresentationInfo *repInfoArray = getRepInfo(numReps);

   if (repInfoArray)
   {
     for (int i = 0; i < numReps; ++i)
     {
       repInfoArray[i].UpdatePeriodKey(periodKey);
     }
   }
 }

/* @brief: check whether MediaStreamStructure id present for
 * all representation
 */

bool RepresentationGroup::IsMediaStreamStructIdPresentAllRep()
{
  bool bOk = true;
  for (int i = 0; i < m_nNumRepresentations; i++)
  {
    if (m_pRepresentationInfo[i].GetNumMediaStreamStructEntry() ==  0)
    {
      bOk = false;
      return bOk;
    }
  }
  return bOk;
}

/* @brief: compares mediastreamstructureId
 */
bool RepresentationGroup::CompareMediaStreamStruct()
{
  bool bOk = false;
  int startRepIndex = 0;
  for (int startRepIndex = 0; startRepIndex < m_nNumRepresentations; startRepIndex++)
  {
    int i = startRepIndex;
    for (int j = startRepIndex + 1; j< m_nNumRepresentations; j++)
    {

      bOk = CompareStreamStructId(&m_pRepresentationInfo[i],
                                   &m_pRepresentationInfo[j]);
      if (bOk)
      {
        m_pRepresentationInfo[i].SetMediaStreamStructMatchedWithOtherRep(true);
        m_pRepresentationInfo[j].SetMediaStreamStructMatchedWithOtherRep(true);
      }
      else
      {
        m_pRepresentationInfo[i].SetMediaStreamStructMatchedWithOtherRep(false);
        m_pRepresentationInfo[j].SetMediaStreamStructMatchedWithOtherRep(false);
      }
    }
  }
  bOk = false;
  int numItr = 0;
  while (numItr <  m_nNumRepresentations)
  {
    for (int i = 0; i < m_nNumRepresentations; i++)
    {
      if (m_pRepresentationInfo[i].GetStartWithSAPVal() == 3  ||
          m_pRepresentationInfo[i].GetSubSegStartWithSAPVal() == 3 )
      {
        if (!m_pRepresentationInfo[i].IsMediaStreamStructMatchedWithOtherRep())
        {
          RemoveRepresentationByIndex(i);
        }
        else
        {
          // atleast one rep's MediaStreamStructId mataches with
          //MediaStreamStruct of other rep's
          bOk = true;
        }
      }
    }
    numItr++;
  }
  return bOk;
}

/* @brief: remove this representation
 */
bool RepresentationGroup::RemoveRepresentation()
{
  bool bOk = true;
  m_nNumRepresentations--;
  return bOk;
}

/* @brief: remove representation by index
 */
bool RepresentationGroup::RemoveRepresentationByIndex(int repIndex)
{
  bool bOk = false;
  for(int i=0;i<m_nNumRepresentations;i++)
  {
    if (i > repIndex)
    {
          bOk = true;
      m_pRepresentationInfo[i-1] = m_pRepresentationInfo[i];
      m_nNumRepresentations--;
    }
  }
  return bOk;
}

/* initilises segment base class
*/
bool RepresentationGroup::InitialiseSegmentBase()
{
  bool bOk = false;
  if (!m_bSegmentBaseFound)
  {
    if(m_pSegmentBase)
    {
      QTV_Delete(m_pSegmentBase);
      m_pSegmentBase = NULL;
    }
    m_pSegmentBase = QTV_New(SegmentBaseType);
    if (m_pSegmentBase)
    {
      bOk = true;
      m_bSegmentBaseFound = true;
    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}

/* return segment base pointer
 */
SegmentBaseType *RepresentationGroup::GetSegmentBase()
{
  return m_pSegmentBase;
}


/* inherit segment base info from period
 */
bool RepresentationGroup::InheritSegmentBaseInfo(SegmentBaseType* periodSegmentBase)
{
  bool bOk = false;
  if (periodSegmentBase)
  {
    bOk =  InitialiseSegmentBase();
    if (bOk)
    {
      m_pSegmentBase->InheritSegmentBase(periodSegmentBase);
    }
    else
    {
      bOk = true;
    }
  }
  return bOk;
}


/* related segment base related info
 */
void RepresentationGroup::SetSegmentBaseInfo(uint32 timeScale, uint64 presentationTimeOffset,
                        char *indexRange,bool indexRangeExact,
                        URLType *initialisation, URLType *representationIndex)
{
  if (m_pSegmentBase)
  {
    m_pSegmentBase->SetTimeScale(timeScale);
    m_pSegmentBase->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentBase->SetIndexRange(indexRange);
    m_pSegmentBase->SetIndexRangeExact(indexRangeExact);
    m_pSegmentBase->SetInitialisation(initialisation);
    m_pSegmentBase->SetRepresentationIndex(representationIndex);
  }
}

/* @brief: Initialises Segment template */
bool RepresentationGroup::InitialiseSegmentTemplate()
{
  bool bOk =  false;
  if (!m_bSegmentTemplateFound)
  {
    if(m_pSegmentTemplate)
    {
      QTV_Delete(m_pSegmentTemplate);
      m_pSegmentTemplate = NULL;
    }
    m_pSegmentTemplate = QTV_New(SegmentTemplateType);
    if (m_pSegmentTemplate)
    {
      m_bSegmentTemplateFound = true;
      bOk = true;
    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}

/* Inherit SegmentTemplate from Period
 */
bool RepresentationGroup::InheritSegmentTemplate(SegmentTemplateType* periodSegmentTemplate)
{
  bool bOk = false;
  if (periodSegmentTemplate)
  {
    bOk =  InitialiseSegmentTemplate();
    if (bOk)
    {
      m_pSegmentTemplate->InheritMultiSegmentBaseInfo(periodSegmentTemplate);
      m_pSegmentTemplate->InheritSegmentTemplateInfo(periodSegmentTemplate);
    }
    else
    {
      bOk = true;
    }
  }
  return bOk;
}

/* Inherit SegmentList from Period
 */

bool RepresentationGroup::InheritSegmentList(SegmentListType *periodSegmentList)
{
  bool bOk = false;
  if (periodSegmentList)
  {
    bOk =  InitialiseSegmentList();
    if (bOk)
    {
      m_pSegmentList->InheritMultiSegmentBaseInfo(periodSegmentList);
      // current level's segment list does not contain any segment url
      //then only inherit
      if (m_pSegmentList->GetNumSegmentUrl() == 0)
      {
        bOk = m_pSegmentList->InitialiseSegmentUrl(periodSegmentList->GetNumSegmentUrl());
        if (bOk)
        {
          for (int i = 0; i<periodSegmentList->GetNumSegmentUrl(); i++)
          {
            m_pSegmentList->InheritSegmentUrl(periodSegmentList, i);
          }
        }
                else
                {
                  bOk = true;
                }
      }
    }
  }
    return bOk;
}

/* @brief: Initialises Segment timeline */
bool RepresentationGroup::InitializeSegmentTimeline(int numSEntry)
{
  bool bOk = false;
  if (numSEntry > 0)
  {
    if(m_pSegmentTemplate)
    {
      bOk = m_pSegmentTemplate->InitializeSegmentTimeline(numSEntry);
    }
    else if (m_pSegmentList)
    {
      bOk = m_pSegmentList->InitializeSegmentTimeline(numSEntry);
    }
  }
    return bOk;
}

/* @brief: Initialises Segment list */
bool RepresentationGroup::InitialiseSegmentList()
{
  bool bOk = false;
  if (!m_bSegmentListFound)
  {
    if(m_pSegmentList)
    {
      QTV_Delete(m_pSegmentList);
      m_pSegmentList = NULL;
    }
    m_pSegmentList= QTV_New(SegmentListType);
    if (m_pSegmentList)
    {
      m_bSegmentListFound = true;
      bOk = true;
    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}

bool RepresentationGroup::InitialiseSegmentUrl(int numSegUrl)
{
  bool bOk = false;
  if (numSegUrl > 0)
  {
    if (m_pSegmentList)
    {
      m_pSegmentList->InitialiseSegmentUrl(numSegUrl);
      bOk = true;
    }
  }
  return bOk;
}

SegmentListType* RepresentationGroup::GetSegmentList()
{
  return m_pSegmentList;
}

/* @brief : set Segment list elements
*/

bool RepresentationGroup::SetSegmentUrl(uint32 urlIndex, char *mediaUrl,
                         char *mediaRange, char *indexUrl, char *indexRange)
{
  bool bOk = false;
  if (m_pSegmentList)
  {
    m_pSegmentList->SetSegmentUrl(urlIndex, mediaUrl,
                                  mediaRange, indexUrl, indexRange);
    bOk = true;
  }
  return bOk;
}




/* @brief: returns segment template pointer*/
SegmentTemplateType *RepresentationGroup::GetSegmentTemplate()
{
  return m_pSegmentTemplate;
}

/* @brief: sets segment template attributes*/
void RepresentationGroup::SetSegmentTemplateInfo(char *mediaTemplate,char *indexTemplate,
                                          char *initialisationTemplate, char *bitstreamSwitchingTemplate)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetMediaTemplate(mediaTemplate);
    m_pSegmentTemplate->SetIndexTemplate(indexTemplate);
    m_pSegmentTemplate->SetInitialisationTemplate(initialisationTemplate);
    m_pSegmentTemplate->SetBSSwitchingTemplate(bitstreamSwitchingTemplate);
  }

}

/* @brief: sets multiple segment base attributes*/
void RepresentationGroup::SetMultiSegmentBaseInfo( uint32 segmentDuration, uint32 startNumber,
                                                   uint32 timeScale, uint64 presentationTimeOffset,
                                                               char *indexRange,bool indexRangeExact,
                                                           URLType *initialisation, URLType *representationIndex)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetDuration(segmentDuration);
    m_pSegmentTemplate->SetStartNumber(startNumber);
    m_pSegmentTemplate->SetTimeScale(timeScale);
    m_pSegmentTemplate->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentTemplate->SetIndexRange(indexRange);
    m_pSegmentTemplate->SetIndexRangeExact(indexRangeExact);
    m_pSegmentTemplate->SetInitialisation(initialisation);
    m_pSegmentTemplate->SetRepresentationIndex(representationIndex);

  }
  else if (m_pSegmentList)
  {
    m_pSegmentList->SetDuration(segmentDuration);
    m_pSegmentList->SetStartNumber(startNumber);
    m_pSegmentList->SetTimeScale(timeScale);
    m_pSegmentList->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentList->SetIndexRange(indexRange);
    m_pSegmentList->SetIndexRangeExact(indexRangeExact);
    m_pSegmentList->SetInitialisation(initialisation);
    m_pSegmentList->SetRepresentationIndex(representationIndex);
  }
}


/* @brief: sets initilisation segment url
 */
void RepresentationGroup::SetInitialisation(URLType *initialisation)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetInitialisation(initialisation);
  }
}

void RepresentationGroup::SetInitialisationSegmentUrl(char *base_url, char *url)
{
  int tmpUrlLength = 0;
  if(m_InitialisationSegmentUrl)
  {
    QTV_Free(m_InitialisationSegmentUrl);
    m_InitialisationSegmentUrl = NULL;
  }
  if(m_InitialisationSegmentRange)
  {
    QTV_Free(m_InitialisationSegmentRange);
    m_InitialisationSegmentRange = NULL;
  }
  if (url)
  {
    if(std_strncmp(url,"http://",7))
    {
      PlaylistParser::ResolveURI(base_url,url,m_InitialisationSegmentUrl,tmpUrlLength);
    }
    else
    {
      tmpUrlLength = (int)std_strlen(url)+1;
    }
  }

  if(tmpUrlLength > 0)
  {
    if(url)
    {
        m_InitialisationSegmentUrl=(char*)QTV_Malloc(tmpUrlLength + 1);
        if(m_InitialisationSegmentUrl)
        {
          if(std_strncmp(url,"http://",7))
          {
            PlaylistParser::ResolveURI(base_url,url,m_InitialisationSegmentUrl,tmpUrlLength);
          }
          else
          {
           std_strlcpy(m_InitialisationSegmentUrl,url,tmpUrlLength);
          }
        }
    }
  }
}

/* @brief - set start and end offset of init segments
*/
void RepresentationGroup::SetInitialisationSegmentRange(int64 nStartOffset, int64 nEndOffset, char *range)
{
  m_nStartOffsetInitSeg = nStartOffset;
  m_nEndOffsetInitSeg = nEndOffset;

  int tmpUrlLength = 0;
  if(m_InitialisationSegmentRange)
  {
    QTV_Free(m_InitialisationSegmentRange);
    m_InitialisationSegmentRange = NULL;
  }

  if(range)
  {
    tmpUrlLength = (int)std_strlen(range)+1;
    m_InitialisationSegmentRange=(char*)QTV_Malloc(tmpUrlLength + 1);
    if(m_InitialisationSegmentRange)
    {
      std_strlcpy(m_InitialisationSegmentRange,range,tmpUrlLength);
    }
  }
}

/* @brief: sets representation segment url
 */
void RepresentationGroup::SetRepresentationIndex(URLType *representationIndex)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetRepresentationIndex(representationIndex);
  }
}

char* RepresentationGroup::GetInitialisationSegmentUrl()
{
  return m_InitialisationSegmentUrl;
}

char* RepresentationGroup::GetInitialisationSegmentRange()
{
  return m_InitialisationSegmentRange;
}

void RepresentationGroup::GetRangeInitialisationSegment(int64& nStartOffset, int64& nEndOffset)
{
  nStartOffset = m_nStartOffsetInitSeg;
  nEndOffset = m_nEndOffsetInitSeg;
}

/* @brief: sets segment timeline
 */
bool RepresentationGroup::SetSegmentTimeline(uint32 segTimelineIndex, uint64 startTime,
                         uint64 duration, int32 repeatCount)
{
  bool bOk = false;
  if (m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetSegmentTimeline(segTimelineIndex, startTime,
                                            duration, repeatCount);
    bOk = true;
  }
  else if (m_pSegmentList)
  {
    m_pSegmentList->SetSegmentTimeline(segTimelineIndex, startTime,
                                            duration, repeatCount);
  }
  return bOk;
}

/* @brief: compare two representation's mediastremastructreid
 */
bool RepresentationGroup::CompareStreamStructId(RepresentationInfo *repInfo1,
                                               RepresentationInfo * repInfo2)
{
  bool bOk = false;
  for (uint32 i = 0; i < repInfo1->GetNumMediaStreamStructEntry(); i++)
  {
    for (uint32 j = 0; j < repInfo2->GetNumMediaStreamStructEntry(); j++)
    {
      if (repInfo1->GetMediaStreamStructureId(i) ==
                     repInfo2->GetMediaStreamStructureId(j))
      {
        bOk = true;
         break;
      }
    }
  }
  return bOk;
}

/* ***************************************************************************
Representation Info Class
***************************************************************************
*/

/* @brief - RepresentationInfo Constructor
*/
RepresentationInfo::RepresentationInfo()
:m_url(NULL),
m_nNumurls(0),
m_nbandwidth(0),
m_codec(NULL),
m_resolution(NULL),
m_nNumSegments(0),
m_nRefreshTime(0),
m_nSize(0),
m_pSegmentInfo(NULL),
m_bIsAvailable(false),
m_nFrameRate(0),
m_SamplingRate(NULL),
m_nNumSamplingRates(0),
m_Channels(NULL),
m_nNumChannels(0),
m_nParX(0),
m_nParY(0),
m_language(NULL),
m_MimeType(NULL),
m_nRepIndex(0),
m_nKey(0),
m_RepId(NULL),
m_InitialisationSegmentUrl(NULL),
m_InitialisationSegmentRange(NULL),
m_nStartOffsetInitSeg(0),
m_nEndOffsetInitSeg(-1),
m_pIndexSegRange(NULL),
m_bSegmentAlinged(false),
m_bMediaStreamStructMatchedWithOtherRep(false),
m_pSegmentTemplate(NULL),
m_bSegmentTemplateFound(false),
m_MediaStreamStructureId(NULL),
m_nNumMediaStreamStructEntry(0),
m_nStartWithSAPVal(-1),
m_nSubSegmentStartsWithSAP(-1),
m_pSegmentBase(NULL),
m_bSegmentBaseFound(false),
m_pSegmentList(NULL),
m_nNumSegURL(0),
m_bSegmentListFound(false),
m_CachedPTSOffset(0),
m_bIsSelectable(false),
m_bIsSelected(false),
m_pByteRangeURLTemplate(NULL),
m_BaseURL(NULL),
m_pSegmentFunc(&m_SegmentFuncDefault),
m_bIsSegmentFuncInitialized(false),
m_nTimeShiftBufferDepth(0.0),
m_nMinUpdatePeriod(0.0)
{
 m_bEstimator = NULL;
}
/* @brief - RepresentationInfo Destructor
*/
RepresentationInfo::~RepresentationInfo()
{
  if(m_BaseURL)
  {
    QTV_Free(m_BaseURL);
    m_BaseURL = NULL;
  }

  if(m_url)
  {
    for(int i=0;i<m_nNumurls;i++)
    {
      QTV_Free(m_url[i]);
      m_url[i]=NULL;
    }
    QTV_Free(m_url);
    m_url = NULL;
  }
  if(m_RepId)
  {
    QTV_Free(m_RepId);
    m_RepId = NULL;
  }
  if(m_language)
  {
    QTV_Free(m_language);
    m_language = NULL;
  }

  if(m_MimeType)
  {
    QTV_Free(m_MimeType);
    m_MimeType = NULL;
  }
   if(m_Channels)
    {
      QTV_Free(m_Channels);
      m_Channels = NULL;
    }
   if(m_SamplingRate)
    {
      QTV_Free(m_SamplingRate);
      m_SamplingRate = NULL;
    }
  if(m_codec)
  {
    if(m_codec->mcodecs)
    {
      QTV_Free(m_codec->mcodecs);
      m_codec->mcodecs=NULL;
    }
    QTV_Free(m_codec);
    m_codec=NULL;
  }
  if(m_resolution)
  {
    QTV_Free(m_resolution);
    m_resolution = NULL;
  }
  if(m_pSegmentInfo)
  {
    QTV_Delete_Array(m_pSegmentInfo);
    m_pSegmentInfo = NULL;
  }
  if(m_InitialisationSegmentUrl)
  {
    QTV_Free(m_InitialisationSegmentUrl);
    m_InitialisationSegmentUrl = NULL;
  }
  if(m_InitialisationSegmentRange)
  {
    QTV_Free(m_InitialisationSegmentRange);
    m_InitialisationSegmentRange = NULL;
  }
  if(m_pIndexSegRange)
  {
    QTV_Free(m_pIndexSegRange);
    m_pIndexSegRange = NULL;
  }
  if (m_MediaStreamStructureId)
  {
    QTV_Delete_Array(m_MediaStreamStructureId);
    m_MediaStreamStructureId = NULL;
  }
  if (m_pSegmentBase)
  {
    QTV_Delete(m_pSegmentBase);
    m_pSegmentBase = NULL;
  }
  if (m_pSegmentList)
  {
    QTV_Delete(m_pSegmentList);
    m_pSegmentList = NULL;
  }
  if (m_pSegmentTemplate)
  {
    QTV_Delete(m_pSegmentTemplate);
    m_pSegmentTemplate = NULL;
  }
  if (m_pByteRangeURLTemplate)
  {
    QTV_Delete(m_pByteRangeURLTemplate);
    m_pByteRangeURLTemplate = NULL;
  }
}
/* @brief: Copy function will copy all attributes except Segment info array
*/
void RepresentationInfo::Copy(const RepresentationInfo &rep_info)
{
  if(m_url)
  {
    for(int i=0;i<m_nNumurls;i++)
    {
      QTV_Free(m_url[i]);
      m_url[i]=NULL;
    }
    QTV_Free(m_url);
    m_url=NULL;
  }
  if(rep_info.m_url)
  {
    m_url = (char**)QTV_Malloc(rep_info.m_nNumurls*sizeof(char*));
    if(m_url)
    {
      m_nNumurls=rep_info.m_nNumurls;
      for(int i=0;i<m_nNumurls;i++)
      {
        m_url[i]=(char*)QTV_Malloc(std_strlen(rep_info.m_url[i])+1);
        if(m_url[i])
        {
          (void)std_strlcpy(m_url[i],rep_info.m_url[i],std_strlen(rep_info.m_url[i])+1);
        }
      }
    }
  }

  if(m_BaseURL)
  {
    QTV_Free(m_BaseURL);
    m_BaseURL = NULL;
  }

  if(rep_info.m_BaseURL)
  {
    int reqdBufSize = 1 + (int)std_strlen(rep_info.m_BaseURL);
    m_BaseURL = (char *)QTV_Malloc(reqdBufSize *sizeof(char));
    if (m_BaseURL)
    {
      std_strlcpy(m_BaseURL, rep_info.m_BaseURL, reqdBufSize);
    }
  }

  m_nbandwidth = rep_info.m_nbandwidth;
  m_nFrameRate = rep_info.m_nFrameRate;
  m_nNumSamplingRates = rep_info.m_nNumSamplingRates;
  m_nNumChannels = rep_info.m_nNumChannels;
  m_nParX = rep_info.m_nParX;
  m_nParY = rep_info.m_nParY;
  m_nKey = rep_info.m_nKey;
  m_bEstimator = rep_info.m_bEstimator;
  m_bSegmentAlinged = rep_info.m_bSegmentAlinged;
  m_bMediaStreamStructMatchedWithOtherRep = rep_info.m_bMediaStreamStructMatchedWithOtherRep;
  m_nStartWithSAPVal = rep_info.m_nStartWithSAPVal;
  m_nSubSegmentStartsWithSAP = rep_info.m_nSubSegmentStartsWithSAP;

  if (rep_info.m_MediaStreamStructureId)
  {
    if (m_MediaStreamStructureId)
    {
      QTV_Delete_Array(m_MediaStreamStructureId);
      m_MediaStreamStructureId = NULL;
    }
    m_MediaStreamStructureId = (MediaStreamStructId *)QTV_New_Array(MediaStreamStructId,
                                                          m_nNumMediaStreamStructEntry);
    for (uint32 i = 0; i < m_nNumMediaStreamStructEntry; i++)
    {
      m_MediaStreamStructureId[i] = rep_info.m_MediaStreamStructureId[i];
    }
  }
  if(rep_info.m_language)
  {
    if(m_language)
    {
      QTV_Free(m_language);
      m_language = NULL;
    }
    m_language = (char*)QTV_Malloc(std_strlen(rep_info.m_language)+1);
    if(m_language)
    {
      std_strlcpy(m_language,rep_info.m_language,std_strlen(rep_info.m_language)+1);
    }
  }
  if(rep_info.m_MimeType)
  {
    if(m_MimeType)
    {
      QTV_Free(m_MimeType);
    }
    int reqdSize = (int)std_strlen(rep_info.m_MimeType) + 1;
    m_MimeType = (char *)QTV_Malloc(reqdSize * sizeof(char));
    if(m_MimeType)
    {
      std_strlcpy(m_MimeType, rep_info.m_MimeType, reqdSize);
    }
  }
  if(rep_info.m_Channels)
  {
    if(m_Channels)
    {
      QTV_Free(m_Channels);
      m_Channels = NULL;
    }
    m_Channels = (double*)QTV_Malloc(sizeof(double)*m_nNumChannels);
    if(m_Channels)
    {
      for(int i=0;i<m_nNumChannels;i++)
      {
        m_Channels[i] = rep_info.m_Channels[i];
      }
    }
  }
  if(rep_info.m_SamplingRate)
  {
    if(m_SamplingRate)
    {
      QTV_Free(m_SamplingRate);
      m_SamplingRate = NULL;
    }
    m_SamplingRate = (uint32*)QTV_Malloc(sizeof(uint32)*m_nNumSamplingRates);
    if(m_SamplingRate)
    {
      for(int i=0;i<m_nNumSamplingRates;i++)
      {
        m_SamplingRate[i] = rep_info.m_SamplingRate[i];
      }
    }
  }


  if(rep_info.m_codec)
  {
    if(m_codec)
    {
      if(m_codec->mcodecs)
      {
        QTV_Free(m_codec->mcodecs);
        m_codec->mcodecs=NULL;
      }
      QTV_Free(m_codec);
      m_codec=NULL;
    }
    m_codec = (Codecs*)QTV_Malloc(sizeof(Codecs));
    if(m_codec)
    {
      m_codec->numcodecs=rep_info.m_codec->numcodecs;
      m_codec->mcodecs=(CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(m_codec->numcodecs));
      if(m_codec->mcodecs)
      {
        for(int i=0;i<m_codec->numcodecs;i++)
        {
          m_codec->mcodecs[i]=rep_info.m_codec->mcodecs[i];
        }
      }
    }
  }
  if(rep_info.m_resolution)
  {
    if(m_resolution)
    {
      QTV_Free(m_resolution);
      m_resolution=NULL;
    }
    m_resolution=(Resolution*)QTV_Malloc(sizeof(Resolution));
    if(m_resolution)
    {
      m_resolution->height = rep_info.m_resolution->height;
      m_resolution->width = rep_info.m_resolution->width;
    }
  }
  if(rep_info.m_RepId)
  {
    if(m_RepId)
    {
      QTV_Free(m_RepId);
      m_RepId = NULL;
    }
    m_RepId = (char*)QTV_Malloc(std_strlen(rep_info.m_RepId)+1);
    if(m_RepId)
    {
       std_strlcpy(m_RepId,rep_info.m_RepId,std_strlen(rep_info.m_RepId)+1);
    }

  }
  if(rep_info.m_InitialisationSegmentUrl)
  {
    if(m_InitialisationSegmentUrl)
    {
      QTV_Free(m_InitialisationSegmentUrl);
      m_InitialisationSegmentUrl = NULL;
    }
    m_InitialisationSegmentUrl = (char*)QTV_Malloc(std_strlen(rep_info.m_InitialisationSegmentUrl)+1);
    if(m_InitialisationSegmentUrl)
    {
      std_strlcpy(m_InitialisationSegmentUrl,rep_info.m_InitialisationSegmentUrl,std_strlen(rep_info.m_InitialisationSegmentUrl)+1);
    }
  }
  if(rep_info.m_InitialisationSegmentRange)
  {
    if(m_InitialisationSegmentRange)
    {
      QTV_Free(m_InitialisationSegmentRange);
      m_InitialisationSegmentRange = NULL;
    }
    m_InitialisationSegmentRange = (char*)QTV_Malloc(std_strlen(rep_info.m_InitialisationSegmentRange)+1);
    if(m_InitialisationSegmentRange)
    {
      std_strlcpy(m_InitialisationSegmentRange,rep_info.m_InitialisationSegmentRange,std_strlen(rep_info.m_InitialisationSegmentRange)+1);
    }
  }
  if (rep_info.m_pIndexSegRange)
  {
    if (m_pIndexSegRange)
    {
      QTV_Free(m_pIndexSegRange);
      m_pIndexSegRange = NULL;
    }
    m_pIndexSegRange = (char *)QTV_Malloc(std_strlen(rep_info.m_pIndexSegRange) + 1);
    if(m_pIndexSegRange)
    {
      std_strlcpy(m_pIndexSegRange, rep_info.m_pIndexSegRange,std_strlen(rep_info.m_pIndexSegRange) + 1);
    }
  }

if (rep_info.m_pByteRangeURLTemplate)
  {
    if (m_pByteRangeURLTemplate)
    {
      QTV_Free(m_pByteRangeURLTemplate);
      m_pByteRangeURLTemplate = NULL;
    }
    m_pByteRangeURLTemplate = (char *)QTV_Malloc(std_strlen(rep_info.m_pByteRangeURLTemplate) + 1);
    if(m_pByteRangeURLTemplate)
    {
      std_strlcpy(m_pByteRangeURLTemplate, rep_info.m_pByteRangeURLTemplate,std_strlen(rep_info.m_pByteRangeURLTemplate) + 1);
    }
  }
  m_nStartOffsetInitSeg = rep_info.m_nStartOffsetInitSeg;
  m_nEndOffsetInitSeg =   rep_info.m_nEndOffsetInitSeg;
  m_nNumSegments = rep_info.m_nNumSegments;
  m_nSize = rep_info.m_nSize;
  m_nRefreshTime = rep_info.m_nRefreshTime;
  m_bIsAvailable = rep_info.m_bIsAvailable;
  if(m_pSegmentInfo)
  {
    QTV_Delete_Array(m_pSegmentInfo);
    m_pSegmentInfo = NULL;
  }

  m_CachedPTSOffset = rep_info.m_CachedPTSOffset;

  m_CommonDescs.CopyDescs(rep_info.m_CommonDescs);
  m_CommonStringValues.CopyStringValues(rep_info.m_CommonStringValues);

  m_bIsSelectable = rep_info.m_bIsSelectable;
  m_bIsSelected = rep_info.m_bIsSelected;


  if (rep_info.m_bSegmentBaseFound)
  {
    if(InitialiseSegmentBase())
    {
      m_pSegmentBase->InheritSegmentBase(rep_info.m_pSegmentBase);
    }
  }
  else if(rep_info.m_bSegmentTemplateFound)
  {
    if(InitialiseSegmentTemplate())
    {
      m_pSegmentTemplate->Copy(rep_info.m_pSegmentTemplate);
    }
  }
  else if(rep_info.m_bSegmentListFound)
  {
    if(InitialiseSegmentList())
    {
      m_pSegmentList->Copy(rep_info.m_pSegmentList);
    }
  }

  m_bIsSegmentFuncInitialized = rep_info.m_bIsSegmentFuncInitialized;
  SetSegmentFuncToTemplate(m_bIsSegmentFuncInitialized);
}

/* @brief Copy constructor
*/
RepresentationInfo::RepresentationInfo(const RepresentationInfo &rep_info)
:m_url(NULL),
m_nNumurls(0),
m_nbandwidth(0),
m_codec(NULL),
m_resolution(NULL),
m_nNumSegments(0),
m_nRefreshTime(0),
m_nSize(0),
m_pSegmentInfo(NULL),
m_bIsAvailable(false),
m_nFrameRate(0),
m_SamplingRate(NULL),
m_nNumSamplingRates(0),
m_Channels(NULL),
m_nNumChannels(0),
m_nParX(0),
m_nParY(0),
m_language(NULL),
m_MimeType(NULL),
m_nKey(0),
m_RepId(NULL),
m_InitialisationSegmentUrl(NULL),
m_InitialisationSegmentRange(NULL),
m_nStartOffsetInitSeg(0),
m_nEndOffsetInitSeg(0),
m_pIndexSegRange(NULL),
m_bSegmentAlinged(false),
m_bMediaStreamStructMatchedWithOtherRep(false),
m_pSegmentTemplate(NULL),
m_bSegmentTemplateFound(false),
m_MediaStreamStructureId(NULL),
m_nNumMediaStreamStructEntry(0),
m_nStartWithSAPVal(-1),
m_nSubSegmentStartsWithSAP(-1),
m_pSegmentBase(NULL),
m_bSegmentBaseFound(false),
m_pSegmentList(NULL),
m_nNumSegURL(0),
m_bSegmentListFound(false),
m_bIsSelectable(false),
m_bIsSelected(false),
m_pByteRangeURLTemplate(NULL),
m_BaseURL(NULL),
m_pSegmentFunc(&m_SegmentFuncDefault),
m_bIsSegmentFuncInitialized(false),
m_nTimeShiftBufferDepth(0.0),
m_nMinUpdatePeriod(0.0)
{
  Copy(rep_info);
  if(rep_info.m_pSegmentInfo)
  {
    m_pSegmentInfo = QTV_New_Array(SegmentInfo,m_nNumSegments);
    for(uint32 i=0;i<m_nNumSegments;i++)
    {
      m_pSegmentInfo[i] = rep_info.m_pSegmentInfo[i];
    }
  }
}
/* @brief:  Assignment Operator overloading
*/
RepresentationInfo& RepresentationInfo::operator=(const RepresentationInfo& rep_info)
{
  Copy(rep_info);
  if(rep_info.m_pSegmentInfo)
  {
    m_pSegmentInfo = QTV_New_Array(SegmentInfo,m_nNumSegments);
    for(uint32 i=0;i<m_nNumSegments;i++)
    {
      m_pSegmentInfo[i] = rep_info.m_pSegmentInfo[i];
    }
  }
  return *this;
}

double RepresentationInfo::getFrameRate()
{
  return m_nFrameRate;
}
uint64 RepresentationInfo::getKey()
{
  return m_nKey;
}
char* RepresentationInfo::getRepIdentifier()
{
  return m_RepId;
}

uint32 RepresentationInfo::getBandwidth()
{
  return m_nbandwidth;
}
/*
* @brief
* Returns the codecs for the representation
*/
bool RepresentationInfo::getCodec(CodecInfo* pCodecs,int& numCodecs)
{
  bool ret = false;
  if(m_codec && m_codec->numcodecs > 0)
  {
    ret = true;
    if(numCodecs < m_codec->numcodecs)
    {
      numCodecs = m_codec->numcodecs;
    }
    else if(pCodecs)
    {
      for(int i=0;i<m_codec->numcodecs;i++)
      {
        pCodecs[i]=m_codec->mcodecs[i];
      }
    }
  }
  else if(numCodecs > 0)
  {
    if(pCodecs)
    {
      for(int i=0;i<numCodecs;i++)
      {
        pCodecs[i].Reset();
      }
    }
    numCodecs = 0;
  }
  return ret;
}

/*
* @brief
* Returns the resolution information for the representation
*/
Resolution* RepresentationInfo::getResolution()
{
  return m_resolution;
}

void RepresentationInfo::setRepInfo(char *ident,uint32 bandwidth,Codecs* codec,
                                    const char *mimeType,
                                    Resolution *presolution,char *language,
                                    uint32 parX,uint32 parY,uint64 key,
                                    uint32 frameRate,int numChannels,double *pChannels,
                                    int numSamplingRates, double *pSamplingRates,
                                    bool bSegmentAligned, int sapVal, int subSegSAP,
                                    char*  mediaStreamStructureId)
{
   m_nbandwidth = bandwidth;
   m_nParX = parX;
   m_nParY = parY;
   m_nKey = key;
   m_nFrameRate = (double)frameRate;
   m_nNumSamplingRates = numSamplingRates;
   m_nNumChannels = numChannels;
   m_bSegmentAlinged = bSegmentAligned;
   m_nStartWithSAPVal = sapVal;
   m_nSubSegmentStartsWithSAP = subSegSAP;
   SetMediaStreamStructureId(mediaStreamStructureId);
   if(m_SamplingRate)
   {
     QTV_Free(m_SamplingRate);
     m_SamplingRate = NULL;
   }
   if(pSamplingRates)
   {
     m_SamplingRate = (uint32*)QTV_Malloc(sizeof(uint32)*m_nNumSamplingRates);
     if(m_SamplingRate)
     {
       for(int i=0;i<m_nNumSamplingRates;i++)
       {
         m_SamplingRate[i] = (uint32)pSamplingRates[i];
       }
     }
   }
   if(m_Channels)
   {
     QTV_Free(m_Channels);
     m_Channels = NULL;
   }
   if(pChannels)
   {
     m_Channels = (double*)QTV_Malloc(sizeof(double)*m_nNumChannels);
     if(m_Channels)
     {
       for(int i=0;i<m_nNumChannels;i++)
       {
         m_Channels[i] = pChannels[i];
       }
     }
   }

   if(language)
   {
     if(m_language)
     {
       QTV_Free(m_language);
       m_language = NULL;
     }
     m_language =(char*)QTV_Malloc(std_strlen(language)+1);
     if(m_language)
     {
       std_strlcpy(m_language,language,std_strlen(language)+1);
     }
   }

   if(mimeType)
   {
     if(m_MimeType)
     {
       QTV_Free(m_MimeType);
     }

     int reqdSize = (int)std_strlen(mimeType) + 1;
     m_MimeType = (char *)QTV_Malloc(reqdSize);

     if(m_MimeType)
     {
       std_strlcpy(m_MimeType, mimeType, reqdSize);
     }
   }
  if(ident)
  {
    if(m_RepId)
    {
      QTV_Free(m_RepId);
      m_RepId = NULL;
    }
     m_RepId =(char*)QTV_Malloc(std_strlen(ident)+1);
     if(m_RepId)
     {
       std_strlcpy(m_RepId,ident,std_strlen(ident)+1);
     }
  }
  if(codec)
  {
    if(m_codec)
    {
      if(m_codec->mcodecs)
      {
        QTV_Free(m_codec->mcodecs);
        m_codec->mcodecs=NULL;
      }
      QTV_Free(m_codec);
      m_codec=NULL;
    }
    m_codec = (Codecs*)QTV_Malloc(sizeof(Codecs));
    if(m_codec)
    {
      m_codec->numcodecs=codec->numcodecs;
      m_codec->mcodecs=(CodecInfo*)QTV_Malloc(sizeof(CodecInfo)*(m_codec->numcodecs));
      if(m_codec->mcodecs)
      {
        for(int i=0;i<m_codec->numcodecs;i++)
        {
          m_codec->mcodecs[i].majorType = codec->mcodecs[i].majorType;
          m_codec->mcodecs[i].minorType = codec->mcodecs[i].minorType;
          m_codec->mcodecs[i].profile = codec->mcodecs[i].profile;
          m_codec->mcodecs[i].level = codec->mcodecs[i].level;
        }
      }
    }
  }
  if(presolution)
  {
    if(m_resolution)
    {
      QTV_Free(m_resolution);
      m_resolution=NULL;
    }
    m_resolution=(Resolution*)QTV_Malloc(sizeof(Resolution));
    if(m_resolution)
    {
      m_resolution->height = presolution->height;
      m_resolution->width = presolution->width;
    }
  }
}

void RepresentationInfo::UpdatePeriodKey(uint64 periodKey)
{
  // zero out the period key part of the rep key.
  m_nKey = m_nKey & (~MPD_PERIOD_MASK);
  // set the updated period key in the rep key
  m_nKey = m_nKey | (periodKey & MPD_PERIOD_MASK);

  uint32 numSegments = 0, tmpSize = 0;
  SegmentInfo *newSegmentArray = getSegmentInfo(numSegments, tmpSize);

  if (newSegmentArray)
  {
    for (uint32 i = 0; i < numSegments; ++i)
    {
      newSegmentArray[i].UpdatePeriodKey(periodKey);
    }
  }
}

/*
 * @brief: Sets the segment info for representation
*/
void RepresentationInfo::setSegmentInfo(SegmentInfo* pSegmentInfo,
                                        uint32 numSegments,uint32 size)
{
  m_nNumSegments = numSegments;
  if(m_pSegmentInfo)
  {
    QTV_Delete_Array(m_pSegmentInfo);
    m_pSegmentInfo = NULL;
  }
  m_pSegmentInfo = QTV_New_Array(SegmentInfo,size);
  if(pSegmentInfo)
  {
    for(uint32 i=0;i<size;i++)
    {
      m_pSegmentInfo[i] = pSegmentInfo[i];
    }
  }
  m_nSize = size;
  return;
}
/* @brief: Gets the segment Info for the representation
*/
SegmentInfo* RepresentationInfo::getSegmentInfo(uint32& numsegments,uint32& size)
{
  numsegments = m_nNumSegments;
  size = m_nSize;
  return m_pSegmentInfo;
}

/**
 * Retrieve the segmentInfo from the segment url.
 */
SegmentInfo* RepresentationInfo::GetSegmentInfoByURL(const char *url)
{
  SegmentInfo *segmentInfo = NULL;
  uint32 numSegments = 0, tmpSize = 0;
  SegmentInfo *segmentInfoArray = getSegmentInfo(numSegments, tmpSize);

  if (url && segmentInfoArray)
  {
    for (uint32 i = 0; i < numSegments; ++i)
    {
      if (segmentInfoArray[i].GetURL() && (std_stricmp(url, segmentInfoArray[i].GetURL()) == 0))
      {
        segmentInfo = &segmentInfoArray[i];
        break;
      }
    }
  }

  return segmentInfo;
}

/**
 * Retrieves segment info from the segmentStartTime. Since,
 * segment start times are computed by dividing by timescale,
 * there is a possibility of rounding error. So, for comparing
 * against 2 startTimes, equality is assumed if the difference
 * between them is less than 0.001ms. That is a fragment will
 * not be of duration less than 0.001ms.
 */
SegmentInfo* RepresentationInfo::GetSegmentInfoByStartTime(double segmentStartTime)
{
  SegmentInfo *segmentInfo = NULL;
  uint32 numSegments = 0, tmpSize = 0;
  SegmentInfo *segmentInfoArray = getSegmentInfo(numSegments, tmpSize);

  for (uint32 i = 0; i < numSegments; ++i)
  {
    double searchStartTime = segmentInfoArray[i].getStartTime();
    double diff = (searchStartTime >= segmentStartTime
                   ? searchStartTime - segmentStartTime
                   : segmentStartTime - searchStartTime);

    if (diff < 0.001)
    {
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "GetSegmentInfoByStartTime: diff between %lf and %lf is %lf",
                    segmentStartTime, searchStartTime, diff);

      // less than equal to 1ms. Assumption here is segment sizes will not be less than 0.001 ms.
      segmentInfo = &segmentInfoArray[i];
      break;
    }
  }

  return segmentInfo;
}

char* RepresentationInfo::GetByteRangeURLTemplate()
{
  return m_pByteRangeURLTemplate;
}

void RepresentationInfo::SetByteRangeURLTemplate(char* pByteRangeTemplate)
{
  //setting url byte range
  if (m_pByteRangeURLTemplate)
  {
    QTV_Free(m_pByteRangeURLTemplate);
    m_pByteRangeURLTemplate = NULL;
  }

  if(pByteRangeTemplate)
  {
    m_pByteRangeURLTemplate = (char *)QTV_Malloc(std_strlen(pByteRangeTemplate)+1);
    if (m_pByteRangeURLTemplate)
    {
      std_strlcpy(m_pByteRangeURLTemplate, pByteRangeTemplate, std_strlen(pByteRangeTemplate)+1);
    }
  }
}

char* RepresentationInfo::GetInitialisationSegmentUrl()
{
  return m_InitialisationSegmentUrl;
}

char* RepresentationInfo::GetInitialisationSegmentRange()
{
  return m_InitialisationSegmentRange;
}

void RepresentationInfo::GetRangeInitialisationSegment(int64& nStartOffset, int64& nEndOffset)
{
  nStartOffset = m_nStartOffsetInitSeg;
  nEndOffset = m_nEndOffsetInitSeg;
}

void RepresentationInfo::SetInitialisationSegmentUrl(char *base_url, char *url)
{
  int tmpUrlLength = 0;
  if(m_InitialisationSegmentUrl)
  {
    QTV_Free(m_InitialisationSegmentUrl);
    m_InitialisationSegmentUrl = NULL;
  }
  if (url)
  {
    if(std_strncmp(url,"http://",7))
    {
      PlaylistParser::ResolveURI(base_url,url,m_InitialisationSegmentUrl,tmpUrlLength);
    }
    else
    {
      tmpUrlLength = (int)std_strlen(url)+1;
    }
  }

  if(tmpUrlLength > 0)
  {
    if(url)
    {
        m_InitialisationSegmentUrl=(char*)QTV_Malloc(tmpUrlLength + 1);
        if(m_InitialisationSegmentUrl)
        {
          if(std_strncmp(url,"http://",7))
          {
            PlaylistParser::ResolveURI(base_url,url,m_InitialisationSegmentUrl,tmpUrlLength);
          }
          else
          {
           std_strlcpy(m_InitialisationSegmentUrl,url,tmpUrlLength);
          }
        }
    }
  }
}

/* @brief - set start and end offset of init segments
*/
void RepresentationInfo::SetInitialisationSegmentRange(int64 nStartOffset, int64 nEndOffset, char *range)
{
  m_nStartOffsetInitSeg = nStartOffset;
  m_nEndOffsetInitSeg = nEndOffset;

  int tmpUrlLength = 0;
  if(m_InitialisationSegmentRange)
  {
    QTV_Free(m_InitialisationSegmentRange);
    m_InitialisationSegmentRange = NULL;
  }

  if(range)
  {
    tmpUrlLength = (int)std_strlen(range)+1;
    m_InitialisationSegmentRange=(char*)QTV_Malloc(tmpUrlLength + 1);
    if(m_InitialisationSegmentRange)
    {
      std_strlcpy(m_InitialisationSegmentRange,range,tmpUrlLength);
    }
  }
}

/* @brief - set byte range for index segments
*/
void RepresentationInfo::SetIndexSegmentRange(char *indexUrlRange)
{
  if (indexUrlRange)
  {
    if (m_pIndexSegRange)
    {
      QTV_Free(m_pIndexSegRange);
      m_pIndexSegRange = NULL;
    }
    m_pIndexSegRange = (char *)QTV_Malloc(std_strlen(indexUrlRange) + 1);
    if(m_pIndexSegRange)
    {
      std_strlcpy(m_pIndexSegRange, indexUrlRange,std_strlen(indexUrlRange) + 1);
    }
  }
}

/* @brief - Get byte Range for Index Segment
*/
char* RepresentationInfo::GetIndexSegmentRange()
{
  return m_pIndexSegRange;
}


/* initialises segment base
*/
bool RepresentationInfo::InitialiseSegmentBase()
{
  bool bOk = false;
  if (!m_bSegmentBaseFound)
  {
    if(m_pSegmentBase)
    {
      QTV_Delete(m_pSegmentBase);
      m_pSegmentBase = NULL;
    }
    m_pSegmentBase = QTV_New(SegmentBaseType);
    if (m_pSegmentBase)
    {
      bOk = true;
      m_bSegmentBaseFound = true;
    }
  }
  {
    bOk = true;
  }
  return bOk;
}

/* return segment base pointer
 */
SegmentBaseType *RepresentationInfo::GetSegmentBase()
{
  return m_pSegmentBase;
}

/*Inherit segment base info from AdaptationSet*/
bool RepresentationInfo::InheritSegmentBaseInfo(SegmentBaseType* grpSegmentBase)
{
  bool bOk = false;
  if (grpSegmentBase)
  {
    bOk =  InitialiseSegmentBase();
    if (bOk)
    {
      m_pSegmentBase->InheritSegmentBase(grpSegmentBase);
    }
    else
    {
      bOk = true;
    }
  }
  return bOk;
}

/* sets segment base related info
 */
void RepresentationInfo::SetSegmentBaseInfo(uint32 timeScale, uint64 presentationTimeOffset,
                        char *indexRange,bool indexRangeExact,
                        URLType *initialisation, URLType *representationIndex)
{
  if (m_pSegmentBase)
  {
    m_pSegmentBase->SetTimeScale(timeScale);
    m_pSegmentBase->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentBase->SetIndexRange(indexRange);
    m_pSegmentBase->SetIndexRangeExact(indexRangeExact);
    m_pSegmentBase->SetInitialisation(initialisation);
    m_pSegmentBase->SetRepresentationIndex(representationIndex);

    SetPTSOffset((timeScale > 0
                  ? (double)presentationTimeOffset/(double)timeScale
                  : (double)presentationTimeOffset));
  }
}


/* @brief: Initialises Segment template */
bool RepresentationInfo::InitialiseSegmentTemplate()
{
  bool bOk = false;
  if (!m_bSegmentTemplateFound)
  {
    if(m_pSegmentTemplate)
    {
      QTV_Delete(m_pSegmentTemplate);
      m_pSegmentTemplate = NULL;
    }
    m_pSegmentTemplate = QTV_New(SegmentTemplateType);
    if (m_pSegmentTemplate)
    {
      m_bSegmentTemplateFound = true;
      bOk = true;
    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}
//
/*Inherit segment base info from AdaptationSet*/
bool RepresentationInfo::InheritSegmentTemplate(SegmentTemplateType* grpSegmentTemplate)
{
  bool bOk = false;
  if (grpSegmentTemplate)
  {
    bOk =  InitialiseSegmentTemplate();
    if (bOk)
    {
      m_pSegmentTemplate->InheritMultiSegmentBaseInfo(grpSegmentTemplate);
      m_pSegmentTemplate->InheritSegmentTemplateInfo(grpSegmentTemplate);
    }
    else
    {
      bOk = true;
    }
  }
  return bOk;
}

/* Inherit SegmentList from AdaptationSet
 */

bool RepresentationInfo::InheritSegmentList(SegmentListType *grpSegmentList)
{
  bool bOk = false;
  if (grpSegmentList)
  {
    bOk =  InitialiseSegmentList();
    if (bOk)
    {
      m_pSegmentList->InheritMultiSegmentBaseInfo(grpSegmentList);
      // current level's segment list does not contain any segment url
      //then only inherit
      if (m_pSegmentList->GetNumSegmentUrl() == 0)
      {
        bOk = m_pSegmentList->InitialiseSegmentUrl(grpSegmentList->GetNumSegmentUrl());
        if (bOk)
        {
          for (int i = 0; i<grpSegmentList->GetNumSegmentUrl(); i++)
          {
            m_pSegmentList->InheritSegmentUrl(grpSegmentList, i);
          }
        }
        else
        {
          bOk = true;
        }
      }
    }
  }
  return bOk;
}


/* @brief: Initialises Segment timeline */
bool RepresentationInfo::InitializeSegmentTimeline(int numSEntry)
{
  bool bOk = false;
  if (numSEntry > 0)
  {
    if(m_pSegmentTemplate)
    {
      bOk = m_pSegmentTemplate->InitializeSegmentTimeline(numSEntry);
    }
    else if (m_pSegmentList)
    {
      bOk = m_pSegmentList->InitializeSegmentTimeline(numSEntry);
    }
  }
    return bOk;
}

/* @brief: Initialises Segment List */
bool RepresentationInfo::InitialiseSegmentList()
{
  bool bOk = false;
  if (!m_bSegmentListFound)
  {
    if(m_pSegmentList)
    {
      QTV_Delete(m_pSegmentList);
      m_pSegmentList = NULL;
    }
    m_pSegmentList=QTV_New(SegmentListType);
    if (m_pSegmentList)
    {
      m_bSegmentListFound = true;
      bOk = true;
    }
  }
  else
  {
    bOk = true;
  }
  return bOk;
}

bool RepresentationInfo::InitialiseSegmentUrl(int numSegUrl)
{
  bool bOk = false;
  if (numSegUrl > 0)
  {
    if (m_pSegmentList)
    {
      m_pSegmentList->InitialiseSegmentUrl(numSegUrl);
      bOk = true;
    }
  }
  return bOk;
}


SegmentListType* RepresentationInfo::GetSegmentList()
{
  return m_pSegmentList;
}


/* @brief : set Segment list elements
*/
bool RepresentationInfo::SetSegmentUrl(uint32 urlIndex, char *mediaUrl,
                         char *mediaRange, char *indexUrl, char *indexRange)
{
  bool bOk = false;
  if (m_pSegmentList)
  {
    m_pSegmentList->SetSegmentUrl(urlIndex, mediaUrl,
                                  mediaRange, indexUrl, indexRange);
    bOk = true;
  }
  return bOk;
}

/* @brief: sets segment template attributes*/
void RepresentationInfo::SetSegmentTemplateInfo(char *mediaTemplate,char *indexTemplate,
                                          char *initialisationTemplate, char *bitstreamSwitchingTemplate)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetMediaTemplate(mediaTemplate);
    m_pSegmentTemplate->SetIndexTemplate(indexTemplate);
    m_pSegmentTemplate->SetInitialisationTemplate(initialisationTemplate);
    m_pSegmentTemplate->SetBSSwitchingTemplate(bitstreamSwitchingTemplate);
  }

}

/* @brief: sets multiple segment base attributes*/
void RepresentationInfo::SetMultiSegmentBaseInfo( uint32 segmentDuration, uint32 startNumber,
                                                  uint32 timeScale, uint64 presentationTimeOffset,
                                                              char *indexRange,bool indexRangeExact,
                                                          URLType *initialisation, URLType *representationIndex)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetDuration(segmentDuration);
    m_pSegmentTemplate->SetStartNumber(startNumber);
    m_pSegmentTemplate->SetTimeScale(timeScale);
    m_pSegmentTemplate->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentTemplate->SetIndexRange(indexRange);
    m_pSegmentTemplate->SetIndexRangeExact(indexRangeExact);
    m_pSegmentTemplate->SetInitialisation(initialisation);
    m_pSegmentTemplate->SetRepresentationIndex(representationIndex);
  }
  else if (m_pSegmentList)
  {
    m_pSegmentList->SetDuration(segmentDuration);
    m_pSegmentList->SetStartNumber(startNumber);
    m_pSegmentList->SetTimeScale(timeScale);
    m_pSegmentList->SetPresentationOffset(presentationTimeOffset);
    m_pSegmentList->SetIndexRange(indexRange);
    m_pSegmentList->SetIndexRangeExact(indexRangeExact);
    m_pSegmentList->SetInitialisation(initialisation);
    m_pSegmentList->SetRepresentationIndex(representationIndex);
  }

  SetPTSOffset((timeScale > 0
                ? (double)presentationTimeOffset/(double)timeScale
                : (double)presentationTimeOffset));
}



/* @brief: sets representation segment url
 */
void RepresentationInfo::SetRepresentationIndex(URLType *representationIndex)
{
  if(m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetRepresentationIndex(representationIndex);
  }
}

/* @brief: sets segment timeline
 */
bool RepresentationInfo::SetSegmentTimeline(uint32 segTimelineIndex, uint64 startTime,
                         uint64 duration, int32 repeatCount)
{
  bool bOk = false;
  if (m_pSegmentTemplate)
  {
    m_pSegmentTemplate->SetSegmentTimeline(segTimelineIndex, startTime,
                                            duration, repeatCount);
        bOk = true;
  }
  else if (m_pSegmentList)
  {
    m_pSegmentList->SetSegmentTimeline(segTimelineIndex, startTime,
                                            duration, repeatCount);
        bOk = true;
  }
    return bOk;

}

/* @brief: sets mediastream structure id
 */
bool RepresentationInfo::SetMediaStreamStructureId(char *mediaStructID)
{
  bool bOk = false;
  if (mediaStructID)
  {
    int len = (int)std_strlen(mediaStructID) +  1;
    char *temp = NULL;
    int index = 0;
    int position = 0;
    if (m_MediaStreamStructureId)
    {
      QTV_Delete_Array(m_MediaStreamStructureId);
      m_MediaStreamStructureId = NULL;
    }
    m_MediaStreamStructureId = (MediaStreamStructId *)QTV_New_Array(MediaStreamStructId, len);
    temp = mediaStructID;
    while ( mediaStructID != NULL)
    {
      char tempVal[10];
      position = (std_strstr(mediaStructID," ") == 0)? -1 : (int)((std_strstr(mediaStructID," ") - mediaStructID));
      if (position >= 0)
      {
        std_strlcpy(tempVal, mediaStructID, position + 1);
      }
      else
      {
        std_strlcpy(tempVal, mediaStructID, std_strlen(mediaStructID)+1);
        m_MediaStreamStructureId[index].id = atoi(tempVal);
        index++;
        break;
      }
      m_MediaStreamStructureId[index].id = atoi(tempVal);
      mediaStructID = mediaStructID + position + 1;
      index++;
    }
    m_nNumMediaStreamStructEntry = index;
  }
  return bOk;
}

/* @brief: returns returns media stream structure id */
uint32 RepresentationInfo::GetMediaStreamStructureId(uint32 index)
{
  return m_MediaStreamStructureId[index].id;
}

/* @brief: returns segment template pointer*/
SegmentTemplateType *RepresentationInfo::GetSegmentTemplate()
{
  return m_pSegmentTemplate;
}

/* @brief: returns number segment timeline entry*/
int RepresentationInfo::GetNumSegmentTimelineEntry()
{
  int numEntry = 0;
  if(m_pSegmentTemplate)
  {
    numEntry = m_pSegmentTemplate->GetNumSegmentTimelineEntry();
  }
  else if (m_pSegmentList)
  {
    numEntry = m_pSegmentList->GetNumSegmentTimelineEntry();
  }
  return numEntry;
}

/* @brief: returns number of mediastream structure id count*/
uint32 RepresentationInfo::GetNumMediaStreamStructEntry()
{
  return m_nNumMediaStreamStructEntry;
}

/**
 * Get a reference to the ContentDescriptors that may be
 * specified at the adaptation-set level as well as the
 * representation-level eg "AudioChannelConfiguration").
 */
ContentDescriptorContainer& RepresentationInfo::GetCommonDescriptors()
{
  return m_CommonDescs;
}

ContentProtectionType& RepresentationInfo::GetContentProtection()
{
  return m_ContentProtection;
}

/**
 * Get a reference to the StringValues that may be
 * specified at the adaptation-set level as well as the
 * representation-level eg "mimeType").
 */
StringValueContainer& RepresentationInfo::GetCommonStringValues()
{
  return m_CommonStringValues;
}

/**
 * Get a reference to the ContentDescriptors that can be
 * specified at the adaptation-set level (eg "Accessibility").
 */
ContentDescriptorContainer& RepresentationGroup::GetAdaptationSetSpecificContentDescriptors()
{
  return m_AdaptationSetSpecificDescs;
}

/**
 * Get a reference to the ContentDescriptors that can occur at
 * the AdaptationSet level as well as the representation level.
 * Eg. "AudioChannelConfiguration"
 */
ContentDescriptorContainer& RepresentationGroup::GetCommonContentDescriptors()
{
  return m_CommonDescs;
}

ContentProtectionType& RepresentationGroup::GetContentProtection()
{
  return m_ContentProtection;
}

/**
 * Get a reference to the string-values that can be
 * specified at the adaptation-set level (eg "contentType").
 */
StringValueContainer& RepresentationGroup::GetAdaptationSetSpecificStringValueContainer()
{
  return m_AdaptationSetStringValues;
}

/**
 * Get a reference to the string-values that can be
 * specified at the adaptation-set level (eg "mimeType").
 */
StringValueContainer& RepresentationGroup::GetCommonStringValueContainer()
{
  return m_CommonStringValues;
}

/**
 * Check if any representation in this adaptation set is
 * selectable.
 */
bool RepresentationGroup::IsAnyRepSelectable()
{
  bool rslt = false;
  int numReps = 0;
  RepresentationInfo *repInfo = getRepInfo(numReps);
  for(int i = 0; i < numReps; ++i)
  {
    if (repInfo[i].IsMarkedSelectable())
    {
      rslt = true;
      break;
    }
  }
  return rslt;
}

/**
 * All representations that were marked as selectable are marked
 * as selected.
 */
void RepresentationGroup::MarkAllSelectableRepsAsSelected()
{
  int numReps = 0;
  RepresentationInfo *repInfo = getRepInfo(numReps);
  for(int i = 0; i < numReps; ++i)
  {
    // only mark a seletable rep as selected.
    repInfo[i].MarkSelected(repInfo[i].IsMarkedSelectable());
  }
}

/**
 * Check if any representation in this adaptation-set is
 * selected.
 */
bool RepresentationGroup::IsAnyRepSelected()
{
  bool rslt = false;
  int numReps = 0;
  RepresentationInfo *repInfo = getRepInfo(numReps);
  for(int i = 0; i < numReps; ++i)
  {
    if (repInfo[i].IsMarkedSelected())
    {
      rslt = true;
      break;
    }
  }
  return rslt;
}

/**
 * Set the 'selectable' flag on this representation. Reset the
 * 'selected' flag as marking a representation as selectable
 * means we are preparing to select representations.
 */
void RepresentationInfo::MarkSelectable(bool bIsSelectable)
{
  m_bIsSelectable = bIsSelectable;
  m_bIsSelected = false;
}

/**
 * Check if a representation is 'selectable'.
 */
bool RepresentationInfo::IsMarkedSelectable() const
{
  return m_bIsSelectable;
}

/**
 * Marks the representation as 'selected'. The caller should
 * have ensured that the representation was already marked
 * 'selectable'.
 */
void RepresentationInfo::MarkSelected(bool bSelectRep)
{
  m_bIsSelected = bSelectRep;

  if(m_bIsSelected)
  {
    uint64 key = getKey();

    uint32 period_index = (uint32)((key & MPD_PERIOD_MASK)>>56);
    uint32 grp_index  = (uint32)((key & MPD_REPGRP_MASK)>>50);
    uint32 rep_index = (uint32)((key & MPD_REPR_MASK) >> 40);

    //QTV_MSG_SPRINTF_PRIO_4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    //  "MarkSelected Representation with id '%s' key (%u,%u,%u)",
    //  m_RepId ? m_RepId : "unknown", period_index, grp_index, rep_index);
  }
}

/**
 * Check if the representation is marked as 'selected'.
 */
bool RepresentationInfo::IsMarkedSelected() const
{
  return m_bIsSelected;
}




/* ***************************************************************************
Segment Info Class
***************************************************************************
*/
/* @brief - SegmentInfo Constructor
*/
SegmentInfo::SegmentInfo()
:m_nStarttime(0),
m_nDuration(0),
m_nAvailabilitytime(0),
m_url(NULL),
m_pMediaRange(NULL),
m_pIndexUrl(NULL),
m_pIndexRange(NULL),
m_pInitRange(NULL),
m_key(0),
m_bdiscontinuous(false),
m_bIsAvailableOnServer(true),
m_bIsDeleted(false),
m_bIsLastSegment(false),
m_bIsAvailableForUse(true),
m_bAvailibilityTimeSet(false),
m_bIsInitializationSegment(false),
m_nContentLength(-1),
m_pSegDataLock(NULL),
m_pByteRangeUrl(NULL),
m_eByteRangeURLRespState(HTTPCommon::BYTERANGE_URL_USED_UNDETERMINED),
m_bIsIndexURLPresent(false),
m_bIsIndexRangeExact(false),
m_bIsInitURLPresent(false)
{
  if (0 != MM_CriticalSection_Create(&m_pSegDataLock))
  {
    MM_MSG_PRIO(MM_QSM, MM_PRIO_ERROR,
                "SegmentInfo::SegmentInfo() Failed to create m_pSegDataLock error");
  }

  m_bEstimator = NULL;
}
/* @brief - SegmentInfo Destructor
*/
SegmentInfo::~SegmentInfo()
{
  if(m_url)
  {
    QTV_Free(m_url);
    m_url = NULL;
  }
  if (m_pMediaRange)
  {
    QTV_Free(m_pMediaRange);
    m_pMediaRange = NULL;
  }
  if (m_pIndexUrl)
  {
    QTV_Free(m_pIndexUrl);
    m_pIndexUrl = NULL;
  }
  if (m_pIndexRange)
  {
    QTV_Free(m_pIndexRange);
    m_pIndexRange = NULL;
  }
  if (m_pInitRange)
  {
    QTV_Free(m_pInitRange);
    m_pInitRange = NULL;
  }
  if (m_pByteRangeUrl)
  {
    QTV_Free(m_pByteRangeUrl);
    m_pByteRangeUrl = NULL;
  }
  if(m_pSegDataLock)
  {
    MM_CriticalSection_Release(m_pSegDataLock);
    m_pSegDataLock = NULL;
  }
}
/* @brief: SegmentInfo copy method. Will copy all attributes except
decryption information */
void SegmentInfo::Copy(const SegmentInfo &segment_info)
{
  m_nStarttime = segment_info.m_nStarttime;
  m_nDuration = segment_info.m_nDuration;
  m_nAvailabilitytime = segment_info.m_nAvailabilitytime;
  if(m_url)
  {
    QTV_Free(m_url);
    m_url=NULL;
  }
  if(segment_info.m_url)
  {
    m_url=(char*)QTV_Malloc(std_strlen(segment_info.m_url)+1);
    if(m_url)
    {
      (void)std_strlcpy(m_url,segment_info.m_url,
      std_strlen(segment_info.m_url)+1);
    }
  }

  if(m_pMediaRange)
  {
    QTV_Free(m_pMediaRange);
    m_pMediaRange=NULL;
  }
  if(segment_info.m_pMediaRange)
  {
    m_pMediaRange=(char*)QTV_Malloc(std_strlen(segment_info.m_pMediaRange)+1);
    if(m_pMediaRange)
    {
      (void)std_strlcpy(m_pMediaRange,segment_info.m_pMediaRange,
        std_strlen(segment_info.m_pMediaRange)+1);
    }
  }

  if(m_pIndexUrl)
  {
    QTV_Free(m_pIndexUrl);
    m_pIndexUrl=NULL;
  }
  if(segment_info.m_pIndexUrl)
  {
    m_pIndexUrl=(char*)QTV_Malloc(std_strlen(segment_info.m_pIndexUrl)+1);
    if(m_pIndexUrl)
    {
      (void)std_strlcpy(m_pIndexUrl,segment_info.m_pIndexUrl,
        std_strlen(segment_info.m_pIndexUrl)+1);
    }
  }

  if(m_pIndexRange)
  {
    QTV_Free(m_pIndexRange);
    m_pIndexRange=NULL;
  }
  if(segment_info.m_pIndexRange)
  {
    m_pIndexRange=(char*)QTV_Malloc(std_strlen(segment_info.m_pIndexRange)+1);
    if(m_pIndexRange)
    {
      (void)std_strlcpy(m_pIndexRange,segment_info.m_pIndexRange,
        std_strlen(segment_info.m_pIndexRange)+1);
    }
  }

  if(m_pInitRange)
  {
    QTV_Free(m_pInitRange);
    m_pInitRange=NULL;
  }

  if(segment_info.m_pInitRange)
  {
    m_pInitRange=(char*)QTV_Malloc(std_strlen(segment_info.m_pInitRange)+1);
    if(m_pInitRange)
    {
      (void)std_strlcpy(m_pInitRange,segment_info.m_pInitRange,
        std_strlen(segment_info.m_pInitRange)+1);
    }
  }

  m_key =segment_info.m_key;
  m_bdiscontinuous= segment_info.m_bdiscontinuous;
  m_bIsAvailableOnServer= segment_info.m_bIsAvailableOnServer;
  m_bIsDeleted= segment_info.m_bIsDeleted;
  m_bIsLastSegment= segment_info.m_bIsLastSegment;
  m_bIsAvailableForUse= segment_info.m_bIsAvailableForUse ;
  m_bAvailibilityTimeSet = segment_info.m_bAvailibilityTimeSet;
  m_bEstimator = segment_info.m_bEstimator;
  m_nContentLength = segment_info.m_nContentLength;
  m_bIsIndexURLPresent = segment_info.m_bIsIndexURLPresent;
  m_bIsIndexRangeExact = segment_info.m_bIsIndexRangeExact;
  m_bIsInitURLPresent = segment_info.m_bIsInitURLPresent;

  m_eByteRangeURLRespState = segment_info.m_eByteRangeURLRespState;
  SetByteRangeURL(segment_info.m_pByteRangeUrl);

  }

/* @brief Assignment operator overloading
*/
SegmentInfo& SegmentInfo::operator=(const SegmentInfo& segment_info)
{
  Copy(segment_info);
  return *this;
}

/* Prints the values for the segment
*/
void SegmentInfo::Print()
{
  if(!m_bIsAvailableForUse)
  {
    if(m_url)
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
        "Segment key [%u %u ]",(uint32)(m_key >> 32),
        (uint32) (m_key));

      QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
        "Segment key [program %u period %u "
        "representation %u segment %u] start time %f",
        (uint32)(m_key >> 51),
        (uint32) ((m_key & PERIOD_MASK) >> 30) ,
        (uint32) ((m_key & REPRESENTATION_MASK) >> 21) ,
        (uint32) (m_key & SEGMENT_MASK) ,
        m_nStarttime);
      QTV_MSG_SPRINTF_1(QTVDIAG_HTTP_STREAMING,"url is %s",
        m_url);

    }
  }
}
/*
* @brief
* Returns the url for the segment
*/
char* SegmentInfo::GetURL()
{
  return m_url;
}
/*
* @brief
* Returns the range of url for the segment
*/

char* SegmentInfo::GetUrlRange()
{
  return m_pMediaRange;
}

/*
* @brief
* Retunrs the Index Url for the segment
*/
char* SegmentInfo::GetIndexURL()
{
  return m_pIndexUrl;
}

char* SegmentInfo::GetByteRangeURL()
{
  return m_pByteRangeUrl;
}

/*
* @brief
* Retunrs the Range of Index Url for the segment
*/
char* SegmentInfo::GetIndexUrlRange()
{
  return m_pIndexRange;
}

/*
* @brief
* Retunrs the Range of Init Url for the segment
*/
char* SegmentInfo::GetInitUrlRange()
{
  return m_pInitRange;
}

/*
* @brief
* Returns the start time for the segment(in seconds)
*/

double SegmentInfo::getStartTime()
{
  return m_nStarttime;
}
/*
* @brief
* Returns the duration for the segment (in seconds)
*/
double SegmentInfo::getDuration()
{
  return m_nDuration;
}
/*
* @brief
* Returns the availability time for the segment
*/
double SegmentInfo::getAvailabilitytime()
{
  return m_nAvailabilitytime;
}
/*
* @brief
* Returns true if there is a discontinuity between this
segment and the previous segment
*/
bool SegmentInfo::isDiscontinuous()
{
  return m_bdiscontinuous;
}
/*
* @brief
* Returns key for the current segment
*/
uint64 SegmentInfo::getKey()
{
  return m_key;
}

void SegmentInfo::SetByteRangeURL(char* pByteRangeUrl)
{
  if(m_pByteRangeUrl)
  {
    QTV_Free(m_pByteRangeUrl);
    m_pByteRangeUrl = NULL;
  }

  if(pByteRangeUrl)
  {
    int length = (int)std_strlen(pByteRangeUrl);
    m_pByteRangeUrl = (char *)QTV_Malloc(sizeof(char)*(length + 1));
    if(m_pByteRangeUrl)
    {
      std_strlcpy(m_pByteRangeUrl, pByteRangeUrl, length+1);
    }
  }
}

void SegmentInfo::SetStartTime(uint32 time)
{
  m_nStarttime= (double)time;
}
void SegmentInfo::SetAvailabilityTime(double time)
{
  if(!m_bAvailibilityTimeSet)
  {
    m_bAvailibilityTimeSet = true;
  }
  m_nAvailabilitytime=time;
}
void SegmentInfo::MarkLastSegment()
{
  m_bIsLastSegment = true;
}
void SegmentInfo::MarkDeleted()
{
  if(!m_bIsAvailableForUse)
  {
    m_bIsDeleted = true;
    if(!m_bIsAvailableOnServer)
    {
      MarkAvailableForUse();
    }
  }
}
bool SegmentInfo::IsAvailableForUse()
{
  return m_bIsAvailableForUse;
}
bool SegmentInfo::IsAvailableOnServer()
{
  return m_bIsAvailableOnServer;
}
void SegmentInfo::MarkRemovedFromServer()
{
  if(!m_bIsAvailableForUse)
  {
    m_bIsAvailableOnServer = false;
    if(m_bIsDeleted)
    {
      MarkAvailableForUse();
    }
  }
}
void SegmentInfo::MarkAvailableForUse()
{
  SetInfo(NULL,0,0,0,false,false);
  m_bIsAvailableForUse = true;
  m_bIsDeleted = false;
  m_bIsAvailableOnServer = true;
  m_nAvailabilitytime = 0;
  m_bIsLastSegment = false;
  m_bAvailibilityTimeSet = false;
}
bool SegmentInfo::IsLastSegment()
{
  return m_bIsLastSegment;
}

void SegmentInfo::SetContentLength(const int64 nContentLength)
{
  MM_CriticalSection_Enter(m_pSegDataLock);
  m_nContentLength = nContentLength;
  MM_CriticalSection_Leave(m_pSegDataLock);
}

int64 SegmentInfo::GetContentLength()
{
  int64 nContentLength;
  MM_CriticalSection_Enter(m_pSegDataLock);
  nContentLength = m_nContentLength;
  MM_CriticalSection_Leave(m_pSegDataLock);
  return nContentLength;
}

/* @brief Sets the given information for the segment
*/
void SegmentInfo::SetInfoBase(char* url, double duration,uint64 key,
                          double start_time, char* mediaRange, char* indexUrl, bool isIndexURLPresent,
                          char* indexRange, bool isIndexRangeExact, bool isInitURLPresent,
                          char* initRange, bool isdiscontinuous, bool /* isencrypted */)
{
  // setting media url
  if(m_url)
  {
    QTV_Free(m_url);
    m_url = NULL;
  }
  if(url)
  {
    m_url=(char*)QTV_Malloc(std_strlen(url)+1);
    if(m_url)
    {
      (void)std_strlcpy(m_url,url,std_strlen(url)+1);
    }
  }

  //setting media url range
  if (m_pMediaRange)
  {
    QTV_Free(m_pMediaRange);
    m_pMediaRange = NULL;
  }

  if(mediaRange)
  {
    m_pMediaRange = (char *)QTV_Malloc(std_strlen(mediaRange)+1);
    if (m_pMediaRange)
    {
      std_strlcpy(m_pMediaRange, mediaRange, std_strlen(mediaRange)+1);
    }
  }

  // setting index url
  int tmpUrlLength = 0;
  if (indexUrl)
  {
    if(std_strncmp(indexUrl,"http://",7))
    {
      PlaylistParser::ResolveURI(url,indexUrl, m_pIndexUrl,tmpUrlLength);
    }
    else
    {
      tmpUrlLength = (int)std_strlen(indexUrl)+1;
    }
  }

  if(tmpUrlLength > 0)
  {
    if(url)
    {
      if (m_pIndexUrl)
      {
        QTV_Free(m_pIndexUrl);
        m_pIndexUrl = NULL;
      }

      m_pIndexUrl =(char*)QTV_Malloc(tmpUrlLength + 1);
      if(m_pIndexUrl)
      {
        if(std_strncmp(indexUrl,"http://",7))
        {
          PlaylistParser::ResolveURI(url,indexUrl,m_pIndexUrl,tmpUrlLength);
        }
        else
        {
          std_strlcpy(m_pIndexUrl,indexUrl,tmpUrlLength);
        }
      }
    }
  }


  //setting index url range
  if (m_pIndexRange)
  {
    QTV_Free(m_pIndexRange);
    m_pIndexRange = NULL;
  }

  if(indexRange)
  {
    m_pIndexRange = (char *)QTV_Malloc(std_strlen(indexRange)+1);
    if (m_pIndexRange)
    {
      std_strlcpy(m_pIndexRange, indexRange, std_strlen(indexRange)+1);
    }
  }

  //setting init url range
  if (m_pInitRange)
  {
    QTV_Free(m_pInitRange);
    m_pInitRange = NULL;
  }

  if(initRange)
  {
    m_pInitRange = (char *)QTV_Malloc(std_strlen(initRange)+1);
    if (m_pInitRange)
    {
      std_strlcpy(m_pInitRange, initRange, std_strlen(initRange)+1);
    }
  }

  m_bIsIndexURLPresent = isIndexURLPresent;
  m_bIsIndexRangeExact = isIndexRangeExact;
  m_bIsInitURLPresent  = isInitURLPresent;

  m_nStarttime = start_time;
  m_nDuration = duration;
  m_key = key;
  m_bdiscontinuous = isdiscontinuous;
  m_bIsAvailableForUse = false;
}

/* @brief Sets the given information for the segment
*/
void SegmentInfo::SetInfo(char* url,double duration,uint64 key,
                          double start_time,bool isdiscontinuous,
                          bool /* isencrypted */)
{
  if(m_url)
  {
    QTV_Free(m_url);
    m_url = NULL;
  }
  if(url)
  {
    m_url=(char*)QTV_Malloc(std_strlen(url)+1);
    if(m_url)
    {
      (void)std_strlcpy(m_url,url,std_strlen(url)+1);
    }
  }
  m_nStarttime = start_time;
  m_nDuration = duration;
  m_key = key;
  m_bdiscontinuous = isdiscontinuous;
  m_bIsAvailableForUse = false;
}

/* @brief Sets the given information for the segment avoiding extra memory allocation
*/
bool SegmentInfo::SetInfoURL(char* base_url, char *url, char* mediaRange,
                  char* indexUrl, bool isIndexURLPresent, char* indexRange, bool isIndexRangeExact,
                  bool isInitURLPresent, char* initRange, double duration,uint64 key,
    double start_time,bool isdiscontinuous,
    bool /* isencrypted */)
{
  bool bOk = true;

  if(0.0 == duration)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "SetInfoURL segDuration zero");
    bOk = false;
  }
  else
  {
    int tmpUrlLength = 0;
    //setting media url
    if(m_url)
    {
      QTV_Free(m_url);
      m_url = NULL;
    }

    if (url)
    {
    if(std_strncmp(url,"http://",7))
    {
      PlaylistParser::ResolveURI(base_url,url,m_url,tmpUrlLength);
    }
    else
    {
      tmpUrlLength = (int)std_strlen(url)+1;
    }
    }

    if(tmpUrlLength > 0)
    {
      if(url)
      {
          m_url=(char*)QTV_Malloc(tmpUrlLength + 1);
        if(m_url)
        {
          if(std_strncmp(url,"http://",7))
          {
            PlaylistParser::ResolveURI(base_url,url,m_url,tmpUrlLength);
          }
          else
          {
            std_strlcpy(m_url,url,tmpUrlLength);
          }
        }
      }
    }

    //setting media url range
    if (m_pMediaRange)
    {
      QTV_Free(m_pMediaRange);
      m_pMediaRange = NULL;
    }

    if(mediaRange)
    {
      m_pMediaRange = (char *)QTV_Malloc(std_strlen(mediaRange)+1);
      if (m_pMediaRange)
      {
        std_strlcpy(m_pMediaRange, mediaRange, std_strlen(mediaRange)+1);
      }
    }

    // setting index url
    tmpUrlLength = 0;
    if (indexUrl)
    {
      if(std_strncmp(indexUrl,"http://",7))
      {
        PlaylistParser::ResolveURI(base_url,indexUrl, m_pIndexUrl,tmpUrlLength);
      }
      else
      {
        tmpUrlLength = (int)std_strlen(indexUrl)+1;
      }
    }

    if(tmpUrlLength > 0)
    {
      if(url)
      {
        if (m_pIndexUrl)
        {
          QTV_Free(m_pIndexUrl);
          m_pIndexUrl = NULL;
        }

        m_pIndexUrl =(char*)QTV_Malloc(tmpUrlLength + 1);
        if(m_pIndexUrl)
        {
          if(std_strncmp(indexUrl,"http://",7))
          {
            PlaylistParser::ResolveURI(base_url,indexUrl,m_pIndexUrl,tmpUrlLength);
          }
          else
          {
            std_strlcpy(m_pIndexUrl,indexUrl,tmpUrlLength);
          }
        }
      }
    }


    //setting index url range
    if (m_pIndexRange)
    {
      QTV_Free(m_pIndexRange);
      m_pIndexRange = NULL;
    }

    if(indexRange)
    {
      m_pIndexRange = (char *)QTV_Malloc(std_strlen(indexRange)+1);
      if (m_pIndexRange)
      {
        std_strlcpy(m_pIndexRange, indexRange, std_strlen(indexRange)+1);
      }
    }

    if(initRange)
    {
      m_pInitRange = (char *)QTV_Malloc(std_strlen(initRange)+1);
      if (m_pInitRange)
      {
        std_strlcpy(m_pInitRange, initRange, std_strlen(initRange)+1);
      }
    }

    m_nStarttime = start_time;
    m_nDuration = duration;
    m_key = key;
    m_bdiscontinuous = isdiscontinuous;
    m_bIsAvailableForUse = false;
    m_bIsIndexURLPresent = isIndexURLPresent;
    m_bIsIndexRangeExact = isIndexRangeExact;
    m_bIsInitURLPresent  = isInitURLPresent;
  }

  return bOk;
}

void SegmentInfo::UpdatePeriodKey(uint64 periodKey)
{
  m_key = m_key & (~MPD_PERIOD_MASK);
  m_key = m_key | (periodKey & MPD_PERIOD_MASK);
}

/**
 * Update the internal key (unique key for segments in the rep)
 * on an mpd update.
 */
void SegmentInfo::SetKey(uint64 key)
{
  m_key = key;
}

void SegmentInfo::SetByteRangeURLResp(const HTTPCommon::ByteRangeURLRespState val)
{
  m_eByteRangeURLRespState = val;
}

HTTPCommon::ByteRangeURLRespState SegmentInfo::GetByteRangeURLResp()
{
  return m_eByteRangeURLRespState;
}

/*
 * SegmentTemplateType
 */

/****************************************************************************
 Segment Template Class
*****************************************************************************
*/
/* @brief - SegmentTemplateType constructor
*/


SegmentTemplateType::SegmentTemplateType()
:m_pMedia(NULL),
m_pIndex(NULL),
m_pInitialisation(NULL),
m_pBitStreamSwitching(NULL),
m_bIsSegmentTimelineFound(false)
{
}

/* @brief - SegmentTemplateType destructor
*/

SegmentTemplateType::~SegmentTemplateType()
{
  if(m_pMedia)
  {
    QTV_Free(m_pMedia);
    m_pMedia = NULL;
  }
  if(m_pIndex)
  {
    QTV_Free(m_pIndex);
    m_pIndex = NULL;
  }
  if(m_pInitialisation)
  {
    QTV_Free(m_pInitialisation);
    m_pInitialisation = NULL;
  }
  if(m_pBitStreamSwitching)
  {
    QTV_Free(m_pBitStreamSwitching);
    m_pBitStreamSwitching = NULL;
  }
}

void SegmentTemplateType::SetMediaTemplate(char *media)
{
  if (m_pMedia)
  {
    QTV_Free(m_pMedia);
    m_pMedia = NULL;
  }
  if (media)
  {
    int medialen = (int)std_strlen(media)+1;
    m_pMedia= (char *)QTV_Malloc(medialen);
    if (m_pMedia)
    {
      std_strlcpy(m_pMedia, media, medialen);
    }
  }
}
void SegmentTemplateType::SetIndexTemplate(char *index)
{
  if (m_pIndex)
  {
    QTV_Free(m_pIndex);
    m_pIndex = NULL;
  }
  if (index)
  {
    int indexlen = (int)std_strlen(index)+1;
    m_pIndex= (char *)QTV_Malloc(indexlen);
    if (m_pIndex)
    {
      std_strlcpy(m_pIndex, index, indexlen);
    }
  }
}
void SegmentTemplateType::SetInitialisationTemplate(char *initialisation)
{
  if (m_pInitialisation)
  {
    QTV_Free(m_pInitialisation);
    m_pInitialisation = NULL;
  }
  if (initialisation)
  {
    int initialisationlen = (int)std_strlen(initialisation)+1;
    m_pInitialisation= (char *)QTV_Malloc(initialisationlen);
    if (m_pInitialisation)
    {
      std_strlcpy(m_pInitialisation, initialisation, initialisationlen);
    }
  }
}
/* @brief - set BitStream Switching
*/

void SegmentTemplateType::SetBSSwitchingTemplate(char *bsSwitching)
{
  if (m_pBitStreamSwitching)
  {
    QTV_Free(m_pBitStreamSwitching);
    m_pBitStreamSwitching = NULL;
  }
  if (bsSwitching)
  {
    int bsSwitchinglen = (int)std_strlen(bsSwitching)+1;
    m_pBitStreamSwitching= (char *)QTV_Malloc(bsSwitchinglen);
    if (m_pBitStreamSwitching)
    {
      std_strlcpy(m_pBitStreamSwitching, bsSwitching, bsSwitchinglen);
    }
  }
}
char* SegmentTemplateType::GetMediaTemplate()
{
  return m_pMedia;
}
char* SegmentTemplateType::GetIndexTemplate()
{
  return m_pIndex;
}
char* SegmentTemplateType::GetInitialisationTemplate()
{
  return m_pInitialisation;

}
char* SegmentTemplateType::GetBSSwitchingTemplate()
{
  return m_pBitStreamSwitching;
}

/* @brief - inherit segment template info
 * if current segment cotains default values then only inherit
 */
void SegmentTemplateType::InheritSegmentTemplateInfo(SegmentTemplateType *segment_template)
{
  if (segment_template)
  {
    if (!GetMediaTemplate())
    {
      SetMediaTemplate(segment_template->GetMediaTemplate());
    }
    if (!GetIndexTemplate())
    {
      SetIndexTemplate(segment_template->GetIndexTemplate());
    }
    if (!GetInitialisationTemplate())
    {
      SetInitialisationTemplate(segment_template->GetInitialisationTemplate());
    }
    if (!GetBSSwitchingTemplate())
    {
      SetBSSwitchingTemplate(segment_template->GetBSSwitchingTemplate());
    }
  }
}

/* @brief - inherit segment base info
 * if current segmenttemplate contains default values, then only inherit
*/
void SegmentTemplateType::InheritMultiSegmentBaseInfo(SegmentTemplateType *segment_template)
{
  int bOk = true;
  if (segment_template)
  {
    if (GetDuration() == 0)
    {
      SetDuration(segment_template->GetDuration());
    }
    if (GetStartNumber() == MAX_UINT32_VAL)
    {
      SetStartNumber(segment_template->GetStartNumber());
    }
    InheritSegmentBaseInfo(segment_template);
    //if current level doesn't contain segment time line then only inherit
    if (!IsSegmentTimelineFound())
    {
      bOk = InitializeSegmentTimeline(segment_template->GetNumSegmentTimelineEntry());
      if (bOk)
      {
        for (int i = 0; i <segment_template->GetNumSegmentTimelineEntry(); i++)
        {
           SetSegmentTimeline(i,
                             segment_template->GetSegTimeLineStartTime(i),
                             segment_template->GetSegTimeLineDuration(i),
                             segment_template->GetSegTimeLineRepeatCount(i));
        }
      }
    }
  }
}

void SegmentTemplateType::Copy(SegmentTemplateType* pRHS)
{
  if (pRHS)
  {
    InheritMultiSegmentBaseInfo(pRHS);
    InheritSegmentTemplateInfo(pRHS);
  }
}

/****************************************************************************
 MultipleSegment Base Class
*****************************************************************************
*/
/* @brief - MultipleSegmentBaseType Constructor
*/



MultipleSegmentBaseType::MultipleSegmentBaseType()
:m_nDuration(0),
m_nStartNumber(MAX_UINT32_VAL),
m_pSegmentTimeline(NULL),
m_nNumSegmentTimeline(0),
m_pBitstreamSwitching(NULL),
m_bSegmentTimelineFound(false)
{
}

/* @brief - MultipleSegmentBaseType destructor
*/

MultipleSegmentBaseType::~MultipleSegmentBaseType()
{
  if(m_pSegmentTimeline)
  {
    QTV_Delete_Array(m_pSegmentTimeline);
    m_pSegmentTimeline = NULL;
  }
  if (m_pBitstreamSwitching)
  {
    if (m_pBitstreamSwitching->sourceURL)
    {
      QTV_Free(m_pBitstreamSwitching->sourceURL);
      m_pBitstreamSwitching->sourceURL = NULL;
    }
    if (m_pBitstreamSwitching->range)
    {
      QTV_Free(m_pBitstreamSwitching->range);
      m_pBitstreamSwitching->range = NULL;
    }
    QTV_Free(m_pBitstreamSwitching);
    m_pBitstreamSwitching = NULL;
  }
}

void MultipleSegmentBaseType::SetSegmentTimeline(uint32 segmentTimelineIndex,
                                             uint64 startTime,
                                             uint64 duration,
                                             int32 repeatCount)
{
  if (m_pSegmentTimeline)
  {
    m_pSegmentTimeline[segmentTimelineIndex].SetStartTime(startTime);
    m_pSegmentTimeline[segmentTimelineIndex].SetDuration(duration);
    m_pSegmentTimeline[segmentTimelineIndex].SetRepeatCount(repeatCount);
  }
}

bool MultipleSegmentBaseType::InitializeSegmentTimeline(int numSEntry)
{
  bool bOk = false;
  m_nNumSegmentTimeline = numSEntry;
  if(m_pSegmentTimeline)
  {
    QTV_Delete_Array(m_pSegmentTimeline);
    m_pSegmentTimeline = NULL;
  }
  if (m_nNumSegmentTimeline > 0 )
    {
    m_pSegmentTimeline=(SegmentTimelineType*)QTV_New_Array(SegmentTimelineType,(m_nNumSegmentTimeline));
      if (m_pSegmentTimeline)
      {
        bOk = true;
        m_bSegmentTimelineFound = true;
      }
    }
    return bOk;
}
bool MultipleSegmentBaseType::IsSegmentTimelineFound()
{
  return m_bSegmentTimelineFound;
}

int MultipleSegmentBaseType::GetNumSegmentTimelineEntry()
{
  return m_nNumSegmentTimeline;
}


void MultipleSegmentBaseType::SetDuration(uint32 duration)
{
  m_nDuration = duration;
}
void MultipleSegmentBaseType::SetStartNumber(uint32 startnumber)
{
  m_nStartNumber = startnumber;
}

uint32 MultipleSegmentBaseType::GetDuration()
{
  return m_nDuration;
}
uint32 MultipleSegmentBaseType::GetStartNumber()
{
  return m_nStartNumber;
}

uint64
MultipleSegmentBaseType::GetSegTimeLineStartTime(uint32 segmentTimelineIndex)
{
  uint64 startTime = 0;
  if (m_pSegmentTimeline)
  {
     startTime = m_pSegmentTimeline[segmentTimelineIndex].GetStartTime();
  }
  return startTime;
}
uint64
MultipleSegmentBaseType::GetSegTimeLineDuration(uint32 segmentTimelineIndex)
{
  uint64 duration = 0;
  if (m_pSegmentTimeline)
  {
    duration = m_pSegmentTimeline[segmentTimelineIndex].GetDuration();
  }
    return duration;
}
int32
MultipleSegmentBaseType::GetSegTimeLineRepeatCount(uint32 segmentTimelineIndex)
{
  int32 repeatCount = 0;
  if (m_pSegmentTimeline)
  {
    repeatCount = m_pSegmentTimeline[segmentTimelineIndex].GetRepeatCount();
  }
  return repeatCount;
}

/* @brief - inherit segment base info
 * Multiple segment base only contains default values then only inherit
*/
void MultipleSegmentBaseType::InheritSegmentBaseInfo(SegmentTemplateType *segment_template)
{
  if (segment_template)
  {
    if (GetTimeScale() == MAX_UINT32_VAL)
    {
      SetTimeScale(segment_template->GetTimeScale());
    }
    if (GetPresentationOffset() == 0)
    {
      SetPresentationOffset(segment_template->GetPresentationOffset());
    }
    if (!GetIndexRange())
    {
      SetIndexRange(segment_template->GetIndexRange());
    }
    if (!GetIndexRangeExact())
    {
      SetIndexRangeExact(segment_template->GetIndexRangeExact());
    }
    if (!GetInitialisation())
    {
      SetInitialisation(segment_template->GetInitialisation());
    }
    if (!GetRepresentationIndex())
    {
      SetRepresentationIndex(segment_template->GetRepresentationIndex());
    }
  }
}

void MultipleSegmentBaseType::InheritSegmentBaseInfo(SegmentListType *segment_list)
{
  if (segment_list)
  {
    if (GetTimeScale() == MAX_UINT32_VAL)
    {
      SetTimeScale(segment_list->GetTimeScale());
    }
    if (GetPresentationOffset() == 0)
    {
      SetPresentationOffset(segment_list->GetPresentationOffset());
    }
    if (!GetIndexRange())
    {
      SetIndexRange(segment_list->GetIndexRange());
    }
    if (!GetIndexRangeExact())
    {
      SetIndexRangeExact(segment_list->GetIndexRangeExact());
    }
    if (!GetInitialisation())
    {
      SetInitialisation(segment_list->GetInitialisation());
    }
    if (!GetRepresentationIndex())
    {
      SetRepresentationIndex(segment_list->GetRepresentationIndex());
    }
  }
}



/****************************************************************************
 Segment Base Class
*****************************************************************************
*/
/* @brief - SegmentBaseType Constructor
*/

SegmentBaseType::SegmentBaseType()
:m_nTimeScale(MAX_UINT32_VAL),
m_nPresentationTimeOffset(0),
m_pIndexRange(NULL),
m_bIndexRangeExact(false),
m_pInitialisation(NULL),
m_pRepresentationIndex(NULL)
{
}

/* @brief - SegmentBaseType Destructor
*/


SegmentBaseType::~SegmentBaseType()
{
  if(m_pIndexRange)
  {
    QTV_Free(m_pIndexRange);
    m_pIndexRange = NULL;
  }
  if (m_pInitialisation)
  {
    if (m_pInitialisation->range)
    {
      QTV_Free(m_pInitialisation->range);
      m_pInitialisation->range = NULL;
    }
    if (m_pInitialisation->sourceURL)
    {
      QTV_Free(m_pInitialisation->sourceURL);
      m_pInitialisation->sourceURL= NULL;
    }
    QTV_Free(m_pInitialisation);
    m_pInitialisation = NULL;
  }
  if (m_pRepresentationIndex)
  {
    if (m_pRepresentationIndex->range)
    {
      QTV_Free(m_pRepresentationIndex->range);
      m_pRepresentationIndex->range = NULL;
    }
    if (m_pRepresentationIndex->sourceURL)
    {
      QTV_Free(m_pRepresentationIndex->sourceURL);
      m_pRepresentationIndex->sourceURL = NULL;
    }
    QTV_Free(m_pRepresentationIndex);
    m_pRepresentationIndex = NULL;
  }
}

void SegmentBaseType::SetTimeScale(uint32 timescale)
{
  m_nTimeScale = timescale;
}

void SegmentBaseType::SetPresentationOffset(uint64 presentationoffset)
{
  m_nPresentationTimeOffset =  presentationoffset;
}

void SegmentBaseType::SetIndexRange(char *indexrange)
{
  if (m_pIndexRange)
  {
    QTV_Free(m_pIndexRange);
    m_pIndexRange = NULL;
  }
  if (indexrange)
  {
    int indexrangelen = (int)std_strlen(indexrange)+1;
    m_pIndexRange = (char *)QTV_Malloc(indexrangelen);
    if (m_pIndexRange)
    {
      std_strlcpy(m_pIndexRange, indexrange, indexrangelen);
    }
  }
}

void SegmentBaseType::SetIndexRangeExact(bool indexrangeexact)
{
  m_bIndexRangeExact = indexrangeexact;
}

void SegmentBaseType::SetInitialisation(URLType *initialisation)
{
  if (m_pInitialisation)
  {
    if (m_pInitialisation->range)
    {
      QTV_Free(m_pInitialisation->range);
      m_pInitialisation->range = NULL;
    }
    if (m_pInitialisation->sourceURL)
    {
      QTV_Free(m_pInitialisation->sourceURL);
      m_pInitialisation->sourceURL= NULL;
    }
    QTV_Free(m_pInitialisation);
    m_pInitialisation = NULL;
  }
  if (initialisation)
  {
    m_pInitialisation = (URLType *)QTV_Malloc(sizeof(URLType));
    if (m_pInitialisation)
    {
      m_pInitialisation->sourceURL = NULL;
      m_pInitialisation->range = NULL;
      if (initialisation->sourceURL)
      {
        int len = (int)std_strlen(initialisation->sourceURL) + 1;
        m_pInitialisation->sourceURL = (char *)QTV_Malloc(len);
        if (m_pInitialisation->sourceURL)
        {
          std_strlcpy(m_pInitialisation->sourceURL, initialisation->sourceURL, len);
        }
      }
            if (initialisation->range)
      {
        int len = (int)std_strlen(initialisation->range) + 1;
        m_pInitialisation->range = (char *)QTV_Malloc(len);
        if (m_pInitialisation->range)
        {
          std_strlcpy(m_pInitialisation->range, initialisation->range, len);
        }
      }
    }
  }
  else
  {
    m_pInitialisation = NULL;
  }
}

void SegmentBaseType::SetRepresentationIndex(URLType *repindex)
{
  if (m_pRepresentationIndex)
  {
    if (m_pRepresentationIndex->range)
    {
      QTV_Free(m_pRepresentationIndex->range);
      m_pRepresentationIndex->range = NULL;
    }
    if (m_pRepresentationIndex->sourceURL)
    {
      QTV_Free(m_pRepresentationIndex->sourceURL);
      m_pRepresentationIndex->sourceURL = NULL;
    }
    QTV_Free(m_pRepresentationIndex);
    m_pRepresentationIndex = NULL;
  }

  if (repindex)
  {
    m_pRepresentationIndex = (URLType *)QTV_Malloc(sizeof(URLType));
    if (m_pRepresentationIndex)
    {
      m_pRepresentationIndex->sourceURL = NULL;
      m_pRepresentationIndex->range = NULL;
      if (repindex->sourceURL)
      {
        int len = (int)std_strlen(repindex->sourceURL) + 1;
        m_pRepresentationIndex->sourceURL = (char *)QTV_Malloc(len);
        if (m_pRepresentationIndex->sourceURL)
        {
          std_strlcpy(m_pRepresentationIndex->sourceURL, repindex->sourceURL, len);
        }
      }
      if (repindex->range)
      {
        int len = (int)std_strlen(repindex->range) + 1;
        m_pRepresentationIndex->range = (char *)QTV_Malloc(len);
        if (m_pRepresentationIndex->range)
        {
          std_strlcpy(m_pRepresentationIndex->range, repindex->range, len);
        }
      }
    }
  }
  else
  {
    m_pRepresentationIndex = NULL;
  }
}


uint32 SegmentBaseType::GetTimeScale()
{
  return m_nTimeScale;
}

uint64 SegmentBaseType::GetPresentationOffset()
{
  return m_nPresentationTimeOffset;
}

char* SegmentBaseType::GetIndexRange()
{
  return m_pIndexRange;
}

bool SegmentBaseType::GetIndexRangeExact()
{
  return m_bIndexRangeExact;
}

URLType* SegmentBaseType::GetInitialisation()
{
  return m_pInitialisation;
}

URLType* SegmentBaseType::GetRepresentationIndex()
{
  return m_pRepresentationIndex;
}

/* @brief - inherit segment base info
* segment base only contains default values then only inherit
*/
void SegmentBaseType::InheritSegmentBase(SegmentBaseType *segment_base)
{
  if(segment_base)
  {
    if (GetTimeScale() == MAX_UINT32_VAL)
    {
      SetTimeScale(segment_base->GetTimeScale());
    }
    if (GetPresentationOffset() == 0)
    {
      SetPresentationOffset(segment_base->GetPresentationOffset());
    }
    if (!GetIndexRange())
    {
      SetIndexRange(segment_base->GetIndexRange());
    }
    if (!GetIndexRangeExact())
    {
      SetIndexRangeExact(segment_base->GetIndexRangeExact());
    }
    if (!GetInitialisation())
    {
      SetInitialisation(segment_base->GetInitialisation());
    }
    if (!GetRepresentationIndex())
    {
      SetRepresentationIndex(segment_base->GetRepresentationIndex());
    }
  }
}

void SegmentBaseType::Copy(SegmentBaseType* pRHS)
{
  if (pRHS)
  {
    InheritSegmentBase(pRHS);
  }
}

/****************************************************************************
 Segment Timeline  Class
*****************************************************************************
*/
/* @brief - SegmentTimelineType Constructor
*/


SegmentTimelineType::SegmentTimelineType()
:m_nStartTime(0),
m_nDuration(0),
m_nRepeatCount(0)
{
}
/* @brief - SegmentTimelineType desstructor
*/

SegmentTimelineType::~SegmentTimelineType()
{
}

void SegmentTimelineType::SetStartTime(uint64 starttime)
{
  m_nStartTime = starttime;
}

void SegmentTimelineType::SetDuration(uint64  duration)
{
  m_nDuration = duration;
}

void SegmentTimelineType::SetRepeatCount(int32  repeatCount)
{
  m_nRepeatCount = repeatCount;
}


uint64 SegmentTimelineType::GetStartTime()
{
  return m_nStartTime;
}

uint64 SegmentTimelineType::GetDuration()
{
  return m_nDuration;
}

int32 SegmentTimelineType::GetRepeatCount()
{
  return m_nRepeatCount;
}

/****************************************************************************
 Segment List  Class
*****************************************************************************
*/
/* @brief - SegmentListType Constructor
*/

SegmentListType::SegmentListType()
:m_pSegmentUrl(NULL),
m_nNumSegmentUrl(0)
{
}

/* @brief - SegmentListType destructor
*/

SegmentListType::~SegmentListType()
{
  if (m_pSegmentUrl)
  {
    QTV_Delete_Array(m_pSegmentUrl);
    m_pSegmentUrl = NULL;
  }
}
/* @brief - inherit segment base info
 * if default values present, then only inherit
*/
void SegmentListType::InheritMultiSegmentBaseInfo(SegmentListType *segment_list)
{
  int bOk = true;
  if (segment_list)
  {
    if (GetDuration() == 0)
    {
      SetDuration(segment_list->GetDuration());
    }
    if (GetStartNumber() == MAX_UINT32_VAL)
    {
      SetStartNumber(segment_list->GetStartNumber());
    }
    InheritSegmentBaseInfo(segment_list);
    //Inherit segmentline only if not present in current level
    if (!IsSegmentTimelineFound())
    {
      bOk = InitializeSegmentTimeline(segment_list->GetNumSegmentTimelineEntry());
      if (bOk)
      {
        for (int i = 0; i <segment_list->GetNumSegmentTimelineEntry(); i++)
        {
           SetSegmentTimeline(i,
                             segment_list->GetSegTimeLineStartTime(i),
                             segment_list->GetSegTimeLineDuration(i),
                             segment_list->GetSegTimeLineRepeatCount(i));
        }
      }
    }
  }
}

/* @brief - initialiases segment url info
*/
bool SegmentListType::InitialiseSegmentUrl(int numSegmentUrl)
{
  bool bOk = false;
  if(m_pSegmentUrl)
  {
    QTV_Delete_Array(m_pSegmentUrl);
    m_pSegmentUrl = NULL;
  }
  m_nNumSegmentUrl = numSegmentUrl;
  if (m_nNumSegmentUrl > 0)
  {
    m_pSegmentUrl=QTV_New_Array(SegmentURLType,(numSegmentUrl));
    if (m_pSegmentUrl)
    {
      bOk = true;
    }
  }
  return bOk;

}

void SegmentListType::InheritSegmentUrl(SegmentListType* segment_list, int segUrlIndex)
{
  if(segment_list)
  {
    SegmentURLType *pSegmentUrl = segment_list->GetSegmentUrl(segUrlIndex);
    if(pSegmentUrl)
    {
      SetSegmentUrl( segUrlIndex, pSegmentUrl->GetMediaUrl(),
                     pSegmentUrl->GetMediaRange(),
                     pSegmentUrl->GetIndexUrl(),
                     pSegmentUrl->GetIndexRange());
    }
  }
}

void SegmentListType::SetSegmentUrl(uint32 urlIndex, char* mediaUrl,
                    char* mediaRange, char *indexUrl,
                    char *indexRange)
{
  if (m_pSegmentUrl)
  {
    m_pSegmentUrl[urlIndex].SetMediaUrl(mediaUrl);
    m_pSegmentUrl[urlIndex].SetMediaRange(mediaRange);
    m_pSegmentUrl[urlIndex].SetIndexUrl(indexUrl);
    m_pSegmentUrl[urlIndex].SetIndexRange(indexRange);

  }
}

SegmentURLType *SegmentListType::GetSegmentUrl(int urlIndex)
{
  SegmentURLType *pSegmentUrl = NULL;
  if(urlIndex < m_nNumSegmentUrl)
  {
    pSegmentUrl = &m_pSegmentUrl[urlIndex];
  }
  return pSegmentUrl;
}


void SegmentListType::Copy(SegmentListType* pRHS)
{
  if (pRHS)
  {
    InheritMultiSegmentBaseInfo(pRHS);
    SetNumSegmentUrl(pRHS->GetNumSegmentUrl());
  }
}

SegmentURLType::SegmentURLType()
:m_pMediaUrl(NULL),
m_pMediaRange(NULL),
m_pIndexUrl(NULL),
m_pIndexRange(NULL)
{
}

/* @brief - SegmentListType destructor
*/

SegmentURLType::~SegmentURLType()
{
  if (m_pMediaUrl)
  {
    QTV_Free(m_pMediaUrl);
    m_pMediaUrl = NULL;
  }
  if (m_pMediaRange)
  {
    QTV_Free(m_pMediaRange);
    m_pMediaRange = NULL;
  }
  if (m_pIndexUrl)
  {
    QTV_Free(m_pIndexUrl);
    m_pIndexUrl = NULL;
  }
  if (m_pIndexRange)
  {
    QTV_Free(m_pIndexRange);
    m_pIndexRange = NULL;
  }
}
void SegmentURLType::SetMediaUrl(char *mediaUrl)
{
  if (m_pMediaUrl)
  {
    QTV_Free(m_pMediaUrl);
    m_pMediaUrl = NULL;
  }
  if (mediaUrl)
  {
    int len = (int)std_strlen(mediaUrl) + 1;
    m_pMediaUrl = (char *)QTV_Malloc(len);
    if (m_pMediaUrl)
    {
      std_strlcpy(m_pMediaUrl, mediaUrl, len);
    }
  }
}
void SegmentURLType::SetMediaRange(char *mediaRange)
{
  if (m_pMediaRange)
  {
    QTV_Free(m_pMediaRange);
    m_pMediaRange = NULL;
  }
  if (mediaRange)
  {
    int len = (int)std_strlen(mediaRange) + 1;
    m_pMediaRange = (char *)QTV_Malloc(len);
    if (m_pMediaRange)
    {
      std_strlcpy(m_pMediaRange, mediaRange, len);
    }
  }
}
void SegmentURLType::SetIndexUrl(char *indexUrl)
{
  if (m_pIndexUrl)
  {
    QTV_Free(m_pIndexUrl);
    m_pIndexUrl = NULL;
  }
  if (indexUrl)
  {
    int len = (int)std_strlen(indexUrl) + 1;
    m_pIndexUrl = (char *)QTV_Malloc(len);
    if (m_pIndexUrl)
    {
      std_strlcpy(m_pIndexUrl, indexUrl, len);
    }
  }
}
void SegmentURLType::SetIndexRange(char *indexRange)
{
  if (m_pIndexRange)
  {
    QTV_Free(m_pIndexRange);
    m_pIndexRange = NULL;
  }
  if (indexRange)
  {
    int len = (int)std_strlen(indexRange) + 1;
    m_pIndexRange = (char *)QTV_Malloc(len);
    if (m_pIndexRange)
    {
      std_strlcpy(m_pIndexRange, indexRange, len);
    }
  }
}
char* SegmentURLType::GetMediaUrl()
{
  return m_pMediaUrl;
}
char* SegmentURLType::GetMediaRange()
{
  return m_pMediaRange;
}
char* SegmentURLType::GetIndexUrl()
{
  return m_pIndexUrl;
}
char* SegmentURLType::GetIndexRange()
{
  return m_pIndexRange;
}

ContentDescriptorType::ContentDescriptorType() :
  m_DescName(NULL), m_SchemeURI(NULL), m_SchemeInfo(NULL) , m_bIsInherited(false)
{

}

ContentDescriptorType::~ContentDescriptorType()
{
  if (m_DescName)
  {
    QTV_Free(m_DescName);
    m_DescName = NULL;
  }

  if (m_SchemeURI)
  {
    QTV_Free(m_SchemeURI);
    m_SchemeURI = NULL;
  }

  if (m_SchemeInfo)
  {
    QTV_Free(m_SchemeInfo);
    m_SchemeInfo = NULL;
  }
}

bool ContentDescriptorType::SetDesc(const char *descName, const char *schemeURI, const char* value)
{
  bool bOk = true;

  if (descName && schemeURI && value)
  {
    if (m_DescName)
    {
      QTV_Free(m_DescName);
    }

    if (m_SchemeURI)
    {
      QTV_Free(m_SchemeURI);
    }

    if (m_SchemeInfo)
    {
      QTV_Free(m_SchemeInfo);
    }

    int descNameSize = 1 + (int)std_strlen(descName);
    m_DescName = (char *)QTV_Malloc(descNameSize * sizeof(char));

    if (m_DescName)
    {
      std_strlcpy(m_DescName, descName, descNameSize);
    }
    else
    {
      bOk = false;
    }

    int schemeUriSize = 1 + (int)std_strlen(schemeURI);
    m_SchemeURI = (char *)QTV_Malloc(schemeUriSize * sizeof(char));

    if (m_SchemeURI)
    {
      std_strlcpy(m_SchemeURI, schemeURI, schemeUriSize);
    }
    else
    {
      bOk = false;
    }

    int schemeInfoSize = 1 + (int)std_strlen(value);
    m_SchemeInfo = (char *)QTV_Malloc(schemeInfoSize);
    if (m_SchemeInfo)
    {
      std_strlcpy(m_SchemeInfo, value, schemeInfoSize);
    }
    else
    {
      bOk = false;
    }

    if (!bOk)
    {
      if (m_DescName)
      {
        QTV_Free(m_DescName);
        m_DescName = NULL;
      }

      if (m_SchemeURI)
      {
        QTV_Free(m_SchemeURI);
        m_SchemeURI = NULL;
      }

      if (m_SchemeInfo)
      {
        QTV_Free(m_SchemeInfo);
        m_SchemeInfo = NULL;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Invalid role descriptor");
  }

  return bOk;
}

void ContentDescriptorType::GetDesc(const char **ppDescName,
                                    const char **ppSchemeURI,
                                    const char **ppValue)
{
  *ppDescName = m_DescName;
  *ppSchemeURI = m_SchemeURI;
  *ppValue = m_SchemeInfo;
}

void ContentDescriptorType::SetInherited(bool bIsInherited)
{
  m_bIsInherited = bIsInherited;
}

bool ContentDescriptorType::IsInherited() const
{
  return m_bIsInherited;
}

bool ContentDescriptorType::Copy(const ContentDescriptorType& rCdt)
{
  bool bOk = true;

  if (rCdt.m_DescName)
  {
    if (m_DescName)
    {
      QTV_Free(m_DescName);
      m_DescName = NULL;
    }

    int descNameSize = 1 + (int)std_strlen(rCdt.m_DescName);
    m_DescName = (char *)QTV_Malloc(descNameSize * sizeof(char));

    if(m_DescName)
    {
      std_strlcpy(m_DescName, rCdt.m_DescName, descNameSize);
    }
    else
    {
      bOk = false;
    }
  }

  if (rCdt.m_SchemeURI)
  {
    if (m_SchemeURI)
    {
      QTV_Free(m_SchemeURI);
      m_SchemeURI = NULL;
    }

    int schemeUriSize = 1 + (int)std_strlen(rCdt.m_SchemeURI);
    m_SchemeURI = (char *)QTV_Malloc(schemeUriSize * sizeof(char));

    if (m_SchemeURI)
    {
      std_strlcpy(m_SchemeURI, rCdt.m_SchemeURI, schemeUriSize);
    }
    else
    {
      bOk = false;
    }
  }

  if (rCdt.m_SchemeInfo)
  {
    int schemeInfoSize = 1 + (int)std_strlen(rCdt.m_SchemeInfo);
    m_SchemeInfo = (char *)QTV_Malloc(schemeInfoSize);
    if (m_SchemeInfo)
    {
      std_strlcpy(m_SchemeInfo, rCdt.m_SchemeInfo, schemeInfoSize);
    }
    else
    {
      bOk = false;
    }
  }

  m_bIsInherited = rCdt.m_bIsInherited;

  return bOk;
}

void ContentDescriptorType::Print()
{
  //QTV_MSG_SPRINTF_PRIO_3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
  //                       "ContentDescriptorType %s %s %s",
  //                       m_DescName, m_SchemeURI, m_SchemeInfo);
}

ContentProtectionType::ContentProtectionType() :
  ContentDescriptorType(),
  m_ContentProtectionSource(CONTENT_PROTECTION_NONE),
  m_PRContentProtectoin(NULL),
  m_MrlnContentProtectoin(NULL),
  m_CencContentProtectoin(NULL)

{

}

ContentProtectionType::~ContentProtectionType()
{
  if (m_PRContentProtectoin)
  {
    QTV_Free(m_PRContentProtectoin);
    m_PRContentProtectoin = NULL;
  }

  if (m_MrlnContentProtectoin)
  {
    QTV_Free(m_MrlnContentProtectoin);
    m_MrlnContentProtectoin = NULL;
  }

  if (m_CencContentProtectoin)
  {
    QTV_Free(m_CencContentProtectoin);
    m_CencContentProtectoin = NULL;
  }
  }

void ContentProtectionType::SetPlayReadyContentProtection
(
  const char* prContentProtectoin
)
  {
  if (prContentProtectoin)
    {
    int regqSize = 1 + (int)std_strlen(prContentProtectoin);
    m_PRContentProtectoin = (char *)QTV_Malloc(regqSize);

    if (m_PRContentProtectoin)
    {
      std_strlcpy(m_PRContentProtectoin, prContentProtectoin, regqSize);
    }
  }
  }
const char *ContentProtectionType::GetPlayReadyContentProtection() const
    {
  return m_PRContentProtectoin;
    }


void ContentProtectionType::SetMarlinContentProtection
(
  const char* mrlnContentProtectoin
)
  {
  if (mrlnContentProtectoin)
    {
    int regqSize = 1 + (int)std_strlen(mrlnContentProtectoin);
    m_MrlnContentProtectoin = (char *)QTV_Malloc(regqSize);

    if (m_MrlnContentProtectoin)
    {
      std_strlcpy(m_MrlnContentProtectoin, mrlnContentProtectoin, regqSize);
  }
}

  }
const char *ContentProtectionType::GetMarlinContentProtection() const
{
  return m_MrlnContentProtectoin;
}


void ContentProtectionType::SetCENCContentProtection
(
  const char* cencContentProtectoin
)
{
  if (cencContentProtectoin)
  {
    int regqSize = 1 + (int)std_strlen(cencContentProtectoin);
    m_CencContentProtectoin = (char *)QTV_Malloc(regqSize);

    if (m_CencContentProtectoin)
    {
      std_strlcpy(m_CencContentProtectoin, cencContentProtectoin, regqSize);
  }
}

    }
const char *ContentProtectionType::GetCENCContentProtection() const
    {
  return m_CencContentProtectoin;
}



ContentProtectionType::ContentProtectionSource ContentProtectionType::GetContentProtectionSource() const
{
  return m_ContentProtectionSource;
}

void ContentProtectionType::Print()
{
  ContentDescriptorType::Print();
}

ContentDescriptorContainer::ContentDescriptorContainer() :
  m_Array(NULL), m_SizeUsed(0), m_ArraySize(0)
{

}

ContentDescriptorContainer::~ContentDescriptorContainer()
{
  if (m_Array)
  {
    QTV_Delete_Array(m_Array);
    m_Array = NULL;
  }

  m_SizeUsed = 0;
  m_ArraySize = 0;
}

bool ContentDescriptorContainer::AddDescriptorType(
  const char *descName, const char *schemeURI, const char *value)
{
  bool bOk = false;

  if(descName && schemeURI && value)
  {
    if(m_SizeUsed == m_ArraySize)
    {
      int newSize = m_ArraySize + 5;
      ContentDescriptorType *resizedDescArray = QTV_New_Array(ContentDescriptorType, newSize);

      if(resizedDescArray)
      {
        bOk = true;

        for(int i = 0; i < m_SizeUsed; ++i)
        {
          resizedDescArray[i].Copy(m_Array[i]);
        }

        if(m_Array)
        {
          QTV_Delete_Array(m_Array);
        }

        m_Array = resizedDescArray;
        m_ArraySize = newSize;
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Failed to resize roles array");
      }
    }

    if(m_SizeUsed < m_ArraySize)
    {
      m_Array[m_SizeUsed].SetDesc(descName, schemeURI, value);

      ++m_SizeUsed;
      bOk = true;
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "Incorrect desctype properties");
  }

  return bOk;
}

StringValue::StringValue() :
  m_Name(NULL), m_Value(NULL), m_bIsInherited(NULL)
{

}

StringValue::~StringValue()
{
  if(m_Name)
  {
    QTV_Free(m_Name);
    m_Name = NULL;
  }

  if(m_Value)
  {
    QTV_Free(m_Value);
    m_Value = NULL;
  }
}

bool StringValue::SetString(const char *name, const char *value)
{
  bool bOk = false;
  if(name && value)
  {
    if(m_Name)
    {
      QTV_Free(m_Name);
    }

    if(m_Value)
    {
      QTV_Free(m_Value);
    }

    int nameBufSize = 1 + (int)std_strlen(name);
    int valueBufSize = 1 + (int)std_strlen(value);

    m_Name = (char *)QTV_Malloc(nameBufSize * sizeof(char));

    if(m_Name)
    {
      m_Value = (char *)QTV_Malloc(valueBufSize * sizeof(char));

      if(m_Value)
      {
        std_strlcpy(m_Name, name, nameBufSize);
        std_strlcpy(m_Value, value, valueBufSize);
        bOk = true;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "StringValue::SetString error occcured");
  }

  return bOk;
}

void StringValue::SetInherited(bool bIsInherited)
{
  m_bIsInherited = bIsInherited;
}

bool StringValue::Copy(const StringValue& strVal)
{
  SetInherited(strVal.m_bIsInherited);
  return SetString(strVal.m_Name, strVal.m_Value);
}

void StringValue::GetStringValue(char **ppName, char **ppValue)
{
  *ppName = m_Name;
  *ppValue = m_Value;
}

StringValueContainer::StringValueContainer() :
  m_Array(NULL), m_SizeUsed(0), m_ArraySize(0)
{

}

StringValueContainer::~StringValueContainer()
{
  if(m_Array)
  {
    QTV_Delete_Array(m_Array);
    m_Array = NULL;
  }

  m_SizeUsed = 0;
  m_ArraySize = 0;
}

bool StringValueContainer::AddString(const char *pName, const char *pValue)
{
  bool bOk = false;

  if(pName && pValue)
  {
    if(m_SizeUsed == m_ArraySize)
    {
      int newSize = m_ArraySize + 5;
      StringValue *resizedArray = QTV_New_Array(StringValue, newSize);

      if(resizedArray)
      {
        bOk = true;

        for(int i = 0; i < m_SizeUsed; ++i)
        {
          bOk = resizedArray[i].Copy(m_Array[i]);

          if(false == bOk)
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "StringValueContainer::AddString failed");
            break;
          }
        }

        if(bOk)
        {
          if(m_Array)
          {
            QTV_Delete_Array(m_Array);
          }

          m_Array = resizedArray;
          m_ArraySize = newSize;
        }
        else
        {
          QTV_Delete_Array(resizedArray);
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Failed to resize roles array");
      }
    }

    if(m_SizeUsed < m_ArraySize)
    {
      bOk = m_Array[m_SizeUsed].SetString(pName, pValue);
      ++m_SizeUsed;
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "StringValueContainer::AddString failed");
  }

  return bOk;
}

bool StringValueContainer::CopyStringValues(const StringValueContainer& rCvc,
                                            bool bIsInherited)
{
  bool bOk = false;

  if(m_Array)
  {
    QTV_Delete_Array(m_Array);
  }

  if(rCvc.m_ArraySize > 0)
  {
    m_Array = QTV_New_Array(StringValue, rCvc.m_ArraySize);

    if(m_Array)
    {
      m_ArraySize = rCvc.m_ArraySize;
      m_SizeUsed = rCvc.m_SizeUsed;

      for(int i = 0; i < m_SizeUsed; ++i)
      {
        m_Array[i].Copy(rCvc.m_Array[i]);
        m_Array[i].SetInherited(bIsInherited);
      }

      bOk = true;
    }
  }
  else
  {
    bOk = true;
  }

  return bOk;
}

void StringValueContainer::GetStringValues(StringValue **ppStringValues, int& size)
{
  *ppStringValues = m_Array;
  size = m_SizeUsed;
}

const char *StringValueContainer::FindString(const char *pName)
{
  const char *pValue = NULL;

  if(pName)
  {
    for(int i = 0; i < m_SizeUsed; ++i)
    {
      char *tmpName = NULL, *tmpValue = NULL;
      m_Array[i].GetStringValue(&tmpName, &tmpValue);
      if(0 == std_stricmp(tmpName, pName))
      {
        pValue = tmpValue;
      }
    }
  }

  return pValue;
}

void StringValueContainer::PrintStringValues()
{
  for(int i = 0; i < m_SizeUsed; ++i)
  {
    char *pName = NULL, *pValue = NULL;
    m_Array[i].GetStringValue(&pName, &pValue);
    QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "String Name %s, value %s",
                           pName, pValue);
  }
}

/**
 * Get a pointer to the array of ContentDescriptorType.
 */
void ContentDescriptorContainer::GetContentDescriptors(ContentDescriptorType** ppContentDescs, int& numDescs)
{
  numDescs = m_SizeUsed;
  *ppContentDescs = m_Array;
}

bool ContentDescriptorContainer::CopyDescs(
  const ContentDescriptorContainer& cdc,
  bool bIsInherited)
{
  bool bOk = false;

  if(m_Array)
  {
    QTV_Delete_Array(m_Array);
    m_Array = NULL;
  }

  if(cdc.m_ArraySize > 0)
  {
    m_Array = QTV_New_Array(ContentDescriptorType, cdc.m_ArraySize);

    if(m_Array)
    {
      m_ArraySize = cdc.m_ArraySize;
      m_SizeUsed = cdc.m_SizeUsed;

      for(int i = 0; i < m_SizeUsed; ++i)
      {
        m_Array[i].Copy(cdc.m_Array[i]);
        if(bIsInherited)
        {
          // if this flag is set, then the entire container is marked inherited
          m_Array[i].SetInherited(bIsInherited);
        }
      }

      bOk = true;
    }
  }
  else
  {
    bOk = true;
  }

  return bOk;
}

/**
 * For debugging
 */
void ContentDescriptorContainer::PrintDescs()
{
  for(int i = 0; i < m_SizeUsed; ++i)
  {
    const char *pName, *pURI, *pValue;
    m_Array[i].GetDesc(&pName, &pURI, &pValue);
    //QTV_MSG_SPRINTF_PRIO_3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    //  "ContentDescriptorContainer::PrintDescs name %s, %s, '%s'",
    //  (pName ? pName : ""), (pURI ? pURI : ""), (pValue ? pValue : ""));
  }
}

}
