#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
import os
import sectools.features.isc.signer.cass.defines as defines
from sectools.features.isc.signer.cass.base_keystore import BaseKeystore
from sectools.common.utils import c_path, c_misc
import sectools.features.isc.signer.signerutils as signerutils
import sectools.common.crypto.utility_functions as utility_functions
from sectools.common.utils.c_logging import logger
from sectools.features.isc.signer.config_error import ConfigError
import sys

class IdentityKeystoreFactory(object):
    @classmethod
    def getKeyStore(cls, user_identity_config):
        keystoreType = user_identity_config.keystore_type
        if keystoreType == HardwareIdentityKeystore.KEYSTORE_TYPE:
            keystore = HardwareIdentityKeystore(user_identity_config)
        elif keystoreType == SoftwareIdentityKeystore.KEYSTORE_TYPE:
            keystore = SoftwareIdentityKeystore(user_identity_config)
        else:
            raise RuntimeError, "Unsupported keystore_type = {0}".format(keystoreType)

        return keystore


class HardwareIdentityKeystore(BaseKeystore):
    KEYSTORE_TYPE = "PKCS11"
    PKCS11_CFG_TEMPLATE=c_path.normalize(os.path.join(defines.CASS_CLIENT_REFAPP_INPUT_DIR, 'pkcs11.cfg.template'))
    MESG_PROMPT_PASSWORD = '\nPlease enter the password for the USB token (user identity):'
    MESG_TOKEN_DRIVER_INVALID = "Token driver path {0} is invalid! Please install Safenet driver."

    def __init__(self, user_identity_config):
        self.file = None
        self._user_identity = user_identity_config
        BaseKeystore.__init__(self, user_identity_config.token_password)
        self.token_driver_home = user_identity_config.token_driver_home
        self.file = self._generate_pkcs11_cfg(self.token_driver_home)

    @property
    def file(self):
        return self._file

    @file.setter
    def file(self, value):
        self._file = value

    @property
    def type(self):
        return self.KEYSTORE_TYPE

    @property
    def token_driver_home(self):
        return self._token_driver_home

    @token_driver_home.setter
    def token_driver_home(self, token_driver_home_config):
        if sys.platform=='linux2':
            self._token_driver_home = token_driver_home_config.linux
        else:
            self._token_driver_home = token_driver_home_config.windows

        if c_path.validate_file(self._token_driver_home) is False:
            raise ConfigError(self.MESG_TOKEN_DRIVER_INVALID.format(self._token_driver_home))

    def _generate_pkcs11_cfg(self, token_driver_home):
        pkcs11_cfg_template_data = c_misc.load_data_from_file(self.PKCS11_CFG_TEMPLATE)
        pkcs11_cfg_data = signerutils.macro_replace(pkcs11_cfg_template_data,
                                  "token_driver_home",
                                  token_driver_home,
                                  isMandatory=True)
        return utility_functions.store_data_to_temp_file(pkcs11_cfg_data)

    def release(self):
        if self.file and os.path.isfile(self.file):
            logger.debug("Deleting temporary file: " + self.file)
            os.unlink(self.file)

    def __del__(self):
        self.release()


class SoftwareIdentityKeystore(BaseKeystore):
    KEYSTORE_TYPE = "PKCS12"
    MESG_PROMPT_PASSWORD = '\nPlease enter the password for the softkey (user identity):'

    def __init__(self, user_identity_config):
        self._user_identity = user_identity_config
        BaseKeystore.__init__(self, user_identity_config.token_password)
        self._file = c_path.normalize(user_identity_config.keystore_file)

    @property
    def file(self):
        return self._file

    @property
    def type(self):
        return self.KEYSTORE_TYPE

    @property
    def token_driver_home(self):
        return None

