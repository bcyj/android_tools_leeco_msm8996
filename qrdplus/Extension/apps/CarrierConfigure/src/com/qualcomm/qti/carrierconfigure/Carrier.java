/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;

import com.qualcomm.qti.loadcarrier.ILoadCarrierService;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class Carrier implements Parcelable {
    private static final String TAG = "Carrier";

    // The path to store the carrier's configuration files.
    public static final String SPEC_CONFIG_PATH = "/system/vendor/speccfg/";

    // The type for carrier.
    private static final String TYPE_UNKNOWN      = "unknown";
    public static final String TYPE_DIRECTORY    = "directory";
    public static final String TYPE_OTA_ZIP_FILE = "ota_zip";

    // This carrier has AP settings and a MBN file.
    public static final String ITEM_MODE_AP_MP = "AP_MP";
    // This carrier has a MBN file only.
    public static final String ITEM_MODE_MP    = "MP";
    // This carrier has AP settings only.
    public static final String ITEM_MODE_AP    = "AP";

    // The content item filter.
    private static final String FILTER_ITEM_ICC      = "ICC";
    private static final String FILTER_ITEM_MCCMNC   = "MCCMNC";
    private static final String FILTER_ITEM_SPN      = "SPN";
    private static final String FILTER_ITEM_BRAND    = "Brand";
    private static final String FILTER_ITEM_TARGET   = "Target";
    private static final String FILTER_ITEM_VERSION  = "Version";
    private static final String FILTER_ITEM_OTA_PATH = "OTA-file-path";
    private static final String FILTER_ITEM_MBN_NAME = "MBN-file-name";
    // enhance trigger
    private static final String FILTER_ITEM_GID      = "GID";
    private static final String FILTER_ITEM_MODEL    = "Model";
    private static final String FILTER_ITEM_HIERARCHY= "Hierarchy";
    private static final String FILTER_ITEM_PRIORITY = "Priority";
    private static final String FILTER_ITEM_STORAGE  = "Storage";
    private static final String FILTER_ITEM_DEPENDENCY="Dependency";
    private static final String FILTER_ITEM_MODE     = "Mode";

    // The default spec name.
    private static final String DEFAULT_SPEC_NAME = "Default";

    // The path used to get the carriers.
    private static final String NOT_TRIGGERED_FILE_PATH = "/data/switch_spec/.not_triggered";
    public static final String SYSTEM_VENDOR_PATH      = "/system/vendor/";

    // The file names.
    private static final String SPEC_MARKED_FILE_NAME = ".preloadspec";
    private static final String SPEC_FILE_NAME        = "spec";
    private static final String OTA_FILE_EXTENSION    = ".ota.zip";

    // The string used to get or set carrier value.
    private static final String CURRENT_CARRIER_STR_PRE = "strSpec=";

    // The separate used in the .preloadspec file.
    private static final String SEP_BLANK = " ";
    private static final String SEP_EQUAL = "=";
    private static final String SEP_QUOTE = "\"";
    private static final String SEP_TWO   = "2";
    private static final String SEP_UNDERLINE = "_";

    public final String mName;
    public final String mType;
    public final String mPath;
    private String mOverridePath =  null;
    public final String mContent;

    private HashMap<String, String> mContentItems = new HashMap<String, String>();

    // This list to save the carrier's name is same,
    // but the type or path or version is not same.
    private ArrayList<Carrier> mSimilarCarrierList = new ArrayList<Carrier>();
    // This list to save the carrier's hierarchy is same,
    // but the name or type or path or version is not same.
    private ArrayList<Carrier> mSameHierarchyCarrierList = new ArrayList<Carrier>();

    public Carrier(String name, String type) {
        this(name, type, null);
    }

    public Carrier(String name, String type, String path) {
        this(name, type, path, null);
    }

    public Carrier(String name, String type, String path, String content) {
        if (TextUtils.isEmpty(name) || TextUtils.isEmpty(type)) {
            throw new IllegalArgumentException("The carrier's name or type is empty.");
        }

        mName = name;
        mType = type;
        mPath = path;
        mContent = content;
        parseContent();
    }

    public static final Creator<Carrier> CREATOR = new Creator<Carrier>() {
        public Carrier createFromParcel(Parcel in) {
            return new Carrier(in);
        }

        public Carrier[] newArray(int size) {
            return new Carrier[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    public Carrier(Parcel in) {
        this.mName = in.readString();
        this.mType = in.readString();
        this.mPath = in.readString();
        this.mContent = in.readString();
        this.mSimilarCarrierList = in.createTypedArrayList(CREATOR);
        this.mSameHierarchyCarrierList = in.createTypedArrayList(CREATOR);
        parseContent();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mName);
        dest.writeString(mType);
        dest.writeString(mPath);
        dest.writeString(mContent);
        dest.writeTypedList(mSimilarCarrierList);
        dest.writeTypedList(mSameHierarchyCarrierList);
    }

    public static Carrier getCurrentCarrierInstance() {
        String currentCarrierName = getCurrentCarriersName(getCurrentCarriers());
        File currentDirectory = new File(SYSTEM_VENDOR_PATH + "/" + currentCarrierName);
        if (currentDirectory.isDirectory()) {
            File propFile = new File(currentDirectory.getAbsolutePath() + "/"
                    + SPEC_MARKED_FILE_NAME);
            if (propFile.exists()) {
                String content = Utils.readFirstLine(propFile, null);
                Carrier carrier = new Carrier(currentCarrierName, TYPE_DIRECTORY,
                        currentDirectory.getAbsolutePath(), content);
                return carrier;
            }
        }
        return null;
    }

    /**
     * Return the current carrier's name.
     */
    public static String getCurrentCarrier() {
        File specFile = new File(SPEC_CONFIG_PATH + SPEC_FILE_NAME);

        try {
            // If the file size is zero, we will return null as no content.
            if (Utils.getFileSize(specFile) == 0) return DEFAULT_SPEC_NAME;

            String regularExpression = "^" + CURRENT_CARRIER_STR_PRE + "\\w+$";
            String content = Utils.readFirstLine(specFile, regularExpression);
            if (TextUtils.isEmpty(content)) return DEFAULT_SPEC_NAME;

            return content.substring(CURRENT_CARRIER_STR_PRE.length());
        } catch (IOException e) {
            Log.e(TAG, "Get current carrier error, caused by: " + e.getMessage());
        }

        return DEFAULT_SPEC_NAME;
    }

    /**
     * Return the current carrier's name.
     */
    private final static String CURRENT_CARRIERS_NUM_PRE = "packCount=";
    private final static String CURRENT_CARRIERS_NAME_PRE = "strSpec";
    public static ArrayList<String> getCurrentCarriers() {

        File specFile = new File(SPEC_CONFIG_PATH + SPEC_FILE_NAME);

        ArrayList<String> Carriers = new ArrayList<String>();
        try {
            // If the file size is zero, we will return null as no content.
            if (Utils.getFileSize(specFile) == 0) {
                Carriers.add(DEFAULT_SPEC_NAME);
                return Carriers;
            }

            ArrayList<String> contents = Utils.readFile(specFile, null, false);

            String carrierNumRegularExpresstion = "^" + CURRENT_CARRIERS_NUM_PRE + "[0-9]$";
            if (!contents.get(0).matches(carrierNumRegularExpresstion)) {
                Carriers.add(DEFAULT_SPEC_NAME);
                return Carriers;
            }
            int carriersNum = Integer.parseInt(contents.get(0)
                    .substring(CURRENT_CARRIERS_NUM_PRE.length()));
            if (carriersNum <= 0 || contents.size() <= carriersNum) {
                Carriers.add(DEFAULT_SPEC_NAME);
                return Carriers;
            }
            for(int i=1; i<=carriersNum; i++) {
                String carrierRegularExpresstion = "^" + CURRENT_CARRIERS_NAME_PRE + "[0-9]=\\w+$";
                if (contents.get(i).matches(carrierRegularExpresstion)) {
                    Carriers.add(contents.get(i).substring(CURRENT_CARRIERS_NAME_PRE.length()+2));
                } else {
                    Carriers.clear();
                    Carriers.add(DEFAULT_SPEC_NAME);
                    return Carriers;
                }
            }
            return Carriers;
        } catch (IOException e) {
            Log.e(TAG, "Get current carrier error, caused by: " + e.getMessage());
        }
        Carriers.add(DEFAULT_SPEC_NAME);
        return Carriers;
    }


    /**
     * Save these carriers as not triggered carriers, and these carriers will never be triggered
     * until the user reset the phone.
     */
    public static void saveAsNotTriggeredCarrier(ArrayList<Carrier> carrierList) {
        if (carrierList == null || carrierList.size() < 1) return;

        for (Carrier carrier : carrierList) {
            if (carrier.mName.endsWith("2Default")) continue;
            saveAsNotTriggeredCarrier(carrier);
        }
    }

    /**
     * Save this carrier as not triggered carrier, and this carrier will never be triggered
     * until the user reset the phone.
     */
    public static void saveAsNotTriggeredCarrier(Carrier carrier) {
        if (carrier == null) return;

        File file = new File(NOT_TRIGGERED_FILE_PATH);
        try {
            if (!file.exists()) file.createNewFile();
            if (file.canWrite()) {
                if (Utils.DEBUG) Log.d(TAG, "Save the " + carrier.mName + " as not triggered.");
                Utils.writeFile(file, carrier.mName + "\n", true /* append */);
            } else {
                throw new IOException("Can not write the file(" + file.getAbsolutePath() + ").");
            }
        } catch (IOException e) {
            Log.e(TAG, "Couldn't save as not triggered carrier(" + carrier + "), caused by: "
                    + e.getMessage());
        }
    }

    /**
     * Return the not triggered carriers' name.
     */
    public static ArrayList<String> getNotTriggeredCarriersName() {
        ArrayList<String> list = null;

        File file = new File(NOT_TRIGGERED_FILE_PATH);
        try {
            // If the file size is 0, it means the file is not exist or the content is empty.
            if (Utils.getFileSize(file) == 0) return null;

            list = Utils.readFile(file, null, false);
        } catch (IOException e) {
            Log.e(TAG, "Can not get the needn't triggered carrier, caused by: " + e.getMessage());
        }

        return list;
    }

    /**
     * Parse the preset carriers and save the result into the storage.
     */
    public static void parsePresetCarriers(CarriersStorage storage) {
        ArrayList<String> carriersName = getCurrentCarriers();
        String currentTopCarrier = carriersName.get(carriersName.size() - 1);
        // Insert all the preset specs into the database.
        File vendorDir = new File(SYSTEM_VENDOR_PATH);
        String[] paths = vendorDir.list();
        for (String path : paths) {
            File child = new File(SYSTEM_VENDOR_PATH + "/" + path);
            if (!child.getName().equals(currentTopCarrier)) continue;
            if (child.isDirectory()) {
                File propFile = new File(child.getAbsolutePath() + "/"
                        + SPEC_MARKED_FILE_NAME);
                if (propFile.exists()) {
                    String content = Utils.readFirstLine(propFile, null);
                    Carrier carrier = new Carrier(getCurrentCarriersName(carriersName),
                            TYPE_DIRECTORY, child.getAbsolutePath(), content);
                    storage.addCarrier(carrier);
                }
            }
        }
    }

    /**
     * Parse the carriers from the given list, and save the result into the storage.
     * The list must be as:<br>
     *     key = carrier's path<br>
     *     value = carrier's content<br>
     */
    public static void parseCarriers(HashMap<String, String> list, CarriersStorage storage) {
        if (list == null || list.size() < 1) return;

        // Build the carrier list for the carriers on the SD card.
        if (list != null && list.size() > 0) {
            Iterator<Entry<String, String>> iterator = list.entrySet().iterator();
            while (iterator.hasNext()) {
                Entry<String, String> entry = iterator.next();
                String path = entry.getKey();
                String name = getCarrierName(path);
                String type = getCarrierType(path);
                Carrier carrier = new Carrier(name, type, path, entry.getValue());
                storage.addCarrier(carrier);
            }
        }
    }

    public static String getCurrentCarriersName(ArrayList<String> list) {
        if (list.size() == 0) return null;
        if (list.size() == 1) return list.get(0);
        String fullName = null;
        for (String name : list) {
            if (fullName == null)
                fullName = name;
            else fullName += "_" + name;
        }
        return fullName;
    }

    public String getRegionalCarrierTitle() {
        String name[] = getSwitchCarrierName().split(SEP_UNDERLINE);
        String title = null;
        String preSpace = "";
        for (int i = name.length; i > 0; i--) {
            if (title == null) title = name[i-1];
            else {
               preSpace += "  ";
               title += "\n" + preSpace + "-" + name[i-1];
            }
        }
        return title;
    }

    public String getTopCarrierTitle() {
        String name[] = getSwitchCarrierName().split(SEP_UNDERLINE);
        return name[name.length - 1];
    }

    public static String[] getCarriersNameFromString(String str) {
        return str.split(SEP_UNDERLINE);
    }

    public static String getBaseCarrierName(String fullName) {
        String items[] = fullName.split(SEP_TWO);
        if (items.length == 1)
            return DEFAULT_SPEC_NAME;
        return items[0];
    }

    public static String getSwitchCarrierName(String fullName) {
        String items[] = fullName.split(SEP_TWO);
        if (items.length == 1)
            return items[0];
        return items[1];
    }

    public String getBaseCarrierName() {
        String items[] = mName.split(SEP_TWO);
        if (items.length == 1)
            return DEFAULT_SPEC_NAME;
        return items[0];
    }

    public String getSwitchCarrierName() {
        String items[] = mName.split(SEP_TWO);
        if (items.length == 1)
            return items[0];
        return items[1];
    }

    /**
     * Get the carrier's name from the path.
     */
    public static String getCarrierName(String path) {
        String fileName = path.substring(path.lastIndexOf("/") + 1);
        return fileName.replace(".ota.zip", "");
    }

    /**
     * Get the carrier's type from the path.
     */
    public static String getCarrierType(String path) {
        File file = new File(path);
        if (file.isDirectory()) {
            return TYPE_DIRECTORY;
        } else if (path.endsWith(OTA_FILE_EXTENSION)) {
            return TYPE_OTA_ZIP_FILE;
        }
        return TYPE_UNKNOWN;
    }

    private boolean isSameHierarchy(Carrier sameHierarchyCarrier) {
        if (this.getHierarchy().equals(sameHierarchyCarrier.getHierarchy())
                && !this.equals(sameHierarchyCarrier)) {
            return true;
        }
        return false;
    }

    /**
     * Add the given carrier as same hierarchy carrier list.
     */
    public boolean addAsSameHierarchyCarrier(Carrier similarCarrier) {
        if (isSameHierarchy(similarCarrier)) {
            mSameHierarchyCarrierList.add(similarCarrier);
            return true;
        }
        return false;
    }

    /**
     * Add the given carrier as similar carrier.
     */
    public boolean addAsSimilarCarrier(Carrier similarCarrier) {
        if (isSimilar(similarCarrier)) {
            mSimilarCarrierList.add(similarCarrier);
            return true;
        }
        return false;
    }

    /**
     * Get the same hierarchy carrier list.
     */
    public ArrayList<Carrier> getSameHierarchyCarrierList() {
        return mSameHierarchyCarrierList;
    }

    /**
     * Get the similar carrier list.
     */
    public ArrayList<Carrier> getSimilarCarrierList() {
        return mSimilarCarrierList;
    }

    /**
     * Get the Hierarchy content.
     */
    public String getHierarchy() {
        String hierarchy = getContentItem(FILTER_ITEM_HIERARCHY);
        return (TextUtils.isEmpty(hierarchy)) ? "" : "Operator";
    }

    /**
     * Get the Priority content.
     */
    public int getPriority() {
        String priority = getContentItem(FILTER_ITEM_PRIORITY);
        if (TextUtils.isEmpty(priority)) {
            return -1;
        } else {
            return Integer.parseInt(priority);
        }
    }

    /**
     * Get the Storage content.
     */
    public String getStorage() {
        return getContentItem(FILTER_ITEM_STORAGE);
    }

    public boolean saveOnCloud() {
        String storage = getStorage();
        if (TextUtils.isEmpty(storage) || storage.equalsIgnoreCase("preset")) {
            return false;
        }
        return true;
    }

    public String getDependency() {
        return getContentItem(FILTER_ITEM_DEPENDENCY);
    }

    /**
     * Get the ICC content.
     */
    public String getICC() {
        return getContentItem(FILTER_ITEM_ICC);
    }

    /**
     * Get the MCCMNC content.
     */
    public String getMCCMNC() {
        return getContentItem(FILTER_ITEM_MCCMNC);
    }

    /**
     * Get the SPN content.
     */
    public String getSPN() {
        return getContentItem(FILTER_ITEM_SPN);
    }

    /**
     * Get the brand content.
     */
    public String getBrand() {
        return getContentItem(FILTER_ITEM_BRAND);
    }

    /**
     * Get the target content.
     */
    public String getTarget() {
        return getContentItem(FILTER_ITEM_TARGET);
    }

    /**
     * Get the GID content.
     */
    public String getGID() {
        return getContentItem(FILTER_ITEM_GID);
    }

    /**
     * Get the version content.
     */
    public int getVersion() {
        String version = getContentItem(FILTER_ITEM_VERSION);
        try {
            return Integer.parseInt(version);
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    /**
     * Get the mode content.
     */
     public String getMode() {
        return getContentItem(FILTER_ITEM_MODE);
     }

    /**
     * Get the ota path content.
     */
    public String getOTAPath() {
        return getContentItem(FILTER_ITEM_OTA_PATH);
    }

    /**
     * Get the download ota path
     */
    public String getPath() {
        if (mOverridePath != null) {
            return mOverridePath;
        }
        return mPath;
    }

    /**
     * Set the download ota override path
     */
    public void setOverridePath(String overridePath) {
        mOverridePath = overridePath;
    }

    /**
     * Get the carrier mbn file name which used to update the NVs.
     *
     * Note: If the file which is named as this "modem_config/mcfg_sw.mbn"
     *       is not exist, it will return null.
     */
    public String getMBNFileName() {
        String name = getContentItem(FILTER_ITEM_MBN_NAME);
        if (TextUtils.isEmpty(name)) {
            name = Utils.getCarrierMBNPathInOta();
        }
        return ensurePathExist(name);
    }

    /**
     * Get the carrier ROW mbn file name which used to update the NVs.
     *
     * Note: If the file which is named as this "modem_row_config/mcfg_sw.mbn"
     *       is not exist, it will return null.
     */
    public String getROWMBNFileName() {
        String name = Utils.getCarrierROWMBNPathInOta();
        return ensurePathExist(name);
    }

    /**
     * Find the latest carrier from similar carriers and this.
     */
    public Carrier findLatestCarrier() {
        if (mSimilarCarrierList == null || mSimilarCarrierList.size() < 1) return this;

        Carrier latestCarrier = this;
        for (Carrier c : mSimilarCarrierList) {
            if (c.getVersion() > latestCarrier.getVersion()) {
                latestCarrier = c;
            }
        }

        return latestCarrier;
    }

    /**
     * Used to get the switch data for this carrier. And this data will be used
     * for start switch action.
     *
     * @param service used to copy the OTA file to data if needed.
     * @return the data used for start switch action.
     * @throws NullServiceException if this carrier need copy the OTA file to data,
     *         but the service used to do this action is null.
     * @throws EmptyPathException if this carrier need copy the OTA file to data,
     *         but the action failed.
     */
    public SwitchData getSwitchData(ILoadCarrierService service)
            throws NullServiceException, EmptyPathException {
        // It will init according to the carrier's type.
        String name = null;
        String path = null;
        ArrayList<SwitchData> dataList = new ArrayList<SwitchData>();

        if (Carrier.TYPE_DIRECTORY.equals(mType) && !saveOnCloud()) {
            name = mName;
            path = getPath();
        }

        if (Carrier.TYPE_OTA_ZIP_FILE.equals(mType) && !saveOnCloud()) {
            if (service == null) throw new NullServiceException();

            // Caused by the system application couldn't access the SD card.
            // So we need copy the file to data first.
            try {
                path = service.copyToData(getPath());
            } catch (RemoteException e) {
                Log.e(TAG, "Copy the file(" + getPath() + ") error.");
            }
            if (TextUtils.isEmpty(path)) throw new EmptyPathException();
        }

        if (saveOnCloud()) {
            // Caused by the system application couldn't access the SD card.
            // So we need download the file first.
            try {
                path = service.downloadToData(getStorage());
            } catch (RemoteException e) {
                Log.e(TAG, "Download the file(" + mPath + ") error.");
            }
        }

        return new SwitchData(name, path);
    }

    @Override
    public String toString() {
        return mName + " is "  + mType + ", stored in " + mPath;
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof Carrier) {
            Carrier c = (Carrier) o;
            return c.mName.equals(mName)
                    && c.mType.equals(mType)
                    && c.mPath.equals(mPath)
                    && c.mContent.equals(mContent);
        }
        return super.equals(o);
    }

    private boolean isSimilar(Carrier similarCarrier) {
        if (mName.equals(similarCarrier.mName)
                && !this.equals(similarCarrier)) {
            return true;
        }
        return false;
    }

    private void parseContent() {
        if (TextUtils.isEmpty(mContent)) return;

        // Build the contents' item.
        if (mContentItems == null) {
            mContentItems = new HashMap<String, String>();
        }
        mContentItems.clear();

        // Items will be separated by bland(" ").
        String[] items = mContent.split(SEP_BLANK);
        for (String item : items) {
            // The item's content must similar as ICC="".
            String[] itemContents = item.split(SEP_EQUAL);
            mContentItems.put(formatKey(itemContents[0]),
                    itemContents[1].replaceAll(SEP_QUOTE, ""));
        }
    }

    private String formatKey(String key) {
        return key.toUpperCase();
    }

    private String getContentItem(String filter) {
        if (mContentItems.size() < 1) return null;

        return mContentItems.get(formatKey(filter));
    }

    private String ensurePathExist(String name) {
        if (name == null) name = "";
        String mbnFileName = null;
        if (mType.equals(TYPE_DIRECTORY)) {
            mbnFileName = Utils.getPresetMBNFile(getPath(), name + ".mbn");
        } else {
            mbnFileName = Utils.getMBNFileFromOta(getPath(), Utils.getMBNPath(), name + ".mbn");
        }

        if (!TextUtils.isEmpty(mbnFileName)) {
            try {
                Runtime.getRuntime().exec("chmod 644 " + mbnFileName);
                Runtime.getRuntime().exec("chown radio.radio " + mbnFileName);
            } catch (IOException e){
                Log.e(TAG, "failed chmod or chown file:" + mbnFileName);
                return null;
            }
            return mbnFileName;
        }
        String filePath = Utils.getMBNPath() + "/" + name + ".mbn";
        File file = new File(filePath);
        if (file != null && file.exists() && file.isFile()) {
            // We will only return the path if this file exist and not one directory.
            return name;
        }

        return null;
    }

    public static class CarriersStorage {
        private HashMap<String, Carrier> mMapCarrierName = new HashMap<String, Carrier>();
        private HashMap<String, Carrier> mMapCarrierHierarchy = new HashMap<String, Carrier>();

        public ArrayList<Carrier> mListCarrier = new ArrayList<Carrier>();
        public ArrayList<Carrier> mListHierarchy = new ArrayList<Carrier>();

        public void reset() {
            mMapCarrierName.clear();
            mMapCarrierHierarchy.clear();
            mListCarrier.clear();
            mListHierarchy.clear();
        }

        public boolean isEmpty() {
            return mMapCarrierName.isEmpty();
        }

        public void addCarrier(Carrier newCarrier) {
            if (newCarrier == null) return;

            Carrier sameCarrier = mMapCarrierName.get(newCarrier.mName);
            if (sameCarrier == null) {
                mMapCarrierName.put(newCarrier.mName, newCarrier);
                mListCarrier.add(newCarrier);
            } else {
                if (!sameCarrier.addAsSimilarCarrier(newCarrier)) {
                    throw new IllegalArgumentException("Couldn't add this carrier("
                            + newCarrier + ")" + " as the similar carrier(" + sameCarrier + ")");
                }
            }

            Carrier sameHierarchy = mMapCarrierHierarchy.get(newCarrier.getHierarchy());
            if (sameHierarchy == null) {
                mMapCarrierHierarchy.put(newCarrier.getHierarchy(), newCarrier);
                addToHierarchyListByPriority(newCarrier);
            } else {
                if (sameCarrier == null && !sameHierarchy.addAsSameHierarchyCarrier(newCarrier)) {
                    throw new IllegalArgumentException("Couldn't add this carrier("
                            + newCarrier + ")" + " as the same hierarchy carrier ("
                            + sameHierarchy + ")");
                }
            }
        }

        private void addToHierarchyListByPriority(Carrier carrier) {
            for (int index = 0; index < mListHierarchy.size() ;index ++) {
                if (mListHierarchy.get(index).getPriority() >= carrier.getPriority()) {
                    mListHierarchy.add(index, carrier);
                    return;
                }
            }
            mListHierarchy.add(carrier);
        }
    }

    public static class SwitchData {
        public String _name;
        public String _path;

        public SwitchData(String name, String path) {
            _name = name;
            _path = path;
        }
    }

    public static class NullServiceException extends IllegalArgumentException {
        private static final long serialVersionUID = 1L;
        public NullServiceException() {
            super("Service is null.");
        }
    }

    public static class EmptyPathException extends IllegalArgumentException {
        private static final long serialVersionUID = 1L;
        public EmptyPathException() {
            super("The new path is empty.");
        }
    }
}
