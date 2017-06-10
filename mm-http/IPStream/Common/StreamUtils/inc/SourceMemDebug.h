
#ifndef SOURCEMEMDEBUG_H
#define SOURCEMEMDEBUG_H
/************************************************************************* */
/**
 * SourceMemDebug.h
 *
 * @brief Heap Allocation Interface for IPStream modules
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/SourceMemDebug.h#8 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */
#include "MMMemory.h"

#define QTV_Malloc(sz)          MM_Malloc(sz)

#define QTV_Realloc(pMem, sz)   MM_Realloc(pMem, sz)

#define QTV_Free(v)             MM_Free(v)

#define QTV_New(t)              MM_New(t)

#define QTV_New_Args(t,a)       MM_New_Args(t,a)

#define QTV_Delete(v)           MM_Delete(v)

#define QTV_New_Array(t,n)      MM_New_Array(t,n)

#define QTV_Delete_Array(pMem) MM_Delete_Array(pMem)

#define RETURN_VOID /* null macro */

#define QTV_NULL_PTR_CHECK(ptr, retval)                          \
  if( NULL == (ptr) )                                            \
  {                                                              \
    QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_DEBUG,            \
      "Unexpected Null Ptr");                                    \
    return retval;                                               \
  }

/**
 * This macro checks if pointer X going beyond pointer Y + length Z
 *
 * @Param[in] X - Pointer which is going for bound check
 * @param[in] Y - Pointer is maximum limit of X pointer
 * 2param[in] Z - value which is going to increment in pointer X
 *
 * @return  value assign to retval
 */

#define BOUNDS_CHECK_FOR_PTR(X, Y, Z, retval)               \
 if ((Z < 0) || ((X) >= ((Y) + sizeof(Y) - (Z))))           \
 {                                                          \
   return retval;                                           \
 }

#endif // SOURCEMEMDEBUG_H
