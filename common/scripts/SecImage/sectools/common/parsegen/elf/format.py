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

import math

import defines as df
from sectools.common.parsegen import PAD_BYTE_0
from sectools.common.parsegen.elf.header.format import extract_ehdr
from sectools.common.parsegen.elf.section.format import zero_out_sections
from sectools.common.parsegen.elf.segment.flags import PF_OS_SEGMENT_HASH,\
    PF_OS_SEGMENT_PHDR
from sectools.common.parsegen.elf.segment.format import extract_phdrs, repr_phdrs,\
    pack_phdrs, extract_segments, get_phdr_class
from sectools.common.parsegen.elf.segment.type import PT_LOAD
from sectools.common.utils.c_logging import logger
from sectools.common.utils.c_misc import store_debug_data_to_file


class ParseGenElf(object):

    def __init__(self, data,
                 debug_dir=None,
                 debug_prefix=None,
                 debug_suffix=None,
                 ):
        # Public properties
        self.debug_dir = debug_dir
        self.debug_prefix = debug_prefix
        self.debug_suffix = debug_suffix

        # Store the original image
        self.store_debug_data(df.FILE_DATA_IN, data)

        """
        Extract the various segments/sections of the data:
        1. Elf header
        2. Prog headers
        3. Bin
        """
        # Extract the header
        self.ehdr = extract_ehdr(data)
        self.store_debug_data(df.FILE_HDR_IN, self.ehdr.pack())
        self.store_debug_data(df.FILE_HDR_IN_REPR, repr(self.ehdr), suffix=df.FILE_HDR_IN_REPR_SUFFIX)

        # Extract the program headers
        self.phdrs = extract_phdrs(data, self.ehdr)
        self.store_debug_data(df.FILE_PHDR_IN, pack_phdrs(self.phdrs))
        self.store_debug_data(df.FILE_PHDR_IN_REPR, repr_phdrs(self.phdrs), suffix=df.FILE_PHDR_IN_REPR_SUFFIX)

        # Dump the individual segments
        self.segments = extract_segments(data, self.phdrs)
        for idx, phdr in enumerate(self.phdrs):
            length = len(self.segments[phdr])
            is_load = self._segment_to_put(phdr)
            if length >= 0 and length <= 16:
                logger.debug(('' if is_load else 'Non-') + 'Loadable segment - ' + str(idx + 1) + ' is of size: '  + str(length))
                if is_load and (length > 0 and length <= 16):
                    logger.warning(('' if is_load else 'Non-') + 'Loadable segment - ' + str(idx + 1) + ' is of size: '  + str(length))
            self.store_debug_data(df.FILE_SEGMENT_IN.format(idx), self.segments[phdr])

        # Zero out the sections for now
        zero_out_sections(self.ehdr)

    def __repr__(self):
        return ('Elf Header: ' + '\n' + repr(self.ehdr) + '\n'
                'Elf Program Headers: ' + '\n' + repr_phdrs(self.phdrs))

    def store_debug_data(self, file_name, data, prefix=None, suffix=None):
        if prefix is None:
            prefix = self.debug_prefix
        if suffix is None:
            suffix = self.debug_suffix
        if prefix is not None and suffix is not None:
            store_debug_data_to_file(prefix + '_' + file_name + suffix,
                                     data, self.debug_dir)

    def get_prog_data(self):
        # Put the elf header
        data = self.ehdr.pack()

        # Put the program headers
        offset = self.ehdr.e_phoff
        data = data.ljust(offset, PAD_BYTE_0)
        data += pack_phdrs(self.phdrs)

        return data

    def get_data(self):
        # Get the prog data
        data = self.get_prog_data()

        # Put the segments
        for phdr in self.phdrs:
            if self._segment_to_put(phdr):
                offset = phdr.p_offset
                segment_data = self._process_segment_data(phdr, self.segments[phdr])
                if len(segment_data):
                    data = data.ljust(offset, PAD_BYTE_0)
                    data = (data[:offset] + segment_data +
                            data[offset + len(segment_data):])

        return data

    def _segment_to_put(self, phdr):
        if phdr.p_type == PT_LOAD or phdr.f_os_segment_type == PF_OS_SEGMENT_HASH:
            return True
        return False

    def _process_segment_data(self, phdr, data):
        return data

    def get_segment_data(self, phdr):
        if phdr.f_os_segment_type == PF_OS_SEGMENT_PHDR and len(self.segments[phdr]) == 0:
            return self.get_prog_data()
        else:
            return self.segments[phdr]

    def remove_segment(self, phdr):
        if phdr not in self.phdrs:
            raise RuntimeError('Given phdr does not exist in the current list.')

        # Remove the phdr entry
        del self.segments[phdr]
        index = self.phdrs.index(phdr)
        self.phdrs.remove(phdr)
        self.ehdr.e_phnum -= 1

        return index

    def add_segment(self, phdr, data, index=0):
        # Check the segment to add
        if self._segment_to_put(phdr):
            # Add phdr to the segments
            offset = phdr.p_offset
            size = phdr.p_filesz
            align = phdr.p_align
            if align:
                size = int(math.ceil(float(size)/align) * align)

            # Calculate the min offset of the segments to move
            min_offset = None
            for i_phdr in self.phdrs:
                if i_phdr.p_filesz and offset + size > i_phdr.p_offset:
                    if min_offset is None or i_phdr.p_offset < min_offset:
                        min_offset = i_phdr.p_offset

            # Adjust the offsets
            if min_offset is not None:
                shift = offset + size - min_offset
                for i_phdr in self.phdrs:
                    #This check does not work on shared object signing(opendsp).
                    #Load segments start at p_offset = 0
                    #if offset <= i_phdr.p_offset:
                    i_phdr.p_offset += shift

            i_phdr = None

        # Add the phdr entry
        self.segments[phdr] = data
        self.phdrs.insert(index, phdr)
        self.ehdr.e_phnum += 1

    def get_new_phdr_entry(self):
        return get_phdr_class(self.ehdr)

