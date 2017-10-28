#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];

use lib "$Bin";

use Dalsik;

my $serial = shift || "/dev/ttyACM0";

my $fh = Dalsik::open_serial($serial);

print "Sending:\n\tCLEAR_KEYMAP\n";
my $cmd = Dalsik::get_cmd('CLEAR_KEYMAP');
unless (print $fh $cmd) {
    die "Could not send CLEAR_KEYMAP: $!";
}

my $res = <$fh>;
my $cmd_res = <$fh>;

print "Received:\n";
print "\t$res";
print "\t$cmd_res";

close $fh;
