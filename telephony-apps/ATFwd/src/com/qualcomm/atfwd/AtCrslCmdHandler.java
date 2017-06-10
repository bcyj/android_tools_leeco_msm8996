/*******************************************************************************
    Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.media.AudioManager;
import android.media.IAudioService;
import android.os.HandlerThread;
import android.util.Log;

public class AtCrslCmdHandler extends AtCmdBaseHandler implements AtCmdHandler {

    private static final String TAG = "AtCrslCmdHandler";

    public AtCrslCmdHandler(Context c) throws AtCmdHandlerInstantiationException
    {
        super(c);
    }

    private static IAudioService getAudioService() {
        IAudioService audioService = IAudioService.Stub.asInterface(
                ServiceManager.checkService(Context.AUDIO_SERVICE));
        if (audioService == null) {
            Log.e(TAG, "Unable to find IAudioService interface.");
        }
        return audioService;
    }

    static int getRingerVolume( String input )  {
        Integer ringVolume = -1;
        try  {
            ringVolume = Integer.parseInt( input );
            /*
             * TBD : Re-scaling of ringer volume level (android specific) based on
             * manufacturer's specifications.
             */
        }
        catch( NumberFormatException ex)  {
            Log.e(TAG, "Not an Integer: " + ex);
        }
        return ringVolume;
    }

    private int getMaxVolume() {
        int nMaxVol = -1;
        IAudioService audioService = getAudioService();
        if (audioService == null) {
            return nMaxVol;
        }
        try  {
            nMaxVol = audioService.getStreamMaxVolume(AudioManager.STREAM_RING);
        } catch( RemoteException ex)  {
            Log.e(TAG, "Unable to obtain Ringer level : " + ex);
        }
        return nMaxVol;
    }

    private String getFormattedRingerVolumeRange() {
        String ret = "";
        // Range {0, <Max Vol>}
        Integer nMaxVol = getMaxVolume();
        if (nMaxVol >= 0) {
            ret += "(0-" + nMaxVol.toString() + ")";
        }
        return ret;
    }

    @Override
    public String getCommandName() {
        return "+CRSL";
    }

    @Override
    public AtCmdResponse handleCommand(AtCmd cmd) {
        String tokens[] = cmd.getTokens();
        String result = null;
        boolean isAtCmdRespOK = false;
        boolean isSetCmd = false;

        IAudioService audioService = getAudioService();
        if (audioService == null) {
            return new AtCmdResponse(AtCmdResponse.RESULT_ERROR,
                            cmd.getAtCmdErrStr(AtCmd.AT_ERR_OP_NOT_ALLOW));
        }

        Log.d(TAG, "OpCode" + cmd.getOpcode());

        switch (cmd.getOpcode()) {
            case AtCmd.ATCMD_OPCODE_NA_EQ_AR:
                try {
                    // AT+CRSL=<LEVEL>
                    // Must have at least one token.
                    if (tokens == null || tokens.length == 0 ) {
                        result = cmd.getAtCmdErrStr(AtCmd.AT_ERR_INCORRECT_PARAMS);
                        break;
                    }
                    Integer ringVolume = getRingerVolume(tokens[0]);
                    int nMaxVol = getMaxVolume();
                    // Range check
                    if ((ringVolume < 0) || (nMaxVol < 0) || (ringVolume > nMaxVol)) {
                        result = cmd.getAtCmdErrStr(AtCmd.AT_ERR_OP_NOT_ALLOW);
                        break;
                    }
                    audioService.setStreamVolume(AudioManager.STREAM_RING,
                        ringVolume, AudioManager.FLAG_SHOW_UI
                        | AudioManager.FLAG_PLAY_SOUND,
                        getContext().getPackageName());
                    isSetCmd = true;
                    isAtCmdRespOK = true;
                }
                catch( RemoteException ex)  {
                    Log.e(TAG, "Unable to perfom AT+CRSL "+ tokens + "Exception : " + ex);
                    result = cmd.getAtCmdErrStr(AtCmd.AT_ERR_OP_NOT_ALLOW);
                }
                break;

            case AtCmd.ATCMD_OPCODE_NA_QU:
                try {
                    // AT+CRSL?
                    Integer ringVolume = audioService.getStreamVolume(AudioManager.STREAM_RING);
                    result = ringVolume.toString(ringVolume);
                    isAtCmdRespOK = true;
                }
                catch( RemoteException ex)  {
                    Log.e(TAG, "Unable to perfom AT+CRSL "+ tokens + "Exception : " + ex);
                    result = cmd.getAtCmdErrStr(AtCmd.AT_ERR_OP_NOT_ALLOW);
                }
                break;

            case  AtCmd.ATCMD_OPCODE_NA_EQ_QU:
                // AT+CRSL=?
                result = getFormattedRingerVolumeRange();
                if(result.length() > 0) {
                    isAtCmdRespOK = true;
                } else {
                    result = cmd.getAtCmdErrStr(AtCmd.AT_ERR_OP_NOT_ALLOW);
                }
                break;
        }

        if (!isSetCmd && isAtCmdRespOK) {
            result = getCommandName() + ": " + result;
        }

        return isAtCmdRespOK ? new AtCmdResponse(AtCmdResponse.RESULT_OK, result) :
            new AtCmdResponse(AtCmdResponse.RESULT_ERROR, result);
    }
}

