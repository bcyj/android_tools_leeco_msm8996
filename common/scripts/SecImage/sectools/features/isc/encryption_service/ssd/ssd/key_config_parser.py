#============================================================================
#
# key_config_parser.py
#
# GENERAL DESCRIPTION
#    Contains methods to parse a TrustZone keystore
#
# Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#----------------------------------------------------------------------------
#
#                      EDIT HISTORY FOR FILE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
# when       who       what, where, why
# --------   ---       ------------------------------------------------------
# 02/19/13   pre       Initial revision
#
#============================================================================
import xml.etree.ElementTree as ET
import os

def get_test_key_id(key_name):
    if (key_name == "rsa device private" or
        key_name == "rsa device public"):
        return ("\x7B\xCE\xFC\x9C\x9B\xE8\x0F\x70"
                "\xBC\x4D\xF3\x8C\x39\xCD\x10\xD7"
                "\x73\x83\xDA\x7C\x97\xD6\x32\xDD"
                "\x5B\x28\xAD\x1C\x47\xA8\x70\xC7")
    elif (key_name == "rsa oem public" or
          key_name == "rsa oem private"):
        return ("\x34\x55\xbf\xbe\x50\xdf\x55\x4f"
                "\x29\xaa\x6c\xf5\x24\xe7\x31\xbc"
                "\xf8\x5a\x2c\x74\xcc\xc3\x26\xc5"
                "\x36\x8d\xb9\x0d\x84\x39\x1c\x5f")
    elif (key_name == "aes device"):
        return ("\x54\xF5\x2E\x22\xA6\xC9\xA2\x6C"
                "\xC9\x02\x0E\x87\xD7\x98\x74\x34"
                "\xC2\x1F\x9B\x64\xAD\x7A\x76\x89"
                "\x5F\xCA\xCB\xBD\xE7\x61\x0C\xC6")
    else:
        print("Error: Unsupported key name passed to get_test_key_id, \"" +
              key_name + "\"")

def is_test_key(key_name, keyid_fn):
    with open(keyid_fn, "rb") as fo:
        return fo.read() == get_test_key_id(key_name)

    print("Error: Couldn't open key id file: " + keyid_fn)
    return True

def get_key_md(key_name, key_type, key_id_fn):
    if (key_type != "test" and key_type != "prod"):
        print("Error: key type \"" + key_type + "\" not supported")
        return ''

    if (key_name == "rsa device private"):
        if (key_type == "test"):
            return "\x03\x00\x00\x00"
        elif (key_type == "prod"):
            return "\x04\x00\x00\x00"

    elif (key_name == "rsa oem public"):
        if (key_type == "test"):
            return "\x01\x00\x00\x00"
        elif (key_type == "prod"):
            return "\x02\x00\x00\x00"

    elif (key_name == "aes device"):
        if (key_type == "test"):
            return "\x05\x00\x00\x00"
        elif (key_type == "prod"):
            return "\x06\x00\x00\x00"

    else:
        print("Error: Unsupported key name passed to get_key_md, \"" +
              key_name + "\"")
        return ''

#-----------------------------------------------------------------------------
# Reads an XML keystore configuration file and returns a list of
# lists with the following format:
#
# ["key filename," "key id filename," "key metadata value"]
#-----------------------------------------------------------------------------
def get_ssd_keystore(config_file):
    keystore = []

    if (not os.path.exists(config_file)):
        print("Keystore configuration file does not exist: " + config_file)
        return keystore

    config_file_dir = os.path.dirname(config_file)

    # Read the configuration file
    tree = ET.parse(config_file)
    root = tree.getroot()

    for child in root.findall("key"):
        key = []

        key_fn    = ''
        key_id_fn = ''
        key_md    = ''

        if ('name' not in child.attrib):
            print("Config Error: Keys must include a \"name\" attribute in "
                  "the configuration file.")
            exit(1)

        if ('type' not in child.attrib):
            print("Config Error: Keys must include a \"type\" attribute in "
                  "the configuration file.")
            exit(1)


        if ((child.attrib['name'] != "rsa device private") and
            (child.attrib['name'] != "rsa oem public") and
            (child.attrib['name'] != "aes device")):
            if ((child.attrib['name'] != "rsa device public") and
                (child.attrib['name'] != "rsa oem private")):
                print("Config Error: Unsupported key name \"" +
                      child.attrib['name'] + "\" in configuration file.")
                exit(1)
            continue

        if (child.find("path") == None):
            print("Config Error: No path tag for " +
                  child.attrib['name'] + " key.")
            exit(1)

        if (child.find("id_path") == None):
            print("Config Error: No ID path tag for " +
                  child.attrib['name'] + " key.")
            exit(1)

        key_fn = os.path.join(config_file_dir, child.find("path").text)
        key_fn = os.path.normpath(key_fn)

        key_id_fn = os.path.join(config_file_dir,
                                 child.find("id_path").text)
        key_id_fn = os.path.normpath(key_id_fn)

        if (not os.path.exists(key_fn)):
            print("Error: Cannot find key file: " + key_fn)
            exit(1)
        if (not os.path.exists(key_id_fn)):
            print("Error: Cannot find key id file: " + key_id_fn)
            exit(1)

        if (child.attrib['type'] == "prod"):
            if (is_test_key(child.attrib['name'], key_id_fn)):
                print("Error: Cannot use \"" + child.attrib['type'] + "\" " +
                      "key type for provided test key:\n" +
                      os.path.abspath(key_fn))
                exit(1)

        key_md = get_key_md(child.attrib['name'],
                            child.attrib['type'],
                            key_id_fn)
        if (key_md == ''):
            print("Error: get_key_md failed for:\n" + key_fn)
            exit(1)

        key.append(key_fn)
        key.append(key_id_fn)
        key.append(key_md)
        keystore.append(key)

    return keystore

def get_file(config_file, key_name, element):

    if (not os.path.exists(config_file)):
        print("Keystore configuration file does not exist: " + config_file)
        return ''

    config_file_dir = os.path.split(config_file)[0]

    # Read the configuration file
    tree = ET.parse(config_file)
    root = tree.getroot()

    for child in root.findall("key"):
        fn = ''

        if ('name' not in child.attrib):
            print("Config Error: Keys must include a \"name\" attribute in "
                  "the configuration file.")
            return ''

        if ('type' not in child.attrib):
            print("Config Error: Keys must include a \"type\" attribute in "
                  "the configuration file.")
            return ''

        if (child.attrib['name'] != key_name):
            continue

        if (child.find(element) == None):
            print("Config Error: No path tag for " +
                  child.attrib['name'] + " key.")
            return ''

        fn = os.path.join(config_file_dir, child.find(element).text)
        fn = os.path.normpath(fn)

        if (not os.path.exists(fn)):
            print("Config Error: Cannot find file: " + fn)
            return ''

        key_id_fn = os.path.join(config_file_dir, child.find("id_path").text)
        key_id_fn = os.path.normpath(key_id_fn)

        if (child.attrib['type'] == "prod"):
            if (is_test_key(child.attrib['name'], key_id_fn)):
                print("Error: Cannot use \"" + child.attrib['type'] + "\" " +
                      "key type for provided test key:\n" +
                      os.path.abspath(fn))
                exit(1)


        return fn

    print("Config Error: Cannot find " + key_name)
    return ''

def get_buffer(config_file, key_name, element):

    buf = ''

    fn = get_file(config_file, key_name, element)

    if (fn != ''):
        with open(fn, "rb") as fo:
            buf = fo.read()

    return buf.rstrip()

# Used for encryption
def get_rsa_pub_dvc_key(config_file):
    return get_file(config_file, "rsa device public", "path")
def get_aes_dvc_key(config_file):
    return get_file(config_file, "aes device", "path")

# Used for signing during encryption
def get_rsa_pri_oem_key(config_file):
    return get_file(config_file, "rsa oem private", "path")

# Used for both keystore and encryption
def get_rsa_dvc_key_id_buf(config_file):
    return get_buffer(config_file, "rsa device private", "id_path")
def get_aes_dvc_key_id_buf(config_file):
    return get_buffer(config_file, "aes device", "id_path")
def get_rsa_oem_key_id_buf(config_file):
    return get_buffer(config_file, "rsa oem public", "id_path")

