#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Apr 2, 2014

@author: hraghav
'''

from sectools.common.parsegen import hex_addr
from sectools.common.parsegen.elf.header.ident import ELFCLASS32, ELFCLASS64,\
    ELFCLASS_DESCRIPTION, ELFMAG_STRING, ELFMAG_DESCRIPTION, ELFCLASS_STRING,\
    ELFDATA_STRING, ELFDATA_DESCRIPTION, ELFVER_STRING, ELFVER_DESCRIPTION,\
    ELFOSABI_STRING, ELFOSABI_DESCRIPTION, ELFABIVER_STRING,\
    ELFABIVER_DESCRIPTION
from sectools.common.parsegen.elf.header.machine import EM_STRING, EM_DESCRIPTION
from sectools.common.parsegen.elf.header.type import ET_STRING, ET_DESCRIPTION
from sectools.common.parsegen.elf.header.version import EV_STRING, EV_DESCRIPTION
from sectools.common.utils.c_misc import properties_repr
from sectools.common.utils.struct_base import StructBase


class Elf_Ehdr_Common(StructBase):
    """ELF Common Header Class"""

    def _unpack_data_list(self, unpacked):
        self.e_ident_mag = unpacked[0]
        self.e_ident_class = unpacked[1]
        self.e_ident_data = unpacked[2]
        self.e_ident_ver = unpacked[3]
        self.e_ident_osabi = unpacked[4]
        self.e_ident_abiver = unpacked[5]
        self.e_ident_unused = unpacked[6]
        self.e_type = unpacked[7]
        self.e_machine = unpacked[8]
        self.e_version = unpacked[9]

    def _pack_data_list(self):
        return [self.e_ident_mag,
                self.e_ident_class,
                self.e_ident_data,
                self.e_ident_ver,
                self.e_ident_osabi,
                self.e_ident_abiver,
                self.e_ident_unused,
                self.e_type,
                self.e_machine,
                self.e_version,]

    @classmethod
    def get_format(self):
        return '4sBBBBB7sHHI'

    def validate(self):
        # Check that the magic is correct
        if self.e_ident_mag not in ELFMAG_DESCRIPTION.keys():
            raise RuntimeError('Invalid magic in the ELF file: ' + self.e_ident_mag + '\n'
                               'This is not an elf file.')

        # Check the class is correct
        if self.e_ident_class not in ELF_HDRS.keys():
            raise RuntimeError('Invalid class in the ELF file: ' + self.e_ident_class)

    def _repr_properties(self):
        properties = [
                      (ELFMAG_STRING, ELFMAG_DESCRIPTION, self.e_ident_mag),
                      (ELFCLASS_STRING, ELFCLASS_DESCRIPTION, self.e_ident_class),
                      (ELFDATA_STRING, ELFDATA_DESCRIPTION, self.e_ident_data),
                      (ELFVER_STRING, ELFVER_DESCRIPTION, self.e_ident_ver),
                      (ELFOSABI_STRING, ELFOSABI_DESCRIPTION, self.e_ident_osabi),
                      (ELFABIVER_STRING, ELFABIVER_DESCRIPTION, self.e_ident_abiver),
                      (ET_STRING, ET_DESCRIPTION, self.e_type),
                      (EM_STRING, EM_DESCRIPTION, self.e_machine),
                      (EV_STRING, EV_DESCRIPTION, self.e_version),
                     ]
        return [(attr, desc.get(val, val)) for attr, desc, val in properties]

    def __repr__(self):
        return properties_repr(self._repr_properties())


class Elf32_Ehdr(Elf_Ehdr_Common):
    """ELF 32bit Header Class"""

    def _unpack_data_list(self, unpacked_data):
        Elf_Ehdr_Common._unpack_data_list(self, unpacked_data)
        self.e_entry = unpacked_data[10]
        self.e_phoff = unpacked_data[11]
        self.e_shoff = unpacked_data[12]
        self.e_flags = unpacked_data[13]
        self.e_ehsize = unpacked_data[14]
        self.e_phentsize = unpacked_data[15]
        self.e_phnum = unpacked_data[16]
        self.e_shentsize = unpacked_data[17]
        self.e_shnum = unpacked_data[18]
        self.e_shstrndx = unpacked_data[19]

    def _pack_data_list(self):
        return (Elf_Ehdr_Common._pack_data_list(self) +
                [self.e_entry,
                 self.e_phoff,
                 self.e_shoff,
                 self.e_flags,
                 self.e_ehsize,
                 self.e_phentsize,
                 self.e_phnum,
                 self.e_shentsize,
                 self.e_shnum,
                 self.e_shstrndx,])

    @classmethod
    def get_format(cls):
        return Elf_Ehdr_Common.get_format() + 'IIIIHHHHHH'

    def _repr_properties(self):
        properties = [
                        ('Entry address', hex_addr(self.e_entry)),
                        ('Program headers offset', hex_addr(self.e_phoff)),
                        ('Section headers offset', hex_addr(self.e_shoff)),
                        ('Flags', hex_addr(self.e_flags)),
                        ('ELF header size', self.e_ehsize),
                        ('Program headers size', self.e_phentsize),
                        ('Number of program headers', self.e_phnum),
                        ('Section headers size', self.e_shentsize),
                        ('Number of section headers', self.e_shnum),
                        ('String table section index', self.e_shstrndx),
                     ]
        return Elf_Ehdr_Common._repr_properties(self) + properties

class Elf64_Ehdr(Elf32_Ehdr):
    """ELF 64bit Header Class"""

    @classmethod
    def get_format(cls):
        return Elf_Ehdr_Common.get_format() + 'QQQIHHHHHH'


ELF_HDRS = {
                ELFCLASS32 : Elf32_Ehdr,
                ELFCLASS64 : Elf64_Ehdr,
           }


def extract_ehdr(data):
    # Ensure data is of common elf header length
    if len(data) < Elf_Ehdr_Common.get_size():
        raise RuntimeError('Data length "' + str(len(data)) + '" is less than common Elf Header length "' + Elf_Ehdr_Common.get_size() + '"')

    # Get the appropriate elf header class
    header = Elf_Ehdr_Common(data)
    header_class = ELF_HDRS.get(header.e_ident_class, None)
    if header_class is None:
        supported_classes = [str(key) + '-' + str(ELFCLASS_DESCRIPTION[key]) for key in ELF_HDRS.keys()]
        raise RuntimeError('File contains invalid class: ' + str(header.e_ident_class) + '\n'
                           '    ' + 'Supported classes are: ' + str(supported_classes))

    # Ensure data is of elf header length
    if len(data) < header_class.get_size():
        raise RuntimeError('Data length "' + str(len(data)) + '" is less than ' +
                           str(ELFCLASS_DESCRIPTION[header.e_ident_class]) + ' Elf Header length "' + header_class.get_size() + '"')

    # Extract the header
    header = header_class(data)
    return header

