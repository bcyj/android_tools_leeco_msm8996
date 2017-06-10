/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountConstant;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatModel;
import com.suntek.mway.rcs.client.aidl.provider.model.MessageSessionModel;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMediaMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTextMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTopicMessage;
import com.suntek.mway.rcs.client.aidl.provider.model.PublicTopicMessage.PublicTopicContent;
import com.suntek.mway.rcs.client.api.im.impl.MessageApi;
import com.suntek.mway.rcs.client.api.im.impl.PaMessageApi;
import com.suntek.mway.rcs.client.api.util.FileDurationException;
import com.suntek.mway.rcs.client.api.util.FileSuffixException;
import com.suntek.mway.rcs.client.api.util.FileTransferException;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import com.suntek.mway.rcs.publicaccount.data.RcsGeoLocation;
import com.suntek.mway.rcs.publicaccount.data.RcsGeoLocationParser;
import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.ContactsContract.Contacts;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class PASendMessageUtil {

    private static final String RCS_MMS_VCARD_PATH = "sdcard/rcs/" + "mms.vcf";
    
    public static boolean sendTextMessage(Context context, String message, String uuid,
            long threadId) {
        if (message != null && !message.toString().trim().equals("")) {
            try {
                RcsApiManager.getPaMessageApi().sendTextMessage(threadId, -1, uuid, message);
                return true;
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
                return false;
            }
        } else {
            Toast.makeText(context, R.string.message_please_input_content, Toast.LENGTH_SHORT)
                    .show();
            return false;
        }
    }

    public static void sendImageMessage(Context context, String imagePath, String uuid,
            long threadId, int imageQuality) {
        try {
            RcsApiManager.getPaMessageApi().sendImageFile(threadId, -1, uuid, imagePath,
                    imageQuality);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        } catch (FileSuffixException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_suffix_vaild_tip),
                    Toast.LENGTH_LONG).show();
        } catch (FileTransferException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_size_over), Toast.LENGTH_LONG)
                    .show();
        }
    }

    public static void sendAudioMessage(Context context, String audioPath, String uuid,
            long threadId, int recordTime, boolean isRecord) {
        try {
            RcsApiManager.getPaMessageApi().sendAudioFile(threadId, -1, uuid, audioPath,
                    recordTime, isRecord);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        } catch (FileSuffixException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_suffix_vaild_tip),
                    Toast.LENGTH_LONG).show();
        } catch (FileTransferException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_size_over), Toast.LENGTH_LONG)
                    .show();
        } catch (FileDurationException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_duration_over),
                    Toast.LENGTH_LONG).show();
        }
    }

    public static void sendVideoMessage(Context context, String videoPath, String uuid,
            long threadId, long length, boolean isRecord) {
        try {
            RcsApiManager.getPaMessageApi().sendVideoFile(threadId, -1, uuid, videoPath,
                    (int)length, isRecord);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        } catch (FileSuffixException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_suffix_vaild_tip),
                    Toast.LENGTH_LONG).show();
        } catch (FileTransferException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_size_over), Toast.LENGTH_LONG)
                    .show();
        } catch (FileDurationException e) {
            e.printStackTrace();
            Toast.makeText(context, context.getString(R.string.file_duration_over),
                    Toast.LENGTH_LONG).show();
        }
    }

    public static void sendGroupVcard(Context context, Intent data, String uuid, long threadId) {
        if (data == null)
            return;
        ArrayList<String> list = data.getStringArrayListExtra("recipients");
        StringBuffer buffer = new StringBuffer();
        for (String string : list) {
            Uri contactUri = ContentUris.withAppendedId(Contacts.CONTENT_URI,
                    Long.parseLong(string));
            String lookup = Uri.encode(Contacts
                    .getLookupUri(context.getContentResolver(), contactUri).getPathSegments()
                    .get(2));
            buffer.append(lookup + ":");
        }
        String buffer2 = buffer.substring(0, buffer.lastIndexOf(":"));
        Uri uri2 = Uri.withAppendedPath(PAMessageUtil.getPAVcardUri(), Uri.encode(buffer2));
        boolean isCreateVcardSucceed = PAMessageUtil.setPAVcard(context, uri2);
        if (isCreateVcardSucceed) {
            try {
                RcsApiManager.getPaMessageApi().sendVCardByPath(threadId, 0, uuid,
                        PAMessageUtil.RCS_MMS_VCARD_PATH);
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
                Toast.makeText(context, context.getString(R.string.message_send_fail),
                        Toast.LENGTH_LONG).show();
            }
        }
    }

    public static void sendContactVcard(Context context, Intent data, String uuid, long threadId) {
        if (data == null)
            return;
        String extraVCard = data.getStringExtra("vcard");
        if (extraVCard != null) {
            Uri vcard = Uri.parse(extraVCard);
            boolean isCreateVcardSucceed = PAMessageUtil.setPAVcard(context, vcard);
            if (isCreateVcardSucceed) {
                try {
                    RcsApiManager.getPaMessageApi().sendVCardByPath(threadId, 0, uuid,
                            PAMessageUtil.RCS_MMS_VCARD_PATH);
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                    Toast.makeText(context, context.getString(R.string.message_send_fail),
                            Toast.LENGTH_LONG).show();
                }
            }
        }
    }

    public static void sendMyVcard(Context context, String uuid, long threadId) {
        String rawContactId = PAMessageUtil.getMyRcsRawContactId(context);
        if (TextUtils.isEmpty(rawContactId)) {
            Toast.makeText(context, R.string.please_set_my_profile, 0).show();
            return;
        }
        Uri contactUri = ContentUris.withAppendedId(Contacts.CONTENT_URI,
                Long.parseLong(rawContactId));
        Log.i("RCS_UI", "CONTACT URI=" + contactUri);
        String lookup = Uri.encode(Contacts.getLookupUri(context.getContentResolver(), contactUri)
                .getPathSegments().get(2));
        Uri uri = Uri.withAppendedPath(Contacts.CONTENT_VCARD_URI, lookup);
        boolean isCreateVcardSucceed = PAMessageUtil.setPAVcard(context, uri);
        if (isCreateVcardSucceed) {
            try {
                RcsApiManager.getPaMessageApi().sendVCardByPath(threadId, 0, uuid,
                        PAMessageUtil.RCS_MMS_VCARD_PATH);
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
                Toast.makeText(context, context.getString(R.string.message_send_fail),
                        Toast.LENGTH_LONG).show();
            }
        }
    }

    public static void sendMapMessage(Context context, String uuid, long threadId, double latitude,
            double longitude, String info) {
        try {
            RcsApiManager.getPaMessageApi().sendLocation(threadId, -1, uuid, latitude, longitude,
                    info);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    public static boolean forwardToRcsGroupMessage(long threadId, List<String> numberList,
            ChatMessage chatMessage, GroupChatModel groupChatModel)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException,
            FileDurationException {
        if (chatMessage == null) {
            return false;
        }
        if (groupChatModel == null) {
            return false;
        }
        threadId = groupChatModel.getThreadId();
        String convId = groupChatModel.getConversationId();
        int groupId = groupChatModel.getId();
        int msgType = chatMessage.getMsgType();
        switch (msgType) {
            case SuntekMessageData.MSG_TYPE_TEXT:
                sendRcsTextGroup(threadId, getPaText(chatMessage), convId, groupId);
                break;
            case SuntekMessageData.MSG_TYPE_AUDIO:
                return sendRcsAudioGroup(threadId, chatMessage, convId, groupId);
            case SuntekMessageData.MSG_TYPE_VIDEO:
                return sendRcsVideoGroup(threadId, chatMessage, convId, groupId);
            case SuntekMessageData.MSG_TYPE_IMAGE:
                return sendRcsImageGroup(threadId, chatMessage, convId, groupId);
            case SuntekMessageData.MSG_TYPE_CONTACT:
                sendRcsVCardGroup(threadId, convId, groupId);
                break;
            case SuntekMessageData.MSG_TYPE_MAP:
                sendRcsMapGroup(threadId, chatMessage, convId, groupId);
                break;
            default:
                break;
        }
        return true;
    }

    public static boolean forwardPaMessage(List<String> numberList,
            ChatMessage chatMessage) {
        try {
            if (chatMessage == null) {
                return false;
            }
            if (numberList == null || (numberList != null && numberList.size() == 0)) {
                return false;
            }
            int sucCount = 0;
            int size = numberList.size();
            String paUuid;
            long threadId;
            for (int i = 0; i < size; i++) {
                paUuid = numberList.get(i);
                threadId = getServiceThreadId(paUuid);
                if (forwardPaMessageByType(threadId, paUuid, chatMessage)) {
                    sucCount ++;
                }
            }
            return sucCount == size;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private static boolean forwardPaMessageByType(long threadId, String paUuid, ChatMessage chatMessage) {
        try {
            if (chatMessage == null) {
                return false;
            }
            int msgType = chatMessage.getMsgType();
            switch (msgType) {
                case SuntekMessageData.MSG_TYPE_TEXT:
                    sendPaText(threadId, getPaText(chatMessage), paUuid);
                    break;
                case SuntekMessageData.MSG_TYPE_AUDIO:
                    return sendPaAudio(threadId, chatMessage, paUuid);
                case SuntekMessageData.MSG_TYPE_VIDEO:
                    return sendPaVideo(threadId, chatMessage, paUuid);
                case SuntekMessageData.MSG_TYPE_IMAGE:
                    return sendPaImage(threadId, chatMessage, paUuid);
                case SuntekMessageData.MSG_TYPE_CONTACT:
                    sendPaVCard(threadId, paUuid);
                    break;
                case SuntekMessageData.MSG_TYPE_MAP:
                    return sendPaMap(threadId, chatMessage, paUuid);
                default:
                    break;
            }
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
            return false;
        } catch (FileSuffixException e) {
            e.printStackTrace();
            return false;
        } catch (FileTransferException e) {
            e.printStackTrace();
            return false;
        } catch (FileDurationException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }
    

    public static boolean forwardPaTopicMessage(List<String> numberList,
            PublicTopicContent topicContent) {
        try {
            if (topicContent == null) {
                return false;
            }
            if (numberList == null || (numberList != null && numberList.size() == 0)) {
                return false;
            }
            int size = numberList.size();
            String paUuid;
            long threadId;
            for (int i = 0; i < size; i++) {
                paUuid = numberList.get(i);
                threadId = getServiceThreadId(paUuid);
                if (threadId == -1) {
                    return false;
                }
                sendPaText(threadId, topicContent.getTitle() + topicContent.getBodyLink(), paUuid);
            }
            return true;
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean forwardTopicMessage(long threadId, List<String> numberList,
            PublicTopicContent topicContent) {
        try {
            if (topicContent == null) {
                return false;
            }
            int chatType;
            if (numberList != null) {
                if (numberList.size() == 1) {
                    chatType = SuntekMessageData.CHAT_TYPE_ONE2ONE;
                } else {
                    chatType = SuntekMessageData.CHAT_TYPE_ONE2GROUP;
                }
            } else {
                return false;
            }
            switch (chatType) {
                case SuntekMessageData.CHAT_TYPE_ONE2ONE: 
                    sendRcsText(threadId, topicContent.getTitle() + topicContent.getBodyLink(), numberList.get(0));
                    break;
                case SuntekMessageData.CHAT_TYPE_ONE2GROUP:
                    sendRcsTextOne2Many(threadId, topicContent.getTitle() + topicContent.getBodyLink(), numberList);
                    break;
            }
            return true;
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean forwardRcsMessage(long threadId, List<String> numberList,
            ChatMessage chatMessage) {
        try {
            if (chatMessage == null) {
                return false;
            }
            int chatType;
            MessageSessionModel model = null;
            if (numberList != null) {
                if (numberList.size() == 1) {
                    chatType = SuntekMessageData.CHAT_TYPE_ONE2ONE;
                } else {
                    chatType = SuntekMessageData.CHAT_TYPE_ONE2GROUP;
                }
            } else {
                return false;
            }
            int msgType = chatMessage.getMsgType();
            switch (chatType) {
                case SuntekMessageData.CHAT_TYPE_ONE2ONE:
                    String number;
                    if (model == null) {
                        if (numberList != null && numberList.size() > 0) {
                            number = numberList.get(0);
                        } else {
                            return false;
                        }
                    } else {
                        number = model.getContact();
                    }
                    switch (msgType) {
                        case SuntekMessageData.MSG_TYPE_TEXT:
                            sendRcsText(threadId, getPaText(chatMessage), number);
                            break;
                        case SuntekMessageData.MSG_TYPE_AUDIO:
                            return sendRcsAudio(threadId, chatMessage, number);
                        case SuntekMessageData.MSG_TYPE_VIDEO:
                            return sendRcsVideo(threadId, chatMessage, number);
                        case SuntekMessageData.MSG_TYPE_IMAGE:
                            return sendRcsImage(threadId, chatMessage, number);
                        case SuntekMessageData.MSG_TYPE_CONTACT:
                            // RCSContact rcsContact =
                            // ChatMessageUtil.readVcardFile(filePath);
                            // messageApi.sendVCard(threadId, -1, number,
                            // rcsContact);
                            sendRcsVCard(threadId, number);
                            break;
                        case SuntekMessageData.MSG_TYPE_MAP:
                            return sendRcsMap(threadId, chatMessage, number);
                        default:
                            break;
                    }
                    break;
                case SuntekMessageData.CHAT_TYPE_ONE2GROUP:
                    List<String> array;
                    if (model != null) {
                        String numbers = model.getReceiversOfOne2Many();
                        if (TextUtils.isEmpty(numbers)) {
                            return false;
                        }
                        String[] numberArray = numbers.split(",");
                        if (numberArray == null) {
                            return false;
                        }
                        array = Arrays.asList(numberArray);
                    } else {
                        array = numberList;
                    }
                    switch (msgType) {
                        case SuntekMessageData.MSG_TYPE_TEXT:
                            sendRcsTextOne2Many(threadId, getPaText(chatMessage), array);
                            break;
                        case SuntekMessageData.MSG_TYPE_AUDIO:
                            return sendRcsAudioOne2Many(threadId, chatMessage, array);
                        case SuntekMessageData.MSG_TYPE_VIDEO:
                            return sendRcsVideoOne2Many(threadId, chatMessage, array);
                        case SuntekMessageData.MSG_TYPE_IMAGE:
                            return sendRcsImageOne2Many(threadId, chatMessage, array);
                        case SuntekMessageData.MSG_TYPE_CONTACT:
                            // Profile profile =
                            // ChatMessageUtil.readVcardFile(filePath);
                            // messageApi.sendOne2ManyVCard(threadId, -1, array,
                            // ProfileManager.profileToRCSContact(profile));
                            sendRcsVCardOne2Many(threadId, array);
                            break;
                        case SuntekMessageData.MSG_TYPE_MAP:
                            return sendRcsMapOne2Many(threadId, chatMessage, array);
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
            return false;
        } catch (FileSuffixException e) {
            e.printStackTrace();
            return false;
        } catch (FileTransferException e) {
            e.printStackTrace();
            return false;
        } catch (FileDurationException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    private static void sendPaText(long threadId, String text, String paUuid)
            throws ServiceDisconnectedException {
        PaMessageApi paMessageApi = RcsApiManager.getPaMessageApi();
        paMessageApi.sendTextMessage(threadId, -1, paUuid, text);
    }

    private static void sendRcsText(long threadId, String text, String number)
            throws ServiceDisconnectedException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        messageApi.sendTextMessage(threadId, number, text,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0);
    }

    private static void sendRcsTextGroup(long threadId, String text, String convId, int groupId)
            throws ServiceDisconnectedException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        messageApi.sendGroupMessage(threadId, convId, -1, text,
                String.valueOf(groupId));
    }

    private static void sendRcsTextOne2Many(long threadId, String text, List<String> number)
            throws ServiceDisconnectedException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        messageApi.sendOne2ManyTextMessage(threadId, number, text,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0);
    }

    private static void sendPaVCard(long threadId, String paUuid)
            throws ServiceDisconnectedException {
        RcsApiManager.getPaMessageApi().sendVCardByPath(threadId, -1, paUuid, RCS_MMS_VCARD_PATH);
    }

    private static void sendRcsVCard(long threadId, String number)
            throws ServiceDisconnectedException {
        RcsApiManager.getMessageApi().sendVCard(threadId, -1, number, RCS_MMS_VCARD_PATH);
    }

    private static void sendRcsVCardGroup(long threadId, String convId, int groupId)
            throws ServiceDisconnectedException {
        RcsApiManager.getMessageApi().sendGroupVCard(threadId, convId, -1, RCS_MMS_VCARD_PATH,
                String.valueOf(groupId));
    }

    private static void sendRcsVCardOne2Many(long threadId, List<String> numbers)
            throws ServiceDisconnectedException {
        RcsApiManager.getMessageApi().sendOne2ManyVCard(threadId, -1, numbers, RCS_MMS_VCARD_PATH);
    }

    private static boolean sendPaMap(long threadId, ChatMessage chatMessage, String paUuid)
            throws ServiceDisconnectedException {
        PaMessageApi paMessageApi = RcsApiManager.getPaMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        RcsGeoLocation geo = readPaMapXml(filePath);
        paMessageApi.sendLocation(threadId, -1, paUuid, geo.getLat(), geo.getLng(), geo.getLabel());
        return true;
    }

    private static boolean sendRcsMap(long threadId, ChatMessage chatMessage, String number)
            throws ServiceDisconnectedException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        RcsGeoLocation geo = readPaMapXml(filePath);
        messageApi.sendLocation(threadId, -1, number, geo.getLat(), geo.getLng(), geo.getLabel());
        return true;
    }

    private static boolean sendRcsMapGroup(long threadId, ChatMessage chatMessage, String convId, int groupId)
            throws ServiceDisconnectedException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        RcsGeoLocation geo = readPaMapXml(filePath);
        messageApi.sendGroupLocation(threadId, convId, -1,
                geo.getLat(), geo.getLng(), geo.getLabel(),
                String.valueOf(groupId));
        return true;
    }

    private static boolean sendRcsMapOne2Many(long threadId, ChatMessage chatMessage, List<String> array)
            throws ServiceDisconnectedException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        RcsGeoLocation geo = readPaMapXml(filePath);
        messageApi.sendOne2ManyLocation(threadId, -1, array, geo.getLat(), geo.getLng(),
                geo.getLabel());
        return true;
    }

    private static boolean sendPaImage(long threadId, ChatMessage chatMessage, String paUuid)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException {
        PaMessageApi paMessageApi = RcsApiManager.getPaMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        paMessageApi.sendImageFile(threadId, -1, paUuid, filePath, 100);
        return true;
    }

    private static boolean sendRcsImage(long threadId, ChatMessage chatMessage, String number)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        messageApi.sendImageFile(threadId, -1, number, filePath,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0, 100);
        return true;
    }

    private static boolean sendRcsImageGroup(long threadId, ChatMessage chatMessage, String convId, int groupId)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        messageApi.sendGroupImageFile(threadId, convId, -1,
                filePath, String.valueOf(groupId), 100);
        return true;
    }

    private static boolean sendRcsImageOne2Many(long threadId, ChatMessage chatMessage, List<String> numbers)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        messageApi.sendOne2ManyImageFile(threadId, -1, numbers, filePath,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0, 100);
        return true;
    }

    private static boolean sendPaVideo(long threadId, ChatMessage chatMessage, String paUuid)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException,
            FileDurationException {
        PaMessageApi paMessageApi = RcsApiManager.getPaMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        paMessageApi.sendVideoFile(threadId, -1, paUuid, filePath, duration, false);
        return true;
    }

    private static boolean sendRcsVideo(long threadId, ChatMessage chatMessage, String number)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException,
            FileDurationException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        messageApi.sendVideoFile(threadId, -1, number, filePath, duration,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0, false);
        return true;
    }

    private static boolean sendRcsVideoGroup(long threadId, ChatMessage chatMessage, String convId,
            int groupId) throws ServiceDisconnectedException, FileSuffixException,
            FileTransferException, FileDurationException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        messageApi.sendGroupVideoFile(threadId, convId, -1, filePath, duration,
                String.valueOf(groupId), false);
        return true;
    }

    private static boolean sendRcsVideoOne2Many(long threadId, ChatMessage chatMessage, List<String> numbers)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException,
            FileDurationException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        messageApi.sendOne2ManyVideoFile(threadId, -1, numbers, filePath, duration,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0, false);
        return true;
    }

    private static boolean sendPaAudio(long threadId, ChatMessage chatMessage, String paUuid)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException,
            FileDurationException {
        PaMessageApi paMessageApi = RcsApiManager.getPaMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        paMessageApi.sendAudioFile(threadId, -1, paUuid, filePath, duration, false);
        return true;
    }

    private static boolean sendRcsAudio(long threadId, ChatMessage chatMessage, String number)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException,
            FileDurationException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        messageApi.sendAudioFile(threadId, -1, number, filePath, duration,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0, false);
        return true;
    }

    private static boolean sendRcsAudioGroup(long threadId, ChatMessage chatMessage, String convId,
            int groupId) throws ServiceDisconnectedException, FileSuffixException,
            FileTransferException, FileDurationException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        messageApi.sendGroupAudioFile(threadId, convId, -1, filePath, duration, String.valueOf(groupId), false);
        return true;
    }

    private static boolean sendRcsAudioOne2Many(long threadId, ChatMessage chatMessage, List<String> numbers)
            throws ServiceDisconnectedException, FileSuffixException, FileTransferException,
            FileDurationException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String filePath = getPaFilePath(chatMessage);
        if (filePath == null) {
            return false;
        }
        int duration = getAudioLength(chatMessage);
        messageApi.sendOne2ManyAudioFile(threadId, -1, numbers, filePath,
                duration,
                SuntekMessageData.MSG_BURN_AFTER_READ_NOT, 0, false);
        return true;
    }

    public static String getFilePath(ChatMessage cMsg)
            throws ServiceDisconnectedException {
        String path = RcsApiManager.getMessageApi()
                .getFilepath(cMsg);
        if (path != null && new File(path).exists()) {
            return path;
        } else {
            if (path != null && path.lastIndexOf("/") != -1) {
                path = path.substring(0, path.lastIndexOf("/") + 1);
                return path + cMsg.getFilename();
            } else {
                return null;
            }
        }
    }

    public static String getPaFilePath(ChatMessage chatMessage) {
        String filePath = chatMessage.getFilepath();
        if (filePath != null) {
            File file = new File(filePath);
            if (isReceiveMsg(chatMessage)) {
                if (file.exists()) {
                    return filePath;
                }
            } else {
                if (file != null && file.exists()) {
                    String dir = file.getParent();
                    String newFilePath = dir + File.separator + chatMessage.getFilename();
                    if (!newFilePath.equals(filePath)) {
                        boolean suc = renameFile(filePath, newFilePath);
                        return suc ? newFilePath : null;
                    } else {
                        return filePath;
                    }
                } else {//may be forwarded
                    return getForwordFileName(filePath, chatMessage.getFilename());
                }
            }
        }
        return null;
    }

    public static int getAudioLength(ChatMessage cMsg) {
        int len = 0;
        try {
            if (cMsg == null) {
                return 0;
            }
            if (isReceiveMsg(cMsg)) {
                len = getDuration(getPaFilePath(cMsg));
            } else {
                String lens = cMsg.getData();
                String length = lens.substring(7, lens.lastIndexOf("-"));
                len = Integer.parseInt(length);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
            len = 0;
        }
        return len;
    }

    private static int getDuration(String path) {
        if (TextUtils.isEmpty(path)) {
            return 0;
        }
        MediaPlayer player = new MediaPlayer();
        int duration = 0;
        try {
            player.setDataSource(path);
            player.prepare();
            duration = player.getDuration() / 1000;
        } catch (Exception e) {
            e.printStackTrace();
            return 0;
        }
        return duration;
    }

    public static String getForwordFileName(ChatMessage cMsg)
            throws ServiceDisconnectedException {
        MessageApi messageApi = RcsApiManager.getMessageApi();
        String path = messageApi
                .getFilepath(cMsg);
        if (path != null && path.lastIndexOf("/") != -1) {
            path = path.substring(0, path.lastIndexOf("/") + 1);
            return path + cMsg.getFilename();
        } else {
            return null;
        }
    }

    public static String getForwordFileName(String sendFilePath, String fileName) {
        if (sendFilePath != null && sendFilePath.lastIndexOf("/") != -1) {
            sendFilePath = sendFilePath.substring(0, sendFilePath.lastIndexOf("/") + 1);
            return sendFilePath + fileName;
        } else {
            return null;
        }
    }

    public static boolean renameFile(String oldFilePath, String newFilePath) {
        if (TextUtils.isEmpty(oldFilePath) || TextUtils.isEmpty(newFilePath)) {
            return false;
        }
        File oldFile = new File(oldFilePath);
        File newFile = new File(newFilePath);
        return oldFile.renameTo(newFile);
    }

    public static RcsGeoLocation readPaMapXml(String filepath) {
        RcsGeoLocation geo = null;
        try {
            RcsGeoLocationParser handler = new RcsGeoLocationParser(new FileInputStream(filepath));
            geo = handler.getGeoLocation();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return geo;
    }

    public static boolean saveMessage(Context context, ChatMessage chatMessage) {
        InputStream input = null;
        FileOutputStream fout = null;
        try {
            if (chatMessage == null) {
                return false;
            }
            int msgType = chatMessage.getMsgType();
            if (msgType != SuntekMessageData.MSG_TYPE_AUDIO
                    && msgType != SuntekMessageData.MSG_TYPE_VIDEO
                    && msgType != SuntekMessageData.MSG_TYPE_IMAGE) {
                return true; // we only save pictures, videos, and sounds.
            }
            String filePath = getPaFilePath(chatMessage);
            if (filePath == null) {
                Toast.makeText(context, R.string.file_not_exist, Toast.LENGTH_SHORT).show();
                return false;
            }
            if (isLoading(filePath, chatMessage.getFilesize())) {
                return false;
            }
            String saveDir = Environment.getExternalStorageDirectory() + "/"
                    + Environment.DIRECTORY_DOWNLOADS + "/";
            File dirFile = new File(saveDir);
            if (!dirFile.exists()) {
                dirFile.mkdirs();
            }
            String fileName = null;
//            File file = null;
//            int index = 0;
//            String extension = null;
//            if (!isReceiveMsg(chatMessage)) {
//                fileName = chatMessage.getFilename();
//                index = fileName.lastIndexOf('.');
//                extension = fileName.substring(index + 1, fileName.length());
//                fileName = fileName.substring(0, index);
//                // Remove leading periods. The gallery ignores files starting with a
//                // period.
//                fileName = fileName.replaceAll("^.", "");
//            } else {
//                index = filePath.lastIndexOf(".");
//                fileName = filePath.substring(0, index);
//                extension = filePath.substring(index + 1);
//            }
            input = new FileInputStream(filePath);
            File file = getUniqueDes(saveDir + getFileName(filePath),
                    getFileExtensionName(filePath));
            fout = new FileOutputStream(file);

            byte[] buffer = new byte[1024];
            int size = 0;
            while ((size = input.read(buffer)) != -1) {
                fout.write(buffer, 0, size);
            }
            // Notify other applications listening to scanner events
            // that a media file has been added to the sd card
            context.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri
                    .fromFile(file)));
        } catch (Exception e) {
            return false;
        } finally {
            if (null != input) {
                try {
                    input.close();
                } catch (IOException e) {
                    // Ignore
                    Log.e("RCS_UI", "IOException caught while closing stream", e);
                    return false;
                }
            }
            if (null != fout) {
                try {
                    fout.close();
                } catch (IOException e) {
                    // Ignore
                    Log.e("RCS_UI", "IOException caught while closing stream", e);
                    return false;
                }
            }
        }
        return true;
    }

    private static String getFileExtensionName(String filePath) {
        int index = filePath.lastIndexOf(".");
        return filePath.substring(index + 1);
    }

    private static String getFileName(String filePath) {
        int start = filePath.lastIndexOf("/");
        int end = filePath.lastIndexOf(".");
        return filePath.substring(start, end + 1);
    }

    private static boolean isLoading(String filePath, long fileSize) {
        if (TextUtils.isEmpty(filePath)) {
            return false;
        }
        File file = new File(filePath);
        if (file.exists() && file.length() < fileSize) {
            return true;
        } else {
            return false;
        }
    }

    private static File getUniqueDes(String base, String extension) {
        File file = new File(base + "." + extension);

        for (int i = 2; file.exists(); i++) {
            file = new File(base + "_" + i + "." + extension);
        }
        return file;
    }

    public static long getServiceThreadId(String paUuid) {
        long threadId = -1;
        try {
            String threadStr = RcsApiManager.getMessageApi().getThreadIdByNumber(
                    Arrays.asList(new String[] {
                        paUuid
                    }));
            if (!TextUtils.isEmpty(threadStr)) {
                threadId = Long.parseLong(threadStr);
            }
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
            threadId = -1;
        }
        return threadId;
    }

    public static boolean isReceiveMsg(ChatMessage cMsg) {
        if (cMsg.getSendReceive() == SuntekMessageData.MSG_SEND) {
            return false;
        } else {
            return true;
        }
    }

    public static String getPaText(ChatMessage chatMessage) {
        String text = null;
        if (isReceiveMsg(chatMessage)) {
            text = ((PublicTextMessage)chatMessage.getPublicMessage()).getContent();
        } else {
            text = chatMessage.getData();
        }
        return text;
    }

    public static boolean checkForwardFilePathExist(Context context, ChatMessage chatMessage) {
        if (chatMessage.getMsgType() != SuntekMessageData.MSG_TYPE_PUBLIC_TOPIC
                && chatMessage.getMsgType() != SuntekMessageData.MSG_TYPE_TEXT) {
            if (!CommonUtil.isFileExists(PASendMessageUtil.getPaFilePath(chatMessage))) {
                Toast.makeText(context, R.string.forward_file_not_exist, Toast.LENGTH_SHORT).show();
                return false;
            }
        }
        return true;
    }

    public static void rcsForwardMessage(Activity context, ChatMessage message) {
//      if (topicForwardCheck()) {
          if (forwardableHint(context, message)) {
              if (isRcsOnline()) {
                  showForwardSelectDialog(context);
              } else {
                  toast(context, R.string.not_online_message_too_big);
              }
          }
//      }
  }

	public static void showForwardSelectDialog(final Activity context) {
		Resources res = context.getResources();
		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		builder.setCancelable(true);
		builder.setTitle(R.string.select_contact_conversation);
		builder.setItems(
				new String[] { res.getString(R.string.forward_contact),
						res.getString(R.string.forward_conversation),
						res.getString(R.string.forward_contact_group),
						res.getString(R.string.forward_public_account) },
				new DialogInterface.OnClickListener() {

					@Override
					public void onClick(DialogInterface dialog, int which) {
						switch (which) {
						case 0:
							if (Build.VERSION.SDK_INT == Build.VERSION_CODES.KITKAT) {
								Intent intent = new Intent(
										"com.android.contacts.action.MULTI_PICK",
										Contacts.CONTENT_URI);
								context.startActivityForResult(intent,
										PAConversationActivity.REQUEST_CODE_RCS_PICK);
							} else {
								Intent intent = new Intent(
										"com.android.mms.ui.SelectRecipientsList");
								intent.putExtra("mode", PAConversationActivity.MODE_DEFAULT);
								// mIsPickingContact = true;
								context.startActivityForResult(intent,
										PAConversationActivity.REQUEST_CODE_RCS_PICK);
							}
							break;
						case 1:
							Intent intent = new Intent(
									"com.android.mms.ui.ConversationList");
							intent.putExtra("select_conversation", true);
							context.startActivityForResult(intent, PAConversationActivity.REQUEST_SELECT_CONV);
							break;
						case 2:
							if (Build.VERSION.SDK_INT == Build.VERSION_CODES.KITKAT) {
								intent = new Intent(
										"com.android.mms.MultiPickContactGroups");
								// mIsPickingContact = true;
								context.startActivityForResult(intent,
										PAConversationActivity.REQUEST_SELECT_GROUP);
							} else {
								intent = new Intent(
										"com.android.mms.ui.SelectRecipientsList");
								intent.putExtra("mode", PAConversationActivity.MODE_DEFAULT);
								// mIsPickingContact = true;
								context.startActivityForResult(intent,
										PAConversationActivity.REQUEST_SELECT_GROUP);
							}
							break;
						case 3:
							intent = new Intent(
									"com.suntek.mway.rcs.nativeui.ui.PUBLIC_ACCOUNT_ACTIVITY");
							intent.putExtra("forward", true);
							context.startActivityForResult(intent,
									PAConversationActivity.REQUEST_SELECT_PUBLIC_ACCOUNT);
							break;
						default:
							break;
						}
					}
				});
		builder.show();
	}

	public static boolean topicForwardCheck(ChatMessage message) {
		if (message.getMsgType() == SuntekMessageData.MSG_TYPE_PUBLIC_TOPIC) {
			return false;
		}
		return true;
	}

	public static boolean forwardableHint(Context context, ChatMessage chatMessage) {
		boolean forwardable = true;
		PublicMessage pm = chatMessage.getPublicMessage();
		if (pm instanceof PublicTextMessage) {
			forwardable = (((PublicTextMessage) pm).getForwardable() == PublicAccountConstant.MESSAGE_FORWARD_ABLE);
		} else if (pm instanceof PublicMediaMessage) {
			forwardable = (((PublicMediaMessage) pm).getForwardable() == PublicAccountConstant.MESSAGE_FORWARD_ABLE);
		} else if (pm instanceof PublicTopicMessage) {
			forwardable = (((PublicTopicMessage) pm).getForwardable() == PublicAccountConstant.MESSAGE_FORWARD_ABLE);
		}
		if (!forwardable) {
			toast(context, R.string.forward_message_not_support);
		}
		return forwardable;
	}

	public static boolean isRcsOnline() {
		boolean isRcsOnline;
		try {
			isRcsOnline = RcsApiManager.getAccountApi().isOnline();
		} catch (ServiceDisconnectedException e) {
			e.printStackTrace();
			isRcsOnline = false;
		}
		return isRcsOnline;
	}

	public static void forwardConversation(Activity context, Intent data, ChatMessage message) {
        try {
//            ChatMessage message = mConversationAdapter.getSelectedList().get(0);
            if (!PASendMessageUtil.checkForwardFilePathExist(context, message))
                return;
            long threadId = -1;
            if (Build.VERSION.SDK_INT == Build.VERSION_CODES.KITKAT) {
                threadId = -1;
            } else {
                threadId = data.getLongExtra("selectThreadId", -1);
            }
            String[] numbers = data.getStringArrayExtra("numbers");
            GroupChatModel groupChatModel = null;
            if (data.hasExtra("groupChatModel")) {
                groupChatModel = data.getParcelableExtra("groupChatModel");
            }
            boolean suc = false;
            if (groupChatModel != null) {
                suc = PASendMessageUtil.forwardToRcsGroupMessage(threadId, Arrays.asList(numbers),
                        message, groupChatModel);
            } else {
                suc = PASendMessageUtil
                        .forwardRcsMessage(threadId, Arrays.asList(numbers), message);
            }
            forwardHint(context, suc);
        } catch (Exception exception) {
            exception.printStackTrace();
            forwardHint(context, false);
        }
    }
	
	public static void forwardPaMessage(Activity context, ChatMessage message, PublicTopicContent topic, List<String> numberList) {
        try {
//            ChatMessage message = mConversationAdapter.getSelectedList().get(0);
            if (!PASendMessageUtil.checkForwardFilePathExist(context, message))
                return;
            boolean suc = false;
//            Map<String, List<PublicTopicContent>> checks = mConversationAdapter.getMultiChecks();
            if (message.getMsgType() == SuntekMessageData.MSG_TYPE_PUBLIC_TOPIC) {
                suc = forwardPaTopic(numberList, topic);
            } else {
                suc = forwardPaMessage(numberList, message);
            }
            forwardHint(context, suc);
        } catch (Exception e) {
            forwardHint(context, false);
        }
    }

	public static boolean forwardPaTopic(List<String> numberList,
			PublicTopicContent topic) {
		return forwardPaTopicMessage(numberList, topic);
	}

    public static void forwardRcsMessage(Activity context, ChatMessage message, PublicTopicContent topicContent, List<String> numberList) {
        try {
//            ChatMessage message = mConversationAdapter.getSelectedList().get(0);
            if (!PASendMessageUtil.checkForwardFilePathExist(context, message))
                return;
            boolean suc = false;
            if (message.getMsgType() == SuntekMessageData.MSG_TYPE_PUBLIC_TOPIC) {
                suc = forwardTopic(numberList, message, topicContent);
            } else {
                suc = PASendMessageUtil.forwardRcsMessage(-1, numberList, message);
            }
            forwardHint(context, suc);
        } catch (Exception e) {
            forwardHint(context, false);
        }
    }

    public static boolean forwardTopic(List<String> numberList, ChatMessage message, PublicTopicContent topicContent) {
        return forwardTopicMessage(-1, numberList, topicContent);
    }

    public static void forwardHint(Activity context, boolean success) {
        if (success) {
            toast(context, R.string.forward_message_success);
        } else {
            toast(context, R.string.forward_message_fail);
        }
    }

    public static void toast(Context context, int resId) {
        Toast.makeText(context, resId, Toast.LENGTH_LONG).show();
    }

    public static void toast(Context context, String string) {
        Toast.makeText(context, string, Toast.LENGTH_LONG).show();
    }
}
