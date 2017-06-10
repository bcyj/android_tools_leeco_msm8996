#===============================================================================
#
# GenSecImage.py
#
# GENERAL DESCRIPTION
#    Pre-processes images so they can be signed, and packages the
#    certs and signatures into the image to create the signed image
#
# Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#-------------------------------------------------------------------------------
#
#  $Header: //source/qcom/qct/core/securemsm/Tools/rel/gensecimage/build_integration/gensecimage/gensecimage.py#1 $
#  $DateTime: 2013/12/07 08:21:17 $
#  $Author: yliong $
#                      EDIT HISTORY FOR FILE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
# when       who     what, where, why
# --------   ---     ---------------------------------------------------------
# 02/19/13   yliong  Rename QDST to QPSA
# 08/20/12   shitalm Standlone secure boot image creation tool based on CreateTestImage.py
# 08/23/11   dxiang  Support ELF signing
# 08/23/11   dxiang  Add ability to get important QDST parameters from the user
# 06/02/11   dxiang  Re-organize menu based on feedback
# 05/27/11   dxiang  Support QDST
# 05/24/11   dxiang  Change way to get input code sample, use absolute path
#                    Change get_user_input_value() to support default value
# 05/13/11   dxiang  Add option to view header data structures for reference
# 05/12/11   dxiang  May use any custom code image, instead of only dummy images
# 05/11/11   dxiang  Options 1/2 now create default non secure images
#                    Change output file names to be more intuitive
# 05/10/11   dxiang  Add 2KB Preamble Support
#                    Add support for custom image_type fields
# 04/19/11   dxiang  Initial Version
#===============================================================================

import stat
import csv
import itertools
import struct
import os
import shutil
import glob
import zipfile
import sys
import ConfigParser
import getopt
import time
import subprocess

CURRENT_VERSION = '2.2.0.0'
gensecimage_dir = os.path.dirname(os.path.realpath(__file__))

ssd_dir = os.path.join(gensecimage_dir, 'ssd')
sys.path.append(ssd_dir)
import gen_ssd_bin

sys.path.append(gensecimage_dir)
import mbn_tools

#----------------------------------------------------------------------------
# GLOBAL VARIABLES BEGIN
#----------------------------------------------------------------------------
#
# boolean types
#
encrypt = False
encrypt_elf = False
has_preamble = False
small_preamble = True
has_magic_num = False
ota_enabled = False

# Cert related
CERT_CHAIN_ONEROOT_MAXSIZE = 6*1024          # Default Cert Chain Max Size for one root
CERT_SINGLE_MAXSIZE = 2*1024                 # Max for one certificate is bounded by 2k
MAX_ROOT_CERT_SUPPORTED = 16
MIN_ROOT_CERT_SUPPORTED = 1

#
# int types
#
header_size = 40
preamble_size = 0
flash_auto_detect_max_page = 0
flash_auto_detect_min_page = 0
cert_chain_maxsize_oneroot = CERT_CHAIN_ONEROOT_MAXSIZE
testsig_serialnum = None

# Preamble related Magic numbers filled in for boot headers
FLASH_CODE_WORD                       = 0x844BDCD1
MAGIC_NUM                             = 0x73D71034
AUTODETECT_PAGE_SIZE_MAGIC_NUM        = 0x7D0B435A
PAD_BYTE_1                            = 255             # Padding byte 1s
PAD_BYTE_0                            = 0               # Padding byte 0s
SBL_VIRTUAL_BLOCK_MAGIC_NUM           = 0xD48B54C6

# ELF Program Header Types
NULL_TYPE                 = 0x0
LOAD_TYPE                 = 0x1
DYNAMIC_TYPE              = 0x2
INTERP_TYPE               = 0x3
NOTE_TYPE                 = 0x4
SHLIB_TYPE                = 0x5
PHDR_TYPE                 = 0x6
TLS_TYPE                  = 0x7

# Segment Type
MI_PBT_L4_SEGMENT                     = 0x0
MI_PBT_AMSS_SEGMENT                   = 0x1
MI_PBT_HASH_SEGMENT                   = 0x2
MI_PBT_BOOT_SEGMENT                   = 0x3
MI_PBT_L4BSP_SEGMENT                  = 0x4
MI_PBT_SWAPPED_SEGMENT                = 0x5
MI_PBT_SWAP_POOL_SEGMENT              = 0x6
MI_PBT_PHDR_SEGMENT                   = 0x7

#
# string types
#
image_format = 'mbn'
source_image = ''
preamble_type = ''
signattrs = []
signattrs_ispath = ['rootcakey', 'rootcacert', 'attestcakey', 'attestcacert', 'rootcerts_dir']
signattrs_mandatory = ['msm_part', 'sw_id', 'oem_id', 'model_id']
signattrs_cass_mandatory = ['host', 'port', 'trust_keystore', 'msm_part', 'sw_id', 'oem_id', 'model_id']
supported_stages = ['presign', 'postsign',
                   'genhashtable', 'encryptonly',
                   'qpsa:sign',
                   'cass:sign']
postsigncmd_supported_stages = ['postsign', 'qpsa:sign', 'encryptonly', 'cass:sign']
supported_image_formats = ['rawelf', 'htelf', 'mbn', 'bin']
total_num_root_certs = MIN_ROOT_CERT_SUPPORTED
source_fpath = ''        #Source location + filename i.e "c:/builds/sbl1.mbn"
source_fname = ''        #Source filename with extension i.e "sbl1.mbn"
source_fname_base = ''   #Source filename without extension i.e "sbl1"
source_fname_ext =  ''   #Source extension i.e ".mbn"
tosign_fname =     '_tosign.out'    #File that must be signed i.e "sbl1_tosign.out or amss_ht_tosign.out"
digest_fname =   '_tosign_digest.out'    #Binary digest of the tosign.out
preamble_fname =   '_preamble.out'    #File where mbn preamble was written i.e "sbl1_preamble.out"
magic_num_removed_fname = '_magic_num_removed' #File where magic number is removed from the source
ht_mbnhdr_fname =  '_ht_mbnhdr.out'    #File containing the mbn header of the Hashtable i.e "amss_ht_mbnhdr.out"
ht_hashes_fname =  '_ht_hashes.out'    #File containing the segment hashes i.e "amss_ht_hashes.out"
signed_fname =     '_signed'         #File for mbn containing the final signed image [preamble] + header + code + signature + cert chain or
                                     #elf containing final signed elf + program + hashtable (mbn + hashes + sig + cert chain) + other elf segments
ht_signed_fname =  '_ht_signed.out'     #File for elf containing the mbn header + hashtable + signature + certs
enc_xml_fname =    '_enc_md.xml'       #File containing Encryption XML header
enc_1_fname =      '_enc_1.out'         #File generating during encryption
enc_2_fname =      '_enc_2.out'         #File generating during encryption
signed_enc_fname = '_signed_encrypted'    #File containing signed and encrypted image
elf_intermediate = '_intermediate_elf.out'     #File containing the intermediate elf which has updated elf header and 2 extra program
                                               #headers. This file does not contain the hashtable, there is an empty space for where
                                               #the hashtable will be written
elf_htremoved_fname = '_htremoved_elf.out'     #File containing a hashtable which was removed and -2 program headers
ht_nonsec_fname =     '_ht_mbnhdr_nonsec.out' #File containing the mbn header + hash table for normal boot (non secure boot)
nonsec_fname =        '_nonsec'            #File containing nonsecure image
nonsec_enc_fname =    '_nonsec_encrypted'  #File containing nonsecure encrypted image
signingpackage_fname = '_signingpackage.xml' #File containing signing package for cass
signaturepackage_fname = '_signaturepackage.xml' #File containing signature package for cass
log_fname =           ''                       #File containing this scripts log
elf_temp_in_fname =  'temp_elf.in'
elf_temp_out_fname = 'temp_elf.out'
temp_fname =         'temp.out'
temp_concat_cert_fname = 'temp_concat_cert.out'
log_fp = None
image_type = 0
QPSA_PY = os.path.join(gensecimage_dir, 'qpsa', 'qpsa.py')
QPSA_ZIP = os.path.join(gensecimage_dir, 'qpsa', 'qpsa.zip')
attest_cert_id =    '-attestation_cert.cer'
attest_ca_cert_id = '-attestation_ca_cert.cer'
root_cert_id = '-root_cert.cer'
root_cert_prefix = '-root_cert'
signature_id = '-signature.bin'
outputdir = ''
final_output_fname = ''
config = None
signing_config = None
postsign_config = None
section_name = ''
stage = ''
config_filename = ''
signingconfig_filename = None
postsignconfig_filename = None
postsigncmd = None
key_config = ''
cass_engine = None

#----------------------------------------------------------------------------
# GLOBAL VARIABLES END
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# CLASS DEFINITIONS BEGIN
#----------------------------------------------------------------------------

#
#----------------------------------------------------------------------------
# ELF Program Header Class
#----------------------------------------------------------------------------
class Mbn_40b_Hdr:
   # Structure object to align and package the 40 byte elf header
   s = struct.Struct('I' * 10)

   def __init__(self, data):
      unpacked_data       = (Mbn_40b_Hdr.s).unpack(data)
      self.image_id              = unpacked_data[0]
      self.flash_parti_ver       = unpacked_data[1]
      self.image_src             = unpacked_data[2]
      self.image_dest_ptr        = unpacked_data[3]
      self.image_size            = unpacked_data[4]
      self.code_size             = unpacked_data[5]
      self.sig_ptr               = unpacked_data[6]
      self.sig_size              = unpacked_data[7]
      self.cert_chain_ptr        = unpacked_data[8]
      self.cert_chain_size       = unpacked_data[9]

   def printValues(self):
      print "ATTRIBUTE / VALUE"
      for attr, value in self.__dict__.iteritems():
         print attr, hex(value)

   def writePackedData(self, fp):
      values = [self.image_id,
                self.flash_parti_ver,
                self.image_src,
                self.image_dest_ptr,
                self.image_size,
                self.code_size,
                self.sig_ptr,
                self.sig_size,
                self.cert_chain_ptr,
                self.cert_chain_size
               ]

      s = struct.Struct('I' * 10)

      packed_data = s.pack(*values)
      fp.write(packed_data)

class Mbn_80b_Hdr:
   # Structure object to align and package the 80 byte elf header
   s = struct.Struct('I' * 20)

   def __init__(self, data):
      unpacked_data       = (Mbn_80b_Hdr.s).unpack(data)
      #self.unpacked_data  = unpacked_data
      self.codeword        = unpacked_data[0]
      self.magic           = unpacked_data[1]
      self.image_id        = unpacked_data[2]
      self.reserved_1      = unpacked_data[3]
      self.reserved_2      = unpacked_data[4]
      self.image_src       = unpacked_data[5]
      self.image_dest_ptr  = unpacked_data[6]
      self.image_size      = unpacked_data[7]
      self.code_size       = unpacked_data[8]
      self.sig_ptr         = unpacked_data[9]
      self.sig_size        = unpacked_data[10]
      self.cert_chain_ptr  = unpacked_data[11]
      self.cert_chain_size = unpacked_data[12]
      self.reserved_3      = unpacked_data[13]
      self.reserved_4      = unpacked_data[14]
      self.reserved_5      = unpacked_data[15]
      self.reserved_6      = unpacked_data[16]
      self.reserved_7      = unpacked_data[17]
      self.reserved_8      = unpacked_data[18]
      self.reserved_9      = unpacked_data[19]

   def printValues(self):
      print "ATTRIBUTE / VALUE"
      for attr, value in self.__dict__.iteritems():
         print attr, hex(value)

   def writePackedData(self, fp):
      values = [self.codeword,
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
                self.reserved_9
               ]

      s = struct.Struct('I' * 20)

      packed_data = s.pack(*values)
      fp.write(packed_data)

class FileLock:
    def __init__(self, filename):
        self.filename = filename
        self.fd = None
        self.pid = os.getpid()

    def acquire(self):
        try:
            self.fd = os.open(self.filename, os.O_CREAT|os.O_EXCL|os.O_RDWR)
            # Only needed to let readers know who's locked the file
            os.write(self.fd, "%d" % self.pid)
            return 1    # return ints so this can be used in older Pythons
        except OSError:
            self.fd = None
            return 0

    def release(self):
        if not self.fd:
            return 0
        try:
            os.close(self.fd)
            os.remove(self.filename)
            self.fd = None
            return 1
        except OSError:
            return 0

    def __del__(self):
        self.release()
#----------------------------------------------------------------------------
# CLASS DEFINITIONS END
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# HELPER FUNCTIONS BEGIN
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# generate_file_names
# Create all the files names for the files we'll need to create for the pre,
# sign and post processes
#----------------------------------------------------------------------------
def generate_file_names():
  global source_fpath
  global source_fname
  global source_fname_base
  global source_fname_ext
  global tosign_fname
  global digest_fname
  global preamble_fname
  global magic_num_removed_fname
  global elf_intermediate
  global signed_fname
  global ht_signed_fname
  global enc_xml_fname
  global enc_1_fname
  global enc_2_fname
  global signed_enc_fname
  global ht_mbnhdr_fname
  global ht_hashes_fname
  global elf_htremoved_fname
  global nonsec_fname
  global nonsec_enc_fname
  global ht_nonsec_fname
  global outputdir
  global final_output_fname
  global signingpackage_fname
  global signaturepackage_fname

  source_fname_base = outputdir + '/' + source_fname_base
  log("in generate %s" % source_fname_base)
  if (image_format == 'mbn'):
    tosign_fname = source_fname_base + '_tosign.out'
    digest_fname = source_fname_base + '_tosign_digest.out'
  else:
    tosign_fname = source_fname_base + "_ht" + '_tosign.out'
    digest_fname = source_fname_base + "_ht" + '_tosign_digest.out'

  preamble_fname = source_fname_base + '_preamble.out'
  magic_num_removed_fname = source_fname_base + '_magic_num_removed' + source_fname_ext
  signed_fname = source_fname_base + '_signed' + source_fname_ext
  enc_xml_fname = source_fname_base + '_enc_md.xml'
  enc_1_fname = source_fname_base + '_enc_1.out'
  enc_2_fname = source_fname_base + '_enc_2.out'
  signed_enc_fname = source_fname_base + '_signed_encrypted' + source_fname_ext
  elf_htremoved_fname = source_fname_base + '_htremoved_elf.out'
  ht_mbnhdr_fname = source_fname_base + '_ht_mbnhdr.out'
  ht_hashes_fname = source_fname_base + '_ht_hashes.out'
  elf_intermediate = source_fname_base + '_intermediate_elf.out'
  ht_signed_fname = source_fname_base + '_ht_signed.out'
  ht_nonsec_fname = source_fname_base + '_ht_mbnhdr_nonsec.out'
  nonsec_fname = source_fname_base + '_nonsec'  + source_fname_ext
  nonsec_enc_fname = source_fname_base + '_nonsec_encrypted' + source_fname_ext
  signingpackage_fname = source_fname_base + '_signingpackage.xml'
  signaturepackage_fname = source_fname_base + '_signaturepackage.xml'
  #final_output_fname will be in the same format as the input.
  #It will be a copy of either signed_fname or signed_enc_fname
  final_output_fname = source_fname_base + source_fname_ext
#----------------------------------------------------------------------------
# clean_files
# Remove the files that are around and will get regenerated, so that if something
# fails, it will be easy to see what files were and were not created, so understand
# where things failed
#----------------------------------------------------------------------------
def clean_files(stage):
  #remove the files if they already exist as new ones will be generated
  if (not(stage=="postsign")):
    #these files get generated in the pre-processing stage
    log("removing %s\n" % tosign_fname)
    if os.path.exists(tosign_fname):
      os.remove(tosign_fname)
    print "removing %s" % preamble_fname
    if os.path.exists(preamble_fname):
      os.remove(preamble_fname)
    if os.path.exists(elf_htremoved_fname):
      os.remove(elf_htremoved_fname)
    if os.path.exists(ht_mbnhdr_fname):
      os.remove(ht_mbnhdr_fname)
    if os.path.exists(ht_hashes_fname):
      os.remove(ht_hashes_fname)
    if os.path.exists(elf_htremoved_fname):
      os.remove(elf_htremoved_fname)

    #these files are generated in signing stage
    if os.path.exists(signed_fname):
      os.remove(signed_fname)
    if os.path.exists(ht_signed_fname):
      os.remove(ht_signed_fname)
    if os.path.exists(digest_fname):
      os.remove(digest_fname)
    if os.path.exists(signingpackage_fname):
      os.remove(signingpackage_fname)
    #if os.path.exists(signaturepackage_fname):
    #  os.remove(signaturepackage_fname)

    #these files are generated in post-processing stage
    if os.path.exists(enc_xml_fname):
      os.remove(enc_xml_fname)
    if os.path.exists(enc_1_fname):
      os.remove(enc_1_fname)
    if os.path.exists(enc_2_fname):
      os.remove(enc_2_fname)
    if os.path.exists(signed_enc_fname):
      os.remove(signed_enc_fname)
    if os.path.exists(elf_intermediate):
      os.remove(elf_intermediate)
    if os.path.exists(ht_nonsec_fname):
      os.remove(ht_nonsec_fname)
    if os.path.exists(nonsec_fname):
      os.remove(nonsec_fname)
    if os.path.exists(nonsec_enc_fname):
      os.remove(nonsec_enc_fname)
    if os.path.exists(temp_fname):
      os.remove(temp_fname)
    if os.path.exists(temp_concat_cert_fname):
      os.remove(temp_concat_cert_fname)


#----------------------------------------------------------------------------
# create_preamble_file
# Reads the preamble bytes from source file and writes it into a seperate file
# returns how many bytes the preamble was
#----------------------------------------------------------------------------
def create_preamble_file(source_file_name, preamble_file_name, max_size_of_verify_buffer, small_preamble, flash_auto_detect_max_page, flash_auto_detect_min_page, preamble_type):
  # Initialize
  max_size_verify = max_size_of_verify_buffer
  flash_max_page = flash_auto_detect_max_page
  flash_min_page = flash_auto_detect_min_page
  autodetectpage = [int('0xFFFFFFFF',16)] * max_size_verify

  log("Finding the preamble from %s and putting it into %s\n" % (source_file_name, preamble_file_name))

  # Open image file
  source_fp = open_file(source_file_name, "rb")
  # Create the preamble file
  preamble_fp = open_file(preamble_file_name, 'wb')

  # Determine if we are skipping special 2KB Preamble
  if flash_max_page == 2048 and flash_min_page == 2048:
   log("This is a 2k preamble\n")
   has_2K_preamble = True
  else:
   has_2K_preamble = False

  # The first three integers in the preamble must include the following values
  read_buff = struct.unpack("III", source_fp.read(12))

  if (read_buff[0] != FLASH_CODE_WORD) or (read_buff[1] != MAGIC_NUM):
    log("Invalid preamble!\n")
    raise RuntimeError, "Preamble not valid"

  # 2kB Preamble does not contain this magic number
  if (has_2K_preamble is False) and (read_buff[2] != AUTODETECT_PAGE_SIZE_MAGIC_NUM):
    log("Preamble not valid!\n")
    raise RuntimeError, "Preamble not valid"

  source_fp.seek(0)
  for i in range(flash_max_page/flash_min_page):
    file_data = source_fp.read(flash_min_page)
    preamble_fp.write(file_data)

  # 2kB Preamble does not contain any additional padding
  if has_2K_preamble is False:
    # Determine appropriate amount of padding for the preamble and
    # update the boot_sbl_header accordingly
    if small_preamble is True:
        amount_to_read = flash_min_page
    else:
        amount_to_read = flash_max_page

    file_data = source_fp.read(amount_to_read)
    preamble_fp.write(file_data)

  position = source_fp.tell();
  preamble_fp.close()
  log("Wrote preamble of %d bytes into %s\n" %(position, preamble_file_name))

  return position

#----------------------------------------------------------------------------
# estimate_cert_size
# This routine adds N-1 number of root certs to CERT_CHAIN_ONEROOT_MAXSIZE (one root chain).
# N is the total number of root certs supported
#----------------------------------------------------------------------------
def estimate_cert_size(total_num_root_certs):
   #log("total_num_root_certs = %s" % total_num_root_certs)
   return (CERT_CHAIN_ONEROOT_MAXSIZE + (CERT_SINGLE_MAXSIZE * (total_num_root_certs-1)))

#----------------------------------------------------------------------------
# create_file_to_sign
# Skips the preamble data, and updates the mbn header in the target file to
# include the certificate chain and signature ptr's and sizes. Image size
# is also updated accordingly
#----------------------------------------------------------------------------
def create_file_to_sign(source_file_name, target_file_name, mbn_header_size, size_of_preamble, cert_chain_size_in = CERT_CHAIN_ONEROOT_MAXSIZE):
  # Open Files
  source_fp = open_file(source_file_name, "rb")
  target_fp = open_file(target_file_name, "wb")

  #skip preamble if it is present to get to the mbn header
  #as we don't sign the preamble (it's not executable code)
  if size_of_preamble != 0:
    log("Skipping %d bytes of preamble from %s to reach the MBN header\n" % (size_of_preamble, source_file_name))
    source_fp.seek(size_of_preamble)

  if mbn_header_size == 80:
     log("Reading 80 byte MBN header from offset %d of file %s\n" % (source_fp.tell(), source_file_name))
     source_header = Mbn_80b_Hdr(source_fp.read(mbn_header_size))
  else:
     log("Reading 40 byte MBN header from offset %d of file %s\n" % (source_fp.tell(), source_file_name))
     source_header = Mbn_40b_Hdr(source_fp.read(mbn_header_size))

  #source_header.printValues()

  log("READ MBN HEADER\n")
  log("                SIG_PTR         = %d" % source_header.sig_ptr)
  log("                SIG_SIZE        = %d" % source_header.sig_size)
  log("                CERT_CHAIN_PTR  = %d" % source_header.cert_chain_ptr)
  log("                CERT_CHAIN_SIZE = %d" % source_header.cert_chain_size)
  log("                IMAGE_SIZE      = %d"  % source_header.image_size)

  orig_image_size = source_header.image_size

  source_header.sig_ptr = source_header.image_dest_ptr + source_header.code_size
  source_header.sig_size = 0x100
  source_header.cert_chain_ptr = source_header.sig_ptr + source_header.sig_size
  source_header.cert_chain_size = cert_chain_size_in
  image_size = source_header.code_size + source_header.sig_size + source_header.cert_chain_size
  if (image_size % 4) != 0:
     log("\nRounding up image_size to be 4 byte aligned\n")
     image_size += (4 - (image_size % 4))
  source_header.image_size = image_size

  log("UPDATED MBN HEADER")
  log("                SIG_PTR         = %d" % source_header.sig_ptr)
  log("                SIG_SIZE        = %d" % source_header.sig_size)
  log("                CERT_CHAIN_PTR  = %d" % source_header.cert_chain_ptr)
  log("                CERT_CHAIN_SIZE = %d" % source_header.cert_chain_size)
  log("                IMAGE_SIZE      = %d" % source_header.image_size)
  source_header.writePackedData(target_fp)
  log("WRITTEN TO %s\n" % target_file_name)

  log("Writing bytes after header\n")

  bin_data = source_fp.read(source_header.code_size)
  target_fp.write(bin_data)

  source_fp.close()
  target_fp.close()

#----------------------------------------------------------------------------
# open_file
# Helper function to open a file and return a valid file object
#----------------------------------------------------------------------------
def open_file(file_name, mode):
    try:
       fp = open(file_name, mode)
    except:
       raise RuntimeError, "The file could not be opened: " + file_name

    # File open has succeeded with the given mode, return the file object
    return fp

#----------------------------------------------------------------------------
# concat_files
# Concatenates the files listed in 'sources' in order and writes to 'target'
#----------------------------------------------------------------------------
def concat_files(target, sources):
   if type(sources) is not list:
      sources = [sources]

   target_file = open_file(target,'wb')

   for fname in sources:
      file = open_file(fname,'rb')
      log("Appending %s to %s\n" % (fname, target))
      while True:
         bin_data = file.read(65536)
         if not bin_data:
            break
         target_file.write(bin_data)
      file.close()
   target_file.close()

#----------------------------------------------------------------------------
# Unzip_Files
# Unzips files from target_zip and places them in the Unzipped_path
#----------------------------------------------------------------------------
def Unzip_Files (target_zip, Unzipped_path):
   target_zipfp1 = zipfile.ZipFile(target_zip, "r")
   List_Of_Files = target_zipfp1.infolist()
   for files in List_Of_Files:
      Data = target_zipfp1.read(files.filename)
      log("Unzipping %s at %s" % (files.filename,Unzipped_path))
      if Data:
         target_file = open_file(Unzipped_path+"/"+files.filename,'wb')
         target_file.write(Data)
         target_file.close()
   target_zipfp1.close()
   return List_Of_Files

def elf_contain_hashtable(elf_in_file_name):
   #get the program headers from the file
   [elf_header, phdr_table] = mbn_tools.preprocess_elf_file(elf_in_file_name)
   num_phdrs = elf_header.e_phnum
   log("Number of program headers listed in elf header of %s is %d\n" % (elf_in_file_name,  elf_header.e_phnum))

   # Found hashtable
   phdr_start  = elf_header.e_phoff
   for i in range(num_phdrs):
      curr_phdr = phdr_table[i]

      #Don't copy the hashtable header
      if (mbn_tools.MI_PBT_SEGMENT_TYPE_VALUE(curr_phdr.p_flags) == mbn_tools.MI_PBT_HASH_SEGMENT) is True:
        log("Found hash table!\n")
        return True

   return False

#----------------------------------------------------------------------------
# remove_hashtable_from_elf
# Generates an elf file from the source elf with the hashtable segment removed
# The elf header will contain 2 less program headers (dummy program header for
# ELF + Program header hash is removed and the program header for the hashtable
# is also removed).
# Note the offsets are not adjusted, i.e the generated elf file will contain
# empty space for the bytes related to the hashtable segment. This is ok
# as the segment's are in the correct offsets based on the program headers
#----------------------------------------------------------------------------
def remove_hashtable_from_elf(elf_in_file_name, elf_out_file_name):

   elf_in_fp = open_file(elf_in_file_name, "rb")
   elf_out_fp = open_file(elf_out_file_name, "wb+")

   #get the program headers from the file
   [elf_header, phdr_table] = mbn_tools.preprocess_elf_file(elf_in_file_name)
   num_phdrs = elf_header.e_phnum
   log("Number of program headers listed in elf header of %s is %d\n" % (elf_in_file_name,  elf_header.e_phnum))

   elf_header.e_phnum -= 2
   log("Writing %d for number of program headers and writing updated elf_header to %s\n" % (elf_header.e_phnum, elf_out_file_name))

   # Copy the new ELF header to the destination file
   elf_out_fp.seek(0)
   elf_out_fp.write(elf_header.getPackedData())

   # Write the program headers without the 2 headers that are added for the hash segment
   # i.e the dummy header for the Elf + Program Headers, and the Header for the hashtable
   # and elf segments. NOTE: We don't change the p_offset's of the segments, so there will
   # be an empty space in btw where the hash segment was located. that should be ok
   phdr_start  = elf_header.e_phoff
   relavent_phdr_count = 0;
   for i in range(num_phdrs):
      curr_phdr = phdr_table[i]

      #Don't copy the dummy header for the ELF + Program headers hash
      if ((curr_phdr.p_type == mbn_tools.NULL_TYPE) and (curr_phdr.p_flags == mbn_tools.MI_PBT_ELF_PHDR_SEGMENT)) is True:
        relavent_phdr_count = relavent_phdr_count + 1
        continue

      #Don't copy the hashtable header
      if (mbn_tools.MI_PBT_SEGMENT_TYPE_VALUE(curr_phdr.p_flags) == mbn_tools.MI_PBT_HASH_SEGMENT) is True:
        relavent_phdr_count = relavent_phdr_count + 1
        log("Found hash table!\n")
        continue

      # Copy the program header
      elf_out_fp.seek(phdr_start)
      elf_out_fp.write(curr_phdr.getPackedData())

      # Update phdr_start
      phdr_start += elf_header.e_phentsize

      # Copy the ELF segment
      mbn_tools.file_copy_offset(elf_in_fp, curr_phdr.p_offset, elf_out_fp, curr_phdr.p_offset, curr_phdr.p_filesz)

   if (relavent_phdr_count != 2):
     log("Unexpected elf format, number of expected program headers found is %s" % (elf_in_file_name, relavent_phdr_count))
     raise RuntimeError, "Unexpected elf format: " + elf_in_file_name

def findMatched(file_list, matched_endswith):
    for file in file_list:
        if file.endswith(matched_endswith):
            return file

    return None

#----------------------------------------------------------------------------
# process_signing
# Routine that creates the final signed image.
#----------------------------------------------------------------------------
def process_signing(cert_dir):
  global total_num_root_certs
  zipfiles = glob.glob(cert_dir+'*.zip')

  if len(zipfiles) is 0:
    log('No cert directory in %s!\n' % (cert_dir))
    raise RuntimeError, "No cert directory at %s " % cert_dir

  if len(zipfiles) is 1:
    #Copy the zip file for user reference
    shutil.copy(zipfiles[0], source_fname_base + '.zip')
    #Let's unzip the files in the original directory
    log('Unzipping %s \n' % (cert_dir+'_qdst.zip'))
    files_unzipped = Unzip_Files(zipfiles[0], cert_dir)
  else:
    log('More than 1 zip files at %s!\n' % (cert_dir))
    raise RuntimeError, "More than 1 zip files at %s" % cert_dir

  # Assuming file naming convention in the *.zip
  attestation_ca_cert = ""
  root_cert_list = []
  for files in files_unzipped:
    print files.filename
    if (files.filename.endswith(attest_cert_id) is True):
      attestation_cert = cert_dir + files.filename
      log('Attestation certificate file is %s\n' % (attestation_cert))
    if (files.filename.endswith(attest_ca_cert_id) is True):
      attestation_ca_cert = cert_dir + files.filename
      log('Attestation CA certificate file is %s\n' % (attestation_ca_cert))
    if (files.filename.endswith(signature_id) is True):
      signature = cert_dir + files.filename
      log('Signature file is %s\n' % (signature))
    if (files.filename.find(root_cert_prefix) >= 0):
      root_cert_list.append(cert_dir + files.filename)

  if (total_num_root_certs == len(root_cert_list)):
    if (total_num_root_certs == 1):
        root_cert = root_cert_list[0]
    else:
        root_cert_list_sorted = []
        for i in range (0, len(root_cert_list)):
            root_cert_endswith = "%s%d.cer" % (root_cert_prefix, i)
            matched_file = findMatched(root_cert_list, root_cert_endswith)
            if matched_file is None:
                log ("Error: expected root ends with %s is not in the zip package" % root_cert_endswith)
                raise RuntimeError, "Error: expected root ends with %s is not in the zip package" % root_cert_endswith
            root_cert_list_sorted.append(matched_file)
        concat_files (temp_concat_cert_fname, root_cert_list_sorted)
        root_cert = temp_concat_cert_fname
  else:
    # output error
    log("Error: Number of root certs in zip package %d is different from total_num_root_certs %d in gensecimage.cfg" \
            % (len(root_cert_list), total_num_root_certs))
    raise RuntimeError, "Error: Number of root certs in zip package %d is different from total_num_root_certs %d in gensecimage.cfg" \
            % (len(root_cert_list), total_num_root_certs)

  log('Root certificate file is %s\n' % (root_cert))

  cert_size = estimate_cert_size(total_num_root_certs)
  cert_chain_fname = source_fname_base + '_cert_chain.out'

  if image_format == 'mbn':
    files_to_cat_in_order = [tosign_fname]

    if has_preamble is True:
      files_to_cat_in_order.insert(0,preamble_fname)

    concat_files(temp_fname, files_to_cat_in_order)

    log('Generating the signed image for the mbn in %s\n' % (signed_fname))
    mbn_tools.image_auth( 0,
                temp_fname,
                signature,
                attestation_cert,
                attestation_ca_cert,
                root_cert,
                cert_chain_fname,
                signed_fname,
                cert_size)

    # Add magic number
    if has_magic_num is True:
      mbn_tools.insert_SBL1_magicCookie(0, signed_fname)

    if ota_enabled is True:
      mbn_tools.pad_SBL1_image(0, signed_fname)

    # Remove temporary file
    os.remove(temp_fname)

    if encrypt is True:
      encrypt_image(signed_fname, signed_enc_fname, False)
      log("FINAL SIGNED IMAGE IS: %s" % (signed_enc_fname))
    else:
      log("FINAL SIGNED IMAGE IS: %s" % (signed_fname))

  else:
    log('Generating the signed hashtable for the elf in %s\n' % (ht_signed_fname))

    mbn_tools.image_auth( 0,
                tosign_fname,
                signature,
                attestation_cert,
                attestation_ca_cert,
                root_cert,
                cert_chain_fname,
                ht_signed_fname,
                cert_size)

    if encrypt is True:
        if encrypt_elf is True:
          # ELF encryption
          encrypt_image(signed_fname, signed_enc_fname, encrypt_elf, ht_signed_fname)
        else:
          # MBN encryption
          mbn_tools.pboot_add_hash( 0, elf_intermediate, ht_signed_fname, signed_fname)
          encrypt_image(signed_fname, signed_enc_fname, False)
        log("FINAL SIGNED IMAGE IS: %s" % (signed_enc_fname))
    else:
      # No encryption
      mbn_tools.pboot_add_hash( 0, elf_intermediate, ht_signed_fname, signed_fname)
      log("FINAL SIGNED IMAGE IS: %s" % (signed_fname))

  # Remove the 'cert' directory made by QPSA
  shutil.rmtree(cert_dir)

  # Remove temp file
  if os.path.exists(temp_concat_cert_fname):
      os.remove(temp_concat_cert_fname)

#----------------------------------------------------------------------------
# sign_with_qpsa
# Signs with QPSA tool
#----------------------------------------------------------------------------
def sign_with_qpsa(image_to_sign, cert_dir):
  global outputdir

  if os.path.exists(QPSA_PY) is True:
      #QPSA_ZIP becomes optional is 0.16 due to appsbl integration requires no zip file for LOST OS scan
      # Issue QPSA command
      log("Issuing QPSA command to sign image %s\n" % (image_to_sign))
      #import pdb; pdb.set_trace()
      cmds = ["python", QPSA_PY, 'image=' + image_to_sign, "hash_alg=SHA256", "total_num_root_certs=%d" % total_num_root_certs]
      for signattr in signattrs:
        #Ignore hash_alg, default it to SHA256
        #Ignore total_num_root_certs, read from gensecimage.cfg
        if (signattr[0].lower() == "hash_alg" or signattr[0].lower() == "total_num_root_certs"):
            continue
        cmds.append("%s=%s"%(signattr[0], signattr[1]))
      log(" ".join(cmds))
      err = subprocess.call(cmds)
      if (err != 0):
        log("QPSA failed error = %d" % (err))
        raise RuntimeError, "QPSA signing failed"
  else:
      log("QPSA python script not found at location %s" % QPSA_PY)
      raise RuntimeError, "QPSA script not found"

#----------------------------------------------------------------------------
# sign_with_cass
# Signs with CASS server
#----------------------------------------------------------------------------
def sign_with_cass(image_to_sign, cert_dir):
  log("Sign with CASS: %s" % image_to_sign)
  from cass import sign

  sign(image_to_sign, cert_dir, signattrs, log_fp,
        signingpackage_fname, signaturepackage_fname, digest_fname,
        CURRENT_VERSION, cass_engine)

#----------------------------------------------------------------------------
# read_config_default
# Reads the configuration file default section and stores the values in global variables
#----------------------------------------------------------------------------
def read_config_default(config):
    global signingconfig_filename
    global signing_config
    global postsignconfig_filename
    global postsign_config
    global stage
    global total_num_root_certs

    default_config_dict = config.defaults()
    #print default_config_dict

    #Read total_num_root_certs
    if 'total_num_root_certs' in default_config_dict.keys():
        total_num_root_certs = int(default_config_dict['total_num_root_certs'])
    log("Total_num_root_certs requested = %d" % (total_num_root_certs))

    #Sanity check for total_num_root_certs
    if (total_num_root_certs > MAX_ROOT_CERT_SUPPORTED) or (total_num_root_certs < MIN_ROOT_CERT_SUPPORTED):
        log("Requested total number of root certs out of supported range: %d - %d" % (MIN_ROOT_CERT_SUPPORTED, MAX_ROOT_CERT_SUPPORTED))
        raise RuntimeError, "Requested total number of root certs out of supported range: %d - %d" % (MIN_ROOT_CERT_SUPPORTED, MAX_ROOT_CERT_SUPPORTED)

    #Read signing configuration
    if ((stage == "qpsa:sign") or (stage == "cass:sign")):
        # Only get from default if the signing config is not supplied by the command line inputs
        if (signingconfig_filename == None):
            if 'signconfig' not in default_config_dict.keys():
                log("Signing config file is missing")
                raise RuntimeError, "Signing config file is missing"

            signingconfig_filename = default_config_dict['signconfig']
            log("Signing config = %s" % signingconfig_filename)

        if not os.path.isfile(signingconfig_filename):
           log("Signing config file does not exist:%s" % (signingconfig_filename))
           raise RuntimeError, "Signing config file does not exist:" + signingconfig_filename

        # Initialize signing config parser
        signing_config = ConfigParser.RawConfigParser()
        log("Reading signing config file %s" % signingconfig_filename)
        signing_config.read(signingconfig_filename)

    #Read postsign configuration
    if (postsignconfig_filename == None) and ('postsignconfig' in default_config_dict.keys()):
        postsignconfig_filename = default_config_dict['postsignconfig']
        log("Post-sign commands config = %s" % postsignconfig_filename)

    if ((postsignconfig_filename != None) and (postsignconfig_filename != '')):
        if not os.path.isfile(postsignconfig_filename):
           log("Post-sign commands config file does not exist:%s" % (postsignconfig_filename))
           raise RuntimeError, "Post-sign commands config file does not exist:" + postsignconfig_filename
    else:
        log("No postsign configuration specified.")


#----------------------------------------------------------------------------
# init_config_parser
# Initialize a config parser based on filename
#----------------------------------------------------------------------------
def init_config_parser(config_filename):
    global config

    config = ConfigParser.SafeConfigParser()
    log("Reading config file %s" % config_filename)
    config.read(config_filename)

    return config

#---------------------------------------------------------------------------------
# list_find
# Return True if the string is found in list. Comparison is case insensitive.
#---------------------------------------------------------------------------------
def list_find(list, str_to_find):
    for string in list:
        if string.lower() == str_to_find.lower():
            return True

    return False

#----------------------------------------------------------------------------
# read_ini_file
# Reads the configuration file and stores the values in global variables
#----------------------------------------------------------------------------
def read_ini_file(section_name):
  global image_format
  global source_image
  global encrypt
  global encrypt_elf
  global header_size
  global has_magic_num
  global ota_enabled
  global has_preamble
  global preamble_size
  global small_preamble
  global flash_auto_detect_max_page
  global flash_auto_detect_min_page
  global preamble_type
  global cert_chain_maxsize_oneroot
  global source_fpath
  global source_fname
  global source_fname_base
  global source_fname_ext
  global image_type
  global outputdir
  global config
  global signing_config
  global encrypt
  global encrypt_elf
  global signattrs
  global testsig_serialnum
  encrypt_format = None

  #Reset optional variables
  encrypt = False
  encrypt_elf = False
  cert_chain_maxsize_oneroot = CERT_CHAIN_ONEROOT_MAXSIZE
  header_size = 40
  preamble_size = 0
  has_preamble = False
  has_magic_num = False
  ota_enabled = False
  signattrs = []
  testsig_serialnum = None

  log("Reading section %s\n" % section_name)

  image_format = config.get(section_name,'file_type')
  source_image = config.get(section_name,'file_name')
  log("Source image to use is %s\n" % source_image)
  if (config.has_option(section_name,'encrypt') == True):
    encrypt = config.getboolean(section_name,'encrypt')
    log("encrypt = %d" % encrypt)
  else:
    log("no encryption option")
  if (config.has_option(section_name,'encrypt_format') == True):
    encrypt_format = config.get(section_name,'encrypt_format')
    log("encrypt_format = %s" % encrypt_format)

  #validate source_image
  if not os.path.isfile(source_image):
    log("Source filename does not exist in section %s: %s" % (section_name, source_image))
    raise RuntimeError, "Source filename does not exist in section " + section_name + ": " + source_image

  #validate image_format
  if image_format not in supported_image_formats:
    log("Image format not supported in section %s: %s" % (section_name, image_format))
    raise RuntimeError, "Image format not supported in section " + section_name + ": " + image_format

  if (image_format == 'rawelf') and (elf_contain_hashtable(source_image) is True):
    log("Source image contains hashtable. Please set file_type=htelf in gensecimage.cfg." )
    raise RuntimeError, "Source image contains hashtable. Please set file_type=htelf in gensecimage.cfg."

  if (image_format == 'htelf') and (elf_contain_hashtable(source_image) is False):
    log("Source image does not contain hashtable. Please set file_type=rawelf in gensecimage.cfg." )
    raise RuntimeError, \
        "Source image does not contain hashtable. Please set file_type=rawelf in gensecimage.cfg."

  #validate encrypt_format
  if encrypt:
      if (encrypt_format == None):
        log("No encrypt_format specified while encrypt is turned on")
        raise RuntimeError, "No encrypt_format specified while encrypt is turned on"

      if ((image_format == 'mbn') and (encrypt_format == 'elf')):
        log("Cannot use elf encrypt_format for mbn image")
        raise RuntimeError, "Cannot use elf encrypt_format for mbn image"

      # Only support 'elf' or 'mbn' format
      if (encrypt_format == 'elf'):
        encrypt_elf = True
      elif (encrypt_format != 'mbn'):
        log("Encrypt_format not supported: %s" % encrypt_format)
        raise RuntimeError, "Encrypt_format not supported: %s" % encrypt_format

  #contruct all the file name's we'll be using
  source_fpath, source_fname = os.path.split((source_image))
  source_fname_base, source_fname_ext = os.path.splitext(source_fname)
  log("source_fpath %s" % source_fpath)
  log("source_fname %s" % source_fname)
  log("source_fname_base %s" % source_fname_base)
  log("source_fname_ext %s" % source_fname_ext)

  if (config.has_option(section_name,'cert_chain_maxsize') == True):
     cert_chain_maxsize_oneroot = config.getint(section_name,'cert_chain_maxsize')

  if (config.has_option(section_name,'testsig_serialnum') == True):
     testsig_serialnum_str = config.get(section_name,'testsig_serialnum')
     if len(testsig_serialnum_str)>8:
        raise RuntimeError, "testsig_serialnum=%s should be 32 bit integer (less than 8 hex digits)!" % testsig_serialnum_str
     try:
        testsig_serialnum = int(testsig_serialnum_str, 16)
     except Exception, e:
        raise RuntimeError, "testsig_serialnum=%s should be 32 bit integer (less than 8 hex digits)!" % testsig_serialnum_str
     log("testsig_serialnum read is 0x%x" % testsig_serialnum)

  if (image_format == 'mbn'):
    if (config.has_option(section_name,'header_size') == True):
      header_size = config.getint(section_name,'header_size')

    #Get the preamble information
    if (config.has_option(section_name,'has_preamble') == True):
      has_preamble = config.getboolean(section_name,'has_preamble')
      if has_preamble is True:
        preamble_size = config.getint(section_name,'preamble_size')
        if preamble_size==10:
          small_preamble=True
          flash_auto_detect_max_page=8192
          flash_auto_detect_min_page=2048
          preamble_type = '10kB Preamble'
        if preamble_size==8:
          small_preamble=False
          flash_auto_detect_max_page=4096
          flash_auto_detect_min_page=2048
          preamble_type = '8kB Preamble'
        if preamble_size==2:
          small_preamble=True
          flash_auto_detect_max_page=2048
          flash_auto_detect_min_page=2048
          preamble_type = '2kB Preamble'

    #Remove the magic number
    if (config.has_option(section_name,'has_magic_num') == True):
      has_magic_num = config.getboolean(section_name,'has_magic_num')

    if (config.has_option(section_name,'ota_enabled') == True):
      ota_enabled = config.getboolean(section_name,'ota_enabled')
  else:
    if (config.has_option(section_name,'image_type') == True):
      image_type = config.getint(section_name,'image_type')

  if ((stage == "qpsa:sign") or (stage == "cass:sign")):
      if (config.has_section(section_name) == False):
        log("Section %s is missing in %s" % (section_name, config_filename))
        raise RuntimeError, "Section %s is missing in %s" % (section_name, config_filename)

      sign_options = signing_config.options(section_name)

      if (stage == "cass:sign"):
          mandatory_options = signattrs_cass_mandatory
      else:
          mandatory_options = signattrs_mandatory

      #Check that mandatory options are present
      for mandatory_option in mandatory_options:
        if list_find(sign_options, mandatory_option) is False:
            # no match
            log("Attribute %s is missing in section %s in %s" % \
                (mandatory_option, section_name, signingconfig_filename))
            raise RuntimeError, "Attribute %s is missing in section %s in %s" % \
                (mandatory_option, section_name, signingconfig_filename)

      log("\nSigning attributes:")
      for option_name in sign_options:
        option_value = signing_config.get(section_name, option_name)
        if (stage == "qpsa:sign"):
        #Change path to absolute path as QPSA runs in its directory
            if list_find(signattrs_ispath, option_name) is True:
                option_value = os.path.abspath(option_value)
        signattrs.append([option_name, option_value])
        log("%s=%s" % (option_name, option_value))

  if (config.has_option(section_name,'output_dir') == True):
    outputdir = config.get(section_name,'output_dir')
    outputdir = os.path.abspath(outputdir)
  else:
    outputdir = os.getcwd()
  log ("Outputdir is %s" % (outputdir))

#----------------------------------------------------------------------------
# read_postsigncmd_config
# Reads the configuration file and stores the values in global variables
#----------------------------------------------------------------------------
def read_postsigncmd_config(section_name):
  global postsign_config
  global postsigncmd

  if (postsign_config.has_option(section_name,'cmd') == True):
     postsigncmd = postsign_config.get(section_name,'cmd')
     print postsigncmd

#----------------------------------------------------------------------------
# initialize the encryption object
#----------------------------------------------------------------------------
def ssd_init():
   print "key_config = <" + key_config + ">"
   ssd_p = gen_ssd_bin.SSDConfigClass(ssd_dir, key_config)
   return ssd_p

#----------------------------------------------------------------------------
# encrypt_mbn
#----------------------------------------------------------------------------
def encrypt_mbn(ssd_p, mbn_in_file_name, mbn_out_file_name):
    # encrypt the input file content and write to output file
    ssd_p.enc_segment(0, mbn_in_file_name, mbn_out_file_name)


#----------------------------------------------------------------------------
# encrypt_elf_segments
#----------------------------------------------------------------------------
def encrypt_elf_segments(ssd_p, elf_in_file_name,
                              elf_out_file_name):

   # Open Files
   elf_in_fp = open_file(elf_in_file_name, "rb")
   elf_out_fp = open_file(elf_out_file_name, "wb+")

   # Initialize
   [elf_header, phdr_table] = mbn_tools.preprocess_elf_file(elf_in_file_name)
   encrypted_seg_counter = 0

   # Copy input file to output file
   shutil.copyfileobj(elf_in_fp, elf_out_fp, os.path.getsize(elf_in_file_name))

   # Begin ELF segment encryption
   for i in range(elf_header.e_phnum):
      curr_phdr = phdr_table[i]

      # Only encrypt segments of LOAD_TYPE. Do not encrypt the hash segment.
      if curr_phdr.p_type == LOAD_TYPE and \
         mbn_tools.MI_PBT_SEGMENT_TYPE_VALUE(curr_phdr.p_flags) != MI_PBT_HASH_SEGMENT:

         # Read full segment into buffer
         elf_in_fp.seek(curr_phdr.p_offset)
         data_len = curr_phdr.p_filesz
         file_buff = elf_in_fp.read(data_len)

         temp_in = open(elf_temp_in_fname, "wb")
         temp_in.write(file_buff)
         temp_in.close()
         # Call encryption routine on buffer
         ssd_p.enc_segment(encrypted_seg_counter, elf_temp_in_fname, elf_temp_out_fname)

         temp_out = open(elf_temp_out_fname, "rb")
         encrypted_buf = temp_out.read()
         temp_out.close()
         encrypted_seg_counter += 1

         # Write encrypted segment into output file in same location
         elf_out_fp.seek(curr_phdr.p_offset)
         elf_out_fp.write(encrypted_buf)

   # Close Files
   elf_in_fp.close()
   elf_out_fp.close()


#----------------------------------------------------------------------------
# log
#----------------------------------------------------------------------------
def log (s):
  print "%s\n" % s
  log_fp.write(s)
  log_fp.write("\n")

#----------------------------------------------------------------------------
# get_cert_dir
#----------------------------------------------------------------------------
def get_cert_dir (stage):
  global outputdir

  # QPSA tool always creates the certificate in cert directory of the folder containing the to-be-signed file.
  # To be consistent, we can use the same for CASS.
  if ((stage == "qpsa:sign") or (stage=="cass:sign")):
    cert_dir =  outputdir + '/cert/'
  else:
	cert_dir = source_fname_base+'_cert/'

  log ("get_cert_dir = %s" % cert_dir)
  return cert_dir

#----------------------------------------------------------------------------
# copy_to_final_output
# Copy signed/ecrypted file to the final output filename
#----------------------------------------------------------------------------
def copy_to_final_output ():
    file_to_copy = ''
    QPSA_LogFile = 'qpsa_log.txt'

    # Copy signed or encrypted file to the original filename
    if ((stage == "postsign") or (stage== "qpsa:sign") or (stage== "cass:sign")):
        if (encrypt is True) and os.path.isfile(signed_enc_fname):
            file_to_copy = signed_enc_fname
        elif (encrypt is False) and os.path.isfile(signed_fname):
            file_to_copy = signed_fname

    if ((stage == "encryptonly") or (stage == "genhashtable")):
        if (encrypt is True) and os.path.isfile(nonsec_enc_fname):
            file_to_copy = nonsec_enc_fname
        elif (encrypt is False) and os.path.isfile(nonsec_fname):
            file_to_copy = nonsec_fname

    if (file_to_copy != ''):
        if os.path.isfile(final_output_fname):
            log ("%s exists! It will be over-written" % (final_output_fname))
            os.remove(final_output_fname)
        log ("copy %s to %s" % (file_to_copy, final_output_fname))
        shutil.copyfile(file_to_copy, final_output_fname)

    # Copy QPSA log to the output directory so that it is not overwritten
    if (stage== "qpsa:sign"):
        shutil.copyfile(QPSA_LogFile, outputdir + '/' + QPSA_LogFile)

#----------------------------------------------------------------------------
# encrypt
# Encrypt mbn or elf
#----------------------------------------------------------------------------
def encrypt_image(src_enc_fname, dest_enc_fname, encrypt_elf, ht_fname = None):
    if encrypt is False:
        raise RuntimeError, "encrypt_image should not be called when encrypt is False"

    # Encrypt in "mbn" format
    if encrypt_elf is False:
         # generate xml header
         ssd_p = ssd_init()
         log('Generating the encryption XML header file %s\n' %(enc_xml_fname))
         ssd_p.gen_signed_ssd_xml(enc_xml_fname)
         log('Encrypting unsigned file %s to generated %s\n' % (src_enc_fname,enc_1_fname))
         # encrypt the full mbn image
         encrypt_mbn(ssd_p, src_enc_fname, enc_1_fname)
         # Clean up keys
         ssd_p.deinit(0)
         # concat xml header in front of encrypted image
         log('Generating non-signed and encrypted mbn %s\n' % (signed_enc_fname))
         concat_files (dest_enc_fname, [enc_xml_fname, enc_1_fname])
    else:
         # Encrypt in "elf" format
         if (ht_fname == None):
             raise RuntimeError, "Missing hashtable filename"
         # generate xml header
         ssd_p = ssd_init()
         ssd_p.gen_signed_ssd_xml(enc_xml_fname)
         # concat xml header after the ht cert chain
         concat_files (enc_1_fname, [ht_fname, enc_xml_fname])
         # Add the hash segment into the ELF and encrypt the necessary elf segments
         mbn_tools.pboot_add_hash(0, elf_intermediate,
                                  enc_1_fname,
                                  src_enc_fname)
         encrypt_elf_segments(ssd_p, src_enc_fname, dest_enc_fname)
         #log("FINAL UN-SIGNED IMAGE IS: %s" % (dest_enc_fname))
         # Clean up keys
         ssd_p.deinit(0)

#----------------------------------------------------------------------------
# process_section
# Process a single section
#----------------------------------------------------------------------------
def process_section (section_name, stage):
    global outputdir
    global source_image
    global total_num_root_certs

    # read the configuration file for image information and signing arguments
    log("process_section(): Read gensecimage.cfg file\n")
    read_ini_file(section_name)

    # If stage==encryptonly and encrypt=no, skip to next session
    if ((stage == "encryptonly") and (encrypt_elf == False) and (encrypt == False)):
      log("No encryption for section %s as encrypt=no is set for stage=encrypt-only." % section_name)
      return

    # create the output directory
    if not os.path.exists(outputdir):
      os.makedirs(outputdir)

    # create the names of the files that we'll be using for processing
    generate_file_names()

    # get the directory where certificates should be stored while signing
    cert_dir = get_cert_dir(stage)
    cert_size = estimate_cert_size(total_num_root_certs)
    log("estimate cert size to add: %s, num_roots=%s" % (cert_size, total_num_root_certs))

    # remove existing files, as they will be regenerated
    clean_files(stage);

    # now start working
    if (image_format == 'mbn'):
      log("Main(): Image format is MBN\n")

      if ((stage == "presign") or (stage== "qpsa:sign") or (stage=="cass:sign")):
        #import pdb; pdb.set_trace()
        if has_magic_num is True:
          mbn_tools.remove_SBL1_magicCookie(0, source_image, magic_num_removed_fname)
          source_image = magic_num_removed_fname

        size_of_preamble = 0
        if has_preamble is True:
          size_of_preamble = create_preamble_file(source_image, preamble_fname, 8192, small_preamble, flash_auto_detect_max_page, flash_auto_detect_min_page, preamble_type)

        create_file_to_sign(source_image, tosign_fname, header_size, size_of_preamble, cert_size)

        if (stage == "presign"):
          if not os.path.exists(cert_dir):
            os.mkdir(cert_dir)
          log("SIGN FILE %s and PLACE ZIP FILE IN %s \n" % (tosign_fname,cert_dir))

        if (stage == "qpsa:sign"):
          #sign using QPSA tool
          sign_with_qpsa(tosign_fname, cert_dir)

        #sign with cass if requested
        if (stage=="cass:sign"):
          sign_with_cass(tosign_fname, cert_dir)

      if ((stage == "postsign") or (stage== "qpsa:sign") or (stage=="cass:sign")):
        #Create the final signed image
        process_signing(cert_dir)
      elif (stage == "encryptonly"):
        encrypt_image(source_image, nonsec_enc_fname, encrypt_elf)

    elif ((image_format == 'rawelf') or (image_format == 'htelf')):
      mbn_tools.init_build_vars({'GLOBAL_DICT': {'IMAGE_KEY_CERT_CHAIN_MAXSIZE':cert_chain_maxsize_oneroot}})
      temp_fname = source_image

      if ((stage == "presign") or (stage== "qpsa:sign") or (stage=="cass:sign")):
        if (image_format == 'htelf'):
          remove_hashtable_from_elf(source_image, elf_htremoved_fname)
          temp_fname = elf_htremoved_fname

        hash_seg_size = None
        last_phys_addr = None

        statinfo_in = os.stat(temp_fname)
        # Secure ELF Files
        pboot_gen_elf_cust(0, temp_fname, ht_hashes_fname,
                           elf_out_file_name = elf_intermediate,
                           secure_type = 'secure',
                           hash_seg_max_size = None,
                           last_phys_addr = None,
                           append_xml_hdr = encrypt_elf,
                           cert_chain_size_in = cert_size,
                           serial=testsig_serialnum)
        statinfo_out = os.stat(elf_intermediate)
        file_increment = (statinfo_out.st_size-statinfo_in.st_size)
        log("File increment = %d" % file_increment)
        gen_dict = {'IMAGE_KEY_MBN_TYPE' : 'elf','IMAGE_KEY_IMAGE_ID' : image_type,}
        mbn_tools.image_header(0, gen_dict, ht_hashes_fname, ht_mbnhdr_fname,
                               'secure', elf_file_name = temp_fname,
                               cert_chain_size_in = cert_size)

        # Combine the secure header and secure hash, which should be signed
        concat_files (tosign_fname, [ht_mbnhdr_fname, ht_hashes_fname])

        if (stage == "presign"):
          if not os.path.exists(cert_dir):
            os.mkdir(cert_dir)

          log("SIGN FILE %s and PLACE ZIP FILE IN %s \n" % (tosign_fname,cert_dir))

        #sign with qpsa if requested
        if (stage== "qpsa:sign"):
          sign_with_qpsa(tosign_fname, cert_dir)

        #sign with cass if requested
        if (stage=="cass:sign"):
          sign_with_cass(tosign_fname, cert_dir)

      #package the files if requested
      if ((stage == "postsign") or (stage== "qpsa:sign") or (stage=="cass:sign")):
        process_signing(cert_dir)

      #generate a non secure hashtable (i.e one that is not for signing) or
      #support encryptonly
      elif ((stage == "genhashtable") or (stage == "encryptonly")):
          if (image_format == 'htelf'):
              remove_hashtable_from_elf(source_image, elf_htremoved_fname)
              temp_fname = elf_htremoved_fname

          hash_seg_size = None
          last_phys_addr = None

          # Non secure ELF Files for normal boot (i.e no secure boot)
          mbn_tools.pboot_gen_elf(0, temp_fname, ht_hashes_fname,
                                  elf_out_file_name = elf_intermediate,
                                  secure_type = 'non_secure',
                                  hash_seg_max_size = None,
                                  last_phys_addr = None,
                                  append_xml_hdr = encrypt_elf)
          gen_dict = {'IMAGE_KEY_MBN_TYPE' : 'elf','IMAGE_KEY_IMAGE_ID' : image_type,}
          mbn_tools.image_header(0, gen_dict, ht_hashes_fname, ht_mbnhdr_fname,
                               'non_secure', elf_file_name = temp_fname)

          # Combine the non secure header and hashes
          concat_files (ht_nonsec_fname, [ht_mbnhdr_fname, ht_hashes_fname])

          if encrypt is True:
            if encrypt_elf is True:
              # ELF encryption
              encrypt_image(enc_2_fname, nonsec_enc_fname, encrypt_elf, ht_nonsec_fname)
            else:
              # MBN encryption
              mbn_tools.pboot_add_hash( 0, elf_intermediate, ht_nonsec_fname, nonsec_fname)
              encrypt_image(nonsec_fname, nonsec_enc_fname, False)
            log("FINAL NONSEC IMAGE IS: %s" % (nonsec_enc_fname))
          else:
            mbn_tools.pboot_add_hash( 0, elf_intermediate, ht_nonsec_fname, nonsec_fname)
            log("FINAL NONSEC IMAGE IS: %s" % (nonsec_fname))

    elif (image_format == 'bin'):
        if (stage == "qpsa:sign"):
          shutil.copyfile(source_image, tosign_fname)
          sign_with_qpsa(tosign_fname, cert_dir)
        elif (stage=="cass:sign"):
          sign_with_cass(source_image, cert_dir)
        else:
          raise RuntimeError, \
            "Unsupported stage %s for image_type = bin. Please set stage to cass:sign or qpsa:sign" % stage

    copy_to_final_output()

#----------------------------------------------------------------------------
# process_postsigncmd
# Process postsign cmds for single section
#----------------------------------------------------------------------------
def process_postsigncmd (section_name):
    global postsignconfig_filename
    global postsign_config
    global postsigncmd

    # No postsignconfig, just return
    if ((postsignconfig_filename == None) or (postsignconfig_filename == '')):
        return

    # Initialize signing config parser
    postsign_config = ConfigParser.SafeConfigParser()
    log("Reading postsign config file: %s\n" % postsignconfig_filename)
    postsign_config.read(postsignconfig_filename)

    # Only process if the section name is present
    if postsign_config.has_section(section_name):
        # Write gensecimage related config
        postsign_config.set(section_name, 'gensec_output_dir', outputdir.replace('\\', '/'))
        postsign_config.set(section_name, 'gensec_source_fname_base', source_fname_base.replace('\\', '/'))
        postsign_config.set(section_name, 'gensec_source_fname_ext', source_fname_ext.replace('\\', '/'))

        #read_postsigncmd_config(section_name)
        if (postsign_config.has_option(section_name,'cmd') == True):
            postsigncmd = postsign_config.get(section_name,'cmd')

        if postsigncmd is not None:
            cmds = postsigncmd.split(';')
            print cmds;
            for single_cmd in cmds:
                log("Running postsign command: %s" % (single_cmd))
                err = os.system(single_cmd)
                log("Result: %s" % (err))

#----------------------------------------------------------------------------
# usage
# Help on how to use this script
#----------------------------------------------------------------------------
def usage ():
  print "========================================================"
  print "-h, --help       prints this help"
  print "-v               prints version"
  print ""
  print "usage"
  print "gensecimage.py --stage=<stage name> --section=<section name> "
  print "               --config=<config filename>"
  print "               --signconfig=<signing config filename>"
  print "               --postsignconfig=<postsign config filename>"
  print "               --encrypt_keys=<encrypt key config>"
  print "   where <stage name> can be one of these:"
  print "      'qpsa:sign' - prepare the image for signing, sign with qpsa"
  print "                   and package the certs and signatures"
  print "      'cass:sign' - prepare the image for signing, sign with CASS"
  print "                   services and package the certs and signatures"
  print "      'presign'  - prepare the image for signing and stop"
  print "      'postsign' - package the signed certificates/signatures to"
  print "                   generate the final signed image"
  print "      'genhashtable' - generate a hashtable from the raw elf image"
  print "      'encryptonly' - encrypt image only without signing"
  print ""
  print "   where <section name> is:"
  print "      section in the 'gensecimage.cfg' file to use"
  print "      if <section name> is \"all\", all sections will be run."
  print ""
  print "   where <config filename> is:"
  print "      the name of the configuration file to use."
  print ""
  print "   where <signing config filename> is:"
  print "      the name of the signing attributes configuration file to use."
  print "      It is optional. If present, it will override the \"signconfig\""
  print "      attribute in the config file"
  print ""
  print "   where <postsign config filename> is:"
  print "      the name of the postsign configuration file to use, which contains"
  print "      the external postsign commands to run. It is optional. If present,"
  print "      it will override the \"postsignconfig\' attribute in the config file"
  print ""
  print "   where <encrypt key config> is:"
  print "      a key config file that is used for key store generation and encryption."
  print "      An example can be found in ssd/key_config.xml. "
  print "      This is optional and will default to the ssd/key_config.xml, which points"
  print "      to the provided test keys in ssd/keys"
  print ""
  print "e.g: gensecimage.py --stage=qpsa:sign --section=sbl1"
  print "                    --config=resources/8974_LA_gensecimage.cfg"
  print "     gensecimage.py --stage=qpsa:sign --section=all "
  print "                    --config=resources/8974_LA_gensecimage.cfg"
  print "     gensecimage.py --stage=qpsa:sign --section=all "
  print "                    --config=resources/8974_LA_gensecimage.cfg"
  print "                    --signconfig=resources/8974_LA_signingattr_qpsa.cfg"
  print "                    --postsignconfig=resources/8974_LA_postsigncmd.cfg"
  print ""
  print "========================================================"

def open_log_file(name):
    global log_fp
    log_fp = open_file(name, "wb")

def close_log_file():
    global log_fp
    log_fp.close()

def parse_args_and_validate(args):
    global section_name
    global stage
    global config_filename
    global log_fp
    global signingconfig_filename
    global postsignconfig_filename
    global key_config
    global cass_engine

    section_name = ''
    stage = ''
    config_filename = ''
    signingconfig_filename = None
    postsignconfig_filename = None
    key_config = ''
    cass_engine=None

    try:
      opts, remainder = getopt.getopt(args, "hv", ["help", "stage=", "section=", "config=", "signconfig=", "postsignconfig=", "encrypt_keys=", "cass_engine="])
    except getopt.GetoptError, err:
      # print help information and exit:
      print str(err) # will print something like "option -a not recognized"
      usage()
      sys.exit(2)
    if len(sys.argv) < 2:
      usage()
      sys.exit(2)
    for opt, arg in opts:
      if opt in ("-h", "--help"):
        usage()
        sys.exit()
      if opt in ("-v"):
        print "gensecimage " + CURRENT_VERSION
        sys.exit()
      elif opt in ("--section"):
        if (section_name == ''):
          section_name = arg
          log("Found argument for section %s" % (section_name))
        else:
          log("Can't have multiple section arguments!")
      elif opt in ("--stage"):
        if (stage == ''):
          stage= arg
          log("Found argument for stage %s" % (stage))
        else:
          log("Can't have multiple stage arguments!")
      elif opt in ("--config"):
        if (config_filename == ''):
          config_filename = arg
          log("Found argument for config file %s" % (config_filename))
        else:
          log("Can't have multiple config arguments!")
      elif opt in ("--signconfig"):
        if (signingconfig_filename == None):
          signingconfig_filename = arg
          log("Found argument for signing config file %s" % (signingconfig_filename))
        else:
          log("Can't have multiple signing config arguments!")
      elif opt in ("--postsignconfig"):
        if (postsignconfig_filename == None):
          postsignconfig_filename = arg
          log("Found argument for postsign config file %s" % (postsignconfig_filename))
        else:
          log("Can't have multiple postsign config arguments!")
      elif opt in ("--encrypt_keys"):
         if (key_config == ''):
            key_config = arg
            log("Found argument for key path %s" % (key_config))
         else:
            log("Can't have multiple key paths!")
      elif opt in ("--cass_engine"):
        if (cass_engine == None):
          cass_engine = arg
          log("Found argument for cass engine %s" % (cass_engine))
        else:
          log("Can't have multiple cass engine arguments!")

    # ensure valid arguments are present
    if stage not in supported_stages:
      log("Stage = %s. Missing stage to proceed.\n" % stage)
      usage()
      raise RuntimeError, "Stage = %s. Missing stage to proceed." % stage
    if (section_name == ''):
      log("Missing section name\n")
      usage()
      raise RuntimeError, "Missing section name"
    if (config_filename == ''):
      log("Missing config file\n")
      usage()
      raise RuntimeError, "Missing config file"

    if not os.path.isfile(config_filename):
       log("Config file does not exist:%s" % (config_filename))
       raise RuntimeError, "Config file does not exist:" + config_filename

def str2bool(v):
  return v.lower() in ("yes", "true", "t", "1")

#----------------------------------------------------------------------------
# pboot_ins_serial_to_hashtable
#----------------------------------------------------------------------------
def pboot_ins_serial_to_hashtable(hash_out_file_name, elf_out_file_name,
                                  is_sha256_algo = True, serial_number = None):
    # If serial supplied, write it to hash file in first
    if serial_number is not None and elf_out_file_name is not None:
        if (is_sha256_algo is True):
            mbn_tools.MI_PROG_BOOT_DIGEST_SIZE = 32

        #Open hash table file for reading
        hash_out_fp = mbn_tools.OPEN(hash_out_file_name, "rb")

        #Save current hash table data to string
        orig_hash_data = hash_out_fp.read()
        hash_out_fp.close()

        serial_packed=struct.pack('L', serial_number)

        # Write all data from before the first hash segment to new_hash_data
        # string then append the first hash segment with the serial number
        # to overwrite it
        new_hash_data=orig_hash_data[:mbn_tools.MI_PROG_BOOT_DIGEST_SIZE]
        new_hash_data+=serial_packed

        # Write all original data from after the serial number insertion
        # to new_hash_data string
        new_hash_data+=orig_hash_data[(mbn_tools.MI_PROG_BOOT_DIGEST_SIZE+len(serial_packed)):]

        #Open hash table file for writing and write new data, then close file
        hash_out_fp = mbn_tools.OPEN(hash_out_file_name, "wb")
        hash_out_fp.write(new_hash_data)
        hash_out_fp.close()

    return

#----------------------------------------------------------------------------
# pboot_gen_elf_cust
# customized version of pboot_gen_elf to insert serial number into first
# hashtable entry of elf
#----------------------------------------------------------------------------
def pboot_gen_elf_cust(env, elf_in_file_name,
                            hash_out_file_name,
                            elf_out_file_name,
                            secure_type = 'non_secure',
                            hash_seg_max_size = None,
                            last_phys_addr = None,
                            append_xml_hdr = False,
                            is_sha256_algo = True,
                            cert_chain_size_in = CERT_CHAIN_ONEROOT_MAXSIZE,
                            serial = None):

    mbn_tools.pboot_gen_elf(env = env, elf_in_file_name = elf_in_file_name,
                                       hash_out_file_name = hash_out_file_name,
                                       elf_out_file_name = elf_out_file_name,
                                       secure_type = secure_type,
                                       hash_seg_max_size = hash_seg_max_size,
                                       last_phys_addr = last_phys_addr,
                                       append_xml_hdr = append_xml_hdr,
                                       is_sha256_algo = is_sha256_algo,
                                       cert_chain_size_in = cert_chain_size_in)

    pboot_ins_serial_to_hashtable(hash_out_file_name = hash_out_file_name,
                                  elf_out_file_name = elf_out_file_name,
                                  is_sha256_algo = is_sha256_algo,
                                  serial_number = serial)
    return

#----------------------------------------------------------------------------
# HELPER FUNCTIONS END
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
# MAIN SCRIPT BEGIN
#----------------------------------------------------------------------------
def main():
  global log_fp
  global outputdir
  global config
  global section_name
  global stage
  global config_filename

  filelock_enable = str2bool(os.environ.get('GENSECIMAGE_LOCK_ENABLE', "true"))
  if filelock_enable:
      lock = FileLock(os.path.join(gensecimage_dir, "gensecimage.lock"))
  num_retry_remaining = os.environ.get('GENSECIMAGE_NUM_RETRY_LOCK', failobj=10)
  while num_retry_remaining > 0:
      num_retry_remaining = num_retry_remaining-1
      if filelock_enable is False or lock.acquire():
          try:
            # open the log file
            open_log_file("gensecimage.log")

            # get the command line arguments
            parse_args_and_validate(sys.argv[1:])

            log("GENSECIMAGE_DIR = %s" % gensecimage_dir)
            log("SSD_DIR = %s" % ssd_dir)
            log("lock_enable = %d" % filelock_enable)

            # Initialize config parser
            init_config_parser(config_filename)

            # Read default config to load signing config
            read_config_default(config)

            # see if we need to execute all
            if (section_name == 'all'):
                section_list = config.sections()
            else:
                section_list = [section_name]

            for each_section in section_list:
                process_section(each_section, stage)
                if stage in postsigncmd_supported_stages:
                    process_postsigncmd(each_section)

            break

          except Exception, e:
            log("ERROR: {0}".format(e))
            sys.exit(2)

          finally:
            close_log_file()
            if filelock_enable:
                lock.release()

      else:
          if (num_retry_remaining == 0):
            print "gensecimage cannot acquire lock " + os.path.join(gensecimage_dir, "gensecimage.lock")
            sys.exit(2)
          time.sleep(2)
          print "try create "  + os.path.join(gensecimage_dir, "gensecimage.lock")

if __name__ == "__main__":
  main()

#----------------------------------------------------------------------------
# MAIN SCRIPT END
#----------------------------------------------------------------------------
