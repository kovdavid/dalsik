#!/usr/bin/env perl

use strict;
use warnings;

my $serial = shift || "/dev/ttyACM0";

open my $fh, "+<:raw", $serial or die "Could not open $serial";

for my $row (0..3) {
    for my $col (0..3) {
        my $cmd = "DAVSGET_KEY\x00".chr($row).chr($col);

        unless (print $fh "$cmd\n") {
            die "Could send command row:$row col:$col: $!";
        }

        my $key = <$fh>;
        my $result = <$fh>;
        chop $key;
        chop $result;
        chop $result;

        if ($result ne 'CMD_OK') {
            die "Command failed for row:$row col:$col: \n$key\n$result\n";
        }

        if ($key =~ /^KEY\|(\d+)\|(\d+)\|(\d+)|/) {
            my $type = $1;
            my $byte1 = $2;
            my $byte2 = $3;

            print "row:$row col:$col type:$type b1:$byte1 b2:$byte2\n";
        } else {
            die "Wrong key format for row:$row col:$col: $key\n";
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
