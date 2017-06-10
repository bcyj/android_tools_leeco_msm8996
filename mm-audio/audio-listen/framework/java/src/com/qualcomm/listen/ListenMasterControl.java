/*
 *    Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 *    Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.listen;

import android.util.Log;

/**
 * ListenMasterControl is a Java API to QTI's ListenEngine
 * used for feature control.
 * <p>
 *
 * Only one single instance of the ListenMasterControl can be
 * acquired at a time. Application must use getInstance() to
 * request this singleton. Once an application gets an non-null
 * instance of this single Master Control all other calls to
 * getInstance will return null, until the first instance is
 * released. There is no public constructor.
 * <p>
 *
 * ListenMasterControl extends Listen Receiver to allow set
 * parameters via setParam() method, such as
 * LISTEN_FEATURE_ENABLE and VOICEWAKEUP_ENABLE
 * <p>
 *
 * Events received by application using this class are <br>
 *  LISTEN_FEATURE_DISABLE_EVENT <br>
 *  LISTEN_FEATURE_ENABLE_EVENT  <br>
 *  VOICEWAKEUP_FEATURE_DISABLE_EVENT <br>
 *  VOICEWAKEUP_FEATURE_ENABLE_EVENT <br>
 *  LISTEN_RUNNING_EVENT <br>
 *  LISTEN_STOPPED_EVENT <br>
 *  LISTEN_ENGINE_DIED_EVENT <br>
 */
public class ListenMasterControl extends ListenReceiver {
   /**
    * reference to itself to indicate if any instance was created
    */
   private static ListenMasterControl sMasterControl = null;
   private ListenMasterControl()  {   }
   private final static String TAG = "ListenMasterControl";

   /**
    * Gets an instance of the MasterControl singleton.
    * <p>
    * Only one client may use MasterControl at any time.
    *
    * @return instance to ListenMasterContrl or null
    *    null returned if another client has already acquired MasterControl
    *                  or if error occurs
    */
   public synchronized static ListenMasterControl getInstance()
   {
      Log.d(TAG, "getInstance");
      int status;
      if (null == sMasterControl) {
         // no other application has instantiated a copy of ListenMasterControl
         sMasterControl = new ListenMasterControl();
         if (null == sMasterControl ) {
            Log.e(TAG, "new ListenMasterControl failed");
            return null;
         } else {
            // initialize the Listen Native Receiver object
            // only one version so version number is null
            status = sMasterControl.init(ListenReceiver.MASTERCONTROL_RECEIVER_TYPE, 0);
            if (ListenTypes.STATUS_SUCCESS == status) {
               sMasterControl.bInitialized = true;
            } else {
               // MasterControl object could be not be acquired from ListenEngine
               Log.e(TAG, "ListenMasterControl could not be initialized");
               sMasterControl = null;
            }
         }
         return sMasterControl;
      } else {
         // an instance of ListenMasterControl already instantiated -
         //    so fail without even bother to check with ListenEngine
         Log.e(TAG, "MasterControl singleton already instanced");
         return null;
      }
   }

   /**
    * Releases an instance of the MasterControl singleton.
    *
    * @return
    *           STATUS_SUCCESS
    *      <br> STATUS_EFAILURE
    */
   public int releaseInstance()
   {
        int status;
        Log.d(TAG, "releaseInstance");
        status = release();  // release instance from ListenEngine
        sMasterControl = null;
        return status;
   }

    /**
     * Sets a Listen Parameter, used to enable/disable Listen and/or
     * VoiceWakeup feature.
     * <p>
     * Only the Master Control will be able to set these parameters.
     *
     * @param paramType [in] string for either type
     *        "ListenFeature" or "VoiceWakeupFeature"
     * @param value [in] string "enable" to Enable, or "disable" to
     *        Disable
     *
     * @return
     *           STATUS_EBAD_PARAM
     *      <br> STATUS_ENOT_INITIALIZED
     */
   public native int setParam(String paramType, String value);

   /**
    * Gets a Listen Parameter, used to query the status of Listen
    * and/or VoiceWakeup feature.
    *
    * @param  paramType [in] string describing parameter type
    *        "ListenFeature" or "VoiceWakeupFeature"
    *
    * @return current value for parameter: "enable" or "disable"
    *         <br> Status is not returned
    */
   public native String getParam(String paramType);

}
