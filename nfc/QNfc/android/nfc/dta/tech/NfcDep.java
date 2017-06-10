/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.nfc.dta.tech;

import android.nfc.ErrorCodes;
import android.nfc.dta.TagDta;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;

import java.io.IOException;

/**
 * Provides access to NFC-DEP properties and I/O operations on a {@link Tag}.
 *
 * <p>Acquire an {@link NfcDep} object using {@link #get}.
 * <p>The primary NFC-DEP I/O operation is {@link #transceive}. Applications must
 * implement their own protocol stack on top of {@link #transceive}.
 * <p>Tags that enumerate the {@link NfcDep} technology in {@link Tag#getTechList}
 * will also enumerate
 * {@link NfcA} or {@link NfcF} (since NfcDep builds on top of either of these).
 *
 * <p class="note"><strong>Note:</strong> Methods that perform I/O operations
 * require the {@link android.Manifest.permission#NFC} permission.
 */
public final class NfcDep extends BasicTagTechnology {
    private static final String TAG = "NFC_DEP";

    /** @hide */
    public static final String EXTRA_HI_LAYER_RESP = "hiresp";
    /** @hide */
    public static final String EXTRA_HIST_BYTES = "histbytes";

    private byte[] mHiLayerResponse = null;
    private byte[] mHistBytes = null;

    /**
     * Get an instance of {@link NfcDep} for the given tag (end point).
     * <p>Does not cause any RF activity and does not block.
     * <p>Returns null if {@link NfcDep} was not enumerated in {@link Tag#getTechList}.
     * This indicates the end point does not support NFC-DEP.
     *
     * @param tag an NFC-DEP compatible tag
     * @return NFC-DEP object
     */
    public static NfcDep get(TagDta tag) {

        if (!tag.hasTech(TagTechnology.NFC_DEP_INITIATOR) &&
            !tag.hasTech(TagTechnology.NFC_DEP_TARGET)) {
                return null;
        }

        // NFC-DEP uses either NFC-A or NFC-F technology:
        int tech = TagTechnology.NFC_A;
        if (!tag.hasTech(tech)) {
            tech = TagTechnology.NFC_F;
        }

        try {
            return new NfcDep(tag, tech);
        } catch (RemoteException e) {
            return null;
        }
    }

    /** @hide */
    public NfcDep(TagDta tag, int tech)
            throws RemoteException {
        super(tag, tech);
        Bundle extras = tag.getTechExtras(tech);
        if (extras != null) {
            mHiLayerResponse = extras.getByteArray(EXTRA_HI_LAYER_RESP);
            mHistBytes = extras.getByteArray(EXTRA_HIST_BYTES);
        }
        mIsConnected = true;
    }

    /**
     * Checks whether the NFC-DEP endpoint is a target or an initiator.
     *
     * @return true if the NFC-DEP endpoint is a target, false if it is an initiator.
     */
    public Boolean isTarget() {
        return mTag.hasTech(TagTechnology.NFC_DEP_TARGET);
    }

    /**
     * Set the timeout of {@link #transceive} in milliseconds.
     * <p>The timeout only applies to NFC-DEP {@link #transceive}, and is
     * reset to a default value when {@link #close} is called.
     * <p>Setting a longer timeout may be useful when performing
     * transactions that require a long processing time on the tag
     * such as key generation.
     *
     * <p class="note">Requires the {@link android.Manifest.permission#NFC} permission.
     *
     * @param timeout timeout value in milliseconds
     */
    public void setTimeout(int timeout) {
        try {
            int err = mTag.getTagService().setTimeout(mSelectedTechnology, timeout);
            if (err != ErrorCodes.SUCCESS) {
                throw new IllegalArgumentException("The supplied timeout is not valid");
            }
        } catch (RemoteException e) {
            Log.e(TAG, "NFC service dead", e);
        }
    }

    /**
     * Get the current timeout for {@link #transceive} in milliseconds.
     *
     * <p class="note">Requires the {@link android.Manifest.permission#NFC} permission.
     *
     * @return timeout value in milliseconds
     */
    public int getTimeout() {
        try {
            return mTag.getTagService().getTimeout(mSelectedTechnology);
        } catch (RemoteException e) {
            Log.e(TAG, "NFC service dead", e);
            return 0;
        }
    }

    /**
     * Return the NFC-DEP historical bytes for {@link NfcA} tags.
     * <p>Does not cause any RF activity and does not block.
     * <p>The historical bytes can be used to help identify a tag. They are present
     * only on {@link NfcDep} tags that are based on {@link NfcA} RF technology.
     * If this tag is not {@link NfcA} then null is returned.
     * <p>In NFC 14443-4 terminology, the historical bytes are a subset of the RATS
     * response.
     *
     * @return NFC-DEP historical bytes, or null if this is not a {@link NfcA} tag
     */
    public byte[] getHistoricalBytes() {
        return mHistBytes;
    }

    /**
     * Return the higher layer response bytes for {@link NfcB} tags.
     * <p>Does not cause any RF activity and does not block.
     * <p>The higher layer response bytes can be used to help identify a tag.
     * They are present only on {@link NfcDep} tags that are based on {@link NfcB}
     * RF technology. If this tag is not {@link NfcB} then null is returned.
     * <p>In NFC 14443-4 terminology, the higher layer bytes are a subset of the
     * ATTRIB response.
     *
     * @return NFC-DEP historical bytes, or null if this is not a {@link NfcB} tag
     */
    public byte[] getHiLayerResponse() {
        return mHiLayerResponse;
    }

    /**
     * Send raw NFC-DEP data to the tag and receive the response.
     *
     * <p>Applications must only send the INF payload, and not the start of frame and
     * end of frame indicators. Applications do not need to fragment the payload, it
     * will be automatically fragmented and defragmented by {@link #transceive} if
     * it exceeds FSD/FSC limits.
     *
     * <p>Use {@link #getMaxTransceiveLength} to retrieve the maximum number of bytes
     * that can be sent with {@link #transceive}.
     *
     * <p>This is an I/O operation and will block until complete. It must
     * not be called from the main application thread. A blocked call will be canceled with
     * {@link IOException} if {@link #close} is called from another thread.
     *
     * <p class="note">Requires the {@link android.Manifest.permission#NFC} permission.
     *
     * @param data command bytes to send, must not be null
     * @return response bytes received, will not be null
     * @throws TagLostException if the tag leaves the field
     * @throws IOException if there is an I/O failure, or this operation is canceled
     */
    public byte[] transceive(byte[] data) throws IOException {
        return transceive(data, true);
    }

    /**
     * Return the maximum number of bytes that can be sent with {@link #transceive}.
     * @return the maximum number of bytes that can be sent with {@link #transceive}.
     */
    public int getMaxTransceiveLength() {
        return getMaxTransceiveLengthInternal();
    }

    /**
     * <p>Standard APDUs have a 1-byte length field, allowing a maximum of
     * 255 payload bytes, which results in a maximum APDU length of 261 bytes.
     *
     * <p>Extended length APDUs have a 3-byte length field, allowing 65535
     * payload bytes.
     *
     * <p>Some NFC adapters, like the one used in the Nexus S and the Galaxy Nexus
     * do not support extended length APDUs. They are expected to be well-supported
     * in the future though. Use this method to check for extended length APDU
     * support.
     *
     * @return whether the NFC adapter on this device supports extended length APDUs.
     */
    public boolean isExtendedLengthApduSupported() {
        try {
            return mTag.getTagService().getExtendedLengthApdusSupported();
        } catch (RemoteException e) {
            Log.e(TAG, "NFC service dead", e);
            return false;
        }
    }
}
