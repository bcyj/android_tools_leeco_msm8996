#ifndef _REFERENCE_COUNTED_POINTER_
#define _REFERENCE_COUNTED_POINTER_


/* =======================================================================
      ReferenceCountedPointer.h

DESCRIPTION
  This module defines the ReferenceCountedPointer class. It is a "smart"
  pointer for objects that implement IReferenceCountable or IUnknown.
  It automatically calls AddRef() and Release() on construction, destruction,
  and assignment.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/ReferenceCountedPointer.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "IReferenceCountable.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */
template<class T>
class ReferenceCountedPointer
{
public:

  ReferenceCountedPointer()
    : m_ptr(NULL)
  {
  }

  ReferenceCountedPointer(T *ptr)
    : m_ptr(ptr)
  {
    if (m_ptr != NULL)
    {
      m_ptr->AddRef();
    }
  }

  ReferenceCountedPointer(const ReferenceCountedPointer &that)
    : m_ptr(that.m_ptr)
  {
    if (m_ptr != NULL)
    {
      m_ptr->AddRef();
    }
  }

  ~ReferenceCountedPointer()
  {
    if (m_ptr != NULL)
    {
      m_ptr->Release();
      m_ptr = NULL;
    }
  }

  ReferenceCountedPointer &operator = (const ReferenceCountedPointer &that)
  {
    if (this == &that)
    {
      return *this;
    }

    // The order here ensures we don't delete something only to AddRef() to
    // it later.
    if (that.m_ptr != NULL)
    {
      that.m_ptr->AddRef();
    }

    if (m_ptr != NULL)
    {
      m_ptr->Release();
    }

    m_ptr = that.m_ptr;

    return *this;
  }

  ReferenceCountedPointer &operator = (T *ptr)
  {
    if (m_ptr == ptr)
    {
      return *this;
    }

    // The order here ensures we don't delete something only to AddRef() to
    // it later.
    if (ptr != NULL)
    {
      ptr->AddRef();
    }

    if (m_ptr != NULL)
    {
      m_ptr->Release();
    }

    m_ptr = ptr;

    return *this;
  }

  // Adapted from "Modern C++ Design: Generic Programming and Design Patterns
  // Applied" by Andrei Alexandrescu.
  friend bool operator == (const ReferenceCountedPointer &lhs,
    const T *rhs)
  {
    return (lhs.m_ptr == rhs);
  }

  friend bool operator == (const T *lhs,
    const ReferenceCountedPointer &rhs)
  {
   return (lhs == rhs.m_ptr);
  }

  friend bool operator != (const ReferenceCountedPointer &lhs,
    const T *rhs)
  {
    return (lhs.m_ptr != rhs);
  }

  friend bool operator != (const T *lhs,
    const ReferenceCountedPointer &rhs)
  {
    return lhs != rhs.m_ptr;
  }

  T &operator * () const
  {
    return *m_ptr;
  }

  // You can call the raw pointer's method directly using this operator,
  // e.g. if T has the method Foo(), can write:
  // ReferenceCountedPointer<T> ptr;
  // ptr->Foo();
  //
  // Usage note: Do not call rcp->AddRef() or rcp->Release()!
  T *operator -> () const
  {
    return m_ptr;
  }

  // Given a pointer to be used as an OUT pointer, fill in the OUT pointer
  // and AddRef() it if it's non-NULL.
  void SaveToOutPointer(T **ppT /* out */)
  {
    if (ppT != NULL)
    {
      if (m_ptr != NULL)
      {
        m_ptr->AddRef();
      }
      *ppT = m_ptr;
    }
  }

  // Do you know what you are doing? If you're unsure, don't call this method!
  T *GetRawPointer() { return m_ptr; }

  // Used to pass in a ReferenceCountedPointer as an IN parameter.
  // This ensures that the pointer is never AddRefed or Released.
  friend T *MakeInPointer(ReferenceCountedPointer<T> &ptr)
  {
    return ptr.m_ptr;
  }

  // Used to pass in a ReferenceCountedPointer as an OUT parameter.
  // This ensures that the pointer is AddRefed only once and that
  // the previous pointer value is released.
  friend T **MakeOutPointer(ReferenceCountedPointer<T> &ptr)
  {
    ptr = NULL;
    return &(ptr.m_ptr);
  }

  // Used to pass in a ReferenceCountedPointer as an IN/OUT parameter.
  // This ensures that the pointer is AddRefed only once.
  friend T **MakeInOutPointer(ReferenceCountedPointer<T> &ptr)
  {
    return &(ptr.m_ptr);
  }

private:

  T *m_ptr;
};
#endif /* _REFERENCE_COUNTED_POINTER_ */
