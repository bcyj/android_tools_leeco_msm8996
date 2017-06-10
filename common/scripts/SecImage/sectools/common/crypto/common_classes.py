#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Feb 28, 2014

@author: vikramn
'''

class HmacParams():
    """Holds the HMAC parameters

    :param int MSM_ID: Concatenation of MSM_PART, OEM_ID, and MODEL_ID
    :param int SW_ID: SW_ID of the image

    """

    def __init__(self, MSM_ID, SW_ID):
        self.MSM_ID = MSM_ID
        self.SW_ID = SW_ID

    @property
    def msm_id_str(self):
        return "%.16X" % self.MSM_ID

    @property
    def sw_id_str(self):
        return "%.16X" % self.SW_ID
