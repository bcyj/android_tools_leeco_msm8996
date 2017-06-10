/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qcrilhook;

import java.nio.ByteBuffer;
import java.security.InvalidParameterException;
import java.util.ArrayList;

import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiItemType;
import com.qualcomm.qcrilhook.BaseQmiTypes.BaseQmiStructType;
import com.qualcomm.qcrilhook.PresenceOemHook.SubscriptionType;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiByte;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiInteger;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiNull;
import com.qualcomm.qcrilhook.QmiPrimitiveTypes.QmiString;

public class PresenceMsgBuilder {

    static class NoTlvPayloadRequest extends BaseQmiStructType {
        QmiNull noParam;

        public static final short IMS_ENABLER_REQ_TYPE = 0x01;

        public NoTlvPayloadRequest() {
            noParam = null;
        }

        @Override
            public BaseQmiItemType[] getItems() {
                return new BaseQmiItemType[] {
                    noParam
                };
            }

        @Override
            public short[] getTypes() {
                return new short[] {
                    IMS_ENABLER_REQ_TYPE
                };
            }
    }

    static class Subscribe {

        /* Note that Imsp_user_uri_struct extends BaseQmiItemType and not BaseQmiStructType.
         * BaseQmiItemType is more flexible in terms of the byte-buffer
         * creations. For example, BaseQmiStructType would auto prepend 3
         * bytes(1 byte for TAG, 2 bytes for Length) followed by the value.
         *
         * Whereas BaseQmiItemType provides you the functionality to create byte
         * buffer the way you want. Apparently, the right tool for nested
         * structure layout as required by few of Presence requests
         *
         */
        static class Imsp_user_uri_struct extends BaseQmiItemType {

            int mNum;
            int mCompleteLen = 0;
            ArrayList<String> mContactList;

            ArrayList<QmiByte> imsp_user_uri_len = new ArrayList();
            ArrayList<QmiString> imsp_user_uri = new ArrayList();

            public Imsp_user_uri_struct(ArrayList<String> contactList) {
                mNum = contactList.size();
                mContactList = contactList;

                for (String s : contactList) {
                    int len = s.length();
                    mCompleteLen += len;

                    imsp_user_uri_len.add(new QmiByte(len));
                    imsp_user_uri.add(new QmiString(s));
                }

            }

            @Override
                public byte[] toByteArray() {
                    int size = getSize();

                    ByteBuffer tempBuf = createByteBuffer(size);
                    for (int i = 0; i < mNum; i++) {
                        tempBuf.put(imsp_user_uri_len.get(i).toByteArray());
                        tempBuf.put(imsp_user_uri.get(i).toByteArray());
                    }

                    return tempBuf.array();
                }

            @Override
                public int getSize() {
                    return mCompleteLen + mNum; // string lengths+num of strings

                }

            @Override
                public byte[] toTlv(short type) throws InvalidParameterException {
                    ByteBuffer buf = createByteBuffer(getSize()); // only the Value
                    // field.

                    buf.put(toByteArray());
                    return buf.array();
                }

            @Override
                public String toString() {
                    String temp = "";
                    for (int i = 0; i < mNum; i++) {
                        temp += String.format("[Contact[%d]_%s]", i, imsp_user_uri
                                .get(i).toString());
                    }
                    return temp;
                }
        }

        static class Imsp_user_info_struct extends BaseQmiItemType {

            QmiByte subscribe_user_list_len;

            Imsp_user_uri_struct imsp_user_uri;

            public Imsp_user_info_struct(ArrayList<String> contactList) {
                int num = contactList.size();
                this.subscribe_user_list_len = new QmiByte(num);
                this.imsp_user_uri = new Imsp_user_uri_struct(contactList);
            }

            @Override
                public byte[] toByteArray() {
                    int size = getSize();

                    ByteBuffer tempBuf = createByteBuffer(size);
                    tempBuf.put(subscribe_user_list_len.toByteArray());
                    tempBuf.put(imsp_user_uri.toByteArray());
                    return tempBuf.array();
                }

            @Override
                public int getSize() {
                    return subscribe_user_list_len.getSize()
                        + imsp_user_uri.getSize();
                }

            @Override
                public byte[] toTlv(short type) throws InvalidParameterException {
                    ByteBuffer buf = createByteBuffer(TLV_TYPE_SIZE
                            + TLV_LENGTH_SIZE + getSize());

                    try {
                        buf.put(PrimitiveParser.parseByte(type));
                    } catch (NumberFormatException e) {
                        throw new InvalidParameterException(e.toString());
                    }
                    buf.putShort(PrimitiveParser.parseShort(getSize()));

                    buf.put(toByteArray());
                    return buf.array();
                }

            @Override
                public String toString() {
                    return String.format(
                            "[subscribe_user_list_len_%s], [imsp_user_uri=%s]",
                            subscribe_user_list_len.toString(),
                            imsp_user_uri.toString());
                }
        }

        static class SubscribeStructRequest extends BaseQmiStructType {

            QmiInteger subscriptionType;
            Imsp_user_info_struct mUserInfo;

            public static final short IMSP_SUBSCRIPTION_TYPE = 0x01;
            public static final short IMSP_USER_INFO = 0x02;

            public SubscribeStructRequest(
                    SubscriptionType subscriptionType,
                    ArrayList<String> contactList) {

                this.subscriptionType = new QmiInteger(
                        subscriptionType.ordinal());
                this.mUserInfo = new Imsp_user_info_struct(contactList);

            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] { subscriptionType, mUserInfo };
                }

            @Override
                public short[] getTypes() {
                    return new short[] { IMSP_SUBSCRIPTION_TYPE, IMSP_USER_INFO };
                }

        }

        static class SubscribeXMLRequest extends BaseQmiStructType {

            QmiString subscribeXml;

            public static final short SUBSCRIBE_XML_TYPE = 0x01;

            public SubscribeXMLRequest() {}

            public SubscribeXMLRequest(String xml) {
                this.subscribeXml = new QmiString(xml);
            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] {
                        subscribeXml
                    };
                }

            @Override
                public short[] getTypes() {
                    return new short[] {
                        SUBSCRIBE_XML_TYPE
                    };
                }
        }
    }

    static class UnSubscribe {
        static class UnSubscribeRequest extends BaseQmiStructType {

            QmiString peerURI;

            public static final short PEER_URI_TYPE = 0x01;

            public UnSubscribeRequest(String peerURI) {
                this.peerURI = new QmiString(peerURI);
            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] {
                        peerURI
                    };
                }

            @Override
                public short[] getTypes() {
                    return new short[] {
                        PEER_URI_TYPE
                    };
                }
        }
    }
    static class Publish {
        static class Imsp_presence_service_description_struct extends BaseQmiItemType {

            QmiString mDescription;
            QmiString mVer;
            QmiString mService_id;

            public Imsp_presence_service_description_struct(String description,
                    String ver,
                    String service_id) {
                this.mDescription = new QmiString(description);
                this.mVer = new QmiString(ver);
                this.mService_id = new QmiString(service_id);
            }

            @Override
                public byte[] toByteArray() {
                    int size = getSize();

                    ByteBuffer tempBuf = createByteBuffer(size);
                    tempBuf.put((byte)mDescription.getSize());
                    tempBuf.put(mDescription.toByteArray());

                    tempBuf.put((byte)mVer.getSize());
                    tempBuf.put(mVer.toByteArray());

                    tempBuf.put((byte)mService_id.getSize());
                    tempBuf.put(mService_id.toByteArray());
                    return tempBuf.array();
                }

            @Override
                public int getSize() {
                    return mDescription.getSize()+1 +mVer.getSize()+1 +mService_id.getSize()+1;
                }

            @Override
                public byte[] toTlv(short type) throws InvalidParameterException {
                    ByteBuffer buf = createByteBuffer(TLV_LENGTH_SIZE + getSize());

                    buf.putShort(PrimitiveParser.parseShort(getSize()));
                    buf.put(toByteArray());
                    return buf.array();
                }

            @Override
                public String toString() {
                    return String
                        .format("[mDescription_%s],[mVer_%s], [mService_id_%s]",
                                mDescription.toString(),
                                mVer.toString(),
                                mService_id.toString());
                }

        }

        static class Imsp_presence_service_capabilities_struct extends BaseQmiItemType {

            QmiByte mIs_audio_supported;
            QmiInteger mAudio_capability;
            QmiByte mIs_video_supported;
            QmiInteger mVideo_capability;

            public Imsp_presence_service_capabilities_struct(
                    int is_audio_supported,
                    int audio_capability,
                    int is_video_supported,
                    int video_capability) {
                this.mIs_audio_supported = new QmiByte(is_audio_supported);
                this.mAudio_capability = new QmiInteger(audio_capability);
                this.mIs_video_supported = new QmiByte(is_video_supported);
                this.mVideo_capability= new QmiInteger(video_capability);

            }

            @Override
                public byte[] toByteArray() {
                    int size = getSize();

                    ByteBuffer tempBuf = createByteBuffer(size);
                    tempBuf.put(mIs_audio_supported.toByteArray());
                    tempBuf.put(mAudio_capability.toByteArray());
                    tempBuf.put(mIs_video_supported.toByteArray());
                    tempBuf.put(mVideo_capability.toByteArray());
                    return tempBuf.array();

                }

            @Override
                public int getSize() {
                    return mIs_audio_supported.getSize()
                        +mAudio_capability.getSize()
                        +mIs_video_supported.getSize()
                        +mVideo_capability.getSize();
                }

            @Override
                public byte[] toTlv(short type) throws InvalidParameterException {
                    ByteBuffer buf = createByteBuffer(TLV_LENGTH_SIZE + getSize());

                    buf.putShort(PrimitiveParser.parseShort(getSize()));
                    buf.put(toByteArray());
                    return buf.array();
                }

            @Override
                public String toString() {
                    return String.format("[mIs_audio_supported_%s], " +
                            "[mAudio_capability_%s], " +
                            "[mIs_video_supported_%s], " +
                            "[mVideo_capability_%s]", mIs_audio_supported.toString(),
                            mAudio_capability.toString(),
                            mIs_video_supported.toString(),
                            mVideo_capability.toString());
                }
        }

        static class Imsp_presence_info_struct extends BaseQmiItemType {

            QmiString mContact_uri;
            Imsp_presence_service_description_struct mService_descriptions;
            Imsp_presence_service_capabilities_struct mService_capabilities;

            public Imsp_presence_info_struct(String contact_uri,
                    String description,
                    String ver,
                    String service_id,
                    int is_audio_supported,
                    int audio_capability,
                    int is_video_supported,
                    int video_capability) {
                this.mContact_uri = new QmiString(contact_uri);
                this.mService_descriptions = new Imsp_presence_service_description_struct(
                        description,
                        ver,
                        service_id);
                this.mService_capabilities = new Imsp_presence_service_capabilities_struct(
                        is_audio_supported,
                        audio_capability,
                        is_video_supported,
                        video_capability);

            }

            @Override
                public byte[] toByteArray() {
                    int size = getSize();

                    ByteBuffer tempBuf = createByteBuffer(size);

                    tempBuf.put((byte)mContact_uri.getSize());
                    tempBuf.put(mContact_uri.toByteArray());

                    tempBuf.put(mService_descriptions.toByteArray());
                    tempBuf.put(mService_capabilities.toByteArray());

                    return tempBuf.array();
                }

            @Override
                public int getSize() {
                    //strings are preceded by 1 byte size field
                    return mContact_uri.getSize()+1+
                        mService_descriptions.getSize()+
                        mService_capabilities.getSize();
                }

            @Override
                public String toString() {
                    return String.format("[mContact_uri_%s],"+
                            " [mService_descriptions=%s],"+
                            " [mService_capabilities=%s] ",
                            mContact_uri.toString(),
                            mService_descriptions.toString(),
                            mService_capabilities.toString());
                }
            // Do not override toTLV(), we need default TLV here,
            //nested structure would not have TAG, so we customize there.
        }


        static class PublishStructRequest extends BaseQmiStructType {

            QmiInteger mPublish_status;
            Imsp_presence_info_struct mPresence_info;


            public static final short PUBLISH_STATUS_TYPE = 0x01;
            public static final short PUBLISH_PRESENCE_INFO_TYPE = 0x10;


            public PublishStructRequest() {
            }

            public PublishStructRequest(int publish_status,
                    String contact_uri,
                    String description,
                    String ver,
                    String service_id,
                    int is_audio_supported,
                    int audio_capability,
                    int is_video_supported,
                    int video_capability) {

                this.mPublish_status = new QmiInteger(publish_status);
                this.mPresence_info = new Imsp_presence_info_struct(contact_uri,
                        description,
                        ver,
                        service_id,
                        is_audio_supported,
                        audio_capability,
                        is_video_supported,
                        video_capability);
            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] {
                        mPublish_status,
                        mPresence_info};
                }

            @Override
                public short[] getTypes() {
                    return new short[] {
                        PUBLISH_STATUS_TYPE,
                            PUBLISH_PRESENCE_INFO_TYPE};
                }

        }

        static class PublishXMLRequest extends BaseQmiStructType {

            QmiString publishXml;

            public static final short PUBLISH_XML_TYPE = 0x01;

            public PublishXMLRequest() {}

            public PublishXMLRequest(String xml) {
                this.publishXml = new QmiString(xml);
            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] {
                        publishXml
                    };
                }

            @Override
                public short[] getTypes() {
                    return new short[] {
                        PUBLISH_XML_TYPE
                    };
                }
        }

    }

    static class UnPublish {
        static class UnPublishRequest extends BaseQmiStructType {

            QmiNull noParam;

            public static final short UNPUBLISH_REQ_TYPE = 0x01;

            public UnPublishRequest() {
                noParam = new QmiNull();
            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] {
                        noParam
                    };
                }

            @Override
                public short[] getTypes() {
                    return new short[] {
                        UNPUBLISH_REQ_TYPE
                    };
                }

        }
    }

    static class NotifyFmt {
        static class SetFmt extends BaseQmiStructType {

            QmiByte mUpdate_with_struct_info;

            public static final short UPDATE_WITH_STRUCT_INFO_TYPE = 0x01;

            public SetFmt() {}

            public SetFmt(short flag) {
                this.mUpdate_with_struct_info = new QmiByte(flag);
            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] {
                        mUpdate_with_struct_info
                    };
                }

            @Override
                public short[] getTypes() {
                    return new short[] {
                        UPDATE_WITH_STRUCT_INFO_TYPE,
                    };
                }
        }
    }

    static class EventReport {
        static class EventReportMaskStruct extends BaseQmiItemType {

            QmiByte mMask;

            public EventReportMaskStruct(int mask) {
                this.mMask = new QmiByte(mask);

            }

            @Override
                public byte[] toByteArray() {
                    int size = getSize();

                    ByteBuffer tempBuf = createByteBuffer(size);
                    tempBuf.put(mMask.toByteArray());
                    return tempBuf.array();
                }

            @Override
                public int getSize() {
                    /* 8 bit mask is represented as 8 byte value, as per IDL.
                     */
                    return 8;
                }

            @Override
                public byte[] toTlv(short type) throws InvalidParameterException {

                    ByteBuffer buf = createByteBuffer(TLV_TYPE_SIZE +
                            TLV_LENGTH_SIZE +
                            getSize());
                    try {
                        buf.put(PrimitiveParser.parseByte(type));
                    } catch (NumberFormatException e) {
                        throw new InvalidParameterException(e.toString());
                    }
                    buf.putShort(PrimitiveParser.parseShort(getSize()));
                    buf.put(toByteArray()); //[0] Actual mask
                    //[1] to [7] bytes are empty as per IDL.

                    return buf.array();
                }

            @Override
                public String toString() {
                    return String.format("[mMask_%s]", mMask);
                }

        }

        static class SetEventReport extends BaseQmiStructType {


            EventReportMaskStruct mask;

            public static final short EVENT_REPORT_MASK_TYPE = 0x01;

            public SetEventReport() {}

            public SetEventReport(int mask) {
                this.mask = new EventReportMaskStruct(mask);
            }

            @Override
                public BaseQmiItemType[] getItems() {
                    return new BaseQmiItemType[] {
                        mask
                    };
                }

            @Override
                public short[] getTypes() {
                    return new short[] {
                        EVENT_REPORT_MASK_TYPE,
                    };
                }

        }
    }
}
