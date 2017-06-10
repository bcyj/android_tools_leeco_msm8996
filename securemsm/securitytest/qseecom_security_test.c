/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2012 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
QSEECom Security Test
*********************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/msm_ion.h>
#include <utils/Log.h>
#include "QSEEComAPI.h"
#include "common_log.h"
#include <sys/mman.h>
#include <semaphore.h>

/** adb log */
#undef LOG_TAG
#undef LOG_NDDEBUG
#undef LOG_NDEBUG
#define LOG_TAG "QSEECOM_SECURITY_TEST: "
#define LOG_NDDEBUG 0 /* Define to enable LOGD */
#define LOG_NDEBUG  0 /* Define to enable LOGV */

/********************************************************
 * SECURITY TEST MACROS/DEFINES
 ********************************************************/

#define CHECK_RET(x, uid, log)  if( x != 0 ) { if( log ) { LOGD( "T%#X, U%u: Return %i, errno: %u", (uint32_t)threadid, uid, x, errno ); g_err = -1; } break; }
#define CHECK_RETURN(x, uid)    if( g_err ) { break; } else { CHECK_RET(x, uid, 1); }
#define GET_BUFFER(x,y)       (((qseecom_req_res_t*)g_qseeCommHandles[x][y]->ion_sbuffer)->buf)

#define GET_LSTNR_SVC_ID(parent_tid,tid)  (100 + (parent_tid * LSTNRS_PER_CLNT) + (tid-1))

/* Various parameters used during the tests */
#define NUM_CLIENTS     2
#define LSTNRS_PER_CLNT 2
#define NUM_CMD_TESTS   1000

#if ( (NUM_CLIENTS * LSTNRS_PER_CLNT) > 8 )
  #error  QSEE only supports 10 listeners, some of which are needed by the service daemons
#endif

/* Feature define for memory access attempts; until B-family devices, accessing
 * XPU protected regions causes the chip to lock up.  The solution, is to
 * specify at runtime whether to run these tests, and limit the breadth.
 */
#define MEM_ACCESS_ATTEMPTS_VIA_ARGS  1

/* So this address can be used as an initializer */
#define PROTECTED_MEM_ADDR_0  
#define PROTECTED_MEM_ADDR_1  

#define REQ_RES_BUF_SIZE      1000

/********************************************************
 * SECURITY TEST TYPES
 ********************************************************/

/**
 * Various security actions to be performed
 */
enum security_test_e {
  SECURITY_ACTION_REPLY = 0,      /* Operation performed on the request buffer */
  SECURITY_ACTION_WAKE_LISTENER,  /* Request buffer to be passed to listener (ID in 'data' field) */
};

/**
 * Security Test request and response structures
 */
typedef struct qseecom_req_res_s
{
  uint32_t  cmd;
  uint32_t  data;
  uint32_t  buf[REQ_RES_BUF_SIZE];
} __attribute__ ((packed)) qseecom_req_res_t;

/**
 * Struct to mess with the QSEEComHandle
 */
typedef struct
{
  unsigned char *ion_sbuffer;
  uint32_t len;
} QSEEComHandlePriv_t;

/********************************************************
 * SECURITY TEST STATICS
 ********************************************************/

/* Make the QSEEComm handles public to allow other threads access to the
 * ION allocated buffers; the additional handle is for the client thread itself
 */
static struct QSEECom_handle *g_qseeCommHandles[NUM_CLIENTS][LSTNRS_PER_CLNT+1];

/* Array storing the various XOR values to be used on the data by the listeners */
static uint32_t g_xors[NUM_CLIENTS][LSTNRS_PER_CLNT+1];

/* Memory access attempts will be run on the locations specificed in this array */
const uint32_t g_protected_memory[] = {
#if defined( QCEECOM_TARGET_MSM8960 )
  0x2a000000,   /* TZ Code Base; access attempts cause the device to lock up */
  0x2e000000,   /* Multi-media sub-system */
  0x8D400000,   /* Modem sub-system, firmware */
  0x89000000,   /* Modem sub-system, software */
  0x8F200000,   /* Riva sub-system */
  0x8DA00000,   /* LPASS sub-system */
#endif
  0x88F00000    /* QSApp location; only protected after the QSApp is loaded */
};

/* File handle to /dev/mem */
static int mem_fd;

/* Which protected memory address, from g_protected_memory[] should be tested */
static uint32_t g_mem_access_arg = (uint32_t)(-1);

/* Whether the random QSApp test should be run */
static uint32_t g_qsapp_rnd = 0;

/* Offset to be applied in order to get user virtual to kernel physical */
static uint32_t g_user_va_to_krnl_pa = 0;

/* Barriers to wait until the listeners are ready */
static sem_t barrier[NUM_CLIENTS];

/* Global Error flag to indicate if any of the tests failed */
static int g_err = 0;

/* Helper function to check if a memory location can be accessed;
 * return negative on access success, 0 otherwise
 */
int attempt_memory_access( uint32_t addr, bool do_mmap )
{
  void *mem_addr = NULL;
  int32_t ret = 0;

  /* Since we're executing in user space (but as root), we need to mmap the
   * memory into our local address space
   */
  LOGD( "Attempting to access %#X...", addr );
  if( do_mmap ) {
    mem_addr = mmap( NULL, 4096, (PROT_READ | PROT_WRITE), MAP_SHARED, mem_fd, addr );
    if( mem_addr == MAP_FAILED ) {
      LOGD( "Failed to mmap with error: %i", errno );
      ret = -1;
    } else if (mem_addr == NULL) {
      LOGE("Could not get memory mapped address, exiting now!\n");
      return -1;
    }else {
      LOGD( "mmap succeed; address: %#X", (uint32_t)mem_addr );
    }
  }
  if(mem_addr == NULL) {
      LOGE("Null Memory address, exiting");
      return -1;
  }
  do {
    volatile uint32_t *mem = (volatile uint32_t *)mem_addr;

    /* Attempt an access */
    *mem = 0xDEADBEEF;
    if( *mem == 0xDEADBEEF ) {
      LOGD( "write successful: #%X", *mem );
      ret = -777777;
    }
  } while( 0 );

  if( do_mmap ) {
    munmap( mem_addr, 4096 );
  }

  return ret;
}

/* Helper function to set the request buffer to a known pattern, which
 * will be verified once the response is received
 */
void set_buffer( uint32_t *buf, uint32_t len, uint32_t seed )
{
  uint32_t i;
  if( buf == 0 ) {
    LOGD( "BUFFER IS NULL! %u", gettid() );
    return;
  }
  for( i = 0; i < len; i++ ) {
    buf[i] = seed + i;
  }
}

/* Helper function to verify the final value of the buffer supplied
 * during each test; returns 0 on success, -777777 otherwise
 */
int check_buffer( qseecom_req_res_t *req_res, uint32_t tid )
{
  uint32_t i;
  for( i = 0; i < ( sizeof(req_res->buf) / sizeof(uint32_t) ); i++ ) {
    if( req_res->buf[i] != ( (tid + i) ^ req_res->data) ) {
      LOGD( "Expected %#x, read %#x, xor %#x, tid %#x, i %#x",
        ((tid + i) ^ req_res->data), req_res->buf[i], req_res->data, tid, i );
      return -777775;
    }
  }
  return 0;
}

/* Thread to act as a listener  */
void *listener_thread( void* threadid )
{
  uint32_t parent_tid = (((uint32_t)threadid) >> 16);
  uint32_t tid = ((uint32_t)threadid) & 0xFFFF;
  qseecom_req_res_t *req_res;
  int32_t ret;
  uint32_t i;

  do {
     if(parent_tid >= NUM_CLIENTS) {
       LOGE("Number of clients is %d, max allowed is %d\n",parent_tid,NUM_CLIENTS);
       return -1;
     }
     if(tid > LSTNRS_PER_CLNT) {
       LOGE("Number of Listeners per client is %d, max allowed %d\n",tid, LSTNRS_PER_CLNT);
       return -1;
     }
    /* Register as a listener with the QSApp */
    LOGD( "L%#X: Registering as listener with QSApp...", (uint32_t)threadid );
    ret = QSEECom_register_listener( &g_qseeCommHandles[parent_tid][tid], GET_LSTNR_SVC_ID(parent_tid, tid),
            sizeof(qseecom_req_res_t), 0 );

    /* Before we check the return, unblock the parent in case we failed and are exiting */
    sem_post( &barrier[parent_tid] );

    CHECK_RETURN( ret, __LINE__ );

    /* Overlay the request/response structure on the ion allocated memory */
    req_res = (qseecom_req_res_t*)g_qseeCommHandles[parent_tid][tid]->ion_sbuffer;
    LOGD( "L%#X: Got buffer @ %#x", (uint32_t)threadid, (uint32_t)req_res );


    for( ;; ) {
      /* Wait for request from the QSApp */
      ret = QSEECom_receive_req( g_qseeCommHandles[parent_tid][tid], req_res, sizeof(qseecom_req_res_t) );
      if( ret ) break;

      /* Attempt to read the parents request and response buffers, MUST be all zeros.
       * This ensures that XPU protection is in place during the send_cmd() call.
       */
      for( i = 0; i < REQ_RES_BUF_SIZE; i++ ) {
        CHECK_RETURN( ( GET_BUFFER( parent_tid, 0 )[i] != 0 ), __LINE__ );
      }

      /* Tamper with all of the other buffers, and check that it has no impact.
       * This ensures that non-secure memory is fully in secure land before being used.
       */
      for( i = 0; i <= LSTNRS_PER_CLNT; i++ ) {
        if( tid == i ) continue;
        set_buffer( GET_BUFFER( parent_tid, i ), REQ_RES_BUF_SIZE, 0xDEADBEEF );
      }

      /* Modify the given data, using the random XOR pattern */
      for( i = 0; i < REQ_RES_BUF_SIZE; i++ ) {
        req_res->buf[i] ^= g_xors[parent_tid][tid];
      }

      /* If we are the last listener, end the chain */
      if( req_res->data == GET_LSTNR_SVC_ID(parent_tid, LSTNRS_PER_CLNT) ) {
        req_res->data = 0;
      } else {
        /* Not the last; simply increment and pass it forward! */
        req_res->data++;
      }

      /* Send the response to the QSApp */
      ret = QSEECom_send_resp( g_qseeCommHandles[parent_tid][tid], req_res, sizeof(qseecom_req_res_t) );
      CHECK_RETURN( ret, __LINE__ );
    }
  } while( 0 );

  pthread_exit( NULL );
  return NULL;
}

/* Function/thread to perform the various security tests */
void *test_thread( void* threadid )
{
  uint32_t tid = (uint32_t)threadid;
  qseecom_req_res_t *req, *res;
  int32_t ret;
  pthread_t threads[LSTNRS_PER_CLNT];
  uint32_t i, j, culm_xor;

  do {
    /* -----------------------------------------
     * T1) Bad Arguments - Start App
     * -----------------------------------------
     */
    /* Attempt to load the "corrupted" QSApp - this will verify the PIL feature is working correctly */
    if( g_qsapp_rnd ) {
      ret = QSEECom_start_app( &g_qseeCommHandles[tid][0], "/firmware/image/rnd",
              "securitytest", sizeof(qseecom_req_res_t)*2 );
      QSEECom_shutdown_app( &g_qseeCommHandles[tid][0] );
      CHECK_RETURN( (ret == 0), __LINE__ );
    }

    /* -----------------------------------------
     * Open a connection to the QSApp
     * -----------------------------------------
     */
    LOGD( "T%#X: Starting QSApp...", (uint32_t)threadid );
    ret = QSEECom_start_app( &g_qseeCommHandles[tid][0], "/firmware/image",
            "securitytest", sizeof(qseecom_req_res_t)*2 );
    LOGD( "T%#X: Started QSApp...", (uint32_t)threadid );
    CHECK_RETURN( ret, __LINE__ );

    /* Attempt to access the protected memory regions (done after the load to ensure the
     * QSApp location is locked)
     */
  #if defined( MEM_ACCESS_ATTEMPTS_VIA_ARGS )
    if( g_mem_access_arg == 0 ) {
      i = g_mem_access_arg;
  #else
    for( i = 0; i < (sizeof(g_protected_memory) / sizeof(g_protected_memory[0])); i++ ) {
  #endif
      CHECK_RETURN( attempt_memory_access( g_protected_memory[i], 1 ), __LINE__ );
    }

    /* Overlay the request/response structure on the ion allocated memory */
    req = (qseecom_req_res_t*)g_qseeCommHandles[tid][0]->ion_sbuffer;
    res = req + 1;

    /* -----------------------------------------
     * Spawn the listener threads for the tests
     * -----------------------------------------
     */
    for( j = 0; j < LSTNRS_PER_CLNT; j++ ) {
      ret = pthread_create( &threads[j], NULL, &listener_thread, (void*)((tid << 16) | (j+1)) );
      sem_wait( &barrier[tid] );
    }

    for( i = 0; i < NUM_CMD_TESTS; i++ ) {
      /* -----------------------------------------
       * T2) Bad Arguments - Send Cmd
       * -----------------------------------------
       */
      /* This test is a bit tricky because the user virtual address goes through several levels of
       * conversions to get to the physical address passed to QSEE.  However, if the conversion
       * factor is given, the ION handle is modified to appear as though the new address
       * was the one originally allocated.  In this way, it is possible to pass in a secure address as
       * the buffer pointers to the QSApp.
       */
      if( g_user_va_to_krnl_pa ) {
        QSEEComHandlePriv_t saved_handle, *hack_handle;
        hack_handle = (QSEEComHandlePriv_t*)g_qseeCommHandles[tid][0];
        saved_handle = *hack_handle;
        for( j = 0; j < (sizeof(g_protected_memory) / sizeof(g_protected_memory[0])); j++ ) {
          void* addr;
          addr = (void*)((uint32_t)g_protected_memory[j] - g_user_va_to_krnl_pa + (uint32_t)saved_handle.ion_sbuffer);
          hack_handle->ion_sbuffer = (unsigned char*) addr;
          hack_handle->len = 0x1024 + 0x20;
          LOGD( "Modified %#X to be %#X", (uint32_t)g_protected_memory[j], (uint32_t)addr );
          ret = QSEECom_send_cmd( g_qseeCommHandles[tid][0], addr, 0x1024, addr, 0x20 );
          /* TODO: API needs to be updated to pass along this error, but verified manually */
          /* CHECK_RETURN( (ret == 0), __LINE__ ); */
        }
        *hack_handle = saved_handle;
      }

      /* -----------------------------------------
       * T3) Simple Test - Request/Response
       * -----------------------------------------
       */
      req->cmd = SECURITY_ACTION_REPLY;
      req->data = 0;
      set_buffer( req->buf, REQ_RES_BUF_SIZE, tid );
      ret = QSEECom_send_cmd( g_qseeCommHandles[tid][0], req, sizeof(qseecom_req_res_t),
              res, sizeof(qseecom_req_res_t) );
      CHECK_RETURN( ret, __LINE__ );
      CHECK_RETURN( check_buffer( res, tid ), __LINE__ );

      /* -----------------------------------------
       * T4) Complex Test - Multiple Listeners
       * -----------------------------------------
       */
      /* Set-up the XOR values to be used by the listeners */
      for( j = 1, culm_xor = 0; j <= LSTNRS_PER_CLNT; j++ ) {
        g_xors[tid][j] = rand();
        culm_xor ^= g_xors[tid][j];
      }

      /* Kick of the tests with a send_cmd, indicating which listener should be notified next */
      req->cmd = SECURITY_ACTION_WAKE_LISTENER;
      req->data = GET_LSTNR_SVC_ID(tid, 1); /* First listener */
      set_buffer( req->buf, REQ_RES_BUF_SIZE, tid );
      ret = QSEECom_send_cmd( g_qseeCommHandles[tid][0], req, sizeof(qseecom_req_res_t),
              res, sizeof(qseecom_req_res_t) );
      res->data = culm_xor;   /* The total modifier applied to the buffer */
      /* Has to be done before the 'ret' check since g_err might already be set */
      CHECK_RETURN( check_buffer( res, tid ), __LINE__ );
      CHECK_RETURN( ret, __LINE__ );
    }

    /* -----------------------------------------
     * Unregister the listeners and wait for them to complete
     * -----------------------------------------
     */
    for( j = 0; j < LSTNRS_PER_CLNT; j++ ) {
      void *pthread_ret = NULL;
      if( g_qseeCommHandles[tid][j+1] != NULL ) {
        QSEECom_unregister_listener( g_qseeCommHandles[tid][j+1] );
      }
      ret = pthread_join( threads[j], &pthread_ret );
    }

    /* -----------------------------------------
     * Stop the QSApp
     * -----------------------------------------
     */
    if ( g_qseeCommHandles[tid][0] != NULL ) {
      QSEECom_shutdown_app( &g_qseeCommHandles[tid][0] );
    }
  } while( 0 );

  pthread_exit( NULL );
  return NULL;
}

int main( int argc, char *argv[] )
{
  /* Handle to mmap'd /dev/mem for attempting accesses */
  int ret = 0;
  uint32_t i, j;
  pthread_t threads[NUM_CLIENTS] = {0};
  void *pthread_ret = NULL;
  struct stat st;
  char rndFileStr[128];

  /* Initialize the global/statics to zero */
  memset( g_qseeCommHandles, 0, sizeof(g_qseeCommHandles) );
  memset( g_xors, 0, sizeof(g_xors) );

  /* Any arguments passed to the program? */
  for( i = 1; i < (uint32_t)argc; i++ ) {
    switch( argv[i][0] ) {
      case 'r': /* Randomize part of the load file */
        /* No parameters */
        LOGD( "Running QSApp randomization test" );

        /* Make a "corrupted" QSApp to test the authentication */
        stat("/firmware/image/securitytest.b03", &st);
        snprintf( rndFileStr, sizeof(rndFileStr),
		"dd if=/dev/urandom of=/firmware/image/rnd/securitytest.b03 bs=%u count=1",
		(uint32_t)st.st_size );
        system( "mkdir /firmware/image/rnd" );
        system( "cat /firmware/image/securitytest.mdt > /firmware/image/rnd/securitytest.mdt" );
        system( "cat /firmware/image/securitytest.b00 > /firmware/image/rnd/securitytest.b00" );
        system( "cat /firmware/image/securitytest.b01 > /firmware/image/rnd/securitytest.b01" );
        system( "cat /firmware/image/securitytest.b02 > /firmware/image/rnd/securitytest.b02" );
        system( rndFileStr );

        /* Mark that the tests should be run */
        g_qsapp_rnd = true;
        break;
    #if defined( MEM_ACCESS_ATTEMPTS_VIA_ARGS )
      case 'p': /* Protected memory access attempt */
        i++;  /* Skip to the next arg */
        if( (uint32_t)argc > i ) {
          if( argv[i][0] >= '0' && argv[i][0] <= '9' ) {
            g_mem_access_arg = (uint32_t)(argv[i][0] - '0');
            LOGD( "Will attempt to access protected memory at index %u: %#X", g_mem_access_arg,
                    g_protected_memory[ g_mem_access_arg ] );
          }
        }
        break;
    #endif
      case 'b': /* Bad buffer input (i.e. pointer inside of secure memory) */
        i++;  /* Skip to the next arg */
        if( (uint32_t)argc > i ) {
          errno = 0;  /* Attempt to get the address */
          g_user_va_to_krnl_pa = strtol( argv[i], NULL, 16 );
          if( !errno && g_user_va_to_krnl_pa ) {
            LOGD( "Using user virtual address to physical address offset of %#X", g_user_va_to_krnl_pa );
          }
        }
        break;
      default:
        break;
    }
  }

  /* Open /dev/mem for reading and writing */
  mem_fd = open("/dev/mem", O_RDWR);
  if( mem_fd < 0 ) {
      LOGD( "Can't open /dev/mem" );
      return -1;
  }

  if( !g_err ) {
    /* Spawn client threads to run through the various tests */
    for( j = 0; j < NUM_CLIENTS; j++ ) {
      /* Initialize the barriers to ensure that commands aren't sent before the listeners
       * have been started.  Otherwise, errors will be generated.
       */
      ret = sem_init( &barrier[j], 0, 0 );
      if( ret ) {
        LOGD( "barrier init failed %i, %i", ret, errno );
        g_err = -1;
        break;
      }

      ret = pthread_create( &threads[j], NULL, &test_thread, (void*)j );
    }

    /* Wait for the threads to complete */
    for( j = 0; j < NUM_CLIENTS; j++ ) {
      ret = pthread_join( threads[j], &pthread_ret );
    }
  }

  /* Done with the /dev/mem file */
  close( mem_fd );

  LOGD( "Exiting... g_err: %i", g_err );
  return g_err;
}
