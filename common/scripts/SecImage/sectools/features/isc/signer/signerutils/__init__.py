#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import subprocess
import tempfile
from collections import defaultdict
from sectools.common.utils.c_logging import logger
import os
import sys
from sectools.common.utils import c_misc, c_path
import zipfile
from sectools.features.isc.cfgparser import HmacParams
import glob
from sectools.features.isc.signer.signer_error import ExternalSignerError

MESG_MULTIPLE_ZIP = "Multiple ZIP files exist in {0}. Please PLACE ONE ZIP FILE only\n"
MESG_SIG_NOT_FOUND = "Signature output not found in {0}\n"

class SigFiles(object):
    attest_cert_id =    '-attestation_cert.cer'
    attest_ca_cert_id = '-attestation_ca_cert.cer'
    root_cert_prefix = '-root_cert'
    signature_id = '-signature.bin'

    def __init__(self, signature = None,
                        attestation_cert = None,
                        attestation_ca_cert = None,
                        root_cert_list = None):
        self.signature = signature
        self.attestation_cert = attestation_cert
        self.attestation_ca_cert = attestation_ca_cert
        self.root_cert_list = root_cert_list

def _findMatched(list, matched_substring, returnAll = False):
    matched_list = []
    returnVal = matched_list

    for eachfile in list:
        if isinstance(eachfile, zipfile.ZipInfo):
            find = eachfile.filename.find
            matched_info = eachfile.filename
        else:
            find = eachfile.find
            matched_info = eachfile

        if find(matched_substring) >= 0:
            matched_list.append(matched_info)
            if returnAll is True:
                continue
            else:
                break

    if returnAll is False:
        if len(matched_list) == 0:
            returnVal = None
        else:
            returnVal = matched_list[0]

    return returnVal

def readFilesFromZip(zipfp,
                     signature,
                     attestation_cert,
                     attestation_ca_cert,
                     root_cert_list):
    cert_chain_list = []
    cert_chain_list.append(zipfp.read(attestation_cert))
    if attestation_ca_cert is not None:
        cert_chain_list.append(zipfp.read(attestation_ca_cert))
    for root_cert in root_cert_list:
        cert_chain_list.append(zipfp.read(root_cert))

    return [zipfp.read(signature), cert_chain_list]

def sortRootCertList(root_cert_list, root_cert_prefix):
    sorted_list = []

    if len(root_cert_list) == 1:
        sorted_list = root_cert_list
    else:
        for i in range (0, len(root_cert_list)):
            substring = "{0}{1}.cer".format(root_cert_prefix, i)
            matched_file = _findMatched(root_cert_list, substring)
            if matched_file is None:
                raise RuntimeError, "Expected root ends with %s is not in the zip package" % substring
            sorted_list.append(matched_file)

    return sorted_list

def getSigFilesFromZip(zipfilename):
    zipfp = zipfile.ZipFile(zipfilename, "r")
    unzipped_infolist = zipfp.infolist()

    sigFiles= SigFiles()
    sigFiles.attestation_cert = _findMatched(unzipped_infolist, sigFiles.attest_cert_id)
    if (sigFiles.attestation_cert == None):
        raise RuntimeError("Attestation cert is not found in {0}".format(zipfilename))

    sigFiles.attestation_ca_cert = _findMatched(unzipped_infolist, sigFiles.attest_ca_cert_id)

    sigFiles.signature = _findMatched(unzipped_infolist, sigFiles.signature_id)
    if (sigFiles.signature == None):
        raise RuntimeError("Signature is not found in {0}".format(zipfilename))

    root_cert_list = _findMatched(unzipped_infolist,
                                  sigFiles.root_cert_prefix,
                                  returnAll = True)
    if (len(root_cert_list) == 0):
        raise RuntimeError("Root cert is not found in {0}".format(zipfilename))

    sigFiles.root_cert_list = sortRootCertList(root_cert_list, sigFiles.root_cert_prefix)

    return [zipfp, sigFiles]

def getSigPackage(cert_folder):
    sig_package = None
    zipfiles = glob.glob(c_path.join(cert_folder, '*.zip'))
    if len(zipfiles) == 1:
        sig_package = zipfiles[0]
    elif len(zipfiles) > 1:
        raise ExternalSignerError(MESG_MULTIPLE_ZIP.format(cert_folder))
    return sig_package

def readSigFromZip(zipfilename):
    if os.path.isfile(zipfilename) is False:
        raise ExternalSignerError(MESG_SIG_NOT_FOUND.format(zipfilename))

    [zipfp, sigFiles] = getSigFilesFromZip(zipfilename)

    [signature, cert_chain_list] = readFilesFromZip(zipfp,
                                     sigFiles.signature,
                                     sigFiles.attestation_cert,
                                     sigFiles.attestation_ca_cert,
                                     sigFiles.root_cert_list)
    zipfp.close()

    return [signature, cert_chain_list]


def get_hmac_params_from_config(default_attributes):
    """
    :returns: A HMAC object with the HMAC parameters from the config file.
    :rtype: obj
    """
    msm_part = default_attributes.msm_part
    oem_id = default_attributes.oem_id
    model_id= default_attributes.model_id
    msm_id = msm_part + oem_id[2:] + model_id[2:]
    msm_id = int(msm_id,16)
    sw_id = default_attributes.sw_id
    sw_id = int(sw_id,16)
    return HmacParams(msm_id,sw_id)

def macro_replace(string_to_replace, macro_name, value, isMandatory=False):
    string_replaced = string_to_replace
    macro_to_search = "${%s}" % macro_name

    try:
        index_start = string_to_replace.index(macro_to_search)
        index_end = string_to_replace.index('}', index_start)
    except ValueError, e:
        if isMandatory is True:
            raise RuntimeError("macro name {0} is not found in string <{1}>".format(macro_name, string_to_replace))
    else:
        string_replaced = string_to_replace[:index_start] + str(value) + string_to_replace[index_end+1:]

    return string_replaced

class MacroReplaceArgs(object):
    def __init__(self, macro_name, value, isMandatory=False):
        self.macro_name = macro_name
        self.value = value
        self.isMandatory = isMandatory

class StringProcessor(object):
    def __init__(self, string_to_replace):
        self.string_to_replace = string_to_replace
        self.macro_replace_arg_list = []

    def addMacroReplaceArgs(self, macro_name, value, isMandatory=False):
        macro_replace_arg = MacroReplaceArgs(macro_name, value, isMandatory)
        self.macro_replace_arg_list.append(macro_replace_arg)

    def get_replaced_string(self):
        processed_string = self.string_to_replace
        for arg in self.macro_replace_arg_list:
            processed_string = macro_replace(processed_string, arg.macro_name, arg.value, arg.isMandatory)

        self.replaced_string = processed_string
        return self.replaced_string



