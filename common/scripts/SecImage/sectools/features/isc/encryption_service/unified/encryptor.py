#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import binascii
from sectools.common.crypto import crypto_functions

from sectools.features.isc.encryption_service.unified.encryption_parameters import EncryptionParameters
from sectools.features.isc.encryption_service.utility_functions import compute_segment_iv
from sectools.common.utils.c_logging import logger
from sectools.features.isc.encryption_service.base.encryptor import BaseEncryptor

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
            computed_segment_iv = compute_segment_iv(segment_num, self.encryption_parameters.get_image_encryption_iv())
            if len(binary_segment)%16==0:
                encrypted_binary_segment, encryption_key, segment_iv = crypto_functions.cbc_encrypt_binary(binary_segment, binascii.hexlify(self.encryption_parameters.get_image_encryption_key()), binascii.hexlify(computed_segment_iv))
            else:
                encrypted_binary_segment, encryption_key, segment_iv = crypto_functions.cbc_cts_encrypt_binary(binary_segment, binascii.hexlify(self.encryption_parameters.get_image_encryption_key()), binascii.hexlify(computed_segment_iv))

            return encrypted_binary_segment
