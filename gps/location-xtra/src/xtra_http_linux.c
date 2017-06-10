/******************************************************************************
  @file:  xtra_http_linux.c
  @brief: implementation of http function on Linux

  DESCRIPTION

  XTRA Daemon

  -----------------------------------------------------------------------------
  Copyright (c) 2013-2014 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

#include <xtra.h>
#include "log_util.h"
#include "platform_lib_includes.h"

#define EXPECTED_HTTP_RESPONSE_HEADER_LEN 1024

/*======
FUNCTION parseURL

DESCRIPTION
    Parse HTTP request URL
    [http://]<host>[:port]/[url_path]

RETURN VALUE
   char* aHost,            [out] host name or IP as text
   char* aPath,            [out] full path to the file
   unsigned short* aPort   [out] port number

DEPENDENCIES

======*/

static int parseURL(
   const char* aUrl,        /* [in] full URL */
   char* aHost,            /* [out] host name or IP as text */
   char* aPath,            /* [out] full path to the file  */
   unsigned short* aPort   /* [out] port number */
   )
{
   const char * p = 0;
   char * p1 = 0;
   int i = 0;

   *aHost = *aPath = '\0';
   *aPort = 80;

   p = aUrl;
   while(*p == ' ' || *p == '\t') p++;

   if(memcmp(p, "http://", 7) == 0)
   {
      p += 7;
   }

   while(*p == ' ' || *p == '\t') p++;

   p1 = aHost;
   i = 0;
   while(*p && (*p != ':' && *p != '/'))
   {
      if(i < XTRA_HTTP_HOST_MAX_LEN)
      {
         *p1++ = *p;
         *p1 = '\0';
         i++;
      }
      p++;
   }

   if(*p == ':')
   {
      p++;
      if(*p >= '0' && *p <= '9')
      {
         *aPort = strtoll(p, 0, 10);

         while(*p >= '0' && *p <= '9') p++;
      }
   }
   else if(*p == '/')
   {
      while(*p == '/') p++;
   }

   p1 = aPath;
   i = 0;
   while(*p)
   {
      if(i < XTRA_HTTP_PATH_MAX_LEN)
      {
         *p1++ = *p;
         *p1 = '\0';
         i++;
      }
      p++;
   }

   return strlen(aHost) > 0;
}

/*======
FUNCTION makeHTTPRequest

DESCRIPTION
   Make HTTP GET request

RETURN VALUE
   char* aHTTPRequest

DEPENDENCIES

======*/


static int makeHTTPRequest(
   const char* aHost,            /* [in] host name or IP as text */
   const char* aPath,            /* [in] full path to the file  */
   char* aHTTPRequest,
   const char * user_agent_string
   )
{
   memset(aHTTPRequest, 0, XTRA_HTTP_HTTPREQ_MAX_LEN);
   snprintf(aHTTPRequest, XTRA_HTTP_HTTPREQ_MAX_LEN,
      "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n", aPath, aHost, user_agent_string);

   return strlen(aHTTPRequest);
}
/*======
FUNCTION resetSocket

DESCRIPTION
   Close socket

RETURN VALUE
   None

DEPENDENCIES

======*/
static void resetSocket(int socket)
{
   if(socket != -1)
   {
      close(socket);
   }
}

/* ------------------------------------------------------------------------------------------------------ */
/*
    Writes error
*/
#define MAKE_ERROR_TEXT(buff, len, error) strlcpy(buff, error, len)

/*======
FUNCTION readDateTime

DESCRIPTION
   Reads date and time from HTTP response

RETURN VALUE
   struct tm * aDT

DEPENDENCIES

======*/

static int readDateTime(char* aRespBuff, struct tm * aDT)
{
   static char* s_months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
   char* _begin = 0;

   if(aRespBuff == 0)
   {
      return 0;
   }

   if(aDT != 0)
   {
      memset(aDT, 0, sizeof(struct tm));
   }

   _begin = strstr((char*)aRespBuff, "Date");

   if(_begin != NULL)
   {
      char date_buff[64];
      char* p = 0;
      struct tm _tmdt;
      int i = 0;

      _begin += 4;
      while(*_begin == ':' || *_begin == ' ' || *_begin == '\t') _begin++;

      p = date_buff;

      memset(date_buff, 0, 64);

      while(*_begin && *_begin != '\r' && *_begin != '\n')
      {
         *p++ = *_begin++;
      }
      *p = 0;

      memset(&_tmdt, 0, sizeof(struct tm));

      // Sun, 06 Nov 1994 08:49:37 GMT
      p = date_buff;
      while(*p && (*p < '0' || *p > '9')) p++;
      _tmdt.tm_mday = atoi(p);

      while((*p >= '0' && *p <= '9') || *p == ' ') p++;

      for(i = 0; i < 12; i++)
      {
         if(memcmp(p, s_months[i], 3) == 0)
         {
            _tmdt.tm_mon = i;
            break;
         }
      }

      while(*p && (*p < '0' || *p > '9')) p++;

      _tmdt.tm_year = atoi(p) - 1900;

      while(*p && *p != ' ') p++;
      while(*p == ' ') p++;

      _tmdt.tm_hour = atoi(p);
      p += 3;
      _tmdt.tm_min = atoi(p);
      p += 3;
      _tmdt.tm_sec = atoi(p);

      if(_tmdt.tm_year > 100 && _tmdt.tm_year < 200 &&
      _tmdt.tm_mon >= 0 && _tmdt.tm_mon <= 11 &&
      _tmdt.tm_mday >= 1 && _tmdt.tm_mday <= 31 &&
      _tmdt.tm_hour >= 0 && _tmdt.tm_hour <= 24 &&
      _tmdt.tm_min >= 0 && _tmdt.tm_min <= 60 &&
      _tmdt.tm_sec >= 0 && _tmdt.tm_sec <= 60)
      {
         if(aDT != 0)
         {
             memcpy(aDT, &_tmdt, sizeof(_tmdt));
         }
         return 1;
      }
   }
   return 0;
}

/*======
FUNCTION processHTTPResponse

DESCRIPTION
    Parses HTTP response: extracts HTTP server's date and time
    and cuts of HTTP response header from aRespBuffer,
    returns length of the response body

RETURN VALUE

DEPENDENCIES

======*/
static unsigned processHTTPResponse(
   char* aRespBuffer,
   unsigned aLength,
   struct tm * aDT,
   char* aErrorBuf)
{
   unsigned _body_length = 0;
   char _error_message[128];

   _error_message[0] = 0;

   if(aRespBuffer != 0 && aLength > 0)
   {
      int idx = 0;
      const char* p = aRespBuffer;
      unsigned _version_major = 0,
           _version_minor = 0,
           _return_code = 0;

      char _error_message[128];

     // char* _begin_data = 0;


      // Parse HTTP first line

      // skip spaces
      while(*p == ' ' || *p == '\t') p++;

      if(memcmp(p, "HTTP/", 5) == 0)
      {
         char tmp[210];
         p += 5;
         tmp[0] = *p;
         if(isdigit(tmp[0]))
         {
            tmp[1] = 0;
            _version_major = atoi(tmp);

            p++;
            if(*p == '.')
            {
               p++;
               tmp[0] = *p;
               if(isdigit(tmp[0]))
               {
                  p++;
                  if(isdigit(*p))
                  {
                     tmp[1] = *p;
                     p++;
                     tmp[2] = 0;
                  }
                  _version_minor = atoi(tmp);
               }
               p++;
            }

            while(*p == ' ' || *p == '\t') p++;

            if(isdigit(*p))
            {
               idx = 0;
               while(isdigit(*p) && idx < 200)
               {
                  tmp[idx] = *p;
                  p++;
                  idx++;
               }

               if(idx < 200)
               {
                  tmp[idx] = 0;
               }

               _return_code = atoi(tmp);
            }

            while(*p == ' ' || *p == '\t') p++;
            idx = 0;
            tmp[0] = 0;
            while(*p && *p != '\r' && *p != '\n' && idx < 200)
            {
               tmp[idx] = *p;
               idx++;
               p++;
            }

            if(idx < 200)
            {
               tmp[idx] = 0;
            }

            if(strlen(tmp) > 0)
            {
               strlcpy(_error_message, tmp, 128);
            }
         }
         else
         {
            strlcpy(_error_message, "Wronf HTTP header: version not found.",sizeof(_error_message));
         }
      } // if(memcmp(p, "HTTP/", 5) == 0)
      else
      {
         strlcpy(_error_message, "Not found 'HTTP/' in the header.",sizeof(_error_message));
      }

      if(_return_code == 200)
      {
         if(aDT)
            readDateTime(aRespBuffer, aDT);

         p = strstr(aRespBuffer,"Length:");

         if(p)
         {
            _body_length = atoi(p+8); //shift to digital
         }
      }
   }

   if(strlen(_error_message) > 0 && aErrorBuf != 0)
   {
      strlcpy(aErrorBuf, _error_message,sizeof(aErrorBuf));
   }

   return _body_length;
}


/*!
 *  \fn DWORD FRM_HttpGet(const char* aUrl, char* dataout,DWORD dwBufSize)
 *
 *   Reads file from URL to memory buffer
 *
 *  \param url url to file to get
 *  \param dataout pointer to output buffer
 *  \param dwBufSize output buffer size
 *  \return Length of the buffer & the pointer of malloc buffer. Returns 0 in case of failure.
 */



/* ------------------------------------------------------------------------------------------------------ */
unsigned char* Xtra_HttpGet(
    const char* aUrl,
    const char * user_agent_string,
    DWORD *data_len    )
{

   static char func_name[] = "XTRA_HttpGet(): ";
   char host[XTRA_HTTP_HOST_MAX_LEN];
   char path[XTRA_HTTP_PATH_MAX_LEN];
   unsigned short port = 80;
   char httpRequest[XTRA_HTTP_HTTPREQ_MAX_LEN];
   char header[EXPECTED_HTTP_RESPONSE_HEADER_LEN] = {0};
//   char *ptr;
   char ch = 0;
   int idx = 0;
   int _soc = -1;
   char *dataout = 0;
   struct hostent* _remote_host = 0;
   char* _ip = NULL;

   int _bytes_sent = 0;

   int _bytes_recv = -1;
   unsigned _offset = 0;

   //unsigned _data_length = 0;

   unsigned _remote_file_length = 0;

   struct tm _date_time;

   char ip_buff[32];

   struct sockaddr_in _addr;

   int bGetHeader = 0;

   //clear length first
   *data_len = 0;

   memset(host, 0, XTRA_HTTP_HOST_MAX_LEN);
   memset(path, 0, XTRA_HTTP_PATH_MAX_LEN);
   memset(httpRequest, 0, XTRA_HTTP_HTTPREQ_MAX_LEN);

   memset(&_date_time, 0, sizeof(_date_time));
   memset(ip_buff, 0, 32);

   memset(&_addr, 0, sizeof(_addr));

   parseURL(aUrl, host, path, &port);

   makeHTTPRequest(host, path, httpRequest,user_agent_string);

   // Create a SOCKET for connecting to server
   _soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(_soc == -1)
   {
   XTRA_TRACE_ERROR( "%s Failed to create _soc, errno=%d: %s",
   func_name, errno, strerror(errno));
   return FAIL;
   }

   // Bind the _soc to the local IP address and port 55555
   _remote_host = 0;
   //_ip = 0; No need to do this here. Causes error in klocwork

   if(isdigit(host[0]))
   {
      unsigned long _addr = inet_addr(host);
      if(_addr == INADDR_NONE)
      {
         resetSocket(_soc);

         XTRA_TRACE_ERROR(  "%s Failed to get inet addres of %s, errno=%d: %s",
         func_name, host, errno, strerror(errno));
         return FAIL;
      }

      // because it may take a long time to access DNS
      //_remote_host = gethostbyaddr((char *)&_addr, 4, AF_INET);

      _remote_host = 0;
   }
   else
   {
      _remote_host = gethostbyname((const char*)&(host[0]));

      if(_remote_host == 0)
      {
         //ss << "gethostbyname for Host '" << _host_name << "' returned NULL";
         resetSocket(_soc);

         XTRA_TRACE_ERROR("%s Failed to get host by name of %s, errno=%d: %s",
         func_name, host, errno, strerror(errno));
         return FAIL;
      }
   }

   if(_remote_host != 0)
   {
      unsigned char* IP = 0;

      IP = (unsigned char*)(_remote_host->h_addr_list[0]);
      snprintf(ip_buff, 32, "%d.%d.%d.%d", IP[0], IP[1], IP[2], IP[3]);
      XTRA_TRACE("IP: %s\n", ip_buff);

   //    _ip = inet_ntoa(*(struct in_addr *)(_remote_host->h_addr_list));
   }
   else if(isdigit(host[0]))
   {
      strlcpy(ip_buff, host, 32);
   }

   _ip = ip_buff;

   memset(&_addr, 0, sizeof(_addr));
   _addr.sin_family = AF_INET;
   _addr.sin_addr.s_addr = inet_addr(_ip);
   _addr.sin_port = htons(port);

   // Connect to server.
   if(connect(_soc, (struct sockaddr*)&_addr, sizeof(_addr)) == -1)
   {
      resetSocket(_soc);

      XTRA_TRACE_ERROR( "%s Failed to connect host %s port %d, errno=%d: %s",
            func_name, host, port, errno, strerror(errno));
      return FAIL;
   }

   XTRA_TRACE("Connect Succeeded!\n");

   _bytes_sent = send(_soc, (const char*)httpRequest, strlen(httpRequest), 0);

   XTRA_TRACE("Bytes Sent = %d\n", _bytes_sent);

   if(_bytes_sent < 0)
   {
      resetSocket(_soc);

      XTRA_TRACE( "%s Failed to send request to host %s port %d, errno=%d: %s",
            func_name, host, port, errno, strerror(errno));
   return FAIL;
   }

   XTRA_TRACE("HTTP Request Sent = \n");
   XTRA_TRACE("%s\n",httpRequest);

   _bytes_recv = -1;
   _offset = 0;


   //read header first?
   while(idx < EXPECTED_HTTP_RESPONSE_HEADER_LEN)
   {
      if(recv(_soc, &ch, 1, 0) <= 0)
         break;
      header[idx] = ch;
      if(strstr(header,"\r\n\r\n"))
      {
         bGetHeader = 1;
         break;
      }
      idx++;
   }
   XTRA_TRACE("Header = \n%s\n",header);
   XTRA_TRACE("Header Index = %d\n",idx);
   if(bGetHeader && idx < EXPECTED_HTTP_RESPONSE_HEADER_LEN) //response is normal then pass to process HTTPResponse
   {

      XTRA_TRACE("Header OK\n");

      _remote_file_length = processHTTPResponse(header,strlen(header),NULL,NULL);

      XTRA_TRACE("Remote File Length = %d\n",_remote_file_length);

      /*limit size to 1 MB */
      if((_remote_file_length < XTRA_MAX_REMOTE_FILE_SIZE)&&( _remote_file_length > 0 ) )
         dataout = (char *) malloc(_remote_file_length);
      else
      {
         XTRA_TRACE("remote file size is zero\n");
         resetSocket(_soc);
         return FAIL;
      }

      if(!dataout)
      {
      XTRA_TRACE("cannot malloc buffer size %d\n",*data_len);
      *data_len = 0;
      resetSocket(_soc);
      return FAIL;
      }

      while(_offset < _remote_file_length)
      {
         int _bytes_to_recv = (_remote_file_length - _offset);

         _bytes_recv = recv(_soc, (char*)(dataout + _offset), _bytes_to_recv, 0);
         XTRA_TRACE(" Bytes received = %d\n",_bytes_recv);

         if(_bytes_recv == 0 )
         {
            XTRA_TRACE_ERROR("Client: Connection Closed.\n");
            break;
         }

         if(_bytes_recv < 0)
         {
            resetSocket(_soc);

            XTRA_TRACE_ERROR( "%s Failed to receive data from host %s port %d, errno=%d: %s",
                  func_name, host, port, errno, strerror(errno));

            free(dataout);
            *data_len = 0;
            dataout = 0;

            return FAIL;
         }

         _offset += _bytes_recv;
      }

      //free buffer if receive buffer doesn't match size specified in response
      if(_offset != _remote_file_length)
      {
         *data_len = 0;
         free(dataout);
         dataout = 0;
         XTRA_TRACE_ERROR("Recieve buffer does not match size specified in response \n");
      }
      else
         *data_len = _offset;

      resetSocket(_soc);
      return (unsigned char*)dataout;
   }
   resetSocket(_soc);
   return 0;
}




