/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.lunarinfo;

import android.content.Context;
import android.text.TextUtils;

import com.qualcomm.qti.lunarinfo.LunarInfo.DateInfo;

import java.util.ArrayList;

public class FestivalParser {
    /**
     * Used to get the lunar festival and solar festival. The result will be stored in the list.
     */
    public static void getFestivals(Context context, DateInfo date, ArrayList<String> list) {
        if (date == null || list == null) return;

        getLunarFestival(context, date, list);
        getSolarFestival(context, date, list);
    }

    private static void getLunarFestival(Context context, DateInfo date, ArrayList<String> list) {
        if (context == null || date == null || date._isLeapMonth || list == null) return;

        String festival = null;
        if (date._lunar_month == 1 && date._lunar_day == 1) {
            festival = context.getString(R.string.chunjie);
        }
        if (date._lunar_month == 1 && date._lunar_day == 15) {
            festival = context.getString(R.string.yuanxiao);
        }
        if (date._lunar_month == 5 && date._lunar_day == 5) {
            festival = context.getString(R.string.duanwu);
        }
        if (date._lunar_month == 7 && date._lunar_day == 7) {
            festival = context.getString(R.string.qixi);
        }
        if (date._lunar_month == 8 && date._lunar_day == 15) {
            festival = context.getString(R.string.zhongqiu);
        }
        if (date._lunar_month == 9 && date._lunar_day == 9) {
            festival = context.getString(R.string.chongyang);
        }
        if (date._lunar_month == 12 && date._lunar_day == 8) {
            festival = context.getString(R.string.laba);
        }
        if (date._lunar_month == 12 && date._lunar_day == 23) {
            festival = context.getString(R.string.xiaonian);
        }

        if (date._lunar_month == 12) {
            int lastDay = LunarParser.getLunarMonthDays(date._lunar_year, date._lunar_month);
            if (date._lunar_day == lastDay) {
                festival = context.getString(R.string.chuxi);
            }
        }

        if (!TextUtils.isEmpty(festival)) {
            list.add(festival);
        }
    }

    private static void getSolarFestival(Context context, DateInfo date, ArrayList<String> list) {
        if (context == null || date == null || list == null) return;

        String festival = null;
        if (date._solar_month == 0 && date._solar_day == 1) {
            festival = context.getString(R.string.new_Year_day);
        }
        if (date._solar_month == 1 && date._solar_day == 14) {
            festival = context.getString(R.string.valentin_day);
        }
        if (date._solar_month == 2 && date._solar_day == 8) {
            festival = context.getString(R.string.women_day);
        }
        if (date._solar_year > 1978 && date._solar_month == 2 && date._solar_day == 12) {
            festival = context.getString(R.string.arbor_day);
        }
        if (date._solar_month == 4 && date._solar_day == 1) {
            festival = context.getString(R.string.labol_day);
        }
        if (date._solar_month == 4 && date._solar_day == 4) {
            festival = context.getString(R.string.youth_day);
        }
        if (date._solar_month == 5 && date._solar_day == 1) {
            festival = context.getString(R.string.children_day);
        }
        if (date._solar_month == 7 && date._solar_day == 1) {
            festival = context.getString(R.string.army_day);
        }
        if (date._solar_year > 1984 && date._solar_month == 8 && date._solar_day == 10) {
            festival = context.getString(R.string.teacher_day);
        }
        if (date._solar_month == 9 && date._solar_day == 1) {
            festival = context.getString(R.string.national_day);
        }
        if (date._solar_month == 11 && date._solar_day == 25) {
            festival = context.getString(R.string.christmas_day);
        }

        if (!TextUtils.isEmpty(festival)) {
            list.add(festival);
        }
    }

}
