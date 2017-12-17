#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];
use Getopt::Long qw( GetOptions );

use lib "$Bin";

use Dalsik;

my $serial = '/dev/ttyACM0';
my $layer = 0;
my $row = 0;
my $col = 0;
my $key_type = 1; # Normal key
my $key = 4; # KC_A

GetOptions(
    "serial=s"   => \$serial,
    "layer=i"    => \$layer,
    "row=i"      => \$row,
    "col=i"      => \$col,
    "key_type=i" => \$key_type,
    "key=i"      => \$key,
);

my $fh = Dalsik::open_serial($serial);

print "Sending:\n\tSET_KEY\n";
my $cmd = Dalsik::get_cmd('SET_KEY', $layer, $row, $col, $key_type, $key);

unless (print $fh $cmd) {
    die "Could not send SET_KEY: $!";
}

my $res = <$fh>;

print "Received:\n";
print "\t$res";

close $fh;
