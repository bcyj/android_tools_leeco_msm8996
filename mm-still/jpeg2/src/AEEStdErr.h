/*======================================================================
Copyright (C) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
====================================================================== */
#ifndef AEESTDERR_H
#define AEESTDERR_H

//
// Basic Error Codes
//
//

#define  AEE_SUCCESS              0  // no error
#define  AEE_EFAILED              1  // general failure
#define  AEE_ENOMEMORY            2  // insufficient RAM
#define  AEE_ECLASSNOTSUPPORT     3  // specified class unsupported
#define  AEE_EVERSIONNOTSUPPORT   4  // version not supported
#define  AEE_EALREADYLOADED       5  // object already loaded
#define  AEE_EUNABLETOLOAD        6  // unable to load object/applet
#define  AEE_EUNABLETOUNLOAD      7  // unable to unload object/applet
#define  AEE_EALARMPENDING        8  // alarm is pending
#define  AEE_EINVALIDTIME         9  // invalid time
#define  AEE_EBADCLASS            10 // NULL class object
#define  AEE_EBADMETRIC           11 // invalid metric specified
#define  AEE_EEXPIRED             12 // App/Component Expired
#define  AEE_EBADSTATE            13 // invalid state
#define  AEE_EBADPARM             14 // invalid parameter
#define  AEE_ESCHEMENOTSUPPORTED  15 // invalid URL scheme
#define  AEE_EBADITEM             16 // invalid item
#define  AEE_EINVALIDFORMAT       17 // invalid format
#define  AEE_EINCOMPLETEITEM      18 // incomplete item
#define  AEE_ENOPERSISTMEMORY     19 // insufficient flash
#define  AEE_EUNSUPPORTED         20 // API is not supported
#define  AEE_EPRIVLEVEL           21 // privileges are insufficient for this operation
#define  AEE_ERESOURCENOTFOUND    22
#define  AEE_EREENTERED           23
#define  AEE_EBADTASK             24
#define  AEE_EALLOCATED           25 // App/Module left memory allocated when released
#define  AEE_EALREADY             26 // operation is already in progress
#define  AEE_EADSAUTHBAD          27 // ADS mutual authorization failed
#define  AEE_ENEEDSERVICEPROG     28 // need service programming
#define  AEE_EMEMPTR              29 // bad memory pointer
#define  AEE_EHEAP                30 // heap corruption
#define  AEE_EIDLE                31 // context (system, interface, etc.) is idle
#define  AEE_EITEMBUSY            32 // context (system, interface, etc.) is busy
#define  AEE_EBADSID              33 // invalid subscriber ID
#define  AEE_ENOTYPE              34 // no type detected/found
#define  AEE_ENEEDMORE            35 // need more data/info
#define  AEE_EADSCAPS             36 // ADS Capabilities do not match those required for phone
#define  AEE_EBADSHUTDOWN         37 // App failed to close properly
#define  AEE_EBUFFERTOOSMALL      38 // destination buffer given is too small
#define  AEE_ENOSUCH              39 // no such name/port/socket/service exists or valid
#define  AEE_EACKPENDING          40 // ACK pending on application
#define  AEE_ENOTOWNER            41 // not an owner authorized to perform the operation
#define  AEE_EINVALIDITEM         42 // current item is invalid
#define  AEE_ENOTALLOWED          43 // not allowed to perform the operation
#define  AEE_EBADHANDLE           44 // invalid handle
#define  AEE_EOUTOFHANDLES        45 // out of handles
#define  AEE_EINTERRUPTED         46 // waitable call is interrupted
#define  AEE_ENOMORE              47 // no more items available -- reached end
#define  AEE_ECPUEXCEPTION        48 // a CPU exception occurred
#define  AEE_EREADONLY            49 // Cannot change read-only object or parameter
// a moratorium on adding to AEEStdErr.h is in effect, 50 and later are
//  already spoken for

#define  AEE_EWOULDBLOCK         516 // Operation would block if not non-blocking; wait and try again

/*
============================================================================
   ERRORS DOCUMENTATION
==============================================================================

Error Codes

Description:
This topic lists the categories of error codes that Brew MP returns. The topic for each
category of error code includes the name of each error, the code that is associated with
the error, and a description of the error.

===H2>
List of Error Code Types
===/H2>
===p>
The categories of error codes include: ~
~

AddrBook error codes  ~
AddrInfo error codes ~
AEE_IS_REMOTE_ERR(): ~
AEE_IS_REMOTE_ERR_PRE(): ~
Basic AEE Error Codes   ~
Database error codes  ~
dbc Error Codes ~
DNS Resolver error codes ~
File error codes  ~
FS AEE Error Codes ~
ICamera error codes  ~
ICMP error codes ~
ILicenseSystem Error Codes ~
Indeterminate errors: (transport failure) ~
ISQL Error Codes ~
ISVGDOM Error Codes: ~
ISSL error codes  ~
IX509Chain error codes   ~
ModCollector Errors  ~
ModInstallerCntx Errors ~
ModMover Errors ~
Multimedia error codes  ~
Network AEE error codes ~
Network subsystem error codes ~
pim_IMessageStore Error Codes ~
pim_IRecordStore Error Codes ~
Port AEE Error Codes  ~
PosDet error codes  ~
Post-invocation errors: (remote errors) ~
Pre-invocation errors: (remote errors) ~
QoS error codes ~
Remote error codes: ~
SSL error codes  ~
VOCODER error codes  ~
VolumeDB Errors   ~
Web error codes ~

*

==================================================================
Basic AEE Error Codes

Description:
This section lists the set of basic AEE errors returned, the codes associated
with the errors, and descriptions of the errors.

Definition:

Error                   Code   Description

AEE_SUCCESS             0      operation Successful
AEE_EFAILED             1      general failure
AEE_ENOMEMORY           2      insufficient RAM
AEE_ECLASSNOTSUPPORT    3      specified class unsupported
AEE_EVERSIONNOTSUPPORT  4      version not supported
AEE_EALREADYLOADED      5      object already loaded
AEE_EUNABLETOLOAD       6      unable to load object/applet
AEE_EUNABLETOUNLOAD     7      unable to unload object/applet
AEE_EALARMPENDING       8      alarm is pending
AEE_EINVALIDTIME        9      invalid time
AEE_EBADCLASS           10     NULL class object
AEE_EBADMETRIC          11     invalid metric specified
AEE_EEXPIRED            12     Application/Component Expired
AEE_EBADSTATE           13     invalid state
AEE_EBADPARM            14     invalid parameter
AEE_ESCHEMENOTSUPPORTED 15     invalid URL scheme
AEE_EBADITEM            16     invalid item
AEE_EINVALIDFORMAT      17     invalid format
AEE_EINCOMPLETEITEM     18     incomplete item
AEE_ENOPERSISTMEMORY    19     insufficient flash
AEE_EUNSUPPORTED        20     API is not supported
AEE_EPRIVLEVEL          21     application privileges are insufficient for this operation
AEE_ERESOURCENOTFOUND   22     unable to find specified resource
AEE_EREENTERED          23     non re-entrant API re-entered
AEE_EBADTASK            24     API called in wrong task context
AEE_EALLOCATED          25     Application/Module left memory allocated when released
AEE_EALREADY            26     operation is already in progress
AEE_EADSAUTHBAD         27     ADS mutual authorization failed
AEE_ENEEDSERVICEPROG    28     need service programming
AEE_EMEMPTR             29     bad memory pointer
AEE_EHEAP               30     heap corruption
AEE_EIDLE               31     context (system, interface, etc.) is idle
AEE_EITEMBUSY           32     context (system, interface, etc.) is busy
AEE_EBADSID             33     invalid subscriber ID
AEE_ENOTYPE             34     no type detected/found
AEE_ENEEDMORE           35     need more data/info
AEE_EADSCAPS            36     capabilities do not match those required
AEE_EBADSHUTDOWN        37     application failed to close properly
AEE_EBUFFERTOOSMALL     38     destination buffer given is too small
AEE_ENOSUCH             39     no such name/port/socket/service exists or valid
AEE_EACKPENDING         40     ACK pending on application
AEE_ENOTOWNER           41     not an owner authorized to perform the operation
AEE_EINVALIDITEM        42     current item is invalid
AEE_ENOTALLOWED         43     not allowed to perform the operation
AEE_EBADHANDLE          44     invalid handle
AEE_EOUTOFHANDLES       45     out of handles
AEE_EINTERRUPTED        46     waitable call is interrupted
AEE_ENOMORE             47     no more items available -- reached end
AEE_ECPUEXCEPTION       48     a CPU exception occurred
AEE_EREADONLY           49     cannot change read-only object or parameter
AEE_EWOULDBLOCK         516    operation would block if not non-blocking; wait and try again

Comments:
These Brew MP error codes have an up-to-date naming convention, and replace older BREW error
codes that use a naming convention that did not include the "AEE_" prefix.

See Also:
  Error Codes

==================================================================
*/
#endif /* #ifndef AEESTDERR_H */

