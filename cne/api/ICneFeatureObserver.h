#ifndef _CNE_FEATURE_OBSERVER_H_
#define _CNE_FEATURE_OBSERVER_H_
/**
 *       Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *            Qualcomm Technologies Proprietary and Confidential.
 */

/**
 * @file ICneFeatureObserver.h
 *
 * @brief Connectivity Engine Feature Observer Class (Abstract)
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Feature Observer
 *
 * Only the interfaces declared shall be used by the user of Connectivity
 * Engine Feature Observer
 *
 */

#include <ICneObserverDefs.h>

/**
 * @brief
 * Connectivity Engine Feature Observer interface class (abstract).
 * This class provides state information related to activation status
 * or user preference of specific Connectivity Engine (CnE) features.
 */
class ICneFeatureObserver
{
  public:

  /**
   * @}
   */
  /**
   * @addtogroup cfo_api
   * @{
   */

  /**
   @brief
   Starts signal notification for the given signal identifier

   Starts aynchronous notification of signals as denoted by \c startSig. The
   client can enable all signals by setting \c startSig to \c SIGNAL_NET_ALL.

   @param[in] startSig signal identifier of the form \c CfoSignalType

   @dependencies
   A \c ICneFeatureObserverNotifier must be passed during creation of this \c
   ICneFeatureObserver for startSignal to succeed.

   @return One of the following codes
        <ul>
         <li> \c CO_RET_SUCCESS - Success
         <li> \c CO_RET_SERVICE_UNAVAILABLE - CFO service error
         <li> \c CO_RET_INVALID_ARGS - invalid params
         <li> \c CO_RET_NOT_ALLOWED - operation not allowed at the moment
       </ul>

   @sa
   \c ICneFeatureObserverNotifier,
   \c CneFactory::CreateCneFeatureObserver,
   \c ICneFeatureObserver::stopSignal
   */
   virtual CORetType startSignal (CfoSignalType startSig) = 0;

   /**
   @brief
   Stops sending signals for the given signal identifier

   Stops aynchronous notification of signals as denoted by \c CfoSignalType. The
   client can disable all signals by setting \c StopSig to \c SIGNAL_NET_ALL.

   @param[in] stopSig identifier of the form CfoSignalType

   @dependencies
   A \c ICneFeatureObserverNotifier must be passed during creation of this \c
   ICneFeatureObserver for stopSignal to succeed.

   @return One of the following codes
       <ul>
         <li> \c CO_RET_SUCCESS - Success
         <li> \c CO_RET_SERVICE_UNAVAILABLE - CFO service error
         <li> \c CO_RET_INVALID_ARGS - invalid params
         <li> \c CO_RET_NOT_ALLOWED - operation not allowed at the moment
       </ul>

   @sa
   \c ICneFeatureObserverNotifier,
   \c CneFactory::CreateCneFeatureObserver,
   \c ICneFeatureObserver::startSignal
   */
   virtual CORetType stopSignal (CfoSignalType stopSig) = 0;


   /**
   @brief
   Gets status of specific feature

   Clients use this api to determine current activation status of specified CNE feature.

   @param[in] feature value of enum CfoFeatureNameType
   @param[out] status reference to enum CfoFeatureStatusType

   @dependencies
   \c ICneFeatureObserver must be created

   @return One of the following codes
       <ul>
         <li> \c CO_RET_SUCCESS - Success
         <li> \c CO_RET_SERVICE_UNAVAILABLE - CFO service error
         <li> \c CO_RET_INVALID_ARGS - invalid params
         <li> \c CO_RET_PERM_DENIED - permission denied
       </ul>

   @sa
   \c ICneFeatureObserverNotifier,
   \c CneFactory::CreateCneFeatureObserver,
   \c ICneFeatureObserver::startSignal
   */
   virtual CORetType getFeatureStatus(CfoFeatureNameType feature, CfoFeatureStatusType& status) = 0;

   /**
  @brief
  Gets current Interworking WLAN (IWLAN) user preference

  End users can enable or disable the IWLAN feature via a user interface.
  Clients use this api to determine current user preference of this feature.

  @param[out] pref reference to enum CfoIwlanUserPrefType

  @dependencies
  \c ICneFeatureObserver must be created

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CFO service error
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_PERM_DENIED - permission denied
          </ul>
  @sa
  \c CneFactory::CreateCneFeatureObserver
  */
  virtual CORetType getIwlanUserPref(CfoIwlanUserPrefType& pref) = 0;

  /**
   @brief
   Destructor.
   */
   virtual ~ICneFeatureObserver(){};

};

#endif /* _CNE_FEATURE_OBSERVER_H_ */
