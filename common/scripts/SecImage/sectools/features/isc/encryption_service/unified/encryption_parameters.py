#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import os
import struct
import binascii
from sectools.common.crypto import crypto_functions
from sectools.common.utils.c_misc import hexdump
from sectools.features.isc.encryption_service.base.encryption_parameters import BaseEncryptionParameters


PREFIXED_L1_KEY = 'L1L1L1L1L1L1L1L1'

class EncryptionParameters(BaseEncryptionParameters):
    '''
    Implements the encryption parameters as defined in unified image encryption docs
    '''

    class ALGORITHM_TYPE:
        CEML_CIPHER_ALG_AES128            = 0x0
        CEML_CIPHER_ALG_AES256            = 0x1
        CEML_CIPHER_ALG_TRIPLE_DES        = 0x2

    class ALGORITHM_MODE_TYPE:
        CEML_CIPHER_MODE_ECB              = 0x0
        CEML_CIPHER_MODE_CBC              = 0x1
        CEML_CIPHER_MODE_CTR              = 0x2
        CEML_CIPHER_MODE_XTS              = 0x3
        CEML_CIPHER_MODE_CCM              = 0x4
        CEML_CIPHER_MODE_CMAC             = 0x5
        CEML_CIPHER_MODE_CTS              = 0x6


    class SECURITY_LEVEL:
        INVALID, EL_3, EL_2 = range(3)

    class FIELD_LEN_BYTES:
        MAGIC_FLD = 4
        VERSION_FLD = 4
        SIZE_FLD = 4
        KEY_SOURCE_ID_FLD = 4
        KEY_LADDER_LEN_FLD = 4
        KEY_LADDER_ALG_FLD = 4
        KEY_LADDER_MODE_FLD = 4
        L1_B0_FLG_FLD = 1
        L1_NONCE_FLD = 13
        L1_B0_Q_FLD = 2
        L1_B1_FLD = 16
        L2_ENC_KEY_FLD = 32
        L2_B0_FLG_FLD = 1
        L2_NONCE_FLD = 13
        L2_B0_Q_FLD = 2
        L2_B1_FLD = 16
        L3_ENC_KEY_FLD = 32
        L3_IMAGE_IV_FLD = 16




    L3_KEY_LEN_BYTES = 16
    L2_KEY_LEN_BYTES = 16
    L1_KEY_LEN_BYTES = 16
    L3_IV_LEN_BYTES = 16
    L2_IV_LEN_BYTES = 13
    L1_IV_LEN_BYTES = 13



    L1_B0_FLAGS_VAL = 0x79
    L1_B0_Q_VAL = 0x0010
    L1_B1_AAD_SIZE_VAL = 0x0010
    L1_B1_AAD_RSVD_LEN = 10
    L2_B0_FLAGS_VAL = 0x79
    L2_B0_Q_VAL = 0x0010
    L2_B1_AAD_SIZE_VAL = 0x0010
    L2_B1_AAD_RSVD_LEN = 2


    MAGIC_NUM = 0x89FECDAB
    VERSION = 1
    SIZE_FLD = (FIELD_LEN_BYTES.KEY_SOURCE_ID_FLD  +
            FIELD_LEN_BYTES.KEY_LADDER_LEN_FLD  +
            FIELD_LEN_BYTES.KEY_LADDER_ALG_FLD  +
            FIELD_LEN_BYTES.KEY_LADDER_MODE_FLD +
            FIELD_LEN_BYTES.L1_B0_FLG_FLD       +
            FIELD_LEN_BYTES.L1_NONCE_FLD        +
            FIELD_LEN_BYTES.L1_B0_Q_FLD         +
            FIELD_LEN_BYTES.L1_B1_FLD           +
            FIELD_LEN_BYTES.L2_ENC_KEY_FLD      +
            FIELD_LEN_BYTES.L2_B0_FLG_FLD       +
            FIELD_LEN_BYTES.L2_NONCE_FLD        +
            FIELD_LEN_BYTES.L2_B0_Q_FLD         +
            FIELD_LEN_BYTES.L2_B1_FLD           +
            FIELD_LEN_BYTES.L3_ENC_KEY_FLD      +
        FIELD_LEN_BYTES.L3_IMAGE_IV_FLD)

    KEY_SOURCE_ID = 1
    LADDER_DEPTH = 3
    KEY_LADDER_MODE = ALGORITHM_MODE_TYPE.CEML_CIPHER_MODE_CCM



    def __init__(self, encryption_params_blob=None, l1_key_blob=None, config=None, debug_dir=None):
        # Settle on params
        self.config=config
        if encryption_params_blob == None and l1_key_blob == None:
            encryption_params_blob, l1_key_blob = self._generate_new_encryption_params_blob()
        elif encryption_params_blob == None or l1_key_blob == None:
            raise RuntimeError("ERROR: Both encryption params and private key need to be provided")

        # Set the base params
        BaseEncryptionParameters.__init__(self, encryption_params_blob=encryption_params_blob, key=l1_key_blob, debug_dir=debug_dir)

        # Get the image encryption key & iv
        self._decode_encryption_parameters_blob(encryption_params_blob, l1_key_blob)

        if debug_dir:
            encryption_params_debug_filename = "enc_param.debug"
            with open(os.path.join(debug_dir, encryption_params_debug_filename),'wb') as debug_file:
                debug_file.write(self.__repr__())

    def get_image_encryption_key(self):
        return self._get_l3_key()

    def get_image_encryption_iv(self):
        return self.l3_image_iv

    def extract_encryption_parameters(self, data):
        encryption_parameters = ''
        length = len(self.encryption_params_blob)
        if len(data) > length and struct.unpack_from('I', data)[0] == self.MAGIC_NUM:
            encryption_parameters = data[:length]
            data = data[length:]
        return data, encryption_parameters

    def _generate_new_encryption_params_blob(self):
        self.l1_key = self._get_l1_key()
        self.l2_key = self._get_l2_key()
        self.l3_key = self._get_l3_key()


        self.magic_num = EncryptionParameters.MAGIC_NUM
        self.version = EncryptionParameters.VERSION
        self.size = EncryptionParameters.SIZE_FLD
        self.key_source_id = EncryptionParameters.KEY_SOURCE_ID
        self.ladder_depth = EncryptionParameters.LADDER_DEPTH
        self.ladder_alg = EncryptionParameters.ALGORITHM_TYPE.CEML_CIPHER_ALG_AES128
        self.key_ladder_mode = EncryptionParameters.KEY_LADDER_MODE
        self.l1_b0_flag = chr(EncryptionParameters.L1_B0_FLAGS_VAL)
        self.l1_nonce = "L1L1L1L1L1L1L"  #os.urandom(EncryptionParameters.L1_IV_LEN_BYTES)
        self.l1_nonce = bytes(self.l1_nonce)
        self.l1_b0_q_fld = EncryptionParameters.L1_B0_Q_VAL
        self.l1_b1_aad_size = EncryptionParameters.L1_B1_AAD_SIZE_VAL
        self.l1_b1_security_level = EncryptionParameters.SECURITY_LEVEL.EL_2
        self.l1_b1_reserved = ''.rjust(EncryptionParameters.L1_B1_AAD_RSVD_LEN, chr(0))

        self.l1_aad = struct.pack('=H',
                                  self.l1_b1_aad_size
                                  )
        self.l1_aad += struct.pack('=I',
                                   self.l1_b1_security_level)
        self.l1_aad += self.l1_b1_reserved
        (self.enc_l2_key, encryption_key, image_iv, hex_preexisting_aad) = crypto_functions.ccm_encrypt_binary(self.l2_key,
                                                                                                               binascii.hexlify(self.l1_key),
                                                                                                               binascii.hexlify(self.l1_nonce),
                                                                                                               binascii.hexlify(self.l1_aad))





        self.l2_b0_flag = chr(EncryptionParameters.L2_B0_FLAGS_VAL)
        self.l2_nonce = "L2L2L2L2L2L2L" #os.urandom(EncryptionParameters.L2_IV_LEN_BYTES)
        self.l2_nonce = bytes(self.l2_nonce)
        self.l2_b0_q_fld = EncryptionParameters.L2_B0_Q_VAL
        self.l2_b1_aad_size = EncryptionParameters.L2_B1_AAD_SIZE_VAL
        self.l2_b1_l3_alg = EncryptionParameters.ALGORITHM_TYPE.CEML_CIPHER_ALG_AES128
        self.l2_b1_l3_mode = EncryptionParameters.ALGORITHM_MODE_TYPE.CEML_CIPHER_MODE_CBC
        self.l2_b1_l3_alg_term_mode = EncryptionParameters.ALGORITHM_MODE_TYPE.CEML_CIPHER_MODE_CTS
        self.l2_b1_reserved = ''.rjust(EncryptionParameters.L2_B1_AAD_RSVD_LEN, chr(0))

        self.l2_aad = struct.pack('=H',
                                  self.l2_b1_aad_size,
                                  )
        self.l2_aad += struct.pack('=III',
                                   self.l2_b1_l3_alg,
                                   self.l2_b1_l3_mode,
                                   self.l2_b1_l3_alg_term_mode)
        self.l2_aad += self.l2_b1_reserved

        (self.enc_l3_key, encryption_key, image_iv, hex_preexisting_aad) = crypto_functions.ccm_encrypt_binary(self.l3_key,
                                                                                                               binascii.hexlify(self.l2_key),
                                                                                                               binascii.hexlify(self.l2_nonce),
                                                                                                               binascii.hexlify(self.l2_aad))



        self.l3_image_iv = "IVIVIVIVIVIVIVIV" #os.urandom(EncryptionParameters.L3_IV_LEN_BYTES)




        self.encryption_params_blob = struct.pack('=IIIIIIIc',
                                                  self.magic_num,
                                                  self.version,
                                                  self.size,
                                                  self.key_source_id,
                                                  self.ladder_depth,
                                                  self.ladder_alg,
                                                  self.key_ladder_mode,
                                                  self.l1_b0_flag,
                                                  )

        self.encryption_params_blob += self.l1_nonce

        self.encryption_params_blob += struct.pack('=H',
                                                   self.l1_b0_q_fld,
                                                   )
        self.encryption_params_blob += self.l1_aad

        self.encryption_params_blob += self.enc_l2_key

        self.encryption_params_blob += struct.pack('=c',
                                                   self.l2_b0_flag)

        self.encryption_params_blob += self.l2_nonce

        self.encryption_params_blob += struct.pack('=H',
                                                   self.l2_b0_q_fld,
                                                   )
        self.encryption_params_blob += self.l2_aad

        self.encryption_params_blob += self.enc_l3_key

        self.encryption_params_blob += self.l3_image_iv
        return self.encryption_params_blob, self.l1_key



    def _decode_encryption_parameters_blob(self, encryption_params_blob, l1_key_blob):


        self.l1_key = l1_key_blob

        string_offset = 0
        string_end = EncryptionParameters.FIELD_LEN_BYTES.MAGIC_FLD + EncryptionParameters.FIELD_LEN_BYTES.VERSION_FLD + EncryptionParameters.FIELD_LEN_BYTES.SIZE_FLD + EncryptionParameters.FIELD_LEN_BYTES.KEY_SOURCE_ID_FLD + EncryptionParameters.FIELD_LEN_BYTES.KEY_LADDER_LEN_FLD + EncryptionParameters.FIELD_LEN_BYTES.KEY_LADDER_ALG_FLD + EncryptionParameters.FIELD_LEN_BYTES.KEY_LADDER_MODE_FLD + EncryptionParameters.FIELD_LEN_BYTES.L1_B0_FLG_FLD

        (self.magic_num,
         self.version,
         self.size,
         self.key_source_id,
         self.ladder_depth,
         self.ladder_alg,
         self.key_ladder_mode,
         self.l1_b0_flag) = struct.unpack('=IIIIIIIc', self.encryption_params_blob[string_offset:string_end])

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L1_NONCE_FLD

        self.l1_nonce = self.encryption_params_blob[string_offset:string_end]

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L1_B0_Q_FLD

        (self.l1_b0_q_fld,) = struct.unpack('=H', self.encryption_params_blob[string_offset:string_end])

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L1_B1_FLD

        self.l1_aad = self.encryption_params_blob[string_offset:string_end]

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L2_ENC_KEY_FLD

        self.enc_l2_key = self.encryption_params_blob[string_offset:string_end]
        self.l2_key = crypto_functions.ccm_decrypt_binary(self.enc_l2_key,
                                                          binascii.hexlify(self.l1_key),
                                                          binascii.hexlify(self.l1_nonce),
                                                          binascii.hexlify(self.l1_aad))

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L2_B0_FLG_FLD

        (self.l2_b0_flag,) = struct.unpack('=c', self.encryption_params_blob[string_offset:string_end])

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L2_NONCE_FLD

        self.l2_nonce = self.encryption_params_blob[string_offset:string_end]

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L2_B0_Q_FLD

        (self.l2_b0_q_fld,) = struct.unpack('=H', self.encryption_params_blob[string_offset:string_end])

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L2_B1_FLD

        self.l2_aad = self.encryption_params_blob[string_offset:string_end]

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L3_ENC_KEY_FLD

        self.enc_l3_key = self.encryption_params_blob[string_offset:string_end]

        self.l3_key = crypto_functions.ccm_decrypt_binary(self.enc_l3_key,
                                                          binascii.hexlify(self.l2_key),
                                                          binascii.hexlify(self.l2_nonce),
                                                          binascii.hexlify(self.l2_aad))

        string_offset = string_end
        string_end = string_offset + EncryptionParameters.FIELD_LEN_BYTES.L3_IMAGE_IV_FLD

        self.l3_image_iv = self.encryption_params_blob[string_offset:string_end]

    def _get_l1_key(self):
        '''
        Returns L1 key
        '''
#         return os.urandom(EncryptionParameters.L1_KEY_LEN_BYTES)
        if self.config.unified_encryption.use_file_interface==False:
            return PREFIXED_L1_KEY
        else:
            try:
                with open(self.config.unified_encryption.L1_encryption_key, 'r') as f:
                    KEY=f.read()
                    return KEY
            except:
                raise RuntimeError("Cannot read from L1 key file")

    def _get_l2_key(self):
        '''
        Returns L2 key
        '''
#         return os.urandom(EncryptionParameters.L2_KEY_LEN_BYTES)

        if self.config.unified_encryption.use_file_interface==False:
            return 'L2L2L2L2L2L2L2L2'
        else:
            try:
                with open(self.config.unified_encryption.L2_encryption_key, 'r') as f:
                    KEY=f.read()
                    return KEY
            except:
                raise RuntimeError("Cannot read from L2 key file")

    def _get_l3_key(self):
        '''
        Returns L3 key
        '''
#         return os.urandom(EncryptionParameters.L3_KEY_LEN_BYTES)
        if self.config.unified_encryption.use_file_interface==False:
            return 'L3L3L3L3L3L3L3L3'
        else:
            try:
                with open(self.config.unified_encryption.L3_encryption_key, 'r') as f:
                    KEY=f.read()
                    return KEY
            except:
                raise RuntimeError("Cannot read from L3 key file")

    def __repr__(self, *args, **kwargs):

        string = ""
        string += "L1 KEY: \n" + hexdump(self.l1_key) + "\n"
        string += "L2 KEY: \n" + hexdump(self.l2_key) + "\n"
        string += "Encrypted L2 KEY: \n" + hexdump(self.enc_l2_key) + "\n"
        string += "L3 KEY: \n" + hexdump(self.l3_key) + "\n"
        string += "Encrypted L3 KEY: \n" + hexdump(self.enc_l3_key) + "\n"
        string += "L1 NONCE: \n" + hexdump(self.l1_nonce) + "\n"
        string += "L2 NONCE: \n" + hexdump(self.l2_nonce) + "\n"
        string += "L1 AAD: \n" + hexdump(self.l1_aad) + "\n"
        string += "L2 AAD: \n" + hexdump(self.l2_aad) + "\n"
        string += "Encryption Parameters Blob: \n" + hexdump(self.encryption_params_blob) + "\n"
        return string

