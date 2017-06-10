#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
'''
Created on Nov 25, 2013

@author: hraghav
'''

from sectools.common.parsegen import PAD_BYTE_0
from sectools.common.parsegen.elf.format import ParseGenElf
from sectools.features.isc.parsegen.base import SecParseGenBase
from sectools.features.isc.parsegen.elf import SecParseGenElf
from sectools.features.isc.parsegen.ewm.header import EwmElf32_Ehdr
from sectools.features.isc.parsegen.ewm.segment import EwmElf32_Phdr,\
    ELF_DEFAULT_PHDR_OFFSET


EWM_FILE_TYPE = 'ewm'


class SecParseGenEwm(SecParseGenElf):

    def __init__(self, data, imageinfo=None, ewm_properties=None,
                 elf_properties=None, general_properties=None,
                 encdec=None,
                 debug_dir=None,
                 debug_prefix=None,
                 debug_suffix=None,):
        SecParseGenBase.__init__(self, data, imageinfo, general_properties,
                                 encdec, debug_dir, debug_prefix, debug_suffix)

        # Check the arguments
        if imageinfo is not None:
            ewm_properties = imageinfo.image_type.ewm_properties
            elf_properties = imageinfo.image_type.elf_properties
            general_properties = imageinfo.general_properties
        if ewm_properties is None:
            raise RuntimeError('EWM properties must not be None.')
        if elf_properties is None:
            raise RuntimeError('EWM ELF properties must not be None.')

        # Initialize internal properties
        self._image_entry = 0
        self._relocatable = False

        # Set properties from the config file
        self.image_entry = int(ewm_properties.image_entry, 16)
        self.relocatable = ewm_properties.relocatable

        # Wrap the data in elf if it doesnt already
        if not self.is_elf(data):
            data = self.inject_elf_wrapper(data)

        SecParseGenElf.__init__(self, data, None, elf_properties, general_properties,
                                self.encdec, debug_dir, debug_prefix, debug_suffix)

    #--------------------------------------------------------------------------
    # Mandatory Properties overloaded from the base class
    #--------------------------------------------------------------------------
    @classmethod
    def file_type(cls):
        return 'ewm'

    def get_wrapped_data(self):
        return self._elf_parsegen.get_segment_data(self._elf_parsegen.phdrs[0])

    def set_wrapped_data(self, data):
        phdr = self._elf_parsegen.phdrs[0]
        self._elf_parsegen.remove_segment(phdr)
        phdr = EwmElf32_Phdr(self.image_entry, len(data), self.relocatable)
        self._elf_parsegen.add_segment(phdr, data)

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'image_entry'
    #--------------------------------------------------------------------------
    @property
    def image_entry(self):
        return self._image_entry

    @image_entry.setter
    def image_entry(self, value):
        self.validate_image_entry(value)
        self._image_entry = value

    def validate_image_entry(self, value):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'relocatable'
    #--------------------------------------------------------------------------
    @property
    def relocatable(self):
        return self._relocatable

    @relocatable.setter
    def relocatable(self, value):
        self.validate_relocatable(value)
        self._relocatable = value

    def validate_relocatable(self, value):
        pass

    #--------------------------------------------------------------------------
    # Helper functions
    #--------------------------------------------------------------------------
    def is_elf(self, data):
        retval = False
        try:
            ParseGenElf(data)
            retval = True
        except Exception:
            pass
        return retval

    def inject_elf_wrapper(self, data):
        return (EwmElf32_Ehdr(self.image_entry).pack() +
                EwmElf32_Phdr(self.image_entry, len(data), self.relocatable).pack()
                ).ljust(ELF_DEFAULT_PHDR_OFFSET, PAD_BYTE_0) + data

