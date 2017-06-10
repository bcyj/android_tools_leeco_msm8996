/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.lunarinfo;

import android.content.Context;
import android.content.res.XmlResourceParser;
import android.util.Log;

import com.qualcomm.qti.lunarinfo.LunarInfo.DateInfo;

import java.util.ArrayList;

public class SolarTermsParser {
    // The parameters to get the lunar solar terms.
    private static final double D_SOLAR_TERMS = 0.2422;
    private static final double[] C_20TH_CENTURY = new double[] { 6.11,
            20.84, 4.6295, 19.4599, 6.3826, 21.4155, 5.59, 20.888, 6.318,
            21.86, 6.5, 22.20, 7.928, 23.65, 8.38, 23.95, 8.44, 23.822, 9.098,
            24.218, 8.218, 23.08, 7.9, 22.60 };
    private static final double[] C_21TH_CENTURY = new double[] { 5.4055,
            20.12, 3.87, 18.73, 5.63, 20.646, 4.81, 20.1, 5.52, 21.04, 5.678,
            21.37, 7.108, 22.83, 7.5, 23.13, 7.646, 23.042, 8.318, 23.438,
            7.438, 22.36, 7.18, 21.94 };

    private static final String XML_TAG_TERM = "term";
    private static final String XML_TAG_ITEM = "item";
    private static final String XML_ATTR_YEAR = "year";
    private static final String XML_ATTR_MONTH = "month";
    private static final String XML_ATTR_DAY = "day";
    private static final String XML_ATTR_LABEL = "label";

    /**
     * Used to get the solar terms for the given date, and save the label to list.
     */
    public static void getSolarTerms(Context context, DateInfo date, ArrayList<String> list) {
        if (context == null || list == null) return;

        // Get the century value.
        int yy = 0;
        double[] c;
        if (date._solar_year > 1999) {
            yy = date._solar_year - 2000;
            c = C_21TH_CENTURY;
        } else {
            yy = date._solar_year - 1900;
            c = C_20TH_CENTURY;
        }

        // We will only handle the year for 20th century and 21th century.
        if (yy > 99 || yy < 0) return;

        // As this will be some error if use this formula to get the 24 solar terms,
        // so try to find the adjust value if it defined in the XML first.
        if (getSolarTermsFromXML(context, date, list)) return;

        // Do not found the defined value in the XML, use the formula to get the value.
        for (int i = 0; i < c.length; i++) {
            int dd = 0;
            if (i < 4) {
                dd = (int) Math.floor((yy * D_SOLAR_TERMS + c[i]) - ((yy - 1) / 4));
            } else {
                dd = (int) Math.floor((yy * D_SOLAR_TERMS + c[i]) - (yy / 4));
            }
            if (date._solar_day == dd && date._solar_month == i / 2) {
                String termsName = "terms" + date._solar_month + (i % 2);
                int termsInfoResId = context.getResources().getIdentifier(termsName, "string",
                        context.getPackageName());
                String solar_term = context.getString(termsInfoResId);
                if (i == 6) {
                    // If it is qingming, we need also add the festival.
                    String festival_qingming = context.getString(R.string.qingming);
                    list.add(festival_qingming);
                }
                list.add(solar_term);
            }
        }

    }

    /**
     * Get the 24 solar terms from the solar_terms.xml
     * @return true if found the defined value
     */
    private static boolean getSolarTermsFromXML(Context context, DateInfo date,
            ArrayList<String> list) {
        try {
            XmlResourceParser xml = context.getResources().getXml(R.xml.solar_terms);
            int eventType = -1;
            boolean foundMatched = false;
            while ((eventType = xml.next()) != XmlResourceParser.END_DOCUMENT) {
                if (eventType == XmlResourceParser.START_TAG
                        && XML_TAG_TERM.equals(xml.getName())) {
                    int year = xml.getAttributeIntValue(null, XML_ATTR_YEAR, 0);
                    int month = xml.getAttributeIntValue(null, XML_ATTR_MONTH, 0);
                    int day = xml.getAttributeIntValue(null, XML_ATTR_DAY, 0);
                    if (year == date._lunar_year
                            && month == date._solar_month) {
                        if (day == date._solar_day) {
                            foundMatched = true;
                            int labelResId = xml.getAttributeResourceValue(null, XML_ATTR_LABEL, 0);
                            if (labelResId > 0) {
                                String label = context.getString(labelResId);
                                list.add(label);
                            }
                        } else if (Math.abs(day - date._solar_day) < 3) {
                            // As the 24 solar terms difference will be in 3 days, so if we found
                            // the day is similar as the defined, we think we also get the solar
                            // terms info as it is not one solar terms with empty label.
                            return true;
                        }
                    }
                } else if (eventType == XmlResourceParser.START_TAG
                        && XML_TAG_ITEM.equals(xml.getName())
                        && foundMatched) {
                    int labelResId = xml.getAttributeResourceValue(null, XML_ATTR_LABEL, 0);
                    if (labelResId > 0) {
                        String label = context.getString(labelResId);
                        list.add(label);
                    }
                } else if (eventType == XmlResourceParser.END_TAG
                        && XML_TAG_TERM.equals(xml.getName())
                        && foundMatched) {
                    return true;
                }
            }
        } catch (Exception e) {
            Log.e("SolarTermsParser", "Catch the exception when get the solar terms from xml.");
        }

        return false;
    }
}
