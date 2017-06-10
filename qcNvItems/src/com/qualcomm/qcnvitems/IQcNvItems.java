/******************************************************************************
 * @file    IQcNvItems.java
 * @brief   Interface to read/write NV items via QMI or NV (legacy support).
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.qcnvitems;

import com.qualcomm.qcnvitems.QmiNvItemTypes.AmrStatus;
import com.qualcomm.qcnvitems.QmiNvItemTypes.AutoAnswer;
import com.qualcomm.qcnvitems.QmiNvItemTypes.CdmaChannels;
import com.qualcomm.qcnvitems.QmiNvItemTypes.MinImsi;
import com.qualcomm.qcnvitems.QmiNvItemTypes.PreferredVoiceSo;
import com.qualcomm.qcnvitems.QmiNvItemTypes.SidNid;
import com.qualcomm.qcnvitems.QmiNvItemTypes.Threegpp2Info;
import com.qualcomm.qcnvitems.QmiNvItemTypes.TimerCount;
import com.qualcomm.qcnvitems.QmiNvItemTypes.TrueImsi;
import com.qualcomm.qcnvitems.QmiNvItemTypes.VoiceConfig;

import java.io.IOException;
import java.security.InvalidParameterException;

public interface IQcNvItems {
    // Service Programming constants

    public static final int P_REV_JSTD008 = 0x01;

    public static final int P_REV_IS95A = 0x03;

    public static final int P_REV_IS95B = 0x04;

    public static final int P_REV_IS2000 = 0x06;

    public static final int P_REV_IS2000_RELA = 0x07;

    public static final int P_REV_IS2000_RELB = 0x08;

    public static final int P_REV_IS2000_RELC = 0x09;

    public static final int P_REV_IS2000_RELC_MI = 0x0A;

    public static final int P_REV_IS2000_RELD = 0x0B;

    // NV constants

    public static final int NV_MAX_MINS = 2;

    public static final int NV_DIR_NUMB_SIZ = 10;

    public static final int NV_MAX_SID_NID = 1;

    public static final int NV_MAX_NUM_OF_ECC_NUMBER = 10;

    public static final int NV_ECC_NUMBER_SIZE = 3;

    public static final int NV_SEC_CODE_SIZE = 6;

    public static final int NV_LOCK_CODE_SIZE = 4;

    public static final int NV_MAX_HOME_SID_NID = 20;

    // NAS
    /**
     * Updates NAS A-KEY.
     *
     * QMI Message:    QMI_NAS_UPDATE_AKEY;
     * NV Items:       NV_A_KEY_I;
     *
     * @param String representing the AKEY value to set.
     * @throws IOException if update operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */

    public void updateAkey(String akey) throws IOException, InvalidParameterException;

    /**
     * Gets NAS 3GPP2 Subscription Info.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_NAME_NAME_I (NAM Name)
     *                 NV_DIR_NUMBER_I (Directory Number),
     *                 NV_HOME_SID_NID_I (Home System ID/Network ID values),
     *                 NV_IMSI_MCC_I (Mobile Country Code),
     *                 NV_IMSI_11_12_I,
     *                 NV_MIN1_I,
     *                 NV_MIN2_I,
     *                 NV_IMSI_T_MCC_I (True Mobile Country Code),
     *                 NV_IMSI_T_11_12_I,
     *                 NV_IMSI_T_S1_I,
     *                 NV_IMSI_T_S2_I,
     *                 NV_IMSI_T_ADDR_NUM_I (Number of True IMSI Digits),
     *                 NV_PCDMACH_I (Primary CDMA Channels),
     *                 NV_SCDMACH_I (Secondary CDMA Channels);
     *
     * @return Structure Threegpp2Info containing 3GPP2 subscription info. on
     *         success.
     * @throws IOException if read operation fails.
     */

    public Threegpp2Info get3gpp2SubscriptionInfo() throws IOException;

    /**
     * Sets NAS 3GPP2 Subscription Info.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_DIR_NUMBER_I (Directory Number),
     *                 NV_HOME_SID_NID_I (Home System ID/Network ID values),
     *                 NV_IMSI_MCC_I (Mobile Country Code),
     *                 NV_IMSI_11_12_I,
     *                 NV_MIN1_I,
     *                 NV_MIN2_I,
     *                 NV_IMSI_T_MCC_I (True Mobile Country Code),
     *                 NV_IMSI_T_11_12_I,
     *                 NV_IMSI_T_S1_I,
     *                 NV_IMSI_T_S2_I,
     *                 NV_IMSI_T_ADDR_NUM_I (Number of True IMSI Digits),
     *                 NV_PCDMACH_I (Primary CDMA Channels),
     *                 NV_SCDMACH_I (Secondary CDMA Channels);
     *
     * @param Structure Threegpp2Info containing the 3GPP2 subscription info.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */

    public void set3gpp2SubscriptionInfo(Threegpp2Info threegpp2Info) throws IOException,
            InvalidParameterException;

    /**
     * Gets the NAM name.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_NAME_NAME_I (NAM Name);
     *
     * @return On success, String containing the NAM name.
     * @throws IOException if read operation fails.
     */

    public String getNamName() throws IOException;

    @Deprecated
    /**
     * Gets the analog home SID.
     *
     * QMI Message:    NOT IMPLEMENTED
     * NV Items:       NV_ANALOG_HOME_SID_I
     *
     * @return Integer containing the analog home SID on success.
     * @throws IOException if read operation fails.
     */

    public int getAnalogHomeSid() throws IOException;

    @Deprecated
    /**
     * Sets the analog home SID.
     *
     * QMI Message:    NOT IMPLEMENTED
     * NV Items:       NV_ANALOG_HOME_SID_I
     *
     * @param sid String containing the analog home SID.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setAnalogHomeSid(int sid) throws IOException, InvalidParameterException;

    /**
     * Read the Mobile Directory Number.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_DIR_NUMBER_I;
     *
     * @return String containing the MDN on success.
     * @throws IOException if read operation fails.
     */
    public String getDirectoryNumber() throws IOException;

    /**
     * Set the MDN (Mobile Directory Number).
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_DIR_NUMBER_I;
     *
     * @param directoryNumber String to set.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setDirectoryNumber(String directoryNumber) throws IOException,
            InvalidParameterException;

    /**
     * Gets the home SID/NID values.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_HOME_SID_NID_I;
     *
     * @return Structure SidNid containing SID/NID values on success.
     * @throws IOException if read operation fails.
     */
    public SidNid getHomeSidNid() throws IOException;

    /**
     * Sets the home SID/NID values.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_HOME_SID_NID_I;
     *
     * @param Structure SidNid containing SID/NID values on success.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setHomeSidNid(SidNid sidNid) throws IOException, InvalidParameterException;

    /**
     * Gets all components of the MIN-based IMSI.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_MCC_I,
     *                 NV_IMSI_11_12_I,
     *                 NV_MIN1_I,
     *                 NV_MIN2_I;
     *
     * @return Structure representing the MIN-based IMSI on success.
     * @throws IOException if read operation fails.
     */

    public MinImsi getMinImsi() throws IOException;

    /**
     * Sets all components of the MIN-based IMSI.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_MCC_I,
     *                 NV_IMSI_11_12_I,
     *                 NV_MIN1_I,
     *                 NV_MIN2_I;
     *
     * @param Structure representing the MIN-based IMSI.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */

    public void setMinImsi(MinImsi minImsi) throws IOException, InvalidParameterException;

    /**
     * Gets the 10-digit IMSI Number.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MIN1_I,
     *                 NV_MIN2_I;
     *
     * @return String containing the IMSI number on success.
     * @throws IOException if read operation fails.
     */

    public String getMinImsiNumber() throws IOException;

    /**
     * Sets the 10-digit IMSI Number.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MIN1_I,
     *                 NV_MIN2_I;
     *
     * @param String containing the IMSI number.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */

    public void setMinImsiNumber(String phNumber) throws IOException, InvalidParameterException;

    /**
     * Gets the Mobile Country Code.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_MCC_I;
     *
     * @return String containing the Mobile Country Code (MCC) on success.
     * @throws IOException if read operation fails.
     */
    public String getImsiMcc() throws IOException;

    /**
     * Sets the Mobile Country Code. (NV_IMSI_MCC_I)
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_MCC_I;
     *
     * @param imsiMcc String containing the Mobile Country Code (MCC).
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setImsiMcc(String imsiMcc) throws IOException, InvalidParameterException;

    /**
     * Read 11th and 12th digits of IMSI.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_11_12_I;
     *
     * @return String containing 11th and 12th digit of the IMSI number on
     *         success.
     * @throws IOException if read operation fails.
     */
    public String getImsi11_12() throws IOException;

    /**
     * Set the 11th and 12th digits of IMSI.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_11_12_I;
     *
     * @param imsi11_12 Value to set.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setImsi11_12(String imsi11_12) throws IOException, InvalidParameterException;

    @Deprecated
    /**
     * Gets MIN1 numbers.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MIN1_1,
     *                 NV_DIR_NUMBER_I;
     *
     * @return String array containing hex representation of MIN1[0] and MIN1[1]
     *         numbers on success.
     * @throws IOException if read operation fails.
     */

    public String[] getImsiMin1() throws IOException;

    @Deprecated
    /**
     * Sets MIN1 numbers.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MIN1_1,
     *                 NV_DIR_NUMBER_I;
     *
     * @param String array containing hex representation of MIN1[0] and MIN1[1]
     *        numbers to be set.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setImsiMin1(String[] min1) throws IOException, InvalidParameterException;

    @Deprecated
    /**
     * Gets MIN2 numbers.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MIN1_1,
     *                 NV_DIR_NUMBER_I;
     *
     * @return String array containing hex representation of MIN2[0] and MIN2[1]
     *         numbers on success.
     * @throws IOException if read operation fails.
     */
    public String[] getImsiMin2() throws IOException;

    @Deprecated
    /**
     * Sets MIN2 number.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MIN1_1,
     *                 NV_DIR_NUMBER_I;
     *
     * @param String array containing hex representation of MIN2[0] and MIN2[1]
     *        numbers to be set.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setImsiMin2(String[] min2) throws IOException, InvalidParameterException;

    /**
     * Gets all components of the True IMSI.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_MCC_I (True Mobile Country Code),
     *                 NV_IMSI_T_11_12_I,
     *                 NV_IMSI_T_S1_I,
     *                 NV_IMSI_T_S2_I,
     *                 NV_IMSI_T_ADDR_NUM_I (Number of True IMSI Digits);
     *
     * @return Structure representing the True IMSI on success.
     * @throws IOException if read operation fails.
     */

    public TrueImsi getTrueImsi() throws IOException;

    /**
     * Sets all components of the True IMSI.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_MCC_I (True Mobile Country Code),
     *                 NV_IMSI_T_11_12_I,
     *                 NV_IMSI_T_S1_I,
     *                 NV_IMSI_T_S2_I,
     *                 NV_IMSI_T_ADDR_NUM_I (Number of True IMSI Digits);
     *
     * @param Structure representing the True IMSI.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */

    public void setTrueImsi(TrueImsi trueImsi) throws IOException, InvalidParameterException;

    /**
     * Gets the 10-digit True IMSI Number.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_S1_I,
     *                 NV_IMSI_T_S2_I;
     *
     * @return String containing the True IMSI number on success.
     * @throws IOException if read operation fails.
     */

    public String getTrueImsiNumber() throws IOException;

    /**
     * Sets the 10-digit True IMSI Number.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_S1_I,
     *                 NV_IMSI_T_S2_I;
     *
     * @param String containing the True IMSI number
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */

    public void setTrueImsiNumber(String phNumber) throws IOException, InvalidParameterException;

    /**
     * Gets the true Mobile Country Code.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_MCC_I;
     *
     * @return String containing the MCC on success.
     * @throws IOException if read operation fails.
     */
    public String getTrueImsiMcc() throws IOException;

    /**
     * Sets the true mobile Country Code.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_MCC_I;
     *
     * @param imsiTMcc String containing the MCC to be set.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setTrueImsiMcc(String imsiTMcc) throws IOException, InvalidParameterException;

    /**
     * Gets the 11 and 12th digit of the True IMSI number.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_11_12_I;
     *
     * @return String containing the 11 and 12th digit of the True IMSI number.
     * @throws IOException if read operation fails.
     */
    public String getTrueImsi11_12() throws IOException;

    /**
     * Sets the 11 and 12th digit of the True IMSI number.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_11_12_I;
     *
     * @param imsiT11_12 String containing the 11 and 12th digit of the True
     *            IMSI number.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setTrueImsi11_12(String imsiT11_12) throws IOException, InvalidParameterException;

    /**
     * Gets the number of True IMSI digits.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_ADDR_NUM_I;
     *
     * @return String containing the number of true IMSI digits.
     * @throws IOException if read operation fails.
     */
    public short getTrueImsiAddrNum() throws IOException;

    /**
     * Sets the number of True IMSI digits.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_IMSI_T_ADDR_NUM_I;
     *
     * @param imsiTAddrNum String containing the number of true IMSI digits.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setTrueImsiAddrNum(short imsiTAddrNum) throws IOException,
            InvalidParameterException;

    /**
     * Gets the current CDMA channels.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_PCDMACH_I,
     *                 NV_SCDMACH_I;
     *
     * @return Structure CdmaChannels containing the current CDMA channels on
     *         success.
     * @throws IOException if read operation fails.
     */
    public CdmaChannels getCdmaChannels() throws IOException;

    /**
     * Sets the current CDMA channels.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_PCDMACH_I,
     *                 NV_SCDMACH_I;
     *
     * @param Structure CdmaChannels containing the current CDMA channels.
     * @throws IOException if read operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setCdmaChannels(CdmaChannels cdmaChannels) throws IOException,
            InvalidParameterException;

    /**
     * Gets the primary CDMA channels.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_PCDMACH_I;
     *
     * @return Integer array of size 2 containing the channel numbers for A and B
     *         on success.
     * @throws IOException if read operation fails.
     */
    public int[] getPrimaryCdmaChannels() throws IOException;

    /**
     * Sets the primary CDMA channels.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_PCDMACH_I;
     *
     * @param primaryChannels Integer array of size 2 containing the channel numbers for
     *            A and B.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setPrimaryCdmaChannels(int[] primaryChannels) throws IOException,
            InvalidParameterException;

    /**
     * Gets the secondary CDMA channels.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_SCDMACH_I;
     *
     * @return String array of size 2 containing the channel numbers for A and B
     *             on success.
     * @throws IOException if read operation fails.
     */
    public int[] getSecondaryCdmaChannels() throws IOException;

    /**
     * Sets the secondary CDMA channels.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_SCDMACH_I;
     *
     * @param secondaryChannels Integer array of size 2 containing the channel
     *            numbers for A and B.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setSecondaryCdmaChannels(int[] secondaryChannels) throws IOException,
            InvalidParameterException;

    /**
     * Gets the mobile P_REV settings.
     *
     * QMI Message:    QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MOB_CAI_REV_I (Mobile Call Air Interface Revision);
     *
     * @return On success, the string containing the mobile P_REV settings.
     * @throws IOException if read operation fails.
     */
    public short getMobCaiRev() throws IOException;

    @Deprecated
    /**
     * Sets the mobile P_REV settings.
     *
     * QMI Message:    QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO;
     * NV Items:       NV_MOB_CAI_REV_I (Mobile Call Air Interface Revision);
     *
     * @param mobCaiRev short containing the mobile P_REV settings.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setMobCaiRev(short mobCaiRev) throws IOException, InvalidParameterException;

    /**
     * Gets the RTRE Configuration.
     *
     * QMI Message:    QMI_NAS_GET_RTRE_CONFIG;
     * NV Items:       NV_RTRE_CONFIG_I (RunTime R-UIM Enable Configuration);
     *
     * @return Short representing RTRE config on success.
     * @throws IOException if read operation fails.
     */

    public short getRtreConfig() throws IOException;

    /**
     * Sets the RTRE Configuration.
     *
     * QMI Message:    QMI_NAS_SET_RTRE_CONFIG;
     * NV Items:       NV_RTRE_CONFIG_I (RunTime R-UIM Enable Configuration);
     *
     * @param Short representing RTRE configuration.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameter passed.
     */

    public void setRtreConfig(short rtreCfg) throws IOException, InvalidParameterException;

    // VOICE
    /**
     * Retrieves various configuration parameters that control the modem
     *  behavior related to circuit switched services.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_AUTO_ANSWER_I (Auto Answer Setting),
     *                 NV_AIR_CNT_I (Air Timer),
     *                 NV_ROAM_CNT_I (Roam Timer),
     *                 NV_TTY_I (TTY Setting),
     *                 NV_PREF_VOICE_SO_I (Voice Preferences),
     *                 NV_GSM_AMR_CALL_CONFIG_I (AMR Capability),
     *                 NV_VOICE_PRIV_I (Preferred Privacy Setting);
     *
     * @return Structure VoiceConfig containing voice configuration settings on success.
     * @throws IOException if read operation fails.
     */

    public VoiceConfig getVoiceConfig() throws IOException;

    /**
     * Sets various configuration parameters that control the modem
     *  behavior related to circuit switched services.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_AUTO_ANSWER_I (Auto Answer Setting),
     *                 NV_AIR_CNT_I (Air Timer),
     *                 NV_ROAM_CNT_I (Roam Timer),
     *                 NV_TTY_I (TTY Setting),
     *                 NV_PREF_VOICE_SO_I (Voice Preferences);
     *
     * @param Structure VoiceConfig containing voice configuration settings.
     * @throws IOException if read operation fails.
     * @throws InvalidParameterException if incorrect parameter passed.
     */
    public void setVoiceConfig(VoiceConfig voiceConfig) throws IOException,
            InvalidParameterException;

    /**
     * Retrieves the current Auto-Answer configuration
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_AUTO_ANSWER_I (Auto Answer Setting);
     *
     * @return Structure AutoAnswer containing feature enable/disable status and if
     *         the feature is enabled, the number of rings before Auto-Answer.
     * @throws IOException if read operation fails.
     */
    public AutoAnswer getAutoAnswerStatus() throws IOException;

    /**
     * Sets the current Auto-Answer configuration
     *
     * QMI Message:    QMI_VOICE_SET_CONFIG;
     * NV Items:       NV_AUTO_ANSWER_I (Auto Answer Setting);
     *
     * @param Structure AutoAnswer containing feature enable/disable status and if
     *         the feature is enabled, the number of rings before Auto-Answer.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameter passed.
     */
    public void setAutoAnswerStatus(AutoAnswer autoAnswer) throws IOException,
            InvalidParameterException;

    /**
     * Disable Auto Answer Setting
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_AUTO_ANSWER_I (Auto Answer Setting);
     *
     * @throws IOException if operation fails.
     */
    public void disableAutoAnswer() throws IOException;

    /**
     * Enable Auto-Answer feature
     *
     * QMI Message:    QMI_VOICE_SET_CONFIG;
     * NV Items:       NV_AUTO_ANSWER_I (Auto Answer Setting);
     *
     * @throws IOException if operation fails.
     */
    public void enableAutoAnswer() throws IOException;

    /**
     * Gets the cumulative air time counted.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_AIR_CNT_I;
     *
     * @return Structure containing the namID and the cumulative air time
     *         counted (in minutes) on success.
     * @throws IOException if read operation fails.
     */
    public TimerCount getAirTimerCount() throws IOException;

    /**
     * Sets the cumulative air time counted.
     *
     * QMI Message:    QMI_VOICE_SET_CONFIG;
     * NV Items:       NV_AIR_CNT_I;
     *
     * @param Structure containing the namId and the cumulative air time counted
     *        (in minutes).
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setAirTimerCount(TimerCount airTimerCount) throws IOException, InvalidParameterException;

    /**
     * Gets the cumulative roam time counted.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_ROAM_CNT_I;
     *
     * @return Structure containing the namID and the cumulative roam time
     *         counted (in minutes) on success.
     * @throws IOException if read operation fails.
     */
    public TimerCount getRoamTimerCount() throws IOException;

    /**
     * Sets the cumulative roam time counted.
     *
     * QMI Message:    QMI_VOICE_SET_CONFIG;
     * NV Items:       NV_ROAM_CNT_I;
     *
     * @param Structure containing the namId and the cumulative roam time counted
     *        (in minutes).
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setRoamTimerCount(TimerCount roamTimerCount) throws IOException,
            InvalidParameterException;

    /**
     * Gets the current TTY mode.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_TTY_I (TTY Setting);
     *
     * @return Short containing the current TTY setting.
     * @throws IOException if read operation fails.
     */
    public short getCurrentTtyMode() throws IOException;

    /**
     * Gets the current TTY mode.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_TTY_I (TTY Setting);
     *
     * @param ttyMode Short containing the current TTY setting.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setCurrentTtyMode(short ttyMode) throws IOException, InvalidParameterException;

    /**
     * Gets the Voice preferences.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_PREF_VOICE_SO_I;
     *
     * @return On success, Structure containing the home/roam voice code settings.
     * @throws IOException if read operation fails.
     */
    public PreferredVoiceSo getPreferredVoiceSo() throws IOException;

    /**
     * Sets the Voice preferences.
     *
     * QMI Message:    QMI_VOICE_SET_CONFIG;
     * NV Items:       NV_PREF_VOICE_SO_I;
     *
     * @param Structure containing the home/roam voice code settings.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setPreferredVoiceSo(PreferredVoiceSo preferredVoiceSo) throws IOException,
            InvalidParameterException;

    /**
     * Gets the current AMR configuration
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_GSM_AMR_CALL_CONFIG_I,
     *                 NV_UMTS_AMR_CODEC_PREFERENCE_CONFIG_I;
     *
     * @return On success, Structure AmrStatus containing the AMR config.
     * @throws IOException if read operation fails.
     */
    public AmrStatus getAmrStatus() throws IOException;

    /**
     * Gets the Voice Privacy preference.
     *
     * QMI Message:    QMI_VOICE_GET_CONFIG;
     * NV Items:       NV_VOICE_PRIV_I;
     *
     * @return On success, short containing the Voice Privacy preference.
     * @throws IOException if read operation fails.
     */
    public short getVoicePrivacyPref() throws IOException;

    // DMS
    /**
     * Gets the FTM Mode.
     *
     * QMI Message:    QMI_DMS_GET_FTM_MODE;
     * NV Items:       NV_FTM_MODE_I;
     *
     * @return String containing FTM Mode on success.
     * @throws IOException if read operation fails.
     */
    public String getFtmMode() throws IOException;

    /**
     * Gets the Software Version.
     *
     * QMI Message:    QMI_DMS_GET_SW_VERSION;
     * NV Items:       NV_SW_VERSION_INFO_I;
     *
     * @return String containing Software Version on success.
     * @throws IOException if read operation fails.
     */
    public String getSwVersion() throws IOException;

    /**
     * Updates Service Programming Code.
     *
     * QMI Message:    QMI_DMS_UPDATE_SERVICE_PROGRAMMING_CODE;
     * NV Items:       NV_SEC_CODE_I (Security Code),
     *                 NV_SPC_CHANGE_ENABLED_I;
     *
     * @throws IOException if update operation fails.
     */
    public void updateSpCode() throws IOException;

    /**
     * Gets the Device Serial Numbers.
     *
     *
     * QMI Message:    QMI_DMS_GET_DEVICE_SERIAL_NUMBERS;
     * NV Items:       NV_UE_IMEISV_SVN_I;
     *
     * @return String array containing Device Serial Numbers on success.
     * @throws IOException if read operation fails.
     */
    public String[] getDeviceSerials() throws IOException;

    /**
     * Gets the SPC overwrite setting.
     *
     * QMI Message:    QMI_DMS_GET_SPC_CHANGE_ENABLED;
     * NV Items:       NV_SPC_CHANGE_ENABLED_I;
     *
     * @return On successful read, returns true if permitted and false if forbidden.
     * @throws IOException if read operation fails.
     */
    public Boolean getSpcChangeEnabled() throws IOException;

    /**
     * Sets the SPC overwrite settings.
     *
     * QMI Message:    QMI_DMS_SET_SPC_CHANGE_ENABLED;
     * NV Items:       NV_SPC_CHANGE_ENABLED_I;
     *
     * @param Boolean true if permitted and false if forbidden.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameter passed.
     */
    public void setSpcChangeEnabled(Boolean spcChangeEnabled) throws IOException, InvalidParameterException;

    // WDS
    /**
     * Gets the default Baud Rate.
     *
     * QMI Message:    QMI_WDS_GET_BAUD_RATE;
     * NV Items:       NV_DS_DEFAULT_BAUDRATE_I;
     *
     * @return String containing Baud Rate on success.
     * @throws IOException if read operation fails.
     */

    public String getDefaultBaudRate() throws IOException;

    /**
     * Sets the default Baud Rate.
     *
     * QMI Message:    QMI_WDS_SET_BAUD_RATE;
     * NV Items:       NV_DS_DEFAULT_BAUDRATE_I;
     *
     * @param String containing Baud Rate.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameter passed.
     */
    public void setDefaultBaudRate(String baudRate) throws IOException, InvalidParameterException;

    // Miscellaneous
    /**
     * Gets the Email Gateway.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_BROWSER_MAILTO_EMAIL_I;
     *
     * @return String containing the email gateway on success.
     * @throws IOException if read operation fails.
     */
    public String getEmailGateway() throws IOException;

    /**
     * Sets the Email Gateway.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_BROWSER_MAILTO_EMAIL_I;
     *
     * @param gateway String containing the email gateway.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setEmailGateway(String gateway) throws IOException, InvalidParameterException;

    /**
     * Gets the list of emergency numbers.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_ECC_LIST_I;
     *
     * @return String array containing the list of emergency numbers on success.
     * @throws IOException if read operation fails.
     */
    public String[] getEccList() throws IOException;

    /**
     * Sets the list of emergency numbers.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_ECC_LIST_I;
     *
     * @param ecc_list String array containing the list of emergency numbers
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setEccList(String[] ecc_list) throws IOException, InvalidParameterException;

    /**
     * Gets the security code.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_SEC_CODE_I;
     *
     * @return String containing the security code on success.
     * @throws IOException if read operation fails.
     */
    public String getSecCode() throws IOException;

    /**
     * Sets the security code.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_SEC_CODE_I;
     *
     * @param securityCode String containing the security code.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setSecCode(String securityCode) throws IOException, InvalidParameterException;

    /**
     * Gets the lock code.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_LOCK_CODE_I;
     *
     * @return String containing the lock code on success.
     * @throws IOException if read operation fails.
     */
    public String getLockCode() throws IOException;

    /**
     * Sets the lock code.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_LOCK_CODE_I;
     *
     * @param lockCode String containing the lock code.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setLockCode(String lockCode) throws IOException, InvalidParameterException;

    /**
     * Gets the GPS1 PDE address.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_GPS1_PDE_ADDR_ESS_I;
     *
     * @return String containing the GPS1 PDE address on success.
     * @throws IOException if read operation fails.
     */
    public String getGpsOnePdeAddress() throws IOException;

    /**
     * Sets the GPS1 PDE address.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_GPS1_PDE_ADDR_ESS_I;
     *
     * @param gps1PdeAddress String containing the GPS1 PDE IP address.
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setGpsOnePdeAddress(String gps1PdeAddress) throws IOException,
    InvalidParameterException;

    /**
     * Gets the GPS1 PDE port.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_GPS1_PDE_PORT_I;
     *
     * @return String containing the GPS1 PDE port on success.
     * @throws IOException if read operation fails.
     */
    public String getGpsOnePdePort() throws IOException;

    /**
     * Sets the GPS1 PDE port.
     *
     * QMI Message:    NOT IMPLEMENTED;
     * NV Items:       NV_GPS1_PDE_PORT_I;
     *
     * @param gps1PdePort String containing the GPS1 PDE port
     * @throws IOException if write operation fails.
     * @throws InvalidParameterException if incorrect parameters are passed.
     */
    public void setGpsOnePdePort(String gps1PdePort) throws IOException,InvalidParameterException;

    /**
     * Disposes the qcNvItems object.
     *
     * You are not supposed to use the qcNvItems object after it's been disposed.
     */
    public void dispose();
}
