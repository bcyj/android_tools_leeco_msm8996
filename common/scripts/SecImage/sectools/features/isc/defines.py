#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Contains constants related to the isc package."""


import os

from sectools.common.utils import c_path


#------------------------------------------------------------------------------
# Common constants
#------------------------------------------------------------------------------
# Absolute path of the package
PACKAGE_PATH = os.path.dirname(os.path.realpath(__file__))

# Default config directory (relative path)
DEFAULT_CONFIG_DIR_REL = '../../..'

# Default config directory (absolute path)
DEFAULT_CONFIG_DIR = c_path.join(PACKAGE_PATH, DEFAULT_CONFIG_DIR_REL)

#------------------------------------------------------------------------------
# ImageInfo Dest Dir constants
#------------------------------------------------------------------------------
DEST_DEBUG_DIR = 'debug'
DEST_DEBUG_DIR_PARSEGEN = 'parsegen'
DEST_DEBUG_DIR_SIGNER = 'signer'
DEST_DEBUG_DIR_ENCDEC = 'encdec'

DEST_DEBUG_FILE_PARSEGEN_UNSIGNED = '_unsigned'
DEST_DEBUG_FILE_PARSEGEN_TOSIGN = '_tosign'
DEST_DEBUG_FILE_PARSEGEN_CERT_CHAIN = '_cert_chain'
DEST_DEBUG_FILE_PARSEGEN_SIGNATURE = '_signature'
DEST_DEBUG_FILE_PARSEGEN_INTEGRITY_CHECK = '_integrity_check'
DEST_DEBUG_FILE_PARSEGEN_SIGNED = '_signed'
DEST_DEBUG_FILE_PARSEGEN_ENCRYPTED = '_encrypted'

DEST_DEBUG_FILE_SIGNER_ROOT_CERT = 'root.cer'
DEST_DEBUG_FILE_SIGNER_ROOT_KEY = 'root.key'
DEST_DEBUG_FILE_SIGNER_ATTESTATION_CA_CERT = 'attestation_ca.cer'
DEST_DEBUG_FILE_SIGNER_ATTESTATION_CA_KEY = 'attestation_ca.key'
DEST_DEBUG_FILE_SIGNER_ATTESTATION_CERT = 'attestation.cer'
DEST_DEBUG_FILE_SIGNER_ATTESTATION_KEY = 'attestation.key'
DEST_DEBUG_FILE_SIGNER_SIGNATURE = 'signature.dat'
DEST_DEBUG_FILE_SIGNER_CERT_CHAIN = 'cert_chain.cer'

DEST_FILE_DECRYPTED = '_decrypted'
DEST_FILE_TO_SIGN = '_tosign'
DEST_DIR_CERT_FOLDER = 'cert'
