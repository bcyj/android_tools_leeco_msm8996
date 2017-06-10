// -*- Mode: C++ -*-
//=============================================================================
// FILE: mp3metadata.h
//
// SERVICES: Audio
//
// DESCRIPTION:
/// @file mp3metadata.h
/// Declarations for meta data . Audio Metadata includes
/// Content metadata and technical metadata.
/// Content metadata is defined as data that describes the content such as:
/// IMelody, ID3v1 and ID3v2.
/// Technical metadata is defined as data that describes the audio data
/// such as SampleRate, BitRate, etc.
///
/// Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
/// All Rights Reserved.
/// Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/inc/mp3metadata.h#11 $
//$DateTime: 2013/08/22 23:16:43 $
//$Change: 4322584 $

//=============================================================================
#ifndef MP3METADATA_H
#define MP3METADATA_H

//=============================================================================
// INCLUDES
//=============================================================================
#include "mp3headers.h"
#include "id3_metadata.h"
#include "parserdatadef.h"
#include "ztl.h"

//=============================================================================
// DATA TYPES
//=============================================================================
//forward declaration
class mp3Parser;

class mp3metadata
{
public:
  friend class mp3parser;
  mp3metadata ();
  ~mp3metadata ();
  // Technical metadata extracted from file and frame headers
  tech_data_mp3 m_techmetadata;
  // ID3V1 metadata struct
  metadata_id3v1_type *m_id3v1;
  // ID3V2 metadata struct array
  //metadata_id3v2_type *m_pid3v2[MAX_ID3_ELEMENTS];
  ZArray<metadata_id3v2_type*> m_aID3AtomArray;
  // Flag indicating if ID3V1 metadata struct is present
  bool m_id3v1_present;
  // Flag indicating if ID3V2 metadata struct is present
  bool m_id3v2_present;
  uint32 m_ulTotalID3V2Tags;
  metadata_id3v1_type*  get_id3v1 () {return m_id3v1;};
  PARSER_ERRORTYPE  set_id3v1 (/*in*/ const metadata_id3v1_type* id3v1);
  metadata_id3v2_type*  get_id3v2 (uint32 ulIndex)
  {
    if (ulIndex < m_ulTotalID3V2Tags)
    {
      return m_aID3AtomArray[ulIndex];
    }
    return NULL;
  };
  uint32 getTotalID3V2Entries() {return m_ulTotalID3V2Tags;};
  PARSER_ERRORTYPE  set_id3v2 (/*in*/ const metadata_id3v2_type* id3v2);
};
#endif // MP3METADATA_H
