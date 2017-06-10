#ifndef _I_TRACKLIST_H_
#define _I_TRACKLIST_H_
/* =======================================================================
                              ITrackList.h
DESCRIPTION
  Track list interface.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/ITrackList.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <StreamMediaHelper.h>
#include "IReferenceCountable.h"

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

class ITrackGrouping : public IReferenceCountable
{
public:
  ITrackGrouping() { }

  virtual ~ITrackGrouping() { }

  virtual const char *GetValueAsString() const = 0;
  virtual int32 GetNumTracks() const = 0;
  virtual int32 GetTrackIndex(int32 index) const = 0;
  virtual float GetRequiredRTPRxBandwidth() const = 0;
  virtual float GetRequiredRTCPRxBandwidth() const = 0;
};

class ITrackGroupingList : public IReferenceCountable
{
public:
  ITrackGroupingList() { }

  virtual ~ITrackGroupingList() { }

  virtual const char *GetType() const = 0;
  virtual const char *GetSubType() const = 0;
  virtual int32 GetNumTrackGroupings() const = 0;
  virtual bool GetTrackGrouping(int32 index,
    ITrackGrouping **ppGrouping /* out */) const = 0;
};

class IAltTrackInfo : public IReferenceCountable
{
public:
  IAltTrackInfo() { }

  virtual ~IAltTrackInfo() { }

  virtual int32 GetBaseTrackIndex(int32 trackIndex) const = 0;

  virtual int32 GetNumGroupingLists() const = 0;
  virtual bool GetGroupingList(int32 index,
    ITrackGroupingList **ppGroupingList /* out */) const = 0;
};

class ITrackList : public IReferenceCountable
{
public:

  ITrackList() { }

  virtual ~ITrackList() { }

  virtual int GetNumTracksAvailable() const = 0;

  virtual bool Clone(ITrackList **pTrackList /* out */) const = 0;

  virtual uint32 GetTrackID(int32 trackIndex) const = 0;
  virtual StreamMediaHelper::CodecType GetCodecType(int32 trackIndex) const = 0;
  virtual int32 GetBitrateBPS(int32 trackIndex) const = 0;
  virtual int32 GetNumAudioChannels(int32 trackIndex) const = 0;
  virtual int32 GetDependsOnID(int32 trackIndex) const = 0;
  virtual int32 GetFrameWidth(int32 trackIndex) const = 0;
  virtual int32 GetFrameHeight(int32 trackIndex) const = 0;
  virtual int32 GetFrameRate(int32 trackIndex) const = 0;
  virtual bool IsPlayable(int32 trackIndex) const = 0;

  virtual bool IsTrackMuted(int32 trackIndex) const = 0;
  virtual bool IsTrackSelected(int32 trackIndex) const = 0;
  virtual bool SelectTrack(int32 trackIndex, bool select = true) = 0;
  virtual bool MuteTrack(int32 trackIndex, bool muted = false) = 0;

  virtual void ResetTrackSelection() = 0;

  virtual int32 GetNumTracksSelected() const = 0;

  virtual int32 GetNextSelectedAudioTrackIndex(
    int32 prevSelectedAudioTrackIndex = -1) const = 0;

  virtual int32 GetNextSelectedVideoTrackIndex(
    int32 prevSelectedVideoTrackIndex = -1) const = 0;

  virtual int32 GetNextSelectedTextTrackIndex(
    int32 prevSelectedTextTrackIndex = -1) const = 0;

  virtual bool GetAttribute(const char *name,
                            IReferenceCountable **pValue /* out */) = 0;
  virtual void PutAttribute(const char *name,
                            IReferenceCountable *value /* in */) = 0;

  virtual bool AreSameTracksSelected(const ITrackList &b) const = 0;

  virtual bool IsMultiBitRateWMClip() const = 0;

  static const char *AttributeAltTrackInfo;
};
#endif /* _I_TRACKLIST_H_ */
