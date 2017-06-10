/******************************************************************************
 * @file    QcNvItemTypes.java
 * @brief   Contains NV item type definitions used in the Modem.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2009 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 *******************************************************************************/

package com.qualcomm.qcnvitems;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.security.InvalidParameterException;

public class QcNvItemTypes {

    public static final String LOG_TAG = "QcNvItemTypes";

    // size of basic data types
    public static final int SIZE_OF_BYTE = 1;

    public static final int SIZE_OF_SHORT = 2;

    public static final int SIZE_OF_INT = 4;

    // some primitive types
    public static class NvByte extends BaseQCNvItemType {
        byte mVal;

        NvByte() {
            this.mVal = 0;
        }

        NvByte(byte[] bArray) {
            this.mVal = bArray[0];
        }

        public int getSize() {
            return SIZE_OF_BYTE;
        }

        public String toDebugString() {
            return String.format("val=%d", mVal);
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mVal);
            return buf.array();
        }
    }

    public static class NvShort extends BaseQCNvItemType {
        short mVal;

        NvShort() {
            this.mVal = 0;
        }

        NvShort(byte[] bArray) {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mVal = buf.getShort();
        }

        public int getSize() {
            return SIZE_OF_SHORT;
        }

        public String toDebugString() {
            return String.format("val=%d", mVal);
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.putShort(mVal);
            return buf.array();
        }
    }

    public static class NvInteger extends BaseQCNvItemType {
        int mVal;

        NvInteger() {
            this.mVal = 0;
        }

        NvInteger(byte[] bArray) {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mVal = buf.getInt();
        }

        public int getSize() {
            return SIZE_OF_INT;
        }

        public String toDebugString() {
            return String.format("val=%d", mVal);
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.putInt(mVal);
            return buf.array();
        }
    }

    // nv_min1_type
    public static class NvMin1Type extends BaseQCNvItemType {
        byte mNam;

        int[] mMin1;

        NvMin1Type() {
            mNam = 0;
            mMin1 = new int[IQcNvItems.NV_MAX_MINS];
        }

        NvMin1Type(byte[] bArray) throws IOException {
            mMin1 = new int[IQcNvItems.NV_MAX_MINS];
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            skipPaddingBytes(buf, 3);
            this.mMin1[0] = buf.getInt();
            this.mMin1[1] = buf.getInt();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mNam);
            addPaddingBytes(buf, 3);
            buf.putInt(mMin1[0]);
            buf.putInt(mMin1[1]);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE + IQcNvItems.NV_MAX_MINS * SIZE_OF_INT + 3 /* padding */);
        }

        public String toDebugString() {
            return String.format("nam:%x, min1[0]:%x, min1[1]:%x", mNam, mMin1[0], mMin1[1]);
        }
    };

    // nv_min2_type
    public static class NvMin2Type extends BaseQCNvItemType {
        byte mNam;

        short[] mMin2;

        NvMin2Type() {
            mNam = 0;
            mMin2 = new short[IQcNvItems.NV_MAX_MINS];
        }

        NvMin2Type(byte[] bArray) throws IOException {
            mMin2 = new short[IQcNvItems.NV_MAX_MINS];
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            skipPaddingBytes(buf, 1);
            this.mMin2[0] = buf.getShort();
            this.mMin2[1] = buf.getShort();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mNam);
            addPaddingBytes(buf, 1);
            buf.putShort(mMin2[0]);
            buf.putShort(mMin2[1]);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE + IQcNvItems.NV_MAX_MINS * SIZE_OF_SHORT + 1 /* padding */);
        }

        public String toDebugString() {
            return String.format("nam:%x, min1[0]:%x, min1[1]:%x", mNam, mMin2[0], mMin2[1]);
        }
    };

    // nv_imsi_11_12_type
    public static class NvImsi1112Type extends BaseQCNvItemType {
        byte mNam;

        byte mImsi1112;

        public NvImsi1112Type() {
            this.mNam = 0;
            this.mImsi1112 = 0;
        }

        NvImsi1112Type(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            this.mImsi1112 = buf.get();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mNam);
            buf.put(mImsi1112);
            return buf.array();
        }

        public int getSize() {
            return (2 * SIZE_OF_BYTE);
        }

        public String toDebugString() {
            return String.format("nam:%x, imsi_11_12:%d", mNam, mImsi1112);
        }
    };

    // nv_dir_number_type
    public static class NvDirNumberType extends BaseQCNvItemType {
        byte mNam;

        byte mDirNumber[] = new byte[IQcNvItems.NV_DIR_NUMB_SIZ];

        public NvDirNumberType() {
            mNam = 0;
            mDirNumber = new byte[IQcNvItems.NV_DIR_NUMB_SIZ];
        }

        NvDirNumberType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            for (int i = 0; i < IQcNvItems.NV_DIR_NUMB_SIZ; i++)
                this.mDirNumber[i] = buf.get();
        }

        public void setDirNumber(String in) throws InvalidParameterException {

            if (in.length() != IQcNvItems.NV_DIR_NUMB_SIZ)
                throw new InvalidParameterException();

            for (int i = 0; i < IQcNvItems.NV_DIR_NUMB_SIZ; i++) {
                Character c = in.charAt(i);
                if (!Character.isDigit(c))
                    throw new InvalidParameterException();
                mDirNumber[i] = (byte)in.charAt(i);
            }
        }

        public String getDirNumber() {
            String ret = "";
            for (int i = 0; i < IQcNvItems.NV_DIR_NUMB_SIZ; i++)
                ret += (char)mDirNumber[i];
            return ret;
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mNam);
            for (int i = 0; i < IQcNvItems.NV_DIR_NUMB_SIZ; i++)
                buf.put(this.mDirNumber[i]);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE + IQcNvItems.NV_DIR_NUMB_SIZ * SIZE_OF_BYTE);
        }

        public String toDebugString() {
            return String.format("nam:%x, dir_number:%s", mNam, getDirNumber());
        }
    };

    // nv_sid_nid_pair_type
    public static class NvSidNidPairType extends BaseQCNvItemType {
        short mSid;

        short mNid;

        NvSidNidPairType() {
            this.mSid = 0;
            this.mNid = 0;
        }

        public NvSidNidPairType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            mSid = buf.getShort();
            mNid = buf.getShort();
        }

        public int getSize() {
            return SIZE_OF_SHORT * 2;
        }

        public String toDebugString() {
            return String.format("SID:%d, NID:%d", mSid, mNid);
        }

        public byte[] toByteArray() {
            return pack(mSid, mNid);
        }
    }

    // nv_sid_type
    public static class NvSidType extends BaseQCNvItemType {
        byte mNam;

        short mSid;

        public NvSidType() {
            this.mNam = 0;
            this.mSid = 0;
        }

        public NvSidType(byte[] bArray) {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            skipPaddingBytes(buf, 1);
            this.mSid = buf.getShort();
        }

        public int getSize() {
            return SIZE_OF_BYTE + SIZE_OF_SHORT + 1 /* padding */;
        }

        public String toDebugString() {
            return String.format("NAM: %d, SID: %d", mNam, mSid);
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(this.mNam);
            addPaddingBytes(buf, 1);
            buf.putShort(this.mSid);
            return buf.array();
        }
    }

    // nv_sid_nid_type
    public static class NvSidNidType extends BaseQCNvItemType {
        byte mNam;

        NvSidNidPairType mPair[][];

        public NvSidNidType() {
            this.mNam = 0;
            this.mPair = new NvSidNidPairType[IQcNvItems.NV_MAX_MINS][IQcNvItems.NV_MAX_SID_NID];
        }

        public NvSidNidType(byte[] bArray) {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            skipPaddingBytes(buf, 1);
            this.mPair = new NvSidNidPairType[IQcNvItems.NV_MAX_MINS][IQcNvItems.NV_MAX_SID_NID];
            for (int i = 0; i < IQcNvItems.NV_MAX_MINS; i++) {
                for (int j = 0; j < IQcNvItems.NV_MAX_SID_NID; j++) {
                    this.mPair[i][j] = new NvSidNidPairType();
                    this.mPair[i][j].mSid = buf.getShort();
                    this.mPair[i][j].mNid = buf.getShort();
                }
            }
        }

        public int getSize() {
            return SIZE_OF_BYTE + IQcNvItems.NV_MAX_MINS * IQcNvItems.NV_MAX_SID_NID
                    * SIZE_OF_SHORT * 2 + 1 /* Padding */;
        }

        public String toDebugString() {
            String ppString = "";

            for (int i = 0; i < IQcNvItems.NV_MAX_MINS; i++) {
                ppString += "[";
                for (int j = 0; j < IQcNvItems.NV_MAX_SID_NID; j++) {
                    ppString = ppString + mPair[i][j].toDebugString() + ", ";
                }
                ppString += "]";
            }
            return ppString;
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(this.mNam);
            addPaddingBytes(buf, 1);
            for (int i = 0; i < IQcNvItems.NV_MAX_MINS; i++) {
                for (int j = 0; j < IQcNvItems.NV_MAX_SID_NID; j++) {
                    buf.put(mPair[i][j].toByteArray());
                }
            }
            return buf.array();
        }
    }

    // nv_home_sid_nid_type
    public static class NvHomeSidNidType extends BaseQCNvItemType {
        byte mNam;

        NvSidNidPairType mPair[];

        public NvHomeSidNidType() {
            this.mNam = 0;
            this.mPair = new NvSidNidPairType[IQcNvItems.NV_MAX_HOME_SID_NID];
        }

        public NvHomeSidNidType(byte[] bArray) {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            skipPaddingBytes(buf, 1);
            this.mPair = new NvSidNidPairType[IQcNvItems.NV_MAX_HOME_SID_NID];
            for (int j = 0; j < IQcNvItems.NV_MAX_HOME_SID_NID; j++) {
                this.mPair[j] = new NvSidNidPairType();
                this.mPair[j].mSid = buf.getShort();
                this.mPair[j].mNid = buf.getShort();
            }
        }

        public int getSize() {
            return SIZE_OF_BYTE + IQcNvItems.NV_MAX_HOME_SID_NID * SIZE_OF_SHORT * 2 + 1 /* padding */;
        }

        public String toDebugString() {
            String ppString = "";

            ppString += "[";
            for (int j = 0; j < IQcNvItems.NV_MAX_HOME_SID_NID; j++) {
                ppString = ppString + mPair[j].toDebugString() + ", ";
            }
            ppString += "]";

            return ppString;
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(this.mNam);
            addPaddingBytes(buf, 1);
            for (int j = 0; j < IQcNvItems.NV_MAX_HOME_SID_NID; j++) {
                buf.put(mPair[j].toByteArray());
            }
            return buf.array();
        }
    }

    // nv_imsi_mcc_type
    public static class NvImsiMccType extends BaseQCNvItemType {
        byte mNam;

        short mImsiMcc;

        NvImsiMccType() throws InvalidParameterException {
            this.mNam = 0;
            this.mImsiMcc = 0;
        }

        NvImsiMccType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            skipPaddingBytes(buf, 1);
            this.mImsiMcc = buf.getShort();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(this.mNam);
            addPaddingBytes(buf, 1);
            buf.putShort(mImsiMcc);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE + SIZE_OF_SHORT + 1 /* padding */);
        }

        public String toDebugString() {
            return String.format("nam:%x, imsi_mcc:%d", mNam, mImsiMcc);
        }
    };

    // nv_imsi_addr_num_type
    public static class NvImsiAddrNumType extends BaseQCNvItemType {
        byte mNam;

        byte mNum;

        NvImsiAddrNumType() {
            this.mNam = 0;
            this.mNum = 0;
        }

        NvImsiAddrNumType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            this.mNum = buf.get();
        }

        public byte[] toByteArray() {
            return pack(mNam, mNum);
        }

        public int getSize() {
            return (2 * SIZE_OF_BYTE);
        }

        public String toDebugString() {
            return String.format("nam:%x, num:%d", mNam, mNum);
        }
    };

    // nv_imsi_type
    public static class NvImsiType extends BaseQCNvItemType {
        byte mNam;

        int mImsi;

        NvImsiType() {
            this.mNam = 0;
            this.mImsi = 0;
        }

        NvImsiType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            this.mImsi = buf.getInt();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mNam);
            addPaddingBytes(buf, 3);
            buf.putInt(mImsi);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE + SIZE_OF_INT + 3 /* padding */);
        }

        public String toDebugString() {
            return String.format("nam:%x, imsi:%d", mNam, mImsi);
        }
    };

    // nv_cdmach_type
    public static class NvCdmaChType extends BaseQCNvItemType {
        byte mNam;

        short mChannelA;

        short mChannelB;

        NvCdmaChType() throws InvalidParameterException {
            this.mNam = 0;
            this.mChannelA = 0;
            this.mChannelB = 0;
        }

        NvCdmaChType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            skipPaddingBytes(buf, 1);
            this.mChannelA = buf.getShort();
            this.mChannelB = buf.getShort();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(mNam);
            addPaddingBytes(buf, 1);
            buf.putShort(mChannelA);
            buf.putShort(mChannelB);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE + SIZE_OF_SHORT * 2 + 1 /* padding */);
        }

        public String toDebugString() {
            return String.format("nam:%x, channel_a:%d, channel_b:%d", mNam, mChannelA, mChannelB);
        }
    };

    // nv_sec_code_type
    public static class NvSecCodeType extends BaseQCNvItemType {
        byte mDigits[];

        NvSecCodeType() {
            mDigits = new byte[IQcNvItems.NV_SEC_CODE_SIZE];
        }

        NvSecCodeType(byte[] bArray) throws IOException {
            mDigits = new byte[IQcNvItems.NV_SEC_CODE_SIZE];
            ByteBuffer buf = createByteBuffer(bArray);
            for (int i = 0; i < IQcNvItems.NV_SEC_CODE_SIZE; i++)
                this.mDigits[i] = buf.get();
        }

        public void setSecCode(String in) throws InvalidParameterException {
            if (in == null || in.length() != IQcNvItems.NV_SEC_CODE_SIZE)
                throw new InvalidParameterException();

            for (int i = 0; i < IQcNvItems.NV_SEC_CODE_SIZE; i++)
                this.mDigits[i] = (byte)in.charAt(i);
        }

        public String getSecCode() {
            String ret = "";
            for (int i = 0; i < IQcNvItems.NV_SEC_CODE_SIZE; i++)
                ret += (char)mDigits[i];
            return ret;
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            for (int i = 0; i < IQcNvItems.NV_SEC_CODE_SIZE; i++)
                buf.put(this.mDigits[i]);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE * IQcNvItems.NV_SEC_CODE_SIZE);
        }

        public String toDebugString() {
            return String.format("sec_code:%s", getSecCode());
        }
    };

    // nv_lock_code_type
    public static class NvLockCodeType extends BaseQCNvItemType {
        byte mDigits[];

        NvLockCodeType() {
            mDigits = new byte[IQcNvItems.NV_LOCK_CODE_SIZE];
        }

        NvLockCodeType(byte[] bArray) throws IOException {
            mDigits = new byte[IQcNvItems.NV_LOCK_CODE_SIZE];
            ByteBuffer buf = createByteBuffer(bArray);
            for (int i = 0; i < IQcNvItems.NV_LOCK_CODE_SIZE; i++)
                this.mDigits[i] = buf.get();
        }

        public void setLockCode(String in) throws InvalidParameterException {
            if (in == null || in.length() != IQcNvItems.NV_LOCK_CODE_SIZE)
                throw new InvalidParameterException();

            for (int i = 0; i < IQcNvItems.NV_LOCK_CODE_SIZE; i++)
                this.mDigits[i] = (byte)in.charAt(i);
        }

        public String getLockCode() {
            String ret = "";
            for (int i = 0; i < IQcNvItems.NV_LOCK_CODE_SIZE; i++)
                ret += (char)mDigits[i];
            return ret;
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            for (int i = 0; i < IQcNvItems.NV_LOCK_CODE_SIZE; i++)
                buf.put(this.mDigits[i]);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE * IQcNvItems.NV_LOCK_CODE_SIZE);
        }

        public String toDebugString() {
            return String.format("lock_code:%s", getLockCode());
        }
    };

    // nv_call_cnt_type
    public static class NvCallCntType extends BaseQCNvItemType {
        byte nam;

        int mCount;

        NvCallCntType() {
            this.nam = 0;
            this.mCount = 0;
        }

        NvCallCntType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.nam = buf.get();
            skipPaddingBytes(buf, 3);
            this.mCount = buf.getInt();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(nam);
            addPaddingBytes(buf, 3);
            buf.putInt(mCount);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE + SIZE_OF_INT + 3 /* padding */);
        }

        public String toDebugString() {
            return String.format("nam:%d, Count:%d", nam, mCount);
        }
    };

    // nv_pref_voice_type
    public static class NvPrefVoiceSoType extends BaseQCNvItemType {
        byte mNam;

        boolean mEvrcCapabilityEnabled;

        short mHomePageVoiceSo;

        short mHomeOrigVoiceSo;

        short mRoamOrigVoiceSo;

        NvPrefVoiceSoType() throws InvalidParameterException {
            this.mNam = 0;
        }

        NvPrefVoiceSoType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            this.mNam = buf.get();
            this.mEvrcCapabilityEnabled = buf.get() == 1 ? Boolean.TRUE : Boolean.FALSE;
            this.mHomePageVoiceSo = buf.getShort();
            this.mHomeOrigVoiceSo = buf.getShort();
            this.mRoamOrigVoiceSo = buf.getShort();
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());
            buf.put(this.mNam);
            buf.put((byte)(this.mEvrcCapabilityEnabled == true ? 1 : 0));
            buf.putShort(this.mHomePageVoiceSo);
            buf.putShort(this.mHomeOrigVoiceSo);
            buf.putShort(this.mRoamOrigVoiceSo);
            return buf.array();
        }

        public int getSize() {
            return (SIZE_OF_BYTE * 2 + SIZE_OF_SHORT * 3);
        }

        public String toDebugString() {
            return String
                    .format(
                            "nam:%x, evrc_capability_enabled:%s, home_page_voice_so=%d, home_orig_voice_so=%d, roam_orig_voice_so=%d",
                            mNam, String.valueOf(mEvrcCapabilityEnabled), mHomePageVoiceSo,
                            mHomeOrigVoiceSo, mRoamOrigVoiceSo);
        }
    };

    // ecc_list
    public static class EccListType extends BaseQCNvItemType {

        byte mEccList[][];

        EccListType() {
            mEccList = new byte[IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER][IQcNvItems.NV_ECC_NUMBER_SIZE];
        }

        EccListType(byte[] bArray) throws IOException {
            ByteBuffer buf = createByteBuffer(bArray);
            mEccList = new byte[IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER][IQcNvItems.NV_ECC_NUMBER_SIZE];

            for (int i = 0; i < IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER; i++) {
                for (int j = 0; j < IQcNvItems.NV_ECC_NUMBER_SIZE; j++) {
                    this.mEccList[i][j] = buf.get();
                }
            }
        }

        public void setEccList(String[] in) throws InvalidParameterException {

            mEccList = new byte[IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER][IQcNvItems.NV_ECC_NUMBER_SIZE];

            if (in == null || in.length != IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER) {
                throw new InvalidParameterException();
            }

            for (int i = 0; i < IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER; i++) {
                if (in[i] == null || in[i].length() == 0) {
                    for (int j = 0; j < IQcNvItems.NV_ECC_NUMBER_SIZE; j++)
                        this.mEccList[i][j] = 0;
                    continue;
                }

                if (in[i].length() != IQcNvItems.NV_ECC_NUMBER_SIZE) {
                    throw new InvalidParameterException();
                }

                for (int j = 0; j < IQcNvItems.NV_ECC_NUMBER_SIZE; j++)
                    this.mEccList[i][j] = (byte)in[i].charAt(j);
            }
        }

        public String[] getEcclist() {
            String[] ret = new String[IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER];

            for (int i = 0; i < IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER; i++) {
                ret[i] = "";
                boolean isAllZero = true;
                for (int j = 0; j < IQcNvItems.NV_ECC_NUMBER_SIZE; j++) {
                    if (mEccList[i][j] != 0) {
                        isAllZero = false;
                    }
                    ret[i] += (char)mEccList[i][j];
                }
                if (isAllZero) {
                    ret[i] = "";
                }
            }
            return ret;
        }

        public byte[] toByteArray() {
            ByteBuffer buf = createByteBuffer(getSize());

            for (int i = 0; i < IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER; i++) {
                for (int j = 0; j < IQcNvItems.NV_ECC_NUMBER_SIZE; j++) {
                    buf.put(mEccList[i][j]);
                }
            }
            return buf.array();
        }

        public int getSize() {
            return (IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER * IQcNvItems.NV_ECC_NUMBER_SIZE * SIZE_OF_BYTE);
        }

        public String toDebugString() {
            String ret = "";
            for (int i = 0; i < IQcNvItems.NV_MAX_NUM_OF_ECC_NUMBER; i++) {
                ret += String.format("%d : ", i);
                for (int j = 0; j < IQcNvItems.NV_ECC_NUMBER_SIZE; j++) {
                    ret += (char)mEccList[i][j];
                }
                ret += ", ";
            }
            return String.format("ecc_list : %s", ret);
        }
    };

    // nv_auto_answer_type
    public static class NvAutoAnswerType {
        boolean enable; // TRUE if auto answer enabled

        byte rings; // Number of rings when to answer call

        public static int getSize() {
            return (2 * SIZE_OF_BYTE);
        }
    };
}
