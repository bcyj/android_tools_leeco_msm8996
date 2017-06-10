/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import android.os.Environment;
import android.os.RecoverySystem;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class Utils {
    private static final String TAG = "LoadCarrier.Utils";
    public static final boolean DEBUG = true;

    public static final String PROP_KEY_TRIGGER        = "persist.radio.trigger";
    public static final String PROP_KEY_TRIGGER_PATH   = "persist.radio.trigger.path";
    public static final String PROP_KEY_TRIGGER_BRAND  = "persist.radio.trigger.brand";
    public static final String PROP_KEY_TRIGGER_TARGET = "persist.radio.trigger.target";

    public static final int FLAG_SYSTEM_VENDOR     = 1;
    public static final int FLAG_STORAGE_EXTERNAL  = 2;
    public static final int FLAG_STORAGE_SECONDARY = 3;

    public static final String FILTER_ITEM_ICC     = "ICC";
    public static final String FILTER_ITEM_MCC     = "MCC";
    public static final String FILTER_ITEM_MCCMNC  = "MCCMNC";
    public static final String FILTER_ITEM_SPN     = "SPN";
    public static final String FILTER_ITEM_BRAND   = "Brand";
    public static final String FILTER_ITEM_TARGET  = "Target";
    public static final String FILTER_ITEM_VERSION = "Version";
    public static final String FILTER_ITEM_NAME    = "Name";
    public static final String FILTER_ITEM_DEPENDENCY = "Dependency";
    // enhance trigger
    public static final String FILTER_ITEM_MODEL   = "Model";
    public static final String FILTER_ITEM_HIERARCHY = "Hierarchy";
    public static final String FILTER_ITEM_GID = "GID";

    //Special ROW package name. Should be sync with CarrierConfig/Utils.java
    public static final String SPECIAL_ROW_PACKAGE_NAME = "ROW";

    private static final String STORAGE_EXTERNAL  = "EXTERNAL_STORAGE";
    private static final String STORAGE_SECONDARY = "SECONDARY_STORAGE";

    private static final String SYSTEM_VENDOR_PATH        = "/system/vendor";
    private static final String SYSTEM_VENDOR_PRIEX       = "system/vendor";
    private static final String SPEC_MARKED_FILE_NAME     = ".preloadspec";
    private static final String CARRIER_OTA_ZIP_FILE_NAME = ".ota.zip";

    /**
     * Get the prop value, if couldn't find the prop, return the default value.
     */
    public static boolean getValue(String propKey, boolean defValue) {
        return SystemProperties.getBoolean(propKey, defValue);
    }

    /**
     * Get the prop value, if couldn't find the prop, return the default value.
     */
    public static String getValue(String propKey, String defValue) {
        return SystemProperties.get(propKey, defValue);
    }

    /**
     * Set the prop value.
     */
    public static void setValue(String propKey, String newValue) {
        SystemProperties.set(propKey, newValue);
    }

    /**
     * To get the external or internal storage path.
     * @param flag which path you want to get, the value must be:<br>
     *                {@link #FLAG_SYSTEM_VENDOR} for system vendor directory<br>
     *                {@link #FLAG_STORAGE_EXTERNAL} for external storage<br>
     *                {@link #FLAG_STORAGE_SECONDARY} for internal storage<br>
     * @return the path
     */
    public static String getPath(int flag) {
        switch (flag) {
            case FLAG_SYSTEM_VENDOR:
                return SYSTEM_VENDOR_PATH;
            case FLAG_STORAGE_EXTERNAL:
                // don't scan internal sdcard
                return null;
                //return System.getenv(STORAGE_EXTERNAL);
            case FLAG_STORAGE_SECONDARY:
                return System.getenv(STORAGE_SECONDARY);
            default:
                throw new IllegalStateException("Do not accept this flag: " + flag);
        }
    }

    /**
     * Only find the carrier as OTA file from the given path.
     * @param carrierList to save the list as key = path, value = content
     * @param path to find the carrier list under this path
     */
    public static void getCarrierList(HashMap<String, String> carrierList, String path) {
        getCarrierList(carrierList, path, true /* only filter for OTA */);
    }

    /**
     * Find the carrier list from the given path.
     * @param carrierList to save the list as key = path, value = content
     * @param path to find the carrier list under this path
     * @param onlyFilterOta if only find the OTA file
     */
    public static void getCarrierList(HashMap<String, String> carrierList, String path,
            boolean onlyFilterOta) {
        getCarrierList(carrierList, path, true /* only filter for OTA */, null);
    }

    /**
     * Find the carrier list from the given path, and the carrier need match the filter items.
     * @param carrierList to save the list as key = path, value = content
     * @param path to find the carrier list under this path
     * @param onlyFilterOta if only find the OTA file
     * @param filterItems the carrier need match this filter items
     */
    public static void getCarrierList(HashMap<String, String> carrierList, String path,
            boolean onlyFilterOta, ArrayList<HashMap<String, String>> filterItems) {
        if (DEBUG) Log.i(TAG, "Try to get the carrier list from " + path);

        if (path == null) return;
        File dir = new File(path);
        if (dir == null || !dir.exists() || !dir.canRead()) return;

        dir.listFiles(new CarrierFileFilter(carrierList, onlyFilterOta, filterItems));
    }

    /**
     * Copy the srcFile to dstFile
     * @param srcFile source file
     * @param dstFile destination file
     * @return true if successfully; false if failed.
     */
    public static boolean copyFile(File srcFile, File dstFile) {
        try {
            if (dstFile.exists()) {
                dstFile.delete();
            }

            InputStream in = new FileInputStream(srcFile);
            OutputStream out = new FileOutputStream(dstFile);
            try {
                int cnt;
                byte[] buf = new byte[4096];
                while ((cnt = in.read(buf)) >= 0) {
                    out.write(buf, 0, cnt);
                }
            } finally {
                out.close();
                in.close();
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    /**
     * The carrier file filter to filter the matched carriers.
     */
    private static class CarrierFileFilter implements FileFilter {
        private static final String OTA_ZIP_PATTERN = "(\\.ota\\.zip)$";

        private boolean mOnlyFilterOta;
        private HashMap<String, String> mCarrierList;
        private ArrayList<HashMap<String, String>> mFilterItems;

        public CarrierFileFilter(HashMap<String, String> carrierList,
                boolean onlyFilterOta, ArrayList<HashMap<String, String>> filterItems) {
            mCarrierList = carrierList;
            mOnlyFilterOta = onlyFilterOta;
            mFilterItems = filterItems;
        }

        @Override
        public boolean accept(File file) {
            try {
                String filename = file.getName();
                if (mOnlyFilterOta) {
                    if (filename.matches("^\\w+" + OTA_ZIP_PATTERN)
                            && acceptAsCarrierOta(file)) {
                        return true;
                    }
                } else {
                    return acceptAsPresetCarrier(file);
                }
            } catch (IOException e) {
                Log.e(TAG, "Catch the IOException when test if file(" + file + ") accept.");
            }

            return false;
        }

        private boolean acceptAsCarrierOta(File otaFile) throws IOException {
            if (DEBUG) Log.i(TAG, "Found the ota file, check if it accept. File: " + otaFile);

            // Verify the cryptographic signature of this OTA file.
            // Marked this to save the time.
            /*
            try {
                RecoverySystem.verifyPackage(otaFile, null, null);
            } catch (Exception ex) {
                // When we got the exception, it means the verify failed.
                Log.e(TAG, "Verify the signature of file(" + otaFile + ") failed. Caused by: "
                        + ex.getMessage());
                return false;
            }
            */

            // Check this OTA file's name.
            // The OTA file's name need same as "[CarrierName].ota.zip".
            String currentCarriers = getCurrentCarriersName(getCurrentCarriers());
            String carrierName = getCarrierName(otaFile);
            if (DEBUG) Log.d(TAG, "The carrier's name is: " + carrierName);

            JarFile carrierOtaFile = null;
            try {
                carrierOtaFile = new JarFile(otaFile);
                Enumeration<JarEntry> en = carrierOtaFile.entries();
                while (en.hasMoreElements()) {
                    JarEntry element = en.nextElement();

                    // If the current entry is directory, we will do nothing.
                    if (element.isDirectory()) continue;

                    // The spec marked file must store in Carrier folder.
                    // So if the length is N, then the items must be as this:
                    //     (N-2)  ----  CarrierName
                    //     (N-1)  ----  .preloadspec
                    String[] names = element.getName().split("/");
                    if (SPEC_MARKED_FILE_NAME.equals(names[names.length - 1])) {
                        if (carrierName.equals(names[names.length - 2])) {
                            // It means this is the accepted carrier OTA package.
                            if (mFilterItems == null || mFilterItems.size() < 1) {
                                try {
                                    CarrierPattern pattern =
                                            new CarrierPattern(carrierOtaFile, element);
                                    mCarrierList.put(otaFile.getAbsolutePath(), pattern.getContents());
                                } catch(Exception ex) {
                                    //carrierOtaFile maybe not security
                                    return false;
                                }
                                return true;
                            } else {
                                // Find the if this OTA match the filter items.
                                try {
                                    CarrierPattern pattern =
                                            new CarrierPattern(carrierOtaFile, element);
                                    if (DEBUG) Log.d(TAG,"Trigger carrierName = "+carrierName);
                                    if (pattern.matches(mFilterItems) || carrierName
                                            .equals(currentCarriers + "2Default")) {
                                        mCarrierList.put(
                                                otaFile.getAbsolutePath(), pattern.getContents());
                                        return true;
                                    }
                                } catch(Exception ex) {
                                    //carrierOtaFile maybe not security
                                    return false;
                                }
                            }
                        }
                    }
                }
            } finally {
                if (carrierOtaFile != null) {
                    carrierOtaFile.close();
                }
            }

            return false;
        }

        private boolean acceptAsPresetCarrier(File carrierDir) {
            if (DEBUG) Log.i(TAG, "Check if this folder accept. File: " + carrierDir);

            if (carrierDir == null || carrierDir.isFile()) return false;

            try {
                File[] subFiles = carrierDir.listFiles();
                if (subFiles == null || subFiles.length < 1) return false;

                for (File file : subFiles) {
                    if (SPEC_MARKED_FILE_NAME.equals(file.getName())) {
                        // It means this is the one carrier folder.
                        if (mFilterItems == null || mFilterItems.size() < 1) {
                            CarrierPattern pattern = new CarrierPattern(file);
                            mCarrierList.put(carrierDir.getAbsolutePath(), pattern.getContents());
                            return true;
                        } else {
                            // Find the if this carrier match the filter items.
                            CarrierPattern pattern = new CarrierPattern(file);
                            if (pattern.matches(mFilterItems)) {
                                mCarrierList.put(
                                        carrierDir.getAbsolutePath(), pattern.getContents());
                                return true;
                            }
                        }
                    }
                }
            } catch (IOException ex) {
                Log.w(TAG, "Catch the IOException: " + ex);
            }

            return false;
        }

        private String getCarrierName(File otaFile) {
            String fileName = otaFile.getName();
            String carrierName = fileName.substring(0, fileName.length()
                    - CARRIER_OTA_ZIP_FILE_NAME.length());
            return carrierName;
        }

        public class CarrierPattern {
            private static final String SEP_BLANK = " ";
            private static final String SEP_EQUAL = "=";
            private static final String SEP_COMMA = ",";
            private static final String SEP_QUOTE = "\"";

            private static final String RE_START = "(^|\\S+,)";
            private static final String RE_END = "(,\\S+|$)";
            private static final String RE_MNC = "(\\d{2,3})?";

            private String mContents;
            private HashMap<String, String> mContentItems;

            public CarrierPattern(JarFile file, JarEntry entry) throws IOException {
                parseContentFromZipFile(file, entry);
            }

            public CarrierPattern(File file) throws IOException {
                parseContentFromFile(file);
            }

            public String getContentItems(String key) {
                if (TextUtils.isEmpty(key)) {
                    return null;
                }

                if (mContentItems == null || mContentItems.size() < 1) {
                    return null;
                }

                return mContentItems.get(formatKey(key));
            }

            /**
             * Tests whether this carrier matches the given pattern items.
             * @return true if matched
             */
            public boolean matches(ArrayList<HashMap<String, String>> patternItems) {
                if (patternItems == null || patternItems.size() < 1) {
                    // The pattern items is null or empty, return true.
                    return true;
                }
                if (mContentItems == null || mContentItems.size() < 1) {
                    // This carrier's content item is null or empty, return false.
                    return false;
                }

                // If one of the patternItems matched the content, it means this
                // carrier is matched the pattern items.
                for (HashMap<String, String> patternItem : patternItems) {
                    // If all the items belong to this patternItem matched the content,
                    // it means this carrier is matched the pattern items.
                    boolean itemMatched = true;

                    Iterator<Entry<String, String>> iterator = patternItem.entrySet().iterator();
                    while (iterator.hasNext()) {
                        Entry<String, String> entry = iterator.next();

                        // Adjust the item for MCC.
                        boolean patternItemIsMCC = false;
                        String patternKey = formatKey(entry.getKey());
                        if (formatKey(FILTER_ITEM_MCC).equals(patternKey)) {
                            // If the filter item is MCC, we need find the content from MCCMNC.
                            patternItemIsMCC = true;
                            patternKey = formatKey(FILTER_ITEM_MCCMNC);
                        }

                        String contentValue = mContentItems.get(patternKey);
                        if (TextUtils.isEmpty(contentValue)) {
                            // Couldn't found this item or the item's content is empty.
                            // We will return false as this carrier do not match.
                            itemMatched = false;
                            break;
                        } else {
                            // If one of the given values matched the content value,
                            // it means this item is matched.
                            boolean found = false;

                            String[] values = entry.getValue().split(SEP_COMMA);
                            for (String value : values) {
                                String regularExpression = RE_START
                                        + value
                                        + RE_END;
                                if (contentValue.matches(regularExpression)) {
                                    // If the value matched the content, set the found as true.
                                    found = true;
                                    if (contentValue.contains(SEP_COMMA)) {
                                        // if this content value contains many item, we will append
                                        // this matched value as actually value.
                                        // For example: *** Target="[value]"
                                        mContents = mContents + SEP_BLANK + patternKey + SEP_EQUAL
                                                + SEP_QUOTE + value + SEP_QUOTE;
                                    }
                                    break;
                                }
                            }
                            if (!found) {
                                // Couldn't find the matched value, set this item do not matched.
                                itemMatched = false;
                                break;
                            }
                        }
                    }

                    // If this patternItem matched, we will return the result as matched.
                    if (itemMatched) return true;
                }

                // It means all the pattern items do not matched the content,
                // we will set this carrier as do not matched and return the result.
                return false;
            }

            /**
             * @return the contents of this carrier.
             */
            public String getContents() {
                return mContents;
            }

            private void parseContentFromZipFile(JarFile file, JarEntry entry)
                    throws IOException {
                if (file == null || entry == null) return;

                // Get the content items.
                InputStream is = file.getInputStream(entry);
                InputStreamReader isReader = new InputStreamReader(is);
                BufferedReader bufReader = new BufferedReader(isReader);
                try {
                    // Read the first line, and it must be have one line to defined the items.
                    // And ignore anything else.
                    mContents = bufReader.readLine();
                } finally {
                    // Close the BufferedReader, InputStreamReader and InputStream.
                    if (bufReader != null) bufReader.close();
                    if (isReader != null) isReader.close();
                    if (is != null) is.close();
                }
                parseContent();
            }

            private void parseContentFromFile(File file) throws IOException {
                if (file == null) return;

                // Get the content items.
                FileReader fr = new FileReader(file);
                BufferedReader br = new BufferedReader(fr);
                try {
                    // Read the first line, and it must be have one line to defined the items.
                    // And ignore anything else.
                    mContents = br.readLine();
                } finally {
                    // Close the used FileReader and BufferedReader.
                    if (br != null) br.close();
                    if (fr != null) fr.close();
                }

                parseContent();
            }

            private void parseContent() {
                if (mContents == null || mContents.trim() == null) return;
                mContents = mContents.trim();

                // Build the contents' item.
                if (mContentItems == null) {
                    mContentItems = new HashMap<String, String>();
                }
                mContentItems.clear();

                // Items will be separated by bland(" ").
                String[] items = mContents.split(SEP_BLANK);
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
        }
    }

    /**
     * Return the current carrier's name.
     */
    private static final String SPEC_CONFIG_PATH = "/system/vendor/speccfg/";
    private static final String DEFAULT_SPEC_NAME = "Default";
    private static final String SPEC_FILE_NAME = "spec";
    private static final String CURRENT_CARRIERS_NUM_PRE = "packCount=";
    private static final String CURRENT_CARRIERS_NAME_PRE = "strSpec";
    private static final String SEP_UNDERLINE = "_";
    private static ArrayList<String> getCurrentCarriers() {

        File specFile = new File(SPEC_CONFIG_PATH + SPEC_FILE_NAME);

        ArrayList<String> Carriers = new ArrayList<String>();
        try {
            // If the file size is zero, we will return null as no content.
            if (getFileSize(specFile) == 0) {
                Carriers.add(DEFAULT_SPEC_NAME);
                return Carriers;
            }

            ArrayList<String> contents = readFile(specFile, null, false);

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

    public static String getCurrentCarriersDependency() {
        String currentCarriers = getCurrentCarriersName(getCurrentCarriers());
        String topCarrier = getTopCarrierName(currentCarriers);
        Log.d(TAG, "topCarrier = " + topCarrier);
        File currentSpecFile = new File(SYSTEM_VENDOR_PATH + File.separator + topCarrier
                + File.separator + SPEC_MARKED_FILE_NAME);
        try {
            CarrierFileFilter filter = new CarrierFileFilter(null, false, null);
            CarrierFileFilter.CarrierPattern pattern =
                    filter.new CarrierPattern(currentSpecFile);
            String dependencyString = pattern.getContentItems(FILTER_ITEM_DEPENDENCY);
            if (!TextUtils.isEmpty(dependencyString)) {
                String[] dependency = dependencyString.split("/");
                // Only return name, ignore hierarchy.
                if (dependency.length == 1) {
                    // Dependency="Name"
                    return dependency[0];
                } else {
                    // Dependency="Hierarchy/Name"
                    return dependency[1];
                }
            } else {
                return null;
            }
        } catch (IOException e) {
            Log.e(TAG, "CarrierPattern IOException:" + e.getMessage());
            return null;
        }
    }

    /**
     * To read the content from the given file. If the lines do not match the regular
     * expression, do not add to result.
     * @param regularExpression used to find the matched line, if it is null, do not check
     *                          if the line matched this expression.
     */
    private static ArrayList<String> readFile(File file, String regularExpression,
            boolean onlyReadFirstLine) {
        if (file == null || !file.exists() || !file.canRead()) return null;

        ArrayList<String> contents = new ArrayList<String>();

        FileReader fr = null;
        BufferedReader br = null;
        try {
            fr = new FileReader(file);
            br = new BufferedReader(fr);
            // Read the lines, and get the current carrier.
            String line = null;
            while ((line = br.readLine()) != null && (line = line.trim()) != null) {
                if (!TextUtils.isEmpty(regularExpression)) {
                    if (line.matches(regularExpression)) {
                        contents.add(line);
                    }
                } else {
                    contents.add(line);
                }
                if (onlyReadFirstLine) break;
            }
        } catch (IOException e) {
            Log.e(TAG, "Read File error, caused by: " + e.getMessage());
        } finally {
            try {
                if (br != null) br.close();
                if (fr != null) fr.close();
            } catch (IOException e) {
                Log.e(TAG, "Close the reader error, caused by: " + e.getMessage());
            }
        }

        return contents;
    }

    /**
     * Return the file size for the given file.
     */
    public static int getFileSize(File file) throws IOException {
        int size = 0;
        if (file != null && file.exists()) {
            FileInputStream fis = new FileInputStream(file);
            try {
                size = fis.available();
            } finally {
                if (fis != null) fis.close();
            }
        }
        return size;
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

    public static String getTopCarrierName(String fullname) {
        String[] name = fullname.split(SEP_UNDERLINE);
        return name[name.length - 1];
    }

    public static String downloadFile(String url) {
        HttpURLConnection connection = null;
        byte buffer[]= new byte[2048];
        InputStream in = null;
        FileOutputStream local = null;
        String otaFileName = null;
        try {
            long downloadedLength = 0;
            long fileLength = -1;
            URL mURL = new URL(url);
            connection = (HttpURLConnection)mURL.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000);
            fileLength = connection.getContentLength();

            connection.connect();

            in = connection.getInputStream();
            String realFileName = url.substring(url.lastIndexOf("/")+1);;
            otaFileName =  getPath(FLAG_STORAGE_SECONDARY) + "/" + realFileName;
            local = new FileOutputStream(otaFileName);

            int readLength = -1;

            while((readLength = in.read(buffer)) != -1){
                local.write(buffer, 0, readLength);
                downloadedLength += readLength;
            }
            connection.disconnect();
        } catch(MalformedURLException e) {
            e.printStackTrace();
        } catch(FileNotFoundException e) {
            e.printStackTrace();
        } catch(IOException e) {
            e.printStackTrace();
        } catch(Exception e){
            e.printStackTrace();
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (local != null) {
                try {
                    local.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            return otaFileName;
        }
    }
}
