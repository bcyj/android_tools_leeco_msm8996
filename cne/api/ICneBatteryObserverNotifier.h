#ifndef _CNE_BATTERY_OBSERVER_NTFR_H_
#define _CNE_BATTERY_OBSERVER_NTFR_H_
/**
 * @file ICneBatteryObserverNotifier.h
 *
 * @brief Connectivity Engine Battery Observer (CBO) Notifier
 * Class Header file.
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Battery Observer signal
 * notifications
 *
 * User must override signal handler methods to process signals delivered by
 * CBO by extending this class.
 *
 *                   Copyright 2010, 2011 Qualcomm Technologies, Inc.
 *                          All Rights Reserved.
 *                       Qualcomm Technologies Proprietary/GTDR
 */

#include <ICneObserverDefs.h>

/**
 * @brief
 * Connectivity Engine Battery Observer Notifier class. The user must
 * extend this class with implementation of each signal handler
 */
class ICneBatteryObserverNotifier
{
  public:

  ICneBatteryObserverNotifier() {};
  virtual ~ICneBatteryObserverNotifier() {};

  /**
   * @addtogroup cbo_api
   * @{
   */
  /**
  @brief
  Call back function to process CBO battery status change

  This method will be called whenever the connectivity engine battery observer
  wants to notify regarding battery charging state change, for e.g. this method
  will be called when: <ul><li> When a charger is connected.<li> When a charger
  is disconnected.<li> When remaining charge on the battery falls to 20%.</ul>
  The client shall override this virtual method to handle the signal.

  @param[in] batteryStatus A reference to a structure of type
                           \c CboBatteryStatusType

  @dependencies
  None

  @return
  None

  @sa
  \c ICneBatteryObserver::CreateCneBatteryObserver
  \c ICneBatteryObserver::startSignal,
  \c ICneBatteryObserver::stopSignal
  */
  virtual void onBatteryStateChange(const CboBatteryStatusType &batteryStatus){};
  /**
   * @}
   */
};

#endif /* _CNE_BATTERY_OBSERVER_NTFR_H_ */
