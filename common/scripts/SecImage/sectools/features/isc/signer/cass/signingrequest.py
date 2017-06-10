#Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
#Qualcomm Technologies Proprietary and Confidential.
from xml.etree.ElementTree import Element, SubElement, tostring

class SigningRequest(object):
    def __init__(self, sequenceId, sourceId, description):
        self.sequenceId = sequenceId
        self.sourceId = sourceId
        self.description = description
        self.attributeDict = dict()
        self.capability = ""
        self.digest = ""

    def setCapability(self, capability):
        self.capability = capability

    def setSigningAttribute(self, attributeName, attributeValue):
        #print "set %s=%s" % (attributeName, attributeValue)
        self.attributeDict.update({attributeName:attributeValue})

    def setDigest(self, digest):
        self.digest = digest

    def getXmlElement(self):
        sr = Element('SigningRequest', {'sequenceId':str(self.sequenceId)})
        sid = SubElement(sr, 'SourceID')
        sid.text = self.sourceId
        desc = SubElement(sr, 'Description')
        desc.text = self.description
        cap = SubElement(sr, 'Capability', {'name':self.capability})
        if len(self.attributeDict)>0:
            #print self.attributeDict
            attrs = SubElement(cap, 'SigningAttributes')
            for key in self.attributeDict:
                attr = SubElement(attrs, 'SigningAttribute', {'name':key})
                attr.text = self.attributeDict[key]
        digest = SubElement(sr, 'Digest')
        digest.text = self.digest
        #print self.prettify(sr)
        return sr

    def release(self):
        return 1

    def __del__(self):
        self.release()
