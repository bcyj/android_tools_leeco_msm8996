# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com

##
## Script to convert all SQLite java files in a directory or directories to Mimer ones.
##
use warnings;
use strict;
use File::Copy;
use File::Spec;
use File::Path;

my @args;
my $newpackage = "";
my $ident = "MIMERSERVICE";
my $password = "MIMERSERVICEPSWD";

my $ANDROID_BUILD_TOP = $ENV{"ANDROID_BUILD_TOP"};
my $ANDROID_MAJOR_VERSION = `cat $ANDROID_BUILD_TOP/build/core/version_defaults.mk | grep "PLATFORM_VERSION :=" | cut -f2 -d"=" | tr -d ' '`;
die "Android version not found in $ANDROID_BUILD_TOP/build/core/version_defaults.mk" if (!$ANDROID_MAJOR_VERSION);

my $curarg = 0;
my $argcnt = 0;
my $recurse = 0;
my $verbose = 0;
$verbose = $ENV{VERBOSE} if (defined $ENV{VERBOSE});
my @ARGSTACK;
ARGLOOP:
while ($curarg<=$#ARGV || $#ARGSTACK>=0) {
    my $param;

    if ($#ARGSTACK>=0) {
	$param = shift @ARGSTACK;
    } else {
	$param = $ARGV[$curarg++];
    }

    ## Binary options, like -notry

    if ($param =~ /^--package=(.+)/) {
	$newpackage = $1;
    } elsif ($param =~ /^--ident=(.+)/) {
	$ident = $1;
    } elsif ($param =~ /^--password=(.+)/) {
	$password = $1;
    } elsif ($param =~ /^-R$/) {
	$recurse = 1;
    } elsif ($param =~ /^-v$/) {
	$verbose++;
    } elsif ($param =~ /^-/) {
	print "Unknown option: $param\n";
	usage();
	exit();
    } else {
	$args[$argcnt++] = $param;
    }
}

if ($argcnt<1) {
    usage();
    exit();
}

push(@args,".") if ($#args<0);
foreach my $dirname (@args) {
    if (-d $dirname) {
	convert_directory($dirname,"",$newpackage,$newpackage);
    } else {
	convert_file($dirname,"",$newpackage,$newpackage);
    }
}
exit(0);

sub convert_directory {
    my ($dirname,$basepackage,$newbasepackage,$newpackage) = @_;

    print "convert_directory($dirname,$basepackage,$newbasepackage,$newpackage)\n" if ($verbose);

    if (opendir(DIR,$dirname)) {
	my @allfiles = grep {/^[^\.]/} readdir(DIR);
	close(DIR);

	foreach my $file (@allfiles) {
	    my $subfile = File::Spec->catfile($dirname,$file);
	    if (-d $subfile) {
		my $subpackage = "$newpackage"; ##.$file";
		convert_directory($subfile,$basepackage,$newbasepackage,$subpackage) if ($recurse);
		next;
	    }

	    next if ($file !~ /\.java$/i);

	    $basepackage = determine_package($subfile) if ($basepackage eq "");
	    convert_file($subfile,$basepackage,$newbasepackage,$newpackage);
	}
    } else {
	print STDERR "Error. Could not open directory $dirname: $!\n";
    }
}

sub convert_file {
    my ($file,$basepackage,$newbasepackage,$newpackage) = @_;

    my ($volume,$directories,$filename) = File::Spec->splitpath($file);

    my $newfile = $filename;
    $newfile =~ s/SQLite/Mimer/;
    $newfile =~ s/sqlite/mimer/;

    my $package = "";
    my $package_regexp = "";
    my $file_package_regexp = "";
    my $file_newbasepackage = $newbasepackage;
    $file_newbasepackage =~ s/\./\//g;

    my @outlines;
    my @databaseobjects;
    my @statementobjects;
    my @fileobjects;

    my %openhelperclass;

    my $lastlinesemicolon = 0;  ## 1 if the most recent line ended with a semi-colon

    my $mimerdatabase_imported = 0;

    open(FILE,$file) or die "Could not open file $file: $!";
    while (<FILE>) {
	if (/^\s*package\s+([A-Za-z0-9\._]+)/) {
	    $package = $1;
	    $package_regexp = ($basepackage?$basepackage:$package);
	    $file_package_regexp = $package_regexp;
	    $package_regexp =~ s/\./\\\./g;
	    $file_package_regexp =~ s/\./\//g;
	    print "package_regexp=$package_regexp\n" if ($verbose>1);
	    print "file_package_regexp=$file_package_regexp\n" if ($verbose>1);
	}
	## The class com.google.android.mms.util.SqliteWrapper is a special case. This class
        ## is found in the framework on some versions of Android (not exactly sure which).
        ## The class is however nearly identical to android.database.sqlite.SQLiteWrapper 
        ## which there is a Mimer implementation of. 
	s/com\.google\.android\.mms\.util\.SqliteWrapper/android.database.mimer.MimerWrapper/g;

	s/SQLite/Mimer/g;
	s/sqlite/mimer/g;
	s/Sqlite/Mimer/g;
	s/([^\w])DatabaseUtils([^\w])/$1MimerDatabaseUtils$2/g;
        s/([^\w])DatabaseErrorHandler([^\w])/$1MimerDatabaseErrorHandler$2/g;
        s/([^\w])DatabaseErrorHandler([^\w])/$1MimerDefaultDatabaseErrorHandler$2/g;

	if( /import\s+android.database.mimer.MimerDatabase/ ){
	    $mimerdatabase_imported = 1;
	}

	if ($ANDROID_MAJOR_VERSION ge "4") {
	    s/com\.android\.common\.content\.SyncStateContentProviderHelper/com.mimer.common.content.SyncStateContentProviderHelper/g;
	} else {
	    s/com\.android\.internal\.content\.SyncStateContentProviderHelper/com.mimer.internal.content.SyncStateContentProviderHelper/g;
	}
	s/android\.content\.AbstractSyncableContentProvider/com.mimer.content.AbstractSyncableContentProvider/g;
	s/android\.content\.AbstractTableMerger/com.mimer.content.AbstractTableMerger/g;

	if ($newpackage ne $package && $newpackage ne "" && $package ne "") {
##	    print "s/$package_regexp/$newbasepackage/g\n" if ($verbose>1);
	    s/$package_regexp/$newbasepackage/g;
	}

	## Remember which object names refers to a MimerDatabase object
	if (/^([^"]|\/[^\/\*])*MimerDatabase\s+(\w+)\s*[=;]/) {
	    push(@databaseobjects,$2);
	}

	## Remember which object names refers to a MimerStatement object
	if (/^([^"]|\/[^\/\*])*MimerStatement\s+(\w+)\s*[=;]/) {
	    push(@statementobjects,$2);
	}

	## Remember which object names refers to a File object
	if (/^([^"]|\/[^\/\*])*File\s+(\w+)\s*[=;]/) {
	    push(@fileobjects,$2);
	}

	## Assignments to MimerDatabase objects must be handled with care since...
	## ...the SQLite version of this call uses a file specification which Mimer...
	## ...handles with an ident/password combination.
	if (/((\w+)\s*=\s*[^"]*)?\.open(OrCreate)?Database\(/) {
	    my $before = $`;
	    my $lvalue = $2;
	    my $path = "\"\"";
	    my $mode = 0;
	    $before =~ s/MimerDatabase$//;
            ##\(\s*("([^"]|"")*|[^,\)]+)"\s*,\s*[^,\)]+\s*,\s*[^,\)]+\s*\)/) {
	    ## Accept that these statements are spread over at the most two rows.
            $_ .= <FILE> if (!/;\s*$/);
            $_ .= <FILE> if (!/;\s*$/);
	    if (/([^\s.]+)\.open(OrCreate)?Database\(\s*("([^\\]|\\.)*"|[^"\s,]+)\s*,/) {
		$path = $3;
		## If the call was to the method openOrCreateDatabase which wasn't the static 
		## SQLiteDatabase.openOrCreateDatabase (we have already converted SQLiteDatabase to MimerDatabase 
		## so that is what we look for in this expression) it is likely that it was a call to Context.openOrCreateDatabase
		## which does some preprocessing on the database name. Therefore we handle it separately.
		if ( (defined $2 && $2 eq "OrCreate") && $1 ne "MimerDatabase") {
		    $path = "$1.getDatabasePath($path)";
		}
	    }
	    $mode = "MimerDatabase.OPEN_READONLY" if (/SQLiteDatabase.OPEN_READONLY/);
	    unless( defined $lvalue ){
		$lvalue = "";
	    }
	    if ($lvalue eq "" || grep {$_ eq $lvalue} @databaseobjects) {
		$_ = $before.$lvalue.($lvalue ne ""?" = ":"")."MimerDatabase.openDatabase($path,\"$ident\",\"$password\",$mode);\n";
	    }	    
	}

	## If we saw a call to a "delete" method this might mean that the application is deleting the SQLite...
	## ...database file. In this case, we inject a call to Mimer to drop the corresponding databank.
	if (/^(\s*)(\w+)\.delete\(\)/) {
	    my $lvalue = $2;
	    if (grep {$_ eq $lvalue} @fileobjects) {
		unless($mimerdatabase_imported){
		    addImport(\@outlines, "import android.database.mimer.MimerDatabase;\n");
		}
		$mimerdatabase_imported = 1;
		$_ = "$1MimerDatabase.dropDatabank($2,\"$ident\",\"$password\");\n";
	    } 
	}

	## If we saw a call to Context.deleteDatabase the probability is even higher that this is about deleting an actual database.
	## As above, in this case, we inject a call to Mimer to drop the corresponding databank.
	if (/([^\s.]+)\.deleteDatabase\(\s*([^\s]+)\s*\)/) {
	    unless($mimerdatabase_imported){
		addImport(\@outlines, "import android.database.mimer.MimerDatabase;");
	    }
	    $mimerdatabase_imported = 1;
	    $_ = "$`MimerDatabase.dropDatabank($1.getDatabasePath($2),\"$ident\",\"$password\");\n";
	}

	## If we are seeing a MimerStatement.executeInsert which does not use the return value, convert it to
	## a MimerStatement.execute to be able (possibly) to batch the executes.
	if ($lastlinesemicolon && /^(\s*(\w+)\.)executeInsert\(\s*\)\s*;/) {
            my $linestart = $1;
            my $stmtobjectname = $2;
            if (grep {$_ eq $stmtobjectname} @statementobjects) {
                $_ = $linestart."execute();\n";
            }
        }

        ## Add ident and password to MimerOpenHelper calls
        if ($ident ne "" && $password ne "" && /(MimerOpenHelper\(.+)\)/) {
	    $_ = "$`$1,\"$ident\",\"$password\")$'";
	}

	## Skip all @overrides in the generated code
        if (/^\s*\@override/i) {
	    s/\@override//i;
	}
	
	## If we are looking at a class definition which inherits from SQLiteOpenHelper/MimerOpenHelper, remember the name if this class
        if (/class\s+(\w+)\s+extends\s+MimerOpenHelper/) {
	    $openhelperclass{$1} = 1;
	    if( $ident eq "media" ){
		s/MimerOpenHelper/MediaOpenHelper/;
		addImport(\@outlines, "import android.database.mimer.MediaOpenHelper;\n");
	    }
	}
 
        ## If we are looking at a method declaration that is a constructor in a class that inherits from SQLiteOpenHelper,
        ## look for the super()-call and augment that (if found) with  Mimer user id and password. This relies on the fact that 
        ## constructors cannot do anything before the call to super().
        if (/^\s*(\/\* package \*\/|public|private|protected)?\s+(\w+)\s*\((\s*Context\s+\w+\s*)?/) {
	    my $class = $2;
	    my $firstparam = $3;
	    ## Ensure that the routine definition is a constructor of a class that inherits from SQLiteOpenHelper
	    if ($openhelperclass{$class}) {
		## If the entire statement has not been seen yet (the call is spread over several lines), we
		## read another line (just one for now) and try to figure out the parameters again */
		if (!defined $firstparam) {
		    $_ .= <FILE>;

		    s/SQLite/Mimer/g;
		    s/sqlite/mimer/g;
		    s/Sqlite/Mimer/g;
		    s/([^\w])DatabaseUtils([^\w])/$1MimerDatabaseUtils$2/g;
		    
		    if (/^\s*(\/\* package \*\/|public|private|protected)?\s+(\w+)\s*\((\s*Context\s+\w+\s*)?/) {
			$firstparam = $3;
		    }
		}
		
		## If the first parameter is not of the Context type we are probably looking at something else so 
		## skip doing anything here now
		if ($firstparam =~ /\s*Context\s+\w+\s*/) {
		    push(@outlines,$_);
		    
		    my $maxsearch = 10; ## Search at most 10 lines for the super() call
		    while (<FILE>) {
			die "Unable to find a super()-call in a SQLiteOpenHelper subclass in $file" if ($maxsearch--==0);
			
			if (/^\s*super\(/) {
			    die "$file contains a super() call to SQLiteOpenHelper which wasn't recognized properly" if (!/\)\s*;/);
			    s/\)\s*;/,"$ident","$password"\);/;
			    last;
			}
			
			s/SQLite/Mimer/g;
			s/sqlite/mimer/g;
			s/Sqlite/Mimer/g;
			s/([^\w])DatabaseUtils([^\w])/$1MimerDatabaseUtils$class/g;
			s/([^\w])DatabaseErrorHandler([^\w])/$1MimerDatabaseErrorHandler$2/g;
			s/([^\w])DatabaseErrorHandler([^\w])/$1MimerDefaultDatabaseErrorHandler$2/g;
			
			push(@outlines,$_);
		    }
		}
	    }
	}

	push(@outlines,$_);

	if (/;$/) {
	    $lastlinesemicolon = 1;
	} else {
	    $lastlinesemicolon = 0;
	}
    }
    close(FILE);

    if ($package ne "") {
        if (!$newpackage) {
	    $newpackage = $package;
  	    $newpackage =~ s/sqlite/mimer/g;
        }
#        if ($newpackage eq $package) {
#            $newpackage =~ s/^com\./com.mimer./;
#        }
    
    }

    my $newdirectories = $directories;
    $newdirectories =~ s/$file_package_regexp/$file_newbasepackage/;
    $newdirectories = "." if ($newdirectories eq "");
	
    print "newdirectories=$newdirectories\n" if ($verbose>1);
    print "newfile=$newfile\n" if ($verbose>1);
    my $newfilename = File::Spec->catfile($newdirectories,$newfile);

    if($newfilename ne ""){
      unlink($file);
    }

    mkpath($newdirectories) or die "Could not create directories $newdirectories: $!" if (!-d $newdirectories);

    ## Make the filename tidier by removing all parent directory references.
    while ($newfilename =~ s/\w+\/\.\.\///) {}
    print "$newfilename\n";

    open(FILEOUT,">$newfilename") or die "Could not create new file $newfilename: $!";
    foreach (@outlines) {
        s/^([^"]*package\s+)$package_regexp(.*;)/$1$newpackage$2/;
        print FILEOUT;
    }
    close(FILEOUT);
}

sub addImport {
    my $a_ref = shift;
    my $imp = shift;

    foreach my $i ( 0..$#{$a_ref} ) {
	if( $a_ref -> [$i] =~ /import/ ){
	    splice(@{$a_ref}, $i,0, ($imp));
	    return;
	}
    }
}

sub usage {
    print "Convert.pl\n";
    print "Usage:\n";
    print "perl Convert.pl [<options>] <sourcedirs>\n";
    print "Options:\n";
    print "--package=<newpackagename>      New package name. By default, a generated package\n";
    print "                                is used.\n";
    print "-R                              Recurse and convert files in subfolders as well.\n";
    print "Example:\n";
    print "perl convert_to_mimer.pl .\n";
}

sub determine_package {
    my ($filename) = @_;

    open(FIL,$filename) or die "Could not open file $filename: $!";
    while (<FIL>) {
	if (/^\w*package ([^;]+);/) {
	    close(FILE);
	    return $1;
	}
    }
    close(FIL);
    return undef;
}


