#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import os
import sys
import struct
import binascii
from sectools.common.crypto import crypto_functions, utility_functions
from sectools.common.utils.c_misc import hexdump
from sectools.features.isc.encryption_service.base.encryption_parameters import BaseEncryptionParameters
from sectools.features.isc.encryption_service.ssd import auto_gen_ssd_xml_config
from sectools.common.utils.c_config import CoreConfig
import ConfigParser

PACKAGE_PATH= os.path.dirname(os.path.realpath(__file__))
CONFIG_DIR_PATH=os.path.join(PACKAGE_PATH,'..','..','..','..','config','config')
SSD_DIR_PATH=os.path.join(PACKAGE_PATH,'ssd')
SSD_CONFIG_FILE_NAME='ssd_bin.cfg'
from sectools.features.isc.encryption_service.ssd.ssd import gen_ssd_bin
from sectools.features.isc.encryption_service.ssd.ssd import key_config_parser

IEK_ENC_ALGO_RSA_2048 = "RSA-2048"
WORD_BOUNDARY = 4
class EncryptionParameters(BaseEncryptionParameters):


    def __init__(self, encryption_params_blob=None, private_key=None, config=None, debug_dir=None):
        keycfg = os.path.join(SSD_DIR_PATH, "key_config.xml")
        self.key_config_file=keycfg
        self.ssd_p = gen_ssd_bin.SSDConfigClass(SSD_DIR_PATH, keycfg)

        # Settle on params
        config_file=ConfigParser.ConfigParser()
        config_file.read(os.path.join(SSD_DIR_PATH,SSD_CONFIG_FILE_NAME))
        self.iek_enc_algo = config_file.get('OEM_CONFIG','IEK_ENC_ALGO')

        if encryption_params_blob == None and private_key == None:
            encryption_params_blob = self._generate_new_encryption_params_blob()
            private_key = self._get_private_key_from_config_file()
        elif encryption_params_blob == None or private_key == None:
            raise RuntimeError("ERROR: Both encryption params and private key need to be provided")

        # Set the base params
        BaseEncryptionParameters.__init__(self, encryption_params_blob=encryption_params_blob, key=private_key, debug_dir=debug_dir)

        # Get the image encryption key & iv
        self.image_encryption_key, self.image_encryption_iv = \
            self._decode_encryption_parameters_blob(encryption_params_blob, private_key)

        if debug_dir:
            encryption_params_debug_filename = "enc_param.debug"
            with open(os.path.join(debug_dir, encryption_params_debug_filename),'wb') as debug_file:
                debug_file.write(self.__repr__())

    def get_image_encryption_key(self):
        return self.image_encryption_key

    def get_image_encryption_iv(self):
        return self.image_encryption_iv

    def extract_encryption_parameters(self, data):
        encryption_parameters = ''
        if data.startswith('<?xml version="1.0" encoding="UTF-8"?><SSD_METADATA>'):
            delim = '</SSD_METADATA>'
            split_data = data.split(delim)
            if len(split_data) >= 2:
                split_data = [seg + delim for seg in split_data[:-1]] + split_data[-1:]
                data = ''.join(split_data[1:])
                encryption_parameters = split_data[0]
            encryption_params_alignment_padding=len(encryption_parameters) % WORD_BOUNDARY
            if encryption_params_alignment_padding:
                data=data[WORD_BOUNDARY-encryption_params_alignment_padding:]
        return data, encryption_parameters

    #Using key config to get PVT key
    def _generate_new_encryption_params_blob(self):
        enc_xml_fname=utility_functions.store_data_to_temp_file('')
        self.ssd_p.gen_signed_ssd_xml(enc_xml_fname)
        enc_xml=utility_functions.get_data_from_file(enc_xml_fname)
        os.unlink(enc_xml_fname)
        return enc_xml

    def _get_private_key_from_config_file(self):
        if self.iek_enc_algo=="RSA-2048":
            return key_config_parser.get_buffer(self.key_config_file, "rsa device private", "path")
        else:
            return key_config_parser.get_buffer(self.key_config_file,  "aes device", "path")

    def _decode_encryption_parameters_blob(self, encryption_params_blob, private_key):
        encryption_params_blob=self.extract_encryption_parameters(encryption_params_blob)[1]
        tmp_config_file_path = utility_functions.store_data_to_temp_file(encryption_params_blob)
        encryption_params_parser = CoreConfig(auto_gen_ssd_xml_config, tmp_config_file_path).root
        encrypted_image_encryption_key_base64=encryption_params_parser.MD_SIGN[0].IEK_ENC_INFO[0].IEK_CIPHER_VALUE
        encrypted_image_encryption_key=binascii.a2b_base64(encrypted_image_encryption_key_base64)
        if self.iek_enc_algo==IEK_ENC_ALGO_RSA_2048:
            image_encryption_key=crypto_functions.decrypt_with_private_der_key(encrypted_image_encryption_key, private_key)
        else:
            image_encryption_iv_base64=encryption_params_parser.MD_SIGN[0].IEK_ENC_INFO[0].IEK_ENC_IV
            image_encryption_iv_bin=binascii.a2b_base64(image_encryption_iv_base64)
            image_encryption_iv_hex=binascii.hexlify(image_encryption_iv_bin)
            image_encryption_key=crypto_functions.cbc_decrypt_binary(encrypted_image_encryption_key, binascii.hexlify(private_key), image_encryption_iv_hex)
        image_encryption_iv_base64=encryption_params_parser.MD_SIGN[0].IMG_ENC_INFO[0].IMG_ENC_IV
        image_encryption_iv=binascii.a2b_base64(image_encryption_iv_base64)
        return image_encryption_key, image_encryption_iv

    def __repr__(self, *args, **kwargs):
        string = self.encryption_params_blob
        return string

