#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
my $side = 'UNSET';

GetOptions(
    "serial=s" => \$serial,
    "side=s" => \$side,
);

if ($side ne 'L' && $side ne 'R') {
    die "Invalid side[$side]! Usage: $0 [--serial /dev/ttyACM0] --side L|R";
}

my $fh = Dalsik::open_serial($serial);

print "Sending:\n\tSET_KEYBOARD_SIDE $side\n";
my $cmd = Dalsik::get_cmd('SET_KEYBOARD_SIDE', $side);

unless (print $fh $cmd) {
    die "Could not send SET_KEYBOARD_SIDE: $!";
}

my $res = <$fh>;

print "Received:\n";
print "\t$res";

close $fh;
