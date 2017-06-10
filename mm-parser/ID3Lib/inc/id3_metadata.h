#ifndef ID3_METADATA_H
#define ID3_METADATA_H
/* =======================================================================
                              id3_metadata.h
DESCRIPTION
  Definitions of data types used in ID3 class.

  Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ID3Lib/main/latest/inc/id3_metadata.h#15 $
$DateTime: 2013/10/21 02:25:48 $
$Change: 4628966 $

======================================================================*/

//======================================================================
//Includes
//======================================================================

#include "filesourcetypes.h"

#define MAX_ID3V1_TEXT_SIZE 31
#define MAX_ID3V1_YEAR_TEXT_SIZE 5

//4 byte textEncode + 2 ENUMs (8 bytes) + 68 bytes of Description +
//32 bytes of image format string
//It is to make size as integer aligned
#define FIXED_APIC_FIELDS_LENGTH 112

//! Byte Order Marker value in Little Endian Format
#define BYTE_ORDER_MARKER_IN_LE (0xFEFF)

//! Byte Order Marker value in Big Endian Format
#define BYTE_ORDER_MARKER_IN_BE (0xFFFE)

/// IMelody Name data is present when set
const uint32 IMELODY_NAME = 0x1;

/// IMelody Composer data is present when set
const uint32 IMELODY_COMPOSER = 0x2;

/// IMelody tag metadata. Contains a copy of IMelody data found in the file.
struct metadata_imelody_type
{
   //flag metadata present, zero if no data is present.
   uint32 data_present_flag;
   //file position of raw metadata
   uint32 file_position;
   //size of raw metadata
   uint32 size;
   //Name, null terminated string
   uint8 name[77];
   //Composer, null terminated string
   uint8 composer[77];
};

// ID3v1 Title data is present when set
const uint32 ID3V1_FLAG_TITLE = 0x1;

// ID3v1 Artist data is present when set
const uint32 ID3V1_FLAG_ARTIST = 0x2;

// ID3v1 Album data is present when set
const uint32 ID3V1_FLAG_ALBUM = 0x4;

// ID3v1 Year data is present when set
const uint32 ID3V1_FLAG_YEAR = 0x8;

// ID3v1 Comment data is present when set
const uint32 ID3V1_FLAG_COMMENT = 0x10;

// ID3v1 Genre data is present when set
const uint32 ID3V1_FLAG_GENRE = 0x20;

// ID3v1 tag metadata that contains any ID3v1 data found in the file.
// This structure contains a data that can exist in id3v1,
// it's offset(file location) and size.
// This structure can also be written to inorder to update the
// data in the file. A dirty flag is used to indicate whether the data in the
// structure has been modified from what has been written to the file. User
// should set the dirty bit whenever the data is modified. Writer clears the
// dirty bit when the data has been written.
//
struct metadata_id3v1_type
{
   //dirty bit, set when data has been modified.
   boolean dirty_bit;
   //Track Number, defined as last byte of comment (ID3v1.1)
   uint8 track;
   //Genre
   uint8 genre;
   //size of raw metadata
   uint32 size;
   //flag metadata present, zero if no data is present.
   uint32 data_present_flag;
   //fileposition of raw metadata
   uint64 file_position;
   //Song Title, null terminated
   uint8 title[MAX_ID3V1_TEXT_SIZE];
   //Artist, null terminated
   uint8 artist[MAX_ID3V1_TEXT_SIZE];
   //Album Title, null terminated
   uint8 album[MAX_ID3V1_TEXT_SIZE];
   //Year published, null terminated
   uint8 year[MAX_ID3V1_YEAR_TEXT_SIZE];
   //Comments, null terminated
   uint8 comment[MAX_ID3V1_TEXT_SIZE];
};

// ID3 content file position
struct id3v2_file_content
{
   boolean is_present;   //< is metadata present
   uint32 file_position; //< file position of raw metadata
   uint32 size;          //< size (bytes) of raw metadata
};

/// Create an apic_type
typedef id3v2_file_content apic_type;

/// Sequence of APICs since more than one can exist in an ID3v2 tag
typedef struct
{
   apic_type* data;//< Audio Picture Data
   int dataLen;    //< actual length
   int dataLenReq; ///< requested length
} apic_seq;

// ID3 content frame data information
struct id3v2_generic_frame_type
{
   //id3v2 frame type id
   uint8 id[5];
   //frame flags as defined by id3v2 spec
   uint16 flags;
   //indicates that content is in memory vs. file
   boolean in_memory;
   //content, only valid if in_memory is true
   uint8* content;
   //actual length
   int contentLen;
   //requested length
   int contentLenReq;
   //file position of the frame, valid when in_memory is false
   uint32 file_position;
   //size (bytes), valid when in_memory is false
   uint32 size;
};

// Sequence of generic_frames
typedef struct
{
  //generic frame data ptr
  id3v2_generic_frame_type* data;
  //actual length
  int dataLen;
  //length requested
  int dataLenReq;
} generic_frame_seq;

// ID3v2 Text frame type.
// Note: Text is a null terminated string for encoding 0x00
// For 0x01 encoding text is terminated by a Unicode BOM followed by
// a unicde null (0xFF 0xFE 0x00 0x00 or 0xFE 0xFF 0x00 0x00)
struct text_frame
{
   /// Text Encoding 0x00 = ISO-8859-1 (Latin-1 includes ASCII),
   //0x01 = Unicode
   uint8 encoding;
   //Text String in encoding defiend above
   char* text;
   //actual
   int textLen;
   //length requested
   int textLenReq;
};

struct id3v2_pic_info
{
  uint8         text_enc;
  uint8         img_format_string[MAX_IMG_FORMAT_LEN];
  uint8         desc[MAX_DESC_LEN];
  uint32        size;
  uint8         *pic_data;
  uint32        pic_data_len;
  FS_IMAGE_FORMAT   img_format;
  FS_PICTURE_TYPE   pict_type;
};

struct private_tag_info
{
   //TimeStamp available in the tag
   uint64 timeStamp;
   bool isTsValid;
};

struct encoder_delay_tag_info
{
  uint8 ucEncoderDelay[8]; //!Delay at start of playback
  uint8 ucPaddingDelay[8]; //!Delay at end of playback
  bool  bDelayPresent;
};

/// ID3v2 tag metadata that contains any ID3v2 data found in the file.
//  The entire data stream may be read directly from the file
/// using the file position and its size.
/// Most data is in the form of text frames which are equivalent to a string
/// where the first byte indicates the text encoding.
/// APICs are defined as a sequnce since there may be multiple Pictures
/// embedded in an id3v2 tag. For pictures only the file position and size
/// are provided since pictures are expected to be large and its uneconomical
/// to copy that data around.
///
/// TBD: clean up apic and generic frame sequences... creating a new struct
///
struct metadata_id3v2_type
{
   //is metadata present
   boolean is_present;
   //fileposition of raw metadata
   uint64 file_position;
   //size of raw metadata
   uint64 size;

   //Title - (TIT2) Title/songname/content description
   text_frame title;
   //Subtitle - (TIT3) Subtitle/Description refinement
   text_frame subtitle;
   //Copyright - (TCOP) Copyright message
   text_frame copyright;
   //Produced - ??? TBD: what frame type does this map to?
   text_frame produced;
   //Composer - (TCOM) '/' seperated list of composer names
   text_frame composer;
   //Lead Artist - (TPE1) '/' spearated list of lead artists
   text_frame artist;
   //Orchestra - (TPE2) The Band/Orchestra/Accompaniment
   text_frame orchestra;
   //Conductor - (TPE3) The name of the conductor
   text_frame conductor;
   //Lyricist - (TEXT) Names of lyricists or text writers
   text_frame lyricist;
   //Album Title - (TALB) Album title
   text_frame album;
   ///< Track - (TRCK) Track Number
   text_frame track;
   //Year  - (TYER) year of the recording
   text_frame year;
   //Publisher - (TPUB) name of publisher or label
   text_frame publisher;
   //Genre - (TCON) Contenet type
   text_frame genre;
   //Station - (TRSN) Internet Radio Station name
   text_frame station;
   //Encoder - (TENC) orginazation that encoded the audio
   text_frame encoder;
   //Length - (TLEN) Length of audio recording in milliseconds
   text_frame length;
   // Comments
   text_frame comment;
   // Album Artist
   text_frame album_artist;
   // Original Artist
   text_frame original_artist;
   // beats per minute
   text_frame beats_per_minute;
   //languages
   text_frame languages;
   //orignial album/movie/show title
   text_frame original_title;
   //orignial lyricist(s)/text(s) writers
   text_frame original_lyricists;
   //orignial release year
   text_frame original_release_year;
   //orignial filename
   text_frame original_filename;
   //recording date
   text_frame recording_dates;
   //ISRC(international standard recording code)
   text_frame isrc_code;
   //content group description
   text_frame content_group_desc;
   //software/hardware and encoding settings
   text_frame sw_hw_enc_settings;
   //official audio file webpage
   text_frame official_audio_webpage;
   //official artist/performer webpage
   text_frame official_artist_webpage;
   //official audio source webpage
   text_frame official_audio_source_webpage;
   //publishers official webpage
   text_frame official_publish_webpage;
   //commercial information
   text_frame commercial_information;
   //copyright-legal information
   text_frame copy_legal_information;
   //unique file identifier
   text_frame unq_file_iden;
   //unsychronized lyric/text transcription
   text_frame unsync_lyric_text_info;
   //user defined URL link frame
   text_frame user_def_url_link;
   //user defined text information frame
   text_frame user_def_text_info;
   //synchronized lyric/text
   text_frame sync_lyric_text;
   //synced tempo codes
   text_frame sync_tempo_codes;
   //reverb
   text_frame reverb;
   //relative volume adjustment
   text_frame rel_volume_adj;
   //popularimeter
   text_frame popularimeter;
   //music cd identifier
   text_frame music_cd_iden;
   //mpeg locn lookup table
   text_frame mpeg_locn_tbl;
   //linked information;
   text_frame linked_info;
   //involved people list
   text_frame involved_people_list;
   //general encapsulated object
   text_frame encapsulated_obj;
   //event timing codes
   text_frame timing_codes;
   //equalization
   text_frame equalization;
   //audio encryption
   text_frame audio_encryption;
   //encrypted meta frame
   text_frame encrypted_meta_frame;
   //play counter
   text_frame play_counter;
   //recommended buffer size
   text_frame recomm_buff_size;
   //date
   text_frame date;
   //playlist delay
   text_frame playlist_delay;
   //file type
   text_frame file_type;
   //time
   text_frame time;
   //initial key
   text_frame init_key;
   //media type
   text_frame media_type;
   //remix, modified by
   text_frame remix_mod_by;
   //part of a set
   text_frame part_of_set;
   //size
   text_frame size_in_bytes;
   //commercial frame
   text_frame commercial_frame;
   //encryption method reg.
   text_frame enc_method_reg;
   //group identification registration
   text_frame group_iden_reg;
   //ownership frame
   text_frame ownership_frame;
   //private frame
   text_frame pvt_frame;
   //position sync. frame
   text_frame posn_sync_frame;
   //file owner licensee
   text_frame file_own_licensee;
   //internet radio station name
   text_frame internet_radio_stn_name;
   //internet radio station owner
   text_frame internet_radio_stn_owner;
   //internet radio station homepage
   text_frame internet_radio_stn_homepage;
   //terms of use
   text_frame terms_of_use;
   //payment
   text_frame payment;

   //attached picture
   id3v2_pic_info pic_info;

   //APIC - (APIC) Sequence of Attached pictures (APIC) frames
   apic_type* pictures;
   //Sequence length actual
   int picturesLen;
   //Sequence length requested
   int picturesLenReq;

   //This tag is used to store timestamp of first frame in case of live streaming
   private_tag_info private_tag;

   //Tag used to store playback delays at start and end
   encoder_delay_tag_info encoder_delay_tag;

   // Generic frames - all other id3v2 framed objects are listed here.
   // unsupported object frames can be found here by itterating thru this
   // sequence and checking for the desired identifier.
   id3v2_generic_frame_type* generic_frames;
   //Sequence length actual
   int generic_framesLen;
   //Sequence length requested
   int generic_framesLenReq;
};

/// @} end id3v2

#endif

