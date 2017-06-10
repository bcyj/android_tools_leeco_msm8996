/******************************************************************************
  @file:  loc_api_xtra_bin.h
  @brief: loc_api_test XTRA binary data services header

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
03/17/09   dx       Android version
01/10/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_xtra_bin.h#3 $
======================================================================*/

#ifndef LOC_API_XTRA_BIN_H_
#define LOC_API_XTRA_BIN_H_

#define GPS_XTRA_READ_WRITE_BUFFER_SIZE       1024
#define GPS_XTRA_DATA_INFO_VALID              0x0001

#define GPS_XTRA_FILE_SERVER_PORT             80       
#define GPS_XTRA_FILE_SERVER_DIR              "/"       
#define GPS_XTRA_FILENAME                     "xtra.bin"

#define GPS_XTRA_INJECT_PART_SIZE             1024   /* Reduced for librpc (<< 2048) */

/**
 * Define where downloaded data is saved
 *   1: use tmpfile()    
 *   0: use GPS_XTRA_FILENAME
 */
#define LOC_TEST_XTRA_USE_TMPFILE                  0

struct in_addr;
extern boolean resolve_in_addr(char *host_addr, struct in_addr *in_addr_ptr);
extern FILE* loc_xtra_download_bin(int *filesize);
extern int loc_xtra_inject_bin(FILE* xtra_fp, int file_size);

#endif /* LOC_API_XTRA_BIN_H_ */
