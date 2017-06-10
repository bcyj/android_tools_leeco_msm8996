#ifndef VIDEOFMT_H
#define VIDEOFMT_H
/*===========================================================================

              V I D E O   F O R M A T S    H E A D E R    F I L E

DESCRIPTION
  This header file contains all the definitions necessary for other
  tasks to interface with the video formats module.

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/inc/videofmt.h#6 $

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
#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */
#include "videofmt_common.h"    /* Common video format definitions         */
#include "videofmt_mp4.h"       /* MP4 format typedefs and prototypes      */

/* <EJECT> */
/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

/* <EJECT> */
/*---------------------------------------------------------------------------
** VIDEOFMT Public Function Prototypes
**---------------------------------------------------------------------------
*/

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
extern void video_fmt_open (
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data,
  video_fmt_type                 format,
  uint8                          playingContext
);

#endif /* VIDEOFMT_H */
