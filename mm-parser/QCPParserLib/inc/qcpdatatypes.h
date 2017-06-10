#ifndef QCP_HDR_DATA_TYPES_H
#define QCP_HDR_DATA_TYPES_H

/* =======================================================================
               qcpdatatypes.h
DESCRIPTION
  
Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/QCPParserLib/main/latest/inc/qcpdatatypes.h#5 $
$DateTime: 2011/03/29 11:00:40 $
$Change: 1676557 $

========================================================================== */

//Data types used
typedef unsigned int        qcp_uint32;
typedef int                 qcp_int32;
typedef short               qcp_int16;
typedef unsigned short      qcp_uint16;
typedef unsigned char       qcp_uint8;
typedef char                qcp_int8;
typedef signed long long    qcp_int64;      
typedef unsigned long long  qcp_uint64;     
typedef qcp_uint64          QCP_FILE_OFFSET;
typedef unsigned char       qcp_boolean;

//QCP Parser states.
typedef enum qcpt_parserState
{
  //Initial Parser state when parser handle is created.
  QCP_PARSER_IDLE,
  //Parser state when parsing begins.
  QCP_PARSER_INIT,
  //Parser state when parsing is successful.
  QCP_PARSER_READY,
  //Parser is seeking to a given timestamp.
  QCP_PARSER_SEEK,
  //Read failed
  QCP_PARSER_READ_FAILED,
  //Data being read is corrupted.
  QCP_PARSER_FAILED_CORRUPTED_FILE,
  //Parser state it fails to allocate memory,
  //that is, when malloc/new fails.
  QCP_PARSER_NO_MEMORY, 
}qcpParserState;


#endif
