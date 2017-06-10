/******************************************************************************

Copyright (c) 2013, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

/**
 * \file  DT_Nfc_i2c.c
 * \brief DT I2C port implementation for linux
 *
 */

#include <cutils/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <errno.h>

#include <DT_Nfc_status.h>
#include <DT_Nfc_types.h>
#include <DT_Nfc_i2c.h>
#include <DT_Nfc_log.h>
#include <qc1990.h>
#include "nfc_hal_int.h"

#if defined(ANDROID)
#include <string.h>
#endif

#define NFC_MSG_THRESH  NFC_ALL_LOGGING

typedef struct
{
   long DeviceFileHandle;
   char TransportStarted;
} DT_Nfc_I2cInst;

/*-----------------------------------------------------------------------------------
                                      VARIABLES
------------------------------------------------------------------------------------*/
static DT_Nfc_I2cInst I2C_Inst;

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_initialize

PURPOSE:  Initialize internal variables

-----------------------------------------------------------------------------*/

void DT_Nfc_i2c_initialize(void)
{
   memset(&I2C_Inst, 0, sizeof(DT_Nfc_I2cInst));
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_is_opened

PURPOSE:  Returns if the link is opened or not. (0 = not opened; 1 = opened)

-----------------------------------------------------------------------------*/

int DT_Nfc_i2c_is_opened(void)
{
   return I2C_Inst.TransportStarted;
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_flush

PURPOSE:  Flushes the link ; clears the link buffers

-----------------------------------------------------------------------------*/

void DT_Nfc_i2c_flush(void)
{
   /* Nothing to do (driver has no internal buffers) */
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_close

PURPOSE:  Closes the link

-----------------------------------------------------------------------------*/

void DT_Nfc_i2c_close(void)
{
   HAL_TRACE_DEBUG0 ("I2C:DT_Nfc_i2c_close : Closing port");
   if (I2C_Inst.TransportStarted == 1)
   {
      close(I2C_Inst.DeviceFileHandle);
      I2C_Inst.DeviceFileHandle = 0;
      I2C_Inst.TransportStarted = 0;
   }
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_setup

PURPOSE:  Closes the link

-----------------------------------------------------------------------------*/

NFC_RETURN_CODE DT_Nfc_i2c_setup(pDT_Nfc_sConfig_t pConfig, int* pdTransportHandle)
{
   HAL_TRACE_DEBUG0 ("I2C:DT_Nfc_i2c_setup");
   pConfig->devFile = "/dev/nfc-nci";
   pConfig->phyType  = ENUM_LINK_TYPE_I2C;

   I2C_Inst.DeviceFileHandle = open(pConfig->devFile, O_RDWR | O_NOCTTY);
   if (I2C_Inst.DeviceFileHandle < 0)
   {
       HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_setup : Open failed: open() returned %d \n", I2C_Inst.DeviceFileHandle);
       *pdTransportHandle = NULL;
       return NFC_INVALID_NFCC;
   }

   I2C_Inst.TransportStarted = 1;
   *pdTransportHandle = I2C_Inst.DeviceFileHandle;

    HAL_TRACE_DEBUG2("I2C:DT_Nfc_i2c_setup : status = %d, handle = %d \n", NFC_SUCCESS, *pdTransportHandle);

   return NFC_SUCCESS;
}


/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_read

PURPOSE:  Reads NumToRd bytes and writes them in pStore.
          Returns the number of bytes really read or -1 in case of error.

-----------------------------------------------------------------------------*/

int DT_Nfc_i2c_read(uint8_t * pStore, int NumToRd)
{
    int ret;
    int numRead = 0;
    struct timeval tv;
    fd_set rfds;

    HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_read : read %d bytes \n", NumToRd);
    while (numRead < NumToRd) {
        FD_ZERO(&rfds);
        FD_SET(I2C_Inst.DeviceFileHandle, &rfds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        ret = select(I2C_Inst.DeviceFileHandle + 1, &rfds, NULL, NULL, &tv);
        if (ret < 0) {
            HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_read : select() errno = %d \n", errno);
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return -1;
        } else if (ret == 0) {
            HAL_TRACE_DEBUG0("I2C:DT_Nfc_i2c_read : Timeout!");
            return -1;
        }
        ret = read(I2C_Inst.DeviceFileHandle, pStore + numRead, NumToRd - numRead);

        if (ret > 0) {
            HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_read : read %d bytes", ret);
            numRead += ret;
        } else if (ret == 0) {
            HAL_TRACE_DEBUG0("I2C:DT_Nfc_i2c_read : EOF");
            return -1;
        } else {
            HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_read : errno=%d \n", errno);
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return -1;
        }
    }
    return numRead;
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_write

PURPOSE:  Writes nNbBytesToWrite bytes from pBuffer to the link
          Returns the number of bytes that have been wrote to the interface or -1 in case of error.

-----------------------------------------------------------------------------*/
int DT_Nfc_i2c_write(uint8_t * pStore, int NumToWr)
{
    int ret;
    int numWrote = 0;

    HAL_TRACE_DEBUG2("I2C:DT_Nfc_i2c_write value = %d, length = %d ", *pStore, NumToWr);

    while (numWrote < NumToWr) {
        ret = write(I2C_Inst.DeviceFileHandle, pStore + numWrote, NumToWr - numWrote);
        if (ret > 0) {
            HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_write : wrote %d bytes", ret);

            numWrote += ret;
        } else if (ret == 0) {
            HAL_TRACE_DEBUG0("I2C:DT_Nfc_i2c_write : EOF");
            return -1;
        } else {
            HAL_TRACE_DEBUG1("DT_Nfc_i2c_write : errno=%d \n", errno);
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return -1;
        }
    }
    HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_write : Wrote = %d bytes\n", numWrote);
    return numWrote;
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_reset

PURPOSE:  Control the (GPIO_RESET/VEN) pin, legacy Reset Pin
          and (SWP/FW_DL) Pin.

-----------------------------------------------------------------------------*/
int DT_Nfc_i2c_reset(long state)
{
    HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_reset : NFCC power = %d \n", state);
    return ioctl(I2C_Inst.DeviceFileHandle, NFC_SET_PWR, state);

}
/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_i2c_version

PURPOSE:  Control the (GPIO_RESET/VEN) pin, legacy Reset Pin
          and (SWP/FW_DL) Pin.

-----------------------------------------------------------------------------*/
int DT_Nfc_i2c_version(long field)
{
    HAL_TRACE_DEBUG1("I2C:DT_Nfc_i2c_version : field = %d \n", field);
    return ioctl(I2C_Inst.DeviceFileHandle, NFCC_VERSION, field);
}

/*-----------------------------------------------------------------------------
FUNCTION: DT_Nfc_i2c_efuse_type

PURPOSE:  Return the value of efuse register to figure out if device is
          fused or unfused device.
-----------------------------------------------------------------------------*/
int DT_Nfc_i2c_efuse_type()
{
    HAL_TRACE_DEBUG0("I2C:DT_Nfc_i2c_efuse_type\n");
    return ioctl(I2C_Inst.DeviceFileHandle, NFC_GET_EFUSE, 0);
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_set_controller_mode

PURPOSE:  Set the mode to communicate with the NFCC,
          Currently just selects read mode (solicited/unsolicited)

-----------------------------------------------------------------------------*/
int DT_Nfc_set_controller_mode(long mode)
{
    HAL_TRACE_DEBUG1("I2C:DT_Nfc_set_controller_mode, Vmode = %d \n", mode);
    return ioctl(I2C_Inst.DeviceFileHandle, NFCC_MODE, mode);
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_set_controller_mode

PURPOSE:  Set the mode to communicate with the NFCC

-----------------------------------------------------------------------------*/
int DT_Nfc_set_rx_block_number(long block_number)
{
    HAL_TRACE_DEBUG1("I2C:DT_Nfc_set_rx_block_number, block number = %d \n", block_number);
    return ioctl(I2C_Inst.DeviceFileHandle, SET_RX_BLOCK, block_number);
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_set_test_point_number

PURPOSE: Configure the emulation platform to output test points to header
         For instance, output polling trigger to check field.

-----------------------------------------------------------------------------*/
int DT_Nfc_set_test_point_number(int TestPointNumber)
{
    HAL_TRACE_DEBUG1("I2C:DT_Nfc_set_test_point_number, test point number = %d \n", TestPointNumber);
    return ioctl(I2C_Inst.DeviceFileHandle, SET_EMULATOR_TEST_POINT, TestPointNumber);
}

/*-----------------------------------------------------------------------------

FUNCTION: DT_Nfc_check_core_reset_ntf

PURPOSE: Configure the emulation platform to output test points to header
         For instance, output polling trigger to check field.

-----------------------------------------------------------------------------*/
int DT_Nfc_check_nfcc_initial_core_reset_ntf()
{
    HAL_TRACE_DEBUG0("I2C:DT_Nfc_check_nfcc_initial_core_reset_ntf");
    return ioctl(I2C_Inst.DeviceFileHandle, NFCC_INITIAL_CORE_RESET_NTF);
}
