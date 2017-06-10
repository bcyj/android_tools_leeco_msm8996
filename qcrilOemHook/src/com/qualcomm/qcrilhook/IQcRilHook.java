/******************************************************************************
 * @file    IQcRilHook.java
 * @brief   Defines the interface provided by the qcrilhook library to Android
 *          Applications. The qcrilhook library adds Qualcomm OEM Header
 *          to Qualcomm RIL OEM Hook Messages and send them using the
 *          RIL OEM Hook interface.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2009 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qualcomm.qcrilhook;

import android.os.AsyncResult;
import android.os.Handler;

/**
 * Interface provided by the QcRilHook library. Applications use this library to
 * interact with qcril using the OEM RIL HOOK RAW interface.
 */
public interface IQcRilHook {

    /** Starting number for QCRILHOOK request and response IDs */
    int QCRILHOOK_BASE = 0x80000;

    /** Starting number for QMI message request and response IDs */
    int SERVICE_PROGRAMMING_BASE = 4096; //0x1000

    /** Qcril level Request ID base **/
    int QCRILHOOK_REQUEST_ID_BASE = QCRILHOOK_BASE + 1;

    /** Qcril level Request ID MAX **/
    int QCRILHOOK_REQUEST_ID_MAX = QCRILHOOK_BASE + 99;

    /**
     * QMI OEM HOOK REQUEST/UNSOL ID common for VT, EMBMS, PRESENCE and all
     * future clients
     */
    int QCRILHOOK_QMI_OEMHOOK_REQUEST_ID = QCRILHOOK_BASE + 100;

    /**
     * QcRilHook level UNSOL ID base
     */
    int QCRILHOOK_UNSOL_BASE = QCRILHOOK_BASE + 1000;

    /**
     * QcRilHook level UNSOL ID base
     */
    int QCRILHOOK_UNSOL_MAX = QCRILHOOK_BASE + 1099;

    /**
     * QMI OEM HOOK UNSOL ID common for VT, EMBMS, PRESENCE and all future
     * clients
     */
    int QCRILHOOK_UNSOL_OEMHOOK = QCRILHOOK_BASE + 1100;

    /**
     * qcrilhook requests Ids
     */
    /** Read NV Item */
    int QCRILHOOK_NV_READ = QCRILHOOK_BASE + 1;

    /** Write NV Item */
    int QCRILHOOK_NV_WRITE = QCRILHOOK_BASE + 2;

    /** Set interface to dormant */
    int QCRILHOOK_GO_DORMANT = QCRILHOOK_BASE + 3;

    /** De-activate SIM personalization */
    int QCRILHOOK_ME_DEPERSONALIZATION = QCRILHOOK_BASE + 4;

    /** Set Tune Away */
    int QCRIL_EVT_HOOK_SET_TUNEAWAY = QCRILHOOK_BASE + 5;

    /** Get Tune Away */
    int QCRIL_EVT_HOOK_GET_TUNEAWAY = QCRILHOOK_BASE + 6;

    /** Set Priority subscription */
    int QCRIL_EVT_HOOK_SET_PAGING_PRIORITY = QCRILHOOK_BASE + 7;

    /** Get Priority subscription */
    int QCRIL_EVT_HOOK_GET_PAGING_PRIORITY = QCRILHOOK_BASE + 8;

    /** Inform shut down so that RIL can issue card power down */
    int QCRIL_EVT_HOOK_INFORM_SHUTDOWN = QCRILHOOK_BASE + 10;

    /** Set CDMA Subscription Source with SPC(Service Programming Code) **/
    int QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC = QCRILHOOK_BASE + 11;
    /** Avoid the current cdma network */
    int QCRIL_EVT_HOOK_CDMA_AVOID_CUR_NWK = QCRILHOOK_BASE + 14;

    /** Clear the cdma avoidance list */
    int QCRIL_EVT_HOOK_CDMA_CLEAR_AVOIDANCE_LIST = QCRILHOOK_BASE + 15;

    /** Get the cdma avoidance list */
    int QCRIL_EVT_HOOK_CDMA_GET_AVOIDANCE_LIST = QCRILHOOK_BASE + 16;

    /** Enable/Disable Engineer mode */
    int QCRIL_EVT_HOOK_ENABLE_ENGINEER_MODE = QCRILHOOK_BASE + 19;

    /** Set builtin PLMN list */
    int QCRIL_EVT_HOOK_SET_BUILTIN_PLMN_LIST = QCRILHOOK_BASE + 17;

    /** Perform incremental nw scan request */
    int QCRIL_EVT_HOOK_PERFORM_INCREMENTAL_NW_SCAN = QCRILHOOK_BASE + 18;

    /** Device Configuration messages */
    int QCRIL_EVT_HOOK_SET_CONFIG = QCRILHOOK_BASE + 21;
    int QCRIL_EVT_HOOK_GET_CONFIG = QCRILHOOK_BASE + 22;
    int QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS = QCRILHOOK_BASE + 23;
    int QCRIL_EVT_HOOK_DELETE_ALL_CONFIGS = QCRILHOOK_BASE + 31;
    int QCRIL_EVT_HOOK_SEL_CONFIG = QCRILHOOK_BASE + 32;
    int QCRIL_EVT_HOOK_GET_META_INFO = QCRILHOOK_BASE + 33;
    int QCRIL_EVT_HOOK_DEACT_CONFIGS = QCRILHOOK_BASE + 44;
    int QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE = QCRILHOOK_BASE + 45;
    int QCRIL_EVT_HOOK_VALIDATE_CONFIG = QCRILHOOK_BASE + 46;
    int QCRIL_EVT_HOOK_GET_QC_VERSION_OF_ID = QCRILHOOK_BASE + 47;
    int QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE = QCRILHOOK_BASE + 48;
    int QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_ID = QCRILHOOK_BASE + 49;
    int QCRIL_EVT_HOOK_ACT_CONFIGS = QCRILHOOK_BASE + 50;

    /** Set/Get Preferred Network Acq Order */
    int QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_ACQ_ORDER = QCRILHOOK_BASE + 27;
    int QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_ACQ_ORDER = QCRILHOOK_BASE + 28;

    /** Set Band info */
    int QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_BAND_PREF = QCRILHOOK_BASE + 37;
    int QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_BAND_PREF = QCRILHOOK_BASE + 38;

    /** Send DDS Info */
    int QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION = QCRILHOOK_BASE + 39;

    /** Send data enable status */
    int QCRIL_EVT_HOOK_SET_IS_DATA_ENABLED = QCRILHOOK_BASE + 40;

    /** Send data roaming status */
    int QCRIL_EVT_HOOK_SET_IS_DATA_ROAMING_ENABLED = QCRILHOOK_BASE + 41;

    /** Send default apn info */
    int QCRIL_EVT_HOOK_SET_APN_INFO = QCRILHOOK_BASE + 42;

    /** Set LTE Tune Away */
    int QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY = QCRILHOOK_BASE + 43;

    /** Set reactivation personalization */
    int QCRIL_EVT_HOOK_SET_PERSONALIZATION = QCRILHOOK_BASE + 1025;

    /** Get personalization status */
    int QCRIL_EVT_HOOK_GET_PERSONALIZATION_STATUS = QCRILHOOK_BASE + 1026;

    /**
     * qcrilhook unsolicited response IDs
     */
    int QCRILHOOK_UNSOL_EXTENDED_DBM_INTL = QCRILHOOK_UNSOL_BASE;

    int QCRILHOOK_UNSOL_CDMA_BURST_DTMF = QCRILHOOK_UNSOL_BASE + 1;

    int QCRILHOOK_UNSOL_CDMA_CONT_DTMF_START = QCRILHOOK_UNSOL_BASE + 2;

    int QCRILHOOK_UNSOL_CDMA_CONT_DTMF_STOP = QCRILHOOK_UNSOL_BASE + 3;

    int QCRILHOOK_UNSOL_LOCAL_RINGBACK_START = QCRILHOOK_UNSOL_BASE + 4;

    int QCRILHOOK_UNSOL_LOCAL_RINGBACK_STOP = QCRILHOOK_UNSOL_BASE + 5;

    /** Device Configuration UNSOL messages */
    int QCRILHOOK_UNSOL_PDC_CONFIG = QCRILHOOK_UNSOL_BASE + 14;
    int QCRILHOOK_UNSOL_PDC_CLEAR_CONFIGS = QCRILHOOK_UNSOL_BASE + 17;

    /**
     * QMI message IDs for Service Programming
     */
    int QCRILHOOK_NAS_UPDATE_AKEY = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE;

    int QCRILHOOK_NAS_GET_3GPP2_SUBSCRIPTION_INFO = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 1;

    int QCRILHOOK_NAS_SET_3GPP2_SUBSCRIPTION_INFO = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 2;

    int QCRILHOOK_NAS_GET_MOB_CAI_REV = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 3;

    int QCRILHOOK_NAS_SET_MOB_CAI_REV = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 4;

    int QCRILHOOK_NAS_GET_RTRE_CONFIG = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 5;

    int QCRILHOOK_NAS_SET_RTRE_CONFIG = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 6;

    int QCRILHOOK_DMS_GET_FTM_MODE = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 7;

    int QCRILHOOK_DMS_GET_SW_VERSION = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 8;

    int QCRILHOOK_DMS_UPDATE_SERVICE_PROGRAMING_CODE = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 9;

    int QCRILHOOK_DMS_GET_DEVICE_SERIAL_NUMBERS = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 10;

    int QCRILHOOK_DMS_GET_SPC_CHANGE_ENABLED = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 11;

    int QCRILHOOK_DMS_SET_SPC_CHANGE_ENABLED = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 12;

    int QCRILHOOK_VOICE_SET_CONFIG = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 13;

    int QCRILHOOK_VOICE_GET_CONFIG = QCRILHOOK_BASE + SERVICE_PROGRAMMING_BASE + 14;

    /**
     * Gets the current device configuration form RIL
     * @return String id of the current configuration or null.
     * Errors are logged.
     */
    public String qcRilGetConfig();

    /**
     * Clean up all configurations loaded in EFS
     * @return true on success, false otherwise
     */
    public boolean qcRilCleanupConfigs();

    /**
     * Sets the specified device configuration in RIL
     * Note: A true value returned from this function only indicates that
     * RIL recieved the command, and not that the confugration has actually
     * been set. Application must wait for QCRIL_EVT_HOOK_UNSOL_SET_CONFIG
     * message to determine when configuration has been activated in the modem.
     *
     * @param config The configuration to be set
     *
     * @return true on success, false otherwise.
     */
    public boolean qcRilSetConfig(String config);

    /**
     * Gets a list of available device configurations from RIL
     *
     * @param device One of EFS, SIM1, or SIM2
     *
     * @return A list of configurations or null
     */
    public String[] qcRilGetAvailableConfigs(String device);

    /**
     * Sends a QCRILHOOK_GO_DORMANT request to qcril along with the
     * interfaceName if specified.
     *
     * @param interfaceName - optional. If included it is the private data
     *            member of PdpConnection returned by RIL when the interface was
     *            set up. Forces the specified interfaceName to go dormant. If
     *            no interface is specified, forces all interfaces to go
     *            dormant.
     * @return true on SUCCESS and false if the specified interface does not
     *         exist or if no interface is set up.
     */
    public boolean qcRilGoDormant(String interfaceName);

    /**
     * Sends a QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC request to qcril
     *
     * @return true on SUCCESS and false if unable to set CDMA Subscription Source
     */
    public boolean qcRilSetCdmaSubSrcWithSpc(int cdmaSubscription, String spc);

    /**
     * Sends a QCRIL_EVT_HOOK_SET_TUNEAWAY request to qcril
     *
     * @param tuneAway - Enable/Disable TuneAway setting
     * @return true on SUCCESS and false if unable to set TuneAway
     */
    public boolean qcRilSetTuneAway(boolean tuneAway);

    /**
     * Sends a QCRIL_EVT_HOOK_GET_TUNEAWAY request to qcril
     *
     * @return tuneAway value
     */
    public boolean qcRilGetTuneAway();

    /**
     * Sends a QCRIL_EVT_HOOK_SET_PAGING_PRIORITY request to qcril
     *
     * @param priorityIndex - subscription index
     * @return true on SUCCESS and false if unable to set Priority Subscription
     */
    public boolean qcRilSetPrioritySubscription(int priorityIndex);

    /**
     * Sends a QCRIL_EVT_HOOK_GET_PAGING_PRIORITY request to qcril
     *
     * @return subscription index
     */
    public int qcRilGetPrioritySubscription();
     /**
     * Adds the QC_RIL_OEM_HOOK_REQ header to the data and uses RIL interface to
     * send the message to qcril.
     *
     * @return AsyncResult (exception set to zero on success and result contains
     *         response data)
     */
    public AsyncResult sendQcRilHookMsg(int requestId);

     /**
     * Adds the QC_RIL_OEM_HOOK_REQ header to the data and uses RIL interface to
     * send the message to qcril.
     *
     * @return AsyncResult (exception set to zero on success and result contains
     *         response data)
     */
    public AsyncResult sendQcRilHookMsg(int requestId, byte payload);

    /**
     * Adds the QC_RIL_OEM_HOOK_REQ header to the data and uses RIL interface to
     * send the message to qcril.
     *
     * @return AsyncResult (exception set to zero on success and result contains
     *         response data)
     */
    public AsyncResult sendQcRilHookMsg(int requestId, byte[] payload);

    /**
     * Adds the QC_RIL_OEM_HOOK_REQ header to the data and uses RIL interface to
     * send the message to qcril.
     *
     * @return AsyncResult (exception set to zero on success and result contains
     *         response data)
     */
    public AsyncResult sendQcRilHookMsg(int requestId, int payload);

    /**
     * Adds the QC_RIL_OEM_HOOK_REQ header to the data and uses RIL interface to
     * send the message to qcril. Method is used when the payload for the
     * request is a String.
     *
     * @return AsyncResult (exception set to zero on success and result contains
     *         response data)
     */
    public AsyncResult sendQcRilHookMsg(int requestId, String payload);

    /**
     * Stores the handlers that have registered for Field Test unsolicited
     * responses and notifies them when the event happens. When the Unsolicited
     * Response with Field Test Data is received, the handleMessage in the
     * Handler object will be invoked with a Message. The Message.what field
     * will be set to the 'what' argument provided in this method.
     *
     * Messages received from this notification will be of the following format:
     *     Message.obj will be an AsyncResult
     *     AsyncResult.userObj = obj
     *     AsyncResult.result = Field test data (Data type still to be defined).
     *
     * @see android.os.Handler
     * @see android.os.Message
     */
    public void registerForFieldTestData(Handler h, int what, Object obj);

    /**
     * Removes the handlers from its internal list.
     */
    public void unregisterForFieldTestData(Handler h);

    /**
     * Stores the handlers that have registered for Extended Databurst Message
     * unsolicited responses and notifies them when the event happens. When the
     * Extended DBM of this type is received, the handleMessage in the Handler
     * object will be invoked with a Message. The Message.what field will be set
     * to the 'what' argument provided in this method.
     *
     * Messages received from this notification will be of the following format:
     *     Message.obj will be an AsyncResult
     *     AsyncResult.userObj = obj
     *     AsyncResult.result = QcRilExtendedDbmIntlKddiAocr instance.
     *
     * @see android.os.Handler
     * @see android.os.Message
     */
    public void registerForExtendedDbmIntl(Handler h, int what, Object obj);

    /**
     * Removes the handlers from its internal list.
     */
    public void unregisterForExtendedDbmIntl(Handler h);

    /**
     * Custom Data Burst Message (DBM) sent by KDDI network to inform the user
     * of call charge rate. The AOCR is sent in the Data Burst Message(DBM) and
     * the international extended information record (Extended Record Type-
     * International) is used.
     */
    public class QcRilExtendedDbmIntlKddiAocr {
        public short mcc; // Mobile Country Code (fixed '0101010011')

        public byte db_subtype; // Data Burst subtype (fixed '000001')

        public byte chg_ind; // Charge Indication

        public byte sub_unit; // Unit call time (1/10 second)

        public byte unit; // Unit call time (second)
    }
}
