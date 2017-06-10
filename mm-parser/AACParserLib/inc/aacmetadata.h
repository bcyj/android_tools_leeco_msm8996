// -*- Mode: C++ -*-
/*============================================================
// FILE: aacmetadata.h
//
// SERVICES: Audio
//
// DESCRIPTION:
/// @file aacmetadata.h
/// Declarations for meta data . Audio Metadata includes
/// Content metadata and technical metadata.
/// Content metadata is defined as data that describes the content such as:
/// IMelody, ID3v1 and ID3v2.
/// Technical metadata is defined as data that describes the audio data
/// such as SampleRate, BitRate, etc.
///
/// Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
/// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AACParserLib/main/latest/inc/aacmetadata.h#9 $
$DateTime: 2013/07/09 02:22:12 $
$Change: 4061505 $

========================================================================== */

#ifndef __AAC_METADATA_H__
#define __AAC_METADATA_H__

//============================================================
// INCLUDES
//============================================================
// CS
#include "AEEStdDef.h" // std typedefs, ie. byte, uint16, uint32, etc.
#include "aacheaders.h"
#include "id3.h"
#include "ztl.h"
//============================================================
// DATA TYPES
//============================================================

//============================================================
// CONSTANTS
//============================================================

//============================================================
// CLASS: aacmetadata
//
// DESCRIPTION:
/// \brief Interface that provides the Metadata for aac format
///
/// This interface defines methods for getting the Metadata that is found
/// in the audio data. It is organized into user data and technical data.
/// User data may be one or more of IMelody, ID3v1, and ID3v2.
/// Technical metadata is particular to the Aduio format and take from either
/// the file header, or first couple of frame headers, or calculated from
/// the data found.
///
//============================================================
class aacmetadata {

//============================================================
// PUBLIC CLASS ATTRIBUTES
//============================================================
public:

//============================================================
// PRIVATE CLASS ATTRIBUTES
//============================================================
private:

  tech_data_aac m_aac_tech_metadata;

  metadata_id3v1_type *m_pid3v1;

  ZArray<metadata_id3v2_type*> m_aID3AtomArray;

  uint32 m_ulTotalID3V2Tags;
  bool m_bid3v1_present;
  bool m_bid3v2_present;
  friend class aacParser;

//============================================================
// PUBLIC CLASS METHODS
//============================================================
public:

  /// Constructor
  aacmetadata ();

  /// Destructor
  ~aacmetadata ();

//============================================================
/// IAudioMetadata interface functions
//============================================================

  /// ID3v1 Metadata, only valid when <b>id3v1_present</b> is true.
  /// Get ID3v1 metadata
  /// @returns id3v1 info set via set_id3v1;
  metadata_id3v1_type* get_id3v1 (){return m_pid3v1;};

  /// Set ID3v1 metadata
  /// @param [in] id3v1 - metadata struct
  uint32 set_id3v1 (/*in*/ const metadata_id3v1_type* id3v1);

  metadata_id3v2_type*  get_id3v2 (uint32 ulIndex)
  {
    if (ulIndex < m_ulTotalID3V2Tags)
    {
      return m_aID3AtomArray[ulIndex];
    }
    return NULL;
  };
  /// Set ID3v2 metadata
  /// @param [in] id3v2 - metadata struct
  uint32 set_id3v2 (/*in*/ const metadata_id3v2_type* id3v2);
  uint32 getTotalID3V2Entries() {return m_ulTotalID3V2Tags;};

};// end_class
#endif // __AACMETADATA_H__
