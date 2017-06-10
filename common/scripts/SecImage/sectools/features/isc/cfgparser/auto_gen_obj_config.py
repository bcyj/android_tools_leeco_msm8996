#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""
Auto generated classes
"""

class Cfg_Secimage:

    def __init__(self):
        self.parsegen = Cfg_Parsegen()
        self.signing = Cfg_Signing()
        self.images_list = Cfg_Images_List()
        self.encryption = Cfg_Encryption()
        self.general_properties = Cfg_General_Properties()
        self.post_process = Cfg_Post_Process()
        self.metadata = Cfg_Metadata()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Parsegen:

    def __init__(self):
        self.image_types_list = Cfg_Image_Types_List()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Image_Types_List:

    def __init__(self):
        self.image_type = []

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Image_Type:

    def __init__(self):
        self.file_type = ''
        self.ewm_properties = Cfg_Ewm_Properties()
        self.mbn_properties = Cfg_Mbn_Properties()
        self.id = ''
        self.elf_properties = Cfg_Elf_Properties()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Ewm_Properties:

    def __init__(self):
        self.image_entry = ''
        self.relocatable = False

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Mbn_Properties:

    def __init__(self):
        self.min_size_with_pad = 0
        self.preamble_size = 0
        self.page_size = 0
        self.has_magic_num = False
        self.num_of_pages = 0
        self.ota_enabled = False
        self.header_size = 0

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Elf_Properties:

    def __init__(self):
        self.image_type = 0

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Signing:

    def __init__(self):
        self.default_attributes = Cfg_Default_Attributes()
        self.signer_attributes = Cfg_Signer_Attributes()
        self.selected_signer = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Default_Attributes:

    def __init__(self):
        self.model_id = ''
        self.sw_id = ''
        self.exponent = 0
        self.rot_en = ''
        self.app_id = ''
        self.tcg_max = ''
        self.tcg_min = ''
        self.msm_part = ''
        self.debug = ''
        self.oem_id = ''
        self.crash_dump = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Signer_Attributes:

    def __init__(self):
        self.cass_signer_attributes = Cfg_Cass_Signer_Attributes()
        self.local_signer_attributes = Cfg_Local_Signer_Attributes()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Cass_Signer_Attributes:

    def __init__(self):
        self.capability = ''
        self.server = Cfg_Server()
        self.user_identity = Cfg_User_Identity()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Server:

    def __init__(self):
        self.trust_keystore_password = ''
        self.host = ''
        self.host_validation_mode = ''
        self.trust_keystore = ''
        self.trust_keystore_type = ''
        self.port = 0

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_User_Identity:

    def __init__(self):
        self.token_identity_slot = 0
        self.keystore_type = ''
        self.keystore_file = ''
        self.token_password = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Local_Signer_Attributes:

    def __init__(self):
        self.openssl_config_inputs = Cfg_Openssl_Config_Inputs()
        self.cert_configs_list = Cfg_Cert_Configs_List()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Openssl_Config_Inputs:

    def __init__(self):
        self.openssl_configfile_path = ''
        self.attestation_certificate_extensions_path = ''
        self.ca_certificate_extensions_path = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Cert_Configs_List:

    def __init__(self):
        self.cert_config = []
        self.selected_cert_config = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Cert_Config:

    def __init__(self):
        self.attest_ca_cert = Cfg_Attest_Ca_Cert()
        self.attest_cert = Cfg_Attest_Cert()
        self.root_cert = Cfg_Root_Cert()
        self.multirootcert = Cfg_Multirootcert()
        self.id = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Attest_Ca_Cert:

    def __init__(self):
        self.use_preexisting_cert = True
        self.params_list = Cfg_Params_List()
        self.preexisting_cert = Cfg_Preexisting_Cert()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Params_List:

    def __init__(self):
        self.cert_param = []

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Preexisting_Cert:

    def __init__(self):
        self.cert_path = ''
        self.private_key_path = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Attest_Cert:

    def __init__(self):
        self.use_preexisting_cert = True
        self.params_list = Cfg_Params_List()
        self.preexisting_cert = Cfg_Preexisting_Cert()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Root_Cert:

    def __init__(self):
        self.use_preexisting_cert = True
        self.params_list = Cfg_Params_List()
        self.preexisting_cert = Cfg_Preexisting_Cert()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Multirootcert:

    def __init__(self):
        self.index = 0
        self.root_cert_name = ''
        self.attest_ca_cert_name = ''
        self.attest_ca_key_name = ''
        self.directory = ''
        self.root_key_name = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Images_List:

    def __init__(self):
        self.image = []

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Image:

    def __init__(self):
        self.post_process_commands = ''
        self.name = ''
        self.general_properties_overrides = Cfg_General_Properties_Overrides()
        self.meta_build_location = ''
        self.image_type = ''
        self.pil_split = False
        self.cert_config = ''
        self.signing_attributes_overrides = Cfg_Signing_Attributes_Overrides()
        self.sign_id = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_General_Properties_Overrides:

    def __init__(self):
        self.num_certs_in_certchain = 0
        self.key_size = 0
        self.max_cert_size = 0
        self.num_root_certs = 0
        self.testsig_serialnum = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Signing_Attributes_Overrides:

    def __init__(self):
        self.model_id = ''
        self.sw_id = ''
        self.exponent = 0
        self.rot_en = ''
        self.app_id = ''
        self.tcg_max = ''
        self.tcg_min = ''
        self.msm_part = ''
        self.debug = ''
        self.oem_id = ''
        self.crash_dump = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Encryption:

    def __init__(self):
        self.selected_encryptor = ''
        self.unified_encryption = Cfg_Unified_Encryption()

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Unified_Encryption:

    def __init__(self):
        self.use_file_interface = False
        self.L3_encryption_key = ''
        self.L1_encryption_key = ''
        self.L2_encryption_key = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_General_Properties:

    def __init__(self):
        self.num_certs_in_certchain = 0
        self.key_size = 0
        self.max_cert_size = 0
        self.num_root_certs = 0
        self.testsig_serialnum = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Post_Process:

    def __init__(self):
        self.pil_splitter = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


class Cfg_Metadata:

    def __init__(self):
        self.chipset = ''
        self.version = ''

    def __str__(self):
        return 'Instance of ' + self.__class__.__name__

    def __repr__(self):
        retval = ''
        for attr in sorted(self.__dict__.keys()):
            value = self.__dict__[attr]
            retval += (str(attr) + ': ' + '\n'
                       '    ' + repr(value) + '\n')
        return (self.__class__.__name__ + ':' + '\n' +
                '\n'.join(['  ' + line for line in retval.split('\n')]))


