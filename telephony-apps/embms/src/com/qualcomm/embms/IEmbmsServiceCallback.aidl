
/******************************************************************************
 * @file    IEmbmsServiceCallback.aidl
 * @brief   This interface describes the APIs for the callback that a client
 *          which uses IEmbmsService should implement in order to be notified
 *          of asynchronous events.
 *
 * @version 00.08.02
 *
 * Copyright © 2011-2014 Qualcomm Technologies, Inc.  All rights reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/

package com.qualcomm.embms;

import com.qualcomm.embms.IEmbmsService;

interface IEmbmsServiceCallback {

    /**
     * enableResponse will be invoked as a response to enable()
     * call to eMBMS service
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param responseCode
     *    Defines the response code for the action taken by eMBMS
     *    service for enable() call from MSDC. It can take the
     *    following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *    ERROR_RADIO_UNAVAILABLE 2
     *       Action taken by eMBMS service has failed as the radio is unavailable.
     *
     * @param interfaceName
     *    Defines the name of the interface to be used
     *
     * @param interfaceIndex
     *    Defines the index or id of the interface to be used
     *
     * @return None
     */
     void enableResponse(in int debugTraceId, in int responseCode, in String interfaceName,
                                                                   in int interfaceIndex);

    /**
     * disableResponse will be invoked as a response to disable()
     * call to eMBMS service
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param responseCode
     *    Defines the response code for the action taken by eMBMS
     *    service for disable() call from MSDC. It can take the
     *    following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *
     * @return None
     */
     void disableResponse(in int debugTraceId, in int responseCode);

    /**
     * activateTMGIResponse will be invoked as a response to activateTMGI()
     * call to eMBMS service
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param responseCode
     *    Defines the response code for the action taken by eMBMS
     *    service for activateTMGI() call from MSDC. It can take the
     *    following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *    ERROR_ALREADY_DONE 2
     *       No action taken by eMBMS service as the requested TMGI is already
     *       active
     *    ERROR_NOT_ALLOWED 3
     *       Action taken by eMBMS service has failed has failed as the eMBMS
     *       capability is not enabled on the device
     *    ERROR_MISSING_CONTROL_INFO 4
     *       Action taken by eMBMS service has failed because some or none of
     *       the MCCHs can be acquired
     *    ERROR_MISSING_TMGI 5
     *       Action taken by eMBMS service has failed because all MCCHs were
     *       read and TMGI of interest cannot be found
     *    ERROR_MCAST_OOC 6
     *       Action taken by eMBMS service has failed because the device is in
     *       Broadcast Out of Coverage area
     *    ERROR_UCAST_OOS 7
     *       Action taken by eMBMS service has failed because the device is in
     *       Unicast Out of Service area
     *    ERROR_FREQUENCY_CONFLICT 8
     *       Action taken by eMBMS service has failed because the device is
     *       already camped to a frequency which is not in the set of
     *       frequencies provided in the activateTMGI request
     *    ERROR_MAX_TMGI_ALREADY_ACTIVE 9
     *       Action taken by eMBMS service has failed because the number of
     *       TMGI's already active has reached the maximum allowed limit
     *    SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_IDLE 100
     *       Action taken by eMBMS service was successful, the bearer is not
     *       active yet. The device was in Unicast Idle mode and the
     *       reconfiguration is in progress in order to activate the bearer
     *    SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_CONNECTED 101
     *       Action taken by eMBMS service was successful, the bearer is not
     *       active yet. The device was in Unicast connected mode and the
     *       reconfiguration is in progress in order to activate the bearer
     *
     * @param tmgi
     *    Defines the TMGI for which this response has been sent
     *
     * @return None
     */
     void activateTMGIResponse(in int debugTraceId, in int responseCode, in byte[] tmgi);

    /**
     * deactivateTMGIResponse will be invoked as a response to deactivateTMGI()
     * call to eMBMS service
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param responseCode
     *    Defines the response code for the action taken by eMBMS
     *    service for deactivateTMGI() call from MSDC. It can take the
     *    following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *    ERROR_ALREADY_DONE 2
     *       No action taken by eMBMS service as the requested TMGI is not
     *       active
     *    ERROR_NOT_ALLOWED 3
     *       Action taken by eMBMS service has failed has failed as the eMBMS
     *       capability is not enabled on the device
     *
     * @param tmgi
     *    Defines the TMGI for which this response has been sent
     *
     * @return None
     */
     void deactivateTMGIResponse(in int debugTraceId, in int responseCode, in byte[] tmgi);

    /**
     * actDeactTMGIResponse will be invoked as a response to actDeactTMGI()
     * call to eMBMS service
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param actResponseCode
     *    Defines the response code for the action taken by eMBMS
     *    service for activate request for a TMGI via actDeactTMGI call from MSDC.
     *    It can take the following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *    ERROR_ALREADY_DONE 2
     *       No action taken by eMBMS service as the requested TMGI is already
     *       active
     *    ERROR_NOT_ALLOWED 3
     *       Action taken by eMBMS service has failed has failed as the eMBMS
     *       capability is not enabled on the device
     *    ERROR_MISSING_CONTROL_INFO 4
     *       Action taken by eMBMS service has failed because some or none of
     *       the MCCHs can be acquired
     *    ERROR_MISSING_TMGI 5
     *       Action taken by eMBMS service has failed because all MCCHs were
     *       read and TMGI of interest cannot be found
     *    ERROR_MCAST_OOC 6
     *       Action taken by eMBMS service has failed because the device is in
     *       Broadcast Out of Coverage area
     *    ERROR_UCAST_OOS 7
     *       Action taken by eMBMS service has failed because the device is in
     *       Unicast Out of Service area
     *    ERROR_FREQUENCY_CONFLICT 8
     *       Action taken by eMBMS service has failed because the device is
     *       already camped to a frequency which is not in the set of
     *       frequencies provided in the actDeactTMGI request
     *    ERROR_MAX_TMGI_ALREADY_ACTIVE 9
     *       Action taken by eMBMS service has failed because the number of
     *       TMGI's already active has reached the maximum allowed limit
     *    SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_IDLE 100
     *       Action taken by eMBMS service was successful, the bearer is not
     *       active yet. The device was in Unicast Idle mode and the
     *       tune in is in progress in order to activate the bearer
     *    SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_CONNECTED 101
     *       Action taken by eMBMS service was successful, the bearer is not
     *       active yet. The device was in Unicast connected mode and the
     *       tune in is in progress in order to activate the bearer
     *
     * @param actTMGI
     *    Defines the TMGI requested to be activated in the actDeactTMGI request
     *    to eMBMS service
     *
     * @param deactResponseCode
     *    Defines the response code for the action taken by eMBMS
     *    service for deactivate request for a TMGI via actDeactTMGI call from MSDC.
     *    It can take the following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *    ERROR_ALREADY_DONE 2
     *       No action taken by eMBMS service as the requested TMGI is not
     *       active
     *    ERROR_NOT_ALLOWED 3
     *       Action taken by eMBMS service has failed has failed as the eMBMS
     *       capability is not enabled on the device
     *
     * @param deactTMGI
     *    Defines the TMGI requested to be deactivated in the actDeactTMGI request
     *    to eMBMS service
     *
     * @return None
     */
     void actDeactTMGIResponse(in int debugTraceId, in int actResponseCode, in byte[] actTMGI,
                                                    in int deactResponseCode, in byte[] deactTMGI);

    /**
     * signalStrengthResponse will be invoked as a response to getSignalStrength()
     * call to eMBMS service
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param responseCode
     *    Defines the response code for the action taken by eMBMS
     *    service for getSignalStrength() call from MSDC. It can take the
     *    following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *
     * @param MBSFN_Area_ID
     *    List of all the MBSFN area Id's where the device is residing currently
     *
     * @param SNR
     *    List of Signal to noise(SNR) ratio of the signal. Each element represents
     *    the value corresponding to the list in MBSFN_Area_ID parameter.
     *    -10db to 30db - Represents the valid range of SNR
     *
     * @param excessSNR
     *    List of excess SNR where each element represents the value corresponding
     *    to the list in MBSFN_Area_ID parameter. Excess SNR is the difference
     *    between the measured SNR and the base SNR level required to decode
     *    the base MCS with 1% error on an AWGN channel
     *
     * @param numberOfTMGIperMBSFN
     *    List of the number of Active TMGI's where each element represents the
     *    value corresponding to the list in MBSFN_Area_ID parameter.
     *
     * @param ActiveTMGIList
     *    List of Active TMGI's for each MBSFN area. The start of the list should
     *    have all the TMGI's corresponding to the first element of MBSFN_Area_ID
     *    and then all the TMGI's corresponding to second element of MBSFN_Area_ID
     *    and so on.
     *
     * @return None
     */
    void signalStrengthResponse(in int debugTraceId, in int responseCode, in int[] MBSFN_Area_ID,
                                    in float[] SNR, in float[] excessSNR,
                                    in int[] numberOfTMGIperMBSFN, in byte[] ActiveTMGIList);

    /**
     * availableTMGIListNotification will give the list of all active TMGI's.
     * This may be invoked as response to getAvailableTMGIList() or whenever
     * there is a change in the list of available TMGI's
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this notification is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param tmgiList
     *    A list formatted as an array of TMGI's with each TMGI being a
     *    6 byte array. If this callback is a response to getAvailableTMGIList()
     *    and there is an error in eMBMS service or lower layers, the tmgiList
     *    will be empty
     *
     * @return None
     */
    void availableTMGIListNotification(in int debugTraceId, in byte[] tmgiList);

    /**
     * activeTMGIListNotification will give the list of all active TMGI's.
     * This may be invoked as response to getActiveTMGIList() or whenever
     * there is a change in the list of active TMGI's
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this notification is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param tmgiList
     *    A list formatted as an array of TMGI's with each TMGI being a
     *    6 byte array. If this callback is a response to getActiveTMGIList()
     *    and there is an error in eMBMS service or lower layers, the tmgiList
     *    will be empty
     *
     * @return None
     */
    void activeTMGIListNotification(in int debugTraceId, in byte[] tmgiList);

    /**
     * broadcastCoverageNotification will give the broadcast coverage state.
     * This may be invoked as response to getCoverageState() or whenever
     * there is a change in the broadcast coverage state
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this notification is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param state
     *    IN_COVERAGE 0
     *        Broadcast is in coverage
     *    OUT_OF_COVERAGE 1
     *        Broadcast is out of coverage
     *    OUT_OF_COVERAGE_DUE_TO_UEMODE 2
     *        Broadcast is out of coverage due to single radio limitation
     *    OUT_OF_COVERAGE_E911 3
     *        Broadcast is out of coverage due to E911 call or E911 callback in progress
     *
     * @return None
     */
    void broadcastCoverageNotification(in int debugTraceId, in int state);

    /**
     * oosNotification will give the out of service information.
     *
     * @param debugTraceId
     *    Defines the debug trace id of the notification
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param reason
     *    UNICAST_OOS 0
     *        Unicast is out of service
     *    MULTICAST_OOS 1
     *        Broadcast is out of service
     *    OOS_CLEARED 2
     *        Both Unicast and Multicast out of service is cleared
     *
     * @param tmgiList
     *    A list formatted as an array of TMGI's with each TMGI being a
     *    6 byte array. This list would be empty in case of OOS_CLEARED.
     *    In case of UNICAST_OOS or MULTICAST_OOS, this list will contain
     *    the list of impacted TMGI's
     *
     * @return None
     */
    void oosNotification(in int debugTraceId, int reason, in byte[] tmgiList);

/**
     * cellGlobalIdNotification will be invoked after enable() and then whenever
     * there is a change of the cell id in which the device resides currently.
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param mcc
     *    The Mobile Country Code(MCC) of the cell from which device is currently
     *    receiving the data
     *
     * @param mnc
     *    The Mobile Network Code(MNC) of the cell from which device is currently
     *    receiving the data
     *
     * @param cellId
     *    The cell id of the cell from which the device is currently receiving the
     *    data. The cell id string is in hexadecimal format.
     *
     * @return None
     */
     void cellGlobalIdNotification(in int debugTraceId, in String mcc, in String mnc,
                                                        in String cellId);

    /**
     * radioStateNotification will be to notify MSDC about the current state of
     * the radio after MSDC tries to enable the broadcast capability. It is then
     * invoked every time there is any change in the availability of the radio.
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this response is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param state
     *    Gives the state of the radio. It can take the following values:
     *    RADIO_AVAILABLE 0
     *        Represents that the radio is available
     *    RADIO_UNAVAILABLE 1
     *        Represents that the radio is unavailable(modem reset)
     *
     * @return None
     */
     void radioStateNotification(in int debugTraceId, in int state);

    /**
     * activeLogPacketIDsResponse will be invoked as a response to
     * getActiveLogPacketIDs() call to eMBMS service
     *
     * @param activeLogPacketIdList
     *    Contains the list of active MSDC log packet id's which need
     *    to be logged.
     *
     * @param activeLogPacketIdList
     *    Gives the list of active log packet id's
     *
     * @return None
     */
     void activeLogPacketIDsResponse(in int debugTraceId, in int[] activeLogPacketIdList);

    /**
     * saiNotification will be sent to MSDC after enable() and at any point of time
     * when there is a change in the the list of SAI's
     *
     * @param campedSAIList
     *    Gives the list of SAIs on the frequencies on which the device is camped to.
     *
     * @param numberofSAIperGroup
     *    Gives the list of SAIs available per SAI Group. An SAI group is a set of
     *    SAIs on which TMGI's can be activated at same time.
     *
     * @param availableSAIList
     *    Gives the list of all the available SAIs in each SAI group. The list is
     *    aligned to the values numberofSAIperGroup parameter.
     *
     * @return None
     */
     void saiNotification (in int debugTraceId, in int[] campedSAIList,
                            in int[] numberofSAIperGroup, in int[] availableSAIList);

   /**
     * timeResponse will be sent to MSDC in response to getTime()
     *
     * @param responseCode
     *    Defines the response code for the action taken by eMBMS
     *    service. It can take the following values:
     *    SUCCESS 0
     *       Action taken by eMBMS service is successful
     *    ERROR_UNKNOWN 1
     *       Action taken by eMBMS service has failed due to unknown reason
     *
     * @param timeMseconds
     *    UTC time in milli seconds w.r.t GPS time
     *
     * @param additionalInfo
     *    If TRUE, it indicates that dayLightSaving, leapSeconds and localTimeOffset
     *    parameters have valid values.
     *
     * @param dayLightSaving
     *    If TRUE, it indicates that the day light saving is on for the local time zone
     *
     * @param leapSeconds
     *    The number of leap seconds already accounted for in timeMseconds
     *
     * @param localTimeOffset
     *    Local time offset in 15min increments
     *
     * @return None
     */
     void timeResponse (in int debugTraceId,in int responseCode, in long timeMseconds,
                        in boolean additionalInfo, in boolean dayLightSaving, in int leapSeconds,
                        in long localTimeOffset);

   /**
     * e911Notification will give the E911 state.
     * This may be invoked as response to getE911State() or whenever
     * there is a change in the E911 state
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this notification is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param state
     *    NOT_IN_E911 0
     *        No E911 call nor E911 callback is in progress
     *    IN_E911 1
     *        E911 call or E911 callback is in progress
     *
     * @return None
     */
    void e911Notification(in int debugTraceId, in int state);

    /**
     * contentDescriptionPerObjectControl will give the
     * control information for content description update per
     * Object and will contain the Content and Status information.
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this notification is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param tmgi
     *    A byte array representing a 6 byte TMGI for which content description
     *    update is provided
     *
     * @param perObjectContentControl
     *    Value of per Object Content Control
     *
     * @param perObjectStatusControl
     *    Value of per Object Status Control
     *
     * @return None
     */
    void contentDescriptionPerObjectControl(in int debugTraceId, in byte[] tmgi,
            in int perObjectContentControl, in int perObjectStatusControl);

    /**
     * getPLMNListResponse will be sent to MSDC in response to
     * getPLMNListRequest() and will give the PLMN information.
     * Each PLMN is encoded in a 6 byte representation.
     *
     * @param debugTraceId
     *    Defines the trace id of the request corresponding to
     *    which this notification is being sent.
     *    0 to 32768 - Represents a valid trace id
     *    -1 - Represents that this parameter is not used
     *
     * @param plmnList
     *    A byte array representing 6 byte PLMN information
     *
     * @return None
     */
    void getPLMNListResponse(in int debugTraceId, in byte[] plmnList);
}

