#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import os
from sectools.common.utils.c_logging import logger
from sectools.common.utils import c_misc, c_path
from sectools.features.isc.signer.cass.identity_keystore import IdentityKeystoreFactory
from sectools.features.isc.signer.cass.trust_keystore import TrustKeystore
from sectools.features.isc.signer.signer_error import ExternalSignerError
from sectools.common.utils.c_process import CoreSubprocess
import shutil
CASSCONNECTOR_DIR = os.path.dirname(os.path.realpath(__file__))

class CassConnector(object):
    CASS_CLIENT_REFAPP_DIR=os.path.join(CASSCONNECTOR_DIR, 'cass-client-refapp')
    CLIENT_REFAPP_JAR =os.path.join('bin', 'java', 'cass-client-refapp.jar')
    CASS_CLIENT_REFAPP_CONFIG_DIR=os.path.join(CASS_CLIENT_REFAPP_DIR, 'config')
    CASS_CLIENT_REFAPP_INPUT_DIR=os.path.join(CASS_CLIENT_REFAPP_DIR, 'input')
    CASS_CLIENT_REFAPP_OUTPUT_DIR=os.path.join(CASS_CLIENT_REFAPP_DIR, 'output')
    SCHEMA_XSD_FILEPATH=os.path.join(CASS_CLIENT_REFAPP_CONFIG_DIR, 'signature_package.xsd')
    SIGNATUREPACKPAGE_RELPATH = os.path.join("signingpackage", "signingpackage.xml")

    DEFAULT_HOST_VALIDATION_MODE = "STAGE_PROD"
    TIMEOUT = 300

    MESG_TIMEOUT = "Connection to CASS server timeout after {0} seconds"

    def __init__(self, cass_signer_attributes):
        self.cass_signer_attributes = cass_signer_attributes
        self.identity_keystore = IdentityKeystoreFactory.getKeyStore(
                                    self.cass_signer_attributes.user_identity)

        self.trust_keystore = None
        self.host_validation_mode = self.DEFAULT_HOST_VALIDATION_MODE
        if self.cass_signer_attributes.server:
            self.trust_keystore =  TrustKeystore(
                    filepath=self.cass_signer_attributes.server.trust_keystore,
                    password=self.cass_signer_attributes.server.trust_keystore_password,
                    keystoreType=self.cass_signer_attributes.server.trust_keystore_type)
            self.host_validation_mode = self.cass_signer_attributes.server.host_validation_mode

    @property
    def host_validation_mode(self):
        return self._host_validation_mode

    @host_validation_mode.setter
    def host_validation_mode(self, value):
        if value:
            self._host_validation_mode = value
        else:
            self._host_validation_mode = self.DEFAULT_HOST_VALIDATION_MODE


    def sign(self, signingpackage_fname, outputdir):
        signaturepackage_binary = None
        cmds = self._getCmds(signingpackage_fname, outputdir)
        cass_server = self.cass_signer_attributes.server.host if self.cass_signer_attributes.server else "default CASS server"
        logger.info("Connecting to {0}".format(cass_server))
        output = self._executeCmds(cmds)
        logger.debug(output)
        signaturepackage_filepath = os.path.join(outputdir, self.SIGNATUREPACKPAGE_RELPATH)
        if os.path.isfile(signaturepackage_filepath):
            logger.info("Signature package retrieved from server.")
            signaturepackage_binary = c_misc.load_data_from_file(signaturepackage_filepath)

            #clean up
            path, filename = os.path.split(signaturepackage_filepath)
            shutil.rmtree(path)
        return signaturepackage_binary

    def _formatpath(self, path):
        return c_path.normalize(path)

    def _getCmds(self, signingpackage_fname, outputdir):
        if os.path.exists(outputdir) is None:
            os.makedirs(outputdir)

        signingpackage_file = self._formatpath(signingpackage_fname)
        cmds = ["java"]
        if self.cass_signer_attributes.server:
            cmds.append("-Dcass_server.host={0}".format(self.cass_signer_attributes.server.host))
            cmds.append("-Dcass_server.sslport={0}".format(self.cass_signer_attributes.server.port))
            cmds.append("-Dtrust_anchor.file={0}".format(self.trust_keystore.file))
            cmds.append("-Dtrust_anchor.password={0}".format(self.trust_keystore.password))
            cmds.append("-Dtrust_anchor.keystoretype={0}".format(self.trust_keystore.type))
            cmds.append("-Dcn_verify.mode={0}".format(self.host_validation_mode))
        cmds.append("-Dsig_authority.file={0}".format(self.identity_keystore.file))
        cmds.append("-Dsig_authority.password={0}".format(self.identity_keystore.password))
        cmds.append("-Dsig_authority.keystoretype={0}".format(self.identity_keystore.type))
        if self.identity_keystore.token_driver_home:
            cmds.append("-Detoken_driver_home={0}".format(self.identity_keystore.token_driver_home))
        cmds.append("-Doutput.location={0}".format(outputdir))
        cmds.append("-Dschema.location={0}".format(self._formatpath(self.SCHEMA_XSD_FILEPATH)))

        cmds.extend(['-jar', self.CLIENT_REFAPP_JAR, signingpackage_file])

        # only enable -v if debug is enabled
        if logger.verbosity < logger.INFO:
            cmds.append('-v')

        return cmds

    def _executeCmds(self, cmds):
        # Need to run in cass-client-refapp directory
        returnValue, returnError, f_timeout, f_retcode, f_output = CassCoreSubprocess.executeCommand(
                            launchCommand = cmds,
                            retcode = 0,
                            timeout = self.TIMEOUT,
                            successString = "SUCCESS",
                            workingDir = self.CASS_CLIENT_REFAPP_DIR)

        if returnValue is False:
            if f_timeout is True:
                raise ExternalSignerError(self.MESG_TIMEOUT.format(self.TIMEOUT))
            else:
                raise ExternalSignerError(f_output)
        return f_output

class CassCoreSubprocess(CoreSubprocess):

    def _maskPassword(self, string_to_replace):
        string_replaced = string_to_replace
        keyword_to_search = "password="

        try:
            index_start = string_to_replace.index(keyword_to_search)+len(keyword_to_search)
            index_end = len(string_to_replace)
        except ValueError:
            #It is ok if password is not found
            pass
        else:
            value = '*' * (index_end-index_start)
            string_replaced = string_to_replace[:index_start] + str(value) + string_to_replace[index_end+1:]

        return string_replaced

    # This is a hook for caller to format the command
    # line string for printing
    def formatLaunchCommandForPrinting(self, cmd):
        formatted_cmd = []

        for eachArg in cmd:
            eachArg = self._maskPassword(eachArg)
            formatted_cmd.append(eachArg)

        return formatted_cmd

    # Suppress printing Finish command
    def printFinish(self, cmd):
        pass
