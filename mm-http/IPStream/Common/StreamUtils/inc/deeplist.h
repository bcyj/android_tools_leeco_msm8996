#ifndef DEEPLIST_H
#define DEEPLIST_H

/* =======================================================================
                               deeplist.h
DESCRIPTION
   This file provides definitions for a list class used by QTV.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/deeplist.h#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "SourceMemDebug.h"

// The internal implementation of DeepList. Also serves as a cheap iterator
// class. This class does no memory management.
template<class T>
class ListPair
{
public:
   // Default constructor used by DeepListIterator. Uses default constructor for T.
   ListPair() : _head(), _hat(NULL), _tail(NULL) { }

   // Create a singleton
   ListPair(const T &head) : _head(head), _hat(NULL), _tail(NULL) { }

   // Make a list with the argument head element and tail list.
   ListPair(const T &head, ListPair *tail) : _head(head), _hat(NULL), _tail(tail) {
      if (_tail != NULL)
      {
         _tail->_hat = this;
      }
   }

   ~ListPair() { }

   T& head() const { return (T&) _head; }

   ListPair<T> *hat() const { return _hat; }
   ListPair<T> *tail() const { return _tail; }

   void setTail(ListPair<T> *tail) { _tail = tail; }

   void setHat(ListPair<T> *hat) { _hat = hat; }

protected:
   T _head;

   // Elements before the head, funny name, no?
   ListPair<T> *_hat;

   // Elements after the head
   ListPair<T> *_tail;
};

// Forward declaration to make friend class of DeepList.
template<class T>
class DeepListIterator;

// A wrapper over ListPair which implements a nice linked list that cleans itself
// up automatically.
template<class T>
class DeepList
{
public:

   // Create an empty list.
   DeepList()
   {
      _pair = NULL;
      _lastPair = NULL;
      _size = 0;
   }

   // Create a singleton.
   DeepList(const T &element)
   {
      _pair = QTV_New_Args(ListPair<T>, (element));
      _lastPair = _pair;
      _size = 1;
   }

   DeepList(const DeepList<T>& that)
   {
      _pair = NULL;
      _lastPair = NULL;
      _size = 0;

      for (const ListPair<T> *list = that.iterator(); list != NULL;
           list = list->tail()) {
         append(list->head());
      }
   }

   // Do a deep delete of the pair list.
   virtual ~DeepList()
   {
      clear();
   }

   // Make a deep copy.
   virtual DeepList<T> &operator = (const DeepList<T> &that) {
      // Protect against a = a
      if (this == &that) {
         return *this;
      }

      clear();

      for (const ListPair<T> *list = that.iterator(); list != NULL;
           list = list->tail()) {
         append(list->head());
      }

      return *this;
   }

   // Append argument list to end, don't check for duplicates.
   virtual DeepList<T> &operator += (const DeepList<T> &that)
   {
      for (const ListPair<T> *it = that.iterator(); it != NULL; it = it->tail())
      {
         append(it->head());
      }
      return *this;
   }

#if 0
   virtual int32 equals(const DeepList<T>& b) const
   {
      if (size() != b.size()) {
         return 0;
      }

      // Keep track of what elements have already been used for comparison.
      bool *used = NULL;
      QTV_Malloc( used, bool, size() );

      for (int i = 0; i < size(); i++) {
         used[i] = false;
      }

      for (const ListPair<T> *ait = iterator(); ait != NULL;
         ait = ait->tail())
      {
         const T &element = ait->head();

         // Look for an equivalent element that hasn't been used yet.
         int i = 0;
         bool found = false;
         for (const ListPair<T> *bit = b.iterator(); bit != NULL;
            bit = bit->tail())
         {
            if (!used[i] && (element == bit->head()))
            {
               found = true;
               used[i] = true;
            }

            i++;
         }

         if (!found)
         {
            QTV_Free( used );
            return 0;
         }
      }

      QTV_Free( used );
      return 1;
   }
#endif

   virtual void prepend(const T& element)
   {
      _pair = QTV_New_Args(ListPair<T>, (element, _pair));

      // If this is the only element, set the last pair.
      if (_size == 0) {
         _lastPair = _pair;
      }

      _size++;
   }

   // Add the element to the end of the list.
   virtual void append(const T& element)
   {
      if (_size == 0) {
         // Make a singleton list
         _pair = QTV_New_Args(ListPair<T>, (element));
         _lastPair = _pair;
         _size = 1;
      } else {
         // Allocate the last list element, and add to the end. Update the last pair.
         ListPair<T> *lastPair;
         lastPair = QTV_New_Args(ListPair<T>,(element));
         _lastPair->setTail(lastPair);
         lastPair->setHat(_lastPair);
         _lastPair = lastPair;
         _size++;
      }
   }

   virtual int size() const { return _size; }

   // O(index) implementation, returns true if successful and sets element.
   // Otherwise, returns false (in case of invalid index).
   virtual bool get(int index, T &element) const
   {
      if (index >= _size) {
         return false;
      }

      ListPair<T> *current = _pair;
      for (int i = 0; i < index; i++) {
         current = current->tail();
      }

      element = current->head();
      return true;
   }

   //returns instance if successful and sets element.
   // Otherwise, returns NULL (in case of invalid index).
   virtual T *get(int index) const
   {
      T *element = NULL;

      if (index >= _size) {
         return NULL;
      }

      ListPair<T> *current = _pair;
      for (int i = 0; i < index; i++) {
         current = current->tail();
      }

      element = &(current->head());
      return element;
   }

   virtual bool getFirst(T &element) const
   {
     if (_size <= 0)
     {
           return false;
       }

       element = _pair->head();

       return true;
   }

   virtual bool getLast(T &element) const
   {
     if (_size <= 0)
       {
           return false;
       }

       element = _lastPair->head();

       return true;
   }

   virtual bool removeFirst()
   {
      if (_size <= 0)
      {
          return false;
      }

      ListPair<T> *newPair = _pair->tail();

      QTV_Delete ( _pair );

      _pair = newPair;

      _pair->setHat(NULL);

      _size--;

      return true;
   }

   virtual bool removeLast()
   {
      if (_size <= 0)
      {
          return false;
      }

      ListPair<T> *newLastPair = _lastPair->hat();

      QTV_Delete ( _lastPair );

      _lastPair = newLastPair;

      _lastPair->setTail(NULL);

      _size--;

      return true;
   }


   // Return an "iterator" which is just the internal implementation of the
   // the list. The elements can be scanned by repeatedly calling head() and
   // updating the iterator pointer to tail(). Once the iterator pointer is NULL,
   // there are no more elements.
   virtual ListPair<T> *iterator() const { return _pair; }

   // Empties the list. The internal data structures are freed.
   virtual void clear()
   {
      while (_pair != NULL) {
         ListPair<T> *tail = _pair->tail();
         QTV_Delete( _pair );
         _pair = tail;
      }
      // _pair is now NULL
      _lastPair = NULL;
      _size = 0;
   }

   friend class DeepListIterator<T>;

protected:
   // The head of the list
   ListPair<T> *_pair;

   // The last element of the list, for appending.
   ListPair<T> *_lastPair;
   int _size;
};

// A more flexible, but less efficient iterator.
template<class T>
class DeepListIterator
{
public:
   // Create an iterator starting from the first element of the
   // argument list.
   DeepListIterator(DeepList<T> &list)
     : _list(list), _currentElement(list.iterator())
   {
   }

   ~DeepListIterator()
   {
   }

   bool retreat()
   {
      if (_currentElement == NULL)
      {
         QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "retreat() called during illegal state");
         return false;
      }

      _currentElement = _currentElement->hat();
      return true;
   }

   bool advance()
   {
      if (_currentElement == NULL)
      {
         QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "advance() called during illegal state");
         return false;
      }
      _currentElement = _currentElement->tail();
      return true;
   }

   // Return true if element() can be called to retrieve the current element.
   // This might be false if we have reached the beginning or end of the list,
   // or if remove() has just been called.
   bool hasMore() const
   {
      // Have we just removed? In that case, this shouldn't be called.
      if (_currentElement == &_dummyElement)
      {
         QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "hasMore() called during illegal state");
         return false;
      }
      else
      {
         return (_currentElement != NULL);
      }
   }

   // Return the current element. This may not be called immediately after remove();
   // either advance() or retreat() must be called first.
   T &element() const
   {
      // If we have no more elements, or we just removed an element, fail.
      if (!hasMore())
      {
          QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "element() called during illegal state");
      }
      return _currentElement->head();
   }

   // Remove the current element. After this call, advance() or retreat() must be
   // called before element(), hasMore(), or remove() are called again.
   bool remove()
   {
      // Fail if we just did a remove, or there are no more elements.
      if ((_currentElement == &_dummyElement) || (_currentElement == NULL))
      {
         QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "remove() called during illegal state");
         return false;
      }

      ListPair<T> *hat = _currentElement->hat();
      ListPair<T> *tail = _currentElement->tail();

      if (NULL != hat)
      {
         hat->setTail(tail);
      }

      if (NULL != tail)
      {
         tail->setHat(hat);
      }

      // Adjust backing list
      if (_list._pair == _currentElement)
      {
         _list._pair = _list._pair->tail();
      }

      if (_list._lastPair == _currentElement)
      {
         _list._lastPair = _list._lastPair->hat();
      }

      _list._size--;

      QTV_Delete( _currentElement );

      // To make implementation easier, use the dummy as the current.
      _dummyElement.setHat(hat);
      _dummyElement.setTail(tail);

      _currentElement = &_dummyElement;

      return true;
   }

protected:
   // The list that backs the iterator.
   DeepList<T> &_list;

   ListPair<T> *_currentElement;

   // After remove() is called, _currentElement points to this element which
   // contains the pointers to the previous and next elements.
   // Note: this requires that T have a default constructor.
   ListPair<T> _dummyElement;

private:
   DeepListIterator<T>& operator=(const DeepListIterator<T> &);
};

// An iterator over a const DeepList.
template<class T>
class ConstDeepListIterator
{
public:
   // Create an iterator starting from the first element of the
   // argument list.
   ConstDeepListIterator(const DeepList<T> &list)
     : _currentElement(list.iterator())
   {
   }

   ~ConstDeepListIterator()
   {
   }

   bool retreat()
   {
      if (_currentElement == NULL)
      {
         QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "retreat() called during illegal state");
         return false;
      }

      _currentElement = _currentElement->hat();
      return true;
   }

   bool advance()
   {
      if (_currentElement == NULL)
      {
         QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "advance() called during illegal state");
         return false;
      }
      _currentElement = _currentElement->tail();
      return true;
   }

   // Return true if element() can be called to retrieve the current element.
   // This might be false if we have reached the beginning or end of the list.
   bool hasMore() const
   {
     return (_currentElement != NULL);
   }

   // Return the current element. This may not be called immediately after remove();
   // either advance() or retreat() must be called first.
   T &element() const
   {
      // If we have no more elements, or we just removed an element, fail.
      if (!hasMore())
      {
          QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR, "element() called during illegal state");
      }
      return _currentElement->head();
   }

protected:

   ListPair<T> *_currentElement;
};

#endif /* DEEPLIST_H */
