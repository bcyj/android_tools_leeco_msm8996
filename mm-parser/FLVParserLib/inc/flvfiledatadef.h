#ifndef _FLV_FILE_DATA_DEFN_H
#define _FLV_FILE_DATA_DEFN_H
/* =======================================================================
                              flvfiledatadef.h
DESCRIPTION
Contains datatype declarations for flv format

Copyright (c) 2012-2013 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FLVParserLib/main/latest/inc/flvfiledatadef.h#3 $
========================================================================== */

//Tracks may not start with 0,
//following table maintains index table for tracks.
typedef struct flv_track_id_to_index_table_
{
  uint8  index;
  uint32 trackId;
  bool   bValid;
}FLVTrackIdToIndexTable;
/* =======================================================================
**                          Function Declaration
** ======================================================================= */
#endif  /* _FLV_FILE_DATA_DEFN_H */
