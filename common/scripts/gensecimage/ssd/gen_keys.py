#============================================================================
#
# gen_keys.py
#
# GENERAL DESCRIPTION
#    Contains methods to parse Keys & generate keystore
#
# Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#----------------------------------------------------------------------------
#
#  $Header: //source/qcom/qct/core/securemsm/Tools/rel/gensecimage/build_integration/gensecimage/ssd/gen_keys.py#1 $
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
# 02/14/13   pre       Initial revision
#
#============================================================================
import getopt
import sys
import os
import subprocess as SP
from subprocess import Popen
from subprocess import PIPE
import tempfile
import hashlib
from binascii import b2a_base64
import shutil

def usage():
   print(
"===============================================================================\n"
" This tool generates the following files in the given key folder:\n"
" [key_dir]/dvc_aes/aes.dat\n"
" [key_dir]/dvc_aes/aes_id.dat\n"
" [key_dir]/dvc_rsa/rsa_pkcs8_pr_key.dat\n"
" [key_dir]/dvc_rsa/rsa_pub_key.dat\n"
" [key_dir]/dvc_rsa/key_id.dat\n"
" [key_dir]/oem_rsa/rsa_pkcs8_pr_key.dat\n"
" [key_dir]/oem_rsa/rsa_pub_key.dat\n"
" [key_dir]/oem_rsa/key_id.dat\n"
"===============================================================================\n"
"Parameters:\n"
"--key_dir       Where to output keys. (Required)\n"
"--clobber       Whether to overwrite existing keys in the given output\n"
"                folder. (Optional)\n"
"=============================================================================="
)

def generate_aes_keys(folder):
    saved_dir = os.getcwd()
    os.chdir(folder)

    print("========================================" +
          "=======================================")
    print("Generating AES 128 key in " + folder)
    print("========================================" +
          "=======================================")

    aes_key_fo = open("aes.dat", "wb")
    rand_buf = os.urandom(16)

    aes_key_fo.write(rand_buf)
    aes_key_fo.close()

    aes_key_id_fo = open("aes_id.dat", "wb")
    rand_buf = os.urandom(32)

    aes_key_id_fo.write(rand_buf)
    aes_key_id_fo.close()

    os.chdir(saved_dir)


def generate_rsa_keys(folder):
    saved_dir = os.getcwd()
    os.chdir(folder)

    print("========================================" +
          "=======================================")
    print("Generating RSA 2048 Public/Private keypair in " + folder)
    print("========================================" +
          "=======================================")

#    dump_stderr = open(os.devnull, "w")
    dump_stderr = sys.stderr

    rand_buf = os.urandom(2048)
    rand_fd,rand_fn = tempfile.mkstemp()
    rand_fo = os.fdopen(rand_fd, "wb")

    rand_fo.write(rand_buf)
    rand_fo.close()

    # Generate RSA Private key in PEM format
    p = Popen(["openssl", "genrsa", "2048", "-rand", rand_fn],
              stdout=PIPE, stderr=dump_stderr)
    pr_key_pem = p.communicate()[0]

    # Extract the Public key (in PEM) from private key (in PEM)
    p = Popen(["openssl", "rsa", "-pubout"],
              stdin=PIPE, stdout=PIPE, stderr=dump_stderr)
    pub_key_pem = p.communicate(pr_key_pem)[0]

    # Convert the PEM keys to DER keys
    p = Popen(["openssl", "rsa", "-outform", "DER"],
              stdin=PIPE, stdout=PIPE, stderr=dump_stderr)
    pr_key_der = p.communicate(pr_key_pem)[0]

    p = Popen(["openssl", "rsa", "-pubin", "-outform", "DER"],
              stdin=PIPE, stdout=PIPE, stderr=dump_stderr)
    pub_key_der = p.communicate(pub_key_pem)[0]

    # Convert the PEM RSA keys into PKCS8 PEM format
    p = Popen(["openssl", "pkcs8", "-topk8", "-inform", "DER",
               "-outform", "DER", "-nocrypt"],
              stdin=PIPE, stdout=PIPE, stderr=dump_stderr)
    pr_key_pkcs8_der = p.communicate(pr_key_der)[0]

    pub_key_digest = hashlib.sha256(pub_key_der).digest()

    with open("key_id.dat", "wb") as pub_key_id_fo:
        pub_key_id_fo.write(pub_key_digest)

    with open("rsa_pub_key.der", "wb") as pub_key_fo:
        pub_key_fo.write(pub_key_der)

    with open("rsa_pkcs8_pr_key.der", "wb") as pr_key_fo:
        pr_key_fo.write(pr_key_pkcs8_der)

    os.chdir(saved_dir)


def main():

    try:
        opts, args = getopt.getopt(sys.argv[1:], '', ["key_dir=", "clobber"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err)
        usage()
        sys.exit(2)

    keydir = '.'
    clobber = False

    if (len(sys.argv) < 2):
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if (opt == "--key_dir"):
            keydir = arg
        elif (opt == "--clobber"):
            clobber = True

    if (keydir != '.' and  (not os.path.exists(keydir) or
                            not os.path.isdir(keydir))):
        os.makedirs(keydir)

    os.chdir(keydir)

    if (os.path.exists("dvc_aes") and os.path.isdir("dvc_aes")):
        if (clobber):
            shutil.rmtree("dvc_aes")
        else:
            print("Error: Output directory already exists")
            exit(1)
    if (os.path.exists("dvc_rsa") and os.path.isdir("dvc_rsa")):
        if (clobber):
            shutil.rmtree("dvc_rsa")
        else:
            print("Error: Output directory already exists")
            exit(1)
    if (os.path.exists("oem_rsa") and os.path.isdir("oem_rsa")):
        if (clobber):
            shutil.rmtree("oem_rsa")
        else:
            print("Error: Output directory already exists")
            exit(1)

    os.makedirs("dvc_aes")
    generate_aes_keys("dvc_aes")
    os.makedirs("dvc_rsa")
    generate_rsa_keys("dvc_rsa")
    os.makedirs("oem_rsa")
    generate_rsa_keys("oem_rsa")


if __name__ == "__main__":
    main()
