#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
import sectools.common.crypto.crypto_functions as crypto_functions
from sectools.common.utils import c_misc
import re
from sectools.common.utils.c_attribute import Attribute
from sectools.common.utils.c_misc import properties_repr

class Certificate(object):
    SIGNATTR_HW_ID = "HW_ID"
    SIGNATTR_OEM_ID = "OEM_ID"
    SIGNATTR_MODEL_ID = "MODEL_ID"
    SIGNATTR_SW_ID = "SW_ID"
    SIGNATTR_DEBUG = "DEBUG"
    SIGNATTR_CRASH_DUMP = "CRASH_DUMP"
    SIGNATTR_ROT_EN = "ROT_EN"
    SIGNATTR_APP_ID = "APP_ID"
    SIGNATTR_SW_SIZE = "SW_SIZE"
    SIGNATTR_SHA256 = "SHA256"
    SIGNATTR_TCG_MIN = "TCG_MIN"
    SIGNATTR_TCG_MAX = "TCG_MAX"

    #OID defined for TCG in attestation certificate
    TCG_OID = "1.3.6.1.4.1.1449.9.6.3"

    '''
    Certificate class
    inputs:
        cert_blob: certificate binary blob (DER/PEM)
        path: certificate binary path (DER/PEM)
        cert_blob will take precedence of cert_path

    Attributes for query:
        tcg_min
        tcg_max
    '''
    def __init__(self, cert_blob=None, path=None):
        if cert_blob is not None:
            self.cert_blob = cert_blob
        elif path is not None:
            self.cert_blob = c_misc.load_data_from_file(path)
        else:
            raise RuntimeError("cert_blob and path cannot be both None")

        self.certificate_text = crypto_functions.get_certificate_text_from_binary(self.cert_blob)
        self.asn1_text = crypto_functions.get_asn1_text_from_binary(self.cert_blob)

        self.tcg_min, self.tcg_max = self._get_tcg(self.asn1_text)


    def _get_tcg(self, asn1_text):
        _tcg_min = None
        _tcg_max = None

        tcg_ext = asn1_text.find(self.TCG_OID)
        if tcg_ext > -1:
            tcg_pattern=re.compile(r'\[HEX DUMP\]:([a-fA-F\d]+)')
            tcg_match=tcg_pattern.search(asn1_text[tcg_ext:])
            if tcg_match is not None:
                #tcg_str = tcg_match.group(1).replace(' ', '') #remove spaces
                tcg_str = tcg_match.group(1)
                _tcg_min = Attribute.init(num_bits=32, string="0x"+tcg_str[0:8])
                _tcg_max = Attribute.init(num_bits=32, string="0x"+tcg_str[8:16])

        return _tcg_min, _tcg_max

    def _get_ou_attr(self, attr_name, certificate_text):
        attr = None

        ou_id = Certificate_OU.GetID(attr_name)
        attr_pattern=re.compile(r'OU={0} ([a-fA-F\d]+) {1}'.format(ou_id, attr_name))
        attr_match=attr_pattern.search(certificate_text)
        if attr_match is not None:
            attr_str = attr_match.group(1)
            if len(attr_str) in [4, 8, 16]:
                attr = Attribute.init(num_bits = len(attr_str)*4, string="0x"+attr_str)
            else:
                attr = attr_str

        return attr

    # Return Attribute object
    def get_attr(self, attr_name):
        attr = None
        if attr_name == self.SIGNATTR_TCG_MIN:
            attr = self.tcg_min
        elif attr_name == self.SIGNATTR_TCG_MAX:
            attr = self.tcg_max
        else:
            attr = self._get_ou_attr(attr_name, self.certificate_text)

        return attr

    # Return attribute as string
    def get_attr_str(self, attr_name):
        attr = self.get_attr(attr_name)
        return None if attr is None else attr.str

    # Return attribute as int/long value
    def get_attr_value(self, attr_name):
        attr = self.get_attr(attr_name)
        return None if attr is None else attr.value

    def _get_exponent(self, certificate_text):
        _exponent = None
        exponent_pattern=re.compile(r'Exponent: (.*) ')
        exponent_match=exponent_pattern.search(certificate_text)
        if exponent_match is not None:
            exponent_str = exponent_match.group(1)
            _exponent = int(exponent_str)

        return _exponent

    def _repr_properties(self):
        properties = [
                      (self.SIGNATTR_SW_ID, self.sw_id),
                      (self.SIGNATTR_HW_ID, self.hw_id),
                      (self.SIGNATTR_DEBUG, self.debug),
                      (self.SIGNATTR_OEM_ID, self.oem_id),
                      (self.SIGNATTR_SW_SIZE, self.sw_size),
                      (self.SIGNATTR_MODEL_ID,  self.model_id),
                      (self.SIGNATTR_SHA256, self.sha256),
                      (self.SIGNATTR_APP_ID, self.app_id),
                      (self.SIGNATTR_CRASH_DUMP, self.crash_dump),
                      (self.SIGNATTR_ROT_EN, self.rot_en),
                      ('Exponent', self.exponent),
                      ('TCG_MIN', self.tcg_min),
                      ('TCG_MAX', self.tcg_max)
                     ]
        return properties

    def __repr__(self):
        return properties_repr(self._repr_properties())

    @property
    def tcg_min(self):
        return self._tcg_min

    @tcg_min.setter
    def tcg_min(self, value):
        self._tcg_min = value

    @property
    def tcg_max(self):
        return self._tcg_max

    @tcg_max.setter
    def tcg_max(self, value):
        self._tcg_max = value

    @property
    def hw_id(self):
        return self.get_attr(self.SIGNATTR_HW_ID)

    @property
    def oem_id(self):
        return self.get_attr(self.SIGNATTR_OEM_ID)

    @property
    def model_id(self):
        return self.get_attr(self.SIGNATTR_MODEL_ID)

    @property
    def sw_id(self):
        return self.get_attr(self.SIGNATTR_SW_ID)

    @property
    def debug(self):
        return self.get_attr(self.SIGNATTR_DEBUG)

    @property
    def crash_dump(self):
        return self.get_attr(self.SIGNATTR_CRASH_DUMP)

    @property
    def rot_en(self):
        return self.get_attr(self.SIGNATTR_ROT_EN)

    @property
    def app_id(self):
        return self.get_attr(self.SIGNATTR_APP_ID)

    @property
    def exponent(self):
        return self._get_exponent(self.certificate_text)

    @property
    def sw_size(self):
        return self.get_attr_value(self.SIGNATTR_SW_SIZE)

    @property
    def sha256(self):
        sha256 = self.get_attr_value(self.SIGNATTR_SHA256)
        return True if sha256 == 1 else False

    @property
    def text(self):
        return self.certificate_text

class Certificate_OU(object):
    OU_DICT = {Certificate.SIGNATTR_SW_ID:"01",
               Certificate.SIGNATTR_HW_ID: "02",
               Certificate.SIGNATTR_DEBUG: "03",
               Certificate.SIGNATTR_OEM_ID: "04",
               Certificate.SIGNATTR_SW_SIZE: "05",
               Certificate.SIGNATTR_MODEL_ID: "06",
               Certificate.SIGNATTR_SHA256: "07",
               Certificate.SIGNATTR_APP_ID: "08",
               Certificate.SIGNATTR_CRASH_DUMP: "09",
               Certificate.SIGNATTR_ROT_EN: "10"}

    @classmethod
    def _validate(cls, attribute_name):
        if not cls.OU_DICT.has_key(attribute_name):
            raise RuntimeError("Attribute name {0} is not supported in Certificate_OU".format(attribute_name))

    @classmethod
    def GetID(cls, attribute_name):
        cls._validate(attribute_name)

        return cls.OU_DICT.get(attribute_name)

    @classmethod
    def GetText(cls, attribute_name, attribute):
        cls._validate(attribute_name)
        assert(isinstance(attribute, Attribute))

        ou_text = "{0} {1} {2}".format(cls.OU_DICT.get(attribute_name),
                             attribute.str[2:], #exclude 0x
                             attribute_name)
        return ou_text