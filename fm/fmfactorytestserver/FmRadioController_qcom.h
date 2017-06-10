/**
 *  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *
 *  Overview: This file declares or exposes various fm functionalities
 *
 **/

#ifndef __QCOM_FM_RADIO_CTRL_H__
#define __QCOM_FM_RADIO_CTRL_H___

#include <pthread.h>
#include <ctime>

class FmRadioController_qcom
{
    private:
        int cur_fm_state;
        char af_enabled;
        bool seek_canceled;
        bool event_listener_canceled;
        pthread_mutex_t mutex_fm_state;
        pthread_mutex_t mutex_turn_on_cond;
        pthread_mutex_t mutex_seek_compl_cond;
        pthread_mutex_t mutex_scan_compl_cond;
        pthread_mutex_t mutex_tune_compl_cond;
        pthread_cond_t turn_on_cond;
        pthread_cond_t seek_compl_cond;
        pthread_cond_t scan_compl_cond;
        pthread_cond_t tune_compl_cond;
        char rds_enabled;
        long int prev_freq;
        int fd_driver;
        pthread_t ps_update_thread;
        pthread_t event_listener_thread;
        pthread_t rt_update_thread;
        pthread_t ert_update_thread;
        pthread_t rtplus_update_thread;
        int SetRdsGrpMask(int mask);
        int SetRdsGrpProcessing(int grps);
        void handle_enabled_event(void);
        void handle_tuned_event(void);
        void handle_seek_next_event(void);
        void handle_seek_complete_event(void);
        void handle_raw_rds_event(void);
        void handle_rt_event(void);
        void handle_ps_event(void);
        void handle_error_event(void);
        void handle_below_th_event(void);
        void handle_above_th_event(void);
        void handle_stereo_event(void);
        void handle_mono_event(void);
        void handle_rds_aval_event(void);
        void handle_rds_not_aval_event(void);
        void handle_srch_list_event(void);
        void handle_af_list_event(void);
        void handle_disabled_event(void);
        void handle_rds_grp_mask_req_event(void);
        void handle_rt_plus_event(void);
        void handle_ert_event(void);
        void handle_af_jmp_event(void);
        void set_fm_state(int state);
        struct timespec set_time_out(int secs);
    public:
       FmRadioController_qcom();
       ~FmRadioController_qcom();
       int Initialise(void);
       int setFMIntenna(bool);
       long GetChannel(void);
       long SeekUp(void);
       long SeekDown(void);
       int SeekCancel(void);
       int TuneChannel(long);
       // emphasis
       int SetDeConstant(long );
       long SearchAll(void);
       int CancelSearchAll(void);
       long GetCurrentRSSI(void);
       int EnableRDS(void);
       int DisableRDS(void);
       int EnableAF(void);
       int DisableAF(void);
       void CancelAfSwitchingProcess(void);
       int SetBand(long);
       int SetChannelSpacing(long);
       int SetStereo(void);
       int SetMono(void);
       int MuteOn(void);
       int MuteOff(void);
       int SetSoftMute(bool mode);
       bool GetSoftMute(void);
       int get_fm_state(void);
       static void* handle_events(void *arg);
       static void* handle_ps(void *arg);
       static void* handle_rt(void *arg);
       static void* handle_ert(void *arg);
       static void* handle_rt_plus(void *arg);
       bool process_radio_events(int event);
};

#endif
