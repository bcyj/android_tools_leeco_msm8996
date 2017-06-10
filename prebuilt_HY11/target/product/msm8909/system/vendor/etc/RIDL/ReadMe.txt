LogKit II Version 4.5.10 Build
06/16/2015

This readme covers important information concerning the Remote
Image Distribution & Logging (RIDL) Kit

This LogKit II release is a standalone version of the RIDL client
which does not communicate with the RIDL web server and is provided
for use by Qualcomm MSM licensees.

Table of Contents

1. Installation notes
2. What's new in this release
3. Known issues

-------------------------------------------------------------------------------

1. RIDL LogKit II INSTALLATION NOTES

The following is required in order to utilize software included
with the RIDL Kit:

a. A Qualcomm phone supported by RIDL (as noted by release below)
b. Access to Qualcomm Technologies Incorporated meta builds
c. Software & hardware required to upload meta build to phone
   (details are noted per release)

Creating LogKit II Packages (INSTRUCTIONS FOR QUALCOMM CUSTOMER ENGINEERING)
----------------------------------------------------------------------------
1.   Get a current list of all mandatory rules by querying rockstar.qualcomm.com
   using mySQL Workbench (http://dev.mysql.com/downloads/workbench/5.2.html
   Contact RIDL.developers for credentials):

   select r.rule_id,r.name from group2rule as g
      inner join (select * from rule) as r on g.rule_OID = r.oid
   where g.Group_OID = 0;

   Example query results:

   504         [mandatory] get build desc
   505         [mandatory] get version AU rev
   506         [mandatory] get platform
   507         [mandatory] Diag health commands
   500         [mandatory] Hourly heartbeat
   508         [mandatory] Report Build ID
   509         [mandatory] Power Up
   501         [mandatory] Rule governor report
   502         [mandatory] Rule procesing error report
   503         [mandatory] Hourly location movement
   515         [mandatory] Detect RIDL crash

2. Assign a RIDL device to the customer's device group and reset or sync
   the RIDL app to freshen the RULES.db database.
   NOTE:  Use device group LogKit II (the default) if unknown

3. Tether the device over USB and repeat for each mandatory rule, replacing XXX
   with the rule ID:

   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db " delete from rules where ruleid = XXX and state = 4;"

4. Delete the previous LogKit II rules, then set the current RIDL rules to LogKit II rules:

   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db "delete from rules where state = 3;"
   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db "update rules set state = 3 where state = 4;"

5. Remove the Token, FileUpload, ActiveData, and ActiveRules entries:

   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db "delete from Token;"
   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db "delete from FileUpload;"
   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db "delete from ActiveData;"
   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db "delete from ActiveRules;"

6. Update the TransState state to state 1 (Idle state)

   adb shell su -c sqlite3 /data/misc/SelfHost/RIDL.db "update TransState set state = 1;"

7. Copy the Rules database from the RIDL phone:

   adb shell su -c pull /data/misc/SelfHost/RIDL.db .

8. Extract the current LogKit II RIDL_KIT.zip package (same location as this
   readme)

9. Replace the RIDL.db from the LogKit II package with the RIDL.db from
   the RIDL phone

10. Zip up the new package for the consumer

Consumer LogKit II Usage:
-----------------------
1. Install the package provided by Qualcomm support teams to OEM phones
2. Run field tests to collect LogKit II logs
3. Retrieve LogKit II logs from /data/SelfHost  E.g.:
   adb pull /data/misc/SelfHost/*.zip

-------------------------------------------------------------------------------

2. WHAT'S NEW

This Release (RIDL Kit 4.5.02 ENG Build) 06/03/2015

Component versions for this release:
   Diag Component 0.13
   QMI component 0.13
   Java User component 1.7
   HLOS component 0.29
   C2S Component 1.5
   Accum component 0.7
   MainCore 0.3

a. Use UNIX domain sockets in mainline builds
b. Validate HLOS monitor file paths for security measures
c. Fix crash due to no valid mStorage path
d. Always show "% Used" and available, reguardless of space
e. Add support for flushing logs
f. Exclude the APK from recent applications to fix Logkit II being restarted
   if selected by the user after 0*7435
g. Move to /data/misc/SelfHost/
h. Move mkdir out of main.cpp
i. Minimize use of system() calls
j. Update new Logkit II rules for 4.5
k. Move to /system/vendor/etc/RIDL to support eCRM HY11 builds
l. Change kmsg to dmsg (required root), add new UI button for gathering dmsg

Prior Release (RIDL Kit 4.4.46 ENG Build) 05/18/2015

Component versions for this release:
   Diag Component 0.13
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.3
   Accum component 0.7
   MainCore 0.3

a. Remove "QC logging" from apps list, launch with "3*RIDL"
   dial sequence instead
b. Allow pull of zip files by everyone
c. Fix RIDLClient.exe crash when processing sHLOSCondition_PatternMatched
d. Fix bug with deleting SQL version of rule on rule fire
e. Fix deadlock in Diag due to both connect and disconnect within same
   method
f. Don't repeatedly call Unregister logic during HLOS destructor
   if already deregistered
g. Re-launch UI if it is supposed to be connected and is not
h. Process all rule elements, even if missing .dmc file
i. Fix 'Initializing core' deadlock due to TRANS to IDLE mode with null
   user component
j. Additional support for Factory Reset; Store/restore all /data files
k. If client is in OFF state when UI is launched, update to correct state
   rather than waiting at 'Initializing core' screen
l. Fix new issue stuck initializing core from idle to 3*7435
m. Do not show the QC logging app for opening pictures
n. Fix Java exception when handling dial sequence with DB bad permissions
o. Remove process while in idle state
p. Change UI text to prevent confusion

Prior Release (RIDL Kit 4.4.40 Build) 04/20/2015

Component versions for this release:
   Diag Component 0.13
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.3
   Accum component 0.7
   MainCore 0.3

a. Record LogKit II logging session duration in SQL db rather
   than starttime (so it doesn't continue during reboot)
b. Add Diag health commands to LogKit II rules
c. Verify temp directory exists before creating files
d. Additional selinux fix for some mainline builds
e. Add eDiagCondition_TimeWindow rule element
f. Fix HLOS circular buffering chunk limit

Prior Release (RIDL Kit 4.4.30 Build) 04/06/2015

Component versions for this release:
   Diag Component 0.12
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.2
   Accum component 0.7
   MainCore 0.3

a. Update user guide to match new style guidelines

Prior Release (RIDL Kit 4.4.10 Build) 04/01/2015

Component versions for this release:
   Diag Component 0.12
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.2
   Accum component 0.7
   MainCore 0.3

a. Update selinux permissions
b. Copy DMC rather than linking due to selinux permissions

Prior Release (RIDL Kit 4.4.00 Build) 03/31/2015

Component versions for this release:
   Diag Component 0.12
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.2
   Accum component 0.7
   MainCore 0.3

a. Fix "initializing core" deadlock in LogKit II
b. Update dial sequences
c. Correctly kill UI process on 'off' button
d. mkdir before creating diag log
e. Fix rare crash when DB updates fail
f. Update QMI for 8909 build
g. Updated GoldenLogmask.dmc

Prior Release (RIDL Kit 4.3.20 Build) 03/20/2015

Component versions for this release:
   Diag Component 0.12
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.2
   Accum component 0.7
   MainCore 0.3

a. Fix memory leaks when diag log masks where large
b. Fix RIDLClient.exe not launching
c. Fix "Re-initializing core" crash
d. Background start script to unblock test teams
e. Fix bug limiting number of files configured for circular
   buffers
f. Remain in submenus when switch, button, or dropdown is
   selected
g. Add Metabuild/RIDL version to logs DB, even if gathered
   in LogKit II mode
h. Fix HLOS crash rules to be gathered in seperate event IDs

Prior Release (RIDL Kit 4.3.10 Build) 03/02/2015

Component versions for this release:
   Diag Component 0.12
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.1
   Accum component 0.7
   MainCore 0.3

a. Transparent RIDL
b. Fix race condition that could block state transition to Logkit II
c. Include Logkit II Startup rule
d. Update User Guide to current release

Prior Release (RIDL Kit 4.2.10 Build) 02/20/2015

Component versions for this release:
   Diag Component 0.12
   QMI component 0.13
   Java User component 1.6
   HLOS component 0.29
   C2S Component 1.1
   Accum component 0.7
   MainCore 0.3

a. Restore rule ID 4 (stop button behavior) which was incorrectly removed
b. Prevent multiple instances of RIDLClient.exe due to framework reboot
c. Warn user if RIDL DB is corrupt
d. Add eUserConfig_ShowDMCPicker and eUserCondition_OnDMCSelected
   to allow selection of DMC files
e. Fix bug which caused UI to show "Reconnecting to core" even if
   core had never connected
f. Fix inconsistent numbering of chunks
g. Add persistence
h. Add LogKit II 'Power Up' rule

Prior Release (RIDL Kit 4.1.20 Build) 01/07/2015

Component versions for this release:
   Diag Component 0.11
   QMI component 0.12
   Java User component 1.5
   HLOS component 0.28
   C2S Component 1.1
   Accum component 0.7
   MainCore 0.2

a. Ensure that networkType is never inserted into the client DB with
   networkType as an empty string

Prior Release (RIDL Kit 4.1.10 Build) 12/26/2014 (FAILED)

Component versions for this release:
   Diag Component 0.11
   QMI component 0.12
   Java User component 1.5
   HLOS component 0.28
   C2S Component 1.1
   Accum component 0.7
   MainCore 0.2

a. Set empty RIDL.db TEXT fields to NULL for correct GetZippedFile() behavior

Prior Release (RIDL Kit 4.1.00 Build) 12/19/2014 (FAILED)

Component versions for this release:
   Diag Component 0.11
   QMI component 0.12
   Java User component 1.5
   HLOS component 0.28
   C2S Component 1.1
   Accum component 0.7
   MainCore 0.2

a. Refresh default Rules database
b. Add 10 minute logcat logging on startup to mandatory rules
c. Fix linking issue on some devices
d. Add new HLOS command eHLOSCommand_Iwconfig
e. Remove invalid rule error check for Add switch element
f. Add feature support for uptime run command
g. Add missing user condition Switch Enabled to fix RPM rule
h. Fix Monitor ANR rule
i. New low memory C2S component
j. Fix bug in rule processing
k. Reduce memory usage by not spawning threads for each diag message
l. Perform rename for MonitorFile/Dir rather than copy
m. Fix possible deadlock in Diag component
n. Update log status (zipped, aged) on a per-log basis
o. Correctly increment chunk indexes
p. Fix issue with overwritten logs when used from same rule
q. Include version info in UI
r. Add more description to filenames
s. Fix incorrect chunk index in filenames resulting in potential lost logs
t. Add human readable timestamps to all log filenames
u. Fix stuck at "Registering rules" when hit pause
v. Fix memory leak in Diag
w. Ignore MCC/MNC change notifications
x. Add OTA log configuration
t. Ignore EINTR signal (tar extraction failure)
u. Add QDSS support for 8909
v. Fix excessive 'write timeout' TRACE messages
w. Fix bug in SSR collection which prevented more than one crash from
   being collected
x. Correcly increment Diag chunk size

Prior Release (RIDL Kit 4.0.00 Build) 10/03/2014

Component versions for this release:
   Diag Component 0.11
   QMI component 0.12
   Java User component 1.5
   HLOS component 0.26
   C2S component 2.9
   Accum component 0.7
   MainCore 0.2

a. Fix bug in recursive MonitorDirectory
b. Add eHLOSConfig_WatchVoltage
c. Fix to location change events to not count first event
   (starting point) as movement
d. Change executables to be position independent (PIE)
e. Fix bug which caused '.data' to be appended to some files
f. Extend MonitorDirectory to support more files
g. Allow location change events from multiple rules
h. Remove prompt for Metabuild
i. Improve client-side rule validation
j. Support user-friendlier filenames

Prior Release (RIDL Kit 3.0.90 Build) 08/30/2014

Component versions for this release:
   Diag Component 0.11
   QMI component 0.12
   Java User component 1.5
   HLOS component 0.25
   C2S component 2.9
   Accum component 0.7
   MainCore 0.2

a. Allow empty masks for pattern match
b. Fix bug in eUserAction_SendFormField where text was extending into
   dropdown menu area
c. Fix bug where UI rules with multiple actions where not firing all of them
d. Fix bug in tar extraction to directories on read only mount paths
e. Lower QMI wait time to 10s
f. Fix crash in libtar
g. Prevent orphan files in rare cases
h. Add eHLOSCondition_OnImageSuccess
i. Merge C2S into RIDL Client app
j. Remove C2S notifications
k. Add eHLOSConfig_WatchLocation, eHLOSCondtion_LocationChanged
l. Don't create ISF until UI is initialized
m. Fix rare crash when regex matches multiple times

Prior Release (RIDL Kit 3.0.80 Build) 08/06/2014

Component versions for this release:
   Diag Component 0.10
   QMI component 0.11
   Java User component 1.5
   HLOS component 0.22
   C2S component 2.9
   Accum component 0.7
   MainCore 0.2

a. Fix bug in client recovery on app kill
b. Add eUserAction_SendFormField
c. Support for xxhdpi screens
d. Add Hexa-decimal string support for SendRequest and PatternMatch
e. Send task to background right away
f. Add tar support for files > 2gb
g. Fix permission on /data/SelfHost so C2S (and others) can write
h. Add eAccumConfig_Repeat, eDiag/HLOS/QMIAction_AverageAccumData,
   and eAccumAction_UploadCSV_V07
i. Add eHLOSAction_SendBufferReport_v021

Prior Release (RIDL Kit 3.0.70 Build) 07/21/2014

Component versions for this release:
   Diag Component 0.9
   QMI component 0.10
   Java User component 1.4
   HLOS component 0.20
   C2S component 2.8
   Accum component 0.6
   MainCore 0.2

a. Add rule to disable Diag on boot
b. Add work-around for Diag subsystem hang
c. Fix Sub-system and System Reset rules to monitor unless
   "Pause and hold" stopped
d. Add support for additional Android Run commands
e. Only generate a report if monitoring for a BT log
f. Update rules 4 & 5 to cancel RPM and logcat logging
g. Add TCP dump rule 17 back to LogKit II Device Group
h. Add support to enable or diablle uploads
i. On press-hold the blue icon near category name, disable all switches
   under it

Prior Release (RIDL Kit 3.0.60 Build) 06/27/2014

Component versions for this release:
   Diag Component 0.9
   QMI component 0.10
   Java User component 1.4
   HLOS component 0.20
   C2S component 2.8
   Accum component 0.6
   MainCore 0.2

a. Add eC2SConfig_SetStoragePaths
b. Fix deadlock in ANR detection logic
c. Add QDSS support for QRD 8916 compatibility
d. Remove "Use Internal Storage" rule
e. Add rule to enable QXDM logging on stopping LogKit II
f. Update documentation to reflect default storage behavior
g. Fix bug in symbolic link creation and deletion
h. Fix bug in determining storage path by C2S component
i. Add eHLOSConfig_WatchBluetooth and eHLOSAction_UploadBluetooth
j. Don't create empty (tracking) files
k. Don't show C2S status in client UI
l. Don't show timestamps in filenames
m. Fix bug where UI which showed incorrect log paths
n. Fix bug where components chose incorrect paths on initialization
o. Correct timestamps in GPS logs (UTC)

Prior Release (RIDL Kit 3.0.50 Build) 05/30/2014

Component versions for this release:
   Diag Component 0.9
   QMI component 0.10
   Java User component 1.4
   HLOS component 0.19
   C2S component 2.8
   Accum component 0.6
   MainCore 0.2

a. Don't connect to diag port until first rule is loaded
b. Change app name to "QC Logging"
c. Listen for system ANR with eHLOSAppCrash_ANR instead of just user apps
d. Fix bug in Log aging which caused source file to be deleted on error
e. UI re-design.  Added eUserConfig_AddSwitch, eUserConfig_AddButton_v1.3,
   eUserCondition_SwitchEnabled, eUserCondition_SwitchDisabled,
   eUserCondition_OnStartButton, eUserCondition_OnStopButton,
   eUserCondition_OnPauseButton.
f. Add eDiagConfig_Flush, eHLOSConfig_Flush, eQMIConfig_Flush
g. Alphabatize buttons/switches/menus
h. Correct word wrap on switches to not get cut off
i. Find and use largest free storage, add new e*Config_SetStoragePaths
   to override automatic detection.
j. Add support for QDSS using rule elements eHLOSConfig_SetQDSSConfig,
   eHLOSCommand_CatQDSTmcEtr, and eHLOSCondition_QDSSFailure
k. Update default rule set and user guide

Prior Release (RIDL Kit 3.0.40 Build) 04/28/2014

Component versions for this release:
   Diag Component 0.7
   QMI component 0.8
   Java User component 1.2
   HLOS component 0.17
   C2S component 2.8
   Accum component 0.4
   MainCore 0.1

a. Fix bug in eDiagConfig_MaxChunkSize which caused it to be ignored unless
   logging was stopped and restarted.
b. Add more verbose messages during client resets
c. Add QmuxD support on select newer targets
f. Add support for RPM logs with eHLOSCommand_CatRPMStats and
   eHLOSCommand_CatRPMMaster
g. Fix bug which could result in serial number being un-readable

Prior Release (RIDL Kit 3.0.30 Build) 03/24/2014

Component versions for this release:
   Diag Component 0.7
   QMI component 0.8
   Java User component 1.2
   HLOS component 0.16
   C2S component 2.8
   Accum component 0.4
   MainCore 0.1

a. Reset "Hide logs" button on notification click
b. Enable back button (cancel text entry, hide logs, minimize app)
c. On Autologin, minimize app
d. Replace IMEI with device ID in KPI metrics
e. Fix issue in C2S which caused it to not attempt to reconnect after
   socket loss
f. Convert event ID to little endian, so it more clearly reflects timestamp
g. Remove merge rule functionality, multiple instances of each rule may exist
h. Enable NonRealtime Diag by default, new rule element
   eDiagConfig_SetNRTMode to change it at runtime
i. Fix a bug which caused pattern match to continually fire in some cases
j. Prevent rules from firing more than once every 30s and add
   new rule elements eMainCoreConfig_TimeBetweenFires and
   eMainCoreConfig_TimeBetweenReports to change it at runtime
k. Fix a bug which caused files to not be deleted on client restart
l. Add support for regular expression pattern matching

Prior Release (RIDL Kit 3.0.20 Build) 01/29/2014

Component versions for this release:
   Diag Component 0.6
   QMI component 0.7
   Java User component 1.1
   HLOS component 0.15
   C2S component 2.7
   Accum component 0.2

a. Add KPI framework support and limited set of Diag APIs
b. Change registration for device ID to IMEI-SerialNumber.  Serial number
   is now required
c. Fix possible exception when connection state change event occurs
d. Fix possible deadlock at "loading components" if rule DB was flushed
e. Correctly handle eventID if sent from main RIDL client to Java UI
f. Revert C2S behavior back to startForeground to prevent process from being
   killed by the Android activity manager due to inactivity
g. Fix possible crash in QMI component when ProcessMessage depends on a
   QMI response
h. Fix C2S stuck in offline mode issue after rule flush
i. Fix possible segmentation fault when handling multiple requests
j. Add partial support for RIDL OTA image update
k. Fix bug in which 4 character filename (IE: sudo) were being incorrectly
   tagged for zip and upload

Prior Release (RIDL Kit 3.0.10 Build) 12/11/2013

Component versions for this release:
   Diag Component 0.5
   QMI component 0.6
   Java User component 1.1
   HLOS component 0.13
   C2S component 2.6
   Accum component 0.2

Features
a. New Accumulator component
b. New elements eDiagAction_UpdateAccumData, eQMIAction_UpdateAccumData, and
   eHLOSAction_UpdateAccumData
c. Fix bug in HLOS/QMI processing of eCondition_Duration which could result in
   a deadlock in rare cases
d. Fix a bug where MCFG HW version was being provided for MCFG SW version too.
e. Fix a bug in JSON parsing of eHLOSConfig_HandleImage
f. Cleanup correctly if app is killed during login screen
g. Don't upload empty files
h. Fix a bug in Java UI which caused SendMessage and SendRule to use different
   event IDs
i. Show which files are finished uploading in the UI
j. Add RF config to RegisterDevice()
k. Fix bug where event ID could be typecast to signed long, resulting in a
   LONG_MAX.
l. New element eC2SConfig_RetryFailedUploads
m. Correctly process GOTA firmware upgrades from SD card
n. Refactor initialization process to correctly handle failures
o. Modify platform permission to /storage/sdcard1/ to WRITE_EXTERNAL_STORAGE
p. New element eAccumAction_UploadJSON
q. By default, retry failed uploads every 20 min

Prior Release (RIDL Kit 3.0.00 Build) 11/12/2013

Component versions for this release:
   Diag Component 0.4
   QMI component 0.5
   Java User component 1.1
   HLOS component 0.12
   C2S component 2.5

Features
a. Add MCFG versions to RegisterDevice()
b. Add screenshot support eUserCondition_OnScreenshot and
   eUserAction_UploadScreenshot
c. Fix bug in eHLOSConfig_MonitorDevFiles (introduced in 2.0.30)
d. Fix memory error in HLOS Pattern match
e. Add mandatory heartbeat rule
f. Add eC2SConfig_QueryImage, eC2SConfig_DownloadImage,
   eC2SCondition_ImageAvailable, and eC2SAction_SendRule
g. Retry failed logs on connectivity changed event

Prior Release (RIDL Kit 2.0.40 Build) 10/24/2013

Component versions for this release:
   Diag Component 0.2
   QMI component 0.4
   Java User component 1.0
   HLOS component 0.12
   C2S component 2.4

Features
a. New Java user component, removal of QT and busybox
b. New elements eUserConfig_AddNotify and eUserConfig_OnNotifyPressed
c. Don't overwrite HLOS boot scripts during install
d. Get metabuild from firmware (if available) rather than prompting user
e. Require IMEI be present and valid, no longer support GEN-* workaround
   for non-provisioned devices
f. Add Metabuild and Platform ID to RegisterDevice()

Prior Release (RIDL Kit 2.0.30 Build) 09/26/2013

Component versions for this release:
   Diag Component 0.2
   QMI component 0.4
   User component 0.2
   HLOS component 0.12
   C2S component 2.4

Features
a. Fix an issue in C2S which caused logs triggered while offline to not
   be uploaded when the device returned to the connected state.
b. Replay Diag health command responses at the end of every Diag log ISF file
c. Fix rare internal segmentation fault
d. Enhance UI - Highlight selected fields, add cancel button, android
   notification on log upload, status window hold for 5 seconds, etc
e. Avoid dropped Diag logs in certain scenarios
f. Add 8x26 SSR support
g. Add Android App crash support (ANR/Seg fault/exception)
h. Add HLOS support for detecting display underrun (debugfs)
i. Add QMI information messages to DIAG log for each REQ/RESP/IND received
   by the QMI component
j. Add mandatory rule for Meta build ID upload
k. Additional UI enhancements - Blue background, white text, status icon
   Qualcomm font, reverse title and icon.
l. Extract Diag bundled event reports before doing pattern matches
m. If IMEI is "0", generate random device ID instead
n. Allow storage locations to be modified at runtime
o. Add Diag config file parsing from DMC file on device
p. Support C2S rules in JSON
q. Fix a bug in eDiagConfig_Duration which calculated the start time wrong
r. Run persisted rules in offline mode until user logs in
s. Fix issue which prevented reconnecting if the RIDL process was killed
t. Show list of logs uploaded and timestamps
u. Attempt to delete circular buffer files which might be leftover on reboot
v. Fix bad pointer reference in Diag component destructor
w. Add eHLOSCommand_DetectUnderrun
x. Add seconds field to diag log filename
y. Update icon
z. Remove busybox

Prior Release (RIDL Kit 2.0.20) 06/28/2013

a. Hardcode "Mandatory Rules" 10052-10055 for device info
b. Allow dynamic Diag log size adjustment (instead of waiting
   for next UploadReport)
c. Fix bug in processing of JSON Diag/QMI Pattern match "mValues"
   array which would have caused false positive matches
d. Add initial support for HLOS inverted pattern match (Work in progress)
e. Add support for QMI on 8x26
f. Fix processing of more than 1000 items (rules or logs)
g. Add 'sudo' command to allow access to /proc/kmsg
h. Allow multiple eHLOSAction_SendBufferReport within a single rule to
   support multiple uploads within a single HLOS rule
i. Fix a bug which prevented gathering both full and compact HLOS crashes
j. Remove trace statement that can cause recursion in logcat pattern match
k. Send Diag Event report control request during startup
l. Fix a crash while processing incorrectly formatted JSON rules
m. If QMI fails to get IMEI, try with Diag.  If both fail, generate a random
   device ID for use until next installation.
n. Wait for user to press "Submit" before executing other rules.
o. During install, prompt user for meta build ID
p. Remove debug print statement causing HLOS logcat pattern match when mPattern
   string was output
q. Add support for HLOS system crash on 8x26
r. Prevent eQMIAction_SendRule from sending the same rule IDs multiple
   times

Prior Release (RIDL Kit 2.0.10) 05/30/2013

a. Support LogKit logging using circular buffers and pattern
   matching
b. Support "on-boot" rules for phone and firmware identification
   upon web server registration
c. Fix "blackscreen" issue where after being minimized, the client UI would
   sometimes not be visible
d. Include locally built Qt library to fix Qt (3rd-party) crash issue

   Follow the instructions provided in "RIDL Phone Setup"
   on the Qualcomm SharePoint RIDL web site (go/RIDL):
   (http://projects/sites/RIDL/SitePages/Home.aspx)

Prior Release (RIDL Kit 2.0.01 ENG BUILD) 05/28/2013

a. Fix memory allignment issue which caused crashes on certain diag packets
b. Fix rare C2S exception in createTemporaryZipFile() which resulted in
   C2S crash
c. Fix build corruption issue with the C2S APK (missing .jar) which resulted
   in failure to upload logs

Prior Release (FAILED - RIDL Kit 2.0.00 - FAILED) 05/23/2013

a. Complete LogKit rules planned for v2.0.00
b. Support spaces in password credentials

   Follow the instructions provided in "RIDL Phone Setup"
   on the Qualcomm SharePoint RIDL web site (go/RIDL):
   (http://projects/sites/RIDL/SitePages/Home.aspx)

Prior Release (RIDL Kit 1.9.99) 05/21/2013 PRE-RELEASE 2.0.00

a. Add initial MST early adoption set of Rules (8)
b. Add LogKit Rules

   Follow the instructions provided in "RIDL Phone Setup"
   on the Qualcomm SharePoint RIDL web site (go/RIDL):
   (http://projects/sites/RIDL/SitePages/Home.aspx)

Prior Release (RIDL Kit 1.0.00) 03/29/2013

a. Add C2S rules and reports persistence (FC)
b. Add HLOS Android API Rule
c. Add QMI Voice Rule

   Follow the instructions provided in "RIDL Phone Setup"
   on the Qualcomm SharePoint RIDL web site (go/RIDL):
   (http://projects/sites/RIDDL/SitePages/Home.aspx)

Prior Release (RIDL Kit 0.0.10) 03/04/2013

a. Initial release:
   RIDL Kit initial deployment (end-to-end Android 8960 phone to
   DMZ RIDL web server

   Follow the instructions provided in "RIDL v.10 Phone Setup"
   on the Qualcomm SharePoint RIDL web site (go/RIDL):
   (http://projects/sites/RIDDL/SitePages/Home.aspx)

3. KNOWN ISSUES

None

-------------------------------------------------------------------------------

Copyright (c) 2015, QUALCOMM Technologies Incorporated
All rights reserved.
