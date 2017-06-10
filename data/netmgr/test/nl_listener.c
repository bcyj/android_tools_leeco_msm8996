/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include "netmgr.h"
#include "netmgr_defs.h"
#include "netmgr_test.h"
#include "netmgr_util.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

LOCAL netmgr_client_hdl_t  client_hndl = 0;

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/


static void netmgr_test_event_cb
(
  netmgr_nl_events_t       event,
  netmgr_nl_event_info_t * info,
  void *                   data
)
{
  static unsigned int cnt = 0;
  
  fprintf(stderr, "Received new indication[%d]: event=%d "
                  "link=%d flow=%d data=%p\n",
                  cnt++, event, info->link, info->flow_info.flow_id, (unsigned int*)data );
}


void netmgr_test_exithandler(void)
{
  /* Release client for NEtMgr events */
  if( NETMGR_SUCCESS !=
      netmgr_client_release( client_hndl ) ) {
    fprintf(stderr, "Error on netmgr_client_release\n");
  }
  fprintf(stderr, "Client release successful, handle=0x%08x\n",
                  client_hndl );
}


int main(int argc, char ** argv)
{
  (void)argc; (void)argv;

  /* Register as a client for NEtMgr events */
  if( NETMGR_SUCCESS !=
      netmgr_client_register( netmgr_test_event_cb, (void*)0x99, &client_hndl ) ) {
    NETMGR_ABORT("Error on netmgr_client_register\n");
    return NETMGR_FAILURE;
  }

  fprintf(stderr, "Client registration successful, handle=0x%08x\n",
                  client_hndl );

  /* Wait on listener thread */
  fprintf(stderr, "Waiting on socket listening thread...\n" );
  netmgr_client_thread_wait();
  
  return NETMGR_SUCCESS;
}
