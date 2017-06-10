/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

/******************************************************************************
 * wifiLoggerApp.c
 *
 * This file implements the Netlink Proxy Server. It sends a logging request
 * to wlan driver and logs the messages received from driver onto the stdout
 * and/or to sdcard location based on the input options.
 *
 * Eranna Vinay Krishna, 05/23/2014
 *
 ******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

/* WNI Header */
#include <aniNlMsg.h>
#include <aniErrors.h>
#include <aniAsfHdr.h>
#include <aniAsfIpc.h>
#include <aniAsfLog.h>
#include <aniNlFuncs.h>

#include "wifiLogger.h"


/*
 * Globals
 */
tWLogCtxt serverCtxt;
static int sigInt;

static int exitLogging = 0;


int wifiLoggerAppSendMsgToKernel(tWLogCtxt *pserver, int radio,
        tAniHdr *msg, int msgType, int msgLen)
{
   tAniNlHdr *wnl;
   char *localBuf;

   if ((localBuf = malloc(MAX_DRV_MSG_SIZE)) == NULL)
   {
      aniAsfLogMsg(LOG_ERR, ANI_WHERE, "WifiLoggerApp: %s, malloc failed\n",
                    __FUNCTION__);
      return -1;
   }

   memset(localBuf, 0, MAX_DRV_MSG_SIZE);

   pserver->nl.nlmsg_len =
                     aniNlAlign(sizeof(tAniNlHdr)) + msgLen - sizeof(tAniHdr);
   pserver->nl.nlmsg_type = msgType; // send it to the HDD
   pserver->nl.nlmsg_seq++;

   // copy the netlink msg hdr first (assuming buf is 4 byte aligned)
   wnl = (tAniNlHdr *)localBuf;
   wnl->radio = radio;

   // setup the tAniHdr next
   wnl->wmsg.length = ntohs(msgLen);
   wnl->wmsg.type = ntohs(msg->type);

   memcpy(localBuf, &pserver->nl, sizeof(struct nlmsghdr));

   memcpy((char *)(wnl+1), (char *)(msg+1), msgLen - sizeof(tAniHdr));

#ifdef ANI_DEBUG
   aniAsfLogMsg(LOG_INFO, ANI_WHERE, "Sending to aniAsfIpcSend len=%d radio=%d nl size=%d",
                 pserver->nl.nlmsg_len, wnl->radio, sizeof(struct nlmsghdr));
#endif

   aniAsfLogConsole(0);
   /*
    * push this message down to the HDD via the netlink
    * socket.
    */
   if (aniAsfIpcSend(pserver->ipcnl, localBuf, pserver->nl.nlmsg_len) < 0)
   {
#ifdef ANI_DEBUG
      aniAsfLogConsole(1);
      aniAsfLogMsg(LOG_ERR, ANI_WHERE, "%s: Could not send message to the kernel",
                   __FUNCTION__);
#endif
      free(localBuf);
      return -1;
   }
   aniAsfLogConsole(1);

   free(localBuf);
   return 0;
}


int wifiLoggerAppRegister(tWLogCtxt *pserver)
{
   tAniHdr msg;

   msg.type = WLAN_NL_TYPE_LOG_REG;
   msg.length = sizeof(tAniHdr);

   if (wifiLoggerAppSendMsgToKernel(pserver, pserver->radio, &msg,
                                    ANI_NL_MSG_LOG, sizeof(tAniHdr)) < 0)
   {
#ifdef ANI_DEBUG
      aniAsfLogMsg(LOG_ERR, ANI_WHERE, "WifiLoggerApp: Could not send data to the HDD\n");
#endif
   }

   return 0;
}

/*
 * Process all the messages from coming from the Radio Driver
 */
void wifiLoggerAppProcNetlinkMsg(void *arg)
{
   int    msgLen;
   char   *buf;
   int    pttRspLen;
   tAniNlHdr *wnl;
   tWLogCtxt *pserver = (tWLogCtxt *)arg;
   tAniIpc      *nIpc = pserver->ipcnl;
   int contentsLength = 0;
   char fileName[60];
   char *mode;

   static int fileIndex = 0;
   static FILE *fp = NULL;

   if ((buf = malloc(MAX_DRV_MSG_SIZE)) == NULL)
   {
      aniAsfLogMsg(LOG_ERR, ANI_WHERE,"WifiLoggerApp: %s - malloc failed\n",
                    __FUNCTION__);
      return;
   }
   memset(buf, 0, MAX_DRV_MSG_SIZE);

   if ((msgLen = aniAsfIpcRecv(nIpc, buf, MAX_DRV_MSG_SIZE)) <= 0)
   {
      aniAsfLogMsg(ANI_IPCRECV_ERR);
   }
   wnl = (tAniNlHdr *)buf;
   if (wnl->wmsg.type == WLAN_NL_TYPE_LOG_MSG)
   {
      // Log to Console
      if (pserver->logToConsole == 1)
      {
         //Using printf the log console (time to log using printf is low)
         //aniAsfLogMsg(LOG_ERR, ANI_LOG_PLAIN, "%s\n",((char *)temp+2));
         printf("%s\n", ((char *)(wnl + 1)));
      }

      // Log to SD Card
      if (pserver->logToSDCard == 1)
      {
         mode = fp ? "a" : "w";
         snprintf(fileName, sizeof(fileName), "%s/" FILENAME_PREFIX "%d",
                                        pserver->logFilePath, fileIndex);
         fp = fopen(fileName, mode);
         if (fp)
         {
            fprintf(fp,"%s",((char *)(wnl + 1)));
            if (ftell(fp) >= MAX_FILE_SIZE)
            {
               fileIndex++;
               fileIndex = fileIndex % MAX_FILES;
               fclose(fp);
               /* Assign fp to null in order to open file in write mode
                  when new file is chosen to log */
               fp = NULL;
            }
            else
               fclose(fp);
         }
         else
         {
            aniAsfLogConsole(0);
            aniAsfLogMsg(LOG_ERR, ANI_WHERE, "WifiLoggerApp: %s - failed open file %s\n",
                         __FUNCTION__, fileName);
            exit(1);
         }
      }
   }
   if(buf)
   {
      free(buf);
   }
}

void wifiLoggerAppCleanup(tAniIpc *ipcs, tAniIpc *clipc, tAniIpc *ipcnl)
{
   if (clipc)
   {
      aniAsfIpcUnSetFd(clipc);
      aniAsfIpcClose(clipc);
   }

   if (ipcs)
   {
      aniAsfIpcUnSetFd(ipcs);
      aniAsfIpcClose(ipcs);
   }

   if (ipcnl)
   {
      aniAsfIpcUnSetFd(ipcnl);
      aniAsfIpcClose(ipcnl);
   }
}

void WifiLoggerAppSigInt(int signum)
{
   sigInt = 1;
#ifdef ANI_DEBUG
   aniAsfLogMsg(LOG_ERR, ANI_WHERE, "WifiLoggerApp: Caught a SIGINT, exiting\n");
#endif
}

void WifiLoggerAppSigIntrInit(void)
{
   struct sigaction act;

   // Initialize the sigaction structure
   memset(&act, 0, sizeof(struct sigaction));
   act.sa_handler = &WifiLoggerAppSigInt;
   sigemptyset(&act.sa_mask);
   act.sa_flags = SA_RESTART;

   // Register the call back function for the SIGALRM signal
   sigaction(SIGINT, &act, NULL);
}

int WifiLoggerAppInit(tWLogCtxt *pserver, int radio)
{
   unsigned char   buf[MAX_DRV_MSG_SIZE];
   int ret = 0;
   int regMsgLen = 0;
   tAniNlAppRegReq *regReq;
   tAniNlHdr *wnl;

   if (!pserver)
   {
      aniAsfLogMsg(LOG_ERR, ANI_WHERE, "Invalid pserver passed in\n");
      return ANI_E_FAILED;
   }

   do
   {

       if (radio && radio >= 2)
       {
          aniAsfLogMsg(LOG_ERR, ANI_WHERE, "Invalid radio id [%d] passed in\n", radio);
          ret = ANI_E_FAILED;
          break;
       }
#ifdef ANI_DEBUG
       aniAsfLogMsg(LOG_INFO, ANI_WHERE, "RADIO id [%d] passed in\n", radio);
#endif

       pserver->radio = radio;

       // Open Netlink Socket to Pseudo Driver
       if ((pserver->ipcnl = aniAsfIpcOpen(AF_NETLINK, SOCK_DGRAM, 1)) == NULL)
       {
          aniAsfLogMsg(ANI_IPCOPEN_ERR);
          wifiLoggerAppCleanup(pserver->ipcs, pserver->clIpc, pserver->ipcnl);
          ret = ANI_E_FAILED;
          break;
       }
       aniAsfIpcConnect(pserver->ipcnl, "localhost", -1, -1);

       /*
        * Init a pre allocated Netlink msg hdr that we will use
        * later on while relaying messages coming in from the client
        * to the Radio Driver.  The nlmsg->type and nlmsg->len
        * parameters are filled in later on for each message as
        * appropriate.
        */
       pserver->nl.nlmsg_seq = 0;

       /*
        * Get the sockaddr_nl structure from ASF for this tAniIpc
        */
       pserver->snl = aniAsfIpcGetSnl(pserver->ipcnl);
       if (!pserver->snl)
       {
          ret = ANI_E_FAILED;
          break;
       }
       pserver->nl.nlmsg_pid = pserver->snl->nl_pid;
       pserver->nl.nlmsg_flags = NLM_F_REQUEST;

       // Register a callback for the Netlink fd
       aniAsfIpcSetFd(pserver->ipcnl, wifiLoggerAppProcNetlinkMsg, pserver);

       // Register with the in kernel Netlink handler
       pserver->nl.nlmsg_type = ANI_NL_MSG_PUMAC;
       pserver->nl.nlmsg_seq++;
       regMsgLen = aniNlLen(sizeof(tAniNlAppRegReq));
       pserver->nl.nlmsg_len = aniNlAlign(sizeof(tAniNlHdr)) + regMsgLen;

       // copy the netlink msg hdr first (assuming buf is 4 byte aligned)
       memcpy(buf, &pserver->nl, sizeof(struct nlmsghdr));

       wnl = (tAniNlHdr *)buf;
       wnl->radio = pserver->radio;

       // setup the tAniHdr next
       wnl->wmsg.type = htons(ANI_MSG_APP_REG_REQ);
       wnl->wmsg.length = regMsgLen;

       // align the buf and setup the tAniAppRegReq next
       regReq = (tAniNlAppRegReq *)(wnl + 1);
       regReq->pid = pserver->snl->nl_pid;

       pserver->nl.nlmsg_type = ANI_NL_MSG_LOG;

   } while (0);

   return ret;
}


void wifiLoggerAppMenu()
{
    aniAsfLogMsg(LOG_ERR, ANI_LOG_PLAIN, "Usage: WifiLogger_app [-o <1/0>] [-i <time_to_poll_in_sec>] [-p <path to store logs at>]\n"
           "       o - 0 to log to console only\n"
           "           1 to log to SD card only\n"
           "           3 to log to both console and SD card\n"
           "\n"
           "       i - Time interval in sec to poll for log msg (default %d)\n"
           "       p - path where log files will be saved (default %s)\n",
            DEFAULT_SELECT_TIMEOUT, DEFAULT_LOG_PATH);
}
/**
 *    Main function for the NetSim Server daemon
 *
 *    Supports two options
 *       -d - not to daemonize
 *       -v - Set Max log level
 *
 */
int main (int argc, char *argv[])
{
   int c;
   int radio = ANI_RADIO_0;
   int ret = 0;
   int outValue = 0;
   int selectTimeout = DEFAULT_SELECT_TIMEOUT;

   tWLogCtxt *pserver = &serverCtxt;

   // Initialize aniAsfLogInit
   aniAsfLogInit("WifiLoggerApp", LOG_INFO, 1);

   snprintf(pserver->logFilePath, sizeof(pserver->logFilePath), "%s",
                                                    DEFAULT_LOG_PATH);
   while ((c = getopt(argc, argv, "?o:i:p:")) != EOF)
   {
      switch(c) {
         case 'o':
            outValue = atoi(optarg);
            break;
         case 'i':
            selectTimeout = atoi(optarg);
            break;
         case 'p':
            strlcpy(pserver->logFilePath, optarg, sizeof(pserver->logFilePath));
            break;
         case '?':
         default:
            wifiLoggerAppMenu();
            exit(1);
      }
   }

   if ( 0 == outValue )
   {
      pserver->logToConsole = 1;
   }
   else if ( 1 == outValue )
   {
      pserver->logToSDCard = 1;
      pserver->logToConsole = 0;
   }
   else
   {
      pserver->logToSDCard = 1;
      pserver->logToConsole = 1;
   }

   aniAsfLogMsg(LOG_ERR, ANI_LOG_PLAIN, "================================================\n\n");
   aniAsfLogMsg(LOG_ERR, ANI_LOG_PLAIN, "Poll interval %d\noutValue %d\n",
                                                      selectTimeout, outValue);
   if (pserver->logToSDCard)
   {
      aniAsfLogMsg(LOG_ERR, ANI_LOG_PLAIN, "Logs File Path %s\n",
                                                         pserver->logFilePath);
   }
   aniAsfLogMsg(LOG_ERR, ANI_LOG_PLAIN, "================================================\n");

   exitLogging = 0;

   WifiLoggerAppSigIntrInit();

   // Init local datastructures
   if ((WifiLoggerAppInit(&serverCtxt, radio)) < 0)
   {
      aniAsfLogMsg(LOG_ERR, ANI_WHERE, "WifiLoggerAppInit failed\n");
      exit(1);
   }

   wifiLoggerAppRegister(pserver);

   // Loop for ever
   while (1)
   {

      ret = aniAsfIpcCheck(pserver->ipcnl, selectTimeout*1000);
      if ( !ret ) // Timeout. No data available on sock
      {
         //This condition is hit when wifi is switched off
         // or when there is not enough logs to be sent from driver
         // So we try to re-register with driver again so that we
         // receive the current buffered logs.
         wifiLoggerAppRegister(pserver);
      }
      else if (1 == ret) // Data Available
      {
         wifiLoggerAppProcNetlinkMsg(pserver);
         if (sigInt || exitLogging)
         {
             break;
         }
      }
   }

   wifiLoggerAppCleanup(serverCtxt.ipcs, serverCtxt.clIpc, serverCtxt.ipcnl);

   if (pserver->logToSDCard)
   {
      aniAsfLogMsg(LOG_ERR, ANI_LOG_PLAIN, "Logs File Path %s\n",
                                                        pserver->logFilePath);
   }

   return 0;
}
