/* test.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "stats_module.h"
#if 0
void testsuite_1()
{ /* Basic bringup */
  fprintf (stderr, "____________BASIC_BRINGUP______________\n");
mct_module_t *mod;
     mct_port_t *port;
   mct_event_t event;
     char name[10] = "stats";
     fprintf (stderr, "calling stats module init\n");
     mod =module_stats_init(name);
     if(mod == NULL)
       fprintf (stderr, "stats module init failed\n");
     port = MCT_PORT_CAST(MCT_MODULE_SINKPORTS(mod)->data);
       fprintf (stderr, "stats src port name %s parent module ptr %p module %p\n", MCT_PORT_NAME(port),
       MCT_PORT_PARENT(port), mod);
   fprintf (stderr, "test caps reserve\n");
     mct_port_caps_t caps;
     mct_stream_info_t stream_info;
   /*MCT_PORT_CAPS_OPAQUE,
  MCT_PORT_CAPS_STATS,*/
   caps.port_caps_type =MCT_PORT_CAPS_STATS;
   caps.u.stats.flag = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS |\
                           MCT_PORT_CAP_STATS_HIST);
   stream_info.identity = 1;
     if (port->check_caps_reserve(port, &caps, &stream_info) == FALSE)
     fprintf (stderr, "port_caps reserve failed\n");
   if (port->ext_link(stream_info.identity,port,port) == FALSE) {
     fprintf (stderr, "port ext link failed\n");
   }
   mod->start_session(mod, stream_info.identity);
   event.identity = stream_info.identity;
   event.direction = MCT_EVENT_DOWNSTREAM;
   /*MCT_EVENT_CONTROL_CMD*/
   event.type = MCT_EVENT_MODULE_EVENT;
   event.u.module_event.type = MCT_EVENT_MODULE_STATS_DATA;
   port->event_func(port,&event);
   sleep(1);
   mod->stop_session(mod, stream_info.identity);
   port->un_link(stream_info.identity, port, port);

  if (port->check_caps_unreserve(port,stream_info.identity) == FALSE)
     fprintf (stderr, "port_caps unreserve failed\n");
   fprintf (stderr, "calling stats module exit\n");

     module_stats_deinit(mod);

}

void testsuite_2()
{ /* Test Port create*/
  fprintf (stderr, "____________TEST PORT CREATE______________\n");
mct_module_t *mod;
     mct_port_t *port;
   mct_event_t event;
     char name[10] = "stats";
     fprintf (stderr, "calling stats module init\n");
     mod =module_stats_init(name);
     if(mod == NULL)
       fprintf (stderr, "stats module init failed\n");
     port = MCT_PORT_CAST(MCT_MODULE_SINKPORTS(mod)->data);
     mct_stream_info_t stream_info;
   stream_info.identity = 0xFFF10000;
   if(mod->request_new_port(&stream_info,MCT_PORT_SINK, mod) == NULL)
     fprintf (stderr, "stats module request new port failed\n");
   if(mod->request_new_port(&stream_info,MCT_PORT_SINK, mod) != NULL)
     fprintf (stderr, "stats module request new port should have failed as same identiy exist\n");
   stream_info.identity = 0xFFFF0000;
   if(mod->request_new_port(&stream_info,MCT_PORT_SINK, mod) == NULL)
     fprintf (stderr, "stats module request new port with different id failed\n");
     module_stats_deinit(mod);
}

void testsuite_3()
{ /* Test Caps reserve/unreserve*/
  fprintf (stderr, "____________TEST CAPS RESERVE/UNRESERVE______________\n");
  mct_module_t *mod;
     mct_port_t *port;
   mct_event_t event;
     char name[10] = "stats";
     fprintf (stderr, "calling stats module init\n");
     mod =module_stats_init(name);
     if(mod == NULL)
       fprintf (stderr, "stats module init failed\n");
     port = MCT_PORT_CAST(MCT_MODULE_SINKPORTS(mod)->data);
     mct_stream_info_t stream_info;
   mct_port_caps_t caps;
   /*MCT_PORT_CAPS_OPAQUE,
  MCT_PORT_CAPS_STATS,*/
   caps.port_caps_type =MCT_PORT_CAPS_STATS;
   caps.u.stats.flag = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS |\
                           MCT_PORT_CAP_STATS_HIST);
   stream_info.identity = 0xFFF10000;
   if (port->check_caps_reserve(port, &caps, &stream_info) == FALSE)
     fprintf (stderr, "port_caps reserve 1 failed\n");
   if (port->check_caps_reserve(port, &caps, &stream_info) == FALSE)
     fprintf (stderr, "port_caps reserve 2  failed\n");

/*  {
    // false id
  stream_info.identity = 0xFFFF0000;;
  if (port->check_caps_reserve(port, &caps, &stream_info) == TRUE)
     fprintf (stderr, "port_caps reserve 3 should have fail\n");

  if (port->check_caps_unreserve(port,stream_info.identity) != FALSE)
     fprintf (stderr, "port_caps unreserve 3 failed\n");
  }
*/
  stream_info.identity = 0xFFF10000;;
   if (port->check_caps_unreserve(port,stream_info.identity) == FALSE)
     fprintf (stderr, "port_caps unreserve 1 failed\n");
   if (port->check_caps_unreserve(port,stream_info.identity) == FALSE)
     fprintf (stderr, "port_caps unreserve good failed\n");

   if (port->check_caps_unreserve(port,stream_info.identity) == TRUE)
     fprintf (stderr, "port_caps unreserve should have failed\n");


     module_stats_deinit(mod);
}
void testsuite_4()
{ /* Test Caps reserve/unreserve*/
  fprintf (stderr, "____________TEST LINK/UNLINK______________\n");
  mct_module_t *mod;
     mct_port_t *port;
   mct_event_t event;
     char name[10] = "stats";
     fprintf (stderr, "calling stats module init\n");
     mod =module_stats_init(name);
     if(mod == NULL)
       fprintf (stderr, "stats module init failed\n");
     port = MCT_PORT_CAST(MCT_MODULE_SINKPORTS(mod)->data);
     mct_stream_info_t stream_info;
   mct_port_caps_t caps;
   /*MCT_PORT_CAPS_OPAQUE,
  MCT_PORT_CAPS_STATS,*/
   caps.port_caps_type =MCT_PORT_CAPS_STATS;
   caps.u.stats.flag = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS |\
                           MCT_PORT_CAP_STATS_HIST);
   stream_info.identity = 0XFFF10000;
   if (port->check_caps_reserve(port, &caps, &stream_info) == FALSE)
     fprintf (stderr, "port_caps reserve 1 failed\n");

   if (port->ext_link(stream_info.identity,port,port) == FALSE) {
     fprintf (stderr, "port ext link failed\n");
   }
   {/* Wrong identity*/
     stream_info.identity = 0XFFFF0000;
     if (port->ext_link(stream_info.identity,port,port) == TRUE) {
     fprintf (stderr, "port ext link should have failed\n");
     port->un_link(stream_info.identity, port, port);
   }
   }
   stream_info.identity = 0XFFF10000;
   if (port->ext_link(stream_info.identity,port,port) == FALSE) {
     fprintf (stderr, "port ext link failed\n");
   }
   port->un_link(stream_info.identity, port, port);
   port->un_link(stream_info.identity, port, port);

   if (port->check_caps_unreserve(port,stream_info.identity) == FALSE)
     fprintf (stderr, "port_caps ddd unreserve should failed\n");
     module_stats_deinit(mod);
}

void testsuite_5()
{ /* Basic bringup */
  fprintf (stderr, "____________TEST SEND EVT______________\n");
mct_module_t *mod;
     mct_port_t *port, *fake_port;
   mct_event_t event;
     char name[10] = "stats";
     mod =module_stats_init(name);
     if(mod == NULL)
       fprintf (stderr, "stats module init failed\n");
     port = MCT_PORT_CAST(MCT_MODULE_SINKPORTS(mod)->data);
       fprintf (stderr, "stats src port name %s parent module ptr %p module %p\n", MCT_PORT_NAME(port),
       MCT_PORT_PARENT(port), mod);
     mct_port_caps_t caps;
     mct_stream_info_t stream_info;
   /*MCT_PORT_CAPS_OPAQUE,
  MCT_PORT_CAPS_STATS,*/
   caps.port_caps_type =MCT_PORT_CAPS_STATS;
   caps.u.stats.flag = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS |\
                           MCT_PORT_CAP_STATS_HIST);
   stream_info.identity = 0XFFF10000;
   fake_port = mod->request_new_port(&stream_info,MCT_PORT_SINK, mod);
   if(fake_port == NULL)
     fprintf (stderr, "stats module request new port failed\n");
   if (fake_port->ext_link(stream_info.identity,fake_port,fake_port) == FALSE) {
     fprintf (stderr, "port ext link failed\n");
   }

   stream_info.identity = 0XFFFF0000;
   if(mod->request_new_port(&stream_info,MCT_PORT_SINK, mod) == NULL)
     fprintf (stderr, "stats module request new port failed\n");
   stream_info.identity = 0XFFF10000;

   if (port->ext_link(stream_info.identity,port,port) == FALSE) {
     fprintf (stderr, "port ext link failed\n");
   }
   event.identity = stream_info.identity;
   event.direction = MCT_EVENT_DOWNSTREAM;
   /*MCT_EVENT_CONTROL_CMD*/
   event.type = MCT_EVENT_MODULE_EVENT;
   event.u.module_event.type = MCT_EVENT_MODULE_STATS_DATA;

   if(port->event_func(port,&event) == FALSE)
     fprintf (stderr, "port_send event failed\n");
   event.identity = 0XFFFF0000;
   if(port->event_func(port,&event) == TRUE)
     fprintf (stderr, "port_send event should have failed\n");

   port->un_link(stream_info.identity, port, port);

  if (port->check_caps_unreserve(port,stream_info.identity) == FALSE)
     fprintf (stderr, "port_caps unreserve failed\n");
   fprintf (stderr, "calling stats module exit\n");

     module_stats_deinit(mod);

}

/*===========================================================================
 * Function: main
 *
 * Description: main encoder test app routine
 *
 * Input parameters:
 *   argc - argument count
 *   argv - argument strings
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
 int main (int argc, char **argv)
     {
     testsuite_1();
     testsuite_2();
   testsuite_3();

       /* test link*/
   testsuite_4();
       /* test send event*/
   testsuite_5();

     return 0;
     }
 #endif
