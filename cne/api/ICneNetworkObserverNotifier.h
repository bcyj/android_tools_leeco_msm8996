#ifndef _CNE_NETWORK_OBSERVER_NTFR_H_
#define _CNE_NETWORK_OBSERVER_NTFR_H_
/**
 * @file ICneNetworkObserverNotifier.h
 *
 * @brief
 * Connectivity Engine Network Observer (CNO) Notifier Class Header file
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Network Observer signal
 * notifications.
 *
 * User must override signal handler methods to process signals delivered by
 * CNO by extending this class.
 *
 *                   Copyright 2010-2012  Qualcomm Technologies, Inc.
 *                          All Rights Reserved.
 *                       Qualcomm Technologies Proprietary/GTDR
 */

#include <ICneObserverDefs.h>

/**
 * @brief
 * The connectivity Engine Network Observer Notifier class. The user
 * must extend this class with implementation of each signal handler.
 */
class ICneNetworkObserverNotifier
{
  public:
  ICneNetworkObserverNotifier(){};
  virtual ~ICneNetworkObserverNotifier(){};

  /**
   * @addtogroup cno_api
   * @{
   */
  /**
  @brief
  Signal handler to process Connectivity Engine Network Observer dormancy state
  change asynchronous signal.

  This method will be called whenever the \c ICneNetworkObserver wants to notify
  regarding dormancy state change to the client, such as: <ul><li> The network
  becomes dormant.<li> The network comes out of dormancy.</ul> The client shall
  override this virtual method to handle \c SIGNAL_NET_DORMANCY_CHANGE signal
  notification.

  @param[in] dormancyStatus A reference to structure of type \c CnoDormancyStatusType

  @return
  None

  @dependencies
  \c ICneNetworkObserver::startSignal must be called for
  \c SIGNAL_NET_DORMANCY_CHANGE or \c SIGNAL_NET_ALL in order to receive this
  signal.

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal
  */
  virtual void onDormancyChange (const CnoDormancyStatusType &dormancyStatus){};

  /**
  @brief
  Signal handler to process Connectivity Engine Network Observer network
  configuration change signal

  This method will be called whenever the \c ICneNetworkObserver wants to notify
  regarding network configuration changes to the client, such as: <ul><li> The
  network type has changed. <li> The network subType has changed. <li> The
  network mtu has changed. <li> The network ip address has changed.</ul> The
  client shall override this virtual method to handle \c SIGNAL_NET_CONFIG_CHANGE
  signal notification.

  @param[in] netConfig A reference to a structure of type \c CnoNetConfigType

  @dependencies
  \c ICneNetworkObserver::startSignal must be called for
  \c SIGNAL_NET_CONFIG_CHANGE or \c SIGNAL_NET_ALL in order to receive this.

  @return
  None

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal
  */
  virtual void onNetConfigChange (const CnoNetConfigType &netConfig){};

  /**
  @brief
  Signal handler to process Connectivity Engine Network Observer latency raw
  values available signal.

  This method will be called in response to
  \c ICneNetworkObserver::getLatencyRawValues call and delivers the raw values
  for latency for all sockets connected to the specified destination.

  @param[in] measurements A reference to type \c CnoLatencyMeasurementsType

  @dependencies
  \c ICneNetworkObserver::startSignal must be called for
  \c SIGNAL_LATENCY_RAW_VALUES_AVAILABLE or \c SIGNAL_NET_ALL

  @return
  None

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserver::getLatencyRawValues
  */
  virtual void onLatencyRawValuesAvailable (const CnoLatencyMeasurementsType &measurements){};

  /**
  @brief
  Signal handler to process Connectivity Engine Network Observer latency
  estimate available signal.

  This method will be called in response to
  \c ICneNetworkObserver::getLatencyEstimate call and delivers the latency
  estimates towards a specified destination and for the active interface.

  @param[in] estimate A reference to type \c CnoLatencyEstimateType

  @dependencies
  \c ICneNetworkObserver::startSignal must be called for
  \c SIGNAL_LATENCY_ESTIMATE_AVAILABLE or \c SIGNAL_NET_ALL

  @return
  None

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserver::getLatencyEstimate
  */
  virtual void onLatencyEstimateAvailable (const CnoLatencyEstimateType &estimate) {};

  /**
  @brief
  Signal handler to process Connectivity Engine Network Observer
  latency estimate service status change signal.

  This method will be called whenever there is a change in \c ICneNetworkObserver
  latency estimate service. The argument type indicates the status of the latency
  estimate service.

  @param[in] status an indication of type \c ICnoServiceResponseType

  @dependencies
  \c ICneNetworkObserver::startSignal must be set with
  \c SIGNAL_LATENCY_SERVICE_CHANGE or
  \c SIGNAL_NET_ALL in order to receive this callback.

  @return
  None

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserver::startLatencyService
  */
  virtual void onLatencyServiceStatusChange (const CnoServiceResponseType &service){};

  /**
  @brief
  Signal handler to process Connectivity Engine Network Observer bitrate
  estimate available signal.

  This method will be called in response to
  \c ICneNetworkObserver::getBitrateEstimate call and delivers the bitrate estimates
  for the active interface.

  @param[in] estimate A reference to a struct of type \c CnoBitrateEstimateType

  @dependencies
  \c ICneNetworkObserver::startSignal must be called for
  \c SIGNAL_BITRATE_ESTIMATE_AVAILABLE or
  \c SIGNAL_NET_ALL in order to receive this callback.

  @return
  None

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserver::getBitrateEstimate
  */
  virtual void onBitrateEstimateAvailable (const CnoBitrateEstimateType &estimate){};

  /**
  Signal handler to process Connectivity Engine Network Observer bitrate service
  status change signal.

  This method will be called whenever there is a change in bitrate estimate
  service status. The argument type indicates the status of the bitrate
  estimate service.

  @param[in] status A reference to type \c CnoServiceResponseType

  @dependencies
  \c ICneNetworkObserver::startSignal must be called for
  \c SIGNAL_BITRATE_SERVICE_CHANGE or
  \c SIGNAL_NET_ALL in order to receive this callback.

  @return
  None

  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserver::startBitrateService

  */
  virtual void onBitrateServiceStatusChange (const CnoServiceResponseType &service){};

  /**
   * @}
   */
};

#endif /* _CNE_NETWORK_OBSERVER_NTFR_H_ */
