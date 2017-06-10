#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
from sectools.features.isc.signer.cass.base_keystore import BaseKeystore
from sectools.common.utils import c_path

class TrustKeystore(BaseKeystore):
    MESG_PROMPT_PASSWORD = '\nPlease enter the password for the trust keystore (server authentication):'

    def __init__(self, filepath=None, password="", keystoreType="JKS"):
        BaseKeystore.__init__(self, password)
        self._file = c_path.normalize(filepath)
        self._type = keystoreType

    @property
    def file(self):
        return self._file

    @property
    def type(self):
        return self._type


