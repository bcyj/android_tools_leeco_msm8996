#ifndef OSCL_STRING_H
#define OSCL_STRING_H
/* =======================================================================
                               oscl_string.h
DESCRIPTION
//    This is a simple string class without any multithread access
//    protection.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/oscl_string.h#13 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "oscl_types.h"
#include "qtv_msg.h"
#include "SourceMemDebug.h"
#include "IPStreamSourceUtils.h"
#include <stdlib.h>

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* This macro is used on string literals to determine if they are
   of type wchar_t or int8. */

#ifndef _T
#ifdef UNICODE
  #define _T(x) L ## x
#else
  #define _T(x) x
#endif
#endif

template <class C>
class OSCL_String;

template<class C>
int32 operator== (const OSCL_String<C>& a, const OSCL_String<C>& b);
template<class C>
int32 operator!= (const OSCL_String<C>& a, const OSCL_String<C>& b);
template<class C>
int32 operator<  (const OSCL_String<C>& a, const OSCL_String<C>& b);
template<class C>
int32 operator<= (const OSCL_String<C>& a, const OSCL_String<C>& b);
template<class C>
int32 operator>  (const OSCL_String<C>& a, const OSCL_String<C>& b);
template<class C>
int32 operator>= (const OSCL_String<C>& a, const OSCL_String<C>& b);


/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Function Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  OSCL_String_Srep

DESCRIPTION
    OSCL_String is a simple string class
    which is compatible with regular character array
    strings as well as Unicode wchar_t array strings.

    The class uses a copy-on-write to minimize unnecessary
    copying when multiple instances of a string are created
    for reading.  Allocated memory is automatically freed by
    the class destructor when the last string referencing the
    memory is destroyed.  The class HAS NO thread synchronization
    built-in, so it is NOT MT-SAFE.  External locks should be used
    if the class is to be shared across threads.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
#ifdef FEATURE_QTV_STRING_REFCNT
template <class C>
struct OSCL_String_Srep
{
  public:
  C *buffer; // holds actual string value
  int32 size; // number of elements;
  int32 refcnt; // reference count;

  OSCL_String_Srep(uint32 nsz, const C *src)
  {
    refcnt = 1;
    size = nsz;
    if ( size > 0 )
    {
      /* allocate enough space including terminator */
      buffer = QTV_New_Array(C, size+1 );
      /* copy the buffer into ours - automatically null-terminates */
      std_strlcpy(buffer,src,size+1);
    }
    else
    {
      buffer = NULL;
    }

  }

  ~OSCL_String_Srep()
  {
    if (buffer)
    {
      QTV_Delete_Array( buffer );
    }
  }

  OSCL_String_Srep* get_own_copy()
  {
    if ( 1 == refcnt )
    {
      // already a private copy so return
      return this;
    }

    --refcnt; // decrement reference

    OSCL_String_Srep *tmp = NULL;
    tmp = QTV_New_Args(OSCL_String_Srep, (size, buffer));
    return tmp;
  }

  void assign(int32 nsz, const C *src)
  {

    if ( size != nsz )
    {
      QTV_Delete_Array( buffer );
      size = nsz;
      buffer = QTV_New_Array( C, size+1 );
    }

    /* strlcpy automatically null-terminates */
    std_strlcpy(buffer, src, size+1);
  }

  private:
  OSCL_String_Srep(const OSCL_String_Srep&);
  OSCL_String_Srep& operator=(const OSCL_String_Srep&);

};
#endif /* FEATURE_QTV_STRING_REFCNT */

#ifdef FEATURE_QTV_STRING_REFCNT
#define BUFFER rep->buffer
#else
#define BUFFER m_buffer
#endif /* FEATURE_QTV_STRING_REFCNT */

/* ======================================================================
CLASS
  OSCL_String

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template <class C>
class OSCL_String
{
public:

  /// Default constructor -- simply creates an empty string
  OSCL_String();

  /// Copy constructor from character array
  OSCL_String(const C *cp);

  /// Copy constructor from character array, but allocates
  /// length according to the length parameter.
  OSCL_String(const C *src, uint32 length);

  /// Copy constructor from another OSCL_String
  OSCL_String(const OSCL_String<C>& src);

  /// Assignment operator from a character array
  OSCL_String<C>& operator=(const C *);

  /// Assignment operator from another OSCL_String
  OSCL_String<C>& operator=(const OSCL_String<C> &);

  // These don't seem to have any effect on the Metrowerks ARM compiler ...
  friend int32 operator== <>(const OSCL_String<C>& a, const OSCL_String<C>& b);
  friend int32 operator!= <>(const OSCL_String<C>& a, const OSCL_String<C>& b);
  friend int32 operator<  <>(const OSCL_String<C>& a, const OSCL_String<C>& b);
  friend int32 operator<= <>(const OSCL_String<C>& a, const OSCL_String<C>& b);
  friend int32 operator>  <>(const OSCL_String<C>& a, const OSCL_String<C>& b);
  friend int32 operator>= <>(const OSCL_String<C>& a, const OSCL_String<C>& b);

  ~OSCL_String();

  // ZREX Needs to remap read (Stream I/O to zrex_read for Stream I/O
  // the names read and read clash, whereas Read and read do not
  // ML
  C Read(int32 index, oscl_status_t & status) const;
  oscl_status_t write(int32 index, C c);
  void write(uint32 offset, uint32 length, C *ptr);

  /// Access functions for the string size
  size_t get_size() const;
  size_t size() const
  {
    return get_size();
  }

  // Return the capacity that this buffer can hold without reallocating.
  uint32 get_capacity() const
  {
#ifdef FEATURE_QTV_STRING_REFCNT
    return get_size();
#else
    return m_capacity;
#endif /* FEATURE_QTV_STRING_REFCNT */
  }

  bool ensure_capacity(int32 capacity);

  /// Access function for the C-style string
  const C * get_cstr() const;
  const C * c_str() const
  {
    return get_cstr();
  }

  /// Append a c-style string
  OSCL_String<C>& operator+=(const C* src);

  /// Append another OSCL_String to this OSCL_String
  OSCL_String<C>& operator+=(const OSCL_String<C>& src);

  /// Append a single character
  OSCL_String<C>& operator+=(const C c);

  OSCL_String<C>& set(const C *cp, int32 length);

  operator const C *() const { return get_cstr(); }

  C operator[] (int32 index) const;

  // Needed so that (OSCL_String == const char *) does not convert OSCL_String
  // and do (const char * == const char *) which would be only a pointer
  // comparison.
  int32 operator==(const char *b) const
  {
    return ((const OSCL_String &) *this == (const OSCL_String &) OSCL_String(b));
  }

  uint32 hash() const;

#ifndef FEATURE_QTV_STRING_REFCNT
  OSCL_String<C> &append(const C *cp);
  OSCL_String<C> &append(const C *cp, int32 length);
  OSCL_String<C> &append(const OSCL_String<C> &suffix);
#endif /* FEATURE_QTV_STRING_REFCNT */

protected:

#ifndef FEATURE_QTV_STRING_REFCNT
  void deallocate();
  void assign(const C *cp);
  void assign(const C *cp, int32 length);
  void assign(const OSCL_String<C> &src);
#endif /* FEATURE_QTV_STRING_REFCNT */

  // Not needed anymore!  struct Srep;   // Note this is a forward declaraton only, allocates no memory.
#ifdef FEATURE_QTV_STRING_REFCNT
  typedef OSCL_String_Srep<C> Srep;
  Srep *rep;
#else
  C *m_buffer;
  size_t m_size;
  size_t m_capacity;
  static const C* EMPTY_STRING;
#endif /* FEATURE_QTV_STRING_REFCNT */
};

template<class C> const C* OSCL_String<C>::EMPTY_STRING = "";

#ifndef FEATURE_QTV_STRING_REFCNT
template<class C>
void OSCL_String<C>::deallocate()
{
  if ( m_buffer != NULL )
  {
    QTV_Free( m_buffer );
    m_buffer = NULL;
  }

  m_buffer = NULL;
  m_size = 0;
  m_capacity = 0;
}

template<class C>
void OSCL_String<C>::assign(const C *cp)
{
  if (cp == NULL)
  {
    cp = "";
  }

  size_t size = std_strlen(cp);

  m_buffer = (C*)QTV_Malloc(sizeof(C) * (size + 1 ));
  if(m_buffer)
  {
    std_strlcpy( BUFFER, cp, size + 1);
  }

  m_capacity = size;
  m_size = size;
}

template<class C>
void OSCL_String<C>::assign(const C *cp, int32 length)
{
  if (( cp == NULL ) || (length <= 0))
  {
    cp = "";
    length = 0;
  }

  m_buffer = (C *)QTV_Malloc(sizeof(C) * (length + 1 ));

  if (m_buffer == NULL)
  {
    m_size = 0;
    m_capacity = 0;
    return;
  }

  /* strlcpy automatically null-terminates */
  std_strlcpy( m_buffer, cp, length+1 );
  m_size = length;
  m_capacity = length;
}

template<class C>
void OSCL_String<C>::assign(const OSCL_String<C> &src)
{
  if (src.m_buffer == NULL)
  {
    QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_FATAL,
                 "assign: src buffer is NULL!");
    return;
  }

  size_t size = src.size();

  if (size <= 0)
  {
    QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_FATAL,
                  "assign: src size = %d <= 0!", size);
    return;
  }

  m_buffer = (C *)QTV_Malloc(sizeof(C) * (size + 1));

  if (m_buffer == NULL)
  {
    m_size = 0;
    m_capacity = 0;
    return;
  }

  std_strlcpy( m_buffer, src.m_buffer, size + 1 );
  m_size = size;
  m_capacity = size;
}

template<class C>
OSCL_String<C>& OSCL_String<C>::append(const C *src)
{
  if (src == NULL)
  {
    return append("", 0);
  }
  return append(src, std_strlen(src));
}

template<class C>
OSCL_String<C>& OSCL_String<C>::append(const C *src, int32 length)
{
  if (!ensure_capacity(length + m_size) || (m_buffer == NULL))
  {
    QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_FATAL,
      "Insufficient memory to append!");
    return *this;
  }

  // Copy starting at null character (automatically null-terminates)
  std_strlcpy(m_buffer + m_size, src, length + 1);

  m_size += length;

  return *this;
}

template<class C>
OSCL_String<C>& OSCL_String<C>::append(const OSCL_String<C> &suffix)
{
  return append(suffix.get_cstr(), suffix.size());
}
#endif /* !FEATURE_QTV_STRING_REFCNT */
/* ======================================================================
FUNCTION
  OSCL_String<C>::OSCL_String

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>::OSCL_String()
{
#ifdef FEATURE_QTV_STRING_REFCNT
  C *ptr = NULL;
  rep = QTV_New_Args(Srep, (0, ptr));
#else
  assign("", 0);
#endif /* FEATURE_QTV_STRING_REFCNT */
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::OSCL_String

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>::OSCL_String(const C *cp)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  if ( cp )
    cp = QTV_New_Args(Srep, (std_strlen(cp), cp));
  else
    cp = QTV_New_Args(Srep, (0, cp));
#else
  assign(cp);
#endif /* FEATURE_QTV_STRING_REFCNT */
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::OSCL_String

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>::OSCL_String(const C *cp, uint32 length)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  rep = QTV_New_Args(Srep, (length,cp));
#else
  assign(cp, length);
#endif /* FEATURE_QTV_STRING_REFCNT */
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::OSCL_String

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>::OSCL_String(const OSCL_String<C> &src )
{
#ifdef FEATURE_QTV_STRING_REFCNT
  src.rep->refcnt++;
  rep = src.rep;
  QTV_MSG2( QTVDIAG_DEBUG, "OSCL_String copy ctor, refcnt=%d, size=%d",
            src.rep->refcnt, src.rep->size );
#else
  assign(src);
#endif /* FEATURE_QTV_STRING_REFCNT */
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::~OSCL_String

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>::~OSCL_String()
{
#ifdef FEATURE_QTV_STRING_REFCNT
  if ( --rep->refcnt == 0 )
  {
    QTV_Delete( rep );
  }
#else
  deallocate();
#endif /* FEATURE_QTV_STRING_REFCNT */
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::Read

DESCRIPTION
// ZREX Needs to remap read (Stream I/O to zrex_read for Stream I/O
// the names read and read clash, whereas Read and read do not
// ML

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
C OSCL_String<C>::Read(int32 index, oscl_status_t & status) const
{
  if ((index < 0) || (index >= size()))
  {
    status = EINDEXOUTOFBOUND;
    return NULL;
  }

  status = SUCCESS;
  return BUFFER[index];
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::write

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
oscl_status_t OSCL_String<C>::write(int32 index, C c)
{
  if ((index < 0) || (index >= size()))
  {
    return EINDEXOUTOFBOUND;
  }

#ifdef FEATURE_QTV_STRING_REFCNT
  rep = rep->get_own_copy();
#endif /* FEATURE_QTV_STRING_REFCNT */

  BUFFER[index] = c;
  return SUCCESS;
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::write

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
void OSCL_String<C>::write(uint32 offset, uint32 length, C *ptr)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  rep = rep->get_own_copy();
#endif /* FEATURE_QTV_STRING_REFCNT */
  int32 to_copy = (length > size() - offset) ? (size() - offset) : length;
  memcpy( BUFFER + offset, ptr, to_copy );
  to_copy += offset;
  if ( to_copy < size() )
  {
    BUFFER[to_copy] = '\0';
  }
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::get_cstr

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
const C * OSCL_String<C>::get_cstr() const
{
#ifndef FEATURE_QTV_STRING_REFCNT
  if ( BUFFER == NULL )
  {
    return OSCL_String<C>::EMPTY_STRING;
  }
#endif /* FEATURE_QTV_STRING_REFCNT */
  return BUFFER;
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::get_size

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
size_t OSCL_String<C>::get_size() const
{
#ifdef FEATURE_QTV_STRING_REFCNT
  return rep->size;
#else
  return m_size;
#endif /* FEATURE_QTV_STRING_REFCNT */
}

// Ensure this buffer has enough room for size characters without
// reallocating.
template<class C>
bool OSCL_String<C>::ensure_capacity(int32 capacity)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  (void)capacity;
#else
  if (m_capacity < capacity)
  {
    C *buffer_to_dealloc = m_buffer;

    m_capacity = QTV_MAX((capacity << 1) - 1, 15);

    // even though C is char, it could be anything else...if C happens to be a
    // class, Qtv_Malloc and other pieces of code here won't work.
    m_buffer = (C *)QTV_Malloc(sizeof(C) * (m_capacity + 1));

    if (m_buffer == NULL)
    {
      m_capacity = capacity;
      m_buffer = buffer_to_dealloc;
      return false;
    }

    // Copy the original string back to the new array.
    std_strlcpy(m_buffer, buffer_to_dealloc, m_capacity + 1);

    QTV_Free( buffer_to_dealloc );
  }
#endif /* FEATURE_QTV_STRING_REFCNT */

  return true;
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::operator=

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>& OSCL_String<C>::operator=(const OSCL_String<C> &src)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  if ( rep == src.rep )
  {
    return(*this);  // protect against "str = str"
  }
  src.rep->refcnt++;
  if ( --rep->refcnt == 0 )
  {
    QTV_Delete( rep );
  }
  QTV_MSG2( QTVDIAG_DEBUG, "OSCL_String op=, refcnt=%d, size=%d",
            src.rep->refcnt, src.rep->size );

  rep = src.rep;
#else
  deallocate();
  assign(src);
#endif /* FEATURE_QTV_STRING_REFCNT */

  return *this;
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::operator=

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>& OSCL_String<C>::operator=(const C *cp)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  if ( --rep->refcnt == 0 )
  {
    QTV_Delete( rep );
  }

  if ( cp == NULL )
  {
    rep = QTV_New_Args(Srep, (0, cp) );
  }
  else
  {
    rep = QTV_New_Args(Srep, (std_strlen(cp), cp));
  }
#else
  deallocate();
  assign(cp);
#endif /* FEATURE_QTV_STRING_REFCNT */

  return *this;
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::operator+=

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>& OSCL_String<C>::operator+=(const C* src)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  int32 new_size = size() + std_strlen(src);
  Srep *new_rep;
  new_rep = QTV_New_Args(Srep, (new_size, BUFFER));
  std_strlcat(new_rep->buffer, src, new_size);
  if ( --rep->refcnt == 0 )
  {
    QTV_Delete( rep );
  }
  rep = new_rep;

  return *this;
#else
  return append(src);
#endif /* FEATURE_QTV_STRING_REFCNT */
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::operator+=

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>& OSCL_String<C>::operator+=(const OSCL_String& src)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  Srep *new_rep = NULL;
  int32 new_size = rep->size + src.rep->size;
  new_rep = QTV_New_Args(Srep, (new_size, BUFFER));
  std_strlcat(new_rep->buffer, src.BUFFER, new_size);
  if ( --rep->refcnt == 0 )
  {
    QTV_Delete( rep );
  }
  rep = new_rep;

  return *this;
#else
  return append(src);
#endif /* FEATURE_QTV_STRING_REFCNT */
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::operator+=

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
OSCL_String<C>& OSCL_String<C>::operator+=(const C c)
{
  C tmp_str[2];
  tmp_str[0] = c;
  tmp_str[1] = (C) '\0';

  operator+=(tmp_str);
  return *this;
}

/* ======================================================================
FUNCTION
  OSCL_String<C>::operator[]

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
C OSCL_String<C>::operator[](int32 index) const
{
  if ( (index < 0) || (index >= size()) )
  {
    QTV_MSG_PRIO2(QTVDIAG_GENERAL, QTVDIAG_PRIO_FATAL,
                  "Invalid string access, size = %d, index = %d", size(), index);
    return '\0';
  }
  return BUFFER[index];
}

template<class C>
OSCL_String<C>& OSCL_String<C>::set(const C *cp, int32 length)
{
#ifdef FEATURE_QTV_STRING_REFCNT
  if ( --rep->refcnt == 0 )
  {
    QTV_Delete( rep );
  }

  if ( cp == NULL )
  {
    rep = QTV_New_Args(Srep, (0, cp));
  }
  else
  {
    rep = QTV_New_Args(Srep, (length, cp));
  }
#else
  deallocate();
  assign(cp, length);
#endif /* FEATURE_QTV_STRING_REFCNT */

  return *this;
}


/* ======================================================================
FUNCTION
  OSCL_String<C>::hash()

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
template<class C>
uint32 OSCL_String<C>::hash() const
{
  uint32 h = 0;
  int32 ii;
  C *ptr;

  for ( ii = 0, ptr = BUFFER ; ii < (int32)size(); ++ii, ++ptr )
  {
    h = 5 * h + *ptr;
  }

  return h;
}

/* Set character code to ASCII or UNICODE */
#ifdef __cplusplus
typedef OSCL_String<char>           OSCL_STRING;
#endif /* __cplusplus */


/* Note that in the following operators, the protected buffer field cannot be
 * dereferenced even though these operators are declared as friends. This seems
 * to be a limitation of the ARM compiler; VC++ does allow the
 * access.
 */

/* ======================================================================
FUNCTION
  operator==(const OSCL_STRING &a, const OSCL_STRING &b)

DESCRIPTION
  Compares two strings, case sensitively, for equality.

DEPENDENCIES
  None.

RETURN VALUE
  1 if the strings are equal, 0 otherwise.

SIDE EFFECTS
  None.

========================================================================== */
template<class C>
int32 operator==(const OSCL_String<C>& a, const OSCL_String<C>& b)
{
#ifndef FEATURE_QTV_STRING_REFCNT
  if (a.size() != b.size())
  {
    return 0;
  }
#endif /* FEATURE_QTV_STRING_REFCNT */

  return ((std_strcmp(a.get_cstr(), b.get_cstr()) == 0) ? 1 : 0);
}

/* ======================================================================
FUNCTION
  operator!=(const OSCL_STRING &a, const OSCL_STRING &b)

DESCRIPTION
  Compares two strings, case sensitively, for equality.

DEPENDENCIES
  None.

RETURN VALUE
  0 if the strings are equal, 1 otherwise.

SIDE EFFECTS
  None.

========================================================================== */
template<class C>
int32 operator!=(const OSCL_String<C>& a, const OSCL_String<C>& b)
{
  return (((a == b) == 0) ? 1 : 0);
}

/* ======================================================================
FUNCTION
  operator>(const OSCL_STRING &a, const OSCL_STRING &b)

DESCRIPTION
  Compares two strings, case sensitively and lexically.

DEPENDENCIES
  None.

RETURN VALUE
  1 if a is lexically greater than b, 0 otherwise.

SIDE EFFECTS
  None.

========================================================================== */
template<class C>
int32 operator>(const OSCL_String<C>& a, const OSCL_String<C>& b)
{
  return (std_strcmp(a.get_cstr(), b.get_cstr()) > 0);
}

/* ======================================================================
FUNCTION
  operator>=(const OSCL_STRING &a, const OSCL_STRING &b)

DESCRIPTION
  Compares two strings, case sensitively and lexically.

DEPENDENCIES
  None.

RETURN VALUE
  1 if a is lexically greater than or equal to b, 0 otherwise.

SIDE EFFECTS
  None.

========================================================================== */
template<class C>
int32 operator>=(const OSCL_String<C>& a, const OSCL_String<C>& b)
{
  return (std_strcmp(a.get_cstr(), b.get_cstr()) >= 0);
}

/* ======================================================================
FUNCTION
  operator<=(const OSCL_STRING &a, const OSCL_STRING &b)

DESCRIPTION
  Compares two strings, case sensitively and lexically.

DEPENDENCIES
  None.

RETURN VALUE
  1 if a is lexically less than or equal to b, 0 otherwise.

SIDE EFFECTS
  None.

========================================================================== */
template<class C>
int32 operator<=(const OSCL_String<C>& a, const OSCL_String<C>& b)
{
  return (std_strcmp(a.get_cstr(), b.get_cstr()) <= 0);
}

/* ======================================================================
FUNCTION
  operator>(const OSCL_STRING &a, const OSCL_STRING &b)

DESCRIPTION
  Compares two strings, case sensitively and lexically.

DEPENDENCIES
  None.

RETURN VALUE
  1 if a is lexically less than b, 0 otherwise.

SIDE EFFECTS
  None.

========================================================================== */
template<class C>
int32 operator<(const OSCL_String<C>& a, const OSCL_String<C>& b)
{
  return (std_strcmp(a.get_cstr(), b.get_cstr()) < 0);
}

#endif /* OSCL_STRING_H */
