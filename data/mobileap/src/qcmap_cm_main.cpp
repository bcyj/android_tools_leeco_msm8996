/*=========================================================================*/
/*!
  @file
  qcmap_cm_main.cpp

  @brief
  basic QCMAP CM Manager Main

  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
*/
/*=========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/29/13   sb      QCMAP boot up optimizations for RNDIS.
01/18/12   mp      Reducing default DSS init time macro to 3 seconds.
10/03/12   mp      Fix to prevent QCMAP starting call before QTI Link UP.
08/31/12   mp      Added support for Extended Firewalls in AP+STA.
08/03/12   mp      Added support for Port Restricted Cone NAT.
07/16/12   sc      Added tethering mode linux handler
07/10/12   mp      Removed port validation while adding ICMP SNAT entry.
06/14/12   rk      Fixed compilation warnings and Embedded call issues.
                   Also added port validation check.
05/18/12   vb      Added support for embedded call bringup.
05/11/12   mp      Adding option in the CLI menu to access the
                   Public NAT IP
05/04/12   cp      Added static IP configuration for STA mode.
04/16/12   sb      IPV6 state machine fix
03/09/12   sb      Adding extended firewall support
03/02/12   ss      Adding support for Concurrent STA+AP mode.
03/01/12   SDT     Adding support for Dual AP mode.
02/20/12   SDT     Adding support for IPv6.

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include "comdef.h"
#include "dssocket.h"
#include "ps_iface_defs.h"
#include "ds_Utils_DebugMsg.h"
#include "QCMAP_ConnectionManager.h"
#include "limits.h"
#include "qcmap_cmdq.h"
#include "dsi_netctrl.h"

#define QCMAP_PORT_MAX_VALUE 65535
static QCMAP_ConnectionManager *QcMapMgr = NULL;

/*===========================================================================
  FUNCTION  qcmap_check_port_value
===========================================================================
 @brief
   Port value is validated against the range 1 - QCMAP_PORT_MAX_VALUE
 @input
   sport - port value
 @return
   0  - success
   -1 - failure
 @dependencies
   None
 @sideefects
   None
=========================================================================*/
static int16
qcmap_cm_check_port (uint32 sport)
{
  if((sport > QCMAP_PORT_MAX_VALUE) || (sport < 1) )
  {
    printf(" port value should be between 1 - %d \n",QCMAP_PORT_MAX_VALUE);
    return -1;
  }
  else
    return 0;
}
void sighandler(int signal)
{
  switch (signal)
  {
    case SIGTERM:
      if (QcMapMgr)
      {
        
        QcMapMgr->Disable();
        while(1)
        {
          sleep(5);
          if(!(QcMapMgr->qcmap_tear_down_in_progress))
          {
            printf("Teardown complete\n");
            break;
          }
        }
      }
      break;

    case SIGUSR1:
        {
         if(QcMapMgr->TetheringOnly())
         {
           QcMapMgr->SetQtiState(TRUE);
           if(QcMapMgr->GetAutoconnect())
           {
             printf("Bring up WWAN call in tethering mode\n");
             if (QcMapMgr->ConnectBackHaul())
             {
               printf("ConnectBackHaul succeeds\n");
             }
             else
             {
               printf("ConnectBackHaul fails\n");
             }
           }
         }
         else
           printf("Tethering is not enabled or autoconnect is disabled\n");
         break;
        }        

    case SIGUSR2:
        {
          if(QcMapMgr->TetheringOnly())
          {
            printf("Tear down WWAN call in thethering mode\n");
            if (QcMapMgr->DisconnectBackHaul())
            {
              printf("DisconnectBackHaul succeeds\n");
            }
            else
            {
              printf("DisconnectBackHaul fails\n");
            }
            QcMapMgr->SetQtiState(FALSE);
          }
          else
            printf("Thethering is not enabled\n");
          break;
        }
    default:
      printf("Received unexpected signal %s\n", signal);
      break;
  }
}

int main(int argc, char **argv)
{
  int test_mode = 0;
  uint32 tmp_input=0;
  switch(argc)
  {
    /* If command line parameters were entered, ...*/
    case 2:
      /* ... read the XML file path from argv[1]. */
      printf("%s XML path %s\n", argv[0], argv[1]);
      QcMapMgr = new QCMAP_ConnectionManager(argv[1]);
      break;

    case 3:
      /* ... read the XML file path from argv[1]. */
      printf("%s XML path %s\n", argv[0], argv[1]);
      QcMapMgr = new QCMAP_ConnectionManager(argv[1]);
      /* ... read the enter test mode value from argv[2]. */
      test_mode = atoi(argv[2]);
      break;

    case 4:
      /* ... see if in deamon mode or normal mode */
      printf("%s XML path %s\n", argv[0], argv[1]);
      QcMapMgr = new QCMAP_ConnectionManager(argv[1]);
      /* ... read the enter test mode value from argv[2]. */
      test_mode = atoi(argv[2]);
      printf(" In new main function ");
      if (0 == strncasecmp(argv[3],"d",1))
      {
        /*------------------------------------------------------------------ 
         If not in dongle only/tethering only mode when started as daemon,
         QCMAP will exit. This is done because we need the QCMAP CLI in
         other modes.
        --------------------------------------------------------------------*/
        if(!QcMapMgr->TetheringOnly())
        {
          exit(1);
        }
        printf(" Correct Tethering mode!!! ");
      }
      break;

    default:
      /* Else, use default paramters to configure the Mobile AP. */
      QcMapMgr = new QCMAP_ConnectionManager(NULL);
      break;
  }

  if(QcMapMgr->IoEMode())
  {
    printf("QCMAP_ConnectionManager exiting because IoE Mode is SET\n");
    LOG_MSG_INFO1("QCMAP_ConnectionManager exiting because IoE Mode is SET",0,0,0);
    return 0;
  }
  else
  {
    system("killall -9 MCM_MOBILEAP_ConnectionManager");
  }

   /* initilize DSI library  */
  /* initilize DSI library	*/
  if (DSI_SUCCESS != dsi_init(DSI_MODE_GENERAL))
  {
    LOG_MSG_ERROR("dsi_init failed try Again!!",0,0,0);
  }

  sleep(2); /* To allow DSI Initialization. */

   /* Initialize qcmap_cmdq module */
  qcmap_cmdq_init();

  /* Register the sighandlers, so the app may be shutdown with a
     kill command.*/
  signal(SIGTERM, sighandler);
  signal(SIGUSR1, sighandler);
  signal(SIGUSR2, sighandler);

  /* Enable the Mobile AP LAN. If successful, ...*/
  QcMapMgr->Enable();

  /* add tethering mode start*/
   if(QcMapMgr->TetheringOnly()) 
  {
    while (TRUE)
    {
      sleep(2);
    }
  }

  /* If the test mode has been set, then the user would like to perform additional commands. */
  if (test_mode > 0)
  {
    int opt = 0;
    char scan_string[QCMAP_CM_MAX_FILE_LEN];

    while (TRUE)
    {
      /* Display menu of options. */
      printf("Please select an option to test from the items listed below.\n\n");
      printf(" 1. Display Current Config    17. Enable/Disable Firewall\n");
      printf(" 2. Delete SNAT Entry         18. Add Extended Firewall Entry\n");
      printf(" 3. Add SNAT Entry            19. Display Extended Firewalls\n");
      printf(" 4. Delete Firewall Entry     20. Delete Extended Firewall Entry\n");
      printf(" 5. Add Firewall Entry        21. Get WWAN Statistics \n");
      printf(" 6. Delete DMZ IP             22. Reset WWAN Statistics\n");
      printf(" 7. Add DMZ IP                23. Get IPv4 WWAN Configuration\n");
      printf(" 8. Set NAT Timeout           24. Reserved\n");
      printf(" 9. Set IPSEC VPN Passthrough 25. Reserved\n");
      printf("10. Set PPTP VPN Passthrough  26. Display Current NAT Type\n");
      printf("11. Set L2TP VPN Passthrough  27. Change NAT Type\n");
      printf("12. Set Autoconnect Config    28. Enable/Disable Mobile AP\n");
      printf("13. Set Roaming Config        29. Enable/Disable Station Mode\n");
      printf("14. Set DHCPD Config          30. Enable/Disable WLAN\n");
      printf("15. Set HostAPD Config        31. Connect/Disconnect Backhaul\n");
      printf("16. Set DualAP Config         32. Teardown/Disable and Exit\n");
      printf("Option > ");
      /* Read the option from the standard input. */
      if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        continue;

      /* Convert the option to an integer, and switch on the option entered. */
      opt = atoi(scan_string);
      switch (opt)
      {
        /* Display the current configuration of the modem. */
        case 1:
          QcMapMgr->DisplayModemConfig();
          QcMapMgr->DisplayLinuxConfig();
          break;

        /* Delete a static NAT entry and save XML if successful. */
        case 2:
        {
          qcmap_cm_port_fwding_entry_conf_t snat_entry;
          in_addr addr;
          memset(&snat_entry, 0, sizeof(qcmap_cm_port_fwding_entry_conf_t));
          while (TRUE)
          {
            printf("   Please input port_fwding_private_ip   : ");
            if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
            {
              if (inet_aton(scan_string, &addr))
                break;
            }
            printf("      Invalid IPv4 address %s", scan_string);
          }
          snat_entry.port_fwding_private_ip = ntohl(addr.s_addr);

          printf("   Please input port_fwding_protocol     : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          snat_entry.port_fwding_protocol = (uint8)atoi(scan_string);

          while (TRUE)
          {
            printf("   Please input port_fwding_private_port : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(snat_entry.port_fwding_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          snat_entry.port_fwding_private_port = (uint16)tmp_input;
          while (TRUE)
          {
            printf("   Please input port_fwding_global_port  : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(snat_entry.port_fwding_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          snat_entry.port_fwding_global_port = (uint16)tmp_input;

          if (QcMapMgr->DeleteStaticNatEntry(&snat_entry))
            printf("Static NAT config deleted.\n");
          else
            printf("Static NAT config not deleted.\n");
          break;
        }

        /* Add a static NAT entry and save XML if successful. */
        case 3:
        {
          qcmap_cm_port_fwding_entry_conf_t snat_entry;
          in_addr addr;
          memset(&snat_entry, 0, sizeof(qcmap_cm_port_fwding_entry_conf_t));
          while (TRUE)
          {
            printf("   Please input port_fwding_private_ip   : ");
            if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
            {
              if (inet_aton(scan_string, &addr))
                break;
            }
            printf("      Invalid IPv4 address %d\n", scan_string);
          }
          snat_entry.port_fwding_private_ip = ntohl(addr.s_addr);

          printf("   Please input port_fwding_protocol     : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          snat_entry.port_fwding_protocol = (uint8)atoi(scan_string);

          while (TRUE)
          {
            printf("   Please input port_fwding_private_port : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(snat_entry.port_fwding_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          snat_entry.port_fwding_private_port = (uint16)tmp_input;
          while (TRUE)
          {
            printf("   Please input port_fwding_global_port  : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(snat_entry.port_fwding_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          snat_entry.port_fwding_global_port = (uint16)tmp_input;

          if (QcMapMgr->AddStaticNatEntry(&snat_entry))
            printf("Static NAT config added.\n");
          else
            printf("Static NAT config not added.\n");
             break;
        }

        /* Delete a firewall entry and save XML if successful. */
        case 4:
        {
          qcmap_cm_firewall_entry_conf_t firewall_entry;
          memset(&firewall_entry, 0, sizeof(qcmap_cm_firewall_entry_conf_t));

          printf("   Please input firewall_protocol        : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          firewall_entry.firewall_protocol = (uint8)atoi(scan_string);

          while (TRUE)
          {
            printf("   Please input firewall_start_dest_port : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(firewall_entry.firewall_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          firewall_entry.firewall_start_dest_port = (uint16)tmp_input;
          while (TRUE)
          {
            printf("   Please input firewall_end_dest_port   : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(firewall_entry.firewall_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          firewall_entry.firewall_end_dest_port = (uint16)tmp_input;

          if (QcMapMgr->DeleteFireWallEntry(&firewall_entry))
            printf("Firewall config deleted.\n");
          else
            printf("Firewall config not deleted.\n");
          break;
        }

        /* Add a firewall entry and save XML if successful. */
        case 5:
        {
          qcmap_cm_firewall_entry_conf_t firewall_entry;
          memset(&firewall_entry, 0, sizeof(qcmap_cm_firewall_entry_conf_t));

          printf("   Please input firewall_protocol        : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          firewall_entry.firewall_protocol = (uint8)atoi(scan_string);

          while (TRUE)
          {
            printf("   Please input firewall_start_dest_port : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(firewall_entry.firewall_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          firewall_entry.firewall_start_dest_port = (uint16)tmp_input;
          while (TRUE)
          {
            printf("   Please input firewall_end_dest_port   : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            tmp_input = atoi(scan_string);
            if(firewall_entry.firewall_protocol == 1)
            /*No port validation check for ICMP*/
               break;
            if(qcmap_cm_check_port (tmp_input) == 0 )
               break;
          }
          firewall_entry.firewall_end_dest_port = (uint16)tmp_input;

          if (QcMapMgr->AddFireWallEntry(&firewall_entry))
            printf("Firewall config added.\n");
          else
            printf("Firewall config not added.\n");
          break;
        }

        /* Delete the current DMZ IP and save XML if successful. */
        case 6:
        {
          uint32 addr = 0;
          if (QcMapMgr->DeleteDMZ(addr))
            printf("DMZ IP deleted.\n");
          else
            printf("DMZ IP not deleted.\n");
          break;
        }

        /* Add a DMZ IP and save XML if successful. */
        case 7:
        {
          in_addr addr;

          while (TRUE)
          {
            printf("   Please input DMZ IP to add : ");
            if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
            {
              if (inet_aton(scan_string, &addr))
                break;
            }
            printf("      Invalid IPv4 address %d\n", scan_string);
          }

          if (QcMapMgr->AddDMZ(ntohl(addr.s_addr)))
            printf("DMZ IP added.\n");
          else
            printf("DMZ IP not added.\n");
          break;
        }

        /* Set the dynamic NAT entry timeout and save XML if successful. */
        case 8:
        {
          printf("   Please input dynamic NAT entry timeout : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (QcMapMgr->SetNATEntryTimeout((uint16)atoi(scan_string)))
            printf("Dynamic NAT entry timeout set.\n");
          else
            printf("Dynamic NAT entry timeout not set.\n");
          break;
        }

        /* Enable/disbale the IPSEC VPN pass through and save XML if successful. */
        case 9:
        {
          printf("   Please input IPSEC VPN Pass Through : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (QcMapMgr->SetIPSECVpnPassThrough((atoi(scan_string)) ? true : false))
            printf("IPSEC VPN Pass Through set.\n");
          else
            printf("IPSEC VPN Pass Through not set.\n");
          break;
        }

        /* Enable/disbale the PPTP VPN pass through and save XML if successful. */
        case 10:
        {
          printf("   Please input PPTP VPN Pass Through : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (QcMapMgr->SetPPTPVpnPassThrough((atoi(scan_string)) ? true : false))
            printf("PPTP VPN Pass Through set.\n");
          else
            printf("PPTP VPN Pass Through not set.\n");
          break;
        }

        /* Enable/disbale the L2TP VPN pass through and save XML if successful. */
        case 11:
        {
          printf("   Please input L2TP VPN Pass Through : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (QcMapMgr->SetL2TPVpnPassThrough((atoi(scan_string)) ? true : false))
            printf("L2TP VPN Pass Through set.\n");
          else
            printf("L2TP VPN Pass Through not set.\n");
          break;
        }

        case 12:
        {
          boolean enable;
          printf("   Please input Autoconnect config : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          enable = (atoi(scan_string)) ? true : false;
          if (QcMapMgr->SetAutoconnect(enable))
          {
            printf("Auto Connect config succeeds.\n");
          }
          else
          {
            printf("Auto Connect config fails.\n");
          }
          break;
        }
        case 13:
        {
          boolean enable;
          printf("   Please input Roaming config : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          enable = (atoi(scan_string)) ? true : false;
          if (QcMapMgr->SetRoaming(enable))
          {
            printf("Roaming config succeeds.\n");
          }
          else
          {
            printf("Roaming config fails.\n");
          }
          break;
        }
        case 14:
        {
          in_addr start, end;
          uint32 leasetime;
          int intf = 0;
          long val = 0;
          char *endptr;
          printf("   Please input Interface number : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          intf = atoi(scan_string);
          while (TRUE)
          {
            printf("   Please input starting DHCPD address : ");
            if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
            {
              if (inet_aton(scan_string, &start))
                break;
            }
            printf("      Invalid IPv4 address %d\n", scan_string);
          }
          while (TRUE)
          {
            printf("   Please input ending DHCPD address : ");
            if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
            {
              if (inet_aton(scan_string, &end))
                break;
            }
            printf("      Invalid IPv4 address %d\n", scan_string);
          }
          while (TRUE)
          {
          printf("   Please input DHCP lease time : ");
          fgets(scan_string, sizeof(scan_string), stdin);
            val = strtol(scan_string, &endptr, 10);
            if (val > 0)
            {
              if ((*endptr == '\0') || (*endptr == '\n') || (*endptr == '\r'))
              {
                printf("Lease time will be %ld seconds\n", val);
                *endptr = '\0';
                break;
              }
              else if ((*endptr == 'h') || (*endptr == 'H'))
              {
                printf("Lease time will be %ld hours\n", val);
                *(endptr + 1) = '\0';
                break;
              }
              else if ((*endptr == 'm') || (*endptr == 'M'))
              {
                printf("Lease time will be %ld minutes\n", val);
                *(endptr + 1) = '\0';
                break;
              }
            }
            else
            {
              scan_string[sizeof("infinite") - 1] = '\0';
              if (!strncmp(scan_string, "infinite", sizeof(scan_string)))
              {
                printf("Lease time will be infinite\n");
                break;
              }
            }
            printf("Lease time format is a positive decimal number followed by\n");
            printf("h for hours, m for minutes, or nothing for seconds.\n");
            printf("Or, can be infinite for an infinite lease time.\n");
          }
          printf("Lease time: %s\n", scan_string);
          if (QcMapMgr->SetDHCPDConfig(intf, ntohl(start.s_addr), ntohl(end.s_addr), scan_string))
          {
            printf("Set DHCPD config succeeds.\n");
          }
          else
          {
            printf("Set DHCPD config fails.\n");
          }
          break;
        }
        case 15:
        {
          int intf = 0, i = 0;
          printf("   Please input Interface number : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          intf = atoi(scan_string);
          printf("   Please input HostAPD cfg file : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          while (i++ < QCMAP_CM_MAX_FILE_LEN)
          {
            if ((scan_string[i] == '\r') || (scan_string[i] == '\n') ||
                (scan_string[i] == '\0'))
            {
              scan_string[i] = '\0';
              break;
            }
          }
          if (QcMapMgr->SetHostAPDConfig(intf, scan_string))
          {
            printf("Set HostAPD config succeeds.\n");
          }
          else
          {
            printf("Set HostAPD config fails.\n");
          }
          break;
        }
        case 16:
        {
          in_addr a5_ip_addr, sub_net_mask;
          printf("   Please input WLAN State : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (atoi(scan_string))
          {
            while (TRUE)
            {
              printf("   Please input interface IP address : ");
              if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
              {
                if (inet_aton(scan_string, &a5_ip_addr))
                  break;
              }
              printf("      Invalid IPv4 address %d\n", scan_string);
            }
            while (TRUE)
            {
              printf("   Please input interface IP subnet  : ");
              if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
              {
                if (inet_aton(scan_string, &sub_net_mask))
          break;
              }
              printf("      Invalid IPv4 address %d\n", scan_string);
            }
            if (QcMapMgr->SetDualAPConfig(true, ntohl(a5_ip_addr.s_addr), ntohl(sub_net_mask.s_addr)))
            {
              printf("Set DualAP config succeeds.\n");
            }
            else
            {
              printf("Set DualAP config fails.\n");
            }
          }
          else
          {
            if (QcMapMgr->SetDualAPConfig(false, 0, 0))
            {
              printf("Set DualAP config succeeds.\n");
          }
          else
          {
              printf("Set DualAP config fails.\n");
            }

          }
          break;
        }
        case 17:
        {
          boolean enable_firewall, pkts_allowed = false;
          printf("   Please input Firewall State          : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (enable_firewall = atoi(scan_string))
          {
            printf("   Please input Packets Allowed Setting : ");
            fgets(scan_string, sizeof(scan_string), stdin);
            pkts_allowed = atoi(scan_string);
          }
          if (QcMapMgr->SetFirewall(enable_firewall, pkts_allowed))
            printf("Firewall State set.\n");
          else
            printf("Firewall State not set.\n");
          break;
        }
        /* Extended firewall add*/
        case 18 :
          qcmap_cm_extd_firewall_conf_t     extd_firewall_add;
          in_addr                           ip4_src_addr;
          in_addr                           ip4_dst_addr;
          in_addr                           ip4_src_subnet_mask;
          in_addr                           ip4_dst_subnet_mask;
          struct in6_addr                   ip6_src_addr;
          struct in6_addr                   ip6_dst_addr;
          int                               input_len;
          int                               inc;
          uint32                            result; 
          int                               ip4_res;
          int                               ip6_res;
          int                               ip4_result;
          int                               ip6_result;
          uint8                             next_hdr_prot;
          char                              scan_string[32];
          char                              ip4_input[32];
          char                              ip6_input[48];
          char                              next_hdr_input[32];

          memset(&extd_firewall_add, 0, sizeof(qcmap_cm_extd_firewall_conf_t));

          printf("\n Please input IP family type : \t"
                 "Enter 4 for IPV4 and 6 for IPV6:");
          fgets(scan_string, sizeof(scan_string), stdin);
          result = atoi(scan_string);
          if(result == IP_V4)
          {
            extd_firewall_add.extd_firewall_entry.filter_spec.ip_vsn = IP_V4;

            printf("\n Do you want to enter IPV4 source address \t "
                   "and subnet mask: 1 for YES, 0 for NO:\n");
            fgets(ip4_input, sizeof(ip4_input), stdin);
            ip4_res = atoi(ip4_input);
            if(ip4_res==1)
            {
              while (TRUE)
              {
                printf("\n Please input IPV4 address: ");
                if (fgets(ip4_input, sizeof(ip4_input), stdin) != NULL)
                {
                  if (inet_aton(ip4_input, &ip4_src_addr))
                  {
                    extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr =
                                                    ntohl(ip4_src_addr.s_addr);
                    extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
                                                     IPFLTR_MASK_IP4_SRC_ADDR;
                    break;
                  }
                }

                printf("Invalid IPv4 address \n");
              }

              while (TRUE)
              {
                printf("\n Please input IPV4 subnet mask: ");
                if (fgets(ip4_input, sizeof(ip4_input), stdin) != NULL)
                {
                  if (inet_aton(ip4_input, &ip4_src_subnet_mask))
                  {
                    extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr =
                                                  ntohl(ip4_src_subnet_mask.s_addr);
                    break;
                  }
                }

                printf("\n Invalid IPv4 subnet mask \n");
              }
            }

            printf("\n Do you want to enter IPV4 destination address and \t"
                   "subnet mask: enter 1 for Yes and 0 for No\n");
            fgets(ip4_input, sizeof(ip4_input), stdin);
            ip4_res = atoi(ip4_input);
            if(ip4_res == 1)
            {
              while (TRUE)
              {
                printf("\n Please input IPV4 address: ");
                if (fgets(ip4_input, sizeof(ip4_input), stdin) != NULL)
                {
                  if (inet_aton(ip4_input, &ip4_dst_addr))
                  {
                    extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.dst.addr.ps_s_addr =
                                                  ntohl(ip4_dst_addr.s_addr);
                    extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
                                             IPFLTR_MASK_IP4_DST_ADDR;
                    break;
                  }

                }
              printf("Invalid IPv4 address\n");
              }

              while (TRUE)
              {
                printf("\n Please input IPV4 subnet mask: ");
                if (fgets(ip4_input, sizeof(ip4_input), stdin) != NULL)
                {
                  if (inet_aton(ip4_input, &ip4_dst_subnet_mask))
                  {
                    extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr =
                                                  ntohl(ip4_dst_subnet_mask.s_addr);
                    break;
                  }
                }
                printf("\n Invalid IPv4 subnet mask \n");
              }
            }

            printf("\n Please input IPV4 type of service value: ");
            fgets(ip4_input, sizeof(ip4_input), stdin);
            if ( ip4_input[0]!='\n')
            {
              extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val =
                                                    (uint8)atoi(ip4_input);
              extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
                                                      IPFLTR_MASK_IP4_TOS ;
            }

            printf("\n Please input IPV4 type of service mask:");
            fgets(ip4_input, sizeof(ip4_input), stdin);
            if(ip4_input[0] !='\n')
            {
              extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask =
                                                   (uint8)atoi(ip4_input);
            }
            while (TRUE)
              {
                printf("\n Please input IPV4 next header protocol : \t"
                        "TCP=6, UDP=17,ICMP=1,ESP=50, TCP_UDP=253: ");
                fgets(ip4_input, sizeof(ip4_input), stdin);
                if ( ip4_input[0]!= '\n')
                {
                  ip4_result = atoi(ip4_input);
                  if(ip4_result == 6 || ip4_result == 17 || ip4_result == 1 ||
                     ip4_result ==50 || ip4_result ==253)
                  {
                    extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot =
                                                     (uint8)atoi(ip4_input);
                    next_hdr_prot =
                         extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;
                    break;
                  }
                }
              }
          }
          else if(result == IP_V6)
          {

            extd_firewall_add.extd_firewall_entry.filter_spec.ip_vsn = IP_V6;

            printf("\n Do you want to enter IPV6 source address and prefix:\t"
                   " 1 for yes and 0 for no\n");
            fgets(ip6_input, sizeof(ip6_input), stdin);
            ip6_res = atoi(ip6_input);
            if(ip6_res==1)
            {
              while (TRUE)
              {
                printf("\n Please input IPV6 address: ");
                if (fgets(ip6_input, sizeof(ip6_input), stdin) != NULL)
                {
                    input_len = strlen(ip6_input);
                    ip6_input[input_len-1] = '\0';
                    ip6_result =inet_pton(AF_INET6,ip6_input, &ip6_src_addr);
                    if (ip6_result)
                    {
                      memcpy(extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
                              ip6_src_addr.s6_addr, QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));
                      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
                                                      IPFLTR_MASK_IP6_SRC_ADDR;
                      break;
                    }
                 }
                 printf("Invalid IPv6 address \n");
              }

              while (TRUE)
              {
                printf("\n Please input IPV6 prefix length: ");
                fgets(ip6_input, sizeof(ip6_input), stdin);
                if ( ip6_input[0] != '\n' )
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len =
                                                        (uint8)atoi(ip6_input);
                  break;
                }

              }
            }

            printf("\n Do you want to enter IPV6 destination address \t"
                   "and subnet mask: 1 for Yes and 0 for No\n");
            fgets(ip6_input, sizeof(ip6_input), stdin);
            ip6_res = atoi(ip6_input);
            if(ip6_res==1)
            {
              while (TRUE)
              {
                printf("\n Please input IPV6 address: ");
                if (fgets(ip6_input, sizeof(ip6_input), stdin) != NULL)
                {
                    input_len = strlen(ip6_input);
                    ip6_input[input_len-1] = '\0';
                    ip6_result = inet_pton(AF_INET6,ip6_input, &ip6_dst_addr);
                  if (ip6_result)
                  {
                     memcpy(extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
                            ip6_dst_addr.s6_addr, QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));

                     extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
                                                    IPFLTR_MASK_IP6_DST_ADDR;
                     break;
                  }
                }
                printf("Invalid IPv6 address \n");
              }

              while (TRUE)
              {
                printf("\n Please input IPV6 prefix length: ");
                fgets(ip6_input, sizeof(ip6_input), stdin);
                if ( ip6_input[0] != '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.dst.prefix_len =
                                                     (uint8)atoi(ip6_input);
                  break;
                }

              }
            }

            printf("\n Please input IPV6 traffic class value: ");
            fgets(ip6_input, sizeof(ip6_input), stdin);
            if ( ip6_input[0]!='\n')
            {
              extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val =
                                                    (uint8)atoi(ip6_input);
              extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
                                             IPFLTR_MASK_IP6_TRAFFIC_CLASS;
            }

            printf("\n Please input IPV6 traffic class mask:");
            fgets(ip6_input, sizeof(ip6_input), stdin);
            if( ip6_input[0] != '\n')
            {
              extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask =
                                            (uint8)atoi(ip6_input);
            }

            while (TRUE)
            {
              printf("\n Please input IPV6 next header protocol: \t"
                     "TCP=6, UDP=17,ICMP6=58,ESP=50, TCP_UDP=253 :");
              fgets(ip6_input, sizeof(ip6_input), stdin);
              if ( ip6_input[0]!= '\n')
              {
                ip6_result = atoi(ip6_input);
                if(ip6_result == 6 || ip6_result == 17 || ip6_result == 58 ||
                   ip6_result ==50 || ip6_result ==253)
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot =
                                                             (uint8)ip6_result;
                  next_hdr_prot =
                        extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot;
                  break;
                }
              }

            }

          }
          else
          {
            printf("\n Unsupported IP protocol ");
            break;
          }

          if(next_hdr_prot == PS_IPPROTO_TCP)
          {
             while (TRUE)
             {
               printf("\n Please enter source/destination port and range. \n");
               printf("\n Please input TCP source port: ");
               fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
               tmp_input = atoi(next_hdr_input);
               if(qcmap_cm_check_port (tmp_input) == 0 )
                  break;
             }
             if ( next_hdr_input[0] != '\n')
             {
                extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port =
                                                (uint16)tmp_input;
                extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |=
                                                     IPFLTR_MASK_TCP_SRC_PORT;
             }

             printf("\n Please input TCP source port range: ");
             fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
             if ( next_hdr_input [0]!= '\n')
             {
                extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range =
                                                  (uint16)atoi(next_hdr_input);
             }

             while (TRUE)
             {
               printf("\n Please input TCP destination port: ");
               fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
               tmp_input = atoi(next_hdr_input);
               if(qcmap_cm_check_port (tmp_input) == 0 )
                  break;
             }
             if ( next_hdr_input[0]!= '\n')
             {
               extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port =
                                                    (uint16)tmp_input;
               extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |=
                                                        IPFLTR_MASK_TCP_DST_PORT;
             }

             printf("\n Please input TCP destination port range: ");
             fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
             if ( next_hdr_input[0] != '\n')
             {
               extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range =
                                                     (uint16)atoi(next_hdr_input);
             }
            }
            else if(next_hdr_prot == PS_IPPROTO_UDP)
            {
                while (TRUE)
                {
                  printf("\n Please enter source/destination port and range \n");
                  printf("\n Please input UDP source port: ");
                  fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                  tmp_input = atoi(next_hdr_input);
                  if(qcmap_cm_check_port (tmp_input) == 0 )
                     break;
                }
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port =
                                                       (uint16)tmp_input;
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |=
                                                          IPFLTR_MASK_UDP_SRC_PORT;
                }

                printf("\n Please input UDP source port range: ");
                fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range =
                                                         (uint16)atoi(next_hdr_input);
                }

                while (TRUE)
                {
                  printf("\n Please input UDP destination port: ");
                  fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                  tmp_input = atoi(next_hdr_input);
                  if(qcmap_cm_check_port (tmp_input) == 0 )
                     break;
                }
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port =
                                                        (uint16)tmp_input;
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |=
                                                              IPFLTR_MASK_UDP_DST_PORT;
                }

                printf("\n Please input UDP destination port range: ");
                fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range =
                                                           (uint16)atoi(next_hdr_input);
                }
            }
            else if (next_hdr_prot == PS_IPPROTO_TCP_UDP)
            {
                while (TRUE)
                {
                  printf("\n Please enter source/destination port and range \n");
                  printf("\n Please input TCP_UDP source port: ");
                  fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                  tmp_input = atoi(next_hdr_input);
                  if(qcmap_cm_check_port (tmp_input) == 0 )
                     break;
                }
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port =
                                                            (uint16)tmp_input;
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
                                                              IPFLTR_MASK_TCP_UDP_SRC_PORT;
                }

                printf("\n Please input TCP_UDP source port range: ");
                fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range =
                                                             (uint16)atoi(next_hdr_input);
                }

                while (TRUE)
                {
                  printf("\n Please input TCP_UDP destination port: ");
                  fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                  tmp_input = atoi(next_hdr_input);
                  if(qcmap_cm_check_port (tmp_input) == 0 )
                     break;
                }
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port =
                                                             (uint16)tmp_input;
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
                                                               IPFLTR_MASK_TCP_UDP_DST_PORT;
                }

                printf("\n Please input TCP_UDP destination port range: ");
                fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range =
                                                             (uint16)atoi(next_hdr_input);
                }
            }
            else if (next_hdr_prot == PS_IPPROTO_ICMP || next_hdr_prot == PS_IPPROTO_ICMP6)
            {
                printf("\n Please input ICMP type: ");
                fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type = 
                                                 (uint8)atoi(next_hdr_input);
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask |=
                                                    IPFLTR_MASK_ICMP_MSG_TYPE;
                }

                printf("\n Please input ICMP code: ");
                fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code = 
                                             (uint8)atoi(next_hdr_input);
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask |=
                                                   IPFLTR_MASK_ICMP_MSG_CODE;
                }

            }
            else if (next_hdr_prot == PS_IPPROTO_ESP)
            {

                printf("\n Please input ESP SPI: ");
                fgets(next_hdr_input, sizeof(next_hdr_input), stdin);
                if ( next_hdr_input[0]!= '\n')
                {
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi =
                                                 atoi(next_hdr_input);
                  extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.esp.field_mask |=
                                                         IPFLTR_MASK_ESP_SPI;
                }
            }
            else
            {
                printf("Unsupported protocol");
                break;
            }

          if (QcMapMgr->AddExtdFireWallEntry(&extd_firewall_add))
            printf("Firewall config added.\n");
          else
            printf("Firewall config not added.\n");
          break;

        case 19:
          qcmap_cm_extd_firewall_conf_t  extd_firewall_handle_list;
          qcmap_cm_extd_firewall_conf_t  extd_firewall_get;
          int                          handle_list_len;
          int                          index;
          char                         ip6_addr[48];
          in_addr                      ip4_src_subnet;
          in_addr                      ip4_dst_subnet;

          memset(&extd_firewall_handle_list, 0, sizeof(qcmap_cm_extd_firewall_conf_t));
          
          handle_list_len=0;

          printf("\n Please input IP family type : \t"
                 "Enter 4 for IPV4 and 6 for IPV6:");
          fgets(scan_string, sizeof(scan_string), stdin);
          result = atoi(scan_string);
          if(result == IP_V4)
          {
            extd_firewall_handle_list.extd_firewall_handle_list.ip_family = IP_V4;
          }
          else if(result == IP_V6)
          {
            extd_firewall_handle_list.extd_firewall_handle_list.ip_family = IP_V6;
          }

          if(QcMapMgr->GetFireWallHandleList(&extd_firewall_handle_list))
          {
            handle_list_len = extd_firewall_handle_list.extd_firewall_handle_list.num_of_entries;
          }

          if(handle_list_len > 0)
          {

            for(index =0; index < handle_list_len; index++)
            {
              printf("\n Index %d", index);
              printf("\n Firewall Handle ID %d \n",
                     extd_firewall_handle_list.extd_firewall_handle_list.handle_list[index]);

              memset(&extd_firewall_get, 0, sizeof(qcmap_cm_extd_firewall_conf_t));

              extd_firewall_get.extd_firewall_entry.filter_spec.ip_vsn =
                  extd_firewall_handle_list.extd_firewall_handle_list.ip_family;
              extd_firewall_get.extd_firewall_entry.firewall_handle =
                  extd_firewall_handle_list.extd_firewall_handle_list.handle_list[index];

              if(QcMapMgr->GetExtdFireWallEntry(&extd_firewall_get))
              {

                switch(extd_firewall_get.extd_firewall_entry.filter_spec.ip_vsn)
                {
                  case IP_V4:

                    if(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask &
                                                                IPFLTR_MASK_IP4_SRC_ADDR)
                    {
                      ip4_src_addr.s_addr =
                      htonl(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
                      ip4_src_subnet.s_addr =
                      htonl(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);

                      printf("\n IPV4 source address: %s", inet_ntoa(ip4_src_addr));
                      printf("\n IPV4 source subnet mask: %s", inet_ntoa(ip4_src_subnet));
                    }

                    if(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask &
                                                                IPFLTR_MASK_IP4_DST_ADDR)
                    {
                      ip4_dst_addr.s_addr =
                      htonl(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.dst.addr.ps_s_addr);
                      ip4_dst_subnet.s_addr =
                      htonl(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);

                      printf("\n IPV4 destination address: %s", inet_ntoa(ip4_dst_addr));
                      printf("\n IPV4 destination subnet mask: %s", inet_ntoa(ip4_dst_subnet));
                    }

                    if(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask &
                                                                     IPFLTR_MASK_IP4_TOS)
                    {
                      printf("\n IPV4 TOS value: %d",
                      extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val);
                      printf("\n IPV4 TOS mask: %d",
                      extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask);
                    }


                      next_hdr_prot = extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;

                   break;

                 case IP_V6:
                   if(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask &
                                                             IPFLTR_MASK_IP6_SRC_ADDR)
                   {
                     memcpy(ip6_src_addr.s6_addr,
                           extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
                           QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));

                     printf("\n IPV6 source address: %s",
                     inet_ntop(AF_INET6,&ip6_src_addr,ip6_addr,sizeof(ip6_addr)));

                     printf("\n IPV6 source prefix length: %d",
                            extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len);
                  }

                  if(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask &
                                                             IPFLTR_MASK_IP6_DST_ADDR)
                  {
                    memcpy(ip6_dst_addr.s6_addr,
                           extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
                           QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));

                    printf("\n IPV6 destination address: %s",
                           inet_ntop(AF_INET6,&ip6_dst_addr,ip6_addr,sizeof(ip6_addr)));

                    printf("\n IPV6 destination prefix length: %d",
                           extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.dst.prefix_len);
                  }

                  if(extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask &
                                                        IPFLTR_MASK_IP6_TRAFFIC_CLASS)
                  {
                    printf("\n IPV6 Traffic class value: %d",
                           extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val);
                    printf("\n IPV6 Traffic class mask: %d",
                           extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask);
                  }

                    next_hdr_prot = extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot;

                  break;

               default:
                 printf("Invalid IP family type");
                 return FALSE;
            }


            printf("\n Next header protocol: %d\n", next_hdr_prot);

            switch(next_hdr_prot)
            {
               case PS_IPPROTO_TCP:
                 if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask &
                                                                IPFLTR_MASK_TCP_SRC_PORT)
                 {
                   printf("\n TCP source port : %d \n",
                          extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port);
                   printf("\n TCP source port range: %d \n",
                          extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range);
                 }

                 if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask &
                                                                IPFLTR_MASK_TCP_DST_PORT)
                 {
                   printf("\n TCP destination port : %d \n",
                          extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port);
                   printf("\n TCP destination port range: %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range);
                 }
                 break;

               case PS_IPPROTO_UDP:
                 if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask &
                                                                   IPFLTR_MASK_UDP_SRC_PORT)
                 {
                   printf("\n UDP source port : %d \n",
                          extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port);
                   printf("\n UDP source port range: %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range);
                 }

                 if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask &
                                                                   IPFLTR_MASK_UDP_DST_PORT)
                 {
                    printf("\n UDP destination port : %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port);
                    printf("\n UDP destination port range: %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range);
                 }
                 break;

               case PS_IPPROTO_ICMP:
               case PS_IPPROTO_ICMP6:
                 if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask &
                                                                  IPFLTR_MASK_ICMP_MSG_CODE)
                 {
                    printf("\n ICMP code : %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code);
                 }

                 if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask &
                                                                 IPFLTR_MASK_ICMP_MSG_TYPE)
                 {
                   printf("\n ICMP type : %d \n",
                          extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type);
                 }
                 break;

                case PS_IPPROTO_ESP:
                  if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.esp.field_mask &
                                                                      IPFLTR_MASK_ESP_SPI)
                  {
                    printf("\n ESP SPI : %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi);
                  }
                  break;

                case PS_IPPROTO_TCP_UDP:
                  if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
                                                                           IPFLTR_MASK_TCP_UDP_SRC_PORT)
                  {
                    printf("\n TCP_UDP source port : %d\n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port);
                    printf("\n TCP_UDP source port range: %d\n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range);
                  }

                  if(extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
                                                                           IPFLTR_MASK_TCP_UDP_DST_PORT)
                  {
                    printf("\n UDP destiantion port : %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port);
                    printf("\n UDP destination port range: %d \n",
                           extd_firewall_get.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range);
                  }
                  break;


               default:
                  printf("Unsupported next header protocol\n");
                  return FALSE;
              }

             }
             else
             {
               printf("Get firewall rule failed\n");
             }
           }
         }
         else
         {
           printf("Get firewall handles failed\n");
         }

         break;

        case 20:
        {
          qcmap_cm_extd_firewall_conf_t  extd_firewall_handle;
          char *handle_string = NULL;

          memset(&extd_firewall_handle, 0,  sizeof(qcmap_cm_extd_firewall_conf_t));

          printf("\n Please input IP family type : \t"
                 "Enter 4 for IPV4 and 6 for IPV6:");
          fgets(scan_string, sizeof(scan_string), stdin);
          result = atoi(scan_string);
          if(result == IP_V4)
          {
            extd_firewall_handle.extd_firewall_handle.ip_family = IP_V4;
          }
          else if(result == IP_V6)
          {
            extd_firewall_handle.extd_firewall_handle.ip_family = IP_V6;
          }

          printf("\n Please input firewall handle(obtained from get extd firewall rules):");
          fgets(scan_string, sizeof(scan_string), stdin);
          result = strtol(scan_string, &handle_string, 10);

          if(result == LONG_MAX || result == LONG_MIN)
          {
            printf("\n Invalid handle entered \n");
            break;
          }
          extd_firewall_handle.extd_firewall_handle.handle = result;
          if(QcMapMgr->DeleteExtdFireWallEntry(&extd_firewall_handle))
          {
            printf("\n Delete firewall succeeded \n");
          }
          else
          {
            printf("\n Delete firewall failed \n");
          }
          break;
        }

        /* Get WWAN Statistics */
        case 21:
        {
          qcmap_cm_statistics_t   wwan_stats;
          int                     error_num;
          ip_version_enum_type  ip_family;

          printf("Please input IP Family IPV4-4 IPV6-6 : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          ip_family = (ip_version_enum_type)atoi(scan_string);
          if (QcMapMgr->GetWWANStatistics(ip_family, &wwan_stats, &error_num))
          {
            printf("WWAN Stats for IP Family: %d \n", ip_family);
            printf("Bytes Rx: %llu\n", wwan_stats.bytes_rx);
            printf("Bytes Tx: %llu\n", wwan_stats.bytes_tx);
            printf("Pkts Rx: %d\n", wwan_stats.pkts_rx);
            printf("Pkts Tx: %d\n", wwan_stats.pkts_tx);
            printf("Pkts Dropped Rx: %d\n", wwan_stats.pkts_dropped_rx);
            printf("Pkts Dropped Tx: %d\n", wwan_stats.pkts_dropped_tx);
          }
          else
          {
            printf("\nGetWWANStatistics Returns Error number %d", error_num);
          }
          break;
        }

        /* RESET WWAN Statistics */
        case 22:
        {
          int                     error_num;
          ip_version_enum_type  ip_family;

          printf("Please input IP Family IPV4-4 IPV6-6 : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          ip_family = (ip_version_enum_type)atoi(scan_string);
          if (QcMapMgr->ResetWWANStatistics(ip_family, &error_num))
          {
            printf("WWAN Stats Reset for IP Family: %d \n", ip_family);
          }
          else
          {
            printf("\nResetWWANStatistics Returns Error number %d", error_num);
          }
          break;
        }
        case 23:
        {
          in_addr public_ip;
          in_addr primary_dns;
          in_addr secondary_dns;
          memset(&public_ip,0,sizeof(in_addr));
          memset(&primary_dns,0,sizeof(in_addr));
          memset(&secondary_dns,0,sizeof(in_addr));

          if(QcMapMgr->GetIPv4WWANNetworkConfiguration
                                                      (
                                                       (uint32 *)&public_ip.s_addr,
                                                       (uint32 *)&primary_dns.s_addr,
                                                       (uint32 *)&secondary_dns.s_addr
                                                      ))
          {
            public_ip.s_addr = htonl(public_ip.s_addr);
            primary_dns.s_addr = htonl(primary_dns.s_addr);
            secondary_dns.s_addr = htonl(secondary_dns.s_addr);
            printf("Public IP for WWAN: %s \n", inet_ntoa(public_ip));
            printf("Primary DNS IP address: %s \n", inet_ntoa(primary_dns));
            printf("Secondary DNS IP address: %s \n", inet_ntoa(secondary_dns));
          }
          else
          {
            printf("\nGetIPv4WWANNetworkConfiguration Returns Error");
          }
          break;
        }

        case 26:
        {
          qcmap_cm_nat_type cur_nat_type;
          if(QcMapMgr->GetNatType(&cur_nat_type))
            (int)cur_nat_type ? printf("PORT RESTRICTED CONE NAT\n"):\
                                printf("SYMMETRIC NAT\n");
          else
            printf("Cannot get NAT Type\n");
          break;
        }
        case 27:
        {
	  printf("Select the Type of NAT : \n"
  	         "(0) SYMMETRIC NAT / (1) PORT RESTRICTED CONE NAT\t");
	  fgets(scan_string, sizeof(scan_string), stdin);
	  if((atoi(scan_string)==0) || (atoi(scan_string)==1))
	  {
	    if (QcMapMgr->ChangeNatType((qcmap_cm_nat_type)atoi(scan_string)))
		  printf("Switching to the selected NAT type\n");
            else
		  printf("Changing of NATType fails\n");
          }
          else
          {
            printf("Invalid NAT Type option provided\n");
          }
	  break;
	}
        case 28:
        {
          printf("   Please input MobileAP State : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (atoi(scan_string))
          {
            if (QcMapMgr->Enable())
              printf("MobileAP Enable succeeds.\n");
            else
              printf("MobileAP Enable fails.\n");
          }
          else
          {
            if (QcMapMgr->Disable())
              printf("MobileAP Disable in progress.\n");
            else
              printf("MobileAP Disable request fails.\n");
          }
          break;
        }
        case 29:
        {
          printf("   Please input STA State : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (atoi(scan_string))
          {
            qcmap_sta_connection_config *cfg_ptr;
            printf("   Please input: 1 for New Config/else for Default Config: ");
            fgets(scan_string, sizeof(scan_string), stdin);
            if (atoi(scan_string) == 1)
            {
              in_addr static_ip_addr, netmask, gw_addr, dns_addr;
              qcmap_sta_connection_config cfg;
              printf("   Please input Connection Type, 0 for DYNAMIC/1 for STATIC:");
              fgets(scan_string, sizeof(scan_string), stdin);
              if (atoi(scan_string) == 1)
              {
                while (TRUE)
                {
                  printf("   Please input a valid Static IP address:");
                  if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
                  {
                    if (inet_aton(scan_string, &static_ip_addr))
                      break;
                  }
                  printf("      Invalid IPv4 address %s", scan_string);
                }
                while (TRUE)
                {
                  printf("   Please input a valid Gateway address:");
                  if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
                  {
                    if (inet_aton(scan_string, &gw_addr))
                      break;
                  }
                  printf("      Invalid IPv4 address %s", scan_string);
                }
                while (TRUE)
                {
                  printf("   Please input a valid Netmask:");
                  if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
                  {
                    if (inet_aton(scan_string, &netmask))
                      break;
                  }
                  printf("      Invalid IPv4 address %s", scan_string);
                }
                while (TRUE)
                {
                  printf("   Please input a valid DNS Address:");
                  if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
                  {
                    if (inet_aton(scan_string, &dns_addr))
                      break;
                  }
                  printf("      Invalid IPv4 address %s", scan_string);
                }
                cfg.static_ip_config.ip_addr = ntohl(static_ip_addr.s_addr);
                cfg.static_ip_config.gw_ip = ntohl(gw_addr.s_addr);
                cfg.static_ip_config.netmask = ntohl(netmask.s_addr);
                cfg.static_ip_config.dns_addr = ntohl(dns_addr.s_addr);
                cfg.conn_type = QCMAP_STA_CONNECTION_STATIC;
              }
              else
              {
                cfg.conn_type = QCMAP_STA_CONNECTION_DYNAMIC;
              }
              cfg_ptr = &cfg;
            }
            else
            {
              cfg_ptr = NULL;
            }
            if(QcMapMgr->EnableStaMode(cfg_ptr))
            {
              printf("Concurrent mode enabled.\n");
            }
            else
            {
              printf("Failed to enable concurrent mode.\n");
            }
          }
          else
          {
            if(QcMapMgr->DisableStaMode())
            {
              printf("Concurrent mode disabled.\n");
            }
            else
            {
              printf("Failed to disable concurrent mode.\n");
            }
          }
          break;
        }
        case 30:
        {
          printf("   Please input WLAN State : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (atoi(scan_string))
          {
            if(QcMapMgr->EnableWLAN())
            {
              printf("Enabled WLAN\n");
            }
            else
            {
              printf("Failed to Enable WLAN\n");
            }
          }
          else
          {
            if(QcMapMgr->DisableWLAN())
            {
              printf("Disabled WLAN\n");
            }
            else
            {
              printf("Failed to Disable WLAN\n");
            }
          }
          break;
        }
        case 31:
        {
          printf("   Please input Backhaul State : ");
          fgets(scan_string, sizeof(scan_string), stdin);
          if (atoi(scan_string))
          {
            if (QcMapMgr->ConnectBackHaul())
              printf("ConnectBackHaul succeeds.\n");
            else
              printf("ConnectBackHaul fails.\n");
          }
          else
          {
            if (QcMapMgr->DisconnectBackHaul())
              printf("DisconnectBackHaul succeeds.\n");
            else
              printf("DisconnectBackHaul fails.\n");
          }
          break;
        }
        /* Disconnect BackHaul, disable LAN and exit application. */
        case 32:
          sighandler(SIGTERM);
          exit(1);
          break;

        /* Invalid integer entered. */
        default :
          printf("Invalid response %d\n", opt);
          break;
      }
    }
  }
  /* Otherwise, leave the backhaul connected until the application receives a SIGTERM. */
  else
  {
    while(1)
    {
      sleep(2);
    }
  }

  return 0;
}
