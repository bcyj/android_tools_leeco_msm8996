#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import binascii
from sectools.common.crypto import crypto_functions, utility_functions

from sectools.features.isc.encryption_service.ssd.encryption_parameters import EncryptionParameters
from sectools.features.isc.encryption_service.utility_functions import compute_segment_iv
from sectools.common.utils.c_logging import logger
from sectools.features.isc.encryption_service.base.encryptor import BaseEncryptor
import os

class Encryptor(BaseEncryptor):
    '''
    The Encryptor Object. Performs encryption based on unified image encryption methodology.
    '''

    def __init__(self, encryption_parameters, debug_dir=None):
        BaseEncryptor.__init__(self, encryption_parameters, debug_dir=debug_dir)


    def encrypt_segment(self, binary_segment, segment_num):
        '''
        Encrypt elf segments using cbc encryption
        input:
        binary_segment: A string representing the binary segment that needs to be encrypted.
        segment_num: The segment number, used to calculate the segment IV

        output:
        encrypted_binary_segment: CBC encrypted segment
        '''
        if len(binary_segment) < 16 and len(binary_segment)!=0:
            raise RuntimeError("The input plaintext is less than the minimum.")
        else:
#             image_iv=self.encryption_parameters.encryption_params_parser.MD_SIGN[0].IMG_ENC_INFO[0].IMG_ENC_IV
#             computed_segment_iv = compute_segment_iv(segment_num, image_iv)
#             pt_buf_len=len(binary_segment)
#             # how much data are we going to encrypt
#             data_to_enc_len = pt_buf_len - (pt_buf_len % 16)
#             data_to_enc=binary_segment[:data_to_enc_len]
#             encrypted_binary_segment, encryption_key, segment_iv = crypto_functions.cbc_encrypt_binary(data_to_enc, binascii.hexlify(self.encryption_parameters.l3_key), binascii.hexlify(computed_segment_iv))
#             encrypted_binary_segment+=binary_segment[data_to_enc_len:]
            pt_fn=utility_functions.store_data_to_temp_file(binary_segment)
            op_fn=utility_functions.store_data_to_temp_file("")
            self.encryption_parameters.ssd_p.enc_segment(segment_num, pt_fn, op_fn)
            encrypted_binary_segment=utility_functions.get_data_from_file(op_fn)
            os.unlink(pt_fn)
            os.unlink(op_fn)
            return encrypted_binary_segment
