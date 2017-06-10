#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from abc import ABCMeta, abstractproperty

class BaseKeystore(object):
    __metaclass__=ABCMeta

    def __init__(self, password):
        self.password = password

    @abstractproperty
    def file(self):
        """
        Returns the keystore file location
        """
        raise RuntimeError('Method must be implemented by the derived class')

    @abstractproperty
    def type(self):
        """
        Returns the keystore type
        """
        raise RuntimeError('Method must be implemented by the derived class')

    @property
    def password(self):
        return self._password

    @password.setter
    def password(self, value):
        if value:
            self._password = value
        else:
            self._password = self._get_user_password()

    def _get_user_password(self):
        import getpass
        user_passphrase=getpass.getpass(self.MESG_PROMPT_PASSWORD)
        return user_passphrase
