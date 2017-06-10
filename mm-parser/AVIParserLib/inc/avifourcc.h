#ifndef AVI_FOURCC_TYPES_H
#define AVI_FOURCC_TYPES_H

/* =======================================================================
                              aviFourCC.h
DESCRIPTION
  
Copyright 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/avifourcc.h#8 $
========================================================================== */

//Various FOURCCs used in an AVI file.
#define AVI_START_BYTES       "RIFF"
#define AVI_SIGNATURE_BYTES   "AVI "
#define AVI_LIST_FOURCC       "LIST"
#define AVI_HDRL_FOURCC       "hdrl"
#define AVI_AVIH_FOURCC       "avih"
#define AVI_STRL_FOURCC       "strl"
#define AVI_STRF_FOURCC       "strf"
#define AVI_STRH_FOURCC       "strh"
#define AVI_STRN_FOURCC       "strn"
#define AVI_VIDS_FOURCC       "vids"
#define AVI_AUDS_FOURCC       "auds"
#define AVI_MOVI_FOURCC       "movi"
#define AVI_STRD_FOURCC       "strd"
#define AVI_IDX1_FOURCC       "idx1"
#define AVI_INDX_FOURCC       "indx"
#define AVI_ODML_FOURCC       "odml"
#define AVI_JUNK_FOURCC       "JUNK"
#define AVI_INFO_FOURCC       "INFO"
#define AVI_IX_FOURCC         "ix"
#define AVI_DRM_INFO_FOURCC   "dc"
#define AVI_RES_FOURCC        "RES"
#define AVI_REC_FOURCC        "rec "

//Various Chunk IDs used in INFO chunk

#define INFO_ARCHIVAL_LOCATION "IARL"
#define INFO_ARTIST            "IART"
#define INFO_COMMISIONED       "ICMS"
#define INFO_COMMENTS          "ICMT"
#define INFO_COPYRIGHT         "ICOP"
#define INFO_CREATION_DATE     "ICRD"
#define INFO_GENRE             "IGNR"
#define INFO_KEYWORD           "IKEY"
#define INFO_NAME              "INAM"
#define INFO_PRODUCT           "IPRD"
#define INFO_SUBJECT           "ISBJ"
#define INFO_SOFTWARE          "ISFT"
#define INFO_SOURCE            "ISRC"

#endif
