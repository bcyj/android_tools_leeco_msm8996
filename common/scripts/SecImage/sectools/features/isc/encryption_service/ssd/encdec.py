#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from sectools.features.isc.encryption_service.base.encdec import BaseEncdec
from sectools.features.isc.encryption_service.ssd.encryptor import Encryptor
from sectools.features.isc.encryption_service.ssd.decryptor import Decryptor
from sectools.features.isc.encryption_service.ssd.encryption_parameters import EncryptionParameters


SSD_ENCRYPTION_IDENT    = 'ssd_encryption'


class Encdec(BaseEncdec):

    @property
    def _encryptor_class(self):
        return Encryptor

    @property
    def _decryptor_class(self):
        return Decryptor

    @property
    def _encryption_parameters_class(self):
        return EncryptionParameters

    def get_dummy_key(self):
        return self.encryption_parameters._get_private_key_from_config_file()

    def extract_encryption_parameters(self, data):
        return self.encryption_parameters.extract_encryption_parameters(data)

    @classmethod
    def ident(self):
        return SSD_ENCRYPTION_IDENT

Encdec.register()
