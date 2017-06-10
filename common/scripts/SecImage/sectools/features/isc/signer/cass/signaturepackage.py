#Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
#Qualcomm Technologies Proprietary and Confidential.
from xml.etree import ElementTree
import os
from signatureresponse import SignatureResponse

class SignaturePackage(object):
    def __init__(self, signaturePackage = None):
        self.signaturePackage = signaturePackage
        self.signatureResponseList = []
        if self.signaturePackage is not None:
            self.initFromSignaturePackage()

    def getSPDigest(self):
        return self.SPDigest

    def getCount(self):
        return self.count

    def getSignatureResponse(self, sequenceId):
        if (sequenceId == 0):
            raise RuntimeError, "getSignatureResponse: sequenceId is 0!"
        return self.signatureResponseList[sequenceId-1]

    def initFromSignaturePackage(self):
        if self.signaturePackage is None:
            raise RuntimeError, "initFromSignaturePackage: signaturePackage is not initialized"

        self.root = ElementTree.fromstring(self.signaturePackage)
        self.count = 0
        self.SPDigest = None
        encoding = self.root.attrib['encoding']

        for child in self.root:
            #print child.tag, child.attrib, child.text
            if child.tag == 'Count':
                self.count = int(child.text)
            elif child.tag == 'SPDigest':
                if child.attrib['digestAlgorithm'] != "SHA-256":
                    raise RuntimeError, "initFromSignaturePackage: digestAlgorithm %s is not supported" % child.attrib['digestAlgorithm']
                self.SPDigest = child.text
            elif child.tag == 'SignatureResponse':
                signatureResponse = SignatureResponse(child, child.attrib['sequenceId'], encoding)
                self.signatureResponseList.append(signatureResponse)

        if self.count == 0:
            raise RuntimeError, "initFromSignaturePackage: signaturePackage has Count = 0!"
        if self.SPDigest is None:
            raise RuntimeError, "initFromSignaturePackage: SPDigest is Null!"


    def readFromFile(self, fileName):
        if not os.path.isfile(fileName):
            raise RuntimeError, "readFromFile: %s does not exist" % fileName
        file = open(fileName, "rt")
        self.signaturePackage = file.read()
        file.close()

        self.initFromSignaturePackage()

    def release(self):
        return 1

    def __del__(self):
        self.release()
