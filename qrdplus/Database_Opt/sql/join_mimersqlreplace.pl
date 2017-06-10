##
## Script which joins two or more mimersqlreplace XML definition files to form
## another XML definition file holding all the contents of the included files.
##
## The output is written to standard output.
##
## Usage:
##  perl join_mimersqlreplace.pl [<options>] <list_of_input_files_separated_by_blanks>
## Options:
##  --outputfile=nn     Specify the file nn as the output file. Default is standard output.
##  --keepblanks        Keep blanks in replacement SQL strings. By default, the script attempts to remove unnecessary blanks. Currently, this includes leading and trailing blanks on each line.
##

## Simplest possible implementation. No XML validation, just concatenate two files, throw away 
## the ending </MimerSQLReplace>-tag of the first file, and the leading <?xml-tag and the 
## <MimerSQLReplace>-tag of the second.

use warnings;
use strict;
use File::stat;
use File::Spec;
use File::Path;
use XML::Parser;

my $has_seen_start = 0;

my $android_major_version = -1;
my $android_full_version = "";

my $javaoutputfile;
my $javaxmlfile = 0;
my $javapackage;
my $javaoutputpending="";
my $outputfile;
my $keepblanks=0;
my $check_file_dates=1;
my $in_statement=0;


## global variables for ID checks
my %fromSQL_ids;

## global variables for verification
my %error_code_stat;
my %fromSQL_dupes;
my $verify_sql=0;
my $find_dupes=1;
my $test_compile=1;
my $verify_db = '';
my $replace_count = 0;
my $replace_success_count = 0;
my @args;

foreach (@ARGV) {
    if (/^-/) {
	if (/^--outputfile=(.+)$/) {
	    $outputfile = $1;
	} elsif (/^--javaoutputfile=(.+)$/) {
	    $javaoutputfile = $1;
	} elsif (/^--javaxml$/) {
	    $javaxmlfile = 1;
	} elsif (/^--package=(.+)$/) {
	    $javapackage = $1;
	} elsif (/^--keepblanks$/) {
	    $keepblanks=1;
	} elsif (/^--force$/) {
	    $check_file_dates=0;
	} elsif (/^--verify_sql/){
	    $verify_sql=1;
	} elsif (/^--verify_db=(.+)$/){
	    $verify_db = $1;
	} elsif (/^--find_dupes=(.+)$/){
	    $find_dupes=$1;
	} elsif (/^--test_compile=(.+)$/){
	    $test_compile=$1;
	} elsif (/^--android_version=(.+)$/){
		$android_full_version = $1;
		$android_full_version =~ /(\d\.\d)(?:\.(\d))?/;
		$android_major_version = $1;
	} else {
	    die "Illegal argument: $_";
	}
    } else {
	push(@args,$_);
    }
}
die "No input files specified on command line." if ($#args==-1);

die "Please run env_setup.sh" unless $ENV{'ANDROID_BUILD_TOP'};


my $provider_name;
if( $verify_sql ){
    die "Package name must be specified with verify_sql. (To guess password)" unless $javapackage;

    $javapackage =~ /.+\.([^\.]+)/; # Find last word in dot-separated package name
    $provider_name = $1;
    die "Unable to guess provider name" unless $provider_name;

    if( -e "tmp.sql" ){
	die "Temporary file tmp.sql already exists, please remove or rename it";
    }

    system("bsql -u $provider_name -p $provider_name". 'pswd -q "select 1 from system.onerow;" ' . $verify_db .' > /dev/null' );
    $verify_db = 'default' unless( $verify_db);
    die "Unable to connect to $verify_db database." if( $? );
}

## If no package is specified, try to figure it out from the output file name assuming the standard Android
## directory structure.

if (defined $javaoutputfile) {
    if ($javapackage eq "") {
	my $abs_javaoutputfile = File::Spec->rel2abs($javaoutputfile);
	my ($volume,$directories,$file) = File::Spec->splitpath($abs_javaoutputfile);
	my @dirs = File::Spec->splitdir($directories);
	my $found_gen = 0;
	foreach (@dirs) {
	    next if (length($_)==0);
	    if ($found_gen) {
		$javapackage .= "." if (length($javapackage)>0); 
		$javapackage .= $_;
	    }
	    if ($_ eq "gen" || $_ eq "src") {
		$javapackage = "";
		$found_gen = 1;
	    }
	}
	$javapackage = "" if (!$found_gen);
    }
    
## If we still has no package name, see if there are other .java files in the same directory. If 
## so we use the same package name as they are. 
    
    if ($javapackage eq "") {
	my $abs_javaoutputfile = File::Spec->rel2abs($javaoutputfile);
	my ($volume,$directories,$file) = File::Spec->splitpath($abs_javaoutputfile);
	my $dirname = File::Spec->catpath($volume,$directories);
	if (opendir(DIR,$dirname)) {
	    my @files = grep {/\.java$/i} readdir(DIR);
	    closedir(DIR);
	    
	    foreach (@files) {
		if (open(JAVAFILE,$_)) {
		    while (<JAVAFILE> && $javapackage eq "") {
			$javapackage = $1 if (/^package (.+);/);
		    }
		    close(JAVAFILE);
		}
		last if ($javapackage ne "");
	    }
	}
    }

    die "No package name specified or could be deduced from file name or other Java files" if ($javapackage eq "");
}

## Check if any of the input files are newer than the output file. We do this in order not to 
## wreck the subsequent make procedure. Always constructing a new file would make the
## resources to always be rebuilt.

if ($check_file_dates && (defined $outputfile || defined $javaoutputfile)) {
    my $outputfiledate = 0;
    my $javaoutputfiledate = 0;
    my $newestinputfiledate = 0;
    $outputfiledate = stat($outputfile)->mtime if (defined $outputfile && -e $outputfile);
    $javaoutputfiledate = stat($javaoutputfile)->mtime if (-e $javaoutputfile);
    foreach (@args) {
	if (-e $_) {
	    my $input_time = stat($_)->mtime;
	    $newestinputfiledate = $input_time if ($input_time>$newestinputfiledate);
	}
    }    

    exit(0) if( not ((defined $outputfile && $outputfiledate<$newestinputfiledate) || (defined $javaoutputfile && $javaoutputfiledate<$newestinputfiledate)) );

	if($outputfile) {
    	unlink($outputfile);
	}
	if($javaoutputfile) {
    	unlink($javaoutputfile);
	}
}

##
##
##

my $outfile = *STDOUT;
if ($outputfile) {
    open(OUTPUTFILE,">$outputfile") or die "Could not create output file $outputfile: $!";
    $outfile = *OUTPUTFILE;
}

if (defined $javaoutputfile) {
    my ($volume,$directories,$file) = File::Spec->splitpath($javaoutputfile);
    my $dirname = File::Spec->catpath($volume,$directories);
    mkpath($dirname) or die "Could not create directory $dirname: $!" if (!-e $dirname && $dirname ne "");

    open(JAVAOUTPUTFILE,">:encoding(UTF-8)", "$javaoutputfile") or die "Could not create Java output file $javaoutputfile: $!";
    print JAVAOUTPUTFILE "/* AUTO-GENERATED FILE.  DO NOT MODIFY.\n";
    print JAVAOUTPUTFILE " *\n";
    print JAVAOUTPUTFILE " * This class was automatically generated by the\n";
    print JAVAOUTPUTFILE " * join_mergefile.pl script from the resource data it found.  It\n";
    print JAVAOUTPUTFILE " * should not be modified by hand.\n";
    print JAVAOUTPUTFILE " */\n\n";
    print JAVAOUTPUTFILE "package $javapackage;\n\n";
    print JAVAOUTPUTFILE "public final class MimerSQLReplace {\n";
    if ($javaxmlfile) {
	print JAVAOUTPUTFILE "  public byte[] xmldata;\n";
	print JAVAOUTPUTFILE "  private java.lang.String xmldataString = \n";
    } else {
	print JAVAOUTPUTFILE "  public static void load() {\n";
    }
}

my $xmlfile = "";
my $input_file ="";
foreach (@args) {
	$input_file = $_;
    open(INPUTFILE,$input_file) or die "Could not open input file $input_file: $!";
    while (<INPUTFILE>) {
	s/^ +// if (!$keepblanks);
	s/ +$// if (!$keepblanks);
	if (/^<\?xml/) {
	    if (/encoding="([^"]+)"?>/) {
		die "Only UTF-8 encoding supported." if ($1 ne "UTF-8");
	    }
	    
	    print $outfile "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" if ($has_seen_start++<1);
	    next;
	}
	if (/<MimerSQLReplace>/) {
	    s/<MimerSQLReplace>// if ($has_seen_start++>1);
	}
	if (/<\/MimerSQLReplace>/) {
	    s/<\/MimerSQLReplace>//;
	}
	
	if ($keepblanks || length($_)>1) {
	    print $outfile $_;
	    
	    if ($javaoutputfile) {
		if ($javaxmlfile) {
		    s/\\/\\\\/g;
		    s/"/\\"/g;
		    s/\n/\\n/g;
		    print JAVAOUTPUTFILE "    \"$_\"+\n";
		} else {
		    $xmlfile .= $_;
		}
	    }
	}
    }
    close(INPUTFILE);
}
print $outfile "</MimerSQLReplace>\n";
$xmlfile .= "</MimerSQLReplace>\n";

if ($outputfile) {
    close(OUTPUTFILE);
}
if ($javaoutputfile) {
    if ($javaxmlfile) {
	print JAVAOUTPUTFILE "    \"\";\n";
	print JAVAOUTPUTFILE "\n";
	print JAVAOUTPUTFILE "    public MimerSQLReplace() {\n";
	print JAVAOUTPUTFILE "        xmldata = xmldataString.getBytes();\n";
    } else {
	my $xmlparser = new XML::Parser(Handlers => {Start => \&handle_start,
				                  End => \&handle_end,
				                  Char => \&handle_char});
	$xmlparser->parse($xmlfile);
    }
    print JAVAOUTPUTFILE "  }\n";
    print JAVAOUTPUTFILE "}\n";
    close(JAVAOUTPUTFILE);
}

if( $verify_sql ){
    my $count = 10;
    print "\n\n --- Top ten errors by error code ---\n";
    foreach my $key (sort { $error_code_stat{$b} <=> $error_code_stat{$a} } keys %error_code_stat) {
	print " --- $error_code_stat{$key} \t $key\n";
	$count --; 
	last unless($count);
    }
    print " --- \n";
    print " --- Total number of tested replacers:     $replace_count ---\n";
    print " --- Total number of successful replacers: $replace_success_count ---\n";
    print " --- \n";
}

exit(0);

my $fromSQL;
my @toSQL;
my $arrcnt=0;
my $translateOnLoad;
my $parameterizeOnLoad;
my $warningsOnLoad;
my $hasID = 0;

sub handle_start {
    my $expat = shift;
    my $element = shift;

    if ($element eq "fromSQL") {
	undef $fromSQL; 
	undef @toSQL;
	$translateOnLoad = "true";
	$parameterizeOnLoad = "true";
	$warningsOnLoad = "true";

	while ($#_>0) {
	    my $attr = shift;
	    my $value = shift;

	    $fromSQL = $value if ($element eq "fromSQL" && $attr eq "statement");

	    if ($element eq "fromSQL")
	    {
		if( $attr eq "translateOnLoad" )
		{
		    if( lc($value) eq "true" || lc($value) eq "false" )
		    {
			$translateOnLoad = lc($value);
		    } 
		    else 
		    {
			die "Bad value: '$value' for attribute translateOnLoad. 'true' or 'false' accepted";
		    }
		}

		if( $attr eq "parameterizeOnLoad" )
		{
		    if( lc($value) eq "true" || lc($value) eq "false" )
		    {
			$parameterizeOnLoad = lc($value);
		    } 
		    else 
		    {
			die "Bad value: '$value' for attribute parameterizeOnLoad. 'true' or 'false' accepted";
		    }
		}

		if( $attr eq "warningsOnLoad" )
		{
		    if( lc($value) eq "true" || lc($value) eq "false" )
		    {
			$warningsOnLoad = lc($value);
		    } 
		    else 
		    {
			die "Bad value: '$value' for attribute warningsOnLoad. 'true' or 'false' accepted";
		    }
		}
		
		if( $attr eq "id" )
		{
		    if( $fromSQL_ids{$value} ){
			die "Duplicate id: $value found, please correct\n";
		    } else {
			$hasID = $value;
			$fromSQL_ids{$value} = 1;
		    }
		}
	    }

	}
    }
}

sub fix_attr_string {
    my ($str) = @_;

    $str =~ s/"/\\"/g;
    $str =~ s/&gt;/>/g;
    $str =~ s/&lt;/</g;

    return $str;
}

sub handle_end {
    my $expat = shift;
    my $element = shift;
    
    if ($element eq "fromSQL") {
	unless( $hasID ){
	    die "Replacer without ID, please correct. fromSQL: $fromSQL\n";
	}

	if( $verify_sql ) {
	    bsql_try($fromSQL, $hasID, $test_compile, $find_dupes);
	}
	my $fromsql = fix_attr_string($fromSQL);
	if ($#toSQL==-1) {
	    print JAVAOUTPUTFILE "    android.database.mimer.MimerSQLReplace.add($translateOnLoad,$parameterizeOnLoad, $warningsOnLoad, \"$fromsql\",\"\");\n";
	} elsif ($#toSQL==0) {
	    my $tosql = fix_attr_string($toSQL[0]);
	    print JAVAOUTPUTFILE "    android.database.mimer.MimerSQLReplace.add($translateOnLoad,$parameterizeOnLoad, $warningsOnLoad, \"$fromsql\",\"$tosql\");\n";
	} else {
	    $arrcnt++;
	    print JAVAOUTPUTFILE "    java.lang.String[] sqlarr$arrcnt = new java.lang.String[".($#toSQL+1)."];\n";
	    for (my $i=0; $i<=$#toSQL; $i++) {
		my $tosql = fix_attr_string($toSQL[$i]);
		print JAVAOUTPUTFILE "    sqlarr".$arrcnt."[$i] = \"$tosql\";\n";
	    }
	    print JAVAOUTPUTFILE "    android.database.mimer.MimerSQLReplace.add($translateOnLoad,$parameterizeOnLoad, $warningsOnLoad, \"$fromsql\",sqlarr$arrcnt);\n";
	}
	$hasID = 0;
	$in_statement = 0;
    } elsif ($element eq "statement") {
	$in_statement++ 
    }
}

sub handle_char {
    my $expat = shift;
    my $string = shift;

    ## Trim leading and trailing blanks
    $string =~ s/\s+$//;
    $string =~ s/^\s+//;
    if ($string =~ /[^\s]/) {
	## To ensure identifiers are always separated, add one blank if necessary
	if (defined $toSQL[$in_statement]) {
	    $toSQL[$in_statement] .= " " if ($toSQL[$in_statement] =~ /\w$/);
	}
        $toSQL[$in_statement] .= $string;
    }
}

sub bsql_try {
    my $sql = shift;
    my $id = shift;
    my $compile_check = shift;
    my $dupe_check = shift;

    open( SQL2, ">:encoding(UTF-8)","trans_input.sql");
    print SQL2 $sql;
    close SQL2;

    open(SQL, ">bsql_input.sql");
    print SQL "set execute off;\nset sqlite on;\n";
    my $use_mosdrv = 0;
    if( $use_mosdrv ){
	print SQL "set explain on;\n";
    }
    my $transSQL = `$ENV{'ANDROID_BUILD_TOP'}/external/mimer/native/SQLiteTranslator/sqlite -q -n trans_input.sql 2> /dev/null`;

    if( $dupe_check ){
	my $dupeSQL;
	if( $? ) {
	    $dupeSQL = $sql;
	} else {
	    $dupeSQL = $transSQL;
	}
	if( $fromSQL_dupes{$dupeSQL} ) {
	    print " --- Duplicate found: $id   \tEQUAL to $fromSQL_dupes{$dupeSQL} ---\n";
	} else {
	    $fromSQL_dupes{$dupeSQL} = $id;
	}
    }

    my $len = 0;
    foreach my $str ( split(/\s+/, $transSQL) ) {
	$len += length($str);
	if($len >= 80){
	    $len = 0;
	    print SQL " \n";
	}
	print SQL ' '.$str;
    }
    
    print SQL ";\nexit;\n";
    close(SQL);

    my $sql_error = 0;
    my $sql_error_string;

    if( $? ){
	$sql_error = 1;
	$sql_error_string = 'Translation failed, no execution';
	print " --- SQL execution of $id   \tFAILED    , Last error: $sql_error_string ---\n";
	$error_code_stat{'Translation failed'} +=1;
    } else {
	if( $compile_check ){
	    if( $use_mosdrv ){
		open(BSQL, "~/V1010/g_dist/bin/mosdrv -u $provider_name -p $provider_name". 'pswd localdb < bsql_input.sql |' );
	    } else {
		open(BSQL, "bsql -u $provider_name -p $provider_name". 'pswd localdb < bsql_input.sql |' );
	    }
	    my %local_errors;
	    my $do_print = 1;
	    while(<BSQL>){
		if( /Mimer SQL error/ || /Connect cancelled/ ){
		    $do_print = 0;
		    chomp;
		    unless( $local_errors{$_} ){
			$local_errors{$_} = 1;
			$error_code_stat{$_} += 1;
			$sql_error_string = $_;
			$sql_error = 1;
		    }
		}
		print if $do_print;

		if( $use_mosdrv && /  <select cost="(\d+)"/ ){
		    print " --- Execution cost of translated query: $1 ---\n";
		}
	    }
	    if( $do_print ){
		print $transSQL, "\n";
	    }
	    close(BSQL);

	    if( $sql_error || $? ){
		print " --- SQL execution of $id   \tFAILED    , Last error: $sql_error_string ---\n";
	    } else {
		print " --- SQL execution of $id   \tSUCCESSFUL, replacer _might_ not be needed ---\n";
		$replace_success_count++;
	    }
	}
    }
    
    $replace_count++;

    close(BSQL);
    unlink("bsql_input.sql");
    unlink("trans_input.sql");
}
