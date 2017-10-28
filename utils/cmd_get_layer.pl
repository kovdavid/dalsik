#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];

use lib "$Bin";

use Dalsik;

my $serial = shift || "/dev/ttyACM0";

my $fh = Dalsik::open_serial($serial);

print "Sending:\n\tGET_LAYER\n";
my $cmd = Dalsik::get_cmd('GET_LAYER');
unless (print $fh $cmd) {
    die "Could not send GET_LAYER: $!";
}

my $res = <$fh>;
my $cmd_res = <$fh>;

print "Received:\n";
print "\t$res";
print "\t$cmd_res";

close $fh;
