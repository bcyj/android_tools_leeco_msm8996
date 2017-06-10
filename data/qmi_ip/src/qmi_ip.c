/******************************************************************************

                        QMI_IP.C

******************************************************************************/

/******************************************************************************

  @file    qmi_ip.c
  @brief   Qualcomm mapping interface over IP

  DESCRIPTION


  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
08/30/13   tw         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "ds_string.h"
#include "qmi_ip.h"

/*===========================================================================
                              VARIABLE DECLARATIONS
===========================================================================*/

int multicast_server_finished;
int acceptTCPConnection;
int heartbeat_check;
int heartbeat_response;
int shut_down;
int sslConnecting;
int ssl_active;

SSL*                ssl;
int                 qmi_handle;
int                 wds_clnt_hndl;
qmi_client_type     qmi_nas_handle;
dsi_call_info_t     dsi_net_hndl;
qmi_ip_conf_t       qmi_ip_conf;
int                 embms_rmnet_link;

int                 device_mode;

pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     *ssl_mutex_lock_array;

void* init_qti_socket(void);

/*===========================================================================
                              FUNCTION DEFINITIONS
===========================================================================*/

/*==========================================================================

FUNCTION INIT_QTI_SOCKET()

DESCRIPTION

  Create thread for for receiving the link up/down events from QTI

DEPENDENCIES
  None.

RETURN VALUE
  None


SIDE EFFECTS
  None

==========================================================================*/
void* init_qti_socket(void){
    int                     val, ret, len, i;
    struct                  sockaddr_un qti_socket;
    struct timeval          rcv_timeo;
    unsigned int            sockfd, max_sd;
    fd_set                  qti_fd_set;
    char                    buf[MAX_BUF_LEN];
    struct sockaddr_storage their_addr;
    socklen_t               addr_len;
    qmi_ip_event_e          event;
    struct timeval *timeout = NULL;

    timeout = calloc(1, sizeof(struct timeval));
    if(timeout == NULL)
    {
        LOG_MSG_ERROR("Couldn't calloc space for timeout", 0, 0, 0);
        return;

    }
    timeout->tv_sec  = 2 ;
    timeout->tv_usec = 0;

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
        return;
    }

    fcntl(sockfd, F_SETFD, FD_CLOEXEC);

    rcv_timeo.tv_sec = 0;
    rcv_timeo.tv_usec = 100000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
    val = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, val);
    /* set bit in os_params */
    FD_ZERO(&qti_fd_set);
    FD_SET(sockfd, &qti_fd_set);

    qti_socket.sun_family = AF_UNIX;
    strlcpy(qti_socket.sun_path, QMI_IP_STA_FILE, sizeof(QMI_IP_STA_FILE));
    unlink(qti_socket.sun_path);
    len = strlen(qti_socket.sun_path) + sizeof(qti_socket.sun_family);

    if (bind(sockfd, (struct sockaddr *)&qti_socket, len) < 0)
    {
        LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
        return;
    }

    while (1) {
        ret = select(sockfd, &qti_fd_set, NULL, NULL, timeout);

        if (ret = 0 && shutdown) {
            break;
        }

        memset(buf, 0, sizeof(buf));
        if ( ( i = recvfrom(sockfd, buf, 1 , 0, (struct sockaddr *)&their_addr, &addr_len)) > 0 ){
            if ( *buf == LINK_UP_STR)
                event = QMI_IP_LINK_UP_EVENT;
            else if (*buf == LINK_DOWN_STR )
                event = QMI_IP_LINK_DOWN_EVENT;

            if (event != 0 )
                ret= qmi_ip_queue_event(event, device_mode);
            else
                LOG_MSG_ERROR("Error incorrect link event, %s", buf, 0, 0);
        }
    }
    LOG_MSG_ERROR("Shutting down socket to QTI", 0, 0, 0);
    close(sockfd);
    return;
}

/*===========================================================================
  FUNCTION ss_lock_callback
==========================================================================*/
/*!
@brief
  Thread lock Call Back to allow SSL to send and recieve at the same time

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void ss_lock_cb(int mode, int type, const char *file, int line)
{
  (void)file;
  (void)line;
  if (mode & CRYPTO_LOCK) {
    pthread_mutex_lock(&(ssl_mutex_lock_array[type]));
  }
  else {
    pthread_mutex_unlock(&(ssl_mutex_lock_array[type]));
  }
}


/*===========================================================================
  FUNCTION ss_thread_id
==========================================================================*/
/*!
@brief
  Thread ID Call Back to allow SSL to send and recieve at the same time

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static unsigned long ss_thread_id(void)
{
  unsigned long ret;

  ret=(unsigned long)pthread_self();
  return(ret);
}

void init_ssl_thread_locks(void);


/*===========================================================================
  FUNCTION init_ssl_thread_locks
==========================================================================*/
/*!
@brief
  Initialize the SSL thread locks, setup call backs

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void init_ssl_thread_locks(void)
{
    int i;

    ssl_mutex_lock_array = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));

    for (i=0; i<CRYPTO_num_locks(); i++) {
        pthread_mutex_init(&(ssl_mutex_lock_array[i]),NULL);
    }

    CRYPTO_set_id_callback((unsigned long (*)(void))ss_thread_id);
    CRYPTO_set_locking_callback((void (*)(int, int,const char*,int))ss_lock_cb);
}


/*===========================================================================
  FUNCTION nas_ind_cb
==========================================================================*/
/*!
@brief
  Handle NAS indications

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void nas_ind_cb
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int    msg_id,                         /* Indicator message ID  */
  void           *ind_buf,                        /* Raw indication data   */
  unsigned int    ind_buf_len,                    /* Raw data length       */
  void           *ind_cb_data                     /* User call back handle */
)
{
    int ret;
    unsigned char *msg_ptr;
    unsigned char *tmp_msg_ptr;
    if (ssl == NULL)
        return;

    int msg_ptr_len = ind_buf_len + QMI_HEADER_SIZE;
    msg_ptr = malloc(msg_ptr_len);

    if (msg_ptr == NULL){
        LOG_MSG_ERROR("MALLOC failure",0,0,0);
        return;
    }

    tmp_msg_ptr = msg_ptr;

    WRITE_QMI_HEADER(tmp_msg_ptr, msg_ptr_len, QMI_NAS_SERVICE_TYPE, msg_id,
                     QMI_IND_CTRL_FLAGS, QMI_IND_TX_ID, msg_ptr_len - QMI_HEADER_SIZE);
    memcpy(tmp_msg_ptr, ind_buf, ind_buf_len);

    ret = SSL_write(ssl, msg_ptr, msg_ptr_len);
    free(msg_ptr);

    return;
}


/*===========================================================================
  FUNCTION netmgr_ind_cb
==========================================================================*/
/*!
@brief
  Handle NETMGR indications

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_ind_cb
(
  netmgr_nl_events_t event,
  netmgr_nl_event_info_t * info,
  void * data
)
{
  int rc;
  dsi_call_tech_type tech;

  if (NULL == info)
  {
      LOG_MSG_ERROR("NULL Info rcvd", 0,0,0);
      return;
  }

  if (((int) info->link) != embms_rmnet_link) //may need to do a mapping between netmgr_link_id_e and qmi_client_qmux_instance_type
  {
      LOG_MSG_ERROR("Event is not for eMBMS link", 0,0,0);
      return;
  }

  switch( event )
  {
  case NET_PLATFORM_UP_EV:
  case NET_PLATFORM_RECONFIGURED_EV:
    netmgr_status = UP;
    LOG_MSG_INFO1("Netmgr UP, %d", event,0,0);
    break;
  case NET_PLATFORM_DOWN_EV:
    LOG_MSG_INFO1("Netmgr DOWN", 0,0,0);
    netmgr_status = DOWN;
    break;
  }

  return;
}


/*==========================================================================

FUNCTION CHECK_SSL_ERROR()

DESCRIPTION

  Print the SSL error and return

DEPENDENCIES
  None.

RETURN VALUE
  None


SIDE EFFECTS
  None

==========================================================================*/
int check_ssl_error(int err){
    int ret = SSL_get_error(ssl, err);

    //print switch for debugging
    switch (ret) {
    case SSL_ERROR_WANT_READ:
        //LOG_MSG_INFO1("SSL_ERROR_WANT_READ %d",SSL_ERROR_WANT_READ,0,0);
        break;
    case SSL_ERROR_WANT_WRITE:
        LOG_MSG_INFO1("SSL_ERROR_WANT_WRITE %d %d",SSL_ERROR_WANT_WRITE,errno,0);
        break;
    case SSL_ERROR_ZERO_RETURN:
        LOG_MSG_INFO1("SSL_ERROR_ZERO_RETURN %d %d",SSL_ERROR_ZERO_RETURN,errno,0);
        break;
    case SSL_ERROR_WANT_CONNECT:
        LOG_MSG_INFO1("SSL_ERROR_WANT_CONNECT %d %d",SSL_ERROR_WANT_CONNECT,errno,0);
        break;
    case SSL_ERROR_WANT_ACCEPT:
        LOG_MSG_INFO1("SSL_ERROR_WANT_ACCEPT %d %d",SSL_ERROR_WANT_ACCEPT,errno,0);
        break;
    case SSL_ERROR_WANT_X509_LOOKUP:
        LOG_MSG_INFO1("SSL_ERROR_WANT_X509_LOOKUP %d %d",SSL_ERROR_WANT_X509_LOOKUP,errno,0);
        break;
    case SSL_ERROR_SYSCALL:
        /*do not print error message here
        Since we have a timeout on the SSL socket this error will be posted
        whenever the timeout is hit.*/
        break;
    case SSL_ERROR_SSL:
        LOG_MSG_INFO1("SSL_ERROR_SSL %d %d",SSL_ERROR_SSL,errno,0);
        break;
    case SSL_ERROR_NONE:
        LOG_MSG_INFO1("SSL_ERROR_NONE %d %d",SSL_ERROR_NONE,errno,0);
        break;
    default:
        LOG_MSG_INFO1("Uknown error %d %d",ret,errno,0);
    }

    return ret;
}

int check_heartbeat(int* err, char* buf);


/*==========================================================================

FUNCTION CHECK_HEARTBEAT()

DESCRIPTION

 Check to see if there is an active connection with the GW

DEPENDENCIES
  None.

RETURN VALUE
  0: Connection is active
  -1: Connection is down


SIDE EFFECTS
  None

==========================================================================*/
int check_heartbeat(int* err, char* buf){
    int ret, rc;
    LOG_MSG_INFO1("Check QMI_IP connection to GW",0,0,0);
    ret = SSL_write(ssl, "HEARTBEAT", strlen("HEARTBEAT"));

    if(ret > 0){
        /* Link is still up */
        ret = SSL_read(ssl, buf, sizeof(buf) - 1);

        if(ret > 0){
            rc = pthread_mutex_lock(&mutex);
            heartbeat_response = -1;
            rc = pthread_cond_broadcast(&cond);
            rc = pthread_mutex_unlock(&mutex);

            *err = ret;
            LOG_MSG_INFO1("QMI_IP connection to GW is still active",0,0,0);
            return 0;
        }
    }

    rc = pthread_mutex_lock(&mutex);
    heartbeat_response = 1;
    rc = pthread_cond_broadcast(&cond);
    rc = pthread_mutex_unlock(&mutex);

    acceptTCPConnection = 0;

    LOG_MSG_ERROR("QMI_IP connection to GW is down",0,0,0);

    return -1;
}

void get_odu_mode();
/*==========================================================================

FUNCTION GET_ODU_MODE()

DESCRIPTION

 Retrieves the current ODU mode.

DEPENDENCIES
  QCMAP_MSGR_HANDLE.

RETURN VALUE
  None.


SIDE EFFECTS
  None

==========================================================================*/
void get_odu_mode(){
    qmi_error_type_v01 qmi_error;
    qcmap_msgr_get_odu_mode_resp_msg_v01 get_mode_resp;
    memset(&get_mode_resp,0,
           sizeof(qcmap_msgr_get_odu_mode_resp_msg_v01));

    qmi_error = qmi_client_send_msg_sync(qmi_ip_conf.qmi_ip_qcmap_msgr_handle,
                       QMI_QCMAP_MSGR_GET_ODU_MODE_REQ_V01,
                       NULL,
                       0,
                       (void*)&get_mode_resp,
                       sizeof(qcmap_msgr_get_odu_mode_resp_msg_v01),
                       QMI_TIMEOUT);

    LOG_MSG_INFO1("qmi_client_send_msg_sync(get_odu_mode): error %d result %d",
      qmi_error, get_mode_resp.resp.result, 0);

    if ((qmi_error != QMI_NO_ERR) ||
          (get_mode_resp.resp.result != QMI_NO_ERR))
    {
      LOG_MSG_ERROR("Can not get ODU mode %d : %d. Setting default mode",
                    qmi_error, get_mode_resp.resp.error,0);
      device_mode = ROUTER_MODE;
    }
    else{
        if (get_mode_resp.mode == QCMAP_MSGR_ODU_BRIDGE_MODE_V01)
            device_mode = BRIDGE_MODE;
        else
           device_mode = ROUTER_MODE;
    }
}

/*===========================================================================
  FUNCTION  MAIN
===========================================================================*/
/*!
@brief

  Main function for QMI_IP.

@return

  - Starts the multicast listener
  - Sets up SSL connection
  - Handles packets from GW

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/

int main(int argc, char *argv[]){
    pthread_t           multicast_thread, qti_thread;
    int                 err;
    int                 qmi_error;
    int                 ret;
    int                 sd;
    int                 rc;
    int                 j = 0;
    int                 listen_sd;
    char                ipstr[1024];
    size_t              client_len;
    SSL_CTX*            ctx;
    X509*               client_cert;
    char*               str;
    unsigned char       buf[4096];
    const SSL_METHOD          *meth;
    struct sockaddr_in6 sa_serv;
    struct sockaddr_in  sa_cli;
    int                 msg_buf_size;
    unsigned char       *tmp_buf;
    unsigned char       *send_buf;
    unsigned char       temp_char;
    int                 reuse = 1;
    int                 flags;
    qmi_idl_service_object_type nas_qmi_idl_service_object;
    qmi_service_info            info[10];
    uint32_t num_services = 0, num_entries = 0;
    qmi_cci_os_signal_type     qmi_nas_os_params;
    qmi_client_type            qmi_nas_notifier;
    struct timeval             rcv_timeo;
    fd_set                     fd_set;
    netmgr_client_hdl_t  netmgr_hndl;

    multicast_server_finished = 1;
    acceptTCPConnection = 0;
    heartbeat_check = 0;
    shut_down = 0;
    sslConnecting = 0;
    ssl_active = 0;

    FILE *pswd_fp;
    char gw_password[MAX_PSWD_SIZE];
    char odu_password[MAX_PSWD_SIZE];

    LOG_MSG_INFO2("QMI_IP initialize", 0, 0, 0);

    qmi_ip_cmdq_init();

    /* Get QCMAP handle */
    memset(&qmi_ip_conf, 0, sizeof(qmi_ip_conf_t));
    qmi_ip_conf.state = QMI_IP_LINK_UP_WAIT;

    if(qmi_ip_qcmap_init(&qmi_ip_conf) == QMI_IP_ERROR)
    {
        LOG_MSG_ERROR("QCMAP unable to initialize, exiting",0,0,0);

        qmi_error = qmi_client_release(qmi_nas_handle);
        qmi_nas_handle = NULL;

        if (qmi_error != QMI_NO_ERR)
          LOG_MSG_ERROR("Can not release client nas handle %d", qmi_error, 0, 0);

        exit(2);
    }

    /* Now that we have the qcmap handle get the current ODU mode*/
    get_odu_mode();
    LOG_MSG_INFO1("device mode = %d", device_mode,0,0);

    ret = pthread_create(&qti_thread, NULL, (void (*)(void))init_qti_socket, NULL);

    /* Initialize DSI now, wait to get handle */
    if (DSI_SUCCESS != dsi_init(DSI_MODE_GENERAL))
    {
        LOG_MSG_ERROR("dsi_init failed, exit",0,0,0);
        exit(1);
    }

    if(netmgr_client_register(netmgr_ind_cb, NULL, &netmgr_hndl) == NETMGR_FAILURE){
        LOG_MSG_ERROR("netmgr_client_register failed",0,0,0);
        exit(1);
    }

    /* Create Multicast listener thread */
    init_ssl_thread_locks();
    ret = pthread_create(&multicast_thread, NULL, (void (*)(void))multicast_listener, NULL);

    /*     */
    /* SSL */
    /*     */
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
    meth = SSLv23_method();
    ctx = SSL_CTX_new (meth);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(2);
    }

    X509 *cert = NULL;
    EVP_PKEY *pkey = NULL;
    X509_NAME *name = NULL;
    RSA *keyPair = NULL;
    BIGNUM *bn = NULL;

    cert = X509_new();

    /* set time values */
    X509_gmtime_adj (X509_get_notBefore (cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);

    /* Set issue name */
    name = X509_get_subject_name (cert);
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)COUNTRY_CODE, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)ORG_NAME, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)COMMON_NAME, -1, -1, 0);
    X509_set_issuer_name(cert, name);

    keyPair = RSA_new();

    /* set big number */
    bn = BN_new();
    BN_set_word(bn, 65537);
    RSA_generate_key_ex(keyPair, 1024, bn, NULL);
    BN_free (bn);

    /* create private key */
    pkey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pkey, keyPair);
    keyPair = NULL;
    X509_set_pubkey(cert, pkey);
    X509_sign(cert, pkey, EVP_sha1());

    if (SSL_CTX_use_certificate(ctx, cert) <= 0) {
        ERR_print_errors_fp(stderr);
    }

    if (SSL_CTX_use_PrivateKey(ctx, pkey) <= 0) {
        ERR_print_errors_fp(stderr);
    }

     if (!SSL_CTX_check_private_key(ctx)) {
        LOG_MSG_ERROR("Private key does not match the certificate public key",0,0,0);
    }

     /* free public key */
    EVP_PKEY_free(pkey);

    while (!shut_down) {

        /* Only accept a TCP connection after the Multicast
           thread verfies the connection */
        rc = pthread_mutex_lock(&mutex);
        while (!acceptTCPConnection) {
            rc = pthread_cond_wait(&cond, &mutex);
        }
        rc = pthread_mutex_unlock(&mutex);

        /* stop listening for multicast connections while
           setting up the TCP connection */
        LOG_MSG_INFO1("TCP connection successfullyestablished..start new SSL connection",0,0,0);
        sslConnecting = 1;

        /* ----------------------------------------------- */
        /* Prepare TCP socket for receiving connections */
        memset(&ipstr[0], 0, sizeof(ipstr));
        listen_sd = socket (AF_INET6, SOCK_STREAM, 0);
        CHK_ERR(listen_sd, "socket");

        /* We want to be able to reuse this address */
        if (setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR,(char*)&reuse, sizeof(int)) == -1)
        {
            LOG_MSG_ERROR("setsockopt FAILED",0,0,0);
        }

        memset (&sa_serv, '\0', sizeof(sa_serv));
        sa_serv.sin6_family      = AF_INET6;
        sa_serv.sin6_addr        = in6addr_any;
        sa_serv.sin6_port        = htons (5006);
        err = bind(listen_sd, (struct sockaddr*) &sa_serv, sizeof (sa_serv));
        CHK_ERR(err, "bind");

        /* Receive a TCP connection. */
        err = listen (listen_sd, 1);
        CHK_ERR(err, "listen");

        inet_ntop(AF_INET6, &sa_serv.sin6_addr, ipstr, 46);
        client_len = sizeof(sa_cli);
        //this needs to be non-blocking.. with a timeout of 5 seconds
        sd = accept (listen_sd, (struct sockaddr*) &sa_cli, &client_len);

        /* We want to be able to reuse this address */
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,(char*)&reuse, sizeof(int)) == -1)
        {
            LOG_MSG_ERROR("setsockopt FAILED",0,0,0);
        }

         /* set timeout value */
        rcv_timeo.tv_sec = 0;
        rcv_timeo.tv_usec = 100000;
        setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
        flags = fcntl(sd, F_GETFL, 0);
        fcntl(sd, F_SETFL, flags);

        close (listen_sd);

        LOG_MSG_INFO1("Connection from %lx, port %x\n", sa_cli.sin_addr.s_addr, sa_cli.sin_port,0);

        /* ----------------------------------------------- */
        /* TCP connection is ready. Do server side SSL. */
        ssl = SSL_new (ctx);
        SSL_set_fd (ssl, sd);

        int i = 0;
        do{
            if (i > 0)
                sleep(1);
            err = SSL_accept (ssl);

            if (err < 1)
                LOG_MSG_ERROR("SSL_accept error %d",err,0,0);
        }while (err < 1 && i++ < SSL_ACCEPT_DELAY);

        if (err < 0){
            SSL_get_error(ssl, err);
            sslConnecting = 0;
            goto retry_connection;
        }

        /* Get the cipher - opt */
        LOG_MSG_INFO1("SSL connection using %s", SSL_get_cipher (ssl),0,0);

        /* Get client's certificate (note: beware of dynamic allocation) - opt */
        client_cert = SSL_get_peer_certificate (ssl);
        if (client_cert != NULL) {
            str = X509_NAME_oneline (X509_get_subject_name (client_cert), 0, 0);
            LOG_MSG_INFO1 ("\t subject: %s\n", str,0,0);
            OPENSSL_free (str);

            str = X509_NAME_oneline (X509_get_issuer_name  (client_cert), 0, 0);
            LOG_MSG_INFO1 ("\t issuer: %s", str,0,0);
            OPENSSL_free (str);

            /* We could do all sorts of certificate verification stuff here before
               deallocating the certificate. */

            X509_free (client_cert);
        } else{
            LOG_MSG_ERROR ("Client does not have certificate. Wait for next connection",0,0,0);

            X509_free (client_cert);
            //goto retry_connection; //not currently sending cert with test app
        }

        /* SSL connection is active, do password verification */
        sslConnecting = 0;

        LOG_MSG_INFO1("SSL connection successful..start authentication",0,0,0);
        /* wait for GW to send password */
        do{
            err = SSL_read(ssl, gw_password, sizeof(gw_password) - 1);

            if(err < 0 && check_ssl_error(err) == SSL_ERROR_ZERO_RETURN){
                goto retry_connection;
            }
        }while(err <= 0);

        if (strlen(gw_password) > MAX_PSWD_SIZE)
                goto retry_connection;

        pswd_fp = fopen(PASSWORD_FILE, "r");
        if (pswd_fp != NULL) {

            if (!fgets(odu_password, MAX_PSWD_SIZE, pswd_fp)) {
                LOG_MSG_ERROR("Passsword does not exist in file, device must be reset",0,0,0);
                close(pswd_fp);

                send_buf = malloc(SIZE_16_BIT_VAL);
                ds_assert(send_buf != NULL);

                tmp_buf = send_buf;
                WRITE_16_BIT_VAL(tmp_buf, QMI_ERR_AUTHENTICATION_FAILED_V01);
                err = SSL_write(ssl, send_buf, SIZE_16_BIT_VAL);

                free(send_buf);
                tmp_buf = NULL;
                goto retry_connection;
            }
            close(pswd_fp);

            if (strncmp(gw_password, odu_password, strlen(odu_password)) != 0 &&
                (strlen(gw_password) != strlen(odu_password))) {
                LOG_MSG_ERROR("Passswords missmatch, invalid connection",0,0,0);
                goto retry_connection;
            }
            else
                LOG_MSG_INFO1("Authentication with H-GW successful",0,0,0);
          } else{
            if ((ret = strncmp(gw_password, DEFAULT_PSWD, strlen(DEFAULT_PSWD))) != 0) {
                LOG_MSG_ERROR("Passswords missmatch, invalid connection: %s %s %d",
                              gw_password, DEFAULT_PSWD, ret);

                send_buf = malloc(SIZE_16_BIT_VAL);
                ds_assert(send_buf != NULL);

                tmp_buf = send_buf;
                WRITE_16_BIT_VAL(tmp_buf, QMI_ERR_AUTHENTICATION_FAILED_V01);
                err = SSL_write(ssl, send_buf, SIZE_16_BIT_VAL);

                free(send_buf);
                tmp_buf = NULL;
                goto retry_connection;
            }
            else
             LOG_MSG_INFO1("Authentication with H-GW successful using default password",0,0,0);
          }

        /* Initialize NAS */
        nas_qmi_idl_service_object = nas_get_service_object_v01();
        if (nas_qmi_idl_service_object == NULL)
        {
            LOG_MSG_ERROR("nas_qmi_idl_service_object failed!",0,0,0);
            goto retry_connection;
        }
        qmi_error = qmi_client_notifier_init(nas_qmi_idl_service_object,
                                           &qmi_nas_os_params,
                                           &qmi_nas_notifier);

        /* We must wait for the service object to be populated before getting NAS handle */
        while(1)
        {
            qmi_error = qmi_client_get_service_list(nas_qmi_idl_service_object,
                                                    NULL,
                                                    NULL,
                                                    &num_services);
            LOG_MSG_INFO1("qmi_client_get_service_list: %d", qmi_error,0,0);
            if(qmi_error == QMI_NO_ERR)
              break;
            /* wait for server to come up */
            QMI_CCI_OS_SIGNAL_WAIT(&qmi_nas_os_params, 0);
        }

        if (num_services == 0)
        {
            LOG_MSG_ERROR("Num services 0",0,0,0);
            goto retry_connection;
        }

        /* Num entries must be equal to num services */
        num_entries = num_services;
        qmi_error = qmi_client_get_service_list(nas_qmi_idl_service_object,
                                              info,
                                              &num_entries,
                                              &num_services);

        /* Now that the service object is populated get the NAS handle */
        qmi_error = qmi_client_init(&info[0],
                                  nas_qmi_idl_service_object,
                                  nas_ind_cb,
                                  NULL,
                                  NULL,
                                  &qmi_nas_handle);

        LOG_MSG_INFO1("Successfully initialized the NAS client",0,0,0);

        /* Get WDS handles */
        //tetherd instance only needed in bridge mode
        if(device_mode == BRIDGE_MODE)
            ret = qmi_ip_wds_init(&(qmi_ip_conf.autoconnect_conf), ssl,
                                  QMI_CLIENT_QMUX_RMNET_INSTANCE_8);

        if (ret == QMI_IP_ERROR){
            LOG_MSG_ERROR("WDS tethered instance failed",0,0,0);
            goto retry_connection;
        }

        /* embedded instance always needed */
        do{
            LOG_MSG_INFO1("WDS embedded instance %d",
                          QMI_CLIENT_QMUX_RMNET_INSTANCE_7 - j,0,0);
            ret = qmi_ip_wds_init(&qmi_ip_conf, ssl,
                                  QMI_CLIENT_QMUX_RMNET_INSTANCE_7 - j++);
        }while(ret == QMI_IP_ERROR && j <= MAX_QMI_EMB_INSTANCES);

        if (j >= MAX_QMI_EMB_INSTANCES && ret == QMI_IP_ERROR){
            LOG_MSG_ERROR("WDS embedded instance failed",0,0,0);
            goto retry_connection;
        }
        embms_rmnet_link = (int) (QMI_CLIENT_QMUX_RMNET_INSTANCE_7 - QMI_CLIENT_QMUX_RMNET_INSTANCE_0 - j - 1);

        ssl_active = 1;
        while(ssl_active)
        {
            memset(&buf, 0, 4096);
            if (heartbeat_check && check_heartbeat(&err, buf) == -1)
                break;
            else
                err = SSL_read(ssl, buf, sizeof(buf) - 1);

            if (err > 0) {
                tmp_buf = buf;

                unsigned long rx_iface;
                unsigned long rx_client_id;
                unsigned long rx_length;
                unsigned long rx_service_type;
                unsigned long rx_ctrl_flags;
                unsigned long rx_tx_id;
                unsigned long qmi_message_id;
                unsigned long qmi_msg_length;

                //change this to a macro
                READ_8_BIT_VAL(tmp_buf, temp_char); //should be hardcoded to 0x00
                rx_iface = (unsigned long) temp_char;
                READ_16_BIT_VAL(tmp_buf, temp_char);
                rx_length = (unsigned long) temp_char;
                READ_8_BIT_VAL(tmp_buf, temp_char);
                rx_ctrl_flags = (unsigned long) temp_char; //should be hardcoded to 0x00
                READ_8_BIT_VAL(tmp_buf, temp_char);
                rx_service_type = (unsigned long) temp_char;
                READ_8_BIT_VAL(tmp_buf, temp_char);
                rx_client_id = (unsigned long) temp_char;
                READ_8_BIT_VAL(tmp_buf, temp_char);
                rx_ctrl_flags = (unsigned long) temp_char; //00-req,01-resp,10-ind
                READ_16_BIT_VAL(tmp_buf, temp_char);
                rx_tx_id = (unsigned long) temp_char;
                READ_16_BIT_VAL(tmp_buf, temp_char);
                qmi_message_id = (unsigned long) temp_char;
                READ_16_BIT_VAL(tmp_buf, temp_char);
                qmi_msg_length = (int) temp_char;

                ret = 0;
                /* Process the message based on the service type */
                switch (rx_service_type) {
                case QMI_WDS_SERVICE_TYPE:
                    ret = proccess_wds_message(&tmp_buf, qmi_message_id, (int)qmi_msg_length, rx_tx_id);
                    break;
                case QMI_NAS_SERVICE_TYPE:
                    ret = proccess_nas_message(&tmp_buf, qmi_message_id, (int)qmi_msg_length, rx_tx_id);
                    break;
                case QMI_IP_ODU_SERVICE_TYPE:
                    ret = proccess_odu_message(&tmp_buf, qmi_message_id, (int)qmi_msg_length, rx_tx_id);
                    break;
                default:
                    LOG_MSG_ERROR("INCORRECT SERVICE_TYPE: 0x%02x\n", rx_service_type,0,0);
                    break;
                }

                if (ret == -1) {
                    LOG_MSG_ERROR("Processing of message failed, type %d", rx_service_type,0,0);
                }
            }
            else if((ret = check_ssl_error(err)) == SSL_ERROR_ZERO_RETURN){
                ssl_active = 0;
            }
            else if(ret == SSL_ERROR_WANT_WRITE){
                //this should not happen, but we will have to loop here until want write is completed
            }
            else if (ret == SSL_ERROR_SYSCALL) {
                LOG_MSG_ERROR("premature close: SSL_ERROR_SYSCALL recieved %d %d", ERR_get_error(), errno,0);
                //ssl_active = 0;
                /*SSL_ERROR_SYSCALL
                Some I/O error occurred. The OpenSSL error queue may contain more information on the error.
                If the error queue is empty (i.e. ERR_get_error() returns 0), ret can be used to find out
                more about the error: If ret == 0, an EOF was observed that violates the protocol. If ret == -1,
                the underlying BIO reported an I/O error (for socket I/O on Unix systems, consult errno for details).*/
            }
        }

        /* Clean up. */
retry_connection:
        LOG_MSG_INFO1("Tear down connection",0,0,0);

       /*if device is in bridge mode, we should release WDS clients*/
        if(device_mode == BRIDGE_MODE)
            qmi_ip_wds_release(&(qmi_ip_conf.autoconnect_conf));

        /* Tear down eMBMS call if UP */
        if(embms_call_state == UP)
        {
          disable_embms();
        }

        /* Release the eMBMS WDS client */
        qmi_ip_wds_release(&qmi_ip_conf);

        acceptTCPConnection = 0;

        /* Release NAS client*/
        qmi_error = qmi_client_release(qmi_nas_handle);
        qmi_nas_handle = NULL;

        if(qmi_error != QMI_NO_ERR)
        {
          LOG_MSG_ERROR("Can not release client nas handle %d",qmi_error,0, 0);
        }
        else
        {
          LOG_MSG_INFO1("Successfully released the nas client handle",0,0,0);
        }

        /* Shutdown SSL */
        LOG_MSG_INFO1("shutting down the SSL",0,0,0);

        SSL_shutdown(ssl);
        close (sd);
        SSL_free (ssl);
    }
    SSL_CTX_free (ctx);

    if(netmgr_client_release(netmgr_hndl) == NETMGR_FAILURE){
        LOG_MSG_ERROR("netmgr_client_release failed",0,0,0);
    }
}
