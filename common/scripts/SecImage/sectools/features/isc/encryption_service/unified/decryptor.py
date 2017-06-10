#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from sectools.common.crypto import crypto_functions

from sectools.features.isc.encryption_service.unified.encryption_parameters import EncryptionParameters
from sectools.features.isc.encryption_service.utility_functions import compute_segment_iv
import binascii
from sectools.features.isc.encryption_service.base.decryptor import BaseDecryptor

class Decryptor(BaseDecryptor):
    '''
    Provides Unified Image Architecture specific decryption
    '''


    def __init__(self, encryption_parameters, debug_dir=None):
        BaseDecryptor.__init__(self, encryption_parameters, debug_dir=debug_dir)

    def decrypt_segment(self, encrypted_binary_segment, segment_num):
        '''
        '''
        computed_segment_iv = compute_segment_iv(segment_num, self.encryption_parameters.get_image_encryption_iv())
        if len(encrypted_binary_segment)%16 == 0:
            decrypted_segment = crypto_functions.cbc_decrypt_binary(encrypted_binary_segment, binascii.hexlify(self.encryption_parameters.get_image_encryption_key()), binascii.hexlify(computed_segment_iv))
        else:
            decrypted_segment = crypto_functions.cbc_cts_decrypt_binary(encrypted_binary_segment, binascii.hexlify(self.encryption_parameters.get_image_encryption_key()), binascii.hexlify(computed_segment_iv))
        return decrypted_segment

