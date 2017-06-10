/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Overview: This file sends basic fm commands(enable, disable, tune etc)
 *           to fm server
 *
 **/

#include "FmRadioController_qcom.h"
#include "Qualcomm_FM_Const.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/un.h>
#include <cstdio>
#include <utils/Log.h>

#define MAX_BUFFER 256
#define MIN_FREQ 87.5
#define MAX_FREQ 108.0
#define DELIM_CHR ':'

static char const *server_filename =
                         "/dev/socket/fm_factory_socket_server";

static int sockfds;
static int sockfdc;
static struct sockaddr_un server_addr;
static struct sockaddr_un client_addr;
static char buf[MAX_BUFFER] = {0};
static char org_buf[MAX_BUFFER] = {0};

static void error
(
   char const *msg
);

static int execute_valid_cmd
(
   char *cmd,
   FmRadioController_qcom **fm_cntrllr
);

int main(int argc, char *argv[])
{
   int servlen, n;
   int ret = -1;
   socklen_t clilen;
   FmRadioController_qcom *fm_cntrllr = NULL;

   unlink(server_filename);

   if((sockfds = socket(1, SOCK_STREAM, 0)) < 0) {
       error("Server: creating socket");
       return -1;
   }

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sun_family = 1;
   strcpy(server_addr.sun_path, server_filename);
   servlen = strlen(server_addr.sun_path) +
                     sizeof(server_addr.sun_family);

   if(bind(sockfds, (struct sockaddr *)&server_addr, servlen) < 0) {
      error("Server: binding socket");
      close(sockfds);
      return -1;
   }

   listen(sockfds, 5);
   clilen = sizeof(client_addr);
   while(true) {
         sockfdc = accept(sockfds, (struct sockaddr *)&client_addr,
                            &clilen);
         if(sockfdc < 0) {
            error("Server: accepting");
            continue;
         }else {
            ALOGE("Server: a connection has been established\n");
         }

         n = read(sockfdc, buf, MAX_BUFFER);
         if(n > 0) {
            memcpy(org_buf, buf, n);
            ALOGE("Server: got command \" %s \"\n", buf);
            ret = execute_valid_cmd(buf, &fm_cntrllr);
         }
         ret = snprintf(buf, MAX_BUFFER, "%d", ret);
         if((ret <= 0) || (ret >= MAX_BUFFER)) {
            snprintf(buf, MAX_BUFFER, "%d", 1);
         }
         ALOGE("Server: sending response %s\n", buf);
         write(sockfdc, buf, strlen(buf));
         close(sockfdc);
         if(strcmp(org_buf, "disable") == 0) {
            break;
         }
         memset(buf, 0, MAX_BUFFER);
         memset(org_buf, 0, MAX_BUFFER);
   }

   close(sockfds);

   return 0;
}

static int execute_valid_cmd
(
    char *cmd,
    FmRadioController_qcom **fm_cntrllr
)
{
    double freq;
    char *delim;
    int ret ;

    if(fm_cntrllr == NULL) {
       return -1;
    }else if(*fm_cntrllr == NULL) {
       *fm_cntrllr = new FmRadioController_qcom();
        if(*fm_cntrllr == NULL)
           return -1;
    }
    if((strcmp(cmd, "enable") == 0)) {
        ret = (*fm_cntrllr)->Initialise();
	if(ret)
	   return -1;
	return (*fm_cntrllr)->SetChannelSpacing(CHAN_SPACE_50);

    }else if((strcmp(cmd, "disable") == 0)) {
        delete (*fm_cntrllr);
        *fm_cntrllr = NULL;
        return 0;
    }else if((strcmp(cmd, "seeknext") == 0)) {
        return (*fm_cntrllr)->SeekUp();
    }else if((delim = strchr(cmd, DELIM_CHR)) != NULL) {
        if(memcmp(cmd, "tune", (delim - cmd)) == 0) {
           delim++;
           freq = atof(delim);
           if((freq >= MIN_FREQ) &&( freq <= MAX_FREQ)) {
               return (*fm_cntrllr)->TuneChannel(freq * 1000);
           }else {
               return -1;
           }
        }else {
           return -1;
        }
    }else {
        return -1;
    }
}

void error
(
   char const *msg
)
{
   perror(msg);
}
