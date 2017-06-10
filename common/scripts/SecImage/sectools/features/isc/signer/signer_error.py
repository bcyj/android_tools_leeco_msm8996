#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

class ExternalSignerError(Exception):
    """
    Exception class to keep the code clean.
    Uses the CoreErrorCode enums to specify the error.
    """

    def __init__(self, errorString):
        """
        Sets the error number and the srror string

        Parameters:
        errorString (str): String that clearly defines the error.
        """
        self.stringData = errorString

    def __str__(self):
        """ Returns the string representation of the CoreError object """
        return self.stringData
