#ifndef _CNE_BATTERY_OBSERVER_H_
#define _CNE_BATTERY_OBSERVER_H_
/**
 * @file ICneBatteryObserver.h
 *
 * @brief
 * Connectivity Engine Battery Observer (CBO) Class (Abstract)
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Battery Observer
 *
 * Only the interfaces declared shall be used by the user of Connectivity
 * Engine Battery Observer
 *
 *                   Copyright 2010, 2011 Qualcomm Technologies, Inc.
 *                          All Rights Reserved.
 *                       Qualcomm Technologies Proprietary/GTDR
 */

#include <ICneObserverDefs.h>

class ICneBatteryObserverNotifier;
/**
 * @brief
 * Connectivity Engine Battery Observer interface class (abstract)
 */
class ICneBatteryObserver
{
  public:
  /**
  @addtogroup cbo_api
  @{
  */
  /**
  @brief
  gets battery charge status

  Returns to the caller the battery charging indication, charger type, and
  remaining charge. The caller must pass a reference to the
  \c CboBatteryStatusType which will be populated by this method.

  @param[out] batteryStatus Reference to \c CboBatteryStatusType

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - \c CBO service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
          </ul>

  @dependencies
  Connectivity Engine Battery Observer must be created before calling this
  method.


  @sa
  \c ICneBatteryObserver::stopSignal,  \c ICneBatteryObserver::startSignal
  */
  virtual CORetType getBatteryStatus (CboBatteryStatusType& batteryStatus) = 0;

  /**
  @brief
  Starts sending signals for the given signal identifier

  Starts aynchronous notification of signals as denoted by \c CboSignalType. A
  signal handler of the form \c ICneBatteryObserverNotifier must be registered
  with \c ICneBatteryObserver during its creation, to able to start signal(s)
  or else the call will fail.

  @param[in] startSig signal identifier of the form \c CboSignalType

  @dependencies
  A signal handler must be passed during creation of this battery observer

  @return One of the following codes
          <ul>
            <li> CO_RET_SUCCESS - Success
            <li> CO_RET_SERVICE_UNAVAILABLE - CBO service error
            <li> CO_RET_INVALID_ARGS - invalid params
            <li> CO_RET_NOT_ALLOWED - operation not allowed at the moment
          </ul>

  @sa
  \c ICneBatteryObserverNotifier, \c ICneBatteryObserver::stopSignal
  */
  virtual CORetType startSignal (CboSignalType startSig) = 0;

  /**
  @brief
  Stops sending signals for the given signal identifier

  Stops aynchronous notification of signals as denoted by \c CboSignalType. A
  signal handler of the form \c ICneBatteryObserverNotifier must be registered
  with \c ICneBatteryObserver at the time of its creation or the call will
  fail.

  @param[in] stopSig identifier of the form \c CboSignalType

  @dependencies
  A signal handler of the form \c ICneBatteryObserverNotifier must be
  registered

  @return One of the following codes
          <ul>
            <li> CO_RET_SUCCESS - Success
            <li> CO_RET_SERVICE_UNAVAILABLE - CBO service error
            <li> CO_RET_INVALID_ARGS - invalid params
            <li> CO_RET_NOT_ALLOWED - operation not allowed at the moment
          </ul>

  @sa
  \c ICneBatteryObserverNotifier, \c ICneBatteryObserver::startSignal
  */
  virtual CORetType stopSignal (CboSignalType stopSig) = 0;

  /**
   * @}
   */

  /**
  @brief
  Destructor.
  */
   virtual ~ICneBatteryObserver(){};
};

#endif /* _CNE_BATTERY_OBSERVER_H_ */
