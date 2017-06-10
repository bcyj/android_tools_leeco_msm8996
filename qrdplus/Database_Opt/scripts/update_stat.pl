#!/usr/bin/perl
# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
use warnings;
use strict;
use File::Basename;
use File::Spec;
use lib dirname(File::Spec->rel2abs( $0 ));
use mimer_common;
 
my @known_schema_list = get_schemas();

my $release_package=0;
$release_package=1;

my $debug = 0;

my $scriptdir = dirname(File::Spec->rel2abs( $0 ));
my $mimdir = File::Spec->catdir($scriptdir, "..");

my $vendor;
$vendor=lc('QTI');

my $android_major_version = "";
my $all_ext_removable = 0;

my $sqldir = File::Spec->catdir($mimdir, "sql");
my $sqlstatdir = File::Spec->catdir($sqldir, "stat_data");
my $sqlstatfile = '';
my $mimerservice = File::Spec->catfile($scriptdir, "mimerservice.pl");

my $MIMER_DATABASE='mimerserver';

my @output;
my $schemas_supplied = 0;
my $global_stat = 0;

my %schemas;
my $use_sdk = 1;
my $schema_cnt = 1;
my $wipe_first = 0;
foreach my $arg ( @ARGV ){
    if( $arg =~ /--database=(.+)/ ){
		$MIMER_DATABASE = $1;
    } elsif( $arg =~ /--help/ ){
		print "Usage: \nupdate_stat.pl [--database=<database name>] [--force_global_stat] [<schema 1>, <schema 2>, ...]\n";
		exit;
    } elsif( $arg =~ /--force_global_stat/ ){
		$global_stat = 1;
    } elsif( $arg =~ /--no_sdk/ ){
		$use_sdk = 0;
    } elsif( $arg =~ /--wipe_first/ ){
		$wipe_first = 1;
    } elsif( $arg =~ /--debug/ ){
		$debug = 1;
    } elsif( $arg =~ /--vendor=(.+)/ ){
		$vendor = lc($1);
    } elsif( $arg =~ /--android_version=(.+)/ ){
		$android_major_version = $1;
    } else {
    	if (! grep {$_ eq $arg} @known_schema_list) {
    		print "Invalid schema: $arg.\n Valid schemas are:\n";
    		print "\t", join("\n\t", @known_schema_list);
    		print "\n";
    		exit;
    	}
	$schemas{$arg} = $schema_cnt++;
    }
}


if ($android_major_version eq "") {
	die "ANDROID_BUILD_TOP not set, please run envsetup.sh and lunch, or specify --android_version" unless $ENV{"ANDROID_BUILD_TOP"};
	$android_major_version = get_android_major_version();
}

my @vendor_schemas = get_schemas($vendor);

if($schemas_supplied == 0) {
	my $i = 1;
	foreach my $schem (@vendor_schemas ){
		$schemas{$schem} = $i++;
	}
}

my $sqlstatvendordir;
if($vendor ne "generic") {
	$sqlstatvendordir = File::Spec->catdir(File::Spec->catdir($sqlstatdir, "vendors"), "vendor_adapt");
}
 

print "Updating statistics\n";
print "================\n";
print "Database: \n\t$MIMER_DATABASE \n";
print "Android version: \n\t$android_major_version \n";
print "Schemas:\n";
foreach my $s (sort { $schemas{$a} <=> $schemas{$b} } keys %schemas) {
	print "\t$s \n";
}

chdir("$mimdir/sql/stat_data");
my $cur_schema;

if($wipe_first == 1) {
    deleteData();
}

insertData();

if($global_stat == 1) {
	updateGlobalStat();
}

if($wipe_first == 0) {
    deleteData();
}

restartMimer();

#This extra call to deleteData is a hack to get around a problem in alter sequence
#deleteData(); 

resetFileSize();

restartMimer();



##### End of the main program


sub insertData {
	foreach $cur_schema (sort { $schemas{$a} <=> $schemas{$b} } keys %schemas) {
		if( $cur_schema eq 'contacts') {
			if( $android_major_version eq "4.0" ) {
				print "Insert Contacts data\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "contacts-40.sql");
			} else {
				#The Qualcomm Technologies Contacts stat file is self contained, so don't include the generic one.
				if($vendor eq "qti") {
					print "Insert Qualcomm Technologies Contacts data\n";
					$sqlstatfile = File::Spec->catfile($sqlstatvendordir, "contacts_all_41.sql");
					@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			                if($debug || $?) { print (@output); }
					
					if( $android_major_version > "4.2" ) {
					        print "Insert Qualcomm Technologies Contacts 4.3 data\n";
						$sqlstatfile = File::Spec->catfile($sqlstatvendordir, "contacts-43.sql");
						@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			                        if($debug || $?) { print (@output); }
					}
				} else {
					print "Insert Contacts data\n";
					$sqlstatfile = File::Spec->catfile($sqlstatdir, "contacts_all_41.sql");
					@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			                if($debug || $?) { print (@output); }
					
					if( $android_major_version > "4.2" ) {
					        print "Insert Contacts 4.3 data\n";
						$sqlstatfile = File::Spec->catfile($sqlstatdir, "contacts-43.sql");
						@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			                        if($debug || $?) { print (@output); }
					}
				}
			}
			
			if($global_stat == 0) {
				print "Update statistics for $cur_schema (and profile)\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "contacts_all_stat.sql");							
				@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }
			}
		}
		
	
		if( $cur_schema eq 'media') {
			if($all_ext_removable) {
				print "Insert Media data (all external schemas are removable)\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_all_all_ext_removable.sql");
			} else {
				print "Insert Media data\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_all.sql");
			}
			@output = `bsql -u media -p mediapswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
			
			if($android_major_version > "4.0") {
				if($all_ext_removable) {
					print "Insert JB Media data (all external schemas are removable)\n";
					$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_all_all_ext_removable-41.sql");
				} else {
					print "Insert JB Media data\n";
					$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_all-41.sql");
				}
				@output = `bsql -u media -p mediapswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }				
			}
			
			if($global_stat == 0) {
				if($all_ext_removable) {
					print "Update statistics for $cur_schema (all external schemas are removable)\n";
					$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_all_stat_all_ext_removable.sql");
				} else {
					print "Update statistics for $cur_schema\n";
					$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_all_stat.sql");
				}			
				@output = `bsql -u media -p mediapswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }
			}
		}
	
		if( $cur_schema eq 'telephony') {
			print "Insert Telephony data\n";
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "telephony.sql");
			@output = `bsql -u telephony -p telephonypswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
			if($global_stat == 0) {
				print "Update statistics for $cur_schema\n";
				@output = `bsql -u telephony -p telephonypswd --query=\"update statistics for ident $cur_schema\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }
			}
		}
	
		if( $cur_schema eq 'calendar') {
			print "Insert Calendar data\n";
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "calendar.sql");
			@output = `bsql -u calendar -p calendarpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
			
			if( $android_major_version > "4.2" ) {
			    print "Insert Calendar 4.3 data\n";
			    $sqlstatfile = File::Spec->catfile($sqlstatdir, "calendar-43.sql");
			    @output = `bsql -u calendar -p calendarpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}			
			
			if($global_stat == 0) {
				print "Update statistics for $cur_schema\n";
				@output = `bsql -u calendar -p calendarpswd --query=\"update statistics for ident $cur_schema\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }
			}
		}
	
	
		if( $cur_schema eq 'email') {
			print "Insert Email data\n";
			if( $android_major_version > "4.0" ) {
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "email-41.sql");
			} 
			else	{
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "email.sql");
			}
			@output = `bsql -u email -p emailpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
                        if( $android_major_version > "4.3" ) {
			    print "Insert Email 4.4 data\n";
			    $sqlstatfile = File::Spec->catfile($sqlstatdir, "email-44.sql");
			    @output = `bsql -u email -p emailpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}
			if($global_stat == 0) {
				print "Update statistics for $cur_schema\n";
				@output = `bsql -u email -p emailpswd --query=\"update statistics for ident $cur_schema\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }
			}
		}
	
		if( $cur_schema eq 'compatibility') {
			if($global_stat == 0) {
				print "Update statistics for $cur_schema\n";
				@output = `bsql -u SYSADM -p SYSADM --query=\"update statistics for ident SYSADM\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }
			}
		}
	}

} #End insertData()


sub updateGlobalStat {
	print "Update statistics for all schemas\n";
	@output = `bsql -u SYSADM -p SYSADM --query=\"update statistics\" $MIMER_DATABASE`;
	if($debug || $?) { print (@output); }
}

sub deleteData {
	foreach $cur_schema (sort { $schemas{$a} <=> $schemas{$b} } keys %schemas) {
		if( $cur_schema eq 'media') {
			if($all_ext_removable) {
				print "Delete Media data (all external schemas are removable)\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_reset_40_all_ext_removable.sql");
			} else {
				print "Delete Media data\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_reset_40.sql");
			}				
			@output = `bsql -u media -p mediapswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
			
			if($android_major_version > "4.0") {
				if($all_ext_removable) {
					print "Delete JB Media data (all external schemas are removable)\n";
					$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_reset_41_all_ext_removable.sql");
				} else {
					print "Delete JB Media data\n";
					$sqlstatfile = File::Spec->catfile($sqlstatdir, "media_reset_41.sql");
				}
				@output = `bsql -u media -p mediapswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }				
			}
		}
	
		if( $cur_schema eq 'contacts') {
			print "Delete Contacts data\n";
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "resetraw_100.sql");
			@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
			for( 1..50 ){
			    @output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "resetall.sql");
			@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\"  $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
			if( $android_major_version gt "4.3" ) {
			    print "Delete Contacts 4.3 data\n";
			    $sqlstatfile = File::Spec->catfile($sqlstatdir, "contacts-43-cleanup.sql");
			    @output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\"  $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}
			if($vendor eq "qti") {
				print("Delete Qualcomm Technologies Contacts data\n");
				if( $android_major_version eq "4.1" ) {
					$sqlstatfile = File::Spec->catfile($sqlstatvendordir, "contacts-41-cleanup.sql");
				} else {
					$sqlstatfile = File::Spec->catfile($sqlstatvendordir, "contacts-43-cleanup.sql");
				}				
				@output = `bsql -u contacts -p contactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
				if($debug || $?) { print (@output); }
			}


		}
	
	
		if( $cur_schema eq 'telephony') {
			print "Delete Telephony data\n";
			for( 1..50 ){
			    @output = `bsql -u telephony -p telephonypswd --query=\"delete from addr where \\"_id\\" > (select max(\\"_id\\") - 100 from addr);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			    @output = `bsql -u telephony -p telephonypswd --query=\"delete from part where \\"_id\\" > (select max(\\"_id\\") - 100 from part);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			    @output = `bsql -u telephony -p telephonypswd --query=\"delete from pdu where \\"_id\\" > (select max(\\"_id\\") - 100 from pdu);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			    @output = `bsql -u telephony -p telephonypswd --query=\"delete from sms where \\"_id\\" > (select max(\\"_id\\") - 100 from sms);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}
	
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_telephony.sql");
			@output = `bsql -u telephony -p telephonypswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
		}
	
		if( $cur_schema eq 'calendar') {
			print "Delete Calendar data\n";
			for( 1..50 ){
			    @output = `bsql -u calendar -p calendarpswd --query=\"delete from Attendees where \\"_id\\" > (select max(\\"_id\\") - 100 from Attendees);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			    @output = `bsql -u calendar -p calendarpswd --query=\"delete from Events where \\"_id\\" > (select max(\\"_id\\") - 100 from Events);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			    @output = `bsql -u calendar -p calendarpswd --query=\"delete from EventsRawTimes where \\"_id\\" > (select max(\\"_id\\") - 100 from EventsRawTimes);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			    @output = `bsql -u calendar -p calendarpswd --query=\"delete from Instances where \\"_id\\" > (select max(\\"_id\\") - 100 from Instances);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_calendar.sql");
			@output = `bsql -u calendar -p calendarpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }		
		}
	
		if( $cur_schema eq 'email') {
			print "Delete Email data\n";
			for( 1..50 ){
			    @output = `bsql -u email -p emailpswd --query=\"delete from Body where \\"_id\\" > (select max(\\"_id\\") - 100 from Body);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			    @output = `bsql -u email -p emailpswd --query=\"delete from Message where \\"_id\\" > (select max(\\"_id\\") - 100 from Message);\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_email.sql");
			@output = `bsql -u email -p emailpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
                        if( $android_major_version > "4.3" ) {
			    print "Reset Email 4.4 data\n";
			    $sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_email_44.sql");
			    @output = `bsql -u email -p emailpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			    if($debug || $?) { print (@output); }
			}
		}
	}
} #End deleteData()


sub restartMimer {
	print "Restarting Mimer SQL\n";
	if($use_sdk) {
		
		@output = `perl $mimerservice stop`;
		if($debug || $?) { print (@output); }
		sleep(5);
		@output = `perl $mimerservice start`;
		if($debug || $?) { print (@output); }
		sleep(5);
	} else {
		@output = `mimcontrol --stop $MIMER_DATABASE`;
		if($debug || $?) { print (@output); }
		@output = `mimcontrol --start $MIMER_DATABASE`;
		if($debug || $?) { print (@output); }
	}
}


sub resetFileSize {
	#Reset file sizes
	foreach $cur_schema (sort { $schemas{$a} <=> $schemas{$b} } keys %schemas) {
		if( $cur_schema eq 'media') {
			if($all_ext_removable) {
				print "Reset Media (all external schemas are removable)\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_filesizes_media_all_ext_removable.sql");
			} else {
				print "Reset Media\n";
				$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_filesizes_media.sql");
			}					
				
			@output = `bsql -umedia -pmediapswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
		}
	
		if( $cur_schema eq 'contacts') {
			print "Reset Contacts\n";
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_filesizes_contacts.sql");
			@output = `bsql -ucontacts -pcontactspswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
		}
	
	
		if( $cur_schema eq 'telephony') {
			print "Reset Telephony\n";
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_filesizes_telephony.sql");
			@output = `bsql -utelephony -ptelephonypswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
		}
	
		if( $cur_schema eq 'calendar') {
			print "Reset Calendar\n";
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_filesizes_calendar.sql");
			@output = `bsql -ucalendar -pcalendarpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
			
		}
	
		if( $cur_schema eq 'email') {
			print "Reset Email\n";
			$sqlstatfile = File::Spec->catfile($sqlstatdir, "reset_filesizes_email.sql");
			@output = `bsql -uemail -pemailpswd --query=\"read '$sqlstatfile'\" $MIMER_DATABASE`;
			if($debug || $?) { print (@output); }
		}
	}
	
	@output = `bsql -uSYSADM -pSYSADM --query=\"alter databank transdb drop filesize\" $MIMER_DATABASE`;
	if($debug || $?) { print (@output); }
	@output = `bsql -uSYSADM -pSYSADM --query=\"alter databank sqldb drop filesize\" $MIMER_DATABASE`;
	if($debug || $?) { print (@output); }
} #end resetFileSize




