/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.nfc.cardemulation;

import com.android.nfc.Debug;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.nfc.cardemulation.ApduServiceInfo;
import android.nfc.cardemulation.CardEmulation;
import android.util.Log;
import android.nfc.cardemulation.AidGroup;
import android.os.UserHandle;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import com.google.android.collect.Maps;
import android.util.SparseArray;
import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.PriorityQueue;
import java.util.TreeMap;
import com.android.nfc.QSecureElementManager;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlSerializer;
import android.util.AtomicFile;
import android.util.Xml;
import com.android.internal.util.FastXmlSerializer;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import android.content.Intent;

public class MultiSeManager {
    static final String TAG = "MultiSeManager";

    static final boolean DBG = true;
    static final String PPSE_AID = "325041592E5359532E4444463031";
    static final char MAX_DB_SZ = 40;

    byte [] mPpseResponse = null;

    final Context mContext;
    CardEmulationManager mCardEmulationManager;
    public RegisteredServicesCache mServiceCache;
    private static MultiSeManager mService;
    private static boolean mPpseRespSent = false;

    //NCI MULTI SE CONTROL SUB CMD
    static final int NCI_GET_MAX_PPSE_SZ = 0x00;
    static final int NCI_SET_PPSE_RSP    = 0x01;
    static final int NCI_CLR_PRV_PPSE_RSP = 0x02;
    static final int NCI_SET_DEFAULT_PPSE = 0x03;

    //Database Object
    class myDataBase{
        String aid;
        String seName;
        int pr;
        int pw;
    };

    static int mDbCount;
    static myDataBase[] mSeDataBase = new myDataBase[MAX_DB_SZ];

    static List<ApduServiceInfo> mValidServices = new ArrayList<ApduServiceInfo>();

    //OffHostService
    public class OffHostService {
        public ApduServiceInfo mApduServiceInfo;
        public int mBannerId;
        public final HashMap<String, String> mAidGroupDescriptions = Maps.newHashMap();

        OffHostService (ApduServiceInfo apduServiceInfo, int bannerId){
            this.mApduServiceInfo = apduServiceInfo;
            this.mBannerId = bannerId;
        }
    }
    //Services per package
    public class AppPackage{
        //SEName, OffHostService
        public final HashMap<String, OffHostService> mOffHostServices =  Maps.newHashMap();
    };
    //packages per user
    public class UserDatabase{
        //package name, AppPackage
        final HashMap<String, AppPackage> mAppPackages = Maps.newHashMap(); // Re-built at run-time
    };
    final SparseArray<UserDatabase> mUserDatabase = new SparseArray<UserDatabase>();

    static final String XML_INDENT_OUTPUT_FEATURE = "http://xmlpull.org/v1/doc/features.html#indent-output";
    final AtomicFile mOffHostServicesFile;

    public MultiSeManager(Context context, CardEmulationManager cardEmulationManager) {
        int userId = UserHandle.myUserId();
        mContext = context;
        mCardEmulationManager = cardEmulationManager;
        mServiceCache = cardEmulationManager.mServiceCache;

        mService = this;
        //Initialie the Database
        for (int i=0; i < MAX_DB_SZ; i++) {
            mSeDataBase[i] = new myDataBase();
        }

        File dataDir = mContext.getFilesDir();
        mOffHostServicesFile = new AtomicFile(new File(dataDir, "offhost_services.xml"));
        readOffHostServices();
        updateDataBase(userId);
    }

    public boolean multiSeRegisterAid(List<String> aids, ComponentName paymentService,
                                         List<String> seName, List<String> priority, List<String> powerState) {

        String category = CardEmulation.CATEGORY_PAYMENT;
        AidGroup aidGroup = new AidGroup(aids, category);
        int userId = UserHandle.myUserId();

        //dump received aid list & other
        if (DBG) Log.d(TAG,"print... the received list ");
        if (DBG) dumpList(aids,seName,priority,powerState);

        //save the received aid list & other to database & print
        if (DBG) Log.d(TAG,"save the list in database ");
        multiSeDataBase(aids,seName,priority,powerState);

        if (DBG) Log.d(TAG,"component info:" + paymentService);

        try {
            Log.d(TAG, "Register the AIDs");
            mCardEmulationManager.getNfcCardEmulationInterface().registerAidGroupForService(userId,paymentService,aidGroup);
        } catch(Exception e) {
            Log.e(TAG, "Register AID failed.");
        }

        return true;
    }


    public byte[] generatePpseResponse() {
        if (DBG) Log.d(TAG, "generatePpseResponse");

        int totalLength = 0;
        int appLabelLength = 0;
        String appLabel = null;
        String seName = null;
        String mseName = null;
        String mseCheck = null;
        String sPriority = null;
        boolean moreSecureElements = false;
        byte [] mPrioritycnt = new byte[] {(byte)0x01};

        ArrayList<String> aidsInPPSE = new ArrayList<String>();
        ArrayList<String> appLabels = new ArrayList<String>();
        ArrayList<String> priorities = new ArrayList<String>();

        mPpseResponse = null;

        // For each AID, find interested services
        for (Map.Entry<String, RegisteredAidCache.AidResolveInfo> aidEntry:
                mCardEmulationManager.mAidCache.mAidCache.entrySet()) {
            String aid = aidEntry.getKey();
            RegisteredAidCache.AidResolveInfo resolveInfo = aidEntry.getValue();
            ApduServiceInfo service;

            if (aid.equals(PPSE_AID)) {
                if (DBG) Log.d(TAG, "Skip PPSE AID");
                continue;
            }

            if (resolveInfo.defaultService != null) {
                // There is a default service set
                service = resolveInfo.defaultService;

                //Check if two/more SEs available for selected AIDs
                //send PPSE resp only if two/more SEs available
                mseName = resolveInfo.defaultService.getSeName();
                if(mseName.equals("MULTISE")) {
                    seName = getAidMappedSe(aid);
                    Log.d(TAG, "Selected SE: " + seName);
                    if ((moreSecureElements == false) && (mseCheck != null) &&
                        (mseCheck.equals(seName) == false)) {
                        moreSecureElements = true;
                        if (DBG) Log.d(TAG, "More SEs Selected, 1st: " + mseCheck + "  2nd: " + seName);
                    }
                    mseCheck = seName;
                }
                else
                    return mPpseResponse;

            } else if (resolveInfo.services.size() >= 1) {
                // More than one service
                service = resolveInfo.services.get(0);
            } else {
                continue;
            }

            // TO DO: May rework on priority assignment with super wallet
            appLabelLength = service.getDescription().length();
            appLabel = service.getDescription();

            mPrioritycnt[0] = (byte)(getAidMappedPriority(aid));
            sPriority = bytesToString(mPrioritycnt);

            // ((0x61 + LEN + (0x4F + LEN + (AID) + 0x50 + LEN + (LABEL) + 0x87 + LEN + (PRI))))
            // PPSE entry size = 9 bytes of constants in header data + hex encoded aid payload + label and (255 + 4) payload and TLV size.
            // 9

            if ((totalLength + 9 + (aid.length()/2) + appLabelLength) > (1024)) {
                if (DBG) Log.d(TAG, "Too much data for PPSE response");
                break;
            }


            aidsInPPSE.add(aid);
            appLabels.add(appLabel);
            priorities.add(sPriority);

            totalLength += 9 + (aid.length()/2) + appLabelLength;
        }

        if ((aidsInPPSE.size() == 0) || (moreSecureElements == false)) {
            if (DBG) Log.d(TAG, "No AID for PPSE Response");
            return mPpseResponse;
        }

        int nextWrite = 0;
        mPpseResponse = new byte[totalLength];

        // adding AID/Priority
        for (int xx = 0; xx < aidsInPPSE.size(); xx++) {

            String aid = aidsInPPSE.get(xx);
            byte[] aidBytes = hexStringToByteArray(aid);

            appLabel = appLabels.get(xx);
            if (appLabel == null)
                appLabelLength = 0;
            else
                appLabelLength = appLabel.length();

            sPriority = priorities.get(xx);
            byte[] priorityBytes = hexStringToByteArray(sPriority);
            int priority = priorityBytes[0];

            if (DBG) Log.d(TAG, "AID = " + aid + ", appLabel = " + appLabel + ", priority = " + priority);

            // (0x61 + LEN + (0x4F + LEN + (AID) + 0x50 + LEN + (LABEL) + 0x87 + LEN + (PRI)))
            // FCI Template
            //
            // Directory Entry
            mPpseResponse[nextWrite++] = (byte)0x61;
            mPpseResponse[nextWrite++] = (byte)(7 + aidBytes.length + appLabelLength);
            // DF Name (AID)
            mPpseResponse[nextWrite++] = (byte)0x4F;
            mPpseResponse[nextWrite++] = (byte)(aidBytes.length);
            for (int i = 0; i < aidBytes.length; i++) {
                mPpseResponse[nextWrite++] = aidBytes[i];
            }
            // Application Label
            mPpseResponse[nextWrite++] = (byte)0x50;
            mPpseResponse[nextWrite++] = (byte)appLabelLength;
            for (int i = 0; i < appLabelLength; i++) {
                mPpseResponse[nextWrite++] = (byte)appLabel.charAt(i);
            }
            // Priority Indicator
            mPpseResponse[nextWrite++] = (byte)0x87;
            mPpseResponse[nextWrite++] = (byte)0x01;
            mPpseResponse[nextWrite++] = (byte)priority;
        }

        if (DBG) Log.d(TAG, " mPpseResponse = 0x" + bytesToString(mPpseResponse));
        return mPpseResponse;
    }

    String dumpEntry(Map.Entry<String, RegisteredAidCache.AidResolveInfo> entry) {
        StringBuilder sb = new StringBuilder();
        sb.append("    \"" + entry.getKey() + "\"\n");
        ApduServiceInfo defaultServiceInfo = entry.getValue().defaultService;
        ComponentName defaultComponent = defaultServiceInfo != null ?
                defaultServiceInfo.getComponent() : null;

        for (ApduServiceInfo serviceInfo : entry.getValue().services) {
            sb.append("        ");
            if (serviceInfo.getComponent().equals(defaultComponent)) {
                sb.append("*DEFAULT* ");
            }
            sb.append(serviceInfo.getComponent() +
                    " (Description: " + serviceInfo.getDescription() + ")\n");
        }
        return sb.toString();
    }

    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        pw.println("    AID cache entries: ");
        for (Map.Entry<String, RegisteredAidCache.AidResolveInfo> entry : mCardEmulationManager.mAidCache.mAidCache.entrySet()) {
            pw.println(dumpEntry(entry));
        }
        //pw.println("    Service preferred by foreground app: " + mPreferredForegroundService);
        //pw.println("    Preferred payment service: " + mPreferredPaymentService);
        pw.println("");
        //mRoutingManager.dump(fd, pw, args);
        pw.println("");
    }


    static byte[] hexStringToByteArray(String s) {
       int len = s.length();
       byte[] data = new byte[len / 2];
       for (int i = 0; i < len; i += 2) {
           data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                                + Character.digit(s.charAt(i+1), 16));
       }
       return data;
    }

    public void processPpse() {
       byte [] ppse_rsp = generatePpseResponse();
       if (ppse_rsp != null) {
           if (DBG) Log.d(TAG,"Sending PPSE response");
           QSecureElementManager.getInstance().multiSeControlCmd(ppse_rsp,NCI_SET_PPSE_RSP);
           mPpseRespSent = true;
       } else if(mPpseRespSent) {
           if (DBG) Log.d(TAG,"Clear previous PPSE response");
           QSecureElementManager.getInstance().multiSeControlCmd(ppse_rsp,NCI_CLR_PRV_PPSE_RSP);
           mPpseRespSent = false;
       }
    }

    static boolean dumpList(List<String> aids, List<String> seName, List<String> priority, List<String> powerState) {

        Log.d(TAG,"dump_list: aid size =" + aids.size());

        for (String aid : aids) {
            Log.d(TAG, "Aid:" + aid);
        }
        for (String se : seName) {
            Log.d(TAG, "SE:" + se);
        }
        for (String pr : priority) {
            Log.d(TAG, "Priority:" + pr);
        }
        for (String pw : powerState) {
            Log.d(TAG, "PowerState:" + pw);
        }
        return true;
    }

    static boolean multiSeDataBase(List<String> aids, List<String> seName, List<String> priority, List<String> powerState) {
        int i = 0;
        mDbCount = aids.size();
        if (mDbCount > MAX_DB_SZ)
            mDbCount = MAX_DB_SZ;

        i=0;
        for (String aid : aids) {
            mSeDataBase[i++].aid = aid;
            if (i >= (MAX_DB_SZ - 1))
                break;
        }

        i=0;
        for (String se : seName) {
            mSeDataBase[i++].seName = se;
            if (i >= (MAX_DB_SZ - 1))
                break;
        }

        i=0;
        for (String pr : priority) {
            mSeDataBase[i++].pr = Integer.parseInt(pr);
            if (i >= (MAX_DB_SZ - 1))
                break;
        }

        i=0;
        for (String pw : powerState) {
            mSeDataBase[i++].pw = Integer.parseInt(pw);
            if (i >= (MAX_DB_SZ - 1))
                break;
        }

        //print the saveddata base
        if(DBG) {
            Log.d(TAG, "Print the stored data base");
            for(i=0; i < mDbCount; i++) {
                Log.d(TAG, "aid: " +  mSeDataBase[i].aid + "  SE:" + mSeDataBase[i].seName +
                          "  Priority:" + mSeDataBase[i].pr + "  PowerState:" + mSeDataBase[i].pw);
            }
        }
        return true;
    }

    static String getAidMappedSe(String aids) {
        int i = 0;
        if (DBG) Log.d(TAG, "Find the SE name from the given AID");

        for(i=0; i < mDbCount; i++) {
            if(aids.equals(mSeDataBase[i].aid))
                return mSeDataBase[i].seName;
        }
        return null;
    }

    static int getAidMappedPriority(String aids) {
        int i = 0;
        if (DBG) Log.d(TAG, "Find the Priority from the given AID");

        for(i=0; i < mDbCount; i++) {
            if(aids.equals(mSeDataBase[i].aid))
                return mSeDataBase[i].pr;
        }
        return 0;
    }

    static int getAidMappedPowerState(String aids) {
        int i = 0;
        if (DBG) Log.d(TAG, "Find the Priority from the given AID");
        for(i=0; i < mDbCount; i++) {
            if(aids.equals(mSeDataBase[i].aid))
                return mSeDataBase[i].pw;
        }
        return 0;
    }

    static String bytesToString(byte[] bytes) {
        final char[] hexChars = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
        char[] chars = new char[bytes.length * 2];
        int byteValue;
        for (int j = 0; j < bytes.length; j++) {
            byteValue = bytes[j] & 0xFF;
            chars[j * 2] = hexChars[byteValue >>> 4];
            chars[j * 2 + 1] = hexChars[byteValue & 0x0F];
        }
        return new String(chars);
    }

    public static MultiSeManager getInstance() {
        if (DBG) Log.d(TAG, "getInstance");
        return mService;
    }

    public boolean getMultiSeServices(ArrayList<ApduServiceInfo> validServices) {
        if (DBG) Log.d(TAG, "getMultiSeServices()");
        int userId = UserHandle.myUserId();

        if (checkAndUpdateInstalledPackages(userId)) {
            updateDataBase(userId);
            writeOffHostServices();
        }

        //save service into database;
        for (ApduServiceInfo apduServiceInfo : mValidServices) {
            if (DBG) Log.d(TAG, "getMultiSeServices() add: " + apduServiceInfo.getComponent());
            validServices.add(apduServiceInfo);
        }

        return true;
    }

    private UserDatabase findOrCreateUserDatabase(int userId) {
        UserDatabase userDatabase = mUserDatabase.get(userId);
        if (userDatabase == null) {
            userDatabase = new UserDatabase();
            mUserDatabase.put(userId, userDatabase);
        }
        return userDatabase;
    }

    private AppPackage findOrCreateUserPackage(int userId, String packageName) {
        UserDatabase userDatabase = findOrCreateUserDatabase(userId);
        AppPackage appPackage = userDatabase.mAppPackages.get(packageName);
        if (appPackage == null) {
            appPackage = new AppPackage();
            userDatabase.mAppPackages.put(packageName, appPackage);
        }
        return appPackage;
    }

    private OffHostService getOffHostService(int userId, String packageName, String seName) {
        UserDatabase userDatabase = mUserDatabase.get(userId);
        if (userDatabase != null) {
            AppPackage appPackage = userDatabase.mAppPackages.get(packageName);
            if (appPackage != null) {
                OffHostService offHostService = appPackage.mOffHostServices.get(seName);
                return offHostService;
            }
        }
        return null;
    }

    private OffHostService findOrCreateUserOffHostService(int userId, String packageName, String seName,
                                                              ApduServiceInfo apduServiceInfo, int bannerId) {
        AppPackage appPackage = findOrCreateUserPackage(userId, packageName);
        OffHostService offHostService = appPackage.mOffHostServices.get(seName);
        if (offHostService == null) {
            offHostService = new OffHostService(apduServiceInfo, bannerId);
            appPackage.mOffHostServices.put(seName, offHostService);
        }
        return offHostService;
    }

    public AppPackage getOffHostServiceForPackage(String packageName) {
        if (DBG) Log.d(TAG, "getOffHostServiceForPackage()" + packageName);

        int userId = UserHandle.myUserId();
        UserDatabase userDatabase = mUserDatabase.get(userId);
        if (userDatabase != null) {
            AppPackage appPackage = userDatabase.mAppPackages.get(packageName);
            return appPackage;
        }
        return null;
    }

    private boolean checkAndUpdateInstalledPackages(int userId) {
        boolean isUpdated = false;
        UserDatabase userDatabase = mUserDatabase.get(userId);
        if (userDatabase == null)
            return isUpdated;

        PackageManager pm;
        try {
            pm = mContext.createPackageContextAsUser("android", 0,
                    new UserHandle(userId)).getPackageManager();
        } catch (Exception e) {
            Log.e(TAG, "checkAndUpdateInstalledPackages(): Could not create user package context");
            return isUpdated;
        }

        Set<String> packageNames = userDatabase.mAppPackages.keySet();
        for(String packageName : packageNames) {
            Intent intent = pm.getLaunchIntentForPackage(packageName);
            if(intent == null) {
                if (DBG) Log.d(TAG, "checkAndUpdateInstalledPackages(): remove " + packageName);
                userDatabase.mAppPackages.remove(packageName);
                isUpdated = true;
            }
        }
        return isUpdated;
    }

    public void updateDataBase(int userId) {
        if (DBG) Log.d(TAG, "updateDataBase()");
        UserDatabase userDatabase = mUserDatabase.get(userId);
        if (userDatabase == null)
            return;

        mValidServices.clear();
        Set<String> packageNames = userDatabase.mAppPackages.keySet();
        for(String packageName : packageNames)
        {
            if (DBG) Log.d(TAG, "updateDataBase() PackageName : " + packageName);
            AppPackage appPackage = userDatabase.mAppPackages.get(packageName);
            if (appPackage != null) {
                Set<String> seNames = appPackage.mOffHostServices.keySet();
                for(String seName : seNames) {
                    OffHostService offHostService = getOffHostService(userId, packageName, seName);
                    if(offHostService != null) {
                        if (DBG) Log.d(TAG, "updateDataBase() SEName : " + seName);
                        mValidServices.add(offHostService.mApduServiceInfo);
                    } else {
                        Log.e(TAG, "updateDataBase() : OffHostService is not found for " + seName);
                    }
                }
            }
        }
    }

    public boolean deleteOffHostService(String packageName, String seName) {
        if (DBG) Log.d(TAG, "deleteOffHostService() " + packageName + ", " + seName);

        int userId = UserHandle.myUserId();
        UserDatabase userDatabase = mUserDatabase.get(userId);
        if (userDatabase != null) {
            AppPackage appPackage = userDatabase.mAppPackages.get(packageName);
            if (appPackage != null) {
                if (appPackage.mOffHostServices.remove(seName) != null) {
                    updateDataBase(userId);
                    mServiceCache.invalidateCache(userId);
                    writeOffHostServices();
                    return true;
                }
            }
        }
        return false;
    }

    public ApduServiceInfo checkAidGroupToRemove(ApduServiceInfo serviceInfo, int userId,
                                                 int uid, List<AidGroup> aidGroups) {
        ComponentName componentName = serviceInfo.getComponent();
        List<String> removedGroups = new ArrayList<String>();
        removedGroups.add(CardEmulation.CATEGORY_PAYMENT);
        removedGroups.add(CardEmulation.CATEGORY_OTHER);

        for(AidGroup aidGroup : aidGroups) {
            removedGroups.remove(aidGroup.getCategory());
        }

        for(String removeCategory : removedGroups) {
             if (DBG) Log.d(TAG, "checkAidGroupToRemove() remove AidGroup for " + removeCategory);
             mServiceCache.removeAidGroupForService(userId, uid, componentName, removeCategory);
             serviceInfo.removeDynamicAidGroupForCategory(removeCategory);
        }
        return serviceInfo;
    }

    public ApduServiceInfo CreateApduServiceInfo(String packageName, String seName, String description,
                                 int bannerResId, int uid, List<String> aidGroupDescriptions,
                                 List<AidGroup> aidGroups) {

        if (DBG) Log.d(TAG, "CreateApduServiceInfo()");
        ResolveInfo resolveInfo = new ResolveInfo();
        resolveInfo.serviceInfo = new ServiceInfo();
        resolveInfo.serviceInfo.applicationInfo = new ApplicationInfo();
        resolveInfo.serviceInfo.packageName = packageName;
        if(seName.equals("SIM"))
            seName = "SIM1";
        resolveInfo.serviceInfo.name = seName;
        ArrayList<AidGroup> staticAidGroups = new ArrayList<AidGroup>();
        ArrayList<AidGroup> dynamicAidGroups = new ArrayList<AidGroup>();
        int index = 0;
        for (AidGroup aidGroup : aidGroups) {
            try {
                dynamicAidGroups.add(aidGroup);
                if (DBG) Log.d(TAG, "CreateApduServiceInfo() AidGroup:" + aidGroup.getCategory() + ", "
                                                                       + aidGroupDescriptions.get(index));
            } catch (Exception e) {
                Log.e(TAG, "CreateApduServiceInfo(): " + e.getMessage());
            }
            index++;
        }
        ApduServiceInfo apduServiceInfo = new ApduServiceInfo(resolveInfo,
                                                              false, // onHost
                                                              description,
                                                              staticAidGroups,
                                                              dynamicAidGroups,
                                                              true,  // requiresUnlock
                                                              bannerResId,
                                                              uid,
                                                              seName);
        if (DBG) Log.d(TAG, "ApduServiceInfo:" + apduServiceInfo.toString());
        return apduServiceInfo;
    }

    public boolean multiSeCommit(String packageName, String seName, String description,
                                 int bannerResId, int uid, List<String> aidGroupDescriptions,
                                 List<AidGroup> aidGroups) {
        if (DBG) Log.d(TAG, "multiSeCommit() packageName: " + packageName + ", SE: " + seName);

        int index = 0;
        int userId = UserHandle.myUserId();
        ApduServiceInfo serviceInfo = null;
        boolean newApduService = false;
        UserDatabase userDatabase = findOrCreateUserDatabase(userId);
        AppPackage appPackage = findOrCreateUserPackage(userId, packageName);
        OffHostService offHostService = getOffHostService(userId, packageName, seName);

        if (offHostService == null) {
            if (DBG) Log.d(TAG, "multiSeCommit() create new service");
            //create ApduServiceInfo
            serviceInfo = CreateApduServiceInfo(packageName, seName, description,
                                                bannerResId, uid, aidGroupDescriptions,
                                                aidGroups);
            offHostService = findOrCreateUserOffHostService(userId, packageName, seName, serviceInfo, bannerResId);
            newApduService = true;
        } else {
            if (DBG) Log.d(TAG, "multiSeCommit() update service");
            //check if bannerResId is changed
            if(offHostService.mBannerId != bannerResId) {
                if (DBG) Log.d(TAG, "multiSeCommit() banner ID changed");
                serviceInfo = CreateApduServiceInfo(packageName, seName, description,
                                                    bannerResId, uid, aidGroupDescriptions,
                                                    aidGroups);

                offHostService.mApduServiceInfo = serviceInfo;
                offHostService.mBannerId = bannerResId;

                for (ApduServiceInfo apduServiceInfo : mValidServices) {
                    if (apduServiceInfo.getComponent() == serviceInfo.getComponent()) {
                        apduServiceInfo = serviceInfo;
                        break;
                    }
                }
                //add service with new banner ID to cardemulation
                mServiceCache.AddDynamicServices(userId, serviceInfo.getComponent(), serviceInfo);
            }
            //check & remove if any AidGroup is removed in wallet
            offHostService.mApduServiceInfo = checkAidGroupToRemove(offHostService.mApduServiceInfo, userId, uid, aidGroups);
            serviceInfo = offHostService.mApduServiceInfo;
        }

        offHostService.mAidGroupDescriptions.clear();
        for (AidGroup aidGroup : aidGroups) {
           offHostService.mAidGroupDescriptions.put(aidGroup.getCategory(), aidGroupDescriptions.get(index));
           offHostService.mApduServiceInfo.setOrReplaceDynamicAidGroup(aidGroup);
           index++;
        }

        //update/save offHostService
        appPackage.mOffHostServices.put(seName, offHostService);

        //update or save Offhostservices
        userDatabase.mAppPackages.put(packageName, appPackage);

        if (newApduService) {
            if (DBG) Log.d(TAG, "multiSeCommit() add component : " + serviceInfo.getComponent());
            mValidServices.add(serviceInfo);
            //add ApduServiceInfo to cardemulation database
            mServiceCache.AddDynamicServices(userId, serviceInfo.getComponent(), serviceInfo);
        }

        for (AidGroup aidGroup : aidGroups) {
            try {
                if (DBG) Log.d(TAG, "multiSeCommit() register the AidGroup(" + aidGroup.getCategory() +
                                    ") for :" + serviceInfo.getComponent());
                mServiceCache.registerAidGroupForService(userId, uid,
                                                         serviceInfo.getComponent(),
                                                         aidGroup);
            } catch(Exception e) {
                Log.e(TAG, "multiSeCommit() register AID failed.");
            }
        }

        writeOffHostServices();
        return true;
    }

    private boolean writeOffHostServices() {
        if (DBG) Log.d(TAG, "writeOffHostServices()");
        FileOutputStream fos = null;
        try {
            fos = mOffHostServicesFile.startWrite();
            XmlSerializer out = new FastXmlSerializer();
            out.setOutput(fos, "utf-8");
            out.startDocument(null, true);
            out.setFeature(XML_INDENT_OUTPUT_FEATURE, true);

            out.startTag(null, "users");
            out.attribute(null, "num_of_users", Integer.toString(mUserDatabase.size()));
            for (int i = 0; i < mUserDatabase.size(); i++) {
                if(DBG) Log.d(TAG, "writeOffHostServices(): user_id = " + Integer.toString(mUserDatabase.keyAt(i)));

                out.startTag(null, "user");
                out.attribute(null, "user_id", Integer.toString(mUserDatabase.keyAt(i)));
                final UserDatabase userDatabase = mUserDatabase.valueAt(i);
                out.attribute(null, "num_of_packages", Integer.toString(userDatabase.mAppPackages.size()));

                for (Map.Entry<String, AppPackage> appPackageEntry : userDatabase.mAppPackages.entrySet()) {
                    if(DBG) Log.d(TAG, "writeOffHostServices(): package = " + appPackageEntry.getKey());

                    out.startTag(null, "package");
                    out.attribute(null, "package_name", appPackageEntry.getKey());
                    final AppPackage appPackage = appPackageEntry.getValue();
                    out.attribute(null, "num_of_apdu_services", Integer.toString(appPackage.mOffHostServices.size()));

                    for (Map.Entry<String, OffHostService> offHostServiceEntry : appPackage.mOffHostServices.entrySet()) {
                        final OffHostService offHostService = offHostServiceEntry.getValue();
                        if(DBG) Log.d(TAG, "writeOffHostServices(): apdu_service on " + offHostServiceEntry.getKey());

                        out.startTag(null, "apdu_service");
                        out.attribute(null, "se_name", offHostServiceEntry.getKey());
                        out.attribute(null, "description", offHostService.mApduServiceInfo.getDescription());
                        out.attribute(null, "banner_id", Integer.toString(offHostService.mBannerId));
                        out.attribute(null, "uid", Integer.toString(offHostService.mApduServiceInfo.getUid()));
                        out.attribute(null, "num_of_aid_groups", Integer.toString(offHostService.mAidGroupDescriptions.size()));
                        for (AidGroup aidGroup : offHostService.mApduServiceInfo.getAidGroups()) {
                            if(DBG) Log.d(TAG, "writeOffHostServices(): aid_group = {" + aidGroup.getCategory() + "}, {"
                                                           + offHostService.mAidGroupDescriptions.get(aidGroup.getCategory()) + "}");
                            out.startTag(null, "aid_group");
                            out.attribute(null, "category", aidGroup.getCategory());
                            out.attribute(null, "description", offHostService.mAidGroupDescriptions.get(aidGroup.getCategory()));
                            for (String aid : aidGroup.getAids()) {
                                if(DBG) Log.d(TAG, "writeOffHostServices(): aid = " + aid);
                                out.startTag(null, "aid");
                                out.attribute(null, "value", aid);
                                out.endTag(null, "aid");
                            }
                            out.endTag(null, "aid_group");
                        }
                        out.endTag(null, "apdu_service");
                    }
                    out.endTag(null, "package");
                }
                out.endTag(null, "user");
            }
            out.endTag(null, "users");

            out.endDocument();
            mOffHostServicesFile.finishWrite(fos);
            return true;
        } catch (Exception e) {
            Log.e(TAG, "Error writing OffHostServices", e);
            if (fos != null) {
                mOffHostServicesFile.failWrite(fos);
            }
            return false;
        }
    }

    private void createApduServiceFromXml(XmlPullParser parser, String packageName, AppPackage appPackage)
                        throws XmlPullParserException, IOException {
        if(DBG) Log.d(TAG, "createApduServiceFromXml()");

        boolean foundAttribute = false;
        String seName = null;
        String description = null;
        int bannerId = -1;
        int uid = -1;
        List<String> aidGroupDescriptions = new ArrayList<String>();
        List<AidGroup> aidGroups = new ArrayList<AidGroup>();

        ApduServiceInfo apduServiceInfo = null;
        OffHostService offHostService = null;

        int eventType = parser.getEventType();
        int minDepth = parser.getDepth();

        while (eventType != XmlPullParser.START_TAG &&
               eventType != XmlPullParser.END_DOCUMENT &&
               parser.getDepth() >= minDepth) {
            eventType = parser.next();
        }

        while (eventType != XmlPullParser.END_DOCUMENT && parser.getDepth() >= minDepth) {
            String tagName = parser.getName();
            if (eventType == XmlPullParser.START_TAG) {
                if ("apdu_service".equals(tagName)) {
                    seName = parser.getAttributeValue(null, "se_name");
                    description = parser.getAttributeValue(null, "description");
                    String bannerIdString = parser.getAttributeValue(null, "banner_id");
                    bannerId = Integer.parseInt(bannerIdString);
                    String uidString = parser.getAttributeValue(null, "uid");
                    uid = Integer.parseInt(uidString);
                    String numAidGroupsString = parser.getAttributeValue(null, "num_of_aid_groups");
                    int numAidGroups = Integer.parseInt(numAidGroupsString);

                    if (DBG) Log.d(TAG, "createApduServiceFromXml(): seName = " + seName +
                                                                ", description = " + description +
                                                                ", numAidGroups = " + numAidGroups);
                    String category = null;
                    String groupDescription = null;
                    ArrayList<String> aidList = new ArrayList<String>();

                    eventType = parser.next();
                    while (eventType != XmlPullParser.END_DOCUMENT && parser.getDepth() >= minDepth) {
                        tagName = parser.getName();
                        if (eventType == XmlPullParser.START_TAG) {
                            if ("aid_group".equals(tagName)) {
                                category = parser.getAttributeValue(null, "category");
                                groupDescription = parser.getAttributeValue(null, "description");

                                if (DBG) Log.d(TAG, "createApduServiceFromXml(): category = " + category +
                                                                ", groupDescription = " + groupDescription);
                            } else if ("aid".equals(tagName)) {
                                String aid = parser.getAttributeValue(null, "value");
                                aidList.add(aid.toUpperCase());
                            } else {
                                if (DBG) Log.d(TAG, "createApduServiceFromXml(): Ignoring unexpected START_TAG: " + tagName);
                            }
                        } else if (eventType == XmlPullParser.END_TAG) {
                            if ("aid_group".equals(tagName)) {
                                if (aidList.size() > 0) {
                                    aidGroupDescriptions.add(groupDescription);
                                    AidGroup aidGroup = new AidGroup(aidList, category);
                                    aidGroups.add(aidGroup);
                                    aidList.clear();
                                }
                                numAidGroups--;
                                if (numAidGroups == 0)
                                    break;
                            } else if ("aid".equals(tagName)) {
                            } else {
                                if (DBG) Log.d(TAG, "createApduServiceFromXml(): Ignoring unexpected END_TAG: " + tagName);
                            }
                        }
                        eventType = parser.next();
                    }
                    foundAttribute = true;
                } else {
                    if (DBG) Log.d(TAG, "createApduServiceFromXml(): Ignoring unexpected START_TAG: " + tagName);
                }
            } else if (eventType == XmlPullParser.END_TAG) {
                if ("apdu_service".equals(tagName)) {
                    if (foundAttribute) {
                        apduServiceInfo = CreateApduServiceInfo(packageName, seName, description,
                                                bannerId, uid, aidGroupDescriptions,
                                                aidGroups);

                        offHostService = new OffHostService(apduServiceInfo, bannerId);
                        for (int xx = 0; xx < aidGroups.size(); xx++) {
                            offHostService.mAidGroupDescriptions.put(aidGroups.get(xx).getCategory(), aidGroupDescriptions.get(xx));
                        }
                        appPackage.mOffHostServices.put(seName, offHostService);
                    }
                    break;
                } else {
                    if (DBG) Log.d(TAG, "createApduServiceFromXml(): Ignoring unexpected END_TAG: " + tagName);
                }
            }
            eventType = parser.next();
        }
    }

    private void createOffHostServicesFromXml(XmlPullParser parser, UserDatabase userDatabase)
                         throws XmlPullParserException, IOException {
        if (DBG) Log.d(TAG, "createOffHostServicesFromXml()");
        String packageName = null;
        AppPackage appPackage = null;

        int eventType = parser.getEventType();
        int minDepth = parser.getDepth();

        while (eventType != XmlPullParser.START_TAG &&
               eventType != XmlPullParser.END_DOCUMENT &&
               parser.getDepth() >= minDepth) {
            eventType = parser.next();
        }

        while (eventType != XmlPullParser.END_DOCUMENT && parser.getDepth() >= minDepth) {
            String tagName = parser.getName();
            if (eventType == XmlPullParser.START_TAG) {
                if ("package".equals(tagName)) {
                    packageName = parser.getAttributeValue(null, "package_name");
                    String numApduServicesString = parser.getAttributeValue(null, "num_of_apdu_services");
                    int numApduServices = Integer.parseInt(numApduServicesString);

                    if (DBG) Log.d(TAG, "createOffHostServicesFromXml(): packageName = " + packageName +
                                                                ", numApduServices = " + numApduServices);

                    appPackage = new AppPackage();
                    for (int xx = 0; xx < numApduServices; xx++) {
                        parser.next();
                        createApduServiceFromXml(parser, packageName, appPackage);
                    }
                    userDatabase.mAppPackages.put(packageName, appPackage);
                } else {
                    if (DBG) Log.d(TAG, "createOffHostServicesFromXml(): Ignoring unexpected START_TAG: " + tagName);
                }
            } else if (eventType == XmlPullParser.END_TAG) {
                if ("package".equals(tagName)) {
                    break;
                } else {
                    if (DBG) Log.d(TAG, "createOffHostServicesFromXml(): Ignoring unexpected END_TAG: " + tagName);
                }
            }
            eventType = parser.next();
        }
    }

    private void createUserDataBaseFromXml(XmlPullParser parser)
                         throws XmlPullParserException, IOException {
        if (DBG) Log.d(TAG, "createUserDataBaseFromXml()");

        int userId = -1;
        UserDatabase userDatabase = null;

        int eventType = parser.getEventType();
        int minDepth = parser.getDepth();

        while (eventType != XmlPullParser.START_TAG &&
               eventType != XmlPullParser.END_DOCUMENT &&
               parser.getDepth() >= minDepth) {
            eventType = parser.next();
        }

        while (eventType != XmlPullParser.END_DOCUMENT && parser.getDepth() >= minDepth) {
            String tagName = parser.getName();
            if (eventType == XmlPullParser.START_TAG) {
                if ("user".equals(tagName)) {
                    userDatabase = new UserDatabase();
                    String userIdString = parser.getAttributeValue(null, "user_id");
                    userId = Integer.parseInt(userIdString);
                    String numPackagesString = parser.getAttributeValue(null, "num_of_packages");
                    int numPackages = Integer.parseInt(numPackagesString);

                    if (DBG) Log.d(TAG, "createUserDataBaseFromXml(): userId = " + userId +
                                                          ", numPackages = " + numPackages);

                    parser.next();
                    for (int xx = 0; xx < numPackages; xx++) {
                        createOffHostServicesFromXml(parser, userDatabase);
                    }
                } else {
                    if (DBG) Log.d(TAG, "createUserDataBaseFromXml(): Ignoring unexpected START_TAG: " + tagName);
                }
            } else if (eventType == XmlPullParser.END_TAG) {
                if ("user".equals(tagName)) {
                    if ((userId != -1)&&(userDatabase != null)) {
                        mUserDatabase.put(userId, userDatabase);
                    }
                    break;
                } else {
                    if (DBG) Log.d(TAG, "createUserDataBaseFromXml(): Ignoring unexpected END_TAG: " + tagName);
                }
            }
            eventType = parser.next();
        }
    }

    private void readOffHostServices() {
        if (DBG) Log.d(TAG, "readOffHostServices()");
        FileInputStream fis = null;
        try {
            if (!mOffHostServicesFile.getBaseFile().exists()) {
                Log.d(TAG, "OffHostServices file does not exist.");
                return;
            }
            fis = mOffHostServicesFile.openRead();
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(fis, null);

            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.START_TAG &&
                    eventType != XmlPullParser.END_DOCUMENT) {
                eventType = parser.next();
            }

            String tagName = parser.getName();
            if ("users".equals(tagName)) {
                String numUsersString = parser.getAttributeValue(null, "num_of_users");
                int numUsers = Integer.parseInt(numUsersString);

                for (int xx = 0; xx < numUsers; xx++) {
                    parser.next();
                    createUserDataBaseFromXml(parser);
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Could not parse OffHostServices file, trashing.");
            mOffHostServicesFile.delete();
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                }
            }
        }
    }
}
