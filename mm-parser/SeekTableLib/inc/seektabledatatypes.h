#ifndef SEEKTABLE_DATA_TYPES_H
#define SEEKTABLE_DATA_TYPES_H

/* =======================================================================
                              seektabledatatypes.h
DESCRIPTION
  
Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SeekTableLib/main/latest/inc/seektabledatatypes.h#4 $
========================================================================== */

//Data types used
typedef unsigned int        st_uint32;
typedef int                 st_int32;
typedef short               st_int16;
typedef unsigned short      st_uint16;
typedef unsigned char       st_uint8;
typedef char                st_int8;
typedef signed long long    st_int64;      
typedef unsigned long long  st_uint64;     

typedef struct {
  st_uint32 frame;
  st_uint64 time;
  st_uint64 position;
} seek_table_entry;
#endif
