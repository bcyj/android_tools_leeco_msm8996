/*==========================================================================

                     FM Platform specfic File

Description
  Platform specific routines to program the V4L2 driver for FM

# Copyright (c) 2011, 2013 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/25/11   rakeshk  Created a source file to implement platform specific
                    routines for FM
===========================================================================*/

#include "fmhalapis.h"
#include <media/tavarua.h>
#include <linux/videodev2.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>

#ifdef ANDROID
#include <cutils/properties.h>
#endif

/* Multiplying factor to convert to Radio freqeuncy */
#define TUNE_MULT 16000
/* Constant to request for Radio Events */
#define EVENT_LISTEN 1
/* 1000 multiplier */
#define MULTIPLE_1000 1000
/* Tavaura I2C address */
int SLAVE_ADDR = 0x2A;
/* Tavaura I2C statu register*/
#define INTSTAT_0 0x0
#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(*a))
/* Debug Macro */
#define FM_DEBUG
#ifdef FM_DEBUG
#define print(x) /*printf(x)*/
#define print2(x,y) printf(x,y)
#define print3(x,y,z) printf(x,y,z)
#else
#define print(x)
#define print2(x,y)
#define print3(x,y,z)
#endif

static char *qsoc_poweron_path = NULL;
static char *qsoc_poweroff_path = NULL;

/* enum to montior the Power On status */
typedef enum
{
  INPROGRESS,
  COMPLETE
}poweron_status;

boolean cmd_queued = FALSE;
/* Resourcse Numbers for Rx/TX */
int FM_RX = 1;
int FM_TX = 2;
/* Boolean to control the power down sequence */
volatile boolean power_down = FALSE;
/* V4L2 radio handle */
int fd_radio = -1;
/* FM asynchornous thread to perform the long running ON */
pthread_t fm_interrupt_thread,fm_on_thread;
/* Prototype ofFM ON thread */
fm_cmd_status_type (ftm_on_long_thread)(void *ptr);
/* Global state ofthe FM task */
fm_station_params_available fm_global_params;

volatile poweron_status poweron;

/*===========================================================================
FUNCTION  set_v4l2_ctrl

DESCRIPTION
  Sets the V4L2 control sent as argument with the requested value and returns the status

DEPENDENCIES
  NIL

RETURN VALUE
  FALSE in failure,TRUE in success

SIDE EFFECTS
  None

===========================================================================*/
boolean set_v4l2_ctrl(int fd,uint32 id,int32 value)
{
  struct v4l2_control control;
  int err;
  control.value = value;
  control.id = id;

  err = ioctl(fd,VIDIOC_S_CTRL,&control);
  if(err < 0)
  {
    print2("set_v4l2_ctrl failed for control = 0x%x\n",control.id);
    return FALSE;
  }
  return TRUE;
}

/*===========================================================================
FUNCTION  read_data_from_v4l2

DESCRIPTION
  reads the fm_radio handle and updates the FM global configuration based on
  the interrupt data received

DEPENDENCIES
  NIL

RETURN VALUE
  FALSE in failure,TRUE in success

SIDE EFFECTS
  None

===========================================================================*/
int read_data_from_v4l2(int fd,uint8* buf,int index)
{
  struct v4l2_requestbuffers reqbuf;
  struct v4l2_buffer v4l2_buf;
  int err;
  memset(&reqbuf, 0x0, sizeof(reqbuf));
  enum v4l2_buf_type type = V4L2_BUF_TYPE_PRIVATE;

  reqbuf.type = V4L2_BUF_TYPE_PRIVATE;
  reqbuf.memory = V4L2_MEMORY_USERPTR;
  memset(&v4l2_buf, 0x0, sizeof(v4l2_buf));
  v4l2_buf.index = index;
  v4l2_buf.type = type;
  v4l2_buf.length = 128;
  v4l2_buf.m.userptr = (unsigned long)buf;
  err = ioctl(fd,VIDIOC_DQBUF,&v4l2_buf) ;
  if(err < 0)
  {
    print2("ioctl failed with error = %d\n",err);
    return -1;
  }
  return v4l2_buf.bytesused;
}

/*===========================================================================
FUNCTION  extract_program_service

DESCRIPTION
  Helper routine to read the Program Services data from the V4L2 buffer
  following a PS event

DEPENDENCIES
  PS event

RETURN VALUE
  TRUE if success,else FALSE

SIDE EFFECTS
  Updates the Global data strutures PS info entry

===========================================================================*/
boolean extract_program_service()
{
  uint8 buf[64] = {0};
  int ret;
  ret = read_data_from_v4l2(fd_radio,buf,TAVARUA_BUF_PS_RDS);
  if(ret < 0) {
     return TRUE;
  }
  int num_of_ps = (int)(buf[0] & 0x0F);
  int ps_services_len = ((int )((num_of_ps*8) + 5)) - 5;

  fm_global_params.pgm_id = (((buf[2] & 0xFF) << 8) | (buf[3] & 0xFF));
  fm_global_params.pgm_type = (int)( buf[1] & 0x1F);
  memset(fm_global_params.pgm_services,0x0,96);
  memcpy(fm_global_params.pgm_services,&buf[5],ps_services_len);
  fm_global_params.pgm_services[ps_services_len] = '\0';
  print2("Pid = %d\n",fm_global_params.pgm_id);
  print2("Ptype = %d\n",fm_global_params.pgm_type);
  print2("PS name %s\n",fm_global_params.pgm_services);
  return TRUE;
}
/*===========================================================================
FUNCTION  extract_radio_text

DESCRIPTION
  Helper routine to read the Radio text data from the V4L2 buffer
  following a RT event

DEPENDENCIES
  RT event

RETURN VALUE
  TRUE if success,else FALSE

SIDE EFFECTS
  Updates the Global data strutures RT info entry

===========================================================================*/

boolean extract_radio_text()
{
  uint8 buf[120] = {0};

  int bytesread = read_data_from_v4l2(fd_radio,buf,TAVARUA_BUF_PS_RDS);
  if(bytesread < 0) {
     return TRUE;
  }
  int radiotext_size = (int)(buf[0] & 0x0F);
  fm_global_params.pgm_id = (((buf[2] & 0xFF) << 8) | (buf[3] & 0xFF));
  fm_global_params.pgm_type = (int)( buf[1] & 0x1F);
  memset(fm_global_params.radio_text,0x0,64);
  memcpy(fm_global_params.radio_text,&buf[5],radiotext_size);
  fm_global_params.radio_text[radiotext_size] = '\0';
  printf("RT : %s\n",fm_global_params.radio_text);
  return TRUE;
}

/*===========================================================================
FUNCTION  stationList

DESCRIPTION
  Helper routine to extract the list of good stations from the V4L2 buffer

DEPENDENCIES

RETURN VALUE
  NONE, If list is non empty then it prints the stations available

SIDE EFFECTS

===========================================================================*/

void stationList (int fd)
{
  int freq = 0;
  int i=0;
  int station_num;
  float real_freq = 0;
  int *stationList;
  uint8 sList[100] = {0};
  int tmpFreqByte1=0;
  int tmpFreqByte2=0;
  float lowBand;
  struct v4l2_tuner tuner;

  tuner.index = 0;
  ioctl(fd, VIDIOC_G_TUNER, &tuner);
  lowBand = (float) (((tuner.rangelow * 1000)/ TUNE_MULT) / 1000.00);

  printf("lowBand %f\n",lowBand);
  if(read_data_from_v4l2(fd, sList, 0) < 0) {
     printf("Data read from v4l2 failed\n");
     return;
  }

  station_num = (int)sList[0];
  stationList = malloc(sizeof(int)*(station_num+1));
  printf("station_num: %d\n",station_num);
  if(stationList == NULL) {
    printf("stationList: failed to allocate memory\n");
    return;
  }

  for (i=0;i<station_num;i++) {
    freq = 0;
    tmpFreqByte1 = sList[i*2+1] & 0xFF;
    tmpFreqByte2 = sList[i*2+2] & 0xFF;
    freq = (tmpFreqByte1 & 0x03) << 8;
    freq |= tmpFreqByte2;
    printf(" freq: %d\n",freq);
    real_freq  = (float)(freq * 0.05) + lowBand;
    printf(" real_freq: %f\n",real_freq);
    stationList[i] = (int)(real_freq*1000);
    printf(" stationList: %d\n",stationList[i]);
  }
  free(stationList);

}

/*===========================================================================
FUNCTION  process_radio_event

DESCRIPTION
  Helper routine to process the radio event read from the V4L2 and performs
  the corresponding action.

DEPENDENCIES
  Radio event

RETURN VALUE
  TRUE if success,else FALSE

SIDE EFFECTS
  Updates the Global data strutures info entry like frequency, station
  available, RDS sync status etc.

===========================================================================*/

boolean process_radio_event(uint8 event_buf)
{
  struct v4l2_frequency freq;
  boolean ret= TRUE;
  switch(event_buf)
  {
    case TAVARUA_EVT_RADIO_READY:
      printf("FM enabled\n");
      break;
    case TAVARUA_EVT_RADIO_DISABLED:
      printf("FM disabled\n");
      close(fd_radio);
      fd_radio = -1;
      pthread_exit(NULL);
      ret = FALSE;
      break;
    case TAVARUA_EVT_TUNE_SUCC:
      freq.type = V4L2_TUNER_RADIO;
      if(ioctl(fd_radio, VIDIOC_G_FREQUENCY, &freq)< 0)
      {
        return FALSE;
      }
      fm_global_params.current_station_freq =((freq.frequency*MULTIPLE_1000)/TUNE_MULT);
      printf("tuned freq: %d\n", fm_global_params.current_station_freq);
      break;
    case TAVARUA_EVT_SEEK_COMPLETE:
      print("Seek Complete\n");
      freq.type = V4L2_TUNER_RADIO;
      if(ioctl(fd_radio, VIDIOC_G_FREQUENCY, &freq)< 0)
      {
        return FALSE;
      }
      fm_global_params.current_station_freq =((freq.frequency*MULTIPLE_1000)/TUNE_MULT);
      printf("Seeked Frequency : %d\n",fm_global_params.current_station_freq);
      break;
    case TAVARUA_EVT_SCAN_NEXT:
      print("Event Scan next\n");
      freq.type = V4L2_TUNER_RADIO;
      if(ioctl(fd_radio, VIDIOC_G_FREQUENCY, &freq)< 0)
      {
        return FALSE;
      }
      printf("Scanned Frequency : %d\n",((freq.frequency*MULTIPLE_1000)/TUNE_MULT));
      break;
    case TAVARUA_EVT_NEW_RAW_RDS:
      print("Received Raw RDS info\n");
      break;
    case TAVARUA_EVT_NEW_RT_RDS:
      print("Received RT \n");
      ret = extract_radio_text();
      break;
    case TAVARUA_EVT_NEW_PS_RDS:
      print("Received PS\n");
      ret = extract_program_service();
      break;
    case TAVARUA_EVT_ERROR:
      print("Received Error\n");
      break;
    case TAVARUA_EVT_BELOW_TH:
      fm_global_params.service_available = FM_SERVICE_NOT_AVAILABLE;
      break;
    case TAVARUA_EVT_ABOVE_TH:
      fm_global_params.service_available = FM_SERVICE_AVAILABLE;
      break;
    case TAVARUA_EVT_STEREO:
      print("Received Stereo Mode\n");
      fm_global_params.stype = FM_RX_STEREO;
      break;
    case TAVARUA_EVT_MONO:
      print("Received Mono Mode\n");
      fm_global_params.stype = FM_RX_MONO;
      break;
    case TAVARUA_EVT_RDS_AVAIL:
      print("Received RDS Available\n");
      fm_global_params.rds_sync_status = FM_RDS_SYNCED;
      break;
    case TAVARUA_EVT_RDS_NOT_AVAIL:
      fm_global_params.rds_sync_status = FM_RDS_NOT_SYNCED;
      break;
    case TAVARUA_EVT_NEW_SRCH_LIST:
      print("Received new search list\n");
      stationList(fd_radio);
      break;
    case TAVARUA_EVT_NEW_AF_LIST:
      print("Received new AF List\n");
      break;

  }
  /*
   * This logic is applied to ensure the exit ofthe Event read thread
   * before the FM Radio control is turned off. This is a temporary fix
   */
  return ret;
}

/*===========================================================================
FUNCTION  interrupt_thread

DESCRIPTION
  Thread to perform a continous read on the radio handle for events

DEPENDENCIES
  NIL

RETURN VALUE
  NIL

SIDE EFFECTS
  None

===========================================================================*/

void * interrupt_thread(void *ptr)
{
  printf("Starting FM event listener\n");
  uint8 buf[128] = {0};
  boolean status = TRUE;
  int i =0;
  int bytesread = 0;
  while(1)
  {
    bytesread = read_data_from_v4l2(fd_radio,buf,EVENT_LISTEN);
    if(bytesread < 0)
      break;
    for(i =0;i<bytesread;i++)
    {
      status = process_radio_event(buf[i]);
      if(status != TRUE)
        return NULL;
    }
  }
  print("FM listener thread exited\n");
  return NULL;
}


/*===========================================================================
FUNCTION  EnableReceiver

DESCRIPTION
  PFAL specific routine to enable the FM receiver with the Radio Cfg
  parameters passed.

PLATFORM SPECIFIC DESCRIPTION
  Opens the handle to /dev/radio0 V4L2 device and intiates a Soc Patch
  download, configurs the Init parameters like Band Limit,RDS type,
  Frequency Band, and Radio State.

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type EnableReceiver
(
  fm_config_data*      radiocfgptr
)
{
  int err;
  int retval;
  char versionStr[40] = {'\0'};
  struct v4l2_capability cap;

#ifdef FM_DEBUG
  print("\nEnable Receiver entry\n");
#endif

  fd_radio = open("/dev/radio0",O_RDONLY, O_NONBLOCK);
  if(fd_radio < 0)
  {
    print2("EnableReceiver Failed to open = %d\n",fd_radio);
    return FM_CMD_FAILURE;
  }

  //Read the driver verions
  err = ioctl(fd_radio, VIDIOC_QUERYCAP, &cap);

  printf("VIDIOC_QUERYCAP returns :%d: version: %d \n", err , cap.version );

  if( err >= 0 )
  {
    printf("Driver Version(Same as ChipId): %x \n",  cap.version );
    /*Conver the integer to string */
    retval = snprintf(versionStr, sizeof(versionStr), "%d", cap.version);
    if ((retval >= sizeof(versionStr))) {
        close(fd_radio);
        fd_radio = -1;
        return FM_CMD_FAILURE;
    }
    property_set("hw.fm.version", versionStr);
    asprintf(&qsoc_poweron_path, "fm_qsoc_patches %d 0", cap.version);
    if(qsoc_poweron_path != NULL)
       printf("qsoc_onpath = %s\n",qsoc_poweron_path);
  }
  else
  {
    return FM_CMD_FAILURE;
  }


  print("\nOpened Receiver\n");
  return ftm_on_long_thread(radiocfgptr);
}

/*===========================================================================
FUNCTION  ftm_on_long_thread

DESCRIPTION
  Helper routine to perform the rest ofthe FM calibration and SoC Patch
  download and configuration settings following the opening ofradio handle

DEPENDENCIES
  NIL

RETURN VALUE
  NIL

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type (ftm_on_long_thread)(void *ptr)
{
  int ret = 0;
  struct v4l2_control control;
  struct v4l2_tuner tuner;
  int i, init_success = 0;
  char value[PROPERTY_VALUE_MAX] = {0};
  char transport[PROPERTY_VALUE_MAX] = {0};
  char soc_type[PROPERTY_VALUE_MAX] = {0};

  fm_config_data*      radiocfgptr = (fm_config_data *)ptr;
  property_get("ro.qualcomm.bt.hci_transport", transport, NULL);
  property_get("qcom.bluetooth.soc", soc_type, NULL);
  if( (3 == strlen(transport)) && ( !strncmp("smd", transport, 3)) )
  {
     property_set("hw.fm.mode", "normal");
     property_set("ctl.start", "fm_dl");
     sleep(1);
     for(i = 0; i < 9; i++)
     {
       property_get("hw.fm.init", value, NULL);
       if(strcmp(value, "1") == 0)
       {
         init_success = 1;
         break;
       }
       else
       {
         sleep(1);
       }
     }
     print3("init_success:%d after %d seconds \n", init_success, i);
     if(!init_success)
     {
       property_set("ctl.stop", "fm_dl");
       // close the fd(power down)
       close(fd_radio);
       fd_radio = -1;
       return FM_CMD_FAILURE;
     }
  }
  else if (strcmp(soc_type, "rome") != 0)
  {
     ret = system(qsoc_poweron_path);
     if(ret !=  0)
     {
         print2("EnableReceiver Failed to download patches = %d\n",ret);
         return FM_CMD_FAILURE;
     }
  }
  /**
   * V4L2_CID_PRIVATE_TAVARUA_STATE
   * V4L2_CID_PRIVATE_TAVARUA_EMPHASIS
   * V4L2_CID_PRIVATE_TAVARUA_SPACING
   * V4L2_CID_PRIVATE_TAVARUA_RDS_STD
   * V4L2_CID_PRIVATE_TAVARUA_REGION
   */

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_STATE,FM_RX);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set Radio State = %d\n",ret);
    close(fd_radio);
    fd_radio = -1;
    return FM_CMD_FAILURE;
  }
#ifdef FM_DEBUG
  print2("emphasis	:%d\n",radiocfgptr->emphasis);
#endif
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_EMPHASIS,
        radiocfgptr->emphasis);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set Emphasis = %d\n",ret);
    return FM_CMD_FAILURE;
  }

#ifdef FM_DEBUG
  print2("spacing	:%d\n",radiocfgptr->spacing);
#endif
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SPACING,
        radiocfgptr->spacing);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set channel spacing = %d\n",ret);
    return FM_CMD_FAILURE;
  }

#ifdef FM_DEBUG
  print2("RDS system	:%d\n",radiocfgptr->rds_system);
#endif
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDS_STD,
        radiocfgptr->rds_system);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set RDS std = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  tuner.index = 0;
  tuner.signal = 0;
  tuner.rangelow = radiocfgptr->bandlimits.lower_limit * (TUNE_MULT/1000);
  tuner.rangehigh = radiocfgptr->bandlimits.upper_limit * (TUNE_MULT/1000);
  ret = ioctl(fd_radio,VIDIOC_S_TUNER,&tuner);
  if(ret < 0)
  {
    print2("EnableReceiver Failed to set Band Limits  = %d\n",ret);
    return FM_CMD_FAILURE;
  }
#ifdef FM_DEBUG
  print2("Band		:%d\n",radiocfgptr->band);
#endif
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_REGION,
        radiocfgptr->band);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set Band = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDSON,1);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set RDS on = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  control.id = V4L2_CID_PRIVATE_TAVARUA_RDSGROUP_PROC;
  ret = ioctl(fd_radio,VIDIOC_G_CTRL,&control);
  if(ret < 0)
  {
    print2("EnableReceiver Failed to set RDS group  = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  int rdsMask = 23;
  uint8  rds_group_mask = (uint8)control.value;
  uint8  rdsFilt = 0;
  int  psAllVal=rdsMask & (1 << 4);

  print2("RdsOptions	:%x\n",rdsMask);
  rds_group_mask &= 0xC7;

  rds_group_mask  |= ((rdsMask & 0x07) << 3);

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDSGROUP_PROC,
        rds_group_mask);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set RDS on = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  if (strcmp(soc_type, "rome") == 0)
  {
      ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDSGROUP_MASK, 1);
      if (ret == FALSE)
      {
          print("Failed to set RDS GRP MASK!!!\n");
          return FM_CMD_FAILURE;
      }
      ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDSD_BUF, 1);
      if (ret == FALSE)
      {
          print("Failed to set RDS BUF!!!\n");
          return FM_CMD_FAILURE;
      }
  } else {
      ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_PSALL,
                    (psAllVal >> 4));
      if (ret == FALSE)
      {
          print2("EnableReceiver Failed to set RDS on = %d\n",ret);
          return FM_CMD_FAILURE;
      }
  }
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_ANTENNA,0);
  if(ret == FALSE)
  {
    print2("EnableReceiver Failed to set RDS on = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  pthread_create( &fm_interrupt_thread, NULL, interrupt_thread, NULL);
#ifdef FM_DEBUG
  print("\nEnable Receiver exit\n");
#endif
  poweron = COMPLETE;
  return FM_CMD_SUCCESS;

}
/*===========================================================================
FUNCTION  DisableReceiver

DESCRIPTION
  PFAL specific routine to disable the FM receiver and free the FM resources

PLATFORM SPECIFIC DESCRIPTION
  Closes the handle to /dev/radio0 V4L2 device

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type DisableReceiver
(
)
{
  struct v4l2_control control;
  uint8 buf[128];
  double tune;
  struct v4l2_frequency freq_struct;
  int ret;
  struct v4l2_capability cap;

  /** Wait till the previous ON sequence has completed */
  if (poweron != COMPLETE) {
      printf("FM is already disabled\n");
      return FM_CMD_FAILURE;
  }
#ifdef FM_DEBUG
  print("Disable start\n");
#endif

  printf("Shutting down FM\n");
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_STATE,0);
  if(ret == FALSE)
  {
    print2("DisableReceiver Failed to set FM OFF = %d\n",ret);
    return FM_CMD_FAILURE;
  }

#ifdef FM_DEBUG
  print("Disabled Receiver\n");
#endif
 return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  ConfigureReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver with the Radio Cfg
  parameters passed.

PLATFORM SPECIFIC DESCRIPTION
  configurs the Init parameters like Band Limit,RDS type,
  Frequency Band, and Radio State.

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type ConfigureReceiver
(
 fm_config_data*      radiocfgptr
)
{
  int ret = 0;
  struct v4l2_control control;
  struct v4l2_tuner tuner;
#ifdef FM_DEBUG
  print("\nConfigure Receiver entry\n");
#endif

  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_EMPHASIS,
        radiocfgptr->emphasis);
  if(ret == FALSE)
  {
    print2("Configure  Failed to set Emphasis = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SPACING,
        radiocfgptr->spacing);
  if(ret == FALSE)
  {
    print2("Configure  Failed to set channel spacing  = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDS_STD,
        radiocfgptr->rds_system);
  if(ret == FALSE)
  {
    print2("Configure  Failed to set RDS std = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  tuner.index = 0;
  tuner.signal = 0;
  tuner.rangelow = radiocfgptr->bandlimits.lower_limit * (TUNE_MULT/1000);
  tuner.rangehigh = radiocfgptr->bandlimits.upper_limit * (TUNE_MULT/1000);
  ret = ioctl(fd_radio,VIDIOC_S_TUNER,&tuner);
  if(ret < 0)
  {
    print2("Configure  Failed to set Band Limits  = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_REGION,radiocfgptr->band);
  if(ret == FALSE)
  {
    print2("Configure  Failed to set Band  = %d\n",ret);
    return FM_CMD_FAILURE;
  }

out :
#ifdef FM_DEBUG
  print("\nConfigure Receiver exit\n");
#endif

  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  SetFrequencyReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's Frequency ofreception

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetFrequencyReceiver
(
 uint32 ulfreq
)
{
  int err;
  double tune;
  struct v4l2_frequency freq_struct;
#ifdef FM_DEBUG
  print2("\nSetFrequency Receiver entry freq = %d\n",(int)ulfreq);
#endif

  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;
  freq_struct.type = V4L2_TUNER_RADIO;
  freq_struct.frequency = (ulfreq*TUNE_MULT/1000);
  err = ioctl(fd_radio, VIDIOC_S_FREQUENCY, &freq_struct);
  if(err < 0)
  {
    return FM_CMD_FAILURE;
  }
#ifdef FM_DEBUG
  print("\nSetFrequency Receiver exit\n");
#endif
  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  SetMuteModeReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's mute status

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetMuteModeReceiver
(
 mute_type mutemode
)
{
  int err,i;
  struct v4l2_control control;
  print2("SetMuteModeReceiver mode = %d\n",mutemode);
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  control.value = mutemode;
  control.id = V4L2_CID_AUDIO_MUTE;

  for(i=0;i<3;i++)
  {
    err = ioctl(fd_radio,VIDIOC_S_CTRL,&control);
    if(err >= 0)
    {
      print("SetMuteMode Success\n");
      return FM_CMD_SUCCESS;
    }
  }
  print2("Set mute mode ret = %d\n",err);
  return FM_CMD_FAILURE;
}
/*===========================================================================
FUNCTION  SetStereoModeReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's Audio mode on the
  frequency tuned

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetStereoModeReceiver
(
 stereo_type stereomode
)
{
  struct v4l2_tuner tuner;
  int err;
  print2("SetStereoModeReceiver stereomode = %d \n",stereomode);
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  tuner.index = 0;
  err = ioctl(fd_radio, VIDIOC_G_TUNER, &tuner);
  print3("Get stereo mode ret = %d tuner.audmode = %d\n",err,tuner.audmode);
  if(err < 0)
    return FM_CMD_FAILURE;

  tuner.audmode = stereomode;
  err = ioctl(fd_radio, VIDIOC_S_TUNER, &tuner);
  print2("Set stereo mode ret = %d\n",err);
  if(err < 0)
    return FM_CMD_FAILURE;
  print("SetStereoMode Success\n");
  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  GetStationParametersReceiver

DESCRIPTION
  PFAL specific routine to get the station parameters ofthe Frequency at
  which the Radio receiver is  tuned

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type GetStationParametersReceiver
(
 fm_station_params_available*    configparams
)
{
  int i;
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;
  configparams->current_station_freq = fm_global_params.current_station_freq;
  configparams->service_available = fm_global_params.service_available;

  struct v4l2_tuner tuner;
  tuner.index = 0;
  tuner.signal = 0;
  if(ioctl(fd_radio, VIDIOC_G_TUNER, &tuner) < 0)
    return FM_CMD_FAILURE;

  configparams->rssi = tuner.signal;
  configparams->stype = fm_global_params.stype;
  configparams->rds_sync_status = fm_global_params.rds_sync_status;

  struct v4l2_control control;
  control.id = V4L2_CID_AUDIO_MUTE;

  for(i=0;i<3;i++)
  {
    int err = ioctl(fd_radio,VIDIOC_G_CTRL,&control);
    if(err >= 0)
    {
      configparams->mute_status = control.value;
      return FM_CMD_SUCCESS;
    }
  }

  return FM_CMD_FAILURE;
}
/*===========================================================================
FUNCTION  SetRdsOptionsReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's RDS options

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetRdsOptionsReceiver
(
 fm_rds_options rdsoptions
)
{
  int ret;
  print("SetRdsOptionsReceiver\n");

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDSGROUP_MASK,
        rdsoptions.rds_group_mask);
  if(ret == FALSE)
  {
    print2("SetRdsOptionsReceiver Failed to set RDS group options = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDSD_BUF,
        rdsoptions.rds_group_buffer_size);
  if(ret == FALSE)
  {
    print2("SetRdsOptionsReceiver Failed to set RDS group options = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  /*Chnage Filter not supported */
  print("SetRdsOptionsReceiver<\n");

  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  SetRdsGroupProcReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's RDS group proc options

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetRdsGroupProcReceiver
(
 uint32 rdsgroupoptions
)
{
  int ret;
  print("SetRdsGroupProcReceiver\n");
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_RDSGROUP_PROC,
        rdsgroupoptions);
  if(ret == FALSE)
  {
    print2("SetRdsGroupProcReceiver Failed to set RDS proc = %d\n",ret);
    return FM_CMD_FAILURE;
  }

  print("SetRdsGroupProcReceiver<\n");
  return FM_CMD_SUCCESS;
}


/*===========================================================================
FUNCTION  SetPowerModeReceiver

DESCRIPTION
  PFAL specific routine to configure the power mode of FM receiver

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetPowerModeReceiver
(
 uint8 powermode
)
{
  struct v4l2_control control;
  int i,err;
  print2("SetPowerModeReceiver mode = %d\n",powermode);
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  control.value = powermode;
  control.id = V4L2_CID_PRIVATE_TAVARUA_LP_MODE;

  for(i=0;i<3;i++)
  {
    err = ioctl(fd_radio,VIDIOC_S_CTRL,&control);
    if(err >= 0)
    {
      print("SetPowerMode Success\n");
      return FM_CMD_SUCCESS;
    }
  }
  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  SetSignalThresholdReceiver

DESCRIPTION
  PFAL specific routine to configure the signal threshold ofFM receiver

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetSignalThresholdReceiver
(
 uint8 signalthreshold
)
{
  struct v4l2_control control;
  int i,err;
  print2("SetSignalThresholdReceiver threshold = %d\n",signalthreshold);
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  control.value = signalthreshold;
  control.id = V4L2_CID_PRIVATE_TAVARUA_SIGNAL_TH;

  for(i=0;i<3;i++)
  {
    err = ioctl(fd_radio,VIDIOC_S_CTRL,&control);
    if(err >= 0)
    {
      print("SetSignalThresholdReceiver Success\n");
      return FM_CMD_SUCCESS;
    }
  }
  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  SearchStationsReceiver

DESCRIPTION
  PFAL specific routine to search for stations from the current frequency of
  FM receiver and print the information on diag

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SearchStationsReceiver
(
fm_search_stations searchstationsoptions
)
{
  int err,i;
  struct v4l2_control control;
  struct v4l2_hw_freq_seek hwseek;
  boolean ret;

  hwseek.type = V4L2_TUNER_RADIO;
  print("SearchStationsReceiver\n");
  hwseek.type = V4L2_TUNER_RADIO;
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCHMODE,
        searchstationsoptions.search_mode);
  if(ret == FALSE)
  {
    print("SearchStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SCANDWELL,
        searchstationsoptions.dwell_period);
  if(ret == FALSE)
  {
    print("SearchStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  hwseek.seek_upward = searchstationsoptions.search_dir;
  err = ioctl(fd_radio,VIDIOC_S_HW_FREQ_SEEK,&hwseek);

  if(err < 0)
  {
    print("SearchStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }
  print("SearchRdsStationsReceiver<\n");
  return FM_CMD_SUCCESS;
}


/*===========================================================================
FUNCTION  SearchRDSStationsReceiver

DESCRIPTION
  PFAL specific routine to search for stations from the current frequency of
  FM receiver with a specific program type and print the information on diag

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SearchRdsStationsReceiver
(
fm_search_rds_stations searchrdsstationsoptions
)
{
  int i,err;
  boolean ret;
  struct v4l2_control control;
  struct v4l2_hw_freq_seek hwseek;

  hwseek.type = V4L2_TUNER_RADIO;
  print("SearchRdsStationsReceiver>\n");
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCHMODE,
        searchrdsstationsoptions.search_mode);
  if(ret == FALSE)
  {
    print("SearchRdsStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SCANDWELL,
        searchrdsstationsoptions.dwell_period);
  if(ret == FALSE)
  {
    print("SearchRdsStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCH_PTY,
        searchrdsstationsoptions.program_type);
  if(ret == FALSE)
  {
    print("SearchRdsStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCH_PI,
        searchrdsstationsoptions.program_id);
  if(ret == FALSE)
  {
    print("SearchRdsStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  hwseek.seek_upward = searchrdsstationsoptions.search_dir;
  err = ioctl(fd_radio,VIDIOC_S_HW_FREQ_SEEK,&hwseek);

  if(err < 0)
  {
    print("SearchRdsStationsReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  print("SearchRdsStationsReceiver<\n");
  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION  SearchStationListReceiver

DESCRIPTION
  PFAL specific routine to search for stations with a specific mode of
  informaation like WEAK,STRONG,STRONGEST etc

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SearchStationListReceiver
(
fm_search_list_stations searchliststationsoptions
)
{
  int i,err;
  boolean ret;
  struct v4l2_control control;
  struct v4l2_hw_freq_seek hwseek;

  hwseek.type = V4L2_TUNER_RADIO;
  print("SearchStationListReceiver>\n");
  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCHMODE,
        searchliststationsoptions.search_mode);
  if(ret == FALSE)
  {
    print("SearchStationListReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCH_CNT,
        searchliststationsoptions.srch_list_max);
  if(ret == FALSE)
  {
    print("SearchStationListReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCH_PTY,
        searchliststationsoptions.program_type);
  if(ret == FALSE)
  {
    print("SearchStationListReceiver failed \n");
    return FM_CMD_FAILURE;
  }

  hwseek.seek_upward = searchliststationsoptions.search_dir;
  err = ioctl(fd_radio,VIDIOC_S_HW_FREQ_SEEK,&hwseek);

  if(err < 0)
  {
    print("SearchStationListReceiver failed \n");
    return FM_CMD_FAILURE;
  }
  print("SearchStationListReceiver<\n");
  return FM_CMD_SUCCESS;
}

/*===========================================================================
FUNCTION

DESCRIPTION
  PFAL specific routine to cancel the ongoing search operation

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type CancelSearchReceiver
(
)
{
  struct v4l2_control control;
  boolean ret;

  if(fd_radio < 0)
    return FM_CMD_NO_RESOURCES;
  ret = set_v4l2_ctrl(fd_radio,V4L2_CID_PRIVATE_TAVARUA_SRCHON,0);
  if(ret == FALSE)
  {
    return FM_CMD_FAILURE;
  }
  return FM_CMD_SUCCESS;
}

