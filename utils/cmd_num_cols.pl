#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
GetOptions("serial=s" => \$serial);

my $fh = Dalsik::open_serial($serial);

print "Sending:\n\tNUM_COLS\n";
my $cmd = Dalsik::get_cmd('NUM_COLS');
unless (print $fh $cmd) {
    die "Could not send NUM_COLS: $!";
}

my $res = <$fh>;
my $cmd_res = <$fh>;

print "Received:\n";
print "\t$res";
print "\t$cmd_res";

close $fh;
