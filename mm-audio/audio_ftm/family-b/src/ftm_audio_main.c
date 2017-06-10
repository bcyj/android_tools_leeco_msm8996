/*==========================================================================

                     FTM Audio Main Task Source File

Description
  Unit test component file for regsitering the routines to Diag library
  for AUDIO FTM commands

===========================================================================*/

/*===========================================================================
Copyright (c) 2011-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/09/11   zhongl   Creation

===========================================================================*/
#include "event.h"
#include "msg.h"
#include "log.h"

#include "diag_lsm.h"
#include "stdio.h"
#include <unistd.h>
#include <pthread.h>


#include "diagpkt.h"
#include "diagcmd.h"
#include "diag.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <sys/time.h>
#include <getopt.h>
#include <semaphore.h>
#include <string.h>

#include "DALSYS_common.h"
#include "ftm_audio_diag_dispatch.h"
#include "ftm_audio_dispatch.h"

#define FTM_DEBUG
#ifdef ANDROID
/* definitions for Android logging */
#include <utils/Log.h>
#include <cutils/properties.h>
#else /* ANDROID */
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#define ALOGI(...)      fprintf(stdout, __VA_ARGS__)
#define ALOGE(...)      fprintf(stderr, __VA_ARGS__)
#define ALOGV(...)      fprintf(stderr, __VA_ARGS__)
#define ALOGD(...)      fprintf(stderr, __VA_ARGS__)
#endif /* ANDROID */

/* Semaphore to monitor the completion of the issued test command */
sem_t semaphore_cmd_complete;

/* Semaphore to monitor the coming of the new FTM command */
sem_t semaphore_cmd_coming;

/* Callback declaration for AUDIO FTM packet processing */
PACK(void *) audio_ftm_diag_dispatch (PACK(void *)req_pkt,
  uint16 pkt_len);

/* Diag pkt table for AUDIO */
static const diagpkt_user_table_entry_type audio_ftm_diag_func_table[] =
{
  {FTM_AUDIO_C,FTM_AUDIO_C, audio_ftm_diag_dispatch},
};


/* ==========================================================
 Unit test program for debug purpose
=============================================================*/

void signal_handler(int sig);
void test_ftm_audio_tone_play( int testcase);
void test_ftm_audio_afe_loopback( int testcase);
void test_ftm_audio_adie_loopback( int32 lp_case);
void test_ftm_audio_volume( int32 vol1, int32 vol2, int32 vol3);
void test_ftm_pcm_record( int32 lp_case);
int execute_test_case(int test_case, int codec, FILE *fp, int vol,
          int fl, int fh, int dur, char *filename);
int parse(struct test_params *commands);
int g_config_test;
/*===========================================================================
FUNCTION   audio_ftm_diag_dispatch

DESCRIPTION
  Processes the request packet and sends it to the FTM FM layer for further
  processing

DEPENDENCIES
  NIL

RETURN VALUE
  pointer to FTM FM Response packet

SIDE EFFECTS
  None

===========================================================================*/
PACK(void *) audio_ftm_diag_dispatch
(
  PACK(void *)req_pkt,
  uint16 pkt_len
)
{
 PACK(void *)rsp = NULL;

 printf("\n Receiving one command from DIAG \n");

 sem_post(&semaphore_cmd_coming);

 rsp = ftm_audio_dispatch(req_pkt,pkt_len);

 printf("\n command is executed \n");

 sem_post(&semaphore_cmd_complete);

 return rsp;
}

/*===========================================================================
FUNCTION  ftm_audio_main_signal_handler

DESCRIPTION
  handle signals for Ctl^C

DEPENDENCIES
  NIL

RETURN VALUE

SIDE EFFECTS
  None

===========================================================================*/

void ftm_audio_main_signal_handler( int thesignal)
{
     if(thesignal == SIGINT)
         {
             exit(0);
         }
}

/*===========================================================================
FUNCTION   main

DESCRIPTION
  Initialises the Diag library and registers the PKT table for AUDIO FTM
  and daemonises

DEPENDENCIES
  NIL

RETURN VALUE
  NIL, Error in the event buffer will mean a NULL App version and Zero HW
  version

SIDE EFFECTS
  None

===========================================================================*/

int main(int argc, char *argv[])
{
  boolean bInit_Success = FALSE;
  struct timespec ts;
  int sem_status;
  int index;
  char c;
  char filename[100], *pfilename = NULL;
  int codec, total_test = 0, result, test_case;
  int volume = 50, duration = 3, fl = 300, fh = 4000, count = 0;

    if(0 != pthread_mutex_init(&params.lock, NULL))
    {
        printf("Mutex init failed.\n");
        return -1;
    }

    if( signal( SIGTERM,signal_handler) == SIG_ERR)
    {
        printf("SIGTERM registration failed.\n");
        return -1;
    }

#ifdef MSM8960_ALSA
    FILE *fp;
    FILE *fp_config;
    char soundCardInfo[200];
    g_config_test = 0;
    if((fp = fopen("/proc/asound/cards","r")) == NULL) {
      printf("Cannot open /proc/asound/cards file to get sound card info\n");
      return -1;
    } else {
        while((fgets(soundCardInfo, sizeof(soundCardInfo), fp) != NULL)) {
            printf("SoundCardInfo %s", soundCardInfo);
            if (strstr(soundCardInfo, "msm8930-sitar-snd-card")) {
                printf("Detected sitar 1.x sound card\n");
                codec = CODEC_SITAR;
                total_test = 27;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-mtp-snd-card")) {
                printf("Detected taiko sound card\n");
                codec = CODEC_CONFIG_SUPPORT;
                total_test = 44;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-fluid-snd-card")) {
                printf("Detected taiko sound card\n");
                codec = CODEC_CONFIG_SUPPORT;
                total_test = 44;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-cdp-snd-card")) {
                printf("Detected taiko sound card\n");
                codec = CODEC_TAIKO;
                total_test = 44;
                break;
            } else if (strstr(soundCardInfo, "msm8974-taiko-liquid-snd-card")) {
                printf("Detected taiko sound card\n");
                codec = CODEC_TAIKO;
                total_test = 44;
                break;
            } else if (strstr(soundCardInfo, "mdm9625-taiko-i2s-snd-card")) {
                printf("Detected taiko sound card I2S\n");
                codec = CODEC_TAIKO_I2S;
                total_test = 20;
                break;
            } else if ((fp_config = fopen("/etc/ftm_test_config","r")) != NULL) {
                printf("Use the codec ftm_test_config file\n");
                codec = CODEC_CONFIG_SUPPORT;
                fclose(fp_config);
                fp_config = NULL;
                break;
            } else {
                printf("Detected tabla 2.x sound card\n");
                codec = CODEC_TABLA;
                total_test = 44;
                break;
            }
        }
        fclose(fp);
        fp = NULL;
    }
    /* FIFO Mode supported for some particular codec with config file support*/
    if (argc == 1) {
        static char test[512];
	char *test_r;
        static config_file[100];
        static char *myfifo = "/data/misc/audio/ftm_commands";
        char *token = NULL;
        int fifofd, config_found = 0;
        g_config_test = 1;
        test_case = -1;
        if (codec != CODEC_CONFIG_SUPPORT) {
            printf("\n Codecs with FTM configuration support only");
            return -1;
        }
        remove(myfifo);
        fp = NULL;
        mkfifo(myfifo, 0666) ;
        fifofd = open(myfifo, O_RDONLY);
        if (fifofd < 0) {
            printf("\n Error in creating a fifo");
            return -1;
        }
        printf("\n echo commands to %s", myfifo);
        int wait_for_command = 0;
        while(1) {
           test_case = -1;
           count = read(fifofd, test, 510);
           if (count < 0) {
               printf("\n read err %d", count);
               if (fifofd > 0) {
                  close(fifofd);
                  remove(myfifo);
              }
              return -1;
           }
           if (!count || count == 1) {
               if (!wait_for_command) {
                   printf("\nwaiting for command or quit");
               }
               wait_for_command = 1;
              sleep(1);
              continue;
           }
           test[count-1] = ' ';
           test[count] = '\0';
           wait_for_command = 0;
           if (!strncmp(test, "quit", 4)) {
              printf("\nQUIT");
              if (fifofd > 0) {
                  close(fifofd);
                  remove(myfifo);
              }
              return -1;
           }
           token = strtok_r(test, " ", &test_r);
           while(token) {
               if (!strncmp(token, "-tc", 3)) {
                    token = strtok_r(NULL, " ", &test_r);
                     if (!token) {
                        printf("Testcase number missing");
                        break;
                     } else {
                        test_case = atoi(token);
                        printf("\n Test case %d", test_case);
                     }
               } else if (!strncmp(token, "-c", 2)) {
                    token = strtok_r(NULL, " ", &test_r);
                    if (!token) {
                        printf("Config file missing");
                        break;
                    } else {
                        if (fp) close(fp);
                        strlcpy(config_file, token, sizeof(config_file));
                        if ((fp = fopen(config_file, "rb"))) {
                            printf("\n Config file found %s", config_file);
                            config_found = 1;
                        } else
                            config_found = 0;
                    }
               } else if (!strncmp(token, "-file", 2)) {
                    token = strtok_r(NULL, " ", &test_r);
                    if (!token) {
                        printf("File name is missing");
                        break;
                    } else {
                        FILE *playfile = NULL;
                        strlcpy(filename, token, sizeof(filename));
                        if ((playfile = fopen(filename, "rb"))) {
                            printf("\n Config file found %s", config_file);
                            pfilename = filename;
                            fclose(playfile);
                        } else
                            pfilename = NULL;
                    }
               } else if (!strncmp(token, "-v", 2)) {
                    token = strtok_r(NULL, " ", &test_r);
                     if (!token) {
                        printf("Volume values are missing");
                        break;
                     } else {
                        volume = atoi(token);
                        if (volume < 0 && volume > 100)
                            volume = 50;
                        printf("\n Volume level %d", volume);
                     }
               } else if (!strncmp(token, "-d", 2)) {
                    token = strtok_r(NULL, " ", &test_r);
                     if (!token) {
                        printf("duration values are missing");
                        break;
                     } else {
                        duration = atoi(token);
                        printf("\n duration in secs %d", duration);
                     }
               } else if (!strncmp(token, "-fl", 3)) {
                    token = strtok_r(NULL, " ", &test_r);
                     if (!token) {
                        printf("frequency low values are missing");
                        break;
                     } else {
                        fl = atoi(token);
                        printf("\n frequency low %d", fl);
                     }
               } else if (!strncmp(token, "-fh", 3)) {
                    token = strtok_r(NULL, " ", &test_r);
                     if (!token) {
                        printf("frequency high values are missing");
                        break;
                     } else {
                        fh = atoi(token);
                        printf("\n frequency high %d", fh);
                     }
               }
               token = strtok_r(NULL, " ", &test_r);
           }
           if (test_case > 0 && config_found) {
             printf("tc number is %d config file name is %s",
                    test_case, config_file);
             printf("\n Execute test case");
             result = execute_test_case(test_case, codec, fp, volume,
             fl, fh, duration, pfilename);
             fseek(fp, 0, SEEK_SET);
             printf("\n Test case done");
           }
           test[0] = '\0';
        }
     /* Command line mode */
    }else if(argc == 3) {
        if (codec == CODEC_CONFIG_SUPPORT) {
            printf("\n Codecs with FTM non-configuration support only");
            return -1;
        }
        if ((strncmp(argv[1], "-tc", 3) == 0))    {
            printf("argc = %d\n",argc);
            test_case = atoi(argv[2]);

            if ((test_case < 1)) {
                printf("error: invalid test case (%d)\n", test_case);
                return -1;
            }
            if ((test_case > total_test)) {
                printf("error: invalid test case (%d)\n", test_case);
                return -1;
            }
        } else {
            printf("\nCommand to enter: mm-audio-ftm -tc <tc number>");
            return -1;
        }
    /* Config file mode */
    } else if (argc >= 5) {
        count = argc;
        fp = NULL;
        int i = 1;
        test_case = -1;
        /* Support only for codecs with config.txt support */
        if (codec != CODEC_CONFIG_SUPPORT) {
            printf("Command to enter: mm-audio-ftm -tc <tc number>\n");
            return -1;
        }
        while((i + 1) < count) {
           if (!strncmp(argv[i], "-tc", 3)) {
                i++;
                test_case = atoi(argv[i++]);
                printf("\n Test case number %d", test_case);
           } else if (!strncmp(argv[i], "-c", 2)) {
                i++;
                printf("\n Conf file name %s", argv[i]);
                if (!(fp = fopen(argv[i++],"rb"))) {
                    printf("\n Failed to open file");
                    return -1;
                }
                g_config_test = 1;
           } else if (!strncmp(argv[i], "-v", 2)) {
                i++;
                volume = atoi(argv[i++]);
                if (volume < 0 && volume > 100)
                    volume = 50;
                printf("\n Volume level set to %d", volume);
           } else if (!strncmp(argv[i], "-d", 2)) {
                i++;
                duration = atoi(argv[i++]);
                printf("\n duration set to %d", duration);
           } else if (!strncmp(argv[i], "-fl", 3)) {
                i++;
                fl = atoi(argv[i++]);
                printf("\n  Frequency set to %d", fl);
           } else if (!strncmp(argv[i], "-fh", 3)) {
                i++;
                fh = atoi(argv[i++]);
                printf("\n  Frequency set to %d", fh);
           } else if (!strncmp(argv[i], "-file", 5)) {
                int len;
                i++;
                len = strnlen(argv[i], sizeof(filename));
                if (len > (sizeof(filename) -1 )) {
                    printf("\n Invalid file name");
                    return -1;
                } else {
                    strlcpy(filename,argv[i++], sizeof(filename));
                    pfilename = filename;
                    printf("\n File name is %s", filename);
                }
           } else {
                printf("\n Invalid param Quit");
                if (fp)
                close(fp);
                return -1;
           }
        }
    } else {
        if (codec == CODEC_CONFIG_SUPPORT)
            printf("mm-audio-ftm -tc <num>  -c <FilePath>\n");
        else
            printf("mm-audio-ftm -tc <tc number>\n");
        return -EINVAL;
    }
    if ((g_config_test && !fp) || test_case < 0) {
        printf("\n Invalid test case %d Invalid file %p", test_case, fp);
        if (fp) {
            close(fp);
        }
        return -1;
    }
    if (g_config_test && !pfilename && (fl > fh || fl < 0 || fh < 0)) {
        printf("\n Invalid fl %d Invalid fh %d", fl, fh);
        return -1;
    }
    result = execute_test_case(test_case, codec, fp, volume,
            fl, fh, duration, pfilename);
    g_config_test = 0;
    if (!result) {
        printf("test %d success\n", test_case);
    } else {
        printf("test %d failure (%d)\n", test_case, result);
    }
    if (fp) {
        fclose(fp);
    }
    return 0;
#endif



/*  Test program for debug ( Start ) */

//   test_ftm_audio_tone_play(2);

   //test_ftm_audio_tone_play(0);  /* handset */
   //test_ftm_audio_tone_play(3);
   //test_ftm_audio_tone_play(4);
//     test_ftm_audio_volume(10,50,90);  /* volume change to 30/40/50  */
    //test_ftm_pcm_record(0);
   //test_ftm_pcm_record(1);

//     test_ftm_audio_afe_loopback(0);    /* Mic1 -> handset */
//     test_ftm_audio_afe_loopback(1);  /* Mic2 -> headset left/right */
//     test_ftm_audio_afe_loopback(2);  /* Mic1 -> speaker phone L/R */

//     test_ftm_audio_adie_loopback(0);  /* handset */
//     test_ftm_audio_adie_loopback(1);  /* headset left */
//     test_ftm_audio_adie_loopback(2);  /* headset right */
//     test_ftm_audio_adie_loopback(3);  /* speaker phone left  */
//     test_ftm_audio_adie_loopback(4);  /* speaker phone right */

//    return 0;

/*  Test program for debug ( End ) */

  openlog("aud_ftm_log", LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "Start audio FTM\n");

  bInit_Success = Diag_LSM_Init(NULL);

  if(!bInit_Success)
  {
    printf("FTMDaemon: Diag_LSM_Init() failed.");
    return -1;
  }

  printf("FTMDaemon: Diag_LSM_Init succeeded. \n");

  DIAGPKT_DISPATCH_TABLE_REGISTER ( DIAG_SUBSYS_FTM ,
                                      audio_ftm_diag_func_table
                                    );
/*
  if( signal( SIGINT,ftm_audio_main_signal_handler) == SIG_ERR)
  {
      PERROR("Unable to set new SIGINT signal handler");
      exit(1);
  }
*/

  sem_init(&semaphore_cmd_complete,0, 1);
  sem_init(&semaphore_cmd_coming,0, 0);

#ifdef FTM_DEBUG
 printf("Initialised the BT FTM cmd queue handlers \n");
#endif

  do
  {
    /* We have the freedom to send the first request without wating
     * for a command complete
     */
     if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
       printf("get clock_gettime error");
     ts.tv_sec += 5;
     /*we wait for 5 secs for a command already queued for
     * transmision
     */
     sem_status = sem_timedwait(&semaphore_cmd_complete,&ts);
     if(sem_status == -1)
     {
       printf("Command TIME OUT\n");
       // Add log cmd...
     }

     printf("Waiting for new command\n");
     sem_wait(&semaphore_cmd_coming);
   }
   while(1);

#ifdef FTM_DEBUG
  printf("\nFTMDaemon Deinit the LSM\n");
#endif
  // Clean up before exiting
  Diag_LSM_DeInit();
  sem_destroy(&semaphore_cmd_complete);
  sem_destroy(&semaphore_cmd_coming);
  closelog();

  return 0;

}

void signal_handler(int sig)
{
    DALSYS_Log_Info("Signal %d Caught.\n", sig);

    if(SIGTERM == sig)
    {
        if (params.fp)
        {
            pthread_mutex_lock(&params.lock);
            params.enable = 0;
            parse(&params);
            pthread_mutex_unlock(&params.lock);
            DALSYS_Log_Info("Mixer disable sequence executed\n");
        }
        else
            DALSYS_Log_Info("\n fp is not valid.\n");
    }
}
