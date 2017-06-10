#ifndef AVI_ERROS_H
#define AVI_ERROS_H

/* =======================================================================
                              aviErrors.h
DESCRIPTION
  
Copyright 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/avierrors.h#6 $
========================================================================== */

/*
* Lists various error types used/reported by AVI parser.
*
* AVI_UNKNOWN_ERROR:Unknown error.

* AVI_PARSE_ERROR:Error while parsing.

* AVI_FAILURE: Operation/API failed.

* AVI_READ_FAILURE:  Failure to read data from the file.

* AVI_CORRUPTED_FILE: Read in data is not same as being expected.

* AVI_INVALID_USER_DATA:Parameter passed in to an API is illegal.

* AVI_OUT_OF_MEMORY:Parser failed to allocate memory.

* AVI_SUCCESS:Indicates successful operation/API call.

* AVI_END_OF_FILE: Returned when retrieving sample info.

*/

typedef enum aviError
{  
  AVI_UNKNOWN_ERROR,
  AVI_PARSE_ERROR,
  AVI_FAILURE,
  AVI_READ_FAILURE,    
  AVI_CORRUPTED_FILE,
  AVI_INVALID_USER_DATA,
  AVI_OUT_OF_MEMORY,  
  AVI_SUCCESS,    
  AVI_END_OF_FILE,
  AVI_INSUFFICIENT_BUFFER,
  AVI_DATA_UNDERRUN
}aviErrorType;
#endif
