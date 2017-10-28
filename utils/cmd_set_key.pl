#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];

use lib "$Bin";

use Dalsik;

my $serial = shift || "/dev/ttyACM0";

my $fh = Dalsik::open_serial($serial);

my $layer = 0;
my $row = 0;
my $col = 1;
my $key_type = 1;
my $key = 4;

print "Sending:\n\tSET_KEY\n";
my $cmd = Dalsik::get_cmd('SET_KEY', $layer, $row, $col, $key_type, $key);
unless (print $fh $cmd) {
    die "Could not send SET_KEY: $!";
}

my $res = <$fh>;

print "Received:\n";
print "\t$res";

close $fh;
