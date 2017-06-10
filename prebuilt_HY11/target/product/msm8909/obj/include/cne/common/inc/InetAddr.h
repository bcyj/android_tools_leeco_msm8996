#ifndef INET_ADDR_H_
#define INET_ADDR_H_

/*==============================================================================
  FILE:         InetAddr

  OVERVIEW:     Class for managing Internet Addresses, assumes IPv6

  DEPENDENCIES: in6_addr, inet_pton, inet_ntop

                Copyright (c) 2011 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/


/*==============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ----------------------------------------------------------
  2011-09-20  jnb     First revision.
==============================================================================*/


/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <string>

#ifndef INET6_OCTETLEN
#define INET6_OCTETLEN 16
#endif
/*------------------------------------------------------------------------------
 * Class Definition
 * ---------------------------------------------------------------------------*/
class InetAddr
{
public:

  /*----------------------------------------------------------------------------
   * FUNCTION      InetAddr
   *
   * DESCRIPTION   default constructor
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  InetAddr();

  /*----------------------------------------------------------------------------
   * FUNCTION      InetAddr
   *
   * DESCRIPTION   constructor
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  InetAddr(in6_addr const& setAddress, int setPort = 0);

  /*----------------------------------------------------------------------------
   * FUNCTION      InetAddr
   *
   * DESCRIPTION   constructor
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  address is stored as a mapped IPv6 address
   *--------------------------------------------------------------------------*/
  InetAddr(in_addr const& setAddress, int setPort = 0);

  /*----------------------------------------------------------------------------
   * FUNCTION      InetAddr
   *
   * DESCRIPTION   constructor, setAddress is assumed big endian
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  InetAddr(uint32_t const setAddress[4], int setPort = 0);

  /*----------------------------------------------------------------------------
   * FUNCTION      InetAddr
   *
   * DESCRIPTION   constructor
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  InetAddr(std::string const& setIPv6Address, int setPort = 0);

  /*----------------------------------------------------------------------------
   * FUNCTION      InetAddr
   *
   * DESCRIPTION   copy constructor
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  InetAddr(InetAddr const& inetAddrToCopy);

  /*----------------------------------------------------------------------------
   * FUNCTION      ~InetAddr
   *
   * DESCRIPTION   destructor
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  virtual ~InetAddr();

  /*----------------------------------------------------------------------------
   * FUNCTION      setAddress
   *
   * DESCRIPTION   set the IPv6 address
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void setAddress(in6_addr const& setAddress);

  /*----------------------------------------------------------------------------
   * FUNCTION      setAddress
   *
   * DESCRIPTION   set the IPv6 address from a IPv4 address
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  address is stored as a mapped IPv6 address
   *--------------------------------------------------------------------------*/
  void setAddress(in_addr const& setAddress);

  /*----------------------------------------------------------------------------
   * FUNCTION      setAddress
   *
   * DESCRIPTION   set the IPv6 address
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void setAddress(uint32_t const setAddress[4]);

  /*----------------------------------------------------------------------------
   * FUNCTION      setAddress
   *
   * DESCRIPTION   set the IPv6 address from a IPv4 address
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  address is stored as a mapped IPv6 address
   *--------------------------------------------------------------------------*/
  void setAddress(uint32_t setAddress);

  /*----------------------------------------------------------------------------
   * FUNCTION      setAddress
   *
   * DESCRIPTION   set the IPv6 address from a string; if an invalid
   *               adderss is passed the address will be set to "::0"
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void setAddress(std::string const& setIPv6Address);

  /*----------------------------------------------------------------------------
   * FUNCTION      setAddress
   *
   * DESCRIPTION   set the IPv6 address from a C string; if an invalid
   *               adderss is passed the address will be set to "::0"
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void setAddress(const char *IPv6Address);

  /*----------------------------------------------------------------------------
   * FUNCTION      isLinkLocal
   *
   * DESCRIPTION   Checks if presentation formatted IPV6 string is Link Local.
   *
   * DEPENDENCIES  None
   *
   * RETURN VALUE  SUCCESS: true
   *               FAILURE: false
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
    bool isLinkLocal();

  /*----------------------------------------------------------------------------
   * FUNCTION      isLoopBack
   *
   * DESCRIPTION   Checks if presentation formatted IPV6 string is Loop Back.
   *
   * DEPENDENCIES  None
   *
   * RETURN VALUE  SUCCESS: true
   *               FAILURE: false
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   bool isLoopBack();

  /*----------------------------------------------------------------------------
   * FUNCTION      isInaddrAny
   *
   * DESCRIPTION   Checks if presentation formatted IPV6 string is IN6ADDR_ANY
   *
   * DEPENDENCIES  None
   *
   * RETURN VALUE  SUCCESS: true
   *               FAILURE: false
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
   bool isInaddrAny();

  /*----------------------------------------------------------------------------
   * FUNCTION      getAddress
   *
   * DESCRIPTION   -
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  return the IPv6 address as an in6_addr
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  in6_addr getAddress() const;

  /*----------------------------------------------------------------------------
   * FUNCTION      getAddressV4
   *
   * DESCRIPTION   -
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  return the IPv6 address translated into an in_addr
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  in_addr getAddressV4() const;

  /*----------------------------------------------------------------------------
   * FUNCTION      getAddressAsString
   *
   * DESCRIPTION   -
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  return the IPv6 address as a std::string
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  std::string getAddressAsString() const;

  /*----------------------------------------------------------------------------
   * FUNCTION      setPort
   *
   * DESCRIPTION   set the port number
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void setPort(int setPort);

  /*----------------------------------------------------------------------------
   * FUNCTION      getPort
   *
   * DESCRIPTION   return the port number
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  port number as an uint16_t
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  uint16_t getPort() const;

  /*----------------------------------------------------------------------------
   * FUNCTION      toString()
   *
   * DESCRIPTION   return a string representing this address with the port
   *               number, as defined in RFC3986 and RFC5952
   *               For Example: [2001:4860:b002::68]:80
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  string
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  std::string toString() const;

  /*----------------------------------------------------------------------------
   * FUNCTION      operator=
   *
   * DESCRIPTION   assignment operator
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  a const reference to self
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  InetAddr& operator=(InetAddr const& rhs);

  /*----------------------------------------------------------------------------
   * FUNCTION      operator==
   *
   * DESCRIPTION   comparison equals operator
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  bool
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  bool operator==(const InetAddr& rhs) const;

  /*----------------------------------------------------------------------------
   * FUNCTION      operator!=
   *
   * DESCRIPTION   comparison equals operator
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  bool
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  bool operator!=(const InetAddr& rhs) const;

  /*----------------------------------------------------------------------------
   * FUNCTION      operator<
   *
   * DESCRIPTION   comparison less than operator
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  bool
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  bool operator<(const InetAddr& rhs) const;

  /*----------------------------------------------------------------------------
   * FUNCTION      operator<=
   *
   * DESCRIPTION   comparison less than or equals operator
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  bool
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  bool operator<=(const InetAddr& rhs) const;

  /*----------------------------------------------------------------------------
   * FUNCTION      operator>
   *
   * DESCRIPTION   comparison greater than operator
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  bool
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  bool operator>(const InetAddr& rhs) const;

  /*----------------------------------------------------------------------------
   * FUNCTION      operator>=
   *
   * DESCRIPTION   comparison greater than or equals operator
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  bool
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  bool operator>=(const InetAddr& rhs) const;

  /*----------------------------------------------------------------------------
   * FUNCTION      operator()
   *
   * DESCRIPTION   Used by STL to determine weak ordering of elements.
   *               Compares source and destination address/port
   *
   * DEPENDENCIES  -
   *
   * RETURN VALUE  true if a < b
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
    bool operator()(InetAddr const& a, InetAddr const& b) const;

  /*----------------------------------------------------------------------------
   * FUNCTION      IPv4to6
   *
   * DESCRIPTION   'address' will be converted to IPv6; assumes 'address' is a
   *               valid IPv4 address; if already IPv6 no action is taken
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  'address' will be modified
   *--------------------------------------------------------------------------*/
  static void IPv4to6(std::string& address);

  /*----------------------------------------------------------------------------
   * FUNCTION      IPv4to6
   *
   * DESCRIPTION   'address' will be converted to IPv6; if already a valid
   *               IPv6 address no action is taken
   *
   *               Assumes:
   *                * 'address' is a valid IPv4 address
   *                * address is in network order (big endian)
   *                * IPv4 address is in the last positon of the array (address[3])
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  'address' will be modified
   *--------------------------------------------------------------------------*/
  static void IPv4to6(uint32_t address[4]);

  /*----------------------------------------------------------------------------
   * FUNCTION      isIPv4MappedIPv6
   *
   * DESCRIPTION   'address' will be checked whether v4 mapped v6.
   *
   *               Assumes:
   *                * 'address' is a valid IPv4 address
   *                * address is in network order (big endian)
   *
   * DEPENDENCIES  none
   *
   * RETURN VALUE  bool
   *
   * SIDE EFFECTS  'address' will be modified
   *--------------------------------------------------------------------------*/
  static bool isIPv4MappedIPv6(uint32_t address[4]);

private:

  /*----------------------------------------------------------------------------
   * Private Attributes
   * -------------------------------------------------------------------------*/
  in6_addr address;
  int port;

};

/*----------------------------------------------------------------------------
 * Inline Methods
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * FUNCTION      operator==
 *
 * DESCRIPTION   comparison equals operator
 *
 * DEPENDENCIES  none
 *
 * RETURN VALUE  bool
 *
 * SIDE EFFECTS  none
 *--------------------------------------------------------------------------*/
inline bool InetAddr::operator==(const InetAddr& rhs) const {
  int diff = memcmp(&address, &(rhs.address), sizeof(address));
  return (diff == 0 && port == rhs.port);
}

/*----------------------------------------------------------------------------
 * FUNCTION      operator!=
 *
 * DESCRIPTION   comparison equals operator
 *
 * DEPENDENCIES  none
 *
 * RETURN VALUE  bool
 *
 * SIDE EFFECTS  none
 *--------------------------------------------------------------------------*/
inline bool InetAddr::operator!=(const InetAddr& rhs) const {
  return !(*this == rhs);
}

/*----------------------------------------------------------------------------
 * FUNCTION      operator<
 *
 * DESCRIPTION   comparison less than operator
 *
 * DEPENDENCIES  none
 *
 * RETURN VALUE  bool
 *
 * SIDE EFFECTS  none
 *--------------------------------------------------------------------------*/
inline bool InetAddr::operator<(const InetAddr& rhs) const {
  int diff = memcmp(&address, &(rhs.address), sizeof(address));
  if (diff < 0 || (diff == 0 && port < rhs.port))  {
    return true;
  } else {
    return false;
  }
}

/*----------------------------------------------------------------------------
 * FUNCTION      operator<=
 *
 * DESCRIPTION   comparison less than or equals operator
 *
 * DEPENDENCIES  none
 *
 * RETURN VALUE  bool
 *
 * SIDE EFFECTS  none
 *--------------------------------------------------------------------------*/
inline bool InetAddr::operator<=(const InetAddr& rhs) const {
  return !(*this > rhs);
}

/*----------------------------------------------------------------------------
 * FUNCTION      operator>
 *
 * DESCRIPTION   comparison greater than operator
 *
 * DEPENDENCIES  none
 *
 * RETURN VALUE  bool
 *
 * SIDE EFFECTS  none
 *--------------------------------------------------------------------------*/
inline bool InetAddr::operator>(const InetAddr& rhs) const {
  int diff = memcmp(&address, &(rhs.address), sizeof(address));
  if (diff > 0 || (diff == 0 && port > rhs.port))  {
    return true;
  } else {
    return false;
  }
}

/*----------------------------------------------------------------------------
 * FUNCTION      operator>=
 *
 * DESCRIPTION   comparison greater than or equals operator
 *
 * DEPENDENCIES  none
 *
 * RETURN VALUE  bool
 *
 * SIDE EFFECTS  none
 *--------------------------------------------------------------------------*/
inline bool InetAddr::operator>=(const InetAddr& rhs) const {
  return !(*this < rhs);
}

/*----------------------------------------------------------------------------
 * FUNCTION      operator()
 *
 * DESCRIPTION   Used by STL to determine weak ordering of elements.
 *               Compares source and destination address/port
 *
 * DEPENDENCIES  -
 *
 * RETURN VALUE  true if a < b
 *
 * SIDE EFFECTS  -
 *--------------------------------------------------------------------------*/
inline bool InetAddr::operator()(InetAddr const& a, InetAddr const& b) const {
  return (a < b);
}

/*----------------------------------------------------------------------------
* FUNCTION      isLinkLocal
*
* DESCRIPTION   Checks if presentation formatted IPV6 string is Link Local.
*
* DEPENDENCIES  None
*
* RETURN VALUE  SUCCESS: true
*               FAILURE: false
*
* SIDE EFFECTS  None
*--------------------------------------------------------------------------*/
inline bool InetAddr::isLinkLocal()
{
    uint8_t ip[INET6_OCTETLEN]; // 16*8 = 128 bits, 32 hex digits
    memcpy(&ip, (void *)&address, sizeof(in6_addr));
    if( (ip[0] & 0xff) ==0xfe && (ip[1] & 0xc0) ==0x80)
    {
        return true;
    }
    return false;
}

/*----------------------------------------------------------------------------
* FUNCTION      isLoopBack
*
* DESCRIPTION   Checks if presentation formatted IPV6 string is Loop Back.
*
* DEPENDENCIES  None
*
* RETURN VALUE  SUCCESS: true
*               FAILURE: false
*
* SIDE EFFECTS  None
*--------------------------------------------------------------------------*/
inline bool InetAddr::isLoopBack()
{
    uint8_t ip[INET6_OCTETLEN];
    memset(&ip, 0, sizeof(in6_addr));
    memset(&ip[INET6_OCTETLEN - 1], 1 , sizeof(char));
    if( memcmp((void *)&address, (void *)&ip, sizeof(in6_addr)) == 0 )
    {
        return true;
    }
    return false;
}

/*----------------------------------------------------------------------------
* FUNCTION      isInaddrAny
*
* DESCRIPTION   Checks if presentation formatted IPV6 string is IN6ADDR_ANY
*
* DEPENDENCIES  None
*
* RETURN VALUE  SUCCESS: true
*               FAILURE: false
*
* SIDE EFFECTS  None
*--------------------------------------------------------------------------*/
inline bool InetAddr::isInaddrAny()
{
    uint8_t ip[INET6_OCTETLEN];
    memset(&ip, 0, sizeof(in6_addr));
    if( memcmp((void *)&address, (void *)&ip, sizeof(in6_addr)) == 0 )
    {
        return true;
    }
    return false;
}

#endif /* INET_ADDR_H_ */
