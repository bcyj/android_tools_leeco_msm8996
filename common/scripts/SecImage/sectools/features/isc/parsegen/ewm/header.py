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
from sectools.common.parsegen.elf.header.format import Elf32_Ehdr


# ELF default header values
ELF_DEFAULT_HDR_MACHINE     = 0xa4
ELF_DEFAULT_HDR_PHOFF       = 52
ELF_DEFAULT_HDR_SHOFF       = 0
ELF_DEFAULT_HDR_FLAGS       = 3
ELF_DEFAULT_HDR_EHSIZE      = 52
ELF_DEFAULT_HDR_PHENTSIZE   = 32
ELF_DEFAULT_HDR_PHNUM       = 1
ELF_DEFAULT_HDR_SHENTSIZE   = 0
ELF_DEFAULT_HDR_SHNUM       = 0
ELF_DEFAULT_HDR_SHSTRNDX    = 0


class EwmElf32_Ehdr(Elf32_Ehdr):

    def __init__(self, image_entry):
        Elf32_Ehdr.__init__(self)
        self.e_ident_mag = elf.header.ident.ELFMAG
        self.e_ident_class = elf.header.ident.ELFCLASS32
        self.e_ident_data = elf.header.ident.ELFDATA2LSB
        self.e_ident_ver = elf.header.ident.ELFVERCURRENT
        self.e_type = elf.header.type.ET_EXEC
        self.e_machine = ELF_DEFAULT_HDR_MACHINE
        self.e_version = elf.header.version.EV_CURRENT
        self.e_entry = image_entry
        self.e_phoff = ELF_DEFAULT_HDR_PHOFF
        self.e_shoff = ELF_DEFAULT_HDR_SHOFF
        self.e_flags = ELF_DEFAULT_HDR_FLAGS
        self.e_ehsize = ELF_DEFAULT_HDR_EHSIZE
        self.e_phentsize = ELF_DEFAULT_HDR_PHENTSIZE
        self.e_phnum = ELF_DEFAULT_HDR_PHNUM
        self.e_shentsize = ELF_DEFAULT_HDR_SHENTSIZE
        self.e_shnum = ELF_DEFAULT_HDR_SHNUM
        self.e_shstrndx = ELF_DEFAULT_HDR_SHSTRNDX


