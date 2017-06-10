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

package com.android.backup.vcalendar;

import android.provider.CalendarContract.Events;
import com.android.backup.vcalendar.VCalendarManager.EventCount;

import java.lang.Boolean;
import java.io.IOException;
import android.os.Handler;
import android.os.Process;
import android.content.ActivityNotFoundException;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import android.content.Context;
import java.io.File;
import android.content.ContentResolver;
import android.util.Log;
import java.util.ArrayList;
import com.android.backup.vcalendar.VCalendarManager;
import android.util.Config;
import java.util.ArrayList;
import java.util.Arrays;

import static com.android.backup.BackupUtils.BACKUP_TYPE_CALENDAR;
import static com.android.backup.BackupUtils.EVENT_INIT_PROGRESS_TITLE;
import static com.android.backup.BackupUtils.EVENT_SET_PROGRESS_VALUE;

/*****
 *** :zhl add :2009-11-11 :support to import vcs file to clendar database
 *****/
public class VcalendarImporter {

    public static final String TAG = "VcalendarImporter";
    private Boolean mCancel = false;
    private Context mContext = null;
    private VCalendarManager mVCalManager = null;
    private Handler mCallBackHandler = null;
    private OnVcsInsertListener mOnInsertListener;

    public static final int CALENDAR_MEMO_FULL = 0x00000002;
    public static final int CALENDAR_APPOINTMENT_FULL = 0x00000004;
    public static final int CALENDAR_ANNIVERSARY_FULL = 0x00000008;
    public static final int CALENDAR_TASK_FULL = 0x00000010;

    private static final String BEGIN = "BEGIN";
    private static final String END = "END";
    private static final String NEWLINE = "\n";
    public static final String VCALENDAR = "VCALENDAR";
    public static final String VEVENT = "VEVENT";
    public static final String VTODO = "VTODO";
    public static final String VJOURNAL = "VJOURNAL";
    public static final String VFREEBUSY = "VFREEBUSY";
    public static final String VTIMEZONE = "VTIMEZONE";
    public static final String VALARM = "VALARM";

    private static String CRLF = "\r\n";

    public VcalendarImporter(Context context, OnVcsInsertListener insertListener) {

        mContext = context;
        mOnInsertListener = insertListener;

    }

    public VcalendarImporter(Context context, OnVcsInsertListener insertListener, Handler handler) {
        mContext = context;
        mOnInsertListener = insertListener;
        mCallBackHandler = handler;
    }

    /**
     * The callback interface used to notify the progress
     */
    public interface OnVcsInsertListener {

        void importProgressSet(int item);
    }

    private Boolean getCancel()
    {
        return mCancel;
    }

    /*
     * return : 1 success 0 cancel -1 error 2 memo full,3 appointment full,4
     * anniversary full
     */
    public final int insertFromFile(File file) {

        int ret = -1;

        if (!isVcsFile(file))
        {
            return ret;
        }

        ContentResolver resolver = mContext.getContentResolver();

        boolean[] fullResult = {
                false, false, false, false
        };
        File opFile = file;

        try {
            String rct = null;
            // String filename = "/sdcard/" +/* vm.getName() +*/
            // "exportcontact.vcf";;

            FileInputStream f = new FileInputStream(opFile);
            BufferedReader d = new BufferedReader(new InputStreamReader(f));
            String str = d.readLine();

            int insertIndex = 0;

            boolean inVevent = false;
            boolean inVtodo = false;
            boolean inVcal = false;
            boolean inTZ = false;
            String vcalHead = "";
            String vTimeZone = "";
            String vEventString = "";
            String vTodoString = "";
            String parseString = "";

            while (str != null) {

                if (true == getCancel())
                {
                    f.close();
                    d.close();
                    ret = 0;
                    return ret;
                }

                if (!inVcal)
                {
                    // grab the name
                    String normalStr = normalizeText(str.substring(0));

                    char c = 0;
                    int len1 = normalStr.length();

                    if (len1 == 0)
                    {
                        str = d.readLine();
                        continue;
                    }

                    int index1 = 0;

                    for (index1 = 0; index1 < len1; index1++)
                    {
                        c = normalStr.charAt(index1);
                        if (c == ':') {
                            break;
                        }
                    }

                    String name = normalStr.substring(0, index1);

                    if (name.toUpperCase().trim().equals(BEGIN))
                    {
                        String vcalName = normalStr.substring(index1 + 1);
                        if (vcalName.toUpperCase().trim().equals(VCALENDAR))
                        {
                            vcalHead = vcalHead + str + CRLF;
                            inVcal = true;
                        }
                        else
                        {

                            str = d.readLine();
                            continue;
                        }
                    }

                }
                else
                {
                    if ((!inVtodo) && (!inVevent))
                    {
                        // grab the name
                        String normalStr = normalizeText(str.substring(0));

                        char c = 0;
                        int len2 = normalStr.length();

                        if (len2 == 0)
                        {
                            vcalHead = vcalHead + str + CRLF;
                            str = d.readLine();
                            continue;
                        }

                        int index2 = 0;

                        for (index2 = 0; index2 < len2; index2++)
                        {
                            c = normalStr.charAt(index2);
                            if (c == ':') {
                                break;
                            }
                        }

                        String name = normalStr.substring(0, index2);

                        if (name.toUpperCase().trim().equals(BEGIN))
                        {
                            String vcalName = normalStr.substring(index2 + 1);

                            if (vcalName.toUpperCase().trim().equals(VEVENT))
                            {
                                inVevent = true;
                                vEventString = "";
                                vEventString = vEventString + str.substring(0) + CRLF;
                            }
                            else if (vcalName.toUpperCase().trim().equals(VTODO))
                            {
                                inVtodo = true;
                                vTodoString = "";
                                vTodoString = vTodoString + str.substring(0) + CRLF;
                            }
                            else
                            {
                                vcalHead = vcalHead + str + CRLF;
                                str = d.readLine();
                                continue;
                            }
                        }
                        else if (name.toUpperCase().trim().equals(END))
                        {

                            String vcalName = normalStr.substring(index2 + 1);

                            if (vcalName.toUpperCase().trim().equals(VCALENDAR))
                            {
                                break;
                            } else {
                                vcalHead = vcalHead + str + CRLF;
                            }

                        }
                        else
                            vcalHead = vcalHead + str + CRLF;

                    }
                    else
                    {

                        if (inVevent)
                        {
                            // grab the name
                            String normalStr = normalizeText(str.substring(0));

                            char c = 0;
                            int len3 = normalStr.length();

                            if (len3 == 0)
                            {
                                vEventString = vEventString + str + CRLF;
                                str = d.readLine();
                                continue;
                            }

                            int index3 = 0;

                            for (index3 = 0; index3 < len3; index3++)
                            {
                                c = normalStr.charAt(index3);
                                if (c == ':') {
                                    break;
                                }
                            }

                            String name = normalStr.substring(0, index3);
                            if (name.toUpperCase().trim().equals(END))
                            {
                                String vcalName = normalStr.substring(index3 + 1);

                                if (vcalName.toUpperCase().trim().equals(VEVENT))
                                {
                                    inVevent = false;
                                    vEventString = vEventString + str + CRLF;

                                    parseString = "";
                                    parseString = new StringBuilder().append(vcalHead)
                                            .append(vEventString)
                                            .append("END:VCALENDAR")
                                            .append(CRLF).toString();
                                    mVCalManager = new VCalendarManager(mContext, parseString);
                                    int result = mVCalManager.saveVcalendar_new(fullResult);
                                    vEventString = "";

                                    if (result > 0)
                                    {
                                        insertIndex += result;
                                        mCallBackHandler.sendMessage(mCallBackHandler
                                                .obtainMessage(
                                                        EVENT_SET_PROGRESS_VALUE, insertIndex, 0));
                                    }
                                }
                                else
                                {
                                    vEventString = vEventString + str + CRLF;
                                }
                            }
                            else
                            {
                                vEventString = vEventString + str.trim() + CRLF;
                                str = d.readLine();
                                continue;
                            }

                        }

                        if (inVtodo)
                        {
                            // grab the name
                            String normalStr = normalizeText(str.substring(0));

                            char c = 0;
                            int len3 = normalStr.length();

                            if (len3 == 0)
                            {
                                vTodoString = vTodoString + str + CRLF;
                                str = d.readLine();
                                continue;
                            }

                            int index3 = 0;

                            for (index3 = 0; index3 < len3; index3++)
                            {
                                c = normalStr.charAt(index3);
                                if (c == ':') {
                                    break;
                                }
                            }

                            String name = normalStr.substring(0, index3);
                            if (name.toUpperCase().trim().equals(END))
                            {
                                String vcalName = normalStr.substring(index3 + 1);

                                if (vcalName.toUpperCase().trim().equals(VTODO))
                                {
                                    inVtodo = false;
                                    vTodoString = vTodoString + str + CRLF;

                                    parseString = "";
                                    parseString = new StringBuilder().append(vcalHead)
                                            .append(vTodoString)
                                            .append("END:VCALENDAR")
                                            .append(CRLF).toString();

                                    mVCalManager = new VCalendarManager(mContext, parseString);
                                    int result = mVCalManager.saveVcalendar_new(fullResult);
                                    vTodoString = "";
                                    if (result > 0)
                                    {
                                        insertIndex += result;
                                        mCallBackHandler.sendMessage(mCallBackHandler
                                                .obtainMessage(
                                                        EVENT_SET_PROGRESS_VALUE, insertIndex, 0));
                                    }

                                }
                                else
                                {
                                    vTodoString = vTodoString + str + CRLF;
                                }
                            }
                            else
                            {
                                vTodoString = vTodoString + str + CRLF;
                                str = d.readLine();
                                continue;
                            }

                        }
                    }

                }
                str = d.readLine();

            }

            if ((!fullResult[0]) && (!fullResult[1]) && (!fullResult[2]) && (!fullResult[3]))
            {
                ret = 1;
            }
            else
            {

                ret = 0;
                if (fullResult[0])
                    ret = ret | CALENDAR_MEMO_FULL;

                if (fullResult[1])
                    ret = ret | CALENDAR_APPOINTMENT_FULL;

                if (fullResult[2])
                    ret = ret | CALENDAR_ANNIVERSARY_FULL;

                if (fullResult[3])
                    ret = ret | CALENDAR_TASK_FULL;
            }

            f.close();
            d.close();
        } catch (java.io.FileNotFoundException e) {
            // If it doesn't exist, that's fine
            Log.e(TAG, "Error for:" + e.getMessage());
            e.printStackTrace();
        } catch (java.io.IOException e) {
            Log.e(TAG, "Error for:" + e.getMessage());
            e.printStackTrace();
        } finally {
        }
        return ret;
    }

    public void cancel()
    {
        mCancel = true;
    }

    private static String normalizeText(String text) {

        text = text.replaceAll("\r\n", "\n");

        text = text.replaceAll("\r", "\n");
        text = text.replaceAll("\n ", "");
        text = text.replaceAll("\n", "");
        return text;
    }

    public boolean isVcsFile(File file)
    {
        String str = getFileExtendName(file.getPath());

        Log.d(TAG, "the vcs file name is:" + str);

        if (str == null)
        {
            return false;
        }
        return str.equalsIgnoreCase("vcs");
    }

    //
    // Get fill name's suffix
    public String getFileExtendName(String filename)
    {
        int index = filename.lastIndexOf('.');
        return index == -1 ? null : filename.substring(index + 1);
    }

    public int getRestoreCount(String str)
    {
        int index = 0;
        int size = 0;
        String normalSizeStr = normalizeText(str.substring(0));
        int len = normalSizeStr.length();

        for (index = 0; index < len; index++) {
            char c = normalSizeStr.charAt(index);
            if (c == ':') {
                break;
            }
        }
        String sizeName = normalSizeStr.substring(0, index);
        if (sizeName != null && sizeName.toUpperCase().trim().equals("SIZE")) {
            size = Integer.parseInt(normalSizeStr.substring(index + 1));
        }
        return size;
    }

}
