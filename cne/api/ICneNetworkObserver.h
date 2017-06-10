#ifndef _CNE_NETWORK_OBSERVER_H_
#define _CNE_NETWORK_OBSERVER_H_
/**
 * @file ICneNetworkObserver.h
 *
 * @brief Connectivity Engine Network Observer Class (Abstract)
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Network Observer
 *
 * Only the interfaces declared shall be used by the user of Connectivity
 * Engine Network Observer
 *
 *                   Copyright 2010, 2011, 2012 Qualcomm Technologies, Inc.
 *                          All Rights Reserved.
 *                       Qualcomm Technologies Proprietary/GTDR
 */

#include <ICneObserverDefs.h>

/**
 * @brief
 * Connectivity Engine Network Observer interface class (abstract).
 */
class ICneNetworkObserver
{
  public:

  /**
   * @}
   */
  /**
   * @addtogroup cno_api
   * @{
   */

  /**
  @brief
  gets active network configuration.

  This method returns to the caller the active network type, subtype, ip
  address, mtu size, and default network indication. If no network is active
  then the call will fail. ip address 0 will signify that the network is not
  connected (i.e not ready for data.)

  @param[out] netConfig reference to NetConfigRspStruct

  @dependencies
  \c ICneNetworkObserver must be created

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
            <li> \c CO_RET_PERM_DENIED - client does not have sufficient permission
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_NET_INACTIVE - no active default network
            <li> \c CO_RET_FAIL - general failure
          </ul>
  @sa
  \c CneFactory::CreateCneNetworkObserver
  */
  virtual CORetType getNetConfig (CnoNetConfigType& netConfig) = 0;

  /**
  @brief
  gets the dormancy status of the active [default] network

  Returns to the caller the active network type, subtype, dormancy indication,
  and default network indication.
  If no network is active then the call will fail.

  @param[out] dormancyStatus reference to the CnoDormancyStatusType

  @dependencies
  \c ICneNetworkObserver must be created.

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
            <li> \c CO_RET_PERM_DENIED - client does not have sufficient permission
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_NET_INACTIVE - no active default network
            <li> \c CO_RET_FAIL - general failure
          </ul>

  @sa
  \c CneFactory::CreateCneNetworkObserver
  */
  virtual CORetType getNetDormancyState (CnoDormancyStatusType& dormancyStatus) = 0;

  /**
  @brief
  Requests Connectivity Engine Network Observer to start computing latency
  estimates.

  The client will call this API if it wishes for CNE to start the latency
  estimate service on the system. The latency estimate service will continue to
  run as long as it is being actively used. If no client has used the latency
  estimation service for some time then it will shut down and the clients will
  be notified via  \c onLatencyServiceStatusChange (assuming the client is
  registered to receive this notification).

  @dependencies
  \c ICneNetworkObserver must be created,
  \c ICneNetworkObserver::startSignal must be set for
  \c SIGNAL_LATENCY_SERVICE_STATUS_CHANGE or \c SIGNAL_NET_ALL

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - latency estimate service started
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO Service error
          </ul>

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::getLatencyEstimate,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserverNotifier::onLatencyServiceStatusChange
  */
  virtual CORetType startLatencyService (void) = 0;

  /**
  @brief
  get the latency estimate to the specified destination

  Returns to the caller the active network type, subtype, latency estimate (in
  milliseconds) to a destination, and a latency estimate for the interface. It
  is recommended that the client listens to the signal
  \c SIGNAL_LATENCY_SERVICE_STATUS_CHANGE and observes the service status prior
  to calling this API. To obtain latency estimates information the latency
  estimate service must be running i.e. status is \c SERVICE_STARTED. The latency
  estimates are delivered as a part of signal \c onLatencyEstimateAvailable if
  the client has registered for it. If no estimate is available then both the
  latency estimate and timestamp for the destination is set to "0".

  @param[in] destIpAddr ip address of destination for which to
                        product an estimate

  @dependencies
  \c ICneNetworkObserver must be created.
  \c ICneNetworkObserver::startSignal must be set for
  \c SIGNAL_LATENCY_ESTIMATE_AVAILABLE or \c SIGNAL_NET_ALL

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_NET_INACTIVE - no active default network
            <li> \c CO_RET_NOT_ALLOWED - estimation service not running
          </ul>

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startLatencyService,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserverNotifier::onLatencyServiceStatusChange,
  \c ICneNetworkObserverNotifier::onLatencyEstimatesAvailable
  */
  virtual CORetType getLatencyEstimate (in_addr destIpAddr) = 0;

  /**
  @brief
  get the raw latency measurement of all sockets currently connected
  to a destination

  Returns to the caller a vector of all sockets currently connected to the
  destination address, along with their current latency values. The latency
  estimate service does not need to be running for this method to succeed. The
  latency raw values are delivered as part of signal
  \c onLatencyRawValuesAvailable

  If no socket is currently connected to the destination then the vector will
  be empty.

  @param[in] destIpAddr ip address of destination for which to get
                        latency measurements

  @dependencies
  \c ICneNetworkObserver must be created.
  \c ICneNetworkObserver::startSignal must be set for
  \c SIGNAL_LATENCY_RAW_VALUES_AVAILABLE or \c SIGNAL_NET_ALL

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
          </ul>

  @sa
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserver::stopSignal,
  \c ICneNetworkObserverNotifier::onLatencyRawValuesAvailable
  */
  virtual CORetType getLatencyRawValues (in_addr destIpAddr) = 0;


  /**
  @brief
  Requests Connectivity Engine Network Observer to start computing bitrate
  estimates periodically.

  The client will call this method if it wishes for CNE to perform bitrate
  measurements and compute estimates on the system. Bitrate will be measured
  for 5 seconds, or for a duration lesser than 60 seconds, if specified. The
  window size will be used for computing the averaging of estimates and should
  be specified in milliseconds. The client will be notified with
  \c onBitrateServiceStopped signal if it has registered for it. The signal
  conveys active network type, subtype, average, max, and instantaneous bitrate
  estimates in bps for the network interface being used by the client. If any
  of the estimates are unavailable they will be set to "0".

  @param[in] window averaging window size in milliseconds.
  @param[in] duration duration for bitrate measurements in seconds [optional].

  @dependencies
  \c ICneNetworkObserver must be created

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - bitrate measurements started
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO Service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_NOT_ALLOWED - ongoing bitrate measurement
          </ul>

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::getBitrateEstimate,
  \c ICneNetworkObserver::startSignal,
  \c ICneNetworkObserverNotifier::onBitrateServiceStatusChange
  */
  virtual CORetType startBitrateService (int window, int duration = 5) = 0;

  /**
  @brief
  gets the bitrate estimate for the default interface

  Returns to the caller the active network type, subtype, average, max, and
  instantaneous bitrate estimate in bps for the interface that will be used by
  the client. Bitrate estimates are computed for all traffic on the interface.
  Bitrate measurements must be active prior to calling this api. The client can
  call \c ICneNetworkObserver::startBitrateService to start measurements. The
  estimates are delivered as part of signal \c onBitrateEstimateAvailable if the
  client has registered for it.

  @dependencies
  \c ICneNetworkObserver must be created.
  \c ICneNetworkObserver::startBitrateService must be called.
  \c ICneNetworkObserver::startSignal must be called for either
  \c SIGNAL_BITRATE_ESTIMATE_AVAILABLE or \c SIGNAL_NET_ALL

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_NET_INACTIVE - no active default network
          </ul>

  @sa
  \c CneFactory::CreateCneNetworkObserver,
  \c ICneNetworkObserver::startBitrateService,
  \c ICneNetworkObserverNotifier::onBitrateServiceStatusChange,
  \c ICneNetworkObserverNotifier::onBitrateEstimateAvailable
  */
  virtual CORetType getBitrateEstimate (void) = 0;

 /**
  @brief
  Gets supplemental information for wlan interface

   This method is used to retrieve additional wlan related information.
   It supplements \c getNetConfig. If clients attempt to call
   this method in the absense of wlan,\c CO_RET_SERVICE_UNAVAILABLE
   will be returned.

  @param[out] netConfig reference to CnoWlanSuppNetConfigType

  @dependencies
  \c ICneNetworkObserver must be created and interface must be
     available

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_PERM_DENIED - permission denied
            <li> \c CO_RET_NET_INACTIVE - wlan network is unavailable
          </ul>
  @sa
  \c CneFactory::CreateCneNetworkObserver
  */
   virtual CORetType getWlanSuppNetConfig(CnoWlanSuppNetConfigType& netConfig) = 0;

   /**
   @brief
   Starts signal notification for the given signal identifier

   Starts aynchronous notification of signals as denoted by \c startSig. The
   client can enable all signals by setting \c startSig to \c SIGNAL_NET_ALL.

   @param[in] startSig signal identifier of the form \c CnoSignalType

   @dependencies
   A \c ICneNetworkObserverNotifier must be passed during creation of this \c
   ICneNetworkObserver for startSignal to succeed.

   @return One of the following codes
           <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_NOT_ALLOWED - operation not allowed at the moment
           </ul>

   @sa
   \c ICneNetworkObserverNotifier,
   \c CneFactory::CreateCneNetworkObserver,
   \c ICneNetworkObserver::stopSignal
   */
   virtual CORetType startSignal (CnoSignalType startSig) = 0;

   /**
   @brief
   Stops sending signals for the given signal identifier

   Stops aynchronous notification of signals as denoted by \c cnoSignalType. The
   client can disable all signals by setting \c stopSig to \c SIGNAL_NET_ALL.

   @param[in] stopSig identifier of the form CnoSignalType

   @dependencies
   A \c ICneNetworkObserverNotifier must be passed during creation of this \c
   ICneNetworkObserver for stopSignal to succeed.

   @return One of the following codes
       <ul>
         <li> \c CO_RET_SUCCESS - Success
         <li> \c CO_RET_SERVICE_UNAVAILABLE - CNO service error
         <li> \c CO_RET_INVALID_ARGS - invalid params
         <li> \c CO_RET_NOT_ALLOWED - operation not allowed at the moment
       </ul>

   @sa
   \c ICneNetworkObserverNotifier,
   \c CneFactory::CreateCneNetworkObserver,
   \c ICneNetworkObserver::startSignal
   */
   virtual CORetType stopSignal (CnoSignalType stopSig) = 0;

   /**
    * @}
    */

   /**
   @brief
   Destructor.
   */
   virtual ~ICneNetworkObserver(){};

  };

#endif /* _CNE_NETWORK_OBSERVER_H_ */
