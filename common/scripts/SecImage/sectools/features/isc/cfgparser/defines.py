#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Contains constants related to the cfgparser package.

Constants relate to:

#. Common constants
#. Auto-generated XML Parser - auto_gen_xml_config
#. Auto-generated OBJ - auto_gen_obj_config
#. Config Data Structure
"""


import os

from sectools.common.utils import c_config, c_path


#------------------------------------------------------------------------------
# Common constants
#------------------------------------------------------------------------------
# Absolute path of the package
PACKAGE_PATH = os.path.dirname(os.path.realpath(__file__))

# Name of the root node of any config related objects
ROOTNODE_NAME = 'secimage'

# Name of the directory containing chip specific folders
CONFIG_DIR_BASENAME = 'config'


#------------------------------------------------------------------------------
# Auto-generated XML Parser related information
#------------------------------------------------------------------------------
# XML - Basic information
XML_PY_FILE_NAME = 'auto_gen_xml_config.py'
XML_PY_PATH = c_path.join(PACKAGE_PATH, XML_PY_FILE_NAME)
XML_NAME_ENDING = '_' + ROOTNODE_NAME + '.xml'

# XML - lines at the beginning of the file
XML_PREPEND_LINES = '<?xml version="1.0" encoding="UTF-8"?>\n'

# XML - rootnode related constants
XML_ROOTNODE_NAMESPACE = 'tns:'
XML_ROOTNODE_NAME = ROOTNODE_NAME
XML_ROOTNODE = XML_ROOTNODE_NAMESPACE + XML_ROOTNODE_NAME

# XML - namespace related constants
XML_NAMESPACE_TNS = 'xmlns:tns="http://www.qualcomm.com/secimage"'
XML_NAMESPACE_W3 = 'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
XML_NAMESPACE_SCHEMA = 'xsi:schemaLocation="http://www.qualcomm.com/secimage ../xsd/secimage.xsd"'
XML_NAMESPACE = XML_NAMESPACE_TNS + '\n\t' + XML_NAMESPACE_W3 + '\n\t' + XML_NAMESPACE_SCHEMA

# XML - classname generator
XML_CLASSNAME_GEN = lambda x: 'complex_' + x


#------------------------------------------------------------------------------
# Auto-generated OBJ Parser related information
#------------------------------------------------------------------------------
# OBJ - Basic information
OBJ_PY_FILE_NAME = 'auto_gen_obj_config.py'
OBJ_PY_PATH = c_path.join(PACKAGE_PATH, OBJ_PY_FILE_NAME)

# OBJ - lines at the beginning of the file
OBJ_PREPEND_LINES = '"""\nAuto generated classes\n"""\n\n'

# OBJ - classname generator
OBJ_CLASSNAME_GEN = lambda x: 'Cfg_' + x.replace('_', ' ').title().replace(' ', '_')


#------------------------------------------------------------------------------
# Config Data Structure
#------------------------------------------------------------------------------
# cert_config block
CONFIG_KEY_CERT_CONFIG = 'cert_config'
CONFIG_STRUCTURE_CERT_CONFIG = \
{
    'id' : '',

    c_config.DICT.DictHandlerKey_Choice :
    {
        'root_cert_choice' :
        {
            'root_cert' :
            {
                'use_preexisting_cert' : True,
                'params_list' :
                {
                    c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
                    'cert_param' : [''],
                },
                'preexisting_cert' :
                {
                    c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
                    'cert_path' : c_config.PATH,
                    'private_key_path' : c_config.PATH,
                },
            },

            'multirootcert' :
            {
                'directory' : c_config.PATH,
                'index' : 0,
                'root_cert_name' : '',
                'root_key_name' : '',
                'attest_ca_cert_name' : ('',),
                'attest_ca_key_name' : ('',),
            },
        },
    },

    'attest_ca_cert' :
    {
        c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
        'use_preexisting_cert' : True,
        'params_list' :
        {
            c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
            'cert_param' : [''],
        },
        'preexisting_cert' :
        {
            c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
            'cert_path' : c_config.PATH,
            'private_key_path' : c_config.PATH,
        },
    },

    'attest_cert' :
    {
        'use_preexisting_cert' : True,
        'params_list' :
        {
            c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
            'cert_param' : [''],
        },
        'preexisting_cert' :
        {
            c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
            'cert_path' : c_config.PATH,
            'private_key_path' : c_config.PATH,
        },
    },
}

# general properties block
CONFIG_KEY_GENERAL_PROPERTIES = 'general_properties'
CONFIG_STRUCTURE_GENERAL_PROPERTIES = \
{
    'max_cert_size' : 0,
    'key_size' : 0,
    'num_certs_in_certchain' : 0,
    'num_root_certs' : 0,
    'testsig_serialnum' : ('',),
}

# signing default attributes
CONFIG_KEY_DEFAULT_ATTRIBUTES = 'default_attributes'
CONFIG_STRUCTURE_DEFAULT_ATTRIBUTES = \
{
    'msm_part' : '',
    'oem_id' : '',
    'model_id' : '',
    'sw_id' : ('',),
    'app_id' : ('',),
    'crash_dump' : ('',),
    'rot_en' : ('',),
    'debug' : ('',),
    'exponent' : 0,
    'tcg_min' : ('',),
    'tcg_max' : ('',),
}

# image type block
CONFIG_KEY_IMAGE_TYPE = 'image_type'
CONFIG_STRUCTURE_IMAGE_TYPE = \
{
    'id' : '',
    'file_type' : '',

    'mbn_properties' :
    {
        c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
        'header_size' : 0,
        'preamble_size' : (0,),
        'has_magic_num' : False,
        'ota_enabled' : False,
        'page_size' : (0,),
        'num_of_pages' : (0,),
        'min_size_with_pad' : (0,),
    },

    'elf_properties' :
    {
        c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
        'image_type' : 0,
    },

    'ewm_properties' :
    {
        c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
        'image_entry' : '',
        'relocatable' : False,
    },
}


# top-level block
CONFIG_KEY = ROOTNODE_NAME
CONFIG_STRUCTURE = \
{
    'metadata' :
    {
        'chipset' : '',
        'version' :  '',
    },

    CONFIG_KEY_GENERAL_PROPERTIES : CONFIG_STRUCTURE_GENERAL_PROPERTIES,

    'parsegen' :
    {
        'image_types_list' :
        {
            CONFIG_KEY_IMAGE_TYPE : [CONFIG_STRUCTURE_IMAGE_TYPE],
        }
    },

    'signing' :
    {
        'selected_signer' : (('local', 'csms', 'cass',),),

        CONFIG_KEY_DEFAULT_ATTRIBUTES : CONFIG_STRUCTURE_DEFAULT_ATTRIBUTES,

        'signer_attributes' :
        {
            'local_signer_attributes' :
            {
                'openssl_config_inputs' :
                {
                    'attestation_certificate_extensions_path' : c_config.PATH,
                    'ca_certificate_extensions_path' : c_config.PATH,
                    'openssl_configfile_path' : c_config.PATH,
                },

                'cert_configs_list' :
                {
                    'selected_cert_config' : '',
                    CONFIG_KEY_CERT_CONFIG :
                    [
                        CONFIG_STRUCTURE_CERT_CONFIG
                    ],
                },
            },

            'cass_signer_attributes' :
            {
                c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],

                'server' :
                {
                    c_config.DICT.DictHandlerKey_Attr : [c_config.DICT.DictHandlerAttr_Optional],
                    'host' : '',
                    'port' : 0,
                    'trust_keystore' : c_config.PATH,
                    'trust_keystore_type' : (('JKS', 'PKCS12',),),
                    'trust_keystore_password' : ('',),
                    'host_validation_mode' : ((('TEST', 'STAGE_PROD',),),),
                },

                'user_identity' :
                {
                    'token_identity_slot' : (0,),
                    'token_password' : ('',),
                    'keystore_file' : (c_config.PATH,),
                    'keystore_type' : (('PKCS11', 'PKCS12',),),
                },

                'capability' : '',
            },
        },
    },

    'encryption' :
    {
        'selected_encryptor' : ('',),
        'unified_encryption' :
        {
             'use_file_interface' : False,
             'L1_encryption_key' : (c_config.PATH,),
             'L2_encryption_key' : (c_config.PATH,),
             'L3_encryption_key' : (c_config.PATH,),
        }
    },

    'post_process' :
    {
        'pil_splitter' : '',
    },

    'images_list' :
    {
        'image' :
        [
            {
                'sign_id' : '',
                'name' : ('',),
                'image_type' : '',
                CONFIG_KEY_CERT_CONFIG : ('',),
                'pil_split' : (False,),
                'meta_build_location' : '',
                'general_properties_overrides' :
                {
                    'max_cert_size' : (0,),
                    'key_size' : (0,),
                    'num_certs_in_certchain' : (0,),
                    'num_root_certs' : (0,),
                    'testsig_serialnum' : ('',),
                },
                'signing_attributes_overrides' :
                {

                    'msm_part' : ('',),
                    'oem_id' : ('',),
                    'model_id' : ('',),
                    'sw_id' : ('',),
                    'app_id' : ('',),
                    'crash_dump' : ('',),
                    'rot_en' : ('',),
                    'debug' : ('',),
                    'exponent' : (0,),
                    'tcg_min' : ('',),
                    'tcg_max' : ('',),
                },
                'post_process_commands' : ('',),
            }
        ]
    }
}

