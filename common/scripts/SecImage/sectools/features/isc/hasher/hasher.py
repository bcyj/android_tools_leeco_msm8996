#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from sectools.common.utils.c_logging import logger
import hashlib
import binascii


class Hasher(object):
    '''
    Hasher service. Provides various hashing implementations.
    '''


    def __init__(self):
        '''
        Constructor
        '''
        pass


    def qcom_hmac(self, data, hmac_params):
        if data == None or hmac_params == None:
            raise RuntimeError('Input parameters to the HMAC function are incorrect')
        else:
            msm_id = hmac_params.MSM_ID
            sw_id = hmac_params.SW_ID
            ipad = 0x3636363636363636
            opad = 0x5C5C5C5C5C5C5C5C
            logger.debug("MSM_ID key : " + repr(msm_id))
            logger.debug("SW_ID key : " + repr(sw_id))
            logger.debug("ipad : " + repr(ipad))
            logger.debug("opad : " + repr(opad))
            Si = sw_id ^ ipad
            Si = binascii.unhexlify(format(Si,'x'))
            So = msm_id ^ opad
            So = binascii.unhexlify(format(So,'x'))
            msg_step1 = hashlib.sha256(data).hexdigest()
            msg_step1_bin = binascii.a2b_hex(msg_step1)
            logger.debug2("H(code image) : " + msg_step1)
            msg_step2 = hashlib.sha256(Si + msg_step1_bin).hexdigest()
            msg_step2_bin = binascii.a2b_hex(msg_step2)
            logger.debug2("H[(SWID^ipad) || H(code image)] : " + msg_step2)
            msg_step3 = hashlib.sha256(So + msg_step2_bin).hexdigest()
            msg_step3_bin = binascii.a2b_hex(msg_step3)
            logger.debug2("H[(MSMID^opad) || H[(SWID^ipad) || H(code image)]] : " + msg_step3)

            hmac = msg_step3_bin
            return hmac

