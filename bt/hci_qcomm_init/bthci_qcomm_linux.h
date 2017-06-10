#ifndef _BTHCI_QCOMM_LINUX_H_
#define _BTHCI_QCOMM_LINUX_H_

#ifdef __cplusplus
extern "C"
{
#endif 

/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

 B L U E T O O T H    P F A L  B T S    D R I V E R    H E A D E R    F I L E

GENERAL DESCRIPTION
 Platform level abstraction  
EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS

-----------------------------------------------------------------------------
Copyright (c) 2007, 2011 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------------
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //linux/pkgs/proprietary/bt/main/source/hci_qcomm_init/bthci_qcomm_linux.h#1 $
  $DateTime: 2008/06/18 11:43:50 $
  $Author: anshulg $


  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2011-07-18  bn   Added support for 8960 for sending NVM tags to RIVA.
  2011-07-02  bhr  Adding -g and -o options for dumping the hci commands to a file.
  2008-03-13  jmf  Add new edit history format and P4 version control string.
  2007-09-06   jn  Adapted from AMSS/WM version.
===========================================================================*/

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

/*=========================================================================*/
/*                               CONSTANTS                                 */
/*=========================================================================*/

/*=========================================================================*/
/*                               TYPEDEFS                                  */
/*=========================================================================*/
/*===========================================================================
             Tables for NVM configuration during bootup
===========================================================================*/


/*=========================================================================*/
/*                                MACROS                                   */
/*=========================================================================*/

#undef BTHCI_QCOMM_TRACE
#define BTHCI_QCOMM_TRACE(astring) DEBUGMSG(1,astring)

#define MAX_FILE_NAME (255)


/*=========================================================================*/
/*                           DATA DECLARATIONS                             */
/*=========================================================================*/

extern bt_qsoc_enum_nvm_mode nvm_mode;
extern char *firmware_file_name;
extern boolean generate_override_mode;
extern char binary_filename[MAX_FILE_NAME];

/*=========================================================================*/
/*                           FUNCTION INTERFACES                           */
/*=========================================================================*/

extern boolean bt_hci_qcomm_get_event
(
  uint8 *evt_buf
);

extern int bt_hci_qcomm_nwrite
(
  uint8 *buf, 
  int   size
);

extern int bt_hci_qcomm_nread
(
  uint8 *buf, 
  int   size
);

extern boolean bt_hci_qcomm_set_ldisc
(
  void
);

extern int fd;


#ifdef __cplusplus
}
#endif 

#endif /* _BTHCI_QCOMM_LINUX_H_ */
