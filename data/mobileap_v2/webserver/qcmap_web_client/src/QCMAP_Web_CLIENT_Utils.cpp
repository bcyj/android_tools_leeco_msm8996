/*===========================================================================

                        QCMAP_WEB_CLIENT_UTILS.CPP

DESCRIPTION

  Utilities functions used for WEB CLIENT functionality.

EXTERNALIZED FUNCTIONS


  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
/*===========================================================================

when        who    what, where, why
--------    ---    ----------------------------------------------------------
01/08/14    rk     Webserver paswd encryption.
11/22/13    rk     Webserver security flaws fixed.
23/04/13    rk     Created module
===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/
#include "QCMAP_Web_CLIENT_Utils.h"

#ifdef __cplusplus
  extern "C" {
#endif

extern char* QMI_Error_String[];
extern char* firewall_string[];
extern qmi_error_type_v01 qmi_error;
/*===========================================================================
    FUNCTION create_error
  ==========================================================================*/
  /*!
  @brief
    Error response message to CGI is buit.

  @params
    values - values buff where response is stored.

  @return
    buf - jason format is created in buf.
    qmi_error - qmi error code.
  @Dependencies
    - None

  @Side Effects
    - None
  */
/*=========================================================================*/
static void create_error
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  qmi_error_type_v01  qmi_error
)
{
  char tmp[MAX_BUF_LEN] ={0};
  LOG_MSG_ERROR("\npage returns, Error: 0x%x", qmi_error, 0, 0);
  memset(tmp,0,MAX_BUF_LEN);
  strncat(buf,"{",1);
  snprintf(tmp,(strlen(values[0]) + strlen(PAGE) +1),PAGE,values[0]);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp,strlen(QMI_Error_String[qmi_error]) + strlen(RESULT_NO_COMMA)+1,
           RESULT_NO_COMMA, QMI_Error_String[qmi_error]);
  strncat(buf,tmp,strlen(tmp));

  strncat(buf,"}",1);
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
}
/*===========================================================================
  FUNCTION create_firewall_buf
==========================================================================*/
/*!
@brief
  Appends Firewall Entries in buffer.

@params
  values - cgi form field values.
  buf - message to be sent to web cgi.
@return
  - None

@Dependencies
  - None

@Side Effects
  - None
*/
/*=========================================================================*/
static void create_firewall_buf
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf
)
{
  char tmp[MAX_BUF_LEN] ={0};
  int index=0;
  strncat(buf,"{",1);
  for(; index<FIREWALL_SETTINGS_PAGE_MAX-5; index++)
  {
    snprintf(tmp,strlen(firewall_string[index])+ strlen(values[index+4]) +
             strlen(JSN_FMT_CMA),
             JSN_FMT_CMA,firewall_string[index],values[index+4]);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
  }
  snprintf(tmp,((strlen(firewall_string[index]))+ (strlen(values[index+4])))+
           strlen(JSN_FMT_NCMA),
           JSN_FMT_NCMA,firewall_string[index],values[index+4]);
  strncat(buf,tmp,strlen(tmp));
}

/*===========================================================================
    FUNCTION create_response_buf
==========================================================================*/
  /*!
  @brief
    response message to CGI is buit.

  @params
    res_result      - output buff where response is stored.
    buf             - jason format is created in buf.
    max_res_cnt     - count of response strings.
    page_string     - string values for each response string.
    start_index     - index at which need pick response values.
    mask_braces     - mask flag for avoiding braces in json response.
  @return
  - None


  @Dependencies
    - None

  @Side Effects
    - None
  */
/*=========================================================================*/
static void create_response_buf
(
  char res_result[][MAX_ELEMENT_LENGTH],
  char* buf,
  int max_res_cnt,
  char** page_string,
  int index,
  int mask_braces
)
{
  char tmp[MAX_BUF_LEN] ={0};
  if(mask_braces != 1 )
  {
    strncat(buf,"{",1);
  }
  for(; index<max_res_cnt-1; index++)
  {
    snprintf(tmp,strlen(page_string[index]) + strlen(res_result[index])+
             strlen(JSN_FMT_CMA)+1,
             JSN_FMT_CMA,page_string[index],res_result[index]);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
  }
  snprintf(tmp,strlen(page_string[index]) + strlen(res_result[index]) +
           strlen(JSN_FMT_NCMA)+1,
           JSN_FMT_NCMA,page_string[index],res_result[index]);
  strncat(buf,tmp,strlen(tmp));
  if( mask_braces != 1)
  {
    strncat(buf,"}",1);
  }
}

/*===========================================================================
  FUNCTION readable_ip_to_system_ip
==========================================================================*/
/*!
@brief
  Converts dot format IP to System IP.

@params
  string - IP address in string format.

@return
  uint32_t - system converted IP address

@Dependencies
  - None

@Side Effects
  - None
*/
/*=========================================================================*/
static uint32_t readable_ip_to_system_ip
(
  char* string
)
{
    uint32_t system_ip=0;
    in_addr addr;
    memset(&addr,0,sizeof(in_addr));
    if ( !(inet_aton(string, &addr) <=0 ))
    {
      system_ip = ntohl(addr.s_addr);
    }
    else
    {
      LOG_MSG_ERROR("Not in presentation format!!  error :%d \n", errno, 0, 0);
      system_ip=0;
    }
    return system_ip;
}
/*===========================================================================
  FUNCTION system_ip_to_readable_ip
===========================================================================*/
/*!
@brief
  converts a numeric address into a text string suitable
  for presentation

@input
  domain - identifies ipv4 or ipv6 domain
  addr   - contains the numeric address
  str   - this is an ouput value contains address in text string

@return
  0  - success
  -1 - failure

@dependencies
  It depends on inet_ntop()

@sideefects
  None
*/
 /*=========================================================================*/
static int system_ip_to_readable_ip
(
  int domain,
  uint32 *addr,
  char *str
)
{
  if((addr!=NULL) && (str!=NULL))
  {
    if (inet_ntop(domain, (const char *)addr, str, INET6_ADDRSTRLEN) == NULL)
    {
      LOG_MSG_ERROR("Not in presentation format!!  error :%d \n", errno, 0, 0);
      return QCMAP_CM_ERROR;
    }
    else
      return QCMAP_CM_SUCCESS;
  }
}
/*===========================================================================
  FUNCTION store_lanconfig_res
===========================================================================*/
/*!
@brief
  lan config response is stored.

@input
  values             - cgi form field values.
  wlan_status           wlan enable/disabled.
  wlan_mode          - wlan mode.
  gap_profile         guest ap profile type.
  i                   index to store buff.
  lan_config          - bridge/lan config of mobileap.
  station_config     - station mode config of mobileap.

@return
None

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static void store_lanconfig_res
(
  char values[][MAX_ELEMENT_LENGTH],
  int wlan_status,
  int wlan_mode,
  int gap_profile,
  int *i,
  qcmap_msgr_lan_config_v01 *lan_config,
  qcmap_msgr_station_mode_config_v01 *station_config
)
{
  int flag=0,temp_size=0;
  in_addr addr;
  /* store response message in values buffer */
    addr.s_addr = ntohl(lan_config->gw_ip);
  snprintf(values[(*i)++], strlen(inet_ntoa(addr)) +1, "%s", inet_ntoa(addr)) ;
    addr.s_addr = ntohl(lan_config->netmask);
  snprintf(values[(*i)++], strlen( inet_ntoa(addr)) +1, "%s", inet_ntoa(addr)) ;
  flag=lan_config->enable_dhcp;
  snprintf(values[(*i)++], sizeof(flag), "%d", flag) ;
    addr.s_addr = ntohl(lan_config->dhcp_config.dhcp_start_ip);
  snprintf(values[(*i)++], strlen(inet_ntoa(addr))+1, "%s", inet_ntoa(addr)) ;
    addr.s_addr = ntohl(lan_config->dhcp_config.dhcp_end_ip);
  snprintf(values[(*i)++],
      strlen(inet_ntoa(addr))+1, "%s", inet_ntoa(addr)) ;

  flag=lan_config->dhcp_config.lease_time;
  //to get no.of digits in a given number!!
  temp_size = log10(flag)+ PADDING;
  snprintf(values[(*i)++], temp_size, "%d", flag) ;

  snprintf(values[(*i)++], sizeof(wlan_status), "%d", wlan_status) ;
  snprintf(values[(*i)++], sizeof(wlan_mode), "%d", wlan_mode) ;
  snprintf(values[(*i)++], sizeof(gap_profile), "%d", gap_profile) ;
  flag=station_config->conn_type;
  snprintf(values[(*i)++], sizeof(flag), "%d", flag) ;
    addr.s_addr = ntohl(station_config->static_ip_config.ip_addr);
  snprintf(values[(*i)++], strlen(inet_ntoa(addr))+1, "%s", inet_ntoa(addr)) ;
    addr.s_addr = ntohl(station_config->static_ip_config.gw_ip);
  snprintf(values[(*i)++], strlen(inet_ntoa(addr))+1, "%s", inet_ntoa(addr)) ;
    addr.s_addr = ntohl(station_config->static_ip_config.dns_addr);
  snprintf(values[(*i)++], strlen(inet_ntoa(addr))+1, "%s", inet_ntoa(addr));
    addr.s_addr = ntohl(station_config->static_ip_config.netmask);
  snprintf(values[(*i)++], strlen(inet_ntoa(addr))+1, "%s", inet_ntoa(addr));

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
  #ifdef DEBUG_PRINT
     printf("source string is %s and length is %d", src, last);
  #endif
   tmp=(char*) malloc(last);
   if(tmp != NULL)
   {
     for(i=0,j=0; ( (i <= last ) && (src[i] != NULL) );)
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
     #ifdef DEBUG_PRINT
       printf("source string translated is %s", src);
     #endif
     //free memory
     free(tmp);
     //set null to pointer.
     tmp=NULL;
     return QCMAP_CM_SUCCESS;
   }
   else
   {
     #ifdef DEBUG_PRINT
       printf("source string translation error ");
     #endif
     LOG_MSG_ERROR("source string translation error : \n", 0, 0, 0);
     return QCMAP_CM_ERROR;
   }
 }
/*===========================================================================
  FUNCTION Tokenizer
===========================================================================*/
/*!
@brief
  parse cgi request message and store it.

@input
  values - cgi form field values.
  buf - message to be sent to web cgi.
  fields - fields buf which will have page name.
  token_pos - position of token in values array

@return
None

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static void Tokenizer
(
  char * buf,
  char fields[][MAX_ELEMENT_LENGTH],
  char values[][MAX_ELEMENT_LENGTH],
  int *token_pos
)
{
//tokenizer starts
char *ch,*ch2;
char* saveptr;
int i=0;
char columns[MAX_BUF_LEN];
//strtok_r reentrant so that context switch doesn't cause loss of data.
ch = strtok_r(buf, "&",&saveptr);
while (ch != NULL)
{
  strncpy(columns,ch,strlen(ch)+1);
  strncpy(fields[i],ch,strlen(ch)+1);
  ch2=strstr(columns,"=");
  strncpy(values[i],&ch2[1],strlen(&ch2[1]) +1);
  //strtok_r reentrant so that context switch doesn't cause loss of data.
  ch = strtok_r(NULL, "&",&saveptr);
  i++;
}
#ifdef DEBUG_PRINT
  printf("Page string %s \n ",fields[0]);
  printf("Values are \n ");
  for(int j = 0;j < i;j++)
 {
    printf(" %s \n",values[j]);
 }
#endif
*token_pos = j-1;
//tokenizer ends
}
/*===========================================================================
    FUNCTION create_msg_to_send
  ==========================================================================*/
  /*!
  @brief
    Error response message to CGI is built.

  @params
    values - values buff where response is stored.
    buf - output buffer to store response.
    output_string - string to be formed in response.
  @return
    buf - jason format is created in buf.

  @Dependencies
    - None

  @Side Effects
    - None
  */
/*=========================================================================*/
static void create_msg_to_send
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  char* output_string
)
{
  char tmp[MAX_BUF_LEN] ={0};
  LOG_MSG_ERROR("\npage returns, msg built for sending to client", 0, 0, 0);
  memset(tmp,0,MAX_BUF_LEN);
  memset(buf,0,MAX_BUF_LEN);
  strncat(buf,"{",1);
  snprintf(tmp,(strlen(values[0]) + strlen(PAGE) +1),PAGE,values[0]);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp,strlen(output_string) + strlen(RESULT_NO_COMMA)+1,
           RESULT_NO_COMMA, output_string);
  strncat(buf,tmp,strlen(tmp));

  strncat(buf,"}",1);
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
}
#ifdef __cplusplus
}
#endif
