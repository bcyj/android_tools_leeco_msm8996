/*=======================================================================
 *                             sidxparserdatatypes.h
 *DESCRIPTION
 * Segment Index Information parser data types
 *
 * Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential..

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SIDXParserLib/main/latest/inc/sidxparserdatatypes.h#7 $
$DateTime: 2012/01/06 01:37:56 $
$Change: 2128420 $
*======================================================================== */
#ifndef SIDX_PARSER_DATA_TYPES_H
#define SIDX_PARSER_DATA_TYPES_H

//! forward declaration for istreamport
namespace video
{
  class iStreamPort;
}

//!Enum to list down types of chunks pointed by sidx.
typedef enum index_info_type_t
{
  INDEX_INFO_SIDX,
  INDEX_INFO_MOOF
}index_info_type;

//!sidx parser status
typedef enum sidx_parser_status_t
{
  SIDX_PARSER_INIT,                  //parser is being initialized
  SIDX_PARSER_INTERNAL_FATAL_ERROR,  //<FATAL>Fatal internal error occured,parsing failed...No further parsing will occur.
  SIDX_PARSER_READ_FAILED,           //<FATAL>Read failed while parsing/locating sidx.No further parsing will occur.
  SIDX_PARSER_OUT_OF_MEMORY,         //<FATAL>Malloc/ReAlloc failed while parsing sidx.No further parsing will occur.
  SIDX_PARSER_SIDX_PARSING_DONE,     //Parsed all sidx boxes successfully.Subsequent parsing request will do nothing.
  SIDX_PARSER_SIDX_BOX_DONE,         //Done parsing one/current sidx box.
  SIDX_PARSER_SIDX_PARSING_PENDING,  //Data under-run, parsing is pending.User needs to call parse_sidx again later.
  SIDX_PARSER_SIDX_NOT_AVAILABLE,    //SIDX atom is not available before MOOF atom. It is expected to have at least
                                     //one SIDX atom before MOOF.
}sidx_parser_status;

//!Structure to store sidx data pointed by a sidx.
typedef struct referenced_sidx_t
{
  unsigned long long offset;//absolute file offset of the sidx/moof.
  unsigned long      nsize; //size of the sidx/moof.
}referenced_sidx;

//!Structure to store data pointed by individual data chunk
typedef struct data_chunk_info_t
{
  bool                b_starts_with_sap;     //flag to indicate if data chunk contains random access point
  bool                b_reference_type;      //reference type(1=sidx,0=fragment/media);always 0 for data chunks.
  unsigned int        n_referenced_size;     //size of this data chunk
  unsigned int        n_subsegment_duration; //duration in milli-seconds for this data chunk
  unsigned int        n_sap_type;            //Indicates a SAP type
  unsigned int        n_sapdelta_time;       //delta time in milli-seconds for random access time
  unsigned long       n_sidxid;              //SIDX in which data chunk stored
  unsigned long long  n_start_time;          //Start time for this fragment(Sum of sub-segment durations of all previous fragments).
  unsigned long long  n_offset;              //absolue file file offset fot this data chunk
}data_chunk_info;

//structure to store data pointed by individual sidx box
typedef struct sidx_data_t
{
  unsigned long       n_size;            //size in bytes for this sidx
  unsigned long       n_offset;          //absolute file offset where this sidx starts
  unsigned char       n_version;         //version of this sidx
  unsigned int        n_flags;           //flag pointed by this sidx
  unsigned int        n_streamid;        //stream id pointed by this sidx
  unsigned int        n_scale;           //scale pointed by this sidx
  unsigned long long  n_earliestprestime;//earliest presention time in milli-seconds pointed by this sidx
  unsigned long long  n_firstoffset;     //nfirstoffset pointed by this sidx
  unsigned int        n_refcount;        //total number of references pointed by this dix
  unsigned int        n_moofpointed;     //Number of data fragments pointed by this sidx
  int                 n_moofstartid;     //Start id for all the moofs/data chunks pointed by this sidx.Set to -1 when not valid(nmoofpointed is 0).
  unsigned int        n_sidxpointed;     //Number of sidx boxes pointed by this sidx
  referenced_sidx*    p_referenced_sidx; //pointer to sidx info/array pointed by this sidx
}sidx_data;

#endif //SIDX_PARSER_DATA_TYPES_H


