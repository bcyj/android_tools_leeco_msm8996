#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Apr 4, 2014

@author: hraghav
'''

from sectools.common.parsegen import hex_addr
from sectools.common.parsegen.elf.header.format import Elf32_Ehdr, Elf64_Ehdr
from sectools.common.parsegen.elf.header.ident import ELFCLASS32, ELFCLASS64,\
    ELFCLASS_DESCRIPTION
from sectools.common.parsegen.elf.segment.defines import PO_STRING, PV_STRING,\
    PP_STRING, PF_STRING, PM_STRING, PA_STRING, PFL_STRING
from sectools.common.parsegen.elf.segment.flags import PF_PERM_VALUE,\
    PF_OS_POOL_INDEX_VALUE, PF_OS_SEGMENT_TYPE_VALUE, PF_OS_ACCESS_TYPE_VALUE,\
    PF_OS_PAGE_MODE_VALUE, PF_PERM_STRING, PF_PERM_DESCRIPTION,\
    PF_OS_POOL_INDEX_STRING, PF_OS_POOL_INDEX_DESCRIPTION,\
    PF_OS_SEGMENT_TYPE_STRING, PF_OS_SEGMENT_TYPE_DESCRIPTION,\
    PF_OS_ACCESS_TYPE_STRING, PF_OS_ACCESS_TYPE_DESCRIPTION,\
    PF_OS_PAGE_MODE_STRING, PF_OS_PAGE_MODE_DESCRIPTION
from sectools.common.parsegen.elf.segment.type import PT_STRING, PT_DESCRIPTION
from sectools.common.utils.c_misc import properties_repr, TablePrinter
from sectools.common.utils.struct_base import StructBase


class Elf32_Phdr(StructBase):
    """ELF 32bit Program Header Class"""

    def _unpack_data_list(self, unpacked):
        self.p_type = unpacked[0]
        self.p_offset = unpacked[1]
        self.p_vaddr = unpacked[2]
        self.p_paddr = unpacked[3]
        self.p_filesz = unpacked[4]
        self.p_memsz = unpacked[5]
        self.p_flags = unpacked[6]
        self.p_align = unpacked[7]

    def _pack_data_list(self):
        return [self.p_type,
                self.p_offset,
                self.p_vaddr,
                self.p_paddr,
                self.p_filesz,
                self.p_memsz,
                self.p_flags,
                self.p_align]

    @classmethod
    def get_format(cls):
        return 'I'*8

    @property
    def f_perm(self):
        return PF_PERM_VALUE(self.p_flags)

    @property
    def f_os_pool_index(self):
        return PF_OS_POOL_INDEX_VALUE(self.p_flags)

    @property
    def f_os_segment_type(self):
        return PF_OS_SEGMENT_TYPE_VALUE(self.p_flags)

    @property
    def f_os_access_type(self):
        return PF_OS_ACCESS_TYPE_VALUE(self.p_flags)

    @property
    def f_os_page_mode(self):
        return PF_OS_PAGE_MODE_VALUE(self.p_flags)

    def _repr_properties(self):
        properties = [
                      (PT_STRING, PT_DESCRIPTION.get(self.p_type, self.p_type)),
                      (PO_STRING, hex_addr(self.p_offset)),
                      (PV_STRING, hex_addr(self.p_vaddr)),
                      (PP_STRING, hex_addr(self.p_paddr)),
                      (PF_STRING, hex_addr(self.p_filesz)),
                      (PM_STRING, hex_addr(self.p_memsz)),
                      (PA_STRING, hex(self.p_align)),
                      (PFL_STRING, hex_addr(self.p_flags)),
                      (PF_PERM_STRING, PF_PERM_DESCRIPTION.get(self.f_perm, self.f_perm)),
                      (PF_OS_POOL_INDEX_STRING, PF_OS_POOL_INDEX_DESCRIPTION.get(self.f_os_pool_index, self.f_os_pool_index)),
                      (PF_OS_SEGMENT_TYPE_STRING, PF_OS_SEGMENT_TYPE_DESCRIPTION.get(self.f_os_segment_type, self.f_os_segment_type)),
                      (PF_OS_ACCESS_TYPE_STRING, PF_OS_ACCESS_TYPE_DESCRIPTION.get(self.f_os_access_type, self.f_os_access_type)),
                      (PF_OS_PAGE_MODE_STRING, PF_OS_PAGE_MODE_DESCRIPTION.get(self.f_os_page_mode, self.f_os_page_mode)),
                     ]
        return [(attr, val) for attr, val in properties]

    def __repr__(self):
        return properties_repr(self._repr_properties())


class Elf64_Phdr(Elf32_Phdr):
    """ELF 64bit Program Header Class"""

    def _unpack_data_list(self, unpacked):
        self.p_type = unpacked[0]
        self.p_flags = unpacked[1]
        self.p_offset = unpacked[2]
        self.p_vaddr = unpacked[3]
        self.p_paddr = unpacked[4]
        self.p_filesz = unpacked[5]
        self.p_memsz = unpacked[6]
        self.p_align = unpacked[7]

    def _pack_data_list(self):
        return [self.p_type,
                self.p_flags,
                self.p_offset,
                self.p_vaddr,
                self.p_paddr,
                self.p_filesz,
                self.p_memsz,
                self.p_align]

    @classmethod
    def get_format(cls):
        return ('I'*2) + ('Q' * 6)


ELF_PHDRS = {
                ELFCLASS32 : Elf32_Phdr,
                ELFCLASS64 : Elf64_Phdr,
            }


def get_phdr_class(elf_header):
    # Get the appropriate elf program header class
    header_class = ELF_PHDRS.get(elf_header.e_ident_class, None)
    if header_class is None:
        supported_classes = [str(key) + '-' + str(ELFCLASS_DESCRIPTION[key]) for key in ELF_PHDRS.keys()]
        raise RuntimeError('File contains unsupported program header class: ' + str(elf_header.e_ident_class) + '\n'
                           '    ' + 'Supported classes are: ' + str(supported_classes))
    return header_class

def extract_phdrs(data, elf_header):
    assert isinstance(elf_header, (Elf32_Ehdr, Elf64_Ehdr))
    phdrs = []

    header_class = get_phdr_class(elf_header)
    for idx in range(elf_header.e_phnum):
        offset = elf_header.e_phoff + idx*elf_header.e_phentsize
        program_header_data = data[offset : offset + elf_header.e_phentsize]

        # Ensure data is of elf program header length
        if len(program_header_data) < header_class.get_size():
            raise RuntimeError('Data length "' + str(len(program_header_data)) + '" is less than ' +
                               str(ELFCLASS_DESCRIPTION[elf_header.e_ident_class]) + ' Elf Program Header length "' + header_class.get_size() + '"')

        # Create the program header class
        phdrs.append(header_class(program_header_data))

    return phdrs

def repr_phdrs(phdrs):
    retval = TablePrinter()

    COLUMN_SNO      = 0
    COLUMN_TYPE     = 1
    COLUMN_OFF      = 2
    COLUMN_VADDR    = 3
    COLUMN_PADDR    = 4
    COLUMN_FSIZE    = 5
    COLUMN_MSIZE    = 6
    COLUMN_FLAGS    = 7
    COLUMN_ALIGN    = 8

    retval.insert_data(0, COLUMN_SNO, 'S.No')
    retval.insert_data(0, COLUMN_TYPE, 'Type')
    retval.insert_data(0, COLUMN_OFF, 'Offset')
    retval.insert_data(0, COLUMN_VADDR, 'VirtAddr')
    retval.insert_data(0, COLUMN_PADDR, 'PhysAddr')
    retval.insert_data(0, COLUMN_FSIZE, 'FileSize')
    retval.insert_data(0, COLUMN_MSIZE, 'MemSize')
    retval.insert_data(0, COLUMN_FLAGS, 'Flags')
    retval.insert_data(0, COLUMN_ALIGN, 'Align')

    stats = [0] * (COLUMN_ALIGN + 1)
    for idx, phdr in enumerate(phdrs):
        stats[COLUMN_OFF] = phdr.p_offset if phdr.p_offset > stats[COLUMN_OFF] else stats[COLUMN_OFF]
        stats[COLUMN_VADDR] = phdr.p_vaddr if phdr.p_vaddr > stats[COLUMN_VADDR] else stats[COLUMN_VADDR]
        stats[COLUMN_PADDR] = phdr.p_paddr if phdr.p_paddr > stats[COLUMN_PADDR] else stats[COLUMN_PADDR]
        stats[COLUMN_FSIZE] = phdr.p_filesz if phdr.p_filesz > stats[COLUMN_FSIZE] else stats[COLUMN_FSIZE]
        stats[COLUMN_MSIZE] = phdr.p_memsz if phdr.p_memsz > stats[COLUMN_MSIZE] else stats[COLUMN_MSIZE]
        stats[COLUMN_FLAGS] = phdr.p_flags if phdr.p_flags > stats[COLUMN_FLAGS] else stats[COLUMN_FLAGS]

    for idx in range(COLUMN_ALIGN + 1):
        stats[idx] = len(hex(stats[idx]).rstrip('L'))-2

    for idx, phdr in enumerate(phdrs):
        retval.insert_data(idx+1, COLUMN_SNO, str(idx+1))
        retval.insert_data(idx+1, COLUMN_TYPE, PT_DESCRIPTION.get(phdr.p_type, phdr.p_type))
        retval.insert_data(idx+1, COLUMN_OFF, hex_addr(phdr.p_offset, stats[COLUMN_OFF]))
        retval.insert_data(idx+1, COLUMN_VADDR, hex_addr(phdr.p_vaddr, stats[COLUMN_VADDR]))
        retval.insert_data(idx+1, COLUMN_PADDR, hex_addr(phdr.p_paddr, stats[COLUMN_PADDR]))
        retval.insert_data(idx+1, COLUMN_FSIZE, hex_addr(phdr.p_filesz, stats[COLUMN_FSIZE]))
        retval.insert_data(idx+1, COLUMN_MSIZE, hex_addr(phdr.p_memsz, stats[COLUMN_MSIZE]))
        retval.insert_data(idx+1, COLUMN_FLAGS, hex_addr(phdr.p_flags, stats[COLUMN_FLAGS]))
        retval.insert_data(idx+1, COLUMN_ALIGN, hex(phdr.p_align), justify=retval.LEFT)
    return retval.get_data()

def pack_phdrs(phdrs):
    retval = ''
    for phdr in phdrs:
        retval += phdr.pack()
    return retval

def _extract_data(data, offset, size):
    subdata = ''
    if (offset < len(data) and size > 0):
        subdata = data[offset : offset + size]
    return subdata

def extract_segment_data(data, phdr):
    offset = phdr.p_offset
    size = phdr.p_filesz
    return _extract_data(data, offset, size)

def extract_segments(data, phdrs):
    retval = {}
    for phdr in phdrs:
        retval[phdr] = extract_segment_data(data, phdr)
    return retval
