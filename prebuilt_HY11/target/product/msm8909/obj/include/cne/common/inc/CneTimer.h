#ifndef CNE_TIMER_H
#define CNE_TIMER_H


/*==============================================================================
  FILE:         <filename>

  OVERVIEW:     <overview>

  DEPENDENCIES: <dependencies>

                Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/


/*==============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ----------------------------------------------------------
  2011-07-27  jnb     First revision.
  2012-03-09  mtony   Fixed warning about printing 64-bit longs
==============================================================================*/


/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/

#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <queue>
#include <vector>

/*------------------------------------------------------------------------------
 * Macros
 * ---------------------------------------------------------------------------*/
#define timespeccopy(a, b)        \
do {                              \
  (a)->tv_sec  = (b)->tv_sec;    \
  (a)->tv_nsec = (b)->tv_nsec;  \
} while (0)


/*------------------------------------------------------------------------------
 * Class Definition
 * ---------------------------------------------------------------------------*/


class ICneTimerMonitor {
public:

  /*----------------------------------------------------------------------------
   * FUNCTION      notifyDelayChange
   *
   * DESCRIPTION   notify the timer monitor when there is a jump in the time
   *               until the next callback is ready
   *
   * DEPENDENCIES  CneTimer
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  virtual void notifyDelayChange() = 0;

  /*----------------------------------------------------------------------------
   * FUNCTION      Destructor
   *
   * DESCRIPTION   -
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  virtual ~ICneTimerMonitor() {}
};

/*------------------------------------------------------------------------------
 * CLASS         CneTimer
 *
 * DESCRIPTION   Handle timed events (via callbacks) for CnE
 *----------------------------------------------------------------------------*/
class CneTimer {

public:

  /*----------------------------------------------------------------------------
   * Public Types
   * -------------------------------------------------------------------------*/
  static const int TIMER_DONE =  -1;  // do not reschedule timer
  static const int TIMER_REPEAT = 0;  // reschedule timer using last delay
  static const int TIMER_ERROR = -1;  // error condition

  // signature for CneTimer callbacks
  typedef int (*TimedCallback)(void *data);

  /*------------------------------------------------------------------------------
   * FUNCTION      subTimeSpec
   *
   * DESCRIPTION   Helper function to get difference of time specs
   *
   * RETURN VALUE  bool
   *
   * ----------------------------------------------------------------------------*/
  static bool subTimeSpec(const timespec &t2, const timespec &t1, timespec &tres);

  /*------------------------------------------------------------------------------
   * FUNCTION      addTimeSpec
   *
   * DESCRIPTION   Helper function to get sum of time specs
   *
   * RETURN VALUE  bool
   *
   * ----------------------------------------------------------------------------*/
  static bool addTimeSpec(const timespec &t2, const timespec &t1, timespec &tres);

  /*------------------------------------------------------------------------------
   * FUNCTION      cmpTimeSpec
   *
   * DESCRIPTION   Helper function to compare time specs
   *
   * RETURN VALUE   -1 if t2 < t1
   *                0 if t2 == t1
   *                +1 if t2 > t1
   *
   * ----------------------------------------------------------------------------*/
  static int cmpTimeSpec(const timespec &t2, const timespec &t1);


  /*----------------------------------------------------------------------------
   * FUNCTION      Constructor
   *
   * DESCRIPTION   creates CneTimer, with an optional timer monitor
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  CneTimer(ICneTimerMonitor *setMoniter = NULL);

  /*------------------------------------------------------------------------------
   * FUNCTION      addTimedCallback
   *
   * DESCRIPTION   Add a callback function that will be called after 'delay'
   *               milliseconds have passed. If 'periodic' is set to true the
   *               callback function will be called every 'delay' milliseconds.
   *               'data' will be passed too the TimedCallback function as a
   *               parameter.
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  returns the ID of the timed callback, or TIMER_ERROR if
   *               there was an error.
   *
   * SIDE EFFECTS  'callback' will be called after 'delay' milliseconds
   *----------------------------------------------------------------------------*/
  int addTimedCallback(int64_t delay, TimedCallback callback, void *data);

   /*------------------------------------------------------------------------------
   * FUNCTION      removeTimedCallback
   *
   * DESCRIPTION   remove a timed callback based on it's ID
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  callback will NOT be called
   *----------------------------------------------------------------------------*/
  void removeTimedCallback(int id);

  /*------------------------------------------------------------------------------
   * FUNCTION      timeUntilNextEvent
   *
   * DESCRIPTION   Returns the time until the next timeout, in milliseconds
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  Earliest timeout, or -1 if there are none
   *
   * SIDE EFFECTS  None
   *----------------------------------------------------------------------------*/
  int timeUntilNextEvent() const;

  /*------------------------------------------------------------------------------
   * FUNCTION      processEvents
   *
   * DESCRIPTION   Process expired timeouts
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *----------------------------------------------------------------------------*/
  void processEvents();

  /*----------------------------------------------------------------------------
   * FUNCTION      log
   *
   * DESCRIPTION   spammy logging of CneTimer object, used for debugging
   *
   * DEPENDENCIES  logging
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  void log() const;

private:

  /*---------------------------------------
   * Timed Events class
   *--------------------------------------*/
  class TimedEvent {
  public:
    int id;                 // timer ID
    timespec trigger;        // time to tigger callback
    TimedCallback callback; // callback
    void *data;             // data to be passed to callback
    int64_t delay;          // how long of a delay was requested?

    std::string toString() const {
      const static int SIZE = 256;
      char rv[SIZE];
      snprintf(rv, SIZE, "TimedEvent id:%d callback:%p data:%p delay:%ldms time: %ld.%06ld (Epoch)",
          id, callback, data, (long) delay, (long)(trigger.tv_sec), (long)(trigger.tv_nsec));
      return std::string(rv);
    }

    bool operator==(TimedEvent const& b) const {
      if (trigger.tv_sec == b.trigger.tv_sec
          && trigger.tv_nsec == b.trigger.tv_nsec
          && callback == b.callback
          && data == b.data
          && delay == b.delay) {
        return true;
      } else {
        return false;
      }
    }
  };

  // helper struct for comparing TimedElements
  struct TimedEventCmp {
    bool operator() (const TimedEvent &lhs, const TimedEvent &rhs) const {
      // true if lhs > rhs
      return(cmpTimeSpec((lhs.trigger), (rhs.trigger)) == 1);
    }
  };

  // enable debugging logs
  const static int DEBUG = true;

  // main queue for timer events
  std::priority_queue<TimedEvent, std::vector<TimedEvent>, TimedEventCmp> timerQueue;

  // pointer to CneTimer monitor, if any
  ICneTimerMonitor *monitor;

  // ID of next timer event
  int nextId;

  // return the next available timer event ID
  int getNextTimerId();

  // helper method to add a delay in ms to a timerval
  void addDelayToTimeval(int64_t delay, timespec &tv);

};

#endif /* CNE_TIMER_H */
