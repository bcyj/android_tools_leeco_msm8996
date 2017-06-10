# Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

import os
import filecmp

ROOTCERTNAME_FORMAT = 'qpsa_rootca%d.cer'
ROOTPREFIX = '_rootca'

def check_file_exists(filename):
    return os.path.isfile(filename) and os.path.getsize(filename) > 0

class Packager:
    def __init__(self, total_num_root_certs, rootcerts_dir):
        if total_num_root_certs < 1 or total_num_root_certs > 16:
            raise RuntimeError, "Total number of root certs supported is from 1 to 16. Input is %d" % total_num_root_certs

        if os.path.exists(rootcerts_dir) == False:
            raise RuntimeError, "%s does not exist." % rootcerts_dir

        # Check rootcerts exist
        for i in range (0, total_num_root_certs-1):
            if check_file_exists(os.path.join(rootcerts_dir, ROOTCERTNAME_FORMAT % i)) == False:
                raise RuntimeError, "%s does not exist." % os.path.join(rootcerts_dir, ROOTCERTNAME_FORMAT % i)

        self.total_num_root_certs = total_num_root_certs
        self.rootcerts_dir = rootcerts_dir

    def getRootCerts(self):
        rootCertList = []
        for i in range (0, self.total_num_root_certs):
            rootCertList.append(os.path.join(self.rootcerts_dir, ROOTCERTNAME_FORMAT % i))

        return rootCertList

    def getRootPrefix(self):
        return ROOTPREFIX

    def getRootPrefixLen(self):
        return len(ROOTPREFIX)

    def containRootCert(self, root_cert):
        print "root_cert = %s" % root_cert
        for i in range (0, self.total_num_root_certs):
            print "Comparing to %s" % os.path.join(self.rootcerts_dir, ROOTCERTNAME_FORMAT % i)
            if filecmp.cmp(os.path.join(self.rootcerts_dir, ROOTCERTNAME_FORMAT % i), root_cert, False) is True:
                return True

        return False

    def getRootCertDir(self):
        return self.rootcerts_dir

    def release(self):
        return 1

    def __del__(self):
        self.release()
