/*
 *	Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 *	Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.listen;

import com.qualcomm.listen.ListenTypes.EventData;
import com.qualcomm.listen.ListenTypes.DetectionData;
import com.qualcomm.listen.ListenTypes.VoiceWakeupDetectionData;
import com.qualcomm.listen.ListenTypes.VoiceWakeupDetectionDataV2;

import com.qualcomm.listen.ListenTypes.ConfidenceData;
import com.qualcomm.listen.ListenTypes.SoundModelInfo;
import com.qualcomm.listen.ListenTypes.SVASoundModelInfo;
import com.qualcomm.listen.ListenTypes.KeywordUserCounts;

import com.qualcomm.listen.ListenTypes.VWUUserKeywordPairConfLevel;


import java.nio.ByteBuffer;
import java.nio.ShortBuffer;
import android.util.Log;

/**
 * ListenSoundModel is a class of helper API to QTI's
 * ListenEngine to be used with SoundModel.
 * <p>
 * No events are generated due to calls to these SoundModel
 * methods so no callback is need for this class.
 */
public class ListenSoundModel {
    private final static String TAG = "ListenSoundModel";

    // Set within SoundModelInfo
    /** SoundModel type not known */
    private static final int UNKNOWN_SOUNDMODEL_TYPE = 0;
    /** SoundModel is Snapdragon Voice Activation format */
    private static final int SVA_SOUNDMODEL_TYPE = 1;

    /** Version number: 2 MSBytes major number, 2 LSBytes minor number
     *  For non-SVA SoundModels version will be unknown */
    private static final int VERSION_UNKNOWN    = 0;
    /** Version 1.0 */
    private static final int VERSION_0100       = 0x0100;
    /** Version 2.0 */
    private static final int VERSION_0200       = 0x0200;


// -----------------------------------------------------------------------
// Public Methods
// -----------------------------------------------------------------------


    // load the Listen JNI library
    static {
        Log.d(TAG, "Load liblistenjni");
        System.loadLibrary("listenjni");
    }

    /**
     * Constructor
     */
     public ListenSoundModel() { }

    /**
     * Verifies that User Recording contains the spoken keyword
     * (as defined in the given User-Independent SVA 1.0 model)
     * and is of good enough quality.
     * <p>
     * A returned confidence level greater than 70 indicates that the
     * phrase in the recording sufficiently matches the keyword
     * defined in given model.
     * Such a recording can be can be used for extending a SoundModel.
     *
     * @param userIndependentModel [in] contains User-Independent
     *        model data
     * @param userRecording [in] a single recording of user speaking
     *        keyword
     *
     * @return percent (0-100) confidence level.
     *        <br> Negative error number is returned if error occurred.
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_EFAILURE
     *
     * @deprecated use {@link verifyUserRecording(ByteBuffer, String, ShortBuffer)}
     * instead.
     */
@Deprecated
     public static int verifyUserRecording(
            ByteBuffer       userIndependentModel,
            ShortBuffer      userRecording)
    {
         // calls SVA 2.0 version of verifyUserRecording() with null Keyword string.
         // This results in UserData being compared to the first Keyword of the SoundModel.
         int status = verifyUserRecording(userIndependentModel, null, userRecording);
         return status;
     }

    /**
     * Verifies that a User Recording contain a specific keyword phrase in the given
     * SVA 2.0 (and above) SoundModel, and that it is of good-enough quality to be
     * used to when calling {@link extend(ByteBuffer, String, String, int,
     *     ShortBuffer[], ByteBuffer, ConfidenceData)}.
     * <p>
     * A returned confidence level greater than 70 indicates that the
     * phrase in the recording sufficiently matches the keyword
     * defined in given model.
     *
     * @param soundModel [in] contains SoundModel containing predefined keywords
     * @param keywordPhrase [in] name of keyword in SM to be extended
     * @param userRecording [in] a single recording of user speaking keyword
     *
     * @return percent (0-100) confidence level.
     *        <br> Negative error number is returned if error occurred.
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_EKEYWORD_NOT_IN_SOUNDMODEL
     *        <br>   STATUS_ENO_SPEACH_IN_RECORDING
     *        <br>   STATUS_ETOO_MUCH_NOISE_IN_RECORDING
     */
     public static native int verifyUserRecording(
            ByteBuffer       soundModel,
            String           keywordPhrase,
            ShortBuffer      userRecording);


    /**
     * Get the total size of bytes required to hold a SoundModel that
     * containing both User-Independent and User-Dependent SVA 1.0 model
     * data.
     * <p>
     * The size returned by this method must be used to create a
     * ByteBuffer that can hold the SoundModel created by extend().
     *
     * @param userIndependentModel [in] contains User-Independent
     *        model data.
     *
     * @return total (unsigned) size of extended SoundModel
     *        <br> Negative error number is returned if error occurred.
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_EFAILURE
     *
     * @deprecated use {@link getSizeWhenExtended(ByteBuffer, String, String)}
     * instead.
     *
     * <p>
     * Refer to {@link extend(ByteBuffer, String, String, int, ShortBuffer[],
     *  ByteBuffer, ConfidenceData)}
     */
@Deprecated
     public static int getSizeWhenExtended(
            ByteBuffer userIndependentModel)
     {
		 // calls SVA 2.0 version of getSizeWhenExtended() with null string for
		 // Keywords Phrase and User Name.
         int status = getSizeWhenExtended(userIndependentModel, null, null);
         return status;
     }

    /**
     * Get the total size of bytes required to hold a SoundModel that
     * containing both User-Independent and User-Dependent SVA 2.0 (and above)
     * model data.
     * <p>
     * The size returned by this method must be used to create a
     * ByteBuffer that can hold the SoundModel created by
     * {@link extend(ByteBuffer, String, String, int, ShortBuffer[], ByteBuffer, ConfidenceData)}.
     * <p>
	 * SoundModels with User-Defined keyword phrases cannot be extended
	 * and thus cannot be passed to this function.
	 *
     * @param soundModel [in] contains model data.
     * @param keywordPhrase [in] name of keyword in SM to be extended
     * @param userName [in] name of user created these training recordings
     *
     * @return total (unsigned) size of extended SoundModel
     *        <br> Negative error number is returned if error occurred.
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_EKEYWORD_NOT_IN_SOUNDMODEL
     *        <br>   STATUS_EMAX_USERS_EXCEEDED
     * <p>
     * Refer to {@link extend(ByteBuffer, String, String, int, ShortBuffer[],
     *  ByteBuffer, ConfidenceData)}
     */
     public static native int getSizeWhenExtended(
            ByteBuffer soundModel,
            String     keywordPhrase,
            String     userName    );

    /**
     * Extends a SoundModel by combining UserIndependentModel with
     * SVA 1.0 UserDependentModel data created from user recordings.
     * <p>
     * Application is responsible for creating a ByteBuffer large
     * enough to hold the SoundModel output by this method.
     * The size of the output SoundModel can be determined calling
     * {@link getSizeWhenExtended(ByteBuffer)}.
     * <p>
     * At least 5 user recordings should be passed to this method.
     * The more user recordings passed as input, the greater the
     * likelihood of getting a higher quality SoundModel made.
     * <p>
     * Confidence level greater than 70 indicates that user's
     * speech characteristics are sufficiently consistent to
     * afford the detection algorithm the ability to do User
     * detection.
     *
     * @param userIndependentModel [in] contains
     *        UserIndependentModel data
     * @param numUserRecordings [in] number of recordings of a
     *        user speaking the keyword
     * @param userRecordings [in] array of N user recordings
     * @param extendedSoundModel [out] extended SoundModel
     * @param confidenceData [out] contains ConfidenceData
     * @return
     *        <br>   STATUS_SUCCESS
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_EFAILURE
     *
     * @deprecated use {@link extend(ByteBuffer, String, String, int,
     *     ShortBuffer[], ByteBuffer, ConfidenceData)} instead.
     *
     * <p>
     * Refer to {@link getSizeWhenExtended(ByteBuffer)}
     */
@Deprecated
     public static int extend(
             ByteBuffer        userIndependentModel,
             int               numUserRecordings,
             ShortBuffer       userRecordings[],
             ByteBuffer        extendedSoundModel,
             ConfidenceData    confidenceData )
     {
         // calls SVA 2.0 version of extend() with null strings for Keywords Phrase
         // and User Name.
         // This results in UserData being added to the first Keyword of the SoundModel.
         int status = extend( userIndependentModel, null, null,
                              numUserRecordings, userRecordings,
                              extendedSoundModel, confidenceData );
         return status;
     }

    /**
     * Extends a SVA 2.0 (and above) SoundModel by combining User-Independent
     * model data with User-Dependent model data created from user recordings.
     * <p>
     * Application is responsible for creating a ByteBuffer large
     * enough to hold the SoundModel output by this method.
     * The size of the output SoundModel can be determined calling
     * {@link getSizeWhenExtended(ByteBuffer, String, String)}.
     * <p>
     * At least 5 user recordings should be passed to this method.
     * The more user recordings passed as input, the greater the
     * likelihood of getting a higher quality SoundModel made.
     * <p>
     * Confidence level greater than 70 indicates that user's
     * speech characteristics are sufficiently consistent to
     * afford the detection algorithm the ability to do User
     * detection.
	 * <p>
	 * SoundModels with User-Defined keyword phrases cannot be extended
	 * and thus cannot be passed to this function.
     *
     * @param soundModel [in] contains the SoundModel to be extended.
     * @param [in]  keywordPhrase - name of keyword in SM to be extended
     *        Null String can be passed if SM only contains one Keyword.
     * @param [in]  userName - name of user created these training recordings
     * @param numUserRecordings [in] number of recordings of a user speaking the keyword
     * @param userRecordings [in] array of N user recordings
     * @param extendedSoundModel [out] extended SoundModel
     * @param confidenceData [out] contains ConfidenceData
     * @return
     *        <br>   STATUS_SUCCESS
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_EKEYWORD_NOT_IN_SOUNDMODEL
     *        <br>   STATUS_EMAX_USERS_EXCEEDED
     *
     * <p>
     * Refer to {@link getSizeWhenExtended(ByteBuffer, String, String)}
     */
     public static native int extend(
             ByteBuffer        soundModel,
             String            keywordPhrase,
             String            userName,
             int               numUserRecordings,
             ShortBuffer       userRecordings[],
             ByteBuffer        extendedSoundModel,
             ConfidenceData    confidenceData );

    /**
     * Verifies that a user recording containing a keyphrase is of good
     * enough quality used for creating a User-Defined SoundModel with
     * {@link createUdkSm(String, String, int, ShortBuffer, ByteBuffer,
     * ByteBuffer, ConfidenceData)}. Checks the recording for problems
     * with Signal to Noise Ratio, signal level, and speech length.
     * <p>
     * A Language Model matching the language the keyphrase is spoken
     * in is used during verification process.
     *
     * @param languageModel [in] buffer containing language specific data
     * @param userRecording [in] the user training recording to be verified
     *
     * @return If positive the return value is the measured Signal-to-Noise ratio of
     *        <br>    the recording.  A value of 12 or higher is acceptable
     *        <br> Negative error number is returned if error occurred.
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_ENO_SPEACH_IN_RECORDING
     *        <br>   STATUS_ETOO_MUCH_NOISE_IN_RECORDING
     */
    public static native int verifyUdkRecording(
            ByteBuffer      languageModel,
            ShortBuffer     userRecording);

    /**
     * Gets the total size in bytes required to hold a User-Defined keyword
     * SoundModel.
     * <p>
     * The size returned by this method can be used to create a ByteBuffer
     * that can hold the user-defined keyphrase SoundModel created by
     * {@link createUdkSm(String, String, int, ShortBuffer, ByteBuffer,
     * ByteBuffer, ConfidenceData)}
     * <p>
     * A Language Model matching the language the keyphrase is spoken
     * in must be supplied as input.
     *
     * @param keywordPhrase [in] name of keyword SM is to be created for
     * @param userName [in] name of user created these training recordings
     * @param userRecordings[] [in] the user training recordings
     * @param languageModel [in] buffer containing language specific data
     *
     * @return total (unsigned) size of UDK SoundModel
     *        <br> Negative error number is returned if error occurred.
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_EFAILURE
     *
     * <p>
     * Refer to {@link createUdkSm(String, String, int, ShortBuffer,
     *  ByteBuffer, ByteBuffer, ConfidenceData)}
     */
    public static native int getUdkSmSize(
            String          keyPhrase,
            String          username,
            ShortBuffer     userRecordings[],
            ByteBuffer      languageModel);

    /**
     * Creates a User-Defined keyword SoundModel.
     * <p>
     * App is responsible for creating a ByteBuffer large
     * enough to hold the SoundModel output by this method.
     * The size of the output SoundModel can be determined calling
     * {@link getUdkSmSize(String, String, ShortBuffer, ByteBuffer)}.
     * <p>
     * At least 5 user recordings should be passed to this method.
     * The more user recordings passed as input, the greater the
     * likelihood of getting a higher quality SoundModel.
     * <p>
     * A Language Model matching the language the keyphrase is spoken
     * in is used during creation process.
     * <p>
     * A confidence level greater than 70 indicates that user's
     * speech characteristics are sufficiently consistent to
     * afford the detection algorithm the ability to do user
     * detection.
     *
     * @param keywordPhrase [in] name of keyword in SM to be extended
     * @param userName [in] name of user created these training recordings
     * @param numUserRecordings [in] number of recordings
     * @param userRecordings[] [in] the user recordings
     * @param languageModel [in] the model containing language specific data
     * @param userDefinedSoundModel [out] the created SoundModel
     * @param confidenceData [out] contains ConfidenceData
     * @return
     *         STATUS_SUCCESS
     *    <br> STATUS_EBAD_PARAM
     *    <br> STATUS_USER_DATA_OVERWRITTEN
     * <p>
     * Refer to {@link getUdkSmSize(String, String, ShortBuffer, ByteBuffer)}
     */
    public static native int createUdkSm(
            String          keyPhrase,
            String          username,
            int             numUserRecordings,
            ShortBuffer     userRecordings[],
            ByteBuffer       languageModel,
            ByteBuffer      userDefinedSoundModel,
            ConfidenceData  confidenceData );

     /**
      * Parses SVA detection events data (received by
      * IListenerEventProcessor.processEvent()) and fills
      * fields of a child class of DetectionData.
      * <p>
      * The data structure created and returned is determined by
      * the DetectionData.type field.
      * <p>
      * If VWU_EVENT_0100 is output as the type,
      * then a VoiceWakeupDetectionData object will be create
      * and returned. For this example, the application would cast
      * this DetectionData to a VoiceWakeupDetectionData object.
      * <p>
      * If VWU_EVENT_0200 is output as the type,
      * then a VoiceWakeupDetectionDataV2 object will be create
      * and returned. For this example, the application would cast
      * this DetectionData to a VoiceWakeupDetectionData object.
      *
      * @param registeredSoundModel [in] SoundModel that was used
      *        for registration/detection
      * @param eventPayload [in] event payload returned by
      *        ListenEngine
      *
      * @return DetectionData child object created by this method
      *         <br> null returned for EventData if
      *             input SoundModel is not an SVA SM or error occurs
      */
     public static  DetectionData parseDetectionEventData(
              ByteBuffer       registeredSoundModel,
              EventData        eventPayload)
     {
         int              status = ListenTypes.STATUS_SUCCESS;
         DetectionData    detData = null;  // generic parent class for all detection data objs
         SoundModelInfo   soundModelInfo = new SoundModelInfo();
          if ( null == soundModelInfo)  {
             Log.e(TAG, "parseDetectionEventData() new SoundModelInfo failed");
             return null;
         }
         status = getTypeVersion( registeredSoundModel, soundModelInfo);
         if ( ListenTypes.STATUS_SUCCESS != status ) {
             Log.e(TAG, "parseDetectionEventData() get SM Info failed w/ " + status);
             return null;
         }
         if (soundModelInfo.type != SVA_SOUNDMODEL_TYPE) { // check that SM is SVA type
             Log.e(TAG, "parseDetectionEventData() SM type " + soundModelInfo.type + " unsupported!");
             return null;
         }

         // There is a one-to-one mapping with the version of the SM and
         // the format of the Detection Event Data
         if (soundModelInfo.version == VERSION_0100) {
             Log.d(TAG, "SM type is SVA 1.0");
             // Create object of type VoiceWakeupDetectionData version 1,
             VoiceWakeupDetectionData vwuDetData = new VoiceWakeupDetectionData();
             // parse SVA 1.0 black box eventPayload data and specifically fill
             //    VoiceWakeupDetectionData fields
             vwuDetData.status = parseVWUDetectionEventData(registeredSoundModel,
                                    eventPayload, vwuDetData);
             vwuDetData.type = ListenTypes.VWU_EVENT_0100;
             detData = vwuDetData;
         } else {
             // Version_2.0 and above
             Log.d(TAG, "SM type is SVA 2.0");
             // Create object of type VoiceWakeupDetectionData version 2,
             //    which extends VoiceWakeupDetectionData class
             VoiceWakeupDetectionDataV2 vwuDetDataV2 = new VoiceWakeupDetectionDataV2();
             // parse SVA 2.0 and above black box eventPayload data and specifically fill
             //     VoiceWakeupDetectionDataV2 fields
             vwuDetDataV2.status = parseVWUDetectionEventDataV2(registeredSoundModel,
                                    eventPayload, vwuDetDataV2);
             vwuDetDataV2.type = ListenTypes.VWU_EVENT_0200;
             detData = vwuDetDataV2;
         }

         if ( detData == null ) {
            Log.e(TAG, "parseDetectionEventData() returns null ptr ");
         } else if ( detData.status != ListenTypes.STATUS_SUCCESS ) {
            Log.e(TAG, "parseDetectionEventData() returns status " + detData.status);
            detData = null;
         }
         return detData;
    }

     /**
      * Parses SVA detection events data into the more meaningful
      * VoiceWakeupDetectionData data structure for SVA 1.0.
      * <p>
      * VWU_EVENT_0100 is output as the DetectionData.type field value.
      *
      * @param registeredSoundModel [in] SoundModel that was used
      *        for registration/detection
      * @param eventPayload [in] event payload returned by
      *        ListenEngine
      * @param vwuDetData [out] VoiceWakeupDetectionData
      *        structure filled when eventPayload is parsed
      * @return
      *        <br> STATUS_SUCCESS
      *        <br> STATUS_EBAD_PARAM
      *        <br> STATUS_EFAILURE
      *        <br> STATUS_ENO_MEMORY
      */
     private static native int parseVWUDetectionEventData(
                 ByteBuffer               registeredSoundModel,
                 EventData                eventPayload,
                 VoiceWakeupDetectionData vwuDetData);

     /**
      * Parses generic detection events data into the more meaningful
      * VoiceWakeupDetectionDataV2 data structure for SVA 2.0 and above.
      * <p>
      * VWU_EVENT_0200 is output as the DetectionData.type field value.
      *
      * @param registeredSoundModel [in] SoundModel that was used
      *        for registration/detection
      * @param eventPayload [in] event payload returned by
      *        ListenEngine
      * @param vwuV2DetData [out] VoiceWakeupDetectionDataV2
      *        structure filled when eventPayload is parsed
      * @return
      *        <br> STATUS_SUCCESS
      *        <br> STATUS_EBAD_PARAM
      *        <br> STATUS_EFAILURE
      *        <br> STATUS_ENO_MEMORY
      */
     private static native int parseVWUDetectionEventDataV2(
                 ByteBuffer                 registeredSoundModel,
                 EventData                  eventPayload,
                 VoiceWakeupDetectionDataV2 vwuV2DetData);
     /**
      * Query information about SoundModel.
      * <p>
      * The data structure created, filled and returned by this
      *   method is determined by the soundmodel type and version.
      * <p>
      * For example, if type is "VoiceWakeup_*", and
      *   version is "Version_2.0" or above,
      *   then a VWUSoundModelInfoV2 object will be created
      *   and returned.
      *
      * @param soundModel [in] SoundModel to query
      * @param soundModelInfo [out] reference to object that is a
      *     child of SoundModelInfo class, allocated in this method
      *
      * @return SoundModelInfo child object created by this method if successful;
      *        <br> null returned if error occurs
      */
     public static SoundModelInfo query(ByteBuffer soundModel)
     {
         int             status = ListenTypes.STATUS_SUCCESS;
         SoundModelInfo  genSMInfo = new SoundModelInfo();
         SVASoundModelInfo  soundModelInfo = new SVASoundModelInfo();
         Log.d(TAG, "query() getTypeVersion");
         // get generic info common to all soundmodel types
         status = getTypeVersion( soundModel, genSMInfo);
         if (status != ListenTypes.STATUS_SUCCESS) {
             Log.e(TAG, "query() getTypeVersion failed, returned " + status);
             return null;
         }
         soundModelInfo.type = genSMInfo.type;
         soundModelInfo.version = genSMInfo.version;
         soundModelInfo.size = genSMInfo.size;

         // check that SM is SVA type
         if (soundModelInfo.type != SVA_SOUNDMODEL_TYPE) {
             Log.e(TAG, "query() SM type " + genSMInfo.type + " unsupported!");
             return null;
         }

         if (genSMInfo.version == VERSION_0100)
         {
             Log.d(TAG, "query() only returns type and version for SVA 1.0 SoundModel");
             // call new native function to get extra V1 Info
             KeywordUserCounts  counts = new KeywordUserCounts();
             soundModelInfo.counts = counts;
             soundModelInfo.counts.numKeywords = 1;
             soundModelInfo.counts.numUsers = 0;
             soundModelInfo.counts.numUserKWPairs = 1;
             soundModelInfo.keywordInfo = null;
             soundModelInfo.userNames = null;
         }
         else
         {
             Log.d(TAG, "query() getInfoV2 called");
             // call new native function to get extra V2 Info
             status = getInfo(soundModel, soundModelInfo); // get num keys and num users
             if (status != ListenTypes.STATUS_SUCCESS) {
                Log.e(TAG, "query() getInfoV2 failed, returned " + status);
                return null;
             }
         }
         return soundModelInfo;
     }

     /**
      * Gets generic header fields common to all SoundModels.
      * <p>
      * Common SoundModelInfo type, version and size fields are
      * guaranteed to be returned by this query.
      *
      * @param soundModel [in] SoundModel to query
      * @param soundModelInfo [out] SoundModelInfo object filled by this method
      * @return status
      *        <br> STATUS_SUCCESS
      *        <br> STATUS_EBAD_PARAM
      */
     public static native int getTypeVersion(
                 ByteBuffer       soundModel,
                 SoundModelInfo   soundModelInfo);

     /**
      * Gets more detailed information about a version 2.0 and above SVA SoundModel
      * content.
      *
      * @param soundModel [in] SoundModel to query
      * @param numKeywords [in]  number of string entries allocated in keywords array
      * @param soundModelInfo [out] reference to VWUSoundModelInfo structure
      * @return status
      *        <br> STATUS_SUCCESS
      *        <br> STATUS_EBAD_PARAM
      */
     private static native int getInfo(
                 ByteBuffer          soundModel,
                 SVASoundModelInfo   vwuSMInfo);

    /**
     * Calculates size of soundmodel when array of input SoundModels are merged.
     * <p>
     * The size returned by this method must be used to create a
     * ByteBuffer that can hold the SoundModel created by
     * {@link merge(ByteBuffer[], ByteBuffer)}.
     *
     * @param  soundModels [in] array of soundModels to be merged
     *
     * @return total (unsigned) size of merged SoundModel
     * <br> Negative status value returned if error occurs.
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_ESOUNDMODELS_WITH_SAME_KEYWORD_CANNOT_BE_MERGED
     *        <br>   STATUS_ESOUNDMODELS_WITH_SAME_USER_KEYWORD_PAIR_CANNOT_BE_MERGED
     *        <br>   STATUS_EMAX_KEYWORDS_EXCEEDED
     *        <br>   STATUS_EMAX_USERS_EXCEEDED
     *
     * <p>
     * Refer to {@link merge(ByteBuffer[], ByteBuffer)}
     */
     public static native int getSizeWhenMerged(
             ByteBuffer        soundModels[]);

    /**
     * Merges two or more SoundModels into a single SM.
     * <p>
     * There can not be more than one SoundModel that has keyword data
     * for the same keyword phrase.
     * <p>
     * Application is responsible for creating a ByteBuffer large
     * enough to hold the SoundModel output by this method.
     * The size of the output SoundModel can be determined calling
     * {@link getSizeWhenMerged(ByteBuffer[])}.
     *
     * @param  soundModels [in] array of soundModels to merge
     * @param  mergedSoundModel [out]  resulting merged SoundModel
     *
     * @return status
     *        <br>   STATUS_SUCCESS
     *        <br>   STATUS_EBAD_PARAM
     *        <br>   STATUS_ESOUNDMODELS_WITH_SAME_KEYWORD_CANNOT_BE_MERGED
     *        <br>   STATUS_ESOUNDMODELS_WITH_SAME_USER_KEYWORD_PAIR_CANNOT_BE_MERGED
     *        <br>   STATUS_EMAX_KEYWORDS_EXCEEDED
     *        <br>   STATUS_EMAX_USERS_EXCEEDED
     *
     * <p>
     * Refer to {@link getSizeWhenMerged(ByteBuffer[])}
     */
     public static native int merge(
             ByteBuffer        soundModels[],
             ByteBuffer        mergedSoundModel);

     /**
       * Get the size required to hold SoundModel that would be created by
       * {@link deleteData(ByteBuffer, String, String, ByteBuffer)}
       * executed for a keyword, a user, or a user+keyword pair.
       * <p>
       * The size returned by this method must be used to create a
       * ByteBuffer that can hold the SoundModel created by deleteData().
	   *
       * @param inputSoundModel [in] SoundModel data is to be deleted from
       * @param keywordPhrase [in] name of keyword for which all data in SM should be delete
       * @param userName [in] name of user for which all data in SM should be delete
       *
       * @return total (unsigned) size of SoundModel after data is deleted
       * <br> Zero returned if error occurs.
       *        <br>   STATUS_EBAD_PARAM
       *        <br>   STATUS_EKEYWORD_NOT_IN_SOUNDMODEL
       *        <br>   STATUS_EUSER_NOT_IN_SOUNDMODEL
       *        <br>   STATUS_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL
       *        <br>   STATUS_ECANNOT_DELETE_LAST_KEYWORD
       *
       * <p>
       * Refer to {@link deleteData(ByteBuffer, String, String, ByteBuffer)}
	   */
      public static native int getSizeAfterDelete(
             ByteBuffer        inputSoundModel,
             String            keywordPhrase,
             String            userName);

     /**
       * Deletes specific data from a given SoundModel
       * <p>
       * Returns a new SoundModel after removing data a keyword, a user,
       * or a user+keyword pair from a given SoundModel.  Exactly which data is
       * deleted depends on combination of input parameter Strings
       * keywordPhrase and userName.
       * <p>
       * If keywordPhrase is non-null, but userName is null then all data associated
       * with a particular keyword (including user+KW pairings) is removed.
       * <p>
       * If keywordPhrase is null, but userName is non-null then all data associated
       * with a particular user (including user+keyword pairings) is removed
       * <p>
       * If both keywordPhrase and userName are non-null then only data for a
       * particular user+keyword pair is removed
       * <p>
       * Application is responsible for creating a ByteBuffer large
       * enough to hold the SoundModel output by this method.
       * The size of the output SoundModel can be determined calling
       * {@link getSizeAfterDelete(ByteBuffer,String, String)}.
       *
       * @param inputSoundModel [in] SoundModel data is to be deleted from
       * @param keywordPhrase [in] name of keyword for which all data in SM should be delete
       * @param userName [in] name of user for which all data in SM should be delete
       * @param outputSoundModel [out] new SoundModel with data removed
       *
       * @return status
       *        <br>   STATUS_SUCCESS
       *        <br>   STATUS_EBAD_PARAM
       *        <br>   STATUS_EKEYWORD_NOT_IN_SOUNDMODEL
       *        <br>   STATUS_EUSER_NOT_IN_SOUNDMODEL
       *        <br>   STATUS_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL
       *        <br>   STATUS_ECANNOT_DELETE_LAST_KEYWORD
       *
       * <p>
       * Refer to {@link getSizeAfterDelete(ByteBuffer,String, String)}
       */
      public static native int deleteData(
             ByteBuffer        inputSoundModel,
             String            keywordPhrase,
             String            userName,
             ByteBuffer        outputSoundModel);
}
