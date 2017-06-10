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
import zipfile

split_string_into_list_of_n_size_strings = lambda string, n:[string[i:i+n] for i in range(0, len(string), n)]
xor = lambda x,y : x^y
join_ints_list_to_hex_string = lambda ints_list : "".join([("0x%0.2X" % x)[2:] for x in ints_list])

if "check_output" not in dir( subprocess ): # duck punch it in!
    def f(*popenargs, **kwargs):
        if 'stdout' in kwargs:
            raise ValueError('stdout argument not allowed, it will be overridden.')
        process = subprocess.Popen(stdout=subprocess.PIPE, *popenargs, **kwargs)
        output, unused_err = process.communicate()
        retcode = process.poll()
        if retcode:
            cmd = kwargs.get("args")
            if cmd is None:
                cmd = popenargs[0]
            raise subprocess.CalledProcessError(retcode, ' '.join(cmd))
        return output
    subprocess.check_output = f

class LogWrappedFunction(object):
    def __init__(self, function):
        self.function = function

    def logAndCall(self, *arguments, **namedArguments):
        logger.debug("Calling %s with arguments %s and named arguments %s" %\
                      (self.function.func_name, arguments, namedArguments))
        return self.function.__call__(*arguments, **namedArguments)

def logwrap(function):
    return LogWrappedFunction(function).logAndCall


def system_command_logged(command_list,stderr_to_temp = False, shell=False):
    tmp_stderr_file = None

    if stderr_to_temp == True:
        tmp_stderr_file=tempfile.NamedTemporaryFile(delete=False)
        logger.debug("Creating temp file for stderr of external command: "+ tmp_stderr_file.name)
    logger.debug("Command List: " + repr(command_list))
    logger.debug("Executing System command:" + " ".join(command_list))
    return subprocess.check_output(command_list, stderr=tmp_stderr_file, shell=shell)

def store_data_to_temp_file(data):
    temp_file = tempfile.NamedTemporaryFile(delete=False)
    logger.debug("Writing data: ")#+ repr(data))
    logger.debug("To temporary file: " + temp_file.name)
    temp_file.write(data)
    temp_file.close()
    return temp_file.name

def store_data_to_file(file_name, data):
    file_handle = open(file_name,"wb")
    logger.debug("Writing data: ")#+ repr(data))
    logger.debug("To file: " + file_handle.name)
    file_handle.write(data)
    file_handle.close()
    return file_handle.name


def normalize_param_list_into_dict(certificate_subject_list):
    """ Converts list of certificate params to dict
    input:
    certificate_subject_list: List of certificate param strings e.g.
    ['C=US', 'ST=California', 'L=San Diego', 'OU=General Use Test Key (for testing only)', 'OU=CDMA Technologies', 'O=QUALCOMM', 'CN=QCT Root CA 1']

    output:
    certificate_subject_dictionary: Dictionary of certificate params
    """
    certificate_subject_list_normalized=[]
    for item in certificate_subject_list:
        temp_list=item.strip().split('=')
        temp_list[0]=temp_list[0].strip()
        temp_list[1]=temp_list[1].strip()
        certificate_subject_list_normalized.append(temp_list)

    dic=defaultdict(list)
    for item in certificate_subject_list_normalized:
        dic[item[0]].append(item[1])
    certificate_subject_dictionary={}
    for k in dic:
        if len(dic[k]) == 1:
            certificate_subject_dictionary[k]=dic[k][0]
        else:
            certificate_subject_dictionary[k]=dic[k]
    if 'OU' in certificate_subject_dictionary.keys() and type(certificate_subject_dictionary['OU'])==list:
        certificate_subject_dictionary['OU'].sort()
    return certificate_subject_dictionary

def _which_cmd(self, filename):
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

def get_data_from_file(file_path):
    with open(file_path,'rb') as f:
        data=f.read()
    return data



