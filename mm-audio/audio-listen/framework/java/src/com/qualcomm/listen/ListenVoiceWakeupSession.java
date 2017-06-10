/*
 *    Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 *    Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.listen;

import com.qualcomm.listen.ListenTypes.SoundModelParams;
import com.qualcomm.listen.ListenTypes.RegisterParams;
import com.qualcomm.listen.ListenTypes.VWUUserKeywordPairConfLevel;
import com.qualcomm.listen.ListenTypes.VWUKeywordConfLevel;
import com.qualcomm.listen.ListenTypes.ReadResults;

import android.util.Log;
import java.nio.ShortBuffer;

/**
 * ListenVoiceWakeupSession is a Java API to QTI's
 * ListenEngine used for keyword detection. <p>
 *
 * ListenVoiceWakeupSession extends Listen Receiver to allow a
 * SoundModel to be registered.
 * Each ListenVoiceWakeupSession is associated with a SoundModel. <p>
 *
 * Global parameters can be queried but can not be changed via
 * this Session object. <p>
 *
 * Events received by application using this class are:<br>
 *  LISTEN_FEATURE_DISABLE_EVENT <br>
 *  LISTEN_FEATURE_ENABLE_EVENT <br>
 *  VOICEWAKEUP_FEATURE_DISABLE_EVENT <br>
 *  VOICEWAKEUP_FEATURE_ENABLE_EVENT <br>
 *  DETECT_SUCCESS_EVENT <br>
 *  DETECT_FAILED_EVENT <br>
 *  DEREGISTERED_EVENT <br>
 *  LISTEN_RUNNING_EVENT <br>
 *  LISTEN_STOPPED_EVENT <br>
 *  LISTEN_ENGINE_DIED_EVENT <br>
 */

public class ListenVoiceWakeupSession extends ListenReceiver {

   /** Application type passed to non-public methods could be "unknown" */
   private static final int UNKNOWN_APP_TYPE  = 0;

   private final static String TAG = "ListenVoiceWakeupSession";

   private ListenVoiceWakeupSession()
   {
       // no member variables to initialize
   }

   /**
    * Creates an instance of ListenVoiceWakeupSession - obsolete
    * <p>
    * API supports backwards compatibility with SVA 1.0 only.
    * Only SVA version 1.0 SoundModels can be registered for this
    * version of VoiceWakeupSession.
    * <p>
    * With SVA 1.0 only one session can be created at a time.
    * @return an instance of ListenVoiceWakeupSession or null if a
    *         session is already created or error occurs.
    * @deprecated use {@link createInstance(int)} instead.
    */
@Deprecated
   public synchronized static ListenVoiceWakeupSession createInstance()
   {
       ListenVoiceWakeupSession session = null;
       Log.d(TAG, "createInstance using SVA app type");
       session = createInstance(ListenTypes.SVA_APP_TYPE);
       return session;
   }

   /**
    * Creates an instance of ListenVoiceWakeupSession for a particular
    * application (algorithm) type.
    * <p>
    * releaseInstance() should be called to free Session when
    * application is finished using it.
    *
    * @param appType [in] defines the detection algorithm used.
    *        Currently supported appType is ListenTypes.SVA_APP_TYPE
    *
    * @return an instance of ListenVoiceWakeupSession, or
    *         null if maximum number of sessions have already been created
    *         or error occurs
    */
   public synchronized static ListenVoiceWakeupSession createInstance(int appType)
   {
       int status;
       int iAppType;
       ListenVoiceWakeupSession session = null;
       // assuming since session is return value that is assigned to variable
       //    in calling method, that it will not be garbage collected
       session = new ListenVoiceWakeupSession();
       if (null == session) {
           Log.e(TAG, "new ListenVoiceWakeupSession failed");
           return null;
       }
       // initialize the Listen Native Receiver object
       if (appType == ListenTypes.SVA_APP_TYPE)  {
           Log.d(TAG, "createInstance() session.init for SVA");
           iAppType = appType;
       } else {
           Log.e(TAG, "createInstance() appType " + appType + " unknown");
           iAppType = UNKNOWN_APP_TYPE;
       }
       status = session.init(ListenReceiver.VWUSESSION_RECEIVER_TYPE, iAppType);

       if (ListenTypes.STATUS_SUCCESS == status) {
           session.bInitialized = true;
       } else {
           Log.e(TAG, "session init returned " + status);
           session.bInitialized = false;
       }
       if (!session.bInitialized) {
           // Session object could be not be acquired from ListenEngine
           Log.e(TAG, "Session could not be initialized");
           session = null;
       }
       return session;
   }

   /**
    * Releases an instance of ListenVoiceWakeupSession.
    *
    * @return
    *          STATUS_SUCCESS
    *     <br> STATUS_EFAILURE
    */
   public int releaseInstance()
   {
       int status;
       status = release(); // release instance from ListenEngine
       return status;
   }

   /**
    * Gets a Listen Parameter, used to query the status of Listen
    * and/or VoiceWakeup feature.
    *
    * @param paramType [in] string describing parameter type
    *        Currently supported paramTypes are
    * 		 "ListenFeature" or "VoiceWakeupFeature"
    *
    * @return current value of parameter: "enable" or "disable"
    *         <br> Status is not returned
    */
   public native String getParam(String paramType);

    /**
     * Registers a SVA 1.0 SoundModel that the application wants
     *     the ListenEngine to use for detection.
     *<p>
     * This form of the registerSoundModel method is obsolete.
     * It should only be used is SoundModel is SVA 1.0 format
     * registerSoundModel(RegisterParams registrationParams) method should be used.
     *<p>
     * Only one Sound Model may be registered per session.
     * If a model was previous register for this session, it must be explicitly de-registered.
     *<p>
     * SoundModelParams is converted to RegisterSoundModel form before registeration.
     *
     * @param  soundModelParams [in] data structure containing required parameters
     *
     * @return
     *              STATUS_SUCCESS
     *         <br> STATUS_EFAILURE
     *         <br> STATUS_EFEATURE_NOT_ENABLED
     *         <br> STATUS_ESOUNDMODEL_ALREADY_REGISTERED
     *         <br> STATUS_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION
    * @deprecated use {@link RegisterParams(registrationParams)} instead.
    */
@Deprecated
    public  int registerSoundModel(SoundModelParams soundModelParams)
    {
        int status = ListenTypes.STATUS_SUCCESS;
        // convert SVA 1.0 SoundModel parameters to SVA 2.0 structure
        RegisterParams regParams = new RegisterParams();
        regParams.soundModelData = soundModelParams.soundModelData;
        regParams.detectionMode = soundModelParams.detectionMode;
        regParams.bFailureNotification = soundModelParams.bFailureNotification;
        // SVA 1.0 SoundModels contain one Keyword
        regParams.numKeywords = 1;
        VWUKeywordConfLevel[] keywordConfLevels = new VWUKeywordConfLevel[1];
        keywordConfLevels[0] = new VWUKeywordConfLevel();
        keywordConfLevels[0].keyword = null; // Keyword phrase is not known
        keywordConfLevels[0].confLevel = soundModelParams.minKeywordConfidence;
        regParams.keywordMinConfLevels = keywordConfLevels;
        // SVA 1.0 SoundModels have optional data for a single User
        if (soundModelParams.minUserConfidence > 0) {
            regParams.numKeywordUserPairs = 1;
            VWUUserKeywordPairConfLevel[] userKWPairConfLevels = new
                            VWUUserKeywordPairConfLevel[1];
            userKWPairConfLevels[0] = new VWUUserKeywordPairConfLevel();
            userKWPairConfLevels[0].keyword = null; // Keyword phrase is not known
            userKWPairConfLevels[0].user = null; // User Name string is not known
            userKWPairConfLevels[0].confLevel = soundModelParams.minUserConfidence;
            regParams.userKWPairMinConfLevels = userKWPairConfLevels;
        } else {
            regParams.numKeywordUserPairs = 0;
            regParams.userKWPairMinConfLevels = null;
        }
        regParams.bufferParams = null;  // Look Ahead buffering was not supported in SVA 1.0

        status = registerSoundModel(regParams);
        return status;
    }

    /**
     * Registers SVA SoundModel using 2.0 form of parameters.
     *<p>
     * Only one Sound Model may be registered per session.
     * If a model was previous register for this session, it must be explicitly de-registered.
     *
     * @param  registrationParams [in] data structure containing required parameters
     *
     * @return
     *              STATUS_SUCCESS
     *         <br> STATUS_EFAILURE
     *         <br> STATUS_EFEATURE_NOT_ENABLED
     *         <br> STATUS_ESOUNDMODEL_ALREADY_REGISTERED
     */
     public native int registerSoundModel(RegisterParams registrationParams);

    /**
     * Deregisters a SoundModel.
     *
     * @return
     *             STATUS_SUCCESS
     *        <br> STATUS_EFAILURE
     *        <br> STATUS_ESOUNDMODEL_NOT_REGISTERED
     */
    public native int deregisterSoundModel() ;

    /**
     * Retrieves the speech buffer collected after a keyword is detected.
     *<p>
     * If buffering is enabled during SoundModel registration, then audio after
     * the end of the keyword phrase is written to buffer immediate after detection
     * and continues until the application stops buffering by
     * calling {@link stopBuffering()}. Detection will only resume once the
     * application explicitly stops buffering.
     *<p>
     * This method can be called after receiving DETECTION_SUCCESS event.
     *<p>
     * This method is a synchronous blocking call.  It will not return until
     * either the number samples requested are available to return, or
     * buffering is stopped. It is recommended that the number samples
     * requested be as small as practical.
     *<p>
     * Buffer contains 16KHz mono 16-bit PCM audio samples.  These are
     * returned by filling the provided ShortBuffer. Requested read size
     * should be 16,000 times number of seconds of audio desired.
     *<p>
     * Buffer will be held until next detection or until session destroyed.
     *
     * @param destBuffer [in] audio captured after keyword is written to this buffer
     * @param readSize [in] number of requested shorts.
     *
     * @return ReadResult object which includes:
     *      status enum
     *         <br> STATUS_SUCCESS
     *         <br> STATUS_ESOUNDMODEL_NOT_REGISTERED
     *         <br> STATUS_EBUFFERING_NOT_ENABLED
     *         <br> STATUS_ENOT_BUFFERING
     *         <br> STATUS_EBUFFER_OVERFLOW_OCCURRED
     *         <br> STATUS_EBAD_PARAM
     *      number of samples (shorts) written
     */
     public ReadResults readBuffer(ShortBuffer destBuffer, int readSize)
     {
         Log.d(TAG, "readBuffer(): enter");
         ReadResults readResults = new ReadResults();
         if (destBuffer.capacity() < readSize) {
             readResults.status = ListenTypes.STATUS_EBAD_PARAM;
             readResults.writeSize = 0;
             Log.e(TAG, "readBuffer(): readSize is larger than the capacity " +
             		"of the destination buffer provided");
             return readResults;
         }
         int status = ListenTypes.STATUS_EFAILURE;
         status = getBuffer(destBuffer, readSize, readResults);
         Log.d(TAG, "readBuffer(): getBuffer() returned with status = " + status);
         if (status != ListenTypes.STATUS_SUCCESS) {
            readResults.status = status;
            Log.d(TAG, "readBuffer(): status " + readResults.status +
                      " stored into ReadResult.status");
         }
         return readResults;
     };

     /**
      * Retrieves the speech buffer collected after a keyword is detected.
      * Helper function to readBuffer where readBuffer supplies the object to pass.
      *
      * @param destBuffer [in] audio captured after keyword is written to this buffer
      * @param readSize [in] number of requested shorts
      * ShortBuffer
      * @return
      *        <br> STATUS_SUCCESS
      *        <br> STATUS_EBAD_PARAM
     */
     private native int getBuffer(ShortBuffer destBuffer, int readSize, ReadResults readResults);

    /**
     * Stops the Listen Engine buffering
     *<p>
     * Detection for this session is immediately resumed.
     *
     * @return status - STATUS_SUCCESS or STATUS_EFAILURE
     *         <br> STATUS_EBUFFERING_NOT_ENABLED
     *         <br> STATUS_ENOT_BUFFERING
     */
     public native int stopBuffering();

}

