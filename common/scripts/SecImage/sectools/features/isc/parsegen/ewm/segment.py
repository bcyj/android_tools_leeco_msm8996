#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
'''
Created on May 2, 2014

@author: hraghav
'''

from sectools.common.parsegen import elf
from sectools.common.parsegen.elf.segment.format import Elf32_Phdr


# ELF default pheader values
ELF_DEFAULT_PHDR_TYPE       = 1
ELF_DEFAULT_PHDR_OFFSET     = 0x1000
ELF_DEFAULT_PHDR_FLAGS      = 7
ELF_DEFAULT_PHDR_ALIGN      = 0
ELF_RELOCATABLE_PHDR_FLAGS  = 0x8000007
ELF_RELOCATABLE_PHDR_ALIGN  = 0x100000


class EwmElf32_Phdr(Elf32_Phdr):

    def __init__(self, image_entry, image_size, relocatable):
        Elf32_Phdr.__init__(self)
        self.p_type = elf.segment.type.PT_LOAD
        self.p_offset = ELF_DEFAULT_PHDR_OFFSET
        self.p_vaddr = image_entry
        self.p_paddr = image_entry
        self.p_filesz = image_size
        self.p_memsz = image_size
        self.p_flags = ELF_RELOCATABLE_PHDR_FLAGS if relocatable else ELF_DEFAULT_PHDR_FLAGS
        self.p_align = ELF_RELOCATABLE_PHDR_ALIGN if relocatable else ELF_DEFAULT_PHDR_ALIGN


