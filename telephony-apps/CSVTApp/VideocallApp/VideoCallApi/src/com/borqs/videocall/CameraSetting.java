/* BORQS Software Solutions Pvt Ltd. CONFIDENTIAL
 * Copyright (c) 2012 All rights reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by BORQS Software
 * Solutions Pvt Ltd. No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without BORQS Software
 * Solutions Pvt Ltd. prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by BORQS Software Solutions Pvt Ltd. in writing.
 *
 */
package com.borqs.videocall;

import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.ArrayList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;

import android.util.Log;

class CameraSetting {

    private final static String TAG = "VTCameraSetting";

    private final static String SettingRootName = "CameraConfig";
    private final static String VTNodeName = "VT";
    private final static String MainNodeName = "CameraMain";
    private final static String SecondNodeName = "CameraSecondary";

    private final static String ContrastNodeName = "contrast";
    // private final static String BrightnessNodeName = "brightness";
    private final static String BrightnessNodeName = "saturation";

    private final static String ValueNodeName = "Value";
    private final static String DefValueName = "DefaultValue";

    final static String UIOrientationMode = "vtmode";
    final static String UIOrientationMode_Landscape = "0";
    final static String UIOrientationMode_Portrait = "1";

    final static int BRIGHT_SETTING = 1;
    final static int CONSTRAST_SETTING = 2;
    final static private VTServiceInterface mVTServiceInterface = new VTServiceInterface();

    class Values {
        Values() {
            mBrightValues = new ArrayList<String>();
            mContrastValues = new ArrayList<String>();
        }

        ArrayList<String> mBrightValues;
        int mDefBrightValueIndex;
        int mCurBrightValueIndex;
        ArrayList<String> mContrastValues;
        int mDefContrastValueIndex;
        int mCurContrastValueIndex;
        String mBrightParameterName = BrightnessNodeName;
    };

    private Values mMainValues = new Values();
    private Values mSecondValues = new Values();
    private boolean mInitialized = false;

    int mActiveCamera;

    CameraSetting() {

        mInitialized = loadCameraConfigFromXML();

        mActiveCamera = VTManager.VideoSource.CAMERA_SECONDARY;

        mMainValues.mCurBrightValueIndex = mMainValues.mDefBrightValueIndex;
        mMainValues.mCurContrastValueIndex = mMainValues.mDefContrastValueIndex;

        mSecondValues.mCurBrightValueIndex = mSecondValues.mDefBrightValueIndex;
        mSecondValues.mCurContrastValueIndex = mSecondValues.mDefContrastValueIndex;
    };

    void updateCameraParams(VTService vts, int whichSetting, int newIndex) {
        if (!mInitialized)
            return;

        Values v;
        if (mActiveCamera == VTManager.VideoSource.CAMERA_SECONDARY) {
            v = mSecondValues;
        } else if (mActiveCamera == VTManager.VideoSource.CAMERA_MAIN) {
            v = mMainValues;
        } else {
            return;
        }

        if (whichSetting == BRIGHT_SETTING) {
            if (newIndex < 0 || newIndex >= v.mBrightValues.size()) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "wrong index range, " + newIndex);
                return;
            }
            if (vts != null)
                mVTServiceInterface.setCameraParameter(v.mBrightParameterName,
                        v.mBrightValues.get(newIndex), vts);
            v.mCurBrightValueIndex = newIndex;
        } else if (whichSetting == CONSTRAST_SETTING) {
            if (newIndex < 0 || newIndex >= v.mContrastValues.size()) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "wrong index range, " + newIndex);
                return;
            }
            if (vts != null)
                mVTServiceInterface.setCameraParameter(ContrastNodeName,
                        v.mContrastValues.get(newIndex), vts);
            v.mCurContrastValueIndex = newIndex;
        }
        return;
    }

    void resetValues(int newCameraIndex) {
        mActiveCamera = newCameraIndex;
        mMainValues.mCurBrightValueIndex = mMainValues.mDefBrightValueIndex;
        mMainValues.mCurContrastValueIndex = mMainValues.mDefContrastValueIndex;
        mSecondValues.mCurBrightValueIndex = mSecondValues.mDefBrightValueIndex;
        mSecondValues.mCurContrastValueIndex = mSecondValues.mDefContrastValueIndex;
    }

    int getSettingDegree(int whichSetting) {

        if (!mInitialized)
            return 0;

        Values v;
        if (mActiveCamera == VTManager.VideoSource.CAMERA_SECONDARY) {
            v = mSecondValues;
        } else if (mActiveCamera == VTManager.VideoSource.CAMERA_MAIN) {
            v = mMainValues;
        } else {
            return 0;
        }

        if (whichSetting == BRIGHT_SETTING) {
            return v.mBrightValues.size() - 1;
        } else if (whichSetting == CONSTRAST_SETTING) {
            return v.mContrastValues.size() - 1;
        }

        return 0;
    }

    int getCurrentValueIndex(int whichSetting) {

        if (!mInitialized)
            return 0;

        Values v;

        if (mActiveCamera == VTManager.VideoSource.CAMERA_SECONDARY) {
            v = mSecondValues;
        } else if (mActiveCamera == VTManager.VideoSource.CAMERA_MAIN) {
            v = mMainValues;
        } else {
            return 0;
        }

        if (whichSetting == BRIGHT_SETTING) {
            return v.mCurBrightValueIndex;
        } else if (whichSetting == CONSTRAST_SETTING) {
            return v.mCurContrastValueIndex;
        }

        return 0;
    }

    private static String CAMERA_CONFIG_XML = VTServiceInterface.getAppContext().getFilesDir().getAbsolutePath()+"/vtcameraconfig.xml";

    private int parseValues(ArrayList<String> values, Node s) {

        NodeList items = s.getChildNodes();
        String def = "";
        int def_index;

        if (MyLog.DEBUG)
            MyLog.d(TAG, "parseValues.");

        for (int k = 0; k < items.getLength(); k++) {
            // parse setting items
            Node item = items.item(k);
            if (item.getNodeType() == Node.ELEMENT_NODE && item.getNodeName().equals(DefValueName)) {
                // default value
                NodeList nodes = item.getChildNodes();
                for (int l = 0; l < nodes.getLength(); l++) {
                    Node node = nodes.item(l);
                    if (node.getNodeType() == Node.TEXT_NODE) {
                        def = node.getNodeValue();
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "get default value:" + def);
                    }
                }
            } else if (item.getNodeType() == Node.ELEMENT_NODE
                    && item.getNodeName().equals(ValueNodeName)) {
                // values
                NodeList nodes = item.getChildNodes();
                for (int l = 0; l < nodes.getLength(); l++) {
                    Node node = nodes.item(l);
                    if (node.getNodeType() == Node.TEXT_NODE) {
                        values.add(node.getNodeValue());
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "get a new value item:" + node.getNodeValue());
                    }
                }
            }
        }// end of k loop

        // to find the def index
        def_index = values.indexOf(def);
        if (def_index == -1)
            def_index = 0;

        return def_index;
    }

    boolean loadCameraConfigFromXML() {

        File file = new File(CAMERA_CONFIG_XML);
        if (!file.exists()) {
            if (MyLog.DEBUG)
                Log.d(TAG, "First Time -----------------");
            int resID = VTServiceCallUtils.getResourseIdByName("com.borqs.videocall", "raw",
                    "vtcameraconfig");
            // Open your local db as the input stream

            InputStream myInput = VTServiceInterface.getAppContext().getResources()
                    .openRawResource(resID);

            // Path to the just created empty db
            String outFileName = CAMERA_CONFIG_XML;

            // Open the empty db as the output stream
            OutputStream myOutput;
            try {
                myOutput = new FileOutputStream(outFileName);
                // transfer bytes from the inputfile to the outputfile
                byte[] buffer = new byte[1024];
                int length;
                while ((length = myInput.read(buffer)) > 0) {
                    myOutput.write(buffer, 0, length);
                }
                // Close the streams
                myOutput.flush();
                myOutput.close();
                myInput.close();
            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        try {
            File configFile = new File(CAMERA_CONFIG_XML);
            if (null == configFile) {
                Log.e(TAG, "could not access " + CAMERA_CONFIG_XML);
                // return false;
            }

            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document configDoc = builder.parse(configFile);

            // find VT node
            NodeList nl = configDoc.getElementsByTagName(VTNodeName);
            Node VTConfig = nl.item(0);
            NodeList cameras = VTConfig.getChildNodes();
            for (int i = 0; i < cameras.getLength(); i++) {
                // parse settings
                Node c = cameras.item(i);

                if (c.getNodeType() != Node.ELEMENT_NODE) {
                    continue;
                }
                // second level settings
                if (MyLog.DEBUG)
                    MyLog.v(TAG, "process " + c.getNodeName());
                NodeList settings;
                Values v;

                if (MainNodeName.equals(c.getNodeName())) {
                    v = mMainValues;
                } else if (SecondNodeName.equals(c.getNodeName())) {
                    v = mSecondValues;
                } else {
                    continue;
                }

                settings = c.getChildNodes();
                for (int j = 0; j < settings.getLength(); j++) {
                    if (MyLog.DEBUG)
                        MyLog.v(TAG, "node: " + j + "; name :" + settings.item(j).getNodeName());
                    Node s = settings.item(j);
                    if (s.getNodeType() != Node.ELEMENT_NODE) {
                        continue;
                    }
                    // second level settings
                    if (MyLog.DEBUG)
                        MyLog.v(TAG, "process " + s.getNodeName());

                    // Contrast
                    if (ContrastNodeName.equals(s.getNodeName())) {
                        v.mDefContrastValueIndex = parseValues(v.mContrastValues, s);
                    } else if (BrightnessNodeName.equals(s.getNodeName())) {
                        v.mDefBrightValueIndex = parseValues(v.mBrightValues, s);
                        if (s.hasAttributes() && s.getAttributes().getNamedItem("name") != null) {
                            v.mBrightParameterName = s.getAttributes().item(0).getNodeValue();
                        } else {
                            v.mBrightParameterName = BrightnessNodeName;
                        }

                    } else {
                        continue;
                    }
                }// end of for settings
            }// end of for cameras
        } catch (SAXException ex) {
            Log.e(TAG, "Failed to parse camera config file" + ex);
            return false;
        } catch (ParserConfigurationException ex) {
            Log.e(TAG, "Failed to parse camera config file" + ex);
            return false;
        } catch (IOException ex) {
            Log.e(TAG, "Failed to parse camera config file" + ex);
            return false;
        }
        return true;
    }

};
