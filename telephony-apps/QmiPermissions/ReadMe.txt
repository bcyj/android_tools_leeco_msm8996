Copyright (C) 2012 Qualcomm Technologies, Inc. All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.


**Do Not remove** QComQMIPermissions.apk from its current location.

Apk to be included in baseline image
--------------------------------------
This apk needs to be included as is and should not be re-signed with a different certificate.
The apk is already signed with a certificate that is owned by Qualcomm Technologies Inc.

This apk should be included in the device system image in order to grant QMI access to QChat
application, which will also be signed with the same certificate.

Secure QMI access
-------------------------
QComQMIPermissions.apk grants secure QMI access to the QChat apk (which can be downloaded
from Google play) through signature verification.
