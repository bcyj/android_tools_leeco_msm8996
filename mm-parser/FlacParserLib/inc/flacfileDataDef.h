#ifndef _FLAC_FILE_DATA_DEFN_H
#define _FLAC_FILE_DATA_DEFN_H
/* =======================================================================
                              flacDataDef.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2009-2013 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FlacParserLib/main/latest/inc/flacfileDataDef.h#2 $
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
//Since flac tracks don't start with number 0,
//following table maintains index table for flac tracks.
typedef struct flac_track_id_to_index_table_
{
  uint8  index;
  uint32 trackId;
  bool   bValid;
}FlacTrackIdToIndexTable;

/* =======================================================================
**                          Function Declaration
** ======================================================================= */
#endif  /* _FLAC_FILE_DATA_DEFN_H */

