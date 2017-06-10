/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

    B L U E T O O T H   B T S   N V M   P L A T F O R M   S P E C I F I C   C O D E 

GENERAL DESCRIPTION
  This file implements Bluetooth Platform specific initialization code

Copyright (c) 2009-2010 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: 
  $DateTime: 
  $Author: 

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2010-04-08  jmf  Add stdlib function std_scanul for SOCCFG new arch.
  2009-06-24  jmf  Minimal Linux port enough to work, not clean up.
  2009-05-07   sa  Support for EFS NVM Mode.
  2008-02-06   sa  Cleanup & Added Support for FTM_BT feature for WM targets
  2008-09-16   sk  Initial version.


===============================================================================*/

/* Linux Specific Header File includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "btqsocnvmplatform.h"
#include "btqsocnvm.h"
#include "bthci_qcomm_pfal.h"
#include "bthci_qcomm_linux.h"

/*=========================================================================*/
/*                               TYPEDEFS                                  */
/*=========================================================================*/


/*===========================================================================
                                 Globals
===========================================================================*/
int hFile;           /* File Handle to nvm file */
#define FILENAME_LENGTH_MAX 255 /* Maximum file length */ /* get from unistd? */

/*==========================================================================

  FUNCTION       std_scanul

  DESCRIPTION    strtoul with error return
		 Local version of STDLIB (brew) routine

  DEPENDENCIES   None.

  PARAMETERS     text: string with digits to evaluate
                 radix: base of number to convert
                 end: place in string text after converted digits
                 error: pointer to where to put error code

  RETURN VALUE   unsigned long: zero or undefined if error != 0

  SIDE EFFECTS   Clears, possibly sets errno.  Sets *end past valid number.

==========================================================================*/

unsigned long std_scanul(const char *text, int radix,
                            const char **end, int *error)
{
	unsigned long retval;

	errno = 0;
	while (isspace(*text))
		*end = ++text;

	switch (radix) {  /* check for valix numeric character */
	case 10:
		if (!isdigit(*text) && (*text != '-') && (*text != '+')) {
			*error = STD_NODIGITS;
			return 0;
		}
		break;
	case 16:
		if (!isxdigit(*text) && (*text != '-') && (*text != '+')) {
			*error = STD_NODIGITS;
			return 0;
		}
		break;
	default:	/* parser doesn't use these */
		if ((radix < 1) || (radix > 36)) {
			*error = STD_BADPARAM;
			return 0;
		}
		if ((*text == '+') || (*text == '-'))
			break;
		if ((radix < 10) && !isdigit(*text) && ((*text >= ('0' + radix)))) {
			*error = STD_NODIGITS;
			return 0;
		}
		if (isdigit(*text))
			break;
		if (isalpha(*text) && (*text >= 'a') && (*text < 'a' + radix - 10))
			break;
		if (isalpha(*text) && (*text >= 'A') && (*text < 'A' + radix - 10))
			break;

		*error = STD_NODIGITS;
		return 0;
		break;
	}

	retval = strtoul(text, (char **)end, radix);
	/* should map errno to STD_* errors, but callers only check for non-zero */
	*error = errno;
	return retval;
}

/*==========================================================================

  FUNCTION       btqsocnvmplatform_find_file

  DESCRIPTION    This queries whether the file for the given SoC is present
                 on the File System

  DEPENDENCIES   None.

  PARAMETERS     qsoc_type: SOC version
                 clock_speed: Clock speed type
                 filename (out): returns the filename

  RETURN VALUE   boolean: True: If file is present.

  SIDE EFFECTS   None.

==========================================================================*/
boolean btqsocnvmplatform_find_file
(
char * filename
)
{
  boolean return_status = FALSE;

  if (firmware_file_name != NULL)
  {
     strlcpy(filename, firmware_file_name, FILENAME_LENGTH_MAX);
     return_status = TRUE;
  }
  return return_status;
}

/*==========================================================================

  FUNCTION       btqsocnvmplatform_open_file

  DESCRIPTION    This opens the released nvm file for the given SoC.

  DEPENDENCIES   None.

  PARAMETERS     qsoc_type: SOC version
                 clock_speed: Clock speed type

  RETURN VALUE   boolean: True: If initialized successfully else false.

  SIDE EFFECTS   Sets global hFile.

==========================================================================*/
boolean btqsocnvmplatform_open_file () {
  char filename[FILENAME_LENGTH_MAX];
  int i=0;

  for (i=0; i<FILENAME_LENGTH_MAX; ++i)
  {
    filename[i] = 0;
  }

  if ( !btqsocnvmplatform_find_file(filename) )
  {
    return FALSE;
  }

  hFile = open(filename, O_RDONLY);

  if ( hFile > -1 )
  {
    DEBUGMSG(ZONE_ERROR, "NVM File found");
    return TRUE;
  }
  else
  {
    /* nvm File not found */
    hFile = 0;
    DEBUGMSG(ZONE_ERROR, "NVM File not found");
    return FALSE;
  }
}

/*==============================================================
 * FUNCTION:  btqsocnvmplatform_get_file_size
 * ==============================================================*/
boolean btqsocnvmplatform_get_file_size
(
  /** pointer to integer for returning file size */
  int *file_size_ptr
)
{
  boolean r_val = FALSE;
  struct stat file_info;

  if ((hFile > -1) && (fstat(hFile, &file_info) == 0))
  {
      *file_size_ptr = (int)file_info.st_size;
      r_val = TRUE;
  }

  return r_val;
} /* btqsocnvmplatform_get_file_size */

/*==========================================================================

  FUNCTION       btqsocnvmplatform_read_file

  DESCRIPTION    This reads specific number of bytes from the file.

  DEPENDENCIES   open_bt_soc_nvm_file must be called successfully.

  PARAMETERS     *buf: pointer to buffer
                 buf_size: number of bytes to read

  RETURN VALUE   uint16: Number of bytes read.

  SIDE EFFECTS   None.

==========================================================================*/
uint16 btqsocnvmplatform_read_file
(
void *buf,
uint16 buf_size
)
{
  int bytes_read;

  bytes_read = read(hFile, (void *) buf, buf_size);

  if(bytes_read > 0)
  {
    return (uint16) bytes_read;
  }
  else
  {
    DEBUGMSG(ZONE_ERROR, "NVM File read error");
  }
  return 0;
}

/*==========================================================================

  FUNCTION       btqsocnvmplatform_close_file

  DESCRIPTION    This closes the file handle.

  DEPENDENCIES   None.

  PARAMETERS     None.

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/
void btqsocnvmplatform_close_file()
{
  if ( hFile > 0 )
  {
    if(close(hFile) != 0)
    {
      DEBUGMSG(ZONE_ERROR, "NVM File Error in Closing ");
    }
  }
}

/*==========================================================================

  FUNCTION       btqsocnvmplatform_log_error

  DESCRIPTION    Logs error.

  DEPENDENCIES   None.

  PARAMETERS     btqsocnvm_parser_err_type

  RETURN VALUE   None.

  SIDE EFFECTS   None.

==========================================================================*/

void btqsocnvmplatform_log_error(btqsocnvm_parser_err_type parser_err)
{
  DEBUGMSG(ZONE_ERROR, "NVM File Error %d", parser_err);
}

/*==============================================================
FUNCTION:  btqsocnvmplatform_malloc
==============================================================*/
void *btqsocnvmplatform_malloc
(
  /** [in] number of bytes to allocate */
  int num_bytes
)
{
  return malloc(num_bytes);
} /* btqsocnvmplatform_malloc */

/*==============================================================
FUNCTION:  btqsocnvmplatform_free
==============================================================*/
void btqsocnvmplatform_free
(
  /** [in] pointer to memory to be deallocated */
  void *mem_ptr
)
{
  free(mem_ptr);
} /* btqsocnvmplatform_free */
