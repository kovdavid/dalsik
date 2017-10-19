#!/usr/bin/env perl

use strict;
use warnings;

my $serial = shift || "/dev/ttyACM0";

`stty -F $serial 9600 raw -echo`;

open my $fh, "+<:raw", $serial or die "Could not open $serial";

for my $row (0..3) {
    for my $col (0..7) {
        my $cmd = "DALSIKGET_KEY\x00".chr($row).chr($col);

        unless (print $fh "$cmd\n") {
            die "Could send command row:$row col:$col: $!";
        }

        sleep 1;

        my $buffer = "";
        sysread($fh, $buffer, 65545);

        if ($buffer !~ 'CMD_OK') {
            die "Command failed for row:$row col:$col: \n$buffer";
        }

        if ($buffer =~ /(KEY<\w+\|\d+>)/) {

            print "row:$row col:$col $1\n";
        } else {
            die "Wrong key format for row:$row col:$col: $buffer\n";
        }
    }

    print "\n";
}

close $fh;

sub to_hex {
    my ($str) = @_;
    join("-",
        map { sprintf "%02X", ord $_ }
        split //, $str);
}
