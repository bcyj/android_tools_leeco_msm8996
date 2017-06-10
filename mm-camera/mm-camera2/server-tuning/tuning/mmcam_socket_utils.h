/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

/**************************************************************
* Portions of this file are derived from code Copyright © 2002, Rob Tougher.
* Modified By:
*   Narendran Rajan
*
*   Article Citation
*   Tougher, Rob. "Linux Socket Programming In C++." Linux Journal. Linux Gazette Issue 74 (January 2002)
*
*   Web url, accessed 3/25/14
*   http://www.tldp.org/LDP/LG/issue74/tougher.html
*
*
* Original file can be found at:
*   http://tldp.org/LDP/LG/issue74/misc/tougher/Socket.cpp.txt
**************************************************************/

#ifndef MMCAM_SOCKET_UTILS_H
#define MMCAM_SOCKET_UTILS_H

#include <string>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace mmcam_utils
{

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;

class Socket
{
public:
    Socket();
    virtual ~Socket();

    // Server initialization
    bool create();
    bool bind(const unsigned short port);
    bool listen() const;
    bool accept(Socket &) const;

    // Client initialization
    bool connect(const std::string host, const int port);

    // Data Transimission
    bool send(const std::string, size_t size) const;
    bool recv(std::string &, size_t size) const;

    void set_non_blocking(const bool);

    bool is_valid() const
    {
        return m_sock != -1;
    }

protected:

    int m_sock;
    sockaddr_in m_addr;

    //for non blocking event polling support
    int m_efd, m_pipefd;
};

class ServerSocket : public Socket
{
public:

    //! Constructor: Takes port parameter and an optional pipefd
    /*!
       If pipefd is non negative value, the server socket uses epoll to wait
       on both socket and pipefd to enable clients to bring socket out of io wait

      \param in port: The server socket port number
      \param in pipefd: For enabling epoll based mode. Set to -1 for a regular blocking operation
    */
    ServerSocket (unsigned short port, int pipefd);
    ServerSocket () {};
    virtual ~ServerSocket();

    //! Socket connect accept API.
    /*!
       Accepts the connection and updates provided socket object with parameters to communicate
       with client. If valid pipefd was provided during Socket create, server would wait
      on both socket connection and events on provided fd. If an event is generated on pipefd which
      was provided during create, accept would return immediatley with pipe_event flag set to true and
      without waiting for socket accept to complete

      \param in new_socket: Reference to new client socket which needs to be populated
      \param out pipe_event: Sets to true if event occured on pipefd and returning due to this
      \return = 0 -  no error case. It could mean accept succeeded or a non data event on pipefd
      \return = -1 - Error scenario. pipe_event flag would be set to true if error happened in pipefd
    */
    int accept (ServerSocket &new_socket, bool &pipe_event);

    //! Socket data receive API.
    /*!
       Receives data from client.If valid pipefd was provided during Socket create, server would wait
      on both socket data receive and events on provided fd. If an event is generated on pipefd which
      was provided during create, recv would return immediatley with pipe_event flag set to true and
      without waiting for data from clients

      \param in s: Input string object to receive data
      \param in size: Size of data expected
      \param out pipe_event: Sets to true if event occured on pipefd and returning due to this
      \return - true to indicate success, false for error condition
       If event occured on pipefd, pipe_event would be set to true
      \return = -1 - Error scenario. pipe_event flag would be set to true if error happened in pipefd
    */
    bool recv(std::string &s, size_t size, bool &pipe_event, uint32_t pipe_msg_size);

    void SetupEpoll(int pipefd);
};

class ClientSocket : public Socket
{
public:

    ClientSocket ( std::string host, int port );
    virtual ~ClientSocket() {};
};

};

#endif
