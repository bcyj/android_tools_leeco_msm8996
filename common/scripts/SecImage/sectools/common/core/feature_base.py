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


class FeatureBase_Abs(SecBase):
    __metaclass__ = abc.ABCMeta

    #--------------------------------------------------------------------------
    # Public Methods/Properties
    #--------------------------------------------------------------------------
    def __init__(self):
        pass

    def get_feature_id(self):
        feature_id = self._feature_id
        if not isinstance(feature_id, str):
            raise RuntimeError('Feature ID implemented by "' + self.__class__.__name + '" must be of type "string". Feature id is ' + feature_id)
        elif not feature_id:
            raise RuntimeError('Feature ID implemented by "' + self.__class__.__name + '" must not be empty')
        return feature_id

    def get_ui_interface(self):
        ui_interface = self._ui_interface
        if ui_interface is not None:
            from sectools.common.core.feature_ui_intf import FeatureUIIntf
            if not isinstance(ui_interface, FeatureUIIntf):
                raise RuntimeError('UI Interface implemented by "' + self.__class__.__name + '" must be of type "' + FeatureUIIntf.__name__ + '". UI Interface is: ' + ui_interface)
        return ui_interface

    #--------------------------------------------------------------------------
    # Private Methods/Properties
    #--------------------------------------------------------------------------
    @abc.abstractproperty
    def _ui_interface(self):
        return None

    @abc.abstractproperty
    def _feature_id(self):
        return None

