/* =======================================================================
                              ztl.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright 2012 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/ztl.h#12 $
$DateTime: 2012/06/21 23:15:24 $
$Change: 2526572 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#if !defined(_ztl_h)  // Sentry, use file only if it's not already included.
#define _ztl_h

#include <stdlib.h>
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "oscl_file_io.h"
#include "limits.h"
#include "MMDebugMsg.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
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
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  ZArrayBase
DESCRIPTION
  Base class for Z arrays
========================================================================== */
class ZArrayBase
{
  const static int32 GROW_BY = 16;

  int32 m_nElementSize;
  int32 m_nLength, m_nSize;
protected:
  void *m_pBuffer;
/* ======================================================================
FUNCTION:
  ZArrayBase::ZArrayBase

DESCRIPTION:
  Constructor

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
========================================================================== */
  ZArrayBase (int32 nElementSize) :
    m_nElementSize (nElementSize),
    m_nLength (0),
    m_nSize (0),
    m_pBuffer (NULL)
  {}
/* ======================================================================
FUNCTION:
  ZArrayBase::~ZArrayBase

DESCRIPTION:
  Destructor
INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
========================================================================== */
  virtual ~ZArrayBase () {  if (m_pBuffer) MM_Free( m_pBuffer );}
/* ======================================================================
FUNCTION:
  ZArrayBase::Clear

DESCRIPTION:
  Clear

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
========================================================================== */
  void Clear () {m_nLength = 0;}
  int32 Stress (int32 nSize);
/* ======================================================================
FUNCTION:
  ZArrayBase :: Increase

DESCRIPTION:
  Increase length

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  Return new length

SIDE EFFECTS:
  None.
========================================================================== */
  int32 Increase ()
  {
    int32 nResult = m_nLength;
          if((MakeRoomAt (m_nLength)) == -1)
          {
            return -1;
          }
    return nResult;
  }

  int32 MakeRoomAt (int32 i);
  int32 DeleteFrom (int32 i);
/* ======================================================================
FUNCTION:
  ZArrayBase :: DeleteLast

DESCRIPTION:
  Delete last element

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
========================================================================== */
  int32 DeleteLast ()
  {
          if(m_nLength > 0)
          {
            if((DeleteFrom (m_nLength - 1)) == -1)
            {
              return -1;
            }
          }
          return m_nLength;
    }

public:

/* ======================================================================
FUNCTION:
  ZArrayBase :: MakeRoomFor

DESCRIPTION:
Make room for i number of elements for the fragmentInfoArray initially through malloc.
This would avoid numerous realloc of fragmentInfoArray during the playback of a large fragmented file
that may lead to heap fragmentation


INPUT/OUPUT PARAMETERS:


RETURN VALUE:
-1 in case of error

SIDE EFFECTS:
  None.
========================================================================== */
  int32 MakeRoomFor(int32 i );



/* =======================================================================
FUNCTION:
  ZArrayBase :: GetLength

DESCRIPTION:
  Returns length(Get number of elements)

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  Returns length

SIDE EFFECTS:
  None.
========================================================================== */
  uint32 GetLength () const {return m_nLength;}
};


/* ======================================================================
CLASS
  ZArray class

DESCRIPTION
  Parameterised array class

========================================================================== */
template <class T> class ZArray : public ZArrayBase
{
protected:
 /* Incase if we try to read/write an element out of buffer area
    instead of corrupting others memory simply return an invalid element.
    so that reading & writing to buffer is not catastrophic. */
 T invalidElement;
public:


/* ======================================================================
FUNCTION:
  ZArray::ZArray

DESCRIPTION:
  Constructor

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
========================================================================== */
  ZArray () : ZArrayBase ((uint32)sizeof (T))
        {
          memset(&invalidElement,0,sizeof(invalidElement));
        }

/* ======================================================================
FUNCTION:
  ZArray::~ZArray

DESCRIPTION:
  Destructor

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
========================================================================== */
  virtual ~ZArray ()
  {
    uint32 i;
    //FILE *fp=fopen("try.txt","w");

    /*-----------------------------------------------------------------
    The following trick will make sure the destructors for all
    elements are called.
    Caution: this cannot be used if the default constructor for T
    allocates any resources!!
    -----------------------------------------------------------------*/
    for (i = 0; i < GetLength (); i++)
    {  T defT;//(fp);

      memcpy (&defT, (T*) m_pBuffer + i, sizeof (T));
    }
  }

/* ======================================================================
FUNCTION:
  ZArray::Clear

DESCRIPTION:
  Clear all

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
========================================================================== */
  void Clear  ()
  {
    uint32 i;

    /*-----------------------------------------------------------------
    The following trick will make sure the destructors for all
    elements are called.
    -----------------------------------------------------------------*/
    for (i = 0; i < GetLength (); i++)
    {  T defT;

      memcpy (&defT, (T*) m_pBuffer + i, sizeof (T));
    }
    ZArrayBase :: Clear ();
  }

/* ======================================================================
FUNCTION:
  ZArray::ZArray

DESCRIPTION:
  TYpe conversion

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  Returns a reference to m_pBuffer

SIDE EFFECTS:
  None.
========================================================================== */
  operator const T* () const {return (T*) m_pBuffer;}

/* ======================================================================
FUNCTION:
   ZArray::ElementAt

DESCRIPTION:
  get element

INPUT/OUPUT PARAMETERS:
  i - position

RETURN VALUE:
  Return the element

SIDE EFFECTS:
  None.
========================================================================== */
  T &ElementAt (int32 i) const
  {
          MM_ASSERT((i >= 0) && (i < GetLength()));
          if((i < 0) && (i >= GetLength())) return invalidElement;
    return ((T*) m_pBuffer) [i];
  }

/* ======================================================================
FUNCTION:
  ZArray::operator []

DESCRIPTION:
  get element

INPUT/OUPUT PARAMETERS:
  None.

RETURN VALUE:
  Return the element

SIDE EFFECTS:
  None.
========================================================================== */
  T &operator [] (int8 i)
  {
    MM_ASSERT((i >= 0) && (i < GetLength()));
    if((i < 0) && (i >= GetLength()))  return invalidElement;
    return ((T*) m_pBuffer) [i];
  }

  T &operator [] (uint8 i)
  {
    MM_ASSERT(((int8)i >= 0) && (i < (int32)GetLength()));
    if(i >= GetLength())  return invalidElement;
    return ((T*) m_pBuffer) [i];
  }

  T &operator [] (int32 i)
  {
    MM_ASSERT((i >= 0) && (i < (int32)GetLength()));
    if((i < 0) && (i >= (int32)GetLength()))  return invalidElement;
    return ((T*) m_pBuffer) [i];
  }

  T &operator [] (uint32 i)
  {
    MM_ASSERT(i < GetLength());
    if(i >= (uint32)GetLength())  return invalidElement;
    return ((T*) m_pBuffer) [i];
  }


  T &operator [] (int32 i) const
  {
    MM_ASSERT((i >= 0) && (i < GetLength()));
    if((i < 0) && (i >= GetLength()))  return invalidElement;
    return ((T*) m_pBuffer) [i];
  }

  /*-----------------------------------------------------------------
  get element, expand if necessary
  -----------------------------------------------------------------*/
  T &ElementAt (int32 i)
  {
            MM_ASSERT((i >= 0) && (i < (int)GetLength()));
            if((i < 0) && (i >= (int)GetLength()))   return invalidElement;
    return ((T*) m_pBuffer) [i];
  }

/* ======================================================================
FUNCTION:
  ZArray::Find

DESCRIPTION:
  Find an element and return its index

INPUT/OUPUT PARAMETERS:
  t

RETURN VALUE:
  Returns index

SIDE EFFECTS:
  None.
========================================================================== */
  int32 Find (const T &t) const
  {
    for (int32 i = GetLength (); --i >= 0;)
      if ((*this) [i] == t)
        return i;
    return -1;
  }

/* ======================================================================
FUNCTION:
  ZArray::operator +=

DESCRIPTION:
  Add an element to the end of the array

INPUT/OUPUT PARAMETERS:
  t

RETURN VALUE:
  Returns new element

SIDE EFFECTS:
  None.
========================================================================== */
  T &operator += (const T &t)
  {
    //FILE *fp=fopen("try.txt","w");
    int32 n = Increase ();
         if(n<0)
    {
       return invalidElement;
    }
    T defT = 0;//(fp);
    T &refT = defT;    // required to avoid compiler warning in release mode

    memcpy ((T*) m_pBuffer + n, &refT, sizeof (T));
    ((T*) m_pBuffer) [n] = t;
    return ((T*) m_pBuffer) [n];
  }

  // JMR - Start Update for SB 10/24/99: added for Karl's code
  /*-------------------------------------------------------
  -------------------------------------------------------*/
/* ======================================================================
FUNCTION:
  ZArray :: operator =

DESCRIPTION:
  copy assignment

INPUT/OUPUT PARAMETERS:
  rSource

RETURN VALUE:
  Reference to calling object

SIDE EFFECTS:
  None.
========================================================================== */
  ZArray<T>& operator = (const ZArray<T>& rSource)
  {
    Clear(); // clear THIS
    // copy each element from SOURCE to THIS
    for (int32 i = 0; i < rSource.GetLength(); i++)
      *this += rSource [i];
    return *this;
  }

  // JMR - End Update for SB

};

#endif                                      // _ztl_h sentry.
