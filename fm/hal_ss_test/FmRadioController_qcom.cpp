/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 **/

#include "FmRadioController_qcom.h"
#include "QcomFmIoctlsInterface.h"
#include "ConfigFmThs.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <math.h>

//callback to JNI regarding AF, RDS data
//extern void RDSDataReceived(Final_RDS_data rdsData);
//extern void RTPlusDataReceived(RTPlus_data rtplusData);
//extern void AFDataReceived(long AF);

//Reset all variables to default value
FmRadioController_qcom :: FmRadioController_qcom
(
)
{
    cur_fm_state = FM_OFF;
    prev_freq = -1;
    seek_canceled = false;
    af_enabled = 0;
    rds_enabled = 0;
    event_listener_canceled = false;
    mutex_fm_state = PTHREAD_MUTEX_INITIALIZER;
    mutex_seek_compl_cond = PTHREAD_MUTEX_INITIALIZER;
    mutex_scan_compl_cond = PTHREAD_MUTEX_INITIALIZER;
    mutex_tune_compl_cond = PTHREAD_MUTEX_INITIALIZER;
    mutex_turn_on_cond = PTHREAD_MUTEX_INITIALIZER;
    turn_on_cond = PTHREAD_COND_INITIALIZER;
    seek_compl_cond = PTHREAD_COND_INITIALIZER;
    scan_compl_cond = PTHREAD_COND_INITIALIZER;
    tune_compl_cond = PTHREAD_COND_INITIALIZER;
    event_listener_thread = 0;
    fd_driver = -1;
    rt_update_thread = 0;
    ert_update_thread = 0;
    rtplus_update_thread = 0;
    ps_update_thread = 0;
    //memset(&rds_data, 0, sizeof(Final_RDS_data));
    //memset(&CurrentRTPlusData, 0, sizeof(RTPlus_data));
}

//Turn off FM
//ToDo: Should all the long operation be stopped?
//prior to setting chip off?
FmRadioController_qcom :: ~FmRadioController_qcom
(
)
{
    if((cur_fm_state != FM_OFF)) {
        SeekCancel();
        CancelSearchAll();
        set_fm_state(FM_OFF_IN_PROGRESS);
        QcomFmIoctlsInterface::set_control(fd_driver,
                        V4L2_CID_PRV_STATE, FM_DEV_NONE);
    }
    if(event_listener_thread != 0) {
        event_listener_canceled = true;
        pthread_join(event_listener_thread, NULL);
    }
}

//Downlaod FM SOC Patches
//Turn On FM Chip
//return FM_FAILURE on failure FM_SUCCESS
//on success
int FmRadioController_qcom :: Initialise
(
)
{
    int ret;
    struct timespec ts;
    ConfigFmThs thsObj;

    if(cur_fm_state == FM_OFF) {
        set_fm_state(FM_ON_IN_PROGRESS);
        fd_driver = open(FM_DEVICE_PATH, O_RDONLY, O_NONBLOCK);
        if(fd_driver < 0) {
            ALOGE("FM kernel driver file open failed: %d\n", fd_driver);
            set_fm_state(FM_OFF);
            return FM_FAILURE;
        }
        ret = QcomFmIoctlsInterface::start_fm_patch_dl(fd_driver);
        if(ret != FM_SUCCESS) {
            ALOGE("FM patch downloader failed: %d\n", ret);
            close(fd_driver);
            fd_driver = -1;
            set_fm_state(FM_OFF);
            return FM_FAILURE;
        }
        if(event_listener_thread == 0) {
           ret = pthread_create(&event_listener_thread, NULL,
                               handle_events, this);
           if(ret == 0) {
              ALOGI("Lock the mutex for FM turn on cond\n");
              pthread_mutex_lock(&mutex_turn_on_cond);
              ts = set_time_out(READY_EVENT_TIMEOUT);
              ret = QcomFmIoctlsInterface::set_control(fd_driver,
                               V4L2_CID_PRV_STATE, FM_RX);
              if(ret == FM_SUCCESS) {
                 ALOGI("Waiting for timedout or FM on\n");
                 pthread_cond_timedwait(&turn_on_cond, &mutex_turn_on_cond, &ts);
                 ALOGI("Unlocked the mutex and timedout or condition satisfied\n");
                 pthread_mutex_unlock(&mutex_turn_on_cond);
                 if(cur_fm_state == FM_ON) {
                    thsObj.SetRxSearchAfThs(FM_PERFORMANCE_PARAMS, fd_driver);
                    return FM_SUCCESS;
                 }else {
                    QcomFmIoctlsInterface::set_control(fd_driver,
                                    V4L2_CID_PRV_STATE, FM_DEV_NONE);
                    event_listener_canceled = true;
                    pthread_join(event_listener_thread, NULL);
                    close(fd_driver);
                    fd_driver = -1;
                    set_fm_state(FM_OFF);
                    return FM_FAILURE;
                 }
              }else {
                 ALOGE("Set FM on control failed\n");
                 pthread_mutex_unlock(&mutex_turn_on_cond);
                 ALOGI("Unlocked the FM on cond mutex\n");
                 event_listener_canceled = true;
                 pthread_join(event_listener_thread, NULL);
                 close(fd_driver);
                 fd_driver = -1;
                 set_fm_state(FM_OFF);
                 return FM_FAILURE;
              }
           }else {
              ALOGE("FM event listener thread failed: %d\n", ret);
              set_fm_state(FM_OFF);
              return FM_FAILURE;
           }
        }else {
           return FM_SUCCESS;
        }
    }else if(cur_fm_state != FM_ON_IN_PROGRESS) {
        return FM_SUCCESS;
    }else {
        return FM_FAILURE;
    }
}

struct timespec FmRadioController_qcom :: set_time_out
(
    int secs
)
{
    struct timespec ts;
    struct timeval tp;

    gettimeofday(&tp, NULL);
    ts.tv_sec = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += secs;

    return ts;
}

//ToDo: Ask Samsung what is the purpose of
//this function to set Antenna or audio path
int FmRadioController_qcom :: setFMIntenna
(
    bool antenna
)
{
    int ret = FM_SUCCESS;

    return ret;
}

int FmRadioController_qcom :: GetAFValid_th
(
    void
)
{
    int ret = -1;

    return ret;
}

void FmRadioController_qcom :: SetAFValid_th
(
    int th
)
{

}

int FmRadioController_qcom :: GetAF_th
(
    void
)
{
    int ret = -1;

    return ret;
}
void FmRadioController_qcom :: SetAF_th
(
    int th
)
{

}
//Get current tuned frequency
//Return -1 if failed to get freq
long FmRadioController_qcom :: GetChannel
(
    void
)
{
    long freq = -1;
    int ret;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
       ret = QcomFmIoctlsInterface::get_cur_freq(fd_driver, freq);
       if(ret == FM_SUCCESS) {
          ALOGI("FM get freq is successfull, freq is: %ld\n", freq);
       }else {
          ALOGE("FM get frequency failed, freq is: %ld\n", freq);
       }
    }else {
       ALOGE("FM get freq is not valid in current state\n");
    }
    return freq;
}

//Tune to a Freq
//Return FM_SUCCESS on success FM_FAILURE
//on failure
int FmRadioController_qcom :: TuneChannel
(
    long freq
)
{
    int ret;
    struct timespec ts;

    if((cur_fm_state == FM_ON) &&
        (freq > 0)) {
        set_fm_state(FM_TUNE_IN_PROGRESS);
        ret = QcomFmIoctlsInterface::set_freq(fd_driver,
                                             freq);
        if(ret == FM_SUCCESS) {
           ALOGI("FM set frequency command set successfully\n");
           pthread_mutex_lock(&mutex_tune_compl_cond);
           ts = set_time_out(TUNE_EVENT_TIMEOUT);
           ret = pthread_cond_timedwait(&tune_compl_cond, &mutex_tune_compl_cond, &ts);
           pthread_mutex_unlock(&mutex_tune_compl_cond);
        }else {
           if((cur_fm_state != FM_OFF)) {
              set_fm_state(FM_ON);
           }
           ALOGE("FM set freq command failed\n");
        }
    }else {
        ALOGE("Fm is not in proper state for tuning to a freq\n");
        ret = FM_FAILURE;
    }
    return ret;
}
//Seeks strong valid channel in upward direction
//return -1 on failure or valid channel on success
long FmRadioController_qcom :: SeekUp
(
    void
)
{
    int ret;
    long freq = -1;
    struct timespec ts;

    if(cur_fm_state == FM_ON) {
        ALOGI("FM seek up started\n");
        set_fm_state(SEEK_IN_PROGRESS);
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCHMODE, SEEK_MODE);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SCANDWELL, SEEK_DWELL_TIME);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCH_PTY, 0);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCH_PI, 0);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::start_search(fd_driver,
                                            SEARCH_UP);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }
        pthread_mutex_lock(&mutex_seek_compl_cond);
        ts = set_time_out(SEEK_COMPL_TIMEOUT);
        ret = pthread_cond_timedwait(&seek_compl_cond, &mutex_seek_compl_cond, &ts);
        pthread_mutex_unlock(&mutex_seek_compl_cond);
        if((cur_fm_state != SEEK_IN_PROGRESS) && !seek_canceled) {
           ALOGI("Seek completed without timeout\n");
           freq = GetChannel();
        }else {
        }
        seek_canceled = false;
        return freq;
    }else {
        ALOGE("FM is not in proper state to start Seek up operation\n");
        return freq;
    }
}

//Seeks strong valid channel in downward direction
//return -1 on failure or valid channel on success
long FmRadioController_qcom :: SeekDown
(
    void
)
{
    long ret;
    long freq = -1;
    struct timespec ts;

    if(cur_fm_state == FM_ON) {
        ALOGI("FM seek down started\n");
        set_fm_state(SEEK_IN_PROGRESS);
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCHMODE, SEEK_MODE);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SCANDWELL, SEEK_DWELL_TIME);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCH_PTY, 0);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCH_PI, 0);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::start_search(fd_driver,
                                            SEARCH_DOWN);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }
        pthread_mutex_lock(&mutex_seek_compl_cond);
        ts = set_time_out(SEEK_COMPL_TIMEOUT);
        ret = pthread_cond_timedwait(&seek_compl_cond, &mutex_seek_compl_cond, &ts);
        pthread_mutex_unlock(&mutex_seek_compl_cond);
        if((cur_fm_state != SEEK_IN_PROGRESS) && !seek_canceled) {
           freq = GetChannel();
        }else {
        }
        seek_canceled = false;
        return freq;
    }else {
        ALOGE("FM is not in proper state to start Seek down operation\n");
        return freq;
    }
}

//Cancel current ongoing seek operation
//return FM_SUCCESS on success, FM_FAILURE
//on failure
int FmRadioController_qcom :: SeekCancel
(
    void
)
{
    int ret;

    if(cur_fm_state == SEEK_IN_PROGRESS) {
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCHON, 0);
        if(ret == FM_SUCCESS) {
            ALOGI("FM Seek cancel command set successfully\n");
            seek_canceled = true;
        }else {
            ALOGE("FM Seek cancel command sent failed\n");
        }
    }else {
        ALOGE("FM is not in proper state for cancelling Seek operation\n");
        ret = FM_FAILURE;
    }
    return ret;
}

void FmRadioController_qcom :: SetSeekRSSI
(
    long rssi
)
{

}

void FmRadioController_qcom :: SetSeekSNR
(
    long snr
)
{

}

void FmRadioController_qcom :: SetRSSI_th
(
    unsigned short rss_th
)
{

}

void FmRadioController_qcom :: SetSNR_th
(
    unsigned char snr_th
)
{

}

void FmRadioController_qcom :: SetCnt_th
(
    unsigned char cnt_th
)
{

}

int FmRadioController_qcom :: GetRSSI_th
(
    void
)
{
    int ret = -1;

    return ret;
}

int FmRadioController_qcom :: GetSNR_th
(
    void
)
{
    int ret = -1;

    return ret;
}

int FmRadioController_qcom :: GetCnt_th
(
    void
)
{
    int ret = -1;

    return ret;
}

//Emphasis:
//75microsec: 0, 50 microsec: 1
//return FM_SUCCESS on success, FM_FAILURE
//on failure
int FmRadioController_qcom :: SetDeConstant
(
    long emphasis
)
{
    int ret;

    if(cur_fm_state == FM_ON) {
        switch(emphasis) {
            case DE_EMP75:
            case DE_EMP50:
                ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_EMPHASIS, emphasis);
                break;
            default:
                ALOGE("FM value pass for set Deconstant is invalid\n");
                ret = FM_FAILURE;
                break;
        }
    }else {
        ALOGE("FM is not in proper state to set De constant\n");
        ret = FM_FAILURE;
    }
    return ret;
}

//ToDo: ASK Samsung diif between
//SeekDown vs SearchDown
long FmRadioController_qcom :: SearchDown
(
    void
)
{
    return SeekDown();
}

//ToDo: ASK Samsung diff between
//SeekUp vs SearchUp
long FmRadioController_qcom :: SearchUp
(
    void
)
{
    return SeekUp();
}

//search all valid stations
//ToDo: return value confirm from JNI layer
//and return freq only when we get tune after scan
//complete event
long FmRadioController_qcom :: SearchAll
(
    void
)
{
    int ret;
    long freq = -1;
    struct timespec ts;

    if(cur_fm_state == FM_ON) {
        ALOGI("FM scan started\n");
        set_fm_state(SCAN_IN_PROGRESS);
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCHMODE, SCAN_MODE);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SCANDWELL, SEARCH_DWELL_TIME);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCH_PTY, 0);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_SRCH_PI, 0);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }

        ret = QcomFmIoctlsInterface::start_search(fd_driver,
                                            SEARCH_UP);
        if(ret != FM_SUCCESS) {
            set_fm_state(FM_ON);
            return freq;
        }
        pthread_mutex_lock(&mutex_scan_compl_cond);
        ts = set_time_out(SCAN_COMPL_TIMEOUT);
        ALOGI("Wait for Scan Timeout or scan complete");
        ret = pthread_cond_timedwait(&scan_compl_cond, &mutex_scan_compl_cond, &ts);
        ALOGI("Scan complete or timedout");
        pthread_mutex_unlock(&mutex_scan_compl_cond);
        if((cur_fm_state != SCAN_IN_PROGRESS)) {

        }else {
        }
        freq = GetChannel();
    }else {
        ALOGE("FM is not in proper state for Scan operation");
    }
    return freq;
}

//Cancels current ongoing scan
//return FM_FAILURE on failure, FM_SUCCESS on success
int FmRadioController_qcom :: CancelSearchAll
(
    void
)
{
    int ret;

    if(cur_fm_state == SCAN_IN_PROGRESS) {
       ret = QcomFmIoctlsInterface::set_control(fd_driver,
                                       V4L2_CID_PRV_SRCHON, 0);
       if(ret == FM_SUCCESS) {
          ALOGI("FM Scan cancel command successfull");
       }else {
          ALOGE("FM Scan cancel command failed");
       }
    }else {
       ALOGE("FM is not in proper state for cancelling scan operation");
       ret = FM_FAILURE;
    }
    return ret;
}
//Get RMSSI for current tuned station
//on failure return -129
//rmssi value: -128 to 127
long FmRadioController_qcom :: GetCurrentRSSI
(
    void
)
{
    int ret;
    //As per qcom implementation max rmssi is -128
    long rmssi = -129;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
        ret = QcomFmIoctlsInterface::get_rmssi(fd_driver, rmssi);
    }else {
    }
    return rmssi;
}

//enable, disable value to receive data of a RDS group
//return FM_SUCCESS on success, FM_FAILURE on failure
int FmRadioController_qcom :: SetRdsGrpProcessing
(
    int grps
)
{
    int ret;
    long mask;

    if(cur_fm_state == FM_ON) {
       ret = QcomFmIoctlsInterface::get_control(fd_driver,
                     V4L2_CID_PRV_RDSGROUP_PROC, mask);
       if(ret != FM_SUCCESS) {
          return ret;
       }
       mask &= 0xC7;
       mask |= ((grps & 0x07) << 3);
       ret = QcomFmIoctlsInterface::set_control(fd_driver,
                    V4L2_CID_PRV_RDSGROUP_PROC, (int)mask);
    }else {
       ret = FM_FAILURE;
    }
    return ret;
}

//Enable RDS data receiving
//Enable RT, PS, AF Jump, RTPLUS, ERT etc
int FmRadioController_qcom :: EnableRDS
(
    void
)
{
    int ret;

    if(cur_fm_state == FM_ON) {
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_RDSON, 1);
        if(ret != FM_SUCCESS) {
            return ret;
        }
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_RT, 1);

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_PS_SIMPLE, 1);

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_3A, 1);

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_ERT, 1);

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_RT_PLUS, 1);

        if(ret != FM_SUCCESS) {
           return ret;
        }
        if(ret == FM_SUCCESS) {
           rds_enabled = 1;
        }
    }else {
        ret = FM_FAILURE;
    }
    return ret;
}

//Disable all RDS data processing
//RT, ERT, RT PLUS, PS
int FmRadioController_qcom :: DisableRDS
(
    void
)
{
    int ret;

    if(cur_fm_state == FM_ON) {
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_IRIS_RDSGRP_RT, 0);
        if(ret != FM_SUCCESS) {
           return ret;
        }
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_IRIS_RDSGRP_PS_SIMPLE, 0);
        if(ret != FM_SUCCESS) {
           return ret;
        }

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_3A, 0);

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_ERT, 0);

        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_PRV_IRIS_RDSGRP_RT_PLUS, 0);

        if(ret != FM_SUCCESS) {
           return ret;
        }
        rds_enabled = 0;
        if(ps_update_thread != 0) {
           pthread_join(ps_update_thread, NULL);
           ps_update_thread = 0;
        }
        if(rtplus_update_thread != 0) {
           pthread_join(rtplus_update_thread, NULL);
           rtplus_update_thread = 0;
        }
        if(ert_update_thread != 0) {
           pthread_join(ert_update_thread, NULL);
           ert_update_thread = 0;
        }
        if(rt_update_thread != 0) {
           pthread_join(rt_update_thread, NULL);
           rt_update_thread = 0;
        }
    }else {
        ret = FM_FAILURE;
    }
    return ret;
}

//Enables Alternate Frequency switching
int FmRadioController_qcom :: EnableAF
(
    void
)
{
    int ret;
    long rdsgrps;

    if(cur_fm_state == FM_ON) {
        ret = QcomFmIoctlsInterface::get_control(fd_driver,
                      V4L2_CID_PRV_RDSGROUP_PROC, rdsgrps);
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_RDSON, 1);
        if(ret == FM_SUCCESS) {
            ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_AF_JUMP, 1);
            //ret = QcomFmIoctlsInterface::set_control(fd_driver,
            //          V4L2_CID_PRV_IRIS_RDSGRP_AFLIST, 1);
            if(ret == FM_SUCCESS) {
               af_enabled = 1;
            }
        }else {
        }
    }else {
        ret = FM_FAILURE;
    }
    return ret;
}

//Disables Alternate Frequency switching
int FmRadioController_qcom :: DisableAF
(
    void
)
{
    int ret;
    long rdsgrps;

    if(cur_fm_state == FM_ON) {
        ret = QcomFmIoctlsInterface::get_control(fd_driver,
                      V4L2_CID_PRV_RDSGROUP_PROC, rdsgrps);
        if(ret == FM_SUCCESS) {
            ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_AF_JUMP, 0);
            if(ret == FM_SUCCESS) {
               af_enabled = 0;
            }
        }else {
        }
    }else {
        ret = FM_FAILURE;
    }
    return ret;
}

//Cancel ongoing af jmp and again enable AF jmp
void FmRadioController_qcom :: CancelAfSwitchingProcess
(
)
{
     if(af_enabled) {
        DisableAF();
        EnableAF();
     }
}

//Set regional band
int FmRadioController_qcom :: SetBand
(
    long band
)
{
    int ret;

    if(cur_fm_state == FM_ON) {
        switch(band) {
            case BAND_87500_108000:
                ret = QcomFmIoctlsInterface::set_band(fd_driver,
                               87500, 108000);
                break;
            case BAND_76000_108000:
                ret = QcomFmIoctlsInterface::set_band(fd_driver,
                               76000, 108000);
                break;
            case BAND_76000_90000:
                ret = QcomFmIoctlsInterface::set_band(fd_driver,
                               76000, 90000);
                break;
            default:
                ALOGE("Band type: %ld is invalid\n", band);
                ret = FM_FAILURE;
                break;
        }
    }else {
        ALOGE("FM is not in proper state to set band type\n");
        ret = FM_FAILURE;
    }
    return ret;
}

//set spacing for successive channels
int FmRadioController_qcom :: SetChannelSpacing
(
    long spacing
)
{
    int ret;

    if(cur_fm_state == FM_ON) {
        switch(spacing)
        {
            case SS_CHAN_SPACE_50:
                ret = QcomFmIoctlsInterface::set_control(fd_driver,
                               V4L2_CID_PRV_CHAN_SPACING, QCOM_CHAN_SPACE_50);
                break;
            case SS_CHAN_SPACE_100:
                ret = QcomFmIoctlsInterface::set_control(fd_driver,
                               V4L2_CID_PRV_CHAN_SPACING, QCOM_CHAN_SPACE_100);
                break;
            case SS_CHAN_SPACE_200:
                ret = QcomFmIoctlsInterface::set_control(fd_driver,
                               V4L2_CID_PRV_CHAN_SPACING, QCOM_CHAN_SPACE_200);
                break;
            default:
                ALOGE("Channel spacing is invalid: %ld\n", spacing);
                ret = FM_FAILURE;
                break;
        }
    }else {
        ALOGE("FM is not in proper state to set the channel spacing\n");
        ret = FM_FAILURE;
    }
    return ret;
}

int FmRadioController_qcom :: SetStereo
(
)
{
    int ret;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
       ret = QcomFmIoctlsInterface::set_audio_mode(fd_driver,
                                           STEREO);
    }else {
       ret = FM_FAILURE;
    }
    return ret;
}

int FmRadioController_qcom :: SetMono
(
)
{
    int ret;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
       ret = QcomFmIoctlsInterface::set_audio_mode(fd_driver,
                                              MONO);
    }else {
       ret = FM_FAILURE;
    }
    return ret;
}

int FmRadioController_qcom :: SetVolume
(
    long vol
)
{
    int ret = 0;

    return ret;
}
int FmRadioController_qcom :: GetVolume
(
    void
)
{
    int ret = -1;

    return ret;
}
int FmRadioController_qcom :: GetMaxVolume
(
    void
)
{
    int ret = -1;

    return ret;
}
void FmRadioController_qcom :: SetRecordMode
(
    int record_mode
)
{

}
void FmRadioController_qcom :: SetSpeakerOn
(
    bool on_off
)
{

}
//HardMute both audio channels
int FmRadioController_qcom :: MuteOn
(
)
{
    int ret;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
       ret = QcomFmIoctlsInterface::set_control(fd_driver,
                       V4L2_CID_AUDIO_MUTE, MUTE_L_R_CHAN);
    }else {
       ret = FM_FAILURE;
    }
    return ret;
}

//Unmute both audio channel
int FmRadioController_qcom :: MuteOff
(
)
{
    int ret;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_AUDIO_MUTE, UNMUTE_L_R_CHAN);
    }else {
        ret = FM_FAILURE;
    }
    return ret;
}

//
int FmRadioController_qcom :: SetSoftMute
(
    bool mode
)
{
    int ret;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                             V4L2_CID_PRV_SOFT_MUTE, mode);
    }else {
        ret = FM_FAILURE;
    }
    return ret;
}

bool FmRadioController_qcom :: GetSoftMute
(
)
{
    int ret = FM_SUCCESS;
    long mode = SMUTE_DISABLED;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
       ret = QcomFmIoctlsInterface::get_control(fd_driver,
                            V4L2_CID_PRV_SOFT_MUTE, mode);
       if(ret == FM_SUCCESS) {
          ALOGI("FM Get soft mute is successful: %ld\n", mode);
       }else {
          ALOGE("FM Get soft mute failed");
       }
    }else {
       ALOGE("FM is not in proper state for getting soft mute\n");
       ret = FM_FAILURE;
    }
    return mode;
}

int FmRadioController_qcom :: get_fm_state
(
)
{
    return cur_fm_state;
}

void FmRadioController_qcom :: set_fm_state
(
    int state
)
{
    pthread_mutex_lock(&mutex_fm_state);
    cur_fm_state = state;
    pthread_mutex_unlock(&mutex_fm_state);
}

void* FmRadioController_qcom :: handle_ps
(
    void *arg
)
{
    int ret;
    char raw_rds[STD_BUF_SIZE];
    char *ps = NULL;
    int len = 0;
    FmRadioController_qcom *obj_p = static_cast<FmRadioController_qcom*>(arg);

    ALOGI("handle ps thread\n");
    if((obj_p->cur_fm_state == FM_ON) && obj_p->rds_enabled) {
       ret = QcomFmIoctlsInterface::get_buffer(obj_p->fd_driver,
                      raw_rds, STD_BUF_SIZE, PS_IND);
       if(ret <= 0) {
          return NULL;
       }else {
          if(raw_rds[PS_STR_NUM_IND] > 0) {
             ps = new char[MAX_PS_LEN + 1];
             if(ps != NULL) {
                for(int i = 0; i < MAX_PS_LEN; i++) {
                    ps[i] = raw_rds[PS_DATA_OFFSET_IND + i];
                    if(ps[i] == 0) {
                        break;
                    }else if((ps[len] <= LAST_CTRL_CHAR) ||
                        (ps[len] >= FIRST_NON_PRNT_CHAR)) {
                        ps[i] = SPACE_CHAR;
                        continue;
                    }
                    len++;
                }
                ps[len] = '\0';
                ALOGI("PS is: %s\n", ps);
                //memcpy(obj_p->rds_data.StationName, ps, (MAX_PS_LEN + 1));
                delete [] ps;
                //RDSDataReceived(obj_p->rds_data);
             }
          }
       }
    }else {
       return NULL;
    }
    return NULL;
}

//ToDo: Parse and proper conversion
//Radio Text, A / B Flag, Len pass to jni layer
void* FmRadioController_qcom :: handle_rt
(
    void *arg
)
{
    int ret;
    char raw_rds[STD_BUF_SIZE];
    char *rt = NULL;
    unsigned int len = 0;
    FmRadioController_qcom *obj_p = static_cast<FmRadioController_qcom*>(arg);

    ALOGI("handle rt thread\n");
    if((obj_p->cur_fm_state == FM_ON) && obj_p->rds_enabled) {
       ret = QcomFmIoctlsInterface::get_buffer(obj_p->fd_driver,
                      raw_rds, STD_BUF_SIZE, RT_IND);
       if(ret <= 0) {
          return NULL;
       }else {
          if((raw_rds[RT_LEN_IND] > 0) &&
             (raw_rds[RT_LEN_IND] <= MAX_RT_LEN)) {
             rt = new char[raw_rds[RT_LEN_IND] + 1];
             if(rt != NULL) {
                for(len = 0; len < raw_rds[RT_LEN_IND]; len++) {
                    rt[len] = raw_rds[RT_DATA_OFFSET_IND + len];
                    ALOGI("Rt byte[%d]: %d\n", len, rt[len]);
                    if((rt[len] <= LAST_CTRL_CHAR) ||
                       (rt[len] >= FIRST_NON_PRNT_CHAR)) {
                       rt[len] = SPACE_CHAR;
                       continue;
                    }
                }
                rt[len] = '\0';
                ALOGI("Rt is: %s\n", rt);
                ALOGI("RT text A / B: %d\n", raw_rds[RT_A_B_FLAG_IND]);
                //memcpy(obj_p->rds_data.RadioText, rt, (len + 1));
                delete [] rt;
                //RDSDataReceived(obj_p->rds_data);
             }
          }
       }
    }else {
       return NULL;
    }
    return NULL;
}

//ToDo: Complete the parsing and conversion
//check library for UTF-8, UCS-2 conversion from byte array
//Ert bytes, encoding, len, alinemnt
void* FmRadioController_qcom :: handle_ert
(
    void *arg
)
{
    int ret;
    char raw_rds[STD_BUF_SIZE];
    char *ert_bytes;
    FmRadioController_qcom *obj_p = static_cast<FmRadioController_qcom*>(arg);

    ALOGI("handle ert thread\n");
    if((obj_p->cur_fm_state == FM_ON) && obj_p->rds_enabled) {
       ret = QcomFmIoctlsInterface::get_buffer(obj_p->fd_driver,
                     raw_rds, STD_BUF_SIZE, ERT_IND);
       if(ret <= 0) {
          return NULL;
       }else {
          ert_bytes = new char[(unsigned int)raw_rds[ERT_LEN_IND]];
          if(ert_bytes != NULL) {
             memcpy(ert_bytes, (raw_rds + ERT_DATA_OFFSET_IND),
                    (unsigned int)raw_rds[ERT_LEN_IND]);
             for(int i = 0; i < raw_rds[ERT_LEN_IND]; i++) {
                 ALOGI("byte[%d] is: %d\n", i, ert_bytes[i]);
             }
             delete [] ert_bytes;
          }
       }
    }else {
       return NULL;
    }
    return NULL;
}

//Handles RT Plus data received from driver
//ToDo: Need to implement Toggle and running info for RT plus data
//Complete the extraction for RT Tags
//Tag type, tag start pos, tag add len, item toggle bit, item running bit
//RT_ERT_FLAG
void* FmRadioController_qcom :: handle_rt_plus
(
    void *arg
)
{
  /*  int ret;
    char raw_rds[STD_BUF_SIZE];
    char tag_code, tag_len, tag_start_pos;
    int i, j = TAGS_DATA_BEGIN_OFFSET;
    int valid_tags_num = 0;
    int len;
    FmRadioController_qcom *obj_p = static_cast<FmRadioController_qcom*>(arg);

    memset(&(obj_p->CurrentRTPlusData), 0, sizeof(RTPlus_data));
    ALOGI("Handle RT Plus Thread\n");
    if((obj_p->cur_fm_state == FM_ON) && obj_p->rds_enabled) {
       ret = QcomFmIoctlsInterface::get_buffer(obj_p->fd_driver,
                     raw_rds, STD_BUF_SIZE, RT_PLUS_IND);
       if(ret <= 0) {
          return NULL;
       }
       //RT Plus for Radio Text
       if((raw_rds[RT_OR_ERT_IND] == 0) &&
          (raw_rds[RT_PLUS_TAGS_NUM_IND] > 0 ) &&
          (raw_rds[RT_PLUS_TAGS_NUM_IND] <= MAX_RT_PLUS_TAGS)) {
          len = strlen(obj_p->rds_data.RadioText);
          ALOGI("Item toggel bit: %d, Item running bit: %d\n",
                   raw_rds[ITEM_TOGGLE_IND], raw_rds[ITEM_RUNNING_IND]);
          for(i = 0; i < raw_rds[RT_PLUS_TAGS_NUM_IND]; i++) {
              tag_code = raw_rds[j++];
              tag_start_pos = raw_rds[j++];
              tag_len = raw_rds[j++];
              if((tag_code == DUMMY_TAG_CODE) || (tag_start_pos < 0)
                 ||(tag_len <= 0) || (tag_start_pos + tag_len) > len) {
              }else {
                 obj_p->CurrentRTPlusData.RTPlusTag[valid_tags_num].contentType =
                                             tag_code;
                 obj_p->CurrentRTPlusData.RTPlusTag[valid_tags_num].startPos =
                                             tag_start_pos;
                 obj_p->CurrentRTPlusData.RTPlusTag[valid_tags_num].additionalLen =
                                             (tag_len - 1);
                 valid_tags_num++;
              }
              obj_p->CurrentRTPlusData.bValidated = RTPLUS_CHECK_BOUND;
              ALOGI("TagCode: %d, TagStartPos: %d, Taglen: %d\n",
                      tag_code, tag_start_pos, tag_len);
          }
          if(valid_tags_num > 0) {
             obj_p->CurrentRTPlusData.bValidated = RTPLUS_CHECK_BOUND;
             obj_p->CurrentRTPlusData.bToggle = raw_rds[ITEM_TOGGLE_IND];
             obj_p->CurrentRTPlusData.bRunning = raw_rds[ITEM_RUNNING_IND];
             //RTPlusDataReceived(obj_p->CurrentRTPlusData);
          }
       }else if(raw_rds[RT_OR_ERT_IND] == 1) {
       }
    }else {
       return NULL;
    }*/
    return NULL;
}

void* FmRadioController_qcom :: handle_events
(
    void *arg
)
{
    int bytesread;
    char event_buff[STD_BUF_SIZE];
    bool status = true;
    FmRadioController_qcom *obj_p = static_cast<FmRadioController_qcom*>(arg);

    while(status && !obj_p->event_listener_canceled) {
        bytesread = QcomFmIoctlsInterface::get_buffer(obj_p->fd_driver,
                      event_buff, STD_BUF_SIZE, EVENT_IND);
        for(int i = 0; i < bytesread; i++) {
            status = obj_p->process_radio_events(event_buff[i]);
            if(status == false) {
                break;
            }
        }
    }
    return NULL;
}

int FmRadioController_qcom :: SetRdsGrpMask
(
    int mask
)
{
    int ret;

    if((cur_fm_state != FM_OFF) &&
       (cur_fm_state != FM_OFF_IN_PROGRESS) &&
       (cur_fm_state != FM_ON_IN_PROGRESS)) {
        ret = QcomFmIoctlsInterface::set_control(fd_driver,
                      V4L2_CID_PRV_RDSGROUP_MASK, mask);
    }else {
        ret = FM_FAILURE;
    }
    return ret;
}

void FmRadioController_qcom :: handle_enabled_event
(
     void
)
{
     ALOGI("FM handle ready Event\n");
     QcomFmIoctlsInterface::set_control(fd_driver,
             V4L2_CID_PRV_AUDIO_PATH, AUDIO_DIGITAL_PATH);
     QcomFmIoctlsInterface::set_calibration(fd_driver);
     pthread_mutex_lock(&mutex_turn_on_cond);
     set_fm_state(FM_ON);
     DisableRDS();
     pthread_cond_broadcast(&turn_on_cond);
     pthread_mutex_unlock(&mutex_turn_on_cond);
}

void FmRadioController_qcom :: handle_tuned_event
(
     void
)
{
     long freq = -1;

     ALOGI("FM handle Tune event\n");
     //memset(&rds_data, 0, sizeof(Final_RDS_data));
     //memset(&CurrentRTPlusData, 0, sizeof(RTPlus_data));
     freq = GetChannel();
     switch(cur_fm_state) {
         case FM_ON:
            if(af_enabled && (freq != prev_freq)
                && (prev_freq > 0)) {
               ALOGI("AF jump happened\n");
               //FDataReceived(freq);
            }
            break;
         case FM_TUNE_IN_PROGRESS:
            pthread_mutex_lock(&mutex_tune_compl_cond);
            set_fm_state(FM_ON);
            pthread_cond_broadcast(&tune_compl_cond);
            pthread_mutex_unlock(&mutex_tune_compl_cond);
            break;
         case SEEK_IN_PROGRESS:
            pthread_mutex_lock(&mutex_seek_compl_cond);
            set_fm_state(FM_ON);
            pthread_cond_broadcast(&seek_compl_cond);
            pthread_mutex_unlock(&mutex_seek_compl_cond);
            break;
         case SCAN_IN_PROGRESS:
            break;
     }
     prev_freq = freq;
}

void FmRadioController_qcom :: handle_seek_next_event
(
     void
)
{
     ALOGI("FM handle seek next event\n");
}

void FmRadioController_qcom :: handle_seek_complete_event
(
     void
)
{
     ALOGI("FM handle seek complete event\n");
     switch(cur_fm_state) {
         case SEEK_IN_PROGRESS:
            break;
         case SCAN_IN_PROGRESS:
            pthread_mutex_lock(&mutex_scan_compl_cond);
            set_fm_state(FM_ON);
            pthread_cond_broadcast(&scan_compl_cond);
            pthread_mutex_unlock(&mutex_scan_compl_cond);
            break;
         default:
            break;
     }
}

void FmRadioController_qcom :: handle_raw_rds_event
(
     void
)
{

}

void FmRadioController_qcom :: handle_rt_event
(
     void
)
{
     ALOGI("FM handle RT event\n");
     if(rt_update_thread != 0) {
        pthread_join(rt_update_thread, NULL);
        rt_update_thread = 0;
     }
     pthread_create(&rt_update_thread, NULL, handle_rt, this);
}

void FmRadioController_qcom :: handle_ps_event
(
    void
)
{
    ALOGI("FM handle PS event\n");
    if(ps_update_thread != 0) {
       pthread_join(ps_update_thread, NULL);
       ps_update_thread = 0;
    }
    pthread_create(&ps_update_thread, NULL, handle_ps, this);
}

void FmRadioController_qcom :: handle_error_event
(
   void
)
{

}

void FmRadioController_qcom :: handle_below_th_event
(
   void
)
{

}

void FmRadioController_qcom :: handle_above_th_event
(
   void
)
{

}

void FmRadioController_qcom :: handle_stereo_event
(
   void
)
{

}
void FmRadioController_qcom :: handle_mono_event
(
  void
)
{

}

void FmRadioController_qcom :: handle_rds_aval_event
(
  void
)
{

}

void FmRadioController_qcom :: handle_rds_not_aval_event
(
  void
)
{

}

void FmRadioController_qcom :: handle_srch_list_event
(
  void
)
{

}

void FmRadioController_qcom :: handle_af_list_event
(
  void
)
{
    char raw_rds[STD_BUF_SIZE];
    int ret;
    ULINT lower_band;

    ALOGI("Got af list event\n");
    ret = QcomFmIoctlsInterface::get_buffer(fd_driver,
                     raw_rds, STD_BUF_SIZE, AF_LIST_IND);
    lower_band = QcomFmIoctlsInterface::get_lowerband_limit(fd_driver,
                     lower_band);
    ALOGI("raw_rds[0]: %d\n", (raw_rds[0] & 0xff));
    ALOGI("raw_rds[1]: %d\n", (raw_rds[1] & 0xff));
    ALOGI("raw_rds[2]: %d\n", (raw_rds[2] & 0xff));
    ALOGI("raw_rds[3]: %d\n", (raw_rds[3] & 0xff));
    ALOGI("raw_rds[4]: %d\n", (raw_rds[4] & 0xff));
    ALOGI("raw_rds[5]: %d\n", (raw_rds[5] & 0xff));
    ALOGI("raw_rds[6]: %d\n", (raw_rds[6] & 0xff));

    for(int i = 0; i < raw_rds[6]; i++) {
        ALOGI("freq[%d][0]: %d, freq[%d][1]: %d, ",
             i, raw_rds[i + 7], i, raw_rds[(i + 1) + 7]);
        ALOGI("freq[%d][2]: %d, freq[%d][3]: %d\n",
             i, raw_rds[(i + 2) + 7], i, raw_rds[(i + 3) + 7]);
       //ALOGI("AF FREQ %d\n: %ld\n", i, (raw_rds[i + 4] & 0xff) * 1000 + lower_band);
    }
}

void FmRadioController_qcom :: handle_disabled_event
(
  void
)
{
     //Expected disabled
     if(cur_fm_state == FM_OFF_IN_PROGRESS) {
        ALOGI("Expected disabled event\n");
     }else {//Enexpected disabled
        ALOGI("Unexpected disabled event\n");
     }

     set_fm_state(FM_OFF);

     close(fd_driver);
     fd_driver = -1;

     if(ps_update_thread != 0) {
        pthread_join(ps_update_thread, NULL);
        rt_update_thread = 0;
     }
     if(rtplus_update_thread != 0) {
        pthread_join(rtplus_update_thread, NULL);
        rtplus_update_thread = 0;
     }
     if(ert_update_thread != 0) {
        pthread_join(ert_update_thread, NULL);
        ert_update_thread = 0;
     }
     if(rt_update_thread != 0) {
        pthread_join(rt_update_thread, NULL);
        rt_update_thread = 0;
     }
     //allow tune function to exit
     pthread_mutex_lock(&mutex_tune_compl_cond);
     pthread_cond_broadcast(&tune_compl_cond);
     pthread_mutex_unlock(&mutex_tune_compl_cond);
     //allow scan function to exit
     pthread_mutex_lock(&mutex_scan_compl_cond);
     pthread_cond_broadcast(&scan_compl_cond);
     pthread_mutex_unlock(&mutex_scan_compl_cond);
     //Allow seek function to exit
     pthread_mutex_lock(&mutex_seek_compl_cond);
     pthread_cond_broadcast(&seek_compl_cond);
     pthread_mutex_unlock(&mutex_seek_compl_cond);
}

void FmRadioController_qcom :: handle_rds_grp_mask_req_event
(
    void
)
{
    SetRdsGrpMask(0);
}

void FmRadioController_qcom :: handle_rt_plus_event
(
    void
)
{
    ALOGI("FM handle RT Plus event\n");
    if(rtplus_update_thread != 0) {
       pthread_join(rtplus_update_thread, NULL);
       rtplus_update_thread = 0;
    }
    pthread_create(&rtplus_update_thread, NULL, handle_rt_plus, this);
}

void FmRadioController_qcom :: handle_af_jmp_event
(
    void
)
{
    long freq = -1;

    freq = GetChannel();
    ALOGI("FM handle AF Jumped event\n");
    if(af_enabled && (freq != prev_freq)) {
       ALOGI("AF Jump occured, prevfreq is: %ld, af freq is: %ld\n", prev_freq, freq);
       //AFDataReceived(freq);
    }
    prev_freq = freq;
}

void FmRadioController_qcom :: handle_ert_event
(
    void
)
{
    ALOGI("FM handle ERT event\n");
    if(ert_update_thread != 0) {
       pthread_join(ert_update_thread, NULL);
       ert_update_thread = 0;
    }
    pthread_create(&ert_update_thread, NULL, handle_ert, this);
}

bool FmRadioController_qcom :: process_radio_events
(
    int event
)
{
    bool ret = true;

    switch(event) {
        case READY_EVENT:
            handle_enabled_event();
            break;
        case TUNE_EVENT:
            handle_tuned_event();
            break;
        case SEEK_COMPLETE_EVENT:
            handle_seek_complete_event();
            break;
        case SCAN_NEXT_EVENT:
            handle_seek_next_event();
            break;
        case RAW_RDS_EVENT:
            handle_raw_rds_event();
            break;
        case RT_EVENT:
            handle_rt_event();
            break;
        case PS_EVENT:
            handle_ps_event();
            break;
        case ERROR_EVENT:
            handle_error_event();
            break;
        case BELOW_TH_EVENT:
            handle_below_th_event();
            break;
        case ABOVE_TH_EVENT:
            handle_above_th_event();
            break;
        case STEREO_EVENT:
            handle_stereo_event();
            break;
        case MONO_EVENT:
            handle_mono_event();
            break;
        case RDS_AVAL_EVENT:
            handle_rds_aval_event();
            break;
        case RDS_NOT_AVAL_EVENT:
            handle_rds_not_aval_event();
            break;
        case SRCH_LIST_EVENT:
            handle_srch_list_event();
            break;
        case AF_LIST_EVENT:
            handle_af_list_event();
            break;
        case DISABLED_EVENT:
            handle_disabled_event();
            ret = false;
            break;
        case RDS_GRP_MASK_REQ_EVENT:
            handle_rds_grp_mask_req_event();
            break;
        case RT_PLUS_EVENT:
            handle_rt_plus_event();
            break;
        case ERT_EVENT:
            handle_ert_event();
            break;
        case AF_JMP_EVENT:
            handle_af_jmp_event();
            break;
        default:
            break;
    }
    return ret;
}
