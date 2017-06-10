/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.dm.vdmc;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import android.content.Context;
import android.util.Log;
import android.os.SystemProperties;
import com.android.dm.DmService;
import android.net.Uri;
import android.database.Cursor;
import android.content.Intent;
import android.text.TextUtils;
import android.provider.Telephony;
import android.content.ContentValues;
import com.android.internal.telephony.TelephonyProperties;
import android.location.LocationManager;
import android.os.Bundle;
import android.content.SharedPreferences;
import android.content.ContentResolver;
import android.provider.Settings;

/**
 * Sample class which demonstrates DM Tree IO handler. Each node requires a
 * different instance of the class.
 */
public class MyTreeIoHandler/* implements NodeIoHandler */{
    private String TAG = Vdmc.DM_TAG + "MyTreeIoHandler: ";
    private static final String OEM_INFO_STR = "Hisense";
    private static final String LANG_INFO_STR = "en-US";
    private static final String DM_VER_INFO_STR = "1.2";
    private static final String FW_VER_INFO_STR = "Android 2.2";
    private static final String HW_VER_INFO_STR = "PXA920";
    private static MyTreeIoHandler mInstance = null;

    private static final int DEVID_IO_HANDLER = 0;
    private static final int FROMFILE_IO_HANDLER = 1;
    private static final int MODEL_IO_HANDLER = 2;
    private static final int MAN_IO_HANDLER = 3;
    private static final int OEM_IO_HANDLER = 4;

    private static final int LANG_IO_HANDLER = 5;
    private static final int DMVERSION_IO_HANDLER = 6;

    private static final int FWVERSION_IO_HANDLER = 7;
    private static final int SWVERSION_IO_HANDLER = 8;
    private static final int HWVERSION_IO_HANDLER = 9;
    private static final int SERVER_ADDR_IO_HANDLER = 10;

    // DM Setting
    private static final int DM_CONN_PROFILE_IO_HANDLER = 11;
    private static final int DM_APN_IO_HANDLER = 12;
    private static final int DM_PROXY_IO_HANDLER = 13;
    private static final int DM_PORT_IO_HANDLER = 14;

    // GPRS-CMNET Setting
    private static final int GPRS_CMNET_APN_IO_HANDLER = 15;
    private static final int GPRS_CMNET_PROXY_IO_HANDLER = 16;
    private static final int GPRS_CMNET_PORT_IO_HANDLER = 17;
    private static final int GPRS_CMNET_USERNAME_IO_HANDLER = 18;
    private static final int GPRS_CMNET_PASSWORD_IO_HANDLER = 19;

    // GPRS-CMWAP Setting
    private static final int GPRS_CMWAP_APN_IO_HANDLER = 20;
    private static final int GPRS_CMWAP_PROXY_IO_HANDLER = 21;
    private static final int GPRS_CMWAP_PORT_IO_HANDLER = 22;
    private static final int GPRS_CMWAP_USERNAME_IO_HANDLER = 23;
    private static final int GPRS_CMWAP_PASSWORD_IO_HANDLER = 24;

    // WAP Setting
    private static final int WAP_CONNPROFILE_IO_HANDLER = 25;
    private static final int WAP_HOMEPAGE_IO_HANDLER = 26;
    private static final int WAP_PROXY_IO_HANDLER = 27;
    private static final int WAP_PORT_IO_HANDLER = 28;
    private static final int WAP_USERNAME_IO_HANDLER = 29;
    private static final int WAP_PASSWORD_IO_HANDLER = 30;

    // MMS Setting
    private static final int MMS_CONNPROFILE_IO_HANDLER = 31;
    private static final int MMS_MMSC_IO_HANDLER = 32;

    // PIM Setting
    private static final int PIM_CONNPROFILE_URI_IO_HANDLER = 33;
    private static final int PIM_SERVER_ADDR_IO_HANDLER = 34;
    private static final int PIM_ADDRESS_BOOK_URI_IO_HANDLER = 35;
    private static final int PIM_CALENDAR_URI_IO_HANDLER = 36;

    // PushMail Setting
    private static final int MAIL_CONNPROFILE_IO_HANDER = 37;
    private static final int MAIL_SEND_SERVER_IO_HANDER = 38;
    private static final int MAIL_SEND_PORT_IO_HANDER = 39;
    private static final int MAIL_SEND_USE_SEC_CON_IO_HANDER = 40;
    private static final int MAIL_RECV_SERVER_IO_HANDER = 41;
    private static final int MAIL_RECV_PORT_IO_HANDER = 42;
    private static final int MAIL_RECV_USE_SEC_CON_IO_HANDER = 43;
    private static final int MAIL_RECV_PROTOCAL_IO_HANDER = 44;

    // Streaming Setting
    private static final int STREAMING_CONNPROFILE_IO_HANDLER = 45;
    private static final int STREAMING_NAME_IO_HANDLER = 46;
    private static final int STREAMING_MAX_UDP_PORT_IO_HANDLER = 47;
    private static final int STREAMING_MIN_UDP_PORT_IO_HANDLER = 48;
    private static final int STREAMING_NET_INFO_IO_HANDLER = 49;

    // AGPS Setting
    private static final int AGPS_CONNPROFILE_IO_HANDLER = 50;
    private static final int AGPS_SERVER_IO_HANDLER = 51;
    private static final int AGPS_SERVER_NAME_IO_HANDLER = 52;
    private static final int AGPS_IAPID_IO_HANDLER = 53;
    private static final int AGPS_PORT_IO_HANDLER = 54;
    private static final int AGPS_PROVIDER_ID_IO_HANDLER = 55;

    // CMCC assisted gps SUPL(Secure User Plane Location) server address
    private static final String ASSISTED_GPS_SUPL_HOST = "assisted_gps_supl_host";
    // CMCC agps SUPL port address
    private static final String ASSISTED_GPS_SUPL_PORT = "assisted_gps_supl_port";
    // location agps position mode,MSB or MSA
    private static final String ASSISTED_GPS_POSITION_MODE = "assisted_gps_position_mode";
    // location agps start mode,cold start or hot start.
    private static final String ASSISTED_GPS_RESET_TYPE = "assisted_gps_reset_type";
    // location agps start network,home or all
    private static final String ASSISTED_GPS_NETWORK = "assisted_gps_network";

    private static final String TEMP_PREFS = "dm_data_prefs";

    /*
     * public enum HandlerType { DEVID_IO_HANDLER, FROMFILE_IO_HANDLER,
     * MODEL_IO_HANDLER, MAN_IO_HANDLER, OEM_IO_HANDLER, LANG_IO_HANDLER,
     * DMVERSION_IO_HANDLER, FWVERSION_IO_HANDLER, SWVERSION_IO_HANDLER,
     * HWVERSION_IO_HANDLER, SERVER_ADDR_IO_HANDLER, //DM Setting
     * DM_CONN_PROFILE_IO_HANDLER, DM_APN_IO_HANDLER, DM_PROXY_IO_HANDLER,
     * DM_PORT_IO_HANDLER, //GPRS-CMNET Setting GPRS_CMNET_APN_IO_HANDLER,
     * GPRS_CMNET_PROXY_IO_HANDLER, GPRS_CMNET_PORT_IO_HANDLER,
     * GPRS_CMNET_USERNAME_IO_HANDLER, GPRS_CMNET_PASSWORD_IO_HANDLER,
     * //GPRS-CMWAP Setting GPRS_CMWAP_APN_IO_HANDLER,
     * GPRS_CMWAP_PROXY_IO_HANDLER, GPRS_CMWAP_PORT_IO_HANDLER,
     * GPRS_CMWAP_USERNAME_IO_HANDLER, GPRS_CMWAP_PASSWORD_IO_HANDLER, //WAP
     * Setting WAP_CONNPROFILE_IO_HANDLER, WAP_HOMEPAGE_IO_HANDLER,
     * WAP_PROXY_IO_HANDLER, WAP_PORT_IO_HANDLER, WAP_USERNAME_IO_HANDLER,
     * WAP_PASSWORD_IO_HANDLER, //MMS Setting MMS_CONNPROFILE_IO_HANDLER,
     * MMS_MMSC_IO_HANDLER, //PIM Setting PIM_CONNPROFILE_URI_IO_HANDLER,
     * PIM_SERVER_ADDR_IO_HANDLER, PIM_ADDRESS_BOOK_URI_IO_HANDLER,
     * PIM_CALENDAR_URI_IO_HANDLER, //PushMail Setting
     * MAIL_CONNPROFILE_IO_HANDER, MAIL_SEND_SERVER_IO_HANDER,
     * MAIL_SEND_PORT_IO_HANDER, MAIL_SEND_USE_SEC_CON_IO_HANDER,
     * MAIL_RECV_SERVER_IO_HANDER, MAIL_RECV_PORT_IO_HANDER,
     * MAIL_RECV_USE_SEC_CON_IO_HANDER, MAIL_RECV_PROTOCAL_IO_HANDER,
     * //Streaming Setting STREAMING_CONNPROFILE_IO_HANDLER,
     * STREAMING_NAME_IO_HANDLER, STREAMING_MAX_UDP_PORT_IO_HANDLER,
     * STREAMING_MIN_UDP_PORT_IO_HANDLER, STREAMING_NET_INFO_IO_HANDLER, //AGPS
     * Setting AGPS_CONNPROFILE_IO_HANDLER, AGPS_SERVER_IO_HANDLER,
     * AGPS_SERVER_NAME_IO_HANDLER, AGPS_IAPID_IO_HANDLER, AGPS_PORT_IO_HANDLER,
     * AGPS_PROVIDER_ID_IO_HANDLER, };
     */

    public MyTreeIoHandler(int t, Context c) {
        mContext = c;
        mHandleType = t;
    }

    public MyTreeIoHandler() {

    }

    public MyTreeIoHandler(Context c) {
        mContext = c;
        mInstance = this;
    }

    public static MyTreeIoHandler getInstance()
    {
        return mInstance;
    }

    /**
     * @return node URI handled by this instance.
     */
    public String getNodeUri() {
        String s = null;
        switch (mHandleType) {
            case DEVID_IO_HANDLER:
                s = "./DevInfo/DevId";
                break;
            case FROMFILE_IO_HANDLER:
                s = "./Ext/IOHandlerTest";
                break;

            case MODEL_IO_HANDLER:
                s = "./DevInfo/Mod";
                break;
            case MAN_IO_HANDLER:
                s = "./DevInfo/Man";
                break;
            case OEM_IO_HANDLER:
                s = "./DevDetail/OEM";
                break;
            case LANG_IO_HANDLER:
                s = "./DevInfo/Lang";
                break;
            case DMVERSION_IO_HANDLER:
                s = "./DevInfo/DmV";
                break;
            case FWVERSION_IO_HANDLER:
                s = "./DevDetail/FwV";
                break;
            case SWVERSION_IO_HANDLER:
                s = "./DevDetail/SwV";
                break;
            case HWVERSION_IO_HANDLER:
                s = "./DevDetail/HwV";
                break;
            case SERVER_ADDR_IO_HANDLER:
                s = "./DMAcc/AndroidAcc/AppAddr/SrvAddr/Addr";
                break;

            // DM setting
            case DM_CONN_PROFILE_IO_HANDLER:
                s = "./Settings/DM/ConnProfile";
                break;
            case DM_APN_IO_HANDLER:
                s = "./Settings/DM/APN";
                break;
            case DM_PROXY_IO_HANDLER:
                s = "./Settings/DM/Proxy";
                break;
            case DM_PORT_IO_HANDLER:
                s = "./Settings/DM/Port";
                break;

            // GPRS-CMNET Setting
            case GPRS_CMNET_APN_IO_HANDLER:
                s = "./Settings/GPRS/CMNet/APN";
                break;
            case GPRS_CMNET_PROXY_IO_HANDLER:
                s = "./Settings/GPRS/CMNet/ProxyAddr";
                break;
            case GPRS_CMNET_PORT_IO_HANDLER:
                s = "./Settings/GPRS/CMNet/ProxyPortNbr";
                break;
            case GPRS_CMNET_USERNAME_IO_HANDLER:
                s = "./Settings/GPRS/CMNet/UserName";
                break;
            case GPRS_CMNET_PASSWORD_IO_HANDLER:
                s = "./Settings/GPRS/CMNet/PassWord";
                break;

            // GPRS-CMWAP Setting
            case GPRS_CMWAP_APN_IO_HANDLER:
                s = "./Settings/GPRS/CMWap/APN";
                break;
            case GPRS_CMWAP_PROXY_IO_HANDLER:
                s = "./Settings/GPRS/CMWap/ProxyAddr";
                break;
            case GPRS_CMWAP_PORT_IO_HANDLER:
                s = "./Settings/GPRS/CMWap/ProxyPortNbr";
                break;
            case GPRS_CMWAP_USERNAME_IO_HANDLER:
                s = "./Settings/GPRS/CMWap/UserName";
                break;
            case GPRS_CMWAP_PASSWORD_IO_HANDLER:
                s = "./Settings/GPRS/CMWap/PassWord";
                break;

            // WAP Setting
            case WAP_CONNPROFILE_IO_HANDLER:
                s = "./Settings/WAP/ConnProfile";
                break;
            case WAP_HOMEPAGE_IO_HANDLER:
                s = "./Settings/WAP/StartPage";
                break;
            case WAP_PROXY_IO_HANDLER:
                s = "./Settings/WAP/ProxyAddr";
                break;
            case WAP_PORT_IO_HANDLER:
                s = "./Settings/WAP/ProxyPortNbr";
                break;
            case WAP_USERNAME_IO_HANDLER:
                s = "./Settings/WAP/UserName";
                break;
            case WAP_PASSWORD_IO_HANDLER:
                s = "./Settings/WAP/PassWord";
                break;

            // MMS Setting
            case MMS_CONNPROFILE_IO_HANDLER:
                s = "./Settings/MMS/ConnProfile";
                break;
            case MMS_MMSC_IO_HANDLER:
                s = "./Settings/MMS/MMSC";
                break;

            // PIM Setting
            case PIM_CONNPROFILE_URI_IO_HANDLER:
                s = "./Settings/PIM/ConnProfile";
                break;
            case PIM_SERVER_ADDR_IO_HANDLER:
                s = "./Settings/PIM/Addr";
                break;
            case PIM_ADDRESS_BOOK_URI_IO_HANDLER:
                s = "./Settings/PIM/AddressBookURI";
                break;
            case PIM_CALENDAR_URI_IO_HANDLER:
                s = "./Settings/PIM/CalendarURI";
                break;

            // PushMail Setting
            case MAIL_CONNPROFILE_IO_HANDER:
                s = "./Settings/PushMail/ConnProfile";
                break;
            case MAIL_SEND_SERVER_IO_HANDER:
                s = "./Settings/PushMail/SendServer";
                break;
            case MAIL_SEND_PORT_IO_HANDER:
                s = "./Settings/PushMail/SendPort";
                break;
            case MAIL_SEND_USE_SEC_CON_IO_HANDER:
                s = "./Settings/PushMail/SendUseSecCon";
                break;
            case MAIL_RECV_SERVER_IO_HANDER:
                s = "./Settings/PushMail/RecvServer";
                break;
            case MAIL_RECV_PORT_IO_HANDER:
                s = "./Settings/PushMail/RecvPort";
                break;
            case MAIL_RECV_USE_SEC_CON_IO_HANDER:
                s = "./Settings/PushMail/RecvUseSecCon";
                break;
            case MAIL_RECV_PROTOCAL_IO_HANDER:
                s = "./Settings/PushMail/RecvPro";
                break;

            // Streaming Setting
            case STREAMING_CONNPROFILE_IO_HANDLER:
                s = "./Settings/Streaming/ConnProfile";
                break;
            case STREAMING_NAME_IO_HANDLER:
                s = "./Settings/Streaming/Name";
                break;
            case STREAMING_MAX_UDP_PORT_IO_HANDLER:
                s = "./Settings/Streaming/MaxUdpPort";
                break;
            case STREAMING_MIN_UDP_PORT_IO_HANDLER:
                s = "./Settings/Streaming/MinUdpPort";
                break;
            case STREAMING_NET_INFO_IO_HANDLER:
                s = "./Settings/Streaming/NetInfo";
                break;

            // AGPS Setting
            case AGPS_CONNPROFILE_IO_HANDLER:
                s = "./Settings/AGPS/ConnProfile";
                break;
            case AGPS_SERVER_IO_HANDLER:
                s = "./Settings/AGPS/AGPSServer";
                break;
            case AGPS_SERVER_NAME_IO_HANDLER:
                s = "./Settings/AGPS/AGPSName";
                break;
            case AGPS_IAPID_IO_HANDLER:
                s = "./Settings/AGPS/IAPID";
                break;
            case AGPS_PORT_IO_HANDLER:
                s = "./Settings/AGPS/AGPSServerPort";
                break;
            case AGPS_PROVIDER_ID_IO_HANDLER:
                s = "./Settings/AGPS/ProviderIP";
                break;
        }
        return s;
    }

    // ///////////////////////////////////
    // NodeIoHandler implementation
    // ///////////////////////////////////

    /*
     * (non-Javadoc)
     * @see com.redbend.vdm.NodeIoHandler#read(int, byte[])
     */
    public int read(int handlerType, int offset, byte[] data) /*
                                                               * throws
                                                               * VdmException
                                                               */{
        int ret = 0;
        offset = 0;
        Log.d(TAG, "read: handlerType = " + handlerType + ", offset = " + offset);
        ByteBuffer buf = null;
        String str = null;
        mHandleType = handlerType;
        switch (mHandleType) {
            case DEVID_IO_HANDLER:
                String imei = "IMEI:" + DmService.getInstance().getImei();
                Log.d(TAG, "read: imei =  " + imei);
                ret = imei.length();
                if (data == null) {
                    Log.d(TAG, "read: data is null!");
                    break;
                }
                buf = ByteBuffer.wrap(data);
                Log.d(TAG, "read: buf = " + buf);
                buf.put(imei.getBytes());
                break;
            case FROMFILE_IO_HANDLER:
                try {
                    FileInputStream is = mContext.openFileInput("TreeExternalNode");
                    ret = is.available();
                    if (data != null) {
                        ret = is.read(data, offset, data.length);
                    }
                    is.close();
                } catch (IOException e) {
                    Log.e("DMC", "TreeHandler: Failed to read from external file");
                    return -1;
                }
                break;

            case MODEL_IO_HANDLER:
                str = DmService.getInstance().getModel();
                Log.d(TAG, "read: model =  " + str);
                ret = readData(str, offset, data);
                break;
            case MAN_IO_HANDLER:
                str = DmService.getInstance().getManufactory();
                Log.d(TAG, "read: manufactory =  " + str);
                ret = readData(str, offset, data);
                break;
            case OEM_IO_HANDLER:
                str = OEM_INFO_STR;
                Log.d(TAG, "read: OEM =  " + str);
                ret = readData(str, offset, data);
                break;
            case LANG_IO_HANDLER:
                str = LANG_INFO_STR;
                Log.d(TAG, "read: OEM =  " + str);
                ret = readData(str, offset, data);
                break;
            case DMVERSION_IO_HANDLER:
                str = DM_VER_INFO_STR;
                Log.d(TAG, "read: DM Version =  " + str);
                ret = readData(str, offset, data);
                break;
            case FWVERSION_IO_HANDLER:
                str = FW_VER_INFO_STR;
                Log.d(TAG, "read: FW Version =  " + str);
                ret = readData(str, offset, data);
                break;
            case SWVERSION_IO_HANDLER:
                str = DmService.getInstance().getSoftwareVersion();
                Log.d(TAG, "read: SW Version =  " + str);
                ret = readData(str, offset, data);
                break;
            case HWVERSION_IO_HANDLER:
                str = HW_VER_INFO_STR;
                Log.d(TAG, "read: HW Version =  " + str);
                ret = readData(str, offset, data);
                break;
            case SERVER_ADDR_IO_HANDLER:
                str = DmService.getInstance().getServerAddr();
                Log.d(TAG, "read: server addr =  " + str);
                ret = readData(str, offset, data);
                break;

            // DM Setting
            case DM_CONN_PROFILE_IO_HANDLER:
            case DM_APN_IO_HANDLER:
            case DM_PROXY_IO_HANDLER:
            case DM_PORT_IO_HANDLER:
                str = readDMParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // GPRS-CMNET Setting
            case GPRS_CMNET_APN_IO_HANDLER:
            case GPRS_CMNET_PROXY_IO_HANDLER:
            case GPRS_CMNET_PORT_IO_HANDLER:
            case GPRS_CMNET_USERNAME_IO_HANDLER:
            case GPRS_CMNET_PASSWORD_IO_HANDLER:
                str = readGprsCmnetParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // GPRS-CMWAP Setting
            case GPRS_CMWAP_APN_IO_HANDLER:
            case GPRS_CMWAP_PROXY_IO_HANDLER:
            case GPRS_CMWAP_PORT_IO_HANDLER:
            case GPRS_CMWAP_USERNAME_IO_HANDLER:
            case GPRS_CMWAP_PASSWORD_IO_HANDLER:
                str = readGprsCmwapParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // WAP Setting
            case WAP_CONNPROFILE_IO_HANDLER:
            case WAP_HOMEPAGE_IO_HANDLER:
            case WAP_PROXY_IO_HANDLER:
            case WAP_PORT_IO_HANDLER:
            case WAP_USERNAME_IO_HANDLER:
            case WAP_PASSWORD_IO_HANDLER:
                str = readWapParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // MMS Setting
            case MMS_CONNPROFILE_IO_HANDLER:
            case MMS_MMSC_IO_HANDLER:
                str = readMMSParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // PIM Setting
            case PIM_CONNPROFILE_URI_IO_HANDLER:
            case PIM_SERVER_ADDR_IO_HANDLER:
            case PIM_ADDRESS_BOOK_URI_IO_HANDLER:
            case PIM_CALENDAR_URI_IO_HANDLER:
                str = readPIMParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // PushMail Setting
            case MAIL_CONNPROFILE_IO_HANDER:
            case MAIL_SEND_SERVER_IO_HANDER:
            case MAIL_SEND_PORT_IO_HANDER:
            case MAIL_SEND_USE_SEC_CON_IO_HANDER:
            case MAIL_RECV_SERVER_IO_HANDER:
            case MAIL_RECV_PORT_IO_HANDER:
            case MAIL_RECV_USE_SEC_CON_IO_HANDER:
            case MAIL_RECV_PROTOCAL_IO_HANDER:
                str = readPushMailParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // Streaming Setting
            case STREAMING_CONNPROFILE_IO_HANDLER:
            case STREAMING_NAME_IO_HANDLER:
            case STREAMING_MAX_UDP_PORT_IO_HANDLER:
            case STREAMING_MIN_UDP_PORT_IO_HANDLER:
            case STREAMING_NET_INFO_IO_HANDLER:
                str = readStreamingParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            // AGPS Setting
            case AGPS_CONNPROFILE_IO_HANDLER:
            case AGPS_SERVER_IO_HANDLER:
            case AGPS_SERVER_NAME_IO_HANDLER:
            case AGPS_IAPID_IO_HANDLER:
            case AGPS_PORT_IO_HANDLER:
            case AGPS_PROVIDER_ID_IO_HANDLER:
                str = readAGPSParam(mHandleType);
                ret = readData(str, offset, data);
                break;

            default:
                break;

        }

        return ret;
    }

    /*
     * (non-Javadoc)
     * @see com.redbend.vdm.NodeIoHandler#write(int, byte[], int)
     */

    // public void write(int offset, byte[] data, int totalSize)/* throws
    // VdmException*/ {
    public void write(int handlerType, int offset, byte[] data, int totalSize)/*
                                                                               * throws
                                                                               * VdmException
                                                                               */{
        Log.d(TAG, "write: handlerType = " + handlerType + ", offset = " + offset);
        String str = null;
        mHandleType = handlerType;
        switch (mHandleType) {

            case DEVID_IO_HANDLER:
                return;

            case FROMFILE_IO_HANDLER:
                try {
                    FileOutputStream f = mContext.openFileOutput("TreeExternalNode",
                            offset == 0 ? Context.MODE_PRIVATE : Context.MODE_APPEND);
                    f.write(data);
                    f.close();
                } catch (IOException e) {
                    Log.e("DMC", "TreeHandler: Failed to write to external file");
                    return;
                }
                break;

            // DM Setting
            case DM_CONN_PROFILE_IO_HANDLER:
            case DM_APN_IO_HANDLER:
            case DM_PROXY_IO_HANDLER:
            case DM_PORT_IO_HANDLER:
                str = new String(data);
                writeDMParam(mHandleType, str);
                break;

            // GPRS-CMNET Setting
            case GPRS_CMNET_APN_IO_HANDLER:
            case GPRS_CMNET_PROXY_IO_HANDLER:
            case GPRS_CMNET_PORT_IO_HANDLER:
            case GPRS_CMNET_USERNAME_IO_HANDLER:
            case GPRS_CMNET_PASSWORD_IO_HANDLER:
                str = new String(data);
                writeGprsCmnetParam(mHandleType, str);
                break;

            // GPRS-CMWAP Setting
            case GPRS_CMWAP_APN_IO_HANDLER:
            case GPRS_CMWAP_PROXY_IO_HANDLER:
            case GPRS_CMWAP_PORT_IO_HANDLER:
            case GPRS_CMWAP_USERNAME_IO_HANDLER:
            case GPRS_CMWAP_PASSWORD_IO_HANDLER:
                str = new String(data);
                writeGprsCmwapParam(mHandleType, str);
                break;

            // WAP Setting
            case WAP_CONNPROFILE_IO_HANDLER:
            case WAP_HOMEPAGE_IO_HANDLER:
            case WAP_PROXY_IO_HANDLER:
            case WAP_PORT_IO_HANDLER:
            case WAP_USERNAME_IO_HANDLER:
            case WAP_PASSWORD_IO_HANDLER:
                str = new String(data);
                writeWapParam(mHandleType, str);
                break;

            // MMS Setting
            case MMS_CONNPROFILE_IO_HANDLER:
            case MMS_MMSC_IO_HANDLER:
                str = new String(data);
                writeMMSParam(mHandleType, str);
                break;

            // PIM Setting
            case PIM_CONNPROFILE_URI_IO_HANDLER:
            case PIM_SERVER_ADDR_IO_HANDLER:
            case PIM_ADDRESS_BOOK_URI_IO_HANDLER:
            case PIM_CALENDAR_URI_IO_HANDLER:
                str = new String(data);
                writePIMParam(mHandleType, str);
                break;

            // PushMail Setting
            case MAIL_CONNPROFILE_IO_HANDER:
            case MAIL_SEND_SERVER_IO_HANDER:
            case MAIL_SEND_PORT_IO_HANDER:
            case MAIL_SEND_USE_SEC_CON_IO_HANDER:
            case MAIL_RECV_SERVER_IO_HANDER:
            case MAIL_RECV_PORT_IO_HANDER:
            case MAIL_RECV_USE_SEC_CON_IO_HANDER:
            case MAIL_RECV_PROTOCAL_IO_HANDER:
                str = new String(data);
                writePushMailParam(mHandleType, str);
                break;

            // Streaming Setting
            case STREAMING_CONNPROFILE_IO_HANDLER:
            case STREAMING_NAME_IO_HANDLER:
            case STREAMING_MAX_UDP_PORT_IO_HANDLER:
            case STREAMING_MIN_UDP_PORT_IO_HANDLER:
            case STREAMING_NET_INFO_IO_HANDLER:
                str = new String(data);
                writeStreamingParam(mHandleType, str);
                break;

            // AGPS Setting
            case AGPS_CONNPROFILE_IO_HANDLER:
            case AGPS_SERVER_IO_HANDLER:
            case AGPS_SERVER_NAME_IO_HANDLER:
            case AGPS_IAPID_IO_HANDLER:
            case AGPS_PORT_IO_HANDLER:
            case AGPS_PROVIDER_ID_IO_HANDLER:
                str = new String(data);
                writeAGPSParam(mHandleType, str);
                break;

            default:
                break;
        }

    }

    public void writenull(int handlerType) { 
        Log.d(TAG, "write null: mHandleType = " + mHandleType);
        String str = "";
        mHandleType = handlerType;
        switch (mHandleType) {

            case DEVID_IO_HANDLER:
                // Changing Device Id is not allowed.
                return;

                // DM Setting
            case DM_CONN_PROFILE_IO_HANDLER:
            case DM_APN_IO_HANDLER:
            case DM_PROXY_IO_HANDLER:
            case DM_PORT_IO_HANDLER:
                writeDMParam(mHandleType, str);
                break;

            // GPRS-CMNET Setting
            case GPRS_CMNET_APN_IO_HANDLER:
            case GPRS_CMNET_PROXY_IO_HANDLER:
            case GPRS_CMNET_PORT_IO_HANDLER:
            case GPRS_CMNET_USERNAME_IO_HANDLER:
            case GPRS_CMNET_PASSWORD_IO_HANDLER:
                writeGprsCmnetParam(mHandleType, str);
                break;

            // GPRS-CMWAP Setting
            case GPRS_CMWAP_APN_IO_HANDLER:
            case GPRS_CMWAP_PROXY_IO_HANDLER:
            case GPRS_CMWAP_PORT_IO_HANDLER:
            case GPRS_CMWAP_USERNAME_IO_HANDLER:
            case GPRS_CMWAP_PASSWORD_IO_HANDLER:
                writeGprsCmwapParam(mHandleType, str);
                break;

            // WAP Setting
            case WAP_CONNPROFILE_IO_HANDLER:
            case WAP_HOMEPAGE_IO_HANDLER:
            case WAP_PROXY_IO_HANDLER:
            case WAP_PORT_IO_HANDLER:
            case WAP_USERNAME_IO_HANDLER:
            case WAP_PASSWORD_IO_HANDLER:
                writeWapParam(mHandleType, str);
                break;

            // MMS Setting
            case MMS_CONNPROFILE_IO_HANDLER:
            case MMS_MMSC_IO_HANDLER:
                writeMMSParam(mHandleType, str);
                break;

            // PIM Setting
            case PIM_CONNPROFILE_URI_IO_HANDLER:
            case PIM_SERVER_ADDR_IO_HANDLER:
            case PIM_ADDRESS_BOOK_URI_IO_HANDLER:
            case PIM_CALENDAR_URI_IO_HANDLER:
                writePIMParam(mHandleType, str);
                break;

            // PushMail Setting
            case MAIL_CONNPROFILE_IO_HANDER:
            case MAIL_SEND_SERVER_IO_HANDER:
            case MAIL_SEND_PORT_IO_HANDER:
            case MAIL_SEND_USE_SEC_CON_IO_HANDER:
            case MAIL_RECV_SERVER_IO_HANDER:
            case MAIL_RECV_PORT_IO_HANDER:
            case MAIL_RECV_USE_SEC_CON_IO_HANDER:
            case MAIL_RECV_PROTOCAL_IO_HANDER:
                writePushMailParam(mHandleType, str);
                break;

            // Streaming Setting
            case STREAMING_CONNPROFILE_IO_HANDLER:
            case STREAMING_NAME_IO_HANDLER:
            case STREAMING_MAX_UDP_PORT_IO_HANDLER:
            case STREAMING_MIN_UDP_PORT_IO_HANDLER:
            case STREAMING_NET_INFO_IO_HANDLER:
                writeStreamingParam(mHandleType, str);
                break;

            // AGPS Setting
            case AGPS_CONNPROFILE_IO_HANDLER:
            case AGPS_SERVER_IO_HANDLER:
            case AGPS_SERVER_NAME_IO_HANDLER:
            case AGPS_IAPID_IO_HANDLER:
            case AGPS_PORT_IO_HANDLER:
            case AGPS_PROVIDER_ID_IO_HANDLER:
                writeAGPSParam(mHandleType, str);
                break;

            default:
                break;
        }

    }

    private int readData(String str, int offset, byte[] data)
    {
        int ret = 0;
        if (str == null)
        {
            Log.d(TAG, "readData: str is null!");
            return ret;
        }
        ret = str.length();
        if (data == null) {
            Log.d(TAG, "readData: data is null!");
            return ret;
        }
        ByteBuffer buf = ByteBuffer.wrap(data);
        buf.put(str.getBytes());
        return ret;
    }

    // read DM param
    private String readDMParam(int type)
    {
        String str = null;

        switch (type)
        {
            case DM_CONN_PROFILE_IO_HANDLER:
                str = DmService.getInstance().getAPN();
                break;
            case DM_APN_IO_HANDLER:
                str = DmService.getInstance().getAPN();
                break;
            case DM_PROXY_IO_HANDLER:
                str = DmService.getInstance().getProxy(mContext);
                break;
            case DM_PORT_IO_HANDLER:
                str = DmService.getInstance().getProxyPort(mContext);
                break;
            default:
                break;
        }
        Log.d(TAG, "readDMParam: type = " + type + ", value = " + str);
        return str;
    }

    // read GPRS-CMNET param
    private String readGprsCmnetParam(int type)
    {
        // add to read APN by type
        final int iType = type;
        String str = chooseByApn(mContext, new IccOperatorChooser() {

            public boolean choose(String apnType, Cursor cursor) {
                if (apnType.contains("default") && apnType.contains("net")
                        && !apnType.contains("wap")) {
                    return true;
                } else {
                    return false;
                }
            }

            public String deal(Cursor cursor) {
                switch (iType) {
                    case GPRS_CMNET_APN_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.APN));
                    case GPRS_CMNET_PROXY_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.PROXY));
                    case GPRS_CMNET_PORT_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.PORT));
                    case GPRS_CMNET_USERNAME_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.USER));
                    case GPRS_CMNET_PASSWORD_IO_HANDLER:
                        return cursor.getString(cursor
                                .getColumnIndexOrThrow(Telephony.Carriers.PASSWORD));
                    default:
                        return null;
                }
            }
        });

        // add end
        Log.d(TAG, "readGprsCmnetParam: type = " + type + ", value = " + str);
        return str;
    }

    // read GPRS-CMWAP param
    private String readGprsCmwapParam(int type)
    {
        final int iType = type;
        String str = chooseByApn(mContext, new IccOperatorChooser() {

            public boolean choose(String apnType, Cursor cursor) {
                if (apnType.contains("default") && apnType.contains("wap")) {
                    return true;
                } else {
                    return false;
                }
            }

            public String deal(Cursor cursor) {
                switch (iType) {
                    case GPRS_CMWAP_APN_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.APN));
                    case GPRS_CMWAP_PROXY_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.PROXY));
                    case GPRS_CMWAP_PORT_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.PORT));
                    case GPRS_CMWAP_USERNAME_IO_HANDLER:
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.USER));
                    case GPRS_CMWAP_PASSWORD_IO_HANDLER:
                        return cursor.getString(cursor
                                .getColumnIndexOrThrow(Telephony.Carriers.PASSWORD));
                    default:
                        return null;
                }
            }
        });

        // add end
        Log.d(TAG, "readGprsCmwapParam: type = " + type + ", value = " + str);
        return str;
    }

    // read WAP param
    private String readWapParam(int type)
    {
        String str = null;
        ContentResolver resolver;
        resolver = mContext.getContentResolver();

        switch (type)
        {
            case WAP_CONNPROFILE_IO_HANDLER:
                // for DM
                str = android.provider.Settings.System.getString(mContext.getContentResolver(),
                        "browser_datalink_type");
                break;
            case WAP_HOMEPAGE_IO_HANDLER:
                Uri table = Uri.parse("content://com.android.browser/homepage");
                String[] projection = {
                    "homepage"
                };
                Cursor cur = null;
                try {
                    cur = mContext.getContentResolver().query(table,
                            projection,
                            null, null, null);
                    if (cur != null && cur.moveToFirst()) {
                        int col = cur.getColumnIndex("HomePage");
                        str = cur.getString(col);
                        Log.d(TAG, "readWapParam: str = " + str);
                    }
                } catch (Exception e) {
                    Log.d(TAG, "readWapParam: exception!");
                } finally {
                    if (cur != null) {
                        cur.close();
                    }
                }
                break;
            case WAP_PROXY_IO_HANDLER:
                break;
            case WAP_PORT_IO_HANDLER:
                break;
            case WAP_USERNAME_IO_HANDLER:
                break;
            case WAP_PASSWORD_IO_HANDLER:
                break;
            default:
                break;
        }
        Log.d(TAG, "readWapParam: type = " + type + ", value = " + str);
        return str;
    }

    // read MMS param
    private String readMMSParam(int type)
    {
        String str = null;

        switch (type)
        {
            case MMS_CONNPROFILE_IO_HANDLER:
                SharedPreferences pre = mContext.getSharedPreferences(TEMP_PREFS,
                        mContext.MODE_WORLD_READABLE);
                str = pre.getString("Dm.mms.conn.config", "CMCC WAP MMS");
                break;
            case MMS_MMSC_IO_HANDLER: {
                str = chooseByApn(mContext, new IccOperatorChooser() {

                    public boolean choose(String apnType, Cursor cursor) {
                        Log.d(TAG, "choose apnType = " + apnType);
                        if (apnType.contains("mms")) {
                            String apn = cursor.getString(cursor
                                    .getColumnIndexOrThrow(Telephony.Carriers.APN));
                            Log.d(TAG, "choose apn = " + apn);
                            // Only want cmcc wap
                            if (apn.contains("cmwap")) {
                                return true;
                            }
                        }
                        return false;
                    }

                    public String deal(Cursor cursor) {
                        return cursor.getString(cursor.getColumnIndexOrThrow(
                                Telephony.Carriers.MMSC));
                    }
                });
            }
                break;
            default:
                break;
        }
        Log.d(TAG, "readMMSParam: type = " + type + ", value = " + str);
        return str;
    }

    // read PIM param
    private String readPIMParam(int type)
    {
        Log.d(TAG, "readPIMParam");
        String str = null;

        switch (type)
        {
            case PIM_CONNPROFILE_URI_IO_HANDLER:
                str = android.provider.Settings.System.getString(mContext.getContentResolver(),
                        "android.intent.action.CONN_CONFIGURE");
                Log.d(TAG, "PIM_CONNPROFILE_URI_IO_HANDLER" + str);
                break;
            case PIM_SERVER_ADDR_IO_HANDLER:
                str = android.provider.Settings.System.getString(mContext.getContentResolver(),
                        "android.intent.action.SERVER_ADDRESS");
                Log.d(TAG, "PIM_SERVER_ADDR_IO_HANDLER" + str);
                break;
            case PIM_ADDRESS_BOOK_URI_IO_HANDLER:
                str = android.provider.Settings.System.getString(mContext.getContentResolver(),
                        "android.intent.action.MY_CONTACT");
                Log.d(TAG, "PIM_ADDRESS_BOOK_URI_IO_HANDLER" + str);
                break;
            case PIM_CALENDAR_URI_IO_HANDLER:
                break;
            default:
                break;
        }
        Log.d(TAG, "readPIMParam: type = " + type + ", value = " + str);
        return str;
    }

    // read PushMail param
    private String readPushMailParam(int type)
    {
        String str = null;
        String sendHost = "218.200.243.234";
        String sendPort = "18025";
        String recvHost = "218.200.243.234";
        String recvPort = "18110";

        switch (type)
        {
            case MAIL_CONNPROFILE_IO_HANDER:
                return "CMWap";
            case MAIL_SEND_SERVER_IO_HANDER:
                str = sendHost;
                break;
            case MAIL_SEND_PORT_IO_HANDER:
                str = sendPort;
                break;
            case MAIL_SEND_USE_SEC_CON_IO_HANDER:
                return "false";
            case MAIL_RECV_SERVER_IO_HANDER:
                str = recvHost;
                break;
            case MAIL_RECV_PORT_IO_HANDER:
                str = recvPort;
                break;
            case MAIL_RECV_USE_SEC_CON_IO_HANDER:
                return "false";
            case MAIL_RECV_PROTOCAL_IO_HANDER:
                return "CMPOP3";
            default:
                break;
        }

        final String[] CONTENT_PROJECTION = new String[] {
                "_id", "protocol", "address", "port", "flags"
        };
        final Uri CONTENT_URI = Uri.parse("content://com.android.pushmail.provider/hostauth");

        Cursor cursor = mContext.getContentResolver().query(CONTENT_URI,
                CONTENT_PROJECTION,
                null, null, null);
        if (cursor == null)
        {
            return str;
        }

        try
        {
            cursor.moveToFirst();
            while (!cursor.isAfterLast())
            {
                int flags = cursor.getInt(4);
                int FLAG_SSL = 1;
                String protocol = cursor.getString(1);
                boolean isEncrypt = (FLAG_SSL == (flags & FLAG_SSL));
                if (!isEncrypt)
                {
                    if (protocol.equalsIgnoreCase("pop3"))
                    {
                        if (type == MAIL_RECV_SERVER_IO_HANDER)
                        {
                            str = cursor.getString(2);
                            break;
                        }
                        else if (type == MAIL_RECV_PORT_IO_HANDER)
                        {
                            str = cursor.getString(3);
                            break;
                        }
                    }
                    else if (protocol.equalsIgnoreCase("smtp"))
                    {
                        if (type == MAIL_SEND_SERVER_IO_HANDER)
                        {
                            str = cursor.getString(2);
                            break;
                        }
                        else if (type == MAIL_SEND_PORT_IO_HANDER)
                        {
                            str = cursor.getString(3);
                            break;
                        }
                    }
                }
                else
                {
                    continue;
                }
                cursor.moveToNext();
            }
        } finally
        {
            cursor.close();
        }
        Log.d(TAG, "readPushMailParam: type = " + type + ", value = " + str);
        return str;
    }

    // read Streaming param
    private String readStreamingParam(int type)
    {
        String str = null;

        switch (type)
        {
            case STREAMING_CONNPROFILE_IO_HANDLER:
                str = android.provider.Settings.System.getString(mContext.getContentResolver(),
                        "apn");
                if (TextUtils.isEmpty(str))
                {
                    str = "";
                }
                else if (str.equals("CMWAP") || str.equals("CMNET"))
                {
                    str = str.toLowerCase();
                }
                break;
            case STREAMING_NAME_IO_HANDLER:
                str = "Streaming";
                break;
            case STREAMING_MAX_UDP_PORT_IO_HANDLER:
                str = android.provider.Settings.System.getString(mContext.getContentResolver(),
                        "streaming_max_udp_port");
                Log.d(TAG, "readStreamingParam: read settings ... max udp port = " + str);
                if (TextUtils.isEmpty(str))
                {
                    str = "65535";
                }
                break;
            case STREAMING_MIN_UDP_PORT_IO_HANDLER:
                str = android.provider.Settings.System.getString(mContext.getContentResolver(),
                        "streaming_min_udp_port");
                Log.d(TAG, "readStreamingParam: read settings ... min udp port = " + str);
                if (TextUtils.isEmpty(str))
                {
                    str = "8192";
                }
                break;
            case STREAMING_NET_INFO_IO_HANDLER:
                SharedPreferences pre = mContext.getSharedPreferences(TEMP_PREFS, 1);
                str = pre.getString("Dm.streaming.net.info", "EGPRS,10,100");
                break;
            default:
                break;
        }
        Log.d(TAG, "readStreamingParam: type = " + type + ", value = " + str);
        return str;
    }

    private String readAGPSParam(int type)
    {
        Log.d(TAG, "++++readAGPSParam++++");
        String str = null;
        String strServ = null;
        String strPort = null;
        LocationManager mLocationManager;
        mLocationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
        Bundle data = new Bundle();
        data.putString("host",Settings.Global.getString(mContext.getContentResolver(),
                ASSISTED_GPS_SUPL_HOST));
        data.putString("port",Settings.Global.getString(mContext.getContentResolver(),
                ASSISTED_GPS_SUPL_PORT));
        data.putString("providerid",Settings.Global.getString(mContext.getContentResolver(),
                ASSISTED_GPS_POSITION_MODE));
        data.putString("network",Settings.Global.getString(mContext.getContentResolver(),
                ASSISTED_GPS_NETWORK));
        data.putString("resettype",Settings.Global.getString(mContext.getContentResolver(),
                ASSISTED_GPS_RESET_TYPE));
        if (data == null) {
            return "";
        }
        switch (type)
        {
            case AGPS_CONNPROFILE_IO_HANDLER:
                str = data.getString("apn");
                break;
            case AGPS_SERVER_IO_HANDLER:
                strServ = data.getString("host");
                Log.d(TAG, "++readAGPSParam++ host=" + strServ);
                strPort = data.getString("port");
                if (strPort != null && !strPort.equals(""))
                {
                    Log.d(TAG, "++readAGPSParam++strPort = " + strPort);
                    str = strServ + ":" + strPort;
                }
                else
                {
                    Log.d(TAG, "++readAGPSParam++ no strPort!");
                    str = strServ;
                }
                break;
            case AGPS_SERVER_NAME_IO_HANDLER:
                str = data.getString("name");
                break;
            case AGPS_IAPID_IO_HANDLER:
                str = data.getString("iapid");
                break;
            case AGPS_PORT_IO_HANDLER:
                str = data.getString("port");
                break;
            case AGPS_PROVIDER_ID_IO_HANDLER:
                str = data.getString("providerid");
                break;
            default:
                break;
        }
        Log.d(TAG, "readAGPSParam: type = " + type + ", value = " + str);
        return str;
    }

    // write DM param
    private void writeDMParam(int type, String str)
    {
        Vdmc.isDmSetting = true;
        switch (type)
        {
            case DM_CONN_PROFILE_IO_HANDLER:
                DmService.getInstance().setAPN(mContext, str);
                break;
            case DM_APN_IO_HANDLER:
                DmService.getInstance().setAPN(mContext, str);
                break;
            case DM_PROXY_IO_HANDLER:
                DmService.getInstance().setProxy(mContext, str);
                break;
            case DM_PORT_IO_HANDLER:
                DmService.getInstance().setProxyPort(mContext, str);
                break;
            default:
                break;
        }
        Log.d(TAG, "writeDMParam: type = " + type + ", value = " + str);
        return;
    }

    // write GPRS-CMNET param
    private void writeGprsCmnetParam(int type, String str)
    {
        final String selection = "name = 'CMNET' and numeric=\""
                + android.os.SystemProperties.get(
                        TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "")
                + "\"";
        ContentValues values = new ContentValues();
        Vdmc.isNetSetting = true;
        switch (type) {
            case GPRS_CMNET_APN_IO_HANDLER:
                Vdmc.tmpnetapn = str;
                break;
            case GPRS_CMNET_PROXY_IO_HANDLER:
                Vdmc.tmpnetproxy = str;
                break;
            case GPRS_CMNET_PORT_IO_HANDLER:
                Vdmc.tmpnetport = str;
                break;
            case GPRS_CMNET_USERNAME_IO_HANDLER:
                Vdmc.tmpnetuser = str;
                break;
            case GPRS_CMNET_PASSWORD_IO_HANDLER:
                Vdmc.tmpnetpwd = str;
                break;
            default:
                break;
        }

        return;
    }

    // write GPRS-CMWAP param
    private void writeGprsCmwapParam(int type, String str)
    {
        final String selection = "name = 'CMWAP' and numeric=\""
                + android.os.SystemProperties.get(
                        TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "")
                + "\"";
        Log.d(TAG, "writeGprsCmwapParam     selection:  = " + selection);
        ContentValues values = new ContentValues();
        Vdmc.isWAPSetting = true;
        switch (type) {
            case GPRS_CMWAP_APN_IO_HANDLER:
                Vdmc.tmpwapapn = str;
                break;
            case GPRS_CMWAP_PROXY_IO_HANDLER:
                Vdmc.tmpwapproxy = str;

                break;
            case GPRS_CMWAP_PORT_IO_HANDLER:
                Vdmc.tmpwapport = str;
                break;
            case GPRS_CMWAP_USERNAME_IO_HANDLER:
                Vdmc.tmpwapuser = str;
                break;
            case GPRS_CMWAP_PASSWORD_IO_HANDLER:
                Vdmc.tmpwappwd = str;
                break;
            default:
                break;
        }

        Log.d(TAG, "writeGprsCmwapParam: type = " + type + ", value = " + str);
        return;
    }

    // write WAP param
    private void writeWapParam(int type, String str)
    {
        ContentResolver resolver;
        resolver = mContext.getContentResolver();

        switch (type)
        {
            case WAP_CONNPROFILE_IO_HANDLER:
                // for DM
                Intent intent = new Intent("android.intent.actioin.BROWSER_WRITE_SETTINGS");
                intent.putExtra("linktype", str);
                mContext.sendBroadcast(intent);
                break;
            case WAP_HOMEPAGE_IO_HANDLER:
                Uri table = Uri.parse("content://com.android.browser/homepage");
                ContentValues values = new ContentValues();
                values.put("homepage", str);
                int c = mContext.getContentResolver().update(table, values, null, null);
                Log.d(TAG, "writeWapParam: write homepage = " + str);
                Log.d(TAG, "writeWapParam: update count = " + c);
                break;
            case WAP_PROXY_IO_HANDLER:
                break;
            case WAP_PORT_IO_HANDLER:
                break;
            case WAP_USERNAME_IO_HANDLER:
                break;
            case WAP_PASSWORD_IO_HANDLER:
                break;
            default:
                break;
        }
        Log.d(TAG, "writeWapParam: type = " + type + ", value = " + str);
        return;
    }

    // write MMS param
    private void writeMMSParam(int type, String str)
    {
        Vdmc.isMmsSetting = true;
        switch (type)
        {
            case MMS_CONNPROFILE_IO_HANDLER:
                SharedPreferences.Editor editor = mContext.getSharedPreferences(TEMP_PREFS, 2)
                        .edit();
                editor.putString("Dm.mms.conn.config", str);
                editor.commit();
                break;
            case MMS_MMSC_IO_HANDLER: {

                Vdmc.tmpmms = str;
            }
                break;
            default:
                break;
        }
        Log.d(TAG, "writeMMSParam: type = " + type + ", value = " + str);
        return;
    }

    // write PIM param
    private void writePIMParam(int type, String str)
    {
        switch (type)
        {
            case PIM_CONNPROFILE_URI_IO_HANDLER: {
                Intent in = new Intent("android.intent.action.DM_SETTING_PIM_PARAMETER");
                in.putExtra("android.intent.action.CONN_CONFIGURE", str);
                mContext.sendBroadcast(in);
                break;
            }
            case PIM_SERVER_ADDR_IO_HANDLER: {
                Intent in = new Intent("android.intent.action.DM_SETTING_PIM_PARAMETER");
                in.putExtra("android.intent.action.SERVER_ADDRESS", str);
                mContext.sendBroadcast(in);
                break;
            }
            case PIM_ADDRESS_BOOK_URI_IO_HANDLER: {
                Intent in = new Intent("android.intent.action.DM_SETTING_PIM_PARAMETER");
                in.putExtra("android.intent.action.MY_CONTACT", str);
                mContext.sendBroadcast(in);
                break;
            }
            case PIM_CALENDAR_URI_IO_HANDLER:
                break;
            default:
                break;
        }
        Log.d(TAG, "readPIMParam: type = " + type + ", value = " + str);
        return;
    }

    // write PushMail param
    private void writePushMailParam(int type, String str)
    {
        int MAIL_CONNPROFILE_IO_HANDER_TYPE = 1;
        int MAIL_SEND_SERVER_IO_HANDER_TYPE = 2;
        int MAIL_SEND_PORT_IO_HANDER_TYPE = 3;
        int MAIL_SEND_USE_SEC_CON_IO_HANDER_TYPE = 4;
        int MAIL_RECV_SERVER_IO_HANDER_TYPE = 5;
        int MAIL_RECV_PORT_IO_HANDER_TYPE = 6;
        int MAIL_RECV_USE_SEC_CON_IO_HANDER_TYPE = 7;
        int MAIL_RECV_PROTOCAL_IO_HANDER_TYPE = 8;
        int extraType = 0;
        switch (type)
        {
            case MAIL_CONNPROFILE_IO_HANDER:
                extraType = MAIL_CONNPROFILE_IO_HANDER_TYPE;
                break;
            case MAIL_SEND_SERVER_IO_HANDER:
                extraType = MAIL_SEND_SERVER_IO_HANDER_TYPE;
                break;
            case MAIL_SEND_PORT_IO_HANDER:
                extraType = MAIL_SEND_PORT_IO_HANDER_TYPE;
                break;
            case MAIL_SEND_USE_SEC_CON_IO_HANDER:
                extraType = MAIL_SEND_USE_SEC_CON_IO_HANDER_TYPE;
                break;
            case MAIL_RECV_SERVER_IO_HANDER:
                extraType = MAIL_RECV_SERVER_IO_HANDER_TYPE;
                break;
            case MAIL_RECV_PORT_IO_HANDER:
                extraType = MAIL_RECV_PORT_IO_HANDER_TYPE;
                break;
            case MAIL_RECV_USE_SEC_CON_IO_HANDER:
                extraType = MAIL_RECV_USE_SEC_CON_IO_HANDER_TYPE;
                break;
            case MAIL_RECV_PROTOCAL_IO_HANDER:
                extraType = MAIL_RECV_PROTOCAL_IO_HANDER_TYPE;
                break;
            default:
                break;
        }
        Intent i = new Intent("pushmail.action.WRITE_SETTINGS");
        i.putExtra("type", extraType);
        i.putExtra("value", str);
        mContext.sendBroadcast(i);
        Log.d(TAG, "writePushMailParam: type = " + type + ", value = " + str);
        return;
    }

    // write Streaming param
    private void writeStreamingParam(int type, String str)
    {
        int STREAMING_CONNPROFILE_IO_HANDLER_TYPE = 1;
        int STREAMING_NAME_IO_HANDLER_TYPE = 2;
        int STREAMING_MAX_UDP_PORT_IO_HANDLER_TYPE = 3;
        int STREAMING_MIN_UDP_PORT_IO_HANDLER_TYPE = 4;
        int STREAMING_NET_INFO_IO_HANDLER_TYPE = 5;
        int extraType = 0;
        switch (type)
        {
            case STREAMING_CONNPROFILE_IO_HANDLER:
                extraType = STREAMING_CONNPROFILE_IO_HANDLER_TYPE;
                if (str.equals("cmwap") || str.equals("cmnet"))
                    str = str.toUpperCase();
                break;
            case STREAMING_NAME_IO_HANDLER:
                extraType = STREAMING_NAME_IO_HANDLER_TYPE;
                break;
            case STREAMING_MAX_UDP_PORT_IO_HANDLER:
                extraType = STREAMING_MAX_UDP_PORT_IO_HANDLER_TYPE;
                break;
            case STREAMING_MIN_UDP_PORT_IO_HANDLER:
                extraType = STREAMING_MIN_UDP_PORT_IO_HANDLER_TYPE;
                break;
            case STREAMING_NET_INFO_IO_HANDLER:
                extraType = STREAMING_NET_INFO_IO_HANDLER_TYPE;
                SharedPreferences.Editor editor = mContext.getSharedPreferences(TEMP_PREFS, 2)
                        .edit();
                editor.putString("Dm.streaming.net.info", str);
                editor.commit();
                break;
            default:
                break;
        }
        Intent i = new Intent("streaming.action.WRITE_SETTINGS");
        i.putExtra("type", extraType);
        i.putExtra("value", str);
        mContext.sendBroadcast(i);
        Log.d(TAG, "writeStreamingParam: type = " + type + ", value = " + str);
        return;
    }

    // write AGPS param
    private void writeAGPSParam(int type, String str)
    {
        // add syncDeviceManagementInfo to store settings
        Log.d(TAG, "writeAGPSParam : type = " + type + ", value = " + str);
        LocationManager mLocationManager;
        mLocationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
        Bundle data = new Bundle();
        switch (type)
        {
            case AGPS_CONNPROFILE_IO_HANDLER:
                data.putString("apn", str);
                break;
            case AGPS_SERVER_IO_HANDLER: {
                String[] tmpStr = str.split(":");
                if (tmpStr.length >= 2)
                {
                    data.putString("host", tmpStr[0]);
                    data.putString("port", tmpStr[1]);
                }
                else
                {
                    data.putString("host", str);
                    data.putString("port","");
                }
            }
                break;
            case AGPS_SERVER_NAME_IO_HANDLER:
                data.putString("name", str);
                break;
            case AGPS_IAPID_IO_HANDLER:
                data.putString("iapid", str);
                break;
            case AGPS_PORT_IO_HANDLER:
                data.putString("port", str);
                break;
            case AGPS_PROVIDER_ID_IO_HANDLER:
                data.putString("providerid", str);
                break;
            default:
                break;
        }
        LocationManager objLocManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
        boolean bRet = objLocManager.sendExtraCommand(LocationManager.GPS_PROVIDER,
                "agps_parms_changed", data);
        Log.d(TAG, "writeAGPSParam: type = " + type + ", value = " + str);
        return;
    }

    private Context mContext;
    private int mHandleType;

    private class IccOperatorChooser {

        public boolean choose(String apnType, Cursor cursor) {
            return false;
        }

        public String deal(Cursor cursor) {
            return null;
        }
    }

    public String chooseByApn(Context context, IccOperatorChooser chooser) {
        String str = null;
        String numeric = android.os.SystemProperties.get(
                TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "");
        String[] operArr = numeric != null ? numeric.split(",") : null;
        //get the registed subId.
        int slotId = DmService.getInstance().getSlotId();
        Log.d(TAG, "registed slotId = " + slotId);
        if (operArr != null && slotId >= 0 && slotId < operArr.length) {
            String regOper = operArr[slotId];
            String selection = "numeric=\"" + regOper + "\"";
            Cursor cursor = context.getContentResolver().query(
                    Telephony.Carriers.CONTENT_URI,
                    null, selection, null, null);
            if (cursor != null) {
                cursor.moveToFirst();
                while (!cursor.isAfterLast()) {
                    String apntype = cursor.getString(cursor
                            .getColumnIndexOrThrow(Telephony.Carriers.TYPE));
                    if (chooser.choose(apntype, cursor)) {
                        str = chooser.deal(cursor);
                        break;
                    }
                    cursor.moveToNext();
                }
                cursor.close();
            }
        }
        return str;
    }

}
