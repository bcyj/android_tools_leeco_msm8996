// -*- Mode: C++ -*-
//=============================================================================
// FILE: mp3vbrheader.h
//
// SERVICES: Audio
//
// DESCRIPTION:
/// @file: mp3vbrheader.h
/// Declarations required for MP3 VBR header parsing
///
/// Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
/// All Rights Reserved.
/// Qualcomm Technologies Proprietary and Confidential.
//=============================================================================

//=============================================================================
//                      EDIT HISTORY FOR FILE
//
//  This section contains comments describing changes made to this file.
//  Notice that changes are listed in reverse chronological order.
//
//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/inc/mp3vbrheader.h#9 $
//$DateTime: 2012/07/25 21:19:29 $
//$Change: 2631309 $
//
//when            what, where, why
//--------   ---     ----------------------------------------------------------
//07/28/06      Created the file

//=============================================================================


#ifndef _MP3VBRHEADER_H_
#define _MP3VBRHEADER_H_

//=============================================================================
// INCLUDES
//=============================================================================

//CS includes
#include "AEEStdDef.h" // std typedefs, ie. byte, uint16, uint32, etc.

#include "mp3headers.h"
#include "oscl_file_io.h"
#include "mp3consts.h"
#include "mp3metadata.h"
#include "id3.h"
#include "parserdatadef.h"

//=============================================================================
// DATA TYPES
//=============================================================================

class mp3Parser;

//=============================================================================
// CLASS:
//
// DESCRIPTION:
/// MP3 Format parser parses the MP3 file headers and frame headers
/// (if required)
///
//=============================================================================
class mp3vbrheader
{

//=============================================================================
// PUBLIC CLASS ATTRIBUTES
//=============================================================================
public:

   /// Finds and parses VBR header if present in given file
   ///
   /// @param[in] file
   ///   IFilePort1 interface to input stream
   ///
   /// @param[in] first_sync_offset
   ///   Start position of first frame in given input file
   ///
   /// @param[in] hdr_info
   ///   MP3 header information (MPEG version and layer)
   ///
   /// @param[out] vbr_header_info
   ///   Interface for accessing vbr header information obtained after parsing
   ///
   /// @return
   ///    AEE_SUCCESS : success
   ///    AEE_EBADPARM : Invalid input parameters
   ///    AEE_EFAILED : general failure
   ///
   static PARSER_ERRORTYPE parse_mp3_vbr_header(
                    OSCL_FILE *m_MP3FilePtr,
                    uint64 first_sync_offset,
                    const struct tech_data_mp3 &hdr_info,
                    mp3vbrheader** vbr_header_info);

//=============================================================================
// PROTECTED CLASS ATTRIBUTES
//=============================================================================
protected:

//=============================================================================
// PRIVATE CLASS ATTRIBUTES
//=============================================================================
private:

//=============================================================================
// PUBLIC CLASS METHODS
//=============================================================================
public:

   /// Destructor
   virtual ~mp3vbrheader() { };

   /// Returns true if VBR header has enough information for calculation of
   /// duration and seek positions
   ///
   /// @param[out] seek_function_defined
   ///   true if optional duration and seek functions are provided
   ///
   /// @return
   ///    AEE_SUCCESS : success
   ///    AEE_EBADPARM : Invalid input parameters
   ///    AEE_EFAILED : general failure
   ///
   virtual PARSER_ERRORTYPE get_seek_function_defined (
                       /*rout*/ bool &seek_function_defined) const = 0;

   /// Calculates seek position based on time.
   /// This function is only implemented when the seek_function_defined is set
   /// to TRUE
   ///
   /// @param[in] time
   ///    Presentation time to seek to
   ///
   /// @param[in] playback_duration
   ///    Total playback duration
   ///
   /// @param[out] file_position
   ///    File position corresponding to the seek time provided
   ///
   /// @return
   ///    AEE_SUCCESS: Function succesfully calculated the playback duration
   ///    AEE_UNSUPPORTED: Function is not implemented for this audio format
   ///    AEE_EFAILED: general failure
   ///
   virtual PARSER_ERRORTYPE get_seek_position (
                       /*in*/ uint64 time, /*in*/ uint64 playback_duration,
                       /*rout*/uint64* file_position) const = 0;

   /// Returns total frame count information stored in the VBR header
   ///
   /// @param[out] total_frames
   ///    Total number of frames
   ///
   /// @return
   ///    AEE_SUCCESS: success
   ///    AEE_EFAILED: general failure
   ///
   virtual PARSER_ERRORTYPE get_total_frames (uint32 &total_frames) const = 0;

   /// Returns total bytes information stored in the VBR header
   ///
   /// @param[out] total_bytes
   ///    Total bytes
   ///
   /// @return
   ///    AEE_SUCCESS: success
   ///    AEE_EFAILED: general failure
   ///
   virtual PARSER_ERRORTYPE get_total_bytes (uint32 &total_bytes) const = 0;

   /// Output class attributes for debugging. These functions should
   /// compile into no-ops when DEBUG is not defined.
   ///
   ///  @param[in] str
   ///     Client string that should be displyed with other debug info
   ///
   virtual void show(const char* str) = 0;

   /// shortcut version of show function
   virtual void show() = 0;

//=============================================================================
// PROTECTED CLASS METHODS
//=============================================================================
protected:
   //===========================================================
   // These functions are available to be overriden by the child class
   // they are however not part of the public interface
   //===========================================================
   ::uint32 read_big_endian_value(const uint8* buffer, uint32 size) const ;

//=============================================================================
// PRIVATE CLASS METHODS
//=============================================================================
private:


}; // end_class

// length of Xing header id
#define XING_ID_SIZE 4

// Max count of TOC entries in Xing header
#define MP3_XING_TOC_LEN 100

// Flags for optional Xing header fields
#define MP3_XING_FRAMES_FLAG    0x01
#define MP3_XING_BYTES_FLAG     0x02
#define MP3_XING_TOC_FLAG       0x04
#define MP3_XING_QUAL_IND       0x08

typedef struct {
   uint32 flags;     ///< Flags for optional Xing header fields
   uint32 frames; ///< Total number of frames
   uint32 bytes;  ///< Total number of Bytes
   uint8 toc_entries[MP3_XING_TOC_LEN];  ///< TOC entries for seeking
   uint32 quality_ind; ///< Quality indicator
}mp3_xing_header_type;

class mp3xingheader: public  mp3vbrheader {

//=============================================================================
// PUBLIC CLASS ATTRIBUTES
//=============================================================================
public:

//=============================================================================
// PROTECTED CLASS ATTRIBUTES
//=============================================================================
protected:

//=============================================================================
// PRIVATE CLASS ATTRIBUTES
//=============================================================================
private:

   // maximum size of xing vbr header
   static const int m_max_xing_header_size;

   // Xing header information
   mp3_xing_header_type m_xing_header;

   /// constrctor; Parses the Xing VBR header
   ///
   /// @param[in] file
   ///   IFilePort1 interface to input stream
   ///
   /// @param[in] buffer
   ///   buffer containing raw data of Xing VBR header
   ///
   /// @param[in] buf_len
   ///   length of input buffer
   ///
   /// @param[out] result
   ///   O/p parameter that is updated with result of parsing
   ///
   //lint --e{1704} Constructor has private access specification - by design
   //lint --e{1712} default constructor not defined for class - by design
   mp3xingheader (const uint8* buffer,
                  uint32 buf_len, PARSER_ERRORTYPE &result);

//=============================================================================
// PUBLIC CLASS METHODS
//=============================================================================
public:

   /// destructor
   virtual ~mp3xingheader ();

   /// Finds and parses XING VBR header if present in given file
   ///
   /// @param[in] file
   ///   IFilePort1 interface to input stream
   ///
   /// @param[in] first_sync_offset
   ///   Start position of first frame in given input file
   ///
   /// @param[in] hdr_info
   ///   MP3 header information (MPEG version and layer)
   ///
   /// @param[out] vbr_header_info
   ///   Interface for accessing vbr header information obtained after parsing
   ///
   /// @return
   ///    AEE_SUCCESS : success
   ///    AEE_EBADPARM : Invalid input parameters
   ///    AEE_EFAILED : general failure
   ///
   static PARSER_ERRORTYPE parse_mp3_xing_header(
                    OSCL_FILE *m_MP3FilePtr,
                    uint64 first_sync_offset,
                    const struct tech_data_mp3 &hdr_info,
                    mp3vbrheader** vbr_header_info);

   /// Returns true if VBR header has enough information for calculation of
   /// duration and seek positions
   ///
   /// @param[out] seek_function_defined
   ///   true if optional duration and seek functions are provided
   ///
   /// @return
   ///    AEE_SUCCESS : success
   ///    AEE_EBADPARM : Invalid input parameters
   ///    AEE_EFAILED : general failure
   ///
   virtual PARSER_ERRORTYPE get_seek_function_defined (
               /*rout*/ bool& seek_function_defined) const;

   /// Calculates seek position based on time.
   /// This function is only implemented when the seek_function_defined is set
   /// to TRUE
   ///
   /// @param[in] time
   ///    Presentation time to seek to
   ///
   /// @param[in] playback_duration
   ///    Total playback duration
   ///
   /// @param[out] file_position
   ///    File position corresponding to the seek time provided
   ///
   /// @return
   ///    AEE_SUCCESS: Function succesfully calculated the playback duration
   ///    AEE_UNSUPPORTED: Function is not implemented for this audio format
   ///    AEE_EFAILED : general failure
   ///
   virtual PARSER_ERRORTYPE get_seek_position (
               /*in*/ uint64 time, /*in*/uint64 playback_duration,
               /*rout*/uint64* file_position) const;

   /// Returns total frame count information stored in the VBR header
   ///
   /// @param[out] total_frames
   ///    Total number of frames
   ///
   /// @return
   ///    AEE_SUCCESS: success
   ///    AEE_EFAILED : general failure
   ///
  virtual PARSER_ERRORTYPE get_total_frames (uint32 &total_frames) const;

   /// Returns total bytes information stored in the VBR header
   ///
   /// @param[out] total_bytes
   ///    Total bytes
   ///
   /// @return
   ///    AEE_SUCCESS: success
   ///    AEE_EFAILED: general failure
   ///
  virtual PARSER_ERRORTYPE get_total_bytes (uint32 &total_bytes) const;

   /// Output class attributes for debugging. These functions should
   /// compile into no-ops when DEBUG is not defined.
   ///
   ///  @param[in] str
   ///     Client string that should be displyed with other debug info
   ///
   virtual void show(const char* str);

   /// shortcut version of show function
   virtual void show();

//=============================================================================
// PROTECTED CLASS METHODS
//=============================================================================
protected:
   //===========================================================
   // These functions are available to be overriden by the child class
   // they are however not part of the public interface
   //===========================================================

//=============================================================================
// PRIVATE CLASS METHODS
//=============================================================================
private:

}; // end_class

// length of VBRI header id
#define VBRI_ID_SIZE 4

//forward declaration
class mp3vbriheader;

typedef struct {
   uint16 version; ///< Version ID
   uint16 delay; ///< Delay
   uint16 quality_ind; ///< Quality indicator
   uint32 frames; ///< Total number of frames
   uint32 bytes;   ///< Total number of Bytes
   uint16 entry_count; ///< Number of entries within TOC table
   uint16 scale; ///< Scale factor of TOC table entries
   uint16 toc_entry_size;///< Size per table entry in bytes
   uint16 frames_per_entry; ///< Frames per table entry
   uint16* toc_entries; ///< TOC entries for seeking
}mp3_vbri_header_type;

class mp3vbriheader: public  mp3vbrheader {

//=============================================================================
// PUBLIC CLASS ATTRIBUTES
//=============================================================================
public:

//=============================================================================
// PROTECTED CLASS ATTRIBUTES
//=============================================================================
protected:

//=============================================================================
// PRIVATE CLASS ATTRIBUTES
//=============================================================================
private:

   // maximum size of vbri vbr header
   // (toc entries field is not considered in this size)
   static const int m_max_vbri_header_size;

   // offset within first MP3 audio frame from where the VBRI header starts
   static const uint8 m_vbri_header_offset;

   // VBRI header information
   mp3_vbri_header_type m_vbri_header;

   /// constrctor; Parses the VBRI VBR header
   ///
   /// @param[in] file
   ///   IFilePort1 interface to input stream
   ///
   /// @param[in] buffer
   ///   buffer containing raw data of VBRI VBR header
   ///
   /// @param[in] buf_len
   ///   length of input buffer
   ///
   /// @param[out] result
   ///   O/p parameter that is updated with result of parsing
   ///
   //lint --e{1704} Constructor has private access specification - by design
   //lint --e{1712} default constructor not defined for class - by design
   mp3vbriheader ( OSCL_FILE *m_MP3FilePtr, uint8* buffer, int buf_len,
                  PARSER_ERRORTYPE &result);

   /// constrctor; Parses the VBRI VBR header
   ///
   /// @param[in] in_obj
   ///   mp3vbriheader instance
   ///
   ///
   mp3vbriheader (const mp3vbriheader& in_obj);

   /// assignment operator
   ///
   /// @param[in] in_obj
   ///   mp3vbriheader instance
   ///
   /// @return
   ///    current mp3vbriheader instance
   ///
   mp3vbriheader& operator= (const mp3vbriheader& in_obj);

//=============================================================================
// PUBLIC CLASS METHODS
//=============================================================================
public:

   /// destructor
   virtual ~mp3vbriheader ();

   /// Finds and parses VBRI VBR header if present in given file
   ///
   /// @param[in] file
   ///   IFilePort1 interface to input stream
   ///
   /// @param[in] first_sync_offset
   ///   Start position of first frame in given input file
   ///
   /// @param[in] hdr_info
   ///   MP3 header information (MPEG version and layer)
   ///
   /// @param[out] vbr_header_info
   ///   Interface for accessing vbr header information obtained after parsing
   ///
   /// @return
   ///    AEE_SUCCESS : success
   ///    AEE_EBADPARM : Invalid input parameters
   ///    AEE_EFAILED : general failure
   ///
   static PARSER_ERRORTYPE parse_mp3_vbri_header(
                                          OSCL_FILE *m_MP3FilePtr,
                                          uint64 first_sync_offset,
                                          mp3vbrheader** vbr_header_info);

   /// Returns true if VBR header has enough information for calculation of
   /// duration and seek positions
   ///
   /// @param[out] seek_function_defined
   ///   true if optional duration and seek functions are provided
   ///
   /// @return
   ///    AEE_SUCCESS : success
   ///    AEE_EBADPARM : Invalid input parameters
   ///    AEE_EFAILED : general failure
   ///
   virtual PARSER_ERRORTYPE get_seek_function_defined (
               /*rout*/ bool& seek_function_defined) const;

   /// Calculates seek position based on time.
   /// This function is only implemented when the seek_function_defined is set
   /// to TRUE
   ///
   /// @param[in] time
   ///    Presentation time to seek to
   ///
   /// @param[in] playback_duration
   ///    Total playback duration
   ///
   /// @param[out] file_position
   ///    File position corresponding to the seek time provided
   ///
   /// @return
   ///    AEE_SUCCESS: Function succesfully calculated the playback duration
   ///    AEE_UNSUPPORTED: Function is not implemented for this audio format
   ///    AEE_EFAILED : general failure
   ///
   virtual PARSER_ERRORTYPE get_seek_position (
               /*in*/ uint64 time, /*in*/uint64 playback_duration,
               /*rout*/uint64* file_position) const;

   /// Returns total frame count information stored in the VBR header
   ///
   /// @param[out] total_frames
   ///    Total number of frames
   ///
   /// @return
   ///    AEE_SUCCESS: success
   ///    AEE_EFAILED : general failure
   ///
  virtual PARSER_ERRORTYPE get_total_frames (uint32 &total_frames) const;

   /// Returns total bytes information stored in the VBR header
   ///
   /// @param[out] total_bytes
   ///    Total bytes
   ///
   /// @return
   ///    AEE_SUCCESS: success
   ///    AEE_EFAILED: general failure
   ///
  virtual PARSER_ERRORTYPE get_total_bytes (uint32 &total_bytes) const;

   /// Output class attributes for debugging. These functions should
   /// compile into no-ops when DEBUG is not defined.
   ///
   ///  @param[in] str
   ///     Client string that should be displyed with other debug info
   ///
   virtual void show(const char* str);

   /// shortcut version of show function
   virtual void show();

//=============================================================================
// PROTECTED CLASS METHODS
//=============================================================================
protected:
   //===========================================================
   // These functions are available to be overriden by the child class
   // they are however not part of the public interface
   //===========================================================

//=============================================================================
// PRIVATE CLASS METHODS
//=============================================================================
private:

}; // end_class

#endif //_MP3VBRHEADER_H_
