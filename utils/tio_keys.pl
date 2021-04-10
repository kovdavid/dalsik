#!/usr/bin/env perl

use v5.10;
use strict;
use warnings;

use Data::Dumper;
use Getopt::Std;
use JSON qw[ decode_json ];
use List::Util qw[ min max ];

use FindBin qw[ $Bin ];
use lib "$Bin";
use Dalsik;

our ($opt_j);
getopts('j:');

my $log = $1 || "/tmp/tio.log";

open my $fh, "<", $log or die "Could not open log $log: $!";

my $pressed_keys = {};
my $pressed_keys_count = 0;

while (my $line = <$fh>) {
    $line =~ s/^\s+(.*)\s+$/$1/g;
    next unless $line;

    if ($line =~ m/^(Slave|Master) ChangedKeyCoords <(REL|PRE)\|ROW:(\d+)\|COL:(\d+)> now:(\d+)$/) {
        # my $side = $1;
        my $action = $2;
        my $row = $3;
        my $col = $4;
        my $timestamp = $5;

        if ($action eq 'PRE') { # Press
            $pressed_keys->{ $row }->{ $col } = $timestamp;
            $pressed_keys_count++;
        } else { # Release
            delete $pressed_keys->{ $row }->{ $col };
            unless (%{ $pressed_keys->{ $row } }) {
                delete $pressed_keys->{ $row };
            }
            $pressed_keys_count--;
        }
    }

    if ($line =~ m/^MasterReport\[BASE]:(.*)$/) {
        my ($mods, undef, @keys) = split(/\|/, $1);

        my @pressed_keys = grep { $_ } @keys;

        if (scalar(@pressed_keys) > 1) {
            my $str1 = join("-", @pressed_keys);
            my $str2 = join("-", map { $Dalsik::byte_to_key->{hex($_)} } @pressed_keys);

            my @timestamps;
            for my $row (keys %{ $pressed_keys }) {
                for my $col (keys %{ $pressed_keys->{$row} }) {
                    push @timestamps, $pressed_keys->{$row}->{$col};
                }
            }

            my $min = min(@timestamps);
            my $max = max(@timestamps);
            my $diff = $max - $min;

            say "$str1 => $str2 [$diff]";

        }

    }
}
