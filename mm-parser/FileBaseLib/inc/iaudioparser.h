/* =======================================================================
                              iaudioparser.h
DESCRIPTION
  Defines class IAudioParser that implements....

Copyright 2011-2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/iaudioparser.h#9 $
$DateTime: 2012/06/20 19:58:12 $
$Change: 2521756 $
========================================================================== */
#ifndef VIDEO_IAUDIOPARSER_H
#define VIDEO_IAUDIOPARSER_H

//======================================================================
// INCLUDES
//======================================================================
#include "parserdatadef.h"
//========================================================================
// Doxygen: Define the module for the interfaces and datatypes
//========================================================================

//======================================================================
// DATA TYPES
//======================================================================

/// Audio Track - defines start and stop fileposition for an audio track
typedef struct
{
  uint64 start; ///< track start position in the file
  uint64 end;   ///< track end position in the file
  uint64 size;  ///< track size
} AudioTrack;

/// Selects method used for determining seek file positon
enum parser_seek_method_type
{
  CALCULATE, ///< use a function to calculate seek position
  SCANAHEAD ///< use a seek table to lookup seek position
};

typedef enum iaudioreturn_t
{
  IAUDIO_FAILURE,
  IAUDIO_NOT_SUPPORTED,
  IAUDIO_SUCCESS
}IAudioReturnType;

//======================================================================
// CONSTANTS
//======================================================================

//======================================================================
// CLASS: IAudioParser
//
// DESCRIPTION:
/// \brief Interface to core parser functions that are audio format specific.
///
/// Audio Parser Core interface defines an audio format specific
/// parser component that implements a consistant set of functions
/// This interface defines attibutes with audio format specific values
/// and functions to perform the audio format specific parsing.
//======================================================================
//
class IAudioParser
{

//======================================================================
// PUBLIC CLASS METHODS
//======================================================================
public:

   // constants defined by the implementation
   //======================================================================

   /// Parse a fream header according to audio specific format and return
   /// the calculated frame size and frame time. AudioBuffer must contain an entire frame
   /// header, otherwise an error will be returned.
   /// This function must not consume data from the AudioBuffer (ie. does not
   /// update the AudioBuffer's pointers).
   ///
   /// @param[in] frame
   ///    A byte pointer to frame_size bytes, the frame header to be parsed
   ///    Note: the entire frame header is expected, thus it
   ///    must be at least <em>frame_header_size</em> in length.
   ///
   /// @param[out] frame_size
   ///    frame size determine from parsing the frame header
   ///
   /// @param[out] frame_time
   ///    frame duration in miliseconds determine from parsing the frame header
   ///
   /// @return
   ///  - AEE_SUCCESS: Frame header was valid, and frame size, frame_time is returned
   ///  - AEE_EFAILED: Frame header was INVALID, frame size, frame_time is not valid
   ///
   virtual IAudioReturnType parse_frame_header(/*in*/ uint8* frame,
                                               /*rout*/ uint32* frame_size,
                                               /*out*/uint32* frame_time)
                                               = 0;

   virtual void set_newfile_position(uint64 file_position) = 0;
   virtual void* get_id3v1_info(){return NULL;};
   virtual void* get_id3v2_info(){return NULL;};

protected:
    IAudioParser() {};
    virtual ~IAudioParser() {};
}; // end_class

#endif // VIDEO_IAUDIOPARSER_H

