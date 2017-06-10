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

import defines as df
from sectools.common.parsegen import hex_addr
from sectools.common.parsegen.mbn import magic, ota, preamble
from sectools.common.parsegen.mbn.header import extract_header, MBN_HDRS
from sectools.common.parsegen.mbn.preamble import PRMBL_ATTRS, PRMBL_PAGE_SIZES,\
    PRMBL_MAGIC_NUMS
from sectools.common.utils.c_misc import store_debug_data_to_file,\
    properties_repr


ALIGNED_IMAGE_SIZE      = lambda x : x + ((4 - (x % 4)) if x % 4 else 0)


class ParseGenMbn(object):

    def __init__(self, data, header_size,
                 preamble_size=None,
                 has_magic_num=None,
                 page_size=None,
                 num_pages=None,
                 ota_enabled=False,
                 ota_min_size=None,
                 debug_dir=None,
                 debug_prefix=None,
                 debug_suffix=None,
                 ):

        # Initialize internal properties
        self._header_size = 0
        self._preamble_size = None
        self._has_magic_num = False
        self._page_size = None
        self._num_pages = None
        self._ota_enabled = False
        self._ota_min_size = None

        self._sign = ''
        self._cert_chain = ''
        self._encryption_params = ''

        # Public properties
        self.debug_dir = debug_dir
        self.debug_prefix = debug_prefix
        self.debug_suffix = debug_suffix

        """
        Extract the various sections of the data into the following components:
        1. magic_number
        2. preamble
        3. header
        4. signature
        5. cert_chain
        6. bin
        """
        # Public attributes
        self.preamble = None
        self.header = None
        self.code = None

        # Set properties
        self.header_size = header_size
        self.preamble_size = preamble_size
        self.has_magic_num = has_magic_num
        self.page_size = page_size
        self.num_pages = num_pages
        self.ota_enabled = ota_enabled
        self.ota_min_size = ota_min_size

        # Store the original image
        self.store_debug_data(df.FILE_DATA_IN, data)

        # Extract the magic number
        if self.has_magic_num:
            data, cookies, magic_cookie = magic.remove(data)
            self.store_debug_data(df.FILE_MAGIC_REMOVED, data)
            magic.validate(cookies, magic_cookie)

        # Extract the preamble
        if self.has_preamble is True:
            # Extract the preamble from the data
            data, prmbl = preamble.remove(data, self.preamble_size)
            self.store_debug_data(df.FILE_PRMBL_REMOVED, data)
            self.store_debug_data(df.FILE_PRMBL_IN, prmbl)

            # Create a new preamble
            self.preamble = preamble.create(self.preamble_size,
                                            self.num_pages)
            self.store_debug_data(df.FILE_PRMBL_OUT, self.preamble)

            # Verify the preamble by matching to the expected preamble
            if prmbl != self.preamble:
                raise RuntimeError('Preamble is invalid. Does not match the expected preamble file.')

        # Extract the header
        data, self.header = extract_header(data, self.header_size)
        self.validate_header(self.header)
        self.store_debug_data(df.FILE_HDR_REMOVED, data)
        self.store_debug_data(df.FILE_HDR_IN, self.header.pack())
        self.store_debug_data(df.FILE_HDR_IN_REPR, repr(self.header), suffix=df.FILE_HDR_IN_REPR_SUFFIX)

        # Extract the encryption params
        data, self.encryption_params = extract_encryption_params(data, self.header)
        self.store_debug_data(df.FILE_ENCRYPTION_PARAMS_REMOVED, data)
        self.store_debug_data(df.FILE_ENCRYPTION_PARAMS_IN, self.encryption_params)

        # Extract the cert chain
        data, self.cert_chain = extract_cert_chain(data, self.header)
        self.store_debug_data(df.FILE_CERT_CHAIN_REMOVED, data)
        self.store_debug_data(df.FILE_CERT_CHAIN_IN, self.cert_chain)

        # Extract the signature
        data, self.sign = extract_sign(data, self.header)
        self.store_debug_data(df.FILE_SIGN_REMOVED, data)
        self.store_debug_data(df.FILE_SIGN_IN, self.sign)

        # Save the remaining as code
        self.code = data
        self.store_debug_data(df.FILE_CODE_IN, self.code)

    def _repr_properties(self):
        properties = [
                      ('Header Size', str(self.header_size) + 'B'),
                      ('Has Preamble', self.has_preamble),
                      ('Preamble Size', self.preamble_size),
                      ('Has Magic Num', self.has_magic_num),
                      ('Page Size',  self.page_size),
                      ('Num Pages', self.num_pages),
                      ('Ota Enabled', self.ota_enabled),
                      ('Ota Min Size', self.ota_min_size),
                     ]
        return properties

    def __repr__(self):
        return (properties_repr(self._repr_properties()) + '\n' +
                'Header: ' + '\n' +
                repr(self.header))

    def store_debug_data(self, file_name, data, prefix=None, suffix=None):
        if prefix is None:
            prefix = self.debug_prefix
        if suffix is None:
            suffix = self.debug_suffix
        if prefix is not None and suffix is not None:
            store_debug_data_to_file(prefix + '_' + file_name + suffix,
                                     data, self.debug_dir)

    def validate_configuration(self):
        # Validate the header
        self.validate_header(self.header)

        # Ensure ota params are available
        if self.ota_enabled:
            # Ensure that the data needed to pad for ota
            if self.page_size is None or self.num_pages is None or self.ota_min_size is None:
                raise RuntimeError('Page size, num of pages and ota min size must be provided if ota is enabled.')

        # Ensure that the data needed to create preamble is available
        if self.has_preamble:
            if self.preamble_size is None or self.num_pages is None:
                raise RuntimeError('Preamble size and num of pages must be provided if preamble is enabled.')

    def get_header(self):
        hdr = self.header

        # Update the pointer & sizes
        hdr.code_size = len(self.code)
        hdr.sig_ptr = hdr.image_dest_ptr + hdr.code_size
        hdr.sig_size = len(self.sign)
        hdr.cert_chain_ptr = hdr.sig_ptr + hdr.sig_size
        hdr.cert_chain_size = len(self.cert_chain)

        # Calculate & byte align the total image size
        image_size = ALIGNED_IMAGE_SIZE(hdr.code_size +
                                        hdr.sig_size +
                                        hdr.cert_chain_size)

        # Update the image size in the header
        hdr.image_size = image_size

        return hdr

    def validate_header(self, hdr):
        expected_image_size = ALIGNED_IMAGE_SIZE(hdr.code_size +
                                                 hdr.sig_size +
                                                 hdr.cert_chain_size)

        if hdr.image_size != expected_image_size:
            raise RuntimeError('Mbn header verification failed. '
                               'Image size does not match the sum of its segments:' + '\n'
                               '\n'
                               '    ' + 'Code Size:               ' + hex_addr(hdr.code_size) + '\n'
                               '    ' + 'Signature Size:          ' + hex_addr(hdr.sig_size) + '\n'
                               '    ' + 'Cert Chain Size:         ' + hex_addr(hdr.cert_chain_size) + '\n'
                               '    ' + 'Image Size (expected):  *' + hex_addr(expected_image_size) + ' (Code + Signature + Cert Chain)' + '\n'
                               '    ' + 'Image Size (from file): *' + hex_addr(hdr.image_size) + '\n'
                               '\n'
                               'Header:' + '\n' +
                                repr(hdr)
                              )

    def get_data(self):
        data = (self.get_header().pack() + self.code +
                self.sign + self.cert_chain + self.encryption_params)

        if self.has_preamble:
            data = self.preamble + data
        if self.has_magic_num:
            data = magic.insert(data)[0]
        if self.ota_enabled:
            data = ota.insert_pad(data, self.page_size, self.num_pages, self.ota_min_size)

        return data

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'sign'
    #--------------------------------------------------------------------------
    @property
    def sign(self):
        return self._sign

    @sign.setter
    def sign(self, sign):
        if sign:
            self.validate_sign(sign)
        self._sign = sign

    def validate_sign(self, sign):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'cert_chain'
    #--------------------------------------------------------------------------
    @property
    def cert_chain(self):
        return self._cert_chain

    @cert_chain.setter
    def cert_chain(self, cert_chain):
        if cert_chain:
            self.validate_cert_chain(cert_chain)
        self._cert_chain = cert_chain

    def validate_cert_chain(self, cert_chain):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'encryption_params'
    #--------------------------------------------------------------------------
    @property
    def encryption_params(self):
        return self._encryption_params

    @encryption_params.setter
    def encryption_params(self, encryption_params):
        if encryption_params:
            self.validate_encryption_params(encryption_params)
        self._encryption_params = encryption_params

    def validate_encryption_params(self, encryption_params):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'header_size'
    #--------------------------------------------------------------------------
    @property
    def header_size(self):
        return self._header_size

    @header_size.setter
    def header_size(self, value):
        self.validate_header_size(value)
        self._header_size = value

    def validate_header_size(self, value):
        if not isinstance(value, int):
            raise AttributeError('Header size must be integer. Header size provided: ' + str(value))
        if value not in MBN_HDRS.keys():
            raise AttributeError('Invalid header size provided: ' + str(value) + '\n'
                                 '    ' + 'Supported header sizes are: ' + str(MBN_HDRS.keys()))

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'has_preamble'
    #--------------------------------------------------------------------------
    @property
    def has_preamble(self):
        return self.preamble_size is not None

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'preamble_size'
    #--------------------------------------------------------------------------
    @property
    def preamble_size(self):
        return self._preamble_size

    @preamble_size.setter
    def preamble_size(self, value):
        self.validate_preamble_size(value)
        self._preamble_size = value

    def validate_preamble_size(self, value):
        if value is not None:
            if not isinstance(value, int):
                raise AttributeError('Preamble size must be integer. Preamble size provided: ' + str(value))
            if value not in PRMBL_ATTRS.keys():
                raise AttributeError('Invalid preamble size provided: ' + str(value) + '\n'
                                     '    ' + 'Supported preamble sizes are: ' + str(PRMBL_ATTRS.keys()))

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'has_magic_num'
    #--------------------------------------------------------------------------
    @property
    def has_magic_num(self):
        return self._has_magic_num

    @has_magic_num.setter
    def has_magic_num(self, value):
        self.validate_has_magic_num(value)
        self._has_magic_num = True if value else False

    def validate_has_magic_num(self, value):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'ota_enabled'
    #--------------------------------------------------------------------------
    @property
    def ota_enabled(self):
        return self._ota_enabled

    @ota_enabled.setter
    def ota_enabled(self, value):
        self.validate_ota_enabled(value)
        self._ota_enabled = True if value else False

    def validate_ota_enabled(self, value):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'page_size'
    #--------------------------------------------------------------------------
    @property
    def page_size(self):
        return self._page_size

    @page_size.setter
    def page_size(self, value):
        self.validate_page_size(value)
        if value is not None:
            value = value * 1024
        self._page_size = value

    def validate_page_size(self, value):
        if value is not None:
            if not isinstance(value, int):
                raise AttributeError('Page_size size must be integer. Page_size size provided: ' + str(value))
            if value not in PRMBL_PAGE_SIZES:
                raise RuntimeError('Invalid page_size provided: ' + str(value) + '\n'
                                   '    ' + 'Supported page sizes are: ' + str(PRMBL_PAGE_SIZES))

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'num_pages'
    #--------------------------------------------------------------------------
    @property
    def num_pages(self):
        return self._num_pages

    @num_pages.setter
    def num_pages(self, value):
        self.validate_num_pages(value)
        self._num_pages = value

    def validate_num_pages(self, value):
        if value is not None:
            if not isinstance(value, int):
                raise AttributeError('Num_pages size must be integer. Num_pages size provided: ' + str(value))
            if value not in PRMBL_MAGIC_NUMS.keys():
                raise RuntimeError('Invalid num_pages provided: ' + str(value) + '\n'
                                   '    ' + 'Supported num pages are: ' + str(PRMBL_MAGIC_NUMS.keys()))

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'ota_min_size'
    #--------------------------------------------------------------------------
    @property
    def ota_min_size(self):
        return self._ota_min_size

    @ota_min_size.setter
    def ota_min_size(self, value):
        self.validate_ota_min_size(value)
        if value is not None:
            value = value * 1024
        self._ota_min_size = value

    def validate_ota_min_size(self, value):
        if value is not None:
            if not isinstance(value, int):
                raise AttributeError('Ota_min_size size must be integer. Ota_min_size size provided: ' + str(value))

def _extract_segment(data, offset, size):
    seg = ''
    if (offset > 0 and offset < len(data) and size > 0):
        seg = data[offset : offset + size]
        data = data[: offset]
    return data, seg

def extract_cert_chain(data, header):
    offset = header.cert_chain_ptr - header.image_dest_ptr
    size = header.cert_chain_size
    return _extract_segment(data, offset, size)

def extract_sign(data, header):
    offset = header.sig_ptr - header.image_dest_ptr
    size = header.sig_size
    return _extract_segment(data, offset, size)

def extract_encryption_params(data, header):
    offset = header.cert_chain_ptr + header.cert_chain_size - header.image_dest_ptr
    size = len(data) - offset
    return _extract_segment(data, offset, size)
