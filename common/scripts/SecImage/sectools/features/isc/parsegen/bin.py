#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
'''
Created on Nov 25, 2013

@author: hraghav
'''

from sectools.features.isc.parsegen.base import SecParseGenBase


# Bin file type
BIN_FILE_TYPE = 'bin'


class SecParseGenBin(SecParseGenBase):

    def __init__(self, data, imageinfo=None, bin_properties=None,
                 general_properties=None,
                 encdec=None,
                 debug_dir=None,
                 debug_prefix=None,
                 debug_suffix=None):
        SecParseGenBase.__init__(self, data, imageinfo, general_properties,
                                 encdec, debug_dir, debug_prefix, debug_suffix)

        """
        # Check the arguments
        if imageinfo is not None:
            bin_properties = imageinfo.image_type.bin_properties
        if bin_properties is None:
            raise RuntimeError('BIN properties must not be None.')
        """

        # Check if file is encrypted
        encryption_params = ''
        if self.encdec is not None:
            data, encryption_params = self.encdec.extract_encryption_parameters(data)
            if encryption_params:
                decryptor = self.encdec.get_decryptor(encryption_params, self.encdec.get_dummy_key())
                data = decryptor.decrypt_segment(data, 0)

        self.data, self.encryption_params = data, encryption_params

    #--------------------------------------------------------------------------
    # Mandatory Properties overloaded from the base class
    #--------------------------------------------------------------------------
    @classmethod
    def file_type(cls):
        return BIN_FILE_TYPE

    def get_wrapped_data(self):
        return self.data

    def set_wrapped_data(self, data):
        self.data = data

    def get_data(self, integrity_check=None, sign=None, encrypt=None):
        # Resolve the operation
        integrity_check = self.integrity_check if integrity_check is None else integrity_check
        sign = self.sign if sign is None else sign
        encrypt = self.encrypt if encrypt is None else encrypt

        if integrity_check:
            raise RuntimeError('Bin Images do not support integrity check.')
        if sign:
            raise RuntimeError('Bin Images do not support signing.')

        data = self.data
        if encrypt:
            data = self.encryption_params + self.encrypt_segment(data, 0)
        return data

    @property
    def data_to_sign(self):
        raise RuntimeError('Bin Images do not support signing')

    def contains_integrity_check(self):
        return False

# Register the parsegen with the factory
SecParseGenBin.register()
