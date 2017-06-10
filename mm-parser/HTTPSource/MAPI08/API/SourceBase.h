#ifndef __SOURCEBASE_H__
#define __SOURCEBASE_H__
/************************************************************************* */
/**
 * @file SourceBase.h
 * @brief Header file for iSourceBase interface definition.
 * 
 * Copyright 2008 Qualcomm Technologies, Inc., All Rights Reserved.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/API/DataSourcePort/main/latest/inc/SourceBase.h#3 $
$DateTime: 2011/04/20 11:29:31 $
$Change: 1710564 $

========================================================================== */
/* =======================================================================
**               Include files for SourceBase.h
** ======================================================================= */
#include <AEEStdDef.h>

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
/**
iSourceBase
The iSourceBase interface is used for generic reference counting and for
interface discovery.
*/
class iSourceBase
{
public:
  /** iSourceBase interface ID
  */
  static const AEEIID SOURCEBASE_IID = 0x01053A30;

  //As with all component systems, a successful QueryInterface call increases
  //the reference count - QueryInterface should not be followed with AddRef
  virtual void* QueryInterface(const AEEIID iid) = 0;
  virtual uint32 AddRef() = 0;
  virtual uint32 Release() = 0;
  virtual ~iSourceBase(){};
};

} /* namespace video */
#endif /* __SOURCEBASE_H__ */
