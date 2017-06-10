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
from sectools.common.parsegen import mbn, PAD_BYTE_0


MBN_FILE_TYPE = 'mbn'


class SecParseGenMbn(SecParseGenBase):

    def __init__(self, data, imageinfo=None, mbn_properties=None,
                 general_properties=None,
                 encdec=None,
                 debug_dir=None,
                 debug_prefix=None,
                 debug_suffix=None,):
        SecParseGenBase.__init__(self, data, imageinfo, general_properties,
                                 encdec, debug_dir, debug_prefix, debug_suffix)

        # Check the arguments
        if imageinfo is not None:
            mbn_properties = imageinfo.image_type.mbn_properties
        if mbn_properties is None:
            raise RuntimeError('MBN properties must not be None.')

        # Initialize the mbn parsegen
        self._mbn_parsegen = mbn.ParseGenMbn(data,
                                             mbn_properties.header_size,
                                             mbn_properties.preamble_size,
                                             mbn_properties.has_magic_num,
                                             mbn_properties.page_size,
                                             mbn_properties.num_of_pages,
                                             mbn_properties.ota_enabled,
                                             mbn_properties.min_size_with_pad,
                                             self.debug_dir,
                                             self.debug_prefix,
                                             self.debug_suffix)

        # Set the security attributes from the mbn parser
        self.data_signature = self._mbn_parsegen.sign
        self.cert_chain = self._mbn_parsegen.cert_chain
        self.encryption_params = self._mbn_parsegen.encryption_params

    def __repr__(self):
        return ('Base Properties: ' + '\n' + SecParseGenBase.__repr__(self) + '\n'
                'MBN Properties: ' + '\n' + repr(self._mbn_parsegen))

    #--------------------------------------------------------------------------
    # Mandatory Properties overloaded from the base class
    #--------------------------------------------------------------------------
    @classmethod
    def file_type(cls):
        return MBN_FILE_TYPE

    def get_data(self, integrity_check=None, sign=None, encrypt=None):
        # Resolve the operation
        integrity_check = self.integrity_check if integrity_check is None else integrity_check
        sign = self.sign if sign is None else sign
        encrypt = self.encrypt if encrypt is None else encrypt

        # Allow base to do any checks
        SecParseGenBase.get_data(self, integrity_check, sign, encrypt)
        if integrity_check:
            raise RuntimeError('Mbn Images do not support integrity check.')
        if encrypt:
            raise RuntimeError('Mbn Images do not support encryption.')
        return self._get_data_int(sign, False)

    @property
    def data_to_sign(self):
        # Backup the parsegen
        backup = _BackupMbnParsegen(self._mbn_parsegen)

        # Put dummy sign and cert chain. This is so that the header of the
        # parsegen reflects the size and cert chain that will be injected later.
        self._mbn_parsegen.sign = PAD_BYTE_0 * self.sig_size
        self._mbn_parsegen.cert_chain = PAD_BYTE_0 * self.cert_chain_size

        # Get the data to sign (header + code)
        retval = self._mbn_parsegen.get_header().pack() + self._mbn_parsegen.code

        # Restore the parsegen
        backup.restore(self._mbn_parsegen)

        return retval

    def contains_integrity_check(self):
        return False

    #--------------------------------------------------------------------------
    # Helper methods
    #--------------------------------------------------------------------------
    def _get_data_int(self, sign, encrypt):
        # Backup the parsegen
        backup = _BackupMbnParsegen(self._mbn_parsegen)

        # Update the security attributes per the flags
        if sign:
            self._mbn_parsegen.sign = self.data_signature
            self._mbn_parsegen.cert_chain = self.cert_chain
        if encrypt:
            self._mbn_parsegen.encryption_params = self.encryption_params

        # Get the signed data
        retval = self._mbn_parsegen.get_data()

        # Restore the parsegen
        backup.restore(self._mbn_parsegen)

        return retval

# Register the parsegen with the factory
SecParseGenMbn.register()


class _BackupMbnParsegen(object):

    def __init__(self, parsegen):
        self.backup(parsegen)

    def backup(self, parsegen):
        # Backup the original
        self.signature = parsegen.sign
        self.cert_chain = parsegen.cert_chain
        self.encryption_params = parsegen.encryption_params

        # Clear the original
        parsegen.sign = ''
        parsegen.cert_chain = ''
        parsegen.encryption_params = ''

        return parsegen

    def restore(self, parsegen):
        # Restore the original
        parsegen.sign = self.signature
        parsegen.cert_chain = self.cert_chain
        parsegen.encryption_params = self.encryption_params

        # Clear self
        self.signature = ''
        self.cert_chain = ''
        self.encryption_params = ''

        return parsegen


