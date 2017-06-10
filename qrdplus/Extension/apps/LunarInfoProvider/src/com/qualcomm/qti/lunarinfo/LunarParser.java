/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.lunarinfo;

import android.content.Context;
import android.util.Log;

import com.qualcomm.qti.lunarinfo.LunarInfo.DateInfo;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

public class LunarParser {
    private static final String TAG = "LunarParser";

    // To defined the days of every lunar month from 1900. If the month has 30 days,
    // set the value as 1, if the month has 29 days, set the value as 0.
    private static final short[] LUNAR_BASE_INFO = new short[] { 0x4bd,              // 1900
            0x4ae, 0xa57, 0x54d, 0xd26, 0xd95, 0x655, 0x56a, 0x9ad, 0x55d, 0x4ae,    // 1901 - 1910
            0xa5b, 0xa4d, 0xd25, 0xd25, 0xb54, 0xd6a, 0xada, 0x95b, 0x497, 0x497,    // 1911 - 1920
            0xa4b, 0xb4b, 0x6a5, 0x6d4, 0xab5, 0x2b6, 0x957, 0x52f, 0x497, 0x656,    // 1921 - 1930
            0xd4a, 0xea5, 0x6e9, 0x5ad, 0x2b6, 0x86e, 0x92e, 0xc8d, 0xc95, 0xd4a,    // 1931 - 1940
            0xd8a, 0xb55, 0x56a, 0xa5b, 0x25d, 0x92d, 0xd2b, 0xa95, 0xb55, 0x6ca,    // 1941 - 1950
            0xb55, 0x535, 0x4da, 0xa5d, 0x457, 0x52d, 0xa9a, 0xe95, 0x6aa, 0xaea,    // 1951 - 1960
            0xab5, 0x4b6, 0xaae, 0xa57, 0x526, 0xf26, 0xd95, 0x5b5, 0x56a, 0x96d,    // 1961 - 1970
            0x4dd, 0x4ad, 0xa4d, 0xd4d, 0xd25, 0xd55, 0xb54, 0xb6a, 0x95a, 0x95b,    // 1971 - 1980
            0x49b, 0xa97, 0xa4b, 0xb27, 0x6a5, 0x6d4, 0xaf4, 0xab6, 0x957, 0x4af,    // 1981 - 1990
            0x497, 0x64b, 0x74a, 0xea5, 0x6b5, 0x55c, 0xab6, 0x96d, 0x92e, 0xc96,    // 1991 - 2000
            0xd95, 0xd4a, 0xda5, 0x755, 0x56a, 0xabb, 0x25d, 0x92d, 0xcab, 0xa95,    // 2001 - 2010
            0xb4a, 0xbaa, 0xad5, 0x55d, 0x4ba, 0xa5b, 0x517, 0x52b, 0xa93, 0x795,    // 2011 - 2020
            0x6aa, 0xad5, 0x5b5, 0x4b6, 0xa6e, 0xa4e, 0xd26, 0xea6, 0xd53, 0x5aa,    // 2021 - 2030
            0x76a, 0x96d, 0x4bd, 0x4ad, 0xa4d, 0xd0b, 0xd25, 0xd52, 0xdd4, 0xb5a,    // 2031 - 2040
            0x56d, 0x55b, 0x49b, 0xa57, 0xa4b, 0xaa5, 0xb25, 0x6d2, 0xada };         // 2041 - 2049

    // To defined the leap month of every year. Set the low bit as the leap month number.
    // If there isn't leap month of this year, set the value as 0. For the high bit, it
    // set as the days number for this leap month. If the month has 30 days, set the value
    // as 1, if the month has 29 days, set the value as 0.
    private static final byte[] LUNAR_SPECIAL_INFO = new byte[] { 0x08,              // 1900
            0x00, 0x00, 0x05, 0x00, 0x00, 0x14, 0x00, 0x00, 0x02, 0x00,              // 1901 - 1910
            0x06, 0x00, 0x00, 0x15, 0x00, 0x00, 0x02, 0x00, 0x17, 0x00,              // 1911 - 1920
            0x00, 0x05, 0x00, 0x00, 0x14, 0x00, 0x00, 0x02, 0x00, 0x06,              // 1921 - 1930
            0x00, 0x00, 0x05, 0x00, 0x00, 0x13, 0x00, 0x17, 0x00, 0x00,              // 1931 - 1940
            0x16, 0x00, 0x00, 0x14, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00,              // 1941 - 1950
            0x00, 0x15, 0x00, 0x00, 0x13, 0x00, 0x08, 0x00, 0x00, 0x06,              // 1951 - 1960
            0x00, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0x07, 0x00, 0x00,              // 1961 - 1970
            0x05, 0x00, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x16, 0x00,              // 1971 - 1980
            0x00, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x06, 0x00, 0x00, 0x05,              // 1981 - 1990
            0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x05, 0x00, 0x00,              // 1991 - 2000
            0x04, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x05, 0x00,              // 2001 - 2010
            0x00, 0x04, 0x00, 0x09, 0x00, 0x00, 0x16, 0x00, 0x00, 0x04,              // 2011 - 2020
            0x00, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x05, 0x00, 0x00,              // 2021 - 2030
            0x03, 0x00, 0x07, 0x00, 0x00, 0x16, 0x00, 0x00, 0x05, 0x00,              // 2031 - 2040
            0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x15, 0x00, 0x00 };                  // 2041 - 2049

    // Defined the base year.
    private static final int BASE_YEAR = 1900;
    // Defined the out bound year.
    private static final int OUT_BOUND_YEAR = 2050;
    // Defined the base day, and the lunar value is 1900-1-1.
    private static final String BASE_DAY = "1900-1-31";

    // For every lunar month has 30 days or 29 days.
    private static final int BIG_MONTH_DAYS = 30;
    private static final int SMALL_MONTH_DAYS = 29;

    private static long sBaseDayTime = 0;
    private static SimpleDateFormat sSimpleDateFormat = new SimpleDateFormat("yyyy-MM-dd");

    private static Context sContext = null;

    // The strings to display the info.
    private static String sLunarLabel = null;
    private static String sLunarDay10 = null;
    private static String sLunarDay20 = null;
    private static String sLeapMonthLabel = null;
    private static String[] sLunarMonthLabels = null;
    private static String[] sLunarShiweiLabels = null;
    private static String[] sLunarGeweiLabels = null;
    private static String[] sAnimalLabels = null;

    static {
        try {
            sBaseDayTime = sSimpleDateFormat.parse(BASE_DAY).getTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }
    }

    /**
     * Used to get the animals label from the lunar year.
     */
    public static String getAnimalsYear(Context context, DateInfo date) {
        if (context == null || date == null) return "";
        initLabels(context);

        return sAnimalLabels[(date._lunar_year - 4) % 12];
    }

    /**
     * Used to get the entire lunar label. The return value will contains the lunar label,
     * lunar month label and the lunar day label.
     */
    public static String getLunarLongLabel(Context context, DateInfo date) {
        if (context == null || date == null) return "";
        initLabels(context);

        // Start with lunar label.
        StringBuilder label = new StringBuilder(sLunarLabel);
        // Append the lunar month label.
        label.append(getLunarMonthString(date));
        // Append the lunar day label.
        label.append(getLunarDayString(date, false));

        return label.toString();
    }

    /**
     * Used to get the short lunar label.<br>
     * If the day is the first day of one month, it will return the lunar month label.<br>
     * If the day is not the first day of the month, it will return the lunar day label.
     */
    public static String getLunarShortLabel(Context context, DateInfo date) {
        if (context == null || date == null) return "";
        initLabels(context);

        return getLunarDayString(date, true /* Show month string if the day is the first day. */);
    }

    /**
     * Used to get the number of the days in one lunar month.
     */
    public static int getLunarMonthDays(int lunarYear, int lunarMonth) {
        if (isBigMonth(lunarYear, lunarMonth)) {
            return BIG_MONTH_DAYS;
        } else {
            return SMALL_MONTH_DAYS;
        }
    }

    /**
     * Used to get the lunar info for the given solar date.
     *
     * @param refer The refer date info which already contains the lunar info.
     */
    public static DateInfo parseLunarCalendar(DateInfo refer, int solarYear, int solarMonth,
            int solarDay) {
        // We could get the info from the refer date info if the day only plus one.
        if (refer != null
                && refer._lunar_year > 0
                && refer._lunar_month > 0
                && refer._lunar_day > 0
                && refer._lunar_day < getLunarMonthDays(refer._lunar_year, refer._lunar_month)
                && solarYear == refer._solar_year
                && solarMonth == refer._solar_month
                && solarDay - refer._solar_day == 1) {
            DateInfo newDate = new DateInfo(solarYear, solarMonth, solarDay, refer._isLeapMonth,
                    refer._lunar_year, refer._lunar_month, refer._lunar_day + 1);
            return newDate;
        }

        // We need parse the date as new.
        int leapLunarMonth = 0;
        Date presentDate = null;
        boolean isLeapMonth = false;

        try {
            presentDate = sSimpleDateFormat.parse(solarYear + "-" + (solarMonth + 1) + "-"
                    + solarDay);
        } catch (ParseException e) {
            Log.e(TAG, "Catch the ParseException: " + e.getMessage());
            return null;
        }

        int offsetDayNum = -1;
        // For example, if the date is 1987-4-13, the offset will be get remainder,
        // and the integer will be same as 1987-4-12. So we will adjust the offset.
        float offset = (float) ((presentDate.getTime() - sBaseDayTime) / 86400000.0F);
        offsetDayNum = (int) offset;
        // Adjust the offsetDayNum
        if ((offset * 10 - offsetDayNum * 10) > 5) {
            offsetDayNum = offsetDayNum + 1;
        }

        int lunarYear = 0;
        int lunarMonth = 0;
        int lunarDay = 0;

        for (lunarYear = BASE_YEAR; lunarYear < OUT_BOUND_YEAR; lunarYear++) {
            int daysOfLunarYear = getYearDays(lunarYear);
            if (offsetDayNum < daysOfLunarYear) break;
            offsetDayNum -= daysOfLunarYear;
        }
        if (offsetDayNum < 0 || lunarYear == OUT_BOUND_YEAR) return null;

        leapLunarMonth = getLeapMonth(lunarYear);
        for (lunarMonth = 1; lunarMonth <= 12; lunarMonth++) {
            int daysOfLunarMonth = 0;
            if (isLeapMonth) {
                daysOfLunarMonth = getLeapMonthDays(lunarYear);
            } else {
                daysOfLunarMonth = getLunarMonthDays(lunarYear, lunarMonth);
            }

            if (offsetDayNum < daysOfLunarMonth) {
                break;
            } else {
                offsetDayNum -= daysOfLunarMonth;
                if (lunarMonth == leapLunarMonth) {
                    if (!isLeapMonth) {
                        lunarMonth--;
                        isLeapMonth = true;
                    } else {
                        isLeapMonth = false;
                    }
                }
            }
        }

        lunarDay = offsetDayNum + 1;

        return new DateInfo(solarYear, solarMonth, solarDay, isLeapMonth, lunarYear, lunarMonth,
                lunarDay);
    }

    private static boolean isBigMonth(int lunarYear, int lunarMonth) {
        short lunarYearBaseInfo = LUNAR_BASE_INFO[lunarYear - BASE_YEAR];
        if ((lunarYearBaseInfo & (0x01000 >>> lunarMonth)) != 0) {
            return true;
        } else {
            return false;
        }
    }

    private static int getYearDays(int lunarYear) {
        int retSum = 0;
        for (int lunarMonth = 1; lunarMonth <= 12; lunarMonth++) {
            retSum += getLunarMonthDays(lunarYear, lunarMonth);
        }
        return (retSum + getLeapMonthDays(lunarYear));
    }

    private static int getLeapMonth(int lunarYear) {
        return LUNAR_SPECIAL_INFO[lunarYear - BASE_YEAR] & 0xf;
    }

    private static int getLeapMonthDays(int lunarYear) {
        if (getLeapMonth(lunarYear) == 0) {
            return 0;
        } else if ((LUNAR_SPECIAL_INFO[lunarYear - BASE_YEAR] & 0x10) != 0) {
            return BIG_MONTH_DAYS;
        } else {
            return SMALL_MONTH_DAYS;
        }
    }

    private static String getLunarMonthString(DateInfo date) {
        if (date == null) return "";

        // If the lunar month is leap month, need add the leap label first.
        StringBuilder label = new StringBuilder(date._isLeapMonth ? sLeapMonthLabel : "");
        // Append the lunar month label.
        label.append(sLunarMonthLabels[date._lunar_month - 1]);

        return label.toString();
    }

    private static String getLunarDayString(DateInfo date, boolean showMonthForFirstDay) {
        if (date == null) return "";

        switch (date._lunar_day) {
            case 10:
                return sLunarDay10;
            case 20:
                return sLunarDay20;
            case 1:
                if (showMonthForFirstDay) {
                    // If the lunar day is the first day of one month, display the month value.
                    return getLunarMonthString(date);
                }
            default:
                StringBuilder label = new StringBuilder();
                label.append(sLunarShiweiLabels[date._lunar_day / 10]);
                label.append(sLunarGeweiLabels[(date._lunar_day + 9) % 10]);
                return label.toString();
        }
    }

    private static void initLabels(Context context) {
        if (context == null) throw new IllegalArgumentException("Context is null.");

        if (sContext != null) {
            // Already init the labels, needn't do again.
            return;
        }

        sContext = context;
        if (sLunarLabel == null) {
            sLunarLabel = getString(R.string.lunar_label);
        }

        if (sLunarDay10 == null) {
            sLunarDay10 = getString(R.string.lunar_day10);
        }

        if (sLunarDay20 == null) {
            sLunarDay20 = getString(R.string.lunar_day20);
        }

        if (sLeapMonthLabel == null) {
            sLeapMonthLabel = getString(R.string.leap_month);
        }

        if (sLunarMonthLabels == null) {
            sLunarMonthLabels = new String[12];
            sLunarMonthLabels[0] = getString(R.string.lunar_month1);
            sLunarMonthLabels[1] = getString(R.string.lunar_month2);
            sLunarMonthLabels[2] = getString(R.string.lunar_month3);
            sLunarMonthLabels[3] = getString(R.string.lunar_month4);
            sLunarMonthLabels[4] = getString(R.string.lunar_month5);
            sLunarMonthLabels[5] = getString(R.string.lunar_month6);
            sLunarMonthLabels[6] = getString(R.string.lunar_month7);
            sLunarMonthLabels[7] = getString(R.string.lunar_month8);
            sLunarMonthLabels[8] = getString(R.string.lunar_month9);
            sLunarMonthLabels[9] = getString(R.string.lunar_month10);
            sLunarMonthLabels[10] = getString(R.string.lunar_month11);
            sLunarMonthLabels[11] = getString(R.string.lunar_month12);
        }

        if (sLunarShiweiLabels == null) {
            sLunarShiweiLabels = new String[4];
            sLunarShiweiLabels[0] = getString(R.string.lunar_shiwei0);
            sLunarShiweiLabels[1] = getString(R.string.lunar_shiwei1);
            sLunarShiweiLabels[2] = getString(R.string.lunar_shiwei2);
            sLunarShiweiLabels[3] = getString(R.string.lunar_shiwei3);
        }

        if (sLunarGeweiLabels == null) {
            sLunarGeweiLabels = new String[10];
            sLunarGeweiLabels[0] = getString(R.string.lunar_gewei1);
            sLunarGeweiLabels[1] = getString(R.string.lunar_gewei2);
            sLunarGeweiLabels[2] = getString(R.string.lunar_gewei3);
            sLunarGeweiLabels[3] = getString(R.string.lunar_gewei4);
            sLunarGeweiLabels[4] = getString(R.string.lunar_gewei5);
            sLunarGeweiLabels[5] = getString(R.string.lunar_gewei6);
            sLunarGeweiLabels[6] = getString(R.string.lunar_gewei7);
            sLunarGeweiLabels[7] = getString(R.string.lunar_gewei8);
            sLunarGeweiLabels[8] = getString(R.string.lunar_gewei9);
            sLunarGeweiLabels[9] = getString(R.string.lunar_gewei10);
        }

        if (sAnimalLabels == null) {
            sAnimalLabels = new String[12];
            sAnimalLabels[0] = getString(R.string.animals0);
            sAnimalLabels[1] = getString(R.string.animals1);
            sAnimalLabels[2] = getString(R.string.animals2);
            sAnimalLabels[3] = getString(R.string.animals3);
            sAnimalLabels[4] = getString(R.string.animals4);
            sAnimalLabels[5] = getString(R.string.animals5);
            sAnimalLabels[6] = getString(R.string.animals6);
            sAnimalLabels[7] = getString(R.string.animals7);
            sAnimalLabels[8] = getString(R.string.animals8);
            sAnimalLabels[9] = getString(R.string.animals9);
            sAnimalLabels[10] = getString(R.string.animals10);
            sAnimalLabels[11] = getString(R.string.animals11);
        }
    }

    private static String getString(int resId) {
        if (sContext == null) return null;

        return sContext.getString(resId);
    }

}
