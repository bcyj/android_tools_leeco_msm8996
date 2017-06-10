/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Overview: This file sends basic fm commands(enable, disable, tune etc)
 *           to fm server
 *
 **/
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <utils/Log.h>

#define MAX_BUFFER 256
#define MIN_FREQ 87.5
#define MAX_FREQ 108.0
#define SERVER_WAIT_TIME 200000
#define MAX_POLL 10

static char const *server_filename =
                        "/dev/socket/fm_factory_socket_server";

static struct sockaddr_un server_addr;
static int sockfds;
static char buffer[MAX_BUFFER] = {0};
static int servlen;

static void error
(
   char const *msg
);

static int set_up_connection
(
   void
);

static int get_cmd_rsp
(
   char *argv[]
);

static int get_cmd
(
   char command[],
   char *argv[]
);

static int valid_cmd
(
   int argc,
   char *argv[]
);

static void print_cmd_rslt
(
   char *argv[],
   int rslt
);

static void print_enable_rslt
(
   char *argv[],
   int rslt
);

static void print_disable_rslt
(
   char *argv[],
   int rslt
);

static void print_seeknext_rslt
(
   char *argv[],
   int rslt
);

static void print_tune_rslt
(
   char *argv[],
   int rslt
);

int main(int argc, char *argv[])
{
   int n;
   int ret = -1;
   pid_t pid;

   ret = valid_cmd(argc, argv);
   if(ret != 0) {
      ALOGE("Client: command is not valid\n");
      return -1;
   }

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sun_family = 1;
   if(strlcpy(server_addr.sun_path, server_filename, UNIX_PATH_MAX)
       >= UNIX_PATH_MAX) {
      ALOGE("Client: path name for socket exceeds MAX limit\n");
      ret = -1;
      print_cmd_rslt(argv, ret);
      return ret;
   }
   servlen = strlen(server_addr.sun_path) +
                 sizeof(server_addr.sun_family);
   if((sockfds = socket(1, SOCK_STREAM,0)) < 0) {
       ALOGE("Client: Error Creating socket");
       ret = -1;
       print_cmd_rslt(argv, ret);
       return ret;
   }

   if(connect(sockfds, (struct sockaddr *)
                         &server_addr, servlen) < 0) {
       ALOGE("Client: Connection refused");
       pid = fork();
       if(pid == 0) {
          ret = system("fmfactorytestserver");
       }else if(pid > 0) {
          if(set_up_connection() == 0) {
             ret = get_cmd_rsp(argv);
             ALOGE("Client: cmd got executed\n");
          }else {
             ALOGE("Client: couldn't connect to server\n");
             ret = -1;
             print_cmd_rslt(argv, ret);
          }
       }else {
         ALOGE("Client: couldn't start server\n");
         ret = -1;
         print_cmd_rslt(argv, ret);
       }
   }else {
       ret = get_cmd_rsp(argv);
       ALOGE("Client: cmd got executed\n");
   }

   close(sockfds);
   return ret;
}

static int valid_cmd
(
   int argc,
   char *argv[]
)
{
   double freq;

   if(argc > 1) {
       if((strcmp(argv[1], "enable") == 0) && (argc == 2)) {
           return 0;
       }else if((strcmp(argv[1], "disable") == 0) && (argc == 2)) {
           return 0;
       }else if((strcmp(argv[1], "tune") == 0) && (argc == 3)) {
           freq = atof(argv[2]);
           if((freq >= MIN_FREQ) &&( freq <= MAX_FREQ)) {
              return 0;
           }else {
              return 1;
           }
       }else if((strcmp(argv[1], "seeknext") == 0) && (argc == 2)) {
           return 0;
       }else {
           return 1;
       }
   }else if((argc <= 1) || (argv == NULL)) {
       return 1;
   }else {
       return 1;
   }
}

static int get_cmd_rsp
(
    char *argv[]
)
{
    int err = -1;
    int n;

    err = get_cmd(buffer, argv);
    if(err != 0) {
       close(sockfds);
       return 1;
    }

    write(sockfds, buffer, strlen(buffer));
    memset(buffer, 0, MAX_BUFFER);
    n = read(sockfds, buffer, MAX_BUFFER);
    if(n > 0) {
       ALOGE("Client: got response: %s\n", buffer);
       err = atol(buffer);
       print_cmd_rslt(argv, err);
       if(!strcmp(argv[1], "seeknext")) {
          err = ((err > 0) ? 0 : 1);
       }
    }else {
    }

    return err;
}

static int get_cmd
(
   char command[],
   char *argv[]
)
{
   if((!strcmp(argv[1], "enable"))
       || (!strcmp(argv[1], "disable"))
       || (!strcmp(argv[1], "seeknext"))) {
       if(strlcpy(command, argv[1], MAX_BUFFER) >= MAX_BUFFER) {
          return 1;
       }
   }else if(strcmp(argv[1], "tune") == 0) {
       if(strlcpy(command, argv[1], MAX_BUFFER) >= MAX_BUFFER) {
          return 1;
       }
       if(strlcat(command, ":", MAX_BUFFER) >= MAX_BUFFER) {
         return 1;
       }
       if(strlcat(command, argv[2], MAX_BUFFER) >= MAX_BUFFER) {
          return 1;
       }
   }else {
       return 1;
   }
   return 0;
}

static int set_up_connection
(
   void
)
{
   int i;
   int err = 0;

   for(i = 0; i < MAX_POLL; i++) {
       if(connect(sockfds, (struct sockaddr *)
                         &server_addr, servlen) < 0) {
          err = -1;
          usleep(SERVER_WAIT_TIME);
          ALOGE("Client: trying again\n");
       }else {
          ALOGE("Client: connection established\n");
          err = 0;
          break;
       }
   }

   return err;
}

static void error
(
   char const *msg
)
{
   perror(msg);
}

static void print_cmd_rslt
(
   char *argv[],
   int rslt
)
{

   if(strcmp(argv[1], "enable") == 0) {
      print_enable_rslt(argv, rslt);
   }else if(strcmp(argv[1], "disable") == 0) {
      print_disable_rslt(argv, rslt);
   }else if(strcmp(argv[1], "tune") == 0) {
      print_tune_rslt(argv, rslt);
   }else if(strcmp(argv[1], "seeknext") == 0) {
      print_seeknext_rslt(argv, rslt);
   }
}

static void print_enable_rslt
(
   char *argv[],
   int rslt
)
{
   printf("cmd: %s\nresult: %d\n", argv[1], rslt);
}

static void print_disable_rslt
(
   char *argv[],
   int rslt
)
{
   printf("cmd: %s\nresult: %d\n", argv[1], rslt);
}

static void print_seeknext_rslt
(
   char *argv[],
   int rslt
)
{
   printf("cmd: %s\nresult: %d\n", argv[1], rslt);
}

static void print_tune_rslt
(
   char *argv[],
   int rslt
)
{
   printf("cmd: %s %s\nresult: %d\n", argv[1], argv[2], rslt);
}
