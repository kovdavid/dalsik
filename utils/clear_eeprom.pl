#!/usr/bin/env perl

use strict;
use warnings;

my $serial = shift || "/dev/ttyACM0";

`stty -F $serial 9600 raw -echo`;

open my $fh, ">:raw", $serial or die "Could not open $serial";
unless (print $fh "DALSIK-CLEAR_EEPROM\n") {
    die "Could not clear eeprom: $!";
}

close $fh;
