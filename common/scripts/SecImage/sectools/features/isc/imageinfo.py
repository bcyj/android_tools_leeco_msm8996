#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Feb 8, 2014

@author: hraghav
'''


import os

from cfgparser import ConfigParser
from cfgparser import defines as cfgdef
from sectools.common.utils import c_path
import defines


class ImagePath(object):

    def __init__(self, image_path=''):
        self._image_dir_base = os.path.dirname(image_path)
        self._image_dir_ext = ''
        self._image_name = os.path.basename(image_path)

    @property
    def image_dir_base(self):
        return self._image_dir_base

    @image_dir_base.setter
    def image_dir_base(self, image_dir_base):
        image_dir_base = c_path.normalize(image_dir_base)
        self._image_dir_base = image_dir_base

    @property
    def image_dir_ext(self):
        return self._image_dir_ext

    @image_dir_ext.setter
    def image_dir_ext(self, image_dir_ext):
        self._image_dir_ext = image_dir_ext

    @property
    def image_dir(self):
        return c_path.join(self.image_dir_base, self.image_dir_ext)

    @property
    def image_name(self):
        return self._image_name

    @image_name.setter
    def image_name(self, image_name):
        self._image_name = image_name

    @property
    def image_path(self):
        return c_path.join(self.image_dir, self.image_name)

    @property
    def image_name_base(self):
        return os.path.splitext(self.image_name)[0]

    @property
    def image_name_ext(self):
        return os.path.splitext(self.image_name)[1]


class StatusInfo(object):

    NOT_EXECUTED = 'NA'
    SUCCESS = 'T'
    ERROR = 'F'

    def __init__(self):
        self.state = self.NOT_EXECUTED
        self.error = ''


class ImageStatus(object):

    def __init__(self):
        self.parsegen = StatusInfo()
        self.integrity_check = StatusInfo()
        self.sign = StatusInfo()
        self.encrypt = StatusInfo()
        self.postprocess = StatusInfo()
        self.validate_parsegen = StatusInfo()
        self.validate_integrity_check = StatusInfo()
        self.validate_sign = StatusInfo()
        self.validate_encrypt = StatusInfo()
        self.overall = StatusInfo()


class DestImagePath(ImagePath):

    def __init__(self, image_path=''):
        ImagePath.__init__(self, image_path=image_path)
        self._debug_enable = False
        self._mid = ''

    @property
    def debug_enable(self):
        return self._debug_enable

    @debug_enable.setter
    def debug_enable(self, value):
        self._debug_enable = value

    @property
    def decrypted_file(self):
        file_name = (self.image_name_base +
                     defines.DEST_FILE_DECRYPTED +
                     self.image_name_ext)
        return c_path.join(self.image_dir, file_name)

    @property
    def to_sign(self):
        file_name = (self.image_name_base +
                     defines.DEST_FILE_TO_SIGN +
                     self.image_name_ext)
        return c_path.join(self.image_dir, file_name)

    @property
    def cert_folder(self):
        dir_name = defines.DEST_DIR_CERT_FOLDER
        return c_path.join(self.image_dir, dir_name)

    @property
    def debug_dir(self):
        return c_path.join(self.image_dir, defines.DEST_DEBUG_DIR) if self._debug_enable else None

    @property
    def debug_dir_parsegen(self):
        return c_path.join(self.debug_dir, self._mid, defines.DEST_DEBUG_DIR_PARSEGEN) if self._debug_enable else None

    @property
    def debug_dir_signer(self):
        return c_path.join(self.debug_dir, self._mid, defines.DEST_DEBUG_DIR_SIGNER) if self._debug_enable else None

    @property
    def debug_dir_encdec(self):
        return c_path.join(self.debug_dir, self._mid, defines.DEST_DEBUG_DIR_ENCDEC) if self._debug_enable else None

    @property
    def debug_file_parsegen_unsigned(self):
        file_name = (self.image_name_base +
                     defines.DEST_DEBUG_FILE_PARSEGEN_UNSIGNED +
                     self.image_name_ext)
        return c_path.join(self.debug_dir_parsegen, file_name) if self._debug_enable else None

    @property
    def debug_file_parsegen_tosign(self):
        file_name = (self.image_name_base +
                     defines.DEST_DEBUG_FILE_PARSEGEN_TOSIGN +
                     self.image_name_ext)
        return c_path.join(self.debug_dir_parsegen, file_name) if self._debug_enable else None

    @property
    def debug_file_parsegen_cert_chain(self):
        file_name = (self.image_name_base +
                     defines.DEST_DEBUG_FILE_PARSEGEN_CERT_CHAIN +
                     self.image_name_ext)
        return c_path.join(self.debug_dir_parsegen, file_name) if self._debug_enable else None

    @property
    def debug_file_parsegen_signature(self):
        file_name = (self.image_name_base +
                     defines.DEST_DEBUG_FILE_PARSEGEN_SIGNATURE +
                     self.image_name_ext)
        return c_path.join(self.debug_dir_parsegen, file_name) if self._debug_enable else None

    @property
    def debug_file_parsegen_integrity_check(self):
        file_name = (self.image_name_base +
                     defines.DEST_DEBUG_FILE_PARSEGEN_INTEGRITY_CHECK +
                     self.image_name_ext)
        return c_path.join(self.debug_dir_parsegen, file_name) if self._debug_enable else None

    @property
    def debug_file_parsegen_signed(self):
        file_name = (self.image_name_base +
                     defines.DEST_DEBUG_FILE_PARSEGEN_SIGNED +
                     self.image_name_ext)
        return c_path.join(self.debug_dir_parsegen, file_name) if self._debug_enable else None

    @property
    def debug_file_parsegen_encrypted(self):
        file_name = (self.image_name_base +
                     defines.DEST_DEBUG_FILE_PARSEGEN_ENCRYPTED +
                     self.image_name_ext)
        return c_path.join(self.debug_dir_parsegen, file_name) if self._debug_enable else None

    @property
    def debug_file_signer_root_cert(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_ROOT_CERT) if self._debug_enable else None

    @property
    def debug_file_signer_root_key(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_ROOT_KEY) if self._debug_enable else None

    @property
    def debug_file_signer_attestation_ca_cert(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_ATTESTATION_CA_CERT) if self._debug_enable else None

    @property
    def debug_file_signer_attestation_ca_key(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_ATTESTATION_CA_KEY) if self._debug_enable else None

    @property
    def debug_file_signer_attestation_cert(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_ATTESTATION_CERT) if self._debug_enable else None

    @property
    def debug_file_signer_attestation_key(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_ATTESTATION_KEY) if self._debug_enable else None

    @property
    def debug_file_signer_signature(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_SIGNATURE) if self._debug_enable else None

    @property
    def debug_file_signer_cert_chain(self):
        return c_path.join(self.debug_dir_signer, defines.DEST_DEBUG_FILE_SIGNER_CERT_CHAIN) if self._debug_enable else None


class ImageInfo(object):

    def __init__(self, image_path, sign_id, img_config_block, img_config_parser):
        import cfgparser.auto_gen_obj_config as agoc

        assert isinstance(image_path, str)
        assert isinstance(sign_id, str)
        assert isinstance(img_config_parser, ConfigParser)

        # Public attributes
        self.config_parser = img_config_parser
        self.config = img_config_parser.config_data
        self.src_image = ImagePath(image_path)
        self.dest_image = DestImagePath()
        self.image_under_operation = self.src_image.image_path
        self.name = self.src_image.image_name
        self.sign_id = sign_id
        self.status = ImageStatus()
        self.chipset = img_config_parser.chipset
        self.encryption = self.config.encryption
        self.encdec = None
        self.status.overall.state = StatusInfo.ERROR

        # Properties obtained from the config file
        self.pil_split = img_config_block.pil_split
        self.post_process_commands = img_config_block.post_process_commands
        self.general_properties_overrides = img_config_block.general_properties_overrides
        self.signing_attributes_overrides = img_config_block.signing_attributes_overrides

        # Properties to be looked up in the config file
        self.image_type = agoc.Cfg_Image_Type()
        self.cert_config = agoc.Cfg_Cert_Config()
        self.general_properties = agoc.Cfg_General_Properties()
        self.signing_attributes = agoc.Cfg_Default_Attributes()

        # Populate the properties from the config file
        self._populate_image_type(img_config_parser, img_config_block.image_type)
        self._populate_cert_config(img_config_parser, img_config_block.cert_config)
        self._populate_general_properties(img_config_parser)
        self._populate_signing_attributes(img_config_parser)


    #--------------------------------------------------------------------------
    # Helper functions
    #--------------------------------------------------------------------------
    def _populate_image_type(self, config_parser, image_type_id):
        assert isinstance(config_parser, ConfigParser)
        assert isinstance(image_type_id, str)

        # Search the id in the config
        config = config_parser.config_data
        for image_type in config.parsegen.image_types_list.image_type:
            if image_type.id == image_type_id:
                break
        else:
            raise RuntimeError('Config does not contain the image id: ' + image_type_id)

        # Transfer the contents of the config into the object
        self.image_type = config_parser.transfer_from_node_to_obj(
                            image_type,
                            self.image_type,
                            cfgdef.CONFIG_STRUCTURE_IMAGE_TYPE,
                            cfgdef.CONFIG_KEY_IMAGE_TYPE)

    def _populate_cert_config(self, config_parser, cert_config_id):
        assert isinstance(config_parser, ConfigParser)
        assert isinstance(cert_config_id, (str, type(None)))
        config = config_parser.config_data

        # Settle on the id to be used
        if cert_config_id is None:
            cert_config_id = config.signing.signer_attributes.local_signer_attributes.cert_configs_list.selected_cert_config

        # Search the id in the config
        for cert_config in config.signing.signer_attributes.local_signer_attributes.cert_configs_list.cert_config:
            if cert_config.id == cert_config_id:
                break
        else:
            raise RuntimeError('Config does not contain the cert config id: ' + cert_config_id)

        # Transfer the contents of the config into the object
        self.cert_config = config_parser.transfer_from_node_to_obj(
                             cert_config,
                             self.cert_config,
                             cfgdef.CONFIG_STRUCTURE_CERT_CONFIG,
                             cfgdef.CONFIG_KEY_CERT_CONFIG)

    def _populate_general_properties(self, config_parser):
        assert isinstance(config_parser, ConfigParser)

        # Transfer the contents of the base into the object
        config = config_parser.config_data
        self.general_properties = config_parser.transfer_from_node_to_obj(
                             config.general_properties,
                             self.general_properties,
                             cfgdef.CONFIG_STRUCTURE_GENERAL_PROPERTIES,
                             cfgdef.CONFIG_KEY_GENERAL_PROPERTIES)

        # Update using the overrides
        self._override_blocks(self.general_properties_overrides,
                              self.general_properties,
                              cfgdef.CONFIG_STRUCTURE_GENERAL_PROPERTIES)

    def _populate_signing_attributes(self, config_parser):
        assert isinstance(config_parser, ConfigParser)

        # Transfer the contents of the base into the object
        config = config_parser.config_data
        self.signing_attributes = config_parser.transfer_from_node_to_obj(
                             config.signing.default_attributes,
                             self.signing_attributes,
                             cfgdef.CONFIG_STRUCTURE_DEFAULT_ATTRIBUTES,
                             cfgdef.CONFIG_KEY_DEFAULT_ATTRIBUTES)

        # Update using the overrides
        self._override_blocks(self.signing_attributes_overrides,
                              self.signing_attributes,
                              cfgdef.CONFIG_STRUCTURE_DEFAULT_ATTRIBUTES)

    def _override_blocks(self, block_from, block_to, structure):
        for key in structure.keys():
            override = getattr(block_from, key, None)
            if override is not None:
                setattr(block_to, key, override)
        pass
