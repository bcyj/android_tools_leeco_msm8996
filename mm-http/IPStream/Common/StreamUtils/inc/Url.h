#ifndef _URL_CLASS_H_
#define _URL_CLASS_H_
/************************************************************************* */
/**
 * URL.h
 * @brief Implementation of URL class.
 *
 COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/Url.h#10 $
$DateTime: 2013/08/02 06:28:55 $
$Change: 4209946 $

========================================================================== */
#include <AEEStdDef.h>
#include "dsbsd.h"
/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

//Max length of a URN, both EFS file name and "rtsp:" spec
const uint32 QTV_MAX_URN_BYTES = 2048;

//max relative control path length
const uint16 QTV_CONTROL_RELATIVE_URL_LEN = 30;

// maximum size of host name, by RFC 1035
#define HOST_NAME_SIZE     255

//max uint32 number's size
const uint8 PORT_STRING_SIZE = 10;

/**
 * @brief Class URL
 * This class provides functions required to manipulate URLs it
 * has also IPV6 awareness.
 */

class URL
{
  public:
    //Error Codes
    enum UrlErrCode
    {
      URL_NOT_DEFINED = -1,
      URL_OK = 0,
      URL_BAD_FORMAT,
      URL_EMPTY,
      URL_BUFFER_NULL,
      URL_MAX_LEN_EXCEEDS,
      URL_OUT_OF_MEMORY,
      URL_UNKNOWN_PROTOCOL,
      URL_ERROR
    };

    enum UrlProtocol
    {
      PROTO_HTTP,
      PROTO_RTSP,
      PROTO_ISDB,
      PROTO_SDP_BUFFER,
      PROTO_UNKNOWN
    };

    static const int  DEF_RTSP_PORT      = 554;

    //URL Maximum length, configurable through QtvConfg
    static uint32 MaxUrlSize;
    static uint32 MaxUrlSizeWithCtrlPath;

    //Ctors
    URL();
    //Creates a URL object based on an existing URL object.
    URL(const URL &url);
    URL(const char* urlStr);
    URL(uint32 urlSize);

    //Dtor
    ~URL();

    // Assigns a new value to the URL object.
    URL & operator =(const URL & otherUrl);
    URL & operator =(const char *urlStr);

    //Checks whether the object and other url/str are identical.
    bool operator == (const URL & otherUrl) const;
    bool operator == (const char *urlstr) const;

    //Checks whether the object and other url/str are not identical.
    bool operator != (const URL & otherUrl) const;
    bool operator != (const char *urlstr) const;

    //provide “const char*” access to URL object
    operator const char*() const;

    //url length
    size_t GetUrlLength() const;

    //provides char * access to url, can be manipulated in c-string style
    char *GetUrlBuffer();
    const char *GetUrlBuffer() const;
#if 0
    //concatenates a string to the url
    UrlErrCode ConcatUrl(const char * str);
#endif
    //Returns TRUE if the other-Uri matches with the start of the Url.
    bool StartsWith(const URL & otherUrl) const;
    bool StartsWith(const char * str) const;

    bool EndsWith(const char * str) const;

    //strips control chars in url
    UrlErrCode StripControlChars();

    //get the protocol
    enum UrlProtocol GetProtocol() const;

    //get the ip addr/dns name
    UrlErrCode GetHost(char * host,size_t maxHostLen) const;

    //returns the port number if present
    UrlErrCode GetPort(uint32 * port) const;

    //returns the clip name
    UrlErrCode GetClipName(char * clipName, size_t size) const;

    UrlErrCode ConvertIPv4mappedIPv6urlToIPv4url();
    static UrlErrCode ConvertIPv4AddrToIPv4MappedIPv6Addr(char *ipv4Addr,
                                                 struct in6_addr *ipv6Addr);
    //Set the maximum URL length
    static void SetMaxURLSize(const uint32 MaxURLSize);

    //is url object empty
    bool IsEmpty() const;

    //empties url object, deletes buffer as well
    void Clear();

  private:
    //Initialization
    void Init();
    //clears the url info
    UrlErrCode Reset();
    //is url buffer null
    bool IsNull() const;
    //get number of allocated bytes for url buffer
    uint32 GetCapacity() const;
#if 0
    //set url
    UrlErrCode SetUrl(const char* urlStr, int32 copylen = -1);
    UrlErrCode SetUrl(const URL * urlObj);
#endif
    //checks a URL whether it meets RFC 2396
    UrlErrCode ValidateURL() const;
    //Get the recent error
    UrlErrCode GetErrorCode() const;

    //Ipv6 specific
    bool IsIPv6Url() const;
    UrlErrCode MapIPv4urlToIPv4mappedIPv6url();

    //url buffer
    char * m_url;

    //holds allocated number of bytes
    size_t m_capacity;

    //holds current url length
    size_t m_urlLen;

    //recent error occured
    mutable UrlErrCode m_errCode;

    //member functions
    //reset the error code
    void ResetErrorCode() const;
};

#endif
