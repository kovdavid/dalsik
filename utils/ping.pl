#!/usr/bin/env perl

use strict;
use warnings;

my $serial = shift || "/dev/ttyACM0";

`stty -F $serial 9600 raw -echo`;

print "Sending PING\n";

open my $fh, "+<:raw", $serial or die "Could not open $serial";
unless (print $fh "DALSIK-PING\n") {
    die "Could not clear eeprom: $!";
}
sleep 1;

my $buffer = "";
sysread($fh, $buffer, 65545);

print "Received: $buffer\n";

close $fh;
