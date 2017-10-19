#!/usr/bin/env perl

use strict;
use warnings;

my $serial = shift || "/dev/ttyACM0";

`stty -F $serial 9600 raw -echo`;

open my $fh, ">:raw", $serial or die "Could not open $serial";

# \x01 - NORMAL_KEY

my $keymap = [
    ["\x01\x59", "\x01\x5a", "\x01\x5b", "\x01\x04", "\x01\x04"],
    ["\x01\x5c", "\x01\x5d", "\x01\x5e", "\x01\x05"],
    ["\x01\x5f", "\x01\x60", "\x01\x61", "\x01\x06"],
    ["\x04\x57", "\x01\x62", "\x01\x56", "\x01\x07"],
];

my $row_num = 0;
for my $row (@{ $keymap }) {
    my $col_num = 0;
    for my $col (@{ $row }) {
        my $prefix = "DALSIKSET_KEY\x00".chr($row_num).chr($col_num);
        unless (print $fh "$prefix${col}\n") {
            die "Could not print row:$row_num col:$col_num: $!";
        }
        $col_num++;
    }

    $row_num++;
}

close $fh;
