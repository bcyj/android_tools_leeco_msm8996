#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from abc import ABCMeta, abstractmethod
from sectools.common.crypto import crypto_functions
from sectools.features.isc.hasher import Hasher
from certificate import Certificate
from sectools.common.utils.c_attribute import Attribute, BaseAttribute
from sectools.common.utils.c_logging import logger
from config_error import ConfigError

DEBUG_DISABLED = 2

class SignerOutput(object):

    def __init__(self):
        self.signature = None
        self.cert_chain = None
        self.root_cert = None
        self.attestation_ca_cert = None
        self.attestation_cert = None
        self.attestation_key = None
        self.root_cert_list = []

class BaseSigner(object):
    __metaclass__=ABCMeta

    @abstractmethod
    def sign(self, hash_to_sign, imageinfo, binary_to_sign = None, debug_dir=None):
        raise NotImplementedError()

    def validate_inputs(self, signing_attributes, general_properties):

        config_params_32bits = {
                      "tcg_min": signing_attributes.tcg_min,
                      "tcg_max": signing_attributes.tcg_max,
                      }

        for key in config_params_32bits.keys():
            if Attribute.validate(num_bits=32, string=config_params_32bits[key]) is False:
                raise ConfigError("{0}:{1} is not a valid 32 bit integer".
                                    format(key, config_params_32bits[key]))

    def _validate_attributes_with_tcg_rule(self, attest_cert_obj):
        isValid = True
        #Enforce TCG rules
        attributes_zero_list = [
                      Certificate.SIGNATTR_SW_ID,
                      Certificate.SIGNATTR_HW_ID,
                      Certificate.SIGNATTR_OEM_ID,
                      Certificate.SIGNATTR_MODEL_ID,
                      ]

        attributes_none_list = [
                      Certificate.SIGNATTR_APP_ID,
                      Certificate.SIGNATTR_CRASH_DUMP,
                      Certificate.SIGNATTR_ROT_EN,
                      ]

        if attest_cert_obj.tcg_min and attest_cert_obj.tcg_max:
            #Only enforce TCG rules currently
            for attr_name in attributes_zero_list:
                attr = attest_cert_obj.get_attr(attr_name)
                if attr.value != 0:
                    logger.debug("{0} should be 0 under TCG validation rules. Current value is {1}".\
                                 format(attr_name, attr.str))
                    isValid = False

            for attr_name in attributes_none_list:
                attr = attest_cert_obj.get_attr(attr_name)
                if attr != None:
                    logger.debug("{0} should be None under TCG validation rules. Current value is {1}".\
                                 format(attr_name, attr.str))
                    isValid = False

            attr = attest_cert_obj.get_attr(Certificate.SIGNATTR_DEBUG)
            if attr is not None and attr.value != DEBUG_DISABLED:
                logger.debug("{0} should be 2 under TCG validation rules. Current value is {1}".\
                             format(Certificate.SIGNATTR_DEBUG, attr.str))
                isValid = False

        return isValid

    def validate_tcg_from_certs(self, ca_cert_der, attest_cert_der):

        attest_cert_obj = Certificate(attest_cert_der)
        ca_cert_obj = Certificate(ca_cert_der)

        isValid = self._validate_attributes_with_tcg_rule(attest_cert_obj) and \
                    self._validate_tcg_raw(attest_cert_obj.tcg_min,
                                     attest_cert_obj.tcg_max,
                                     ca_cert_obj.tcg_min,
                                     ca_cert_obj.tcg_max)

        return isValid

    def validate_tcg_from_config(self, ca_cert_path, signing_attributes):
        ca_cert_obj = Certificate(path=ca_cert_path)

        tcg_min_str = signing_attributes.tcg_min
        tcg_max_str = signing_attributes.tcg_max

        tcg_min_attest = Attribute.init(num_bits=32, string=tcg_min_str)
        tcg_max_attest = Attribute.init(num_bits=32, string=tcg_max_str)

        return self._validate_tcg_raw(tcg_min_attest,
                                     tcg_max_attest,
                                     ca_cert_obj.tcg_min,
                                     ca_cert_obj.tcg_max)

    def _validate_tcg_raw(self, tcg_min_attest, tcg_max_attest,
                                tcg_min_ca, tcg_max_ca):
        tcg_ok = False

        if (self.config.general_properties.num_certs_in_certchain == 2) and \
            (self.config.signing.signature_format == "opendsp"):
            logger.info("2-level certificate chain is not supported for opendsp signature")
            return False

        if tcg_min_attest is not None:
            assert(isinstance(tcg_min_attest, BaseAttribute))
        if tcg_max_attest is not None:
            assert(isinstance(tcg_max_attest, BaseAttribute))
        if tcg_min_ca is not None:
            assert(isinstance(tcg_min_ca, BaseAttribute))
        if tcg_max_ca is not None:
            assert(isinstance(tcg_max_ca, BaseAttribute))

        if ((tcg_min_attest is None) and (tcg_max_attest is None) and
            (tcg_min_ca is None) and (tcg_max_ca is None)):
            # This is ok. No TCGs in attest cert.
            tcg_ok = True
            logger.debug("\nNo TCGs found in Attestation cert or CA cert. This is OK.")
        elif ((tcg_min_attest is not None) and (tcg_max_attest is not None) and
            (tcg_min_ca is None) and (tcg_max_ca is None)):
            logger.info("\nTCGs found in Attestation cert, but not in CA cert. This is invalid.")
        elif ((tcg_min_attest is None) and (tcg_max_attest is None) and
              (tcg_min_ca is not None) and (tcg_max_ca is not None)):
            logger.info("\nNo TCGs found in Attestation cert, but there are TCGs in CA cert. This is invalid.")
        elif ((tcg_min_attest is not None) and (tcg_max_attest is not None) and
              (tcg_min_ca is not None) and (tcg_max_ca is not None)):
            if (tcg_min_ca.value <= tcg_min_attest.value <=
                  tcg_max_attest.value <= tcg_max_ca.value):
                tcg_ok = True
                logger.debug("\nTCG values fall within CA constraints.")
            else:
                logger.info("\nTCG values are outside the CA constraints.")
        else:
            logger.info("\nInvalid TCG values")

        tcg_log_mesg =  "\nAttestation cert : tcg_min={0} tcg_max={1}". \
                                format(tcg_min_attest, tcg_max_attest) + \
                        "\nCA cert (allowed): tcg_min={0} tcg_max={1}\n". \
                                format(tcg_min_ca, tcg_max_ca)
        if (tcg_ok is False):
            logger.info(tcg_log_mesg)
        else:
            logger.debug(tcg_log_mesg)

        return tcg_ok

    def validate_sig(self, to_sign, signature, cert_chain_der):
        hmac_params=crypto_functions.get_hmacparams_from_certificate_chain(cert_chain_der[0])
        cert_chain_pem = []
        for cert in cert_chain_der:
            cert_chain_pem.append(crypto_functions.cert_der_to_pem(cert))

        public_key = crypto_functions.get_public_key_from_cert_chain(cert_chain_pem)
        decrypted_hash =  crypto_functions.decrypt_with_public_key(signature, public_key)

        hasher = Hasher()
        image_hash = hasher.qcom_hmac(to_sign, hmac_params)

        return image_hash == decrypted_hash

    def validate(self, image):
        if image.is_signed():
            isValid = False
            cert_chain_blob = image.cert_chain

            cert_chain_der = crypto_functions.split_certificate_blob_into_certs(cert_chain_blob)

            isValid = self.validate_tcg_from_certs(cert_chain_der[1], cert_chain_der[0]) and \
                      self.validate_sig(image.data_to_sign,
                                     image.data_signature,
                                     cert_chain_der)

            return isValid
        else:
            raise RuntimeError("Image supplied is not signed.")

    def _is_tcg_supported(self, signing_attributes):
        isSupported = False
        if signing_attributes.tcg_min and signing_attributes.tcg_max:
            isSupported = True

        return isSupported

    #This routine only supports 2-level and 3-level cert chain
    def _get_signer_output(self, signature, cert_chain_list):
        signer_output = SignerOutput()
        signer_output.attestation_cert = cert_chain_list[0]
        if len(cert_chain_list)==3:
            signer_output.attestation_ca_cert = cert_chain_list[1]
            signer_output.root_cert = cert_chain_list[2]
        elif len(cert_chain_list)==2:
            signer_output.root_cert = cert_chain_list[2]
        else:
            raise RuntimeError(
                    "Only 2-level or 3-level cert chain is supported. Number of certificates found = {0}\n".
                                format(len(cert_chain_list)))

        cert_chain=crypto_functions.create_certificate_chain(cert_chain_list)

        signer_output.signature = signature
        signer_output.cert_chain = cert_chain

        return signer_output
