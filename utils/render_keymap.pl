#!/usr/bin/env perl

use v5.10;
use strict;
use warnings;

use Text::SimpleTable;
use List::Util qw[ sum min max ];
use FindBin qw[ $Bin ];
use lib "$Bin";

use Dalsik;

binmode(STDOUT, ":utf8");

my $fh;

my $file = shift;
if ($file) {
    open $fh, "<", $file or die "Unable to open file($file): $!";
} else {
    $fh = *STDIN;
}

my $data = {};
my $column_lengths = {};
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

        $str =~ s/\bMINUS\b/-/;
        $str =~ s/\bSCOLON\b/:/;
        $str =~ s/\bSLSH\b/\//;
        $str =~ s/\bQUOTE\b/'/;
        $str =~ s/\bEQUAL\b/=/;
        $str =~ s/\bBSLS\b/\\/;
        $str =~ s/\bDOT\b/./;
        $str =~ s/\bCOMMA\b/,/;
        $str =~ s/\bKC_NO\b/ /;
        $str =~ s/\bLBRC\b/[/;
        $str =~ s/\bRBRC\b/]/;
        $str =~ s/\bGRV\b/`/;

        $data->{$layer}->{$row}->{$col} = $str;
        if (
            !$column_lengths->{$col}
            || length($str) > $column_lengths->{$col}
        ) {
            $column_lengths->{$col} = length($str);
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
    print "\n";
}

say "TapDance:";
for my $tapdance (sort keys %{ $tapdances }) {
    render_tapdance($tapdance, $tapdances->{$tapdance});
}

sub render_layer {
    my ($layer, $rows) = @_;

    say "Layer:$layer";

    my $t = Text::SimpleTable->new(
        map { $column_lengths->{$_} } 0..$num_cols-1
    );

    for my $row (sort { $a <=> $b } keys %{ $rows }) {
        $t->row(
            map { $rows->{$row}->{$_} } 0..$num_cols-1
        );
    }

    print $t->boxes->draw();
}

sub render_tapdance {
    my ($tapdance, $taps) = @_;
    say "TD($tapdance) [$taps->{1} - $taps->{2} - $taps->{3}]";
}
