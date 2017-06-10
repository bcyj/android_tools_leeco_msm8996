#ifndef EventDispatcher_H
#define EventDispatcher_H

/*=============================================================================
       Copyright (c) 2009-2010,2012 Qualcomm Technologies, Inc.  All Rights Reserved.
       Qualcomm Technologies Proprietary and Confidential.
 =============================================================================*/

#include <map>

using namespace std;

template<class Event> class EventDispatcher
{

  typedef void (*event_cb)(Event event, const void *event_data, void *cbdata);

public:
  bool regEventCallback(Event event, event_cb cb, void *cbdata);
  bool deregEventCallback(Event event, event_cb cb);

protected:
  void dispatchEvent(Event event, const void *event_data);

private:
  typedef enum cbstatus_e
  {
    CB_NOT_IN_PROGRESS,
    CB_IN_PROGRESS_DONT_DEREG,
    CB_IN_PROGRESS_DEREG,
  } cbstatus_t;

  struct cbentry
  {
    event_cb cb;
    void *cbdata;
    cbstatus_t cbstatus;
  };

  typedef multimap<Event, cbentry*> map_type;

  map_type event_map;
};

template<class Event>
bool EventDispatcher<Event>::regEventCallback(Event event, event_cb cb, void *cbdata)
{
  cbentry *e = new cbentry;
  e->cb = cb;
  e->cbdata = cbdata;
  e->cbstatus = CB_NOT_IN_PROGRESS;

  event_map.insert(make_pair(event, e));
  return true;
}

template<class Event>
bool EventDispatcher<Event>::deregEventCallback(Event event, event_cb cb)
{
  typedef typename map_type::iterator iter;
  pair<iter, iter> cbs = event_map.equal_range(event);

  iter i = cbs.first;
  while ( i != cbs.second )
  {
    if ( i->second->cb == cb )
    {
      switch ( i->second->cbstatus )
      {
        case CB_NOT_IN_PROGRESS:
        {
          delete i->second;
          event_map.erase ( i++ );
          break;
        }
        case CB_IN_PROGRESS_DONT_DEREG:
        {
          i->second->cbstatus = CB_IN_PROGRESS_DEREG;
          ++i;
          break;
        }
        default:
          ++i;
          break;
      }
    }
    else
    {
      ++i;
    }
  }

  return true;
}

template<class Event>
void EventDispatcher<Event>::dispatchEvent(Event event, const void *event_data)
{
  typedef typename map_type::iterator iter;
  pair<iter, iter> cbs = event_map.equal_range(event);

  size_t size = event_map.size();
  int evtsProcessed = 0;
  for (iter i = cbs.first; i != cbs.second;)
  {
    ++evtsProcessed;
    if( i->second->cbstatus == CB_NOT_IN_PROGRESS )
    {
      i->second->cbstatus = CB_IN_PROGRESS_DONT_DEREG;
    }
    i->second->cb(event, event_data, i->second->cbdata);
    if ( i->second->cbstatus == CB_IN_PROGRESS_DEREG )
    {
      delete i->second;
      event_map.erase ( i++ );
      --evtsProcessed;
    }
    else
    {
      i->second->cbstatus = CB_NOT_IN_PROGRESS;
      ++i;
    }
    // If some entries are removed from the map, then the positions
    // of the remaining entries will change thus changing their
    // addresses. Thus we need to update the pointer to point to
    // the updated address.
    if(size != event_map.size())
    {
      size = event_map.size();
      cbs = event_map.equal_range(event);
      i = cbs.first;
      for(int j = 0; j != evtsProcessed; j++, i++)
      {
        if (i == cbs.second)
        {
          break;
        }
      }
    }
  }
}

#endif
