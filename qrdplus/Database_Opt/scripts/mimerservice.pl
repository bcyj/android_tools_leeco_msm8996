#!/usr/bin/perl
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
use warnings;
use strict;

my $usage_string = "   usage: mimerservice.pl start|stop|kill|start_monitor|stop_monitor\n";

unless ( @ARGV ){
    print "No command given.\n";
    print $usage_string;
    exit(1);
}

#The flag 32 means Intent.FLAG_INCLUDE_STOPPED_PACKAGES that is needed
#to broad cast to stopped services in Android 3.1 and later.

if ( $ARGV[0] eq "start" ) {
    system("adb shell am broadcast -a com.mimer.server.CallStart -f 32");
} elsif( $ARGV[0] eq "stop" ) {
    system("adb shell am broadcast -a com.mimer.server.CallStop -f 32");
} elsif( $ARGV[0] eq "kill" ) {
    system("adb shell am broadcast -a com.mimer.server.CallKill -f 32");
} elsif( $ARGV[0] eq "start_monitor" ) {
    system("adb shell am broadcast -a com.mimer.server.CallStartMonitor -f 32");
} elsif( $ARGV[0] eq "stop_monitor" ) {
    system("adb shell am broadcast -a com.mimer.server.CallStopMonitor -f 32");
} elsif( $ARGV[0] eq "verify_packages" ) {
    system("adb shell am broadcast -a com.mimer.server.CallVerifyPackages -f 32");
} else {
    print "Unkown command: $ARGV[0].";
    print $usage_string;
    exit(1);
}
