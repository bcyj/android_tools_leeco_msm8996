/******************************************************************************

                           QCMAP_WEB_CGI.C

******************************************************************************/

/******************************************************************************

  @file    qcmap_web_cgi.c
  @brief   Mobile AP HTTP CGI Module Implementation

  DESCRIPTION
  Mobile AP HTTP CGI Module Implementation

  ---------------------------------------------------------------------------
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        --------------------------------------------------------
01/28/14   rk         webserver kw fixes.
01/08/14   rk         webserver paswd encryption.
11/22/13   rk         webserver security flaws fixed.
04/04/12   vb         Initial CGI implementation.
******************************************************************************/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif
#define UNIX_PATH_MAX    108
#define WEBCGI_WEBCLIENT_SOCK "/www/qcmap_webclient_cgi_file"
#define WEBCLIENT_WEBCGI_SOCK "/www/qcmap_cgi_webclient_file"
//Time to wait for socket responce in secs.
#define SOCK_TIMEOUT 3
//Returns Minimum value of both
#define MIN(a,b) (((a)<(b))?(a):(b))
//Max Buffer size for HTTP and Socket transactions
#define MAX_BUFFER_SIZE 40960

#define HTTP_RESPONCE_HEADER "Content-Type: text/html\nContent-Length: "
#define SYSTEM_ERR "System Error"
#define SYSTEM_ERR_LEN 12
#define GOT_NOTHING "Nothing"
#define GOT_NOTHING_LEN 7
#define MEMORY_ERROR "Memory Error"
#define MEMORY_ERROR_LEN 12
#define HTTP_READ_ERROR "HTTP Read Error"
#define HTTP_READ_ERROR_LEN 15
#define SOCK_OPEN_ERROR "{\"commit\":\"Socket Open Error\"}"
#define SOCK_OPEN_ERROR_LEN 30
#define SOCK_FD_ERROR "{\"commit\":\"Socket FD Error\"}"
#define SOCK_FD_ERROR_LEN 28
#define SOCK_BIND_ERROR "{\"commit\":\"Socket Bind Error\"}"
#define SOCK_BIND_ERROR_LEN 30
#define SOCK_PATH_ERROR "{\"commit\":\"Socket Path Error\"}"
#define SOCK_PATH_ERROR_LEN 30
#define SOCK_SEND_ERROR "{\"commit\":\"Socket Send Error\"}"
#define SOCK_SEND_ERROR_LEN 30
#define SOCK_SEND_COMPLETE_ERROR "{\"commit\":\"Unable to Send Complete "\
                                 "data through socket\"}"
#define SOCK_SEND_COMPLETE_ERROR_LEN 56
#define SOCK_TIMEOUT_ERROR "{\"commit\":\"Socket TimeOut\"}"
#define SOCK_TIMEOUT_ERROR_LEN 27
#define SOCK_RESPONSE_ERROR "{\"commit\":\"Socket Responce Error\"}"
#define SOCK_RESPONSE_ERROR_LEN 34
#define SOCK_RECEIVE_ERROR "{\"commit\":\"Socket Receive Error\"}"
#define SOCK_RECEIVE_ERROR_LEN 33
#define SUCCESS 0
#define FAIL    -1
#define SESSION_FILE "/www/qcmap_session"
//restrict file access to user,group only.
#define CHG_PERM_SESSION_FILE "chmod 770 /www/qcmap_session"
//change file owner,group name to www-data for security reasons instead  root.
#define CHG_PERM_SESSION_FILE_OWNR "chown www-data:www-data /www/qcmap_session"
#define LOG_BUFF_SIZE 28
#define MAX_HTTP_BUFFER_SIZE 100
#define IPV4_STRING_LEN 16
#define AUTH_FAIL "Content-Type: text/html\nContent-Length: \
                            22\n\n{\"result\":\"AUTH_FAIL\"}"
typedef struct
{
  //To store last login time
  struct timeval log_time;
  //To store last login login IP
  char ip[16];
  //To specify session timeout
  unsigned int timeout;
}loginfo;
int webcgi_webcli_communication(char *webcgi_webcli_buff,
                                int webcgi_webcli_buff_size,
                                char *webcli_webcgi_buff,
                                int *webcli_webcgi_buff_size);

int send_to_webcli(char *webcgi_webcli_buff,
                   int webcgi_webcli_buff_size,
                   char *webcli_webcgi_buff,
                   int *webcli_webcgi_buff_size,
                   int *webcli_sockfd);

int recv_from_webcli(char *webcli_webcgi_buff,
                     int *webcli_webcgi_buff_size,
                     int *webcli_sockfd);

static int Auth_request_ip();
int main(void)
{
  //To parse incoming HTTP Traffic Length.
  char *http_inc_traffic_len = NULL;
  //To store Min buffer length required to read HTTP,
  //afterwards this variable can be used as temp. buffer.
  int html_cgi_buff_len = 0;
  char cgi_html_buff[MAX_BUFFER_SIZE]={0}; //To Generate HTTP Response.
  //To store Min buffer length required to read HTTP,
  //afterwards this variable can be used as temp. buffer.
  int cgi_html_buff_len = 0;
  char *html_cgi_buff; //To store HTTP Request
  int res = 0;
  //Fetch incomming data length
  http_inc_traffic_len = getenv("CONTENT_LENGTH");
  if (http_inc_traffic_len)
  {
    // Authenticate request I.P
    if(SUCCESS != Auth_request_ip( ))
    {
      printf("%s",AUTH_FAIL);
      return FAIL;
    }
    //Initialize memories to hold data.
    memset(cgi_html_buff, 0, MAX_BUFFER_SIZE);
    html_cgi_buff_len = atoi(http_inc_traffic_len);
    if (html_cgi_buff_len > 0)
    {
      //Allocate Memory for storing HTML Request
      html_cgi_buff = 0;
      html_cgi_buff = (char *)calloc(html_cgi_buff_len,1);
      if (html_cgi_buff == 0)
      {
        printf("%s%d\n\n", HTTP_RESPONCE_HEADER, HTTP_READ_ERROR_LEN);
        printf("%s", HTTP_READ_ERROR);
        return FAIL;
      }
      memset(html_cgi_buff, 0, html_cgi_buff_len);
      res = fread( html_cgi_buff, 1, html_cgi_buff_len, stdin );
      if ( res != html_cgi_buff_len)
      {
        free((void *)html_cgi_buff);
        printf("%s%d\n\n", HTTP_RESPONCE_HEADER, HTTP_READ_ERROR_LEN);
        printf("%s", HTTP_READ_ERROR);
        return FAIL;
      }
    }
    else
    {
      printf("%s%d\n\n",HTTP_RESPONCE_HEADER,GOT_NOTHING_LEN);
      printf("%s",GOT_NOTHING);
     return FAIL;
    }
    //Send received data to webclient and get responce in return.
    webcgi_webcli_communication(html_cgi_buff, html_cgi_buff_len,
                                cgi_html_buff, &cgi_html_buff_len);
    free((void *)html_cgi_buff);
    html_cgi_buff = 0;
    printf("%s%d\n\n",HTTP_RESPONCE_HEADER,cgi_html_buff_len);
    printf("%s",cgi_html_buff);
  }
  else
  {
    printf("%s%d\n\n",HTTP_RESPONCE_HEADER,GOT_NOTHING_LEN);
    printf("%s",GOT_NOTHING);
  }
  return SUCCESS;
}
/*===========================================================================
  FUNCTION webcgi_webcli_communication
===========================================================================*/
/*!
@brief
  This function acts like proxy by sending data received from HTTP request
  to Webclient and returning the responce from Webclient as HTTP responce.

@return
  int  - Returns whether Socket communication is successfull or failure.
      0 - Success
      1 - Failed
@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int webcgi_webcli_communication(char *webcgi_webcli_buff,
                                int webcgi_webcli_buff_size,
                                char *webcli_webcgi_buff,
                                int *webcli_webcgi_buff_size)
{
  //Socket FD for Webclient socket. Used to communicate with Webclient socket.
  unsigned int webcli_sockfd = 0;

  //Send data to WEB_CLI socket
  if (send_to_webcli(webcgi_webcli_buff, webcgi_webcli_buff_size,
                     webcli_webcgi_buff, webcli_webcgi_buff_size,
                     &webcli_sockfd) == 0)
  {
    //Receive data from Webclient
    recv_from_webcli(webcli_webcgi_buff, webcli_webcgi_buff_size,
                     &webcli_sockfd);
  }
  return SUCCESS;
}
/*===========================================================================
  FUNCTION send_to_webcli
===========================================================================*/
/*!
@brief
  This function send data received from HTTP post message to QCMAP_CLI socket.

@return
  int  - Returns whether Socket communication is successfull or failure.
      0 - Success
      1 - Failed
@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int send_to_webcli(char *webcgi_webcli_buff,
                   int webcgi_webcli_buff_size,
                   char *webcli_webcgi_buff,
                   int *webcli_webcgi_buff_size,
                   int *webcli_sockfd)
{
  //Socket Address to store address of webclient and web cgi sockets.
  struct sockaddr_un webcli_webcgi_sock, webcgi_webcli_sock;
  //Variables to track data received and sent.
  int bytes_sent_to_cli = 0;
  int sock_addr_len = 0;

  memset(&webcli_webcgi_sock,0,sizeof(struct sockaddr_un));
  memset(&webcgi_webcli_sock,0,sizeof(struct sockaddr_un));
  if ((*webcli_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    *webcli_webcgi_buff_size = SOCK_OPEN_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_OPEN_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  if (fcntl(*webcli_sockfd, F_SETFD, FD_CLOEXEC) < 0)
  {
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_FD_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_FD_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  webcli_webcgi_sock.sun_family = AF_UNIX;
  if(strlen(WEBCLIENT_WEBCGI_SOCK) >= UNIX_PATH_MAX )
  {
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_PATH_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_PATH_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  strlcat(webcli_webcgi_sock.sun_path, WEBCLIENT_WEBCGI_SOCK,UNIX_PATH_MAX);
  unlink(webcli_webcgi_sock.sun_path);
  sock_addr_len = strlen(webcli_webcgi_sock.sun_path) + sizeof(webcli_webcgi_sock.sun_family);
  if (bind(*webcli_sockfd, (struct sockaddr *)&webcli_webcgi_sock, sock_addr_len) == -1)
  {
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_BIND_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_BIND_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  webcgi_webcli_sock.sun_family = AF_UNIX;
  if(strlen(WEBCGI_WEBCLIENT_SOCK) >= UNIX_PATH_MAX )
  {
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_PATH_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_PATH_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  strlcat(webcgi_webcli_sock.sun_path, WEBCGI_WEBCLIENT_SOCK,UNIX_PATH_MAX);
  /*d rwx rwx rwx = dec
    0 110 110 110 = 666
  */
  system("chmod 777 /www/qcmap_cgi_webclient_file");
  sock_addr_len = strlen(webcgi_webcli_sock.sun_path) + sizeof(webcgi_webcli_sock.sun_family);
  if ((bytes_sent_to_cli = sendto(*webcli_sockfd, webcgi_webcli_buff, webcgi_webcli_buff_size, 0,
      (struct sockaddr *)&webcgi_webcli_sock, sock_addr_len)) == -1)
  {
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_SEND_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_SEND_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  if (bytes_sent_to_cli == webcgi_webcli_buff_size)
  {
   return SUCCESS;
  }
  else
  {
    close((int)*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_SEND_COMPLETE_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_SEND_COMPLETE_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
}
/*===========================================================================
  FUNCTION recv_from_webcli
===========================================================================*/
/*!
@brief
  This function frames HTTP response message with data received from WEB_CLI
  socket.

@return
  int  - Returns whether Socket communication is successfull or failure.
      0 - Success
      1 - Failed
@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int recv_from_webcli(char *webcli_webcgi_buff,
              int *webcli_webcgi_buff_size,
              int *webcli_sockfd)
{
  //FD set used to hold sockets for select.
  fd_set webcgi_sockfd;
  //Time out for webcgi response.
  struct timeval webcli_sock_timeout;
  //To evaluate webclient socket responce(timed out, error, ....)
  int webcli_sock_resp_status = 0;
  //Variables to track data received and sent.
  int bytes_recv_from_cli = 0;

  FD_ZERO(&webcgi_sockfd);
  if( *webcli_sockfd < 0 )
  {
    *webcli_webcgi_buff_size = SOCK_FD_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_FD_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  FD_SET(*webcli_sockfd, &webcgi_sockfd);
  webcli_sock_timeout.tv_sec = SOCK_TIMEOUT;
  webcli_sock_timeout.tv_usec = 0;
  webcli_sock_resp_status = select(((int)(*webcli_sockfd))+1,
                                   &webcgi_sockfd, NULL, NULL,
                                   &webcli_sock_timeout);
  if (webcli_sock_resp_status == 0)
  {
    FD_CLR(*webcli_sockfd, &webcgi_sockfd);
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_TIMEOUT_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_TIMEOUT_ERROR, MAX_BUFFER_SIZE);
    return FAIL;  //Time out
  }
  else if  (webcli_sock_resp_status == -1)
  {
    FD_CLR(*webcli_sockfd, &webcgi_sockfd);
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_RESPONSE_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_RESPONSE_ERROR, MAX_BUFFER_SIZE);
    return FAIL;  //Error
  }
  memset(webcli_webcgi_buff, 0, MAX_BUFFER_SIZE);
  bytes_recv_from_cli = recv(*webcli_sockfd, webcli_webcgi_buff, MAX_BUFFER_SIZE, 0);
  if (bytes_recv_from_cli == -1)
  {
    FD_CLR(*webcli_sockfd, &webcgi_sockfd);
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = SOCK_RECEIVE_ERROR_LEN;
    strlcat(webcli_webcgi_buff, SOCK_RECEIVE_ERROR, MAX_BUFFER_SIZE);
    return FAIL;
  }
  else
  {
    FD_CLR(*webcli_sockfd, &webcgi_sockfd);
    close(*webcli_sockfd);
    *webcli_webcgi_buff_size = bytes_recv_from_cli;
    webcli_webcgi_buff[bytes_recv_from_cli] = 0;
    return SUCCESS;
  }
}
/*===========================================================================
  FUNCTION Auth_request_ip
===========================================================================*/
/*!
@brief
  Autheticate if request is from the configured IP or not.

@input
  None

@return
   0 - success
   -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int Auth_request_ip()
{
  int DEFAULT_TIMEOUT  = 300;
  char *remote_addr = NULL;
  remote_addr = getenv("REMOTE_ADDR");
  //To store return values from system calls for comparison
  int res = 0;
  char remote_ip[IPV4_STRING_LEN] = {0};
  strlcat(remote_ip, remote_addr, IPV4_STRING_LEN);
  FILE *fp;
  loginfo *linfo;
  //To store log message written to file
  loginfo sinfo;
  //Time stamp of the system
  struct timeval sys_time;
  char session_buff[MAX_HTTP_BUFFER_SIZE]={0};
  memset(session_buff, 0, MAX_HTTP_BUFFER_SIZE);
  fp = fopen(SESSION_FILE, "r");
  if (fp)
  {
    //Read session IP info
    fread(session_buff,1,LOG_BUFF_SIZE,fp);
    //Close file after reading
    fclose(fp);
    linfo = (loginfo *)session_buff;
    // compare to see if no other ip is using webserver
    if ((!strncmp(linfo->ip,remote_ip,strlen(linfo->ip))))
    {
      // Also update timestamp with every request
      fp = fopen(SESSION_FILE, "w+");
      if (fp)
      {
        //file access restrict to only owner, and group,
        //as per security recommendations.
        system(CHG_PERM_SESSION_FILE);
        //change owner,group of file to www-data,
        //as per security recommendations.
        system(CHG_PERM_SESSION_FILE_OWNR);
        //Read system time just to fill in structure
        if (gettimeofday(&sys_time, 0) == 0)
        {
          strlcat(sinfo.ip, remote_ip,IPV4_STRING_LEN);
          sinfo.log_time.tv_sec = sys_time.tv_sec;
          sinfo.log_time.tv_usec = sys_time.tv_usec;
          if( linfo)
            //store timeout in session info struct.
            sinfo.timeout = linfo->timeout;
          else
            //can't read from file, store default timeout value.
            sinfo.timeout = DEFAULT_TIMEOUT;
          //update timeout value in structure inturn updating it in file
          res = fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp);
          fclose(fp);
          if(res == LOG_BUFF_SIZE)
          {
            return SUCCESS;
          }
          else
          {
            return FAIL;
          }
        }
        else
        {
          fclose(fp);
          return FAIL;
        }
      }
      else
      {
        //Send failure HTTP responce
        return FAIL;
      }
    }
  }
  return FAIL;
}
