#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Mar 31, 2014

@author: hraghav
'''

#------------------------------------------------------------------------------
# Magic numbers used in mbn format
#------------------------------------------------------------------------------
FLASH_CODE_WORD                     = 0x844BDCD1
MAGIC_NUM                           = 0x73D71034
SBL_VIRTUAL_BLOCK_MAGIC_NUM         = 0xD48B54C6
VIRTUAL_BLOCK_SIZE                  = 0x20000

#------------------------------------------------------------------------------
# File names for debugging
#------------------------------------------------------------------------------
FILE_DATA_IN                        = 'mbn_in_0_data'

FILE_MAGIC_REMOVED                  = 'mbn_in_1_magic_removed'

FILE_PRMBL_REMOVED                  = 'mbn_in_2_prmbl_removed'
FILE_PRMBL_IN                       = 'mbn_in_2_prmbl'
FILE_PRMBL_OUT                      = 'mbn_out_2_prmbl'

FILE_HDR_REMOVED                    = 'mbn_in_3_hdr_removed'
FILE_HDR_IN                         = 'mbn_in_3_hdr'
FILE_HDR_IN_REPR                    = 'mbn_in_3_hdr_repr'
FILE_HDR_IN_REPR_SUFFIX             = '.txt'

FILE_ENCRYPTION_PARAMS_REMOVED      = 'mbn_in_4_encryption_params_removed'
FILE_ENCRYPTION_PARAMS_IN           = 'mbn_in_4_encryption_params'

FILE_CERT_CHAIN_REMOVED             = 'mbn_in_5_cert_chain_removed'
FILE_CERT_CHAIN_IN                  = 'mbn_in_5_cert_chain'

FILE_SIGN_REMOVED                   = 'mbn_in_6_sign_removed'
FILE_SIGN_IN                        = 'mbn_in_6_sign'

FILE_CODE_IN                        = 'mbn_in_7_code'
