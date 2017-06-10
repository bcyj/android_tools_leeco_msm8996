/*
 *    Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 *    Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.listen;

import com.qualcomm.listen.ListenTypes.EventData;
import android.util.Log;

/**
 * ListenReceiver is a Java API to QTI's ListenEngine for
 * setting a callback to receive notification.
 * <p>
 * It is a base class for ListenVoiceWakeupSession and
 * ListenMasterControl Classes.
 * <p>
 * It saves a reference to the applications' implementation of
 * IListenEventProcessor and forwards events from ListenEngine to it.
 */
public class ListenReceiver
{
    protected boolean               bInitialized;
    protected IListenEventProcessor listener;
    private   int                   type;

    // hidden C++ pointer to native client object
    //     this ptr is set when Listen Native Receiver object new'ed
    private int                   nativeClient;

    private final static String TAG = "Java ListenReceiver";

    // load the Listen JNI library
    static {
        Log.d(TAG, "Load liblistenjni");
        System.loadLibrary("listenjni");
    }

    protected static final int UNDEFINED_RECEIVER_TYPE = 0;
    /** Master Control receiver type */
    protected static final int MASTERCONTROL_RECEIVER_TYPE = 1;
    /** VoiceWakeup receiver type for SVA */
    protected static final int VWUSESSION_RECEIVER_TYPE = 2;

    protected ListenReceiver()
    {
        listener = null;
        bInitialized = false;
        nativeClient = 0;
        type = UNDEFINED_RECEIVER_TYPE;
    }

    /**
     * Initializes ListenReceiver object.
     *
     * @param receiverType [in] - int type of ListenReceiver
     *        either MASTERCONTROL_RECEIVER_TYPE or
     *        VWUSESSION_RECEIVER_TYPE
     * @param algoType [in] - algorithm to be used for detection
     * @return
     *        <br> STATUS_SUCCESS
     *        <br> STATUS_EFAILURE - Listen service disabled or JNI error
     *        <br> STATUS_EBAD_PARAM
     *        <br> STATUS_ENO_MEMORY
     *        <br> errors from Listen Engine
     */
    protected native int init(int receiverType, int appType);

    /**
      * Releases ListenReceiver object.
      *
      * @param none
      * @return errors
      *        <br> STATUS_SUCCESS
      *        <br> errors from Listen Engine
      */
     protected native int release();

     /**
       * Get the strings of the keyword phrase and (optional) the user name
       * with the highest detected score returned in the EventData payload.
       * <p>
       * Places these string in into EventData data structure.
	   *
       * @param  eventType [in] - enumerated event type
       * @param  eventData [in] - EventData data structure
       * @return
       *        <br> STATUS_SUCCESS
       *        <br> STATUS_EFAILURE - unable to execute
       *        <br> STATUS_ENO_MEMORY
       *        <br> errors from Listen Engine
	   */
      private native int getDetectionStrings( int eventType,
                               EventData eventData );

    // ----------------------------------------------------------------------------
    //  Public Methods
    // ----------------------------------------------------------------------------
    /**
     * Sets the callback that implements IListenEventProcessor which will receive
     * events.
     * <p>
     * Stores a reference to the Java application class that implements the
     * IListenEventProcessor processEvent() method that will be used for
     * event callback.
     * Usually this would be same application class that creates a MasterControl
     * or Session object.
     * <p>
     * Setting this callback is mandatory.
     *
     * @param listener [in] Java App object that implements IListenEventProcessor
     * @return
     *        <br> STATUS_SUCCESS
     *        <br> STATUS_ERESOURCE_NOT_AVAILABLE - receiver object not initialized
     */
    public int setCallback(IListenEventProcessor listener)
    {
         Log.d(TAG, "Call setCallback");
         if (!bInitialized) {
            Log.e(TAG, "Init was not successfully done");
            return ListenTypes.STATUS_ERESOURCE_NOT_AVAILABLE;
         }
         this.listener = listener;
         return ListenTypes.STATUS_SUCCESS;
    }

     // ----------------------------------------------------------------------------
     //    Callback Mechanism
     // ----------------------------------------------------------------------------
     /**
      * Receives events from Listen Engine and forwards it to
      * IListenEvent.processEvent().
      * <p>
      * This is an internal function that is called only by
      *     Listen Service.
      *
      * @param  eventType [in] enumerated event type
      * @param  eventData [in] EventData data structure
      *
      * @return - none.  Status of event is part of EventData.
      */
     public void receiveEvent( int eventType,
                               EventData eventData )
     {
         int status;
         Log.d(TAG, "receiveEvent event type = " + eventType +
                    ", eventData type = " + eventData.type);
         eventData.keyword = null;  // init as default
         eventData.user = null;

         if ((ListenTypes.EVENT_DATA_TYPE_V2 == eventData.type) &&
               (ListenTypes.DETECT_SUCCESS_EVENT == eventType ||
                ListenTypes.DETECT_FAILED_EVENT  == eventType) ) {
            // EVENT_DATA_TYPE_V2 expanded to include strings for keyword detected
            //   and optional user verified; fill these before calling processEvent()
            status = getDetectionStrings(eventType, eventData);
            if (status != ListenTypes.STATUS_SUCCESS) {
               Log.d(TAG, "getDetectionStrings failed with status = " + status);
            }
            Log.d(TAG, "getDetectionStrings returned " + eventData.keyword +
                              " and " + eventData.user);
         }
         Log.d(TAG, "receiveEvent calls processEvent");
         listener.processEvent(eventType, eventData);
         return;
     }

}
