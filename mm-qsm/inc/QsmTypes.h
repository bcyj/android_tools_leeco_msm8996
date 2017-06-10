//=============================================================================
// QsmTypes.h
//
// COPYRIGHT 2011-2013 Qualcomm Technologies Incorporated.
// All rights reserved. Qualcomm Technologies proprietary and confidential.
//
// $Header: //depot/asic/msmshared/users/crd_dash/QSM/inc/QsmTypes.h#52 $
// $DateTime: 2014/03/13 18:08:10 $
// $Change: 5460255 $
//=============================================================================

#ifndef QSM_TYPES_H
#define QSM_TYPES_H

#include "AEEStdDef.h"
#include "MMMemory.h"

namespace QSM {
/**
 * Major type mask
 */
typedef enum {
  MAJOR_TYPE_UNKNOWN = 0x00000000,
  MAJOR_TYPE_AUDIO   = 0x00000001,
  MAJOR_TYPE_VIDEO   = 0x00000002,
  MAJOR_TYPE_TEXT    = 0x00000004,
  MAJOR_TYPE_MAX     = 0x00000008,
} MajorType;

/**
 * Minor type
 */
typedef enum {
   MINOR_TYPE_UNKNOWN,
   MINOR_TYPE_H264,
   MINOR_TYPE_HEVC,
   MINOR_TYPE_AAC,
   MINOR_TYPE_AC3,
   MINOR_TYPE_MAX
} MinorType;

/**
 * Constants
 */
enum {
   QUALITY_DEFAULT = 1,
   NO_REPRESENTATION_SELECTED = MAX_UINT64,
};

/**
 * Status codes
 */
typedef enum {
   QSM_STATUS_OK,
   QSM_STATUS_UNEXPECTED_ERROR,
   QSM_STATUS_UNSUPPORTED_OP,
   QSM_STATUS_INSUFFICIENT_SPACE
} QsmStatus;

/**
 * Selected representation status codes
 */
typedef enum {
   SELREP_STATUS_OK,
   SELREP_STATUS_AUDIO_ONLY,
   SELREP_STATUS_ERROR
} SelRepStatus;

/**
 * Resolution of video track
 */
class CResolution {
public:
   inline CResolution(uint32 width, uint32 height) :
      m_nWidth(width), m_nHeight(height) {
   }
   uint32 m_nWidth;
   uint32 m_nHeight;
};

/**
 * Downloadable unit
 */
class CDataUnitInfo {
public:
   uint64 m_nKey; // unique key to identify each data unit
   uint64 m_nStartTime; // the time when the data unit will be played
   uint64 m_nDuration; // duration of the data unit
   int32 m_nRandomAccessPoint; // indicates the offset of the first RAP from
                               // m_nStartTime. -1 if no RAP is present
   uint32 m_nSize; // size of the data unit in bytes
};

/**
 * Group information
 */
class CGroupInfo {
public:
   inline CGroupInfo(uint64 key = 0,
                     uint32 majorType = MAJOR_TYPE_UNKNOWN,
                     MinorType minorType = MINOR_TYPE_UNKNOWN,
                     bool dataUnitsAligned = false,
                     bool dataUnitsStartWithRap = false,
                     bool mandatory = true,
                     bool switchable = false,
                     double quality = QUALITY_DEFAULT,
                     bool RAPAvail = true,
                     uint64 switchReq = 0) :
   m_nKey(key), m_eMajorType(majorType), m_eMinorType(minorType),
   m_bDataUnitsAligned(dataUnitsAligned),
   m_bDataUnitsStartWithRap(dataUnitsStartWithRap),
   m_bMandatory(mandatory), m_bSwitchable(switchable), m_nQuality(quality),
   m_bRAPInfoAvailable(RAPAvail), m_nSwitchBuffReq(switchReq),
   m_nBinThresholds(NULL),
   m_nNumThresholds(0) {
   }

   ~CGroupInfo()
   {
      if (m_nBinThresholds)
      {
         MM_Delete_Array(m_nBinThresholds);
         m_nBinThresholds = NULL;
      }
   }

   CGroupInfo(const CGroupInfo& rhs) :
     m_nBinThresholds(NULL),
     m_nNumThresholds(0)
   {
      Copy(rhs);
   }

   CGroupInfo& operator=(const CGroupInfo& rhs)
   {
      if (m_nBinThresholds)
      {
         MM_Delete_Array(m_nBinThresholds);
         m_nBinThresholds = NULL;
      }

      Copy(rhs);
      return *this;
   }

   uint64 m_nKey; // unique key to identify each record
   uint32 m_eMajorType; // codec type, set if known
   MinorType m_eMinorType; // media type, set if known
   bool m_bDataUnitsAligned; // indicates if fragments are aligned among
                             // representations in this group
   bool m_bDataUnitsStartWithRap;
   bool m_bMandatory; // False if can playback without this group
   bool m_bSwitchable; // True if can switch representation during playback
   double m_nQuality; // to decide tradeoffs choosing
   bool m_bRAPInfoAvailable; // indicates if RAP info is available
   uint64 m_nSwitchBuffReq; // buffering requirement for overlapping
                            // representation for switching
                            //representations for different groups

   uint32 *m_nBinThresholds;
   uint32 m_nNumThresholds; // the Source component must guarantee that the number of bin
                            // thresholds is the same for all switchable groups and zero
                            // for all non-switchable groups.

private:
   void Copy(const CGroupInfo& rhs)
   {
      if (&rhs != this)
      {
         m_nKey                   = rhs.m_nKey;
         m_eMajorType             = rhs.m_eMajorType;
         m_eMinorType             = rhs.m_eMinorType;
         m_bDataUnitsAligned      = rhs.m_bDataUnitsAligned;
         m_bDataUnitsStartWithRap = rhs.m_bDataUnitsStartWithRap;
         m_bMandatory             = rhs.m_bMandatory;
         m_bSwitchable            = rhs.m_bSwitchable;
         m_nQuality               = rhs.m_nQuality;
         m_bRAPInfoAvailable      = rhs.m_bRAPInfoAvailable;
         m_nSwitchBuffReq         = rhs.m_nSwitchBuffReq;

         m_nNumThresholds = 0;

         if (rhs.m_nNumThresholds > 0)
         {
            m_nBinThresholds = MM_New_Array(uint32, rhs.m_nNumThresholds);

            if (m_nBinThresholds)
            {
               m_nNumThresholds = rhs.m_nNumThresholds;

               for (uint32 i = 0; i < m_nNumThresholds; ++i)
               {
                  m_nBinThresholds[i] = rhs.m_nBinThresholds[i];
               }
            }
         }
      }
   }
};

/**
 * Details for representation
 */
class CRepresentationInfo {
public:
   inline CRepresentationInfo(uint64 key, CResolution resolution,
                              uint32 bitrate, uint32 frameRate) :
      m_nMimeType(0), m_nKey(key), m_resolution(resolution),
      m_nBitrate(bitrate), m_nFrameRate(frameRate) {
   }

   inline CRepresentationInfo() :
      m_nMimeType(0), m_nKey(0), m_resolution(0, 0),
      m_nBitrate(0), m_nFrameRate(0) {
   }

   inline bool operator < (const CRepresentationInfo& rhs) const
   {
      return m_nBitrate < rhs.m_nBitrate;
   }

   uint32 m_nMimeType; // media type if known
   uint64 m_nKey; // unique key to identify each record
   CResolution m_resolution; // set if known
   uint32 m_nBitrate; // Kbit/s
   uint32 m_nFrameRate;
};

/**
 * Capabilities info for the source component
 */
class StreamSourceCapabilitiesInfo {
public:
   uint32 nNumConnections; // number of requests that can be outstanding
   bool   bMustStartWithAV;
   bool   bLive; // flag to indicate live versus on-demand mode
};

/**
 * Data unit download information
 */
struct CDataUnitDownloadInfo
{
    // Group key
    uint64 nGroupKey;
    // Key for the representation within the group
    uint64 nRepresentationKey;
    // Actual key of the data unit
    uint64 nKey;
    // Total number of bytes downloaded
    uint32 nDownloaded;
    // Size of the data unit in bytes
    uint32 nSize;
    // Start time of the data unit (in msec)
    uint64 nStartTime;
    // Duration of the data unit (in msec)
    uint64 nDuration;
    // Indicates if the data segment has failed
    bool bFailed;

    CDataUnitDownloadInfo()
    {
       bFailed = false;
    }
};

/**
 * Selected group and representation pair info
 */
struct GroupRepresentationPair {
   uint64 group;
   uint64 representation;
};

/*
 * QSM configuration parameters obtained
 * from source at startup
 */
struct QsmConfigParams {
   uint32 nStartupPreroll;
   uint32 nRebuffPreroll;
   uint32 nMediaAvailWindow;
   uint32 nMaxBufferDuration;
   uint32 nMaxResponseTime;
};

}

#endif //QSM_TYPES_H
