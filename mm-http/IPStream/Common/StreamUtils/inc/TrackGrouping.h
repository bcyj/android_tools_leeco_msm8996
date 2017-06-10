#ifndef _TRACK_GROUPING_H_
#define _TRACK_GROUPING_H_
/* =======================================================================
                              TrackGrouping.h
DESCRIPTION
  This file declares the implementation class for the ITrackGrouping
  class.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/TrackGrouping.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "ITrackList.h"
#include "sdp_info.h"
#include "AltGroup.h"

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

class TrackGrouping : public ITrackGrouping
{
  IMPLEMENT_IREFERENCECOUNTABLE_MT(TrackGrouping)

public:

  TrackGrouping(const AltGrouping &altGrouping, const SDPInfo &sdpInfo);

  virtual ~TrackGrouping();

  virtual const char *GetValueAsString() const;
  virtual int32 GetNumTracks() const;
  virtual int32 GetTrackIndex(int32 index) const;
  virtual float GetRequiredRTPRxBandwidth() const;
  virtual float GetRequiredRTCPRxBandwidth() const;

  void ComputeRequiredBandwidth(const SDPInfo &sdpInfo,
    const OSCL_STRING &type, const OSCL_STRING &subType);

private:

  OSCL_STRING m_value;
  int32 *m_trackIndexArray;
  int32 m_trackIndexArrayLength;
  float m_requiredRTPRxBandwidth;
  float m_requiredRTCPRxBandwidth;
};

class TrackGroupingList : public ITrackGroupingList
{
  IMPLEMENT_IREFERENCECOUNTABLE_MT(TrackGroupingList)

public:
  TrackGroupingList(const AltGroup &altGroup, const SDPInfo &sdpInfo);
  virtual ~TrackGroupingList();

  virtual const char *GetType() const;
  virtual const char *GetSubType() const;
  virtual int32 GetNumTrackGroupings() const;
  virtual bool GetTrackGrouping(int32 index,
    ITrackGrouping **ppGrouping /* out */) const;

private:

  OSCL_STRING m_type;
  OSCL_STRING m_subType;
  TrackGrouping **m_groupingArray;
  int32 m_groupingArrayLength;
};

class AltTrackInfo : public IAltTrackInfo
{
  IMPLEMENT_IREFERENCECOUNTABLE_MT(AltTrackInfo)

public:

  AltTrackInfo(const DeepList<AltGroup> &altGroupList,
               const SDPInfo &sdpInfo);

  virtual ~AltTrackInfo();

  virtual int32 GetBaseTrackIndex(int32 trackIndex) const;

  virtual int32 GetNumGroupingLists() const;
  virtual bool GetGroupingList(int32 index,
    ITrackGroupingList **ppGroupingList /* out */) const;

private:

  int32 m_numTracks;
  int32 *m_baseTrackIndexArray;

  TrackGroupingList **m_groupingListArray;
  int32 m_groupingListArrayLength;
};

bool AnnotateTrackListWithAltTrackInfo(ITrackList *trackList,
                                       const SDPInfo *sdpInfo);

#endif /* FEATURE_QTV_ALT_TRACKS */

#endif /* _TRACKLIST_GROUPING_H_ */
