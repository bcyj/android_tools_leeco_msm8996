/* linesize(132)
** pagesize(60)
** title("Dual Mode Subscriber Station")
** subtitle("Video Formats Services")
*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                 V I D E O   F O R M A T S   S E R V I C E S

GENERAL DESCRIPTION
  This module contains functions which handle the construction and
  deconstruction of "video files".  These are actually multimedia files (not
  just video) containing one or more streams (tracks) of media content, such
  as video, audio, or other types of media.

  Modules such as the video encode engine use this module to work with the
  appropriate file format.

EXTERNALIZED FUNCTIONS
  video_fmt_open
    This function opens a video file and prepares it for reading or editing.

INITIALIZATION AND SEQUENCING REQUIREMENTS

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* <EJECT> */
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/src/videofmt.c#11 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/05/03   enj     Removing FEATURE_VIDEOFMT (i.e. permanently enabling it)
08/04/03   rpw     Reformatted code in file reader.
                   Renamed most instances of "video_fmt_mp4"
                   to "video_fmt_mp4r".
                   Added file writer under "video_fmt_mp4w" - most of the
                   code came from the video encoder engine.
06/23/03   rpw     Replaced FEATURE_MP4_DECODER with FEATURE_VIDEOFMT.
11/01/02   rpw     Created file.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                        INCLUDE FILES FOR MODULE

===========================================================================*/
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"              /* Common definitions                      */

#include "videofmt.h"           /* Video format typedefs and prototypes    */
#include "videofmti.h"          /* Internal video format definitions       */
/* <EJECT> */
/*===========================================================================

                DECLARATIONS FOR MODULE

===========================================================================*/

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_open

DESCRIPTION
  This function opens an existing video file and prepares it for reading.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_open (
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data,
  video_fmt_type                 eFormat,
  uint8                          playingContext
)
{
    /* Dispatch command to appropriate format-level handler. */
    switch (eFormat)
    {
    case VIDEO_FMT_MP4     :
    case VIDEO_FMT_MP4_DASH:
        video_fmt_mp4r_open (callback_ptr, client_data,playingContext,
                             eFormat);
        break;

    case VIDEO_FMT_INVALID:
    default:
        /* Indicate the format is invalid or not supported. */
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                     "video_fmt_open: Invalid file format code: %d", eFormat);
        callback_ptr (VIDEO_FMT_FAILURE, client_data, NULL, NULL);
    }
}
