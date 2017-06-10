#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Feb 3, 2014

@author: yliong
'''
from base_signer import BaseSigner
from sectools.common.utils import c_path, c_misc
from signer_error import ExternalSignerError
import os
import shutil
import sectools.features.isc.signer.signerutils as signerutils

class CsmsSigner(BaseSigner):
    '''
    classdocs
    '''
    MESG_ASKUSERTOSIGN = "CSMS Signer selected. Please SIGN FILE {0} and PLACE ZIP FILE IN {1} \n"
    MESG_INVALID_SIG = "CSMS Signer selected. Signature ZIP file validation failed. Please SIGN FILE {0} and replace FILE {1} \n"

    def __init__(self, config):
        '''
        Constructor
        '''
        self.config = config

    def sign(self, hash_to_sign, imageinfo, binary_to_sign, debug_dir=None):
        return self._sign(binary_to_sign,
                          imageinfo.dest_image.to_sign,
                          imageinfo.dest_image.cert_folder)

    def _sign(self, binary_to_sign,
                    image_tosign_filename, cert_folder):

        c_path.create_dir(cert_folder)

        c_misc.store_data_to_file(image_tosign_filename, binary_to_sign)
        sig_package = signerutils.getSigPackage(cert_folder)
        if sig_package is not None:
            [signature, cert_chain_list] = signerutils.\
                                        readSigFromZip(sig_package)
            if self.validate_sig(binary_to_sign, signature,
                                        cert_chain_list) is False:
                raise ExternalSignerError(
                        self.MESG_INVALID_SIG.
                                    format(image_tosign_filename, sig_package))

        else:
            raise ExternalSignerError(
                    self.MESG_ASKUSERTOSIGN.
                                format(image_tosign_filename, cert_folder))

        signer_output = self._get_signer_output(signature, cert_chain_list)

        self._cleanup(cert_folder)

        return signer_output

    def _cleanup(self, cert_folder):
        if os.path.exists(cert_folder):
            shutil.rmtree(cert_folder)



