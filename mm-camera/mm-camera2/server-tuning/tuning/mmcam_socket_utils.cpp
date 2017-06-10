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
/* =============================================================================
*
 * This file contains socket utils class which abstracts socket communicaton
 * API
 *
 =============================================================================*/

#ifdef _ANDROID_
#include <log/log.h>
#endif

#include <sys/epoll.h>
#include <errno.h>
#include <string.h>

#include "mmcam_log_utils.h"
#include "mmcam_socket_utils.h"
#include "eztune.h"

extern "C" void dump_list_of_daemon_fd ();
namespace mmcam_utils {

Socket::Socket()
{
    m_sock = m_pipefd = m_efd = -1;

    memset(&m_addr, 0, sizeof(m_addr));
}

Socket::~Socket()
{
    if (is_valid())
        ::close(m_sock);

    if (m_efd >= 0)
        ::close(m_efd);
}

bool Socket::create()
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock >= MAX_FD_PER_PROCESS) {
        dump_list_of_daemon_fd();
        return false;
    }

    if (!is_valid())
        return false;

    int on = 1;
    if (setsockopt (m_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof (on)) == -1)
        return false;

    return true;
}

bool Socket::bind (const unsigned short port)
{
    if (!is_valid())
        return false;

    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons (port);

    int bind_return = ::bind(m_sock,
                             (struct sockaddr *)&m_addr,
                             sizeof (m_addr));

    if (bind_return == -1)
        return false;

    return true;
}


bool Socket::listen() const
{
    if (!is_valid())
        return false;

    int listen_return = ::listen(m_sock, MAXCONNECTIONS);

    if (listen_return == -1) {
        return false;
    }

    return true;
}


bool Socket::accept(Socket &new_socket) const
{
    int addr_length = sizeof(m_addr);
    new_socket.m_sock = ::accept(m_sock, (sockaddr *) &m_addr, (socklen_t *) &addr_length);

    if (new_socket.m_sock <= 0) {
        MMCAM_LOGE("Socket accept error: %s, errno:%d", strerror(errno), errno );
        return false;
    }

    return true;
}


bool Socket::send(const std::string s, size_t size) const
{
    MMCAM_ASSERT_PRE(s.capacity() >= size, "Buffer size(%zu) less than Msg size(%zu)",
                     s.capacity(), size);
    int status = 0;
    size_t sent_size = 0;
    bool rc = true;
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 2000000L; //2ms
    char *ptr = (char *)s.c_str();

    do {
        status = ::send (m_sock, ptr, size - sent_size, MSG_NOSIGNAL);

        MMCAM_LOGV("%d: Send size: %d", m_sock, status);

        if ( status == -1 ) {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                MMCAM_LOGE("Socket send error: %s", strerror(errno));
                rc = false;
                break;
            }
            //sleep before next iteration
            nanosleep(&tim , &tim2);
        } else {
            //status is actual size sent
            sent_size += status;
            ptr += status;
        }
    } while (sent_size < size);

    return rc;
}


bool Socket::recv(std::string &s, size_t size) const
{
    size_t recv_size = 0;
    int status = 0;
    bool rc = true;

    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 2000000L; //2ms

    //temp buffer to recv data
    auto buf = new char[size + 1];

    //clear receive string before adding new data
    s.clear();
    do {
        status = ::recv (m_sock, buf, size - recv_size, 0);

        if (status == -1) {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
                MMCAM_LOGE("Socket receive error: %s", strerror(errno));
                rc = false;
                break;
            }
            //sleep before next iteration
            nanosleep(&tim , &tim2);

        } else if (status == 0) {
            MMCAM_LOGI("Socket received zero size data: Client terminated connection");
            rc = false;
            break;
        }  else {
            s.append(buf, status);
            recv_size = recv_size + status;
        }

    } while (recv_size < size);

    MMCAM_LOGV("Exiting recv: last size: %zu: Total size: %zu", status, size);

    delete buf;

    return rc;
}


bool Socket::connect(const std::string host, const int port)
{
    if ( ! is_valid() ) return false;

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons (port);

    int status = inet_pton (AF_INET, host.c_str(), &m_addr.sin_addr);

    if (errno == EAFNOSUPPORT) return false;

    status = ::connect(m_sock, ( sockaddr *) &m_addr, sizeof(m_addr));

    if (status == 0)
        return true;
    else
        return false;
}

void Socket::set_non_blocking(const bool b)
{
    int opts;

    opts = fcntl(m_sock, F_GETFL);

    if (opts < 0)
        return;

    if (b)
        opts = ( opts | O_NONBLOCK );
    else
        opts = ( opts & ~O_NONBLOCK );

    fcntl(m_sock, F_SETFL, opts);
}

void ServerSocket::SetupEpoll(int pipefd)
{
    m_pipefd = pipefd;
    Socket::set_non_blocking (true);

    m_efd = epoll_create(sizeof(epoll_event));

    struct epoll_event event;
    event.data.fd = m_sock;
    event.events = EPOLLIN;
    epoll_ctl (m_efd, EPOLL_CTL_ADD, m_sock, &event);

    event.data.fd = m_pipefd;
    event.events = EPOLLIN;
    epoll_ctl (m_efd, EPOLL_CTL_ADD, m_pipefd, &event);
}

ServerSocket::ServerSocket(unsigned short port, int pipefd)
{
    bool rc = Socket::create();
    MMCAM_ASSERT(rc == true, "Could not create server socket: %s", strerror(errno));

    if (port) {
        rc = Socket::bind(port);
        if (rc != true)
            MMCAM_LOGE("Could not bind to port: %s", strerror(errno));
    }

    rc = Socket::listen();
    if (rc != true)
        MMCAM_LOGE("Could not listen to socket: %s", strerror(errno));

    if (pipefd >= 0) {
        SetupEpoll(pipefd);
    }
}

ServerSocket::~ServerSocket()
{
}

int ServerSocket::accept ( ServerSocket &sock, bool &pipe_event)
{
    int rc = 0;

    //set default to false assuming a typical blocking operation
    pipe_event = false;

    if (m_efd >= 0) {

        struct epoll_event event;
        int nr;
        //ignore EINTR errors which gets triggered during gdb debugs
        do {
            nr = epoll_wait (m_efd, &event, 1, -1);
        } while (nr < 0 && errno == EINTR);

        if ((event.events & EPOLLERR) || (event.events & EPOLLHUP)
                || (!(event.events & EPOLLIN))) {

            /* An error has occured on this fd, or the socket is not
            ready for reading (why were we notified then?) */
            MMCAM_LOGW("epoll error :%s. Event: %x \n",  strerror(errno), event.events);
            rc = -1;

            if (m_pipefd == event.data.fd)
                pipe_event = true;

            goto end;

        } else if (m_pipefd == event.data.fd) {
            pipe_event = true;
            MMCAM_LOGW("Received event from non socket fd");
            goto end;
        }
    }

    if ( ! Socket::accept (sock) ) {
        MMCAM_LOGE("Could not accept socket.");
        rc = -1;
    } else {
        // If server socket was in non blocking mode,
        // initialize client socket also in non blocking mode
        if (m_pipefd >= 0)
            sock.SetupEpoll(m_pipefd);
    }

end:
    return rc;
}

bool ServerSocket::recv(std::string &s, size_t size, bool &pipe_event, uint32_t pipe_msg_size)
{
    bool rc = false;

    //set default to false assuming a typical blocking operation
    pipe_event = false;

    if (m_efd >= 0) {

        struct epoll_event event;
        int nr;
        //ignore EINTR errors which gets triggered during gdb debugs
        do {
            nr = epoll_wait (m_efd, &event, 1, -1);
        } while (nr < 0 && errno == EINTR);

        if ((event.events & EPOLLERR) || (event.events & EPOLLHUP)
                || (!(event.events & EPOLLIN))) {

            /* An error has occured on this fd, or the socket is not
            ready for reading (why were we notified then?) */
            MMCAM_LOGW("epoll Event (%d), fd(%d)", event.events, event.data.fd);

            if (m_pipefd == event.data.fd)
                pipe_event = true;

            goto end;

        } else if (m_pipefd == event.data.fd) {
            auto buf = new char[pipe_msg_size+1];
            pipe_event = true;

            MMCAM_LOGW("Received event from non socket fd");

            //clear receive string before adding new data
            s.clear();
            //read async response from the pipe
            int32_t read_bytes = read(m_pipefd, buf, pipe_msg_size);

            if ((read_bytes < 0) ||
                (read_bytes != (int32_t)pipe_msg_size)) {
              MMCAM_LOGE("failed: read_bytes %d", read_bytes);
            } else {
                s.append(buf, read_bytes);
            }
            delete buf;
            goto end;
        }
    }

    rc = Socket::recv(s, size);

end:
    return rc;
}

ClientSocket::ClientSocket (std::string host, int port)
{
    bool rc = Socket::create();
    MMCAM_ASSERT(rc == true, "Could not create server socket");

    rc = Socket::connect (host, port);
    MMCAM_ASSERT(rc == true, "Could not connect to host");
}

}; //namespace mmcam_utils
