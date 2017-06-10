/* linesize(132)
** pagesize(60)
** title("Dual Mode Subscriber Station")
** subtitle("Video Formats Services MP4 Reader")
*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

               V I D E O   F O R M A T S   -   M P 4   R E A D E R

GENERAL DESCRIPTION
  This module contains functions which parse and read files in the MP4
  file format.

REFERENCES
  ISO/IEC 14496, Part 1: Systems (MPEG-4)
  ISO/IEC 14496, Part 2: Visual (MPEG-4)
  ISO/IEC 14496, Part 3: Audio (MPEG-4)
  3GPP TS 26.244V12.3.0, 3GPP File Format
EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2008-2015 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* <EJECT> */
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/src/videofmt_mp4r_parse.c#105 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/12/06   dbv     Added ability to parse avcc atom irrespective of its size
12/09/05   rpw     Added AMR-WB and AMR-WB+ support
12/08/04   rpw     Added ability to read/write language code
                   from 'mdhd' atom.
                   Added ability to read width/height and origin
                   information from 'tkhd' atom.
09/16/03   rpw     Added parsing of MP3 header for sampling rate and number
                   of channels.
09/05/03   enj     Removing FEATURE_VIDEOFMT (i.e. permanently enabling it)
08/18/03   rpw     Previous FFWD fix not complete - applied complete fix and
                   expanded out and commented parts of the logic to make it
                   more readable.
08/12/03    ny     Added support for the case when it is required to forward
                   reposition a stream that only has one I-frame. Basically
                   no repositioning will be attempted in this case.
08/12/03    ny     Fixed FFWD functionality.
07/31/03   rpw     Added "sampling_frequency" parsing for AAC audio.
07/29/03   rpw     Added VIDEO_FMT_STREAM_AUDIO_EVRC_PV to distinguish
                   between EVRC streams in 3GPP2 and PacketVideo's variant
                   which uses AudioSampleEntry with a special OTI value.
07/29/03   rpw     Fixed bug where reading samples by units of frames from a
                   stream with fixed sample sizes would try to read
                   non-existant 'stsz' atom.
07/28/03    ny     Initialization of default values for some file-level fields:
                   file_level_data.audio_only_allowed = TRUE;
                   file_level_data.video_only_allowed = TRUE;
                   file_level_data.no_rand_access = FALSE;
07/11/03   rpw     Added FEATURE_AAC_ONLY_EXTRA_RESYNC_MARKERS code to insert
                   resync markers at the beginning of each AAC frame, for
                   AAC bitstreams with audio_object_type between 17 and 27,
                   inclusive.
07/11/03   rpw     Added workaround to resolve CR 32732, where files with a
                   zero "sample_count" field in the 'stsz' atom caused
                   problems.  In this case, the code now recovers the frame
                   count by summing all the "sample_count" fields in the table
                   entries of the 'stts' atom.
07/10/03   rpw     Bug fix described on 07/01/03 was not done properly.
                   Fixed the bug the right way this time.
07/07/03   rpw     Fixed bug where code which parses H.263 bitstream to
                   determine frame size was looking at first sample in 5th
                   chunk, not first sample in 1st chunk.
07/07/03   rpw     Added missing code to fail upon parsing bad H.263
                   source_format field.
07/01/03   rpw     Fixed bug where parsing of audio stream would generate a
                   failure when parsing the very end.
06/30/03   rpw     Fixed bug where audio bitstream parsing would run out of
                   syntax before running out of bitstream.
06/25/03   rpw     Created file, from original videofmt_mp4.c file.  Renamed
                   all symbols from video_fmt_mp4r_* to video_fmt_mp4r_*.
06/23/03   rpw     Replaced FEATURE_MP4_DECODER with FEATURE_VIDEOFMT.
06/18/03   rpw     Fixed bug where reading a stream and spanning more than
                   one chunk in the file would cause the wrong number of
                   bytes to be read from the movie file.
06/11/03   rpw     Added parsing of MPEG-4 AAC audio bitstream headers.
06/03/03   rpw     Added code to recognize MPEG-4 video short header format
                   as H.263 content.  Added type for MPEG-4 AAC audio.
05/28/03   rpw     Fixed bug where data overrun by client would cause an
                   abrupt and erroneous state change to idle.
05/23/03   rpw     Added timed text atom minimal support.
05/22/03   rpw     Added interface for finding nearest sync sample at or
                   after any given sample in a stream.
                   Added stream byte offset to sample information.
05/22/03   rpw     Fixed typo in 3GPP2 EVRC sample entry atom names.
03/28/03   rpw     Fixed bug in assuming length of udta child atoms.
03/18/03   rpw     Merged in changes from MPEG4_FILE_FORMAT branch.
03/17/03   rpw     Fixed stream reading bugs when read unit is set
                   to VIDEO_FMT_DATA_UNIT_FRAME.
03/10/03   rpw     Fixed bug in getting size of first frame - header was not
                   included in the length.
03/05/03   rpw     Fixed bug when calling stream read function to obtain
                   read amount without actually reading.
02/27/03   rpw     Fixed bug where sample delta count was not reset for
                   each 'stts' entry.
02/26/03   rpw     Added code to handle case where 'stss' table is not
                   present in a file.  Also fixed handling of 'stsz' table
                   when samples are fixed length.
02/25/03   rpw     Added method of getting sample information (timestamps,
                   sample sizes, sync samples, etc.)
02/24/03   rpw     Replaced reading and caching of entire 'stco', 'stsc', and
                   'stsz' chunks with partial reading and caching, and only
                   when needed to read stream data.  This effectively lifts
                   the maximum file size limit and reduces the initial file
                   parsing delay.  Also the initial file parsing delay
                   is now the same for all files, big or small.
02/18/03   rpw     Added AMR track and sample entry atom support
11/04/02   rpw     Created file.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                        INCLUDE FILES FOR MODULE

===========================================================================*/
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"              /* Common definitions                      */

#include "videofmt_mp4.h"       /* MP4 format typedefs and prototypes      */
#include "videofmt_mp4r.h"      /* Internal MP4 reader definitions         */
#include <string.h>             /* Memory Manipulation routines            */
#include <stdio.h>              /* Standard I/O routines                   */

/* <EJECT> */
/*===========================================================================

                DECLARATIONS FOR MODULE

===========================================================================*/

/* 'devc' atom fields definitions */
static video_fmt_atom_field_type devc_fields[] =
{
  { 5, 1, 0}       /* frames per sample  */
};
static video_fmt_consume_atom_structure_type devc =
{
    video_fmt_mp4r_devc_prep_dest, /* dest_proc_func               */
    1,                             /* field_count for devc_fields  */
    devc_fields                    /* point to 'devc' fields       */
};

/* 'decb' atom field definitions */
static video_fmt_atom_field_type decb_fields[] =
{
  { 5, 1, 0}       /* frames per sample  */
};
static video_fmt_consume_atom_structure_type decb =
{
    video_fmt_mp4r_decb_prep_dest, /* dest_proc_func               */
    1,                             /* field_count for decb_fields  */
    decb_fields                    /* point to 'decb' fields       */
};

/* 'decw' atom field definitions */
static video_fmt_atom_field_type decw_fields[] =
{
  { 5, 1, 0}       /* frames per sample  */
};
static video_fmt_consume_atom_structure_type decw =
{
    video_fmt_mp4r_decw_prep_dest, /* dest_proc_func               */
    1,                             /* field_count for decw_fields  */
    decw_fields                    /* point to 'decw' fields       */
};

/* 'dqcp' atom fields definitions */
static video_fmt_atom_field_type dqcp_fields[] =
{
  { 5, 1, 0}       /* frames per sample  */
};
static video_fmt_consume_atom_structure_type dqcp =
{
    video_fmt_mp4r_dqcp_prep_dest, /* dest_proc_func               */
    1,                             /* field_count for dqcp_fields  */
    dqcp_fields                    /* point to 'dqcp' fields       */
};

/* 'stco' atom fields definitions */
static video_fmt_atom_field_type stco_fields[] =
{
    { 4, 4, 0}      /* entry count        */
};
static video_fmt_consume_atom_structure_type stco =
{
    video_fmt_mp4r_stco_prep_dest,  /* dest_proc_func               */
    1,                              /* field_count for stco_fields  */
    stco_fields                     /* point to fields of atom      */
};

/* here we are adding co64 atom parsing support. Currently
   we don't support 64 bit filelds so we are parsing the co64 atom
   and taking the least 32 bits as the offsets.
*/
static video_fmt_consume_atom_structure_type co64 =
{
    video_fmt_mp4r_co64_prep_dest,  /* dest_proc_func               */
    1,                              /* field_count for stco_fields  */
    stco_fields                     /* point to fields of atom      */
};

/* 'stsc' atom fields definitions */
static video_fmt_atom_field_type stsc_fields[] =
{
    { 4, 4, 0}      /* entry count        */
};
static video_fmt_consume_atom_structure_type stsc =
{
    video_fmt_mp4r_stsc_prep_dest,  /* dest_proc_func               */
    1,                              /* field_count for stsc_fields  */
    stsc_fields                     /* point to fields of atom      */
};

/* 'stts' atom fields definitions */
static video_fmt_atom_field_type stts_fields[] =
{
    { 4, 4, 0}      /* entry count        */
};
static video_fmt_consume_atom_structure_type stts =
{
    video_fmt_mp4r_stts_prep_dest,  /* dest_proc_func               */
    1,                              /* field_count for stts_fields  */
    stts_fields                     /* point to fields of atom      */
};

/* 'ctts' atom fields definitions */
static video_fmt_atom_field_type ctts_fields[] =
{
    { 0, 1, 0},     /* version info       */
    { 4, 4, 0}      /* entry count        */
};
static video_fmt_consume_atom_structure_type ctts =
{
    video_fmt_mp4r_ctts_prep_dest,  /* dest_proc_func               */
    2,                              /* field_count for ctts_fields  */
    ctts_fields                     /* point to fields of atom      */
};

/* 'stss' atom fields definitions */
static video_fmt_atom_field_type stss_fields[] =
{
    { 4, 4, 0}      /* entry count        */
};
static video_fmt_consume_atom_structure_type stss =
{
    video_fmt_mp4r_stss_prep_dest,  /* dest_proc_func               */
    1,                              /* field_count for stss_fields  */
    stss_fields                     /* point to fields of atom      */
};

/* 'stsd' atom fields definitions */
static video_fmt_atom_field_type stsd_fields[] =
{
  { 4, 4, 0}      /* entry count        */
};
static video_fmt_consume_atom_structure_type stsd =
{
  video_fmt_mp4r_stsd_prep_dest, /* dest_proc_func                  */
  1,                             /* field_count for stsd_fields     */
  stsd_fields                    /* point to fields of atom         */
};

/* 'mp4v' container atom */
static video_fmt_consume_atom_structure_type mp4v =
{
    NULL,
    78,         /* size of atom (without header) */
    NULL
};

/* 'encv' atom fields definitions */
static video_fmt_atom_field_type encv_fields[] =
{
    { 24, 2, 0},       /*  width    */
    { 26, 2, 0},       /*  height  */
    { 28, 50, 0},      /*  skip */
};

/* 'encv' container atom */
static video_fmt_consume_atom_structure_type encv =
{
    video_fmt_mp4r_encv_prep_dest,   /* dest_proc_func                  */
    3,                               /* field_count for encv_fields     */
    encv_fields                      /* point to 'encv' fields of atom  */
};

/* 'enca' container atom */
static video_fmt_consume_atom_structure_type enca =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 'sinf' container atom
    Here we are declaring this as sinf_atom because this sinf
    symbol is already defined else where
*/
static video_fmt_consume_atom_structure_type sinf_atom =
{
    NULL,
    0,         /* size of atom (without header) */
    NULL
};

/* 'schi' container atom */
static video_fmt_consume_atom_structure_type schi =
{
    NULL,
    0,         /* size of atom (without header) */
    NULL
};


/* 'frma' atom fields definitions */
static video_fmt_atom_field_type frma_fields[] =
{
    { 0, 4, 0}       /* data_format  */
};

/* 'frma' atom fields definitions */
static video_fmt_consume_atom_structure_type frma =
{
    video_fmt_mp4r_frma_prep_dest, /* dest_proc_func               */
    1,                             /* field count for frma_fields  */
    frma_fields                    /* point to 'frma' fields       */
};


/* 'schm' atom fields definitions */
static video_fmt_atom_field_type schm_fields[] =
{
    { 4, 4, 0},       /* Scheme type */
    { 8, 4, 0}        /* Scheme version */
};

/* 'schm' atom fields definitions */
static video_fmt_consume_atom_structure_type schm =
{
    video_fmt_mp4r_schm_prep_dest, /* dest_proc_func               */
    2,                             /* field count for schm_fields  */
    schm_fields                    /* point to 'schm' fields       */
};

/* 'odkm' container atom */
static video_fmt_consume_atom_structure_type odkm =
{
    NULL,
    4,         /* size of atom (without header) */
    NULL
};

/* 'ohdr' atom fields definitions */
static video_fmt_atom_field_type ohdr_fields[] =
{
    { 0, 1, 0},       /*  EncryptionMethod */
    { 1, 1, 0},       /* PaddingScheme  */
    { 2, 8, 0},       /* PlaintextLength  */
    { 10, 2, 0},       /* ContentIDLength   */
    { 12, 2, 0},       /*  RightsIssuerURLLength */
    { 14, 2, 0}       /*  TextualHeadersLength */
};

/* 'ohdr' atom fields definitions */
static video_fmt_consume_atom_structure_type ohdr =
{
    video_fmt_mp4r_ohdr_prep_dest, /* dest_proc_func               */
    6,                             /* field count for ohdr_fields  */
    ohdr_fields                    /* point to 'ohdr' fields       */
};

/* 'odaf' atom fields definitions */
static video_fmt_atom_field_type odaf_fields[] =
{
    { 4, 1, 0},       /* SelectiveEncryption  */
    { 5, 1, 0},       /* KeyIndicatorLength  */
    { 6, 1, 0}        /* IVLength */
};

/* 'odaf' atom fields definitions */
static video_fmt_consume_atom_structure_type odaf =
{
    video_fmt_mp4r_odaf_prep_dest, /* dest_proc_func               */
    3,                             /* field count for odaf_fields  */
    odaf_fields                    /* point to 'odaf' fields       */
};

/* 'mdri' container atom */
static video_fmt_consume_atom_structure_type mdri =
{
    NULL,
    0,         /* size of atom (without header) */
    NULL
};
/* Following three types of sound version atoms are defined in
   MP4 Audio atom. Each version has different size.
   Sound version bit field is 2bits and its available at bit offset# 8.
   Parser will read this field and decides on amount of that needs to be
   skipped.
*/
#define SOUND_VER_0_SIZE (28)
#define SOUND_VER_1_SIZE (44)
#define SOUND_VER_2_SIZE (64)
/* 'mp4a' container atom */
static video_fmt_consume_atom_structure_type mp4a =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 'wave' container atom */
static video_fmt_consume_atom_structure_type wave =
{
  NULL,
  0,         /* size of atom (without header) */
  NULL
};

/* 'mp4s' container atom */
static video_fmt_consume_atom_structure_type mp4s =
{
    NULL,
    8,         /* size of atom (without header) */
    NULL
};

/* 'sawp' container atom */
static video_fmt_consume_atom_structure_type sawp =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 'sevc' container atom */
static video_fmt_consume_atom_structure_type sevc =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 'evrc' container atom - TEMPORARY fix to OTT authoring tool */
static video_fmt_consume_atom_structure_type evrc =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 'secb' container atom */
static video_fmt_consume_atom_structure_type secb =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 'secw' container atom */
static video_fmt_consume_atom_structure_type secw =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 'sqcp' container atom */
static video_fmt_consume_atom_structure_type sqcp =
{
    NULL,
    28,         /* size of atom (without header) */
    NULL
};

/* 's263' atom fields definitions */
static video_fmt_atom_field_type s263_fields[] =
{
    { 24, 2, 0},       /*  width    */
    { 26, 2, 0},        /*  height    */
    { 28, 50, 0}        /* skip these many bytes */
};

/* 's263' container atom */
static video_fmt_consume_atom_structure_type s263 =
{
    video_fmt_mp4r_s263_prep_dest,   /* dest_proc_func                  */
    3,                               /* field_count for s263_fields     */
    s263_fields                      /* point to 's263' fields of atom         */
};

static video_fmt_atom_field_type mfhd_fields[] =
{
    { 4, 4, 0}      /* sequence_number     */
};
static video_fmt_consume_atom_structure_type mfhd =
{
    video_fmt_mp4r_mfhd_prep_dest,  /* dest_proc_func               */
    1,                              /* field_count for mfhd_fields  */
    mfhd_fields                     /* point to 'mfhd' fields       */
};


/* This table organizes information about the different MP4 file atom types
** that are defined in the file format.
*/
static video_fmt_mp4r_atom_info_type video_fmt_mp4r_atom_info_table [] = {
    {FTYP_TYPE /* 'ftyp' */, TRUE,video_fmt_mp4r_process_atom_ftyp, 0},
    {MOOV_TYPE /* 'moov' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {MVHD_TYPE /* 'mvhd' */, FALSE, video_fmt_mp4r_process_atom_mvhd, 0},
    {TRAK_TYPE /* 'trak' */, TRUE, video_fmt_mp4r_process_atom_trak, 0},
    {TKHD_TYPE /* 'tkhd' */, TRUE, video_fmt_mp4r_process_atom_tkhd, 0},
    {TREF_TYPE /* 'tref' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {HINT_TYPE /* 'hint' */, FALSE, video_fmt_mp4r_process_atom_tref_child, 0},
    {DPND_TYPE /* 'dpnd' */, FALSE, video_fmt_mp4r_process_atom_tref_child, 0},
    {IPIR_TYPE /* 'ipir' */, FALSE, video_fmt_mp4r_process_atom_tref_child, 0},
    {MPOD_TYPE /* 'mpod' */, FALSE, video_fmt_mp4r_process_atom_tref_child, 0},
    {SYNC_TYPE /* 'sync' */, FALSE, video_fmt_mp4r_process_atom_tref_child, 0},
    {MDIA_TYPE /* 'mdia' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {MDHD_TYPE /* 'mdhd' */, FALSE, video_fmt_mp4r_process_atom_mdhd, 0},
    {HDLR_TYPE /* 'hdlr' */, FALSE, video_fmt_mp4r_process_atom_hdlr, 0},
    {MINF_TYPE /* 'minf' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {DINF_TYPE /* 'dinf' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {DREF_TYPE /* 'dref' */, FALSE, video_fmt_mp4r_process_atom_skip, 0},
    {STBL_TYPE /* 'stbl' */, TRUE, video_fmt_mp4r_process_atom_stbl, 0},
    {STSD_TYPE /* 'stsd' */, TRUE, video_fmt_mp4r_process_container, &stsd},
    {MP4V_TYPE /* 'mp4v' */, TRUE, video_fmt_mp4r_process_container, &mp4v},
    /* These are for PDCF file formats  */
    {ENCV_TYPE /* 'encv' */, TRUE, video_fmt_mp4r_process_container, &encv},
    {ENCA_TYPE /* 'enca' */, TRUE, video_fmt_mp4r_process_container, &enca},
    {SAIZ_TYPE /* 'saiz*/ , FALSE, video_fmt_mp4r_process_atom_saiz,0},
    {SAIO_TYPE /* 'saio*/ , FALSE, video_fmt_mp4r_process_atom_saio,0},
    {PSSH_TYPE /* 'pssh' */, TRUE, video_fmt_mp4r_process_atom_skip,0},
    {SINF_TYPE /* 'sinf' */, TRUE, video_fmt_mp4r_process_atom_skip,0},
    {FRMA_TYPE /* 'frma' */, FALSE, video_fmt_mp4r_parse_atom, &frma},
    {SCHM_TYPE /* 'schm' */, TRUE, video_fmt_mp4r_process_container, &schm},
    {SCHI_TYPE /* 'schi' */, TRUE, video_fmt_mp4r_process_container, &schi},
    {ODKM_TYPE /* 'odkm' */, TRUE, video_fmt_mp4r_process_container, &odkm},
    {OHDR_TYPE /* 'ohdr' */, FALSE, video_fmt_mp4r_parse_atom, &ohdr},
    {ODAF_TYPE /* 'odaf' */, FALSE, video_fmt_mp4r_parse_atom, &odaf},
    {MDRI_TYPE /* 'mdri' */, TRUE, video_fmt_mp4r_process_container, &mdri},
    {MP4A_TYPE /* 'mp4a' */, TRUE, video_fmt_mp4r_process_container, &mp4a},
    {WAVE_TYPE /* 'wave' */, TRUE, video_fmt_mp4r_process_container, &wave},
    {SAMR_TYPE /* 'samr' */, TRUE, video_fmt_mp4r_process_atom_amr, 0},
    {SAWB_TYPE /* 'sawb' */, TRUE, video_fmt_mp4r_process_atom_amr, 0},
    {SAWP_TYPE /* 'sawp' */, TRUE, video_fmt_mp4r_process_atom_amr, 0},
    {DAMR_TYPE /* 'damr' */, FALSE, video_fmt_mp4r_process_atom_damr, 0},
    {DAWP_TYPE /* 'dawp' */, FALSE, video_fmt_mp4r_process_atom_dawp, 0},
    {SEVC_TYPE /* 'sevc' */, TRUE, video_fmt_mp4r_process_container, &sevc},
    {EVRC_TYPE /* 'evrc' */, TRUE, video_fmt_mp4r_process_container, &evrc},  /* TEMPORARY support */
    {DEVC_TYPE /* 'devc' */, FALSE, video_fmt_mp4r_parse_atom, &devc},
    {SECB_TYPE /* 'secb' */, TRUE, video_fmt_mp4r_process_container, &secb},
    {DECB_TYPE /* 'decb' */, FALSE, video_fmt_mp4r_parse_atom, &decb},
    {SECW_TYPE /* 'secw' */, TRUE, video_fmt_mp4r_process_container, &secw},
    {DECW_TYPE /* 'decw' */, FALSE, video_fmt_mp4r_parse_atom, &decw},
    {SQCP_TYPE /* 'sqcp' */, TRUE, video_fmt_mp4r_process_container, &sqcp},
    {DQCP_TYPE /* 'dqcp' */, FALSE, video_fmt_mp4r_parse_atom, &dqcp},
    {S263_TYPE /* 's263' */, TRUE, video_fmt_mp4r_process_container, &s263},
    {D263_TYPE /* 'd263' */, FALSE, video_fmt_mp4r_process_atom_d263, 0},
    {BITR_TYPE /* 'bitr' */, FALSE, video_fmt_mp4r_process_atom_bitr, 0},
    {TX3G_TYPE /* 'tx3g' */, TRUE, video_fmt_mp4r_process_atom_text, 0},
    {STPP_TYPE /* 'stpp' */, FALSE, video_fmt_mp4r_process_atom_text, 0},
    {MP4S_TYPE /* 'mp4s' */, TRUE, video_fmt_mp4r_process_container, &mp4s},
    {ESDS_TYPE /* 'esds' */, FALSE, video_fmt_mp4r_process_atom_esds, 0},
    {AVC1_TYPE /* 'avc1' */, TRUE, video_fmt_mp4r_process_atom_avc1, 0},
    {AVC2_TYPE /* 'avc2' */, TRUE, video_fmt_mp4r_process_atom_avc1, 0},
    {AVC3_TYPE /* 'avc3' */, TRUE, video_fmt_mp4r_process_atom_avc1, 0},
    {AVC4_TYPE /* 'avc4' */, TRUE, video_fmt_mp4r_process_atom_avc1, 0},
    {AVCC_TYPE /* 'avcC' */, FALSE, video_fmt_mp4r_process_atom_avcC, 0},
    {MVCC_TYPE /* 'mvcC' */, FALSE, video_fmt_mp4r_process_atom_mvcC, 0},
    {VWID_TYPE /* 'vwid' */, FALSE, video_fmt_mp4r_process_atom_vwid, 0},
    {BTRT_TYPE /* 'btrt' */, FALSE, video_fmt_mp4r_process_atom_btrt, 0},
    {HVC1_TYPE /* 'hvc1' */, TRUE, video_fmt_mp4r_process_atom_hvc1, 0},
    {HEV1_TYPE /* 'hev1' */, TRUE, video_fmt_mp4r_process_atom_hvc1, 0},
    {HVCC_TYPE /* 'hvcC' */, FALSE, video_fmt_mp4r_process_atom_hvcC, 0},
    {STCO_TYPE /* 'stco' */, FALSE, video_fmt_mp4r_parse_atom, &stco},
    {CO64_TYPE /* 'co64' */, FALSE, video_fmt_mp4r_parse_atom, &co64},
    {STSC_TYPE /* 'stsc' */, FALSE, video_fmt_mp4r_parse_atom, &stsc},
    {STSZ_TYPE /* 'stsz' */, FALSE, video_fmt_mp4r_process_atom_stsz, 0},
    {STTS_TYPE /* 'stts' */, FALSE, video_fmt_mp4r_parse_atom, &stts},
    {CTTS_TYPE /* 'ctts' */, FALSE, video_fmt_mp4r_parse_atom, &ctts},
    {STSS_TYPE /* 'stss' */, FALSE, video_fmt_mp4r_parse_atom, &stss},
    {FREE_TYPE /* 'free' */, FALSE, video_fmt_mp4r_process_atom_skip, 0},
    {SKIP_TYPE /* 'skip' */, FALSE, video_fmt_mp4r_process_atom_skip, 0},
    {UDTA_TYPE /* 'udta' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    { DRM_TYPE /* 'drm ' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {VINF_TYPE /* 'vinf' */, FALSE, video_fmt_mp4r_process_atom_udta_child, 0},
    {CVER_TYPE /* 'cver' */, FALSE, video_fmt_mp4r_process_atom_udta_child, 0},
    {RAND_TYPE /* 'rand' */, FALSE, video_fmt_mp4r_process_atom_udta_child, 0},
    {RQMT_TYPE /* 'rqmt' */, FALSE, video_fmt_mp4r_process_atom_udta_child, 0},
    {MIDI_TYPE /* 'midi' */, FALSE, video_fmt_mp4r_process_atom_udta_child, 0},
    {LINK_TYPE /* 'link' */, FALSE, video_fmt_mp4r_process_atom_udta_child, 0},
    {MVEX_TYPE /* 'mvex' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {MEHD_TYPE /* 'mehd' */, FALSE, video_fmt_mp4r_process_atom_mehd, 0},
    {TREX_TYPE /* 'trex' */, FALSE, video_fmt_mp4r_process_atom_trex, 0},
    {MOOF_TYPE /* 'moof' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {MFHD_TYPE /* 'mfhd' */, FALSE, video_fmt_mp4r_parse_atom, &mfhd},
    {TRAF_TYPE /* 'traf' */, TRUE, video_fmt_mp4r_process_atom_empty, 0},
    {TFHD_TYPE /* 'tfhd' */, TRUE, video_fmt_mp4r_process_atom_tfhd, 0},
    {TFDT_TYPE /* 'tfdt' */, TRUE, video_fmt_mp4r_process_atom_tfdt, 0},
    {TRUN_TYPE /* 'trun' */, FALSE, video_fmt_mp4r_process_atom_trun, 0},
    {MDAT_TYPE /* 'mdat' */, FALSE, video_fmt_mp4r_process_atom_skip, 0},
    //Adding new atom type and handling function
    {MP3_TYPE  /* 'mp3 ' */, FALSE, video_fmt_mp4r_process_atom_mp3,0},
    {MSOU_TYPE /* 'ms u'*/ , FALSE, video_fmt_mp4r_process_atom_mp3,0},
    // Since the H263/S263_TYPE are same,so adding s263.
    {H263_TYPE /* 'h263' */, FALSE, video_fmt_mp4r_parse_atom, &s263},
    {AC_3_TYPE /* 'ac-3' */, TRUE, video_fmt_mp4r_process_atom_ec3, 0},
    {EC_3_TYPE /* 'ec-3' */, TRUE, video_fmt_mp4r_process_atom_ec3, 0},
    {DAC3_TYPE /* 'dac3' */, FALSE, video_fmt_mp4r_process_atom_dac3, 0},
    {DEC3_TYPE /* 'dec3' */, FALSE, video_fmt_mp4r_process_atom_dec3, 0},
    {DTSC_TYPE /* 'dtsc' */, TRUE, video_fmt_mp4r_process_atom_dts, 0},
    {DTSH_TYPE /* 'dtsh' */, TRUE, video_fmt_mp4r_process_atom_dts, 0},
    {DTSL_TYPE /* 'dtsl' */, TRUE, video_fmt_mp4r_process_atom_dts, 0},
    {DTSE_TYPE /* 'dtse' */, TRUE, video_fmt_mp4r_process_atom_dts, 0},
    {DDTS_TYPE /* 'ddts' */, FALSE, video_fmt_mp4r_process_atom_ddts, 0}
};
#define VIDEO_FMT_MP4R_ATOM_INFO_TABLE_SIZE \
    (sizeof (video_fmt_mp4r_atom_info_table) \
     / sizeof (video_fmt_mp4r_atom_info_table [0]))


/* The following describes as much of the ISO/IEC 14496-2 (MPEG-4) video
** bitstream format as necessary to decode the video output width and height
** fields.
**
** The format is described in the micro-language defined by the video format
** bitstream services for parsing bitstreams.  See the comments for the
** function "video_fmt_bs_decode" for more details on this micro-language.
*/
static const char video_fmt_mp4r_m4v_bitstream [] =
/* video_signal_type */
"f'vsig'{"
  "v1'vsig_v'"       /* video_signal_type            */
  "?'vsig_v'{"
    "v3'vfmt'"       /* video_format                 */
    "v1'vrng'"       /* video_range                  */
    "v1'cdes'"       /* colour_description           */
    "?'cdes'{"
      "v8'cpri'"     /* colour_primaries             */
      "v8'tchar'"    /* transfer_characteristics     */
      "v8'mcoef'"    /* matrix_coefficients          */
    "}{}"
  "}{}"
"}"

/* next_start_code */
"f'next'{"
  "c1''#0#"          /* zero bit                     */
  "wAo#7#{"
/* Technically, compliant bitstreams should pad with one bits here, but we've
** relaxed the requirement to allow parsing non-compliant bitstreams where the
** pad bits are all "zero" instead of "one".
*/
    "r1''#0##1#"     /* any bit                      */
  "}"
"}"

/* user_data */
"f'user'{"
  "c32'udat_start'#1B2#" /* user_data_start_code     */
  "w=n24#1##0#{"
    "r8''#0##FF#"    /* any byte                     */
  "}"
"}"

/* Video Object Layer */
"f'vol'{"
  "?r32#120##12F#{"  /* video_object_layer_start_code */
    "='short'#0#"    /* short_video_header = 0       */
    "r32'vol_start'#120##12F#"  /* video_object_layer_start_code */
    "v1'rand'"       /* random_accessible_vol        */
    "v8'vo_typ'"     /* video_object_type_indication */
    "v1'is_ol'"      /* is_object_layer_indication   */
    "?'is_ol'{"
      "v4'vol_verid'" /* video_object_layer_verid    */
      "v3'vol_pri'"  /* video_object_layer_priority  */
    "}{"
      "='vol_verid'#1#"
    "}"
    "v4'aratio'"     /* aspect_ratio_info            */
    "?='aratio'#F#{"
      "v8'pwidth'"   /* par_width                    */
      "v8'pheight'"  /* par_height                   */
    "}{}"
    "v1'vol_ctrl'"   /* vol_control_parameters       */
    "?'vol_ctrl'{"
      "v2'chr_fmt'"  /* chroma_format                */
      "v1'low_dly'"  /* low_delay                    */
      "v1'vbv_par'"  /* vbv_parameters               */
      "?'vbv_par'{"
        "v15'fhbr'"  /* first_half_bit_rate          */
        "c1''#1#"      /* marker_bit                   */
        "v15'lhbr'"  /* latter_half_bit_rate         */
        "c1''#1#"      /* marker_bit                   */
        "v15'fhvbs'" /* first_half_vbv_buffer_size   */
        "c1''#1#"      /* marker_bit                   */
        "v3'lhvbs'"  /* latter_half_vbv_buffer_size  */
        "v11'fhvo'"  /* first_half_vbv_occupancy     */
        "c1''#1#"      /* marker_bit                   */
        "v15'lhvo'"  /* latter_half_vbv_occupancy    */
        "c1''#1#"      /* marker_bit                   */
      "}{}"
    "}{}"
    "v2'vol_shape'"  /* video_object_layer_shape     */
    "?&='vol_shape'#3#!='vol_verid'#1#{"
      "v4'vol_shape_ext'" /* video_object_layer_shape_extension */
    "}{}"
    "c1''#1#"          /* marker_bit                   */
    "v16'vop_tinc'"  /* vop_time_increment_resolution */

    /* Calculation of time increment length in bits */
    "='temp''vop_tinc'"
    "='vop_tlen'#0#"
    "w'temp'{"
      "='temp'>'temp'#1#"
      "='vop_tlen'+'vop_tlen'#1#"
    "}"

    "c1''#1#"          /* marker_bit                   */
    "v1'fix_vrate'"  /* fixed_vop_rate               */
    "?'fix_vrate'{"
      "v'vop_tlen''fix_vop_tinc'" /* fixed_vop_time_increment */
    "}{}"
    "?!='vol_shape'#2#{"
      "?='vol_shape'#0#{"
        "c1''#1#"      /* marker_bit                   */
        "v13'vol_width'" /* video_object_layer_width */
        "c1''#1#"      /* marker_bit                   */
        "v13'vol_height'" /* video_object_layer_height */
        "c1''#1#"      /* marker_bit                   */
      "}{}"

/* At this point, we don't care about the rest of the VOL.  Set the special
** "end" marker flag and then generate an exception if parsed further.
*/
      "='end'#1#"
      "!"
    "}{!}"
  "}{"

/* Definitely a short-header file.  So at this point, we can stop parsing this
** part of the VOL.  The rest of the information we need is in a different
** part of the movie file.  Set the special "end" marker flag and then
** generate an exception if parsed further.
*/
    "='end'#1#"
    "!"
  "}"
"}"

/* VisualObject */
"f'vo'{"
  "c32'vo_start'#1B5#" /* visual_object_start_code   */
  "v1'is_vo'"        /* is_visual_object_identifier  */
  "?'is_vo'{"
    "v4'vo_verid'"   /* visual_object_verid          */
    "v3'vo_pri'"     /* visual_object_priority       */
  "}{}"
  "v4'vo_typ'"       /* visual_object_type           */
  "?|='vo_typ'#1#='vo_typ'#2#{"
    "s'vsig'"
  "}{}"
  "s'next'"
  "wn32#1B2#{"
    "s'user'"        /* user_data                    */
  "}"
  "?='vo_typ'#1#{"
    "r32'vo_start'#100##11F#" /* video_object_start_code */
    "s'vol'"         /* VideoObjectLayer             */
  "}{!}"
  "?=n24#1##0#{"
    "s'next'"
  "}"
"}"

/* VisualObjectSequence */
"f'vos'{"
  "d{"
    "c32'vos_start'#1B0#" /* visual_object_sequence_start_code */
    "v8'plev'"       /* profile_and_level_indication */
    "wn32#1B2#{"
      "s'user'"      /* user_data                    */
    "}"
    "s'vo'"          /* VisualObject                 */
  "}=n32#1B1##0#"
  "c32'vos_end'#1B1#" /* visual_object_sequence_end_code */
"}"

/* Assume for how this is a short-header mode bitstream.  We need to clear
** this if later we find out it is not.
*/
"='short'#1#"    /* short_video_header = 1       */

/* Skip any junk before first VOS or VO start code */
"w!|n32#1B0#r32#100##11F#{"
  "r8''#0##FF#"    /* any byte                     */
"}"

/* Main (may consist of a visual object sequence or video object layer) */
"?n32#1B0#{"         /* visual_object_sequence_start_code */
  "s'vos'"
"}{"
"?r32#100##11F#{"    /* video_object_start_code      */
  "r32'vo_start'#100##11F#" /* video_object_start_code */
  "s'vol'"
"}{!}}";

/* The following describes as much of the ISO/IEC 14496-3 (MPEG-4) audio
** bitstream format as necessary to decode the parameters required for the
** video_fmt_mp4r_aac_params_type structure for the audio stream.
**
** The format is described in the micro-language defined by the video format
** bitstream services for parsing bitstreams.  See the comments for the
** function "video_fmt_bs_decode" for more details on this micro-language.
*/
static const char video_fmt_mp4r_m4a_bitstream [] =
/* byte_alignment - ISO/IEC 14496-3:2001(E) */
"f'byte_alignment'{"
  "wAo#7#{"
    "r1''#0##1#"     /* any bit */
  "}"
"}"

/* program_config_element - ISO/IEC 14496-3:2001(E) subpart 4 */
"f'program_config_element'{"
  "v4'element_instance_tag'"
  "v2'object_type'"
  "v4'sampling_frequency_index'"
  "v4'num_front_channel_elements'"
  "v4'num_side_channel_elements'"
  "v4'num_back_channel_elements'"
  "v2'num_lfe_channel_elements'"
  "v3'num_assoc_data_elements'"
  "v4'num_valid_cc_elements'"
  "v1'mono_mixdown_present'"
  "?='mono_mixdown_present'#1#{"
    "v4'mono_mixdown_element_number'"
  "}{}"
  "v1'stereo_mixdown_present'"
  "?='stereo_mixdown_present'#1#{"
    "v4'stereo_mixdown_element_number'"
  "}{}"
  "v1'matrix_mixdown_present'"
  "?='matrix_mixdown_present'#1#{"
    "v4'matrix_mixdown_element_number'"
  "}{}"
  "='i'#0#"
  "wL'i''num_front_channel_elements'{"
    "v1'front_element_is_cpe'"
    "v4'front_element_tag_select'"
    "='i'+'i'#1#"
  "}"
  "='i'#0#"
  "wL'i''num_side_channel_elements'{"
    "v1'side_element_is_cpe'"
    "v4'side_element_tag_select'"
    "='i'+'i'#1#"
  "}"
  "='i'#0#"
  "wL'i''num_back_channel_elements'{"
    "v1'back_element_is_cpe'"
    "v4'back_element_tag_select'"
    "='i'+'i'#1#"
  "}"
  "='i'#0#"
  "wL'i''num_lfe_channel_elements'{"
    "v4'lfe_element_tag_select'"
    "='i'+'i'#1#"
  "}"
  "='i'#0#"
  "wL'i''num_assoc_data_elements'{"
    "v4'assoc_data_element_tag_select'"
    "='i'+'i'#1#"
  "}"
  "='i'#0#"
  "wL'i''num_valid_cc_elements'{"
    "v1'cc_element_is_ind_sw'"
    "v4'valid_cc_element_tag_select'"
    "='i'+'i'#1#"
  "}"
  "s'byte_alignment'"
  "v8'comment_field_bytes'"
  "='i'#0#"
  "wL'i''comment_field_bytes'{"
    "v8'comment_field_data'"
    "='i'+'i'#1#"
  "}"
"}"

/* GASpecificConfig - ISO/IEC 14496-3:2001(E) subpart 4 */
"f'ga_specific_config'{"
  "v1'frame_length_flag'"
  "v1'depends_on_core_coder'"
  "?'depends_on_core_coder'{"
    "v14'core_coder_delay'"
  "}{}"
  "v1'extension_flag'"
  "?!'channel_configuration'{"
    "s'program_config_element'"
  "}{}"
  "?|='audio_object_type'#6#='audio_object_type'#14#{"
    "v3'layer_nr'"
  "}{}"
  "?'extension_flag'{"
    "?='audio_object_type'#16#{"
      "v5'num_of_sub_frame'"
      "v11'layer_length'"
    "}{}"
    "?||='audio_object_type'#11#='audio_object_type'#13#"
      "|='audio_object_type'#14#='audio_object_type'#17#{"
        "v1'aac_section_data_resilience_flag'"
        "v1'aac_scalefactor_data_resilience_flag'"
        "v1'aac_spectral_data_resilience_flag'"
    "}{}"
    "v1'extension_flag_3'"
    "?'extension_flag_3'{"
      /* ISO/IEC 14496-3:2001(E) says "TBD in version 3" here */
    "}{}"
  "}{}"
  "?='audio_object_type'#2#{"
        "v11'sync_word'"
        "}{}"
   "?='sync_word'#2B7#{"
        "v5'ext_audio_object_type'"
        "v1'ps_present_flag'"
    "}{}"
"}"

/* AudioSpecificConfig - ISO/IEC 14496-3:2001(E) */
"v5'audio_object_type'"
"v4'm4a_sampling_freq_index'"
"?='m4a_sampling_freq_index'#F#{"
  "v24'm4a_sampling_freq'"
"}{}"
"v4'channel_configuration'"
"?|||='audio_object_type'#1#='audio_object_type'#2#"
   "|='audio_object_type'#3#='audio_object_type'#4#"
  "|='audio_object_type'#6#='audio_object_type'#7#{"
    "s'ga_specific_config'"
"}{}"
"?='audio_object_type'#8#{"
  "!" /* CelpSpecificConfig - not supported */
"}{}"
"?='audio_object_type'#9#{"
  "!" /* HvxcSpecificConfig - not supported */
"}{}"
"?='audio_object_type'#C#{"
  "!" /* TTSSpecificConfig - not supported */
"}{}"
"?||='audio_object_type'#D#='audio_object_type'#E#"
  "|='audio_object_type'#F#='audio_object_type'#10#{"
    "!" /* StructuredAudioSpecificConfig - not supported */
"}{}"
"?|||='audio_object_type'#11#='audio_object_type'#13#"
   "|='audio_object_type'#14#='audio_object_type'#15#"
  "|='audio_object_type'#16#='audio_object_type'#17#{"
    "s'ga_specific_config'"
"}{}"
"?='audio_object_type'#18#{"
  "!" /* ErrorResilientCelpSpecificConfig - not supported */
"}{}"
"?='audio_object_type'#19#{"
  "!" /* ErrorResilientHvxcSpecificConfig - not supported */
"}{}"
"?|='audio_object_type'#1A#='audio_object_type'#1B#{"
  "!" /* ParametricSpecificConfig - not supported */
"}{}"
"?||||='audio_object_type'#11#='audio_object_type'#13#"
    "|='audio_object_type'#14#='audio_object_type'#15#"
   "||='audio_object_type'#16#='audio_object_type'#17#"
    "|='audio_object_type'#18#='audio_object_type'#19#"
  "|='audio_object_type'#1A#='audio_object_type'#1B#{"
    "v2'ep_config'"
    "?|='ep_config'#2#='ep_config'#3#{"
      "!" /* ErrorProtectionSpecificConfig - not supported */
    "}{}"
    "?|='ep_config'#3#='ep_config'#3#{"
      "v1'direct_mapping'"
      "?!'direct_mapping'{"
        /* ISO/IEC 14496-3:2001(E) says "TBD" here */
      "}{}"
    "}{}"
"}{}"

/* Stop parsing once this point is reached. */
"='end'#1#";

/* The following describes as much of the ISO/IEC 11172-3 (MPEG-1 layer 3)
** audio bitstream format as necessary to decode the parameters required for
** the video_fmt_stream_audio_subtype structure for the audio stream.
**
** The format is described in the micro-language defined by the video format
** bitstream services for parsing bitstreams.  See the comments for the
** function "video_fmt_bs_decode" for more details on this micro-language.
*/
static const char video_fmt_mp4r_mp3_bitstream [] =
/* 2.4.1.3 -- header */
"f'header'{"
  "c12'syncword'#FFF#"
  "v1'mp3_id'"
  "v2'layer'"
  "v1'protection_bit'"
  "v4'bitrate_index'"
  "v2'mp3_sampling_frequency'"
  "v1'padding_bit'"
  "v1'private_bit'"
  "v2'mp3_mode'"
  "v2'mode_extension'"
  "v1'copyright'"
  "v1'original'"
  "v2'emphasis'"
"}"

/* 2.4.1.2 -- audio frame */
"f'frame'{"
  "s'header'"

/* Stop parsing once this point is reached. */
"='end'#1#"

/* The following parts of this structure are not needed */
/*  "s'error_check'" */
/*  "s'audio_data'" */
/*  "s'ancillary_data'" */
"}"

/* 2.4.1.1 -- audio sequence */
"wn12#FFF#{"
  "s'frame'"
"}";

/*===========================================================================

FUNCTION  GET_CH_COUNT

DESCRIPTION
  This function will .return number of channel count based on channel
  location.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
uint8 GET_CH_COUNT(uint16 usChLoc)
{
  uint32 ulChMaskIdx;
  uint8 ucChMapIdx=0;
  uint8 ucChCount = 0;
  for( ulChMaskIdx = 1; ulChMaskIdx < 0xFFFF,
       ucChMapIdx < 9 ;ulChMaskIdx<<=1,ucChMapIdx++)
  {
    if(usChLoc & ulChMaskIdx)
    {
      ucChCount = CHAN_LOC[ucChMapIdx];
    }
  }
  return ucChCount;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_open

DESCRIPTION
  This function opens an existing MP4 file and prepares it for reading.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_open
(
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data,
  uint8                          playingContext,
  video_fmt_type                 eFormatType
)
{
  video_fmt_mp4r_context_type  *context;
  video_fmt_alloc_type        alloc;
  uint32                      test_word = 1;

  video_fmt_file_size         file_Size;
  file_Size.fileSize = 0;

  /* Call the callback to allocate space for the decoder context. */
  alloc.size = (uint32)sizeof (video_fmt_mp4r_context_type);
  callback_ptr (VIDEO_FMT_ALLOC, client_data, &alloc, NULL);

  if ( alloc.ptr )
  {
    context = (video_fmt_mp4r_context_type *) alloc.ptr;
    //Initialized memory to 0.
    memset(context, 0, alloc.size);
  }
  else
  {
    callback_ptr (VIDEO_FMT_FAILURE, client_data, NULL, NULL);
    return;
  }

  /* Determine if we need to byte-swap words to store network byte order
  ** words in host order.
  */
  if (((uint8 *) &test_word) [0] == 1)
  {
      context->byte_swap_needed = TRUE;
  }
  else
  {
      context->byte_swap_needed = FALSE;
  }

  /* set defaults */
  context->file_level_data.audio_only_allowed = TRUE;
  context->file_level_data.video_only_allowed = TRUE;
  context->file_level_data.no_rand_access = FALSE;

  /* Set up initial input buffer state. */
  context->in_buffer_which = 1;  /* will cause immediate switch */
                                           /* to buffer 0 on first read.  */
  context->in_buffer_size = 0;
  context->in_buffer_pos = 0;
  context->abs_pos = 0;

  context->size_retrieve_buffer_which = 1; /* will cause immediate switch */
                                           /* to buffer 0 on first read.  */
  context->size_retrieve_buffer_size = 0;
  context->size_retrieve_buffer_pos = 0;
  context->abs_size_retrieve_pos = 0;

  context->mfra_present = FALSE;
  context->eof_reached = FALSE;
  memset(&context->tfra, 0x0, sizeof(video_fmt_mp4r_sample_table_type));

  callback_ptr( VIDEO_FMT_FILESIZE, client_data,  &file_Size, NULL );
  context->file_size           = file_Size.fileSize;
  context->mfro_present        = FALSE;

  if( context->file_size )
  {
    /* Changing the abs_pos to determine if mfro atom is present.
     If present then this points us to the start of mfro atom */
    context->abs_pos = context->file_size - 16 ;
  }

  /*Store the clients playing context*/
  context->client_playContext = playingContext;
  context->valid_track_count = 0;

  /* Initialize state machine and start processing. */
  context->state = VIDEO_FMT_MP4R_STATE_INIT;
  context->callback_ptr = callback_ptr;
  context->client_data = client_data;

  //If FileSource creator marked input data as DASH complaint then update
  //boolean variable accordingly.
  if(VIDEO_FMT_MP4_DASH == eFormatType)
  {
    context->isDashClip = TRUE;
  }

  video_fmt_mp4r_process (context);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_end

DESCRIPTION
  This function terminates the current MP4 file format session.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_end (void *server_data)
{
  video_fmt_mp4r_context_type     *context;
  video_fmt_status_cb_func_type  callback_ptr;
  void                           *client_data;
  video_fmt_free_type            free;
  video_fmt_stream_info_type    *stream_info;
  int                            j;
  uint32                         i;

  /* Deallocate memory used by the video format services. */
  context = (video_fmt_mp4r_context_type *) server_data;
  callback_ptr = context->callback_ptr;
  client_data = context->client_data;

  /* search for H264 video stream and free all allocated memory */
  for(i=0; i<context->num_streams; i++)
  {
    stream_info = &context->stream_info [i];
    if( (stream_info->type == VIDEO_FMT_STREAM_VIDEO) &&
      (stream_info->subinfo.video.format == VIDEO_FMT_STREAM_VIDEO_H264) )
    {
      video_fmt_h264_dec_info_type *h264_info = &stream_info->dec_specific_info.h264_info;

      /* free picture param sets for avcC */
      if( h264_info->avcc_info && h264_info->avcc_info->num_pic_param &&
         (h264_info->avcc_info->pic_param_set != NULL ) )
      {
        for(j=0; j<h264_info->avcc_info->num_pic_param; j++)
        {
          if(h264_info->avcc_info->pic_param_set[j].len)
          {
            free.ptr = h264_info->avcc_info->pic_param_set[j].data;
            callback_ptr (VIDEO_FMT_FREE, client_data,
                          &free, NULL);
            h264_info->avcc_info->pic_param_set[j].data = NULL;
          }
        }
        free.ptr = h264_info->avcc_info->pic_param_set;
        callback_ptr (VIDEO_FMT_FREE, client_data,
                      &free, NULL);
        h264_info->avcc_info->num_pic_param = 0;
        h264_info->avcc_info->pic_param_set = NULL;
      }

      /* free sequence param sets for avcC */
      if( h264_info->avcc_info && h264_info->avcc_info->num_seq_param &&
         (h264_info->avcc_info->seq_param_set != NULL ) )
      {
        for(j=0; j<h264_info->avcc_info->num_seq_param; j++)
        {
          if(h264_info->avcc_info->seq_param_set[j].len)
          {
            free.ptr = h264_info->avcc_info->seq_param_set[j].data;
            callback_ptr (VIDEO_FMT_FREE, client_data,
                          &free, NULL);
            h264_info->avcc_info->seq_param_set[j].data = NULL;
          }
        }
        free.ptr = h264_info->avcc_info->seq_param_set;
        callback_ptr (VIDEO_FMT_FREE, client_data,
                      &free, NULL);
        h264_info->avcc_info->num_seq_param = 0;
        h264_info->avcc_info->seq_param_set = NULL;
      }

      /* Free the memory allocated at runtime for avcC atom */
      if(h264_info->avcc_alloc.memory_allocated)
      {
        free.ptr = h264_info->avcc_alloc.memory_ptr;
        callback_ptr (VIDEO_FMT_FREE, client_data,
                      &free, NULL);
        h264_info->avcc_alloc.memory_allocated = FALSE;
        h264_info->avcc_alloc.memory_ptr = NULL;
        if(h264_info->avcc_info)
        {
          free.ptr = h264_info->avcc_info;
          callback_ptr (VIDEO_FMT_FREE, client_data,
                        &free, NULL);
        }
        h264_info->avcc_info = NULL;
      }
      //Reset bAvcParseDone to FALSE;
      h264_info->bAvcParseDone = FALSE;

      /* free picture param sets for mvcC */
      if( h264_info->mvcc_info && h264_info->mvcc_info->num_pic_param &&
        (h264_info->mvcc_info->pic_param_set != NULL ) )
      {
        for(j=0; j<h264_info->mvcc_info->num_pic_param; j++)
        {
          if(h264_info->mvcc_info->pic_param_set[j].len)
          {
            free.ptr = h264_info->mvcc_info->pic_param_set[j].data;
            callback_ptr (VIDEO_FMT_FREE, client_data,
                          &free, NULL);
            h264_info->mvcc_info->pic_param_set[j].data = NULL;
          }
        }
        free.ptr = h264_info->mvcc_info->pic_param_set;
        callback_ptr (VIDEO_FMT_FREE, client_data,
                      &free, NULL);
        h264_info->mvcc_info->num_pic_param = 0;
        h264_info->mvcc_info->pic_param_set = NULL;
      }

      /* free sequence param sets for mvcC */
      if( h264_info->mvcc_info && h264_info->mvcc_info->num_seq_param &&
         (h264_info->mvcc_info->seq_param_set != NULL ) )
      {
        for(j=0; j<h264_info->mvcc_info->num_seq_param; j++)
        {
          if(h264_info->mvcc_info->seq_param_set[j].len)
          {
            free.ptr = h264_info->mvcc_info->seq_param_set[j].data;
            callback_ptr (VIDEO_FMT_FREE, client_data,
                          &free, NULL);
            h264_info->mvcc_info->seq_param_set[j].data = NULL;
          }
        }
        free.ptr = h264_info->mvcc_info->seq_param_set;
        callback_ptr (VIDEO_FMT_FREE, client_data,
                      &free, NULL);
        h264_info->mvcc_info->num_seq_param = 0;
        h264_info->mvcc_info->seq_param_set = NULL;
      }

      //check if mvcc was parsed/allocated to free up the memory
      if(h264_info->mvcc_alloc.memory_allocated)
      {
        free.ptr = h264_info->mvcc_alloc.memory_ptr;
        callback_ptr (VIDEO_FMT_FREE, client_data,
                      &free, NULL);
        h264_info->mvcc_alloc.memory_allocated = FALSE;
        h264_info->mvcc_alloc.memory_ptr = NULL;
        if(h264_info->mvcc_info)
        {
          free.ptr = h264_info->mvcc_info;
          callback_ptr (VIDEO_FMT_FREE, client_data,
                        &free, NULL);
        }
        h264_info->mvcc_info = NULL;
      }//if(h264_info->mvcc_alloc.memory_allocated)

      //check if vwid was parsed/allocated to free up the memory
      if(h264_info->vwid_alloc.memory_allocated)
      {
        free.ptr = h264_info->vwid_alloc.memory_ptr;
        callback_ptr (VIDEO_FMT_FREE, client_data,
                      &free, NULL);
        h264_info->vwid_alloc.memory_allocated = FALSE;
        h264_info->vwid_alloc.memory_ptr = NULL;
        if(h264_info->vwid_info)
        {
          int cnt = 0;
          while(cnt < h264_info->vwid_info->num_views)
          {
            if(h264_info->vwid_info->p_view_info[cnt].ref_view_id)
            {
              free.ptr = h264_info->vwid_info->p_view_info[cnt].ref_view_id;
              callback_ptr (VIDEO_FMT_FREE, client_data,
                            &free, NULL);
              h264_info->vwid_info->p_view_info[cnt].ref_view_id = NULL;
            }
            cnt++;
          }
          if(h264_info->vwid_info->num_views)
          {
            free.ptr = h264_info->vwid_info->p_view_info;
            callback_ptr (VIDEO_FMT_FREE, client_data,
                          &free, NULL);
            h264_info->vwid_info->p_view_info = NULL;
          }
          free.ptr = h264_info->vwid_info;
          callback_ptr (VIDEO_FMT_FREE, client_data,
          (video_fmt_status_cb_info_type *) &free, NULL);
        }
        h264_info->vwid_info = NULL;
      }//if(h264_info->vwid_alloc.memory_allocated)
    }//if(H264)
    else if( ( VIDEO_FMT_STREAM_VIDEO == stream_info->type ) &&
      (VIDEO_FMT_STREAM_VIDEO_HEVC == stream_info->subinfo.video.format ) )
    {
      video_fmt_hevc_dec_info_type *pHEVCInfo = \
          &stream_info->dec_specific_info.sHEVCInfo;
      uint32 ulArrIdx = 0;
      uint32 ulNalIdx = 0;

      if( ( NULL != pHEVCInfo->pHVCCInfo) &&
          ( NULL != pHEVCInfo->pHVCCInfo->pArrayNALU ) )
      {
        for(ulArrIdx = 0; ulArrIdx < pHEVCInfo->pHVCCInfo->ucNumOfNALArray; ulArrIdx++)
        {
          for( ulNalIdx = 0;
               ulNalIdx < pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].usNumNALU;
               ulNalIdx++)
          {
            if( ( NULL != pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU ) &&
                ( NULL != pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].data ))
            {
              //free data
              free.ptr = pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].data;
              callback_ptr ( VIDEO_FMT_FREE,
                             client_data,
                             &free,
                             NULL);
              pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].data = NULL;
            }//if(pNALU[ulNalIdx]->data)
          }//for(ulnalIdx)

          // free pNALU
          free.ptr = pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU;
          callback_ptr ( VIDEO_FMT_FREE,
                         client_data,
                         &free,
                        NULL);
          pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU = NULL;
        }//for(ulArrIdx)

        // free pArrayNALU
        free.ptr = pHEVCInfo->pHVCCInfo->pArrayNALU;
        callback_ptr ( VIDEO_FMT_FREE,
                       client_data,
                       &free,
                       NULL);
        pHEVCInfo->pHVCCInfo->pArrayNALU = NULL;

        // free pHVCCInfo
        free.ptr = pHEVCInfo->pHVCCInfo;
        callback_ptr ( VIDEO_FMT_FREE,
                       client_data,
                       &free,
                      NULL);
        pHEVCInfo->pHVCCInfo = NULL;

      }//if(pHVCCInfo)
    }//if(HEVC)
  }//for(num_streams)

  free.ptr = context;
  callback_ptr (VIDEO_FMT_FREE, client_data, &free, NULL);

  /* Stop internal processing. */
  callback_ptr (VIDEO_FMT_DONE, client_data, NULL, NULL);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process

DESCRIPTION
  This function runs the MP4 format encode/decode state machine.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_process (void *server_data)
{
    video_fmt_mp4r_atom_pos_type *atom_stack_top = 0;
    boolean                      exit_loop = FALSE;
    video_fmt_mp4r_context_type  *context;
    uint32                       i;
    uint64                       atom_size = 0;
    uint32                       atom_type = 0;
    uint32                       mfhd_size = 0;
    uint32                       mfhd_type = 0;
    uint32                       sequence_number = 0;

    /* Process format services state machine. */
    context = (video_fmt_mp4r_context_type *) server_data;
    while (!exit_loop)
    {
      switch (context->state)
      {
        case VIDEO_FMT_MP4R_STATE_INIT:
           /* Disable MFRA atom parsing logic.
              In future, if we want to use this MFRA atom based seek logic
              enable this below feature guarded code. */
#ifdef MFRA_ATOM_PARSE
            if((!context->eof_reached) && (context->file_size > 16) &&
               (context->file_size != MAX_FILE_SIZE))
            {
              if(video_fmt_mp4r_check_mfra_box(context))
              {
                break;
              }
            }
#endif
            /* Set up initial input buffer state. */
            context->in_buffer_which = 1;  /* will cause immediate switch */
                                           /* to buffer 0 on first read.  */
            context->in_buffer_size = 0;
            context->in_buffer_pos = 0;
            context->abs_pos = 0;

            context->mdat_present = FALSE;
            context->moov_present = FALSE;
            context->moof_present = FALSE;
            /* Set up initial atom stack state. */
            context->atom_stack_top = 0;

            /* Initialize stream information. */
            context->num_streams = 0;

            /* Initialize other state variables. */
            context->in_sample_description_atom = FALSE;
            context->current_sequence_number = 0;
            context->get_data_src_in_mdat = FALSE;

            /* Start processing. */
            context->state = VIDEO_FMT_MP4R_STATE_DECODE_ATOM_HEADER;
            context->current_track_id = 0;
            context->fragment_present = FALSE;
            context->trex_count = 0;

            exit_loop = TRUE;
            context->cb_info.info.num_streams
                    = context->num_streams;
            context->cb_info.info.streams
                    = context->stream_info;
            context->cb_info.info.file_info
                    = context->file_level_data;
            context->cb_info.info.read_cb
                    = video_fmt_mp4r_read_stream;
            context->cb_info.info.sample_info_cb
                    = video_fmt_mp4r_get_sample_info;
            context->cb_info.info.sync_sample_cb
                    = video_fmt_mp4r_find_sync_sample;
            context->cb_info.info.parse_fragment_cb
                    = video_fmt_mp4r_parse_fragment;
            context->cb_info.info.fragment_size_cb
                    = video_fmt_mp4r_fragment_size;

            context->cb_info.info.fragment_size_peek_cb
                    = video_fmt_mp4r_peek_fragment_size;

            context->cb_info.info.abs_file_offset = 0;
            context->cb_info.info.abs_file_offset_cb
                    = video_fmt_mp4r_abs_file_offset;

            context->cb_info.info.access_point_cb
                    = video_fmt_mp4r_find_access_point;

            context->cb_info.info.server_data = context;
            context->callback_ptr (VIDEO_FMT_INIT,
                                   context->client_data,
                                   &context->cb_info,
                                   video_fmt_mp4r_end);
            context->cb_info.info.largest_frame_size_cb
                    = video_fmt_mp4r_largest_frame_size;


            break;

        case VIDEO_FMT_MP4R_STATE_GET_DATA:
            /* No matter what, exit the state machine loop each time this
            ** state is visited.  This is necessary in order to limit the
            ** amount of processing done each state machine iteration.
            */
            exit_loop = TRUE;

            /* Request more data from the user. */
            context->cb_info.get_data.buffer = context->get_data_dst;
            context->cb_info.get_data.offset = context->get_data_src;
            context->cb_info.get_data.num_bytes = context->get_data_size;
            context->cb_info.get_data.callback_ptr = video_fmt_mp4r_process;
            context->cb_info.get_data.server_data = context;

            /* Check if the (offset + num bytes) crosses max file boundary */
            if((MAX_FILE_SIZE - context->cb_info.get_data.offset ) <
                context->get_data_size)
            {
              MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_ERROR,"process: offset corrupt!");
              video_fmt_mp4r_failure (context);
              return;
            }

            context->callback_ptr (VIDEO_FMT_GET_DATA,
                                   context->client_data,
                                   &context->cb_info,
                                   video_fmt_mp4r_end);

            /* Verify the user gave us a legal number of bytes. */
            if (context->cb_info.get_data.num_bytes
                > context->get_data_size)
            {
              MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_ERROR,
                          "process: get_data overrun @offset %llu",
                          context->get_data_src);
              video_fmt_mp4r_failure (context);
              return;
            }

            /* Advance the internal variables tracking the reading. */
            context->bEndOfData    = context->cb_info.get_data.bEndOfData;
            context->get_data_dst += context->cb_info.get_data.num_bytes;
            context->get_data_src += context->cb_info.get_data.num_bytes;
            context->get_data_size -= context->cb_info.get_data.num_bytes;
            context->get_data_read += context->cb_info.get_data.num_bytes;
            context->get_data_needed
                -= FILESOURCE_MIN (context->get_data_needed,
                        context->cb_info.get_data.num_bytes);

            /* Move to the next state if no bytes were given, or no more bytes
            ** are needed.
            */
            if (!context->cb_info.get_data.num_bytes
                || !context->get_data_needed)
            {
              context->state = context->state_next [0];
              context->state_next [0] = context->state_next [1];
            }
            break;

        case VIDEO_FMT_MP4R_STATE_UPDATE_BUFFER:

        case VIDEO_FMT_MP4R_STATE_UPDATE_SIZE_RETRIEVE_BUFFER:
            if(context->state == VIDEO_FMT_MP4R_STATE_UPDATE_BUFFER)
            {
              /* Advance the input buffer size by the number of bytes
              ** actually read.
              */
              context->in_buffer_size += (uint32)context->get_data_read;
            }
            else
            {
              /* Advance the size retrieve buffer size by the number of bytes
              ** actually read.
              */
              context->size_retrieve_buffer_size += (uint32)context->get_data_read;
            }

            /* If not enough bytes were read, perform end of
            ** file processing.
            */
            if((context->get_data_needed) &&
               (context->state_next[0] == VIDEO_FMT_MP4R_STATE_INIT))
            {
              context->eof_reached = TRUE;
              context->state = context->state_next [0];
              break;
            }
            if (context->get_data_needed)
            {
              /* No matter what, exit the state machine loop if end of file
              ** is reached.
              */
              exit_loop = TRUE;

              /* Update eof flag with bEndOfData flag before checking whether it is actual end of data or not */
              context->expect_eof = context->bEndOfData;

              /*End of file is to be detected in the state VIDEO_FMT_MP4R_STATE_PEEK_FRAGMENT_SIZE,
              which is called at the beginning of file and at fragment boundaries.This will detect
              end of file in fragmented files. For non-fragmented files,end of file is detected by
              getsampleinfo returning 0 where it detects by the number of frames in main fragment.
              */
              if(context->expect_eof &&
                (context->state_next[0] == VIDEO_FMT_MP4R_STATE_PEEK_FRAGMENT_SIZE ||
                 context->state_next[0] == VIDEO_FMT_MP4R_STATE_PROCESS_FRAGMENT))
              {
                /* Construct data for movie information callback. */
                context->cb_info.info.num_streams
                    = context->num_streams;
                context->cb_info.info.streams
                    = context->stream_info;
                context->cb_info.info.file_info
                    = context->file_level_data;
                context->cb_info.info.read_cb
                    = video_fmt_mp4r_read_stream;
                context->cb_info.info.sample_info_cb
                    = video_fmt_mp4r_get_sample_info;
                context->cb_info.info.sync_sample_cb
                    = video_fmt_mp4r_find_sync_sample;
                context->cb_info.info.largest_frame_size_cb
                    = video_fmt_mp4r_largest_frame_size;

                context->cb_info.info.parse_fragment_cb
                    = video_fmt_mp4r_parse_fragment;

                context->cb_info.info.fragment_size_cb
                    = video_fmt_mp4r_fragment_size;
                context->cb_info.info.fragment_size_peek_cb
                    = video_fmt_mp4r_peek_fragment_size;
                context->cb_info.info.abs_file_offset_cb
                    = video_fmt_mp4r_abs_file_offset;
                context->cb_info.info.access_point_cb
                    = video_fmt_mp4r_find_access_point;

                context->cb_info.info.server_data = context;
                context->callback_ptr (VIDEO_FMT_INFO,
                    context->client_data,
                    &context->cb_info,
                    video_fmt_mp4r_end);
                context->state = VIDEO_FMT_MP4R_STATE_READY;
                for (i = 0; i < context->num_streams; ++i)
                {
                  context->stream_state [i].state
                      = VIDEO_FMT_MP4R_STREAM_STATE_READY;
                }
              }
              else
              {

                /* Construct data for movie information callback. */
                context->cb_info.info.num_streams
                    = context->num_streams;
                context->cb_info.info.streams
                    = context->stream_info;
                context->cb_info.info.file_info
                    = context->file_level_data;
                context->cb_info.info.read_cb
                    = video_fmt_mp4r_read_stream;
                context->cb_info.info.sample_info_cb
                    = video_fmt_mp4r_get_sample_info;
                context->cb_info.info.sync_sample_cb
                    = video_fmt_mp4r_find_sync_sample;
                context->cb_info.info.largest_frame_size_cb
                    = video_fmt_mp4r_largest_frame_size;

                context->cb_info.info.parse_fragment_cb
                    = video_fmt_mp4r_parse_fragment;

                context->cb_info.info.fragment_size_cb
                    = video_fmt_mp4r_fragment_size;
                context->cb_info.info.fragment_size_peek_cb
                    = video_fmt_mp4r_peek_fragment_size;
                context->cb_info.info.abs_file_offset_cb
                    = video_fmt_mp4r_abs_file_offset;
                context->cb_info.info.access_point_cb
                    = video_fmt_mp4r_find_access_point;

                context->cb_info.info.server_data = context;

                context->callback_ptr (VIDEO_FMT_DATA_INCOMPLETE,
                                           context->client_data,
                                           &context->cb_info,
                                           video_fmt_mp4r_end);
                /* Put state machine in the state where stream reads are
                ** now allowed.
                */
                context->state = VIDEO_FMT_MP4R_STATE_READY;
                for (i = 0; i < context->num_streams; ++i)
                {
                  context->stream_state [i].state
                      = VIDEO_FMT_MP4R_STREAM_STATE_READY;
                }
              }
            }

            /* Otherwise, move to the next state. */
            else
            {
              context->expect_eof = FALSE;
              context->state = context->state_next [0];
            }
            break;

        case VIDEO_FMT_MP4R_STATE_DECODE_ATOM_HEADER:
            if(1 != context->cur_atom_size ||
               MDAT_TYPE != context->cur_atom_type)
            {
              /* Read in enough data to decode atom type. */
              if (!video_fmt_mp4r_read_buffer (context, 8, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
              {
                break;
              }

              /* Push atom onto stack. */
              ++context->atom_stack_top;
              if (context->atom_stack_top >= VIDEO_FMT_MP4R_ATOM_STACK_DEPTH)
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                            "process: MP4 file atom stack overflow @offset %llu",
                            context->abs_pos);
                video_fmt_mp4r_failure (context);
                return;
              }
              atom_stack_top = &context->atom_stack
                  [context->atom_stack_top - 1];
              if (context->atom_stack_top == 1) /* only atom on stack? */
              {
                 atom_stack_top->atom.parent = 0;
              }
              else
              {
                atom_stack_top->atom.parent = context->atom_stack
                    [context->atom_stack_top - 2].atom.offset;
              }

              /* Copy over atom size and type. */
              video_fmt_mp4r_consume_data
                      (context,
                       (uint8 *) &atom_stack_top->atom.size,
                       FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                       VIDEO_FMT_MP4R_IN_BUFFER);

              video_fmt_mp4r_consume_data
                      (context,
                       (uint8 *) &atom_stack_top->atom.type,
                       FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                       VIDEO_FMT_MP4R_IN_BUFFER);

              /* Check whether child atom size is greater than parent atom
                 size. */
              if ((context->atom_stack_top > 1) &&
                  (atom_stack_top->atom.size >
                   context->atom_stack[context->atom_stack_top - 2].atom.size))
              {
                atom_stack_top->atom.size = 0;
                context->callback_ptr (VIDEO_FMT_FAILURE,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_mp4r_end);
                exit_loop = TRUE;
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                            "process: MP4 file atom corrupted @offset %llu",
                            context->abs_pos - DEFAULT_ATOM_SIZE);
                break;
              }

              /* Store current atom size information. This is used in sample table atoms to
                 check whether atom size and number of entries are matching or not  */
              context->cur_atom_size = atom_stack_top->atom.size;
              context->cur_atom_type = atom_stack_top->atom.type;
            }
            else
            {
              /* At buffer boundaries, Parser will not execute above if condition, so updating
                 atom_stack_top variable with top element. */
              atom_stack_top = &context->atom_stack
                                [context->atom_stack_top - 1];
            }

            /* If atom size is 1 byte and atom type is mdat, it is version 1 mdat type*/
            if(atom_stack_top->atom.size == 1 && atom_stack_top->atom.type == MDAT_TYPE)
            {
              /* Read 8 more bytes of data to get the mdat size. */
              if (!video_fmt_mp4r_read_buffer (context, 8, TRUE, VIDEO_FMT_MP4R_IN_BUFFER))
              {
                break;
              }
              /* Read upper 4 bytes which should contain ZERO only */
              video_fmt_mp4r_consume_data
                                    (context,
                                     (uint8 *) &atom_stack_top->atom.size,
                                     FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                                     VIDEO_FMT_MP4R_IN_BUFFER);
              if(atom_stack_top->atom.size)
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                            "mdat size should be less than 4GB");
                video_fmt_mp4r_failure (context);
                return;
              }
              /* Read lower 4 bytes which contains actual mdat size */
              video_fmt_mp4r_consume_data
                                    (context,
                                     (uint8 *) &atom_stack_top->atom.size,
                                     FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                                     VIDEO_FMT_MP4R_IN_BUFFER);

              /* Subtract 8bytes which are used to store atom size */
              atom_stack_top->atom.size -= 8;
              context->cur_atom_size = atom_stack_top->atom.size;
              context->cur_atom_type = atom_stack_top->atom.type;
            }

            if (atom_stack_top->atom.size < 8)
            {
              /* Reset current atom size field, this is to make sure that atom size is
                 not equal to 1.*/
              context->cur_atom_size = 0;

              /* If atom size is less than 8 bytes and atom type is not MDAT,
                 then revert the 4 bytes read for atom type which will be again read as atom size */
              if(atom_stack_top->atom.type != MDAT_TYPE)
              {
                context->in_buffer_pos -= 4;
                context->abs_pos -= 4;
                context->in_buffer_size += 4;
              }

              /*If this is a global scope atom and we have the necessary data
                then stop further parsing*/
              if(!atom_stack_top->atom.parent &&
                  (context->moov_present || context->moof_present))
              {
                /* Construct data for movie information callback. */
                context->cb_info.info.num_streams
                    = context->num_streams;
                context->cb_info.info.streams
                    = context->stream_info;
                context->cb_info.info.file_info
                    = context->file_level_data;
                context->cb_info.info.read_cb
                    = video_fmt_mp4r_read_stream;
                context->cb_info.info.sample_info_cb
                    = video_fmt_mp4r_get_sample_info;
                context->cb_info.info.sync_sample_cb
                    = video_fmt_mp4r_find_sync_sample;
                context->cb_info.info.parse_fragment_cb
                    = video_fmt_mp4r_parse_fragment;
                context->cb_info.info.fragment_size_cb
                    = video_fmt_mp4r_fragment_size;
                context->cb_info.info.fragment_size_peek_cb
                    = video_fmt_mp4r_peek_fragment_size;
                context->cb_info.info.abs_file_offset_cb
                    = video_fmt_mp4r_abs_file_offset;
                context->cb_info.info.access_point_cb
                    = video_fmt_mp4r_find_access_point;
                context->cb_info.info.server_data = context;

                context->callback_ptr (VIDEO_FMT_INFO,
                                       context->client_data,
                                       &context->cb_info,
                                       video_fmt_mp4r_end);

                /* Put state machine in the state where stream reads are
                ** now allowed.
                */
                context->state = VIDEO_FMT_MP4R_STATE_READY;
                for (i = 0; i < context->num_streams; ++i)
                {
                  context->stream_state [i].state
                      = VIDEO_FMT_MP4R_STREAM_STATE_READY;
                }
              }
              else
              {
                video_fmt_mp4r_finish_atom (context);
              }
              break;
            }

            /* Don't count atom header in atom contents size. */
            atom_stack_top->atom.size -= FOURCC_SIGNATURE_BYTES * 2;

            /* If the type matches the user extension type, read in extended
            ** type code.
            */
            if (atom_stack_top->atom.type == 0x75756964) /* 'uuid' */
            {
              atom_stack_top->atom.user = TRUE;
              context->state
                  = VIDEO_FMT_MP4R_STATE_DECODE_EXTENDED_ATOM_HEADER;
            }

            /* Otherwise, there is no extended type code. */
            else
            {
              atom_stack_top->atom.user = FALSE;

              /* Decode the atom. */
              video_fmt_mp4r_decode_atom (context);
            }
            break;

        case VIDEO_FMT_MP4R_STATE_RETRIEVE_FRAGMENT_SIZE:
            if(!context->fragment_size_found)
            {
              if((1 != context->cur_atom_size || MDAT_TYPE != context->cur_atom_type) &&
                 MOOF_TYPE != context->cur_atom_type)
              {
                /* Read in enough data to decode atom type. */
                if (!video_fmt_mp4r_read_buffer (context, 8, FALSE, VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER))
                {
                  break;
                }

                /* Copy over atom size and type.  If size doesn't cover atom type,
                ** don't decode rest of atom.
                */
                video_fmt_mp4r_atom_read_size_type(context, (uint8 *) &atom_size,
                                                   (uint8 *) &atom_type);

                /* If atom size or atom type are ZERO, we has to return failure irrespective of
                   operation mode. As per standard, ZERO size atoms are not supported even in
                   progressive download or pseudo streaming scenarios */
                if (!atom_size || !atom_type)
                {
                  exit_loop = TRUE;
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                               "atom_size %llu, atom_type %lu @ offset %llu",
                               atom_size, atom_type, context->abs_pos);
                  video_fmt_mp4r_failure(context);
                  break;
                }

                /* Store atom size and type information */
                context->cur_atom_size = atom_size;
                context->cur_atom_type = atom_type;
              }
              else
              {
                atom_type = context->cur_atom_type;
                atom_size = context->cur_atom_size;
              }

              /* For version1 mdat atoms, size usually kept as 1 byte and actual size will be
                 available after mdat. So, parser is using additional 8 bytes to read mdat size*/
              if(1 == atom_size && MDAT_TYPE == atom_type)
              {
                /* Read 8 more bytes of data to get the mdat size. */
                if (!video_fmt_mp4r_read_buffer (context, 4, TRUE, VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER))
                {
                  break;
                }
                /* Read upper 4 bytes which should contain ZERO only */
                video_fmt_mp4r_consume_data
                                      (context,
                                       (uint8 *) &atom_size,
                                       FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                                       VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);
                if(atom_size)
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                              "mdat size should be less than 4GB");
                  video_fmt_mp4r_failure (context);
                  return;
                }
                /* Read lower 4 bytes which contains actual mdat size */
                video_fmt_mp4r_consume_data
                               (context,
                                (uint8 *) &atom_size,
                                FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                                VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);

                /* Subtract 8bytes which are used to store atom size */
                atom_size -= 8;
                context->cur_atom_size = atom_size;
              }

              if(context->fragment_requested == 0 && atom_type == MOOV_TYPE)
              {
                context->moov_present = TRUE;
                context->fragment_size += atom_size;
                /*Offset for atoms that appear before MOOV*/
                context->fragment_size = FILESOURCE_MAX(context->fragment_size,
                                             context->abs_size_retrieve_pos +
                                             atom_size - (FOURCC_SIGNATURE_BYTES * 2));
              }
              else if(atom_type == MOOF_TYPE)
              {
                //find the sequence number of the fragment.
                /* Read in enough data to decode atom type. */
                if (!video_fmt_mp4r_read_buffer (context, 16, FALSE, VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER))
                {
                  break;
                }

                video_fmt_mp4r_atom_read_size_type(context,
                    (uint8 *) &mfhd_size, (uint8 *) &mfhd_type);

                /* If mfhd size or mfhd type are ZERO, we has to return failure irrespective of
                   operation mode. As per standard, ZERO size atoms are not supported even in
                   progressive download or pseudo streaming scenarios. mfhd size should be less
                   than moof size (atom_size contains this value) */
                if (!mfhd_size || !mfhd_type || (mfhd_type != MFHD_TYPE) || mfhd_size >= atom_size)
                {
                  exit_loop = TRUE;
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                               "mfhd_size %lu, mfhd_type %lu @ offset %llu",
                               mfhd_size, mfhd_type, context->abs_pos);
                  video_fmt_mp4r_failure(context);
                  break;
                }

                video_fmt_mp4r_skip_data (context, FOURCC_SIGNATURE_BYTES,
                                          VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);

                video_fmt_mp4r_consume_data
                    (context,
                    (uint8 *) &sequence_number,
                    FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                    VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);
                if(sequence_number == context->fragment_requested ||
                   (TRUE == context->isDashClip && sequence_number > context->fragment_requested))
                {
                  context->fragment_available = sequence_number;
                  context->fragment_size = atom_size;
                }
                /* Added an error check if the sequence numbers of each
                ** movie fragment in a file does't increase in the order
                ** in which they occur.
                */
                else if(sequence_number > context->fragment_requested)
                {
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                              "seq num %lu > frag requested %lu @ offset %llu",
                               sequence_number, context->fragment_requested,
                               context->abs_pos);
                  video_fmt_mp4r_failure (context);
                  return;
                }
                //reduce the atom_size by total size of mfhd atom as its already consumed.
                atom_size = atom_size - sizeof(uint32) * 4;

                /* Reset variable to avoid any further problems. */
                context->cur_atom_size = 0;
                context->cur_atom_type = 0;
              }
              else if(atom_type == MDAT_TYPE)
              {
                context->mdat_size = atom_size;
                if(context->fragment_requested == 0)
                { //Main fragment..
                  context->mdat_present = TRUE;
                  if(context->moov_present)
                  {
                    context->fragment_size_found = TRUE;
                  }
                }
                else if(context->fragment_size)
                {
                  context->fragment_size_found = TRUE;
                  context->fragment_size += atom_size;
                }
              }

              /* Don't count atom header in atom contents size. If atom size is less than 8 bytes
                 then we don't have any data to skip in the current atom. So parser will read data
                 after 8 bytes which are used for atom size and atom type */
              if(atom_size >= 8)
              {
                 atom_size = atom_size - FOURCC_SIGNATURE_BYTES * 2;

                 video_fmt_mp4r_skip_data (context, atom_size,
                                           VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);
              }
              else
              {
                /* Reset current atom size which will allow parser to parse next atom */
                context->cur_atom_size = 0;
              }
            }
            else
            {
              if(!context->moov_present && (context->fragment_requested == 0))
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                            "VIDEO_FMT_RETRIEVE_FRAGMENT_SIZE "
                            "[Main Fragment]: failed..!!");
              }
              context->moov_present = FALSE;
              context->mdat_present = FALSE;

              exit_loop = TRUE;

              /* Construct data for movie information callback. */
              context->cb_info.info.num_streams
                  = context->num_streams;
              context->cb_info.info.streams
                  = context->stream_info;
              context->cb_info.info.file_info
                  = context->file_level_data;
              context->cb_info.info.read_cb
                  = video_fmt_mp4r_read_stream;
              context->cb_info.info.sample_info_cb
                  = video_fmt_mp4r_get_sample_info;
              context->cb_info.info.sync_sample_cb
                  = video_fmt_mp4r_find_sync_sample;
              context->cb_info.info.parse_fragment_cb
                  = video_fmt_mp4r_parse_fragment;
              context->cb_info.info.fragment_size_cb
                  = video_fmt_mp4r_fragment_size;
              context->cb_info.info.fragment_size_peek_cb
                  = video_fmt_mp4r_peek_fragment_size;
              context->cb_info.info.largest_frame_size_cb
                  = video_fmt_mp4r_largest_frame_size;
              context->cb_info.info.abs_file_offset_cb
                  = video_fmt_mp4r_abs_file_offset;
                  context->cb_info.info.access_point_cb
                      = video_fmt_mp4r_find_access_point;

              context->cb_info.info.server_data = context;
              context->callback_ptr (VIDEO_FMT_FRAGMENT_SIZE,
                                     context->client_data,
                                     &context->cb_info,
                                     video_fmt_mp4r_end);

              /* Put state machine in the state where stream reads are
              ** now allowed.
              */
              context->state = VIDEO_FMT_MP4R_STATE_READY;
              for (i = 0; i < context->num_streams; ++i)
              {
                context->stream_state [i].state
                    = VIDEO_FMT_MP4R_STREAM_STATE_READY;
              }
            }
            break;

        case VIDEO_FMT_MP4R_STATE_DECODE_EXTENDED_ATOM_HEADER:
            /* Read in enough data to decode extended atom type. */
            if (!video_fmt_mp4r_read_buffer (context, 16, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
              break;
            }

            /* Copy over extended atom type. */
            atom_stack_top = &context->atom_stack
                [context->atom_stack_top - 1];
            video_fmt_mp4r_consume_data
                (context,
                 (uint8 *) &atom_stack_top->atom.type,
                 FOURCC_SIGNATURE_BYTES,
                 context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);
            video_fmt_mp4r_consume_data
                (context,
                 (uint8 *) &atom_stack_top->atom.uuid [0],
                 (uint32)sizeof (atom_stack_top->atom.uuid), FALSE,
                 VIDEO_FMT_MP4R_IN_BUFFER);

            /* Don't count atom header in atom contents size. */
            atom_stack_top->atom.size -= FOURCC_SIGNATURE_BYTES
                          + (int)sizeof (atom_stack_top->atom.uuid);

            /* Decode the atom. */
            video_fmt_mp4r_decode_atom (context);
            break;

        case VIDEO_FMT_MP4R_STATE_PROCESS_ATOM:
            /* If processing atom children, move to the decode atom header
            ** state if there is more data, or move to the children_done
            ** substate otherwise.
            */
            atom_stack_top = &context->atom_stack
                [context->atom_stack_top - 1];
            if (atom_stack_top->process_substate
                == VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN)
            {
              if (context->abs_pos - atom_stack_top->atom.offset
                  < atom_stack_top->atom.size)
              {
                exit_loop = FALSE;
                context->state = VIDEO_FMT_MP4R_STATE_DECODE_ATOM_HEADER;
                break;
              }
              else
              {
                atom_stack_top->process_substate
                    = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE;
                if(atom_stack_top->atom.type == MOOV_TYPE)
                  context->moov_present = TRUE;
                else if(atom_stack_top->atom.type == MOOF_TYPE)
                  context->moof_present = TRUE;
              }
            }

            /* Run atom processing function. */
            if(atom_stack_top->atom_process_func)
              exit_loop = atom_stack_top->atom_process_func
                (context, atom_stack_top->atom_struct);
            else
            {
              exit_loop = TRUE;
              context->callback_ptr (VIDEO_FMT_FAILURE,
                                     context->client_data,
                                     &context->cb_info,
                                     video_fmt_mp4r_end);
            }
            break;

        case VIDEO_FMT_MP4R_STATE_READY:
            /* Idle state - nothing to do. */
            exit_loop = TRUE;
            break;

        case VIDEO_FMT_MP4R_STATE_PROCESS_FRAGMENT:

            if(video_fmt_mp4r_check_end_of_file(context))
            {
              break;
            }

            exit_loop = TRUE;
            /* Inform the client that parsing is complete for this fragment. */

            /* Construct data for movie information callback. */
            context->cb_info.info.num_streams
                = context->num_streams;
            context->cb_info.info.streams
                = context->stream_info;
            context->cb_info.info.file_info
                = context->file_level_data;
            context->cb_info.info.read_cb
                = video_fmt_mp4r_read_stream;
            context->cb_info.info.sample_info_cb
                = video_fmt_mp4r_get_sample_info;
            context->cb_info.info.sync_sample_cb
                = video_fmt_mp4r_find_sync_sample;
            context->cb_info.info.parse_fragment_cb
                = video_fmt_mp4r_parse_fragment;
            context->cb_info.info.largest_frame_size_cb
                = video_fmt_mp4r_largest_frame_size;
            context->cb_info.info.fragment_size_cb
                    = video_fmt_mp4r_fragment_size;
            context->cb_info.info.fragment_size_peek_cb
                    = video_fmt_mp4r_peek_fragment_size;
            context->cb_info.info.access_point_cb
                    = video_fmt_mp4r_find_access_point;

            context->cb_info.info.server_data = context;
            if((context->fragment_present == TRUE))
            context->callback_ptr (VIDEO_FMT_FRAGMENT,
                                   context->client_data,
                                   &context->cb_info,
                                   video_fmt_mp4r_end);

            else
              context->callback_ptr (VIDEO_FMT_INFO,
                                   context->client_data,
                                   &context->cb_info,
                                   video_fmt_mp4r_end);

            /* Put state machine in the state where stream reads are
            ** now allowed.
            */
            context->state = VIDEO_FMT_MP4R_STATE_READY;
            for (i = 0; i < context->num_streams; ++i)
            {
              context->stream_state [i].state
                  = VIDEO_FMT_MP4R_STREAM_STATE_READY;
            }
            break;

        case VIDEO_FMT_MP4R_STATE_PEEK_FRAGMENT_SIZE:
            if(!context->fragment_size_found)
            {
              /* If we have already parsed, parent atom field, then no need to parse one more time.*/
              if((1 != context->cur_atom_size || MDAT_TYPE != context->cur_atom_type) &&
                 MOOF_TYPE != context->cur_atom_type)
              {
                if(context->fragment_requested==0)
                {
                  context->expect_eof = FALSE;
                }
                else
                {
                  context->expect_eof = TRUE;
                }
                if (!video_fmt_mp4r_read_buffer (context, 8, context->expect_eof, VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER))
                {
                  break;
                }
                context->expect_eof = FALSE;

                /* Copy over atom size and type.  If size doesn't cover atom type,
                ** don't decode rest of atom.
                */
                video_fmt_mp4r_atom_read_size_type(context, (uint8 *) &atom_size,
                                                   (uint8 *) &atom_type);

                if (!atom_size || !atom_type)
                {
                  exit_loop = TRUE;
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                               "atom_size %llu, atom_type %lu @ offset %llu",
                               atom_size, atom_type, context->abs_pos);
                  video_fmt_mp4r_failure(context);
                  break;
                }

                /* Store atom size and type info in context. We use this info to check whether to parse
                   buffer for atom size and type or to skip that section to parse mdat atom size field */
                context->cur_atom_size = atom_size;
                context->cur_atom_type = atom_type;
              }
              else
              {
                 atom_type = context->cur_atom_type;
                 atom_size = context->cur_atom_size;
              }

              /* For version1 mdat atoms, size usually kept as 1 byte and actual size will be
                 available after mdat. So, parser is using additional 8 bytes to read mdat size*/
              if(1 == atom_size && MDAT_TYPE == atom_type)
              {
                /* Read 8 more bytes of data to get the mdat size. */
                if (!video_fmt_mp4r_read_buffer (context, 4, TRUE, VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER))
                {
                  break;
                }
                /* Read upper 4 bytes which should contain ZERO only */
                video_fmt_mp4r_consume_data
                                   (context,
                                    (uint8 *) &atom_size,
                                    FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                                    VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);
                if(atom_size)
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                              "mdat size should be less than 4GB");
                  video_fmt_mp4r_failure (context);
                  return;
                }
                /* Read lower 4 bytes which contains actual mdat size */
                video_fmt_mp4r_consume_data
                                   (context,
                                    (uint8 *) &atom_size,
                                    FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                                    VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);

                /* Subtract 8bytes which are used to store atom size */
                atom_size -= 8;

                context->cur_atom_size = atom_size;
              }

              if(context->fragment_requested == 0 && atom_type == MOOV_TYPE)
              {
                context->moov_present = TRUE;
                context->fragment_size = atom_size;
                context->fragment_size_found = TRUE;
                /*Offset for atoms that appear before MOOV*/
                context->fragment_offset =
                context->abs_size_retrieve_pos - (FOURCC_SIGNATURE_BYTES * 2);
              }
              else if(atom_type == MOOF_TYPE)
              {
                //find the sequence number of the fragment.
                /* Read in enough data to decode atom type. */
                if (!video_fmt_mp4r_read_buffer (context, 16, FALSE, VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER))
                {
                  break;
                }

                video_fmt_mp4r_atom_read_size_type(context, (uint8 *) &mfhd_size,
                                                   (uint8 *) &mfhd_type);

                /* If mfhd size or mfhd type are ZERO, we has to return failure irrespective of
                   operation mode. As per standard, ZERO size atoms are not supported even in
                   progressive download or pseudo streaming scenarios. mfhd size should be less
                   than moof size (atom_size contains this value) */
                if (!mfhd_size || !mfhd_type || (mfhd_type != MFHD_TYPE) || mfhd_size >= atom_size)
                {
                  exit_loop = TRUE;
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                              "mfhd_size %lu, mfhd_type %lu @ offset %llu",
                              mfhd_size, mfhd_type, context->abs_pos);
                  video_fmt_mp4r_failure(context);
                  break;
                }

                video_fmt_mp4r_skip_data (context, FOURCC_SIGNATURE_BYTES,
                                          VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);

                video_fmt_mp4r_consume_data
                    (context,
                    (uint8 *) &sequence_number,
                    FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                    VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                             " videofmt sequence_number (%lu)"
                             " context->fragment_requested  (%lu)",
                             sequence_number, context->fragment_requested);
                if(sequence_number == context->fragment_requested ||
                  (TRUE == context->isDashClip && sequence_number > context->fragment_requested))
                {
                  context->fragment_available = sequence_number;
                  context->fragment_size = atom_size;
                  context->fragment_offset =
                  context->abs_size_retrieve_pos - (FOURCC_SIGNATURE_BYTES * 4);
                  context->fragment_size_found = TRUE;
                }
                /* Added an error check if the sequence numbers of each
                ** movie fragment in a file does't increase in increments of 1.
                */
                else if(sequence_number > context->fragment_requested)
                {
                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
                              "seq num %lu > frag requested %lu @ offset %llu",
                               sequence_number, context->fragment_requested,
                               context->abs_pos);
                  video_fmt_mp4r_failure (context);
                  return;
                }
                //reduce the atom_size by total size of mfhd atom as its already consumed.
                atom_size = atom_size - sizeof(uint32) * 4;

                /* Reset variable to avoid any further problems. */
                context->cur_atom_type = 0;
              }

              /* Don't count atom header in atom contents size. If atom size is less than 8 bytes
                 then we don't have any data to skip in the current atom. So parser will read data
                 after 8 bytes which are used for atom size and atom type */
              if(atom_size > 8)
              {
                atom_size = atom_size - FOURCC_SIGNATURE_BYTES * 2;

                video_fmt_mp4r_skip_data (context, atom_size,
                                          VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);
              }
              else
              {
                /* Reset current atom size field, which will allow videofmt to parse next atom */
                context->cur_atom_size = 0;
              }
            }
            else
            {
              if(!context->moov_present && (context->fragment_requested == 0))
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                            "VIDEO_FMT_RETRIEVE_FRAGMENT_SIZE"
                            "[Main Fragment]: failed..!!");
              }
              context->moov_present = FALSE;
              context->mdat_present = FALSE;

              exit_loop = TRUE;

              /* Construct data for movie information callback. */
              context->cb_info.info.num_streams
                  = context->num_streams;
              context->cb_info.info.streams
                  = context->stream_info;
              context->cb_info.info.file_info
                  = context->file_level_data;
              context->cb_info.info.read_cb
                  = video_fmt_mp4r_read_stream;
              context->cb_info.info.sample_info_cb
                  = video_fmt_mp4r_get_sample_info;
              context->cb_info.info.sync_sample_cb
                  = video_fmt_mp4r_find_sync_sample;
              context->cb_info.info.parse_fragment_cb
                  = video_fmt_mp4r_parse_fragment;
              context->cb_info.info.fragment_size_cb
                  = video_fmt_mp4r_fragment_size;
              context->cb_info.info.fragment_size_peek_cb
                  = video_fmt_mp4r_peek_fragment_size;
              context->cb_info.info.largest_frame_size_cb
                  = video_fmt_mp4r_largest_frame_size;
              context->cb_info.info.abs_file_offset_cb
                  = video_fmt_mp4r_abs_file_offset;
              context->cb_info.info.access_point_cb
                  = video_fmt_mp4r_find_access_point;

              context->cb_info.info.server_data = context;
              context->callback_ptr (VIDEO_FMT_FRAGMENT_PEEK,
                                     context->client_data,
                                     &context->cb_info,
                                     video_fmt_mp4r_end);

              /* Put state machine in the state where stream reads are
              ** now allowed.
              */
              context->state = VIDEO_FMT_MP4R_STATE_READY;
              for (i = 0; i < context->num_streams; ++i)
              {
                context->stream_state [i].state
                    = VIDEO_FMT_MP4R_STREAM_STATE_READY;
              }
            }
            break;
        case VIDEO_FMT_MP4R_STATE_INVALID:
        default:
            /* We should hopefully never reach here.  However, if we do, clean
            ** up as best we can, and exit.
            */
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "process: Invalid format services state");
            video_fmt_mp4r_failure (context);
            return;
      }
    }
 }

/*===========================================================================

FUNCTION  video_fmt_mp4r_parse_fragment

DESCRIPTION
  This function is given to the client as a callback.  It is called in order
  to request parsing of the fragment if it exists.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_parse_fragment
(
  void                        *server_data
)
{
    video_fmt_mp4r_context_type  *context;

    /* Return an error if the format services is not currently waiting in the
    ** proper state.
    */
    context = (video_fmt_mp4r_context_type *) server_data;

    context->state = VIDEO_FMT_MP4R_STATE_DECODE_ATOM_HEADER;
    /* Set up initial input buffer state. */
    context->in_buffer_which = 1;  /* will cause immediate switch */
                                   /* to buffer 0 on first read.  */
    context->in_buffer_size = 0;
    context->in_buffer_pos = 0;

    video_fmt_mp4r_process (server_data);
}


/*===========================================================================

FUNCTION  video_fmt_mp4r_fragment_size

DESCRIPTION
  This function is given to the client as a callback.  It is called in order
  to request the size of the fragment (moov + mdat size or moof + mdat size)

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_fragment_size
(
  void                        *server_data,
  uint32                      fragment_number
)
{
    video_fmt_mp4r_context_type  *context;

    /* Return an error if the format services is not currently waiting in the
    ** proper state.
    */
    context = (video_fmt_mp4r_context_type *) server_data;

    context->size_retrieve_buffer_which = 1;  /* will cause immediate switch */
                                           /* to buffer 0 on first read.  */
    context->abs_size_retrieve_pos = 0;
    context->size_retrieve_buffer_pos = 0;
    context->size_retrieve_buffer_size = 0;
    context->fragment_requested = fragment_number;
    context->fragment_size_found = FALSE;
    context->fragment_size = 0;

    //reset the ping-pong buffers used to retrieve the fragment size.
    memset(context->size_retrieve_buffers [0],0,VIDEO_FMT_MP4R_BUFFER_SIZE);
    memset(context->size_retrieve_buffers [1],0,VIDEO_FMT_MP4R_BUFFER_SIZE);

    context->state = VIDEO_FMT_MP4R_STATE_RETRIEVE_FRAGMENT_SIZE;
    context->moov_present = FALSE;
    context->mdat_present = FALSE;
    video_fmt_mp4r_process (server_data);
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_peek_fragment_size

DESCRIPTION
  This function is given to the client as a callback.  It is called in order
  to request the size of the fragment (moov + mdat size or moof + mdat size)

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_peek_fragment_size
(
  void                        *server_data,
  uint32                      fragment_number
)
{
  if(server_data)
  {
    video_fmt_mp4r_context_type  *context = (video_fmt_mp4r_context_type*)server_data;

    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_DEBUG, "abs_size_retrieve_pos %llu",
                context->abs_size_retrieve_pos);
    /* In general, we always start from the start of the file to play.
       But in case of DASH, streamer may not maintain all the fragments
       So we start reading from the most recent fragment offset value.*/
    if(!context->isDashClip)
      context->abs_size_retrieve_pos = 0;

    /* Return an error if the format services is not currently waiting in the
    ** proper state.
    */
    context = (video_fmt_mp4r_context_type *) server_data;

    context->size_retrieve_buffer_which = 1;  /* will cause immediate switch */
                                           /* to buffer 0 on first read.  */

    context->size_retrieve_buffer_pos = 0;
    context->size_retrieve_buffer_size = 0;
    context->fragment_requested = fragment_number;
    context->fragment_size_found = FALSE;
    /* No need to reset this field, and this field is useful while playing DASH clips,
       if DATA-UNDERRUN happens in between.  */
    //context->fragment_size = 0;

    //reset the ping-pong buffers used to retrieve the fragment size.
    memset(context->size_retrieve_buffers [0],0,VIDEO_FMT_MP4R_BUFFER_SIZE);
    memset(context->size_retrieve_buffers [1],0,VIDEO_FMT_MP4R_BUFFER_SIZE);

    context->state = VIDEO_FMT_MP4R_STATE_PEEK_FRAGMENT_SIZE;
    context->moov_present = FALSE;
    context->mdat_present = FALSE;
    video_fmt_mp4r_process (server_data);
  }
}


/*===========================================================================

FUNCTION  video_fmt_mp4r_largest_frame_size

DESCRIPTION
  This function is given to the client as a callback.  It is called in order
  to request the size of the largest frame in the given track.

DEPENDENCIES
  For Fragmented files this is returning only the largest Frame size among
  the fragments already parsed.

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_largest_frame_size
(
  void                        *server_data,
  uint32                       stream_num
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_stream_type   *stream;
    video_fmt_stream_info_type  *stream_info;
    video_fmt_alloc_type          alloc_atom;
    video_fmt_free_type           free;
    uint64 table_size;
    uint32 frame_size = 0;
    uint32 bytes_consumed = 0;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) server_data;
    stream = &context->stream_state [stream_num];
    stream_info = &context->stream_info [stream_num];
    /* if we already traversed this STSZ table then and found the largest
    then don't traverse again and also for the fragmented files if this is
    main fragment then only traverse. */
    if((!stream_info->largest_found) && (context->current_sequence_number == 0) )
    {
        stream_info->largest = 0;
        /* Allocate STSZ table size local buffer to read data */
        alloc_atom.size = (uint32)stream->stsz.table_size * 4;
        context->callback_ptr (VIDEO_FMT_ALLOC, context->client_data,
                               &alloc_atom, NULL );
        memset((byte *)alloc_atom.ptr, 0, alloc_atom.size);
        /* Read 'stsz' table from the given offset. */
        context->get_data_dst = (uint8*)alloc_atom.ptr;
        context->get_data_src = stream->stsz.file_offset;
        context->get_data_size = stream->stsz.table_size * 4;
        context->get_data_needed = stream->stsz.table_size * 4;
        context->get_data_read = 0;
        context->expect_eof = FALSE;
        context->state_next [0] = context->state;
        context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
        video_fmt_mp4r_process (server_data);
        table_size = stream->stsz.table_size;
        while (table_size)
        {
            /* Read each frame size one by one from the loacal buffer */
            bytes_consumed += video_fmt_consume_data((uint8*)alloc_atom.ptr,
                                                     (byte *)&frame_size,
                                                     (int32)alloc_atom.size,
                                                     bytes_consumed,4,
                                                     context->byte_swap_needed,
                                                     context);
            if (stream_info->bytes)
            {
                stream_info->largest = FILESOURCE_MAX (stream_info->largest, frame_size);
            }
            else
            {
                stream_info->largest = FILESOURCE_MAX
                      (stream_info->largest, frame_size);
            }
            stream_info->bytes += frame_size;
            --table_size;
        }
        free.ptr = alloc_atom.ptr;
        context->callback_ptr (VIDEO_FMT_FREE, context->client_data,
                               &free, NULL);
    }
    /* Found the Largest */
    stream_info->largest_found = TRUE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_data_incomplete

DESCRIPTION
  This function is called whenever there is no more data to retrieve.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_data_incomplete (video_fmt_mp4r_context_type  *context)
{
    uint32                       i;
    context->cb_info.info.num_streams
        = context->num_streams;
    context->cb_info.info.streams
        = context->stream_info;
    context->cb_info.info.file_info
        = context->file_level_data;
    context->cb_info.info.read_cb
        = video_fmt_mp4r_read_stream;
    context->cb_info.info.sample_info_cb
        = video_fmt_mp4r_get_sample_info;
    context->cb_info.info.sync_sample_cb
        = video_fmt_mp4r_find_sync_sample;

    context->cb_info.info.parse_fragment_cb
        = video_fmt_mp4r_parse_fragment;

    context->cb_info.info.fragment_size_cb
        = video_fmt_mp4r_fragment_size;

    context->cb_info.info.fragment_size_peek_cb
        = video_fmt_mp4r_peek_fragment_size;

    context->cb_info.info.largest_frame_size_cb
        = video_fmt_mp4r_largest_frame_size;

    context->cb_info.info.abs_file_offset_cb
        = video_fmt_mp4r_abs_file_offset;


    context->cb_info.info.access_point_cb
      = video_fmt_mp4r_find_access_point;

    context->cb_info.info.server_data = context;

    context->callback_ptr (VIDEO_FMT_DATA_INCOMPLETE,
                           context->client_data,
                           &context->cb_info,
                           video_fmt_mp4r_end);

    context->state = VIDEO_FMT_MP4R_STATE_READY;
    for (i = 0; i < context->num_streams; ++i)
    {
        context->stream_state [i].state  = VIDEO_FMT_MP4R_STREAM_STATE_READY;
    }
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_atom_read_size_type

DESCRIPTION
  This function is called whenever there is no more data to retrieve.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_atom_read_size_type (video_fmt_mp4r_context_type  *context,
                                         uint8                        *atom_size,
                                         uint8                        *atom_type)
{
    video_fmt_mp4r_consume_data
                    (context,
                     atom_size,
                     FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                     VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);

    video_fmt_mp4r_consume_data
                    (context,
                     atom_type,
                     FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                     VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_failure

DESCRIPTION
  This function is called whenever there is a critical failure in the video
  format services, in order to free any resources that the services had
  allocated.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_failure (video_fmt_mp4r_context_type  *context)
{
    video_fmt_status_cb_func_type  callback_ptr;
    void                           *client_data;


    callback_ptr = context->callback_ptr;
    client_data = context->client_data;

    /* Stop services by indicating failure code to client. */
    callback_ptr (VIDEO_FMT_FAILURE, client_data, NULL, video_fmt_mp4r_end);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_decode_atom

DESCRIPTION
  This function determines which MP4 file atom was just parsed, and moves the
  state machine to process the atom accordingly.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_decode_atom (video_fmt_mp4r_context_type  *context)
{
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    uint32 index;

    /* Save offset of atom's contents. */
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    atom_stack_top->atom.offset = context->abs_pos;

    /* Until we know more about this atom type, assume it does not contain
    ** other atoms.
    */
    atom_stack_top->atom.contain = FALSE;

    /* If atom type is not a user-defined type, search for the type in
    ** the table of atom types.  If found, call the appropriate
    ** function to process the atom, and if the atom is a potential
    ** container, process any contained atoms.
    */
    if (!atom_stack_top->atom.user)
    {
        /* Search for the atom type in the atom type table. */
        for (index = 0; index < VIDEO_FMT_MP4R_ATOM_INFO_TABLE_SIZE;
             ++index)
        {
            if (video_fmt_mp4r_atom_info_table [index].type
                == atom_stack_top->atom.type)
            {
                break;
            }
        }

        /* If the atom type is not in the table, skip it. */
        if (index >= VIDEO_FMT_MP4R_ATOM_INFO_TABLE_SIZE)
        {
            atom_stack_top->atom.contain = FALSE;
            atom_stack_top->atom_process_func
                = video_fmt_mp4r_process_atom_skip;
        }

        /* Otherwise, save atom processing function pointer and move state
        ** machine to begin processing the atom.
        */
        else
        {
            /* Determine whether or not this atom can contain other atoms.
            ** Note that this is currently only needed for diagnostic
            ** purposes. Any atom with extra data in it after parsing is
            ** assumed to contain other atoms.
            */
            atom_stack_top->atom.contain
                = video_fmt_mp4r_atom_info_table [index].is_container;

            /* Prepare pointer to atom processing function, and move to atom
            ** processing state.
            */
            atom_stack_top->atom_process_func
                = video_fmt_mp4r_atom_info_table [index].process_func;
            atom_stack_top->atom_struct
                = video_fmt_mp4r_atom_info_table [index].atom_struct;
        }

        /* Move to the processing atom state. */
        context->state = VIDEO_FMT_MP4R_STATE_PROCESS_ATOM;
        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT;
    }

    /* Otherwise, this is a user-defined atom. */
    else
    {
        /* Send atom info to client, so client can parse the atom on
        ** its own.
        */
        context->cb_info.uuid_atom.offset = atom_stack_top->atom.offset;
        context->cb_info.uuid_atom.size = atom_stack_top->atom.size;
        context->cb_info.uuid_atom.atom_type = atom_stack_top->atom.type;
        context->callback_ptr (VIDEO_FMT_UUID,
                               context->client_data,
                               &context->cb_info,
                               video_fmt_mp4r_end);

        /* skip atom contents */
        atom_stack_top->atom.contain = FALSE;
        atom_stack_top->atom_process_func
            = video_fmt_mp4r_process_atom_skip;
        context->state = VIDEO_FMT_MP4R_STATE_PROCESS_ATOM;
        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT;
    }

    /* Send atom hint information to client. */
    if(context->num_streams > 0)
    {
       context->cb_info.hint.stream_info = &context->stream_info [context->num_streams - 1];
    }
    context->cb_info.hint.mp4 = &atom_stack_top->atom;
    context->callback_ptr (VIDEO_FMT_HINT,
                           context->client_data,
                           &context->cb_info,
                           video_fmt_mp4r_end);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_finish_atom

DESCRIPTION
  This function pops the current MP4 file atom off the atom stack, and returns
  to processing the parent atom.  If there is no parent, it moves the MP4 file
  format services state machine to decode the next atom in the file.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_finish_atom (video_fmt_mp4r_context_type  *context)
{
    /* Pop atom off stack. */
    if (context->atom_stack_top)
    {
      --context->atom_stack_top;
    }

    /* If stack is empty, decode next atom at global scope. */
    if (!context->atom_stack_top)
    {

        if(context->atom_stack[context->atom_stack_top].atom.type == MOOV_TYPE ||
           context->atom_stack[context->atom_stack_top].atom.type == MOOF_TYPE )
           /* if atom type is 'moov' and fragment is present*/
        {
            context->state = VIDEO_FMT_MP4R_STATE_PROCESS_FRAGMENT;
        }
        else
        {
            context->state = VIDEO_FMT_MP4R_STATE_DECODE_ATOM_HEADER;
        }
    }

    /* Otherwise, return to processing the parent atom. */
    else
    {
        context->state = VIDEO_FMT_MP4R_STATE_PROCESS_ATOM;
    }
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_read_buffer

DESCRIPTION
  This function moves the MP4 format services state machine as necessary to
  get the requested number of bytes into an input buffer.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the requested number of bytes is available in the active input
  buffer.

  If FALSE, not all the bytes are available, and the state machine needs to be
  run to retrieve the needed bytes.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_read_buffer
(
  video_fmt_mp4r_context_type  *context,
  uint32                      num_bytes,
  boolean                     expect_eof,
  video_fmt_mp4r_buffer_type  buffer_type
)
{
    if(buffer_type == VIDEO_FMT_MP4R_IN_BUFFER)
    {
        /* If the requested number of bytes is already available, return TRUE. */
        if (context->in_buffer_size >= num_bytes)
        {
            return TRUE;
        }

        /* If the total number of bytes will not fit in the current buffer, move
        ** the data we already have into the other buffer.
        */
        if (context->in_buffer_pos + num_bytes >= VIDEO_FMT_MP4R_BUFFER_SIZE)
        {
            if (context->in_buffer_size > 0)
            {
                /* Copy to the new buffer only if the current buffer position
                is correct and if the destination buffer is big enough to hold
                the data left over */
                if (context->in_buffer_pos < VIDEO_FMT_MP4R_BUFFER_SIZE &&
                    context->in_buffer_size <= VIDEO_FMT_MP4R_BUFFER_SIZE &&
                    context->in_buffer_which <= 1)
                {
                  memcpy (&context->in_buffers [1 - context->in_buffer_which] [0],
                      &context->in_buffers [context->in_buffer_which]
                      [context->in_buffer_pos],
                      context->in_buffer_size);
                }
                else
                {
                   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                               "context->in_buffer_pos has an invalid value");
                   video_fmt_mp4r_failure (context);
                   return FALSE;
                }

            }
            context->in_buffer_which = 1 - context->in_buffer_which;
            context->in_buffer_pos = 0;
        }

        /* Check to make sure we are not going to overflow the input buffer. */
        if (context->in_buffer_pos + num_bytes - context->in_buffer_size
            > VIDEO_FMT_MP4R_BUFFER_SIZE)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "read_buffer: in_buffer too small to "
                      "support this read request!");
          video_fmt_mp4r_failure (context);
          return FALSE;
        }

        /* Set up variables for getting data into the buffer, and move the state
        ** machine into the "get data" state.
        */
        context->get_data_dst = &context->in_buffers
            [context->in_buffer_which]
            [context->in_buffer_pos + context->in_buffer_size];
        context->get_data_src = context->abs_pos + context->in_buffer_size;
        context->get_data_size = VIDEO_FMT_MP4R_BUFFER_SIZE
            - (context->in_buffer_pos + context->in_buffer_size);
        context->get_data_needed = num_bytes - context->in_buffer_size;
        context->get_data_read = 0;
        context->expect_eof = expect_eof;
        context->state_next [1] = context->state;
        context->state_next [0] = VIDEO_FMT_MP4R_STATE_UPDATE_BUFFER;
        context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
        return FALSE;
    }
    else if(buffer_type == VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER)
    {
        /* If the requested number of bytes is already available, return TRUE. */
        if (context->size_retrieve_buffer_size >= num_bytes)
        {
            return TRUE;
        }

        /* If the total number of bytes will not fit in the current buffer, move
        ** the data we already have into the other buffer.
        */
        if (context->size_retrieve_buffer_pos + num_bytes >= VIDEO_FMT_MP4R_BUFFER_SIZE)
        {
          if(context->size_retrieve_buffer_size > 0)
          {
            if (context->size_retrieve_buffer_pos < VIDEO_FMT_MP4R_BUFFER_SIZE &&
                context->size_retrieve_buffer_size <= VIDEO_FMT_MP4R_BUFFER_SIZE &&
                context->size_retrieve_buffer_which <= 1)
            {
              memcpy (&context->size_retrieve_buffers [1 - context->size_retrieve_buffer_which] [0],
                &context->size_retrieve_buffers [context->size_retrieve_buffer_which]
               [context->size_retrieve_buffer_pos],
                context->size_retrieve_buffer_size);
            }
            else
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                          " context->size_retrieve_buffer_pos"
                          " contains an invalid value " );
              video_fmt_mp4r_failure (context);
              return FALSE;
            }
          }
          context->size_retrieve_buffer_which = 1 - context->size_retrieve_buffer_which;
          context->size_retrieve_buffer_pos = 0;
        }

        /* Check to make sure we are not going to overflow the input buffer. */
        if (context->size_retrieve_buffer_pos + num_bytes - context->size_retrieve_buffer_size
            > VIDEO_FMT_MP4R_BUFFER_SIZE)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "read_buffer: size_retrieve_buffer too small to "
                      "support this read request!");
          video_fmt_mp4r_failure (context);
          return FALSE;
        }

        /* Set up variables for getting data into the buffer, and move the state
        ** machine into the "get data" state.
        */
        context->get_data_dst = &context->size_retrieve_buffers
            [context->size_retrieve_buffer_which]
            [context->size_retrieve_buffer_pos + context->size_retrieve_buffer_size];
        context->get_data_src = context->abs_size_retrieve_pos + context->size_retrieve_buffer_size;
        context->get_data_size = VIDEO_FMT_MP4R_BUFFER_SIZE
            - (context->size_retrieve_buffer_pos + context->size_retrieve_buffer_size);
        context->get_data_needed = num_bytes - context->size_retrieve_buffer_size;
        context->get_data_read = 0;
        context->expect_eof = expect_eof;
        context->state_next [1] = context->state;
        context->state_next [0] = VIDEO_FMT_MP4R_STATE_UPDATE_SIZE_RETRIEVE_BUFFER;
        context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
        return FALSE;
    }
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_consume_data

DESCRIPTION
  This function removes the given number of bytes from the MP4 format services
  input buffers, storing the data in the given destination buffer, and
  updating the state of the input buffers.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_consume_data
(
  video_fmt_mp4r_context_type  *context,
  uint8                       *destination,
  uint32                      amount,
  boolean                     byte_reverse,
  video_fmt_mp4r_buffer_type  buffer_type
)
{
    uint32 index;
    if (destination == NULL)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "video_fmt_mp4r_consume_data:destination ptr is NULL ");
      video_fmt_mp4r_failure(context);
      return;
    }
    if(buffer_type == VIDEO_FMT_MP4R_IN_BUFFER)
    {
        if (byte_reverse)
        {
            for (index = 0; index < amount; ++index)
            {
                destination [index] = context->in_buffers
                    [context->in_buffer_which]
                    [context->in_buffer_pos + amount - index - 1];
            }
        }
        else
        {
            memcpy (destination,
                    &context->in_buffers [context->in_buffer_which]
                    [context->in_buffer_pos], amount);
        }
        context->in_buffer_pos += amount;
        context->abs_pos += amount;
        context->in_buffer_size -= amount;
    }
    if(buffer_type == VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER)
    {
        if (byte_reverse)
        {
            for (index = 0; index < amount; ++index)
            {
                destination [index] = context->size_retrieve_buffers
                    [context->size_retrieve_buffer_which]
                    [context->size_retrieve_buffer_pos + amount - index - 1];
            }
        }
        else
        {
            memcpy (destination,
                    &context->size_retrieve_buffers [context->size_retrieve_buffer_which]
                    [context->size_retrieve_buffer_pos], amount);
        }
        context->size_retrieve_buffer_pos += amount;
        context->abs_size_retrieve_pos += amount;
        context->size_retrieve_buffer_size -= amount;
    }
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_peek_data

DESCRIPTION
  This function peeks the given number of bytes from the MP4 format services
  input buffers, storing the data in the given destination buffer.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_peek_data
(
  video_fmt_mp4r_context_type  *context,
  uint8                       *destination,
  uint32                      amount,
  boolean                     byte_reverse
)
{
    uint32 index;
    if (destination == NULL)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "video_fmt_mp4r_peek_data:destination ptr is NULL ");
       video_fmt_mp4r_failure(context);
       return ;
    }
    if (byte_reverse)
    {
        for (index = 0; index < amount; ++index)
        {
            destination [index] = context->in_buffers
                [context->in_buffer_which]
                [context->in_buffer_pos + amount - index - 1];
        }
    }
    else
    {
        memcpy (destination,
                &context->in_buffers [context->in_buffer_which]
                [context->in_buffer_pos], amount);
    }
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_skip_data

DESCRIPTION
  This function advances the given number of bytes in the MP4 file being
  parsed.  Data skipped that is already in the input buffers is removed.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_skip_data
(
  video_fmt_mp4r_context_type  *context,
  uint64                      amount_to_discard,
  video_fmt_mp4r_buffer_type  buffer_type
)
{
    uint32 amt_discarded;
    uint32 amount = (uint32)amount_to_discard;

    if(buffer_type == VIDEO_FMT_MP4R_IN_BUFFER)
    {
        /* Remove any data from the input buffer that falls in the region being
        ** skipped.
        */
        amt_discarded = FILESOURCE_MIN (context->in_buffer_size, amount);
        context->in_buffer_pos += amt_discarded;
        context->in_buffer_size -= amt_discarded;

        /* Advance the absolute file pointer past the amount to be skipped.
           Check against max file size overflow. Required if we have extra >= 8
           junk bytes at the clip end. */
        if((MAX_FILE_SIZE - amount) < context->abs_pos)
        {
          MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_ERROR,
                       "process: offset corrupt in abs_pos %llu!",
                       context->abs_pos);
          context->abs_pos = MAX_FILE_SIZE;
          video_fmt_mp4r_failure(context);
        }
        else
        {
          context->abs_pos += amount;
        }
    }
    else if(buffer_type == VIDEO_FMT_MP4R_SIZE_RETRIEVE_BUFFER)
    {
        /* Remove any data from the input buffer that falls in the region being
        ** skipped.
        */
        amt_discarded = FILESOURCE_MIN (context->size_retrieve_buffer_size,
                                        amount);
        context->size_retrieve_buffer_pos += amt_discarded;
        context->size_retrieve_buffer_size -= amt_discarded;
        if((MAX_FILE_SIZE - amount) < context->abs_size_retrieve_pos)
        {
          MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_ERROR,
                      "process: offset corrupt in abs_size_retrieve_pos! %llu",
                      context->abs_size_retrieve_pos);
          context->abs_size_retrieve_pos = MAX_FILE_SIZE;
          video_fmt_mp4r_failure(context);
        }
        else
        {
          context->abs_size_retrieve_pos += amount;
        }
    }
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_check_end_of_file

DESCRIPTION
  This function checks to see if end of file

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_check_end_of_file
(
  video_fmt_mp4r_context_type  *context
)
{
    video_fmt_mp4_atom_type atom;

    if (!video_fmt_mp4r_read_buffer (context, 8, TRUE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
        return TRUE;
    }

    /* Copy over atom size and type.  If size doesn't cover atom type,
    ** don't decode rest of atom.
    */
    video_fmt_mp4r_peek_data
    (context,
     (uint8 *) &atom.size,
      FOURCC_SIGNATURE_BYTES, context->byte_swap_needed);


    /* Temporarily skipping ahead
    */
    context->in_buffer_pos += FOURCC_SIGNATURE_BYTES;

    video_fmt_mp4r_peek_data
    (context,
     (uint8 *) &atom.type,
     FOURCC_SIGNATURE_BYTES, context->byte_swap_needed);

    /* Reverting Back
    */
    context->in_buffer_pos -= FOURCC_SIGNATURE_BYTES;

    /* If the type matches mfra skip the atom
    */
    if (atom.type == 0x6D667261) /* 'mfra' */
    {
        video_fmt_mp4r_skip_data (context, atom.size, VIDEO_FMT_MP4R_IN_BUFFER);

        /* Now see if there is more data
        */
        if (!video_fmt_mp4r_read_buffer (context, 8, TRUE, VIDEO_FMT_MP4R_IN_BUFFER))
        {
            return TRUE;
        }

        /* Copy over atom size and type.  If size doesn't cover atom type,
        ** don't decode rest of atom.
        */
        video_fmt_mp4r_peek_data
        (context,
        (uint8 *) &atom.size,
        FOURCC_SIGNATURE_BYTES, context->byte_swap_needed);


        /* Temporarily skipping ahead
        */
        context->in_buffer_pos += FOURCC_SIGNATURE_BYTES;

        video_fmt_mp4r_peek_data
         (context,
         (uint8 *) &atom.type,
         FOURCC_SIGNATURE_BYTES, context->byte_swap_needed);

        /* Reverting Back
        */
        context->in_buffer_pos -= FOURCC_SIGNATURE_BYTES;

        if (atom.type == 0x6D66726F) /* 'mfro' */
        {
            video_fmt_mp4r_skip_data (context, atom.size, VIDEO_FMT_MP4R_IN_BUFFER);
        }
    }

    /* Now see if there is more data
    */
    if (!video_fmt_mp4r_read_buffer (context, 8, TRUE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
        return TRUE;
    }

    return FALSE;
}
/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_empty

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes atoms which themselves are empty except
  for the other atoms they contain.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_empty
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;

    UNUSED_PARAM(atom_struct_ptr);
    /* Branch according to processing substate. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];

    if(atom_stack_top->atom.type == MVEX_TYPE)
    {
        context->fragment_present = TRUE;
    }

    switch (atom_stack_top->process_substate)
    {
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
        /* Move on to processing child atoms. */
        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
        break;

    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
        video_fmt_mp4r_finish_atom (context);
        break;
    default:
      break;
    }
    return FALSE;
}


/*===========================================================================

FUNCTION  video_fmt_mp4r_check_valid_track

DESCRIPTION
  This function checks to see if a track can be processed in the current context.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, we are processing a track in the correct playing context.
  i.e. Audio Track in Audio-Context / Video Track in Video-Context etc..

SIDE EFFECTS
  None

===========================================================================*/
static boolean video_fmt_mp4r_check_valid_track(uint32 track_id,
                                   video_fmt_mp4r_context_type *context)
{
    uint8 index;
    for(index = 0; index <context->valid_track_count; index++)
    {
        if(context->valid_track_id[index] == track_id)
            return TRUE;
    }
    return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_hdlr

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function identifies if the track contains Audio/Video and
  if it should further allow to continue parsing of the current track. It
  checks if the track is being parsed in the correct client play(audio/video)
  context.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_hdlr
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    uint32 track_handler = 0;
    video_fmt_stream_info_type  *stream_info;

    UNUSED_PARAM(atom_struct_ptr);
    context = (video_fmt_mp4r_context_type *) context_ptr;
    if(context->num_streams > 0)
    {
       stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return FALSE;
    }
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];

    /* Read up to and including track type. */
    if (!video_fmt_mp4r_read_buffer (context, 12, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
       return TRUE;
    }
    /* Skip atom type 'hdlr' and its size */
    video_fmt_mp4r_skip_data (context, 8, VIDEO_FMT_MP4R_IN_BUFFER);

    /*Read the track handler*/
    video_fmt_mp4r_consume_data
                (context,
                 (uint8 *) &track_handler,
                 FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);

    if( ( ( track_handler == AUDIO_HANDLER ) &&
         !( context->client_playContext & PLAYING_AUDIO ) ) ||
        ( ( track_handler == VIDE0_HANDLER ) &&
         !( context->client_playContext & PLAYING_VIDEO ) ) ||
        ( ( track_handler == TEXT_HANDLER) &&
         !( context->client_playContext & PLAYING_TEXT ) ) ||
        ( ( track_handler == SUBTITLE_HANDLER) &&
         !( context->client_playContext & PLAYING_TEXT ) )
        )
    {
        /*Skip the 'trak' atom
          hierarchy: moov->trak->mdia->hdlr*/
        if((context->atom_stack_top>=3) &&
           (context->atom_stack [context->atom_stack_top - 3].atom.type == TRAK_TYPE))
        {
            atom_stack_top = &context->atom_stack [context->atom_stack_top - 3];
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "video_fmt_mp4r_process_atom_hdlr: Corrupt file:"
                      "cannot locate 'trak' atom..!");
          video_fmt_mp4r_failure (context);
          return TRUE;
        }

        video_fmt_mp4r_skip_data (context, (atom_stack_top->atom.size) -
                                           (uint32)(context->abs_pos  - atom_stack_top->atom.offset),
                                           VIDEO_FMT_MP4R_IN_BUFFER);
    }
    else
    {
        /*Add this track_id to the valid list if not already present*/
        if(!video_fmt_mp4r_check_valid_track(stream_info->track_id,context))
            context->valid_track_id[++context->valid_track_count - 1] = stream_info->track_id;
        /*Skip the rest of the atom*/
        video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size - 12, VIDEO_FMT_MP4R_IN_BUFFER);
    }

    /* Done with this atom. */
    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_skip

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes atoms which contain data we do not need
  to process, and will skip.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_skip
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;

    UNUSED_PARAM(atom_struct_ptr);
    /* Skip the rest of the atom. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size, VIDEO_FMT_MP4R_IN_BUFFER);

    if(atom_stack_top->atom.type == MDAT_TYPE)
        context->mdat_present = TRUE;

    /* Done with this atom. */
    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_tref_child

DESCRIPTION

This function is one of many functions which process MP4 file atoms.
This particular function parses the 'tref' children.
The following atoms may be contained in the 'tref' container:
'hint', 'dpnd', 'ipir', 'mpod', 'sync', which contain a track-id list.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_tref_child
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_stream_info_type  *stream_info;
    uint32 i;

    UNUSED_PARAM(atom_struct_ptr);
    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    if(context->num_streams > 0)
    {
       stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return FALSE;
    }
    stream_info->ref_track.ref_atom = atom_stack_top->atom.type;
    stream_info->ref_track.track_count = atom_stack_top->atom.size / 4;
    /* tref_child.fields[6] is hardcoded 6 element arrays */
    if(stream_info->ref_track.track_count > VIDEO_FMT_MAX_MEDIA_STREAMS)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,"more ref tracks than supported"
                  " (%lu).", stream_info->ref_track.track_count);
      stream_info->ref_track.track_count = VIDEO_FMT_MAX_MEDIA_STREAMS;
    }

    /* read required bytes */
    if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
      return TRUE;
    }
    for (i = 0; i < stream_info->ref_track.track_count; ++i)
    {
      video_fmt_mp4r_consume_data(context,
                                  (uint8 *)(stream_info->ref_track.track_id+i),
                                  FOURCC_SIGNATURE_BYTES,
                                  context->byte_swap_needed,
                                  VIDEO_FMT_MP4R_IN_BUFFER );
    }
    /* Done with this atom. */
    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}
/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_devc_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'devc' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_devc_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /*
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type   *stream;
    */
    video_fmt_stream_info_type   *stream_info;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;

    /*
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    stream = &context->stream_state [context->num_streams - 1];
    */
    if(context->num_streams > 0)
    {
       stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }

    stream_info->type = VIDEO_FMT_STREAM_AUDIO;
    stream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_EVRC;
    stream_info->subinfo.audio.sampling_frequency = 8000;
    stream_info->subinfo.audio.num_channels = 1;

    /* Set the destination fields before "consuming" the data */
    devc.fields [0].dest = (uint8 *)
        &stream_info->subinfo.audio.audio_params.frames_per_sample;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_decb_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'decb' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_decb_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /*
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type   *stream;
    */
    video_fmt_stream_info_type   *stream_info;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;

    /*
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    stream = &context->stream_state [context->num_streams - 1];
    */
    if(context->num_streams > 0)
    {
       stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }

    stream_info->type = VIDEO_FMT_STREAM_AUDIO;
    stream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_EVRC_B;
    stream_info->subinfo.audio.sampling_frequency = 8000;
    stream_info->subinfo.audio.num_channels = 1;

    /* Set the destination fields before "consuming" the data */
    decb.fields [0].dest = (uint8 *)
       &stream_info->subinfo.audio.audio_params.frames_per_sample;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_decw_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'decw' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_decw_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /*
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type   *stream;
    */
    video_fmt_stream_info_type   *stream_info;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;

    /*
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    stream = &context->stream_state [context->num_streams - 1];
    */
    if(context->num_streams > 0)
    {
       stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }

    stream_info->type = VIDEO_FMT_STREAM_AUDIO;
    stream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_EVRC_WB;
    stream_info->subinfo.audio.sampling_frequency = 16000;
    stream_info->subinfo.audio.num_channels = 1;

    /* Set the destination fields before "consuming" the data */
    decw.fields [0].dest = (uint8 *)
        &stream_info->subinfo.audio.audio_params.frames_per_sample;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_dqcp_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'dqcp' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_dqcp_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /*
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type   *stream;
    */
    video_fmt_stream_info_type   *stream_info;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /*
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    stream = &context->stream_state [context->num_streams - 1];
    */
    if(context->num_streams > 0)
    {
       stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }

    stream_info->type = VIDEO_FMT_STREAM_AUDIO;
    stream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_PUREVOICE;
    stream_info->subinfo.audio.sampling_frequency = 8000;
    stream_info->subinfo.audio.num_channels = 1;

    /* Set the destination fields before "consuming" the data */
    dqcp.fields [0].dest = (uint8 *)
        &stream_info->subinfo.audio.audio_params.frames_per_sample;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_s263_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 's263' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_s263_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_stream_info_type   *stream_info;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    if(context->num_streams > 0)
    {
      stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }

    stream_info->type = VIDEO_FMT_STREAM_VIDEO;
    stream_info->subinfo.video.format = VIDEO_FMT_STREAM_VIDEO_H263;

    /* Set the destination fields before "consuming" the data */
    s263.fields [0].dest = (uint8 *) &stream_info->subinfo.video.width;
    s263.fields [1].dest = (uint8 *) &stream_info->subinfo.video.height;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_encv_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'encv' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/

void video_fmt_mp4r_encv_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_stream_info_type   *stream_info;

    if(NULL == context_ptr)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context_ptr is NULL");
      return;
    }
    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    if(context->num_streams > 0)
    {
      stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }
    /* Set the destination fields before "consuming" the data */
    encv.fields [0].dest = (uint8 *) &stream_info->subinfo.video.width;
    encv.fields [1].dest = (uint8 *) &stream_info->subinfo.video.height;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_frma_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'frma' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_frma_prep_dest (void *context_ptr)
{
  video_fmt_mp4r_context_type  *context = NULL;
  video_fmt_stream_info_type   *stream_info = NULL;

  context = (video_fmt_mp4r_context_type *) context_ptr;
  if((context) && (context->num_streams >= 1))
  {
    stream_info = &context->stream_info [context->num_streams - 1];
  }
  if(stream_info)
  {
    if(stream_info->type == VIDEO_FMT_STREAM_VIDEO)
    {
      frma.fields [0].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.data_format;
    }
    else if(stream_info->type == VIDEO_FMT_STREAM_AUDIO)
    {
      frma.fields [0].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.data_format;
    }
  }
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_schm_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'schm' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_schm_prep_dest (void *context_ptr)
{
  video_fmt_mp4r_context_type  *context = NULL;
  video_fmt_stream_info_type   *stream_info = NULL;

  context = (video_fmt_mp4r_context_type *) context_ptr;
  if((context) && (context->num_streams >= 1))
  {
    stream_info = &context->stream_info [context->num_streams - 1];
  }
  if(stream_info)
  {
    if(stream_info->type == VIDEO_FMT_STREAM_VIDEO)
    {
      schm.fields [0].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.scheme_type;
      schm.fields [1].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.scheme_version;
    }
    else if(stream_info->type == VIDEO_FMT_STREAM_AUDIO)
    {
      schm.fields [0].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.scheme_type;
      schm.fields [1].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.scheme_version;
    }
  }
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_ohdr_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'ohdr' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_ohdr_prep_dest (void *context_ptr)
{
  video_fmt_mp4r_context_type  *context = NULL;
  video_fmt_stream_info_type   *stream_info = NULL;

  context = (video_fmt_mp4r_context_type *) context_ptr;
  if((context) && (context->num_streams >= 1))
  {
    stream_info = &context->stream_info [context->num_streams - 1];
  }
  if(stream_info)
  {
    if(stream_info->type == VIDEO_FMT_STREAM_VIDEO)
    {
      ohdr.fields [0].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.EncryptionMethod;
      ohdr.fields [1].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.PaddingScheme;
      ohdr.fields [2].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.PlaintextLength;
      ohdr.fields [3].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.ContentIDLength;
      ohdr.fields [4].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.RightsIssuerURLLength;
      ohdr.fields [5].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.TextualHeadersLength;
    }
    else if(stream_info->type == VIDEO_FMT_STREAM_AUDIO)
    {
      ohdr.fields [0].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.EncryptionMethod;
      ohdr.fields [1].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.PaddingScheme;
      ohdr.fields [2].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.PlaintextLength;
      ohdr.fields [3].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.ContentIDLength;
      ohdr.fields [4].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.RightsIssuerURLLength;
      ohdr.fields [5].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.TextualHeadersLength;
    }
  }
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_odaf_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'odaf' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_odaf_prep_dest (void *context_ptr)
{
  video_fmt_mp4r_context_type  *context = NULL;
  video_fmt_stream_info_type   *stream_info = NULL;

  context = (video_fmt_mp4r_context_type *) context_ptr;
  if((context) && (context->num_streams >= 1))
  {
    stream_info = &context->stream_info [context->num_streams - 1];
  }
  if(stream_info)
  {
    if(stream_info->type == VIDEO_FMT_STREAM_VIDEO)
    {
      odaf.fields [0].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.SelectiveEncryption;
      odaf.fields [1].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.KeyIndicatorLength;
      odaf.fields [2].dest = (uint8 *) &stream_info->subinfo.video.pdcf_info.IVLength;
    }
    else if(stream_info->type == VIDEO_FMT_STREAM_AUDIO)
    {
      odaf.fields [0].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.SelectiveEncryption;
      odaf.fields [1].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.KeyIndicatorLength;
      odaf.fields [2].dest = (uint8 *) &stream_info->subinfo.audio.pdcf_info.IVLength;
    }
  }
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_stco_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'stco' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_stco_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /* video_fmt_mp4r_atom_pos_type *atom_stack_top; */
    video_fmt_mp4r_stream_type   *stream;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /* atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];*/
    if(context->num_streams > 0)
    {
       stream = &context->stream_state [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }
    if(context->cur_atom_size >= 8)
    {
      stream->stco.atom_size = (uint32)context->cur_atom_size - 8;
    }

    /* Set the destination fields before "consuming" the data */
    stco.fields [0].dest = (uint8 *) &stream->stco.table_size;

    /* Record offset of table in stream state. */
    stream->stco.file_offset = context->abs_pos + stco.fields [0].offset
        + stco.fields [0].size;
}


/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_c064_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'co64' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_co64_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /* video_fmt_mp4r_atom_pos_type *atom_stack_top; */
    video_fmt_mp4r_stream_type   *stream;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /* atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];*/
    if(context->num_streams > 0)
    {
       stream = &context->stream_state [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }
    if(context->cur_atom_size >= 8)
    {
      stream->stco.atom_size = (uint32)context->cur_atom_size - 8;
    }

    /* Set the destination fields before "consuming" the data */
    /* we don't have 64 bit support we are using stco table to compute the chunk offset */
    co64.fields [0].dest = (uint8 *) &stream->stco.table_size;

    /* Record offset of table in stream state. */
    stream->stco.file_offset = context->abs_pos + stco.fields [0].offset
        + stco.fields [0].size;
    /* This flag is to use the correct 32 bit offset from the stco table */
    stream->co64_present = TRUE;
}


/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_stsc_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'stsc' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_stsc_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /* video_fmt_mp4r_atom_pos_type *atom_stack_top; */
    video_fmt_mp4r_stream_type   *stream;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /* atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];*/
    if(context->num_streams > 0)
    {
       stream = &context->stream_state [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }
    if(context->cur_atom_size >= 8)
    {
      stream->stsc.atom_size = (uint32)context->cur_atom_size - 8;
    }

    /* Set the destination fields before "consuming" the data */
    stsc.fields [0].dest = (uint8 *)&stream->stsc.table_size;

    /* Record offset of table in stream state. */
    stream->stsc.file_offset = context->abs_pos + stsc.fields [0].offset
        + stsc.fields [0].size;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_stts_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'stts' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_stts_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /* video_fmt_mp4r_atom_pos_type *atom_stack_top;*/
    video_fmt_mp4r_stream_type   *stream;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /* atom_stack_top = &context->atom_stack [context->atom_stack_top - 1]; */
    if(context->num_streams > 0)
    {
       stream = &context->stream_state [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }
    if(context->cur_atom_size >= 8)
    {
      stream->stts.atom_size = (uint32)context->cur_atom_size - 8;
    }

    /* Set the destination fields before "consuming" the data */
    stts.fields [0].dest = (uint8 *) &stream->stts.table_size;

    /* Record offset of table in stream state. */
    stream->stts.file_offset = context->abs_pos + stts.fields [0].offset
        + stts.fields [0].size;
}


/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_ctts_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'ctts' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_ctts_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /* video_fmt_mp4r_atom_pos_type *atom_stack_top;*/
    video_fmt_mp4r_stream_type   *stream;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /* atom_stack_top = &context->atom_stack [context->atom_stack_top - 1]; */
    if(context->num_streams > 0)
    {
       stream = &context->stream_state [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }
    if(context->cur_atom_size >= 8)
    {
      stream->ctts.atom_size = (uint32)context->cur_atom_size - 8;
    }

    /* Set the destination fields before "consuming" the data */
    ctts.fields [0].dest = (uint8 *)
                           &stream->ctts.atom_version;
    ctts.fields [1].dest = (uint8 *) &stream->ctts.table_size;

    /* Record offset of table in stream state. */
    stream->ctts.file_offset = context->abs_pos + ctts.fields [1].offset
        + ctts.fields [1].size;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_stss_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'stss' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_stss_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /* video_fmt_mp4r_atom_pos_type *atom_stack_top;*/
    video_fmt_mp4r_stream_type   *stream;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /* atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];*/
    if(context->num_streams > 0)
    {
    stream = &context->stream_state [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }
    if(context->cur_atom_size >= 8)
    {
      stream->stss.atom_size = (uint32)context->cur_atom_size - 8;
    }

    /* Set the destination fields before "consuming" the data */
    stss.fields [0].dest = (uint8 *) &stream->stss.table_size;

    /* Record offset of table in stream state. */
    stream->stss.file_offset = context->abs_pos + stss.fields [0].offset
        + stss.fields [0].size;
}

/* <EJECT> */
/*===========================================================================
FUNCTION  video_fmt_mp4r_stsd_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'stsd' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_stsd_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    /* video_fmt_mp4r_atom_pos_type *atom_stack_top; */
    video_fmt_mp4r_stream_type   *stream;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    /* atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];*/
    if(context->num_streams > 0)
    {
       stream = &context->stream_state [context->num_streams - 1];
    }
   else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return;
    }

    /* Set the destination fields before "consuming" the data */
    stsd.fields[0].dest = (uint8 *)&stream->stsd_entry_count;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_mfhd_prep_dest

DESCRIPTION
  This function prepares the destination fields where information
  from the 'mfhd' atom is to be stored.

DEPENDENCIES
  None

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_mfhd_prep_dest (void *context_ptr)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_stream_info_type   *stream_info = 0;
    video_fmt_mp4r_stream_type   *stream = 0;
    uint32                       index;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;

    /* Set the destination fields before "consuming" the data */
    mfhd.fields [0].dest = (uint8 *) &context->current_sequence_number;
    context->current_track_id = 0;

    context->mdat_present = FALSE;
    context->moov_present = FALSE;
    context->moof_present = FALSE;

    /*Initialize all truns for the stream for a new fragment */
    for(index = 0 ; index < context->num_streams; index++)
    {
        stream = &context->stream_state [index];
        stream_info = &context->stream_info[index];
        stream->initialize_trun = TRUE;
        stream->fill_trun_cache = FALSE;
        /*Subtract the mfhd + moof header size to get the abs fragment_offset*/
        stream_info->fragment_offset = context->abs_pos -
                                      ((FOURCC_SIGNATURE_BYTES) * 4);

    }
}
/* <EJECT> */

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_saiz

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'saiz' atom, which contains
  properties of the auxillary sample sizes which are used in encryption..

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_saiz
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type         *context;
  video_fmt_mp4r_stream_type          *stream = 0;
  video_fmt_stream_info_type          *stream_info = 0;
  video_fmt_mp4r_sample_table_type    *saiz;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  uint32 data_consumed = 0;
  uint32 index = 0;

  UNUSED_PARAM(atom_struct_ptr);
  if(NULL == context_ptr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context_ptr is NULL");
    return TRUE;
  }
  /* Branch according to processing substate. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];

  if (context->current_track_id > VIDEO_FMT_MAX_MEDIA_STREAMS)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "track_id %lu is more than max media streams",
                 context->current_track_id);
    video_fmt_mp4r_failure (context);
    return TRUE;
  }

  for(index = 0 ; index < context->num_streams; index++)
  {
    if(context->stream_info[index].track_id == context->current_track_id)
    {
      stream_info = &context->stream_info [index];
      stream = &context->stream_state [index];
      break;
    }
  }

  if(!(stream_info) || !(stream))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "process_atom_saiz: Could not find stream for track!");
    video_fmt_mp4r_failure (context);
    return TRUE;
  }

  saiz = &stream->saiz;

  /* Read atom buffer. */
  if (!video_fmt_mp4r_read_buffer (context,
                                   atom_stack_top->atom.size,
                                   FALSE,
                                   VIDEO_FMT_MP4R_IN_BUFFER))
  {
      return TRUE;
  }

  /* skip version info */
  video_fmt_mp4r_skip_data (context, 1, VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed +=1;

  /* get the flag info  */
  video_fmt_mp4r_consume_data
    (context, (uint8 *) &saiz->atom_flag, 3,
    context->byte_swap_needed,
    VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed +=3;

  /* SAIZ atom may have aux info type and parameter value, which determines the
     format of the auxiliary information as mentioned in ISO/IEC 14496-12
     ( 4th Edition-2012) :8.7.8.1 */
  if(saiz->atom_flag)
  {
    /* get the aux_info_type info  */
    video_fmt_mp4r_consume_data (context, (uint8 *) &saiz->aux_info_type, 4,
                                 context->byte_swap_needed,
                                  VIDEO_FMT_MP4R_IN_BUFFER);
    data_consumed +=4;
    /* get the aux_info_type_parameter info  */
    video_fmt_mp4r_consume_data (context,
                                (uint8 *) &saiz->aux_info_type_parameter, 4,
                                 context->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
    data_consumed +=4;
  }
  /* get the default sample info size */
  video_fmt_mp4r_consume_data
      (context, (uint8 *) &saiz->default_sample_size, 1,
       context->byte_swap_needed,
       VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed +=1;

  /* get the sample_count */
  video_fmt_mp4r_consume_data
      (context, (uint8 *) &saiz->table_size, 4,
       context->byte_swap_needed,
       VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed +=4;

  saiz->file_offset = context->abs_pos;
  saiz->track_id    = context->current_track_id;
  saiz->cache_size  = 0;
  saiz->cache_start = 0;

  /* Skip the remaining data */
  video_fmt_mp4r_skip_data (context,
                            atom_stack_top->atom.size - data_consumed,
                            VIDEO_FMT_MP4R_IN_BUFFER);
  video_fmt_mp4r_finish_atom(context);
  return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_saio

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'saio' atom, which contains
  properties of the auxillary sample sizes which are used in encryption..

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_saio
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type         *context;
  video_fmt_mp4r_stream_type          *stream = 0;
  video_fmt_stream_info_type          *stream_info = 0;
  video_fmt_mp4r_sample_table_type    *saio;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  uint32 data_consumed = 0;
  uint32 index = 0;

  UNUSED_PARAM(atom_struct_ptr);
  if(NULL == context_ptr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context_ptr is NULL");
    return TRUE;
  }
  /* Branch according to processing substate. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];

  if (context->current_track_id > VIDEO_FMT_MAX_MEDIA_STREAMS)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "SAIO_Process: too many streams!");
    video_fmt_mp4r_failure (context);
    return TRUE;
  }

  for(index = 0 ; index < context->num_streams; index++)
  {
    if(context->stream_info[index].track_id == context->current_track_id)
    {
      stream_info = &context->stream_info [index];
      stream = &context->stream_state [index];
      break;
    }
  }

  if(!(stream_info) || !(stream))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "SAIO_Process: Could not find stream for track!");
    video_fmt_mp4r_failure (context);
    return TRUE;
  }

  saio = &stream->saio;

  /* Read atom buffer. */
  if (!video_fmt_mp4r_read_buffer (context,
                                   atom_stack_top->atom.size,
                                   FALSE,
                                   VIDEO_FMT_MP4R_IN_BUFFER))
  {
      return TRUE;
  }

  /* get version info */
  video_fmt_mp4r_consume_data
    (context, (uint8 *) &saio->atom_version, 1,
    context->byte_swap_needed,
    VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 1;

  /* get the flag info  */
  video_fmt_mp4r_consume_data
    (context, (uint8 *) &saio->atom_flag, 3,
    context->byte_swap_needed,
    VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 3;

  /* SAIO atom may have aux info type and parameter value, which determines the
     format of the auxiliary information as mentioned in ISO/IEC 14496-12
     ( 4th Edition-2012) :8.7.8.1 */
  if(saio->atom_flag)
  {
    /* get the aux_info_type info  */
    video_fmt_mp4r_consume_data (context, (uint8 *) &saio->aux_info_type, 4,
                                 context->byte_swap_needed,
                                  VIDEO_FMT_MP4R_IN_BUFFER);
    data_consumed +=4;
    /* get the aux_info_type_parameter info  */
    video_fmt_mp4r_consume_data (context,
                                (uint8 *) &saio->aux_info_type_parameter, 4,
                                 context->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
    data_consumed +=4;
  }

  /* get the sample_count */
  video_fmt_mp4r_consume_data
      (context, (uint8 *) &saio->table_size, 4,
       context->byte_swap_needed,
       VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed +=4;

  saio->file_offset = context->abs_pos;
  saio->track_id    = context->current_track_id;
  saio->cache_size  = 0;
  saio->cache_start = 0;

  stream->saio.base_data_offset = stream->base_data_offset;

  if( context->isDashClip == TRUE)
  {
    video_fmt_mp4r_atom_pos_type *atom_stack_top = &context->atom_stack
                                            [context->atom_stack_top - 3];
    /* SAIO data offset is relative value w.r.t moof/tfhd offset*/
    stream->saio.data_offset = atom_stack_top->atom.offset - 8;
  }
  /* Skip the remaining data */
  video_fmt_mp4r_skip_data (context,
                            atom_stack_top->atom.size - data_consumed,
                            VIDEO_FMT_MP4R_IN_BUFFER);
  video_fmt_mp4r_finish_atom(context);
  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_ftyp

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'ftyp' atom, which contains info
  about the different brands that are supported in the input file.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_ftyp
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type        *context;
  uint32 atom_size;
  uint32 dataConsumed = 0;
  uint32 brandType    = 0;

  UNUSED_PARAM(atom_struct_ptr);
  /* Verify that the atom is not missing any key data. */
  context = (video_fmt_mp4r_context_type *) context_ptr;

  atom_size = context->atom_stack[context->atom_stack_top - 1].atom.size;

  /* Read complete "ftyp" atom size
  */
  if (!video_fmt_mp4r_read_buffer (context, atom_size,
                                   FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }

  /* Read major brand */
  video_fmt_mp4r_consume_data
        (context, (uint8 *) &brandType, 4,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);
  dataConsumed = 4;
  // Store major brand type
  context->ulMajorBrand = brandType;
  if(brandType == DASH_TYPE || brandType == MSDH_TYPE )
  {
    context->isDashClip = TRUE;
  }
  else
  {
    // Skip major version and minor version info
    video_fmt_mp4r_skip_data (context, 4, VIDEO_FMT_MP4R_IN_BUFFER);
    dataConsumed += 4;
    while(atom_size > dataConsumed)
    {
      // Parse Compatible brand
      video_fmt_mp4r_consume_data
          (context, (uint8 *) &brandType, 4,
           context->byte_swap_needed,
           VIDEO_FMT_MP4R_IN_BUFFER);
      dataConsumed += 4;
      // Supported brandtype 'dash' and 'msdh'
      if(brandType == DASH_TYPE || brandType == MSDH_TYPE )
      {
        context->isDashClip = TRUE;
        break;
      }
    }//while
  }//else

  /* Skip any data that has not parsed */
  video_fmt_mp4r_skip_data (context, atom_size - dataConsumed,
                            VIDEO_FMT_MP4R_IN_BUFFER);
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_tfdt

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'tfhd' atom, which contains a table
  of description common for all samples in all TRUN's.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_tfdt
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type  *context;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type *stream = 0;
  uint8 version = 0;
  uint32 data_consumed = 0, index;

  UNUSED_PARAM(atom_struct_ptr);
  /* Verify that the atom is not missing any key data. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];

  if (context->num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS ||
      context->num_streams == 0)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "process_atom_tfdt: too many/few streams!");
    video_fmt_mp4r_failure (context);
    return TRUE;
  }

  for(index = 0 ; index < context->num_streams; index++)
  {
    if(context->stream_info[index].track_id == context->current_track_id)
    {
      stream = &context->stream_state [index];
      break;
    }
  }

  if(!(stream))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "process_atom_tfdt: Could not find stream for track!");
    video_fmt_mp4r_failure (context);
    return TRUE;
  }

  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size,
                                   FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  video_fmt_mp4r_consume_data(context, &version, 1, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 1;
  video_fmt_mp4r_skip_data(context, 3, VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 3;
  /* If it is version#1 type, then 8 bytes are used to store timestamp info */
  if(1 == version)
  {
    video_fmt_mp4r_consume_data(context, (uint8 *)&stream->cur_fragment_timestamp, 4, context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    stream->cur_fragment_timestamp <<= 32;
    data_consumed += 4;
  }
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream->cur_fragment_timestamp, 4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;


  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "cur_fragment_timestamp is %lld", stream->cur_fragment_timestamp);
  stream->fragment_boundary = TRUE;
  video_fmt_mp4r_skip_data(context, atom_stack_top->atom.size - data_consumed, VIDEO_FMT_MP4R_IN_BUFFER);
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_mvhd

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'mvhd' atom, which contains
  properties of the file like creation time, modified time and timescale etc...

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_mvhd
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type  *context;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  video_fmt_file_info_type   *file_info;
  uint8 version = 0;
  uint32 data_consumed = 0;

  UNUSED_PARAM(atom_struct_ptr);
  /* Verify that the atom is not missing any key data. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  file_info = &context->file_level_data;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];

  /* Read complete atom data in single attempt */
  if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size,
                                   FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  video_fmt_mp4r_consume_data(context, &version, 1, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 1;
  video_fmt_mp4r_skip_data(context, 3, VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 3;

  /* If it is version#1 type, then 8 bytes are used to store
     creationtime info */
  if(1 == version)
  {
    /* creation time */
    video_fmt_mp4r_consume_data(context, (uint8 *)&file_info->creation_time,4,
                                context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
                                file_info->creation_time <<= 32;
    data_consumed += 4;
  }
  video_fmt_mp4r_consume_data(context, (uint8 *)&file_info->creation_time,4,
                              context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;

  /* If it is version#1 type, then 8 bytes are used to store mod-time info */
  if(1 == version)
  {
    /* mod time */
    video_fmt_mp4r_consume_data(context, (uint8 *)&file_info->mod_time,
                                4, context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    file_info->mod_time <<= 32;
    data_consumed += 4;
  }
  /* mod time */
  video_fmt_mp4r_consume_data(context, (uint8 *)&file_info->mod_time,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;


  /* movie_timescale -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&file_info->movie_timescale,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;

  if(1 == version)
  {
    /* total_movie_duration -- 8 bytes */
    video_fmt_mp4r_consume_data(context,(uint8 *)&file_info->total_movie_duration,
                                4, context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    file_info->total_movie_duration <<= 32;
    data_consumed += 4;
  }
  /* media_duration -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&file_info->total_movie_duration,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;


  /* skip all the data till last 4 bytes which contains next track-ID info */
  video_fmt_mp4r_skip_data(context,
                           atom_stack_top->atom.size - data_consumed - 4,
                           VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += atom_stack_top->atom.size - data_consumed - 4;

  /* tkhd_width -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&file_info->next_track_id,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;

  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_mdhd

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'mvhd' atom, which contains
  properties of the file like creation time, modified time and timescale etc...

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_mdhd
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type  *context;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  video_fmt_stream_info_type   *stream_info = 0;
  uint8 version = 0;
  uint32 data_consumed = 0;

  UNUSED_PARAM(atom_struct_ptr);
  /* Verify that the atom is not missing any key data. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  if(context->num_streams > 0)
  {
    stream_info = &context->stream_info [context->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (context);
    context->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }

  /* Read up to and including tf_flags track_id base data offset
  ** sample_description_index default_sample_duration default_sample_size
  ** default_sample_flags
  */
  if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size,
                                   FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  video_fmt_mp4r_consume_data(context, &version, 1, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 1;
  video_fmt_mp4r_skip_data(context, 3, VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 3;
  if(1 == version)
  {
    /* creation time */
    video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->creation_time,
                                4, context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    stream_info->creation_time <<= 32;
    data_consumed += 4;
  }
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->creation_time,
                              4, context->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);

  data_consumed += 4;

  if(1 == version)
  {
    /* mod time */
    video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->mod_time,4,
                                context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    stream_info->mod_time <<= 32;
    data_consumed += 4;
  }
  /* mod time */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->mod_time,
                              4,context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;


  /* movie_timescale -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->media_timescale,
                              4,context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;

  if(1 == version)
  {
    /* total_movie_duration -- 8 bytes */
    video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->media_duration,
                                4,context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    stream_info->media_duration <<= 32;
    data_consumed += 4;
  }
  /* media_duration -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->media_duration,
                              4,context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;

  /* skip the remaining data */
  video_fmt_mp4r_skip_data(context, atom_stack_top->atom.size - data_consumed,
                           VIDEO_FMT_MP4R_IN_BUFFER);
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_tkhd

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'tkhd' atom, which contains
  properties of the track like height and width info etc...

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_tkhd
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type  *context;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  video_fmt_stream_info_type   *stream_info = 0;
  uint8 version = 0;
  uint32 ulDataConsumed = 0;
  int32 nX00 = 0;
  int32 nX01 = 0;
  int32 nX10 = 0;
  int32 nX11 = 0;
  static const int32 cDefaultVal = 0x00010000;

  UNUSED_PARAM(atom_struct_ptr);
  /* Verify that the atom is not missing any key data. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  if(context->num_streams > 0)
  {
    stream_info = &context->stream_info [context->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (context);
    context->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }

  /* Read complete atom data */
  if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size,
                                   FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  video_fmt_mp4r_consume_data(context, &version, 1, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 1;
  video_fmt_mp4r_skip_data(context, 3, VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 3;
  if(1 == version)
  {
    /* creation time */
    video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->creation_time,
                                4, context->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
    stream_info->creation_time <<= 32;
    ulDataConsumed += 4;
  }
  /* creation time */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->creation_time,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;

  if(1 == version)
  {
    /* mod time */
    video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->mod_time,
                                4, context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    stream_info->mod_time <<= 32;
    ulDataConsumed += 4;
  }
  /* mod time */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->mod_time,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;

  /* track_id -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->track_id,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;

  /* reserved 4 bytes */
  video_fmt_mp4r_skip_data(context, 4, VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;

  if(1 == version)
  {
    /* media_duration -- 8 bytes */
    video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->media_duration,
                                4, context->byte_swap_needed,
                                VIDEO_FMT_MP4R_IN_BUFFER);
    stream_info->media_duration <<= 32;
    ulDataConsumed += 4;
  }

  /* media_duration -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->media_duration,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;
  //! Use Movie Time-scale (TKHD duration field uses MVHD time-scale value only)
  stream_info->media_timescale = context->file_level_data.movie_timescale;
  //! If the duration of this track cannot be determined,
  //! then duration is set to all 1s (32-bit maxint).
  if((0xFFFFFFFF == stream_info->media_duration ) ||
     (0xFFFFFFFFFFFFFFFF == stream_info->media_duration ) )
  {
    stream_info->media_duration = context->file_level_data.total_movie_duration;
    stream_info->media_timescale= context->file_level_data.movie_timescale;
  }


  /* skip following fields ( 16 bytes)
      const unsigned int(32)[2] reserved = 0;
      template int(16) layer = 0;
      template int(16) alternate_group = 0;
      template int(16) volume = {if track_is_audio 0x0100 else 0};
      const unsigned int(16) reserved = 0;
   */
  video_fmt_mp4r_skip_data(context,16,VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 16;

  /*
    Transformation matrix ISO-BFF 6.2.2
    | a  b  u |     | X00  X01  X02|
    | c  d  v |  =  | X10  X11  X12|
    | x  y  z |     | X20  X21  X23|
    The value of matrix are stored in order { a, b, u, c, d, v, x, y, z}
    All the values in a matrix stored as 16.16 fixed point values, except for
    u, v and w, which are stored as 2.30 fixed point value.
    default = { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
    Composition Matrix:
     0 degree:  a = 0x00010000, b = 0, c = 0, d = 0x00010000 [ Landscape Record]
    90 degree: a = 0, b = 0x00010000, c = 0xFFFF0000, d = 0 [ Portrait Record]
   180 degree: a = 0xFFFF0000, b =0, c = 0; d= 0xFFFF0000
   270 degree: a = 0, b = 0xFFFF0000, c = 0x00010000, d = 0
   */

  /* Matrix element a[X00] -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&nX00,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed +=4;
  /* Matrix element b[X01] -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&nX01,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed +=4;
  /* Skip Matrix element u[X02] -- 4 bytes */
  video_fmt_mp4r_skip_data(context,4,VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;

  /* Matrix element c[X10] -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&nX10,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed +=4;
  /* Matrix element d[X11] -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&nX11,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed +=4;

  /* Skip Matrix element v[X12],x[X20],y[X21],z[X23] -- 16 bytes */
  video_fmt_mp4r_skip_data(context,16,VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 16;

  if( ( cDefaultVal == nX00 /*a*/) && ( 0 == nX01 /*b*/) &&
    ( 0 == nX10/*c*/) &&( cDefaultVal == nX11/*d*/))
  {
    stream_info->rotation_degrees = 0;
  }
  else if( ( 0 == nX00 /*a*/) && ( cDefaultVal == nX01/*b*/ ) &&
    ( -cDefaultVal == nX10 /*c*/) && ( 0 == nX11 /*d*/))
  {
    stream_info->rotation_degrees = 90;
  }
  else if( ( -cDefaultVal == nX00 /*a*/) && ( 0 == nX01 /*b*/)&&
    ( 0 == nX10 /*c*/) &&( -cDefaultVal == nX11 /*d*/))
  {
    stream_info->rotation_degrees = 180;
  }
  else if( ( 0== nX00 /*a*/) && ( -cDefaultVal == nX01 /*b*/) &&
    ( cDefaultVal == nX10 /*c*/) && ( 0 == nX11 /*d*/))
  {
    stream_info->rotation_degrees = 270;
  }
  else
  {
    stream_info->rotation_degrees = 0;
  }

  /* tkhd_width -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->tkhd_width,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed +=4;

  /* tkhd_height -- 4 bytes */
  video_fmt_mp4r_consume_data(context, (uint8 *)&stream_info->tkhd_height,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;
  stream_info->tkhd_origin_x = 0;
  stream_info->tkhd_origin_y = 0;
  //skip any bytes which are not consumed...
  if(ulDataConsumed != atom_stack_top->atom.size)
  {
    video_fmt_mp4r_skip_data ( context,
      atom_stack_top->atom.size- ulDataConsumed,
      VIDEO_FMT_MP4R_IN_BUFFER);
  }
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_tfhd

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'tfhd' atom, which contains a table
  of description common for all samples in all TRUN's.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_tfhd
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type        *context;
    video_fmt_mp4r_stream_type         *stream = 0;
    video_fmt_stream_info_type         *stream_info = 0;
    uint32                             index;
    video_fmt_track_frag_info_type     *track_info = NULL;
    uint32                             track_id;
    uint32                             tf_flag = 0;
    int64                              base_data_offset = 0;
    uint32                             default_sample_duration = 0;
    uint32                             default_sample_size = 0;
    uint32                             default_sample_flags = 0;
    uint32                             default_sample_description_index=0;

    UNUSED_PARAM(atom_struct_ptr);
    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;

    /* Read up to and including tf_flags track_id base data offset
    ** sample_description_index default_sample_duration default_sample_size
    ** default_sample_flags
    */
    if (!video_fmt_mp4r_read_buffer (context, 32, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
        return TRUE;
    }

    video_fmt_mp4r_skip_data (context, 1, VIDEO_FMT_MP4R_IN_BUFFER);

    video_fmt_mp4r_consume_data
        (context, (uint8 *) &tf_flag, 3,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);
    tf_flag &= 0x00FFFFFF;  /* take only three bytes */

    video_fmt_mp4r_consume_data
        (context, (uint8 *) &track_id, 4,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);

    if(!video_fmt_mp4r_check_valid_track(track_id,context))
    {
      video_fmt_mp4r_atom_pos_type *atom_stack_top
                  = &context->atom_stack [context->atom_stack_top - 1];

      video_fmt_mp4r_skip_data (context, (atom_stack_top->atom.size) -
                                         (uint32)(context->abs_pos  - atom_stack_top->atom.offset),
                                         VIDEO_FMT_MP4R_IN_BUFFER);

      /*Incrementing the fragment number for each track*/
      for(index = 0 ; index < context->num_streams; index++)
      {
        if((&context->stream_info[index] != NULL) &&
           (&context->stream_state[index] != NULL) )
        {
          context->stream_info[index].fragment_number = context->current_sequence_number;
        }
      }
    }
    else
    {
      /*Initialize stream_info for all the streams because
      * if the stream is missing in the fragment the stream_info
      * is not initialized
      */

      for(index = 0 ; index < context->num_streams; index++)
      {
        if((&context->stream_info[index] != NULL) &&
           (&context->stream_state[index] != NULL))
        {
          context->stream_info[index].fragment_number = context->current_sequence_number;
          if(context->stream_state[index].initialize_trun)
            context->stream_info[index].inter_frames = FALSE;
        }
      }

      /* get the track_info */
      for(index = 0 ; index < context->num_streams; index++)
      {
        if(context->stream_info[index].track_id == track_id)
        {
          stream_info = &context->stream_info [index];
          stream = &context->stream_state [index];
          context->current_track_id = track_id;
          break;
        }
      }
      if(stream_info)
      {
        track_info = &stream_info->track_frag_info;
      }

      /* get the base_data_offset if the tf_flags indicates that it is present*/
      if(tf_flag & TF_FLAG_BASE_DATA_OFFSET_MASK)
      {
        video_fmt_mp4r_consume_data
            (context, (uint8 *) &base_data_offset, 8,
            context->byte_swap_needed,
            VIDEO_FMT_MP4R_IN_BUFFER);
      }

      /* get the sample_description_index if the tf_flags indicates that it is present*/
      if(tf_flag & TF_FLAG_SAMPLE_DESCRIPTION_INDEX_MASK)
      {
        video_fmt_mp4r_consume_data
             (context, (uint8 *) &default_sample_description_index, 4,
               context->byte_swap_needed,
               VIDEO_FMT_MP4R_IN_BUFFER);
      }
      /*get default_sample_duration if the tf_flags indicates that it is present*/
      if(tf_flag & TF_FLAG_DEFAULT_SAMPLE_DURATION_MASK)
      {
        video_fmt_mp4r_consume_data
             (context, (uint8 *) &default_sample_duration, 4,
              context->byte_swap_needed,
              VIDEO_FMT_MP4R_IN_BUFFER);
      }

      /*get default_sample_size if the tf_flags indicates that it is present*/
      if(tf_flag & TF_FLAG_DEFAULT_SAMPLE_SIZE_MASK)
      {
        video_fmt_mp4r_consume_data
             (context, (uint8 *) &default_sample_size, 4,
              context->byte_swap_needed,
              VIDEO_FMT_MP4R_IN_BUFFER);
      }
      /*get default_sample_flags if the tf_flags indicates that it is present*/
      if(tf_flag & TF_FLAG_DEFAULT_SAMPLE_FLAGS_MASK)
      {
        video_fmt_mp4r_consume_data
             (context, (uint8 *) &default_sample_flags, 4,
              context->byte_swap_needed,
              VIDEO_FMT_MP4R_IN_BUFFER);
      }

      if(stream && stream_info && track_info)
      {
        trex_data_type default_sample_prop;
        memset(&default_sample_prop, 0, sizeof(trex_data_type));

        stream->tf_flags = tf_flag;

        for(index = 0 ; index< context->trex_count; index++)
        {
          if(stream_info->track_id == context->trex_data[index].track_id)
          {
            default_sample_prop = context->trex_data[index];
            break;
          }
        }

        if(base_data_offset != 0)
          stream->base_data_offset = base_data_offset;

        if(default_sample_duration != 0)
          stream->default_sample_duration = default_sample_duration;
        else
          stream->default_sample_duration = default_sample_prop.default_sample_duration;

        if(default_sample_size != 0)
          stream->default_sample_size = default_sample_size;
        else
          stream->default_sample_size = default_sample_prop.default_sample_size;

        if(default_sample_flags != 0)
          stream->default_sample_flags = default_sample_flags;
        else
          stream->default_sample_flags = default_sample_prop.default_sample_flags;

        /* Update inter frames flag based on default sample properties*/
        {
          uint8 sample_depends_on_flag = READ_BIT_FIELD(stream->default_sample_flags,
                                             I_VOP_POSITION, I_VOP_SIZE);
          /* if sample_depends_on_flag is 0, then use the is_non_sync_sample
             value to determine the Sync Sample.
             please refer Section 8.8.3.1 in the standard "ISO/IEC 14496-12. */

          uint8 is_non_sync_sample=READ_BIT_FIELD(stream->default_sample_flags,
                                    NON_SYNC_VOP_POSITION, NON_SYNC_VOP_SIZE);

          if( (FLAG_I_VOP == sample_depends_on_flag) ||
            ( (FLAG_UNKNOWN_VOP == sample_depends_on_flag) &&
              ( !is_non_sync_sample) ))
          {
            stream_info->inter_frames = TRUE;
          }
        }

        if(default_sample_description_index != 0)
          stream->default_sample_description_index = default_sample_description_index;
        else
          stream->default_sample_description_index = default_sample_prop.default_sample_description_index;

        track_info->sample_description_index = stream->default_sample_description_index ;

        /*Initialize trun for the first track fragment, then dont initialize
        * for subsequent same track fragments in the same fragment
        */
        if(stream->initialize_trun)
        {
          if(context->current_sequence_number == 1) /* First Fragment after Main Fragment */
          {
            track_info->first_frame = stream->main_fragment_frames;
            track_info->first_timestamp = stream->main_fragment_timestamp;
          }
          else
          {
            stream->last_fragment_frames = stream_info->frames - stream->main_fragment_frames;
            stream->last_fragment_bytes = stream_info->bytes - stream->main_fragment_bytes;
            stream->last_fragment_timestamp = stream_info->media_duration - stream->main_fragment_timestamp;
            track_info->first_frame = stream->last_fragment_frames + stream->main_fragment_frames;
            track_info->first_timestamp = stream->last_fragment_timestamp + stream->main_fragment_timestamp;
          }
          /*renitialize trun*/
          (void) memset (stream->trun, 0,
              sizeof(video_fmt_mp4r_sample_table_type)*VIDEO_FMT_MAX_RUNS);

          /*reinitialize trun cache*/
          (void) memset (stream->trun_cache, 0,
               sizeof(video_fmt_mp4r_trun_entry_type)*VIDEO_FMT_MP4R_TABLE_CACHE_SIZE);
          (void) memset (stream->trun_one_entry_cache, 0,
               sizeof(video_fmt_mp4r_trun_one_entry_type)*VIDEO_FMT_MP4R_TABLE_CACHE_SIZE);
          (void) memset (stream->trun_two_entry_cache, 0,
               sizeof(video_fmt_mp4r_trun_two_entry_type)*VIDEO_FMT_MP4R_TABLE_CACHE_SIZE);
          (void) memset (stream->trun_three_entry_cache, 0,
               sizeof(video_fmt_mp4r_trun_three_entry_type)*VIDEO_FMT_MP4R_TABLE_CACHE_SIZE);
          (void) memset (stream->trun_four_entry_cache, 0,
               sizeof(video_fmt_mp4r_trun_four_entry_type)*VIDEO_FMT_MP4R_TABLE_CACHE_SIZE);

          stream->current_trun = 0;
          stream->fragment_repositioned = FALSE;
          stream->trun_entry_count = 0;
          stream->initialize_trun = FALSE;
          /*re-initialize SAIO/SAIZ */
          stream->saio.current_table_pos = 0;
          stream->saiz.current_table_pos = 0;
          stream->saiz_table_relative_offset = 0;
          stream->saiz.cache_size = 0;
          stream->saio.cache_size = 0;

          /*reinitialize SAIO/SAIZ cache*/
          (void) memset (stream->saio_cache, 0,
            sizeof(video_fmt_mp4r_saio_entry_type)*VIDEO_FMT_MP4R_TABLE_CACHE_SIZE);
          (void) memset (stream->saiz_cache, 0,
            sizeof(video_fmt_mp4r_saiz_entry_type)*VIDEO_FMT_MP4R_TABLE_CACHE_SIZE);
        }
      }
    }//!video_fmt_mp4r_check_valid_track(track_id)

    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_trex

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'trex' atom, which contains a table
  of description common for all samples in all TRUN's.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_trex
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type        *context;
    uint32                             track_id;
    uint32                             sample_desc_index;
    uint32                             sample_duration;
    uint32                             sample_size;
    uint32                             sample_flags;

    UNUSED_PARAM(atom_struct_ptr);
    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;

    /* Read up to and including flags track_id default_sample_description_index
    ** default_sample_duration default_sample_size default_sample_flags
    */
    if (!video_fmt_mp4r_read_buffer (context, 24, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
        return TRUE;
    }

    video_fmt_mp4r_skip_data (context, 4, VIDEO_FMT_MP4R_IN_BUFFER);

    video_fmt_mp4r_consume_data
        (context, (uint8 *) &track_id, 4,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);

    video_fmt_mp4r_consume_data
        (context, (uint8 *) &sample_desc_index, 4,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);

    video_fmt_mp4r_consume_data
        (context, (uint8 *) &sample_duration, 4,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);

    video_fmt_mp4r_consume_data
        (context, (uint8 *) &sample_size, 4,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);

    video_fmt_mp4r_consume_data
        (context, (uint8 *) &sample_flags, 4,
         context->byte_swap_needed,
         VIDEO_FMT_MP4R_IN_BUFFER);

    /*Support when "mvex" atom parse when it is before the "trak" atom*/
     if(context->trex_count < VIDEO_FMT_MAX_MEDIA_STREAMS)
     {
        context->trex_data[context->trex_count].track_id = track_id;
        context->trex_data[context->trex_count].default_sample_description_index = sample_desc_index;
        context->trex_data[context->trex_count].default_sample_duration = sample_duration;
        context->trex_data[context->trex_count].default_sample_size = sample_size;
        context->trex_data[context->trex_count].default_sample_flags = sample_flags;
        context->trex_count++;
     }


    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_trak

DESCRIPTION

  This function parses a specific atom.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_trak
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type  *context;
  video_fmt_mp4r_stream_type   *stream;
  video_fmt_stream_info_type   *stream_info;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;

  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];

  /* if we are going to start parsing a new TRAK atom, then increase the
     number of tracks and initialize its variables */
  if(atom_stack_top->process_substate == VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT)
  {
    /* we only process VIDEO_FMT_MAX_MEDIA_STREAMS number of tracks */
    if(context->num_streams >= VIDEO_FMT_MAX_MEDIA_STREAMS)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "more tracks than supported (%lu).", context->num_streams);
      return video_fmt_mp4r_process_atom_skip(context_ptr, atom_struct_ptr);
    }

    /* Increment number of streams, and initialize new stream data. */
    ++context->num_streams;

    stream = &context->stream_state [context->num_streams - 1];
    stream_info = &context->stream_info [context->num_streams - 1];

    (void) memset (stream, 0, sizeof (*stream));
    stream_info->stream_num = context->num_streams - 1;

    /* Copy over byte-swap flag. */
    stream->byte_swap_needed = context->byte_swap_needed;
  }

  return video_fmt_mp4r_process_atom_empty(context_ptr, atom_struct_ptr);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_parse_atom

DESCRIPTION

  This function parses a specific atom.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_parse_atom
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type            *context;
    video_fmt_mp4r_atom_pos_type           *atom_stack_top;
    video_fmt_consume_atom_structure_type  *atom_struct;
    video_fmt_atom_field_type              *field;
    uint32                                 index, pos;
    video_fmt_atom_field_type              last_field;
    uint32                                 size_2read;

    /* Verify that the atom is not missing any key data. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    atom_struct = (video_fmt_consume_atom_structure_type *) atom_struct_ptr;

    /* Read fields, if there are any. */
    pos = 0;
    if (atom_struct->field_count > 0)
    {
        /* Calculate size (in bytes) that has to be buffered */
        last_field = atom_struct->fields[atom_struct->field_count-1];
        size_2read = last_field.offset + last_field.size;

        if (!video_fmt_mp4r_read_buffer (context, size_2read, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
        {
            return TRUE;
        }
    }

    /* Call atom-specific processing function. */
    atom_struct->dest_proc_func(context);

    /* Parse fields, if there are any. */
    if (atom_struct->field_count > 0)
    {
        for ( index = 0, field = atom_struct->fields;
              index < atom_struct->field_count;
              field = atom_struct->fields + index )
        {
            if ( field->offset == pos )
            {
              index++;
              if(field->dest)
              {
                video_fmt_mp4r_consume_data (context,
                                             field->dest,
                                             field->size,
                                             context->byte_swap_needed,
                                             VIDEO_FMT_MP4R_IN_BUFFER);
              }
              else
              {
                  /* Skip entry data. */
                  video_fmt_mp4r_skip_data (context, field->size, VIDEO_FMT_MP4R_IN_BUFFER);
              }
              pos = field->offset + field->size;
            }
            else
            {
                video_fmt_mp4r_skip_data (context, (field->offset - pos), VIDEO_FMT_MP4R_IN_BUFFER);
                pos = field->offset;
            }
        }
    }
    if ( atom_stack_top->atom.size > pos )
    {
        video_fmt_mp4r_skip_data (context, (atom_stack_top->atom.size - pos), VIDEO_FMT_MP4R_IN_BUFFER);
    }

    /* Done with this atom. */
    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_container

DESCRIPTION
  This function skips a container atom.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_container
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_consume_atom_structure_type *atom_struct;

    /* Branch according to processing substate. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    atom_struct = (video_fmt_consume_atom_structure_type *) atom_struct_ptr;
    //! Check if atom is MP4A or not and update amount of data that needs to be
    //! skipped based on sound version field.
    if( ( MP4A_TYPE == atom_stack_top->atom.type )  &&
        (atom_stack_top->atom.size > SOUND_VER_0_SIZE) &&
        (VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT ==
         atom_stack_top->process_substate)
       )
    {
      uint16 usSoundVer = 0;
      /* Read in enough data to decode entry data. */
      uint32 ulCount =
        (atom_stack_top->atom.size >= atom_struct->field_count)? \
                                      atom_struct->field_count: \
                                      atom_stack_top->atom.size;
      //! Do not try to read more than Buffer size in single attempt
      if (!video_fmt_mp4r_read_buffer (context,
                                       FILESOURCE_MIN(atom_stack_top->atom.size,
                                                      VIDEO_FMT_MP4R_BUFFER_SIZE),
                                       FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
      {
        return TRUE;
      }
      atom_struct->field_count = SOUND_VER_0_SIZE;
      /* Skip reserved data (6byte) and data ref index(2byte). */
      video_fmt_mp4r_skip_data (context, 8, VIDEO_FMT_MP4R_IN_BUFFER);
      /* Skip reserved data (6byte) and data ref index(2byte). */
      video_fmt_mp4r_consume_data (context, (uint8*)&usSoundVer, 2,
                                   context->byte_swap_needed,
                                   VIDEO_FMT_MP4R_IN_BUFFER);
      // Amount of Data that needs to be skipped in normal case
      if(1 == usSoundVer)
      {
        atom_struct->field_count = SOUND_VER_1_SIZE;
      }
      else if(2 == usSoundVer)
      {
        atom_struct->field_count = SOUND_VER_2_SIZE;
      }
      //! 10 bytes are already consumed, so update the amount of data that
      //! needs to be skipped accordingly
      atom_struct->field_count -= 10;
    }

    switch (atom_stack_top->process_substate)
    {
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
        if(atom_struct->dest_proc_func)
        {
            video_fmt_atom_field_type             *field;
            uint32 index, pos;
            video_fmt_atom_field_type             last_field;
            uint32                                size_2read;

            /* Read fields, if there are any. */
            pos = 0;
            if (atom_struct->field_count > 0)
            {
                /* Calculate size (in bytes) that has to be buffered */
                last_field = atom_struct->fields[atom_struct->field_count-1];
                size_2read = last_field.offset + last_field.size;

                if (!video_fmt_mp4r_read_buffer (context, size_2read, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
                {
                    return TRUE;
                }
            }

            /* Call atom-specific processing function. */
            atom_struct->dest_proc_func(context);

            /* Parse fields, if there are any. */
            if (atom_struct->field_count > 0)
            {
                for ( index = 0, pos = 0, field = atom_struct->fields;
                      index < atom_struct->field_count;
                      field = atom_struct->fields + index )
                {
                    if ( field->offset == pos )
                    {
                        index++;
                        if(field->dest)
                        {
                            /* read entry data. */
                            video_fmt_mp4r_consume_data
                                (context, field->dest, field->size,
                                 context->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
                        }
                        else
                        {
                            /* Skip entry data. */
                            video_fmt_mp4r_skip_data (context, field->size, VIDEO_FMT_MP4R_IN_BUFFER);
                        }
                        pos = field->offset + field->size;
                    }
                    else
                    {
                        video_fmt_mp4r_skip_data
                            (context, (field->offset - pos), VIDEO_FMT_MP4R_IN_BUFFER);
                        pos = field->offset;
                    }
                }
            }
        }
        else  /* just want to skip all the atom fields */
        {
          /* Read in enough data to decode entry data. */
          uint32 ulCount =
            (atom_stack_top->atom.size >= atom_struct->field_count)? \
                                          atom_struct->field_count: \
                                          atom_stack_top->atom.size;
          if (!video_fmt_mp4r_read_buffer (context, ulCount, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
          {
            return TRUE;
          }
          /* Skip entry data. */
          video_fmt_mp4r_skip_data (context, ulCount, VIDEO_FMT_MP4R_IN_BUFFER);
        }
        /* Move on to processing child atoms. */
        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
        break;

    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
        video_fmt_mp4r_finish_atom (context);
        break;
    default:
      break;
    }
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_stbl

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'stbl' atom, which contains the
  sample table for a track.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_stbl
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type   *stream;
    video_fmt_stream_info_type  *stream_info;
    boolean bMainFragmentHasFrames = TRUE;

    UNUSED_PARAM(atom_struct_ptr);
    /* Branch according to processing substate. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    if(context->num_streams > 0)
    {
       stream = &context->stream_state [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return FALSE;
    }
    stream_info = &context->stream_info [context->num_streams - 1];
    switch (atom_stack_top->process_substate)
    {
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
        /* Move on to processing child atoms. */
        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
        break;

    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE:
        /* Jump to current parsing position and continue parsing.  Note that
        ** a single parsing iteration can go through several parsing
        ** positions, there are no break statements between cases.
        **
        ** NOTE:  This state is used for 3 different parsing situations, as
        **        indicated by atom_stack_top->parse_pos, as follows:
        **            0-2 -- determining frame size for H.263 video
        **            3-4 -- counting total frames in track
        **            5-8 -- parsing MP3 header
        */
        switch (atom_stack_top->parse_pos)
        {
        case 0: /* Read chunk offset from 'stco' table. */
            /* Set up to read the first chunk offset entry in the 'stco' atom
            ** for this stream.
            */
            context->get_data_dst
                = (uint8 *) &atom_stack_top->parse_uint32 [0];
            context->get_data_src = stream->stco.file_offset;
            context->get_data_size = 4;
            context->get_data_needed = 4;
            context->get_data_read = 0;
            context->expect_eof = FALSE;
            context->state_next [0] = context->state;
            context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
            ++atom_stack_top->parse_pos;
            return TRUE;

        case 1: /* Read bytes 5-16 from first sample in first chunk. */
            /* Byte-reverse chunk offset value, if necessary. */
            video_fmt_mp4r_read_network_word
                (context, &atom_stack_top->parse_uint32 [0]);

            /* Set up to read the first chunk offset entry in the 'stco' atom
            ** for this stream.
            */
            context->get_data_dst
                = (uint8 *) &atom_stack_top->parse_uint32 [1];
            context->get_data_src = atom_stack_top->parse_uint32 [0] + 4;
            context->get_data_src_in_mdat = TRUE;
            context->get_data_size = 8;
            context->get_data_needed = 8;
            context->get_data_read = 0;
            context->expect_eof = FALSE;
            context->state_next [0] = context->state;
            context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
            ++atom_stack_top->parse_pos;
            return TRUE;

        case 2: /* Determine and set frame size. */
            context->get_data_src_in_mdat = FALSE;
            /* If no bytes are given: i.e. all the bytes read are 0x0 */
            if(!atom_stack_top->parse_uint32 [1] &&
               !atom_stack_top->parse_uint32 [2] &&
               context->cb_info.get_data.num_bytes == 8)
            {
                //Re-initialize the context here.
                /* Set up initial input buffer state. */
                context->in_buffer_which = 1;  /* will cause immediate switch */
                                            /* to buffer 0 on first read.  */
                context->in_buffer_size = 0;
                context->in_buffer_pos = 0;
                context->abs_pos = 0;
                context->mdat_present = FALSE;
                context->moov_present = FALSE;
                context->moof_present = FALSE;
                /* Set up initial atom stack state. */
                context->atom_stack_top = 0;
                /* Initialize stream information. */
                context->num_streams = 0;
                /* Initialize other state variables. */
                context->in_sample_description_atom = FALSE;
                context->cb_info.info.num_streams = 0;
                context->cb_info.info.abs_file_offset = 0;
                context->cb_info.info.server_data = context;
                context->state = VIDEO_FMT_MP4R_STATE_DECODE_ATOM_HEADER;

                /* Stop further parsing by indicating failure code to client. */
                context->callback_ptr(VIDEO_FMT_DATA_INCOMPLETE, context->client_data, NULL, NULL);
                return TRUE;
            }
            /* Parsing bits in a 32-bit word, big-endian order
               msb position=0, lsb position=32 */
            video_fmt_mp4r_read_network_word
                (context, &atom_stack_top->parse_uint32 [1]);
            video_fmt_mp4r_read_network_word
                (context, &atom_stack_top->parse_uint32 [2]);

            switch ( READ_BIT_FIELD(atom_stack_top->parse_uint32 [1], 3, 3))
            {
            case 1: /* sub-QCIF */
                stream_info->subinfo.video.width = 128;
                stream_info->subinfo.video.height = 96;
                break;
            case 2: /* QCIF */
                stream_info->subinfo.video.width = 176;
                stream_info->subinfo.video.height = 144;
                break;
            case 3: /* CIF */
                stream_info->subinfo.video.width = 352;
                stream_info->subinfo.video.height = 288;
                break;
            case 4: /* 4CIF */
                stream_info->subinfo.video.width = 704;
                stream_info->subinfo.video.height = 576;
                break;
            case 5: /* 16CIF */
                stream_info->subinfo.video.width = 1408;
                stream_info->subinfo.video.height = 1152;
                break;
            case 7: /* extended PTYPE - H.263 Profile 3 */
                if ( READ_BIT_FIELD(atom_stack_top->parse_uint32 [1], 6, 3) == 1 )
                {
                  /* UFEP */
                  switch ( READ_BIT_FIELD(atom_stack_top->parse_uint32 [1], 9, 3) )
                  {
                    case 1:   /* sub-QCIF */
                      stream_info->subinfo.video.width = 128;
                      stream_info->subinfo.video.height = 96;
                      break;
                    case 2:   /* QCIF */
                      stream_info->subinfo.video.width = 176;
                      stream_info->subinfo.video.height = 144;
                      break;
                    case 3:   /* CIF */
                      stream_info->subinfo.video.width = 352;
                      stream_info->subinfo.video.height = 288;
                      break;
                    case 4:   /* 4CIF */
                      stream_info->subinfo.video.width = 704;
                      stream_info->subinfo.video.height = 576;
                      break;
                    case 5:   /* 16CIF */
                      stream_info->subinfo.video.width = 1408;
                      stream_info->subinfo.video.height = 1152;
                      break;
                    case 6:   /* custom format - need to parse CPFMT */
                      {
                          /* CPFMT is in a different location depending on
                          ** the value of CPM bit (defines presence of PSBI
                          ** 2-bit field, or lack thereof).
                          */
                          uint32 cpfmt;
                          if (READ_BIT_FIELD(atom_stack_top->parse_uint32
                                             [2], 4, 1) == 1 )
                          {
                              cpfmt = READ_BIT_FIELD
                                  (atom_stack_top->parse_uint32 [2], 7, 23);
                          }
                          else
                          {
                              cpfmt = READ_BIT_FIELD
                                  (atom_stack_top->parse_uint32 [2], 5, 23);
                          }

                          /* Decode width/height from CPFMT. */
                          stream_info->subinfo.video.width
                              = (uint16) ((((cpfmt >> 10) & 0x1FF) + 1) * 4);
                          stream_info->subinfo.video.height
                              = (uint16) ((cpfmt & 0x1FF) * 4);
                      }
                      break;
                    default:
                      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                                  "process_atom_stbl: invalid H.263 profile 3"
                                  "source_format!");
                      video_fmt_mp4r_failure (context);
                      return TRUE;
                  }
                }
                else
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"process_atom_stbl:"
                              "invalid H.263 profile 3 source_format!");
                  video_fmt_mp4r_failure (context);
                  return TRUE;
                }
                break;
            default: /* reserved */
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"process_atom_stbl:"
                            "invalid H.263 source_format!");
                video_fmt_mp4r_failure (context);
                return TRUE;
            }
            atom_stack_top->process_substate
                = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE;
            break;

        case 3: /* Read next 'stts' table entry "sample_count" field. */
            /* Set up to read the next "sample_count" entry in the 'stts' atom
            ** for this stream.
            */
            context->get_data_dst = (uint8 *)
                &atom_stack_top->parse_uint32 [1];
            context->get_data_src = stream->stts.file_offset
                + atom_stack_top->parse_uint32 [0] * 8;
            context->get_data_size = 4;
            context->get_data_needed = 4;
            context->get_data_read = 0;
            context->expect_eof = FALSE;
            context->state_next [0] = context->state;
            context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
            ++atom_stack_top->parse_pos;
            return TRUE;

        case 4: /* Add "sample_count" field to frame total. */
            /* Byte-reverse "sample_count" value, if necessary. */
            video_fmt_mp4r_read_network_word
                (context, &atom_stack_top->parse_uint32 [1]);

            /* Add "sample_count" entry to total frame count. */
            stream_info->frames += atom_stack_top->parse_uint32 [1];
            stream->main_fragment_frames = stream_info->frames;
            if(stream_info->frames == 0)
            {
              /* if stts table is empty, this is an empty track */
              stream_info->type = VIDEO_FMT_STREAM_INVALID;
              /* now skip the atom */
              video_fmt_mp4r_skip_data( context, atom_stack_top->atom.size +
                                                 (uint32)(atom_stack_top->atom.offset
                                                 - context->abs_pos),
                                                 VIDEO_FMT_MP4R_IN_BUFFER );
              video_fmt_mp4r_finish_atom (context);
              return TRUE;
            }

            /* Count 'stts' table entry read.  If read the last entry, move on
            ** to the CHILDREN_DONE state to continue parsing.  Otherwise,
            ** return to the last parse position to parse the next table
            ** entry.
            */
            if (++(atom_stack_top->parse_uint32 [0]) >= stream->stts.table_size)
            {
                /* Recalculate total number of bytes in stream. */
                stream_info->bytes = stream_info->frames * stream->sample_size;
                stream->main_fragment_bytes = stream_info->bytes;
                atom_stack_top->process_substate
                    = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE;
            }
            else
            {
                --atom_stack_top->parse_pos;
            }
            break;

        case 5: /* Read chunk offset from 'stco' table. */
            /* Set up to read the first chunk offset entry in the 'stco' atom
            ** for this stream.
            */
            context->get_data_dst
                = (uint8 *) &atom_stack_top->parse_uint32 [0];
            context->get_data_src = stream->stco.file_offset;
            context->get_data_size = 4;
            context->get_data_needed = 4;
            context->get_data_read = 0;
            context->expect_eof = FALSE;
            context->state_next [0] = context->state;
            context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
            ++atom_stack_top->parse_pos;
            return TRUE;

        case 6: /* Read sample size from 'stsz' table. */
            /* Set up to read the first sample size entry in the 'stsz' atom
            ** for this stream.
            */
            context->get_data_dst
                = (uint8 *) &atom_stack_top->parse_uint32 [1];
            context->get_data_src = stream->stsz.file_offset;
            context->get_data_size = 4;
            context->get_data_needed = 4;
            context->get_data_read = 0;
            context->expect_eof = FALSE;
            context->state_next [0] = context->state;
            context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
            ++atom_stack_top->parse_pos;
            return TRUE;

        case 7: /* Set up to parse the first sample of the stream. */
            /* Byte-reverse chunk offset and sample size values,
            ** if necessary.
            */
            video_fmt_mp4r_read_network_word
                (context, &atom_stack_top->parse_uint32 [0]);
            video_fmt_mp4r_read_network_word
                (context, &atom_stack_top->parse_uint32 [1]);

            /* Set up to parse the first sample of the stream. */
            context->bs_offset = atom_stack_top->parse_uint32 [0];
            context->bs_size = atom_stack_top->parse_uint32 [1];
            context->bs_done = FALSE;
            video_fmt_bs_decode (video_fmt_mp4r_bs_callback,
                                 video_fmt_mp4r_mp3_bitstream,
                                 (uint32)(sizeof (video_fmt_mp4r_mp3_bitstream) - 1),
                                 context);
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 8: /* video/audio stream parsing */
            /* Continue parsing bitstream until done. */
            while ((context->bs_status != VIDEO_FMT_BS_DONE)
                && (context->bs_status != VIDEO_FMT_BS_FAILURE)
                && !context->bs_done)
            {
                context->bs_cont (context->bs_server_data);
            }

            /* Stop parser if it is still running. */
            if ((context->bs_status != VIDEO_FMT_BS_FAILURE)
                && (context->bs_status != VIDEO_FMT_BS_DONE))
            {
                context->bs_end (context->bs_server_data);
            }

            if ( context->bs_status == VIDEO_FMT_BS_FAILURE )
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"process_atom_stbl: failed"
                          " bit-stream processing");
              video_fmt_mp4r_failure (context);
              return TRUE;
            }

            /* Move back to children done state, if sampling frequency was
            ** found.
            */
            if (stream_info->subinfo.audio.sampling_frequency != 0)
            {
                atom_stack_top->process_substate
                    = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE;
            }

            /* Otherwise, flag an error. */
            else
            {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"process_atom_stbl:"
                            "invalid data!");
                video_fmt_mp4r_failure (context);
                return TRUE;
            }
            break;

        case 9: /* Read next 'stts' table entry "sample_delta" field. */
            /* Set up to read the next "sample_delta" entry in the 'stts' atom
            ** for this stream.
            */
            context->get_data_dst = (uint8 *)
                &atom_stack_top->parse_uint32 [1];
            context->get_data_src = stream->stts.file_offset + 4
                + atom_stack_top->parse_uint32 [0] * 8;
            context->get_data_size = 4;
            context->get_data_needed = 4;
            context->get_data_read = 0;
            context->expect_eof = FALSE;
            context->state_next [0] = context->state;
            context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;
            ++atom_stack_top->parse_pos;
            return TRUE;

        case 10: /* get sample duration from stts. */

            video_fmt_mp4r_read_network_word
                (context, &atom_stack_top->parse_uint32 [1]);

            /* Add "sample_count" entry to total frame count. */
            //if the sample duration is 0 set it to 1 to prevent
            //infinite loop e.g. BIFS and OD track
            if(atom_stack_top->parse_uint32 [1] == 0)
              stream->sample_duration = 1;
            else
              stream->sample_duration = atom_stack_top->parse_uint32 [1];
            stream_info->media_duration = stream_info->frames *
                                 stream->sample_duration;
            stream->main_fragment_timestamp = stream_info->media_duration;
            atom_stack_top->process_substate
                    = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE;
            break;
        }
        break;
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
        //! If the main fragment does not contain any valid frames.
        //! All the tables size shoudl be ZERO.
        if (!stream->stco.table_size && !stream->stts.table_size &&
            !stream->stsz.table_size)
        {
          bMainFragmentHasFrames = FALSE;
        }
        /* if stco table is empty, this is an empty track */
        if((context->isDashClip != TRUE) &&
           (TRUE == bMainFragmentHasFrames) )
        {
          if( stream->stco.table_size == 0 ||
              (stream->stts.table_size == 0 && stream_info->frames > 1 ) ||
              stream->stts.atom_size < stream->stts.table_size * 8 ||
              stream->stco.atom_size < stream->stco.table_size * 4 ||
              stream->stsz.table_size == 0 || (stream->sample_size == 0 &&
              stream->stsz.atom_size < stream->stsz.table_size * 4) )
          {
            stream_info->type = VIDEO_FMT_STREAM_INVALID;
            video_fmt_mp4r_finish_atom (context);
            return FALSE;
          }
          /* If no frame count was determined, the content has a bad
          ** "sample_count" field of the 'stsz' atom.  If this happens, count
          ** the total number of frames by summing the "sample-count" fields in
          ** the table in the 'stts' atom.
          */
          if (0 == stream_info->frames)
          {
              atom_stack_top->parse_pos = 3;
              atom_stack_top->parse_uint32 [0] = 0;
              atom_stack_top->process_substate
                  = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE;
              break;
          }

          /* If there is only one entry in the 'stts' atom, the stream is fixed
          ** rate.  Otherwise, it is variable rate.
          */
          stream_info->fixed_rate = ((stream->stts.table_size == 1) ? TRUE : FALSE);

          if((stream_info->fixed_rate) && (!stream->sample_duration))
          {
              atom_stack_top->parse_pos = 9;
              atom_stack_top->parse_uint32 [0] = 0;
              atom_stack_top->process_substate
                  = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE;
              break;
          }
          else
          {
              stream->main_fragment_timestamp = stream_info->media_duration;
          }

          /* If there is no 'stss' table, the stream definitely does not have
          ** any inter-frames.  Otherwise, no assumption can be made until the
          ** 'stss' entries are parsed.
          */
          stream_info->inter_frames
              = ((stream->stss.file_offset == 0) ? FALSE : TRUE);
          /* If there is no STSS atom then all the frames in the main fragment are
             Sync frames. So set inter_frames as True. */
          if(!stream->stss.table_size)
          {
            stream_info->inter_frames = TRUE;
          }
        }
        else
        {
          /* At this moment hard coded these values for set of DASH clips */
          stream->main_fragment_timestamp = 0;
          stream_info->media_duration     = 0;
          /* This info is required while providing media data to handle data under-run cases */
          stream->isDashClip = context->isDashClip;
        }
        /* If the track contains MP3 audio, and sampling frequency has not yet
        ** been determined, extract this and other information from the
        ** header of the MP3, located in the first sample of the track.
        */
        if ((stream_info->type == VIDEO_FMT_STREAM_AUDIO)
            && ((stream_info->subinfo.audio.format
                 == VIDEO_FMT_STREAM_AUDIO_MPEG1_L3)
                || (stream_info->subinfo.audio.format
                    == VIDEO_FMT_STREAM_AUDIO_MPEG2_L3))
            && (stream_info->subinfo.audio.sampling_frequency == 0))
        {
            atom_stack_top->parse_pos = 5;
            atom_stack_top->process_substate
                = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE;
            break;
        }

        /* If the track contained H.263 video, and the frame size has not yet
        ** been determined, extract the "source_format" field of the header of
        ** the first frame, by reading the fifth byte of the first chunk of
        ** the stream.  This field determines the frame size, according to
        ** table 6-25 in ISO/IEC 14496-2 - "Parameters Defined by
        ** source_format Field".
        */
        if ((stream_info->type == VIDEO_FMT_STREAM_VIDEO)
            && (stream_info->subinfo.video.format
                == VIDEO_FMT_STREAM_VIDEO_H263)
            && (stream_info->subinfo.video.width == 0))
        {
            atom_stack_top->parse_pos = 0;
            atom_stack_top->process_substate
                = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE;
        }

        /* Otherwise, finish up the atom. */
        else
        {
            video_fmt_mp4r_finish_atom (context);
        }
        break;
    default:
      break;
    }
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_text

DESCRIPTION
  This function is one of many functions which process MP4 file atoms

  This particular function processes the 'tx3g' atom, which contains
  timed text specific coding information.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_text
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type      *context;
    video_fmt_mp4r_atom_pos_type     *atom_stack_top;
    video_fmt_stream_info_type       *stream_info;
    video_fmt_mp4r_stream_type       *stream;
    video_fmt_stream_text_subtype    *text;

    UNUSED_PARAM(atom_struct_ptr);
    /* Set stream type and format to indicate 3GPP Timed Text */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    if( (context->num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS)||
        (context->num_streams == 0 ) )
    {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"process_atom_tx3g:"
                    " too many/few streams!");
        video_fmt_mp4r_failure (context);
        return TRUE;
    }
    stream = &context->stream_state [context->num_streams - 1];
    stream_info = &context->stream_info [context->num_streams - 1];
    text = &(stream_info->subinfo.text);

    if(!text->cur_atom_index )
    {
        stream_info->type = VIDEO_FMT_STREAM_TEXT;
        if( TX3G_TYPE == atom_stack_top->atom.type )
        {
          // OTI value for text decoder
          stream_info->dec_specific_info.obj_type = 0xFD;
          stream_info->subinfo.text.format = VIDEO_FMT_STREAM_TEXT_TIMEEDTEXT;
        }
        else if( STPP_TYPE == atom_stack_top->atom.type )
        {
          //OTI value for smptt-tt decoder not known
          stream_info->dec_specific_info.obj_type = 0xED;
          stream_info->subinfo.text.format =
            VIDEO_FMT_STREAM_TEXT_SMPTE_TIMED_TEXT;
        }
    }//if(!text->cur_atom_index )

    /* Send atom info to client, so client can parse the atom on its own */
    context->cb_info.text_atom.format = text->format;
    context->cb_info.text_atom.stream = context->num_streams - 1;
    context->cb_info.text_atom.offset = atom_stack_top->atom.offset;
    context->cb_info.text_atom.size = atom_stack_top->atom.size;
    context->cb_info.text_atom.cur_index = text->cur_atom_index;
    context->cb_info.text_atom.num_atoms = stream->stsd_entry_count;
    context->callback_ptr ( VIDEO_FMT_TEXT,
                            context->client_data,
                            &context->cb_info,
                            video_fmt_mp4r_end);

    text->cur_atom_index++;

    /* now skip the atom */
    video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size, VIDEO_FMT_MP4R_IN_BUFFER);

    /* Done with this atom. */
    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_avc1

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'avc1' atom, which contains
  advance video codec informations for H264 streams.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_avc1
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type   *context;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  /* video_fmt_consume_atom_structure_type *atom_struct; */
  video_fmt_stream_info_type    *stream_info;
  video_fmt_h264_dec_info_type  *h264_info;

  UNUSED_PARAM(atom_struct_ptr);
  /* Branch according to processing substate. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  if(!context)
  {
    return TRUE;
  }
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  /* atom_struct = (video_fmt_consume_atom_structure_type *) atom_struct_ptr;*/

  switch (atom_stack_top->process_substate)
  {
  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
      stream_info = &context->stream_info [context->num_streams - 1];
      h264_info = &stream_info->dec_specific_info.h264_info;
      if(h264_info->bAvcParseDone == TRUE)
      {
        //We have already parsed AVC atom, so just skip it.
        video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size, VIDEO_FMT_MP4R_IN_BUFFER);
        video_fmt_mp4r_finish_atom (context);
        break;
      }
      /*Initializing the structure variables for avcc/mvcc atom*/
      memset(&h264_info->avcc_alloc,0,sizeof(h264_info->avcc_alloc));
      memset(&h264_info->mvcc_alloc,0,sizeof(h264_info->mvcc_alloc));

      stream_info->type = VIDEO_FMT_STREAM_VIDEO;
      stream_info->subinfo.video.format = VIDEO_FMT_STREAM_VIDEO_H264;

      /* read required bytes */
      if (!video_fmt_mp4r_read_buffer
          (context, 78, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
      {
        return TRUE;
      }

      /* Skip redefined, reserved etc */
      video_fmt_mp4r_skip_data (context, 24, VIDEO_FMT_MP4R_IN_BUFFER);

      /* Read width and height */
      video_fmt_mp4r_consume_data(  context, (byte *)&stream_info->subinfo.video.width, 2,
                                    context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );
      video_fmt_mp4r_consume_data(  context, (byte *)&stream_info->subinfo.video.height, 2,
                                    context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

      /* read Hor/Ver resolutions */
      video_fmt_mp4r_consume_data(  context, (byte *)&h264_info->horiz_resolution,
                                    4, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );
      video_fmt_mp4r_consume_data(  context, (byte *)&h264_info->verti_resolution,
                                    4, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

      /* Skip reserved bytes */
      video_fmt_mp4r_skip_data (context, 4, VIDEO_FMT_MP4R_IN_BUFFER);

      /* read frame count */
      video_fmt_mp4r_consume_data(  context, (byte *)&h264_info->frame_count,
                                    2, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

      /* read compressor name and make it NULL terminated */
      video_fmt_mp4r_consume_data(  context, stream_info->dec_specific_info.info,
                                    32, FALSE, VIDEO_FMT_MP4R_IN_BUFFER );
      stream_info->dec_specific_info.info[32] = '\0';

      /* read depth */
      video_fmt_mp4r_consume_data(  context, (byte *)&h264_info->depth,
                                    2, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

      /* Skip pre-defined bytes */
      video_fmt_mp4r_skip_data (context, 2, VIDEO_FMT_MP4R_IN_BUFFER);

      /* Move on to processing child atoms. */
      atom_stack_top->process_substate
          = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
      break;

  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
      //We are done parsing AVC atom, so set bAvcParseDone to true to avoid memory allocation
      //when we reposition to first fragment and process AVC again.
      stream_info = &context->stream_info [context->num_streams - 1];
      h264_info = &stream_info->dec_specific_info.h264_info;
      if(h264_info)
      {
        h264_info->bAvcParseDone = TRUE;
      }
      video_fmt_mp4r_finish_atom (context);
      break;
  default:
    break;
  }
  return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_avcC

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'avcC' atom, which contains
  advance video codec informations for H264 streams.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_avcC
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type   *context;
  video_fmt_mp4r_atom_pos_type  *atom_stack_top;
  video_fmt_stream_info_type    *stream_info;
  video_fmt_h264_dec_info_type  *h264_info;
  video_fmt_alloc_type          alloc;
  video_fmt_alloc_type          alloc_atom;
  uint32 i=0,bytes_consumed;

  UNUSED_PARAM(atom_struct_ptr);
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  if(context->num_streams > 0)
  {
    stream_info = &context->stream_info [context->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (context);
    context->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }
  h264_info = &stream_info->dec_specific_info.h264_info;

  stream_info->type = VIDEO_FMT_STREAM_VIDEO;
  stream_info->subinfo.video.format = VIDEO_FMT_STREAM_VIDEO_H264;

  /*
  ** Allocate the memory dynamically for the amount of data to be read.
  ** Create a new dynamic buffer and then set the destination pointer
  ** to that dynamic buffer.
  */

  if(h264_info->avcc_alloc.memory_allocated == FALSE)
  {

    alloc_atom.size = atom_stack_top->atom.size;
    context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                            &alloc_atom, NULL );
    if(alloc_atom.ptr == NULL)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "memory not allocated to alloc_atom.ptr ");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    memset((byte*)alloc_atom.ptr, 0, alloc_atom.size);

    /* Invalidate the ping pong buffer */
    context->in_buffer_pos = 0;
    context->in_buffer_size = 0;

    context->get_data_dst = (uint8 *)alloc_atom.ptr;
    context->get_data_src = context->abs_pos;
    context->get_data_size = alloc_atom.size;
    context->get_data_needed = alloc_atom.size;
    context->get_data_read = 0;
    context->expect_eof = FALSE;
    context->state_next [0] = context->state;
    context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;


    /*Setting the structure variable indicating memory had been allocated*/
    h264_info->avcc_alloc.memory_ptr       = (uint8*)alloc_atom.ptr;
    h264_info->avcc_alloc.allocated_size   = alloc_atom.size;
    h264_info->avcc_alloc.memory_allocated = TRUE;

    return TRUE;
  }

  if(!h264_info->avcc_info)
  {
    alloc_atom.size = (uint32)sizeof(video_fmt_mp4r_avcc_info);
    context->callback_ptr( VIDEO_FMT_ALLOC, context->client_data,
                           &alloc_atom, NULL);
    if(alloc_atom.ptr == NULL)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "memory not allocated to alloc_atom.ptr ");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    memset((byte*)alloc_atom.ptr, 0, alloc_atom.size);
    h264_info->avcc_info = (video_fmt_mp4r_avcc_info*)alloc_atom.ptr;
  }
  if(h264_info->avcc_info)
  {
    /* track that we should not read more than atom size */
    bytes_consumed = 0;

    /* Read configuration version */
    if((bytes_consumed+1) <= h264_info->avcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                              &h264_info->avcc_info->configurationVersion,
                                               h264_info->avcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    /* Read profile code */
    if((bytes_consumed+1) <= h264_info->avcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                              &h264_info->avcc_info->AVCProfileIndication,
                                               h264_info->avcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
      stream_info->subinfo.video.profile = h264_info->avcc_info->AVCProfileIndication;
    }
    /*Read profile compatibility */
    if((bytes_consumed+1) <= h264_info->avcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                              &h264_info->avcc_info->profile_compatibility,
                                               h264_info->avcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    /*  Read level code */
    if((bytes_consumed+1) <= h264_info->avcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                              &h264_info->avcc_info->AVCLevelIndication,
                                               h264_info->avcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
      stream_info->subinfo.video.level = h264_info->avcc_info->AVCLevelIndication;
    }
    if((bytes_consumed+1) <= h264_info->avcc_alloc.allocated_size)
    {
      /* Read "NAL unit length minus one" field (only first 2 bits are valid) */
      bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                              &h264_info->avcc_info->len_minus_one,
                                               h264_info->avcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    h264_info->avcc_info->len_minus_one &= 0x03;

    if((bytes_consumed+1) <= h264_info->avcc_alloc.allocated_size)
    {
      /* Read number of sequence parameter sets (only first 5 bits are valid) */
      bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                              &h264_info->avcc_info->num_seq_param,
                                               h264_info->avcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    h264_info->avcc_info->num_seq_param &= 0x1F;

    if(h264_info->avcc_info->num_seq_param)
    {
      alloc.size = h264_info->avcc_info->num_seq_param *
                   (uint16)sizeof(video_fmt_nalu_data_type);
      context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                              &alloc, NULL );
      if(alloc.ptr == NULL)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "memory not allocated to alloc.ptr ");
        video_fmt_mp4r_failure (context);
        return TRUE;
      }
      h264_info->avcc_info->seq_param_set = (video_fmt_nalu_data_type*)alloc.ptr;
      memset(h264_info->avcc_info->seq_param_set, 0, alloc.size);

      for(i=0; (i<h264_info->avcc_info->num_seq_param) && (bytes_consumed < atom_stack_top->atom.size); i++)
      {
        /* Read seq parameter length */
        bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                        (byte *)&h264_info->avcc_info->seq_param_set[i].len,
                                                 h264_info->avcc_alloc.allocated_size,
                                                 bytes_consumed,2,
                                                 context->byte_swap_needed,
                                                 context);

        if(h264_info->avcc_info->seq_param_set[i].len)
        {
          alloc.size = h264_info->avcc_info->seq_param_set[i].len;
          context->callback_ptr( VIDEO_FMT_ALLOC, context->client_data,
                                 &alloc, NULL );
          if(alloc.ptr == NULL)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "memory not allocated to alloc.ptr ");
            video_fmt_mp4r_failure (context);
            return TRUE;
          }
          h264_info->avcc_info->seq_param_set[i].data = (byte*)alloc.ptr;
          memset(h264_info->avcc_info->seq_param_set[i].data, 0, alloc.size);
          /* Read seq parameter */
          if((bytes_consumed+h264_info->avcc_info->seq_param_set[i].len) <= h264_info->avcc_alloc.allocated_size)
          {
            bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                                     h264_info->avcc_info->seq_param_set[i].data,
                                                     h264_info->avcc_alloc.allocated_size,
                                                     bytes_consumed,
                                                     h264_info->avcc_info->seq_param_set[i].len,
                                                     FALSE,context);
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "error while parsing AVCC sequence parameters");
            video_fmt_mp4r_failure (context);
            return TRUE;
          }
        }
      }//for(i=0; (i<h264_info->num_seq_param) && (bytes_consumed < atom_stack_top->atom.size); i++)
    }//if(h264_info->num_seq_param)

    if((bytes_consumed+1) <= h264_info->avcc_alloc.allocated_size)
    {
      /* Read number of picture parameter sets */
      bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                              &h264_info->avcc_info->num_pic_param,
                                               h264_info->avcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    if(h264_info->avcc_info->num_pic_param)
    {
      alloc.size = h264_info->avcc_info->num_pic_param *
                   (uint16)sizeof(video_fmt_nalu_data_type);
      context->callback_ptr( VIDEO_FMT_ALLOC, context->client_data,
                             &alloc, NULL );
      if(alloc.ptr == NULL)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                     "memory not allocated to alloc.ptr ");
        video_fmt_mp4r_failure (context);
        return TRUE;
      }

      h264_info->avcc_info->pic_param_set = (video_fmt_nalu_data_type*)alloc.ptr;
      memset(h264_info->avcc_info->pic_param_set, 0, alloc.size);

      for(i=0; (i<h264_info->avcc_info->num_pic_param) && (bytes_consumed<atom_stack_top->atom.size); i++)
      {
        /* Read pic parameter length */
        bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                        (byte *)&h264_info->avcc_info->pic_param_set[i].len,
                                                 h264_info->avcc_alloc.allocated_size,
                                                 bytes_consumed,2,
                                                 context->byte_swap_needed,context);

        if(h264_info->avcc_info->pic_param_set[i].len)
        {
          alloc.size = h264_info->avcc_info->pic_param_set[i].len;
          context->callback_ptr( VIDEO_FMT_ALLOC, context->client_data,
                                 &alloc, NULL );
          if(alloc.ptr== NULL)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "memory not allocated to alloc.ptr ");
            video_fmt_mp4r_failure (context);
            return TRUE;
          }

          h264_info->avcc_info->pic_param_set[i].data = (byte*)alloc.ptr;
          memset(h264_info->avcc_info->pic_param_set[i].data, 0, alloc.size);
          if((bytes_consumed+h264_info->avcc_info->pic_param_set[i].len) <= h264_info->avcc_alloc.allocated_size)
          {
            /* Read seq parameter */
            bytes_consumed += video_fmt_consume_data(h264_info->avcc_alloc.memory_ptr,
                                                     h264_info->avcc_info->pic_param_set[i].data,
                                                     h264_info->avcc_alloc.allocated_size,
                                                     bytes_consumed,
                                                     h264_info->avcc_info->pic_param_set[i].len,
                                                     FALSE,context);
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "error while parsing AVCC picture parameters");
            video_fmt_mp4r_failure (context);
            return TRUE;
          }
        }//if(h264_info->pic_param_set[i].len)
      }//for(i=0; (i<h264_info->num_pic_param) && (bytes_consumed<atom_stack_top->atom.size); i++)
    }//if(h264_info->num_pic_param)
    //skip any bytes which are not consumed...
    if(bytes_consumed != atom_stack_top->atom.size)
    {
      video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size-bytes_consumed, VIDEO_FMT_MP4R_IN_BUFFER);
    }
  }//if(h264_info->avcc_info)
  /* now we are done with this atom */
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_vwid

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'avcC' atom, which contains
  advance video codec informations for H264 streams.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_vwid
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type   *context;
  video_fmt_mp4r_atom_pos_type  *atom_stack_top;
  video_fmt_stream_info_type    *stream_info;
  video_fmt_h264_dec_info_type  *h264_info;
  video_fmt_alloc_type          alloc;
  video_fmt_alloc_type          alloc_atom;
  uint32 i=0,j=0,bytes_consumed=0;
  uint8 temp_value1 = 0;
  uint16 temp_value2 = 0;
  uint32 temp_value3 = 0;

  UNUSED_PARAM(atom_struct_ptr);
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  if(context->num_streams > 0)
  {
     stream_info = &context->stream_info [context->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "process_atom_vwid context->num_streams is 0");
    video_fmt_mp4r_failure (context);
    context->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }
  h264_info = &stream_info->dec_specific_info.h264_info;

  /*
  ** Allocate the memory dynamically for the amount of data to be read.
  ** Create a new dynamic buffer and then set the destination pointer
  ** to that dynamic buffer.
  */

  if(h264_info->vwid_alloc.memory_allocated == FALSE)
  {

    alloc_atom.size = atom_stack_top->atom.size;
    context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                            &alloc_atom, NULL );
    if(alloc_atom.ptr == NULL)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "process_atom_vwid malloc failed for %lu ",
                    atom_stack_top->atom.size);
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    memset((byte*)alloc_atom.ptr, 0, alloc_atom.size);

    /* Invalidate the ping pong buffer */
    context->in_buffer_pos = 0;
    context->in_buffer_size = 0;

    context->get_data_dst = (uint8 *)alloc_atom.ptr;
    context->get_data_src = context->abs_pos;
    context->get_data_size = alloc_atom.size;
    context->get_data_needed = alloc_atom.size;
    context->get_data_read = 0;
    context->expect_eof = FALSE;
    context->state_next [0] = context->state;
    context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;

/*Setting the structure variable indicating memory had been allocated*/
    h264_info->vwid_alloc.memory_ptr = (uint8*)alloc_atom.ptr;
    h264_info->vwid_alloc.allocated_size = alloc_atom.size;
    h264_info->vwid_alloc.memory_allocated = TRUE;

    return TRUE;
  }
  if(!h264_info->vwid_info)
  {
    alloc_atom.size = (uint32)sizeof(video_fmt_vwid_info);
    context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                            &alloc_atom, NULL );
    if(alloc_atom.ptr == NULL)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "process_atom_vwid malloc failed for %lu ",
                    alloc_atom.size);
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    memset((byte*)alloc_atom.ptr, 0, alloc_atom.size);
    h264_info->vwid_info =(video_fmt_vwid_info *)alloc_atom.ptr;
  }
  if(h264_info->vwid_info)
  {
    /* make surre we do not read more than atom size */
    //vwid starts with version (1 bytes) and 3 bytes flag.
    //just consume these 4 bytes as we want to update the offset correctly.
    if((bytes_consumed+4) <= h264_info->vwid_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->vwid_alloc.memory_ptr,
                                               (uint8*)&temp_value3,
                                               h264_info->vwid_alloc.allocated_size,
                                               bytes_consumed,4,context->byte_swap_needed,context);
    }
    /* read min and max temporal*/
    if((bytes_consumed+1) <= h264_info->vwid_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->vwid_alloc.memory_ptr,
                                               &temp_value1,
                                               h264_info->vwid_alloc.allocated_size,
                                               bytes_consumed,1,context->byte_swap_needed,context);
      h264_info->vwid_info->min_temporal_id = (uint8)((temp_value1 & 0x38)>>3);
      h264_info->vwid_info->max_temporal_id = (uint8)(temp_value1 & 0x07);
    }
    /* read number of views*/
    if((bytes_consumed+2) <= h264_info->vwid_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->vwid_alloc.memory_ptr,
                                               (uint8*)&h264_info->vwid_info->num_views,
                                               h264_info->vwid_alloc.allocated_size,
                                               bytes_consumed,2,context->byte_swap_needed,context);
    }
    if(h264_info->vwid_info->num_views)
    {
      alloc.size = h264_info->vwid_info->num_views *
                   (uint16)sizeof(video_fmt_view_info);
      context->callback_ptr( VIDEO_FMT_ALLOC, context->client_data,
                             &alloc, NULL );
      if(alloc.ptr == NULL)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                     "process_atom_vwid malloc failed for %lu",
                      alloc.size);
        video_fmt_mp4r_failure (context);
        return TRUE;
      }
      h264_info->vwid_info->p_view_info = (video_fmt_view_info*)alloc.ptr;
      memset(h264_info->vwid_info->p_view_info, 0, alloc.size);

      for(i=0; (i<h264_info->vwid_info->num_views) && (bytes_consumed < atom_stack_top->atom.size); i++)
      {
        /* Read view id */
        bytes_consumed += video_fmt_consume_data(h264_info->vwid_alloc.memory_ptr,
                                                 (uint8*)&temp_value2,
                                                 h264_info->vwid_alloc.allocated_size,
                                                bytes_consumed,2,context->byte_swap_needed,context);
        h264_info->vwid_info->p_view_info[i].view_id = temp_value2 & 0x03FF;

        /* Read view order index */
        bytes_consumed += video_fmt_consume_data(h264_info->vwid_alloc.memory_ptr,
                                                 (uint8*)&temp_value2,
                                                 h264_info->vwid_alloc.allocated_size,
                                                bytes_consumed,2,context->byte_swap_needed,context);
        h264_info->vwid_info->p_view_info[i].view_order_index = temp_value2 & 0x03FF;

        /* Read base view type and number of ref views */
        bytes_consumed += video_fmt_consume_data(h264_info->vwid_alloc.memory_ptr,
                                                 (uint8*)&temp_value2,
                                                 h264_info->vwid_alloc.allocated_size,
                                                 bytes_consumed,2,context->byte_swap_needed,context);

        h264_info->vwid_info->p_view_info[i].base_view_type =
                                      (uint8)((temp_value2 & 0x0C00)>>10);
        h264_info->vwid_info->p_view_info[i].num_ref_views  =
                                      (uint8) (temp_value2 & 0x03FF);

        //check if there are additional ref. views pointed by this view
        if(h264_info->vwid_info->p_view_info[i].num_ref_views)
        {
          alloc.size = h264_info->vwid_info->p_view_info[i].num_ref_views << 2;
          context->callback_ptr( VIDEO_FMT_ALLOC, context->client_data,
                                 &alloc, NULL );
          if(alloc.ptr == NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                         "process_atom_vwid malloc failed for %lu", alloc.size);
            video_fmt_mp4r_failure (context);
            return TRUE;
          }
          h264_info->vwid_info->p_view_info[i].ref_view_id = (uint16*)alloc.ptr;
          memset(h264_info->vwid_info->p_view_info[i].ref_view_id, 0, alloc.size);
          //read each ref. view info now
          for(j = 0; (j < h264_info->vwid_info->p_view_info[i].num_ref_views) && (bytes_consumed < atom_stack_top->atom.size);j++)
          {
            /* Read ref view id */
            bytes_consumed += video_fmt_consume_data(h264_info->vwid_alloc.memory_ptr,
                                                     (uint8*)&temp_value2,
                                                      h264_info->vwid_alloc.allocated_size,
                                                     bytes_consumed,2,context->byte_swap_needed,context);
            h264_info->vwid_info->p_view_info[i].ref_view_id[j] = temp_value2 & 0x03FF;
          }
        }//if(h264_info->vwid_info->p_view_info[i].num_ref_views)
      }//for(i=0; (i<h264_info->vwid_info->num_views) && (bytes_consumed < atom_stack_top->atom.size); i++)
    }//if(h264_info->vwid_info->num_views)
    //skip any bytes which are not consumed...
    if(bytes_consumed != atom_stack_top->atom.size)
    {
      video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size-bytes_consumed, VIDEO_FMT_MP4R_IN_BUFFER);
    }
  }//if(h264_info->vwid_info)
  /* now we are done with this atom */
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_mvcC

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'mvcC' atom, which contains
  multi view coding information

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_mvcC
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type   *context;
  video_fmt_mp4r_atom_pos_type  *atom_stack_top;
  video_fmt_stream_info_type    *stream_info;
  video_fmt_h264_dec_info_type  *h264_info;
  video_fmt_alloc_type          alloc;
  video_fmt_alloc_type          alloc_atom;
  uint32 i=0,bytes_consumed;
  uint8 value_byte = 0;

  UNUSED_PARAM(atom_struct_ptr);
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  if(context->num_streams > 0)
  {
     stream_info = &context->stream_info [context->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "process_atom_mvcc context->num_streams is 0");
    video_fmt_mp4r_failure (context);
    context->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }
  h264_info = &stream_info->dec_specific_info.h264_info;

  /*
  ** Allocate the memory dynamically for the amount of data to be read.
  ** Create a new dynamic buffer and then set the destination pointer
  ** to that dynamic buffer.
  */

  if(h264_info->mvcc_alloc.memory_allocated == FALSE)
  {

    alloc_atom.size = atom_stack_top->atom.size;
    context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                            &alloc_atom, NULL );
    if(alloc_atom.ptr == NULL)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "process_atom_mvcc malloc failed for %lu", alloc_atom.size);
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    memset((byte*)alloc_atom.ptr, 0, alloc_atom.size);

    /* Invalidate the ping pong buffer */
    context->in_buffer_pos = 0;
    context->in_buffer_size = 0;

    context->get_data_dst = (uint8 *)alloc_atom.ptr;
    context->get_data_src = context->abs_pos;
    context->get_data_size = alloc_atom.size;
    context->get_data_needed = alloc_atom.size;
    context->get_data_read = 0;
    context->expect_eof = FALSE;
    context->state_next [0] = context->state;
    context->state = VIDEO_FMT_MP4R_STATE_GET_DATA;

    /*Setting the structure variable indicating memory had been allocated*/
    h264_info->mvcc_alloc.memory_ptr = (uint8*)alloc_atom.ptr;
    h264_info->mvcc_alloc.allocated_size = alloc_atom.size;
    h264_info->mvcc_alloc.memory_allocated = TRUE;

    return TRUE;
  }

  if(!h264_info->mvcc_info)
  {
    alloc_atom.size = (uint32)sizeof(video_fmt_mp4r_mvcc_info);
    context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                            &alloc_atom, NULL );
    if(alloc_atom.ptr == NULL)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "process_atom_mvcc malloc failed for %lu ", alloc_atom.size);
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    memset((byte*)alloc_atom.ptr, 0, alloc_atom.size);
    h264_info->mvcc_info = (video_fmt_mp4r_mvcc_info*)alloc_atom.ptr;
  }
  if(h264_info->mvcc_info)
  {
    /* track that we should not read more than atom size */
    bytes_consumed = 0;

    /* Read configuration version */
    if((bytes_consumed+1) <= h264_info->mvcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                               &h264_info->mvcc_info->configurationVersion,
                                               h264_info->mvcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    /* Read profile code */
    if((bytes_consumed+1) <= h264_info->mvcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                               &h264_info->mvcc_info->MVCProfileIndication,
                                               h264_info->mvcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    /*Read profile compatibility */
    if((bytes_consumed+1) <= h264_info->mvcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                               &h264_info->mvcc_info->profile_compatibility,
                                               h264_info->mvcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    /*  Read level code */
    if((bytes_consumed+1) <= h264_info->mvcc_alloc.allocated_size)
    {
      bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                               &h264_info->mvcc_info->MVCLevelIndication,
                                               h264_info->mvcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    if((bytes_consumed+1) <= h264_info->mvcc_alloc.allocated_size)
    {
      /* Read "NAL unit length minus one" field (only first 2 bits are valid) */
      bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                               &h264_info->mvcc_info->len_minus_one,
                                               h264_info->mvcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    value_byte = h264_info->mvcc_info->len_minus_one;

    /*bit(1) complete_representation;
     * bit(1) explicit_au_track;
     * bit(4) reserved = 1111b;
     * unsigned int(2) lengthSizeMinusOne;
    */
    h264_info->mvcc_info->len_minus_one = (uint8)(value_byte & 0x03);
    h264_info->mvcc_info->complete_representation = (uint8)((value_byte & 0x80)>>7);
    h264_info->mvcc_info->explicit_au_track = (uint8)( (value_byte & 0x40)>>6);

    if((bytes_consumed+1) <= h264_info->mvcc_alloc.allocated_size)
    {
      /* Read number of sequence parameter sets (only first 5 bits are valid) */
      bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                              &h264_info->mvcc_info->num_seq_param,
                                              h264_info->mvcc_alloc.allocated_size,
                                              bytes_consumed,1,FALSE,context);
    }
    h264_info->mvcc_info->num_seq_param &= 0x1F;

    if(h264_info->mvcc_info->num_seq_param)
    {
      alloc.size = h264_info->mvcc_info->num_seq_param *
                   (uint32)sizeof(video_fmt_nalu_data_type);
      context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                              &alloc, NULL );
      if(alloc.ptr == NULL)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                     "process_atom_mvcc malloc failed for %lu", alloc.size);
        video_fmt_mp4r_failure (context);
        return TRUE;
      }
      h264_info->mvcc_info->seq_param_set = (video_fmt_nalu_data_type*)alloc.ptr;
      memset(h264_info->mvcc_info->seq_param_set, 0, alloc.size);

      for(i=0; (i<h264_info->mvcc_info->num_seq_param) && (bytes_consumed < atom_stack_top->atom.size); i++)
      {
        /* Read seq parameter length */
        bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                                 (byte *)&h264_info->mvcc_info->seq_param_set[i].len,
                                                 h264_info->mvcc_alloc.allocated_size,
                                                bytes_consumed,2,context->byte_swap_needed,context);

        if(h264_info->mvcc_info->seq_param_set[i].len)
        {
          alloc.size = h264_info->mvcc_info->seq_param_set[i].len;
          context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                                  &alloc, NULL );
          if(alloc.ptr == NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                         "process_atom_mvcc malloc failed for %lu", alloc.size);
            video_fmt_mp4r_failure (context);
            return TRUE;
          }
          h264_info->mvcc_info->seq_param_set[i].data = (byte*)alloc.ptr;
          memset(h264_info->mvcc_info->seq_param_set[i].data, 0, alloc.size);
          /* Read seq parameter */
          if((bytes_consumed+h264_info->mvcc_info->seq_param_set[i].len) <= h264_info->mvcc_alloc.allocated_size)
          {
            bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                             h264_info->mvcc_info->seq_param_set[i].data,
                                             h264_info->mvcc_alloc.allocated_size,
                                                     bytes_consumed,h264_info->mvcc_info->seq_param_set[i].len,
                                                     FALSE,context);
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "error while parsing MVCC sequence parameters");
            video_fmt_mp4r_failure (context);
            return TRUE;
          }
        }
      }//for(i=0; (i<h264_info->mvcc_info->num_seq_param) && (bytes_consumed < atom_stack_top->atom.size); i++)
    }//if(h264_info->mvcc_info->num_seq_param)

    if((bytes_consumed+1) <= h264_info->mvcc_alloc.allocated_size)
    {
      /* Read number of picture parameter sets */
      bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                               &h264_info->mvcc_info->num_pic_param,
                                               h264_info->mvcc_alloc.allocated_size,
                                               bytes_consumed,1,FALSE,context);
    }
    if(h264_info->mvcc_info->num_pic_param)
    {
      alloc.size = h264_info->mvcc_info->num_pic_param *
                   (uint32)sizeof(video_fmt_nalu_data_type);
      context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                              &alloc, NULL );
      if(alloc.ptr == NULL)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                     "process_atom_mvcc malloc failed for %lu", alloc.size);
        video_fmt_mp4r_failure (context);
        return TRUE;
      }

      h264_info->mvcc_info->pic_param_set = (video_fmt_nalu_data_type*)alloc.ptr;
      memset(h264_info->mvcc_info->pic_param_set, 0, alloc.size);

      for(i=0; (i<h264_info->mvcc_info->num_pic_param) && (bytes_consumed<atom_stack_top->atom.size); i++)
      {
        /* Read pic parameter length */
        bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                                 (byte *)&h264_info->mvcc_info->pic_param_set[i].len,
                                                 h264_info->mvcc_alloc.allocated_size,
                                                 bytes_consumed,2,
                                                 context->byte_swap_needed,context);

        if(h264_info->mvcc_info->pic_param_set[i].len)
        {
          alloc.size = h264_info->mvcc_info->pic_param_set[i].len;
          context->callback_ptr ( VIDEO_FMT_ALLOC, context->client_data,
                                  &alloc, NULL );
          if(alloc.ptr== NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                         "process_atom_mvcc malloc failed for %lu", alloc.size);
            video_fmt_mp4r_failure (context);
            return TRUE;
          }

          h264_info->mvcc_info->pic_param_set[i].data = (byte*)alloc.ptr;
          memset(h264_info->mvcc_info->pic_param_set[i].data, 0, alloc.size);
          if((bytes_consumed+h264_info->mvcc_info->pic_param_set[i].len) <= h264_info->mvcc_alloc.allocated_size)
          {
            /* Read seq parameter */
            bytes_consumed += video_fmt_consume_data(h264_info->mvcc_alloc.memory_ptr,
                                                     h264_info->mvcc_info->pic_param_set[i].data,
                                                     h264_info->mvcc_alloc.allocated_size,
                                                     bytes_consumed,h264_info->mvcc_info->pic_param_set[i].len,
                                                     FALSE,context);
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "error while parsing MVCC picture parameters");
            video_fmt_mp4r_failure (context);
            return TRUE;
          }
        }//if(h264_info->mvcc_info->pic_param_set[i].len)
      }//for(i=0; (i<h264_info->mvcc_info->num_pic_param) && (bytes_consumed<atom_stack_top->atom.size); i++)
    }//if(h264_info->mvcc_info->num_pic_param)
    //skip any bytes which are not consumed...
    if(bytes_consumed != atom_stack_top->atom.size)
    {
      video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size-bytes_consumed, VIDEO_FMT_MP4R_IN_BUFFER);
    }
  }//if(h264_info->mvcc_info)
  /* now we are done with this atom */
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}
//
/* <EJECT> */

/*===========================================================================
FUNCTION  video_fmt_consume_data

DESCRIPTION
  This function removes the given number of bytes from the dynamic
  allocate buffers, storing the data in the given destination buffer, and
  updating the state.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
int video_fmt_consume_data
(
 uint8 *source,
 uint8 *destination,
 uint32 size,
 uint32 readpos,
 uint32 readsize,
 boolean  byte_reverse,
 video_fmt_mp4r_context_type   *context
 )
{
  int index;
  if (context == NULL)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                " Null Context ");
    return 0;
  }
  if (source == NULL || destination == NULL)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "video_fmt_consume_data: source or destination \
                 or context are NULL, context %p",
                 context);
    video_fmt_mp4r_failure(context);
    return 0;
  }
  else if((readpos + readsize) > size)
  {
    MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_FATAL,
      "video_fmt_consume_data:Read is outside input buffer size. \
      readpos %d, readsize %d, buf size %d, abs_pos %llu",
      readpos, readsize, size, context->abs_pos);
    video_fmt_mp4r_failure(context);
    return 0;
  }

  if (byte_reverse)
  {
    for (index = 0; index < (int)readsize; ++index)
    {
      destination [index] = source[readpos + readsize - index - 1];
    }
  }
  else
  {
    memcpy (destination, source + readpos, readsize);
  }
  context->abs_pos += readsize;
  return readsize;
}
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_btrt

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'btrt' atom, which contains
  advance video codec informations for H264 streams.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_btrt
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type   *context;
  video_fmt_mp4r_atom_pos_type  *atom_stack_top;
  video_fmt_stream_info_type    *stream_info;

  UNUSED_PARAM(atom_struct_ptr);
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  if(context->num_streams > 0)
  {
     stream_info = &context->stream_info [context->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (context);
    context->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }

  /* read required bytes */
  if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }

  /* Read decode buffer size */
  video_fmt_mp4r_consume_data( context, (byte *)&stream_info->dec_specific_info.max_buffer_size,
                               4, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

  /* Read max bit rate */
  video_fmt_mp4r_consume_data( context, (byte *)&stream_info->dec_specific_info.maxbitrate,
                               4, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

  /* Read avg bit rate */
  video_fmt_mp4r_consume_data( context, (byte *)&stream_info->dec_specific_info.avgbitrate,
                               4, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

  /* now we are done with this atom */
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}


/* <EJECT> */

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_bitr

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'bitr' atom, which contains
  advance video codec informations for H263 streams.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_bitr
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type   *context;
    video_fmt_mp4r_atom_pos_type  *atom_stack_top;
    video_fmt_stream_info_type    *stream_info;

    UNUSED_PARAM(atom_struct_ptr);
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    if(context->num_streams > 0)
    {
       stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
      video_fmt_mp4r_failure (context);
      context->state = VIDEO_FMT_MP4R_STATE_INVALID;
      return FALSE;
    }

    /* read required bytes */
    if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
      return TRUE;
    }

    /* Read max bit rate */
    video_fmt_mp4r_consume_data( context, (byte *)&stream_info->dec_specific_info.maxbitrate,
                                 4, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

    /* Read avg bit rate */
    video_fmt_mp4r_consume_data( context, (byte *)&stream_info->dec_specific_info.avgbitrate,
                                 4, context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER );

    /* now we are done with this atom */
    video_fmt_mp4r_finish_atom (context);
    return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_esds

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'esds' atom, which contains an
  elementary stream descriptor for a range of media samples.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_esds
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type   *stream;
    video_fmt_stream_info_type  *stream_info;
    uint8                       temp8;
    uint8                       objectType;
    uint8                       streamType;
    const char                  *bitstream;
    uint32                      bitstream_size;

    UNUSED_PARAM(atom_struct_ptr);
    /* Branch according to processing substate. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 2];
    if (context->num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "process_atom_esds: too many streams!");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }

    if(atom_stack_top->atom.type != MP4A_TYPE &&
       atom_stack_top->atom.type != MP4S_TYPE &&
       atom_stack_top->atom.type != MP4V_TYPE &&
       atom_stack_top->atom.type != ENCV_TYPE &&
       atom_stack_top->atom.type != WAVE_TYPE &&
       atom_stack_top->atom.type != ENCA_TYPE)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "process_atom_esds: Wrong parent atom");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }

    if(context->num_streams == 0)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "process_atom_esds: Stream descriptor without track");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    stream = &context->stream_state [context->num_streams - 1];
    stream_info = &context->stream_info [context->num_streams - 1];
    switch (atom_stack_top->process_substate)
    {
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
        /* Skip full atom header and descriptor tag field. */
        video_fmt_mp4r_skip_data (context, 5, VIDEO_FMT_MP4R_IN_BUFFER);

        /* Start parsing the rest of the atom, starting with the descriptor
        ** size.
        */
        atom_stack_top->parse_uint32 [0] = 0;
        atom_stack_top->parse_pos = 0;
        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE;

        /* Fall-through to next case is intentional. */

    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE:
        /* Jump to current parsing position and continue parsing.  Note that
        ** a single parsing iteration can go through several parsing
        ** positions, there are no break statements between cases.
        */
        switch (atom_stack_top->parse_pos)
        {
        case 0: /* descriptor size through ES_ID field */
            /* Read size byte. */
            if (!video_fmt_mp4r_read_buffer (context, 1, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
                return TRUE;
            }
            video_fmt_mp4r_consume_data (context, &temp8, 1, FALSE,
                                         VIDEO_FMT_MP4R_IN_BUFFER);

            /* Accumulate size bits. */
            atom_stack_top->parse_uint32 [0] <<= 7;
            atom_stack_top->parse_uint32 [0] |= (((uint32) temp8) & 0x7f);

            /* Continue in this state if the "more" bit is set. */
            if ((temp8 & 0x80) != 0)
            {
                break;
            }

            /* Verify size of descriptor is valid and move on to decode the
            ** rest of the descriptor if it is okay.
            */
            if (atom_stack_top->parse_uint32 [0] > atom_stack_top->atom.size
                + atom_stack_top->atom.offset - context->abs_pos)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                          "process_atom_esds: invalid descriptor size!");
              video_fmt_mp4r_failure (context);
              return TRUE;
            }

            /* Skip the ES_ID field. */
            atom_stack_top->parse_uint32 [1] = (uint32)context->abs_pos;
            video_fmt_mp4r_skip_data (context, 2, VIDEO_FMT_MP4R_IN_BUFFER);
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 1: /* streamDependenceFlag through dependsOn_ES_ID field */
            /* Read the next byte, which conains flags and the stream priority
            ** field.
            */
            if (!video_fmt_mp4r_read_buffer (context, 1, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
                return TRUE;
            }
            video_fmt_mp4r_consume_data (context, &temp8, 1, FALSE,
                                         VIDEO_FMT_MP4R_IN_BUFFER);

            /* Cache URL and OCRstream flags. */
            atom_stack_top->parse_flags [0] = ((temp8 >> 6) & 1);
            atom_stack_top->parse_flags [1] = ((temp8 >> 5) & 1);

            /* Skip stream dependency if stream dependence flag is set. */
            if (((temp8 >> 7) & 1) != 0)
            {
                video_fmt_mp4r_skip_data (context, 2, VIDEO_FMT_MP4R_IN_BUFFER);
            }
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 2: /* URL length field through decConfigDescr tag */
            /* Read URL length field and skip URL if URL flag is set. */
            if (atom_stack_top->parse_flags [0])
            {
                /* Read URL length field. */
                if (!video_fmt_mp4r_read_buffer (context, 1, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
                {
                    return TRUE;
                }
                video_fmt_mp4r_consume_data
                    (context, &temp8, 1, FALSE,
                     VIDEO_FMT_MP4R_IN_BUFFER);

                /* Skip URL field. */
                video_fmt_mp4r_skip_data (context, temp8, VIDEO_FMT_MP4R_IN_BUFFER);
            }

            /* Skip OCR_ES_Id field if OCRstream is set. */
            if (atom_stack_top->parse_flags [1])
            {
                video_fmt_mp4r_skip_data (context, 2, VIDEO_FMT_MP4R_IN_BUFFER);
            }

            /* Skip decConfigDescr tag field. */
            video_fmt_mp4r_skip_data (context, 1, VIDEO_FMT_MP4R_IN_BUFFER);

            /* Set up to read decConfigDescr size. */
            atom_stack_top->parse_uint32 [2] = 0;
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 3: /* decConfigDescr size */
            /* Read size byte. */
            if (!video_fmt_mp4r_read_buffer (context, 1, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
                return TRUE;
            }
            video_fmt_mp4r_consume_data (context, &temp8, 1, FALSE,
                                         VIDEO_FMT_MP4R_IN_BUFFER);

            /* Accumulate size bits. */
            atom_stack_top->parse_uint32 [2] <<= 7;
            atom_stack_top->parse_uint32 [2] |= (((uint32) temp8) & 0x7f);

            /* Continue in this state if the "more" bit is set. */
            if ((temp8 & 0x80) != 0)
            {
                break;
            }

            /* Verify size of descriptor is valid and move on to decode the
            ** rest of the descriptor if it is okay.
            */
            if ( atom_stack_top->parse_uint32 [2]> atom_stack_top->parse_uint32 [0]
            + atom_stack_top->parse_uint32 [1] - context->abs_pos)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                          "process_atom_esds: invalid descriptor size!");
            }
            atom_stack_top->parse_uint32 [3] = (uint32)context->abs_pos;
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 4: /* objectTypeIndication field */
            /* Read up to decoder specific info descriptor. */
            if (!video_fmt_mp4r_read_buffer (context, 13, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
                return TRUE;
            }

            /* Read object type indication and stream type fields. */
            video_fmt_mp4r_consume_data (context, &objectType, 1, FALSE,
                                         VIDEO_FMT_MP4R_IN_BUFFER);
            video_fmt_mp4r_consume_data (context, &streamType, 1, FALSE,
                                         VIDEO_FMT_MP4R_IN_BUFFER);
            streamType = (uint8)(streamType >>2);

            stream_info->dec_specific_info.obj_type = objectType;
            stream_info->dec_specific_info.stream_type = streamType;

            /* Interpret stream type field. */
            switch (streamType)
            {
            case 0x04:
                stream_info->type = VIDEO_FMT_STREAM_VIDEO;
                stream_info->subinfo.video.width
                                  = (uint16)(stream_info->tkhd_width >> 16);
                stream_info->subinfo.video.height
                                  = (uint16)(stream_info->tkhd_height >> 16);
                stream_info->subinfo.video.frame_rate = 0.0;
                switch (objectType)
                {
                case 0x20:
                    stream_info->subinfo.video.format
                        = VIDEO_FMT_STREAM_VIDEO_MPEG4;
                    break;
                case 0x6C:
                    stream_info->subinfo.video.format
                        = VIDEO_FMT_STREAM_VIDEO_JPEG;
                    break;

                case 0xE2:
                    stream_info->subinfo.video.format
                        = VIDEO_FMT_STREAM_VIDEO_BMP;
                    break;
                case 0xFE:
                    stream_info->subinfo.video.format
                        = VIDEO_FMT_STREAM_VIDEO_STILL_IMAGE;
                    break;

                default:
                    stream_info->subinfo.video.format
                        = VIDEO_FMT_STREAM_VIDEO_UNK;
                    break;
                }
                break;

            case 0x05:
                stream_info->type = VIDEO_FMT_STREAM_AUDIO;
                switch (objectType)
                {
                case 0x40:
                    stream_info->subinfo.audio.format
                        = VIDEO_FMT_STREAM_AUDIO_MPEG4_AAC;
                    break;

                case 0x67:
                    stream_info->subinfo.audio.format
                        = VIDEO_FMT_STREAM_AUDIO_MPEG2_AAC;
                    break;

                case 0x69:  /* ISO/IEC 13818-3 -- MPEG-2 audio */
                    stream_info->subinfo.audio.format
                        = VIDEO_FMT_STREAM_AUDIO_MPEG2_L3;
                  break;

                case 0x6B:  /* ISO/IEC 11172-3 -- MPEG-1 audio */
                    stream_info->subinfo.audio.format
                        = VIDEO_FMT_STREAM_AUDIO_MPEG1_L3;
                  break;

                case 0xD1:
                    stream_info->subinfo.audio.format
                        = VIDEO_FMT_STREAM_AUDIO_EVRC_PV;
                    stream_info->subinfo.audio.sampling_frequency = 8000;
                    stream_info->subinfo.audio.num_channels = 1;
                    break;

                case 0xD2:
                case 0xE1:
                    stream_info->subinfo.audio.format
                        = VIDEO_FMT_STREAM_AUDIO_QCELP13K_FULL;
                    stream_info->subinfo.audio.sampling_frequency = 8000;
                    stream_info->subinfo.audio.num_channels = 1;
                    break;

                default:
                    stream_info->subinfo.audio.format
                        = VIDEO_FMT_STREAM_AUDIO_UNK;
                }
                break;

            case 0x03:
                stream_info->type = VIDEO_FMT_STREAM_DATA;
                stream_info->subinfo.data.format = VIDEO_FMT_STREAM_DATA_BIFS;
                break;

            case 0x01:
      switch (objectType)
      {
        case 0x20:
          stream_info->type = VIDEO_FMT_STREAM_VIDEO;
          stream_info->subinfo.video.width = 0;
          stream_info->subinfo.video.height = 0;
          stream_info->subinfo.video.frame_rate = 0.0;
          stream_info->subinfo.video.format = VIDEO_FMT_STREAM_VIDEO_MPEG4;
          break;
        default:
                stream_info->type = VIDEO_FMT_STREAM_DATA;
                stream_info->subinfo.data.format = VIDEO_FMT_STREAM_DATA_OD;
                break;
      }
      break;
            case 0x20:
                switch (objectType)
                {
                case 0x6C:
                    stream_info->type = VIDEO_FMT_STREAM_VIDEO;
                    stream_info->subinfo.video.width = 0;
                    stream_info->subinfo.video.height = 0;
                    stream_info->subinfo.video.frame_rate = 0.0;
                    stream_info->subinfo.video.format
                        = VIDEO_FMT_STREAM_VIDEO_JPEG;
                    break;
                case 0xd0:
                    stream_info->type = VIDEO_FMT_STREAM_TEXT;
                    break;
                default:
                    stream_info->type = VIDEO_FMT_STREAM_UNK;
                    break;
                }
                break;

            default:
                stream_info->type = VIDEO_FMT_STREAM_UNK;
            }

            video_fmt_mp4r_consume_data
                (context, (uint8 *)
                 &stream_info->dec_specific_info.max_buffer_size,
                 3, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);

            /* retain only the 3 LSBytes */
            stream_info->dec_specific_info.max_buffer_size &= 0x00FFFFFF;

            video_fmt_mp4r_consume_data
                (context,
                 (uint8 *) &stream_info->dec_specific_info.maxbitrate,
                 4, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);

            video_fmt_mp4r_consume_data
                (context,
                 (uint8 *) &stream_info->dec_specific_info.avgbitrate,
                 4, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 5: /* decSpecificInfo tag */
            /* If at the end of the atom, break out early. */
            if (context->abs_pos >= atom_stack_top->atom.offset
                + atom_stack_top->atom.size)
            {
                video_fmt_mp4r_finish_atom (context);
                return FALSE;
            }

            /* Read descriptor tag. */
            if (!video_fmt_mp4r_read_buffer (context, 1, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
                return TRUE;
            }
            video_fmt_mp4r_consume_data (context, &temp8, 1, FALSE,
                                         VIDEO_FMT_MP4R_IN_BUFFER);

            /* Break out early if this is not a decSpecificInfo descriptor. */
            if (temp8 != 0x05)
            {
                /*Check if we have a sampling frequency. If we dont, assign
                  the media timescale to the sampling frequency. This is ok
                  since the standard says that if there is an mp4a atom,
                  the sampling frequency must be equal to the media timescale.
                  If there is still a problem, videofmt will still throw an
                   error.
                 */

                if(stream_info->subinfo.audio.sampling_frequency == 0)
                {
                   stream_info->subinfo.audio.sampling_frequency =
                      stream_info->media_timescale;
                }

                video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size
                                          + atom_stack_top->atom.offset
                                          - context->abs_pos, VIDEO_FMT_MP4R_IN_BUFFER);
                video_fmt_mp4r_finish_atom (context);
                return FALSE;
            }

            /* Set up to read decSpecificInfo size. */
            atom_stack_top->parse_uint32 [4] = 0;
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 6: /* decSpecificInfo size */
            /* Read size byte. */
            if (!video_fmt_mp4r_read_buffer (context, 1, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
                return TRUE;
            }
            video_fmt_mp4r_consume_data (context, &temp8, 1, FALSE,
                                         VIDEO_FMT_MP4R_IN_BUFFER);

            /* Accumulate size bits. */
            atom_stack_top->parse_uint32 [4] <<= 7;
            atom_stack_top->parse_uint32 [4] |= (((uint32) temp8) & 0x7f);

            /* Continue in this state if the "more" bit is set. */
            if ((temp8 & 0x80) != 0)
            {
                break;
            }

            /* Verify size of descriptor is valid and move on to decode the
            ** rest of the descriptor if it is okay.
            */
            if ( atom_stack_top->parse_uint32 [4] > atom_stack_top->parse_uint32 [2]
            + atom_stack_top->parse_uint32 [3] - context->abs_pos)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                          "process_atom_esds: invalid descriptor size!");
            }

            /* Store header location and size.  Store a copy in both the
            ** stream state and stream information structures.
            */
            stream->header_location = (uint32)context->abs_pos;
            stream->header = atom_stack_top->parse_uint32 [4];
            stream_info->header = atom_stack_top->parse_uint32 [4];

            /* Set up to read the decSpecificInfo  */
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 7: /* decSpecificInfo */
            /* Read and cache only up to the maximum amount we can store. */
            if (!video_fmt_mp4r_read_buffer
                (context, FILESOURCE_MIN (stream_info->header,
                               VIDEO_FMT_MAX_DEC_SPECIFIC_INFO), FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
            {
                return TRUE;
            }

            /* SKT-MOD JPEG only */
            if( (stream_info->type==VIDEO_FMT_STREAM_VIDEO) &&
                ( (stream_info->subinfo.video.format==VIDEO_FMT_STREAM_VIDEO_JPEG) ||
                  (stream_info->subinfo.video.format==VIDEO_FMT_STREAM_VIDEO_BMP) ) )
            {
                /* skip headerlength */
                video_fmt_mp4r_skip_data(context, 2, VIDEO_FMT_MP4R_IN_BUFFER);

                /* read frame width */
                video_fmt_mp4r_consume_data(context, (byte *)&stream_info->subinfo.video.width, 2,
                                            context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER);

                /* read frame height */
                video_fmt_mp4r_consume_data(context, (byte *)&stream_info->subinfo.video.height, 2,
                                            context->byte_swap_needed, VIDEO_FMT_MP4R_IN_BUFFER);

                /* skip rest of the atom and finish */
                video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size
                                          + atom_stack_top->atom.offset
                                          - context->abs_pos, VIDEO_FMT_MP4R_IN_BUFFER);
                video_fmt_mp4r_finish_atom (context);
                return FALSE;
            }

            video_fmt_mp4r_consume_data
                (context,
                 &stream_info->dec_specific_info.info[0],
                 FILESOURCE_MIN (stream_info->header,
                      VIDEO_FMT_MAX_DEC_SPECIFIC_INFO),
                 FALSE,
                 VIDEO_FMT_MP4R_IN_BUFFER);

            /* If there is more we can store, skip it. */
            if (stream_info->header > VIDEO_FMT_MAX_DEC_SPECIFIC_INFO)
            {
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                           "VOL size=%lu, Can Store=%u, skipping rest",
                           stream_info->header, VIDEO_FMT_MAX_DEC_SPECIFIC_INFO);
              video_fmt_mp4r_skip_data(context,
                                       stream_info->header- VIDEO_FMT_MAX_DEC_SPECIFIC_INFO,
                                       VIDEO_FMT_MP4R_IN_BUFFER);
            }

            /* Break out early if this is not an MPEG_4 video or MPEG4 audio
            ** or MPEG2 Audio stream.
            */
            if (((stream_info->type != VIDEO_FMT_STREAM_VIDEO)
                 || (stream_info->subinfo.video.format
                     != VIDEO_FMT_STREAM_VIDEO_MPEG4))
                && ((stream_info->type != VIDEO_FMT_STREAM_AUDIO)
                    || !((stream_info->subinfo.audio.format
                        == VIDEO_FMT_STREAM_AUDIO_MPEG4_AAC)||
                       (stream_info->subinfo.audio.format
                        == VIDEO_FMT_STREAM_AUDIO_MPEG2_AAC))))
            {
                video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size
                                          + atom_stack_top->atom.offset
                                          - context->abs_pos, VIDEO_FMT_MP4R_IN_BUFFER);
                video_fmt_mp4r_finish_atom (context);
                return FALSE;
            }

            /* Set up to parse the contents as a video or audio stream. */
            context->bs_offset = (uint32)(context->abs_pos - stream_info->header);
            context->bs_size = stream_info->header;
            context->bs_done = FALSE;
            context->bs_short_header = FALSE;
            bitstream = ((stream_info->type == VIDEO_FMT_STREAM_VIDEO)
                         ? video_fmt_mp4r_m4v_bitstream
                         : video_fmt_mp4r_m4a_bitstream);
            bitstream_size = (uint32)((stream_info->type == VIDEO_FMT_STREAM_VIDEO)
                              ? sizeof (video_fmt_mp4r_m4v_bitstream)
                              : sizeof (video_fmt_mp4r_m4a_bitstream)) - 1;
            video_fmt_bs_decode (video_fmt_mp4r_bs_callback, bitstream,
                                 bitstream_size, context);
            ++atom_stack_top->parse_pos;

            /* Fall-through to next case is intentional. */

        case 8: /* video/audio stream parsing */
            /* Continue parsing bitstream until done. */
            while ((context->bs_status != VIDEO_FMT_BS_DONE)
                && (context->bs_status != VIDEO_FMT_BS_FAILURE)
                && !context->bs_done)
            {
                context->bs_cont (context->bs_server_data);
            }

            /* Stop parser if it is still running. */
            if ((context->bs_status != VIDEO_FMT_BS_FAILURE)
                && (context->bs_status != VIDEO_FMT_BS_DONE))
            {
                context->bs_end (context->bs_server_data);
            }

            /* If the video uses short header mode, set H.263 video type. */
            if ((stream_info->type == VIDEO_FMT_STREAM_VIDEO)
                 && (stream_info->subinfo.video.format
                 == VIDEO_FMT_STREAM_VIDEO_INVALID)
                && (context->bs_short_header))
            {
                stream_info->subinfo.video.format
                    = VIDEO_FMT_STREAM_VIDEO_H263;
            }

            /* Otherwise, skip the rest of the atom. */
            video_fmt_mp4r_skip_data (context, (uint32)(atom_stack_top->atom.size
                                      + atom_stack_top->atom.offset
                                      - context->abs_pos), VIDEO_FMT_MP4R_IN_BUFFER);
            video_fmt_mp4r_finish_atom (context);
            break;
        }
    default:
      break;
    }
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_stsz

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'stsz' atom, which contains a table
  of sizes for every sample in the corresponding elementary stream.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_stsz
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_mp4r_stream_type   *stream;
    video_fmt_stream_info_type   *stream_info;

    UNUSED_PARAM(atom_struct_ptr);
    /* Branch according to processing substate. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    if( (context->num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS)||
        (context->num_streams ==0 ) )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "process_atom_stsz: too many/few streams!");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }
    stream = &context->stream_state [context->num_streams - 1];
    stream_info = &context->stream_info [context->num_streams - 1];
    stream_info->track_frag_info.first_frame = 0;
    switch (atom_stack_top->process_substate)
    {
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
        /* Read up to and including sample size and count. */
        if (!video_fmt_mp4r_read_buffer (context, 12, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
        {
            return TRUE;
        }
        video_fmt_mp4r_skip_data (context, 4, VIDEO_FMT_MP4R_IN_BUFFER);
        video_fmt_mp4r_consume_data
            (context, (uint8 *) &stream->sample_size, 4,
             context->byte_swap_needed,
             VIDEO_FMT_MP4R_IN_BUFFER);
        video_fmt_mp4r_consume_data
            (context, (uint8 *) &stream->stsz.table_size, 4,
             context->byte_swap_needed,
             VIDEO_FMT_MP4R_IN_BUFFER);

        /* Store stsz atom size. This info is used to check whether
           stsz atom is valid or not. If it is not valid, VIDEOFMT
           mark such tracks as INVALID */
        if(context->cur_atom_size >= 8)
        {
          stream->stsz.atom_size = (uint32)context->cur_atom_size - 8;
        }

        /* The stream size in frames is also the size of the 'stsz' table. */
        stream_info->frames = stream->stsz.table_size;
        stream->main_fragment_frames = stream_info->frames;
        /* If the sample size field is non-zero, every sample has the same
        ** size, and there is no 'stsz' table.  In this case, store the sample
        ** size in the stream table and finish the atom.
        */
        if (stream->sample_size != 0)
        {
            video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size
                                     + atom_stack_top->atom.offset
                                     - context->abs_pos, VIDEO_FMT_MP4R_IN_BUFFER);
            video_fmt_mp4r_finish_atom (context);
            stream_info->bytes = stream_info->frames * stream->sample_size;
            stream_info->largest = stream->sample_size;
            stream_info->fixed_size = TRUE;
            /* Found the largest */
            stream_info->largest_found = TRUE;
            stream->main_fragment_bytes = stream_info->bytes;
            return FALSE;
        }

        /* Verify size of atom matches number of entries. */
        if (stream->stsz.table_size * 4 != atom_stack_top->atom.size
            + atom_stack_top->atom.offset - context->abs_pos)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "process_atom_stsz: invalid atom size!");
          video_fmt_mp4r_failure (context);
          return TRUE;
        }

        /* Record offset of table in stream state. */
        stream->stsz.file_offset = context->abs_pos;

        /* skip traversing STSZ table and set the largest size to 0;
        need to remove this the stream_info->largest variable later. */
        stream_info->largest = 0;
        stream_info->largest_found = FALSE;

        stream->main_fragment_bytes = 0;

        /* skip rest of the atom and finish */
        video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size
                                  + atom_stack_top->atom.offset
                                  - context->abs_pos, VIDEO_FMT_MP4R_IN_BUFFER);
        video_fmt_mp4r_finish_atom (context);
    default:
      break;
    }
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_trun

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'trun' atom, which contains a table
  of sample size offset and duration  for every sample in track run.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_trun
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type         *context;
    video_fmt_mp4r_atom_pos_type        *atom_stack_top;
    video_fmt_mp4r_stream_type          *stream = 0;
    video_fmt_stream_info_type          *stream_info = 0;
    video_fmt_track_frag_info_type      *track_info;
    video_fmt_mp4r_sample_table_type    *trun;
    uint8                               version = 0;
    uint32                              index, size, duration, tr_flags,
                                        sampleFlags, sampleCompTimeOffset;

    UNUSED_PARAM(atom_struct_ptr);
    /* Branch according to processing substate. */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    size = 0;

    if (context->num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "process_atom_trun: too many streams!");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }

    for(index = 0 ; index < context->num_streams; index++)
    {
        if(context->stream_info[index].track_id == context->current_track_id)
        {
            stream_info = &context->stream_info [index];
            stream = &context->stream_state [index];
            break;
        }
    }

    if(!(stream_info) || !(stream))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "process_atom_trun: Could not find stream for track!");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }

    trun = &stream->trun[stream->trun_entry_count];

    switch (atom_stack_top->process_substate)
    {
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:

        /* Read up to and including tr_flags sample count data offset
        ** first sample flags
        */
        if (!video_fmt_mp4r_read_buffer (context, 16, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
        {
            return TRUE;
        }
        video_fmt_mp4r_consume_data
            (context, (uint8 *) &version, 1,
             context->byte_swap_needed,
             VIDEO_FMT_MP4R_IN_BUFFER);

        video_fmt_mp4r_consume_data
            (context, (uint8 *) &tr_flags, 3,
             context->byte_swap_needed,
             VIDEO_FMT_MP4R_IN_BUFFER);
        trun->tr_flags = tr_flags & 0xFFFFFF;

        /* Masking last byte out to leave out the
        *  data-offset-present and first-sample-flags-present
        *  which are the last byte of the tr_flag
        */
        trun->trun_sample_combination = (video_fmt_mp4r_trun_sample_combination_type)(trun->tr_flags & 0xFFFFF0);

       /* get the sample_count */
        video_fmt_mp4r_consume_data
            (context, (uint8 *) &trun->table_size, 4,
             context->byte_swap_needed,
             VIDEO_FMT_MP4R_IN_BUFFER);

        /* get the data_offset if the tr_flags indicates that it is present*/
        if(trun->tr_flags & TR_FLAG_DATA_OFFSET_FLAG_MASK)
        {
          video_fmt_mp4r_consume_data
              (context, (uint8 *) &trun->data_offset, 4,
               context->byte_swap_needed,
               VIDEO_FMT_MP4R_IN_BUFFER);
          if(context->isDashClip == TRUE)
          {
            video_fmt_mp4r_atom_pos_type *atom_stack_top = &context->atom_stack
                    [context->atom_stack_top - 3];
            /* trun data offset is relative value w.r.t moof offset*/
            trun->data_offset += atom_stack_top->atom.offset - 8;
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "trun data offset is %ld", (uint32)trun->data_offset);
          }
        }
        else
        {
          trun->data_offset = 0;
        }
        trun->base_data_offset = stream->base_data_offset;

        /* get the first_sample_flags if the tr_flags indicates that it is present*/
        if(trun->tr_flags & TR_FLAG_FIRST_SAMPLE_FLAG_MASK)
        {
          uint8 sample_depends_on_flag = 0 ,is_non_sync_sample = 0 ;
          video_fmt_mp4r_consume_data
               (context, (uint8 *) &trun->first_sample_flags, 4,
                 context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);

          sample_depends_on_flag = READ_BIT_FIELD(trun->first_sample_flags,
                                                   I_VOP_POSITION, I_VOP_SIZE);
          /* if sample_depends_on_flag is 0, then use the is_non_sync_sample
             value to determine the Sync Sample.*/
          is_non_sync_sample = READ_BIT_FIELD(trun->first_sample_flags,
                               NON_SYNC_VOP_POSITION, NON_SYNC_VOP_SIZE);

          if( (FLAG_I_VOP == sample_depends_on_flag) ||
            ( (FLAG_UNKNOWN_VOP == sample_depends_on_flag) &&
              ( ! is_non_sync_sample) ))
          {
             stream_info->inter_frames = TRUE;
          }

          trun->tr_flag_first_sample_flag_present = TRUE;
        }
        else
        {
          //no first_sample_flags present so no I-Frames present
          if(stream_info->type != VIDEO_FMT_STREAM_VIDEO)
            stream_info->inter_frames = TRUE;
          else
          {
            if(!stream_info->inter_frames)
                stream_info->inter_frames = FALSE;
          }
        }
        /*
        * We can have multiple TFHD atom for one track, so save default sample duration
        * from current TFHD atom in TRUN structure itself.
        * If 'trun' has sample duration information, it will be updated when we check
        * TR_FLAG_SAMPLE_DURATION_MASK below.
        */
        trun->default_sample_duration = stream->default_sample_duration;

       if(trun->tr_flags & TR_FLAG_SAMPLE_DURATION_MASK)
        {
          trun->tr_flag_sample_duration_present = TRUE;
          trun->numOptTrunFields++;
        }

        /*
        * We can have multiple TFHD atom for one track, so save default sample size
        * from current TFHD atom in TRUN structure itself.
        * If 'trun' has sample size information, it will be updated when we check
        * TR_FLAG_SAMPLE_SIZE_MASK below.
        */
        trun->default_sample_size = stream->default_sample_size;

        if(trun->tr_flags & TR_FLAG_SAMPLE_SIZE_MASK)
        {
          trun->tr_flag_sample_size_present = TRUE;
          trun->numOptTrunFields++;
        }

        if(trun->tr_flags & TR_FLAG_SAMPLE_FLAG_MASK)
        {
          trun->tr_flag_sample_flags_present = TRUE;

          /* if the sample flags are set then the
             current trun is having sync frame */
          stream_info->inter_frames = TRUE;
          trun->numOptTrunFields++;
        }

        if(trun->tr_flags & TR_FLAG_SAMPLE_COMPOSITE_TIME_OFFSET_MASK)
        {
          trun->tr_flag_sample_composition_time_offset_present = TRUE;
          trun->numOptTrunFields++;
          trun->atom_version = version;
        }

        /* Record offset of the sample table in the trun field */
        trun->file_offset = context->abs_pos;

        /* The run size in frames is added to the total frames. */
        stream_info->frames += trun->table_size;

        //! Validate TRUN table size field (Each field is of 4bytes)
        //! Left shift by 2 is equivalent to multiplication with 4.
        //! "abs_pos" is after the data consumed. "atom.offset" contains atom
        //! start offset. With the subtraction, we are comparing exactly with
        //! TRUN table data size.
        if (((trun->table_size * trun->numOptTrunFields) << 2) >
            (atom_stack_top->atom.size + atom_stack_top->atom.offset -
             context->abs_pos))
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                       "process_atom_trun: table_size %llu is corrupted",
                       trun->table_size);
          video_fmt_mp4r_failure (context);
          return TRUE;
        }
        /* Proceed to parsing the sample table in trun atom, in order to count total stream
        ** size and size of largest sample/frame.
        */
        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE;
        atom_stack_top->parse_uint32 [0] = (uint32)trun->table_size;
        /* Fall-through to next case is intentional. */

    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE:
        /* Read more entries into the table. */
        while (atom_stack_top->parse_uint32 [0]
               && video_fmt_mp4r_read_buffer (context, (trun->numOptTrunFields * 4)
                   , FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
        {
            /* Get the Sample duration if the tr_flags specifies that it exists*/
            if(trun->tr_flag_sample_duration_present)
            {
              video_fmt_mp4r_consume_data
                (context, (uint8 *) &duration, 4, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);
                 stream_info->media_duration += duration;
            }
            else
            {
               if(stream->default_sample_duration)
                   stream_info->media_duration += stream->default_sample_duration;
            }
            /* get the sample_size if the tr_flags specifies that it exists*/
            if(trun->tr_flag_sample_size_present)
            {
              video_fmt_mp4r_consume_data
                (context, (uint8 *) &size, 4, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);
            }
            else
            {
              if(stream->default_sample_size)
                size = stream->default_sample_size;
            }
            if (stream_info->bytes)
            {
                stream_info->largest = FILESOURCE_MAX (stream_info->largest, size);
            }
            else
            {
                stream_info->largest = FILESOURCE_MAX
                    (stream_info->largest, size);
            }
            stream_info->bytes += size;

            /* get the sample_flags if the tr_flags specifies that it exists*/
            if(trun->tr_flag_sample_flags_present)
            {
              video_fmt_mp4r_consume_data
                (context, (uint8 *) &sampleFlags, 4, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);
            }

            /* get the sample_composition_time_offset if the tr_flags specifies that it exists*/
            if(trun->tr_flag_sample_composition_time_offset_present)
            {
              video_fmt_mp4r_consume_data
                (context, (uint8 *) &sampleCompTimeOffset, 4, context->byte_swap_needed,
                 VIDEO_FMT_MP4R_IN_BUFFER);
            }

            --atom_stack_top->parse_uint32 [0];
        }

        /* If the entire table is read, finish up the atom. */
        if (!atom_stack_top->parse_uint32 [0])
        {
            video_fmt_mp4r_finish_atom (context);
            stream->trun_entry_count++;
            track_info = &stream_info->track_frag_info;
            trun->sample_description_index = track_info->sample_description_index;
            ++track_info->trun_count;
            return FALSE;
        }

        /* Otherwise, we will reach here because we need more data from the
        ** client.
        */
        else
        {
            return TRUE;
        }
    default:
      break;
    }
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_udta_child

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the atoms contained in the file or
  in the track 'udta' container atom:
    - 'vinf' atoms (at track level), which contain the video frame rate
    - 'cver' atoms (at track level), containing the track content version
    - 'rand' atoms (both at file and track level)

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_udta_child
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_mp4r_atom_pos_type *atom_stack_top;
    video_fmt_stream_info_type  *stream_info;
    video_fmt_mp4r_atom_pos_type *parent_atom, *grandparent_atom;
    boolean                     track_level, file_level;
    uint8                       data[8];
    uint16                      rate;

    UNUSED_PARAM(atom_struct_ptr);
    /* Process atom in the container 'udta' */
    context = (video_fmt_mp4r_context_type *) context_ptr;
    atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
    if (context->num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "process_atom_udta_child: too many streams!");
      video_fmt_mp4r_failure (context);
      return TRUE;
    }

    if(context->num_streams > 0)
    {
        stream_info = &context->stream_info [context->num_streams - 1];
    }
    else
    {
        stream_info = NULL;
    }

    switch (atom_stack_top->process_substate)
    {
    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
        if (!video_fmt_mp4r_read_buffer (context, 8, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
        {
            return TRUE;
        }

        atom_stack_top->process_substate
            = VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE;

        /* Fall-through to next case is intentional. */

    case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_PARSE:
        track_level = FALSE;
        file_level = FALSE;

        parent_atom = &context->atom_stack [context->atom_stack_top - 2];
        if ( parent_atom->atom.parent )
        {
            grandparent_atom
                = &context->atom_stack [context->atom_stack_top - 3];
            if ( grandparent_atom->atom.type == TRAK_TYPE )
            {
                track_level = TRUE;
            }
        }
        else
            file_level = TRUE;

        if ( !track_level && !file_level)
        {
            /* uninteresting case -> send this user atom info to client,
             * so client can parse the atom on its own.
            */
            context->cb_info.uuid_atom.offset = atom_stack_top->atom.offset;
            context->cb_info.uuid_atom.size = atom_stack_top->atom.size;
            context->cb_info.uuid_atom.atom_type = atom_stack_top->atom.type;
            context->callback_ptr ( VIDEO_FMT_UDTA_CHILD,
                                    context->client_data,
                                    &context->cb_info,
                                    video_fmt_mp4r_end  );
            /* then skip */
            video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size, VIDEO_FMT_MP4R_IN_BUFFER);
            video_fmt_mp4r_finish_atom (context);
            return FALSE;
        }
        else
        {
           (void) memset ((uint8 *) data, 0, 8);
           video_fmt_mp4r_consume_data (
                                        context,
                                        (uint8 *) data,
                                        FILESOURCE_MIN (8, atom_stack_top->atom.size),
                                        context->byte_swap_needed,
                                        VIDEO_FMT_MP4R_IN_BUFFER
                                        );
        }

        switch ( atom_stack_top->atom.type )
        {
        case VINF_TYPE:
            if ( track_level )
            {
                if(stream_info)
                {
                    /* this should be a track-level atom */
                    rate = (uint16)((data[3] << 8) | data[2]);
                    stream_info->subinfo.video.frame_rate
                        = (float)((rate >> 8) & 0xFF);
                    stream_info->subinfo.video.iframe_interval
                        = (uint16)((data[1] << 8) | data[0]);
                }
            }
            break;

        case CVER_TYPE:
            if ( track_level )
            {
                if(stream_info)
                {
                    /* this should be a track-level atom */
                    stream_info->user_data.content_version
                        = (uint16)((data[3] << 8) | data[2]);
                }
            }
            break;

        case RAND_TYPE:
            if ( track_level )
            {
                if(stream_info)
                {
                    stream_info->user_data.no_rand_access = data[7];
                }
            }
            else
            {
                if ( file_level )
                {
                    /* file-level atom */
                    context->file_level_data.no_rand_access = data[7];
                }
            }
            break;

        case RQMT_TYPE:
            if ( file_level )
            {
                context->file_level_data.video_only_allowed = data[0];
                context->file_level_data.audio_only_allowed = data[1];
            }
            break;

        default:
            break;
        }

        video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size
                                 - FILESOURCE_MIN (8, atom_stack_top->atom.size), VIDEO_FMT_MP4R_IN_BUFFER);
        video_fmt_mp4r_finish_atom (context);
        return FALSE;
    default:
      break;
    }
    return FALSE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_bs_callback

DESCRIPTION
  This function is the callback function from the video bitstream
  services.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_bs_callback
(
  video_fmt_bs_status_type          status,
  void                              *client_data,
  void                              *pInfo,
  video_fmt_bs_end_cb_func_type     end
)
{
    video_fmt_mp4r_context_type  *context;
    video_fmt_stream_info_type  *stream_info;
    video_fmt_alloc_type        alloc;
    video_fmt_free_type         free;
    video_fmt_bs_status_cb_info_type  *info =
                 (video_fmt_bs_status_cb_info_type*)pInfo;

    /* Recover context from client data. */
    context = (video_fmt_mp4r_context_type *) client_data;
    if ((context->num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS)||
        (context->num_streams == 0) )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "bs_callback: too many/few streams!");
      video_fmt_mp4r_failure (context);
      return;
    }

    /* Access current stream information. */
    stream_info = &context->stream_info [context->num_streams - 1];

    /* Store status and end callback. */
    context->bs_status = status;
    context->bs_end = end;

    /* Handle particular callback status. */
    switch (status)
    {
    case VIDEO_FMT_BS_ALLOC:
        alloc.size = info->alloc.size;
        context->callback_ptr (VIDEO_FMT_ALLOC, context->client_data,
                               &alloc, NULL);
        info->alloc.ptr = alloc.ptr;
        break;

    case VIDEO_FMT_BS_FREE:
        free.ptr = info->free.ptr;
        context->callback_ptr (VIDEO_FMT_FREE, context->client_data,
                               &free, NULL);
        break;

    case VIDEO_FMT_BS_GET_DATA:
        context->bs_cont = info->get_data.callback_ptr;
        context->bs_server_data = info->get_data.server_data;
        context->cb_info.get_data.buffer = info->get_data.buffer;
        context->cb_info.get_data.offset = info->get_data.offset
            + context->bs_offset;
        context->cb_info.get_data.num_bytes
            = FILESOURCE_MIN (info->get_data.num_bytes,
                   context->bs_size - FILESOURCE_MIN (info->get_data.offset,
                                           context->bs_size));
        context->cb_info.get_data.callback_ptr = video_fmt_mp4r_process;
        context->cb_info.get_data.server_data = context;
        context->callback_ptr (VIDEO_FMT_GET_DATA,
                               context->client_data,
                               &context->cb_info,
                               video_fmt_mp4r_end);
        info->get_data.num_bytes = (uint32)context->cb_info.get_data.num_bytes;
        break;

    case VIDEO_FMT_BS_VAR_INFO:
        context->bs_cont = info->var_info.callback_ptr;
        context->bs_server_data = info->var_info.server_data;

        /* Scan for width and height variables. */
        if (!strcmp (info->var_info.name, "vol_width"))
        {
            stream_info->subinfo.video.width = (uint16) info->var_info.value;
        }
        else if (!strcmp (info->var_info.name, "vol_height"))
        {
            stream_info->subinfo.video.height = (uint16) info->var_info.value;
        }

        /* Scan for "short" variable, which is used to determine whether or
        ** not the bitstream is using "short header" mode.
        */
        else if (!strcmp (info->var_info.name, "short"))
        {
            context->bs_short_header
                = ((info->var_info.value == 1) ? TRUE : FALSE);
        }

        /* Scan for AAC parameters. */
        else if (!strcmp (info->var_info.name, "audio_object_type"))
        {
            stream_info->subinfo.audio.aac_params.audio_object_type
                = (uint8) info->var_info.value;
            if( (stream_info->subinfo.audio.aac_params.audio_object_type == 5)
                || (stream_info->subinfo.audio.aac_params.audio_object_type == 29))
            {
              stream_info->subinfo.audio.aac_params.sbr_present_flag = 1;
              if( stream_info->subinfo.audio.aac_params.audio_object_type == 29)
              {
                stream_info->subinfo.audio.aac_params.ps_present_flag = 1;
              }
            }
        }
        else if (!strcmp (info->var_info.name, "m4a_sampling_freq_index"))
        {
            switch ((uint8) info->var_info.value)
            {
            case 0: stream_info->subinfo.audio
                        .sampling_frequency = 96000; break;
            case 1: stream_info->subinfo.audio
                        .sampling_frequency = 88200; break;
            case 2: stream_info->subinfo.audio
                        .sampling_frequency = 64000; break;
            case 3: stream_info->subinfo.audio
                        .sampling_frequency = 48000; break;
            case 4: stream_info->subinfo.audio
                        .sampling_frequency = 44100; break;
            case 5: stream_info->subinfo.audio
                        .sampling_frequency = 32000; break;
            case 6: stream_info->subinfo.audio
                        .sampling_frequency = 24000; break;
            case 7: stream_info->subinfo.audio
                        .sampling_frequency = 22050; break;
            case 8: stream_info->subinfo.audio
                        .sampling_frequency = 16000; break;
            case 9: stream_info->subinfo.audio
                        .sampling_frequency = 12000; break;
            case 10: stream_info->subinfo.audio
                         .sampling_frequency = 11025; break;
            case 11: stream_info->subinfo.audio
                         .sampling_frequency = 8000; break;
            case 12: stream_info->subinfo.audio
                         .sampling_frequency = 7350; break;
            }
        }
        else if (!strcmp (info->var_info.name, "m4a_sampling_freq"))
        {
            stream_info->subinfo.audio.sampling_frequency
                = info->var_info.value;
        }
        else if (!strcmp (info->var_info.name, "mp3_sampling_frequency"))
        {
            if (context->bs_mp3_id_bit)
            {
                /* MPEG-1 (ISO/IEC 11172-3) */
                switch ((uint8) info->var_info.value)
                {
                case 0: stream_info->subinfo.audio
                            .sampling_frequency = 44100; break;
                case 1: stream_info->subinfo.audio
                            .sampling_frequency = 48000; break;
                case 2: stream_info->subinfo.audio
                            .sampling_frequency = 32000; break;
                }
            }
            else
            {
                /* MPEG-2 (ISO/IEC 13818-3) extension for lower sampling
                ** frequencies.
                */
                switch ((uint8) info->var_info.value)
                {
                case 0: stream_info->subinfo.audio
                            .sampling_frequency = 22050; break;
                case 1: stream_info->subinfo.audio
                            .sampling_frequency = 24000; break;
                case 2: stream_info->subinfo.audio
                            .sampling_frequency = 16000; break;
                }
            }
        }
        else if (!strcmp (info->var_info.name, "mp3_id"))
        {
            context->bs_mp3_id_bit
                = ((info->var_info.value == 1) ? TRUE : FALSE);
        }
        else if (!strcmp (info->var_info.name, "mp3_mode"))
        {
            switch ((uint8) info->var_info.value)
            {
            case 0: /* stereo */
            case 1: /* joint stereo */
            case 2: /* dual channel */
                stream_info->subinfo.audio.num_channels = 2;
                break;

            case 3: /* single channel */
                stream_info->subinfo.audio.num_channels = 1;
                break;
            }
        }
        else if (!strcmp (info->var_info.name, "channel_configuration"))
        {
            stream_info->subinfo.audio.aac_params.channel_configuration
                = (uint8) info->var_info.value;
            stream_info->subinfo.audio.num_channels
                = (uint8) info->var_info.value;
        }
        else if (!strcmp (info->var_info.name, "ext_audio_object_type"))
        {
          if(info->var_info.value == 5)
          {
            stream_info->subinfo.audio.aac_params.sbr_present_flag = 1;
          }
        }
        else if (!strcmp (info->var_info.name, "ps_present_flag"))
        {
          stream_info->subinfo.audio.aac_params.ps_present_flag
                = (uint8) info->var_info.value;
        }
        else if (!strcmp (info->var_info.name, "ep_config"))
        {
            stream_info->subinfo.audio.aac_params.ep_config
                = (uint8) info->var_info.value;
        }
        else if (!strcmp (info->var_info.name,
                          "aac_section_data_resilience_flag"))
        {
            stream_info->subinfo.audio.aac_params.
                aac_section_data_resilience_flag
                = ((info->var_info.value != 0) ? TRUE : FALSE);
        }
        else if (!strcmp (info->var_info.name,
                          "aac_scalefactor_data_resilience_flag"))
        {
            stream_info->subinfo.audio.aac_params.
                aac_scalefactor_data_resilience_flag
                = ((info->var_info.value != 0) ? TRUE : FALSE);
        }
        else if (!strcmp (info->var_info.name,
                          "aac_spectral_data_resilience_flag"))
        {
            stream_info->subinfo.audio.aac_params.
                aac_spectral_data_resilience_flag
                = ((info->var_info.value != 0) ? TRUE : FALSE);
        }

        /* If the special "end" marker variable is set, stop parsing. */
        else if (!strcmp (info->var_info.name, "end"))
        {
            context->bs_done = TRUE;
        }
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                     "video_fmt_mp4r_bs_callback: offset %lu, size %lu:",
                     info->var_info.offset,info->var_info.size);
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"video_fmt_mp4r_bs_callback: "
                     "variable '%s' = '0x%x'",info->var_info.name,
                      (unsigned int)info->var_info.value);
        break;

    case VIDEO_FMT_BS_CONST_INFO:
        context->bs_cont = info->var_info.callback_ptr;
        context->bs_server_data = info->var_info.server_data;
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                     "video_fmt_mp4r_bs_callback: offset %lu, size %lu:",
                     info->var_info.offset,info->var_info.size);
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"video_fmt_mp4r_bs_callback: "
                     "constant '%s' = '0x%x'",
                     info->var_info.name,(unsigned int)info->var_info.value);
        break;

    case VIDEO_FMT_BS_FUNC_CALL:
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                     "video_fmt_mp4r_bs_callback: function call '%s' "
                     "at offset %lu",
                     info->func_call.name,info->func_call.offset);
        break;

    case VIDEO_FMT_BS_FUNC_DONE:
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                     "video_fmt_mp4r_bs_callback: function returned "
                     "at offset %lu",info->func_done.offset);
        break;

    case VIDEO_FMT_BS_CONTINUE:
        context->bs_cont = info->cont.callback_ptr;
        context->bs_server_data = info->cont.server_data;
        break;

    case VIDEO_FMT_BS_DONE:
        break;

    case VIDEO_FMT_BS_ABORT:
    case VIDEO_FMT_BS_FAILURE:
        /* Commenting here because of this error if we try to play AAC clip with corrupted esds atom
           we are simply exiting even though decoder can play that clip. */
        //stream_info->type = VIDEO_FMT_STREAM_INVALID;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "bs_callback: failure during bitstream parsing!");
        break;

    case VIDEO_FMT_BS_STATUS_INVALID:
    default:
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"bs_callback: invalid status!");
    }
}

/* <EJECT> */
/*===========================================================================

FUNCTION  video_fmt_mp4r_read_network_word

DESCRIPTION
  This function byte-swaps the given 4-byte word stored in network byte order
  (big-endian) if necessary so that it can be accessed by the native processor
  (host order, which may be big-endian or little-endian).

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void video_fmt_mp4r_read_network_word
(
  video_fmt_mp4r_context_type *context,
  uint32 *network_word
)
{
    int i;
    uint8 *network_bytes = (uint8 *) network_word;
    uint8 temp8;

    /* Byte-reverse word, if necessary. */
    if (context->byte_swap_needed)
    {
        for (i = 0; i < 2; ++i)
        {
            temp8 = network_bytes [i];
            network_bytes [i] = network_bytes [3 - i];
            network_bytes [3 - i] = temp8;
        }
    }
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_check_mfra_box

DESCRIPTION
  This function checks to see if the mfra box exists and processes it

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_check_mfra_box
(
  video_fmt_mp4r_context_type  *context
)
{
  uint32 mfraSize = 0;
  if(!context->mfra_present)
  {
    video_fmt_mp4_atom_type atom;

    if( !context->mfro_present )
    {
      if (!video_fmt_mp4r_read_buffer (context, 16, TRUE, VIDEO_FMT_MP4R_IN_BUFFER))
      {
        return TRUE;
      }

      video_fmt_mp4r_consume_data
                  (context,
                   (uint8 *) &atom.size,
                   FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                   VIDEO_FMT_MP4R_IN_BUFFER);

      video_fmt_mp4r_consume_data
                  (context,
                   (uint8 *) &atom.type,
                   FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                   VIDEO_FMT_MP4R_IN_BUFFER);

      if(!atom.size || !atom.type)
        return FALSE;

      if (atom.type != MFRO_TYPE)
      {
        return FALSE;
      }
      else
      {
        /*Skipping Atom version (1byte) and Atom flags (3bytes)*/
        video_fmt_mp4r_skip_data(context,4,VIDEO_FMT_MP4R_IN_BUFFER);

        video_fmt_mp4r_consume_data
                  (context,
                  (uint8 *)&mfraSize,
                  FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                  VIDEO_FMT_MP4R_IN_BUFFER);

        if(context->file_size > mfraSize)
        {
          /* flushing/resetting the IN_BUFFER before seeking abs_pos..!  */
          context->in_buffer_size = 0;
          context->in_buffer_pos = 0;
          context->abs_pos = context->file_size - mfraSize;
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "Corrupted size field in mfro atom.!!");
          return FALSE;
        }
        context->mfro_present = TRUE;
      }
    }

    if (!video_fmt_mp4r_read_buffer (context, 8, TRUE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
      return TRUE;
    }

    video_fmt_mp4r_consume_data
      (context,
      (uint8 *) &atom.size,
      FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
      VIDEO_FMT_MP4R_IN_BUFFER);

    video_fmt_mp4r_consume_data
      (context,
      (uint8 *) &atom.type,
      FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
      VIDEO_FMT_MP4R_IN_BUFFER);

    if(!atom.size || !atom.type)
      return FALSE;

    if (atom.type != MFRA_TYPE)
    {
      return FALSE;
    }
    else
    {
      context->mfra_present = TRUE;
      /* mfra present now parse the tfra atom */
      if (!video_fmt_mp4r_read_buffer (context, 24, TRUE, VIDEO_FMT_MP4R_IN_BUFFER))
      {
        return TRUE;
      }
      return TRUE;
    }
  }
  else
  {
      video_fmt_mp4_atom_type atom;

      /* Copy over atom size and type.  If size doesn't cover atom type,
      ** don't decode rest of atom.
      */
      video_fmt_mp4r_consume_data
                  (context,
                   (uint8 *) &atom.size,
                   FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                   VIDEO_FMT_MP4R_IN_BUFFER);

      video_fmt_mp4r_consume_data
                  (context,
                   (uint8 *) &atom.type,
                   FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                   VIDEO_FMT_MP4R_IN_BUFFER);

      if(!atom.size || !atom.type)
          return FALSE;

      /* If the type does not match TFRA skip the atom
      */
      if (atom.type != TFRA_TYPE)
      {
          video_fmt_mp4r_skip_data (context, atom.size, VIDEO_FMT_MP4R_IN_BUFFER);
          return TRUE;
      }
      else
      {
          uint32 tempVariable;

           //TFRA atom found
          video_fmt_mp4r_skip_data (context, 4, VIDEO_FMT_MP4R_IN_BUFFER);

          video_fmt_mp4r_consume_data
                      (context,
                       (uint8 *) &context->tfra.track_id,
                       FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                       VIDEO_FMT_MP4R_IN_BUFFER);

          video_fmt_mp4r_consume_data
                      (context,
                       (uint8 *) &tempVariable,
                       FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                       VIDEO_FMT_MP4R_IN_BUFFER);

          context->tfra.length_size_of_traf_num = tempVariable & 48; //(and with 110000)
          context->tfra.length_size_of_trun_num = tempVariable & 12; //(and with 001100)
          context->tfra.length_size_of_sample_num = tempVariable & 3; //(and with 000011)

          video_fmt_mp4r_consume_data
                      (context,
                       (uint8 *) &context->tfra.table_size,
                       FOURCC_SIGNATURE_BYTES, context->byte_swap_needed,
                       VIDEO_FMT_MP4R_IN_BUFFER);

          /* Record offset of the sample table in the tfra field */
          context->tfra.file_offset = context->abs_pos;
          return FALSE;
      }
  }
}
/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_mp3

DESCRIPTION
  This function consumes the .mp3 and ms\0u atoms. These atoms are not a part
  of the normal MP4 file format but are usually encountered in files encoded
  by ffmpeg. When these atoms are encountered, we set the codec type to mp3
  audio and set other parametes to NULL. These parameters are set later by
  reading the ES headers by the client application.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_mp3
(
  void  *context_ptr,
  void  *atom_struct
)
{
  video_fmt_mp4r_context_type *context =(video_fmt_mp4r_context_type*)context_ptr;
  video_fmt_stream_info_type  *stream_info;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  UNUSED_PARAM(atom_struct);
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];
  if(context->num_streams > 0)
  {
     stream_info = &context->stream_info [context->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (context);
    context->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }
  stream_info->type = VIDEO_FMT_STREAM_AUDIO;
  stream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_MPEG1_L3;
  video_fmt_mp4r_skip_data (context, atom_stack_top->atom.size, VIDEO_FMT_MP4R_IN_BUFFER);
  video_fmt_mp4r_finish_atom(context);
  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_mehd

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'mehd' atom, which contains total file's
  duration which includes the fragments duration as well.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_mehd
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type  *context;
  video_fmt_mp4r_atom_pos_type *atom_stack_top;
  uint8 version = 0;
  uint32 data_consumed = 0, lower4Byte;

  UNUSED_PARAM(atom_struct_ptr);
  /* Verify that the atom is not missing any key data. */
  context = (video_fmt_mp4r_context_type *) context_ptr;
  atom_stack_top = &context->atom_stack [context->atom_stack_top - 1];


  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer (context, atom_stack_top->atom.size,
                                   FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  video_fmt_mp4r_consume_data(context, &version, 1, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 1;
  video_fmt_mp4r_skip_data(context, 3, VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 3;
  context->file_level_data.fragment_file_total_movie_duration = 0;
  /* If it is version#1 type, then 8 bytes are used to store timestamp info */
  if(1 == version)
  {
    video_fmt_mp4r_consume_data(context,
        (uint8 *)&context->file_level_data.fragment_file_total_movie_duration,
        4, context->byte_swap_needed,VIDEO_FMT_MP4R_IN_BUFFER);
    context->file_level_data.fragment_file_total_movie_duration <<= 32;
    data_consumed += 4;
  }
  video_fmt_mp4r_consume_data(context, (uint8 *)&lower4Byte,
                              4, context->byte_swap_needed,
                              VIDEO_FMT_MP4R_IN_BUFFER);
  data_consumed += 4;
  context->file_level_data.fragment_file_total_movie_duration |= lower4Byte;

  video_fmt_mp4r_skip_data(context, atom_stack_top->atom.size - data_consumed,
                           VIDEO_FMT_MP4R_IN_BUFFER);
  video_fmt_mp4r_finish_atom (context);
  return FALSE;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_ec3

DESCRIPTION
    This function is used to parser ec3 & ac3 sample entry atom
DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_ec3
(
  void  *context_ptr,
  void  *atom_struct
)
{
  video_fmt_mp4r_context_type  *pContext;
  video_fmt_mp4r_atom_pos_type *pAtom_stack_top;
  /* video_fmt_consume_atom_structure_type *atom_struct; */
  video_fmt_stream_info_type   *pStream_info;
  video_fmt_stream_audio_subtype  *pAudio_info;
  uint32 ulSamplingRate = 0;

  UNUSED_PARAM(atom_struct);
  /* Branch according to processing substate. */
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if(!pContext)
  {
    return TRUE;
  }
  pAtom_stack_top = &pContext->atom_stack [pContext->atom_stack_top - 1];
  /* atom_struct = (video_fmt_consume_atom_structure_type *) atom_struct_ptr;*/

  switch (pAtom_stack_top->process_substate)
  {
  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
    pStream_info = &pContext->stream_info [pContext->num_streams - 1];
    pAudio_info = &pStream_info->subinfo.audio;

    memset(&pAudio_info->dd_params, 0, sizeof(pAudio_info->dd_params));
    pStream_info->type = VIDEO_FMT_STREAM_AUDIO;
    // Mark audio track as AC3 audio & EC3 audio based on atom type
    if( AC_3_TYPE == pAtom_stack_top->atom.type )
    {
      pStream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_AC3;
    }
    else if( EC_3_TYPE == pAtom_stack_top->atom.type )
    {
      pStream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_EAC3;
    }

    /* read required bytes */
    if (!video_fmt_mp4r_read_buffer
      (pContext, 28, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
      return TRUE;
    }

    /* Skip redefined, reserved etc */
    video_fmt_mp4r_skip_data (pContext, 24, VIDEO_FMT_MP4R_IN_BUFFER);

    /* Read sampling rate */
    video_fmt_mp4r_consume_data( pContext,
                                 (byte *)&ulSamplingRate,
                                 4,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );
    pStream_info->subinfo.audio.sampling_frequency = ulSamplingRate >> 16;

    /* Move on to processing child atoms. */
    pAtom_stack_top->process_substate = \
        VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
    break;

  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
    video_fmt_mp4r_finish_atom (pContext);
    break;

  default:
    break;
  }

  return FALSE;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_dac3

DESCRIPTION
    This function parse ac3 sample entry ac3 decoder specific atom
DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_dac3
(
  void  *context_ptr,
  void  *atom_struct
  )
{
  video_fmt_mp4r_context_type   *pContext;
  video_fmt_mp4r_atom_pos_type  *pAtom_stack_top;
  video_fmt_stream_info_type    *pStream_info;
  video_fmt_dd_params_type      *pDD_Info;
  uint8 ucData[4];
  uint8 ucBitRateCode = 0;
  uint32 ulDataConsumed = 0;

  UNUSED_PARAM(atom_struct);
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if( !pContext)
  {
    return TRUE;
  }
  pAtom_stack_top = &pContext->atom_stack [pContext->atom_stack_top - 1];
  if(pContext->num_streams > 0)
  {
    pStream_info = &pContext->stream_info [pContext->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (pContext);
    pContext->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }
  pDD_Info = &pStream_info->subinfo.audio.dd_params;
  memset(pDD_Info, 0, sizeof(video_fmt_dd_params_type));
  memset(ucData, 0, sizeof(uint32));

  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer ( pContext,
                                    pAtom_stack_top->atom.size,
                                    FALSE,
                                    VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  /*AC3SpecificBox
   * Field           #Bits
   * fscod            2
   * bsid             5
   * bsmod            3
   * acmod            3
   * lfeon            1
   * bit_rate_code    5
   * reserved         5
   */
  pDD_Info->ucNumIndSubs = 0;
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&ucData,
                               3,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);

  /*fscod[2 bits] sample rate code
   * 00 - 48kHz, 01 - 44.1kHz, 10 - 32kHz
   */
  pDD_Info->sBSIInfo[0].ucFscod = (uint8)( ( ucData[2] >> 6 ) & 0x03 );
  //bsid [5 bits ]Bit stream identification
  pDD_Info->sBSIInfo[0].ucBsid  = (uint8)( ( ucData[2] >> 1 ) & 0x1F );
  // bsmod [3 bits ] Bit stream mode
  pDD_Info->sBSIInfo[0].ucBsmod = (uint8)(( ( ucData[2] & 0x01) << 2 )| \
                                         ( ( ucData[1] >> 6 ) & 0x03 ));
  // acmod [ 3 bits ] Audio coding mode
  pDD_Info->sBSIInfo[0].ucAcmod = (uint8)( ( ucData[1] >> 3 ) & 0x07 );
  //lfeon [ 1 bits ] Low frequency effects channel ON
  pDD_Info->sBSIInfo[0].ucLfeon = (uint8)( ( ucData[1] >> 2 ) & 0x01 );

  // update number of channel based on acmod + lfeon
  pStream_info->subinfo.audio.num_channels = (uint8)\
    ( ( ACMOD_CHANNELS[pDD_Info->sBSIInfo[0].ucAcmod] ) + \
      ( pDD_Info->sBSIInfo[0].ucLfeon ? 1 : 0 ) );
  //! Update channel info in core stream structure
  pDD_Info->sBSIInfo[0].ucNumChannels = pStream_info->subinfo.audio.num_channels;
  //bit_rate_code [ 5 bits ]
  ucBitRateCode = (uint8)(( ( ucData[1] & 0x03 ) << 3 ) |
                          ((ucData[0] >> 5 ) & 0x07) );
  pDD_Info->usBitRate = (uint16)AC3_BITRATE_CODE[ucBitRateCode];

  ulDataConsumed += 3;
  //skip unconsumed bytes if any
  if(ulDataConsumed != pAtom_stack_top->atom.size)
  {
    video_fmt_mp4r_skip_data ( pContext,
                               pAtom_stack_top->atom.size - ulDataConsumed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  }
  // finish dac3 atom processing
  video_fmt_mp4r_finish_atom ( pContext);
  return FALSE;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_dec3

DESCRIPTION
    This function parse ec3 sample entry EC3 codec specific atom.

DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_dec3
(
  void  *context_ptr,
  void  *atom_struct
  )
{
  video_fmt_mp4r_context_type   *pContext;
  video_fmt_mp4r_atom_pos_type  *pAtom_stack_top;
  video_fmt_stream_info_type    *pStream_info;
  video_fmt_dd_params_type      *pDD_Info;
  uint8  ucData[4];
  uint8  ucTempData = 0;
  uint16 usDataRate = 0;
  int    nIndex = 0;
  uint32 ulDataConsumed = 0;

  UNUSED_PARAM(atom_struct);
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if( !pContext)
  {
    return TRUE;
  }

  pAtom_stack_top = &pContext->atom_stack [pContext->atom_stack_top - 1];
  if(pContext->num_streams > 0)
  {
    pStream_info = &pContext->stream_info [pContext->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (pContext);
    pContext->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }
  pDD_Info = &pStream_info->subinfo.audio.dd_params;
  memset(pDD_Info, 0, sizeof(video_fmt_dd_params_type));
  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer ( pContext,
                                    pAtom_stack_top->atom.size,
                                    FALSE,
                                    VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  /* EC3SpecificBox
    Field                 No# Bits
    data_rate .............13
    num_ind_sub ........... 3
    fscod ................. 2
    bsid .................. 5
    reserved                1
    asvc                    1
    bsmod ................. 3
    acmod ................. 3
    lfeon ................. 1
    reserved .............. 3
    num_dep_sub ........... 4
    if num_dep_sub > 0
    {
      chan_loc ............ 9
    }
    else
    {
      reserved ............ 1
    }
  */
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&usDataRate,
                               2,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);

  ulDataConsumed +=2;
  // data-rate: 13 bits
  pDD_Info->usBitRate = (uint16)( usDataRate >> 3 );
  // number of independent sub stream is sub-stream + 1: 3 bits
  pDD_Info->ucNumIndSubs = (uint8)(( ( usDataRate << 13 ) & 0x07 ) + 1);

  // EAC3 audio bit-stream can have 8 independent sub-stream.
  // Store BSI information related all independent sub-stream
  for( nIndex = 0; nIndex < pDD_Info->ucNumIndSubs ; nIndex++)
  {
    memset(ucData, 0, sizeof(uint32));
    // Consume next 3 bytes
    video_fmt_mp4r_consume_data( pContext,
                                (uint8*)&ucData,
                                 3,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);

    //fscod: 2bits
      pDD_Info->sBSIInfo[nIndex].ucFscod = (uint8)( ( ucData[2] >> 6 ) & 0x03 );
    //bsid: 5 bits
      pDD_Info->sBSIInfo[nIndex].ucBsid = (uint8)( ( ucData[2] >> 1 ) & 0x1F );
    // skip reserved 1 bit & asvc 1 bit and read bsmod 3 bits
    // |------|---------|---------|-------|
    // | 7    | 6  5  4 | 3  2  1 | 0     |
    // |------|---------|---------|-------|
    // |asvc  | bsmod   | acmod   | lfeon |
    // |------|---------|---------|-------|
      pDD_Info->sBSIInfo[nIndex].ucBsmod = (uint8)( ( ucData[1] & 0xF0 ) >> 1 );
    //acmod: 3 bits
      pDD_Info->sBSIInfo[nIndex].ucAcmod = (uint8)( ( ucData[1] >> 1 ) & 0x07 );
    //lfeon: 1 bits
      pDD_Info->sBSIInfo[nIndex].ucLfeon = (uint8)( ucData[1] & 0x01 );
    // update number of channel based on acmod + lfeon information
      pDD_Info->sBSIInfo[nIndex].ucNumChannels = \
        (uint8)( ( ACMOD_CHANNELS[pDD_Info->sBSIInfo[nIndex].ucAcmod] )+ \
                 ( pDD_Info->sBSIInfo[nIndex].ucLfeon ? 1: 0 ) );
    // |---------|------------|-----------------|
    // | 7  6  5 | 4  3  2  1 | 0               |
    // |---------|------------|-----------------|
    // |reserved | no_dep_sub | reserved/ch_loc |
    // |---------|------------|-----------------|
    //number of dependent sub-stream
      pDD_Info->sBSIInfo[nIndex].ucNumDepSubs = \
            (uint8)( ( ucData[0] >> 1 ) & 0xF);

      ulDataConsumed +=3;

      if ( pDD_Info->sBSIInfo[nIndex].ucNumDepSubs > 0)
      {
        video_fmt_mp4r_consume_data( pContext,
                                     (uint8*)&ucTempData,
                                     1,
                                     pContext->byte_swap_needed,
                                     VIDEO_FMT_MP4R_IN_BUFFER);
        pDD_Info->sBSIInfo[nIndex].usChLocation = (uint16)
          (((ucData[0] & 0x01 )<< 8) | ucTempData );
        ulDataConsumed++;
        pDD_Info->sBSIInfo[nIndex].ucNumChannels = (uint8) \
          ( pDD_Info->sBSIInfo[nIndex].ucNumChannels + \
            GET_CH_COUNT(pDD_Info->sBSIInfo[nIndex].usChLocation) );
      }
      // Make sure not to surpass atom.size in loop
      if ( ulDataConsumed == pAtom_stack_top->atom.size )
      {
        break;
      }
  }//for(num_ind_subs)

  // Check if further data to process i.e. ec3 extended data
  if( ulDataConsumed +1 <= pAtom_stack_top->atom.size)
  {
    // Read 1 byte ( ec3 extension data)
    ucTempData = 0;
    video_fmt_mp4r_consume_data( pContext,
                                (uint8*)&ucTempData,
                                 1,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
    // 7 byte reserved & 1 bit(ec3 extension type a)
    pDD_Info->ucEC3ExtTypeA = (ucTempData & 0x1);
    ulDataConsumed++;
    if(pDD_Info->ucEC3ExtTypeA)
    {
      // set format type as EC3-JOC
      pStream_info->subinfo.audio.format = VIDEO_FMT_STREAM_AUDIO_EAC3_JOC;
      // Read 1 byte ( complexity index type a)
      video_fmt_mp4r_consume_data( pContext,
                                  (uint8*)&pDD_Info->ucComplexIdxTypeA,
                                   1,
                                   pContext->byte_swap_needed,
                                   VIDEO_FMT_MP4R_IN_BUFFER);
      ulDataConsumed++;
    }
  }

  //skip unconsumed bytes if any
  if(ulDataConsumed != pAtom_stack_top->atom.size)
  {
    video_fmt_mp4r_skip_data ( pContext,
                               pAtom_stack_top->atom.size - ulDataConsumed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  }
  // finish atom processing
  video_fmt_mp4r_finish_atom ( pContext);
  return FALSE;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_dts

DESCRIPTION
    This function parse various DTS sample entry atom
DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_dts
(
  void  *context_ptr,
  void  *atom_struct
)
{
  video_fmt_mp4r_context_type  *pContext;
  video_fmt_mp4r_atom_pos_type *pAtom_stack_top;
  /* video_fmt_consume_atom_structure_type *atom_struct; */
  video_fmt_stream_info_type   *pStream_info;
  video_fmt_stream_audio_subtype  *pAudio_Info;
  uint32 ulSamplingRate = 0;
  uint16 usChannelCount = 0;

  UNUSED_PARAM(atom_struct);
  /* Branch according to processing substate. */
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if(!pContext)
  {
    return TRUE;
  }
  pAtom_stack_top = &pContext->atom_stack [pContext->atom_stack_top - 1];
  /* atom_struct = (video_fmt_consume_atom_structure_type *) atom_struct_ptr;*/

  switch (pAtom_stack_top->process_substate)
  {
  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
    pStream_info = &pContext->stream_info [pContext->num_streams - 1];
    pAudio_Info = &pStream_info->subinfo.audio;

    memset(&pAudio_Info->dts_params, 0, sizeof(pAudio_Info->dts_params));
    pStream_info->type = VIDEO_FMT_STREAM_AUDIO;
    // Based on sample entry box, define dts audio format type.
    if( DTSC_TYPE == pAtom_stack_top->atom.type )
    {
      // DTS formats prior to DTS-HD. DTS Core
      pAudio_Info->format = VIDEO_FMT_STREAM_AUDIO_DTS_CORE;
    }
    else if( DTSH_TYPE == pAtom_stack_top->atom.type )
    {
      // DTS-HD audio formats
      pAudio_Info->format = VIDEO_FMT_STREAM_AUDIO_DTS_HD;
    }
    else if( DTSL_TYPE == pAtom_stack_top->atom.type )
    {
      // DTS-HD lossless formats
      pAudio_Info->format = VIDEO_FMT_STREAM_AUDIO_DTS_HD_LOSSLESS;
    }
    else if( DTSE_TYPE == pAtom_stack_top->atom.type )
    {
      // DTS-LBR(Low Bit Rate) formats
      pAudio_Info->format = VIDEO_FMT_STREAM_AUDIO_DTS_LBR;
    }
    /* Read required bytes. AudioSampleEntry() size = 28 */
    if (!video_fmt_mp4r_read_buffer
      (pContext, 28, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
      return TRUE;
    }

    /* Skip redefined, reserved etc */
    video_fmt_mp4r_skip_data (pContext, 16, VIDEO_FMT_MP4R_IN_BUFFER);

    /* Read channel count */
    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&usChannelCount,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );
    pStream_info->subinfo.audio.num_channels = (uint8)usChannelCount;

    /* Skip SampleSize(16),predefined(16) & reserved(16)  */
    video_fmt_mp4r_skip_data (pContext, 6, VIDEO_FMT_MP4R_IN_BUFFER);

    /* Read sampling rate */
    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&ulSamplingRate,
                                 4,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );
    pStream_info->subinfo.audio.sampling_frequency = ulSamplingRate >> 16;

    /* Move on to processing child atoms. */
    pAtom_stack_top->process_substate = \
      VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
    break;

  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
    video_fmt_mp4r_finish_atom (pContext);
    break;

  default:
    break;
  }

  return FALSE;

}
/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_ddts

DESCRIPTION
    This function parse DTS sample entry codec specific atom.

DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_ddts
(
  void  *context_ptr,
  void  *atom_struct
)
{
  video_fmt_mp4r_context_type   *pContext;
  video_fmt_mp4r_atom_pos_type  *pAtom_stack_top;
  video_fmt_stream_info_type    *pStream_info;
  video_fmt_dts_params_type     *pDTS_info;
  uint8 ucData[4];
  uint32 ulDataConsumed = 0;

  UNUSED_PARAM(atom_struct);
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if( !pContext)
  {
    return TRUE;
  }
  pAtom_stack_top = &pContext->atom_stack [pContext->atom_stack_top - 1];
  if(pContext->num_streams > 0)
  {
    pStream_info = &pContext->stream_info [pContext->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (pContext);
    pContext->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }
  pDTS_info = &pStream_info->subinfo.audio.dts_params;
  memset(ucData, 0, sizeof(uint32));

  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer ( pContext,
                                    pAtom_stack_top->atom.size,
                                    FALSE,
                                    VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }
  /*
   * DTSSpecificBox
      unsigned int(32)    DTSSampling Frequency;
      unsigned int(32)    maxBitrate;
      unsigned int(32)    avgBitrate;
      unsigned char       pcmSampleDepth;    // value is 16 or 24 bits
      bit(2)              FrameDuration;     // 0 = 512, 1 = 1024, 2 = 2048, 3 = 4096
      bit(5)              StreamConstruction;// Table 2-2
      bit(1)              CoreLFEPresent;    // 0 = none; 1 = LFE exists
      bit(6)              CoreLayout;        // Table 2-3
      bit(14)             CoreSize;
      bit(1)              StereoDownmix      // 0 = none; 1 = embedded downmix present
      bit(3)              RepresentationType;// Table 2-4
      bit(16)             ChannelLayout;     // Table 2-5
      bit(1)              MultiAssetFlag     // 0 = single asset, 1 = multiple asset
      bit(1)              LBRDurationMod     // 0 = ignore, 1 = Special LBR duration modifier
      bit(1)              ReservedBoxPresent // 0 = no ReservedBox, 1 = Reserved present
      bit(5)              Reserved           // needs to be changed to 5.
      ReservedBox()                          // optional, for future expansion
  */
  /* Read maximum sampling frequency (32)*/
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&pDTS_info->ulMaxSamplingFrequency,
                               4,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;

  /* Read peak bit-rate(32) */
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&pDTS_info->ulMaxBitRate,
                               4,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  ulDataConsumed += 4;

  /* Read average bit-rate(32) */
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&pDTS_info->ulAvgBitRate,
                               4,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);

  ulDataConsumed += 4;
  /* Read sample depth (8) */
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&pDTS_info->ucPcmSampleDepth,
                               1,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);

  ulDataConsumed += 1;
  //skip unconsumed bytes if any. Currently not reading rest of parameter
  //as its not at all used.
  if(ulDataConsumed != pAtom_stack_top->atom.size)
  {
    video_fmt_mp4r_skip_data ( pContext,
                               pAtom_stack_top->atom.size - ulDataConsumed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  }
  // finish dac3 atom processing
  video_fmt_mp4r_finish_atom ( pContext);
  return FALSE;

}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_hvc1

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'hvc1' atom, which contains
  High Efficiency Video Codec informations for HEVC streams.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_hvc1
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type  *pContext;
  video_fmt_mp4r_atom_pos_type *pAtomStackTop;
  video_fmt_stream_info_type   *pStreamInfo;
  video_fmt_hevc_dec_info_type *pHEVCInfo;

  UNUSED_PARAM(atom_struct_ptr);
  /* Branch according to processing substate. */
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if(!pContext)
  {
    return TRUE;
  }
  pAtomStackTop = &pContext->atom_stack [pContext->atom_stack_top - 1];
  pStreamInfo = &pContext->stream_info [pContext->num_streams - 1];
  pHEVCInfo = &pStreamInfo->dec_specific_info.sHEVCInfo;

  switch (pAtomStackTop->process_substate)
  {
  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
    if( TRUE == pHEVCInfo->bHEVCParseDone )
    {
      //We have already parsed HEVC atom, so just skip it.
      video_fmt_mp4r_skip_data ( pContext,
                                 pAtomStackTop->atom.size,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
      video_fmt_mp4r_finish_atom (pContext);
      break;
    }
    /*Initializing the structure variables for hvcC atom*/
    memset(&pHEVCInfo->sHVCCAtomAlloc,0,sizeof(pHEVCInfo->sHVCCAtomAlloc));

    // Read required bytes. hvc1 extends VisualSampleEntry()
    if (!video_fmt_mp4r_read_buffer
      (pContext, 78, FALSE, VIDEO_FMT_MP4R_IN_BUFFER))
    {
      return TRUE;
    }

    /* Skip redefined, reserved etc */
    video_fmt_mp4r_skip_data (pContext, 24, VIDEO_FMT_MP4R_IN_BUFFER);

    /* Read width and height */
    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&pStreamInfo->subinfo.video.width,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );

    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&pStreamInfo->subinfo.video.height,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );

    /* Read Horiz/Vert resolutions */
    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&pHEVCInfo->ulHorizResolution,
                                 4,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );

    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&pHEVCInfo->ulVertResolution,
                                 4,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );

    /* Skip reserved bytes */
    video_fmt_mp4r_skip_data ( pContext, 4, VIDEO_FMT_MP4R_IN_BUFFER);

    /* Read frame count */
    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&pHEVCInfo->usFrameCount,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );

    /* Read compressor name and make it NULL terminated */
    video_fmt_mp4r_consume_data( pContext,
                                 pStreamInfo->dec_specific_info.info,
                                 32,
                                 FALSE,
                                 VIDEO_FMT_MP4R_IN_BUFFER );
    pStreamInfo->dec_specific_info.info[32] = '\0';

    /* Read depth */
    video_fmt_mp4r_consume_data( pContext,
                                (byte *)&pHEVCInfo->usDepth,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER );

    /* Skip Pre-defined bytes */
    video_fmt_mp4r_skip_data ( pContext, 2, VIDEO_FMT_MP4R_IN_BUFFER);

    /* Move on to processing child atoms.*/
    pAtomStackTop->process_substate = \
      VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
    break;

  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
    // We are done parsing HEVC atom, so set bHEVCParseDone to true to
    // avoid memory allocation when we reposition to first fragment and
    // process HEVC again.
    if(pHEVCInfo)
    {
      pHEVCInfo->bHEVCParseDone = TRUE;
    }
    video_fmt_mp4r_finish_atom (pContext);
    break;
  default:
    break;
  }

  return FALSE;
}

/*===========================================================================

FUNCTION  video_fmt_mp4r_process_atom_hvcC

DESCRIPTION
  This function is one of many functions which process MP4 file atoms.

  This particular function processes the 'hvcC' atom, which contains
  High Efficiency Video Codec informations for HEVC streams.

DEPENDENCIES
  None

RETURN VALUE
  If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
  None

===========================================================================*/
boolean video_fmt_mp4r_process_atom_hvcC
(
  void *context_ptr,
  void *atom_struct_ptr
)
{
  video_fmt_mp4r_context_type   *pContext;
  video_fmt_mp4r_atom_pos_type  *pAtomStackTop;
  video_fmt_stream_info_type    *pStreamInfo;
  video_fmt_hevc_dec_info_type  *pHEVCInfo;
  video_fmt_alloc_type          sAllocMem;
  uint32 ulArrIdx = 0;
  uint32 ulNalIdx = 0;
  uint32 ulBytesConsumed = 0;
  unsigned char ucTempData = 0;

  UNUSED_PARAM(atom_struct_ptr);
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if( !pContext)
  {
    return TRUE;
  }

  pAtomStackTop = &pContext->atom_stack [pContext->atom_stack_top - 1];
  if(pContext->num_streams > 0)
  {
    pStreamInfo = &pContext->stream_info [pContext->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"pContext->num_streams is 0");
    video_fmt_mp4r_failure (pContext);
    pContext->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return FALSE;
  }

  pHEVCInfo = &pStreamInfo->dec_specific_info.sHEVCInfo;

  //Update stream type as VIDEO & Codec type as HEVC
  pStreamInfo->type = VIDEO_FMT_STREAM_VIDEO;
  pStreamInfo->subinfo.video.format = VIDEO_FMT_STREAM_VIDEO_HEVC;

  /*
  ** Allocate the memory dynamically for the amount of data to be read
  ** equal to hvcC atom size.Create a new dynamic buffer and then set
  ** the destination pointer to that dynamic buffer.
  */

  if(pHEVCInfo->sHVCCAtomAlloc.memory_allocated == FALSE)
  {

    sAllocMem.size = pAtomStackTop->atom.size;
    pContext->callback_ptr ( VIDEO_FMT_ALLOC,
                             pContext->client_data,
                             &sAllocMem, NULL );
    if( NULL == sAllocMem.ptr )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "memory not allocated to alloc_atom.ptr ");
      video_fmt_mp4r_failure (pContext);
      return TRUE;
    }
    memset((byte*)sAllocMem.ptr, 0, sAllocMem.size);

    // Invalidate the ping pong buffer & read whole HVCC atom data
    pContext->in_buffer_pos = 0;
    pContext->in_buffer_size = 0;

    pContext->get_data_dst = (uint8*)sAllocMem.ptr;
    pContext->get_data_src = pContext->abs_pos;
    pContext->get_data_size = sAllocMem.size;
    pContext->get_data_needed = sAllocMem.size;
    pContext->get_data_read = 0;
    pContext->expect_eof = FALSE;
    pContext->state_next [0] = pContext->state;
    pContext->state = VIDEO_FMT_MP4R_STATE_GET_DATA;

    /*Setting the structure variable indicating memory had been allocated*/
    pHEVCInfo->sHVCCAtomAlloc.memory_ptr = (uint8*)sAllocMem.ptr;
    pHEVCInfo->sHVCCAtomAlloc.allocated_size = sAllocMem.size;
    pHEVCInfo->sHVCCAtomAlloc.memory_allocated = TRUE;

    return TRUE;
  }
  /* HEVC Configuration stored in HVCC atom
  aligned(8) class HEVCDecoderConfigurationRecord {
        unsigned int(8)   configurationVersion = 1;
        unsigned int(2)   general_profile_space;
        unsigned int(1)   general_tier_flag;
        unsigned int(5)   general_profile_idc;
        unsigned int(32)  general_profile_compatibility_flags;
        unsigned int(48)  general_constraint_indicator_flags;
        unsigned int(8)   general_level_idc;
        bit(4)            reserved = '1111'b;
        unsigned int(12)  min_spatial_segmentation_idc;
        bit(6)            reserved = '111111'b;
        unsigned int(2)   parallelismType;
        bit(6)            reserved = '111111'b;
        unsigned int(2)   chromaFormat;
        bit(5)            reserved = '11111'b;
        unsigned int(3)   bitDepthLumaMinus8;
        bit(5)            reserved = '11111'b;
        unsigned int(3)   bitDepthChromaMinus8;
        bit(16)           avgFrameRate;
        bit(2)            constantFrameRate;
        bit(3)            numTemporalLayers;
        bit(1)            temporalIdNested;
        unsigned int(2)   lengthSizeMinusOne;
        unsigned int(8)   numOfArrays;
        for (j=0; j < numOfArrays; j++) {
        bit(1) array_completeness;
        unsigned int(1) reserved = 0;
        unsigned int(6) NAL_unit_type;
        unsigned int(16) numNalus;
        for (i=0; i< numNalus; i++) {
        unsigned int(16) nalUnitLength;
        bit(8*nalUnitLength) nalUnit;
      }
    }
  }
  */
  // Allocate space for HVCC Info
  if( !pHEVCInfo->pHVCCInfo )
  {
    memset(&sAllocMem, 0, sizeof(sAllocMem));
    sAllocMem.size = (uint32)sizeof(video_fmt_mp4r_hvcc_info);
    pContext->callback_ptr( VIDEO_FMT_ALLOC,
                            pContext->client_data,
                            &sAllocMem, NULL);
    if( NULL == sAllocMem.ptr )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "memory not allocated to sAllocAtom.ptr ");
      video_fmt_mp4r_failure (pContext);
      return TRUE;
    }
    memset((byte*)sAllocMem.ptr, 0, sAllocMem.size);
    pHEVCInfo->pHVCCInfo = (video_fmt_mp4r_hvcc_info*)sAllocMem.ptr;
  }

  if( pHEVCInfo->pHVCCInfo )
  {
    /* Read unsigned int(8) configurationVersion */
    if( ( ulBytesConsumed+1) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size)
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                             (byte*)&pHEVCInfo->pHVCCInfo->ucConfigVersion,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              1,
                              FALSE,
                              pContext);
    }
    /* Read  unsigned int(2) general_profile_space;
     *       unsigned int(1) general_tier_flag;
     *       unsigned int(5) general_profile_idc;
     */
    if( ( ulBytesConsumed+1 ) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size )
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                             (byte*)&ucTempData,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              1,
                              FALSE,
                              pContext);
      pHEVCInfo->pHVCCInfo->ucProfileSpace = (uint8)((ucTempData & 0xC0 ) >> 6);
      pHEVCInfo->pHVCCInfo->ucTierFlag = (uint8)((ucTempData & 0x20 ) >> 5);
      pHEVCInfo->pHVCCInfo->ucProfileIDC = (uint8)(ucTempData & 0x1F);
      pStreamInfo->subinfo.video.profile = pHEVCInfo->pHVCCInfo->ucProfileIDC;
    }
    /* Read unsigned int(32)general_profile_compatibility_flags */
    if( ( ulBytesConsumed+1) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size)
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                              (byte*)&pHEVCInfo->pHVCCInfo->ulProfileCompatibilityFlags,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              4,
                              pContext->byte_swap_needed,
                              pContext);
    }
    /* Read unsigned int(48)  general_constraint_indicator_flags */
    if( ( ulBytesConsumed+1) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size)
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                              (byte*)&pHEVCInfo->pHVCCInfo->usContraintIndicatorFlags,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              6,
                              pContext->byte_swap_needed,
                              pContext);
    }

    /*  Read unsigned int(8) general_level_idc */
    if((ulBytesConsumed+1) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size )
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                              (byte*)&pHEVCInfo->pHVCCInfo->ucLevelIDC,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              1,
                              FALSE,
                              pContext);
      pStreamInfo->subinfo.video.level = pHEVCInfo->pHVCCInfo->ucLevelIDC;
    }

    /* Skip following field as of now, if required we can parse in future
     * bit (4)  reserved = '1111'b;
     * uint(12) min_spatial_segmentation_idc;
     * bit (6)  reserved = '111111'b;
     * uint(2)  parallelismType;
     * bit (6)  reserved = '111111'b;
     * uint(2)  chromaFormat;
     * bit (5)  reserved = '11111'b;
     * uint(3)  bitDepthLumaMinus8;
     * bit (5)  reserved = '11111'b;
     * uint(3)  bitDepthChromaMinus8;
     *
     * Skip: bits(48): bytes(6)
    */
    video_fmt_mp4r_skip_data ( pContext, 6, VIDEO_FMT_MP4R_IN_BUFFER);
    ulBytesConsumed +=6;

    /*  Read int(16)avgFrameRate; */
    if((ulBytesConsumed+1) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size )
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                              (byte*)&pHEVCInfo->pHVCCInfo->usAvgFrameRate,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              2,
                              pContext->byte_swap_needed,
                              pContext);
    }
    /* Read lengthSizeMinusOne & skip other filed in byte
     * bit(2) constantFrameRate;
     * bit(3) numTemporalLayers;
     * bit(1) temporalIdNested;
     * uint(2) lengthSizeMinusOne;
     */
    if((ulBytesConsumed+1) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size )
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                             (byte*)&pHEVCInfo->pHVCCInfo->ucLenSizeMinusOne,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              1,
                              FALSE,
                              pContext);
      pHEVCInfo->pHVCCInfo->ucLenSizeMinusOne &=0x03;
    }

    /*  Read Number of NAL array int(8)  */
    if((ulBytesConsumed+1) <= pHEVCInfo->sHVCCAtomAlloc.allocated_size )
    {
      ulBytesConsumed += video_fmt_consume_data( \
                              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                             (byte*)&pHEVCInfo->pHVCCInfo->ucNumOfNALArray,
                              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                              ulBytesConsumed,
                              1,
                              FALSE,
                              pContext);
    }
    // If number of NALU array valid, allocate space to hold all array data
    if( pHEVCInfo->pHVCCInfo->ucNumOfNALArray)
    {
      memset(&sAllocMem, 0, sizeof(sAllocMem));
      sAllocMem.size = pHEVCInfo->pHVCCInfo->ucNumOfNALArray *
                       (uint32)sizeof(video_fmt_nalu_array_type);

      pContext->callback_ptr ( VIDEO_FMT_ALLOC,
                               pContext->client_data,
                               &sAllocMem, NULL );
      if(sAllocMem.ptr == NULL)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "memory not allocated to alloc.ptr ");
        video_fmt_mp4r_failure (pContext);
        return TRUE;
      }
      pHEVCInfo->pHVCCInfo->pArrayNALU = \
                   (video_fmt_nalu_array_type*)sAllocMem.ptr;
      memset(pHEVCInfo->pHVCCInfo->pArrayNALU, 0, sAllocMem.size);

      // Process through all NALU array
      for( ulArrIdx = 0;
           ( ulArrIdx < pHEVCInfo->pHVCCInfo->ucNumOfNALArray) &&
           ( ulBytesConsumed < pAtomStackTop->atom.size ); ulArrIdx++ )
      {
        ucTempData = 0;
        // Read array completeness, NALU type. If '1'then all NAL units
        // of indicated type will be available in following array. If '0'
        // NAL units of indicate type can be available in bit stream also.
        ulBytesConsumed += video_fmt_consume_data( \
                            pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                           (byte*)&ucTempData,
                            pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                            ulBytesConsumed,
                            1,
                            FALSE,
                            pContext);
        pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].ucArrayCompleteness = \
                      (ucTempData & 0x80) >> 7;
        // Update NAL unit type
        pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].ucNALUType = \
                      (ucTempData & 0x3F);

        // Read number of NALU of indicated type available in NALU array
        ulBytesConsumed += video_fmt_consume_data( \
                 pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                (byte*)&pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].usNumNALU,
                 pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                 ulBytesConsumed,
                 2,
                 pContext->byte_swap_needed,
                 pContext);

        // Process & store all indicated NALU data
        if( pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].usNumNALU )
        {
          // Allocate memory to store all NAL unit of indicated type in
          // an NALU array
          memset(&sAllocMem, 0, sizeof(sAllocMem));
          sAllocMem.size = \
            pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].usNumNALU * \
            (uint32)sizeof(video_fmt_nalu_data_type);

          pContext->callback_ptr ( VIDEO_FMT_ALLOC, pContext->client_data,
                                   &sAllocMem, NULL );
          if(sAllocMem.ptr == NULL)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "memory not allocated to alloc.ptr ");
            video_fmt_mp4r_failure (pContext);
            return TRUE;
          }
          pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU = \
                                  (video_fmt_nalu_data_type*)sAllocMem.ptr;

          memset(pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU, 0, sAllocMem.size);

          // Process each NAL unit
          for( ulNalIdx = 0;
               ( ulNalIdx < pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].usNumNALU) &&
               ( ulBytesConsumed < pAtomStackTop->atom.size ); ulNalIdx++ )
          {
            // Read NAL unit length
            ulBytesConsumed += video_fmt_consume_data( \
              pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
              (byte*)&pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len,
              pHEVCInfo->sHVCCAtomAlloc.allocated_size,
              ulBytesConsumed,
              2,
              pContext->byte_swap_needed,
              pContext);

            if(pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len)
            {
              memset(&sAllocMem, 0, sizeof(sAllocMem));
              // Store all NALU data length, this will require to advertise
              // NALU buffer size to provide codec config data. 4 additional
              // byte to accommodate Start-Code (0x01000000)
              pHEVCInfo->pHVCCInfo->ulNaluDataLength += \
                pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len + 4;

              sAllocMem.size = pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len + 4;
              pContext->callback_ptr( VIDEO_FMT_ALLOC, pContext->client_data,
                                      &sAllocMem, NULL);
              if(sAllocMem.ptr == NULL)
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "memory not allocated to alloc.ptr ");
                  video_fmt_mp4r_failure (pContext);
                return TRUE;
              }
              pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].data = \
                (byte*) sAllocMem.ptr;
              memset( pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].data,
                      0,
                      pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len);

              if( ( ulBytesConsumed + \
                    pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len ) <= \
                    pHEVCInfo->sHVCCAtomAlloc.allocated_size)
              {
                ulBytesConsumed += video_fmt_consume_data( \
                  pHEVCInfo->sHVCCAtomAlloc.memory_ptr,
                  (byte*)pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].data,
                  pHEVCInfo->sHVCCAtomAlloc.allocated_size,
                  ulBytesConsumed,
                  pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len,
                  FALSE,
                  pContext);
              }
              else
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "error while parsing HVCC NAL parameters");
                  video_fmt_mp4r_failure (pContext);
                return TRUE;
              }
            }//if(pNALU[ulNalIdx].len)
          }//for(pNALUData)
        }//if(usNumNALU)
      }//for(pArrayNALU)
    }//if(ucNumOfNALArray)
    //skip any bytes which are not consumed...
    if(ulBytesConsumed != pAtomStackTop->atom.size)
    {
      video_fmt_mp4r_skip_data ( pContext,
                                 pAtomStackTop->atom.size- ulBytesConsumed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
    }
  }//if(pHEVCDecInfo->pHVCCInfo)
  // now we are done with this atom */
  video_fmt_mp4r_finish_atom (pContext);

  return FALSE;
}
/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_d263

DESCRIPTION
    This function parse d263 sample entry h263 decoder specific atom
DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_d263
(
  void  *context_ptr,
  void  *atom_struct
  )
{

  video_fmt_mp4r_context_type  *pContext;
  video_fmt_stream_info_type   *pStreamInfo;
  video_fmt_mp4r_atom_pos_type  *pAtomStackTop;
  uint32 ulDataConsumed = 0;

  UNUSED_PARAM(atom_struct);
  /* Mark stream as H.263 format. */
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if( !pContext)
  {
    return TRUE;
  }
  if(pContext->num_streams > 0)
  {
     pStreamInfo = &pContext->stream_info [pContext->num_streams - 1];
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (pContext);
    pContext->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return TRUE;
  }
  pAtomStackTop = &pContext->atom_stack [pContext->atom_stack_top - 1];

  /* d263 atom structure--
     Vendor Details   -- 4 bytes
     Decoder version  -- 1 bytes
     h263 level       -- 1 byte
     h263 profile     -- 1 byte
  */

  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer ( pContext,
                                    pAtomStackTop->atom.size,
                                    FALSE,
                                    VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }

  //Check whether atom size is minimum required value or not
  if (pAtomStackTop->atom.size >= 7)
  {
    video_fmt_mp4r_skip_data( pContext, 5,
                              VIDEO_FMT_MP4R_IN_BUFFER);
    ulDataConsumed += 5;

    // read 1byte which contains H263 level info
    video_fmt_mp4r_consume_data( pContext,
                                (uint8*)&pStreamInfo->subinfo.video.level,
                                 1,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
    ulDataConsumed++;

    // read 1byte which contains H263 level info
    video_fmt_mp4r_consume_data( pContext,
                                (uint8*)&pStreamInfo->subinfo.video.profile,
                                 1,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);
    ulDataConsumed++;
  }

  //skip unconsumed bytes if any
  if(ulDataConsumed != pAtomStackTop->atom.size)
  {
    video_fmt_mp4r_skip_data ( pContext,
                               pAtomStackTop->atom.size - ulDataConsumed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  }
  // finish d263 atom processing
  video_fmt_mp4r_finish_atom ( pContext);
  return FALSE;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_amr

DESCRIPTION
    This function parse amr/amrwb sample entry atom
DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_amr
(
  void  *context_ptr,
  void  *atom_struct
  )
{
  video_fmt_mp4r_context_type  *pContext;
  video_fmt_mp4r_atom_pos_type *pAtomStackTop;
  /* video_fmt_consume_atom_structure_type *atom_struct; */
  video_fmt_stream_info_type   *pStream_info;
  video_fmt_stream_audio_subtype  *pAudio_Info;
  uint16 usDataRefIndex = 0;
  uint16 usChannelCount = 0;
  uint16 usSampleRate   = 0;

  UNUSED_PARAM(atom_struct);
  /* Branch according to processing substate. */
  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if(!pContext)
  {
    return TRUE;
  }
  pAtomStackTop = &pContext->atom_stack [pContext->atom_stack_top - 1];

  switch (pAtomStackTop->process_substate)
  {
  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_INIT:
    pStream_info = &pContext->stream_info [pContext->num_streams - 1];
    pAudio_Info = &pStream_info->subinfo.audio;

    pStream_info->type = VIDEO_FMT_STREAM_AUDIO;
    // Based on sample entry box, define AMR audio format type.
    // SAMR in case of AMR, SAWB in case of ARM-WB.
    // Updating codec type here also based on atom type, as
    // some of the clip doesn't contain "DAMR"

    if( SAMR_TYPE == pAtomStackTop->atom.type )
    {
      pAudio_Info->format = VIDEO_FMT_STREAM_AUDIO_AMR;
      pAudio_Info->sampling_frequency = 8000;
    }
    else if( SAWB_TYPE == pAtomStackTop->atom.type )
    {
      pAudio_Info->format = VIDEO_FMT_STREAM_AUDIO_AMR_WB;
      pAudio_Info->sampling_frequency = 16000;
    }
    else if(SAWP_TYPE == pAtomStackTop->atom.type)
    {
      pAudio_Info->format = VIDEO_FMT_STREAM_AUDIO_AMR_WB_PLUS;
    }
    /* Read complete atom data  */
    if (!video_fmt_mp4r_read_buffer ( pContext,
                                      pAtomStackTop->atom.size,
                                      FALSE,
                                      VIDEO_FMT_MP4R_IN_BUFFER))
    {
      return TRUE;
    }
    /*
     AMRBB(samr), AMRWB(sawb)& AMRWB+(sawp)sample description box.
     Refer:
     field            bytes
     reserved_           6
     dataReferenceIndex  2
     reserved_2          8
     channelcount        2
     sampleSize          2
     pre_defined         2
     reserved_3          2
     samplingrate        2
     reserved_4          2
     */
    // Skip reserved_1(6bytes)
    video_fmt_mp4r_skip_data (pContext, 6, VIDEO_FMT_MP4R_IN_BUFFER);

    // read dataReferenceIndex 2 bytes
    video_fmt_mp4r_consume_data( pContext,
                                &usDataRefIndex,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);

    // Skip reserved_2(8bytes)
    video_fmt_mp4r_skip_data (pContext, 8, VIDEO_FMT_MP4R_IN_BUFFER);

    // read channelcount (2 bytes)
    video_fmt_mp4r_consume_data( pContext,
                                &usChannelCount,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);

    pAudio_Info->num_channels = usChannelCount;
    // Skip samplesize(2bytes), pre_defined(2bytes) & reserved_3(2bytes)
    video_fmt_mp4r_skip_data (pContext, 6, VIDEO_FMT_MP4R_IN_BUFFER);


    // read samplerate (2 bytes)
    video_fmt_mp4r_consume_data( pContext,
                                &usSampleRate,
                                 2,
                                 pContext->byte_swap_needed,
                                 VIDEO_FMT_MP4R_IN_BUFFER);

    pAudio_Info->sampling_frequency = usSampleRate;

    //skip reserved_4(2 bytes)
    video_fmt_mp4r_skip_data (pContext, 2, VIDEO_FMT_MP4R_IN_BUFFER);

    // For codec completeness don't skip AMR/AMR-WB & AMR-WB+ SampleEntry box
    // though no useful information present in decoder box.
    pAtomStackTop->process_substate = \
      VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN;
    break;

  case VIDEO_FMT_MP4R_PROCESS_ATOM_SUBSTATE_CHILDREN_DONE:
    video_fmt_mp4r_finish_atom (pContext);
    break;

  default:
    break;
  }
  return FALSE;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_damr

DESCRIPTION
    This function parse amr/amrwb sample entry decoder configuration atom
DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_damr
(
  void  *context_ptr,
  void  *atom_struct
  )
{
  video_fmt_mp4r_context_type    *pContext;
  video_fmt_stream_info_type     *pStreamInfo;
  video_fmt_mp4r_atom_pos_type   *pAtomStackTop;
  video_fmt_stream_audio_subtype *pAudio_Info;
  uint16 usDataConsumed = 0;

  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if( !pContext)
  {
    return TRUE;
  }
  if(pContext->num_streams > 0)
  {
     pStreamInfo = &pContext->stream_info [pContext->num_streams - 1];
     pAudio_Info = &pStreamInfo->subinfo.audio;
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (pContext);
    pContext->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return TRUE;
  }
  pAtomStackTop = &pContext->atom_stack [pContext->atom_stack_top - 1];

  /* AMRSpecificBox ( damr) atom structure. Applicable to amr, amr-wb.
     audio codec.
     struct AMRDecSpecStruc{
      unsinged int(32) vendor;
      unsigned int (8) decoder_version
      unsigned int(16) mode_set
      unsigned int (8) mode_change_period
      unsigned int (8) frames_per_sample
     }
  */

  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer ( pContext,
                                    pAtomStackTop->atom.size,
                                    FALSE,
                                    VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }

  //Skip vendor(4bytes) & decoder_version(1bytes)
  video_fmt_mp4r_skip_data( pContext, 5,
                            VIDEO_FMT_MP4R_IN_BUFFER);
  usDataConsumed += 5;

  // read mode_set(2bytes) information.
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&pAudio_Info->audio_params.mode_set,
                               2,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  usDataConsumed += 2;

  // read mode_change_period(1bytes) information.
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&pAudio_Info->audio_params.mode_change_period,
                               1,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  usDataConsumed ++;

  // read frames_per_sample(1bytes) information.
  video_fmt_mp4r_consume_data( pContext,
                              (uint8*)&pAudio_Info->audio_params.frames_per_sample,
                               1,
                               pContext->byte_swap_needed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  usDataConsumed ++;

  //skip unconsumed bytes if any
  if(usDataConsumed != pAtomStackTop->atom.size)
  {
    video_fmt_mp4r_skip_data ( pContext,
                               pAtomStackTop->atom.size - usDataConsumed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  }
  // finish damr atom processing
  video_fmt_mp4r_finish_atom ( pContext);
  return FALSE;
}

/*===========================================================================
FUNCTION  video_fmt_mp4r_process_atom_dawp

DESCRIPTION
    This function parse amrwb+ sample entry decoder configuration atom
DEPENDENCIES
None

RETURN VALUE
    If TRUE, the MP4 file format services should return to the caller.

SIDE EFFECTS
None

===========================================================================*/
extern boolean video_fmt_mp4r_process_atom_dawp
(
  void  *context_ptr,
  void  *atom_struct
  )
{
  video_fmt_mp4r_context_type    *pContext;
  video_fmt_stream_info_type     *pStreamInfo;
  video_fmt_mp4r_atom_pos_type   *pAtomStackTop;
  video_fmt_stream_audio_subtype *pAudio_Info;
  uint16 usDataConsumed = 0;

  pContext = (video_fmt_mp4r_context_type *) context_ptr;
  if( !pContext)
  {
    return TRUE;
  }
  if(pContext->num_streams > 0)
  {
     pStreamInfo = &pContext->stream_info [pContext->num_streams - 1];
     pAudio_Info = &pStreamInfo->subinfo.audio;
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"context->num_streams is 0");
    video_fmt_mp4r_failure (pContext);
    pContext->state = VIDEO_FMT_MP4R_STATE_INVALID;
    return TRUE;
  }
  pAtomStackTop = &pContext->atom_stack [pContext->atom_stack_top - 1];

  /* AMRWPDecSpecificBox ( damr) atom structure. Applicable to amr-wbp.
     audio codec.
     struct AMRDecSpecStruc{
      unsinged int(32) vendor;
      unsigned int (8) decoder_version
     }
  */

  /* Read complete atom data  */
  if (!video_fmt_mp4r_read_buffer ( pContext,
                                    pAtomStackTop->atom.size,
                                    FALSE,
                                    VIDEO_FMT_MP4R_IN_BUFFER))
  {
    return TRUE;
  }

  //Skip vendor(4bytes) & decoder_version(1bytes). As of now
  //no useful information in 'dawp' putting this for code
  //completeness.
  video_fmt_mp4r_skip_data( pContext, 5,
                            VIDEO_FMT_MP4R_IN_BUFFER);
  usDataConsumed += 5;

  //skip unconsumed bytes if any
  if(usDataConsumed != pAtomStackTop->atom.size)
  {
    video_fmt_mp4r_skip_data ( pContext,
                               pAtomStackTop->atom.size - usDataConsumed,
                               VIDEO_FMT_MP4R_IN_BUFFER);
  }
  // finish damr atom processing
  video_fmt_mp4r_finish_atom ( pContext);
  return FALSE;
}
