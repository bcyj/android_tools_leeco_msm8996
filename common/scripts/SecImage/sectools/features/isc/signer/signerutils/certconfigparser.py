#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from abc import ABCMeta, abstractmethod
from sectools.common.crypto import utility_functions
import os
import sectools.features.isc.signer.signerutils as signerutils
from sectools.features.isc.signer.signerutils import StringProcessor
from sectools.common.utils import c_misc
from sectools.common.utils.c_logging import logger

class BaseCertConfigParser(object):
    __metaclass__=ABCMeta

    def __init__(self, cert_config, signing_attributes, general_properties):
        self.cert_config = cert_config
        self.signing_attributes = signing_attributes
        self.general_properties = general_properties

    @abstractmethod
    def get_crypto_params(self):
        raise NotImplementedError()

    @abstractmethod
    def get_rootcerts(self):
        raise NotImplementedError()

    @classmethod
    def replace_macros(cls, string_to_replace, exponent, key_size, index = None ):
        str_processor = StringProcessor(string_to_replace)
        if index is not None:
            str_processor.addMacroReplaceArgs("index", index, isMandatory=True)
        str_processor.addMacroReplaceArgs("exponent", exponent)
        str_processor.addMacroReplaceArgs("key_size", key_size)
        return str_processor.get_replaced_string()

    def _certificate_properties_parse(self, cert_info):
        if cert_info.params_list:
            params = utility_functions.normalize_param_list_into_dict(cert_info.params_list.cert_param)
        else:
            params = {}
            params['certificate_path'] = self.replace_macros(
                                cert_info.preexisting_cert.cert_path,
                                exponent=str(self.signing_attributes.exponent),
                                key_size=str(self.general_properties.key_size))

            params['private_key_path'] = self.replace_macros(
                                cert_info.preexisting_cert.private_key_path,
                                exponent=str(self.signing_attributes.exponent),
                                key_size=str(self.general_properties.key_size))

        return params


class CertConfigParser(object):
    def __init__(self, cert_config, signing_attributes, general_properties):
        '''
        Constructor
        '''
        if cert_config.multirootcert:
            self.cert_config_parser = MultiCertConfigParser(cert_config,
                                                            signing_attributes,
                                                            general_properties)
        else:
            self.cert_config_parser = GenericCertConfigParser(cert_config,
                                                              signing_attributes,
                                                              general_properties)


    def get_crypto_params(self):
        return self.cert_config_parser.get_crypto_params()

    def get_rootcerts(self):
        return self.cert_config_parser.get_rootcerts()

    @classmethod
    def replace_macros(cls, string_to_replace, exponent, key_size, index = None ):
        return BaseCertConfigParser.replace_macros(string_to_replace, exponent, key_size, index)


class MultiCertConfigParser(BaseCertConfigParser):
    def __init__(self, cert_config, signing_attributes, general_properties):
        BaseCertConfigParser.__init__(self, cert_config,
                                      signing_attributes,
                                      general_properties)

    def _multirootcert_properties_parse(self, multirootcert_config, config_cert_name, config_key_name):
        params = {}

        params['certificate_path'] = self.replace_macros(
                                os.path.join(multirootcert_config.directory, config_cert_name),
                                exponent=str(self.signing_attributes.exponent),
                                key_size=str(self.general_properties.key_size),
                                index=str(multirootcert_config.index))

        params['private_key_path'] = self.replace_macros(
                                os.path.join(multirootcert_config.directory, config_key_name),
                                exponent=str(self.signing_attributes.exponent),
                                key_size=str(self.general_properties.key_size),
                                index=str(multirootcert_config.index))

        return params

    def get_crypto_params(self):
        crypto_params_dict = {}

        crypto_params_dict['root_certificate_properties'] = \
                self._multirootcert_properties_parse(self.cert_config.multirootcert,
                                                     self.cert_config.multirootcert.root_cert_name,
                                                     self.cert_config.multirootcert.root_key_name)
        crypto_params_dict['attest_ca_certificate_properties'] = \
                self._multirootcert_properties_parse(self.cert_config.multirootcert,
                                                     self.cert_config.multirootcert.attest_ca_cert_name,
                                                     self.cert_config.multirootcert.attest_ca_key_name)
        crypto_params_dict['attest_certificate_properties'] = self._certificate_properties_parse(self.cert_config.attest_cert)

        return crypto_params_dict

    def get_rootcerts(self):
        root_cert_list = []
        for i in range(0, self.general_properties.num_root_certs):
            config_cert_name = self.cert_config.multirootcert.root_cert_name
            root_cert_path_to_replace = os.path.join(self.cert_config.multirootcert.directory, config_cert_name)

            root_cert_path = self.replace_macros(
                                root_cert_path_to_replace,
                                exponent=str(self.signing_attributes.exponent),
                                key_size=str(self.general_properties.key_size),
                                index=str(i))

            if os.path.isfile(root_cert_path) is False:
                err_str = "certificate_path does not exist: {0}!".format(root_cert_path)
                raise RuntimeError(err_str)
            root_cert = c_misc.load_data_from_file(root_cert_path)
            root_cert_list.append(root_cert)
            logger.debug('Package root cert {0}: {1}'.format(i, root_cert_path))

        return root_cert_list

class GenericCertConfigParser(BaseCertConfigParser):
    def __init__(self, cert_config, signing_attributes, general_properties):
        BaseCertConfigParser.__init__(self, cert_config,
                                      signing_attributes,
                                      general_properties)

    def get_crypto_params(self):
        crypto_params_dict = {}

        crypto_params_dict['root_certificate_properties'] = \
                self._certificate_properties_parse(self.cert_config.root_cert)
        if self.cert_config.attest_ca_cert != None:
            crypto_params_dict['attest_ca_certificate_properties'] = \
                self._certificate_properties_parse(self.cert_config.attest_ca_cert)
        crypto_params_dict['attest_certificate_properties'] = \
                self._certificate_properties_parse(self.cert_config.attest_cert)

        return crypto_params_dict

    def get_rootcerts(self):
        root_cert_list = []
        return root_cert_list