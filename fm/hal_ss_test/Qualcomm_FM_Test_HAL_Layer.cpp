/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 **/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "FmRadioController_qcom.h"

static const unsigned int FN_WIDTH = 20;
static const unsigned int DSCRP_WIDTH = 50;
static pthread_t seek_up_th;
static pthread_t seek_down_th;
static pthread_t search_all_th;
static pthread_t cancel_seek_th;
static pthread_t cancel_search_th;

enum FUNCTION_MAP {
    ENABLE,
    DISABLE,
    SET_STATION,
    GET_STATION,
    SEEK_UP,
    SEEK_DOWN,
    SCAN,
    CANCEL_SEEK,
    CANCEL_SEARCH,
    SET_REGION,
    SET_MONO,
    SET_STEREO,
    SET_SOFT_MUTE,
    GET_SOFT_MUTE,
    MUTE,
    UNMUTE,
    ENABLE_RDS,
    DISABLE_RDS,
    ENABLE_AF,
    DISABLE_AF,
    SET_CHAN_SPACE,
    SET_DECONST,
};

static const char *const FUNCTIONS [] = {
    "Enable",
    "Disable",
    "SetStation",
    "GetStation",
    "SeekUp",
    "SeekDown",
    "Scan",
    "CancelSeek",
    "CancelSearch",
    "SetRegion",
    "SetMono",
    "SetStereo",
    "SetSoftMute",
    "GetSoftMute",
    "Mute",
    "Unmute",
    "EnableRDS",
    "DisableRDS",
    "EnableAF",
    "DisableAF",
    "ChannelSpace",
    "SetDeconst",
};

static const char *const FUNS_DSCRP[] =
{
    "Turn on Fm",
    "Turn off Fm",
    "Tune to a frequency",
    "Get tuned frequency",
    "Seek next valid station in up direction",
    "Seek next valid station in down direction",
    "Seek all valid channels",
    "Cancel seek operation",
    "Cancel scan in progress",
    "Set FM band type",
    "Set Mono audio quality",
    "Set Stereo audio quality",
    "Enable blending to avoid noise",
    "Is soft mute enabled or disabled",
    "Mute audio from Soc",
    "Unmute audio from Soc",
    "Enable RDS data processing",
    "Disable RDS data processing",
    "Enable AF jump switching",
    "Disable AF jump switching",
    "Set channel spacing",
    "Set De constant for channel"
};

void print_functions(void);
void handle_user_input(FmRadioController_qcom *&controller);
void enable_option(FmRadioController_qcom *&controller);
void disable_option(FmRadioController_qcom *&controller);
void set_station_option(FmRadioController_qcom *&controller);
void get_station_option(FmRadioController_qcom *&controller);
static void *seek_up_option(void *arg);
static void *seek_down_option(void *arg);
static void *search_all_option(void *arg);
static void *cancel_seek_option(void *arg);
static void *cancel_search_option(void *arg);
void set_region_option(FmRadioController_qcom *&controller);
void set_mono_option(FmRadioController_qcom *&controller);
void set_stere_option(FmRadioController_qcom *&controller);
void set_soft_mute_option(FmRadioController_qcom *&controller);
void get_soft_mute_option(FmRadioController_qcom *&controller);
void set_mute_on_option(FmRadioController_qcom *&controller);
void set_mute_off_option(FmRadioController_qcom *&controller);
void enable_rds_option(FmRadioController_qcom *&controller);
void disable_rds_option(FmRadioController_qcom *&controller);
void enable_af_jmp_option(FmRadioController_qcom *&controller);
void disable_af_jmp_option(FmRadioController_qcom *&controller);
void set_spacing_option(FmRadioController_qcom *&controller);
void set_deconst_option(FmRadioController_qcom *&controller);

int main()
{


    FmRadioController_qcom *controller  = NULL;

    do {
        print_functions();
        if(controller == NULL) {
            controller = new FmRadioController_qcom();
        }
        handle_user_input(controller);
    }while(true);

    if(controller != NULL) {
       printf("Exiting the main function so turning off FM\n");
       delete controller;
       controller = NULL;
    }

    return 0;
}

void enable_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to turn ON FM\n");
    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->Initialise();
        if(ret == 0) {
           printf("FM turned on successfully\n");
        }else {
          printf("FM on failed\n");
        }
    }
}

void disable_option(FmRadioController_qcom *&controller)
{
    printf("You have selected option to turn OFF FM\n");
    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        //pthread_join(seek_up_th, NULL);
        //pthread_join(seek_down_th, NULL);
        //pthread_join(cancel_seek_th, NULL);
        delete controller;
        controller = NULL;
    }
}

void set_station_option(FmRadioController_qcom *&controller)
{
    long freq;
    int ret;

    printf("You have selected option to set frequency\n");
    if(controller == NULL) {
        printf("controller is null\n");
    }else {
        printf("Please enter frequency: ");
        scanf("%ld", &freq);
        ret = controller->TuneChannel(freq);
        if(ret == 0) {
            printf("Command sent successfully\n");
        }else {
            printf("Failed to tune station\n");
        }
    }
}

void get_station_option(FmRadioController_qcom *&controller)
{
    long freq;;

    printf("You have selected option to get frequency\n");
    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        freq = controller->GetChannel();
        printf("Tuned  freq is: %ld\n", freq);
    }
}

static void *seek_up_option(void *arg)
{
    long freq;
    FmRadioController_qcom *controller = (FmRadioController_qcom *)arg;

    printf("You have selected option to Seek up\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        freq = controller->SeekUp();
        printf("Seeked up freq is: %ld\n", freq);
    }
    return NULL;
}

static void *seek_down_option(void *arg)
{
    long freq;
    FmRadioController_qcom *controller = (FmRadioController_qcom *)arg;

    printf("You have selected option to seek down\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        freq = controller->SeekDown();
        printf("Seeked down freq is: %ld\n", freq);
    }
    return NULL;
}

static void *search_all_option(void *arg)
{
    int ret;
    FmRadioController_qcom *controller = (FmRadioController_qcom *)arg;

    printf("You have selected option to scan\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->SearchAll();
        if(ret > 0) {
            printf("Scan command successful, tuneed to: %d\n", ret);
        }else {
            printf("Scan failed\n");
        }
    }
    return NULL;
}

static void *cancel_seek_option(void *arg)
{
    int ret;
    FmRadioController_qcom *controller = (FmRadioController_qcom *)arg;

    printf("You have selected option to cancel seek\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->SeekCancel();
        if(ret == 0) {
            printf("Seek stop command successful\n");
        }else {
            printf("Seek stop failed\n");
        }
    }
    return NULL;
}

static void *cancel_search_option(void *arg)
{
    int ret;
    FmRadioController_qcom *controller = (FmRadioController_qcom *)arg;

    printf("You have selected option to cancel scan\n");

    if(controller == NULL) {
       printf("Controller is null\n");
    }else {
       ret = controller->CancelSearchAll();
       if(ret == 0) {
          printf("Scan cancle successful\n");
       }else {
          printf("Scan cancel failed\n");
       }
    }
    return NULL;
}

void set_region_option(FmRadioController_qcom *&controller)
{
    int band;
    int ret;

    printf("You have selected option to set region\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        printf("Please select following options to set region\n");
        printf("1 for 87500 - 108000\n");
        printf("2 for 76000 - 108000\n");
        printf("3 for 76000 - 90000\n");
        scanf("%d", &band);
        ret = controller->SetBand(band);
        if(ret == 0) {
            printf("Set region command successful\n");
        }else {
            printf("Set region failed\n");
        }
    }
}

void set_mono_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to set mono\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->SetMono();
        if(ret == 0) {
            printf("Set mono command successful\n");
        }else {
            printf("Set mono failed\n");
        }
    }
}

void set_stere_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to set Stereo\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->SetStereo();
        if(ret == 0) {
            printf("Set stereo command successful\n");
        }else {
            printf("Set stereo failed\n");
        }
    }
}

void set_soft_mute_option(FmRadioController_qcom *&controller)
{
    int ret;
    int enable;
    bool option;

    printf("You have selected option to enable or disable soft mute\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        printf("Please enter 0 for soft mute disable and 1 for enable: ");
        scanf("%d", &enable);
        if((enable != 0) && (enable != 1)) {
          return;
        }else {
          option = (bool)enable;
        }
        ret = controller->SetSoftMute(option);
        if(ret == 0) {
            printf("Set soft mute command successful\n");
        }else {
            printf("Set soft mute failed\n");
        }
    }
}

void get_soft_mute_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to get soft mute\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->GetSoftMute();
        printf("Get soft mute command: %d\n", ret);
    }
}

void set_mute_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to hard mute\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->MuteOn();
        if(ret == 0) {
            printf("Set hard mute command successful\n");
        }else {
            printf("Set hard mute failed\n");
        }
    }
}

void set_mute_off_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to hard unmute\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->MuteOff();
        if(ret == 0) {
            printf("hard umute command successful\n");
        }else {
            printf("hard umute failed\n");
        }
    }
}

void enable_rds_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to enable rds data processing\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->EnableRDS();
        if(ret == 0) {
            printf("Enable RDS command successful\n");
        }else {
            printf("Enable RDS  failed\n");
        }
    }
}
void disable_rds_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to Disable rds data processing\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->DisableRDS();
        if(ret == 0) {
            printf("Disble RDS command successful\n");
        }else {
            printf("Disable RDS  failed\n");
        }
    }
}
void enable_af_jmp_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to Enable AF Jump switching\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->EnableAF();
        if(ret == 0) {
            printf("Enable AF jump successful\n");
        }else {
            printf("Enable AF jump failed\n");
        }
    }

}
void disable_af_jmp_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to Disable AF Jump switching\n");

    if(controller == NULL) {
        printf("Controller is null\n");
    }else {
        ret = controller->DisableAF();
        if(ret == 0) {
            printf("Disable AF jump successful\n");
        }else {
            printf("Disable AF jump failed\n");
        }
    }
}

void set_spacing_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to set channel spacing\n");

    if(controller == NULL) {
       printf("Controller is null\n");
    }else {
       printf("Enter 5 for 50khz\n");
       printf("Enter 10 for 100khz\n");
       printf("Enter 20 for 200khz\n");
       printf("Please enter the channel spacing:");
       scanf("%d", &ret);
       ret = controller->SetChannelSpacing(ret);
       if(ret == 0) {
         printf("Set channel spacing successful\n");
       }else {
         printf("Set channel spacing failed\n");
       }
    }
}

void set_deconst_option(FmRadioController_qcom *&controller)
{
    int ret;

    printf("You have selected option to set De constant for band\n");

    if(controller == NULL) {
       printf("Controller is null\n");
    }else {
       printf("Enter 0 for 75microsec\n");
       printf("Enter 1 for 50microsec\n");
       printf("Please enter the De-constant :");
       scanf("%d", &ret);
       ret = controller->SetDeConstant(ret);
       if(ret == 0) {
         printf("Set channel spacing successful\n");
       }else {
         printf("Set channel spacing failed\n");
       }
    }
}

void handle_user_input(FmRadioController_qcom *&controller)
{
    int option;

    if(controller == NULL) {
        return;
    }

    printf("Please enter your options to execute command: ");
    scanf("%d", &option);

    switch(option) {
        case ENABLE:
            enable_option(controller);
            break;
        case DISABLE:
            disable_option(controller);
            break;
        case SET_STATION:
            set_station_option(controller);
            break;
        case GET_STATION:
            get_station_option(controller);
            break;
        case SEEK_UP:
            pthread_join(seek_up_th, NULL);
            pthread_create(&seek_up_th, NULL,
                    seek_up_option, controller);
            break;
        case SEEK_DOWN:
            pthread_join(seek_down_th, NULL);
            pthread_create(&seek_down_th, NULL,
                    seek_down_option, controller);
            break;
        case SCAN:
            pthread_join(search_all_th, NULL);
            pthread_create(&search_all_th, NULL,
                    search_all_option, controller);
            break;
        case CANCEL_SEEK:
            pthread_join(cancel_seek_th, NULL);
            pthread_create(&cancel_seek_th, NULL,
                    cancel_seek_option, controller);
            break;
        case CANCEL_SEARCH:
            pthread_join(cancel_search_th, NULL);
            pthread_create(&cancel_search_th, NULL,
                    cancel_search_option, controller);
            break;
        case SET_REGION:
            set_region_option(controller);
            break;
        case SET_MONO:
            set_mono_option(controller);
            break;
        case SET_STEREO:
            set_stere_option(controller);
            break;
        case SET_SOFT_MUTE:
            set_soft_mute_option(controller);
            break;
        case GET_SOFT_MUTE:
            get_soft_mute_option(controller);
            break;
        case MUTE:
            set_mute_option(controller);
            break;
        case UNMUTE:
            set_mute_off_option(controller);
            break;
        case ENABLE_RDS:
            enable_rds_option(controller);
            break;
        case DISABLE_RDS:
            disable_rds_option(controller);
            break;
        case ENABLE_AF:
            enable_af_jmp_option(controller);
            break;
        case DISABLE_AF:
            disable_af_jmp_option(controller);
            break;
        case SET_CHAN_SPACE:
            set_spacing_option(controller);
            break;
        case SET_DECONST:
            set_deconst_option(controller);
            break;
        default:
            break;
    }
}

void print_functions(void)
{
    int f_l = sizeof(FUNCTIONS) / sizeof(FUNCTIONS[0]);
    int d_l = sizeof(FUNS_DSCRP) / sizeof(FUNS_DSCRP[0]);

    printf("Total %d Functions to check\n", f_l);
    printf("SN ");
    printf("%-*s ", FN_WIDTH, "Functions");
    printf("%-*s\n", DSCRP_WIDTH, "Description");
    printf("**********************************************\n");

    for(int i = 0; i < f_l; i++) {
        printf("%d. %-*s ", i, FN_WIDTH, FUNCTIONS[i]);
        if(i < d_l) {
            printf("%-*s", DSCRP_WIDTH, FUNS_DSCRP[i]);
        }
        printf("\n");
    }
    printf("\n**********************************************\n");
}
