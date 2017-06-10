#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Mar 31, 2014

@author: hraghav
'''

from sectools.common.parsegen import PAD_BYTE_0


def remove_pad(data):
    raise RuntimeError('Removing of pad logic is not known')

def insert_pad(data, page_size, num_pages, min_size):
    pad_len = page_size * num_pages
    if pad_len > 0:
        import math
        pad_len = int((math.ceil(len(data) / float(pad_len)))) * pad_len
    data = data.ljust(max(min_size, pad_len), PAD_BYTE_0)
    return data
