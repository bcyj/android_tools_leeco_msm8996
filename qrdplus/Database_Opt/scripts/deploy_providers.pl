#!/usr/bin/perl

# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
use warnings;
use strict;
use File::Basename;
use Config;


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


my ($file, $dir) = fileparse($0);
my $reldir = "$dir..";
my $bin_dir = "$reldir/bin";
my $mimer_service = "com.mimer.server";
my $mimer_service_root = "/data/data/$mimer_service";
my $mimer_root = "$mimer_service_root/files";
my $ANDROID_BUILD_TOP = $ENV{"ANDROID_BUILD_TOP"};
my $ANDROID_MAJOR_VERSION;

if( $ANDROID_BUILD_TOP ){
    open(VERSION, "$ANDROID_BUILD_TOP/build/core/version_defaults.mk");
    while( <VERSION> ){
	if( /PLATFORM_VERSION := (\d\.\d)\.\d/ ){
	    $ANDROID_MAJOR_VERSION = $1;
	    last;
	}
    }
}

my $deploy_hard = 0;
my $wipe_data = 0;
my $use_root = 0;
my $provider_type = "mimer";
foreach my $param (@ARGV) {
    if ($param =~ /^--?hard/) {
	$deploy_hard = 1;
        $use_root = 1;
    } elsif ($param =~ /^--?wipe_data/) {
        $wipe_data = 1;
        $deploy_hard = 1;
        $use_root = 1;
    } elsif( $param =~ /--provider_type=(.+)/ ){
		$provider_type = $1;
		if( $provider_type eq "sqlite") {
			$bin_dir = "$reldir/org";
		}
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
        print "--provider_type=sqlite/mimer\n";
	exit();
    }
}

if( $provider_type eq "sqlite") {
	print("=== Using SQLite providers\n\n");
} else {
	print("=== Using Mimer SQL providers\n\n");
}

system("adb wait-for-device");

print("=== Remounting /system and /data writable\n");
if( $use_root && $Config{'osname'} eq 'linux' ){
    system("sudo `which adb` root");
    system("sudo adb wait-for-device");
    system("sudo `which adb` remount /system");
    system("sudo adb wait-for-device");
    system("sudo `which adb` remount /data");
} else {
    system("adb root");
    system("adb wait-for-device");
    system("adb remount /system");
    system("adb wait-for-device");
    system("adb remount /data");
}

print("=== Removing $contacts_provider.apk\n");
system("adb shell rm $contacts_provider_path/$contacts_provider.* > /dev/null");


print("=== Removing /data/data/com.android.providers.contacts\n");
system("adb uninstall com.mimer.providers.contacts > /dev/null");
system("adb shell rm -r /data/data/com.mimer.providers.contacts > /dev/null");

system("adb uninstall com.android.providers.contacts > /dev/null");
system("adb shell rm -r /data/data/com.android.providers.contacts > /dev/null");
system("adb shell rm -r /cache/dalvik-cache/*$contacts_provider.apk*  > /dev/null");


print("=== Removing $media_provider.apk\n");
system("adb shell rm $media_provider_path/$media_provider.* > /dev/null");


print("=== Removing /data/data/com.android.providers.media\n");
system("adb uninstall com.mimer.providers.media > /dev/null");
system("adb shell rm -r /data/data/com.mimer.providers.media > /dev/null");

system("adb uninstall com.android.providers.media > /dev/null");
system("adb shell rm -r /data/data/com.android.providers.media > /dev/null");
system("adb shell rm -r /cache/dalvik-cache/*$media_provider.apk*  > /dev/null");


print("=== Removing $email_app.apk\n");
system("adb shell rm $email_app_path/$email_app.* > /dev/null");
system("adb shell rm $email_app_path/$email_app?.* > /dev/null");

print("=== Removing /data/data/com.android.email\n");
system("adb uninstall com.mimer.email > /dev/null");
system("adb shell rm -r /data/data/com.mimer.email > /dev/null");

system("adb uninstall com.android.email > /dev/null");
system("adb shell rm -r /data/data/com.android.email > /dev/null");
system("adb shell rm -r /cache/dalvik-cache/*$email_app.apk*  > /dev/null");

print("=== Removing $telephony_provider.apk\n");
system("adb shell rm $telephony_provider_path/$telephony_provider.* > /dev/null");

print("=== Removing /data/data/com.android.providers.telephony\n");
system("adb uninstall com.mimer.providers.telephony > /dev/null");
system("adb shell rm -r /data/data/com.mimer.providers.telephony > /dev/null");

system("adb uninstall com.android.providers.telephony > /dev/null");
system("adb shell rm -r /data/data/com.android.providers.telephony > /dev/null");
system("adb shell rm -r /cache/dalvik-cache/*$telephony_provider.apk*  > /dev/null");

print("=== Removing $calendar_provider.apk\n");
system("adb shell rm $calendar_provider_path/$calendar_provider.* > /dev/null");

print("=== Removing /data/data/com.android.providers.calendar\n");
system("adb uninstall com.mimer.providers.calendar > /dev/null");
system("adb shell rm -r /data/data/com.mimer.providers.calendar > /dev/null");

system("adb uninstall com.android.providers.calendar > /dev/null");
system("adb shell rm -r /data/data/com.android.providers.calendar > /dev/null");
system("adb shell rm -r /cache/dalvik-cache/*$calendar_provider.apk*  > /dev/null");


if( $deploy_hard ) {
    print("=== Reboot and cleanup\n");
    if($Config{'osname'} eq 'linux' ){
        system("sudo `which adb` reboot bootloader");
        if( $wipe_data ) {
            system("sudo `which fastboot` erase cache erase userdata");
        } else {
            system("sudo `which fastboot` erase cache");
        }
        system("sudo `which fastboot` reboot");
    } else {
        system("adb reboot bootloader");
        if( $wipe_data ) {
            system("fastboot erase cache erase userdata");
        } else {
            system("fastboot erase cache");
        }
        system("fastboot reboot");
    }
    
	system("adb wait-for-device");
	if ( $wipe_data ){
	    print "Wait for the device to be initialized and root access is granted\n";
	    print "Ignore any errors on the device!\n";
	    print "Press <ENTER> to coninue\n";
	    my $dummy = <STDIN>;
	} elsif( $deploy_hard ) {
	    print("=== Reboot done but wait 60 more seconds for the device to initialize\n");
	    sleep(60);
	}
	
	print("=== Remounting /system and /data writable\n");
	if($use_root && $Config{'osname'} eq 'linux' ){
	    system("sudo `which adb` root");
	    system("sudo adb wait-for-device");
	    system("sudo `which adb` remount /system");
	    system("sudo `which adb` remount /system");
	    system("sudo `which adb` remount /data");
	} else {
	    system("adb root");
	    system("adb wait-for-device");
	    system("adb remount /system");
	    system("adb wait-for-device");
	    system("adb remount /data");
	    system("adb wait-for-device");
	}
}



print("=== Pushing $contacts_provider.apk\n");
system("adb push $bin_dir/$contacts_provider.apk $contacts_provider_path");


print("=== Pushing $media_provider.apk\n");
system("adb push $bin_dir/$media_provider.apk $media_provider_path");


print("=== Pushing $email_app.apk\n");
system("adb push $bin_dir/$email_app.apk $email_app_path");


print("=== Pushing $telephony_provider.apk\n");
system("adb push $bin_dir/$telephony_provider.apk $telephony_provider_path");

print("=== Pushing $calendar_provider.apk\n");
system("adb push $bin_dir/$calendar_provider.apk $calendar_provider_path");

print("=== You should probably reboot the device\n");
