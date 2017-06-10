/******************************************************************************
 * @file    QmiNvItems.java
 * @brief   Implementation of the IServiceProgramming interface functions used
 *          to get/set various NV parameters via QMI messages.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.qcnvitems;

import com.qualcomm.qcnvitems.QmiNvItemTypes.*;
import com.qualcomm.qcrilhook.*;
import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.*;

import android.util.Log;
import java.io.IOException;
import java.security.InvalidParameterException;

public class QmiNvItems implements IQcNvItems {

    private static String LOG_TAG = "SERVICE_PROG";

    private QmiOemHook mQmiOemHook;

    private static final boolean enableVLog = true;

    private static int modemId = 0;

    public QmiNvItems() {
        super();
        vLog("Service Programming instance created.");

        mQmiOemHook = QmiOemHook.getInstance(null); //QmiNvItems does not pass a context
    }

    public void dispose() {
        mQmiOemHook.dispose();
        mQmiOemHook = null;
    }

    public static int getModemId() {
        return modemId;
    }

    public static void setModemId(int id) {
        modemId = id;
    }

    // IO Functions
    private static void vLog(String logString) {
        if (enableVLog) {
            Log.v(LOG_TAG, logString);
        }
    }

    // NAS
    public void updateAkey(String akey) throws IOException, InvalidParameterException {
        vLog("updateAkey()");
        QmiLong o = new QmiLong(akey);
        mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_NAS_UPDATE_AKEY, FieldTypeValues.AKEY_TYPE, o);
    }

    public Threegpp2Info get3gpp2SubscriptionInfo(short namId) throws IOException,
            InvalidParameterException {
        vLog("get3gpp2SubscriptionInfo()");
        Threegpp2Info result = new Threegpp2Info(mQmiOemHook.sendQmiMessage(
                IQcRilHook.QCRILHOOK_NAS_GET_3GPP2_SUBSCRIPTION_INFO, Threegpp2Info.NAM_ID_TYPE,
                new QmiByte(namId)));
        vLog(result.toString());
        return result;
    }

    public Threegpp2Info get3gpp2SubscriptionInfo() throws IOException {
        return get3gpp2SubscriptionInfo(BaseQmiTypes.DEFAULT_NAM);
    }

    public void set3gpp2SubscriptionInfo(Threegpp2Info threegpp2Info, short namId)
            throws IOException, InvalidParameterException {
        vLog("set3gpp2SubscriptionInfo()");

        if (threegpp2Info == null) {
            throw new InvalidParameterException();
        }
        short[] s = threegpp2Info.getTypes();
        BaseQmiItemType[] t = threegpp2Info.getItems();

        short[] types = new short[s.length + 1];
        types[0] = Threegpp2Info.NAM_ID_TYPE;
        System.arraycopy(s, 0, types, 1, s.length);

        BaseQmiItemType[] tlvs = new BaseQmiItemType[t.length + 1];
        tlvs[0] = new QmiByte(namId);
        System.arraycopy(t, 0, tlvs, 1, t.length);

        mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_NAS_SET_3GPP2_SUBSCRIPTION_INFO, types, tlvs);
    }

    public void set3gpp2SubscriptionInfo(Threegpp2Info threegpp2Info) throws IOException,
            InvalidParameterException {
        set3gpp2SubscriptionInfo(threegpp2Info, BaseQmiTypes.DEFAULT_NAM);
    }

    public String getNamName(short namId) throws InvalidParameterException, IOException {
        vLog("getNamName()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getNamName();
    }

    public String getNamName() throws IOException {
        return getNamName(BaseQmiTypes.DEFAULT_NAM);
    }

    public int getAnalogHomeSid() throws IOException {
        // Not implemented by QMI
        return 0;
    }

    public void setAnalogHomeSid(int sid) throws IOException, InvalidParameterException {
        // Not implemented by QMI
    }

    public String getDirectoryNumber(short namId) throws IOException {
        vLog("getDirectoryNumber()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getDirNum();
    }

    public String getDirectoryNumber() throws IOException {
        return getDirectoryNumber(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setDirectoryNumber(String directoryNumber, short namId) throws IOException,
            InvalidParameterException {
        vLog("setDirectoryNumber()");
        if (directoryNumber == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        info.setDirNum(directoryNumber);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setDirectoryNumber(String directoryNumber) throws IOException,
            InvalidParameterException {
        setDirectoryNumber(directoryNumber, BaseQmiTypes.DEFAULT_NAM);
    }

    public SidNid getHomeSidNid(short namId) throws IOException {
        vLog("getSidNid()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getSidNid();
    }

    public SidNid getHomeSidNid() throws IOException {
        return getHomeSidNid(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setHomeSidNid(SidNid sidNid, short namId) throws IOException,
            InvalidParameterException {
        vLog("setSidNid()");
        if (sidNid == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        info.setSidNid(sidNid);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setHomeSidNid(SidNid sidNid) throws IOException, InvalidParameterException {
        setHomeSidNid(sidNid, BaseQmiTypes.DEFAULT_NAM);
    }

    public MinImsi getMinImsi(short namId) throws IOException {
        vLog("getMinImsi()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getMinImsi();
    }

    public MinImsi getMinImsi() throws IOException {
        return getMinImsi(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setMinImsi(MinImsi minImsi, short namId) throws IOException,
            InvalidParameterException {
        vLog("setMinImsi()");
        if (minImsi == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        info.setMinImsi(minImsi);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setMinImsi(MinImsi minImsi) throws IOException, InvalidParameterException {
        setMinImsi(minImsi, BaseQmiTypes.DEFAULT_NAM);
    }

    public String getMinImsiNumber(short namId) throws IOException {
        vLog("getMinImsiNumber()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getMinImsi().getImsiNumber();
    }

    public String getMinImsiNumber() throws IOException {
        return getMinImsiNumber(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setMinImsiNumber(String phNumber, short namId) throws IOException,
            InvalidParameterException {
        vLog("setMinImsiNumber()");
        if (phNumber == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        MinImsi minImsi = info.getMinImsi();
        minImsi.setImsiNumber(phNumber);
        info.setMinImsi(minImsi);
        set3gpp2SubscriptionInfo(info);
    }

    public void setMinImsiNumber(String phNumber) throws IOException, InvalidParameterException {
        setMinImsiNumber(phNumber, BaseQmiTypes.DEFAULT_NAM);
    }

    public String getImsiMcc(short namId) throws IOException {
        vLog("getImsiMcc()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getMinImsi().getMcc();
    }

    public String getImsiMcc() throws IOException {
        return getImsiMcc(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setImsiMcc(String imsiMcc, short namId) throws IOException,
            InvalidParameterException {
        vLog("setImsiMcc()");
        if (imsiMcc == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        MinImsi minImsi = info.getMinImsi();
        minImsi.setMcc(imsiMcc);
        info.setMinImsi(minImsi);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setImsiMcc(String imsiMcc) throws IOException, InvalidParameterException {
        setImsiMcc(imsiMcc, BaseQmiTypes.DEFAULT_NAM);
    }

    public String getImsi11_12(short namId) throws IOException {
        vLog("getImsi11_12()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getMinImsi().getImsi11_12();
    }

    public String getImsi11_12() throws IOException {
        return getImsi11_12(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setImsi11_12(String imsi11_12, short namId) throws IOException,
            InvalidParameterException {
        vLog("setImsi11_12()");
        if (imsi11_12 == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        MinImsi minImsi = info.getMinImsi();
        minImsi.setImsi11_12(imsi11_12);
        info.setMinImsi(minImsi);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setImsi11_12(String imsi11_12) throws IOException, InvalidParameterException {
        setImsi11_12(imsi11_12, BaseQmiTypes.DEFAULT_NAM);
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public String[] getImsiMin1(short namId) throws IOException {
        vLog("getImsiMin1()");
        String dirNum = getDirectoryNumber(namId);
        int min1_0 = MinImsi.phStringToMin1(dirNum);
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        MinImsi minImsi = info.getMinImsi();
        int min1_1 = MinImsi.phStringToMin1(minImsi.getImsiNumber());

        String retVal[] = new String[2];
        retVal[0] = PrimitiveParser.toUnsignedString(min1_0);
        retVal[1] = PrimitiveParser.toUnsignedString(min1_1);
        return retVal;
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public String[] getImsiMin1() throws IOException {
        return getImsiMin1(BaseQmiTypes.DEFAULT_NAM);
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public void setImsiMin1(String[] min1, short namId) throws IOException,
            InvalidParameterException {
        vLog("setImsiMin1()");
        if (min1 == null) {
            throw new InvalidParameterException();
        }
        int min1_0, min1_1;
        try {
            min1_0 = PrimitiveParser.parseUnsignedInt(min1[0]);
            min1_1 = PrimitiveParser.parseUnsignedInt(min1[1]);
        } catch (NumberFormatException e) {
            throw new InvalidParameterException(e.toString());
        }

        String[] min2 = getImsiMin2(namId);
        short min2_0 = PrimitiveParser.parseUnsignedShort(min2[0]);
        short min2_1 = PrimitiveParser.parseUnsignedShort(min2[1]);

        setDirectoryNumber(MinImsi.minToPhString(min1_0, min2_0), namId);
        setMinImsiNumber(MinImsi.minToPhString(min1_1, min2_1), namId);
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public void setImsiMin1(String[] min1) throws IOException, InvalidParameterException {
        setImsiMin1(min1, BaseQmiTypes.DEFAULT_NAM);
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public String[] getImsiMin2(short namId) throws IOException {
        vLog("getImsiMin2()");
        String dirNum = getDirectoryNumber(namId);
        int min2_0 = MinImsi.phStringToMin2(dirNum);
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        MinImsi minImsi = info.getMinImsi();
        int min2_1 = MinImsi.phStringToMin2(minImsi.getImsiNumber());

        String retVal[] = new String[2];
        retVal[0] = PrimitiveParser.toUnsignedString(min2_0);
        retVal[1] = PrimitiveParser.toUnsignedString(min2_1);
        return retVal;
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public String[] getImsiMin2() throws IOException {
        return getImsiMin2(BaseQmiTypes.DEFAULT_NAM);
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public void setImsiMin2(String[] min2, short namId) throws IOException,
            InvalidParameterException {
        vLog("setImsiMin2()");
        if (min2 == null) {
            throw new InvalidParameterException();
        }
        short min2_0, min2_1;
        try {
            min2_0 = PrimitiveParser.parseUnsignedShort(min2[0]);
            min2_1 = PrimitiveParser.parseUnsignedShort(min2[1]);
        } catch (NumberFormatException e) {
            throw new InvalidParameterException(e.toString());
        }

        String[] min1 = getImsiMin1(namId);
        int min1_0 = PrimitiveParser.parseUnsignedInt(min1[0]);
        int min1_1 = PrimitiveParser.parseUnsignedInt(min1[1]);

        setDirectoryNumber(MinImsi.minToPhString(min1_0, min2_0), namId);
        setMinImsiNumber(MinImsi.minToPhString(min1_1, min2_1), namId);
    }

    /*
     * Not a required function. Mirrors NV functionality.
     */
    public void setImsiMin2(String[] min2) throws IOException, InvalidParameterException {
        setImsiMin2(min2, BaseQmiTypes.DEFAULT_NAM);
    }

    public TrueImsi getTrueImsi(short namId) throws IOException {
        vLog("getTrueImsi()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getTrueImsi();
    }

    public TrueImsi getTrueImsi() throws IOException {
        return getTrueImsi(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setTrueImsi(TrueImsi trueImsi, short namId) throws IOException,
            InvalidParameterException {
        vLog("setTrueImsi()");
        if (trueImsi == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        info.setTrueImsi(trueImsi);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setTrueImsi(TrueImsi trueImsi) throws IOException, InvalidParameterException {
        setTrueImsi(trueImsi, BaseQmiTypes.DEFAULT_NAM);
    }

    public String getTrueImsiNumber(short namId) throws IOException {
        vLog("getTrueImsiNumber()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getTrueImsi().getImsiNumber();
    }

    public String getTrueImsiNumber() throws IOException {
        return getTrueImsiNumber(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setTrueImsiNumber(String phNumber, short namId) throws IOException,
            InvalidParameterException {
        vLog("setTrueImsiNumber()");
        if (phNumber == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        MinImsi trueImsi = info.getTrueImsi();
        trueImsi.setImsiNumber(phNumber);
        info.setMinImsi(trueImsi);
        set3gpp2SubscriptionInfo(info);
    }

    public void setTrueImsiNumber(String phNumber) throws IOException, InvalidParameterException {
        setTrueImsiNumber(phNumber, BaseQmiTypes.DEFAULT_NAM);
    }

    public String getTrueImsiMcc(short namId) throws IOException {
        vLog("getTrueImsiMcc()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getTrueImsi().getMcc();
    }

    public String getTrueImsiMcc() throws IOException {
        return getTrueImsiMcc(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setTrueImsiMcc(String imsiTMcc, short namId) throws IOException,
            InvalidParameterException {
        vLog("setTrueImsiMcc()");
        if (imsiTMcc == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        TrueImsi trueImsi = info.getTrueImsi();
        trueImsi.setMcc(imsiTMcc);
        info.setTrueImsi(trueImsi);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setTrueImsiMcc(String imsiTMcc) throws IOException, InvalidParameterException {
        setTrueImsiMcc(imsiTMcc, BaseQmiTypes.DEFAULT_NAM);
    }

    public String getTrueImsi11_12(short namId) throws IOException {
        vLog("getTrueImsi11_12()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getTrueImsi().getImsi11_12();
    }

    public String getTrueImsi11_12() throws IOException {
        return getTrueImsi11_12(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setTrueImsi11_12(String imsiT11_12, short namId) throws IOException,
            InvalidParameterException {
        vLog("setTrueImsi11_12()");
        if (imsiT11_12 == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        TrueImsi trueImsi = info.getTrueImsi();
        trueImsi.setImsi11_12(imsiT11_12);
        info.setTrueImsi(trueImsi);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setTrueImsi11_12(String imsiT11_12) throws IOException, InvalidParameterException {
        setTrueImsi11_12(imsiT11_12, BaseQmiTypes.DEFAULT_NAM);
    }

    public short getTrueImsiAddrNum(short namId) throws IOException {
        vLog("getTrueImsiAddrNum()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getTrueImsi().getImsiAddrNum();
    }

    public short getTrueImsiAddrNum() throws IOException {
        return getTrueImsiAddrNum(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setTrueImsiAddrNum(short imsiTAddrNum, short namId) throws IOException,
            InvalidParameterException {
        vLog("setTrueImsiAddrNum()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        TrueImsi trueImsi = info.getTrueImsi();
        trueImsi.setImsiAddrNum(imsiTAddrNum);
        info.setTrueImsi(trueImsi);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setTrueImsiAddrNum(short imsiTAddrNum) throws IOException,
            InvalidParameterException {
        setTrueImsiAddrNum(imsiTAddrNum, BaseQmiTypes.DEFAULT_NAM);
    }

    public CdmaChannels getCdmaChannels(short namId) throws IOException {
        vLog("getCdmaChannels()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        return info.getCdmaChannels();
    }

    public CdmaChannels getCdmaChannels() throws IOException {
        return getCdmaChannels(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setCdmaChannels(CdmaChannels cdmaChannels, short namId) throws IOException,
            InvalidParameterException {
        vLog("setCdmaChannels()");
        if (cdmaChannels == null) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        info.setCdmaChannels(cdmaChannels);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setCdmaChannels(CdmaChannels cdmaChannels) throws IOException,
            InvalidParameterException {
        setCdmaChannels(cdmaChannels, BaseQmiTypes.DEFAULT_NAM);
    }

    public int[] getPrimaryCdmaChannels(short namId) throws IOException {
        vLog("getPrimaryCdmaChannels()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        CdmaChannels cdmaChannels = info.getCdmaChannels();
        int[] retVal = new int[2];
        retVal[0] = cdmaChannels.getPrimaryChannelA();
        retVal[1] = cdmaChannels.getPrimaryChannelB();
        return retVal;
    }

    public int[] getPrimaryCdmaChannels() throws IOException {
        return getPrimaryCdmaChannels(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setPrimaryCdmaChannels(int[] primaryChannels, short namId) throws IOException,
            InvalidParameterException {
        vLog("setPrimaryCdmaChannels()");
        if (primaryChannels == null || primaryChannels.length != 2) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        CdmaChannels cdmaChannels = info.getCdmaChannels();
        cdmaChannels.setPrimaryChannelA(primaryChannels[0]);
        cdmaChannels.setPrimaryChannelB(primaryChannels[1]);
        info.setCdmaChannels(cdmaChannels);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setPrimaryCdmaChannels(int[] primaryChannels) throws IOException,
            InvalidParameterException {
        setPrimaryCdmaChannels(primaryChannels, BaseQmiTypes.DEFAULT_NAM);
    }

    public int[] getSecondaryCdmaChannels(short namId) throws IOException {
        vLog("getSecondaryCdmaChannels()");
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        CdmaChannels cdmaChannels = info.getCdmaChannels();
        int[] retVal = new int[2];
        retVal[0] = cdmaChannels.getSecondaryChannelA();
        retVal[1] = cdmaChannels.getSecondaryChannelB();
        return retVal;
    }

    public int[] getSecondaryCdmaChannels() throws IOException {
        return getSecondaryCdmaChannels(BaseQmiTypes.DEFAULT_NAM);
    }

    public void setSecondaryCdmaChannels(int[] secondaryChannels, short namId) throws IOException,
            InvalidParameterException {
        vLog("setSecondaryCdmaChannels()");
        if (secondaryChannels == null || secondaryChannels.length != 2) {
            throw new InvalidParameterException();
        }
        Threegpp2Info info = get3gpp2SubscriptionInfo(namId);
        CdmaChannels cdmaChannels = info.getCdmaChannels();
        cdmaChannels.setPrimaryChannelA(secondaryChannels[0]);
        cdmaChannels.setPrimaryChannelB(secondaryChannels[1]);
        info.setCdmaChannels(cdmaChannels);
        set3gpp2SubscriptionInfo(info, namId);
    }

    public void setSecondaryCdmaChannels(int[] secondaryChannels) throws IOException,
            InvalidParameterException {
        setSecondaryCdmaChannels(secondaryChannels, BaseQmiTypes.DEFAULT_NAM);
    }

    public short getMobCaiRev() throws IOException {
        vLog("getMobCaiRev()");

        QmiByte result = new QmiByte(mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_NAS_GET_MOB_CAI_REV));
        vLog(result.toString());

        return result.toShort();
    }

    public void setMobCaiRev(short mobCaiRev) throws IOException, InvalidParameterException {
        vLog("setMobCaiRev()");

        QmiByte o;
        o = new QmiByte(mobCaiRev);
        mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_NAS_SET_MOB_CAI_REV, FieldTypeValues.MOB_CAI_REV_TYPE,
                o);
    }

    public short getRtreConfig() throws IOException {
        vLog("getRtreConfig()");

        QmiByte result = new QmiByte(mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_NAS_GET_RTRE_CONFIG));
        vLog(result.toString());

        return result.toShort();
    }

    public void setRtreConfig(short rtreCfg) throws IOException, InvalidParameterException {
        vLog("setRtreConfig()");

        if (rtreCfg < 0) {
            throw new InvalidParameterException();
        }
        QmiByte o;
        o = new QmiByte(rtreCfg);
        mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_NAS_SET_RTRE_CONFIG, FieldTypeValues.RTRE_CONFIG_TYPE,
                o);
    }

    // VOICE
    public VoiceConfig getVoiceConfig() throws IOException {
        vLog("getVoiceConfig()");

        /*
         * VoiceConfig currently requires a structure indicating which
         * parameters to return. Sending VoiceConfig.generateRequest() retrieves
         * all parameters. This notation differs from other QMI calls.
         */
        VoiceConfig result = new VoiceConfig(mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_VOICE_GET_CONFIG,
                VoiceConfig.getTypes(false), VoiceConfig.generateRequest()));
        vLog(result.toString());

        return result;
    }

    public void setVoiceConfig(VoiceConfig voiceConfig) throws IOException,
            InvalidParameterException {
        vLog("setVoiceConfig()");
        if (voiceConfig == null) {
            throw new InvalidParameterException();
        }
        mQmiOemHook.sendQmiMessage(IQcRilHook.QCRILHOOK_VOICE_SET_CONFIG, voiceConfig.getTypes(), voiceConfig
                .getItems());
    }

    public AutoAnswer getAutoAnswerStatus() throws IOException {
        vLog("getAutoAnswerStatus()");
        VoiceConfig vc = getVoiceConfig();
        return vc.getAutoAnswerStatus();
    }

    public void setAutoAnswerStatus(AutoAnswer autoAnswer) throws IOException,
            InvalidParameterException {
        vLog("setAutoAnswerStatus()");
        if (autoAnswer == null) {
            throw new InvalidParameterException();
        }
        VoiceConfig vc = getVoiceConfig();
        vc.setAutoAnswerStatus(autoAnswer);
        setVoiceConfig(vc);
    }

    public void disableAutoAnswer() throws IOException {
        vLog("disableAutoAnswer");
        VoiceConfig vc = getVoiceConfig();
        vc.setAutoAnswerStatus(new AutoAnswer(false));
        setVoiceConfig(vc);
    }

    public void enableAutoAnswer() throws IOException {
        vLog("enableAutoAnswer");
        VoiceConfig vc = getVoiceConfig();
        vc.setAutoAnswerStatus(new AutoAnswer(true));
        setVoiceConfig(vc);
    }

    public TimerCount getAirTimerCount() throws IOException {
        vLog("getAirTimerCount()");
        VoiceConfig vc = getVoiceConfig();
        return vc.getAirTimerCount();
    }

    public void setAirTimerCount(TimerCount airTimerCount) throws IOException,
            InvalidParameterException {
        vLog("setAirTimerCount()");
        if (airTimerCount == null) {
            throw new InvalidParameterException();
        }
        VoiceConfig vc = getVoiceConfig();
        vc.setAirTimerCount(airTimerCount);
        setVoiceConfig(vc);
    }

    public TimerCount getRoamTimerCount() throws IOException {
        vLog("getRoamTimerCount()");
        VoiceConfig vc = getVoiceConfig();
        return vc.getRoamTimerCount();
    }

    public void setRoamTimerCount(TimerCount roamTimerCount) throws IOException,
            InvalidParameterException {
        vLog("setRoamTimerCount()");
        if (roamTimerCount == null) {
            throw new InvalidParameterException();
        }
        VoiceConfig vc = getVoiceConfig();
        vc.setRoamTimerCount(roamTimerCount);
        setVoiceConfig(vc);
    }

    public short getCurrentTtyMode() throws IOException {
        vLog("getCurrentTtyMode()");
        VoiceConfig vc = getVoiceConfig();
        return vc.getCurrentTtyMode();
    }

    public void setCurrentTtyMode(short ttyMode) throws IOException, InvalidParameterException {
        vLog("setCurrentTtyMode()");
        VoiceConfig vc = getVoiceConfig();
        vc.setCurrentTtyMode(ttyMode);
        setVoiceConfig(vc);
    }

    public PreferredVoiceSo getPreferredVoiceSo() throws IOException {
        vLog("getPreferredVoiceSo()");
        VoiceConfig vc = getVoiceConfig();
        return vc.getPreferredVoiceSo();
    }

    public void setPreferredVoiceSo(PreferredVoiceSo preferredVoiceSo) throws IOException,
            InvalidParameterException {
        vLog("setPreferredVoiceSo()");
        if (preferredVoiceSo == null) {
            throw new InvalidParameterException();
        }
        VoiceConfig vc = getVoiceConfig();
        vc.setPreferredVoiceSo(preferredVoiceSo);
        setVoiceConfig(vc);
    }

    public AmrStatus getAmrStatus() throws IOException {
        vLog("getAmrStatus()");
        VoiceConfig vc = getVoiceConfig();
        return vc.getAmrStatus();
    }

    public short getVoicePrivacyPref() throws IOException {
        vLog("getVoicePrivacyPref()");
        VoiceConfig vc = getVoiceConfig();
        return vc.getVoicePrivacy();
    }

    // DMS
    public String getFtmMode() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public String getSwVersion() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void updateSpCode() throws IOException {
        // TODO Auto-generated method stub
    }

    public String[] getDeviceSerials() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public Boolean getSpcChangeEnabled() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setSpcChangeEnabled(Boolean spcChangeEnabled) throws IOException,
            InvalidParameterException {
        // TODO Auto-generated method stub
    }

    // WDS
    public String getDefaultBaudRate() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setDefaultBaudRate(String baudRate) throws IOException, InvalidParameterException {
        // TODO Auto-generated method stub
    }

    // Miscellaneous
    public String getEmailGateway() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setEmailGateway(String gateway) throws IOException, InvalidParameterException {
        // TODO Auto-generated method stub
    }

    public String[] getEccList() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setEccList(String[] eccList) throws IOException, InvalidParameterException {
        // TODO Auto-generated method stub
    }

    public String getSecCode() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setSecCode(String securityCode) throws IOException, InvalidParameterException {
        // TODO Auto-generated method stub
    }

    public String getLockCode() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setLockCode(String lockCode) throws IOException, InvalidParameterException {
        // TODO Auto-generated method stub
    }

    public String getGpsOnePdeAddress() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setGpsOnePdeAddress(String gps1PdeAddress) throws IOException,
            InvalidParameterException {
        // TODO Auto-generated method stub
    }

    public String getGpsOnePdePort() throws IOException {
        // TODO Auto-generated method stub
        return null;
    }

    public void setGpsOnePdePort(String gps1PdePort) throws IOException, InvalidParameterException {
        // TODO Auto-generated method stub

    }
}
