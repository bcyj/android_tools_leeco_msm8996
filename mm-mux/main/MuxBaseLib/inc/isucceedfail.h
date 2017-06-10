#ifndef __ISucceedFail_H__
#define __ISucceedFail_H__
/* =======================================================================
                              isucceedfail.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2011,2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/isucceedfail.h#7 $
$DateTime: 2011/07/27 22:59:05 $
$Change: 1856237 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MAX_ERROR_CODES 102

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
  READ_FAILED = -1,
  //0
  EVERYTHING_FINE,
  DEFAULT_ERROR,
  READ_USER_DATA_ATOM_FAILED,
  READ_MEDIA_DATA_ATOM_FAILED,
  READ_MOVIE_ATOM_FAILED,
  READ_MOVIE_HEADER_ATOM_FAILED,
  READ_TRACK_ATOM_FAILED,
  READ_TRACK_HEADER_ATOM_FAILED,
  READ_TRACK_REFERENCE_ATOM_FAILED,
  READ_TRACK_REFERENCE_TYPE_ATOM_FAILED,
  //10
  READ_OBJECT_DESCRIPTOR_ATOM_FAILED,
  READ_INITIAL_OBJECT_DESCRIPTOR_FAILED,
  READ_OBJECT_DESCRIPTOR_FAILED,
  READ_MEDIA_ATOM_FAILED,
  READ_MEDIA_HEADER_ATOM_FAILED,
  READ_HANDLER_ATOM_FAILED,
  READ_MEDIA_INFORMATION_ATOM_FAILED,
  READ_MEDIA_INFORMATION_HEADER_ATOM_FAILED,
  READ_VIDEO_MEDIA_HEADER_ATOM_FAILED,
  READ_SOUND_MEDIA_HEADER_ATOM_FAILED,
  //20
  READ_HINT_MEDIA_HEADER_ATOM_FAILED,
  READ_MPEG4_MEDIA_HEADER_ATOM_FAILED,
  READ_DATA_INFORMATION_ATOM_FAILED,
  READ_DATA_REFERENCE_ATOM_FAILED,
  READ_DATA_ENTRY_URL_ATOM_FAILED,
  READ_DATA_ENTRY_URN_ATOM_FAILED,
  READ_SAMPLE_TABLE_ATOM_FAILED,
  READ_TIME_TO_SAMPLE_ATOM_FAILED,
  READ_SAMPLE_DESCRIPTION_ATOM_FAILED,
  READ_SAMPLE_SIZE_ATOM_FAILED,
  //30
  READ_SAMPLE_TO_CHUNK_ATOM_FAILED,
  READ_CHUNK_OFFSET_ATOM_FAILED,
  READ_SYNC_SAMPLE_ATOM_FAILED,
  READ_SAMPLE_ENTRY_FAILED,
  READ_AUDIO_SAMPLE_ENTRY_FAILED,
  READ_VISUAL_SAMPLE_ENTRY_FAILED,
  READ_HINT_SAMPLE_ENTRY_FAILED,
  READ_MPEG_SAMPLE_ENTRY_FAILED,
  READ_AUDIO_HINT_SAMPLE_FAILED,
  READ_VIDEO_HINT_SAMPLE_FAILED,
  //40
  READ_ESD_ATOM_FAILED,
  READ_ES_DESCRIPTOR_FAILED,
  READ_SL_CONFIG_DESCRIPTOR_FAILED,
  READ_DECODER_CONFIG_DESCRIPTOR_FAILED,
  READ_DECODER_SPECIFIC_INFO_FAILED,
  DUPLICATE_MOVIE_ATOMS,
  NO_MOVIE_ATOM_PRESENT,
  DUPLICATE_OBJECT_DESCRIPTORS,
  NO_OBJECT_DESCRIPTOR_ATOM_PRESENT,
  DUPLICATE_MOVIE_HEADERS,
  //50
  NO_MOVIE_HEADER_ATOM_PRESENT,
  DUPLICATE_TRACK_REFERENCE_ATOMS,
  DUPLICATE_TRACK_HEADER_ATOMS,
  NO_TRACK_HEADER_ATOM_PRESENT,
  DUPLICATE_MEDIA_ATOMS,
  NO_MEDIA_ATOM_PRESENT,
  READ_UNKNOWN_ATOM,
  NON_PV_CONTENT,
  FILE_NOT_STREAMABLE,
  INSUFFICIENT_BUFFER_SIZE,
  //60
  INVALID_SAMPLE_SIZE,
  INVALID_CHUNK_OFFSET,
  END_OF_TRACK,
  MEMORY_ALLOCATION_FAILED,
  READ_FILE_TYPE_ATOM_FAILED,
  ZERO_OR_NEGATIVE_ATOM_SIZE,
  NO_MEDIA_TRACKS_IN_FILE,
  NO_META_DATA_FOR_MEDIA_TRACKS,
  MEDIA_DATA_NOT_SELF_CONTAINED,
  READ_PVTI_SESSION_INFO_FAILED,
  //70
  READ_PVTI_MEDIA_INFO_FAILED,
  READ_CONTENT_VERSION_FAILED,
  READ_DOWNLOAD_ATOM_FAILED,
  READ_TRACK_INFO_ATOM_FAILED,
  READ_REQUIREMENTS_ATOM_FAILED,
  READ_WMF_SET_MEDIA_ATOM_FAILED,
  READ_WMF_SET_SESSION_ATOM_FAILED,
  READ_PV_USER_DATA_ATOM_FAILED,
  READ_VIDEO_INFORMATION_ATOM_FAILED,
  READ_RANDOM_ACCESS_ATOM_FAILED,
  //80
  READ_AMR_SAMPLE_ENTRY_FAILED,
  READ_H263_SAMPLE_ENTRY_FAILED,
  FILE_OPEN_FAILED,
  READ_UUID_ATOM_FAILED,
  FILE_VERSION_NOT_SUPPORTED,
  TRACK_VERSION_NOT_SUPPORTED,
  READ_COPYRIGHT_ATOM_FAILED,
  READ_FONT_TABLE_ATOM_FAILED,
  //90
  READ_FONT_RECORD_FAILED,
  FILE_PSEUDO_STREAMABLE,
  FILE_NOT_PSEUDO_STREAMABLE,
  READ_PV_ENTITY_TAG_ATOM_FAILED,
  READ_TEXT_SAMPLE_MODIFIERS_FAILED,
  READ_KDDI_DRM_ATOM_FAILED,
  READ_KDDI_CONTENT_PROPERTY_ATOM_FAILED,
  READ_KDDI_MOVIE_MAIL_ATOM_FAILED,
  READ_KDDI_ENCODER_INFO_ATOM_FAILED,
  READ_KDDI_TELOP_ATOM_FAILED,
  //100
  READ_KDDI_GPS_ATOM_FAILED,
  READ_FTYP_ATOM_FAILED,
  READ_DCMD_DRM_ATOM_FAILED,
  READ_DREF_ATOM_FAILED,
  TRACK_AUDIO_UNSUPPORTED_BITRATE,
  TRACK_VIDEO_UNSUPPORTED_BITRATE,
  TRACK_VIDEO_UNSUPPORTED_PROFILE,
  DRM_AUTHORIZATION_ERROR,
  DRM_ERROR_DEVICE_NOT_REGISTERED,
  DRM_RENTAL_COUNT_EXPIRED,
  DRM_OUT_OF_MEMORY,
  DRM_PLAYBACK_GENERAL_ERROR,
  SEEK_UNDERRUN
} MP4_ERROR_CODE;

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
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  ISucceedFail

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class ISucceedFail
{

public:
  bool FileSuccess()
  {
    return _success;
  }

protected:
  bool _success;
};


#endif /* __ISucceedFail_H__ */
