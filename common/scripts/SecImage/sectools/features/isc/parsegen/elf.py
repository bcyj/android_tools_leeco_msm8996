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

import hashlib
import math
import struct

from sectools.common.parsegen import elf, PAD_BYTE_1, PAD_BYTE_0
from sectools.common.parsegen.elf import segment
from sectools.common.parsegen.mbn.format import ALIGNED_IMAGE_SIZE
from sectools.common.parsegen.mbn.header import MbnHdr40B
from sectools.common.utils.c_logging import logger
from sectools.common.utils.c_misc import properties_repr, hex_addr
from sectools.features.isc.cfgparser.auto_gen_obj_config import Cfg_Mbn_Properties
from sectools.features.isc.parsegen.base import SecParseGenBase
from sectools.features.isc.parsegen.mbn import SecParseGenMbn

# Debug files
FILE_PROG_SEG_IN            = 'sec_elf_in_prog_segment'
FILE_HASH_SEG_IN            = 'sec_elf_in_hash_segment'
FILE_PROG_HASH_REMOVED_IN   = 'sec_elf_in_prog_hash_removed'
FILE_DECRYPTED_IN           = 'sec_elf_in_decrypted'

# Elf file type
ELF_FILE_TYPE               = 'elf'

# Constants
ELF_BLOCK_ALIGN             = 0x1000        # Block alignment size
MAX_PHDR_COUNT              = 100           # Maximum allowable program headers
FLASH_PARTI_VERSION         = 3             # Flash partition version

# Full flags for specific segments
PF_OS_SEGMENT_HASH_FULL     = 0x02200000
PF_OS_SEGMENT_BOOT_FULL     = 0x03200000
PF_OS_SEGMENT_PHDR_FULL     = 0x07000000


class SecParseGenElf(SecParseGenMbn):

    def __init__(self, data, imageinfo=None, elf_properties=None,
                 general_properties=None,
                 encdec=None,
                 debug_dir=None,
                 debug_prefix=None,
                 debug_suffix=None,):
        SecParseGenBase.__init__(self, data, imageinfo, general_properties,
                                 encdec, debug_dir, debug_prefix, debug_suffix)

        # Check the arguments
        if imageinfo is not None:
            elf_properties = imageinfo.image_type.elf_properties
            general_properties = imageinfo.general_properties
        if elf_properties is None:
            raise RuntimeError('ELF properties must not be None.')

        # Initialize internal properties
        self._image_type = 0
        self._serial_num = None

        # Set properties from the config file
        self.image_type = elf_properties.image_type
        self.serial_num = general_properties.testsig_serialnum

        # Initialize the elf parsegen
        self._elf_parsegen = elf.ParseGenElf(data,
                                             self.debug_dir,
                                             self.debug_prefix,
                                             self.debug_suffix)

        # Remove the prog header and hash segment
        phdr_segment, hash_segment = self.extract_phdr_hash(self._elf_parsegen)
        self.store_debug_data(FILE_PROG_SEG_IN, phdr_segment)
        self.store_debug_data(FILE_HASH_SEG_IN, hash_segment)
        self.store_debug_data(FILE_PROG_HASH_REMOVED_IN, self._elf_parsegen.get_data())

        self.hash_segment = hash_segment

        # If hash_segment is empty, create dummy hash_segment
        if not hash_segment:
            hash_segment = self._generate_default_hash_segment()

        # Initialize the base now
        SecParseGenMbn.__init__(self,
                                data=hash_segment,
                                imageinfo=None,
                                mbn_properties=self._get_sec_parsegen_mbn_properties(),
                                general_properties=general_properties,
                                encdec=self.encdec,
                                debug_dir=debug_dir,
                                debug_prefix=debug_prefix,
                                debug_suffix=debug_suffix)

        # Check if the file is encrypted
        if self.is_encrypted():
            self._decrypt_data()
            self.store_debug_data(FILE_DECRYPTED_IN, self._elf_parsegen.get_data())

            # Get the original data
            self._elf_parsegen = elf.ParseGenElf(self._elf_parsegen.get_data(),
                                                 self.debug_dir,
                                                 self.debug_prefix,
                                                 self.debug_suffix)

    def __repr__(self):
        return ('Base Properties: ' + '\n' + SecParseGenBase.__repr__(self) + '\n'
                'ELF Properties: ' + '\n' + repr(self._elf_parsegen) + '\n'
                'Hash Segment Properties: ' + '\n' + repr(self._mbn_parsegen) + '\n'
                'SecElf Properties: ' + '\n'
                '' + properties_repr([('image_type', self.image_type),
                                      ('testsig_serialnum', hex_addr(self.serial_num) if self.serial_num is not None else self.serial_num),]))

    #--------------------------------------------------------------------------
    # Mandatory Properties overloaded from the base class
    #--------------------------------------------------------------------------
    @classmethod
    def file_type(cls):
        return ELF_FILE_TYPE

    def get_data(self, integrity_check=None, sign=None, encrypt=None):
        # Resolve the operation
        integrity_check = self.integrity_check if integrity_check is None else integrity_check
        sign = self.sign if sign is None else sign
        encrypt = self.encrypt if encrypt is None else encrypt
        integrity_check = True if (integrity_check or sign or encrypt) else False

        # Allow base to do any checks
        SecParseGenBase.get_data(self, integrity_check, sign, encrypt)

        if not (integrity_check or sign or encrypt):
            data = self._elf_parsegen.get_data()
        else:
            # Get the size of the hash segment
            hash_segment_size = self._compute_hash_segment_size(integrity_check, sign, encrypt)
            hash_segment_addr = self._compute_hash_address(self._elf_parsegen)

            # Add the prog & hash entries in phdrs
            # The data in the prog and hash at this time maybe dummy data
            phdr_class = self._elf_parsegen.get_new_phdr_entry()
            prog_phdr_entry = self._get_prog_phdr_entry(phdr_class(),
                                                        self._elf_parsegen.ehdr)
            hash_phdr_entry = self._get_hash_phdr_entry(phdr_class(),
                                                        hash_segment_size,
                                                        hash_segment_addr)
            self._elf_parsegen.add_segment(hash_phdr_entry, PAD_BYTE_1 * hash_segment_size)
            self._elf_parsegen.add_segment(prog_phdr_entry, '')

            # Update the dest pointer for the mbn
            self._mbn_parsegen.header.image_dest_ptr = hash_segment_addr + self._mbn_parsegen.header.get_size()

            error = None
            try:
                # Generate the hash segment now
                hash_segment = self.get_hash_segment(integrity_check, sign, encrypt)

                # Check here for sizes mismatching just in case
                if len(hash_segment) != hash_segment_size:
                    raise RuntimeError('Estimating the size of the hash table was wrong.')

                # Re-add the hash segment, this time with the real data
                self._elf_parsegen.remove_segment(hash_phdr_entry)
                self._elf_parsegen.remove_segment(prog_phdr_entry)
                self._elf_parsegen.add_segment(hash_phdr_entry, hash_segment)
                self._elf_parsegen.add_segment(prog_phdr_entry, '')

                # If encrypting, change the process segment data
                if encrypt:
                    orig_process_data = self._elf_parsegen._process_segment_data
                    self._elf_parsegen._process_segment_data = self._int_process_data
                    self._int_process_data_segment_number = 0

                try:
                    # Get the elf data
                    data = self._elf_parsegen.get_data()
                except Exception as e:
                    error = e
                finally:
                    # Restore the process segment data
                    if encrypt:
                        try:
                            self._elf_parsegen._process_segment_data = orig_process_data
                        except Exception as tmp_e:
                            logger.warning(str(tmp_e))

                    # Raise error if needed
                    if error is not None:
                        raise error

            except Exception as e:
                error = e
            finally:
                # Remove the prog
                try:
                    self._elf_parsegen.remove_segment(prog_phdr_entry)
                except Exception as tmp_e:
                    logger.warning(str(tmp_e))

                # Remove the hash
                try:
                    self._elf_parsegen.remove_segment(hash_phdr_entry)
                except Exception as e:
                    logger.warning(str(tmp_e))

                # Raise error if needed
                if error is not None:
                    raise error

        return data

    @property
    def data_to_sign(self):
        # Get the size of the hash segment
        hash_segment_size = self._compute_hash_segment_size(self.integrity_check, self.sign, self.encrypt)
        hash_segment_addr = self._compute_hash_address(self._elf_parsegen)

        # Add the prog & hash entries in phdrs
        # The data in the prog and hash at this time maybe dummy data
        phdr_class = self._elf_parsegen.get_new_phdr_entry()
        prog_phdr_entry = self._get_prog_phdr_entry(phdr_class(),
                                                    self._elf_parsegen.ehdr)
        hash_phdr_entry = self._get_hash_phdr_entry(phdr_class(),
                                                    hash_segment_size,
                                                    hash_segment_addr)
        self._elf_parsegen.add_segment(hash_phdr_entry, PAD_BYTE_1 * hash_segment_size)
        self._elf_parsegen.add_segment(prog_phdr_entry, '')

        # Update the dest pointer for the mbn
        self._mbn_parsegen.header.image_dest_ptr = hash_segment_addr + self._mbn_parsegen.header.get_size()

        # Get the data to sign from the hash segment
        data = self.get_hash_segment_data_to_sign()

        # Remove the prog and hash
        self._elf_parsegen.remove_segment(prog_phdr_entry)
        self._elf_parsegen.remove_segment(hash_phdr_entry)

        return data

    def contains_integrity_check(self):
        return self.hash_segment != ''

    #--------------------------------------------------------------------------
    # Helper methods
    #--------------------------------------------------------------------------
    def extract_phdr_hash(self, elf_parsegen):
        phdr_segment, hash_segment = '', ''
        phdr_entry, hash_entry = None, None

        # Find the phdr entries
        for phdr in elf_parsegen.phdrs:
            if phdr.f_os_segment_type == segment.flags.PF_OS_SEGMENT_PHDR:
                phdr_entry = phdr
            elif phdr.f_os_segment_type == segment.flags.PF_OS_SEGMENT_HASH:
                hash_entry = phdr
            if phdr_entry and hash_entry:
                break

        # Remove the phdr entries
        if phdr_entry:
            phdr_segment = elf_parsegen.get_segment_data(phdr_entry)
            elf_parsegen.remove_segment(phdr_entry)
        if hash_entry:
            hash_segment = elf_parsegen.get_segment_data(hash_entry)
            elf_parsegen.remove_segment(hash_entry)

        return phdr_segment, hash_segment

    def get_hash_segment(self, integrity_check, sign, encrypt):
        hash_table = self.get_hash_table()
        self._mbn_parsegen.code = hash_table
        return SecParseGenMbn._get_data_int(self, sign, encrypt)

    def get_hash_segment_data_to_sign(self):
        hash_table = self.get_hash_table()
        self._mbn_parsegen.code = hash_table
        return SecParseGenMbn.data_to_sign.fget(self)

    def get_hash_table(self):
        # Check the number of headers that can be hashed
        phdr_count = len(self._elf_parsegen.phdrs)
        if phdr_count > MAX_PHDR_COUNT:
            raise RuntimeError('Number of prog header entries in ELF "' + phdr_count + '" is more than the maximum supported "' + MAX_PHDR_COUNT + '"')
        hash_table = ''
        for phdr in self._elf_parsegen.phdrs:
            if phdr.f_os_page_mode == segment.flags.PF_OS_NON_PAGED_SEGMENT:
                if phdr.p_type not in [segment.type.PT_PHDR]:
                    data = self._elf_parsegen.get_segment_data(phdr)
                    hash_data = self._get_hash(data)
                    if ((phdr.f_os_segment_type in [segment.flags.PF_OS_SEGMENT_HASH]) or
                        (phdr.f_os_access_type in [segment.flags.PF_OS_ACCESS_NOTUSED,
                                                   segment.flags.PF_OS_ACCESS_SHARED]) or
                        len(data) == 0):
                        hash_table += PAD_BYTE_0 * len(hash_data)
                    else:
                        hash_table += hash_data
            else:
                raise RuntimeError('Paged segment encountered. Cannot hash.')

        # Insert testsig at this point if its needed
        if self.serial_num is not None:
            hash_table = self._insert_serial_in_ht(hash_table, self._serial_num)
        return hash_table

    def _compute_hash_segment_size(self, integrity_check, sign, encrypt):
        phdr_count = len([phdr for phdr in self._elf_parsegen.phdrs \
                          if ((phdr.f_os_page_mode == segment.flags.PF_OS_NON_PAGED_SEGMENT) and
                              (phdr.p_type not in [segment.type.PT_PHDR]))])
        return (self._mbn_parsegen.get_header().get_size() +                        # Mbn Header
                (len(self._get_hash('')) * (2 + phdr_count)) +                      # Hash of all segments
                (self.sig_size if sign else 0) +                                    # Size of signature
                (self.cert_chain_size if sign else 0) +                             # Size of cert chain
                (len(self.encryption_params) if encrypt else 0)                     # Size of encryption params
               )

    @classmethod
    def _get_hash(cls, data):
        h = hashlib.sha256()
        h.update(data)
        return h.digest()

    def _generate_default_hash_segment(self):
        # Get the hash table
        hash_table = self.get_hash_table()

        # Create the header
        header = MbnHdr40B('\0' * MbnHdr40B.get_size())
        header.image_id = self.image_type
        header.flash_parti_ver = FLASH_PARTI_VERSION
        header.image_src = 0
        header.image_dest_ptr = self._compute_hash_address(self._elf_parsegen) + MbnHdr40B.get_size()
        header.image_size = ALIGNED_IMAGE_SIZE(len(hash_table))
        header.code_size = len(hash_table)
        header.sig_ptr = header.image_dest_ptr + header.code_size
        header.sig_size = 0
        header.cert_chain_ptr = header.sig_ptr + header.sig_size
        header.cert_chain_size = 0

        return header.pack() + hash_table

    def _get_hash_phdr_entry(self, hash_phdr, size, addr):
        hash_phdr.p_type = segment.type.PT_NULL
        hash_phdr.p_flags = PF_OS_SEGMENT_HASH_FULL
        hash_phdr.p_align = ELF_BLOCK_ALIGN
        hash_phdr.p_offset = ELF_BLOCK_ALIGN
        hash_phdr.p_memsz = int(math.ceil(float(size)/ELF_BLOCK_ALIGN) * ELF_BLOCK_ALIGN)
        hash_phdr.p_filesz = size
        hash_phdr.p_vaddr = addr
        hash_phdr.p_paddr = addr
        return hash_phdr

    def _get_prog_phdr_entry(self, prog_phdr, ehdr):
        prog_phdr.p_type = segment.type.PT_NULL
        prog_phdr.p_flags = PF_OS_SEGMENT_PHDR_FULL
        prog_phdr.p_filesz = ehdr.e_phoff + ((ehdr.e_phnum + 2) * ehdr.e_phentsize)
        return prog_phdr

    def _get_sec_parsegen_mbn_properties(self):
        properties = Cfg_Mbn_Properties()
        properties.header_size = MbnHdr40B.get_size()
        properties.has_magic_num = False
        properties.ota_enabled = False
        properties.preamble_size = None
        properties.page_size = None
        properties.num_of_pages = None
        properties.min_size_with_pad = None
        return properties

    def _int_process_data(self, phdr, data):
        if phdr.f_os_segment_type != segment.flags.PF_OS_SEGMENT_HASH:
            data = self.encrypt_segment(data, self._int_process_data_segment_number)
            self.store_debug_data('encr_' + str(self._int_process_data_segment_number), data)
            self._int_process_data_segment_number += 1
        return data

    def _decrypt_data(self):
        if self.encdec is None:
            raise RuntimeError('Image is encrypted. Cannot proceed without a decryptor.')

        decryptor = self.encdec.get_decryptor(encryption_parameters_blob=self.encryption_params,
                                               key=self.encdec.get_dummy_key())
        seg = 0
        for phdr in self._elf_parsegen.phdrs:
            if (self._elf_parsegen._segment_to_put(phdr) and
                phdr.f_os_segment_type != segment.flags.PF_OS_SEGMENT_HASH):
                data = decryptor.decrypt_segment(self._elf_parsegen.segments[phdr], seg)
                self.store_debug_data('decr_' + str(seg), data)
                seg += 1
                self._elf_parsegen.segments[phdr] = data

    @classmethod
    def _insert_serial_in_ht(cls, hash_table, serial):
        hash_len = len(cls._get_hash(''))
        serial_packed = struct.pack('L', serial)
        return (hash_table[:hash_len] +
                serial_packed +
                hash_table[hash_len + len(serial_packed):])

    def _compute_hash_address(self, elf_parsegen):
        max_phdr = None
        for phdr in elf_parsegen.phdrs:
            if max_phdr is None or phdr.p_paddr > max_phdr.p_paddr:
                if (phdr.f_os_segment_type != segment.flags.PF_OS_SEGMENT_SWAPPED):
                    max_phdr = phdr

        return (((max_phdr.p_paddr + max_phdr.p_memsz - 1) &
                 (~ (ELF_BLOCK_ALIGN - 1))) +
                ELF_BLOCK_ALIGN)

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'image_type'
    #--------------------------------------------------------------------------
    @property
    def image_type(self):
        return self._image_type

    @image_type.setter
    def image_type(self, value):
        self.validate_image_type(value)
        self._image_type = value

    def validate_image_type(self, value):
        pass

    #--------------------------------------------------------------------------
    # Get, Set & Validate functions for 'serial_num'
    #--------------------------------------------------------------------------
    @property
    def serial_num(self):
        return self._serial_num

    @serial_num.setter
    def serial_num(self, value):
        if value is not None:
            if value.startswith('0x'):
                value = int(value, 16)
            else:
                value = int(value, 10)
            self.validate_serial_num(value)
        self._serial_num = value

    def validate_serial_num(self, value):
        if not isinstance(value, int):
            raise RuntimeError('testsig serial number ' + str(value) + ' is not a number')
        if value < 0 or value > 0xFFFFFFFF:
            raise RuntimeError('testsig serial number ' + str(value) + ' is not a valid 32 bit number')

SecParseGenElf.register()

