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


from sectools.common.utils.struct_base import StructBase


class MbnHdr40B(StructBase):

    def _unpack_data_list(self, unpacked):
        self.image_id = unpacked[0]
        self.flash_parti_ver = unpacked[1]
        self.image_src = unpacked[2]
        self.image_dest_ptr = unpacked[3]
        self.image_size = unpacked[4]
        self.code_size = unpacked[5]
        self.sig_ptr = unpacked[6]
        self.sig_size = unpacked[7]
        self.cert_chain_ptr = unpacked[8]
        self.cert_chain_size = unpacked[9]

    def _pack_data_list(self):
        return [self.image_id,
                self.flash_parti_ver,
                self.image_src,
                self.image_dest_ptr,
                self.image_size,
                self.code_size,
                self.sig_ptr,
                self.sig_size,
                self.cert_chain_ptr,
                self.cert_chain_size]

    @classmethod
    def get_format(cls):
        return ('I'*10)

    def validate(self):
        pass


class MbnHdr80B(StructBase):

    def _unpack_data_list(self, unpacked):
        self.codeword = unpacked[0]
        self.magic = unpacked[1]
        self.image_id = unpacked[2]
        self.reserved_1 = unpacked[3]
        self.reserved_2 = unpacked[4]
        self.image_src = unpacked[5]
        self.image_dest_ptr = unpacked[6]
        self.image_size = unpacked[7]
        self.code_size = unpacked[8]
        self.sig_ptr = unpacked[9]
        self.sig_size = unpacked[10]
        self.cert_chain_ptr = unpacked[11]
        self.cert_chain_size = unpacked[12]
        self.reserved_3 = unpacked[13]
        self.reserved_4 = unpacked[14]
        self.reserved_5 = unpacked[15]
        self.reserved_6 = unpacked[16]
        self.reserved_7 = unpacked[17]
        self.reserved_8 = unpacked[18]
        self.reserved_9 = unpacked[19]

    def _pack_data_list(self):
        return [self.codeword,
                self.magic,
                self.image_id,
                self.reserved_1,
                self.reserved_2,
                self.image_src,
                self.image_dest_ptr,
                self.image_size,
                self.code_size,
                self.sig_ptr,
                self.sig_size,
                self.cert_chain_ptr,
                self.cert_chain_size,
                self.reserved_3,
                self.reserved_4,
                self.reserved_5,
                self.reserved_6,
                self.reserved_7,
                self.reserved_8,
                self.reserved_9]

    @classmethod
    def get_format(cls):
        return ('I'*20)

    def validate(self):
        pass


MBN_HDR_80B_SIZE    = MbnHdr80B.get_size()
MBN_HDR_40B_SIZE    = MbnHdr40B.get_size()

"""
Map of: MbnHeaderSize -> Mbn Header Class

Dictionary containing a mapping of the class to use for parsing the mbn header
based on the header type specified
"""
MBN_HDRS =  {
                MBN_HDR_80B_SIZE : MbnHdr80B,
                MBN_HDR_40B_SIZE : MbnHdr40B,
            }

def extract_header(data, header_size):
    if header_size not in MBN_HDRS.keys():
        raise RuntimeError('Unexpected header size provided: ' + str(header_size) + '\n'
                           '    ' + 'Supported header types: ' + str(MBN_HDRS.keys()))
    header = MBN_HDRS[header_size](data)
    data = data[header_size:]
    return data, header
