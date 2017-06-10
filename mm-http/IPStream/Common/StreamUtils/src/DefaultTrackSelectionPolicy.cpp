/* =======================================================================
                       DefaultTrackSelectionPolicy.cpp
DESCRIPTION
  This module contains the definition of the
  DefaultTrackSelectionPolicy class. This is implements the default algorithm
  for selecting tracks, which depends on the codec select criteria.

EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/DefaultTrackSelectionPolicy.cpp#12 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "IReferenceCountable.h"
#include "DefaultTrackSelectionPolicy.h"
#include "StreamMediaHelper.h"
#include "qtv_msg.h"

DefaultTrackSelectionPolicy::DefaultTrackSelectionPolicy()
 : m_selectionCriteria(DEFAULT_CRITERIA), m_doBlockH264(false)
{
  RESET_REFCOUNT;
}

DefaultTrackSelectionPolicy::DefaultTrackSelectionPolicy(
  streaming_codec_select_criteria selectionCriteria)
  : m_selectionCriteria(selectionCriteria), m_doBlockH264(false)
{
  RESET_REFCOUNT;
}

DefaultTrackSelectionPolicy::~DefaultTrackSelectionPolicy()
{
  NONZERO_REFCOUNT_DESTRUCTOR_WARNING(DefaultTrackSelectionPolicy);
}

void DefaultTrackSelectionPolicy::SelectTracks(ITrackList *trackList /* in */,
  bool /*reselect*/)
{
  QTV_NULL_PTR_CHECK(trackList, RETURN_VOID);

  QTV_MSG1(QTVDIAG_STREAMING, "QTVCONFIG_CODECSELECT_CRITERIA = %d",
    m_selectionCriteria);

  SelectVideoTracks(*trackList);
  SelectAudioTracks(*trackList);
  SelectTimedTextTracks(*trackList);

  return;
}

/**
 * @brief
 *  Select video tracks.
 *  Preference: MPEG4 over H263.
 *
 * @param trackList
 */
void DefaultTrackSelectionPolicy::SelectVideoTracks(ITrackList& trackList)
{
  int numTracks = trackList.GetNumTracksAvailable();
  bool hasBaseMpeg4 = false;

  // Look for MPEG-4 base track.
  int selectedVideoTrack = SelectBaseMpeg4Track(trackList);

  if (selectedVideoTrack >= 0)
  {
    hasBaseMpeg4 = trackList.SelectTrack(selectedVideoTrack);

    if (false == hasBaseMpeg4)
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                    "DefaultTrackSelectionPolicy::SelectVideoTracks "
                    "Failed to select base Mpeg4 track '%d'",
                    selectedVideoTrack);

      selectedVideoTrack = -1;
    }
  }

  for (int i = 0; i < (int) numTracks; i++)
  {
    StreamMediaHelper::CodecType codecType = trackList.GetCodecType(i);

    if ((false == StreamMediaHelper::IsVideoCodec(codecType)) ||
        (false == trackList.IsPlayable(i)))
    {
      continue;
    }

    //If base Mpeg4 track has been selected, select all tracks that depend on it.
    if (true == hasBaseMpeg4)
    {
      // Select dependent Mpeg4 tracks.
      if (i != selectedVideoTrack)
      {
        SelectIfDependentMpeg4Track(trackList, i, selectedVideoTrack);

        // can select only one dependent mpeg4 track.
        break;
      }
    }
    else
    {
      if (selectedVideoTrack < 0)
      {
        selectedVideoTrack = SelectNonMpeg4VideoTrack(trackList, i);

        if (selectedVideoTrack >= 0)
        {
          QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
                        "Selected video track '%d'", selectedVideoTrack);

          break;
        }
      }
    } // if (hasBaseMpeg4)
  }
}

/**
 * @brief
 *  Select audio tracks.
 *  Preferrance order:
 *    1. AAC
 *    2. Other audio codec
 *    3. EVRC, GSM, QCELP
 *    4. high bitrate codec
 *
 * @param trackList
 */
void DefaultTrackSelectionPolicy::SelectAudioTracks(ITrackList& trackList)
{
  int aacIdx = -1;
  int someAudioCodecTrackIdx = -1;
  int evrcOrGsmIdx = -1;
  int highRateCodecIndex = -1;

  int rejectedAACTrackIndex = -1;

  int numTracks = trackList.GetNumTracksAvailable();

  for (int i = 0; i < (int) numTracks; i++)
  {
    StreamMediaHelper::CodecType codecType = trackList.GetCodecType(i);

    if (false == StreamMediaHelper::IsAudioCodec(codecType))
    {
      continue;
    }

    //Check whether player can play this track.
    if (!trackList.IsPlayable(i))
    {
      if (codecType == StreamMediaHelper::AAC_CODEC )
      {
        rejectedAACTrackIndex = i;
      }
      continue;
    }

    if ((m_selectionCriteria == PERFORMANCE_BASED_STREAMING_CODEC_CRITERIA) ||
        (m_selectionCriteria == DEFAULT_CRITERIA) )
    {
      switch (codecType)
      {
      case StreamMediaHelper::AAC_CODEC:
        if (aacIdx < 0)
        {
          aacIdx = i;
        }
        break;

      case StreamMediaHelper::GSM_AMR_CODEC:
      case StreamMediaHelper::EVRC_CODEC:
      case StreamMediaHelper::QCELP_CODEC:
        //Look for first GSM/EVRC track.
        if (evrcOrGsmIdx < 0)
        {
          //save this index but don't select the track yet,
          //since we still want to look for AAC.
          evrcOrGsmIdx = i;
        }
        break;

        // Disable audio if it's WMA on LTK, since the LTK doesn't
        // support it yet.

      case StreamMediaHelper::WMA_CODEC:
      case StreamMediaHelper::WMA_PRO_CODEC:
      case StreamMediaHelper::WMA_PRO_PLUS_CODEC:
        someAudioCodecTrackIdx = i;
        break;

      default:
        if (someAudioCodecTrackIdx < 0)
        {
          someAudioCodecTrackIdx = i;
        }

        break;
      } // end of switch (codecType)
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED, "Codec Selection based on BitRate");

      int32 trackBitRate = 0;

      if (highRateCodecIndex == -1)
      {
        trackBitRate = trackList.GetBitrateBPS(i);
        highRateCodecIndex = i;
      }
      else
      {
        if (trackBitRate < trackList.GetBitrateBPS(i))
        {
          highRateCodecIndex = i;
        }
      }
    }
  }

  ChoosePreferredAudioTrack(trackList,
                            aacIdx,
                            someAudioCodecTrackIdx,
                            evrcOrGsmIdx,
                            highRateCodecIndex,
                            rejectedAACTrackIndex);
  return;
}

/**
 * @brief
 *  Select all timed text tracks.
 *
 * @param trackList
 */
void
DefaultTrackSelectionPolicy::SelectTimedTextTracks(ITrackList& trackList)
{
  int numTracks = trackList.GetNumTracksAvailable();

  for (int i = 0; i < numTracks; i++)
  {
    StreamMediaHelper::CodecType codecType = trackList.GetCodecType(i);

    // Always select timed text track if present
    if (codecType == StreamMediaHelper::TIMED_TEXT_CODEC)
    {
      (void) trackList.SelectTrack(i);
    }
  }
}

/**
 * @brief
 *  Selects the first independent MPEG4 video track. Still image
 *  is also MPEG4.
 *
 * @param trackList
 *
 * @return int    The selected base MPEG4 track.
 *                -1 if base MPEG4 track not found.
 */
int DefaultTrackSelectionPolicy::SelectBaseMpeg4Track(ITrackList& trackList)
{
  int baseMpeg4SelectedTrackIndex = -1;

  int numTracks = trackList.GetNumTracksAvailable();

  for (int i = 0; i < (int) numTracks; i++)
  {
    if (!trackList.IsPlayable(i))
    {
      continue;
    }

    StreamMediaHelper::CodecType codecType = trackList.GetCodecType(i);

    //If an Mpeg4 base track. (Still image IS mpeg4!)
    if (((codecType == StreamMediaHelper::MPEG4_CODEC) && (trackList.GetDependsOnID(i) == -1)) ||
        (codecType == StreamMediaHelper::STILL_IMAGE_CODEC))
    {
      // Set base mpeg4 track flag.
      if (trackList.SelectTrack(i))
      {
        baseMpeg4SelectedTrackIndex = i;
        break;
      }
    }
  }

  return baseMpeg4SelectedTrackIndex;
}

/**
 * @brief
 *  Selects the track with 'trackIndex' if it is dependent on
 *  the MPEG4 track with index 'baseMpeg4SelectedTrackIndex'.
 *
 * @param trackList
 * @param trackIndex
 * @param baseMpeg4SelectedTrackIndex
 */
void DefaultTrackSelectionPolicy::SelectIfDependentMpeg4Track(
  ITrackList& trackList,
  int trackIndex,
  int baseMpeg4SelectedTrackIndex)
{
  StreamMediaHelper::CodecType codecType =
    trackList.GetCodecType(trackIndex);

  //If this is an enhancement track for the base track, select it.
  if ((codecType == StreamMediaHelper::MPEG4_CODEC) &&
      (trackList.GetDependsOnID(trackIndex) == baseMpeg4SelectedTrackIndex))
  {
    //Select enhancement track.
    (void) trackList.SelectTrack(trackIndex);
  }

  return;
}

/**
 * @brief
 *  Selects track if its a non-MPEG4 supported codec.
 *
 * @param trackList
 * @param nonMpeg4TrackIndex
 *
 * @return int  Selected non-mpeg4 codec track.
 *              -1 if no track was selected.
 */
int DefaultTrackSelectionPolicy::SelectNonMpeg4VideoTrack(ITrackList& trackList,
                                                          int nonMpeg4TrackIndex)
{
  int selectedNonMpeg4VideoTrack = -1;

  StreamMediaHelper::CodecType codecType =
    trackList.GetCodecType(nonMpeg4TrackIndex);

  if (codecType == StreamMediaHelper::H263_CODEC)
  {
    //Select track.
    if (trackList.SelectTrack(nonMpeg4TrackIndex))
    {
      selectedNonMpeg4VideoTrack = nonMpeg4TrackIndex;
    }
  }
  else if ((codecType == StreamMediaHelper::H264_CODEC) && !m_doBlockH264)
  {
    //Select track.
    if (trackList.SelectTrack(nonMpeg4TrackIndex))
    {
      selectedNonMpeg4VideoTrack = nonMpeg4TrackIndex;
    }
  }
  else if (codecType == StreamMediaHelper::JPEG_CODEC)
  {
    //Select track.
    if (trackList.SelectTrack(nonMpeg4TrackIndex))
    {
      selectedNonMpeg4VideoTrack = nonMpeg4TrackIndex;
    }
  }
  else if ((codecType == StreamMediaHelper::WMV1_CODEC) ||
           (codecType == StreamMediaHelper::WMV2_CODEC) ||
           (codecType == StreamMediaHelper::WMV3_CODEC))
  {
    //Select track.
    if (trackList.SelectTrack(nonMpeg4TrackIndex))
    {
      selectedNonMpeg4VideoTrack = nonMpeg4TrackIndex;
    }
  }

  return selectedNonMpeg4VideoTrack;
}

/**
 * @brief
 *  Selects an audio track from the one of the param
 *  trackIndices.
 *  In order of preference:
 *    AAC
 *    Any audio codec other than AAC, EVRC, GSM, QCELP
 *    EVRC or GSM or QCELP High
 *    Bit Rate codec.
 *    Rejected track index with AAC codec.
 *
 * @param trackList
 * @param aacIdx
 * @param someAudioCodecTrackIdx
 * @param evrcOrGsmIdx
 * @param highRateCodecIndex
 * @param rejectedAACTrackIndex
 *
 * @return int    The selected track index. -1 if no audio track
 *                was selected.
 */
int DefaultTrackSelectionPolicy::ChoosePreferredAudioTrack(
  ITrackList& trackList,
  int aacIdx,
  int someAudioCodecTrackIdx,
  int evrcOrGsmIdx,
  int highRateCodecIndex,
  int rejectedAACTrackIndex)
{
  int selectedAudioTrack = -1;

  if ((selectedAudioTrack < 0) && (aacIdx >= 0))
  {
    if (trackList.SelectTrack(aacIdx))
    {
      selectedAudioTrack = aacIdx;
    }
  }

  if ((selectedAudioTrack < 0) && (someAudioCodecTrackIdx >= 0))
  {
    if (trackList.SelectTrack(someAudioCodecTrackIdx))
    {
      selectedAudioTrack = someAudioCodecTrackIdx;
    }
  }

  if ((selectedAudioTrack < 0) && (evrcOrGsmIdx >= 0))
  {
    if (trackList.SelectTrack(evrcOrGsmIdx))
    {
      selectedAudioTrack = evrcOrGsmIdx;
    }
  }

  if ((selectedAudioTrack < 0) && (highRateCodecIndex >= 0))
  {
    if (trackList.SelectTrack(highRateCodecIndex))
    {
      selectedAudioTrack = highRateCodecIndex;
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED, "Bit Rate selected = %d",
        trackList.GetBitrateBPS(highRateCodecIndex));
    }
  }

  // This is a work around for a server problem. In case of AAC PV server sends us wrong
  // config information in the SDP which results in we rejecting the track. If no other
  // track information is present in the SDP we will go ahead and select that track because
  // the PC version of PV player plays that track
  if ((selectedAudioTrack < 0) && (rejectedAACTrackIndex >= 0))
  {
    if (trackList.SelectTrack(rejectedAACTrackIndex))
    {
      selectedAudioTrack = rejectedAACTrackIndex;
    }
  }

  return selectedAudioTrack;
}

void DefaultTrackSelectionPolicy::SetCodecSelectCriteria(
  streaming_codec_select_criteria selectionCriteria)
{
  m_selectionCriteria = selectionCriteria;
}

void DefaultTrackSelectionPolicy::BlockH264(bool doBlock)
{
  m_doBlockH264 = doBlock;
}

bool DefaultTrackSelectionPolicy::Notify(int /*eventType*/, void* /*value*/)
{
  return false;
}
