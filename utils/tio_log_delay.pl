#!/usr/bin/env perl

use v5.10;
use strict;
use warnings;

my $log = $1 || "/tmp/tio.log";
my $last_ts;

open my $fh, "<", $log or die "Could not open log $log: $!";

my $result = {};

while (my $line = <$fh>) {
    $line =~ s/\s+$//g;
    next unless $line;

    if ($line =~ m/^\w+ ChangedKeyCoords .* now:(\d+)$/) {
        my $ts = $1;
        if ($last_ts) {
            my $diff = $ts - $last_ts;
            $result->{ $diff }++;
        }
        $last_ts = $ts;
    }
}

for my $key (sort { $a <=> $b } keys %{ $result }) {
    next if $key > 200;
    say "$key ms => $result->{$key}";
}
