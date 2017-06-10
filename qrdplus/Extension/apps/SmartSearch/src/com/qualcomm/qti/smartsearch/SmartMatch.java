/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.smartsearch;

import android.os.SystemProperties;
import android.text.TextUtils;

import java.util.ArrayList;
import java.util.Iterator;
import java.lang.Character.UnicodeBlock;

public class SmartMatch {

    // The display contact name
    private String mContactName;

    // The length of mContactName
    private int mContactNameLen;

    // The contact name converted to digits
    private String mDigitContactName;

    // the length of mDigitContactName
    private int mDigNameLen;

    // The digit type converted to digits
    private int mDigitType;

    // The digits that the user input
    private String mDigits;

    // Inputed digits length
    private int mDigitCnt;

    // The matched result
    private MatchResult mResult;

    /**
     * Convert contact name to digits
     *
     * @param contactName Contact name for converting
     * @return Digits string of this contact name
     */
    public String getNameNumber(String contactName, int inputType) {
        if (TextUtils.isEmpty(contactName)) {
            return null;
        }

        mDigitType = getDigitType();
        DigitMapTableLatin digitMapTable = getDigitMapTable(mDigitType);
        char[] chs = contactName.toCharArray();
        StringBuilder totalDigits = new StringBuilder();
        for (char c : chs) {
            String digits = digitMapTable.toDigits(c);
            totalDigits.append(digits);
        }

        return totalDigits.toString();
    }

    /*
     * Try to obtain contact name's digits type
     */
    /*private int guessNameStyle(String contactName) {
        if (TextUtils.isEmpty(contactName)) {
            return Constants.DIGITS_TYPE_UNKNOW;
        }

        int nameStyle = Constants.DIGITS_TYPE_UNKNOW;
        int length = contactName.length();
        int offset = 0;
        while (offset < length) {
            int codePoint = Character.codePointAt(contactName, offset);
            if (Character.isLetter(codePoint)) {
                UnicodeBlock unicodeBlock = UnicodeBlock.of(codePoint);

                if (unicodeBlock == UnicodeBlock.ARABIC) {
                    return Constants.DIGITS_TYPE_ARABIC;
                } else if (unicodeBlock == UnicodeBlock.BENGALI) {
                    return Constants.DIGITS_TYPE_BENGALI;
                } else if (unicodeBlock == UnicodeBlock.DEVANAGARI) {
                    return Constants.DIGITS_TYPE_HINDI;
                } else if (unicodeBlock == UnicodeBlock.CYRILLIC) {
                    return Constants.DIGITS_TYPE_RUSSIAN;
                } else if (unicodeBlock == UnicodeBlock.TAMIL) {
                    return Constants.DIGITS_TYPE_TAMIL;
                } else if (unicodeBlock == UnicodeBlock.TELUGU) {
                    return Constants.DIGITS_TYPE_TELUGU;
                } else if (unicodeBlock == UnicodeBlock.THAI) {
                    return Constants.DIGITS_TYPE_THAI;
                } else if (isCJKUnicodeBlock(unicodeBlock)) {
                    // For Chinese language, we should distinguish
                    // Simplified and HongKong type.
                    String locale = getSystemLocale();
                    if (locale.equals("zh_CN"))
                        return Constants.DIGITS_TYPE_PINYIN;
                    else if (locale.equals("zh_HK"))
                        return Constants.DIGITS_TYPE_HONGKONG;
                    else // Default
                        return Constants.DIGITS_TYPE_PINYIN;
                } else if (isLatinUnicodeBlock(unicodeBlock)) {
                    nameStyle = Constants.DIGITS_TYPE_LATIN;
                }
            }
            offset += Character.charCount(codePoint);
        }

        return nameStyle;
    }*/

    private int getDigitType() {
        String locale = getSystemLocale();
        if (TextUtils.isEmpty(locale))
            return Constants.DIGITS_TYPE_UNKNOW;
        else if (locale.equals("ar_EG"))
            return Constants.DIGITS_TYPE_ARABIC;
        else if (locale.equals("in_ID"))
            return Constants.DIGITS_TYPE_BAHASA;
        else if (locale.equals("bn_IN"))
            return Constants.DIGITS_TYPE_BENGALI;
        else if (locale.equals("fa_AF") || locale.equals("fa_IR"))
            return Constants.DIGITS_TYPE_FARSI;
        else if (locale.equals("fr_FR"))
            return Constants.DIGITS_TYPE_FRENCH;
          else if (locale.equals("de_DE"))
            return Constants.DIGITS_TYPE_GERMAN;
        else if (locale.equals("gu_IN"))
            return Constants.DIGITS_TYPE_GUJARATI;
        else if (locale.equals("hi_IN"))
            return Constants.DIGITS_TYPE_HINDI;
        else if (locale.equals("zh_HK"))
            return Constants.DIGITS_TYPE_HONGKONG;
        else if (locale.equals("it_IT"))
            return Constants.DIGITS_TYPE_ITALIAN;
        else if (locale.equals("kn_IN"))
            return Constants.DIGITS_TYPE_KANNADA;
        else if (locale.equals("km_KH"))
            return Constants.DIGITS_TYPE_KHMER;
        else if (locale.equals("en_US"))
            return Constants.DIGITS_TYPE_LATIN;
        else if (locale.equals("ml_IN"))
            return Constants.DIGITS_TYPE_MALAYALAM;
        else if (locale.equals("mr_IN"))
            return Constants.DIGITS_TYPE_MARATHI;
        else if (locale.equals("my_MM"))
            return Constants.DIGITS_TYPE_MYANMAR;
        else if (locale.equals("or_IN"))
            return Constants.DIGITS_TYPE_ORIYA;
        else if (locale.equals("zh_CN"))
            return Constants.DIGITS_TYPE_PINYIN;
        else if (locale.equals("pl_PL"))
            return Constants.DIGITS_TYPE_POLISH;
        else if (locale.equals("pt_BR"))
            return Constants.DIGITS_TYPE_PORTUGUESE_BRAZILIAN;
        else if (locale.equals("ru_RU"))
            return Constants.DIGITS_TYPE_RUSSIAN;
        else if (locale.equals("sr_BA") || locale.equals("sr_CS")
                || locale.equals("sr_ME") || locale.equals("sr_RS"))
            return Constants.DIGITS_TYPE_SERBIAN;
        else if (locale.equals("sl_SL"))
            return Constants.DIGITS_TYPE_SLOVENIAN;
        else if (locale.equals("es_US"))
            return Constants.DIGITS_TYPE_SPANISH;
        else if (locale.equals("sw_KE") || locale.equals("sw_TZ") || locale.equals("sw_UG"))
            return Constants.DIGITS_TYPE_SWAHILI;
        else if (locale.equals("tl_PH"))
            return Constants.DIGITS_TYPE_TAGALOG;
        else if (locale.equals("ta_IN"))
            return Constants.DIGITS_TYPE_TAMIL;
        else if (locale.equals("te_IN"))
            return Constants.DIGITS_TYPE_TELUGU;
        else if (locale.equals("th_TH"))
            return Constants.DIGITS_TYPE_THAI;
        else if (locale.equals("tr_TR"))
            return Constants.DIGITS_TYPE_TURKEY;
        else if (locale.equals("uk_UA"))
            return Constants.DIGITS_TYPE_UKRAINIAN;
        else if (locale.equals("ur_IN") || locale.equals("ur_PK"))
            return Constants.DIGITS_TYPE_URDU;
        else if (locale.equals("vi_VN"))
            return Constants.DIGITS_TYPE_VIETNAMESE;
        else
            return Constants.DIGITS_TYPE_UNKNOW;
    }

    /**
     * Get the  system's locale (like en_US)
     */
    private String getSystemLocale() {
        StringBuilder builder = new StringBuilder();

        // Get current locale
        builder.append(SystemProperties.get("persist.sys.language", "" ));
        String country = SystemProperties.get("persist.sys.country", "" );
        if (!country.equals("")) {
            builder.append("_");
            builder.append(country);
        }

        // Get orignal locale
        if (builder.equals("")) {
            builder.append(SystemProperties.get("ro.product.locale.language", "" ));
            String region = SystemProperties.get("ro.product.locale.region", "" );
            if (!region.equals("")) {
                builder.append("_");
                builder.append(region);
            }
        }

        return builder.toString();
    }

    /*private boolean isLatinUnicodeBlock(UnicodeBlock unicodeBlock) {
        return unicodeBlock == UnicodeBlock.BASIC_LATIN ||
                unicodeBlock == UnicodeBlock.LATIN_1_SUPPLEMENT ||
                unicodeBlock == UnicodeBlock.LATIN_EXTENDED_A ||
                unicodeBlock == UnicodeBlock.LATIN_EXTENDED_B ||
                unicodeBlock == UnicodeBlock.LATIN_EXTENDED_ADDITIONAL;
    }

    private boolean isCJKUnicodeBlock(UnicodeBlock block) {
        return block == UnicodeBlock.CJK_UNIFIED_IDEOGRAPHS
                || block == UnicodeBlock.CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A
                || block == UnicodeBlock.CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B
                || block == UnicodeBlock.CJK_SYMBOLS_AND_PUNCTUATION
                || block == UnicodeBlock.CJK_RADICALS_SUPPLEMENT
                || block == UnicodeBlock.CJK_COMPATIBILITY
                || block == UnicodeBlock.CJK_COMPATIBILITY_FORMS
                || block == UnicodeBlock.CJK_COMPATIBILITY_IDEOGRAPHS
                || block == UnicodeBlock.CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT;
    }*/

    private DigitMapTableLatin getDigitMapTable(int type) {
        switch (type) {
            case Constants.DIGITS_TYPE_UNKNOW:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_ARABIC:
                return DigitMapTableArabic.getInstance();
            case Constants.DIGITS_TYPE_BAHASA:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_BENGALI:
                return DigitMapTableBengali.getInstance();
            case Constants.DIGITS_TYPE_FARSI:
                return DigitMapTableFarsi.getInstance();
            case Constants.DIGITS_TYPE_FRENCH:
                return DigitMapTableFrench.getInstance();
            case Constants.DIGITS_TYPE_GERMAN:
                return DigitMapTableGerman.getInstance();
            case Constants.DIGITS_TYPE_GUJARATI:
                return DigitMapTableGujarati.getInstance();
            case Constants.DIGITS_TYPE_HINDI:
                return DigitMapTableHindi.getInstance();
            case Constants.DIGITS_TYPE_HONGKONG:
                return DigitMapTableHongKong.getInstance();
            case Constants.DIGITS_TYPE_ITALIAN:
                return DigitMapTableItalian.getInstance();
            case Constants.DIGITS_TYPE_KANNADA:
                return DigitMapTableKannada.getInstance();
            case Constants.DIGITS_TYPE_KHMER:
                return DigitMapTableKhmer.getInstance();
            case Constants.DIGITS_TYPE_LATIN:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_MALAYALAM:
                return DigitMapTableMalayalam.getInstance();
            case Constants.DIGITS_TYPE_MARATHI:
                return DigitMapTableMarathi.getInstance();
            case Constants.DIGITS_TYPE_MYANMAR:
                return DigitMapTableMyanmar.getInstance();
            case Constants.DIGITS_TYPE_ORIYA:
                return DigitMapTableOriya.getInstance();
            case Constants.DIGITS_TYPE_PINYIN:
                return DigitMapTablePinyin.getInstance();
            case Constants.DIGITS_TYPE_POLISH:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_PORTUGUESE_BRAZILIAN:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_RUSSIAN:
                return DigitMapTableRussian.getInstance();
            case Constants.DIGITS_TYPE_SERBIAN:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_SLOVENIAN:
                return  DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_SPANISH:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_SWAHILI:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_TAGALOG:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_TAMIL:
                return DigitMapTableTamil.getInstance();
            case Constants.DIGITS_TYPE_TELUGU:
                return DigitMapTableTelugu.getInstance();
            case Constants.DIGITS_TYPE_THAI:
                return DigitMapTableThai.getInstance();
            case Constants.DIGITS_TYPE_TURKEY:
                return DigitMapTableLatin.getInstance();
            case Constants.DIGITS_TYPE_UKRAINIAN:
                return DigitMapTableUkrainian.getInstance();
            case Constants.DIGITS_TYPE_URDU:
                return DigitMapTableUrdu.getInstance();
            case Constants.DIGITS_TYPE_VIETNAMESE:
                return DigitMapTableLatin.getInstance();
            default:
                return DigitMapTableLatin.getInstance();
        }
    }

    /**
     * Get the matched string index
     *
     * @param digits User inputed digits
     * @param contactName Contact's display name
     * @return Matched string index
     */
    public int[] getMatchStringIndex(String digits, String contactName, int inputType) {

        if (TextUtils.isEmpty(digits) || TextUtils.isEmpty(contactName)) {
            return null;
        }

        mContactName = contactName;
        mContactNameLen = contactName.length();

        String digName = getNameNumber(contactName, inputType);
        if (TextUtils.isEmpty(digName)) {
            return null;
        }

        mDigitContactName = digName;
        mDigNameLen = digName.length();
        mDigits = digits;
        mDigitCnt = digits.length();

        int[] matchBits = null;

        if (mDigitType == Constants.DIGITS_TYPE_HONGKONG) {
            // For HongKong, first search the strokes
            matchBits = getMatchStrokeBits();
            // If have no match, search in Latin again
            if (matchBits == null) {
                // Covert HongKong digits to Latin digits
                mDigitContactName = mDigitContactName.replaceAll("\\[[1-5]+\\]",
                        "" + Constants.SYMBOL_UNKNOWN_CHAR);
		  mDigNameLen = mDigitContactName.length();
                // Check whether has Latin digits
                if (mDigitContactName.substring(0).matches("[.]+")) {
                    return null;
                }
                matchBits = getMatchNormalBits();
            }
        } else {
            matchBits = getMatchNormalBits();
        }

        // Convert match bits array to position index array
        if (matchBits != null) {
            ArrayList<Integer> posIndex = new ArrayList<Integer>();
            for (int i = 0; i < matchBits.length; i++) {
                if (matchBits[i] == 1) {
                    if (i == 0 || matchBits[i - 1] == 0) // "01..."
                        posIndex.add(i);
                    if (i == matchBits.length - 1 || matchBits[i + 1] == 0) // "...10"
                        posIndex.add(i);
                }
            }

            int[] index = null;
            if (posIndex.size() != 0) {
                index = new int[posIndex.size()];
                int idx = 0;
                Iterator<Integer> it = posIndex.iterator();
                while (it.hasNext()) {
                    index[idx++] = it.next().intValue();
                }
            }

            return index;
        } else {
            return null;
        }
    }

    /**
     * Get the matched bits array for normal type (one unicode one digit)
     *
     * @return Matched bits array
     */
    private int[] getMatchNormalBits() {

        int[] matchBits = null;

        int matchedCnt = 0;
        int priority = 0, maxPriority = Integer.MIN_VALUE;
        // Matched result with maximum priority
        MatchResult maxPriorityResult = null;

        // Initialize a match result (no digit is matched)
        initMatchResult();
        mResult.setDigitName(mDigitContactName);
        while (true) {
            // Match from current result mResult
            boolean matched = matchFromCurResult();
            if (matched) {
                matchedCnt++;
                priority = getMatchResultPriority(mResult);
                if (priority > maxPriority) {
                    maxPriority = priority;
                    maxPriorityResult = mResult.copy();
                }
            }

            // Let mResult change to the next valid result for a further
            // match
            boolean hasMoreResult = toNextMatchedResult();

            if (!hasMoreResult) {
                break;
            }
        }
        maxPriority = 0;

        if (matchedCnt != 0 && maxPriorityResult != null) {
            // For pinyin type, should convert pinyin result bits
            // to map display contact name
            if (mDigitType == Constants.DIGITS_TYPE_PINYIN) {
                int[] matchPinyinBits = maxPriorityResult.getResultBits();
                matchBits = new int[mContactNameLen];
                int index = 0;
                boolean startMark = false;
                boolean inPinyinToken = false;
                for (int i = 0; i < mDigNameLen; i++) {
                    char dn = mDigitContactName.charAt(i);

                    if (dn == Constants.SYMBOL_CHAR_BOUND_LEFT) {
                        if (matchPinyinBits[i + 1] == 1) {
                            inPinyinToken = true;
                        }
                        startMark = true;
                    } else if (dn == Constants.SYMBOL_CHAR_BOUND_RIGHT) {
                        if (inPinyinToken) {
                            matchBits[index++] = 1;
                            inPinyinToken = false;
                        } else {
                            matchBits[index++] = 0;
                        }
                        startMark = false;
                    } else if (!startMark) { // For Latin
                        if (matchPinyinBits[i] == 1)
                            matchBits[index++] = 1;
                        else
                            matchBits[index++] = 0;
                    }
                }
            } else {
                matchBits = maxPriorityResult.getResultBits();
            }
        }

        return matchBits;
    }

    private boolean isDigit(char c) {
        return !(c == Constants.SYMBOL_UNKNOWN_CHAR ||
                c == Constants.SYMBOL_CHAR_BOUND_LEFT || c == Constants.SYMBOL_CHAR_BOUND_RIGHT);
    }

    /**
     * Calculate the priority of matched result for returning the best match.
     *
     * @param matchResult A matched result
     * @return priority
     */
    private int getMatchResultPriority(MatchResult matchResult) {

        if (matchResult == null) {
            return -1;
        }

        String matchedDigName = matchResult.getDigitName();

        int nameLen = matchedDigName.length();
        int matchedNameLen = matchResult.getBitsLen();

        int partialMatchedTokenCount = 0;
        boolean lastInToken = false;
        boolean inToken = false;
        boolean fullMatch = true;
        int matchedCount = 0;
        int charCountOfToken = 0;
        int discreteToken = 0;
        boolean previousTokenMatched = true;
        boolean thisTokenMatched = false;
        boolean everMatched = false;

        for (int i = 0; i < nameLen && i < matchedNameLen; i++) {
            char c = matchedDigName.charAt(i);
            inToken = isDigit(c) ? true : false;

            // Token end
            if (!inToken && lastInToken) {
                partialMatchedTokenCount += fullMatch ? 0 : (thisTokenMatched
                        && charCountOfToken > 1 ? 1 : 0);
                previousTokenMatched = thisTokenMatched;
                // Initiate values for next token
                fullMatch = true;
                matchedCount = 0;
                charCountOfToken = 0;
            }

            if (inToken) {
                charCountOfToken++;
                if (!matchResult.isSetBit(i)) {
                    fullMatch = false;
                } else {
                    matchedCount++;
                }
            }

            // Token start
            if (inToken && !lastInToken) {
                thisTokenMatched = matchedCount > 0;
                if (!previousTokenMatched && thisTokenMatched && everMatched) {
                    discreteToken++;
                }
                if (thisTokenMatched) {
                    everMatched = true;
                }
            }

            lastInToken = inToken;
        }

        if (lastInToken) {
            partialMatchedTokenCount += fullMatch ? 0 : (thisTokenMatched
                    && charCountOfToken > 1 ? 1 : 0);
        }

        // Base priority
        float priority = 2000;

        // Prefer less partial matched word
        priority -= (partialMatchedTokenCount - 1) * 100;
        // Prefer match at left
        priority -= matchResult.getFirstSetBitPos() * 50;

        // Don't like skipping match
        if (discreteToken > 0) {
            priority -= discreteToken * 200;
        }

        return (int) priority;
    }

    private void initMatchResult() {
        if (TextUtils.isEmpty(mDigitContactName)) {
            return;
        }
        mResult = new MatchResult(mDigitContactName.length());
    }

    /**
     * Try to move to the next partially matched result
     */
    private boolean toNextMatchedResultOneChar() {
        int pos = mResult.getLastSetBitPos();
        char c;

        if (pos >= 0) {
            c = mDigitContactName.charAt(pos);
        } else {
            c = mDigits.charAt(0);
        }

        mResult.unsetBit(pos);
        pos++;

        if (pos >= mDigNameLen) {
            return false;
        }

        boolean matchDigitAreaStart = true;
        for (; pos < mDigNameLen; pos++) {
            if (matchDigitAreaStart && !atDigitAreaStart(pos)) {
                continue;
            }

            matchDigitAreaStart = true;
            if (mDigitContactName.charAt(pos) == c) {
                mResult.setBit(pos);
                return true;
            }
        }
        return false;
    }

    private boolean toNextMatchedResult() {
        int pos = mResult.getLastSetBitPos();

        while (true) {
            boolean matched = toNextMatchedResultOneChar();
            if (matched) {
                return true;
            }

            pos = mResult.getLastSetBitPos();
            if (pos < 0) {
                return false;
            }
        }
    }

    private boolean atSplit(int pos) {
        if (pos < 0) {
            return true;
        }
        char c = mDigitContactName.charAt(pos);
        return (c == Constants.SYMBOL_UNKNOWN_CHAR);
    }

    private boolean atDigit(int pos) {
        if (pos < 0 || pos >= mDigNameLen) {
            return false;
        }
        char c = mDigitContactName.charAt(pos);
        return isDigit(c);
    }

    private boolean atDigitAreaStart(int pos) {
        return pos < 0 || atDigit(pos) && !atDigit(pos - 1);
    }

    private boolean matchADigitFromPos(int pos, char c) {
        boolean matchDigitAreaStart = true;
        if (pos > 0 && !atDigitAreaStart(pos)) {
            if (mResult.isSetBit(pos - 1)) {
                matchDigitAreaStart = false;
            }
        }
        for (; pos < mDigNameLen; pos++) {
            if (matchDigitAreaStart && !atDigitAreaStart(pos)) {
                continue;
            }
            matchDigitAreaStart = true;
            if (atSplit(pos)) {
                continue;
            }
            if (mDigitContactName.charAt(pos) == c) {
                mResult.setBit(pos);
                return true;
            }
        }
        return false;
    }

    private boolean matchFromCurResult() {

        int pos = mResult.getLastSetBitPos() + 1;
        int cnt = mResult.getSetBitsCount();

        boolean matched;

        while (cnt < mDigitCnt) {
            // Try to match a more digit from last
            matched = matchADigitFromPos(pos, mDigits.charAt(cnt));
            if (!matched) {
                return false;
            }

            pos = mResult.getLastSetBitPos() + 1;
            cnt++;
        }
        return true;
    }

    /**
     * Get the matched bits array for stroke type (one unicode multi digits)
     * @return Matched bits array
     */
    private int[] getMatchStrokeBits() {

        // Check whether stroke input is OK
        if (!mDigits.substring(0).matches(Constants.STROKE_MATCH_MODE_WITH_WILDCARD_CHAR)) {
            return null;
        }

        // Obtain Chinese character token order list
        ArrayList<Integer> tokenPosition = matchStroke(0, 0, -1);

        // Transfer token position list to match bits
        if (tokenPosition == null || tokenPosition.isEmpty())
            return null;

        int[] matchBits = new int[mContactNameLen];
        int index = 0;

        int tokenPos = -1;
        boolean startMark = false;

        Iterator<Integer> it = tokenPosition.iterator();
        int pos = -1;
        if (it.hasNext())
            pos = it.next().intValue();

        for (int i = 0; i < mDigNameLen; i++) {
            char dn = mDigitContactName.charAt(i);

            if (dn == Constants.SYMBOL_CHAR_BOUND_LEFT) {
                ++tokenPos;
                startMark = true;
            } else if (dn == Constants.SYMBOL_CHAR_BOUND_RIGHT) {
                if (tokenPos == pos) {
                    matchBits[index++] = 1;
                    if (it.hasNext())
                        pos = it.next().intValue();
                } else {
                    matchBits[index++] = 0;
                }
                startMark = false;
            } else if (!startMark) {
                matchBits[index++] = 0;
            }
        }

        return matchBits;
    }

    /**
     * Obtain matched Chinese character token order list
     *
     * @param dnStart The start position of digitName
     * @param dStart The start position of digits
     * @param tokenPos The current token position
     * @return Matched Chinese character token order list
     */
    private ArrayList<Integer> matchStroke(int dnStart, int dStart, int tokenPos) {

        ArrayList<Integer> tokenPosition = new ArrayList<Integer>();

        int pDigits = -1;
        int curLeftBracketCharPos = -1;
        boolean startMatch = false;
        boolean lastIsFullMatch = false;
        boolean matchFromSrcHead = true;

        for (int i = dnStart; i < mDigNameLen; i++) {
            char dn = mDigitContactName.charAt(i);

            // Token start
            if (dn == Constants.SYMBOL_CHAR_BOUND_LEFT) {
                curLeftBracketCharPos = i;
                if (matchFromSrcHead)
                    pDigits = dStart;
                tokenPos++;
                startMatch = true;
            } else if (startMatch) {
                char digit = mDigits.charAt(pDigits);

                // Token end
                if (dn == Constants.SYMBOL_CHAR_BOUND_RIGHT) {
                    tokenPosition.add(tokenPos);

                    lastIsFullMatch = true;
                    matchFromSrcHead = false;

                    // For next token, run this algorithm recursively
                    if ((i + 1) < mDigNameLen - 1 && pDigits < mDigitCnt) {
                        ArrayList<Integer> position =
                                matchStroke(i + 1, pDigits, tokenPos);
                        if (position != null && !position.isEmpty()) {
                            tokenPosition.addAll(position);
                            return tokenPosition;
                        } else
                            startMatch = false;
                    }
                } else if (dn == digit || Constants.STROKE_WILDCARD == digit) { // Matched
                    if (pDigits < mDigitCnt - 1)
                        pDigits++;
                    else {
                        tokenPosition.add(tokenPos);
                        return tokenPosition;
                    }
                } else { // Not matched
                    matchFromSrcHead = true;
                    if (lastIsFullMatch) {
                        tokenPosition.clear();
                        i = curLeftBracketCharPos;
                        pDigits = dStart;
                        lastIsFullMatch = false;
                    } else
                        startMatch = false;
                }
            }
        }
        return null;
    }
}
