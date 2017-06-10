#ifndef _TRACKLIST_H_
#define _TRACKLIST_H_
/* =======================================================================
                              TrackList.h
DESCRIPTION
  This file declares the default implementation of ITrackList. It uses an array of
  StreamMedia::MediaInfo to store information about each track. It uses a map
  to store attributes
  .
COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/TrackList.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "oscl_string.h"
#include "IReferenceCountable.h"
#include "ITrackList.h"
#include "ReferenceCountedPointer.h"
#include "DeepMap.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* =======================================================================
**                        Class Declarations
** ======================================================================= */
class TrackList : public ITrackList
{
  IMPLEMENT_IREFERENCECOUNTABLE_MT(TrackList)

public:

  TrackList();

  // This just copies the pointer value, it does not copy the array!
  // The argument value is set to NULL to prevent the caller from using it
  // again.
  TrackList(StreamMediaHelper::TrackInfo *&trackInfoArray, int numTracks);

  TrackList(const TrackList &b);

  virtual ~TrackList();

  virtual TrackList &operator = (const TrackList &b);

  virtual int GetNumTracksAvailable() const;

  virtual bool Clone(ITrackList **pTrackList /* out */) const;

  virtual uint32 GetTrackID(int32 trackIndex) const;
  virtual StreamMediaHelper::CodecType GetCodecType(int32 trackIndex) const;
  virtual int32 GetBitrateBPS(int32 trackIndex) const;
  virtual int32 GetNumAudioChannels(int32 trackIndex) const;
  virtual int32 GetDependsOnID(int32 trackIndex) const;
  virtual int32 GetFrameWidth(int32 trackIndex) const;
  virtual int32 GetFrameHeight(int32 trackIndex) const;
  virtual int32 GetFrameRate(int32 trackIndex) const;
  virtual bool IsPlayable(int32 trackIndex) const;

  virtual bool IsTrackMuted(int32 trackIndex) const;
  virtual bool IsTrackSelected(int32 trackIndex) const;
  virtual bool SelectTrack(int32 trackIndex, bool select = true);
  virtual bool MuteTrack(int32 trackIndex, bool muted = false);
  virtual void ResetTrackSelection();

  virtual int32 GetNumTracksSelected() const;

  virtual int32 GetNextSelectedAudioTrackIndex(
    int32 prevSelectedAudioTrackIndex = -1) const;

  virtual int32 GetNextSelectedVideoTrackIndex(
    int32 prevSelectedVideoTrackIndex = -1) const;

  virtual int32 GetNextSelectedTextTrackIndex(
    int32 prevSelectedTextTrackIndex = -1) const;

  virtual bool GetAttribute(const char *name,
                            IReferenceCountable **pValue /* out */);
  virtual void PutAttribute(const char *name,
                            IReferenceCountable *value /* in */);

  virtual bool AreSameTracksSelected(const ITrackList &b) const;

  virtual bool IsMultiBitRateWMClip() const;

private:

  static StreamMediaHelper::TrackInfo *_CloneTrackInfoArray(
    StreamMediaHelper::TrackInfo *pTrackInfoArray, int32 numTracks);

  void _ComputeCachedData() const;
  void _InvalidateCachedData() const;

  int m_numTracks;
  StreamMediaHelper::TrackInfo *m_trackInfoArray;

  mutable int32 m_cachedNumTracksSelected;

  DeepMap< OSCL_STRING,
           ReferenceCountedPointer< IReferenceCountable > > m_attributeMap;

};
#endif /* _TRACKLIST_H_ */
