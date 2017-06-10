// -*- Mode: C++ -*-
//=============================================================================
// FILE: qcpconstants.h
//
// SERVICES: Audio
//
// DESCRIPTION:
/// Declarations required for QCP Format parser implementation
///
/// Copyright (c) 2011 Qualcomm Technologies, Inc.
/// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/QCPParserLib/main/latest/inc/qcpconstants.h#6 $
//$DateTime: 2011/03/29 11:00:40 $
//$Change: 1676557 $

//=============================================================================

#ifndef QCP_CONSTANTS_H
#define QCP_CONSTANTS_H


//=============================================================================
// CONSTANTS
//=============================================================================
//QCP  File Header size

#define QCP_FRAME_HEADER_SIZE 1

#define QCP_MIN_BUFFER_SIZE 35         // One Frame Size @ 13 kbps

#define QCP_MAX_BUFFER_SIZE 800        // 500ms data @ 13 kbps

#define QCP_READ_BUFFER_SIZE 875
#define QCP_PREF_BUFF_NUM 4

#define QCP_FIELD_OFFSET 4

#define QCP_FRAME_DURATION 20 //in msec (it is fixed)

#define QCP_CACHE_SIZE (QCP_MAX_BUFFER_SIZE *5)

#define MILLISECONDS 1000

#define QCP_CHUNK_HEADER_SIZE 8

#define QCP_RIFF_HEADER_SIZE 12

#define QCP_FMT_HEADER_SIZE 158

#define QCP_VRAT_HEADER_SIZE 16

#define QCP_LABL_HEADER_SIZE 56

#define QCP_OFFS_HEADER_SIZE 20

#define QCP_Fixed_DATA_HEADER_SIZE 8  //Additionally Padding may be supported

#define QCP_CNFG_HEADER_SIZE 10

#define QCP_Fixed_TEXT_HEADER_SIZE 8  //Additionally Padding may be supported

#define QCP_FILE_HEADER_SIZE 270 // size of [riff + fmt + vrat + labl + offs + data chunks]


#endif

