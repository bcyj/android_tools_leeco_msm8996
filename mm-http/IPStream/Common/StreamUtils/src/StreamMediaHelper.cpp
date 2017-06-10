/* =======================================================================
                              StreamMediaHelper.cpp
DESCRIPTION
  This module defines the helper class to StreamMedia.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */
/* =======================================================================

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "StreamMediaHelper.h"

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Variables
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */
/* ======================================================================
FUNCTION
  StreamMediaHelper::IsAudioCodec

DESCRIPTION
  Tell whether the argument codec is an audio codec.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None
========================================================================== */
bool StreamMediaHelper::IsAudioCodec(CodecType codec)
{
  switch (codec)
  {
    case AAC_CODEC:

    case EVRC_CODEC:
    case QCELP_CODEC:
    case GSM_AMR_CODEC:
    case AMR_WB_CODEC:
    case AMR_WB_PLUS_CODEC:
    case WMA_CODEC:
    case WMA_PRO_CODEC:
    case WMA_PRO_PLUS_CODEC:
      return true;

    default:
      return false;
  }
}

/* ======================================================================
FUNCTION
  StreamMediaHelper::IsVideoCodec

DESCRIPTION
  Tell whether the argument codec is a video codec.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None
========================================================================== */
bool StreamMediaHelper::IsVideoCodec(CodecType codec)
{
  switch (codec)
  {
    case MPEG4_CODEC:
    case H263_CODEC:
    case H264_CODEC:
    case STILL_IMAGE_CODEC:
    case JPEG_CODEC:
    case WMV1_CODEC:
    case WMV2_CODEC:
    case WMV3_CODEC:
      return true;

    default:
      return false;
  }
}

/* ======================================================================
FUNCTION
  StreamMediaHelper::IsTextCodec

DESCRIPTION
  Tell whether the argument codec is a text codec.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None
========================================================================== */
bool StreamMediaHelper::IsTextCodec(CodecType codec)
{
  switch (codec)
  {
    case TIMED_TEXT_CODEC:
      return true;

    default:
    return false;
  }
}

/* ======================================================================
FUNCTION
  StreamMediaHelper::IsWindowsMediaCodec

DESCRIPTION
  Determine if the codec is Windows Media audio or video.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None
========================================================================== */
bool StreamMediaHelper::IsWindowsMediaCodec(CodecType codec)
{
  switch (codec)
  {
    case WMA_CODEC:
    case WMA_PRO_CODEC:
    case WMA_PRO_PLUS_CODEC:
    case WMV1_CODEC:
    case WMV2_CODEC:
    case WMV3_CODEC:
    return true;

    default:
    return false;
  }
}

/* ======================================================================
FUNCTION
  StreamMediaHelper::GetTrackType

DESCRIPTION
  Tell whether the argument codec is a video or audio or text codec.

DEPENDENCIES
  None

RETURN VALUE
  TrackType

SIDE EFFECTS
  None
========================================================================== */
StreamMediaHelper::TrackType StreamMediaHelper::GetTrackType
(
 const CodecType codec
)
{
  StreamMediaHelper::TrackType trackType = UNKNOWN_TRACK;

  //Determine if the input codec is a video or audio or text track type
  if (IsVideoCodec(codec))
  {
    trackType = VIDEO_TRACK;
  }
  else if (IsAudioCodec(codec))
  {
    trackType = AUDIO_TRACK;
  }
  else if (IsTextCodec(codec))
  {
    trackType = TEXT_TRACK;
  }

  return trackType;
}

/* ======================================================================
FUNCTION
  StreamMediaHelper::GetFileTypeByComponents

DESCRIPTION
  Given a set of boolean flags indicating if each components is present
  return the corresponding MediaFileType.

DEPENDENCIES
  None

RETURN VALUE
  See description.

SIDE EFFECTS
  None

========================================================================== */
StreamMediaHelper::MediaFileType StreamMediaHelper::GetFileTypeByComponents
(
  bool hasAudio,
  bool hasVideo,
  bool hasStillImage,
  bool hasText
)
{
  MediaFileType type = CONTENT_UNKNOWN;

  if (hasAudio)
  {
    if (hasVideo)
    {
      if (hasText)
      {
        type = CONTENT_AUDIO_VIDEO_TEXT;
      }
      else
      {
        type = CONTENT_AUDIO_VIDEO;
      }
    }
    else if (hasStillImage)
    {
      if (hasText)
      {
        type = CONTENT_AUDIO_VIDEO_STILL_IMAGE_TEXT;
      }
      else
      {
        type = CONTENT_AUDIO_VIDEO_STILL_IMAGE;
      }
    }
    else
    {
      if (hasText)
      {
        type = CONTENT_AUDIO_TEXT;
      }
      else
      {
        type = CONTENT_AUDIO;
      }
    }
  }
  else
  {
    if (hasVideo)
    {
      if (hasText)
      {
        type = CONTENT_VIDEO_TEXT;
      }
      else
      {
        type = CONTENT_VIDEO;
      }
    }
    else if (hasStillImage)
    {
      if (hasText)
      {
        type = CONTENT_VIDEO_STILL_IMAGE_TEXT;
      }
      else
      {
        type = CONTENT_VIDEO_STILL_IMAGE;
      }
    }
  }

  return type;
}

/* ======================================================================
FUNCTION
  StreamMediaHelper::CanPlay

DESCRIPTION
  Tell whether player supports the argument track.

DEPENDENCIES
  None

RETURN VALUE
  True if supported, false otherwise.

SIDE EFFECTS
  None
========================================================================== */
bool StreamMediaHelper::CanPlay(const TrackInfo *pTrackInfo)
{
  bool bRet = false;

  if (pTrackInfo == NULL)
  {
     return false;
  }

  switch (pTrackInfo->codec)
  {
    case AAC_CODEC:

    if ((pTrackInfo->numAudioChannels < 1) ||
        (pTrackInfo->numAudioChannels > 2))
    {
       bRet = false;
    }
    else
    {
       bRet = true;
    }
    break;

    case WMA_CODEC:
    case WMA_PRO_CODEC:
    case WMA_PRO_PLUS_CODEC:
    // Windows Media Audio not supported on LTK yet.
    bRet = true;
    break;

    case EVRC_CODEC:
    case QCELP_CODEC:

    case GSM_AMR_CODEC:
    case MPEG4_CODEC:
    case H263_CODEC:
    case H264_CODEC:
    case STILL_IMAGE_CODEC:
    case TIMED_TEXT_CODEC:
    case JPEG_CODEC:
    case WMV1_CODEC:
    case WMV2_CODEC:
    case WMV3_CODEC:
    case AMR_WB_CODEC:
    case AMR_WB_PLUS_CODEC:
      bRet = true;
      break;

    case UNKNOWN_CODEC:
    default:
      bRet = false;
      break;
  }
  return bRet;
}
