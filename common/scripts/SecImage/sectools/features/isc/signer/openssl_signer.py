#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from base_signer import BaseSigner, SignerOutput
from sectools.common.crypto import utility_functions, crypto_functions
from sectools.common.crypto import common_paths as paths
from sectools.common.utils import c_misc, c_path
from ..hasher import Hasher
from sectools.common.utils.c_logging import logger
import os
import signerutils
from config_error import ConfigError
from signerutils.certconfigparser import CertConfigParser
from sectools.common.utils.c_attribute import Attribute
from certificate import Certificate

class OpenSSLSigner(BaseSigner):
    '''
    classdocs
    '''
    def __init__(self, config):
        '''
        Constructor
        '''
        self.config = config
        self.path_to_openssl_binary = paths.openssl_binary_path


    def sign(self, hash_to_sign, imageinfo, binary_to_sign = None, debug_dir=None):
        return self._sign(hash_to_sign,
                   imageinfo.cert_config,
                   imageinfo.signing_attributes,
                   imageinfo.general_properties,
                   binary_to_sign)


    def _sign(self, hash_to_sign,
                    cert_config,
                    signing_attributes,
                    general_properties,
                    binary_to_sign):

        openssl_config_file_paths = self.config.signing.signer_attributes.local_signer_attributes.openssl_config_inputs

        self._validate_config(cert_config, general_properties, openssl_config_file_paths)

        # Obtain all the information from the signing_attributes

        debug_val = int(signing_attributes.debug, 16) if signing_attributes.debug is not None else None
        msm_part = int(signing_attributes.msm_part, 16)
        oem_id = int(signing_attributes.oem_id, 16)
        model_id = int(signing_attributes.model_id, 16)
        num_certs_in_certchain = general_properties.num_certs_in_certchain
        app_id = int(signing_attributes.app_id, 16) if signing_attributes.app_id is not None else None
        crash_dump = int(signing_attributes.crash_dump, 16) if signing_attributes.crash_dump is not None else None
        rot_en = int(signing_attributes.rot_en, 16) if signing_attributes.rot_en is not None else None

        # Create the crypto_params_dict
        self.certconfig_parser = CertConfigParser(cert_config, signing_attributes, general_properties)
        crypto_params_dict = self.certconfig_parser.get_crypto_params()

        hmac_params = signerutils.get_hmac_params_from_config(signing_attributes)

        # Create the attestation_certificate_key_pair
        attestation_certificate_key_pair = None

        root_certificate_params = crypto_params_dict['root_certificate_properties']
        root_certificate_params_is_valid, generate_new_root_certificate = self._validate_certificate_params_dict(root_certificate_params)
        if root_certificate_params_is_valid:
            if generate_new_root_certificate:
                logger.info('Generating new Root certificate and a random key')
                generated_root_key_pair = crypto_functions.gen_rsa_key_pair(general_properties.key_size, key_exponent=signing_attributes.exponent)
                root_cert, root_key_pair = crypto_functions.create_root_certficate(root_certificate_params, generated_root_key_pair, 7300, openssl_config_file_paths.openssl_configfile_path, 1)
            else:
                logger.info('Using a predefined Root certificate and a predefined key')
                logger.info('Key Used: '+ root_certificate_params['private_key_path'])
                logger.info('Certificate Used: '+ root_certificate_params['certificate_path'])
                root_cert, root_key_pair = self._get_certificate_and_key_pair_from_files(root_certificate_params)
        else:
            logger.error("Root certificate params are invalid! Please check config file.")
            raise RuntimeError("Root certificate params are invalid! Please check config file.")

        if num_certs_in_certchain > 2:
            logger.debug("Generating Attestation CA certificate, since certchain size is greater than 2")
            attestation_ca_certificate_params = crypto_params_dict['attest_ca_certificate_properties']
            attestation_ca_params_is_valid, generate_new_attestation_ca = self._validate_certificate_params_dict(attestation_ca_certificate_params)
            if attestation_ca_params_is_valid:
                if generate_new_attestation_ca:
                    logger.info('Generating new Attestation CA certificate and a random key')
                    generated_attestation_ca__key_pair = crypto_functions.gen_rsa_key_pair(general_properties.key_size, key_exponent=signing_attributes.exponent)
                    attestation_ca_certificate, attestation_ca_certificate_key_pair = \
                        crypto_functions.create_certificate(attestation_ca_certificate_params,
                                                            generated_attestation_ca__key_pair,
                                                            root_cert,
                                                            root_key_pair,
                                                            days=7300,
                                                            configfile=openssl_config_file_paths.openssl_configfile_path,
                                                            serial_num=1,
                                                            extfile_name=openssl_config_file_paths.ca_certificate_extensions_path)
                else:
                    logger.info('Using a predefined Attestation CA certificate and a predefined key')
                    logger.info('Key Used: '+ attestation_ca_certificate_params['private_key_path'])
                    logger.info('Certificate Used: '+ attestation_ca_certificate_params['certificate_path'])
                    attestation_ca_certificate, attestation_ca_certificate_key_pair = self._get_certificate_and_key_pair_from_files(attestation_ca_certificate_params)
            else:
                logger.error("Attestation CA certificate params are invalid! Please check config file.")
                raise RuntimeError("Attestation CA certificate params are invalid! Please check config file.")

        attestation_certificate_params = crypto_params_dict['attest_certificate_properties']
        attestation_certificate_params_is_valid, generate_new_attestation_certificate = self._validate_certificate_params_dict(attestation_certificate_params)

        if attestation_certificate_params_is_valid:
            if generate_new_attestation_certificate:
                #TCG support
                if self._is_tcg_supported(signing_attributes) is True:
                    if self.validate_tcg_from_config(attestation_ca_certificate_params['certificate_path'], signing_attributes) is False:
                        raise ConfigError("tcg_min and tcg_max are not set correctly in configuration."\
                                          "Signing will not continue."
                                          )
                    attestation_certificate_extensions_path = self._generate_attestation_certificate_extensions(
                                                                    openssl_config_file_paths.attestation_certificate_extensions_path,
                                                                    signing_attributes.tcg_min,
                                                                    signing_attributes.tcg_max)
                else:
                    attestation_certificate_extensions_path = openssl_config_file_paths.attestation_certificate_extensions_path


                logger.info('Generating new Attestation certificate and a random key')
                certificate_ou_sw_id ="01 " + hmac_params.sw_id_str + " SW_ID"
                certificate_ou_hw_id ="02 " + hmac_params.msm_id_str + " HW_ID"
                certificate_ou_oem_id ="04 " + "%0.4X" % oem_id + " OEM_ID"
                certificate_ou_sw_size ="05 " + "%0.8X" % len(binary_to_sign) + " SW_SIZE"
                certificate_ou_model_id ="06 " + "%0.4X" % model_id + " MODEL_ID"
                certificate_hash_alg = "07 0001 SHA256"

                certificate_ou = [
                                  certificate_ou_sw_id,
                                  certificate_ou_hw_id,
                                  certificate_ou_oem_id,
                                  certificate_ou_sw_size,
                                  certificate_ou_model_id,
                                  certificate_hash_alg
                                 ]
                #Optional attributes
                if debug_val is not None:
                    certificate_ou_debug_id ="03 " + "%0.16X" % debug_val + " DEBUG"
                    certificate_ou.append(certificate_ou_debug_id)
                if app_id is not None:
                    certificate_app_id = "08 " + "%0.16X" % app_id + " APP_ID"
                    certificate_ou.append(certificate_app_id)
                if crash_dump is not None:
                    certificate_crash_dump = "09 " + "%0.16X" % crash_dump + " CRASH_DUMP"
                    certificate_ou.append(certificate_crash_dump)
                if rot_en is not None:
                    certificate_rot_en = "10 " + "%0.16X" % rot_en + " ROT_EN"
                    certificate_ou.append(certificate_rot_en)

                if 'OU' in attestation_certificate_params.keys():
                    if type(attestation_certificate_params['OU'])==list:
                        for item in attestation_certificate_params['OU']:
                            certificate_ou.append(item)
                    else:
                        certificate_ou.append(attestation_certificate_params['OU'])

                attestation_certificate_params['OU']=certificate_ou

                if attestation_certificate_key_pair is None:
                    attestation_certificate_key_pair = crypto_functions.gen_rsa_key_pair(key_exponent=signing_attributes.exponent,
                                                                                         key_size_in_bits = general_properties.key_size)
                if num_certs_in_certchain > 2: #sign the attestation cert with the attestation_ca_cert
                    attestation_certificate, attestation_certificate_key_pair = \
                        crypto_functions.create_certificate(attestation_certificate_params,
                                                            attestation_certificate_key_pair,
                                                            attestation_ca_certificate,
                                                            attestation_ca_certificate_key_pair,
                                                            days=7300,
                                                            configfile=openssl_config_file_paths.openssl_configfile_path,
                                                            serial_num=1,
                                                            extfile_name=attestation_certificate_extensions_path)
                else: #sign the attestation cert with the root cert
                    attestation_certificate, attestation_certificate_key_pair = \
                        crypto_functions.create_certificate(attestation_certificate_params,
                                                            attestation_certificate_key_pair,
                                                            root_cert,
                                                            root_key_pair,
                                                            days=7300,
                                                            configfile=openssl_config_file_paths.openssl_configfile_path,
                                                            serial_num=1,
                                                            extfile_name=attestation_certificate_extensions_path)
                attestation_certificate = crypto_functions.cert_pem_to_der(attestation_certificate)

                #Clean temp file
                if self._is_tcg_supported(signing_attributes) is True:
                    c_path.clean_file(attestation_certificate_extensions_path)


            else:           #generate_new_attestation_certificate == False
                logger.info('Using a predefined Attestation certificate and a predefined key')
                logger.info('Key Used: '+ attestation_certificate_params['private_key_path'])
                logger.info('Certificate Used: '+ attestation_certificate_params['certificate_path'])
                attestation_certificate, attestation_certificate_key_pair = self._get_certificate_and_key_pair_from_files(attestation_certificate_params)
                attestation_certificate = crypto_functions.cert_pem_to_der(attestation_certificate)

                # Since the get_hmac_params_from_certificate_chain always works with the first cert in the cert chain,
                # this function will work for a single der certificate as well.
                hmac_params = crypto_functions.get_hmacparams_from_certificate_chain(attestation_certificate)
                hasher = Hasher()
                hash_to_sign=hasher.qcom_hmac(binary_to_sign, hmac_params)

            signature = crypto_functions.encrypt_with_private_key(hash_to_sign, attestation_certificate_key_pair['private_key'])
        else:
            logger.error("Attestation certificate params are invalid! Please check config file.")
            raise RuntimeError("Attestation certificate params are invalid! Please check config file.")

        if num_certs_in_certchain > 2:
            attestation_ca_certificate = crypto_functions.cert_pem_to_der(attestation_ca_certificate)
        else:
            attestation_ca_certificate = None

        root_cert = crypto_functions.cert_pem_to_der(root_cert)

        root_cert_list = self.certconfig_parser.get_rootcerts()
        certificate_list = self._get_certificate_list(general_properties.num_root_certs,
                                                      num_certs_in_certchain,
                                                      attestation_certificate,
                                                      attestation_ca_certificate,
                                                      root_cert,
                                                      root_cert_list)

        cert_chain=crypto_functions.create_certificate_chain(certificate_list)

        signer_output = SignerOutput()
        signer_output.root_cert = root_cert
        signer_output.attestation_ca_cert = attestation_ca_certificate
        signer_output.attestation_cert = attestation_certificate
        signer_output.signature = signature
        signer_output.cert_chain = cert_chain
        signer_output.root_cert_list = root_cert_list
        signer_output.attestation_key = attestation_certificate_key_pair['private_key']

        return signer_output

    def _get_certificate_list(self, num_root_certs,
                              num_certs_in_certchain,
                              attestation_certificate,
                              attestation_ca_certificate,
                              root_cert,
                              root_cert_list):
        certificate_list = [attestation_certificate]
        if num_certs_in_certchain > 2:
            certificate_list.append(attestation_ca_certificate)
        if num_root_certs == 1:
            certificate_list.append(root_cert)
        else:
            #multirootcert case
            for i in range(0, num_root_certs):
                certificate_list.append(root_cert_list[i])

        return certificate_list

    def _get_certificate_and_key_pair_from_files(self, certificate_params_dict):
        key_pair={}
        with open(certificate_params_dict['private_key_path'], 'rb') as private_key_file:
            private_key = private_key_file.read()
            if 'BEGIN RSA PRIVATE KEY' not in private_key:
                #key is in DER format
                private_key = crypto_functions.privkey_der_to_pem(private_key)
            public_key = crypto_functions.get_public_key_from_private_key(private_key)
            key_pair['public_key']=public_key
            key_pair['private_key']=private_key

        with open(certificate_params_dict['certificate_path'], 'rb') as cert_file:
            cert_raw = cert_file.read()
            if 'BEGIN CERTIFICATE' in cert_raw:
                cert = cert_raw
            else:
                cert = crypto_functions.cert_der_to_pem(cert_raw)

        return (cert, key_pair)

    def _validate_certificate_params_dict(self, certificate_params_dict):
        certificate_params_is_valid = False
        generate_new_certificate = False
        for key in certificate_params_dict:
            if key not in ['C', 'CN', 'L', 'O', 'ST', 'OU']:
                if key not in ['private_key_path', 'certificate_path']:
                    logger.error("Invalid Key is being passed in configuration!" + repr(key))
                    raise RuntimeError("Invalid Key is being passed in configuration!")
                else:
                    if os.path.exists(certificate_params_dict['private_key_path']) is False:
                        err_str = "private_key_path does not exist: {0}!".format(certificate_params_dict['private_key_path'])
                        logger.error(err_str)
                        certificate_params_is_valid = False
                        raise RuntimeError(err_str)

                    if os.path.exists(certificate_params_dict['certificate_path']) is False:
                        err_str = "certificate_path does not exist: {0}!".format(certificate_params_dict['certificate_path'])
                        logger.error(err_str)
                        certificate_params_is_valid = False
                        raise RuntimeError(err_str)

                    generate_new_certificate = False
                    certificate_params_is_valid = True

            else:
                certificate_params_is_valid = True
                generate_new_certificate = True

        return certificate_params_is_valid, generate_new_certificate


    def _validate_config(self, cert_config, general_properties, openssl_config_file_paths):

        if c_path.validate_file(openssl_config_file_paths.attestation_certificate_extensions_path) is False:
            raise ConfigError("Attestation certificate extensions path is invalid: {0}".
                              format(openssl_config_file_paths.attestation_certificate_extensions_path))

        if c_path.validate_file(openssl_config_file_paths.ca_certificate_extensions_path) is False:
            raise ConfigError("CA certificate extensions path is invalid: {0}".
                              format(openssl_config_file_paths.ca_certificate_extensions_path))

        if c_path.validate_file(openssl_config_file_paths.openssl_configfile_path) is False:
            raise ConfigError("Openssl config file path is invalid: {0}".
                              format(openssl_config_file_paths.openssl_configfile_path))

        if (general_properties.num_root_certs == 0):
            raise ConfigError("Number of root certificates cannot be set zero")

        if (general_properties.num_root_certs > 16):
            raise ConfigError("Number of root certificates cannot be more than 16")

        if (cert_config.multirootcert and
            cert_config.multirootcert.index >= general_properties.num_root_certs):
            raise ConfigError("Multirootcert index {0} must be smaller than the number of root certs {1}"
                              .format(cert_config.multirootcert.index, general_properties.num_root_certs))


    def _generate_tcg_config(self, tcg_min_str, tcg_max_str):
        tcg_min = Attribute.init(num_bits=32, string=tcg_min_str)
        tcg_max = Attribute.init(num_bits=32, string=tcg_max_str)

        tcg_str = "%.8X%.8X" % (tcg_min.value, tcg_max.value)
        tcg_cfg = "\n%s=DER:%s:%s:%s:%s:%s:%s:%s:%s" % \
                (Certificate.TCG_OID, tcg_str[0:2], tcg_str[2:4], tcg_str[4:6], tcg_str[6:8], \
                 tcg_str[8:10], tcg_str[10:12], tcg_str[12:14], tcg_str[14:16])

        return tcg_cfg

    def _generate_attestation_certificate_extensions(self,
                                                     attestation_certificate_extensions_path,
                                                     tcg_min,
                                                     tcg_max):
        v3_attest_file = c_misc.load_data_from_file(attestation_certificate_extensions_path)
        v3_attest_file_new = v3_attest_file + \
                                   self._generate_tcg_config(tcg_min, tcg_max)
        v3_attest_file_temp = utility_functions.store_data_to_temp_file(v3_attest_file_new)

        return v3_attest_file_temp

