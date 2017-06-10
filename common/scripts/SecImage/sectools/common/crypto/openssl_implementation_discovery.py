#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import inspect
import os
import sys

from sectools.common.utils.c_logging import logger


class OpenSSL_Service(object):
    """ """

    def which_cmd(self, filename):
        if os.name.lower()=='nt' and sys.platform is not 'cygwin':
            filename+=".exe"
        matches=[]
        path_dirs = os.environ.get("PATH").split(os.pathsep)
        path_dirs.insert(0,'.')
        path_dirs.insert(0,self.relative_path_to_packaged_openssl_windows_binary_path)

        for directory in path_dirs:
            fullpath = os.path.join(directory, filename)
            if os.path.isfile(fullpath):
                matches.append(fullpath)
        return matches

    def discover_openSSL_implementation(self):
        logger.debug("Beginning discovery of openssl implementation")
        logger.debug("Looking for openssl binary")
        openssl_binaries = self.which_cmd('openssl')
        if len(openssl_binaries)>0:
            logger.debug("Found OpenSSL binary at {0}".format(openssl_binaries[0]))
            import openssl_binary_implementation as openssl_impl
            if sys.platform!='linux2':
                openssl_impl.openssl_binary_path=self.relative_path_to_packaged_openssl_binary
            else:
                openssl_impl.openssl_binary_path=openssl_binaries[0]
            if 'OPENSSL_DIR' in os.environ:
                if sys.platform!='linux2':
                    openssl_impl.openssl_binary_path=os.path.join(os.environ['OPENSSL_DIR'],'openssl.exe')
                else:
                    openssl_impl.openssl_binary_path=os.path.join(os.environ['OPENSSL_DIR'],'openssl')
            openssl_impl.ccm_crypto_path=self.relative_path_to_packaged_ccm_binary
            openssl_impl.cbc_crypto_path=self.relative_path_to_packaged_cbc_binary
        else:
            try:
                logger.debug("Looking for M2Crypto")
                import M2Crypto as openssl_impl
            except ImportError:
                logger.debug("Could not find M2Crypto module")
                logger.critical("Could not find OpenSSL Implementation")
                raise RuntimeError("Could not find OpenSSL Implementation")
        return openssl_impl

    def __init__(self):
        self.path_to_current_file=os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
        self.relative_path_to_packaged_openssl_windows_binary_path = os.path.join(self.path_to_current_file,'..','..','..','bin','WIN')
        if sys.platform=='linux2':
            self.relative_path_to_packaged_cbc_binary = os.path.join(self.path_to_current_file,'..','..','..','bin','LIN','crypto_cbc')
        else:
            self.relative_path_to_packaged_cbc_binary = os.path.join(self.path_to_current_file,'..','..','..','bin','WIN','crypto_cbc.exe')
        if sys.platform=='linux2':
            self.relative_path_to_packaged_ccm_binary = os.path.join(self.path_to_current_file,'..','..','..','bin','LIN','crypto_ccm')
        else:
            self.relative_path_to_packaged_ccm_binary = os.path.join(self.path_to_current_file,'..','..','..','bin','WIN','crypto_ccm.exe')
        if sys.platform!='linux2':
            os.environ['OPENSSL_CONF'] = os.path.join(self.relative_path_to_packaged_openssl_windows_binary_path,'openssl.cfg')
            self.relative_path_to_packaged_openssl_binary = os.path.join(self.relative_path_to_packaged_openssl_windows_binary_path,'openssl.exe')

        self.openssl_implementation=self.discover_openSSL_implementation()
