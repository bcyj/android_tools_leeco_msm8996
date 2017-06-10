# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
package mimer_common;
use strict;
use warnings;
 
our $VERSION = '1.00';
 
use base 'Exporter';
 
#our @EXPORT = qw(get_android_major_version);
our @EXPORT = qw(get_android_major_version get_android_minor_version get_schemas get_vendors);

sub get_android_major_version
{
    return get_android_version(0);
}

sub get_android_minor_version
{
    return get_android_version(1);
}

sub get_android_version
{
    my $get_minor_version = 0;
    if($_[0]) {
        $get_minor_version = $_[0];
    }
    my $build_top = $ENV{"ANDROID_BUILD_TOP"};
    open(VERSIONS, File::Spec->catfile(File::Spec->catdir($build_top, "build","core"),"version_defaults.mk"));
    
    my @versions = <VERSIONS>;
    
    my @version = grep(/PLATFORM_VERSION :=(.*)/, @versions);
    
    if( @version != 1 ){
	die "*** Unable to determine version from version_defaults.mk";
    }
    my $res = -1;
    $version[0] =~ /(\d\.\d)(?:\.(\d))?/;
    if($get_minor_version == 0) {
        $res = $1;
    } else {
        if($2) {
			$res= $2;
        }
    }
    
    return $res;
}


sub get_schemas {
	my $vendor = shift;
	my @schemas;
	
	if( !defined $vendor ){
	 	@schemas=('compatibility','contacts','media','telephony','calendar','email');
	 } else {
	 
		if($vendor eq 'generic'
			|| $vendor eq 'qti'
		) {
			@schemas=('compatibility','contacts','media','telephony','calendar','email');
		}
		else {
			die "Unkown vendor: $vendor";
		}
	}
	return @schemas;
}

sub get_vendors {
	my @vendors;
	push(@vendors, 'generic');
	push(@vendors, 'qti');
	return @vendors;
}
