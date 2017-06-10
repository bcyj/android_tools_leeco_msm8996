#ifndef __BOX_RECORD_H__
#define __BOX_RECORD_H__
/* =======================================================================
                              boxrecord.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/boxrecord.h#7 $
$DateTime: 2011/11/24 00:50:52 $
$Change: 2061573 $


========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "atomutils.h"

/* =======================================================================
**                        Class Declarations
** ======================================================================= */
class BoxRecord
{

public:
  BoxRecord (OSCL_FILE *fp); // Default constructor
  BoxRecord (uint8 *&buf); // Default constructor
  virtual ~BoxRecord ()
  {
  }; // Destructor

  int16 getBoxTop()
  {
    return _top;
  }

  int16 getBoxLeft()
  {
    return _left;
  }

  int16 getBoxBottom()
  {
    return _bottom;
  }

  int16 getBoxRight()
  {
    return _right;
  }

  bool  FileSuccess()
  {
    return _success;
  }

  PARSER_ERRORTYPE GetFileError()
  {
    return _fileErrorCode;
  }

private:
  int16 _top;
  int16 _left;
  int16 _bottom;
  int16 _right;

  bool  _success;
  PARSER_ERRORTYPE _fileErrorCode;
};


#endif /* __BOX_RECORD_H__ */
