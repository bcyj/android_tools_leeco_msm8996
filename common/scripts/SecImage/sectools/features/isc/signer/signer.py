#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from ..hasher import Hasher
import signerutils
from signer_factory import SignerFactory
from certificate import Certificate
from sectools.common.utils.c_logging import logger

class Signer(object):
    '''
    Objects of this class will discover the signing implementations available on the system.
    The objects implement the sign method that call into the discovered impl's sign function, that
    performs the actual signing.

    Usage by client:

    signer=Signer(config)
    signer_output=signer.sign(to_sign, config.signing_config.attributes)

    '''

    def __init__(self, config):
        '''
        Discover what the signing impl is going to be.
        '''
        self.config=config
        self._signer_impl= SignerFactory.getSigner(config.signing.selected_signer, config)                                                                            # perform validation at this time

    def sign(self, binary_to_sign, imageinfo, debug_dir=None):
        '''
        This function returns a SignerOutput object which has all the security assets generated
        by the signer.
        '''
        self._signer_impl.validate_inputs(imageinfo.signing_attributes, imageinfo.general_properties)

        hasher = Hasher()
        hmacParams = signerutils.get_hmac_params_from_config(imageinfo.signing_attributes)
        hash_to_sign=hasher.qcom_hmac(binary_to_sign, hmacParams)
        signer_output = self._signer_impl.sign(hash_to_sign, imageinfo, binary_to_sign, debug_dir)

        #print certificate properties
        attestation_cert_obj = Certificate(signer_output.attestation_cert)
        logger.info('\nAttestation Certificate Properties:\n' + str(attestation_cert_obj))

        return signer_output

    def validate(self, image):
        return self._signer_impl.validate(image)



