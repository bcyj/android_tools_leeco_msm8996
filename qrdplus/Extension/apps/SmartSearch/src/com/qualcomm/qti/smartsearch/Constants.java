/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

public class Constants {
    /**
     * Converted digits types for different locale
     */
    public static final int DIGITS_TYPE_UNKNOW = 0;
    public static final int DIGITS_TYPE_ARABIC = DIGITS_TYPE_UNKNOW + 1;
    public static final int DIGITS_TYPE_BAHASA = DIGITS_TYPE_ARABIC + 1;
    public static final int DIGITS_TYPE_BENGALI = DIGITS_TYPE_BAHASA + 1;
    public static final int DIGITS_TYPE_FARSI = DIGITS_TYPE_BENGALI + 1;
    public static final int DIGITS_TYPE_FRENCH = DIGITS_TYPE_FARSI + 1;
    public static final int DIGITS_TYPE_GERMAN = DIGITS_TYPE_FRENCH + 1;
    public static final int DIGITS_TYPE_GUJARATI = DIGITS_TYPE_GERMAN + 1;
    public static final int DIGITS_TYPE_HINDI = DIGITS_TYPE_GUJARATI + 1;
    public static final int DIGITS_TYPE_HONGKONG = DIGITS_TYPE_HINDI + 1;
    public static final int DIGITS_TYPE_ITALIAN = DIGITS_TYPE_HONGKONG + 1;
    public static final int DIGITS_TYPE_KANNADA = DIGITS_TYPE_ITALIAN + 1;
    public static final int DIGITS_TYPE_KHMER = DIGITS_TYPE_KANNADA + 1;
    public static final int DIGITS_TYPE_LATIN = DIGITS_TYPE_KHMER + 1;
    public static final int DIGITS_TYPE_MALAYALAM = DIGITS_TYPE_LATIN + 1;
    public static final int DIGITS_TYPE_MARATHI =DIGITS_TYPE_MALAYALAM + 1;
    public static final int DIGITS_TYPE_MYANMAR =DIGITS_TYPE_MARATHI + 1;
    public static final int DIGITS_TYPE_ORIYA =DIGITS_TYPE_MYANMAR + 1;
    public static final int DIGITS_TYPE_PINYIN = DIGITS_TYPE_ORIYA + 1;
    public static final int DIGITS_TYPE_POLISH = DIGITS_TYPE_PINYIN + 1;
    public static final int DIGITS_TYPE_PORTUGUESE_BRAZILIAN = DIGITS_TYPE_POLISH + 1;
    public static final int DIGITS_TYPE_RUSSIAN = DIGITS_TYPE_PORTUGUESE_BRAZILIAN + 1;
    public static final int DIGITS_TYPE_SERBIAN = DIGITS_TYPE_RUSSIAN + 1;
    public static final int DIGITS_TYPE_SLOVENIAN = DIGITS_TYPE_SERBIAN + 1;
    public static final int DIGITS_TYPE_SPANISH = DIGITS_TYPE_SLOVENIAN + 1;
    public static final int DIGITS_TYPE_SWAHILI =DIGITS_TYPE_SPANISH + 1;
    public static final int DIGITS_TYPE_TAGALOG = DIGITS_TYPE_SWAHILI + 1;
    public static final int DIGITS_TYPE_TAMIL = DIGITS_TYPE_TAGALOG + 1;
    public static final int DIGITS_TYPE_TELUGU = DIGITS_TYPE_TAMIL + 1;
    public static final int DIGITS_TYPE_THAI = DIGITS_TYPE_TELUGU + 1;
    public static final int DIGITS_TYPE_TURKEY = DIGITS_TYPE_THAI + 1;
    public static final int DIGITS_TYPE_UKRAINIAN =DIGITS_TYPE_TURKEY + 1;
    public static final int DIGITS_TYPE_URDU =DIGITS_TYPE_UKRAINIAN + 1;
    public static final int DIGITS_TYPE_VIETNAMESE = DIGITS_TYPE_URDU + 1;

    /**
     * Constants for searching and highlighting use
     */
    public static final char SYMBOL_UNKNOWN_CHAR = '.';
    public static final char SYMBOL_CHAR_BOUND_LEFT = '[';
    public static final char SYMBOL_CHAR_BOUND_RIGHT = ']';
    public static final char SYMBOL_SEPERATOR = '|';
    public static final char STROKE_WILDCARD = '6';
    public static final String STROKE_MATCH_MODE_NORMAL = "[1-5]+";
    public static final String STROKE_MATCH_MODE_WITH_WILDCARD_CHAR = "[1-6]+";
}
