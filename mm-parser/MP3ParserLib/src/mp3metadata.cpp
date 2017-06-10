// -*- Mode: C++ -*-
/* =======================================================================
                              mp3metadata.cpp
DESCRIPTION

  Defines an interface to access Audio Metadata. Audio Metadata includes
  Content metadata and technical metadata.
  Content metadata is defined as data that describes the content such as:
  IMelody, ID3v1 and ID3v2.
  Technical metadata is defined as data that describes the audio data
  such as SampleRate, BitRate, etc.

  Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/src/mp3metadata.cpp#20 $
$DateTime: 2013/08/22 23:16:43 $
$Change: 4322584 $

========================================================================== */

//=============================================================================
// INCLUDES
//=============================================================================

//local includes
#include "mp3metadata.h"
#include "mp3parser.h"
#include "MMMemory.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"

//=============================================================================
// GLOBALS
//=============================================================================

//=============================================================================
// CONSTANTS
//=============================================================================

//=============================================================================
// FUNCTION DEFINATONS
//=============================================================================

//=============================================================
// FUNCTION : Constructor
//
// DESCRIPTION
//  Constructor for mp3metadata class
//
// PARAMETERS
//  pEnv : CS Env required for malloc,createinstance etc.
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================
//
mp3metadata::mp3metadata()
{
   //Initialize instance variables
   memset (&m_techmetadata,0, sizeof(m_techmetadata));

   m_id3v1            = NULL;
   m_ulTotalID3V2Tags = 0;

   m_id3v1_present = false;
   m_id3v2_present = false;
}
//=============================================================
// FUNCTION : Destructor
//
// DESCRIPTION
//  Destructor for mp3metadata class
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
mp3metadata::~mp3metadata()
{
   for (uint32 ulCount = 0; ulCount < m_ulTotalID3V2Tags; ulCount++)
   {
     metadata_id3v2_type* pID3Meta = m_aID3AtomArray[ulCount];
     FreeID3V2MetaDataMemory(pID3Meta);
     MM_Free( pID3Meta);
   }
   m_ulTotalID3V2Tags = 0;
   MM_Delete( m_id3v1);
   m_id3v1 = NULL;
}

//=============================================================================
// FUNCTION : set_id3v1
//
// DESCRIPTION
//  Set ID3v1 metadata
//
// PARAMETERS
//  id3v1 - metadata struct
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_PARM : invalid parameters
//  MP3_FAILURE: id3v1 tag is already set
//  AEE_ENOMEMORY : Not enough memory
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3metadata::set_id3v1 (/*in*/ const metadata_id3v1_type* id3v1)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
   if (!id3v1)
   {
     result = PARSER_ErrorInvalidParam;
   }
   if (m_id3v1)
   {
      result = PARSER_ErrorDefault;
   }
   if(result == PARSER_ErrorNone)
   {
     m_id3v1_present = true;
     m_id3v1 = MM_New(metadata_id3v1_type);
     if (!m_id3v1 )
     {
       result = PARSER_ErrorInsufficientBufSize;
     }
     else
     {
       (void) std_memmove(m_id3v1, id3v1, STD_SIZEOF(*m_id3v1));
     }
   }
   return result;
}

//=============================================================================
// FUNCTION : set_id3v2
//
// DESCRIPTION
//  Set ID3v2 metadata
//
// PARAMETERS
//  id3v2 - metadata struct
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_PARM : invalid parameters
//  MP3_FAILURE: id3v2 tag is already set
//  AEE_ENOMEMORY : Not enough memory
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3metadata::set_id3v2 (/*in*/ const metadata_id3v2_type* pid3v2)
{
  metadata_id3v2_type* pID3V2Meta = NULL;
  if (pid3v2)
  {
    pID3V2Meta = (metadata_id3v2_type*)MM_Malloc(sizeof(metadata_id3v2_type));
    if (pID3V2Meta)
    {
      m_id3v2_present = true;
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

