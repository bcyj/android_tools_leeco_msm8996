/*
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.qrd.engineeringmode;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.content.res.Resources;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.text.TextUtils;
import android.os.Build;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.cdma.CDMAPhone;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccUtils;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;

/**
 * Display the following information # Battery Strength : TODO # Uptime # Awake
 * Time # XMPP/buzz/tickle status : TODO
 */
public class DeviceInfo extends PreferenceActivity {

    private static final String KEY_DEVICE_MODEL = "device_model";
    private static final String KEY_HW_VERSION = "hardware_version";
    private static final String KEY_MEID = "meid_number";
    private static final String KEY_ESN = "esn_number";
    private static final String KEY_BASEBAND_VERSION = "baseband_version";
    private static final String KEY_PRL_VERSION = "prl_version";
    private static final String KEY_ANDROID_VERSION = "android_version";
    private static final String KEY_SOFTWARE_VERSION = "software_version";
    private static final String KEY_PHONE_NUMBER = "number";
    private static final String KEY_IMSI = "imsi";
    private static final String KEY_UIM_ID = "uim_id";
    private static final String KEY_SID_NUMBER = "sid_number";
    private static final String KEY_NID_NUMBER = "nid_number";
    private static final String KEY_EPRL_NUMBER = "eprl_number";
    private static final String KEY_MSPL_NUMBER = "mspl_number";
    private static final String KEY_MLPL_NUMBER = "mlpl_number";
    private static final String KEY_ICCID_NUMBER = "iccid_number";
    private static final String KEY_IMEI_NUMBER = "imei_number";

    private Phone mPhone = null;
    private static String mUnknown = null;

    private static final String FILENAME_MSV = "/sys/board_properties/soc/msv";

    private ObtainIdHandler mHandler = new ObtainIdHandler();

    private class ObtainIdHandler extends Handler {
        private static final int MESSAGE_GET_EF_RUIM = 1;
        private static final int MESSAGE_GET_EF_PRL = 2;
        private static final int MESSAGE_GET_EF_MSPL = 3;
        private static final int MESSAGE_GET_EF_MLPL = 4;
        //RUIM ID is 8 bytes data
        private static final int NUM_BYTES_RUIM_ID = 8;
        //True PRL ID is x bytes data
        private static final int NUM_BYTES_PRL_ID = 4;
        //MSPL ID is x bytes data
        private static final int NUM_BYTES_MSPL_ID =5;
        //MLPL ID is 8 bytes data
        private static final int NUM_BYTES_MLPL_ID = 5;

        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MESSAGE_GET_EF_RUIM:
                handleGetEFRuim(msg);
                break;
            case MESSAGE_GET_EF_PRL:
                handleGetEFPrl(msg);
                break;
            case MESSAGE_GET_EF_MSPL:
                handleGetEFMspl(msg);
                break;
            case MESSAGE_GET_EF_MLPL:
                handleGetEFMlpl(msg);
                break;
            }
        }

        private void handleGetEFRuim(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            byte[] data = (byte[])ar.result;
            if (ar.exception == null) {
                //for RUIM ID data, the first byte represent the num bytes of valid data. From
                //the second byte to num+1 byte, it is valid RUIM ID data. And the second
                //byte is the lowest-order byte, the num+1 byte is highest-order
                int numOfBytes = data[0];
                if (numOfBytes < NUM_BYTES_RUIM_ID) {
                    byte[] decodeData = new byte[numOfBytes];
                    for (int i = 0; i < numOfBytes; i++) {
                        decodeData[i] = data[numOfBytes - i];
                    }
                    String ruimid = IccUtils.bytesToHexString(decodeData);
                    setSummaryText(KEY_UIM_ID, ruimid);
                }
            }
        }

        private void handleGetEFPrl(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            byte[] data = (byte[]) ar.result;
            if (ar.exception == null) {
                if (data.length > NUM_BYTES_PRL_ID - 1) {
                    int prlId = ((data[2] & 0xFF) << 8) | (data[3] & 0xFF);
                    setSummaryText(KEY_PRL_VERSION, String.valueOf(prlId));
                }
            }
        }

        private void handleGetEFMspl(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            byte[] data = (byte[])ar.result;
            if (ar.exception == null) {
                if (data.length > NUM_BYTES_MSPL_ID -1) {
                    int msplId = ((data[3] & 0xFF) << 8 ) | (data[4] & 0xFF);
                    setSummaryText(KEY_MSPL_NUMBER, String.valueOf(msplId));
                }
            }
        }

        private void handleGetEFMlpl(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            byte[] data = (byte[])ar.result;
            if (ar.exception == null) {
                if (data.length > NUM_BYTES_MLPL_ID - 1) {
                    int mlplId = ((data[3] & 0xFF) << 8) | (data[4] & 0xFF);
                    setSummaryText(KEY_MLPL_NUMBER, String.valueOf(mlplId));
                }
            }
        }

        public void GetRuimId() {
            UiccController controller = UiccController.getInstance();
            if (controller != null) {
                int numPhone = TelephonyManager.getDefault().getPhoneCount();
                for (int i = 0; i < numPhone; i++) {
                    Phone phone = PhoneFactory.getPhone(i);
                    if (phone.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA) {
                        IccFileHandler fh = controller.getIccFileHandler(i, UiccController.APP_FAM_3GPP2);
                        if (fh != null) {
                            fh.loadEFTransparent(IccConstants.EF_RUIM_ID, NUM_BYTES_RUIM_ID,
                                    mHandler.obtainMessage(ObtainIdHandler.MESSAGE_GET_EF_RUIM));
                        }
                        break;
                    }
                }
            }
        }

        public void getPrlVersion() {
            UiccController controller = UiccController.getInstance();
            if (controller != null) {
                int cdmaPhoneSub = getCdmaPhoneSub();
                if (cdmaPhoneSub != Integer.MAX_VALUE) {
                    IccFileHandler fh = controller.getIccFileHandler(cdmaPhoneSub, UiccController.APP_FAM_3GPP2);
                    if (fh != null) {
                        fh.loadEFTransparent(IccConstants.EF_CSIM_PRL, NUM_BYTES_PRL_ID,
                            mHandler.obtainMessage(ObtainIdHandler.MESSAGE_GET_EF_PRL));
                    }
                }
            }
        }

        public void getMsplVersion() {
            UiccController controller = UiccController.getInstance();
            if (controller != null) {
                int cdmaPhoneSub = getCdmaPhoneSub();
                if (cdmaPhoneSub != Integer.MAX_VALUE) {
                    IccFileHandler fh = controller.getIccFileHandler(cdmaPhoneSub, UiccController.APP_FAM_3GPP2);
                    if (fh != null) {
                        fh.loadEFTransparent(IccConstants.EF_CSIM_MSPL, NUM_BYTES_MSPL_ID,
                            mHandler.obtainMessage(ObtainIdHandler.MESSAGE_GET_EF_MSPL));
                    }
                }
            }
        }

        public void getMlplVersion() {
            UiccController controller = UiccController.getInstance();
            if (controller != null) {
                int cdmaPhoneSub = getCdmaPhoneSub();
                if (cdmaPhoneSub != Integer.MAX_VALUE) {
                    IccFileHandler fh = controller.getIccFileHandler(cdmaPhoneSub, UiccController.APP_FAM_3GPP2);
                    if (fh != null) {
                        fh.loadEFTransparent(IccConstants.EF_CSIM_MLPL, NUM_BYTES_MLPL_ID,
                            mHandler.obtainMessage(ObtainIdHandler.MESSAGE_GET_EF_MLPL));
                    }
                }
            }
        }

        public int getCdmaPhoneSub() {
            int numPhone = TelephonyManager.getDefault().getPhoneCount();
            for (int i=0; i<numPhone; i++) {
                Phone phone = PhoneFactory.getPhone(i);
                if (phone.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA) {
                    return i;
                }
            }
            return Integer.MAX_VALUE;
        }
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.device_info);

        int numPhone = TelephonyManager.getDefault().getPhoneCount();
        for (int i = 0; i < numPhone; i++) {
            Phone phone = PhoneFactory.getPhone(i);
            if (phone.getPhoneType() == TelephonyManager.PHONE_TYPE_CDMA) {
                mPhone = phone;
                break;
            }
        }

        if (mUnknown == null) {
            mUnknown = getResources().getString(R.string.device_info_unknown);
        }

        setPreferenceSummary();
    }

    private void setPreferenceSummary() {
        // get device model
        setSummaryText(KEY_DEVICE_MODEL, Build.MODEL + getMsvSuffix());

        // get hardware
        String hwVersion = SystemProperties.get("ro.hw_version", mUnknown);
        if (TextUtils.isEmpty(hwVersion) || hwVersion.equals(mUnknown)) {
            hwVersion = "PVT2.0";
        }
        setSummaryText(KEY_HW_VERSION, hwVersion);

        // get baseband version
        String basebandVersion = TelephonyManager.getTelephonyProperty(
                PhoneFactory.getDefaultSubscription(), "gsm.version.baseband", null);
        setSummaryText(KEY_BASEBAND_VERSION, basebandVersion);

        // get android version
        setSummaryText(KEY_ANDROID_VERSION, Build.VERSION.RELEASE);

        // get software version
        setSummaryText(KEY_SOFTWARE_VERSION, Build.DISPLAY);

        // get my phone number
        if (mPhone != null) {
            // get MEID
            setSummaryText(KEY_MEID, mPhone.getMeid());

            // get ESN
            setSummaryText(KEY_ESN, mPhone.getEsn());

            // get PRL version
            mHandler.getPrlVersion();

            // get IMSI
            String imsi = mPhone.getSubscriberId();
            if (imsi == null || imsi.length() == 0) {
                IccCard card = mPhone.getIccCard();
                if (card != null) {
                    IccRecords record = card.getIccRecords();
                    if (record != null) {
                        imsi = record.getIMSI();
                    }
                }
            }
            setSummaryText(KEY_IMSI, imsi);

            // get UIM ID
            mHandler.GetRuimId();

            // get EPrl number
            setSummaryText(KEY_EPRL_NUMBER, mPhone.getCdmaPrlVersion());

            // get Mspl number
            mHandler.getMsplVersion();

            // get Mlpl number
            mHandler.getMlplVersion();

            // get NID and SID
            CdmaCellLocation cellLocation = (CdmaCellLocation) (mPhone
                    .getCellLocation());
            if (cellLocation != null) {
                setSummaryText(KEY_SID_NUMBER, cellLocation.getSystemId() + "");
                setSummaryText(KEY_NID_NUMBER, cellLocation.getNetworkId() + "");
            }

            // get iccid
            setSummaryText(KEY_ICCID_NUMBER, mPhone.getIccSerialNumber());

            //get imei
            setSummaryText(KEY_IMEI_NUMBER, mPhone.getImei());
        }

    }

    private void setSummaryText(String preferenceKey, String value) {
        Preference preference = findPreference(preferenceKey);
        if (preference == null)
            return;

        if (TextUtils.isEmpty(value))
            preference.setSummary(mUnknown);
        else
            preference.setSummary(value);
    }

    /**
     * Returns " (ENGINEERING)" if the msv file has a zero value, else returns
     * "".
     *
     * @return a string to append to the model number description.
     */
    private static String getMsvSuffix() {
        // Production devices should have a non-zero value. If we can't read it,
        // assume it's a
        // production device so that we don't accidentally show that it's an
        // ENGINEERING device.
        try {
            String msv = readLine(FILENAME_MSV);
            // Parse as a hex number. If it evaluates to a zero, then it's an
            // engineering build.
            if (Long.parseLong(msv, 16) == 0) {
                return " (ENGINEERING)";
            }
        } catch (IOException ioe) {
            // Fail quietly, as the file may not exist on some devices.
        } catch (NumberFormatException nfe) {
            // Fail quietly, returning empty string should be sufficient
        }
        return "";
    }

    /**
     * Reads a line from the specified file.
     *
     * @param filename
     *            the file to read from
     * @return the first line, if any.
     * @throws IOException
     *             if the file couldn't be read
     */
    private static String readLine(String filename) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(filename),
                256);
        try {
            return reader.readLine();
        } finally {
            reader.close();
        }
    }

}
