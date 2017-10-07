#!/usr/bin/env perl

use strict;
use warnings;

my $serial = shift || "/dev/ttyACM0";

`stty -F $serial 9600 raw -echo`;

open my $fh, ">:raw", $serial or die "Could not open $serial";

my $keymap = [
    ["\x59", "\x5a", "\x5b", "\x04"],
    ["\x5c", "\x5d", "\x5e", "\x05"],
    ["\x5f", "\x60", "\x61", "\x06"],
    ["\x57", "\x62", "\x56", "\x07"],
];

my $row_num = 0;
for my $row (@{ $keymap }) {
    my $col_num = 0;
    for my $col (@{ $row }) {
        my $prefix = "DAVSSET_KEY\x00".chr($row_num).chr($col_num);
        unless (print $fh "$prefix\x01${col}\x00\n") {
            die "Could not print row:$row_num col:$col_num: $!";
        }
        $col_num++;
    }

    $row_num++;
}

close $fh;
