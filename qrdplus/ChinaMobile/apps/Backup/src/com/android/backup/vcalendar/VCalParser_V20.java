/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.HashSet;
import android.util.Log;
import com.android.backup.pim.VBuilder;
import android.text.TextUtils;
import java.util.TimeZone;

public class VCalParser_V20 extends VCalParser_V10 {
    private static final String V10LINEBREAKER = "\r\n";

    private static final HashSet<String> acceptableComponents = new HashSet<String>(
            Arrays.asList("VEVENT", "VTODO", "VALARM", "VTIMEZONE"));

    private static final HashSet<String> acceptableV20Props = new HashSet<String>(
            Arrays.asList("DESCRIPTION", "DTEND", "DTSTART", "DUE",
                    "COMPLETED", "RRULE", "STATUS", "SUMMARY", "LOCATION", "CATEGORIES", "CLASS",
                    "UID",
                    "DTSTAMP", "PRIORITY", "DURATION", "X-ALLDAY"));// zhl
                                                                    // add."CATEGORIES","CLASS"..//sunxiaoming
                                                                    // modify
                                                                    // 20120904

    private boolean hasTZ = false; // MUST only have one TZ property

    private String[] lines;
    private static final String TAG = "VCalParser_V20";
    private int index;
    private String startTime = "";

    @Override
    public boolean parse(InputStream is, String encoding, VBuilder builder)
            throws IOException {
        // get useful info for android calendar, and alter to vcal 1.0
        byte[] bytes = new byte[is.available()];
        is.read(bytes);
        String scStr = new String(bytes);
        StringBuilder v10str = new StringBuilder("");

        lines = splitProperty(scStr);
        for (int i = 0; i < lines.length; i++) {
            Log.d("VCalParser_V20", "Lines :" + lines[i]);
        }
        index = 0;

        if ("BEGIN:VCALENDAR".equals(lines[index]))
            v10str.append("BEGIN:VCALENDAR" + V10LINEBREAKER);
        else
            return false;
        index++;
        if (false == parseV20Calbody(lines, v10str)
                || index > lines.length - 1)
            return false;

        if (lines.length - 1 == index && "END:VCALENDAR".equals(lines[index]))
            v10str.append("END:VCALENDAR" + V10LINEBREAKER);
        else
            return false;

        return super.parse(
                // use vCal 1.0 parser
                new ByteArrayInputStream(v10str.toString().getBytes()),
                encoding, builder);
    }

    /**
     * Parse and pick acceptable iCalendar body and translate it to calendarV1.0
     * format.
     *
     * @param lines iCalendar components/properties line list.
     * @param buffer calendarV10 format string buffer
     * @return true for success, or false
     */
    private boolean parseV20Calbody(String[] lines, StringBuilder buffer) {
        try {
            while (!"VERSION:2.0".equals(lines[index]))
                index++;
            buffer.append("VERSION:1.0" + V10LINEBREAKER);

            index++;
            for (; index < lines.length - 1; index++) {
                String[] keyAndValue = lines[index].split(":", 2);
                String key = keyAndValue[0];
                String value = "";
                if (keyAndValue.length > 1) {
                    value = keyAndValue[1];
                }
                Log.d("VCalParser_V20", "Key.trim()=" + (key.trim()));
                if ("BEGIN".equals(key.trim())) {
                    if (!key.equals(key.trim()))
                        return false; // MUST be "BEGIN:componentname"
                    index++;
                    Log.d("VCalParser_V20", "Index=" + index);
                    if (false == parseV20Component(value, buffer)) {
                        Log.d("VCalParser_V20", "Buffer=" + buffer);
                        return false;
                    }
                }
            }
            Log.d("VCalParser_V20", "Buffer=" + buffer);
        } catch (ArrayIndexOutOfBoundsException e) {
            Log.e("VCalParser_V20", "ArrayIndexOutOfBoundsException" + e);
            return false;
        }

        return true;
    }

    /**
     * Parse and pick acceptable calendar V2.0's component and translate it to
     * V1.0 format.
     *
     * @param compName component name
     * @param buffer calendarV10 format string buffer
     * @return true for success, or false
     * @throws ArrayIndexOutOfBoundsException
     */
    private boolean parseV20Component(String compName, StringBuilder buffer)
            throws ArrayIndexOutOfBoundsException {
        String endTag = "END:" + compName;
        String[] propAndValue;
        String propName, value;
        // zhl add
        String[] propNameAndParams;
        if (acceptableComponents.contains(compName)) {
            if ("VEVENT".equals(compName) || "VTODO".equals(compName)) {
                buffer.append("BEGIN:" + compName + V10LINEBREAKER);
                while (!endTag.equals(lines[index])) {
                    String params = null;
                    propAndValue = lines[index].split(":", 2);
                    // zhl delete
                    // propName = propAndValue[0].split(";", 2)[0];
                    // delete end
                    // /zhl add
                    // when transfer v2.0 to v1.0, if remove property parameter,
                    // chinese decode will show bad code.
                    propNameAndParams = propAndValue[0].split(";", 2);
                    propName = propNameAndParams[0];
                    if (propNameAndParams.length > 1)
                    {
                        params = propNameAndParams[1];
                    }
                    // add end
                    value = propAndValue[1];

                    if ("".equals(lines[index])) {
                        buffer.append(V10LINEBREAKER);
                    }
                    else if (acceptableV20Props.contains(propName)) {
                        // zhl delete
                        // buffer.append(propName + ":" + value +
                        // V10LINEBREAKER);
                        // delete end

                        // sunxiaoming add to get the startTime
                        if (propName.equals("DTSTART")) {
                            startTime = value;
                            Log.d("VCalParser_V20", "StartTime:" + startTime);
                        }
                        //

                        // zhl add
                        buffer.append(propName);
                        if (params != null)
                            buffer.append(";" + params + ":");
                        else
                            buffer.append(":");
                        buffer.append(value + V10LINEBREAKER);
                        // add end
                    } else if ("BEGIN".equals(propName.trim())) {
                        // MUST be BEGIN:VALARM
                        if (propName.equals(propName.trim())
                                && "VALARM".equals(value)) {
                            // buffer.append("AALARM:default" +
                            // V10LINEBREAKER);//sunxiaoming delete
                            while (!"END:VALARM".equals(lines[index])) {
                                if ((lines[index]).contains("TRIGGER")) {// sunxiaoming
                                                                         // add
                                                                         // for
                                                                         // translate
                                                                         // 2.0-valarm
                                                                         // to
                                                                         // 1.0-valarm
                                    String[] mSplits = (lines[index]).split(":", 2);
                                    String oriTime = mSplits[1];
                                    // we only need the number ,so ,we get it
                                    // from "oriTime"
                                    String[] numTime = getTime(oriTime);
                                    long alarmTime = 0;
                                    alarmTime = getAlarmTime(numTime);
                                    buffer.append("AALARM:" + alarmTime + V10LINEBREAKER);
                                    //
                                }
                                index++;
                            }
                        } else
                        {
                            return false;
                        }
                    }
                    index++;
                } // end while
                buffer.append(endTag + V10LINEBREAKER);
            } else if ("VALARM".equals(compName)) { // VALARM component MUST
                // only appear within either VEVENT or VTODO
                return false;
            } else if ("VTIMEZONE".equals(compName)) {// sunxiaoming add for
                                                      // Timezone restore
                do {
                    if (false == hasTZ) {// MUST only have 1 time TZ property
                        propAndValue = lines[index].split(":", 2);
                        propName = propAndValue[0].split(";", 2)[0];
                        if ("TZOFFSETFROM".equals(propName)) {
                            value = propAndValue[1];
                            /*
                             * TimeZone tz = TimeZone.getTimeZone("GMT"+value);
                             * Log
                             * .d(TAG+"_sunxiaoming","----->parseV20Component : "
                             * +tz.getID());
                             */
                            buffer.append("TZ" + ":" + value + V10LINEBREAKER);
                            hasTZ = true;
                        }
                    }
                    index++;
                } while (!endTag.equals(lines[index]));
            } else
                return false;
        } else {
            while (!endTag.equals(lines[index]))
                index++;
        }

        return true;
    }

    /** split ever property line to String[], not split folding line. */
    private String[] splitProperty(String scStr) {
        /*
         * Property splitted by \n, and unfold folding lines by removing
         * CRLF+LWSP-char
         */
        scStr = scStr// .replaceAll("=\r\n  ","")
                .replaceAll("=\r\n", "")// sxm add 20121022
                .replaceAll("\r\n", "\n").replaceAll("\n ", "")
                .replaceAll("\n\t", "");
        String[] strs = scStr.split("\n");
        return strs;
    }

    // sunxiaoming add to get the number time
    private String[] getTime(String oriTime) {
        String[] dateAndTime = {
                "", "", "", "", "", "", ""
        };
        boolean isP = false;
        boolean isT = false;
        int offset = 0;
        int sum = 0;
        // P;Y,M,W,D,
        // T;H,M,S
        for (int i = 0; i < oriTime.length(); i++) {
            //
            if (oriTime.charAt(i) == 'P') {
                isP = true;
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'Y')) {
                dateAndTime[0] = oriTime.substring(offset + 1, i);
                Log.d(TAG, "DateAndTime[0]=" + dateAndTime[0]);
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'M') && isT == false) {
                dateAndTime[1] = oriTime.substring(offset + 1, i);
                Log.d(TAG, "DateAndTime[1]=" + dateAndTime[1]);
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'W')) {
                dateAndTime[2] = oriTime.substring(offset + 1, i);
                Log.d(TAG, "DateAndTime[2]=" + dateAndTime[2]);
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'D')) {
                dateAndTime[3] = oriTime.substring(offset + 1, i);
                Log.d(TAG, "DateAndTime[3]=" + dateAndTime[3]);
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'T')) {
                isT = true;
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'H')) {
                dateAndTime[4] = oriTime.substring(offset + 1, i);
                Log.d(TAG, "DateAndTime[4]=" + dateAndTime[4]);
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'M') && isT == true) {
                dateAndTime[5] = oriTime.substring(offset + 1, i);
                Log.d(TAG , "Offset+1=" + (offset + 1) + " (i-1)=" + (i));
                Log.d(TAG, "DateAndTime[5]=" + dateAndTime[5]);
                offset = i;
                continue;
            }
            if ((oriTime.charAt(i) == 'S') && isT == true) {
                dateAndTime[5] = oriTime.substring(offset + 1, i);
                Log.d(TAG, "DateAndTime[6]=" + dateAndTime[6]);
                offset = i;
                continue;
            }

        }

        return dateAndTime;
    }

    // sunxiaoming add for change the time to millis style
    public long getAlarmTime(String[] numTime) {
        Log.d(TAG, "NumTime.length=" + numTime.length);
        long time = 0;
        for (int i = 0; i < 7; i++) {
            //
            if (!TextUtils.isEmpty(numTime[i])) {
                //
                if (i == 0) {
                    time += Integer.parseInt(numTime[i]) * 365 * 24 * 60 * 60000;
                }
                if (i == 1) {
                    time += Integer.parseInt(numTime[i]) * 30 * 24 * 60 * 60000;
                }
                if (i == 2) {
                    time += Integer.parseInt(numTime[i]) * 7 * 24 * 60 * 60000;
                }
                if (i == 3) {
                    time += Integer.parseInt(numTime[i]) * 24 * 60 * 60000;
                }
                if (i == 4) {
                    time += Integer.parseInt(numTime[i]) * 60 * 60000;
                }
                if (i == 5) {
                    time += Integer.parseInt(numTime[i]) * 60000;
                }
                if (i == 6) {
                    time += Integer.parseInt(numTime[i]) * 1000;
                }
            } else {
                continue;
            }
        }
        Log.d(TAG, "GetAlarmTime:time=" + time);
        return time;
    }
}
