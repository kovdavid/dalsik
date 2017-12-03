#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw[ $Bin ];

use lib "$Bin";

use Dalsik;

my $serial = shift || "/dev/ttyACM0";

my $fh = Dalsik::open_serial($serial);

print "Sending:\n\tGET_TAPDANCE_KEYMAP\n";
my $cmd = Dalsik::get_cmd('GET_TAPDANCE_KEYMAP');
unless (print $fh $cmd) {
    die "Could not send GET_TAPDANCE_KEYMAP: $!";
}

print "Received:\n";
while (my $line = <$fh>) {
    print "\t$line";

    if ($line =~ /^CMD/) {
        last;
    }
}

close $fh;
