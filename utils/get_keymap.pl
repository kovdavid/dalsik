#!/usr/bin/env perl

use strict;
use warnings;

my $serial = shift || "/dev/ttyACM0";

`stty -F $serial 9600 raw -echo`;

open my $fh, "+<:raw", $serial or die "Could not open $serial";

unless (print $fh "DALSIK-KEYMAP\n") {
    die "Could send command KEYMAP: $!";
}

while (my $line = <$fh>) {
    if ($line =~ /^CMD_ERROR/) {
        die "Error: $line";
    }
    if ($line =~ /^CMD_OK/) {
        last;
    }
    print "$line";
}

close $fh;
