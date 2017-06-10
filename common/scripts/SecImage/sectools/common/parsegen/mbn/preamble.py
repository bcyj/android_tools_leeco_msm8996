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

from sectools.common.parsegen import PAD_BYTE_1
from sectools.common.parsegen.mbn.defines import FLASH_CODE_WORD, MAGIC_NUM
from sectools.common.parsegen.mbn.header import MbnHdr80B
from sectools.common.utils.c_misc import obj_repr


class PrmblAttr(object):

    def __init__(self,
                 small_preamble,
                 flash_max_page_kb,
                 flash_min_page_kb,
                 max_size_magic_data_kb,
                 ):
        self.small_preamble = small_preamble
        self.flash_max_page = flash_max_page_kb * 1024
        self.flash_min_page = flash_min_page_kb * 1024
        self.max_size_magic_data = max_size_magic_data_kb * 1024

    def __repr__(self):
        return obj_repr(self)


class PrmblHdr(MbnHdr80B):

    def validate(self):
        MbnHdr80B.validate(self)
        if ((self.codeword != FLASH_CODE_WORD) or
                (self.magic != MAGIC_NUM) or
                (self.image_id not in PRMBL_MAGIC_NUMS.values())):
            raise RuntimeError('Preamble is invalid')


PRMBL_SIZE_10KB     = 10
PRMBL_SIZE_08KB     = 8
PRMBL_SIZE_02KB     = 2

PRMBL_ATTRS = {
                    PRMBL_SIZE_10KB : PrmblAttr(True, 8, 2, 8),
                    PRMBL_SIZE_08KB : PrmblAttr(False, 4, 2, 8),
                    PRMBL_SIZE_02KB : PrmblAttr(True, 2, 2, 8),
              }

PRMBL_NUM_PAGE_128  = 128
PRMBL_NUM_PAGE_064  = 64
PRMBL_NUM_PAGE_000  = 0

PRMBL_MAGIC_128     = 0x7D0B6577
PRMBL_MAGIC_064     = 0x7D0B5436
PRMBL_MAGIC_000     = 0x7D0B435A

PRMBL_MAGIC_NUMS = {
                        PRMBL_NUM_PAGE_128 : PRMBL_MAGIC_128,
                        PRMBL_NUM_PAGE_064 : PRMBL_MAGIC_064,
                        PRMBL_NUM_PAGE_000 : PRMBL_MAGIC_000,
                   }

PRMBL_PAGE_SIZES = [0, 2, 4]


def _add_padding(hdr, size):
    # Get a handle on preamble attrs object based on the preamble size
    prmbl_attr = PRMBL_ATTRS[size]

    # Put the padding
    prmbl = hdr.pack().ljust(prmbl_attr.flash_min_page, PAD_BYTE_1)

    # Repeat the header
    prmbl = ''.join([prmbl] * (prmbl_attr.flash_max_page / prmbl_attr.flash_min_page))

    # Add more padding if needed
    prmbl += ''.ljust(prmbl_attr.flash_min_page, PAD_BYTE_1)

    # Return the preamble
    return prmbl

def _estimate_prmbl_size(size):
    return len(_add_padding(PrmblHdr(), size))

def create(prmbl_size, num_pages):
    # Create the preamble header
    prmbl_hdr = MbnHdr80B(PAD_BYTE_1 * PrmblHdr.get_size())
    prmbl_hdr.codeword = FLASH_CODE_WORD
    prmbl_hdr.magic = MAGIC_NUM
    prmbl_hdr.image_id = PRMBL_MAGIC_NUMS[num_pages]
    prmbl_hdr = PrmblHdr(prmbl_hdr.pack())
    return _add_padding(prmbl_hdr, prmbl_size)

def remove(data, prmbl_size):
    # Check that the given preamble size is supported
    if prmbl_size not in PRMBL_ATTRS:
        raise RuntimeError('Preamble size: ' + str(prmbl_size) + ' is not supported.')

    # Estimate the size of the preamble
    prmbl_size = _estimate_prmbl_size(prmbl_size)
    return data[prmbl_size:], data[:prmbl_size]
