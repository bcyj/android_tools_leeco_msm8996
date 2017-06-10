/*************************************************************************
        Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
                   Qualcomm Technologies Proprietary and Confidential.
 **************************************************************************/

package com.android.iperfapp;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;

public class IPerfUtils {

    private static final String IPERF_FILE_STRING = "/data/data/com.android.iperfapp/files";
    private static final String IPERF_CONFIG_STRING = "/data/data/com.android.iperfapp/"
                                                    + "config/config.txt";

    public String getLocalIpAddress() {
        try {
            for (Enumeration<NetworkInterface> ni = NetworkInterface.getNetworkInterfaces();
                 ni.hasMoreElements();) {
                NetworkInterface nif = ni.nextElement();
                for (Enumeration<InetAddress> eipa = nif.getInetAddresses(); 
                    eipa.hasMoreElements();) {
                    InetAddress inetAddress = eipa.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        return inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException ex) {
        }
        return null;
    }

    public void recursiveTraversal(File fileObject) {
        if (fileObject.isDirectory()) {
            File allFiles[] = fileObject.listFiles();
            for (File aFile : allFiles) {
                recursiveTraversal(aFile);
            }
        } else if (fileObject.isFile()) {
            if (fileObject.exists())
                fileObject.delete();
        }
    }

    public void cleanUpMyDepot() {
        File file = new File(IPERF_FILE_STRING);
        if (!file.isDirectory())
            file.mkdir();
        else {
            if (file.exists())
                recursiveTraversal(file);
        }
    }

    public ArrayList getChoices() {
        ArrayList al = new ArrayList();
        File file = new File(IPERF_CONFIG_STRING);
        if (file.exists()) {
            al = readChoicesFile(file);
        }
        return al;
    }

    private ArrayList readChoicesFile(File file) {
        ArrayList<String> choices = new ArrayList<String>();
        String temp = null;
        try {
            FileInputStream fs = new FileInputStream(file);
            BufferedReader buf = new BufferedReader(new InputStreamReader(fs));
            while (buf != null) {
                temp = buf.readLine().trim();
                if (temp != null)
                    choices.add(temp);
            }

        } catch (Exception e) {

        }
        return choices;
    }
}
