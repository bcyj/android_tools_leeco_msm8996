#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""
Created on Feb 12, 2013

@author: hraghav

This module provides a simple interface to send email.
"""

from c_logging import logger
from email import Encoders
from email.MIMEText import MIMEText
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email.utils import COMMASPACE
import os
import platform
import smtplib
import sys
import traceback


class CoreEmail(object):
    """
    This class contains methods that provide a simple interface to the email
    capabilities of python. The methods should be used directly without
    creating a instance of the class.
    """

    @classmethod
    def sendMail(cls, sender, recipients, subject, body, filesList=[], recipients_cc=[], recipients_bcc=[]):
        """
        Parameters:
        1. sender (str): Email address or name to be specified as the sender.
        2. recipients (str): Comma separated email addresses for "to".
        3. subject (str): Subject of the email.
        4. body (str): Body of the email.
        5. filesList (str): List of file paths to attach in the email.
        6. recipients_cc (str): Comma separated email addresses for "cc".
        7. recipients_bcc (str): Comma separated email addresses for "bcc".

        Return:
        1. returnValue (bool): success/failure
        2. returnError (str): error if sending email failed.
        """

        returnValue = True
        returnError = ''

        # Append the sender's machine name to the end of the email.
        body = ''.join([
                        body,
                        '\n',
                        'This is an automated message from "{0}"'.format(platform.node())
                        ])

        # Create the multi-part email message
        msg = MIMEMultipart()
        msg['Subject'] = subject
        msg['From'] = sender
        msg['To'] = COMMASPACE.join(recipients)
        msg['Cc'] = COMMASPACE.join(recipients_cc)
        msg['Bcc'] = COMMASPACE.join(recipients_bcc)
        msg.attach(MIMEText(body))

        for eachFile in filesList:
            try:
                part = MIMEBase('application', "octet-stream")
                part.set_payload(open(eachFile, "rb").read())
                Encoders.encode_base64(part)
                part.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(eachFile))
                msg.attach(part)
            except Exception:
                logger.debug(traceback.format_exc())
                logger.warning('Could not attach file to email: {0}'.format(eachFile))

        # Create connection to the SMTP server and send the email
        try:
            server = smtplib.SMTP('qcmail1.qualcomm.com')
        except Exception:
            logger.debug(traceback.format_exc())
            returnValue = False
            returnError = sys.exc_info()[1]
        else:
            server.starttls()
            try:
                server.sendmail(sender, recipients + recipients_cc + recipients_bcc, msg.as_string())
            except Exception:
                logger.debug(traceback.format_exc())
                returnValue = False
                returnError = sys.exc_info()[1]
            server.quit()

        return returnValue, returnError
