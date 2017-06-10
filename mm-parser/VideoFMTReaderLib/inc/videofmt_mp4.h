#ifndef VIDEOFMT_MP4_H
#define VIDEOFMT_MP4_H
/*===========================================================================

                  V I D E O   F O R M A T S   -   M P 4
                          H E A D E R   F I L E

DESCRIPTION
  This header file contains all the definitions necessary for the video
  formats module to interface with the MP4-specific video format support.

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/inc/videofmt_mp4.h#8 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/05/03   enj     Removing FEATURE_VIDEOFMT (i.e. permanently enabling it)
08/04/03   rpw     Reformatted code in file reader.
                   Renamed most instances of "video_fmt_mp4"
                   to "video_fmt_mp4r".
                   Added file writer under "video_fmt_mp4w" - most of the
                   code came from the video encoder engine.
06/23/03   rpw     Replaced FEATURE_MP4_DECODER with FEATURE_VIDEOFMT.
02/24/03   rpw     Removed video_fmt_encode, and renamed video_fmt_decode to
                   video_fmt_open.
11/01/02   rpw     Created file.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */

#include "videofmt_common.h"    /* Common video format definitions         */

/* <EJECT> */
/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

/* <EJECT> */
/*---------------------------------------------------------------------------
** VIDEOFMT_MP4 Public Function Prototypes
**---------------------------------------------------------------------------
*/

/*===========================================================================

FUNCTION  video_fmt_mp4r_open

DESCRIPTION
  This function opens an existing MP4 file and prepares it for reading.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void video_fmt_mp4r_open (
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data,
  uint8                          playingContext,
  video_fmt_type                 eFormat
);

/*===========================================================================

FUNCTION  video_fmt_mp4w_create

DESCRIPTION
  This function creates videofmt instance to write the file.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
extern void video_fmt_mp4w_create(
         const video_fmt_create_params_type  *params,
         video_fmt_status_cb_func_type       callback_ptr,
         void                                *client_data
);
#endif /* VIDEOFMT_MP4_H */
