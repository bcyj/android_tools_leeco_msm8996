#! /usr/bin/python
#
#=============================================================================
#
# Secure Boot Certificate Generator
#
# Description:
# Creates the certificates and signature of an image using SHA1 or SHA256
#   hashing.
#
# Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#=============================================================================
#
#  $Header: //source/qcom/qct/core/securemsm/Tools/rel/gensecimage/build_integration/gensecimage/qpsa/qpsa.py#1 $
#  $DateTime: 2013/12/07 08:21:17 $
#  $Author: yliong $
#  $Change: 4918860 $
#                      EDIT HISTORY FOR FILE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
# when       who     what, where, why
# --------   ---     -------------------------------------------------------------------------
# 08/30/13   ricd    Changed DEBUG attribute for attestation cert ot be optional
# 08/09/13   ricd    Add CRASH_DUMP support for signing TZ image (but not restricted)
# 02/19/13   yl      Add APP_ID support for signing TZ apps
# 02/19/13   yl      Baseline from QDST 04/06/12 version
#
#=============================================================================

import os, sys
import time
import re
import binascii
import subprocess as sub
import shutil
import shlex

CURRENT_VERSION = '2.1.2.0'

internal_openssl_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..","openssl","win32")

if sys.platform == "win32" and os.path.isfile(os.path.join(internal_openssl_dir,"openssl.exe")):
   default_openssl_dir = internal_openssl_dir
else:
   default_openssl_dir=""

OPENSSL_CMD = os.path.join(os.environ.get('OPENSSL_DIR', default_openssl_dir), "openssl")

debug={'level':0}

def usage():
    print "\n\nERROR: No image was specified, see below for examples"
    print "python qpsa.py -v            prints version"
    print "python qpsa.py hash_alg=SHA1 image=C:\\Dropbox\\secureboot\\qcomdevsigningtool\\dbl_SCTUSS_nonsec.mbn"
    print "python qpsa.py hash_alg=SHA1 image=images/dbl_SCTUSS_nonsec.mbn xml=myconfig.xml"
    print "python qpsa.py hash_alg=SHA256 image=dbl_SCTUSS_nonsec.mbn SW_ID=0x0000000000000002"
    print "python qpsa.py hash_alg=SHA256 image=dbl_SCTUSS_nonsec.mbn SW_ID=0x0000000000000003"
    print "python qpsa.py hash_alg=SHA256 image=dbl_SCTUSS_nonsec.mbn SW_ID=0x0000000000000004 CN=\"Your name here\""
    print "python qpsa.py hash_alg=sha256 image=.\dbl_nonsec.mbn sw_id=0x12 msm_part=0x004200E1 OEM_ID=0x0001 MODEL_ID=0x0002"

    print "\nOther options:"
    print "-----------------"
    print "exponent=3                       : Takes value 3 or 65537, sets the exponent used in all RSA key generation"
    print "rootcakey=rootkey_file.key       : Specify root CA key file (.key)"
    print "rootcacert=rootcert_file.crt     : Specify root CA certificate as .cer (DER format)"
    print "attestcakey=attestkey_file.key   : Specify attest CA key file (.key)"
    print "attestcacert=attestcert_file.crt : Specify attest CA certificate in .cer (DER format)"
    print "certchainsize=3                  : Size of cert chain to sign with, default is 3 (sign with attest, signed by attestCA, signed by root)"
    print "certchainsize=2                  : Size of cert chain set to 2 (sign with attest, signed by root)"
    #print "certchainsize=1                  : Size of cert chain set to 1 (sign with root certificate)"
    print "app_id=0x0000000000012345        : Set APP_ID for TZApps signing"
    print "debug=0x1234abcd00000001         : Set DEBUG to enable debug support on target"
    print "crash_dump=0x1234abcd00000001    : Set CRASH_DUMP to enable crash dump support on target"
    print "tcg_min=0x0001E240 or 123456     : Specify minimum TCG value (8 hex digits or 32 bit decimal)"
    print "tcg_max=0x0001E24B or 123467     : Specify maximum TCG value (8 hex digits or 32 bit decimal)"
    print "tcgvalidate=yes/no               : Turn on/off TCG validation. Default to no if missing"
    print "attestcert_sha1sign=yes/no       : Turn on/off SHA1 algorithm to sign attestation certificate. "
    print "                                   Default to no (i.e. SHA256 signed) if missing. "
    print "rootcerts_dir=alt_resources/multirootcert_sha256/pki/"
    print "                                 : Specify the directory to load multiple root certificates. "

    print "\nNOTE: The output is stored in the same directory as the input image (actually a \"cert\" sub folder)"
    print "\nNOTE: If sha1 is not specified the default is sha256"

def external_call(command, capture_output=True, print_capture=False, posix=None, shell=False, raise_except=False):
    errors = None
    output = None
    cmd_list = None
    log_str=None

    if posix==None:
        if os.name.lower()=='nt':
            posix=False
        else:
            posix=True

    #Prepare a list conversion of the command line string if needed
    if capture_output:
        if isinstance(command, basestring):
            log_str="\nqpsa.external_call - Calling SYSTEM with\n\"{0}\"\n".format(command)

            import shlex
            cmd_list=shlex.split(command,posix=posix)

            if debug['level']>=1:
                debug_str='DEBUG: qpsa.external_call - shlex.split returned "{0}"'.format(repr(cmd_list))
                print debug_str
                LOG.write(debug_str)

            cmd_string=command
        elif isinstance(command, list) or isinstance(command, tuple):
            cmd_string=' '.join(command)
            log_str="\nqpsa.external_call - Calling SYSTEM with\n\"{0}\"\n".format(command)
            cmd_list=command
            log_str+="cmd_list is '{0}'".format(repr(cmd_list))
        else:
            error_str='ERROR: qpsa.external_call - command must be either a basestring, a list, or a tuple, or a derivitive of one of those'
            log_and_leave(error_str, raise_except, RuntimeError(error_str))

    if debug["level"]>=1:
        if log_str==None:
            pass
        else:
            print 'DEBUG: '+log_str
            LOG.write('DEBUG: '+log_str)
        debug_str="DEBUG: qpsa.external_call - Doing {0}system call: '{1}'".format('(captured) ' if capture_output else '',cmd_string)
        print debug_str
        LOG.write(debug_str)

    #Execute command
    if capture_output:
        try:
            p = sub.Popen(cmd_list, stdout=sub.PIPE, stderr=sub.PIPE, shell=shell)
        except Exception, e:
            error_str="ERROR: qpsa.external_call - Error executing command '{0}'. Reason: Error starting subprocess '{1}'".format(cmd_string,e)
            log_and_leave(error_str, raise_except, e)

        try:
            (output, errors) = p.communicate() #blocks until process finishes
        except Exception, e:
            error_str="ERROR: qpsa.external_call - Error executing command '{0}'. Reason: Error communicating with subprocess. '{1}'".format(cmd_string,e)
            log_and_leave(error_str, raise_except, e)

        if debug['level']>=1:
            debug_str="DEBUG: qpsa.external_call - Command output>>>\n{0}<<<".format(output)
            if errors==None or errors=='':
                pass
            else:
                debug_str+="\nDEBUG: qpsa.external_call - Process stderr: >>>\n{0}<<<".format(errors)
            print debug_str
            LOG.write(debug_str)

        try:
            return_code=p.poll()
        except Exception, e:
            error_str="ERROR: qpsa.external_call - Error executing command '{0}'. Reason: Error polling subprocess for return code. '{1}'\n".format(cmd_string,e) + \
                      'Process stderr: {0}'.format(errors)
            log_and_leave(error_str, raise_except, e)
        if debug['level']>=1:
            debug_str="DEBUG: qpsa.external_call - Return code from app was '{0}'".format(return_code)
            print debug_str
            LOG.write(debug_str)

        if return_code==0:
            pass
        else:
            error_str="ERROR: qpsa.external_call - Error executing command '{0}'. Reason: Subprocess returned a non-zero exit code. Stdout:\n".format(cmd_string) + \
                      "{0}\n".format(output) + \
                      "Process stderr:\n" + \
                      "{0}".format(errors)
            log_and_leave(error_str, raise_except, RuntimeError(error_str))

        if print_capture:
            if not output is None:
                print "Result: %s" % output
        if (not errors is None) and (not errors == ""):
            print "Process stderr: %s" % errors
    else:
        try:
            os.system(command)
        except Exception, e:
            error_str="Error executing command '%s'. Reason: %s" % (str(command), e)
            #clean_up()
            log_and_leave(error_str, raise_except, e)

    return output

def unzip(filename):
    from zipfile import ZipFile
    z = ZipFile(filename, 'r')
    names = z.namelist()
    for name in names:
        outfile = file(name, 'wb')
        print "Inflating %s." % name
        LOG.write("Inflating %s." % name)
        outfile.write(z.read(name))
        outfile.close()
    z.close()

def zip_files(zip_name, file_list, arc_name_list = None):
    from zipfile import ZipFile
    z = ZipFile(zip_name, 'w')
    for i in range(0, len(file_list)):
        if arc_name_list is not None:
            if len(arc_name_list) != len(file_list):
                print "Archive name list has length = %d while zip file list has length = %d.\n" % (len(arc_name_list), len(file_list))
                LOG.write("Archive name list has length = %d while zip file list has length = %d.\n" % (len(arc_name_list), len(file_list)))
                LOG.close()
                sys.exit(1)
            LOG.write("Zipping %s to %s.\n" % (file_list[i], arc_name_list[i]))
            z.write(file_list[i], arc_name_list[i])
        else:
            print "Zipping %s." % file_list[i]
            LOG.write("Zipping %s." % file_list[i])
            z.write(file_list[i])
    z.close()
    print "Created file %s." % zip_name
    LOG.write("Created file %s." % zip_name)

def MySystem(temp, subtext="", posix=None, shell=False):
    i = 0

    if isinstance(temp, basestring):
        print "\n%s" % os.getcwd()
        log_str="\nCalling SYSTEM with\n\"%s\" %s\n" % (temp, subtext )
    elif isinstance(temp, list) or isinstance(temp, tuple):
        print "\n%s" % os.getcwd()
        log_str="\nCalling SYSTEM with\n\"%s\" %s\n" % (repr(temp), subtext )
    else:
        error_str='ERROR: qpsa.MySystem - temp must be either a basestring, a list, or a tuple, or a derivitive of one of those'
        log_and_leave(error_str)
    print log_str
    LOG.write(log_str)
    #os.system(temp)
    output=external_call(command=temp, capture_output=True, print_capture=True, posix=posix, shell=shell)

    try:
        OUT = open("temp.txt",'wb')
    except Exception, e:
        print "\nCan't open temp.txt for writing: %s\n" % e
        sys.exit(1)
    try:
        OUT.write(output)
    except Exception, e:
        print "\nCan't write to temp.txt: %s\n" % e
        sys.exit(1)
    try:
        OUT.close()
    except Exception, e:
        print "Can't close temp.txt: %s\n" % e
        sys.exit(1)

    LOG.write(output)

def path_to_posix_shlex_ready(cmd):
    new_cmd=cmd.replace("\\","\\\\")
    return new_cmd

def check_file_exists(filename):
    return os.path.isfile(filename) and os.path.getsize(filename) > 0

def gen_rsa_key(outfile, exponent=3, bits=2048):
    MySystem("%s genrsa -out %s %s %d" % (OPENSSL_CMD, outfile, exponent==3 and "-3" or "", bits))

def self_signed_cert(keyfile, outfile, subj='/C=US/ST=California/L=San Diego/OU=General Use Test Key (for testing only)/OU=CDMA Technologies/O=QUALCOMM/CN=QCT Root CA 1', configfile='opensslroot.cfg', days=7300, setserial=1):
    MySystem("%s req -new -key %s -x509 -out %s -subj \"%s\" -days %d -set_serial %d -config %s -sha256" %
        (path_to_posix_shlex_ready(OPENSSL_CMD), keyfile, outfile, subj, days, setserial, configfile),
        posix=True)

def gen_signing_request(keyfile, outfile,
                subj='/C=US/ST=CA/L=San Diego/OU=CDMA Technologies/O=QUALCOMM/CN=QUALCOMM Attestation CA',
                configfile='opensslroot.cfg', days=7300):
    MySystem("%s req -new -key %s -out %s -subj \"%s\" -days %d -config %s" % (path_to_posix_shlex_ready(OPENSSL_CMD), keyfile, outfile, subj, days, configfile), posix=True)

def sign_request(infile, ca_file, ca_key_file, outfile, extfile, days=7300, setserial=5):
    MySystem("%s x509 -req -in %s -CA %s -CAkey %s -out %s -set_serial %d -days %d -extfile %s -sha256" % (OPENSSL_CMD, infile, ca_file, ca_key_file, outfile, setserial, days, extfile))

def pem_der_conversion(pem_to_der, infile, outfile):
    MySystem("%s x509 -inform %s -in %s -outform %s -out %s" % (OPENSSL_CMD, (pem_to_der and "PEM" or "DER"), infile, (pem_to_der and "DER" or "PEM"), outfile))

def chain_verify(ca_file, filename):
    MySystem("%s verify -CAfile %s %s" % (OPENSSL_CMD, ca_file, filename))

def generate_root_ca_files(rootcakey_file='qpsa_rootca.key', rootcacert_file='qpsa_rootca.crt',
                           rootcacert_der_file='qpsa_rootca.cer'):
    gen_rsa_key(rootcakey_file)
    self_signed_cert(keyfile="qpsa_rootca.key", outfile=rootcacert_file, subj='/C=US/ST=California/L=San Diego/OU=General Use Test Key (for testing only)/OU=CDMA Technologies/O=None/CN=Generated Root CA 1')
    pem_der_conversion(pem_to_der=True, infile=rootcacert_file, outfile=rootcacert_der_file)

def generate_attestation_ca_files(attestcakey_file='qpsa_attestca.key',
                                  attestcacert_file='qpsa_attestca.crt',
                                  attestcacert_der_file='qpsa_attestca.cer'):
    gen_rsa_key(attestcakey_file)
    gen_signing_request(keyfile="qpsa_attestca.key", outfile="qpsa_attestca.csr", subj='/C=US/ST=CA/L=San Diego/OU=CDMA Technologies/O=None/CN=Generated Attestation CA')
    sign_request("qpsa_attestca.csr", "qpsa_rootca.crt", "qpsa_rootca.key", attestcacert_file, "v3.ext")
    pem_der_conversion(pem_to_der=True, infile=attestcacert_file, outfile=attestcacert_der_file)

def GeneratePhaseOutput(XORwith, S, hash, filename):

    for i in range(len(XORwith)):
        S[i] = S[i] ^ XORwith[i]

    print "H(code_image) = 0x%s\n" % hash
    sys.stdout.write("            S = 0x")
    LOG.write("H(code_image) = 0x%s\n" % hash)
    LOG.write("            S = 0x")
    for i in range(len(S)):
        sys.stdout.write("%.2X" % S[i])
        LOG.write("%.2X" % S[i])

    ## combine these and write a binary file
    try:
        OUTF = open(filename, "wb")
    except Exception, e:
        print "\nCan't open %s for writing: %s\n" % (filename,  e)
        sys.exit(1)

    for i in range(len(S)):
        OUTF.write(chr(S[i]))

    for i in range(0, len(hash), 2):  ## 40 string characters
        temp = hash[i:i+2]
        OUTF.write(chr(int("0x%s" % temp, 16)))
        #print OUTF pack("c",$pad->[$i])

    try:
        OUTF.close()
    except Exception, e:
        print "Can't close %s: %s\n" % (filename, e)
        sys.exit(1)

    print "\nStored in %s\n" % filename
    LOG.write("\nStored in %s\n" % filename)


def getData(FileName):
    buffer = None

    ## OPEN FILE ---------------------------------------------------------------------------------------

    try:
        IN = open(FileName, "rb")
    except Exception, e:
        print "\nCan't open %s for reading: %s\n" % (FileName, e)
        sys.exit(1)

    for i in range(8):
        buffer = IN.read(1)   # offset is 0
        PrintByte(buffer)

    print "  "
    for i in range(20):
        buffer = IN.read(1)   # offset is 0
        PrintByte(buffer)
    print "\n"
    for i in range(36):
        buffer = IN.read(1)   # offset is 0
        PrintByte(buffer)

    try:
        IN.close()
    except Exception, e:
        print "Can't close %s: %s\n" % (FileName, e)
        sys.exit(1)

    filesize = os.path.getsize(FileName)
    if filesize == 0:
        filesize = 1

    print "\n\nFile Size is %.2fB (%dbits)" % (filesize, filesize * 8)

    ## OPEN FILE ---------------------------------------------------------------------------------------
    ## end of GetData ##--------------------------------------------------------------------------


print "\n\n"

def PrintByte(buffer):
    print binascii.hexlify(buffer).upper(),


def GetCurrentTimeInSeconds():
   (year, month, dayOfMonth, hour, minute, second, dayOfWeek, dayOfYear, daylightSavings) = time.localtime()
   TimeInSeconds = hour*3600 + minute*60 + second


   ##print "\n\n-------------------------------------------------------------------------------------------------------"
   ##print "\nCurrent time is $TimeInSeconds seconds"
   ##print "\n-------------------------------------------------------------------------------------------------------\n\n"

   return TimeInSeconds


def Sha1(FileName):
    filesize = os.path.getsize(FileName)
    if filesize == 0:
        print "\n\nfilesize is 0, EXITING"
        CleanUp()
        LOG.close()
        sys.exit(1)

    print "\n\nSHA1 on \"%s\" of size %dBytes (%dbits)" % (FileName, filesize, filesize*8)
    MySystem("sha1sum %s" % FileName)

    ## temp.txt will look like this
    ## 64dcd3442435089474b845725dc9f746fef61e3e *dbl_SCTUSS_nonsec.mbn

    try:
        IN = open("temp.txt")
    except Exception, e:
        print "\nCan't open temp.txt for reading: %s\n" % e
        sys.exit(1);
    line = IN.readline()

    try:
        IN.close()
    except Exception, e:
        print "Can't close temp.txt: %s\n" % e
        sys.exit(1)

    temp = re.search(r'[\da-fA-F]{40}', line)
    if not temp is None:
        return temp.group(0)

    ## to get this far means hash didn't work
    print "\n\nsha1sum didn't work, are you sure OPENSSL is installed?"
    CleanUp()
    LOG.close()
    sys.exit(1);


def Sha256(FileName):
    filesize = os.path.getsize(FileName)
    if filesize == 0:
        print "\n\n\"%s\" filesize is 0, can't do Sha256 EXITING" % FileName
        LOG.write("\n\n\"%s\" filesize is 0, can't do Sha256 EXITING" % FileName)
        CleanUp()
        LOG.close()
        sys.exit(1)

    print "\nSHA256 on \"%s\" of size %dBytes (%dbits)\n" % (FileName, filesize,filesize * 8)
    LOG.write("\nSHA256 on \"%s\" of size %dBytes (%dbits)\n" % (FileName, filesize, filesize * 8))
    #$temp = "sha1sum $FileName > temp.txt"

    if "sha1".lower() in SigCalc["HASH_ALG"].lower():
        MySystem("%s dgst -sha1 %s" % (OPENSSL_CMD, FileName))
    else:
        MySystem("%s dgst -sha256 %s" % (OPENSSL_CMD, FileName))

    ## temp.txt will look like this
    ## 64dcd3442435089474b845725dc9f746fef61e3e *dbl_SCTUSS_nonsec.mbn

    try:
        IN = open("temp.txt")
    except Exception, e:
        print "\nCan't open temp.txt for reading: %s\n" % e
        sys.exit(1)

    line = IN.readline()

    try:
        IN.close()
    except Exception, e:
        print "Can't close temp.txt: %s\n" % e
        sys.exit(1)

    temp = re.search(r'([\da-fA-F]{64})', line)
    if not temp is None:
        return temp.group(0)

    ## to get this far means hash didn't work
    print "\n\nsha256 didn't work, are you sure OPENSSL is installed?"
    LOG.write("\n\nsha256 didn't work, are you sure OPENSSL is installed?")
    CleanUp()
    LOG.close()
    sys.exit(1)


def ParseDefaultsFromXML(FileName): #Recommended change: change to using XML library to parse XML objects
    try: #Recommended change: Use 'with' statement so we automatically close the filehandle when we go out of scope
        IN = open(FileName)
    except Exception, e:
        return #Recommended change: Add logging for this exception
    line = IN.readlines()
    try:
        IN.close()
    except Exception, e:
        print "Can't close %s: %s\n" % (FileName, e)
        sys.exit(1)

    global SigCalc, app_id
    NumLines = len(line)

    i = None

    # Start parsing the log file
    ## This below gets each line from the file at a time
    line_pattern_one = re.compile(r'<(.+)>0x([a-fA-F\d]+)<\/(.+)>') #Recommended change: regular expression should be careful to only
    line_pattern_two = re.compile(r'<(.+)>(.+)<\/(.+)>')            #use the dot operator when absolutely necessary

    for i in range(NumLines): #Recommended change: use the file contents as an iterator rather than for loop through indexed array
        match_one = line_pattern_one.search(line[i])
        match_two = line_pattern_two.search(line[i])

        if not match_one is None:
            #print line[i]

            if match_one.groups()[0] == match_one.groups()[2]:
                ## to be here means I have an XML tag like OEM_ID (which is in $1 and $3)
                ## and $2 contains a string of hex digits. Every 2 digits is 1 hex value for the array

                print "\n\tArgument \"%s\" updated to 0x%s" % (match_one.groups()[0], match_one.groups()[1])
                LOG.write("\n\tArgument \"%s\" updated to 0x%s" % (match_one.groups()[0], match_one.groups()[1]))

                if match_one.groups()[0] == "APP_ID":
                    app_id = match_one.groups()[1]

                if match_one.groups()[0] == "DEBUG":
                    attestcert[match_one.groups()[0].upper()] = match_one.groups()[1]
                    continue

                if match_one.groups()[0].upper() == "CRASH_DUMP":
                    attestcert[match_one.groups()[0].upper()] = match_one.groups()[1]
                    continue

                UpdateSigCalcHash(match_one.groups()[0],match_one.groups()[1])  ## key, input

                temp = []

                for j in range(0, len(match_one.groups()[1]), 2):
                    temp.append(int(match_one.groups()[1][j:j+2], 16))

                #$hash{$1} = @temp #This is perl code; what is it doing here?
                #$HoA{flintstones}[0] = "Fred"

                SigCalc[match_one.groups()[0]] = []   ## empty out old hash for new values

                SigCalc[match_one.groups()[0]].extend(temp)   ## $SigCalc["OEM_ID"] is an array
                ## $SigCalc["OEM_ID"][1] is the 2nd element of the array etc

                ## end of testing for HMAC

        elif not match_two is None:
            #print $line[$i]
            if (match_two.groups()[0] == "PART_NAME") or (match_two.groups()[0] == "HASH_ALG") or (match_two.groups()[0] == "SW_ID_NAME"):
                print "\n\tArgument \"%s\" updated to %s" % (match_two.groups()[0], match_two.groups()[1])
                LOG.write("\n\tArgument \"%s\" updated to %s" % (match_two.groups()[0], match_two.groups()[1]))
                SigCalc[match_two.groups()[0]]=match_two.groups()[1]
            elif match_two.groups()[0]==match_two.groups()[2]:
                print "\n\t**AttestCERT key (%s), updated to \"%s\""  % (match_two.groups()[0], match_two.groups()[1])
                LOG.write("\n\t**AttestCERT key (%s), updated to \"%s\""  % (match_two.groups()[0], match_two.groups()[1]))
                attestcert[match_two.groups()[0]] = match_two.groups()[1]

    ## end of GetData ##--------------------------------------------------------------------------


def MySub(var):
    var = var


def ArrayToHexString(a):
    LengthA = len(a)
    sz = ""

    for i in range(LengthA):
        sz = "%s%.2X" % (sz, a[i])

    return sz


def UpdateSigCalcHash(hashkey, hashinput):
    temp = []
    j = None

    for j in range(0, len(hashinput), 2):
        temp.append(int(hashinput[j:j+2], 16))

    #$hash{$1} = @temp
    #$HoA{flintstones}[0] = "Fred"

    SigCalc[hashkey] = []   ## empty out old hash for new values

    SigCalc[hashkey].extend(temp)   ## $SigCalc["OEM_ID"] is an array
    ## $SigCalc["OEM_ID"][1] is the 2nd element of the array etc


def PrintSigCalcHash():
    key = None
    value = None

    print "\n----------These values used for signature calculation------------"
    LOG.write("\n----------These values used for signature calculation------------")

    for key in SigCalc.keys():
        print "\nkey='%s'   \t" % key
        LOG.write("\nkey=%s   \t" % key)

        if key == "PART_NAME":
            print SigCalc[key]
            LOG.write(SigCalc[key])
            continue
        if key == "HASH_ALG":
            print SigCalc[key]
            LOG.write(SigCalc[key])
            continue
        if key == "SW_ID_NAME":
            print SigCalc[key]
            LOG.write(SigCalc[key])
            continue

        sys.stdout.write("0x")
        LOG.write("0x")

        for value in SigCalc[key]:
            sys.stdout.write("%.2X" % value)
            LOG.write("%.2X" % value)
            #print " " #TODO check this

    print "\n-----------------------------------------------------------------\n\n"
    LOG.write("\n-----------------------------------------------------------------\n\n")


def PrintAttestCertHash(attestcakey):
    key = None
    value = None

    print "\n----------These values used for attestation certificate------------"
    LOG.write("\n----------These values used for attestation certificate------------")

    for key in attestcert.keys():
        if key.lower() in ["PART_NAME".lower(),"HASH_ALG".lower(),"DEBUG".lower(),'CRASH_DUMP'.lower()]:
            continue

        print "\nkey: %s   \t" % key
        LOG.write("\n%s   \t" % key)

        print attestcert[key]
        LOG.write(attestcert[key])

    print "\n-----------------------------------------------------------------\n\n"
    LOG.write("\n-----------------------------------------------------------------\n\n")

    if certchainsize == 3:
        filesize = os.path.getsize(attestcakey)

        if filesize == 0:
            print "\n\nERROR: qpsa_attestca.key does not exist"
            print "\nWithout this KEY no image can be signed - EXITING"

            LOG.write("\n\nERROR: qpsa_attestca.key does not exist")
            LOG.write("\nWithout this KEY no image can be signed - EXITING")
            CleanUp()
            LOG.close()
            sys.exit(1)

    print "\n\nSUCCESS: the certificate chain exists"
    LOG.write("\n\nSUCCESS: the certificate chain exists")


def TestForOpenSSL():
    global OpenSSLInstalled

    ## Test if the version I need is installed
    response = ''
    try:
        response = external_call("%s version" % OPENSSL_CMD, raise_except=True)
    except:
        error_str="-"*78 + "\n" + \
                  "ERROR: OpenSSL was not found??\n\n"
        log_and_leave(error_str)

    print "OpenSSL was found: ", response

    for line in response.strip("\r").split("\n"):
        print "\t"+line

    ## Needs 0.9.8m or better, ex. OpenSSL 0.9.8r 8 Feb 2011 or  OpenSSL 1.0.1 14 Mar 2012
    line_pattern = re.compile(r'OpenSSL (\d+).(\d+).(\d+)(.{1})(.+)',flags=re.IGNORECASE)
    line_pattern_match = line_pattern.search(response)

    if not line_pattern_match is None:
        # DOUBT - Versions
        VersionA = int(line_pattern_match.groups()[0])  # 0
        VersionB = int(line_pattern_match.groups()[1])  # 9
        VersionC = int(line_pattern_match.groups()[2])  # 8
        VersionD = line_pattern_match.groups()[3]       # m

        if VersionA > 0:
                OpenSSLInstalled = 1
        elif VersionA == 0 and VersionB > 9:
                OpenSSLInstalled = 1
        elif VersionA == 0 and VersionB == 9 and VersionC > 8:
                OpenSSLInstalled = 1

        elif sys.platform=='win32':   ## Needs 0.9.8m or better
            if VersionA == 0 and VersionB == 9 and VersionC == 8 and VersionD > "l":
                    OpenSSLInstalled = 1
        else:                       ## Linux needs 0.9.8k or better
            if VersionA == 0 and VersionB == 9 and VersionC == 8 and VersionD > "j":
                    OpenSSLInstalled = 1

        if OpenSSLInstalled==1:
            print "\n\nSUCCESS: An OpenSSL version that can do SHA256 is installed\n"
            LOG.write("\n\nSUCCESS: An OpenSSL version that can do SHA256 is installed\n")
            if ShowSteps:
                sleep(2)
        else:
            print "\n\nERROR: Didn't find an OpenSSL version that can do SHA256 is installed\n"

    else:
        print "\nERROR: Didn't match the line pattern\n\n"

def InstallOpenSSL():
    print "\n\n-----------------------------------------------------"
    print "-------Verifying correct version of OpenSSL----------"
    print "---Qualcomm Platform Signing Application Code Signing----"
    print "-----------------------------------------------------"

    print "\n\nWARNING: This system *does not* have a version of OpenSSL installed that"
    print "can perform SHA256!! It needs to be installed. This will now be attempted."

    ## else to be this far means I need to install OpenSSL

    if sys.platform=='win32':
        print "\nYou are running Windows since sys.platform='",sys.platform,"'"
        print "\nNOTE: You must ensure that all versions of openssl.exe are removed"
        print "i.e. such as in your CYGWIN path!!\n"

        InstallFile1 = "vcredist_x86.exe"
        InstallFile2 = "Win32OpenSSL_Light-0_9_8m.exe"
        if os.path.exists(InstallFile1):
            print "\nInstalling '", InstallFile1, "'"
            MySystem(InstallFile1,' /q')

            if os.path.exists(InstallFile2):
                print "\nInstalling '", InstallFile2, "'"
                MySystem(InstallFile2)
            else:
                print "ERROR: Could not find '", InstallFile2, "', you need to install this to continue"
                sys.exit()

        else:
            print "ERROR: Could not find '", InstallFile1, "', you need to install this to continue"
            sys.exit()
    else:
        print "Not running Windows?? (running %s)" % sys.platform
        print "\nERROR: Sorry, this program cannot automatically install OpenSLL on this system"
        print "You must *manually* install an OpenSSL version that can do SHA256 '0.9.8k' or better to continue\n"

        print "Under Ubuntu Linux for example, use these commands"
        print "sudo apt-get install openssl"
        print "sudo apt-get -f upgrade openssl\n"
        sys.exit()

def PrintStep(sz):
    global CurrentStep
    CurrentStep = CurrentStep + 1

    print "\n\n-------------------------------------------------------------------------------------------------"
    print "\nStep %d: %s" % (CurrentStep, sz)
    print "\n-------------------------------------------------------------------------------------------------"
    ##print getcwd() . "\n"
    if ShowSteps:
        time.sleep(3)

    LOG.write("\n\n-------------------------------------------------------------------------------------------------")
    LOG.write("\nStep %d: %s" % (CurrentStep, sz))
    LOG.write("\n-------------------------------------------------------------------------------------------------\n")


def LookForOKinFile():
    try:
        IN = open("temp.txt")
    except Exception, e:
        print "\nCan't open temp.txt for reading: %s" % e
        sys.exit(1)


    for line in IN.readlines():
        print line
        if "OK" in line:
            print "\n\t** OK ** FOUND IT"
            IN.close()
            return 1

    IN.close()

    print "\n\t** FAILED - not verified **"
    sys.exit(1)
    return 0

def reverse_numeric(x, y):
    return y - x

def ParseCommandLine_testversion(ShowDebugOutput):
    global testversion

    if ShowDebugOutput:
        PrintStep("Parsing command line arguments (if any) for testversion")

    argument = None
    indexes_to_del=[]

    line_pattern_testversion = re.compile(r'testversion=(\d)', flags=re.IGNORECASE)

    for argument_index in range(0,len(sys.argv)):
        argument=sys.argv[argument_index]
        match_testversion = line_pattern_testversion.search(argument)

        if not match_testversion is None:
            testversion = int(match_testversion.groups()[0])
            indexes_to_del.append(argument_index)
            continue

    indexes_to_del=sorted(indexes_to_del, cmp=reverse_numeric)

    for cur_index in indexes_to_del:
        if debug['level']>=1:
            debug_str='DEBUG: qpsa.ParseCommandLine_testversion - deleting index {0} ("{1}") from sys.argv'.format(cur_index,sys.argv[cur_index])
        del sys.argv[cur_index]


def ParseCommandLine(ShowDebugOutput):
    global ImageFilename, XMLFilename, SigCalc, attestcert, exponent, rootcakey, rootcacert, \
        attestcakey, attestcacert, certchainsize, app_id, tcg_min, tcg_max, tcgvalidate, \
        attestcert_sha1sign, multirootcert_packager

    if ShowDebugOutput:
        PrintStep("Parsing command line arguments (if any)")

    argument = None
    total_num_root_certs = 1
    rootcerts_dir = None

    line_pattern_build = re.compile(r'(image|build)=(.+)', flags=re.IGNORECASE)
    line_pattern_xml = re.compile(r'(xml)=(.+)', flags=re.IGNORECASE)
    line_pattern_part = re.compile(r'PART_NAME=(.+)', flags=re.IGNORECASE)
    line_pattern_hash = re.compile(r'HASH_ALG=(.+)', flags=re.IGNORECASE)
    line_pattern_swid = re.compile(r'=(0x[a-fA-F\d]+)', flags=re.IGNORECASE)
    line_pattern_exponent = re.compile(r'exponent=(\d+)', flags=re.IGNORECASE)
    line_pattern_rootcakey = re.compile(r'rootcakey=(.+\.key)', flags=re.IGNORECASE)
    line_pattern_rootcacert = re.compile(r'rootcacert=(.+\.cer)', flags=re.IGNORECASE)
    line_pattern_attestcakey = re.compile(r'attestcakey=(.+\.key)', flags=re.IGNORECASE)
    line_pattern_attestcacert = re.compile(r'attestcacert=(.+\.cer)', flags=re.IGNORECASE)
    line_pattern_certchainsize = re.compile(r'certchainsize=(\d)', flags=re.IGNORECASE)
    line_pattern_appid = re.compile(r'APP_ID=(0x[a-fA-F\d]+)', flags=re.IGNORECASE)
    line_pattern_debug = re.compile(r'DEBUG=(0x[a-fA-F\d]+)', flags=re.IGNORECASE)
    line_pattern_crashdump = re.compile(r'CRASH_DUMP=(0x[a-fA-F\d]+)', flags=re.IGNORECASE)
    line_pattern_tcg_min = re.compile(r'tcg_min=([0xa-fA-F\d]+)', flags=re.IGNORECASE)
    line_pattern_tcg_max = re.compile(r'tcg_max=([0xa-fA-F\d]+)', flags=re.IGNORECASE)
    line_pattern_tcgvalidate = re.compile(r'(tcgvalidate)=(.+)', flags=re.IGNORECASE)
    line_pattern_attestcert_sha1sign = re.compile(r'(attestcert_sha1sign)=(.+)', flags=re.IGNORECASE)
    line_pattern_rootcerts_dir = re.compile(r'ROOTCERTS_DIR=(.+)', flags=re.IGNORECASE)
    line_pattern_total_num_root_certs = re.compile(r'TOTAL_NUM_ROOT_CERTS=(\d+)', flags=re.IGNORECASE)

    for argument in sys.argv:
        match_build = line_pattern_build.search(argument)
        match_xml = line_pattern_xml.search(argument)
        match_exponent = line_pattern_exponent.search(argument)

        match_rootcakey = line_pattern_rootcakey.search(argument)
        match_rootcacert = line_pattern_rootcacert.search(argument)
        match_attestcakey = line_pattern_attestcakey.search(argument)
        match_attestcacert = line_pattern_attestcacert.search(argument)
        match_certchainsize = line_pattern_certchainsize.search(argument)

        match_appid = line_pattern_appid.search(argument)
        match_debug = line_pattern_debug.search(argument)
        match_crashdump = line_pattern_crashdump.search(argument)
        match_tcg_min = line_pattern_tcg_min.search(argument)
        match_tcg_max = line_pattern_tcg_max.search(argument)
        match_tcgvalidate = line_pattern_tcgvalidate.search(argument)
        match_attestcert_sha1sign = line_pattern_attestcert_sha1sign.search(argument)

        match_rootcerts_dir = line_pattern_rootcerts_dir.search(argument)
        match_total_num_root_certs = line_pattern_total_num_root_certs.search(argument)

        if not match_rootcerts_dir is None:
            rootcerts_dir = os.path.abspath(match_rootcerts_dir.groups()[0])
            if ShowDebugOutput:
                print "\n\tArgument \"rootcerts_dir\" updated to %s" % rootcerts_dir
                LOG.write("\n\tArgument \"rootcerts_dir\" updated to %s" % rootcerts_dir)
            continue

        if not match_total_num_root_certs is None:
            total_num_root_certs = int(match_total_num_root_certs.groups()[0])
            if ShowDebugOutput:
                print "\n\tArgument \"total_num_root_certs\" updated to %d" % total_num_root_certs
                LOG.write("\n\tArgument \"total_num_root_certs\" updated to %d" % total_num_root_certs)
            continue

        if not match_tcgvalidate is None:
            if (match_tcgvalidate.groups()[1] == "yes"):
                tcgvalidate = True
            elif (match_tcgvalidate.groups()[1] == "no"):
                tcgvalidate = False
            else:
                print "\n\nERROR: Argument tcgvalidate does not support %s. Yes/No only." % match_tcgvalidate.groups()[1]
                LOG.write("\n\nERROR: Argument tcgvalidate does not support %s. Yes/No only." % match_tcgvalidate.groups()[1])
                LOG.close()
                sys.exit(1)
            if ShowDebugOutput:
                print "\n\tArgument \"tcgvalidate\" updated to %d" % tcgvalidate
                LOG.write("\n\tArgument \"tcgvalidate\" updated to %d" % tcgvalidate)
            continue

        if not match_attestcert_sha1sign is None:
            if (match_attestcert_sha1sign.groups()[1] == "yes"):
                attestcert_sha1sign = True
            elif (match_attestcert_sha1sign.groups()[1] == "no"):
                attestcert_sha1sign = False
            else:
                print "\n\nERROR: Argument attestcert_sha1sign does not support %s. Yes/No only." % match_attestcert_sha1sign.groups()[1]
                LOG.write("\n\nERROR: Argument attestcert_sha1sign does not support %s. Yes/No only." % match_attestcert_sha1sign.groups()[1])
                LOG.close()
                sys.exit(1)
            if ShowDebugOutput:
                print "\n\tArgument \"attestcert_sha1sign\" updated to %d" % attestcert_sha1sign
                LOG.write("\n\tArgument \"attestcert_sha1sign\" updated to %d" % attestcert_sha1sign)
            continue

        if not match_appid is None:
            app_id = match_appid.groups()[0]
            app_id = app_id[2:]  #remove "0x"
            if ShowDebugOutput:
                print "\n\tArgument \"APP_ID\" updated to 0x%s" % app_id
                LOG.write("\n\tArgument \"APP_ID\" updated to 0x%s" % app_id)
            continue
        if not match_debug is None:
            debug = match_debug.groups()[0]
            debug = debug[2:]  #remove "0x"
            if ShowDebugOutput:
                debug_str="\n\tArgument \"DEBUG\" updated to 0x%s" % debug
                print debug_str
                LOG.write(debug_str)
            attestcert['DEBUG']=debug #this is how we are saving it
            continue
        if not match_crashdump is None:
            crash_dump = match_crashdump.groups()[0]
            crash_dump = crash_dump[2:]  #remove "0x"
            if ShowDebugOutput:
                debug_str="\n\tArgument \"CRASH_DUMP\" updated to 0x%s" % crash_dump
                print debug_str
                LOG.write(debug_str)
            attestcert['CRASH_DUMP']=crash_dump #this is how we are saving it
            continue
        if not match_tcg_min is None:
            tcg_min = match_tcg_min.groups()[0]
            if ShowDebugOutput:
                print "\n\tArgument \"tcg_min\" updated to %s" % tcg_min
                LOG.write("\n\tArgument \"tcg_min\" updated to %s" % tcg_min)
            continue
        if not match_tcg_max is None:
            tcg_max = match_tcg_max.groups()[0]
            if ShowDebugOutput:
                print "\n\tArgument \"tcg_max\" updated to %s" % tcg_max
                LOG.write("\n\tArgument \"tcg_max\" updated to %s" % tcg_max)
            continue


        if not match_build is None:
            ImageFilename = match_build.groups()[1]
            continue

        if not match_xml is None:
            XMLFilename = match_xml.groups()[1]
            continue

        if not match_exponent is None:
            exponent = int(match_exponent.groups()[0])
            if not exponent in [3, 65537]:
                print "\n\nERROR: Exponent %d is invalid. Accepted values are 3 and 65537." % exponent
                LOG.write("\n\nERROR: Exponent %d is invalid. Accepted values are 3 and 65537." % exponent)
                LOG.close()
                sys.exit(1)
            continue

        if not match_rootcakey is None:
            rootcakey = match_rootcakey.groups()[0]
            if check_file_exists(rootcakey) == False:
                print "\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % rootcakey
                LOG.write("\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % rootcakey)
                LOG.close()
                sys.exit(1)
            continue
        if not match_rootcacert is None:
            rootcacert = match_rootcacert.groups()[0]
            if check_file_exists(rootcacert) == False:
                print "\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % rootcacert
                LOG.write("\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % rootcacert)
                LOG.close()
                sys.exit(1)
            else:
                pem_der_conversion(False, rootcacert, "%s.crt" % rootcacert[:rootcacert.rindex(".")])
                rootcacert = "%s.crt" % rootcacert[:rootcacert.rindex(".")]
            continue
        if not match_attestcakey is None:
            attestcakey = match_attestcakey.groups()[0]
            if check_file_exists(attestcakey) == False:
                print "\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % attestcakey
                LOG.write("\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % attestcakey)
                LOG.close()
                sys.exit(1)
            continue
        if not match_attestcacert is None:
            attestcacert = match_attestcacert.groups()[0]
            if check_file_exists(attestcacert) == False:
                print "\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % attestcacert
                LOG.write("\n\nERROR: File name \"%s\" has file size 0 (or does not exist)" % attestcacert)
                LOG.close()
                sys.exit(1)
            else:
                pem_der_conversion(False, attestcacert, "%s.crt" % attestcacert[:attestcacert.rindex(".")])
                attestcacert = "%s.crt" % attestcacert[:attestcacert.rindex(".")]
            continue
        if not match_certchainsize is None:
            certchainsize = int(match_certchainsize.groups()[0])
            if not certchainsize in [2,3]:
                print "\n\nERROR: Cert chain size %d is invalid. Accepted values are 1,2 and 3." % certchainsize
                LOG.write("\n\nERROR: Cert chain size %d is invalid. Accepted values are 1,2 and 3." % certchainsize)
                LOG.close()
                sys.exit(1)
            continue

        for key in SigCalc.keys():
            if argument.lower().startswith(key.lower()):
                match_part = line_pattern_part.search(argument)
                match_hash = line_pattern_hash.search(argument)
                match_swid = line_pattern_swid.search(argument)

                if not match_part is None:
                    if ShowDebugOutput:
                        print "\n\tArgument \"%s\" updated to %s" % (key, match_part.groups()[0])
                        LOG.write("\n\tArgument \"%s\" updated to %s" % (key, match_part.groups()[0]))

                    SigCalc[key] = match_part.groups()[0]
                    continue

                elif not match_hash is None:
                    if ShowDebugOutput:
                        print "\n\tArgument \"%s\" updated to %s" % (key, match_hash.groups()[0])
                        LOG.write("\n\tArgument \"%s\" updated to %s" % (key, match_hash.groups()[0]))

                    SigCalc[key] = match_hash.groups()[0]
                    continue

                elif not match_swid is None:
                    ## requirement is to have SW_ID entered as 0x1 but needs to be actually 0x0000000000000001
                    temp = "0x%.*X" % (SigCalcByteLengths[key], int(match_swid.groups()[0], 16))

                    if ShowDebugOutput:
                        print "\n\tArgument \"%s\" updated to %s" % (key, temp)
                        LOG.write("\n\tArgument \"%s\" updated to %s" % (key, temp))

                    temp  = temp.replace("0x", "")
                    UpdateSigCalcHash(key,temp)  ## key, input
                    continue

                else:
                    if ShowDebugOutput:
                        print "\nERROR: Argument for \"%s\" must be in HEX format, i.e. 0x1234ABCD" % key
                        LOG.write("\nERROR: Argument for \"%s\" must be in HEX format, i.e. 0x1234ABCD" % key)
                        CleanUp()
                        LOG.close()
                        sys.exit(1)




        for key in attestcert.keys():
            line_pattern_key = re.compile(r"^%s=(.+)" % key, flags=re.IGNORECASE)
            match_key = line_pattern_key.search(argument)
            if not match_key is None:
                if ShowDebugOutput:
                    print "\nAttestCERT key (%s), updated to \"%s\"" % (key, match_key.groups()[0])
                    LOG.write("\nAttestCERT key (%s), updated to \"%s\"" % (key, match_key.groups()[0]))

                attestcert[key] = match_key.groups()[0]
                continue

        #Recommended change: Add an error message indicating a failed match
        # DOUBT: i = i + 1


    if ImageFilename == "empty":
        usage()

        LOG.write("\n\nERROR: No image was specified")
        LOG.close()
        sys.exit(1)

    # Multirootcert support
    if total_num_root_certs > 1:
        try:
            import multirootcert
            multirootcert_packager = multirootcert.Packager(total_num_root_certs, rootcerts_dir)
        except Exception, e:
            print "\n\nERROR: %s" % e
            LOG.write("\n\nERROR: %s" % e)
            LOG.close()
            sys.exit(1)

        LOG.write("\n\nMultirootcert Packager is created")

    if (tcg_min is None and tcg_max is not None) or (tcg_min is not None and tcg_max is None):
        print "\n\nERROR: One of the tcg value is not provided: tcg_min=[%s] and tcg_max=[%s]" % (tcg_min, tcg_max)
        LOG.write("\n\nERROR: One of the tcg value is not provided: tcg_min=[%s] and tcg_max=[%s]" % (tcg_min, tcg_max))
        LOG.close()
        sys.exit(1)

def validate_appid():
    # Argument validation
    sw_id = long("0x"+ArrayToHexString(SigCalc["SW_ID"]),16)
    if (sw_id & 0xFFFFFFFF == 0xC):
        if app_id is None:
            LOG.write("\nSW_ID 0X%.16X have to set APP_ID" % sw_id)
            LOG.close()
            raise RuntimeError, "SW_ID 0X%.16X have to set APP_ID" % sw_id

        if (len(app_id) != 16):
            LOG.write("\nAPP_ID %s must be 16 digits" % app_id)
            LOG.close()
            raise RuntimeError, "APP_ID 0x%s must be 16 digits" % app_id

        app_id_num = long(app_id,16)
        if (app_id_num == 0):
            LOG.write("\nAPP_ID cannot be set to zero")
            LOG.close()
            raise RuntimeError, "APP_ID cannot be set to zero"

    else:
        if app_id is not None:
            LOG.write("\nSW_ID 0X%.16X should not set APP_ID %s" % (sw_id, app_id))
            LOG.close()
            raise RuntimeError, "SW_ID 0X%.16X should not set APP_ID 0x%s" % (sw_id, app_id)

def validate_debug():
    if attestcert.has_key('DEBUG') and not attestcert['DEBUG']==None:
        debug=attestcert['DEBUG'] #This should currently be a string
        if not len(debug)==16:
            error_str="\nDEBUG 0x{0} must be 16 digits".format(debug)
            log_and_leave(error_str)

        debug=long(debug,16) #verifies this is a valid hex number
        debug_str=debug

        #This section can be uncommented to restrict the valid values of the debug attribute
        #if (debug & 0xFFFFFFFF) in [0,1,2]: #Valid debug values are 0,1,or 2
        #    pass
        #else:
        #    error_str="DEBUG 0x%016X: last 32 bits are boolean and must be 0,1,or 2" % debug
        #    LOG.write("\n"+error_str)
        #    LOG.close()
        #    raise RuntimeError, error_str

    return 0

def validate_crash_dump():
    if attestcert.has_key('CRASH_DUMP') and not attestcert['CRASH_DUMP']==None:
        crash_dump=attestcert['CRASH_DUMP'] #This should currently be a string
        if not len(crash_dump)==16:
            error_str="\nERROR: qpsa.validate_crash_dump - CRASH_DUMP 0x{0} must be 16 digits".format(crash_dump)
            log_and_leave(error_str)

        crash_dump=long(crash_dump,16) #verifies this is a valid hex number
        crash_dump_str=crash_dump

        if (crash_dump & 0xFFFFFFFF) in [0,1]: #Valid crashdump values are boolean; 1 or 0 are valid
            pass
        else:
            error_str="\nERROR: qpsa.validate_crash_dump - CRASH_DUMP 0x%016X: last 32 bits are boolean and must be either 1 or 0" % crash_dump
            log_and_leave(error_str)

    return 0

def CleanUp():
    PrintStep("Removing TEMPORARY FILES")
    if ShowSteps:
        time.sleep(5) ## extra 5 seconds here so I can BREAK if I need to

    file_list = ["opensslversion.txt", "%sPhaseOne.bin" % ImageFilenameNoExt, "%sPhaseTwo.bin" % ImageFilenameNoExt, "%sattest.csr" % ImageFilenameNoExt, "sighash.bin", "sha256rootcert.txt"]

    for some_file in file_list:
        if os.path.exists(some_file):
            os.unlink(some_file)

    output_file_list = ["%s-signature.bin" % OutputFilename, \
                 "%s-root_cert.cer" % OutputFilename, \
                 "%s-attestation_ca_cert.cer" % OutputFilename, \
                 "%s-attestation_cert.cer" % OutputFilename, \
                 "all.crt", \
                 "%s-root_cert.crt" % OutputFilename, \
                 "%s-attestation_ca_cert.crt" % OutputFilename, \
                 "%s-attestation_cert.crt" % OutputFilename, \
                 "qpsa_attestca.crt", \
                 "qpsa_attestca.csr", \
                 "%sattest.key" % ImageFilenameNoExt]

    if sys.platform=='win32':   ## Needs 0.9.8m or better
        cmds = {'temp':["del"],'shell':True}
    else:
        cmds = {'temp':["rm"]}

    for file in output_file_list:
        if os.path.exists(file):
            #cmds = cmds + " %s" % file
            cmds['temp'].append(file)
    MySystem(**cmds)

    #PrintStep("*MANUAL STEP* Move \"".$OutputFilename.".zip\" to the correct location - (MANUAL FOR NOW SORRY)")

    file_list = ["QTIDevSigningToolDefaults.xml", "qpsa_attestca.cer", "qpsa_attestca.key", \
                "qpsa_rootca.cer", "qpsa_rootca.key", "opensslroot.cfg", "v3_attest.ext", \
                "tee.exe", "temp.txt", "qpsa_rootca.crt", "v3.ext", \
                "%s.crt" % rootcacert[:rootcacert.rindex(".")], \
                "%s.crt" % attestcacert[:attestcacert.rindex(".")]]

    for some_file in file_list:
        if os.path.exists(some_file):
            os.unlink(some_file)


def printDateTimeInLog():
    months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
    weekDays = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
    (year, month, dayOfMonth, hour, minute, second, dayOfWeek, dayOfYear, daylightSavings) = time.localtime()
    theTime = "%d:%d:%d, %s %s %d, %d" % (hour, minute, second, weekDays[dayOfWeek], months[month-1], dayOfMonth, year)
    LOG.write("\n\n\n==============================================================")
    LOG.write("====================== BEGIN =================================")
    LOG.write("==============================================================\n")
    LOG.write("qpsa.py started on %s" % theTime)

def CompareArrays(a, b):
    i = None
    j = None
    LengthA = len(a)
    LengthB = len(b)
    Errors = 0

    ## NOTE: Array is actually a number split up, i.e. 0x01 is 0,1
    ## I expect it to be the same length BUT if not, leading ZEROs are ok
    ## i.e. 0x0001 = 0x1

    while LengthA + LengthB > 0:
        if LengthA>0 and LengthB>0:
            LengthA = LengthA - 1
            LengthB = LengthB - 1

            if not a[LengthA] == b[LengthB]:
                Errors = 1
                break
        else:
            if LengthA==0:
                LengthB = LengthB - 1

                if not b[LengthB] == 0:
                    Errors = 1
                    break
            elif LengthB==0:
                LengthA = LengthA - 1
                if not a[LengthA] == 0:
                    Errors = 1
                    break

    return Errors   ## returns 0 if the arrays are the same

def getMinMaxTcg(filename):
    # Get hexdump of cert extensions.
    cmds = [OPENSSL_CMD, "x509", "-text", "-certopt", "ext_dump", "-in", filename]
    LOG.write("\nCalling SYSTEM with")
    LOG.write("\n\"" + ' '.join(cmds) + "\"")

    try:
        openSslOut=sub.check_output(cmds)
    except Exception, e:
        print "Error: %s" % (e)
        LOG.write("\nError: %s" % (e))
        LOG.close()
        sys.exit()

    # Check if the correct OID is in the output.
    if ("1.3.6.1.4.1.1449.9.6.3:" in openSslOut):
        tcg_str=openSslOut[openSslOut.find('1.3.6.1.4.1.1449.9.6.3:') + 48:openSslOut.find('-                 ')]
        tcg_str = tcg_str.replace(' ', '') # remove spaces
        tcg_min_str = tcg_str[0:8]
        tcg_max_str = tcg_str[8:16]
        tcg_min=int(tcg_min_str,16)
        tcg_max=int(tcg_max_str,16)
    else:
        tcg_min = -1
        tcg_max = -1
    return (tcg_min, tcg_max)

def checkTcgConstraints(tcgMinAttest, tcgMaxAttest, tcgMinCa, tcgMaxCa):
    result = False
    if (tcgMinCa <= tcgMinAttest <= tcgMaxAttest <= tcgMaxCa):
       result = True
    return (result)

def LogTCGValue(mesg, tcg):
    if (tcg == -1):
        log_mesg = mesg + "%d"
    else:
        log_mesg = mesg + "0x%.8X"

    print log_mesg % tcg
    LOG.write("\n" + log_mesg % tcg)


def VerifyTCG(ca_cert_in_pem, tcg_min, tcg_max):
    #tcg_min_attest, tcg_max_attest = getMinMaxTcg(attest_cert_in_pem)

    tcg_min_attest = -1
    tcg_max_attest = -1
    if (tcg_min is not None) and (tcg_max is not None):
        #tcg_str = "%.16X" % long(tcg, 16)
        tcg_min_attest = int(tcg_min, 16)
        tcg_max_attest = int(tcg_max, 16)

    # If there is no CA Cert, this file will not exist, so check before looking for TCGs.
    if (os.path.exists(ca_cert_in_pem)):
        tcg_min_ca, tcg_max_ca = getMinMaxTcg(ca_cert_in_pem)
    else:
        tcg_min_ca = -1
        tcg_max_ca = -1
    # tcg_min/max_ca will be -1 if there are not TCGs in the CA cert, OR if there is no CA cert.

    LogTCGValue("\nMin input TCG = ", tcg_min_attest)
    LogTCGValue("Max inpt TCG = ", tcg_max_attest)
    LogTCGValue("Min TCG in CA Cert = ", tcg_min_ca)
    LogTCGValue("Max TCG in CA Cert = ", tcg_max_ca)

    tcg_ok = False
    # If the attestation cert has TCGs, but the CA cert does not (or doesn't exist), this is invalid.
    #These should be written as nested conditionals;
    if ((tcg_min_attest != -1) and (tcg_max_attest != -1) and (tcg_min_ca == -1) and(tcg_max_ca == -1)):
        print "TCGs found in Attestation cert, but not in CA cert. This is invalid."
        LOG.write("\nTCGs found in Attestation cert, but not in CA cert. This is invalid.")
    elif ((tcg_min_attest == -1) and (tcg_max_attest == -1) and (tcg_min_ca != -1) and (tcg_max_ca != -1)):
        print "No TCGs found in Attestation cert, but there are TCGs in CA cert. This is invalid."
        LOG.write("No TCGs found in Attestation cert, but there are TCGs in CA cert. This is invalid.")
    elif ((tcg_min_attest == -1) and (tcg_max_attest == -1) and (tcg_min_ca == -1) and (tcg_max_ca == -1)): # This is ok. No TCGs in attest cert.
        tcg_ok = True
        print "No TCGs found in Attestation cert or CA cert. This is OK."
        LOG.write("No TCGs found in Attestation cert or CA cert. This is OK.")
    elif (checkTcgConstraints(tcg_min_attest, tcg_max_attest, tcg_min_ca, tcg_max_ca)): # Check constraints.
        tcg_ok = True
        print "TCG values fall within CA constraints."
        LOG.write("\nTCG values fall within CA constraints.")
    else:
        print "TCG values are outside the CA constraints."
        LOG.write("\nTCG values are outside the CA constraints.")

    # Verify that all signing attributes are zero if CA cert contain TCG
    if ((tcg_min_ca!=-1) and (tcg_max_ca!=-1)):
        if ((attestcert.has_key("DEBUG") and long(attestcert["DEBUG"], 16) != 2) or
            (long("0x"+ArrayToHexString(SigCalc["MODEL_ID"]),16) != 0) or
            (long("0x"+ArrayToHexString(SigCalc["OEM_ID"]),16) != 0) or
            (long("0x"+ArrayToHexString(SigCalc["MSM_PART"]),16) != 0) or
            (long("0x"+ArrayToHexString(SigCalc["SW_ID"]),16) != 0)):
            error_str="DEBUG (if present) should be 2; MODEL_ID, OEM_ID, MSM_PART and SW_ID should be all zero."
            print error_str
            LOG.write("\n"+error_str)
            tcg_ok = False

    if (tcg_ok == True):
        print "*****TCG checking PASSED."
        LOG.write("\n*****TCG checking PASSED.")
    else:
        print "*****TCG checking FAILED!!!"
        LOG.write("\n*****TCG checking FAILED!!!")
        CleanUp()
        LOG.close()
        sys.exit(1)

    return

def find_openssl_location(exit_on_fail=False):
    openssl_path=None
    if 'OPENSSL_DIR' in os.environ.keys() and os.environ.get('OPENSSL_DIR', '') is not '':
        abspath_openssl=os.path.abspath(OPENSSL_CMD)
    else:
        abspath_openssl=OPENSSL_CMD
    try:
        openssl_path=find_command_location(command=abspath_openssl,exit_on_fail=True)
    except (SystemExit),e:
        if exit_on_fail:
            error_str="-"*78 + "\n" + \
                      "ERROR: OpenSSL was not found??\n\n"
            log_and_leave(error_str)
    except Exception, e:
        if exit_on_fail:
            log_and_leave(str(e))
        else:
            print e
    else:
        openssl_path=os.path.realpath(openssl_path)
    return openssl_path

def which_cmd(filename):
    if os.name.lower()=='nt' and sys.platform is not 'cygwin':
        filename+=".exe"
    matches=[]
    path_dirs = os.environ.get("PATH").split(os.pathsep)
    path_dirs.append('.')
    for directory in path_dirs:
        fullpath = os.path.join(directory, filename)
        if os.path.isfile(fullpath):
            matches.append(fullpath)
    return matches

def find_command_location(command, exit_on_fail=False):
    return which_cmd(command)[0]
# def find_command_location(command, exit_on_fail=False):
#     abspath_command=command
#     where_command=None
#     output=''
#
#     if os.name.lower()=='nt' and sys.platform is not 'cygwin':
#         (head_path,tail_path)=os.path.split(abspath_command)
#         if head_path==None or head_path=='':
#             where_command=['where',abspath_command]
#         else:
#             where_command=['where','{0}:{1}'.format(head_path,tail_path)]
#         #failure indicated by "INFO: Could not find files for the given pattern(s)." and non-zero exit code
#     else:
#         where_command=['which','-a',abspath_command]
#     if debug['level']>=1:
#         debug_str='DEBUG: qpsa.find_command_location - external call command: {0}'.format(repr(where_command))
#         print debug_str
#         LOG.write(debug_str+"\n")
#
#     try:
#         output=external_call(command=where_command, capture_output=True, print_capture=False, posix=None, shell=False, raise_except=True)
#     except Exception, e:
#         error_str='ERROR: Path to command could not be verified; external_call threw an exception'
#         print "\n"+error_str
#         LOG.write("\n"+error_str+"\n")
#         if exit_on_fail:
#             log_and_leave(error_str)
#     else:
#         if debug['level']>=1:
#             debug_str='DEBUG: qpsa.find_command_location - output from external call >>>' + "\n" + \
#                        output + "<<<"
#             print debug_str
#             LOG.write(debug_str+"\n")
#
#         output_lines=output.splitlines()
#
#         abspath_command=os.path.abspath(output_lines[0])
#
#     return abspath_command

def safe_mv_file(origin_filename, dest_filename):
    skip_move=False
    if os.path.exists(dest_filename):
        if debug['level']>=1:
            debug_str='DEBUG: qpsa.safe_rename_file - (rename) dest_filename ("{0}") exists!'
            print debug_str
            LOG.write(debug_str)
        if os.path.normcase(os.path.realpath(origin_filename)) == os.path.normcase(os.path.realpath(dest_filename)):
            if debug['level']>=1:
                debug_str='DEBUG: qpsa.safe_rename_file - (rename) dest_filename ("{0}") is the same file as the source file!'
                print debug_str
                LOG.write(debug_str)
            skip_move=True
        else:
            os.remove(dest_filename)
    else:
        if debug['level']>=1:
            debug_str='DEBUG: qpsa.safe_rename_file - (rename) dest_filename ("{0}") does NOT exist!'
            print debug_str
            LOG.write(debug_str)
    if skip_move:
        pass
    else:
        shutil.move(origin_filename, dest_filename)

#----------------------------------------------------------------------------
# concat_files
# Concatenates the files listed in 'sources' in order and writes to 'target'
#----------------------------------------------------------------------------
def concat_files(target, sources):
    if type(sources) is not list:
        sources = [sources]

    target_file = open(target,'wb')

    for fname in sources:
        file = open(fname,'rb')
        LOG.write("Appending %s to %s\n" % (fname, target))
        while True:
            bin_data = file.read(65536)
            if not bin_data:
                break
            target_file.write(bin_data)
        file.close()
    target_file.close()

def log_and_leave(error_msg, raise_except = False, exception = None):
    print error_msg
    LOG.write(error_msg+"\n")
    if raise_except and exception is not None:
        raise exception
    else:
        sys.exit(1)

# Print version only if needed
for argument in sys.argv[1:]:
    if argument == "-v":
        print "QPSA " + CURRENT_VERSION
        sys.exit()

CurrentStep = 0
ShowSteps = 0
## Where was this script started
dir = os.getcwd()

LogFileName = "qpsa_log.txt"

if dir[-1] == "/" or dir[-1] == "\\": ## makes it have to have an anchor at the end
    dir = dir[:-1]  ## Take the left of what matched - get rid of slash

dir.replace("\\", "/")
#dir.replace("\", "/")

dir = "%s/" % dir

print "Script was started in '%s'\n" % dir

try:
    LOG = open(LogFileName, "w")
except Exception, e:
    print "\nCan't open %s for writing: %s" % (LogFileName, e)
    sys.exit(1)

printDateTimeInLog()


SigCalc = {
   'SW_ID'    : [ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ],
   'OEM_ID'   : [ 0x00, 0x00 ],
   'MODEL_ID' : [ 0x00, 0x00 ],
   'MSM_PART' : [ 0x00, 0x00, 0x00, 0x00 ],
   'ipad'     : [ 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36 ],
   'opad'     : [ 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C ],
   'PART_NAME': "DefaultPart",
   'HASH_ALG' : "SHA256",
   'SW_ID_NAME': "IMAGE",
}

SigCalcByteLengths = {
   'SW_ID'    : 16,
   'OEM_ID'   : 4,
   'MODEL_ID' : 4,
   'MSM_PART' : 8,
   'ipad'     : 16,
   'opad'     : 16,
}

attestcert = {
   'C'              : "US",
   'ST'             : "California",
   'L'              : "San Diego",
   'O'              : "ASIC",
   'CN'             : "Qualcomm",
   #'DEBUG'          : "0x0000000000000000",   ## set to off; on is 0xF
}



i = None
j = None
key = None
ImageFilename = "empty"
XMLFilename = "empty"
exponent = 3
rootcakey = None
rootcacert = None
attestcakey = None
attestcacert = None
certchainsize = 3
testversion = 1
app_id = None
tcg_min = None
tcg_max = None
tcgvalidate = False
attestcert_sha1sign = False
multirootcert_packager = None

## There is a good chance this script will be called like this
## python c:\windows\qpsa.py etc

## Therefore, I need to be in the c:\windows to run the script

PathToQPSA = sys.path[0]

print "\nChanging to sys.path[0]=",PathToQPSA
os.chdir(PathToQPSA) #Recommended change: Rather than change directories, ensure resources directory is in the search path
print "I'm currently in ",os.getcwd()


#path = os.environ['PATH']

#if not "openssl".lower() in path.lower():
#	if os.path.exists("c:\\OpenSSL\\bin"):
#		os.environ['PATH'] = "c:\\OpenSSL\\bin;%s" % path
#	elif os.path.exists("c:\\OpenSSL-Win32\\bin"):
#		os.environ['PATH'] = "c:\\OpenSSL-Win32\\bin;%s" % path
#	else:
        # DOUBT
#		print "Could not find location of OpenSSL bin folder!"

print "\n------------------------------------------------------------------"
print   "-------Qualcomm Platform Signing Application Code Signing V2.0--------"
print   "------------------------------------------------------------------"

ParseCommandLine_testversion(1) ## done here mostly to see if we need to test the openssl version

OpenSSLInstalled = 0


PrintStep("Testing if you have a current OpenSSL version installed")

print "\nOPENSSL COMMAND = %s\n" % OPENSSL_CMD
LOG.write("\n\nOPENSSL COMMAND = %s\n" % OPENSSL_CMD)

path_to_openssl=find_openssl_location(exit_on_fail=not testversion)
log_str="Full path to openssl is '{0}'\n".format(path_to_openssl)

print log_str
LOG.write(log_str+"\n")

if testversion==1:
    TestForOpenSSL()    ## if not installed, it tries to install it
else:
    OpenSSLInstalled = 1
    print "-"*78
    print "NOTE: OpenSSL version was NOT tested because \"testversion=0\" was specified\n"

if OpenSSLInstalled == 0:
    InstallOpenSSL()

    TestForOpenSSL()    ## test *now* if it is installed. If not, script bails here

    if OpenSSLInstalled==0:
        print "\nERROR: The correct version of OpenSSL is *not* installed and this"
        print "script could not install it.\n"

        LOG.write("\n\nERROR: The correct version of OpenSSL is *not* installed and this")
        LOG.write("\nscript could not install it.\n")

        LOG.close()
        sys.exit(1)

ParseCommandLine(1) ## done here mostly to see if an image was provided

## there is a chance the files are already unzipped, thus must remove them first
file_list = ["QTIDevSigningToolDefaults.xml", "qpsa_attestca.cer", "qpsa_attestca.key", "qpsa_rootca.cer", "qpsa_rootca.key", "opensslroot.cfg", "sha256rootcert.txt", "v3_attest.ext"]
for some_file in file_list:
    if os.path.exists(some_file):
        os.unlink(some_file)


#MySystem("unzip qpsa.zip")
#TODO: This requires testing
if check_file_exists("qpsa.zip") == True:
    try:
        unzip('qpsa.zip')
    except Exception, e:
        error_str="ERROR: Please check that I have access to qpsa.zip OR ensure it is *zipped up* correctly\n\n. Exception: '{0}'".format(e)
        log_and_leave(error_str, True, e)
else:
    #appsbl build integration support requires no zip file for OS scan
    LOG.write("\n\nqpsa.zip does not exist. Copy files from alt_resources\/default\/openssl to current directory\n\n")
    openssl_configfiles = ["opensslroot.cfg", "v3_attest.ext", "v3.ext", "QTIDevSigningToolDefaults.xml", "opensslroot.cfg"]
    for file in openssl_configfiles:
        file_to_copy = "alt_resources/default/openssl/%s" % file
        if check_file_exists(file_to_copy) == False:
            LOG.write("\n\nBoth qpsa.zip and %s does not exist.\n\n" % file_to_copy)
            print "\n\nERROR:Both qpsa.zip and %s does not exist.\n\n" % file_to_copy
            LOG.close()
            sys.exit(1)
        shutil.copy("alt_resources/default/openssl/%s" % file, "./")

    # Generate random key if rootcakey/rootcacert/attestcakey/attestcacert
    # is not specified.
    if (rootcakey is None or check_file_exists(rootcakey) == False or
        rootcacert is None or check_file_exists(rootcacert) == False or
        attestcakey is None or check_file_exists(attestcakey) == False or
        attestcacert is None or check_file_exists(attestcacert) == False):
        generate_root_ca_files()
        generate_attestation_ca_files()

if XMLFilename == "empty":
    PrintStep("Getting defaults from XML file \"QTIDevSigningToolDefaults.xml\"")
    ParseDefaultsFromXML("QTIDevSigningToolDefaults.xml")
else:
    PrintStep("Getting defaults from XML file \"%s\"" % XMLFilename)
    ParseDefaultsFromXML(XMLFilename)

ParseCommandLine(1) ## get command line arguments that will overwrite xml arguments

validate_appid()
validate_debug()
validate_crash_dump()

PrintStep("Showing the defaults to be used in the Signature Calculation")

print "\n---------------------------------------------------------------------"
print "A signature generation is applied to a code image to tie MSM HW info"
print "(MSM_ID,OEM_ID,PROD_ID etc) cryptographically into the image. The"
print "following performs the Hash Message Authentication Code (HMAC) to allow"
print "the image specified to be digitally signed"
print "---------------------------------------------------------------------"

PrintSigCalcHash()

if ShowSteps:
    time.sleep(2)

PrintStep("Verifying image provided exists")

#$ImageFilename
## Ex. $ImageFilename = c:\temp\dbl_SCTUSS_nonsec.mbn
temp = ImageFilename
## TODO: temp =~ s#/#\\#g ## replace  / with \

if sys.platform=='win32':   ## Needs 0.9.8m or better
    temp = temp.replace("/", "\\")
    # Note, there is a chance there are no slashes in this filename
    if(temp.rfind("\\") != -1):
        ImageFilenameNoExt = temp[temp.rindex("\\")+len("\\"):]
    else:
        ImageFilenameNoExt = temp

else:
    if(temp.rfind("/") != -1):
        ImageFilenameNoExt = temp[temp.rindex("/")+len("/"):]
    else:
        ImageFilenameNoExt = temp

if "." in ImageFilenameNoExt:
    ImageFilenameNoExt = ImageFilenameNoExt[0:ImageFilenameNoExt.rindex(".")]

print "ImageFilename =\t\t%s" % ImageFilename
print "ImageFilenameNoExt =\t%s\n\n" % ImageFilenameNoExt

ImagePath = temp.index(ImageFilenameNoExt)
ImagePath = "%scert/" % temp[0:ImagePath]

ImageFilenameNoExt = "qpsa_%s_" % ImageFilenameNoExt

printDateTimeInLog()

## $ImageFilenameNoExt = dbl_SCTUSS_nonsec
#print "\n\n$ImageFilenameNoExt and ".index($ImageFilename, ".") . " and " . length($ImageFilename)

#$ImageFilename provided, but does it exist?
ImageFileSize = os.path.getsize(ImageFilename)

if ImageFileSize == 0:
    print "\n\nERROR: image name \"%s\" has file size 0 (or does not exist)" % ImageFilename
    print "\nEx."
    print "\npython qpsa.py image=dbl_SCTUSS_nonsec.mbn\n\n"

    LOG.write("\n\nERROR: image name \"%s\" has file size 0 (or does not exist)" % ImageFilename)
    LOG.write("\nEx.")
    LOG.write("\npython qpsa.py image=dbl_SCTUSS_nonsec.mbn\n\n")
    CleanUp()
    LOG.close()
    sys.exit(1)
else:
    print "\nImage to sign exists \"%s\" image (%d bytes)\n" % (ImageFilename, ImageFileSize)
    LOG.write("\nImage to sign exists \"%s\" image (%d bytes)\n" % (ImageFilename, ImageFileSize))

OutputFilename = "empty"

if CompareArrays(SigCalc["SW_ID"],[0,0,0,0,0,0,0,0]) == 0:         #SBL1
    OutputFilename = "%s_SBL" % SigCalc["PART_NAME"]
elif CompareArrays(SigCalc["SW_ID"],[0,0,0,0,0,0,0,2]) == 0:       #MODEM
    OutputFilename = "%s_AMSS_HASH_TABLE" % SigCalc["PART_NAME"]
elif CompareArrays(SigCalc["SW_ID"],[0,0,0,0,0,0,0,3]) == 0:       #EMMCBLD
    OutputFilename = "%s_FLASH_PRG" % SigCalc["PART_NAME"]
elif CompareArrays(SigCalc["SW_ID"],[0,0,0,0,0,0,0,4]) == 0:       #ADSP
    OutputFilename = "%s_DSP_HASH_TABLE" % SigCalc["PART_NAME"]
else:
    OutputFilename = "%s_%s" % (SigCalc["PART_NAME"], SigCalc["SW_ID_NAME"])

OutputFilename = "qpsa_%s" % OutputFilename
## Final step is to create the MSM_ID, which is a concat of the MSM_PART, OEM_ID and MODEL_ID

# If generation of qpsa_rootca.cer and/or qpsa_attestca.cer is required, it can be called here
if rootcakey is None or check_file_exists(rootcakey) == False:
    rootcakey = "qpsa_rootca.key"
if rootcacert is None or check_file_exists(rootcacert) == False:
    rootcacert = "qpsa_rootca.crt"
if attestcakey is None or check_file_exists(attestcakey) == False:
    attestcakey = "qpsa_attestca.key"
if attestcacert is None or check_file_exists(attestcacert) == False:
    attestcacert = "qpsa_attestca.crt"

PrintStep("Preparing ROOTCA and ATTESTCA certificate names to match image name (as per CSMS website)")
shutil.copyfile("%s.cer" % rootcacert[:rootcacert.rindex(".")], "%s-root_cert.cer" % OutputFilename)
if certchainsize == 3:
    shutil.copyfile("%s.cer" % attestcacert[:attestcacert.rindex(".")], "%s-attestation_ca_cert.cer" % OutputFilename)

if multirootcert_packager is not None and \
    multirootcert_packager.containRootCert("%s-root_cert.cer" % OutputFilename) is False:
    print "%s does not contain the selected root cert" % multirootcert_packager.getRootCertDir()
    LOG.write("%s does not contain the selected root cert" % multirootcert_packager.getRootCertDir())
    LOG.close()
    sys.exit(1)

MySystem("%s x509 -inform DER -in %s.cer -outform PEM -out %s.crt" % \
        (OPENSSL_CMD, rootcacert[:rootcacert.rindex(".")], rootcacert[:rootcacert.rindex(".")]))
MySystem("%s x509 -inform DER -in %s.cer -outform PEM -out %s.crt" % \
        (OPENSSL_CMD, attestcacert[:attestcacert.rindex(".")], attestcacert[:attestcacert.rindex(".")]))

PrintStep("Converting root/sub-CA certs to PEM format")
MySystem("%s x509 -inform DER -in %s-root_cert.cer -outform PEM -out %s-root_cert.crt" % (OPENSSL_CMD, OutputFilename, OutputFilename))
if certchainsize == 3:
    MySystem("%s x509 -inform DER -in %s-attestation_ca_cert.cer -outform PEM -out %s-attestation_ca_cert.crt" % (OPENSSL_CMD, OutputFilename, OutputFilename))

    if tcgvalidate is True:
        PrintStep("VERIFYING input tcg (optional)")
        VerifyTCG("%s-attestation_ca_cert.crt" % (OutputFilename), tcg_min, tcg_max)

try:
    teemp = SigCalc["MSM_PART"][3]
except:
    print "-"*78
    print "ERROR: MSM_PART is not formed correctly. Please ensure it is in the form 0x12345678 (or 0x00000000)\n"
    sys.exit()

try:
    teemp = SigCalc["OEM_ID"][1]
except:
    print "-"*78
    print "ERROR: OEM_ID is not formed correctly. Please ensure it is in the form 0x1234 (or 0x0000)\n"
    sys.exit()

try:
    teemp = SigCalc["MODEL_ID"][1]
except:
    print "-"*78
    print "ERROR: MODEL_ID is not formed correctly. Please ensure it is in the form 0x1234 (or 0x0000)\n"
    sys.exit()

try:
    temp = [ SigCalc["MSM_PART"][0], SigCalc["MSM_PART"][1], SigCalc["MSM_PART"][2], SigCalc["MSM_PART"][3], SigCalc["OEM_ID"][0], SigCalc["OEM_ID"][1], SigCalc["MODEL_ID"][0], SigCalc["MODEL_ID"][1] ]
except:
    print "-"*78
    print "ERROR: The values in the XML file are not correct\n"
    sys.exit()

if "MSM_ID" in SigCalc.keys():
    SigCalc["MSM_ID"].extend(temp)   ## $SigCalc["OEM_ID"] is an array
else:
    SigCalc["MSM_ID"] = temp

PrintStep("Generating \"Phase 1\" of signature (see Figure 3-1 Keyed Hash Algorithm)")
if "sha1".lower() in SigCalc["HASH_ALG"].lower():
    GeneratePhaseOutput(SigCalc["SW_ID"] ,SigCalc["ipad"], Sha1(ImageFilename), "%sPhaseOne.bin" % ImageFilenameNoExt)
else:
    GeneratePhaseOutput(SigCalc["SW_ID"], SigCalc["ipad"], Sha256(ImageFilename), "%sPhaseOne.bin" % ImageFilenameNoExt)

PrintStep("Generating \"Phase 2\" of signature (see Figure 3-1 Keyed Hash Algorithm)")
if "sha1".lower() in SigCalc["HASH_ALG"].lower():
    GeneratePhaseOutput(SigCalc["MSM_ID"], SigCalc["opad"], Sha1("%sPhaseOne.bin" % ImageFilenameNoExt), "%sPhaseTwo.bin" % ImageFilenameNoExt)
else:
    GeneratePhaseOutput(SigCalc["MSM_ID"], SigCalc["opad"], Sha256("%sPhaseOne.bin" % ImageFilenameNoExt), "%sPhaseTwo.bin" % ImageFilenameNoExt)

filesize = os.path.getsize("%sPhaseTwo.bin" % ImageFilenameNoExt)

print "\n%sPhaseTwo.bin is ready to be signed (%d bytes)" % (ImageFilenameNoExt, filesize)
#print Sha256($ImageFilenameNoExt."PhaseTwo.bin")


## CERTIFICATE CODE BEGINS HERE

if "sha1".lower() in SigCalc["HASH_ALG"].lower():
    attestcert["OU07"] = "07 0000 SHA1"
else:
    attestcert["OU07"] = "07 0001 SHA256"

#if long(attestcert["DEBUG"], 16) == 0xF:					## attestcert["DEBUG"] looks like '0000000000000000'
#    attestcert["OU03"]  = "03 000000000000000F DEBUG"
#else:
#    attestcert["OU03"]  = "03 0000000000000000 DEBUG"

# Now any value is allowed for DEBUG
#attestcert["OU03"] = "03 %.16X DEBUG" % long(attestcert["DEBUG"], 16)
if attestcert.has_key('DEBUG') and attestcert['DEBUG'] is not None:  #DEBUG is optional
    debug_long=None
    try:
        debug_long=long(attestcert['DEBUG'],16)
    except Exception, e:
        LOG.write(e)
        raise e
    attestcert["OU03"] = "03 %.16X DEBUG" % debug_long

attestcert["OU05"] = "05 %.8X SW_SIZE" % ImageFileSize

#attestcert["OU06"] = "06 %s MODEL_ID" % ArrayToHexString(SigCalc["MODEL_ID"])
#attestcert["OU04"] = "04 %s OEM_ID" % ArrayToHexString(SigCalc["OEM_ID"])
#attestcert["OU02"] = "02 %s HW_ID" % ArrayToHexString(SigCalc["MSM_ID"])
#attestcert["OU01"] = "01 %s SW_ID" % ArrayToHexString(SigCalc["SW_ID"])

attestcert["OU06"] = "06 %.4X MODEL_ID" % long("0x"+ArrayToHexString(SigCalc["MODEL_ID"]),16)
attestcert["OU04"] = "04 %.4X OEM_ID" % long("0x"+ArrayToHexString(SigCalc["OEM_ID"]),16)
attestcert["OU02"] = "02 %.16X HW_ID" % long("0x"+ArrayToHexString(SigCalc["MSM_ID"]),16)
attestcert["OU01"] = "01 %.16X SW_ID" % long("0x"+ArrayToHexString(SigCalc["SW_ID"]),16)
if app_id is not None:  #APP_ID is optional
    attestcert["OU08"] = "08 %.16X APP_ID" % long(app_id,16)

if attestcert.has_key('CRASH_DUMP') and attestcert['CRASH_DUMP'] is not None:  #CRASH_DUMP is optional
    crash_dump_long=None
    try:
        crash_dump_long=long(attestcert['CRASH_DUMP'],16)
    except Exception, e:
        LOG.write(e)
        raise e
    attestcert["OU09"] = "09 %.16X CRASH_DUMP" % crash_dump_long

# Write TCG to configuration file
if (tcg_min is not None) and (tcg_max is not None):
    v3_attest_file_new = open("v3_attest_new.ext",'w')
    v3_attest_file = open("v3_attest.ext", 'r')
    for line in v3_attest_file:
        v3_attest_file_new.write(line)
        if (line.find("keyUsage")>=0):
                #Add tcg after "keyUsage"
                tcg_min_base = 16
                tcg_max_base = 16
                if (tcg_min.lower().find("0x")!=0):
                    tcg_min_base = 10
                if (tcg_max.lower().find("0x")!=0):
                    tcg_max_base = 10
                tcg_str = "%.8X%.8X" % (int(tcg_min, tcg_min_base), int(tcg_max, tcg_max_base))
                tcg_cfg = "1.3.6.1.4.1.1449.9.6.3=DER:%s:%s:%s:%s:%s:%s:%s:%s" % \
                        (tcg_str[0:2], tcg_str[2:4], tcg_str[4:6], tcg_str[6:8], \
                         tcg_str[8:10], tcg_str[10:12], tcg_str[12:14], tcg_str[14:16])
                v3_attest_file_new.write(tcg_cfg)
    v3_attest_file_new.close()
    v3_attest_file.close()
    os.remove("v3_attest.ext")
    os.rename("v3_attest_new.ext", "v3_attest.ext")
    #shutil.copy("v3_attest_new.ext", "v3_attest.ext")

PrintStep("Ready to create \"Attestation Certificate\" - showing the values to be used")
PrintAttestCertHash(attestcakey)
if ShowSteps:
    time.sleep(3)

PrintStep("Creating a NEW 2048 bit private/public key pair")

MySystem("%s genrsa -out %sattest.key %s 2048" % (OPENSSL_CMD, ImageFilenameNoExt, exponent==3 and "-3" or ""))

PrintStep("Making a certificate signing request (CSR) (will be in PEM format)")

posix_openssl_cmd=path_to_posix_shlex_ready(OPENSSL_CMD)

temp = "%s req -new -key %sattest.key -out %sattest.csr -subj " % (posix_openssl_cmd, ImageFilenameNoExt, ImageFilenameNoExt)

subject_str=''
for key in attestcert.keys():
    if debug['level']>=1:
        debug_str='DEBUG: qpsa.__main__ - attestcert key "{0}" >>>{1}<<<'.format(key,attestcert[key])
        print debug_str
        LOG.write(debug_str)

    if key.lower() in ["DEBUG".lower(),"CRASH_DUMP".lower()]:
        continue
    if not re.search(r'OU\d\d', key) is None:
        subject_str = "%s/OU=%s" % (subject_str, attestcert[key])
    else:
        subject_str = "%s/%s=%s" % (subject_str, key, attestcert[key])

if debug['level']>=1:
    debug_str='DEBUG: qpsa.__main__ - subject string is >>>{0}<<<'.format(subject_str)
    print debug_str
    LOG.write(debug_str)

temp = "%s \"%s\" -days 7300 -config opensslroot.cfg" % (temp,subject_str)

MySystem(temp, posix=True)

PrintStep("Creating the ATTESTATION certificate (signed with AttestCA key)")

if certchainsize == 2:
    cmds = "%s x509 -req -in %sattest.csr -CA %s -CAkey %s -outform DER -out %s-attestation_cert.cer -days 7300 -set_serial 38758 -extfile v3_attest.ext" % \
            (OPENSSL_CMD, ImageFilenameNoExt, rootcacert, rootcakey, OutputFilename)
else:
    cmds = "%s x509 -req -in %sattest.csr -CA %s -CAkey %s -outform DER -out %s-attestation_cert.cer -days 7300 -set_serial 38758 -extfile v3_attest.ext" % \
            (OPENSSL_CMD, ImageFilenameNoExt, attestcacert, attestcakey, OutputFilename)

if attestcert_sha1sign is False:
    cmds = cmds + " -sha256"
MySystem(cmds)

PrintStep("Creating the **SIGNATURE** for %sPhaseTwo.bin based on new ATTESTATION CERTIFICATE" % ImageFilenameNoExt)

output_str=None
if "sha1".lower() in SigCalc["HASH_ALG"].lower():
    output_str=external_call("%s dgst -sha1 -binary %sPhaseTwo.bin" % (OPENSSL_CMD, ImageFilenameNoExt))
else:
    output_str=external_call("%s dgst -sha256 -binary %sPhaseTwo.bin" % (OPENSSL_CMD, ImageFilenameNoExt))

try:
    sighash_bin_file=open('sighash.bin','wb')
    sighash_bin_file.write(output_str)
    sighash_bin_file.close()
except Exception, e:
    error_str='ERROR: qpsa.__main__ - Error writing to sighash.bin: {0}'.format(e)
    log_and_leave(error_str)

if certchainsize == 1:
    MySystem("%s rsautl -sign -pkcs -in sighash.bin -inkey %s -out %s-signature.bin" % (OPENSSL_CMD, rootcakey, OutputFilename))
else:
    MySystem("%s rsautl -sign -pkcs -in sighash.bin -inkey %sattest.key -out %s-signature.bin" % (OPENSSL_CMD, ImageFilenameNoExt, OutputFilename))


#PrintStep("VERIFYING THE CERTIFICATE CHAIN (optional) - Need to first convert all certs to PEM format")
#MySystem("openssl x509 -inform DER -in %s-root_cert.cer -outform PEM -out %s-root_cert.crt" % (OutputFilename, OutputFilename))
#MySystem("openssl x509 -inform DER -in %s-attestation_ca_cert.cer -outform PEM -out %s-attestation_ca_cert.crt" % (OutputFilename, OutputFilename))

PrintStep("VERIFYING THE CERTIFICATE CHAIN (optional) - Combine AttestCA + Attest CERT into ALL.CRT")
MySystem("%s x509 -inform DER -in %s-attestation_cert.cer -outform PEM -out %s-attestation_cert.crt" % \
        (OPENSSL_CMD, OutputFilename, OutputFilename))
if certchainsize == 3:
    concat_files(target='all.crt',sources=["{0}-attestation_ca_cert.crt".format(OutputFilename),"{0}-attestation_cert.crt".format(OutputFilename)])

PrintStep("VERIFYING THE CERTIFICATE CHAIN (optional) - VERIFY ROOT CERT - output should say \"OK\"")
MySystem("%s verify -CAfile %s-root_cert.crt %s-root_cert.crt" % (OPENSSL_CMD, OutputFilename, OutputFilename))

LookForOKinFile()

if certchainsize == 3:
    PrintStep("VERIFYING THE CERTIFICATE CHAIN (optional) - VERIFY ATTEST CA CERT - output should say \"OK\"")
    MySystem("%s verify -CAfile %s-root_cert.crt %s-attestation_ca_cert.crt" % (OPENSSL_CMD, OutputFilename, OutputFilename))
    LookForOKinFile()

PrintStep("VERIFYING THE CERTIFICATE CHAIN (optional) - VERIFY ATTEST CERT (all.crt) - output should say \"OK\"")
if certchainsize == 2:
    MySystem("%s verify -CAfile %s-root_cert.crt %s-attestation_cert.crt" % (OPENSSL_CMD, OutputFilename, OutputFilename))
else:
    MySystem("%s verify -CAfile %s-root_cert.crt all.crt" % (OPENSSL_CMD, OutputFilename))
LookForOKinFile()

## ZIP FILE IS HERE ----------------------------------------------------------
PrintStep("Final phase is Zipping up the contents")
print "\n\%s=%s\n" % (OutputFilename, OutputFilename)

#MySystem("zip zipfile %s-signature.bin %s-root_cert.cer %s-attestation_ca_cert.cer %s-attestation_cert.cer %sattest.key" % (OutputFilename, OutputFilename, OutputFilename, OutputFilename, ImageFilenameNoExt))
zip_file_list = ["%s-signature.bin" % OutputFilename, "%sattest.key" % ImageFilenameNoExt]
if certchainsize == 2:
    zip_file_list.append("%s-attestation_cert.cer" % OutputFilename)
elif certchainsize == 3:
    zip_file_list.append("%s-attestation_cert.cer" % OutputFilename)
    zip_file_list.append("%s-attestation_ca_cert.cer" % OutputFilename)

# Initialize arcname_list from zip_file_list
arc_name_list = None
if multirootcert_packager is not None:
    arc_name_list = list(zip_file_list)
    rootCertList = multirootcert_packager.getRootCerts()
    for rootcert in rootCertList:
        zip_file_list.append(rootcert)
        file, ext = os.path.splitext(rootcert)
        index = file.find(multirootcert_packager.getRootPrefix())
        if (index <= 0):
            print "Root cert %s does not contain root prefix %s" % (rootcert, multirootcert_packager.getRootPrefix())
            LOG.write("Root cert %s does not contain root prefix %s" % (rootcert, multirootcert_packager.getRootPrefix()))
            LOG.close()
            sys.exit(1)
        arc_name_list.append("%s-root_cert%s.cer" % (OutputFilename, file[index+multirootcert_packager.getRootPrefixLen():]))
else:
    zip_file_list.append("%s-root_cert.cer" % OutputFilename)
zip_files("zipfile.zip", zip_file_list, arc_name_list)

PrintStep("Renaming default ZIP file to %s.zip" % OutputFilename)

dst_zip_file_path="%s.zip" % OutputFilename
safe_mv_file('zipfile.zip',dst_zip_file_path)

## Final step is to move the zip file to the correct directory, i.e. $ImagePath
## First make sure old file doesn't exist there, if so delete it
if not os.path.exists(ImagePath):
    os.mkdir(ImagePath)

dst_zip_file_path="%s%s.zip" % (ImagePath, OutputFilename)
OutputFilename_zip_path="%s.zip" % (OutputFilename)

safe_mv_file(OutputFilename_zip_path,dst_zip_file_path)
CleanUp()

PrintStep("OUTPUT STORED AT: %s" % (dst_zip_file_path))
PrintStep("Check %s for LOG" % LogFileName)

print "\n\n----------------------------------------------------------"
print "\n DONE Qualcomm Platform Signing Application Code Signing V2.0"
print "\n----------------------------------------------------------\n"

print "\n"
LOG.close()
sys.exit(0)
