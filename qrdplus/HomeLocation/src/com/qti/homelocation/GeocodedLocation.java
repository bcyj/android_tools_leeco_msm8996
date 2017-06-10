/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.homelocation;

import java.io.Serializable;
import java.util.Locale;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.BaseColumns;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

public class GeocodedLocation implements Serializable {

    private static final long serialVersionUID = 1L;

    public static final String AUTHORITY = "geocoded_location";

    public static final String PATH = "location";

    public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/" + PATH);

    public static final int TYPE_LANDLINE = 0;
    public static final int TYPE_MOBILE = 1;

    public static final class Area implements BaseColumns {

        public static final String PATH = "area";

        /**
         * the unique province ID
         */
        public static final String AREA_ID = "area_id";

        /**
         * province name
         */
        public static final String AREA_NAME = "province";

        public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/" + PATH);

        private String mName;
        private long mAreaId;

        public Area(Cursor c) {
            mAreaId = c.getLong(c.getColumnIndex(AREA_ID));
            mName = c.getString(c.getColumnIndex(AREA_NAME));
        }

        public String getName() {
            return mName;
        }

        public long getAreaId() {
            return mAreaId;
        }
    }

    public static final class AreaCode implements BaseColumns {

        public static final String PATH = "area_code";

        /**
         * the unique area ID
         */
        public static final String CODE_ID = "code_id";

        /**
         * area code
         */
        public static final String CODE = "code";

        /**
         * city name
         */
        public static final String CITY_NAME = "city";

        public static final String AREA_ID = Area.AREA_ID;

        public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/" + PATH);

        private long mCodeId;
        private String mCode;
        private String mCity;
        private Area mAarea;

        public AreaCode(Cursor c) {
            mCodeId = c.getLong(c.getColumnIndex(CODE_ID));
            mCode = c.getString(c.getColumnIndex(CODE));
            mCity = c.getString(c.getColumnIndex(CITY_NAME));
            mAarea = new Area(c);
        }

        public long getCodeId() {
            return mCodeId;
        }

        public String getCode() {
            return mCode;
        }

        public String getCity() {
            return mCity;
        }

        public Area getArea() {
            return mAarea;
        }

        public String getAddress() {
            if (mCity.contains(mAarea.getName())) {
                return mCity;
            } else {
                return mCity + "," + mAarea.getName();
            }
        }
    }

    public static final class Section implements BaseColumns {

        public static final String PATH = "section";

        /**
         * start of the section
         */
        public static final String NUMBER_START = "number1";

        /**
         * end of the section
         */
        public static final String NUMBER_END = "number2";

        public static final String CODE_ID = AreaCode.CODE_ID;

        public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/" + PATH);

    }

    private AreaCode mAreaCode;

    private int mType;

    public GeocodedLocation(Cursor c, int type) {
        mAreaCode = new AreaCode(c);
        this.mType = type;
    }

    public AreaCode getAreaCode() {
        return mAreaCode;
    }

    public int getType() {
        return mType;
    }

    private static final class PhoneNumber {
        private int type;
        private String actualNumber;

        private PhoneNumber(int type, String actualNumber) {
            this.type = type;
            this.actualNumber = actualNumber;
        }

        private static PhoneNumber getActualPhoneNumber(Context context, String number) {
            PhoneNumber phoneNumber = null;
            String countryIso = TelephonyManager.getDefault().getNetworkCountryIso();
            if (!TextUtils.isEmpty(countryIso)) {
                // check by country ISO
                if ("cn".equals(countryIso)) {
                    // Chinese
                    phoneNumber = getChineseActualPhoneNumber(number);
                } else {
                    // other
                }
            } else {
                // check by locale if country ISO is empty
                Locale locale = context.getResources().getConfiguration().locale;
                if (Locale.CHINA.equals(locale)) {
                    // Chinese
                    phoneNumber = getChineseActualPhoneNumber(number);
                } else {
                    // other
                }
            }

            return phoneNumber;
        }

        /**
         * parse by Chinese rules
         *
         * @param number
         * @return
         */
        private static PhoneNumber getChineseActualPhoneNumber(String number) {

            // try to parse the mobile number
            if (number.length() >= 11) {
                String actualNumber = number.substring(number.length() - 11);
                // valid mobile number: 13xxxxxxxxx/15xxxxxxxxx/18xxxxxxxxx
                if (actualNumber.startsWith("13") || actualNumber.startsWith("15")
                        || actualNumber.startsWith("18")) {
                    return new PhoneNumber(TYPE_MOBILE, actualNumber.substring(0, 8));
                }
            }

            // try to parse area code
            char[] chars = number.toCharArray();
            int areaCodeIndex = -1;
            /**
             * if the number's length is more than 8 or start with "+", the area
             * code is in the position that the first 0 and with non-zero next
             * it;
             * for example, +86{021}10086, 17951{021}10086
             *
             * else check is the number start with the area code,
             * for example, {021}10086
             *
             * if not, the number is recognized as no area code number, and return null
             * for example, 10086
             **/
            if (chars.length > 8 || chars[0] == '+') {
                for (int index = Math.max(0, chars.length - 12);
                        index <= chars.length - 6; index++) {
                    if (chars[index] == '0' && chars[index + 1] != '0') {
                        areaCodeIndex = index;
                        break;
                    }
                }
            } else if (chars.length >= 6 && chars[0] == '0' && chars[1] != '0') {
                areaCodeIndex = 0;
            }
            if (areaCodeIndex != -1) {
                if ((chars[areaCodeIndex + 1] == '1') || chars[areaCodeIndex + 1] == '2') {
                    // capital etc. such as 010, 020
                    return new PhoneNumber(TYPE_LANDLINE, number.substring(areaCodeIndex,
                            areaCodeIndex + 3));
                } else {
                    // normal area code, such as 0523, 0512
                    return new PhoneNumber(TYPE_LANDLINE, number.substring(areaCodeIndex,
                            areaCodeIndex + 4));
                }
            }
            return null;
        }
    }

    /**
     * get the location of the number
     *
     * @param context
     * @param number
     * @return
     */
    public static GeocodedLocation getLocation(Context context, String number) {
        GeocodedLocation location = null;

        if (TextUtils.isEmpty(number)) {
            return location;
        }

        PhoneNumber phoneNumber = PhoneNumber.getActualPhoneNumber(context, number);
        if (phoneNumber == null) {
            return location;
        }

        Cursor c = null;
        try {
            if (phoneNumber.type == TYPE_LANDLINE) {
                // land line number.
                c = context.getContentResolver().query(CONTENT_URI, null, AreaCode.CODE + "=?",
                        new String[] {
                            phoneNumber.actualNumber
                        }, null);
            } else if (phoneNumber.type == TYPE_MOBILE) {
                // mobile number.
                c = context.getContentResolver().query(CONTENT_URI, null,
                        Section.NUMBER_START + "<=? and " + Section.NUMBER_END + ">=?",
                        new String[] {
                                phoneNumber.actualNumber, phoneNumber.actualNumber
                        }, null);
            }
            if (c != null && c.moveToNext()) {
                location = new GeocodedLocation(c, phoneNumber.type);
            }
        } finally {
            if (c != null && !c.isClosed()) {
                c.close();
            }
        }
        return location;
    }
}
