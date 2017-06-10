/* =======================================================================
                      DefaultTrackSelectionPolicy.h
DESCRIPTION
  This module contains the declaration of the DefaultTrackSelectionPolicy
  class.

EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/DefaultTrackSelectionPolicy.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

#ifndef DEFAULT_TRACK_SELECTION_POLICY_H
#define DEFAULT_TRACK_SELECTION_POLICY_H

#include "ITrackList.h"
#include "ITrackSelectionPolicy.h"
#include "qtv_msg.h"
#include "SourceMemDebug.h"

enum streaming_codec_select_criteria
{
  DEFAULT_CRITERIA = -1,
  BITRATE_BASED_STREAMING_CODEC_CRITERIA = 0,
  PERFORMANCE_BASED_STREAMING_CODEC_CRITERIA = 1
};

class DefaultTrackSelectionPolicy : public ITrackSelectionPolicy
{
  IMPLEMENT_IREFERENCECOUNTABLE_MT(DefaultTrackSelectionPolicy)

public:

  DefaultTrackSelectionPolicy();
  DefaultTrackSelectionPolicy(streaming_codec_select_criteria selectionCriteria);
  virtual ~DefaultTrackSelectionPolicy();

  virtual void SelectTracks(ITrackList *trackList /* in */, bool reselect);

  virtual bool Notify(int eventType, void *value);

  void SetCodecSelectCriteria(streaming_codec_select_criteria selectionCriteria);

  void BlockH264(bool doBlock);

protected:

  /**
   * @brief
   *  Select video tracks.
   *  Preference: MPEG4 over H263.
   *
   * @param trackList
   */
  void SelectVideoTracks(ITrackList& trackList);

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
  void SelectAudioTracks(ITrackList& trackList);

  /**
   * @brief
   *  Select all timed text tracks.
   *
   * @param trackList
   */
  void SelectTimedTextTracks(ITrackList& trackList);


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
  int SelectBaseMpeg4Track(ITrackList& trackList);

  /**
   * @brief
   *  Selects the track with 'trackIndex' if it is dependent on
   *  the MPEG4 track with index 'baseMpeg4SelectedTrackIndex'.
   *
   * @param trackList
   * @param trackIndex
   * @param baseMpeg4SelectedTrackIndex
   */
  void SelectIfDependentMpeg4Track(ITrackList& trackList,
                                   int trackIndex,
                                   int baseMpeg4SelectedTrackIndex);

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
  int SelectNonMpeg4VideoTrack(ITrackList& trackList,
                               int nonMpeg4TrackIndex);

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
  int ChoosePreferredAudioTrack(ITrackList& trackList,
                                int aacIdx,
                                int someAudioCodecTrackIdx,
                                int evrcOrGsmIdx,
                                int highRateCodecIndex,
                                int rejectedAACTrackIndex);

  streaming_codec_select_criteria m_selectionCriteria;
  bool m_doBlockH264;
};

#endif /* DEFAULT_TRACK_SELECTION_POLICY_H */
