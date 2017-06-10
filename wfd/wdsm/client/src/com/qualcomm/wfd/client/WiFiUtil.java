/*
 * Copyright (c) 2012 - 2013 QUALCOMM Technologies, Inc.  All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
*/

package com.qualcomm.wfd.client;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import com.qualcomm.wfd.WfdEnums.WFDDeviceType;
import static com.qualcomm.wfd.client.WfdOperationUtil.*;

import android.content.Context;
import android.net.wifi.p2p.WifiP2pWfdInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

public class WiFiUtil {
	private static final String TAG = "Client.WiFiUtil";
	public WifiP2pManager manager;
	public Channel channel;
	private HandlerThread processThread;
	private WifiP2pManager.ActionListener wfdSettingStatuslistener;
	private Handler eventHandler;
	private static String peerIP;
	private static WiFiUtil uniqueInstance = null;

	public static WiFiUtil getInstance(Context context, Handler wfdClientEventHandler) {
		if (uniqueInstance == null) {
			Log.e(TAG, "WifiUtil unique instance is null");
			uniqueInstance = new WiFiUtil(context, wfdClientEventHandler);
		}
		return uniqueInstance;
	}

	private WiFiUtil(Context context, Handler wfdClientEventHandler) {
		processThread = new HandlerThread("WiFiUtil");
		processThread.start();
		manager = (WifiP2pManager) context
				.getSystemService(Context.WIFI_P2P_SERVICE);
		channel = manager.initialize(context, processThread.getLooper(), null);

		eventHandler = wfdClientEventHandler;

		wfdSettingStatuslistener = new WifiP2pManager.ActionListener() {

			@Override
			public void onSuccess() {
				  Log.e(TAG,"WFD Settings Success");
				  Message messageInit = eventHandler.obtainMessage(SET_WFD_FINISHED);
				  messageInit.arg1 = 0;
				  eventHandler.sendMessage(messageInit);
			}

			@Override
			public void onFailure(int reasonCode) {
				  Log.e(TAG,"WFD Settings Failure");
				  Message messageInit = eventHandler.obtainMessage(SET_WFD_FINISHED);
				  messageInit.arg1 = -1;
				  eventHandler.sendMessage(messageInit);
			}
        	};
	}

	public void send_wfd_set(boolean isAvailableForSession, WFDDeviceType type) {

		int wfdDeviceInfo = 0;

		if (type == WFDDeviceType.SOURCE)
			wfdDeviceInfo |= P2pWfdDeviceInfo.DEVICETYPE_SOURCE;
		else if(type == WFDDeviceType.PRIMARY_SINK)
			wfdDeviceInfo |= P2pWfdDeviceInfo.DEVICETYPE_PRIMARYSINK;
		else if(type == WFDDeviceType.SECONDARY_SINK)
			wfdDeviceInfo |= P2pWfdDeviceInfo.DEVICETYPE_SECONDARYSINK;
		else if(type == WFDDeviceType.SOURCE_PRIMARY_SINK)
			wfdDeviceInfo |= P2pWfdDeviceInfo.DEVICETYPE_SOURCE_PRIMARYSINK;

		// Needs to be enabled when service discovery is supported
		//wfdDeviceInfo |= SERVICE_DISCOVERY_SUPPORTED;

		//wfdDeviceInfo |= PREFERRED_CONNECTIVITY_TDLS;

		wfdDeviceInfo |= P2pWfdDeviceInfo.CP_SUPPORTED;

		//wfdDeviceInfo |= TIME_SYNC_SUPPORTED;

		//if(type == WFDDeviceType.PRIMARY_SINK ||
		//type == WFDDeviceType.SECONDARY_SINK ||
		//type == WFDDeviceType.SOURCE_PRIMARY_SINK) {

		//	wfdDeviceInfo |= AUDIO_NOT_SUPPORTED_AT_PSINK;
		//}

		if ( type == WFDDeviceType.SOURCE ||
			type == WFDDeviceType.SOURCE_PRIMARY_SINK ) {
			wfdDeviceInfo |= P2pWfdDeviceInfo.AUDIO_ONLY_SUPPORTED_AT_SOURCE;
		}

		//wfdDeviceInfo |= TDLS_PERSISTENT_GROUP;

		//wfdDeviceInfo |= TDLS_PERSISTENT_GROUP_REINVOKE;

		WifiP2pWfdInfo wfdP2pInfo = new WifiP2pWfdInfo(wfdDeviceInfo,
								P2pWfdDeviceInfo.DEFAULT_SESSION_MGMT_CTRL_PORT,
								P2pWfdDeviceInfo.WIFI_MAX_THROUGHPUT);
		wfdP2pInfo.setWfdEnabled(true);
		wfdP2pInfo.setSessionAvailable(isAvailableForSession);
		if ((type == WFDDeviceType.SOURCE ||
			type == WFDDeviceType.SOURCE_PRIMARY_SINK )) {
				wfdP2pInfo.setCoupledSinkSupportAtSource(false);
		}

		if ( (type == WFDDeviceType.PRIMARY_SINK ||
			type == WFDDeviceType.SECONDARY_SINK ||
			type == WFDDeviceType.SOURCE_PRIMARY_SINK)) {

			wfdP2pInfo.setCoupledSinkSupportAtSink(true);
		}
		manager.setWFDInfo(channel, wfdP2pInfo, new ActionListener() {

			@Override
			public void onSuccess() {
				Log.d(TAG, "Successfully set WFD IE Params");
			}

			@Override
			public void onFailure(int error) {
				Log.d(TAG, "Failed to set WFD IE Params: " + error + ".");
			}

		});

	}

    public static String getPeerIP(String peerMac) {
        Log.d(TAG, "getPeerIP():  peerMac= " + peerMac);

        String ip = null;

        /* Try ARP table lookup */
        BufferedReader br = null;
        try {
            br = new BufferedReader(new FileReader("/proc/net/arp"));
            String line;
            if (br != null) {
                while ((line = br.readLine()) != null) {
                    Log.d(TAG, "line in /proc/net/arp is " + line);
                    String[] splitted = null;
                    if (line != null) {
                        splitted = line.split(" +");
                    }

                    // consider it as a match if 5 out of 6 bytes of the mac
                    // address match
                    // ARP output is in the format
                    // <IP address> <HW type> <Flags> <HW address> <Mask Device>

                    if (splitted != null && splitted.length >= 4) {
                        String[] peerMacBytes = peerMac.split(":");
                        String[] arpMacBytes = splitted[3].split(":");

                        if (arpMacBytes == null) {
                            continue;
                        }

                        int matchCount = 0;
                        for (int i = 0; i < arpMacBytes.length; i++) {
                            if (peerMacBytes[i]
                                    .equalsIgnoreCase(arpMacBytes[i])) {
                                matchCount++;
                            }
                        }

                        if (matchCount >= 5) {
                            ip = splitted[0];
                            // Perfect match!
                            if (matchCount == 6) {
                                // Perfect match!
                                return ip;
                            }
                        }
                    }
                }
            } else {
                Log.e(TAG, "Unable to open /proc/net/arp");
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (br != null)
                    br.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return ip;
    }

    /**
     * @return the local IP if found on p2p interface
     */
    public static String getLocalIp() {
        Enumeration<NetworkInterface> netIntfList, virtualIntfList;
        Enumeration<InetAddress> phyAddresses, virtualAddresses;
        try {
            netIntfList = NetworkInterface.getNetworkInterfaces();
            while (netIntfList.hasMoreElements()) {
                NetworkInterface netIntf = netIntfList.nextElement();
                if (netIntf.isUp() && netIntf.getName().contains("p2p")) {
                    virtualIntfList = netIntf.getSubInterfaces();
                    while (virtualIntfList.hasMoreElements()) {
                        NetworkInterface virtualNetIntf = virtualIntfList
                                .nextElement();
                        virtualAddresses = virtualNetIntf.getInetAddresses();
                        while (virtualAddresses.hasMoreElements()) {
                            InetAddress virtualIPAddress = virtualAddresses
                                    .nextElement();
                            if (virtualIPAddress instanceof Inet4Address) {
                                Log.e(TAG,
                                        "IP address of device on virtual interface "
                                                + netIntf.getName()
                                                + " is "
                                                + virtualIPAddress
                                                        .getHostAddress());
                                return virtualIPAddress.getHostAddress();
                            }
                        }
                    }
                    phyAddresses = netIntf.getInetAddresses();
                    while (phyAddresses.hasMoreElements()) {
                        InetAddress phyIPAddress = phyAddresses.nextElement();
                        if (phyIPAddress instanceof Inet4Address) {
                            Log.e(TAG,
                                    "IP address of device on physical interface "
                                + netIntf.getName() + " is "
                                            + phyIPAddress.getHostAddress());
                            return phyIPAddress.getHostAddress();
                        }
                    }
                }
            }
        } catch (SocketException e) {
            Log.e(TAG, "SocketException");
        }
        return null;
    }
}
