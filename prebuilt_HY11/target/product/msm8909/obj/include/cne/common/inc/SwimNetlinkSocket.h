#ifndef _SwimNetlinkSocket_h_
#define _SwimNetlinkSocket_h_

/*==============================================================================
  FILE:         SwimNetlinkSocket.h

  OVERVIEW:     Contains logic related to technology-independent portion
                of network interface management.

  DEPENDENCIES: Logging, Ppc, C++ STL

                Copyright (c) 2010,2014 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Qualcomm Technologies Confidential and Proprietary
==============================================================================*/


/*==============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ----------------------------------------------------------
  05-18-2010  dwc     First revision.
==============================================================================*/


/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ---------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * Forward Class Declarations
 * ---------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * Type Declarations
 * ---------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * Class Definition
 * ---------------------------------------------------------------------------*/


class SwimNetlinkSocket
{

public:

  /*----------------------------------------------------------------------------
   * Public Types
   * -------------------------------------------------------------------------*/


  typedef void ( *MessageHandler_t )( void *arg, struct nlmsghdr *, unsigned int len );


  /*----------------------------------------------------------------------------
   * Public Method Specifications
   * -------------------------------------------------------------------------*/


  /*----------------------------------------------------------------------------
   * FUNCTION      SwimNetlinkSocket, constructor
   *
   * DESCRIPTION   Default constructor, will initialize netlink socket for
   *               the service.  The stype argument specifies socket protocol
   *
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  SwimNetlinkSocket( int stype, unsigned groups, bool log=false);


  /*----------------------------------------------------------------------------
   * FUNCTION      SwimNetlinkSocket, constructor
   *
   * DESCRIPTION   Send a netlink message.
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  0 if successful, -1 otherwise
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  int NetlinkSend( struct nlmsghdr *msg, size_t len );


  /*----------------------------------------------------------------------------
   * FUNCTION      SwimNetlinkSocket, constructor
   *
   * DESCRIPTION   Receive a netlink message, handles SARing.
   *
   * DEPENDENCIES  Logging
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  Calls mhldr for each netlink message found
   *--------------------------------------------------------------------------*/
  void NetlinkRecv( uint32_t a_token, MessageHandler_t mhldr, void *arg );

  /*--------------------------------------------------------------------------
   * FUNCTION      DumpNlSock
   *
   * DESCRIPTION   Used for getting socket information
   *
   * DEPENDENCIES  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  void DumpNlSock();

  /*----------------------------------------------------------------------------
   * FUNCTION      Accessor for file descriptor
   *
   * DESCRIPTION   Used for getting poll information
   *
   * DEPENDENCIES  None
   *
   * RETURN VALUE  None
   *
   * SIDE EFFECTS  None
   *--------------------------------------------------------------------------*/
  int getFd( ) const
  {
    return( nlsock );
  }

private:

  /*----------------------------------------------------------------------------
   * Private Types
   * -------------------------------------------------------------------------*/


  /*----------------------------------------------------------------------------
   * Private Method Specifications
   * -------------------------------------------------------------------------*/


  /*----------------------------------------------------------------------------
   * Private Attributes
   * -------------------------------------------------------------------------*/


  //
  // Cached file descriptor for interfacing w/ netlink.
  //
  int nlsock;

  //
  // PID of this daemon for use with netlink.
  //
  int mpid;

  //
  // Sequence number for interfacing with netlink.
  //
  uint32_t nltoken;

  //debug logging
  const bool VDBG;

};

#endif /* _SwimNetlinkSocket_h_ */
