#===========================================================================
# Copyright (C) 2011 Qualcomm Technologies, Inc. All rights reserved.
#                    Qualcomm Technologies Proprietary/GTDR
# All data and information contained in or disclosed by this document is
# confidential and proprietary information of Qualcomm Technologies, Inc. and all
# rights therein are expressly reserved.  By accepting this material the
# recipient agrees that this material and the information contained therein
# is held in confidence and in trust and will not be used, copied, reproduced
# in whole or in part, nor its contents revealed in any manner to others
# without the express written permission of Qualcomm Technologies, Inc.
#===========================================================================*/
===============
About kickstart
===============

kickstart is a command line based utility for uploading boot images to the targets using
DMSS download protocol or sahara protocol. This README explains how to compile and use the tool.

============================================
Instructions for compiling & using kickstart
============================================

==========
To compile
========== 
   1. "make" on the root directory of kickstart, it will result in an executable named kickstart
   3. To clean the generated object files and kickstart binary, do "make clean"     

==========      
To install
==========
   kickstart can be run from the directory where the application was compiled or by adding it to the 
   PATH variable by doing "export PATH=$PATH:$PWD". 

======   
To use
======
   kickstart comes with the following command line options.
   
   -h    "prints a help menu with various possible options supported by the tool"
           Usage:
            -h                     --help                      Display this usage information
            -v                     --verbose                   Print verbose messages
            -p                     --port                      Device name for USB driver
            -d <img_id:file_name>  --dload  <img_id:file_name> Use DMSS download protocol for loading the image
            -s <img_id:file_name>  --sahara <img_id:file_name> Use Sahara download protocol for transferring the image


	  Example usage:
            sudo kickstart -p /dev/ttyUSB0 -d 10:dbl.mbn
            sudo kickstart -p /dev/ttyUSB0 -s 11:osbl.mbn -s 2:amss.mbn
         
           The following are the possible image id's that can be given as input to the tool. DBL_IMG is the only image type that is supported 
           for DLOAD protocol, and for Sahara protocol OSBL_IMG and AMSS_IMG are the only images that were tested.
             
                NONE_IMG        = 0,
                OEM_SBL_IMG     = 1,
                AMSS_IMG        = 2,
                QCSBL_IMG       = 3,
                HASH_IMG        = 4,
                NANDPRG_IMG     = 5,
                CFG_DATA        = 6,
                NORPRG_IMG      = 7,
                HOSTDL_IMG      = 8,
                FSBL_IMG        = 9,
                DBL_IMG         = 10,
                OSBL_IMG        = 11,
                APPS_IMG        = 12,
                APPSBL_IMG      = 13,
                DSP1_IMG        = 14,
                DSP2_IMG        = 15,
                EHOSTDL_IMG     = 16,
                RAMFS1_IMG      = 17,
                RAMFS2_IMG      = 18,
                ADSP_Q5_IMG     = 19,
                APPS_KERNEL_IMG = 20
              

                
   -v      "Prints verbose messages, kickstart comes with four levels of logging such as 
            EVENT, ERROR, INFO and WARN"
            By default, EVENT, WARN and ERROR level messages are always turned on, if the user 
            wants to see more detailed information, enable this flag -v to print out INFO level messages"
                   
            sample INFO messges:
            INFO function:start_sahara_based_transfer line:60 "STATE <-- SAHARA_HELLO_WAIT"
            ....
            INFO function:start_sahara_based_transfer line:150 "STATE <-- SAHARA_WAIT_COMMAND"
            INFO function:start_sahara_based_transfer line:155 "Received End of Image Transfer"
            INFO function:is_ack_successful line:555 "SAHARA_ACK_SUCCESS"
            INFO function:start_sahara_based_transfer line:158 "STATE --> SENDING_SAHARA_DONE_COMMAND"
                    
   -p     "device name", ex: /dev/ttyUSB0
   
   -d     "Uploads the input image using DMSS download protocol" 
   
   -s     "Uploads the input image using Sahara protocol"
        
          
 
=== 
FAQ
===
  Q. What if the user sees the following error message when executing the application?
       ERROR function:connect_and_configure_device line:253 "Not Connected to the device: /dev/ttyUSB0"
       
  A: Check if the device is enumerated on the host side by doing a "lsusb":
          1. In DMSS download mode the device should enumerate as 
           ... ID 05c6:9008 Qualcomm, Inc
          2. If the target is ready to get the images using sahara protocol then 
             it should enumerate as
           ... ID 05c6:900e Qualcomm, Inc
          
          Assuming that the device is enumerated with the required details, check the kernel 
          logs by doing "dmesg" to see if the kernel had assigned a driver to the device and there is a node
          created on the /dev/ directory.
          If there is no node like /dev/ttyUSB0 seen on the device list, then check if the usbserial driver is already
          running or not by doing lsmod. If not seen on the module list, then attach the device to the driver by doing the 
          following:
           sudo /sbin/modprobe usbserial vendor=0x05C6 product=0x9008 for DMSS download mode
           sudo /sbin/modprobe usbserial vendor=0x05C6 product=0x900e for Sahara mode         
           
          Alternatively, udev rules can be created under /etc/udev/rules.d to automatically detect and assign a driver 
          based on the vendor id and product id

