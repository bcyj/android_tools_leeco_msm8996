#!/usr/bin/perl

# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
use warnings;
use strict;
use File::Basename;
use Config;

my ($file, $dir) = fileparse($0);
my $reldir = "$dir..";
my $mimer_service = "com.mimer.server";
my $mimer_service_root = "/data/data/$mimer_service";
my $mimer_root = "$mimer_service_root/files";
my $external_db_path = "/system/etc/mimerdbfiles";
my $ANDROID_BUILD_TOP = $ENV{"ANDROID_BUILD_TOP"};
my $ANDROID_MAJOR_VERSION;

my $debug = 0;
my @output;

if( $ANDROID_BUILD_TOP ){
    open(VERSION, "$ANDROID_BUILD_TOP/build/core/version_defaults.mk");
    while( <VERSION> ){
	if( /PLATFORM_VERSION := (\d\.\d)\.\d/ ){
	    $ANDROID_MAJOR_VERSION = $1;
	    last;
	}
    }
}

my $email_app="Email";
my $email_app_path="/system/app";
my $contacts_provider="ContactsProvider";
my $contacts_provider_path="/system/priv-app";
my $calendar_provider="CalendarProvider";
my $calendar_provider_path="/system/priv-app";
my $telephony_provider="TelephonyProvider";
my $telephony_provider_path="/system/app";
my $media_provider="MediaProvider";
my $media_provider_path="/system/priv-app";

my $deploy_hard = 0;
my $wipe_data = 0;
my $use_root = 0;
foreach my $param (@ARGV) {
    if ($param =~ /^--?hard/) {
	$deploy_hard = 1;
        $use_root = 1;
    } elsif ($param =~ /^--?wipe_data/) {
        $wipe_data = 1;
        $deploy_hard = 1;
        $use_root = 1;
    } elsif ($param =~ /^--?root/) {
	$use_root = 1;
    } elsif ($param =~ /^-/) {
	if ($param =~ /^--?help/) {
		print "Perl script used to deploy Mimer SQL for Android"
	} else {
		print "Unknown option: $param\n";
	}
        print "Valid options are: \n";
        print "--root Use sudo on Linux\n";
	print "--hard Reboot the device and reset the cache\n";
        print "--wipe_data Same as --hard, but also wipe user data\n";
	exit();
    }
}

system_die_on_failure("adb wait-for-device");

print("=== Remounting /system and /data writable\n");
if( $use_root && $Config{'osname'} eq 'linux' ){
    system_die_on_failure("sudo `which adb` root");
    system_die_on_failure("sudo adb wait-for-device");
    system_die_on_failure("sudo `which adb` remount /system");
    system_die_on_failure("sudo adb wait-for-device");
    system_die_on_failure("sudo `which adb` remount /data");
    system_die_on_failure("sudo adb wait-for-device");
} else {
    system_die_on_failure("adb root");
    system_die_on_failure("adb wait-for-device");
    system_die_on_failure("adb remount /system");
    system_die_on_failure("adb wait-for-device");
    system_die_on_failure("adb remount /data");
    system_die_on_failure("adb wait-for-device");
}

if ( -d "$reldir/org" ) {
	print("===Backup directory found, skipping backup of original providers\n");
} else {
	print("=== Creating backup directory\n");
	mkdir("$reldir/org");
	system("adb pull $contacts_provider_path/$contacts_provider.apk $reldir/org");
	system("adb pull $contacts_provider_path/$contacts_provider.odex $reldir/org");
	
	
	
	system("adb pull $media_provider_path/$media_provider.apk $reldir/org");
	system("adb pull $media_provider_path/$media_provider.odex $reldir/org");
	
	system("adb pull $email_app_path/$email_app.apk $reldir/org");
	system("adb pull $email_app_path/$email_app.odex $reldir/org");
	
	system("adb pull $telephony_provider_path/$telephony_provider.apk $reldir/org");
	system("adb pull $telephony_provider_path/$telephony_provider.odex $reldir/org");

	system("adb pull $calendar_provider_path/$calendar_provider.apk $reldir/org");
	system("adb pull $calendar_provider_path/$calendar_provider.odex $reldir/org");
}

print("=== Removing $contacts_provider.apk\n");

@output = `adb shell rm $contacts_provider_path/$contacts_provider.*`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.providers.contacts`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.mimer.providers.contacts`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.android.providers.contacts`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.android.providers.contacts`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /cache/dalvik-cache/*$contacts_provider.apk* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.android.mimbench`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");


print("=== Removing $media_provider.apk\n");
@output = `adb shell rm $media_provider_path/$media_provider.*`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.providers.media`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.mimer.providers.media`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.android.providers.media`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.android.providers.media`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /cache/dalvik-cache/*$media_provider.apk* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.android.mediascanner`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

print("=== Removing $email_app.apk\n");
@output = `adb shell rm $email_app_path/$email_app.*`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.email`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.mimer.email`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.android.email`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.android.email`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /cache/dalvik-cache/*$email_app.apk* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

print("=== Removing $telephony_provider.apk\n");
@output = `adb shell rm $telephony_provider_path/$telephony_provider.*`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.providers.telephony`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.mimer.providers.telephony`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.android.providers.telephony`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.android.providers.telephony`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /cache/dalvik-cache/*$telephony_provider.apk* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

print("=== Removing $calendar_provider.apk\n");
@output = `adb shell rm $calendar_provider_path/$calendar_provider.*`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.providers.calendar`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.mimer.providers.calendar`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.android.providers.calendar`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.android.providers.calendar`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /cache/dalvik-cache/*$calendar_provider.apk* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");


print("=== Stopping server\n");
@output = `adb  shell $mimer_root/mimexper -k mimerserver`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

print("=== Removing MimerService.apk\n");
@output = `adb shell rm /system/app/MimerService.*`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

@output = `adb uninstall com.mimer.server`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/data/com.mimer.server`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell killall mimer-adm`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /cache/dalvik-cache/*MimerService.apk* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/dalvik-cache/*MimerService.apk* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

system("adb wait-for-device");
@output = `adb shell rm $external_db_path/db.zip`;
if($debug || $?) { print (@output); }

print("=== Removing Mimer shared libraries\n");
@output = `adb shell rm /system/lib/libmimer.so`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm /system/lib/libmimcl.so`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm /system/lib/libmimcomm.so`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm /system/lib/libmimmicroapi.so`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");

print("=== Removing Mimer Framework\n");
system("adb wait-for-device");
@output = `adb shell rm /system/framework/MimerFramework.jar`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm /system/etc/permissions/MimerFramework.xml`;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /cache/dalvik-cache/*MimerFramework.jar* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");
@output = `adb shell rm -r /data/dalvik-cache/*MimerFramework.jar* `;
if($debug || $?) { print (@output); }
system("adb wait-for-device");


print("=== Pushing Mimer SQL shared libraries\n");
system_die_on_failure("adb wait-for-device");
system_die_on_failure("adb  push $reldir/native/SQLiteTranslator/libmimcl.so /system/lib");
system_die_on_failure("adb wait-for-device");

print("=== Pushing Mimer Framework\n");
system_die_on_failure("adb  push $reldir/framework/MimerFramework.jar /system/framework");
system_die_on_failure("adb wait-for-device");
system_die_on_failure("adb  push $reldir/framework/MimerFramework.xml /system/etc/permissions/");
system_die_on_failure("adb wait-for-device");
system_die_on_failure("adb  push $reldir/native/mimcomm/libmimcomm.so /system/lib");
system_die_on_failure("adb wait-for-device");
system_die_on_failure("adb  push $reldir/native/mimmicroapi/libmimmicroapi.so /system/lib");
system_die_on_failure("adb wait-for-device");


if( $deploy_hard ) {
    print("=== Reboot and cleanup\n");
    if($Config{'osname'} eq 'linux' ){
        system_die_on_failure("sudo `which adb` reboot bootloader");
        if( $wipe_data ) {
            system_die_on_failure("sudo `which fastboot` erase cache erase userdata");
        } else {
            system_die_on_failure("sudo `which fastboot` erase cache");
        }
        system_die_on_failure("sudo `which fastboot` reboot");
    } else {
        system_die_on_failure("adb reboot bootloader");
        if( $wipe_data ) {
            system_die_on_failure("fastboot erase cache erase userdata");
        } else {
            system_die_on_failure("fastboot erase cache");
        }
        system_die_on_failure("fastboot reboot");
    }
} else {
    print("=== Reboot\n");
    system_die_on_failure("adb reboot");
}
## Wait a few seconds after reboot or else there is a chance that the following wait-for-device will happen before the device goes down for reboot
sleep(5);  

system_die_on_failure("adb wait-for-device");
if ( $wipe_data ){
    print "Wait for the device to be initialized and root access is granted\n";
    print "Ignore any errors on the device!\n";
    print "Press <ENTER> to coninue\n";
    my $dummy = <STDIN>;
} elsif( $deploy_hard ) {
    print("=== Reboot done but wait 60 more seconds for the device to initialize\n");
    sleep(60);
    system_die_on_failure("adb wait-for-device");
}

print("=== Remounting /system and /data writable\n");
if($use_root && $Config{'osname'} eq 'linux' ){
    system_die_on_failure("sudo `which adb` root");
    system_die_on_failure("sudo adb wait-for-device");
    system_die_on_failure("sudo `which adb` remount /system");
    system_die_on_failure("sudo adb wait-for-device");
    system_die_on_failure("sudo `which adb` remount /data");
    system_die_on_failure("sudo adb wait-for-device");
} else {
    system_die_on_failure("adb root");
    system_die_on_failure("adb wait-for-device");
    system_die_on_failure("adb remount /system");
    system_die_on_failure("adb wait-for-device");
    system_die_on_failure("adb remount /data");
    system_die_on_failure("adb wait-for-device");
}


if ( -e "$reldir/bin/busybox" ) {
    print("=== Installing Busybox\n");
    system_die_on_failure("adb  push $reldir/bin/busybox /system/bin/busybox");
    system_die_on_failure("adb wait-for-device");
    system_die_on_failure("adb  shell chmod 777 /system/bin/busybox");
    system_die_on_failure("adb wait-for-device");
}




@output = `adb  uninstall $mimer_service`;
if($debug || $?) { print (@output); }
system_die_on_failure("adb wait-for-device");

print("=== Pushing Mimer database files\n");
@output = `adb shell mkdir $external_db_path`;
if($debug || $?) { print (@output); }
system_die_on_failure("adb wait-for-device");
system_die_on_failure("adb  push $reldir/db/db.zip $external_db_path");
system_die_on_failure("adb wait-for-device");

print("=== Pushing MimerService.apk\n");
system_die_on_failure("adb  push $reldir/mimerservice/MimerService.apk /system/app");
system_die_on_failure("adb wait-for-device");

print("=== Reboot\n");
system("adb reboot");
system("adb wait-for-device");
if($use_root && $Config{'osname'} eq 'linux' ){
    system_die_on_failure("sudo `which adb` root");
    system_die_on_failure("sudo adb wait-for-device");
    system_die_on_failure("sudo `which adb` remount /system");
    system_die_on_failure("sudo adb wait-for-device");
} else {
    system_die_on_failure("adb root");
    system_die_on_failure("adb wait-for-device");
    system_die_on_failure("adb remount /system");
    system_die_on_failure("adb wait-for-device");
}





print("=== Pushing $contacts_provider.apk\n");
system_die_on_failure("adb  push $reldir/bin/$contacts_provider.apk $contacts_provider_path");
system_die_on_failure("adb wait-for-device");


print("=== Pushing $media_provider.apk\n");
system_die_on_failure("adb  push $reldir/bin/$media_provider.apk $media_provider_path");
system_die_on_failure("adb wait-for-device");

print("=== Pushing $email_app.apk\n");
system_die_on_failure("adb  push $reldir/bin/$email_app.apk $email_app_path");
system_die_on_failure("adb wait-for-device");

print("=== Pushing $telephony_provider.apk\n");
system_die_on_failure("adb  push $reldir/bin/$telephony_provider.apk $telephony_provider_path");
system_die_on_failure("adb wait-for-device");

print("=== Pushing $calendar_provider.apk\n");
system_die_on_failure("adb  push $reldir/bin/$calendar_provider.apk $calendar_provider_path");
system_die_on_failure("adb wait-for-device");


if( -e "$reldir/../../../benchmark/MimerBench.apk" ) {
    print("=== Install MimerBench for Contacts with \"adb push <MIMER_RELEASE_DIR>/benchmarks/MimerBench.apk /system/app/\"\n");
}

if( -e "$reldir/../../../benchmark/MediaScanner.apk" ) {
    print("=== Install MediaScanner benchmark with \"adb install <MIMER_RELEASE_DIR>/benchmarks/MediaScanner.apk\"\n");
}


print("=== Reboot to finalize installation\n");
system_die_on_failure("adb reboot");
system_die_on_failure("adb wait-for-device");
exit(0);

sub system_die_on_failure {
    my ($cmd) = @_;

    my @callr = caller;
    
    print "$cmd\n" if (defined $ENV{VERBOSE});
    system($cmd)==0 or die "Command $cmd terminated with error status $? from line ".$callr[2];
}
