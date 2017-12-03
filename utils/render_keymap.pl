#!/usr/bin/env perl

use v5.10;
use strict;
use warnings;

use FindBin qw[ $Bin ];
use lib "$Bin";
use Dalsik;
use List::Util qw[ sum ];

my $fh;

my $file = shift;
if ($file) {
    open $fh, "<", $file or die "Unable to open file($file): $!";
} else {
    $fh = *STDIN;
}

my $data = {};
my $column_lengths_on_layers = {};
my $num_keys_on_layers = {};
my $tapdances = {};

my $num_rows = 0;
my $num_cols = 0;

while (my $line = <$fh>) {
    if ($line =~ /KEY<L(\d+)-R(\d+)-C(\d+)\|(\w+)\|(\w+)>/) {
        my $layer = $1+0;
        my $row = $2+0;
        my $col = $3+0;
        my $type_str = $4;
        my $key_hex = $5;

        my $str = Dalsik::type_and_key_to_str($type_str, $key_hex);
        if ($str ne 'UNSET' && $str ne 'TRANS' && $str ne 'KC_NO') {
            $num_keys_on_layers->{$layer}++;
        }

        $data->{$layer}->{$row}->{$col} = $str;
        if (
            !$column_lengths_on_layers->{$layer}->{$col}
            || length($str) > $column_lengths_on_layers->{$layer}->{$col}
        ) {
            $column_lengths_on_layers->{$layer}->{$col} = length($str);
        }

        if ($row+1 > $num_rows) {
            $num_rows = $row+1;
        }
        if ($col+1 > $num_cols) {
            $num_cols = $col+1;
        }
    } elsif ($line =~ /TAPDANCE<I(\d+)-T(\d+)\|(\w+)\|(\w+)>/) {
        my $index = $1+0;
        my $tap = $2+0;
        my $type_str = $3;
        my $key_hex = $4;

        my $str = Dalsik::type_and_key_to_str($type_str, $key_hex);
        if ($str ne 'UNSET' && $str ne 'TRANS' && $str ne 'KC_NO') {
            $tapdances->{$index}->{$tap} = $str;
        }
    }
}
close $fh;

for my $layer (sort keys %{ $data }) {
    next unless $num_keys_on_layers->{$layer};
    render_layer($layer, $data->{$layer});
    say "\n";
}
for my $tapdance (sort keys %{ $tapdances }) {
    render_tapdance($tapdance, $tapdances->{$tapdance});
}

sub render_layer {
    my ($layer, $rows) = @_;

    my $col_lengths = $column_lengths_on_layers->{$layer};
    my $col_length_sum = sum(values %{ $col_lengths });

    say "Layer:$layer";
    say (('-')x($col_length_sum+4+($num_cols-1)*3));

    for my $row (sort { $a <=> $b } keys %{ $rows }) {
        print "| ";
        for my $col (sort { $a <=> $b } keys %{ $rows->{$row} }) {
            if ($col+1 == $num_cols) {
                my $len = $col_lengths->{$col};
                printf "%-${len}s |", $rows->{$row}->{$col};
            } else {
                my $len = $col_lengths->{$col};
                printf "%-${len}s | ", $rows->{$row}->{$col};
            }
        }
        print "\n";

        say (('-')x($col_length_sum+4+($num_cols-1)*3));
    }
}

sub render_tapdance {
    my ($tapdance, $taps) = @_;
    say "TapDance $tapdance [$taps->{1} - $taps->{2} - $taps->{3}]";
}
