/* =======================================================================
                             WFDMMSourceApp.cpp
DESCRIPTION

Copyright (c) 2011 - 2014  Qualcomm Technologies, Inc. All Rights Reserved
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video_8960/wfd-source/wfd-app/src/WFDMMSourceApp.cpp

========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "WFDMMSourceComDef.h"
#include "MMMemory.h"
#include "MMTimer.h"
#include "MMThread.h"
#include "wdsm_mm_interface.h"
#include "MMDebugMsg.h"
extern boolean bFileDumpEnable;
extern unsigned int WFDbitrate;
extern unsigned int output_width;
extern unsigned int output_height;
extern unsigned int input_width;
extern unsigned int input_height;
extern unsigned int ngSleepTime;
extern unsigned int nInPackets;
extern unsigned int nOutPackets;
extern unsigned int nInFrameRate;

WFD_MM_HANDLE handle = NULL;
bool playcommandcomplete = false;
bool pausecommandcomplete = false;
static const unsigned int SOURCEAPP_THREAD_STACK_SIZE = 16384;

static void wfd_mm_stream_play_callback(WFD_MM_HANDLE handle, WFD_status_t status);
static void wfd_mm_stream_pause_callback(WFD_MM_HANDLE handle, WFD_status_t status);
static void wfd_mm_capability_change_callback(WFD_MM_HANDLE handle);



#ifdef FEEDBACK_FROM_SINK
static int WFDMMSourceThreadAppEntry( void* ptr );
int WFDMMSourceAppThread(WFD_MM_capability_t* ptr );
int FBSocket;
int FBCount= 0;
struct FBmessage
{
    uint32 generateIFrame;
    uint32 changeBitrate;
};
#endif

int main (int argc, char *argv[])
{
  OMX_ERRORTYPE result = OMX_ErrorNone;
  WFD_device_t wfd_device_type = WFD_DEVICE_SOURCE;
  WFD_AV_select_t av_select = WFD_STREAM_AV;
  OMX_U32 nSleep = 100000;

  WFD_MM_callbacks_t pCallBacks;

  nInPackets = 0;
  nOutPackets = 0;
  int nTCPFLag = 0;

  WFD_MM_capability_t *WFD_negotiated_capability = (WFD_MM_capability_t *)MM_Malloc(sizeof(WFD_MM_capability_t));
  if( WFD_negotiated_capability != NULL)
  {
    WFD_negotiated_capability->video_config.video_config.h264_codec = (WFD_h264_codec_config_t *)MM_Malloc(sizeof(WFD_h264_codec_config_t) * 1);
    //memset(WFD_negotiated_capability, 0, sizeof(WFD_MM_capability_t));
    WFD_negotiated_capability->video_method = WFD_VIDEO_H264;
    WFD_negotiated_capability->video_config.video_config.num_h264_profiles  = 1;
    //640 * 480, 30 fps
    if(WFD_negotiated_capability->video_config.video_config.h264_codec != NULL)
    {
        ((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->supported_cea_mode = 0;
        ((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->supported_hh_mode = 0;
        ((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->supported_vesa_mode = 0;
        ((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->h264_level = 2;
        ((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->h264_profile = OMX_VIDEO_AVCProfileBaseline;
    }

    if(atoi(argv[3]) == 1)
    {
      WFD_negotiated_capability->audio_method = WFD_AUDIO_AAC;
      WFD_negotiated_capability->audio_config.lpcm_codec.supported_modes_bitmap = 1;//48K, 2 channels
    }
    else
    {
    WFD_negotiated_capability->audio_method = WFD_AUDIO_LPCM;
    WFD_negotiated_capability->audio_config.lpcm_codec.supported_modes_bitmap = 2;//48K, 2 channels
    }
    for ( int i = 0; i< argc; i++)
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "WFDSource in %d argument is %s \n ",i,argv[i]  );
    if(argc == 10)
    {
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "WFDSource in 4 arguments %s %s",argv[1],argv[2]  );
       WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1= inet_addr(argv[1]);
       WFD_negotiated_capability->transport_capability_config.port1_id = (uint16)atoi(argv[2]);
     //  nSleep = atoi(argv[3]);

       if(atoi(argv[3]) == 1)
       {
         bFileDumpEnable = true;
       }
       else
           bFileDumpEnable = false;

       if(atoi(argv[4]) == 1)
       {
           output_width = 800;
           output_height = 480;
           WFDbitrate = 1536000;
       }
       else
       if(atoi(argv[4]) == 0)
       {
           output_width = 1280;
           output_height = 720;
           input_width = 720;
           input_height = 1280;
           WFDbitrate = 14000000;
       }
       else
       if(atoi(argv[4]) == 2)
       {
           output_width = 640;
           output_height = 480;
           WFDbitrate = 512*1024;
       }
       nSleep = atoi(argv[5]);

       ngSleepTime = atoi(argv[6]);

       nInFrameRate = atoi(argv[7]);

       WFDbitrate = atoi(argv[8]);
       nTCPFLag = atoi(argv[9]);

       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "WFDSource in 4 arguments %d %d",
                   WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1,
                    WFD_negotiated_capability->transport_capability_config.port1_id );
    }
    else if( argc == 4)
    {
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "WFDSource in 4 arguments %s %s",argv[1],argv[2]  );
       WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1= inet_addr(argv[1]);
       WFD_negotiated_capability->transport_capability_config.port1_id = (uint16)atoi(argv[2]);
     //  nSleep = atoi(argv[3]);

       if(atoi(argv[3]) == 1)
       {
         bFileDumpEnable = true;
       }
       else
           bFileDumpEnable = false;
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "WFDSource in 4 arguments %d %d",
                   WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1,
                    WFD_negotiated_capability->transport_capability_config.port1_id );
    }
    else if( argc == 3)
    {
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "WFDSource in 3 arguments %s %s",argv[1],argv[2]  );
       WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1= inet_addr(argv[1]);
       WFD_negotiated_capability->transport_capability_config.port1_id = (uint16)atoi(argv[2]);
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "WFDSource in 3 arguments %d %d",
                   WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1,
                    WFD_negotiated_capability->transport_capability_config.port1_id );
    }
    else
    {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDSource in default number arguments");
       WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1= inet_addr("10.44.23.4");
       WFD_negotiated_capability->transport_capability_config.port1_id = 1234;
    }

    if(WFD_negotiated_capability->video_config.video_config.h264_codec != NULL)
    {
      if(nTCPFLag)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "WFDSource nTcP is %d",nTCPFLag);
       ((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->supported_cea_mode = nTCPFLag;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDSource cea mode is %d",(((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->supported_cea_mode));
      }
      else
      {
        ((WFD_negotiated_capability->video_config.video_config.h264_codec)+ 0)->supported_cea_mode = -4;
      }
    }
    //Fill the call backs
    pCallBacks.idr_cb = NULL;
    pCallBacks.capability_cb = &wfd_mm_capability_change_callback;

    wfd_mm_create_session(&handle, wfd_device_type, WFD_negotiated_capability, &pCallBacks);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDSource in created");

#ifdef FEEDBACK_FROM_SINK
    OMX_ERRORTYPE result;
    MM_HANDLE  FeedBackThread;

    if ( ( 0 != MM_Thread_CreateEx( 152,
                                   0,
                                   WFDMMSourceThreadAppEntry,
                                   WFD_negotiated_capability,
                                   SOURCEAPP_THREAD_STACK_SIZE,
                                   "FeedBackThread", &FeedBackThread) ) )
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceApp thread creation failed");
      result = OMX_ErrorInsufficientResources;
    }
#endif
    wfd_mm_stream_play(handle, av_select, &wfd_mm_stream_play_callback);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "wfd_mm_stream_play issued");

    MM_Timer_Sleep(nSleep);
  //  if(playcommandcomplete)
    {
   //   wfd_mm_stream_pause(handle, av_select, &wfd_mm_stream_pause_callback);
    }
   // MM_Timer_Sleep(10000);
   // if(pausecommandcomplete)
    {
      wfd_mm_destroy_session(handle);
    }

#ifdef FEEDBACK_FROM_SINK
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceApp feedback count is  = %d",FBCount );
    if(FeedBackThread != NULL)
    MM_Thread_Release(FeedBackThread);
#endif
    MM_Timer_Sleep(1000);
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"num In RTPpackets = %d, numSentPackets = %d", nInPackets, nOutPackets);
    MM_Timer_Sleep(1000);

    if(WFD_negotiated_capability)
    {
        MM_Free(WFD_negotiated_capability);
        WFD_negotiated_capability = NULL;
    }
  }

}

#ifdef FEEDBACK_FROM_SINK

int WFDMMSourceThreadAppEntry( void* ptr )
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceApp::WFDMMSourceThreadEntry");
    WFD_MM_capability_t *WFD_negotiated_capability = (WFD_MM_capability_t *) ptr;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSourceApp %d",WFD_negotiated_capability->transport_capability_config.port1_id);
    FBSocket  = -1;
    FBSocket= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(FBSocket == -1)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceApp feedback socket creation failed");
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFDMMSourceApp socket created");
      WFDMMSourceAppThread(WFD_negotiated_capability);
    }

    return 0;
}


int WFDMMSourceAppThread(WFD_MM_capability_t* WFD_negotiated_capability )
{

    int ConnectResult=-1;
    struct sockaddr_in addr;
    int portNum= 4321;//(WFD_negotiated_capability->transport_capability_config.port1_id)+1;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_addr.s_addr =  WFD_negotiated_capability->peer_ip_addrs.ipv4_addr1;;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16)portNum);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, " WFDMMSourceApp socket port=  %d", portNum);
    ConnectResult = connect( FBSocket, (struct sockaddr*)&addr, sizeof(addr));
    if (ConnectResult < 0)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, " WFDMMSourceApp socket connect failed %d", ConnectResult);
    }
    else
    {
         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSourceApp socket connect success = %d", ConnectResult);
    }
    OMX_BOOL bFBworking = OMX_TRUE;
    while(bFBworking == OMX_TRUE)
    {
      FBmessage fbmsg;
      int flags = 0;
      flags |=  MSG_WAITALL;
      int nBytes = recv(FBSocket,&fbmsg, sizeof(FBmessage),flags);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceApp bytes = %d",nBytes );
      if(nBytes == sizeof(FBmessage) )
      {
         if(fbmsg.generateIFrame > 0)
         {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceApp sending i-frame  = %d FBcount = %d", fbmsg.generateIFrame, FBCount++);
            wfd_SendIframeNext(handle);
         }
         if(fbmsg.changeBitrate > 0)
         {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceApp changing bitrate = %d  FBcount = %d", fbmsg.changeBitrate, FBCount++);
            //WFDbitrate= (WFDbitrate/2);
            wfd_ChangeBitRate(handle,(WFDbitrate/2));
         }
         else
         {
            //As sink sends notification when drops are there, so this condition never occurs
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"WFDMMSourceApp zero no of Packet lost = %d", fbmsg.generateIFrame);
         }
      }
      else
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "WFDMMSourceApp error in recieving data from socket");
          bFBworking = OMX_FALSE;
      }
    }

    return 0;
}
#endif
static void wfd_mm_stream_play_callback(WFD_MM_HANDLE handle, WFD_status_t status)
{
   (void) handle;
   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "wfd_mm_stream_play callback");
   if(status == WFD_STATUS_SUCCESS )
   {
        playcommandcomplete = true;
   }

}

static void wfd_mm_stream_pause_callback(WFD_MM_HANDLE handle, WFD_status_t status)
{
  (void) handle;
  if(status == WFD_STATUS_SUCCESS )
  {
    pausecommandcomplete = true;
  }
}

static void wfd_mm_capability_change_callback(WFD_MM_HANDLE handle)
{
   (void) handle;
}

