#ifndef _IPSTREAMSOURCEUTILS_H_
#define _IPSTREAMSOURCEUTILS_H_

/* =======================================================================
                               IPStreamSourceUtils.h
DESCRIPTION
COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/IPStreamSourceUtils.h#14 $
$DateTime: 2013/04/30 16:30:49 $
$Change: 3694067 $

========================================================================== */

#include "AEEstd.h"
#include "SourceMemDebug.h"

/** ---------------------------------------------------------------------
 * Type definitions
 * ---------------------------------------------------------------------*/

typedef  unsigned char     byte;         // Unsigned 8  bit value type.
typedef  unsigned short    word;         // Unsinged 16 bit value type.
typedef  unsigned long     dword;        // Unsigned 32 bit value type.

/* ----------------------------------------------------------------------
**                          STANDARD MACROS
** ---------------------------------------------------------------------- */


/*===========================================================================

MACRO QTV_MAX
MACRO QTV_MIN

DESCRIPTION
  Evaluate the maximum/minimum of 2 specified arguments.

PARAMETERS
  x     parameter to compare to 'y'
  y     parameter to compare to 'x'

DEPENDENCIES
  'x' and 'y' are referenced multiple times, and should remain the same
  value each time they are evaluated.

RETURN VALUE
  QTV_MAX   greater of 'x' and 'y'
  QTV_MIN   lesser of 'x' and 'y'

SIDE EFFECTS
  None

===========================================================================*/
#define  QTV_MAX( x, y ) STD_MAX( x, y )
#define  QTV_MIN( x, y ) STD_MIN( x, y )

/*===========================================================================

MACRO FPOS

DESCRIPTION
  This macro computes the offset, in bytes, of a specified field
  of a specified structure or union type.

PARAMETERS
  type          type of the structure or union
  field         field in the structure or union to get the offset of

DEPENDENCIES
  None

RETURN VALUE
  The byte offset of the 'field' in the structure or union of type 'type'.

SIDE EFFECTS
  The lint error "Warning 545: Suspicious use of &" is suppressed within
  this macro.  This is due to the desire to have lint not complain when
  'field' is an array.

===========================================================================*/
#define FPOS( type, field ) \
    /*lint -e545 */ ( (dword) &(( type *) 0)-> field ) /*lint +e545 */


/*===========================================================================

MACRO FSIZ

DESCRIPTION
  This macro computes the size, in bytes, of a specified field
  of a specified structure or union type.

PARAMETERS
  type          type of the structure or union
  field         field in the structure or union to get the size of

DEPENDENCIES
  None

RETURN VALUE
  size in bytes of the 'field' in a structure or union of type 'type'

SIDE EFFECTS
  None

===========================================================================*/
#define FSIZ( type, field ) sizeof( ((type *) 0)->field )


/*===========================================================================

MACRO ARR_SIZE

DESCRIPTION
  Return the number of elements in an array.

PARAMETERS
  a             array name

DEPENDENCIES
  None

RETURN VALUE
  Number of elements in array a

SIDE EFFECTS
  None.

===========================================================================*/
#define  ARR_SIZE( a )  ( sizeof( (a) ) / sizeof( (a[0]) ) )

/**
 * class IPStreamStringTokenizer
 *  Tokenizes an input string based on a set of input delimiters
 *  provided as a string. The class functions modify the input
 *  string, so the caller may need to save a copy before
 *  providing to this class if needed.
 */
class IPStreamStringTokenizer
{
public:
  /**
   * c'tor
   *
   * @param str   Null terminated string to tokenize. Will get
   *              modified by this class.
   * @param delimiters  Null terminated string of delimiters.
   */
  IPStreamStringTokenizer(char *str, const char *delimiters);

  /**
   * d'tor
   */
  ~IPStreamStringTokenizer();

  /**
   * Returns a pointer to the next token.
   */
  char *GetNextToken();

private:
  IPStreamStringTokenizer();
  IPStreamStringTokenizer& operator=(const IPStreamStringTokenizer&);

  bool IsDelimiter(char c) const;

  char *m_pStr;
  const char *m_pDelimiters;
  size_t m_LenDelimStr;
};

/**
 * ListElement class used in IPStreamList below.
 * IPStreamList is a simple class that provides list-like
 * functionality.
 */
template <typename T>
class IPStreamListElem
{
public:
  IPStreamListElem() : m_Next(NULL)
  {

  }

  ~IPStreamListElem()
  {

  }

  IPStreamListElem(const IPStreamListElem& rElem);
  IPStreamListElem& operator=(const IPStreamListElem& rElem);

  T m_Val;
  IPStreamListElem<T> *m_Next;
};

/**
 * Basic class for list functions. Can be enhanced if needed.
 * This class does not throw exceptions (as long as QTV_New does
 * not throw).
 */
template <typename T>
class IPStreamList
{
public:
  IPStreamList() :
    m_pHead(NULL), m_pTail(NULL), m_Size(0)
  {

  }

  ~IPStreamList()
  {
    Clear();
  }

  void Clear()
  {
    while(Pop())
    {
    }

    m_pHead = NULL;
    m_pTail = NULL;
  }

  int Size() const
  {
    return m_Size;
  }

  bool Push(const T& t)
  {
    bool bOk = false;

    IPStreamListElem<T> *pElem = QTV_New(IPStreamListElem<T>);

    if(pElem)
    {
      pElem->m_Val = t;

      if(NULL == m_pHead)
      {
        // update head ptr and tail ptr.
        m_pHead = pElem;
        m_pTail = pElem;
      }
      else
      {
        // update tail ptr only.
        m_pTail->m_Next = pElem;
        m_pTail = pElem;
      }

      ++m_Size;
      bOk = true;
    }

    return bOk;
  }

  bool PushFront(const T& t)
  {
    bool bOk = false;

    IPStreamListElem<T> *pElem = QTV_New(IPStreamListElem<T>);

    if(pElem)
    {
      pElem->m_Val = t;

      if(NULL == m_pHead)
      {
        // update head ptr and tail ptr.
        m_pHead = pElem;
        m_pTail = pElem;
      }
      else
      {
        // update head ptr only.
        pElem->m_Next = m_pHead;
        m_pHead = pElem;
      }

      ++m_Size;
      bOk = true;
    }

    return bOk;
  }

  bool PeekHead(T& t)
  {
    bool bOk = false;
    if(m_pHead)
    {
      t = m_pHead->m_Val;
      bOk = true;
    }

    return bOk;
  }

  bool PeekTail(T& t)
  {
    bool bOk = false;
    if(m_pTail)
    {
      t = m_pTail->m_Val;
      bOk = true;
    }

    return bOk;
  }

  bool Pop()
  {
    bool bOk = false;
    IPStreamListElem<T> *p = m_pHead;
    if(p)
    {
      --m_Size;
      bOk = true;

      m_pHead = m_pHead->m_Next;
      if(NULL == m_pHead)
      {
        m_pTail = m_pHead;
        //QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "list is empty");
      }

      QTV_Delete(p);
    }

    return bOk;
  }

  bool IsEmpty()
  {
    return (m_Size > 0 ? false : true);
  }

  class Iterator
  {
  public:
    Iterator(IPStreamList<T>* pList, IPStreamListElem<T> *pElem) :
      m_pList(pList), m_pElem(pElem)
    {

    }

    Iterator(const Iterator& rIter)
    {
      m_pList = rIter.m_pList;
      m_pElem = rIter.m_pElem;
    }

    Iterator& operator=(const Iterator& rIter)
    {
      if(this != &rIter)
      {
        m_pList = rIter.m_pList;
        m_pElem = rIter.m_pElem;
      }
      return *this;
    }

    ~Iterator()
    {

    }

    T& operator*()
    {
      return m_pElem->m_Val;
    }

    Iterator Next()
    {
      return Iterator(m_pList, m_pElem->m_Next);
    }

    bool operator==(const Iterator& rIter)
    {
      return ((m_pList == rIter.m_pList) && (m_pElem == rIter.m_pElem));
    }

    bool operator!=(const Iterator& rIter)
    {
      return !(*this == rIter);
    }

    void Erase()
    {
      IPStreamListElem<T> *p = m_pList->m_pHead;


      if(m_pList->m_pHead == m_pElem)
      {
        // delete the first elem.
        m_pList->m_pHead = m_pList->m_pHead->m_Next;
        if(NULL == m_pList->m_pHead)
        {
          m_pList->m_pTail = NULL;
        }

        QTV_Delete(m_pElem);
      }
      else
      {
        // delete an element other than the first.
        while(p->m_Next != m_pElem)
        {
          p = p->m_Next;
        }

        p->m_Next = m_pElem->m_Next;

        if(NULL == m_pElem->m_Next)
        {
          m_pList->m_pTail = p;
        }

        QTV_Delete(m_pElem);
      }

      --m_pList->m_Size;
    }


  private:
    Iterator();

    IPStreamList<T>* m_pList;
    IPStreamListElem<T> *m_pElem;
  };

  Iterator Begin()
  {
    return Iterator(this, this->m_pHead);
  }

  Iterator End()
  {
    return Iterator(this, NULL);
  }

private:
  IPStreamList(const IPStreamList& rIPStreamList);
  IPStreamList& operator=(const IPStreamList& rIPStreamList);

  IPStreamListElem<T> *m_pHead;
  IPStreamListElem<T> *m_pTail;
  int m_Size;
};

#endif
