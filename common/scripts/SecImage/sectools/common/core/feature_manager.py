#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Mar 20, 2014

@author: hraghav
'''

import abc

from sec_base import SecBase


class FeatureManager(SecBase):
    __metaclass__ = abc.ABCMeta

    #--------------------------------------------------------------------------
    # Public Methods/Properties
    #--------------------------------------------------------------------------
    def __init__(self):
        self._available_features = []
        pass

    def get_available_features(self):
        return self._available_features

    def register(self, feature):
        self._available_features += feature

    def get_ui_feature_manager(self):
        pass

