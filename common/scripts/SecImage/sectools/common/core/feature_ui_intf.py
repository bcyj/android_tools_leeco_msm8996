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


class AuthParams(object):

    def __init__(self):
        self.param1 = ''
        self.param2 = ''


class FeatureUIIntf(SecBase):
    __metaclass__ = abc.ABCMeta

    #--------------------------------------------------------------------------
    # Public Methods/Properties
    #--------------------------------------------------------------------------
    def __init__(self, base):
        self.base = base
        pass

    def authenticate(self, auth_params):
        return True

    def get_id(self):
        return self.base.get_id()

    def get_user_type(self):
        pass

    def get_func_container(self):
        func_container = self._func_container
        if func_container is not None:
            from sectools.common.core.func_container import FuncContainer
            if not isinstance(func_container, FuncContainer):
                raise RuntimeError('Function container implemented by "' + self.__class__.__name + '" must be of type "' + FuncContainer.__name__ + '". Function container is: ' + func_container)
        return func_container

    def get_state_machine(self):
        state_machine = self._ui_state_machine
        if state_machine is not None:
            from sectools.common.core.state_machine import StateMachine
            if not isinstance(state_machine, StateMachine):
                raise RuntimeError('State machine implemented by "' + self.__class__.__name + '" must be of type "' + StateMachine.__name__ + '". State machine is: ' + state_machine)
        return state_machine

    #--------------------------------------------------------------------------
    # Private Methods/Properties
    #--------------------------------------------------------------------------
    @abc.abstractproperty
    def _func_container(self):
        return None

    @abc.abstractproperty
    def _state_machine(self):
        return None
