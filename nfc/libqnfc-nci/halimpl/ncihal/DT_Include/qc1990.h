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
 /*====================================================================================================*
 *                                DEFNITIONS                                                           *
 *=====================================================================================================*/

#define NFC_MAGIC 0xE9

#define NFC_SET_PWR             _IOW(NFC_MAGIC, 0x01, unsigned int)
#define NFCC_MODE               _IOW(NFC_MAGIC, 0x02, unsigned int)
#define SET_RX_BLOCK            _IOW(NFC_MAGIC, 0x03, unsigned int)
#define SET_EMULATOR_TEST_POINT _IOW(0xE9, 0x04, unsigned int)
#define NFCC_VERSION            _IOW(0xE9, 0x08, unsigned int)
#define NFC_GET_EFUSE           _IOW(0xE9, 0x09, unsigned int)
#define NFCC_INITIAL_CORE_RESET_NTF  _IOW(0xE9, 0x10, unsigned int)
