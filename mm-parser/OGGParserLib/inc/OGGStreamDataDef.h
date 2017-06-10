#ifndef _OGG_STREAM_DATA_DEFN_H
#define _OGG_STREAM_DATA_DEFN_H
/* =======================================================================
                              OGGStreamDataDef.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/inc/OGGStreamDataDef.h#2 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "AEEStdDef.h"
/* ==========================================================================
                       DATA DECLARATIONS
========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */
//Since OGG tracks don't start with number 0,
//following table maintains index table for OGG tracks.
typedef struct ogg_track_id_to_index_table_
{
  uint8  index;
  uint32 trackId;
  bool   bValid;
}OggTrackIdToIndexTable;
/* =======================================================================
**                          Function Declaration
** ======================================================================= */
#endif  /* _OGG_STREAM_DATA_DEFN_H */
