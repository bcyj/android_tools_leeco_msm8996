#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on May 19, 2014

@author: hraghav
'''

class SecurityPolicy(object):

    def __init__(self, file_type, integrity_check, sign, encrypt):
        self.file_type = file_type
        self.integrity_check = integrity_check
        self.sign = sign
        self.encrypt = encrypt

def create_security_policy_list(image):
    security_policy_list = []
    file_type = image.image_type.file_type

    if file_type == 'elf_bin' or file_type == 'mbn_bin':
        file_type = file_type.split('_')[0]
        security_policy_list.append(SecurityPolicy('bin', False, False, True))
        security_policy_list.append(SecurityPolicy(file_type, False if file_type == 'mbn' else True, True, False))
    elif file_type == 'ewm':
        security_policy_list.append(SecurityPolicy('ewm', False, False, False))
        security_policy_list.append(SecurityPolicy('mbn', False, True, False))
    elif file_type == 'ds_ewm':
        security_policy_list.append(SecurityPolicy('ewm', True, True, True))
        security_policy_list.append(SecurityPolicy('mbn', False, True, False))
    elif file_type == 'mbn':
        security_policy_list.append(SecurityPolicy(file_type, False, True, False))
    else:
        security_policy_list.append(SecurityPolicy(file_type, True, True, True))

    return security_policy_list
