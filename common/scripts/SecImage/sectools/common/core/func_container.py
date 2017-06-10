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


class FuncContainer(SecBase):
    __metaclass__ = abc.ABCMeta

    #--------------------------------------------------------------------------
    # Public Methods/Properties
    #--------------------------------------------------------------------------
    def __init__(self):
        pass

    def authenticate(self, auth_params):
        return True

    #--------------------------------------------------------------------------
    # Private Methods/Properties
    #--------------------------------------------------------------------------
