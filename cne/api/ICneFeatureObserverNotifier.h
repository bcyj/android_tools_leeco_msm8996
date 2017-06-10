#ifndef _CNE_FEATURE_OBSERVER_NTFR_H_
#define _CNE_FEATURE_OBSERVER_NTFR_H_
/**
 *       Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *            Qualcomm Technologies Proprietary and Confidential.
 */

/**
 * @file ICneFeatureObserverNotifier.h
 *
 * @brief
 * Connectivity Engine Feature Observer (CFO) Notifier Class Header file
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Feature Observer signal
 * notifications.
 *
 * User must override signal handler methods to process signals delivered by
 * CFO by extending this class.
 *
 */

#include <ICneObserverDefs.h>

/**
 * @brief
 * The connectivity Engine Feature Observer Notifier class. The user
 * must extend this class with implementation of each signal handler.
 */
class ICneFeatureObserverNotifier
{
  public:
  ICneFeatureObserverNotifier(){};
  virtual ~ICneFeatureObserverNotifier(){};

  /**
   * @addtogroup cfo_api
   * @{
   */

  /**
  @brief
  Signal handler to process Connectivity Engine Feature Observer CNE
  feature status change signal

  This method will be called whenever the \c ICneFeatureObserver wants to notify clients
  regarding a specific feature's change of status, such as:
  <ul>
  <li> Feature WQE  is activated.
  <li> Feature WQE is deactivated.
  </ul> The client shall override this virtual method to handle
  \c SIGNAL_FEATURE_STATUS_CHANGE  signal notification.

  @param[in] feature Value of enum of type \c CfoFeatureNameType
  @param[in] pref Value of enum \c CfoFeatureStatusType

  @dependencies
  \c ICneFeatureObserver::startSignal must be called for
  \c SIGNAL_FEATURE_STATUS_CHANGE or \c SIGNAL_NET_ALL in order to receive this.

  @return
  None

  @sa
  \c CneFactory::CreateCneFeatureObserver,
  \c ICneFeatureObserver::startSignal,
  \c ICneFeatureObserver::stopSignal
  */
  virtual void onFeatureStatusChange (const CfoFeatureNameType feature,
  const CfoFeatureStatusType pref){};

  /**
  @brief
  Signal handler to process Connectivity Engine Feature Observer
  Interworking WLAN (IWLAN) user preference change signal

  This method will be called whenever the \c ICneFeatureObserver wants to notify
  regarding IWLAN user preference changes to the client, such as: <ul><li> IWLAN
  is enabled. <li> IWLAN is disabled. </ul> The
  client shall override this virtual method to handle \c SIGNAL_IWLAN_USER_PREF_CHANGE
  signal notification.

  @param[in] pref Value of enum type \c CfoIwlanUserPrefType

  @dependencies
  \c ICneFeatureObserver::startSignal must be called for
  \c SIGNAL_IWLAN_USER_PREF_CHANGE or \c SIGNAL_NET_ALL in order to receive this.

  @return
  None

  @sa
  \c CneFactory::CreateCneFeatureObserver,
  \c ICneFeatureObserver::startSignal,
  \c ICneFeatureObserver::stopSignal
  */
  virtual void onIwlanUserPrefChange (const CfoIwlanUserPrefType pref){};

  /**
   * @}
   */
};

#endif /* _CNE_FEATURE_OBSERVER_NTFR_H_ */
