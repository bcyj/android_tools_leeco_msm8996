/******************************************************************************

                           QCMAP_AUTH.C

******************************************************************************/

/******************************************************************************

  @file    qcmap_auth.c
  @brief   Mobile AP Web Authentication Module Implementation

  DESCRIPTION
  Mobile AP Web Authentication Module Implementation

  ----------------------------------------------------------------------------
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ----------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        --------------------------------------------------------
04/04/12   vb         Initial Autorization bringup.
08/30/12   at         Added Timeout update and unecode of password.
11/22/13   rk         webserver security flaws fixed.
01/08/14   rk         webserver paswd encryption.
01/28/14   rk         webserver kw fixes.
03/17/14   rk         token generate command change.
04/16/14   rk         sha512 encryption of user password.
05/22/14   rk         sha512 encryption remove help.
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
#include <stdbool.h>
#include <sys/time.h>
#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif
//File used to store user credentials
#define PASSWORD_FILE "/www/lighttpd.user"
#define SESSION_FILE "/www/qcmap_session"
#define ATTEMPT_FILE "/www/login_attempt"
//Default and only user name for accessing web server
#define DEFAULT_USER_NAME "admin"
#define DEFAULT_PASSWD "admin"
//Returns Minimum value of both
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MAX_HTTP_BUFFER_SIZE 100       //Max Buffer size for HTTP Buffer
#define MAX_ELEMENT_LENGTH 45          //Max Size of any element value
#define MAX_ELEMENT_COUNT 5            //Max Elements can be processed
#define IPV4_STRING_LEN 16             //Max string length of IPV4 address
#define PASSWORD_LENGTH 107             //Max password in md5 form's length + 1
#define PASSWORD_POSITION 6
#define CHANGE_PWD_SUCCESS "Content-Type: text/html\nContent-Length: \
                            24\n\n{\"result\":\"0\", \"ip\":\"0\"}"
#define LOAD_WEBSERVER_SUCCESS "%s%d\n\n{\"result\":\"0\", \"ip\":\"%s\", \"timeout\":\"%06d\",\"token\":\"%s\"}"
#define LOAD_WEBSERVER_FAIL "%s%d\n\n{\"result\":\"1\", \"ip\":\"%s\"}"
#define FIRST_LOGIN_SUCCESS "%s%d\n\n{\"result\":\"2\", \"ip\":\"0\",\"token\":\"%s\"}"
//debug log for issue debugging
#define PRINT_DEBUG_LOG "%s%d\n\n{\"filepw\":\"%s\", \"pw\":\"%s\"}"
#define PRINT_DEBUG_LOG_CRYPT "%s%d\n\n{\"cryptf\":\"%s\", \"sa\":\"%s\"}"
#define LOGIN_FAIL "Content-Type: text/html\nContent-Length: \
                            24\n\n{\"result\":\"3\", \"ip\":\"0\"}"
#define MAX_ATTEMPT "Content-Type: text/html\nContent-Length: \
                            24\n\n{\"result\":\"4\", \"ip\":\"0\"}"
#define LOGIN_ENABLE "Content-Type: text/html\nContent-Length: \
                            14\n\n{\"result\":\"0\"}"
#define FILE_READ_ERR "Content-Type: text/html\nContent-Length: \
                            24\n\n{\"result\":\"9\", \"ip\":\"0\"}"
//error message for MD5 SUM failure
#define MD5_SUM_ERR "Content-Type: text/html\nContent-Length: \
                            24\n\n{\"result\":\"8\", \"ip\":\"0\"}"
#define FILE_CRED_ERR "Content-Type: text/html\nContent-Length: \
                            25\n\n{\"result\":\"13\", \"ip\":\"0\"}"
#define FILE_WRITE_ERR "Content-Type: text/html\nContent-Length: \
                            25\n\n{\"result\":\"10\", \"ip\":\"0\"}"
//error message for username entered wrong !!.
#define USERNAME_FAIL_ERR "Content-Type: text/html\nContent-Length: \
                            24\n\n{\"result\":\"7\", \"ip\":\"0\"}"
#define FILE_OPEN_ERR "Content-Type: text/html\nContent-Length: \
                            25\n\n{\"result\":\"11\", \"ip\":\"0\"}"
#define PASWD_COMPLEXITY_ERR "Content-Type: text/html\nContent-Length: \
                            24\n\n{\"result\":\"6\", \"ip\":\"0\"}"
#define SYSTEM_ERR "Content-Type: text/html\nContent-Length: \
                            25\n\n{\"result\":\"12\", \"ip\":\"0\"}"
#define GOT_NOTHING "Content-Type: text/html\nContent-Length: 7\n\n\
                     Nothing"
#define UNENCODE_ERR "Content-Type: text/html\nContent-Length: \
                       14\n\nunencode Error"
#define HTML_HEADER "Content-Type: text/html\nContent-Length: "
#define LOG_BUFF_SIZE 28 //Size of log structure
#define RESPONSE_SUCCESS_SIZE 54
//length of first time login success message.
#define FIRST_LGN_SUCC 35
#define RESPONSE_FAIL_SIZE 23
#define SUCCESS 0
#define FAIL    -1
#define ATTEMPT_BUFF_SIZE 12
#define MAX_LOGIN_ATTEMPT 4
#define LOGIN_DISABLE_DURATION 900
#define CHECK_ATTEMPT 1
#define CLEAR_ATTEMPT 2
#define TOKEN_SIZE 16
#define SESSION_TOKEN_FILE "/www/session_token.txt"
#define TOKEN_GENERATE "od -A n -t x -N 16 /dev/urandom | tr -dc A-Z-a-z-0-9 | head -c16 \
                        > /www/session_token.txt "
#define SALT_GENERATE "od -A n -t x -N 16 /dev/urandom | tr -dc A-Z-a-z-0-9 | head -c16 "
#define TOKEN_CLEAR "rm -rf /www/session_token.txt "
//changing permissions to restirct access to user and group only.
#define CHG_PERM_PASWD_FILE "chmod 770 /www/lighttpd.user"
//changing owner and group of file to www-data,as per security recommendations.
#define CHG_PERM_PASWD_FILE_OWNR "chown www-data:www-data /www/lighttpd.user"
#define CHG_PERM_ATTEMPT_FILE "chmod 770 /www/login_attempt"
#define CHG_PERM_ATTEMPT_FILE_OWNR "chown www-data:www-data /www/login_attempt"
#define CHG_PERM_SESSION_FILE "chmod 770 /www/qcmap_session"
#define CHG_PERM_SESSION_FILE_OWNR "chown www-data:www-data /www/qcmap_session"
#define CHG_SESSION_TOKEN_FILE "chmod 770 /www/session_token.txt"
#define CHG_SESSION_TOKEN_FILE_OWNR "chown www-data:www-data /www/session_token.txt"
#define MIN_PWD_LN 8
#define MAX_PWD_LN 15
#define MAX_COMMAND_STR_LEN 200
#define MAX_CRYPT_STR_LEN 200
#define MAX_CRYPT_SALT_LEN 20
//if there is a mismatch of token sent by browser, send failure msg.
#define TOKN_FAIL "Content-Type: text/html\nContent-Length: \
                            22\n\n{\"result\":\"TOKN_FAIL\"}"
//if password update is failing, send update error message.
#define UPDT_FAIL "Content-Type: text/html\nContent-Length: \
                    24\n\n{\"result\":\"5\", \"ip\":\"0\"}"
#define DEBUG 0
typedef struct
{
  //To store last login time
  struct timeval log_time;
  //To store last login login IP
  char ip[16];
  //To specify session timeout
  unsigned int timeout;
}loginfo;
typedef struct
{
  //To store last fail login time
  struct timeval log_time;
  //to store count of login attempts
  unsigned int attempt_count;
}attemptinfo;
void http_parser(char *http_cgi_buff,
                 char http_element_name[][MAX_ELEMENT_LENGTH],
                 char http_element_val[][MAX_ELEMENT_LENGTH],
                 int elements);
//store sha512 crypt encoded password.
char sha512_final_encryptd_paswd[MAX_CRYPT_STR_LEN];
//store sha512 salt code.
char sha512_salt_code[MAX_CRYPT_SALT_LEN];
//generate sha512 salt code 16 bytes + 3 bytes id
char* sha512_salt_generate(char* sha512_salt_code);
// encode password into sha512 format.
char* sha512_crypt(char* plain_str,const char *sha512_salt_code);

int main(void)
{
  //To parse incoming HTTP Traffic Length.
  char *http_inc_traffic_len = NULL;
  int DEFAULT_TIMEOUT  = 300;
  char DEFAULT_IP[IPV4_STRING_LEN] = {0};
  strlcpy(DEFAULT_IP, "0.0.0.0",IPV4_STRING_LEN);
  //To store Min buffer length required to read HTTP, afterwards this
  // variable can be used as temp buffer.
  int html_cgi_buff_len = 0;
  //To Generate HTTP Request.
  char cgi_html_buff[MAX_HTTP_BUFFER_SIZE]={0};
  //To store Min buffer length required to read HTTP, afterwards this
  //variable can be used as temp. buffer.
  int cgi_html_buff_len = 0;
  char html_cgi_buff[MAX_HTTP_BUFFER_SIZE]={0}; //To store HTTP Request.
  char attempt_log_buff[MAX_HTTP_BUFFER_SIZE]={0};
  FILE *fp = NULL;  //File pointer to File.
  FILE *fpe = NULL; //File pointer to session file.
  char session_token[MAX_HTTP_BUFFER_SIZE] = {0};
  // To store individual Element Names.
  char http_element_name[MAX_ELEMENT_COUNT][MAX_ELEMENT_LENGTH] = {0};
  // To Store individual Element values.
  char http_element_val[MAX_ELEMENT_COUNT][MAX_ELEMENT_LENGTH] = {0};
  //To store credentials read from user credentials file.
  char pwd[PASSWORD_LENGTH] = {0};
  //Time stamp of the system
  struct timeval sys_time;
  //To store log message read from file
  loginfo *linfo;
  //To store log message written to file
  loginfo sinfo;
  // to store login attempt information
  attemptinfo *ainfo;
  attemptinfo atinfo;
  //To get client IP address
  char remote_ip[IPV4_STRING_LEN] = {0};
  //To store return values from system calls for comparison
  int res = 0;
  int hasUpper = false;
  int hasLower = false;
  int hasDigit = false;
  int hasSpecial = false;
  int i=0;
  char *sha512_code = NULL;
  //Open file containing password to read.
  fp = fopen(PASSWORD_FILE, "r");
  //If password file opened successfully
  if (fp)
  {
    memset(pwd,0,PASSWORD_LENGTH);
    //Read contents of the password file
    fseek(fp,PASSWORD_POSITION,SEEK_SET);
    res = fread(pwd,1,(PASSWORD_LENGTH - 1),fp);
    if (res > 0)
    pwd[res] = 0;
    fclose(fp);
  }
  else
  {
    //Send failure HTTP responce
    printf("%s",FILE_OPEN_ERR);
    return FAIL;
  }
  //from the password read, obtain salt code and store
  strlcpy(sha512_salt_code,pwd,MAX_CRYPT_SALT_LEN);
  // check for  attempt file, if not there create
  memset(&atinfo, 0, sizeof(attemptinfo));

  fp = fopen(ATTEMPT_FILE, "r");
  if (fp)
  {
    //if file exist on device restarts, make sure permissions for safe side.
    //file access restrict to only owner, and group,
    //as per security recommendations.
    system(CHG_PERM_ATTEMPT_FILE);
    //change owner,group of file to www-data as per security recommendations.
    system(CHG_PERM_ATTEMPT_FILE_OWNR);
    fclose(fp);
  }
  else
  {
    fp = fopen(ATTEMPT_FILE, "w+");
    if(fp)
    {
      //file access restrict to only owner, and group,
      //as per security recommendations.
      system(CHG_PERM_ATTEMPT_FILE);
      //change owner,group of file to www-data as per security recommendations.
      system(CHG_PERM_ATTEMPT_FILE_OWNR);
      fclose(fp);
    }
    else
    {
      printf("%s",FILE_OPEN_ERR);
      return FAIL;
    }
  }
  char *remote_addr = NULL;

  http_inc_traffic_len = getenv("CONTENT_LENGTH");

  //Get clients IP address
  if (http_inc_traffic_len)
  {
    //Initialize memories to hold data.
    remote_addr = getenv("REMOTE_ADDR");
    if(remote_addr)
    {
      strlcat(remote_ip, remote_addr, IPV4_STRING_LEN);
      memset(html_cgi_buff, 0, MAX_HTTP_BUFFER_SIZE);
      memset(cgi_html_buff, 0, MAX_HTTP_BUFFER_SIZE);
      memset(&sinfo, 0, LOG_BUFF_SIZE);
      html_cgi_buff_len = atoi(http_inc_traffic_len);
      if (html_cgi_buff_len > 0)
      {
        // Avoid buffer overflow
        html_cgi_buff_len = MIN(html_cgi_buff_len,
                                sizeof(html_cgi_buff)-1 );
        //Read incoming stream data into html_cgi_buff
        if( (fread(html_cgi_buff, 1, html_cgi_buff_len, stdin )) <= 0)
        {
          printf("%s",FILE_READ_ERR);
          return FAIL;
        }
      }
      else
      {
        return FAIL;
      }
      //Send fetched data to parse.
      http_parser(html_cgi_buff, http_element_name,
                  http_element_val, MAX_ELEMENT_COUNT);
      //If request is to load from QCMAP page
      if (strncmp("load",http_element_val[0],MAX(strlen("load"),
          strlen(http_element_val[0]))) == SUCCESS)
      {
        //default password check for first time login
        //Open file containing password to read.
        fp = fopen(PASSWORD_FILE, "r");
        //If password file opened successfully
        if (fp)
        {
          memset(pwd,0,PASSWORD_LENGTH);
          //Read contents of the password file
          fseek(fp,PASSWORD_POSITION,SEEK_SET);
          res = fread(pwd,1,(PASSWORD_LENGTH - 1),fp);
          if (res > 0)
            pwd[res] = 0;
          fclose(fp);
        }
        else
        {
          //Send failure HTTP responce
          printf("%s",FILE_OPEN_ERR);
          return FAIL;
        }
        //calculate sha512 crypt of default password
        if( ( sha512_code = sha512_crypt(DEFAULT_PASSWD,sha512_salt_code)) == NULL)
        {
          //Send failure HTTP responce
          printf("%s",MD5_SUM_ERR);
          return FAIL;
        }
        // check for new modem, first login :IF
        if (strncmp(sha512_code,pwd,MAX(strlen(sha512_code),strlen(pwd))) == SUCCESS)
        {
          fpe = fopen(SESSION_TOKEN_FILE, "w+");
          if(fpe)
          {
            //generate a randon number to use it as session token
            system(TOKEN_GENERATE);
            //file access restrict to only owner,
            //and group as per security recommendations.
            system(CHG_SESSION_TOKEN_FILE);
            //change owner,group of file to www-data,
            //as per security recommendations.
            system(CHG_SESSION_TOKEN_FILE_OWNR);
            fclose(fpe);
          }
          else
          {
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
          fpe = fopen(SESSION_TOKEN_FILE, "r");
          if(fpe)
          {
            memset(session_token,0,TOKEN_SIZE + 1 );
            if( (fread(&session_token, 1, TOKEN_SIZE, fpe)) <= 0)
            {
              printf("%s",FILE_READ_ERR);
              fclose(fpe);
              return FAIL;
            }
            fclose(fpe);
          }
          else
          {
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
          // Return to  HTML page to redirect user for change password!
          printf(FIRST_LOGIN_SUCCESS,HTML_HEADER,(int)(FIRST_LGN_SUCC +
                 strlen(session_token)),session_token);
          return SUCCESS;
        }
        else
        {
          //Open file for reading previous request timestamp
          fp = fopen(SESSION_FILE, "r");
          memset(html_cgi_buff, 0, MAX_HTTP_BUFFER_SIZE);
          if (fp)
          {
            //Read previous timestamp
            res = fread(html_cgi_buff,1,LOG_BUFF_SIZE,fp);
            //Close file after reading for reopening it in write mode
            fclose(fp);
            if (res != LOG_BUFF_SIZE )
            {
              //something gone wrong,will default it starts.
              fp = fopen(SESSION_FILE, "w+");
              if(fp)
              {
                //file access restrict to only owner,
                //and group as per security recommendations.
                system(CHG_PERM_SESSION_FILE);
                //change owner,group of file to www-data,
                //as per security recommendations.
                system(CHG_PERM_SESSION_FILE_OWNR);
                // update IP to be default
                memset(&sinfo, 0, LOG_BUFF_SIZE);
                strlcat(sinfo.ip, DEFAULT_IP,IPV4_STRING_LEN);
                sinfo.log_time.tv_sec = sys_time.tv_sec;
                sinfo.log_time.tv_usec = sys_time.tv_usec;
                sinfo.timeout = DEFAULT_TIMEOUT;
                if( (fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp)) <= 0 )
                {
                  printf("%s",FILE_WRITE_ERR);
                  fclose(fp);
                  return FAIL;
                }
                fclose(fp);
              }
              else
              {
                printf("%s",FILE_OPEN_ERR);
                return FAIL;
              }
              //something gone wrong,will default it ends.
              printf("%s",FILE_READ_ERR);
              return FAIL;
            }
            linfo = (loginfo *)html_cgi_buff;
            //configure to default IP if time out has occured on load request
            //Get timestamp from system
            gettimeofday(&sys_time, 0);
            //Previous timestamp - Present timestamp if greater than timeout time
            if ((sys_time.tv_sec - linfo->log_time.tv_sec) > linfo->timeout)
            {
              fp = fopen(SESSION_FILE, "w+");
              if(fp)
              {
                //file access restrict to only owner,
                //and group as per security recommendations.
                system(CHG_PERM_SESSION_FILE);
                //change owner,group of file to www-data,
                //as per security recommendations.
                system(CHG_PERM_SESSION_FILE_OWNR);
                // update IP to be default
                memset(&sinfo, 0, LOG_BUFF_SIZE);
                strlcat(sinfo.ip, DEFAULT_IP,IPV4_STRING_LEN);
                sinfo.log_time.tv_sec = sys_time.tv_sec;
                sinfo.log_time.tv_usec = sys_time.tv_usec;
                sinfo.timeout = linfo->timeout;
                if( (fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp)) <= 0 )
                {
                  printf("%s",FILE_WRITE_ERR);
                  fclose(fp);
                  return FAIL;
                }
                fclose(fp);
              }
              else
              {
                printf("%s",FILE_OPEN_ERR);
                return FAIL;
              }
            }
            // check if any same IP is configured in session file and send token
            if ((strncmp(linfo->ip,remote_ip,strlen(linfo->ip))) == SUCCESS)
            {
              memset(session_token,0,TOKEN_SIZE + 1);
              printf(LOAD_WEBSERVER_SUCCESS,HTML_HEADER,(int)(RESPONSE_SUCCESS_SIZE+
                     strlen(linfo->ip)+strlen(session_token)),
                     linfo->ip,linfo->timeout,session_token);
            }
            else
            {
              printf(LOAD_WEBSERVER_FAIL,HTML_HEADER,(int)(RESPONSE_FAIL_SIZE+
                     strlen(linfo->ip)),linfo->ip);
            }
          }
          else
          {
            //Close File and send failure HTTP response
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
        }
      }
      //If request for login
      else if (strncmp("login",http_element_val[0],MAX(strlen("login"),
               strlen(http_element_val[0]))) == SUCCESS)
      {
        //check username
        if (strncmp(DEFAULT_USER_NAME,http_element_val[3],
            MAX(strlen(DEFAULT_USER_NAME),strlen(http_element_val[3])) != SUCCESS))
        {
          //Send failure HTTP responce
          printf("%s",USERNAME_FAIL_ERR);
          return FAIL;
        }
        //Open file for reading previous request timestamp
        fp = fopen(SESSION_FILE, "r");
        if (fp == NULL)
        {
          fpe = fopen(SESSION_FILE, "w+");
          if(fpe)
          {
            //file access restrict to only owner,
            //and group as per security recommendations.
            system(CHG_PERM_SESSION_FILE);
            //change owner,group of file to www-data,
            //as per security recommendations.
            system(CHG_PERM_SESSION_FILE_OWNR);
            // update IP to be default
            memset(&sinfo, 0, LOG_BUFF_SIZE);
            strlcat(sinfo.ip, DEFAULT_IP,IPV4_STRING_LEN);
            sinfo.log_time.tv_sec = sys_time.tv_sec;
            sinfo.log_time.tv_usec = sys_time.tv_usec;
            sinfo.timeout = DEFAULT_TIMEOUT;
            if( (fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fpe)) <= 0)
            {
              printf("%s",FILE_WRITE_ERR);
              fclose(fpe);
              return FAIL;
            }
            fclose(fpe);
            fp = fopen(SESSION_FILE, "r");
          }
          else
          {
            //Send failure HTTP responce
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
          //something gone wrong,will default it ends.
        }
        memset(html_cgi_buff, 0, MAX_HTTP_BUFFER_SIZE);
        if(fp)
        {
          //file access restrict to only owner, and group,
          //as per security recommendations.
          system(CHG_PERM_SESSION_FILE);
          //change owner,group of file to www-data,
          //as per security recommendations.
          system(CHG_PERM_SESSION_FILE_OWNR);
          fclose(fp);
          //default password check for first time login
          //Open file containing password to read.
          fp = fopen(PASSWORD_FILE, "r");
          //If password file opened successfully
          if (fp)
          {
            memset(pwd,0,PASSWORD_LENGTH);
            //Read contents of the password file
            fseek(fp,PASSWORD_POSITION,SEEK_SET);
            res = fread(pwd,1,(PASSWORD_LENGTH - 1),fp);
            if (res > 0)
              pwd[res] = 0;
            fclose(fp);
          }
          else
          {
            //Send failure HTTP responce
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
          // check for new modem, first login :IF
          //If it is very first login(like brand new modem)
          //calculate sha512 crypt of default password
          if( ( sha512_code = sha512_crypt(DEFAULT_PASSWD,sha512_salt_code)) == NULL)
          {
            //Send failure HTTP responce
            printf("%s",MD5_SUM_ERR);
            return FAIL;
          }
          if ( strncmp(pwd,sha512_code,MAX(strlen(pwd),strlen(sha512_code))) == SUCCESS)
          {
            //calculate crypt of entered password against default paswd encrypted code
            if( ( sha512_code = sha512_crypt(http_element_val[1],sha512_salt_code))
                  == NULL)
            {
              //Send failure HTTP responce
              printf("%s",MD5_SUM_ERR);
              return FAIL;
            }
            if ( strncmp(pwd,sha512_code,MAX(strlen(pwd),strlen(sha512_code))) == SUCCESS)
            {
              fpe = fopen(SESSION_TOKEN_FILE, "w+");
              if(fpe)
              {
                //generate a randon number to use it as session token.
                system(TOKEN_GENERATE);
                //file access restrict to only owner, and group,
                //as per security recommendations.
                system(CHG_SESSION_TOKEN_FILE);
                //change owner,group of file to www-data,
                //as per security recommendations.
                system(CHG_SESSION_TOKEN_FILE_OWNR);
                fclose(fpe);
              }
              else
              {
                printf("%s",FILE_OPEN_ERR);
                return FAIL;
              }
              fpe = fopen(SESSION_TOKEN_FILE, "r");
              if(fpe)
              {
                memset(session_token,0,TOKEN_SIZE + 1);
                if( (fread(&session_token, 1, TOKEN_SIZE, fpe)) <= 0)
                {
                  printf("%s",FILE_READ_ERR);
                  fclose(fpe);
                  return FAIL;
                }
                fclose(fpe);
                // Return to  HTML page to redirect user for change password!
                printf(FIRST_LOGIN_SUCCESS,HTML_HEADER,(int)(FIRST_LGN_SUCC+
                       strlen(session_token)),session_token);
                return SUCCESS;
              }
              else
              {
                //Send failure HTTP responce
                printf("%s",FILE_OPEN_ERR);
                return FAIL;
              }
            }
            else
            {
              //Send failure HTTP responce
              printf("%s",LOGIN_FAIL);
              return FAIL;
            }
          }
          else
          {
            if(unencode_string( http_element_val[1],strlen(http_element_val[1]))
               == FAIL)
            {
              printf("%s",UNENCODE_ERR);
              return FAIL;
            }
            else
            {
              //calculate sha512 crypt of entered password
              if( ( sha512_code = sha512_crypt(http_element_val[1],sha512_salt_code))
                 == NULL)
              {
                //Send failure HTTP responce
                printf("%s",MD5_SUM_ERR);
                return FAIL;
              }
              if ( (strlen(pwd) > 0) && (strncmp(pwd,sha512_code,MAX(strlen(pwd),
                   strlen(sha512_code))) == SUCCESS))
              {
                //Open file for reading previous request timestamp
                fp = fopen(SESSION_FILE, "r");
                if (fp == NULL)
                  fp = fopen(SESSION_FILE, "w+");
                memset(html_cgi_buff, 0, MAX_HTTP_BUFFER_SIZE);
                if(fp)
                {
                  //file access restrict to only owner, and group,
                  //as per security recommendations.
                  system(CHG_PERM_SESSION_FILE);
                  //change owner,group of file to www-data,
                  //as per security recommendations.
                  system(CHG_PERM_SESSION_FILE_OWNR);
                  //Read previous timestamp
                  res = fread(html_cgi_buff,1,LOG_BUFF_SIZE,fp);
                  //Close file after reading for reopening it in write mode
                  fclose(fp);
                  if (res != LOG_BUFF_SIZE)
                  {
                    //something gone wrong,will default it starts.
                    fp = fopen(SESSION_FILE, "w+");
                    if(fp)
                    {
                      //file access restrict to only owner,
                      //and group as per security recommendations.
                      system(CHG_PERM_SESSION_FILE);
                      //change owner,group of file to www-data,
                      //as per security recommendations.
                      system(CHG_PERM_SESSION_FILE_OWNR);
                      // update IP to be default
                      memset(&sinfo, 0, LOG_BUFF_SIZE);
                      strlcat(sinfo.ip, DEFAULT_IP,IPV4_STRING_LEN);
                      sinfo.log_time.tv_sec = sys_time.tv_sec;
                      sinfo.log_time.tv_usec = sys_time.tv_usec;
                      sinfo.timeout = DEFAULT_TIMEOUT;
                      if( (fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp)) <= 0)
                      {
                        printf("%s",FILE_WRITE_ERR);
                        fclose(fp);
                        return FAIL ;
                      }
                      fclose(fp);
                    }
                    else
                    {
                      printf("%s",FILE_OPEN_ERR);
                      return FAIL;
                    }
                    //something gone wrong,will default it ends.
                    printf("%s",FILE_READ_ERR);
                    return FAIL;
                  }
                  linfo = (loginfo *)html_cgi_buff;
                  //As password is matched, reset the count
                  fp = fopen(ATTEMPT_FILE, "w+");
                  if(fp)
                  {
                    //file access restrict to only owner, and group,
                    //as per security recommendations.
                    system(CHG_PERM_ATTEMPT_FILE);
                    //change owner,group of file to www-data,
                    //as per security recommendations.
                    system(CHG_PERM_ATTEMPT_FILE_OWNR);
                    atinfo.attempt_count = 0;
                    atinfo.log_time.tv_sec = 0;
                    atinfo.log_time.tv_usec = 0;
                    if( (fwrite((char *)&atinfo,1,ATTEMPT_BUFF_SIZE,fp)) <= 0)
                    {
                      printf("%s",FILE_WRITE_ERR);
                      fclose(fp);
                      return FAIL;
                    }
                    fclose(fp);
                  }
                  else
                  {
                    printf("%s",FILE_OPEN_ERR);
                    return FAIL;
                  }
                  // Update IP if no other ip is using webserver
                  if ((!strncmp(linfo->ip,remote_ip,MAX(strlen(linfo->ip),strlen(remote_ip)))) ||
                     (!strncmp(linfo->ip,DEFAULT_IP,MAX(strlen(linfo->ip),strlen(DEFAULT_IP)))))
                  {
                    // Also update timestamp
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
                      if (gettimeofday(&sys_time, 0) == SUCCESS)
                      {
                        memset(&sinfo, 0, LOG_BUFF_SIZE);
                        strlcat(sinfo.ip, remote_ip,IPV4_STRING_LEN);
                        sinfo.log_time.tv_sec = sys_time.tv_sec;
                        sinfo.log_time.tv_usec = sys_time.tv_usec;
                        sinfo.timeout = linfo->timeout;
                        fpe = fopen(SESSION_TOKEN_FILE, "w+");
                        if(fpe)
                        {
                          //generate a random number to use as session token.
                          system(TOKEN_GENERATE);
                          //file access restrict to only owner, and group,
                          //as per security recommendations.
                          system(CHG_SESSION_TOKEN_FILE);
                          //change owner,group of file to www-data
                          //as per security recommendations.
                          system(CHG_SESSION_TOKEN_FILE_OWNR);
                          fclose(fpe);
                        }
                        else
                        {
                          printf("%s",FILE_OPEN_ERR);
                          fclose(fp);
                          return FAIL;
                        }
                        fpe = fopen(SESSION_TOKEN_FILE, "r");
                        if(fpe)
                        {
                          memset(session_token,0,TOKEN_SIZE + 1 );
                          if( (fread(&session_token, 1, TOKEN_SIZE, fpe)) <= 0)
                          {
                            printf("%s",FILE_READ_ERR);
                            fclose(fpe);
                            fclose(fp);
                            return FAIL;
                          }
                          fclose(fpe);
                        }
                        else
                        {
                          printf("%s",FILE_OPEN_ERR);
                          fclose(fp);
                          return FAIL;
                        }
                        //update timeout value in structure inturn updating it in file
                        res = fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp);
                        fclose(fp);
                        if(res == LOG_BUFF_SIZE)
                        {
                          printf(LOAD_WEBSERVER_SUCCESS,HTML_HEADER,(int)(RESPONSE_SUCCESS_SIZE+
                                 strlen(linfo->ip)+TOKEN_SIZE),
                                 linfo->ip,linfo->timeout,session_token);
                        }
                        else
                        {
                          //Send failure HTTP responce
                          printf("%s",FILE_WRITE_ERR);
                          return FAIL;
                        }
                      }
                      else
                      {
                        if(fp)
                          fclose(fp);
                        printf("%s",SYSTEM_ERR);
                        return FAIL;
                      }
                    }
                    else
                    {
                      //Send failure HTTP responce
                      printf("%s",FILE_OPEN_ERR);
                      return FAIL;
                    }
                  }
                  else
                  {
                    printf("%s%d\n\n{\"result\":\"1\", \"ip\":\"%s\"}",HTML_HEADER,(int)(23+strlen(linfo->ip)),linfo->ip);
                  }
                }
                else
                {
                  //Close File and send failure HTTP response
                  printf("%s",FILE_OPEN_ERR);
                  return FAIL;
                }
              }
              else
              {
                // Increment the attempt count by 1 and copy timestamp
                fp = fopen(ATTEMPT_FILE, "r");
                if (fp)
                {
                  gettimeofday(&sys_time, 0);
                  memset(attempt_log_buff, 0, MAX_HTTP_BUFFER_SIZE);
                  if( (fread(attempt_log_buff,1,ATTEMPT_BUFF_SIZE,fp)) < 0)
                  {
                    printf("%s",FILE_READ_ERR);
                    fclose(fp);
                    return FAIL;
                  }
                  ainfo = (attemptinfo *)attempt_log_buff;
                  fclose(fp);
                  atinfo.attempt_count = (ainfo->attempt_count) + 1;
                  atinfo.log_time.tv_sec = sys_time.tv_sec;
                  atinfo.log_time.tv_usec = sys_time.tv_usec;
                }
                else
                {
                  atinfo.attempt_count = 0;
                  atinfo.log_time.tv_sec = sys_time.tv_sec;
                  atinfo.log_time.tv_usec = sys_time.tv_usec;
                }
                fp = fopen(ATTEMPT_FILE, "w+");
                if(fp)
                {
                  //file access restrict to only owner, and group,
                  //as per security recommendations.
                  system(CHG_PERM_ATTEMPT_FILE);
                  //change owner,group of file to www-data,
                  //as per security recommendations.
                  system(CHG_PERM_ATTEMPT_FILE_OWNR);
                  if( (fwrite((char*)&atinfo,1,ATTEMPT_BUFF_SIZE,fp)) <= 0)
                  {
                    printf("%s",FILE_WRITE_ERR);
                    fclose(fp);
                    return FAIL;
                  }
                  fclose(fp);
                }
                else
                {
                  printf("%s",FILE_OPEN_ERR);
                  return FAIL;
                }
                //Send password mismatch HTTP response
                printf("%s",LOGIN_FAIL);
                return FAIL;
              }
            }
          }

        }
        else
        {
          printf("%s",FILE_CRED_ERR);
          return FAIL;
        }
      }
      //If request is to update password
      else if (strncmp("update",http_element_val[0],MAX(strlen("update"),
               strlen(http_element_val[0])))== SUCCESS)
      {
        //token validate starts
        fpe = fopen(SESSION_TOKEN_FILE, "r");
        memset(session_token,0,TOKEN_SIZE + 1 );
        if(fpe)
        {
          if( (fread(&session_token, 1, TOKEN_SIZE, fpe)) <= 0)
          {
            printf("%s",FILE_READ_ERR);
            fclose(fpe);
            return FAIL;
          }
          fclose(fpe);
        }
        else
        {
          printf("%s",FILE_OPEN_ERR);
          return FAIL;
        }
        if(strncmp(http_element_val[4],session_token,strlen(session_token)))
        {
          //The current token value should be set to a
          //new random value when a mismatch is detected.
          //Then a new token issued after the re-login,
          //this will restrict hackers to restrict attempts.
          fp = fopen(SESSION_TOKEN_FILE, "w+");
          if(fp)
          {
            //generate a random number to use it as session token.
            system(TOKEN_GENERATE);
            //file access restrict to only owner,
            //and group as per security recommendations.
            system(CHG_SESSION_TOKEN_FILE);
            //change owner,group of file to www-data,
            //as per security recommendations.
            system(CHG_SESSION_TOKEN_FILE_OWNR);
            fclose(fp);
          }
          else
          {
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
          #ifdef DEBUG
          printf(PRINT_DEBUG_LOG,HTML_HEADER,(int)(RESPONSE_FAIL_SIZE+
          strlen(session_token) + strlen(http_element_val[4])),
          session_token,http_element_val[4]);
          #endif
          printf("%s",TOKN_FAIL);
          return FAIL;
        }
        //token validate ends
        //Open file containing password to read.
        fp = fopen(PASSWORD_FILE, "r");
        //If password file opened successfully
        if (fp)
        {
          memset(pwd,0,PASSWORD_LENGTH);
          //Read contents of the password file
          fseek(fp,PASSWORD_POSITION,SEEK_SET);
          res = fread(pwd,1,(PASSWORD_LENGTH - 1),fp);
          if (res > 0)
            pwd[res] = 0;
          fclose(fp);
          //Compare if given old password is valid after unencode
          if(unencode_string( http_element_val[1],strlen(http_element_val[1]))
             == FAIL)
          {
            printf("%s",UNENCODE_ERR);
            return FAIL;
          }
          else
          {
            //calculate sha512 crypt of old password
            if( ( sha512_code = sha512_crypt(http_element_val[1],sha512_salt_code)) == NULL)
            {
              //Send failure HTTP responce
              printf("%s",MD5_SUM_ERR);
              return FAIL;
            }
            if ((strlen(pwd) > 0) && (strncmp(pwd,sha512_code,MAX(strlen(pwd),
                strlen(sha512_code))) == SUCCESS ))
            {
              fp = fopen(PASSWORD_FILE, "w+");
              if (fp)
              {
                //file access restrict to only owner, and group,
                //as per security recommendations.
                system(CHG_PERM_PASWD_FILE);
                //change owner,group of file to www-data,
                //as per security recommendations.
                system(CHG_PERM_PASWD_FILE_OWNR);
                //If given old password is valid update it with new password.
                // Before update unencode new password
                if(unencode_string( http_element_val[2],strlen(http_element_val[2]))
                   == FAIL)
                {
                  printf("%s",UNENCODE_ERR);
                  //restore old password in password file.
                  //calculate sha512 crypt of old password
                  if( ( sha512_code = sha512_crypt(http_element_val[1],sha512_salt_code))
                      == NULL )
                  {
                    //Send failure HTTP responce
                    printf("%s",MD5_SUM_ERR);
                    fclose(fp);
                    return FAIL;
                  }
                  fprintf(fp, "%s:%s",DEFAULT_USER_NAME,sha512_code);
                  fclose(fp);
                  return FAIL;
                }
                 //check for password complexity, i.e should have
                 //   1. one numeric
                 //   2. one lowecase
                 //   3. one upper case
                 //   4. one special character
                 //   5. length of password shoube (8-15) inclusive.
                else if( (strlen(http_element_val[2]) >= MIN_PWD_LN ) &&  (strlen(http_element_val[2]) <= MAX_PWD_LN ) )
                {
                  char* paswd_str=http_element_val[2];
                  for( i =0; i < strlen(paswd_str); ++i )
                    {

                      if( islower(paswd_str[i]) )
                        hasLower = true;
                      if( isupper(paswd_str[i]) )
                        hasUpper = true;
                      if( isdigit(paswd_str[i]) )
                        hasDigit = true;
                      if( !(isalnum(paswd_str[i])) )
                        hasSpecial=true;
                    }
                  if( !(hasLower && hasUpper && hasDigit && hasSpecial) )
                  {
                    //Send failure HTTP responce
                    printf("%s",PASWD_COMPLEXITY_ERR);
                    //calculate sha512 crypt of old password
                    if( ( sha512_code = sha512_crypt(http_element_val[1],sha512_salt_code))
                        == NULL )
                    {
                      //Send failure HTTP responce
                      printf("%s",MD5_SUM_ERR);
                      fclose(fp);
                      return FAIL;
                    }
                    fprintf(fp, "%s:%s",DEFAULT_USER_NAME,sha512_code);
                    fclose(fp);
                    return FAIL;
                  }
                  //calculate sha512 crypt of new password
                  //need to generate a sha512 salt here
                  if(sha512_salt_generate(sha512_salt_code)== NULL)
                    {
                      //Send failure HTTP responce
                      printf("%s",MD5_SUM_ERR);
                      fclose(fp);
                      return FAIL;
                    }
                  if( ( sha512_code = sha512_crypt(http_element_val[2],sha512_salt_code))
                      == NULL)
                  {
                    //Send failure HTTP responce
                    printf("%s",MD5_SUM_ERR);
                    fclose(fp);
                    return FAIL;
                  }
                  //write sha512 crypt value into password file.
                  fprintf(fp, "%s:%s",DEFAULT_USER_NAME,sha512_code);
                  fclose(fp);
                  // Also update timeout as entered by the user
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
                    if (gettimeofday(&sys_time, 0) == SUCCESS)
                    {
                      memset(&sinfo, 0, LOG_BUFF_SIZE);
                      strlcat(sinfo.ip, remote_ip,IPV4_STRING_LEN);
                      sinfo.log_time.tv_sec = sys_time.tv_sec;
                      sinfo.log_time.tv_usec = sys_time.tv_usec;
                      sinfo.timeout = atoi(http_element_val[3]);
                      //update timeout value in structure inturn updating it in file
                      res = fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp);
                      fclose(fp);
                      if(res == LOG_BUFF_SIZE)
                      {
                        //Send Success HTTP responce
                        printf("%s",CHANGE_PWD_SUCCESS);
                      }
                      else
                      {
                        //Send failure HTTP responce
                        printf("%s",FILE_OPEN_ERR);
                        return FAIL;
                      }
                    }
                    else
                    {
                      fclose(fp);
                      printf("%s",SYSTEM_ERR);
                      return FAIL;
                    }
                  }
                  else
                  {
                    //Send failure HTTP responce
                    printf("%s",FILE_OPEN_ERR);
                    return FAIL;
                  }
                }
                else
                {
                    //Send failure HTTP responce
                    printf("%s",PASWD_COMPLEXITY_ERR);
                    //calculate sha512 crypt of old password,to restore it
                    if( ( sha512_code = sha512_crypt(http_element_val[1],sha512_salt_code))
                        == NULL )
                    {
                      //Send failure HTTP responce
                      printf("%s",MD5_SUM_ERR);
                      fclose(fp);
                      return FAIL;
                    }
                    fprintf(fp, "%s:%s",DEFAULT_USER_NAME,sha512_code);
                    fclose(fp);
                    return FAIL;
                }
              }
              else
              {
                printf("%s",FILE_OPEN_ERR);
                return FAIL;
              }
            }
            else
            {
              //Send failure HTTP responce
              printf("%s",UPDT_FAIL);
              return FAIL;
            }
          }
        }
        else
        {
          //Send failure HTTP responce
          printf("%s",FILE_OPEN_ERR);
        }
      }
      //If the request is for update time out only
      else if (strncmp("timeupdate",http_element_val[0],MAX(strlen("timeupdate"),
               strlen(http_element_val[0])))== SUCCESS)
      {
        //token validate starts
        fpe = fopen(SESSION_TOKEN_FILE, "r");
        if(fpe)
        {
          memset(session_token,0,TOKEN_SIZE + 1);
          if( (fread(&session_token, 1, TOKEN_SIZE, fpe)) <= 0)
          {
            printf("%s",FILE_READ_ERR);
            fclose(fpe);
            return FAIL;
          }
          fclose(fpe);
        }
        else
        {
          printf("%s",FILE_OPEN_ERR);
          return FAIL;
        }
        if(strncmp(http_element_val[2],session_token,strlen(session_token)))
        {
          //The current token value should be set to a,
          //new random value when a mismatch is detected.
          //Then a new token issued after the re-login,
          //this will restrict hackers to restrict attempts.
          fp = fopen(SESSION_TOKEN_FILE, "w+");
          if(fp)
          {
            //generate a random number to use it as session token.
            system(TOKEN_GENERATE);
            //file access restrict to only owner, and group,
            //as per security recommendations.
            system(CHG_SESSION_TOKEN_FILE);
            //change owner,group of file to www-data,
            //as per security recommendations.
            system(CHG_SESSION_TOKEN_FILE_OWNR);
            fclose(fp);
          }
          else
          {
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
          printf("%s",TOKN_FAIL);
          return FAIL;
        }
        //token validate ends
        //update timeout as entered by the user
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
          if (gettimeofday(&sys_time, 0) == SUCCESS)
          {
            memset(&sinfo, 0, LOG_BUFF_SIZE);
            strlcat(sinfo.ip, remote_ip,IPV4_STRING_LEN);
            sinfo.log_time.tv_sec = sys_time.tv_sec;
            sinfo.log_time.tv_usec = sys_time.tv_usec;
            sinfo.timeout = atoi(http_element_val[1]);
            //update timeout value in structure inturn updating it in file
            res = fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp);
            fclose(fp);
            if(res == LOG_BUFF_SIZE)
            {
              //Send Success HTTP responce
              printf("%s",CHANGE_PWD_SUCCESS);
              return SUCCESS;
            }
            else
            {
              //Send failure HTTP responce
              printf("%s",FILE_OPEN_ERR);
              return FAIL;
            }
          }
          else
          {
            fclose(fp);
            printf("%s",SYSTEM_ERR);
            return FAIL;
          }
        }
        else
        {
          //Send failure HTTP responce
          printf("%s",FILE_OPEN_ERR);
          return FAIL;
        }
      }
      //If the request is for logout or close session forcefully
      else if (strncmp("close",http_element_val[0],MAX(strlen("close"),
               strlen(http_element_val[0])))== SUCCESS)
      {
        //Get timestamp from system
        gettimeofday(&sys_time, 0);
        fp = fopen(SESSION_FILE, "r");
        if (fp)
        {
          //Read previous timestamp
          res = fread(html_cgi_buff,1,LOG_BUFF_SIZE,fp);
          fclose(fp);
          if (res != LOG_BUFF_SIZE )
          {
            //something gone wrong,will default it starts.
            fp = fopen(SESSION_FILE, "w+");
            if(fp)
            {
              //file access restrict to only owner,
              //and group as per security recommendations.
              system(CHG_PERM_SESSION_FILE);
              //change owner,group of file to www-data,
              //as per security recommendations.
              system(CHG_PERM_SESSION_FILE_OWNR);
              // update IP to be default
              memset(&sinfo, 0, LOG_BUFF_SIZE);
              strlcat(sinfo.ip, DEFAULT_IP,IPV4_STRING_LEN);
              sinfo.log_time.tv_sec = sys_time.tv_sec;
              sinfo.log_time.tv_usec = sys_time.tv_usec;
              sinfo.timeout = DEFAULT_TIMEOUT;
              if( (fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp)) <= 0)
              {
                printf("%s",FILE_WRITE_ERR);
                fclose(fp);
                return FAIL;
              }
              fclose(fp);
            }
            else
            {
              printf("%s",FILE_OPEN_ERR);
              return FAIL;
            }
            //something gone wrong,will default it ends.
            printf("%s",FILE_READ_ERR);
            return FAIL;
          }
          linfo = (loginfo *)html_cgi_buff;
          // configure default ip in session file
          memset(&sinfo, 0, LOG_BUFF_SIZE);
          strlcat(sinfo.ip, DEFAULT_IP,IPV4_STRING_LEN);
          sinfo.log_time.tv_sec = sys_time.tv_sec;
          sinfo.log_time.tv_usec = sys_time.tv_usec;
          sinfo.timeout = linfo->timeout;
          fp = fopen(SESSION_FILE, "w+");
          if(fp)
          {
            //file access restrict to only owner, and group,
            //as per security recommendations.
            system(CHG_PERM_SESSION_FILE);
            //change owner,group of file to www-data,
            //as per security recommendations.
            system(CHG_PERM_SESSION_FILE_OWNR);
            if( (fwrite((char *)&sinfo,1,LOG_BUFF_SIZE,fp)) <= 0)
            {
              printf("%s",FILE_WRITE_ERR);
              fclose(fp);
              return FAIL;
            }
            fclose(fp);
            //remove token file.
            system(TOKEN_CLEAR);
            printf("%s",CHANGE_PWD_SUCCESS);
          }
          else
          {
            //Send failure HTTP responce
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
        }
        else
        {
          //Send failure HTTP responce
          printf("%s",FILE_OPEN_ERR);
          return FAIL;
        }
      }
      // If the request is to check attempt count on login page load
      else if (strncmp("count_attempt",http_element_val[0],MAX(strlen("count_attempt"),
               strlen(http_element_val[0])))== SUCCESS)
      {
        gettimeofday(&sys_time, 0);
        // Request is to check number of attempts and timestamp
        if(atoi(http_element_val[1]) == CHECK_ATTEMPT)
        {
          // read attempt count from file
          fp = fopen(ATTEMPT_FILE, "r");
          if (fp)
          {
            memset(attempt_log_buff, 0, MAX_HTTP_BUFFER_SIZE);
            if( (fread(attempt_log_buff,1,ATTEMPT_BUFF_SIZE,fp)) < 0)
            {
              printf("%s",FILE_READ_ERR);
              fclose(fp);
              return FAIL;
            }
            ainfo = (attemptinfo *)attempt_log_buff;
            fclose(fp);
            if((sys_time.tv_sec - ainfo->log_time.tv_sec )< LOGIN_DISABLE_DURATION)
            {
              if(ainfo->attempt_count > MAX_LOGIN_ATTEMPT )
                printf("%s",MAX_ATTEMPT);
              else
                printf("%s",LOGIN_ENABLE);
            }
            else
            {
              fp = fopen(ATTEMPT_FILE, "w+");
              if(fp)
              {
                //file access restrict to only owner, and group,
                //as per security recommendations.
                system(CHG_PERM_ATTEMPT_FILE);
                //change owner,group of file to www-data,
                //as per security recommendations.
                system(CHG_PERM_ATTEMPT_FILE_OWNR);
                atinfo.attempt_count = 0;
                atinfo.log_time.tv_sec = 0;
                atinfo.log_time.tv_usec = 0;
                if( (fwrite(&atinfo,1,ATTEMPT_BUFF_SIZE,fp)) <= 0)
                {
                  printf("%s",FILE_WRITE_ERR);
                  fclose(fp);
                  return FAIL;
                }
                fclose(fp);
              }
              else
              {
                printf("%s",FILE_OPEN_ERR);
                return FAIL;
              }
              printf("%s",LOGIN_ENABLE);
            }
          }
          else
          {
            //Send failure HTTP responce
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
        }
        else if (atoi(http_element_val[1]) == CLEAR_ATTEMPT)
        {
          // reset the count to 0
          fp = fopen(ATTEMPT_FILE, "w+");
          if(fp)
          {
            //file access restrict to only owner, and group,
            //as per security recommendations.
            system(CHG_PERM_ATTEMPT_FILE);
            //change owner,group of file to www-data,
            //as per security recommendations.
            system(CHG_PERM_ATTEMPT_FILE_OWNR);
            atinfo.attempt_count = 0;
            atinfo.log_time.tv_sec = 0;
            atinfo.log_time.tv_usec = 0;
            if( (fwrite(&atinfo,1,ATTEMPT_BUFF_SIZE,fp)) <= 0)
            {
              printf("%s",FILE_WRITE_ERR);
              fclose(fp);
              return FAIL;
            }
            fclose(fp);
          }
          else
          {
            printf("%s",FILE_OPEN_ERR);
            return FAIL;
          }
          printf("%s",LOGIN_ENABLE);
        }
      }
    }
    else
    {
      //Send HTTP responce reporting broken message
      printf("%s",GOT_NOTHING);
    }
  }
  else
  {
    //Send HTTP responce reporting broken message
    printf("%s",GOT_NOTHING);
  }
  return SUCCESS;
}
/*===========================================================================
  FUNCTION http_parser
===========================================================================*/
/*!
@brief
  This function parses data fetched from HTTP post message.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void http_parser(char *http_cgi_buff,
                 char http_element_name[][MAX_ELEMENT_LENGTH],
                 char http_element_val[][MAX_ELEMENT_LENGTH],
                 int elements)
{
  int i = 0, j = 0, k = 0;
  int len = 0;

  len = strlen(http_cgi_buff);
  i = 0;
  j = 0;
  if (len > 0)
  {
    //Parse till end of string and till it reaches max. element count
    while((k < len) && (i < elements))
    {
      j = 0;
      //Read till we reach delimiter to seperate value and element
      while((http_cgi_buff[k] != '=') && (k < len) && (j < MAX_ELEMENT_LENGTH))
      {
        http_element_name[i][j] = http_cgi_buff[k];
        k++;
        j++;
      }
      j = 0;
      k++;
      //Read till we reach delimiter to seperate modules
      while((http_cgi_buff[k] != '&') && (k < len) && (j < MAX_ELEMENT_LENGTH))
      {
        http_element_val[i][j] = http_cgi_buff[k];
        k++;
        j++;
      }
      k++;
      i++;
    }
  }
}
/*===========================================================================
  FUNCTION unencode_string
===========================================================================*/
/*!
@brief
  parses HTML code.

@input
  values             - cgi form field values.

@return
 1  - success
 -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
int unencode_string
(
  char *src,
  int last
)
{
  char* tmp=NULL;
  char hexs[3];
  int i=0,j=0;
  int ascii=0;
   tmp=(char*) malloc(last);
   if(tmp)
   {
     for(i=0,j=0; ( (i <= last ) && (src[i]) );)
     {
       //look for % special char which indicates html chars needs to be url encoded!!
       if( (src[i] == '%') )
       {
         //take next 2 char's into a hex string
         hexs[0]= src[i+1];
         hexs[1]= src[i+2];
         hexs[2]= '\0';
         //convert it into int from hex string.
         ascii= (int)strtol(hexs, NULL, 16);
         //convert int into character and store it. this is actual char.
         tmp[j]=(char) ascii;
         i += 3;
         j++;
       }
       else
       {
         //for other chars store as it is.
         tmp[j]=src[i];
         i++;
         j++;
       }
     }
     tmp[j]='\0';
     //copy transalated string into source string so that called fun can utilize.
     memcpy(src, tmp, strlen(tmp)+1);
     //free memory
     free(tmp);
     //set null to pointer.
     tmp=NULL;
     return SUCCESS;
   }
   else
   {
     return FAIL;
   }
 }
/*===========================================================================
  FUNCTION sha512_crypt
===========================================================================*/
/*!
@brief
  calculate sha512 crypt for plain text.

@input
  plain_str          - string to be encrypted.
  sha512_salt_code   - salt code including id.

@return
  on success  - returns sha512 crypt of given string
  on failure  - return null.

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
char* sha512_crypt(char* plain_str,const char *sha512_salt_code)
{
  FILE *cmd;
  char *sha512_encrypted_paswd=NULL;
  memset(sha512_final_encryptd_paswd, 0, MAX_CRYPT_STR_LEN);
  /* Create the sha512 crypt of plain_str */
  sha512_encrypted_paswd=crypt(plain_str, sha512_salt_code);
  if((*sha512_encrypted_paswd ==EINVAL )|| (*sha512_encrypted_paswd ==ENOSYS) ||
     (*sha512_encrypted_paswd ==EPERM) )
  {
    return NULL;
  }
  else
  {
    #ifdef DEBUG
    printf(PRINT_DEBUG_LOG_CRYPT,HTML_HEADER,(int)(RESPONSE_FAIL_SIZE+
    strlen(sha512_salt_code) + strlen(sha512_encrypted_paswd)),
    sha512_salt_code,sha512_encrypted_paswd);
    #endif
    strlcpy(sha512_final_encryptd_paswd,sha512_encrypted_paswd,MAX_CRYPT_STR_LEN);
    return sha512_final_encryptd_paswd;
  }
}// char* sha512_crypt(char* plain_str)

/*===========================================================================
  FUNCTION sha512_salt_generate
===========================================================================*/
/*!
@brief
  calculate sha512 salt code for plain text.

@input
  sha512_salt_code   - string to store salt code.

@return
  on success  - returns sha512 salt of given string
  on failure  - return null.

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
char* sha512_salt_generate(char* sha512_salt_code)
{
  FILE *cmd;
  char *sha512_encrypted_paswd=NULL;
  /* Create the sha512 crypt of plain_str */
  memset(sha512_salt_code, 0, MAX_CRYPT_SALT_LEN);
  strlcpy(sha512_salt_code, "$6$",MAX_CRYPT_SALT_LEN);

  //generate 16bit urandom code command
  cmd = popen(SALT_GENERATE, "r");
  if(cmd == NULL)
    return NULL;
  //store urandom 16bit code calculated value into buffer.
  fgets(&sha512_salt_code[3], MAX_CRYPT_SALT_LEN-3, cmd);
  if( ( pclose(cmd) ) == -1 )
    return NULL;
  // null terminate so, that it only have id and 16 bit salt code.
  sha512_salt_code[19]=0;
  #ifdef DEBUG
    printf(PRINT_DEBUG_LOG_CRYPT,HTML_HEADER,(int)(RESPONSE_FAIL_SIZE+
    strlen(sha512_salt_code) + strlen(sha512_salt_code)),
    sha512_salt_code,sha512_salt_code);
  #endif
  return sha512_salt_code;
}// char* sha512_salt_generate(char* sha512_salt_code)
