#ifndef __Atom_H__
#define __Atom_H__
/* =======================================================================
                              atom.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2008-2013 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/atom.h#11 $
$DateTime: 2012/11/27 20:01:35 $
$Change: 3077641 $


========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"

#include "oscl_file_io.h"
#include "parentable.h"
#include "renderable.h"
#include "isucceedfail.h"


#define DEFAULT_ATOM_SIZE 8 //need to change when they use various size field

class Atom : public Parentable, public Renderable, public ISucceedFail
{

public:
  Atom(uint32 type);
  Atom(OSCL_FILE *fp); // file pointer constructor
  Atom(uint8* &buf);
  virtual ~Atom(){return;}

  // Member get methods
  virtual uint32 getSize() const { return _size;}
  uint32 getType() const { return _type;}
  virtual uint64 getOffsetInFile() const { return _offsetInFile;}
#ifndef __CC_ARM
  inline
#endif
  virtual uint32 getDefaultSize() const{ return DEFAULT_ATOM_SIZE;}//tag+size field length

protected:
  uint32 _size; // 4 (32bits)
  uint32 _type; // 4 (32bits)
  uint64 _offsetInFile; //  (64bits)

private:


};

#endif /* __Atom_H__ */
