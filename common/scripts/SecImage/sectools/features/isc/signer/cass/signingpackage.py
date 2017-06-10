#Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
#Qualcomm Technologies Proprietary and Confidential.
from signingrequest import SigningRequest
from xml.etree.ElementTree import Element, SubElement, tostring
import hashlib

BASE64_ENCODING = "Base64"

class SigningPackage(object):
    def __init__(self, sourceId):
        self.encoding = BASE64_ENCODING
        self.count = 0
        self.signingRequestList = []
        self.sourceId = sourceId
        self.sp = None

    def createSigningRequest(self, description):
        SR = SigningRequest(self.count+1, self.sourceId, description)
        self.signingRequestList.append(SR)
        self.count = self.count + 1
        return SR

    def toxml(self):
        sp = Element('SigningPackage', {'xmlns:xsi':'http://www.w3.org/2001/XMLSchema-instance'})
        count = SubElement(sp, 'Count')
        count.text = str(self.count)
        encoding = SubElement(sp, 'Encoding')
        encoding.text = BASE64_ENCODING
        if len(self.signingRequestList)>0:
            for SR in self.signingRequestList:
                sp.append(SR.getXmlElement())
        self.sp = sp
        return tostring(sp)

    def saveToFile(self, fileName):
        if self.sp is None:
            self.toxml()
        file = open(fileName, "w")
        file.write(tostring(self.sp))
        file.close()

    def getDigest(self):
        if self.sp is None:
            self.toxml()
        m = hashlib.sha256(tostring(self.sp))
        return m.hexdigest()

    def release(self):
        return 1

    def __del__(self):
        self.release()
