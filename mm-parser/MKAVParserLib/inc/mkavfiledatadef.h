#ifndef _MKAV_FILE_DATA_DEFN_H
#define _MKAV_FILE_DATA_DEFN_H
/* =======================================================================
                              MKAVFileDataDef.h
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
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/inc/mkavfiledatadef.h#2 $
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
//Since MKAV tracks don't start with number 0,
//following table maintains index table for MKAV tracks.
typedef struct mkav_track_id_to_index_table_
{
  uint8  index;
  uint32 trackId;
  bool   bValid;
}MKAVTrackIdToIndexTable;
/* =======================================================================
**                          Function Declaration
** ======================================================================= */
#endif  /* _MKAV_FILE_DATA_DEFN_H */
