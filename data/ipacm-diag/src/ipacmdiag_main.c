/******************************************************************************

                        IPACMDIAG_MAIN.C

******************************************************************************/

/******************************************************************************

  @file    ipacmdiag_main.c
  @brief   Qualcomm IPA_Configuration_Mamager_Log Module

  DESCRIPTION
  Implementation of IPACM DIAG log mechanism.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when        who       what, where, why
--------   -----      -------------------------------------------------------
05/05/14   skylar     Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/if.h>
#include <sys/un.h>
#include <errno.h>

#define IPACMDIAG_SUCCESS                0         /* Successful operation   */
#define IPACMDIAG_FAILURE               -1         /* Unsuccessful operation */
#ifdef FEATURE_IPA_ANDROID
#include <cutils/sockets.h>
#define IPACMDIAG_FILE "ipacm_log_file"
#else/* defined(FEATURE_IPA_ANDROID) */
#define IPACMDIAG_FILE "/etc/ipacm_log_file"
#endif /* defined(NOT FEATURE_IPA_ANDROID)*/
#define MAX_BUF_LEN 256

#include "ds_util.h"

/*	Log Message Macros	*/
#define LOG_MSG_INFO1_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO2_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO3_LEVEL           MSG_LEGACY_LOW
#define LOG_MSG_ERROR_LEVEL           MSG_LEGACY_ERROR
#define PRINT_MSG( level, fmtString, x, y, z)                         \
        MSG_SPRINTF_4( MSG_SSID_LINUX_DATA, level, "%s(): " fmtString,      \
                       __FUNCTION__, x, y, z);
#define LOG_MSG_INFO1( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO1_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_INFO2( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO2_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_INFO3( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO3_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_ERROR( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_ERROR_LEVEL, fmtString, x, y, z);                \
}


typedef struct
{
  fd_set fds;
  int    max_fd;
} ipacm_os_params;

typedef struct ipacm_log_buffer_s {
	char	user_data[MAX_BUF_LEN];
} ipacm_log_buffer_t;

/* Server sockets */
int ipacm_log_sockfd;
static ipacm_os_params	os_params;


int create_socket( int *sockfd)
{

  if ((*sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == IPACMDIAG_FAILURE)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return IPACMDIAG_FAILURE;
  }

  if(fcntl(*sockfd, F_SETFD, FD_CLOEXEC) < 0)
  {
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);
  }

  return IPACMDIAG_SUCCESS;
}

int create_ipacm_log_socket()
{
  int val, rval;
  struct sockaddr_un ipacmdiag_socket;
  int len;
  struct timeval rcv_timeo;

#ifdef FEATURE_IPA_ANDROID /* defined(NOT FEATURE_IPA_ANDROID)*/
  ipacm_log_sockfd = android_get_control_socket(IPACMDIAG_FILE);
#else/* defined(FEATURE_IPA_ANDROID) */
  rval = create_socket(&ipacm_log_sockfd);
#endif
  if (ipacm_log_sockfd < 0)
  {
    LOG_MSG_ERROR("Can't get socket file descriptor", 0, 0, 0);
    exit(0);
  }

  printf("server create ipacm_log socket successfully (%d)\n", ipacm_log_sockfd);

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(ipacm_log_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(ipacm_log_sockfd, F_GETFL, 0);
  fcntl(ipacm_log_sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(ipacm_log_sockfd, &os_params.fds);
  os_params.max_fd = MAX(os_params.max_fd, ipacm_log_sockfd);

#ifndef FEATURE_IPA_ANDROID /* defined(NOT FEATURE_IPA_ANDROID)*/
  ipacmdiag_socket.sun_family = AF_UNIX;
  strlcpy(ipacmdiag_socket.sun_path, IPACMDIAG_FILE,sizeof(ipacmdiag_socket.sun_path));
  unlink(ipacmdiag_socket.sun_path);
  len = strlen(ipacmdiag_socket.sun_path) + sizeof(ipacmdiag_socket.sun_family);
  if (bind(ipacm_log_sockfd, (struct sockaddr *)&ipacmdiag_socket, len) == IPACMDIAG_FAILURE)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    return IPACMDIAG_FAILURE;
  }
#endif
  return IPACMDIAG_SUCCESS;
}

int main(int argc, char ** argv)
{
	int ret;
	int ret_val,value;
	int fd;
	ipacm_log_buffer_t *ipacm_log_buffer = NULL;
	fd_set master_fd_set;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	int nbytes=0, i;
	char buf[MAX_BUF_LEN];

	/*Initialize the Diag for QXDM logs*/
	if (TRUE != Diag_LSM_Init(NULL))
	{
		perror("Diag_LSM_Init failed !!");
	}

	/* Create ipacm_log -> ipacm server socket */
	ret_val = create_ipacm_log_socket();
	if (ret_val != IPACMDIAG_SUCCESS)
	{
		LOG_MSG_ERROR("Unable to create ipacm_log socket!", 0,0,0);
		return IPACMDIAG_FAILURE;
	}
	printf(" Start IPACM_LOG Successfully\n");
	LOG_MSG_INFO1("Start IPACM_LOG Successfully",0,0,0);

    while(1)
	{
		master_fd_set = os_params.fds;
		ret = select(os_params.max_fd+1, &master_fd_set, NULL, NULL, NULL);
		if (ret < 0)
		{
			LOG_MSG_ERROR("Error in select, errno:%d", errno, 0, 0);
			if( errno == EINTR )
			continue;
			else
			return -1;
		}
		for (i = 0; ( i <= os_params.max_fd ); i++)
		{
			if (FD_ISSET(i, &master_fd_set))
			{
				if ( i == ipacm_log_sockfd )
				{
					if ( ( nbytes = recvfrom(i, buf, MAX_BUF_LEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) <= 0 )
					{
						if ( nbytes == 0 )
						{
							LOG_MSG_INFO1("Completed full recv from ipacm callback", 0, 0, 0);
						}
						else
						{
							LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
						}
					}
					else
					{
						ipacm_log_buffer = (ipacm_log_buffer_t *)buf;
						/* Print the msg to QXDM log */
						if(ipacm_log_buffer->user_data != NULL)
						{
							LOG_MSG_INFO1("get %s\n", ipacm_log_buffer->user_data, 0, 0);
						}
					}
				}
			}
		}// end for loop
	}
}
