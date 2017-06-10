#ifndef __STREAMDATAQUEUE_H__
#define __STREAMDATAQUEUE_H__
/************************************************************************* */
/**
 * StreamDataQueue.h
 * @brief Defines the StreamDataQ interface.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/StreamDataQueue.h#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "StreamQueue.h"
#include "SourceMemDebug.h"
#include "qtv_msg.h"
#include <MMCriticalSection.h>

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

namespace video {

/* =======================================================================

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
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
template <class T>
class StreamDataQ
{
public:
  StreamDataQ()
    : m_nSize(0),
      m_pQElem(NULL),
      m_pDataLock(NULL)
  {
    //Initialization
    MM_CriticalSection_Create(&m_pDataLock);
    StreamQ_init(&m_cDataFreeQ);
    StreamQ_init(&m_cDataInUseQ);
  }

  ~StreamDataQ()
  {
    if (m_pQElem)
    {
      QTV_Delete_Array(m_pQElem);
      m_pQElem = NULL;
    }
    if (m_pDataLock)
    {
      MM_CriticalSection_Release(m_pDataLock);
      m_pDataLock = NULL;
    }
  }

  bool Init(const uint32 nSize = DEFAULT_SIZE)
  {
    bool bResult = true;

    //If not already initialized, create an array of Q elements
    //ToDo: Might enhance to re-size the Q dynamically
    if (m_pQElem == NULL)
    {
      m_nSize = nSize;
      m_pQElem = QTV_New_Array(StreamDataQElement, m_nSize);
      if (m_pQElem)
      {
        //Set up data free Q
        for (uint32 i = 0; i < m_nSize; i++)
        {
          StreamDataQElement* pQElem = m_pQElem + i;
          (void)StreamQ_link(pQElem, &pQElem->sLink);
          StreamQ_put(&m_cDataFreeQ, &pQElem->sLink);
        }
      }
      else
      {
        bResult = false;
      }
    }

    return bResult;
  }

public:
  uint32 Size() const
  {
    return m_nSize;
  }

  uint32 Count() const
  {
    uint32 cnt = 0;

    MM_CriticalSection_Enter(m_pDataLock);
    cnt = StreamQ_cnt((StreamQ_type*)&m_cDataInUseQ);
    MM_CriticalSection_Leave(m_pDataLock);

    return cnt;
  }

  bool Full() const
  {
    bool bFull = false;

    MM_CriticalSection_Enter(m_pDataLock);
    bFull = (StreamQ_cnt((StreamQ_type*)&m_cDataFreeQ) == 0);
    MM_CriticalSection_Leave(m_pDataLock);

    return bFull;
  }

  bool Empty() const
  {
    return (Count() == 0);
  }

  bool EnQ(const T& item) const
  {
    bool bResult = false;

    if (!Full())
    {
      //EnQ takes an element from free Q and puts it in use Q
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_get((StreamQ_type*)&m_cDataFreeQ);
      if (pQElem)
      {
        pQElem->tData = item;
        StreamQ_put((StreamQ_type*)&m_cDataInUseQ, &pQElem->sLink);
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool DeQ(T& item) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //DeQ takes an element from head of use Q and puts it in free Q
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_get((StreamQ_type*)&m_cDataInUseQ);
      if (pQElem)
      {
        item = pQElem->tData;
        StreamQ_put((StreamQ_type*)&m_cDataFreeQ, &pQElem->sLink);
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool Pop(T& item) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //Pop takes an element from tail of use Q and puts it in free Q
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_last_get((StreamQ_type*)&m_cDataInUseQ);
      if (pQElem)
      {
        item = pQElem->tData;
        StreamQ_put((StreamQ_type*)&m_cDataFreeQ, &pQElem->sLink);
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool PeekHead(T& item) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //PeekHead takes a peek at the head of use Q
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_check((StreamQ_type*)&m_cDataInUseQ);
      if (pQElem)
      {
        item = pQElem->tData;
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool PeekHead(T** item) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //PeekHead takes a peek at the head of use Q
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_check((StreamQ_type*)&m_cDataInUseQ);
      if (pQElem && item)
      {
        *item = &pQElem->tData;
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool PeekTail(T& item) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //PeekTail takes a peek at the tail of use Q
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_last_check((StreamQ_type*)&m_cDataInUseQ);
      if (pQElem)
      {
        item = pQElem->tData;
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool Iterator(void*& pItData) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //Iterator returns the head to use Q. Caller to use this to iterate
      //through the Q in a loop
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_check((StreamQ_type*)&m_cDataInUseQ);
      if (pQElem)
      {
        pItData = (void*)&pQElem->sLink;
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool Next(void*& pItData, T& item) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //Next returns (peeks) the next element in use Q to the element
      //pointed by pItData. pItData is also an out param that is prepared
      //for the next Next call
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem = (pItData == NULL) ?
        (StreamDataQElement *)StreamQ_check((StreamQ_type*)&m_cDataInUseQ) :
        (StreamDataQElement *)StreamQ_next((StreamQ_type*)&m_cDataInUseQ,
                                           (StreamQ_link_type*)pItData);
      if (pQElem)
      {
        pItData = (void*)&pQElem->sLink;
        item = pQElem->tData;
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }


  bool Next(void*& pItData, T** item) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //Next returns (peeks) the next element in use Q to the element
      //pointed by pItData. pItData is also an out param that is prepared
      //for the next Next call
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem = (pItData == NULL) ?
        (StreamDataQElement *)StreamQ_check((StreamQ_type*)&m_cDataInUseQ) :
        (StreamDataQElement *)StreamQ_next((StreamQ_type*)&m_cDataInUseQ,
                                           (StreamQ_link_type*)pItData);
      if (pQElem && item)
      {
        *item = &pQElem->tData;
        pItData = (void*)&pQElem->sLink;
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool Remove(void*& pItData) const
  {
    bool bResult = false;

    if (!Empty())
    {
      //Remove removes the element pointed by pItData. pItData is also an
      //out param that is updated accordingly for the next Next call
      MM_CriticalSection_Enter(m_pDataLock);
      void* pTemp = NULL;
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_prev((StreamQ_type*)&m_cDataInUseQ,
                                           (StreamQ_link_type*)pItData);
      if (pQElem)
      {
        pTemp = (void*)&pQElem->sLink;
      }
      bResult = (StreamQ_delete_ext((StreamQ_link_type*)pItData) != 0);
      if (bResult)
      {
        StreamQ_put((StreamQ_type*)&m_cDataFreeQ, (StreamQ_link_type*)pItData);
        pItData = pTemp;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool Peek(const uint32 index, T& item) const
  {
    bool bResult = false;

    if (!Empty() && index < Count())
    {
      //Peek takes a peek at the use Q element at the specified index
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_check((StreamQ_type*)&m_cDataInUseQ);

      //Get to the right element first
      for (uint32 i = 1; i <= index && pQElem; i++)
      {
        pQElem = (StreamDataQElement *)StreamQ_next((StreamQ_type*)&m_cDataInUseQ,
                                                    &pQElem->sLink);
      }

      if (pQElem)
      {
        item = pQElem->tData;
        bResult = true;
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool Remove(const uint32 index, T& item) const
  {
    bool bResult = false;

    if (!Empty() && index < Count())
    {
      //Remove removes the use Q element at the specified index
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem =
        (StreamDataQElement *)StreamQ_check((StreamQ_type*)&m_cDataInUseQ);

      //Get to the right element first
      for (uint32 i = 1; i <= index && pQElem; i++)
      {
        pQElem = (StreamDataQElement *)StreamQ_next((StreamQ_type*)&m_cDataInUseQ,
                                                    &pQElem->sLink);
      }

      if (pQElem)
      {
        item = pQElem->tData;
        bResult = (StreamQ_delete_ext(&pQElem->sLink) != 0);
        if (bResult)
        {
          StreamQ_put((StreamQ_type*)&m_cDataFreeQ, &pQElem->sLink);
        }
      }
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

  bool Reset() const
  {
    bool bResult = true;

    if (!Empty())
    {
      //Reset dequeues all use Q elements and puts them back in free Q. Note
      //that it's caller's responsibility to free up any memory associated
      //with Q data (T) if needed (e.g. if T is a pointer data to a class
      //caller might not want to use Reset, instead DeQ all elements manually
      //and invoke the destructor for the class after each such DeQ call)
      MM_CriticalSection_Enter(m_pDataLock);
      StreamDataQElement* pQElem = NULL;
      do
      {
        pQElem = (StreamDataQElement *)StreamQ_get((StreamQ_type*)&m_cDataInUseQ);
        if (pQElem)
        {
          StreamQ_put((StreamQ_type*)&m_cDataFreeQ, &pQElem->sLink);
        }
      }while(pQElem);
      MM_CriticalSection_Leave(m_pDataLock);
    }

    return bResult;
  }

private:
  static const int DEFAULT_SIZE = 5;
  struct StreamDataQElement
  {
    StreamQ_link_type sLink;
    T tData;
  };

  uint32 m_nSize;
  StreamDataQElement* m_pQElem;
  StreamQ_type m_cDataFreeQ;
  StreamQ_type m_cDataInUseQ;

  MM_HANDLE m_pDataLock;
};

} // namespace video

#endif //__STREAMDATAQUEUE_H__
