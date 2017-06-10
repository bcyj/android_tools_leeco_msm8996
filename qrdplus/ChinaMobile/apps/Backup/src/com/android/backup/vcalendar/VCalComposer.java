/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
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

//zhl add
import org.apache.commons.codec.net.QuotedPrintableCodec;
import java.io.UnsupportedEncodingException;
import java.util.TimeZone;
import android.util.Log;
//add end
import android.text.TextUtils;//sxm add 20121021
import android.os.SystemProperties;

/**
 * vCalendar string composer class
 */
public class VCalComposer {
    private static final String TAG = "VCalComposer";
    public final static String VERSION_VCALENDAR10 = "vcalendar1.0";
    public final static String VERSION_VCALENDAR20 = "vcalendar2.0";

    public final static int VERSION_VCAL10_INT = 1;
    public final static int VERSION_VCAL20_INT = 2;

    // zhl add
    public static final int MODE_MASK_HEAD = 0x00000001;
    public static final int MODE_MASK_VCAL = 0x00000004;
    public static final int MODE_MASK_TAIL = 0x00000008;

    public final int MODE_DEFAULT = MODE_MASK_HEAD | MODE_MASK_VCAL | MODE_MASK_TAIL;

    private int mCreateMod = MODE_DEFAULT;
    // add end
    private static String mNewLine = "\r\n";
    private String mVersion = null;
    private boolean isDue = false;

    public VCalComposer() {
    }

    // zhl add
    public VCalComposer(int vcalMode) {
        mCreateMod = vcalMode;
    }

    // add end

    /**
     * Create a vCalendar String.
     *
     * @param struct see more from CalendarStruct class
     * @param vcalversion MUST be VERSION_VCAL10 /VERSION_VCAL20
     * @return vCalendar string
     * @throws VcalException if version is invalid or create failed
     */
    public String createVCal(CalendarStruct struct, int vcalversion)
            throws VCalException {

        StringBuilder returnStr = new StringBuilder();

        // Version check
        if (vcalversion != 1 && vcalversion != 2)
            throw new VCalException("version not match 1.0 or 2.0.");
        if (vcalversion == 1)
            mVersion = VERSION_VCALENDAR10;
        else
            mVersion = VERSION_VCALENDAR20;

        // zhl modify
        if ((mCreateMod & MODE_MASK_HEAD) == MODE_MASK_HEAD)
        {
            // Build vCalendar:
            returnStr.append("BEGIN:VCALENDAR").append(mNewLine);

            if (vcalversion == VERSION_VCAL10_INT)
                returnStr.append("VERSION:1.0").append(mNewLine);
            else
                returnStr.append("VERSION:2.0").append(mNewLine);
            String proId = SystemProperties.get("ro.product.fullname", "null");
            returnStr.append("PRODID:").append(proId).append(mNewLine);

            if (!isNull(struct.timezone)) {// one vcalendar , one timezone
                if (vcalversion == VERSION_VCAL10_INT)
                    returnStr.append("TZ:").append(struct.timezone).append(mNewLine);
                else {
                    TimeZone tz = TimeZone.getTimeZone(struct.timezone);
                    String zoneOffset = VCalendarManager.generateTimezoneOffset(tz, vcalversion);
                    String time = VCalendarManager.transitionMillisToVCalendarTime(System.currentTimeMillis(),
                            tz, false);
                    // down here MUST have
                    returnStr.append("BEGIN:VTIMEZONE").append(mNewLine).
                            append("TZID:").append(tz.getID()).append(mNewLine).
                            append("BEGIN:STANDARD").append(mNewLine).
                            append("DTSTART:").append(time).append(mNewLine).
                            append("TZOFFSETFROM:").append(zoneOffset).append(mNewLine).
                            append("TZOFFSETTO:").append(zoneOffset).append(mNewLine).
                            append("END:STANDARD").append(mNewLine).
                            append("BEGIN:DAYLIGHT").append(mNewLine).
                            append("DTSTART:").append(time).append(mNewLine).
                            append("TZOFFSETFROM:").append(zoneOffset).append(mNewLine).
                            append("TZOFFSETTO:").append(zoneOffset).append(mNewLine).
                            append("END:DAYLIGHT").append(mNewLine).
                            append("END:VTIMEZONE").append(mNewLine);
                }
            }

        }
        // Build VEVNET
        if (struct.eventList != null)// zhl add ..the eventlist must not be null
        {
            for (int i = 0; i < struct.eventList.size(); i++) {
                String str = buildEventStr(struct.eventList.get(i));
                returnStr.append(str);
            }
        }

        // Build VTODO
        // TODO
        // zhl add for vtodo
        if (struct.todoList != null)
        {
            for (int i = 0; i < struct.todoList.size(); i++) {
                String str = buildTodoStr(struct.todoList.get(i));
                returnStr.append(str);
            }

        }
        // add end
        // zhl modify
        if ((mCreateMod & MODE_MASK_TAIL) == MODE_MASK_TAIL)
            returnStr.append("END:VCALENDAR").append(mNewLine).append(mNewLine);

        return returnStr.toString();
    }

    private String buildEventStr(CalendarStruct.EventStruct stru) {

        StringBuilder strbuf = new StringBuilder();
        // zhl add
        String temStr = null;

        strbuf.append("BEGIN:VEVENT").append(mNewLine);

        if (!isNull(stru.uid)) {
            if (stru.uid.contains("content:")) {
                String uidStr = stru.uid.replace("content:", "content");
                strbuf.append("UID:").append(uidStr).append(mNewLine);
            } else {
                strbuf.append("UID:").append(stru.uid).append(mNewLine);
            }
        }
        // zhl
        if (!isNull(stru.description))
        {
            if (hasChinese(stru.description))
            {
                temStr = convertToQpEncoding("DESCRIPTION", foldingString(stru.description));
                strbuf.append(temStr).append(mNewLine);
            }
            else// add end
            {
                strbuf.append("DESCRIPTION:").append(foldingString(stru.description))
                        .append(mNewLine);
            }
        }

        if (!isNull(stru.category))
            strbuf.append("CATEGORIES:").append(stru.category).append(mNewLine);

        if (!isNull(stru.classfication))
            strbuf.append("CLASS:").append(stru.classfication).append(mNewLine);

//      DUE is not RFC5545 Standards point, use DURATION
        if (!isNull(stru.duration)) {//
            isDue = true;
            strbuf.append("DURATION:").append(stru.duration).append(mNewLine);
        }

        if (!isDue) {
            if (!isNull(stru.dtend))
                strbuf.append("DTEND").append(";TZID=" + stru.timezone + ":").append(stru.dtend)
                        .append(mNewLine);// sunxiaoming 2012.9.3
        }

        if (!isNull(stru.last_date))
            // strbuf.append("COMPLETED:").append(stru.last_date).append(mNewLine);
            strbuf.append("DTSTAMP:").append(stru.last_date).append(mNewLine);// for
                                                                              // the
                                                                              // chinaMobile
                                                                              // standard

        if (!isNull(stru.allDay)) {
            strbuf.append("X-ALLDAY:").append(stru.allDay).append(mNewLine);
        }

        if (!isNull(stru.dtstart))
            strbuf.append("DTSTART").append(";TZID=" + stru.timezone + ":").append(stru.dtstart)
                    .append(mNewLine);// sunxiaoming 2012.9.3

        /*
         * if(!isNull(stru.last_date))
         * strbuf.append("DTSTAMP:").append(stru.last_date).append(mNewLine);
         */
        if (!isNull(stru.event_location))
        {
            // zhl add to support chinese characters.
            if (hasChinese(stru.event_location))
            {
                temStr = convertToQpEncoding("LOCATION", stru.event_location);
                strbuf.append(temStr).append(mNewLine);
            }
            else
                // add end
                strbuf.append("LOCATION:").append(stru.event_location).append(mNewLine);
        }

        if (!isNull(stru.rrule))
            strbuf.append("RRULE:").append(stru.rrule).append(mNewLine);

        if (!isNull(stru.title))
        {
            if (hasChinese(stru.title))
            {
                temStr = convertToQpEncoding("SUMMARY", stru.title);
                strbuf.append(temStr).append(mNewLine);
            }
            else
                // add end
                strbuf.append("SUMMARY:").append(stru.title).append(mNewLine);

        }
        if (!isNull(stru.status)) {
            String stat = "TENTATIVE";
            switch (Integer.parseInt(stru.status)) {
                case 0:// Calendar.Calendars.STATUS_TENTATIVE
                    stat = "TENTATIVE";
                    break;
                case 1:// Calendar.Calendars.STATUS_CONFIRMED
                    stat = "CONFIRMED";
                    break;
                case 2:// Calendar.Calendars.STATUS_CANCELED
                    stat = "CANCELLED";
                    break;
            }
            strbuf.append("STATUS:").append(stat).append(mNewLine);
        }
        // Alarm
        if (!isNull(stru.has_alarm)
                && stru.reminderList != null
                && stru.reminderList.size() > 0) {

            if (mVersion.equals(VERSION_VCALENDAR10)) {
                String prefix = "";
                String method = "";
                String alarmTime = "";

                // zhl add
                for (int i = 0; i < stru.reminderList.size(); i++)
                {
                    method = stru.reminderList.get(i);
                    alarmTime = stru.reminderTime.get(i);

                    switch (Integer.parseInt(method)) {
                        case 0:
                            prefix = "DALARM";
                            break;
                        case 1:
                            prefix = "AALARM";
                            break;
                        case 2:
                            prefix = "MALARM";
                            break;
                        case 3:
                        default:
                            prefix = "AALARM";
                            break;
                    }

                    if (alarmTime.length() != 0)
                    {
                        strbuf.append(prefix).append(':').append(alarmTime).append(mNewLine);
                    }
                    else
                    {
                        strbuf.append(prefix).append(":default").append(mNewLine);
                    }
                }
                // add end

                /*
                 * zhl modify for(String method : stru.reminderList){ switch
                 * (Integer.parseInt(method)){ case 0: prefix = "DALARM"; break;
                 * case 1: prefix = "AALARM"; break; case 2: prefix = "MALARM";
                 * break; case 3: default: prefix = "AALARM"; break; }
                 * strbuf.append(prefix).append(":default").append(mNewLine); }
                 */

            } else {// version 2.0 only support audio-method now.
                for (int i = 0; i < stru.reminderList.size(); i++) {
                    String alarmTime = "";
                    alarmTime = stru.reminderTime.get(i);
                    strbuf.append("BEGIN:VALARM").append(mNewLine).
                            append("ACTION:AUDIO").append(mNewLine).
                            // append("TRIGGER:-PT10M").append(mNewLine).
                            append("TRIGGER:").append(alarmTime).append(mNewLine).
                            append("END:VALARM").append(mNewLine);
                }
            }
        }
        strbuf.append("END:VEVENT").append(mNewLine);
        return strbuf.toString();
    }

    // zhl add begin
    private String buildTodoStr(CalendarStruct.TodoStruct stru) {

        StringBuilder strbuf = new StringBuilder();
        String temStr = null;

        strbuf.append("BEGIN:VTODO").append(mNewLine);

        if (!isNull(stru.uid))
            strbuf.append("UID:").append(stru.uid).append(mNewLine);

        if (!isNull(stru.description))
        {
            if (hasChinese(stru.description))
            {
                temStr = convertToQpEncoding("DESCRIPTION", foldingString(stru.description));
                strbuf.append(temStr).append(mNewLine);
            }
            else
            {
                strbuf.append("DESCRIPTION:");
                strbuf.append(foldingString(stru.description)).append(mNewLine);
            }
        }

        if (!isNull(stru.duration))
        {
            isDue = true;
            strbuf.append("DUE:").append(stru.duration).append(mNewLine);
        }

        if (!isDue) {
            if (!isNull(stru.dtend))
                strbuf.append("DTEND").append(";TZID=" + stru.timezone + ":").append(stru.dtend)
                        .append(mNewLine);// sunxiaoming2012.9.3
        }

        if (!isNull(stru.dtstart))
            strbuf.append("DTSTART").append(";TZID=" + stru.timezone + ":").append(stru.dtstart)
                    .append(mNewLine);// sunxiaoming 2012.9.3

        if (!isNull(stru.allDay)) {
            strbuf.append("X-ALLDAY:").append(stru.allDay).append(mNewLine);
        }

        if (!isNull(stru.last_date))
            // strbuf.append("COMPLETED:").append(stru.last_date).append(mNewLine);//
            strbuf.append("DTSTAMP:").append(stru.last_date).append(mNewLine);

        if (!isNull(stru.title))
        {
            if (hasChinese(stru.title))
            {
                temStr = convertToQpEncoding("SUMMARY", stru.title);
                strbuf.append(temStr).append(mNewLine);
            }
            else
                strbuf.append("SUMMARY:").append(stru.title).append(mNewLine);
        }
        if (!isNull(stru.priority))
            strbuf.append("PRIORITY:").append(stru.priority).append(mNewLine);

        if (!isNull(stru.status)) {
            String stat = "TENTATIVE";
            switch (Integer.parseInt(stru.status)) {
                case 0:// Calendar.Calendars.STATUS_TENTATIVE
                    stat = "TENTATIVE";
                    break;
                case 1:// Calendar.Calendars.STATUS_CONFIRMED
                    stat = "CONFIRMED";
                    break;
                case 2:// Calendar.Calendars.STATUS_CANCELED
                    stat = "CANCELLED";
                    break;
            }
            strbuf.append("STATUS:").append(stat).append(mNewLine);
        }
        // Alarm
        if (!isNull(stru.has_alarm)
                && stru.reminderList != null
                && stru.reminderList.size() > 0) {

            if (mVersion.equals(VERSION_VCALENDAR10)) {
                String prefix = "";
                String method = "";
                String alarmTime = "";

                // zhl add
                for (int i = 0; i < stru.reminderList.size(); i++)
                {
                    method = stru.reminderList.get(i);
                    alarmTime = stru.reminderTime.get(i);

                    switch (Integer.parseInt(method)) {
                        case 0:
                            prefix = "DALARM";
                            break;
                        case 1:
                            prefix = "AALARM";
                            break;
                        case 2:
                            prefix = "MALARM";
                            break;
                        case 3:
                        default:
                            prefix = "AALARM";
                            break;
                    }
                    if (alarmTime.length() != 0)
                    {
                        strbuf.append(prefix).append(':').append(alarmTime).append(mNewLine);
                    }
                    else
                    {
                        strbuf.append(prefix).append(":default").append(mNewLine);
                    }
                }
                // add end

                /*
                 * zhl modify for(String method : stru.reminderList){ switch
                 * (Integer.parseInt(method)){ case 0: prefix = "DALARM"; break;
                 * case 1: prefix = "AALARM"; break; case 2: prefix = "MALARM";
                 * break; case 3: default: prefix = "AALARM"; break; }
                 * strbuf.append(prefix).append(":default").append(mNewLine); }
                 */
            } else {// version 2.0 only support audio-method now.
                /*
                 * String alarmTime=""; alarmTime=stru.reminderTime.get(i);
                 * strbuf.append("BEGIN:VALARM").append(mNewLine).
                 * append("ACTION:AUDIO").append(mNewLine).
                 * //append("TRIGGER:-PT10M").append(mNewLine).
                 * append("TRIGGER:").append(alarmTime).append(mNewLine).
                 * append("END:VALARM").append(mNewLine);
                 */
                for (int i = 0; i < stru.reminderList.size(); i++) {
                    String alarmTime = "";
                    alarmTime = stru.reminderTime.get(i);
                    strbuf.append("BEGIN:VALARM").append(mNewLine).
                            append("ACTION:AUDIO").append(mNewLine).
                            // append("TRIGGER:-PT10M").append(mNewLine).
                            append("TRIGGER:").append(alarmTime).append(mNewLine).
                            append("END:VALARM").append(mNewLine);
                }
            }
        }
        strbuf.append("END:VTODO").append(mNewLine);
        return strbuf.toString();
    }

    // TODO: only for utf-8..dzm add
    private boolean hasChinese(String value1)
    {
        if (value1 == null)
            return false;

        byte[] valueBytes = value1.getBytes();
        return value1.length() != valueBytes.length;
    }

    private String convertToQpEncoding(String nameString, String originalString)
    {
        QuotedPrintableCodec qpc = new QuotedPrintableCodec();
        String rStr = null;
        // sxm add 20121021
        String mEncodeStr = null;
        mEncodeStr = encodeQuotedPrintable(nameString.length() + 41, originalString);
        // sxm add 20121021 End
        try {
            // rStr = ";ENCODING=QUOTED-PRINTABLE:"+qpc.encode(originalString);
            // rStr =
            // ";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:"+qpc.encode(originalString);
            rStr = nameString + ";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:" + mEncodeStr;
        } catch (Exception e) {
            Log.e("VCalComposer", "Failed to decode quoted-printable: " + e);
        }
        return rStr;
    }

    // sxm add 20121021 ,
    private String encodeQuotedPrintable(int lineCount, final String str) {
        if (TextUtils.isEmpty(str)) {
            return "";
        }

        final StringBuilder builder = new StringBuilder();
        int index = 0;
        byte[] strArray = null;

        strArray = str.getBytes();
        while (index < strArray.length) {
            builder.append(String.format("=%02X", strArray[index]));

            index += 1;
            lineCount += 3;

            if (lineCount >= 67) {
                // Specification requires CRLF must be inserted before the
                // length of the line
                // becomes more than 76.
                // Assuming that the next character is a multi-byte character,
                // it will become
                // 6 bytes.
                // 76 - 6 - 3 = 67
                builder.append("=\r\n");
                lineCount = 0;
            }
        }

        return builder.toString();
    }

    // add end
    /** Alter str to folding supported format. */
    private String foldingString(String str) {
        return str.replaceAll("\r\n", "\n").replaceAll("\n", "\r\n ");
    }

    /** is null */
    private boolean isNull(String str) {
        if (str == null || str.trim().equals(""))
            return true;
        return false;
    }
}
