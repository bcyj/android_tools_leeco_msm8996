#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Dec 9, 2013

@author: hraghav

This module contains the ParserBase class which is the base class for any
secure data parsing classes.
'''

import abc

from sectools.common.parsegen import PAD_BYTE_1
from sectools.common.utils.c_misc import properties_repr,\
    store_debug_data_to_file


class SecParseGenBase(object):

    __metaclass__ = abc.ABCMeta

    def __init__(self, data, imageinfo=None, general_properties=None, encdec=None,
                 debug_dir=None,
                 debug_prefix=None,
                 debug_suffix=None,):
        """
        Initializes properties and methods which are expected in all the
        parsers. These properties and methods can be overloaded by the derived
        class for parser specific logic. Also sets some properties based on the
        config object.

        Parameters:
        #. data (str): Data to be parsed.
        #. imageinfo (obj): Config object.

        Attributes:
        #. cert_chain_size (int): Estimated maximum size of the cert chain.
        #. max_cert_size (int): Estimated maximum size of one cert.
        #. sig_size (int): Size of the signature.
        #. num_certs_in_certchain(int): Number of certs in the cert chain.
        #. num_root_certs (int): Number of root certs used in the cert chain.
        """
        # Check the arguments
        if imageinfo is not None:
            general_properties = imageinfo.general_properties
            encdec = imageinfo.encdec
        if general_properties is None:
            raise RuntimeError('General properties must not be None.')

        # Initialize internal properties
        self._max_cert_size = 0
        self._sig_size = 0
        self._num_certs_in_certchain = 0
        self._num_root_certs = 0

        # Public properties
        self.encdec = encdec
        self.debug_dir = debug_dir
        self.debug_prefix = debug_prefix
        self.debug_suffix = debug_suffix

        # Variables for security data
        self._data_signature = ''
        self._cert_chain = ''
        self._encryption_params = ''

        # Options for what security mechanism to apply
        self._integrity_check = False
        self._sign = False
        self._encrypt = False

        # Set properties from the config file
        self.max_cert_size = general_properties.max_cert_size
        self.sig_size = general_properties.key_size / 8
        self.num_certs_in_certchain = general_properties.num_certs_in_certchain
        self.num_root_certs = general_properties.num_root_certs

    def _repr_properties(self):
        properties = [
                      ('Integrity Check', self.contains_integrity_check()),
                      ('Signed', self.is_signed()),
                      ('Encrypted', self.is_encrypted()),
                      ('Size of signature', self.sig_size),
                      ('Size of one cert', self.max_cert_size),
                      ('Num of certs in cert chain',  self.num_certs_in_certchain),
                      ('Number of root certs', self.num_root_certs),
                      ('Cert chain size', self.cert_chain_size),
                     ]
        return properties

    def __repr__(self):
        return properties_repr(self._repr_properties())

    @classmethod
    def register(cls):
        from sectools.features.isc.parsegen import _register_parser
        _register_parser(cls.file_type(), cls)

    #--------------------------------------------------------------------------
    # Properties & Functions that must be implemented by the derived classes
    #--------------------------------------------------------------------------
    @classmethod
    def file_type(cls):
        raise NotImplemented

    @abc.abstractmethod
    def get_data(self, integrity_check, sign, encrypt):
        #if encrypt and not self.is_signed():
        #    raise RuntimeError('Image is not signed. Cannot generate encrypted image.')
        if sign and not self.is_signed():
            raise RuntimeError('Image does not contain the required security metadata (signature & cert chain) needed to generate signed image.')

    @abc.abstractproperty
    def data_to_sign(self):
        """
        Return the binary data blob that is used for signing (creating the data
        signature).
        """
        raise RuntimeError('Method must be implemented by the derived class')

    #--------------------------------------------------------------------------
    # Properties that can be overloaded by the derived classes (optional)
    #--------------------------------------------------------------------------
    @property
    def data_signature(self):
        return self._data_signature

    @data_signature.setter
    def data_signature(self, signature):
        if signature:
            self.validate_data_signature(signature)
        self._data_signature = signature

    def validate_data_signature(self, signature):
        if not isinstance(signature, str):
            raise RuntimeError('Signature must be of type "str". Given signature type is: "' + type(signature) + '"')
        #This is commented because this check fails 2k key to 4k key resigning
        #if len(signature) != self.sig_size:
        #    raise RuntimeError('Length of signature "' + str(len(signature)) + '" does not match the expected signature size "' + str(self.sig_size) + '"')

    @property
    def cert_chain(self):
        return self._cert_chain

    @cert_chain.setter
    def cert_chain(self, cert_chain):
        if cert_chain:
            self.validate_cert_chain(cert_chain)
            cert_chain = cert_chain.ljust(self.cert_chain_size, PAD_BYTE_1)
        self._cert_chain = cert_chain

    def validate_cert_chain(self, cert_chain):
        if not isinstance(cert_chain, str):
            raise RuntimeError('Cert chain must be of type "str". Given cert chain type is: "' + type(cert_chain) + '"')
        if len(cert_chain) > self.cert_chain_size:
            raise RuntimeError('Length of cert chain "' + str(len(cert_chain)) + '" is greater than the maximum cert_chain size "' + str(self.cert_chain_size) + '"')

    @property
    def encryption_params(self):
        return self._encryption_params

    @encryption_params.setter
    def encryption_params(self, encryption_params):
        if encryption_params:
            self.validate_encryption_params(encryption_params)
        self._encryption_params = encryption_params

    def validate_encryption_params(self, encryption_params):
        if not isinstance(encryption_params, str):
            raise RuntimeError('Encryption params must be of type "str". Given encryption params type is: "' + type(encryption_params) + '"')

    def get_wrapped_data(self):
        raise NotImplementedError

    def set_wrapped_data(self, data):
        raise NotImplementedError

    #--------------------------------------------------------------------------
    # Tests to check various attributes of the data
    #--------------------------------------------------------------------------
    def is_signed(self):
        """ Returns True if data contains all the security metadata """
        return self.contains_data_signature() and self.contains_cert_chain()

    def is_encrypted(self):
        """ Returns True if data contains all the encryption metadata """
        return self.contains_encryption_params()

    @abc.abstractmethod
    def contains_integrity_check(self):
        """ Returns True if data contains integrity check metadata """
        pass

    def contains_data_signature(self):
        """ Returns True if data contains the data signature """
        return self.data_signature != ''

    def contains_cert_chain(self):
        """ Returns True if data contains the certificate chain """
        return self.cert_chain != ''

    def contains_encryption_params(self):
        """ Returns True if data contains the encryption params """
        return self.encryption_params != ''

    #--------------------------------------------------------------------------
    # Get & Set functions for 'cert_chain_size'
    #--------------------------------------------------------------------------
    @property
    def cert_chain_size(self):
        return (self.max_cert_size * (self.num_certs_in_certchain + self.num_root_certs - 1))

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'max_cert_size'
    #--------------------------------------------------------------------------
    @property
    def max_cert_size(self):
        return self._max_cert_size

    @max_cert_size.setter
    def max_cert_size(self, value):
        self.validate_max_cert_size(value)
        self._max_cert_size = value

    def validate_max_cert_size(self, value):
        if value <= 0:
            raise AttributeError('Invalid size of one cert "' + str(value) + '" provided. Must be greater than 0')

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'sig_size'
    #--------------------------------------------------------------------------
    @property
    def sig_size(self):
        return self._sig_size

    @sig_size.setter
    def sig_size(self, value):
        self.validate_sig_size(value)
        self._sig_size = value

    def validate_sig_size(self, value):
        if value <= 0:
            raise AttributeError('Invalid size of signature "' + str(value) + '" provided. Must be greater than 0')

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'num_certs_in_certchain'
    #--------------------------------------------------------------------------
    @property
    def num_certs_in_certchain(self):
        return self._num_certs_in_certchain

    @num_certs_in_certchain.setter
    def num_certs_in_certchain(self, value):
        self.validate_num_certs_in_certchain(value)
        self._num_certs_in_certchain = value

    def validate_num_certs_in_certchain(self, value):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'num_root_certs'
    #--------------------------------------------------------------------------
    @property
    def num_root_certs(self):
        return self._num_root_certs

    @num_root_certs.setter
    def num_root_certs(self, value):
        self.validate_num_root_certs(value)
        self._num_root_certs = value

    def validate_num_root_certs(self, value):
        if value <= 0:
            raise AttributeError('Invalid number of root certs "' + str(value) + '" provided. Must be greater than 0')

    #--------------------------------------------------------------------------
    # Get, Set functions for 'integrity_check'
    #--------------------------------------------------------------------------
    @property
    def integrity_check(self):
        return self._integrity_check

    @integrity_check.setter
    def integrity_check(self, value):
        self._integrity_check = True if value else False

    #--------------------------------------------------------------------------
    # Get, Set functions for 'sign'
    #--------------------------------------------------------------------------
    @property
    def sign(self):
        return self._sign

    @sign.setter
    def sign(self, value):
        self._sign = True if value else False

    #--------------------------------------------------------------------------
    # Get, Set functions for 'encrypt'
    #--------------------------------------------------------------------------
    @property
    def encrypt(self):
        return self._encrypt

    @encrypt.setter
    def encrypt(self, value):
        self._encrypt = True if value else False

    #--------------------------------------------------------------------------
    # Helper functions
    #--------------------------------------------------------------------------
    def store_debug_data(self, file_name, data, prefix=None, suffix=None):
        if prefix is None:
            prefix = self.debug_prefix
        if suffix is None:
            suffix = self.debug_suffix
        if prefix is not None and suffix is not None:
            store_debug_data_to_file(prefix + '_' + file_name + suffix,
                                     data, self.debug_dir)

    def encrypt_segment(self, segment_data, segment_num):
        return self.encdec.get_encryptor().encrypt_segment(segment_data, segment_num)

