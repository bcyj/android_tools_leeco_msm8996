#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
from abc import abstractproperty, ABCMeta, abstractmethod


class BaseEncdec(object):
    __metaclass__=ABCMeta

    def __init__(self, imageinfo, debug_dir=None):
        self.debug_dir = debug_dir
        self.config = imageinfo.encryption
        self.encryption_parameters = self._encryption_parameters_class(config=self.config, debug_dir=self.debug_dir)

    def get_encryptor(self, encryption_parameters_blob=None, key=None):
        encryption_parameters = self.encryption_parameters
        if encryption_parameters_blob and key:
            encryption_parameters = self._encryption_parameters_class(encryption_parameters_blob,
                                                                      key,
                                                                      self.debug_dir)
        return self._encryptor_class(encryption_parameters, debug_dir=self.debug_dir)

    def get_decryptor(self, encryption_parameters_blob=None, key=None):
        encryption_parameters = self.encryption_parameters
        if encryption_parameters_blob and key:
            encryption_parameters = self._encryption_parameters_class(encryption_parameters_blob,
                                                                      key,
                                                                      self.config,
                                                                      self.debug_dir)
        return self._decryptor_class(encryption_parameters, debug_dir=self.debug_dir)

    def get_encryption_parameters_blob(self):
        return self.encryption_parameters.get_encryption_params_blob()

    @abstractproperty
    def _encryptor_class(self):
        raise NotImplementedError

    @abstractproperty
    def _decryptor_class(self):
        raise NotImplementedError

    @abstractproperty
    def _encryption_parameters_class(self):
        raise NotImplementedError

    @abstractmethod
    def get_dummy_key(self):
        raise NotImplementedError

    @abstractmethod
    def extract_encryption_parameters(self, data):
        raise NotImplementedError

    @classmethod
    def ident(self):
        raise NotImplementedError

    @classmethod
    def register(cls):
        from sectools.features.isc.encryption_service import _register_encdec
        _register_encdec(cls.ident(), cls)

