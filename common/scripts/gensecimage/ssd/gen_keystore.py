#============================================================================
#
#
#
# GENERAL DESCRIPTION
#    Contains methods to parse Keys & generate keystore
#
# Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#----------------------------------------------------------------------------
#
#  $Header: //source/qcom/qct/core/securemsm/Tools/rel/gensecimage/build_integration/gensecimage/ssd/gen_keystore.py#1 $
#  $DateTime: 2013/12/07 08:21:17 $
#  $Author: yliong $
#  $Change: 4918860 $
#
#                      EDIT HISTORY FOR FILE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
# when       who       what, where, why
# --------   ---       ------------------------------------------------------
# 12/14/09    hv       Initial revision
#
#============================================================================
import ConfigParser
import os
import binascii
import shutil
import stat
import io
import key_config_parser

#----------------------------------------------------------------------------
# Global values
#----------------------------------------------------------------------------
E_SUCCESS = 0
E_FAIL = 1
SSD_FS_EMMC_BLOCK_SIZE = 512

def get_file_path(line):

    start = 1 +     line.find('>')
    end = line.find('<', start)
    return line[start:end]


def _to_hex_string(int_):

    hex_values = { 0:  '0', 1:  '1',  2: '2',  3: '3',
                   4:  '4', 5:  '5',  6: '6',  7: '7',
                   8:  '8', 9:  '9', 10: 'a', 11: 'b',
                   12: 'c', 13: 'd', 14: 'e', 15: 'f' }

    A = int_ / 4096
    B = int_ % 4096

    C = B / 256
    D = B % 256

    E = D / 16
    F = B % 16
    return (hex_values.get(E) + hex_values.get(F) +
            hex_values.get(A) + hex_values.get(C) + "0000")

def main():

    # setup the path for working directories
    SSD_TOOLS_DIR = os.getcwd()

    keystore_fn = os.path.join("keys", "keystore.dat")

    keystore = key_config_parser.get_ssd_keystore("key_config.xml")

    # open output file
    if os.path.exists(keystore_fn):
      os.chmod(keystore_fn,
               stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO | stat.S_IWRITE)
      os.remove(keystore_fn)

    keystore_fp = open(keystore_fn, "wb")

    zero_buf = ''

    # TODO: remove zeros.dat
    # open and read the zero buf file
    with open("zeros.dat", "rb") as zb_fo:
        zero_buf = zb_fo.read()

    if len(keystore) > 10:
        print("Error: Too many keys in keyfile (" + len(keystore) + ")")

    dirbuf = io.BytesIO()
    keybuf = io.BytesIO()

    # write zeros for the magic number (version)
    dirbuf.write(zero_buf[0:4])

    # write the number of keys (4 bytes)
    dirbuf.write(binascii.unhexlify(_to_hex_string(len(keystore))))

    # the directory contains an entry for the directory itself.
    dir_key_start = ''

    # 10 keys is the max size for the directory.
    # Zero out all entries
    for ii in range(10):
        # key metadata size
        dirbuf.write(zero_buf[0:4])

        # key_id is sha 256
        dirbuf.write(zero_buf[0:32])

        # save the offset in the directory where the key info should
        # start
        if (ii == 0):
            dir_key_start = dirbuf.tell()

    # Save this for later.  Arguably better than seeking to the end,
    # then calling tell again?
    dirbuf_len = dirbuf.tell()

    dirbuf.seek(dir_key_start)

    for key in keystore:
        # open file
        keyid_fn = os.path.normpath(key[1])

        print("Reading key_id file: " + os.path.abspath(keyid_fn))
        keyid_fp = open(keyid_fn, "rb")

        # read file
        keyid_buf = keyid_fp.read()
        keyid_fp.close()

        # write key metadata (4 bytes)
        dirbuf.write(key[2])

        # key id must be less than 32 bytes
        if len(keyid_buf) > 32:
            print("Key ID not the correct length: " +
                  os.path.normpath(keyid_fn))
            # TODO: return an error

        # write key id to keystore directory
        dirbuf.write(keyid_buf)

        # pad the keyid if necessary
        if len(keyid_buf) < 32:
            dirbuf.write(zero_buf[0:32 - len(keyid_buf)])

        ###########

        # Encrypted Message Version: HMAC-256 AES-128
        keybuf.write("\x01\x00\x00\x00")

        # add space for auth_tag
        keybuf.write(zero_buf[0:32])

        key_fn = os.path.normpath(key[0])

        # write the key size (4 bytes)
        keybuf.write(binascii.unhexlify(
                _to_hex_string(os.path.getsize(key_fn))))

        # add space for IV
        keybuf.write(zero_buf[0:16])

        # open file
        print("Reading key file: " + os.path.abspath(key_fn))
        key_file_fp = open(key_fn, "rb")

        # read file
        key_file_buf = key_file_fp.read()
        key_file_fp.close()

        # write plaintext key file to output file
        keybuf.write(key_file_buf)

    direntrybuf = io.BytesIO()

    # Encrypted Message Version: HMAC-256 AES-128
    direntrybuf.write("\x01\x00\x00\x00")

    # add space for auth_tag
    direntrybuf.write(zero_buf[0:32])

    # write the key size (4 bytes)
    direntrybuf.write(binascii.unhexlify(_to_hex_string(dirbuf_len)))

    # add space for IV
    direntrybuf.write(zero_buf[0:16])

    # Write directory
    direntrybuf.write(dirbuf.getvalue())

    headerbuf = io.BytesIO()

    headerbuf.write("ssdksimg")

    # Write header, directory entry and key entries to output file
    keystore_fp.write(headerbuf.getvalue())
    keystore_fp.write(direntrybuf.getvalue())
    keystore_fp.write(keybuf.getvalue())

    # close file
    keystore_fp.close()

if __name__ == "__main__":
    main()
