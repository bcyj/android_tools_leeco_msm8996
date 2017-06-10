/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

import java.nio.ByteBuffer;
import java.util.ArrayList;

import android.util.Log;
import android.widget.TextView;

import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;

public class PresenceMsgParser {
    private static String LOG_TAG = "PresenceMsgParser";


    static enum MediaCapabilities {
        FULL_DUPLEX, HALF_RECEIVE_ONLY, HALF_SEND_ONLY
    }

    public static class ContactInfo {
        public ListHeaderInfo listHeaderInfo;
        public String mResourceUri;
        public String mResourceId;
        public String mResourceState;
        public String mResourceReason;
        public String mResourceCid;
        public String mDescription;
        public String mVersion;
        public String mServiceId;

        public String mContactUri;
        public boolean mIsVolteContact;
        public int mPublishStatus;
        public boolean mIsAudioSupported;
        public boolean mIsVideoSupported;
        public String mAudioCapabilities;
        public String mVideoCapabilities;
        public String mTimeStamp;

        @Override
        public String toString() {
            return "ContactInfo [listHeaderInfo=" + listHeaderInfo
                + ", mResourceUri=" + mResourceUri + ", mResourceId="
                + mResourceId + ", mResourceState=" + mResourceState
                + ", mResourceReason=" + mResourceReason
                + ", mResourceCid=" + mResourceCid + ", mDescription="
                + mDescription + ", mVersion=" + mVersion + ", mServiceId="
                + mServiceId + ", mContactUri=" + mContactUri
                + ", mIsVolteContact=" + mIsVolteContact
                + ", mPublishStatus=" + mPublishStatus
                + ", mIsAudioSupported=" + mIsAudioSupported
                + ", mIsVideoSupported=" + mIsVideoSupported
                + ", mAudioCapabilities=" + mAudioCapabilities
                + ", mVideoCapabilities=" + mVideoCapabilities
                + ", mTimeStamp=" + mTimeStamp + "]";
        }
    }

    public static class ListHeaderInfo {
        public String mListContactUri;
        public String mListName;
        public String mListVersion;
        public String mListFullState;


        @Override
        public String toString() {
            return "ListHeaderInfo [mListContactUri=" + mListContactUri
                + ", mListName=" + mListName + ", mListVersion="
                + mListVersion + ", mListFullState=" + mListFullState + "]";
        }

    }

    static class PresenceRichNotifyParser {

        private ArrayList<ContactInfo> parsedContactList;

        /* temp c for current contact which is being parsed.*/
        private ContactInfo c;

        /* current list header info for all the aggregated contacts in the list */
        private ListHeaderInfo listHeaderInfo;

        private ByteBuffer respByteBuf;
        private int totalBytes;

        public PresenceRichNotifyParser(ByteBuffer respByteBuf, int n) {
            this.respByteBuf = respByteBuf;
            this.totalBytes = n;
        }

        private String parseString(int n) {

            final int STRING_LENGTH = n;

            if(respByteBuf.remaining() < STRING_LENGTH) {
                new Exception().printStackTrace();
                return "";
            }

            byte[] data = new byte[STRING_LENGTH];

            for (int i = 0; i < STRING_LENGTH; i++) {
                data[i] = respByteBuf.get();
            }

            return new String(data);
        }

        private int parseInteger() {

            final int INTEGER_LENGTH = 4;

            if(respByteBuf.remaining() < INTEGER_LENGTH) {
                new Exception().printStackTrace();
                return 0;
            }

            byte[] data = new byte[INTEGER_LENGTH];

            for (int i = 0; i < INTEGER_LENGTH; i++) {
                data[i] = respByteBuf.get();
            }

            return PrimitiveParser.toUnsigned(data[0]);
        }

        private int parseShort() {
            final int SHORT_LENGTH = 2;

            if(respByteBuf.remaining() < SHORT_LENGTH) {
                new Exception().printStackTrace();
                return 0;
            }

            byte[] data = new byte[SHORT_LENGTH];

            for (int i = 0; i < SHORT_LENGTH; i++) {
                data[i] = respByteBuf.get();
            }

            return PrimitiveParser.toUnsigned(data[0]);
        }

        private int parseByte() {

            if(respByteBuf.remaining() < 1) {
                new Exception().printStackTrace();
                return 0;
            }

            byte[] data = new byte[1];
            data[0] = respByteBuf.get();

            return PrimitiveParser.toUnsigned(data[0]);
        }

        private void parseListContactUri() {
            int len = parseByte();
            String s = parseString(len);

            listHeaderInfo.mListContactUri = s;
            Log.d(LOG_TAG, "Parsing ListContactUri = "+s);
        }

        private void parseListName() {
            int len = parseByte();
            String s = parseString(len);

            listHeaderInfo.mListName = s;
            Log.d(LOG_TAG, "Parsing ListName = "+s);
        }

        private void parseListVersion() {
            int listVersion = parseInteger();
            listHeaderInfo.mListVersion = ""+listVersion;
            Log.d(LOG_TAG, "Parsing ListVersion = "+listVersion);
        }

        private void parseListFullState() {
            int b = parseByte();

            listHeaderInfo.mListFullState = ""+b;
            Log.d(LOG_TAG, "Parsing ListFullState = "+b);
        }

        private void parseListInfo() {
            parseListContactUri();
            parseListName();
            parseListVersion();
            parseListFullState();
        }

        private void parseResourceUri() {
            int len = parseByte();
            String s = parseString(len);

            c.mResourceUri = s;
            Log.d(LOG_TAG, "Parsing ResourceUri = "+s);
        }

        private void parseIsVolteContact() {
            int val = parseByte();
            c.mIsVolteContact = ((val == 1) ? true : false);

            Log.d(LOG_TAG, "Parsing IsVolteContact = "+c.mIsVolteContact);
        }

        private void parsePublishStatus() {
            int val = parseInteger();

            c.mPublishStatus = val; //closed =0, Open =1
            Log.d(LOG_TAG, "Parsing PublishStatus = "+val);
        }

        private void parseResourceId() {
            int len = parseByte();
            String s = parseString(len);

            c.mResourceId = s;
            Log.d(LOG_TAG, "Parsing ResourceId = "+s);
        }

        private void parseResourceState() {
            int len = parseByte();
            String s = parseString(len);

            c.mResourceState = s;
            Log.d(LOG_TAG, "Parsing ResourceState = "+s);
        }

        private void parseResourceReason() {
            int len = parseByte();
            String s = parseString(len);

            c.mResourceReason = s;
            Log.d(LOG_TAG, "Parsing ResourceReason = "+s);
        }

        private void parseResourceCid() {
            int len = parseShort();
            String s = parseString(len);

            c.mResourceCid = s;
            Log.d(LOG_TAG, "Parsing ResourceCid = "+s);
        }

        private void parseResouceInstance() {
            parseResourceId();
            parseResourceState();
            parseResourceReason();
            parseResourceCid();
        }

        private void parseContactUri() {
            int len = parseByte();
            String s = parseString(len);

            c.mContactUri = s;
            Log.d(LOG_TAG,"Parsing Contact Uri = "+s);
        }

        private void parseDescription() {
            int len = parseByte();
            String s = parseString(len);

            c.mDescription = s;
            Log.d(LOG_TAG, "Parsing Description = "+s);
        }

        private void parseVersion() {
            int len = parseByte();
            String s = parseString(len);

            c.mVersion = s;
            Log.d(LOG_TAG, "Parsing Version = "+s);
        }

        private void parseServiceid() {
            int len = parseByte();
            String s = parseString(len);

            c.mServiceId = s;
            Log.d(LOG_TAG, "Parsing ServiceId = "+s);
        }

        private void parseServiceDescriptions() {
            parseDescription();
            parseVersion();
            parseServiceid();
        }

        private void parseIsAudioSupported() {
            int val = parseByte();

            c.mIsAudioSupported = (((val == 1) ? true : false));

            Log.d(LOG_TAG, "Parsing isAudioSupported="+c.mIsAudioSupported );
        }

        private void parseAudioCapability() {
            int val = parseInteger();
            c.mAudioCapabilities = MediaCapabilities.values()[val].toString();

            Log.d(LOG_TAG, "Parsing AudioCapabilities="+c.mAudioCapabilities );
        }

        private void parseVideoCapability() {
            int val = parseInteger();

            c.mVideoCapabilities = MediaCapabilities.values()[val].toString();

            Log.d(LOG_TAG, "Parsing VideoCapabilities="+c.mVideoCapabilities );
        }

        private void parseIsVideoSupported() {
            int val = parseByte();

            c.mIsVideoSupported = (((val == 1) ? true : false));

            Log.d(LOG_TAG, "Parsing isVideoSupported="+c.mIsVideoSupported );
        }

        private void parseServiceCapabilities() {
            parseIsAudioSupported(); // IMP
            parseAudioCapability(); // IMP
            parseIsVideoSupported(); // IMP
            parseVideoCapability(); // IMP
        }

        private void parsePresenceInfo(	) {
            parseContactUri(); // IMP
            parseServiceDescriptions();
            parseServiceCapabilities();
        }

        private void parseTimeStamp() {
            int len = parseByte();
            String s = parseString(len);

            c.mTimeStamp = s;

            Log.d(LOG_TAG, "Parsing timeStamp="+c.mTimeStamp);
        }

        private void parsePresenceUserInfoWithTs() {
            parsePresenceInfo();
            parseTimeStamp();
        }

        public int parseUserListInfoLen() {
            return parseByte();

        }

        private void parseUserListInfo() {
            int numOfContacts = parseUserListInfoLen();
            Log.d(LOG_TAG, "Parsing numOfContacts = "+numOfContacts);

            for(int i=0;i<numOfContacts; i++) {
                this.c = new ContactInfo();
                this.c.listHeaderInfo = this.listHeaderInfo;

                parseResourceUri();
                parseIsVolteContact(); // IMP
                parsePublishStatus(); // IMP
                parseResouceInstance();
                parsePresenceUserInfoWithTs();

                parsedContactList.add(c);
            }
        }

        private  ArrayList<ContactInfo> parseRichInfo() {

            this.parsedContactList = new ArrayList();
            this.listHeaderInfo  = new ListHeaderInfo();

            parseListInfo();
            parseUserListInfo();

            return parsedContactList;
        }

    }


    static ArrayList<ContactInfo> parseNotifyUpdate(ByteBuffer respByteBuf,
            int responseSize,
            int successStatus) {
        final short NOTIFY_DETAIL_TYPE = 0x01;
        final short IMSP_SUBSCRIBE_CALLID_TYPE = 0x10;
        int callId =0;

        Log.d(LOG_TAG, "notifyUpdate(), Thread="+Thread.currentThread().getName());

        while (responseSize > 0) {
            short type = PrimitiveParser.toUnsigned(respByteBuf.get());
            int length = PrimitiveParser.toUnsigned(respByteBuf.getShort());

            switch (type) {
                case IMSP_SUBSCRIBE_CALLID_TYPE:
                    byte[] data = new byte[length];
                    for (int i = 0; i < length;i++) {
                        data[i] = respByteBuf.get();
                    }
                    callId = PrimitiveParser.toUnsigned(data[0]);
                    Log.v(LOG_TAG, "callId = " + callId);
                    break;
                case NOTIFY_DETAIL_TYPE:
                    Log.v(LOG_TAG, "NOTIFY_DETAIL_TYPE");

                    PresenceRichNotifyParser parser = new PresenceRichNotifyParser(
                            respByteBuf, length);
                    ArrayList<ContactInfo> parsedContactList = parser.parseRichInfo();
                    Log.d(LOG_TAG, "parsed contact info "+parsedContactList);
                    return parsedContactList;

                default:
                    // TODO
                    break;
            }
            responseSize -= BaseQmiItemType.TLV_TYPE_SIZE +
                BaseQmiItemType.TLV_LENGTH_SIZE
                + length;
        }
        return null;
    }

    static int parseEnablerState(ByteBuffer respByteBuf) {
        byte type = (byte) PrimitiveParser.toUnsigned(respByteBuf.get());
        short len = (short) PrimitiveParser.toUnsigned(respByteBuf.getShort());
        int val = (int) PrimitiveParser.toUnsigned(respByteBuf.getInt());

        return val;
    }

    static String parseNotifyUpdateXML(ByteBuffer respByteBuf) {
        byte tag = (byte) PrimitiveParser.toUnsigned(respByteBuf.get());
        short len = (short) PrimitiveParser.toUnsigned(respByteBuf
                .getShort());
        byte[] data = new byte[len];

        for (int i = 0; i < len; i++) {
            data[i] = respByteBuf.get();
        }
        return new String(data);
    }

    static int parseGetNotifyReq(ByteBuffer respByteBuf) {

        byte optionalTag = (byte) PrimitiveParser.toUnsigned(respByteBuf.get());
        short optionalLen = (short) PrimitiveParser.toUnsigned(respByteBuf
                .getShort());
        byte optionalVal = (byte) PrimitiveParser.toUnsigned(respByteBuf.get());

        return optionalVal;
    }

    static int parseGetEventReport(ByteBuffer respByteBuf) {
        byte optionalTag = (byte) PrimitiveParser.toUnsigned(respByteBuf
                .get());
        short optionalLen = (short) PrimitiveParser.toUnsigned(respByteBuf
                .getShort());
        byte optionalVal = (byte) PrimitiveParser.toUnsigned(respByteBuf
                .get());

        return optionalVal;
    }

    static int parsePublishTrigger(ByteBuffer respByteBuf) {
        byte tag = (byte) PrimitiveParser.toUnsigned(respByteBuf.get());
        short len = (short) PrimitiveParser.toUnsigned(respByteBuf
                .getShort());
        int val = (int) PrimitiveParser.toUnsigned(respByteBuf.getInt());

        return val;
    }

    static int parseEnablerStateInd(ByteBuffer respByteBuf) {
        byte tag = (byte) PrimitiveParser.toUnsigned(respByteBuf.get());
        short len = (short) PrimitiveParser.toUnsigned(respByteBuf
                .getShort());
        int val = (int) PrimitiveParser.toUnsigned(respByteBuf.getInt());

        return val;
    }
}
