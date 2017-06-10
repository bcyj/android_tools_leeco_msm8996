/* =======================================================================
                              TrackList.cpp
DESCRIPTION
  This file is the default implementation of ITrackList. It uses an array of
  StreamMedia::MediaInfo to store information about each track. It uses a map
  to store attributes.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/TrackList.cpp#9 $
$DateTime: 2012/03/21 02:10:49 $
$Change: 2287580 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "qtv_msg.h"
#include "TrackList.h"
#include "StreamMediaHelper.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
const char *ITrackList::AttributeAltTrackInfo = "AltTrackInfo";

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
FUNCTION
  StringHash

DESCRIPTION
  A hash function which just relies on the implementation of
  OSCL_STRING::hash().

DEPENDENCIES
  None

RETURN VALUE
  A hash code based on the string.

SIDE EFFECTS
  None.

========================================================================== */
static uint32 StringHash(const OSCL_STRING &str)
{
  return str.hash();
}


/* ======================================================================
FUNCTION
  TrackList::TrackList

DESCRIPTION
  Construct an empty track list. This is useful when indicating an error.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
TrackList::TrackList()
 : m_numTracks(0), m_trackInfoArray(NULL), m_attributeMap(StringHash)
{
  RESET_REFCOUNT;

  _InvalidateCachedData();
}

/* ======================================================================
FUNCTION
  TrackList::TrackList

DESCRIPTION
  Construct a track list based on the argument array and attribute map.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
TrackList::TrackList(StreamMediaHelper::TrackInfo *&trackInfoArray, int numTracks)
 : m_numTracks(numTracks), m_trackInfoArray(trackInfoArray),
   m_attributeMap(StringHash)
{
  RESET_REFCOUNT;

  _InvalidateCachedData();

  // Prevent caller from using array again.
  trackInfoArray = NULL;
}

/* ======================================================================
FUNCTION
  TrackList::TrackList

DESCRIPTION
  Copy constructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
TrackList::TrackList(const TrackList &b)
 : m_attributeMap(b.m_attributeMap)
{
  RESET_REFCOUNT;

  m_trackInfoArray = _CloneTrackInfoArray(b.m_trackInfoArray, b.m_numTracks);
  m_numTracks = b.m_numTracks;

  _InvalidateCachedData();

  m_cachedNumTracksSelected = b.m_cachedNumTracksSelected;
}

/* ======================================================================
FUNCTION
  TrackList::~TrackList

DESCRIPTION
  Destructor.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
TrackList::~TrackList()
{
  NONZERO_REFCOUNT_DESTRUCTOR_WARNING(TrackList);

  if(m_trackInfoArray)
  {
    QTV_Delete_Array(m_trackInfoArray);
    m_trackInfoArray = NULL;
  }

  m_numTracks = 0;
}

/* ======================================================================
FUNCTION
  TrackList::operator =

DESCRIPTION
  Assignment operator

DEPENDENCIES
  None

RETURN VALUE
  A reference to this.

SIDE EFFECTS
  None

========================================================================== */
TrackList &TrackList::operator=(const TrackList &b)
{
  if(NULL != m_trackInfoArray)
  {
    QTV_Delete_Array(m_trackInfoArray);
  }

  m_trackInfoArray = _CloneTrackInfoArray(b.m_trackInfoArray, b.m_numTracks);
  m_numTracks = b.m_numTracks;

  m_attributeMap = b.m_attributeMap;

  _InvalidateCachedData();

  m_cachedNumTracksSelected = b.m_cachedNumTracksSelected;

  return *this;
}

/* ======================================================================
FUNCTION
  TrackList::GetNumTracksAvailable

DESCRIPTION
  Return the number of tracks in this track list. This overrides a
  virtual method.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int TrackList::GetNumTracksAvailable() const
{
  return m_numTracks;
}

/* ======================================================================
FUNCTION
  TrackList::Clone

DESCRIPTION
  Create a clone of this track list.

DEPENDENCIES
  None

RETURN VALUE
  Return true iff successful.

SIDE EFFECTS
  Sets the out pointer.

========================================================================== */
bool TrackList::Clone(ITrackList **pTrackList /* out */) const
{
  if (pTrackList == NULL)
  {
    return false;
  }

  ReferenceCountedPointer<ITrackList> copy;
  copy = QTV_New_Args(TrackList, (*this));

  copy.SaveToOutPointer(pTrackList);

  return true;
}

/* ======================================================================
FUNCTION
  TrackList::GetTrackID

DESCRIPTION
  Return the track ID for the given track index.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
uint32 TrackList::GetTrackID(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return 0xFFFFFFFF;
  }

  return m_trackInfoArray[trackIndex].actualTrackID;
}


/* ======================================================================
FUNCTION
  TrackList::GetCodecType

DESCRIPTION
  Return the codec type for the given track index.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
StreamMediaHelper::CodecType TrackList::GetCodecType(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return StreamMediaHelper::UNKNOWN_CODEC;
  }

  return m_trackInfoArray[trackIndex].codec;
}

/* ======================================================================
FUNCTION
  TrackList::GetBitrateBPS

DESCRIPTION
  Return the bitrate in bps for the given track index.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetBitrateBPS(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return -1;
  }

  return m_trackInfoArray[trackIndex].bitrate;
}

/* ======================================================================
FUNCTION
  TrackList::GetNumAudioChannels

DESCRIPTION
  Return the number of audio channels for the given track index. If the
  corresponding track is not an audio track, return 0.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetNumAudioChannels(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return -1;
  }

  return m_trackInfoArray[trackIndex].numAudioChannels;
}

/* ======================================================================
FUNCTION
  TrackList::GetDependsOnID

DESCRIPTION
  Return the ID of the track that the given track index depends on. If
  the track is an independent track, return -1.

  CHECKME: is this an ID or an index?
  CHECKME: Do all implementations return -1 for independent tracks?
  GenericBcastMedia might return the same track index.


DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetDependsOnID(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return -1;
  }

  return m_trackInfoArray[trackIndex].dependsOnID;
}

/* ======================================================================
FUNCTION
  TrackList::GetFrameWidth

DESCRIPTION
  Return frame width of the given track index. If the track is not a video
  track, return -1.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetFrameWidth(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return -1;
  }

  return m_trackInfoArray[trackIndex].frameWidth;
}


/* ======================================================================
FUNCTION
  TrackList::GetFrameHeight

DESCRIPTION
  Return frame width of the given track index. If the track is not a video
  track, return -1.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetFrameHeight(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return -1;
  }

  return m_trackInfoArray[trackIndex].frameHeight;
}

/* ======================================================================
FUNCTION
  TrackList::GetFrameRate

DESCRIPTION
  Return frame rate of the given track index.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetFrameRate(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return -1;
  }

  return m_trackInfoArray[trackIndex].frameRate;
}

/* ======================================================================
FUNCTION
  TrackList::IsPlayable

DESCRIPTION
  Return true iff the track can be played by QTV.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
bool TrackList::IsPlayable(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);
    return false;
  }

  return m_trackInfoArray[trackIndex].isPlayable;
}

bool TrackList::SelectTrack(int32 trackIndex, bool select)
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);

    return false;
  }

  m_trackInfoArray[trackIndex].isTrackSelected = select;

  _InvalidateCachedData();

  return true;
}

bool TrackList::MuteTrack(int32 trackIndex, bool muted)
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);

    return false;
  }

  m_trackInfoArray[trackIndex].isTrackMuted = muted;

  return true;
}

/* ======================================================================
FUNCTION
  TrackList::IsTrackSelected

DESCRIPTION
  Return true iff the track is selected.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
bool TrackList::IsTrackSelected(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);

    return false;
  }

  return m_trackInfoArray[trackIndex].isTrackSelected;
}

/* ======================================================================
FUNCTION
  TrackList::IsTrackMuted

DESCRIPTION
  Return true if the track is muted.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
bool TrackList::IsTrackMuted(int32 trackIndex) const
{
  if ((trackIndex < 0) || (trackIndex >= m_numTracks))
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
      "TrackList: trackIndex %d is invalid, numTracks = %d",
      trackIndex, m_numTracks);

    return false;
  }

  return m_trackInfoArray[trackIndex].isTrackMuted;
}

/* ======================================================================
FUNCTION
  TrackList::ResetTrackSelection

DESCRIPTION
  Unselect all tracks.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
void TrackList::ResetTrackSelection()
{
  for (int32 trackIndex = 0; trackIndex < m_numTracks; trackIndex++)
  {
    m_trackInfoArray[trackIndex].isTrackSelected = false;
  }

  _InvalidateCachedData();
}

/* ======================================================================
FUNCTION
  TrackList::GetNumTracksSelected

DESCRIPTION
  Return the number of track selected.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetNumTracksSelected() const
{
  if (m_cachedNumTracksSelected < 0)
  {
    _ComputeCachedData();
  }

  return m_cachedNumTracksSelected;
}

/* A private interface that allows us to match tracks meeting a certain
 * (subclass defined) criteria.
 */
class ITrackInfoMatcher
{
public:
  virtual ~ITrackInfoMatcher() { }
  virtual bool Matches(const StreamMediaHelper::TrackInfo *trackInfo) = 0;
};

/* U
 */

/* ======================================================================
FUNCTION
  TrackList::GetNextMatchingTrackIndex

DESCRIPTION
  Use the matching criteria to find the next match after the argument index.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
static int32 GetNextMatchingTrackIndex(StreamMediaHelper::TrackInfo *pTrackList,
  int32 numTracks, int32 prevMatchingIndex, ITrackInfoMatcher *matcher)
{
  QTV_NULL_PTR_CHECK(pTrackList, -1);

  if (numTracks <= 0)
  {
    return -1;
  }

  int32 startIndex = 0;

  if (prevMatchingIndex < 0)
  {
    startIndex = 0;
  }
  else
  {
    startIndex = prevMatchingIndex + 1;
  }

  for (int32 trackIndex = startIndex; trackIndex < numTracks; trackIndex++)
  {
    if (matcher->Matches(pTrackList + trackIndex))
    {
      return trackIndex;
    }
  }

  return -1;
}

/* An implementation of ITrackInfoMatcher that matches selected audio tracks. */
class SelectedAudioTrackInfoMatcher : public ITrackInfoMatcher
{
public:
  SelectedAudioTrackInfoMatcher() { }
  virtual ~SelectedAudioTrackInfoMatcher() { }

  virtual bool Matches(const StreamMediaHelper::TrackInfo *trackInfo)
  {
    QTV_NULL_PTR_CHECK(trackInfo, false);

    return (trackInfo->isTrackSelected && StreamMediaHelper::IsAudioCodec(trackInfo->codec));
  }
};

/* ======================================================================
FUNCTION
  TrackList::GetNextSelectedAudioTrackIndex

DESCRIPTION
  Find the next selectedaudio track index after the previous index. For the
  first index, use -1.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetNextSelectedAudioTrackIndex(
  int32 prevSelectedAudioTrackIndex) const
{
  SelectedAudioTrackInfoMatcher matcher;

  return GetNextMatchingTrackIndex(m_trackInfoArray, m_numTracks,
    prevSelectedAudioTrackIndex, &matcher);
}

/* An implementation of ITrackInfoMatcher that matches selected video tracks. */
class SelectedVideoTrackInfoMatcher : public ITrackInfoMatcher
{
public:
  SelectedVideoTrackInfoMatcher() { }
  virtual ~SelectedVideoTrackInfoMatcher() { }

  virtual bool Matches(const StreamMediaHelper::TrackInfo *trackInfo)
  {
    QTV_NULL_PTR_CHECK(trackInfo, false);

    return (trackInfo->isTrackSelected && StreamMediaHelper::IsVideoCodec(trackInfo->codec));
  }
};

/* ======================================================================
FUNCTION
  TrackList::GetNextSelectedVideoTrackIndex

DESCRIPTION
  Find the next selected video track index after the previous index. For the
  first index, use -1.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetNextSelectedVideoTrackIndex(
  int32 prevSelectedVideoTrackIndex) const
{
  SelectedVideoTrackInfoMatcher matcher;

  return GetNextMatchingTrackIndex(m_trackInfoArray, m_numTracks,
    prevSelectedVideoTrackIndex, &matcher);
}

/* An implementation of ITrackInfoMatcher that matches selected text tracks. */
class SelectedTextTrackInfoMatcher : public ITrackInfoMatcher
{
public:
  SelectedTextTrackInfoMatcher() { }
  virtual ~SelectedTextTrackInfoMatcher() { }

  virtual bool Matches(const StreamMediaHelper::TrackInfo *trackInfo)
  {
    QTV_NULL_PTR_CHECK(trackInfo, false);

    return (trackInfo->isTrackSelected && StreamMediaHelper::IsTextCodec(trackInfo->codec));
  }
};

/* ======================================================================
FUNCTION
  TrackList::GetNextSelectedTextTrackIndex

DESCRIPTION
  Find the next selected text track index after the previous index. For the
  first index, use -1.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
int32 TrackList::GetNextSelectedTextTrackIndex(
  int32 prevSelectedTextTrackIndex) const
{
  SelectedTextTrackInfoMatcher matcher;

  return GetNextMatchingTrackIndex(m_trackInfoArray, m_numTracks,
    prevSelectedTextTrackIndex, &matcher);
}

/* ======================================================================
FUNCTION
  TrackList::GetAttribute

DESCRIPTION
  Retrieve an attribute from the map.

DEPENDENCIES
  None

RETURN VALUE
  Return true iff the attribute is found.

SIDE EFFECTS
  AddRefs the value and saves it in *pValue.

========================================================================== */
bool TrackList::GetAttribute(const char *name,
                             IReferenceCountable **pValue /* out */)
{
  QTV_NULL_PTR_CHECK(pValue, false);

  ReferenceCountedPointer<IReferenceCountable> ptr;

  if (m_attributeMap.get(OSCL_STRING(name), &ptr))
  {
    ptr.SaveToOutPointer(pValue);

    return true;
  }

  *pValue = NULL;
  return false;
}

/* ======================================================================
FUNCTION
  TrackList::PutAttribute

DESCRIPTION
  Put an attribute in the map.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  AddRefs the value.

========================================================================== */
void TrackList::PutAttribute(const char *name,
                             IReferenceCountable *value /* in */)
{
  ReferenceCountedPointer<IReferenceCountable> ptr(value);

  m_attributeMap.put(OSCL_STRING(name), ptr);
}

/* ======================================================================
FUNCTION
  TrackList::_ComputeCachedData

DESCRIPTION
  Compute and cache all parameters.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
void TrackList::_ComputeCachedData() const
{
  m_cachedNumTracksSelected = 0;

  for (int32 trackIndex = 0; trackIndex < m_numTracks; trackIndex++)
  {
    if (!m_trackInfoArray[trackIndex].isTrackSelected)
    {
      continue;
    }

    m_cachedNumTracksSelected++;
  }
}

/* ======================================================================
FUNCTION
  TrackList::_InvalidateCachedData

DESCRIPTION
  In response to a change, invalidate all computed parameters.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
void TrackList::_InvalidateCachedData() const
{
  m_cachedNumTracksSelected = -1;
}

/* ======================================================================
FUNCTION
  TrackList::_CloneTrackInfoArray

DESCRIPTION
  Allocate a copy of the given track info array.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
StreamMediaHelper::TrackInfo *TrackList::_CloneTrackInfoArray(StreamMediaHelper::TrackInfo *pTrackList,
                                                  int32 numTracks)
{
  StreamMediaHelper::TrackInfo *pRetArray = NULL;
  pRetArray = QTV_New_Array(StreamMediaHelper::TrackInfo, numTracks);

  if (pRetArray == NULL)
  {
    return NULL;
  }

  for (int32 trackIndex = 0; trackIndex < numTracks; trackIndex++)
  {
    pRetArray[trackIndex] = pTrackList[trackIndex];
  }

  return pRetArray;
}


/* ======================================================================
FUNCTION
  TrackList::AreSameTracksSelected

DESCRIPTION
  Return true iff the argument has the same tracks selected. This does not
  check the codec types or anything else. If the argument has a different
  number of available tracks, return false.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
bool TrackList::AreSameTracksSelected(const ITrackList &b) const
{
  if (m_numTracks != b.GetNumTracksAvailable())
  {
    return false;
  }

  // Does not check that each track matches!
  for (int32 trackIndex = 0; trackIndex < m_numTracks; trackIndex++)
  {
    if (m_trackInfoArray[trackIndex].isTrackSelected !=
        b.IsTrackSelected(trackIndex))
    {
      return false;
    }
  }

  return true;
}

/* ======================================================================
FUNCTION
  TrackList::IsMultiBitRateWMClip

DESCRIPTION
  Return true iff the track list has more than one WM audio or WM video track.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
bool TrackList::IsMultiBitRateWMClip() const
{
  int numWMAudioTracks = 0;
  int numWMVideoTracks = 0;

  // This does check whether we can actually play the tracks ...
  // Perhaps we need a flag in the TrackInfo to indicate whether the track is
  // playable.
  for (int32 trackIndex = 0; trackIndex < m_numTracks; trackIndex++)
  {
    switch (m_trackInfoArray[trackIndex].codec)
    {
      case StreamMediaHelper::WMA_CODEC:
      case StreamMediaHelper::WMA_PRO_CODEC:
      case StreamMediaHelper::WMA_PRO_PLUS_CODEC:
      numWMAudioTracks++;
      break;

      case StreamMediaHelper::WMV1_CODEC:
      case StreamMediaHelper::WMV2_CODEC:
      case StreamMediaHelper::WMV3_CODEC:
      numWMVideoTracks++;
      break;

      case StreamMediaHelper::UNKNOWN_CODEC:
      case StreamMediaHelper::EVRC_CODEC:
      case StreamMediaHelper::QCELP_CODEC:
      case StreamMediaHelper::AAC_CODEC:
      case StreamMediaHelper::BSAC_CODEC:
      case StreamMediaHelper::GSM_AMR_CODEC:
      case StreamMediaHelper::MPEG4_CODEC:
      case StreamMediaHelper::H263_CODEC:
      case StreamMediaHelper::H264_CODEC:
      case StreamMediaHelper::STILL_IMAGE_CODEC:
      case StreamMediaHelper::TIMED_TEXT_CODEC:
      case StreamMediaHelper::JPEG_CODEC:
      case StreamMediaHelper::MP3_CODEC:
      case StreamMediaHelper::OSCAR_CODEC:
      case StreamMediaHelper::CONC_CODEC:
      case StreamMediaHelper::COOK_CODEC:
      case StreamMediaHelper::SIPR_CODEC:
      case StreamMediaHelper::RV30_CODEC:
      case StreamMediaHelper::RV40_CODEC:
      case StreamMediaHelper::AMR_WB_CODEC:
      case StreamMediaHelper::AMR_WB_PLUS_CODEC:
      case StreamMediaHelper::NONMP4_MP3_CODEC:
      case StreamMediaHelper::QCP_CODEC:
      case StreamMediaHelper::MIDI_CODEC:
      case StreamMediaHelper::NONMP4_AAC_CODEC:
      case StreamMediaHelper::NONMP4_AMR_CODEC:
      case StreamMediaHelper::MAX_CODEC:
      default:
          break;
    }
  }

  return ((numWMAudioTracks > 1) || (numWMVideoTracks > 1));
}
