/* -*- Mode: C++ -*-
============================================================
 FILE: aacmetadata.cpp

 SERVICES: Audio

 DESCRIPTION:
 @file IAudioMetadata.h
 Defines an interface to access Audio Metadata. Audio Metadata includes
 Content metadata and technical metadata.
 Content metadata is defined as data that describes the content such as:
 IMelody, ID3v1 and ID3v2.
 Technical metadata is defined as data that describes the audio data
 such as SampleRate, BitRate, etc.

 Copyright (c) 2009-2014 QUALCOMM Technologies Incorporated.
 All Rights Reserved.
 QUALCOMM Technologies Proprietary and Confidential.
============================================================*/

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AACParserLib/main/latest/src/aacmetadata.cpp#24 $
$DateTime: 2013/07/09 02:22:12 $
$Change: 4061505 $
========================================================================== */

#define _AAC_METADATA_CPP_

//============================================================
// INCLUDES
//============================================================
#include "parserdatadef.h"
#include "aacmetadata.h"
#include "filebase.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"

//=============================================================
// GLOBALS
//=============================================================

//=============================================================
// MACROS
//=============================================================

//=============================================================
// CONSTANTS
//=============================================================

//=============================================================
// FUNCTION DEFINITONS
//=============================================================

//=============================================================
//   Class aacmetadata
//=============================================================

//=============================================================
// FUNCTION : Constructor
//
// DESCRIPTION
//  Constructor for Aacmetadata class
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================
//
aacmetadata::aacmetadata()
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "aacmetadata::aacmetadata");
#endif
  m_pid3v1 = NULL;
  m_bid3v1_present = FALSE;
  m_bid3v2_present = FALSE;
  m_ulTotalID3V2Tags = 0;
  memset(&m_aac_tech_metadata,0,sizeof(tech_data_aac));
}
//=============================================================
// FUNCTION : Destructor
//
// DESCRIPTION
//  Destructor for Aacmetadata class
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================
//
aacmetadata::~aacmetadata()
{
  for (uint32 ulCount = 0; ulCount < m_ulTotalID3V2Tags; ulCount++)
  {
    metadata_id3v2_type* pID3Meta = m_aID3AtomArray[ulCount];
    FreeID3V2MetaDataMemory(pID3Meta);
    MM_Free( pID3Meta);
  }
  m_ulTotalID3V2Tags = 0;
  if(m_pid3v1)
  {
    MM_Delete( m_pid3v1);
  }
  m_pid3v1 = NULL;
}

//=============================================================
// FUNCTION : set_id3v1
//
// DESCRIPTION
//  Set ID3v1 metadata
//
// PARAMETERS
//  id3v1 - metadata struct
//
// RETURN VALUE
//  AAC_SUCCESS | AAC_FAILURE --> success | failure
//
// SIDE EFFECTS
//  None
//=============================================================
//
uint32  aacmetadata::set_id3v1 (const metadata_id3v1_type* id3v1)
{
  uint32 ret = 0;
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "set_id3v1");
#endif
  m_bid3v1_present = TRUE;
  if(m_pid3v1)
  {
    MM_Delete( m_pid3v1);
    m_pid3v1 = NULL;
  }
  m_pid3v1 = MM_New(metadata_id3v1_type);
  if(id3v1)
  {
    ret = 1;
    (void) std_memmove(m_pid3v1,id3v1,STD_SIZEOF(metadata_id3v1_type));
  }
  return ret;
}

//=============================================================
// FUNCTION : set_id3v2
//
// DESCRIPTION
//  Set ID3v2 metadata
//
// PARAMETERS
//  id3v2 - metadata struct
//
// RETURN VALUE
//  AAC_SUCCESS | AAC_FAILURE --> success | failure
//
// SIDE EFFECTS
//  None
//=============================================================
//
uint32  aacmetadata::set_id3v2 (const metadata_id3v2_type* pid3v2)
{
  metadata_id3v2_type* pID3V2Meta = NULL;
  if (pid3v2)
  {
    pID3V2Meta = (metadata_id3v2_type*)MM_Malloc(sizeof(metadata_id3v2_type));
    if (pID3V2Meta)
    {
      m_bid3v2_present = true;
      memcpy(pID3V2Meta, pid3v2, sizeof(metadata_id3v2_type));
      m_aID3AtomArray += pID3V2Meta;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "Stored ID3 located @ FileOffset %llu",
                     pid3v2->file_position);
        m_ulTotalID3V2Tags++;
    }//! if (pID3V2Meta)
  }//! if (pid3v2)
  return PARSER_ErrorNone;
}

