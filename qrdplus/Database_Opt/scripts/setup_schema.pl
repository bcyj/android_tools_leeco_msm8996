#!/usr/bin/perl
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
use warnings;
use strict;
use File::Basename;
use File::Spec;
use lib dirname(File::Spec->rel2abs( $0 ));
use mimer_common;

my $release_package=0;

my @known_schema_list = get_schemas();

my @known_vendor_list = get_vendors();

my $MIMER_DATABASE='mimerserver';
my $vendor;
$vendor=lc('QTI');

my $scriptdir = dirname(File::Spec->rel2abs( $0 ));
my $mimdir = File::Spec->catdir($scriptdir, "..");
my $base_provider_package = "com.android.providers";

my $build_top = $mimdir;
my $android_major_version = -1;
my $android_minor_version = -1;
my $android_full_version = "";

my $schema_created = 0;
my %schemas;
my $schema_cnt = 1;
my $use_sdk = 1;
my $verbose = 0;

my $demo = 0;
foreach my $arg ( @ARGV ){
    if( $arg =~ /--database=(.+)/ ){
	$MIMER_DATABASE = $1;
    } elsif( $arg =~ /--vendor=(.+)/ ){
		$vendor = lc($1);
	} elsif( $arg =~ /--provider_package=(.+)/ ){
     	$base_provider_package = $1
    } elsif( $arg =~ /--android_version=(.+)/ ){
		$android_full_version = $1;
		$android_full_version =~ /(\d\.\d)(?:\.(\d))?/;
	    $android_major_version = $1;
    	if($2) {
        	$android_minor_version = $2;
        } 
    } elsif( $arg =~ /--verbose/ ){
	$verbose = 1;
    } elsif( $arg =~ /--demo/ ){
	$demo = 1;
    } elsif( $arg =~ /--no_sdk/ ){
	$use_sdk = 0;
    } elsif( $arg =~ /--help/ || $arg =~ /-h/ ){
		print_usage();
		exit;
    } else {
	$schemas{$arg} = $schema_cnt++;
    }
}

if ($android_full_version eq "") {
	die "ANDROID_BUILD_TOP not set, please run envsetup.sh and lunch, or specify --android_version" unless $ENV{"ANDROID_BUILD_TOP"};
	$build_top = $ENV{"ANDROID_BUILD_TOP"};
	$android_major_version = get_android_major_version($android_full_version);
	$android_minor_version = get_android_minor_version($android_full_version);
	if($android_minor_version != -1) {
		$android_full_version = "$android_major_version.$android_minor_version";
	} else {
		$android_full_version = "$android_major_version";
	}
}

my @vendor_schemas = get_schemas($vendor);

if($schema_cnt == 1) {
	my $i = 1;
	foreach my $schem (@vendor_schemas ){
		$schemas{$schem} = $i++;
	}
}


my $sqldir = File::Spec->catdir($mimdir, "sql");
my $vendor_sqldir;
if($vendor ne "generic") {
	$vendor_sqldir = File::Spec->catdir($sqldir, "schemas", "vendor_adapt", "sql");
}

print "Setting up the following schemas:\n";
foreach my $s (sort { $schemas{$a} <=> $schemas{$b} } keys %schemas) {
	print "\t$s \n";
}

port_forward();

foreach my $prov (sort { $schemas{$a} <=> $schemas{$b} } keys %schemas) {

    if( $prov eq 'compatibility' ){
	$schema_created = 1;
	chdir($sqldir) or die "*** Unable to find schema directory.";
	print "=== Setting up $prov schema\n";

	kill_process("com.mimer.providers.calendar");
	kill_process("mms");
	kill_process("phone");
	kill_process("acore");
	print "=== Setting database offline\n";
	bsql_query_continue("set database offline", 'SYSADM','SYSADM');

	print "=== Setting up MIMERSERVICE ident\n";
	bsql_query_continue("drop ident MIMERSERVICE cascade;", 'SYSADM', 'SYSADM');
	my $sql=<<EOL;
	    create ident MIMERSERVICE as user using 'MIMERSERVICEPSWD'
	    grant databank to MIMERSERVICE with grant option
	    grant schema to MIMERSERVICE with grant option
	    grant backup to MIMERSERVICE
	    grant ident to MIMERSERVICE with grant option
	    grant select on system.objects to MIMERSERVICE with grant option
	    grant select on system.server_info to MIMERSERVICE with grant option
EOL

        bsql_query($sql,'SYSADM','SYSADM');

	print "=== Clearing database\n";
	$sql = <<EOL;
	    drop schema compatibility cascade
	    drop databank sqlite_comp cascade
EOL

	bsql_query_continue($sql);

	print "=== Setting max and goal size on TRANSDB and SQLDB\n";
	$sql = <<EOL;
	    alter databank sqldb set goalsize 4M
	    alter databank transdb set goalsize 4M
	    alter databank sqldb set minsize 2M
	    alter databank transdb set minsize 2M
EOL

	bsql_query($sql, 'SYSADM','SYSADM');

	bsql_query("read 'sysadm_schema.sql'", 'SYSADM','SYSADM');
	bsql_query("read 'compatibility_schema.sql'");
	bsql_query("read 'compatibility_ISO639.sql'");
	if ( $android_major_version eq "2.3" ) {
		bsql_query("read 'compatibility_schema_23.sql'");
	}

	print "=== Setting database online\n";
	bsql_query_continue('set database online preserve log', 'SYSADM','SYSADM');


} 
if( $prov eq 'contacts' ) {
	$schema_created = 1;
	chdir($vendor_sqldir) or die "*** Unable to find schema directory for vendor $vendor.";
	print "=== Setting up $prov schema.\n";
	print "=== Setting database offline\n";
	bsql_query("set database offline", 'SYSADM','SYSADM');

	print "=== Clearing database\n";
	my $sql = <<EOL;
	    drop ident contacts cascade
EOL

	bsql_query_continue($sql);

## CTSDATABASE user is temporarily created here of convenience reasons only.
## It should be removed soon.

	$sql = <<EOL;
	    create ident contacts as user using 'contactspswd2'
	    grant ident to contacts with grant option
	    grant databank to contacts with grant option
	    grant schema to contacts with grant option
	    create ident ctsdatabase as user using 'ctsdatabasepswd2'
	    grant ident to ctsdatabase with grant option
	    grant databank to ctsdatabase with grant option
	    grant schema to ctsdatabase with grant option
EOL
        bsql_query($sql);

	print "=== Setting database online\n";
	bsql_query('set database online preserve log', 'SYSADM','SYSADM');

    bsql_query('create ident mimer\$permission\$contacts_read as group', 'MIMERSERVICE', 'MIMERSERVICEPSWD');

	if( $android_major_version > "4.0") {
	    bsql_query("read 'contacts_schema_41.sql'",'contacts','contactspswd2');
	} elsif( $android_major_version eq "4.0" ) {
	    bsql_query("read 'contacts_schema_40.sql'",'contacts','contactspswd2');
	} elsif ( $android_major_version eq "2.3" ) {
	    bsql_query("read 'contacts_schema_23.sql'",'contacts','contactspswd2');
	} elsif ( $android_major_version eq "2.1" ) {
	    bsql_query("read 'contacts_schema.sql'",'contacts','contactspswd2');
	} else {
	    clean_exit("Unsupported Android version($android_major_version) for contacts schema.",1);
	}
	if($demo) {
		print "=== Using Demo mode.\n";
		bsql_query("read 'contacts_demo.sql'",'contacts','contactspswd2');
	}
	bsql_query("alter ident contacts using 'contactspswd'");

} 

if ( $prov eq 'media' ){
	$schema_created = 1;
	chdir($vendor_sqldir) or die "*** Unable to find schema directory for vendor $vendor.";
	print "=== Setting up $prov schema.\n";
	print "=== Setting database offline\n";
	bsql_query("set database offline", 'SYSADM','SYSADM');

	print "=== Clearing database\n";
	my $sql = <<EOL;
	    drop ident media cascade
EOL

        print "=== Creating idents\n";
	bsql_query_continue($sql);

	$sql = <<EOL;
	    create ident media as user using 'mediapswd2'
	    grant ident to media with grant option
	    grant databank to media with grant option
	    grant schema to media with grant option
EOL
        bsql_query($sql);

	print "=== Setting database online\n";
	bsql_query('set database online preserve log', 'SYSADM','SYSADM');

    bsql_query('create ident mimer\$permission\$media_read as group', 'MIMERSERVICE', 'MIMERSERVICEPSWD');

	if( $android_major_version > "2.3") {
	    bsql_query("read 'media_schema_40.sql'",'media','mediapswd2');
	} elsif ( $android_major_version eq "2.3" ) {
	    bsql_query("read 'media_schema_23.sql'",'media','mediapswd2');
	} elsif ( $android_major_version eq "2.1" ) {
	    bsql_query("read 'media_schema.sql'",'media','mediapswd2');
	} else {
	    clean_exit("Unsupported Android version($android_major_version) for $prov schema.",1);
	}

	bsql_query("alter ident media using 'mediapswd'");

} 

if ( $prov eq 'telephony' ) {
	$schema_created = 1;
	chdir($vendor_sqldir) or die "*** Unable to find schema directory for vendor $vendor.";
	print "=== Setting up $prov schema.\n";
	kill_process("phone");
	kill_process("mms");
	kill_process("acore");

	print "=== Setting database offline\n";
	bsql_query("set database offline", 'SYSADM','SYSADM');

	my $sql=<<EOL;
	    drop ident telephony cascade
EOL

        bsql_query_continue($sql);

	$sql=<<EOL;
	    create ident telephony as user using 'telephonypswd2'
	    grant ident to telephony with grant option
	    grant databank to telephony with grant option
	    grant schema to telephony with grant option
EOL

	bsql_query($sql);

	print "=== Setting database online\n";
	bsql_query('set database online preserve log', 'SYSADM','SYSADM');

    bsql_query('create ident mimer\$permission\$telephony_read as group', 'MIMERSERVICE', 'MIMERSERVICEPSWD');

    if ( $prov eq 'telephony' ) {
	bsql_query("read 'telephony_core.sql'", 'telephony', 'telephonypswd2');

	if( $android_major_version > "2.3") {
	    bsql_query("read 'telephony_schema_40.sql'", 'telephony', 'telephonypswd2');
	} elsif ( $android_major_version eq "2.3" ) {
	    bsql_query("read 'telephony_schema_23.sql'", 'telephony', 'telephonypswd2');
	} else {
	    clean_exit("Unsupported Android version($android_major_version) for telephony schema.",1);
	}
	bsql_query("read 'telephony_schema_triggers.sql'", 'telephony', 'telephonypswd2');
    }
	bsql_query("alter ident telephony using 'telephonypswd'");

}

if ( $prov eq 'calendar' ) {
	$schema_created = 1;
	chdir($vendor_sqldir) or die "*** Unable to find schema directory for vendor $vendor.";
	print "=== Setting up $prov schema.\n";
	kill_process("calendar");
	
	print "=== Setting database offline\n";
	bsql_query("set database offline", 'SYSADM','SYSADM');

	my $sql=<<EOL;
	    drop ident calendar cascade
EOL
        bsql_query_continue($sql);

	$sql=<<EOL;
	    create ident calendar as user using 'calendarpswd2'
	    grant ident to calendar with grant option
	    grant databank to calendar with grant option
	    grant schema to calendar with grant option
EOL

	bsql_query($sql);

	print "=== Setting database online\n";
	bsql_query('set database online preserve log', 'SYSADM','SYSADM');

	bsql_query('create ident mimer\$permission\$calendar_read as group', 'MIMERSERVICE', 'MIMERSERVICEPSWD');

	bsql_query("read 'calendar_core.sql'", 'calendar', 'calendarpswd2');

	if( $android_major_version > "4.0") {
	    bsql_query("read 'calendar_schema_40.sql'", 'calendar', 'calendarpswd2');
	    bsql_query("read 'calendar_schema_41.sql'", 'calendar', 'calendarpswd2');
	} elsif( $android_major_version eq "4.0" ) {
		bsql_query("read 'calendar_schema_40.sql'", 'calendar', 'calendarpswd2');
	 } else {
	    die "Unsupported Android version($android_major_version) for calendar schema.";
	}
	
	bsql_query("alter ident calendar using 'calendarpswd'");

}


if ( $prov eq 'email' ) {
	$schema_created = 1;
	chdir($vendor_sqldir) or die "*** Unable to find schema directory for vendor $vendor.";
	print "=== Setting up $prov schema.\n";
	kill_process("email");

	print "=== Setting database offline\n";
	bsql_query("set database offline", 'SYSADM','SYSADM');

	my $sql=<<EOL;
	    drop ident email cascade
EOL
        bsql_query_continue($sql);

	$sql=<<EOL;
	    create ident email as user using 'emailpswd2'
	    grant ident to email with grant option
	    grant databank to email with grant option
	    grant schema to email with grant option
EOL

	bsql_query($sql);

	print "=== Setting database online\n";
	bsql_query('set database online preserve log', 'SYSADM','SYSADM');

        bsql_query('create ident mimer\$permission\$email_read as group', 'MIMERSERVICE', 'MIMERSERVICEPSWD');

	bsql_query("read 'email_core.sql'", 'email', 'emailpswd2');

	if(  $android_major_version > "2.3") {
	    bsql_query("read 'email_schema.sql'", 'email', 'emailpswd2');
	} else {
	    die "Unsupported Android version($android_major_version) for calendar schema.";
	}

	bsql_query("alter ident email using 'emailpswd'");

}

 if($schema_created == 0) {
	clean_exit("*** Unknown schema $prov.\n",1);
    }

}

print "Granting Group privileges to predefined idents\n";
my @sorted_schemas = sort { $schemas{$a} <=> $schemas{$b} } keys %schemas;
foreach my $prov1 (@sorted_schemas) {
   if ( $prov1 eq 'compatibility' ) {
	next;
   }
   foreach my $prov2 (@sorted_schemas) {
	if ( $prov2 eq 'compatibility' ) {
	    next;
        }
	if( $prov2 ne  $prov1 ) {
            bsql_query('grant member on mimer\$permission\$'.$prov2.'_read to '.$prov1, 'MIMERSERVICE', 'MIMERSERVICEPSWD');
        }
   } 
}

clean_exit("*** Done setting up the schema for $vendor Android $android_major_version from $vendor_sqldir.\n",0);

sub port_forward
{
	if($use_sdk) {
		print "=== Setting up port forwarding\n";
		my_system( "adb forward tcp:1361 tcp:1360" ) and die "*** Unable to forward tcp ports with adb";
	}
}


sub bsql_query_continue
{
    my $q = shift;
    my $u = shift;
    my $p = shift;
    my $s = shift;
    bsql_query_whenever(1,$q,$u,$p,$s);
}

sub bsql_query
{
    my $q = shift;
    my $u = shift;
    my $p = shift;
    my $s = shift;
    bsql_query_whenever(0,$q,$u,$p,$s);
}

sub bsql_query_whenever
{
    my $ignore_error = shift;
    my $q = shift;
    my $u = shift;
    my $p = shift;
    my $s = shift;

    unless( defined $q ){
	clean_exit("*** Query cannot be empty.",1);
    }

    unless( defined $u ){
	$u = 'MIMERSERVICE';
    }

    unless( defined $p ){
	$p = 'MIMERSERVICEPSWD';
    }
 
    unless( defined $s ){
	$s = $MIMER_DATABASE;
    }
    
    my @queries = split("\n", $q);

    foreach my $sub_q ( @queries ){
	my $status = my_system( "bsql -u $u -p $p --query=\"" . $sub_q . "\" $s" );
	$status==0 or $ignore_error or clean_exit("*** Unable to run \"$sub_q\" with bsql.",$status>>8);
    }
}

sub kill_process 
{
	if($use_sdk) {
		my $process = shift;

		my @proc_list = `adb shell ps`;

		my @proc_line = grep( /$process/, @proc_list );
		if( @proc_line != 1 ){
		print "*** Unable to find process ID of $process with adb\n";
		return;
		}

		my @proc_line_split = split( /\s+/, $proc_line[0] );

		my_system_die("adb shell kill $proc_line_split[1]");
		my_system_die("adb shell kill -9 $proc_line_split[1]");
	}
}


sub clean_exit
{
    my $message = shift;
    my $status = shift;
    `bsql -u SYSADM -p SYSADM --query=\"set database online preserve log\" $MIMER_DATABASE`;
    print $message;
    exit(($status?$status:0));
}

sub print_usage
{
    print<<EOP;
    usage:

       setup_schema.pl [<options>] S1 S2 S3 ...
       Where S1, S2, S3 is a schema to set up, it can be one of:
EOP
print "\t", join("\n\t", @known_schema_list);
print<<EOP


       The schema will be set up in the order given.
       or
       setup_schema.pl [<options>] --vendor=<vendor>
       Where <vendor> determines what schemas to setup
	   
       Valid options are:
       --database The database to use, mimerserver if not specified.
       --vendor Used specify a predefined set of schemas. For exampe, generic will create schemas for all providers.
       --android_version Set the Android version to generate schema for. If not set, the ANDROID_BUILD_TOP must be
	                     set so that the version can be determined automatically.
       --no_sdk Don't use any "adb" commands, for example to kill providers.
       --no_make_schema Don't call make_schema.pl to pre-process the SQL files. The default behaviour is to do this.
       --provider_package The provider package name. This is used in the database mappings. For example com.android.providers
EOP
;
}

sub my_system {
    print "@_\n" if (exists $ENV{"VERBOSE"} || $verbose);
    return system(@_);
}

sub my_system_die {
    my_system(@_)==0 or die "Failed to execute \"@_\" $!";
    return 0;
}

sub my_system_exit {
    my $ex = my_system(@_);
    if ($ex!=0) {
	print STDERR "Failed to execute \"@_\" $!";
	exit($ex);
    }
    return 0;
}
