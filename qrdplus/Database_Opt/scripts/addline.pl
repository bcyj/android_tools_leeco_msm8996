# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
##
## Script to insert a new line into a text file.
##
use File::Copy;
use File::Spec;
use File::Path;

my @args;
my $match="";
my $after=0;
my $before=0;
my $savebackup=0;
my $caseinsensitive=0;
my $newlinemarker="<newline>";
my $skipif = "";

ARGLOOP:
while ($curarg<=$#ARGV || $#ARGSTACK>=0) {
    my $param;

    if ($#ARGSTACK>=0) {
	$param = shift @ARGSTACK;
    } else {
	$param = $ARGV[$curarg++];
    }

    ## Binary options, like -notry

    if ($param =~ /^--match=(.+)/) {
	$match = $1;
    } elsif ($param =~ /^--after/) {
	$after = 1;
    } elsif ($param =~ /^--before/) {
	$before = 1;
    } elsif ($param =~ /^--skipif=(.+)/) {
	$skipif = $1;
    } elsif ($param =~ /^-b/) {
	$savebackup = 1;
    } elsif ($param =~ /^-i/) {
	$caseinsensitive = 1;
    } elsif ($param =~ /^--newline=(.+)/) {
	$newlinemarker = 1;
    } elsif ($param =~ /^-/) {
	print "Unknown option: $param\n";
	usage();
	exit();
    } else {
	$args[$argcnt++] = $param;
    }
}

if ($argcnt<0) {
    usage();
    exit();
}

$file = $args[1];
$textline = join("\n",split(/$newlinemarker/,$args[0]))."\n";

open(FILE,$file) or die "Could not open text file $file: $!";
my @lines = <FILE>;
close(FILE);

my @newlines;
my $foundcount=0;
my $lastline;
my $skip_next_equals=0;
my $filechanged=0;

foreach (@lines) {
    if ($skipif ne "" && ($caseinsensitive && /$skipif/ || !$caseinsensitive && /$skipif/i)) {
	exit(0);
    }

    ## If we just inserted a new line after the match line and the next line happens to be equal
    ## to be one just inserted, we skip this line as it appears as we have already inserted 
    ## the line into the file.
    if ($skip_next_equals && $_ eq $lastline) {
	$skip_next_equals = 0;
	$filechanged--;
	next;
    }

    $skip_next_equals = 0;

    my $found =($current==$#lines
		|| $caseinsensitive && /$match/i
		|| !$caseinsensitive && /$match/);
    die "Error. Found several occurrences of $match in $file" if ($found && $foundcount>0);
    
    if ($found && $before && $lastline ne $textline) {
	push(@newlines,$textline);
	$filechanged++;
    }
    push(@newlines,$_);
    $lastline = $_;

    if ($found && $after) {
	push(@newlines,$textline);
	$skip_next_equals=1;
	$lastline = $textline;
	$filechanged++;
    }

    $foundcount++ if ($found);
}

die "Error. No occurrence of $match in $file" if ($foundcount==0);

if ($filechanged>0) {
    copy($file,$file."~") if ($savebackup);

    open(FILE,">$file") or die "Could not create new file $file: $!";
    foreach (@newlines) {
	print FILE;
    }
    close(FILE);

    print "$file\n";
}
exit(0);

