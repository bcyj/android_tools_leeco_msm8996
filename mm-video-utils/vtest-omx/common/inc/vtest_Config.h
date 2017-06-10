/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential

Copyright (c) 2010 The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#ifndef _VTEST_CONFIG_H
#define _VTEST_CONFIG_H

#include "OMX_Core.h"
#include "vtest_ComDef.h"


namespace vtest {

class File;

/**
 * @brief Class for configuring video encoder and video encoder test cases
 */
class Config {

public:
    /**
     * @brief Constructor
     */
    Config();

    /**
     * @brief Destructor
     */
    ~Config();

    /**
     * @brief Parses the config file obtaining the configuration
     *
     * @param pFileName The name of the config file
     * @param pEncoderConfig The encoder config.
     * @param pDynamicConfig The dynamic encoder config. Optional (NULL is valid).
     */
    OMX_ERRORTYPE Parse(OMX_STRING pFileName,
            CodecConfigType *pEncoderConfig, DynamicConfigType *pDynamicConfig);

    /**
     * @brief Gets the current or default encoder configuration.
     *
     * @param pEncoderConfig The configuration
     */
    OMX_ERRORTYPE GetCodecConfig(CodecConfigType *pEncoderConfig);

    /**
     * @brief Gets the current or default dynamic encoder configuration.
     *
     * @param pDynamicConfig The configuration
     */
    OMX_ERRORTYPE GetDynamicConfig(DynamicConfigType *pDynamicConfig);

private:
    CodecConfigType m_sCodecConfig;
    DynamicConfigType m_sDynamicConfig;
    DynamicConfig *m_pDynamicProperties;
};

} // namespace vtest

#endif // #ifndef _VTEST_CONFIG_H
