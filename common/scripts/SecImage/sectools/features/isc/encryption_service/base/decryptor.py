#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from abc import ABCMeta, abstractmethod


class BaseDecryptor(object):

    __metaclass__=ABCMeta

    def __init__(self, encryption_parameters, debug_dir=None):
        self.encryption_parameters = encryption_parameters
        self.debug_dir = debug_dir

    @abstractmethod
    def decrypt_segment(self, encrypted_binary_segment, segment_num):
        raise NotImplementedError()
